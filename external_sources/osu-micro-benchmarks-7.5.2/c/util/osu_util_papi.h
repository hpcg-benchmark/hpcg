/*
 *Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 *(NBCL), The Ohio State University.
 *
 *Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 *For detailed copyright and licensing information, please refer to the
 *copyright file COPYRIGHT in the top level OMB directory.
 */

/*
 * PAPI uses -1 as a nonexistent hardware event placeholder
 * https://icl.utk.edu/papi/docs/df/d34/group__consts.html
 */
#ifdef _ENABLE_PAPI_
#define OMB_PAPI_NULL PAPI_NULL
#else
#define OMB_PAPI_NULL -1
#endif

#define OMB_PAPI_FILE_PATH_MAX_LENGTH OMB_FILE_PATH_MAX_LENGTH
#define OMB_PAPI_NUMBER_OF_EVENTS     100

void omb_papi_init(int *papi_eventset);
void omb_papi_start(int *papi_eventset);
void omb_papi_stop_and_print(int *papi_eventset, int size);
void omb_papi_free(int *papi_eventset);
void omb_papi_parse_event_options(char *opt_arr);
