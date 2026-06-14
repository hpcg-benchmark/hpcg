#define BENCHMARK "OSU MPI%s Gatherv Latency Test"
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
    int i, j, numprocs, rank, size, disp;
    double latency = 0.0, t_start = 0.0, t_stop = 0.0;
    double timer = 0.0;
    int errors = 0, local_errors = 0;
    double avg_time = 0.0, max_time = 0.0, min_time = 0.0;
    char *sendbuf, *recvbuf;
    int *rdispls, *recvcounts;
    int po_ret;
    size_t bufsize;
    omb_graph_options_t omb_graph_options;
    omb_graph_data_t *omb_graph_data = NULL;
    int papi_eventset = OMB_PAPI_NULL;
    options.bench = COLLECTIVE;
    options.subtype = GATHER;
    size_t num_elements = 0;
    MPI_Datatype omb_curr_datatype = MPI_CHAR;
    size_t omb_ddt_transmit_size = 0;
    int mpi_type_itr = 0, mpi_type_size = 0, mpi_type_name_length = 0;
    char mpi_type_name_str[OMB_DATATYPE_STR_MAX_LEN];
    MPI_Datatype mpi_type_list[OMB_NUM_DATATYPES];
    MPI_Comm omb_comm = MPI_COMM_NULL;
    omb_mpi_init_data omb_init_h;
    struct omb_buffer_sizes_t omb_buffer_sizes;
    int root_rank = 0;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    set_header(HEADER);
    set_benchmark_name("osu_gatherv");
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
    bufsize = options.max_message_size * numprocs;
    if (allocate_memory_coll((void **)&recvcounts, numprocs * sizeof(int),
                             NONE)) {
        fprintf(stderr, "Could Not Allocate Memory [rank %d]\n", rank);
        MPI_CHECK(MPI_Abort(omb_comm, EXIT_FAILURE));
    }
    if (allocate_memory_coll((void **)&rdispls, numprocs * sizeof(int), NONE)) {
        fprintf(stderr, "Could Not Allocate Memory [rank %d]\n", rank);
        MPI_CHECK(MPI_Abort(omb_comm, EXIT_FAILURE));
    }

    if (allocate_memory_coll((void **)&recvbuf, bufsize, options.accel)) {
        fprintf(stderr, "Could Not Allocate Memory [rank %d]\n", rank);
        MPI_CHECK(MPI_Abort(omb_comm, EXIT_FAILURE));
    }
    set_buffer(recvbuf, options.accel, 1, bufsize);
    omb_buffer_sizes.recvbuf_size = bufsize;

    if (allocate_memory_coll((void **)&sendbuf, bufsize, options.accel)) {
        fprintf(stderr, "Could Not Allocate Memory [rank %d]\n", rank);
        MPI_CHECK(MPI_Abort(omb_comm, EXIT_FAILURE));
    }
    set_buffer(sendbuf, options.accel, 0, bufsize);
    omb_buffer_sizes.sendbuf_size = bufsize;
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    print_preamble(rank);
    omb_papi_init(&papi_eventset);

    for (mpi_type_itr = 0; mpi_type_itr < options.omb_dtype_itr;
         mpi_type_itr++) {
        MPI_CHECK(MPI_Type_size(mpi_type_list[mpi_type_itr], &mpi_type_size));
        MPI_CHECK(MPI_Type_get_name(mpi_type_list[mpi_type_itr],
                                    mpi_type_name_str, &mpi_type_name_length));
        omb_curr_datatype = mpi_type_list[mpi_type_itr];
        OMB_MPI_RUN_AT_RANK_ZERO(
            fprintf(stdout, "# Datatype: %s.\n", mpi_type_name_str));
        fflush(stdout);
        print_only_header(rank);
        for (size = options.min_message_size; size <= options.max_message_size;
             size *= options.message_size_incr) {
            num_elements = size / mpi_type_size;
            if (0 == num_elements) {
                continue;
            }

            get_iterations(size);
            get_skips(size);

            omb_ddt_transmit_size =
                omb_ddt_assign(&omb_curr_datatype, mpi_type_list[mpi_type_itr],
                               num_elements) *
                mpi_type_size;
            num_elements = omb_ddt_get_size(num_elements);
            MPI_CHECK(MPI_Barrier(omb_comm));

            disp = 0;
            for (i = 0; i < numprocs; i++) {
                recvcounts[i] = num_elements;
                rdispls[i] = disp;
                disp += num_elements;
            }

            omb_graph_allocate_and_get_data_buffer(
                &omb_graph_data, &omb_graph_options, size, options.iterations);
            MPI_CHECK(MPI_Barrier(omb_comm));
            timer = 0.0;
            for (i = 0; i < options.iterations + options.skip; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                }
                root_rank = omb_get_root_rank(i, numprocs);
                if (options.validate) {
                    set_buffer_validation(sendbuf, recvbuf, size, options.accel,
                                          i, omb_curr_datatype,
                                          omb_buffer_sizes);
                    for (j = 0; j < options.warmup_validation; j++) {
                        MPI_CHECK(MPI_Barrier(omb_comm));
                        MPI_CHECK(MPI_Gatherv(
                            sendbuf, num_elements, omb_curr_datatype, recvbuf,
                            recvcounts, rdispls, omb_curr_datatype, root_rank,
                            omb_comm));
                    }
                    MPI_CHECK(MPI_Barrier(omb_comm));
                }

                t_start = MPI_Wtime();

                if (1 == options.omb_enable_mpi_in_place) {
                    if (root_rank == rank) {
                        MPI_CHECK(MPI_Gatherv(
                            MPI_IN_PLACE, num_elements, omb_curr_datatype,
                            recvbuf, recvcounts, rdispls, omb_curr_datatype,
                            root_rank, omb_comm));
                    } else {
                        MPI_CHECK(MPI_Gatherv(
                            sendbuf, num_elements, omb_curr_datatype, NULL,
                            recvcounts, rdispls, omb_curr_datatype, root_rank,
                            omb_comm));
                    }
                } else {
                    MPI_CHECK(
                        MPI_Gatherv(sendbuf, num_elements, omb_curr_datatype,
                                    recvbuf, recvcounts, rdispls,
                                    omb_curr_datatype, root_rank, omb_comm));
                }

                t_stop = MPI_Wtime();

                MPI_CHECK(MPI_Barrier(omb_comm));

                if (options.validate && root_rank == rank) {
                    local_errors +=
                        validate_data(recvbuf, size, numprocs, options.accel, i,
                                      omb_curr_datatype);
                }

                if (i >= options.skip) {
                    timer += t_stop - t_start;
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
            omb_papi_stop_and_print(&papi_eventset, size);

            latency = (double)(timer * 1e6) / options.iterations;

            MPI_CHECK(MPI_Reduce(&latency, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0,
                                 omb_comm));
            MPI_CHECK(MPI_Reduce(&latency, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0,
                                 omb_comm));
            MPI_CHECK(MPI_Reduce(&latency, &avg_time, 1, MPI_DOUBLE, MPI_SUM, 0,
                                 omb_comm));
            avg_time = avg_time / numprocs;
            omb_stat = omb_get_stats(omb_lat_arr);

            if (options.validate) {
                MPI_CHECK(MPI_Allreduce(&local_errors, &errors, 1, MPI_INT,
                                        MPI_SUM, omb_comm));
            }

            if (options.validate) {
                print_stats_validate(rank, size, avg_time, min_time, max_time,
                                     errors, omb_stat);
            } else {
                print_stats(rank, size, avg_time, min_time, max_time, omb_stat);
            }
            if (options.graph && 0 == rank) {
                omb_graph_data->avg = avg_time;
            }
            omb_ddt_append_stats(omb_ddt_transmit_size);
            omb_ddt_free(&omb_curr_datatype);
            MPI_CHECK(MPI_Barrier(omb_comm));

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
    free_buffer(rdispls, NONE);
    free_buffer(recvcounts, NONE);
    free_buffer(recvbuf, options.accel);
    free_buffer(sendbuf, options.accel);
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
