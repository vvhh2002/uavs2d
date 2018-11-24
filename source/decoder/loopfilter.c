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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "global.h"
#include <assert.h>
#include "commonVariables.h"
#include "loop-filter.h"
#include "AEC.h"
#define  IClip( Min, Max, Val) (((Val)<(Min))? (Min):(((Val)>(Max))? (Max):(Val)))

extern tab_char_t tab_qp_scale_cr[64];

static tab_char_t tab_deblock_alpha[64] = {
    0, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 1, 2, 2, 2, 3, 3,
    4, 4, 5, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 15, 16, 18, 20,
    22, 24, 26, 28, 30, 33, 33, 35,
    35, 36, 37, 37, 39, 39, 42, 44,
    46, 48, 50, 52, 53, 54, 55, 56,
    57, 58, 59, 60, 61, 62, 63, 64
};
static tab_char_t  tab_deblock_beta[64] = {
    0, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 1, 1, 1, 2, 2, 2,
    2, 2, 3, 3, 3, 3, 4, 4,
    4, 4, 5, 5, 5, 5, 6, 6,
    6, 7, 7, 7, 8, 8, 8, 9,
    9, 10, 10, 11, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22,
    23, 23, 24, 24, 25, 25, 26, 27
};

////////////////////////////ADD BY HJQ 2012-11-1//////////////////////////////////////////////
#define LOOPFILTER_SIZE 3   //8X8

static void xSetEdgeFilterParam(avs2_dec_t *h_dec, unsigned int uiBitSize, unsigned int b8_x_start, unsigned int b8_y_start, int idir, int flag)
{
    int i;
    int i_b8 = h_dec->seq->img_width_in_mcu;
    int i_b4 = h_dec->seq->b4_info_stride;
    int b8_offset = b8_y_start * i_b8 + b8_x_start;
    int b4_offset = (b8_y_start * i_b4 + b8_x_start) * 2;

    uchar_t *p_edge;
    uchar_t *p_skip;
    int edge_size = 1 << (uiBitSize - LOOPFILTER_SIZE);

    com_cu_t *cuP;
    com_cu_t *cuQ = h_dec->cu_array + b8_offset;

    char_t *pref       = h_dec->frm_cur->refbuf + b4_offset;
    i16s_t(*pmvbuf)[2] = h_dec->frm_cur->mvbuf  + b4_offset;

    int imgType = h_dec->type;
    int check_imgType = (imgType == P_IMG || imgType == F_IMG);
    int img_width_in_mcu = h_dec->seq->img_width_in_mcu;
   
#define IS_DEBLOCK_SKIPED(off1, off2)(              \
    (abs(pmvbuf[off1][0] - pmvbuf[off2][0]) < 4) && \
    (abs(pmvbuf[off1][1] - pmvbuf[off2][1]) < 4) && \
    (pref[off1] == pref[off2]) &&                   \
    (pref[off1] != -1)                              \
    )

    if (idir == 0) {
        if ((int)b8_x_start >= (h_dec->seq->img_width >> LOOPFILTER_SIZE)) {
            return;
        }
        p_edge = h_dec->deblock_edge_flag[0] + b8_offset;
        p_skip = h_dec->deblock_skip_flag[0] + b8_offset * 2;

        for (i = 0; i < edge_size && !(*p_edge); i++) {
            int check_mv_ref = check_imgType;

            p_edge[0] = 1;
            p_edge += i_b8;

            cuP = (cuQ - 1);
            check_mv_ref &= (cuP->cbp == 0 && cuQ->cbp == 0);
            cuQ += i_b8;

            p_skip[0] = (check_mv_ref && IS_DEBLOCK_SKIPED(0, -1)) ? 0 : flag;
            pref += i_b4;
            pmvbuf += i_b4;

            p_skip[1] = (check_mv_ref && IS_DEBLOCK_SKIPED(0, -1)) ? 0 : flag;
            pref += i_b4;
            pmvbuf += i_b4;

            p_skip += i_b8 * 2;
        }
    } else {
        p_edge = h_dec->deblock_edge_flag[1] + b8_offset;
        p_skip = h_dec->deblock_skip_flag[1] + b8_offset * 2;
        for (i = 0; i < edge_size && !(*p_edge); i++) {
            int check_mv_ref = check_imgType;

            *p_edge++ = 1;

            cuP = (cuQ - img_width_in_mcu);
            check_mv_ref &= (cuP->cbp == 0 && cuQ->cbp == 0);
            cuQ++;

            *p_skip++ = (check_mv_ref && IS_DEBLOCK_SKIPED(0, -i_b4)) ? 0 : flag;
            pref++;
            pmvbuf++;

            *p_skip++ = (check_mv_ref && IS_DEBLOCK_SKIPED(0, -i_b4)) ? 0 : flag;
            pref++;
            pmvbuf++;
        }
    }

}

void deblock_set_cu_edge(avs2_dec_t *h_dec, unsigned int uiBitSize, unsigned int cu_idx)
{
    int i;
    com_cu_t *cu = h_dec->cu;
    int cuType = cu->cuType;
    unsigned int b8_x_start = h_dec->cu_b8_x; 
    unsigned int b8_y_start = h_dec->cu_b8_y;
    int flag_deblk_all = 2;
    int flag_deblk_luma = 1;

    // cu border
    if (b8_x_start) {
        xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start, 0, flag_deblk_all);
    }
    if (b8_y_start) {
        xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start, 1, flag_deblk_all);
    }

    if (uiBitSize <= B8X8_IN_BIT) {
        return;
    }

    i = (uiBitSize - LOOPFILTER_SIZE - 1); /// b8

    // pu border
    switch (cuType) {
    case  P2NXN:
        xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << i), 1, flag_deblk_all);
        break;
    case PNX2N:
        xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << i), b8_y_start, 0, flag_deblk_all);
        break;
    case I8MB:
        xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << i), b8_y_start, 0, flag_deblk_luma);
        xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << i), 1, flag_deblk_luma);
        break;
    case InNxNMB:
        if (i) {
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << (i - 1))    , 1, flag_deblk_luma);
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << (i    ))    , 1, flag_deblk_luma);
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << (i - 1)) * 3, 1, flag_deblk_luma);
        } else {
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + 1, 1, flag_deblk_luma);
        }
        break;
    case INxnNMB:
        if (i) {
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << (i - 1))    , b8_y_start, 0, flag_deblk_luma);
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << (i    ))    , b8_y_start, 0, flag_deblk_luma);
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << (i - 1)) * 3, b8_y_start, 0, flag_deblk_luma);
        } else {
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + 1, b8_y_start, 0, flag_deblk_luma);
        }
        break;
    case PHOR_UP:
        if (i) {
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << (i - 1)), 1, flag_deblk_all);
        }
        break;
    case PHOR_DOWN:
        if (i) {
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << (i - 1)) * 3, 1, flag_deblk_all);
        }
        break;
    case PVER_LEFT:
        if (i) {
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << (i - 1)), b8_y_start, 0, flag_deblk_all);
        }
        break;
    case PVER_RIGHT:
        if (i) {
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << (i - 1)) * 3, b8_y_start, 0, flag_deblk_all);
        }
        break;
    default:
        break;
    }

    // tu border
    if (cuType != I8MB && h_dec->cu_loc_dat->trans_size == 1 && cu->cbp != 0) {
        if (h_dec->seq->useNSQT && (cuType == P2NXN || cuType == PHOR_UP || cuType == PHOR_DOWN || cuType == InNxNMB)) {
            if (i) {
                xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << (i - 1))    , 1, flag_deblk_luma);
                xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << (i    ))    , 1, flag_deblk_luma);
                xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << (i - 1)) * 3, 1, flag_deblk_luma);
            } else {
                xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + 1, 1, flag_deblk_luma);
            }
        } else if (h_dec->seq->useNSQT && (cuType == PNX2N || cuType == PVER_LEFT || cuType == PVER_RIGHT || cuType == INxnNMB)) {
            if (i) {
                xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << (i - 1))    , b8_y_start, 0, flag_deblk_luma);
                xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << (i    ))    , b8_y_start, 0, flag_deblk_luma);
                xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << (i - 1)) * 3, b8_y_start, 0, flag_deblk_luma);
            } else {
                xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << (i)), b8_y_start, 0, flag_deblk_luma);
            }
        } else {
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start + (1 << i), b8_y_start, 0, flag_deblk_luma);
            xSetEdgeFilterParam(h_dec, uiBitSize, b8_x_start, b8_y_start + (1 << i), 1, flag_deblk_luma);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
/*
*************************************************************************
* Function:The main MB-filtering function
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
static void deblock_b8(com_rec_t *rec, com_pic_t *pic, int b8y, int b8x, int dir)
{
    com_cu_t *MbP, *MbQ;
    const seq_info_t *seq = rec->seq;
    int i_b8 = rec->seq->img_width_in_mcu;
    uchar_t *skip_flag = rec->deblock_skip_flag[dir] + (b8y * i_b8 + b8x) * 2;
    uchar_t fixed_skip_flag[2];

    MbQ = &rec->cu_array[b8y * seq->img_width_in_mcu + b8x];
    MbP = (dir) ? (MbQ - seq->img_width_in_mcu) : (MbQ - 1);

    if ((MbQ->slice_nr == MbP->slice_nr || seq->lf_cross_slice) && (skip_flag[0] || skip_flag[1])) {
        int i_src = pic->i_stride;
        const pic_hdr_t *pichdr = rec->pichdr;
        pel_t *src_y = pic->p_y + (b8y << MIN_CU_SIZE_IN_BIT) * i_src + (b8x << MIN_CU_SIZE_IN_BIT);
        int QP = ((MbP->qp + MbQ->qp + 1) >> 1) - (8 * (seq->sample_bit_depth - 8));
        int shift = seq->sample_bit_depth - 8;
        int Alpha = tab_deblock_alpha[Clip3(0, 63, QP + pichdr->alpha_c_offset)] << shift;
        int Beta  = tab_deblock_beta [Clip3(0, 63, QP + pichdr->beta_offset   )] << shift;

        fixed_skip_flag[0] = skip_flag[0];
        fixed_skip_flag[1] = skip_flag[1];

        g_funs_handle.deblock_edge[dir](src_y, i_src, Alpha, Beta, fixed_skip_flag);

        fixed_skip_flag[0] = (uchar_t) (skip_flag[0] == 2);
        fixed_skip_flag[1] = (uchar_t) (skip_flag[1] == 2);

        if ((((b8x & 0x1) == 0 && (!dir)) || ((b8y & 0x1) == 0) && dir) && (fixed_skip_flag[0] || fixed_skip_flag[1])) {
            int i_srcc = pic->i_stridec;
            int offset = (b8y << (MIN_CU_SIZE_IN_BIT - 1)) * i_srcc + (b8x << (MIN_CU_SIZE_IN_BIT - 1));
            QP    = tab_qp_scale_cr[QP];
            Alpha = tab_deblock_alpha[Clip3(0, 63, QP + pichdr->alpha_c_offset)] << shift;
            Beta  = tab_deblock_beta [Clip3(0, 63, QP + pichdr->beta_offset   )] << shift;
            g_funs_handle.deblock_edge_c[dir](pic->p_u + offset, pic->p_v + offset, i_srcc, Alpha, Beta, fixed_skip_flag);
        }
    }
}

void deblock_lcu_row(com_rec_t *rec, com_pic_t *pic)
{
    int i, j;
    const seq_info_t* seq = rec->seq;
    int b8_width = seq->img_width_in_mcu;
    int start_b8y = rec->lcu_b8_y;
    int end_b8y = start_b8y + ((1 << seq->g_uiMaxSizeInBit) >> 3);

    uchar_t *p_edge_ver = rec->deblock_edge_flag[0] + start_b8y * b8_width;
    uchar_t *p_edge_hor = rec->deblock_edge_flag[1] + start_b8y * b8_width;

    end_b8y = min(end_b8y, seq->img_height_in_mcu);

    for (i = start_b8y; i < end_b8y; i++) {
        for (j = 0; j < b8_width; j++) {
            if (p_edge_ver[j + 1] && j + 1 < b8_width) {
                deblock_b8(rec, pic, i, j + 1, 0);
            }
            if (p_edge_hor[j] && i) {
                deblock_b8(rec, pic, i, j, 1);
            }
        }
        p_edge_ver += b8_width;
        p_edge_hor += b8_width;
    }
}

static void read_sao_smb(avs2_dec_t *h_dec, int smb_index, int pix_y, int pix_x, int smb_pix_width, int smb_pix_height, int *slice_sao_on, sap_param_t *sao_cur_param, sap_param_t *rec_sao_cur_param)
{
    int mb_x = pix_x >> MIN_CU_SIZE_IN_BIT;
    int mb_y = pix_y >> MIN_CU_SIZE_IN_BIT;
    int mb = mb_y * h_dec->seq->img_width_in_mcu + mb_x;
    sap_param_t merge_candidate[NUM_SAO_MERGE_TYPES][3];
    int merge_avail[NUM_SAO_MERGE_TYPES];
    int MergeLeftAvail, MergeUpAvail;
    int mergemode, saomode, saotype;
    int offset[32];
    int compIdx;
    int offsetTh = 7;
    int stBnd[2];

    int db_temp;

    getMergeNeighbor(h_dec, smb_index, pix_y,  pix_x, smb_pix_width, smb_pix_height, h_dec->seq->g_uiMaxSizeInBit, h_dec->seq->slice_set_enable, h_dec->sao_blk_params_rec, merge_avail, merge_candidate);
    MergeLeftAvail = merge_avail[0];
    MergeUpAvail = merge_avail[1];
    mergemode = 0;
    if (MergeLeftAvail + MergeUpAvail > 0) {
        mergemode = aec_sao_merge_flag(h_dec, MergeLeftAvail, MergeUpAvail, mb);
    }
    if (mergemode) {
        if (mergemode == 2) {
            copySAOParam_for_blk(rec_sao_cur_param, merge_candidate[SAO_MERGE_LEFT]);
        } else {
            assert(mergemode == 1);
            copySAOParam_for_blk(rec_sao_cur_param, merge_candidate[SAO_MERGE_ABOVE]);
        }
        copySAOParam_for_blk(sao_cur_param, rec_sao_cur_param);
        sao_cur_param->modeIdc = SAO_MODE_MERGE;
        sao_cur_param->typeIdc = mergemode;
    } else {
        for (compIdx = 0; compIdx < 3; compIdx++) {
            if (!slice_sao_on[compIdx]) {
                sao_cur_param[compIdx].modeIdc = SAO_MODE_OFF;
            } else {
                if (1) {
                    saomode = aec_sao_mode(h_dec, mb);
                    switch (saomode) {
                    case 0:
                        sao_cur_param[compIdx].modeIdc = SAO_MODE_OFF;
                        break;
                    case 1:
                        sao_cur_param[compIdx].modeIdc = SAO_MODE_NEW;
                        sao_cur_param[compIdx].typeIdc = SAO_TYPE_BO;
                        break;
                    case 3:
                        sao_cur_param[compIdx].modeIdc = SAO_MODE_NEW;
                        sao_cur_param[compIdx].typeIdc = SAO_TYPE_EO_0;
                        break;
                    default:
                        assert(1);
                        break;
                    }
                } else {
                    sao_cur_param[compIdx].modeIdc = sao_cur_param[SAO_Cb].modeIdc;
                    if (sao_cur_param[compIdx].modeIdc != SAO_MODE_OFF) {
                        sao_cur_param[compIdx].typeIdc = (sao_cur_param[SAO_Cb].typeIdc == SAO_TYPE_BO) ? SAO_TYPE_BO : SAO_TYPE_EO_0;
                    }
                }
                if (sao_cur_param[compIdx].modeIdc == SAO_MODE_NEW) {
                    aec_sao_offsets(h_dec, &(sao_cur_param[compIdx]), mb, offsetTh, offset);
                    saotype = aec_sao_type(h_dec, &(sao_cur_param[compIdx]), mb);

                    if (sao_cur_param[compIdx].typeIdc == SAO_TYPE_BO) {
                        memset(sao_cur_param[compIdx].offset, 0, sizeof(int)*MAX_NUM_SAO_CLASSES);
                        db_temp = saotype >> NUM_SAO_BO_CLASSES_LOG2;
                        stBnd[0] = saotype - (db_temp << NUM_SAO_BO_CLASSES_LOG2);
                        stBnd[1] = (stBnd[0] + db_temp) % 32;
                        sao_cur_param[compIdx].startBand = stBnd[0];
                        sao_cur_param[compIdx].startBand2 = stBnd[1];

                        memcpy(sao_cur_param[compIdx].offset, offset, 4 * sizeof(int));
                    } else {
                        assert(sao_cur_param[compIdx].typeIdc == SAO_TYPE_EO_0);
                        sao_cur_param[compIdx].typeIdc = saotype;
                        sao_cur_param[compIdx].offset[SAO_CLASS_EO_FULL_VALLEY] = offset[0];
                        sao_cur_param[compIdx].offset[SAO_CLASS_EO_HALF_VALLEY] = offset[1];
                        sao_cur_param[compIdx].offset[SAO_CLASS_EO_PLAIN      ] = 0;
                        sao_cur_param[compIdx].offset[SAO_CLASS_EO_HALF_PEAK  ] = offset[2];
                        sao_cur_param[compIdx].offset[SAO_CLASS_EO_FULL_PEAK  ] = offset[3];
                    }
                }
            }
        }
        copySAOParam_for_blk(rec_sao_cur_param, sao_cur_param);

    }

}


void readParaSAO_one_SMB(avs2_dec_t *h_dec, int smb_index, int mb_y, int mb_x, int *slice_sao_on, sap_param_t *saoBlkParam, sap_param_t *rec_saoBlkParam)
{
    int pix_x = mb_x << MIN_CU_SIZE_IN_BIT;
    int pix_y = mb_y << MIN_CU_SIZE_IN_BIT;
    int smb_pix_width = h_dec->lcu_width;
    int smb_pix_height = h_dec->lcu_height;
    if (!slice_sao_on[0] && !slice_sao_on[1] && !slice_sao_on[2]) {
        off_sao(rec_saoBlkParam);
        off_sao(saoBlkParam);
    } else {
        read_sao_smb(h_dec, smb_index, pix_y, pix_x, smb_pix_width, smb_pix_height, slice_sao_on, saoBlkParam, rec_saoBlkParam);
    }

}