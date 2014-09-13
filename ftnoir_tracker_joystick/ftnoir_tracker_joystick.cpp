#include "ftnoir_tracker_joystick.h"
#include "facetracknoir/plugin-support.h"
#include <QMutexLocker>

FTNoIR_Tracker::FTNoIR_Tracker() :
    g_pDI(nullptr),
    g_pJoystick(nullptr),
    iter(-1),
    mtx(QMutex::Recursive)
{
    GUID bar = {0};
}

void FTNoIR_Tracker::reload()
{
    s.b->reload();
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
	{
        g_pDI->Release();
	}
}

static BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                   VOID* pContext )
{
    auto self = (FTNoIR_Tracker*) pContext;
	
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg = {0};
        diprg.diph.dwSize = sizeof( DIPROPRANGE );
        diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType;
	diprg.lMax = 65535;
	diprg.lMin = -65536;

        // Set the range for the axis

        if( FAILED( self->g_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
            return DIENUM_STOP;

        self->iter++;
    }

    return self->iter == 8 ? DIENUM_STOP : DIENUM_CONTINUE;
}

static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
	auto self = reinterpret_cast<FTNoIR_Tracker*>(pContext);
	bool stop = QString(pdidInstance->tszInstanceName) == self->s.joyid;

	if (stop)
	{
		(void) self->g_pDI->CreateDevice( pdidInstance->guidInstance, &self->g_pJoystick, NULL);
		qDebug() << "device" << static_cast<QString>(self->s.joyid);
	}

    return stop ? DIENUM_STOP : DIENUM_CONTINUE;
}

void FTNoIR_Tracker::StartTracker(QFrame* frame)
{
    QMutexLocker foo(&mtx);
    this->frame = frame;
    iter = 0;
    auto hr = CoInitialize( nullptr );

    if( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION,
                                         IID_IDirectInput8, ( VOID** )&g_pDI, NULL ) ) )
    {
        qDebug() << "create";
        goto fail;
    }
	
	if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL,
                                         EnumJoysticksCallback,
                                         this,
                                         DIEDFL_ATTACHEDONLY)))
    {
        qDebug() << "enum2";
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

	bool ok = false;

	for (int i = 0; i < 100; i++)
	{
		if (!FAILED(g_pJoystick->Poll()))
		{
			ok = true;
			break;
		}
		if (g_pJoystick->Acquire() != DI_OK)
			continue;
		else
			ok = true;
		break;
	}

	if (!ok)
		return;

	HRESULT hr = 0;

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
	
	int axes[] = {
		s.axis_0,
		s.axis_1,
		s.axis_2,
		s.axis_3,
		s.axis_4,
		s.axis_5
	};

    for (int i = 0; i < 6; i++)
    {
        auto idx = axes[i] - 1;
        if (idx < 0 || idx > 7)
        {
            data[i] = 0;
        }
	else
		data[i] = values[i] * limits[i] / AXIS_MAX;
    }
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Tracker;
}
