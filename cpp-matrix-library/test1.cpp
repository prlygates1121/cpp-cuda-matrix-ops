//#include <iostream>
//#include <stdexcept>
//#include <cassert>
//#include "Matrix.h"  // Ensure this file path points to the Matrix.h file
//
//void testCreateAndFill() {
//    auto mat = Matrix<int>::create(3, 3);
//    mat.fill(7);
//    for (size_t i = 0; i < 3; ++i)
//        for (size_t j = 0; j < 3; ++j)
//            assert(mat(i, j) == 7);
//    std::cout << "testCreateAndFill passed!" << std::endl;
//}
//
//void testAddition() {
//    auto mat1 = Matrix<int>::create(2, 2);
//    mat1.fill(3);
//    auto mat2 = Matrix<int>::create(2, 2);
//    mat2.fill(4);
//    auto result = mat1 + mat2;
//    for (size_t i = 0; i < 2; ++i)
//        for (size_t j = 0; j < 2; ++j)
//            assert(result(i, j) == 7);
//    std::cout << "testAddition passed!" << std::endl;
//}
//
//void testMultiplication() {
//    auto mat1 = Matrix<int>::create(2, 2);
//    mat1(0, 0) = 1;
//    mat1(0, 1) = 2;
//    mat1(1, 0) = 3;
//    mat1(1, 1) = 4;
//    auto mat2 = Matrix<int>::create(2, 2);
//    mat2(0, 0) = 5;
//    mat2(0, 1) = 6;
//    mat2(1, 0) = 7;
//    mat2(1, 1) = 8;
//    auto result = mat1 * mat2;
//    assert(result(0, 0) == 19);
//    assert(result(0, 1) == 22);
//    assert(result(1, 0) == 43);
//    assert(result(1, 1) == 50);
//    std::cout << "testMultiplication passed!" << std::endl;
//}
//
//void testEquality() {
//    auto mat1 = Matrix<int>::create(2, 2);
//    auto mat2 = mat1;
//    assert(mat1 == mat2);
//    mat1.fill(1);
//    mat2.fill(1);
//    assert(mat1 == mat2);
//    mat2(0, 0) = 5;
//    assert(mat1 != mat2);
//    std::cout << "testEquality passed!" << std::endl;
//}
//
//void testCopyOnWrite() {
//    auto mat1 = Matrix<int>::create(2, 2);
//    auto mat2 = mat1;
//    mat1.fill(10);
//    mat2.fill(20);
//    assert(mat1 != mat2);
//    assert(mat1(0, 0) == 10);
//    assert(mat2(0, 0) == 20);
//    std::cout << "testCopyOnWrite passed!" << std::endl;
//}
//
//int main() {
//    testCreateAndFill();
//    testAddition();
//    testMultiplication();
//    testEquality();
//    testCopyOnWrite();
//    std::cout << "All tests passed!" << std::endl;
//    return 0;
//}
