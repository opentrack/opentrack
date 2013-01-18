#pragma once
#ifndef HT_API
#ifndef __cplusplus
# define HT_EXTERN 
#else
# define HT_EXTERN extern "C" 
#endif
#   if defined(_WIN32) && !defined(MINGW)
#     define HT_API(t) HT_EXTERN __declspec(dllexport) t __stdcall
#   else
#    define HT_API(t) HT_EXTERN t
#   endif
#endif
#if !defined(_WIN32) && !defined(_isnan)
#  define _isnan isnan
#endif
#include <stdio.h>
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
    int   keypoint_quality;
	float keypoint_distance;
    float keypoint_3distance;
    int   force_width;
	int   force_height;
	int   force_fps;
	int   camera_index;
	bool  debug;
    int   ransac_num_iters;
    float ransac_min_features;
    float ransac_max_mean_error;
    float ransac_abs_max_mean_error;
} ht_config_t;

typedef struct {
    double rotx, roty, rotz;
    double tx, ty, tz;
	bool filled;
} ht_result_t;

typedef enum {
	cfg_type_float = 0,
	cfg_type_int   = 1,
	cfg_type_bool  = 2,
    cfg_type_double = 3
} ht_cfg_type_t;

typedef union
{
	double d;
	float f;
	int i;
} ht_cfg_value_t;

typedef struct {
	const char* name;
	int offset;
	ht_cfg_type_t type;
	ht_cfg_value_t default_value;
	ht_cfg_value_t min;
	ht_cfg_value_t max;
	const char* docstring;
} ht_reflection_t;

typedef struct {
    int rows, cols, channels;
    unsigned char* data;
} ht_frame_t;

HT_API(headtracker_t*) ht_make_context(const ht_config_t* config, const char* filename);
HT_API(void) ht_load_config(FILE* stream, ht_config_t* cfg);
HT_API(void) ht_free_context(headtracker_t* ctx);
HT_API(void) ht_get_bgr_frame(headtracker_t* ctx, ht_frame_t* ret);
HT_API(void) ht_make_config(ht_config_t* cfg);
HT_API(bool) ht_cycle(headtracker_t* ctx, ht_result_t* euler);
HT_API(void) ht_reset(headtracker_t* ctx);
