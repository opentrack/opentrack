#include "tracker_types.h"
#include "rotation.h"

#define PI 3.14159265358979323846264
#define D2R PI/180.0
#define R2D 180.0/PI

T6DOF operator-(const T6DOF& A, const T6DOF& B)
{
    RotationType R_A(A.axes[Yaw]*D2R, A.axes[Pitch]*D2R, A.axes[Roll]*D2R);
    RotationType R_B(B.axes[Yaw]*D2R, B.axes[Pitch]*D2R, B.axes[Roll]*D2R);
	RotationType R_C = R_A * R_B.inv();

	T6DOF C;
    R_C.toEuler(C.axes[Yaw], C.axes[Pitch], C.axes[Roll]);
    C.axes[Yaw] *= R2D;
    C.axes[Pitch] *= R2D;
    C.axes[Roll] *= R2D;

    C.axes[TX] = A.axes[TX] - B.axes[TX];
    C.axes[TY] = A.axes[TY] - B.axes[TY];
    C.axes[TZ] = A.axes[TZ] - B.axes[TZ];
	//C.frame_number?
	return C;
}

T6DOF operator+(const T6DOF& A, const T6DOF& B)
{
    RotationType R_A(A.axes[Yaw]*D2R, A.axes[Pitch]*D2R, A.axes[Roll]*D2R);
    RotationType R_B(B.axes[Yaw]*D2R, B.axes[Pitch]*D2R, B.axes[Roll]*D2R);
	RotationType R_C = R_A * R_B;

	T6DOF C;
    R_C.toEuler(C.axes[Yaw], C.axes[Pitch], C.axes[Roll]);
    C.axes[Yaw] *= R2D;
    C.axes[Pitch] *= R2D;
    C.axes[Roll] *= R2D;

    C.axes[TX] = A.axes[TX] + B.axes[TX];
    C.axes[TY] = A.axes[TY] + B.axes[TY];
    C.axes[TZ] = A.axes[TZ] + B.axes[TZ];
	//C.frame_number?
	return C;
}
