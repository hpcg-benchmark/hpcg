#ifdef __cplusplus
#include "sycl.hpp"
#include "dpct.hpp"

extern "C" {
#endif
    void syclMemset(void *buf, int data, size_t size);
    void syclFree(void *buf);
    void syclMemcpy(void *dst, void *src, int size);
    void syclMalloc(void **buf, int size);
    void syclDeviceSynchronize();
    void syclGetDeviceCount(int *dev_count);
    void syclSetDevice(int dev_id);
    void syclDeviceReset();
    void syclPrintDeviceName(int local_rank);
#ifdef __cplusplus
}
#endif

