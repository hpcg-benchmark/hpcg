OSU Micro-Benchmarks for Java 
------------------------------

This package is an addition to the OMB benchmarking suite that
provides a testing platform for Java MPI libraries, such as
MVAPICH2-J and the Open MPI Java bindings. The package consists of
point-to-point and blocking collective benchmarks. It
supports testing communication of Java arrays (of basic Java datatypes)
along with NIO ByteBuffers---these are referred to as ``Arrays API''
and ``Buffer API'' respectively. The package currently only 
supports CPU-based communication. 

The following is a quick guide to compiling and running benchmarks using
MVAPICH2-J and Open MPI Java bindings. 

For instructions for building MVAPICH2-J, please follow 
http://mvapich.cse.ohio-state.edu/userguide/mv2j/.

Compiling and Running benchmarks with MVAPICH2-J:
-------------------------------------------------

1) Install and build MVAPICH2-J using the following guide, 
   http://mvapich.cse.ohio-state.edu/userguide/mv2j/.

2) Compile benchmarks Latency, Bandiwdth, and Allgather benchmarks with MVAPICH2-J:
   $ javac -cp $MV2J_HOME/lib/mvapich2-j.jar:. mpi/common/BenchmarkUtils.java \
         mpi/pt2pt/OSULatency.java mpi/pt2pt/OSUBandwidth.java \
         mpi/collective/OSUAllgather.java

3) Run OSULatency benchmark:
   3.a) Running the benchmark with Buffer API using Java NIO ByteBuffers:
        $ mpirun_rsh -np 2 -hostfile hosts \
            LD_PRELOAD=${MPILIB}/lib/libmpi.so java -cp $MV2J_HOME/lib/mvapich2-j.jar:. \
            -Djava.library.path=$MV2J_HOME/lib mpi.pt2pt.OSULatency
  
   3.b) To run the benchmark with Arrays API using Java arrays:
        $ mpirun_rsh -np 2 -hostfile hosts \
            LD_PRELOAD=${MPILIB}/lib/libmpi.so java -cp $MV2J_HOME/lib/mvapich2-j.jar:. \
            -Djava.library.path=$MV2J_HOME/lib mpi.pt2pt.OSULatency -a arrays

4) Run OSUBandwidth benchmark:    
   4.a) Running the benchmark with Buffer API using Java NIO ByteBuffers:
        $ mpirun_rsh -np 2 -hostfile hosts \
            LD_PRELOAD=${MPILIB}/lib/libmpi.so java -cp $MV2J_HOME/lib/mvapich2-j.jar:. \
            -Djava.library.path=$MV2J_HOME/lib mpi.pt2pt.OSUBandwidth
  
   4.b) To run the benchmark with Arrays API using Java arrays:
        $ mpirun_rsh -np 2 -hostfile hosts \
            LD_PRELOAD=${MPILIB}/lib/libmpi.so java -cp $MV2J_HOME/lib/mvapich2-j.jar:. \
            -Djava.library.path=$MV2J_HOME/lib mpi.pt2pt.OSUBandwidth -a arrays

5) Run OSUAllgather benchmark:
   5.a) Running the benchmark with Buffer API using Java NIO ByteBuffers:
        $ mpirun_rsh -np 8 -hostfile hosts \
            LD_PRELOAD=$MPILIB/lib/libmpi.so java -cp $MV2J_HOME/lib/mvapich2-j.jar:. \
            -Djava.library.path=$MV2J_HOME/lib mpi.collective.OSUAllgather
   
   5.b) Running the benchmark with Arrays API using Java arrays:
        $ mpirun_rsh -np 8 -hostfile hosts \
            LD_PRELOAD=$MPILIB/lib/libmpi.so java -cp $MV2J_HOME/lib/mvapich2-j.jar:. \
            -Djava.library.path=$MV2J_HOME/lib mpi.collective.OSUAllgather -a arrays

Note that same compile and run commands are used for running other point-to-point
and collective benchmarks.

Compiling and Running benchmarks with Open MPI Java bindings:
-------------------------------------------------------------

The Open MPI Java bindings do not support communicating Java arrays 
for non-blocking point-to-point operations. For this reason, custom
benchmarks (OSUBandwidthOMPI and OSUBiBandwidthOMPI) were written 
to measure bandwidth and bi-bandwidth, respectively.

1) Compile with Open MPI Java Bindings:
   $ javac -cp path/to/mpi.jar:. mpi/common/BenchmarkUtils.java \
        mpi/pt2pt/OSULatency.java mpi/pt2pt/OSUBandwidthOMPI.java \
        mpi/collective/OSUAllgather.java 

2) Running OSULatency benchmark:
   2.a) Running the benchmark with Buffer API using NIO ByteBuffers:
        $ mpirun -np 2 --hostfile hosts -mca pml ucx --mca btl \ 
            ^vader,openib,uct -x UCX_NET_DEVICES=mlx5_0:1 \
            java -cp ${OMPILIB}/lib/mpi.jar:. mpi.pt2pt.OSULatency

   2.b) Running the benchmark with Arrays API using Java arrays:
        $ mpirun -np 2 --hostfile hosts -mca pml ucx --mca btl \ 
            ^vader,openib,uct -x UCX_NET_DEVICES=mlx5_0:1 \
            java -cp ${OMPILIB}/lib/mpi.jar:. mpi.pt2pt.OSULatency -a arrays


3) Running OSUBandwidth benchmark:
   3.a) Running the benchmark with Buffer API using NIO ByteBuffers:
        $ mpirun -np 2 --hostfile hosts -mca pml ucx --mca btl \ 
            ^vader,openib,uct -x UCX_NET_DEVICES=mlx5_0:1 \
            java -cp ${OMPILIB}/lib/mpi.jar:. mpi.pt2pt.OSUBandwidthOMPI

4) Running OSUAllgather benchmark:
   4.a) Running the benchmark with Buffer API using NIO ByteBuffers:
        $ mpirun -np 2 --hostfile hosts -mca pml ucx --mca btl \ 
            ^vader,openib,uct -x UCX_NET_DEVICES=mlx5_0:1 \
            java -cp ${OMPILIB}/lib/mpi.jar:. mpi.collective.OSUAllgather

   4.b) Running the benchmark with Arrays API using Java arrays:
        $ mpirun -np 2 --hostfile hosts -mca pml ucx --mca btl \ 
            ^vader,openib,uct -x UCX_NET_DEVICES=mlx5_0:1 \
            java -cp ${OMPILIB}/lib/mpi.jar:. mpi.collective.OSUAllgather \
            -a arrays


Command Line Arguments:
-----------------------

The following command line options can be provided to the benchmarks:

   -m, --message-size [MIN:]MAX  set the minimum and/or the 
                                 maximum message size to MIN and/or MAX
                                 bytes respectively. 
                                 Examples:
                                 -m 128      // min = default, max = 128
                                 -m 2:128    // min = 2, max = 128
                                 -m 2:       // min = 2, max = default
   
   -x, --warmup ITER             number of warmup iterations to skip before
                                 timing (default 10000)

   -i, --iterations ITER         number of iterations for timing
                                 (default 10000)

   -W, --window-size SIZE        set number of messages to send before
                                 synchronization (default 64)

   -a API                        the api to use for exchanging data. Options
                                 are 'buffer' or 'arrays' APIs. (default buffer)

   -c, --validation              validates exchanged data

   -h, --help                    print this help message
   
   -v, --version                 print the version info
