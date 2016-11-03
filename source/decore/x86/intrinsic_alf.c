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

#include "intrinsic.h"
#include "../commonStructures.h"

void alf_flt_one_block_sse128(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcu_height, int xPos, int lcu_width, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail)
{
    pel_t *imgPad1, *imgPad2, *imgPad3, *imgPad4, *imgPad5, *imgPad6;

    __m128i T00, T01, T10, T11, T20, T21, T30, T31, T40, T41, T50, T51;
    __m128i T1, T2, T3, T4, T5, T6, T7, T8;
    __m128i E00, E01, E10, E11, E20, E21, E30, E31, E40, E41;
    __m128i C0, C1, C2, C3, C4, C30, C31, C32, C33;
    __m128i S0, S00, S01, S1, S10, S11, S2, S20, S21, S3, S30, S31, S4, S40, S41, S5, S50, S51, S6, S60, S61, S7, S8, SS1, SS2, S;
    __m128i mSwitch1, mSwitch2, mSwitch3, mSwitch4, mSwitch5;
    __m128i mAddOffset;
    __m128i mZero = _mm_set1_epi16(0);
    __m128i mMax = _mm_set1_epi16((short)((1 << sample_bit_depth) - 1));
    __m128i mask;

    int i, j;
    int startPos = isAboveAvail ? (yPos - 4) : yPos;
    int endPos = isBelowAvail ? (yPos + lcu_height - 4) : (yPos + lcu_height);

    int xPosEnd = xPos + lcu_width;
    int xPosEnd16 = xPosEnd - (lcu_width & 0x0f);

    mask = _mm_loadu_si128((__m128i*)(intrinsic_mask[(lcu_width & 15) - 1]));

    imgPad += (startPos*stride);
    imgRes += (startPos*stride);

    C0 = _mm_set1_epi8(coef[0]);
    C1 = _mm_set1_epi8(coef[1]);
    C2 = _mm_set1_epi8(coef[2]);
    C3 = _mm_set1_epi8(coef[3]);
    C4 = _mm_set1_epi8(coef[4]);

    mSwitch1 = _mm_setr_epi8(0, 1, 2, 3, 2, 1, 0, 3, 0, 1, 2, 3, 2, 1, 0, 3);
    C30 = _mm_loadu_si128((__m128i*)&coef[5]);
    C31 = _mm_packs_epi32(C30, C30);
    C32 = _mm_packs_epi16(C31, C31);
    C33 = _mm_shuffle_epi8(C32, mSwitch1);
    mSwitch2 = _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, -1, 1, 2, 3, 4, 5, 6, 7, -1);
    mSwitch3 = _mm_setr_epi8(2, 3, 4, 5, 6, 7, 8, -1, 3, 4, 5, 6, 7, 8, 9, -1);
    mSwitch4 = _mm_setr_epi8(4, 5, 6, 7, 8, 9, 10, -1, 5, 6, 7, 8, 9, 10, 11, -1);
    mSwitch5 = _mm_setr_epi8(6, 7, 8, 9, 10, 11, 12, -1, 7, 8, 9, 10, 11, 12, 13, -1);
    mAddOffset = _mm_set1_epi16(32);

    for (i = startPos; i < endPos; i++) {
        int yUp = Clip3(startPos, endPos - 1, i - 1);
        int yBottom = Clip3(startPos, endPos - 1, i + 1);
        imgPad1 = imgPad + (yBottom - i)*stride;
        imgPad2 = imgPad + (yUp - i)*stride;

        yUp = Clip3(startPos, endPos - 1, i - 2);
        yBottom = Clip3(startPos, endPos - 1, i + 2);
        imgPad3 = imgPad + (yBottom - i)*stride;
        imgPad4 = imgPad + (yUp - i)*stride;

        yUp = Clip3(startPos, endPos - 1, i - 3);
        yBottom = Clip3(startPos, endPos - 1, i + 3);
        imgPad5 = imgPad + (yBottom - i)*stride;
        imgPad6 = imgPad + (yUp - i)*stride;

        for (j = xPos; j < xPosEnd - 15; j += 16) {
            T00 = _mm_loadu_si128((__m128i*)&imgPad6[j]);
            T01 = _mm_loadu_si128((__m128i*)&imgPad5[j]);
            E00 = _mm_unpacklo_epi8(T00, T01);
            E01 = _mm_unpackhi_epi8(T00, T01);
            S00 = _mm_maddubs_epi16(E00, C0);//前8个像素所有C0*P0的结果
            S01 = _mm_maddubs_epi16(E01, C0);//后8个像素所有C0*P0的结果

            T10 = _mm_loadu_si128((__m128i*)&imgPad4[j]);
            T11 = _mm_loadu_si128((__m128i*)&imgPad3[j]);
            E10 = _mm_unpacklo_epi8(T10, T11);
            E11 = _mm_unpackhi_epi8(T10, T11);
            S10 = _mm_maddubs_epi16(E10, C1);//前8个像素所有C1*P1的结果
            S11 = _mm_maddubs_epi16(E11, C1);//后8个像素所有C1*P1的结果

            T20 = _mm_loadu_si128((__m128i*)&imgPad2[j - 1]);
            T21 = _mm_loadu_si128((__m128i*)&imgPad1[j + 1]);
            E20 = _mm_unpacklo_epi8(T20, T21);
            E21 = _mm_unpackhi_epi8(T20, T21);
            S20 = _mm_maddubs_epi16(E20, C2);
            S21 = _mm_maddubs_epi16(E21, C2);

            T30 = _mm_loadu_si128((__m128i*)&imgPad2[j]);
            T31 = _mm_loadu_si128((__m128i*)&imgPad1[j]);
            E30 = _mm_unpacklo_epi8(T30, T31);
            E31 = _mm_unpackhi_epi8(T30, T31);
            S30 = _mm_maddubs_epi16(E30, C3);
            S31 = _mm_maddubs_epi16(E31, C3);

            T40 = _mm_loadu_si128((__m128i*)&imgPad2[j + 1]);
            T41 = _mm_loadu_si128((__m128i*)&imgPad1[j - 1]);
            E40 = _mm_unpacklo_epi8(T40, T41);
            E41 = _mm_unpackhi_epi8(T40, T41);
            S40 = _mm_maddubs_epi16(E40, C4);
            S41 = _mm_maddubs_epi16(E41, C4);

            T50 = _mm_loadu_si128((__m128i*)&imgPad[j - 3]);
            T51 = _mm_loadu_si128((__m128i*)&imgPad[j + 5]);
            T1 = _mm_shuffle_epi8(T50, mSwitch2);
            T2 = _mm_shuffle_epi8(T50, mSwitch3);
            T3 = _mm_shuffle_epi8(T50, mSwitch4);
            T4 = _mm_shuffle_epi8(T50, mSwitch5);
            T5 = _mm_shuffle_epi8(T51, mSwitch2);
            T6 = _mm_shuffle_epi8(T51, mSwitch3);
            T7 = _mm_shuffle_epi8(T51, mSwitch4);
            T8 = _mm_shuffle_epi8(T51, mSwitch5);

            S5 = _mm_maddubs_epi16(T1, C33);
            S6 = _mm_maddubs_epi16(T2, C33);
            S7 = _mm_maddubs_epi16(T3, C33);
            S8 = _mm_maddubs_epi16(T4, C33);
            S50 = _mm_hadds_epi16(S5, S6);
            S51 = _mm_hadds_epi16(S7, S8);
            S5 = _mm_hadds_epi16(S50, S51);//前8个
            S4 = _mm_maddubs_epi16(T5, C33);
            S6 = _mm_maddubs_epi16(T6, C33);
            S7 = _mm_maddubs_epi16(T7, C33);
            S8 = _mm_maddubs_epi16(T8, C33);
            S60 = _mm_hadds_epi16(S4, S6);
            S61 = _mm_hadds_epi16(S7, S8);
            S6 = _mm_hadds_epi16(S60, S61);//后8个

            S0 = _mm_adds_epi16(S00, S10);
            S1 = _mm_adds_epi16(S30, S20);
            S2 = _mm_adds_epi16(S40, S5);
            S3 = _mm_adds_epi16(S1, S0);
            SS1 = _mm_adds_epi16(S2, S3);//前8个

            S0 = _mm_adds_epi16(S01, S11);
            S1 = _mm_adds_epi16(S31, S21);
            S2 = _mm_adds_epi16(S41, S6);
            S3 = _mm_adds_epi16(S1, S0);
            SS2 = _mm_adds_epi16(S2, S3);//后8个


            SS1 = _mm_adds_epi16(SS1, mAddOffset);
            SS1 = _mm_srai_epi16(SS1, 6);
            SS1 = _mm_min_epi16(SS1, mMax);
            SS1 = _mm_max_epi16(SS1, mZero);

            SS2 = _mm_adds_epi16(SS2, mAddOffset);
            SS2 = _mm_srai_epi16(SS2, 6);
            SS2 = _mm_min_epi16(SS2, mMax);
            SS2 = _mm_max_epi16(SS2, mZero);

            S = _mm_packus_epi16(SS1, SS2);
            if (j != xPosEnd16){
                _mm_storeu_si128((__m128i*)(imgRes + j), S);
            } else{
                _mm_maskmoveu_si128(S, mask, (char *)(imgRes + j));
                break;
            }
        }

        imgPad += stride;
        imgRes += stride;
    }
}


void alf_flt_one_block_sse128_10bit(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcuHeight, int xPos, int lcuWidth, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail)
{
    pel_t *imgPad1, *imgPad2, *imgPad3, *imgPad4, *imgPad5, *imgPad6;

    __m128i T00, T01, T10, T11, T20, T21, T30, T31, T40, T41;
    __m128i E00, E01, E10, E11, E20, E21, E30, E31, E40, E41;
    __m128i C0, C1, C2, C3, C4, C5, C6, C7, C8;
    __m128i S00, S01, S10, S11, S20, S21, S30, S31, S40, S41, S50, S51, S60, S61, SS1, SS2, S, S70, S71, S80, S81;
    __m128i mAddOffset;
    __m128i mask;
    __m128i zero = _mm_setzero_si128();
    int max_pixel = (1 << sample_bit_depth) - 1;
    __m128i max_val = _mm_set1_epi16(max_pixel);

    int i, j;
    int startPos = isAboveAvail ? (yPos - 4) : yPos;
    int endPos = isBelowAvail ? (yPos + lcuHeight - 4) : (yPos + lcuHeight);
    int xPosEnd = xPos + lcuWidth;

    imgPad += (startPos*stride);
    imgRes += (startPos*stride);

    C0 = _mm_set1_epi16((pel_t)coef[0]);
    C1 = _mm_set1_epi16((pel_t)coef[1]);
    C2 = _mm_set1_epi16((pel_t)coef[2]);
    C3 = _mm_set1_epi16((pel_t)coef[3]);
    C4 = _mm_set1_epi16((pel_t)coef[4]);
    C5 = _mm_set1_epi16((pel_t)coef[5]);
    C6 = _mm_set1_epi16((pel_t)coef[6]);
    C7 = _mm_set1_epi16((pel_t)coef[7]);
    C8 = _mm_set1_epi16((pel_t)coef[8]);

    mAddOffset = _mm_set1_epi32(32);

    if (lcuWidth & 7){
        int xPosEnd8 = xPosEnd - (lcuWidth & 0x07);
        mask = _mm_loadu_si128((__m128i*)(intrinsic_mask_10bit[(lcuWidth & 7) - 1]));
        for (i = startPos; i < endPos; i++) {
            int yUp = Clip3(startPos, endPos - 1, i - 1);
            int yBottom = Clip3(startPos, endPos - 1, i + 1);
            imgPad1 = imgPad + (yBottom - i)*stride;
            imgPad2 = imgPad + (yUp - i)*stride;

            yUp = Clip3(startPos, endPos - 1, i - 2);
            yBottom = Clip3(startPos, endPos - 1, i + 2);
            imgPad3 = imgPad + (yBottom - i)*stride;
            imgPad4 = imgPad + (yUp - i)*stride;

            yUp = Clip3(startPos, endPos - 1, i - 3);
            yBottom = Clip3(startPos, endPos - 1, i + 3);
            imgPad5 = imgPad + (yBottom - i)*stride;
            imgPad6 = imgPad + (yUp - i)*stride;

            for (j = xPos; j < xPosEnd; j += 8) {
                T00 = _mm_loadu_si128((__m128i*)&imgPad6[j]);
                T01 = _mm_loadu_si128((__m128i*)&imgPad5[j]);
                E00 = _mm_unpacklo_epi16(T00, T01);
                E01 = _mm_unpackhi_epi16(T00, T01);
                S00 = _mm_madd_epi16(E00, C0);
                S01 = _mm_madd_epi16(E01, C0);

                T10 = _mm_loadu_si128((__m128i*)&imgPad4[j]);
                T11 = _mm_loadu_si128((__m128i*)&imgPad3[j]);
                E10 = _mm_unpacklo_epi16(T10, T11);
                E11 = _mm_unpackhi_epi16(T10, T11);
                S10 = _mm_madd_epi16(E10, C1);
                S11 = _mm_madd_epi16(E11, C1);

                T20 = _mm_loadu_si128((__m128i*)&imgPad2[j - 1]);
                T21 = _mm_loadu_si128((__m128i*)&imgPad1[j + 1]);
                E20 = _mm_unpacklo_epi16(T20, T21);
                E21 = _mm_unpackhi_epi16(T20, T21);
                S20 = _mm_madd_epi16(E20, C2);
                S21 = _mm_madd_epi16(E21, C2);

                T30 = _mm_loadu_si128((__m128i*)&imgPad2[j]);
                T31 = _mm_loadu_si128((__m128i*)&imgPad1[j]);
                E30 = _mm_unpacklo_epi16(T30, T31);
                E31 = _mm_unpackhi_epi16(T30, T31);
                S30 = _mm_madd_epi16(E30, C3);
                S31 = _mm_madd_epi16(E31, C3);

                T40 = _mm_loadu_si128((__m128i*)&imgPad2[j + 1]);
                T41 = _mm_loadu_si128((__m128i*)&imgPad1[j - 1]);
                E40 = _mm_unpacklo_epi16(T40, T41);
                E41 = _mm_unpackhi_epi16(T40, T41);
                S40 = _mm_madd_epi16(E40, C4);
                S41 = _mm_madd_epi16(E41, C4);

                T40 = _mm_loadu_si128((__m128i*)&imgPad[j - 3]);
                T41 = _mm_loadu_si128((__m128i*)&imgPad[j + 3]);
                E40 = _mm_unpacklo_epi16(T40, T41);
                E41 = _mm_unpackhi_epi16(T40, T41);
                S50 = _mm_madd_epi16(E40, C5);
                S51 = _mm_madd_epi16(E41, C5);

                T40 = _mm_loadu_si128((__m128i*)&imgPad[j - 2]);
                T41 = _mm_loadu_si128((__m128i*)&imgPad[j + 2]);
                E40 = _mm_unpacklo_epi16(T40, T41);
                E41 = _mm_unpackhi_epi16(T40, T41);
                S60 = _mm_madd_epi16(E40, C6);
                S61 = _mm_madd_epi16(E41, C6);

                T40 = _mm_loadu_si128((__m128i*)&imgPad[j - 1]);
                T41 = _mm_loadu_si128((__m128i*)&imgPad[j + 1]);
                E40 = _mm_unpacklo_epi16(T40, T41);
                E41 = _mm_unpackhi_epi16(T40, T41);
                S70 = _mm_madd_epi16(E40, C7);
                S71 = _mm_madd_epi16(E41, C7);

                T40 = _mm_loadu_si128((__m128i*)&imgPad[j]);
                E40 = _mm_unpacklo_epi16(T40, zero);
                E41 = _mm_unpackhi_epi16(T40, zero);
                S80 = _mm_madd_epi16(E40, C8);
                S81 = _mm_madd_epi16(E41, C8);

                SS1 = _mm_add_epi32(S00, S10);
                SS1 = _mm_add_epi32(SS1, S20);
                SS1 = _mm_add_epi32(SS1, S30);
                SS1 = _mm_add_epi32(SS1, S40);
                SS1 = _mm_add_epi32(SS1, S50);
                SS1 = _mm_add_epi32(SS1, S60);
                SS1 = _mm_add_epi32(SS1, S70);
                SS1 = _mm_add_epi32(SS1, S80);

                SS2 = _mm_add_epi32(S01, S11);
                SS2 = _mm_add_epi32(SS2, S21);
                SS2 = _mm_add_epi32(SS2, S31);
                SS2 = _mm_add_epi32(SS2, S41);
                SS2 = _mm_add_epi32(SS2, S51);
                SS2 = _mm_add_epi32(SS2, S61);
                SS2 = _mm_add_epi32(SS2, S71);
                SS2 = _mm_add_epi32(SS2, S81);

                SS1 = _mm_add_epi32(SS1, mAddOffset);
                SS1 = _mm_srai_epi32(SS1, 6);

                SS2 = _mm_add_epi32(SS2, mAddOffset);
                SS2 = _mm_srai_epi32(SS2, 6);

                S = _mm_packus_epi32(SS1, SS2);
                S = _mm_min_epu16(S, max_val);
                if (j != xPosEnd8){
                    _mm_storeu_si128((__m128i*)(imgRes + j), S);
                }
                else{
                    _mm_maskmoveu_si128(S, mask, (char *)(imgRes + j));
                    break;
                }
            }

            imgPad += stride;
            imgRes += stride;
        }
    }
    else{
        for (i = startPos; i < endPos; i++) {
            int yUp = Clip3(startPos, endPos - 1, i - 1);
            int yBottom = Clip3(startPos, endPos - 1, i + 1);
            imgPad1 = imgPad + (yBottom - i)*stride;
            imgPad2 = imgPad + (yUp - i)*stride;

            yUp = Clip3(startPos, endPos - 1, i - 2);
            yBottom = Clip3(startPos, endPos - 1, i + 2);
            imgPad3 = imgPad + (yBottom - i)*stride;
            imgPad4 = imgPad + (yUp - i)*stride;

            yUp = Clip3(startPos, endPos - 1, i - 3);
            yBottom = Clip3(startPos, endPos - 1, i + 3);
            imgPad5 = imgPad + (yBottom - i)*stride;
            imgPad6 = imgPad + (yUp - i)*stride;

            for (j = xPos; j < xPosEnd; j += 8) {
                T00 = _mm_loadu_si128((__m128i*)&imgPad6[j]);
                T01 = _mm_loadu_si128((__m128i*)&imgPad5[j]);
                E00 = _mm_unpacklo_epi16(T00, T01);
                E01 = _mm_unpackhi_epi16(T00, T01);
                S00 = _mm_madd_epi16(E00, C0);
                S01 = _mm_madd_epi16(E01, C0);

                T10 = _mm_loadu_si128((__m128i*)&imgPad4[j]);
                T11 = _mm_loadu_si128((__m128i*)&imgPad3[j]);
                E10 = _mm_unpacklo_epi16(T10, T11);
                E11 = _mm_unpackhi_epi16(T10, T11);
                S10 = _mm_madd_epi16(E10, C1);
                S11 = _mm_madd_epi16(E11, C1);

                T20 = _mm_loadu_si128((__m128i*)&imgPad2[j - 1]);
                T21 = _mm_loadu_si128((__m128i*)&imgPad1[j + 1]);
                E20 = _mm_unpacklo_epi16(T20, T21);
                E21 = _mm_unpackhi_epi16(T20, T21);
                S20 = _mm_madd_epi16(E20, C2);
                S21 = _mm_madd_epi16(E21, C2);

                T30 = _mm_loadu_si128((__m128i*)&imgPad2[j]);
                T31 = _mm_loadu_si128((__m128i*)&imgPad1[j]);
                E30 = _mm_unpacklo_epi16(T30, T31);
                E31 = _mm_unpackhi_epi16(T30, T31);
                S30 = _mm_madd_epi16(E30, C3);
                S31 = _mm_madd_epi16(E31, C3);

                T40 = _mm_loadu_si128((__m128i*)&imgPad2[j + 1]);
                T41 = _mm_loadu_si128((__m128i*)&imgPad1[j - 1]);
                E40 = _mm_unpacklo_epi16(T40, T41);
                E41 = _mm_unpackhi_epi16(T40, T41);
                S40 = _mm_madd_epi16(E40, C4);
                S41 = _mm_madd_epi16(E41, C4);

                T40 = _mm_loadu_si128((__m128i*)&imgPad[j - 3]);
                T41 = _mm_loadu_si128((__m128i*)&imgPad[j + 3]);
                E40 = _mm_unpacklo_epi16(T40, T41);
                E41 = _mm_unpackhi_epi16(T40, T41);
                S50 = _mm_madd_epi16(E40, C5);
                S51 = _mm_madd_epi16(E41, C5);

                T40 = _mm_loadu_si128((__m128i*)&imgPad[j - 2]);
                T41 = _mm_loadu_si128((__m128i*)&imgPad[j + 2]);
                E40 = _mm_unpacklo_epi16(T40, T41);
                E41 = _mm_unpackhi_epi16(T40, T41);
                S60 = _mm_madd_epi16(E40, C6);
                S61 = _mm_madd_epi16(E41, C6);

                T40 = _mm_loadu_si128((__m128i*)&imgPad[j - 1]);
                T41 = _mm_loadu_si128((__m128i*)&imgPad[j + 1]);
                E40 = _mm_unpacklo_epi16(T40, T41);
                E41 = _mm_unpackhi_epi16(T40, T41);
                S70 = _mm_madd_epi16(E40, C7);
                S71 = _mm_madd_epi16(E41, C7);

                T40 = _mm_loadu_si128((__m128i*)&imgPad[j]);
                E40 = _mm_unpacklo_epi16(T40, zero);
                E41 = _mm_unpackhi_epi16(T40, zero);
                S80 = _mm_madd_epi16(E40, C8);
                S81 = _mm_madd_epi16(E41, C8);

                SS1 = _mm_add_epi32(S00, S10);
                SS1 = _mm_add_epi32(SS1, S20);
                SS1 = _mm_add_epi32(SS1, S30);
                SS1 = _mm_add_epi32(SS1, S40);
                SS1 = _mm_add_epi32(SS1, S50);
                SS1 = _mm_add_epi32(SS1, S60);
                SS1 = _mm_add_epi32(SS1, S70);
                SS1 = _mm_add_epi32(SS1, S80);

                SS2 = _mm_add_epi32(S01, S11);
                SS2 = _mm_add_epi32(SS2, S21);
                SS2 = _mm_add_epi32(SS2, S31);
                SS2 = _mm_add_epi32(SS2, S41);
                SS2 = _mm_add_epi32(SS2, S51);
                SS2 = _mm_add_epi32(SS2, S61);
                SS2 = _mm_add_epi32(SS2, S71);
                SS2 = _mm_add_epi32(SS2, S81);

                SS1 = _mm_add_epi32(SS1, mAddOffset);
                SS1 = _mm_srai_epi32(SS1, 6);

                SS2 = _mm_add_epi32(SS2, mAddOffset);
                SS2 = _mm_srai_epi32(SS2, 6);

                S = _mm_packus_epi32(SS1, SS2);
                S = _mm_min_epu16(S, max_val);

                _mm_storeu_si128((__m128i*)(imgRes + j), S);

            }

            imgPad += stride;
            imgRes += stride;
        }
    }
}


