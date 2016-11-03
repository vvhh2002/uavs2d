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

#include <assert.h>

#include "common.h"
#include "inter-prediction.h"

ALIGNED_16(tab_char_t if_coef_luma[4][8]) = {
    {  0, 0,   0, 64,  0,   0, 0,  0 }, // for full-pixel, no use
    { -1, 4, -10, 57, 19,  -7, 3, -1 },
    { -1, 4, -11, 40, 40, -11, 4, -1 },
    { -1, 3,  -7, 19, 57, -10, 4, -1 }
};

ALIGNED_16(tab_char_t if_coef_chroma[9][4]) = {
    {  0, 64,  0,  0 }, // for full-pixel, no use
    { -4, 62,  6,  0 },
    { -6, 56, 15, -1 },
    { -5, 47, 25, -3 },
    { -4, 36, 36, -4 },
    { -3, 25, 47, -5 },
    { -1, 15, 56, -6 },
    {  0,  6, 62, -4 },
    {  0,  0,  0,  0 }, // padding buffer
};

void set_ref_info(avs2_dec_t *h_dec)
{
    int i;

    com_ref_t *refs = h_dec->ref_list;

    for (i = 0; i < h_dec->i_refs; i++) {
        refs[i].dist[0] = COM_ADD_MODE(h_dec->img_tr_ext   * 2 - refs[i].frm->imgtr * 2, 512);
        refs[i].dist[1] = COM_ADD_MODE(h_dec->imgtr_next_P * 2 - h_dec->img_tr_ext  * 2, 512);

        refs[i].ref_imgtr = refs[i].frm->imgtr;
        
        memcpy(refs[i].ref_reftrs, refs[i].frm->reftrs, sizeof(refs[i].frm->reftrs));
    }

    if (h_dec->type == B_IMG) {
        refs[0].dist[0] = refs[1].dist[0];
    }
}

/*
*************************************************************************
* Function:
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
int scale_motion_vector(avs2_dec_t *h_dec, int motion_vector, int currblkref, int neighbourblkref, int ref)    //qyu 0820 modified , int currsmbtype, int neighboursmbtype, int block_y_pos, int curr_block_y, int ref, int direct_mv)
{
    if (motion_vector == 0) {
        return 0;
    } else  {
        int sign = (motion_vector > 0 ? 1 : -1);
        int mult_distance;
        int devide_distance;
        int b_fwd = ref >= 0 ? 0 : 1;

        motion_vector = abs(motion_vector);

        mult_distance = h_dec->ref_list[currblkref].dist[b_fwd];
        devide_distance = h_dec->ref_list[neighbourblkref].dist[b_fwd];

        motion_vector = sign * ((motion_vector * mult_distance * (MULTI / devide_distance) + HALF_MULTI) >> OFFSET);

        motion_vector = Clip3(-32768, 32767, motion_vector);

        return motion_vector;
    }
}

void com_if_filter_cpy(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    int row;
    for (row = 0; row < height; row++) {
        memcpy(dst, src, sizeof(pel_t)* width);
        src += i_src;
        dst += i_dst;
    }
}

#define FLT_8TAP_HOR(src, i, coef) ( \
    (src)[i-3] * (coef)[0] + \
    (src)[i-2] * (coef)[1] + \
    (src)[i-1] * (coef)[2] + \
    (src)[i  ] * (coef)[3] + \
    (src)[i+1] * (coef)[4] + \
    (src)[i+2] * (coef)[5] + \
    (src)[i+3] * (coef)[6] + \
    (src)[i+4] * (coef)[7])

#define FLT_8TAP_VER(src, i, i_src, coef) ( \
    (src)[i-3 * i_src] * (coef)[0] + \
    (src)[i-2 * i_src] * (coef)[1] + \
    (src)[i-1 * i_src] * (coef)[2] + \
    (src)[i          ] * (coef)[3] + \
    (src)[i+1 * i_src] * (coef)[4] + \
    (src)[i+2 * i_src] * (coef)[5] + \
    (src)[i+3 * i_src] * (coef)[6] + \
    (src)[i+4 * i_src] * (coef)[7])

#define FLT_4TAP_HOR(src, i, coef) ( \
    (src)[i - 1] * (coef)[0] + \
    (src)[i    ] * (coef)[1] + \
    (src)[i + 1] * (coef)[2] + \
    (src)[i + 2] * (coef)[3])

#define FLT_4TAP_VER(src, i, i_src, coef) ( \
    (src)[i-1 * i_src] * (coef)[0] + \
    (src)[i          ] * (coef)[1] + \
    (src)[i+1 * i_src] * (coef)[2] + \
    (src)[i+2 * i_src] * (coef)[3])

static void com_if_filter_hor_4(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
    int row, col;
    int sum, val;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            sum = FLT_4TAP_HOR(src, col, coeff);
            val = (sum + 32) >> 6;
            dst[col] = COM_CLIP3(0, max_val, val);
        }
        src += i_src;
        dst += i_dst;
    }
}

static void com_if_filter_hor_8(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
    int row, col;
    int sum, val;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            sum = FLT_8TAP_HOR(src, col, coeff);
            val = (sum + 32) >> 6;
            dst[col] = COM_CLIP3(0, max_val, val);
        }
        src += i_src;
        dst += i_dst;
    }
}

static void com_if_filter_ver_4(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
    int row, col;
    int sum, val;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            sum = FLT_4TAP_VER(src, col, i_src, coeff);
            val = (sum + 32) >> 6;
            dst[col] = COM_CLIP3(0, max_val, val);
        }
        src += i_src;
        dst += i_dst;
    }
}

static void com_if_filter_ver_8(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
    int row, col;
    int sum, val;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            sum = FLT_8TAP_VER(src, col, i_src, coeff);
            val = (sum + 32) >> 6;
            dst[col] = COM_CLIP3(0, max_val, val);
        }
        src += i_src;
        dst += i_dst;
    }
}

static void com_if_filter_hor_ver_4(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff_h, const char_t *coeff_v, int max_val)
{
    int row, col;
    int sum, val;
    int add1, shift1;
    int add2, shift2;

    ALIGNED_16(i16s_t tmp_res[(32 + 3) * 32]);
    i16s_t *tmp;

    if (max_val == 255) { // 8 bit_depth
        shift1 = 0;
        shift2 = 12;
    } else { // 10 bit_depth
        shift1 = 2;
        shift2 = 10;
    }

    add1 = (1 << (shift1)) >> 1;
    add2 = 1 << (shift2 - 1);

    src += -1 * i_src;
    tmp = tmp_res;

    if (shift1) {
        for (row = -1; row < height + 2; row++) {
            for (col = 0; col < width; col++) {
                sum = FLT_4TAP_HOR(src, col, coeff_h);
                tmp[col] = (sum + add1) >> shift1;
            }
            src += i_src;
            tmp += 32;
        }
    } else {
        for (row = -1; row < height + 2; row++) {
            for (col = 0; col < width; col++) {
                tmp[col] = FLT_4TAP_HOR(src, col, coeff_h);
            }
            src += i_src;
            tmp += 32;
        }
    }

    tmp = tmp_res + 1 * 32;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            sum = FLT_4TAP_VER(tmp, col, 32, coeff_v);
            val = (sum + add2) >> shift2;
            dst[col] = COM_CLIP3(0, max_val, val);
        }
        dst += i_dst;
        tmp += 32;
    }
}

static void com_if_filter_hor_ver_8(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff_h, const char_t *coeff_v, int max_val)
{
    int row, col;
    int sum, val;
    int add1, shift1;
    int add2, shift2;

    ALIGNED_16(i16s_t tmp_res[(64 + 7) * 64]);
    i16s_t *tmp;

    if (max_val == 255) { // 8 bit_depth
        shift1 = 0;
        shift2 = 12;
    } else { // 10 bit_depth
        shift1 = 2;
        shift2 = 10;
    }

    add1 = (1 << (shift1)) >> 1;
    add2 = 1 << (shift2 - 1);

    src += -3 * i_src;
    tmp = tmp_res;

    if (shift1) {
        for (row = -3; row < height + 4; row++) {
            for (col = 0; col < width; col++) {
                sum = FLT_8TAP_HOR(src, col, coeff_h);
                tmp[col] = (sum + add1) >> shift1;
            }
            src += i_src;
            tmp += 64;
        }
    } else {
        for (row = -3; row < height + 4; row++) {
            for (col = 0; col < width; col++) {
                tmp[col] = FLT_8TAP_HOR(src, col, coeff_h);
            }
            src += i_src;
            tmp += 64;
        }
    }

    tmp = tmp_res + 3 * 64;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            sum = FLT_8TAP_VER(tmp, col, 64, coeff_v);
            val = (sum + add2) >> shift2;
            dst[col] = COM_CLIP3(0, max_val, val);
        }
        dst += i_dst;
        tmp += 64;
    }
}

void core_inter_get_luma(int x_pos, int y_pos, int width, int height, pel_t *dst, int i_dst, com_frm_t* p_ref_frm, int bit_depth, int check_ref)
{
    int max_pel_value = (1 << bit_depth) - 1;
    int dx = x_pos & 3;
    int dy = y_pos & 3;
    com_pic_t *p_ref = p_ref_frm->yuv_data;
    pel_t *src;
    int i_src = p_ref->i_stride;
    int width_idx = width / 4 - 1;

    int min_fpel_x = -68;
    int min_fpel_y = -68;
    int max_fpel_x = p_ref->i_width + 4;
    int max_fpel_y = p_ref->i_height + 4;

    x_pos = (x_pos - dx) >> 2;
    y_pos = (y_pos - dy) >> 2;

    x_pos = COM_CLIP3(min_fpel_x, max_fpel_x, x_pos);
    y_pos = COM_CLIP3(min_fpel_y, max_fpel_y, y_pos);

    src = p_ref->p_y + y_pos * i_src + x_pos;

    if (check_ref) {
        check_ref_avaliable(p_ref_frm, y_pos + height + 8);
    }

    if (dx == 0 && dy == 0) {   
        g_funs_handle.ipcpy[IPFILTER_EXT_8][width_idx](src, i_src, dst, i_dst, width, height);
    } else if (dy == 0) { 
        g_funs_handle.ipflt[IPFILTER_H_8][width_idx](src, i_src, dst, i_dst, width, height, if_coef_luma[dx], max_pel_value);
    } else if (dx == 0) {
        g_funs_handle.ipflt[IPFILTER_V_8][width_idx](src, i_src, dst, i_dst, width, height, if_coef_luma[dy], max_pel_value);
    } else {
        g_funs_handle.ipflt_ext[IPFILTER_EXT_8][width_idx](src, i_src, dst, i_dst, width, height, if_coef_luma[dx], if_coef_luma[dy], max_pel_value);
    }
}


void core_inter_get_chroma(pel_t *dst, int i_dst, com_frm_t *p_ref_frm, int uv, int start_x, int start_y, int width, int height, int bit_depth, int check_ref)
{
    int posx, posy;

    int shift1 = bit_depth - 8;
    int shift2 = 20 - bit_depth;

    int add1 = (1 << (shift1)) >> 1;
    int add2 = 1 << (19 - bit_depth);

    int max_pel_value = (1 << bit_depth) - 1;
    com_pic_t *p_ref = p_ref_frm->yuv_data;

    pel_t *dst_bakup = dst;
    pel_t *src = uv ? p_ref->p_v : p_ref->p_u;
    int i_src = p_ref->i_stridec;
    int width_idx = width / 2 - 1;
    int min_fpel_x = - 34;
    int min_fpel_y = - 34;
    int max_fpel_x = p_ref->i_widthc + 2;
    int max_fpel_y = p_ref->i_heightc + 2;

    int sx, sy;

    sx = COM_CLIP3(min_fpel_x, max_fpel_x, start_x >> 3);
    sy = COM_CLIP3(min_fpel_y, max_fpel_y, start_y >> 3);

    if (check_ref) {
        check_ref_avaliable(p_ref_frm, 2 * (sy + height + 4));
    }

    posx = (start_x & 7);
    posy = (start_y & 7);

    src += sy * i_src + sx;

    if (posx == 0 && posy == 0) {
        g_funs_handle.ipcpy[IPFILTER_EXT_4][width_idx](src, i_src, dst, i_dst, width, height);
    } else if (posy == 0) {
        g_funs_handle.ipflt[IPFILTER_H_4][width_idx](src, i_src, dst, i_dst, width, height, if_coef_chroma[posx], max_pel_value);
    } else if (posx == 0){
        g_funs_handle.ipflt[IPFILTER_V_4][width_idx](src, i_src, dst, i_dst, width, height, if_coef_chroma[posy], max_pel_value);
    } else {
        g_funs_handle.ipflt_ext[IPFILTER_EXT_4][width_idx](src, i_src, dst, i_dst, width, height, if_coef_chroma[posx], if_coef_chroma[posy], max_pel_value);
    }

}

void com_funs_init_ip_filter()
{
    int i;

    for (i = 0; i < 16; i++) {
        g_funs_handle.ipcpy[IPFILTER_EXT_4][i] = com_if_filter_cpy;
        g_funs_handle.ipcpy[IPFILTER_EXT_8][i] = com_if_filter_cpy;
        g_funs_handle.ipflt[IPFILTER_H_4][i] = com_if_filter_hor_4;
        g_funs_handle.ipflt[IPFILTER_H_8][i] = com_if_filter_hor_8;
        g_funs_handle.ipflt[IPFILTER_V_4][i] = com_if_filter_ver_4;
        g_funs_handle.ipflt[IPFILTER_V_8][i] = com_if_filter_ver_8;
        g_funs_handle.ipflt_ext[IPFILTER_EXT_4][i] = com_if_filter_hor_ver_4;
        g_funs_handle.ipflt_ext[IPFILTER_EXT_8][i] = com_if_filter_hor_ver_8;
    }
}
