#include "ftnoir_tracker_joystick.h"
#include "facetracknoir/global-settings.h"
#undef NDEBUG
#include <cassert>
#include <QMutexLocker>

static BOOL CALLBACK EnumJoysticksCallback2( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
    auto self = ( FTNoIR_Tracker* )pContext;

    self->def = *pdidInstance;

    return self->iter++ == self->joyid ? DIENUM_STOP : DIENUM_CONTINUE;
}

FTNoIR_Tracker::FTNoIR_Tracker() :
    g_pDI(nullptr),
    g_pJoystick(nullptr),
    joyid(-1),
    iter(-1),
    mtx(QMutex::Recursive)
{
    for (int i = 0; i < 6; i++)
        axes[i] = min_[i] = max_[i] = 0;
    GUID bar = {0};
    preferred = bar;
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

    g_pJoystick = nullptr;
    g_pDI = nullptr;

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

        // Set the range for the axis

        if( FAILED( self->g_pJoystick->GetProperty( DIPROP_RANGE, &diprg.diph ) ) )
            return DIENUM_STOP;

        self->min_[self->iter] = diprg.lMin;
        self->max_[self->iter] = diprg.lMax;
        qDebug() << "axis" << self->iter << diprg.lMin << diprg.lMax;
        self->iter++;
    }

    return self->iter == 8 ? DIENUM_STOP : DIENUM_CONTINUE;
}

static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
    DI_ENUM_CONTEXT* pEnumContext = ( DI_ENUM_CONTEXT* )pContext;

    if (!IsEqualGUID(pEnumContext->preferred_instance, pdidInstance->guidInstance))
        return DIENUM_CONTINUE;

    (void) pEnumContext->g_pDI->CreateDevice( pdidInstance->guidInstance, pEnumContext->g_pJoystick, NULL);

    return DIENUM_STOP;
}

void FTNoIR_Tracker::StartTracker(QFrame* frame)
{
    QMutexLocker foo(&mtx);
    this->frame = frame;
    iter = 0;
    loadSettings();
    auto hr = CoInitialize( nullptr );
    DI_ENUM_CONTEXT enumContext = {0};

    if( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION,
                                         IID_IDirectInput8, ( VOID** )&g_pDI, NULL ) ) )
    {
        qDebug() << "create";
        goto fail;
    }

    if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL,
                                         EnumJoysticksCallback2,
                                         this,
                                         DIEDFL_ATTACHEDONLY)))
    {
        qDebug() << "enum2";
        goto fail;
    }

    enumContext.g_pDI = g_pDI;
    enumContext.g_pJoystick = &g_pJoystick;
    enumContext.preferred_instance = def.guidInstance;

    if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL,
                                         EnumJoysticksCallback,
                                         &enumContext,
                                         DIEDFL_ATTACHEDONLY)))
    {
        qDebug() << "enum1";
        goto fail;
    }

    if (!g_pJoystick)
    {
        qDebug() << "ENODEV";
        goto fail;
    }

    if (FAILED(g_pJoystick->SetDataFormat(&c_dfDIJoystick)))
    {
        qDebug() << "format";
        goto fail;
    }

    if (FAILED(g_pJoystick->SetCooperativeLevel((HWND) frame->window()->winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
    {
        qDebug() << "coop";
        goto fail;
    }

    iter = 0;

    if( FAILED( hr = g_pJoystick->EnumObjects( EnumObjectsCallback,
                                               ( VOID* )this, DIDFT_ALL )))
    {
        qDebug() << "enum axes";
        goto fail;
    }

    qDebug() << "joy init success";

    return;

fail:
    if (g_pJoystick)
        g_pJoystick->Release();
    if (g_pDI)
        g_pDI->Release();
    g_pJoystick = nullptr;
    g_pDI = nullptr;

    qDebug() << "joy init failure";
}

void FTNoIR_Tracker::GetHeadPoseData(double *data)
{
    QMutexLocker foo(&mtx);
    DIJOYSTATE js = {0};

    if( !g_pDI || !g_pJoystick)
        return;

    auto hr = g_pJoystick->Poll();
    if( FAILED( hr ))
    {
        hr = g_pJoystick->Acquire();
        for (int i = 0; hr == DIERR_INPUTLOST && i < 200; i++)
            hr = g_pJoystick->Acquire();
        if (hr != DI_OK)
            return;
    }

    if( FAILED( hr = g_pJoystick->GetDeviceState( sizeof( js ), &js ) ) )
        return;

    const LONG values[] = {
        js.lRx,
        js.lRy,
        js.lRz,
        js.lX,
        js.lY,
        js.lZ,
        js.rglSlider[0],
        js.rglSlider[1]
    };

    const double limits[] = {
        100,
        100,
        100,
        180,
        90,
        180
    };

    for (int i = 0; i < 6; i++)
    {
        auto idx = axes[i] - 1;
        if (idx < 0 || idx > 7)
        {
            data[i] = 0;
        }
        else {
            auto mid = (min_[idx] + max_[idx]) / 2;
            auto val = values[idx] - mid;

            auto max = (max_[idx] - min_[idx]) / 2;
            auto min = (min_[idx] - max_[idx]) / -2;
            data[i] = val * limits[i] / (double) (val >= 0 ? max : min);
        }
    }
}

void FTNoIR_Tracker::loadSettings() {

    QMutexLocker foo(&mtx);
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
