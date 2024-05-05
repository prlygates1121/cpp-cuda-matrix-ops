#include "Matrix.h"
#include <iostream>

using namespace std;

int main() {
    Matrix<int> m1 = Matrix<int>::create(1, 2);
    m1.fill(1);
    Matrix<int> m2 = Matrix<int>::create(2, 3);
    m2.fill(1);
    Matrix<int> m3 = m1 * m2;
    Matrix<int> m4 = Matrix<int>::create(1, 3);
    m4.fill(2);
    if (m3 == m4) {
        cout << "m3 == m4" << endl;
    } else {
        cout << "m3 != m4" << endl;
    }
}