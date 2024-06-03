template <typename T>
template <typename U>
Matrix<T>::MatrixData<U>::MatrixData(size_t rows, size_t cols) :
    data_rows(rows), data_cols(cols), refCount(0), data(new U[rows * cols]) {
    cout << "MatrixData constructor called" << endl;
}

template <typename T>
template <typename U>
Matrix<T>::MatrixData<U>::~MatrixData() {
    cout << "MatrixData destructor called" << endl;
    delete[] data;
}

template <typename T>
Matrix<T>::Matrix() {
    MAKE_NONE
}

template <typename T>
Matrix<T>::Matrix(size_t rows, size_t cols) {
    if (rows == 0 || cols == 0) {
        throw std::invalid_argument("Do not create a non-empty matrix with zero rows or columns");
    }
    sharedData = new MatrixData<T>(rows, cols);
    sharedData->refCount++;
    MAKE_FULL
}

template <typename T>
Matrix<T>::Matrix(size_t rows, size_t cols, T fill) : Matrix(rows, cols) {
    std::fill(sharedData->data, sharedData->data + sharedData->data_rows * sharedData->data_cols, fill);
}

template <typename T>
Matrix<T>::Matrix(const Matrix<T>& other) {
    cout << "Matrix copy constructor called" << endl;
    if (other.sharedData != nullptr) {
        sharedData = other.sharedData;
        sharedData->refCount++;
        m_data = other.m_data;
        m_rows = other.m_rows;
        m_cols = other.m_cols;
        isROI = other.isROI;
    } else {
        MAKE_NONE
    }
}

template <typename T>
Matrix<T>::Matrix(const string& fileName) {
    ifstream inFile(fileName);
    if (inFile.is_open()) {
        size_t rows, cols;
        inFile >> rows >> cols;
        if (rows != 0 && cols != 0) {
            sharedData = new MatrixData<T>(rows, cols);
            sharedData->refCount++;
            size_t length = sharedData->data_rows * sharedData->data_cols;
            T* data = sharedData->data;
            for (size_t i = 0; i < length; i++) {
                inFile >> *(data++);
            }
            MAKE_FULL
        } else {
            MAKE_NONE
        }
        inFile.close();
    } else {
        cout << "Unable to load the matrix" << endl;
    }
}

template <typename T>
Matrix<T>::~Matrix() {
    cout << "Matrix destructor called" << endl;
    if (sharedData != nullptr && --sharedData->refCount == 0) {
        delete sharedData;
    }
}

template <typename T>
inline size_t Matrix<T>::getMCols() const {
    return m_cols;
}

template <typename T>
inline size_t Matrix<T>::getMRows() const {
    return m_rows;
}

template <typename T>
inline unsigned int Matrix<T>::getDataRefCount() const {
    return sharedData->refCount;
}

template <typename T>
inline void Matrix<T>::detach() {
    *this = clone();
}

template <typename T>
inline Matrix<T> Matrix<T>::clone() const{
    Matrix<T> newMatrix(m_rows, m_cols);
    if (!isROI) {
        memcpy(newMatrix.sharedData->data, sharedData->data, sharedData->data_rows * sharedData->data_cols * sizeof(T));
    } else {
        for (size_t r = 0; r < m_rows; r++) {
            memcpy(newMatrix.m_data + r * newMatrix.STEP, m_data + r * STEP, m_cols * sizeof(T));
        }
    }
    return newMatrix;
}

template <typename T>
inline Matrix<T> Matrix<T>::getROI(size_t x, size_t y, size_t width, size_t height) const{
    if (width == 0 || height == 0 ||
        x + width > m_cols || y + height > m_rows ||
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

template <typename T>
inline void Matrix<T>::cancelROI() {
    if (isROI) {
        MAKE_FULL
    }
}

template <typename T>
inline void Matrix<T>::adjustROI(size_t x, size_t y, size_t width, size_t height) {
    if (!isROI) {
        throw std::invalid_argument("Cannot adjust ROI from a non-ROI matrix");
    }
    MAKE_FULL
    *this = (*this).getROI(x, y, width, height);
}

template <typename T>
inline void Matrix<T>::resize(size_t newRows, size_t newCols, T fill) {
    if (sharedData == nullptr) {
        throw std::invalid_argument("Do not resize an empty matrix");
    }
    if (newRows == sharedData->data_rows && newCols == sharedData->data_cols) {
        return;
    }
    Matrix<T> resized(newRows, newCols);
    if (newRows > sharedData->data_rows || newCols > sharedData->data_cols) {
        std::fill(resized.sharedData->data, resized.sharedData->data + resized.sharedData->data_rows * resized.sharedData->data_cols, fill);
    }
    size_t minRows = std::min(sharedData->data_rows, newRows);
    size_t minCols = std::min(sharedData->data_cols, newCols);
    for (size_t r = 0; r < minRows; r++) {
        memcpy(resized.sharedData->data + r * resized.STEP, sharedData->data + r * STEP, minCols * sizeof(T));
    }
    *this = resized;
}

template <typename T>
inline Matrix<T>& Matrix<T>::operator=(const Matrix<T>& other) {
    cout << "Matrix copy assignment called" << endl;
    if (sharedData != nullptr) {
        sharedData->refCount--;
        if (sharedData->refCount == 0) delete sharedData;
    }
    if (other.sharedData == nullptr) {
        MAKE_NONE
    } else if (this != &other || sharedData != other.sharedData) {
        sharedData = other.sharedData;
        sharedData->refCount++;
        m_data = other.m_data;
        m_rows = other.m_rows;
        m_cols = other.m_cols;
        isROI = other.isROI;
    }
    return *this;
}

template <typename T>
inline bool Matrix<T>::operator==(const Matrix<T>& other) const {
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

template <typename T>
inline bool Matrix<T>::operator!=(const Matrix<T>& other) const {
    return !(operator==(other));
}

template <typename T>
inline Matrix<T> Matrix<T>::operator+(const Matrix<T>& other) const {
    if (m_rows != other.m_rows || m_cols != other.m_cols) {
        throw std::invalid_argument("Matrix dimensions do not match for addition");
    }
    Matrix<T> res(m_rows, m_cols);
    if (!isROI && !other.isROI) {
        size_t length = sharedData->data_rows * sharedData->data_cols;
#pragma omp parallel for
        for (size_t i = 0; i < length; i++) {
            res.sharedData->data[i] = sharedData->data[i] + other.sharedData->data[i];
        }
    } else {
#pragma omp parallel for collapse(2)
        for (size_t r = 0; r < m_rows; r++) {
            for (size_t c = 0; c < m_cols; c++) {
                *(res.m_data + r * res.STEP + c) = *(m_data + r * STEP + c) + *(other.m_data + r * other.STEP + c);
            }
        }
    }
    return res; // RVO, which is a compiler optimization that eliminates the temporary object.
    // Specifically, the object is constructed in the memory of the caller, and no copy is made.
}

template <typename T>
inline Matrix<T>& Matrix<T>::operator+=(const Matrix<T>& other) {
    *this = *this + other;
    return *this;
}

template <typename T>
inline Matrix<T> Matrix<T>::operator-(const Matrix<T>& other) const {
    if (m_rows != other.m_rows || m_cols != other.m_cols) {
        throw std::invalid_argument("Matrix dimensions do not match for subtraction");
    }
    Matrix<T> res(m_rows, m_cols);
    if (!isROI && !other.isROI) {
        size_t length = sharedData->data_rows * sharedData->data_cols;
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

template <typename T>
inline Matrix<T>& Matrix<T>::operator-=(const Matrix<T>& other) {
    *this = *this - other;
    return *this;
}

template <typename T>
inline Matrix<T> Matrix<T>::operator*(const Matrix<T>& other) const {
    if (m_cols != other.m_rows) {
        throw std::invalid_argument("Matrix dimensions are invalid for multiplication");
    }
    if (m_rows >= 1024 && m_cols >= 1024 && other.m_cols >= 1024) {
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

template <typename T>
inline Matrix<T>& Matrix<T>::operator*=(const Matrix<T>& other) {
    *this = *this * other;
    return *this;
}

template <typename T>
inline Matrix<T> Matrix<T>::operator*(const int& scalar) const {
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

template <typename T>
inline Matrix<T>& Matrix<T>::operator*=(const int& scalar) {
    *this = *this * scalar;
    return *this;
}

template <typename U>
inline Matrix<U> operator*(const int& scalar, const Matrix<U>& matrix) {
    return matrix * scalar;
}

template <typename T>
inline const T& Matrix<T>::operator()(size_t row, size_t col) const {
    if (row >= m_rows || col >= m_cols) {
        throw std::out_of_range("Matrix index out of range");
    }
    return *(m_data + row * STEP + col);
}

template <typename T>
inline T& Matrix<T>::operator()(size_t row, size_t col) {
    if (row >= m_rows || col >= m_cols) {
        throw std::out_of_range("Matrix index out of range");
    }
    return *(m_data + row * STEP + col);
}

template <typename T>
inline void Matrix<T>::save(const string& fileName) {
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



template <typename T>
inline void Matrix<T>::fill(T value) {
    if (sharedData == nullptr) {
        throw std::invalid_argument("Do not fill an empty matrix");
    }
    if (!isROI)
        std::fill(sharedData->data, sharedData->data + sharedData->data_rows * sharedData->data_cols, value);
    else
        for (size_t r = 0; r < m_rows; r++) {
            std::fill(m_data + r * STEP, m_data + r * STEP + m_cols, value);
        }
}

template <typename T>
void Matrix<T>::show_detailed() {
    if (sharedData == nullptr) {
        cout << "Do not print an empty matrix" << endl;
        return;
    }
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

template <typename T>
void Matrix<T>::show() {
    if (sharedData == nullptr) {
        cout << "Do not print an empty matrix" << endl;
        return;
    }
    for (size_t r = 0; r < m_rows; r++) {
        for (size_t c = 0; c < m_cols; c++) {
            cout << (*this)(r, c) << " ";
        }
        cout << endl;
    }
}

#undef MAKE_FULL
#undef STEP
