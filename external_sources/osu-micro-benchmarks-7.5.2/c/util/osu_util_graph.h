/*
 *Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 *(NBCL), The Ohio State University.
 *
 *Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 *For detailed copyright and licensing information, please refer to the
 *copyright file COPYRIGHT in the top level OMB directory.
 */

#include <stdio.h>
#include <stdlib.h>

#define OMB_GRAPH_FLOATING_PRECISION 15
#define OMB_DUMB_COL_SIZE            120
#define OMB_DUMB_ROW_SIZE            40
#define OMB_PNG_COL_SIZE             2048
#define OMB_PNG_ROW_SIZE             1080
#define OMB_ENV_ROW_SIZE             "OMB_ROW_SIZE"
#define OMB_ENV_COL_SIZE             "OMB_COL_SIZE"
#define OMB_ENV_SLURM_ROW_SIZE       "SLURM_PTY_WIN_ROW"
#define OMB_ENV_SLURM_COL_SIZE       "SLURM_PTY_WIN_COL"
#define OMB_PNG_MAX_FILE_LENGTH      30
#define OMB_CMD_MAX_LENGTH           1024
#define OMB_GNUPLOT_PNG_FONT_SIZE    20
#define OMB_GNUPLOT_DATA_FILENAME    "plot_data.dat"
#define OMB_3D_GRAPH_PARTS           2 /*Not to be modified*/

#ifndef _GNUPLOT_BUILD_PATH_
#define OMB_GNUPLOT_PATH "gnuplot"
#else
#define OMB_GNUPLOT_PATH _GNUPLOT_BUILD_PATH_
#endif
#ifndef _CONVERT_BUILD_PATH_
#define OMB_CONVERT_PATH "convert"
#else
#define OMB_CONVERT_PATH _CONVERT_BUILD_PATH_
#endif

typedef struct omb_terminal_size {
    size_t row_size;
    size_t col_size;
} omb_terminal_size_t;

typedef struct omb_graph_data {
    double *data;
    size_t length;
    double avg;
    size_t message_size;
} omb_graph_data_t;

typedef struct omb_graph_options {
    FILE *gnuplot_pointer;
    size_t number_of_graphs;
    omb_graph_data_t **graph_datas;
} omb_graph_options_t;

struct omb_terminal_size omb_get_terminal_size();
int omb_graph_init(omb_graph_options_t *graph_options);
void omb_graph_options_init(omb_graph_options_t *graph_options);
void omb_graph_plot(omb_graph_options_t *graph_options, const char *filename);
void omb_graph_combined_plot(omb_graph_options_t *graph_options,
                             const char *filename);
void omb_graph_allocate_data_buffer(omb_graph_options_t *graph_options,
                                    size_t message_size, size_t length);
int omb_graph_free_data_buffers(omb_graph_options_t *graph_options);
void omb_graph_close(omb_graph_options_t *graph_options);
void omb_graph_allocate_and_get_data_buffer(omb_graph_data_t **graph_data,
                                            omb_graph_options_t *graph_options,
                                            size_t message_size, size_t length);
void omb_graph_allocate_and_get_data_buffer_cus_rank(
    omb_graph_data_t **graph_data, omb_graph_options_t *graph_options,
    size_t message_size, size_t length, int cus_rank);
