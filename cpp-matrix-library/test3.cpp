#include "Matrix.h"

int main() {
    Matrix<int> a = Matrix<int>::create(2, 2, 2);
    a.printMatrix();
    a.resize(3, 3);
    a(2, 2) = 0;
    a(2, 1) = 0;
    a(2, 0) = 0;
    a(0, 2) = 0;
    a(1, 2) = 0;
    a.printMatrix();
}