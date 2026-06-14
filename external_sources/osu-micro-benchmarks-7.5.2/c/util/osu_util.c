/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level directory.
 */

#include "osu_util.h"
#ifdef _ENABLE_OPENACC_
#include <openacc.h>
#endif

/*
 * GLOBAL VARIABLES
 */
char const *benchmark_header = NULL;
char const *benchmark_name = NULL;
int accel_enabled = 0;
struct options_t options = {0};

struct bad_usage_t bad_usage;

void print_header(int rank, int full)
{
    int itr = 0;

    switch (options.bench) {
        case MBW_MR:
        case PT2PT:
            if (0 == rank) {
                switch (options.accel) {
                    case CUDA:
                    case OPENACC:
                    case ROCM:
                    case SYCL:
                        fprintf(
                            stdout,
                            "# Send Buffer on %s and Receive Buffer on %s\n",
                            'M' == options.src ?
                                ('D' == options.MMsrc ? "MANAGED (MD)" :
                                                        "MANAGED (MH)") :
                                ('D' == options.src ? "DEVICE (D)" :
                                                      "HOST (H)"),
                            'M' == options.dst ?
                                ('D' == options.MMdst ? "MANAGED (MD)" :
                                                        "MANAGED (MH)") :
                                ('D' == options.dst ? "DEVICE (D)" :
                                                      "HOST (H)"));
                    default:
                        if ((BW == options.subtype ||
                             CONG_BW == options.subtype) &&
                            MBW_MR != options.bench) {
                            fprintf(stdout, "%-*s%*s", 10, "# Size",
                                    FIELD_WIDTH, "Bandwidth (MB/s)");
                        } else if (options.subtype == LAT ||
                                   options.subtype == PART_LAT) {
                            fprintf(stdout, "%-*s%*s", 10, "# Size",
                                    FIELD_WIDTH, "Latency (us)");
                        } else if (options.subtype == LAT_MP) {
                            fprintf(stdout, "%-*s%*s", 10, "# Size",
                                    FIELD_WIDTH, "Latency (us)");
                        } else if (options.subtype == LAT_MT) {
                            fprintf(stdout, "%-*s%*s", 10, "# Size",
                                    FIELD_WIDTH, "Latency (us)");
                        }
                        if (options.validate && !(options.subtype == BW &&
                                                  options.bench == MBW_MR)) {
                            fprintf(stdout, "%*s", FIELD_WIDTH, "Validation");
                        }
                        if (options.omb_tail_lat) {
                            itr = 0;
                            while (itr < OMB_STAT_MAX_NUM &&
                                   -1 != options.omb_stat_percentiles[itr]) {
                                if (BW == options.subtype) {
                                    fprintf(
                                        stdout, "%*sP%d Tail BW(MB/s)",
                                        FIELD_WIDTH -
                                            strlen("Px Tail BW(MB/s)") -
                                            (options.omb_stat_percentiles[itr] >
                                             9),
                                        "", options.omb_stat_percentiles[itr]);
                                } else {
                                    fprintf(
                                        stdout, "%*sP%d Tail Lat(us)",
                                        FIELD_WIDTH -
                                            strlen("Px Tail Lat(us)") -
                                            (options.omb_stat_percentiles[itr] >
                                             9),
                                        "", options.omb_stat_percentiles[itr]);
                                }
                                itr++;
                            }
                        }
                        if (options.omb_enable_ddt &&
                            !(options.subtype == BW &&
                              options.bench == MBW_MR)) {
                            fprintf(stdout, "%*s", FIELD_WIDTH,
                                    "Transmit Size");
                        }
                        fprintf(stdout, "\n");
                        fflush(stdout);
                }
            }
            break;
        case COLLECTIVE:
            if (rank == 0) {
                fprintf(stdout, HEADER, "");

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
            break;
        default:
            break;
    }
}

void print_data(int rank, int full, int size, double avg_time, double min_time,
                double max_time, int iterations)
{
    if (rank == 0) {
        if (options.show_size) {
            fprintf(stdout, "%-*d", 10, size);
            fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION, avg_time);
        } else {
            fprintf(stdout, "%*.*f", 17, FLOAT_PRECISION, avg_time);
        }

        if (full) {
            fprintf(stdout, "%*.*f%*.*f%*d\n", FIELD_WIDTH, FLOAT_PRECISION,
                    min_time, FIELD_WIDTH, FLOAT_PRECISION, max_time, 12,
                    iterations);
        } else {
            fprintf(stdout, "\n");
        }

        fflush(stdout);
    }
}
static int check_iteration_value(char *optarg, char type)
{
    int val = atoi(optarg);
    /*
     * note - there is no way to differentiate between passing a 0 argument
     * and an error with atoi. Maybe switch to strtol() and check errno?
     */
    if (0 == val && 'i' == type) {
        bad_usage.message = "Invalid iterations argument";

        bad_usage.optarg = optarg;
        /*
         * the enum for PO_BAD_USAGE is 2, which is a valid return here.
         * so need to special case here.
         */
        return -1;
    } else {
        return val;
    }
}

static int process_iterations(char *optarg, char type)
{
    /*
     * This routine is used for -x (skip iterations) and -i (measured
     * iterations)
     *
     * There are 3 cases to look at.
     * 1) -i or -x XXX - the basic case, same behavior as today.
     *    All message sizes will use XXX for the iteration count
     * 2) -i or -x Size:Iteration Count LT:Iteration Count GT - similar
     *    to behavior today. User can specify a message size and the
     *    number of iterations for sizes less than that cutoff and sizes
     *    greater than that cutoff. -i 8192:10000:1000 would be the same
     *    behavior as today (and the default if -i is not specified)
     * 3) -i or -x Size1:Count1,Size2:Count2,...SizeN:CountN - allows
     *    fine grain switching. Messages from 0 or 1 up to Size1 have
     *    Count1 iterations. From Size1 to Size2, Count2 iterations, ...
     *    SizeN-1 to SizeN have CountN-1 iterations. The last SizeN
     *    value is not actually used. So:
     *    -i 8192:1000,131072:100,16777216:5,16777217:2
     *    says 1..8192 -> 1000 iterations. 8192..131072 -> 100 iterations
     *    131072..16777216  -> 5 iterations. Over 16777216 -> 2 iterations
     */

    char *str, *tofree, *tofree2, *token, *subtoken;
    int retval, count = 0, val, itr = 0;

    /*
     * strsep clobbers the input string so make a copy of it, but
     * also keep track of the pointer to later free it.
     */
    tofree = str = strdup(optarg);

    /* what sort of argument was provided? */
    while ((token = strsep(&str, ":,")) != NULL)
        count++;

    /*
     * A single value was provided. This is the same behavior as
     * previously.   * Just set all message size iterations/skips
     * to the value provided.
     */
    if (1 == count) {
        val = check_iteration_value(optarg, type);
        if (-1 == val) {
            free(tofree);
            return PO_BAD_USAGE;
        }

        /* Was it iteration (-i) or skip (-x)? */
        if ('i' == type) {
            options.iteration_sizes[0] = 0;
            options.iteration_counts[0] = val;
            options.num_iteration_sizes = 1;

            /* TODO: legacy until every benchmark is changed */
            options.iterations = val;
            options.iterations_large = val;
        } else {
            options.skip_sizes[0] = 0;
            options.iteration_counts[0] = val;
            options.num_skip_sizes = 1;

            options.skip = val;
            options.skip_large = val;
        }
        free(tofree);
        return 0;
    }

    /*
     * We found two tokens (so a single colon or single comma), but that
     * is not a valid argument. Abort.
     */
    if (2 == count) {
        if ('i' == type)
            bad_usage.message = "Invalid iteration argument";
        else
            bad_usage.message = "Invalid warmup argument";
        bad_usage.optarg = optarg;
        free(tofree);
        return PO_BAD_USAGE;
    }

    /* We found 2 tokens so this should be xxx:yyy:zzz */
    if (3 == count) {
        tofree = str = strdup(optarg);
        token = strsep(&str, ":");

        val = check_iteration_value(token, type);
        if (-1 == val)
            return PO_BAD_USAGE;

        if ('i' == type)
            options.iteration_sizes[1] = val;
        else
            options.skip_sizes[1] = val;

        token = strsep(&str, ":"); /* grab the next 2 values */
        /* the middle value, aka count for messages < size */
        val = check_iteration_value(token, type);
        if (-1 == val)
            return PO_BAD_USAGE;

        if ('i' == type)
            options.iteration_counts[0] = val;
        else
            options.skip_counts[0] = val;

        /* the right hand side */
        val = check_iteration_value(str, type);
        if (-1 == val)
            return PO_BAD_USAGE;

        /*
         * It would be nice to change the LARGE_MESSAGE_SIZE macro into an
         * options member
         */
        if ('i' == type) {
            options.iteration_sizes[0] = 0;
            options.iteration_counts[1] = val;
            options.num_iteration_sizes = 2;
            /* TODO: legacy until every benchmark is changed */
            options.iterations = options.iteration_counts[0];
            options.iterations_large = options.iteration_counts[1];
        } else {
            options.skip_sizes[0] = 0;
            options.skip_counts[1] = val;
            options.num_skip_sizes = 2;
            /* TODO: legacy until every benchmark is changed */
            options.skip = options.skip_counts[0];
            options.skip_large = options.skip_counts[1];
        }

        free(tofree);
        return 0;
    }

    /* An even number of tokens, so should be X:Y,...XN:YN */
    if (0 == count % 2) {
        tofree = str = strdup(optarg);
        itr = 0;

        /* process each group */
        while ((token = strsep(&str, ",")) != NULL) {
            subtoken = strsep(&token, ":");
            if (subtoken == NULL || token == NULL) {
                /* We should have 2 tokens. If not abort */
                if ('i' == type)
                    bad_usage.message = "Invalid iteration argument";
                else
                    bad_usage.message = "Invalid warmup argument";
                bad_usage.optarg = optarg;
                return PO_BAD_USAGE;
            }

            val = check_iteration_value(token, type);
            if (-1 == val)
                return PO_BAD_USAGE;

            if ('i' == type)
                options.iteration_counts[itr] = atoi(token);
            else
                options.skip_counts[itr] = atoi(token);

            val = check_iteration_value(subtoken, type);
            if (-1 == val)
                return PO_BAD_USAGE;

            if ('i' == type) {
                options.iteration_sizes[itr] = atoi(subtoken);
                options.num_iteration_sizes++;
            } else {
                options.skip_sizes[itr] = atoi(subtoken);
                options.num_skip_sizes++;
            }
            itr++;

            /* make sure the user didn't provide too many ranges. */
            if ('i' == type && itr > MAX_ITERATION_SIZES - 1) {
                bad_usage.message = "Too many iteration ranges";
                bad_usage.optarg = optarg;
                return PO_BAD_USAGE;
            }
            if ('x' == type && itr > MAX_SKIP_SIZES - 1) {
                bad_usage.message = "Too many skip ranges.";
                bad_usage.optarg = optarg;
                return PO_BAD_USAGE;
            }
        }

        if ('i' == type) {
            options.num_iteration_sizes = itr + 1;
            options.iteration_sizes[options.num_iteration_sizes] = -1;
        } else {
            options.num_skip_sizes = itr + 1;
            options.skip_sizes[options.num_skip_sizes] = -1;
        }

        return 0;

    } else { /* not a multiple of 2, not 1 2 or 3 */
        if ('i' == type)
            bad_usage.message = "Invalid iteration argument";
        else
            bad_usage.message = "Invalid warmup argument";
        bad_usage.optarg = optarg;
        return PO_BAD_USAGE;
    }

    return 0;
}

static int set_min_message_size(long long value)
{
    if (0 >= value) {
        return -1;
    }

    options.min_message_size = value ? value : 1;

    return 0;
}

static int set_max_message_size(long long value)
{
    if (0 > value) {
        return -1;
    }

    options.max_message_size = value;

    return 0;
}
static int set_message_size_incr(int value)
{
    if (0 > value) {
        return -1;
    }
    options.message_size_incr = value;
    return 0;
}

static size_t strpower_to_int(char *val_str)
{
    char *token, *str = strdup(str);
    int base, mant;
    token = strsep(&str, "^");
    base = atoi(token);
    mant = atoi(str);
    return (size_t)pow(base, mant);
}

static int set_message_size(char *val_str)
{
    /*
     * Adds the ability to use "^" for larger message sizes
     * Adds an ability to change the increment multiplier
     * Usage: -m [MIN]:MAX[:INCR]
     * -m 128       - Set the max to 128, leave minimum as is
     * -m :128      - Set the max to 128, leave minimum as-is
     * 		   (same as just -m 128)
     * -m :2^16     - set the max to 2^16, 16384. leave minimum as-is
     * -m :2^16:4   - set the max to 2^16 and increment by size by a
     *                factor of 4
     * -m 65536:    - set the min to 65536, leave max as-is
     * -m 2^20:     - set the min to 1MB, leave max as-is
     * -m 16:2^28:4 - set the min to 16, set max to 256MB,
     * 		   increment by a factor of 4
     */

    int retval = -1;
    char *str, *token, *subtoken, *subsubtoken, *lhs, *freeme, *freeme2;
    int ccount = 0, i;

    /* First, see how many options we have. Could be done with strsep */
    for (i = 0; val_str[i]; i++) {
        if (':' == val_str[i])
            ccount++;
    }

    /* We just have a single value, so setting max */
    if (0 == ccount) {
        /* check if an exponent was provided */
        subtoken = strsep(&token, "^");
        if (NULL == token) {
            retval = set_max_message_size(atoll(subtoken));
        } else {
            retval = set_max_message_size(
                (long long)pow(atoi(subtoken), atoi(token)));
        }
    }
    /* We have at least one colon */
    else {
        freeme = str = strdup(optarg);
        token = strsep(&str, ":");

        /* Do we have a left side of the colon? */
        if (*token != '\0') {
            /*
             * We could have both a min and max (and increment for the rhs
             * of the colon)
             */
            freeme2 = lhs = strdup(token);
            /* Do we have an exponent for the min? */
            subtoken = strsep(&lhs, "^");
            if (NULL == lhs) {
                retval = set_min_message_size(atoll(subtoken));
            } else {
                retval = set_min_message_size(
                    (long long)pow(atoi(subtoken), atoi(token)));
            }
            free(freeme2);
        }
        /* We have a value to the right of a colon */
        if (*str != '\0') {
            if (ccount == 1) {
                subtoken = strsep(&str, "^");
                if (NULL == str) {
                    retval = set_max_message_size(atoll(subtoken));
                } else {
                    retval = set_max_message_size(
                        (long long)pow(atoi(subtoken), atoi(str)));
                }
            } else {
                subtoken = strsep(&str, ":");
                retval = set_message_size_incr(atoi(str));
                subsubtoken = strsep(&subtoken, "^");
                if (NULL == subtoken) {
                    set_max_message_size(atoll(subsubtoken));
                } else {
                    retval = set_max_message_size(
                        (long long)pow(atoi(subsubtoken), atoi(subtoken)));
                }
            }
        }
        free(freeme);
    }

    return retval;
}

static int set_receiver_threads(int value)
{
    if (MIN_NUM_THREADS > value || value >= MAX_NUM_THREADS) {
        return -1;
    }

    options.num_threads = value;

    return 0;
}

static int set_sender_threads(int value)
{
    if (MIN_NUM_THREADS > value || value >= MAX_NUM_THREADS) {
        return -1;
    }

    options.sender_thread = value;

    return 0;
}

static int set_threads(char *val_str)
{
    int retval = -1;
    int i, count = 0;
    char *val1, *val2;

    for (i = 0; val_str[i]; i++) {
        if (val_str[i] == ':')
            count++;
    }

    if (!count) {
        retval = set_receiver_threads(atoi(val_str));
        options.sender_thread = -1;
    } else if (count == 1) {
        val1 = strtok(val_str, ":");
        val2 = strtok(NULL, ":");

        if (val1 && val2) {
            retval = set_sender_threads(atoi(val1));
            if (retval == -1) {
                return retval;
            }
            retval = set_receiver_threads(atoi(val2));
            if (retval == -1) {
                return retval;
            }
        }
    }

    return retval;
}

static int set_receiver_processes(int value)
{
    if (MIN_NUM_PROCESSES > value || value >= MAX_NUM_PROCESSES) {
        return -1;
    }

    options.num_processes = value;

    return 0;
}

static int set_sender_processes(int value)
{
    if (MIN_NUM_PROCESSES > value || value >= MAX_NUM_PROCESSES) {
        return -1;
    }

    options.sender_processes = value;

    return 0;
}

static int set_processes(char *val_str)
{
    int retval = -1;
    int i, count = 0;
    char *val1, *val2;

    for (i = 0; val_str[i]; i++) {
        if (val_str[i] == ':')
            count++;
    }

    if (!count) {
        retval = set_receiver_processes(atoi(val_str));
        options.sender_processes = -1;
    } else if (count == 1) {
        val1 = strtok(val_str, ":");
        val2 = strtok(NULL, ":");

        if (val1 && val2) {
            retval = set_sender_processes(atoi(val1));
            if (retval == -1) {
                return retval;
            }
            retval = set_receiver_processes(atoi(val2));
            if (retval == -1) {
                return retval;
            }
        }
    }

    return retval;
}

static int set_num_warmup(int value)
{
    if (0 > value) {
        return -1;
    }

    options.skip = value;
    options.skip_large = value;

    return 0;
}

static int set_num_partitions(int value)
{
    if (0 > value) {
        return -1;
    }

    options.num_partitions = value;

    return 0;
}

static int set_num_warmup_validation(int value)
{
    if (0 > value) {
        return -1;
    }

    options.warmup_validation = value;

    return 0;
}

static int set_num_iterations(int value)
{
    if (1 > value) {
        return -1;
    }

    options.iterations = value;
    options.iterations_large = value;

    return 0;
}

static int set_window_size(int value)
{
    if (1 > value) {
        return -1;
    }

    options.window_size = value;

    return 0;
}

static int set_device_array_size(int value)
{
    if (value < 1) {
        return -1;
    }

    options.device_array_size = value;

    return 0;
}

static int set_num_probes(int value)
{
    if (value < 0) {
        return -1;
    }

    options.num_probes = value;

    return 0;
}

void set_header(const char *header) { benchmark_header = header; }

void get_iterations(int size)
{
    int low, high, val, index;

    if (options.num_iteration_sizes == 2) {
        if (size > options.iteration_sizes[1])
            options.iterations = options.iteration_counts[1];
        else
            options.iterations = options.iteration_counts[0];
    }
    if (options.num_iteration_sizes > 2) {
        for (int i = 0; i < options.num_iteration_sizes; i++) {
            low = options.iteration_sizes[i];
            high = options.iteration_sizes[i + 1];

            if (i == 0)
                low = 1;

            if (i == options.num_iteration_sizes - 1) {
                options.iterations = options.iteration_counts[i - 1];
                break;
            }

            if (size < high && size >= low) {
                options.iterations = options.iteration_counts[i];
                break;
            }
        }
    }
}

void get_skips(int size)
{
    int low, high, val, index;

    if (options.num_skip_sizes == 2) {
        if (size > options.skip_sizes[1])
            options.skip = options.skip_counts[1];
        else
            options.skip = options.skip_counts[0];
    }

    if (options.num_skip_sizes > 2) {
        for (int i = 0; i < options.num_skip_sizes; i++) {
            low = options.skip_sizes[i];
            high = options.skip_sizes[i + 1];

            if (i == 0)
                low = 1;

            if (i == options.num_skip_sizes - 1) {
                options.skip = options.skip_counts[i - 1];
                break;
            }

            if (size < high && size >= low) {
                options.skip = options.skip_counts[i];
                break;
            }
        }
    }
}

void set_benchmark_name(const char *name) { benchmark_name = name; }

void enable_accel_support(void)
{
    accel_enabled =
        ((CUDA_ENABLED || OPENACC_ENABLED || ROCM_ENABLED || SYCL_ENABLED) &&
         !(options.subtype == LAT_MT || options.subtype == LAT_MP));
}

void omb_process_long_options(struct option *long_options,
                              const char *optstring)
{
    int i = 0, j = 0, num_long_options = 0, omb_long_options_itr = 0;
    struct option const omb_long_options_all[OMB_LONG_OPTIONS_ARRAY_SIZE] =
        OMBOP_LONG_OPTIONS_ALL;
    num_long_options = sizeof(omb_long_options_all) / sizeof(struct option);
    for (i = 0; i < strlen(optstring); i++) {
        if (isalpha(optstring[i])) {
            for (j = 0; j < num_long_options; j++) {
                if (optstring[i] == omb_long_options_all[j].val) {
                    memcpy(&long_options[omb_long_options_itr],
                           &omb_long_options_all[j], sizeof(struct option));
                    omb_long_options_itr++;
                }
            }
        }
    }
}

int process_options(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, optopt;
    int itr = 0, val = 0, count = 0;
    char optstring_buf[80];
    int c, ret = PO_OKAY;
    int option_index = 0;
    char *graph_term_type = NULL;
    char *validation_log_option = NULL;
    char *root_rank_type = NULL;
    char *strtok_parsed = NULL;
    static struct option long_options[OMB_LONG_OPTIONS_ARRAY_SIZE];

    enable_accel_support();
    if (PT2PT == options.bench) {
        switch (options.subtype) {
            case BW:
                OMBOP_OPTSTR_BLK(PT2PT, BW);
                break;
            case LAT:
                OMBOP_OPTSTR_BLK(PT2PT, LAT);
                break;
            case PART_LAT:
                OMBOP_OPTSTR_BLK(PT2PT, PART_LAT);
                break;
            case LAT_MP:
                OMBOP_OPTSTR_BLK(PT2PT, LAT_MP);
                break;
            case LAT_MT:
                OMBOP_OPTSTR_BLK(PT2PT, LAT_MT);
                break;
            case CONG_BW:
                OMBOP_OPTSTR_BLK(PT2PT, CONG_BW);
                break;
            default:
                OMB_ERROR_EXIT("Unknown subtype");
                break;
        }
    } else if (COLLECTIVE == options.bench) {
        switch (options.subtype) {
            case LAT:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, LAT);
                break;
            case GATHER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, GATHER);
                break;
            case ALL_GATHER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, ALL_GATHER);
                break;
            case SCATTER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, SCATTER);
                break;
            case ALLTOALL:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, ALLTOALL);
                break;
            case BCAST:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, BCAST);
                break;
            case BARRIER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, BARRIER);
                break;
            case REDUCE:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, REDUCE);
                break;
            case ALL_REDUCE:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, ALL_REDUCE);
                break;
            case REDUCE_SCATTER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, REDUCE_SCATTER);
                break;
            case GATHER_P:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, GATHER_P);
                break;
            case ALL_GATHER_P:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, ALL_GATHER_P);
                break;
            case SCATTER_P:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, SCATTER_P);
                break;
            case ALLTOALL_P:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, ALLTOALL_P);
                break;
            case BCAST_P:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, BCAST_P);
                break;
            case BARRIER_P:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, BARRIER_P);
                break;
            case REDUCE_P:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, REDUCE_P);
                break;
            case ALL_REDUCE_P:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, ALL_REDUCE_P);
                break;
            case REDUCE_SCATTER_P:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, REDUCE_SCATTER_P);
                break;
            case NBC_BARRIER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_BARRIER);
                break;
            case NBC_ALLTOALL:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_ALLTOALL);
                break;
            case NBC_GATHER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_GATHER);
                break;
            case NBC_ALL_GATHER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_ALL_GATHER);
                break;
            case NBC_REDUCE:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_REDUCE);
                break;
            case NBC_ALL_REDUCE:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_ALL_REDUCE);
                break;
            case NBC_SCATTER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_SCATTER);
                break;
            case NBC_BCAST:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_BCAST);
                break;
            case NBC_REDUCE_SCATTER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_REDUCE_SCATTER);
                break;
            case NHBR_ALLTOALL:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NHBR_ALLTOALL);
                break;
            case NHBR_GATHER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NHBR_GATHER);
                break;
            case NBC_NHBR_ALLTOALL:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_NHBR_ALLTOALL);
                break;
            case NBC_NHBR_GATHER:
                OMBOP_OPTSTR_CUDA_BLK(COLLECTIVE, NBC_NHBR_GATHER);
                break;
            default:
                OMB_ERROR_EXIT("Unknown subtype");
                break;
        }
    } else if (MBW_MR == options.bench) {
        options.optstring =
            (accel_enabled) ? OMBOP__ACCEL__MBW_MR : OMBOP__MBW_MR;
    } else if (OSHM == options.bench) {
        options.optstring = OMBOP__OSHM;
    } else if (UPC == options.bench) {
        options.optstring = OMBOP__UPC;
    } else if (UPCXX == options.bench) {
        options.optstring = OMBOP__UPCXX;
    } else if (STARTUP == options.bench && INIT == options.subtype) {
        options.optstring = OMBOP__STARTUP__INIT;
    } else if (options.bench == ONE_SIDED) {
        int jchar = 0;

        jchar = sprintf(&optstring_buf[jchar], "%s", "+:w:s:hvm:x:i:G:Iz::");
        if (options.subtype == BW) {
            jchar += sprintf(&optstring_buf[jchar], "%s", "W:");
        }
        if (accel_enabled) {
            jchar += sprintf(&optstring_buf[jchar], "%s", "d:");
        } else {
            jchar += sprintf(&optstring_buf[jchar], "%s", "P:");
        }
        if (options.show_validation) {
            jchar += sprintf(&optstring_buf[jchar], "%s", "cT:");
        }
        options.optstring = malloc(strlen(optstring_buf) * sizeof(char) + 1);
        OMB_CHECK_NULL_AND_EXIT(options.optstring,
                                "Unable to allocated memory");
        strcpy((char *)options.optstring, optstring_buf);
    } else {
        OMB_ERROR_EXIT("Unknown benchmark");
    }
    omb_process_long_options(long_options, options.optstring);
    /* Set default options*/
    options.accel = NONE;
    options.show_size = 1;
    options.show_full = 0;
    options.num_probes = 0;
    options.device_array_size = 32;
    options.target = CPU;
    options.min_message_size = MIN_MESSAGE_SIZE;
    options.message_size_incr = MESSAGE_SIZE_INCR;
    if (options.bench == COLLECTIVE) {
        options.max_message_size = MAX_MSG_SIZE_COLL;
    } else {
        options.max_message_size = MAX_MESSAGE_SIZE;
    }
    options.window_size_large = WINDOW_SIZE_LARGE;
    options.window_size = WINDOW_SIZE_LARGE;
    options.window_varied = 0;
    options.print_rate = 1;
    options.validate = 0;
    options.log_validation = 0;
    options.papi_enabled = 0;
    options.buf_num = SINGLE;
    options.omb_enable_ddt = 0;
    options.ddt_type_parameters.block_length = OMB_DDT_BLOCK_LENGTH_DEFAULT;
    options.ddt_type_parameters.stride = OMB_DDT_STRIDE_DEFAULT;
    options.graph = 0;
    options.graph_output_term = 0;
    options.graph_output_png = 0;
    options.graph_output_pdf = 0;
    options.omb_enable_ddt = 0;
    options.ddt_type_parameters.block_length = OMB_DDT_BLOCK_LENGTH_DEFAULT;
    options.ddt_type_parameters.stride = OMB_DDT_STRIDE_DEFAULT;
    options.nhbrhd_type = OMB_NHBRHD_TYPE_CART;
    options.nhbrhd_type_parameters.dim = 1;
    options.nhbrhd_type_parameters.rad = 1;
    options.src = 'H';
    options.dst = 'H';
    options.omb_dtype_itr = 0;
    options.omb_enable_session = 0;
    options.omb_enable_mpi_in_place = 0;
    options.omb_root_rank = 0;
    options.omb_tail_lat = 0;
    options.num_partitions = DEFAULT_NUM_PARTITIONS;

    for (itr = 0; itr < OMB_STAT_MAX_NUM; itr++) {
        options.omb_stat_percentiles[itr] = -1;
    }

    switch (options.subtype) {
        case BW:
        case CONG_BW:
            options.iterations = BW_LOOP_SMALL;
            options.skip = BW_SKIP_SMALL;
            options.iterations_large = BW_LOOP_LARGE;
            options.skip_large = BW_SKIP_LARGE;
            options.warmup_validation = VALIDATION_SKIP_DEFAULT;
            break;
        case LAT_MT:
            options.num_threads = DEF_NUM_THREADS;
            options.min_message_size = 1;
            options.sender_thread = -1;
        case LAT_MP:
            options.num_processes = DEF_NUM_PROCESSES;
            options.min_message_size = 1;
            options.sender_processes = DEF_NUM_PROCESSES;
        case PART_LAT:
        case LAT:
        case BARRIER:
        case GATHER:
        case ALL_GATHER:
        case ALLTOALL:
        case NBC_ALLTOALL:
        case NBC_GATHER:
        case NBC_ALL_GATHER:
        case NHBR_GATHER:
        case NHBR_ALLTOALL:
        case NBC_NHBR_GATHER:
        case NBC_NHBR_ALLTOALL:
        case NBC_REDUCE:
        case NBC_ALL_REDUCE:
        case NBC_SCATTER:
        case NBC_BCAST:
        case REDUCE:
        case ALL_REDUCE:
        case SCATTER:
        case BCAST:
        case REDUCE_SCATTER:
        case NBC_REDUCE_SCATTER:
        case NBC_BARRIER:
        case BARRIER_P:
        case GATHER_P:
        case ALL_GATHER_P:
        case ALLTOALL_P:
        case REDUCE_P:
        case ALL_REDUCE_P:
        case SCATTER_P:
        case BCAST_P:
        case REDUCE_SCATTER_P:
            if (options.bench == COLLECTIVE) {
                options.iterations = COLL_LOOP_SMALL;
                options.skip = COLL_SKIP_SMALL;
                options.iterations_large = COLL_LOOP_LARGE;
                options.skip_large = COLL_SKIP_LARGE;
            } else {
                options.iterations = LAT_LOOP_SMALL;
                options.skip = LAT_SKIP_SMALL;
                options.iterations_large = LAT_LOOP_LARGE;
                options.skip_large = LAT_SKIP_LARGE;
            }
            if (options.bench == PT2PT) {
                options.min_message_size = 1;
            }
            options.warmup_validation = VALIDATION_SKIP_DEFAULT;
            break;
        default:
            break;
    }

    switch (options.bench) {
        case UPCXX:
        case UPC:
            options.show_size = 0;
        case OSHM:
            options.iterations = OSHM_LOOP_SMALL;
            options.skip = OSHM_SKIP_SMALL;
            options.iterations_large = OSHM_LOOP_LARGE;
            options.skip_large = OSHM_SKIP_LARGE;
            options.max_message_size = 1 << 20;
            break;
        default:
            break;
    }

    int skiparg = 0, iterarg = 0, retval = 0;
    ;
    while ((c = getopt_long(argc, argv, options.optstring, long_options,
                            &option_index)) != -1) {
        bad_usage.opt = c;
        bad_usage.optarg = NULL;
        bad_usage.message = NULL;

        switch (c) {
            case 'h':
                return PO_HELP_MESSAGE;
            case 'v':
                return PO_VERSION_MESSAGE;
            case 'm':
                if (set_message_size(optarg)) {
                    bad_usage.message = "Invalid Message Size";
                    bad_usage.optarg = optarg;

                    return PO_BAD_USAGE;
                }
                break;
            case 't':
                if (options.bench == COLLECTIVE) {
                    if (set_num_probes(atoi(optarg))) {
                        bad_usage.message = "Invalid Number of Probes";
                        bad_usage.optarg = optarg;

                        return PO_BAD_USAGE;
                    }
                } else if (options.bench == PT2PT) {
                    if (options.subtype == LAT_MT) {
                        if (set_threads(optarg)) {
                            bad_usage.message = "Invalid Number of Threads";
                            bad_usage.optarg = optarg;

                            return PO_BAD_USAGE;
                        }
                    } else if (options.subtype == LAT_MP) {
                        if (set_processes(optarg)) {
                            bad_usage.message = "Invalid Number of Processes";
                            bad_usage.optarg = optarg;

                            return PO_BAD_USAGE;
                        }
                    }
                }
                break;
            case 'i':
                retval = process_iterations(optarg, 'i');
                if (retval) {
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                } else
                    iterarg = 1;
                break;
            case 'x':
                retval = process_iterations(optarg, 'x');
                if (retval) {
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                } else
                    skiparg = 1;
                break;
            case 'R':
                options.print_rate = atoi(optarg);
                if (0 != options.print_rate && 1 != options.print_rate) {
                    return PO_BAD_USAGE;
                }
                break;
            case 'W':
                if (set_window_size(atoi(optarg))) {
                    bad_usage.message = "Invalid Window Size";
                    bad_usage.optarg = optarg;

                    return PO_BAD_USAGE;
                }
                break;
            case 'V':
                options.window_varied = 1;
                break;
            case 'p':
                options.pairs = atoi(optarg);
                break;
            case 'a':
                if (set_device_array_size(atoi(optarg))) {
                    bad_usage.message = "Invalid Device Array Size";
                    bad_usage.optarg = optarg;

                    return PO_BAD_USAGE;
                }
                break;
            case 'f':
                options.show_full = 1;
                break;
            case 'z':
                options.omb_tail_lat = 1;
                if (NULL == optarg) {
                    options.omb_stat_percentiles[0] = 50;
                    options.omb_stat_percentiles[1] = 90;
                    options.omb_stat_percentiles[2] = 99;
                    break;
                }
                strtok_parsed = strtok(optarg, ",");
                itr = 0;
                while (NULL != strtok_parsed) {
                    options.omb_stat_percentiles[itr] = atoi(strtok_parsed);
                    if (options.omb_stat_percentiles[itr] < 0 ||
                        options.omb_stat_percentiles[itr] > 100) {
                        bad_usage.message =
                            "Percentile range must be in between 0 and 100";
                        bad_usage.optarg = optarg;

                        return PO_BAD_USAGE;
                    }
                    strtok_parsed = strtok(NULL, ",");
                    itr++;
                }
                break;
            case 'd':
                if (!accel_enabled) {
                    bad_usage.message = "Benchmark Does Not Support "
                                        "Accelerator Transfers";
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                } else if (0 == strncasecmp(optarg, "cuda", 10)) {
                    if (CUDA_ENABLED) {
                        options.accel = CUDA;
                    } else {
                        bad_usage.message =
                            "CUDA Support Not Enabled\n"
                            "Please recompile benchmark with CUDA support";
                        bad_usage.optarg = optarg;
                        return PO_BAD_USAGE;
                    }
                } else if (0 == strncasecmp(optarg, "managed", 10)) {
                    if (CUDA_ENABLED) {
                        options.accel = MANAGED;
                    } else {
                        bad_usage.message =
                            "CUDA Managed Memory Support Not Enabled\n"
                            "Please recompile benchmark with CUDA support";
                        bad_usage.optarg = optarg;
                        return PO_BAD_USAGE;
                    }
                } else if (0 == strncasecmp(optarg, "openacc", 10)) {
                    if (OPENACC_ENABLED) {
                        options.accel = OPENACC;
                    } else {
                        bad_usage.message =
                            "OpenACC Support Not Enabled\n"
                            "Please recompile benchmark with OpenACC support";
                        bad_usage.optarg = optarg;
                        return PO_BAD_USAGE;
                    }
                } else if (0 == strncasecmp(optarg, "rocm", 10)) {
                    if (ROCM_ENABLED) {
                        options.accel = ROCM;
                    } else {
                        bad_usage.message =
                            "ROCm Support Not Enabled\n"
                            "Please recompile benchmark with ROCm support";
                        bad_usage.optarg = optarg;
                        return PO_BAD_USAGE;
                    }
                } else if (0 == strncasecmp(optarg, "sycl", 10)) {
                    if (SYCL_ENABLED) {
                        options.accel = SYCL;
                    } else {
                        bad_usage.message =
                            "SYCL Support Not Enabled\n"
                            "Please recompile benchmark with SYCL support";
                        bad_usage.optarg = optarg;
                        return PO_BAD_USAGE;
                    }
                } else {
                    bad_usage.message = "Invalid Accel Type Specified";
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                }
                break;
            case 'r':
                if (CUDA_KERNEL_ENABLED) {
                    if (0 == strncasecmp(optarg, "cpu", 10)) {
                        options.target = CPU;
                    } else if (0 == strncasecmp(optarg, "gpu", 10)) {
                        options.target = GPU;
                    } else if (0 == strncasecmp(optarg, "both", 10)) {
                        options.target = BOTH;
                    } else {
                        bad_usage.message = "Please use cpu, gpu, or both";
                        bad_usage.optarg = optarg;
                        return PO_BAD_USAGE;
                    }
                } else {
                    bad_usage.message =
                        "CUDA Kernel Support Not Enabled\n"
                        "Please recompile benchmark with CUDA Kernel support";
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                }
                break;
            case 'w':
                ret = process_one_sided_options(c, optarg);
                if (ret == PO_BAD_USAGE) {
                    bad_usage.message = "Invalid option or invalid argument";
                    bad_usage.opt = c;
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                }
                break;
            case 's':
                ret = process_one_sided_options(c, optarg);
                if (ret == PO_BAD_USAGE) {
                    bad_usage.message = "Invalid option or invalid argument";
                    bad_usage.opt = c;
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                }
                break;
            case 'c':
                options.validate = 1;
                if (options.omb_enable_ddt) {
                    bad_usage.message = "Derived data type does not support"
                                        " validation";
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                }
                if (NULL != optarg) {
                    validation_log_option = strtok(optarg, ":");
                    if (0 == strcmp("log", validation_log_option)) {
                        options.log_validation = 1;
                        validation_log_option = strtok(NULL, ":");
                        if (NULL != validation_log_option) {
                            if (OMB_FILE_PATH_MAX_LENGTH <
                                strlen(validation_log_option)) {
                                fprintf(stderr,
                                        "ERROR: Max allowed size for filepath "
                                        "is:%d\n"
                                        "To increase the max allowed filepath "
                                        "limit, update"
                                        " OMB_FILE_PATH_MAX_LENGTH in "
                                        "c/util/osu_util.h.\n",
                                        OMB_FILE_PATH_MAX_LENGTH);
                                fflush(stderr);
                                bad_usage.message =
                                    "Filepath exceeds maximum length"
                                    " allowed";
                                bad_usage.optarg = optarg;
                                return PO_BAD_USAGE;
                            }
                            strcpy(options.log_validation_dir_path,
                                   validation_log_option);
                        } else {
                            strcpy(options.log_validation_dir_path,
                                   OMB_VALIDATION_LOG_DIR_PATH);
                        }
                        if (0 ==
                            access(options.log_validation_dir_path, F_OK)) {
                            fprintf(
                                stderr,
                                "ERROR: \"%s\" already exists. Set validation "
                                "log directory using \"-clog:<dir>\n",
                                options.log_validation_dir_path);
                            fflush(stderr);
                            bad_usage.message = "Directory already exists.";
                            bad_usage.optarg = optarg;
                            return PO_BAD_USAGE;
                        }
                    }
                }
                break;
            case 'P':
#ifdef _ENABLE_PAPI_
                options.papi_enabled = 1;
                omb_papi_parse_event_options(optarg);
#else
                bad_usage.message = "Invalid option. Please reconfigure with"
                                    " PAPI.";
                bad_usage.opt = optopt;
                return PO_BAD_USAGE;
#endif
                break;
            case 'u':
                if (set_num_warmup_validation(atoi(optarg))) {
                    bad_usage.message = "Invalid Number of Validation Warmup "
                                        " Iterations";
                    bad_usage.optarg = optarg;

                    return PO_BAD_USAGE;
                }
                if (options.warmup_validation > VALIDATION_SKIP_MAX) {
                    bad_usage.message = "Number of Validation Warmup Iterations"
                                        "must be less than 10";
                    bad_usage.optarg = optarg;

                    return PO_BAD_USAGE;
                }
                break;
            case 'b':
                if (0 == strncasecmp(optarg, "single", 10)) {
                    options.buf_num = SINGLE;
                } else if (0 == strncasecmp(optarg, "multiple", 10)) {
                    options.buf_num = MULTIPLE;
                } else {
                    bad_usage.message =
                        "Please use 'single' or 'multiple' for buffer type";
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                }
                break;
            case 'G':
                options.graph = 1;
                graph_term_type = strtok(optarg, ",");
                if (NULL == graph_term_type) {
                    bad_usage.message = "Please pass graph"
                                        " types[tty,png,pdf]\n";
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                }
                while (NULL != graph_term_type) {
                    if (0 == strncasecmp(graph_term_type, "png", 3)) {
                        options.graph_output_png = 1;
                    } else if (0 == strncasecmp(graph_term_type, "tty", 3)) {
                        options.graph_output_term = 1;
                    } else if (0 == strncasecmp(graph_term_type, "pdf", 3)) {
                        options.graph_output_pdf = 1;
                    } else {
                        bad_usage.message = "Invalid graph type. Valid graph"
                                            " types[tty,png,pdf]\n";
                        bad_usage.optarg = optarg;
                        return PO_BAD_USAGE;
                    }
                    graph_term_type = strtok(NULL, ",");
                }
                break;
            case 'D':
                options.omb_enable_ddt = 1;
                if (options.validate) {
                    bad_usage.message = "Derived data type does not support"
                                        " validation";
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                }
                ret = PO_OKAY;
                ret = omb_ddt_process_options(optarg, &bad_usage);
                if (ret == PO_BAD_USAGE) {
                    return ret;
                }
                break;
            case 'N':
                ret = PO_OKAY;
                ret = omb_nhbrhd_process_options(optarg, &bad_usage);
                if (ret == PO_BAD_USAGE) {
                    return ret;
                }
                break;
            case 'T':
                if (0 == strncasecmp(optarg, "MPI_CHAR", 8)) {
                    options.omb_dtype_list[options.omb_dtype_itr++] = OMB_CHAR;
                } else if (0 == strncasecmp(optarg, "MPI_INT", 7)) {
                    options.omb_dtype_list[options.omb_dtype_itr++] = OMB_INT;
                } else if (0 == strncasecmp(optarg, "MPI_FLOAT", 9)) {
                    options.omb_dtype_list[options.omb_dtype_itr++] = OMB_FLOAT;
                } else if (0 == strncasecmp(optarg, "MPI_DOUBLE", 10)) {
                    options.omb_dtype_list[options.omb_dtype_itr++] =
                        OMB_DOUBLE;
                } else if (0 == strncasecmp(optarg, "ALL", 3)) {
                    options.omb_dtype_list[options.omb_dtype_itr++] = OMB_CHAR;
                    options.omb_dtype_list[options.omb_dtype_itr++] = OMB_INT;
                    options.omb_dtype_list[options.omb_dtype_itr++] = OMB_FLOAT;
                    options.omb_dtype_list[options.omb_dtype_itr++] =
                        OMB_DOUBLE;
                }
                break;
            case 'I':
#ifdef _ENABLE_MPI4_
            {
                options.omb_enable_session = 1;
            }
#else
            {
                bad_usage.message = "Pass \"--enable-mpi4\" while configuring "
                                    "to enable MPI-4 support.";
                bad_usage.opt = optopt;
                return PO_BAD_USAGE;
            }
#endif
            break;
            case 'q':
                if (set_num_partitions(atoi(optarg))) {
                    bad_usage.message = "Invalid Number of partitions";
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                }
                break;
            case 'l':
                options.omb_enable_mpi_in_place = 1;
                break;
            case 'k':
                root_rank_type = strtok(optarg, ":");
                if (NULL == root_rank_type) {
                    bad_usage.message = "Please pass root rank rotate"
                                        " types[fixed, rotate]\n";
                    bad_usage.optarg = optarg;
                    return PO_BAD_USAGE;
                }
                if (NULL != root_rank_type) {
                    if (0 == strncasecmp(root_rank_type, "rotate", 6)) {
                        options.omb_root_rank = OMB_ROOT_ROTATE_VAL;
                    } else if (0 == strncasecmp(root_rank_type, "fixed", 5)) {
                        root_rank_type = strtok(NULL, ":");
                        if (NULL == root_rank_type) {
                            bad_usage.message = "Please pass root node rank."
                                                " E.g: -k fixed:1\n";
                            bad_usage.optarg = optarg;
                            return PO_BAD_USAGE;
                        }
                        options.omb_root_rank = atoi(root_rank_type);
                    } else {
                        bad_usage.message = "Invalid root rank rotate type."
                                            " Valid types[fixed, rotate]\n";
                        bad_usage.optarg = optarg;
                        return PO_BAD_USAGE;
                    }
                }
                break;
            case ':':
                bad_usage.message = "Option Missing Required Argument";
                bad_usage.opt = optopt;
                return PO_BAD_USAGE;
            default:
                bad_usage.message = "Invalid option";
                bad_usage.opt = optopt;
                return PO_BAD_USAGE;
        }
    }
    /*
     * provide the legacy values if the user didn't specify new ones
     * These are primarily used in the tests which have been modified
     * to take more message ranges, e.g. MPI collectives
     */
    if (0 == skiparg) {
        options.skip_sizes[0] = 0;
        options.skip_sizes[1] = LARGE_MESSAGE_SIZE;
        options.skip_counts[0] = options.skip;
        options.skip_counts[1] = options.skip_large;
        options.num_skip_sizes = 2;
    }
    if (0 == iterarg) {
        options.num_iteration_sizes = 2;
        options.iteration_sizes[0] = 0;
        options.iteration_sizes[1] = LARGE_MESSAGE_SIZE;
        options.iteration_counts[0] = options.iterations;
        options.iteration_counts[1] = options.iterations_large;
    }

    if (!options.validate) {
        options.warmup_validation = 0;
    }

    if (0 == options.omb_dtype_itr) {
        if (REDUCE == options.subtype || REDUCE_SCATTER == options.subtype ||
            ALL_REDUCE == options.subtype ||
            NBC_ALL_REDUCE == options.subtype ||
            NBC_REDUCE == options.subtype ||
            NBC_REDUCE_SCATTER == options.subtype ||
            ALL_REDUCE_P == options.subtype || REDUCE_P == options.subtype ||
            REDUCE_SCATTER_P == options.subtype) {
            options.omb_dtype_list[options.omb_dtype_itr] = OMB_INT;
        } else {
            options.omb_dtype_list[options.omb_dtype_itr] = OMB_CHAR;
        }
        options.omb_dtype_itr++;
    }

    if (accel_enabled) {
        if ((optind + 2) == argc) {
            options.src = argv[optind][0];
            if (options.src == 'M') {
#ifdef _ENABLE_CUDA_KERNEL_
                options.MMsrc = argv[optind][1];
                if (options.MMsrc == '\0' && ONE_SIDED != options.bench) {
                    fprintf(stderr,
                            "The M flag for destination buffer is "
                            "deprecated. Please use MD or MH to set the "
                            "effective location of CUDA Unified Memory buffers "
                            "to be device or host respectively. Currently M "
                            "flag is considered as MH\n");
                    options.MMsrc = 'H';
                } else if (options.MMsrc != 'D' && options.MMsrc != 'H' &&
                           ONE_SIDED != options.bench) {
                    fprintf(stderr,
                            "Please use MD or MH to set the effective "
                            "location of CUDA Unified Memory buffers to be "
                            "device or host respectively\n");
                    return PO_BAD_USAGE;
                }
#else
                fprintf(stderr,
                        "Managed memory support requires CUDA kernels.\n");
                return PO_BAD_USAGE;
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
            }
            options.dst = argv[optind + 1][0];
            if (options.dst == 'M') {
#ifdef _ENABLE_CUDA_KERNEL_
                options.MMdst = argv[optind + 1][1];
                if (options.MMdst == '\0' && ONE_SIDED != options.bench) {
                    fprintf(stderr,
                            "The M flag for destination buffer is "
                            "deprecated. Please use MD or MH to set the "
                            "effective location of CUDA Unified Memory buffers "
                            "to be device or host respectively. Currently M "
                            "flag is considered as MH\n");
                    options.MMdst = 'H';
                } else if (options.MMdst != 'D' && options.MMdst != 'H' &&
                           ONE_SIDED != options.bench) {
                    fprintf(stderr,
                            "Please use MD or MH to set the effective "
                            "location of CUDA Unified Memory buffers to be "
                            "device or host respectively\n");
                    return PO_BAD_USAGE;
                }
#else
                fprintf(stderr,
                        "Managed memory support requires CUDA kernels.\n");
                return PO_BAD_USAGE;
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
            }
            /* No need to check if '-d' is given */
            if (NONE == options.accel || options.bench == PT2PT) {
                setAccel(options.src);
                setAccel(options.dst);
            }
        } else if (optind != argc) {
            return PO_BAD_USAGE;
        }
    }

    return PO_OKAY;
}

int omb_ddt_process_options(char *optarg, struct bad_usage_t *bad_usage)
{
    char *option = NULL;
    if (NULL == optarg) {
        bad_usage->message = "Please pass a ddt"
                             " type[cont,vect,indx]\n";
        bad_usage->optarg = optarg;
        return PO_BAD_USAGE;
    }
    option = strtok(optarg, ":");
    if (0 == strncasecmp(optarg, "vect", 4)) {
        options.ddt_type = OMB_DDT_VECTOR;
        option = strtok(NULL, ":");
        OMB_CHECK_NULL_AND_EXIT(
            option, "DDT of type 'vect' takes 2 args. Please see --help");
        options.ddt_type_parameters.stride = atoi(option);
        OMB_CHECK_NULL_AND_EXIT(
            option, "DDT of type 'vect' takes 2 args. Please see --help");
        option = strtok(NULL, ":");
        options.ddt_type_parameters.block_length = atoi(option);
    } else if (0 == strncasecmp(optarg, "indx", 4)) {
        options.ddt_type = OMB_DDT_INDEXED;
        option = strtok(NULL, ":");
        OMB_CHECK_NULL_AND_EXIT(
            option, "DDT of type 'indx' takes 2 args. Please see --help");
        if (OMB_DDT_FILE_PATH_MAX_LENGTH < strlen(option)) {
            fprintf(stderr,
                    "ERROR: Max allowed size for filepath is:%d\n"
                    "To increase the max allowed filepath limit, update"
                    " OMB_DDT_FILE_PATH_MAX_LENGTH in c/util/osu_util.h.\n",
                    OMB_DDT_FILE_PATH_MAX_LENGTH);
            fflush(stderr);
            bad_usage->message = "Index DDT filepath exceeds maximum length"
                                 " allowed";
            bad_usage->optarg = optarg;
            return PO_BAD_USAGE;
        }
        strcpy(options.ddt_type_parameters.filepath, option);
    } else if (0 == strncasecmp(optarg, "cont", 4)) {
        options.ddt_type = OMB_DDT_CONTIGUOUS;
    } else {
        bad_usage->message = "Invalid ddt type. Valid ddt"
                             " types[cont,vect,indx]\n";
        bad_usage->optarg = optarg;
        return PO_BAD_USAGE;
    }
    return PO_OKAY;
}

int omb_nhbrhd_process_options(char *optarg, struct bad_usage_t *bad_usage)
{
    char *option = NULL;
    if (NULL == optarg) {
        bad_usage->message = "Please pass a topology type[cart,graph]\n";
        bad_usage->optarg = optarg;
        return PO_BAD_USAGE;
    }
    option = strtok(optarg, ":");
    if (0 == strncasecmp(optarg, "cart", 4)) {
        options.nhbrhd_type = OMB_NHBRHD_TYPE_CART;
        option = strtok(NULL, ":");
        if (NULL == option) {
            bad_usage->message = "Please pass -Ncart:<dim:rad>";
            bad_usage->opt = optopt;
            return PO_BAD_USAGE;
        }
        options.nhbrhd_type_parameters.dim = atoi(option);
        option = strtok(NULL, ":");
        if (NULL == option) {
            bad_usage->message = "Please pass -Ncart:<dim:rad>";
            bad_usage->opt = optopt;
            return PO_BAD_USAGE;
        }
        options.nhbrhd_type_parameters.rad = atoi(option);
    } else if (0 == strncasecmp(optarg, "graph", 5)) {
        options.nhbrhd_type = OMB_NHBRHD_TYPE_GRAPH;
        option = strtok(NULL, ":");
        if (NULL != option) {
            if (OMB_NHBRHD_FILE_PATH_MAX_LENGTH < strlen(option)) {
                fprintf(
                    stderr,
                    "ERROR: Max allowed size for filepath is:%d\n"
                    "To increase the max allowed filepath limit, update"
                    " OMB_NHBRHD_FILE_PATH_MAX_LENGTH in c/util/osu_util.h.\n",
                    OMB_NHBRHD_FILE_PATH_MAX_LENGTH);
                fflush(stderr);
                bad_usage->message = "Filepath exceeds maximum length"
                                     " allowed";
                bad_usage->optarg = optarg;
                return PO_BAD_USAGE;
            }
            strcpy(options.nhbrhd_type_parameters.filepath, option);
        }
    } else {
        bad_usage->message = "Invalid topology type. Valid topology"
                             " types[cart,graph]\n";
        bad_usage->optarg = optarg;
        return PO_BAD_USAGE;
    }
    return PO_OKAY;
}

/* Set the initial accelerator type */
int setAccel(char buf_type)
{
    switch (buf_type) {
        case 'H':
            break;
        case 'M':
            /* For managed memory benchmarks, use multiple buffers to report
             * accurate performance numbers */
            options.buf_num = MULTIPLE;
        case 'D':
            if (options.bench != PT2PT && options.bench != ONE_SIDED &&
                options.bench != MBW_MR) {
                bad_usage.opt = buf_type;
                bad_usage.message = "This argument is only supported for "
                                    "one-sided and pt2pt benchmarks";
                return PO_BAD_USAGE;
            }
            if (NONE == options.accel || MANAGED == options.accel) {
#if defined(_ENABLE_OPENACC_) && !defined(_ENABLE_CUDA_)
                options.accel = OPENACC;
#elif defined(_ENABLE_CUDA_)
                options.accel = CUDA;
#elif defined(_ENABLE_ROCM_)
                options.accel = ROCM;
#elif defined(_ENABLE_SYCL_)
                options.accel = SYCL;
#endif
            }
            break;
        default:
            return PO_BAD_USAGE;
    }
    return PO_OKAY;
}

double getMicrosecondTimeStamp()
{
    double retval;
    struct timeval tv;
    if (gettimeofday(&tv, NULL)) {
        perror("gettimeofday");
        abort();
    }
    retval = ((double)tv.tv_sec) * 1000000 + tv.tv_usec;
    return retval;
}

void wtime(double *t)
{
    static int sec = -1;
    struct timeval tv;
    /* gettimeofday(&tv, (void *)0); */
    gettimeofday(&tv, 0);
    if (sec < 0)
        sec = tv.tv_sec;
    *t = (tv.tv_sec - sec) * 1.0e+6 + tv.tv_usec;
}

/* vi:set sw=4 sts=4 tw=80: */
