//#include "matrix.h"
//#include <chrono>
//#include <cstdlib>
//#define r 3333
//#define c 3333
//#define k 3333
//int main() {
//    srand(time(nullptr));
//    Matrix<float> m(r, k);
//    for (int i = 0; i < r; i++)
//        for (int j = 0; j < k; j++)
//            m(i, j) = (float)rand() / RAND_MAX;
//    Matrix<float> n(k, c);
//    for (int i = 0; i < k; i++)
//        for (int j = 0; j < c; j++)
//            n(i, j) = (float)rand() / RAND_MAX;
//    Matrix<float> result1;
//    Matrix<float> result2;
//    result2 = m.plain_multiply(n);
//    result1 = m * n;
//    if (result1 == result2)
//        std::cout << "Correctness test passed!" << std::endl;
//    else
//        std::cout << "Correctness test failed!" << std::endl;
//}