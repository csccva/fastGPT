#include <hip/hip_runtime.h>
#include <iostream>
#include <chrono>
#include <cmath>

__global__ void matrix_multiply_kernel(float *a, float *b, float *c, int N, int M) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;

    if (i < N && j < M) {
        float temp = 0.0f;
        for (int k = 0; k < N; k++) {
            temp += a[i * N + k] * b[k * M + j];
        }
        c[i * M + j] = temp;
    }
}

#include <hip/hip_runtime.h>
#include <iostream>
#include <cmath>

extern "C" void matrix_multiply_gpu(float *h_matrix_a, float *h_matrix_b, float *h_matrix_c, int N, int M, int K) {
    float *d_matrix_a, *d_matrix_b, *d_matrix_c;

    // Allocate memory on the device
    hipMalloc(&d_matrix_a, N * K * sizeof(float));
    hipMalloc(&d_matrix_b, K * M * sizeof(float));
    hipMalloc(&d_matrix_c, N * M * sizeof(float));

    // Copy matrices from host to device
    hipMemcpy(d_matrix_a, h_matrix_a, N * K * sizeof(float), hipMemcpyHostToDevice);
    hipMemcpy(d_matrix_b, h_matrix_b, K * M * sizeof(float), hipMemcpyHostToDevice);

    // Define grid and block dimensions
    dim3 threadsPerBlock(16, 16); // Modify as needed
    dim3 numBlocks((N + threadsPerBlock.x - 1) / threadsPerBlock.x, 
                   (M + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // Kernel execution
    matrix_multiply_kernel<<<numBlocks, threadsPerBlock>>>(d_matrix_a, d_matrix_b, d_matrix_c, N, M);
    hipDeviceSynchronize();

    // Check for kernel execution errors
    hipError_t err = hipGetLastError();
    if (err != hipSuccess) {
        std::cerr << "HIP Kernel launch failed: " << hipGetErrorString(err) << std::endl;
        return;
    }

    // Copy result matrix back to host
    hipMemcpy(h_matrix_c, d_matrix_c, N * M * sizeof(float), hipMemcpyDeviceToHost);

    // Free memory on device
    hipFree(d_matrix_a);
    hipFree(d_matrix_b);
    hipFree(d_matrix_c);
}


int main() {
    // Set matrix dimensions
    int N = 1024; // Rows of matrix A and C
    int M = 4096; // Columns of matrix B and C
    int K = 2048; // Columns of matrix A and rows of matrix B

    // Allocate memory for matrices on host
    float *h_matrix_a = new float[N * K];
    float *h_matrix_b = new float[K * M];
    float *h_matrix_c = new float[N * M];
    float *h_matrix_d = new float[N * M];

    // Initialize matrices
    float v1 = 2.0f;
    float v2 = 3.0f;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < K; j++) {
            h_matrix_a[i * K + j] = v1++;
        }
    }
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < M; j++) {
            h_matrix_b[i * M + j] = v2++;
        }
    }

    // Initialize result matrix to zero
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            h_matrix_c[i * M + j] = 0.0f;
            h_matrix_d[i * M + j] = 0.0f;
        }
    }

    // Allocate memory for matrices on device
    float *d_matrix_a, *d_matrix_b, *d_matrix_c;
    hipMalloc(&d_matrix_a, N * K * sizeof(float));
    hipMalloc(&d_matrix_b, K * M * sizeof(float));
    hipMalloc(&d_matrix_c, N * M * sizeof(float));

    // Copy matrices from host to device
    hipMemcpy(d_matrix_a, h_matrix_a, N * K * sizeof(float), hipMemcpyHostToDevice);
    hipMemcpy(d_matrix_b, h_matrix_b, K * M * sizeof(float), hipMemcpyHostToDevice);

    // Define grid and block dimensions
    dim3 threadsPerBlock(16, 16); // Modify as needed
    dim3 numBlocks((N + threadsPerBlock.x - 1) / threadsPerBlock.x, 
                   (M + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // Start kernel execution
    auto start = std::chrono::high_resolution_clock::now();
    matrix_multiply_kernel<<<numBlocks, threadsPerBlock>>>(d_matrix_a, d_matrix_b, d_matrix_c, N, M);
    hipDeviceSynchronize();
    auto kernel_duration = std::chrono::high_resolution_clock::now() - start;

    // Check for kernel execution errors
    hipError_t err = hipGetLastError();
    if (err != hipSuccess) {
        std::cerr << "HIP Kernel launch failed: " << hipGetErrorString(err) << std::endl;
        return 1;
    }

    // Copy result matrix back to host
    hipMemcpy(h_matrix_c, d_matrix_c, N * M * sizeof(float), hipMemcpyDeviceToHost);

    // Print execution time
    auto kernel_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(kernel_duration).count();
    std::cout << "Kernel Execution Time : " << kernel_duration_ms / 1000.0 << " seconds" << std::endl;

    // Print first element of result matrix
    std::cout << "First element [0][0] = " << h_matrix_c[0] << std::endl;

    // Verification: Compare with CPU-based matrix multiplication
    bool fail = false;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            h_matrix_d[i * M + j] = 0.0f;
            for (int k = 0; k < K; k++) {
                h_matrix_d[i * M + j] += h_matrix_a[i * K + k] * h_matrix_b[k * M + j];
            }
            if (std::abs(h_matrix_c[i * M + j] - h_matrix_d[i * M + j]) / h_matrix_c[i * M + j] > 1.0e-5) {
                fail = true;
                std::cout << "Mismatch at [" << i << "][" << j << "]" << std::endl;
            }
        }
    }

    if (fail) {
        std::cout << "FAIL" << std::endl;
    } else {
        std::cout << "PASS" << std::endl;
    }

    // Free memory on device and host
    hipFree(d_matrix_a);
    hipFree(d_matrix_b);
    hipFree(d_matrix_c);
    delete[] h_matrix_a;
    delete[] h_matrix_b;
    delete[] h_matrix_c;
    delete[] h_matrix_d;

    return 0;
}
