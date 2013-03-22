#if defined(_WIN32) || defined(WIN32)
#   ifdef __cplusplus
#       define FDAPI_EXTERN extern "C"
#   else
#       define FDAPI_EXTERN
#   endif
#   define FDAPI(ret) FDAPI_EXTERN __declspec(dllexport) ret __cdecl
#else
#   define FDAPI(ret) ret
#endif

struct face_detect_settings {
    unsigned char magic, quit, newOutput, widgetp;
    int redetect_ms, camera_id;
};

struct face_detect;
FDAPI(struct face_detect*) face_detect_init(const char* eyes_model,
	const char* nose_model,
	const char* mouth_model,
	const char* face_model,
	int capture_no,
	struct face_detect_settings* settings);
FDAPI(void) face_detect_free(struct face_detect *ctx);
FDAPI(int) face_detect_cycle(struct face_detect *ctx, float *data);
FDAPI(void) face_detect_zero(struct face_detect *ctx);

FDAPI(unsigned char*) face_detect_video(struct face_detect* ctx);

#define FD_VIDEO_WIDTH 252
#define FD_VIDEO_HEIGHT 189
#define FD_MAGIC 0x42

struct face_detect_shm {
    unsigned char zerop, received;
    float data[6];
    unsigned char pixels[FD_VIDEO_WIDTH * FD_VIDEO_HEIGHT * 3];
    struct face_detect_settings settings;
};
