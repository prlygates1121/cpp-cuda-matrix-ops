#ifndef PROJECT4_MATRIX_H
#define PROJECT4_MATRIX_H

#include <iostream>
#include <stdexcept>

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
        for (int i = 0; i < sharedData->rows * sharedData->cols; i++) {
            sharedData->data[i] = value;
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
        if (sharedData->rows != other.sharedData->rows || sharedData->cols != other.sharedData->cols) {
            throw std::invalid_argument("Matrix dimensions do not match");
        }
        for (int i = 0; i < sharedData->rows * sharedData->cols; i++) {
            if (sharedData->data[i] != other.sharedData->data[i]) {
                return false;
            }
        }
        return true;
    }

    Matrix<T> operator+(const Matrix<T> &other) const {
        if (sharedData->rows != other.sharedData->rows || sharedData->cols != other.sharedData->cols) {
            throw std::invalid_argument("Matrix dimensions do not match");
        }
        Matrix<T> res = create(sharedData->rows, sharedData->cols);
        for (int i = 0; i < sharedData->rows * sharedData->cols; i++) {
            res.sharedData->data[i] = sharedData->data[i] + other.sharedData->data[i];
        }
        return res;
    }
};

#endif //PROJECT4_MATRIX_H
