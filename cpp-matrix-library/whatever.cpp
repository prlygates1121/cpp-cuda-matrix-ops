//#include "Matrix.h"
//
//int main() {
//    // generate some crazy tests for the matrix class
//    Matrix<int> m1(5, 5, 0);
//    m1(0, 2) = 3;
//    m1(1, 1) = 4;
//    m1(1, 2) = 5;
//    m1(2, 1) = 6;
//    m1(2, 2) = 7;
//    m1(3, 1) = 8;
//    m1(3, 2) = 9;
//    m1(4, 1) = 10;
//    m1(4, 2) = 11;
//    m1.show_detailed();
//    Matrix<int> roi = m1.getROI(1, 1, 3, 3);
//    Matrix<int> roi2(2, 2, 2);
//    roi2 = roi;
//    roi2.show_detailed();
//}