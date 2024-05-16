#include "gtest/gtest.h"
#include "matrix.h"  // Include your Matrix class header

#define EDGE 3333

// Define a test fixture class
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

// Test extraction of a valid ROI
TEST_F(MatrixEffectivenessTest, ValidROIExtraction) {
    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
    ASSERT_EQ(roi.getMRows(), 5);
    ASSERT_EQ(roi.getMCols(), 5);
    EXPECT_EQ(roi(0, 0), matrix(2, 2));
    EXPECT_EQ(roi(4, 4), matrix(6, 6));
}

// Test ROI extraction with invalid dimensions
TEST_F(MatrixEffectivenessTest, InvalidROIExtraction) {
    EXPECT_THROW(matrix.getROI(88888, 8, 3, 3), std::invalid_argument);
}

// Test adjusting the ROI size within bounds
TEST_F(MatrixEffectivenessTest, AdjustROIWithinBounds) {
    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
    roi.adjustROI(1, 1, 3, 3);
    ASSERT_EQ(roi.getMRows(), 3);
    ASSERT_EQ(roi.getMCols(), 3);
    EXPECT_EQ(roi(0, 0), matrix(1, 1));
}

// Test attempting to adjust ROI outside of original bounds
TEST_F(MatrixEffectivenessTest, AdjustROIOutsideBounds) {
    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
    EXPECT_THROW(roi.adjustROI(11111, 11111, 6, 6), std::invalid_argument);
}

// Test consecutive ROI operations to check data consistency
TEST_F(MatrixEffectivenessTest, ConsecutiveROIOperations) {
    Matrix<int> roi = matrix.getROI(1, 1, 8, 8);
    roi = roi.getROI(1, 1, 6, 6);
    EXPECT_EQ(roi.getSharedData()->refCount, 2);
    EXPECT_EQ(roi.getMRows(), 6);
    EXPECT_EQ(roi.getMCols(), 6);
    EXPECT_EQ(roi(0, 0), matrix(2, 2));
    roi = roi.getROI(1, 1, 4, 4);
    EXPECT_EQ(roi.getSharedData()->refCount, 2);
    EXPECT_EQ(roi.getMRows(), 4);
    EXPECT_EQ(roi.getMCols(), 4);
    EXPECT_EQ(roi(1, 1), matrix(4, 4));
    Matrix<int> roi2 = roi.getROI(1, 1, 2, 2);
    EXPECT_EQ(roi2.getSharedData()->refCount, 3);
    EXPECT_EQ(roi2(1, 1), matrix(5, 5));
}

TEST_F(MatrixEffectivenessTest, EditViaROI) {
    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
    roi(0, 0) = 100;
    EXPECT_EQ(roi(0, 0), 100);
    EXPECT_EQ(matrix(2, 2), 100);
    matrix(2, 2) = 200;
    EXPECT_EQ(roi(0, 0), 200);
    EXPECT_EQ(matrix(2, 2), 200);
}

// Test addition of two matrices of the same size
TEST_F(MatrixEffectivenessTest, ValidMatrixAddition) {
    Matrix<int> matrix2 = matrix; // Use the same matrix for simplicity
    Matrix<int> result = matrix2 + matrix;
    for (int i = 0; i < matrix.getMRows(); ++i) {
        for (int j = 0; j < matrix.getMCols(); ++j) {
            EXPECT_EQ(result(i, j), matrix(i, j) * 2);
        }
    }
}

// Test matrix addition with mismatched dimensions
TEST_F(MatrixEffectivenessTest, MatrixAdditionDimensionMismatch) {
    Matrix<int> matrix2(5, 5); // Smaller matrix
    EXPECT_THROW(matrix + matrix2, std::invalid_argument);
}

// Test subtraction of two matrices
TEST_F(MatrixEffectivenessTest, ValidMatrixSubtraction) {
    Matrix<int> matrix2 = matrix;
    Matrix<int> result = matrix - matrix2;

    for (int i = 0; i < matrix.getMRows(); ++i) {
        for (int j = 0; j < matrix.getMCols(); ++j) {
            EXPECT_EQ(result(i, j), 0);
        }
    }
}

// Test matrix multiplication
TEST_F(MatrixEffectivenessTest, ValidMatrixMultiplication) {
    Matrix<int> matrix2(EDGE, EDGE, 1); // Matrix of ones
    Matrix<int> result = matrix * matrix2;

    for (int i = 0; i < matrix.getMRows(); ++i) {
        int expectedValue = 0;
        for (int j = 0; j < matrix.getMCols(); ++j) {
            expectedValue += matrix(i, j);
        }
        for (int j = 0; j < matrix2.getMCols(); ++j) {
            EXPECT_EQ(result(i, j), expectedValue);
        }
    }
}

// Test scalar multiplication
TEST_F(MatrixEffectivenessTest, ScalarMultiplication) {
    int scalar = 2;
    Matrix<int> result = matrix * scalar;

    for (int i = 0; i < matrix.getMRows(); ++i) {
        for (int j = 0; j < matrix.getMCols(); ++j) {
            EXPECT_EQ(result(i, j), matrix(i, j) * scalar);
        }
    }
}

// Test matrix multiplication with dimension mismatch
TEST_F(MatrixEffectivenessTest, MatrixMultiplicationDimensionMismatch) {
    Matrix<int> matrix2(5, 20); // Incorrect dimensions for multiplication
    EXPECT_THROW(matrix * matrix2, std::invalid_argument);
}


// Add this line to your test file to run all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
