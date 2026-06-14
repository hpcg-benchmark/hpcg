#define BENCHMARK "OSU MPI%s Latency Persistent Test"
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

double calculate_total(double, double, double);

int main(int argc, char *argv[])
{
    int myid, numprocs, i, j;
    size_t size;
    MPI_Status reqstat;
    MPI_Request send_obj, recv_obj;
    omb_graph_options_t omb_graph_options;
    omb_graph_data_t *omb_graph_data = NULL;
    char *s_buf, *r_buf;
    double t_start = 0.0, t_end = 0.0, t_lo = 0.0, t_total = 0.0;
    int po_ret = 0;
    int errors = 0;
    size_t num_elements = 0;
    MPI_Datatype omb_curr_datatype = MPI_CHAR;
    size_t omb_ddt_transmit_size = 0;
    int mpi_type_itr = 0, mpi_type_size = 0, mpi_type_name_length = 0;
    char mpi_type_name_str[OMB_DATATYPE_STR_MAX_LEN];
    MPI_Datatype mpi_type_list[OMB_NUM_DATATYPES];
    int papi_eventset = OMB_PAPI_NULL;
    MPI_Comm omb_comm = MPI_COMM_NULL;
    omb_mpi_init_data omb_init_h;
    struct omb_buffer_sizes_t omb_buffer_sizes;

    options.bench = PT2PT;
    options.subtype = LAT;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    set_header(HEADER);
    set_benchmark_name("osu_latency_persistent");

    po_ret = process_options(argc, argv);
    omb_populate_mpi_type_list(mpi_type_list);

    if (PO_OKAY == po_ret && NONE != options.accel) {
        if (init_accel()) {
            fprintf(stderr, "Error initializing device\n");
            exit(EXIT_FAILURE);
        }
    }
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }

    omb_init_h = omb_mpi_init(&argc, &argv);
    omb_comm = omb_init_h.omb_comm;
    if (MPI_COMM_NULL == omb_comm) {
        OMB_ERROR_EXIT("Cant create communicator");
    }
    MPI_CHECK(MPI_Comm_rank(omb_comm, &myid));
    MPI_CHECK(MPI_Comm_size(omb_comm, &numprocs));

    omb_graph_options_init(&omb_graph_options);
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
                omb_mpi_finalize(omb_init_h);
                exit(EXIT_SUCCESS);
            case PO_OKAY:
                break;
        }
    }

    switch (po_ret) {
        case PO_CUDA_NOT_AVAIL:
        case PO_OPENACC_NOT_AVAIL:
        case PO_BAD_USAGE:
            omb_mpi_finalize(omb_init_h);
            exit(EXIT_FAILURE);
        case PO_HELP_MESSAGE:
        case PO_VERSION_MESSAGE:
            omb_mpi_finalize(omb_init_h);
            exit(EXIT_SUCCESS);
        case PO_OKAY:
            break;
    }

    if (numprocs != 2) {
        if (myid == 0) {
            fprintf(stderr, "This test requires exactly two processes\n");
        }

        omb_mpi_finalize(omb_init_h);
        exit(EXIT_FAILURE);
    }

    if (options.buf_num == SINGLE) {
        if (allocate_memory_pt2pt(&s_buf, &r_buf, myid)) {
            /* Error allocating memory */
            omb_mpi_finalize(omb_init_h);
            exit(EXIT_FAILURE);
        }
    }

    print_preamble(myid);
    omb_papi_init(&papi_eventset);

    /* Latency test */
    for (mpi_type_itr = 0; mpi_type_itr < options.omb_dtype_itr;
         mpi_type_itr++) {
        MPI_CHECK(MPI_Type_size(mpi_type_list[mpi_type_itr], &mpi_type_size));
        MPI_CHECK(MPI_Type_get_name(mpi_type_list[mpi_type_itr],
                                    mpi_type_name_str, &mpi_type_name_length));
        omb_curr_datatype = mpi_type_list[mpi_type_itr];
        if (0 == myid) {
            fprintf(stdout, "# Datatype: %s.\n", mpi_type_name_str);
        }
        fflush(stdout);
        print_only_header(myid);
        for (size = options.min_message_size; size <= options.max_message_size;
             size *= options.message_size_incr) {
            num_elements = size / mpi_type_size;
            if (0 == num_elements) {
                continue;
            }

            get_iterations(size);
            get_skips(size);

            if (options.buf_num == MULTIPLE) {
                if (allocate_memory_pt2pt_size(&s_buf, &r_buf, myid, size)) {
                    /* Error allocating memory */
                    omb_mpi_finalize(omb_init_h);
                    exit(EXIT_FAILURE);
                }
            }

            omb_ddt_transmit_size =
                omb_ddt_assign(&omb_curr_datatype, mpi_type_list[mpi_type_itr],
                               num_elements) *
                mpi_type_size;
            num_elements = omb_ddt_get_size(num_elements);
            set_buffer_pt2pt(s_buf, myid, options.accel, 'a', size);
            set_buffer_pt2pt(r_buf, myid, options.accel, 'b', size);

#ifdef _ENABLE_CUDA_KERNEL_
            if ((options.src == 'M' && options.MMsrc == 'D') ||
                (options.dst == 'M' && options.MMdst == 'D')) {
                t_lo = measure_kernel_lo_no_window(s_buf, size);
            }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

            omb_graph_allocate_and_get_data_buffer(
                &omb_graph_data, &omb_graph_options, size, options.iterations);
            if (0 == myid) {
                MPI_CHECK(MPI_Send_init(s_buf, num_elements, omb_curr_datatype,
                                        1, 1, omb_comm, &send_obj));
                MPI_CHECK(MPI_Recv_init(r_buf, num_elements, omb_curr_datatype,
                                        1, 1, omb_comm, &recv_obj));
            } else if (1 == myid) {
                MPI_CHECK(MPI_Recv_init(r_buf, num_elements, omb_curr_datatype,
                                        0, 1, omb_comm, &recv_obj));
                MPI_CHECK(MPI_Send_init(s_buf, num_elements, omb_curr_datatype,
                                        0, 1, omb_comm, &send_obj));
            }
            MPI_CHECK(MPI_Barrier(omb_comm));
            t_total = 0.0;

            for (i = 0; i < options.iterations + options.skip; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                }
                if (options.validate) {
                    set_buffer_validation(s_buf, r_buf, size, options.accel, i,
                                          omb_curr_datatype, omb_buffer_sizes);
                    MPI_CHECK(MPI_Barrier(omb_comm));
                }
                if (myid == 0) {
                    for (j = 0; j <= options.warmup_validation; j++) {
                        if (i >= options.skip &&
                            j == options.warmup_validation) {
                            t_start = MPI_Wtime();
                        }
#ifdef _ENABLE_CUDA_KERNEL_
                        if (options.src == 'M') {
                            touch_managed_src_no_window(s_buf, size, ADD);
                        }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
                        MPI_CHECK(MPI_Start(&send_obj));
                        MPI_CHECK(MPI_Wait(&send_obj, &reqstat));
                        MPI_CHECK(MPI_Start(&recv_obj));
                        MPI_CHECK(MPI_Wait(&recv_obj, &reqstat));
#ifdef _ENABLE_CUDA_KERNEL_
                        if (options.src == 'M') {
                            touch_managed_src_no_window(r_buf, size, SUB);
                        }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
                        if (i >= options.skip &&
                            j == options.warmup_validation) {
                            t_end = MPI_Wtime();
                            t_total += calculate_total(t_start, t_end, t_lo);
                            if (options.omb_tail_lat) {
                                omb_lat_arr[i - options.skip] =
                                    calculate_total(t_start, t_end, t_lo) *
                                    1e6 / 2.0;
                            }
                            if (options.graph) {
                                omb_graph_data->data[i - options.skip] =
                                    calculate_total(t_start, t_end, t_lo) *
                                    1e6 / 2.0;
                            }
                        }
#ifdef _ENABLE_CUDA_KERNEL_
                        if (options.src == 'M' && options.MMsrc == 'D' &&
                            options.validate) {
                            touch_managed_src_no_window(s_buf, size, SUB);
                        }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
                    }
                    if (options.validate) {
                        int errors_recv = 0;
                        MPI_CHECK(MPI_Recv(&errors_recv, 1, MPI_INT, 1, 2,
                                           omb_comm, &reqstat));
                        errors += errors_recv;
                    }
                } else if (myid == 1) {
                    for (j = 0; j <= options.warmup_validation; j++) {
#ifdef _ENABLE_CUDA_KERNEL_
                        if (options.dst == 'M') {
                            touch_managed_dst_no_window(s_buf, size, ADD);
                        }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
                        MPI_CHECK(MPI_Start(&recv_obj));
                        MPI_CHECK(MPI_Wait(&recv_obj, &reqstat));
                        MPI_CHECK(MPI_Start(&send_obj));
                        MPI_CHECK(MPI_Wait(&send_obj, &reqstat));
#ifdef _ENABLE_CUDA_KERNEL_
                        if (options.dst == 'M') {
                            touch_managed_dst_no_window(r_buf, size, SUB);
                        }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
                    }
#ifdef _ENABLE_CUDA_KERNEL_
                    if (options.validate &&
                        !(options.src == 'M' && options.MMsrc == 'D' &&
                          options.dst == 'M' && options.MMdst == 'D')) {
                        if (options.src == 'M' && options.MMsrc == 'D') {
                            touch_managed(r_buf, size, SUB);
                            synchronize_stream();
                        } else if (options.dst == 'M' && options.MMdst == 'D') {
                            touch_managed(r_buf, size, ADD);
                            synchronize_stream();
                        }
                    }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
                    if (options.validate) {
                        errors = validate_data(r_buf, size, 1, options.accel, i,
                                               omb_curr_datatype);
                        MPI_CHECK(
                            MPI_Send(&errors, 1, MPI_INT, 0, 2, omb_comm));
                    }
                }
            }

            omb_papi_stop_and_print(&papi_eventset, size);
            MPI_Request_free(&send_obj);
            MPI_Request_free(&recv_obj);

            if (myid == 0) {
                double latency = (t_total * 1e6) / (2.0 * options.iterations);
                fprintf(stdout, "%-*d", 10, size);
                if (options.validate) {
                    fprintf(stdout, "%*.*f%*s", FIELD_WIDTH, FLOAT_PRECISION,
                            latency, FIELD_WIDTH, VALIDATION_STATUS(errors));
                } else {
                    fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION,
                            latency);
                }
                if (options.omb_tail_lat) {
                    omb_stat = omb_calculate_tail_lat(omb_lat_arr, myid, 1);
                    OMB_ITR_PRINT_STAT(omb_stat.res_arr);
                }
                if (options.omb_enable_ddt) {
                    fprintf(stdout, "%*zu", FIELD_WIDTH, omb_ddt_transmit_size);
                }
                fprintf(stdout, "\n");
                fflush(stdout);
                if (options.graph && 0 == myid) {
                    omb_graph_data->avg = latency;
                }
            }
            omb_ddt_free(&omb_curr_datatype);
            if (options.buf_num == MULTIPLE) {
                free_memory(s_buf, r_buf, myid);
            }

            if (options.validate) {
                MPI_CHECK(MPI_Bcast(&errors, 1, MPI_INT, 0, omb_comm));
                if (0 != errors) {
                    break;
                }
            }
        }
    }
    if (options.graph) {
        omb_graph_plot(&omb_graph_options, benchmark_name);
    }
    omb_graph_combined_plot(&omb_graph_options, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_options);
    omb_papi_free(&papi_eventset);
    if (options.buf_num == SINGLE) {
        free_memory(s_buf, r_buf, myid);
    }
    free(omb_lat_arr);
    omb_mpi_finalize(omb_init_h);

    if (NONE != options.accel) {
        if (cleanup_accel()) {
            fprintf(stderr, "Error cleaning up device\n");
            exit(EXIT_FAILURE);
        }
    }

    if (errors != 0 && options.validate && myid == 0) {
        fprintf(stdout,
                "DATA VALIDATION ERROR: %s exited with status %d on"
                " message size %d.\n",
                argv[0], EXIT_FAILURE, size);
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

double calculate_total(double t_start, double t_end, double t_lo)
{
    double t_total;

    if ((options.src == 'M' && options.MMsrc == 'D') &&
        (options.dst == 'M' && options.MMdst == 'D')) {
        t_total = (t_end - t_start) - (2 * t_lo);
    } else if ((options.src == 'M' && options.MMsrc == 'D') ||
               (options.dst == 'M' && options.MMdst == 'D')) {
        t_total = (t_end - t_start) - t_lo;
    } else {
        t_total = (t_end - t_start);
    }

    return t_total;
}
