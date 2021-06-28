#include "thread.hpp"
#include "compat/sleep.hpp"

tobii_thread::~tobii_thread()
{
    exit_thread = true;
    wait();

    if (device) tobii_device_destroy(device);
    if (api) tobii_api_destroy(api);

    quit();
}

void tobii_thread::run()
{
    /* See https://developer.tobii.com/consumer-eye-trackers/stream-engine/ */
    if (tobii_api_create(&api, nullptr, nullptr) != TOBII_ERROR_NO_ERROR)
    {
        error_last = "Failed to initialize the Tobii Stream Engine API.";
        exit_thread = true;
    }

    std::vector<std::string> devices;
    if (tobii_enumerate_local_device_urls(api,
        [](char const* url, void* user_data)
        {
            auto list = (std::vector<std::string>*) user_data;
            list->push_back(url);
        }, &devices) != TOBII_ERROR_NO_ERROR)
    {
        error_last = "Failed to enumerate devices.";
        exit_thread = true;
    }

    if (devices.size() == 0)
    {
        tobii_api_destroy(api);
        error_last = "No stream engine compatible device(s) found.";
        exit_thread = true;
    }
    std::string selected_device = devices[0];

    unsigned int retry = 0;
    tobii_error_t tobii_error = TOBII_ERROR_NO_ERROR;
    do
    {
        tobii_error = tobii_device_create(api, selected_device.c_str(), TOBII_FIELD_OF_USE_INTERACTIVE, &device);
        if ((tobii_error != TOBII_ERROR_CONNECTION_FAILED) && (tobii_error != TOBII_ERROR_FIRMWARE_UPGRADE_IN_PROGRESS)) break;
        portable::sleep(interval);
        ++retry;
    } while (retry < retries);
    if (tobii_error != TOBII_ERROR_NO_ERROR) {
        tobii_api_destroy(api);
        error_last = "Failed to connect to device.";
        exit_thread = true;
    }

    if (tobii_head_pose_subscribe(device, [](tobii_head_pose_t const* head_pose, void* user_data) {

        if ((*head_pose).position_validity != TOBII_VALIDITY_VALID
            || (*head_pose).rotation_validity_xyz[0] != TOBII_VALIDITY_VALID
            || (*head_pose).rotation_validity_xyz[1] != TOBII_VALIDITY_VALID
            || (*head_pose).rotation_validity_xyz[2] != TOBII_VALIDITY_VALID) return;

        tobii_head_pose_t* tobii_head_pose_storage = (tobii_head_pose_t*)user_data;
        *tobii_head_pose_storage = *head_pose;

        }, head_pose) != TOBII_ERROR_NO_ERROR)
    {
        error_last = "Failed to subscribe to head pose stream.";
        exit_thread = true;
    }

    tobii_error = TOBII_ERROR_NO_ERROR;
    while (!exit_thread)
    {
        tobii_error = tobii_wait_for_callbacks(1, &device);
        if (tobii_error == TOBII_ERROR_TIMED_OUT) continue;
        else if (tobii_error != TOBII_ERROR_NO_ERROR)
        {
            error_last = "tobii_wait_for_callbacks failed.";
        }
        
        tobii_error = tobii_device_process_callbacks(device);
        if (tobii_error == TOBII_ERROR_CONNECTION_FAILED)
        {
            unsigned int retry = 0;
            auto tobii_error = TOBII_ERROR_NO_ERROR;
            do
            {
                tobii_error = tobii_device_reconnect(device);
                if ((tobii_error != TOBII_ERROR_CONNECTION_FAILED) && (tobii_error != TOBII_ERROR_FIRMWARE_UPGRADE_IN_PROGRESS)) break;
                portable::sleep(interval);
                ++retry;
            } while (retry < retries);
            if (tobii_error != TOBII_ERROR_NO_ERROR)
            {
                error_last = "Connection was lost and reconnection failed.";
                exit_thread = true;
            }
            continue;
        }
        else if (tobii_error != TOBII_ERROR_NO_ERROR)
        {
            error_last = "tobii_device_process_callbacks failed.";
        }
    }
}
