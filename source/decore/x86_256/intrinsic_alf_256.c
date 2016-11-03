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

#include "intrinsic_256.h"

ALIGNED_8(i32s_t alf_mask256[7][8]) = {
        { -1, 0, 0, 0, 0, 0, 0, 0 },
        { -1, -1, 0, 0, 0, 0, 0, 0 },
        { -1, -1, -1, 0, 0, 0, 0, 0 },
        { -1, -1, -1, -1, 0, 0, 0, 0 },
        { -1, -1, -1, -1, -1, 0, 0, 0 },
        { -1, -1, -1, -1, -1, -1, 0, 0 },
        { -1, -1, -1, -1, -1, -1, -1, 0 }
};

void alf_flt_one_block_sse256(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcu_height, int xPos, int lcu_width, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail)
{
	pel_t *imgPad1, *imgPad2, *imgPad3, *imgPad4, *imgPad5, *imgPad6;

	__m256i T00, T01, T10, T11, T20, T21, T30, T31, T40, T41, T50, T51, T60, T61, T70, T71, T80, T81;
	__m256i T8;
	__m256i T000, T001, T010, T011, T100, T101, T110, T111, T200, T201, T210, T211, T310, T311, T300, T301, T400, T401, T410, T411, T500, T501, T510, T511, T600, T601, T610, T611, T700, T701, T710, T711;
	__m256i E00, E01, E10, E11;
	__m256i C0, C1, C2, C22, C3, C4, C5, C6, C7, C8, C33, C44;
	__m256i S0, S00, S01, S1, S10, S11, S2, S20, S21, S3, S30, S31, S4, S40, S41, S5, S50, S51, S6, S60, S61, S70, S71, S80, S81, SS1, SS2, S;
	__m256i mAddOffset, index1, index2;
	__m256i mZero = _mm256_set1_epi16(0);
	__m256i mMax = _mm256_set1_epi16((short)((1 << sample_bit_depth) - 1));
	__m256i mask;

	int i, j;
	int startPos = isAboveAvail ? (yPos - 4) : yPos;
	int endPos = isBelowAvail ? (yPos + lcu_height - 4) : (yPos + lcu_height);

	int xPosEnd = xPos + lcu_width;
	int xPosEnd32 = xPosEnd - (lcu_width % 32);
	int maskType = ((lcu_width % 32) >> 2) - 1; //lcu_width是4的倍数

	mask = _mm256_loadu_si256((__m256i*)&alf_mask256[maskType]);

	imgPad += (startPos*stride);
	imgRes += (startPos*stride);

	C0 = _mm256_set1_epi16(coef[0]);
	C1 = _mm256_set1_epi16(coef[1]);
	C2 = _mm256_set1_epi16(coef[2]);
	C3 = _mm256_set1_epi16(coef[3]);
	C4 = _mm256_set1_epi16(coef[4]);
	C5 = _mm256_set1_epi16(coef[5]);
	C6 = _mm256_set1_epi16(coef[6]);
	C7 = _mm256_set1_epi16(coef[7]);
	C8 = _mm256_set1_epi16(coef[8]);

	C33 = _mm256_setr_epi8(coef[5], coef[6], coef[7], coef[8], coef[7], coef[6], coef[5], 0, coef[5], coef[6], coef[7], coef[8], coef[7], coef[6], coef[5], 0, coef[5], coef[6], coef[7], coef[8], coef[7], coef[6], coef[5], 0, coef[5], coef[6], coef[7], coef[8], coef[7], coef[6], coef[5], 0);
	C22 = _mm256_setr_epi8(coef[2], coef[3], coef[4], 0, coef[2], coef[3], coef[4], 0, coef[2], coef[3], coef[4], 0, coef[2], coef[3], coef[4], 0, coef[2], coef[3], coef[4], 0, coef[2], coef[3], coef[4], 0, coef[2], coef[3], coef[4], 0, coef[2], coef[3], coef[4], 0);
	C44 = _mm256_setr_epi8(coef[4], coef[3], coef[2], 0, coef[4], coef[3], coef[2], 0, coef[4], coef[3], coef[2], 0, coef[4], coef[3], coef[2], 0, coef[4], coef[3], coef[2], 0, coef[4], coef[3], coef[2], 0, coef[4], coef[3], coef[2], 0, coef[4], coef[3], coef[2], 0);
	index1 = _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7);
	index2 = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
	mAddOffset = _mm256_set1_epi16(32);

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

		for (j = xPos; j < xPosEnd; j += 32) {
			T00 = _mm256_loadu_si256((__m256i*)&imgPad6[j]);
			T000 = _mm256_unpacklo_epi8(T00, mZero);
			T001 = _mm256_unpackhi_epi8(T00, mZero);
			T01 = _mm256_loadu_si256((__m256i*)&imgPad5[j]);
			T010 = _mm256_unpacklo_epi8(T01, mZero);
			T011 = _mm256_unpackhi_epi8(T01, mZero);
			E00 = _mm256_add_epi16(T000, T010);
			E01 = _mm256_add_epi16(T001, T011);
			S00 = _mm256_mullo_epi16(C0, E00);//前16个像素所有C0*P0的结果
			S01 = _mm256_mullo_epi16(C0, E01);//后16个像素所有C0*P0的结果

			T10 = _mm256_loadu_si256((__m256i*)&imgPad4[j]);
			T100 = _mm256_unpacklo_epi8(T10, mZero);
			T101 = _mm256_unpackhi_epi8(T10, mZero);
			T11 = _mm256_loadu_si256((__m256i*)&imgPad3[j]);
			T110 = _mm256_unpacklo_epi8(T11, mZero);
			T111 = _mm256_unpackhi_epi8(T11, mZero);
			E10 = _mm256_add_epi16(T100, T110);
			E11 = _mm256_add_epi16(T101, T111);
			S10 = _mm256_mullo_epi16(C1, E10);//前16个像素所有C1*P1的结果
			S11 = _mm256_mullo_epi16(C1, E11);//后16个像素所有C1*P1的结果


			T20 = _mm256_loadu_si256((__m256i*)&imgPad2[j - 1]);
			T200 = _mm256_unpacklo_epi8(T20, mZero);
			T201 = _mm256_unpackhi_epi8(T20, mZero);
			T30 = _mm256_loadu_si256((__m256i*)&imgPad2[j]);
			T300 = _mm256_unpacklo_epi8(T30, mZero);
			T301 = _mm256_unpackhi_epi8(T30, mZero);
			T40 = _mm256_loadu_si256((__m256i*)&imgPad2[j + 1]);
			T400 = _mm256_unpacklo_epi8(T40, mZero);
			T401 = _mm256_unpackhi_epi8(T40, mZero);

			T41 = _mm256_loadu_si256((__m256i*)&imgPad1[j - 1]);
			T410 = _mm256_unpacklo_epi8(T41, mZero);
			T411 = _mm256_unpackhi_epi8(T41, mZero);
			T31 = _mm256_loadu_si256((__m256i*)&imgPad1[j]);
			T310 = _mm256_unpacklo_epi8(T31, mZero);
			T311 = _mm256_unpackhi_epi8(T31, mZero);
			T21 = _mm256_loadu_si256((__m256i*)&imgPad1[j + 1]);
			T210 = _mm256_unpacklo_epi8(T21, mZero);
			T211 = _mm256_unpackhi_epi8(T21, mZero);

			T20 = _mm256_add_epi16(T200, T210); // 前16个数
			T21 = _mm256_add_epi16(T201, T211); //后16个数
			T30 = _mm256_add_epi16(T300, T310);
			T31 = _mm256_add_epi16(T301, T311);
			T40 = _mm256_add_epi16(T400, T410);
			T41 = _mm256_add_epi16(T401, T411);


			S20 = _mm256_mullo_epi16(T20, C2);
			S21 = _mm256_mullo_epi16(T21, C2);
			S30 = _mm256_mullo_epi16(T30, C3);
			S31 = _mm256_mullo_epi16(T31, C3);
			S40 = _mm256_mullo_epi16(T40, C4);
			S41 = _mm256_mullo_epi16(T41, C4);

			T50 = _mm256_loadu_si256((__m256i*)&imgPad[j - 3]);
			T500 = _mm256_unpacklo_epi8(T50, mZero);
			T501 = _mm256_unpackhi_epi8(T50, mZero);
			T60 = _mm256_loadu_si256((__m256i*)&imgPad[j - 2]);
			T600 = _mm256_unpacklo_epi8(T60, mZero);
			T601 = _mm256_unpackhi_epi8(T60, mZero);
			T70 = _mm256_loadu_si256((__m256i*)&imgPad[j - 1]);
			T700 = _mm256_unpacklo_epi8(T70, mZero);
			T701 = _mm256_unpackhi_epi8(T70, mZero);
			T8 = _mm256_loadu_si256((__m256i*)&imgPad[j]);
			T80 = _mm256_unpacklo_epi8(T8, mZero);
			T81 = _mm256_unpackhi_epi8(T8, mZero);
			T71 = _mm256_loadu_si256((__m256i*)&imgPad[j + 1]);
			T710 = _mm256_unpacklo_epi8(T71, mZero);
			T711 = _mm256_unpackhi_epi8(T71, mZero);
			T61 = _mm256_loadu_si256((__m256i*)&imgPad[j + 2]);
			T610 = _mm256_unpacklo_epi8(T61, mZero);
			T611 = _mm256_unpackhi_epi8(T61, mZero);
			T51 = _mm256_loadu_si256((__m256i*)&imgPad[j + 3]);
			T510 = _mm256_unpacklo_epi8(T51, mZero);
			T511 = _mm256_unpackhi_epi8(T51, mZero);

			T50 = _mm256_add_epi16(T500, T510);
			T51 = _mm256_add_epi16(T501, T511);
			T60 = _mm256_add_epi16(T600, T610);
			T61 = _mm256_add_epi16(T601, T611);
			T70 = _mm256_add_epi16(T700, T710);
			T71 = _mm256_add_epi16(T701, T711);

			S50 = _mm256_mullo_epi16(T50, C5);
			S51 = _mm256_mullo_epi16(T51, C5);
			S60 = _mm256_mullo_epi16(T60, C6);
			S61 = _mm256_mullo_epi16(T61, C6);
			S70 = _mm256_mullo_epi16(T70, C7);
			S71 = _mm256_mullo_epi16(T71, C7);
			S80 = _mm256_mullo_epi16(T80, C8);
			S81 = _mm256_mullo_epi16(T81, C8);


			S0 = _mm256_adds_epi16(S00, S10);
			S1 = _mm256_adds_epi16(S0, S20);
			S2 = _mm256_adds_epi16(S1, S30);
			S3 = _mm256_adds_epi16(S2, S40);
			S4 = _mm256_adds_epi16(S3, S50);
			S5 = _mm256_adds_epi16(S4, S60);
			S6 = _mm256_adds_epi16(S5, S70);
			SS1 = _mm256_adds_epi16(S6, S80);//前16个

			S0 = _mm256_adds_epi16(S01, S11);
			S1 = _mm256_adds_epi16(S0, S21);
			S2 = _mm256_adds_epi16(S1, S31);
			S3 = _mm256_adds_epi16(S2, S41);
			S4 = _mm256_adds_epi16(S3, S51);
			S5 = _mm256_adds_epi16(S4, S61);
			S6 = _mm256_adds_epi16(S5, S71);
			SS2 = _mm256_adds_epi16(S6, S81);


			SS1 = _mm256_adds_epi16(SS1, mAddOffset);
			SS1 = _mm256_srai_epi16(SS1, 6);
			SS1 = _mm256_min_epi16(SS1, mMax);
			SS1 = _mm256_max_epi16(SS1, mZero);

			SS2 = _mm256_adds_epi16(SS2, mAddOffset);
			SS2 = _mm256_srai_epi16(SS2, 6);
			SS2 = _mm256_min_epi16(SS2, mMax);
			SS2 = _mm256_max_epi16(SS2, mZero);

			S = _mm256_packus_epi16(SS1, SS2);
			if (j != xPosEnd32){
				_mm256_storeu_si256((__m256i*)(imgRes + j), S);
			} else{
				_mm256_maskstore_epi32((int*)(imgRes + j), mask, S);
				break;
			}
		}

		imgPad += stride;
		imgRes += stride;
	}
}

void alf_flt_one_block_sse256_10bit(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcuHeight, int xPos, int lcuWidth, int *coef, int bit_depth, int isAboveAvail, int isBelowAvail)
{
    pel_t *imgPad1, *imgPad2, *imgPad3, *imgPad4, *imgPad5, *imgPad6;

    __m256i T00, T01, T10, T11, T20, T21, T30, T31, T40, T41;
    __m256i E00, E01, E10, E11, E20, E21, E30, E31, E40, E41;
    __m256i C0, C1, C2, C3, C4, C5, C6, C7, C8;
    __m256i S00, S01, S10, S11, S20, S21, S30, S31, S40, S41, S50, S51, S60, S61, SS1, SS2, S, S70, S71, S80, S81;
    __m256i mAddOffset;
    __m256i mask;
    __m256i zero = _mm256_setzero_si256();
    int max_pixel = (1 << bit_depth) - 1;
    __m256i max_val = _mm256_set1_epi16(max_pixel);

    int i, j;
    int startPos = isAboveAvail ? (yPos - 4) : yPos;
    int endPos = isBelowAvail ? (yPos + lcuHeight - 4) : (yPos + lcuHeight);
    int xPosEnd = xPos + lcuWidth;

    imgPad += (startPos*stride);
    imgRes += (startPos*stride);

    C0 = _mm256_set1_epi16((pel_t)coef[0]);
    C1 = _mm256_set1_epi16((pel_t)coef[1]);
    C2 = _mm256_set1_epi16((pel_t)coef[2]);
    C3 = _mm256_set1_epi16((pel_t)coef[3]);
    C4 = _mm256_set1_epi16((pel_t)coef[4]);
    C5 = _mm256_set1_epi16((pel_t)coef[5]);
    C6 = _mm256_set1_epi16((pel_t)coef[6]);
    C7 = _mm256_set1_epi16((pel_t)coef[7]);
    C8 = _mm256_set1_epi16((pel_t)coef[8]);

    mAddOffset = _mm256_set1_epi32(32);

    if (lcuWidth & 15){
        int xPosEnd15 = xPosEnd - (lcuWidth & 15);
        mask = _mm256_loadu_si256((__m256i*)(intrinsic_mask_10bit[(lcuWidth & 15) - 1]));
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

            for (j = xPos; j < xPosEnd; j += 16) {
                T00 = _mm256_loadu_si256((__m256i*)&imgPad6[j]);
                T01 = _mm256_loadu_si256((__m256i*)&imgPad5[j]);
                E00 = _mm256_unpacklo_epi16(T00, T01);
                E01 = _mm256_unpackhi_epi16(T00, T01);
                S00 = _mm256_madd_epi16(E00, C0);//前8个像素所有C0*P0的结果
                S01 = _mm256_madd_epi16(E01, C0);//后8个像素所有C0*P0的结果

                T10 = _mm256_loadu_si256((__m256i*)&imgPad4[j]);
                T11 = _mm256_loadu_si256((__m256i*)&imgPad3[j]);
                E10 = _mm256_unpacklo_epi16(T10, T11);
                E11 = _mm256_unpackhi_epi16(T10, T11);
                S10 = _mm256_madd_epi16(E10, C1);//前8个像素所有C1*P1的结果
                S11 = _mm256_madd_epi16(E11, C1);//后8个像素所有C1*P1的结果

                T20 = _mm256_loadu_si256((__m256i*)&imgPad2[j - 1]);
                T21 = _mm256_loadu_si256((__m256i*)&imgPad1[j + 1]);
                E20 = _mm256_unpacklo_epi16(T20, T21);
                E21 = _mm256_unpackhi_epi16(T20, T21);
                S20 = _mm256_madd_epi16(E20, C2);
                S21 = _mm256_madd_epi16(E21, C2);

                T30 = _mm256_loadu_si256((__m256i*)&imgPad2[j]);
                T31 = _mm256_loadu_si256((__m256i*)&imgPad1[j]);
                E30 = _mm256_unpacklo_epi16(T30, T31);
                E31 = _mm256_unpackhi_epi16(T30, T31);
                S30 = _mm256_madd_epi16(E30, C3);
                S31 = _mm256_madd_epi16(E31, C3);

                T40 = _mm256_loadu_si256((__m256i*)&imgPad2[j + 1]);
                T41 = _mm256_loadu_si256((__m256i*)&imgPad1[j - 1]);
                E40 = _mm256_unpacklo_epi16(T40, T41);
                E41 = _mm256_unpackhi_epi16(T40, T41);
                S40 = _mm256_madd_epi16(E40, C4);
                S41 = _mm256_madd_epi16(E41, C4);

                T40 = _mm256_loadu_si256((__m256i*)&imgPad[j - 3]);
                T41 = _mm256_loadu_si256((__m256i*)&imgPad[j + 3]);
                E40 = _mm256_unpacklo_epi16(T40, T41);
                E41 = _mm256_unpackhi_epi16(T40, T41);
                S50 = _mm256_madd_epi16(E40, C5);
                S51 = _mm256_madd_epi16(E41, C5);

                T40 = _mm256_loadu_si256((__m256i*)&imgPad[j - 2]);
                T41 = _mm256_loadu_si256((__m256i*)&imgPad[j + 2]);
                E40 = _mm256_unpacklo_epi16(T40, T41);
                E41 = _mm256_unpackhi_epi16(T40, T41);
                S60 = _mm256_madd_epi16(E40, C6);
                S61 = _mm256_madd_epi16(E41, C6);

                T40 = _mm256_loadu_si256((__m256i*)&imgPad[j - 1]);
                T41 = _mm256_loadu_si256((__m256i*)&imgPad[j + 1]);
                E40 = _mm256_unpacklo_epi16(T40, T41);
                E41 = _mm256_unpackhi_epi16(T40, T41);
                S70 = _mm256_madd_epi16(E40, C7);
                S71 = _mm256_madd_epi16(E41, C7);

                T40 = _mm256_loadu_si256((__m256i*)&imgPad[j]);
                E40 = _mm256_unpacklo_epi16(T40, zero);
                E41 = _mm256_unpackhi_epi16(T40, zero);
                S80 = _mm256_madd_epi16(E40, C8);
                S81 = _mm256_madd_epi16(E41, C8);

                SS1 = _mm256_add_epi32(S00, S10);
                SS1 = _mm256_add_epi32(SS1, S20);
                SS1 = _mm256_add_epi32(SS1, S30);
                SS1 = _mm256_add_epi32(SS1, S40);
                SS1 = _mm256_add_epi32(SS1, S50);
                SS1 = _mm256_add_epi32(SS1, S60);
                SS1 = _mm256_add_epi32(SS1, S70);
                SS1 = _mm256_add_epi32(SS1, S80);

                SS2 = _mm256_add_epi32(S01, S11);
                SS2 = _mm256_add_epi32(SS2, S21);
                SS2 = _mm256_add_epi32(SS2, S31);
                SS2 = _mm256_add_epi32(SS2, S41);
                SS2 = _mm256_add_epi32(SS2, S51);
                SS2 = _mm256_add_epi32(SS2, S61);
                SS2 = _mm256_add_epi32(SS2, S71);
                SS2 = _mm256_add_epi32(SS2, S81);

                SS1 = _mm256_add_epi32(SS1, mAddOffset);
                SS1 = _mm256_srai_epi32(SS1, 6);

                SS2 = _mm256_add_epi32(SS2, mAddOffset);
                SS2 = _mm256_srai_epi32(SS2, 6);

                S = _mm256_packus_epi32(SS1, SS2);
                S = _mm256_min_epu16(S, max_val);
                if (j != xPosEnd15){
                    _mm256_storeu_si256((__m256i*)(imgRes + j), S);
                }
                else{
                    _mm256_maskstore_epi32((int *)(imgRes + j), mask, S);
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

            for (j = xPos; j < xPosEnd; j += 16) {
                T00 = _mm256_loadu_si256((__m256i*)&imgPad6[j]);
                T01 = _mm256_loadu_si256((__m256i*)&imgPad5[j]);
                E00 = _mm256_unpacklo_epi16(T00, T01);
                E01 = _mm256_unpackhi_epi16(T00, T01);
                S00 = _mm256_madd_epi16(E00, C0);//前8个像素所有C0*P0的结果
                S01 = _mm256_madd_epi16(E01, C0);//后8个像素所有C0*P0的结果

                T10 = _mm256_loadu_si256((__m256i*)&imgPad4[j]);
                T11 = _mm256_loadu_si256((__m256i*)&imgPad3[j]);
                E10 = _mm256_unpacklo_epi16(T10, T11);
                E11 = _mm256_unpackhi_epi16(T10, T11);
                S10 = _mm256_madd_epi16(E10, C1);//前8个像素所有C1*P1的结果
                S11 = _mm256_madd_epi16(E11, C1);//后8个像素所有C1*P1的结果

                T20 = _mm256_loadu_si256((__m256i*)&imgPad2[j - 1]);
                T21 = _mm256_loadu_si256((__m256i*)&imgPad1[j + 1]);
                E20 = _mm256_unpacklo_epi16(T20, T21);
                E21 = _mm256_unpackhi_epi16(T20, T21);
                S20 = _mm256_madd_epi16(E20, C2);
                S21 = _mm256_madd_epi16(E21, C2);

                T30 = _mm256_loadu_si256((__m256i*)&imgPad2[j]);
                T31 = _mm256_loadu_si256((__m256i*)&imgPad1[j]);
                E30 = _mm256_unpacklo_epi16(T30, T31);
                E31 = _mm256_unpackhi_epi16(T30, T31);
                S30 = _mm256_madd_epi16(E30, C3);
                S31 = _mm256_madd_epi16(E31, C3);

                T40 = _mm256_loadu_si256((__m256i*)&imgPad2[j + 1]);
                T41 = _mm256_loadu_si256((__m256i*)&imgPad1[j - 1]);
                E40 = _mm256_unpacklo_epi16(T40, T41);
                E41 = _mm256_unpackhi_epi16(T40, T41);
                S40 = _mm256_madd_epi16(E40, C4);
                S41 = _mm256_madd_epi16(E41, C4);

                T40 = _mm256_loadu_si256((__m256i*)&imgPad[j - 3]);
                T41 = _mm256_loadu_si256((__m256i*)&imgPad[j + 3]);
                E40 = _mm256_unpacklo_epi16(T40, T41);
                E41 = _mm256_unpackhi_epi16(T40, T41);
                S50 = _mm256_madd_epi16(E40, C5);
                S51 = _mm256_madd_epi16(E41, C5);

                T40 = _mm256_loadu_si256((__m256i*)&imgPad[j - 2]);
                T41 = _mm256_loadu_si256((__m256i*)&imgPad[j + 2]);
                E40 = _mm256_unpacklo_epi16(T40, T41);
                E41 = _mm256_unpackhi_epi16(T40, T41);
                S60 = _mm256_madd_epi16(E40, C6);
                S61 = _mm256_madd_epi16(E41, C6);

                T40 = _mm256_loadu_si256((__m256i*)&imgPad[j - 1]);
                T41 = _mm256_loadu_si256((__m256i*)&imgPad[j + 1]);
                E40 = _mm256_unpacklo_epi16(T40, T41);
                E41 = _mm256_unpackhi_epi16(T40, T41);
                S70 = _mm256_madd_epi16(E40, C7);
                S71 = _mm256_madd_epi16(E41, C7);

                T40 = _mm256_loadu_si256((__m256i*)&imgPad[j]);
                E40 = _mm256_unpacklo_epi16(T40, zero);
                E41 = _mm256_unpackhi_epi16(T40, zero);
                S80 = _mm256_madd_epi16(E40, C8);
                S81 = _mm256_madd_epi16(E41, C8);

                SS1 = _mm256_add_epi32(S00, S10);
                SS1 = _mm256_add_epi32(SS1, S20);
                SS1 = _mm256_add_epi32(SS1, S30);
                SS1 = _mm256_add_epi32(SS1, S40);
                SS1 = _mm256_add_epi32(SS1, S50);
                SS1 = _mm256_add_epi32(SS1, S60);
                SS1 = _mm256_add_epi32(SS1, S70);
                SS1 = _mm256_add_epi32(SS1, S80);

                SS2 = _mm256_add_epi32(S01, S11);
                SS2 = _mm256_add_epi32(SS2, S21);
                SS2 = _mm256_add_epi32(SS2, S31);
                SS2 = _mm256_add_epi32(SS2, S41);
                SS2 = _mm256_add_epi32(SS2, S51);
                SS2 = _mm256_add_epi32(SS2, S61);
                SS2 = _mm256_add_epi32(SS2, S71);
                SS2 = _mm256_add_epi32(SS2, S81);

                SS1 = _mm256_add_epi32(SS1, mAddOffset);
                SS1 = _mm256_srai_epi32(SS1, 6);

                SS2 = _mm256_add_epi32(SS2, mAddOffset);
                SS2 = _mm256_srai_epi32(SS2, 6);

                S = _mm256_packus_epi32(SS1, SS2);
                S = _mm256_min_epu16(S, max_val);

                _mm256_storeu_si256((__m256i*)(imgRes + j), S);

            }

            imgPad += stride;
            imgRes += stride;
        }
    }
}