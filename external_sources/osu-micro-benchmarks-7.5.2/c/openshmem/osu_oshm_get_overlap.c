#define BENCHMARK "OSU OpenSHMEM Put_nbi Test"
/*
 * Copyright (c) 2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include <shmem.h>
#include <osu_util_pgas.h>

#define max(a, b) (a > b ? a : b)

#define MAX_MSG_SIZE (1 << 20)

char s_buf_original[MYBUFSIZE];
char r_buf_original[MYBUFSIZE];

int skip = 1000;
int loop = 10000;
int skip_large = 10;
int loop_large = 100;
int large_message_size = 8192;

#ifdef PACKAGE_VERSION
#define HEADER "# " BENCHMARK " v" PACKAGE_VERSION "\n"
#else
#define HEADER "# " BENCHMARK "\n"
#endif

#ifndef FIELD_WIDTH
#define FIELD_WIDTH 20
#endif

#ifndef FLOAT_PRECISION
#define FLOAT_PRECISION 2
#endif

double test_time = 0.0, test_total = 0.0;
double latency = 0.0, t_start = 0.0, t_stop = 0.0;
double tcomp = 0.0, tcomp_total = 0.0, latency_in_secs = 0.0;
double timer = 0.0;
double wait_time = 0.0, init_time = 0.0;
double init_total = 0.0, wait_total = 0.0;

void compute_on_host_()
{
    int x = 0;
    int i = 0, j = 0;
    for (i = 0; i < 25; i++)
        for (j = 0; j < 25; j++)
            x = x + i * j;
}

static inline void do_compute_cpu_(double target_seconds)
{
    double t1 = 0.0, t2 = 0.0;
    double time_elapsed = 0.0;
    while (time_elapsed < target_seconds) {
        t1 = TIME();
        compute_on_host_();
        t2 = TIME();
        time_elapsed += (t2 - t1);
    }
}

int main(int argc, char *argv[])
{
    int myid, numprocs, i;
    size_t size;
    char *s_buf, *r_buf;
    char *s_buf_heap, *r_buf_heap;
    int align_size;
    double t_start = 0, t_end = 0;
    int use_heap = 0; // default uses global

#ifdef OSHM_1_3
    shmem_init();
    myid = shmem_my_pe();
    numprocs = shmem_n_pes();
#else
    start_pes(0);
    myid = _my_pe();
    numprocs = _num_pes();
#endif

    if (numprocs != 2) {
        if (myid == 0) {
            fprintf(stderr, "This test requires exactly two processes\n");
        }

        return EXIT_FAILURE;
    }

    if (argc != 2) {
        usage_oshm_pt2pt(myid);

        return EXIT_FAILURE;
    }

    if (0 == strncmp(argv[1], "heap", strlen("heap"))) {
        use_heap = 1;
    } else if (0 == strncmp(argv[1], "global", strlen("global"))) {
        use_heap = 0;
    } else {
        usage_oshm_pt2pt(myid);
        return EXIT_FAILURE;
    }

    align_size = MESSAGE_ALIGNMENT;

    /**************Allocating Memory*********************/

    if (use_heap) {
#ifdef OSHM_1_3
        s_buf_heap = (char *)shmem_malloc(MYBUFSIZE);
        r_buf_heap = (char *)shmem_malloc(MYBUFSIZE);
#else
        s_buf_heap = (char *)shmalloc(MYBUFSIZE);
        r_buf_heap = (char *)shmalloc(MYBUFSIZE);
#endif

        s_buf = (char *)(((unsigned long)s_buf_heap + (align_size - 1)) /
                         align_size * align_size);

        r_buf = (char *)(((unsigned long)r_buf_heap + (align_size - 1)) /
                         align_size * align_size);
    } else {
        s_buf = (char *)(((unsigned long)s_buf_original + (align_size - 1)) /
                         align_size * align_size);

        r_buf = (char *)(((unsigned long)r_buf_original + (align_size - 1)) /
                         align_size * align_size);
    }

    /**************Memory Allocation Done*********************/

    if (myid == 0) {
        fprintf(stdout, HEADER);
        fprintf(stdout,
                "# Overall = Coll. Init + Compute + MPI_Test + MPI_Wait\n\n");
        fprintf(stdout, "%-*s", 10, "# Size");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Compute(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Coll. Init(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "MPI_Wait(us)");
        fprintf(stdout, "%*s", FIELD_WIDTH, "Pure Comm.(us)");
        fprintf(stdout, "%*s\n", FIELD_WIDTH, "Overlap(%)");
        fflush(stdout);
    }

    for (size = 1; size <= MAX_MSG_SIZE; size = (size ? size * 2 : 1)) {
        /* touch the data */
        for (i = 0; i < size; i++) {
            s_buf[i] = 'a';
            r_buf[i] = 'b';
        }

        if (size > large_message_size) {
            loop = loop_large = 100;
            skip = skip_large = 0;
        }

        shmem_barrier_all();

        timer = 0.0;
        if (myid == 0) {
            for (i = 0; i < loop + skip; i++) {
                t_start = TIME();
                shmem_getmem_nbi(r_buf, s_buf, size, 1);
                shmem_quiet();
                t_stop = TIME();

                if (i >= skip) {
                    timer += t_stop - t_start;
                }

                shmem_fence();
            }

            latency = (timer * 1e6) / loop;

            /* Comm. latency in seconds, fed to dummy_compute */
            latency_in_secs = timer / loop;

            timer = 0.0;
            tcomp_total = 0;
            tcomp = 0;
            init_total = 0.0;
            wait_total = 0.0;
            test_time = 0.0;

            for (i = 0; i < loop + skip; i++) {
                t_start = TIME();
                init_time = TIME();
                shmem_getmem_nbi(r_buf, s_buf, size, 1);
                init_time = TIME() - init_time;

                tcomp = TIME();
                do_compute_cpu_(latency_in_secs);
                tcomp = TIME() - tcomp;

                wait_time = TIME();
                shmem_quiet();
                wait_time = TIME() - wait_time;

                t_stop = TIME();

                if (i >= skip) {
                    timer += t_stop - t_start;
                    tcomp_total += tcomp;
                    init_total += init_time;
                    wait_total += wait_time;
                }

                shmem_fence();
            }
        }

        shmem_barrier_all();

        if (myid == 0) {
            double overlap =
                max(0, 100 - ((((timer / loop) - (tcomp_total / loop)) /
                               latency_in_secs) *
                              100));

            int LOCAL_WIDTH = 17;
            fprintf(stdout, "%-*d", 10, size);
            fprintf(stdout, "%*.*f%*.*f%*.*f%*.*f%*.*f\n", LOCAL_WIDTH,
                    FLOAT_PRECISION, tcomp_total / loop, LOCAL_WIDTH,
                    FLOAT_PRECISION, init_time / loop, LOCAL_WIDTH,
                    FLOAT_PRECISION, wait_time / loop, LOCAL_WIDTH,
                    FLOAT_PRECISION, latency_in_secs, LOCAL_WIDTH,
                    FLOAT_PRECISION, overlap);

            fflush(stdout);
        }
    }

    shmem_barrier_all();

    if (use_heap) {
#ifdef OSHM_1_3
        shmem_free(s_buf_heap);
        shmem_free(r_buf_heap);
#else
        shfree(s_buf_heap);
        shfree(r_buf_heap);
#endif
    }

    shmem_barrier_all();

#ifdef OSHM_1_3
    shmem_finalize();
#endif
    return EXIT_SUCCESS;
}

/* vi: set sw=4 sts=4 tw=80: */
