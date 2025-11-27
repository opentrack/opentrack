/*
 * Copyright (c) 2017, Benjamin Flegel
 * Copyright (c) 2017, Stanislaw Halik
 * Copyright (c) 2017, Anthony Coddington
 * Copyright (c) 2025, Sander Saarend
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "steamvr.hpp"

#include "api/plugin-api.hpp"

#include <cmath>
#include <cstdlib>
#include <algorithm>

#include <QMessageBox>
#include <QDebug>
#include <QCoreApplication>
#include <QThread>

QRecursiveMutex device_list::mtx;

template<typename F>
auto with_vr_lock(F&& fun) -> decltype(fun(vr_t(), vr_error_t()))
{
    QMutexLocker l(&device_list::mtx);
    auto [v, e] = device_list::vr_init();
    return fun(v, e);
}

void device_list::fill_device_specs(QList<device_spec>& list)
{
    with_vr_lock([&](vr_t v, vr_error_t)
    {
        list.clear();

        pose_t device_states[max_devices];

        if (!v)
            return;

        v->GetDeviceToAbsoluteTrackingPose(origin::TrackingUniverseSeated, 0,
                                           device_states, vr::k_unMaxTrackedDeviceCount);

        constexpr unsigned bufsiz = vr::k_unMaxPropertyStringSize;
        static char str[bufsiz+1] {}; // vr_lock prevents reentrancy

        for (unsigned k = 0; k < vr::k_unMaxTrackedDeviceCount; k++)
        {
            if (v->GetTrackedDeviceClass(k) == vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid)
            {
                qDebug() << "steamvr: no device with index";
                continue;
            }

            if (!device_states[k].bDeviceIsConnected)
            {
                qDebug() << "steamvr: device not connected but proceeding";
                continue;
            }

            unsigned len;

            len = v->GetStringTrackedDeviceProperty(k, vr::ETrackedDeviceProperty::Prop_SerialNumber_String, str, bufsiz);
            if (!len)
            {
                qDebug() << "steamvr: getting serial number failed for" << k;
                continue;
            }

            device_spec dev;

            dev.serial = str;

            len = v->GetStringTrackedDeviceProperty(k, vr::ETrackedDeviceProperty::Prop_ModelNumber_String, str, bufsiz);
            if (!len)
            {
                qDebug() << "steamvr: getting model number failed for" << k;
                continue;
            }

            switch (v->GetTrackedDeviceClass(k))
            {
            using enum vr::ETrackedDeviceClass;
            case TrackedDeviceClass_HMD:
                dev.type = "HMD"; break;
            case TrackedDeviceClass_Controller:
                dev.type = "Controller"; break;
            case TrackedDeviceClass_TrackingReference:
                dev.type = "Tracking reference"; break;
            case TrackedDeviceClass_DisplayRedirect:
                dev.type = "Display redirect"; break;
            case TrackedDeviceClass_GenericTracker:
                dev.type = "Generic"; break;
            default:
                dev.type = "Unknown"; break;
            }

            dev.model = str;
            dev.pose = device_states[k];
            dev.k = k;
            dev.is_connected = device_states[k].bDeviceIsConnected;

            list.push_back(dev);
        }
    });
}

device_list::device_list()
{
    refresh_device_list();
}

void device_list::refresh_device_list()
{
    device_specs.clear();
    device_specs.reserve(max_devices);
    fill_device_specs(device_specs);
}

device_list::maybe_pose device_list::get_pose(int k)
{
    if (!(unsigned(k) < max_devices))
        return maybe_pose(false, pose_t{});

    return with_vr_lock([k](vr_t v, vr_error_t)
    {
        static pose_t poses[max_devices] {}; // vr_lock removes reentrancy

        v->GetDeviceToAbsoluteTrackingPose(origin::TrackingUniverseSeated, 0,
                                           poses, max_devices);

        const pose_t& pose = poses[k];

        if (pose.bPoseIsValid && pose.bDeviceIsConnected)
            return maybe_pose{ true, poses[k] };
        else
            eval_once(qDebug() << "steamvr:"
                               << "no valid pose from device" << k
                               << "valid" << pose.bPoseIsValid
                               << "connected" << pose.bDeviceIsConnected);

        return maybe_pose{ false, {} };
    });
}

tt device_list::vr_init()
{
    static tt t = vr_init_();
    return t;
}

tt device_list::vr_init_()
{
    vr_error_t error = vr_error_t::VRInitError_Unknown;
    vr_t v = vr::VR_Init(&error, vr::EVRApplicationType::VRApplication_Other);

    if (v)
        std::atexit(vr::VR_Shutdown);
    else
        qDebug() << "steamvr: init failure" << error << device_list::error_string(error);

    return { v, error };
}

QString device_list::error_string(vr_error_t err)
{
    const char* str = vr::VR_GetVRInitErrorAsSymbol(err);
    const char* desc = vr::VR_GetVRInitErrorAsEnglishDescription(err);

    if (!desc)
        desc = "No description";

    if (str)
        return QStringLiteral("%1: %2").arg(str, desc);
    else
        return { "Unknown error" };
}

steamvr::steamvr() = default;
steamvr::~steamvr() = default;

module_status steamvr::start_tracker(QFrame*)
{
    return with_vr_lock([this](vr_t v, vr_error_t e)
    {
        QString err;

        if (!v)
        {
            err = device_list::error_string(e);
            return error(err);
        }

        const QString serial = s.device_serial().toString();
        device_list d;
        const QList<device_spec>& specs = d.devices();
        const int sz = specs.count();

        if (sz == 0)
            err = tr("No HMD connected");

        for (const device_spec& spec : specs)
        {
            if (serial.isEmpty() || serial == spec.to_string())
            {
                device_index = spec.k;
                break;
            }
        }

        if (device_index == UINT_MAX && err.isEmpty())
            err = tr("Can't find device with that serial");

        if (err.isEmpty())
        {
            if (auto* c = vr::VRCompositor(); c != nullptr)
            {
                c->SetTrackingSpace(origin::TrackingUniverseSeated);
                
                QMutexLocker lock(&calibration_mtx);
                calibration_ready = false;
                steamvr::mat34 m;
                if (s.calibration_enabled() && from_variant_list(s.calibration_matrix(), m))
                {
                    calibration_offset = m;
                    calibration_ready = true;
                }
                else if (s.calibration_enabled())
                {
                    eval_once(qDebug() << "steamvr: failed to load calibration matrix");
                }
                return status_ok();
            }
            else
                return error("vr::VRCompositor == NULL");
        }

      return error(err);
    });
}

void steamvr::data(double* data)
{
    if (device_index != UINT_MAX)
    {
        auto [ok, pose] = device_list::get_pose(device_index);
        if (ok)
        {
            // Scale factor for converting meters to centimeters
            constexpr int c = 10;

            const auto& result_raw = pose.mDeviceToAbsoluteTracking;
            vr::HmdMatrix34_t corrected = result_raw;

            // Apply calibration offset if available
            {
                QMutexLocker lock(&calibration_mtx);
                if (calibration_ready)
                {
                    mat34 raw = from_vr_matrix(result_raw);
                    mat34 corrected_mat = multiply(raw, calibration_offset);
                    corrected = to_vr_matrix(corrected_mat);
                }
            }

            data[TX] = (double)(-corrected.m[0][3] * c);
            data[TY] = (double)(corrected.m[1][3] * c);
            data[TZ] = (double)(corrected.m[2][3] * c);

            matrix_to_euler(data[Yaw], data[Pitch], data[Roll], corrected);

            constexpr double r2d = 180 / M_PI;
            data[Yaw] *= r2d; data[Pitch] *= r2d; data[Roll] *= r2d;
        }
    }
}

bool steamvr::center()
{
    return with_vr_lock([&](vr_t v, vr_error_t)
    {
        if (v)
        {
            if (v->GetTrackedDeviceClass(device_index) == vr::ETrackedDeviceClass::TrackedDeviceClass_HMD)
            {
                auto* c = vr::VRChaperone();
                if (!c)
                {
                    eval_once(qDebug() << "steamvr: vr::VRChaperone == NULL");
                    return false;
                }
                else
                {
                    c->ResetZeroPose(origin::TrackingUniverseSeated);
                    // Use chaperone universe real world up instead of opentrack's initial pose centering
                    // Note: Controllers will be centered based on initial headset position.
                    return true;
                }
            }
            else
                // with controllers, resetting the seated pose does nothing
                return false;
        }
        return false;
    });
}

void steamvr::matrix_to_euler(double& yaw, double& pitch, double& roll, const vr::HmdMatrix34_t& result)
{
    using d = double;

    yaw = std::atan2(d(result.m[2][0]), d(result.m[0][0]));
    pitch = std::atan2(-d(result.m[1][2]), d(result.m[1][1]));
    roll = std::asin(d(result.m[1][0]));
}

void steamvr::matrix_to_euler(double& yaw, double& pitch, double& roll, const mat34& mat)
{
    yaw = std::atan2(mat.m[2][0], mat.m[0][0]);
    pitch = std::atan2(-mat.m[1][2], mat.m[1][1]);
    roll = std::asin(mat.m[1][0]);
}

auto steamvr::identity_matrix() -> mat34
{
    mat34 out;
    out.m[0][0] = out.m[1][1] = out.m[2][2] = 1.0;
    return out;
}

auto steamvr::from_vr_matrix(const vr::HmdMatrix34_t& mat) -> mat34
{
    mat34 out;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 4; c++)
            out.m[r][c] = mat.m[r][c];
    return out;
}

auto steamvr::to_vr_matrix(const mat34& mat) -> vr::HmdMatrix34_t
{
    vr::HmdMatrix34_t out{};
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 4; c++)
            out.m[r][c] = float(mat.m[r][c]);
    return out;
}

// Multiplies two rigid transformation matrices (3x4 augmented format).
// Computes C = A * B where matrices are in [R|t] form (3x3 rotation + 3x1 translation).
auto steamvr::multiply(const mat34& a, const mat34& b) -> mat34
{
    mat34 out{};
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            double v = 0;
            for (int k = 0; k < 3; k++)
                v += a.m[r][k] * b.m[k][c];
            out.m[r][c] = v;
        }

        double t = 0;
        for (int k = 0; k < 3; k++)
            t += a.m[r][k] * b.m[k][3];
        out.m[r][3] = t + a.m[r][3];
    }
    return out;
}

// Serializes a 3x4 matrix to QVariantList for storage.
// Row-major order: [m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23]
auto steamvr::to_variant_list(const mat34& mat) -> QVariantList
{
    QVariantList list;
    list.reserve(12);
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 4; c++)
            list << mat.m[r][c];
    return list;
}

// Deserializes a 3x4 matrix from QVariantList.
// Expects 12 elements in row-major order.
bool steamvr::from_variant_list(const QVariantList& list, mat34& out)
{
    if (list.size() != 12)
        return false;

    for (int i = 0; i < 12; i++)
        out.m[i / 4][i % 4] = list[i].toDouble();
    return true;
}

namespace {
// Solves a 3x3 linear system Ax = b using Gaussian elimination with partial pivoting.
// Modifies A and b in place. Returns false if the system is singular (determinant near zero).
// This is used in calibration to solve for the tracker-to-head offset vector.
bool solve_3x3(double A[3][3], double b[3], double out[3])
{
    constexpr double epsilon = 1e-9;
    
    for (int i = 0; i < 3; i++)
    {
        // Find pivot row
        int pivot = i;
        for (int r = i + 1; r < 3; r++)
            if (std::abs(A[r][i]) > std::abs(A[pivot][i]))
                pivot = r;

        // Check for singular matrix
        if (std::abs(A[pivot][i]) < epsilon)
            return false;

        // Swap rows if needed
        if (pivot != i)
        {
            for (int c = 0; c < 3; c++)
                std::swap(A[i][c], A[pivot][c]);
            std::swap(b[i], b[pivot]);
        }

        // Normalize pivot row
        const double div = A[i][i];
        for (int c = i; c < 3; c++)
            A[i][c] /= div;
        b[i] /= div;

        // Eliminate column in other rows
        for (int r = 0; r < 3; r++)
        {
            if (r == i) continue;
            const double factor = A[r][i];
            for (int c = i; c < 3; c++)
                A[r][c] -= factor * A[i][c];
            b[r] -= factor * b[i];
        }
    }

    for (int i = 0; i < 3; i++)
        out[i] = b[i];
    return true;
}
} // namespace

steamvr_dialog::steamvr_dialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.runCalibration, &QPushButton::clicked, this, &steamvr_dialog::doRunCalibration);
    connect(ui.clearCalibration, &QPushButton::clicked, this, &steamvr_dialog::doClearCalibration);

    ui.device->clear();
    ui.device->addItem("First available", QVariant(QVariant::String));

    device_list list;
    for (const device_spec& spec : list.devices())
    {
        QString text = spec.to_string();
        if (!spec.is_connected)
            text = QStringLiteral("%1 [disconnected]").arg(text);
        ui.device->addItem(text, spec.to_string());
    }

    tie_setting(s.device_serial, ui.device);

    update_calibration_label();
}

void steamvr_dialog::doOK()
{
    s.b->save();
    close();
}

void steamvr_dialog::doCancel()
{
    close();
}

void steamvr_dialog::update_calibration_label()
{
    if (s.calibration_enabled())
        ui.calibrationStatus->setText(tr("Saved"));
    else
        ui.calibrationStatus->setText(tr("No calibration"));
}

void steamvr_dialog::doRunCalibration()
{
    run_calibration_wizard();
}

void steamvr_dialog::run_calibration_wizard()
{
    const QString serial = ui.device->currentData().toString();
    device_list d;
    unsigned idx = UINT_MAX;

    for (const device_spec& spec : d.devices())
    {
        if (serial.isEmpty() || serial == spec.to_string())
        {
            idx = spec.k;
            break;
        }
    }

    if (idx == UINT_MAX)
    {
        QMessageBox::warning(this, tr("Calibration"), tr("No device selected for calibration."));
        return;
    }

    // Step 1: Capture neutral pose
    QMessageBox::information(this, tr("Calibration"),
                             tr("Sit upright and face the screen directly. Keep your head still.\n\n"
                                "Press OK to capture your neutral pose."));

    // Check device stability by sampling multiple poses
    constexpr int stability_samples = 10;
    constexpr double max_position_variance = 0.001; // 1mm variance
    std::vector<steamvr::mat34> stability_check_poses;
    stability_check_poses.reserve(stability_samples);
    
    for (int i = 0; i < stability_samples; i++)
    {
        auto [ok, pose] = device_list::get_pose(idx);
        if (!ok)
        {
            QMessageBox::warning(this, tr("Calibration"), tr("Unable to read pose from the selected device."));
            return;
        }
        stability_check_poses.push_back(steamvr::from_vr_matrix(pose.mDeviceToAbsoluteTracking));
        QThread::msleep(50); // 50ms between samples
    }
    
    // Check position stability
    double mean_pos[3] = {0, 0, 0};
    for (const auto& p : stability_check_poses)
        for (int i = 0; i < 3; i++)
            mean_pos[i] += p.m[i][3];
    for (int i = 0; i < 3; i++)
        mean_pos[i] /= stability_samples;
    
    double variance = 0;
    for (const auto& p : stability_check_poses)
        for (int i = 0; i < 3; i++)
        {
            double diff = p.m[i][3] - mean_pos[i];
            variance += diff * diff;
        }
    variance /= (stability_samples * 3);
    
    if (variance > max_position_variance)
    {
        QMessageBox::warning(this, tr("Calibration"),
                           tr("Device tracking is unstable. Please ensure the device is properly tracking\n"
                              "and keep your head completely still during the neutral pose capture.\n\n"
                              "Position variance: %.2f mm").arg(std::sqrt(variance) * 1000));
        return;
    }
    
    // Use the most recent pose as the base pose
    steamvr::mat34 base_pose = stability_check_poses.back();

    // Step 2: Record movements with live feedback
    calibration_recording_dialog recording_dialog(idx, base_pose, this);
    steamvr::mat34 offset;
    
    if (!recording_dialog.run(offset))
    {
        // User cancelled or calibration failed
        return;
    }

    // Save calibration
    s.calibration_matrix = steamvr::to_variant_list(offset);
    s.calibration_enabled = true;
    s.b->save();

    update_calibration_label();
    QMessageBox::information(this, tr("Calibration"), tr("Calibration saved. It will be applied when tracking starts."));
}

void steamvr_dialog::doClearCalibration()
{
    s.calibration_enabled = false;
    s.calibration_matrix = QVariantList();
    s.b->save();
    update_calibration_label();
}

// Calibration Recording Dialog Implementation

calibration_recording_dialog::calibration_recording_dialog(unsigned device_idx, const steamvr::mat34& base_pose, QWidget* parent)
    : QDialog(parent), device_idx(device_idx), base_pose(base_pose)
{
    ui.setupUi(this);
    
    sample_timer = new QTimer(this);
    sample_timer->setInterval(sample_interval_ms);
    
    connect(ui.startButton, &QPushButton::clicked, this, &calibration_recording_dialog::on_startButton_clicked);
    connect(ui.finishButton, &QPushButton::clicked, this, &calibration_recording_dialog::on_finishButton_clicked);
    connect(ui.cancelButton, &QPushButton::clicked, this, &calibration_recording_dialog::on_cancelButton_clicked);
    connect(sample_timer, &QTimer::timeout, this, &calibration_recording_dialog::on_sample_timer);
}

bool calibration_recording_dialog::run(steamvr::mat34& out_offset)
{
    exec();
    
    if (!finished_successfully)
        return false;
    
    // Compute calibration from collected samples
    // Algorithm: Estimate the offset from tracker to head pivot point using least squares.
    // The constraint is: when the head rotates (changing R), the tracker translates (changing t),
    // but the head pivot point stays fixed. This gives us: dR * offset = -dp
    // We solve this overdetermined system in a least-squares sense.
    
    double ATA[3][3] {};
    double ATb[3] {};
    
    for (const auto& sample : samples)
    {
        // Change in position and rotation relative to base pose
        double dp[3] {
            sample.pose.m[0][3] - base_pose.m[0][3],
            sample.pose.m[1][3] - base_pose.m[1][3],
            sample.pose.m[2][3] - base_pose.m[2][3],
        };
        
        double dR[3][3];
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++)
                dR[r][c] = sample.pose.m[r][c] - base_pose.m[r][c];
        
        // Build normal equations: A^T * A and A^T * b
        for (int r = 0; r < 3; r++)
        {
            for (int c = 0; c < 3; c++)
            {
                ATb[c] -= dR[r][c] * dp[r];
                for (int k = 0; k < 3; k++)
                    ATA[c][k] += dR[r][c] * dR[r][k];
            }
        }
    }
    
    double offset_tracker_to_head[3] {};
    if (!solve_3x3(ATA, ATb, offset_tracker_to_head))
    {
        QMessageBox::warning(this, tr("Calibration Failed"),
                            tr("Unable to solve calibration equations. The system may be poorly conditioned.\n"
                               "Try making larger, smoother movements."));
        return false;
    }
    
    // Validate that the offset is reasonable (not a numerical artifact)
    constexpr double max_reasonable_offset_m = 0.5; // 50cm
    double offset_magnitude = std::sqrt(offset_tracker_to_head[0] * offset_tracker_to_head[0] +
                                       offset_tracker_to_head[1] * offset_tracker_to_head[1] +
                                       offset_tracker_to_head[2] * offset_tracker_to_head[2]);
    
    if (offset_magnitude > max_reasonable_offset_m)
    {
        QMessageBox::warning(this, tr("Calibration Failed"),
                            tr("Computed offset is unreasonably large (%.1f cm).\n"
                               "This usually means you moved your head during recording.\n"
                               "Please try again, keeping your head in the same spot while rotating.")
                            .arg(offset_magnitude * 100));
        return false;
    }
    
    // Build the calibration offset matrix:
    // - Rotation part: transpose of base rotation (aligns neutral pose to identity)
    // - Translation part: estimated offset from tracker to head pivot
    out_offset = steamvr::identity_matrix();
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
            out_offset.m[r][c] = base_pose.m[c][r]; // Transpose
        out_offset.m[r][3] = offset_tracker_to_head[r];
    }
    
    return true;
}

void calibration_recording_dialog::on_startButton_clicked()
{
    samples.clear();
    finished_successfully = false;

    // Initialize coverage to the current pose so neutral orientation doesn't count as movement
    double start_yaw = 0;
    double start_pitch = 0;
    double start_roll = 0;
    if (auto [ok, pose] = device_list::get_pose(device_idx); ok)
    {
        auto start_pose = steamvr::from_vr_matrix(pose.mDeviceToAbsoluteTracking);
        steamvr::matrix_to_euler(start_yaw, start_pitch, start_roll, start_pose);
    }
    min_yaw = max_yaw = start_yaw;
    min_pitch = max_pitch = start_pitch;

    ui.yawStatusLabel->setText("--");
    ui.pitchStatusLabel->setText("--");

    ui.startButton->setEnabled(false);
    ui.statusLabel->setText(tr("Recording..."));
    sample_timer->start();
}

void calibration_recording_dialog::on_finishButton_clicked()
{
    sample_timer->stop();
    finished_successfully = true;
    accept();
}

void calibration_recording_dialog::on_cancelButton_clicked()
{
    sample_timer->stop();
    finished_successfully = false;
    reject();
}

void calibration_recording_dialog::on_sample_timer()
{
    auto [ok, pose] = device_list::get_pose(device_idx);
    if (!ok)
        return;
    
    steamvr::mat34 current_pose = steamvr::from_vr_matrix(pose.mDeviceToAbsoluteTracking);
    
    sample_data sample;
    sample.pose = current_pose;
    double roll; // unused, but needed for the function call
    steamvr::matrix_to_euler(sample.yaw, sample.pitch, roll, current_pose);
    
    samples.push_back(sample);
    
    // Update coverage tracking
    min_yaw = std::min(min_yaw, sample.yaw);
    max_yaw = std::max(max_yaw, sample.yaw);
    min_pitch = std::min(min_pitch, sample.pitch);
    max_pitch = std::max(max_pitch, sample.pitch);
    
    update_ui();
    
    // Enable finish button when requirements are met
    bool enough_samples = samples.size() >= min_samples;
    bool good_coverage = check_coverage();
    ui.finishButton->setEnabled(enough_samples && good_coverage);
    
    if (good_coverage && enough_samples)
    {
        ui.statusLabel->setText(tr("Recording... (ready to finish)"));
    }
}

void calibration_recording_dialog::update_ui()
{
    // Update yaw coverage
    constexpr double deg = 180.0 / M_PI;
    double yaw_range = (max_yaw - min_yaw) * deg;
    double pitch_range = (max_pitch - min_pitch) * deg;
    
    if (samples.size() < 5)
    {
        ui.yawStatusLabel->setText("--");
        ui.pitchStatusLabel->setText("--");
    }
    else
    {
        if (yaw_range >= min_yaw_range_deg)
            ui.yawStatusLabel->setText(tr("Good"));
        else
            ui.yawStatusLabel->setText(tr("Move more"));
        
        if (pitch_range >= min_pitch_range_deg)
            ui.pitchStatusLabel->setText(tr("Good"));
        else
            ui.pitchStatusLabel->setText(tr("Move more"));
    }
}

bool calibration_recording_dialog::check_coverage() const
{
    constexpr double deg = 180.0 / M_PI;
    double yaw_range = (max_yaw - min_yaw) * deg;
    double pitch_range = (max_pitch - min_pitch) * deg;
    
    return yaw_range >= min_yaw_range_deg && pitch_range >= min_pitch_range_deg;
}

QString device_spec::to_string() const
{
    return QStringLiteral("<%1> %2 [%3]").arg(type, model, serial);
}

OPENTRACK_DECLARE_TRACKER(steamvr, steamvr_dialog, steamvr_metadata)
