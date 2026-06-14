#define BENCHMARK "OSU UPC Broadcast Latency Test"
/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include <upc.h>
#include <upc_collective.h>
#include <../util/osu_util_pgas.h>

shared[] char *src;
shared char *dst;

shared double avg_time, max_time, min_time;
shared double latency[THREADS];

int main(int argc, char *argv[])
{
    int i = 0, size = 0, iterations, po_ret;
    int skip;
    double t_start = 0, t_stop = 0, timer = 0;
    int max_msg_size = 1 << 20, full = 0;

    options.bench = UPC;

    po_ret = process_options(argc, argv);

    max_msg_size = options.max_message_size;
    full = options.show_full;
    skip = options.skip;
    iterations = options.iterations;
    options.show_size = 1;

    switch (po_ret) {
        case PO_BAD_USAGE:
            print_usage_pgas(MYTHREAD, argv[0], size != 0);
            exit(EXIT_FAILURE);
        case PO_HELP_MESSAGE:
            print_usage_pgas(MYTHREAD, argv[0], size != 0);
            exit(EXIT_SUCCESS);
        case PO_VERSION_MESSAGE:
            if (MYTHREAD == 0) {
                print_version_pgas(HEADER);
            }
            exit(EXIT_SUCCESS);
        case PO_OKAY:
            break;
    }

    if (THREADS < 2) {
        if (MYTHREAD == 0) {
            fprintf(stderr, "This test requires at least two processes\n");
        }
        return -1;
    }

    print_header_pgas(HEADER, MYTHREAD, full);

    src = upc_all_alloc(1, max_msg_size * sizeof(char));
    dst = upc_all_alloc(THREADS, max_msg_size * sizeof(char));

    if (NULL == src || NULL == dst) {
        fprintf(stderr, "malloc failed.\n");
        exit(1);
    }

    for (size = 1; size <= max_msg_size; size *= 2) {
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
            upc_all_broadcast(dst, src, size, SYNC_MODE);
            t_stop = TIME();

            if (i >= skip) {
                timer += t_stop - t_start;
            }
            upc_barrier;
        }
        upc_barrier;
        latency[MYTHREAD] = (1.0 * timer) / iterations;

        upc_all_reduceD(&min_time, latency, UPC_MIN, THREADS, 1, NULL,
                        SYNC_MODE);
        upc_all_reduceD(&max_time, latency, UPC_MAX, THREADS, 1, NULL,
                        SYNC_MODE);
        upc_all_reduceD(&avg_time, latency, UPC_ADD, THREADS, 1, NULL,
                        SYNC_MODE);
        if (!MYTHREAD)
            avg_time = avg_time / THREADS;

        print_data_pgas(MYTHREAD, full, size * sizeof(char), avg_time, min_time,
                        max_time, iterations);
    }

    return EXIT_SUCCESS;
}

/* vi: set sw=4 sts=4 tw=80: */
