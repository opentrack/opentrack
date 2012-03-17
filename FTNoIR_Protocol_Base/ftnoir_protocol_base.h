#ifndef FTNOIR_PROTOCOL_BASE_H
#define FTNOIR_PROTOCOL_BASE_H

#include "ftnoir_protocol_base_global.h"
#include "..\ftnoir_tracker_base\ftnoir_tracker_types.h"
#include <QtGui/QWidget>
#include <QtGui/QFrame>
//#include "winbase.h"

#include "windows.h"
#include "winable.h"

// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct IProtocol
{
    virtual void Release() = 0;									// Member required to enable Auto-remove
	virtual void Initialize() = 0;
	virtual bool checkServerInstallationOK ( HANDLE handle ) = 0;
	virtual void sendHeadposeToGame( T6DOF *headpose ) = 0;
	virtual void getNameFromGame( char *dest ) = 0;				// Take care dest can handle up to 100 chars...
};

// Handle type. In C++ language the iterface type is used.
typedef IProtocol* PROTOCOLHANDLE;

////////////////////////////////////////////////////////////////////////////////
// 
#ifdef __cplusplus
#   define EXTERN_C     extern "C"
#else
#   define EXTERN_C
#endif // __cplusplus

// Factory function that creates instances of the Protocol object.
EXTERN_C
FTNOIR_PROTOCOL_BASE_EXPORT
PROTOCOLHANDLE
__stdcall
GetProtocol(
    void);


// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct IProtocolDialog
{
    virtual void Release() = 0;									// Member required to enable Auto-remove
	virtual void Initialize(QWidget *parent) = 0;
};

// Handle type. In C++ language the iterface type is used.
typedef IProtocolDialog* PROTOCOLDIALOGHANDLE;

// Factory function that creates instances of the Protocol object.
EXTERN_C
FTNOIR_PROTOCOL_BASE_EXPORT
PROTOCOLDIALOGHANDLE
__stdcall
GetProtocolDialog(void);


#endif // FTNOIR_PROTOCOL_BASE_H
