#define BENCHMARK "OSU OpenSHMEM Collect Latency Test"
/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 */

/*
This program is available under BSD licensing.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

(1) Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

(2) Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

(3) Neither the name of The Ohio State University nor the names of
their contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <shmem.h>
#include <osu_util_pgas.h>

long pSyncCollect1[_SHMEM_COLLECT_SYNC_SIZE];
long pSyncCollect2[_SHMEM_COLLECT_SYNC_SIZE];
long pSyncRed1[_SHMEM_REDUCE_SYNC_SIZE];
long pSyncRed2[_SHMEM_REDUCE_SYNC_SIZE];

double pWrk1[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];
double pWrk2[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];

int main(int argc, char *argv[])
{
    int i, numprocs, rank, iterations, po_ret;
    unsigned long align_size = sysconf(_SC_PAGESIZE);
    int skip;
    int size = 0;
    static double latency = 0.0;
    double t_start = 0, t_stop = 0, timer = 0;
    static double avg_time = 0.0, max_time = 0.0, min_time = 0.0;
    char *recvbuff, *sendbuff;
    int max_msg_size = 1048576, full = 0, t;

    options.bench = OSHM;

    for (t = 0; t < _SHMEM_REDUCE_SYNC_SIZE; t += 1)
        pSyncRed1[t] = _SHMEM_SYNC_VALUE;
    for (t = 0; t < _SHMEM_REDUCE_SYNC_SIZE; t += 1)
        pSyncRed2[t] = _SHMEM_SYNC_VALUE;
    for (t = 0; t < _SHMEM_COLLECT_SYNC_SIZE; t += 1)
        pSyncCollect1[t] = _SHMEM_SYNC_VALUE;
    for (t = 0; t < _SHMEM_COLLECT_SYNC_SIZE; t += 1)
        pSyncCollect2[t] = _SHMEM_SYNC_VALUE;

#ifdef OSHM_1_3
    shmem_init();
    rank = shmem_my_pe();
    numprocs = shmem_n_pes();
#else
    start_pes(0);
    rank = _my_pe();
    numprocs = _num_pes();
#endif

    po_ret = process_options(argc, argv);
    full = options.show_full;
    max_msg_size = options.max_message_size;

    switch (po_ret) {
        case PO_BAD_USAGE:
            print_usage_pgas(rank, argv[0], size != 0);
            exit(EXIT_FAILURE);
        case PO_HELP_MESSAGE:
            print_usage_pgas(rank, argv[0], size != 0);
            exit(EXIT_SUCCESS);
        case PO_VERSION_MESSAGE:
            if (rank == 0) {
                print_version_pgas(HEADER);
            }
            exit(EXIT_SUCCESS);
        case PO_OKAY:
            break;
    }

    /*
    if (process_args(argc, argv, rank, &max_msg_size, &full, HEADER)) {
        return 0;
    }*/

    if (numprocs < 2) {
        if (rank == 0) {
            fprintf(stderr, "This test requires at least two processes\n");
        }
        return -1;
    }

    print_header_pgas(HEADER, rank, full);

#ifdef OSHM_1_3
    recvbuff =
        (char *)shmem_align(align_size, sizeof(char) * max_msg_size * numprocs);
#else
    recvbuff =
        (char *)shmemalign(align_size, sizeof(char) * max_msg_size * numprocs);
#endif

    if (NULL == recvbuff) {
        fprintf(stderr, "shmemalign failed.\n");
        exit(1);
    }

#ifdef OSHM_1_3
    sendbuff = (char *)shmem_align(align_size, sizeof(char) * max_msg_size);
#else
    sendbuff = (char *)shmemalign(align_size, sizeof(char) * max_msg_size);
#endif

    if (NULL == sendbuff) {
        fprintf(stderr, "shmemalign failed.\n");
        exit(1);
    }

    memset(recvbuff, 1, max_msg_size * numprocs);
    memset(sendbuff, 0, max_msg_size);

    for (size = 1; size <= max_msg_size / sizeof(uint32_t); size *= 2) {
        if (size > LARGE_MESSAGE_SIZE) {
            skip = options.skip_large;
            iterations = options.iterations_large;
        } else {
            skip = options.skip;
            iterations = options.iterations;
        }

        shmem_barrier_all();

        timer = 0;
        for (i = 0; i < iterations + skip; i++) {
            t_start = TIME();
            if (i % 2)
                shmem_collect32(recvbuff, sendbuff, size, 0, 0, numprocs,
                                pSyncCollect1);
            else
                shmem_collect32(recvbuff, sendbuff, size, 0, 0, numprocs,
                                pSyncCollect2);
            t_stop = TIME();

            if (i >= skip) {
                timer += t_stop - t_start;
            }
            shmem_barrier_all();
        }

        shmem_barrier_all();
        latency = (double)(timer * 1.0) / iterations;

        shmem_double_min_to_all(&min_time, &latency, 1, 0, 0, numprocs, pWrk1,
                                pSyncRed1);
        shmem_double_max_to_all(&max_time, &latency, 1, 0, 0, numprocs, pWrk2,
                                pSyncRed2);
        shmem_double_sum_to_all(&avg_time, &latency, 1, 0, 0, numprocs, pWrk1,
                                pSyncRed1);
        avg_time = avg_time / numprocs;

        print_data_pgas(rank, full, size * sizeof(uint32_t), avg_time, min_time,
                        max_time, iterations);
    }

    shmem_barrier_all();

#ifdef OSHM_1_3
    shmem_free(recvbuff);
    shmem_free(sendbuff);
    shmem_finalize();
#else
    shfree(recvbuff);
    shfree(sendbuff);
#endif

    return EXIT_SUCCESS;
}

/* vi: set sw=4 sts=4 tw=80: */
