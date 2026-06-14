#define BENCHMARK "OSU MPI Hello World Test"
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
#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int myid, numprocs;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    if (myid == 0) {
        fprintf(stdout, HEADER);
        fprintf(stdout, "This is a test with %d processes\n", numprocs);
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}
