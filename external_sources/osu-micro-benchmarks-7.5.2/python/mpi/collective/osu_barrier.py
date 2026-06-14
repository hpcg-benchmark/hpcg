"""
Copyright (c) 2002-2022 the Network-Based Computing Laboratory
(NBCL), The Ohio State University.

Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)

For detailed copyright and licensing information, please refer to the
copyright file COPYRIGHT in the top level OMB directory.
"""

from mpi4py import MPI
from util.osu_util_mpi import util
from util.options import Options


def osu_barrier(args):

    comm = MPI.COMM_WORLD
    myid = comm.Get_rank()
    numprocs = comm.Get_size()

    options = Options("Barrier", args)
    util.check_numprocs(numprocs, myid, limit=3)
    util.print_header(options.benchmark, myid)

    iterations = list(range(options.iterations+options.skip))

    comm.Barrier()
    for i in iterations:
        if i == options.skip:
            t_start = MPI.Wtime()
        comm.Barrier()
    t_end = MPI.Wtime()
    comm.Barrier()

    avg_lat = util.avg_lat(t_end, t_start, options.iterations, comm, numprocs)
    if myid == 0:
        print('%-10d%18.2f' % (0, avg_lat))
