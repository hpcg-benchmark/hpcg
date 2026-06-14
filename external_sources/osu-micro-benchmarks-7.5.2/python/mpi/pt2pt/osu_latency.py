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

def osu_latency(args):

   
    comm = MPI.COMM_WORLD
    myid = comm.Get_rank()
    numprocs = comm.Get_size()

    options = Options("Latency", args)
    util.check_numprocs(numprocs, myid, limit=2)

    structure = util.find_structure(options.buffer)
    s_buf = util.allocate(options.max_message_size, structure)
    r_buf = util.allocate(options.max_message_size*2, structure)
    util.print_header(options.benchmark, myid)

    for size in util.message_sizes(options):
        if size > options.large_message_size:
            options.skip = options.skip_large
            options.iterations = options.iterations_large
        iterations = list(range(options.iterations+options.skip))
        s_msg = [s_buf, size, MPI.BYTE]
        r_msg = [r_buf, size, MPI.BYTE]

        if options.pickle:
            s_msg = s_buf[0:size]
            comm.Barrier()
            if myid == 0:
                for i in iterations:
                    if i == options.skip:
                        t_start = MPI.Wtime()
                    comm.send(s_msg, 1, 1)
                    r_msg = comm.recv(source=1, tag=1)
                t_end = MPI.Wtime()
            elif myid == 1:
                for i in iterations:
                    if i == options.skip:
                        t_start = MPI.Wtime()
                    r_msg = comm.recv(source=0, tag=1)
                    comm.send(s_msg, 0, 1)
            t_end = MPI.Wtime()
        else:
            comm.Barrier()
            if myid == 0:
                for i in iterations:
                    if i == options.skip:
                        t_start = MPI.Wtime()
                    comm.Send(s_msg, 1, 1)
                    comm.Recv(r_msg, 1, 1)
                t_end = MPI.Wtime()
            elif myid == 1:
                for i in iterations:
                    if i == options.skip:
                        t_start = MPI.Wtime()
                    comm.Recv(r_msg, 0, 1)
                    comm.Send(s_msg, 0, 1)
            t_end = MPI.Wtime()
        
        util.print_stats(t_end, t_start, 2*options.iterations, myid, comm, numprocs, size)
