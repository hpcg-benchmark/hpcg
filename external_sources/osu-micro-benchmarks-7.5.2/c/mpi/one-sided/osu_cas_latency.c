#define BENCHMARK "OSU MPI_Compare_and_swap%s latency Test"
/*
 * Copyright (c) 2003-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include <osu_util_mpi.h>

double t_start = 0.0, t_end = 0.0;
uint64_t *sbuf = NULL, *tbuf = NULL, *cbuf = NULL, *win_base = NULL;
omb_graph_options_t omb_graph_op;
int validation_error_flag = 0;
int dtype_size;
MPI_Datatype mpi_type_list[OMB_NUM_DATATYPES];
MPI_Comm omb_comm = MPI_COMM_NULL;

void print_latency(int, int, float, struct omb_stat_t);
void run_cas_with_lock(int, enum WINDOW, MPI_Datatype);
void run_cas_with_fence(int, enum WINDOW, MPI_Datatype);
void run_cas_with_lock_all(int, enum WINDOW, MPI_Datatype);
void run_cas_with_flush(int, enum WINDOW, MPI_Datatype);
void run_cas_with_flush_local(int, enum WINDOW, MPI_Datatype);
void run_cas_with_pscw(int, enum WINDOW, MPI_Datatype);

int main(int argc, char *argv[])
{
    int rank, nprocs;
    int po_ret = PO_OKAY;
    int ntypes = 0;
    int jtype_test = 0;
    int jrank_print = 0;
    omb_mpi_init_data omb_init_h;
    options.win = WIN_ALLOCATE;
    options.sync = FLUSH;
    options.bench = ONE_SIDED;
    options.subtype = LAT;
    options.synctype = ALL_SYNC;
    options.max_message_size = 1 << 20;
    options.show_validation = 1;
    set_header(HEADER);
    set_benchmark_name("osu_cas_latency");

    po_ret = process_options(argc, argv);
    omb_populate_mpi_type_list(mpi_type_list);
    ntypes = options.omb_dtype_itr;

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
    MPI_CHECK(MPI_Comm_size(omb_comm, &nprocs));

    if (0 == rank) {
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
                print_bad_usage_message(rank);
            case PO_HELP_MESSAGE:
                usage_one_sided("osu_cas_latency");
                break;
            case PO_VERSION_MESSAGE:
                print_version_message(rank);
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

    if (nprocs != 2) {
        if (rank == 0) {
            fprintf(stderr, "This test requires exactly two processes\n");
        }
        omb_mpi_finalize(omb_init_h);
        return EXIT_FAILURE;
    }

    for (jtype_test = 0; jtype_test < ntypes; jtype_test++) {
        MPI_CHECK(MPI_Type_size(mpi_type_list[jtype_test], &dtype_size));

        print_header_one_sided(rank, options.win, options.sync,
                               mpi_type_list[jtype_test]);

        switch (options.sync) {
            case LOCK:
                run_cas_with_lock(rank, options.win, mpi_type_list[jtype_test]);
                break;
            case LOCK_ALL:
                run_cas_with_lock_all(rank, options.win,
                                      mpi_type_list[jtype_test]);
                break;
            case PSCW:
                run_cas_with_pscw(rank, options.win, mpi_type_list[jtype_test]);
                break;
            case FENCE:
                run_cas_with_fence(rank, options.win,
                                   mpi_type_list[jtype_test]);
                break;
            case FLUSH_LOCAL:
                run_cas_with_flush_local(rank, options.win,
                                         mpi_type_list[jtype_test]);
                break;
            default:
                run_cas_with_flush(rank, options.win,
                                   mpi_type_list[jtype_test]);
                break;
        }
    }

    if (options.validate) {
        for (jrank_print = 0; jrank_print < 2; jrank_print++) {
            if (jrank_print == rank) {
                printf("-------------------------------------------\n");
                printf("Atomic Data Validation results for Rank=%d:\n", rank);
                atomic_data_validation_print_summary();
                printf("-------------------------------------------\n");
            }
            MPI_CHECK(MPI_Barrier(omb_comm));
        }
    }

    omb_mpi_finalize(omb_init_h);

    if (NONE != options.accel) {
        if (cleanup_accel()) {
            fprintf(stderr, "Error cleaning up device\n");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}

void print_latency(int rank, int size, float latency_factor,
                   struct omb_stat_t omb_stat)
{
    char *validation_string;
    if (rank != 0)
        return;
    if (options.validate) {
        if (2 & validation_error_flag)
            validation_string = "skipped";
        else if (1 & validation_error_flag)
            validation_string = "failed";
        else
            validation_string = "passed";
        fprintf(stdout, "%-*d%*.*f%*s", 10, size, FIELD_WIDTH, FLOAT_PRECISION,
                (t_end - t_start) * 1.0e6 * latency_factor / options.iterations,
                FIELD_WIDTH, validation_string);
        validation_error_flag = 0;
    } else {
        fprintf(stdout, "%-*d%*.*f", 10, size, FIELD_WIDTH, FLOAT_PRECISION,
                (t_end - t_start) * 1.0e6 * latency_factor /
                    options.iterations);
    }
    if (options.omb_tail_lat) {
        OMB_ITR_PRINT_STAT(omb_stat.res_arr);
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}

/*Run CAS with flush */
void run_cas_with_flush(int rank, enum WINDOW win_type, MPI_Datatype data_type)
{
    int i = 0;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    omb_graph_data_t *omb_graph_data = NULL;
    int papi_eventset = OMB_PAPI_NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_graph_op.number_of_graphs = 0;
    omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op, 8,
                                           options.iterations);
    allocate_atomic_memory(rank, (char **)&sbuf, (char **)&tbuf, (char **)&cbuf,
                           (char **)&win_base, options.max_message_size,
                           win_type, &win);

    omb_papi_init(&papi_eventset);
    MPI_CHECK(MPI_Barrier(omb_comm));

    if (rank == 0) {
        if (win_type == WIN_DYNAMIC) {
            disp = disp_remote;
        }
        MPI_CHECK(MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win));
        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip && options.validate) {
                MPI_CHECK(MPI_Barrier(omb_comm));
                atomic_data_validation_setup(data_type, rank, win_base,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, sbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, tbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, cbuf,
                                             options.max_message_size);
                MPI_CHECK(MPI_Barrier(omb_comm));
            }

            if (i == options.skip) {
                omb_papi_start(&papi_eventset);
                t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                t_graph_start = MPI_Wtime();
            }
            MPI_CHECK(MPI_Compare_and_swap(sbuf, cbuf, tbuf, data_type, 1, disp,
                                           win));
            MPI_CHECK(MPI_Win_flush(1, win));
            if (i >= options.skip) {
                t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    omb_lat_arr[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6;
                }
                if (options.graph) {
                    omb_graph_data->data[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6;
                }
            }
            if (i == options.skip && options.validate) {
                MPI_CHECK(MPI_Barrier(omb_comm));
                atomic_data_validation_check(
                    data_type, (MPI_Op)-1, rank, win_base, tbuf,
                    options.max_message_size, 0, 1, &validation_error_flag);
            }
        }
        t_end = MPI_Wtime();
        MPI_CHECK(MPI_Win_unlock(1, win));
    } else if (options.validate) {
        MPI_CHECK(MPI_Barrier(omb_comm));
        atomic_data_validation_setup(data_type, rank, win_base,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, sbuf,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, tbuf,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, cbuf,
                                     options.max_message_size);
        MPI_CHECK(MPI_Barrier(omb_comm));
        MPI_CHECK(MPI_Barrier(omb_comm));
        atomic_data_validation_check(data_type, (MPI_Op)-1, rank, win_base,
                                     tbuf, options.max_message_size, 1, 0,
                                     &validation_error_flag);
    }

    MPI_CHECK(MPI_Barrier(omb_comm));

    omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
    omb_papi_stop_and_print(&papi_eventset, 8);
    print_latency(rank, dtype_size, 1, omb_stat);
    if (options.graph && 0 == rank) {
        omb_graph_data->avg = (t_end - t_start) * 1.0e6 / options.iterations;
    }
    omb_graph_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free(omb_lat_arr);
    free_atomic_memory(sbuf, win_base, tbuf, cbuf, win_type, win, rank);
}

/*Run CAS with Lock_all/unlock_all */
void run_cas_with_lock_all(int rank, enum WINDOW win_type,
                           MPI_Datatype data_type)
{
    int i = 0;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    omb_graph_data_t *omb_graph_data = NULL;
    int papi_eventset = OMB_PAPI_NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_graph_op.number_of_graphs = 0;
    omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op, 8,
                                           options.iterations);
    omb_papi_init(&papi_eventset);
    allocate_atomic_memory(rank, (char **)&sbuf, (char **)&tbuf, (char **)&cbuf,
                           (char **)&win_base, options.max_message_size,
                           win_type, &win);

    if (rank == 0) {
        if (win_type == WIN_DYNAMIC) {
            disp = disp_remote;
        }

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip && options.validate) {
                MPI_CHECK(MPI_Barrier(omb_comm));
                atomic_data_validation_setup(data_type, rank, win_base,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, sbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, tbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, cbuf,
                                             options.max_message_size);
                MPI_CHECK(MPI_Barrier(omb_comm));
            }

            if (i == options.skip) {
                omb_papi_start(&papi_eventset);
                t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                t_graph_start = MPI_Wtime();
            }
            MPI_CHECK(MPI_Win_lock_all(0, win));
            MPI_CHECK(MPI_Compare_and_swap(sbuf, cbuf, tbuf, data_type, 1, disp,
                                           win));
            MPI_CHECK(MPI_Win_unlock_all(win));
            if (i >= options.skip) {
                t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    omb_lat_arr[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6;
                }
                if (options.graph) {
                    omb_graph_data->data[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6;
                }
            }
            if (i == options.skip && options.validate) {
                MPI_CHECK(MPI_Barrier(omb_comm));
                atomic_data_validation_check(
                    data_type, (MPI_Op)-1, rank, win_base, tbuf,
                    options.max_message_size, 0, 1, &validation_error_flag);
            }
        }
        t_end = MPI_Wtime();
    } else if (options.validate) {
        MPI_CHECK(MPI_Barrier(omb_comm));
        atomic_data_validation_setup(data_type, rank, win_base,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, sbuf,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, tbuf,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, cbuf,
                                     options.max_message_size);
        MPI_CHECK(MPI_Barrier(omb_comm));
        MPI_CHECK(MPI_Barrier(omb_comm));
        atomic_data_validation_check(data_type, (MPI_Op)-1, rank, win_base,
                                     tbuf, options.max_message_size, 1, 0,
                                     &validation_error_flag);
    }

    MPI_CHECK(MPI_Barrier(omb_comm));

    omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
    omb_papi_stop_and_print(&papi_eventset, 8);
    print_latency(rank, dtype_size, 1, omb_stat);
    if (options.graph && 0 == rank) {
        omb_graph_data->avg = (t_end - t_start) * 1.0e6 / options.iterations;
    }
    omb_graph_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free_atomic_memory(sbuf, win_base, tbuf, cbuf, win_type, win, rank);
    free(omb_lat_arr);
}

/*Run CAS with flush */
void run_cas_with_flush_local(int rank, enum WINDOW win_type,
                              MPI_Datatype data_type)
{
    int i = 0;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    omb_graph_data_t *omb_graph_data = NULL;
    int papi_eventset = OMB_PAPI_NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_graph_op.number_of_graphs = 0;
    omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op, 8,
                                           options.iterations);
    omb_papi_init(&papi_eventset);
    allocate_atomic_memory(rank, (char **)&sbuf, (char **)&tbuf, (char **)&cbuf,
                           (char **)&win_base, options.max_message_size,
                           win_type, &win);

    MPI_CHECK(MPI_Barrier(omb_comm));

    if (rank == 0) {
        if (win_type == WIN_DYNAMIC) {
            disp = disp_remote;
        }
        MPI_CHECK(MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win));
        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip && options.validate) {
                MPI_CHECK(MPI_Barrier(omb_comm));
                atomic_data_validation_setup(data_type, rank, win_base,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, sbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, tbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, cbuf,
                                             options.max_message_size);
                MPI_CHECK(MPI_Barrier(omb_comm));
            }

            if (i == options.skip) {
                omb_papi_start(&papi_eventset);
                t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                t_graph_start = MPI_Wtime();
            }
            MPI_CHECK(MPI_Compare_and_swap(sbuf, cbuf, tbuf, data_type, 1, disp,
                                           win));
            MPI_CHECK(MPI_Win_flush_local(1, win));
            if (i >= options.skip) {
                t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    omb_lat_arr[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6;
                }
                if (options.graph) {
                    omb_graph_data->data[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6;
                }
            }
            if (i == options.skip && options.validate) {
                MPI_CHECK(MPI_Barrier(omb_comm));
                atomic_data_validation_check(
                    data_type, (MPI_Op)-1, rank, win_base, tbuf,
                    options.max_message_size, 0, 1, &validation_error_flag);
            }
        }
        t_end = MPI_Wtime();
        MPI_CHECK(MPI_Win_unlock(1, win));
    } else if (options.validate) {
        MPI_CHECK(MPI_Barrier(omb_comm));
        atomic_data_validation_setup(data_type, rank, win_base,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, sbuf,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, tbuf,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, cbuf,
                                     options.max_message_size);
        MPI_CHECK(MPI_Barrier(omb_comm));
        MPI_CHECK(MPI_Barrier(omb_comm));
        atomic_data_validation_check(data_type, (MPI_Op)-1, rank, win_base,
                                     tbuf, options.max_message_size, 1, 0,
                                     &validation_error_flag);
    }

    MPI_CHECK(MPI_Barrier(omb_comm));

    omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
    omb_papi_stop_and_print(&papi_eventset, 8);
    print_latency(rank, dtype_size, 1, omb_stat);
    if (options.graph && 0 == rank) {
        omb_graph_data->avg = (t_end - t_start) * 1.0e6 / options.iterations;
    }
    omb_graph_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free_atomic_memory(sbuf, win_base, tbuf, cbuf, win_type, win, rank);
    free(omb_lat_arr);
}

/*Run CAS with Lock/unlock */
void run_cas_with_lock(int rank, enum WINDOW win_type, MPI_Datatype data_type)
{
    int i = 0;
    double t_graph_start = 0, t_graph_end = 0;
    omb_graph_data_t *omb_graph_data = NULL;
    int papi_eventset = OMB_PAPI_NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    int dtype_size;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_graph_op.number_of_graphs = 0;
    omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op, 8,
                                           options.iterations);
    omb_papi_init(&papi_eventset);
    allocate_atomic_memory(rank, (char **)&sbuf, (char **)&tbuf, (char **)&cbuf,
                           (char **)&win_base, options.max_message_size,
                           win_type, &win);

    MPI_CHECK(MPI_Type_size(data_type, &dtype_size));

    if (rank == 0) {
        if (win_type == WIN_DYNAMIC) {
            disp = disp_remote;
        }
        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip && options.validate) {
                MPI_CHECK(MPI_Barrier(omb_comm));
                atomic_data_validation_setup(data_type, rank, win_base,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, sbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, tbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, cbuf,
                                             options.max_message_size);
                MPI_CHECK(MPI_Barrier(omb_comm));
            }

            if (i == options.skip) {
                omb_papi_start(&papi_eventset);
                t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                t_graph_start = MPI_Wtime();
            }
            MPI_CHECK(MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 1, 0, win));
            MPI_CHECK(MPI_Compare_and_swap(sbuf, cbuf, tbuf, data_type, 1, disp,
                                           win));
            MPI_CHECK(MPI_Win_unlock(1, win));
            if (i >= options.skip) {
                t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    omb_lat_arr[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6;
                }
                if (options.graph) {
                    omb_graph_data->data[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6;
                }
            }
            if (i == options.skip && options.validate) {
                MPI_CHECK(MPI_Barrier(omb_comm));
                atomic_data_validation_check(
                    data_type, (MPI_Op)-1, rank, win_base, tbuf,
                    options.max_message_size, 0, 1, &validation_error_flag);
            }
        }
        t_end = MPI_Wtime();
    } else if (options.validate) {
        MPI_CHECK(MPI_Barrier(omb_comm));
        atomic_data_validation_setup(data_type, rank, win_base,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, sbuf,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, tbuf,
                                     options.max_message_size);
        atomic_data_validation_setup(data_type, rank, cbuf,
                                     options.max_message_size);
        MPI_CHECK(MPI_Barrier(omb_comm));
        MPI_CHECK(MPI_Barrier(omb_comm));
        atomic_data_validation_check(data_type, (MPI_Op)-1, rank, win_base,
                                     tbuf, options.max_message_size, 1, 0,
                                     &validation_error_flag);
    }

    MPI_CHECK(MPI_Barrier(omb_comm));

    omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
    omb_papi_stop_and_print(&papi_eventset, 8);
    print_latency(rank, dtype_size, 1, omb_stat);
    if (options.graph && 0 == rank) {
        omb_graph_data->avg = (t_end - t_start) * 1.0e6 / options.iterations;
    }
    omb_graph_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free_atomic_memory(sbuf, win_base, tbuf, cbuf, win_type, win, rank);
    free(omb_lat_arr);
}

/*Run CAS with Fence */
void run_cas_with_fence(int rank, enum WINDOW win_type, MPI_Datatype data_type)
{
    int i = 0;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    omb_graph_data_t *omb_graph_data = NULL;
    int papi_eventset = OMB_PAPI_NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_graph_op.number_of_graphs = 0;
    omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op, 8,
                                           options.iterations);
    omb_papi_init(&papi_eventset);
    MPI_CHECK(MPI_Barrier(omb_comm));

    allocate_atomic_memory(rank, (char **)&sbuf, (char **)&tbuf, (char **)&cbuf,
                           (char **)&win_base, options.max_message_size,
                           win_type, &win);

    if (win_type == WIN_DYNAMIC) {
        disp = disp_remote;
    }
    if (rank == 0) {
        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&papi_eventset);
                t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                if (options.validate) {
                    atomic_data_validation_setup(data_type, rank, win_base,
                                                 options.max_message_size);
                    atomic_data_validation_setup(data_type, rank, sbuf,
                                                 options.max_message_size);
                    atomic_data_validation_setup(data_type, rank, tbuf,
                                                 options.max_message_size);
                    atomic_data_validation_setup(data_type, rank, cbuf,
                                                 options.max_message_size);
                }
                t_graph_start = MPI_Wtime();
            }
            MPI_CHECK(MPI_Win_fence(0, win));
            MPI_CHECK(MPI_Compare_and_swap(sbuf, cbuf, tbuf, data_type, 1, disp,
                                           win));
            MPI_CHECK(MPI_Win_fence(0, win));
            MPI_CHECK(MPI_Win_fence(0, win));
            if (i >= options.skip) {
                t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    omb_lat_arr[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6;
                }
                if (options.graph) {
                    omb_graph_data->data[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6 / 2.0;
                }
                if (options.validate) {
                    atomic_data_validation_check(
                        data_type, (MPI_Op)-1, rank, win_base, tbuf,
                        options.max_message_size, 0, 1, &validation_error_flag);
                }
            }
        }
        t_end = MPI_Wtime();
    } else {
        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&papi_eventset);
            }
            if (i >= options.skip && options.validate) {
                atomic_data_validation_setup(data_type, rank, win_base,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, sbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, tbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, cbuf,
                                             options.max_message_size);
            }
            MPI_CHECK(MPI_Win_fence(0, win));
            MPI_CHECK(MPI_Win_fence(0, win));
            MPI_CHECK(MPI_Compare_and_swap(sbuf, cbuf, tbuf, data_type, 0, disp,
                                           win));
            MPI_CHECK(MPI_Win_fence(0, win));
            if (i >= options.skip && options.validate) {
                atomic_data_validation_check(
                    data_type, (MPI_Op)-1, rank, win_base, tbuf,
                    options.max_message_size, 0, 1, &validation_error_flag);
            }
        }
    }

    MPI_CHECK(MPI_Barrier(omb_comm));

    omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
    omb_papi_stop_and_print(&papi_eventset, 8);
    print_latency(rank, dtype_size, 0.5, omb_stat);
    if (options.graph && 0 == rank) {
        omb_graph_data->avg =
            (t_end - t_start) * 1.0e6 / options.iterations / 2;
    }
    omb_graph_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free_atomic_memory(sbuf, win_base, tbuf, cbuf, win_type, win, rank);
    free(omb_lat_arr);
}

/*Run CAS with Post/Start/Complete/Wait */
void run_cas_with_pscw(int rank, enum WINDOW win_type, MPI_Datatype data_type)
{
    int destrank, i;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    omb_graph_data_t *omb_graph_data = NULL;
    int papi_eventset = OMB_PAPI_NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    MPI_Group comm_group, group;
    MPI_CHECK(MPI_Comm_group(omb_comm, &comm_group));
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_graph_op.number_of_graphs = 0;
    omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op, 8,
                                           options.iterations);
    omb_papi_init(&papi_eventset);
    allocate_atomic_memory(rank, (char **)&sbuf, (char **)&tbuf, (char **)&cbuf,
                           (char **)&win_base, options.max_message_size,
                           win_type, &win);

    if (win_type == WIN_DYNAMIC) {
        disp = disp_remote;
    }

    if (rank == 0) {
        destrank = 1;

        MPI_CHECK(MPI_Group_incl(comm_group, 1, &destrank, &group));
        MPI_CHECK(MPI_Barrier(omb_comm));

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i >= options.skip && options.validate) {
                atomic_data_validation_setup(data_type, rank, win_base,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, sbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, tbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, cbuf,
                                             options.max_message_size);
            }

            MPI_CHECK(MPI_Win_start(group, 0, win));

            if (i == options.skip) {
                omb_papi_start(&papi_eventset);
                t_start = MPI_Wtime();
            }

            if (i >= options.skip) {
                t_graph_start = MPI_Wtime();
            }
            MPI_CHECK(MPI_Compare_and_swap(sbuf, cbuf, tbuf, data_type, 1, disp,
                                           win));
            MPI_CHECK(MPI_Win_complete(win));
            MPI_CHECK(MPI_Win_post(group, 0, win));
            MPI_CHECK(MPI_Win_wait(win));
            if (i >= options.skip) {
                t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    omb_lat_arr[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6;
                }
                if (options.graph) {
                    omb_graph_data->data[i - options.skip] =
                        (t_graph_end - t_graph_start) * 1.0e6 / 2.0;
                }
                if (options.validate) {
                    atomic_data_validation_check(
                        data_type, (MPI_Op)-1, rank, win_base, tbuf,
                        options.max_message_size, 1, 1, &validation_error_flag);
                }
            }
        }

        t_end = MPI_Wtime();
    } else {
        /* rank=1 */
        destrank = 0;

        MPI_CHECK(MPI_Group_incl(comm_group, 1, &destrank, &group));
        MPI_CHECK(MPI_Barrier(omb_comm));

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i >= options.skip && options.validate) {
                atomic_data_validation_setup(data_type, rank, win_base,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, sbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, tbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(data_type, rank, cbuf,
                                             options.max_message_size);
            }

            if (i == options.skip) {
                omb_papi_start(&papi_eventset);
            }
            MPI_CHECK(MPI_Win_post(group, 0, win));
            MPI_CHECK(MPI_Win_wait(win));
            MPI_CHECK(MPI_Win_start(group, 0, win));
            MPI_CHECK(MPI_Compare_and_swap(sbuf, cbuf, tbuf, data_type, 0, disp,
                                           win));
            MPI_CHECK(MPI_Win_complete(win));

            if (i >= options.skip && options.validate) {
                atomic_data_validation_check(
                    data_type, (MPI_Op)-1, rank, win_base, tbuf,
                    options.max_message_size, 1, 1, &validation_error_flag);
            }
        }
    }

    MPI_CHECK(MPI_Barrier(omb_comm));

    omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
    omb_papi_stop_and_print(&papi_eventset, 8);
    print_latency(rank, dtype_size, 0.5, omb_stat);
    if (options.graph && 0 == rank) {
        omb_graph_data->avg =
            (t_end - t_start) * 1.0e6 / options.iterations / 2;
    }
    omb_graph_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);

    MPI_CHECK(MPI_Group_free(&group));
    MPI_CHECK(MPI_Group_free(&comm_group));

    free_atomic_memory(sbuf, win_base, tbuf, cbuf, win_type, win, rank);
    free(omb_lat_arr);
}
/* vi: set sw=4 sts=4 tw=80: */
