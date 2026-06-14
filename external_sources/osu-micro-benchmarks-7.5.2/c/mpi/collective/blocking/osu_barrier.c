#define BENCHMARK "OSU MPI%s Barrier Latency Test"
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
    int i = 0, rank;
    int numprocs;
    double avg_time = 0.0, max_time = 0.0, min_time = 0.0;
    double latency = 0.0, t_start = 0.0, t_stop = 0.0;
    double timer = 0.0;
    int po_ret;
    omb_graph_options_t omb_graph_options;
    omb_graph_data_t *omb_graph_data = NULL;
    int papi_eventset = OMB_PAPI_NULL;
    options.bench = COLLECTIVE;
    options.subtype = BARRIER;
    MPI_Comm omb_comm = MPI_COMM_NULL;
    omb_mpi_init_data omb_init_h;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    set_header(HEADER);
    set_benchmark_name("osu_barrier");
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
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }

    omb_graph_options.number_of_graphs = 0;
    omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_options,
                                           1, options.iterations);
    print_preamble(rank);
    print_only_header(rank);
    omb_papi_init(&papi_eventset);

    timer = 0.0;

    for (i = 0; i < options.iterations + options.skip; i++) {
        if (i == options.skip) {
            omb_papi_start(&papi_eventset);
        }
        t_start = MPI_Wtime();
        MPI_CHECK(MPI_Barrier(omb_comm));
        t_stop = MPI_Wtime();

        if (i >= options.skip) {
            timer += t_stop - t_start;
            if (options.omb_tail_lat) {
                omb_lat_arr[i - options.skip] = (t_stop - t_start) * 1e6;
            }
            if (options.graph && 0 == rank) {
                omb_graph_data->data[i - options.skip] =
                    (t_stop - t_start) * 1e6;
            }
        }
    }

    MPI_CHECK(MPI_Barrier(omb_comm));
    omb_papi_stop_and_print(&papi_eventset, 0);

    latency = (timer * 1e6) / options.iterations;

    MPI_CHECK(
        MPI_Reduce(&latency, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0, omb_comm));
    MPI_CHECK(
        MPI_Reduce(&latency, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, omb_comm));
    MPI_CHECK(
        MPI_Reduce(&latency, &avg_time, 1, MPI_DOUBLE, MPI_SUM, 0, omb_comm));
    avg_time = avg_time / numprocs;
    omb_stat = omb_get_stats(omb_lat_arr);

    print_stats(rank, 0, avg_time, min_time, max_time, omb_stat);
    if (0 == rank && options.graph) {
        omb_graph_data->avg = avg_time;
        omb_graph_plot(&omb_graph_options, benchmark_name);
        omb_graph_combined_plot(&omb_graph_options, benchmark_name);
        omb_graph_free_data_buffers(&omb_graph_options);
    }
    omb_papi_free(&papi_eventset);
    omb_mpi_finalize(omb_init_h);
    free(omb_lat_arr);

    return EXIT_SUCCESS;
}

/* vi: set sw=4 sts=4 tw=80: */
