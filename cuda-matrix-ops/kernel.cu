#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include "cublas_v2.h"
#include <omp.h>
#include <iostream>
#include "matrix.h"

#define CEIL(n, m) ((n + m - 1) / m)
#define START start = std::chrono::steady_clock::now();
#define END(NAME) end = std::chrono::steady_clock::now(); \
			duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); \
			printf(NAME " took %d ms\n", duration);


__global__ void add(const float* lhs, const float* rhs, float* res, size_t n) {
	const size_t index = threadIdx.x + blockIdx.x * blockDim.x;
	if (index < n) {
		res[index] = lhs[index] + rhs[index];
	}
}

__global__ void mul_scalar(const float* lhs, float scalar, float* res, size_t n) {
	const size_t index = threadIdx.x + blockIdx.x * blockDim.x;
	if (index < n) {
		res[index] = lhs[index] * scalar;
	}
}

__global__ void matmul_naive(const float* lhs, const float* rhs, float* res, size_t r, size_t k, size_t c) {
	const int x = threadIdx.x + blockIdx.x * blockDim.x;
	const int y = threadIdx.y + blockIdx.y * blockDim.y;
	if (x < r && y < c) {
		float temp = 0.0f;
		for (size_t i = 0; i < k; i++) {
			temp += lhs[x * k + i] * rhs[i * c + y];
		}
		res[x * c + y] = temp;
	}
}

__global__ void matmul_coalesce(const float* lhs, const float* rhs, float* res, size_t r, size_t k, size_t c) {
	const int x = threadIdx.x + blockIdx.x * blockDim.x;
	const int y = threadIdx.y + blockIdx.y * blockDim.y;
	if (x < c && y < r) {
		float temp = 0.0f;
		for (size_t i = 0; i < k; i++) {
			temp += lhs[y * k + i] * rhs[i * c + x];
		}
		res[y * c + x] = temp;
	}
}

#define BLOCK_SIZE 32
#define lhs(m, n) lhs[(m) * edge + (n)]
#define rhs(m, n) rhs[(m) * edge + (n)]
#define res(m, n) res[(m) * edge + (n)]

__global__ void matmul_tiling_shared_memory(const float* lhs, const float* rhs, float* res, size_t edge) {
	__shared__ float shared_lhs[BLOCK_SIZE][BLOCK_SIZE];
	__shared__ float shared_rhs[BLOCK_SIZE][BLOCK_SIZE];
	const int block_x = blockIdx.x; 
	const int block_y = blockIdx.y; 
	const int thread_x = threadIdx.x;
	const int thread_y = threadIdx.y; 
	const int x = block_x * BLOCK_SIZE + thread_x; 
	const int y = block_y * BLOCK_SIZE + thread_y;
	float temp = 0.0f;
	for (int i = 0; i < edge; i += BLOCK_SIZE) {
		shared_lhs[thread_y][thread_x] = lhs(y, i + thread_x);
		shared_rhs[thread_y][thread_x] = rhs(i + thread_y, x);
		__syncthreads();
		for (int j = 0; j < BLOCK_SIZE; j++) {
			temp += shared_lhs[thread_y][j] * shared_rhs[j][thread_x];
		}
		__syncthreads();
	}
	res(y, x) = temp;
}

__global__ void matmul_tiling_shared_memory_inside(const float* lhs, const float* rhs, float* res, size_t edge) {
	__shared__ float shared_lhs[BLOCK_SIZE][BLOCK_SIZE];
	__shared__ float shared_rhs[BLOCK_SIZE][BLOCK_SIZE];
	const int block_x = blockIdx.x;
	const int block_y = blockIdx.y;
	const int thread_x = threadIdx.x;
	const int thread_y = threadIdx.y;
	const int x = block_x * BLOCK_SIZE + thread_x;
	const int y = block_y * BLOCK_SIZE + thread_y;
	float temp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (int i = 0; i < edge; i += BLOCK_SIZE) {
		shared_lhs[thread_y][thread_x] = lhs(y, i + thread_x);
		shared_lhs[thread_y + 8][thread_x] = lhs(y + 8, i + thread_x);
		shared_lhs[thread_y + 16][thread_x] = lhs(y + 16, i + thread_x);
		shared_lhs[thread_y + 24][thread_x] = lhs(y + 24, i + thread_x);

		shared_rhs[thread_y][thread_x] = rhs(i + thread_y, x);
		shared_rhs[thread_y + 8][thread_x] = rhs(i + thread_y + 8, x);
		shared_rhs[thread_y + 16][thread_x] = rhs(i + thread_y + 16, x);
		shared_rhs[thread_y + 24][thread_x] = rhs(i + thread_y + 24, x);

		__syncthreads();
		for (int j = 0; j < BLOCK_SIZE; j++) {
			temp[0] += shared_lhs[thread_y][j] * shared_rhs[j][thread_x];
			temp[1] += shared_lhs[thread_y + 8][j] * shared_rhs[j][thread_x];
			temp[2] += shared_lhs[thread_y + 16][j] * shared_rhs[j][thread_x];
			temp[3] += shared_lhs[thread_y + 24][j] * shared_rhs[j][thread_x];

		}
		__syncthreads();
	}
	res(y, x) = temp[0];
	res(y + 8, x) = temp[1];
	res(y + 16, x) = temp[2];
	res(y + 24, x) = temp[3];
}

#undef BLOCK_SIZE
#undef lhs(m, n)
#undef rhs(m, n)
#undef res(m, n)



inline Matrix Matrix::add_gpu(const Matrix& lhs, const Matrix& rhs) {
	if (lhs.sharedData->data_cols != rhs.sharedData->data_cols || lhs.sharedData->data_rows != rhs.sharedData->data_rows) {
		throw std::invalid_argument("Matrix dimensions do not match");
	}
	Matrix result(lhs.sharedData->data_rows, lhs.sharedData->data_cols);
	const size_t total = lhs.sharedData->data_cols * lhs.sharedData->data_rows;
	if (cudaMemcpy(lhs.sharedData->data_gpu, lhs.sharedData->data_cpu, total * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess
		|| cudaMemcpy(rhs.sharedData->data_gpu, rhs.sharedData->data_cpu, total * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) {
		throw std::runtime_error("Error copying data to GPU");
	}
	add <<< CEIL(total, 1024), 1024 >> > (lhs.sharedData->data_gpu, rhs.sharedData->data_gpu, result.sharedData->data_gpu, total);
	if (cudaMemcpy(result.sharedData->data_cpu, result.sharedData->data_gpu, total * sizeof(float), cudaMemcpyDeviceToHost) != cudaSuccess) {
		throw std::runtime_error("Error copying data to CPU");
	}
	return result;
}

inline Matrix Matrix::add_cpu(const Matrix& lhs, const Matrix& rhs) {
	if (lhs.sharedData->data_cols != rhs.sharedData->data_cols || lhs.sharedData->data_rows != rhs.sharedData->data_rows) {
		throw std::invalid_argument("Matrix dimensions do not match");
	}
	Matrix result(lhs.sharedData->data_rows, lhs.sharedData->data_cols);
	const size_t total = lhs.sharedData->data_cols * lhs.sharedData->data_rows;
	const float* p_lhs = lhs.sharedData->data_cpu;
	const float* p_rhs = rhs.sharedData->data_cpu;
	float* p_result = result.sharedData->data_cpu;
#pragma omp parallel for
	for (size_t i = 0; i < total; i++) {
		p_result[i] = p_lhs[i] + p_rhs[i];
	}
	return result;
}

inline Matrix Matrix::multiply_scalar_gpu(const Matrix& lhs, float scalar) {
	Matrix result(lhs.sharedData->data_rows, lhs.sharedData->data_cols);
	const size_t total = lhs.sharedData->data_cols * lhs.sharedData->data_rows;
	if (cudaMemcpy(lhs.sharedData->data_gpu, lhs.sharedData->data_cpu, total * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) {
		throw std::runtime_error("Error copying data to GPU");
	}
	mul_scalar << < CEIL(total, 1024), 1024 >> > (lhs.sharedData->data_gpu, scalar, result.sharedData->data_gpu, total);
	if (cudaMemcpy(result.sharedData->data_cpu, result.sharedData->data_gpu, total * sizeof(float), cudaMemcpyDeviceToHost) != cudaSuccess) {
		throw std::runtime_error("Error copying data to CPU");
	}
	return result;
}

inline Matrix Matrix::multiply_scalar_cpu(const Matrix& lhs, float scalar) {
	Matrix result(lhs.sharedData->data_rows, lhs.sharedData->data_cols);
	const size_t total = lhs.sharedData->data_cols * lhs.sharedData->data_rows;
	const float* p_lhs = lhs.sharedData->data_cpu;
	float* p_result = result.sharedData->data_cpu;
#pragma omp parallel for
	for (size_t i = 0; i < total; i++) {
		p_result[i] = p_lhs[i] * scalar;
	}
	return result;
}

inline Matrix Matrix::matmul_cpu_naive(const Matrix& lhs, const Matrix& rhs) {
	if (lhs.sharedData->data_cols != rhs.sharedData->data_rows) {
		throw std::invalid_argument("Matrix dimensions do not match");
	}
	Matrix result(lhs.sharedData->data_rows, rhs.sharedData->data_cols, 0.0f);
	// naive matrix multiplication on CPU
	const size_t total = lhs.sharedData->data_rows * rhs.sharedData->data_cols;
	const float* p_lhs = lhs.sharedData->data_cpu;
	const float* p_rhs = rhs.sharedData->data_cpu;
	float* p_result = result.sharedData->data_cpu;
#pragma omp parallel for
	for (size_t i = 0; i < lhs.sharedData->data_rows; i++) {
		for (size_t k = 0; k < lhs.sharedData->data_cols; k++) {
			for (size_t j = 0; j < rhs.sharedData->data_cols; j++) {
				p_result[i * rhs.sharedData->data_cols + j] += p_lhs[i * lhs.sharedData->data_cols + k] * p_rhs[k * rhs.sharedData->data_cols + j];
			}
		}
	}
	return result;
}

inline Matrix Matrix::matmul_gpu_naive(const Matrix& lhs, const Matrix& rhs) {
	if (lhs.sharedData->data_cols != rhs.sharedData->data_rows) {
		throw std::invalid_argument("Matrix dimensions do not match");
	}
	Matrix result(lhs.sharedData->data_rows, rhs.sharedData->data_cols, 0.0f);
	const size_t result_total = result.sharedData->data_rows * result.sharedData->data_cols;
	if (cudaMemcpy(lhs.sharedData->data_gpu, lhs.sharedData->data_cpu, lhs.sharedData->data_rows * lhs.sharedData->data_cols * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess
		|| cudaMemcpy(rhs.sharedData->data_gpu, rhs.sharedData->data_cpu, rhs.sharedData->data_rows * rhs.sharedData->data_cols * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) {
		throw std::runtime_error("Error copying data to GPU");
	}
	dim3 threadsPerBlock(32, 32, 1);
	dim3 blocksPerGrid(CEIL(lhs.sharedData->data_rows, 32), CEIL(rhs.sharedData->data_cols, 32));
	matmul_naive << < blocksPerGrid, threadsPerBlock >> > (lhs.sharedData->data_gpu, rhs.sharedData->data_gpu, result.sharedData->data_gpu, lhs.sharedData->data_rows, lhs.sharedData->data_cols, rhs.sharedData->data_cols);
	if (cudaMemcpy(result.sharedData->data_cpu, result.sharedData->data_gpu, result_total * sizeof(float), cudaMemcpyDeviceToHost) != cudaSuccess) {
		throw std::runtime_error("Error copying data to CPU");
	}
	return result;
}

inline Matrix Matrix::matmul_gpu_coalesce(const Matrix& lhs, const Matrix& rhs) {
	if (lhs.sharedData->data_cols != rhs.sharedData->data_rows) {
		throw std::invalid_argument("Matrix dimensions do not match");
	}
	Matrix result(lhs.sharedData->data_rows, rhs.sharedData->data_cols, 0.0f);
	if (cudaMemcpy(lhs.sharedData->data_gpu, lhs.sharedData->data_cpu, lhs.sharedData->data_rows * lhs.sharedData->data_cols * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess
		|| cudaMemcpy(rhs.sharedData->data_gpu, rhs.sharedData->data_cpu, rhs.sharedData->data_rows * rhs.sharedData->data_cols * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) {
		throw std::runtime_error("Error copying data to GPU");
	}
	dim3 threadsPerBlock(32, 32, 1);
	dim3 blocksPerGrid(CEIL(lhs.sharedData->data_rows, 32), CEIL(rhs.sharedData->data_cols, 32));
	matmul_coalesce << < blocksPerGrid, threadsPerBlock >> > (lhs.sharedData->data_gpu, rhs.sharedData->data_gpu, result.sharedData->data_gpu, lhs.sharedData->data_rows, lhs.sharedData->data_cols, rhs.sharedData->data_cols);
	if (cudaMemcpy(result.sharedData->data_cpu, result.sharedData->data_gpu, result.sharedData->data_rows * result.sharedData->data_cols * sizeof(float), cudaMemcpyDeviceToHost) != cudaSuccess) {
		throw std::runtime_error("Error copying data to CPU");
	}
	return result;
}

inline Matrix Matrix::matmul_gpu_tiling_shared_memory(const Matrix& lhs, const Matrix& rhs) {
	if (lhs.sharedData->data_cols != rhs.sharedData->data_rows) {
		throw std::invalid_argument("Matrix dimensions do not match");
	}
	Matrix result(lhs.sharedData->data_rows, rhs.sharedData->data_cols, 0.0f);
	if (cudaMemcpy(lhs.sharedData->data_gpu, lhs.sharedData->data_cpu, lhs.sharedData->data_rows * lhs.sharedData->data_cols * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess
		|| cudaMemcpy(rhs.sharedData->data_gpu, rhs.sharedData->data_cpu, rhs.sharedData->data_rows * rhs.sharedData->data_cols * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) {
		throw std::runtime_error("Error copying data to GPU");
	}
	dim3 threadsPerBlock(32, 32, 1);
	dim3 blocksPerGrid(CEIL(lhs.sharedData->data_rows, 32), CEIL(rhs.sharedData->data_cols, 32));
	matmul_tiling_shared_memory << < blocksPerGrid, threadsPerBlock >> > (lhs.sharedData->data_gpu, rhs.sharedData->data_gpu, result.sharedData->data_gpu, lhs.sharedData->data_rows);
	if (cudaMemcpy(result.sharedData->data_cpu, result.sharedData->data_gpu, result.sharedData->data_rows * result.sharedData->data_cols * sizeof(float), cudaMemcpyDeviceToHost) != cudaSuccess) {
		throw std::runtime_error("Error copying data to CPU");
	}
	return result;
}

inline Matrix Matrix::matmul_gpu_tiling_shared_memory_inside(const Matrix& lhs, const Matrix& rhs) {
	if (lhs.sharedData->data_cols != rhs.sharedData->data_rows) {
		throw std::invalid_argument("Matrix dimensions do not match");
	}
	Matrix result(lhs.sharedData->data_rows, rhs.sharedData->data_cols, 0.0f);
	if (cudaMemcpy(lhs.sharedData->data_gpu, lhs.sharedData->data_cpu, lhs.sharedData->data_rows * lhs.sharedData->data_cols * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess
		|| cudaMemcpy(rhs.sharedData->data_gpu, rhs.sharedData->data_cpu, rhs.sharedData->data_rows * rhs.sharedData->data_cols * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) {
		throw std::runtime_error("Error copying data to GPU");
	}
	dim3 threadsPerBlock(32, 8, 1);
	dim3 blocksPerGrid(CEIL(lhs.sharedData->data_rows, 32), CEIL(rhs.sharedData->data_cols, 32));
	matmul_tiling_shared_memory_inside << < blocksPerGrid, threadsPerBlock >> > (lhs.sharedData->data_gpu, rhs.sharedData->data_gpu, result.sharedData->data_gpu, lhs.sharedData->data_rows);

	if (cudaMemcpy(result.sharedData->data_cpu, result.sharedData->data_gpu, result.sharedData->data_rows * result.sharedData->data_cols * sizeof(float), cudaMemcpyDeviceToHost) != cudaSuccess) {
		throw std::runtime_error("Error copying data to CPU");
	}
	return result;
}



inline Matrix Matrix::matmul_cuBLAS(const Matrix& lhs, const Matrix& rhs) {
	if (lhs.sharedData->data_cols != rhs.sharedData->data_rows) {
		throw std::invalid_argument("Matrix dimensions do not match");
	}
	Matrix result(lhs.sharedData->data_rows, rhs.sharedData->data_cols);
	const size_t result_total = result.sharedData->data_rows * result.sharedData->data_cols;
	if (cudaMemcpy(lhs.sharedData->data_gpu, lhs.sharedData->data_cpu, lhs.sharedData->data_rows * lhs.sharedData->data_cols * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess
		|| cudaMemcpy(rhs.sharedData->data_gpu, rhs.sharedData->data_cpu, rhs.sharedData->data_rows * rhs.sharedData->data_cols * sizeof(float), cudaMemcpyHostToDevice) != cudaSuccess) {
		throw std::runtime_error("Error copying data to GPU");
	}
	const float alpha = 1.0f;
	const float beta = 0.0f;
	cublasHandle_t handle;
	cublasCreate(&handle);
	//if (cublasSgemm(handle, 
	//	CUBLAS_OP_N, CUBLAS_OP_N,
	//	lhs.sharedData->data_rows,
	//	rhs.sharedData->data_cols,
	//	lhs.sharedData->data_cols, &alpha,
	//	lhs.sharedData->data_gpu,
	//	lhs.sharedData->data_rows,
	//	rhs.sharedData->data_gpu,
	//	rhs.sharedData->data_rows, &beta, 
	//	result.sharedData->data_gpu, 
	//	result.sharedData->data_rows) != CUBLAS_STATUS_SUCCESS) {
	//	throw std::runtime_error("Error multiplying matrices");
	//}
	if (cublasSgemm(handle,
		CUBLAS_OP_N, CUBLAS_OP_N, // transpose both matrices
		rhs.sharedData->data_cols,
		lhs.sharedData->data_rows,
		rhs.sharedData->data_rows,
		&alpha,
		rhs.sharedData->data_gpu, // pass rhs first because of transpose
		rhs.sharedData->data_cols,
		lhs.sharedData->data_gpu, // pass lhs second
		lhs.sharedData->data_cols,
		&beta,
		result.sharedData->data_gpu,
		result.sharedData->data_cols) != CUBLAS_STATUS_SUCCESS) {
		throw std::runtime_error("Error multiplying matrices");
	}
	cublasDestroy(handle);
	if (cudaMemcpy(result.sharedData->data_cpu, result.sharedData->data_gpu, result_total * sizeof(float), cudaMemcpyDeviceToHost) != cudaSuccess) {
		throw std::runtime_error("Error copying data to CPU");
	}
	return result;
}


int main() {
	try {
		// get device properties
		cudaDeviceProp prop;
		cudaGetDeviceProperties(&prop, 0);
		// max shared memory per block
		printf("Max shared memory per block: %d\n", prop.sharedMemPerBlock);
		Matrix a(2560, 2560);
		Matrix b(2560, 2560);
		Matrix c, d, e, f, g;

		a.fill_random(-1.0f, 1.0f);
		b.fill_random(-1.0f, 1.0f);
		auto start = std::chrono::steady_clock::now();
		auto end = std::chrono::steady_clock::now();
		auto duration = 0L;


		START
			c = Matrix::matmul_cuBLAS(a, b);
		END("GPU_cuBLAS")
		std::cout << std::endl;

		//START
		//	d = Matrix::matmul_gpu_naive(a, b);
		//END("GPU_naive");
		//Matrix::printMaxDiff(c, d);
		//std::cout << std::endl;

		START
			e = Matrix::matmul_gpu_coalesce(a, b);
		END("GPU_coalesce");
		Matrix::printMaxDiff(c, e);
		std::cout << std::endl;

		START
			f = Matrix::matmul_gpu_tiling_shared_memory(a, b);
		END("GPU_blocking");
		Matrix::printMaxDiff(c, f);
		std::cout << std::endl;

		START
			g = Matrix::matmul_gpu_tiling_shared_memory_inside(a, b);
		END("GPU_blocking_inside");
		Matrix::printMaxDiff(c, g);
		std::cout << std::endl;





		//START
		//	g = Matrix::matmul_cpu_naive(a, b);
		//END("CPU");
		//Matrix::printMaxDiff(c, g);
		//std::cout << std::endl;

		//c.show();
		//d.show();
		//e.show();
		//f.show();
		//g.show();


		//c.show();
		//d.show();

		//std::cout << std::boolalpha << (c == d) << std::endl;


		//Matrix::printMaxDiff(d, f);

		return EXIT_SUCCESS;
	}
	catch (std::bad_alloc& e) {
		printf("Out of memory\n");
		return EXIT_FAILURE;
	}
	catch (std::invalid_argument& e) {
		printf("Invalid argument: %s\n", e.what());
		return EXIT_FAILURE;
	}
	catch (std::exception& e) {
		printf("Error: %s\n", e.what());
		return EXIT_FAILURE;
	}

}

/*
__global__ void matmul_tiling_shared_memory (const float* lhs, const float* rhs, float* res, size_t edge) {
	__shared__ shared_lhs[BLOCK_SIZE][BLOCK_SIZE];
	__shared__ shared_rhs[BLOCK_SIZE][BLOCK_SIZE];
	const int block_x = blockIdx.x;
	const int block_y = blockIdx.y;
	const int thread_x = threadIdx.x;
	const int thread_y = threadIdx.y;
	const int x = block_x * BLOCK_SIZE + thread_x;
	const int y = block_y * BLOCK_SIZE + thread_y;
	float temp = 0.0f;
	for (int i = 0; i < edge; i += BLOCK_SIZE) {
		shared_lhs[thread_y][thread_x] = lhs(y, i + thread_x);
		shared_rhs[thread_y][thread_x] = rhs(i + thread_y, x);
		__syncthreads();
		for (int j = 0; j < BLOCK_SIZE; j++) {
			temp += shared_lhs[thread_y][j] * shared_rhs[j][thread_x];
		}
		__syncthreads();
	}
	res(y, x) = temp;
}
*/