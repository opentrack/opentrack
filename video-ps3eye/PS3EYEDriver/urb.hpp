#pragma once

#include <memory>
#include <cstdint>
#include <mutex>
#include <condition_variable>

struct FrameQueue;
struct libusb_device;
struct libusb_transfer;
struct libusb_device_handle;

#ifdef _WIN32
#   define USB_CALLBACK __stdcall
#else
#   define USB_CALLBACK
#endif

struct URBDesc
{
    URBDesc() = default;
    ~URBDesc();

    bool start_transfers(libusb_device_handle *handle, uint32_t curr_frame_size);
    void free_transfers();
    void close_transfers();

    FrameQueue& queue() { return *frame_queue; }

private:
    enum {
        TRANSFER_SIZE = 65536,
        NUM_TRANSFERS  = 5,
    };

    /* packet types when moving from iso buf to frame buf */
    enum gspca_packet_type {
        DISCARD_PACKET,
        FIRST_PACKET,
        INTER_PACKET,
        LAST_PACKET,
    };

    std::shared_ptr<FrameQueue>             frame_queue;
    std::mutex                              num_active_transfers_mutex;
    std::condition_variable                 num_active_transfers_condition;

    gspca_packet_type                       last_packet_type = DISCARD_PACKET;
    libusb_transfer*                        xfers[NUM_TRANSFERS] {};
    uint8_t*                                cur_frame_start = nullptr;
    uint32_t                                cur_frame_data_len = 0;
    uint32_t                                frame_size = 0;
    uint32_t                                last_pts = 0;
    uint16_t                                last_fid = 0;
    uint8_t                                 transfer_buffer[TRANSFER_SIZE * NUM_TRANSFERS];
    uint8_t                                 num_active_transfers = 0;
    bool                                    teardown = false;

    int transfer_cancelled();
    void frame_add(enum gspca_packet_type packet_type, const uint8_t *data, int len);
    void pkt_scan(uint8_t *data, int len);
    static uint8_t find_ep(struct libusb_device *device);
    static void USB_CALLBACK transfer_completed_callback(struct libusb_transfer *xfr);
};
