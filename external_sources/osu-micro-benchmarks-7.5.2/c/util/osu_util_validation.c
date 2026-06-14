/*
 * Copyright (c) 2023-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include "osu_util.h"
#include "osu_util_mpi.h"

/* not consistent with errno.h, but errno.h conflicts with CUDA_CHECK macro. */
#define ENODATA 2

static char rank_buffer_type = '\0';
struct atomic_dv_summary {
    MPI_Datatype datatype;
    MPI_Op op;
    size_t trials;
    size_t validation_failures;
    size_t validations_performed;
    size_t first_failure;
    size_t last_failure;
    struct atomic_dv_summary *next;
};
#define bool _Bool
struct atomic_dv_summary *dv_summary_root = NULL;
char osc_str_output[OMB_DATATYPE_STR_MAX_LEN];

static char inline get_rank_buffer_type()
{
    if (rank_buffer_type == '\0') {
        int rank = 0;
        MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
        rank_buffer_type = (rank == 0) ? options.src : options.dst;
    }
    return rank_buffer_type;
}

static int set_hmem_buffer(void *dst, void *src, size_t size)
{
    char buf_type = get_rank_buffer_type();

    if (buf_type == 'H' || options.accel == NONE) {
        memcpy(dst, src, size);
        return 0;
    }
    switch (options.accel) {
#ifdef _ENABLE_CUDA_
        case CUDA:
            if (buf_type == 'M') {
                memcpy(dst, src, size);
            } else {
                CUDA_CHECK(cudaMemcpy(dst, src, size, cudaMemcpyHostToDevice));
            }
            CUDA_CHECK(cudaDeviceSynchronize());
            break;
#endif
        default:
            fprintf(stderr, "Memory copy not implemented for the selected "
                            "acceleration platform\n");
            return -1;
    }
    return 0;
}

static int get_hmem_buffer(void *dst, void *src, size_t size)
{
    if (rank_buffer_type == 'H' || options.accel == NONE) {
        memcpy(dst, src, size);
        return 0;
    }
    switch (options.accel) {
#ifdef _ENABLE_CUDA_
        case CUDA:
            if (rank_buffer_type == 'M') {
                memcpy(dst, src, size);
            } else {
                CUDA_CHECK(cudaMemcpy(dst, src, size, cudaMemcpyDeviceToHost));
            }
            CUDA_CHECK(cudaDeviceSynchronize());
            break;
#endif
        default:
            fprintf(stderr, "Memory copy not implemented for the selected "
                            "acceleration platform\n");
            return -1;
    }
    return 0;
}

char *osc_tostr(void *val)
{
    memset(osc_str_output, 0, sizeof(osc_str_output));
    MPI_Datatype type = *(MPI_Datatype *)val;
    if (type == MPI_CHAR)
        sprintf(osc_str_output, "%s", "MPI_CHAR");
    if (type == MPI_INT)
        sprintf(osc_str_output, "%s", "MPI_INT");
    if (type == MPI_FLOAT)
        sprintf(osc_str_output, "%s", "MPI_FLOAT");
    return osc_str_output;
}

int mpi_dtype_enumerate(MPI_Datatype dtype)
{
    if (dtype == MPI_CHAR)
        return 1;
    if (dtype == MPI_INT)
        return 2;
    if (dtype == MPI_FLOAT)
        return 3;
    return -1;
}

/*
 * @brief Prints a summary of test failures.
 * @return 0 if all validations passed, <0 if any failures recorded.
 */
int atomic_data_validation_print_summary()
{
    int retval = 0;
    char type_str[10] = {0};
    char op_str[32] = {0};
    char test_name[64] = {};
    int validation_combos = 0;
    int failure_count = 0;

    struct atomic_dv_summary *node = dv_summary_root;
    struct atomic_dv_summary *next = NULL;

    if (!node) {
        fprintf(stderr, "SKIPPED: No validations were performed!\n");
        return 0;
    }

    while (node) {
        snprintf(type_str, sizeof(type_str) - 1, "%s",
                 osc_tostr(&node->datatype));
        snprintf(op_str, sizeof(op_str) - 1, "MPI_SUM");
        snprintf(test_name, sizeof(test_name), "%s on %s", op_str, type_str);
        validation_combos += 1;

        if (node->validation_failures) {
            fprintf(stdout,
                    "FAILED: %s had %zu of %zu tests fail data validation.\n",
                    test_name, node->validation_failures, node->trials);
            fprintf(
                stdout,
                "\t\tFirst failure at trial %zu, last failure at trial %zu.\n",
                node->first_failure, node->last_failure);
            retval = -1;
            failure_count++;
        } else if (node->validations_performed < node->trials) {
            fprintf(stdout, "SKIPPED: Data validation not available for %s\n",
                    test_name);
            retval = -1;
            failure_count++;
        }
        next = node->next;
        free(node);
        node = next;
    }
    if (retval == 0) {
        fprintf(
            stdout,
            "PASSED: All %d combinations of ops and datatypes tested passed.\n",
            validation_combos);
    } else {
        fprintf(stdout,
                "FAILED: %d of the %d combinations of ops and datatypes tested "
                "failed.\n",
                failure_count, validation_combos);
    }
    dv_summary_root = NULL;
    return retval;
}

static void atomic_dv_record(MPI_Datatype dtype, MPI_Op op, bool failed,
                             bool checked)
{
    struct atomic_dv_summary *node = dv_summary_root;

    if (!node || node->op != op || node->datatype != dtype) {
        node = calloc(1, sizeof(struct atomic_dv_summary));
        node->next = dv_summary_root;
        dv_summary_root = node;
        node->op = op;
        node->datatype = dtype;
    }

    node->trials++;
    if (failed) {
        if (node->validation_failures == 0)
            node->first_failure = node->trials;
        node->last_failure = node->trials;
        node->validation_failures++;
    }
    if (checked)
        node->validations_performed++;
}

#define ATOM_FOR_DMPI_SUM(a, ao, b) (ao) = ((a) + (b))
#define ATOM_FOR_DMPI_CSWAP(a, ao, b, c)                                       \
    if ((c) == (a)) {                                                          \
        (ao) = (b);                                                            \
    }

#define atomic_case(ftype, fop)                                                \
    case ENUM_OF_##ftype: {                                                    \
        if (result)                                                            \
            *(ATOM_CTYPE_FOR_##ftype *)result =                                \
                *(ATOM_CTYPE_FOR_##ftype *)addr_in;                            \
        ATOM_FOR_##fop(*(ATOM_CTYPE_FOR_##ftype *)addr_in,                     \
                       *(ATOM_CTYPE_FOR_##ftype *)addr_out,                    \
                       *(ATOM_CTYPE_FOR_##ftype *)buf);                        \
        break;                                                                 \
    }

#define atomic_case_cas(ftype)                                                 \
    case ENUM_OF_##ftype: {                                                    \
        if (result) {                                                          \
            *(ATOM_CTYPE_FOR_##ftype *)result =                                \
                *(ATOM_CTYPE_FOR_##ftype *)addr_in;                            \
        }                                                                      \
        ATOM_FOR_DMPI_CSWAP(*(ATOM_CTYPE_FOR_##ftype *)addr_in,                \
                            *(ATOM_CTYPE_FOR_##ftype *)addr_out,               \
                            *(ATOM_CTYPE_FOR_##ftype *)buf,                    \
                            *(ATOM_CTYPE_FOR_##ftype *)compare);               \
        break;                                                                 \
    }

#define atomic_int_ops(dtype) atomic_case(dtype, DMPI_SUM)

#define atomic_real_float_ops(dtype) atomic_case(dtype, DMPI_SUM)

int perform_atomic_op(MPI_Datatype dtype, MPI_Op op, void *addr_in, void *buf,
                      void *addr_out, void *compare, void *result)
{
    int dtype_enumeration = mpi_dtype_enumerate(dtype);
    if (op != MPI_SUM) {
        addr_out = addr_in;
        return 0;
    }
    switch (dtype_enumeration) {
        atomic_int_ops(DMPI_CHAR) atomic_int_ops(DMPI_INT)
            atomic_real_float_ops(DMPI_FLOAT) default : return -1;
    }
    return 0;
}

int perform_atomic_cas(MPI_Datatype dtype, void *addr_in, void *buf,
                       void *addr_out, void *compare, void *result)
{
    int dtype_enumeration = mpi_dtype_enumerate(dtype);
    switch (dtype_enumeration) {
        atomic_case_cas(DMPI_CHAR) atomic_case_cas(DMPI_INT)
            atomic_case_cas(DMPI_FLOAT) default : return -1;
    }
    return 0;
}

static int validation_input_value(MPI_Datatype dtype, int jrank, void *val)
{
    if (dtype == MPI_CHAR)
        *(char *)val = (1 + jrank) * 10;
    else if (dtype == MPI_INT)
        *(int *)val = (1 + jrank) * 10;
    else if (dtype == MPI_FLOAT)
        *(float *)val = (1 + jrank) * 1.11f;
    else {
        fprintf(stderr,
                "No initial value defined, cannot perform data validation "
                "on atomic operations using %s\n",
                osc_tostr(&dtype));
        return -1;
    }
    return 0;
}

#define COMPARE_AS_TYPE(c_type, a, b) *(c_type *)(a) == *(c_type *)(b)
static int atom_binary_compare(MPI_Datatype dtype, void *a, void *b)
{
    int dtype_size = 0;
    int err;

    if (dtype == MPI_FLOAT) {
        return COMPARE_AS_TYPE(ATOM_CTYPE_FOR_DMPI_FLOAT, a, b);
    }

    err = MPI_Type_size(dtype, &dtype_size);
    if (err)
        return 0;

    switch (dtype_size) {
        case 1:
            return COMPARE_AS_TYPE(__int8_t, a, b);
        case 2:
            return COMPARE_AS_TYPE(__int16_t, a, b);
        case 4:
            return COMPARE_AS_TYPE(__int32_t, a, b);
        case 8:
            return COMPARE_AS_TYPE(__int64_t, a, b);
        case 16:
            return COMPARE_AS_TYPE(__int128_t, a, b);
    }
    return 0;
}

int atomic_data_validation_setup(MPI_Datatype datatype, int jrank, void *buf,
                                 size_t buf_size)
{
    char set_value[64];
    char *set_buf;
    int jatom;
    int dtype_size;
    size_t natoms;
    int err;

    set_buf = calloc(buf_size, 1);
    err = MPI_Type_size(datatype, &dtype_size);
    if (err)
        goto exit_path;
    natoms = buf_size / dtype_size;
    err = validation_input_value(datatype, jrank, set_value);
    if (err)
        goto exit_path;
    for (jatom = 0; jatom < natoms; jatom++) {
        memcpy(set_buf + jatom * dtype_size, set_value, dtype_size);
    }
    err = set_hmem_buffer(buf, set_buf, buf_size);
exit_path:
    free(set_buf);
    return err;
}

#define PRINT_ADR_COMPARISON(dtype, fmt, ai, bi, ci, ao, ae)                   \
    fprintf(stderr,                                                            \
            "Initial Values: [local]addr=" fmt ", [remote]buf=" fmt            \
            ", [remote]compare=" fmt "\n"                                      \
            "Observed Final Value: addr=" fmt "\n"                             \
            "Expected Final Value: addr=" fmt "\n",                            \
            *(ATOM_CTYPE_FOR_##dtype *)(ai), *(ATOM_CTYPE_FOR_##dtype *)(bi),  \
            *(ATOM_CTYPE_FOR_##dtype *)(ci), *(ATOM_CTYPE_FOR_##dtype *)(ao),  \
            *(ATOM_CTYPE_FOR_##dtype *)(ae));

#define PRINT_RES_COMPARISON(dtype, fmt, ai, bi, ci, ro, re)                   \
    fprintf(stderr,                                                            \
            "Initial Values: [remote]addr=" fmt ", [local]buf=" fmt            \
            ", [local]compare=" fmt "\n"                                       \
            "Observed Final Value: result=" fmt "\n"                           \
            "Expected Final Value: result=" fmt "\n",                          \
            *(ATOM_CTYPE_FOR_##dtype *)(ai), *(ATOM_CTYPE_FOR_##dtype *)(bi),  \
            *(ATOM_CTYPE_FOR_##dtype *)(ci), *(ATOM_CTYPE_FOR_##dtype *)(ro),  \
            *(ATOM_CTYPE_FOR_##dtype *)(re))

static void print_failure_message(MPI_Datatype datatype, void *adr_in,
                                  void *buf_in, void *compare_in, void *adr_obs,
                                  void *res_obs, void *adr_expect,
                                  void *res_expect)
{
    if (datatype == MPI_CHAR) {
        if (adr_obs)
            PRINT_ADR_COMPARISON(DMPI_CHAR, "%d", adr_in, buf_in, compare_in,
                                 adr_obs, adr_expect);
        if (res_obs)
            PRINT_RES_COMPARISON(DMPI_CHAR, "%d", adr_in, buf_in, compare_in,
                                 res_obs, res_expect);
    }
    if (datatype == MPI_INT) {
        if (adr_obs)
            PRINT_ADR_COMPARISON(DMPI_INT, "%d", adr_in, buf_in, compare_in,
                                 adr_obs, adr_expect);
        if (res_obs)
            PRINT_RES_COMPARISON(DMPI_INT, "%d", adr_in, buf_in, compare_in,
                                 res_obs, res_expect);
    }
    if (datatype == MPI_FLOAT) {
        if (adr_obs)
            PRINT_ADR_COMPARISON(DMPI_FLOAT, "%f", adr_in, buf_in, compare_in,
                                 adr_obs, adr_expect);
        if (res_obs)
            PRINT_RES_COMPARISON(DMPI_FLOAT, "%f", adr_in, buf_in, compare_in,
                                 res_obs, res_expect);
    }
}

/**
 * Checks the result of an operation descriped by op.
 *
 * Arguments:
 *   datatype: the type of data being operated on
 *   op: The operation to do.  -1 means Compare-and-swap.
 *   jrank: the rank of this processes
 *   addr: the pointer to a local buffer that may have been operated on
 *   res: the fetch result from the operation
 *   buf_size: the number of bytes operated on
 *   check_addr: true if the remote has done an operation on addr.
 *   check_result: true if this process did an operation on a remote.
 *   validation_results: a bitmask which is updated.  User should set to 0.
 *          |=1 when failure occurred.
 *          |=2 when no validation performed.
 * Note that addr and res might be in GPU memory or in system memory.
 * Validation will only pass if both local and remote memories were initialized
 * with atomic_data_validation_setup.
 *
 * Additionally validation results are saved in a list and can be printed with
 * atomic_data_validation_print_summary().
 */
int atomic_data_validation_check(MPI_Datatype datatype, MPI_Op op, int jrank,
                                 void *addr, void *res, size_t buf_size,
                                 bool check_addr, bool check_result,
                                 int *validation_results)
{
    /* these all fit the maximum atom size of 256 bits. */
    const int MAX_ATOM_BYTES = 64;
    char local_addr[MAX_ATOM_BYTES], remote_addr[MAX_ATOM_BYTES];
    char local_buf[MAX_ATOM_BYTES], remote_buf[MAX_ATOM_BYTES];
    char local_compare[MAX_ATOM_BYTES], remote_compare[MAX_ATOM_BYTES];
    char expected_local_addr[MAX_ATOM_BYTES], dummy_remote_addr[MAX_ATOM_BYTES];
    char expected_local_result[MAX_ATOM_BYTES];

    char *local_addr_in_sysmem = NULL;
    char *local_result_in_sysmem = NULL;
    int dtype_size;
    size_t natoms;
    int jatom;
    int err, addr_eq, res_eq, any_errors = 0;
    int jrank_remote = (jrank + 1) % 2;

    err = MPI_Type_size(datatype, &dtype_size);
    if (err)
        return err;

    natoms = buf_size / dtype_size;
    natoms = 1;
    local_addr_in_sysmem = malloc(buf_size);
    OMB_CHECK_NULL_AND_EXIT(local_addr_in_sysmem, "Unable to allocate memory");
    local_result_in_sysmem = malloc(buf_size);
    OMB_CHECK_NULL_AND_EXIT(local_result_in_sysmem,
                            "Unable to allocate memory");

    /* setup initial conditions so we can mock the test */
    err = validation_input_value(datatype, jrank, local_addr);
    err |= validation_input_value(datatype, jrank, local_buf);
    err |= validation_input_value(datatype, jrank, local_compare);
    err |= validation_input_value(datatype, jrank, expected_local_addr);
    err |= validation_input_value(datatype, jrank_remote, remote_addr);
    err |= validation_input_value(datatype, jrank_remote, remote_buf);
    err |= validation_input_value(datatype, jrank_remote, remote_compare);
    if (err == -ENODATA)
        goto nocheck;
    if (err)
        goto error;

    if ((long long)op == -1) {
        /* mock the remote side performing CAS on our local addr */
        err = perform_atomic_cas(datatype, local_addr, remote_buf,
                                 expected_local_addr, remote_compare, NULL);
        /* mock the local side performing CAS on remote addr    */
        err |= perform_atomic_cas(datatype, remote_addr, local_buf,
                                  dummy_remote_addr, local_compare,
                                  expected_local_result);
    } else {
        /* mock the remote side performing operations on our local addr */
        err = perform_atomic_op(datatype, op, local_addr, remote_buf,
                                expected_local_addr, remote_compare, NULL);
        /* mock the local side performing operations on remote addr */
        err |= perform_atomic_op(datatype, op, remote_addr, local_buf,
                                 dummy_remote_addr, local_compare,
                                 expected_local_result);
    }
    if (err == -ENODATA)
        goto nocheck;
    if (err)
        goto error;

    err = 0;
    if (check_addr)
        err |= get_hmem_buffer(local_addr_in_sysmem, addr, buf_size);
    if (check_result)
        err |= get_hmem_buffer(local_result_in_sysmem, res, buf_size);

    if (err)
        goto error;
    for (jatom = 0; jatom < natoms; jatom++) {
        addr_eq = 1;
        res_eq = 1;
        if (check_addr) {
            addr_eq =
                atom_binary_compare(datatype, expected_local_addr,
                                    local_addr_in_sysmem + jatom * dtype_size);
        }
        if (!addr_eq) {
            fprintf(stderr, "FAILED:");
            fprintf(stderr, " %s failed validation of addr at atom index %d.\n",
                    osc_tostr(&datatype), jatom);
            print_failure_message(datatype, local_addr, remote_buf,
                                  remote_compare,
                                  local_addr_in_sysmem + jatom * dtype_size,
                                  NULL, expected_local_addr, NULL);
        }
        if (check_result) {
            res_eq = atom_binary_compare(datatype, expected_local_result,
                                         local_result_in_sysmem +
                                             jatom * dtype_size);
        }
        if (!res_eq) {
            fprintf(stderr, "FAILED: ");
            fprintf(stderr,
                    " %s failed validation of result at atom index %d.\n",
                    osc_tostr(&datatype), jatom);
            print_failure_message(datatype, remote_addr, local_buf,
                                  local_compare, NULL,
                                  local_result_in_sysmem + jatom * dtype_size,
                                  NULL, expected_local_result);
        }
        if (!res_eq || !addr_eq) {
            any_errors = 1;
            break;
        }
    }
    atomic_dv_record(datatype, op, any_errors, 1);
    if (any_errors)
        *validation_results |= 1;
    free(local_result_in_sysmem);
    free(local_addr_in_sysmem);
    return 0;

nocheck:
    atomic_dv_record(datatype, op, 0, 0);
    *validation_results |= 2;
    free(local_result_in_sysmem);
    free(local_addr_in_sysmem);
    return 0;
error:
    atomic_dv_record(datatype, op, 0, 0);
    free(local_result_in_sysmem);
    free(local_addr_in_sysmem);
    return err;
}
