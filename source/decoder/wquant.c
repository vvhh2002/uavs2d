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

#include "wquant.h"
#include "commonStructures.h"

tab_i32s_t tab_wq_param_default[2][6] = {
    { 67, 71, 71, 80, 80, 106 },
    { 64, 49, 53, 58, 58,  64 }
};

tab_i16s_t tab_WqMDefault4x4[16] = {
    64, 64, 64, 68,
    64, 64, 68, 72,
    64, 68, 76, 80,
    72, 76, 84, 96
};

tab_i16s_t tab_WqMDefault8x8[64] = {
     64,  64,  64,  64,  68,  68,  72,  76,
     64,  64,  64,  68,  72,  76,  84,  92,
     64,  64,  68,  72,  76,  80,  88, 100,
     64,  68,  72,  80,  84,  92, 100, 112,
     68,  72,  80,  84,  92, 104, 112, 128,
     76,  80,  84,  92, 104, 116, 132, 152,
     96, 100, 104, 116, 124, 140, 164, 188,
    104, 108, 116, 128, 152, 172, 192, 216
};

static tab_uchar_t tab_WeightQuantModel[4][64] = {
    //   l a b c d h
    //   0 1 2 3 4 5
    {
        // Mode 0
        0, 0, 0, 4, 4, 4, 5, 5,
        0, 0, 3, 3, 3, 3, 5, 5,
        0, 3, 2, 2, 1, 1, 5, 5,
        4, 3, 2, 2, 1, 5, 5, 5,
        4, 3, 1, 1, 5, 5, 5, 5,
        4, 3, 1, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5
    }, {
        // Mode 1
        0, 0, 0, 4, 4, 4, 5, 5,
        0, 0, 4, 4, 4, 4, 5, 5,
        0, 3, 2, 2, 2, 1, 5, 5,
        3, 3, 2, 2, 1, 5, 5, 5,
        3, 3, 2, 1, 5, 5, 5, 5,
        3, 3, 1, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5
    }, {
        // Mode 2
        0, 0, 0, 4, 4, 3, 5, 5,
        0, 0, 4, 4, 3, 2, 5, 5,
        0, 4, 4, 3, 2, 1, 5, 5,
        4, 4, 3, 2, 1, 5, 5, 5,
        4, 3, 2, 1, 5, 5, 5, 5,
        3, 2, 1, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5
    }, {
        // Mode 3
        0, 0, 0, 3, 2, 1, 5, 5,
        0, 0, 4, 3, 2, 1, 5, 5,
        0, 4, 4, 3, 2, 1, 5, 5,
        3, 3, 3, 3, 2, 5, 5, 5,
        2, 2, 2, 2, 5, 5, 5, 5,
        1, 1, 1, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5
    }
};

static tab_uchar_t tab_WeightQuantModel4x4[4][16] = {
    //   l a b c d h
    //   0 1 2 3 4 5
    {
        // Mode 0
        0, 4, 3, 5,
        4, 2, 1, 5,
        3, 1, 1, 5,
        5, 5, 5, 5
    }, {
        // Mode 1
        0, 4, 4, 5,
        3, 2, 2, 5,
        3, 2, 1, 5,
        5, 5, 5, 5
    }, {
     
        // Mode 2
        0, 4, 3, 5,
        4, 3, 2, 5,
        3, 2, 1, 5,
        5, 5, 5, 5
    }, {
        // Mode 3
        0, 3, 1, 5,
        3, 4, 2, 5,
        1, 2, 2, 5,
        5, 5, 5, 5
    }
};

void drive_pic_wq_matrix_param(i32s_t *param_vector, int mode, i16s_t pic_wq_matrix[2][64])
{
    int i;
    for (i = 0; i < 16; i++) {
        pic_wq_matrix[0][i] = (i16s_t) param_vector[tab_WeightQuantModel4x4[mode][i]];
    }
    for (i = 0; i < 64; i++) {
        pic_wq_matrix[1][i] = (i16s_t) param_vector[tab_WeightQuantModel[mode][i]];
    }
}

void cal_all_size_wq_matrix(i16s_t *cur_wq_matrix[4], i16s_t *pic_wq_matrix)
{
    int i, j;

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            cur_wq_matrix[2][(i << 4) + j] = pic_wq_matrix[((i >> 1) & 7) * 8 + ((j >> 1) & 7)];
        }
    }
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            cur_wq_matrix[3][(i << 5) + j] = pic_wq_matrix[((i >> 2) & 7) * 8 + ((j >> 2) & 7)];
        }
    }
}