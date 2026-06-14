/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level directory.
 */

#include "osu_util_mpi.h"

MPI_Request request[MAX_REQ_NUM];
MPI_Status reqstat[MAX_REQ_NUM];
MPI_Request send_request[MAX_REQ_NUM];
MPI_Request recv_request[MAX_REQ_NUM];

MPI_Aint disp_remote;
MPI_Aint disp_local;

MPI_T_cvar_handle dynamic_tuning_cvar = NULL;

/* A is the A in DAXPY for the Compute Kernel */
#define A     2.0
#define DEBUG 0
/*
 * We are using a 2-D matrix to perform dummy
 * computation in non-blocking collective benchmarks
 */
#define DIM 25
static float a[DIM][DIM], x[DIM], y[DIM];

/* Validation multiplier constants*/
#define FLOAT_VALIDATION_MULTIPLIER  (float)2.0
#define DOUBLE_VALIDATION_MULTIPLIER (float)2.0
#define INT_VALIDATION_MULTIPLIER    (int)10
#define CHAR_VALIDATION_MULTIPLIER   (char)7
#define CHAR_RANGE                   (int)pow(2, __CHAR_BIT__)

#ifdef _ENABLE_CUDA_
CUcontext cuContext;
#endif

char const *win_info[20] = {
    "MPI_Win_create",
#if MPI_VERSION >= 3
    "MPI_Win_allocate",
    "MPI_Win_create_dynamic",
#endif
};

char const *sync_info[20] = {
    "MPI_Win_lock/unlock",
    "MPI_Win_post/start/complete/wait",
    "MPI_Win_fence",
#if MPI_VERSION >= 3
    "MPI_Win_flush",
    "MPI_Win_flush_local",
    "MPI_Win_lock_all/unlock_all",
#endif
};

#ifdef _ENABLE_CUDA_KERNEL_
/* Using new stream for kernels on gpu */
static cudaStream_t stream;
/* Using new stream and events for UM buffer handling */
static cudaStream_t um_stream;
static cudaEvent_t start, stop;

static int is_alloc = 0;

/* Arrays on device for dummy compute */
static float *d_x, *d_y;
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

void set_device_memory(void *ptr, int data, size_t size)
{
#ifdef _ENABLE_OPENACC_
    size_t i;
    char *p = (char *)ptr;
#endif

    switch (options.accel) {
#ifdef _ENABLE_CUDA_
        case CUDA:
            CUDA_CHECK(cudaMemset(ptr, data, size));
            break;
#endif
#ifdef _ENABLE_OPENACC_
        case OPENACC:
#pragma acc parallel copyin(size) deviceptr(p)
            for (i = 0; i < size; i++) {
                p[i] = data;
            }
            break;
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            ROCM_CHECK(hipMemset(ptr, data, size));
            break;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL:
            syclMemset(ptr, data, size);
            break;
#endif
        default:
            break;
    }
}

int free_device_buffer(void *buf)
{
    if (buf == NULL)
        return 0;
    switch (options.accel) {
#ifdef _ENABLE_CUDA_
        case CUDA:
            CUDA_CHECK(cudaFree(buf));
            break;
#endif
#ifdef _ENABLE_OPENACC_
        case OPENACC:
            acc_free(buf);
            break;
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            ROCM_CHECK(hipFree(buf));
            break;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL:
            syclFree(buf);
            break;
#endif
        default:
            /* unknown device */
            return 1;
    }

    buf = NULL;
    return 0;
}

void *align_buffer(void *ptr, unsigned long align_size)
{
    unsigned long buf =
        (((unsigned long)ptr + (align_size - 1)) / align_size * align_size);
    return (void *)buf;
}

void usage_one_sided(char const *name)
{
    if (accel_enabled) {
        fprintf(stdout, "Usage: %s [options] [SRC DST]\n\n", name);
        fprintf(
            stdout,
            "SRC and DST are buffer types for the source and destination\n");
        fprintf(
            stdout,
            "SRC and DST may be `D', `H', or 'M' which specifies whether\n"
            "the buffer is allocated on the accelerator device memory, host\n"
            "memory or using CUDA Unified memory respectively for each mpi "
            "rank\n\n");
    } else {
        fprintf(stdout, "Usage: %s [options]\n", name);
    }

    fprintf(stdout, "Options:\n");
    print_help_message_common();
}

int process_one_sided_options(int opt, char *arg)
{
    switch (opt) {
        case 'w':
#if MPI_VERSION >= 3
            if (0 == strcasecmp(arg, "create")) {
                options.win = WIN_CREATE;
            } else if (0 == strcasecmp(arg, "allocate")) {
                options.win = WIN_ALLOCATE;
            } else if (0 == strcasecmp(arg, "dynamic")) {
                options.win = WIN_DYNAMIC;
            } else
#endif
            {
                return PO_BAD_USAGE;
            }
            break;
        case 's':
            if (0 == strcasecmp(arg, "pscw")) {
                options.sync = PSCW;
            } else if (0 == strcasecmp(arg, "fence")) {
                options.sync = FENCE;
            } else if (options.synctype == ALL_SYNC) {
                if (0 == strcasecmp(arg, "lock")) {
                    options.sync = LOCK;
                }
#if MPI_VERSION >= 3
                else if (0 == strcasecmp(arg, "flush")) {
                    options.sync = FLUSH;
                } else if (0 == strcasecmp(arg, "flush_local")) {
                    options.sync = FLUSH_LOCAL;
                } else if (0 == strcasecmp(arg, "lock_all")) {
                    options.sync = LOCK_ALL;
                }
#endif
                else {
                    return PO_BAD_USAGE;
                }
            } else {
                return PO_BAD_USAGE;
            }
            break;
        default:
            return PO_BAD_USAGE;
    }

    return PO_OKAY;
}

void usage_mbw_mr()
{
    if (accel_enabled) {
        fprintf(stdout, "Usage: osu_mbw_mr [options] [SRC DST]\n\n");
        fprintf(
            stdout,
            "SRC and DST are buffer types for the source and destination\n");
        fprintf(
            stdout,
            "SRC and DST may be `D', `H', or 'M' which specifies whether\n"
            "the buffer is allocated on the accelerator device memory, host\n"
            "memory or using CUDA Unified memory respectively for each mpi "
            "rank\n\n");
    } else {
        fprintf(stdout, "Usage: osu_mbw_mr [options]\n");
    }

    fprintf(stdout, "Options:\n");
    print_help_message_common();
    fprintf(stdout, "  Note: This benchmark relies on block ordering of the "
                    "ranks.  Please see\n");
    fprintf(stdout, "        the README for more information.\n");
    fflush(stdout);
}

void print_bad_usage_message(int rank)
{
    if (rank) {
        return;
    }

    if (bad_usage.optarg) {
        fprintf(stderr, "%s [-%c %s]\n\n", bad_usage.message,
                (char)bad_usage.opt, bad_usage.optarg);
    } else {
        fprintf(stderr, "%s [-%c]\n\n", bad_usage.message, (char)bad_usage.opt);
    }
    fflush(stderr);

    if (options.bench != ONE_SIDED) {
        print_help_message(rank);
    }
}

void print_help_message(int rank)
{
    if (rank) {
        return;
    }
    if (accel_enabled && (options.bench == PT2PT)) {
        fprintf(stdout, "Usage: %s [options] [SRC DST]\n\n", benchmark_name);
        fprintf(
            stdout,
            "SRC and DST are buffer types for the source and destination\n");
        fprintf(
            stdout,
            "SRC and DST may be `D', `H', 'MD'or 'MH' which specifies whether\n"
            "the buffer is allocated on the accelerator device memory, host\n"
            "memory or using CUDA Unified Memory allocated on device or host "
            "respectively for each mpi rank\n\n");
    } else {
        fprintf(stdout, "Usage: %s [options]\n", benchmark_name);
        fprintf(stdout, "Options:\n");
    }
    print_help_message_common();
}

void print_help_message_common()
{
    int str_itr = 0, long_itr = 0, num_long_options = 0;
    char *temp_long_opt = NULL;
    struct option long_options[OMB_LONG_OPTIONS_ARRAY_SIZE] =
        OMBOP_LONG_OPTIONS_ALL;
    struct help_msg_t help_msg[OMB_LONG_OPTIONS_ARRAY_SIZE] = OMBOP_HELP_MSG;
    char *help_str = NULL;
    num_long_options = sizeof(long_options) / sizeof(struct option);
    for (str_itr = 0; str_itr < strlen(options.optstring); str_itr++) {
        if (!isalpha(options.optstring[str_itr])) {
            continue;
        }
        for (long_itr = 0; long_itr < num_long_options; long_itr++) {
            if (options.optstring[str_itr] == long_options[long_itr].val) {
                if (options.optstring[str_itr] != help_msg[long_itr].opt) {
                    OMB_ERROR_EXIT(
                        "OMBOP_LONG_OPTIONS_ALL and OMBOP_HELP_MSG mismatch.");
                }
                temp_long_opt =
                    malloc(strlen(long_options[long_itr].name) * sizeof(char));
                OMB_CHECK_NULL_AND_EXIT(temp_long_opt,
                                        "Unable to allocate memory");
                sprintf(temp_long_opt, "%s%s", "--",
                        long_options[long_itr].name);
                fprintf(stdout, "  -%c, --%-20s", options.optstring[str_itr],
                        long_options[long_itr].name);
                help_str = strtok(help_msg[long_itr].msg, "~~");
                fprintf(stdout, "%s\n", help_str);
                help_str = strtok(NULL, "~~");
                while (help_str != NULL) {
                    fprintf(stdout, "%28s%s\n", "", help_str);
                    help_str = strtok(NULL, "~~");
                }

                fflush(stdout);
                free(temp_long_opt);
            }
        }
    }
}

void print_help_message_get_acc_lat(int rank)
{
    if (rank) {
        return;
    }

    if (bad_usage.optarg) {
        fprintf(stderr, "%s [-%c %s]\n\n", bad_usage.message,
                (char)bad_usage.opt, bad_usage.optarg);
    } else {
        fprintf(stderr, "%s [-%c]\n\n", bad_usage.message, (char)bad_usage.opt);
    }
    fflush(stderr);

    fprintf(stdout, "Usage: ./osu_get_acc_latency -w <win_option>  -s < "
                    "sync_option> [-x ITER] [-i ITER]\n");
    print_help_message_common();
}

void print_header_one_sided(int rank, enum WINDOW win, enum SYNC sync,
                            MPI_Datatype dtype)
{
    char dtype_name_str[OMB_DATATYPE_STR_MAX_LEN];
    int dtype_name_size = 0;
    int itr = 0;

    if (rank == 0) {
        switch (options.accel) {
            case CUDA:
                printf(benchmark_header, "-CUDA");
                break;
            case OPENACC:
                printf(benchmark_header, "-OPENACC");
                break;
            case ROCM:
                printf(benchmark_header, "-ROCM");
                break;
            case SYCL:
                printf(benchmark_header, "-SYCL");
                break;
            default:
                printf(benchmark_header, "");
                break;
        }
        fprintf(stdout, "# Window creation: %s\n", win_info[win]);
        fprintf(stdout, "# Synchronization: %s\n", sync_info[sync]);
        MPI_CHECK(MPI_Type_get_name(dtype, dtype_name_str, &dtype_name_size));
        printf("# Datatype: %s.\n", dtype_name_str);

        switch (options.accel) {
            case CUDA:
            case OPENACC:
            case ROCM:
            case SYCL:
                fprintf(stdout,
                        "# Rank 0 Memory on %s and Rank 1 Memory on %s\n",
                        'M' == options.src ?
                            "MANAGED (M)" :
                            ('D' == options.src ? "DEVICE (D)" : "HOST (H)"),
                        'M' == options.dst ?
                            "MANAGED (M)" :
                            ('D' == options.dst ? "DEVICE (D)" : "HOST (H)"));
            default:
                if (options.subtype == BW) {
                    fprintf(stdout, "%-*s%*s", 10, "# Size", FIELD_WIDTH,
                            "Bandwidth (MB/s)");
                } else {
                    fprintf(stdout, "%-*s%*s", 10, "# Size", FIELD_WIDTH,
                            "Latency (us)");
                }
                if (options.validate) {
                    fprintf(stdout, "%*s", FIELD_WIDTH, "Validation");
                }
                if (options.omb_tail_lat) {
                    itr = 0;
                    while (itr < OMB_STAT_MAX_NUM &&
                           -1 != options.omb_stat_percentiles[itr]) {
                        if (BW == options.subtype) {
                            fprintf(stdout, "%*sP%d Tail BW(MB/s)",
                                    FIELD_WIDTH - strlen("Px Tail BW(MB/s)") -
                                        (options.omb_stat_percentiles[itr] > 9),
                                    "", options.omb_stat_percentiles[itr]);
                        } else {
                            fprintf(stdout, "%*sP%d Tail Lat(us)",
                                    FIELD_WIDTH - strlen("Px Tail Lat(us)") -
                                        (options.omb_stat_percentiles[itr] > 9),
                                    "", options.omb_stat_percentiles[itr]);
                        }
                        itr++;
                    }
                }
                fprintf(stdout, "\n");
                fflush(stdout);
        }
    }
}

void print_version_message(int rank)
{
    if (rank) {
        return;
    }

    switch (options.accel) {
        case CUDA:
            printf(benchmark_header, "-CUDA");
            break;
        case OPENACC:
            printf(benchmark_header, "-OPENACC");
            break;
        case MANAGED:
            printf(benchmark_header, "-CUDA MANAGED");
            break;
        case ROCM:
            printf(benchmark_header, "-ROCM");
            break;
        case SYCL:
            printf(benchmark_header, "-SYCL");
            break;
        default:
            printf(benchmark_header, "");
            break;
    }

    fflush(stdout);
}

void print_preamble_nbc(int rank)
{
    if (rank) {
        return;
    }

    fprintf(stdout, "\n");

    switch (options.accel) {
        case CUDA:
            printf(benchmark_header, "-CUDA");
            break;
        case OPENACC:
            printf(benchmark_header, "-OPENACC");
            break;
        case MANAGED:
            printf(benchmark_header, "-MANAGED");
            break;
        case ROCM:
            printf(benchmark_header, "-ROCM");
            break;
        case SYCL:
            printf(benchmark_header, "-SYCL");
            break;
        default:
            printf(benchmark_header, "");
            break;
    }

    fprintf(stdout,
            "# Overall = Coll. Init + Compute + MPI_Test + MPI_Wait\n\n");

    fflush(stdout);
}

void print_only_header_nbc(int rank)
{
    int itr = 0;

    if (rank) {
        return;
    }
    if (options.show_size) {
        fprintf(stdout, "%-*s", 10, "# Size");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Overall(us)");
    } else {
        fprintf(stdout, "%s", "# Overall(us)");
    }

    if (options.show_full) {
        fprintf(stdout, "%*s", FIELD_WIDTH, "Compute(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Coll. Init(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "MPI_Test(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "MPI_Wait(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Pure Comm.(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Min Comm.(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Max Comm.(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Overlap(%)");

    } else {
        fprintf(stdout, "%*s", FIELD_WIDTH, "Compute(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Pure Comm.(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Overlap(%)");
    }

    if (options.validate) {
        fprintf(stdout, "%*s", FIELD_WIDTH, "Validation");
    }
    if (options.omb_tail_lat) {
        itr = 0;
        while (itr < OMB_STAT_MAX_NUM &&
               -1 != options.omb_stat_percentiles[itr]) {
            fprintf(stdout, "%*sP%d Tail Lat(us)",
                    FIELD_WIDTH - strlen("Px Tail Lat(us)") -
                        (options.omb_stat_percentiles[itr] > 9),
                    "", options.omb_stat_percentiles[itr]);
            itr++;
        }
    }

    if (options.omb_enable_ddt) {
        fprintf(stdout, "%*s", FIELD_WIDTH, "Transmit Size");
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}
void display_nbc_params()
{
    if (options.show_full) {
        fprintf(stdout, "%*s", FIELD_WIDTH, "Compute(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Coll. Init(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "MPI_Test(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "MPI_Wait(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Pure Comm.(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Min Comm.(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Max Comm.(us)");
        fprintf(stdout, "%*s\n", FIELD_WIDTH, "Overlap(%)");

    } else {
        fprintf(stdout, "%*s", FIELD_WIDTH, "Compute(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Pure Comm.(us)");
        fprintf(stdout, "%*s\n", FIELD_WIDTH, "Overlap(%)");
    }
}

void print_preamble(int rank)
{
    if (rank) {
        return;
    }

    fprintf(stdout, "\n");

    switch (options.accel) {
        case CUDA:
            printf(benchmark_header, "-CUDA");
            break;
        case OPENACC:
            printf(benchmark_header, "-OPENACC");
            break;
        case ROCM:
            printf(benchmark_header, "-ROCM");
            break;
        case SYCL:
            printf(benchmark_header, "-SYCL");
            break;
        default:
            printf(benchmark_header, "");
            break;
    }
    fflush(stdout);
}

void print_only_header(int rank)
{
    int itr = 0;

    if (rank) {
        return;
    }
    if (options.show_size) {
        fprintf(stdout, "%-*s", 10, "# Size");
        if (BW == options.subtype && MBW_MR != options.bench) {
            fprintf(stdout, "%*s", FIELD_WIDTH, "Bandwidth (MB/s)");
        } else if (MBW_MR == options.bench) {
            if (options.print_rate) {
                fprintf(stdout, "%*s%*s", FIELD_WIDTH, "MB/s", FIELD_WIDTH,
                        "Messages/s");
            } else {
                fprintf(stdout, "%*s", FIELD_WIDTH, "MB/s");
            }
        } else {
            fprintf(stdout, "%*s", FIELD_WIDTH, "Avg Latency(us)");
        }
    } else {
        if (BW == options.subtype && MBW_MR != options.bench) {
            fprintf(stdout, "# Bandwidth (MB/s)");
        } else if (MBW_MR == options.bench) {
            if (options.print_rate) {
                fprintf(stdout, "# %*s%*s", FIELD_WIDTH, "MB/s", FIELD_WIDTH,
                        "Messages/s");
            } else {
                fprintf(stdout, "# %*s", FIELD_WIDTH, "MB/s");
            }
        } else {
            fprintf(stdout, "# Avg Latency(us)");
        }
    }

    if (options.show_full) {
        fprintf(stdout, "%*s", FIELD_WIDTH, "Min Latency(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Max Latency(us)");
        fprintf(stdout, "%*s", 12, "Iterations");
    }

    if (options.validate)
        fprintf(stdout, "%*s", FIELD_WIDTH, "Validation");
    if (options.omb_tail_lat) {
        itr = 0;
        while (itr < OMB_STAT_MAX_NUM &&
               -1 != options.omb_stat_percentiles[itr]) {
            if (BW == options.subtype) {
                fprintf(stdout, "%*sP%d Tail BW(MB/s)",
                        FIELD_WIDTH - strlen("Px Tail BW(MB/s)") -
                            (options.omb_stat_percentiles[itr] > 9),
                        "", options.omb_stat_percentiles[itr]);
            } else {
                fprintf(stdout, "%*sP%d Tail Lat(us)",
                        FIELD_WIDTH - strlen("Px Tail Lat(us)") -
                            (options.omb_stat_percentiles[itr] > 9),
                        "", options.omb_stat_percentiles[itr]);
            }
            itr++;
        }
    }
    if (options.omb_enable_ddt) {
        fprintf(stdout, "%*s", FIELD_WIDTH, "Transmit Size");
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}

omb_mpi_init_data omb_mpi_init(int *argc, char ***argv)
{
    int cvar_index, count, err = MPI_SUCCESS;
    int threadsupport;
    omb_mpi_init_data init_struct;
#ifdef _ENABLE_MPI4_
    init_struct.omb_shandle = MPI_SESSION_NULL;
    MPI_Group wgroup = MPI_GROUP_NULL;
#endif
    init_struct.omb_comm = MPI_COMM_NULL;

    if (1 == options.omb_enable_session) {
#ifdef _ENABLE_MPI4_
        {
            MPI_CHECK(MPI_Session_init(MPI_INFO_NULL, MPI_ERRORS_RETURN,
                                       &init_struct.omb_shandle));
            MPI_CHECK(MPI_Group_from_session_pset(
                init_struct.omb_shandle, OMB_MPI_SESSION_PSET_NAME, &wgroup));
            MPI_CHECK(MPI_Comm_create_from_group(
                wgroup, OMB_MPI_SESSION_GROUP_NAME, MPI_INFO_NULL,
                MPI_ERRORS_RETURN, &init_struct.omb_comm));
            MPI_CHECK(MPI_Group_free(&wgroup));
        }
#endif
    } else {
        MPI_CHECK(MPI_Init(argc, argv));
        init_struct.omb_comm = MPI_COMM_WORLD;
    }

    MPI_CHECK(MPI_T_init_thread(MPI_THREAD_SINGLE, &threadsupport));
    err = MPI_T_cvar_get_index("MPIR_CVAR_ENABLE_DYNAMIC_TUNING", &cvar_index);

    if (MPI_SUCCESS == err) {
        MPI_CHECK(MPI_T_cvar_handle_alloc(cvar_index, NULL,
                                          &dynamic_tuning_cvar, &count));
        assert(NULL != dynamic_tuning_cvar);
    }

    return init_struct;
}

void omb_mpi_finalize(omb_mpi_init_data mpi_init)
{
    if (dynamic_tuning_cvar) {
        MPI_CHECK(MPI_T_cvar_handle_free(&dynamic_tuning_cvar));
    }
    MPI_T_finalize();
    if (1 == options.omb_enable_session) {
#ifdef _ENABLE_MPI4_
        MPI_CHECK(MPI_Comm_free(&mpi_init.omb_comm));
        MPI_CHECK(MPI_Session_finalize(&mpi_init.omb_shandle));
#endif
    } else {
        MPI_CHECK(MPI_Finalize());
    }
}

int omb_ascending_cmp_double(const void *a, const void *b)
{
    double v1 = *(const double *)a;
    double v2 = *(const double *)b;
    if (v1 < v2) {
        return -1;
    } else if (v1 > v2) {
        return 1;
    } else {
        return 0;
    }
}

struct omb_stat_t omb_get_stats(double *lat_arr)
{
    int rank = 0, comm_size = 0;
    double avg_lat_arr[options.iterations];
    int i = 0;
    struct omb_stat_t omb_stats;

    if (!options.omb_tail_lat) {
        return omb_stats;
    }
    OMB_CHECK_NULL_AND_EXIT(lat_arr, "Passed array is NULL");
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &comm_size));
    MPI_CHECK(MPI_Reduce(lat_arr, avg_lat_arr, options.iterations, MPI_DOUBLE,
                         MPI_SUM, 0, MPI_COMM_WORLD));
    return omb_calculate_tail_lat(avg_lat_arr, rank, comm_size);
}

struct omb_stat_t omb_calculate_tail_lat(double *avg_lat_arr, int rank,
                                         int comm_size)
{
    struct omb_stat_t omb_stats;
    int itr = 0;

    if (0 != rank || !options.omb_tail_lat) {
        return omb_stats;
    }
    qsort(avg_lat_arr, options.iterations, sizeof(double),
          omb_ascending_cmp_double);
    itr = 0;
    while (itr < OMB_STAT_MAX_NUM && -1 != options.omb_stat_percentiles[itr]) {
        omb_stats.res_arr[itr] =
            avg_lat_arr[(int)round(options.iterations *
                                   options.omb_stat_percentiles[itr] / 100) -
                        1] /
            comm_size;
        itr++;
    }
    return omb_stats;
}

double calculate_and_print_stats(int rank, int size, int numprocs, double timer,
                                 double latency, double test_time,
                                 double cpu_time, double wait_time,
                                 double init_time, int errors,
                                 struct omb_stat_t omb_stats)
{
    double test_total = (test_time * 1e6) / options.iterations;
    double tcomp_total = (cpu_time * 1e6) / options.iterations;
    double overall_time = (timer * 1e6) / options.iterations;
    double wait_total = (wait_time * 1e6) / options.iterations;
    double init_total = (init_time * 1e6) / options.iterations;
    double avg_comm_time = latency;
    double min_comm_time = latency, max_comm_time = latency;

    if (rank != 0) {
        MPI_CHECK(MPI_Reduce(&test_total, &test_total, 1, MPI_DOUBLE, MPI_SUM,
                             0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(&avg_comm_time, &avg_comm_time, 1, MPI_DOUBLE,
                             MPI_SUM, 0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(&overall_time, &overall_time, 1, MPI_DOUBLE,
                             MPI_SUM, 0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(&tcomp_total, &tcomp_total, 1, MPI_DOUBLE, MPI_SUM,
                             0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(&wait_total, &wait_total, 1, MPI_DOUBLE, MPI_SUM,
                             0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(&init_total, &init_total, 1, MPI_DOUBLE, MPI_SUM,
                             0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(&latency, &min_comm_time, 1, MPI_DOUBLE, MPI_MIN,
                             0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(&latency, &max_comm_time, 1, MPI_DOUBLE, MPI_MAX,
                             0, MPI_COMM_WORLD));
    } else {
        MPI_CHECK(MPI_Reduce(MPI_IN_PLACE, &test_total, 1, MPI_DOUBLE, MPI_SUM,
                             0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(MPI_IN_PLACE, &avg_comm_time, 1, MPI_DOUBLE,
                             MPI_SUM, 0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(MPI_IN_PLACE, &overall_time, 1, MPI_DOUBLE,
                             MPI_SUM, 0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(MPI_IN_PLACE, &tcomp_total, 1, MPI_DOUBLE, MPI_SUM,
                             0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(MPI_IN_PLACE, &wait_total, 1, MPI_DOUBLE, MPI_SUM,
                             0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(MPI_IN_PLACE, &init_total, 1, MPI_DOUBLE, MPI_SUM,
                             0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(MPI_IN_PLACE, &min_comm_time, 1, MPI_DOUBLE,
                             MPI_MIN, 0, MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(MPI_IN_PLACE, &max_comm_time, 1, MPI_DOUBLE,
                             MPI_MAX, 0, MPI_COMM_WORLD));
    }

    MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));

    /* Overall Time (Overlapped) */
    overall_time = overall_time / numprocs;
    /* Computation Time */
    tcomp_total = tcomp_total / numprocs;
    /* Time taken by MPI_Test calls */
    test_total = test_total / numprocs;
    /* Pure Communication Time */
    avg_comm_time = avg_comm_time / numprocs;
    /* Time for MPI_Wait() call */
    wait_total = wait_total / numprocs;
    /* Time for the NBC call */
    init_total = init_total / numprocs;

    print_stats_nbc(rank, size, overall_time, tcomp_total, avg_comm_time,
                    min_comm_time, max_comm_time, wait_total, init_total,
                    test_total, errors, omb_stats);
    return overall_time;
}

void print_stats_nbc(int rank, int size, double overall_time, double cpu_time,
                     double avg_comm_time, double min_comm_time,
                     double max_comm_time, double wait_time, double init_time,
                     double test_time, int errors, struct omb_stat_t omb_stats)
{
    int itr = 0;

    if (rank) {
        return;
    }

    double overlap;

    /* Note : cpu_time received in this function includes time for
     *      dummy compute as well as test calls so we will subtract
     *      the test_time for overlap calculation as test is an
     *      overhead
     */

    overlap = MAX(
        0, 100 - (((overall_time - (cpu_time - test_time)) / avg_comm_time) *
                  100));

    if (options.show_size) {
        fprintf(stdout, "%-*d", 10, size);
        fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION, overall_time);
    } else {
        fprintf(stdout, "%*.*f", 13, FLOAT_PRECISION, overall_time);
    }

    if (options.show_full) {
        fprintf(stdout, "%*.*f%*.*f%*.*f%*.*f%*.*f%*.*f%*.*f%*.*f", FIELD_WIDTH,
                FLOAT_PRECISION, (cpu_time - test_time), FIELD_WIDTH,
                FLOAT_PRECISION, init_time, FIELD_WIDTH, FLOAT_PRECISION,
                test_time, FIELD_WIDTH, FLOAT_PRECISION, wait_time, FIELD_WIDTH,
                FLOAT_PRECISION, avg_comm_time, FIELD_WIDTH, FLOAT_PRECISION,
                min_comm_time, FIELD_WIDTH, FLOAT_PRECISION, max_comm_time,
                FIELD_WIDTH, FLOAT_PRECISION, overlap);
    } else {
        fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION,
                (cpu_time - test_time));
        fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION, avg_comm_time);
        fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION, overlap);
    }

    if (options.validate) {
        fprintf(stdout, "%*s", FIELD_WIDTH, VALIDATION_STATUS(errors));
    }
    if (options.omb_tail_lat) {
        itr = 0;
        while (itr < OMB_STAT_MAX_NUM &&
               -1 != options.omb_stat_percentiles[itr]) {
            fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION,
                    omb_stats.res_arr[itr]);
            itr++;
        }
    }
    if (!options.omb_enable_ddt) {
        fprintf(stdout, "\n");
    }

    fflush(stdout);
}

void print_stats(int rank, int size, double avg_time, double min_time,
                 double max_time, struct omb_stat_t omb_stats)
{
    int itr = 0;

    if (rank) {
        return;
    }

    if (options.show_size) {
        fprintf(stdout, "%-*d", 10, size);
        fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION, avg_time);
    } else {
        fprintf(stdout, "%*.*f", 17, FLOAT_PRECISION, avg_time);
    }

    if (options.show_full) {
        fprintf(stdout, "%*.*f%*.*f%*lu", FIELD_WIDTH, FLOAT_PRECISION,
                min_time, FIELD_WIDTH, FLOAT_PRECISION, max_time, 12,
                options.iterations);
    }
    if (options.omb_tail_lat) {
        itr = 0;
        while (itr < OMB_STAT_MAX_NUM &&
               -1 != options.omb_stat_percentiles[itr]) {
            fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION,
                    omb_stats.res_arr[itr]);
            itr++;
        }
    }
    if (!options.omb_enable_ddt) {
        fprintf(stdout, "\n");
    }
    fflush(stdout);
}

void print_stats_validate(int rank, int size, double avg_time, double min_time,
                          double max_time, int errors,
                          struct omb_stat_t omb_stats)
{
    int itr = 0;

    if (rank) {
        return;
    }

    if (options.show_size) {
        fprintf(stdout, "%-*d", 10, size);
        fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION, avg_time);
    } else {
        fprintf(stdout, "%*.*f", 17, FLOAT_PRECISION, avg_time);
    }

    if (options.show_full) {
        fprintf(stdout, "%*.*f%*.*f%*lu", FIELD_WIDTH, FLOAT_PRECISION,
                min_time, FIELD_WIDTH, FLOAT_PRECISION, max_time, 12,
                options.iterations);
    }
    fprintf(stdout, "%*s", FIELD_WIDTH, VALIDATION_STATUS(errors));
    if (options.omb_tail_lat) {
        itr = 0;
        while (itr < OMB_STAT_MAX_NUM &&
               -1 != options.omb_stat_percentiles[itr]) {
            fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION,
                    omb_stats.res_arr[itr]);
            itr++;
        }
    }
    if (!options.omb_enable_ddt) {
        fprintf(stdout, "\n");
    }
    fflush(stdout);
}

int omb_get_root_rank(int itr, size_t comm_size)
{
    if (OMB_ROOT_ROTATE_VAL != options.omb_root_rank) {
        if (options.omb_root_rank >= comm_size) {
            OMB_ERROR_EXIT(
                "Root rank(\'-k\') cannot be more than number of processes");
        }
        return options.omb_root_rank;
    }
    return itr % comm_size;
}

void omb_populate_mpi_type_list(MPI_Datatype *mpi_type_list)
{
    int i = 0;
    for (i = 0; i < OMB_NUM_DATATYPES; i++) {
        switch (options.omb_dtype_list[i]) {
            case OMB_DTYPE_NULL:
                mpi_type_list[i] = MPI_CHAR;
                break;
            case OMB_CHAR:
                mpi_type_list[i] = MPI_CHAR;
                break;
            case OMB_INT:
                mpi_type_list[i] = MPI_INT;
                break;
            case OMB_FLOAT:
                mpi_type_list[i] = MPI_FLOAT;
                break;
            case OMB_DOUBLE:
                mpi_type_list[i] = MPI_DOUBLE;
                break;
            default:
                OMB_ERROR_EXIT("Unknown data type");
                break;
        }
    }
}

void omb_ddt_append_stats(size_t omb_ddt_transmit_size)
{
    int rank;
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (rank) {
        return;
    }
    if (options.omb_enable_ddt) {
        fprintf(stdout, "%*zu\n", FIELD_WIDTH, omb_ddt_transmit_size);
    }
}

void set_buffer_pt2pt(void *buffer, int rank, enum accel_type type, int data,
                      size_t size)
{
    char buf_type = 'H';

    OMB_CHECK_NULL_AND_EXIT(buffer, "NULL passed for buffer");
    if (options.bench == MBW_MR) {
        buf_type = (rank < options.pairs) ? options.src : options.dst;
    } else {
        buf_type = (rank == 0) ? options.src : options.dst;
    }

    switch (buf_type) {
        case 'H':
            memset(buffer, data, size);
            break;
        case 'D':
        case 'M':
#ifdef _ENABLE_OPENACC_
            if (type == OPENACC) {
                size_t i;
                char *p = (char *)buffer;
#pragma acc parallel loop deviceptr(p)
                for (i = 0; i < size; i++) {
                    p[i] = data;
                }
                break;
            } else
#endif
#ifdef _ENABLE_CUDA_
            {
                CUDA_CHECK(cudaMemset(buffer, data, size));
            }
#endif
#ifdef _ENABLE_ROCM_
            {
                ROCM_CHECK(hipMemset(buffer, data, size));
            }
#endif
#ifdef _ENABLE_SYCL_
            {
                syclMemset(buffer, data, size);
            }
#endif
            break;
    }
}

void set_buffer(void *buffer, enum accel_type type, int data, size_t size)
{
#ifdef _ENABLE_OPENACC_
    size_t i;
    char *p = (char *)buffer;
#endif
    switch (type) {
        case NONE:
            memset(buffer, data, size);
            break;
        case CUDA:
        case MANAGED:
#ifdef _ENABLE_CUDA_
            CUDA_CHECK(cudaMemset(buffer, data, size));
#endif
            break;
        case OPENACC:
#ifdef _ENABLE_OPENACC_
#pragma acc parallel loop deviceptr(p)
            for (i = 0; i < size; i++) {
                p[i] = data;
            }
#endif
            break;
        case ROCM:
#ifdef _ENABLE_ROCM_
            ROCM_CHECK(hipMemset(buffer, data, size));
#endif
        case SYCL:
#ifdef _ENABLE_SYCL_
            syclMemset(buffer, data, size);
#endif
            break;
        default:
            break;
    }
}

void set_buffer_validation(void *s_buf, void *r_buf, size_t size,
                           enum accel_type type, int iter, MPI_Datatype dtype,
                           struct omb_buffer_sizes_t omb_buffer_sizes)
{
    void *temp_r_buffer = NULL;
    void *temp_s_buffer = NULL;
    int rank = 0;
    char buf_type = 'H';
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    switch (options.bench) {
        case PT2PT:
        case MBW_MR: {
            int num_elements = omb_get_num_elements(size, dtype);
            int val = 0;
            temp_r_buffer = malloc(size);
            temp_s_buffer = malloc(size);
            register int i;
            for (i = 0; i < num_elements; i++) {
                val = (CHAR_VALIDATION_MULTIPLIER * (i + 1) + size + iter);
                omb_assign_to_type(temp_s_buffer, i, val, dtype);
            }
            for (i = 0; i < num_elements; i++) {
                omb_assign_to_type(temp_r_buffer, i, 0, dtype);
            }
            if (options.bench == MBW_MR) {
                buf_type = (rank < options.pairs) ? options.src : options.dst;
            } else {
                buf_type = (rank == 0) ? options.src : options.dst;
            }
            switch (buf_type) {
                case 'H':
                    memcpy((void *)s_buf, (void *)temp_s_buffer, size);
                    memcpy((void *)r_buf, (void *)temp_r_buffer, size);
                    break;
                case 'D':
                case 'M':
#ifdef _ENABLE_OPENACC_
                    if (type == OPENACC) {
                        size_t i;
                        char *p = (char *)s_buf;
#pragma acc parallel loop deviceptr(p)
                        for (i = 0; i < size; i++) {
                            p[i] = temp_char_s_buffer[i];
                        }
                        p = (char *)r_buf;
#pragma acc parallel loop deviceptr(p)
                        for (i = 0; i < size; i++) {
                            p[i] = temp_char_r_buffer[i];
                        }
                        break;
                    } else
#endif
#ifdef _ENABLE_CUDA_
                    {
                        CUDA_CHECK(cudaMemcpy((void *)s_buf,
                                              (void *)temp_s_buffer, size,
                                              cudaMemcpyHostToDevice));
                        CUDA_CHECK(cudaMemcpy((void *)r_buf,
                                              (void *)temp_r_buffer, size,
                                              cudaMemcpyHostToDevice));
                        CUDA_CHECK(cudaDeviceSynchronize());
                    }
#endif
#ifdef _ENABLE_ROCM_
                    {
                        ROCM_CHECK(hipMemcpy((void *)s_buf,
                                             (void *)temp_s_buffer, size,
                                             hipMemcpyHostToDevice));
                        ROCM_CHECK(hipMemcpy((void *)r_buf,
                                             (void *)temp_r_buffer, size,
                                             hipMemcpyHostToDevice));
                        ROCM_CHECK(hipDeviceSynchronize());
                    }
#endif
#ifdef _ENABLE_SYCL_
                    {
                        syclMemcpy((void *)s_buf, (void *)temp_s_buffer, size);
                        syclMemcpy((void *)r_buf, (void *)temp_r_buffer, size);
                        syclDeviceSynchronize();
                    }
#endif
                    break;
            }
            free(temp_s_buffer);
            free(temp_r_buffer);
        } break;
        case COLLECTIVE: {
            switch (options.subtype) {
                case ALLTOALL:
                case ALLTOALL_P:
                case NBC_ALLTOALL: {
                    int rank, numprocs;
                    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    if (0 == options.omb_enable_mpi_in_place) {
                        set_buffer_dtype(s_buf, 1, size, rank, numprocs, type,
                                         iter, dtype,
                                         omb_buffer_sizes.sendbuf_size);
                        set_buffer_dtype(r_buf, 0, size, rank, numprocs, type,
                                         iter, dtype,
                                         omb_buffer_sizes.recvbuf_size);
                    } else {
                        set_buffer_dtype(r_buf, 1, size, rank, numprocs, type,
                                         iter, dtype,
                                         omb_buffer_sizes.recvbuf_size);
                    }
                } break;
                case GATHER:
                case GATHER_P:
                case NBC_GATHER: {
                    int rank, numprocs;
                    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    if (0 == options.omb_enable_mpi_in_place) {
                        set_buffer_dtype(s_buf, 1, size, rank * numprocs, 1,
                                         type, iter, dtype,
                                         omb_buffer_sizes.sendbuf_size);
                        set_buffer_dtype(r_buf, 0, size, rank, numprocs, type,
                                         iter, dtype,
                                         omb_buffer_sizes.recvbuf_size);
                    } else {
                        set_buffer_dtype(s_buf, 1, size, rank * numprocs, 1,
                                         type, iter, dtype,
                                         omb_buffer_sizes.sendbuf_size);
                        set_buffer_dtype(r_buf, 0, size, rank * numprocs, 1,
                                         type, iter, dtype,
                                         omb_buffer_sizes.recvbuf_size);
                    }
                } break;
                case ALL_GATHER:
                case ALL_GATHER_P:
                case NBC_ALL_GATHER: {
                    int rank, numprocs;
                    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    if (0 == options.omb_enable_mpi_in_place) {
                        set_buffer_dtype(s_buf, 1, size, rank * numprocs, 1,
                                         type, iter, dtype,
                                         omb_buffer_sizes.sendbuf_size);
                        if (0 == rank) {
                            set_buffer_dtype(r_buf, 0, size, rank, numprocs,
                                             type, iter, dtype,
                                             omb_buffer_sizes.recvbuf_size);
                        }
                    } else {
                        set_buffer_dtype(r_buf, 1, size, rank * numprocs, 1,
                                         type, iter, dtype,
                                         omb_buffer_sizes.recvbuf_size);
                    }
                } break;
                case REDUCE:
                case ALL_REDUCE:
                case REDUCE_P:
                case ALL_REDUCE_P:
                case NBC_REDUCE:
                case NBC_ALL_REDUCE: {
                    int rank, numprocs;
                    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    if (0 == options.omb_enable_mpi_in_place) {
                        set_buffer_dtype_reduce(s_buf, 1, size, iter,
                                                options.accel, dtype);
                        set_buffer_dtype_reduce(r_buf, 0, size, iter,
                                                options.accel, dtype);
                    } else {
                        set_buffer_dtype_reduce(r_buf, 1, size, iter,
                                                options.accel, dtype);
                    }
                    break;
                }
                case SCATTER:
                case SCATTER_P:
                case NBC_SCATTER: {
                    int rank, numprocs;
                    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    if (0 == options.omb_enable_mpi_in_place) {
                        set_buffer_dtype(s_buf, 1, size, 0, numprocs, type,
                                         iter, dtype,
                                         omb_buffer_sizes.sendbuf_size);
                        set_buffer_dtype(r_buf, 0, size, rank * numprocs, 1,
                                         type, iter, dtype,
                                         omb_buffer_sizes.recvbuf_size);
                    } else {
                        set_buffer_dtype(r_buf, 1, size, 0, numprocs, type,
                                         iter, dtype,
                                         omb_buffer_sizes.recvbuf_size);
                    }
                } break;
                case REDUCE_SCATTER:
                case REDUCE_SCATTER_P:
                case NBC_REDUCE_SCATTER: {
                    int rank, numprocs;
                    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    if (0 == options.omb_enable_mpi_in_place) {
                        set_buffer_dtype_reduce(s_buf, 1, size, iter,
                                                options.accel, dtype);
                        set_buffer_dtype_reduce(r_buf, 0, size / numprocs + 1,
                                                iter, options.accel, dtype);
                    } else {
                        set_buffer_dtype_reduce(r_buf, 1, size, iter,
                                                options.accel, dtype);
                    }
                } break;
                case BCAST:
                case BCAST_P:
                case NBC_BCAST: {
                    int rank, numprocs;
                    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    if (0 == rank) {
                        set_buffer_dtype(s_buf, 1, size, 1, 1, type, iter,
                                         dtype, omb_buffer_sizes.sendbuf_size);
                    } else {
                        set_buffer_dtype(s_buf, 0, size, 1, 1, type, iter,
                                         dtype, omb_buffer_sizes.recvbuf_size);
                    }
                } break;
                default:
                    break;
            }
        } break;
        default:
            break;
    }
}

void set_buffer_dtype_reduce(void *buffer, int is_send_buf, size_t size,
                             int iter, enum accel_type type, MPI_Datatype dtype)
{
    if (NULL == buffer) {
        return;
    }

    int i = 0, j = 0;
    int val = 0;
    int num_elements = omb_get_num_elements(size, dtype);
    float *temp_buffer = malloc(size);
    if (is_send_buf) {
        for (i = 0; i < num_elements; i++) {
            j = (i % 100);
            val = (j + 1) * (iter + 1);
            omb_assign_to_type(temp_buffer, i, val, dtype);
        }
    } else {
        for (i = 0; i < num_elements; i++) {
            omb_assign_to_type(temp_buffer, i, 0, dtype);
        }
    }
    switch (type) {
        case NONE:
            memcpy((void *)buffer, (void *)temp_buffer, size);
            break;
        case CUDA:
        case MANAGED:
#ifdef _ENABLE_CUDA_
            CUDA_CHECK(cudaMemcpy((void *)buffer, (void *)temp_buffer, size,
                                  cudaMemcpyHostToDevice));
            CUDA_CHECK(cudaDeviceSynchronize());
#endif
            break;
        case SYCL:
#ifdef _ENABLE_SYCL_
            syclMemcpy((void *)buffer, (void *)temp_buffer, size);
            syclDeviceSynchronize();
#endif
        case ROCM:
#ifdef _ENABLE_ROCM_
            ROCM_CHECK(hipMemcpy((void *)buffer, (void *)temp_buffer, size,
                                 hipMemcpyHostToDevice));
            ROCM_CHECK(hipDeviceSynchronize());
#endif
            break;
        default:
            break;
    }
    free(temp_buffer);
}

void omb_assign_to_type(void *buf, int pos, int val, MPI_Datatype dtype)
{
    if (MPI_CHAR == dtype || MPI_SIGNED_CHAR == dtype) {
        ((char *)buf)[pos] =
            (CHAR_VALIDATION_MULTIPLIER * (char)(val)) % CHAR_RANGE;
    } else if (MPI_INT == dtype) {
        ((int *)buf)[pos] = (int)val * INT_VALIDATION_MULTIPLIER;
    } else if (MPI_FLOAT == dtype) {
        ((float *)buf)[pos] = (float)val * FLOAT_VALIDATION_MULTIPLIER;
    } else if (MPI_DOUBLE == dtype) {
        ((double *)buf)[pos] = (double)val * DOUBLE_VALIDATION_MULTIPLIER;
    } else {
        OMB_ERROR_EXIT("Invalid data type passed");
    }
}

int omb_get_num_elements(size_t size, MPI_Datatype dtype)
{
    if (MPI_CHAR == dtype || MPI_SIGNED_CHAR == dtype) {
        return size / sizeof(ATOM_CTYPE_FOR_DMPI_CHAR);
    } else if (MPI_FLOAT == dtype) {
        return size / sizeof(ATOM_CTYPE_FOR_DMPI_FLOAT);
    } else if (MPI_DOUBLE == dtype) {
        return size / sizeof(ATOM_CTYPE_FOR_DMPI_DOUBLE);
    } else if (MPI_INT == dtype) {
        return size / sizeof(ATOM_CTYPE_FOR_DMPI_INT);
    } else {
        OMB_ERROR_EXIT("Invalid datatype passed");
    }
    return -1;
}

void set_buffer_dtype(void *buffer, int is_send_buf, size_t size, int rank,
                      int num_procs, enum accel_type type, int iter,
                      MPI_Datatype dtype, size_t buffer_size)
{
    if (NULL == buffer) {
        return;
    }

    int num_elements = omb_get_num_elements(size, dtype);
    int i, j;
    int val = 0;
    void *temp_buffer = NULL;
    int id = 0, com_size = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);
    temp_buffer = malloc(buffer_size);
    OMB_CHECK_NULL_AND_EXIT(temp_buffer, "Unable to allocate memory");

    if (is_send_buf || GATHER == options.subtype ||
        GATHER_P == options.subtype || NBC_GATHER == options.subtype) {
        for (i = 0; i < num_procs; i++) {
            for (j = 0; j < num_elements; j++) {
                val = (rank * num_procs + i +
                       ((iter + 1) * (rank * num_procs + 1) * (i + 1)));
                if (1 == options.omb_enable_mpi_in_place &&
                    (ALL_GATHER == options.subtype ||
                     ALL_GATHER_P == options.subtype ||
                     NBC_ALL_GATHER == options.subtype ||
                     (GATHER == options.subtype && 0 == is_send_buf) ||
                     (GATHER_P == options.subtype && 0 == is_send_buf) ||
                     (NBC_GATHER == options.subtype && 0 == is_send_buf))) {
                    omb_assign_to_type(temp_buffer,
                                       (id % com_size) * num_elements +
                                           (i * num_elements + j),
                                       val, dtype);
                } else {
                    omb_assign_to_type(temp_buffer, i * num_elements + j, val,
                                       dtype);
                }
            }
        }
    } else {
        for (i = 0; i < num_procs * num_elements; i++) {
            omb_assign_to_type(temp_buffer, i, 0, dtype);
        }
    }
    switch (type) {
        case NONE:
            memcpy((void *)buffer, (void *)temp_buffer, buffer_size);
            break;
        case CUDA:
        case MANAGED:
#ifdef _ENABLE_CUDA_
            CUDA_CHECK(cudaMemcpy((void *)buffer, (void *)temp_buffer,
                                  buffer_size, cudaMemcpyHostToDevice));
            CUDA_CHECK(cudaDeviceSynchronize());
#endif
            break;
        case ROCM:
#ifdef _ENABLE_ROCM_
            ROCM_CHECK(hipMemcpy((void *)buffer, (void *)temp_buffer,
                                 buffer_size, hipMemcpyHostToDevice));
            ROCM_CHECK(hipDeviceSynchronize());
#endif
        case SYCL:
#ifdef _ENABLE_SYCL_
            syclMemcpy((void *)buffer, (void *)temp_buffer, buffer_size);
            syclDeviceSynchronize();
#endif
            break;
        default:
            break;
    }
    free(temp_buffer);
}

int omb_neighborhood_create(MPI_Comm comm, int **indegree_ptr,
                            int **sources_ptr, int **sourceweights_ptr,
                            int **outdegree_ptr, int **destinations_ptr,
                            int **destweights_ptr)
{
    int i = 0, inidx = 0, outidx = 0, comm_size = 0;
    int my_rank = 0, nbr_rank = 0, min_dim = 0;
    int *indgr = NULL, *outdgr = NULL;
    int *srcs = NULL, *srcwghts = NULL, *dests = NULL, *destwghts = NULL;
    int overflow = 0;
    int *dims = NULL, *periods = NULL, *my_coords = NULL;
    int *nbr_coords = NULL, *disp_vec = NULL;
    int d = 0, r = 0;
    int src_itr = 0, dest_itr = 0, src_temp = 0, dest_temp = 0;
    FILE *fp = NULL;
    char *token = NULL;
    char line[OMB_NHBRHD_FILE_LINE_MAX_LENGTH];
    MPI_Comm cart_comm;
    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(comm, &my_rank);

    if (OMB_NHBRHD_TYPE_GRAPH == options.nhbrhd_type) {
        srcs = (int *)malloc(OMB_NHBRHD_ADJ_EDGES_MAX_NUM * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(srcs, "Unable to allocate memory");
        dests = (int *)malloc(OMB_NHBRHD_ADJ_EDGES_MAX_NUM * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(dests, "Unable to allocate memory");
        fp = fopen(options.nhbrhd_type_parameters.filepath, "r");
        OMB_CHECK_NULL_AND_EXIT(fp, "Unable to open graph adjacency list"
                                    " file.\n");
        while (fgets(line, OMB_NHBRHD_FILE_LINE_MAX_LENGTH, fp)) {
            src_temp = dest_temp = -1;
            if ('#' == line[0]) {
                continue;
            }
            if (OMB_NHBRHD_ADJ_EDGES_MAX_NUM < src_itr ||
                OMB_NHBRHD_ADJ_EDGES_MAX_NUM < dest_itr) {
                fprintf(stderr,
                        "ERROR: Max allowed number of edges is:%d\n"
                        "To increase the max allowed edges limit, update"
                        " OMB_NHBRHD_ADJ_EDGES_MAX_NUM in"
                        " c/util/osu_util_mpi.h.\n",
                        OMB_NHBRHD_ADJ_EDGES_MAX_NUM);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            token = strtok(line, ",");
            OMB_CHECK_NULL_AND_EXIT(token,
                                    "Unable to recognise the pattern."
                                    " Check graph adjacency list file.\n");
            src_temp = atoi(token);
            token = strtok(NULL, ",");
            OMB_CHECK_NULL_AND_EXIT(token,
                                    "Unable to recognise the pattern."
                                    " Check graph adjacency list file.\n");
            dest_temp = atoi(token);
            if (src_temp == my_rank) {
                if (dest_temp >= comm_size || src_temp >= comm_size) {
                    OMB_ERROR_EXIT(
                        "Number of processes is less than graph"
                        " nodes. Please increase number of processes.");
                }
                dests[dest_itr++] = dest_temp;
            }
            if (dest_temp == my_rank) {
                if (dest_temp >= comm_size || src_temp >= comm_size) {
                    OMB_ERROR_EXIT(
                        "Number of processes is less than graph"
                        " nodes. Please increase number of processes.");
                }
                srcs[src_itr++] = src_temp;
            }
        }
        fclose(fp);
        indgr = (int *)malloc(sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(indgr, "Unable to allocate memory");
        outdgr = (int *)malloc(sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(outdgr, "Unable to allocate memory");
        *indgr = src_itr;
        *outdgr = dest_itr;
        srcwghts = (int *)malloc(*indgr * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(srcwghts, "Unable to allocate memory");
        destwghts = (int *)malloc(*outdgr * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(destwghts, "Unable to allocate memory");
        for (i = 0; i < *indgr; i++) {
            srcwghts[i] = 1;
        }
        for (i = 0; i < *outdgr; i++) {
            destwghts[i] = 1;
        }
        *indegree_ptr = indgr;
        *sources_ptr = srcs;
        *sourceweights_ptr = srcwghts;
        *outdegree_ptr = outdgr;
        *destinations_ptr = dests;
        *destweights_ptr = destwghts;
    } else {
        d = options.nhbrhd_type_parameters.dim;
        r = options.nhbrhd_type_parameters.rad;
        dims = (int *)malloc(d * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(dims, "Unable to allocate memory");
        periods = (int *)malloc(d * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(periods, "Unable to allocate memory");
        my_coords = (int *)malloc(d * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(my_coords, "Unable to allocate memory");
        nbr_coords = (int *)malloc(d * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(my_coords, "Unable to allocate memory");
        disp_vec = (int *)malloc(d * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(disp_vec, "Unable to allocate memory");
        for (i = 0; i < d; i++) {
            periods[i] = 1;
            dims[i] = 0;
        }
        MPI_CHECK(MPI_Dims_create(comm_size, d, dims));
        MPI_CHECK(MPI_Cart_create(comm, d, dims, periods, 0, &cart_comm));
        MPI_Barrier(MPI_COMM_WORLD);
        if (0 == my_rank) {
            fprintf(stdout, "Dimensions size = ");
            for (i = 0; i < d; i++) {
                fprintf(stdout, "%d ", dims[i]);
            }
            fprintf(stdout, "\n");
        }
        /*Find max valid r based on minimum dimension size*/
        min_dim = comm_size;
        for (i = 0; i < d; i++) {
            if (dims[i] < min_dim)
                min_dim = dims[i];
        }
        /*Divided by 2 to avoid duplicate neighbors*/
        if (r > ((min_dim - 1) / 2)) {
            if (0 == my_rank) {
                fprintf(stderr,
                        "ERROR: the given neighborhood radius (r = %d)"
                        "is greater than the half of the minimum dimension"
                        " size %d. Increase the number of processes or reduce"
                        " dim and rad. Aborting!\n",
                        r, min_dim);
                fflush(stderr);
            }
            MPI_Comm_free(&cart_comm);
            MPI_Finalize();
            exit(0);
        }
        /*Calculate number of neighbors*/
        indgr = (int *)malloc(sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(indgr, "Unable to allocate memory");
        outdgr = (int *)malloc(sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(outdgr, "Unable to allocate memory");
        *outdgr = pow((2 * r + 1), d) - 1;
        *indgr = pow((2 * r + 1), d) - 1;
        srcs = (int *)malloc(*indgr * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(srcs, "Unable to allocate memory");
        srcwghts = (int *)malloc(*indgr * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(srcwghts, "Unable to allocate memory");
        dests = (int *)malloc(*outdgr * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(dests, "Unable to allocate memory");
        destwghts = (int *)malloc(*outdgr * sizeof(int));
        OMB_CHECK_NULL_AND_EXIT(destwghts, "Unable to allocate memory");
        for (i = 0; i < *indgr; i++) {
            srcwghts[i] = 1;
        }
        for (i = 0; i < *outdgr; i++) {
            destwghts[i] = 1;
        }
        for (i = 0; i < d; i++) {
            disp_vec[i] = -r;
        }

        MPI_CHECK(MPI_Cart_coords(cart_comm, my_rank, d, my_coords));
        while (!overflow) {
            for (i = 0; i < d; i++) {
                nbr_coords[i] = my_coords[i] + disp_vec[i];
            }
            MPI_Cart_rank(cart_comm, nbr_coords, &nbr_rank);
            if (nbr_rank != my_rank) {
                dests[outidx] = nbr_rank;
                srcs[inidx] = nbr_rank;
                outidx++;
                inidx++;
            }
            for (i = d - 1; i >= -1; i--) {
                if (-1 == i) {
                    overflow = 1;
                    break;
                }
                if (r == disp_vec[i]) {
                    disp_vec[i] = -r;
                } else {
                    disp_vec[i]++;
                    break;
                }
            }
        }
        *indegree_ptr = indgr;
        *sources_ptr = srcs;
        *sourceweights_ptr = srcwghts;
        *outdegree_ptr = outdgr;
        *destinations_ptr = dests;
        *destweights_ptr = destwghts;
        MPI_Comm_free(&cart_comm);
        free(dims);
        free(periods);
        free(my_coords);
        free(nbr_coords);
        free(disp_vec);
    }
    return 0;
}

uint8_t validate_data(void *r_buf, size_t size, int num_procs,
                      enum accel_type type, int iter, MPI_Datatype dtype)
{
    void *temp_r_buf = NULL;
    int numprocs = 0;
    int rank = 0, error = 0;

    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    switch (options.bench) {
        case PT2PT:
        case MBW_MR: {
            register int i = 0;
            int num_elements = omb_get_num_elements(size, dtype);
            temp_r_buf = malloc(size);
            void *expected_buffer = malloc(size);
            int val = 0, j = 0;
            char buf_type = 'H';

            if (options.bench == MBW_MR) {
                buf_type = (rank < options.pairs) ? options.src : options.dst;
            } else {
                buf_type = (rank == 0) ? options.src : options.dst;
            }
            switch (buf_type) {
                case 'H':
                    memcpy((void *)temp_r_buf, (void *)r_buf, size);
                    break;
                case 'D':
                case 'M':
#ifdef _ENABLE_OPENACC_
                    if (type == OPENACC) {
                        size_t i;
                        char *p = (char *)r_buf;
#pragma acc parallel loop deviceptr(p)
                        for (i = 0; i < num_elements; i++) {
                            temp_r_buf[i] = p[i];
                        }
                        break;
                    } else
#endif
#ifdef _ENABLE_CUDA_
                    {
                        CUDA_CHECK(cudaMemcpy((void *)temp_r_buf, (void *)r_buf,
                                              size, cudaMemcpyDeviceToHost));
                        CUDA_CHECK(cudaDeviceSynchronize());
                    }
#endif
#ifdef _ENABLE_ROCM_
                    {
                        ROCM_CHECK(hipMemcpy((void *)temp_r_buf, (void *)r_buf,
                                             size, hipMemcpyDeviceToHost));
                        ROCM_CHECK(hipDeviceSynchronize());
                    }
#endif
#ifdef _ENABLE_SYCL_
                    {
                        syclMemcpy((void *)temp_r_buf, (void *)r_buf, size);
                        syclDeviceSynchronize();
                    }
#endif
                    break;
            }
            for (i = 0; i < num_elements; i++) {
                val = (CHAR_VALIDATION_MULTIPLIER * (i + 1) + size + iter);
                omb_assign_to_type(expected_buffer, i, val, dtype);
            }
            if (dtype == MPI_FLOAT) {
                for (i = 0; i < num_elements; i++) {
                    j = (i % 100);
                    if (abs(((float *)temp_r_buf)[i] -
                            ((float *)expected_buffer)[i]) > ERROR_DELTA) {
                        error = 1;
                        break;
                    }
                }
            } else if (dtype == MPI_DOUBLE) {
                for (i = 0; i < num_elements; i++) {
                    j = (i % 100);
                    if (abs(((double *)temp_r_buf)[i] -
                            ((double *)expected_buffer)[i]) > ERROR_DELTA) {
                        error = 1;
                        break;
                    }
                }
            } else if (memcmp(temp_r_buf, expected_buffer, num_elements)) {
                error = 1;
            }
            if (1 == error && options.log_validation) {
                validation_log(temp_r_buf, expected_buffer, size, num_elements,
                               dtype, iter);
            }
            free(expected_buffer);
            free(temp_r_buf);
            return error;
        } break;
        case COLLECTIVE:
            switch (options.subtype) {
                case REDUCE:
                case ALL_REDUCE:
                case REDUCE_P:
                case ALL_REDUCE_P:
                case NBC_REDUCE:
                case NBC_ALL_REDUCE: {
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    return validate_reduction(r_buf, size, iter, numprocs,
                                              options.accel, dtype);
                } break;
                case ALLTOALL:
                case ALLTOALL_P:
                case NBC_ALLTOALL: {
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
                    return validate_collective(r_buf, size, rank, numprocs,
                                               type, iter, dtype);
                } break;
                case GATHER:
                case GATHER_P:
                case NBC_GATHER: {
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    return validate_collective(r_buf, size, 0, numprocs, type,
                                               iter, dtype);
                } break;
                case ALL_GATHER:
                case ALL_GATHER_P:
                case NBC_ALL_GATHER: {
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    return validate_collective(r_buf, size, 0, numprocs, type,
                                               iter, dtype);
                } break;
                case SCATTER:
                case SCATTER_P:
                case NBC_SCATTER: {
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
                    return validate_collective(r_buf, size, rank, 1, type, iter,
                                               dtype);
                } break;
                case REDUCE_SCATTER:
                case REDUCE_SCATTER_P:
                case NBC_REDUCE_SCATTER: {
                    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
                    return validate_reduction(r_buf, size, iter, numprocs,
                                              options.accel, dtype);
                } break;
                case BCAST:
                case BCAST_P:
                case NBC_BCAST: {
                    return validate_collective(r_buf, size, 1, 1, type, iter,
                                               dtype);
                } break;

                default:
                    break;
            }
            break;
        default:
            break;
    }
    return 1;
}

void set_buffer_nhbr_validation(void *s_buf, void *r_buf, int indegree,
                                int *sources, int outdegree, int *destinations,
                                size_t size, enum accel_type type, int iter,
                                MPI_Datatype dtype)
{
    void *temp_s_buf = NULL;
    void *temp_r_buf = NULL;
    int rank = -1, numprocs = -1;
    int l = 0, i = 0, res = 0;
    int value1 = 0, send_numprocs = 0, recv_numprocs = 0;

    if (NULL == s_buf || NULL == r_buf) {
        return;
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    switch (options.subtype) {
        case NHBR_GATHER:
        case NBC_NHBR_GATHER: {
            send_numprocs = 1;
            recv_numprocs = indegree;
        } break;
        case NHBR_ALLTOALL:
        case NBC_NHBR_ALLTOALL: {
            send_numprocs = outdegree;
            recv_numprocs = indegree;
        } break;
        default: {
            OMB_ERROR_EXIT("Unknown subtype");
        } break;
    }
    temp_r_buf = malloc(recv_numprocs * size);
    OMB_CHECK_NULL_AND_EXIT(temp_r_buf, "Unable to allocate memory");
    temp_s_buf = malloc(size * send_numprocs);
    OMB_CHECK_NULL_AND_EXIT(temp_s_buf, "Unable to allocate memory");
    for (i = 0; i < send_numprocs; i++) {
        switch (options.subtype) {
            case NHBR_GATHER:
            case NBC_NHBR_GATHER: {
                value1 = 0;
            } break;
            case NHBR_ALLTOALL:
            case NBC_NHBR_ALLTOALL: {
                value1 = destinations[i];
            } break;
            default: {
                OMB_ERROR_EXIT("Unknown subtype");
            } break;
        }
        for (l = 0; l < omb_get_num_elements(size, dtype); l++) {
            res = rank + iter + l + value1 + 1;
            omb_assign_to_type(temp_s_buf,
                               i * omb_get_num_elements(size, dtype) + l, res,
                               dtype);
        }
    }
    for (l = 0; l < omb_get_num_elements(size, dtype) * recv_numprocs; l++) {
        omb_assign_to_type(temp_r_buf, l, 0, dtype);
    }
    switch (type) {
        case NONE:
            memcpy((void *)s_buf, (void *)temp_s_buf, size * send_numprocs);
            memcpy((void *)r_buf, (void *)temp_r_buf, size * recv_numprocs);
            break;
        case CUDA:
        case MANAGED:
#ifdef _ENABLE_CUDA_
            CUDA_CHECK(cudaMemcpy((void *)s_buf, (void *)temp_s_buf,
                                  size * send_numprocs,
                                  cudaMemcpyHostToDevice));
            CUDA_CHECK(cudaMemcpy((void *)r_buf, (void *)temp_r_buf,
                                  size * recv_numprocs,
                                  cudaMemcpyHostToDevice));
            CUDA_CHECK(cudaDeviceSynchronize());
#endif
            break;
        case ROCM:
#ifdef _ENABLE_ROCM_
            ROCM_CHECK(hipMemcpy((void *)s_buf, (void *)temp_s_buf,
                                 size * send_numprocs, hipMemcpyHostToDevice));
            ROCM_CHECK(hipMemcpy((void *)r_buf, (void *)temp_r_buf,
                                 size * recv_numprocs, hipMemcpyHostToDevice));
            ROCM_CHECK(hipDeviceSynchronize());
#endif
            break;
        case SYCL:
#ifdef _ENABLE_SYCL_
        {
            syclMemcpy((void *)s_buf, (void *)temp_s_buf, size * send_numprocs);
            syclMemcpy((void *)r_buf, (void *)temp_r_buf, size * recv_numprocs);
            syclDeviceSynchronize();
        }
#endif
        break;
        default:
            OMB_ERROR_EXIT("Unknown accelerator type");
            break;
    }
    free(temp_r_buf);
    free(temp_s_buf);
}

int omb_validate_neighborhood_col(MPI_Comm comm, char *buffer, int indegree,
                                  int outdegree, size_t size,
                                  enum accel_type type, int iter,
                                  MPI_Datatype dtype)
{
    int *sources = NULL, *sourceweights = NULL;
    int *destinations = NULL, *destweights = NULL;
    void *temp_r_buf = NULL;
    int rank = -1, errors = 0;
    int l = 0;
    int s = 0;
    int expected_value = 0;
    int value1 = 0, recv_numprocs = 0;
    void *log_buffer = NULL, *expected_buffer = NULL;
    int i = 0;
    int num_elements = 0;

    temp_r_buf = malloc(size * indegree);
    OMB_CHECK_NULL_AND_EXIT(temp_r_buf, "Unable to allocate memory");
    sources = (int *)malloc(indegree * sizeof(int));
    OMB_CHECK_NULL_AND_EXIT(sources, "Unable to allocate memory");
    sourceweights = (int *)malloc(indegree * sizeof(int));
    OMB_CHECK_NULL_AND_EXIT(sourceweights, "Unable to allocate memory");
    destinations = (int *)malloc(outdegree * sizeof(int));
    OMB_CHECK_NULL_AND_EXIT(destinations, "Unable to allocate memory");
    destweights = (int *)malloc(outdegree * sizeof(int));
    OMB_CHECK_NULL_AND_EXIT(destweights, "Unable to allocate memory");
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    MPI_CHECK(MPI_Dist_graph_neighbors(comm, indegree, sources, sourceweights,
                                       outdegree, destinations, destweights));
    switch (options.subtype) {
        case NHBR_GATHER:
        case NBC_NHBR_GATHER: {
            value1 = 0;
            recv_numprocs = indegree;
        } break;
        case NHBR_ALLTOALL:
        case NBC_NHBR_ALLTOALL: {
            value1 = rank;
            recv_numprocs = indegree;
        } break;
        default: {
            OMB_ERROR_EXIT("Unknown subtype");
        } break;
    }
    switch (type) {
        case NONE:
            memcpy((void *)temp_r_buf, (void *)buffer, size * recv_numprocs);
            break;
#ifdef _ENABLE_CUDA_
        case CUDA:
        case MANAGED:
            CUDA_CHECK(cudaMemcpy((void *)temp_r_buf, (void *)buffer,
                                  size * recv_numprocs,
                                  cudaMemcpyDeviceToHost));
            CUDA_CHECK(cudaDeviceSynchronize());
            break;
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            ROCM_CHECK(hipMemcpy((void *)temp_r_buf, (void *)buffer,
                                 size * recv_numprocs, hipMemcpyDeviceToHost));
            ROCM_CHECK(hipDeviceSynchronize());
            break;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL: {
            syclMemcpy((void *)temp_r_buf, (void *)buffer,
                       size * recv_numprocs);
            syclDeviceSynchronize();
        } break;
#endif
        default:
            OMB_ERROR_EXIT("Unknown device type");
            break;
    }
    num_elements = omb_get_num_elements(size, dtype);
    log_buffer = malloc(size * recv_numprocs);
    OMB_CHECK_NULL_AND_EXIT(log_buffer, "Unable to allocate memory");
    expected_buffer = malloc(size * recv_numprocs);
    OMB_CHECK_NULL_AND_EXIT(expected_buffer, "Unable to allocate memory");
    i = 0;
    for (s = 0; s < omb_get_num_elements(size, dtype); s++) {
        for (l = 0; l < recv_numprocs; l++) {
            expected_value = (sources[l] + value1 + s + iter + 1);
            if (MPI_CHAR == dtype || MPI_SIGNED_CHAR == dtype) {
                ((char *)log_buffer)[i] =
                    ((char *)
                         temp_r_buf)[l * omb_get_num_elements(size, dtype) + s];
                ((char *)expected_buffer)[i] =
                    (char)(CHAR_VALIDATION_MULTIPLIER * expected_value) %
                    CHAR_RANGE;
                if (((char *)temp_r_buf)[l * omb_get_num_elements(size, dtype) +
                                         s] !=
                    (char)(CHAR_VALIDATION_MULTIPLIER * expected_value) %
                        CHAR_RANGE) {
                    errors = 1;
                }
            } else if (MPI_INT == dtype) {
                ((int *)log_buffer)[i] =
                    ((int *)
                         temp_r_buf)[l * omb_get_num_elements(size, dtype) + s];
                ((int *)expected_buffer)[i] =
                    (int)expected_value * INT_VALIDATION_MULTIPLIER;
                if (((int *)temp_r_buf)[l * omb_get_num_elements(size, dtype) +
                                        s] !=
                    (int)expected_value * INT_VALIDATION_MULTIPLIER) {
                    errors = 1;
                }
            } else if (MPI_FLOAT == dtype) {
                ((float *)log_buffer)[i] =
                    ((float *)
                         temp_r_buf)[l * omb_get_num_elements(size, dtype) + s];
                ((float *)expected_buffer)[i] =
                    (float)expected_value * FLOAT_VALIDATION_MULTIPLIER;
                if (((float *)
                         temp_r_buf)[l * omb_get_num_elements(size, dtype) +
                                     s] !=
                    (float)expected_value * FLOAT_VALIDATION_MULTIPLIER) {
                    errors = 1;
                }
            } else if (MPI_DOUBLE == dtype) {
                ((double *)log_buffer)[i] =
                    ((double *)
                         temp_r_buf)[l * omb_get_num_elements(size, dtype) + s];
                ((double *)expected_buffer)[i] =
                    (double)expected_value * DOUBLE_VALIDATION_MULTIPLIER;
                if (((double *)
                         temp_r_buf)[l * omb_get_num_elements(size, dtype) +
                                     s] !=
                    (double)expected_value * DOUBLE_VALIDATION_MULTIPLIER) {
                    errors = 1;
                }
            }
            i++;
        }
    }
    if (1 == errors && options.log_validation) {
        validation_log(log_buffer, expected_buffer, size, i, dtype, iter);
    }
    free(log_buffer);
    free(expected_buffer);
    free(sources);
    free(sourceweights);
    free(destinations);
    free(destweights);
    free(temp_r_buf);
    return errors;
}

int validate_reduce_scatter(void *buffer, size_t size, int *recvcounts,
                            int rank, int num_procs, enum accel_type type,
                            int iter, MPI_Datatype dtype)
{
    int i = 0, j = 0, k = 0, l = 0, m = 0, errors = 0;
    void *expected_buffer = malloc(size);
    void *temp_buffer = malloc(size);
    int val = 0;
    void *log_buffer = NULL;

    switch (type) {
        case NONE:
            memcpy((void *)temp_buffer, (void *)buffer, size);
            break;
#ifdef _ENABLE_CUDA_
        case CUDA:
        case MANAGED:
            CUDA_CHECK(cudaMemcpy((void *)temp_buffer, (void *)buffer, size,
                                  cudaMemcpyDeviceToHost));
            CUDA_CHECK(cudaDeviceSynchronize());
            break;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL:
            syclMemcpy((void *)temp_buffer, (void *)buffer, size);
            syclDeviceSynchronize();
            break;
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            ROCM_CHECK(hipMemcpy((void *)temp_buffer, (void *)buffer, size,
                                 hipMemcpyDeviceToHost));
            ROCM_CHECK(hipDeviceSynchronize());
            break;
#endif
        default:
            break;
    }
    for (k = 0; k < rank; k++) {
        m += recvcounts[k];
    }
    log_buffer = malloc(size);
    OMB_CHECK_NULL_AND_EXIT(log_buffer, "Unable to allocate buffer");
    for (i = m, l = 0; i < recvcounts[rank]; i++, l++) {
        j = (i % 100);
        val = (j + 1) * (iter + 1) * num_procs;
        if (MPI_CHAR == dtype || MPI_SIGNED_CHAR == dtype) {
            ((char *)expected_buffer)[l] =
                (char)(CHAR_VALIDATION_MULTIPLIER * val) % CHAR_RANGE;
            ((char *)log_buffer)[l] = ((char *)temp_buffer)[i];
            if (((char *)temp_buffer)[i] !=
                (char)(CHAR_VALIDATION_MULTIPLIER * val) % CHAR_RANGE) {
                errors = 1;
            }
        } else if (MPI_INT == dtype) {
            ((int *)expected_buffer)[l] = (int)val * INT_VALIDATION_MULTIPLIER;
            ((int *)log_buffer)[l] = ((int *)temp_buffer)[i];
            if (((int *)temp_buffer)[i] !=
                (int)val * INT_VALIDATION_MULTIPLIER) {
                errors = 1;
            }
        } else if (MPI_FLOAT == dtype) {
            ((float *)expected_buffer)[l] =
                (float)val * FLOAT_VALIDATION_MULTIPLIER;
            ((float *)log_buffer)[l] = ((float *)temp_buffer)[i];
            if (((float *)temp_buffer)[i] !=
                (float)val * FLOAT_VALIDATION_MULTIPLIER) {
                errors = 1;
            }
        } else if (MPI_DOUBLE == dtype) {
            ((double *)expected_buffer)[l] =
                (double)val * DOUBLE_VALIDATION_MULTIPLIER;
            ((double *)log_buffer)[l] = ((double *)temp_buffer)[i];
            if (((double *)temp_buffer)[i] !=
                (double)val * DOUBLE_VALIDATION_MULTIPLIER) {
                errors = 1;
            }
        }
    }
    if (1 == errors && options.log_validation) {
        validation_log(log_buffer, expected_buffer, size, recvcounts[k], dtype,
                       iter);
    }
    free(log_buffer);
    free(expected_buffer);
    free(temp_buffer);
    return errors;
}

int validate_reduction(void *buffer, size_t size, int iter, int num_procs,
                       enum accel_type type, MPI_Datatype dtype)
{
    int i = 0, j = 0, errors = 0, val = 0;
    void *expected_buffer = malloc(size);
    void *temp_buffer = malloc(size);
    int num_elements = omb_get_num_elements(size, dtype);

    switch (type) {
        case NONE:
            memcpy((void *)temp_buffer, (void *)buffer, size);
            break;
#ifdef _ENABLE_CUDA_
        case CUDA:
        case MANAGED:
            CUDA_CHECK(cudaMemcpy((void *)temp_buffer, (void *)buffer, size,
                                  cudaMemcpyDeviceToHost));
            CUDA_CHECK(cudaDeviceSynchronize());
            break;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL:
            syclMemcpy((void *)temp_buffer, (void *)buffer, size);
            syclDeviceSynchronize();
            break;
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            ROCM_CHECK(hipMemcpy((void *)temp_buffer, (void *)buffer, size,
                                 hipMemcpyDeviceToHost));
            ROCM_CHECK(hipDeviceSynchronize());
            break;
#endif
        default:
            break;
    }
    for (i = 0; i < num_elements; i++) {
        j = (i % 100);
        val = (j + 1) * (iter + 1) * num_procs;
        omb_assign_to_type(expected_buffer, i, val, dtype);
    }
    if (dtype == MPI_FLOAT) {
        for (i = 0; i < num_elements; i++) {
            j = (i % 100);
            if (abs(((float *)temp_buffer)[i] - ((float *)expected_buffer)[i]) >
                ERROR_DELTA) {
                errors = 1;
                break;
            }
        }
    } else if (dtype == MPI_DOUBLE) {
        for (i = 0; i < num_elements; i++) {
            j = (i % 100);
            if (abs(((double *)temp_buffer)[i] -
                    ((double *)expected_buffer)[i]) > ERROR_DELTA) {
                errors = 1;
                break;
            }
        }
    } else {
        if (memcmp(temp_buffer, expected_buffer, size) != 0) {
            errors = 1;
        }
    }
    if (1 == errors && options.log_validation) {
        validation_log(temp_buffer, expected_buffer, size, num_elements, dtype,
                       iter);
    }
    free(expected_buffer);
    free(temp_buffer);
    return errors;
}

int validate_collective(void *buffer, size_t size, int value1, int value2,
                        enum accel_type type, int itr, MPI_Datatype dtype)
{
    int i = 0, j = 0, errors = 0, val = 0;
    void *expected_buffer = malloc(size * value2);
    void *temp_buffer = malloc(size * value2);
    int num_elements = omb_get_num_elements(size, dtype);

    switch (type) {
        case NONE:
            memcpy((void *)temp_buffer, (void *)buffer, size * value2);
            break;
#ifdef _ENABLE_CUDA_
        case CUDA:
        case MANAGED:
            CUDA_CHECK(cudaMemcpy((void *)temp_buffer, (void *)buffer,
                                  size * value2, cudaMemcpyDeviceToHost));
            CUDA_CHECK(cudaDeviceSynchronize());
            break;
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            ROCM_CHECK(hipMemcpy((void *)temp_buffer, (void *)buffer,
                                 size * value2, hipMemcpyDeviceToHost));
            ROCM_CHECK(hipDeviceSynchronize());
            break;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL:
            syclMemcpy((void *)temp_buffer, (void *)buffer, size * value2);
            syclDeviceSynchronize();
            break;
#endif
        default:
            break;
    }

    for (i = 0; i < value2; i++) {
        for (j = 0; j < num_elements; j++) {
            val = (i * value2 + value1 +
                   ((itr + 1) * (value1 + 1) * (i * value2 + 1)));
            omb_assign_to_type(expected_buffer, i * num_elements + j, val,
                               dtype);
        }
    }
    if (dtype == MPI_FLOAT) {
        for (i = 0; i < num_elements; i++) {
            j = (i % 100);
            if (abs(((float *)temp_buffer)[i] - ((float *)expected_buffer)[i]) >
                ERROR_DELTA) {
                errors = 1;
                break;
            }
        }
    } else if (dtype == MPI_DOUBLE) {
        for (i = 0; i < num_elements; i++) {
            j = (i % 100);
            if (abs(((double *)temp_buffer)[i] -
                    ((double *)expected_buffer)[i]) > ERROR_DELTA) {
                errors = 1;
                break;
            }
        }
    } else {
        if (memcmp(temp_buffer, expected_buffer, size * value2) != 0) {
            errors = 1;
        }
    }
    if (1 == errors && options.log_validation) {
        validation_log(temp_buffer, expected_buffer, size, num_elements, dtype,
                       itr);
    }
    free(expected_buffer);
    free(temp_buffer);
    return errors;
}

void validation_log(void *buffer, void *expected_buffer, size_t size,
                    size_t num_elements, MPI_Datatype dtype, int itr)
{
    int rank = 0, i = 0;
    char *log_file_loc = NULL;
    FILE *log_file_fp = NULL;

    if (0 != mkdir(options.log_validation_dir_path, 0744)) {
        if (0 != access(options.log_validation_dir_path, F_OK)) {
            OMB_ERROR_EXIT("Unable to create directory");
        }
    }
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    log_file_loc = malloc(OMB_FILE_PATH_MAX_LENGTH * sizeof(char));
    OMB_CHECK_NULL_AND_EXIT(log_file_loc, "Unable to allocate memory.");
    sprintf(log_file_loc, "%s/log-%d.log", options.log_validation_dir_path,
            rank);
    log_file_fp = fopen(log_file_loc, "a");
    OMB_CHECK_NULL_AND_EXIT(log_file_loc, "Unable to open file.");
    fprintf(log_file_fp, "Size: %d, Iteration:%d, ", size, itr);
    if (MPI_FLOAT == dtype) {
        fprintf(log_file_fp, "Datatype: MPI_FLOAT\n");
    } else if (MPI_DOUBLE == dtype) {
        fprintf(log_file_fp, "Datatype: MPI_DOUBLE\n");
    } else if (MPI_INT == dtype) {
        fprintf(log_file_fp, "Datatype: MPI_INT\n");
    } else if (MPI_CHAR == dtype) {
        fprintf(log_file_fp, "Datatype: MPI_CHAR\n");
    } else if (MPI_SIGNED_CHAR == dtype) {
        fprintf(log_file_fp, "Datatype: MPI_SIGNED_CHAR\n");
    } else {
        OMB_ERROR_EXIT("Invalid MPI Datatype.");
    }
    fprintf(log_file_fp, "%-*s%*s%*s\n", 10, "Position", FIELD_WIDTH,
            "Expected", FIELD_WIDTH, "Actual");

    if (dtype == MPI_FLOAT) {
        for (i = 0; i < num_elements; i++) {
            if (abs(((float *)buffer)[i] - ((float *)expected_buffer)[i]) >
                ERROR_DELTA) {
                fprintf(log_file_fp, "%-*d%*f%*f\n", 10, i, FIELD_WIDTH,
                        ((float *)expected_buffer)[i], FIELD_WIDTH,
                        ((float *)buffer)[i]);
            }
        }
    } else if (dtype == MPI_DOUBLE) {
        for (i = 0; i < num_elements; i++) {
            if (abs(((double *)buffer)[i] - ((double *)expected_buffer)[i]) >
                ERROR_DELTA) {
                fprintf(log_file_fp, "%-*d%*f%*f\n", 10, i, FIELD_WIDTH,
                        ((double *)expected_buffer)[i], FIELD_WIDTH,
                        ((double *)buffer)[i]);
            }
        }
    } else if (dtype == MPI_INT) {
        for (i = 0; i < num_elements; i++) {
            if (((int *)buffer)[i] != ((int *)expected_buffer)[i]) {
                fprintf(log_file_fp, "%-*d%*d%*d\n", 10, i, FIELD_WIDTH,
                        ((int *)expected_buffer)[i], FIELD_WIDTH,
                        ((int *)buffer)[i]);
            }
        }
    } else if (MPI_CHAR == dtype || MPI_SIGNED_CHAR == dtype) {
        if (((char *)buffer)[i] != ((char *)expected_buffer)[i]) {}
        for (i = 0; i < num_elements; i++) {
            if (((char *)buffer)[i] != ((char *)expected_buffer)[i]) {
                fprintf(log_file_fp, "%-*d%*d%*d\n", 10, i, FIELD_WIDTH,
                        ((char *)expected_buffer)[i], FIELD_WIDTH,
                        ((char *)buffer)[i]);
            }
        }
    }
    fclose(log_file_fp);
}

int allocate_memory_coll(void **buffer, size_t size, enum accel_type type)
{
    if (options.target == CPU || options.target == BOTH) {
        allocate_host_arrays();
    }

    size_t alignment = sysconf(_SC_PAGESIZE);

    switch (type) {
        case NONE:
            return posix_memalign(buffer, alignment, size);
#ifdef _ENABLE_CUDA_
        case CUDA:
            CUDA_CHECK(cudaMalloc(buffer, size));
            return 0;
        case MANAGED:
            CUDA_CHECK(cudaMallocManaged(buffer, size, cudaMemAttachGlobal));
            return 0;
#endif
#ifdef _ENABLE_OPENACC_
        case OPENACC:
            *buffer = acc_malloc(size);
            if (NULL == *buffer) {
                return 1;
            } else {
                return 0;
            }
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            ROCM_CHECK(hipMalloc(buffer, size));
            return 0;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL:
            syclMalloc(buffer, size);
            return 0;
#endif
        default:
            return 1;
    }
}

int allocate_device_buffer(char **buffer)
{
    switch (options.accel) {
#ifdef _ENABLE_CUDA_
        case CUDA:
            CUDA_CHECK(cudaMalloc((void **)buffer, options.max_message_size));
            break;
#endif
#ifdef _ENABLE_OPENACC_
        case OPENACC:
            *buffer = acc_malloc(options.max_message_size);
            if (NULL == *buffer) {
                fprintf(stderr, "Could not allocate device memory\n");
                return 1;
            }
            break;
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            ROCM_CHECK(hipMalloc((void **)buffer, options.max_message_size));
            break;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL:
            syclMalloc(buffer, options.max_message_size);
            break;
#endif
        default:
            fprintf(stderr, "Could not allocate device memory\n");
            return 1;
    }

    return 0;
}

int allocate_managed_buffer(char **buffer)
{
    switch (options.accel) {
#ifdef _ENABLE_CUDA_
        case CUDA:
            CUDA_CHECK(cudaMallocManaged((void **)buffer,
                                         options.max_message_size,
                                         cudaMemAttachGlobal));
            break;
#endif
        default:
            fprintf(stderr, "Could not allocate managed/unified memory\n");
            return 1;
    }
    return 0;
}

int allocate_device_buffer_size(char **buffer, size_t size)
{
    switch (options.accel) {
#ifdef _ENABLE_CUDA_
        case CUDA:
            CUDA_CHECK(cudaMalloc((void **)buffer, size));
            break;
#endif
#ifdef _ENABLE_OPENACC_
        case OPENACC:
            *buffer = acc_malloc(size);
            if (NULL == *buffer) {
                fprintf(stderr, "Could not allocate device memory\n");
                return 1;
            }
            break;
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            ROCM_CHECK(hipMalloc((void **)buffer, size));
            break;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL:
            syclMalloc(buffer, size);
            break;
#endif
        default:
            fprintf(stderr, "Could not allocate device memory\n");
            return 1;
    }

    return 0;
}

int allocate_managed_buffer_size(char **buffer, size_t size)
{
    switch (options.accel) {
#ifdef _ENABLE_CUDA_
        case CUDA:
            CUDA_CHECK(
                cudaMallocManaged((void **)buffer, size, cudaMemAttachGlobal));
            break;
#endif
        default:
            fprintf(stderr, "Could not allocate managed memory\n");
            return 1;
    }
    return 0;
}

int allocate_device_buffer_one_sided(char **buffer, size_t size, char dev)
{
    if ('D' == dev) {
        if (allocate_device_buffer_size(buffer, size)) {
            fprintf(stderr, "Error allocating cuda memory\n");
            return 1;
        }
    } else if ('M' == dev) {
        if (allocate_managed_buffer_size(buffer, size)) {
            fprintf(stderr, "Error allocating cuda unified memory\n");
            return 1;
        }
    }

    return 0;
}

int allocate_memory_pt2pt_mul(char **sbuf, char **rbuf, int rank, int pairs)
{
    unsigned long align_size = sysconf(_SC_PAGESIZE);

    if (rank < pairs) {
        if ('D' == options.src) {
            if (allocate_device_buffer(sbuf)) {
                fprintf(stderr, "Error allocating cuda memory\n");
                return 1;
            }

            if (allocate_device_buffer(rbuf)) {
                fprintf(stderr, "Error allocating cuda memory\n");
                return 1;
            }
        } else if ('M' == options.src) {
            if (allocate_managed_buffer(sbuf)) {
                fprintf(stderr, "Error allocating cuda unified memory\n");
                return 1;
            }

            if (allocate_managed_buffer(rbuf)) {
                fprintf(stderr, "Error allocating cuda unified memory\n");
                return 1;
            }
        } else {
            if (posix_memalign((void **)sbuf, align_size,
                               options.max_message_size)) {
                fprintf(stderr, "Error allocating host memory\n");
                return 1;
            }

            if (posix_memalign((void **)rbuf, align_size,
                               options.max_message_size)) {
                fprintf(stderr, "Error allocating host memory\n");
                return 1;
            }

            memset(*sbuf, 0, options.max_message_size);
            memset(*rbuf, 0, options.max_message_size);
        }
    } else {
        if ('D' == options.dst) {
            if (allocate_device_buffer(sbuf)) {
                fprintf(stderr, "Error allocating cuda memory\n");
                return 1;
            }

            if (allocate_device_buffer(rbuf)) {
                fprintf(stderr, "Error allocating cuda memory\n");
                return 1;
            }
        } else if ('M' == options.dst) {
            if (allocate_managed_buffer(sbuf)) {
                fprintf(stderr, "Error allocating cuda unified memory\n");
                return 1;
            }

            if (allocate_managed_buffer(rbuf)) {
                fprintf(stderr, "Error allocating cuda unified memory\n");
                return 1;
            }
        } else {
            if (posix_memalign((void **)sbuf, align_size,
                               options.max_message_size)) {
                fprintf(stderr, "Error allocating host memory\n");
                return 1;
            }

            if (posix_memalign((void **)rbuf, align_size,
                               options.max_message_size)) {
                fprintf(stderr, "Error allocating host memory\n");
                return 1;
            }
            memset(*sbuf, 0, options.max_message_size);
            memset(*rbuf, 0, options.max_message_size);
        }
    }

    return 0;
}

int allocate_memory_pt2pt_mul_size(char **sbuf, char **rbuf, int rank,
                                   int pairs, size_t allocate_size)
{
    size_t size;
    unsigned long align_size = sysconf(_SC_PAGESIZE);

    if (allocate_size == 0) {
        size = 1;
    } else {
        size = allocate_size;
    }

    if (rank < pairs) {
        if ('D' == options.src) {
            if (allocate_device_buffer(sbuf)) {
                fprintf(stderr, "Error allocating cuda memory\n");
                return 1;
            }

            if (allocate_device_buffer(rbuf)) {
                fprintf(stderr, "Error allocating cuda memory\n");
                return 1;
            }
        } else if ('M' == options.src) {
            if (allocate_managed_buffer_size(sbuf, size)) {
                fprintf(stderr, "Error allocating cuda unified memory\n");
                return 1;
            }

            if (allocate_managed_buffer_size(rbuf, size)) {
                fprintf(stderr, "Error allocating cuda unified memory\n");
                return 1;
            }
        } else {
            if (posix_memalign((void **)sbuf, align_size, size)) {
                fprintf(stderr, "Error allocating host memory\n");
                return 1;
            }

            if (posix_memalign((void **)rbuf, align_size, size)) {
                fprintf(stderr, "Error allocating host memory\n");
                return 1;
            }

            memset(*sbuf, 0, size);
            memset(*rbuf, 0, size);
        }
    } else {
        if ('D' == options.dst) {
            if (allocate_device_buffer(sbuf)) {
                fprintf(stderr, "Error allocating cuda memory\n");
                return 1;
            }

            if (allocate_device_buffer(rbuf)) {
                fprintf(stderr, "Error allocating cuda memory\n");
                return 1;
            }
        } else if ('M' == options.dst) {
            if (allocate_managed_buffer_size(sbuf, size)) {
                fprintf(stderr, "Error allocating cuda unified memory\n");
                return 1;
            }

            if (allocate_managed_buffer_size(rbuf, size)) {
                fprintf(stderr, "Error allocating cuda unified memory\n");
                return 1;
            }
        } else {
            if (posix_memalign((void **)sbuf, align_size, size)) {
                fprintf(stderr, "Error allocating host memory\n");
                return 1;
            }

            if (posix_memalign((void **)rbuf, align_size, size)) {
                fprintf(stderr, "Error allocating host memory\n");
                return 1;
            }
            memset(*sbuf, 0, size);
            memset(*rbuf, 0, size);
        }
    }

    return 0;
}

int allocate_memory_pt2pt(char **sbuf, char **rbuf, int rank)
{
    unsigned long align_size = sysconf(_SC_PAGESIZE);

    switch (rank) {
        case 0:
            if ('D' == options.src) {
                if (allocate_device_buffer(sbuf)) {
                    fprintf(stderr, "Error allocating cuda memory\n");
                    return 1;
                }

                if (allocate_device_buffer(rbuf)) {
                    fprintf(stderr, "Error allocating cuda memory\n");
                    return 1;
                }
            } else if ('M' == options.src) {
                if (allocate_managed_buffer(sbuf)) {
                    fprintf(stderr, "Error allocating cuda unified memory\n");
                    return 1;
                }

                if (allocate_managed_buffer(rbuf)) {
                    fprintf(stderr, "Error allocating cuda unified memory\n");
                    return 1;
                }
            } else {
                if (posix_memalign((void **)sbuf, align_size,
                                   options.max_message_size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }

                if (posix_memalign((void **)rbuf, align_size,
                                   options.max_message_size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }
            }
            break;
        case 1:
            if ('D' == options.dst) {
                if (allocate_device_buffer(sbuf)) {
                    fprintf(stderr, "Error allocating cuda memory\n");
                    return 1;
                }

                if (allocate_device_buffer(rbuf)) {
                    fprintf(stderr, "Error allocating cuda memory\n");
                    return 1;
                }
            } else if ('M' == options.dst) {
                if (allocate_managed_buffer(sbuf)) {
                    fprintf(stderr, "Error allocating cuda unified memory\n");
                    return 1;
                }

                if (allocate_managed_buffer(rbuf)) {
                    fprintf(stderr, "Error allocating cuda unified memory\n");
                    return 1;
                }
            } else {
                if (posix_memalign((void **)sbuf, align_size,
                                   options.max_message_size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }

                if (posix_memalign((void **)rbuf, align_size,
                                   options.max_message_size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }
            }
            break;
    }

    return 0;
}

int allocate_memory_pt2pt_size(char **sbuf, char **rbuf, int rank,
                               size_t allocate_size)
{
    size_t size;
    unsigned long align_size = sysconf(_SC_PAGESIZE);

    if (allocate_size == 0) {
        size = 1;
    } else {
        size = allocate_size;
    }

    switch (rank) {
        case 0:
            if ('D' == options.src) {
                if (allocate_device_buffer(sbuf)) {
                    fprintf(stderr, "Error allocating cuda memory\n");
                    return 1;
                }

                if (allocate_device_buffer(rbuf)) {
                    fprintf(stderr, "Error allocating cuda memory\n");
                    return 1;
                }
            } else if ('M' == options.src) {
                if (allocate_managed_buffer_size(sbuf, size)) {
                    fprintf(stderr, "Error allocating cuda unified memory\n");
                    return 1;
                }

                if (allocate_managed_buffer_size(rbuf, size)) {
                    fprintf(stderr, "Error allocating cuda unified memory\n");
                    return 1;
                }
            } else {
                if (posix_memalign((void **)sbuf, align_size, size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }

                if (posix_memalign((void **)rbuf, align_size, size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }
            }
            break;
        case 1:
            if ('D' == options.dst) {
                if (allocate_device_buffer(sbuf)) {
                    fprintf(stderr, "Error allocating cuda memory\n");
                    return 1;
                }

                if (allocate_device_buffer(rbuf)) {
                    fprintf(stderr, "Error allocating cuda memory\n");
                    return 1;
                }
            } else if ('M' == options.dst) {
                if (allocate_managed_buffer_size(sbuf, size)) {
                    fprintf(stderr, "Error allocating cuda unified memory\n");
                    return 1;
                }

                if (allocate_managed_buffer_size(rbuf, size)) {
                    fprintf(stderr, "Error allocating cuda unified memory\n");
                    return 1;
                }
            } else {
                if (posix_memalign((void **)sbuf, align_size, size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }

                if (posix_memalign((void **)rbuf, align_size, size)) {
                    fprintf(stderr, "Error allocating host memory\n");
                    return 1;
                }
            }
            break;
    }

    return 0;
}

void allocate_memory_one_sided(int rank, char **user_buf, char **win_base,
                               size_t size, enum WINDOW type, MPI_Win *win)
{
    int page_size;
    int purehost = 0;
    int mem_on_dev = 0;

    page_size = getpagesize();
    assert(page_size <= MAX_ALIGNMENT);

    if ('H' == options.src && 'H' == options.dst) {
        purehost = 1;
    }

    if (0 == rank) {
        /* always allocate device buffers explicitly since most MPI libraries do
         * not support allocating device buffers during window creation */
        if ('H' != options.src) {
            CHECK(
                allocate_device_buffer_one_sided(user_buf, size, options.src));
            set_device_memory(*user_buf, 'a', size);
            CHECK(
                allocate_device_buffer_one_sided(win_base, size, options.src));
            set_device_memory(*win_base, 'a', size);
        } else {
            CHECK(posix_memalign((void **)user_buf, page_size, size));
            memset(*user_buf, 'a', size);
            if (type != WIN_ALLOCATE || !purehost) {
                CHECK(posix_memalign((void **)win_base, page_size, size));
                memset(*win_base, 'a', size);
            }
        }
    } else {
        /* always allocate device buffers explicitly since most MPI libraries do
         * not support allocating device buffers during window creation */
        if ('H' != options.dst) {
            CHECK(
                allocate_device_buffer_one_sided(user_buf, size, options.dst));
            set_device_memory(*user_buf, 'a', size);
            CHECK(
                allocate_device_buffer_one_sided(win_base, size, options.dst));
            set_device_memory(*win_base, 'a', size);
        } else {
            CHECK(posix_memalign((void **)user_buf, page_size, size));
            memset(*user_buf, 'a', size);
            if (type != WIN_ALLOCATE || !purehost) {
                CHECK(posix_memalign((void **)win_base, page_size, size));
                memset(*win_base, 'a', size);
            }
        }
    }

#if MPI_VERSION >= 3
    MPI_Status reqstat;

    switch (type) {
        case WIN_CREATE:
            MPI_CHECK(MPI_Win_create(*win_base, size, 1, MPI_INFO_NULL,
                                     MPI_COMM_WORLD, win));
            break;
        case WIN_DYNAMIC:
            MPI_CHECK(
                MPI_Win_create_dynamic(MPI_INFO_NULL, MPI_COMM_WORLD, win));
            MPI_CHECK(MPI_Win_attach(*win, (void *)*win_base, size));
            MPI_CHECK(MPI_Get_address(*win_base, &disp_local));
            if (rank == 0) {
                MPI_CHECK(
                    MPI_Send(&disp_local, 1, MPI_AINT, 1, 1, MPI_COMM_WORLD));
                MPI_CHECK(MPI_Recv(&disp_remote, 1, MPI_AINT, 1, 1,
                                   MPI_COMM_WORLD, &reqstat));
            } else {
                MPI_CHECK(MPI_Recv(&disp_remote, 1, MPI_AINT, 0, 1,
                                   MPI_COMM_WORLD, &reqstat));
                MPI_CHECK(
                    MPI_Send(&disp_local, 1, MPI_AINT, 0, 1, MPI_COMM_WORLD));
            }
            break;
        default:
            if (purehost) {
                MPI_CHECK(MPI_Win_allocate(size, 1, MPI_INFO_NULL,
                                           MPI_COMM_WORLD, (void *)win_base,
                                           win));
            } else {
                MPI_CHECK(MPI_Win_create(*win_base, size, 1, MPI_INFO_NULL,
                                         MPI_COMM_WORLD, win));
            }
            break;
    }
#else
    MPI_CHECK(
        MPI_Win_create(*win_base, size, 1, MPI_INFO_NULL, MPI_COMM_WORLD, win));
#endif
}

size_t omb_ddt_assign(MPI_Datatype *datatype, MPI_Datatype base_datatype,
                      size_t count)
{
    /* Since all the benchmarks that supports ddt currently use char, count is
     * equal to char. This should be modified in the future if ddt support
     * for other bencharks are included.
     */
    size_t transmit_size = 0;
    FILE *fp = NULL;
    char line[OMB_DDT_FILE_LINE_MAX_LENGTH];
    char *token;
    int i = 0;
    int block_lengths[OMB_DDT_INDEXED_MAX_LENGTH] = {0};
    int displacements[OMB_DDT_INDEXED_MAX_LENGTH] = {0};

    if (0 == options.omb_enable_ddt) {
        return count;
    }
    switch (options.ddt_type) {
        case OMB_DDT_CONTIGUOUS:
            MPI_CHECK(MPI_Type_contiguous(count, base_datatype, datatype));
            MPI_CHECK(MPI_Type_commit(datatype));
            transmit_size = count;
            break;
        case OMB_DDT_VECTOR:
            MPI_CHECK(MPI_Type_vector(
                count / options.ddt_type_parameters.stride,
                options.ddt_type_parameters.block_length,
                options.ddt_type_parameters.stride, base_datatype, datatype));
            MPI_CHECK(MPI_Type_commit(datatype));
            transmit_size = (count / options.ddt_type_parameters.stride) *
                            options.ddt_type_parameters.block_length;
            break;
        case OMB_DDT_INDEXED:
            fp = fopen(options.ddt_type_parameters.filepath, "r");
            OMB_CHECK_NULL_AND_EXIT(fp, "Unable to open ddt indexed file.\n");
            transmit_size = 0;
            while (fgets(line, OMB_DDT_FILE_LINE_MAX_LENGTH, fp)) {
                if ('#' == line[0]) {
                    continue;
                }
                token = strtok(line, ",");
                if (NULL != token) {
                    displacements[i] = atoi(token);
                }
                token = strtok(NULL, ",");
                if (NULL != token) {
                    block_lengths[i] = atoi(token);
                    transmit_size += block_lengths[i];
                }
                i++;
            }
            fclose(fp);
            MPI_CHECK(MPI_Type_indexed(i - 1, block_lengths, displacements,
                                       base_datatype, datatype));
            MPI_CHECK(MPI_Type_commit(datatype));
            break;
    }
    return transmit_size;
}

void omb_ddt_free(MPI_Datatype *datatype)
{
    if (0 == options.omb_enable_ddt) {
        return;
    }
    OMB_CHECK_NULL_AND_EXIT(datatype, "Received NULL datatype");
    MPI_CHECK(MPI_Type_free(datatype));
}

size_t omb_ddt_get_size(size_t size)
{
    if (0 == options.omb_enable_ddt) {
        return size;
    }
    return 1;
}

void free_buffer(void *buffer, enum accel_type type)
{
    switch (type) {
        case NONE:
            free(buffer);
            break;
        case MANAGED:
        case CUDA:
#ifdef _ENABLE_CUDA_
            CUDA_CHECK(cudaFree(buffer));
#endif
            break;
        case OPENACC:
#ifdef _ENABLE_OPENACC_
            acc_free(buffer);
#endif
            break;
        case ROCM:
#ifdef _ENABLE_ROCM_
            ROCM_CHECK(hipFree(buffer));
#endif
        case SYCL:
#ifdef _ENABLE_SYCL_
            syclFree(buffer);
#endif
            break;
    }

    if (GPU == options.target || BOTH == options.target) {
#ifdef _ENABLE_CUDA_KERNEL_
        free_device_arrays();
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
    }
}

#if defined(_ENABLE_OPENACC_) || defined(_ENABLE_CUDA_) ||                     \
    defined(_ENABLE_ROCM_) || defined(_ENABLE_SYCL_)
int omb_get_local_rank()
{
    char *str = NULL;
    int local_rank = -1;

    if ((str = getenv("MV2_COMM_WORLD_LOCAL_RANK")) != NULL) {
        local_rank = atoi(str);
    } else if ((str = getenv("MVP_COMM_WORLD_LOCAL_RANK")) != NULL) {
        local_rank = atoi(str);
    } else if ((str = getenv("OMPI_COMM_WORLD_LOCAL_RANK")) != NULL) {
        local_rank = atoi(str);
    } else if ((str = getenv("MPI_LOCALRANKID")) != NULL) {
        local_rank = atoi(str);
    } else if ((str = getenv("SLURM_PROCID")) != NULL) {
        local_rank = atoi(str);
    } else if ((str = getenv("LOCAL_RANK")) != NULL) {
        local_rank = atoi(str);
    } else {
        fprintf(
            stderr,
            "Warning: OMB could not identify the local rank of the process.\n");
        fprintf(stderr, "         This can lead to multiple processes using "
                        "the same GPU.\n");
        fprintf(stderr, "         Please use the get_local_rank script in the "
                        "OMB repo for this.\n");
    }

    return local_rank;
}
#endif /* defined(_ENABLE_OPENACC_) || defined(_ENABLE_CUDA_) ||               \
          defined(_ENABLE_ROCM_) || defined(_ENABLE_SYCL_) */

int init_accel(void)
{
#ifdef _ENABLE_CUDA_
    CUresult curesult = CUDA_SUCCESS;
    CUdevice cuDevice;
#endif
#if defined(_ENABLE_OPENACC_) || defined(_ENABLE_CUDA_) ||                     \
    defined(_ENABLE_ROCM_) || defined(_ENABLE_SYCL_)
    int local_rank = -1, dev_count = 0;
    int dev_id = 0;

    local_rank = omb_get_local_rank();
#endif

    switch (options.accel) {
#ifdef _ENABLE_CUDA_KERNEL_
        case MANAGED:
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
#ifdef _ENABLE_CUDA_
        case CUDA:
            if (local_rank >= 0) {
                CUDA_CHECK(cudaGetDeviceCount(&dev_count));
                dev_id = local_rank % dev_count;
            }
            CUDA_CHECK(cudaSetDevice(dev_id));

            curesult = cuInit(0);
            if (curesult != CUDA_SUCCESS) {
                return 1;
            }

            curesult = cuDeviceGet(&cuDevice, dev_id);
            if (curesult != CUDA_SUCCESS) {
                return 1;
            }

            curesult = cuDevicePrimaryCtxRetain(&cuContext, cuDevice);
            if (curesult != CUDA_SUCCESS) {
                return 1;
            }

#ifdef _ENABLE_CUDA_KERNEL_
            create_cuda_stream();
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
            break;
#endif
#ifdef _ENABLE_OPENACC_
        case OPENACC:
            if (local_rank >= 0) {
                dev_count = acc_get_num_devices(acc_device_not_host);
                assert(dev_count > 0);
                dev_id = local_rank % dev_count;
            }

            acc_set_device_num(dev_id, acc_device_not_host);
            break;
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            if (local_rank >= 0) {
                ROCM_CHECK(hipGetDeviceCount(&dev_count));
                dev_id = local_rank % dev_count;
            }
            ROCM_CHECK(hipSetDevice(dev_id));
            break;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL:
            if (local_rank >= 0) {
                syclGetDeviceCount(&dev_count);
                dev_id = local_rank % dev_count;
            }
            syclSetDevice(dev_id);
            break;
#endif
        default:
            fprintf(stderr,
                    "Invalid device type, should be cuda, openacc, or rocm. "
                    "Check configure time options to verify that support for "
                    "chosen "
                    "device type is enabled.\n");
            return 1;
    }

    return 0;
}

int cleanup_accel(void)
{
#ifdef _ENABLE_CUDA_
    CUresult curesult = CUDA_SUCCESS;
#endif

    switch (options.accel) {
#ifdef _ENABLE_CUDA_KERNEL_
        case MANAGED:
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
#ifdef _ENABLE_CUDA_
        case CUDA:
            /* Reset the device to release all resources */
#ifdef _ENABLE_CUDA_KERNEL_
            destroy_cuda_stream();
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
            CUDA_CHECK(cudaDeviceReset());
            break;
#endif
#ifdef _ENABLE_OPENACC_
        case OPENACC:
            acc_shutdown(acc_device_nvidia);
            break;
#endif
#ifdef _ENABLE_ROCM_
        case ROCM:
            ROCM_CHECK(hipDeviceReset());
            break;
#endif
#ifdef _ENABLE_SYCL_
        case SYCL:
            syclDeviceReset();
            break;
#endif
        default:
            fprintf(stderr, "Invalid accel type, should be cuda or openacc\n");
            return 1;
    }

    return 0;
}

#ifdef _ENABLE_CUDA_KERNEL_
void free_device_arrays()
{
    if (is_alloc) {
        CUDA_CHECK(cudaFree(d_x));
        CUDA_CHECK(cudaFree(d_y));

        is_alloc = 0;
    }
}
#endif

void free_memory(void *sbuf, void *rbuf, int rank)
{
    switch (rank) {
        case 0:
            if ('D' == options.src || 'M' == options.src) {
                free_device_buffer(sbuf);
                free_device_buffer(rbuf);
            } else {
                if (sbuf) {
                    free(sbuf);
                }
                if (rbuf) {
                    free(rbuf);
                }
            }
            break;
        case 1:
            if ('D' == options.dst || 'M' == options.dst) {
                free_device_buffer(sbuf);
                free_device_buffer(rbuf);
            } else {
                if (sbuf) {
                    free(sbuf);
                }
                if (rbuf) {
                    free(rbuf);
                }
            }
            break;
    }
}

void free_memory_pt2pt_mul(void *sbuf, void *rbuf, int rank, int pairs)
{
    if (rank < pairs) {
        if ('D' == options.src || 'M' == options.src) {
            free_device_buffer(sbuf);
            free_device_buffer(rbuf);
        } else {
            free(sbuf);
            free(rbuf);
        }
    } else {
        if ('D' == options.dst || 'M' == options.dst) {
            free_device_buffer(sbuf);
            free_device_buffer(rbuf);
        } else {
            free(sbuf);
            free(rbuf);
        }
    }
}

void free_memory_one_sided(void *user_buf, void *win_baseptr,
                           enum WINDOW win_type, MPI_Win win, int rank)
{
    int purehost = 0;

    if ('H' == options.src && 'H' == options.dst) {
        purehost = 1;
    }
    MPI_CHECK(MPI_Win_free(&win));
    /* if MPI_Win_allocate is specified, win_baseptr would be freed by
     * MPI_Win_free, so only need to free the user_buf */
    if (win_type != WIN_ALLOCATE || !purehost) {
        free_memory(user_buf, win_baseptr, rank);
    } else {
        free_memory(user_buf, NULL, rank);
    }
}

double dummy_compute(double seconds, MPI_Request *request)
{
    double test_time = 0.0;

    test_time = do_compute_and_probe(seconds, request);

    return test_time;
}

void omb_scatter_offset_copy(void *buf, int root_rank, size_t size)
{
    switch (options.accel) {
        case NONE:
            memcpy(buf, buf + root_rank * size, size);
            break;
        case CUDA:
        case MANAGED:
#ifdef _ENABLE_CUDA_
        {
            CUDA_CHECK(cudaMemcpy((void *)buf, (void *)buf + root_rank * size,
                                  size, cudaMemcpyDeviceToDevice));
            CUDA_CHECK(cudaDeviceSynchronize());
        }
#endif
        case ROCM:
#ifdef _ENABLE_ROCM_
        {
            ROCM_CHECK(hipMemcpy((void *)buf, (void *)buf + root_rank * size,
                                 size, hipMemcpyDeviceToDevice));
            ROCM_CHECK(hipDeviceSynchronize());
        }
#endif
        case SYCL:
#ifdef _ENABLE_SYCL_
        {
            syclMemcpy((void *)buf, (void *)buf + root_rank * size, size);
            syclDeviceSynchronize();
        }
#endif
        break;
            break;
        default:
            break;
    }
}

#ifdef _ENABLE_CUDA_KERNEL_
void create_cuda_stream() { CUDA_CHECK(cudaStreamCreate(&um_stream)); }

void destroy_cuda_stream() { CUDA_CHECK(cudaStreamDestroy(um_stream)); }

void create_cuda_event()
{
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
}

void destroy_cuda_event()
{
    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));
}

void event_record_start() { CUDA_CHECK(cudaEventRecord(start, um_stream)); }

void event_record_stop() { CUDA_CHECK(cudaEventRecord(stop, um_stream)); }

void event_elapsed_time(float *t_elapsed)
{
    CUDA_CHECK(cudaEventSynchronize(stop));
    CUDA_CHECK(cudaEventElapsedTime(t_elapsed, start, stop));
}

void synchronize_device() { CUDA_CHECK(cudaDeviceSynchronize()); }

void synchronize_stream() { CUDA_CHECK(cudaStreamSynchronize(um_stream)); }

void prefetch_data(char *buf, size_t length, int devid)
{
#if CUDART_VERSION >= 13000
    struct cudaMemLocation loc;
    loc.id = devid;
    loc.type = cudaMemLocationTypeDevice;
    unsigned int flags = 0;

    CUDA_CHECK(cudaMemPrefetchAsync(buf, length, loc, flags, um_stream));
#else
    CUDA_CHECK(cudaMemPrefetchAsync(buf, length, devid, um_stream));
#endif
}

void touch_managed(char *buf, size_t length, enum op_type type)
{
    if (ADD == type) {
        call_touch_managed_kernel_add(buf, length, &um_stream);
    } else if (SUB == type) {
        call_touch_managed_kernel_sub(buf, length, &um_stream);
    }
}

void launch_empty_kernel(char *buf, size_t length)
{
    call_empty_kernel(buf, length, &um_stream);
}

void do_compute_gpu(double seconds)
{
    double time_elapsed = 0.0, t1 = 0.0, t2 = 0.0;

    {
        t1 = MPI_Wtime();

        /* Execute Dummy Kernel on GPU if set by user */
        if (options.target == BOTH || options.target == GPU) {
            {
                CUDA_CHECK(cudaStreamCreate(&stream));
                call_kernel(A, d_x, d_y, options.device_array_size, &stream);
            }
        }

        t2 = MPI_Wtime();
        time_elapsed += (t2 - t1);
    }
}
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

void compute_on_host()
{
    int i = 0, j = 0;
    for (i = 0; i < DIM; i++)
        for (j = 0; j < DIM; j++)
            x[i] = x[i] + a[i][j] * a[j][i] + y[j];
}

static inline void do_compute_cpu(double target_seconds)
{
    double t1 = 0.0, t2 = 0.0;
    double time_elapsed = 0.0;
    while (time_elapsed < target_seconds) {
        t1 = MPI_Wtime();
        compute_on_host();
        t2 = MPI_Wtime();
        time_elapsed += (t2 - t1);
    }
    if (DEBUG) {
        fprintf(stderr, "time elapsed = %f\n", (time_elapsed * 1e6));
    }
}

double do_compute_and_probe(double seconds, MPI_Request *request)
{
    double t1 = 0.0, t2 = 0.0;
    double test_time = 0.0;
    int num_tests = 0;
    double target_seconds_for_compute = 0.0;
    int flag = 0;
    MPI_Status status;

    if (options.num_probes) {
        target_seconds_for_compute = (double)seconds / options.num_probes;
        if (DEBUG) {
            fprintf(stderr, "setting target seconds to %f\n",
                    (target_seconds_for_compute * 1e6));
        }
    } else {
        target_seconds_for_compute = seconds;
        if (DEBUG) {
            fprintf(stderr, "setting target seconds to %f\n",
                    (target_seconds_for_compute * 1e6));
        }
    }

#ifdef _ENABLE_CUDA_KERNEL_
    if (options.target == GPU) {
        if (options.num_probes) {
            /* Do the dummy compute on GPU only */
            do_compute_gpu(target_seconds_for_compute);
            num_tests = 0;
            while (num_tests < options.num_probes) {
                t1 = MPI_Wtime();
                MPI_CHECK(MPI_Test(request, &flag, &status));
                t2 = MPI_Wtime();
                test_time += (t2 - t1);
                num_tests++;
            }
        } else {
            do_compute_gpu(target_seconds_for_compute);
        }
    } else if (options.target == BOTH) {
        if (options.num_probes) {
            /* Do the dummy compute on GPU and CPU*/
            do_compute_gpu(target_seconds_for_compute);
            num_tests = 0;
            while (num_tests < options.num_probes) {
                t1 = MPI_Wtime();
                MPI_CHECK(MPI_Test(request, &flag, &status));
                t2 = MPI_Wtime();
                test_time += (t2 - t1);
                num_tests++;
                do_compute_cpu(target_seconds_for_compute);
            }
        } else {
            do_compute_gpu(target_seconds_for_compute);
            do_compute_cpu(target_seconds_for_compute);
        }
    } else
#endif
        if (options.target == CPU) {
        if (options.num_probes) {
            num_tests = 0;
            while (num_tests < options.num_probes) {
                do_compute_cpu(target_seconds_for_compute);
                t1 = MPI_Wtime();
                MPI_CHECK(MPI_Test(request, &flag, &status));
                t2 = MPI_Wtime();
                test_time += (t2 - t1);
                num_tests++;
            }
        } else {
            do_compute_cpu(target_seconds_for_compute);
        }
    }

#ifdef _ENABLE_CUDA_KERNEL_
    if (options.target == GPU || options.target == BOTH) {
        CUDA_CHECK(cudaDeviceSynchronize());
        CUDA_CHECK(cudaStreamDestroy(stream));
    }
#endif

    return test_time;
}

void allocate_host_arrays()
{
    int i = 0, j = 0;
    for (i = 0; i < DIM; i++) {
        x[i] = y[i] = 1.0f;
        for (j = 0; j < DIM; j++) {
            a[i][j] = 2.0f;
        }
    }
}

void allocate_atomic_memory(int rank, char **sbuf, char **tbuf, char **cbuf,
                            char **win_base, size_t size, enum WINDOW type,
                            MPI_Win *win)
{
    int page_size;
    int purehost = 0;
    int mem_on_dev = 0;

    page_size = getpagesize();
    assert(page_size <= MAX_ALIGNMENT);

    if ('H' == options.src && 'H' == options.dst) {
        purehost = 1;
    }
    if (0 == rank) {
        if ('H' != options.src) {
            CHECK(allocate_device_buffer_one_sided(sbuf, size, options.src));
            set_device_memory(*sbuf, 'a', size);
            CHECK(
                allocate_device_buffer_one_sided(win_base, size, options.src));
            set_device_memory(*win_base, 'b', size);
            CHECK(allocate_device_buffer_one_sided(tbuf, size, options.src));
            set_device_memory(*tbuf, 'c', size);
            if (cbuf != NULL) {
                CHECK(
                    allocate_device_buffer_one_sided(cbuf, size, options.src));
                set_device_memory(*cbuf, 'a', size);
            }
        } else {
            CHECK(posix_memalign((void **)sbuf, page_size, size));
            memset(*sbuf, 'a', size);
            CHECK(posix_memalign((void **)win_base, page_size, size));
            memset(*win_base, 'b', size);
            CHECK(posix_memalign((void **)tbuf, page_size, size));
            memset(*tbuf, 'c', size);
            if (cbuf != NULL) {
                CHECK(posix_memalign((void **)cbuf, page_size, size));
                memset(*cbuf, 'a', size);
            }
        }
    } else {
        if ('H' != options.dst) {
            CHECK(allocate_device_buffer_one_sided(sbuf, size, options.dst));
            set_device_memory(*sbuf, 'a', size);
            CHECK(
                allocate_device_buffer_one_sided(win_base, size, options.dst));
            set_device_memory(*win_base, 'b', size);
            CHECK(allocate_device_buffer_one_sided(tbuf, size, options.dst));
            set_device_memory(*tbuf, 'c', size);
            if (cbuf != NULL) {
                CHECK(
                    allocate_device_buffer_one_sided(cbuf, size, options.dst));
                set_device_memory(*cbuf, 'a', size);
            }
        } else {
            CHECK(posix_memalign((void **)sbuf, page_size, size));
            memset(*sbuf, 'a', size);
            CHECK(posix_memalign((void **)win_base, page_size, size));
            memset(*win_base, 'b', size);
            CHECK(posix_memalign((void **)tbuf, page_size, size));
            memset(*tbuf, 'c', size);
            if (cbuf != NULL) {
                CHECK(posix_memalign((void **)cbuf, page_size, size));
                memset(*cbuf, 'a', size);
            }
        }
    }

#if MPI_VERSION >= 3
    MPI_Status reqstat;

    switch (type) {
        case WIN_CREATE:
            MPI_CHECK(MPI_Win_create(*win_base, size, 1, MPI_INFO_NULL,
                                     MPI_COMM_WORLD, win));
            break;
        case WIN_DYNAMIC:
            MPI_CHECK(
                MPI_Win_create_dynamic(MPI_INFO_NULL, MPI_COMM_WORLD, win));
            MPI_CHECK(MPI_Win_attach(*win, (void *)*win_base, size));
            MPI_CHECK(MPI_Get_address(*win_base, &disp_local));
            if (rank == 0) {
                MPI_CHECK(
                    MPI_Send(&disp_local, 1, MPI_AINT, 1, 1, MPI_COMM_WORLD));
                MPI_CHECK(MPI_Recv(&disp_remote, 1, MPI_AINT, 1, 1,
                                   MPI_COMM_WORLD, &reqstat));
            } else {
                MPI_CHECK(MPI_Recv(&disp_remote, 1, MPI_AINT, 0, 1,
                                   MPI_COMM_WORLD, &reqstat));
                MPI_CHECK(
                    MPI_Send(&disp_local, 1, MPI_AINT, 0, 1, MPI_COMM_WORLD));
            }
            break;
        default:
            if (purehost) {
                MPI_CHECK(MPI_Win_allocate(size, 1, MPI_INFO_NULL,
                                           MPI_COMM_WORLD, (void *)win_base,
                                           win));
            } else {
                MPI_CHECK(MPI_Win_create(*win_base, size, 1, MPI_INFO_NULL,
                                         MPI_COMM_WORLD, win));
            }
            break;
    }
#else
    MPI_CHECK(
        MPI_Win_create(*win_base, size, 1, MPI_INFO_NULL, MPI_COMM_WORLD, win));
#endif
}

void free_atomic_memory(void *sbuf, void *win_baseptr, void *tbuf, void *cbuf,
                        enum WINDOW win_type, MPI_Win win, int rank)
{
    int mem_on_dev = 0;
    MPI_CHECK(MPI_Win_free(&win));

    if (0 == rank) {
        if ('H' != options.src) {
            free_device_buffer(sbuf);
            free_device_buffer(win_baseptr);
            free_device_buffer(tbuf);
            if (NULL != cbuf) {
                free_device_buffer(cbuf);
            }
        } else {
            free(sbuf);
            if (NULL == win_baseptr) {
                free(win_baseptr);
            }
            free(tbuf);
            if (NULL != cbuf) {
                free(cbuf);
            }
        }
    } else {
        if ('H' != options.dst) {
            free_device_buffer(sbuf);
            free_device_buffer(win_baseptr);
            free_device_buffer(tbuf);
            if (NULL != cbuf) {
                free_device_buffer(cbuf);
            }
        } else {
            free(sbuf);
            if (NULL == win_baseptr) {
                free(win_baseptr);
            }
            free(tbuf);
            if (NULL != cbuf) {
                free(cbuf);
            }
        }
    }
}

void init_arrays(double target_time)
{
    if (DEBUG) {
        fprintf(stderr, "called init_arrays with target_time = %f \n",
                (target_time * 1e6));
    }

#ifdef _ENABLE_CUDA_KERNEL_
    if (options.target == GPU || options.target == BOTH) {
        /* Setting size of arrays for Dummy Compute */
        int N = options.device_array_size;

        /* Device Arrays for Dummy Compute */
        allocate_device_arrays(N);

        double t1 = 0.0, t2 = 0.0;

        while (1) {
            t1 = MPI_Wtime();

            if (options.target == GPU || options.target == BOTH) {
                CUDA_CHECK(cudaStreamCreate(&stream));
                call_kernel(A, d_x, d_y, N, &stream);

                CUDA_CHECK(cudaDeviceSynchronize());
                CUDA_CHECK(cudaStreamDestroy(stream));
            }

            t2 = MPI_Wtime();
            if ((t2 - t1) < target_time) {
                N += 32;

                /* Now allocate arrays of size N */
                allocate_device_arrays(N);
            } else {
                break;
            }
        }

        /* we reach here with desired N so save it and pass it to options */
        options.device_array_size = N;
        if (DEBUG) {
            fprintf(stderr, "correct N = %d\n", N);
        }
    }
#endif
}

#ifdef _ENABLE_CUDA_KERNEL_
void allocate_device_arrays(int n)
{
    /* First free the old arrays */
    free_device_arrays();

    /* Allocate Device Arrays for Dummy Compute */
    CUDA_CHECK(cudaMalloc((void **)&d_x, n * sizeof(float)));

    CUDA_CHECK(cudaMalloc((void **)&d_y, n * sizeof(float)));

    CUDA_CHECK(cudaMemset(d_x, 1.0f, n));
    CUDA_CHECK(cudaMemset(d_y, 2.0f, n));
    is_alloc = 1;
}

double measure_kernel_lo_window(char **buf, int size, int window_size)
{
    int i = 0;
    double t_lo = 0.0, t_start = 0.0, t_end = 0.0;

    for (i = 0; i < 10; i++) {
        launch_empty_kernel(buf[i % window_size], size); // Warmup
    }

    for (i = 0; i < 1000; i++) {
        t_start = MPI_Wtime();
        launch_empty_kernel(buf[i % window_size], size);
        synchronize_stream();
        t_end = MPI_Wtime();
        t_lo = t_lo + (t_end - t_start);
    }

    t_lo = t_lo / 1000; // Averaging the kernel launch overhead
    return t_lo;
}

double measure_kernel_lo_no_window(char *buf, int size)
{
    int i = 0;
    double t_lo = 0.0, t_start = 0.0, t_end = 0.0;

    for (i = 0; i < 10; i++) {
        launch_empty_kernel(buf, size);
    }

    for (i = 0; i < 1000; i++) {
        t_start = MPI_Wtime();
        launch_empty_kernel(buf, size);
        synchronize_stream();
        t_end = MPI_Wtime();
        t_lo = t_lo + (t_end - t_start);
    }

    t_lo = t_lo / 1000;
    return t_lo;
}

void touch_managed_src_window(char **buf, int size, int window_size,
                              enum op_type type)
{
    int j = 0;

    if (options.src == 'M') {
        if (options.MMsrc == 'D') {
            for (j = 0; j < window_size; j++) {
                touch_managed(buf[j], size, type);
                synchronize_stream();
            }
        } else if ((options.MMsrc == 'H') && size > PREFETCH_THRESHOLD) {
            for (j = 0; j < window_size; j++) {
                prefetch_data(buf[j], size, -1);
                synchronize_stream();
            }
        } else {
            if (!options.validate) {
                for (j = 0; j < window_size; j++) {
                    memset(buf[j], 'c', size);
                }
            }
        }
    }
}

void touch_managed_dst_window(char **buf, int size, int window_size,
                              enum op_type type)
{
    int j = 0;

    if (options.dst == 'M') {
        if (options.MMdst == 'D') {
            for (j = 0; j < window_size; j++) {
                touch_managed(buf[j], size, type);
                synchronize_stream();
            }
        } else if ((options.MMdst == 'H') && size > PREFETCH_THRESHOLD) {
            for (j = 0; j < window_size; j++) {
                prefetch_data(buf[j], size, -1);
                synchronize_stream();
            }
        } else {
            if (!options.validate) {
                for (j = 0; j < window_size; j++) {
                    memset(buf[j], 'c', size);
                }
            }
        }
    }
}

void touch_managed_src_no_window(char *buf, int size, enum op_type type)
{
    if (options.src == 'M') {
        if (options.MMsrc == 'D') {
            touch_managed(buf, size, type);
            synchronize_stream();
        } else if ((options.MMsrc == 'H') && size > PREFETCH_THRESHOLD) {
            prefetch_data(buf, size, cudaCpuDeviceId);
            synchronize_stream();
        } else {
            if (!options.validate) {
                memset(buf, 'c', size);
            }
        }
    }
}

void touch_managed_dst_no_window(char *buf, int size, enum op_type type)
{
    if (options.dst == 'M') {
        if (options.MMdst == 'D') {
            touch_managed(buf, size, type);
            synchronize_stream();
        } else if ((options.MMdst == 'H') && size > PREFETCH_THRESHOLD) {
            prefetch_data(buf, size, -1);
            synchronize_stream();
        } else {
            if (!options.validate) {
                memset(buf, 'c', size);
            }
        }
    }
}
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

/* vi:set sw=4 sts=4 tw=80: */
