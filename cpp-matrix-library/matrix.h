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

        MatrixData();
        MatrixData(size_t rows, size_t cols);
        MatrixData(const MatrixData<U>& other);

        inline static void allocateData(size_t rows, size_t cols, MatrixData<U>** pMatrixData);
        inline static void copyData(const MatrixData<U>& other, MatrixData<U>** pMatrixData);

        ~MatrixData();
    };

    MatrixData<T>* sharedData;
    T* m_data;
    size_t m_rows, m_cols;
    bool isROI;

public:
    Matrix();
    Matrix(size_t rows, size_t cols);
    Matrix(size_t rows, size_t cols, T fill);
    Matrix(const Matrix<T>& other);
    explicit Matrix(const string& fileName);
    ~Matrix();

    [[nodiscard]] size_t getMCols() const;
    [[nodiscard]] size_t getMRows() const;
    [[nodiscard]] MatrixData<T>* getSharedData() const;

    inline void detach();
    inline Matrix<T> clone();
    inline Matrix<T> getROI(size_t x, size_t y, size_t width, size_t height);
    inline void cancelROI();
    inline void adjustROI(size_t x, size_t y, size_t width, size_t height);
    inline void resize(size_t newRows, size_t newCols, T fill = 0);

    inline Matrix<T>& operator=(const Matrix<T>& other);
    inline bool operator==(const Matrix<T>& other) const;
    inline bool operator!=(const Matrix<T>& other) const;
    inline Matrix<T> operator+(const Matrix<T>& other) const;
    inline Matrix<T>& operator+=(const Matrix<T>& other);
    inline Matrix<T> operator-(const Matrix<T>& other) const;
    inline Matrix<T>& operator-=(const Matrix<T>& other);
    inline Matrix<T> operator*(const Matrix<T>& other) const;
    inline Matrix<T>& operator*=(const Matrix<T>& other);
    inline Matrix<T> operator*(const int& scalar) const;
    inline Matrix<T>& operator*=(const int& scalar);

    template <typename U>
    inline friend Matrix<U> operator*(const int& scalar, const Matrix<U>& matrix);

    inline const T& operator()(size_t row, size_t col) const;
    inline T& operator()(size_t row, size_t col);

    inline void save(const string& fileName);
    inline Matrix<T> plain_multiply(const Matrix<T>& other) const;
    inline void fill(T value);
    inline void show_detailed();
    inline void show();
};

#include "matrix.tpp"

#endif //PROJECT4_MATRIX_H
