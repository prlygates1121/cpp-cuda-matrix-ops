#include "Matrix.h"

int main() {
    Matrix<int> a = Matrix<int>(2, 2, 2);
    a.printMatrix();
    a.resize(3, 3);
    a(2, 2) = 1;
    a(2, 1) = 1;
    a(2, 0) = 1;
    a(0, 2) = 1;
    a(1, 2) = 1;
    a.printMatrix();
    a = a * a;
    Matrix<int> c = a - a;
    a.printMatrix();
    a.resize(1, 1);
    Matrix<int> b = a;
    a.printMatrix();
    c.printMatrix();
}