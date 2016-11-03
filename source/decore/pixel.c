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

#include "common.h"

static void com_if_filter_cpy(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    int row;
    for (row = 0; row < height; row++) {
        memcpy(dst, src, sizeof(pel_t)* width);
        src += i_src;
        dst += i_dst;
    }
}

static void cpy_pel_to_uchar(const pel_t *src, int i_src, uchar_t *dst, int i_dst, int width, int height, int bit_size)
{
    int i, j;

    if (bit_size == 8) {
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                dst[j] = (uchar_t)src[j];
            }
            src += i_src;
            dst += i_dst;
        }
    } else { // bit_size == 10
        int shift = 2;
        int add = 2;
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                dst[j] = (uchar_t)COM_CLIP3(0, 255, (src[j] + 2) >> 2);
            }
            src += i_src;
            dst += i_dst;
        }
    }
}

static void cpy_pel_I420_to_uchar_YUY2(const pel_t *srcy, const pel_t *srcu, const pel_t *srcv, int i_src, int i_srcc, uchar_t *dst, int i_dst, int width, int height, int bit_size)
{
    int i, j, k, l;
    const pel_t *src1 = srcy + i_src;
    uchar_t *dst1 = dst + i_dst;

    if (bit_size == 8) {
        for (i = 0; i < height; i+=2) {
            for (j = 0, k = 0, l = 0; j < width; j += 2, k++, l += 4) {
                dst [l + 0] = (uchar_t)(srcy[j + 0]);
                dst [l + 1] = (uchar_t)(srcu[k]);
                dst [l + 2] = (uchar_t)(srcy[j + 1]);
                dst [l + 3] = (uchar_t)(srcv[k]);
                dst1[l + 0] = (uchar_t)(src1[j + 0]);
                dst1[l + 1] = (uchar_t)(srcu[k]);
                dst1[l + 2] = (uchar_t)(src1[j + 1]);
                dst1[l + 3] = (uchar_t)(srcv[k]);
            }
            srcy += 2 * i_src;
            src1 += 2 * i_src;
            dst  += 2 * i_dst;
            dst1 += 2 * i_dst;
            srcu += i_srcc;
            srcv += i_srcc;
        }
    } else {
        for (i = 0; i < height; i += 2) {
            for (j = 0, k = 0, l = 0; j < width; j += 2, k++, l += 4) {
                dst [l + 0] = (uchar_t)COM_CLIP3(0, 255, (srcy[j + 0] + 2) >> 2);
                dst [l + 1] = (uchar_t)COM_CLIP3(0, 255, (srcu[k    ] + 2) >> 2);
                dst [l + 2] = (uchar_t)COM_CLIP3(0, 255, (srcy[j + 1] + 2) >> 2);
                dst [l + 3] = (uchar_t)COM_CLIP3(0, 255, (srcv[k    ] + 2) >> 2);
                dst1[l + 0] = (uchar_t)COM_CLIP3(0, 255, (src1[j + 0] + 2) >> 2);
                dst1[l + 1] = (uchar_t)COM_CLIP3(0, 255, (srcu[k    ] + 2) >> 2);
                dst1[l + 2] = (uchar_t)COM_CLIP3(0, 255, (src1[j + 1] + 2) >> 2);
                dst1[l + 3] = (uchar_t)COM_CLIP3(0, 255, (srcv[k    ] + 2) >> 2);
            }
            srcy += 2 * i_src;
            src1 += 2 * i_src;
            dst += 2 * i_dst;
            dst1 += 2 * i_dst;
            srcu += i_srcc;
            srcv += i_srcc;
        }
    }
}

static void add_pel_clip(const pel_t *src1, int i_src1, const resi_t *src2, int i_src2, pel_t *dst, int i_dst, int width, int height, int bit_depth)
{
    int i, j;

    if (bit_depth == 8){
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                dst[j] = COM_CLIP3(0, 255, src1[j] + src2[j]);
            }
            src1 += i_src1;
            src2 += i_src2;
            dst += i_dst;
        }
    } else {
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                dst[j] = COM_CLIP3(0, 1023, src1[j] + src2[j]);
            }
            src1 += i_src1;
            src2 += i_src2;
            dst += i_dst;
        }
    }
}

void avg_pel(pel_t *dst, int i_dst, pel_t *src1, int i_src1, pel_t *src2, int i_src2, int width, int height)
{
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            dst[j] = (src1[j] + src2[j] + 1) >> 1;
        }
        dst += i_dst;
        src1 += i_src1;
        src2 += i_src2;
    }
}

static void padding_rows(pel_t *src, int i_src, int width, int height, int start, int rows, int pad)
{
    int i, j;
    pel_t *p;

    start = max(start, 0);
    rows  = min(rows, height - start);

    if (start + rows == height) {
        rows += pad;
        p = src + i_src * (height - 1);
        for (i = 1; i <= pad; i++) {
            memcpy(p + i_src * i, p, width * sizeof(pel_t));
        }
    }

    if (start == 0) {
        start = -pad;
        rows += pad;
        p = src;
        for (i = 1; i <= pad; i++) {
            memcpy(p - i_src * i, p, width * sizeof(pel_t));
        }
    }

    p = src + start * i_src;

    // left & right
    for (i = 0; i < rows; i++) {
        for (j = 0; j < pad; j++) {
            p[-pad + j] = p[0];
            p[width + j] = p[width - 1];
        }
        p += i_src;
    }

    
}

static void padding_rows_lr(pel_t *src, int i_src, int width, int height, int start, int rows, int pad)
{
    int i, j;
    pel_t *p;

    start = max(start, 0);

    if (start + rows > height) {
        rows = height - start;
    }

    p = src + start * i_src;

    // left & right
    for (i = 0; i < rows; i++) {
        for (j = 0; j < pad; j++) {
            p[-pad + j] = p[0];
            p[width + j] = p[width - 1];
        }
        p += i_src;
    }
}


void com_funs_init_pixel_opt()
{
    g_funs_handle.cpy_pel_I420_to_uchar_YUY2 = cpy_pel_I420_to_uchar_YUY2;

    g_funs_handle.cpy              = com_if_filter_cpy;
    g_funs_handle.cpy_pel_to_uchar = cpy_pel_to_uchar;
    g_funs_handle.add_pel_clip     = add_pel_clip;
    g_funs_handle.avg_pel          = avg_pel;
    g_funs_handle.padding_rows     = padding_rows;
    g_funs_handle.padding_rows_lr  = padding_rows_lr;
}