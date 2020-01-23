#include "thread.hpp"
#include "compat/sleep.hpp"

tobii_thread::tobii_thread()
{
    head_pose = new tobii_head_pose_t();
    connect(this, &tobii_thread::tobii_ready_signal, this, &tobii_thread::tobii_ready_signal_impl, Qt::QueuedConnection);
    connect(this, &tobii_thread::tobii_error_signal, this, &tobii_thread::tobii_error_signal_impl, Qt::QueuedConnection);
}

tobii_thread::~tobii_thread()
{
    if (device) tobii_device_destroy(device);
    if (api) tobii_api_destroy(api);
    exit_thread = true;
    terminate();
    wait();
}

void tobii_thread::run()
{
    /* See https://developer.tobii.com/consumer-eye-trackers/stream-engine/ */
    if (tobii_api_create(&api, nullptr, nullptr) != TOBII_ERROR_NO_ERROR)
    {
        emit tobii_error_signal("Failed to initialize the Tobii Stream Engine API.");
    }

    std::vector<std::string> devices;
    if (tobii_enumerate_local_device_urls(api,
        [](char const* url, void* user_data)
        {
            auto list = (std::vector<std::string>*) user_data;
            list->push_back(url);
        }, &devices) != TOBII_ERROR_NO_ERROR)
    {
        emit tobii_error_signal("Failed to enumerate devices.");
    }

    if (devices.size() == 0)
    {
        tobii_api_destroy(api);
        emit tobii_error_signal("No stream engine compatible device(s) found.");
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
        emit tobii_error_signal("Failed to connect to device.");
    }

    emit tobii_ready_signal();

    tobii_error = TOBII_ERROR_NO_ERROR;
    while (!exit_thread)
    {
        tobii_error = tobii_wait_for_callbacks(1, &device);
        if (tobii_error == TOBII_ERROR_TIMED_OUT) continue;
        else if (tobii_error != TOBII_ERROR_NO_ERROR)
        {
            emit tobii_error_signal("tobii_wait_for_callbacks failed.");
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
                emit tobii_error_signal("Connection was lost and reconnection failed.");
            }
            continue;
        }
        else if (tobii_error != TOBII_ERROR_NO_ERROR)
        {
            emit tobii_error_signal("tobii_device_process_callbacks failed.");
        }
    }
}

void tobii_thread::tobii_error_signal_impl(QString error_message)
{
    //TODO: log? terminate?
}

void tobii_thread::tobii_ready_signal_impl()
{
    if (tobii_head_pose_subscribe(device, [](tobii_head_pose_t const* head_pose, void* user_data) {

        if ((*head_pose).position_validity != TOBII_VALIDITY_VALID
            || (*head_pose).rotation_validity_xyz[0] != TOBII_VALIDITY_VALID
            || (*head_pose).rotation_validity_xyz[1] != TOBII_VALIDITY_VALID
            || (*head_pose).rotation_validity_xyz[2] != TOBII_VALIDITY_VALID) return;

        tobii_head_pose_t* tobii_head_pose_storage = (tobii_head_pose_t*)user_data;
        *tobii_head_pose_storage = *head_pose;

        }, head_pose) != TOBII_ERROR_NO_ERROR)
    {
        emit tobii_error_signal("Failed to subscribe to head pose stream.");
    }
}
