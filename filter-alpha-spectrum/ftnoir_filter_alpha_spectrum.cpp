#include "ftnoir_filter_alpha_spectrum.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
#include <QDebug>

alpha_spectrum::alpha_spectrum() = default;

namespace {
static double clamp01(double x) { return std::clamp(x, 0.0, 1.0); }
static double remap_with_threshold(double x, double threshold)
{
    if (x <= threshold)
        return 0.0;
    return clamp01((x - threshold) / std::max(1e-9, 1.0 - threshold));
}

// True Rényi/Tsallis likelihood from generalized alpha-spectrum.
// alpha > 15: near min-entropy regime (hard cutoff)
// alpha ~= 1: Shannon/Gaussian limit
// alpha < 1: heavy-tail regime
static double renyi_tsallis_likelihood(double mahalanobis_sq, double alpha)
{
    constexpr double eps = 1e-12;
    if (alpha > 15.0)
        return (mahalanobis_sq < 0.12) ? 1.0 : 1e-10;
    if (std::fabs(alpha - 1.0) < 1e-3)
        return std::exp(-0.5 * mahalanobis_sq) + eps;

    const double core = 1.0 - (1.0 - alpha) * 0.5 * mahalanobis_sq;
    if (core <= 0.0)
        return eps;
    return std::pow(core, 1.0 / (1.0 - alpha)) + eps;
}

static constexpr int mode_count_local = 7;
static constexpr double mode_diffusion_rc_local = .75;
static constexpr std::array<double, mode_count_local> mode_centers {
    0.05, 0.2, 0.35, 0.5, 0.65, 0.8, 0.95
};

static void initialize_uniform(std::array<double, mode_count_local>& p)
{
    p.fill(1.0 / static_cast<double>(p.size()));
}

static void diffuse_modes(std::array<double, mode_count_local>& p, double dt)
{
    const double mix = clamp01(dt / (dt + mode_diffusion_rc_local));
    const double uniform = 1.0 / static_cast<double>(p.size());
    for (double& x : p)
        x = (1.0 - mix) * x + mix * uniform;
}

static void update_modes_from_measurement(std::array<double, mode_count_local>& p, double measurement)
{
    static constexpr double sigma = 0.17;
    static constexpr double inv_two_sigma2 = 1.0 / (2.0 * sigma * sigma);
    static constexpr double epsilon = 1e-8;

    measurement = clamp01(measurement);
    double sum = 0.0;
    for (int i = 0; i < static_cast<int>(p.size()); i++)
    {
        const double err = measurement - mode_centers[i];
        const double likelihood = std::exp(-(err * err) * inv_two_sigma2) + epsilon;
        p[i] *= likelihood;
        sum += p[i];
    }

    if (sum <= 1e-12)
    {
        initialize_uniform(p);
        return;
    }

    for (double& x : p)
        x /= sum;
}

static double mode_expectation(const std::array<double, mode_count_local>& p)
{
    double e = 0.0;
    for (int i = 0; i < static_cast<int>(p.size()); i++)
        e += p[i] * mode_centers[i];
    return clamp01(e);
}

static double mode_peak_center(const std::array<double, mode_count_local>& p)
{
    int idx = 0;
    for (int i = 1; i < static_cast<int>(p.size()); i++)
        if (p[i] > p[idx])
            idx = i;
    return mode_centers[idx];
}
}

detail::alpha_spectrum::calibration_status& detail::alpha_spectrum::shared_calibration_status()
{
    static calibration_status status;
    return status;
}

void alpha_spectrum::set_tracker(ITracker* tracker)
{
    highrate_source = dynamic_cast<IHighrateSource*>(tracker);
    has_highrate_source = false;
    qDebug() << "alpha-spectrum: set_tracker called, highrate_source =" << (highrate_source ? "available" : "null");
    last_highrate_pose_valid = false;
    std::fill(gyro_integrated_rotation, gyro_integrated_rotation + 3, 0.0);
}

void alpha_spectrum::integrate_highrate_samples()
{
    std::fill(gyro_integrated_rotation, gyro_integrated_rotation + 3, 0.0);
    has_highrate_source = false;

    if (!highrate_source)
        return;

    std::vector<highrate_pose_sample> samples;
    if (!highrate_source->get_highrate_samples(samples))
        return;

    has_highrate_source = true;
    
    static int log_counter = 0;
    if (++log_counter <= 3 || log_counter % 250 == 0)
        qDebug() << "alpha-spectrum: integrated" << samples.size() << "high-rate samples";

    static constexpr double full_turn = 360.0;
    static constexpr double half_turn = 180.0;

    for (const auto& sample : samples)
    {
        if (!last_highrate_pose_valid)
        {
            last_highrate_pose[0] = sample.pose[Yaw];
            last_highrate_pose[1] = sample.pose[Pitch];
            last_highrate_pose[2] = sample.pose[Roll];
            last_highrate_pose_valid = true;
            continue;
        }

        for (int axis = 0; axis < 3; axis++)
        {
            const int pose_axis = Yaw + axis;
            double delta = sample.pose[pose_axis] - last_highrate_pose[axis];
            if (std::fabs(delta) > half_turn)
            {
                const double wrap = std::copysign(full_turn, delta);
                delta -= wrap;
            }

            gyro_integrated_rotation[axis] += delta;
            last_highrate_pose[axis] = sample.pose[pose_axis];
        }
    }
}

void alpha_spectrum::filter(const double* input, double* output)
{
    static constexpr double full_turn = 360.0;
    static constexpr double half_turn = 180.0;

    // Stage 0: first-frame bootstrap to avoid transient spikes.
    if (first_run) [[unlikely]]
    {
        first_run = false;
        timer.start();
        noise_rc = 0.0;
        rot_activity = 0.0;
        pos_activity = 0.0;
        std::copy(input, input + axis_count, last_input);
        std::copy(input, input + axis_count, last_output);
        std::fill(last_delta, last_delta + axis_count, 0.0);
        std::fill(last_noise, last_noise + axis_count, 0.0);
        std::fill(raw_brownian_energy, raw_brownian_energy + axis_count, 0.0);
        std::fill(filtered_brownian_energy, filtered_brownian_energy + axis_count, 0.0);
        std::copy(input, input + axis_count, predicted_next_output);
        coupling_residual = 0.0;
        last_Z = std::max(std::fabs(input[TZ]), 0.3);
        std::fill(gyro_integrated_rotation, gyro_integrated_rotation + 3, 0.0);
        last_highrate_pose_valid = false;
        initialize_uniform(rot_mode_prob);
        initialize_uniform(pos_mode_prob);
        std::copy(last_output, last_output + axis_count, output);
        return;
    }

    const double dt = timer.elapsed_seconds();
    timer.start();
    const double safe_dt = std::min(dt_max, std::max(dt_min, dt));

    // Integrate high-rate gyro samples into Predictive head prediction
    integrate_highrate_samples();

    noise_rc = std::min(noise_rc + safe_dt, noise_rc_max);
    const double delta_alpha = safe_dt / (safe_dt + delta_rc);
    const double noise_alpha = safe_dt / (safe_dt + noise_rc);
    const double activity_alpha = safe_dt / (safe_dt + activity_rc);
    const double brownian_alpha = safe_dt / (safe_dt + brownian_rc);
    const bool adaptive_mode = *s.adaptive_mode;
    const bool ema_enabled = *s.ema_enabled;
    const bool brownian_enabled = *s.brownian_enabled;
    const bool predictive_enabled = *s.predictive_enabled;
    const bool mtm_enabled = *s.mtm_enabled;
    auto& status = detail::alpha_spectrum::shared_calibration_status();
    status.active.store(true, std::memory_order_relaxed);

    double rot_mode_e_prior = 0.0;
    double pos_mode_e_prior = 0.0;
    if (mtm_enabled)
    {
        diffuse_modes(rot_mode_prob, safe_dt);
        diffuse_modes(pos_mode_prob, safe_dt);
        rot_mode_e_prior = mode_expectation(rot_mode_prob);
        pos_mode_e_prior = mode_expectation(pos_mode_prob);
    }

    if (!adaptive_mode)
    {
        rot_activity = 0.0;
        pos_activity = 0.0;
    }

    const double rot_alpha_min = *s.rot_alpha_min;
    const double rot_alpha_max_source = *s.rot_alpha_max;
    const double rot_alpha_max = std::max(rot_alpha_min, rot_alpha_max_source);
    const double rot_curve = *s.rot_curve;
    const double rot_deadzone = *s.rot_deadzone;

    const double pos_alpha_min = *s.pos_alpha_min;
    const double pos_alpha_max_source = *s.pos_alpha_max;
    const double pos_alpha_max = std::max(pos_alpha_min, pos_alpha_max_source);
    const double pos_curve = *s.pos_curve;
    const double pos_deadzone = *s.pos_deadzone;
    const double brownian_head_gain = *s.brownian_head_gain;
    const double adaptive_threshold_lift = *s.adaptive_threshold_lift;
    const double predictive_head_gain = *s.predictive_head_gain;
    const double mtm_shoulder_base = *s.mtm_shoulder_base;
    const double ngc_kappa = *s.ngc_kappa;
    const double ngc_nominal_z = *s.ngc_nominal_z;

    // NGC: explicit depth-scale commutator coupling residual.
    {
        const double curr_Z = std::max(std::fabs(input[TZ]), 0.3);
        const double delta_Z = curr_Z - last_Z;
        last_Z = curr_Z;

        const double radial_pos = std::hypot(input[TX], input[TY]);
        coupling_residual = ngc_kappa * std::fabs(delta_Z) * (radial_pos / (ngc_nominal_z * ngc_nominal_z));
        coupling_residual = std::clamp(coupling_residual, 0.0, 3.0);
    }

    // Stage 1: per-axis measurement processing.
    // - wrap handling for rotational continuity
    // - deadzone suppression
    // - innovation/noise estimation for adaptive alpha
    double rot_jitter_sum = 0.0;
    double pos_jitter_sum = 0.0;
    double rot_objective_sum = 0.0;
    double pos_objective_sum = 0.0;
    double rot_brownian_raw_sum = 0.0;
    double rot_brownian_filtered_sum = 0.0;
    double rot_predictive_error_sum = 0.0;
    double pos_brownian_raw_sum = 0.0;
    double pos_brownian_filtered_sum = 0.0;
    double pos_predictive_error_sum = 0.0;
    double rot_ema_drive_sum = 0.0;
    double rot_brownian_drive_sum = 0.0;
    double rot_adaptive_drive_sum = 0.0;
    double rot_predictive_drive_sum = 0.0;
    double rot_mtm_drive_sum = 0.0;
    double pos_ema_drive_sum = 0.0;
    double pos_brownian_drive_sum = 0.0;
    double pos_adaptive_drive_sum = 0.0;
    double pos_predictive_drive_sum = 0.0;
    double pos_mtm_drive_sum = 0.0;

    for (int i = TX; i <= Roll; i++)
    {
        const bool is_rotation = i >= Yaw;
        double input_value = input[i];
        double raw_input_value = input[i];
        double delta = input_value - last_output[i];

        double raw_delta = raw_input_value - last_input[i];
        if (is_rotation && std::fabs(raw_delta) > half_turn)
        {
            const double wrap = std::copysign(full_turn, raw_delta);
            raw_input_value -= wrap;
            raw_delta = raw_input_value - last_input[i];
        }

        if (is_rotation && std::fabs(delta) > half_turn)
        {
            const double wrap = std::copysign(full_turn, delta);
            delta -= wrap;
            input_value -= wrap;
        }

        const double filtered_prev = last_output[i];

        const double deadzone = is_rotation ? rot_deadzone : pos_deadzone;
        if (std::fabs(delta) < deadzone)
        {
            delta = 0.0;
            input_value = last_output[i];
        }

        last_delta[i] += delta_alpha * (delta - last_delta[i]);

        const double innovation = last_delta[i] * last_delta[i];
        last_noise[i] = noise_alpha * innovation + (1.0 - noise_alpha) * last_noise[i];
        const double norm_innovation =
            last_noise[i] < 1e-12 ? 0.0 : std::min(innovation / (9.0 * last_noise[i]), 1.0);

        if (is_rotation)
        {
            rot_jitter_sum += std::sqrt(std::max(last_noise[i], 0.0));
            rot_objective_sum += norm_innovation;
        }
        else
        {
            pos_jitter_sum += std::sqrt(std::max(last_noise[i], 0.0));
            pos_objective_sum += norm_innovation;
        }

        const double alpha_min = is_rotation ? rot_alpha_min : pos_alpha_min;
        const double alpha_max = is_rotation ? rot_alpha_max : pos_alpha_max;
        const double curve = is_rotation ? rot_curve : pos_curve;
        double& activity = is_rotation ? rot_activity : pos_activity;
        const double mode_e = is_rotation ? rot_mode_e_prior : pos_mode_e_prior;
        const double mtm_drive = mtm_enabled ? mode_e : 0.0;

        if (adaptive_mode)
        {
            const double raw_brownian = std::sqrt(std::max(raw_brownian_energy[i], 0.0));
            const double filtered_brownian = std::sqrt(std::max(filtered_brownian_energy[i], 0.0));
            const double brownian_drive =
                brownian_enabled && raw_brownian > 1e-12 ?
                    clamp01(1.0 - filtered_brownian / raw_brownian) : 0.0;
            const double instantaneous_drive = std::max(std::max(norm_innovation, brownian_drive), mtm_drive);
            activity += activity_alpha * (instantaneous_drive - activity);
            activity = std::clamp(activity, 0.0, 1.0);
        }

        // Stage 2: build hydra heads (EMA/Brownian) and compose through
        // a Rényi-style neck under MTM shoulder control.
        const double motion = std::pow(norm_innovation, curve);
        double alpha_ema = alpha_min + (alpha_max - alpha_min) * motion;

        if (adaptive_mode)
            alpha_ema += adaptive_boost * activity * (1.0 - alpha_ema);

        alpha_ema = std::clamp(alpha_ema, 0.0, 1.0);

        const double raw_brownian = std::sqrt(std::max(raw_brownian_energy[i], 0.0));
        const double filtered_brownian = std::sqrt(std::max(filtered_brownian_energy[i], 0.0));
        const double brownian_drive =
            brownian_enabled && raw_brownian > 1e-12 ?
                clamp01(1.0 - filtered_brownian / raw_brownian) : 0.0;

        const double tuned_brownian_drive = clamp01(brownian_drive * brownian_head_gain);
        double alpha_brownian = alpha_min + (alpha_max - alpha_min) * std::pow(tuned_brownian_drive, curve);
        if (adaptive_mode)
            alpha_brownian += adaptive_boost * activity * (1.0 - alpha_brownian);
        alpha_brownian = std::clamp(alpha_brownian, 0.0, 1.0);

        struct head_candidate final {
            bool enabled;
            double sample;
            double weight;
            bool is_ema;
            bool is_brownian;
            bool is_adaptive;
            bool is_predictive;
        };

        std::array<head_candidate, hydra_head_capacity> heads {{
            {false, input_value, 0.0, false, false, false, false},
            {false, input_value, 0.0, false, false, false, false},
            {false, input_value, 0.0, false, false, false, false},
            {false, input_value, 0.0, false, false, false, false}
        }};

        int head_count = 0;
        auto add_head = [&](bool enabled, double sample, bool is_ema, bool is_brownian, bool is_adaptive, bool is_predictive) {
            if (!enabled || head_count >= hydra_head_capacity)
                return;
            heads[head_count++] = {true, sample, 0.0, is_ema, is_brownian, is_adaptive, is_predictive};
        };

        const double adaptive_gate_input = std::max(norm_innovation, brownian_drive);
        const double adaptive_gate = remap_with_threshold(adaptive_gate_input, adaptive_threshold_lift);
        double alpha_adaptive = alpha_min + (alpha_max - alpha_min) * std::pow(adaptive_gate, curve);
        if (adaptive_mode)
            alpha_adaptive += adaptive_boost * activity * (1.0 - alpha_adaptive);
        alpha_adaptive = std::clamp(alpha_adaptive, 0.0, 1.0);

        const double ema_head_sample = filtered_prev + alpha_ema * (input_value - filtered_prev);
        const double brownian_head_sample =
            filtered_prev + alpha_brownian * (input_value - filtered_prev);
        const double adaptive_head_sample =
            filtered_prev + alpha_adaptive * (input_value - filtered_prev);
        const double predictive_head_sample =
            filtered_prev + predictive_head_gain * (predicted_next_output[i] - filtered_prev);

        const double predictive_error = std::fabs(input_value - predictive_head_sample);
        if (is_rotation)
            rot_predictive_error_sum += predictive_error;
        else
            pos_predictive_error_sum += predictive_error;

        add_head(ema_enabled, ema_head_sample, true, false, false, false);
        add_head(brownian_enabled, brownian_head_sample, false, true, false, false);
        add_head(adaptive_mode, adaptive_head_sample, false, false, true, false);
        add_head(predictive_enabled, predictive_head_sample, false, false, false, true);

        double ema_share = 0.0;
        double brownian_share = 0.0;
        double adaptive_share = 0.0;
        double predictive_share = 0.0;
        const double shoulder_gain = mtm_enabled ?
            std::clamp(mtm_shoulder_base + (1.0 - mtm_shoulder_base) * mode_e, 0.0, 1.0) :
            0.0;
        double composed_output = input_value;

        if (head_count == 0)
        {
            composed_output = input_value;
        }
        else if (!mtm_enabled || head_count == 1)
        {
            heads[0].weight = 1.0;
            composed_output = heads[0].sample;
            if (heads[0].is_ema)
                ema_share = 1.0;
            if (heads[0].is_brownian)
                brownian_share = 1.0;
            if (heads[0].is_adaptive)
                adaptive_share = 1.0;
            if (heads[0].is_predictive)
                predictive_share = 1.0;
        }
        else
        {
            // Rényi neck: true Renyi/Tsallis likelihood + NGC residual coupling.
            const double sigma2 = std::max(last_noise[i], 1e-8);
            const double alpha = 0.1 + 24.9 * mode_e; // [0.1, 25.0]

            double sum = 0.0;
            for (int h = 0; h < head_count; h++)
            {
                double residual = input_value - heads[h].sample;

                if (!is_rotation)
                {
                    if (i == TX || i == TY)
                        residual += coupling_residual * 0.7;
                    else if (i == TZ)
                        residual += coupling_residual * 1.3;
                }

                const double mahalanobis_sq =
                    (residual * residual) / sigma2 + coupling_residual * coupling_residual;
                const double likelihood = renyi_tsallis_likelihood(mahalanobis_sq, alpha);
                heads[h].weight = likelihood;
                sum += heads[h].weight;
            }

            if (sum <= 1e-15)
            {
                const double uniform_w = 1.0 / static_cast<double>(head_count);
                for (int h = 0; h < head_count; h++)
                    heads[h].weight = uniform_w;
            }
            else
            {
                for (int h = 0; h < head_count; h++)
                    heads[h].weight /= sum;
            }

            // MTM shoulder composes the neck-normalized heads and controls
            // how strongly we trust the composed estimate this frame.
            double hydra_sample = 0.0;
            for (int h = 0; h < head_count; h++)
            {
                hydra_sample += heads[h].weight * heads[h].sample;
                if (heads[h].is_ema)
                    ema_share += heads[h].weight;
                if (heads[h].is_brownian)
                    brownian_share += heads[h].weight;
                if (heads[h].is_adaptive)
                    adaptive_share += heads[h].weight;
                if (heads[h].is_predictive)
                    predictive_share += heads[h].weight;
            }

            composed_output = filtered_prev + shoulder_gain * (hydra_sample - filtered_prev);
        }

        if (!mtm_enabled && head_count > 1)
        {
            // deterministic no-MTM path: EMA -> Brownian -> Adaptive -> Predictive.
            const bool use_ema = ema_enabled;
            const bool use_brownian = !use_ema && brownian_enabled;
            const bool use_adaptive = !use_ema && !use_brownian && adaptive_mode;
            const bool use_predictive = !use_ema && !use_brownian && !use_adaptive && predictive_enabled;
            ema_share = use_ema ? 1.0 : 0.0;
            brownian_share = use_brownian ? 1.0 : 0.0;
            adaptive_share = use_adaptive ? 1.0 : 0.0;
            predictive_share = use_predictive ? 1.0 : 0.0;
            if (use_ema)
                composed_output = ema_head_sample;
            else if (use_brownian)
                composed_output = brownian_head_sample;
            else if (use_adaptive)
                composed_output = adaptive_head_sample;
            else if (use_predictive)
                composed_output = predictive_head_sample;
            else
                composed_output = input_value;
        }

        last_output[i] = composed_output;
        output[i] = last_output[i];

        double prediction_delta = last_output[i] - filtered_prev;
        if (is_rotation && std::fabs(prediction_delta) > half_turn)
        {
            const double wrap = std::copysign(full_turn, prediction_delta);
            prediction_delta -= wrap;
        }
        const double predicted_velocity = prediction_delta / safe_dt;
        
        // Predictive head: incorporate high-rate gyro integration for rotation axes
        if (is_rotation && has_highrate_source && (i == Yaw || i == Pitch || i == Roll))
        {
            // Use gyro-integrated rotation instead of velocity extrapolation
            const int gyro_idx = i - 3; // Map Yaw/Pitch/Roll to gyro_integrated_rotation[0/1/2]
            predicted_next_output[i] = last_output[i] + gyro_integrated_rotation[gyro_idx];
            // Reset accumulator after use
            gyro_integrated_rotation[gyro_idx] = 0.0;
        }
        else
        {
            predicted_next_output[i] = last_output[i] + predicted_velocity * safe_dt;
        }

        double filtered_delta = last_output[i] - filtered_prev;
        if (is_rotation && std::fabs(filtered_delta) > half_turn)
        {
            const double wrap = std::copysign(full_turn, filtered_delta);
            filtered_delta -= wrap;
        }

        raw_brownian_energy[i] += brownian_alpha * (raw_delta * raw_delta - raw_brownian_energy[i]);
        filtered_brownian_energy[i] +=
            brownian_alpha * (filtered_delta * filtered_delta - filtered_brownian_energy[i]);

        if (is_rotation)
        {
            rot_brownian_raw_sum += std::sqrt(std::max(raw_brownian_energy[i], 0.0));
            rot_brownian_filtered_sum += std::sqrt(std::max(filtered_brownian_energy[i], 0.0));
            rot_ema_drive_sum += ema_share;
            rot_brownian_drive_sum += brownian_share;
            rot_adaptive_drive_sum += adaptive_share;
            rot_predictive_drive_sum += predictive_share;
            rot_mtm_drive_sum += shoulder_gain;
        }
        else
        {
            pos_brownian_raw_sum += std::sqrt(std::max(raw_brownian_energy[i], 0.0));
            pos_brownian_filtered_sum += std::sqrt(std::max(filtered_brownian_energy[i], 0.0));
            pos_ema_drive_sum += ema_share;
            pos_brownian_drive_sum += brownian_share;
            pos_adaptive_drive_sum += adaptive_share;
            pos_predictive_drive_sum += predictive_share;
            pos_mtm_drive_sum += shoulder_gain;
        }

        last_input[i] = raw_input_value;
    }

    const double rot_brownian_raw_avg = rot_brownian_raw_sum / 3.0;
    const double rot_brownian_filtered_avg = rot_brownian_filtered_sum / 3.0;
    const double rot_predictive_error_avg = rot_predictive_error_sum / 3.0;
    const double pos_brownian_raw_avg = pos_brownian_raw_sum / 3.0;
    const double pos_brownian_filtered_avg = pos_brownian_filtered_sum / 3.0;
    const double pos_predictive_error_avg = pos_predictive_error_sum / 3.0;
    const double rot_brownian_delta_avg = rot_brownian_raw_avg - rot_brownian_filtered_avg;
    const double pos_brownian_delta_avg = pos_brownian_raw_avg - pos_brownian_filtered_avg;
    const double rot_ema_drive_avg = rot_ema_drive_sum / 3.0;
    const double rot_brownian_drive_avg = rot_brownian_drive_sum / 3.0;
    const double rot_adaptive_drive_avg = rot_adaptive_drive_sum / 3.0;
    const double rot_predictive_drive_avg = rot_predictive_drive_sum / 3.0;
    const double rot_mtm_drive_avg = rot_mtm_drive_sum / 3.0;
    const double pos_ema_drive_avg = pos_ema_drive_sum / 3.0;
    const double pos_brownian_drive_avg = pos_brownian_drive_sum / 3.0;
    const double pos_adaptive_drive_avg = pos_adaptive_drive_sum / 3.0;
    const double pos_predictive_drive_avg = pos_predictive_drive_sum / 3.0;
    const double pos_mtm_drive_avg = pos_mtm_drive_sum / 3.0;

    const double rot_brownian_damped =
        rot_brownian_raw_avg > 1e-12 ?
            std::clamp(1.0 - rot_brownian_filtered_avg / rot_brownian_raw_avg, -1.0, 1.0) : 0.0;
    const double pos_brownian_damped =
        pos_brownian_raw_avg > 1e-12 ?
            std::clamp(1.0 - pos_brownian_filtered_avg / pos_brownian_raw_avg, -1.0, 1.0) : 0.0;

    double rot_mode_e = 0.0;
    double pos_mode_e = 0.0;
    double rot_mode_peak = 0.0;
    double pos_mode_peak = 0.0;
    const double rot_objective_avg = rot_objective_sum / 3.0;
    const double pos_objective_avg = pos_objective_sum / 3.0;
    if (mtm_enabled)
    {
        const double rot_measurement = brownian_enabled ?
            clamp01(0.65 * rot_objective_avg + 0.35 * std::max(rot_brownian_damped, 0.0)) :
            clamp01(rot_objective_avg);
        const double pos_measurement = brownian_enabled ?
            clamp01(0.65 * pos_objective_avg + 0.35 * std::max(pos_brownian_damped, 0.0)) :
            clamp01(pos_objective_avg);
        update_modes_from_measurement(rot_mode_prob, rot_measurement);
        update_modes_from_measurement(pos_mode_prob, pos_measurement);
        rot_mode_e = mode_expectation(rot_mode_prob);
        pos_mode_e = mode_expectation(pos_mode_prob);
        rot_mode_peak = mode_peak_center(rot_mode_prob);
        pos_mode_peak = mode_peak_center(pos_mode_prob);
    }

    // Stage 4: publish status for the settings panel.
    // If MTM is disabled, objective falls back to innovation objective.
    status.rot_objective.store(mtm_enabled ? rot_mode_e : rot_objective_avg, std::memory_order_relaxed);
    status.pos_objective.store(mtm_enabled ? pos_mode_e : pos_objective_avg, std::memory_order_relaxed);
    status.rot_jitter.store(rot_jitter_sum / 3.0, std::memory_order_relaxed);
    status.pos_jitter.store(pos_jitter_sum / 3.0, std::memory_order_relaxed);
    status.rot_brownian_raw.store(rot_brownian_raw_avg, std::memory_order_relaxed);
    status.rot_brownian_filtered.store(rot_brownian_filtered_avg, std::memory_order_relaxed);
    status.rot_brownian_delta.store(rot_brownian_delta_avg, std::memory_order_relaxed);
    status.rot_brownian_damped.store(rot_brownian_damped, std::memory_order_relaxed);
    status.rot_predictive_error.store(rot_predictive_error_avg, std::memory_order_relaxed);
    status.pos_brownian_raw.store(pos_brownian_raw_avg, std::memory_order_relaxed);
    status.pos_brownian_filtered.store(pos_brownian_filtered_avg, std::memory_order_relaxed);
    status.pos_brownian_delta.store(pos_brownian_delta_avg, std::memory_order_relaxed);
    status.pos_brownian_damped.store(pos_brownian_damped, std::memory_order_relaxed);
    status.pos_predictive_error.store(pos_predictive_error_avg, std::memory_order_relaxed);
    status.rot_ema_drive.store(rot_ema_drive_avg, std::memory_order_relaxed);
    status.rot_brownian_drive.store(rot_brownian_drive_avg, std::memory_order_relaxed);
    status.rot_adaptive_drive.store(rot_adaptive_drive_avg, std::memory_order_relaxed);
    status.rot_predictive_drive.store(rot_predictive_drive_avg, std::memory_order_relaxed);
    status.rot_mtm_drive.store(rot_mtm_drive_avg, std::memory_order_relaxed);
    status.pos_ema_drive.store(pos_ema_drive_avg, std::memory_order_relaxed);
    status.pos_brownian_drive.store(pos_brownian_drive_avg, std::memory_order_relaxed);
    status.pos_adaptive_drive.store(pos_adaptive_drive_avg, std::memory_order_relaxed);
    status.pos_predictive_drive.store(pos_predictive_drive_avg, std::memory_order_relaxed);
    status.pos_mtm_drive.store(pos_mtm_drive_avg, std::memory_order_relaxed);
    status.rot_mode_expectation.store(rot_mode_e, std::memory_order_relaxed);
    status.pos_mode_expectation.store(pos_mode_e, std::memory_order_relaxed);
    status.rot_mode_peak.store(rot_mode_peak, std::memory_order_relaxed);
    status.pos_mode_peak.store(pos_mode_peak, std::memory_order_relaxed);
    status.ngc_coupling_residual.store(coupling_residual, std::memory_order_relaxed);
    status.rot_alpha_min.store(*s.rot_alpha_min, std::memory_order_relaxed);
    status.rot_alpha_max.store(*s.rot_alpha_max, std::memory_order_relaxed);
    status.rot_curve.store(*s.rot_curve, std::memory_order_relaxed);
    status.rot_deadzone.store(*s.rot_deadzone, std::memory_order_relaxed);
    status.pos_alpha_min.store(*s.pos_alpha_min, std::memory_order_relaxed);
    status.pos_alpha_max.store(*s.pos_alpha_max, std::memory_order_relaxed);
    status.pos_curve.store(*s.pos_curve, std::memory_order_relaxed);
    status.pos_deadzone.store(*s.pos_deadzone, std::memory_order_relaxed);
}

OPENTRACK_DECLARE_FILTER(alpha_spectrum, dialog_alpha_spectrum, alpha_spectrumDll)
