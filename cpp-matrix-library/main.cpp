#include "Matrix.h"
#include <iostream>

using namespace std;

int main() {
    Matrix <int> m1 = Matrix<int>::create(2, 2);
    m1.fill(1);
    Matrix <int> m2 = m1;
    Matrix <int> m3 = m1 + m2;
    Matrix <int> m4 = m2;


    m3.printMatrix();

    m1.printMatrix();
}