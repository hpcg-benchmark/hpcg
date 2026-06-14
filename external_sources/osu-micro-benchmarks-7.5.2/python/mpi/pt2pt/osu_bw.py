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

def osu_bw(args):

    comm = MPI.COMM_WORLD
    myid = comm.Get_rank()
    numprocs = comm.Get_size()

    options = Options("Bandwidth", args)
    util.check_numprocs(numprocs, myid, limit=2)

    structure = util.find_structure(options.buffer)

    s_buf = util.allocate(options.max_message_size, structure)
    r_buf = util.allocate(options.max_message_size*2, structure)


    if myid == 0:
        print ('# OMB-Py MPI %s Test' % (options.benchmark))
        print ('# %-8s%18s' % ("Size (B)", "Bandwidth (MB/s)"))

    window_size = 64
    for size in util.message_sizes(options):
        if size > options.large_message_size:
            options.skip = options.skip_large
            options.iterations = options.iterations_large

        iterations = list(range(options.iterations+options.skip))
        window_sizes = list(range(window_size))
        requests = [MPI.REQUEST_NULL] * window_size
        
        #TODO review pickle code
        if options.pickle:
            comm.Barrier()
            if myid == 0:
                s_msg = s_buf[0:size]
                for i in iterations:
                    if i == options.skip:
                        t_start = MPI.Wtime()
                    for j in window_sizes:
                        requests[j] = comm.isend(s_msg, 1, 100)
                    MPI.Request.Waitall(requests)
                    r_msg = comm.recv(source=1, tag=101)
                t_end = MPI.Wtime()
            elif myid == 1:
                s_msg = s_buf[0:4]
                r_msg = r_buf
                for i in iterations:
                    for j in window_sizes:
                        requests[j] = comm.irecv(r_msg, 0, 100)
                    MPI.Request.Waitall(requests)
                    comm.send(s_msg, 0, 101)
        else:
            comm.Barrier()
            if myid == 0:
                s_msg = [s_buf, size, MPI.BYTE]
                r_msg = [r_buf,    4, MPI.BYTE]
                for i in iterations:
                    if i == options.skip:
                        t_start = MPI.Wtime()
                    for j in window_sizes:
                        requests[j] = comm.Isend(s_msg, 1, 100)
                    MPI.Request.Waitall(requests)
                    comm.Recv(r_msg, 1, 101)
                t_end = MPI.Wtime()
            elif myid == 1:
                s_msg = [s_buf,    4, MPI.BYTE]
                r_msg = [r_buf, size, MPI.BYTE]
                for i in iterations:
                    for j in window_sizes:
                        requests[j] = comm.Irecv(r_msg, 0, 100)
                    MPI.Request.Waitall(requests)
                    comm.Send(s_msg, 0, 101)

        if myid == 0:
            bw = size / 1e6 * options.iterations * window_size
            time = t_end - t_start
            print ('%-10d%18.2f' % (size, bw/time), flush=True)