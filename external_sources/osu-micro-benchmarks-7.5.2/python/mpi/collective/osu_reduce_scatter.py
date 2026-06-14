"""
Copyright (c) 2002-2022 the Network-Based Computing Laboratory
(NBCL), The Ohio State University.

Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)

For detailed copyright and licensing information, please refer to the
copyright file COPYRIGHT in the top level OMB directory.
"""

from mpi4py import MPI
import numpy as np
from util.osu_util_mpi import util
from util.options import Options


def osu_reduce_scatter(args):

    comm = MPI.COMM_WORLD
    myid = comm.Get_rank()
    numprocs = comm.Get_size()

    options = Options("Reduce_scatter", args)
    util.check_numprocs(numprocs, myid, limit=3)
    util.print_header(options.benchmark, myid)

    structure = util.find_structure(options.buffer, reduce=True)
    r_buf = util.allocate(
        int(options.max_message_size / (4 * numprocs) + 1),
        structure)
    s_buf = util.allocate(int(options.max_message_size/4), structure)

    for size in util.message_sizes(options):
        if size > options.large_message_size:
            options.skip = options.skip_large
            options.iterations = options.iterations_large
        iterations = list(range(options.iterations+options.skip))

        recvcounts = np.zeros(numprocs)
        portion = 0
        remainder = 0
        portion = (size/4)/numprocs
        remainder = (size/4) % numprocs
        for i in range(numprocs):
            recvcounts[i] = 0
            if (size/4) < numprocs:
                if i < (size/4):
                    recvcounts[i] = 1
            else:
                if((remainder != 0) and (i < remainder)):
                    recvcounts[i] += 1
                recvcounts[i] += portion

        s_msg = [s_buf, size/4, MPI.FLOAT]
        r_msg = [r_buf, recvcounts[myid], MPI.FLOAT]

        comm.Barrier()
        for i in iterations:
            if i == options.skip:
                t_start = MPI.Wtime()
            comm.Reduce_scatter(s_msg, r_msg, recvcounts=recvcounts, op=MPI.SUM)
        t_end = MPI.Wtime()
        comm.Barrier()

        util.print_stats(t_end, t_start, options.iterations,
                         myid, comm, numprocs, size)
