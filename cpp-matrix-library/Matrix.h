#ifndef PROJECT4_MATRIX_H
#define PROJECT4_MATRIX_H

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <omp.h>

using namespace std;

#define CANCEL_ROI m_data = sharedData->data; m_rows = sharedData->data_rows; m_cols = sharedData->data_cols; isROI = false;
#define STEP sharedData->data_cols

template <typename T>
class Matrix {
private:
    template <typename U>
    class MatrixData {
    public:
        size_t data_rows, data_cols;
        U* data;
        unsigned int refCount;

        MatrixData() : refCount(0), data_rows(0), data_cols(0), data(nullptr) {};
        MatrixData(size_t rows, size_t cols) : refCount(0), data_rows(rows), data_cols(cols), data(new U[rows * cols]) {};
        MatrixData(const MatrixData<U> &other)
                : refCount(0), data_rows(other.data_rows), data_cols(other.data_cols), data(new U[data_rows * data_cols]) {
            std::copy(other.data, other.data + data_rows * data_cols, data);
        }

        inline static void allocateData(size_t rows, size_t cols, MatrixData<U>** pMatrixData) {
            try {
                *pMatrixData = new MatrixData<U>(rows, cols);
            } catch (const std::bad_alloc&) {
                throw std::runtime_error("Memory allocation failed");
            }
        }

        inline static void copyData(const MatrixData<U> &other, MatrixData<U>** pMatrixData) {
            try {
                *pMatrixData = new MatrixData<U>(other);
            } catch (const std::bad_alloc&) {
                throw std::runtime_error("Memory allocation failed");
            }
        }

        ~MatrixData() {
            delete[] data;
        }
    };

    MatrixData<T>* sharedData;
    T* m_data;
    size_t m_rows, m_cols;
    bool isROI;

    inline static void fill_data(MatrixData<T>* matrixData, T value) {
        size_t length = matrixData->data_rows * matrixData->data_cols;
        std::fill(matrixData->data, matrixData->data + length, value);
    }

public:
    // default constructor
    Matrix() {
        sharedData = new MatrixData<T>;
        sharedData->refCount++;
        CANCEL_ROI
    }

    // constructor with size
    Matrix(size_t rows, size_t cols) {
        MatrixData<T>::allocateData(rows, cols, &sharedData);
        sharedData->refCount++;
        CANCEL_ROI
    };

    // constructor with size and filling
    Matrix(size_t rows, size_t cols, T fill) {
        MatrixData<T>::allocateData(rows, cols, &sharedData);
        fill_data(sharedData, fill);
        sharedData->refCount++;
        CANCEL_ROI
    };

    // copy constructor
    Matrix(const Matrix<T> &other) {
        sharedData = other.sharedData;
        sharedData->refCount++;
        m_data = other.m_data;
        m_rows = other.m_rows;
        m_cols = other.m_cols;
        isROI = other.isROI;
    }

    explicit Matrix(const string &fileName) {
        ifstream inFile(fileName);
        if (inFile.is_open()) {
            inFile >> sharedData->data_rows >> sharedData->data_cols;
            MatrixData<T>::allocateData(sharedData->data_rows, sharedData->data_cols, &sharedData);
            size_t length = sharedData->data_rows * sharedData->data_cols;
            T* data = sharedData->data;
            for (size_t i = 0; i < length; i++) {
                inFile >> *(data++);
            }
            CANCEL_ROI
            inFile.close();
        } else {
            cout << "Unable to load the matrix" << endl;
        }
    }

    ~Matrix(){
        if (--sharedData->refCount == 0) {
            delete sharedData;
        }
    }

    [[nodiscard]] size_t getMCols() const {
        return m_cols;
    }

    [[nodiscard]] size_t getMRows() const {
        return m_rows;
    }

    // getSharedData
    [[nodiscard]] MatrixData<T>* getSharedData() const {
        return sharedData;
    }

    inline void detach() {
        if (sharedData->refCount > 1) {
            sharedData->refCount--;
            if (!isROI){
                MatrixData<T>::copyData(*sharedData, &sharedData);
                sharedData->refCount++;
            } else {
                Matrix<T> detached(m_rows, m_cols);
                T* data = detached.m_data;
                for (size_t r = 0; r < m_rows; r++) {
                    for (size_t c = 0; c < m_cols; c++) {
                        *(data++) = *(m_data + r * STEP + c);
                    }
                }
                *this = detached;
            }
//            cout << "refCount: " << sharedData->refCount << endl;
        }
    }

    inline Matrix<T> clone() {
        Matrix<T> newMatrix = (*this);
        newMatrix.detach();
        return newMatrix;
    }

    inline Matrix<T> getROI(size_t x, size_t y, size_t width, size_t height) {
//        if (isROI) {
//            throw std::invalid_argument("Cannot get ROI from a ROI matrix");
//        }
        if (width == 0 || height == 0 ||
            x + width > m_cols ||
            y + height > m_rows ||
            (x == 0 && y == 0 && width == m_cols && height == m_rows)) {
            throw std::invalid_argument("Invalid ROI range");
        }
        Matrix<T> roi = (*this);
        roi.m_data = m_data + y * STEP + x;
        roi.m_cols = width;
        roi.m_rows = height;
        roi.isROI = true;
        return roi;
    }

    inline void cancelROI() {
        if (!isROI) {
            return;
        } else {
            CANCEL_ROI
        }
    }

    inline void adjustROI(size_t x, size_t y, size_t width, size_t height) {
        if (!isROI) {
            throw std::invalid_argument("Cannot adjust ROI from a non-ROI matrix");
        }
        CANCEL_ROI
        *this = (*this).getROI(x, y, width, height);
    }

    inline void resize(size_t newRows, size_t newCols, T fill = 0) {
        if (newRows == sharedData->data_rows && newCols == sharedData->data_cols) {
            return;
        }
        Matrix<T> resized(newRows, newCols);
        if (newRows > sharedData->data_rows || newCols > sharedData->data_cols) {
            fill_data(resized.sharedData, fill);
        }
        size_t minRows = std::min(sharedData->data_rows, newRows);
        size_t minCols = std::min(sharedData->data_cols, newCols);
        for (size_t r = 0; r < minRows; r++) {
            for (size_t c = 0; c < minCols; c++) {
                *(resized.m_data + r * resized.STEP + c) = *(sharedData->data + r * STEP + c);
            }
        }
        *this = resized;
    }

    inline Matrix<T>& operator=(const Matrix<T> &other) {
        if (this != &other || sharedData != other.sharedData) {
            sharedData->refCount--;
            if (sharedData->refCount == 0) delete sharedData;
            sharedData = other.sharedData;
            sharedData->refCount++;
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
        for (int r = 0; r < m_rows; r++) {
            for (int c = 0; c < m_cols; c++) {
                if (*(m_data + r * STEP + c) != *(other.m_data + r * other.STEP + c)) {
                    return false;
                }
            }
        }
        return true;
    }

    inline bool operator!=(const Matrix<T> &other) const {
        return !(operator==(other));
    }

    inline Matrix<T> operator+(const Matrix<T> &other) const {
        if (m_rows != other.m_rows || m_cols != other.m_cols) {
            throw std::invalid_argument("Matrix dimensions do not match for addition");
        }
        Matrix<T> res(m_rows, m_cols);
        if (!isROI && !other.isROI) {
            size_t length = sharedData->data_rows * sharedData->data_cols;
//            const T* lhs = sharedData->data;
//            const T* rhs = other.sharedData->data;
//            T* data = res.sharedData->data;
#pragma omp parallel for
            for (size_t i = 0; i < length; i++) {
                res.sharedData->data[i] = sharedData->data[i] + other.sharedData->data[i];
            }
//            for (size_t i = 0; i < length; i++) {
//                *(data++) = *(lhs++) + *(rhs++);
//            }
        } else {
#pragma omp parallel for collapse(2)
            for (size_t r = 0; r < m_rows; r++) {
                for (size_t c = 0; c < m_cols; c++) {
                    *(res.m_data + r * res.STEP + c) = *(m_data + r * STEP + c) + *(other.m_data + r * other.STEP + c);
                }
            }
        }
        return res;
    }

    inline Matrix<T>& operator+=(const Matrix<T> &other) {
        *this = *this + other;
        return *this;
    }

    inline Matrix<T> operator-(const Matrix<T> &other) const {
        if (m_rows != other.m_rows || m_cols != other.m_cols) {
            throw std::invalid_argument("Matrix dimensions do not match for subtraction");
        }
        Matrix<T> res(m_rows, m_cols);
        if (!isROI && !other.isROI) {
            size_t length = sharedData->data_rows * sharedData->data_cols;
//            const T* lhs = sharedData->data;
//            const T* rhs = other.sharedData->data;
//            T* data = res.sharedData->data;
#pragma omp parallel for
            for (size_t i = 0; i < length; i++) {
                res.sharedData->data[i] = sharedData->data[i] - other.sharedData->data[i];
            }
        } else {
#pragma omp parallel for collapse(2)
            for (size_t r = 0; r < m_rows; r++) {
                for (size_t c = 0; c < m_cols; c++) {
                    *(res.m_data + r * res.STEP + c) = *(m_data + r * STEP + c) - *(other.m_data + r * other.STEP + c);
                }
            }
        }
        return res;
    }

    inline Matrix<T>& operator-=(const Matrix<T> &other) {
        *this = *this - other;
        return *this;
    }

    inline Matrix<T> operator*(const Matrix<T> &other) const {
        if (m_cols != other.m_rows) {
            throw std::invalid_argument("Matrix dimensions are invalid for multiplication");
        }
        if (m_rows >= 1024 && m_cols >= 1024 && other.m_cols >= 1024) {
//            cout << "Using block matrix multiplication" << endl;
#define BLOCK_SIZE 112
            const size_t padding_col = (m_cols % BLOCK_SIZE == 0) ? 0 : BLOCK_SIZE - m_cols % BLOCK_SIZE;
            const size_t padding_row = (m_rows % BLOCK_SIZE == 0) ? 0 : BLOCK_SIZE - m_rows % BLOCK_SIZE;
            const size_t padding_col_other = (other.m_cols % BLOCK_SIZE == 0) ? 0 : BLOCK_SIZE - other.m_cols % BLOCK_SIZE;
            const size_t padding_row_other = (other.m_rows % BLOCK_SIZE == 0) ? 0 : BLOCK_SIZE - other.m_rows % BLOCK_SIZE;

            Matrix<T> result(m_rows + padding_row, other.m_cols + padding_col_other, 0);
            Matrix<T> left_padded(m_rows + padding_row, m_cols + padding_col, 0);
            Matrix<T> right_padded(other.m_rows + padding_row_other, other.m_cols + padding_col_other, 0);

            for (size_t r = 0; r < m_rows; r++) {
                for (size_t c = 0; c < m_cols; c++) {
                    *(left_padded.m_data + r * left_padded.STEP + c) = *(m_data + r * STEP + c);
                }
            }
            for (size_t r = 0; r < other.m_rows; r++) {
                for (size_t c = 0; c < other.m_cols; c++) {
                    *(right_padded.m_data + r * right_padded.STEP + c) = *(other.m_data + r * other.STEP + c);
                }
            }
#pragma omp parallel for
            for (size_t block_r = 0; block_r < m_rows; block_r += BLOCK_SIZE) {
                for (size_t block_k = 0; block_k < m_cols; block_k += BLOCK_SIZE) {
                    for (size_t block_c = 0; block_c < other.m_cols; block_c += BLOCK_SIZE) {
                        for (size_t r = 0; r < BLOCK_SIZE; r++) {
                            for (size_t k = 0; k < BLOCK_SIZE; k++) {
                                for (size_t c = 0; c < BLOCK_SIZE; c++) {
                                    *(result.m_data + (block_r + r) * result.STEP + block_c + c) +=
                                            *(left_padded.m_data + (block_r + r) * left_padded.STEP + block_k + k) *
                                            *(right_padded.m_data + (block_k + k) * right_padded.STEP + block_c + c);
                                }
                            }
                        }
                    }
                }
            }
            result.resize(m_rows, other.m_cols);
            return result;
#undef BLOCK_SIZE
        } else {
            Matrix<T> result(m_rows, other.m_cols, 0);
#pragma omp parallel for
            for (size_t r = 0; r < m_rows; r++) {
                for (size_t k = 0; k < m_cols; k++) {
                    for (size_t c = 0; c < other.m_cols; c++) {
                        *(result.m_data + r * result.STEP + c) += *(m_data + r * STEP + k) * *(other.m_data + k * other.STEP + c);
                    }
                }
            }
            return result;
        }
    }

    inline Matrix<T>& operator*=(const Matrix<T> &other) {
        *this = *this * other;
        return *this;
    }

    inline Matrix<T> operator*(const int &scalar) const {
        Matrix<T> res(m_rows, m_cols);
        if (!isROI) {
            size_t length = sharedData->data_rows * sharedData->data_cols;
            const T* lhs = sharedData->data;
            T* data = res.sharedData->data;
            for (size_t i = 0; i < length; i++) {
                *(data++) = *(lhs++) * scalar;
            }
        } else {
            for (size_t r = 0; r < m_rows; r++) {
                for (size_t c = 0; c < m_cols; c++) {
                    *(res.m_data + r * res.STEP + c) = *(m_data + r * STEP + c) * scalar;
                }
            }
        }
        return res;
    }

    inline Matrix<T>& operator*=(const int &scalar) {
        *this = *this * scalar;
        return *this;
    }

    inline friend Matrix<T> operator*(const int &scalar, const Matrix<T> &matrix) {
        return matrix * scalar;
    }

    inline const T& operator()(size_t row, size_t col) const {
        if (row >= m_rows || col >= m_cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return *(m_data + row * STEP + col);
    }

    inline T& operator()(size_t row, size_t col) {
        if (row >= m_rows || col >= m_cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return *(m_data + row * STEP + col);
    }

    inline void save(const string &fileName) {
        ofstream outFile(fileName);
        if (outFile.is_open()) {
            outFile << m_rows << " " << m_cols << endl;
            for (size_t r = 0; r < m_rows; r++) {
                for (size_t c = 0; c < m_cols; c++) {
                    outFile << *(m_data + r * STEP + c) << " ";
                }
                outFile << endl;
            }
            outFile.close();
            cout << "Matrix saved to " << fileName << endl;
        } else {
            cout << "Error: Unable to save the matrix" << endl;
        }
    }

    inline Matrix<T> plain_multiply(const Matrix<T> &other) const {
        if (m_cols != other.m_rows) {
            throw std::invalid_argument("Matrix dimensions are invalid for multiplication");
        }
        Matrix<T> result(m_rows, other.m_cols, 0);
#pragma omp parallel for
        for (size_t r = 0; r < m_rows; r++) {
            for (size_t k = 0; k < m_cols; k++) {
                for (size_t c = 0; c < other.m_cols; c++) {
                    *(result.m_data + r * result.STEP + c) += *(m_data + r * STEP + k) * *(other.m_data + k * other.STEP + c);
                }
            }
        }
        return result;
    }

    inline void fill(T value) {
        if (!isROI)
            fill_data(sharedData, value);
        else
            for (size_t r = 0; r < m_rows; r++) {
                for (size_t c = 0; c < m_cols; c++) {
                    *(m_data + r * STEP + c) = value;
                }
            }
    }

    inline void show_detailed() {
        cout << "-----------MatrixData-----------" << endl;
        size_t length = sharedData->data_rows * sharedData->data_cols;
        for (size_t i = 0; i < length; i++) {
            cout << sharedData->data[i] << " ";
            if ((i + 1) % STEP == 0) cout << endl;
        }
        cout << "> data          = " << sharedData->data << endl;
        cout << "> size          = " << sharedData->data_rows << " x " << sharedData->data_cols << endl;
        cout << "> refCount      = " << sharedData->refCount << endl;
        cout << "-------------Matrix-------------" << endl;
        for (size_t r = 0; r < m_rows; r++) {
            for (size_t c = 0; c < m_cols; c++) {
                cout << (*this)(r, c) << " ";
            }
            cout << endl;
        }
        cout << "> m_data        = " << m_data << endl;
        cout << "> size          = " << m_rows << " x " << m_cols << endl;
        cout << "> ROI           = " << boolalpha << isROI << endl;
        cout << "--------------------------------" << endl;
    }

    inline void show() {
        for (size_t r = 0; r < m_rows; r++) {
            for (size_t c = 0; c < m_cols; c++) {
                cout << (*this)(r, c) << " ";
            }
            cout << endl;
        }
    }
};

#undef CANCEL_ROI
#undef STEP

#endif //PROJECT4_MATRIX_H