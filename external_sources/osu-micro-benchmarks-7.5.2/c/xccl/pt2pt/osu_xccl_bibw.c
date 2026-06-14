#define BENCHMARK "OSU " OMB_XCCL_TYPE_STR "%s Bi-Directional Bandwidth Test"
/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "osu_util_xccl_interface.h"

int main(int argc, char *argv[])
{
    omb_xccl_int_t *omb_xccl_interface = omb_xccl_interface_inject();
    int myid = 0, numprocs = 0, i = 0, j = 0;
    int size = 0;
    char *s_buf = NULL, *r_buf = NULL;
    double t_start = 0.0, t_end = 0.0, t = 0.0, t_total = 0.0;
    int window_size = 64;
    int po_ret = 0;
    double tmp = 0;
    int peer = 0;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    options.bench = PT2PT;
    options.subtype = BW;

    set_header(HEADER);
    set_benchmark_name("osu_xccl_bibw");

    po_ret = process_options(argc, argv);
    window_size = options.window_size;

    if (PO_OKAY == po_ret) {
        if (OMB_XCCL_ACC_TYPE != options.accel) {
            omb_force_accelerator();
        }
        if (init_accel()) {
            fprintf(stderr, "Error initializing device\n");
            exit(EXIT_FAILURE);
        }
    }

    MPI_CHECK(MPI_Init(&argc, &argv));
    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &myid));

    peer = 0 == myid ? 1 : 0;

    if (0 == myid) {
        switch (po_ret) {
            case PO_CUDA_NOT_AVAIL:
                fprintf(stderr, "CUDA support not enabled.  Please recompile "
                                "benchmark with CUDA support.\n");
                break;
            case PO_OPENACC_NOT_AVAIL:
                fprintf(stderr, "OPENACC support not enabled.  Please "
                                "recompile benchmark with OPENACC support.\n");
                break;
            case PO_BAD_USAGE:
                print_bad_usage_message(myid);
                break;
            case PO_HELP_MESSAGE:
                print_help_message(myid);
                break;
            case PO_VERSION_MESSAGE:
                print_version_message(myid);
                MPI_CHECK(MPI_Finalize());
                exit(EXIT_SUCCESS);
            case PO_OKAY:
                break;
        }
    }

    switch (po_ret) {
        case PO_CUDA_NOT_AVAIL:
        case PO_OPENACC_NOT_AVAIL:
        case PO_BAD_USAGE:
            MPI_CHECK(MPI_Finalize());
            exit(EXIT_FAILURE);
        case PO_HELP_MESSAGE:
        case PO_VERSION_MESSAGE:
            MPI_CHECK(MPI_Finalize());
            exit(EXIT_SUCCESS);
        case PO_OKAY:
            break;
    }

    if (numprocs != 2) {
        if (0 == myid) {
            fprintf(stderr, "This test requires exactly two processes\n");
        }

        MPI_CHECK(MPI_Finalize());
        exit(EXIT_FAILURE);
    }

    omb_xccl_interface->allocate_xccl_stream();
    omb_xccl_interface->create_xccl_comm(numprocs, myid);

    if (allocate_memory_pt2pt(&s_buf, &r_buf, myid)) {
        /* Error allocating memory */
        MPI_CHECK(MPI_Finalize());
        exit(EXIT_FAILURE);
    }
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }

    print_header(myid, BW);

    /* Bi-Directional Bandwidth test */
    for (size = options.min_message_size; size <= options.max_message_size;
         size *= 2) {
        /* touch the data */
        t_total = 0.0;
        set_buffer_pt2pt(s_buf, myid, options.accel, 'a', size);
        set_buffer_pt2pt(r_buf, myid, options.accel, 'b', size);

        if (size > LARGE_MESSAGE_SIZE) {
            options.iterations = options.iterations_large;
            options.skip = options.skip_large;
        }

        for (i = 0; i < options.iterations + options.skip; i++) {
            if (i >= options.skip) {
                t_start = MPI_Wtime();
            }
            omb_xccl_interface->ncclGroupStart();
            for (j = 0; j < window_size; j++) {
                NCCL_CHECK(omb_xccl_interface->ncclSend(
                    s_buf, size, ncclChar, peer, nccl_comm, nccl_stream));
                NCCL_CHECK(omb_xccl_interface->ncclRecv(
                    r_buf, size, ncclChar, peer, nccl_comm, nccl_stream));
            }
            omb_xccl_interface->ncclGroupEnd();
            omb_xccl_interface->synchronize_xccl_stream(nccl_stream);
            if (i >= options.skip) {
                t_end = MPI_Wtime();
                t_total += t_end - t_start;
                if (options.omb_tail_lat) {
                    omb_lat_arr[i - options.skip] =
                        (size / 1e6 * window_size * 2) / (t_end - t_start);
                }
            }
        }

        if (0 == myid) {
            tmp = size / 1e6 * options.iterations * window_size * 2;
            fprintf(stdout, "%-*d%*.*f", 10, size, FIELD_WIDTH, FLOAT_PRECISION,
                    tmp / t_total);
            if (options.omb_tail_lat) {
                omb_stat = omb_calculate_tail_lat(omb_lat_arr, myid, 1);
                OMB_ITR_PRINT_STAT(omb_stat.res_arr);
            }
            fprintf(stdout, "\n");
            fflush(stdout);
        }
    }

    free_memory(s_buf, r_buf, myid);
    free(omb_lat_arr);
    omb_xccl_interface->deallocate_xccl_stream();
    omb_xccl_interface->destroy_xccl_comm();
    MPI_CHECK(MPI_Finalize());

    if (NONE != options.accel) {
        if (cleanup_accel()) {
            fprintf(stderr, "Error cleaning up device\n");
            exit(EXIT_FAILURE);
        }
    }
    omb_xccl_interface_free(omb_xccl_interface);
    return EXIT_SUCCESS;
}
