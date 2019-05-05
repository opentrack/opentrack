#pragma once

#include <vector>
#include <memory>
#include <cinttypes>

struct libusb_device;
struct libusb_device_handle;

struct ps3eye_camera final
{
    enum class format
    {
        format_Bayer,					// Output in Bayer. Destination buffer must be width * height bytes
        format_BGR,					// Output in BGR. Destination buffer must be width * height * 3 bytes
        format_RGB,					// Output in RGB. Destination buffer must be width * height * 3 bytes
        format_BGRA,					// Output in BGRA. Destination buffer must be width * height * 4 bytes
        format_RGBA,					// Output in RGBA. Destination buffer must be width * height * 4 bytes
        format_Gray					// Output in Grayscale. Destination buffer must be width * height bytes
    };

    static constexpr format format_Bayer = format::format_Bayer;
    static constexpr format format_BGR = format::format_BGR;
    static constexpr format format_RGB = format::format_RGB;
    static constexpr format format_BGRA = format::format_BGRA;
    static constexpr format format_RGBA = format::format_RGBA;
    static constexpr format format_Gray = format::format_Gray;

    using device = std::shared_ptr<ps3eye_camera>;

    static constexpr uint16_t VENDOR_ID = 0x1415;
    static constexpr uint16_t PRODUCT_ID = 0x2000;

    ps3eye_camera(libusb_device *device);
    ~ps3eye_camera();
    ps3eye_camera(const ps3eye_camera&) = delete;
    void operator=(const ps3eye_camera&) = delete;

    bool init(uint32_t width = 0, uint32_t height = 0, uint16_t desiredFrameRate = 30, format outputFormat = format_BGR);
    [[nodiscard]] bool start();
    void stop();

    // Controls

    bool get_auto_gain() const { return autogain; }
    void set_auto_gain(bool val);
    bool get_auto_white_balance() const { return awb; }
    void set_auto_white_balance(bool val);
    uint8_t get_gain() const { return gain; }
    void set_gain(uint8_t val);
    uint8_t get_exposure() const { return exposure; }
    void set_exposure(uint8_t val);
    uint8_t getSharpness() const { return sharpness; }
    void set_sharpness(uint8_t val);
    uint8_t get_contrast() const { return contrast; }
    void set_contrast(uint8_t val);
    uint8_t get_brightness() const { return brightness; }
    void set_brightness(uint8_t val);
    uint8_t get_hue() const { return hue; }
    void set_hue(uint8_t val);
    uint8_t get_red_balance() const { return redblc; }
    void set_red_balance(uint8_t val);
    uint8_t get_blue_balance() const { return blueblc; }
    void set_blue_balance(uint8_t val);
    uint8_t get_green_balance() const { return greenblc; }
    void set_green_balance(uint8_t val);
    bool get_flip_h() const { return flip_h; }
    bool get_flip_v() const { return flip_v; }
    void set_flip(bool horizontal = false, bool vertical = false);
    bool get_test_pattern() const { return testPattern; }
    void set_test_pattern(bool enable);
    bool isStreaming() const { return is_streaming_; }
    bool isInitialized() const { return device_ && handle_; }
    bool getUSBPortPath(char *out_identifier, size_t max_identifier_length) const;

    // Get a frame from the camera. Notes:
    // - If there is no frame available, this function will block until one is
    // - The output buffer must be sized correctly, depending out the output format. See EOutputFormat.
    [[nodiscard]] bool get_frame(uint8_t* frame);

    uint32_t getWidth() const { return frame_width; }
    uint32_t getHeight() const { return frame_height; }
    uint16_t getFrameRate() const { return frame_rate; }
    bool setFrameRate(uint8_t val);
    uint32_t get_stride() const { return frame_width * get_depth(); }
    uint32_t get_depth() const;

    static const std::vector<device>& get_devices(bool force_refresh = false);

private:
    // usb ops
    uint16_t ov534_set_frame_rate(uint16_t frame_rate, bool dry_run = false);
    void ov534_set_led(int status);
    void ov534_reg_write(uint16_t reg, uint8_t val);
    uint8_t ov534_reg_read(uint16_t reg);
    int sccb_check_status();
    void sccb_reg_write(uint8_t reg, uint8_t val);
    uint8_t sccb_reg_read(uint16_t reg);
    void reg_w_array(const uint8_t (*data)[2], int len);
    void sccb_w_array(const uint8_t (*data)[2], int len);

    // controls
    uint8_t gain = 63;          // 0 <-> 63
    uint8_t exposure = 255;     // 0 <-> 255
    uint8_t hue = 143;          // 0 <-> 255
    uint8_t brightness = 20;    // 0 <-> 255
    uint8_t contrast = 37;      // 0 <-> 255
    uint8_t blueblc = 128;      // 0 <-> 255
    uint8_t redblc = 128;       // 0 <-> 255
    uint8_t greenblc = 128;     // 0 <-> 255
    uint8_t sharpness = 0;      // 0 <-> 63

    static bool devicesEnumerated;
    static std::vector<device> devices;

    uint32_t frame_width = 0;
    uint32_t frame_height = 0;
    uint16_t frame_rate = 0;
    format frame_output_format = format_Bayer;

    //usb stuff
    libusb_device *device_ = nullptr;
    libusb_device_handle *handle_ = nullptr;
    uint8_t usb_buf[64] = {};

    std::shared_ptr<struct URBDesc> urb = nullptr;

    bool awb = false;
    bool flip_h = false;
    bool flip_v = false;
    bool testPattern = false;
    bool autogain = false;
    bool is_streaming_ = false;

    void release();
    bool open_usb();
    void close_usb();

};
