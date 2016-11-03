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

#ifndef _MACROBLOCK_H_
#define _MACROBLOCK_H_

tab_char_t tab_qp_scale_cr[64] = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 42, 43, 43, 44, 44, 45, 45,
    46, 46, 47, 47, 48, 48, 48, 49, 49, 49,
    50, 50, 50, 51,
};

tab_char_t tab_ncbp[64] = {                              
    63, 15, 31, 47,  0, 14, 13, 11,  7,  5,
    10,  8, 12, 61,  4, 55,  1,  2, 59,  3,
    62,  9,  6, 29, 45, 51, 23, 39, 27, 46,
    53, 30, 43, 37, 60, 16, 21, 28, 19, 35,
    42, 26, 44, 32, 58, 24, 20, 17, 18, 48,
    22, 33, 25, 49, 40, 36, 34, 50, 52, 54,
    41, 56, 38, 57
};

//! used to control block sizes : Not used/16x16/16x8/8x16/8x8/8x4/4x8/4x4

tab_char_t tab_block_step[9][2] = {
    {2, 2}, {2, 2}, {2, 1}, {1, 2}, {2, 1}, {2, 1}, {1, 2}, {1, 2}, {1, 1}
};

void read_coeffs(avs2_dec_t *h_dec);
#endif


