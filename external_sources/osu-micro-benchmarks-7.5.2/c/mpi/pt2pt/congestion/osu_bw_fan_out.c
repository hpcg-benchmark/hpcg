#define BENCHMARK "OSU MPI%s Bandwidth Test"
/*
 * Copyright (C) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include <osu_util_mpi.h>
#include "osu_bw_fan_util.h"

double calculate_total(double, double, double, int);

int main(int argc, char *argv[])
{
    int myid, numprocs, i, j, k, n;
    size_t size;
    char **s_buf, **r_buf;
    double t_start = 0.0, t_end = 0.0, t_lo = 0.0, t_total = 0.0;
    double temp_t_total_reduce = 0.0;
    int window_size = 64;
    int po_ret = 0;
    int errors = 0;
    double tmp_total = 0.0;
    omb_graph_options_t omb_graph_options;
    omb_graph_data_t *omb_graph_data = NULL;
    MPI_Datatype omb_curr_datatype = MPI_CHAR;
    size_t num_elements = 0;
    size_t omb_ddt_transmit_size = 0;
    int mpi_type_itr = 0, mpi_type_size = 0, mpi_type_name_length = 0;
    char mpi_type_name_str[OMB_DATATYPE_STR_MAX_LEN];
    MPI_Datatype mpi_type_list[OMB_NUM_DATATYPES];
    int papi_eventset = OMB_PAPI_NULL;
    options.bench = PT2PT;
    options.subtype = CONG_BW;
    MPI_Comm omb_comm = MPI_COMM_NULL;
    omb_mpi_init_data omb_init_h;
    struct omb_buffer_sizes_t omb_buffer_sizes;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;
    MPI_Comm barrier_comm;

    set_header(HEADER);

    po_ret = process_options(argc, argv);
    omb_populate_mpi_type_list(mpi_type_list);
    if (PO_OKAY == po_ret && NONE != options.accel) {
        if (init_accel()) {
            fprintf(stderr, "Error initializing device\n");
            exit(EXIT_FAILURE);
        }
    }
    window_size = options.window_size;
    if (options.buf_num == MULTIPLE) {
        s_buf = malloc(sizeof(char *) * window_size);
        r_buf = malloc(sizeof(char *) * window_size);
    } else {
        s_buf = malloc(sizeof(char *) * 1);
        r_buf = malloc(sizeof(char *) * 1);
    }
    OMB_CHECK_NULL_AND_EXIT(s_buf, "Unable to allcoate send buffer");
    OMB_CHECK_NULL_AND_EXIT(r_buf, "Unable to allcoate receive buffer");
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
    fan_in_out_info_t fan_in_out_info = omb_fan_init(omb_comm);
    MPI_CHECK(MPI_Comm_split(omb_comm, 1 == fan_in_out_info.is_parent, 0,
                             &barrier_comm));
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
#ifdef _ENABLE_CUDA_KERNEL_
    if (options.src == 'M' || options.dst == 'M') {
        if (options.buf_num == SINGLE) {
            fprintf(stderr, "Warning: Tests involving managed buffers will use"
                            " multiple buffers by default\n");
        }
        options.buf_num = MULTIPLE;
    }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

    if (options.buf_num == SINGLE) {
        if (allocate_memory_pt2pt(&s_buf[0], &r_buf[0],
                                  !(1 == fan_in_out_info.is_parent))) {
            /* Error allocating memory */
            omb_mpi_finalize(omb_init_h);
            exit(EXIT_FAILURE);
        }
    }
    print_preamble(myid);
    omb_papi_init(&papi_eventset);

    /* Bandwidth test */
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

            omb_ddt_transmit_size =
                omb_ddt_assign(&omb_curr_datatype, mpi_type_list[mpi_type_itr],
                               num_elements) *
                mpi_type_size;
            num_elements = omb_ddt_get_size(num_elements);
            if (options.buf_num == MULTIPLE) {
                for (i = 0; i < window_size; i++) {
                    if (allocate_memory_pt2pt_size(
                            &s_buf[i], &r_buf[i],
                            !(1 == fan_in_out_info.is_parent), size)) {
                        /* Error allocating memory */
                        omb_mpi_finalize(omb_init_h);
                        exit(EXIT_FAILURE);
                    }
                }

                for (i = 0; i < window_size; i++) {
                    set_buffer_pt2pt(s_buf[i],
                                     !(1 == fan_in_out_info.is_parent),
                                     options.accel, 'a', size);
                    set_buffer_pt2pt(r_buf[i],
                                     !(1 == fan_in_out_info.is_parent),
                                     options.accel, 'b', size);
                }
            } else {
                set_buffer_pt2pt(s_buf[0], !(1 == fan_in_out_info.is_parent),
                                 options.accel, 'a', size);
                set_buffer_pt2pt(r_buf[0], !(1 == fan_in_out_info.is_parent),
                                 options.accel, 'b', size);
            }

#ifdef _ENABLE_CUDA_KERNEL_
            if (options.dst == 'M' && options.MMdst == 'D') {
                t_lo = measure_kernel_lo_window(s_buf, size, window_size);
            }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

            omb_graph_allocate_and_get_data_buffer(
                &omb_graph_data, &omb_graph_options, size, options.iterations);
            MPI_CHECK(MPI_Barrier(omb_comm));
            t_total = 0.0;

            for (i = 0; i < options.iterations + options.skip; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                }
                if (1 == fan_in_out_info.is_parent) {
                    for (k = 0; k <= options.warmup_validation; k++) {
                        if (i >= options.skip &&
                            k == options.warmup_validation) {
                            t_start = MPI_Wtime();
                        }

#ifdef _ENABLE_CUDA_KERNEL_
                        if (options.src == 'M') {
                            touch_managed_src_window(s_buf, size, window_size,
                                                     ADD);
                        }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

                        for (j = 0; j < window_size; j++) {
                            for (n = 0; n < fan_in_out_info.total_nodes - 1;
                                 n++) {
                                if (SINGLE == options.buf_num) {
                                    MPI_CHECK(MPI_Isend(
                                        s_buf[0], num_elements,
                                        omb_curr_datatype,
                                        fan_in_out_info.ranks_queue[n], 100,
                                        omb_comm,
                                        &request[n + j * (fan_in_out_info
                                                              .total_nodes -
                                                          1)]));
                                } else {
                                    MPI_CHECK(MPI_Isend(
                                        s_buf[j], num_elements,
                                        omb_curr_datatype,
                                        fan_in_out_info.ranks_queue[n], 100,
                                        omb_comm,
                                        &request[n + j * (fan_in_out_info
                                                              .total_nodes -
                                                          1)]));
                                }
                            }
                        }
                        MPI_CHECK(MPI_Waitall(
                            (fan_in_out_info.total_nodes - 1) * window_size,
                            request, reqstat));
                        for (n = 0; n < fan_in_out_info.total_nodes - 1; n++) {
                            MPI_CHECK(MPI_Recv(r_buf[0], 1, MPI_CHAR,
                                               fan_in_out_info.ranks_queue[n],
                                               101, omb_comm, &reqstat[0]));
                        }

#ifdef _ENABLE_CUDA_KERNEL_
                        if (options.src == 'M') {
                            touch_managed_src_window(r_buf, size, window_size,
                                                     SUB);
                        }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
                        if (i >= options.skip &&
                            k == options.warmup_validation) {
                            t_end = MPI_Wtime();
                            t_total += calculate_total(t_start, t_end, t_lo,
                                                       window_size);
                            if (options.omb_enable_ddt) {
                                tmp_total = omb_ddt_transmit_size / 1e6 *
                                            window_size * fan_in_out_info.ppn *
                                            (fan_in_out_info.total_nodes - 1);
                            } else {
                                tmp_total = size / 1e6 * window_size *
                                            fan_in_out_info.ppn *
                                            (fan_in_out_info.total_nodes - 1);
                            }
                            if (options.omb_tail_lat) {
                                omb_lat_arr[i - options.skip] =
                                    tmp_total / calculate_total(t_start, t_end,
                                                                t_lo,
                                                                window_size);
                            }
                            if (options.graph && 0 == myid) {
                                omb_graph_data->data[i - options.skip] =
                                    tmp_total / calculate_total(t_start, t_end,
                                                                t_lo,
                                                                window_size);
                            }
                        }
                    }
                } else {
                    for (k = 0; k <= options.warmup_validation; k++) {
#ifdef _ENABLE_CUDA_KERNEL_
                        if (options.dst == 'M') {
                            touch_managed_dst_window(s_buf, size, window_size,
                                                     ADD);
                        }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
                        for (j = 0; j < window_size; j++) {
                            if (SINGLE == options.buf_num) {
                                MPI_CHECK(MPI_Irecv(
                                    r_buf[0], num_elements, omb_curr_datatype,
                                    fan_in_out_info.ranks_queue[0], 100,
                                    omb_comm, &request[j]));
                            } else {
                                MPI_CHECK(MPI_Irecv(
                                    r_buf[j], num_elements, omb_curr_datatype,
                                    fan_in_out_info.ranks_queue[0], 100,
                                    omb_comm, &request[j]));
                            }
                        }
                        MPI_CHECK(MPI_Waitall(window_size, request, reqstat));
#ifdef _ENABLE_CUDA_KERNEL_
                        if (options.dst == 'M') {
                            touch_managed_dst_window(r_buf, size, window_size,
                                                     SUB);
                        }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

                        MPI_CHECK(MPI_Send(s_buf[0], 1, MPI_CHAR,
                                           fan_in_out_info.ranks_queue[0], 101,
                                           omb_comm));
                    }
                }
            }
            omb_papi_stop_and_print(&papi_eventset, size);

            MPI_CHECK(MPI_Reduce(&t_total, &temp_t_total_reduce, 1, MPI_DOUBLE,
                                 MPI_SUM, 0, barrier_comm));
            t_total = temp_t_total_reduce / fan_in_out_info.ppn;

            if (myid == 0) {
                if (options.omb_enable_ddt) {
                    tmp_total = omb_ddt_transmit_size / 1e6 *
                                fan_in_out_info.ppn * options.iterations *
                                window_size * (fan_in_out_info.total_nodes - 1);
                } else {
                    tmp_total = size / 1e6 * fan_in_out_info.ppn *
                                options.iterations * window_size *
                                (fan_in_out_info.total_nodes - 1);
                }
                fprintf(stdout, "%-*d", 10, size);
                fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION,
                        tmp_total / t_total);
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
                    omb_graph_data->avg = tmp_total / t_total;
                }
            }
            omb_ddt_free(&omb_curr_datatype);
            if (options.buf_num == MULTIPLE) {
                for (i = 0; i < window_size; i++) {
                    free_memory(s_buf[i], r_buf[i],
                                !(1 == fan_in_out_info.is_parent));
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
        free_memory(s_buf[0], r_buf[0], !(1 == fan_in_out_info.is_parent));
    }
    omb_fan_free(fan_in_out_info);
    free(s_buf);
    free(r_buf);
    free(omb_lat_arr);
    omb_mpi_finalize(omb_init_h);

    if (NONE != options.accel) {
        if (cleanup_accel()) {
            fprintf(stderr, "Error cleaning up device\n");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}

double calculate_total(double t_start, double t_end, double t_lo,
                       int window_size)
{
    double t_total;

    if (options.dst == 'M' && options.MMdst == 'D') {
        t_total = ((t_end - t_start) - (t_lo * window_size));
    } else {
        t_total = (t_end - t_start);
    }

    return t_total;
}
