#ifndef PROJECT4_MATRIX_H
#define PROJECT4_MATRIX_H

#include <iostream>
#include <stdexcept>

using namespace std;

template <typename T>
class MatrixData {
public:
    size_t rows_data, cols_data;
    T* data;
    int refCount;

    // initialize a new MatrixData with a given number of rows_data and columns
    // used only when a new matrix is created with create()
    MatrixData(size_t rows, size_t cols)
        : refCount(0), rows_data(rows), cols_data(cols), data(new T[rows * cols]) {};

    // initialize a new MatrixData with hard copy of another MatrixData
    MatrixData(const MatrixData<T> &other)
        : refCount(0), rows_data(other.rows_data), cols_data(other.cols_data), data(new T[rows_data * cols_data]) { // initialize refCount to 0
        std::copy(other.data, other.data + rows_data * cols_data, data); // hard copy the data
    }

    inline static void createData(size_t rows, size_t cols, MatrixData<T>** pMatrixData) {
        // allocate memory for the shared data
        try {
            *pMatrixData = new MatrixData<T>(rows, cols);
        } catch (const std::bad_alloc&) {
            throw std::runtime_error("Memory allocation failed");
        }
    }

    inline static void createData(size_t rows, size_t cols, T fill, MatrixData<T>** pMatrixData) {
        // allocate memory for the shared data
        try {
            *pMatrixData = new MatrixData<T>(rows, cols);
        } catch (const std::bad_alloc&) {
            throw std::runtime_error("Memory allocation failed");
        }
        // fill the matrix with the given value
        size_t length = (*pMatrixData)->rows_data * (*pMatrixData)->cols_data;
        T* data = (*pMatrixData)->data;
        for (size_t i = 0; i < length; i++) {
            *(data++) = fill;
        }
    }

    ~MatrixData() {
        delete[] data;
    }
};

template <typename T>
class Matrix {
private:
    MatrixData<T>* sharedData;
    T* data_matrix;
    size_t rows_matrix, cols_matrix;
    bool isROI;
public:
    MatrixData<T>* getSharedData() const {
        return sharedData;
    }
private:
    // replace the sharedData with a new sharedData and decrement the refCount
    // delete the original sharedData, if refCount == 1;
    inline void assignData(MatrixData<T>* anotherData) {
        sharedData->refCount--;
        if (sharedData->refCount == 0) delete sharedData;
        sharedData = anotherData;
        sharedData->refCount++;
        cout << sharedData->refCount << endl;
    }

    // duplicate the sharedData (and later modify the copy) and decrement refCount,                     if refCount >= 1
    // do nothing (and later modify the original data directly),                                        if refCount == 1
    // used before modifying the sharedData
    inline void detach() {
        // if the original sharedData has a refCount > 1, duplicate it
        if (sharedData->refCount > 1) { // decrement the refCount of the original sharedData, since it is not linked to the current Matrix anymore
            sharedData->refCount--;
            if (!isROI){
                try {
                    // create a new sharedData with hard copy of the original sharedData, and set it as the sharedData of the current Matrix
                    sharedData = new MatrixData<T>(*sharedData);
                    sharedData->refCount++;
                } catch (const std::bad_alloc &) {
                    throw std::runtime_error("Memory allocation failed");
                }
            } else {
                // if the current Matrix is a ROI, create a new sharedData same as the ROI
                MatrixData<int> *newData = new MatrixData<T>(rows_matrix, cols_matrix);
                sharedData->refCount++;
                T* data = newData->data;
                for (size_t r = 0; r < rows_matrix; r++) {
                    for (size_t c = 0; c < cols_matrix; c++) {
                        *(data++) = *(data_matrix + r * sharedData->cols_data + c);
                    }
                }
                assignData(newData);
                data_matrix = sharedData->data;
                isROI = false;
            }
        }
        // if the original sharedData has a refCount == 1, do nothing
    }
public:
    // constructor
    Matrix(size_t rows, size_t cols) {
        MatrixData<T>::createData(rows, cols, &sharedData);
        sharedData->refCount++;
        data_matrix = sharedData->data;
        rows_matrix = sharedData->rows_data;
        cols_matrix = sharedData->cols_data;
        isROI = false;
    };

    // constructor with filling
    Matrix(size_t rows, size_t cols, T fill) {
        MatrixData<T>::createData(rows, cols, fill, &sharedData);
        sharedData->refCount++;
        data_matrix = sharedData->data;
        rows_matrix = sharedData->rows_data;
        cols_matrix = sharedData->cols_data;
        isROI = false;
    };

    // copy constructor (sharing another matrix's sharedData)
    Matrix(const Matrix<T> &other) {
        sharedData = other.sharedData;
        sharedData->refCount++;
        data_matrix = other.data_matrix;
        rows_matrix = other.rows_matrix;
        cols_matrix = other.cols_matrix;
        isROI = other.isROI;
    }

    // constructor with ROI
    // select a region of interest (ROI) from a MatrixData and create a new Matrix with the ROI
    Matrix(MatrixData<T> *parentData, size_t rowStart, size_t colStart, size_t rowEnd, size_t colEnd) {
        if (rowStart >= rowEnd || colStart >= colEnd || rowEnd > parentData->rows_data || colEnd > parentData->cols_data) {
            throw std::invalid_argument("Invalid ROI");
        }
        sharedData = parentData;
        sharedData->refCount++;
        data_matrix = parentData->data + rowStart * parentData->cols_data + colStart;
        rows_matrix = rowEnd - rowStart;
        cols_matrix = colEnd - colStart;
        isROI = true;
    }

    inline void fill(T value) {
        detach();
        size_t length = rows_matrix * cols_matrix;
        if (!isROI) {
            // if the current Matrix is a full matrix
            T* data = sharedData->data;
            for (size_t i = 0; i < length; i++) {
                *(data++) = value;
            }
            return;
        } else {
            // if the current Matrix is a ROI
            T* data = data_matrix;
            for (size_t r = 0; r < rows_matrix; r++) {
                for (size_t c = 0; c < cols_matrix; c++) {
                    *(data + r * sharedData->cols_data + c) = value;
                }
            }
        }
    }

    // used to resize the matrix
    // create a new sharedData and replace the old one with it for this matrix
    // cancel the ROI property of the current Matrix, and set the new data as the full matrix
    inline void resize(size_t rows, size_t cols) {
        if (rows == sharedData->rows_data && cols == sharedData->cols_data) {
            return;
        }
        MatrixData<T>* newData;
        MatrixData<T>::createData(rows, cols, &newData);
        size_t minRows = std::min(sharedData->rows_data, rows);
        size_t minCols = std::min(sharedData->cols_data, cols);
        for (size_t r = 0; r < minRows; r++) {
            for (size_t c = 0; c < minCols; c++) {
                newData->data[r * cols + c] = sharedData->data[r * sharedData->cols_data + c];
            }
        }
        assignData(newData);
        // cancel the ROI property
        data_matrix = newData->data;
        rows_matrix = sharedData->rows_data;
        cols_matrix = sharedData->cols_data;
        isROI = false;
    }

    inline void resize(size_t rows, size_t cols, T fill) {
        if (rows == sharedData->rows_data && cols == sharedData->cols_data) {
            return;
        }
        MatrixData<T>* newData;
        MatrixData<T>::createData(rows, cols, fill, &newData);
        size_t minRows = std::min(sharedData->rows_data, rows);
        size_t minCols = std::min(sharedData->cols_data, cols);
        for (size_t r = 0; r < minRows; r++) {
            for (size_t c = 0; c < minCols; c++) {
                newData->data[r * cols + c] = sharedData->data[r * sharedData->cols_data + c];
            }
        }
        assignData(newData);
        // cancel the ROI property
        data_matrix = newData->data;
        rows_matrix = sharedData->rows_data;
        cols_matrix = sharedData->cols_data;
        isROI = false;
    }

    inline void printMatInfo() {
        cout << "------------------------" << endl;
        for (size_t r = 0; r < rows_matrix; r++) {
            for (size_t c = 0; c < cols_matrix; c++) {
                std::cout << (*this)(r, c) << " ";
            }
            std::cout << endl;
        }
        cout << "> refCount = " << sharedData->refCount << endl;
        cout << "> data address = " << sharedData->data << endl;
        cout << "> ROI = " << boolalpha << isROI << endl;
        cout << "------------------------" << endl;
    }

    ~Matrix(){
        if (--sharedData->refCount == 0) {
            delete sharedData;
        }
    }

    // never preserve the original data, always use other's data
    inline Matrix<T>& operator=(const Matrix<T> &other) {
        if (this != &other || sharedData != other.sharedData) {
            assignData(other.sharedData);
        }
        data_matrix = other.data_matrix;
        rows_matrix = other.rows_matrix;
        cols_matrix = other.cols_matrix;
        isROI = other.isROI;
        return *this;
    }

    inline bool operator==(const Matrix<T> &other) const {
        if (this == &other) {
            return true;
        }
        if (rows_matrix != other.rows_matrix || cols_matrix != other.cols_matrix) {
            return false;
        }
        for (size_t i = 0; i < sharedData->rows_data * sharedData->cols_data; i++) {
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
        if (rows_matrix != other.rows_matrix || cols_matrix != other.cols_matrix) {
            throw std::invalid_argument("Matrix dimensions do not match");
        }
        Matrix<T> res(rows_matrix, cols_matrix);
        for (size_t r = 0; r < rows_matrix; r++) {
            for (size_t c = 0; c < cols_matrix; c++) {
                *(res.data_matrix + r * res.sharedData->cols_data + c) = *(data_matrix + r * sharedData->cols_data + c) + *(other.data_matrix + r * other.sharedData->cols_data + c);
                // print addition process
                // std::cout << (*this)(r, c) << " - " << other(r, c) << " = " << res(r, c) << std::endl;
            }
        }
        return res;
    }

    inline Matrix<T> operator-(const Matrix<T> &other) const {
        if (rows_matrix != other.rows_matrix || cols_matrix != other.cols_matrix) {
            throw std::invalid_argument("Matrix dimensions do not match");
        }
        Matrix<T> res(rows_matrix, cols_matrix);
        for (size_t r = 0; r < rows_matrix; r++) {
            for (size_t c = 0; c < cols_matrix; c++) {
                *(res.data_matrix + r * res.sharedData->cols_data + c) = *(data_matrix + r * sharedData->cols_data + c) - *(other.data_matrix + r * other.sharedData->cols_data + c);
                // print subtraction process
                // std::cout << (*this)(r, c) << " - " << other(r, c) << " = " << res(r, c) << std::endl;
            }
        }
        return res;
    }

    inline Matrix<T> operator*(const Matrix<T> &other) const {
        if (cols_matrix != other.rows_matrix) {
            throw std::invalid_argument("Matrix dimensions are invalid for multiplication");
        }
        Matrix<T> res(rows_matrix, other.cols_matrix, 0);
        for (size_t r = 0; r < rows_matrix; r++) {
            for (size_t k = 0; k < cols_matrix; k++) {
                for (size_t c = 0; c < other.cols_matrix; c++) {
                    *(res.data_matrix + r * res.sharedData->cols_data + c) += *(data_matrix + r * sharedData->cols_data + k) * *(other.data_matrix + k * other.sharedData->cols_data + c);
                }
                // print how the result is accumulated
                // std::cout << "res(" << r << ", " << c << ") = " << res(r, c) << std::endl;
            }
        }
        return res;
    }

    // used to access the value of a matrix element
    inline const T& operator()(size_t row, size_t col) const {
        if (row >= rows_matrix || col >= cols_matrix) {
            throw std::out_of_range("Matrix index out of range");
        }
        return *(data_matrix + row * sharedData->cols_data + col);
    }

    // used to modify the value of a matrix element
    inline T& operator()(size_t row, size_t col) {
        if (row >= rows_matrix || col >= cols_matrix) {
            throw std::out_of_range("Matrix index out of range");
        }
        detach();
        return *(data_matrix + row * sharedData->cols_data + col);
    }
};

#endif //PROJECT4_MATRIX_H
