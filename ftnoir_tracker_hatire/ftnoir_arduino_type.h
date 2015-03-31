#ifndef FTNOIR_TARDUINO_TYPE_H
#define FTNOIR_TARDUINO_TYPE_H

#include <QDataStream>

// Arduino trame structure
#pragma pack(push,2)
struct TArduinoData
{
	quint16  Begin;    // Header trame 0xAAAA;
	quint16  Code;     // 0->999 Num Trame  >=2000  Info >=3000 Init  >=5000 Start Command  >=9000 Error 
	float Rot[3];  
	float Trans[3];
	quint16  End;     // End trame   0x5555;
} ;
#pragma pack(pop) 


inline QDataStream & operator >> ( QDataStream& in, TArduinoData& out )
{
	in.setFloatingPointPrecision(QDataStream::SinglePrecision ); 

    in >> out.Begin  >> out.Code
       >> out.Rot[0] >> out.Rot[1]  >> out.Rot[2]
       >> out.Trans[0] >> out.Trans[1] >> out.Trans[2]
       >> out.End;
	return in;
}



#endif
