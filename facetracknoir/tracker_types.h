#ifndef __TRACKER_TYPES_H__
#define __TRACKER_TYPES_H__

#include "ftnoir_tracker_base/ftnoir_tracker_types.h"

struct T6DOF {
public:
    double axes[6];

    T6DOF() {
        for (int i = 0; i < 6; i++)
            axes[i] = 0;
    }
};

T6DOF operator-(const T6DOF& A, const T6DOF& B); // get new pose with respect to reference pose B
T6DOF operator+(const T6DOF& A, const T6DOF& B); // get new pose with respect to reference pose B^-1

#endif //__TRACKER_TYPES_H__
