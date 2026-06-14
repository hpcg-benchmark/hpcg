/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include <osu_util_mpi.h>

#ifdef _ENABLE_PAPI_
static char **PAPI_values = NULL;
static int omb_papi_noe = 0;
static FILE *omb_papi_output = NULL;
char omb_papi_output_filename[OMB_PAPI_FILE_PATH_MAX_LENGTH];
#endif /*#ifdef _ENABLE_PAPI_*/

void omb_papi_init(int *papi_eventset)
{
#ifdef _ENABLE_PAPI_
    int papi_retval = 0, i = 0;
    omb_papi_output = fopen(omb_papi_output_filename, "w");
    if (!options.papi_enabled) {
        return;
    }
    papi_retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (papi_retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "Error initializing PAPI: %s\n",
                PAPI_strerror(papi_retval));
        exit(EXIT_FAILURE);
    }
    papi_retval = PAPI_multiplex_init();
    if (papi_retval != PAPI_OK) {
        fprintf(stderr, "Error initializing multiplexing. %s\n",
                PAPI_strerror(papi_retval));
        exit(EXIT_FAILURE);
    }
    papi_retval = PAPI_create_eventset(papi_eventset);
    if (papi_retval != PAPI_OK) {
        fprintf(stderr, "Error creating eventset! %s\n",
                PAPI_strerror(papi_retval));
        exit(EXIT_FAILURE);
    }
    papi_retval = PAPI_assign_eventset_component(*papi_eventset, 0);
    if (papi_retval != PAPI_OK) {
        fprintf(stderr, "Error creating eventset 2! %s\n",
                PAPI_strerror(papi_retval));
        exit(EXIT_FAILURE);
    }
    papi_retval = PAPI_set_multiplex(*papi_eventset);
    if (papi_retval != PAPI_OK) {
        fprintf(stderr, "Error setting up multiplexing! %s\n",
                PAPI_strerror(papi_retval));
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < omb_papi_noe; i++) {
        papi_retval = PAPI_add_named_event(*papi_eventset, PAPI_values[i]);
        if (papi_retval != PAPI_OK) {
            fprintf(stderr, "Error adding %s: %s\n", PAPI_values[i],
                    PAPI_strerror(papi_retval));
            exit(EXIT_FAILURE);
        }
    }
    PAPI_reset(*papi_eventset);
#endif /*#ifdef _ENABLE_PAPI_*/
}

void omb_papi_start(int *papi_eventset)
{
#ifdef _ENABLE_PAPI_
    int papi_retval = 0;

    if (!options.papi_enabled) {
        return;
    }
    papi_retval = PAPI_start(*papi_eventset);
    if (papi_retval != PAPI_OK) {
        fprintf(stderr, "Error Starting PAPI: %s\n",
                PAPI_strerror(papi_retval));
        exit(EXIT_FAILURE);
    }
#endif /*#ifdef _ENABLE_PAPI_*/
}

void omb_papi_stop_and_print(int *papi_eventset, int size)
{
#ifdef _ENABLE_PAPI_
    int myid = 0, i = 0, j = 0, papi_retval = 0, numprocs = 0;
    long long *count;
    long long *recvbuf;

    if (!options.papi_enabled) {
        return;
    }
    count = malloc(sizeof(long long) * omb_papi_noe);
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &myid));
    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
    recvbuf = malloc(numprocs * sizeof(long long));
    memset(recvbuf, 0, numprocs * sizeof(long long));
    memset(count, 0, omb_papi_noe * sizeof(long long));
    papi_retval = PAPI_stop(*papi_eventset, count);
    if (papi_retval != PAPI_OK && papi_retval != PAPI_ENOTRUN) {
        fprintf(stderr, "Error Stopping PAPI: %s\n",
                PAPI_strerror(papi_retval));
        exit(EXIT_FAILURE);
    }
    if (myid == 0) {
        fprintf(omb_papi_output, "Size: %d\n", size);
        fprintf(omb_papi_output, ">>========================================"
                                 "=======================>>\n");
        fprintf(omb_papi_output, "%-*s", FIELD_WIDTH, "PAPI Event Name");
        for (j = 0; j < numprocs; j++) {
            fprintf(omb_papi_output, "%*s:%d", FIELD_WIDTH - 2, "Rank", j);
        }
        fprintf(omb_papi_output, "\n");
    }
    for (i = 0; i < omb_papi_noe; i++) {
        MPI_CHECK(MPI_Gather((void *)&count[i], 1, MPI_LONG_LONG, recvbuf, 1,
                             MPI_LONG_LONG, 0, MPI_COMM_WORLD));
        if (myid == 0) {
            fprintf(omb_papi_output, "%-*s", FIELD_WIDTH, PAPI_values[i]);
            for (j = 0; j < numprocs; j++) {
                fprintf(omb_papi_output, "%*lld", FIELD_WIDTH, recvbuf[j]);
            }
            fprintf(omb_papi_output, "\n");
        }
    }
    if (myid == 0) {
        fprintf(omb_papi_output, "##=========================================="
                                 "=====================##\n\n");
        fflush(omb_papi_output);
    }
    PAPI_reset(*papi_eventset);
    free(count);
    free(recvbuf);
#endif /*#ifdef _ENABLE_PAPI_*/
}

void omb_papi_free(int *papi_eventset)
{
#ifdef _ENABLE_PAPI_
    if (!options.papi_enabled) {
        return;
    }
    PAPI_cleanup_eventset(*papi_eventset);
    PAPI_destroy_eventset(papi_eventset);
    free(PAPI_values);
#endif /*#ifdef _ENABLE_PAPI_*/
}

void omb_papi_parse_event_options(char *opt_arr)
{
#ifdef _ENABLE_PAPI_
    char *token, *opt_token, *file_name;
    PAPI_values = malloc(sizeof(char *) * OMB_PAPI_NUMBER_OF_EVENTS);
    opt_token = strtok(opt_arr, ":");
    file_name = strtok(NULL, ":");
    token = strtok(opt_token, ",");
    while (NULL != token) {
        PAPI_values[omb_papi_noe] = malloc(sizeof(char) * strlen(token));
        strcpy(PAPI_values[omb_papi_noe], token);
        token = strtok(NULL, ",");
        omb_papi_noe++;
    }
    if (NULL != file_name) {
        if (OMB_PAPI_FILE_PATH_MAX_LENGTH < strlen(file_name)) {
            fprintf(stderr,
                    "ERROR: Max allowed size for filepath is:%d\n"
                    "To increase the max allowed filepath limit, update"
                    " OMB_PAPI_FILE_PATH_MAX_LENGTH in c/util/osu_util.h.\n",
                    OMB_PAPI_FILE_PATH_MAX_LENGTH);
            fflush(stderr);
        }
        strcpy(omb_papi_output_filename, file_name);
    } else {
        fprintf(stderr, "Warning! Using"
                        " \'papi_output.out for papi results\n");
        fflush(stderr);
        strcpy(omb_papi_output_filename, "papi_output.out");
    }
#endif /*#ifdef _ENABLE_PAPI_*/
}
/* vi:set sw=4 sts=4 tw=80: */
