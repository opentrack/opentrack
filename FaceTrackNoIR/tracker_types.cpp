#include "tracker_types.h"
#include "rotation.h"

const double PI = 3.14159265358979323846264;
const double D2R = PI/180.0;
const double R2D = 180.0/PI;

T6DOF operator-(const T6DOF& A, const T6DOF& B)
{
	Rotation R_A(A.yaw*D2R, A.pitch*D2R, A.roll*D2R);
	Rotation R_B(B.yaw*D2R, B.pitch*D2R, B.roll*D2R);
	Rotation R_C = R_A * R_B.inv();

	T6DOF C;
	R_C.toEuler(C.yaw, C.pitch, C.roll);
	C.yaw *= R2D;
	C.pitch *= R2D;
	C.roll *= R2D;

	C.x = A.x - B.x;
	C.y = A.y - B.y;
	C.z = A.z - B.z;
	//C.frame_number?
	return C;
}

T6DOF operator+(const T6DOF& A, const T6DOF& B)
{
	Rotation R_A(A.yaw*D2R, A.pitch*D2R, A.roll*D2R);
	Rotation R_B(B.yaw*D2R, B.pitch*D2R, B.roll*D2R);
	Rotation R_C = R_A * R_B;

	T6DOF C;
	R_C.toEuler(C.yaw, C.pitch, C.roll);
	C.yaw *= R2D;
	C.pitch *= R2D;
	C.roll *= R2D;

	C.x = A.x + B.x;
	C.y = A.y + B.y;
	C.z = A.z + B.z;
	//C.frame_number?
	return C;
}