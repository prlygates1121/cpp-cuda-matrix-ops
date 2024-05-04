#include "Matrix.h"
#include <iostream>

using namespace std;

int main() {
    Matrix<int> m1 = Matrix<int>::create(2, 2);
    m1.fill(1);
    Matrix<int> m2 = Matrix<int>::create(2, 2);
    m2.fill(2);
    Matrix<int> m3 = m1 + m2;
    Matrix<int> m4 = Matrix<int>::create(2, 2);
    m4.fill(3);
    if (m3 == m4) {
        cout << "Test passed" << endl;
    } else {
        cout << "Test failed" << endl;
    }
}