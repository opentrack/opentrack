#include "urb.hpp"
#include "singleton.hpp"
#include "frame-queue.hpp"
#include "internal.hpp"

#include <algorithm>
#include <cstring>

/* Values for bmHeaderInfo (Video and Still Image Payload Headers, 2.4.3.3) */
enum {
    UVC_STREAM_EOH = (1 << 7),
    UVC_STREAM_ERR = (1 << 6),
    UVC_STREAM_STI = (1 << 5),
    UVC_STREAM_RES = (1 << 4),
    UVC_STREAM_SCR = (1 << 3),
    UVC_STREAM_PTS = (1 << 2),
    UVC_STREAM_EOF = (1 << 1),
    UVC_STREAM_FID = (1 << 0),
};

void USB_CALLBACK URBDesc::transfer_completed_callback(struct libusb_transfer *xfr)
{
    URBDesc *urb = reinterpret_cast<URBDesc*>(xfr->user_data);
    enum libusb_transfer_status status = xfr->status;

    switch (status)
    {
    case LIBUSB_TRANSFER_TIMED_OUT:
        urb->frame_add(URBDesc::DISCARD_PACKET, nullptr, 0);
        break;
    case LIBUSB_TRANSFER_COMPLETED:
        urb->pkt_scan(xfr->buffer, xfr->actual_length);
        if (int error = libusb_submit_transfer(xfr); error < 0) {
            debug("libusb_submit_transfer(%d), exiting\n", error);
            urb->close_transfers();
        }
        break;
    default:
        debug("transfer 0x%p failed(%d)\n", (void*)xfr, status);
        urb->close_transfers();
        break;
    case LIBUSB_TRANSFER_CANCELLED:
        urb->transfer_cancelled();
        debug("transfer 0x%p cancelled\n", (void*)xfr);
        break;
    }

    //debug("length:%u, actual_length:%u\n", xfr->length, xfr->actual_length);
}

URBDesc::~URBDesc()
{
    close_transfers();
    free_transfers();
}

bool URBDesc::start_transfers(libusb_device_handle* handle, uint32_t curr_frame_size)
{
    // Initialize the frame queue
    frame_size = curr_frame_size;
    frame_queue = std::make_shared<FrameQueue>(frame_size);

    // Initialize the current frame pointer to the start of the buffer; it will be updated as frames are completed and pushed onto the frame queue
    cur_frame_start = frame_queue->ptr();
    cur_frame_data_len = 0;

    // Find the bulk transfer endpoint
    uint8_t bulk_endpoint = find_ep(libusb_get_device(handle));
    libusb_clear_halt(handle, bulk_endpoint);

    // Allocate the transfer buffer
    memset(transfer_buffer, 0, sizeof(transfer_buffer));

    bool res = true;
    for (int index = 0; index < NUM_TRANSFERS; ++index)
    {
        // Create & submit the transfer
        xfers[index] = libusb_alloc_transfer(0);

        if (!xfers[index])
        {
            debug("libusb_alloc_transfer failed\n");
            res = false;
            break;
        }

        libusb_fill_bulk_transfer(xfers[index], handle, bulk_endpoint, transfer_buffer + index * TRANSFER_SIZE, TRANSFER_SIZE, transfer_completed_callback, reinterpret_cast<void*>(this), 0);

        if (int status = libusb_submit_transfer(xfers[index]); status != LIBUSB_SUCCESS)
        {
            debug("libusb_submit_transfer status %d\n", status);
            libusb_free_transfer(xfers[index]);
            xfers[index] = nullptr;
            res = false;
            break;
        }

        num_active_transfers++;
    }

    last_pts = 0;
    last_fid = 0;

    USBMgr::instance().camera_started();

    return res;
}

void URBDesc::close_transfers()
{
    std::unique_lock<std::mutex> lock(num_active_transfers_mutex);

    if (teardown)
        return;
    teardown = true;

    // Cancel any pending transfers
    for (int i = 0; i < NUM_TRANSFERS; ++i)
    {
        if (!xfers[i])
            continue;
        enum libusb_error status = (enum libusb_error)libusb_cancel_transfer(xfers[i]);
        if (status)
            debug("libusb_cancel_transfer error(%d) %s\n", status, libusb_strerror(status));
    }
    num_active_transfers_condition.notify_one();
}

void URBDesc::free_transfers()
{
    std::unique_lock<std::mutex> lock(num_active_transfers_mutex);

    // Wait for cancelation to finish
    num_active_transfers_condition.wait(lock, [this] { return num_active_transfers == 0; });

    // Free completed transfers
    for (libusb_transfer*& xfer : xfers)
    {
        if (!xfer)
            continue;
        libusb_free_transfer(xfer);
        xfer = nullptr;
    }

    USBMgr::instance().camera_stopped();
}

int URBDesc::transfer_cancelled()
{
    std::lock_guard<std::mutex> lock(num_active_transfers_mutex);
    int refcnt = --num_active_transfers;
    num_active_transfers_condition.notify_one();

    return refcnt;
}

void URBDesc::frame_add(enum gspca_packet_type packet_type, const uint8_t* data, int len)
{
    if (packet_type == FIRST_PACKET)
        cur_frame_data_len = 0;
    else
        switch(last_packet_type)  // ignore warning.
        {
        case DISCARD_PACKET:
            if (packet_type == LAST_PACKET) {
                last_packet_type = packet_type;
                cur_frame_data_len = 0;
            }
            return;
        case LAST_PACKET:
            return;
        default:
            break;
        }

    /* append the packet to the frame buffer */
    if (len > 0)
    {
        if(cur_frame_data_len + len > frame_size)
        {
            packet_type = DISCARD_PACKET;
            cur_frame_data_len = 0;
        } else {
            memcpy(cur_frame_start+cur_frame_data_len, data, (size_t) len);
            cur_frame_data_len += len;
        }
    }

    last_packet_type = packet_type;

    if (packet_type == LAST_PACKET) {
        cur_frame_data_len = 0;
        cur_frame_start = frame_queue->Enqueue();
        //debug("frame completed %d\n", frame_complete_ind);
    }
}

void URBDesc::pkt_scan(uint8_t* data, int len)
{
    uint32_t this_pts;
    uint16_t this_fid;
    int remaining_len = len;
    constexpr int payload_len = 2048; // bulk type

    do {
        len = std::min(remaining_len, payload_len);

        /* Payloads are prefixed with a UVC-style header.  We
           consider a frame to start when the FID toggles, or the PTS
           changes.  A frame ends when EOF is set, and we've received
           the correct number of bytes. */

        /* Verify UVC header.  Header length is always 12 */
        if (data[0] != 12 || len < 12) {
            debug("bad header\n");
            goto discard;
        }

        /* Check errors */
        if (data[1] & UVC_STREAM_ERR) {
            debug("payload error\n");
            goto discard;
        }

        /* Extract PTS and FID */
        if (!(data[1] & UVC_STREAM_PTS)) {
            debug("PTS not present\n");
            goto discard;
        }

        this_pts = (data[5] << 24) | (data[4] << 16) | (data[3] << 8) | data[2];
        this_fid = (uint16_t) ((data[1] & UVC_STREAM_FID) ? 1 : 0);

        /* If PTS or FID has changed, start a new frame. */
        if (this_pts != last_pts || this_fid != last_fid) {
            if (last_packet_type == INTER_PACKET)
            {
                /* The last frame was incomplete, so don't keep it or we will glitch */
                frame_add(DISCARD_PACKET, nullptr, 0);
            }
            last_pts = this_pts;
            last_fid = this_fid;
            frame_add(FIRST_PACKET, data + 12, len - 12);
        } /* If this packet is marked as EOF, end the frame */
        else if (data[1] & UVC_STREAM_EOF)
        {
            last_pts = 0;
            if (cur_frame_data_len + len - 12 != frame_size)
                goto discard;
            frame_add(LAST_PACKET, data + 12, len - 12);
        } else {
            /* Add the data from this payload */
            frame_add(INTER_PACKET, data + 12, len - 12);
        }

        /* Done this payload */
        goto scan_next;

discard:
        /* Discard data until a new frame starts. */
        frame_add(DISCARD_PACKET, nullptr, 0);
scan_next:
        remaining_len -= len;
        data += len;
    } while (remaining_len > 0);
}

/*
 * look for an input transfer endpoint in an alternate setting
 * libusb_endpoint_descriptor
 */
uint8_t URBDesc::find_ep(struct libusb_device *device)
{
    const struct libusb_interface_descriptor *altsetting = nullptr;
    const struct libusb_endpoint_descriptor *ep;
    struct libusb_config_descriptor *config = nullptr;
    int i;
    uint8_t ep_addr = 0;

    libusb_get_active_config_descriptor(device, &config);

    if (!config) return 0;

    for (i = 0; i < config->bNumInterfaces; i++) {
        altsetting = config->interface[i].altsetting;
        if (altsetting[0].bInterfaceNumber == 0) {
            break;
        }
    }

    if (altsetting)
        for (i = 0; i < altsetting->bNumEndpoints; i++) {
            ep = &altsetting->endpoint[i];
            if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK
                && ep->wMaxPacketSize != 0)
            {
                ep_addr = ep->bEndpointAddress;
                break;
            }
        }

    libusb_free_config_descriptor(config);

    return ep_addr;
}


