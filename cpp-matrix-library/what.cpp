#include <iostream>
#include "matrix.h"

int layer2() {
    Matrix<int> m(3, 3);
    throw runtime_error("Layer 2");
    return 0;
}

int layer1() {
    Matrix<int> n(3, 3);
    layer2();
    return 0;
}

//class MyClass {
//public:
//    MyClass() {
//        cout << "MyClass constructor\n";
//    }
//    MyClass(const MyClass& other) {
//        cout << "MyClass copy constructor\n";
//    }
//
//    MyClass& operator=(const MyClass& other) {
//        cout << "MyClass copy assignment\n";
//        return *this;
//    }
//
//    ~MyClass() {
//        cout << "MyClass destructor\n";
//    }
//
//    MyClass operator+(const MyClass& other) {
//        cout << "MyClass operator+\n";
//        MyClass result;
//        return result;
//    }
//};

int main() {
    Matrix<int> m(4, 4, 4);
    Matrix<int> n(4, 4, 4);

    cout << "\ncreated m and n\n\n";

    m = n * n;

    cout << "\nfinished\n\n";

}

// Matrix<int> o = m + n; // copy constructor is called
// When the instance is returned by value, the copy constructor is called to create a new instance of Matrix
// copy assignment is called (an instance of Matrix is created inside the operator* function and is returned by value)