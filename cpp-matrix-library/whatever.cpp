#include "Matrix.h"

int main() {
    Matrix<int> a(5, 5, 9);
    a(1, 1) = 0;
    a(2, 2) = 1;
    a(3, 3) = 2;
    a(4, 4) = 3;
    a.show();

    Matrix<int> b = a;
    b.show();

    Matrix<int> a1(a.getSharedData(), 2, 2, 5, 5);
    a1.show();

    a1 = a1 * a1;
    a1.show();
    a.show();

    Matrix<int> a2(a.getSharedData(), 0, 0, 3, 3);
    a2.show();
    a2.detach();

    a2.show();
    a.show();
}