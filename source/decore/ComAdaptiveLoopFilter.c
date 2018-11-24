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
#include "ComAdaptiveLoopFilter.h"

#define  IClip( Min, Max, Val) (((Val)<(Min))? (Min):(((Val)>(Max))? (Max):(Val)))


void reconstructCoefficients(const ALFParam *alfParam, int filterCoeff[16][9])
{
    int g, sum, i, coeffPred;
    for (g = 0; g < alfParam->filters_per_group; g++) {
        sum = 0;
        for (i = 0; i < ALF_MAX_NUM_COEF - 1; i++) {
            sum += (2 * alfParam->coeffmulti[g][i]);
            filterCoeff[g][i] = alfParam->coeffmulti[g][i];
        }
        coeffPred = (1 << 6) - sum;
        filterCoeff[g][ALF_MAX_NUM_COEF - 1] = coeffPred + alfParam->coeffmulti[g][ALF_MAX_NUM_COEF - 1];
    }
}
void reconstructCoefInfo(int compIdx, const ALFParam *alfParam, int filterCoeff[16][9], int *varIndTab)
{
    int i;
    if (compIdx == ALF_Y) {
        memset(varIndTab, 0, NO_VAR_BINS * sizeof(int));
        if (alfParam->filters_per_group > 1) {
            for (i = 1; i < NO_VAR_BINS; ++i) {
                if (alfParam->filterPattern[i]) {
                    varIndTab[i] = varIndTab[i - 1] + 1;
                } else {
                    varIndTab[i] = varIndTab[i - 1];
                }
            }
        }
    }
    reconstructCoefficients(alfParam, filterCoeff);
}

void alf_flt_one_block(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcu_height, int xPos, int lcu_width, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail) 
{
    pel_t *imgPad1,*imgPad2,*imgPad3, *imgPad4, *imgPad5, *imgPad6;

    int i, j;
    int startPos = isAboveAvail ? (yPos-4) : yPos;
    int endPos   = isBelowAvail ? (yPos + lcu_height - 4) : (yPos + lcu_height);

    int xPosEnd = xPos + lcu_width;

    imgPad += (startPos*stride);
    imgRes += (startPos*stride);

    for(i=startPos; i < endPos; i++)
    {
        int yUp		= Clip3(startPos, endPos-1, i-1);
        int yBottom = Clip3(startPos, endPos-1, i+1);
        imgPad1 = imgPad + (yBottom-i)*stride;
        imgPad2 = imgPad + (yUp - i)*stride;

        yUp		= Clip3(startPos, endPos-1, i-2);
        yBottom = Clip3(startPos, endPos-1, i+2);
        imgPad3 = imgPad + (yBottom-i)*stride;
        imgPad4 = imgPad + (yUp-i)*stride;

        yUp		= Clip3(startPos, endPos-1, i-3);
        yBottom = Clip3(startPos, endPos-1, i+3);
        imgPad5 = imgPad + (yBottom-i)*stride;
        imgPad6 = imgPad + (yUp-i)*stride;

        for (j = xPos; j < xPosEnd; j++)
        {
            int pixelInt;
            int xLeft = j - 1;
            int xRight = j + 1;

            pixelInt = coef[0] * (imgPad5[j] + imgPad6[j]);
            pixelInt += coef[1] * (imgPad3[j] + imgPad4[j]);
            pixelInt += coef[2] * (imgPad1[xRight] + imgPad2[xLeft]);
            pixelInt += coef[3] * (imgPad1[j] + imgPad2[j]);
            pixelInt += coef[4] * (imgPad1[xLeft] + imgPad2[xRight]);
            pixelInt += coef[7] * (imgPad[xRight] + imgPad[xLeft]);

            xLeft = j - 2;
            xRight = j + 2;
            pixelInt += coef[6] * (imgPad[xRight] + imgPad[xLeft]);

            xLeft = j - 3;
            xRight = j + 3;
            pixelInt += coef[5] * (imgPad[xRight] + imgPad[xLeft]);

            pixelInt += coef[8] * (imgPad[j]);

            pixelInt = (int)((pixelInt + 32) >> 6);

            imgRes[j] = IClip(0, ((1 << sample_bit_depth) - 1), pixelInt);

        }
        imgPad += stride;
        imgRes += stride;
    }  
}

void alf_flt_one_block_fix(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcu_height, int xPos, int lcu_width, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail)
{
    pel_t *imgPad1, *imgPad2, *imgPad3, *imgPad4, *imgPad5, *imgPad6;
    int pixelInt;
    int startPos = isAboveAvail ? (yPos - 4) : yPos;
    int endPos = isBelowAvail ? (yPos + lcu_height - 4) : (yPos + lcu_height);

    int xPosEnd = xPos + lcu_width;

    /* first line */
    imgPad += (startPos * stride) + xPos;
    imgRes += (startPos * stride) + xPos;

    if (imgPad[0] != imgPad[-1]) {
        imgPad1 = imgPad + 1 * stride;
        imgPad2 = imgPad;
        imgPad3 = imgPad + 2 * stride;
        imgPad4 = imgPad;
        imgPad5 = imgPad + 3 * stride;
        imgPad6 = imgPad;

        pixelInt  = coef[0] * (imgPad5[ 0] + imgPad6[ 0]);
        pixelInt += coef[1] * (imgPad3[ 0] + imgPad4[ 0]);
        pixelInt += coef[2] * (imgPad1[ 1] + imgPad2[ 0]);
        pixelInt += coef[3] * (imgPad1[ 0] + imgPad2[ 0]);
        pixelInt += coef[4] * (imgPad1[-1] + imgPad2[ 1]);
        pixelInt += coef[7] * (imgPad [ 1] + imgPad [-1]);
        pixelInt += coef[6] * (imgPad [ 2] + imgPad [-2]);
        pixelInt += coef[5] * (imgPad [ 3] + imgPad [-3]);
        pixelInt += coef[8] * (imgPad [ 0]);

        pixelInt = (int)((pixelInt + 32) >> 6);
        imgRes[0] = IClip(0, ((1 << sample_bit_depth) - 1), pixelInt);
    }

    imgPad += lcu_width - 1;
    imgRes += lcu_width - 1;

    if (imgPad[0] != imgPad[1]) {
        imgPad1 = imgPad + 1 * stride;
        imgPad2 = imgPad;
        imgPad3 = imgPad + 2 * stride;
        imgPad4 = imgPad;
        imgPad5 = imgPad + 3 * stride;
        imgPad6 = imgPad;

        pixelInt  = coef[0] * (imgPad5[ 0] + imgPad6[ 0]);
        pixelInt += coef[1] * (imgPad3[ 0] + imgPad4[ 0]);
        pixelInt += coef[2] * (imgPad1[ 1] + imgPad2[-1]);
        pixelInt += coef[3] * (imgPad1[ 0] + imgPad2[ 0]);
        pixelInt += coef[4] * (imgPad1[-1] + imgPad2[ 0]);
        pixelInt += coef[7] * (imgPad [ 1] + imgPad [-1]);
        pixelInt += coef[6] * (imgPad [ 2] + imgPad [-2]);
        pixelInt += coef[5] * (imgPad [ 3] + imgPad [-3]);
        pixelInt += coef[8] * (imgPad [ 0]);

        pixelInt = (int)((pixelInt + 32) >> 6);
        imgRes[0] = IClip(0, ((1 << sample_bit_depth) - 1), pixelInt);
    }

    /* last line */
    imgPad -= lcu_width - 1;
    imgRes -= lcu_width - 1;
    imgPad += ((endPos - startPos - 1) * stride);
    imgRes += ((endPos - startPos - 1) * stride);

    if (imgPad[0] != imgPad[-1]) {
        imgPad1 = imgPad;
        imgPad2 = imgPad - 1 * stride;
        imgPad3 = imgPad;
        imgPad4 = imgPad - 2 * stride;
        imgPad5 = imgPad;
        imgPad6 = imgPad - 3 * stride;

        pixelInt  = coef[0] * (imgPad5[ 0] + imgPad6[ 0]);
        pixelInt += coef[1] * (imgPad3[ 0] + imgPad4[ 0]);
        pixelInt += coef[2] * (imgPad1[ 1] + imgPad2[-1]);
        pixelInt += coef[3] * (imgPad1[ 0] + imgPad2[ 0]);
        pixelInt += coef[4] * (imgPad1[ 0] + imgPad2[ 1]);
        pixelInt += coef[7] * (imgPad [ 1] + imgPad [-1]);
        pixelInt += coef[6] * (imgPad [ 2] + imgPad [-2]);
        pixelInt += coef[5] * (imgPad [ 3] + imgPad [-3]);
        pixelInt += coef[8] * (imgPad [ 0]);

        pixelInt = (int)((pixelInt + 32) >> 6);
        imgRes[0] = IClip(0, ((1 << sample_bit_depth) - 1), pixelInt);
    }

    imgPad += lcu_width - 1;
    imgRes += lcu_width - 1;

    if (imgPad[0] != imgPad[1]) {
        imgPad1 = imgPad;
        imgPad2 = imgPad - 1 * stride;
        imgPad3 = imgPad;
        imgPad4 = imgPad - 2 * stride;
        imgPad5 = imgPad;
        imgPad6 = imgPad - 3 * stride;

        pixelInt  = coef[0] * (imgPad5[ 0] + imgPad6[ 0]);
        pixelInt += coef[1] * (imgPad3[ 0] + imgPad4[ 0]);
        pixelInt += coef[2] * (imgPad1[ 0] + imgPad2[-1]);
        pixelInt += coef[3] * (imgPad1[ 0] + imgPad2[ 0]);
        pixelInt += coef[4] * (imgPad1[-1] + imgPad2[ 1]);
        pixelInt += coef[7] * (imgPad [ 1] + imgPad [-1]);
        pixelInt += coef[6] * (imgPad [ 2] + imgPad [-2]);
        pixelInt += coef[5] * (imgPad [ 3] + imgPad [-3]);
        pixelInt += coef[8] * (imgPad [ 0]);

        pixelInt = (int)((pixelInt + 32) >> 6);
        imgRes[0] = (pel_t) IClip(0, ((1 << sample_bit_depth) - 1), pixelInt);
    }
}



void com_funs_init_alf_filter()
{
    g_funs_handle.alf_flt = alf_flt_one_block;
    g_funs_handle.alf_flt_fix = alf_flt_one_block_fix;
}
