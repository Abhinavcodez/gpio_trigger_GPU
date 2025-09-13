// gpu_trigger_user.cu
// Build: nvcc -O2 -o gpu_trigger_user gpu_trigger_user.cu
// Run: sudo ./gpu_trigger_user <mode>
// mode: 0=IRQ, 1=IOCTL, 2=Sysfs-direct

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <cuda_runtime.h>

#define DEVICE_PATH      "/dev/gpio_trigger"
#define SYSFS_MODE_DIR   "/sys/class/gpio_trigger_class_unique"
#define SYSFS_MODE_FILE  SYSFS_MODE_DIR "/mode"
#define GPU_TRIGGER_IOCTL _IO('K', 1)

static void checkCuda(cudaError_t err, const char* where) {
    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA error at %s: %s\n", where, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

static void print_cuda_device_info() {
    int devCount = 0;
    cudaError_t err = cudaGetDeviceCount(&devCount);
    if (err != cudaSuccess) {
        fprintf(stderr, "cudaGetDeviceCount error: %s\n", cudaGetErrorString(err));
        return;
    }
    printf("[CUDA] Device count: %d\n", devCount);
    for (int d = 0; d < devCount; ++d) {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, d);
        printf("[CUDA] Device %d: %s (SMs=%d, totalMem=%.2f MB)\n",
               d, prop.name, prop.multiProcessorCount, prop.totalGlobalMem / (1024.0*1024.0));
    }
}

// Different kernels / sizes for modes
__global__ void vec_add_kernel(const float *A, const float *B, float *C, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) C[i] = A[i] + B[i];
}

void run_cuda_vec_add(int N) {
    size_t bytes = (size_t)N * sizeof(float);
    float *h_A = (float*)malloc(bytes);
    float *h_B = (float*)malloc(bytes);
    float *h_C = (float*)malloc(bytes);
    if (!h_A || !h_B || !h_C) {
        fprintf(stderr, "Host malloc failed\n");
        free(h_A); free(h_B); free(h_C);
        return;
    }

    // init inputs so result won't be all zeros
    for (int i = 0; i < N; ++i) {
        h_A[i] = (float)i * 1.0f;
        h_B[i] = (float)i * 2.0f;
    }

    float *d_A = NULL, *d_B = NULL, *d_C = NULL;
    checkCuda(cudaMalloc((void**)&d_A, bytes), "cudaMalloc d_A");
    checkCuda(cudaMalloc((void**)&d_B, bytes), "cudaMalloc d_B");
    checkCuda(cudaMalloc((void**)&d_C, bytes), "cudaMalloc d_C");

    checkCuda(cudaMemcpy(d_A, h_A, bytes, cudaMemcpyHostToDevice), "cudaMemcpy H2D d_A");
    checkCuda(cudaMemcpy(d_B, h_B, bytes, cudaMemcpyHostToDevice), "cudaMemcpy H2D d_B");

    int threads = 256;
    int blocks = (N + threads - 1) / threads;
    vec_add_kernel<<<blocks, threads>>>(d_A, d_B, d_C, N);

    // synchronize & check kernel error
    checkCuda(cudaDeviceSynchronize(), "cudaDeviceSynchronize after kernel");
    cudaError_t kerr = cudaGetLastError();
    if (kerr != cudaSuccess) {
        fprintf(stderr, "Kernel launch error: %s\n", cudaGetErrorString(kerr));
    }

    checkCuda(cudaMemcpy(h_C, d_C, bytes, cudaMemcpyDeviceToHost), "cudaMemcpy D2H d_C");

    // print a few samples
    printf("[CUDA] N=%d sample: C[0]=%f, C[%d]=%f\n", N, h_C[0], N-1, h_C[N-1]);

    cudaFree(d_A); cudaFree(d_B); cudaFree(d_C);
    free(h_A); free(h_B); free(h_C);
}

int write_sysfs_mode(int mode) {
    int fd = open(SYSFS_MODE_FILE, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open sysfs mode '%s': %s\n", SYSFS_MODE_FILE, strerror(errno));
        return -1;
    }
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "%d\n", mode);
    if (write(fd, buf, len) != len) {
        fprintf(stderr, "Failed to write mode to sysfs: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

ssize_t read_device_once(int devfd, char *buf, size_t bufsize) {
    // read from device: driver returns result_size bytes if available
    lseek(devfd, 0, SEEK_SET); // reset file pos
    ssize_t r = read(devfd, buf, bufsize-1);
    if (r > 0) buf[r] = '\0';
    return r;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mode>\n 0=IRQ 1=IOCTL 2=Sysfs\n", argv[0]);
        return 1;
    }

    int mode = atoi(argv[1]);
    if (mode < 0 || mode > 2) {
        fprintf(stderr, "Invalid mode %d\n", mode);
        return 1;
    }

    print_cuda_device_info();
    // select device 0 by default (adjust if you want another)
    cudaError_t err = cudaSetDevice(0);
    if (err != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice(0) failed: %s\n", cudaGetErrorString(err));
        // still continue; cudaMalloc will fail if invalid
    }

    // open char device
    int fd = open(DEVICE_PATH, O_RDWR | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "Failed to open device '%s': %s\n", DEVICE_PATH, strerror(errno));
        return 1;
    }

    printf("gpu_trigger_user: running for mode %d\n", mode);

    char devbuf[256];
    ssize_t r;

    if (mode == 1) {
        // IOCTL mode: set sysfs mode then issue ioctl once and read result
        if (write_sysfs_mode(1) != 0) { close(fd); return 1; }
        printf("[Mode 1] writing sysfs mode=1 and issuing IOCTL\n");
        if (ioctl(fd, GPU_TRIGGER_IOCTL) < 0) {
            fprintf(stderr, "ioctl failed: %s\n", strerror(errno));
        } else {
            // wait small time for kernel work to finish; driver writes result quickly
            for (int i = 0; i < 10; ++i) {
                r = read_device_once(fd, devbuf, sizeof(devbuf));
                if (r > 0) break;
                usleep(100000); // 100ms
            }
            if (r > 0) {
                printf("[Driver result] %s", devbuf);
            } else {
                printf("[Driver result] no result available after ioctl\n");
            }
            // Launch CUDA with mode-dependent size
            int N = 1<<20; // medium
            run_cuda_vec_add(N);
        }
    }
    else if (mode == 2) {
        // Sysfs trigger: write mode=2, kernel triggers immediately; read result then run CUDA
        if (write_sysfs_mode(2) != 0) { close(fd); return 1; }
        printf("[Mode 2] wrote sysfs mode=2 (kernel triggers work)\n");
        // wait for device result
        for (int i = 0; i < 20; ++i) {
            r = read_device_once(fd, devbuf, sizeof(devbuf));
            if (r > 0) break;
            usleep(100000);
        }
        if (r > 0) {
            printf("[Driver result] %s", devbuf);
        } else {
            printf("[Driver result] no result available after sysfs write\n");
        }
        // Mode 2: bigger job
        int N = 1<<22; // ~4M elements (be careful with memory)
        // reduce if your GPU memory is small:
        size_t approx_bytes = (size_t)N * sizeof(float);
        if (approx_bytes > (size_t)1024ull*1024ull*1024ull) {
            // if >1GB, reduce
            N = 1<<20;
            printf("[Mode2] large job reduced to N=%d to fit memory\n", N);
        }
        run_cuda_vec_add(N);
    }
    else { // mode == 0 (IRQ)
        if (write_sysfs_mode(0) != 0) { close(fd); return 1; }
        printf("[Mode 0] waiting for kernel to produce result (driver's IRQ handling)...\n");
        // Poll/read loop: only run CUDA when driver read returns >0
        while (1) {
            r = read_device_once(fd, devbuf, sizeof(devbuf));
            if (r > 0) {
                printf("[Driver result] %s", devbuf);
                // Small job for interrupt mode
                int N = 1<<16;
                run_cuda_vec_add(N);
            }
            // sleep a bit to avoid busy loop
            usleep(200000); // 200ms
        }
    }

    close(fd);
    return 0;
}