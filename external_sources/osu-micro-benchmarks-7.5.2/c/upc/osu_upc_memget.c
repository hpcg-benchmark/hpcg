#define BENCHMARK "OSU UPC MEMGET Test"
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
#include <../util/osu_util_pgas.h>

int skip = 1000;
int loop = 10000;

int main(int argc, char **argv)
{
    int iters = 0;
    double t_start, t_end;
    int peerid = (MYTHREAD + THREADS / 2) % THREADS;
    int iamsender = 0;
    int i;

    if (THREADS == 1) {
        if (MYTHREAD == 0) {
            fprintf(stderr, "This test requires at least two UPC threads\n");
        }
        return 0;
    }

    if (MYTHREAD < THREADS / 2)
        iamsender = 1;

    shared char *data = upc_all_alloc(THREADS, MAX_MESSAGE_SIZE * 2);
    shared[] char *remote = (shared[] char *)(data + peerid);
    char *local = ((char *)(data + MYTHREAD)) + MAX_MESSAGE_SIZE;

    if (!MYTHREAD) {
        fprintf(stdout, HEADER);
        fprintf(stdout, "# [ pairs: %d ]\n", THREADS / 2);
        fprintf(stdout, "%-*s%*s\n", 10, "# Size", FIELD_WIDTH, "Latency (us)");
        fflush(stdout);
    }

    for (int size = 1; size <= MAX_MESSAGE_SIZE; size *= 2) {
        if (iamsender)
            for (i = 0; i < size; i++) {
                local[i] = 'a';
            }
        else
            for (i = 0; i < size; i++) {
                local[i] = 'b';
            }

        upc_barrier;

        if (size > LARGE_MESSAGE_SIZE) {
            loop = UPC_LOOP_LARGE;
            skip = UPC_SKIP_LARGE;
        }

        if (iamsender) {
            for (i = 0; i < loop + skip; i++) {
                if (i == skip) {
                    upc_barrier;
                    wtime(&t_start);
                }

                upc_memget(local, remote, size);
            }

            upc_barrier;

            wtime(&t_end);
            if (!MYTHREAD) {
                double latency = (t_end - t_start) / (1.0 * loop);
                fprintf(stdout, "%-*d%*.*f\n", 10, size, FIELD_WIDTH,
                        FLOAT_PRECISION, latency);
                fflush(stdout);
            }
        } else {
            upc_barrier;
            upc_barrier;
        }
    }
    return 0;
}
