#pragma once
#include "ps3eye.hpp"

#include <mutex>
#include <condition_variable>

struct FrameQueue final
{
    FrameQueue(uint32_t frame_size);

    inline uint8_t* ptr() { return frame_buffer.get(); }
    uint8_t* Enqueue();

    bool Dequeue(uint8_t* dest, int width, int height, ps3eye_camera::format fmt, bool flip_v);
    static void DebayerGray(int frame_width, int frame_height, const uint8_t* inBayer, uint8_t* outBuffer);

    template<int nchannels>
    static void set_alpha(uint8_t* destGreen);

    template<int nchannels>
    void debayer_RGB(int frame_width, int frame_height, const uint8_t* inBayer, uint8_t* outBuffer, bool inBGR, bool flip_v);

private:
    std::unique_ptr<uint8_t[]>              frame_buffer;
    std::mutex                              mutex;
    std::condition_variable                 queue_cvar;
    uint32_t                                frame_size = 0;
    uint32_t                                head = 0;
    uint32_t                                tail = 0;
    uint32_t                                available = 0;

    static constexpr unsigned               num_frames = 4;
};
