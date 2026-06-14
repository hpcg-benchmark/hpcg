/*
 * Copyright (c) 2023-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */
#ifndef OMB_UTIL_OP_H
#define OMB_UTIL_OP_H                 1
#define OMBOP_GET_NONACCEL_NAME(A, B) OMBOP__##A##__##B
#define OMBOP_GET_ACCEL_NAME(A, B)    OMBOP__ACCEL__##A##__##B
#define OMBOP_OPTSTR_BLK(bench, subtype)                                       \
    if (accel_enabled) {                                                       \
        options.optstring = OMBOP_GET_ACCEL_NAME(bench, subtype);              \
    } else {                                                                   \
        options.optstring = OMBOP_GET_NONACCEL_NAME(bench, subtype);           \
    }
#define OMBOP_OPTSTR_CUDA_BLK(bench, subtype)                                  \
    if (accel_enabled) {                                                       \
        options.optstring = (CUDA_KERNEL_ENABLED) ?                            \
                                OMBOP_GET_ACCEL_NAME(bench, subtype) "r:" :    \
                                OMBOP_GET_ACCEL_NAME(bench, subtype);          \
    } else {                                                                   \
        options.optstring = OMBOP_GET_NONACCEL_NAME(bench, subtype);           \
    }
#define OMBOP_LONG_OPTIONS_ALL                                                 \
    {{"help", no_argument, 0, 'h'},                                            \
     {"version", no_argument, 0, 'v'},                                         \
     {"full", no_argument, 0, 'f'},                                            \
     {"message-size", required_argument, 0, 'm'},                              \
     {"window-size", required_argument, 0, 'W'},                               \
     {"num-test-calls", required_argument, 0, 't'},                            \
     {"iterations", required_argument, 0, 'i'},                                \
     {"warmup", required_argument, 0, 'x'},                                    \
     {"array-size", required_argument, 0, 'a'},                                \
     {"sync-option", required_argument, 0, 's'},                               \
     {"win-options", required_argument, 0, 'w'},                               \
     {"accelerator", required_argument, 0, 'd'},                               \
     {"cuda-target", required_argument, 0, 'r'},                               \
     {"print-rate", required_argument, 0, 'R'},                                \
     {"num-pairs", required_argument, 0, 'p'},                                 \
     {"vary-window", required_argument, 0, 'V'},                               \
     {"validation", optional_argument, 0, 'c'},                                \
     {"buffer-num", required_argument, 0, 'b'},                                \
     {"validation-warmup", required_argument, 0, 'u'},                         \
     {"graph", required_argument, 0, 'G'},                                     \
     {"papi", required_argument, 0, 'P'},                                      \
     {"ddt", required_argument, 0, 'D'},                                       \
     {"nhbr", required_argument, 0, 'N'},                                      \
     {"type", required_argument, 0, 'T'},                                      \
     {"session", no_argument, 0, 'I'},                                         \
     {"in-place", no_argument, 0, 'l'},                                        \
     {"tail-lat", optional_argument, 0, 'z'},                                  \
     {"partitions", optional_argument, 0, 'q'},                                \
     {"root-rank", required_argument, 0, 'k'}}
/*OMBOP[__ACCEL]__<options.bench>__<options.subtype>*/
#define OMBOP__PT2PT__LAT                    "+:hvm:x:i:b:c::u:G:D:P:T:Iz::"
#define OMBOP__PT2PT__PART_LAT               "+:hvm:x:i:b:c::u:G:D:P:T:Iz::q:"
#define OMBOP__ACCEL__PT2PT__LAT             "+:x:i:m:d:hvc::u:G:D:T:Iz::"
#define OMBOP__ACCEL__PT2PT__PART_LAT        "+:x:i:m:d:hvc::u:G:D:T:Iz::q:"
#define OMBOP__PT2PT__BW                     "+:hvm:x:i:t:W:b:c::u:G:D:P:T:Iz::"
#define OMBOP__ACCEL__PT2PT__BW              "+:x:i:t:m:d:W:hvb:c::u:G:D:T:Iz::"
#define OMBOP__PT2PT__LAT_MT                 "+:hvm:x:i:t:c::u:G:D:T:Iz::"
#define OMBOP__ACCEL__PT2PT__LAT_MT          OMBOP__ACCEL__PT2PT__LAT
#define OMBOP__PT2PT__LAT_MP                 "+:hvm:x:i:t:c::u:G:D:P:T:Iz::"
#define OMBOP__ACCEL__PT2PT__LAT_MP          OMBOP__ACCEL__PT2PT__LAT
#define OMBOP__COLLECTIVE__ALLTOALL          "+:hvfm:i:x:a:c::u:G:D:P:T:Ilz::"
#define OMBOP__ACCEL__COLLECTIVE__ALLTOALL   "+:d:hvfm:i:x:a:c::u:G:D:T:Ilz::"
#define OMBOP__PT2PT__CONG_BW                "+:hvm:x:i:W:b:G:D:P:T:Iz::"
#define OMBOP__ACCEL__PT2PT__CONG_BW         "p:W:R:x:i:m:d:Vhvb:G:D:T:Iz::"
#define OMBOP__COLLECTIVE__GATHER            OMBOP__COLLECTIVE__ALLTOALL "k:"
#define OMBOP__ACCEL__COLLECTIVE__GATHER     OMBOP__ACCEL__COLLECTIVE__ALLTOALL "k:"
#define OMBOP__COLLECTIVE__ALL_GATHER        OMBOP__COLLECTIVE__ALLTOALL
#define OMBOP__ACCEL__COLLECTIVE__ALL_GATHER OMBOP__ACCEL__COLLECTIVE__ALLTOALL
#define OMBOP__COLLECTIVE__SCATTER           OMBOP__COLLECTIVE__ALLTOALL "k:"
#define OMBOP__ACCEL__COLLECTIVE__SCATTER                                      \
    OMBOP__ACCEL__COLLECTIVE__ALLTOALL "k:"
#define OMBOP__COLLECTIVE__BCAST              "+:hvfm:i:x:a:c::u:G:D:P:T:Iz::"
#define OMBOP__ACCEL__COLLECTIVE__BCAST       "+:d:hvfm:i:x:a:c::u:G:D:T:Iz::"
#define OMBOP__COLLECTIVE__NHBR_GATHER        "+:hvfm:i:x:a:c::u:N:G:D:P:T:Iz::"
#define OMBOP__ACCEL__COLLECTIVE__NHBR_GATHER "+:d:hvfm:i:x:a:c::u:N:G:D:T:Iz::"
#define OMBOP__COLLECTIVE__NHBR_ALLTOALL      OMBOP__COLLECTIVE__NHBR_GATHER
#define OMBOP__ACCEL__COLLECTIVE__NHBR_ALLTOALL                                \
    OMBOP__ACCEL__COLLECTIVE__NHBR_GATHER
#define OMBOP__COLLECTIVE__BARRIER           "+:hvfm:i:x:a:c::u:G:P:Iz::"
#define OMBOP__ACCEL__COLLECTIVE__BARRIER    "+:d:hvfm:i:x:a:c::u:G:Iz::"
#define OMBOP__COLLECTIVE__LAT               "+:hvfm:i:x:a:z::"
#define OMBOP__ACCEL__COLLECTIVE__LAT        "+:d:hvfm:i:x:a:z::"
#define OMBOP__COLLECTIVE__ALL_REDUCE        "+:hvfm:i:x:a:c::u:G:P:T:Ilz::"
#define OMBOP__ACCEL__COLLECTIVE__ALL_REDUCE "+:d:hvfm:i:x:a:c::u:G:T:Ilz::"
#define OMBOP__COLLECTIVE__REDUCE            OMBOP__COLLECTIVE__ALL_REDUCE "k:"
#define OMBOP__ACCEL__COLLECTIVE__REDUCE                                       \
    OMBOP__ACCEL__COLLECTIVE__ALL_REDUCE "k:"
#define OMBOP__COLLECTIVE__REDUCE_SCATTER OMBOP__COLLECTIVE__ALL_REDUCE
#define OMBOP__ACCEL__COLLECTIVE__REDUCE_SCATTER                               \
    OMBOP__ACCEL__COLLECTIVE__ALL_REDUCE
#define OMBOP__COLLECTIVE__NBC_BARRIER        "+:hvfm:i:x:t:a:c::u:G:P:Iz::"
#define OMBOP__ACCEL__COLLECTIVE__NBC_BARRIER "+:d:hvfm:i:x:t:a:c::u:G:Iz::"
#define OMBOP__COLLECTIVE__NBC_ALLTOALL       "+:hvfm:i:x:t:a:c::u:G:D:P:T:Ilz::"
#define OMBOP__ACCEL__COLLECTIVE__NBC_ALLTOALL                                 \
    "+:d:hvfm:i:x:t:a:c::u:G:D:T:Ilz::"
#define OMBOP__COLLECTIVE__NBC_GATHER OMBOP__COLLECTIVE__NBC_ALLTOALL "k:"
#define OMBOP__ACCEL__COLLECTIVE__NBC_GATHER                                   \
    OMBOP__ACCEL__COLLECTIVE__NBC_ALLTOALL "k:"
#define OMBOP__COLLECTIVE__NBC_ALL_GATHER OMBOP__COLLECTIVE__NBC_ALLTOALL
#define OMBOP__ACCEL__COLLECTIVE__NBC_ALL_GATHER                               \
    OMBOP__ACCEL__COLLECTIVE__NBC_ALLTOALL
#define OMBOP__COLLECTIVE__NBC_SCATTER OMBOP__COLLECTIVE__NBC_ALLTOALL "k:"
#define OMBOP__ACCEL__COLLECTIVE__NBC_SCATTER                                  \
    OMBOP__ACCEL__COLLECTIVE__NBC_ALLTOALL "k:"
#define OMBOP__COLLECTIVE__NBC_BCAST        "+:hvfm:i:x:t:a:c::u:G:D:P:T:Iz::"
#define OMBOP__ACCEL__COLLECTIVE__NBC_BCAST "+:d:hvfm:i:x:t:a:c::u:G:D:T:Iz::"
#define OMBOP__COLLECTIVE__NBC_ALL_REDUCE   "+:hvfm:i:x:t:a:c::u:G:P:T:Ilz::"
#define OMBOP__ACCEL__COLLECTIVE__NBC_ALL_REDUCE                               \
    "+:d:hvfm:i:x:t:a:c::u:G:T:Ilz::"
#define OMBOP__COLLECTIVE__NBC_REDUCE OMBOP__COLLECTIVE__NBC_ALL_REDUCE "k:"
#define OMBOP__ACCEL__COLLECTIVE__NBC_REDUCE                                   \
    OMBOP__ACCEL__COLLECTIVE__NBC_ALL_REDUCE "k:"
#define OMBOP__COLLECTIVE__NBC_REDUCE_SCATTER OMBOP__COLLECTIVE__NBC_ALL_REDUCE
#define OMBOP__ACCEL__COLLECTIVE__NBC_REDUCE_SCATTER                           \
    OMBOP__ACCEL__COLLECTIVE__NBC_ALL_REDUCE
#define OMBOP__COLLECTIVE__NBC_NHBR_GATHER "+:hvfm:i:x:t:a:c::u:N:G:D:P:T:Iz::"
#define OMBOP__ACCEL__COLLECTIVE__NBC_NHBR_GATHER                              \
    "+:d:hvfm:i:x:t:a:c::u:N:G:D:T:Iz::"
#define OMBOP__COLLECTIVE__NBC_NHBR_ALLTOALL OMBOP__COLLECTIVE__NBC_NHBR_GATHER
#define OMBOP__ACCEL__COLLECTIVE__NBC_NHBR_ALLTOALL                            \
    OMBOP__ACCEL__COLLECTIVE__NBC_NHBR_GATHER
#define OMBOP__ONE_SIDED__BW         "+:w:s:hvm:x:i:W:G:P:I"
#define OMBOP__ACCEL__ONE_SIDED__BW  "+:w:s:hvm:d:x:i:W:G:I"
#define OMBOP__ONE_SIDED__LAT        "+:w:s:hvm:x:i:G:P:I"
#define OMBOP__ACCEL__ONE_SIDED__LAT "+:w:s:hvm:d:x:i:G:I"
#define OMBOP__MBW_MR                "p:W:R:x:i:m:Vhvb:c::u:G:D:P:T:Iz::"
#define OMBOP__ACCEL__MBW_MR         "p:W:R:x:i:m:d:Vhvb:c::u:G:D:T:Iz::"
#define OMBOP__OSHM                  ":hvfm:i:";
#define OMBOP__UPC                   OMBOP__OSHM
#define OMBOP__UPCXX                 OMBOP__OSHM
#define OMBOP__STARTUP__INIT         "I"
/*Persistent Collectives*/
#define OMBOP__COLLECTIVE__ALLTOALL_P        "+:hvfm:i:x:a:c::u:G:D:P:T:Ilz::"
#define OMBOP__ACCEL__COLLECTIVE__ALLTOALL_P "+:d:hvfm:i:x:a:c::u:G:D:T:Ilz::"
#define OMBOP__COLLECTIVE__GATHER_P          OMBOP__COLLECTIVE__ALLTOALL_P
#define OMBOP__ACCEL__COLLECTIVE__GATHER_P   OMBOP__ACCEL__COLLECTIVE__ALLTOALL_P
#define OMBOP__COLLECTIVE__ALL_GATHER_P      OMBOP__COLLECTIVE__ALLTOALL_P
#define OMBOP__ACCEL__COLLECTIVE__ALL_GATHER_P                                 \
    OMBOP__ACCEL__COLLECTIVE__ALLTOALL_P
#define OMBOP__COLLECTIVE__SCATTER_P           OMBOP__COLLECTIVE__ALLTOALL_P
#define OMBOP__ACCEL__COLLECTIVE__SCATTER_P    OMBOP__ACCEL__COLLECTIVE__ALLTOALL_P
#define OMBOP__COLLECTIVE__BCAST_P             "+:hvfm:i:x:a:c::u:G:D:P:T:Iz::"
#define OMBOP__ACCEL__COLLECTIVE__BCAST_P      "+:d:hvfm:i:x:a:c::u:G:D:T:Iz::"
#define OMBOP__COLLECTIVE__BARRIER_P           "+:hvfm:i:x:a:u:G:P:Iz::"
#define OMBOP__ACCEL__COLLECTIVE__BARRIER_P    "+:d:hvfm:i:x:a:u:G:Iz::"
#define OMBOP__COLLECTIVE__ALL_REDUCE_P        "+:hvfm:i:x:a:c::u:G:P:T:Ilz::"
#define OMBOP__ACCEL__COLLECTIVE__ALL_REDUCE_P "+:d:hvfm:i:x:a:c::u:G:T:Ilz::"
#define OMBOP__COLLECTIVE__REDUCE_P            OMBOP__COLLECTIVE__ALL_REDUCE_P
#define OMBOP__ACCEL__COLLECTIVE__REDUCE_P                                     \
    OMBOP__ACCEL__COLLECTIVE__ALL_REDUCE_P
#define OMBOP__COLLECTIVE__REDUCE_SCATTER_P OMBOP__COLLECTIVE__ALL_REDUCE_P
#define OMBOP__ACCEL__COLLECTIVE__REDUCE_SCATTER_P                             \
    OMBOP__ACCEL__COLLECTIVE__ALL_REDUCE_P

#define OMBOP_HELP_MSG                                                         \
    {{'h', "print this help"},                                                 \
     {'v', "print version info"},                                              \
     {'f', "print full format listing (MIN/MAX latency and ITERATIONS"         \
           "~~displayed in addition to AVERAGE latency)"},                     \
     {'m', "[MIN:]MAX[:INCR] - set the minimum and/or the maximum "            \
           "~~message size and increment to MIN and/or MAX and INCR "          \
           "~~bytes respectively. Examples:"                                   \
           "~~-m 128      // min = default, max = 128"                         \
           "~~-m 2:128    // min = 2, max = 128"                               \
           "~~-m 2:       // min = 2, max = default"                           \
           "~~-m 2^4:2^27 // min = 16, max = 128 mb"                           \
           "~~-m 2:2^20:4 // min = 2, max = 1mb, increment by x4"},            \
     {'W', "SIZE - set number of messages to send before "                     \
           "synchronization (default 64)"},                                    \
     {'t', "Non-blocking collectives-> CALLS - set the number of "             \
           "MPI_Test() calls during the dummy computation,"                    \
           "~~set CALLS to 100, 1000, or any number > 0."                      \
           "~~Multi-Threaded/Process-> SEND:[RECV] - set the sender and "      \
           "receiver number of threads/processes(t/p)"                         \
           "~~min: 1 default: (receiver t/p: 2 sender t/p: 1), max: 128."      \
           "~~Examples:"                                                       \
           "~~-t 4        // receiver t/p = 4 and sender t/p = 1"              \
           "~~-t 4:6      // sender t/p = 4 and receiver t/p = 6"              \
           "~~-t 2:       // not defined"},                                    \
     {'i', "ITER - number of iterations for timing (default 10000 for "        \
           "large messages and 1000 for small messages). Change the   "        \
           "~~ cutoff size and iterations with a colon separated list."        \
           "~~ Multiple ranges can be provided, separated by commas.  "        \
           "~~ Examples: "                                                     \
           "~~-i 100            // 100 iterations for ALL message sizes"       \
           "~~-i 8192:1000:100  // 1000 iterations for messages <= 8192"       \
           "~~ and 100 for messages > 8192                            "        \
           "~~-i 8192:100,1048576:10,1:2 //min<size<=8192 - 100,"              \
           "~~ 8192<size<=1048576 - 10, and size>1048576 - 2. The value"       \
           "~~ before the last iteration count is ignored."},                  \
     {'x', "ITER - set number of warmup"                                       \
           "~~iterations to skip before timing (default 200). This can"        \
           "~~take similar range size and arguments as the -i option"},        \
     {'a', "SIZE - set the size of arrays to be allocated on device (GPU)"     \
           "~~for dummy compute on device (GPU) (default 32). OMB must be "    \
           "configured with CUDA support."},                                   \
     {'s', "SYNC_OPTION - can be any of the follows:"                          \
           "~~pscw          use Post/Start/Complete/Wait synchronization "     \
           "calls"                                                             \
           "~~fence         use MPI_Win_fence synchronization call"            \
           "~~lock          use MPI_Win_lock/unlock synchronizations calls"    \
           "~~flush         use MPI_Win_flush synchronization call"            \
           "~~flush_local   use MPI_Win_flush_local synchronization call"      \
           "~~lock_all      use MPI_Win_lock_all/unlock_all "                  \
           "synchronization calls"},                                           \
     {'w', "WIN_OPTION - Supports MPI>=3. Can be any of the follows:"          \
           "~~create       use MPI_Win_create to create an MPI Window "        \
           "object"                                                            \
           "~~allocate     use MPI_Win_allocate to create an MPI "             \
           "Window object(not valid when using device memory)"                 \
           "~~dynamic      use MPI_Win_create_dynamic to create an "           \
           "MPI Window object"},                                               \
     {'d', "TYPE - use accelerator device buffers, which can be of "           \
           "TYPE 'cuda',"                                                      \
           "~~'managed', 'openacc', 'rocm' or 'sycl' (uses standard "          \
           "host buffers if not specified)"},                                  \
     {'r', "TARGET - set the compute target for dummy computation"             \
           "~~set TARGET to cpu (default) to execute"                          \
           "~~on CPU only, set to gpu for executing kernel"                    \
           "~~on the GPU only, and set to both for compute on both."           \
           "~~OMB must be configured with CUDA support."},                     \
     {'R', "[0|1] - Print uni-directional message rate (default 1)"},          \
     {'p', "PAIRS - Number of pairs involved (default np / 2)"},               \
     {'V', "Vary the window size (default no)"                                 \
           "~~[cannot be used with -W]"},                                      \
     {'c', "[log:<dir>]Enable validation. Disabled by default."                \
           "~~Results are logged into <dir> by passing \"log\""},              \
     {'b', "Use different buffers to perform data transfer (default single)"   \
           "~~Options: single, multiple"},                                     \
     {'u', "ITR Set number of warmup iterations to skip before timing "        \
           "when validation"                                                   \
           "is enabled (default 5)"},                                          \
     {'G', "[tty,png,pdf] - graph output of per iteration values."},           \
     {'P', "[EVENTS]:[PATH] - Enable PAPI support. OMB must be "               \
           "configured with CUDA support."                                     \
           "~~[EVENTS]       //Comma seperated list of PAPI events"            \
           "~~[PATH]         //PAPI output file path"},                        \
     {'D', "[TYPE]:[ARGS] - Enable DDT support"                                \
           "~~-D cont                          //Contiguous"                   \
           "~~-D vect:[stride]:[block_length]  //Vector"                       \
           "~~-D indx:[ddt file path]          //Index"},                      \
     {'N', "[TYPE]:[ARGS] - Configure neighborhood collectives. "              \
           "(default:- cart:1:1)"                                              \
           "~~-N cart:<num of dimentions:radius>   //Cartesian"                \
           "~~-N graph:<adjacency graph file>      //Graph"},                  \
     {'T', "[all,mpi_char,mpi_int,mpi_float] - Set MPI_TYPE . "                \
           "Default:MPI_CHAR. Reduction defaults: MPI_INT"},                   \
     {'I', "Enable session based MPI initialization."},                        \
     {'l', "Run benchmark with MPI_IN_PLACE support."},                        \
     {'z', "Print tail latencies."                                             \
           "~~-z Outputs P99, P90, P50 percentiles"                            \
           "~~-z<1-99,1-99,1-99..> Comma seperated percentile range"},         \
     {'q', "Number of MPI partitions."},                                       \
     {'k', "Set root rank. Default: fixed:0"                                   \
           "~~-k fixed:[RANK] //Fixed root rank."                              \
           "~~-k rotate       //Rotate root rank for each iteration."}}

#endif
