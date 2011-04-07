#ifndef __PPJIOCTL_H__
#define __PPJIOCTL_H__

#include "Windows.h" 

/* Define to use byte-size values for joystick axes, else dword size */
#undef UCHAR_AXES

#define	PPJOY_AXIS_MIN				1
#ifdef UCHAR_AXES
#define	PPJOY_AXIS_MAX				127
#else
#define	PPJOY_AXIS_MAX				32767
#endif

#define FILE_DEVICE_PPORTJOY			FILE_DEVICE_UNKNOWN

#define PPORTJOY_IOCTL(_index_)	\
	CTL_CODE (FILE_DEVICE_PPORTJOY, _index_, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PPORTJOY_SET_STATE		PPORTJOY_IOCTL (0x0)

#define	JOYSTICK_STATE_V1	0x53544143

typedef struct
{
 unsigned long Version;
 unsigned char Data[1];
} JOYSTICK_SET_STATE, *PJOYSTICK_SET_STATE;


#endif
