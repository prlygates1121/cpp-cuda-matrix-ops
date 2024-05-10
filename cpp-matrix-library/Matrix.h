#ifndef PROJECT4_MATRIX_H
#define PROJECT4_MATRIX_H

#include <iostream>
#include <stdexcept>

using namespace std;

template <typename T>
class MatrixData {
public:
    size_t data_rows, data_cols;
    T* data;
    int refCount;

    // initialize a new MatrixData with a given number of data_rows and columns
    // used only when a new matrix is created with create()
    MatrixData(size_t rows, size_t cols)
        : refCount(0), data_rows(rows), data_cols(cols), data(new T[rows * cols]) {};

    // initialize a new MatrixData with hard copy of another MatrixData
    MatrixData(const MatrixData<T> &other)
        : refCount(0), data_rows(other.data_rows), data_cols(other.data_cols), data(new T[data_rows * data_cols]) { // initialize refCount to 0
        std::copy(other.data, other.data + data_rows * data_cols, data); // hard copy the data
    }

    inline static void allocateData(size_t rows, size_t cols, MatrixData<T>** pMatrixData) {
        // allocate memory for the shared data
        try {
            *pMatrixData = new MatrixData<T>(rows, cols);
        } catch (const std::bad_alloc&) {
            throw std::runtime_error("Memory allocation failed");
        }
    }

    inline static void allocateData(size_t rows, size_t cols, T fill, MatrixData<T>** pMatrixData) {
        // allocate memory for the shared data
        try {
            *pMatrixData = new MatrixData<T>(rows, cols);
        } catch (const std::bad_alloc&) {
            throw std::runtime_error("Memory allocation failed");
        }
        // fill the matrix with the given value
        size_t length = (*pMatrixData)->data_rows * (*pMatrixData)->data_cols;
        T* data = (*pMatrixData)->data;
        for (size_t i = 0; i < length; i++) {
            *(data++) = fill;
        }
    }

    inline static void allocateDataHardCopy(const MatrixData<T> &other, MatrixData<T>** pMatrixData) {
        // allocate memory for the shared data
        try {
            *pMatrixData = new MatrixData<T>(other);
        } catch (const std::bad_alloc&) {
            throw std::runtime_error("Memory allocation failed");
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
    T* m_data;
    size_t m_rows, m_cols;
    bool isROI;
public:
    MatrixData<T>* getSharedData() const {
        return sharedData;
    }
private:
    // replace the sharedData with a new sharedData and decrement the refCount
    // delete the original sharedData, if refCount == 1;
    // used before replacing the sharedData with another
    inline void assign(MatrixData<T>* anotherData) {
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
        // if the original sharedData has a refCount > 1, duplicate it or its ROI
        if (sharedData->refCount > 1) { // decrement the refCount of the original sharedData, since it is not linked to the current Matrix anymore
            sharedData->refCount--;
            if (!isROI){
                MatrixData<T>::allocateDataHardCopy(*sharedData, &sharedData);
                sharedData->refCount++;
            } else {
                MatrixData<T> *newData;
                MatrixData<T>::allocateData(m_rows, m_cols, &newData);
                sharedData->refCount++;
                // copy the ROI data to the new sharedData
                T* data = newData->data;
                for (size_t r = 0; r < m_rows; r++) {
                    for (size_t c = 0; c < m_cols; c++) {
                        *(data++) = *(m_data + r * sharedData->data_cols + c);
                    }
                }
                // replace the sharedData with the new sharedData
                assign(newData);
                m_data = sharedData->data;
                isROI = false;
            }
        }
        // if the original sharedData has a refCount == 1, do nothing
    }
public:
    // constructor
    Matrix(size_t rows, size_t cols) {
        MatrixData<T>::allocateData(rows, cols, &sharedData);
        sharedData->refCount++;
        m_data = sharedData->data;
        m_rows = sharedData->data_rows;
        m_cols = sharedData->data_cols;
        isROI = false;
    };

    // constructor with filling
    Matrix(size_t rows, size_t cols, T fill) {
        MatrixData<T>::allocateData(rows, cols, fill, &sharedData);
        sharedData->refCount++;
        m_data = sharedData->data;
        m_rows = sharedData->data_rows;
        m_cols = sharedData->data_cols;
        isROI = false;
    };

    // copy constructor (sharing another matrix's sharedData)
    Matrix(const Matrix<T> &other) {
        sharedData = other.sharedData;
        sharedData->refCount++;
        m_data = other.m_data;
        m_rows = other.m_rows;
        m_cols = other.m_cols;
        isROI = other.isROI;
    }

    // constructor with ROI
    // select a region of interest (ROI) from a MatrixData and create a new Matrix with the ROI
    Matrix(MatrixData<T> *parentData, size_t rowStart, size_t colStart, size_t rowEnd, size_t colEnd) {
        if (rowStart >= rowEnd || colStart >= colEnd || rowEnd > parentData->data_rows || colEnd > parentData->data_cols) {
            throw std::invalid_argument("Invalid ROI");
        }
        sharedData = parentData;
        sharedData->refCount++;
        m_data = parentData->data + rowStart * parentData->data_cols + colStart;
        m_rows = rowEnd - rowStart;
        m_cols = colEnd - colStart;
        isROI = true;
    }

    inline void fill(T value) {
        detach();
        size_t length = m_rows * m_cols;
        if (!isROI) {
            // if the current Matrix is a full matrix
            T* data = sharedData->data;
            for (size_t i = 0; i < length; i++) {
                *(data++) = value;
            }
            return;
        } else {
            // if the current Matrix is a ROI
            T* data = m_data;
            for (size_t r = 0; r < m_rows; r++) {
                for (size_t c = 0; c < m_cols; c++) {
                    *(data + r * sharedData->data_cols + c) = value;
                }
            }
        }
    }

    // used to resize the matrix
    // create a new sharedData and replace the old one with it for this matrix
    // cancel the ROI property of the current Matrix, and set the new data as the full matrix
    inline void resize(size_t rows, size_t cols) {
        if (rows == sharedData->data_rows && cols == sharedData->data_cols) {
            return;
        }
        MatrixData<T>* newData;
        MatrixData<T>::allocateData(rows, cols, &newData);
        size_t minRows = std::min(sharedData->data_rows, rows);
        size_t minCols = std::min(sharedData->data_cols, cols);
        for (size_t r = 0; r < minRows; r++) {
            for (size_t c = 0; c < minCols; c++) {
                newData->data[r * cols + c] = sharedData->data[r * sharedData->data_cols + c];
            }
        }
        assign(newData);
        // cancel the ROI property
        m_data = newData->data;
        m_rows = sharedData->data_rows;
        m_cols = sharedData->data_cols;
        isROI = false;
    }

    inline void resize(size_t rows, size_t cols, T fill) {
        if (rows == sharedData->data_rows && cols == sharedData->data_cols) return;

        MatrixData<T>* newData;
        MatrixData<T>::allocateData(rows, cols, fill, &newData);

        size_t minRows = std::min(sharedData->data_rows, rows);
        size_t minCols = std::min(sharedData->data_cols, cols);
        for (size_t r = 0; r < minRows; r++) {
            for (size_t c = 0; c < minCols; c++) {
                newData->data[r * cols + c] = sharedData->data[r * sharedData->data_cols + c];
            }
        }

        assign(newData);
        // cancel the ROI property
        m_data = newData->data;
        m_rows = sharedData->data_rows;
        m_cols = sharedData->data_cols;
        isROI = false;
    }

    inline void printMatInfo() {
        cout << "------------------------" << endl;
        for (size_t r = 0; r < m_rows; r++) {
            for (size_t c = 0; c < m_cols; c++) {
                cout << (*this)(r, c) << " ";
            }
            cout << endl;
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
            assign(other.sharedData);
            m_data = other.m_data;
            m_rows = other.m_rows;
            m_cols = other.m_cols;
            isROI = other.isROI;
        }
        return *this;
    }

    inline bool operator==(const Matrix<T> &other) const {
        if (this == &other) {
            return true;
        }
        if (m_rows != other.m_rows || m_cols != other.m_cols) {
            return false;
        }
        for (size_t i = 0; i < sharedData->data_rows * sharedData->data_cols; i++) {
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
        if (m_rows != other.m_rows || m_cols != other.m_cols) {
            throw std::invalid_argument("Matrix dimensions do not match");
        }
        Matrix<T> res(m_rows, m_cols);
        if (!isROI && !other.isROI) {
            size_t length = sharedData->data_rows * sharedData->data_cols;
            const T* lhs = sharedData->data;
            const T* rhs = other.sharedData->data;
            T* data = res.sharedData->data;
            for (size_t i = 0; i < length; i++) {
                *(data++) = *(lhs++) + *(rhs++);
            }
        } else {
            for (size_t r = 0; r < m_rows; r++) {
                for (size_t c = 0; c < m_cols; c++) {
                    *(res.m_data + r * res.sharedData->data_cols + c) = *(m_data + r * sharedData->data_cols + c) + *(other.m_data + r * other.sharedData->data_cols + c);
                }
            }
        }
        return res;
    }

    inline Matrix<T> operator-(const Matrix<T> &other) const {
        if (m_rows != other.m_rows || m_cols != other.m_cols) {
            throw std::invalid_argument("Matrix dimensions do not match");
        }
        Matrix<T> res(m_rows, m_cols);
        if (!isROI && !other.isROI) {
            size_t length = sharedData->data_rows * sharedData->data_cols;
            const T* lhs = sharedData->data;
            const T* rhs = other.sharedData->data;
            T* data = res.sharedData->data;
            for (size_t i = 0; i < length; i++) {
                *(data++) = *(lhs++) - *(rhs++);
            }
        } else {
            for (size_t r = 0; r < m_rows; r++) {
                for (size_t c = 0; c < m_cols; c++) {
                    *(res.m_data + r * res.sharedData->data_cols + c) = *(m_data + r * sharedData->data_cols + c) - *(other.m_data + r * other.sharedData->data_cols + c);
                }
            }
        }
        return res;
    }

    inline Matrix<T> operator*(const Matrix<T> &other) const {
        if (m_cols != other.m_rows) {
            throw std::invalid_argument("Matrix dimensions are invalid for multiplication");
        }
        Matrix<T> res(m_rows, other.m_cols, 0);
        for (size_t r = 0; r < m_rows; r++) {
            for (size_t k = 0; k < m_cols; k++) {
                for (size_t c = 0; c < other.m_cols; c++) {
                    *(res.m_data + r * res.sharedData->data_cols + c) += *(m_data + r * sharedData->data_cols + k) * *(other.m_data + k * other.sharedData->data_cols + c);
                }
            }
        }
        return res;
    }

    // used to access the value of a matrix element
    inline const T& operator()(size_t row, size_t col) const {
        if (row >= m_rows || col >= m_cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return *(m_data + row * sharedData->data_cols + col);
    }

    // used to modify the value of a matrix element
    inline T& operator()(size_t row, size_t col) {
        if (row >= m_rows || col >= m_cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        detach();
        return *(m_data + row * sharedData->data_cols + col);
    }
};

#endif //PROJECT4_MATRIX_H
