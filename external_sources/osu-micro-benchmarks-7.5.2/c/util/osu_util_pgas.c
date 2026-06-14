/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level directory.
 */

#include "osu_util_pgas.h"

void usage_oshm_pt2pt(int myid)
{
    if (myid == 0) {
        fprintf(stderr,
                "Invalid arguments. Usage: <prog_name> <heap|global>\n");
    }
}

void print_header_pgas(const char *header, int rank, int full)
{
    if (rank == 0) {
        fprintf(stdout, header, "");

        if (options.show_size) {
            fprintf(stdout, "%-*s", 10, "# Size");
            fprintf(stdout, "%*s", FIELD_WIDTH, "Avg Latency(us)");
        }

        else {
            fprintf(stdout, "# Avg Latency(us)");
        }

        if (full) {
            fprintf(stdout, "%*s", FIELD_WIDTH, "Min Latency(us)");
            fprintf(stdout, "%*s", FIELD_WIDTH, "Max Latency(us)");
            fprintf(stdout, "%*s\n", 12, "Iterations");
        }

        else {
            fprintf(stdout, "\n");
        }

        fflush(stdout);
    }
}

void print_data_pgas(int rank, int full, int size, double avg_time,
                     double min_time, double max_time, int iterations)
{
    if (rank == 0) {
        if (size) {
            fprintf(stdout, "%-*d", 10, size);
            fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION, avg_time);
        }

        else {
            fprintf(stdout, "%*.*f", 17, FLOAT_PRECISION, avg_time);
        }

        if (full) {
            fprintf(stdout, "%*.*f%*.*f%*d\n", FIELD_WIDTH, FLOAT_PRECISION,
                    min_time, FIELD_WIDTH, FLOAT_PRECISION, max_time, 12,
                    iterations);
        }

        else {
            fprintf(stdout, "\n");
        }

        fflush(stdout);
    }
}

void print_usage_pgas(int rank, const char *prog, int has_size)
{
    if (rank == 0) {
        if (has_size) {
            fprintf(stdout,
                    " USAGE : %s [-m SIZE] [-i ITER] [-f] [-hv] [-M SIZE]\n",
                    prog);
            fprintf(
                stdout,
                "  -m, --message-size : Set maximum message size to SIZE.\n");
            fprintf(stdout, "                       By default, the value of "
                            "SIZE is 1MB.\n");
            fprintf(stdout, "  -i, --iterations   : Set number of iterations "
                            "per message size to ITER.\n");
            fprintf(stdout, "                       By default, the value of "
                            "ITER is 1000 for small messages\n");
            fprintf(stdout,
                    "                       and 100 for large messages.\n");
            fprintf(stdout, "  -M, --mem-limit    : Set maximum memory "
                            "consumption (per process) to SIZE. \n");
            fprintf(stdout, "                       By default, the value of "
                            "SIZE is 512MB.\n");
        }

        else {
            fprintf(stdout, " USAGE : %s [-i ITER] [-f] [-hv] \n", prog);
            fprintf(
                stdout,
                "  -i, --iterations   : Set number of iterations to ITER.\n");
            fprintf(stdout, "                       By default, the value of "
                            "ITER is 1000.\n");
        }

        fprintf(stdout, "  -f, --full         : Print full format listing.  "
                        "With this option\n");
        fprintf(stdout, "                      the MIN/MAX latency and number "
                        "of ITERATIONS are\n");
        fprintf(stdout, "                      printed out in addition to the "
                        "AVERAGE latency.\n");

        fprintf(stdout, "  -h, --help         : Print this help.\n");
        fprintf(stdout, "  -v, --version      : Print version info.\n");
        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

void print_version_pgas(const char *header)
{
    fprintf(stdout, header, "");
    fflush(stdout);
}

int process_one_sided_options(int opt, char *arg) { return PO_BAD_USAGE; }
