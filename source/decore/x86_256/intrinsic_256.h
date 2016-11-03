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
#include <immintrin.h>

#include "../common.h"
#include "../commonStructures.h"

#ifdef _WIN32

#define _mm256_extract_epi64(a, i) (a.m256i_i64[i])
#define _mm256_extract_epi32(a, i) (a.m256i_i32[i])
#define _mm256_extract_epi16(a, i) (a.m256i_i16[i])
#define _mm256_extract_epi8(a, i)  (a.m256i_i8 [i])
#define _mm256_insert_epi64(a, v, i) (a.m256i_i64[i] = v)
#define _mm_extract_epi64(r, i) r.m128i_i64[i]

#else

#define _mm256_set_m128i(/* __m128i */ hi, /* __m128i */ lo)  _mm256_insertf128_si256(_mm256_castsi128_si256(lo), (hi), 0x1)
#define _mm256_loadu2_m128i(/* __m128i const* */ hiaddr, /* __m128i const* */ loaddr) _mm256_set_m128i(_mm_loadu_si128(hiaddr), _mm_loadu_si128(loaddr))

#define _mm256_storeu2_m128i(/* __m128i* */ hiaddr, /* __m128i* */ loaddr, \
                             /* __m256i */ a) \
    do { \
        __m256i _a = (a); /* reference a only once in macro body */ \
        _mm_storeu_si128((loaddr), _mm256_castsi256_si128(_a)); \
        _mm_storeu_si128((hiaddr), _mm256_extractf128_si256(_a, 0x1)); \
    } while (0)

#endif

ALIGNED_16(extern char intrinsic_mask16[15][16]);
ALIGNED_32(extern char intrinsic_mask32[32][32]);
ALIGNED_32(extern i16s_t intrinsic_mask_10bit[15][16]);

void add_pel_clip_sse256(const pel_t *src1, int i_src1, const resi_t *src2, int i_src2, pel_t *dst, int i_dst, int width, int height, int bit_depth);
void avg_pel_sse256(pel_t *dst, int i_dst, pel_t *src1, int i_src1, pel_t *src2, int i_src2, int width, int height);
void padding_rows_sse256(pel_t *src, int i_src, int width, int height, int start, int rows, int pad);
void padding_rows_lr_sse256(pel_t *src, int i_src, int width, int height, int start, int rows, int pad);

void add_pel_clip_sse256_10bit(const pel_t *src1, int i_src1, const resi_t *src2, int i_src2, pel_t *dst, int i_dst, int width, int height, int bit_depth);
void avg_pel_sse256_10bit(pel_t *dst, int i_dst, pel_t *src1, int i_src1, pel_t *src2, int i_src2, int width, int height);
void padding_rows_sse256_10bit(pel_t *src, int i_src, int width, int height, int start, int rows, int pad);
void padding_rows_lr_sse256_10bit(pel_t *src, int i_src, int width, int height, int start, int rows, int pad);

void SAO_on_block_sse256(com_pic_t *pic_src, com_pic_t *pic_dst, sap_param_t *saoBlkParam, int compIdx, int smb_index, int pix_y, int pix_x, int smb_pix_height, int smb_pix_width,
    int smb_available_left, int smb_available_right, int smb_available_up, int smb_available_down, int smb_available_upleft,
    int smb_available_upright, int smb_available_leftdown, int smb_available_rightdwon, int sample_bit_depth);

void alf_flt_one_block_sse256(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcu_height, int xPos, int lcu_width, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail);
void alf_flt_one_block_sse256_10bit(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcuHeight, int xPos, int lcuWidth, int *coef, int bit_depth, int isAboveAvail, int isBelowAvail);

void com_if_filter_cpy_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);

void com_if_filter_hor_4_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_hor_4_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_hor_8_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_hor_8_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_ver_4_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_ver_4_w32_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_ver_4_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_ver_8_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_ver_8_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_ver_8_w32_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_ver_8_w48_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_ver_8_w64_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);

void com_if_filter_hor_ver_4_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);
void com_if_filter_hor_ver_4_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);
void com_if_filter_hor_ver_8_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);
void com_if_filter_hor_ver_8_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);

void com_if_filter_hor_4_w8_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_4_w16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_4_w24_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_4_w32_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_hor_ver_4_w16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);
void com_if_filter_hor_ver_4_w24_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);
void com_if_filter_hor_8_w16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_hor_8_w24_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
void com_if_filter_ver_8_w16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_8_w24_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_hor_ver_8_w16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);
void com_if_filter_hor_ver_8_w24_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);


void deblock_edge_hor_sse256(pel_t *SrcPtr, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_hor_c_sse256(pel_t *SrcPtrU, pel_t *SrcPtrV, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_ver_sse256(pel_t *SrcPtr, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_ver_c_sse256(pel_t *SrcPtrU, pel_t *SrcPtrV, int stride, int Alpha, int Beta, uchar_t *flt_flag);

void idct_8x8_sse256(coef_t *blk, int shift, int clip);
void idct_16x16_sse256(coef_t *blk, int shift, int clip);
void idct_32x32_sse256(coef_t *blk, int shift, int clip);

void xPredIntraVertAdi_sse256(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight);
void xPredIntraHorAdi_sse256(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight);
void xPredIntraDCAdi_see256(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bAboveAvail, int bLeftAvail, int sample_bit_depth);

void inv_wavelet_B64_sse256(coef_t *blk);
void inv_wavelet_NSQT_Hor_sse256(coef_t *blk);
void inv_wavelet_NSQT_Ver_sse256(coef_t *blk);

#endif