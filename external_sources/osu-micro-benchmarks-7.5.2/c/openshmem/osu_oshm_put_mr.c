#define BENCHMARK "OSU OpenSHMEM Put Message Rate Test"
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

char global_msg_buffer[MYBUFSIZE_MR];

#ifndef MEMORY_SELECTION
#define MEMORY_SELECTION 1
#endif

struct pe_vars {
    int me;
    int npes;
    int pairs;
    int nxtpe;
};

struct pe_vars init_openshmem(void)
{
    struct pe_vars v;

#ifdef OSHM_1_3
    shmem_init();
    v.me = shmem_my_pe();
    v.npes = shmem_n_pes();
#else
    start_pes(0);
    v.me = _my_pe();
    v.npes = _num_pes();
#endif

    v.pairs = v.npes / 2;
    v.nxtpe = v.me < v.pairs ? v.me + v.pairs : v.me - v.pairs;

    return v;
}

void check_usage(int me, int npes, int argc, char *argv[])
{
    if (MEMORY_SELECTION) {
        if (2 == argc) {
            /*
             * Compare more than 4 and 6 characters respectively to make sure
             * that we're not simply matching a prefix but the entire string.
             */
            if (strncmp(argv[1], "heap", 10) &&
                strncmp(argv[1], "global", 10)) {
                usage_oshm_pt2pt(me);
                exit(EXIT_FAILURE);
            }
        }

        else {
            usage_oshm_pt2pt(me);
            exit(EXIT_FAILURE);
        }
    }

    if (2 > npes) {
        if (0 == me) {
            fprintf(stderr, "This test requires at least two processes\n");
        }

        exit(EXIT_FAILURE);
    }
}

void print_header_local(int myid)
{
    if (myid == 0) {
        fprintf(stdout, HEADER);
        fprintf(stdout, "%-*s%*s\n", 10, "# Size", FIELD_WIDTH, "Messages/s");
        fflush(stdout);
    }
}

char *allocate_memory(int me, long align_size, int use_heap)
{
    char *msg_buffer;

    if (!use_heap) {
        return global_msg_buffer;
    }

#ifdef OSHM_1_3
    msg_buffer = (char *)shmem_malloc(MAX_MESSAGE_SIZE * OSHM_LOOP_LARGE_MR +
                                      align_size);
#else
    msg_buffer =
        (char *)shmalloc(MAX_MESSAGE_SIZE * OSHM_LOOP_LARGE_MR + align_size);
#endif

    if (NULL == msg_buffer) {
        fprintf(stderr, "Failed to shmalloc (pe: %d)\n", me);
        exit(EXIT_FAILURE);
    }

    return msg_buffer;
}

char *align_memory(unsigned long address, int const align_size)
{
    return (char *)((address + (align_size - 1)) / align_size * align_size);
}

double message_rate(struct pe_vars v, char *buffer, int size, int iterations)
{
    double begin, end;
    int i, offset;

    /*
     * Touch memory
     */
    memset(buffer, size, MAX_MESSAGE_SIZE * OSHM_LOOP_LARGE_MR);

    shmem_barrier_all();

    if (v.me < v.pairs) {
        begin = TIME();

        for (i = 0, offset = 0; i < iterations; i++, offset += size) {
            shmem_putmem(&buffer[offset], &buffer[offset], size, v.nxtpe);
        }

        shmem_quiet();
        end = TIME();

        return ((double)iterations * 1e6) / ((double)end - (double)begin);
    }

    return 0;
}

void print_message_rate(int myid, int size, double rate)
{
    if (myid == 0) {
        fprintf(stdout, "%-*d%*.*f\n", 10, size, FIELD_WIDTH, FLOAT_PRECISION,
                rate);
        fflush(stdout);
    }
}

void benchmark(struct pe_vars v, char *msg_buffer)
{
    static double pwrk[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];
    static long psync[_SHMEM_REDUCE_SYNC_SIZE];
    static double mr, mr_sum;
    unsigned long size, i;

    memset(psync, _SHMEM_SYNC_VALUE, sizeof(long[_SHMEM_REDUCE_SYNC_SIZE]));

    /*
     * Warmup
     */
    if (v.me < v.pairs) {
        for (i = 0; i < (OSHM_LOOP_LARGE_MR * MAX_MESSAGE_SIZE);
             i += MAX_MESSAGE_SIZE) {
            shmem_putmem(&msg_buffer[i], &msg_buffer[i], MAX_MESSAGE_SIZE,
                         v.nxtpe);
        }
    }

    shmem_barrier_all();

    /*
     * Benchmark
     */
    for (size = 1; size <= MAX_MESSAGE_SIZE; size <<= 1) {
        i = size < LARGE_MESSAGE_SIZE ? OSHM_LOOP_SMALL_MR : OSHM_LOOP_LARGE_MR;

        mr = message_rate(v, msg_buffer, size, i);
        shmem_double_sum_to_all(&mr_sum, &mr, 1, 0, 0, v.npes, pwrk, psync);
        print_message_rate(v.me, size, mr_sum);
    }
}

int main(int argc, char *argv[])
{
    struct pe_vars v;
    char *msg_buffer, *aligned_buffer;
    long alignment;
    int use_heap;

    /*
     * Initialize
     */
    v = init_openshmem();
    check_usage(v.me, v.npes, argc, argv);
    print_header_local(v.me);

    /*
     * Allocate Memory
     */
    use_heap = !strncmp(argv[1], "heap", 10);
    alignment = use_heap ? sysconf(_SC_PAGESIZE) : MESSAGE_ALIGNMENT_MR;
    msg_buffer = allocate_memory(v.me, alignment, use_heap);
    aligned_buffer = align_memory((unsigned long)msg_buffer, alignment);
    memset(aligned_buffer, 0, MAX_MESSAGE_SIZE * OSHM_LOOP_LARGE_MR);

    /*
     * Time Put Message Rate
     */
    benchmark(v, aligned_buffer);

    /*
     * Finalize
     */
    if (use_heap) {
#ifdef OSHM_1_3
        shmem_free(msg_buffer);
#else
        shfree(msg_buffer);
#endif
    }

#ifdef OSHM_1_3
    shmem_finalize();
#endif

    return EXIT_SUCCESS;
}
