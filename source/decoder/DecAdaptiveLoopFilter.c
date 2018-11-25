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
#include <string.h>
#include <assert.h>
#include <math.h>
#include "global.h"
#include "commonVariables.h"

#include "commonStructures.h"
#include "ComAdaptiveLoopFilter.h"
#include "DecAdaptiveLoopFilter.h"
#include "AEC.h"
#include "bitstream.h"

#define Clip_post(high,val) ((val > high)? high: val)

static void deriveBoundaryAvail(com_rec_t *rec, int numLCUInPicWidth, int numLCUInPicHeight, int ctu, int *isLeftAvail, int *isRightAvail, int *isAboveAvail, int *isBelowAvail)
{
    int numLCUsInFrame = numLCUInPicHeight * numLCUInPicWidth;
    int  lcuHeight = 1 << rec->seq->g_uiMaxSizeInBit;
    int  lcuWidth = lcuHeight;
    int  img_height = rec->seq->img_height;
    int  img_width = rec->seq->img_width;
    com_cu_t *cu_array = rec->cu_array;
    int  NumCUInFrame;
    int  pic_x;
    int  pic_y;
    int  mb_x;
    int  mb_y;
    int  mb_nr;
    int  pic_mb_width = img_width / 8;
    int  cuCurrNum;

    int curSliceNr, neighorSliceNr;

    NumCUInFrame = numLCUInPicHeight * numLCUInPicWidth;

    pic_x = (ctu % numLCUInPicWidth) * lcuWidth;
    pic_y = (ctu / numLCUInPicWidth) * lcuHeight;

    pic_mb_width += (img_width % 8) ? 1 : 0;

    mb_x = pic_x / 8;
    mb_y = pic_y / 8;
    mb_nr = mb_y * pic_mb_width + mb_x;
    cuCurrNum = mb_nr;

    *isLeftAvail = (ctu % numLCUInPicWidth != 0);
    *isRightAvail = (ctu % numLCUInPicWidth != numLCUInPicWidth - 1);
    *isAboveAvail = (ctu >= numLCUInPicWidth);
    *isBelowAvail = (ctu < numLCUsInFrame - numLCUInPicWidth);

    if (!rec->seq->lf_cross_slice) {
        com_cu_t *cuCurr = &(cu_array[cuCurrNum]);
        com_cu_t *cuLeft = *isLeftAvail ? &(cu_array[cuCurrNum - 1]) : NULL;
        com_cu_t *cuRight = *isRightAvail ? &(cu_array[cuCurrNum + 1]) : NULL;
        com_cu_t *cuAbove = *isAboveAvail ? &(cu_array[cuCurrNum - pic_mb_width]) : NULL;

        *isLeftAvail = *isRightAvail = *isAboveAvail = 0;

        curSliceNr = cuCurr->slice_nr;
        if (cuLeft != NULL) {
            neighorSliceNr = cuLeft->slice_nr;
            if (curSliceNr == neighorSliceNr) {
                *isLeftAvail = 1;
            }
        }

        if (cuRight != NULL) {
            neighorSliceNr = cuRight->slice_nr;
            if (curSliceNr == neighorSliceNr) {
                *isRightAvail = 1;
            }
        }

        if (cuAbove != NULL) {
            neighorSliceNr = cuAbove->slice_nr;
            if (curSliceNr == neighorSliceNr) {
                *isAboveAvail = 1;
            }
        }
    }
}


/*
*************************************************************************
* Function: ALF filter on CTB
*************************************************************************
*/
static void filterOneCTB(com_rec_t *rec, pel_t *pRest, pel_t *pDec, int stride, int compIdx, const ALFParam *alfParam, int ctuYPos, int ctuHeight, int ctuXPos, int ctuWidth
    , int isAboveAvail, int isBelowAvail, int sample_bit_depth)
{
    const int skipSize = (ALF_FOOTPRINT_SIZE >> 1); //half size of 7x7cross+ 3x3square
    const int formatShift = (compIdx == ALF_Y) ? 0 : 1;
    int ypos, xpos, height, width;
    int mergeIdx;
    int *coef;
    alf_param_t *param = rec->alf_params;
    reconstructCoefInfo(compIdx, alfParam, param->m_filterCoeffSym, param->m_varIndTab);

    ypos = (ctuYPos >> formatShift);
    height = (ctuHeight >> formatShift);
    xpos = (ctuXPos >> formatShift);
    width = (ctuWidth >> formatShift);

    mergeIdx = param->m_varImg[ypos / 4 * rec->seq->b4_info_stride + xpos / 4];
    coef = (compIdx != ALF_Y) ? param->m_filterCoeffSym[0] : param->m_filterCoeffSym[param->m_varIndTab[mergeIdx]];

    g_funs_handle.alf_flt(pRest, pDec, stride, (compIdx != ALF_Y), ypos, height, xpos, width, coef, sample_bit_depth, isAboveAvail, isBelowAvail);
    g_funs_handle.alf_flt_fix(pRest, pDec, stride, (compIdx != ALF_Y), ypos, height, xpos, width, coef, sample_bit_depth, isAboveAvail, isBelowAvail);
}

void alf_lcu_row(com_rec_t *rec, const ALFParam *alfParam, int sample_bit_depth, int pix_y)
{
    int ctu, ctuy, ctux, numLCUInPicWidth, numLCUInPicHeight;
    int y, x, ctuHeight, ctuWidth;
    int lcuHeight, lcuWidth, img_height, img_width;
    int  isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail;
    com_pic_t *pic = rec->frm_cur->yuv_data;
    const seq_info_t *seq = rec->seq;
    char_t(*enable_flag)[3] = rec->alf_params->m_AlfLCUEnabled;
    lcuHeight = 1 << seq->g_uiMaxSizeInBit;
    lcuWidth = lcuHeight;
    img_height = seq->img_height;
    img_width = seq->img_width;
    numLCUInPicWidth = seq->img_width_in_lcu;
    numLCUInPicHeight = seq->img_height_in_lcu;

    ctuy = pix_y / lcuHeight;
    y = ctuy * lcuHeight;
    ctuHeight = (y + lcuHeight > img_height) ? (img_height - y) : lcuHeight;

    enable_flag += ctuy * numLCUInPicWidth;

    for (ctux = 0; ctux < numLCUInPicWidth; ctux++) {
        ctu = ctuy * numLCUInPicWidth + ctux;
        x = ctux * lcuWidth;
        ctuWidth = (x + lcuWidth > img_width) ? (img_width - x) : lcuWidth;

        deriveBoundaryAvail(rec, numLCUInPicWidth, numLCUInPicHeight, ctu, &isLeftAvail, &isRightAvail, &isAboveAvail, &isBelowAvail);

        if (enable_flag[ctux][0]) {
            filterOneCTB(rec, pic->p_y, rec->g_pic_flt_tmp->p_y, pic->i_stride,  0, &alfParam[0], y, ctuHeight, x, ctuWidth, isAboveAvail, isBelowAvail, sample_bit_depth);
        }
        if (enable_flag[ctux][1]) {
            filterOneCTB(rec, pic->p_u, rec->g_pic_flt_tmp->p_u, pic->i_stridec, 1, &alfParam[1], y, ctuHeight, x, ctuWidth, isAboveAvail, isBelowAvail, sample_bit_depth);
        }
        if (enable_flag[ctux][2]) {
            filterOneCTB(rec, pic->p_v, rec->g_pic_flt_tmp->p_v, pic->i_stridec, 2, &alfParam[2], y, ctuHeight, x, ctuWidth, isAboveAvail, isBelowAvail, sample_bit_depth);
        }
    }
}

int alf_buf_create(avs2_dec_t *h_dec, seq_info_t *seq)
{
    int lcuHeight, lcuWidth, img_height, img_width;
    int NumCUInFrame;
    int i, g;
    int numCoef = (int) ALF_MAX_NUM_COEF;
    int regionTable[NO_VAR_BINS] = {0, 1, 4, 5, 15, 2, 3, 6, 14, 11, 10, 7, 13, 12,  9,  8};
    int xInterval  ;
    int yInterval  ;
    int yIndex, xIndex;
    int yIndexOffset;
    char_t *pvar;
    int i_b4 = seq->b4_info_stride;

    lcuWidth = lcuHeight = 1 << seq->g_uiMaxSizeInBit;
    img_width = seq->img_width;
    img_height = seq->img_height;
    NumCUInFrame = seq->img_size_in_lcu; 

    xInterval = ((((img_width + lcuWidth - 1) / lcuWidth) + 1) / 4 * lcuWidth) ;
    yInterval = ((((img_height + lcuHeight - 1) / lcuHeight) + 1) / 4 * lcuHeight) ;

    h_dec->alf_params.m_AlfLCUEnabled = (char_t(*)[3])com_malloc(NumCUInFrame * sizeof(char_t) * 3);
    h_dec->alf_params.m_varImg = (char_t *)com_malloc(seq->b4_info_size * sizeof(char_t));

    pvar = h_dec->alf_params.m_varImg;

    for (i = 0; i < img_height; i = i + 4) {
        yIndex = (yInterval == 0) ? (3) : (Clip_post(3, i / yInterval));
        yIndexOffset = yIndex * 4 ;
        for (g = 0; g < img_width; g = g + 4) {
            xIndex = (xInterval == 0) ? (3) : (Clip_post(3, g / xInterval));
            pvar[g >> 2] = (char_t) regionTable[yIndexOffset + xIndex];
        }
        pvar += i_b4;
    }

    return NumCUInFrame * sizeof(char_t)* 3 + seq->b4_info_size * sizeof(char_t);
}
void alf_buf_destroy(avs2_dec_t *h_dec)
{
    int lcuHeight, lcuWidth, img_height, img_width;
    int NumCUInFrame, numLCUInPicWidth, numLCUInPicHeight;
    int numCoef = (int) ALF_MAX_NUM_COEF;

    lcuHeight         = 1 << (h_dec->seq->g_uiMaxSizeInBit);
    lcuWidth          = lcuHeight;
    img_height        = h_dec->seq->img_height;
    img_width         = h_dec->seq->img_width;
    numLCUInPicWidth  = img_width / lcuWidth ;
    numLCUInPicHeight = img_height / lcuHeight ;
    numLCUInPicWidth  += (img_width % lcuWidth) ? 1 : 0;
    numLCUInPicHeight += (img_height % lcuHeight) ? 1 : 0;
    NumCUInFrame = numLCUInPicHeight * numLCUInPicWidth;

    com_free(h_dec->alf_params.m_varImg);
    com_free(h_dec->alf_params.m_AlfLCUEnabled);
}

void Read_ALF_param(pic_hdr_t *hdr, bs_stream_t *bs)
{
    int pos;
    hdr->m_alfPictureParam[0].alf_flag = U_V(bs, 1, "alf_pic_flag_Y");
    hdr->m_alfPictureParam[1].alf_flag = U_V(bs, 1, "alf_pic_flag_Cb");
    hdr->m_alfPictureParam[2].alf_flag = U_V(bs, 1, "alf_pic_flag_Cr");

    if (hdr->m_alfPictureParam[0].alf_flag) {
        int pre_symbole, i;
        hdr->m_alfPictureParam[0].filters_per_group = UE_V(bs, "ALF filter number") + 1;

        memset(hdr->m_alfPictureParam[0].filterPattern, 0, NO_VAR_BINS * sizeof(int));
        pre_symbole = 0;
        for (i = 0; i < hdr->m_alfPictureParam[0].filters_per_group; i++) {
            if (i > 0) {
                int symbol;
                if (hdr->m_alfPictureParam[0].filters_per_group != 16) {
                    symbol = UE_V(bs, "Region distance");
                } else {
                    symbol = 1;
                }
                hdr->m_alfPictureParam[0].filterPattern[symbol + pre_symbole] = 1;
                pre_symbole = symbol + pre_symbole;
            }

            for (pos = 0; pos < ALF_MAX_NUM_COEF; pos++) {
                hdr->m_alfPictureParam[0].coeffmulti[i][pos] = SE_V(bs, "Luma ALF coefficients");
            }
        }
    }
    if (hdr->m_alfPictureParam[1].alf_flag) {
        hdr->m_alfPictureParam[1].filters_per_group = 1;
        for (pos = 0; pos < ALF_MAX_NUM_COEF; pos++) {
            hdr->m_alfPictureParam[1].coeffmulti[0][pos] = SE_V(bs, "Chroma ALF coefficients");
        }
    }
    if (hdr->m_alfPictureParam[2].alf_flag) {
        hdr->m_alfPictureParam[2].filters_per_group = 1;
        for (pos = 0; pos < ALF_MAX_NUM_COEF; pos++) {
            hdr->m_alfPictureParam[2].coeffmulti[0][pos] = SE_V(bs, "Chroma ALF coefficients");
        }
    }
}
