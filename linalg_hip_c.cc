#include <hip/hip_runtime.h>
#include <iostream>
#include <chrono>
#include <cmath>

__global__ void matrix_multiply_kernel(double *a, double *b, double *c, int N, int M) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;

    if (i < N && j < M) {
        double temp = 0.0f;
        for (int k = 0; k < N; k++) {
            temp += a[i * N + k] * b[k * M + j];
        }
        c[i * M + j] = temp;
    }
}

#include <hip/hip_runtime.h>
#include <iostream>
#include <cmath>


extern "C" void matrix_multiply_2d_gpu(double *h_matrix_a, double *h_matrix_b, double *h_matrix_c, int N, int M, int K){

    // Allocate memory for matrices on device
    double *d_matrix_a, *d_matrix_b, *d_matrix_c;
    hipMalloc(&d_matrix_a, N * K * sizeof(double));
    hipMalloc(&d_matrix_b, K * M * sizeof(double));
    hipMalloc(&d_matrix_c, N * M * sizeof(double));

    // Copy matrices from host to device
    hipMemcpy(d_matrix_a, h_matrix_a, N * K * sizeof(double), hipMemcpyHostToDevice);
    hipMemcpy(d_matrix_b, h_matrix_b, K * M * sizeof(double), hipMemcpyHostToDevice);

    // Define grid and block dimensions
    dim3 threadsPerBlock(16, 16); // Modify as needed
    dim3 numBlocks((N + threadsPerBlock.x - 1) / threadsPerBlock.x, 
                   (M + threadsPerBlock.y - 1) / threadsPerBlock.y);
    matrix_multiply_kernel<<<numBlocks, threadsPerBlock>>>(d_matrix_a, d_matrix_b, d_matrix_c, N, M);
    hipDeviceSynchronize();

    // Check for kernel execution errors
    hipError_t err = hipGetLastError();
    if (err != hipSuccess) {
        std::cerr << "HIP Kernel launch failed: " << hipGetErrorString(err) << std::endl;
        return;
    }
    // Copy result matrix back to host
    hipMemcpy(h_matrix_c, d_matrix_c, N * M * sizeof(double), hipMemcpyDeviceToHost);
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
    double *h_matrix_a = new double[N * K];
    double *h_matrix_b = new double[K * M];
    double *h_matrix_c = new double[N * M];
    double *h_matrix_d = new double[N * M];

    // Initialize matrices
    double v1 = 2.0f;
    double v2 = 3.0f;
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

    // Start kernel execution
    auto start = std::chrono::high_resolution_clock::now();

    matrix_multiply_2d_gpu(h_matrix_a, h_matrix_b, h_matrix_c, N, M, K);
    
    auto kernel_duration = std::chrono::high_resolution_clock::now() - start;


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
    delete[] h_matrix_a;
    delete[] h_matrix_b;
    delete[] h_matrix_c;
    delete[] h_matrix_d;

    return 0;
}
