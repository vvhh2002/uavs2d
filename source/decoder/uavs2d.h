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

#ifndef __ULTRA_AVS2__
#define __ULTRA_AVS2__


#ifdef __cplusplus
extern "C" { 
#endif
               
/* ---------------------------------------------------------------------------
 * build config
 */
#define OUTPUT_BG_FRM 1

#if defined(_WIN32)

#ifdef ULTRAAVS2DEC_EXPORTS
#define AVS2_API __declspec(dllexport)
#else
#define AVS2_API __declspec(dllimport)
#endif

#else 

#define AVS2_API
#define __cdecl

#endif 

/* ---------------------------------------------------------------------------
 * output type
 */
#define AVS2_OUT_I420        0         /* one picture was decoded */
#define AVS2_OUT_YUY2        1         /* sequence header was decoded */


/* ---------------------------------------------------------------------------
 * type of decoder state 
 */
#define AVS2_TYPE_DECODED    1         /* one picture was decoded */
#define AVS2_TYPE_SEQ        2         /* sequence header was decoded */
#define AVS2_TYPE_ERROR     -1         /* error, current frame was not decoded */
#define AVS2_TYPE_NEEDMORE  -2         /* nothing was decoded, need more data */
#define AVS2_TYPE_DROP      -3         /* nothing was decoded, drop this frame */

/* ---------------------------------------------------------------------------
 * decoded frame type
 */
#define AVS2_I_IMG  1
#define AVS2_G_IMG  2
#define AVS2_GB_IMG 3
#define AVS2_S_IMG  4
#define AVS2_P_IMG  5
#define AVS2_F_IMG  6
#define AVS2_B_IMG  7

typedef struct {            
    int     profile_id;
    int     level_id;
    int     progressive_seq;
    int     img_width;
    int     img_height;
    int     output_bit_depth;
    int     frame_rate_code;
} avs2_info_t;

typedef struct avs2_frame_t {
    unsigned char     *bs_buf;      // [in ]
    int                bs_len;      // [in ]
    int                dec_stats;   // [out]

    unsigned long long pts;         // [in/out]

    unsigned int       frm_type;    // [out]
    unsigned int       qp;          // [out]

    int                i_stride;    // [in ]
    int                i_stridec;   // [in ]
    int                i_output_type;
    unsigned char     *p_buf_y;     // [in: pointer; out: data]
    unsigned char     *p_buf_u;     // [in: pointer; out: data]
    unsigned char     *p_buf_v;     // [in: pointer; out: data]

    avs2_info_t        info;        // [out]

} avs2_frame_t;


/**
 * ===========================================================================
 * function handler defines
 * ===========================================================================
 */

// rec_threads:
//    0: single thread for aec and rec
//    1: one aec thread and one rec thread
//    X: one aec thread and X rec threads
AVS2_API void *__cdecl uavs2d_lib_create(int frm_threads, int rec_threads);

AVS2_API void  __cdecl uavs2d_lib_decode(void *handle, avs2_frame_t *frm);
AVS2_API void  __cdecl uavs2d_lib_destroy(void *handle);
AVS2_API void  __cdecl uavs2d_lib_flush(void *handle, avs2_frame_t *frm);

typedef void *(__cdecl *uavs2d_lib_create_t)(int threads, int rec_threads);
typedef void  (__cdecl *uavs2d_lib_decode_t)(void *handle, avs2_frame_t *frm);
typedef void  (__cdecl *uavs2d_lib_destroy_t)(void *handle);
typedef void  (__cdecl *uavs2d_lib_flush_t)(void *handle, avs2_frame_t *frm);


#ifdef __cplusplus
}
#endif

#endif  /* __ULTRA_AVS2__ */
