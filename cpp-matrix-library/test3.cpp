//#include "Matrix.h"
//#include <chrono>
//#include <cmath>
//
//
//
//int main() {
//    Matrix<float> m(2211, 4567, 4.2f);
//    Matrix<float> n(4567, 2211, 3.2f);
//    Matrix<float> result(0, 0);
//
//    // time the matrix multiplication
//    for (int i = 0; i < 2; i++) {
//        auto start = std::chrono::high_resolution_clock::now();
//        result = m * n;
//        auto end = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> diff = end - start;
//        std::cout << "Time to multiply: " << diff.count() << " s\n";
//    }
//
//    cout << "result size: " << result.getMRows() << " " << result.getMCols() << endl;
//
//    float ave_error = 0.0;
//    float max_error = 0.0;
//    const float* data = result.getSharedData()->data;
//    for (size_t i = 0; i < 2211 * 2211; i++) {
//        ave_error += fabs(*(data++) - 61380.48f);
//        if (fabs(*(data - 1) - 61380.48f) > max_error) {
//            max_error = fabs(*(data - 1) - 61380.48f);
//        }
//    }
//    // print average ave_error
//    std::cout << "Average ave_error: " << ave_error / (3456 * 4097) << std::endl;
//    // print max ave_error
//    std::cout << "Max ave_error: " << max_error << std::endl;
//}
