"""
Copyright (c) 2002-2022 the Network-Based Computing Laboratory
(NBCL), The Ohio State University.

Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)

For detailed copyright and licensing information, please refer to the
copyright file COPYRIGHT in the top level OMB directory.
"""

from mpi4py import MPI
import numpy as np
import math


class util:
    def print_stats(t_end, t_start, iterations, myid, comm, numprocs, size):
        avglatency = util.avg_lat(t_end, t_start, iterations, comm, numprocs)
        if myid == 0:
            print('%-10d%18.2f' % (size, avglatency), flush=True)

    def avg_lat(t_end, t_start, iterations, comm, numprocs):
        latency = np.array((t_end - t_start) * 1e6 / iterations, 'd')
        avglatency = np.array(0.0, 'd')
        comm.Reduce(
            [latency, MPI.DOUBLE],
            [avglatency, MPI.DOUBLE],
            op=MPI.SUM, root=0)
        avglatency /= numprocs
        return avglatency

    def print_header(benchmark, myid):
        if myid == 0:
            print('# OMB Python MPI %s Test' % (benchmark))
            print('# %-8s%18s' % ("Size (B)", "Latency (us)"))

    def check_numprocs(numprocs, myid, limit):
        if limit == 2:
            if numprocs != 2:
                if myid == 0:
                    errmsg = "This test requires exactly two processes"
                else:
                    errmsg = None
                raise SystemExit(errmsg)
        else:
            if numprocs < 2:
                if myid == 0:
                    errmsg = "This test requires at least two processes"
                else:
                    errmsg = None
                raise SystemExit(errmsg)

    def nbc_print_stats(
            rank, size, numprocs, loop, comm, timer, latency, test_time,
            tcomp_time, wait_time, init_time):
        overall_avg, tcomp_avg, test_avg, avg_comm_time, wait_avg, init_avg = util.nbc_calc_stats(
            size, numprocs, loop, comm, timer, latency, test_time, tcomp_time, wait_time, init_time)
        if rank == 0:
            overlap = max(
                0, 100 -
                (((overall_avg - (tcomp_avg - test_avg)) / avg_comm_time) * 100))
            print(
                '%-10d%18.2f%18.2f%18.2f%18.2f%18.2f%18.2f' %
                (size, overall_avg, (tcomp_avg - test_avg),
                 avg_comm_time, overlap, wait_avg, init_avg),
                flush=True)

    def nbc_print_header(rank):
        if rank == 0:
            print(
                "# Size           Overall(us)       Compute(us)    Pure Comm.(us)        Overlap(%)      Wait avg(us)      Init avg(us)",
                flush=True)

    def nbc_calc_stats(
            size, numprocs, loop, comm, timer, latency, test_time, tcomp_time,
            wait_time, init_time):

        test_total_s = np.array((test_time * 1e6) / loop, 'd')
        tcomp_total_s = np.array((tcomp_time * 1e6) / loop, 'd')
        overall_time_s = np.array((timer * 1e6) / loop, 'd')
        wait_total_s = np.array((wait_time * 1e6) / loop, 'd')
        init_total_s = np.array((init_time * 1e6) / loop, 'd')
        avg_comm_time_s = np.array(latency, 'd')
        min_comm_time_s = np.array(latency, 'd')
        max_comm_time_s = np.array(latency, 'd')

        test_total = np.array(0, 'd')
        tcomp_total = np.array(0, 'd')
        overall_time = np.array(0, 'd')
        wait_total = np.array(0, 'd')
        init_total = np.array(0, 'd')
        avg_comm_time = np.array(0, 'd')
        min_comm_time = np.array(0, 'd')
        max_comm_time = np.array(0, 'd')
        comm.Reduce(
            [test_total_s, MPI.DOUBLE],
            [test_total, MPI.DOUBLE],
            op=MPI.SUM, root=0)
        comm.Reduce(
            [avg_comm_time_s, MPI.DOUBLE],
            [avg_comm_time, MPI.DOUBLE],
            op=MPI.SUM, root=0)
        comm.Reduce(
            [overall_time_s, MPI.DOUBLE],
            [overall_time, MPI.DOUBLE],
            op=MPI.SUM, root=0)
        comm.Reduce(
            [tcomp_total_s, MPI.DOUBLE],
            [tcomp_total, MPI.DOUBLE],
            op=MPI.SUM, root=0)
        comm.Reduce(
            [wait_total_s, MPI.DOUBLE],
            [wait_total, MPI.DOUBLE],
            op=MPI.SUM, root=0)
        comm.Reduce(
            [init_total_s, MPI.DOUBLE],
            [init_total, MPI.DOUBLE],
            op=MPI.SUM, root=0)
        comm.Reduce(
            [max_comm_time_s, MPI.DOUBLE],
            [max_comm_time, MPI.DOUBLE],
            op=MPI.SUM, root=0)
        comm.Reduce(
            [min_comm_time_s, MPI.DOUBLE],
            [min_comm_time, MPI.DOUBLE],
            op=MPI.SUM, root=0)
        comm.Barrier()

        overall_time = overall_time/numprocs
        tcomp_total = tcomp_total/numprocs
        test_total = test_total/numprocs
        avg_comm_time = avg_comm_time/numprocs
        wait_total = wait_total/numprocs
        init_total = init_total/numprocs

        return overall_time, tcomp_total, test_total, avg_comm_time, wait_total, init_total

    def message_sizes(options):
        max_size = int(math.log(options.max_message_size, 2)) + 1
        if(options.min_message_size > 0):
            min_size = int(math.log(options.min_message_size, 2))
        else:
            min_size = 0
        message_sizes = [(1 << i) for i in range(min_size, max_size)]
        if(options.min_message_size == 0):
            message_sizes = [0] + message_sizes
        return message_sizes

    def allocate(n, dtype='bytearray'):

        if "cupy" in dtype:
            import cupy as cp
        elif "numba" in dtype:
            from numba import cuda
        elif "pycuda" in dtype:
            import pycuda.driver as drv
            import pycuda.gpuarray as gpuarray
            import pycuda.autoinit

        if(dtype == 'bytearray'):
            return bytearray(n)
        elif(dtype == 'numpyB'):
            return np.zeros(n, 'B')
        elif(dtype == 'numpyF'):
            return np.zeros(n, 'F')
        elif(dtype == 'cupyB'):
            return cp.arange(n, dtype='u1')
        elif(dtype == 'cupyF'):
            return cp.arange(n, dtype='F')
        elif(dtype == 'numbaB'):
            ary = np.zeros(n, 'B')
            return cuda.to_device(ary)
        elif(dtype == 'numbaF'):
            ary = np.zeros(n, 'F')
            return cuda.to_device(ary)
        elif(dtype == 'pycudaB'):
            return gpuarray.zeros(n, dtype=np.byte)
        elif(dtype == 'pycudaF'):
            return gpuarray.zeros(n, dtype=np.float32)

    def find_structure(mode, reduce=False):
        if reduce:
            structure = 'numpyF'
            if(mode == 'cupy'):
                structure = 'cupyF'
            elif(mode == 'numba'):
                structure = 'numbaF'
            elif(mode == 'pycuda'):
                structure = 'pycudaF'
        else:
            structure = 'bytearray'
            if(mode == 'numpy'):
                structure = 'numpyB'
            elif(mode == 'cupy'):
                structure = 'cupyB'
            elif(mode == 'numba'):
                structure = 'numbaB'
            elif(mode == 'pycuda'):
                structure = "pycudaB"
        return structure

    def dummy_compute(seconds, request, mode):
        test_time = 0
        test_time = util.do_compute_and_probe(seconds, request, mode)
        return test_time

    def do_compute_and_probe(seconds, request, mode='cpu'):
        test_time = 0
        if mode == 'cpu':
            util.do_compute_cpu(seconds)
        elif mode == 'gpu':
            util.do_compute_gpu(seconds)
        return test_time

    def do_compute_cpu(seconds):
        t1 = 0
        t2 = 0
        time_elapsed = 0
        t1 = MPI.Wtime()
        while (time_elapsed < seconds):
            util.compute_on_host()
            t2 = MPI.Wtime()
            time_elapsed = (t2-t1)