#ifndef FTNOIR_TARDUINO_TYPE_H
#define FTNOIR_TARDUINO_TYPE_H

#include <QtGlobal>

// Arduino frame structure
#pragma pack(push,2)
struct TArduinoData
{
	quint16  Begin;    // Header trame 0xAAAA;
    quint16  Code;     // 0->999 Num Frame  >=2000  Info >=3000 Init  >=5000 Start Command  >=9000 Error
	float Gyro[3];  
	float Acc[3];
    quint16  End;     // End frame   0x5555;
} ;
#pragma pack(pop) 

inline QDataStream & operator >> ( QDataStream& in, TArduinoData& out )
{
	in.setByteOrder(QDataStream::LittleEndian );  
	in.setFloatingPointPrecision(QDataStream::SinglePrecision ); 

    in >> out.Begin   >> out.Code
       >> out.Gyro[0] >> out.Gyro[1] >> out.Gyro[2]
       >> out.Acc[0]  >> out.Acc[1]  >> out.Acc[2]
       >> out.End;
	return in;
}

#endif
