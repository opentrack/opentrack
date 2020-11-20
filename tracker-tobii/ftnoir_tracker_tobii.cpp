#include "ftnoir_tracker_tobii.h"
#include "api/plugin-api.hpp"
#include "compat/math.hpp"

tobii::~tobii() = default;

module_status tobii::start_tracker(QFrame*)
{
    t.start();
    return status_ok();
}

void tobii::data(double *data)
{
    if (t.head_pose) {
        tobii_head_pose_t p = *t.head_pose;
        if (p.position_validity == TOBII_VALIDITY_VALID) {
            if (center_pose.position_validity == TOBII_VALIDITY_VALID) {
                p.position_xyz[0] = p.position_xyz[0] - center_pose.position_xyz[0];
                p.position_xyz[1] = p.position_xyz[1] - center_pose.position_xyz[1];
                p.position_xyz[2] = p.position_xyz[2] - center_pose.position_xyz[2];
            }
            else {
                center_pose = p;
            }
            data[0] = clamp(p.position_xyz[0] * 30.0 / 300.0, -30.0, 30.0);
            data[1] = clamp(p.position_xyz[1] * 30.0 / 300.0, -30.0, 30.0);
            data[2] = clamp(p.position_xyz[2] * 30.0 / 300.0, -30.0, 30.0);
        }

        double max_yaw = 90.0;
        if (p.rotation_validity_xyz[1] == TOBII_VALIDITY_VALID) {
            if (center_pose.rotation_validity_xyz[1] == TOBII_VALIDITY_VALID) {
                p.rotation_xyz[1] = p.rotation_xyz[1] - center_pose.rotation_xyz[1];
            }
            data[3] = clamp(p.rotation_xyz[1] * 100.0 * max_yaw / 90.0, -max_yaw, max_yaw);
        }

        double max_pitch = 90.0;
        if (p.rotation_validity_xyz[0] == TOBII_VALIDITY_VALID) {
            if (center_pose.rotation_validity_xyz[0] == TOBII_VALIDITY_VALID) {
                p.rotation_xyz[0] = p.rotation_xyz[0] - center_pose.rotation_xyz[0];
            }
            data[4] = clamp(p.rotation_xyz[0] * 100.0 * max_pitch / 90.0, -max_pitch, max_pitch);
        }

        double max_roll = 90.0;
        if (p.rotation_validity_xyz[2] == TOBII_VALIDITY_VALID) {
            if (center_pose.rotation_validity_xyz[2] == TOBII_VALIDITY_VALID) {
                p.rotation_xyz[2] = p.rotation_xyz[2] - center_pose.rotation_xyz[2];
            }
            data[5] = clamp(p.rotation_xyz[2] * 100.0 * max_roll / 90.0, -max_roll, max_roll);
        }
    }
}

bool tobii::center  ()
{
    if (t.head_pose) {
        center_pose = *t.head_pose;
    }
    return false;
}

OPENTRACK_DECLARE_TRACKER(tobii, dialog_tobii, tobiiDll)
