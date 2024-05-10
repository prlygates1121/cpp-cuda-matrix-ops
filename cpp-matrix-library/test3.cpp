#include "Matrix.h"
#include <chrono>
#include <cmath>



int main() {
    Matrix<float> m(4096, 4096, 4.2f);
    Matrix<float> n(4096, 4096, 3.2f);
    Matrix<float> result(4096, 4096);

    // time the matrix multiplication
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        result = m * n;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "Time to multiply: " << diff.count() << " s\n";
    }

//    float error = 0.0;
//    const float* data = result.getSharedData()->data;
//    for (size_t i = 0; i < 3456 * 4096; i++) {
//        error += fabs(*(data++) - 61380.48f);
//    }
//    // print average error
//    std::cout << "Average error: " << error / (3456 * 4096) << std::endl;
    return 0;

}
