//#include <gtest/gtest.h>
//#include "Matrix.h"
//
//#define ALLOWED_ERROR 0
//
//// Demonstrate some basic assertions.
//TEST(Algebra, AdditionAndSubtraction) {
//    Matrix<int> m(512, 512, 4);
//    Matrix<int> n(512, 512, 3);
//    Matrix<int> result(0, 0);
//
//    result = m + n;
//    for (size_t i = 0; i < 512 * 512; i++) {
//        ASSERT_EQ(result.getSharedData()->data[i], 7);
//    }
//
//    result = result - n;
//    for (size_t i = 0; i < 512 * 512; i++) {
//        ASSERT_EQ(result.getSharedData()->data[i], 4);
//    }
//
//    result = result - m;
//    for (size_t i = 0; i < 512 * 512; i++) {
//        ASSERT_EQ(result.getSharedData()->data[i], 0);
//    }
//
//    Matrix<float> m2(512, 512, 4.2f);
//    Matrix<float> n2(512, 512, 3.2f);
//    Matrix<float> result2(0, 0);
//
//    result2 = m2 + n2;
//    for (size_t i = 0; i < 512 * 512; i++) {
//        ASSERT_FLOAT_EQ(result2.getSharedData()->data[i], 7.4f);
//    }
//
//    result2 = result2 - n2;
//    for (size_t i = 0; i < 512 * 512; i++) {
//        ASSERT_FLOAT_EQ(result2.getSharedData()->data[i], 4.2f);
//    }
//}
//
//TEST(Algebra, NormalMultiplication_DOUBLE) {
//    Matrix<double> m(512, 512, 4.2);
//    Matrix<double> n(512, 512, 3.2);
//    Matrix<double> result(0, 0);
//
//    result = m * n;
//    for (size_t i = 0; i < 512 * 512; i++) {
//        ASSERT_NEAR(result.getSharedData()->data[i], 6881.28, 1e-10);
//    }
//}
//
//TEST(Algebra, NormalMultiplication_FLOAT) {
//    Matrix<float> m(512, 512, 10.4f);
//    Matrix<float> n(512, 512, -0.234f);
//    Matrix<float> result(0, 0);
//
//    result = m * n;
//    for (size_t i = 0; i < 512 * 512; i++) {
//        ASSERT_NEAR(result.getSharedData()->data[i], -1246.0032f, 1e-2);
//    }
//}
//
//TEST(Algebra, BlockedMultiplication_DOUBLE) {
//    Matrix<double> m(2345, 4231, 9.2);
//    Matrix<double> n(4231, 1025, 0.859);
//    Matrix<double> result(0, 0);
//
//    result = m * n;
//    for (size_t i = 0; i < 2345 * 1025; i++) {
//        ASSERT_NEAR(result.getSharedData()->data[i], 9.2 * 0.859 * 4231, 1e-9);
//    }
//}
//
//TEST(Algebra, BlockedMultiplication_DOUBLE_2) {
//    Matrix<double> m(1340, 1024, 3452.2);
//    Matrix<double> n(1024, 1111, 0.0231);
//    Matrix<double> result(0, 0);
//
//    result = m * n;
//    for (size_t i = 0; i < 1340 * 1111; i++) {
//        ASSERT_NEAR(result.getSharedData()->data[i], 3452.2 * 0.0231 * 1024, 1e-7);
//    }
//}
//
//TEST(Algebra, BlockedMultiplication_FLOAT) {
//    Matrix<float> m(2345, 4231, 9.2f);
//    Matrix<float> n(4231, 1024, 0.859f);
//    Matrix<float> result(0, 0);
//
//    result = m * n;
//    for (size_t i = 0; i < 2345 * 1024; i++) {
//        ASSERT_NEAR(result.getSharedData()->data[i], 9.2f * 0.859f * 4231, 20);
//    }
//}
//
//TEST(Algebra, BlockedMultiplication_FLOAT_2) {
//    Matrix<float> m(1344, 6048, 3452.2f);
//    Matrix<float> n(6048, 1111, 0.0231f);
//    Matrix<float> result(0, 0);
//
//    result = m * n;
//    for (size_t i = 0; i < 1344 * 1111; i++) {
//        ASSERT_NEAR(result.getSharedData()->data[i], 3452.2f * 0.0231f * 6048, 20);
//    }
//}
//
//TEST(Memory, CopyConstructor) {
//    Matrix<int> m(512, 512, 4);
//    Matrix<int> n(m);
//
//    for (size_t i = 0; i < 512 * 512; i++) {
//        ASSERT_EQ(n.getSharedData()->data[i], 4);
//    }
//}
//
