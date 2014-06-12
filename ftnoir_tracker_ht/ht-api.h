#pragma once
#ifndef HT_API
#   if defined(_WIN32) && !defined(MINGW)
#     define HT_API(t) __declspec(dllexport) t __stdcall
#   else
#    define HT_API(t) t
#   endif
#endif
#if !defined(_WIN32) && !defined(_isnan)
#  define _isnan isnan
#endif
#include <opencv2/core/core.hpp>
struct ht_context;
typedef struct ht_context headtracker_t;

typedef struct ht_config {
	float field_of_view;
	float classification_delay;
	int   pyrlk_pyramids;
	int   pyrlk_win_size_w;
	int   pyrlk_win_size_h;
    float ransac_max_inlier_error;
    float ransac_max_reprojection_error;
	int   max_keypoints;
	float keypoint_distance;
    int   force_width;
	int   force_height;
	int   force_fps;
	int   camera_index;
	bool  debug;
    int   ransac_num_iters;
    float ransac_min_features;
    float ransac_max_mean_error;
    float ransac_abs_max_mean_error;
    float flandmark_delay;
    double dist_coeffs[5];
} ht_config_t;

typedef struct {
    double rotx, roty, rotz;
    double tx, ty, tz;
	bool filled;
} ht_result_t;

HT_API(headtracker_t*) ht_make_context(const ht_config_t* config, const char* filename);
HT_API(void) ht_free_context(headtracker_t* ctx);
HT_API(const cv::Mat) ht_get_bgr_frame(headtracker_t* ctx);
HT_API(bool) ht_cycle(headtracker_t* ctx, ht_result_t* euler);
HT_API(void) ht_reset(headtracker_t* ctx);
