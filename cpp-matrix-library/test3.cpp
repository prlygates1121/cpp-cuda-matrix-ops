#include "Matrix.h"

int main() {
    Matrix<int> m(9, 9, 9);
    m.printMatInfo();
    m(1, 1) = 5;
    m(2, 2) = 6;
    m(3, 3) = 7;
    m.printMatInfo();
    Matrix<int> m2 = Matrix<int>(m.getSharedData(), 2, 2, 5, 7);
    m2.printMatInfo();
    m2(1, 1) = 8;
    m2(2, 2) = 0;
    m2.printMatInfo();
    return 0;
}
