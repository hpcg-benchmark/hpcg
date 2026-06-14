/*
 * Copyright (C) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */
#include "osu_util_mpi.h"
#include "osu_bw_fan_util.h"

int comparator(const void *a, const void *b)
{
    node_info_t *a_node_info = *(node_info_t **)a;
    node_info_t *b_node_info = *(node_info_t **)b;

    return strcmp(a_node_info->proc_name, b_node_info->proc_name);
}

fan_in_out_info_t omb_fan_init(MPI_Comm comm)
{
    int rank = 0;
    int comm_size = 0;
    int comm_itr = 0;
    char *all_proc_names = NULL;
    int *all_ranks = NULL;
    node_info_t **all_node_info = NULL;
    int num_nodes = 1;
    int local_rank_counter = 1;
    int max_ppn = 0;
    fan_in_out_info_t fan_in_out_info;
    int *is_parent_arr = NULL;
    int *queue = NULL;
    int max_ppn_itr = 0, num_nodes_itr = 0;
    int parent_rank = 0;
    int child_rank = 0;
    MPI_Request *send_reqstat = NULL;
    MPI_Request *recv_reqstat = NULL;
    int reqstat_itr = 0;
    int *local_rank_info_arr = NULL;

    MPI_CHECK(MPI_Comm_rank(comm, &rank));
    MPI_CHECK(MPI_Comm_size(comm, &comm_size));
    node_info_t *cur_node_info;
    /*Allocating node_info_t structure to store current process's information*/
    cur_node_info = (node_info_t *)malloc(sizeof(node_info_t));
    OMB_CHECK_NULL_AND_EXIT(cur_node_info, "Error allocating memory");
    /* Storing processor name for current rank*/
    cur_node_info->proc_name =
        (char *)malloc(sizeof(char) * MPI_MAX_PROCESSOR_NAME);
    OMB_CHECK_NULL_AND_EXIT(cur_node_info->proc_name,
                            "Error allocating memory");
    cur_node_info->rank = rank;
    MPI_CHECK(MPI_Get_processor_name(cur_node_info->proc_name,
                                     &cur_node_info->proc_name_len));
    /*Get processor names for all ranks*/
    all_proc_names =
        (char *)malloc(sizeof(char) * MPI_MAX_PROCESSOR_NAME * comm_size);
    OMB_CHECK_NULL_AND_EXIT(all_proc_names, "Error allocating memory");
    MPI_CHECK(MPI_Gather(cur_node_info->proc_name, MPI_MAX_PROCESSOR_NAME,
                         MPI_CHAR, all_proc_names, MPI_MAX_PROCESSOR_NAME,
                         MPI_CHAR, 0, comm));
    all_node_info = (node_info_t **)malloc(sizeof(node_info_t *) * comm_size);
    for (comm_itr = 0; comm_itr < comm_size; comm_itr++) {
        all_node_info[comm_itr] = (node_info_t *)malloc(sizeof(node_info_t));
        OMB_CHECK_NULL_AND_EXIT(all_node_info[comm_itr],
                                "Error allocating memory");
    }
    if (0 == rank) {
        for (comm_itr = 0; comm_itr < comm_size; comm_itr++) {
            all_node_info[comm_itr]->proc_name =
                (char *)malloc(sizeof(char) * MPI_MAX_PROCESSOR_NAME);
            OMB_CHECK_NULL_AND_EXIT(all_node_info[comm_itr]->proc_name,
                                    "Error allocating memory");

            all_node_info[comm_itr]->rank = comm_itr;
            strcpy(all_node_info[comm_itr]->proc_name,
                   all_proc_names + MPI_MAX_PROCESSOR_NAME * comm_itr);
        }
        qsort(all_node_info, comm_size, sizeof(node_info_t *), comparator);
        all_node_info[0]->node_id = num_nodes;
        all_node_info[0]->local_rank = 0;
        /*Calculate total number of nodes and ppn*/
        for (comm_itr = 1; comm_itr < comm_size; comm_itr++) {
            if (0 != strcmp(all_node_info[comm_itr]->proc_name,
                            all_node_info[comm_itr - 1]->proc_name)) {
                if (1 == num_nodes) {
                    max_ppn = local_rank_counter;
                } else if (max_ppn != local_rank_counter) {
                    OMB_ERROR_EXIT("Please run this benchmark with same ppn "
                                   "for all nodes");
                }
                num_nodes += 1;
                local_rank_counter = 0;
            }
            all_node_info[comm_itr]->node_id = num_nodes;
            all_node_info[comm_itr]->local_rank = local_rank_counter;
            local_rank_counter += 1;
        }
        if (1 == num_nodes) {
            OMB_ERROR_EXIT("Please run this benchmark on more than 1 node");
        }
        if (max_ppn != local_rank_counter) {
            OMB_ERROR_EXIT(
                "Please run this benchmark with same ppn for all nodes");
        }
    }

    local_rank_info_arr = malloc(sizeof(int) * comm_size);
    OMB_CHECK_NULL_AND_EXIT(local_rank_info_arr, "Unable to allocate memory");
    if (0 == rank) {
        for (comm_itr = 0; comm_itr < comm_size; comm_itr += 1) {
            local_rank_info_arr[comm_itr] = all_node_info[comm_itr]->local_rank;
        }
    }
    MPI_CHECK(MPI_Scatter(local_rank_info_arr, 1, MPI_INT,
                          &(cur_node_info->local_rank), 1, MPI_INT, 0, comm));

    /*Communicate parent node and child nodes*/
    is_parent_arr = malloc(sizeof(int) * comm_size);
    OMB_CHECK_NULL_AND_EXIT(is_parent_arr, "Unable to allocate memory");
    if (0 == rank) {
        for (comm_itr = 0; comm_itr < comm_size; comm_itr += 1) {
            is_parent_arr[comm_itr] = all_node_info[comm_itr]->node_id;
        }
    }
    MPI_CHECK(MPI_Scatter(is_parent_arr, 1, MPI_INT, &(cur_node_info->node_id),
                          1, MPI_INT, 0, comm));
    if (1 == cur_node_info->node_id) {
        fan_in_out_info.is_parent = 1;
    } else {
        fan_in_out_info.is_parent = 0;
    }

    /*Communicate parent and child ranks with each other*/
    MPI_CHECK(MPI_Bcast(&num_nodes, 1, MPI_INT, 0, comm));
    queue = malloc(sizeof(int) * num_nodes);
    OMB_CHECK_NULL_AND_EXIT(queue, "Unable to allocate memory.");
    send_reqstat = malloc(sizeof(MPI_Request) * (num_nodes - 1) * max_ppn * 2);
    OMB_CHECK_NULL_AND_EXIT(send_reqstat, "Unable to allocate memory.");
    if (0 == rank) {
        for (max_ppn_itr = 0; max_ppn_itr < max_ppn; max_ppn_itr++) {
            parent_rank = (all_node_info[max_ppn_itr])->rank;
            for (num_nodes_itr = 1; num_nodes_itr < num_nodes;
                 num_nodes_itr++) {
                child_rank =
                    (all_node_info[max_ppn_itr + num_nodes_itr * max_ppn])
                        ->rank;
                MPI_CHECK(MPI_Isend(&parent_rank, 1, MPI_INT, child_rank, 102,
                                    comm, &send_reqstat[reqstat_itr++]));
                if (0 == max_ppn_itr) {
                    queue[num_nodes_itr - 1] = child_rank;
                } else {
                    MPI_CHECK(MPI_Isend(&child_rank, 1, MPI_INT, parent_rank,
                                        102, comm,
                                        &send_reqstat[reqstat_itr++]));
                }
            }
        }
    }
    if (0 == fan_in_out_info.is_parent) {
        num_nodes = 2;
    }
    recv_reqstat = malloc(sizeof(MPI_Request) * (num_nodes - 1));
    OMB_CHECK_NULL_AND_EXIT(recv_reqstat, "Unable to allocate memory.");
    if (0 != rank) {
        for (num_nodes_itr = 1; num_nodes_itr < num_nodes; num_nodes_itr++) {
            MPI_CHECK(MPI_Irecv(&queue[num_nodes_itr - 1], 1, MPI_INT, 0, 102,
                                comm, &recv_reqstat[num_nodes_itr - 1]));
            MPI_Waitall(1, &recv_reqstat[num_nodes_itr - 1],
                        MPI_STATUSES_IGNORE);
        }
    }
    if (0 == rank) {
        MPI_Waitall(reqstat_itr - 1, send_reqstat, MPI_STATUSES_IGNORE);
    } else {
        MPI_Waitall(num_nodes - 1, recv_reqstat, MPI_STATUSES_IGNORE);
    }

    fan_in_out_info.total_nodes = num_nodes;
    fan_in_out_info.ppn = max_ppn;
    fan_in_out_info.ranks_queue = queue;
    fan_in_out_info.cur_node_info = cur_node_info;
    MPI_CHECK(MPI_Bcast(&fan_in_out_info.ppn, 1, MPI_INT, 0, comm));
    MPI_CHECK(MPI_Bcast(&fan_in_out_info.total_nodes, 1, MPI_INT, 0, comm));

    for (comm_itr = 0; comm_itr < comm_size; comm_itr++) {
        free(all_node_info[comm_itr]);
    }
    free(all_node_info);
    free(is_parent_arr);
    free(all_ranks);
    free(send_reqstat);
    free(recv_reqstat);
    free(all_proc_names);
    return fan_in_out_info;
}

void omb_fan_free(fan_in_out_info_t fan_in_out_info)
{
    free(fan_in_out_info.ranks_queue);
    free(fan_in_out_info.cur_node_info->proc_name);
    free(fan_in_out_info.cur_node_info);
}
