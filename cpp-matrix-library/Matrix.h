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

    // initialize a new MatrixData with a given number of rows and columns
    MatrixData(size_t rows, size_t cols)
        : refCount(1), rows(rows), cols(cols), data(new T[rows * cols]) {}; // initialize refCount to 1

    // initialize a new MatrixData with hard copy of another MatrixData
    MatrixData(const MatrixData<T> &other)
        : refCount(0), rows(other.rows), cols(other.cols), data(new T[rows * cols]) { // initialize refCount to 0
        std::copy(other.data, other.data + rows * cols, data);
    }

    ~MatrixData() {
        delete[] data;
    }
};

template <typename T>
class Matrix {
private:
    MatrixData<T>* sharedData;

    // used to create a new Matrix with a sharedData
    explicit Matrix(MatrixData<T>* sharedData): sharedData(sharedData) {};

    // used to set the sharedData of the current Matrix
    void setMatrixData(MatrixData<T>* newData) {
        if (--sharedData->refCount == 0) { // if there are no more references to sharedData after decrementing
            delete sharedData;
        }
        sharedData = newData;
        sharedData->refCount++;
    }

    // used to duplicate the sharedData of the current Matrix
    void duplicateMatrixData() {
        try {
            setMatrixData(new MatrixData<T>(*sharedData));
        } catch (const std::bad_alloc&) {
            throw std::runtime_error("Memory allocation failed");
        }
    }
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
        if (sharedData->refCount > 1) { // if sharedData is shared
            duplicateMatrixData(); // copy on write
        }
        size_t length = sharedData->rows * sharedData->cols;
        T* data = sharedData->data;
        for (int i = 0; i < length; i++) {
            *(data++) = value;
        }
    }

    void printMatrix() {
        for (size_t i = 0; i < sharedData->rows; i++) {
            for (size_t j = 0; j < sharedData->cols; j++) {
                std::cout << sharedData->data[i * sharedData->cols + j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "refCount = " << sharedData->refCount << std::endl << std::endl;
    }

    ~Matrix(){
        if (--sharedData->refCount == 0) {
            delete sharedData;
        }
    }

    Matrix<T>& operator=(const Matrix<T> &other) {
        if (this != &other || sharedData != other.sharedData) {
            setMatrixData(other.sharedData);
        }
        return *this;
    }

    bool operator==(const Matrix<T> &other) const {
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

    bool operator!=(const Matrix<T> &other) const {
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

    // used to access the value of a matrix element
    const T& operator()(size_t row, size_t col) const{
        if (row >= sharedData->rows || col >= sharedData->cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return sharedData->data[row * sharedData->cols + col];
    }

    // used to modify the value of a matrix element
    T& operator()(size_t row, size_t col) {
        if (row >= sharedData->rows || col >= sharedData->cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        if (sharedData->refCount > 1) { // if sharedData is shared
            duplicateMatrixData(); // copy on write
        }
        return sharedData->data[row * sharedData->cols + col];
    }
};

#endif //PROJECT4_MATRIX_H
