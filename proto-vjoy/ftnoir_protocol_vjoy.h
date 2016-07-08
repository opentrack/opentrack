/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */
#pragma once
#include "ui_ftnoir_vjoy_controls.h"
#include <cmath>
#include "opentrack/plugin-api.hpp"

#define FT_PROGRAMID "FT_ProgramID"

class FTNoIR_Protocol : public IProtocol
{
public:
    FTNoIR_Protocol();
    ~FTNoIR_Protocol() override;
    bool correct() {
        return true;
    }
    void pose( const double *headpose );
    QString game_name() {
        return "Virtual joystick";
    }
private:
};

// Widget that has controls for FTNoIR protocol client-settings.
class VJoyControls: public IProtocolDialog
{
    Q_OBJECT
public:

    explicit VJoyControls();
    void register_protocol(IProtocol *) {}
    void unregister_protocol() {}

private:
    Ui::UICVJoyControls ui;
    void save();

private slots:
    void doOK();
    void doCancel();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
    QString name() { return QString("Joystick emulation -- VJoy"); }
    QIcon icon() { return QIcon(":/images/vjoy.png"); }
};

#define VJOY_AXIS_MIN   -32768
#define VJOY_AXIS_NIL   0
#define VJOY_AXIS_MAX   32767

#include <windows.h>

#include <pshpack1.h>

typedef struct _JOYSTICK_STATE
{
    UCHAR ReportId;                         // Report Id
    SHORT XAxis;                            // X Axis
    SHORT YAxis;                            // Y Axis
    SHORT ZAxis;                            // Z Axis
    SHORT XRotation;                        // X Rotation
    SHORT YRotation;                        // Y Rotation
    SHORT ZRotation;                        // Z Rotation
    SHORT Slider;                           // Slider
    SHORT Dial;                                     // Dial
    USHORT POV;                                     // POV
    UINT32 Buttons;                         // 32 Buttons
} JOYSTICK_STATE, * PJOYSTICK_STATE;

#ifndef _MSC_VER
#define VJOY_API extern "C" __declspec(dllimport)
VJOY_API BOOL __stdcall VJoy_Initialize(PCHAR name, PCHAR serial);
VJOY_API VOID __stdcall VJoy_Shutdown();
VJOY_API BOOL __stdcall VJoy_UpdateJoyState(int id, PJOYSTICK_STATE pJoyState);
#else
#define VJOY_API __declspec(dllimport)
VJOY_API BOOL __stdcall VJoy_Initialize(PCHAR name, PCHAR serial);
VJOY_API VOID __stdcall VJoy_Shutdown();
VJOY_API BOOL __stdcall VJoy_UpdateJoyState(int id, PJOYSTICK_STATE pJoyState);
#endif
