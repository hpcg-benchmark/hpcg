/*
 * Copyright (C) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */
#define CHILD_RANK_TAG  101
#define PARENT_RANK_TAG 102

typedef struct node_info_t {
    char *proc_name;
    int proc_name_len;
    int rank;
    int node_id;
    int local_rank;
} node_info_t;

typedef struct fan_in_out_info_t {
    int total_nodes;
    int ppn;
    int is_parent;
    int *ranks_queue;
    node_info_t *cur_node_info;
} fan_in_out_info_t;

fan_in_out_info_t omb_fan_init(MPI_Comm comm);
void omb_fan_free(fan_in_out_info_t fan_in_out_info);
