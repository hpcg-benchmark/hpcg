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
#include <math.h>

int omb_graph_init(omb_graph_options_t *graph_options)
{
    char cmd[OMB_CMD_MAX_LENGTH];
    int return_code = -1;
    graph_options->gnuplot_pointer = popen(OMB_GNUPLOT_PATH, "w");
    OMB_CHECK_NULL_AND_EXIT(graph_options->gnuplot_pointer,
                            "Unable to open"
                            " Gnuplot.\nPlease check Gnuplot installation\n");
    if (options.graph_output_png || options.graph_output_pdf) {
        sprintf(cmd,
                "%s -e 'print GPVAL_TERMINALS' 2>&1 | tr ' ' '\n'"
                " |grep png > /dev/null",
                OMB_GNUPLOT_PATH);
        return_code = system(cmd);
        if (1 == WEXITSTATUS(return_code)) {
            fprintf(stderr,
                    "Error: Gnuplot does not support png"
                    " terminal.\nPlease rebuild/install Gnuplot with png"
                    " terminal.\n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

void omb_graph_options_init(omb_graph_options_t *graph_options)
{
    graph_options->number_of_graphs = 0;
}

void omb_graph_allocate_data_buffer(omb_graph_options_t *graph_options,
                                    size_t message_size, size_t length)
{
    omb_graph_data_t *graph_data = NULL;
    int rank = -1;

    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (!options.graph) {
        return;
    }
    OMB_CHECK_NULL_AND_EXIT(graph_options, "NULL pointer received");
    graph_data = malloc(sizeof(omb_graph_data_t));
    OMB_CHECK_NULL_AND_EXIT(graph_data, "Unable to allocate memory");
    graph_data->data = malloc(sizeof(double) * length);
    OMB_CHECK_NULL_AND_EXIT(graph_data->data, "Unable to allocate memory");
    graph_data->length = length;
    graph_data->message_size = message_size;
    if (0 == graph_options->number_of_graphs) {
        graph_options->graph_datas =
            malloc(sizeof(struct graph_data *) *
                   ((int)log2(options.max_message_size) + 2));
    }
    graph_options->graph_datas[graph_options->number_of_graphs++] = graph_data;
    return;
}

int omb_graph_free_data_buffers(omb_graph_options_t *graph_options)
{
    int i = 0;
    int rank = -1;

    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (!options.graph || NULL == graph_options) {
        return 0;
    }
    for (i = 0; i < graph_options->number_of_graphs; i++) {
        omb_graph_data_t *graph_data = graph_options->graph_datas[i];
        free(graph_data->data);
        free(graph_options->graph_datas[i]);
    }
    if (0 != graph_options->number_of_graphs) {
        free(graph_options->graph_datas);
    }
    return 0;
}

struct omb_terminal_size omb_get_terminal_size()
{
    char *output = NULL;
    omb_terminal_size_t term_size;

    output = getenv(OMB_ENV_ROW_SIZE);
    if (NULL == output) {
        output = getenv(OMB_ENV_SLURM_ROW_SIZE);
        if (NULL == output) {
            term_size.row_size = OMB_DUMB_ROW_SIZE;
        } else {
            term_size.row_size = atoi(output);
        }
    } else {
        term_size.row_size = atoi(output);
    }
    output = getenv(OMB_ENV_COL_SIZE);
    if (NULL == output) {
        output = getenv(OMB_ENV_SLURM_COL_SIZE);
        if (NULL == output) {
            term_size.col_size = OMB_DUMB_COL_SIZE;
        } else {
            term_size.col_size = atoi(output);
        }
    } else {
        term_size.col_size = atoi(output);
    }
    return term_size;
}

void omb_graph_plot(omb_graph_options_t *graph_options, const char *filename)
{
    int i = 0, graph_itr = 0;
    char png_file_name[OMB_PNG_MAX_FILE_LENGTH];
    omb_terminal_size_t terminal_size;

    OMB_CHECK_NULL_AND_EXIT(graph_options, "NULL pointer received");
    if (0 == graph_options->number_of_graphs) {
        return;
    }
    for (graph_itr = 0; graph_itr < graph_options->number_of_graphs;
         graph_itr++) {
        omb_graph_data_t *graph_data = graph_options->graph_datas[graph_itr];
        if (0 != omb_graph_init(graph_options)) {
            fprintf(stderr, "Failed to open gnuplot \n");
            return;
        }
        terminal_size = omb_get_terminal_size();
        fprintf(graph_options->gnuplot_pointer, "set style data dots\n");
        if ((MBW_MR == options.bench) || (BW == options.subtype) ||
            (CONG_BW == options.subtype)) {
            fprintf(graph_options->gnuplot_pointer, "set ylabel \"BW(MB/s)\""
                                                    " offset character 3,0\n");
        } else {
            fprintf(graph_options->gnuplot_pointer, "set ylabel \"Lat(us)\""
                                                    "offset character 3,0\n");
        }
        fprintf(graph_options->gnuplot_pointer, "set xlabel \"Iteration"
                                                " Count\"\n");
        fprintf(graph_options->gnuplot_pointer,
                "set title \"Size: %zu, Avg:"
                "%.*f\"\n",
                graph_data->message_size, FLOAT_PRECISION, graph_data->avg);
        if (options.graph_output_term) {
            fprintf(graph_options->gnuplot_pointer,
                    "set terminal dumb %zu %zu\n", terminal_size.col_size,
                    terminal_size.row_size);
            fprintf(graph_options->gnuplot_pointer, "plot '-' notitle lc rgb"
                                                    " \"blue\"\n");
            for (i = 0; i < graph_data->length; i++) {
                fprintf(graph_options->gnuplot_pointer, "%d %.*lf\n", i + 1,
                        OMB_GRAPH_FLOATING_PRECISION, graph_data->data[i]);
            }
            fprintf(graph_options->gnuplot_pointer, "e\n");
            fflush(graph_options->gnuplot_pointer);
        }
        if (options.graph_output_png || options.graph_output_pdf) {
            sprintf(png_file_name, "%s-%zu", filename,
                    graph_data->message_size);
            fprintf(graph_options->gnuplot_pointer,
                    "set terminal png size %d,"
                    " %d\n",
                    OMB_PNG_COL_SIZE, OMB_PNG_ROW_SIZE);
            fprintf(graph_options->gnuplot_pointer, "set output \"%s.png\"\n",
                    png_file_name);
            fprintf(graph_options->gnuplot_pointer, "set grid\n");
            fprintf(graph_options->gnuplot_pointer,
                    "plot '-' with linespoints"
                    " notitle pt 20 lc rgb \"blue\"\n");
            for (i = 0; i < graph_data->length; i++) {
                fprintf(graph_options->gnuplot_pointer, "%d %.*lf\n", i + 1,
                        OMB_GRAPH_FLOATING_PRECISION, graph_data->data[i]);
            }
            fprintf(graph_options->gnuplot_pointer, "e\n");
            fflush(graph_options->gnuplot_pointer);
        }
        omb_graph_close(graph_options);
    }
}

void omb_graph_combined_plot(omb_graph_options_t *graph_options,
                             const char *filename)
{
    char png_file_name[OMB_PNG_MAX_FILE_LENGTH];
    char cmd[OMB_CMD_MAX_LENGTH];
    int i = 0, k = 0, l = 0;
    int current_graph_index = 0;
    int gnuplot_index = 0;
    int return_code = 0;
    int rank = -1;

    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (!options.graph || 0 != rank) {
        return;
    }
    if (0 != omb_graph_init(graph_options)) {
        fprintf(stderr, "Failed to open gnuplot \n");
        return;
    }
    if (options.graph_output_png || options.graph_output_pdf) {
        fprintf(graph_options->gnuplot_pointer, "set table '%s'\n",
                OMB_GNUPLOT_DATA_FILENAME);
        fprintf(graph_options->gnuplot_pointer, "splot '-' using 1:2:3\n");
        for (l = 0; l < graph_options->number_of_graphs; l++) {
            omb_graph_data_t *graph_data = graph_options->graph_datas[l];
            for (k = 0; k < graph_data->length; k++) {
                fprintf(graph_options->gnuplot_pointer, "%d %d %.*lf\n", l,
                        k + 1, OMB_GRAPH_FLOATING_PRECISION,
                        graph_data->data[k]);
            }
            fprintf(graph_options->gnuplot_pointer, "\n\n");
        }
        fprintf(graph_options->gnuplot_pointer, "unset table\n");
        fflush(graph_options->gnuplot_pointer);
        omb_graph_close(graph_options);
        if (0 != omb_graph_init(graph_options)) {
            fprintf(stderr, "Failed to open gnuplot \n");
            return;
        }
        for (i = 0; i < OMB_3D_GRAPH_PARTS; i++) {
            sprintf(png_file_name, "%s%s%d", filename, "3D", i);
            fprintf(graph_options->gnuplot_pointer,
                    "set terminal png size %d,"
                    " %d\n",
                    OMB_PNG_COL_SIZE, OMB_PNG_ROW_SIZE);
            fprintf(graph_options->gnuplot_pointer, "set output \"%s.png\"\n",
                    png_file_name);
            fprintf(graph_options->gnuplot_pointer, "set title \"%s\"\n",
                    (i == 0) ? "Small Message Size" : "Large Message Size");
            if (MBW_MR == options.bench || BW == options.subtype ||
                CONG_BW == options.subtype) {
                fprintf(
                    graph_options->gnuplot_pointer,
                    "set zlabel rotate"
                    " parallel offset -3,0 \"{/:Bold Bandwidth (MB/s)}\" font"
                    " \",%d\"\n",
                    OMB_GNUPLOT_PNG_FONT_SIZE);
            } else {
                fprintf(graph_options->gnuplot_pointer,
                        "set zlabel rotate"
                        " parallel offset -3,0 \"{/:Bold Latency (us)}\" font"
                        " \",%d\"\n",
                        OMB_GNUPLOT_PNG_FONT_SIZE);
            }
            fprintf(graph_options->gnuplot_pointer,
                    "set xlabel rotate"
                    " parallel offset 0,-3 \"{/:Bold Size (Bytes)}\" font"
                    " \",%d\"\n",
                    OMB_GNUPLOT_PNG_FONT_SIZE);
            fprintf(graph_options->gnuplot_pointer,
                    "set ylabel rotate"
                    " parallel offset 0,-3 \"{/:Bold Iteration Count}\" font"
                    " \",%d\"\n",
                    OMB_GNUPLOT_PNG_FONT_SIZE);
            fprintf(graph_options->gnuplot_pointer, "set logscale z 10\n");
            fprintf(graph_options->gnuplot_pointer, "set ticslevel 0\n");
            fprintf(graph_options->gnuplot_pointer, "set grid ytics ztics\n");
            fprintf(graph_options->gnuplot_pointer, "set key font \",%d\"\n",
                    OMB_GNUPLOT_PNG_FONT_SIZE);
            fprintf(graph_options->gnuplot_pointer, "set key spacing 2\n");
            fprintf(graph_options->gnuplot_pointer,
                    "set arrow from graph 1,0,0"
                    " to graph 1.1,0,0 filled\n");
            fprintf(graph_options->gnuplot_pointer,
                    "set arrow from graph"
                    " 0,0,0.1 to graph 0,0,1.1 filled\n");
            fprintf(graph_options->gnuplot_pointer,
                    "set arrow from graph 1,0,0"
                    " to graph 1,1.1,0 filled\n");
            fprintf(graph_options->gnuplot_pointer,
                    "set ytics offset 0,-1 font"
                    " \",%d\"\n",
                    OMB_GNUPLOT_PNG_FONT_SIZE);
            fprintf(graph_options->gnuplot_pointer, "set ztics font \",%d\"\n",
                    OMB_GNUPLOT_PNG_FONT_SIZE);
            fprintf(graph_options->gnuplot_pointer, "set title font \",%d\"\n",
                    OMB_GNUPLOT_PNG_FONT_SIZE);
            fprintf(graph_options->gnuplot_pointer,
                    "set xtics offset 0,-2 font"
                    " \",%d\" (",
                    OMB_GNUPLOT_PNG_FONT_SIZE);
            for (l = 0; l < graph_options->number_of_graphs - 1; l++) {
                fprintf(graph_options->gnuplot_pointer, "\"%zu\" %d,",
                        graph_options->graph_datas[l]->message_size, l);
            }
            fprintf(graph_options->gnuplot_pointer, "\"%zu\" %d)\n",
                    graph_options->graph_datas[l]->message_size, l);
            gnuplot_index = 0;
            fprintf(graph_options->gnuplot_pointer, "splot ");
            while (current_graph_index <= graph_options->number_of_graphs) {
                fprintf(graph_options->gnuplot_pointer,
                        "'%s' using 1:2:3 index"
                        " %d with linespoints pt 20 lc  %d t sprintf('%zu') ",
                        OMB_GNUPLOT_DATA_FILENAME, current_graph_index,
                        gnuplot_index,
                        graph_options->graph_datas[current_graph_index]
                            ->message_size);
                gnuplot_index++;
                if (((current_graph_index + 1) ==
                     graph_options->number_of_graphs) ||
                    (graph_options->graph_datas[current_graph_index]
                         ->message_size == LARGE_MESSAGE_SIZE)) {
                    fprintf(graph_options->gnuplot_pointer, "\n");
                    current_graph_index++;
                    break;
                } else {
                    fprintf(graph_options->gnuplot_pointer, ", ");
                    current_graph_index++;
                }
            }
            fflush(graph_options->gnuplot_pointer);
            if (current_graph_index >= graph_options->number_of_graphs) {
                break;
            }
        }
    }
    omb_graph_close(graph_options);
    if (options.graph_output_png || options.graph_output_pdf) {
        sprintf(cmd, "rm %s", OMB_GNUPLOT_DATA_FILENAME);
        system(cmd);
    }
    if (options.graph_output_pdf) {
        sprintf(cmd, "%s $(ls -v %s-*.png) $(ls -v %s3D*.png) %s.pdf",
                OMB_CONVERT_PATH, filename, filename, filename);
        return_code = system(cmd);
    }
    if (!options.graph_output_png && options.graph_output_pdf) {
        sprintf(cmd, "rm $(ls -v %s-*.png) $(ls -v %s3D*.png)", filename,
                filename);
        system(cmd);
    }
    if (0 != WEXITSTATUS(return_code)) {
        fprintf(stderr, "Unable to use covert command.\n"
                        " Please check \'convert\' command installation.\n");
        exit(EXIT_FAILURE);
    }
}

void omb_graph_close(omb_graph_options_t *graph_options)
{
    pclose(graph_options->gnuplot_pointer);
}

void omb_graph_allocate_and_get_data_buffer(omb_graph_data_t **graph_data,
                                            omb_graph_options_t *graph_options,
                                            size_t message_size, size_t length)
{
    int rank = -1;

    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (options.graph && 0 == rank) {
        omb_graph_allocate_data_buffer(graph_options, message_size, length);
        *graph_data =
            graph_options->graph_datas[graph_options->number_of_graphs - 1];
    }
}

void omb_graph_allocate_and_get_data_buffer_cus_rank(
    omb_graph_data_t **graph_data, omb_graph_options_t *graph_options,
    size_t message_size, size_t length, int cus_rank)
{
    int rank = -1;

    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (options.graph && cus_rank == rank) {
        omb_graph_allocate_data_buffer(graph_options, message_size, length);
        *graph_data =
            graph_options->graph_datas[graph_options->number_of_graphs - 1];
    }
}
