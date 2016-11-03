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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>

#include "defines.h"
#include "commonVariables.h"
#include "transform.h"
#include "block_info.h"
#include "intra-prediction.h"
#include "inter-prediction.h"

#include "global.h"
#include "block.h"
#include "bitstream.h"
#include "AEC.h"

static tab_i32s_t tab_dmh_pos[DMH_MODE_NUM + DMH_MODE_NUM - 1][2][2] = {
    { { 0, 0 }, { 0, 0 } },
    { { -1, 0 }, { 1, 0 } },
    { { 0, -1 }, { 0, 1 } },
    { { -1, 1 }, { 1, -1 } },
    { { -1, -1 }, { 1, 1 } },
    { { -2, 0 }, { 2, 0 } },
    { { 0, -2 }, { 0, 2 } },
    { { -2, 2 }, { 2, -2 } },
    { { -2, -2 }, { 2, 2 } }
};

void dec_add_idct(com_rec_t *rec, int block8x8, int qp, coef_t *blk, int uiBitSize)
{
    int  b8_y = (block8x8 / 2) << uiBitSize;
    int  b8_x = (block8x8 % 2) << uiBitSize;
    int Size = 1 << uiBitSize;
    pel_t *dst;
    int i_dst;
    int max_pix_val = (1 << (rec->seq->sample_bit_depth)) - 1;
    com_pic_t *pic = rec->frm_cur->yuv_data;
    
    if (block8x8 <= 3) {
        i_dst  = pic->i_stride;
        dst = pic->p_y + (rec->cu_pix_y + b8_y) * i_dst + rec->cu_pix_x + b8_x;
    } else {
        i_dst  = pic->i_stridec;
        dst = (block8x8 == 4 ? pic->p_u : pic->p_v) + (rec->cu_pix_c_y) * i_dst + rec->cu_pix_c_x;
    }

    if (rec->cu->cbp & (1 << block8x8)) {
        core_inv_trans(rec, blk, uiBitSize, block8x8);
        g_funs_handle.add_pel_clip(dst, i_dst, blk, Size, dst, i_dst, Size, Size, rec->seq->sample_bit_depth);
    }
}

void dec_add_idct_nsqt(com_rec_t *rec, int block8x8, coef_t *blk, int uiBitSize)
{
    int  iSizeX, iSizeY;
    int  iStartX, iStartY;
    int cuType = rec->cu->cuType;
    pel_t *dst;
    int i_dst;
    int max_pix_val = (1 << (rec->seq->sample_bit_depth)) - 1;
    com_pic_t *pic = rec->frm_cur->yuv_data;

    if (cuType == P2NXN || cuType == PHOR_UP || cuType == PHOR_DOWN || cuType == InNxNMB) {
        iStartX = 0;
        iStartY = block8x8 * (1 << (uiBitSize - 1));
        iSizeX = (1 << (uiBitSize + 1));
        iSizeY = (1 << (uiBitSize - 1));
    } else {
        iStartX = block8x8 * (1 << (uiBitSize - 1));
        iStartY = 0;
        iSizeX = (1 << (uiBitSize - 1));
        iSizeY = (1 << (uiBitSize + 1));
    }

    if (block8x8 <= 3) {
        i_dst  = pic->i_stride;
        dst = pic->p_y + (rec->cu_pix_y + iStartY) * i_dst + rec->cu_pix_x + iStartX;
    } else {
        i_dst = pic->i_stridec;
        dst = (block8x8 == 4 ? pic->p_u : pic->p_v) + (rec->cu_pix_c_y) * i_dst + rec->cu_pix_c_x;
    } 

    if (rec->cu->cbp & (1 << block8x8)) {
        core_inv_trans_nsqt(rec, blk, uiBitSize, block8x8);
        g_funs_handle.add_pel_clip(dst, i_dst, blk, iSizeX, dst, i_dst, iSizeX, iSizeY, rec->seq->sample_bit_depth);
    }
}

/*
*************************************************************************
* Function:Make Intra prediction for all 5 modes for 8*8 blocks.
bs_x and bs_y may be only 4 and 8.
img_x and img_y are pixels offsets in the picture.
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
void dec_intra_pred(com_rec_t *rec, int img_x, int img_y, int uiBitSize)    //need to fix !!!
{
    pel_t edgepixels[1024];
    pel_t *edge = edgepixels + ((1 << (uiBitSize + 1)) * 2) + 4;
    pel_t* edgepixels_trans_y = edge + 144;
    int x, y;
    int i, predmode;
    int block_available_up = 0, block_available_up_right = 0;
    int block_available_left = 0, block_available_left_down = 0;
    int bs_x, bs_y;
    int b4x, b4y;
    com_cu_t *cu = rec->cu;
    int cuType = cu->cuType;
    
    int mb_left_available = 0;
    int mb_up_available = 0;
    int mb_up_right_available = 0;
    int mb_left_down_available = 0;
    int mb_up_left_available = 0;    
    int block_available_up_left = 0;
    int N8_SizeScale;
    int leftdown;
    int upright;
    int pix_smb_x;
    int pix_smb_y;
    int  uiBitSize1 = cuType == INxnNMB ? uiBitSize - 2  : uiBitSize;
    int  uiBitSize2 = cuType == InNxNMB ? uiBitSize - 2 : uiBitSize;
    int NumMBWidthInBlk1 = 1 << (uiBitSize1 - MIN_BLOCK_SIZE_IN_BIT);
    int NumMBWidthInBlk2 = 1 << (uiBitSize2 - MIN_BLOCK_SIZE_IN_BIT);
    com_pic_t *pic = rec->frm_cur->yuv_data;
    pel_t *psrcT, *psrcL, *psrcTL;
    pel_t *dst;
    int i_src, i_dst;
    
    const seq_info_t *seq = rec->seq;
    int cu_idx = rec->cu_idx;
    int defalt_val = 1 << (seq->sample_bit_depth - 1);
    int MBRowSize = seq->img_width_in_mcu;

    i_dst = rec->frm_cur->yuv_data->i_stride;

    N8_SizeScale = 1 << (uiBitSize - MIN_BLOCK_SIZE_IN_BIT);
    if (cuType == InNxNMB) { //add yuqh 20130825
        mb_left_available = (img_x >= (1 << (uiBitSize + 1))) ? cu->slice_nr == rec->cu_array[cu_idx % MBRowSize < 1 ? cu_idx : cu_idx - 1].slice_nr : 0; /*oliver*/
        mb_up_available = (img_y >= (1 << (uiBitSize - 1))) ? cu->slice_nr == rec->cu_array[cu_idx > MBRowSize ? cu_idx - MBRowSize : cu_idx].slice_nr : 0; /*oliver*/
    } else if (cuType == INxnNMB) {
        mb_left_available = (img_x >= (1 << (uiBitSize - 1))) ? cu->slice_nr == rec->cu_array[cu_idx % MBRowSize < 1 ? cu_idx : cu_idx - 1].slice_nr : 0; /*oliver*/
        mb_up_available = (img_y >= (1 << (uiBitSize + 1))) ? cu->slice_nr == rec->cu_array[cu_idx > MBRowSize ? cu_idx - MBRowSize : cu_idx].slice_nr : 0; /*oliver*/
    } else {
        mb_left_available = (img_x >= (8 << (uiBitSize - MIN_BLOCK_SIZE_IN_BIT))) ? cu->slice_nr == cu[ - 1].slice_nr : 0;       /*oliver*/
        mb_up_available = (img_y >= (uiBitSize == seq->g_uiMaxSizeInBit ? (1 << uiBitSize) : (8 << (uiBitSize - MIN_BLOCK_SIZE_IN_BIT)))) ? cu->slice_nr == cu[- MBRowSize].slice_nr : 0;    /*oliver*/
    }

    get_lcu_pos(rec->seq, cu_idx, &pix_smb_x, &pix_smb_y);

    if (!seq->slice_set_enable) { //added by mz, 2008.04
        if (img_y == 0 || (img_x + (1 << (uiBitSize1 + 1)) > seq->img_width)) {
            mb_up_right_available = 0;
        } else if ((img_y - pix_smb_y) > 0) {
            mb_up_right_available = 1;
        } else {
            mb_up_right_available = (cu->slice_nr == cu[- MBRowSize + (NumMBWidthInBlk1 >> 1)].slice_nr);
        }
        if (img_x == 0 || (img_y + (1 << (uiBitSize2 + 1)) > seq->img_height)) {
            mb_left_down_available = 0;
        } else if (img_x - pix_smb_x > 0) {
            mb_left_down_available = 1;
        } else {
            mb_left_down_available = (cu->slice_nr == cu[- 1 + MBRowSize * (NumMBWidthInBlk2 >> 1)].slice_nr);
        }
    } else {  //added by mz, 2008.04
        mb_left_available = (img_x >= 8) ? (cu->slice_set_index == cu[- 1].slice_set_index) : 0;
        mb_up_available = (img_y >= 8) ? (cu->slice_set_index == cu[- MBRowSize].slice_set_index) : 0;

        if (img_y == 0 || (img_x + (1 << (uiBitSize1 + 1)) > seq->img_width)) {
            mb_up_right_available = 0;
        }
        else if ((img_y - rec->cu_pix_y) > 0) {
            mb_up_right_available = 1;
        } else {
            mb_up_right_available = (cu->slice_set_index == cu[- MBRowSize + (NumMBWidthInBlk1 >> 1)].slice_set_index);
        }

        if (img_x == 0 || (img_y + (1 << (uiBitSize2 + 1)) > seq->img_height)) {
            mb_left_down_available = 0;
        } else if (img_x - pix_smb_x > 0) {
            mb_left_down_available = 1;
        } else {
            mb_left_down_available = (cu->slice_set_index == cu[- 1 + MBRowSize * (NumMBWidthInBlk2 >> 1)].slice_set_index);
        }

        if ((rec->cu_b8_y == 0) || (rec->cu_b8_x == 0)) {
            mb_up_left_available = 0;
        } else if ((img_y - pix_smb_y) > 0) {
            mb_up_left_available = (img_x - pix_smb_x) > 0 ? 1 : (cu->slice_set_index == cu[- 1].slice_set_index);
        } else {
            mb_up_left_available = ((img_x - pix_smb_x) > 0 ? (cu->slice_set_index == cu[-MBRowSize].slice_set_index) : (cu->slice_set_index == cu[- MBRowSize - 1].slice_set_index));
        }
    }


    b4x = img_x >> MIN_BLOCK_SIZE_IN_BIT; //qyu 0903
    b4y = img_y >> MIN_BLOCK_SIZE_IN_BIT; //qyu 0903
    if (cuType == InNxNMB || cuType == INxnNMB) { //add yuqh1020130825
        bs_x = cuType == InNxNMB ? (1 << uiBitSize)     : (1 << (uiBitSize - 2));
        bs_y = cuType == InNxNMB ? (1 << (uiBitSize - 2)) : (1 << uiBitSize);
    } else {
        bs_x =  bs_y = MIN_BLOCK_SIZE * N8_SizeScale;
    }

    predmode = rec->ipredmode[b4y * seq->b4_info_stride + b4x];

    assert(predmode >= 0);

    //check block up
    block_available_up = (b4y > 0 && (mb_up_available || ((uiBitSize < (cuType == InNxNMB ? (seq->g_uiMaxSizeInBit + 1) : seq->g_uiMaxSizeInBit)) && (cuType == InNxNMB ? ((img_y / (1 << (uiBitSize - 2))) % 2)   : (img_y / (1 << (uiBitSize))  % 2)))));
    //check block left
    block_available_left = (b4x > 0 && (mb_left_available || (cuType == INxnNMB ? (img_x / (1 << (uiBitSize - 2))) % 2   : img_x / (1 << (uiBitSize))) % 2));

    //check block left down
    leftdown = tab_Left_Down_Avail_Matrix64[(img_y >> 2) - (pix_smb_y >> 2) + bs_y / 4 - 1][(img_x >> 2) - (pix_smb_x >> 2)];

    upright = tab_Up_Right_Avail_Matrix64[(img_y >> 2) - (pix_smb_y >> 2)][(img_x >> 2) - (pix_smb_x >> 2) + bs_x / 4 - +1];

    if (img_y + bs_y >= rec->lcu_pix_y + rec->lcu_height) {
        leftdown = 0;
    }

    if (img_x + bs_x >= seq->img_width) {
        upright = 0;
    }

    block_available_left_down = (leftdown && mb_left_down_available); 
    block_available_up_right = (upright && mb_up_right_available);
    block_available_up_left = (b4y > 0 && b4x > 0 && mb_up_left_available);
    rec->block_available_up = block_available_up;
    rec->block_available_left = block_available_left;

    i_src = pic->i_stride;

    if (img_y == pix_smb_y) {
        psrcT = rec->filter_bak_rec[0] + img_x;
    } else {
        psrcT = pic->p_y + (img_y - 1) * i_src + img_x;
    }

    //get prediciton pixels
    if (block_available_up) {
        dst = &edge[1];
        for (x = 0; x < bs_x; x++) {
            dst[x] = psrcT[x];
        }

        dst = &edge[1 + bs_x];
        if (block_available_up_right) {
            int tbs = min(bs_x, seq->img_width - img_x - bs_x);
            
            for (x = 0; x < tbs; x++) {
                dst[x] = psrcT[bs_x + x];
            }
            for (; x < bs_x; x++) {
                dst[x] = psrcT[seq->img_width - 1 - img_x];
            }
        } else {
            for (x = 0; x < bs_x; x++) {
                dst[x] = edge[bs_x];
            }
        }

        if (block_available_left) {
            edge[0] = psrcT[-1];
        } else {
            edge[0] = psrcT[0];
        }
    } else {
        for (i = 0; i <= 2 * bs_x; i++) {
            edge[i] = defalt_val;
        }
    }

    /* padding */
    dst = &edge[1 + bs_x];
    dst[bs_x] = dst[bs_x + 1] = dst[bs_x + 2] = dst[bs_x + 3] = dst[bs_x - 1];

    psrcL = pic->p_y + img_y * i_src + img_x - 1;

    if (block_available_left) {
        if (!block_available_up) {
            edge[0] =psrcL[0];
        }

        dst = &edge[-1];
        for (y = 0; y < bs_y; y++) {
            dst[- y] = psrcL[0];
            psrcL += i_src;
        }

        dst = &edge[-1 - bs_y];
        if (block_available_left_down) {
            int tby = min(bs_y, seq->img_height - img_y - bs_y);
            for (y = 0; y < tby; y++) {
                dst[-y] = psrcL[0];
                psrcL += i_src;
            }
            for (; y < bs_y; y++) {
                dst[-y] = psrcL[0];
            }
        } else {
            for (y = 0; y < bs_y; y++) {
                dst[-y] = edge[-bs_y];
            }
        }
    } else {
        for (i = -2 * bs_y; i < 0; i++) {
            edge[i] = defalt_val;
        }
    }

    /* padding */
    dst = &edge[-1 - bs_y];
    dst[-bs_y] = dst[-bs_y - 1] = dst[-bs_y - 2] = dst[-bs_y - 3] = dst[-bs_y + 1];

    psrcTL = psrcT - 1;

    if (!seq->slice_set_enable) {
        if (block_available_up && block_available_left) {
            edge[0] = psrcTL[0];
        }
    } else {
        if (block_available_up_left) {
            edge[0] = psrcTL[0];
        } else if (block_available_up) {
            edge[0] = psrcTL[1];
        } else if (block_available_left) {
            edge[0] = psrcTL[i_src];
        }
    }

    dst = rec->frm_cur->yuv_data->p_y + img_y * i_dst + img_x;

    for (y = 0; y < 2 * bs_y + 4; y++) {
        edgepixels_trans_y[y] = edge[-y];
    }

    core_intra_get_luma(edge, dst, i_dst, predmode, uiBitSize, block_available_up, block_available_left, bs_y, bs_x, seq->sample_bit_depth);
}

void dec_intra_pred_chroma(com_rec_t *rec, int uv)
{
    com_cu_t *cu = rec->cu;
    pel_t edgepixels[512 + 80];
    pel_t *edge = edgepixels + (1 << rec->cu_bitsize) * 2 + 4;
    pel_t* edgepixels_trans_y = edge + 144;
    const seq_info_t* seq = rec->seq;
    int x, y;
    int bs_x;
    int bs_y;
    int i = 0, j = 0;

    int img_x = (rec->cu_b8_x << MIN_CU_SIZE_IN_BIT);
    int img_y = (rec->cu_b8_y << MIN_CU_SIZE_IN_BIT);

    int predLmode = rec->ipredmode[(img_y / MIN_BLOCK_SIZE) * seq->b4_info_stride + img_x / MIN_BLOCK_SIZE];
    int mb_available_left_down;
    int leftdown;
    int mb_nr = rec->cu_idx;//GBimg->cu_idx;
    int mb_available_up;
    int mb_available_left;
    int  mb_available_up_left;
    int mb_available_up_right;
    int cu_bitsize = rec->cu_bitsize;
    int N8_SizeScale;
    int HalfPixWidth;
    int upright;
    int mb_x, mb_y;
    com_pic_t *pic = rec->frm_cur->yuv_data;
    pel_t *psrcT, *psrcL, *psrcTL;
    pel_t *dst;
    int i_src, i_dst;
    int default_val = 1 << (seq->sample_bit_depth - 1);

    i_dst = rec->frm_cur->yuv_data->i_stridec;

    get_lcu_pos(seq, rec->cu_idx, &mb_x, &mb_y);
    N8_SizeScale = (1 << cu_bitsize) >> MIN_CU_SIZE_IN_BIT;

    bs_x = MIN_BLOCK_SIZE * N8_SizeScale;
    bs_y = MIN_BLOCK_SIZE * N8_SizeScale;


    if (seq->slice_set_enable) { //added by mz, 2008.04
        mb_available_up_right = ((rec->cu_b8_y == 0) || (rec->cu_b8_x > seq->img_width / 8 - N8_SizeScale * 2)) ? 0 : (cu->slice_set_index == cu[- seq->img_width_in_mcu + N8_SizeScale].slice_set_index);
        mb_available_left_down = ((rec->cu_b8_x == 0) || (rec->cu_b8_y > seq->img_height / 8 - N8_SizeScale * 2)) ? 0 : (cu->slice_set_index == cu[ - 1 + seq->img_width_in_mcu * N8_SizeScale].slice_set_index);
        mb_available_up = (rec->cu_b8_y == 0) ? 0 : (cu->slice_set_index == cu[ - seq->img_width_in_mcu].slice_set_index);
        mb_available_left = (rec->cu_b8_x == 0) ? 0 : (cu->slice_set_index == cu[- 1].slice_set_index);
        mb_available_up_left = (rec->cu_b8_x == 0 || rec->cu_b8_y == 0) ? 0 : (cu->slice_set_index == cu[- seq->img_width_in_mcu - 1].slice_set_index);
    }
    else {
        if ((rec->cu_b4_y == 0) || ((rec->cu_b4_x << MIN_BLOCK_SIZE_IN_BIT) + (1 << (cu_bitsize + 1)) > seq->img_width)) {
            mb_available_up_right = 0;
        }
        else if ((rec->cu_b4_y * MIN_BLOCK_SIZE - mb_y) > 0) {
            mb_available_up_right = 1;
        }
        else {
            mb_available_up_right = (cu->slice_nr == cu[- seq->img_width / 8 + N8_SizeScale].slice_nr);
        }

        if ((rec->cu_b4_x == 0) || ((rec->cu_b4_y << MIN_BLOCK_SIZE_IN_BIT) + (1 << (cu_bitsize + 1)) > seq->img_height)) {
            mb_available_left_down = 0;
        }
        else if ((rec->cu_b4_x * MIN_BLOCK_SIZE - mb_x) > 0) {
            mb_available_left_down = 1;
        }
        else {
            mb_available_left_down = (cu->slice_nr == cu[- 1 + seq->img_width / 8 * N8_SizeScale].slice_nr);
        }

        mb_available_up = (rec->cu_b8_y == 0) ? 0 : (cu->slice_nr == cu[-seq->img_width_in_mcu].slice_nr);
        mb_available_left = (rec->cu_b8_x == 0) ? 0 : (cu->slice_nr == cu[-1].slice_nr);
        mb_available_up_left = (rec->cu_b8_x == 0 || rec->cu_b8_y == 0) ? 0 : (cu->slice_nr == cu[-seq->img_width_in_mcu - 1].slice_nr);
    }
    HalfPixWidth = MIN_BLOCK_SIZE / 2 * N8_SizeScale;

    leftdown = tab_Left_Down_Avail_Matrix64[rec->cu_b4_y - mb_y / 4 + bs_y / 2 - 1][rec->cu_b4_x - mb_x / 4];

    upright = tab_Up_Right_Avail_Matrix64[rec->cu_b4_y - mb_y / 4][rec->cu_b4_x - mb_x / 4 + bs_x / 2 - 1];

    if (img_x + (1 << cu_bitsize) >= seq->img_width) {
        upright = 0;
    }

    mb_available_up_right = upright && mb_available_up_right;

    if (img_y + (1 << cu_bitsize) >= rec->lcu_pix_y + rec->lcu_height) {
        leftdown = 0;
    }

    mb_available_left_down = leftdown && mb_available_left_down;

    i_src = pic->i_stridec;

    if (rec->cu_b4_y * MIN_BLOCK_SIZE == mb_y) {
        psrcT = (uv ? rec->filter_bak_rec[2] : rec->filter_bak_rec[1]) + rec->cu_pix_c_x;
    } else {
        psrcT = (uv ? pic->p_v : pic->p_u) + (rec->cu_pix_c_y - 1) * i_src + rec->cu_pix_c_x;
    }
    
    if (mb_available_up) {
        dst = &edge[1];
        for (x = 0; x < bs_x; x++) {
            dst[x] = psrcT[x];
        }

        dst = &edge[1 + bs_x];
        if (mb_available_up_right) {
            int tbx = min(bs_x, seq->img_widthc - rec->cu_pix_c_x - bs_x);
            for (x = 0; x < tbx; x++) {
                dst[x] = psrcT[bs_x + x];
            }
            for(; x < bs_x; x++) {
                dst[x] = psrcT[seq->img_widthc - 1 - rec->cu_pix_c_x];
            }
        } else {
            for (x = 0; x < bs_x; x++) {
                dst[x] = edge[bs_x];
            }
        }

        if (mb_available_up_left) {
            edge[0] = psrcT[-1];
        } else {
            edge[0] = psrcT[0];
        }
    } else {
        for (i = 0; i <= 2 * bs_x; i++) {
            edge[i] = default_val;
        }
    }

    /* padding */
    dst = &edge[1 + bs_x * 2];
    dst[0] = dst[1] = dst[2] = dst[3] = dst[- 1];

    psrcL = (uv ? pic->p_v : pic->p_u) + rec->cu_pix_c_y * i_src + rec->cu_pix_c_x - 1;

    if (mb_available_left) {
        if (!mb_available_up) {
            edge[0] = psrcL[0];
        }

        dst = &edge[-1];
        for (y = 0; y < bs_y; y++) {
            dst[- y] = psrcL[0];
            psrcL += i_src;
        }

        dst = &edge[-1 - bs_y];
        if (mb_available_left_down) {
            int tby = min(bs_y, seq->img_heightc - rec->cu_pix_c_y - bs_y);
            for (y = 0; y < tby; y++) {
                dst[-y] = psrcL[0];
                psrcL += i_src;
            }
            for (; y < bs_y; y++) {
                dst[-y] = psrcL[0];
            }
        }
        else {
            for (y = 0; y < bs_y; y++) {
                dst[-y] = edge[-bs_y];
            }
        }
    } else {
        for (i = -2 * bs_y; i < 0; i++) {
            edge[i] = default_val;
        }
    }

    /* padding */
    dst = &edge[-1 - bs_y * 2];
    dst[0] = dst[-1] = dst[-2] = dst[-3] = dst[1];

    psrcTL = psrcT - 1;

    if (!seq->slice_set_enable) {
        if (mb_available_up && mb_available_left) {
            edge[0] = psrcTL[0];
        }
    } else {
        if (mb_available_up_left) {
            edge[0] = psrcTL[0];
        }
        else if (mb_available_up) {
            edge[0] = psrcTL[1];
        }
        else if (mb_available_left) {
            edge[0] = psrcTL[i_src];
        }
    }
    dst = uv ? rec->frm_cur->yuv_data->p_v : rec->frm_cur->yuv_data->p_u;
    dst += rec->cu_pix_c_y * i_dst + rec->cu_pix_c_x;

    for (y = 0; y < 2 * bs_y + 4; y++) {
        edgepixels_trans_y[y] = edge[-y];
    }

    core_intra_get_chroma(edge, dst, i_dst, cu->c_ipred_mode, cu_bitsize - 1, mb_available_up, mb_available_left, predLmode, seq->sample_bit_depth);
}

void dec_inter_pred(com_rec_t *rec)
{
    com_cu_t *cu = rec->cu;
    int cu_bitsize = rec->cu_bitsize;
    int cu_size = 1 << cu_bitsize;
    ALIGNED_32(pel_t tmp_blk[64 * 64]);
    ALIGNED_32(pel_t tmp_blkb[64 * 64]);
    const seq_info_t* seq = rec->seq;
    int i, pu_num;
    pel_t *dst;
    int check_ref = rec->check_ref;
    int b4y, b4x;
    int vec1_x = 0, vec1_y = 0, vec2_x = 0, vec2_y = 0;
    int ioff, joff;

    int pred_dir; 
    int mb_nr = rec->cu_idx;

    int start_x, start_y, width, height;
    int direct_change = 0;
    int delta[4];
    int i_b4 = rec->seq->b4_info_stride;
    com_frm_t *cur_frm = rec->frm_cur;
    int i_dst = cur_frm->yuv_data->i_stride;
    
    com_cu_local_t *local_cu = rec->cu_loc_dat;
    int md_directskip_mode = local_cu->md_directskip_mode;
    int weighted_skipmode  = local_cu->weighted_skipmode;
    char_t *b8pdir         = local_cu->b8pdir;

    for (i = 0; i < rec->i_refs; i++) {
        delta[i] = rec->ref_list[i].dist[0];
        if (rec->pichdr->background_reference_enable && i == rec->i_refs - 1) {
            delta[i] = 1;
        }
    }

    pu_num = tab_pu_blk_num[cu->cuType];

    if (!cu->cuType && (md_directskip_mode || cu_bitsize <= 4)) {
        direct_change = 1;
        pu_num = 1;
    }

    for (i = 0; i < pu_num; i++) {
        int blk = tab_pu_2_blk_idx[cu->cuType][i];
        get_pu_pix_info(cu->cuType, cu_bitsize, i, &start_x, &start_y, &width, &height);

        if (direct_change) {
            start_x <<= 1;
            start_y <<= 1;
            width <<= 1;
            height <<= 1;
        }

        b4x = rec->cu_b4_x + start_x / 4;
        b4y = rec->cu_b4_y + start_y / 4;

        ioff = start_x;
        joff = start_y;

        dst = cur_frm->yuv_data->p_y + (rec->cu_pix_y + joff) * i_dst + rec->cu_pix_x + ioff;

        pred_dir = b8pdir[blk];

        if (rec->type == B_IMG) {
            if (pred_dir != SYM && pred_dir != BID) {
                i16s_t(*mv_array)[2] = pred_dir ? rec->bw_mv : cur_frm->mvbuf;

                vec1_x = (rec->cu_pix_x + ioff) * 4 + mv_array[b4y * i_b4 + b4x][0];
                vec1_y = (rec->cu_pix_y + joff) * 4 + mv_array[b4y * i_b4 + b4x][1];

                if (!pred_dir) {
                    core_inter_get_luma(vec1_x, vec1_y, width, height, dst, i_dst, rec->ref_fwd->frm, seq->sample_bit_depth, check_ref);
                } else {
                    core_inter_get_luma(vec1_x, vec1_y, width, height, dst, i_dst, rec->ref_bwd->frm, seq->sample_bit_depth, check_ref);
                }
            } else {
                i16s_t(*fw_mv_array)[2] = cur_frm->mvbuf;
                i16s_t(*bw_mv_array)[2] = rec->bw_mv;

                vec1_x = (rec->cu_pix_x + start_x) * 4 + fw_mv_array[b4y * i_b4 + b4x][0];
                vec1_y = (rec->cu_pix_y + start_y) * 4 + fw_mv_array[b4y * i_b4 + b4x][1];

                vec2_x = (rec->cu_pix_x + start_x) * 4 + bw_mv_array[b4y * i_b4 + b4x][0];
                vec2_y = (rec->cu_pix_y + start_y) * 4 + bw_mv_array[b4y * i_b4 + b4x][1];

                core_inter_get_luma(vec1_x, vec1_y, width, height, tmp_blk, 64, rec->ref_fwd->frm, seq->sample_bit_depth, check_ref);
                core_inter_get_luma(vec2_x, vec2_y, width, height, tmp_blkb, 64, rec->ref_bwd->frm, seq->sample_bit_depth, check_ref);
                g_funs_handle.avg_pel(dst, i_dst, tmp_blk, 64, tmp_blkb, 64, width, height);
            }
        } else {
            i16s_t *mv = cur_frm->mvbuf[b4y * i_b4 + b4x];
            int refframe;
            com_frm_t *p_ref_frm;
            int i_b4 = seq->b4_info_stride;
            refframe = cur_frm->refbuf[b4y * i_b4 + b4x];


            vec1_x = (rec->cu_pix_x + ioff) * 4 + mv[0];
            vec1_y = (rec->cu_pix_y + joff) * 4 + mv[1];

            p_ref_frm = rec->ref_list[refframe].frm;

            if (rec->type != F_IMG) {
                core_inter_get_luma(vec1_x, vec1_y, width, height, dst, i_dst, p_ref_frm, seq->sample_bit_depth, check_ref);
            } else {
                int dmh_mode = local_cu->dmh_mode;

                if (dmh_mode == 0 || rec->typeb == BP_IMG) {
                    core_inter_get_luma(vec1_x, vec1_y, width, height, dst, i_dst, p_ref_frm, seq->sample_bit_depth, check_ref);
                } else {
                    core_inter_get_luma(vec1_x + tab_dmh_pos[dmh_mode][0][0], vec1_y + tab_dmh_pos[dmh_mode][0][1], width, height, tmp_blk, 64, p_ref_frm, seq->sample_bit_depth, check_ref);
                    core_inter_get_luma(vec1_x + tab_dmh_pos[dmh_mode][1][0], vec1_y + tab_dmh_pos[dmh_mode][1][1], width, height, tmp_blkb, 64, p_ref_frm, seq->sample_bit_depth, check_ref);
                    g_funs_handle.avg_pel(dst, i_dst, tmp_blk, 64, tmp_blkb, 64, width, height);
                }

                if (rec->i_refs > 1 && weighted_skipmode && (cu->cuType == 0)) {
                    int vec_wgt_x = (rec->cu_pix_x + ioff) * 4 + (int)((delta[weighted_skipmode] * mv[0] * (MULTI / delta[0]) + HALF_MULTI) >> OFFSET);
                    int vec_wgt_y = (rec->cu_pix_y + joff) * 4 + (int)((delta[weighted_skipmode] * mv[1] * (MULTI / delta[0]) + HALF_MULTI) >> OFFSET);

                    p_ref_frm = rec->ref_list[weighted_skipmode].frm;

                    if (seq->is_field_sequence) {
                        int deltaT, delta2;
                        i64s_t oriPOC = 2 * rec->img_tr_ext;
                        i64s_t oriRefPOC = oriPOC - delta[0];
                        i64s_t scaledPOC = 2 * rec->img_tr_ext;
                        i64s_t scaledRefPOC = scaledPOC - delta[weighted_skipmode];
                        getDeltas(rec->pichdr->is_top_field, &deltaT, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                        vec_wgt_y = (rec->cu_pix_y + joff) * 4 + (int)((delta[weighted_skipmode] * (mv[1] + deltaT) * (16384 / delta[0]) + 8192) >> 14) - delta2;
                    }
                    vec_wgt_x = Clip3(-32768, 32767, vec_wgt_x);
                    vec_wgt_y = Clip3(-32768, 32767, vec_wgt_y);
                    core_inter_get_luma(vec_wgt_x, vec_wgt_y, width, height, tmp_blk, 64, p_ref_frm, seq->sample_bit_depth, check_ref);
                    g_funs_handle.avg_pel(dst, i_dst, dst, i_dst, tmp_blk, 64, width, height);
                }

                if (pred_dir == DUAL || md_directskip_mode == BID_P_FST || md_directskip_mode == BID_P_SND) {
                    int vec_wgt_x = (rec->cu_pix_x + ioff) * 4 + rec->bw_mv[b4y * i_b4 + b4x][0];
                    int vec_wgt_y = (rec->cu_pix_y + joff) * 4 + rec->bw_mv[b4y * i_b4 + b4x][1];
                    p_ref_frm = rec->ref_list[rec->snd_ref[b4y * i_b4 + b4x]].frm;

                    core_inter_get_luma(vec_wgt_x, vec_wgt_y, width, height, tmp_blk, 64, p_ref_frm, seq->sample_bit_depth, check_ref);
                    g_funs_handle.avg_pel(dst, i_dst, dst, i_dst, tmp_blk, 64, width, height);
                }
            }
        }
    }
}


void dec_inter_pred_chroma(com_rec_t *rec, int uv)
{
    com_cu_t *cu = rec->cu;
    int i;
    int check_ref = rec->check_ref;
    int ioff, joff;
    int b8x, b8y;
    const seq_info_t *seq = rec->seq;
    int refframe, pred_dir;
    com_frm_t *p_ref_frm;
    int mb_nr = rec->cu_idx;

    int cu_bitsize = rec->cu_bitsize;
    int start_x, start_y, width, height;

    ALIGNED_32(pel_t tmp_blk[2][32 * 32]);
    i16s_t *fw_mv, *bw_mv;
    int i_b4 = seq->b4_info_stride;
    int base_x, base_y;
    int direct_change = 0;
    int pu_num = tab_pu_blk_num[cu->cuType];
    int i_dst = rec->frm_cur->yuv_data->i_stridec;
    pel_t *dst;
    int delta[4];
    com_cu_local_t *cu_loc_dat = rec->cu_loc_dat;

    for (i = 0; i < rec->i_refs; i++) {
        delta[i] = rec->ref_list[i].dist[0];
        if (rec->pichdr->background_reference_enable && i == rec->i_refs - 1) {
            delta[i] = 1;
        }
    }

    if (!cu->cuType && (cu_loc_dat->md_directskip_mode || cu_bitsize <= 4)) {
        direct_change = 1;
        pu_num = 1;
    }

    for (i = 0; i < pu_num; i++) {
        int blk = tab_pu_2_blk_idx[cu->cuType][i];
        get_pu_pix_info(cu->cuType, cu_bitsize, i, &start_x, &start_y, &width, &height);

        if (direct_change) {
            start_x <<= 1;
            start_y <<= 1;
            width <<= 1;
            height <<= 1;
        }

        ioff = start_x = start_x >> 1;
        joff = start_y = start_y >> 1;
        width = width >> 1;
        height = height >> 1;

        dst = uv ? rec->frm_cur->yuv_data->p_v : rec->frm_cur->yuv_data->p_u;
        dst += (rec->cu_pix_c_y + joff) * i_dst + rec->cu_pix_c_x + ioff;

        b8x = rec->cu_b4_x + (start_x >> 1);
        b8y = rec->cu_b4_y + (start_y >> 1);

        fw_mv = rec->frm_cur->mvbuf[b8y * i_b4 + b8x];
        bw_mv = rec->bw_mv[b8y * i_b4 + b8x];

        pred_dir = cu_loc_dat->b8pdir[blk];

        if (rec->type == B_IMG) {
            int *col_dist = rec->ref_list[0].dist;

            if (pred_dir != SYM && pred_dir != BID) {
                if (!pred_dir) {
                    base_x = (rec->cu_pix_c_x + ioff) * 8 + fw_mv[0];
                    base_y = (rec->cu_pix_c_y + joff) * 8 + fw_mv[1];
                } else {
                    base_x = (rec->cu_pix_c_x + ioff) * 8 + bw_mv[0];
                    base_y = (rec->cu_pix_c_y + joff) * 8 + bw_mv[1];
                }

                if (seq->is_field_sequence) {
                    int is_bwd = pred_dir ? 1 : 0;
                    int distance = col_dist[is_bwd];
                    int delta = (distance % 4) == 0 ? 0 : rec->pichdr->is_top_field ? 2 : -2;
                    base_y -= delta;
                }

                if (!pred_dir) {
                    core_inter_get_chroma(dst, i_dst, rec->ref_fwd->frm, uv, base_x, base_y, width, height, seq->sample_bit_depth, check_ref);
                } else {
                    core_inter_get_chroma(dst, i_dst, rec->ref_bwd->frm, uv, base_x, base_y, width, height, seq->sample_bit_depth, check_ref);
                }
            } else {
                base_x = (rec->cu_pix_c_x + ioff) * 8 + fw_mv[0];
                base_y = (rec->cu_pix_c_y + joff) * 8 + fw_mv[1];

                if (seq->is_field_sequence) {
                    base_y -= (col_dist[0] % 4) == 0 ? 0 : rec->pichdr->is_top_field ? 2 : -2;
                }
                core_inter_get_chroma(tmp_blk[0], 32, rec->ref_fwd->frm, uv, base_x, base_y, width, height, seq->sample_bit_depth, check_ref);

                base_x = (rec->cu_pix_c_x + ioff) * 8 + bw_mv[0];
                base_y = (rec->cu_pix_c_y + joff) * 8 + bw_mv[1];

                if (seq->is_field_sequence) {
                    base_y -= (col_dist[1] % 4) == 0 ? 0 : rec->pichdr->is_top_field ? 2 : -2;
                }
                core_inter_get_chroma(tmp_blk[1], 32, rec->ref_bwd->frm, uv, base_x, base_y, width, height, seq->sample_bit_depth, check_ref);

                g_funs_handle.avg_pel(dst, i_dst, tmp_blk[0], 32, tmp_blk[1], 32, width, height);
            }
        } else {
            int i_b4 = seq->b4_info_stride;

            refframe = rec->frm_cur->refbuf[b8y * i_b4 + b8x];

            p_ref_frm = rec->ref_list[refframe].frm;

            base_x = (rec->cu_pix_c_x + ioff) * 8 + fw_mv[0];
            base_y = (rec->cu_pix_c_y + joff) * 8 + fw_mv[1];

            if (seq->is_field_sequence) {
                base_y -= (rec->ref_list[refframe].dist[0] % 4) == 0 ? 0 : rec->pichdr->is_top_field ? 2 : -2;
            }

            if (rec->type != F_IMG) {
                core_inter_get_chroma(dst, i_dst, p_ref_frm, uv, base_x, base_y, width, height, seq->sample_bit_depth, check_ref);
            } else {
                int dmh_mode = cu_loc_dat->dmh_mode;
                int weighted_skipmode = cu_loc_dat->weighted_skipmode;

                if (dmh_mode == 0) {
                    core_inter_get_chroma(dst, i_dst, p_ref_frm, uv, base_x, base_y, width, height, seq->sample_bit_depth, check_ref);
                } else {
                    int tbase_x, tbase_y;
                    tbase_x = base_x + tab_dmh_pos[dmh_mode][0][0];
                    tbase_y = base_y + tab_dmh_pos[dmh_mode][0][1];
                    core_inter_get_chroma(tmp_blk[0], 32, p_ref_frm, uv, tbase_x, tbase_y, width, height, seq->sample_bit_depth, check_ref);

                    tbase_x = base_x + tab_dmh_pos[dmh_mode][1][0];
                    tbase_y = base_y + tab_dmh_pos[dmh_mode][1][1];
                    core_inter_get_chroma(tmp_blk[1], 32, p_ref_frm, uv, tbase_x, tbase_y, width, height, seq->sample_bit_depth, check_ref);

                    g_funs_handle.avg_pel(dst, i_dst, tmp_blk[0], 32, tmp_blk[1], 32, width, height);
                }

                if (rec->i_refs > 1 && weighted_skipmode && (cu->cuType == 0)) {
                    int vec_wgt_x = (rec->cu_pix_c_x + ioff) * 8 + (int)((delta[weighted_skipmode] * fw_mv[0] * (MULTI / delta[0]) + HALF_MULTI) >> OFFSET);
                    int vec_wgt_y = (rec->cu_pix_c_y + joff) * 8 + (int)((delta[weighted_skipmode] * fw_mv[1] * (MULTI / delta[0]) + HALF_MULTI) >> OFFSET);

                    p_ref_frm = rec->ref_list[weighted_skipmode].frm;

                    if (seq->is_field_sequence) {
                        int deltaT, delta2;
                        i64s_t oriPOC = 2 * rec->img_tr_ext;
                        i64s_t oriRefPOC = oriPOC - delta[0];
                        i64s_t scaledPOC = 2 * rec->img_tr_ext;
                        i64s_t scaledRefPOC = scaledPOC - delta[weighted_skipmode];
                        getDeltas(rec->pichdr->is_top_field, &deltaT, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                        vec_wgt_y = (rec->cu_pix_c_y + joff) * 8 + (int)((delta[weighted_skipmode] * (fw_mv[1] + deltaT) * (16384 / delta[0]) + 8192) >> 14) - delta2;
                    }
                    vec_wgt_x = Clip3(-32768, 32767, vec_wgt_x);
                    vec_wgt_y = Clip3(-32768, 32767, vec_wgt_y);
                    core_inter_get_chroma(tmp_blk[0], 32, p_ref_frm, uv, vec_wgt_x, vec_wgt_y, width, height, seq->sample_bit_depth, check_ref);

                    g_funs_handle.avg_pel(dst, i_dst, dst, i_dst, tmp_blk[0], 32, width, height);
                }

                if (pred_dir == DUAL || cu_loc_dat->md_directskip_mode == BID_P_FST || cu_loc_dat->md_directskip_mode == BID_P_SND) {
                    int psndrefframe = rec->snd_ref[b8y * i_b4 + b8x];
                    base_x = (rec->cu_pix_c_x + ioff) * 8 + bw_mv[0];
                    base_y = (rec->cu_pix_c_y + joff) * 8 + bw_mv[1];

                    if (seq->is_field_sequence) {
                        base_y -= (rec->ref_list[psndrefframe].dist[0] % 4) == 0 ? 0 : rec->pichdr->is_top_field ? 2 : -2;
                    }
                    p_ref_frm = rec->ref_list[psndrefframe].frm;
                    core_inter_get_chroma(tmp_blk[0], 32, p_ref_frm, uv, base_x, base_y, width, height, seq->sample_bit_depth, check_ref);

                    g_funs_handle.avg_pel(dst, i_dst, dst, i_dst, tmp_blk[0], 32, width, height);
                }
            }
        }
    }
}
