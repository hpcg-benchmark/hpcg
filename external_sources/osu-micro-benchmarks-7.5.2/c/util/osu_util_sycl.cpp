#include "osu_util_sycl.hpp"

#define SYCL_CHECK(stmt)                                                       \
    do {                                                                       \
        try {                                                                  \
            (stmt);                                                            \
        } catch (sycl::exception & e) {                                        \
            std::cout << "Caught sync SYCL exception: " << e.what() << "\n";   \
            assert(0);                                                         \
        }                                                                      \
    } while (0)

sycl::queue q;

void syclMemset(void *buf, int data, size_t size)
{
    SYCL_CHECK(q.memset(buf, data, size));
    SYCL_CHECK(q.wait_and_throw());
}

void syclFree(void *buf) { SYCL_CHECK(sycl::free(buf, q)); }

void syclMemcpy(void *dst, void *src, int size)
{
    SYCL_CHECK(q.memcpy(dst, src, size).wait());
}

void syclMalloc(void **buf, int size)
{
    SYCL_CHECK(*buf = (void *)malloc_device(size, q));
}

void syclDeviceSynchronize() { SYCL_CHECK(q.wait_and_throw()); }

void syclGetDeviceCount(int *dev_count)
{
    SYCL_CHECK(*dev_count = dpct::dev_mgr::instance().device_count());
}

void syclSetDevice(int dev_id)
{
    SYCL_CHECK(dpct::dev_mgr::instance().select_device(dev_id));
}

void syclDeviceReset() { SYCL_CHECK(dpct::get_current_device().reset()); }

void syclPrintDeviceName(int local_rank)
{
    sycl::device d;
    std::string deviceName;
    SYCL_CHECK(d = q.get_device());
    SYCL_CHECK(deviceName = d.get_info<sycl::info::device::name>());
    const char *device_name = deviceName.c_str();

    fprintf(stderr, "Rank %d: %s\n", local_rank, device_name);
}
