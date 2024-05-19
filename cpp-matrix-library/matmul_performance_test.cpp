//#include "matrix.h"
//#include <chrono>
//#include <cmath>
//#define r 6231
//#define c 5291
//#define k 4567
//int main() {
//    Matrix<float> m(r, k, 4.2);
//    for (int i = 0; i < k - 1; i++)
//        m(i, i) = 680.48;
//    Matrix<float> n(k, c, 3.2);
//    Matrix<float> result;
//    for (int i = 0; i < 100; i++) {
//        auto start = std::chrono::high_resolution_clock::now();
//        result = m * n;
//        auto end = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> diff = end - start;
//        std::cout << "Time to multiply: " << diff.count() << " s\n";
//    }
//}
