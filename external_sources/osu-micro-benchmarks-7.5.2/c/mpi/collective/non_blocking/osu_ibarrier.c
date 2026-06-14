#define BENCHMARK "OSU MPI%s Non-blocking Barrier Latency Test"
/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include <osu_util_mpi.h>

int main(int argc, char *argv[])
{
    int i = 0, rank, size = 0;
    int numprocs;
    double latency = 0.0, t_start = 0.0, t_stop = 0.0;
    double test_time = 0.0, test_total = 0.0;
    double tcomp = 0.0, tcomp_total = 0.0, latency_in_secs = 0.0;
    double wait_time = 0.0, init_time = 0.0;
    double init_total = 0.0, wait_total = 0.0;
    double timer = 0.0;
    double avg_time = 0.0;
    int po_ret;
    omb_graph_options_t omb_graph_options;
    omb_graph_data_t *omb_graph_data = NULL;
    int papi_eventset = OMB_PAPI_NULL;
    MPI_Comm omb_comm = MPI_COMM_NULL;
    omb_mpi_init_data omb_init_h;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    set_header(HEADER);
    set_benchmark_name("osu_ibarrier");

    options.bench = COLLECTIVE;
    options.subtype = NBC_BARRIER;

    po_ret = process_options(argc, argv);

    if (PO_OKAY == po_ret && NONE != options.accel) {
        if (init_accel()) {
            fprintf(stderr, "Error initializing device\n");
            exit(EXIT_FAILURE);
        }
    }

    options.show_size = 0;

    omb_init_h = omb_mpi_init(&argc, &argv);
    omb_comm = omb_init_h.omb_comm;
    if (MPI_COMM_NULL == omb_comm) {
        OMB_ERROR_EXIT("Cant create communicator");
    }
    MPI_CHECK(MPI_Comm_rank(omb_comm, &rank));
    MPI_CHECK(MPI_Comm_size(omb_comm, &numprocs));
    MPI_Request request;
    MPI_Status status;

    omb_graph_options_init(&omb_graph_options);
    switch (po_ret) {
        case PO_BAD_USAGE:
            print_bad_usage_message(rank);
            omb_mpi_finalize(omb_init_h);
            exit(EXIT_FAILURE);
        case PO_HELP_MESSAGE:
            print_help_message(rank);
            omb_mpi_finalize(omb_init_h);
            exit(EXIT_SUCCESS);
        case PO_VERSION_MESSAGE:
            print_version_message(rank);
            omb_mpi_finalize(omb_init_h);
            exit(EXIT_SUCCESS);
        case PO_OKAY:
            break;
    }

    if (numprocs < 2) {
        if (rank == 0) {
            fprintf(stderr, "This test requires at least two processes\n");
        }

        omb_mpi_finalize(omb_init_h);

        return EXIT_FAILURE;
    }

    print_preamble_nbc(rank);
    print_only_header_nbc(rank);
    omb_papi_init(&papi_eventset);

    options.skip = options.skip_large;
    options.iterations = options.iterations_large;
    timer = 0.0;

    allocate_host_arrays();

    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_options,
                                           1, options.iterations);
    for (i = 0; i < options.iterations + options.skip; i++) {
        if (i == options.skip) {
            omb_papi_start(&papi_eventset);
        }
        t_start = MPI_Wtime();
        MPI_CHECK(MPI_Ibarrier(omb_comm, &request));
        MPI_CHECK(MPI_Wait(&request, &status));
        t_stop = MPI_Wtime();

        if (i >= options.skip) {
            timer += t_stop - t_start;
        }
    }

    MPI_CHECK(MPI_Barrier(omb_comm));
    omb_papi_stop_and_print(&papi_eventset, 0);

    latency = (timer * 1e6) / options.iterations;

    /* Comm. latency in seconds, fed to dummy_compute */
    latency_in_secs = timer / options.iterations;

    init_arrays(latency_in_secs);

    MPI_CHECK(MPI_Barrier(omb_comm));

    timer = 0.0;
    tcomp_total = 0;
    tcomp = 0;
    init_total = 0.0;
    wait_total = 0.0;
    test_time = 0.0, test_total = 0.0;

    for (i = 0; i < options.iterations + options.skip; i++) {
        t_start = MPI_Wtime();

        init_time = MPI_Wtime();
        MPI_CHECK(MPI_Ibarrier(omb_comm, &request));
        init_time = MPI_Wtime() - init_time;

        tcomp = MPI_Wtime();
        test_time = dummy_compute(latency_in_secs, &request);
        tcomp = MPI_Wtime() - tcomp;

        wait_time = MPI_Wtime();
        MPI_CHECK(MPI_Wait(&request, &status));
        wait_time = MPI_Wtime() - wait_time;

        t_stop = MPI_Wtime();

        if (i >= options.skip) {
            timer += t_stop - t_start;
            tcomp_total += tcomp;
            test_total += test_time;
            init_total += init_time;
            wait_total += wait_time;
            if (options.omb_tail_lat) {
                omb_lat_arr[i - options.skip] = (t_stop - t_start) * 1e6;
            }
            if (options.graph && 0 == rank) {
                omb_graph_data->data[i - options.skip] =
                    (t_stop - t_start) * 1e6;
            }
        }
        MPI_CHECK(MPI_Barrier(omb_comm));
    }

    MPI_Barrier(omb_comm);
    omb_stat = omb_get_stats(omb_lat_arr);

    avg_time = calculate_and_print_stats(rank, size, numprocs, timer, latency,
                                         test_total, tcomp_total, wait_total,
                                         init_total, 0, omb_stat);
    if (0 == rank && options.graph) {
        omb_graph_data->avg = avg_time;
        omb_graph_plot(&omb_graph_options, benchmark_name);
    }
    omb_graph_combined_plot(&omb_graph_options, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_options);
    omb_papi_free(&papi_eventset);

#ifdef _ENABLE_CUDA_KERNEL_
    free_device_arrays();
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

    omb_mpi_finalize(omb_init_h);
    free(omb_lat_arr);

    return EXIT_SUCCESS;
}

/* vi: set sw=4 sts=4 tw=80: */
