#define BENCHMARK "OSU OpenSHMEM Broadcast Latency Test"
/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include <shmem.h>
#include <osu_util_pgas.h>

long pSyncBcast1[_SHMEM_BCAST_SYNC_SIZE];
long pSyncBcast2[_SHMEM_BCAST_SYNC_SIZE];
long pSyncRed1[_SHMEM_REDUCE_SYNC_SIZE];
long pSyncRed2[_SHMEM_REDUCE_SYNC_SIZE];

double pWrk1[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];
double pWrk2[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];

int main(int argc, char *argv[])
{
    int i = 0, rank, size = 0;
    int skip, numprocs, iterations;
    static double avg_time = 0.0, max_time = 0.0, min_time = 0.0;
    static double latency = 0.0;
    double t_start = 0, t_stop = 0, timer = 0;
    char *buffer = NULL;
    int max_msg_size = 1048576, full = 0;
    int t;
    int po_ret;

    options.bench = OSHM;

    for (t = 0; t < _SHMEM_BCAST_SYNC_SIZE; t += 1)
        pSyncBcast1[t] = _SHMEM_SYNC_VALUE;
    for (t = 0; t < _SHMEM_BCAST_SYNC_SIZE; t += 1)
        pSyncBcast2[t] = _SHMEM_SYNC_VALUE;
    for (t = 0; t < _SHMEM_REDUCE_SYNC_SIZE; t += 1)
        pSyncRed1[t] = _SHMEM_SYNC_VALUE;
    for (t = 0; t < _SHMEM_REDUCE_SYNC_SIZE; t += 1)
        pSyncRed2[t] = _SHMEM_SYNC_VALUE;

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

    if (numprocs < 2) {
        if (rank == 0) {
            fprintf(stderr, "This test requires at least two processes\n");
        }
        return -1;
    }

    max_msg_size = options.max_message_size;
    full = options.show_full;
    print_header_pgas(HEADER, rank, full);

#ifdef OSHM_1_3
    buffer = (char *)shmem_malloc(max_msg_size * sizeof(char));
#else
    buffer = (char *)shmalloc(max_msg_size * sizeof(char));
#endif

    if (NULL == buffer) {
        fprintf(stderr, "malloc failed.\n");
        exit(1);
    }

    memset(buffer, 1, max_msg_size);

    for (size = 1; size <= max_msg_size / sizeof(uint32_t); size *= 2) {
        if (size > LARGE_MESSAGE_SIZE) {
            skip = options.skip_large;
            iterations = options.iterations_large;
        } else {
            skip = options.skip;
            iterations = options.iterations;
        }

        timer = 0;
        for (i = 0; i < iterations + skip; i++) {
            t_start = TIME();
            if (i % 2)
                shmem_broadcast32(buffer, buffer, size, 0, 0, 0, numprocs,
                                  pSyncBcast1);
            else
                shmem_broadcast32(buffer, buffer, size, 0, 0, 0, numprocs,
                                  pSyncBcast2);
            t_stop = TIME();

            if (i >= skip) {
                timer += t_stop - t_start;
            }
            shmem_barrier_all();
        }
        shmem_barrier_all();
        latency = (1.0 * timer) / iterations;

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

#ifdef OSHM_1_3
    shmem_free(buffer);
    shmem_finalize();
#else
    shfree(buffer);
#endif

    return EXIT_SUCCESS;
}

/* vi: set sw=4 sts=4 tw=80: */
