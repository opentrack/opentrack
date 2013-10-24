#include "ftnoir_tracker_joystick.h"
#include "facetracknoir/global-settings.h"
#undef NDEBUG
#include <cassert>
#include <QMutexLocker>

FTNoIR_Tracker::FTNoIR_Tracker() :
    g_pDI(nullptr),
    g_pJoystick(nullptr),
    joyid(-1)
{
    for (int i = 0; i < 6; i++)
        axes[i] = -1;
    GUID foo = {0};
    preferred = foo;
}

void FTNoIR_Tracker::reload()
{
    QMutexLocker foo(&mtx);
    if (g_pJoystick)
    {
        g_pJoystick->Unacquire();
        g_pJoystick->Release();
    }
    if (g_pDI)
        g_pDI->Release();

    StartTracker(frame);
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    if (g_pJoystick)
    {
        g_pJoystick->Unacquire();
        g_pJoystick->Release();
    }
    if (g_pDI)
        g_pDI->Release();
}

static BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                   VOID* pContext )
{
    auto self = (FTNoIR_Tracker*) pContext;

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg = {0};
        diprg.diph.dwSize = sizeof( DIPROPRANGE );
        diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType;
        diprg.lMin = -AXIS_MAX;
        diprg.lMax = AXIS_MAX;

        // Set the range for the axis
        if( FAILED( self->g_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
            return DIENUM_STOP;
    }

    return DIENUM_CONTINUE;
}

static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
    DI_ENUM_CONTEXT* pEnumContext = ( DI_ENUM_CONTEXT* )pContext;

    if (!IsEqualGUID(pEnumContext->preferred_instance, pdidInstance->guidInstance))
        return DIENUM_CONTINUE;

    if (SUCCEEDED(pEnumContext->g_pDI->CreateDevice( pdidInstance->guidInstance,
                                                     pEnumContext->g_pJoystick, NULL )))
        pEnumContext->bPreferredJoyCfgValid = true;

    return DIENUM_STOP;
}

void FTNoIR_Tracker::StartTracker(QFrame* win)
{
    QMutexLocker foo(&mtx);
    frame = win;
    loadSettings();
    auto hr = CoInitialize( nullptr );
    IDirectInputJoyConfig8* pJoyConfig = nullptr;
    DIJOYCONFIG PreferredJoyCfg = {0};
    DI_ENUM_CONTEXT enumContext = {0};
    enumContext.pPreferredJoyCfg = &PreferredJoyCfg;
    enumContext.bPreferredJoyCfgValid = false;
    enumContext.g_pJoystick = &g_pJoystick;

    if( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION,
                                         IID_IDirectInput8, ( VOID** )&g_pDI, NULL ) ) )
        goto fail;

    if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL,
                                         EnumJoysticksCallback,
                                         &enumContext, DIEDFL_ATTACHEDONLY ) ) )
        goto fail;

    PreferredJoyCfg.dwSize = sizeof( PreferredJoyCfg );
    if( SUCCEEDED( pJoyConfig->GetConfig( 0, &PreferredJoyCfg, DIJC_GUIDINSTANCE ) ) )
        enumContext.bPreferredJoyCfgValid = true;
    if (pJoyConfig)
    {
        pJoyConfig->Release();
        pJoyConfig = nullptr;
    }

    assert((!!enumContext.bPreferredJoyCfgValid) == !!(g_pJoystick != nullptr));

    if (FAILED(g_pJoystick->SetDataFormat(&c_dfDIJoystick2)))
        goto fail;

    if (FAILED(g_pJoystick->SetCooperativeLevel((HWND) win->winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
        goto fail;

    if( FAILED( hr = g_pJoystick->EnumObjects( EnumObjectsCallback,
                                               ( VOID* )this, DIDFT_ALL ) ) )
        goto fail;

    return;

fail:
    if (g_pJoystick)
        g_pJoystick->Release();
    if (g_pDI)
        g_pDI->Release();
    g_pJoystick = nullptr;
    g_pDI = nullptr;
}

bool FTNoIR_Tracker::GiveHeadPoseData(double *data)
{
    QMutexLocker foo(&mtx);
    DIJOYSTATE2 js;

    if( !g_pJoystick)
        return false;

start:
    auto hr = g_pJoystick->Poll();
    if( FAILED( hr ) )
    {
        hr = g_pJoystick->Acquire();
        while( hr == DIERR_INPUTLOST )
            hr = g_pJoystick->Acquire();
        goto start;
    }

    if( FAILED( hr = g_pJoystick->GetDeviceState( sizeof( DIJOYSTATE2 ), &js ) ) )
        return false;

    const LONG values[] = {
        js.lX,
        js.lY,
        js.lZ,
        js.lRx,
        js.lRy,
        js.lRz,
        js.rglSlider[0],
        js.rglSlider[1]
    };

    for (int i = 0; i < 6; i++)
    {
        auto idx = axes[i] - 1;
        if (idx < 0 || idx > 7)
        {
            data[i] = 0;
        }
        else {
            data[i] = values[i] / (double) AXIS_MAX;
        }
    }

	return true;
}

void FTNoIR_Tracker::loadSettings() {

	QSettings settings("opentrack");	// Registry settings (in HK_USER)
	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)
    iniFile.beginGroup ( "tracker-joy" );
    joyid = iniFile.value("joyid", -1).toInt();
    for (int i = 0; i < 6; i++)
        axes[i] = iniFile.value(QString("axis-%1").arg(i), 0).toInt() - 1;
    iniFile.endGroup ();
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Tracker;
}
