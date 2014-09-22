#pragma once

struct T6DOF {
public:
    double axes[6];

    T6DOF() : axes {0,0,0, 0,0,0 } {}
};

T6DOF operator-(const T6DOF& A, const T6DOF& B); // get new pose with respect to reference pose B
T6DOF operator+(const T6DOF& A, const T6DOF& B); // get new pose with respect to reference pose B^-1
