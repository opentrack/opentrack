#ifndef FTNOIR_TARDUINO_TYPE_H
#define FTNOIR_TARDUINO_TYPE_H

// Arduino trame structure
#pragma pack(push,2)
struct TArduinoData
{
	quint16  Begin;    // Header trame 0xAAAA;
	quint16  Code;     // 0->999 Num Trame  >=2000  Info >=3000 Init  >=5000 Start Command  >=9000 Error 
	float Gyro[3];  
	float Acc[3];
	quint16  End;     // End trame   0x5555;
} ;
#pragma pack(pop) 


inline QDataStream & operator >> ( QDataStream& in, TArduinoData& out )
{
	in.setByteOrder(QDataStream::LittleEndian );  
	in.setFloatingPointPrecision(QDataStream::SinglePrecision ); 

	in >> (quint16)out.Begin  >> (quint16)out.Code 
	   >> (float)out.Gyro[0] >> (float)out.Gyro[1]  >> (float)out.Gyro[2]	
	   >> (float)out.Acc[0]  >> (float)out.Acc[1] 	 >> (float)out.Acc[2]
	   >> (quint16)out.End;
	return in;
}



#endif