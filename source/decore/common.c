/*****************************************************************************
*  Copyright (C) 2016 uavs2dec project,
*  National Engineering Laboratory for Video Technology(Shenzhen),
*  Digital Media R&D Center at Peking University Shenzhen Graduate School, China
*  Project Leader: Ronggang Wang <rgwang@pkusz.edu.cn>
*
*  Main Authors: Zhenyu Wang <wangzhenyu@pkusz.edu.cn>, Kui Fan <kuifan@pku.edu.cn>
*               Shenghao Zhang <1219759986@qq.com>�� Bingjie Han, Kaili Yao, Hongbin Cao,  Yueming Wang,
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
#include "headers.h"

const int cst_log_level = 2;

funs_handle_t g_funs_handle;

/****************************************************************************
 * com_malloc: malloc & free
 ****************************************************************************/
void *com_malloc(int i_size)
{
    int mask = ALIGN_BASIC - 1;
    uchar_t *align_buf=NULL;
    uchar_t *buf = (uchar_t *)malloc(i_size + mask + sizeof(void **));

    if (buf) {
        align_buf = buf + mask + sizeof(void **);
        align_buf -= (intptr_t)align_buf & mask;
        *(((void **)align_buf) - 1) = buf;
    } else {
        com_log(COM_LOG_ERROR, (char_t *)"malloc of size %d failed\n", i_size);
    }
    memset(align_buf, 0, i_size);
    return align_buf;
}

void com_free(void *p)
{
    if (p) {
        free(*(((void **)p) - 1));
    } else {
        com_log(COM_LOG_WARNING, (char_t *)"free a NULL pointer\n");
    }
}

/****************************************************************************
 * create and destroy a yuv picture:
 ****************************************************************************/
com_pic_t* com_pic_yuv_create(int width, int height, int pad)
{
    int widthc = width >> 1;
    int heightc = height >> 1;
    int padc = pad >> 1;
    int i_stride = (width + 2 * pad + 31) >> 5 << 5;
    int i_stridec = (widthc + 2 * padc + 31) >> 5 << 5;
    int luma_size = i_stride * (height + 2 * pad) * sizeof(pel_t);
    int chroma_size = i_stridec * (heightc + 2 * padc) * sizeof(pel_t);
    int total = (luma_size + ALIGN_MASK) + (chroma_size + ALIGN_MASK) * 2 + sizeof(com_pic_t);
    com_pic_t *ret;
    uchar_t* p;

    if (width % 4 != 0 && height % 4 != 0) {
        com_log(COM_LOG_ERROR, (char_t *)"%s: picture's width and height are not correct: %d x %d \n", __FUNCTION__, width, height);
    }

    ret = (com_pic_t *)com_malloc(total);

    ret->pic_memory = total;
    ret->i_width    = width;
    ret->i_height   = height;
    ret->i_widthc   = widthc;
    ret->i_heightc  = heightc;
    ret->i_stride   = i_stride;
    ret->i_stridec  = i_stridec;

    ret->p_data = (uchar_t*)ret + sizeof(com_pic_t);
    ret->i_data_len = total - sizeof(com_pic_t);

    p = (uchar_t*)ret + sizeof(com_pic_t);
    p = ALIGN_POINTER(p);
    ret->p_y = (pel_t*)p + pad * (ret->i_stride) + pad;

    p += sizeof(pel_t) * (height + 2 * pad) * ret->i_stride;
    ret->p_u = (pel_t*)p + padc * (ret->i_stridec) + padc;

    p += sizeof(pel_t) * ((heightc + 2 * padc) * ret->i_stridec);
    ret->p_v = (pel_t*)p + padc * (ret->i_stridec) + padc;

    return ret;
}

void com_pic_yuv_destroy(com_pic_t* pic)
{
    com_free((void*)pic);
}

void com_pic_yuv_copy_ext(com_pic_t *dst, com_pic_t *src)
{
    assert(dst->i_width == src->i_width);
    assert(dst->i_height == src->i_height);
    assert(dst->i_widthc == src->i_widthc);
    assert(dst->i_heightc == src->i_heightc);
    assert(dst->i_stride == src->i_stride);
    assert(dst->i_stridec == src->i_stridec);
    assert(dst->i_data_len == src->i_data_len);

    memcpy(dst->p_data, src->p_data, src->i_data_len);
}

void com_pic_yuv_copy_rows_ext(com_pic_t *dst, com_pic_t *src, int start, int end, int cpy_flag[3])
{
    pel_t *ps, *pd;
    int i_s;
    int i_size;
    int rows;

    assert(dst->i_width    == src->i_width   );
    assert(dst->i_height   == src->i_height  );
    assert(dst->i_widthc   == src->i_widthc  );
    assert(dst->i_heightc  == src->i_heightc );
    assert(dst->i_stride   == src->i_stride  );
    assert(dst->i_stridec  == src->i_stridec );
    assert(dst->i_data_len == src->i_data_len);

    start = max(start, 0);
    end   = min(end, src->i_height);

    rows = end - start;

    if (start + rows > src->i_height) {
        rows = src->i_height - start;
    }

    if (cpy_flag[0]) {
        i_s = src->i_stride;
        i_size = rows * i_s - (i_s - src->i_width);
        pd = dst->p_y + start * i_s;
        ps = src->p_y + start * i_s;
        memcpy(pd, ps, sizeof(pel_t)* i_size);
    }

    i_s = src->i_stridec;
    i_size = rows / 2 * i_s - (i_s - src->i_widthc);

    if (cpy_flag[1]) {
        pd = dst->p_u + start / 2 * i_s;
        ps = src->p_u + start / 2 * i_s;
        memcpy(pd, ps, sizeof(pel_t)* i_size);
    }
    if (cpy_flag[2]) {
        pd = dst->p_v + start / 2 * i_s;
        ps = src->p_v + start / 2 * i_s;
        memcpy(pd, ps, sizeof(pel_t)* i_size);
    }
}

void com_pic_yuv_copy_block(com_pic_t *dst, com_pic_t *src, int x, int y, int width, int height, int plane)
{
    int i;
    pel_t *d, *s;
    int i_d, i_s;
    switch (plane) {
    case 0:
        d = dst->p_y;
        s = src->p_y;
        i_d = dst->i_stride;
        i_s = src->i_stride;
        break;
    case 1:
        d = dst->p_u;
        s = src->p_u;
        i_d = dst->i_stridec;
        i_s = src->i_stridec;
        break;
    case 2:
        d = dst->p_v;
        s = src->p_v;
        i_d = dst->i_stridec;
        i_s = src->i_stridec;
        break;
    default:
        break;
    }

    s += y * i_s + x;
    d += y * i_d + x;

    for (i = 0; i < height; i++) {
        memcpy(d, s, width * sizeof(pel_t));
        d += i_d;
        s += i_s;
    }
}

void com_pic_yuv_padding_rows(com_pic_t *pic, int start, int rows)
{
    int w = pic->i_width;
    int h = pic->i_height;
    int wc = pic->i_widthc;
    int hc = pic->i_heightc;
    int pad = 64 + 8;
    int padc = 32 + 4;

    g_funs_handle.padding_rows(pic->p_y, pic->i_stride, w, h, start, rows, pad);
    g_funs_handle.padding_rows(pic->p_u, pic->i_stridec, wc, hc, start / 2, rows / 2, padc);
    g_funs_handle.padding_rows(pic->p_v, pic->i_stridec, wc, hc, start / 2, rows / 2, padc);
}

void com_pic_yuv_padding_rows_lr(com_pic_t *pic, int start, int rows, int cpy_flag[3])
{
    int w = pic->i_width;
    int h = pic->i_height;
    int wc = pic->i_widthc;
    int hc = pic->i_heightc;
    int pad = 64 + 8;
    int padc = 32 + 4;

    if (cpy_flag[0]) {
        g_funs_handle.padding_rows_lr(pic->p_y, pic->i_stride, w, h, start, rows, pad);
    }
    if (cpy_flag[1]) {
        g_funs_handle.padding_rows_lr(pic->p_u, pic->i_stridec, wc, hc, start / 2, rows / 2, padc);
    }
    if (cpy_flag[2]) {
        g_funs_handle.padding_rows_lr(pic->p_v, pic->i_stridec, wc, hc, start / 2, rows / 2, padc);
    }
}


static void cavs_log_default(int i_level, const char_t *psz_fmt, va_list arg)
{
    char_t *psz_prefix;

    switch (i_level) {
    case COM_LOG_ERROR:
        psz_prefix = (char_t *)"error";
        break;
    case COM_LOG_WARNING:
        psz_prefix = (char_t *)"warning";
        break;
    case COM_LOG_INFO:
        psz_prefix = (char_t *)"info";
        break;
    case COM_LOG_DEBUG:
        psz_prefix = (char_t *)"debug";
        break;
    default:
        psz_prefix = (char_t *)"unknown";
        break;
    }

    fprintf(stderr, "[%s]: ", (char *) psz_prefix);
    vfprintf(stderr, (const char *) psz_fmt, arg);

    if (i_level == COM_LOG_ERROR) {
        assert(0);
    }
}

void com_log(int i_level, const char_t *psz_fmt, ...)
{
    if (i_level <= cst_log_level) {
        va_list arg;
        va_start(arg, psz_fmt);
        cavs_log_default(i_level, psz_fmt, arg);
        va_end(arg);
    }
}

void check_ref_avaliable(com_frm_t *frm, int line_num)
{
    if (frm->linend < line_num) {
        avs2_pthread_mutex_lock(&frm->mutex);
        while (frm->linend < line_num) {
            avs2_pthread_cond_wait(&frm->cond, &frm->mutex);
        }
        avs2_pthread_mutex_unlock(&frm->mutex);
    }
}
