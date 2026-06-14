#define BENCHMARK "OSU MPI_Get%s Bandwidth Test"
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
char *rbuf = NULL, *win_base = NULL;
omb_graph_options_t omb_graph_op;
MPI_Comm omb_comm = MPI_COMM_NULL;

void print_bw(int, int, double, struct omb_stat_t);
void run_get_with_lock(int, enum WINDOW);
void run_get_with_fence(int, enum WINDOW);
#if MPI_VERSION >= 3
void run_get_with_lock_all(int, enum WINDOW);
void run_get_with_flush(int, enum WINDOW);
void run_get_with_flush_local(int, enum WINDOW);
#endif
void run_get_with_pscw(int, enum WINDOW);

int main(int argc, char *argv[])
{
    int rank, nprocs;
    int po_ret = PO_OKAY;
    omb_mpi_init_data omb_init_h;
#if MPI_VERSION >= 3
    options.sync = FLUSH;
    options.win = WIN_ALLOCATE;
#else
    options.sync = LOCK;
    options.win = WIN_CREATE;
#endif

    options.bench = ONE_SIDED;
    options.subtype = BW;
    options.synctype = ALL_SYNC;
    MPI_Datatype mpi_type_list[OMB_NUM_DATATYPES];

    set_header(HEADER);
    set_benchmark_name("osu_get_bw");

    po_ret = process_options(argc, argv);
    omb_populate_mpi_type_list(mpi_type_list);
    if (options.validate) {
        OMB_ERROR_EXIT("Benchmark does not support validation");
    }

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
        if (options.omb_dtype_itr > 1 || mpi_type_list[0] != MPI_CHAR) {
            fprintf(stderr, "Benchmark supports only MPI_CHAR. Continuing with "
                            "MPI_CHAR.\n");
            fflush(stderr);
        }
    }

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
                usage_one_sided("osu_get_bw");
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

    print_header_one_sided(rank, options.win, options.sync, MPI_CHAR);

    switch (options.sync) {
        case LOCK:
            run_get_with_lock(rank, options.win);
            break;
        case PSCW:
            run_get_with_pscw(rank, options.win);
            break;
        case FENCE:
            run_get_with_fence(rank, options.win);
            break;
#if MPI_VERSION >= 3
        case LOCK_ALL:
            run_get_with_lock_all(rank, options.win);
            break;
        case FLUSH_LOCAL:
            run_get_with_flush_local(rank, options.win);
            break;
        default:
            run_get_with_flush(rank, options.win);
            break;
#endif
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

void print_bw(int rank, int size, double t, struct omb_stat_t omb_stat)
{
    if (rank == 0) {
        double tmp = size / 1e6 * options.iterations * options.window_size;
        fprintf(stdout, "%-*d%*.*f", 10, size, FIELD_WIDTH, FLOAT_PRECISION,
                tmp / t);
        if (options.omb_tail_lat) {
            OMB_ITR_PRINT_STAT(omb_stat.res_arr);
        }
        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

#if MPI_VERSION >= 3
/*Run GET with flush local */
void run_get_with_flush_local(int rank, enum WINDOW type)
{
    double t = 0.0;
    size_t size;
    int i, j;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    int papi_eventset = OMB_PAPI_NULL;
    omb_graph_data_t *omb_graph_data = NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    size_t window_size = options.window_size;
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_papi_init(&papi_eventset);
    for (size = options.min_message_size; size <= options.max_message_size;
         size *= options.message_size_incr) {
        allocate_memory_one_sided(rank, &rbuf, &win_base, size * window_size,
                                  type, &win);

        get_iterations(size);
        get_skips(size);

        if (type == WIN_DYNAMIC) {
            disp = disp_remote;
        }

        omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op,
                                               size, options.iterations);
        if (rank == 0) {
            MPI_CHECK(MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win));
            for (i = 0; i < options.skip + options.iterations; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                    t_start = MPI_Wtime();
                }
                if (i >= options.skip) {
                    t_graph_start = MPI_Wtime();
                }
                for (j = 0; j < window_size; j++) {
                    MPI_CHECK(MPI_Get(rbuf + (j * size), size, MPI_CHAR, 1,
                                      disp + (j * size), size, MPI_CHAR, win));
                }
                MPI_CHECK(MPI_Win_flush_local(1, win));
                if (i >= options.skip) {
                    t_graph_end = MPI_Wtime();
                    if (options.omb_tail_lat) {
                        omb_lat_arr[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                    if (options.graph) {
                        omb_graph_data->data[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                }
            }
            t_end = MPI_Wtime();
            MPI_CHECK(MPI_Win_unlock(1, win));
            t = t_end - t_start;
        }

        MPI_CHECK(MPI_Barrier(omb_comm));

        omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
        omb_papi_stop_and_print(&papi_eventset, size);
        print_bw(rank, size, t, omb_stat);
        if (options.graph && 0 == rank) {
            omb_graph_data->avg =
                (size / 1e6 * options.iterations * options.window_size) / t;
        }
        if (options.graph) {
            omb_graph_plot(&omb_graph_op, benchmark_name);
        }
        free_memory_one_sided(rbuf, win_base, type, win, rank);
    }
    omb_graph_combined_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free(omb_lat_arr);
}

/*Run GET with flush */
void run_get_with_flush(int rank, enum WINDOW type)
{
    double t = 0.0;
    size_t size;
    int i, j;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    int papi_eventset = OMB_PAPI_NULL;
    omb_graph_data_t *omb_graph_data = NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    size_t window_size = options.window_size;
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_papi_init(&papi_eventset);
    for (size = options.min_message_size; size <= options.max_message_size;
         size *= options.message_size_incr) {
        allocate_memory_one_sided(rank, &rbuf, &win_base, size * window_size,
                                  type, &win);

        get_iterations(size);
        get_skips(size);

        if (type == WIN_DYNAMIC) {
            disp = disp_remote;
        }

        omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op,
                                               size, options.iterations);
        if (rank == 0) {
            MPI_CHECK(MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win));
            for (i = 0; i < options.skip + options.iterations; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                    t_start = MPI_Wtime();
                }
                if (i >= options.skip) {
                    t_graph_start = MPI_Wtime();
                }
                for (j = 0; j < window_size; j++) {
                    MPI_CHECK(MPI_Get(rbuf + (j * size), size, MPI_CHAR, 1,
                                      disp + (j * size), size, MPI_CHAR, win));
                }
                MPI_CHECK(MPI_Win_flush(1, win));
                if (i >= options.skip) {
                    t_graph_end = MPI_Wtime();
                    if (options.omb_tail_lat) {
                        omb_lat_arr[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                    if (options.graph) {
                        omb_graph_data->data[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                }
            }
            t_end = MPI_Wtime();
            MPI_CHECK(MPI_Win_unlock(1, win));
            t = t_end - t_start;
        }

        MPI_CHECK(MPI_Barrier(omb_comm));

        omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
        omb_papi_stop_and_print(&papi_eventset, size);
        print_bw(rank, size, t, omb_stat);
        if (options.graph && 0 == rank) {
            omb_graph_data->avg =
                (size / 1e6 * options.iterations * options.window_size) / t;
        }
        if (options.graph) {
            omb_graph_plot(&omb_graph_op, benchmark_name);
        }
        free_memory_one_sided(rbuf, win_base, type, win, rank);
    }
    omb_graph_combined_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free(omb_lat_arr);
}

/*Run GET with Lock_all/unlock_all */
void run_get_with_lock_all(int rank, enum WINDOW type)
{
    double t = 0.0;
    size_t size;
    int i, j;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    int papi_eventset = OMB_PAPI_NULL;
    omb_graph_data_t *omb_graph_data = NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    size_t window_size = options.window_size;
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_papi_init(&papi_eventset);
    for (size = options.min_message_size; size <= options.max_message_size;
         size *= options.message_size_incr) {
        allocate_memory_one_sided(rank, &rbuf, &win_base, size * window_size,
                                  type, &win);

        get_iterations(size);
        get_skips(size);

        if (type == WIN_DYNAMIC) {
            disp = disp_remote;
        }

        omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op,
                                               size, options.iterations);
        if (rank == 0) {
            for (i = 0; i < options.skip + options.iterations; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                    t_start = MPI_Wtime();
                }
                if (i >= options.skip) {
                    t_graph_start = MPI_Wtime();
                }
                MPI_CHECK(MPI_Win_lock_all(0, win));
                for (j = 0; j < window_size; j++) {
                    MPI_CHECK(MPI_Get(rbuf + (j * size), size, MPI_CHAR, 1,
                                      disp + (j * size), size, MPI_CHAR, win));
                }
                MPI_CHECK(MPI_Win_unlock_all(win));
                if (i >= options.skip) {
                    t_graph_end = MPI_Wtime();
                    if (options.omb_tail_lat) {
                        omb_lat_arr[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                    if (options.graph) {
                        omb_graph_data->data[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                }
            }
            t_end = MPI_Wtime();
            t = t_end - t_start;
        }

        MPI_CHECK(MPI_Barrier(omb_comm));

        omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
        omb_papi_stop_and_print(&papi_eventset, size);
        print_bw(rank, size, t, omb_stat);
        if (options.graph && 0 == rank) {
            omb_graph_data->avg =
                (size / 1e6 * options.iterations * options.window_size) / t;
        }
        if (options.graph) {
            omb_graph_plot(&omb_graph_op, benchmark_name);
        }
        free_memory_one_sided(rbuf, win_base, type, win, rank);
    }
    omb_graph_combined_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free(omb_lat_arr);
}
#endif

/*Run GET with Lock/unlock */
void run_get_with_lock(int rank, enum WINDOW type)
{
    double t = 0.0;
    size_t size;
    int i, j;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    int papi_eventset = OMB_PAPI_NULL;
    omb_graph_data_t *omb_graph_data = NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    size_t window_size = options.window_size;
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_papi_init(&papi_eventset);
    for (size = options.min_message_size; size <= options.max_message_size;
         size *= options.message_size_incr) {
        allocate_memory_one_sided(rank, &rbuf, &win_base, size * window_size,
                                  type, &win);

        get_iterations(size);
        get_skips(size);

#if MPI_VERSION >= 3
        if (type == WIN_DYNAMIC) {
            disp = disp_remote;
        }
#endif

        omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op,
                                               size, options.iterations);
        if (rank == 0) {
            for (i = 0; i < options.skip + options.iterations; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                    t_start = MPI_Wtime();
                }
                if (i >= options.skip) {
                    t_graph_start = MPI_Wtime();
                }
                MPI_CHECK(MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win));
                for (j = 0; j < window_size; j++) {
                    MPI_CHECK(MPI_Get(rbuf + (j * size), size, MPI_CHAR, 1,
                                      disp + (j * size), size, MPI_CHAR, win));
                }
                MPI_CHECK(MPI_Win_unlock(1, win));
                if (i >= options.skip) {
                    t_graph_end = MPI_Wtime();
                    if (options.omb_tail_lat) {
                        omb_lat_arr[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                    if (options.graph) {
                        omb_graph_data->data[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                }
            }
            t_end = MPI_Wtime();
            t = t_end - t_start;
        }

        MPI_CHECK(MPI_Barrier(omb_comm));

        omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
        omb_papi_stop_and_print(&papi_eventset, size);
        print_bw(rank, size, t, omb_stat);
        if (options.graph && 0 == rank) {
            omb_graph_data->avg =
                (size / 1e6 * options.iterations * options.window_size) / t;
        }
        if (options.graph) {
            omb_graph_plot(&omb_graph_op, benchmark_name);
        }
        free_memory_one_sided(rbuf, win_base, type, win, rank);
    }
    omb_graph_combined_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free(omb_lat_arr);
}

/*Run GET with Fence */
void run_get_with_fence(int rank, enum WINDOW type)
{
    double t = 0.0;
    size_t size;
    int i, j;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    int papi_eventset = OMB_PAPI_NULL;
    omb_graph_data_t *omb_graph_data = NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    size_t window_size = options.window_size;
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_papi_init(&papi_eventset);
    for (size = options.min_message_size; size <= options.max_message_size;
         size *= options.message_size_incr) {
        allocate_memory_one_sided(rank, &rbuf, &win_base, size * window_size,
                                  type, &win);

        get_iterations(size);
        get_skips(size);

#if MPI_VERSION >= 3
        if (type == WIN_DYNAMIC) {
            disp = disp_remote;
        }
#endif

        omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op,
                                               size, options.iterations);
        MPI_CHECK(MPI_Barrier(omb_comm));

        if (rank == 0) {
            for (i = 0; i < options.skip + options.iterations; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                    t_start = MPI_Wtime();
                }
                if (i >= options.skip) {
                    t_graph_start = MPI_Wtime();
                }
                MPI_CHECK(MPI_Win_fence(0, win));
                for (j = 0; j < window_size; j++) {
                    MPI_CHECK(MPI_Get(rbuf + (j * size), size, MPI_CHAR, 1,
                                      disp + (j * size), size, MPI_CHAR, win));
                }
                MPI_CHECK(MPI_Win_fence(0, win));
                if (i >= options.skip) {
                    t_graph_end = MPI_Wtime();
                    if (options.omb_tail_lat) {
                        omb_lat_arr[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                    if (options.graph) {
                        omb_graph_data->data[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                }
            }
            t_end = MPI_Wtime();
            t = t_end - t_start;
        } else {
            for (i = 0; i < options.skip + options.iterations; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                }
                MPI_CHECK(MPI_Win_fence(0, win));
                MPI_CHECK(MPI_Win_fence(0, win));
            }
        }

        MPI_CHECK(MPI_Barrier(omb_comm));

        omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
        omb_papi_stop_and_print(&papi_eventset, size);
        print_bw(rank, size, t, omb_stat);
        if (options.graph && 0 == rank) {
            omb_graph_data->avg =
                (size / 1e6 * options.iterations * options.window_size) / t;
        }
        if (options.graph) {
            omb_graph_plot(&omb_graph_op, benchmark_name);
        }
        free_memory_one_sided(rbuf, win_base, type, win, rank);
    }
    omb_graph_combined_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free(omb_lat_arr);
}

/*Run GET with Post/Start/Complete/Wait */
void run_get_with_pscw(int rank, enum WINDOW type)
{
    double t = 0.0;
    size_t size;
    int destrank, i, j;
    double t_graph_start = 0.0, t_graph_end = 0.0;
    int papi_eventset = OMB_PAPI_NULL;
    omb_graph_data_t *omb_graph_data = NULL;
    MPI_Aint disp = 0;
    MPI_Win win;
    MPI_Group comm_group, group;
    MPI_CHECK(MPI_Comm_group(omb_comm, &comm_group));
    double *omb_lat_arr = NULL;
    struct omb_stat_t omb_stat;

    size_t window_size = options.window_size;
    if (options.omb_tail_lat) {
        omb_lat_arr = malloc(options.iterations * sizeof(double));
        OMB_CHECK_NULL_AND_EXIT(omb_lat_arr, "Unable to allocate memory");
    }
    omb_papi_init(&papi_eventset);
    for (size = options.min_message_size; size <= options.max_message_size;
         size *= options.message_size_incr) {
        allocate_memory_one_sided(rank, &rbuf, &win_base, size * window_size,
                                  type, &win);

        get_iterations(size);
        get_skips(size);

#if MPI_VERSION >= 3
        if (type == WIN_DYNAMIC) {
            disp = disp_remote;
        }
#endif

        omb_graph_allocate_and_get_data_buffer(&omb_graph_data, &omb_graph_op,
                                               size, options.iterations);
        MPI_CHECK(MPI_Barrier(omb_comm));

        if (rank == 0) {
            destrank = 1;
            MPI_CHECK(MPI_Group_incl(comm_group, 1, &destrank, &group));
            for (i = 0; i < options.skip + options.iterations; i++) {
                MPI_CHECK(MPI_Win_start(group, 0, win));
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                    t_start = MPI_Wtime();
                }
                if (i >= options.skip) {
                    t_graph_start = MPI_Wtime();
                }
                for (j = 0; j < window_size; j++) {
                    MPI_CHECK(MPI_Get(rbuf + j * size, size, MPI_CHAR, 1,
                                      disp + (j * size), size, MPI_CHAR, win));
                }
                MPI_CHECK(MPI_Win_complete(win));
                if (i >= options.skip) {
                    t_graph_end = MPI_Wtime();
                    if (options.omb_tail_lat) {
                        omb_lat_arr[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                    if (options.graph) {
                        omb_graph_data->data[i - options.skip] =
                            (size / 1e6) * options.window_size /
                            (t_graph_end - t_graph_start);
                    }
                }
            }
            t_end = MPI_Wtime();
            t = t_end - t_start;
        } else {
            destrank = 0;
            MPI_CHECK(MPI_Group_incl(comm_group, 1, &destrank, &group));
            for (i = 0; i < options.skip + options.iterations; i++) {
                if (i == options.skip) {
                    omb_papi_start(&papi_eventset);
                }
                MPI_CHECK(MPI_Win_post(group, 0, win));
                MPI_CHECK(MPI_Win_wait(win));
            }
        }

        MPI_CHECK(MPI_Barrier(omb_comm));

        omb_stat = omb_calculate_tail_lat(omb_lat_arr, rank, 1);
        omb_papi_stop_and_print(&papi_eventset, size);
        print_bw(rank, size, t, omb_stat);
        if (options.graph && 0 == rank) {
            omb_graph_data->avg =
                (size / 1e6 * options.iterations * options.window_size) / t;
        }
        if (options.graph) {
            omb_graph_plot(&omb_graph_op, benchmark_name);
        }
        MPI_CHECK(MPI_Group_free(&group));

        free_memory_one_sided(rbuf, win_base, type, win, rank);
    }
    omb_graph_combined_plot(&omb_graph_op, benchmark_name);
    omb_graph_free_data_buffers(&omb_graph_op);
    omb_papi_free(&papi_eventset);
    free(omb_lat_arr);
    MPI_CHECK(MPI_Group_free(&comm_group));
}
/* vi: set sw=4 sts=4 tw=80: */
