#define BENCHMARK "OSU MPI%s Non-blocking Allreduce Latency Test"
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
    int i = 0, j, rank, size;
    int numprocs;
    double latency = 0.0, t_start = 0.0, t_stop = 0.0;
    double tcomp = 0.0, tcomp_total = 0.0, latency_in_secs = 0.0;
    double test_time = 0.0, test_total = 0.0;
    double timer = 0.0;
    int errors = 0, local_errors = 0;
    double wait_time = 0.0, init_time = 0.0;
    double init_total = 0.0, wait_total = 0.0;
    double avg_time = 0.0;
    omb_graph_options_t omb_graph_options;
    omb_graph_data_t *omb_graph_data = NULL;
    int papi_eventset = OMB_PAPI_NULL;
    options.bench = COLLECTIVE;
    options.subtype = NBC_ALL_REDUCE;
    size_t num_elements = 0;
    MPI_Datatype omb_curr_datatype = MPI_SIGNED_CHAR;
    int mpi_type_itr = 0, mpi_type_size = 0, mpi_type_name_length = 0;
    char mpi_type_name_str[OMB_DATATYPE_STR_MAX_LEN];
    MPI_Datatype mpi_type_list[OMB_NUM_DATATYPES];
    MPI_Comm omb_comm = MPI_COMM_NULL;
    omb_mpi_init_data omb_init_h;
    struct omb_buffer_sizes_t omb_buffer_sizes;
    void *sendbuf_warmup = NULL, *recvbuf_warmup = NULL;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    char *sendbuf = NULL;
    char *recvbuf = NULL;
    int po_ret;
    size_t bufsize;

    set_header(HEADER);
    set_benchmark_name("osu_iallreduce");
    po_ret = process_options(argc, argv);
    omb_populate_mpi_type_list(mpi_type_list);

    if (PO_OKAY == po_ret && NONE != options.accel) {
        if (init_accel()) {
            fprintf(stderr, "Error initializing device\n");
            exit(EXIT_FAILURE);
        }
    }

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
        exit(EXIT_FAILURE);
    }
    bufsize = (options.max_message_size);

    if (allocate_memory_coll((void **)&sendbuf, bufsize, options.accel)) {
        fprintf(stderr, "Could Not Allocate Memory [rank %d]\n", rank);
        MPI_CHECK(MPI_Abort(omb_comm, EXIT_FAILURE));
    }
    set_buffer(sendbuf, options.accel, 1, bufsize);
    omb_buffer_sizes.sendbuf_size = bufsize;
    sendbuf_warmup = sendbuf;

    if (allocate_memory_coll((void **)&recvbuf, bufsize, options.accel)) {
        fprintf(stderr, "Could Not Allocate Memory [rank %d]\n", rank);
        MPI_CHECK(MPI_Abort(omb_comm, EXIT_FAILURE));
    }
    omb_buffer_sizes.recvbuf_size = bufsize;
    set_buffer(recvbuf, options.accel, 0, bufsize);
    if (allocate_memory_coll((void **)&recvbuf_warmup, bufsize,
                             options.accel)) {
        fprintf(stderr, "Could Not Allocate Memory [rank %d]\n", rank);
        MPI_CHECK(MPI_Abort(omb_comm, EXIT_FAILURE));
    }
    set_buffer(recvbuf_warmup, options.accel, 0, bufsize);
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }

    print_preamble_nbc(rank);
    omb_papi_init(&papi_eventset);

    for (mpi_type_itr = 0; mpi_type_itr < options.omb_dtype_itr;
         mpi_type_itr++) {
        MPI_CHECK(MPI_Type_size(mpi_type_list[mpi_type_itr], &mpi_type_size));
        omb_curr_datatype = mpi_type_list[mpi_type_itr];
        OMB_MPI_REDUCE_CHAR_CHECK(omb_curr_datatype);
        MPI_CHECK(MPI_Type_get_name(omb_curr_datatype, mpi_type_name_str,
                                    &mpi_type_name_length));
        OMB_MPI_RUN_AT_RANK_ZERO(
            fprintf(stdout, "# Datatype: %s.\n", mpi_type_name_str));
        fflush(stdout);
        print_only_header_nbc(rank);
        for (size = options.min_message_size; size <= options.max_message_size;
             size *= options.message_size_incr) {
            num_elements = size / mpi_type_size;
            if (0 == num_elements) {
                continue;
            }
            get_iterations(size);
            get_skips(size);

            omb_graph_allocate_and_get_data_buffer(
                &omb_graph_data, &omb_graph_options, size, options.iterations);
            MPI_CHECK(MPI_Barrier(omb_comm));

            timer = 0.0;

            if (1 == options.omb_enable_mpi_in_place) {
                sendbuf = MPI_IN_PLACE;
            }
            for (i = 0; i < options.iterations + options.skip; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                }
                if (options.validate) {
                    set_buffer_validation(sendbuf, recvbuf, size, options.accel,
                                          i, omb_curr_datatype,
                                          omb_buffer_sizes);
                    for (j = 0; j < options.warmup_validation; j++) {
                        MPI_CHECK(MPI_Barrier(omb_comm));
                        MPI_CHECK(MPI_Iallreduce(
                            sendbuf_warmup, recvbuf_warmup, num_elements,
                            omb_curr_datatype, MPI_SUM, omb_comm, &request));
                        MPI_CHECK(MPI_Wait(&request, &status));
                    }
                    MPI_CHECK(MPI_Barrier(omb_comm));
                }

                t_start = MPI_Wtime();
                MPI_CHECK(MPI_Iallreduce(sendbuf, recvbuf, num_elements,
                                         omb_curr_datatype, MPI_SUM, omb_comm,
                                         &request));
                MPI_CHECK(MPI_Wait(&request, &status));

                t_stop = MPI_Wtime();

                MPI_CHECK(MPI_Barrier(omb_comm));

                if (options.validate) {
                    local_errors +=
                        validate_data(recvbuf, size, numprocs, options.accel, i,
                                      omb_curr_datatype);
                }

                if (i >= options.skip) {
                    timer += t_stop - t_start;
                }
            }

            MPI_CHECK(MPI_Barrier(omb_comm));
            omb_papi_stop_and_print(&papi_eventset, size);

            latency = (timer * 1e6) / options.iterations;

            /* Comm. latency in seconds, fed to dummy_compute */
            latency_in_secs = timer / options.iterations;

            init_arrays(latency_in_secs);

            MPI_CHECK(MPI_Barrier(omb_comm));

            timer = 0.0;
            tcomp_total = 0.0;
            tcomp = 0.0;
            init_total = 0.0;
            wait_total = 0.0;
            test_time = 0.0, test_total = 0.0;

            for (i = 0; i < options.iterations + options.skip; i++) {
                if (options.validate) {
                    set_buffer_validation(sendbuf, recvbuf, size, options.accel,
                                          i, omb_curr_datatype,
                                          omb_buffer_sizes);
                    for (j = 0; j < options.warmup_validation; j++) {
                        MPI_CHECK(MPI_Barrier(omb_comm));
                        MPI_CHECK(MPI_Iallreduce(
                            sendbuf_warmup, recvbuf_warmup, num_elements,
                            omb_curr_datatype, MPI_SUM, omb_comm, &request));
                        MPI_CHECK(MPI_Wait(&request, &status));
                    }
                    MPI_CHECK(MPI_Barrier(omb_comm));
                }

                t_start = MPI_Wtime();
                init_time = MPI_Wtime();
                MPI_CHECK(MPI_Iallreduce(sendbuf, recvbuf, num_elements,
                                         omb_curr_datatype, MPI_SUM, omb_comm,
                                         &request));
                init_time = MPI_Wtime() - init_time;

                tcomp = MPI_Wtime();
                test_time = dummy_compute(latency_in_secs, &request);
                tcomp = MPI_Wtime() - tcomp;

                wait_time = MPI_Wtime();
                MPI_CHECK(MPI_Wait(&request, &status));
                wait_time = MPI_Wtime() - wait_time;

                t_stop = MPI_Wtime();

                MPI_CHECK(MPI_Barrier(omb_comm));

                if (options.validate) {
                    local_errors +=
                        validate_data(recvbuf, size, numprocs, options.accel, i,
                                      omb_curr_datatype);
                }

                if (i >= options.skip) {
                    timer += t_stop - t_start;
                    tcomp_total += tcomp;
                    wait_total += wait_time;
                    test_total += test_time;
                    init_total += init_time;
                    if (options.omb_tail_lat) {
                        omb_lat_arr[i - options.skip] =
                            (t_stop - t_start) * 1e6;
                    }
                    if (options.graph && 0 == rank) {
                        omb_graph_data->data[i - options.skip] =
                            (t_stop - t_start) * 1e6;
                    }
                }
            }

            MPI_CHECK(MPI_Barrier(omb_comm));

            if (options.validate) {
                MPI_CHECK(MPI_Allreduce(&local_errors, &errors, 1, MPI_INT,
                                        MPI_SUM, omb_comm));
            }
            omb_stat = omb_get_stats(omb_lat_arr);

            avg_time = calculate_and_print_stats(
                rank, size, numprocs, timer, latency, test_total, tcomp_total,
                wait_total, init_total, errors, omb_stat);

            if (options.graph && 0 == rank) {
                omb_graph_data->avg = avg_time;
            }

            if (0 != errors) {
                break;
            }
        }
    }
    if (0 == rank && options.graph) {
        omb_graph_plot(&omb_graph_options, benchmark_name);
    }
    omb_graph_combined_plot(&omb_graph_options, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_options);
    omb_papi_free(&papi_eventset);

    free_buffer(sendbuf_warmup, options.accel);
    free_buffer(recvbuf_warmup, options.accel);
    free_buffer(recvbuf, options.accel);
    free(omb_lat_arr);
    omb_mpi_finalize(omb_init_h);

    if (NONE != options.accel) {
        if (cleanup_accel()) {
            fprintf(stderr, "Error cleaning up device\n");
            exit(EXIT_FAILURE);
        }
    }

    if (0 != errors && options.validate && 0 == rank) {
        fprintf(stdout,
                "DATA VALIDATION ERROR: %s exited with status %d on"
                " message size %d.\n",
                argv[0], EXIT_FAILURE, size);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

/* vi: set sw=4 sts=4 tw=80: */
