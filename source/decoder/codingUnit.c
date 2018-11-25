/*****************************************************************************
*  Copyright (C) 2016 uavs2dec project,
*  National Engineering Laboratory for Video Technology(Shenzhen),
*  Digital Media R&D Center at Peking University Shenzhen Graduate School, China
*  Project Leader: Ronggang Wang <rgwang@pkusz.edu.cn>
*
*  Main Authors: Zhenyu Wang <wangzhenyu@pkusz.edu.cn>, Kui Fan <kuifan@pku.edu.cn>
*               Shenghao Zhang <1219759986@qq.com>， Bingjie Han, Kaili Yao, Hongbin Cao,  Yueming Wang,
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
#include <assert.h>
#include <string.h>

#include "global.h"
#include "commonVariables.h"
#include "codingUnit.h"
#include "bitstream.h"
#include "transform.h"
#include "defines.h"
#include "intra-prediction.h"
#include "inter-prediction.h"
#include "block_info.h"

#include "block.h"
#include "AEC.h"
#include "common.h"

#include "wquant.h"

void read_mv(avs2_dec_t *h_dec);
void read_ref(avs2_dec_t *h_dec);
void pmvr_mv_derivation(i16s_t mv[2], i16s_t mvd[2], i16s_t mvp[2]);

static tab_i16u_t tab_iq_tab[80] = {
    32768, 36061, 38968, 42495, 46341, 50535, 55437, 60424,
    32932, 35734, 38968, 42495, 46177, 50535, 55109, 59933,
    65535, 35734, 38968, 42577, 46341, 50617, 55027, 60097,
    32809, 35734, 38968, 42454, 46382, 50576, 55109, 60056,
    65535, 35734, 38968, 42495, 46320, 50515, 55109, 60076,
    65535, 35744, 38968, 42495, 46341, 50535, 55099, 60087,
    65535, 35734, 38973, 42500, 46341, 50535, 55109, 60097,
    32771, 35734, 38965, 42497, 46341, 50535, 55109, 60099,
    32768, 36061, 38968, 42495, 46341, 50535, 55437, 60424,
    32932, 35734, 38968, 42495, 46177, 50535, 55109, 59933

};

static tab_i16s_t tab_iq_shift[80] = {
    15, 15, 15, 15, 15, 15, 15, 15,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 13, 13, 13, 13, 13, 13, 13,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 11, 11, 11, 11, 11, 11, 11,
    11, 10, 10, 10, 10, 10, 10, 10,
    10, 9, 9, 9, 9, 9, 9, 9,
    8, 8, 8, 8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6

};

/*
*************************************************************************
* Function:Set motion vector predictor
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
#define MEDIAN(a,b,c)  (a + b + c - min(a, min(b, c)) - max(a, max(b, c)));

static void get_pmv(avs2_dec_t *h_dec, unsigned int uiBitSize, unsigned int cu_idx, i16s_t pmv[2], char_t *refFrArr,
                              i16s_t (*tmp_mv)[2], int  ref_frame, int  mb_pix_x, int  mb_pix_y, int  blockshape_x, int  blockshape_y, int  ref, int  direct_mv)  //Lou 1016//qyu 0816 还要添加一个参数mb_nr
{
    int mb_nr = cu_idx;
    int mb_width = h_dec->seq->img_width / 8;
    int mb_x = mb_nr % mb_width;
    int mb_y = mb_nr / mb_width;
    int mb_pix_x_temp = mb_pix_x % MIN_BLOCK_SIZE == 0 ? mb_pix_x : MIN_BLOCK_SIZE;
    int mb_pix_y_temp = mb_pix_y % MIN_BLOCK_SIZE == 0 ? mb_pix_y : MIN_BLOCK_SIZE;
    int pic_block_x = (mb_x << 1) + (mb_pix_x_temp >> MIN_BLOCK_SIZE_IN_BIT);
    int pic_block_y = (mb_y << 1) + (mb_pix_y_temp >> MIN_BLOCK_SIZE_IN_BIT);

    int mb_available_upright;
    int block_available_upright;
    int mv_a, mv_b, mv_c, mv_d, pred_vec = 0;
    int mvPredType, rFrameL, rFrameU, rFrameUR;
    int hv;
    int mva[3] , mvb[3], mvc[3];
    int rFrameUL;
    int upright;
    int b_bwd = ref >= 0 ? 0 : 1;
    int mvc_temp;
    int i_b4 = h_dec->seq->b4_info_stride;

    char_t *ref_buf = refFrArr + pic_block_y * i_b4 + pic_block_x;
    i16s_t(*mv_buf)[2] = tmp_mv + pic_block_y * i_b4 + pic_block_x;

    blockshape_x = blockshape_x % MIN_BLOCK_SIZE == 0 ? blockshape_x : MIN_BLOCK_SIZE;
    blockshape_y = blockshape_y % MIN_BLOCK_SIZE == 0 ? blockshape_y : MIN_BLOCK_SIZE;

    if (h_dec->seq->slice_set_enable) { //added by mz, 2008.04
        mb_available_upright = (mb_x >= mb_width - (1 << (uiBitSize - MIN_CU_SIZE_IN_BIT)) || mb_y == 0) ? 0 : (h_dec->cu_array[mb_nr].slice_set_index == h_dec->cu_array[mb_nr - mb_width + (1 << (uiBitSize - MIN_CU_SIZE_IN_BIT)) ].slice_set_index);
    } else {
        if ((pic_block_y == 0) || ((pic_block_x << MIN_BLOCK_SIZE_IN_BIT) + blockshape_x == h_dec->seq->img_width)) {
            mb_available_upright = 1;
        } else if (mb_pix_y > 0) {
            mb_available_upright = 0;
        } else {
            mb_available_upright = (h_dec->cu_array[cu_idx].slice_nr != h_dec->cu_array[cu_idx - h_dec->seq->img_width_in_mcu + (mb_pix_x_temp + blockshape_x) / 8].slice_nr);
        }
    }

    upright = tab_Up_Right_Avail_Matrix64[pic_block_y - h_dec->lcu_b8_y * 2][pic_block_x - h_dec->lcu_b8_x * 2 + blockshape_x / MIN_BLOCK_SIZE - 1];

    if ((pic_block_x << MIN_BLOCK_SIZE_IN_BIT) + blockshape_x >= h_dec->seq->img_width) {
        upright = 0;
    }

    block_available_upright = upright && (!mb_available_upright);

    /*Lou 1016 Start*/
    mvPredType = MVPRED_xy_MIN;

    rFrameL  = ref_buf[-1];
    rFrameU  = ref_buf[-i_b4];
    rFrameUR = block_available_upright ? ref_buf[-i_b4 + (blockshape_x / 4)] : ref_buf[-i_b4 - 1];
    rFrameUL = ref_buf[-i_b4 - 1];

    if (h_dec->type == P_IMG && h_dec->typeb == BP_IMG) {
        rFrameL = rFrameU = rFrameUR = rFrameUL = -1;
    } else if (h_dec->seq->background_picture_enable && h_dec->pic_hdr.background_reference_enable && (h_dec->type == P_IMG || h_dec->type == F_IMG)) {
        if ((ref_frame == h_dec->i_refs - 1 && rFrameL != h_dec->i_refs - 1 || ref_frame != h_dec->i_refs - 1 && rFrameL == h_dec->i_refs - 1)) {
            rFrameL = -1;
        }
        if ((ref_frame == h_dec->i_refs - 1 && rFrameU != h_dec->i_refs - 1 || ref_frame != h_dec->i_refs - 1 && rFrameU == h_dec->i_refs - 1)) {
            rFrameU = -1;
        }
        if ((ref_frame == h_dec->i_refs - 1 && rFrameUR != h_dec->i_refs - 1 || ref_frame != h_dec->i_refs - 1 && rFrameUR == h_dec->i_refs - 1)) {
            rFrameUR = -1;
        }
        if ((ref_frame == h_dec->i_refs - 1 && rFrameUL != h_dec->i_refs - 1 || ref_frame != h_dec->i_refs - 1 && rFrameUL == h_dec->i_refs - 1)) {
            rFrameUL = -1;
        }
    }

    if ((rFrameL >= 0) && (rFrameU < 0) && (rFrameUR < 0)) {
        mvPredType = MVPRED_L;
    }
    else if ((rFrameL < 0) && (rFrameU >= 0) && (rFrameUR < 0)) {
        mvPredType = MVPRED_U;
    }
    else if ((rFrameL < 0) && (rFrameU < 0) && (rFrameUR >= 0)) {
        mvPredType = MVPRED_UR;
    } else if (blockshape_x < blockshape_y) {
        if (mb_pix_x == 0) {
            if (rFrameL == ref_frame) {
                mvPredType = MVPRED_L;
            }
        } else {
            if (rFrameUR == ref_frame) {
                mvPredType = MVPRED_UR;
            }
        }
    } else if (blockshape_x > blockshape_y) {
        if (mb_pix_y == 0) {
            if (rFrameU == ref_frame) {
                mvPredType = MVPRED_U;
            }
        } else {
            if (rFrameL == ref_frame) {
                mvPredType = MVPRED_L;
            }
        }
    }

    for (hv = 0; hv < 2; hv++) {
        mva[hv] = mv_a = mv_buf[ - 1][hv];
        mvb[hv] = mv_b = mv_buf[ - i_b4][hv];
        mv_d           = mv_buf[-i_b4-1][hv];
        mvc[hv] = mv_c = block_available_upright ? mv_buf[-i_b4+(blockshape_x / MIN_BLOCK_SIZE)][hv] : mv_d;

        if ((rFrameL < 0 && (h_dec->type == P_IMG || h_dec->type == F_IMG) && h_dec->seq->background_picture_enable) || (rFrameL < 0 && h_dec->typeb == BP_IMG)) {
            mva[hv] = 0;
        } else {
            if (h_dec->seq->is_field_sequence && hv == 1 && rFrameL >= 0) {
                int delta, delta2;
                i64s_t oriPOC, oriRefPOC, scaledPOC, scaledRefPOC;
                int mult_distance = h_dec->ref_list[ref_frame].dist[b_bwd];
                int devide_distance = h_dec->ref_list[rFrameL].dist[b_bwd];

                oriPOC = 2 * h_dec->img_tr_ext;
                oriRefPOC = oriPOC - devide_distance;
                scaledPOC = 2 * h_dec->img_tr_ext;
                scaledRefPOC = scaledPOC - mult_distance;
                getDeltas(h_dec->pic_hdr.is_top_field, &delta, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                mva[hv] = scale_motion_vector(h_dec, mva[hv] + delta, ref_frame, rFrameL, ref); 
                mva[hv] -= delta2;
            } else {
                mva[hv] = scale_motion_vector(h_dec, mva[hv], ref_frame, rFrameL, ref); 
            }
        }


        if ((rFrameU < 0 && (h_dec->type == P_IMG || h_dec->type == F_IMG) && h_dec->seq->background_picture_enable) || (rFrameU < 0 && h_dec->typeb == BP_IMG)) {
            mvb[hv] = 0;
        } else {
            if (h_dec->seq->is_field_sequence && hv == 1 && rFrameU >= 0) {
                int delta, delta2;
                i64s_t oriPOC, oriRefPOC, scaledPOC, scaledRefPOC;
                int mult_distance = h_dec->ref_list[ref_frame].dist[b_bwd];
                int devide_distance = h_dec->ref_list[rFrameU].dist[b_bwd];

                oriPOC = 2 * h_dec->img_tr_ext;
                oriRefPOC = oriPOC - devide_distance;
                scaledPOC = 2 * h_dec->img_tr_ext;
                scaledRefPOC = scaledPOC - mult_distance;
                getDeltas(h_dec->pic_hdr.is_top_field, &delta, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                mvb[hv] = scale_motion_vector(h_dec, mvb[hv] + delta, ref_frame, rFrameU, ref);  //, smbtypecurr, smbtypeU, pic_block_y-1, pic_block_y, ref, direct_mv);
                mvb[hv] -= delta2;
            } else {
                mvb[hv] = scale_motion_vector(h_dec, mvb[hv], ref_frame, rFrameU, ref);    //, smbtypecurr, smbtypeU, pic_block_y-1, pic_block_y, ref, direct_mv);
            }
        }


        if ((rFrameUL < 0 && (h_dec->type == P_IMG || h_dec->type == F_IMG) && h_dec->seq->background_picture_enable) || (rFrameUL < 0 && h_dec->typeb == BP_IMG)) {
            mv_d = 0;
        } else {
            if (h_dec->seq->is_field_sequence && hv == 1 && rFrameUL >= 0) {
                int delta, delta2;
                i64s_t oriPOC, oriRefPOC, scaledPOC, scaledRefPOC;
                int mult_distance = h_dec->ref_list[ref_frame].dist[b_bwd];
                int devide_distance = h_dec->ref_list[rFrameUL].dist[b_bwd];

                oriPOC = 2 * h_dec->img_tr_ext;
                oriRefPOC = oriPOC - devide_distance;
                scaledPOC = 2 * h_dec->img_tr_ext;
                scaledRefPOC = scaledPOC - mult_distance;
                getDeltas(h_dec->pic_hdr.is_top_field, &delta, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                mv_d = scale_motion_vector(h_dec, mv_d + delta, ref_frame, rFrameUL, ref);  //, smbtypecurr, smbtypeUL, pic_block_y-1, pic_block_y, ref, direct_mv);
                mv_d -= delta2;
            } else {
                mv_d = scale_motion_vector(h_dec, mv_d, ref_frame, rFrameUL, ref);    //, smbtypecurr, smbtypeUL, pic_block_y-1, pic_block_y, ref, direct_mv);
            }
        }


        if ((rFrameUR < 0 && (h_dec->type == P_IMG || h_dec->type == F_IMG) && h_dec->seq->background_picture_enable) || (rFrameUR < 0 && h_dec->typeb == BP_IMG)) {
            mvc_temp = 0;
        } else {
            if (h_dec->seq->is_field_sequence && hv == 1 && rFrameUR >= 0) {
                int delta, delta2;
                i64s_t oriPOC, oriRefPOC, scaledPOC, scaledRefPOC;
                int mult_distance = h_dec->ref_list[ref_frame].dist[b_bwd];
                int devide_distance = h_dec->ref_list[rFrameUR].dist[b_bwd];

                oriPOC = 2 * h_dec->img_tr_ext;
                oriRefPOC = oriPOC - devide_distance;
                scaledPOC = 2 * h_dec->img_tr_ext;
                scaledRefPOC = scaledPOC - mult_distance;
                getDeltas(h_dec->pic_hdr.is_top_field, &delta, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                mvc_temp = scale_motion_vector(h_dec, mvc[hv] + delta, ref_frame, rFrameUR, ref);
                mvc_temp -= delta2;
            } else {
                mvc_temp = scale_motion_vector(h_dec, mvc[hv], ref_frame, rFrameUR, ref);
            }
        }
        mvc[hv] = block_available_upright ? mvc_temp : mv_d;


        switch (mvPredType) {
        case MVPRED_xy_MIN:
            if (hv == 1) {
                //for x component
                if (((mva[0] < 0) && (mvb[0] > 0) && (mvc[0] > 0)) || (mva[0] > 0) && (mvb[0] < 0) && (mvc[0] < 0)) {
                    pmv[0] = (i16s_t) ((mvb[0] + mvc[0]) / 2);

                } else if (((mvb[0] < 0) && (mva[0] > 0) && (mvc[0] > 0)) || ((mvb[0] > 0) && (mva[0] < 0) && (mvc[0] < 0))) {
                    pmv[0] = (i16s_t) ((mvc[0] + mva[0]) / 2);

                } else if (((mvc[0] < 0) && (mva[0] > 0) && (mvb[0] > 0)) || ((mvc[0] > 0) && (mva[0] < 0) && (mvb[0] < 0))) {
                    pmv[0] = (i16s_t) ((mva[0] + mvb[0]) / 2);

                } else {
                    // !! for Ax
                    mva[2] = abs(mva[0] - mvb[0]);
                    // !! for Bx
                    mvb[2] = abs(mvb[0] - mvc[0]);
                    // !! for Cx
                    mvc[2] = abs(mvc[0] - mva[0]);

                    pred_vec = min(mva[2], min(mvb[2], mvc[2]));

                    if (pred_vec == mva[2]) {
                        pmv[0] = (i16s_t) ((mva[0] + mvb[0]) / 2);
                    } else if (pred_vec == mvb[2]) {
                        pmv[0] = (i16s_t) ((mvb[0] + mvc[0]) / 2);
                    } else {
                        pmv[0] = (i16s_t) ((mvc[0] + mva[0]) / 2);
                    }
                }

                //for y component
                if (((mva[1] < 0) && (mvb[1] > 0) && (mvc[1] > 0)) || (mva[1] > 0) && (mvb[1] < 0) && (mvc[1] < 0)) {
                    pmv[1] = (i16s_t) ((mvb[1] + mvc[1]) / 2);

                } else if (((mvb[1] < 0) && (mva[1] > 0) && (mvc[1] > 0)) || ((mvb[1] > 0) && (mva[1] < 0) && (mvc[1] < 0))) {
                    pmv[1] = (i16s_t) ((mvc[1] + mva[1]) / 2);

                } else if (((mvc[1] < 0) && (mva[1] > 0) && (mvb[1] > 0)) || ((mvc[1] > 0) && (mva[1] < 0) && (mvb[1] < 0))) {
                    pmv[1] = (i16s_t) ((mva[1] + mvb[1]) / 2);

                } else {
                    // !! for Ay
                    mva[2] =  abs(mva[1] - mvb[1]);
                    // !! for By
                    mvb[2] =  abs(mvb[1] - mvc[1]);
                    // !! for Cy
                    mvc[2] =  abs(mvc[1] - mva[1]);

                    pred_vec = min(mva[2], min(mvb[2], mvc[2]));

                    if (pred_vec == mva[2]) {
                        pmv[1] = (i16s_t) ((mva[1] + mvb[1]) / 2);
                    } else if (pred_vec == mvb[2]) {
                        pmv[1] = (i16s_t) ((mvb[1] + mvc[1]) / 2);
                    } else {
                        pmv[1] = (i16s_t) ((mvc[1] + mva[1]) / 2);
                    }
                }

            }
            break;
        case MVPRED_L:
            pred_vec = mva[hv];
            break;
        case MVPRED_U:
            pred_vec = mvb[hv];
            break;
        case MVPRED_UR:
            pred_vec = mvc[hv];
            break;
        default:
            break;
        }

        if (mvPredType != MVPRED_xy_MIN) {
            pmv[hv] = pred_vec;
        }
    }
}

static void get_pmv_bskip(avs2_dec_t *h_dec, unsigned int cu_idx, i16s_t fw_pmv[2], i16s_t bw_pmv[2])
{
    int uiBitSize = h_dec->cu_bitsize;
    int blockshape_x = (1 << uiBitSize) / 4;
    int blockshape_y = (1 << uiBitSize) / 4 - 1;
    int mb_nr                = cu_idx;
    int mb_width             = h_dec->seq->img_width / 8;
    int mb_x = mb_nr % mb_width;
    int mb_y = mb_nr / mb_width;

    int pic_block_x = (mb_x << 1);
    int pic_block_y = (mb_y << 1);

    int ref_frames[6][2];
    int pdir_mode[6];

    ALIGNED_16(i16s_t pmv[6][2][2]);

    int j;
    int bid_flag = 0, bw_flag = 0, fw_flag = 0, sym_flag = 0, bid2=0;
    int md_mode = h_dec->cu_loc_dat->md_directskip_mode;

    int i_b4 = h_dec->seq->b4_info_stride;

    char_t *pdir       = h_dec->pdir            + pic_block_y * i_b4 + pic_block_x;
    char_t *ref_1st    = h_dec->frm_cur->refbuf + pic_block_y * i_b4 + pic_block_x;
    char_t *ref_2nd    = h_dec->bw_ref          + pic_block_y * i_b4 + pic_block_x;
    i16s_t(*mv_1st)[2] = h_dec->frm_cur->mvbuf  + pic_block_y * i_b4 + pic_block_x;
    i16s_t(*mv_2nd)[2] = h_dec->bw_mv           + pic_block_y * i_b4 + pic_block_x;

    ALIGNED_16(i16s_t tmp_1st_mv[5][2]);
    ALIGNED_16(i16s_t tmp_2nd_mv[5][2]);

    memset(tmp_1st_mv, 0, sizeof(tmp_1st_mv));
    memset(tmp_2nd_mv, 0, sizeof(tmp_2nd_mv));

    blockshape_y = blockshape_y * i_b4 - 1;

    pdir_mode[2] = pdir[-1];
    pdir_mode[5] = pdir[blockshape_y];
    pdir -= i_b4;
    pdir_mode[0] = pdir[-1];
    pdir_mode[1] = pdir[0];
    pdir_mode[3] = pdir[blockshape_x];
    pdir_mode[4] = pdir[blockshape_x - 1];

    ref_frames[2][0] = ref_2nd[-1];
    ref_frames[5][0] = ref_2nd[blockshape_y];
    ref_2nd -= i_b4;
    ref_frames[0][0] = ref_2nd[-1];
    ref_frames[1][0] = ref_2nd[0];
    ref_frames[3][0] = ref_2nd[blockshape_x];
    ref_frames[4][0] = ref_2nd[blockshape_x - 1];
    
    ref_frames[2][1] = ref_1st[-1];
    ref_frames[5][1] = ref_1st[blockshape_y];
    ref_1st -= i_b4;
    ref_frames[0][1] = ref_1st[-1];
    ref_frames[1][1] = ref_1st[0];
    ref_frames[3][1] = ref_1st[blockshape_x];
    ref_frames[4][1] = ref_1st[blockshape_x - 1];
    
    CP32(pmv[2][0], mv_2nd[-1]);
    CP32(pmv[5][0], mv_2nd[blockshape_y]);
    mv_2nd -= i_b4;
    CP32(pmv[0][0], mv_2nd[-1]);
    CP32(pmv[1][0], mv_2nd[0]);

    CP32(pmv[3][0], mv_2nd[blockshape_x]);
    CP32(pmv[4][0], mv_2nd[blockshape_x - 1]);

    CP32(pmv[2][1], mv_1st[-1]);
    CP32(pmv[5][1], mv_1st[blockshape_y]);
    mv_1st -= i_b4;
    CP32(pmv[0][1], mv_1st[-1]);
    CP32(pmv[1][1], mv_1st[0]);

    CP32(pmv[3][1], mv_1st[blockshape_x]);
    CP32(pmv[4][1], mv_1st[blockshape_x - 1]);
   
    if (md_mode != DS_SYM) {
        for (j = 0; j < 6; j++) {
            if (pdir_mode[j] == BID) {
                CP32(tmp_1st_mv[DS_BID], pmv[j][0]);
                CP32(tmp_2nd_mv[DS_BID], pmv[j][1]);
                bid_flag++;
                if (bid_flag == 1) {
                    bid2 = j;
                }
            } else if (pdir_mode[j] == BACKWARD) {
                CP32(tmp_1st_mv[DS_BACKWARD], pmv[j][0]);
                bw_flag++;
            } else if (pdir_mode[j] == FORWARD) {
                CP32(tmp_2nd_mv[DS_FORWARD], pmv[j][1]);
                fw_flag++;
            }
        }
    } else {
        for (j = 0; j < 6; j++) {
            if (pdir_mode[j] == BID) {
                CP32(tmp_1st_mv[DS_BID], pmv[j][0]);
                CP32(tmp_2nd_mv[DS_BID], pmv[j][1]);
                bid_flag++;
                if (bid_flag == 1) {
                    bid2 = j;
                }
            } else if (pdir_mode[j] == SYM) {
                CP32(tmp_1st_mv[DS_SYM], pmv[j][0]);
                CP32(tmp_2nd_mv[DS_SYM], pmv[j][1]);
                sym_flag++;
            } else if (pdir_mode[j] == BACKWARD) {
                CP32(tmp_1st_mv[DS_BACKWARD], pmv[j][0]);
                bw_flag++;
            } else if (pdir_mode[j] == FORWARD) {
                CP32(tmp_2nd_mv[DS_FORWARD], pmv[j][1]);
                fw_flag++;
            }
        }
    }

    if (md_mode != DS_SYM) {
        if (bid_flag == 0 && fw_flag != 0 && bw_flag != 0) {
            CP32(tmp_1st_mv[DS_BID], tmp_1st_mv[DS_BACKWARD]);
            CP32(tmp_2nd_mv[DS_BID], tmp_2nd_mv[DS_FORWARD]);
        }
    }

    if (md_mode == DS_SYM) {
        if (sym_flag == 0 && bid_flag > 1) {
            CP32(tmp_1st_mv[DS_SYM], pmv[bid2][0]);
            CP32(tmp_2nd_mv[DS_SYM], pmv[bid2][1]);
        } else if (sym_flag == 0 && bw_flag != 0) {
            CP32(tmp_1st_mv[DS_SYM], tmp_1st_mv[DS_BACKWARD]);
            tmp_2nd_mv[DS_SYM][0] = -tmp_1st_mv[DS_BACKWARD][0];
            tmp_2nd_mv[DS_SYM][1] = -tmp_1st_mv[DS_BACKWARD][1];
        } else if (sym_flag == 0 && fw_flag != 0) {
            // check point
            tmp_1st_mv[DS_SYM][0] = -tmp_2nd_mv[DS_FORWARD][0];
            tmp_1st_mv[DS_SYM][1] = -tmp_2nd_mv[DS_FORWARD][1];
            CP32(tmp_2nd_mv[DS_SYM], tmp_2nd_mv[DS_FORWARD]);
        }
    }

    if (md_mode == DS_BACKWARD) {
        if (bw_flag == 0 && bid_flag > 1) {
            CP32(tmp_1st_mv[DS_BACKWARD], pmv[bid2][0]);
        } else if (bw_flag == 0 && bid_flag != 0) {
            CP32(tmp_1st_mv[DS_BACKWARD], tmp_1st_mv[DS_BID]);
        }
    }
    
    if (md_mode == DS_FORWARD) {
        if (fw_flag == 0 && bid_flag > 1) {
            CP32(tmp_2nd_mv[DS_FORWARD], pmv[bid2][1]);
        } else if (fw_flag == 0 && bid_flag != 0) {
            CP32(tmp_2nd_mv[DS_FORWARD], tmp_2nd_mv[DS_BID]);
        }
    }

    if (h_dec->i_frame_num == 2 && md_mode == 3) {
        printf("\t");
    }

    CP32(fw_pmv, tmp_2nd_mv[md_mode]);
    CP32(bw_pmv, tmp_1st_mv[md_mode]);

}

void init_lcu(avs2_dec_t *h_dec, int lcu_x, int lcu_y)
{
    int i, j;
    const seq_info_t *seq = h_dec->seq;
    com_cu_t *cu_array = h_dec->cu_array;
    int lcu_size   = seq->lcu_size;
    int pix_x      = lcu_x * lcu_size;
    int pix_y      = lcu_y * lcu_size;
    int b8_x       = pix_x >> 3;
    int b8_y       = pix_y >> 3;
    int lcu_width  = lcu_size;
    int lcu_height = lcu_size;
    int lcu_offset = 0;

    if (pix_x + lcu_size >= seq->img_width) {
        lcu_width = seq->img_width - pix_x;
    }

    if (pix_y + lcu_size >= seq->img_height) {
        lcu_height = seq->img_height - pix_y;
    }

    if (h_dec->i_rec_threads) {
        lcu_offset = lcu_y * seq->img_width_in_lcu + lcu_x;
    }

    h_dec->lcu_pix_x  = pix_x;
    h_dec->lcu_pix_y  = pix_y;
    h_dec->lcu_width  = lcu_width;
    h_dec->lcu_height = lcu_height;
    h_dec->lcu_b8_x   = b8_x;
    h_dec->lcu_b8_y   = b8_y;
    h_dec->lcu        = h_dec->lcu_array + lcu_offset;

    cu_array += b8_y * seq->img_width_in_mcu + b8_x;

    for (i = 0; i < lcu_height / 8; i++) {
        for (j = 0; j < lcu_width / 8; j++) {
            cu_array[j].slice_nr = (i16s_t) h_dec->current_slice_nr;
        }
        cu_array += seq->img_width_in_mcu;
    }
}

void init_rec_core_lcu(com_rec_t *rec, com_lcu_t *lcu, int lcu_x, int lcu_y)
{
    const seq_info_t* seq = rec->seq;
    int lcu_size = seq->lcu_size;
    int pix_x = lcu_x * lcu_size;
    int pix_y = lcu_y * lcu_size;
    int lcu_width = lcu_size;
    int lcu_height = lcu_size;

    if (pix_x + lcu_size >= seq->img_width) {
        lcu_width = seq->img_width - pix_x;
    }

    if (pix_y + lcu_size >= seq->img_height) {
        lcu_height = seq->img_height - pix_y;
    }

    rec->lcu        = lcu;
    rec->lcu_pix_x  = pix_x;
    rec->lcu_pix_y  = pix_y;
    rec->lcu_width  = lcu_width;
    rec->lcu_height = lcu_height;
    rec->lcu_b8_x   = pix_x >> 3;
    rec->lcu_b8_y   = pix_y >> 3;

}

static void init_cu(avs2_dec_t *h_dec)
{
    int r, c;
    int cu_idx     = h_dec->cu_idx;
    int cu_bitsize = h_dec->cu_bitsize;
    int b8size     = h_dec->cu_b8size;
    com_cu_t *cu   = h_dec->cu;

    assert(cu_idx >= 0 && cu_idx < h_dec->seq->img_size_in_mcu);

    cu->ui_MbBitSize = (uchar_t) cu_bitsize;
    cu->cuType       = SKIPDIRECT;
    h_dec->cu_loc_dat->dmh_mode = 0;

    for (r = 0; r < b8size; r++) {
        for (c = 0; c < b8size; c++) {
            cu[c].ui_MbBitSize = (uchar_t) cu_bitsize;
        }
        cu += h_dec->seq->img_width_in_mcu;
    }
}
/*
*************************************************************************
* Function:Interpret the mb mode for P-Frames
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
void mvd_prediction(int *mvd_U, int *mvd_L, int *mvd_C, int available_U, int available_L, int available_C, int *pmvd)
{
    int abs_U, abs_L, abs_C;
    abs_U = abs(mvd_U[0]) + abs(mvd_U[1]);
    abs_L = abs(mvd_L[0]) + abs(mvd_L[1]);
    abs_C = abs(mvd_C[0]) + abs(mvd_C[1]);

    if (!abs_L || !abs_U) {
        pmvd[0] = pmvd[1] = 0;
    } else {
        pmvd[0] = MEDIAN(mvd_U[0], mvd_L[0], mvd_C[0]);
        pmvd[1] = MEDIAN(mvd_U[1], mvd_L[1], mvd_C[1]);
    }
}

static int get_colocal_idx(avs2_dec_t *h_dec, int bx, int by)
{
    const seq_info_t* seq = h_dec->seq;
    int i_b4 = seq->b4_info_stride;
    int b4w = seq->img_width_in_mcu * 2;
    int b4h = seq->img_height_in_mcu * 2;

    int xpos = bx / 4 * 4 + 2;
    int ypos = by / 4 * 4 + 2;

    if (ypos >= b4h){
        ypos = (by / 4 * 4 + b4h) / 2;
    }
    if (xpos >= b4w){
        xpos = (bx / 4 * 4 + b4w) / 2;
    }

    return ypos * i_b4 + xpos;
}

static void PskipMV_COL(avs2_dec_t *h_dec, com_cu_t *cu)
{
    //  int bx, by;
    int cu_idx = h_dec->cu_idx;
    int b8size = h_dec->cu_b8size;
    int mb_nr = cu_idx;
    int mb_width = h_dec->seq->img_width / 8;
    int mb_x = mb_nr % mb_width;
    int mb_y = mb_nr / mb_width;
    int pic_block8_x = mb_x << (MIN_CU_SIZE_IN_BIT - MIN_BLOCK_SIZE_IN_BIT); //qyu 0830
    int pic_block8_y = mb_y << (MIN_CU_SIZE_IN_BIT - MIN_BLOCK_SIZE_IN_BIT);
    int pic_pix_x = mb_x << (MIN_CU_SIZE_IN_BIT);
    int pic_pix_y = mb_y << (MIN_CU_SIZE_IN_BIT);
    com_ref_t *col_frm = &h_dec->ref_list[0];
    int *col_dist = h_dec->ref_list[0].dist;

    int i_b4 = h_dec->seq->b4_info_stride;
    
    i64s_t *col_pic_poc = col_frm->ref_reftrs;
    int refframe;
    int curT, colT;
    int i, j, l, m;
    int delta = 0, delta2 = 0;

    if (h_dec->cu_bitsize > MIN_CU_SIZE_IN_BIT) {
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 2; j++) {
                int block_x = pic_block8_x + b8size * i;
                int block_y = pic_block8_y + b8size * j;
                int col_idx = get_colocal_idx(h_dec, block_x, block_y);
                i16s_t *col_mv  = col_frm->frm->mvbuf [col_idx];
                char_t  col_ref = col_frm->frm->refbuf[col_idx];
                i16s_t(*cur_mv)[2] = h_dec->frm_cur->mvbuf + block_y * i_b4 + block_x;
                i16s_t *mv = cur_mv[0];

                refframe = col_ref;

                if (refframe >= 0) {
                    curT = col_dist[0];
                    colT = COM_ADD_MODE(2 * (col_frm->ref_imgtr - col_pic_poc[refframe]), 512);

                    if (h_dec->pic_hdr.background_reference_enable) {
                        if (h_dec->i_refs == 1) {
                            curT = 1;
                            colT = 1;
                        }
                        if (refframe == h_dec->i_refs - 1) {
                            colT = 1;
                        }
                    }

                    if (h_dec->seq->is_field_sequence) {
                        i64s_t oriPOC, oriRefPOC, scaledPOC, scaledRefPOC;
                        oriPOC = 2 * h_dec->ref_list[0].ref_imgtr;
                        oriRefPOC = oriPOC - colT;
                        scaledPOC = 2 * h_dec->img_tr_ext;
                        scaledRefPOC = 2 * h_dec->ref_list[0].ref_imgtr;
                        getDeltas(h_dec->pic_hdr.is_top_field, &delta, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                    }


                    mv[0] = (curT * col_mv[0] * (MULTI / colT) + HALF_MULTI) >> OFFSET;
                    mv[1] = ((curT * (col_mv[1] + delta) * (MULTI / colT) + HALF_MULTI) >> OFFSET) - delta2;
                    mv[0] = Clip3(-32768, 32767, mv[0]);
                    mv[1] = Clip3(-32768, 32767, mv[1]);
                } else {
                    mv[0] = 0;
                    mv[1] = 0;
                }

                for (m = 0; m < b8size; m++) {
                    for (l = 0; l < b8size; l++) {
                        cur_mv[l][0] = mv[0];
                        cur_mv[l][1] = mv[1];
                    }
                    cur_mv += i_b4;
                }
            }
        }
    } else {
        int col_idx = get_colocal_idx(h_dec, pic_block8_x, pic_block8_y);
        i16s_t *col_mv = col_frm->frm->mvbuf[col_idx];
        char_t  col_ref = col_frm->frm->refbuf[col_idx];
        i16s_t(*cur_mv)[2] = h_dec->frm_cur->mvbuf + pic_block8_y * i_b4 + pic_block8_x;
        i16s_t *mv = cur_mv[0];

        refframe = col_ref;

        if (refframe >= 0) {
            curT = col_dist[0];
            colT = (int) COM_ADD_MODE(2 * (col_frm->ref_imgtr - col_pic_poc[refframe]), 512);

            if (h_dec->pic_hdr.background_reference_enable) {
                if (h_dec->i_refs == 1) {
                    curT = 1;
                    colT = 1;
                }
                if (refframe == h_dec->i_refs - 1) {
                    colT = 1;
                }
            }

            if (h_dec->seq->is_field_sequence) {
                i64s_t oriPOC, oriRefPOC, scaledPOC, scaledRefPOC;
                oriPOC = 2 * h_dec->ref_list[0].ref_imgtr;
                oriRefPOC = oriPOC - colT;
                scaledPOC = 2 * h_dec->img_tr_ext;
                scaledRefPOC = 2 * h_dec->ref_list[0].ref_imgtr;
                getDeltas(h_dec->pic_hdr.is_top_field, &delta, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
            }

            mv[0] = (i16s_t) ((curT * col_mv[0] * (MULTI / colT) + HALF_MULTI) >> OFFSET);
            mv[1] = (i16s_t) (((curT * (col_mv[1] + delta) * (MULTI / colT) + HALF_MULTI) >> OFFSET) - delta2);
            mv[0] = (i16s_t) Clip3(-32768, 32767, mv[0]);
            mv[1] = (i16s_t) Clip3(-32768, 32767, mv[1]);
        } else {
            mv[0] = 0;
            mv[1] = 0;
        }

        for (m = 0; m < b8size * 2; m++) {
            for (l = 0; l < b8size * 2; l++) {
                cur_mv[l][0] = mv[0];
                cur_mv[l][1] = mv[1];
            }
            cur_mv += i_b4;
        }
    }
}

static void get_pmv_pskip(avs2_dec_t *h_dec, unsigned int cu_idx, int b8size)
{
    int uiBitSize = h_dec->cu_bitsize;
    int blockshape_x = 1 << uiBitSize;
    int blockshape_y = 1 << uiBitSize;
    int mb_nr                = cu_idx;
    int mb_width             = h_dec->seq->img_width / 8;
    int mb_x = mb_nr % mb_width;
    int mb_y = mb_nr / mb_width;
    int pic_block_x = (mb_x << 1);
    int pic_block_y = (mb_y << 1);
    int block8_y = mb_y << 1;
    int block8_x = mb_x << 1;
    int hv;
    int ref_frames[2][6];
    int pmv[2][2][6];
    int i, j;
    int bid_flag = 0, bw_flag = 0, fw_flag = 0, sym_flag = 0, bid2 = 0, fw2 = 0;
    int i_b4 = h_dec->seq->b4_info_stride;
    int b4size = h_dec->cu_b8size << 1;
    int md_mode = h_dec->cu_loc_dat->md_directskip_mode;

    char_t *ref_1st    = h_dec->frm_cur->refbuf + pic_block_y * i_b4 + pic_block_x;
    char_t *ref_2nd    = h_dec->snd_ref  + pic_block_y * i_b4 + pic_block_x;
    i16s_t(*mv_1st)[2] = h_dec->frm_cur->mvbuf  + pic_block_y * i_b4 + pic_block_x;
    i16s_t(*mv_2nd)[2] = h_dec->bw_mv           + pic_block_y * i_b4 + pic_block_x;

    char_t tmp_pref_fst[5];
    char_t tmp_pref_snd[5];
    i16s_t tmp_1st_mv[5][2];
    i16s_t tmp_2nd_mv[5][2];

    memset(tmp_1st_mv, 0, sizeof(tmp_1st_mv));
    memset(tmp_2nd_mv, 0, sizeof(tmp_2nd_mv));
    memset(tmp_pref_fst, 0, sizeof(tmp_pref_fst));
    memset(tmp_pref_snd, 0, sizeof(tmp_pref_snd));

    ref_frames[0][0] = ref_1st[-i_b4 - 1];
    ref_frames[0][1] = ref_1st[-i_b4];
    ref_frames[0][2] = ref_1st[-1];
    ref_frames[0][3] = ref_1st[-i_b4 + (blockshape_x / 4)];
    ref_frames[0][4] = ref_1st[-i_b4 + blockshape_x / 4 - 1];
    ref_frames[0][5] = ref_1st[(blockshape_y / 4 - 1)*i_b4 - 1];

    ref_frames[1][0] = ref_2nd[-i_b4 - 1];
    ref_frames[1][1] = ref_2nd[-i_b4];
    ref_frames[1][2] = ref_2nd[-1];
    ref_frames[1][3] = ref_2nd[-i_b4 + (blockshape_x / 4)];
    ref_frames[1][4] = ref_2nd[-i_b4 + blockshape_x / 4 - 1];
    ref_frames[1][5] = ref_2nd[(blockshape_y / 4 - 1)*i_b4 - 1];


    for (hv = 0; hv < 2; hv++) {
        pmv[0][hv][0] = mv_1st[-i_b4 - 1][hv];
        pmv[0][hv][1] = mv_1st[-i_b4][hv];
        pmv[0][hv][2] = mv_1st[-1][hv];
        pmv[0][hv][3] = mv_1st[-i_b4 + (blockshape_x / 4)][hv];
        pmv[0][hv][4] = mv_1st[-i_b4 + blockshape_x / 4 - 1][hv];
        pmv[0][hv][5] = mv_1st[(blockshape_y / 4 - 1)*i_b4 - 1][hv];

        pmv[1][hv][0] = mv_2nd[-i_b4 - 1][hv];
        pmv[1][hv][1] = mv_2nd[-i_b4][hv];
        pmv[1][hv][2] = mv_2nd[-1][hv];
        pmv[1][hv][3] = mv_2nd[-i_b4 + (blockshape_x / 4)][hv];
        pmv[1][hv][4] = mv_2nd[-i_b4 + blockshape_x / 4 - 1][hv];
        pmv[1][hv][5] = mv_2nd[(blockshape_y / 4 - 1)*i_b4 - 1][hv];
    }

    for (j = 0; j < 6; j++) {
        if (ref_frames[0][j] >= 0 && ref_frames[1][j] >= 0) {
            tmp_pref_fst[BID_P_FST] = (char_t) ref_frames[0][j];
            tmp_pref_snd[BID_P_FST] = (char_t) ref_frames[1][j];
            tmp_1st_mv[BID_P_FST][0] = (i16s_t) pmv[0][0][j];
            tmp_1st_mv[BID_P_FST][1] = (i16s_t) pmv[0][1][j];
            tmp_2nd_mv[BID_P_FST][0] = (i16s_t) pmv[1][0][j];
            tmp_2nd_mv[BID_P_FST][1] = (i16s_t) pmv[1][1][j];
            bid_flag++;
            if (bid_flag == 1) {
                bid2 = j;
            }
        } else if (ref_frames[0][j] >= 0 && ref_frames[1][j] < 0) {
            tmp_pref_fst[FW_P_FST] = (char_t) ref_frames[0][j];
            tmp_1st_mv[FW_P_FST][0] = (i16s_t) pmv[0][0][j];
            tmp_1st_mv[FW_P_FST][1] = (i16s_t) pmv[0][1][j];
            fw_flag++;
            if (fw_flag == 1) {
                fw2 = j;
            }
        }
    }

    //first bid
    if (bid_flag == 0 && fw_flag > 1) {
        tmp_pref_fst[BID_P_FST] = tmp_pref_fst[FW_P_FST];
        tmp_pref_snd[BID_P_FST] = (char_t) ref_frames[0][fw2];
        tmp_1st_mv[BID_P_FST][0] = tmp_1st_mv[FW_P_FST][0];
        tmp_1st_mv[BID_P_FST][1] = tmp_1st_mv[FW_P_FST][1];
        tmp_2nd_mv[BID_P_FST][0] = (i16s_t) pmv[0][0][fw2];
        tmp_2nd_mv[BID_P_FST][1] = (i16s_t) pmv[0][1][fw2];
    }

    if (md_mode == BID_P_SND) {
        if (bid_flag > 1) {
            tmp_pref_fst[BID_P_SND] = (char_t) ref_frames[0][bid2];
            tmp_pref_snd[BID_P_SND] = (char_t) ref_frames[1][bid2];
            tmp_1st_mv[BID_P_SND][0] = (i16s_t) pmv[0][0][bid2];
            tmp_1st_mv[BID_P_SND][1] = (i16s_t) pmv[0][1][bid2];
            tmp_2nd_mv[BID_P_SND][0] = (i16s_t) pmv[1][0][bid2];
            tmp_2nd_mv[BID_P_SND][1] = (i16s_t) pmv[1][1][bid2];
        } else if (bid_flag == 1 && fw_flag > 1) {
            tmp_pref_fst[BID_P_SND] = tmp_pref_fst[FW_P_FST];
            tmp_pref_snd[BID_P_SND] = (char_t) ref_frames[0][fw2];
            tmp_1st_mv[BID_P_SND][0] = tmp_1st_mv[FW_P_FST][0];
            tmp_1st_mv[BID_P_SND][1] = tmp_1st_mv[FW_P_FST][1];
            tmp_2nd_mv[BID_P_SND][0] = (i16s_t) pmv[0][0][fw2];
            tmp_2nd_mv[BID_P_SND][1] = (i16s_t) pmv[0][1][fw2];
        }
    }

    if (md_mode == FW_P_FST) {
        if (fw_flag == 0 && bid_flag > 1) {
            tmp_pref_fst[FW_P_FST] = (char_t) ref_frames[0][bid2];
            tmp_1st_mv[FW_P_FST][0] = (i16s_t) pmv[0][0][bid2];
            tmp_1st_mv[FW_P_FST][1] = (i16s_t) pmv[0][1][bid2];
        } else if (fw_flag == 0 && bid_flag == 1) {
            tmp_pref_fst[FW_P_FST] = tmp_pref_fst[BID_P_FST];
            tmp_1st_mv[FW_P_FST][0] = tmp_1st_mv[BID_P_FST][0];
            tmp_1st_mv[FW_P_FST][1] = tmp_1st_mv[BID_P_FST][1];
        }
    }

    if (md_mode == FW_P_SND) {
        if (fw_flag > 1) {
            tmp_pref_fst[FW_P_SND] = (char_t) ref_frames[0][fw2];
            tmp_1st_mv[FW_P_SND][0] = (i16s_t) pmv[0][0][fw2];
            tmp_1st_mv[FW_P_SND][1] = (i16s_t) pmv[0][1][fw2];
        } else if (bid_flag > 1) {
            tmp_pref_fst[FW_P_SND] = (char_t) ref_frames[1][bid2];
            tmp_1st_mv[FW_P_SND][0] = (i16s_t) pmv[1][0][bid2];
            tmp_1st_mv[FW_P_SND][1] = (i16s_t) pmv[1][1][bid2];
        } else if (bid_flag == 1) {
            tmp_pref_fst[FW_P_SND] = tmp_pref_snd[BID_P_FST];
            tmp_1st_mv[FW_P_SND][0] = tmp_2nd_mv[BID_P_FST][0];
            tmp_1st_mv[FW_P_SND][1] = tmp_2nd_mv[BID_P_FST][1];
        }
    }

    if (md_mode == FW_P_FST || md_mode == FW_P_SND) {
        tmp_pref_snd[md_mode] = -1;
    }

    for (j = 0; j < b4size; j++) {
        for (i = 0; i < b4size; i++) {
            ref_1st[i] = tmp_pref_fst[md_mode];
            ref_2nd[i] = tmp_pref_snd[md_mode];
            mv_1st[i][0] = tmp_1st_mv[md_mode][0];
            mv_1st[i][1] = tmp_1st_mv[md_mode][1];
            mv_2nd[i][0] = tmp_2nd_mv[md_mode][0];
            mv_2nd[i][1] = tmp_2nd_mv[md_mode][1];
        }
        ref_1st += i_b4;
        ref_2nd += i_b4;
        mv_1st += i_b4;
        mv_2nd += i_b4;
    }
}

/*
*************************************************************************
* Function:Interpret the mb mode for P-Frames
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
void interpret_b8pdir_PF(avs2_dec_t *h_dec, int pdir)
{
    static tab_i32s_t pdir0[4] = { FORWARD, FORWARD, DUAL, DUAL };
    static tab_i32s_t pdir1[4] = { FORWARD, DUAL, FORWARD, DUAL };

    char_t *curr_b8pdir = h_dec->cu_loc_dat->b8pdir;

    switch (h_dec->cu->cuType)
    {
    case 0:
        curr_b8pdir[0] = curr_b8pdir[1] = \
        curr_b8pdir[2] = curr_b8pdir[3] = FORWARD;
        break;
    case 1:
        curr_b8pdir[0] = curr_b8pdir[1] = \
        curr_b8pdir[2] = curr_b8pdir[3] = (char_t) (pdir == 0 ? FORWARD : DUAL);
        break;
    case 2:
    case 4:
    case 5:
        curr_b8pdir[0] = curr_b8pdir[1] = (char_t) pdir0[pdir];
        curr_b8pdir[2] = curr_b8pdir[3] = (char_t) pdir1[pdir];
        break;
    case 3:
    case 6:
    case 7:
        curr_b8pdir[0] = curr_b8pdir[2] = (char_t) pdir0[pdir];
        curr_b8pdir[1] = curr_b8pdir[3] = (char_t) pdir1[pdir];
        break;
    case 9:
        curr_b8pdir[0] = curr_b8pdir[1] = \
        curr_b8pdir[2] = curr_b8pdir[3] = INTRA;
        break;
    default:
        break;
    }
}


/*
*************************************************************************
* Function:Interpret the mb mode for I-Frames
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/

void interpret_b8pdir_I(avs2_dec_t *h_dec)
{
    char_t *curr_b8pdir = h_dec->cu_loc_dat->b8pdir;
    curr_b8pdir[0] = curr_b8pdir[1] = curr_b8pdir[2] = curr_b8pdir[3] = INTRA;
}

/*
*************************************************************************
* Function:Interpret the mb mode for B-Frames
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/

void interpret_b8pdir_B(avs2_dec_t *h_dec, int pdir)
{

    static tab_i32s_t pdir0[16] = {FORWARD, BACKWARD, FORWARD, BACKWARD, FORWARD, BACKWARD, SYM, SYM, SYM, FORWARD, BACKWARD, SYM, BID, BID, BID, BID};
    static tab_i32s_t pdir1[16] = { FORWARD, BACKWARD, BACKWARD, FORWARD, SYM, SYM, FORWARD, BACKWARD, SYM, BID, BID, BID, FORWARD, BACKWARD, SYM, BID };
    static tab_i32s_t pdir_ext[5] = { SYM, BID, BACKWARD, SYM, FORWARD };

    char_t *curr_b8pdir = h_dec->cu_loc_dat->b8pdir;

    switch (h_dec->cu->cuType)
    {
    case 0:
        curr_b8pdir[0] = curr_b8pdir[1] = \
        curr_b8pdir[2] = curr_b8pdir[3] = (char_t) pdir_ext[h_dec->cu_loc_dat->md_directskip_mode];
        break;
    case 9:
        curr_b8pdir[0] = curr_b8pdir[1] = \
        curr_b8pdir[2] = curr_b8pdir[3] = INTRA;
        break;
    case 1:
        curr_b8pdir[0] = curr_b8pdir[1] = \
        curr_b8pdir[2] = curr_b8pdir[3] = (char_t) pdir;
        break;
    case 2:
    case 4:
    case 5:
        curr_b8pdir[0] = curr_b8pdir[1] = (char_t) pdir0[pdir];
        curr_b8pdir[2] = curr_b8pdir[3] = (char_t) pdir1[pdir];
        break;
    case 3:
    case 6:
    case 7:
        curr_b8pdir[0] = curr_b8pdir[2] = (char_t) pdir0[pdir];
        curr_b8pdir[1] = curr_b8pdir[3] = (char_t) pdir1[pdir];
        break;
    default:
        break;
    }
}

/*
*************************************************************************
* Function:init com_cu_t I and P frames
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
void init_sub_cu_ref(avs2_dec_t *h_dec)
{
    int i;
    int b4y = h_dec->cu_b8_y << 1;
    int b4x = h_dec->cu_b8_x << 1;
    int b4size = h_dec->cu_b8size << 1;
    int i_b4 = h_dec->seq->b4_info_stride;

    char_t *ref_1st = h_dec->frm_cur->refbuf + b4y * i_b4 + b4x;

    for (i = 0; i < b4size; i++) {
        memset(ref_1st, 0, b4size);
        ref_1st += i_b4;
    }
}

void init_sub_cu_ref_B(avs2_dec_t *h_dec, unsigned int cu_idx)
{
    com_cu_t *cu = h_dec->cu;

    int i, j, k;
    int r, row, col, width, height;
    int block8_y = (cu_idx / h_dec->seq->img_width_in_mcu) << 1;
    int block8_x = (cu_idx % h_dec->seq->img_width_in_mcu) << 1;

    int i_b4 = h_dec->seq->b4_info_stride;
    char_t *bw_ref_base;
    char_t *bw_ref;
    char_t *b8pdir = h_dec->cu_loc_dat->b8pdir;

    bw_ref_base = h_dec->bw_ref + block8_y * i_b4 + block8_x;

    for (j = 0; j < 2; j++) {
        for (i = 0; i < 2; i++) {
            k = 2 * j + i;
            get_sub_blk_offset(cu->cuType, h_dec->cu_bitsize, i, j, &col, &row, &width, &height);

            bw_ref = bw_ref_base + row * i_b4 + col;
 
            if (b8pdir[k] == BACKWARD || b8pdir[k] == SYM || b8pdir[k] == BID) {
                for (r = 0; r < height; r++) {
                    memset(bw_ref, 0, width);
                    bw_ref += i_b4;
                }
            }
        }
    }
}

void init_sub_cu_pdir(avs2_dec_t *h_dec, unsigned int cu_idx)
{
    com_cu_t *cu = h_dec->cu;

    int i, j, k;
    int r, row, col, width, height;
    int block8_y = (cu_idx / h_dec->seq->img_width_in_mcu) << 1;
    int block8_x = (cu_idx % h_dec->seq->img_width_in_mcu) << 1;

    int i_b4 = h_dec->seq->b4_info_stride;
    char_t *b8pdir = h_dec->cu_loc_dat->b8pdir;
    char_t *pdir_base;
    char_t *pdir;

    pdir_base = h_dec->pdir + block8_y * i_b4 + block8_x;

    for (j = 0; j < 2; j++) {
        for (i = 0; i < 2; i++) {
            k = 2 * j + i;
            get_sub_blk_offset(cu->cuType, h_dec->cu_bitsize, i, j, &col, &row, &width, &height);

            pdir = pdir_base + row * i_b4 + col;
            for (r = 0; r < height; r++) {
                memset(pdir, b8pdir[k], width);
                pdir += i_b4;
            }
        }
    }
}

void read_ipred_block_modes(avs2_dec_t *h_dec, int b8, unsigned int cu_idx)
{
    int bi, bj, dec_mode, i, j;
    int cu_bitsize = h_dec->cu_bitsize;
    int cuType = h_dec->cu->cuType;
    int mostProbableIntraPredMode[2];
    int upIntraPredMode;
    int leftIntraPredMode;
    aec_core_t *aec;
    int b8size = h_dec->cu_b8size;
    int b4size = b8size << 1;
    int sb4size = b4size >> 1;
    int block8_y = (cu_idx / h_dec->seq->img_width_in_mcu) << 1;
    int block8_x = (cu_idx % h_dec->seq->img_width_in_mcu) << 1;

    aec = &h_dec->aec_core;

    if (b8 < 4) {
        int ipred = aec_intra_pred_mode(aec);
        int i_stride = h_dec->seq->b4_info_stride;
        char_t *mode_buf;
        
        if (cuType == InNxNMB) {
            bi = block8_x;
            bj = block8_y + b8 * (1 << (cu_bitsize - 4));
        }
        else if (cuType == INxnNMB) {
            bi = block8_x + b8 * (1 << (cu_bitsize - 4));
            bj = block8_y;
        }
        else {
            bi = block8_x + (b8 & 1) * b8size;
            bj = block8_y + (b8 / 2) * b8size;
        }

        mode_buf = h_dec->ipredmode + bj * i_stride + bi;
        upIntraPredMode   = mode_buf[-i_stride];
        leftIntraPredMode = mode_buf[-1];

        mostProbableIntraPredMode[0] = min(upIntraPredMode, leftIntraPredMode);
        mostProbableIntraPredMode[1] = max(upIntraPredMode, leftIntraPredMode);

        if (mostProbableIntraPredMode[0] == mostProbableIntraPredMode[1]) {
            mostProbableIntraPredMode[0] = DC_PRED;
            mostProbableIntraPredMode[1] = (mostProbableIntraPredMode[1] == DC_PRED) ? BI_PRED : mostProbableIntraPredMode[1];
        }

        dec_mode = (ipred < 0) ? mostProbableIntraPredMode[ipred + 2] : ipred + (ipred >= mostProbableIntraPredMode[0]) + (ipred + 1 >= mostProbableIntraPredMode[1]);

        if (h_dec->cu_loc_dat->trans_size == 0) {
            mode_buf = h_dec->ipredmode + block8_y * i_stride + block8_x;

            for (j = 0; j < b4size; j++) {
                for (i = 0; i < b4size; i++) {
                    mode_buf[i] = dec_mode;
                }
                mode_buf += i_stride;
            }
        } else {
            if (cuType == InNxNMB) {
                i = b8 * (1 << (cu_bitsize - 4));
                mode_buf = h_dec->ipredmode + (block8_y + i) * i_stride + block8_x;

                for (j = 0; j < (1 << (cu_bitsize + 1 - MIN_CU_SIZE_IN_BIT)); j++) {
                    if (cu_bitsize == 4) {
                        mode_buf[j] = dec_mode;
                    } else {
                        mode_buf[j] = dec_mode;
                        mode_buf[j + i_stride] = dec_mode;
                    }
                }
            } else if (cuType == INxnNMB) {
                i = b8 * (1 << (cu_bitsize - 4));
                mode_buf = h_dec->ipredmode + block8_y * i_stride + block8_x + i;

                for (j = 0; j < (1 << (cu_bitsize + 1 - MIN_CU_SIZE_IN_BIT)); j++) {
                    if (cu_bitsize == 4) {
                        mode_buf[j * i_stride] = (char_t) dec_mode;
                    } else {
                        mode_buf[j * i_stride] = dec_mode;
                        mode_buf[j * i_stride + 1] = dec_mode;

                    }
                }
            } else {
                mode_buf = h_dec->ipredmode + bj * i_stride + bi;
                
                for (j = 0; j < sb4size; j++) {
                    for (i = 0; i < sb4size; i++) {
                        mode_buf[i] = (char_t) dec_mode;
                    }
                    mode_buf += i_stride;
                }
            }
        }
        h_dec->cu_loc_dat->intra_pred_modes[b8] = (char_t) dec_mode;

    } else {
        int cipred = aec_intra_pred_mode_c(h_dec, aec);

        for (i = 0; i < sb4size; i++) {
            int pos = cu_idx + i * h_dec->seq->img_width_in_mcu;

            for (j = 0; j < sb4size; j++, pos++) {
                com_cu_t *tmpMB = &h_dec->cu_array[pos];
                tmpMB->c_ipred_mode = (char_t) cipred;
            }
        }
    }
}


/*
*************************************************************************
* Function:Get the syntax elements from the NAL
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
void read_cu_hdr(avs2_dec_t *h_dec, int *real_cuType)
{
    int cu_idx      = h_dec->cu_idx;
    com_cu_t *cu    = h_dec->cu;
    int cuType;
    int b8size      = h_dec->cu_b8size;
    aec_core_t *aec = &(h_dec->aec_core);
    int i;
    int pdir;
    com_cu_local_t *cu_loc_dat = h_dec->cu_loc_dat;
    int md_directskip_mode = 0;
    cu_loc_dat->md_directskip_mode = 0;


    if (h_dec->type == I_IMG) {
        cuType = *real_cuType = I8MB;
    } else if (h_dec->type == P_IMG && h_dec->typeb == BP_IMG) {
        *real_cuType = aec_cu_type_sfrm(h_dec, aec);
        cuType = max(*real_cuType, 0);
    } else {
        *real_cuType = aec_cu_type(h_dec, aec);
        cuType = max(*real_cuType, 0);
    }

    cu->cuType = (char_t) cuType;

    if (h_dec->type == B_IMG && (cuType >= P2NX2N && cuType <= PVER_RIGHT)) {
        pdir = aec_bpred_dir(h_dec, aec);
    } else if (h_dec->type == F_IMG  && h_dec->i_refs > 1 && h_dec->seq->dhp_enabled && (cuType >= P2NX2N && cuType <= PVER_RIGHT)) {
        if (h_dec->cu_bitsize == B8X8_IN_BIT && cuType >= P2NXN && cuType <= PVER_RIGHT) {
            pdir = 0;
        } else {
            pdir = aec_p_dhp_dir(aec, cu);
        }
    } else {
        pdir = 0;
    }

    cu_loc_dat->weighted_skipmode = 0;

    if (IS_P_SKIP(cu) && h_dec->type == F_IMG) {
        if (h_dec->seq->wsm_enabled && h_dec->i_refs > 1) {
            cu_loc_dat->weighted_skipmode = (char_t) aec_weight_skip_mode(h_dec, aec);
        } else {
            cu_loc_dat->weighted_skipmode = 0;
        }
        
        if ((cu_loc_dat->weighted_skipmode == 0) && h_dec->seq->b_mhpskip_enabled) {
            cu_loc_dat->md_directskip_mode = (char_t) aec_pskip_mode(aec);
        }
    }

    if (IS_B_SKIP(cu)) {
        cu_loc_dat->md_directskip_mode = (char_t) aec_direct_skip(aec);
    }

    if ((h_dec->type == F_IMG) || (h_dec->type == P_IMG)) {
        interpret_b8pdir_PF(h_dec, pdir);
    } else if (h_dec->type == I_IMG) {                            // intra frame
        interpret_b8pdir_I(h_dec);
    } else if ((B_IMG == h_dec->type)) { // B frame
        interpret_b8pdir_B(h_dec, pdir);
    }

    if (IS_INTRA(cu)) {
        cu_loc_dat->trans_size = (char_t) aec_trans_size(aec, h_dec->cu_bitsize, h_dec->seq->useSDIP);
        if (h_dec->seq->useSDIP) {
            cuType = aec_sdip(h_dec, aec);
        }
        cuType = ((cu_loc_dat->trans_size == 0) && (cuType == I8MB)) ? I16MB : cuType;
        cu->cuType = (char_t) cuType;

        if (cuType == I16MB) {
            read_ipred_block_modes(h_dec, 0, (unsigned int) cu_idx);
            read_ipred_block_modes(h_dec, 4, (unsigned int) cu_idx);
        } else {
            for (i = 0; i < 5; i++) {
                read_ipred_block_modes(h_dec, i, (unsigned int) cu_idx);
            }
        }
    } else {
        if (h_dec->type != B_IMG) {
            init_sub_cu_ref(h_dec);
        } else if (cuType){
            init_sub_cu_ref_B(h_dec, (unsigned int) cu_idx);
        }
    }
    init_sub_cu_pdir(h_dec, (unsigned int) cu_idx);
}

void read_cbp(avs2_dec_t *h_dec)
{
    int cu_idx = h_dec->cu_idx;
    int b8size = h_dec->cu_b8size;
    const seq_info_t *seq = h_dec->seq;
    com_cu_t *cu = h_dec->cu;
    com_cu_local_t *cu_loc_dat = h_dec->cu_loc_dat;
    aec_core_t *aec;
    int fixqp = (h_dec->pic_hdr.fixed_picture_qp || h_dec->slice_hdr.fixed_slice_qp);
    int i, j, k, tu_num;
    int delta_qp = 0;
    int mode = cu->cuType;
    int cu_size = 1 << cu->ui_MbBitSize;
    int tu_b4w, tu_b4h;

    aec = &(h_dec->aec_core);
    cu->cbp = aec_cbp(h_dec, aec); 

    tu_num = cu_loc_dat->trans_size ? 4 : 1;

    if (tu_num == 4) {
        if (h_dec->nsqt_shape == 1) {
            tu_b4w = cu_size /  4;
            tu_b4h = cu_size / 16;
        } else if (h_dec->nsqt_shape == 2) {
            tu_b4w = cu_size / 16;
            tu_b4h = cu_size /  4;
        } else {
            tu_b4w = tu_b4h = cu_size / 8;
        }
    } else {
        tu_b4w = tu_b4h = cu_size / 4;
    }

    for (k = 0; k < tu_num; k++) {
        int tu_b4_x, tu_b4_y;
        char_t *nz;
        char_t nz_val;

        if (tu_b4w == tu_b4h) {
            tu_b4_x = h_dec->cu_b4_x + (k % 2) * tu_b4w;
            tu_b4_y = h_dec->cu_b4_y + (k / 2) * tu_b4h;
        } else if (tu_b4w > tu_b4h) {
            tu_b4_x = h_dec->cu_b4_x;
            tu_b4_y = h_dec->cu_b4_y + k * tu_b4h;
        } else {
            tu_b4_x = h_dec->cu_b4_x + k * tu_b4w;
            tu_b4_y = h_dec->cu_b4_y;
        }
       
        nz = h_dec->nz + tu_b4_y * seq->b4_info_stride + tu_b4_x;
        nz_val = (cu->cbp >> k) & 1;

        for (i = 0; i < tu_b4h; i++) {
            for (j = 0; j < tu_b4w; j++) {
                nz[j] = nz_val;
            }
            nz += seq->b4_info_stride;
        }
    }

    if (!cu->cbp) {
        h_dec->last_dquant = 0;
    }

    if (cu->cbp && !fixqp) {
        delta_qp = aec_delta_qp(h_dec, aec); 
    }
    cu->qp += delta_qp;

    for (i = 0; i < b8size; i++) {
        int pos = cu_idx + i * h_dec->seq->img_width_in_mcu;

        for (j = 0; j < b8size; j++, pos++) {
            com_cu_t *tmpMB = &h_dec->cu_array[pos];
            tmpMB->cbp = cu->cbp;
            tmpMB->qp = cu->qp;
        }
    }
}

void fill_bskip_blk(avs2_dec_t *h_dec, com_cu_t *cu)
{
    int i;
    int cu_idx = h_dec->cu_idx;
    int b8size = h_dec->cu_size / 8;
    int b4size = b8size << 1;
    int sb4size = b8size;
    int cu_bitsize = h_dec->cu_bitsize;
    int i_b4 = h_dec->seq->b4_info_stride;
    char_t *fwd_ref;
    char_t *bwd_ref;
    i16s_t(*fwd_mv)[2];
    i16s_t(*bwd_mv)[2];

    if (h_dec->cu_loc_dat->md_directskip_mode == 0) {
        com_ref_t *col_frm = &h_dec->ref_list[0];
        int i8_1st;
        int j8_1st;
        for (i = 0; i < 4; i++) {
            int i8 = ((cu_idx % h_dec->seq->img_width_in_mcu) << 1) + (i % 2) * sb4size;
            int j8 = ((cu_idx / h_dec->seq->img_width_in_mcu) << 1) + (i / 2) * sb4size;
            int r, c;
            int col_idx = get_colocal_idx(h_dec, i8, j8);
            i16s_t *col_mv = col_frm->frm->mvbuf[col_idx];
            char_t  col_ref = col_frm->frm->refbuf[col_idx];
            i16s_t *fmv, *bmv;

            fwd_mv = h_dec->frm_cur->mvbuf + j8 * i_b4 + i8;
            bwd_mv = h_dec->bw_mv + j8 * i_b4 + i8;
            fmv = fwd_mv[0];
            bmv = bwd_mv[0];

            if (i == 0) {
                i8_1st = i8;
                j8_1st = j8;
            }

            fwd_ref = h_dec->frm_cur->refbuf + j8 * i_b4 + i8;
            bwd_ref = h_dec->bw_ref + j8 * i_b4 + i8;

            
            if (col_ref < 0) {
                for (r = 0; r < sb4size; r++) {
                    memset(fwd_ref, 0, sb4size);
                    memset(bwd_ref, 0, sb4size);
                    fwd_ref += i_b4;
                    bwd_ref += i_b4;
                }

                if (i == 0 || cu_bitsize != MIN_CU_SIZE_IN_BIT) {
                    get_pmv(h_dec, cu_bitsize, cu_idx, fmv, h_dec->frm_cur->refbuf, h_dec->frm_cur->mvbuf, 0, 0, 0, 8 * b8size, 8 * b8size, 0, 1);
                    get_pmv(h_dec, cu_bitsize, cu_idx, bmv, h_dec->bw_ref, h_dec->bw_mv, 0, 0, 0, 8 * b8size, 8 * b8size, -1, 1);
                } else {
                    CP32(fmv, h_dec->frm_cur->mvbuf[j8_1st * i_b4 + i8_1st]);
                    CP32(bmv, h_dec->bw_mv[j8_1st * i_b4 + i8_1st]);
                }

                for (r = 0; r < sb4size; r++) {
                    for (c = 0; c < sb4size; c++) {
                        CP32(fwd_mv[c], fmv);
                        CP32(bwd_mv[c], bmv);
                    }
                    fwd_mv += i_b4;
                    bwd_mv += i_b4;
                }
            } else { // next P is skip or inter mode
                int iTRp, iTRb, iTRd;
                i64s_t frame_no_next_P = 2 * h_dec->imgtr_next_P;
                i64s_t frame_no_B = 2 * h_dec->img_tr_ext;

                iTRp = 2 * (int)(h_dec->imgtr_next_P - col_frm->ref_reftrs[col_ref]);
                iTRd = 2 * (int)(h_dec->imgtr_next_P - h_dec->img_tr_ext);
                iTRb = 2 * (int)(h_dec->imgtr_next_P - h_dec->ref_list[1].ref_imgtr) - iTRd;

                iTRp = COM_ADD_MODE(iTRp, 512);
                iTRd = COM_ADD_MODE(iTRd, 512);
                iTRb = COM_ADD_MODE(iTRb, 512);

                // only MV of block 0 is calculated, block 1/2/3 will copy the MV from block 0
                if (i == 0 || h_dec->cu_bitsize != MIN_CU_SIZE_IN_BIT) {
                    int fval[2], bval[2];
                    if (col_mv[0] < 0) {
                        fval[0] = -(((MULTI / iTRp) * (1 - iTRb * col_mv[0]) - 1) >> OFFSET);
                        bval[0] =  (((MULTI / iTRp) * (1 - iTRd * col_mv[0]) - 1) >> OFFSET);
                    } else {
                        fval[0] =  (((MULTI / iTRp) * (1 + iTRb * col_mv[0]) - 1) >> OFFSET);
                        bval[0] = -(((MULTI / iTRp) * (1 + iTRd * col_mv[0]) - 1) >> OFFSET);
                    }

                    if (h_dec->seq->is_field_sequence) {
                        int delta, delta2, delta_d, delta2_d;
                        i64s_t oriPOC = frame_no_next_P;
                        i64s_t oriRefPOC = frame_no_next_P - iTRp;
                        i64s_t scaledPOC = frame_no_B;
                        i64s_t scaledRefPOC = frame_no_B - iTRb;
                        getDeltas(h_dec->pic_hdr.is_top_field, &delta, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                        scaledRefPOC = frame_no_B - iTRd;
                        getDeltas(h_dec->pic_hdr.is_top_field, &delta_d, &delta2_d, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                        assert(delta == delta_d);

                        if (col_mv[1] + delta < 0) {
                            fval[1] = -(((MULTI / iTRp) * (1 - iTRb * (col_mv[1] + delta)) - 1) >> OFFSET) - delta2;
                            bval[1] =  (((MULTI / iTRp) * (1 - iTRd * (col_mv[1] + delta)) - 1) >> OFFSET) - delta2_d;
                        } else {
                            fval[1] =  (((MULTI / iTRp) * (1 + iTRb * (col_mv[1] + delta)) - 1) >> OFFSET) - delta2;
                            bval[1] = -(((MULTI / iTRp) * (1 + iTRd * (col_mv[1] + delta)) - 1) >> OFFSET) - delta2_d;
                        }
                    } else {
                        if (col_mv[1] < 0) {
                            fval[1] = -(((MULTI / iTRp) * (1 - iTRb * col_mv[1]) - 1) >> OFFSET);
                            bval[1] =  (((MULTI / iTRp) * (1 - iTRd * col_mv[1]) - 1) >> OFFSET);
                        } else {
                            fval[1] =  (((MULTI / iTRp) * (1 + iTRb * col_mv[1]) - 1) >> OFFSET);
                            bval[1] = -(((MULTI / iTRp) * (1 + iTRd * col_mv[1]) - 1) >> OFFSET);
                        }
                    }
                    fmv[0] = Clip3(-32768, 32767, fval[0]);
                    bmv[0] = Clip3(-32768, 32767, bval[0]);
                    fmv[1] = Clip3(-32768, 32767, fval[1]);
                    bmv[1] = Clip3(-32768, 32767, bval[1]);
                } else {
                    CP32(fmv, h_dec->frm_cur->mvbuf[j8_1st * i_b4 + i8_1st]);
                    CP32(bmv, h_dec->bw_mv[j8_1st * i_b4 + i8_1st]);
                }

                for (r = 0; r < sb4size; r++) {
                    for (c = 0; c < sb4size; c++) {
                        CP32(fwd_mv[c], fmv);
                        CP32(bwd_mv[c], bmv);
                        fwd_ref[c] = 0;
                        bwd_ref[c] = 0;
                    }
                    fwd_ref += i_b4;
                    bwd_ref += i_b4;
                    fwd_mv  += i_b4;
                    bwd_mv  += i_b4;
                }
            }
        }
    } else {
        int tmp_fwd_ref, tmp_bwd_ref;
        int i8 = ((cu_idx % h_dec->seq->img_width_in_mcu) << 1);
        int j8 = ((cu_idx / h_dec->seq->img_width_in_mcu) << 1);
        int r, c;
        i16s_t *fmv, *bmv;

        fwd_mv = h_dec->frm_cur->mvbuf + j8 * i_b4 + i8;
        bwd_mv = h_dec->bw_mv + j8 * i_b4 + i8;
        fmv = fwd_mv[0];
        bmv = bwd_mv[0];

        fwd_ref = h_dec->frm_cur->refbuf + j8 * i_b4 + i8;
        bwd_ref = h_dec->bw_ref + j8 * i_b4 + i8;

        switch (h_dec->cu_loc_dat->md_directskip_mode) {
        case DS_SYM:
        case DS_BID:
            tmp_fwd_ref = 0;
            tmp_bwd_ref = 0;
            break;
        case DS_BACKWARD:
            tmp_fwd_ref = -1;
            tmp_bwd_ref = 0;
            break;
        case DS_FORWARD:
            tmp_fwd_ref = 0;
            tmp_bwd_ref = -1;
            break;
        }

        get_pmv_bskip(h_dec, cu_idx, fmv, bmv);

        for (r = 0; r < b4size; r++) {
            for (c = 0; c < b4size; c++) {
                CP32(fwd_mv[c], fmv);
                CP32(bwd_mv[c], bmv);
                fwd_ref[c] = (char_t) tmp_fwd_ref;
                bwd_ref[c] = (char_t) tmp_bwd_ref;
            }
            fwd_ref += i_b4;
            bwd_ref += i_b4;
            fwd_mv += i_b4;
            bwd_mv += i_b4;
        }
    }
}

static void fill_pskip_blk(avs2_dec_t *h_dec, com_cu_t *cu)
{
    int cu_idx = h_dec->cu_idx;
    int b8size = h_dec->cu_b8size;
    int mb_y = cu_idx / h_dec->seq->img_width_in_mcu;
    int mb_x = cu_idx % h_dec->seq->img_width_in_mcu;
    int block8_y = mb_y << 1;
    int block8_x = mb_x << 1;
    int i_b4 = h_dec->seq->b4_info_stride;
    i16s_t(*fw_mv)[2] = h_dec->frm_cur->mvbuf + block8_y * i_b4 + block8_x;
    i16s_t(*bw_mv)[2] = h_dec->bw_mv + block8_y * i_b4 + block8_x;
    int i, j;
    int delta[4];
    int tmv[2];
    com_cu_local_t *cu_loc_dat = h_dec->cu_loc_dat;
    int md_directskip_mode = cu_loc_dat->md_directskip_mode;
    int weighted_skipmode  = cu_loc_dat->weighted_skipmode;

    if (md_directskip_mode == 0) {
        PskipMV_COL(h_dec, cu);
        
        for (j = 0; j < 2 * b8size; j++) {
            for (i = 0; i < 2 * b8size; i++) {
                if (weighted_skipmode != 0) {
                    int m;
                    for (m = 0; m < h_dec->i_refs; m++) {
                        delta[m] = h_dec->ref_list[m].dist[0];
                        if (h_dec->pic_hdr.background_reference_enable && m == h_dec->i_refs - 1) {
                            delta[m] = 1;
                        }
                    }
                    
                    h_dec->snd_ref[(block8_y + j) * i_b4 + block8_x + i] = (char_t) weighted_skipmode;

                    tmv[0] = (int)((delta[weighted_skipmode] * fw_mv[i][0] * (MULTI / delta[0]) + HALF_MULTI) >> OFFSET);
                    
                    if (h_dec->seq->is_field_sequence) {
                        int deltaT, delta2;
                        i64s_t oriPOC = 2 * h_dec->img_tr_ext;
                        i64s_t oriRefPOC = oriPOC - delta[0];
                        i64s_t scaledPOC = 2 * h_dec->img_tr_ext;
                        i64s_t scaledRefPOC = scaledPOC - delta[weighted_skipmode];
                        getDeltas(h_dec->pic_hdr.is_top_field, &deltaT, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                        tmv[1] = (int)(((delta[weighted_skipmode] * (fw_mv[i][1] + deltaT) * (16384 / delta[0]) + 8192) >> 14) - delta2);
                    } else {
                        tmv[1] = (int)((delta[weighted_skipmode] * fw_mv[i][1] * (MULTI / delta[0]) + HALF_MULTI) >> OFFSET);
					}

                    bw_mv[i][0] = (i16s_t) Clip3(-32768, 32767, tmv[0]);
                    bw_mv[i][1] = (i16s_t) Clip3(-32768, 32767, tmv[1]);
                } else {
                    h_dec->snd_ref[(block8_y + j) * i_b4 + block8_x + i] = -1;
                    M32(bw_mv[i]) = 0;
                }
            }
            fw_mv += i_b4;
            bw_mv += i_b4;
        }
    } else {
        if (md_directskip_mode == BID_P_FST || md_directskip_mode == BID_P_SND || md_directskip_mode == FW_P_FST || md_directskip_mode == FW_P_SND) {
            get_pmv_pskip(h_dec, cu_idx, b8size);
        }
    }
}


/*
*************************************************************************
* Function:decode one com_cu_t
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
int cu_reconstruct(com_rec_t *rec)
{
    com_cu_t *cu = rec->cu;
    int cu_idx = rec->cu_idx;
    int b8size = rec->cu_b8size;
    int uv, i, j;
    int block8Nx8N;
    int cu_bitsize = rec->cu_bitsize;
    int cuType = cu->cuType;
    int cu_zig_offset = rec->cu_zig_offset;
    int cu_zig_size = rec->cu_b8num * 64;
    com_lcu_t *lcu = rec->lcu;
    coef_t *coef_y = lcu->coef_luma + 64 * cu_zig_offset;
    coef_t *coef_uv[2];
    coef_uv[0] = lcu->coef_chroma[0] + 16 * cu_zig_offset;
    coef_uv[1] = lcu->coef_chroma[1] + 16 * cu_zig_offset;

    if (rec->cu_loc_dat->trans_size == 0) {
        if (cuType != I16MB) {
            dec_inter_pred(rec);
            dec_add_idct(rec, 0, cu->qp, coef_y, cu_bitsize);
        } else {
            dec_intra_pred(rec, (rec->cu_b8_x << MIN_CU_SIZE_IN_BIT), (rec->cu_b8_y << MIN_CU_SIZE_IN_BIT), cu_bitsize);
            dec_add_idct(rec, 0, cu->qp, coef_y, cu_bitsize);
        }
    } else {
        if (cuType < I8MB) {
            dec_inter_pred(rec);

            for (block8Nx8N = 0; block8Nx8N < 4; block8Nx8N++) {
                if (rec->seq->useNSQT && cu_bitsize > B8X8_IN_BIT && (cuType == P2NXN || cuType == PHOR_UP || cuType == PHOR_DOWN || cuType == PNX2N || cuType == PVER_LEFT || cuType == PVER_RIGHT)) {
                    dec_add_idct_nsqt(rec, block8Nx8N, coef_y, cu_bitsize - 1);
                } else {
                    dec_add_idct(rec, block8Nx8N, cu->qp, coef_y, cu_bitsize - 1);
                }
                coef_y += cu_zig_size >> 2;
            }
        } else {
            for (block8Nx8N = 0; block8Nx8N < 4; block8Nx8N++) {
                if (cuType == InNxNMB) {
                    dec_intra_pred(rec, (rec->cu_b8_x << MIN_CU_SIZE_IN_BIT), (rec->cu_b8_y << MIN_CU_SIZE_IN_BIT) + block8Nx8N * (1 << (cu_bitsize - 2)), cu_bitsize);
                    dec_add_idct_nsqt(rec, block8Nx8N, coef_y, cu_bitsize - 1);
                } else if (cuType == INxnNMB) {
                    dec_intra_pred(rec, (rec->cu_b8_x << MIN_CU_SIZE_IN_BIT) + block8Nx8N * (1 << (cu_bitsize - 2)), (rec->cu_b8_y << MIN_CU_SIZE_IN_BIT), cu_bitsize);
                    dec_add_idct_nsqt(rec, block8Nx8N, coef_y, cu_bitsize - 1);
                } else {
                    dec_intra_pred(rec, (rec->cu_b8_x << MIN_CU_SIZE_IN_BIT) + ((block8Nx8N & 1) << MIN_BLOCK_SIZE_IN_BIT) * b8size, (rec->cu_b8_y << MIN_CU_SIZE_IN_BIT) + ((block8Nx8N & 2) << (MIN_BLOCK_SIZE_IN_BIT - 1)) * b8size, cu_bitsize - 1);
                    dec_add_idct(rec, block8Nx8N, cu->qp, coef_y, cu_bitsize - 1);
                }
                coef_y += cu_zig_size >> 2;
            }
        }
    }

    for (uv = 0; uv < 2; uv++) {
        int qp_shift = (8 * (rec->seq->sample_bit_depth - 8));
        int qp_chroma = tab_qp_scale_cr[Clip3(0, 63, (cu->qp - qp_shift))] + qp_shift;

        if (IS_INTRA(cu)) {
            dec_intra_pred_chroma(rec, uv);
        } else {
            dec_inter_pred_chroma(rec, uv);
        }
        dec_add_idct(rec, 4 + uv, qp_chroma, coef_uv[uv], cu_bitsize - 1);
    }


    for (i = 0; i < b8size; i++) {
        for (j = 0; j < b8size; j++) {
            cu[j].b_decoded = 1;
        }
        cu += rec->seq->img_width_in_mcu;
    }

    return 0;
}


int cu_decode(avs2_dec_t* h_dec, unsigned int uiBitSize, unsigned int cu_idx)
{
    int split_flag;
    const seq_info_t *seq = h_dec->seq;

    int cu_size = 1 << uiBitSize;
    int cu_b8_x = cu_idx % seq->img_width_in_mcu;
    int cu_b8_y = cu_idx / seq->img_width_in_mcu;
    int cu_pix_x = cu_b8_x * 8;
    int cu_pix_y = cu_b8_y * 8;
    int cu_pix_x_end = cu_pix_x + cu_size;
    int cu_pix_y_end = cu_pix_y + cu_size;
    int cu_zig_offset = tab_b8_xy_cvto_zigzag[cu_b8_y - h_dec->lcu_b8_y][cu_b8_x - h_dec->lcu_b8_x];
    com_cu_local_t *cu_local_dat = &h_dec->lcu->cus[cu_zig_offset];

    if (uiBitSize == MIN_CU_SIZE_IN_BIT) { 
        split_flag = 0;
    } else if (cu_pix_x_end <= h_dec->seq->img_width && cu_pix_y_end <= h_dec->seq->img_height) {
        split_flag = aec_cu_split_flag(h_dec, uiBitSize);
    } else {
        split_flag = 1;
    }

    if (split_flag) {
        int i;
        int sub_cu_size = cu_size / 2;

        for (i = 0; i < 4; i++) {
            int sub_cu_x = cu_pix_x + (i % 2) * sub_cu_size;
            int sub_cu_y = cu_pix_y + (i / 2) * sub_cu_size;

            if (sub_cu_x < seq->img_width && sub_cu_y < seq->img_height) {
                int pos = sub_cu_y / 8 * seq->img_width_in_mcu + sub_cu_x / 8;
                cu_decode(h_dec, uiBitSize - 1, pos);
            }
        }
        return DECODE_MB;
    } else {
        int i, j;
        int real_cuType;
        int b8size = cu_size / 8;
        com_cu_t *cu = &h_dec->cu_array[cu_idx];
        int mode;

        /* init cu info */
        h_dec->cu            = cu;
        h_dec->cu_idx        = cu_idx;
        h_dec->cu_bitsize    = uiBitSize;
        h_dec->cu_size       = cu_size;
        h_dec->cu_b8size     = b8size;
        h_dec->cu_b8num      = b8size * b8size;
        h_dec->cu_b8_x       = cu_b8_x;
        h_dec->cu_b8_y       = cu_b8_y;
        h_dec->cu_pix_x      = cu_pix_x;
        h_dec->cu_pix_y      = cu_pix_y;
        h_dec->cu_pix_c_x    = cu_pix_x / 2;
        h_dec->cu_pix_c_y    = cu_pix_y / 2;
        h_dec->cu_b4_x       = cu_b8_x * 2;
        h_dec->cu_b4_y       = cu_b8_y * 2;
        h_dec->cu_zig_offset = cu_zig_offset;
        h_dec->cu_loc_dat    = cu_local_dat;

        /* start read */
        init_cu(h_dec);

        read_cu_hdr(h_dec, &real_cuType);

        mode = cu->cuType;

        if (mode == InNxNMB || mode == PHOR_UP || mode == PHOR_DOWN || (mode == P2NXN && seq->useNSQT && cu->ui_MbBitSize != B8X8_IN_BIT)) {
            h_dec->nsqt_shape = 1;
        } else if (mode == INxnNMB || mode == PVER_LEFT || mode == PVER_RIGHT || (mode == PNX2N && seq->useNSQT && cu->ui_MbBitSize != B8X8_IN_BIT)) {
            h_dec->nsqt_shape = 2;
        } else {
            h_dec->nsqt_shape = 0;
        }

        if (h_dec->cu_pix_x > 0 && cu->slice_nr == cu[-1].slice_nr) {
            cu->qp = cu[-1].qp;
        } else {
            cu->qp = h_dec->slice_hdr.slice_qp;
        }

        if (real_cuType < 0) {
            if (IS_B_SKIP(cu)) {
                fill_bskip_blk(h_dec, cu);
            } else {
                fill_pskip_blk(h_dec, cu);
            }

            for (i = 0; i < b8size; i++) {
                int pos = cu_idx + i * h_dec->seq->img_width_in_mcu;
                for (j = 0; j < b8size; j++, pos++) {
                    com_cu_t *tmpMB = &h_dec->cu_array[pos];
                    tmpMB->qp = cu->qp;
                    tmpMB->cbp = 0;
                }
            }
            h_dec->last_dquant = 0;
        } else {
            if (!IS_INTRA(cu) && h_dec->typeb != BP_IMG) {
                if (cu->cuType) {
                    read_ref(h_dec);
                    read_mv(h_dec);
                } else {
                    if (h_dec->type == B_IMG) {
                        fill_bskip_blk(h_dec, cu);
                    } else {
                        fill_pskip_blk(h_dec, cu);
                    }
                }
            }

            read_cbp(h_dec);
            read_coeffs(h_dec);
        }

        if (!h_dec->pic_hdr.loop_filter_disable) {
            deblock_set_cu_edge(h_dec, uiBitSize, cu_idx);
        }

        if (!h_dec->i_rec_threads) {
            com_rec_t *rec = h_dec->rec;

            rec->cu            = cu;
            rec->cu_idx        = cu_idx;
            rec->cu_bitsize    = uiBitSize;
            rec->cu_size       = cu_size;
            rec->cu_b8size     = b8size;
            rec->cu_b8num      = b8size * b8size;
            rec->cu_b8_x       = cu_b8_x;
            rec->cu_b8_y       = cu_b8_y;
            rec->cu_pix_x      = cu_pix_x;
            rec->cu_pix_y      = cu_pix_y;
            rec->cu_pix_c_x    = cu_pix_x / 2;
            rec->cu_pix_c_y    = cu_pix_y / 2;
            rec->cu_b4_x       = cu_b8_x * 2;
            rec->cu_b4_y       = cu_b8_y * 2;
            rec->cu_zig_offset = cu_zig_offset;
            rec->cu_loc_dat    = cu_local_dat;

            cu_reconstruct(rec);
        }
    }

    return DECODE_MB;
}

int cu_decode_reconstruct(com_rec_t *rec, unsigned int uiBitSize, unsigned int cu_idx)
{
    const seq_info_t *seq = rec->seq;

    int cu_size = 1 << uiBitSize;
    int cu_b8size = cu_size / 8;
    int cu_b8_x = cu_idx % seq->img_width_in_mcu;
    int cu_b8_y = cu_idx / seq->img_width_in_mcu;
    int cu_pix_x = cu_b8_x * 8;
    int cu_pix_y = cu_b8_y * 8;
    int cu_pix_x_end = cu_pix_x + cu_size;
    int cu_pix_y_end = cu_pix_y + cu_size;
    int cu_zig_offset = tab_b8_xy_cvto_zigzag[cu_b8_y - rec->lcu_b8_y][cu_b8_x - rec->lcu_b8_x];
    com_cu_local_t *cu_local_dat = &rec->lcu->cus[cu_zig_offset];

    if (uiBitSize > rec->cu_array[cu_idx].ui_MbBitSize) {
        int i;
        int sub_cu_size = cu_size / 2;

        for (i = 0; i < 4; i++) {
            int sub_cu_x = cu_pix_x + (i % 2) * sub_cu_size;
            int sub_cu_y = cu_pix_y + (i / 2) * sub_cu_size;

            if (sub_cu_x < seq->img_width && sub_cu_y < seq->img_height) {
                int pos = sub_cu_y / 8 * seq->img_width_in_mcu + sub_cu_x / 8;
                cu_decode_reconstruct(rec, uiBitSize - 1, pos);
            }
        }
        return DECODE_MB;
    } else {
        /* init cu info */
        rec->cu            = &rec->cu_array[cu_idx];
        rec->cu_idx        = cu_idx;
        rec->cu_bitsize    = uiBitSize;
        rec->cu_size       = cu_size;
        rec->cu_b8size     = cu_b8size;
        rec->cu_b8num      = cu_b8size * cu_b8size;
        rec->cu_b8_x       = cu_b8_x;
        rec->cu_b8_y       = cu_b8_y;
        rec->cu_pix_x      = cu_pix_x;
        rec->cu_pix_y      = cu_pix_y;
        rec->cu_pix_c_x    = cu_pix_x / 2;
        rec->cu_pix_c_y    = cu_pix_y / 2;
        rec->cu_b4_x       = cu_b8_x * 2;
        rec->cu_b4_y       = cu_b8_y * 2;
        rec->cu_zig_offset = cu_zig_offset;
        rec->cu_loc_dat    = cu_local_dat;

        cu_reconstruct(rec);
    }

    return DECODE_MB;
}

/*
*************************************************************************
* Function:Set context for reference frames
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/

int BType2CtxRef(int btype)
{
    if (btype < 4) {
        return 0;
    } else {
        return 1;
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
void read_ref(avs2_dec_t *h_dec)
{
    int k;
    int cu_idx = h_dec->cu_idx;
    int cu_bitsize = h_dec->cu_bitsize;

    int cuType  = h_dec->cu->cuType;
    int step_h0 = tab_block_step[cuType][0];
    int step_v0 = tab_block_step[cuType][1];

    int i0, j0, refframe;

    aec_core_t *aec;

    int r, c;
    int b4x = h_dec->cu_b8_x << 1;
    int b4y = h_dec->cu_b8_y << 1;
    int i_b4 = h_dec->seq->b4_info_stride;
    char_t *fwd_ref;
    char_t *bwd_ref;
    char_t *b8pdir = h_dec->cu_loc_dat->b8pdir;

    aec = &h_dec->aec_core;

    for (j0 = 0; j0 < 2;) {
        for (i0 = 0; i0 < 2;) {
            int b4_x, b4_y;
            k = 2 * j0 + i0;
            if (b8pdir[k] != BACKWARD) {
                int start_x, start_y, tmp_step_h, tmp_step_v;
                get_sub_blk_offset(cuType, cu_bitsize, i0, j0, &start_x, &start_y, &tmp_step_h, &tmp_step_v);

                if (h_dec->i_refs > 1 && (h_dec->type == F_IMG || h_dec->type == P_IMG)) {
                    refframe = aec_ref_index(h_dec, aec, start_x, start_y);
                } else {
                    refframe = 0;
                }

                b4_x = b4x + start_x;
                b4_y = b4y + start_y;

                fwd_ref = h_dec->frm_cur->refbuf + b4_y * i_b4 + b4_x;

                for (r = 0; r < step_v0 * tmp_step_v; r++) {
                    for (c = 0; c < step_h0 * tmp_step_h; c++) {
                        fwd_ref[c] = (char_t) refframe;
                    }
                    fwd_ref += i_b4;
                }
            }
     
            i0 += max(1, step_h0);
        }

        j0 += max(1, step_v0);
    }

    for (j0 = 0; j0 < 2;) {
        for (i0 = 0; i0 < 2;) {
            int b4_x, b4_y;
            int start_x, start_y, tmp_step_h, tmp_step_v;

            k = 2 * j0 + i0;

            get_sub_blk_offset(cuType, cu_bitsize, i0, j0, &start_x, &start_y, &tmp_step_h, &tmp_step_v);
            b4_x = b4x + start_x;
            b4_y = b4y + start_y;

            if (b8pdir[k] == DUAL) {
                fwd_ref = h_dec->frm_cur->refbuf + b4_y * i_b4 + b4_x;
                bwd_ref = h_dec->snd_ref + b4_y * i_b4 + b4_x;
                for (r = 0; r < step_v0 * tmp_step_v; r++) {
                    for (c = 0; c < step_h0 * tmp_step_h; c++) {
                        bwd_ref[c] = (char_t) (fwd_ref[c] == 0 ? 1 : 0);
                    }
                    fwd_ref += i_b4;
                    bwd_ref += i_b4;
                }
            } 

            i0 += max(1, step_h0);
        }

        j0 += max(1, step_v0);
    }
}

static int pmvr_sign(int val)
{
    if (val > 0) {
        return 1;
    } else if (val < 0) {
        return -1;
    } else {
        return 0;
    }
}

void pmvr_mv_derivation(i16s_t mv[2], i16s_t mvd[2], i16s_t mvp[2])
{
    int ctrd[2];
    int tmv[2];
    ctrd[0] = ((mvp[0] >> 1) << 1) - mvp[0];
    ctrd[1] = ((mvp[1] >> 1) << 1) - mvp[1];
    if (abs(mvd[0] - ctrd[0]) > TH) {
        tmv[0] = mvp[0] + (mvd[0] << 1) - ctrd[0] - pmvr_sign(mvd[0] - ctrd[0]) * TH;
        tmv[1] = mvp[1] + (mvd[1] << 1) + ctrd[1];
    } else if (abs(mvd[1] - ctrd[1]) > TH) {
        tmv[0] = mvp[0] + (mvd[0] << 1) + ctrd[0];
        tmv[1] = mvp[1] + (mvd[1] << 1) - ctrd[1] - pmvr_sign(mvd[1] - ctrd[1]) * TH;
    } else {
        tmv[0] = mvd[0] + mvp[0];
        tmv[1] = mvd[1] + mvp[1];
    }

    mv[0] = (i16s_t) Clip3(-32768, 32767, tmv[0]);
    mv[1] = (i16s_t) Clip3(-32768, 32767, tmv[1]);
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
void read_mv(avs2_dec_t *h_dec)
{
    int cu_idx = h_dec->cu_idx;
    int cu_bitsize = h_dec->cu_bitsize;
    int b8size = h_dec->cu_b8size;
    int b4size = b8size << 1;
    int sb4size = b4size >> 1;
    int k;
    int mvd[2];

    int cuType = h_dec->cu->cuType;
    int step_h0 = tab_block_step[cuType][0];
    int step_v0 = tab_block_step[cuType][1];
    aec_core_t *aec = &h_dec->aec_core;

    int i0, j0;
    ALIGNED_4(i16s_t pmv[2]);
    int j8, i8, ii, jj;

    int block8_y = (cu_idx / h_dec->seq->img_width_in_mcu) << 1;
    int block8_x = (cu_idx % h_dec->seq->img_width_in_mcu) << 1;

    int size = 1 << h_dec->cu_bitsize;
    int pix_x = (cu_idx % h_dec->seq->img_width_in_mcu) << MIN_CU_SIZE_IN_BIT;
    int pix_y = (cu_idx / h_dec->seq->img_width_in_mcu) << MIN_CU_SIZE_IN_BIT;
    int start_x, start_y, step_h, step_v;
    ALIGNED_4(i16s_t pmvr_mvd[2]);
    ALIGNED_4(i16s_t pmvr_mv[2]);
    int i_b4 = h_dec->seq->b4_info_stride;

    i16s_t(*bw_mv)[2];
    i16s_t(*fw_mv)[2];

    char_t *b8pdir = h_dec->cu_loc_dat->b8pdir;

    /* read dmh mode */
    if (h_dec->type == F_IMG && (b8pdir[0] + b8pdir[1] + b8pdir[2] + b8pdir[3]) == 0) {
        if (cuType && (cu_bitsize != B8X8_IN_BIT || cuType == P2NX2N)) {
            h_dec->cu_loc_dat->dmh_mode = (char_t) aec_dmh_mode(aec, cu_bitsize);
        }
    }

    /* READ FORWARD MOTION VECTORS */
    for (j0 = 0; j0 < 2;) {
        for (i0 = 0; i0 < 2;) {
            k = 2 * j0 + i0;
            if (b8pdir[k] == FORWARD || b8pdir[k] == SYM || b8pdir[k] == BID || b8pdir[k] == DUAL) {     //has forward vector
                int pix_start_x, pix_start_y, pix_step_h, pix_step_v;
                get_sub_blk_offset(cuType, cu_bitsize, i0, j0, &start_x, &start_y, &step_h, &step_v);

                j8 = block8_y + start_y;
                i8 = block8_x + start_x;

                // first make mv-prediction
                get_pu_pix_info(cuType, cu_bitsize, k, &pix_start_x, &pix_start_y, &pix_step_h, &pix_step_v);
                get_pmv(h_dec, cu_bitsize, cu_idx, pmv, h_dec->frm_cur->refbuf, h_dec->frm_cur->mvbuf, h_dec->frm_cur->refbuf[j8 *i_b4 + i8], pix_start_x, pix_start_y, pix_step_h, pix_step_v, 0, 0);    //Lou 1016

                if (h_dec->typeb != BP_IMG) {  //no mvd for S frame, just set it to 0
                    pmvr_mvd[0] = (i16s_t) aec_mvd(h_dec, aec, 0);
                    pmvr_mvd[1] = (i16s_t) aec_mvd(h_dec, aec, 1);
                } else {
                    M32(pmvr_mvd) = 0;
                }

                if (h_dec->seq->b_pmvr_enabled) {
                    pmvr_mv_derivation(pmvr_mv, pmvr_mvd, pmv);
                } else {
                    pmvr_mv[0] = pmvr_mvd[0] + pmv[0];
                    pmvr_mv[1] = pmvr_mvd[1] + pmv[1];
                    pmvr_mv[0] = (i16s_t) Clip3(-32768, 32767, pmvr_mv[0]);
                    pmvr_mv[1] = (i16s_t) Clip3(-32768, 32767, pmvr_mv[1]);
                }

                fw_mv = h_dec->frm_cur->mvbuf + j8 * i_b4 + i8;

                for (jj = 0; jj < step_v0 * step_v; jj++) {
                    for (ii = 0; ii < step_h0 * step_h; ii++) {
                        CP32(fw_mv[ii], pmvr_mv);
                    }
                    fw_mv += i_b4;
                }
            }
            i0 += max(1, step_h0);
        }
        j0 += max(1, step_v0);
    }


    //=====  READ BACKWARD MOTION VECTORS =====

    for (j0 = 0; j0 < 2;) {
        for (i0 = 0; i0 < 2;) {
            k = 2 * j0 + i0;

            if (b8pdir[k] == BACKWARD || b8pdir[k] == SYM || b8pdir[k] == BID || b8pdir[k] == DUAL) {     //has backward vector
                i16s_t *fmv;
                get_sub_blk_offset(cuType, cu_bitsize, i0, j0, &start_x, &start_y, &step_h, &step_v);

                j8 = block8_y + start_y ;
                i8 = block8_x + start_x ;

                fmv = h_dec->frm_cur->mvbuf[j8 * i_b4 + i8];
                
                if (h_dec->type == B_IMG) {
                    int pix_start_x, pix_start_y, pix_step_h, pix_step_v;
                    get_pu_pix_info(cuType, cu_bitsize, k, &pix_start_x, &pix_start_y, &pix_step_h, &pix_step_v);
                    get_pmv(h_dec, cu_bitsize, cu_idx, pmv, h_dec->bw_ref, h_dec->bw_mv, h_dec->bw_ref[j8 *i_b4 + i8], pix_start_x, pix_start_y, pix_step_h, pix_step_v, -1, 0);    //Lou 1016
                }

                
                if (b8pdir[2 * j0 + i0] == SYM) {
                        com_frm_t *col_frm = h_dec->ref_list[0].frm;

                        int DistanceIndexFw, DistanceIndexBw;
                        int refframe = h_dec->frm_cur->refbuf[j8 * i_b4 + i8];

                        DistanceIndexFw = h_dec->ref_list[0].dist[0];
                        DistanceIndexBw = (int) COM_ADD_MODE((refframe + 1) * 2 * (h_dec->imgtr_next_P - h_dec->ref_list[1].ref_imgtr) - DistanceIndexFw, 512);

                        mvd[0] = -((fmv[0] * DistanceIndexBw * (MULTI / DistanceIndexFw) + HALF_MULTI) >> OFFSET);
                        
                        if (h_dec->seq->is_field_sequence) {
                            int delta, delta2;
                            i64s_t oriPOC, oriRefPOC, scaledPOC, scaledRefPOC;
                            oriPOC = 2 * h_dec->img_tr_ext;
                            oriRefPOC = oriPOC - DistanceIndexFw;
                            scaledPOC = 2 * h_dec->img_tr_ext;
                            scaledRefPOC = scaledPOC - DistanceIndexBw;
                            getDeltas(h_dec->pic_hdr.is_top_field, &delta, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                            mvd[1] = -(((fmv[1] + delta) * DistanceIndexBw * (MULTI / DistanceIndexFw) + HALF_MULTI) >> OFFSET);
                            mvd[1] -= delta2;
                        } else {
                            mvd[1] = -((fmv[1] * DistanceIndexBw * (MULTI / DistanceIndexFw) + HALF_MULTI) >> OFFSET);
                        }

                        pmvr_mv[0] = (i16s_t) Clip3(-32768, 32767, mvd[0]);
                        pmvr_mv[1] = (i16s_t) Clip3(-32768, 32767, mvd[1]);
                        mvd[0] = mvd[0] - pmv[0];
                        mvd[1] = mvd[1] - pmv[1];
                        pmvr_mvd[0] = (i16s_t) Clip3(-32768, 32767, mvd[0]);
                        pmvr_mvd[1] = (i16s_t) Clip3(-32768, 32767, mvd[1]);
                } else if (b8pdir[2 * j0 + i0] == DUAL) {
                        int DistanceIndexFw, DistanceIndexBw;
                        int fw_refframe = h_dec->frm_cur->refbuf[j8* i_b4 + i8];
                        int refframe = h_dec->snd_ref[j8* i_b4 + i8];

                        DistanceIndexFw = h_dec->ref_list[fw_refframe].dist[0];
                        DistanceIndexBw = h_dec->ref_list[refframe].dist[0];

                        if (h_dec->pic_hdr.background_reference_enable) {
                            if (fw_refframe == h_dec->i_refs - 1) {
                                DistanceIndexFw = 1;
                            }
                            if (refframe == h_dec->i_refs - 1) {
                                DistanceIndexBw = 1;
                            }
                        }

                        mvd[0] = (fmv[0] * DistanceIndexBw * (MULTI / DistanceIndexFw) + HALF_MULTI) >> OFFSET;
                        
                        if (h_dec->seq->is_field_sequence) {
                            int delta, delta2;
                            i64s_t oriPOC, oriRefPOC, scaledPOC, scaledRefPOC;
                            oriPOC = 2 * h_dec->img_tr_ext;
                            oriRefPOC = oriPOC - DistanceIndexFw;
                            scaledPOC = 2 * h_dec->img_tr_ext;
                            scaledRefPOC = scaledPOC - DistanceIndexBw;
                            getDeltas(h_dec->pic_hdr.is_top_field, &delta, &delta2, oriPOC, oriRefPOC, scaledPOC, scaledRefPOC);
                            mvd[1] = ((fmv[1] + delta) * DistanceIndexBw * (MULTI / DistanceIndexFw) + HALF_MULTI) >> OFFSET;
                            mvd[1] -= delta2;
                        } else {
                            mvd[1] = (fmv[1] * DistanceIndexBw * (MULTI / DistanceIndexFw) + HALF_MULTI) >> OFFSET;
                        }

                        pmvr_mv[0] = (i16s_t) Clip3(-32768, 32767, mvd[0]);
                        pmvr_mv[1] = (i16s_t) Clip3(-32768, 32767, mvd[1]);
                        mvd[0] = mvd[0] - pmv[0];
                        mvd[1] = mvd[1] - pmv[1];
                        pmvr_mvd[0] = (i16s_t) Clip3(-32768, 32767, mvd[0]);
                        pmvr_mvd[1] = (i16s_t) Clip3(-32768, 32767, mvd[1]);
         
                } else {
                    mvd[0] = aec_mvd(h_dec, aec, 0);
                    mvd[1] = aec_mvd(h_dec, aec, 1);
                    pmvr_mv[0] = (i16s_t) (mvd[0] + pmv[0]);
                    pmvr_mv[1] = (i16s_t) (mvd[1] + pmv[1]);
                    pmvr_mvd[0] = (i16s_t) Clip3(-32768, 32767, mvd[0]);
                    pmvr_mvd[1] = (i16s_t) Clip3(-32768, 32767, mvd[1]);
                }

                if (h_dec->seq->b_pmvr_enabled && b8pdir[2 * j0 + i0] != SYM && b8pdir[2 * j0 + i0] != DUAL) {
                    pmvr_mv_derivation(pmvr_mv, pmvr_mvd, pmv);
                }


                bw_mv = h_dec->bw_mv + j8 * i_b4 + i8;
                

                for (jj = 0; jj < step_v0 * step_v; jj++) {
                    for (ii = 0; ii < step_h0 * step_h; ii++) {
                        CP32(bw_mv[ii], pmvr_mv);
                    }
                    bw_mv += i_b4;
                }
            }

            i0 += max(1, step_h0);
        }

        j0 += max(1, step_v0);

    }
}


/*
*************************************************************************
* Function:Get coded block pattern and coefficients (run/level)
from the bitstream
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
void read_coeffs(avs2_dec_t *h_dec)
{
    int i, j;
    com_cu_t *cu = h_dec->cu;
    int b8;
    int cu_bitsize = h_dec->cu_bitsize;
    int cuType = cu->cuType;
    int bit_size;
    int blk_size;
    
    int level, run, coef_ctr, k, i0, j0, uv, qp;
    int shift, add, QPI, sum;
    aec_core_t *aec = &h_dec->aec_core;

    tab_i16s_t (*SCAN)[2];
    int bsize_x, bsize_y;
    
    int dct_pairs;

    // Adaptive frequency weighting quantization
    int WQMSizeId;
    int WQMSize;
    const int wqm_shift = 2;

#define wq_get_weight(x,y) (h_dec->cur_wq_matrix[WQMSizeId][(y) * WQMSize + (x)])

    int DCT_CGFlag[CG_SIZE * CG_SIZE];
    int DCT_PairsInCG[CG_SIZE * CG_SIZE];
    int DCT_CGNum;

    extern int tab_intraModeClassified[NUM_INTRA_PMODE];

    int dct_levels[32 * 32 + 1];
    int dct_runs[32 * 32 + 1];

    int iCG = 0;
    int pairs = 0;
    
    int iVer = 0, iHor = 0;

    int cu_zig_offset = h_dec->cu_zig_offset;
    int cu_zig_size = h_dec->cu_b8num * 64;
    coef_t *coef_y = h_dec->lcu->coef_luma + 64 * cu_zig_offset;
    coef_t *coef_uv[2];

    com_cu_local_t *cu_loc_dat = h_dec->cu_loc_dat;

    coef_uv[0] = h_dec->lcu->coef_chroma[0] + 16 * cu_zig_offset;
    coef_uv[1] = h_dec->lcu->coef_chroma[1] + 16 * cu_zig_offset;

    if (cu_loc_dat->trans_size == 1) {
        if (h_dec->seq->useNSQT && cu_bitsize > B8X8_IN_BIT && (cuType == P2NXN || cuType == PHOR_UP || cuType == PHOR_DOWN)) {
            iHor = 1;
        } else if (h_dec->seq->useNSQT && cu_bitsize > B8X8_IN_BIT && (cuType == PNX2N || cuType == PVER_LEFT || cuType == PVER_RIGHT)) {
            iVer = 1;
        } else if (h_dec->seq->useSDIP && (cuType == InNxNMB)) { //add yuqh 20130825
            iHor = 1;
        } else if (h_dec->seq->useSDIP &&(cuType == INxnNMB)) { //add yuqh 20130825
            iVer = 1;
        }
    }

    qp  = cu->qp;

    // luma coefficients
    if (cuType == I16MB || cu_loc_dat->trans_size == 0) {
        if (cu->cbp & 0xF) {
            int swap = (IS_INTRA(cu) && tab_intraModeClassified[cu_loc_dat->intra_pred_modes[0]] == INTRA_PRED_HOR);

            SCAN = tab_scan_list_cg[min(3, cu_bitsize - 2)];
            bsize_x = min(32, 1 << cu_bitsize);
            bsize_y = bsize_x;
            WQMSizeId = min(3, cu_bitsize - 2);
            WQMSize = 1 << (WQMSizeId + 2);

            bit_size = min(5, cu_bitsize);
            blk_size = (1 << bit_size) * (1 << bit_size) + 1;

            memset(coef_y, 0, bsize_x * bsize_y * sizeof(coef_t));

            coef_ctr = -1;
            level    =  1;
            iCG      =  0;
            pairs    =  0;

            dct_pairs = aec_run_level(h_dec, aec, cu, 0, dct_runs, dct_levels, DCT_CGFlag, DCT_PairsInCG, &DCT_CGNum, LUMA_8x8) - 1;

            shift = tab_iq_shift[qp] + (h_dec->seq->sample_bit_depth + 1) + cu_bitsize - LIMIT_BIT;
            add = 1 << (shift - 1);
            QPI = tab_iq_tab[qp];

            for (k = 0; level; k++) {
                if (dct_pairs >= 0) {
                    level = dct_levels[dct_pairs];
                    run = dct_runs[dct_pairs];
                    dct_pairs--;
                } else {
                    run = level = 0;
                }

                if (level != 0) {  /* leave if len=1 */
                    int base_icg = iCG;
                    while (DCT_CGFlag[DCT_CGNum - iCG - 1] == 0) {
                        iCG ++;
                    }
                    coef_ctr += (iCG - base_icg) << 4;

                    pairs ++;
                    coef_ctr += run + 1;

                    if (swap) {
                        i = SCAN[coef_ctr][1];
                        j = SCAN[coef_ctr][0];
                    } else {
                        i = SCAN[coef_ctr][0];
                        j = SCAN[coef_ctr][1];
                    }

                    if (h_dec->pic_hdr.weighting_quant_flag) {
                        sum = ((((((int)level * wq_get_weight(i, j)) >> wqm_shift) * QPI) >> 4) + add) >> (shift); // M2239, N1466
                    } else {
                        sum = (level * QPI + add) >> (shift);
                    }

                    sum = Clip3(0 - (1 << 15), (1 << 15) - 1,  sum);

                    coef_y[j * bsize_x + i] = (coef_t) sum;
                }
                if (pairs == DCT_PairsInCG[DCT_CGNum - iCG - 1]) {
                    coef_ctr |= 0xf;
                    pairs = 0;
                    iCG ++;
                }
            }
        }
    } else {
        if (iHor) {
            bsize_x = min(32, 1 << cu_bitsize);
            bsize_y = min(8, 1 << (cu_bitsize - 2));
            bit_size = min(5, cu_bitsize);
            SCAN = tab_scan_list_hor[min(2, cu_bitsize - 3)];
            WQMSizeId = min(3, cu_bitsize - 2);
        } else if (iVer) {
            bsize_x  = min( 8, 1 << (cu_bitsize - 2));
            bsize_y  = min(32, 1 << cu_bitsize);
            bit_size = min(5, cu_bitsize);
            SCAN = tab_scan_list_ver[min(2, cu_bitsize - 3)];
            WQMSizeId = min(3, cu_bitsize - 2);
        } else {
            bsize_x = 1 << (cu_bitsize - 1);
            bsize_y = bsize_x;
            bit_size = cu_bitsize;
            SCAN = tab_scan_list_cg[cu_bitsize - 3];
            WQMSizeId = cu_bitsize - 3;
        }
        WQMSize = 1 << (WQMSizeId + 2);
        blk_size = (1 << (bit_size - 1)) * (1 << (bit_size - 1)) + 1;

        for (b8 = 0; b8 < 4; b8++) { /* all modes */
            if (cu->cbp & (1 << b8)) {
                int swap = (IS_INTRA(cu) && tab_intraModeClassified[cu_loc_dat->intra_pred_modes[b8]] == INTRA_PRED_HOR && cuType != InNxNMB && cuType != INxnNMB);
                memset(coef_y, 0, bsize_x * bsize_y * sizeof(coef_t));
                coef_ctr = -1;
                level    =  1;
                iCG      =  0;
                pairs    =  0;

                dct_pairs = aec_run_level(h_dec, aec, cu, b8, dct_runs, dct_levels, DCT_CGFlag, DCT_PairsInCG, &DCT_CGNum, LUMA_8x8) - 1;

                if (h_dec->seq->useNSQT && cu_bitsize > B8X8_IN_BIT && (iHor || iVer) && cu_bitsize == 6) {
                    shift = tab_iq_shift[qp] + (h_dec->seq->sample_bit_depth + 1) + (bit_size)-LIMIT_BIT;
                } else {
                    shift = tab_iq_shift[qp] + (h_dec->seq->sample_bit_depth + 1) + (bit_size - 1) - LIMIT_BIT;
                }

                add = 1 << (shift - 1);
                QPI = tab_iq_tab[qp];

                for (k = 0; (level); k++) {
                    if (dct_pairs >= 0) {
                        level = dct_levels[dct_pairs];
                        run = dct_runs[dct_pairs];
                        dct_pairs--;
                    } else {
                        run = level = 0;
                    }

                    //============ decode =============
                    if (level != 0) {  /* leave if len=1 */
                        while (DCT_CGFlag[DCT_CGNum - iCG - 1] == 0) {
                            coef_ctr += 16;
                            iCG++;
                        }

                        pairs++;
                        coef_ctr += run + 1;

                        if (swap) {
                            i = SCAN[coef_ctr][1];
                            j = SCAN[coef_ctr][0];
                        } else {
                            i = SCAN[coef_ctr][0];
                            j = SCAN[coef_ctr][1];
                        }

                        if (h_dec->pic_hdr.weighting_quant_flag) {
                            sum = ((((((int)level * wq_get_weight(i, j)) >> wqm_shift) * QPI) >> 4) + add) >> shift; // M2239, N1466
                        } else {
                            sum = (level * QPI + add) >> shift;
                        }

                        sum = Clip3(0 - (1 << 15), (1 << 15) - 1, sum);

                        coef_y[j * bsize_x + i] = (coef_t) sum;
                    }
                    if (pairs == DCT_PairsInCG[DCT_CGNum - iCG - 1]) {
                        coef_ctr |= 0xf;
                        pairs = 0;
                        iCG++;
                    }
                }
            }
            coef_y += cu_zig_size >> 2;
        }
    }

    // Chroma
    SCAN = tab_scan_list_cg[cu_bitsize - 3];
    bsize_x = 1 << (cu_bitsize - 1);
    bsize_y = bsize_x;
    WQMSizeId = cu_bitsize - 3;
    WQMSize = 1 << (WQMSizeId + 2);

    bit_size = cu_bitsize;
    blk_size = (1 << (bit_size - 1)) * (1 << (bit_size - 1)) + 1;

    for (uv = 0; uv < 2; uv++) {
        if (h_dec->seq->sample_bit_depth > 8) {
            qp = tab_qp_scale_cr[Clip3(0, 63, (cu->qp - (8 * (h_dec->seq->sample_bit_depth - 8))))] + (8 * (h_dec->seq->sample_bit_depth - 8));
            if (!h_dec->pic_hdr.chroma_quant_param_disable) {
                qp = tab_qp_scale_cr[Clip3(0, 63, (cu->qp + (uv == 0 ? h_dec->pic_hdr.chroma_quant_param_delta_u : h_dec->pic_hdr.chroma_quant_param_delta_v) - (8 * (h_dec->seq->sample_bit_depth - 8))))] + (8 * (h_dec->seq->sample_bit_depth - 8));
            }
        } else {
            qp = tab_qp_scale_cr[cu->qp];
            if (!h_dec->pic_hdr.chroma_quant_param_disable) {
                qp = tab_qp_scale_cr[Clip3(0, 63, cu->qp + (uv == 0 ? h_dec->pic_hdr.chroma_quant_param_delta_u : h_dec->pic_hdr.chroma_quant_param_delta_v))];
            }
        }

        if ((cu->cbp >> (uv + 4)) & 0x1) {
            iCG = 0;
            pairs = 0;
            coef_ctr = -1;
            level = 1;

            memset(coef_uv[uv], 0, bsize_x * bsize_y * sizeof(coef_t));

            dct_pairs = aec_run_level(h_dec, aec, cu, 0, dct_runs, dct_levels, DCT_CGFlag, DCT_PairsInCG, &DCT_CGNum, CHROMA) - 1;

            shift = tab_iq_shift[qp] + (h_dec->seq->sample_bit_depth + 1) + (bit_size - 1) - LIMIT_BIT + 1;
            QPI = tab_iq_tab[qp];

            for (k = 0; (level); k++) {
                if (dct_pairs >= 0) {
                    level = dct_levels[dct_pairs];
                    run = dct_runs[dct_pairs];
                    dct_pairs--;
                } else {
                    run = level = 0;
                }

                if (level != 0) {                   // leave if len=1
                    while (DCT_CGFlag[ DCT_CGNum - iCG - 1 ] == 0) {
                        coef_ctr += 16;
                        iCG ++;
                    }

                    pairs ++;
                    coef_ctr = coef_ctr + run + 1;

                    i0 = SCAN[coef_ctr][0];
                    j0 = SCAN[coef_ctr][1];

                    if (h_dec->pic_hdr.weighting_quant_flag) {
                        sum = ((((((int)level * wq_get_weight(i0, j0)) >> wqm_shift) * QPI) >> 4) + (1 << (shift - 2))) >> (shift - 1); // M2239, N1466
                    } else {
                        sum = (level * QPI + (1 << (shift - 2))) >> (shift - 1);
                    }

                    sum = Clip3(0 - (1 << 15), (1 << 15) - 1,  sum);

                    coef_uv[uv][j0 * bsize_x + i0] = (coef_t) sum;
                } 
                if (pairs == DCT_PairsInCG[DCT_CGNum - iCG - 1]) {
                    coef_ctr |= 0xf;
                    pairs = 0;
                    iCG ++;
                }
            } 
        }
    }
}
