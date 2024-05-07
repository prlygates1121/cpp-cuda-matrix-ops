#ifndef PROJECT4_MATRIX_H
#define PROJECT4_MATRIX_H

#include <iostream>
#include <stdexcept>

using namespace std;

template <typename T>
class MatrixData {
public:
    size_t rows, cols;
    T* data;
    int refCount;

    // initialize a new MatrixData with a given number of rows and columns
    // used only when a new matrix is created with create()
    MatrixData(size_t rows, size_t cols)
        : refCount(0), rows(rows), cols(cols), data(new T[rows * cols]) {};

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
public:
    MatrixData<T> *getSharedData() const {
        return sharedData;
    }

private:

    // private constructor
    explicit Matrix(MatrixData<T>* sharedData): sharedData(sharedData) {};

    // replace the sharedData of the current Matrix with a new sharedData
    // delete the original sharedData, if its refCount is now 0
    // used by the assignment operator
    inline void assignData(MatrixData<T>* anotherData) {
        sharedData->refCount--;
//        cout << getSharedData()->refCount << endl;
        if (sharedData->refCount == 0) delete sharedData;
        sharedData = anotherData;
        sharedData->refCount++;
    }

    // duplicate the sharedData, if the refCount of the original sharedData is greater than 1
    // used before modifying the data of the current Matrix
    inline void CoW() {
        // if the original sharedData has a refCount > 1, duplicate it
        if (sharedData->refCount > 1) {
            // decrement the refCount of the original sharedData, since it is not linked to the current Matrix anymore
            sharedData->refCount--;
            try {
                // create a new sharedData with hard copy of the original sharedData, and set it as the sharedData of the current Matrix
                sharedData = new MatrixData<T>(*sharedData);
                sharedData->refCount++;
            } catch (const std::bad_alloc&) {
                throw std::runtime_error("Memory allocation failed");
            }
        }
        // if the original sharedData has a refCount == 1, do nothing
    }
public:
    inline static Matrix<T> create(size_t rows, size_t cols) {
        MatrixData<T>* sharedData;
        try {
            sharedData = new MatrixData<T>(rows, cols);
        } catch (const std::bad_alloc&) {
            throw std::runtime_error("Memory allocation failed");
        }
        sharedData->refCount++;
        return Matrix<T>(sharedData);
    }

    inline static Matrix<T> create(size_t rows, size_t cols, T fill) {
        MatrixData<T>* sharedData;
        try {
            sharedData = new MatrixData<T>(rows, cols);
        } catch (const std::bad_alloc&) {
            throw std::runtime_error("Memory allocation failed");
        }
        sharedData->refCount++;
        size_t length = sharedData->rows * sharedData->cols;
        T* data = sharedData->data;
        for (size_t i = 0; i < length; i++) {
            *(data++) = fill;
        }
        return Matrix<T>(sharedData);
    }

    // copy constructor
    Matrix(const Matrix<T> &other): sharedData(other.sharedData) {
        sharedData->refCount++;
    }

    inline void fill(T value) {
        CoW();
        size_t length = sharedData->rows * sharedData->cols;
        T* data = sharedData->data;
        for (size_t i = 0; i < length; i++) {
            *(data++) = value;
        }
    }

    inline void resize(size_t rows, size_t cols) {
        if (rows == sharedData->rows && cols == sharedData->cols) {
            return;
        }
        MatrixData<T>* newData;
        try {
            newData = new MatrixData<T>(rows, cols);
        } catch (const std::bad_alloc& ) {
            throw std::runtime_error("Memory allocation failed");
        }
        size_t minRows = std::min(sharedData->rows, rows);
        size_t minCols = std::min(sharedData->cols, cols);
        for (size_t r = 0; r < minRows; r++) {
            for (size_t c = 0; c < minCols; c++) {
                newData->data[r * cols + c] = sharedData->data[r * sharedData->cols + c];
            }
        }
        assignData(newData);
    }

    inline void printMatrix() {
        for (size_t i = 0; i < sharedData->rows; i++) {
            for (size_t j = 0; j < sharedData->cols; j++) {
                std::cout << sharedData->data[i * sharedData->cols + j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "refCount = " << sharedData->refCount << std::endl;
        std::cout << "data address = " << sharedData->data << std::endl << std::endl;
    }

    ~Matrix(){
        if (--sharedData->refCount == 0) {
            delete sharedData;
        }
    }

    inline Matrix<T>& operator=(const Matrix<T> &other) {
        if (this != &other || sharedData != other.sharedData) {
            assignData(other.sharedData);
        }
        return *this;
    }

    inline bool operator==(const Matrix<T> &other) const {
        if (this == &other) {
            return true;
        }
        if (sharedData == other.sharedData) {
            return true;
        }
        if (sharedData->rows != other.sharedData->rows || sharedData->cols != other.sharedData->cols) {
            return false;
        }
        for (size_t i = 0; i < sharedData->rows * sharedData->cols; i++) {
            if (sharedData->data[i] != other.sharedData->data[i]) {
                return false;
            }
        }
        return true;
    }

    inline bool operator!=(const Matrix<T> &other) const {
        return !(operator==(other));
    }

    inline Matrix<T> operator+(const Matrix<T> &other) const {
        if (sharedData->rows != other.sharedData->rows || sharedData->cols != other.sharedData->rows) {
            throw std::invalid_argument("Matrix dimensions do not match");
        }
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

    inline Matrix<T> operator-(const Matrix<T> &other) const {
        if (sharedData->rows != other.sharedData->rows || sharedData->cols != other.sharedData->rows) {
            throw std::invalid_argument("Matrix dimensions do not match");
        }
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

    inline Matrix<T> operator*(const Matrix<T> &other) const {
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
    inline const T& operator()(size_t row, size_t col) const{
        if (row >= sharedData->rows || col >= sharedData->cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return sharedData->data[row * sharedData->cols + col];
    }

    // used to modify the value of a matrix element
    inline T& operator()(size_t row, size_t col) {
        if (row >= sharedData->rows || col >= sharedData->cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        CoW();
        return sharedData->data[row * sharedData->cols + col];
    }
};

#endif //PROJECT4_MATRIX_H
