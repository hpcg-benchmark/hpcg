/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include <mpi.h>
#include "osu_util.h"
#include "osu_util_graph.h"
#include "osu_util_papi.h"

#define MPI_CHECK(stmt)                                                        \
    do {                                                                       \
        int mpi_errno = (stmt);                                                \
        if (MPI_SUCCESS != mpi_errno) {                                        \
            fprintf(stderr, "[%s:%d] MPI call failed with %d \n", __FILE__,    \
                    __LINE__, mpi_errno);                                      \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        assert(MPI_SUCCESS == mpi_errno);                                      \
    } while (0)
#define OMB_MPI_RUN_AT_RANK_ZERO(stmt)                                         \
    if (0 == rank) {                                                           \
        stmt;                                                                  \
    }

#define OMB_ITR_PRINT_STAT(_stat_arr)                                          \
    {                                                                          \
        int _itr = 0;                                                          \
        while (_itr < OMB_STAT_MAX_NUM &&                                      \
               -1 != options.omb_stat_percentiles[_itr]) {                     \
            fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION,             \
                    _stat_arr[_itr]);                                          \
            _itr++;                                                            \
        }                                                                      \
    }

extern MPI_Aint disp_remote;
extern MPI_Aint disp_local;

/*
 * Non-blocking Collectives
 */
double call_test(int *num_tests, MPI_Request **request);
void allocate_device_arrays(int n);
double dummy_compute(double target_secs, MPI_Request *request);
void init_arrays(double seconds);
double do_compute_and_probe(double seconds, MPI_Request *request);

#ifdef _ENABLE_CUDA_KERNEL_
extern void call_kernel(float a, float *d_x, float *d_y, int N,
                        cudaStream_t *stream);
void free_device_arrays();
#endif

/*
 * Managed Memory
 */
#ifdef _ENABLE_CUDA_KERNEL_
#define PREFETCH_THRESHOLD 131072
enum op_type { ADD, SUB };
void touch_managed(char *buf, size_t length, enum op_type type);
double measure_kernel_lo_window(char **, int, int);
double measure_kernel_lo_no_window(char *, int);
void touch_managed_src_window(char **, int, int, enum op_type);
void touch_managed_dst_window(char **, int, int, enum op_type);
void touch_managed_src_no_window(char *, int, enum op_type);
void touch_managed_dst_no_window(char *, int, enum op_type);
void launch_empty_kernel(char *buf, size_t length);
void create_cuda_stream();
void destroy_cuda_stream();
void synchronize_device();
void synchronize_stream();
void prefetch_data(char *buf, size_t length, int devid);
void create_cuda_event();
void destroy_cuda_event();
void event_record_start();
void event_record_stop();
void event_elapsed_time(float *);
extern void call_touch_managed_kernel_add(char *buf, size_t length,
                                          cudaStream_t *stream);
extern void call_touch_managed_kernel_sub(char *buf, size_t length,
                                          cudaStream_t *stream);
extern void call_empty_kernel(char *buf, size_t length, cudaStream_t *stream);
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

/*
 * Print Information
 */
typedef struct omb_stat_t {
    double res_arr[OMB_STAT_MAX_NUM];
} omb_stat_t;

void print_bad_usage_message(int rank);
void print_help_message(int rank);
void print_help_message_common();
void print_version_message(int rank);
void print_preamble(int rank);
void print_only_header(int rank);
void print_preamble_nbc(int rank);
void print_only_header_nbc(int rank);
void print_stats(int rank, int size, double avg, double min, double max,
                 struct omb_stat_t omb_stats);
void print_stats_validate(int rank, int size, double avg, double min,
                          double max, int errors, struct omb_stat_t omb_stats);
void print_stats_nbc(int rank, int size, double ovrl, double cpu,
                     double avg_comm, double min_comm, double max_comm,
                     double wait, double init, double test, int errors,
                     struct omb_stat_t omb_stats);
int omb_ascending_cmp_double(const void *a, const void *b);
struct omb_stat_t omb_get_stats(double *lat_arr);
struct omb_stat_t omb_calculate_tail_lat(double *avg_lat_arr, int rank,
                                         int comm_size);
double calculate_and_print_stats(int rank, int size, int numprocs, double timer,
                                 double latency, double test_time,
                                 double cpu_time, double wait_time,
                                 double init_time, int errors,
                                 struct omb_stat_t omb_stat);

/*
 * Memory Management
 */
struct omb_buffer_sizes_t {
    size_t sendbuf_size;
    size_t recvbuf_size;
};
int allocate_memory_coll(void **buffer, size_t size, enum accel_type type);
void free_buffer(void *buffer, enum accel_type type);
void set_buffer(void *buffer, enum accel_type type, int data, size_t size);
void set_buffer_pt2pt(void *buffer, int rank, enum accel_type type, int data,
                      size_t size);
void set_buffer_validation(void *s_buf, void *r_buf, size_t size,
                           enum accel_type type, int iter, MPI_Datatype dtype,
                           struct omb_buffer_sizes_t omb_buffer_sizes);
void set_buffer_float(float *buffer, int is_send_buf, size_t size, int iter,
                      enum accel_type type);
void set_buffer_dtype(void *buffer, int is_send_buf, size_t size, int rank,
                      int num_procs, enum accel_type type, int iter,
                      MPI_Datatype dtype, size_t bufsize);
void set_buffer_dtype_reduce(void *buffer, int is_send_buf, size_t size,
                             int iter, enum accel_type type,
                             MPI_Datatype dtype);

/*
 * CUDA Context Management
 */
int init_accel(void);
int cleanup_accel(void);

extern MPI_Request request[MAX_REQ_NUM];
extern MPI_Status reqstat[MAX_REQ_NUM];
extern MPI_Request send_request[MAX_REQ_NUM];
extern MPI_Request recv_request[MAX_REQ_NUM];

void usage_mbw_mr();
int allocate_memory_pt2pt(char **sbuf, char **rbuf, int rank);
int allocate_memory_pt2pt_size(char **sbuf, char **rbuf, int rank, size_t size);
int allocate_memory_pt2pt_mul(char **sbuf, char **rbuf, int rank, int pairs);
int allocate_memory_pt2pt_mul_size(char **sbuf, char **rbuf, int rank,
                                   int pairs, size_t size);
void print_header_pt2pt(int rank, int type);
void free_memory(void *sbuf, void *rbuf, int rank);
void free_memory_pt2pt_mul(void *sbuf, void *rbuf, int rank, int pairs);
void free_memory_pt2pt_mul(void *sbuf, void *rbuf, int rank, int pairs);
void print_header(int rank, int full);
void usage_one_sided(char const *);
void print_header_one_sided(int, enum WINDOW, enum SYNC, MPI_Datatype dtype);

void print_help_message_get_acc_lat(int);

extern char const *benchmark_header;
extern char const *benchmark_name;
extern int accel_enabled;
extern struct options_t options;
extern struct bad_usage_t bad_usage;

void allocate_memory_one_sided(int rank, char **sbuf, char **win_base,
                               size_t size, enum WINDOW type, MPI_Win *win);
void free_memory_one_sided(void *user_buf, void *win_baseptr,
                           enum WINDOW win_type, MPI_Win win, int rank);
void allocate_atomic_memory(int rank, char **sbuf, char **tbuf, char **cbuf,
                            char **win_base, size_t size, enum WINDOW type,
                            MPI_Win *win);
void free_atomic_memory(void *sbuf, void *win_baseptr, void *tbuf, void *cbuf,
                        enum WINDOW type, MPI_Win win, int rank);
int omb_get_local_rank();

/*
 * Data Validation
 */
#define VALIDATION_STATUS(error) (error > 0) ? "Fail" : "Pass"
#define ERROR_DELTA              0.001
uint8_t validate_data(void *r_buf, size_t size, int num_procs,
                      enum accel_type type, int iter, MPI_Datatype dtype);
int validate_reduction(void *buffer, size_t size, int iter, int num_procs,
                       enum accel_type type, MPI_Datatype dtype);
int validate_collective(void *buffer, size_t size, int value1, int value2,
                        enum accel_type type, int itr, MPI_Datatype dtype);
int validate_reduce_scatter(void *buffer, size_t size, int *recvcounts,
                            int rank, int num_procs, enum accel_type type,
                            int iter, MPI_Datatype dtype);
void validation_log(void *buffer, void *expected_buffer, size_t size,
                    size_t num_elements, MPI_Datatype dtype, int itr);
int omb_validate_neighborhood_col(MPI_Comm comm, char *buffer, int indegree,
                                  int outdegree, size_t size,
                                  enum accel_type type, int iter,
                                  MPI_Datatype dtype);
void set_buffer_nhbr_validation(void *s_buf, void *r_buf, int indegree,
                                int *sources, int outdegree, int *destinations,
                                size_t size, enum accel_type type, int iter,
                                MPI_Datatype dtype);

/*
 * DDT
 */
#define OMB_DDT_INDEXED_MAX_LENGTH   100
#define OMB_DDT_FILE_LINE_MAX_LENGTH 500
size_t omb_ddt_assign(MPI_Datatype *datatype, MPI_Datatype base_datatype,
                      size_t count);
void omb_ddt_free(MPI_Datatype *datatype);
size_t omb_ddt_get_size(size_t size);
void omb_ddt_append_stats(size_t omb_ddt_transmit_size);

/*
 * Neighborhood Collectives
 */
#define OMB_NHBRHD_FILE_LINE_MAX_LENGTH 500
#define OMB_NHBRHD_ADJ_EDGES_MAX_NUM    500
int omb_neighborhood_create(MPI_Comm comm, int **indegree_ptr,
                            int **sources_ptr, int **sourceweights_ptr,
                            int **outdegree_ptr, int **destinations_ptr,
                            int **destweights_ptr);

#define ATOM_CTYPE_FOR_DMPI_CHAR   char
#define ATOM_CTYPE_FOR_DMPI_INT    int
#define ATOM_CTYPE_FOR_DMPI_FLOAT  float
#define ATOM_CTYPE_FOR_DMPI_DOUBLE double

#define ENUM_OF_DMPI_CHAR   1
#define ENUM_OF_DMPI_INT    2
#define ENUM_OF_DMPI_FLOAT  3
#define ENUM_OF_DMPI_DOUBLE 4
int omb_get_num_elements(size_t size, MPI_Datatype dtype);
void omb_assign_to_type(void *buf, int pos, int val, MPI_Datatype dtype);
int atomic_data_validation_setup(MPI_Datatype datatype, int jrank, void *buf,
                                 size_t buf_size);
int atomic_data_validation_check(MPI_Datatype datatype, MPI_Op op, int jrank,
                                 void *addr, void *res, size_t buf_size,
                                 _Bool check_addr, _Bool check_result,
                                 int *validation_error_flag);
int atomic_data_validation_print_summary();
/*
 * Data Types
 */
void omb_populate_mpi_type_list(MPI_Datatype *mpi_type_list);

/*
 * Per MPI forum documentation "Signed Characters and Reductions":
 *  The types MPI_SIGNED_CHAR and MPI_UNSIGNED_CHAR can be
 *  used in reduction operations. MPI_CHAR, MPI_WCHAR, and
 *  MPI_CHARACTER (which represent printable characters) cannot be used
 *  in reduction operations. In a heterogeneous environment, MPI_CHAR,
 *  MPI_WCHAR, and MPI_CHARACTER will be translated so as to preserve
 *  the printable character, whereas MPI_SIGNED_CHAR and
 *  MPI_UNSIGNED_CHAR will be translated so as to preserve the integer
 *  value.
 */
#define OMB_MPI_REDUCE_CHAR_CHECK(dt)                                          \
    if (MPI_CHAR == dt) {                                                      \
        dt = MPI_SIGNED_CHAR;                                                  \
    }

extern MPI_T_cvar_handle dynamic_tuning_cvar;
#define OMB_PAUSE_DYNAMIC_TUNING                                               \
    do {                                                                       \
        if (dynamic_tuning_cvar) {                                             \
            int _off = 0;                                                      \
            MPI_T_cvar_write(dynamic_tuning_cvar, &_off);                      \
        }                                                                      \
    } while (0)

#define OMB_START_DYNAMIC_TUNING                                               \
    do {                                                                       \
        if (dynamic_tuning_cvar) {                                             \
            int _on = 1;                                                       \
            MPI_T_cvar_write(dynamic_tuning_cvar, &_on);                       \
        }                                                                      \
    } while (0)

/*
 * Session
 */
#define OMB_MPI_SESSION_PSET_NAME  "mpi://WORLD"
#define OMB_MPI_SESSION_GROUP_NAME "omb"
typedef struct omb_mpi_init_data_t {
#ifdef _ENABLE_MPI4_
    MPI_Session omb_shandle;
#endif
    MPI_Comm omb_comm;
} omb_mpi_init_data;
void omb_mpi_finalize(omb_mpi_init_data omb_init_h);
omb_mpi_init_data omb_mpi_init(int *argc, char ***argv);

int omb_get_root_rank(int itr, size_t comm_size);
void omb_scatter_offset_copy(void *buf, int root_rank, size_t size);
