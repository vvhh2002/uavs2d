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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <memory.h>
#include "threadpool.h"
#include "headers.h"

/* ---------------------------------------------------------------------------
 * common tables
 */
extern tab_i16s_t tab_log2size[65];

extern tab_i16s_t tab_scan_4x4[16][2];
extern tab_i16s_t tab_scan_8x8[64][2];
extern tab_i16s_t tab_scan_16x16[256][2];

extern tab_i16s_t tab_scan_2x8[16][2];
extern tab_i16s_t tab_scan_8x2[16][2];
extern tab_i16s_t tab_scan_4x16[64][2];
extern tab_i16s_t tab_scan_16x4[64][2];
extern tab_i16s_t tab_scan_8x32[256][2];
extern tab_i16s_t tab_scan_32x8[256][2];

extern tab_i16s_t tab_scan_cg_8x8[64][2];
extern tab_i16s_t tab_scan_cg_16x16[256][2];
extern tab_i16s_t tab_scan_cg_32x32[1024][2];

extern tab_i16s_t (*tab_scan_list    [3])[2];
extern tab_i16s_t (*tab_scan_list_cg [4])[2];
extern tab_i16s_t (*tab_scan_list_hor[3])[2];
extern tab_i16s_t (*tab_scan_list_ver[3])[2];

/* ---------------------------------------------------------------------------
 * log level
 */
#define COM_LOG_NONE       (-1)
#define COM_LOG_ERROR      0
#define COM_LOG_WARNING    1
#define COM_LOG_INFO       2
#define COM_LOG_DEBUG      3

/* ---------------------------------------------------------------------------
 * date align
 */
#if defined(_WIN32)
#define DECLARE_ALIGNED(var, n) __declspec(align(n)) var
#else
#define DECLARE_ALIGNED(var, n) var __attribute__((aligned (n))) 
#endif
#define ALIGNED_32(var)    DECLARE_ALIGNED(var, 32)
#define ALIGNED_16(var)    DECLARE_ALIGNED(var, 16)
#define ALIGNED_8(var)    DECLARE_ALIGNED(var, 8)
#define ALIGNED_4(var)    DECLARE_ALIGNED(var, 4)

#define ALIGN_BASIC 32 // for new generation CPU with 256-bits SIMD
#define ALIGN_MASK (ALIGN_BASIC - 1)
#define ALIGN_POINTER(x) (x + ALIGN_MASK - (((intptr_t)x + ALIGN_MASK) & ((intptr_t)ALIGN_MASK)))

/* ---------------------------------------------------------------------------
 * basic math operations
 */
#define COM_ABS(a)         ((a) < (0) ? (-(a)) : (a))
#define COM_MAX(a, b)      ((a) > (b) ? (a) : (b))
#define COM_MIN(a, b)      ((a) < (b) ? (a) : (b))
#define COM_CLIP3( min, max, val) (((val)<(min))? (min):(((val)>(max))? (max):(val)))
#define COM_ADD_MODE(v, mode) ((v) & (mode - 1))

/* ---------------------------------------------------------------------------
 * memory opterations
 */
typedef union {
    i16u_t    i;
    uchar_t   c[2];
} com_union16_t;

typedef union {
    i32u_t    i;
    i16u_t    b[2];
    uchar_t   c[4];
} com_union32_t;

typedef union {
    i64u_t    i;
    i32u_t    a[2];
    i16u_t    b[4];
    uchar_t   c[8];
} com_union64_t;

#if defined(_WIN64)

#include <mmintrin.h>
#include <emmIntrin.h>
#include <tmmIntrin.h>
#include <smmIntrin.h>
#include <immintrin.h>

#define M128(src) (*(__m128*)(src))

#else

typedef struct {
    i64u_t    i[2];
} avs2_uint128_t;

typedef union {
    avs2_uint128_t i;
    i64u_t    a[2];
    i32u_t    b[4];
    i16u_t    c[8];
    uchar_t   d[16];
} avs2_union128_t;

#define M128(src)               (((avs2_union128_t*)(src))->i)

#endif // defined(_MSC_VER)

#define M16(src)                (((com_union16_t*)(src))->i)
#define M32(src)                (((com_union32_t*)(src))->i)
#define M64(src)                (((com_union64_t*)(src))->i)

#define CP16(dst,src)           M16(dst) = M16(src)
#define CP32(dst,src)           M32(dst) = M32(src)
#define CP64(dst,src)           M64(dst) = M64(src)
#define CP128(dst,src)          M128(dst) = M128(src)

/* ---------------------------------------------------------------------------
 * simd/neon/.... declare
 */
#define SIMD_NAME(func) func##_sse128

/* ---------------------------------------------------------------------------
 * memory alloc & free
 */
void *com_malloc(int i_size);
void  com_free(void *p);

/* ---------------------------------------------------------------------------
 * functions handle
 */
enum IPFilterConf {
    IPFILTER_H_8,
    IPFILTER_H_4,
    IPFILTER_V_8,
    IPFILTER_V_4,
    NUM_IPFILTER
};

enum IPFilterConf_Ext {
    IPFILTER_EXT_8,
    IPFILTER_EXT_4,
    NUM_IPFILTER_Ext
};


typedef struct {
    void(*ipcpy[NUM_IPFILTER_Ext][16]) (const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
    void(*ipflt[NUM_IPFILTER][16]) (const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val);
    void(*ipflt_ext[NUM_IPFILTER_Ext][16]) (const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);
    
    void(*cpy) (const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
    void(*cpy_pel_to_uchar)(const pel_t *src, int i_src, uchar_t *dst, int i_dst, int width, int height, int bit_size);
    void(*cpy_pel_I420_to_uchar_YUY2)(const pel_t *srcy, const pel_t *srcu, const pel_t *srcv, int i_src, int i_srcc, uchar_t *dst, int i_dst, int width, int height, int bit_size);

    void(*padding_rows)(pel_t *src, int i_src, int width, int height, int start, int rows, int pad);
    void(*padding_rows_lr)(pel_t *src, int i_src, int width, int height, int start, int rows, int pad);

    void(*add_pel_clip)(const pel_t *src1, int i_src1, const resi_t *src2, int i_src2, pel_t *dst, int i_dst, int width, int height, int bit_depth);

    void(*avg_pel)(pel_t *dst, int i_dst, pel_t *src1, int i_src1, pel_t *src2, int i_src2, int width, int height);

    void(*deblock_edge[2])(pel_t *src, int stride, int alpha, int beta, uchar_t *flt_flag);
    void(*deblock_edge_c[2])(pel_t *srcu, pel_t *srcv, int stride, int alpha, int beta, uchar_t *flt_flag);

    void(*idct_sqt[5])(coef_t *blk, int shift, int clip);
    void(*idct_hor[3])(coef_t *blk, int shift, int clip);
    void(*idct_ver[3])(coef_t *blk, int shift, int clip);
    void(*inv_2nd_hor)(coef_t *blk, int i_blk, int shift, const i16s_t coef[4][4]);
    void(*inv_2nd_ver)(coef_t *blk, int i_blk, int shift, const i16s_t coef[4][4]);
    void(*inv_2nd)(coef_t *blk, int i_blk, int shift, int clip_depth, const i16s_t coef[4][4]);
    void(*inv_wavelet)(coef_t *blk);
    void(*inv_wavelet_hor)(coef_t *blk);
    void(*inv_wavelet_ver)(coef_t *blk);

    void(*alf_flt)(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcu_height, int xPos, int lcu_width, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail);
    void(*alf_flt_fix)(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcu_height, int xPos, int lcu_width, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail);

    void(*sao_flt)(void *pic_src, void *pic_dst, void *saoBlkParam, int compIdx, int smb_index, int pix_y, int pix_x, int smb_pix_height, int smb_pix_width,\
        int smb_available_left, int smb_available_right, int smb_available_up, int smb_available_down, int smb_available_upleft,\
        int smb_available_upright, int smb_available_leftdown, int smb_available_rightdwon, int sample_bit_depth);

    void(*intra_pred_ang_x [ 9])(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
    void(*intra_pred_ang_y [ 8])(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
    void(*intra_pred_ang_xy[11])(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

    void(*intra_pred_dc)(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bAboveAvail, int bLeftAvail, int sample_bit_depth);
    void(*intra_pred_ver)(pel_t *pSrc, pel_t *dst, int i_dst, int bs_x, int bs_y);
    void(*intra_pred_hor)(pel_t *pSrc, pel_t *dst, int i_dst, int bs_x, int bs_y);
    void(*intra_pred_plane)(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int sample_bit_depth);
    void(*intra_pred_bi)(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int sample_bit_depth);

} funs_handle_t;

extern funs_handle_t g_funs_handle;

/* ---------------------------------------------------------------------------
 * basic frame picture
 */
typedef struct {
    uchar_t *p_data;
    int i_data_len;

    pel_t *p_y; 
    pel_t *p_u;
    pel_t *p_v;
    
    int i_width;
    int i_height;
    int i_widthc;
    int i_heightc;
    int i_stride;
    int i_stridec;

    int pic_memory;
} com_pic_t;

typedef struct com_frm_t {
    /* referenced counter, label life-time of this frame */
    int usecnt;

    /* inited and used by ref buffer controler, must be copy to com_ref_t after inited */
    i64s_t imgtr;
    i64s_t reftrs[4];

    /* inited and used by ref buffer controler */
    int layer;
    i64s_t coiref;

    /* used to control output in ctrl thread */
    i64s_t bak_imgtr;
    int b_refed;

    /* store frame data, and used for other frame */
    char_t *b4_info_buf;

    i16s_t(*mvbuf)[2];
    char_t *refbuf;
    com_pic_t *yuv_data;

    /* used for threads sync */
    int linend;
    avs2_pthread_mutex_t mutex;
    avs2_pthread_cond_t  cond;
    
} com_frm_t;

com_pic_t* com_pic_yuv_create(int width, int height, int pad);
void com_pic_yuv_destroy(com_pic_t* pic);
void com_pic_yuv_copy_ext(com_pic_t *dst, com_pic_t *src); // copy pic & padding area
void com_pic_yuv_copy_rows_ext(com_pic_t *dst, com_pic_t *src, int start, int end, int cpy_flag[3]);
void com_pic_yuv_padding_rows(com_pic_t *pic, int start, int rows);
void com_pic_yuv_padding_rows_lr(com_pic_t *pic, int start, int rows, int cpy_flag[3]);
void com_pic_yuv_copy_block(com_pic_t *dst, com_pic_t *src, int x, int y, int width, int height, int plane);

/* ---------------------------------------------------------------------------
 * log information
 */
void com_log(int i_level, const char_t *psz_fmt, ...);

/* ---------------------------------------------------------------------------
 * threads operations
 */
void check_ref_avaliable(com_frm_t *frm, int line_num);


/* ---------------------------------------------------------------------------
 * function handle init for c coding
 */
void com_funs_init_intra_pred();
void com_funs_init_ip_filter();
void com_funs_init_pixel_opt();
void com_funs_init_deblock_filter();
void com_funs_init_dct();
void com_funs_init_alf_filter();

void com_init_intrinsic_256();
void com_init_intrinsic();
void com_init_neon128();

void com_init_intrinsic_256_10bit();
void com_init_intrinsic_10bit();

#endif // #ifndef __COMMON_H__