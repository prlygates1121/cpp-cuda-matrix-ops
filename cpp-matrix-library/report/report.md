# Project4: A Class to Describe a Matrix

**Name**: 蓝一鸣

**SID**: 12312808

[TOC]

## Analysis

My class `Matrix` is able to:

- support various data types
- use soft copy or hard copy where needed
- manage memory properly via reference counting
- perform basic matrix operations via overloaded operators
- support ROI matrices flexibly
- read/write matrices from/to files

The project consists of 2 files:

- `matrix.h`, which contains the declaration of the class `Matrix` and its nested class `MatrixData`
- `matrix.tpp`, which contains the implementation
- `test.cpp`, which contains a comprehensive set of tests based on GoogleTest framework

Below is an overview of the project structure. 

- `MatrixData`, a nested class, represents a block of data, which **can be shared** among many `Matrix` instances
  - `data`: a pointer to a block of data
  - `data_rows` & `data_cols`: the size of the data
  - `refCount`: the number of instances of `Matrix` that is linked to this `MatrixData`
- `Matrix` represents a matrix
  - `sharedData`: a pointer to an instance of `MatrixData` 
  - (ROI) `m_data` : the starting point of the matrix
  - (ROI) `m_rows` & `m_cols` : the size of the matrix
  - (ROI) `isROI` : whether the matrix is a Region Of Interest

```C++
template <typename T>
class Matrix {

    template <typename U>
    class MatrixData {
    public:
        const size_t data_rows, data_cols;
        U* data;
        unsigned int refCount;
        MatrixData(size_t rows, size_t cols);
        ~MatrixData();
    };

private:
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

    // other functions ...
};

```

Global macros:

```c++
// used to cancel the ROI properties for a matrix instance, i.e. make it FULL
#define MAKE_FULL m_data = sharedData->data; m_rows = sharedData->data_rows; m_cols = sharedData->data_cols; isROI = false;

// used to make a matrix empty
#define MAKE_NONE sharedData = nullptr; m_data = nullptr; m_rows = 0; m_cols = 0; isROI = false;

// used when accessing matrix elements
/* When you want to jump across rows in a matrix, you will always need to use "(matrix).sharedData->data_cols" as a multiplier. Using a macro for it distinguishes it from other uses. */
#define STEP sharedData->data_cols
```

## Codes & Implementation

### Matrix Construction

**For MatrixData:**

Below is the constructor of `MatrixData`:

- It is the **only** constructor of `MatrixData`.
- It will **only** be called inside constructors of `Matrix`.

```c++
MatrixData(size_t rows, size_t cols) :
    data_rows(rows), data_cols(cols), refCount(0), data(new U[rows * cols]) {}
```

---

**For Matrix:**

There are 5 constructors for `Matrix`, including a copy constructor and a constructor that reads from a file. Except the copy constructor, all of them **must** call the constructor of  `MatrixData` to initialize a matrix.

1. The default constructor:
   - It uses `MAKE_NONE` macro to create an empty matrix.

   > Note: A matrix constructed with the default constructor is called an **empty** **matrix**, which **is not allowed in matrix operations**, as will be shown later.
   >
   > ```c++
   > Matrix<int> nothing; // an "empty" matrix
   > ```

```c++
Matrix() {
    MAKE_NONE 
    // sharedData = nullptr;
    // m_data = nullptr;
    // m_rows = 0;
    // m_cols = 0;
    // isROI = false;
}
```

2. The constructor specifying a matrix's rows and columns:

   > Note: This constructor is frequently used in matrix operations. **It can only be used to create a non-empty matrix** (with positive rows and columns), **otherwise an exception is thrown**. Empty matrices can only be created through the default constructor.  

```c++
Matrix(size_t rows, size_t cols) {
    if (rows == 0 || cols == 0) {
        throw std::invalid_argument("Cannot create a non-empty matrix with zero rows or columns");
    }
    sharedData = new MatrixData<T>(rows, cols);
    sharedData->refCount++;
    MAKE_FULL
}
```

3. The constructor delegating the last constructor, and filling it with a given value:

```c++
Matrix(size_t rows, size_t cols, T fill) : Matrix(rows, cols) {
    std::fill(sharedData->data, sharedData->data + sharedData->data_rows * sharedData->data_cols, fill);
}
```

4. The constructor that reads from a file:
   - Empty matrices are saved with only its dimensions (0 0), so this constructor can construct an empty matrix accordingly using `MAKE_NULL` macro.


```c++
Matrix(const string& fileName) {
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
```

5. The copy constructor:
   - It replaces the pointer `sharedData` of the current matrix with that of another matrix, which means this matrix is going to share the same `MatrixData` with another matrix, i.e. soft copy.
   - It increments the `refCount` of the new `sharedData` by one.
   - It copies ROI properties from the other matrix to this one.
   - It constructs an empty matrix when the other matrix is empty.


```c++
Matrix<T>::Matrix(const Matrix<T>& other) {
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
```

---

### Matrix Destruction

`Matrix` destructor:

- It decrements the `refCount` of its `sharedData`. 

- If the matrix is not an empty matrix, and if `refCount` decreases to 0, then it calls the destructor of the `MatrixData` that `sharedData` points to.

```c++
~Matrix() {
    if (sharedData != nullptr && --sharedData->refCount == 0) {
        delete sharedData;
    }
}
```

`MatrixData` destructor:

- The `data` pointer that it holds is deleted immediately.

```c++
~MatrixData() {
    delete[] data;
}
```

---

### Copy Assignment

Below is the copy assignment operator,

- If the other matrix is empty, it makes this matrix empty and return immediately.
- If this matrix is empty, it 

- Just like copy constructor, it replaces the pointer `sharedData` with a new one, increments `refCount` and copies ROI properties.
- In addition, it must also take care of the original pointer by decrementing the `MatrixData` it points to and deleting it when needed. 

```c++
inline Matrix<T>& operator=(const Matrix<T>& other) {
    if (other.sharedData == nullptr) {
        MAKE_NONE
    } else if (this != &other || sharedData != other.sharedData) {
        if (sharedData != nullptr) {
            sharedData->refCount--;
            if (sharedData->refCount == 0) delete sharedData;
        }
        sharedData = other.sharedData;
        sharedData->refCount++;
        m_data = other.m_data;
        m_rows = other.m_rows;
        m_cols = other.m_cols;
        isROI = other.isROI;
    }
    return *this;
}
```

---

**Note: By now, we have covered all the functions directly responsible for matrix creation, reference counting or memory clean-up. All the functions below simply rely on them and thus do not need to take care of these issues themselves, which makes the code more logical and easier to debug.**

---

### Region Of Interest (ROI)

Via ROI, we can get new matrices that share the same `MatrixData` with its mother matrix but with a different starting point `m_data` and sizes `m_cols` , `m_rows`. Changes made on the ROI will also apply to the mother matrix (except when detached manually).

Below is the function `getROI`:

- It creates a copy (`roi`) of this matrix via copy constructor.
- It moves its starting point `m_data` to the desired coordinate `(x, y)`.
- It sets its `m_cols` and `m_rows` according to the width and height of the ROI.
- It toggles `isROI` to true.
- It returns the copy `roi`.

> Note: If you call `getROI` on an existing ROI matrix, it still works but the function parameters (x, y, width, length) will be subjected to the size and starting point of the original ROI.
>
> ```c++
> // a 50 * 50 matrix
> Matrix<int> mother(50, 50); 
> // get a submatrix from mother, which starts at (2, 0) in mother's coordinate
> Matrix<int> roi = mother.getROI(2, 0, 15, 15); 
> // get a submatrix from itself, which starts at (1, 3) in ROI's coordinate, i.e. (3, 3) in mother's coordinate
> roi = roi.getROI(1, 3, 2, 2);
> ```

```c++
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
```

Below is the function `cancelROI`:

- If this matrix is a ROI, it clears all its ROI properties. This means the matrix will have full access to the whole `MatrixData` that it holds.

```c++
inline void cancelROI() {
    if (isROI) {
        MAKE_FULL
    }
}
```

Below is the function `adjustROI`:

- It first cancels the ROI property of the matrix, setting its starting point the start of `MatrixData`.

- It then calls `getROI` on this matrix and updates itself via copy assignment.

```C++
inline void adjustROI(size_t x, size_t y, size_t width, size_t height) {
    if (!isROI) {
        throw std::invalid_argument("Cannot adjust ROI from a non-ROI matrix");
    }
    MAKE_FULL
    *this = (*this).getROI(x, y, width, height);
}
```

These ROI matrices support matrix opperations just as normal matrices do, as will be shown later.

---

### Matrix Detachment & Cloning

When a matrix shares `MatrixData` with another matrix, whether as a ROI or as a soft copy, we call this matrix attachment. Changes made on the submatrix also applies to the mother matrix.

However, there exist scenarios where we want to detach such a submatrix, that is, making it independent with its own `MatrixData`.  After this, it will not be able to modify the original `MatrixData` anymore.

Below is the function `clone`:

- It creates a new matrix with a hard copy of the original matrix data and returns it.
- If the matrix is not a ROI, which has a contagious memory pattern, it uses a `memcpy` to copy the whole block of matrix.
- If the matrix is a ROI, it uses multiple `memcpy` to copy the matrix row by row.

```c++
inline Matrix<T> clone() const{
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
```

Below is the function `detach`:

- It simply calls `clone` on itself.
- Typically, detachment implies that the `refCount` of the original `MatrixData` must be decremented. However, we do not have to take care of that here ourselves: the copy assignment operator has done all the job!

```c++
inline void detach() {
    *this = clone();
}
```

> Note: While functions `detach` and `clone` allow the user to detach a matrix explicitly, there remain other implicit ways to make a matrix independent.
>
> - You are detaching a submatrix (a ROI or a soft copy of another matrix) whenever you assign another matrix to it via assignment operator (often used in matrix arithmetic).
>
>   ```c++
>   Matrix<int> mother(10, 10, 2);
>   Matrix<int> motherROI = mother.getROI(2, 2, 5, 5); // attach
>   motherROI = motherROI * 2; // detach implicitly
>   Matrix<int> motherSoftCopy = mother; // attach
>   motherSoftCopy += motherSoftCopy; // detach implicitly
>   ```

---

### Matrix Resizing

When we resize a matrix, we replace it with another matrix instance that has a different size.

Below is the function `resize`:

- Resizing an empty matrix is not allowed. You should first assign a non-empty matrix to it.

- It fills the new matrix with a certain value, if there are uninitialized new entries, with the default filling being 0.

- It fills the original data to the same location of the new matrix, where possible.
- It updates itself via copy assignment.

```c++
inline void resize(size_t newRows, size_t newCols, T fill) {
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
```

---

### Matrix Arithmetic

> Note: Empty matrices are forbidden to get involved in matrix arithmetic. Here's how we do it:
>
> 1. **No empty & empty:**
>
>    All functions for matrix arithmetic involves the creation of the resultant matrix, which calls the constructor that allows only positive rows and columns. 
>
>    Therefore, any attempt to perform arithmetic operations on 2 empty matrices will be rejected and an exception will be thrown.
>
> 2. **No empty & non-empty**
>
>    This is simply prevented via the dimension check.

Below is the overloaded operator + for matrix addition:

- If the matrix is not a ROI, which has a contagious memory pattern, it uses one loop to interate the pointer throughout the matrix.
- If the matrix is a ROI, it uses a nested loop to jump to the nextline where necessary.
- OpenMP is used to speed up this process.

```c++
inline Matrix<T> operator+(const Matrix<T>& other) const {
    if (m_rows != other.m_rows || m_cols != other.m_cols) {
        throw std::invalid_argument("Matrix dimensions do not match for addition");
    }
    Matrix<T> res(m_rows, m_cols); // detects empty matrices here
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
    return res;
}
```

Below is the overloaded operator * for matrix multiplication:

- It uses *OpenMP*, *loop interchanging* and *blocking* to speed up the multiplication.

  (The compiler can make use of intrinsics skillfully so I didn't implement them myself)

- To accomodate various matrix sizes, it pads the matrices to fit them into the blocks before calculating.

```c++
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
```

Below is the overloaded operator * for scalar multiplication:

```c++
inline Matrix<T> operator*(const int& scalar) const {
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
```

Below is the overload operator * as a friend function for reversed scalar multiplication:

```c++
inline friend Matrix<T> operator*(const int &scalar, const Matrix<T> &matrix) {
    return matrix * scalar;
}
```

Other operators including -, -=, +=, *= are omitted here for brevity.

---

### Matrix Element Access

Operator () is used for accessing element from a matrix:

> Note: In most functions within this project, I used direct pointer access instead of these operators because:
>
> - They have inherent boundary checks, adding to the overhead.
> - Sometimes they do not get inlined as expected, again adding to the overhead.

```c++
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
```

---

### Matrix Display

Below is the function `show_detailed`:

- It prints all the information of a matrix and the `MatrixData` that it holds. They have the same content when the matrix is not a ROI.
- In particular, it informs the user when the matrix is empty.

```c++
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
```

Below is the function `show`:

- It prints the matrix instead of the whole `MatrixData`.

```c++
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
```

## Test & Verifications

### Matrix Effectiveness Test

Please refer to `test.cpp`. It contains **> 300 lines of code** that test the correctness of basic matrix operations and proper exception throwing. It is based on GoogleTest framework.

A *test fixture* is used here to create a test object of Matrix (named "matrix") filled with different values, for every individual test.

```C++
class MatrixEffectivenessTest : public ::testing::Test {
protected:
    Matrix<int> matrix;
    void SetUp() override {
        matrix = Matrix<int>(EDGE, EDGE, EDGE);
        int value = 0;
        for (int i = 0; i < EDGE; ++i) {
            for (int j = 0; j < EDGE; ++j) {
                matrix(i, j) = value++;
            }
        }
    }
};
```

This is a test that specifically tests the empty matrices:

```c++
// Test empty matrices
TEST_F(MatrixEffectivenessTest, EmptyMatrixValidation) {
    Matrix<int> empty;
    // no addition for empty
    EXPECT_THROW(empty = empty + empty, std::invalid_argument);
    // noa addition for empty and non-empty
    EXPECT_THROW(empty = empty + matrix, std::invalid_argument);
    // no multiplication for empty
    EXPECT_THROW(empty = empty * empty, std::invalid_argument);
    // no scalar multiplication for empty
    EXPECT_THROW(empty = empty * 2, std::invalid_argument);
    // no reversed scalar multiplication for empty
    EXPECT_THROW(empty = 2 * empty, std::invalid_argument);
    // no clone/detach for empty
    EXPECT_THROW(empty.detach(), std::invalid_argument);
    EXPECT_THROW(matrix = empty.clone(), std::invalid_argument);
    // no resize for empty
    EXPECT_THROW(empty.resize(3, 3), std::invalid_argument);
    // no element access for empty
    EXPECT_THROW(empty(0, 0), std::out_of_range);
    // compare empty and another empty
    Matrix<int> empty2;
    EXPECT_EQ(empty, empty2);
    // compare empty and non-empty
    EXPECT_NE(empty, matrix);
    // construct empty via copy constructor
    Matrix<int> matrix1(empty);
    EXPECT_EQ(matrix1, empty);
    // assign empty to empty
    Matrix<int> empty3;
    empty3 = empty;
    EXPECT_EQ(empty3, empty);
    // assign non-empty to empty
    Matrix<int> empty4;
    empty4 = matrix;
    EXPECT_EQ(empty4, matrix);
    // assign empty to non-empty
    Matrix<int> matrix2(3, 3, 3);
    matrix2 = empty;
    EXPECT_EQ(matrix2, empty);
    // no roi for empty
    Matrix<int> roi;
    EXPECT_THROW(roi = empty.getROI(2, 2, 4, 4), std::invalid_argument);
    // no fill for empty
    EXPECT_THROW(empty.fill(4), std::invalid_argument);
    // save empty
    empty.save("GOOD.txt");
    // load empty
    Matrix<int> empty5("GOOD.txt");
    EXPECT_EQ(empty, empty5);
}
```

Here's another one that tests consecutive ROI operations on the same matrix:

```c++
// Test consecutive ROI operations to check data consistency
TEST_F(MatrixEffectivenessTest, ConsecutiveROIOperations) {
    Matrix<int> roi = matrix.getROI(1, 1, 8, 8); // ROI on mother 
    roi = roi.getROI(1, 1, 6, 6); // ROI on ROI
    EXPECT_EQ(roi.getDataRefCount(), 2);
    EXPECT_EQ(roi(0, 0), matrix(2, 2));
    roi = roi.getROI(1, 1, 4, 4); // ROI on ROI
    EXPECT_EQ(roi.getDataRefCount(), 2);
    EXPECT_EQ(roi(1, 1), matrix(4, 4));
    Matrix<int> roi2 = roi.getROI(1, 1, 2, 2); // ROI on ROI
    EXPECT_EQ(roi2.getDataRefCount(), 3);
    EXPECT_EQ(roi2(1, 1), matrix(5, 5));
}
```

---

### Blocked Matrix Multiplication Test

Since blocked matrix multiplication is harder to implement. I conducted extra tests on them to test correctness and performance.

They are stand-alone cpp files and do not rely on GoogleTest.

They are not uploaded.

#### Correctness

It compares the result of m * n computed by blocked method and plain method.

> Note: The function `plain_multiply` is removed in the final version of the project.

```c++
#include "matrix.h"
#include <chrono>
#include <cstdlib>
#define r 3333
#define c 3333
#define k 3333
int main() {
    srand(time(nullptr));
    Matrix<float> m(r, k);
    for (int i = 0; i < r; i++)
        for (int j = 0; j < k; j++)
            m(i, j) = (float)rand() / RAND_MAX;
    Matrix<float> n(k, c);
    for (int i = 0; i < k; i++)
        for (int j = 0; j < c; j++)
            n(i, j) = (float)rand() / RAND_MAX;
    Matrix<float> result1;
    Matrix<float> result2;
    result2 = m.plain_multiply(n);
    result1 = m * n;
    if (result1 == result2)
        std::cout << "Correctness test passed!" << std::endl;
    else
        std::cout << "Correctness test failed!" << std::endl;
}
```

![](D:\Blue\C++\CS205-2024-Spring-Projects\Project4\report\correctness.bmp)

#### Performance

Finishes multiplication of matrices > 5k * 5k magnitude within 1 second, roughly 10~20% slower than the implementation in C with Matrix struct.

```c++
#include "matrix.h"
#include <chrono>
#include <cmath>
#define r 6231
#define c 5291
#define k 4567
int main() {
    Matrix<float> m(r, k, 4.2);
    for (int i = 0; i < k - 1; i++)
        m(i, i) = 680.48;
    Matrix<float> n(k, c, 3.2);
    Matrix<float> result;
    for (int i = 0; i < 100; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        result = m * n;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "Time to multiply: " << diff.count() << " s\n";
    }
}
```

<img src="D:\Blue\C++\CS205-2024-Spring-Projects\Project4\report\performance.bmp" style="zoom: 67%;" />

---

### Memory Check

A part of `test.cpp` is rewritten and run without GoogleTest via Valgrind in Linux to find memory leaks.

No exception is meant to be thrown. No OpenMP enabled to prevent incompatibility with Valgrind.

**Test 1** (about addition & blocked multiplication ...) :

![](D:\Blue\C++\CS205-2024-Spring-Projects\Project4\report\valgrind1.bmp)

**Test 2** (about copy assignment, copy constructor, ROI, resize, detach, clone ...) :

![](D:\Blue\C++\CS205-2024-Spring-Projects\Project4\report\valgrind2.bmp)

## Difficulty & Solutions

### Template Syntax in C++

Syntax regarding templates in C++ is ~~driving me crazy~~ interesting.

1. **Separation of class declaration and definition**

   For template classes, the compiler needs to have access to the full definition of the class template in order to instantiate it for specific types. This is because templates are not actual code until they are instantiated with specific types. 

   My implementation is as follows:

   In `matrix.h`:

   ```c++
   #ifndef PROJECT4_MATRIX_H
   #define PROJECT4_MATRIX_H
   
   // class declarations ...
   
   #include "matrix.tpp" // including the definitions of functions
   #endif
   ```

   In `matrix.tpp`:

   ```c++
   // class definitions ...
   ```

2. **Nested class**

   Since my project contains a nested class `MatrixData`, its templated declaration and definition is somewhat different.

   Declaration:

   ```c++
   template <typename T>
   class Matrix {
       
       template <typename U>
       class MatrixData {
       public:
           const size_t data_rows, data_cols;
           U* data;
           unsigned int refCount;
           MatrixData(size_t rows, size_t cols);
           ~MatrixData();
       };
       
       // other members ...
   
   };
   ```

   Definition (for `MatrixData`) :

   ```c++
   template <typename T>
   template <typename U> // two templates are needed because Matrix and MatrixData are both templated
   Matrix<T>::MatrixData<U>::MatrixData(size_t rows, size_t cols) : 
       data_rows(rows), data_cols(cols), refCount(0), data(new U[rows * cols]) {}
   
   template <typename T>
   template <typename U>
   Matrix<T>::MatrixData<U>::~MatrixData() {
       delete[] data;
   }
   ```

3. **Friend function**

   Templated friend function needs a separate template statement.

   Declaration:

   ```C++
   template <typename U> // must wtite an extra typename U here, instead of using the class' typename T
   inline friend Matrix<U> operator*(const int& scalar, const Matrix<U>& matrix);
   ```

   Definition:

   ```C++
   template <typename U>
   inline Matrix<U> operator*(const int& scalar, const Matrix<U>& matrix) {
       return matrix * scalar;
   }
   ```

---

### Race Conditions

**What is a race condition?**

> A race condition occurs when two or more threads can access shared data and they try to change it at the same time.

In my original implementation of operator +, I wrote something like this:

```c++
size_t length = sharedData->data_rows * sharedData->data_cols;
const T* lhs = sharedData->data;
const T* rhs = other.sharedData->data;
T* data = res.sharedData->data;
#pragma omp parallel for // introduces multithreading for computation
for (size_t i = 0; i < length; i++) {
    *(data++) = *(lhs++) + *(rhs++);
}
```

This should be a fast way to do additions because all the pointers iterate through a contagious memory with less overhead.

However, with OpenMP enabled, strange things happened:

```c++
Matrix<int> m1(50, 50, 2); // a 50 * 50 matrix filled with 2
Matrix<int> m2(50, 50, 3); // a 50 * 50 matrix filled with 3
Matrix<int> result = m1 + m2; // addition, no error reported
result.show(); // the result is not filled with 5!
// The result matrix actually looks like:
// 5 5 5 5 5 5 5 ...... 5 
// 5 5 5 5 5 5 5 ...... 5
// ......................
// ......................
// 5 5 5 5 5 5 5 ...... 5
// 5 5 5 0 0 0 0 ...... 0 // a lot of trailing zeros ?
// 0 0 0 0 0 0 0 ...... 0 // a lot of trailing zeros ?
```

**Why did this happen?**

- The `lhs`, `rhs`, and `data` pointers are being incremented in a multi-threaded environment:

  `*(data++) = *(lhs++) + *(rhs++)`

- This operation is not atomic, which means that multiple threads can interfere with each other when they read, modify, and write back the pointer values. This can result in some elements being **skipped**, **overwritten**, or **added incorrectly**, leading to incorrect results and potential crashes.

**How to solve this?**

I rewrote the function in a more traditional way. In this way, the threads enabled by OpenMP have a clearer idea what each of them should do (each handles some of the `i`s in the loop) and will not lead to race conditions. Influence on the performance is negligible.

```c++
#pragma omp parallel for
for (size_t i = 0; i < length; i++) {
    res.sharedData->data[i] = sharedData->data[i] + other.sharedData->data[i];
}
```

---

### Memory Management in C, and in C++

Back in Project2 and Project3, I implemented Strassen's algorithm with struct `Matrix` in C.

A big headache at that time was **how to clean up all the resources before an abnormal exit**.

The **recursive** function that implements Strassen's algorithm involves the **creation of numerous submatrices**, and they all need to be freed properly before the program is terminated.

Below is what I had to do in this C function **if one memory allocation fails**: 

1. **Check NULL pointer:** Of course, I have to check NULL pointer to know that an allocation has failed!
2. **Clean up the local resources:** I must free all the matrices that have been allocated in the function scope, before I exit 
3. **Propagate the error message:** I return an error signal manually via function return values for every new matrix, when I exit

Downsides of this include:

1. Very time-consuming to code
2. Very error-prone
3. Can be slow
4. Can make the code messy

However, in C++, `new` is used instead of `malloc` for memory allocation, which

> Throws an exception of a type that would match a handler of type [std::bad_alloc](https://en.cppreference.com/w/cpp/memory/new/bad_alloc) on failure to allocate memory.

So it throws an exception.

After looking into what exceptions are in C++, I find that exception (without a corresponding catch) works in a similar way with my manual implementation in C.

Here's the process triggered by exception (without catch), called **Stack Unwinding**:

1. Exception is thrown upon allocation failure.

2. It cleans up the local resources, not by manually freeing up all spaces like in C, but by calling the destructors of all the objects created. The only thing we need to do is write a good destructor that is reliable for freeing up heap blocks allocated for the object. 

   It also means we should make good use of objects in C++. We should encapsulate resource management, like `new` and `delete`, into these objects, instead of leaving them scattered in the program, following **Resource Acquisition Is Initialization** principle.

3. It propagates all the way to the top of the call stack, cleans up resources where necessary, and terminates the program.

Because of this, I have used exceptions extensively in the project, indicating invalid arguments, index out of range and others.
