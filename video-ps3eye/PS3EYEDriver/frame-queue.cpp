#include "frame-queue.hpp"
#include "log.hpp"
#include "compat/macros1.h"

using namespace std::chrono_literals;

FrameQueue::FrameQueue(uint32_t frame_size) :
    frame_buffer(std::make_unique<uint8_t[]>(frame_size * num_frames)),
    frame_size(frame_size)
{
}

uint8_t* FrameQueue::Enqueue()
{
    std::lock_guard<std::mutex> lock(mutex);

    uint8_t* new_frame;

    // Unlike traditional producer/consumer, we don't block the producer if the buffer is full (ie. the consumer is not reading data fast enough).
    // Instead, if the buffer is full, we simply return the current frame pointer, causing the producer to overwrite the previous frame.
    // This allows performance to degrade gracefully: if the consumer is not fast enough (< Camera FPS), it will miss frames, but if it is fast enough (>= Camera FPS), it will see everything.
    //
    // Note that because the the producer is writing directly to the ring buffer, we can only ever be a maximum of num_frames-1 ahead of the consumer,
    // otherwise the producer could overwrite the frame the consumer is currently reading (in case of a slow consumer)
    if (available >= num_frames - 1)
        return ptr() + head * frame_size;

    // Note: we don't need to copy any data to the buffer since the USB packets are directly written to the frame buffer.
    // We just need to update head and available count to signal to the consumer that a new frame is available
    head = (head + 1) % num_frames;
    available++;

    // Determine the next frame pointer that the producer should write to
    new_frame = ptr() + head * frame_size;

    // Signal consumer that data became available
    queue_cvar.notify_one();

    return new_frame;
}

bool FrameQueue::Dequeue(uint8_t* dest, int width, int height, ps3eye_camera::format fmt, bool flip_v)
{
    std::unique_lock<std::mutex> lock(mutex);

    if (!queue_cvar.wait_for(lock, 3000ms, [this] { return available != 0; }))
    {
        debug2("frame timeout\n");
        return false;
    }

    // Copy from internal buffer
    uint8_t* src = ptr() + frame_size * tail;

    using f = typename ps3eye_camera::format;

    switch (fmt)
    {
    case f::format_Bayer:
        memcpy(dest, src, frame_size);
        break;
    case f::format_BGR:
    case f::format_RGB:
        debayer_RGB<3>(width, height, src, dest, fmt == f::format_BGR, flip_v);
        break;
    case f::format_BGRA:
    case f::format_RGBA:
        debayer_RGB<4>(width, height, src, dest, fmt == ps3eye_camera::format_BGRA, flip_v);
        break;
    case f::format_Gray:
        DebayerGray(width, height, src, dest);
        break;
    default:
        unreachable();
    }

    // Update tail and available count
    tail = (tail + 1) % num_frames;
    available--;

    return true;
}

void FrameQueue::DebayerGray(int frame_width, int frame_height, const uint8_t* inBayer, uint8_t* outBuffer)
{
    // PSMove output is in the following Bayer format (GRBG):
    //
    // G R G R G R
    // B G B G B G
    // G R G R G R
    // B G B G B G
    //
    // This is the normal Bayer pattern shifted left one place.

    int                             source_stride   = frame_width;
    const uint8_t*  source_row              = inBayer;                                              // Start at first bayer pixel
    int                             dest_stride             = frame_width;
    uint8_t*                dest_row                = outBuffer + dest_stride + 1;  // We start outputting at the second pixel of the second row's G component
    uint32_t R,G,B;

    // Fill rows 1 to height-2 of the destination buffer. First and last row are filled separately (they are copied from the second row and second-to-last rows respectively)
    for (int y = 0; y < frame_height-2; source_row += source_stride, dest_row += dest_stride, ++y)
    {
        const uint8_t* source           = source_row;
        const uint8_t* source_end       = source + (source_stride-2);                                                           // -2 to deal with the fact that we're starting at the second pixel of the row and should end at the second-to-last pixel of the row (first and last are filled separately)
        uint8_t* dest                           = dest_row;

        // Row starting with Green
        if (y % 2 == 0)
        {
            // Fill first pixel (green)
            B = (uint32_t) ((source[source_stride] + source[source_stride + 2] + 1) >> 1);
            G = source[source_stride + 1];
            R = (uint32_t) ((source[1] + source[source_stride * 2 + 1] + 1) >> 1);
            *dest = (uint8_t)((R*77 + G*151 + B*28)>>8);

            source++;
            dest++;

            // Fill remaining pixel
            for (; source <= source_end - 2; source += 2, dest += 2)
            {
                // Blue pixel
                B = source[source_stride + 1];
                G = (uint32_t) ((source[1] + source[source_stride] + source[source_stride + 2] + source[source_stride * 2 + 1] + 2) >> 2);
                R = (uint32_t) ((source[0] + source[2] + source[source_stride * 2] + source[source_stride * 2 + 2] + 2) >> 2);
                dest[0] = (uint8_t)((R*77 + G*151 + B*28)>>8);

                //  Green pixel
                B = (uint32_t) ((source[source_stride + 1] + source[source_stride + 3] + 1) >> 1);
                G = source[source_stride + 2];
                R = (uint32_t) ((source[2] + source[source_stride * 2 + 2] + 1) >> 1);
                dest[1] = (uint8_t)((R*77 + G*151 + B*28)>>8);

            }
        }
        else
        {
            for (; source <= source_end - 2; source += 2, dest += 2)
            {
                // Red pixel
                B = (uint32_t) ((source[0] + source[2] + source[source_stride * 2] + source[source_stride * 2 + 2] + 2) >> 2);;
                G = (uint32_t) ((source[1] + source[source_stride] + source[source_stride + 2] + source[source_stride * 2 + 1] + 2) >> 2);;
                R = source[source_stride + 1];
                dest[0] = (uint8_t)((R*77 + G*151 + B*28)>>8);

                // Green pixel
                B = (uint32_t) ((source[2] + source[source_stride * 2 + 2] + 1) >> 1);
                G = source[source_stride + 2];
                R = (uint32_t) ((source[source_stride + 1] + source[source_stride + 3] + 1) >> 1);
                dest[1] = (uint8_t)((R*77 + G*151 + B*28)>>8);
            }
        }

        if (source < source_end)
        {
            B = source[source_stride + 1];
            G = (uint32_t) ((source[1] + source[source_stride] + source[source_stride + 2] + source[source_stride * 2 + 1] + 2) >> 2);
            R = (uint32_t) ((source[0] + source[2] + source[source_stride * 2] + source[source_stride * 2 + 2] + 2) >> 2);;
            dest[0] = (uint8_t)((R*77 + G*151 + B*28)>>8);

            source++;
            dest++;
        }

        // Fill first pixel of row (copy second pixel)
        uint8_t* first_pixel    = dest_row-1;
        first_pixel[0]                  = dest_row[0];

        // Fill last pixel of row (copy second-to-last pixel). Note: dest row starts at the *second* pixel of the row, so dest_row + (width-2) * num_output_channels puts us at the last pixel of the row
        uint8_t* last_pixel                             = dest_row + (frame_width - 2);
        uint8_t* second_to_last_pixel   = last_pixel - 1;
        last_pixel[0]                                   = second_to_last_pixel[0];
    }

    // Fill first & last row
    for (int i = 0; i < dest_stride; i++)
    {
        outBuffer[i]                                                                    = outBuffer[i + dest_stride];
        outBuffer[i + (frame_height - 1)*dest_stride]   = outBuffer[i + (frame_height - 2)*dest_stride];
    }
}

template<int nchannels>
void FrameQueue::debayer_RGB(int frame_width, int frame_height, const uint8_t* inBayer, uint8_t* outBuffer, bool inBGR, bool flip_v)
{
    // PSMove output is in the following Bayer format (GRBG):
    //
    // G R G R G R
    // B G B G B G
    // G R G R G R
    // B G B G B G
    //
    // This is the normal Bayer pattern shifted left one place.

    int source_stride = frame_width;
    int dest_stride = frame_width * nchannels;
    // Start at first bayer pixel
    const uint8_t*  source_row  = inBayer;
    // We start outputting at the second pixel of the second row's G component
    uint8_t* dest_row  = outBuffer + dest_stride + nchannels + 1;
    int swap_br = inBGR ? 1 : -1;

    // Fill rows 1 to height-2 of the destination buffer. First and last row are filled separately (they are copied from the second row and second-to-last rows respectively)
    for (int y = 0; y < frame_height-2; source_row += source_stride, dest_row += dest_stride, ++y)
    {
        const uint8_t* source       = source_row;
        // -2 to deal with the fact that we're starting at the second pixel of the row and should end at the second-to-last pixel of the row (first and last are filled separately)
        const uint8_t* source_end   = source + (source_stride-2);
        uint8_t*       dest         = dest_row;

        // Row starting with Green
        if (y % 2 == (int)flip_v)
        {
            // Fill first pixel (green)
            dest[-1*swap_br]        = (uint8_t) ((source[source_stride] + source[source_stride + 2] + 1) >> 1);
            dest[0]                         = source[source_stride + 1];
            dest[1*swap_br]         = (uint8_t) ((source[1] + source[source_stride * 2 + 1] + 1) >> 1);
            set_alpha<nchannels>(dest);

            source++;
            dest += nchannels;

            // Fill remaining pixel
            for (; source <= source_end - 2; source += 2, dest += nchannels * 2)
            {
                // Blue pixel
                uint8_t* cur_pixel      = dest;
                cur_pixel[-1*swap_br]   = source[source_stride + 1];
                cur_pixel[0]                    = (uint8_t) ((source[1] +
                                                              source[source_stride] +
                                                              source[source_stride + 2] +
                                                              source[source_stride * 2 + 1] +
                                                              2) >> 2);
                cur_pixel[1*swap_br]    = (uint8_t) ((source[0] +
                                                      source[2] +
                                                      source[source_stride * 2] +
                                                      source[source_stride * 2 + 2] +
                                                      2) >> 2);
                set_alpha<nchannels>(cur_pixel);

                //  Green pixel
                uint8_t* next_pixel             = cur_pixel+nchannels;
                next_pixel[-1*swap_br]  = (uint8_t) ((source[source_stride + 1] + source[source_stride + 3] + 1) >> 1);
                next_pixel[0]                   = source[source_stride + 2];
                next_pixel[1*swap_br]   = (uint8_t) ((source[2] + source[source_stride * 2 + 2] + 1) >> 1);
                set_alpha<nchannels>(next_pixel);
            }
        }
        else
        {
            for (; source <= source_end - 2; source += 2, dest += nchannels * 2)
            {
                // Red pixel
                uint8_t* cur_pixel      = dest;
                cur_pixel[-1*swap_br]   = (uint8_t) ((source[0] +
                                                      source[2] +
                                                      source[source_stride * 2] +
                                                      source[source_stride * 2 + 2] +
                                                      2) >> 2);;
                cur_pixel[0]                    = (uint8_t) ((source[1] +
                                                              source[source_stride] +
                                                              source[source_stride + 2] +
                                                              source[source_stride * 2 + 1] +
                                                              2) >> 2);;
                cur_pixel[1*swap_br]    = source[source_stride + 1];
                set_alpha<nchannels>(cur_pixel);

                // Green pixel
                uint8_t* next_pixel             = cur_pixel+nchannels;
                next_pixel[-1*swap_br]  = (uint8_t) ((source[2] + source[source_stride * 2 + 2] + 1) >> 1);
                next_pixel[0]                   = source[source_stride + 2];
                next_pixel[1*swap_br]   = (uint8_t) ((source[source_stride + 1] + source[source_stride + 3] + 1) >> 1);
                set_alpha<nchannels>(next_pixel);
            }
        }

        if (source < source_end)
        {
            dest[-1*swap_br]        = source[source_stride + 1];
            dest[0]                         = (uint8_t) ((source[1] +
                                                          source[source_stride] +
                                                          source[source_stride + 2] +
                                                          source[source_stride * 2 + 1] +
                                                          2) >> 2);
            dest[1*swap_br]         = (uint8_t) ((source[0] +
                                                  source[2] +
                                                  source[source_stride * 2] +
                                                  source[source_stride * 2 + 2] +
                                                  2) >> 2);
            set_alpha<nchannels>(dest);

            source++;
            dest += nchannels;
        }

        // Fill first pixel of row (copy second pixel)
        uint8_t* first_pixel            = dest_row-nchannels;
        first_pixel[-1*swap_br]         = dest_row[-1*swap_br];
        first_pixel[0]                          = dest_row[0];
        first_pixel[1*swap_br]          = dest_row[1*swap_br];
        set_alpha<nchannels>(first_pixel);

        // Fill last pixel of row (copy second-to-last pixel). Note: dest row starts at the *second* pixel of the row, so dest_row + (width-2) * nchannels puts us at the last pixel of the row
        uint8_t* last_pixel                             = dest_row + (frame_width - 2)*nchannels;
        uint8_t* second_to_last_pixel   = last_pixel - nchannels;

        last_pixel[-1*swap_br]                  = second_to_last_pixel[-1*swap_br];
        last_pixel[0]                                   = second_to_last_pixel[0];
        last_pixel[1*swap_br]                   = second_to_last_pixel[1*swap_br];
        set_alpha<nchannels>(last_pixel);
    }

    // Fill first & last row
    for (int i = 0; i < dest_stride; i++)
    {
        outBuffer[i]                                                                    = outBuffer[i + dest_stride];
        outBuffer[i + (frame_height - 1)*dest_stride]   = outBuffer[i + (frame_height - 2)*dest_stride];
    }
}

template<> void FrameQueue::set_alpha<3>(uint8_t*) {}
template<> void FrameQueue::set_alpha<4>(uint8_t* destGreen) { destGreen[2] = 255; }

template void FrameQueue::debayer_RGB<3>(int, int, const uint8_t*, uint8_t*, bool, bool);
template void FrameQueue::debayer_RGB<4>(int, int, const uint8_t*, uint8_t*, bool, bool);
