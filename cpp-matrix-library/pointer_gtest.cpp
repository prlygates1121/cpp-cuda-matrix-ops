#include "Matrix.h"
#include <gtest/gtest.h>

TEST(Pointer, testCopyOnWriteModification) {
    Matrix<int> mat1(3, 3, 5);
    Matrix<int> mat2 = mat1; // mat2 should now share data with mat1
    ASSERT_EQ(mat1.getSharedData(), mat2.getSharedData());

    mat2(1, 1) = 10; // Modification should trigger copy-on-write

    // After modification, mat1 and mat2 should no longer be equal
    ASSERT_NE(mat1.getSharedData(), mat2.getSharedData());

    mat1.printMatInfo();

    // Check the values to ensure copy-on-write worked correctly
    ASSERT_EQ(mat1.read(1, 1), 5);
    ASSERT_EQ(mat2.read(1, 1), 10);
}

TEST(Pointer, testReferenceCounting) {
    Matrix<int> mat1(3, 3, 5);
    ASSERT_EQ(mat1.getSharedData()->refCount, 1); // Initial refCount should be 1

    Matrix<int> mat2 = mat1; // mat2 should now share data with mat1
    ASSERT_EQ(mat1.getSharedData()->refCount, 2); // refCount should be incremented to 2

    {
        Matrix<int> mat3 = mat1; // mat3 should also share data with mat1 and mat2
        ASSERT_EQ(mat1.getSharedData()->refCount, 3); // refCount should be 3
    }

    // After mat3 goes out of scope, refCount should decrement
    ASSERT_EQ(mat1.getSharedData()->refCount, 2);

    mat2.fill(10); // Modification should trigger copy-on-write for mat2

    // After copy-on-write, mat1 and mat2 should have separate data, and refCount for mat1 should be 1
    ASSERT_EQ(mat1.getSharedData()->refCount, 1);
    ASSERT_EQ(mat2.getSharedData()->refCount, 1);
}

TEST(Pointer, testRegionOfInterest) {
    Matrix<int> child1;
    {
        Matrix<int> mother(5, 5, 1);
        mother(1, 1) = 2;
        mother(1, 2) = 3;
        mother(2, 1) = 4;
        mother(2, 2) = 5;
        child1 = Matrix<int>(mother.getSharedData(), 1, 1, 3, 3);
        ASSERT_EQ(child1.read(0, 0), 2);
        ASSERT_EQ(child1.read(0, 1), 3);
        ASSERT_EQ(child1.read(1, 0), 4);
        ASSERT_EQ(child1.read(1, 1), 5);
        cout << child1.read(0, 0) << endl;
        ASSERT_EQ(child1.getSharedData()->refCount, 2);
    }
    ASSERT_EQ(child1.read(0, 0), 2);
    ASSERT_EQ(child1.read(0, 1), 3);
    ASSERT_EQ(child1.read(1, 0), 4);
    ASSERT_EQ(child1.read(1, 1), 5);
    ASSERT_EQ(child1.getSharedData()->refCount, 1);
}

//int main() {
//    Matrix<int> a;
//    Matrix<int> b;
//
//
//    {
//        Matrix<int> mother(5, 5, 1);
//        mother.printMatInfo();
//        a = Matrix<int>(mother.getSharedData(), 1, 1, 3, 3);
//        a.printMatInfo();
//        b = a;
//    }
//
//    a.printMatInfo();
//    b.printMatInfo();
//}