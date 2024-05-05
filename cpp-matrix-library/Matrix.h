#ifndef PROJECT4_MATRIX_H
#define PROJECT4_MATRIX_H

#include <iostream>
#include <stdexcept>

#define CHECK_DIMENSIONS if (sharedData->rows != other.sharedData->rows || sharedData->cols != other.sharedData->rows) { \
    throw std::invalid_argument("Matrix dimensions do not match"); }

template <typename T>
class MatrixData {
public:
    size_t rows, cols;
    T* data;
    int refCount;
    MatrixData(size_t rows, size_t cols)
        : refCount(1), rows(rows), cols(cols), data(new T[rows * cols]) {};
    ~MatrixData() {
        delete[] data;
    }
};

template <typename T>
class Matrix {
private:
    MatrixData<T>* sharedData;
    explicit Matrix(MatrixData<T>* sharedData): sharedData(sharedData) {};
public:
    static Matrix<T> create(size_t rows, size_t cols) {
        MatrixData<T>* sharedData;
        try {
            sharedData = new MatrixData<T>(rows, cols);
        } catch (const std::bad_alloc&) {
            throw std::runtime_error("Memory allocation failed");
        }
        return Matrix<T>(sharedData);
    }

    void fill(T value) {
        size_t length = sharedData->rows * sharedData->cols;
        T* data = sharedData->data;
        for (int i = 0; i < length; i++) {
            *(data++) = value;
        }
    }

    ~Matrix(){
        if (--sharedData->refCount == 0) {
            delete sharedData;
        }
    }

    Matrix<T>& operator=(const Matrix<T> &other) {
        if (this != &other) {
            if (--sharedData->refCount == 0) {
                delete sharedData;
            }
            sharedData = other.sharedData;
            sharedData->refCount++;
        }
        return *this;
    }

    bool operator==(const Matrix<T> &other) {
        if (this == &other) {
            return true;
        }
        if (sharedData == other.sharedData) {
            return true;
        }
        if (sharedData->rows != other.sharedData->rows || sharedData->cols != other.sharedData->cols) {
            return false;
        }
        for (int i = 0; i < sharedData->rows * sharedData->cols; i++) {
            if (sharedData->data[i] != other.sharedData->data[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const Matrix<T> &other) {
        return !(operator==(other));
    }

    Matrix<T> operator+(const Matrix<T> &other) const {
        CHECK_DIMENSIONS
        Matrix<T> res = create(sharedData->rows, sharedData->cols);
        size_t length = sharedData->rows * sharedData->cols;
        const T* lhsData = sharedData->data;
        const T* rhsData = other.sharedData->data;
        T* resultData = res.sharedData->data;
        for (size_t i = 0; i < length; i++) {
            *(resultData++) = *(lhsData++) + *(rhsData++);
        }
        return res;
    }

    Matrix<T> operator-(const Matrix<T> &other) const {
        CHECK_DIMENSIONS
        Matrix<T> res = create(sharedData->rows, sharedData->cols);
        size_t length = sharedData->rows * sharedData->cols;
        const T* lhsData = sharedData->data;
        const T* rhsData = other.sharedData->data;
        T* resultData = res.sharedData->data;
        for (size_t i = 0; i < length; i++) {
            *(resultData++) = *(lhsData++) - *(rhsData++);
        }
        return res;
    }

    Matrix<T> operator*(const Matrix<T> &other) const {
        if (sharedData->cols != other.sharedData->rows) {
            throw std::invalid_argument("Matrix dimensions are invalid for multiplication");
        }
        Matrix<T> res = create(sharedData->rows, other.sharedData->cols);
        res.fill(0);
        for (size_t r = 0; r < sharedData->rows; r++) {
            for (size_t k = 0; k < sharedData->cols; k++) {
                for (size_t c = 0; c < other.sharedData->cols; c++) {
                    res.sharedData->data[r * res.sharedData->cols + c] +=
                            sharedData->data[r * sharedData->cols + k] * other.sharedData->data[k * other.sharedData->cols + c];
                }
            }
        }
        return res;
    }
};

#endif //PROJECT4_MATRIX_H
