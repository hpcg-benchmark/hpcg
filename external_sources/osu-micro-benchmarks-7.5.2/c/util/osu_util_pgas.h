/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */
#include "osu_util.h"

#define UPC_LOOP_SMALL 1000
#define UPC_LOOP_LARGE 100
#define UPC_SKIP_SMALL 200
#define UPC_SKIP_LARGE 10

#define MYBUFSIZE_MR                                                           \
    (MAX_MESSAGE_SIZE * OSHM_LOOP_LARGE_MR + MESSAGE_ALIGNMENT_MR)
#define SYNC_MODE (UPC_IN_ALLSYNC | UPC_OUT_ALLSYNC)

void usage_oshm_pt2pt(int myid);
void print_header_pgas(const char *header, int rank, int full);
void print_data_pgas(int rank, int full, int size, double avg_time,
                     double min_time, double max_time, int iterations);
void print_usage_pgas(int rank, const char *prog, int has_size);
void print_version_pgas(const char *header);
