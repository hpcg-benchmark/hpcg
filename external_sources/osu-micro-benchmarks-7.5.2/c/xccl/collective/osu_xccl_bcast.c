#define BENCHMARK "OSU " OMB_XCCL_TYPE_STR "%s Broadcast Latency Test"
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
    int i = 0, rank = 0, size = 0;
    int numprocs = 0;
    double avg_time = 0.0, max_time = 0.0, min_time = 0.0;
    double latency = 0.0, t_start = 0.0, t_stop = 0.0;
    double timer = 0.0;
    char *buffer = NULL;
    int po_ret = 0;
    options.bench = COLLECTIVE;
    options.subtype = LAT;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    set_header(HEADER);
    set_benchmark_name("osu_xccl_bcast");

    po_ret = process_options(argc, argv);

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
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));

    switch (po_ret) {
        case PO_BAD_USAGE:
            print_bad_usage_message(rank);
            MPI_CHECK(MPI_Finalize());
            exit(EXIT_FAILURE);
        case PO_HELP_MESSAGE:
            print_help_message(rank);
            MPI_CHECK(MPI_Finalize());
            exit(EXIT_SUCCESS);
        case PO_VERSION_MESSAGE:
            print_version_message(rank);
            MPI_CHECK(MPI_Finalize());
            exit(EXIT_SUCCESS);
        case PO_OKAY:
            break;
    }

    if (numprocs < 2) {
        if (0 == rank) {
            fprintf(stderr, "This test requires at least two processes\n");
        }

        MPI_CHECK(MPI_Finalize());
        exit(EXIT_FAILURE);
    }

    omb_xccl_interface->allocate_xccl_stream();
    omb_xccl_interface->create_xccl_comm(numprocs, rank);

    if (allocate_memory_coll((void **)&buffer, options.max_message_size,
                             options.accel)) {
        fprintf(stderr, "Could Not Allocate Memory [rank %d]\n", rank);
        MPI_CHECK(MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE));
    }
    set_buffer(buffer, options.accel, 1, options.max_message_size);
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }

    print_preamble(rank);
    print_only_header(rank);

    for (size = options.min_message_size; size <= options.max_message_size;
         size *= 2) {
        if (size > LARGE_MESSAGE_SIZE) {
            options.skip = options.skip_large;
            options.iterations = options.iterations_large;
        }

        MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));

        timer = 0.0;
        for (i = 0; i < options.iterations + options.skip; i++) {
            t_start = MPI_Wtime();
            NCCL_CHECK(omb_xccl_interface->ncclBroadcast(
                buffer, buffer, size, ncclChar, 0, nccl_comm, nccl_stream));
            omb_xccl_interface->synchronize_xccl_stream(nccl_stream);
            t_stop = MPI_Wtime();

            if (i >= options.skip) {
                timer += t_stop - t_start;
                if (options.omb_tail_lat) {
                    omb_lat_arr[i - options.skip] = (t_stop - t_start) * 1e6;
                }
            }
            MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));
        }

        MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));

        latency = (timer * 1e6) / options.iterations;

        MPI_CHECK(MPI_Reduce(&latency, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0,
                             MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(&latency, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0,
                             MPI_COMM_WORLD));
        MPI_CHECK(MPI_Reduce(&latency, &avg_time, 1, MPI_DOUBLE, MPI_SUM, 0,
                             MPI_COMM_WORLD));
        avg_time = avg_time / numprocs;
        omb_stat = omb_get_stats(omb_lat_arr);

        print_stats(rank, size, avg_time, min_time, max_time, omb_stat);
        MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));
    }

    free_buffer(buffer, options.accel);
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

/* vi: set sw=4 sts=4 tw=80: */
