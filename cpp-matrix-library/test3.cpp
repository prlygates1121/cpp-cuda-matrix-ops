#include "Matrix.h"

int main() {
    Matrix<int> a(2, 2);
    a.printMatrix();
    a.resize(3, 3);
    a(2, 2) = 1;
    a(2, 1) = 1;
    a(2, 0) = 1;
    a(0, 2) = 1;
    a(1, 2) = 1;
    a.printMatrix();
}