#include "Matrix.h"

int main() {
    Matrix<int> a = Matrix<int>(4, 4, 2);
    a.fill(4);
    a(2, 2) = 1;
    a(2, 1) = 1;
    a(2, 0) = 1;
    a(0, 2) = 1;
    a(1, 2) = 1;
    a.resize(5, 9, 1);
    a.printMatInfo();
    Matrix<int> b = Matrix<int>(a.getSharedData(), 1, 1, 3, 4);
    std::cout << "b" << std::endl;
    b.printMatInfo();
    b(1, 1) = 2;
    std::cout << "b after modification" << std::endl;
    b.printMatInfo();
    std::cout << "a" << std::endl;
    a.printMatInfo();
    Matrix<int> c = Matrix<int>(a.getSharedData(), 2, 3, 5, 5);
    std::cout << "c" << std::endl;
    c.printMatInfo();
    std::cout << "b" << std::endl;
    b.printMatInfo();
    Matrix<int> d = c * b;
    std::cout << "d" << std::endl;
    d.printMatInfo();
//    Matrix<int> a = Matrix<int>(3, 4, 2);
//    a(1, 1) = 1;
//    Matrix<int> b = Matrix<int>(4, 3, 1);
//    b(1, 1) = 2;
//    b(2, 2) = 2;
//    cout << "a" << endl;
//    a.printMatInfo();
//    cout << "b" << endl;
//    b.printMatInfo();
//    Matrix<int> c = a * b;
//    cout << "c" << endl;
//    c.printMatInfo();
}
