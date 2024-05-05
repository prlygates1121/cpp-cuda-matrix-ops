#include "Matrix.h"
#include <iostream>

using namespace std;

int main() {
    // test the overloaded operators
    Matrix<int> m1 = Matrix<int>::create(2, 2);
    m1(0, 0) = 1;
    m1(0, 1) = 2;
    m1(1, 0) = 3;
    m1(1, 1) = 4;
    m1.printMatrix();

    Matrix<int> m2 = Matrix<int>::create(2, 2);
    m2 = m1;

    m2.printMatrix();

    // change m2
    m2(0, 0) = 5;
    m2.printMatrix();
    m2(0, 1) = 6;
    m2(1, 0) = 7;
    m2(1, 1) = 8;

    m2.printMatrix();
}