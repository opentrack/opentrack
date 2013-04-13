#include "tracker_types.h"
#include "rotation.h"

#define PI 3.14159265358979323846264
#define D2R PI/180.0
#define R2D 180.0/PI

T6DOF operator-(const T6DOF& A, const T6DOF& B)
{
    Rotation R_A(A.axes[RX]*D2R, A.axes[RY]*D2R, A.axes[RZ]*D2R);
    Rotation R_B(B.axes[RX]*D2R, B.axes[RY]*D2R, B.axes[RZ]*D2R);
	Rotation R_C = R_A * R_B.inv();

	T6DOF C;
    R_C.toEuler(C.axes[RX], C.axes[RY], C.axes[RZ]);
    C.axes[RX] *= R2D;
    C.axes[RY] *= R2D;
    C.axes[RZ] *= R2D;

    C.axes[TX] = A.axes[TX] - B.axes[TX];
    C.axes[TY] = A.axes[TY] - B.axes[TY];
    C.axes[TZ] = A.axes[TZ] - B.axes[TZ];
	//C.frame_number?
	return C;
}

T6DOF operator+(const T6DOF& A, const T6DOF& B)
{
    Rotation R_A(A.axes[RX]*D2R, A.axes[RY]*D2R, A.axes[RZ]*D2R);
    Rotation R_B(B.axes[RX]*D2R, B.axes[RY]*D2R, B.axes[RZ]*D2R);
	Rotation R_C = R_A * R_B;

	T6DOF C;
    R_C.toEuler(C.axes[RX], C.axes[RY], C.axes[RZ]);
    C.axes[RX] *= R2D;
    C.axes[RY] *= R2D;
    C.axes[RZ] *= R2D;

    C.axes[TX] = A.axes[TX] + B.axes[TX];
    C.axes[TY] = A.axes[TY] + B.axes[TY];
    C.axes[TZ] = A.axes[TZ] + B.axes[TZ];
	//C.frame_number?
	return C;
}
