/*****************************************************************************
*  Copyright (C) 2016 uavs2dec project,
*  National Engineering Laboratory for Video Technology(Shenzhen),
*  Digital Media R&D Center at Peking University Shenzhen Graduate School, China
*  Project Leader: Ronggang Wang <rgwang@pkusz.edu.cn>
*
*  Main Authors: Zhenyu Wang <wangzhenyu@pkusz.edu.cn>, Kui Fan <kuifan@pku.edu.cn>
*               Shenghao Zhang <1219759986@qq.com>£¬ Bingjie Han, Kaili Yao, Hongbin Cao,  Yueming Wang,
*               Jing Su, Jiaying Yan, Junru Li
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
*
* This program is also available under a commercial proprietary license.
* For more information, contact us at rgwang@pkusz.edu.cn.
*****************************************************************************/

#ifndef __INTRINSIC_H__
#define __INTRINSIC_H__

#include <mmintrin.h>
#include <emmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>

#include "../common.h"
#include "../commonStructures.h"

ALIGNED_16(extern char intrinsic_mask[15][16]);
ALIGNED_32(extern i16s_t intrinsic_mask_10bit[15][16]);

#ifdef _WIN32

#ifndef _WIN64
#define _mm_extract_epi64(a, i) (a.m128i_i64[i])
#endif

#endif

void com_if_filter_cpy_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy2_sse128 (const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy4_sse128 (const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy6_sse128 (const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy8_sse128 (const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy12_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy16_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);

void com_if_filter_cpy_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
#define com_if_filter_cpy2_sse128_10bit com_if_filter_cpy4_sse128
#define com_if_filter_cpy4_sse128_10bit com_if_filter_cpy8_sse128
void com_if_filter_cpy6_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
#define com_if_filter_cpy8_sse128_10bit com_if_filter_cpy16_sse128
void com_if_filter_cpy12_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy16_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);

void com_if_filter_hor_4_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_hor_8_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_4_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_8_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_hor_ver_4_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff_h, const char_t *coeff_v, int max_val);
void com_if_filter_hor_ver_8_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff_h, const char_t *coeff_v, int max_val);
void com_if_filter_hor_4_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_4_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_hor_ver_4_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff_h, const char_t *coeff_v, int max_val);
void com_if_filter_hor_8_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_8_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_hor_ver_8_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);

void cpy_pel_I420_to_uchar_YUY2_sse128(const pel_t *srcy, const pel_t *srcu, const pel_t *srcv, int i_src, int i_srcc, uchar_t *dst, int i_dst, int width, int height, int bit_size);
void add_pel_clip_sse128(const pel_t *src1, int i_src1, const resi_t *src2, int i_src2, pel_t *dst, int i_dst, int width, int height, int bit_depth);
void avg_pel_sse128(pel_t *dst, int i_dst, pel_t *src1, int i_src1, pel_t *src2, int i_src2, int width, int height);
void padding_rows_sse128(pel_t *src, int i_src, int width, int height, int start, int rows, int pad);
void padding_rows_lr_sse128(pel_t *src, int i_src, int width, int height, int start, int rows, int pad);

void cpy_pel_I420_to_uchar_YUY2_sse128_10bit(const pel_t *srcy, const pel_t *srcu, const pel_t *srcv, int i_src, int i_srcc, uchar_t *dst, int i_dst, int width, int height, int bit_size);
void add_pel_clip_sse128_10bit(const pel_t *src1, int i_src1, const resi_t *src2, int i_src2, pel_t *dst, int i_dst, int width, int height, int bit_depth);
void avg_pel_sse128_10bit(pel_t *dst, int i_dst, pel_t *src1, int i_src1, pel_t *src2, int i_src2, int width, int height);
void padding_rows_sse128_10bit(pel_t *src, int i_src, int width, int height, int start, int rows, int pad);
void padding_rows_lr_sse128_10bit(pel_t *src, int i_src, int width, int height, int start, int rows, int pad);


void deblock_edge_ver_sse128(pel_t *SrcPtr, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_hor_sse128(pel_t *SrcPtr, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_ver_c_sse128(pel_t *SrcPtrU, pel_t *SrcPtrV, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_hor_c_sse128(pel_t *SrcPtrU, pel_t *SrcPtrV, int stride, int Alpha, int Beta, uchar_t *flt_flag);

void deblock_edge_ver_sse128_10bit(pel_t *SrcPtr, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_hor_sse128_10bit(pel_t *SrcPtr, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_ver_c_sse128_10bit(pel_t *SrcPtrU, pel_t *SrcPtrV, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_hor_c_sse128_10bit(pel_t *SrcPtrU, pel_t *SrcPtrV, int stride, int Alpha, int Beta, uchar_t *flt_flag);

void idct_4x4_sse128(coef_t *blk, int shift, int clip);
void idct_8x8_sse128(coef_t *blk, int shift, int clip);
void idct_16x16_sse128(coef_t *blk, int shift, int clip);
void idct_32x32_sse128(coef_t *blk, int shift, int clip);

void idct_16x4_sse128(coef_t *blk, int shift, int clip);
void idct_32x8_sse128(coef_t *blk, int shift, int clip);

void idct_4x16_sse128(coef_t *blk, int shift, int clip);
void idct_8x32_sse128(coef_t *blk, int shift, int clip);

void inv_2nd_trans_hor_sse128(coef_t *blk, int i_blk, int shift, const i16s_t coef[4][4]);
void inv_2nd_trans_ver_sse128(coef_t *blk, int i_blk, int shift, const i16s_t coef[4][4]);
void inv_2nd_trans_sse128(coef_t *blk, int i_blk, int shift, int clip_depth, const i16s_t coef[4][4]);

void inv_wavelet_NSQT_Hor_sse128(coef_t *blk);
void inv_wavelet_NSQT_Ver_sse128(coef_t *blk);
void inv_wavelet_B64_sse128(coef_t *blk);

void SAO_on_block_sse128(void *pic_src, void *pic_dst, void *saoBlkParam, int compIdx, int smb_index, int pix_y, int pix_x, int smb_pix_height, int smb_pix_width,
                        int smb_available_left, int smb_available_right, int smb_available_up, int smb_available_down, int smb_available_upleft,
                        int smb_available_upright, int smb_available_leftdown, int smb_available_rightdwon, int sample_bit_depth);
void SAO_on_block_sse128_10bit(void *p1, void *p2, void *p3, int compIdx, int smb_index, int pix_y, int pix_x, int smb_pix_height,
                                int smb_pix_width, int smb_available_left, int smb_available_right, int smb_available_up, int smb_available_down,
                                int smb_available_upleft, int smb_available_upright, int smb_available_leftdown, int smb_available_rightdwon, int sample_bit_depth);
void alf_flt_one_block_sse128(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcu_height, int xPos, int lcu_width, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail);
void alf_flt_one_block_sse128_10bit(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcuHeight, int xPos, int lcuWidth, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail);

void xPredIntraAngAdi_X_3_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_5_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_7_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_9_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_11_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void xPredIntraAngAdi_X_4_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_6_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_8_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_10_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void xPredIntraAngAdi_Y_25_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_27_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_29_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_31_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void xPredIntraAngAdi_Y_26_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_28_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_30_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_32_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void xPredIntraAngAdi_XY_14_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_16_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_18_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_20_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_22_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void xPredIntraVertAdi_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight);
void xPredIntraHorAdi_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight);
void xPredIntraDCAdi_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bAboveAvail, int bLeftAvail, int bit_depth);
void xPredIntraPlaneAdi_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bit_depth);
void xPredIntraBiAdi_sse128(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bit_depth);

void xPredIntraAngAdi_X_3_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_4_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_5_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_6_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_7_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_8_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_9_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_10_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_11_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_14_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_16_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_18_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_20_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_22_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_25_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_26_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_27_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_28_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_29_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_30_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_31_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_32_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void xPredIntraDCAdi_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bAboveAvail, int bLeftAvail, int bit_depth);
void xPredIntraVertAdi_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight);
void xPredIntraHorAdi_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight);
void xPredIntraPlaneAdi_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bit_depth);
void xPredIntraBiAdi_sse128_10bit(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bit_depth);


#endif // #ifndef __INTRINSIC_H__