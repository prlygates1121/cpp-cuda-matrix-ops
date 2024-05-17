//#include "Matrix.h"
//#include <chrono>
//#include <cstdlib>
//
//#define r 3333
//#define c 3333
//#define k 3333
//
//
//int main() {
//    srand(time(nullptr));
//
//    Matrix<float> m(r, k);
//    // fill m with random values
//    for (int i = 0; i < r; i++) {
//        for (int j = 0; j < k; j++) {
//            m(i, j) = (float)rand() / RAND_MAX;
//        }
//    }
//
//    Matrix<float> n(k, c);
//    // fill n with random values
//    for (int i = 0; i < k; i++) {
//        for (int j = 0; j < c; j++) {
//            n(i, j) = (float)rand() / RAND_MAX;
//        }
//    }
//
//    Matrix<float> result1;
//    Matrix<float> result2;
//
//    result2 = m.plain_multiply(n);
//    result1 = m * n;
//
//
//    // check if the two results are the same
//    if (result1 == result2) {
//        std::cout << "Correctness test passed!" << std::endl;
//    } else {
//        std::cout << "Correctness test failed!" << std::endl;
//    }
//    // print the first 10 elements of the results
//    for (int i = 0; i < 10; i++) {
//        std::cout << result1(0, i) << " " << result2(0, i) << std::endl;
//    }
//}