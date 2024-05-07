//#include <iostream>
//#include <cassert>
//#include "Matrix.h" // Ensure this file path points to the Matrix.h file
//
//void testCopyOnWriteModification() {
//    auto mat1 = Matrix<int>::create(3, 3);
//    mat1.fill(5);
//
//    auto mat2 = mat1; // mat2 should now share data with mat1
//    assert(mat1 == mat2);
//
//    mat2(1, 1) = 10; // Modification should trigger copy-on-write
//
//    // After modification, mat1 and mat2 should no longer be equal
//    assert(mat1 != mat2);
//
//    // Check the values to ensure copy-on-write worked correctly
//    assert(mat1(1, 1) == 5);
//    assert(mat2(1, 1) == 10);
//
//    std::cout << "testCopyOnWriteModification passed!" << std::endl;
//}
//
//void testReferenceCounting() {
//    auto mat1 = Matrix<int>::create(3, 3);
//    mat1.fill(5);
//
//    assert(mat1.getSharedData()->refCount == 1); // Initial refCount should be 1
//
//    auto mat2 = mat1; // mat2 should now share data with mat1
//    assert(mat1.getSharedData()->refCount == 2); // refCount should be incremented to 2
//
//    {
//        auto mat3 = mat1; // mat3 should also share data with mat1 and mat2
//        assert(mat1.getSharedData()->refCount == 3); // refCount should be 3
//    }
//
//    // After mat3 goes out of scope, refCount should decrement
//    assert(mat1.getSharedData()->refCount == 2);
//
//    mat2.fill(10); // Modification should trigger copy-on-write for mat2
//
//    // After copy-on-write, mat1 and mat2 should have separate data, and refCount for mat1 should be 1
//    assert(mat1.getSharedData()->refCount == 1);
//    assert(mat2.getSharedData()->refCount == 1);
//
//    std::cout << "testReferenceCounting passed!" << std::endl;
//}
//
//int main() {
//    testCopyOnWriteModification();
//    testReferenceCounting();
//    std::cout << "All tests passed!" << std::endl;
//    return 0;
//}
