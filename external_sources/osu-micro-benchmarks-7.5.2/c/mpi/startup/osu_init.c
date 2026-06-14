#define BENCHMARK "OSU MPI Init Test"
#ifdef PACKAGE_VERSION
#define HEADER "# " BENCHMARK " v" PACKAGE_VERSION "\n"
#else
#define HEADER "# " BENCHMARK "\n"
#endif
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
    int myid, numprocs;
    struct timespec tp_before, tp_after;
    long duration = 0, min, max, avg;
    options.bench = STARTUP;
    options.subtype = INIT;
    MPI_Comm omb_comm = MPI_COMM_NULL;
    omb_mpi_init_data omb_init_h;
    int po_ret = 0;

    po_ret = process_options(argc, argv);
    if (PO_OKAY == po_ret && NONE != options.accel) {
        if (init_accel()) {
            fprintf(stderr, "Error initializing device\n");
            exit(EXIT_FAILURE);
        }
    }
    clock_gettime(CLOCK_REALTIME, &tp_before);
    omb_init_h = omb_mpi_init(&argc, &argv);
    clock_gettime(CLOCK_REALTIME, &tp_after);

    duration = (tp_after.tv_sec - tp_before.tv_sec) * 1e3;
    duration += (tp_after.tv_nsec - tp_before.tv_nsec) / 1e6;

    omb_comm = omb_init_h.omb_comm;
    if (MPI_COMM_NULL == omb_comm) {
        OMB_ERROR_EXIT("Cant create communicator");
    }
    MPI_CHECK(MPI_Comm_rank(omb_comm, &myid));
    MPI_CHECK(MPI_Comm_size(omb_comm, &numprocs));

    MPI_Reduce(&duration, &min, 1, MPI_LONG, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&duration, &max, 1, MPI_LONG, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&duration, &avg, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    avg = avg / numprocs;

    if (myid == 0) {
        fprintf(stdout, HEADER);
        fprintf(stdout, "nprocs: %d, min: %ld ms, max: %ld ms, avg: %ld ms\n",
                numprocs, min, max, avg);
        fflush(stdout);
    }

    omb_mpi_finalize(omb_init_h);

    return EXIT_SUCCESS;
}
