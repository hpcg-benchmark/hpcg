"""
Copyright (c) 2002-2022 the Network-Based Computing Laboratory
(NBCL), The Ohio State University.

Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)

For detailed copyright and licensing information, please refer to the
copyright file COPYRIGHT in the top level OMB directory.
"""

import sys
import util.parser as argparser
import numpy as np
from mpi4py import MPI
from mpi.collective import osu_allgather
from mpi.collective import osu_allgatherv
from mpi.collective import osu_allreduce
from mpi.collective import osu_alltoall
from mpi.collective import osu_alltoallv
from mpi.collective import osu_barrier
from mpi.collective import osu_bcast
from mpi.collective import osu_gather
from mpi.collective import osu_gatherv
from mpi.collective import osu_reduce_scatter
from mpi.collective import osu_reduce
from mpi.collective import osu_scatter
from mpi.collective import osu_scatterv
from mpi.pt2pt import osu_bibw
from mpi.pt2pt import osu_bw
from mpi.pt2pt import osu_latency
from mpi.pt2pt import osu_multi_lat

args = argparser.get_parser().parse_args()
benchmark = args.benchmark

if "/" in benchmark:
    benchmark = benchmark.split("/")[1]
if "osu_" in benchmark:
    benchmark = benchmark.replace('osu_','')

if(benchmark=='allgather'):
    osu_allgather.osu_allgather(args=args)
elif(benchmark=='allgatherv'):
    osu_allgatherv.osu_allgatherv(args=args)
elif(benchmark=='allreduce'):
    osu_allreduce.osu_allreduce(args=args)
elif(benchmark=='alltoall'):
    osu_alltoall.osu_alltoall(args=args)
elif(benchmark=='alltoallv'):
    osu_alltoallv.osu_alltoallv(args=args)
elif(benchmark=='barrier'):
    osu_barrier.osu_barrier(args)
elif(benchmark=='bcast'):
    osu_bcast.osu_bcast(args=args)
elif(benchmark=='gather'):
    osu_gather.osu_gather(args=args)
elif(benchmark=='gatherv'):
    osu_gatherv.osu_gatherv(args=args)
elif(benchmark=='reduce_scatter'):
    osu_reduce_scatter.osu_reduce_scatter(args=args)
elif(benchmark=='reduce'):
    osu_reduce.osu_reduce(args=args)
elif(benchmark=='scatter'):
    osu_scatter.osu_scatter(args=args)
elif(benchmark=='scatterv'):
    osu_scatterv.osu_scatterv(args=args)
elif(benchmark=='async_lat'):
    async_osu_latency.async_osu_latency(args=args)
elif(benchmark=='bibw'):
    osu_bibw.osu_bibw(args=args)
elif(benchmark=='bw'):
    osu_bw.osu_bw(args=args)
elif(benchmark=='latency'):
    osu_latency.osu_latency(args=args)
elif(benchmark=='multi_lat'):
    osu_multi_lat.osu_multi_lat(args=args)
