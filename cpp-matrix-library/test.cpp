//#include "gtest/gtest.h"
//#include "matrix.h"  // Include your Matrix class header
//
//#define EDGE 3333
//
//// Define a test fixture class
//class MatrixEffectivenessTest : public ::testing::Test {
//protected:
//    Matrix<int> matrix;
//
//    void SetUp() override {
//        matrix = Matrix<int>(EDGE, EDGE, EDGE);
//        int value = 0;
//        for (int i = 0; i < EDGE; ++i) {
//            for (int j = 0; j < EDGE; ++j) {
//                matrix(i, j) = value++;
//            }
//        }
//    }
//};
//
//// Test memory allocation failure
//TEST_F(MatrixEffectivenessTest, MemoryAllocationFailure) {
//    EXPECT_THROW(Matrix<int>(999999, 999999, 9), std::bad_alloc);
//    EXPECT_THROW(matrix.resize(999999, 999999), std::bad_alloc);
//}
//
//// Test empty matrices
//TEST_F(MatrixEffectivenessTest, EmptyMatrixValidation) {
//    Matrix<int> empty;
//    // no addition for empty
//    EXPECT_THROW(empty = empty + empty, std::invalid_argument);
//    // noa addition for empty and non-empty
//    EXPECT_THROW(empty = empty + matrix, std::invalid_argument);
//    // no multiplication for empty
//    EXPECT_THROW(empty = empty * empty, std::invalid_argument);
//    // no scalar multiplication for empty
//    EXPECT_THROW(empty = empty * 2, std::invalid_argument);
//    // no reversed scalar multiplication for empty
//    EXPECT_THROW(empty = 2 * empty, std::invalid_argument);
//    // no clone/detach for empty
//    EXPECT_THROW(empty.detach(), std::invalid_argument);
//    EXPECT_THROW(matrix = empty.clone(), std::invalid_argument);
//    // no resize for empty
//    EXPECT_THROW(empty.resize(3, 3), std::invalid_argument);
//    // no element access for empty
//    EXPECT_THROW(empty(0, 0), std::out_of_range);
//    // compare empty and another empty
//    Matrix<int> empty2;
//    EXPECT_EQ(empty, empty2);
//    // compare empty and non-empty
//    EXPECT_NE(empty, matrix);
//    // construct empty via copy constructor
//    Matrix<int> matrix1(empty);
//    EXPECT_EQ(matrix1, empty);
//    // assign empty to empty
//    Matrix<int> empty3;
//    empty3 = empty;
//    EXPECT_EQ(empty3, empty);
//    // assign non-empty to empty
//    Matrix<int> empty4;
//    empty4 = matrix;
//    EXPECT_EQ(empty4, matrix);
//    // assign empty to non-empty
//    Matrix<int> matrix2(3, 3, 3);
//    matrix2 = empty;
//    EXPECT_EQ(matrix2, empty);
//    // no roi for empty
//    Matrix<int> roi;
//    EXPECT_THROW(roi = empty.getROI(2, 2, 4, 4), std::invalid_argument);
//    // no fill for empty
//    EXPECT_THROW(empty.fill(4), std::invalid_argument);
//    // save empty
//    empty.save("GOOD.txt");
//    // load empty
//    Matrix<int> empty5("GOOD.txt");
//    EXPECT_EQ(empty, empty5);
//}
//
//// Test constructor with fill
//TEST_F(MatrixEffectivenessTest, FillConstructor) {
//    Matrix<int> filled(EDGE, EDGE, 9);
//    for (int i = 0; i < EDGE; ++i) {
//        for (int j = 0; j < EDGE; ++j) {
//            EXPECT_EQ(filled(i, j), 9);
//        }
//    }
//}
//
//// Test self assignment
//TEST_F(MatrixEffectivenessTest, SelfAssignment) {
//    matrix = matrix;
//    EXPECT_EQ(matrix.getDataRefCount(), 1);
//    Matrix<int> matrix2 = matrix;
//    matrix2 = matrix;
//    EXPECT_EQ(matrix.getDataRefCount(), 2);
//    matrix2.detach();
//    matrix2 = matrix;
//}
//
//// Test Clone function
//TEST_F(MatrixEffectivenessTest, CloneMatrix) {
//    Matrix<int> clone = matrix.clone();
//    EXPECT_EQ(clone.getMRows(), matrix.getMRows());
//    EXPECT_EQ(clone.getMCols(), matrix.getMCols());
//    EXPECT_EQ(clone.getDataRefCount(), 1);
//    EXPECT_EQ(clone, matrix);
//}
//
//// Test extraction of a valid ROI
//TEST_F(MatrixEffectivenessTest, ValidROIExtraction) {
//    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
//    ASSERT_EQ(roi.getMRows(), 5);
//    ASSERT_EQ(roi.getMCols(), 5);
//    EXPECT_EQ(roi(0, 0), matrix(2, 2));
//    EXPECT_EQ(roi(4, 4), matrix(6, 6));
//}
//
//// Test ROI extraction with invalid dimensions
//TEST_F(MatrixEffectivenessTest, InvalidROIExtraction) {
//    EXPECT_THROW(matrix.getROI(88888, 8, 3, 3), std::invalid_argument);
//}
//
//// Test adjusting the ROI size within bounds
//TEST_F(MatrixEffectivenessTest, AdjustROIWithinBounds) {
//    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
//    roi.adjustROI(1, 1, 3, 3);
//    ASSERT_EQ(roi.getMRows(), 3);
//    ASSERT_EQ(roi.getMCols(), 3);
//    EXPECT_EQ(roi(0, 0), matrix(1, 1));
//}
//
//// Test attempting to adjust ROI outside of original bounds
//TEST_F(MatrixEffectivenessTest, AdjustROIOutsideBounds) {
//    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
//    EXPECT_THROW(roi.adjustROI(11111, 11111, 6, 6), std::invalid_argument);
//}
//
//// Test consecutive ROI operations to check data consistency
//TEST_F(MatrixEffectivenessTest, ConsecutiveROIOperations) {
//    Matrix<int> roi = matrix.getROI(1, 1, 8, 8);
//    roi = roi.getROI(1, 1, 6, 6);
//    EXPECT_EQ(roi.getDataRefCount(), 2);
//    EXPECT_EQ(roi(0, 0), matrix(2, 2));
//    roi = roi.getROI(1, 1, 4, 4);
//    EXPECT_EQ(roi.getDataRefCount(), 2);
//    EXPECT_EQ(roi(1, 1), matrix(4, 4));
//    Matrix<int> roi2 = roi.getROI(1, 1, 2, 2);
//    EXPECT_EQ(roi2.getDataRefCount(), 3);
//    EXPECT_EQ(roi2(1, 1), matrix(5, 5));
//}
//
//// Test ROI assignment
//TEST_F(MatrixEffectivenessTest, ROIAssignment) {
//    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
//    Matrix<int> roi2 = matrix.getROI(1, 2, 3, 5);
//    roi = roi2;
//    EXPECT_EQ(roi, roi2);
//    EXPECT_EQ(roi.getDataRefCount(), 3);
//    Matrix<int> matrix2(5, 5);
//    matrix2 = roi;
//    EXPECT_EQ(matrix2, roi);
//    EXPECT_EQ(matrix2.getDataRefCount(), 4);
//}
//
//TEST_F(MatrixEffectivenessTest, CancelROI) {
//    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
//    EXPECT_EQ(roi.getDataRefCount(), 2);
//    roi.cancelROI();
//    EXPECT_EQ(roi.getMRows(), EDGE);
//    EXPECT_EQ(roi.getMCols(), EDGE);
//    roi.fill(999);
//    EXPECT_EQ(matrix(2, 2), 999);
//}
//
//TEST_F(MatrixEffectivenessTest, ROIReferenceCount) {
//    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
//    EXPECT_EQ(roi.getDataRefCount(), 2);
//    Matrix<int> roi2 = roi;
//    EXPECT_EQ(roi.getDataRefCount(), 3);
//    roi2.cancelROI();
//    EXPECT_EQ(roi.getDataRefCount(), 3);
//    roi2.detach();
//    EXPECT_EQ(roi.getDataRefCount(), 2);
//    roi.detach();
//    EXPECT_EQ(roi.getDataRefCount(), 1);
//    // test adjustROI's reference count
//    roi = matrix.getROI(2, 2, 5, 5);
//    roi.adjustROI(1, 1, 3, 3);
//    EXPECT_EQ(roi.getDataRefCount(), 2);
//    roi2 = roi;
//    EXPECT_EQ(roi.getDataRefCount(), 3);
//}
//
//
//TEST_F(MatrixEffectivenessTest, EditViaROI) {
//    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
//    roi(0, 0) = 100;
//    EXPECT_EQ(roi(0, 0), 100);
//    EXPECT_EQ(matrix(2, 2), 100);
//    matrix(2, 2) = 200;
//    EXPECT_EQ(roi(0, 0), 200);
//    EXPECT_EQ(matrix(2, 2), 200);
//}
//
//// Explicit detach test
//TEST_F(MatrixEffectivenessTest, DetachEnthusiast) {
//    Matrix<int> a = matrix;
//    EXPECT_EQ(a.getDataRefCount(), 2);
//    a.detach();
//    EXPECT_EQ(a.getDataRefCount(), 1);
//    a.detach();
//    EXPECT_EQ(a.getDataRefCount(), 1);
//    matrix.detach();
//    EXPECT_EQ(matrix.getDataRefCount(), 1);
//    a = matrix;
//    EXPECT_EQ(a.getDataRefCount(), 2);
//    matrix.detach();
//    EXPECT_EQ(a.getDataRefCount(), 1);
//    EXPECT_EQ(matrix.getDataRefCount(), 1);
//}
//
//// Implicit detach test
//TEST_F(MatrixEffectivenessTest, DetachEnthusiastImplicit) {
//    Matrix<int> a = matrix;
//    EXPECT_EQ(a.getDataRefCount(), 2);
//    a = a * 2;
//    EXPECT_EQ(a.getDataRefCount(), 1);
//    Matrix<int> b = matrix.getROI(2, 2, 5, 5);
//    EXPECT_EQ(b.getDataRefCount(), 2);
//    b += b;
//    EXPECT_EQ(b.getDataRefCount(), 1);
//}
//
//// Test ROI detachment
//TEST_F(MatrixEffectivenessTest, DetachROI) {
//    EXPECT_EQ(matrix.getDataRefCount(), 1);
//    Matrix<int> roi = matrix.getROI(2, 2, 5, 5);
//    EXPECT_EQ(matrix.getDataRefCount(), 2);
//    roi.detach();
//    EXPECT_EQ(roi.getDataRefCount(), 1);
//    EXPECT_EQ(matrix.getDataRefCount(), 1);
//    roi.fill(999);
//    EXPECT_NE(matrix(2, 2), 999);
//}
//
//// Test resize() function
//TEST_F(MatrixEffectivenessTest, ResizeMatrix) {
//    Matrix<int> matrixClone = matrix.clone();
//    matrix.resize(EDGE + 1, EDGE + 1, 0);
//    EXPECT_EQ(matrix.getMRows(), EDGE + 1);
//    EXPECT_EQ(matrix.getMCols(), EDGE + 1);
//    EXPECT_EQ(matrix(EDGE, EDGE), 0);
//    Matrix<int> roi = matrix.getROI(0, 0, EDGE, EDGE);
//    EXPECT_EQ(roi, matrixClone);
//    matrix.resize(EDGE - 1, EDGE - 1, 0);
//    EXPECT_EQ(matrix.getMRows(), EDGE - 1);
//    EXPECT_EQ(matrix.getMCols(), EDGE - 1);
//}
//
//// Test addition of two matrices of the same size
//TEST_F(MatrixEffectivenessTest, ValidMatrixAddition) {
//    Matrix<int> matrix2 = matrix; // Use the same matrix for simplicity
//    Matrix<int> result = matrix2 + matrix;
//    for (int i = 0; i < matrix.getMRows(); ++i) {
//        for (int j = 0; j < matrix.getMCols(); ++j) {
//            EXPECT_EQ(result(i, j), matrix(i, j) * 2);
//        }
//    }
//}
//
//// Test matrix addition with mismatched dimensions
//TEST_F(MatrixEffectivenessTest, MatrixAdditionDimensionMismatch) {
//    Matrix<int> matrix2(5, 5); // Smaller matrix
//    EXPECT_THROW(matrix + matrix2, std::invalid_argument);
//}
//
//// Test subtraction of two matrices
//TEST_F(MatrixEffectivenessTest, ValidMatrixSubtraction) {
//    Matrix<int> matrix2 = matrix;
//    Matrix<int> result = matrix - matrix2;
//
//    for (int i = 0; i < matrix.getMRows(); ++i) {
//        for (int j = 0; j < matrix.getMCols(); ++j) {
//            EXPECT_EQ(result(i, j), 0);
//        }
//    }
//}
//
//// Test I/O operations
//TEST_F(MatrixEffectivenessTest, IOTest) {
//    matrix.save("test.txt");
//    Matrix<int> matrix2("test.txt");
//    EXPECT_EQ(matrix, matrix2);
//}
//
//// Test matrix multiplication
//TEST_F(MatrixEffectivenessTest, ValidMatrixMultiplication) {
//    Matrix<int> matrix1(1028, 1025, 1); // Matrix of ones
//    Matrix<int> matrix2(1025, 1029, 1); // Matrix of ones
//    Matrix<int> result = matrix1 * matrix2;
//
//    for (int i = 0; i < matrix1.getMRows(); ++i) {
//        int expectedValue = 0;
//        for (int j = 0; j < matrix1.getMCols(); ++j) {
//            expectedValue += matrix1(i, j);
//        }
//        for (int j = 0; j < matrix2.getMCols(); ++j) {
//            EXPECT_EQ(result(i, j), expectedValue);
//        }
//    }
//}
//
//// Test scalar multiplication
//TEST_F(MatrixEffectivenessTest, ScalarMultiplication) {
//    int scalar = 2;
//    Matrix<int> result = matrix * scalar;
//    for (int i = 0; i < matrix.getMRows(); ++i) {
//        for (int j = 0; j < matrix.getMCols(); ++j) {
//            EXPECT_EQ(result(i, j), matrix(i, j) * scalar);
//        }
//    }
//}
//
//// Test matrix multiplication with dimension mismatch
//TEST_F(MatrixEffectivenessTest, MatrixMultiplicationDimensionMismatch) {
//    Matrix<int> matrix2(5, 20); // Incorrect dimensions for multiplication
//    EXPECT_THROW(matrix * matrix2, std::invalid_argument);
//}
//
//
//int main(int argc, char **argv) {
//    ::testing::InitGoogleTest(&argc, argv);
//    return RUN_ALL_TESTS();
//}
