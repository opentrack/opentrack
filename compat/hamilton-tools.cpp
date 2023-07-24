#include "hamilton-tools.h"
#include <cmath>

tQuat QuatFromAngleAxe(const double angle, const tVector& axe)
{
    double a = TO_RAD * 0.5 * angle;
    double d = sin(a) / VectorLength(axe);
    return  ( tQuat (
                        axe.v[0] * d,
                        axe.v[1] * d,
                        axe.v[2] * d,
                        cos(a)
                    )
            );
}
		
tQuat QuatMultiply(const tQuat& qL, const tQuat& qR)
{
    tQuat Q;
    Q.x = qL.w*qR.x + qL.x*qR.w + qL.y*qR.z - qL.z*qR.y;
    Q.y = qL.w*qR.y + qL.y*qR.w + qL.z*qR.x - qL.x*qR.z;
    Q.z = qL.w*qR.z + qL.z*qR.w + qL.x*qR.y - qL.y*qR.x;
    Q.w = qL.w*qR.w - qL.x*qR.x - qL.y*qR.y - qL.z*qR.z;
    return(Q);
}

tQuat QuatFromYPR(const double YPR[])
{
    tQuat	Q, Qp, Qy;
    Q  = QuatFromAngleAxe( -YPR[2], {0, 0, 1} ); //Roll,  Z axe
    Qp = QuatFromAngleAxe( -YPR[1], {1, 0, 0} ); //Pitch, X axe
    Qy = QuatFromAngleAxe( -YPR[0], {0, 1, 0} ); //Yaw,   Y axe

    Q  = QuatMultiply(Qp, Q);
    return(QuatMultiply(Qy, Q));
}

void Normalize(tQuat& Q)
{
    double m = sqrt(Q.x*Q.x + Q.y*Q.y + Q.z*Q.z + Q.w*Q.w);
    if (m > EPSILON)
    {
        m   = 1 / m;
        Q.x = Q.x * m;
        Q.y = Q.y * m;
        Q.z = Q.z * m;
        Q.w = Q.w * m;
    }
    else Q = tQuat(0, 0, 0, 1);
}

tQuat Slerp(const tQuat& S, const tQuat& D, const double alpha)
{
    // calc cosine of half angle
    double cosin = S.x*D.x + S.y*D.y + S.z*D.z + S.w*D.w;

    // select nearest rotation direction
    tQuat Q;
    if (cosin < 0)
    {
        cosin = - cosin;
        Q.x   = - D.x;
        Q.y   = - D.y;
        Q.z   = - D.z;
        Q.w   = - D.w;
    }
    else Q = D;

    // calculate coefficients
    double scale0, scale1;
    if ((1.0 - cosin) > EPSILON)
    {
        double omega = acos(cosin);
        double sinus = 1 / sin(omega);
        scale0 = sin((1.0 - alpha) * omega) * sinus;
        scale1 = sin(alpha * omega)* sinus;
    }
    else
    {
        scale0 = 1.0 - alpha;
        scale1 = alpha;
    }

    Q.x = scale0 * S.x + scale1 * Q.x;
    Q.y = scale0 * S.y + scale1 * Q.y;
    Q.z = scale0 * S.z + scale1 * Q.z;
    Q.w = scale0 * S.w + scale1 * Q.w;
	
    Normalize(Q);

    return( Q );
}

void QuatToYPR(const tQuat& Q, double YPR[])
{
    const double xx = Q.x * Q.x;
    const double xy = Q.x * Q.y;
    const double xz = Q.x * Q.z;
    const double xw = Q.x * Q.w;
    const double yy = Q.y * Q.y;
    const double yz = Q.y * Q.z;
    const double yw = Q.y * Q.w;
    const double zz = Q.z * Q.z;
    const double zw = Q.z * Q.w;

    YPR[0] = TO_DEG * ( -atan2( 2 * ( xz + yw ), 1 - 2 * ( xx + yy ) ));
    YPR[1] = TO_DEG * (  asin ( 2 * ( yz - xw ) ));
    YPR[2] = TO_DEG * ( -atan2( 2 * ( xy + zw ), 1 - 2 * ( xx + zz ) ));
}
