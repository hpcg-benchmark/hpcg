/*
 * Copyright (c) 2002-2021 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

__global__ void compute_kernel(float a, float *x, float *y, int N)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    int count = 0;

    if (i < N) {
        for (count = 0; count < (N / 8); count++) {
            y[i] = a * x[i] + y[i];
        }
    }
}

__global__ void touch_managed_kernel_add(char *buf, size_t len)
{
    int i;

    i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < len) {
        buf[i] = buf[i] + 1;
    }
}

__global__ void touch_managed_kernel_sub(char *buf, size_t len)
{
    int i;

    i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < len) {
        buf[i] = buf[i] - 1;
    }
}

__global__ void empty_kernel(char *buf, size_t len) {}

extern "C" void call_kernel(float a, float *d_x, float *d_y, int N,
                            cudaStream_t *stream)
{
    compute_kernel<<<(N + 255) / 256, 256, 0, *stream>>>(a, d_x, d_y, N);
}

extern "C" void call_touch_managed_kernel_add(char *buf, size_t length,
                                              cudaStream_t *stream)
{
    touch_managed_kernel_add<<<(length + 255) / 256, 256, 0, *stream>>>(buf,
                                                                        length);
}

extern "C" void call_touch_managed_kernel_sub(char *buf, size_t length,
                                              cudaStream_t *stream)
{
    touch_managed_kernel_sub<<<(length + 255) / 256, 256, 0, *stream>>>(buf,
                                                                        length);
}

extern "C" void call_empty_kernel(char *buf, size_t length,
                                  cudaStream_t *stream)
{
    empty_kernel<<<(length + 255) / 256, 256, 0, *stream>>>(buf, length);
}
