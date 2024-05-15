//#include "Matrix.h"
//#include <chrono>
//#include <cmath>
//
//#define r 6231
//#define c 5291
//#define k 4567
//
//
//int main() {
//    Matrix<float> m(r, k, 4.2);
//    // use loop to make m diagonal
//    for (int i = 0; i < k - 1; i++) {
//        m(i, i) = 680.48;
//    }
//    Matrix<float> n(k, c, 3.2);
//    Matrix<float> result(0, 0);
//
//    // time the matrix multiplication
//    for (int i = 0; i < 100; i++) {
//        auto start = std::chrono::high_resolution_clock::now();
//        result = m * n;
//        auto end = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> diff = end - start;
//        std::cout << "Time to multiply: " << diff.count() << " s\n";
//    }
//
////    float total_error = 0.0;
////    float max_error = 0.0;
////    const float* data = result.getSharedData()->data;
////    for (size_t i = 0; i < r * c; i++) {
////        total_error += fabs(*(data++) - 61380.48f);
////        if (fabs(*(data - 1) - 61380.48f) > max_error) {
////            max_error = fabs(*(data - 1) - 61380.48f);
////        }
////    }
////    // print average ave_error
////    std::cout << "Average ave_error: " << total_error / (r * c) << std::endl;
////    // print max ave_error
////    std::cout << "Max ave_error: " << max_error << std::endl;
//
//    }
