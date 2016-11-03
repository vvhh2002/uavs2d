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

#include "block_info.h"
#include "assert.h"

tab_char_t tab_log2[65] = {
    -1,
    0, 1, -1, 2, -1, -1, -1, 3,
    -1, -1, -1, -1, -1, -1, -1, 4,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, 5,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, 6
};

/////////////////////////////////////////////////////////////////////////////
/// variables definition
/////////////////////////////////////////////////////////////////////////////
tab_i32s_t tab_b8_xy_cvto_zigzag[8][8] = {
    {  0,  1,  4,  5, 16, 17, 20, 21 },
    {  2,  3,  6,  7, 18, 19, 22, 23 },
    {  8,  9, 12, 13, 24, 25, 28, 29 },
    { 10, 11, 14, 15, 26, 27, 30, 31 },
    { 32, 33, 36, 37, 48, 49, 52, 53 },
    { 34, 35, 38, 39, 50, 51, 54, 55 },
    { 40, 41, 44, 45, 56, 57, 60, 61 },
    { 42, 43, 46, 47, 58, 59, 62, 63 }
};

/* block size table, based on 8x8 cu */
tab_i32s_t tab_blk_size[10][4] = {
    { 8, 8, 8, 8 }, // 0. SKIPDIRECT
    { 8, 8, 8, 8 }, // 1. P2NX2N
    { 8, 4, 8, 4 }, // 2. P2NXN
    { 4, 8, 4, 8 }, // 3. PNX2N
    { 8, 2, 8, 6 }, // 4. PHOR_UP
    { 8, 6, 8, 2 }, // 5. PHOR_DOWN
    { 2, 8, 6, 8 }, // 6. PVER_LEFT
    { 6, 8, 2, 8 }, // 7. PVER_RIGHT
    { 4, 4, 4, 4 }, // 8. 
    { 4, 4, 4, 4 }  // 9. I8MB
};

/* PU pixel's info, based on 8x8 cu */
tab_i32s_t tab_pu_pix_info[10][4] = {
/* 1st_blk_size   2nd_blk_start */
    { 4, 4,            4, 4},    // 0. SKIPDIRECT
    { 8, 8,            0, 0},    // 1. P2NX2N
    { 8, 4,            0, 4},    // 2. P2NXN
    { 4, 8,            4, 0},    // 3. PNX2N
    { 8, 2,            0, 2},    // 4. PHOR_UP
    { 8, 6,            0, 6},    // 5. PHOR_DOWN
    { 2, 8,            2, 0},    // 6. PVER_LEFT
    { 6, 8,            6, 0},    // 7. PVER_RIGHT
    { 4, 4,            4, 4},    // 8. 
    { 4, 4,            4, 4}     // 9. I8MB
};

tab_i32s_t tab_pu_blk_num[10] = { 4, 1, 2, 2, 2, 2, 2, 2, 4, 4 };

tab_i32s_t tab_pu_2_blk_idx[10][4] = {
    { 0, 1, 2, 3 },    // 0. SKIPDIRECT
    { 0, 0, 0, 0 },    // 1. P2NX2N
    { 0, 2, 0, 0 },    // 2. P2NXN
    { 0, 1, 0, 0 },    // 3. PNX2N
    { 0, 2, 0, 0 },    // 4. PHOR_UP
    { 0, 2, 0, 0 },    // 5. PHOR_DOWN
    { 0, 1, 0, 0 },    // 6. PVER_LEFT
    { 0, 1, 0, 0 },    // 7. PVER_RIGHT
    { 0, 1, 2, 3 },    // 8. 
    { 0, 1, 2, 3 }     // 9. I8MB
};


tab_i32s_t tab_Left_Down_Avail_Matrix64[16][16] = {
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

tab_i32s_t tab_Up_Right_Avail_Matrix64[16][16] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
    { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 },
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 }
};

/////////////////////////////////////////////////////////////////////////////
/// function definition
/////////////////////////////////////////////////////////////////////////////
void get_sub_blk_offset(int blocktype, int uiBitSize, int i, int j, int *start_x, int *start_y, int *width, int *height)
{
    if (uiBitSize == MIN_CU_SIZE_IN_BIT || (blocktype < 4 || blocktype > 7)) {
        *start_x = i << (uiBitSize - 3);
        *start_y = j << (uiBitSize - 3);
        *width   = 1 << (uiBitSize - 3);
        *height  = 1 << (uiBitSize - 3);
    } else {
        int k = j * 2 + i;
        const i32s_t* size_tab = tab_blk_size[blocktype];
        int b_hor = (blocktype == 4 || blocktype == 5);
        int h0 = b_hor ? (size_tab[0] << (uiBitSize - 3)) >> 3 : (size_tab[0] << (uiBitSize - 3)) >> 2;
        int v0 = b_hor ? (size_tab[1] << (uiBitSize - 3)) >> 2 : (size_tab[1] << (uiBitSize - 3)) >> 3;
        int h1 = b_hor ? (size_tab[2] << (uiBitSize - 3)) >> 3 : (size_tab[2] << (uiBitSize - 3)) >> 2;
        int v1 = b_hor ? (size_tab[3] << (uiBitSize - 3)) >> 2 : (size_tab[3] << (uiBitSize - 3)) >> 3;

        switch (k) {
        case 0:
            *start_x = 0;
            *start_y = 0;
            *width = h0;
            *height = v0;
            break;
        case 1:
            *start_x = h0;
            *start_y = 0;
            *width = h1;
            *height = v0;
            break;
        case 2:
            *start_x = 0;
            *start_y = v0;
            *width = h0;
            *height = v1;
            break;
        case 3:
            *start_x = h0;
            *start_y = v0;
            *width = h1;
            *height = v1;
            break;
        }
    }
}

void get_pu_pix_info(int type, int cu_bitsize, int blk, int *startx, int *starty, int *width, int *height)
{
    int w_1st = tab_pu_pix_info[type][0] << (cu_bitsize - 3);
    int h_1st = tab_pu_pix_info[type][1] << (cu_bitsize - 3);

    if (w_1st == h_1st) {
        *startx = (blk  & 1) * w_1st;
        *starty = (blk >> 1) * h_1st;
        *width  = w_1st;
        *height = h_1st;
    }  else if (blk == 0) { 
        *startx = 0;
        *starty = 0;
        *width  = w_1st;
        *height = h_1st;
    } else {
        int cu_size = 1 << cu_bitsize;
        *startx = tab_pu_pix_info[type][2] << (cu_bitsize - 3);
        *starty = tab_pu_pix_info[type][3] << (cu_bitsize - 3);
        *width  = cu_size - *startx;
        *height = cu_size - *starty;
    }
}

/*!
************************************************************************
* \brief
*    returns the x and y sample coordinates for a given MbAddress
************************************************************************
*/
void get_lcu_pos(const seq_info_t *seq, int mb_addr, int *x, int *y)
{
    int uiBitSize = seq->g_uiMaxSizeInBit;
    int SizeScale = 1 << (uiBitSize - MIN_CU_SIZE_IN_BIT);
    *x = (mb_addr % (seq->img_width_in_mcu)) / SizeScale;
    *y = (mb_addr / (seq->img_width_in_mcu)) / SizeScale;

    (*x) *= 8 * SizeScale;
    (*y) *= 8 * SizeScale;
}

int getDeltas(int b_top, int *delt, int *delt2, i64s_t OriPOC, i64s_t OriRefPOC, i64s_t ScaledPOC, i64s_t ScaledRefPOC)
{
    *delt = 0;
    *delt2 = 0;

    OriPOC       = COM_ADD_MODE(OriPOC      , 512);
    OriRefPOC    = COM_ADD_MODE(OriRefPOC   , 512);
    ScaledPOC    = COM_ADD_MODE(ScaledPOC   , 512);
    ScaledRefPOC = COM_ADD_MODE(ScaledRefPOC, 512);

    assert((OriPOC % 2) + (OriRefPOC % 2) + (ScaledPOC % 2) + (ScaledRefPOC % 2) == 0);

    OriPOC       /= 2;
    OriRefPOC    /= 2;
    ScaledPOC    /= 2;
    ScaledRefPOC /= 2;

#define OddEvenDIFF(a, b) (((a)^(b)) & 1)

    if (b_top) {  //Scaled is top field
        *delt2 = OddEvenDIFF(ScaledRefPOC, ScaledPOC) ? 2 : 0;

        if (!OddEvenDIFF(ScaledPOC, OriPOC)) { //ori is top
            *delt = OddEvenDIFF(OriRefPOC, OriPOC) ?  2 : 0;
        } else {
            *delt = OddEvenDIFF(OriRefPOC, OriPOC) ? -2 : 0;
        }
    } else { //Scaled is bottom field
        *delt2 = OddEvenDIFF(ScaledRefPOC, ScaledPOC) ? -2 : 0;
        if (!OddEvenDIFF(ScaledPOC, OriPOC)) { //ori is bottom
            *delt = OddEvenDIFF(OriRefPOC, OriPOC) ? -2 : 0;
        } else {
            *delt = OddEvenDIFF(OriRefPOC, OriPOC) ? 2 : 0;
        }
    }

#undef OddEvenDIFF

    return 0;
}