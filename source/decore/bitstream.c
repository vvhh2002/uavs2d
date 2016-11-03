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

#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

#include "bitstream.h"

int get_vlc_symbol(bs_stream_t *bs, int* read_len)
{

    int val, val_bit;
    int ctr_bit; 
    int len = 0;
    int info_bit;
    int bit_cnt;

    uchar_t *start = bs->p_start_test;
    uchar_t *end = bs->p_end;
    int bits_to_go = bs->bits_to_go;

    ctr_bit = ((*start) & (0x01 << bits_to_go));    

    while (ctr_bit == 0) {
        len++;
        if (--bits_to_go < 0) {
            bits_to_go = 7;
            if ((++start) == end) { // error
                assert(0);
            }
        }
        ctr_bit = ((*start) & (0x01 << bits_to_go));
    }

    // make infoword
    val = 0; 
    bit_cnt = len + 1;

    for (info_bit = 0; (info_bit < len); info_bit++) {
        bit_cnt++;
        if (--bits_to_go < 0) {
            bits_to_go = 7;
            if ((++start) == end) { // error
                assert(0);
            }
        }

        val_bit = ((*start) >> bits_to_go) & 0x1;
        val = (val << 1) | val_bit;
    }

    /* __zywang__: is a bug ? */

    if (--bits_to_go < 0) {
        bits_to_go = 7;
        if ((++start) == end) { // error
            assert(0);
        }
    }

    bs->p_start_test = start;
    bs->bits_to_go = bits_to_go;

    *read_len = bit_cnt;



    return val;         
}

int ue_v(bs_stream_t *bs)
{
    int len;
    int val = get_vlc_symbol(bs, &len);
    return ((int)pow(2, (len / 2))) + val - 1;
}

int se_v(bs_stream_t *bs)
{
    int n, len;
    int val = get_vlc_symbol(bs, &len);

    n = (int)pow(2, (len / 2)) + val - 1;
    val = (n + 1) / 2;

    if ((n & 0x01) == 0) {
        val = -val;
    }

    return val;
}

int u_v(bs_stream_t *bs, int bits)
{
    int val = 0;
    int bit_val;

    assert(bs->p_start_test < bs->p_end);

    while (bits > 0) {
        bits--;

        bit_val = ((*bs->p_start_test) >> bs->bits_to_go) & 0x1;
        val = (val << 1) | bit_val;
        if (--bs->bits_to_go < 0) {
            bs->bits_to_go = 7;
            bs->p_start_test++;
            assert(bs->p_start_test < bs->p_end || bits == 0);
        }
    }

    return val;
}

int bs_one_unit_init(bs_stream_t *bs, uchar_t *start, uchar_t *end)
{
    int len = (int)(end - start);

    /* start of one unit */
    while (len >= 4 && (*(i32u_t *)start & 0x00FFFFFF) != 0x00010000) {
        start++;
        len--;
    }

    if (len < 4) {
        return 0;
    }

    bs->bs_data = start;
    bs->p_start_test = start + 4;

    /* end of one unit */
    start++;
    len--;
    while (len >= 4 && (*(i32u_t *)start & 0x00FFFFFF) != 0x00010000) {
        start++;
        len--;
    }

    if (len < 4) {
        bs->p_end = end;
    }
    else {
        bs->p_end = start;
    }

    bs->bits_to_go = 7;
    return 1;
}