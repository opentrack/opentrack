#include "ftnoir_tracker_openhmd.h"
#include "compat/euler.hpp"
#include <cmath>
#include <QDebug>

openhmd_tracker::openhmd_tracker() {}

openhmd_tracker::~openhmd_tracker()
{
    should_quit = true;
    requestInterruption();
    wait();
    
    if (device)
        ohmd_close_device(device);
    if (ctx)
        ohmd_ctx_destroy(ctx);
}

module_status openhmd_tracker::start_tracker(QFrame*)
{
    ctx = ohmd_ctx_create();
    if (!ctx)
        return error(tr("Failed to create OpenHMD context"));
    
    int num_devices = ohmd_ctx_probe(ctx);
    
    if (num_devices <= 0)
        return error(tr("No OpenHMD devices found"));
    
    qDebug() << "OpenHMD: Found" << num_devices << "device(s)";
    
    int idx = s.device_index;
    if (idx >= num_devices)
        idx = 0;
    
    device = ohmd_list_open_device(ctx, idx);
    if (!device)
        return error(tr("Failed to open device %1").arg(idx));
    
    // Get device info
    int device_class = 0, device_flags = 0;
    ohmd_device_geti(device, OHMD_DEVICE_CLASS, &device_class);
    ohmd_device_geti(device, OHMD_DEVICE_FLAGS, &device_flags);
    
    const char* vendor = ohmd_list_gets(ctx, idx, OHMD_VENDOR);
    const char* product = ohmd_list_gets(ctx, idx, OHMD_PRODUCT);
    
    qDebug() << "OpenHMD: Opened device:" << vendor << product;
    qDebug() << "OpenHMD: Rotation tracking:" 
             << ((device_flags & OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING) ? "YES" : "NO");
    
    start();
    return status_ok();
}

void openhmd_tracker::run()
{
    while (!should_quit && !isInterruptionRequested())
    {
        if (!device)
            break;
        
        // Update device state (triggers IMU read)
        ohmd_ctx_update(ctx);
        
        // Read quaternion rotation
        float quat[4];
        ohmd_device_getf(device, OHMD_ROTATION_QUAT, quat);
        
        // Deterministic conversion using fixed frame transform (no Euler order guessing)
        double yaw, pitch, roll;
        quat_to_euler(quat, yaw, pitch, roll);

        if ((bool)s.invert_yaw)
            yaw = -yaw;
        if ((bool)s.invert_pitch)
            pitch = -pitch;
        if ((bool)s.invert_roll)
            roll = -roll;
        
        // Update pose (rotation only, no translation for DK1)
        {
            QMutexLocker lock(&mutex);
            last_pose[0] = 0;      // TX
            last_pose[1] = 0;      // TY
            last_pose[2] = 0;      // TZ
            last_pose[3] = yaw;    // Yaw (degrees)
            last_pose[4] = pitch;  // Pitch (degrees)
            last_pose[5] = roll;   // Roll (degrees)
        }
        
        // 1000Hz target (1ms sleep)
        QThread::usleep(1000);
    }
}

void openhmd_tracker::data(double *data)
{
    QMutexLocker lock(&mutex);
    for (int i = 0; i < 6; i++)
        data[i] = last_pose[i];
}

void openhmd_tracker::quat_to_euler(const float q[4], double &yaw, double &pitch, double &roll)
{
    // OpenHMD returns quaternion as [x, y, z, w]
    const double x = q[0], y = q[1], z = q[2], w = q[3];

    // Quaternion -> OpenGL/right-handed rotation matrix
    euler::rmat Rg {
        1.0 - 2.0*(y*y + z*z), 2.0*(x*y - z*w),       2.0*(x*z + y*w),
        2.0*(x*y + z*w),       1.0 - 2.0*(x*x + z*z), 2.0*(y*z - x*w),
        2.0*(x*z - y*w),       2.0*(y*z + x*w),       1.0 - 2.0*(x*x + y*y)
    };

    // OpenTrack Euler extraction frame E from OpenGL frame G (used elsewhere in project):
    // -z_G -> x_E, x_G -> -y_E, y_G -> z_E
    const euler::rmat R_EG {
        0,  0, -1,
       -1,  0,  0,
        0,  1,  0
    };

    // Transform orientation into OpenTrack Euler frame and extract angles.
    const euler::rmat Re = R_EG * Rg * R_EG.t();
    const euler::Pose_ ypr = euler::rmat_to_euler(Re);

    constexpr double rad2deg = 180.0 / M_PI;
    yaw = ypr(0) * rad2deg;
    pitch = ypr(1) * rad2deg;
    roll = ypr(2) * rad2deg;
}

OPENTRACK_DECLARE_TRACKER(openhmd_tracker, openhmd_dialog, openhmd_metadata)
