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

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "commonVariables.h"
#include "commonStructures.h"
#include "loop-filter.h"
#define  IClip( Min, Max, Val) (((Val)<(Min))? (Min):(((Val)>(Max))? (Max):(Val)))

void off_sao(sap_param_t *saoblkparam)
{
    int i;
    for (i = 0; i < 3; i++) {
        saoblkparam[i].modeIdc = SAO_MODE_OFF;
        saoblkparam[i].typeIdc = -1;
        saoblkparam[i].startBand = -1;
        saoblkparam[i].startBand2 = -1;
        saoblkparam[i].deltaband = -1;
        memset(saoblkparam[i].offset, 0, sizeof(int)*MAX_NUM_SAO_CLASSES);
    }
}
void copySAOParam_for_blk(sap_param_t *saopara_dst, sap_param_t *saopara_src)
{
    int i, j;
    for (i = 0; i < 3; i++) {
        saopara_dst[i].modeIdc = saopara_src[i].modeIdc;
        saopara_dst[i].typeIdc = saopara_src[i].typeIdc;
        saopara_dst[i].startBand = saopara_src[i].startBand;
        saopara_dst[i].startBand2 = saopara_src[i].startBand2;
        saopara_dst[i].deltaband = saopara_src[i].deltaband;
        for (j = 0; j < MAX_NUM_SAO_CLASSES; j++) {
            saopara_dst[i].offset[j] = saopara_src[i].offset[j];
        }
    }
}
void copySAOParam_for_blk_onecomponent(sap_param_t *saopara_dst, sap_param_t *saopara_src)
{
    int  j;
    saopara_dst->modeIdc = saopara_src->modeIdc;
    saopara_dst->typeIdc = saopara_src->typeIdc;
    saopara_dst->startBand = saopara_src->startBand;
    saopara_dst->startBand2 = saopara_src->startBand2;
    saopara_dst->deltaband = saopara_src->deltaband;

    for (j = 0; j < MAX_NUM_SAO_CLASSES; j++) {
        saopara_dst->offset[j] = saopara_src->offset[j];
    }
}

void getMergeNeighbor(avs2_dec_t *h_dec, int smb_index, int pix_y, int pix_x, int smb_pix_width, int smb_pix_height, int input_MaxsizeInBit, int input_slice_set_enable, sap_param_t(*rec_saoBlkParam)[3], int *MergeAvail, sap_param_t sao_merge_param[][3])
{
    int mb_y = pix_y >> MIN_CU_SIZE_IN_BIT;
    int mb_x = pix_x >> MIN_CU_SIZE_IN_BIT;
    int pic_mb_width             = h_dec->seq->img_width / 8;
    int mb_nr;
    int mergeup_avail, mergeleft_avail;
    int width_in_smb;
    sap_param_t *sao_left_param;
    sap_param_t *sao_up_param;
    mb_nr = mb_y * pic_mb_width + mb_x;
    width_in_smb = (h_dec->seq->img_width % (1 << input_MaxsizeInBit)) ? (h_dec->seq->img_width / (1 << input_MaxsizeInBit) + 1) : (h_dec->seq->img_width / (1 << input_MaxsizeInBit));
    if (input_slice_set_enable) { //added by mz, 2008.04
        mergeup_avail      = (mb_y == 0) ? 0 : (h_dec->cu_array[mb_nr].slice_set_index == h_dec->cu_array[mb_nr - pic_mb_width].slice_set_index);
        mergeleft_avail    = (mb_x == 0) ? 0 : (h_dec->cu_array[mb_nr].slice_set_index == h_dec->cu_array[mb_nr - 1].slice_set_index);
    } else {
        mergeup_avail      = (mb_y == 0) ? 0 : (h_dec->cu_array[mb_nr].slice_nr == h_dec->cu_array[mb_nr - pic_mb_width].slice_nr);
        mergeleft_avail    = (mb_x == 0) ? 0 : (h_dec->cu_array[mb_nr].slice_nr == h_dec->cu_array[mb_nr - 1].slice_nr);
    }
    if (mergeleft_avail) {
        sao_left_param =  rec_saoBlkParam[smb_index -  1] ;
        copySAOParam_for_blk(sao_merge_param[SAO_MERGE_LEFT], sao_left_param);
    }
    if (mergeup_avail) {
        sao_up_param = rec_saoBlkParam[smb_index - width_in_smb];
        copySAOParam_for_blk(sao_merge_param[SAO_MERGE_ABOVE], sao_up_param);
    }
    MergeAvail[SAO_MERGE_LEFT] = mergeleft_avail;
    MergeAvail[SAO_MERGE_ABOVE] = mergeup_avail;
}

void checkBoundaryAvail(com_rec_t *rec, int mb_y, int mb_x, int smb_pix_height, int smb_pix_width, int input_slice_set_enable, int *smb_available_left, int *smb_available_right, int *smb_available_up, int *smb_available_down, int *smb_available_upleft, int *smb_available_upright, int *smb_available_leftdown, int *smb_available_rightdwon)
{
    com_cu_t *cu_array = rec->cu_array;

    int pic_mb_width  = rec->seq->img_width / 8;
    int pic_mb_height = rec->seq->img_height / 8;
    int mb_nr         = mb_y * pic_mb_width + mb_x;
    int smb_mb_width  = smb_pix_width >> MIN_CU_SIZE_IN_BIT;
    int smb_mb_height = smb_pix_height >> MIN_CU_SIZE_IN_BIT;

    if (input_slice_set_enable) { //added by mz, 2008.04
        *smb_available_up      = (mb_y == 0) ? 0 : (cu_array[mb_nr].slice_set_index == cu_array[mb_nr - pic_mb_width].slice_set_index);
        *smb_available_down    = (mb_y >= pic_mb_height - smb_mb_height) ? 0 : (cu_array[mb_nr].slice_set_index == cu_array[mb_nr + smb_mb_height * pic_mb_width].slice_set_index);
        *smb_available_left    = (mb_x == 0) ? 0 : (cu_array[mb_nr].slice_set_index == cu_array[mb_nr - 1].slice_set_index);
        *smb_available_right   = (mb_x >= pic_mb_width - smb_mb_width) ? 0 : (cu_array[mb_nr].slice_set_index == cu_array[mb_nr + smb_mb_width].slice_set_index);
        *smb_available_upleft  = (mb_x == 0 || mb_y == 0) ? 0 : (cu_array[mb_nr].slice_set_index == cu_array[mb_nr - pic_mb_width - 1].slice_set_index);
        *smb_available_upright = (mb_x >= pic_mb_width - smb_mb_height  || mb_y == 0) ? 0 : (cu_array[mb_nr].slice_set_index == cu_array[mb_nr - pic_mb_width + smb_mb_width].slice_set_index);
        *smb_available_leftdown = (mb_x == 0 || mb_y >= pic_mb_height - smb_mb_height) ? 0 : (cu_array[mb_nr].slice_set_index == cu_array[mb_nr  + smb_mb_height * pic_mb_width - 1].slice_set_index);
        *smb_available_rightdwon = (mb_x >= pic_mb_width - smb_mb_width || mb_y >= pic_mb_height - smb_mb_height) ? 0 : (cu_array[mb_nr].slice_set_index == cu_array[mb_nr  + smb_mb_height * pic_mb_width + smb_mb_width].slice_set_index);
    } else {
        *smb_available_up      = (mb_y == 0) ? 0 : (cu_array[mb_nr].slice_nr == cu_array[mb_nr - pic_mb_width].slice_nr);
        *smb_available_down    = (mb_y >= pic_mb_height - smb_mb_height) ? 0 : (cu_array[mb_nr].slice_nr == cu_array[mb_nr + smb_mb_height * pic_mb_width].slice_nr);
        *smb_available_left    = (mb_x == 0) ? 0 : (cu_array[mb_nr].slice_nr == cu_array[mb_nr - 1].slice_nr);
        *smb_available_right   = (mb_x >= pic_mb_width - smb_mb_width) ? 0 : (cu_array[mb_nr].slice_nr == cu_array[mb_nr + smb_mb_width].slice_nr);
        *smb_available_upleft  = (mb_x == 0 || mb_y == 0) ? 0 : (cu_array[mb_nr].slice_nr == cu_array[mb_nr - pic_mb_width - 1].slice_nr);
        *smb_available_upright = (mb_x >= pic_mb_width - smb_mb_width  || mb_y == 0) ? 0 : (cu_array[mb_nr].slice_nr == cu_array[mb_nr - pic_mb_width + smb_mb_width].slice_nr);
        *smb_available_leftdown = (mb_x == 0 || mb_y >= pic_mb_height - smb_mb_height) ? 0 : (cu_array[mb_nr].slice_nr == cu_array[mb_nr  + smb_mb_height * pic_mb_width - 1].slice_nr);
        *smb_available_rightdwon = (mb_x >= pic_mb_width - smb_mb_width || mb_y >= pic_mb_height - smb_mb_height) ? 0 : (cu_array[mb_nr].slice_nr == cu_array[mb_nr  + smb_mb_height * pic_mb_width + smb_mb_width].slice_nr);
    }
}

void SAO_on_block(void *p1, void *p2, void *p3, int compIdx, int smb_index, int pix_y, int pix_x, int smb_pix_height, int smb_pix_width,
    int smb_available_left, int smb_available_right, int smb_available_up, int smb_available_down, int smb_available_upleft, 
    int smb_available_upright, int smb_available_leftdown, int smb_available_rightdwon, int sample_bit_depth)
{
    com_pic_t *pic_src = (com_pic_t*)p1;
    com_pic_t *pic_dst = (com_pic_t*)p2;
    sap_param_t *saoBlkParam = (sap_param_t*)p3;
    int type;
    int start_x, end_x, start_y, end_y;
    int start_x_r0, end_x_r0, start_x_r, end_x_r, start_x_rn, end_x_rn;
    int x, y;
    pel_t *src, *dst;
    int i_src, i_dst;
    char_t leftsign, rightsign, upsign, downsign;
    int diff;
    char_t signupline[65], *signupline1;
    int reg = 0;
    int edgetype;

    if (compIdx == SAO_Y) {
        i_src = pic_src->i_stride;
        src   = pic_src->p_y + pix_y * i_src + pix_x;
        i_dst = pic_dst->i_stride;
        dst   = pic_dst->p_y + pix_y * i_dst + pix_x;
    } else {
        i_src = pic_src->i_stridec;
        src   = (compIdx == SAO_Cb) ? pic_src->p_u : pic_src->p_v;
        src  += pix_y * i_src + pix_x;
        i_dst = pic_dst->i_stridec;
        dst   = (compIdx == SAO_Cb) ? pic_dst->p_u : pic_dst->p_v;
        dst  += pix_y * i_dst + pix_x;
    }

    assert(saoBlkParam->modeIdc == SAO_MODE_NEW);
    type = saoBlkParam->typeIdc;

    switch (type) {
    case SAO_TYPE_EO_0: {

        start_x = smb_available_left ? 0 : 1;
        end_x = smb_available_right ? smb_pix_width : (smb_pix_width - 1);

        for (y = 0; y < smb_pix_height; y++) {
            diff = src[start_x] - src[start_x - 1];
            leftsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
            for (x = start_x; x < end_x; x++) {
                diff = src[x] - src[x + 1];
                rightsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
                edgetype = leftsign + rightsign;
                leftsign = - rightsign;
                dst[x] = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[x] + saoBlkParam->offset[edgetype + 2]);
            }
            dst += i_dst;
            src += i_src;
        }

    }
    break;
    case SAO_TYPE_EO_90: {
        pel_t *dst_base = dst;
        pel_t *src_base = src;
        start_y = smb_available_up ? 0 : 1;
        end_y = smb_available_down ? smb_pix_height : (smb_pix_height - 1);
        for (x = 0; x < smb_pix_width; x++) {
            src = src_base + start_y * i_src;
            diff = src[0] - src[-i_src];
            upsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
            dst = dst_base + start_y * i_dst;
            for (y = start_y; y < end_y; y++) {
                diff = src[0] - src[i_src];
                downsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
                edgetype = downsign + upsign;
                upsign = - downsign;
                *dst = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[0] + saoBlkParam->offset[edgetype + 2]);
                dst += i_dst;
				src += i_src;
            }
            dst_base++;
            src_base++;
        }
    }
    break;
    case SAO_TYPE_EO_135: {
        start_x_r0 = smb_available_upleft ? 0 : 1;
        end_x_r0 = smb_available_up ? (smb_available_right ? smb_pix_width : (smb_pix_width - 1)) : 1;
        start_x_r = smb_available_left ? 0 : 1;
        end_x_r = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
        start_x_rn = smb_available_down ? (smb_available_left ? 0 : 1) : (smb_pix_width - 1);
        end_x_rn = smb_available_rightdwon ? smb_pix_width : (smb_pix_width - 1);

        //init the line buffer
        for (x = start_x_r + 1; x < end_x_r + 1; x ++) {
            diff = src[x + i_src] - src[x - 1];
            upsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
            signupline[x] = upsign;
        }
        //first row
        for (x = start_x_r0; x < end_x_r0 ; x++) {
            diff = src[x] - src[x - 1 - i_src];
            upsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
            edgetype = upsign - signupline[x + 1];
            dst[x] = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[x] + saoBlkParam->offset[edgetype + 2]);
        }
        dst += i_dst;
        src += i_src;

        //middle rows
        for (y = 1 ; y < smb_pix_height - 1; y++) {
            for (x = start_x_r; x < end_x_r; x++) {
                if (x == start_x_r) {
                    diff = src[x] - src[x - 1 - i_src];
                    upsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
                    signupline[x] = upsign;
                }
                diff = src[x] - src[x + 1 + i_src];
                downsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
                edgetype = downsign + signupline[x];
                dst[x] = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[x] + saoBlkParam->offset[edgetype + 2]);
                signupline[x] = (char_t) reg;
                reg = -downsign;
            }
            dst += i_dst;
            src += i_src;
        }
        //last row
        for (x = start_x_rn; x < end_x_rn; x++) {
            if (x == start_x_r) {
                diff = src[x] - src[x - 1 - i_src];
                upsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
                signupline[x] = upsign;
            }
            diff = src[x] - src[x + 1 + i_src];
            downsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
            edgetype = downsign + signupline[x];
            dst[x] = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[x] + saoBlkParam->offset[edgetype + 2]);
        }
    }
    break;
    case SAO_TYPE_EO_45: {
        start_x_r0 = smb_available_up ? (smb_available_left ? 0 : 1) : (smb_pix_width - 1);
        end_x_r0 = smb_available_upright ?  smb_pix_width : (smb_pix_width - 1);
        start_x_r = smb_available_left ? 0 : 1;
        end_x_r = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
        start_x_rn = smb_available_leftdown ? 0 : 1;
        end_x_rn = smb_available_down ? (smb_available_right ? smb_pix_width : (smb_pix_width - 1)) : 1;

        //init the line buffer
        signupline1 = signupline + 1;
        for (x = start_x_r - 1; x < end_x_r - 1; x ++) {
            diff = src[x + i_src] - src[x + 1];
            upsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
            signupline1[x] = upsign;
        }
        //first row
        for (x = start_x_r0; x < end_x_r0; x++) {
            diff = src[x] - src[x + 1 - i_src];
            upsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
            edgetype = upsign - signupline1[x - 1];
            dst[x] = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[x] + saoBlkParam->offset[edgetype + 2]);
        }
        dst += i_dst;
        src += i_src;

        //middle rows
        for (y = 1 ; y < smb_pix_height - 1; y++) {
            for (x = start_x_r; x < end_x_r; x++) {
                if (x == end_x_r - 1) {
                    diff = src[x] - src[x + 1 - i_src];
                    upsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
                    signupline1[x] = upsign;
                }
                diff = src[x] - src[x - 1 + i_src];
                downsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
                edgetype = downsign + signupline1[x];
                dst[x] = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[x] + saoBlkParam->offset[edgetype + 2]);
                signupline1[x - 1] = -downsign;
            }
            dst += i_dst;
            src += i_src;
        }
        for (x = start_x_rn; x < end_x_rn; x++) {
            if (x == end_x_r - 1) {
                diff = src[x] - src[x + 1 - i_src];
                upsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
                signupline1[x] = upsign;
            }
            diff = src[x] - src[x - 1 + i_src];
            downsign = (char_t) (diff > 0 ? 1 : (diff < 0 ? -1 : 0));
            edgetype = downsign + signupline1[x];
            dst[x] = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[x] + saoBlkParam->offset[edgetype + 2]);
        }
    }
    break;
    case SAO_TYPE_BO: {
        pel_t *dst_base = dst;
        pel_t *src_base = src;

        for (x = 0; x < smb_pix_width; x++) {
            dst = dst_base;
            src = src_base;
            for (y = 0; y < smb_pix_height; y++) {
                int tmp = src[0] >> (sample_bit_depth - NUM_SAO_BO_CLASSES_IN_BIT);
                if (tmp == saoBlkParam->startBand){
                    *dst = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[0] + saoBlkParam->offset[0]);
                }
                else if (tmp == (saoBlkParam->startBand + 1)%32)
                {
                    *dst = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[0] + saoBlkParam->offset[1]);
                }
                else if (tmp == saoBlkParam->startBand2)
                {
                    *dst = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[0] + saoBlkParam->offset[2]);
                }
                else if (tmp == (saoBlkParam->startBand2 + 1)%32)
                {
                    *dst = (pel_t) Clip3(0, ((1 << sample_bit_depth) - 1), src[0] + saoBlkParam->offset[3]);
                }
                
                dst += i_dst;
                src += i_src;
            }
            dst_base++;
            src_base++;
        }

    }
    break;
    default: {
        printf("Not a supported SAO types\n");
        assert(0);
        exit(-1);
    }
    }
}



void SAO_on_smb(com_rec_t *rec, int smb_index, int pix_y, int pix_x, int smb_pix_width, int smb_pix_height, int input_slice_set_enable, sap_param_t *saoBlkParam, int sample_bit_depth)
{
    int compIdx;
    int mb_y = pix_y >> MIN_CU_SIZE_IN_BIT;
    int mb_x = pix_x >> MIN_CU_SIZE_IN_BIT;
    int isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail;
    int smb_pix_height_t, smb_pix_width_t, pix_x_t, pix_y_t;
    com_pic_t *pic_dst = rec->frm_cur->yuv_data;
    com_pic_t *pic_src = rec->g_pic_flt_tmp;

    checkBoundaryAvail(rec, mb_y, mb_x, smb_pix_height, smb_pix_width, input_slice_set_enable, &isLeftAvail, &isRightAvail, &isAboveAvail, &isBelowAvail, &isAboveLeftAvail, &isAboveRightAvail, &isBelowLeftAvail, &isBelowRightAvail);

    for (compIdx = 0; compIdx < 3; compIdx++) {
        if (saoBlkParam[compIdx].modeIdc != SAO_MODE_OFF) {
            if (!isAboveAvail && !isLeftAvail) {
                smb_pix_width_t = compIdx ? ((smb_pix_width >> 1) - SAO_SHIFT_PIX_NUM) : (smb_pix_width - SAO_SHIFT_PIX_NUM);
                smb_pix_height_t = compIdx ? ((smb_pix_height >> 1) - SAO_SHIFT_PIX_NUM) : (smb_pix_height - SAO_SHIFT_PIX_NUM);
                pix_x_t = compIdx ? (pix_x >> 1) : pix_x;
                pix_y_t = compIdx ? (pix_y >> 1) : pix_y;
                g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, 0 /*Left*/, 1/*Right*/, 0/*Above*/, 1/*Below*/, 0/*AboveLeft*/, isAboveAvail/*AboveRight*/, isLeftAvail/*BelowLeft*/, 1/*BelowRight*/, sample_bit_depth);

            } else if (!isAboveAvail && isLeftAvail){
                smb_pix_width_t = compIdx ? (smb_pix_width >> 1) : smb_pix_width;
                smb_pix_height_t = compIdx ? ((smb_pix_height >> 1) - SAO_SHIFT_PIX_NUM) : (smb_pix_height - SAO_SHIFT_PIX_NUM);
                pix_x_t = compIdx ? ((pix_x >> 1) - SAO_SHIFT_PIX_NUM) : (pix_x - SAO_SHIFT_PIX_NUM);
                pix_y_t = compIdx ? (pix_y >> 1) : pix_y;
                g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, 1 /*Left*/, 1/*Right*/, isAboveAvail/*Above*/, 1/*Below*/, isAboveLeftAvail/*AboveLeft*/, isAboveAvail/*AboveRight*/, isLeftAvail/*BelowLeft*/, 1/*BelowRight*/, sample_bit_depth);

            } else if (!isLeftAvail && isAboveAvail){
                smb_pix_width_t = compIdx ? ((smb_pix_width >> 1) - SAO_SHIFT_PIX_NUM) : (smb_pix_width - SAO_SHIFT_PIX_NUM);
                smb_pix_height_t = compIdx ? (smb_pix_height >> 1) : smb_pix_height;
                pix_x_t = compIdx ? (pix_x >> 1) : pix_x;
                pix_y_t = compIdx ? ((pix_y >> 1) - SAO_SHIFT_PIX_NUM) : (pix_y - SAO_SHIFT_PIX_NUM);
                g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, 0 /*Left*/, 1/*Right*/, 1/*Above*/, 1/*Below*/, isAboveLeftAvail/*AboveLeft*/, isAboveAvail/*AboveRight*/, isLeftAvail/*BelowLeft*/, 1/*BelowRight*/, sample_bit_depth);

            } else if (isAboveLeftAvail) {
                smb_pix_width_t = compIdx ? (smb_pix_width >> 1) : smb_pix_width;
                smb_pix_height_t = compIdx ? (smb_pix_height >> 1) : smb_pix_height;
                pix_x_t = compIdx ? ((pix_x >> 1) - SAO_SHIFT_PIX_NUM) : (pix_x - SAO_SHIFT_PIX_NUM);
                pix_y_t = compIdx ? ((pix_y >> 1) - SAO_SHIFT_PIX_NUM) : (pix_y - SAO_SHIFT_PIX_NUM);
                g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, 1 /*Left*/, 1/*Right*/, 1/*Above*/, 1/*Below*/, isAboveLeftAvail/*AboveLeft*/, 1/*AboveRight*/, isLeftAvail/*BelowLeft*/, 1/*BelowRight*/, sample_bit_depth);
            
            } else { //cross slice
                //above ctu
                smb_pix_width_t = compIdx ? ((smb_pix_width >> 1) - SAO_SHIFT_PIX_NUM) : (smb_pix_width - SAO_SHIFT_PIX_NUM);
                smb_pix_height_t = SAO_SHIFT_PIX_NUM;
                pix_x_t = compIdx ? (pix_x >> 1) : pix_x;
                pix_y_t = compIdx ? ((pix_y >> 1) - SAO_SHIFT_PIX_NUM) : (pix_y - SAO_SHIFT_PIX_NUM);
                g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, isAboveLeftAvail /*Left*/, isAboveAvail/*Right*/, isAboveAvail/*Above*/, 1/*Below*/, isAboveLeftAvail/*AboveLeft*/, isAboveAvail/*AboveRight*/, isLeftAvail/*BelowLeft*/, 1/*BelowRight*/, sample_bit_depth);

                //left ctu
                smb_pix_width_t = SAO_SHIFT_PIX_NUM;
                smb_pix_height_t = compIdx ? ((smb_pix_height >> 1) - SAO_SHIFT_PIX_NUM) : (smb_pix_height - SAO_SHIFT_PIX_NUM);
                pix_x_t = compIdx ? ((pix_x >> 1) - SAO_SHIFT_PIX_NUM) : (pix_x - SAO_SHIFT_PIX_NUM);
                pix_y_t = compIdx ? (pix_y >> 1) : pix_y;
                g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, isLeftAvail /*Left*/, 1/*Right*/, isAboveLeftAvail/*Above*/, isLeftAvail/*Below*/, isAboveLeftAvail/*AboveLeft*/, isAboveAvail/*AboveRight*/, isLeftAvail/*BelowLeft*/, 1/*BelowRight*/, sample_bit_depth);

                //current ctu
                smb_pix_width_t = compIdx ? ((smb_pix_width >> 1) - SAO_SHIFT_PIX_NUM) : (smb_pix_width - SAO_SHIFT_PIX_NUM);
                smb_pix_height_t = compIdx ? ((smb_pix_height >> 1) - SAO_SHIFT_PIX_NUM) : (smb_pix_height - SAO_SHIFT_PIX_NUM);
                pix_x_t = compIdx ? (pix_x >> 1) : pix_x;
                pix_y_t = compIdx ? (pix_y >> 1) : pix_y;
                g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, 0 /*Left*/, 1/*Right*/, 0/*Above*/, 1/*Below*/, 0/*AboveLeft*/, isAboveAvail/*AboveRight*/, isLeftAvail/*BelowLeft*/, 1/*BelowRight*/, sample_bit_depth);

            }

            if (!isRightAvail) {
                if (isAboveAvail) {
                    smb_pix_width_t = SAO_SHIFT_PIX_NUM;
                    smb_pix_height_t = compIdx ? (smb_pix_height >> 1) : smb_pix_height;
                    pix_x_t = compIdx ? ((pix_x >> 1) + (smb_pix_width >> 1) - SAO_SHIFT_PIX_NUM) : (pix_x + smb_pix_width - SAO_SHIFT_PIX_NUM);
                    pix_y_t = compIdx ? ((pix_y >> 1) - SAO_SHIFT_PIX_NUM) : (pix_y - SAO_SHIFT_PIX_NUM);
                    g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, isAboveAvail /*Left*/, isAboveRightAvail/*Right*/, isAboveAvail/*Above*/, 1/*Below*/, isAboveAvail/*AboveLeft*/, isAboveRightAvail/*AboveRight*/, 1/*BelowLeft*/, isRightAvail/*BelowRight*/, sample_bit_depth);
                } else {
                    smb_pix_width_t = SAO_SHIFT_PIX_NUM;
                    smb_pix_height_t = compIdx ? ((smb_pix_height >> 1) - SAO_SHIFT_PIX_NUM) : (smb_pix_height - SAO_SHIFT_PIX_NUM);
                    pix_x_t = compIdx ? ((pix_x >> 1) + (smb_pix_width >> 1) - SAO_SHIFT_PIX_NUM) : (pix_x + smb_pix_width - SAO_SHIFT_PIX_NUM);
                    pix_y_t = compIdx ? (pix_y >> 1) : pix_y;
                    g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, 1 /*Left*/, isRightAvail/*Right*/, isAboveAvail/*Above*/, 1/*Below*/, isAboveAvail/*AboveLeft*/, isAboveRightAvail/*AboveRight*/, 1/*BelowLeft*/, isRightAvail/*BelowRight*/, sample_bit_depth);
                }
            }
            if (!isBelowAvail) {
                if (isLeftAvail) {
                    smb_pix_width_t = compIdx ? (smb_pix_width >> 1) : smb_pix_width;
                    smb_pix_height_t = SAO_SHIFT_PIX_NUM;
                    pix_x_t = compIdx ? ((pix_x >> 1) - SAO_SHIFT_PIX_NUM) : (pix_x - SAO_SHIFT_PIX_NUM);
                    pix_y_t = compIdx ? ((pix_y >> 1) + (smb_pix_height >> 1) - SAO_SHIFT_PIX_NUM) : (pix_y + smb_pix_height - SAO_SHIFT_PIX_NUM);
                    g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, isLeftAvail /*Left*/, 1/*Right*/, isLeftAvail/*Above*/, isBelowLeftAvail/*Below*/, isLeftAvail/*AboveLeft*/, 1/*AboveRight*/, isBelowLeftAvail/*BelowLeft*/, isBelowAvail/*BelowRight*/, sample_bit_depth);
                } else {
                    smb_pix_width_t = compIdx ? ((smb_pix_width >> 1) - SAO_SHIFT_PIX_NUM) : (smb_pix_width - SAO_SHIFT_PIX_NUM);
                    smb_pix_height_t = SAO_SHIFT_PIX_NUM;
                    pix_x_t = compIdx ? (pix_x >> 1) : pix_x;
                    pix_y_t = compIdx ? ((pix_y >> 1) + (smb_pix_height >> 1) - SAO_SHIFT_PIX_NUM) : (pix_y + smb_pix_height - SAO_SHIFT_PIX_NUM);
                    g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, isLeftAvail /*Left*/, 1/*Right*/, 1/*Above*/, isBelowAvail/*Below*/, isLeftAvail/*AboveLeft*/, 1/*AboveRight*/, isBelowLeftAvail/*BelowLeft*/, isBelowAvail/*BelowRight*/, sample_bit_depth);
                }
            }
            if (!isBelowRightAvail && !isRightAvail && !isBelowAvail) {
                smb_pix_width_t = SAO_SHIFT_PIX_NUM;
                smb_pix_height_t = SAO_SHIFT_PIX_NUM;
                pix_x_t = compIdx ? ((pix_x >> 1) + (smb_pix_width >> 1) - SAO_SHIFT_PIX_NUM) : (pix_x + smb_pix_width - SAO_SHIFT_PIX_NUM);
                pix_y_t = compIdx ? ((pix_y >> 1) + (smb_pix_height >> 1) - SAO_SHIFT_PIX_NUM) : (pix_y + smb_pix_height - SAO_SHIFT_PIX_NUM);
                g_funs_handle.sao_flt(pic_src, pic_dst, &(saoBlkParam[compIdx]), compIdx, smb_index, pix_y_t, pix_x_t, smb_pix_height_t, smb_pix_width_t, 1 /*Left*/, isRightAvail/*Right*/, 1/*Above*/, isBelowAvail/*Below*/, 1/*AboveLeft*/, isRightAvail/*AboveRight*/, isBelowAvail/*BelowLeft*/, isBelowRightAvail/*BelowRight*/, sample_bit_depth);
            }
        }
    }
}

void sao_lcu_row(com_rec_t *rec, int input_MaxSizeInBit, int input_slice_set_enable, sap_param_t(*rec_saoBlkParam)[3], int sample_bit_depth, int pix_y)
{
    const seq_info_t *seq = rec->seq;
    int pix_x, mb_y, smb_x;
    int lcu_size = 1 << seq->g_uiMaxSizeInBit;
    int smb_pix_height, smb_pix_width;
    int smb_index;

    pix_y = max(pix_y, 0);

    smb_index = (seq->img_width + lcu_size - 1) / lcu_size * (pix_y / lcu_size);
    smb_pix_height = min(1 << (input_MaxSizeInBit), (seq->img_height - pix_y));
    mb_y = pix_y >> MIN_CU_SIZE_IN_BIT;
    for (pix_x = 0, smb_x = 0; pix_x < seq->img_width; pix_x += smb_pix_width, smb_x++) {
        smb_pix_width = min(1 << (input_MaxSizeInBit), (seq->img_width - pix_x));
        SAO_on_smb(rec, smb_index, pix_y, pix_x, smb_pix_width, smb_pix_height, input_slice_set_enable, rec_saoBlkParam[smb_index], sample_bit_depth);
        smb_index++;
    }
}


static void deblock_edge_ver(pel_t *SrcPtr, int stride, int Alpha, int Beta, uchar_t *flt_flag)
{
    int pel;
    int AbsDelta;
    int L2, L1, L0, R0, R1, R2;
    int fs;                     //fs stands for filtering strength.  The larger fs is, the stronger filter is applied.
    int FlatnessL, FlatnessR;   // FlatnessL and FlatnessR describe how flat the curve is of one macroblock.
    int flt_len = 8;

    if (!(flt_flag[0] & flt_flag[1])) {
        flt_len = 4;
        if (flt_flag[1]) {
            SrcPtr += 4 * stride;
        }
    }

    for (pel = 0; pel < flt_len; pel++, SrcPtr += stride) {
        L0 = SrcPtr[-1];
        R0 = SrcPtr[0];
        AbsDelta = COM_ABS(R0 - L0);

        if ((AbsDelta < Alpha) && (AbsDelta > 1)) {
            L2 = SrcPtr[-3];
            L1 = SrcPtr[-2];
            R1 = SrcPtr[1];
            R2 = SrcPtr[2];
            FlatnessL = (COM_ABS(L1 - L0) < Beta) ? 2 : 0;
            if (COM_ABS(L2 - L0) < Beta) {
                FlatnessL++;
            }

            FlatnessR = (COM_ABS(R0 - R1) < Beta) ? 2 : 0;
            if (COM_ABS(R0 - R2) < Beta) {
                FlatnessR++;
            }

            switch (FlatnessL + FlatnessR) {
            case 6:
                fs = ((R1 == R0) && ((L0 == L1))) ? 4 : 3;
                break;
            case 5:
                fs = ((R1 == R0) && ((L0 == L1))) ? 3 : 2;
                break;
            case 4:
                fs = (FlatnessL == 2) ? 2 : 1;
                break;
            case 3:
                fs = (COM_ABS(L1 - R1) < Beta) ? 1 : 0;
                break;
            default:
                fs = 0;
                break;
            }

            switch (fs) {
            case 4:
                SrcPtr[-1] = (pel_t)((L0 + ((L0 + L2) << 3) + L2 + (R0 << 3) + (R2 << 2) + (R2 << 1) + 16) >> 5);             //L0
                SrcPtr[-2] = (pel_t)(((L0 << 3) - L0 + (L2 << 2) + (L2 << 1) + R0 + (R0 << 1) + 8) >> 4);           //L1
                SrcPtr[-3] = (pel_t)(((L0 << 2) + L2 + (L2 << 1) + R0 + 4) >> 3);       //L2
                SrcPtr[0] = (pel_t)((R0 + ((R0 + R2) << 3) + R2 + (L0 << 3) + (L2 << 2) + (L2 << 1) + 16) >> 5);             //R0
                SrcPtr[1] = (pel_t)(((R0 << 3) - R0 + (R2 << 2) + (R2 << 1) + L0 + (L0 << 1) + 8) >> 4);           //R1
                SrcPtr[2] = (pel_t)(((R0 << 2) + R2 + (R2 << 1) + L0 + 4) >> 3);       //R2
                break;
            case 3:
                SrcPtr[-1] = (pel_t)((L2 + (L1 << 2) + (L0 << 2) + (L0 << 1) + (R0 << 2) + R1 + 8) >> 4);           //L0
                SrcPtr[-2] = (pel_t)((L2 + (L2 << 1) + (L1 << 3) + (L0 << 2) + R0 + 8) >> 4);
                SrcPtr[0] = (pel_t)((L1 + (L0 << 2) + (R0 << 2) + (R0 << 1) + (R1 << 2) + R2 + 8) >> 4);           //R0

                SrcPtr[1] = (pel_t)((R2 + (R2 << 1) + (R1 << 3) + (R0 << 2) + L0 + 8) >> 4);
                break;
            case 2:
                SrcPtr[-1] = (pel_t)(((L1 << 1) + L1 + (L0 << 3) + (L0 << 1) + (R0 << 1) + R0 + 8) >> 4);
                SrcPtr[0] = (pel_t)(((L0 << 1) + L0 + (R0 << 3) + (R0 << 1) + (R1 << 1) + R1 + 8) >> 4);
                break;
            case 1:
                SrcPtr[-1] = (pel_t)((L0 + (L0 << 1) + R0 + 2) >> 2);
                SrcPtr[0] = (pel_t)((R0 + (R0 << 1) + L0 + 2) >> 2);
                break;
            default:
                break;
            }
        }
    }
}

static void deblock_edge_ver_c(pel_t *SrcPtrU, pel_t *SrcPtrV, int stride, int Alpha, int Beta, uchar_t *flt_flag)
{
    int pel;
    int AbsDelta;
    int L2, L1, L0, R0, R1, R2;
    int fs;                     //fs stands for filtering strength.  The larger fs is, the stronger filter is applied.
    int FlatnessL, FlatnessR;   // FlatnessL and FlatnessR describe how flat the curve is of one macroblock.
    pel_t *SrcPtrs[2];
    int uv;
    pel_t *SrcPtr;
    int flt_len = 4;

    if (!(flt_flag[0] & flt_flag[1])) {
        flt_len = 2;
        if (flt_flag[1]) {
            SrcPtrU += 2 * stride;
            SrcPtrV += 2 * stride;
        }
    }

    SrcPtrs[0] = SrcPtrU;
    SrcPtrs[1] = SrcPtrV;

    for (uv = 0; uv < 2; uv++) {
        SrcPtr = SrcPtrs[uv];
        for (pel = 0; pel < flt_len; pel++, SrcPtr += stride) {
            L0 = SrcPtr[-1];
            R0 = SrcPtr[0];
            AbsDelta = COM_ABS(R0 - L0);

            if ((AbsDelta < Alpha) && (AbsDelta > 1)) {
                L2 = SrcPtr[-3];
                L1 = SrcPtr[-2];
                R1 = SrcPtr[1];
                R2 = SrcPtr[2];
                FlatnessL = (COM_ABS(L1 - L0) < Beta) ? 2 : 0;
                if (COM_ABS(L2 - L0) < Beta) {
                    FlatnessL++;
                }

                FlatnessR = (COM_ABS(R0 - R1) < Beta) ? 2 : 0;
                if (COM_ABS(R0 - R2) < Beta) {
                    FlatnessR++;
                }

                switch (FlatnessL + FlatnessR) {
                case 6:
                    fs = ((R1 == R0) && ((L0 == L1))) ? 3 : 2;
                    break;
                case 5:
                    fs = ((R1 == R0) && ((L0 == L1))) ? 2 : 1;
                    break;
                case 4:
                    fs = (FlatnessL == 2) ? 1 : 0;
                    break;
                default:
                    fs = 0;
                    break;
                }

                switch (fs) {
                case 4:
                    SrcPtr[-1] = (pel_t)((L0 + ((L0 + L2) << 3) + L2 + (R0 << 3) + (R2 << 2) + (R2 << 1) + 16) >> 5);             //L0
                    SrcPtr[-2] = (pel_t)(((L0 << 3) - L0 + (L2 << 2) + (L2 << 1) + R0 + (R0 << 1) + 8) >> 4);           //L1
                    SrcPtr[-3] = (pel_t)(((L0 << 2) + L2 + (L2 << 1) + R0 + 4) >> 3);       //L2
                    SrcPtr[0] = (pel_t)((R0 + ((R0 + R2) << 3) + R2 + (L0 << 3) + (L2 << 2) + (L2 << 1) + 16) >> 5);             //R0
                    SrcPtr[1] = (pel_t)(((R0 << 3) - R0 + (R2 << 2) + (R2 << 1) + L0 + (L0 << 1) + 8) >> 4);           //R1
                    SrcPtr[2] = (pel_t)(((R0 << 2) + R2 + (R2 << 1) + L0 + 4) >> 3);       //R2
                    break;
                case 3:
                    SrcPtr[-1] = (pel_t)((L2 + (L1 << 2) + (L0 << 2) + (L0 << 1) + (R0 << 2) + R1 + 8) >> 4);           //L0
                    SrcPtr[-2] = (pel_t)((L2 + (L2 << 1) + (L1 << 3) + (L0 << 2) + R0 + 8) >> 4);
                    SrcPtr[0] = (pel_t)((L1 + (L0 << 2) + (R0 << 2) + (R0 << 1) + (R1 << 2) + R2 + 8) >> 4);           //R0

                    SrcPtr[1] = (pel_t)((R2 + (R2 << 1) + (R1 << 3) + (R0 << 2) + L0 + 8) >> 4);
                    break;
                case 2:
                    SrcPtr[-1] = (pel_t)(((L1 << 1) + L1 + (L0 << 3) + (L0 << 1) + (R0 << 1) + R0 + 8) >> 4);
                    SrcPtr[0] = (pel_t)(((L0 << 1) + L0 + (R0 << 3) + (R0 << 1) + (R1 << 1) + R1 + 8) >> 4);
                    break;
                case 1:
                    SrcPtr[-1] = (pel_t)((L0 + (L0 << 1) + R0 + 2) >> 2);
                    SrcPtr[0] = (pel_t)((R0 + (R0 << 1) + L0 + 2) >> 2);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

static void deblock_edge_hor(pel_t *SrcPtr, int stride, int Alpha, int Beta, uchar_t *flt_flag)
{
    int inc = stride;
    int inc2 = inc << 1;
    int inc3 = inc + inc2;
    int pel;
    int AbsDelta;
    int L2, L1, L0, R0, R1, R2;
    int fs_buf[16];
    int *fs = fs_buf;                     //fs stands for filtering strength.  The larger fs is, the stronger filter is applied.
    int FlatnessL, FlatnessR;   // FlatnessL and FlatnessR describe how flat the curve is of one macroblock.
    int flt_len = 8;

    if (!(flt_flag[0] & flt_flag[1])) {
        flt_len = 4;
        if (flt_flag[1]) {
            SrcPtr += 4;
        }
    }

    for (pel = 0; pel < flt_len; pel++, SrcPtr++) {
        L0 = SrcPtr[-inc];
        R0 = SrcPtr[0];
        AbsDelta = COM_ABS(R0 - L0);
        *fs = 0;

        if ((AbsDelta < Alpha) && (AbsDelta > 1)) {
            L2 = SrcPtr[-inc3];
            L1 = SrcPtr[-inc2];
            R1 = SrcPtr[inc];
            R2 = SrcPtr[inc2];
            FlatnessL = (COM_ABS(L1 - L0) < Beta) ? 2 : 0;
            if (COM_ABS(L2 - L0) < Beta) {
                FlatnessL++;
            }

            FlatnessR = (COM_ABS(R0 - R1) < Beta) ? 2 : 0;
            if (COM_ABS(R0 - R2) < Beta) {
                FlatnessR++;
            }

            switch (FlatnessL + FlatnessR) {
            case 6:
                *fs = ((R1 == R0) && ((L0 == L1))) ? 4 : 3;
                break;
            case 5:
                *fs = ((R1 == R0) && ((L0 == L1))) ? 3 : 2;
                break;
            case 4:
                *fs = (FlatnessL == 2) ? 2 : 1;
                break;
            case 3:
                *fs = (COM_ABS(L1 - R1) < Beta) ? 1 : 0;
                break;
            default:
                *fs = 0;
                break;
            }

            switch (*fs) {
            case 4:
                SrcPtr[-inc] = (pel_t)((L0 + ((L0 + L2) << 3) + L2 + (R0 << 3) + (R2 << 2) + (R2 << 1) + 16) >> 5);             //L0
                SrcPtr[-inc2] = (pel_t)(((L0 << 3) - L0 + (L2 << 2) + (L2 << 1) + R0 + (R0 << 1) + 8) >> 4);           //L1
                SrcPtr[-inc3] = (pel_t)(((L0 << 2) + L2 + (L2 << 1) + R0 + 4) >> 3);       //L2
                SrcPtr[0] = (pel_t)((R0 + ((R0 + R2) << 3) + R2 + (L0 << 3) + (L2 << 2) + (L2 << 1) + 16) >> 5);             //R0
                SrcPtr[inc] = (pel_t)(((R0 << 3) - R0 + (R2 << 2) + (R2 << 1) + L0 + (L0 << 1) + 8) >> 4);           //R1
                SrcPtr[inc2] = (pel_t)(((R0 << 2) + R2 + (R2 << 1) + L0 + 4) >> 3);       //R2
                break;
            case 3:
                SrcPtr[-inc] = (pel_t)((L2 + (L1 << 2) + (L0 << 2) + (L0 << 1) + (R0 << 2) + R1 + 8) >> 4);           //L0
                SrcPtr[-inc2] = (pel_t)((L2 + (L2 << 1) + (L1 << 3) + (L0 << 2) + R0 + 8) >> 4);
                SrcPtr[0] = (pel_t)((L1 + (L0 << 2) + (R0 << 2) + (R0 << 1) + (R1 << 2) + R2 + 8) >> 4);           //R0

                SrcPtr[inc] = (pel_t)((R2 + (R2 << 1) + (R1 << 3) + (R0 << 2) + L0 + 8) >> 4);
                break;
            case 2:
                SrcPtr[-inc] = (pel_t)(((L1 << 1) + L1 + (L0 << 3) + (L0 << 1) + (R0 << 1) + R0 + 8) >> 4);
                SrcPtr[0] = (pel_t)(((L0 << 1) + L0 + (R0 << 3) + (R0 << 1) + (R1 << 1) + R1 + 8) >> 4);
                break;
            case 1:
                SrcPtr[-inc] = (pel_t)((L0 + (L0 << 1) + R0 + 2) >> 2);
                SrcPtr[0] = (pel_t)((R0 + (R0 << 1) + L0 + 2) >> 2);
                break;
            default:
                break;
            }
        }

        fs++;
    }
}

static void deblock_edge_hor_c(pel_t *SrcPtrU, pel_t *SrcPtrV, int stride, int Alpha, int Beta, uchar_t *flt_flag)
{
    int inc = stride;
    int inc2 = inc << 1;
    int inc3 = inc + inc2;
    int pel;
    int AbsDelta;
    int L2, L1, L0, R0, R1, R2;
    int fs_buf[8];
    int *fs;                    
    int FlatnessL, FlatnessR;   
    int uv;
    pel_t *PtrSrc[2];
    pel_t *SrcPtr;
    int flt_len = 4;

    if (!(flt_flag[0] & flt_flag[1])) {
        flt_len = 2;
        if (flt_flag[1]) {
            SrcPtrU += 2;
            SrcPtrV += 2;
        }
    }

    PtrSrc[0] = SrcPtrU;
    PtrSrc[1] = SrcPtrV;

    for (uv = 0; uv < 2; uv++) {
        fs = fs_buf;
        SrcPtr = PtrSrc[uv];
        for (pel = 0; pel < flt_len; pel++, SrcPtr++) {
            L0 = SrcPtr[-inc];
            R0 = SrcPtr[0];
            AbsDelta = COM_ABS(R0 - L0);
            *fs = 0;

            if ((AbsDelta < Alpha) && (AbsDelta > 1)) {
                L2 = SrcPtr[-inc3];
                L1 = SrcPtr[-inc2];
                R1 = SrcPtr[inc];
                R2 = SrcPtr[inc2];
                FlatnessL = (COM_ABS(L1 - L0) < Beta) ? 2 : 0;
                if (COM_ABS(L2 - L0) < Beta) {
                    FlatnessL++;
                }

                FlatnessR = (COM_ABS(R0 - R1) < Beta) ? 2 : 0;
                if (COM_ABS(R0 - R2) < Beta) {
                    FlatnessR++;
                }

                switch (FlatnessL + FlatnessR) {
                case 6:
                    *fs = ((R1 == R0) && ((L0 == L1))) ? 3 : 2;
                    break;
                case 5:
                    *fs = ((R1 == R0) && ((L0 == L1))) ? 2 : 1;
                    break;
                case 4:
                    *fs = (FlatnessL == 2) ? 1 : 0;
                    break;
                default:
                    *fs = 0;
                    break;
                }

                switch (*fs) {
                case 4:
                    SrcPtr[-inc] = (pel_t)((L0 + ((L0 + L2) << 3) + L2 + (R0 << 3) + (R2 << 2) + (R2 << 1) + 16) >> 5);             //L0
                    SrcPtr[-inc2] = (pel_t)(((L0 << 3) - L0 + (L2 << 2) + (L2 << 1) + R0 + (R0 << 1) + 8) >> 4);           //L1
                    SrcPtr[-inc3] = (pel_t)(((L0 << 2) + L2 + (L2 << 1) + R0 + 4) >> 3);       //L2
                    SrcPtr[0] = (pel_t)((R0 + ((R0 + R2) << 3) + R2 + (L0 << 3) + (L2 << 2) + (L2 << 1) + 16) >> 5);             //R0
                    SrcPtr[inc] = (pel_t)(((R0 << 3) - R0 + (R2 << 2) + (R2 << 1) + L0 + (L0 << 1) + 8) >> 4);           //R1
                    SrcPtr[inc2] = (pel_t)(((R0 << 2) + R2 + (R2 << 1) + L0 + 4) >> 3);       //R2
                    break;
                case 3:
                    SrcPtr[-inc] = (pel_t)((L2 + (L1 << 2) + (L0 << 2) + (L0 << 1) + (R0 << 2) + R1 + 8) >> 4);           //L0
                    SrcPtr[-inc2] = (pel_t)((L2 + (L2 << 1) + (L1 << 3) + (L0 << 2) + R0 + 8) >> 4);
                    SrcPtr[0] = (pel_t)((L1 + (L0 << 2) + (R0 << 2) + (R0 << 1) + (R1 << 2) + R2 + 8) >> 4);           //R0

                    SrcPtr[inc] = (pel_t)((R2 + (R2 << 1) + (R1 << 3) + (R0 << 2) + L0 + 8) >> 4);
                    break;
                case 2:
                    SrcPtr[-inc] = (pel_t)(((L1 << 1) + L1 + (L0 << 3) + (L0 << 1) + (R0 << 1) + R0 + 8) >> 4);
                    SrcPtr[0] = (pel_t)(((L0 << 1) + L0 + (R0 << 3) + (R0 << 1) + (R1 << 1) + R1 + 8) >> 4);
                    break;
                case 1:
                    SrcPtr[-inc] = (pel_t)((L0 + (L0 << 1) + R0 + 2) >> 2);
                    SrcPtr[0] = (pel_t)((R0 + (R0 << 1) + L0 + 2) >> 2);
                    break;
                default:
                    break;
                }
            }
            fs++;
        }
    }
}

void com_funs_init_deblock_filter()
{
    g_funs_handle.deblock_edge[0] = deblock_edge_ver;
    g_funs_handle.deblock_edge[1] = deblock_edge_hor;
    g_funs_handle.deblock_edge_c[0] = deblock_edge_ver_c;
    g_funs_handle.deblock_edge_c[1] = deblock_edge_hor_c;

    g_funs_handle.sao_flt = SAO_on_block;
}
