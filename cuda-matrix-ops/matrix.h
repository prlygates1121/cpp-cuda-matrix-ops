#pragma once

#include <stdexcept>
#include <iostream>
#include <iomanip>

//#include "cublas_v2.h"

class Matrix {

    class MatrixData {
    public:
        const size_t data_rows, data_cols;
        float* data_cpu;
		float* data_gpu;
        unsigned int refCount;
        MatrixData(size_t rows, size_t cols) :
            data_rows(rows), data_cols(cols), refCount(0), data_cpu(new float[rows * cols]) {
            if (cudaMalloc(&data_gpu, rows * cols * sizeof(float)) != cudaSuccess) {
				delete[] data_cpu;
				throw std::runtime_error("Failed to allocate memory on GPU");
            }
        }
        ~MatrixData() {
            delete[] data_cpu;
			cudaFree(data_gpu);
        }
    };

private:
    MatrixData* sharedData;

public:
    Matrix() : sharedData(nullptr) {}
    Matrix(size_t rows, size_t cols) {
        if (rows == 0 || cols == 0) {
            throw std::invalid_argument("Do not create a non-empty matrix with zero rows or columns");
        }
        sharedData = new MatrixData(rows, cols);
        sharedData->refCount++;
    }
	Matrix(size_t rows, size_t cols, float fill) : Matrix(rows, cols) {
		std::fill(sharedData->data_cpu, sharedData->data_cpu + rows * cols, fill);
	}
    Matrix(const Matrix& other) {
        if (other.sharedData != nullptr) {
            sharedData = other.sharedData;
			sharedData->refCount++;
        }
        else {
			sharedData = nullptr;
        }
    }
    ~Matrix() {
		if (sharedData != nullptr) {
			sharedData->refCount--;
			if (sharedData->refCount == 0) {
				delete sharedData;
			}
		}
	}

    inline Matrix clone() const {
		Matrix result(sharedData->data_rows, sharedData->data_cols);
		std::copy(sharedData->data_cpu, sharedData->data_cpu + sharedData->data_rows * sharedData->data_cols, result.sharedData->data_cpu);
		return result;
    }
    inline void detach() {
		if (sharedData->refCount == 1) {
			return;
		}
		*this = clone();
	}
    inline void resize(size_t newRows, size_t newCols, float fill = 0) {
        if (sharedData == nullptr) {
            throw std::invalid_argument("Matrix is empty");
        }
        if (newRows == sharedData->data_rows && newCols == sharedData->data_cols) {
            return;
        }
        Matrix resized(newRows, newCols);
        if (newRows > sharedData->data_rows || newCols > sharedData->data_cols) {
            std::fill(resized.sharedData->data_cpu, resized.sharedData->data_cpu + newRows * newCols, fill);
        }
        size_t minRows = std::min(sharedData->data_rows, newRows);
        size_t minCols = std::min(sharedData->data_cols, newCols);
        for (size_t r = 0; r < minRows; r++) {
			memcpy(resized.sharedData->data_cpu + r * newCols, sharedData->data_cpu + r * sharedData->data_cols, minCols * sizeof(float));
        }
		*this = resized;
    }

    inline Matrix& operator=(const Matrix& other) {
		if (sharedData != nullptr) {
			sharedData->refCount--;
			if (sharedData->refCount == 0) {
				delete sharedData;
			}
		}
		sharedData = other.sharedData;
        if (sharedData != nullptr) {
            sharedData->refCount++;
        }
		return *this;
    }

    inline bool operator==(const Matrix& other) const {
		if (sharedData == other.sharedData) {
			return true;
		}
		if (sharedData == nullptr || other.sharedData == nullptr) {
			return false;
		}
		if (sharedData->data_rows != other.sharedData->data_rows || sharedData->data_cols != other.sharedData->data_cols) {
			return false;
		}
		// compare data with epsilon
		for (size_t i = 0; i < sharedData->data_rows * sharedData->data_cols; i++) {
			if (fabs(sharedData->data_cpu[i] - other.sharedData->data_cpu[i]) > 1.f) {
				return false;
			}
		}
		return true;
	}
    inline bool operator!=(const Matrix& other) const {
		return !(*this == other);
    }
    inline static Matrix add_gpu(const Matrix& lhs, const Matrix& rhs);
    inline static Matrix add_cpu(const Matrix& lhs, const Matrix& rhs);
	inline static Matrix multiply_scalar_gpu(const Matrix& lhs, float scalar);
    inline static Matrix multiply_scalar_cpu(const Matrix& lhs, float scalar);
	inline static Matrix matmul_cpu_naive(const Matrix& lhs, const Matrix& rhs);
	inline static Matrix matmul_gpu_naive(const Matrix& lhs, const Matrix& rhs);
	inline static Matrix matmul_gpu_coalesce(const Matrix& lhs, const Matrix& rhs);
	inline static Matrix matmul_gpu_tiling_shared_memory(const Matrix& lhs, const Matrix& rhs);
	inline static Matrix matmul_gpu_tiling_shared_memory_inside(const Matrix& lhs, const Matrix& rhs);
	inline static Matrix matmul_cuBLAS(const Matrix& lhs, const Matrix& rhs);


    inline const float& operator()(size_t row, size_t col) const {
		if (row >= sharedData->data_rows || col >= sharedData->data_cols) {
			throw std::out_of_range("Matrix subscript out of range");
		}
		return sharedData->data_cpu[row * sharedData->data_cols + col];
    }
    inline float& operator()(size_t row, size_t col) {
		if (row >= sharedData->data_rows || col >= sharedData->data_cols) {
			throw std::out_of_range("Matrix subscript out of range");
		}
        return sharedData->data_cpu[row * sharedData->data_cols + col];
    }

    inline void fill(float value) {
		std::fill(sharedData->data_cpu, sharedData->data_cpu + sharedData->data_rows * sharedData->data_cols, value);
    }

	inline void fill_random(float min, float max) {
		srand(time(0));
		for (size_t i = 0; i < sharedData->data_rows * sharedData->data_cols; i++) {
			sharedData->data_cpu[i] = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
		}
	}

	inline static void printMaxDiff(const Matrix& lhs, const Matrix& rhs) {
		float maxDiff = 0;
		float lhsValue = lhs.sharedData->data_cpu[0];
		float rhsValue = rhs.sharedData->data_cpu[0];
		for (size_t i = 0; i < lhs.sharedData->data_rows * lhs.sharedData->data_cols; i++) {
			if (fabs(lhs.sharedData->data_cpu[i] - rhs.sharedData->data_cpu[i]) > maxDiff) {
				maxDiff = fabs(lhs.sharedData->data_cpu[i] - rhs.sharedData->data_cpu[i]);
				lhsValue = lhs.sharedData->data_cpu[i];
				rhsValue = rhs.sharedData->data_cpu[i];
			}
		}
		std::cout << "Max diff: " << maxDiff << std::endl;
		std::cout << "lhs: " << lhsValue << " rhs: " << rhsValue << std::endl;
	}

	inline static void verifyDimensions(const Matrix& lhs, const Matrix& rhs) {
		if (lhs.sharedData == nullptr || rhs.sharedData == nullptr) {
			throw std::invalid_argument("Matrix is empty");
		}
		std::cout << "lhs: " << lhs.sharedData->data_rows << "x" << lhs.sharedData->data_cols << std::endl;
		std::cout << "rhs: " << rhs.sharedData->data_rows << "x" << rhs.sharedData->data_cols << std::endl;
		return;
	}

    inline void show() {
		if (sharedData == nullptr) {
			throw std::invalid_argument("Matrix is empty");
		}
		for (size_t r = 0; r < sharedData->data_rows; r++) {
			for (size_t c = 0; c < sharedData->data_cols; c++) {
				// print with 2 decimal places
				std::cout << std::fixed << std::setprecision(2) << sharedData->data_cpu[r * sharedData->data_cols + c] << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
};