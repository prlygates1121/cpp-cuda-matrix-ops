# Project 5: GPU Acceleration with CUDA

**Name:** 蓝一鸣 (Yiming Lan)

**SID:** 12312808

[TOC]

## Analysis

In this project, I:

- Modified my Matrix class to add GPU support (ROI properties are removed for brevity).
- Implemented operations a*A and A + b on CUDA.
- Compared performance of cuBLAS and OpenBLAS.
- Implemented improved matrix multiplication A*B on CUDA.

Illustrations without a source are made by myself.

## Codes & Implementation

Macros used:

```c++
#define CEIL(n, m) ((n + m - 1) / m)
#define START start = std::chrono::steady_clock::now();
#define END(NAME) end = std::chrono::steady_clock::now(); \
			duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); \
			printf(NAME " took %d ms\n", duration);
```

### Modified Matrix Class

I have modified my matrix class so that each `MatrixData` now holds 2 pointers, one for CPU and the other for GPU.

The CPU data will be copied to GPU only during calculation.

For details please refer to my source codes.

### Scalar Multiplication

Kernel:

```c++
__global__ void mul_scalar(const float* lhs, float scalar, float* res, size_t n) {
	const size_t index = threadIdx.x + blockIdx.x * blockDim.x;
	if (index < n) {
		res[index] = lhs[index] * scalar;
	}
}
```

Wrapper host function:

```c++
inline static Matrix Matrix::multiply_scalar_gpu(const Matrix& lhs, float scalar) {
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
```

We simply let the threads go through the matrix and do the multiplication element-wise.

### Matrix Addition

A + b is implemented as A + B, where B is initialized with bs.

Kernel:

```c++
__global__ void add(const float* lhs, const float* rhs, float* res, size_t n) {
	const size_t index = threadIdx.x + blockIdx.x * blockDim.x;
	if (index < n) {
		res[index] = lhs[index] + rhs[index];
	}
}
```

Wrapper host function:

```c++
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
```

We simply let the threads go through the matrix and do the addition element-wise.

### Matrix Multiplication

#### 1-D vs. 2-D Thread Representation

In the kernel for scalar multiplication and matrix addition, a single integer `index` is used to locate a thread that we want. 

```c++
int index = blockDim.x * blockIdx.x + threadIdx.x;
```

We call the kernel in a host function like this:

```c++
addKernel <<< CEIL(total, 1024), 1024 >>> (/* function arguments */); 
```

where:

- `CEIL(total, 1024)` specifies **the number of blocks in a grid**. 

  (`CEIL()` pads the blocks to make sure they can hold all threads)

- `1024`, specifies **the number of threads per block.**

The 1-D representation is feasible here because we simply want the threads to iterate through the entire matrix.

---

However, unlike these elementary operations, matrix multiplication involves more complex handling of GPU threads.

Therefore, in the kernel for matrix multiplication, we turn to the more straightforward 2-D representation of thread index. 

```c++
const int x = threadIdx.x + blockIdx.x * blockDim.x;
const int y = threadIdx.y + blockIdx.y * blockDim.y;
```

This picture shows how a thread is located in 2-D representation. We find that it perfect matches the layout of a matrix.

![image-20240602174549974](D:\Blue\C++\CS205-2024-Spring-Projects\Project5\report\threadsAlignment.png)

[]: http://users.wfu.edu/choss/CUDA/docs/Lecture%205.pdf

For 2-D indices, we need to call the kernel in the host function like this:

```c++
dim3 threadsPerBlock(32, 32);
dim3 blocksPerGrid(CEIL(lhs.sharedData->data_rows, 32), CEIL(rhs.sharedData->data_cols, 32));
// this time we put dim3 types in the <<< >>>, rather than plain numbers
matmul_naive <<< blocksPerGrid, threadsPerBlock >> > (/* function arguments */);
```

where:

- `dim3` is a built-in CUDA struct type with 3 unsigned int values as dimensions.

  ```c++
  unsigned int x, y, z;
  ```

  It has a constructor like this:

  ```c++
  __host__ __device__ dim3(unsigned int vx = 1, unsigned int vy = 1, unsigned int vz = 1) : x(vx), y(vy), z(vz) {}
  ```

  Here, `vx`, `vy` and `vz` specify the x, y and z dimensions that might be used to locate a thread.

  Since we do not need the z dimension, it is initialized as 1 by default.

- `dim3 threadsPerBlock(32, 32)` specifies that there are 32 threads respectively in the x and y axis of a block, which means a block has 32 * 32 = 1024 threads in total.

- `dim3 dim3 blocksPerGrid(CEIL(lhs.sharedData->data_rows, 32), CEIL(rhs.sharedData->data_cols, 32))` specifies the number of blocks in the x and y axis of a grid respectively.

#### 1. Naive Method

```c++
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
```

In this kernel, each thread is responsible for the calculation of a single entry of C.

The memory access pattern of this method is not good.

To understand why, we introduce the concept of *warp*:

- A warp is a group of "contiguous" threads (usually 32 threads) in the same block.
- A warp is meant to be executed following the single instruction, multiple data (SIMD) model. In particular, when threads in a warp access consecutive memory locations, all these accesses will be coalesced, or combined into a single request for consecutive locations when accessing the DRAMs.

- In 2-D representation, threads with the same `dim3.y` and adjacent `dim3.x` values are considered "contiguous", and thus tend to stay in the same warp.

However, in our method:

- For A matrix, no coalescing is possible because the elements each thread accesses at a time are not stored contiguously. The hardware will determine that accesses to these elements are far away from each other and cannot be coalesced. Therefore, **each thread in the warp has to load memory separately** for each `i`, significantly slowing down the program.
- For B matrix, although the threads can share memory now, they have to stride across rows, which is not good enough.

Therefore, this method does not meet the requirement for coalesced access.

![image-20240602121230297](D:\Blue\C++\CS205-2024-Spring-Projects\Project5\report\Naive.png)



#### 2. Coalesced Access

```c++
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
```

In this kernel, by interchanging the role of `x` and `y`, we have a different memory access pattern.

```c++
temp += lhs[y * k + i] * rhs[i * c + x];
```

Here, in `rhs[i * c + x]`, threads with x = 0, 1, 2 ... that are in the same warp can access contiguous memory.

- For B matrix, a warp of threads **only need to load once** for each `i` because they are accessing contiguous memory at the same time.
- A matrix looks really nice. Threads in a warp can make even better use of their shared memory.

![image-20240602122300164](D:\Blue\C++\CS205-2024-Spring-Projects\Project5\report\Coalesce.png)



#### 3. Tiling For Shared Memory 

```c++
#define BLOCK_SIZE 32
#define lhs(m, n) lhs[(m) * edge + (n)]
#define rhs(m, n) rhs[(m) * edge + (n)]
#define res(m, n) res[(m) * edge + (n)]
__global__ void matmul_blocking(const float* lhs, const float* rhs, float* res, size_t edge) {
    // carve a block of shared memory for lhs and rhs
	__shared__ float shared_lhs[BLOCK_SIZE][BLOCK_SIZE];
	__shared__ float shared_rhs[BLOCK_SIZE][BLOCK_SIZE];
    // make copies of the thread location index
	const int block_x = blockIdx.x;
	const int block_y = blockIdx.y; 
	const int thread_x = threadIdx.x; 
	const int thread_y = threadIdx.y; 
	const int x = block_x * BLOCK_SIZE + thread_x;
	const int y = block_y * BLOCK_SIZE + thread_y;
    // temp holds the value to be assigned to an element in res
	float temp = 0.0f;
    // the i loop is used to jump to the next block
	for (int i = 0; i < edge; i += BLOCK_SIZE) { 
        // each thread is responsible for transfering 1 element from lhs and rhs to shared memory
		shared_lhs[thread_y][thread_x] = lhs(y, i + thread_x); 
		shared_rhs[thread_y][thread_x] = rhs(i + thread_y, x);
        // sync to ensure a block of threads have all completed the data transfer
		__syncthreads();
        // the j loop is used to accumulate the dot product for this block in temp
		for (int j = 0; j < BLOCK_SIZE; j++) {
			temp += shared_lhs[thread_y][j] * shared_rhs[j][thread_x];
		}
        // sync to ensure a block of threads have all have completed the accumulation in their own temps
		__syncthreads();
        // now a block is done
	}
	res(y, x) = temp;
}
#undef BLOCK_SIZE
#undef lhs(m, n)
#undef rhs(m, n)
#undef res(m, n)
```

> It only works for square matrices whose size is multiples of 32.

This tiling method enhances access pattern even more, because:

- It makes use of the high speed memory which is shared by threads within a block, enhancing locality. Now, each block of threads (32 * 32 in my implementation) can make use of the same shared memory for each of their calculation. 

  Yet in the traditional methods, threads that work together need to span many rows or columns, and **shared memory** is often too small to be used.

  ![image-20240602150832310](D:\Blue\C++\CS205-2024-Spring-Projects\Project5\report\CUDAMemoryModel.png)

  [David B. Kirk, Wen Mei W. Hwu]: https://safari.ethz.ch/architecture/fall2019/lib/exe/fetch.php?media=2013_programming_massively_parallel_processors_a_hands-on_approach_2nd.pdf	"p98"

- I made local copies of all the frequently used indices like `blockIdx.x` and `threadIdx.x`. These copies are `automatic variables` in CUDA, placed in the registers. Accessing these variables is extremely fast and parallel, so long as the register is is big enough to accommodate them.![image-20240602150219332](D:\Blue\C++\CS205-2024-Spring-Projects\Project5\report\CUDAVariableTypes.png)

[David B. Kirk, Wen Mei W. Hwu]: https://safari.ethz.ch/architecture/fall2019/lib/exe/fetch.php?media=2013_programming_massively_parallel_processors_a_hands-on_approach_2nd.pdf	"p102"

The implementation of this method is rather tricky (harder than the tiling method for CPU!), so I spent quite some time making the illustration in Excel. This illustration is complementary to the source codes above and is very helpful in understanding how blocking is done.



![image-20240602142429679](D:\Blue\C++\CS205-2024-Spring-Projects\Project5\report\sharedMemory.png)

#### 4. Tiling Inside Shared Memory

```c++
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
```

In prior implementations, each thread **computes only one element** in the resultant matrix.

This time, we let one thread **compute a column vector** (having 4 entries) in the resultant matrix. 

- This approach increases the workload of each thread, makes better use of their registers, and reduces the frequency of memory accesses.

- With more work per thread, the ratio of computation to memory accesses (the arithmetic intensity) increases. Higher arithmetic intensity means that each memory latency period is overlapped with more computation, effectively hiding the latency.

When we call this kernel in the host function, we must note that the blockDim.y has been divided by 4.

```c++
dim3 threadsPerBlock(32, 8, 1);
dim3 blocksPerGrid(CEIL(lhs.sharedData->data_rows, 32), CEIL(rhs.sharedData->data_cols, 32));
matmul_tiling_shared_memory_inside <<< blocksPerGrid, threadsPerBlock >>> (/* function arguments */);
```

It should also be noted that, while we could alternatively cut the blockDim.x by 3/4 and maintain the blockDim.y, in that case the multiplication would be significantly slower, potentially due to a worse access pattern.

## Tests & Verifications

10240 * 10240:

![image-20240602233629115](D:\Blue\C++\CS205-2024-Spring-Projects\Project5\report\10240.png)

5120 * 5120:

![image-20240602233743362](D:\Blue\C++\CS205-2024-Spring-Projects\Project5\report\5120.png)

2560 * 2560:

![image-20240602233947300](D:\Blue\C++\CS205-2024-Spring-Projects\Project5\report\2560.png)

OpenBLAS:

|      | 2560 * 2560 | 5120 * 5120 | 10240 * 10240 |
| ---- | ----------- | ----------- | ------------- |
| Time | 36 ms       | 275 ms      | 2176 ms       |

Conclusion:

CPU sucks at matrix multiplication. Even my CUDA implementation can beat it.

## Difficulty & Solutions

### Row-major & Column-major Matrices

I was confused with the results I got using cuBLAS' matrix multiplication function.

```c++
inline Matrix Matrix::matmul_cuBLAS(const Matrix& lhs, const Matrix& rhs) {
	/* ...some codes... */
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
		CUBLAS_OP_N, CUBLAS_OP_N,
		rhs.sharedData->data_cols,
		lhs.sharedData->data_rows,
		rhs.sharedData->data_rows,
		&alpha,
		rhs.sharedData->data_gpu,
		rhs.sharedData->data_cols,
		lhs.sharedData->data_gpu,
		lhs.sharedData->data_cols,
		&beta,
		result.sharedData->data_gpu,
		result.sharedData->data_cols) != CUBLAS_STATUS_SUCCESS) {
		throw std::runtime_error("Error multiplying matrices");
	}
	cublasDestroy(handle);
	/* ... some codes ... */
	return result;
}
```

The commented-out codes were the ones that I initially used, and they ended up producing wrong results. It even caused me to doubt the correctness of my other implementations of matrix multiplication.

After googling, I learned that cuBLAS assumes column-major matrices by default, yet every matrix I used was row-major.

![](D:\Blue\C++\CS205-2024-Spring-Projects\Project5\report\New Bitmap image.bmp)

[Cmglee]: https://commons.wikimedia.org/w/index.php?curid=65107030

This means every matrix I passed to the cublasSgemm function is interpreted as its transpose. And the resultant matrix is also transposed. 

When I tried to calculate A * B, it ended up giving me B * A.
$$
AB \rightarrow A^TB^T \rightarrow (A^TB^T)^T = BA
$$
To fix this, I only need to pass the matrices in reversed order, as shown in the source codes above.

### Using OpenBLAS on Visual Studio

While configuring OpenBLAS library via CMake on CLion is fairly easy, Visual Studio doesn't seem to use CMake by default.

Switching completely to CMake meant I had to configure the CUDA libraries again via CMake. So I didn't.

Manually configuring OpenBLAS in settings was truly a nightmare. It kept saying that it "cannot find libopenblas.lib" despite the fact that I had set the path correctly.

Finally I gave up and tested OpenBLAS' performance using my Matrix class in previous projects on CLion.

### Lessons Learned

Coding and writing report at the same time, helped me understand things better.

## References

[Simon Boehm]: https://siboehm.com/articles/22/CUDA-MMM
[David B. Kirk, Wen Mei W. Hwu]: https://safari.ethz.ch/architecture/fall2019/lib/exe/fetch.php?media=2013_programming_massively_parallel_processors_a_hands-on_approach_2nd.pdf
[University of California San Diego]: https://cseweb.ucsd.edu/classes/fa15/cse260-a/static/Lec09-slides.pdf

