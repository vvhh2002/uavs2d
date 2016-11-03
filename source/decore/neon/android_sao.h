#ifndef __ANDROID_SAO_H__
#define __ANDROID_SAO_H__

#include "android.h"

void SAO_EO_0_neon(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, int start_x, int end_x, int mb_height, char_t* mask);
void SAO_EO_90_neon(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, int start_y, int end_y, int mb_width);
void SAO_EO_135_neon(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, char_t* mask, int mb_height, int start_x_r0, int end_x_r0, int start_x_r, int end_x_r, int start_x_rn, int end_x_rn);
void SAO_EO_45_neon(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, char_t* mask, int mb_height, int start_x_r0, int end_x_r0, int start_x_r, int end_x_r, int start_x_rn, int end_x_rn);
void SAO_BO_neon(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, int start_band, int start_band_2, int mb_width, int mb_height);
#endif