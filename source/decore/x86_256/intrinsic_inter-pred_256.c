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

#include "intrinsic_256.h"

void com_if_filter_cpy_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    int col;

    if (width & 15) {
        while (height--) {
            for (col = 0; col < width; col += 8) {
                CP128(dst + col, src + col);
            }
            src += i_src;
            dst += i_dst;
        }
    } else {
        while (height--) {
            for (col = 0; col < width; col += 16) {
                _mm256_storeu_si256((__m256i*)(dst + col), _mm256_loadu_si256((const __m256i*)(src + col)));
            }
            src += i_src;
            dst += i_dst;
        }
    }
}

void com_if_filter_cpy16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    while (height--) {
        _mm256_storeu_si256((__m256i*)dst, _mm256_loadu_si256((const __m256i*)src));
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_hor_4_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row, col;
	const int offset = 32;
	const int shift = 6;

	__m256i mCoef = _mm256_set1_epi32(*(i32s_t*)coeff);
	__m256i mSwitch1 = _mm256_setr_epi8(0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6, 0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6);
	__m256i mSwitch2 = _mm256_setr_epi8(4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10, 4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10);
	__m256i mAddOffset = _mm256_set1_epi16(offset);
	__m256i T0, T1, S, S0, R1, R2, sum;
	__m256i mask16 = _mm256_setr_epi32(-1, -1, -1, -1, 0, 0, 0, 0);
	src -= 1;

	for (row = 0; row < height; row++)
	{
		
		for (col = 0; col < width; col += 16)
		{
			S = _mm256_loadu_si256((__m256i*)(src + col));
			S0 = _mm256_permute4x64_epi64(S, 0x94);
			R1 = _mm256_shuffle_epi8(S0, mSwitch1);
			R2 = _mm256_shuffle_epi8(S0, mSwitch2);
			T0 = _mm256_maddubs_epi16(R1, mCoef);
			T1 = _mm256_maddubs_epi16(R2, mCoef);
			sum = _mm256_hadd_epi16(T0, T1);

			sum = _mm256_add_epi16(sum, mAddOffset);
			sum = _mm256_srai_epi16(sum, shift);
			sum = _mm256_packus_epi16(sum, sum);
			sum = _mm256_permute4x64_epi64(sum, 0xd8);

			_mm256_maskstore_epi32((int*)(dst + col), mask16, sum);
		}
		src += i_src;
		dst += i_dst;
	}
}

void com_if_filter_hor_4_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;

	__m256i mCoef = _mm256_set1_epi32(*(i32s_t*)coeff);
	__m256i mSwitch = _mm256_setr_epi8(0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6, 4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10);
	__m256i mSwitch1 = _mm256_setr_epi8(0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6, 0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6);
	__m256i mSwitch2 = _mm256_setr_epi8(4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10, 4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10);
	__m256i mask24 = _mm256_setr_epi16(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0);
	__m256i mAddOffset = _mm256_set1_epi16(offset);
	__m256i T0, T1, T2, S, S0, sum1, sum2,R1,R2,R3;
	__m256i index = _mm256_setr_epi32(0, 1, 2, 6, 4, 5, 3, 7);
	src -= 1;

	for (row = 0; row < height; row++)
	{
		S = _mm256_loadu_si256((__m256i*)(src));
		S0 = _mm256_permute4x64_epi64(S, 0x99);
		R1 = _mm256_shuffle_epi8(S, mSwitch1);
		R2 = _mm256_shuffle_epi8(S, mSwitch2);
		R3 = _mm256_shuffle_epi8(S0, mSwitch);
		T0 = _mm256_maddubs_epi16(R1, mCoef);
		T1 = _mm256_maddubs_epi16(R2, mCoef);
		T2 = _mm256_maddubs_epi16(R3, mCoef);
		sum1 = _mm256_hadd_epi16(T0, T1);
		sum2 = _mm256_hadd_epi16(T2, T2);

		sum1 = _mm256_add_epi16(sum1, mAddOffset);
		sum1 = _mm256_srai_epi16(sum1, shift);
		sum2 = _mm256_add_epi16(sum2, mAddOffset);
		sum2 = _mm256_srai_epi16(sum2, shift);
		sum1 = _mm256_packus_epi16(sum1, sum2);
		sum1 = _mm256_permutevar8x32_epi32(sum1, index);

		_mm256_maskstore_epi32((int*)(dst), mask24, sum1);

		src += i_src;
		dst += i_dst;
	}
}

void com_if_filter_hor_8_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row, col;
	const int offset = 32;
	const int shift = 6;
	__m256i mAddOffset = _mm256_set1_epi16(offset);
	__m256i mask16 = _mm256_setr_epi32(-1, -1, -1, -1, 0, 0, 0, 0);
	__m256i mSwitch1 = _mm256_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8);
	__m256i mSwitch2 = _mm256_setr_epi8(2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10);
	__m256i mSwitch3 = _mm256_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12);
	__m256i mSwitch4 = _mm256_setr_epi8(6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14);
	__m256i mCoef;
	__m256i T0, T1, S0, sum, S, T2, T3;
	__m256i r0, r1, r2, r3;
	src -= 3;

#if defined(_WIN64)
	mCoef = _mm256_set1_epi64x(*(long long*)coeff);
#else
	mCoef = _mm256_loadu_si256((__m256i*)coeff);
	mCoef = _mm256_permute4x64_epi64(mCoef, 0x0);
#endif

	for (row = 0; row < height; row++)
	{
		for (col = 0; col < width; col += 16)
		{
			S = _mm256_loadu_si256((__m256i*)(src + col));
			S0 = _mm256_permute4x64_epi64(S, 0x94);

			r0 = _mm256_shuffle_epi8(S0, mSwitch1);
			r1 = _mm256_shuffle_epi8(S0, mSwitch2);
			r2 = _mm256_shuffle_epi8(S0, mSwitch3);
			r3 = _mm256_shuffle_epi8(S0, mSwitch4);

			T0 = _mm256_maddubs_epi16(r0, mCoef);
			T1 = _mm256_maddubs_epi16(r1, mCoef);
			T2 = _mm256_maddubs_epi16(r2, mCoef);
			T3 = _mm256_maddubs_epi16(r3, mCoef);

			T0 = _mm256_hadd_epi16(T0, T1);
			T1 = _mm256_hadd_epi16(T2, T3);
			sum = _mm256_hadd_epi16(T0, T1);

			sum = _mm256_add_epi16(sum, mAddOffset);
			sum = _mm256_srai_epi16(sum, shift);

			sum = _mm256_packus_epi16(sum, sum);
			sum = _mm256_permute4x64_epi64(sum, 0xd8);

			_mm256_maskstore_epi32((int*)(dst + col), mask16, sum);
		}
		src += i_src;
		dst += i_dst;
	}
}

void com_if_filter_hor_8_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	__m256i mAddOffset = _mm256_set1_epi16(offset);
	__m256i index = _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7);
	__m256i mask24 = _mm256_setr_epi16(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0);
	__m256i T0, T1, S1, sum1, sum2, S, T2, T3,T4,T5;
	__m256i r0, r1, r2, r3,r4,r5;
	__m256i mSwitch1 = _mm256_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10);
	__m256i mSwitch2 = _mm256_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14);
	__m256i mSwitch3 = _mm256_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8);
	__m256i mSwitch4 = _mm256_setr_epi8(2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10);
	__m256i mSwitch5 = _mm256_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12);
	__m256i mSwitch6 = _mm256_setr_epi8(6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14);
	__m256i mCoef;
	src -= 3;

#if defined(_WIN64)
	mCoef = _mm256_set1_epi64x(*(long long*)coeff);
#else
	mCoef = _mm256_loadu_si256((__m256i*)coeff);
	mCoef = _mm256_permute4x64_epi64(mCoef, 0x0);
#endif

	for (row = 0; row < height; row++)
	{
		S = _mm256_loadu_si256((__m256i*)(src));
		S1 = _mm256_permute4x64_epi64(S, 0x99);

		r0 = _mm256_shuffle_epi8(S1, mSwitch1);
		r1 = _mm256_shuffle_epi8(S1, mSwitch2);
		r2 = _mm256_shuffle_epi8(S, mSwitch3);
		r3 = _mm256_shuffle_epi8(S, mSwitch4);
		r4 = _mm256_shuffle_epi8(S, mSwitch5);
		r5 = _mm256_shuffle_epi8(S, mSwitch6);

		T0 = _mm256_maddubs_epi16(r0, mCoef);
		T1 = _mm256_maddubs_epi16(r1, mCoef);
		T2 = _mm256_maddubs_epi16(r2, mCoef);
		T3 = _mm256_maddubs_epi16(r3, mCoef);
		T4 = _mm256_maddubs_epi16(r4, mCoef);
		T5 = _mm256_maddubs_epi16(r5, mCoef);

		T0 = _mm256_hadd_epi16(T0, T1);
		T1 = _mm256_hadd_epi16(T2, T3);
		T2 = _mm256_hadd_epi16(T4, T5);
		sum1 = _mm256_hadd_epi16(T1, T2);
		sum2 = _mm256_hadd_epi16(T0, T0);

		sum1 = _mm256_add_epi16(sum1, mAddOffset);
		sum1 = _mm256_srai_epi16(sum1, shift);
		sum2 = _mm256_add_epi16(sum2, mAddOffset);
		sum2 = _mm256_srai_epi16(sum2, shift);

		sum2 = _mm256_permutevar8x32_epi32(sum2, index);

		sum1 = _mm256_packus_epi16(sum1, sum2);

		_mm256_maskstore_epi32((int*)(dst), mask24, sum1);
		src += i_src;
		dst += i_dst;
	}
}

void com_if_filter_ver_4_w32_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[2]);
	__m256i mAddOffset = _mm256_set1_epi16(offset);
	const int i_src2 = i_src * 2;
	const int i_src3 = i_src * 3;

	src -= i_src;

	if (bsym)
	{
		__m256i coeff0 = _mm256_set1_epi8(coeff[0]);
		__m256i coeff1 = _mm256_set1_epi8(coeff[1]);
		__m256i S0, S1, S2, S3;
		__m256i T0, T1, T2, T3, mVal1, mVal2;
		for (row = 0; row < height; row++)
		{
			S0 = _mm256_loadu_si256((__m256i*)(src));
			S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));

			T0 = _mm256_unpacklo_epi8(S0, S3);
			T1 = _mm256_unpacklo_epi8(S1, S2);
			T2 = _mm256_unpackhi_epi8(S0, S3);
			T3 = _mm256_unpackhi_epi8(S1, S2);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff0);
			T3 = _mm256_maddubs_epi16(T3, coeff1);
			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T2, T3);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);
			_mm256_storeu_si256((__m256i*)(dst), mVal1);
			src += i_src;
			dst += i_dst;
		}
	}
	else
	{
		__m256i coeff0 = _mm256_set1_epi16(*(i16s_t*)coeff);
		__m256i coeff1 = _mm256_set1_epi16(*(i16s_t*)(coeff + 2));
		__m256i S0, S1, S2, S3;
		__m256i T0, T1, T2, T3, mVal1, mVal2;
		for (row = 0; row < height; row++)
		{
			S0 = _mm256_loadu_si256((__m256i*)(src));
			S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));

			T0 = _mm256_unpacklo_epi8(S0, S1);
			T1 = _mm256_unpacklo_epi8(S2, S3);
			T2 = _mm256_unpackhi_epi8(S0, S1);
			T3 = _mm256_unpackhi_epi8(S2, S3);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff0);
			T3 = _mm256_maddubs_epi16(T3, coeff1);
			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T2, T3);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);
			_mm256_storeu_si256((__m256i*)(dst), mVal1);

			src += i_src;
			dst += i_dst;
		}
	}
}

void com_if_filter_ver_4_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[2]);
	__m256i mAddOffset = _mm256_set1_epi16(offset);
	__m256i mask24 = _mm256_setr_epi16(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0);
	const int i_src2 = i_src * 2;
	const int i_src3 = i_src * 3;

	src -= i_src;

	if (bsym)
	{
		__m256i coeff0 = _mm256_set1_epi8(coeff[0]);
		__m256i coeff1 = _mm256_set1_epi8(coeff[1]);
		__m256i S0, S1, S2, S3;
		__m256i T0, T1, T2, T3, mVal1, mVal2;
		for (row = 0; row < height; row++)
		{
			S0 = _mm256_loadu_si256((__m256i*)(src));
			S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));

			T0 = _mm256_unpacklo_epi8(S0, S3);
			T1 = _mm256_unpacklo_epi8(S1, S2);
			T2 = _mm256_unpackhi_epi8(S0, S3);
			T3 = _mm256_unpackhi_epi8(S1, S2);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff0);
			T3 = _mm256_maddubs_epi16(T3, coeff1);
			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T2, T3);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);
			_mm256_maskstore_epi32((int*)(dst), mask24, mVal1);
			src += i_src;
			dst += i_dst;
		}
	}
	else
	{
		__m256i coeff0 = _mm256_set1_epi16(*(i16s_t*)coeff);
		__m256i coeff1 = _mm256_set1_epi16(*(i16s_t*)(coeff + 2));
		__m256i S0, S1, S2, S3;
		__m256i T0, T1, T2, T3, mVal1, mVal2;
		for (row = 0; row < height; row++)
		{
			S0 = _mm256_loadu_si256((__m256i*)(src));
			S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));

			T0 = _mm256_unpacklo_epi8(S0, S1);
			T1 = _mm256_unpacklo_epi8(S2, S3);
			T2 = _mm256_unpackhi_epi8(S0, S1);
			T3 = _mm256_unpackhi_epi8(S2, S3);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff0);
			T3 = _mm256_maddubs_epi16(T3, coeff1);
			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T2, T3);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);
			_mm256_storeu_si256((__m256i*)(dst), mVal1);

			src += i_src;
			dst += i_dst;
		}
	}
}

void com_if_filter_ver_4_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[2]);
	__m256i mAddOffset = _mm256_set1_epi16(offset);
	const int i_src2 = i_src * 2;
	const int i_src3 = i_src * 3;
	const int i_src4 = i_src * 4;

	src -= i_src;

	if (bsym)
	{
		__m256i coeff0 = _mm256_set1_epi8(coeff[0]);
		__m256i coeff1 = _mm256_set1_epi8(coeff[1]);
		__m256i T0, T1, T2, T3, mVal1, mVal2;
		__m256i R0, R1, R2, R3;
		for (row = 0; row < height; row = row + 2)
		{
			__m128i S0 = _mm_loadu_si128((__m128i*)(src));
			__m128i S1 = _mm_loadu_si128((__m128i*)(src + i_src));
			__m128i S2 = _mm_loadu_si128((__m128i*)(src + i_src2));
			__m128i S3 = _mm_loadu_si128((__m128i*)(src + i_src3));
			__m128i S4 = _mm_loadu_si128((__m128i*)(src + i_src4));

			R0 = _mm256_set_m128i(S0, S1);
			R1 = _mm256_set_m128i(S1, S2);
			R2 = _mm256_set_m128i(S2, S3);
			R3 = _mm256_set_m128i(S3, S4);
			
			T0 = _mm256_unpacklo_epi8(R0, R3);
			T1 = _mm256_unpackhi_epi8(R0, R3);
			T2 = _mm256_unpacklo_epi8(R1, R2);
			T3 = _mm256_unpackhi_epi8(R1, R2);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff0);
			T2 = _mm256_maddubs_epi16(T2, coeff1);
			T3 = _mm256_maddubs_epi16(T3, coeff1);

			mVal1 = _mm256_add_epi16(T0, T2);
			mVal2 = _mm256_add_epi16(T1, T3);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu2_m128i((__m128i*)dst, (__m128i*)(dst + i_dst), mVal1);

			src += 2*i_src;
			dst += 2*i_dst;
		}
	}
	else
	{
		__m256i coeff0 = _mm256_set1_epi16(*(i16s_t*)coeff);
		__m256i coeff1 = _mm256_set1_epi16(*(i16s_t*)(coeff + 2));
		__m256i T0, T1, T2, T3, mVal1, mVal2;
		__m256i R0, R1, R2, R3;
		for (row = 0; row < height; row = row + 2)
		{
			__m128i S0 = _mm_loadu_si128((__m128i*)(src));
			__m128i S1 = _mm_loadu_si128((__m128i*)(src + i_src));
			__m128i S2 = _mm_loadu_si128((__m128i*)(src + i_src2));
			__m128i S3 = _mm_loadu_si128((__m128i*)(src + i_src3));
			__m128i S4 = _mm_loadu_si128((__m128i*)(src + i_src4));

			R0 = _mm256_set_m128i(S0, S1);
			R1 = _mm256_set_m128i(S1, S2);
			R2 = _mm256_set_m128i(S2, S3);
			R3 = _mm256_set_m128i(S3, S4);

			T0 = _mm256_unpacklo_epi8(R0, R1);
			T1 = _mm256_unpackhi_epi8(R0, R1);
			T2 = _mm256_unpacklo_epi8(R2, R3);
			T3 = _mm256_unpackhi_epi8(R2, R3);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff0);
			T2 = _mm256_maddubs_epi16(T2, coeff1);
			T3 = _mm256_maddubs_epi16(T3, coeff1);

			mVal1 = _mm256_add_epi16(T0, T2);
			mVal2 = _mm256_add_epi16(T1, T3);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu2_m128i((__m128i*)dst, (__m128i*)(dst + i_dst), mVal1);

			src += 2 * i_src;
			dst += 2 * i_dst;

		}
	}
}

void com_if_filter_ver_8_w32_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[6]);
	const int i_src2 = i_src * 2;
	const int i_src3 = i_src * 3;
	const int i_src4 = i_src * 4;
	const int i_src5 = i_src * 5;
	const int i_src6 = i_src * 6;
	const int i_src7 = i_src * 7;

	src -= 3 * i_src;

	if (bsym)
	{
		__m256i mAddOffset = _mm256_set1_epi16(offset);
		__m256i coeff0 = _mm256_set1_epi8(coeff[0]);
		__m256i coeff1 = _mm256_set1_epi8(coeff[1]);
		__m256i coeff2 = _mm256_set1_epi8(coeff[2]);
		__m256i coeff3 = _mm256_set1_epi8(coeff[3]);
		__m256i T0, T1, T2, T3, T4, T5, T6, T7, mVal1, mVal2;

		for (row = 0; row < height; row++)
		{
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));
			__m256i S5 = _mm256_loadu_si256((__m256i*)(src + i_src5));
			__m256i S6 = _mm256_loadu_si256((__m256i*)(src + i_src6));
			__m256i S7 = _mm256_loadu_si256((__m256i*)(src + i_src7));

			T0 = _mm256_unpacklo_epi8(S0, S7);
			T1 = _mm256_unpacklo_epi8(S1, S6);
			T2 = _mm256_unpacklo_epi8(S2, S5);
			T3 = _mm256_unpacklo_epi8(S3, S4);
			T4 = _mm256_unpackhi_epi8(S0, S7);
			T5 = _mm256_unpackhi_epi8(S1, S6);
			T6 = _mm256_unpackhi_epi8(S2, S5);
			T7 = _mm256_unpackhi_epi8(S3, S4);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu_si256((__m256i*)(dst), mVal1);

			src += i_src;
			dst += i_dst;
		}
	}
	else
	{
			__m256i mAddOffset = _mm256_set1_epi16(offset);
			__m256i coeff0 = _mm256_set1_epi16(*(i32s_t*)coeff);
			__m256i coeff1 = _mm256_set1_epi16(*(i32s_t*)(coeff + 2));
			__m256i coeff2 = _mm256_set1_epi16(*(i32s_t*)(coeff + 4));
			__m256i coeff3 = _mm256_set1_epi16(*(i32s_t*)(coeff + 6));
			__m256i T0, T1, T2, T3, T4, T5, T6, T7, mVal1, mVal2;

			for (row = 0; row < height; row++)
			{
				__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
				__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
				__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
				__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
				__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));
				__m256i S5 = _mm256_loadu_si256((__m256i*)(src + i_src5));
				__m256i S6 = _mm256_loadu_si256((__m256i*)(src + i_src6));
				__m256i S7 = _mm256_loadu_si256((__m256i*)(src + i_src7));

				T0 = _mm256_unpacklo_epi8(S0, S1);
				T1 = _mm256_unpacklo_epi8(S2, S3);
				T2 = _mm256_unpacklo_epi8(S4, S5);
				T3 = _mm256_unpacklo_epi8(S6, S7);
				T4 = _mm256_unpackhi_epi8(S0, S1);
				T5 = _mm256_unpackhi_epi8(S2, S3);
				T6 = _mm256_unpackhi_epi8(S4, S5);
				T7 = _mm256_unpackhi_epi8(S6, S7);

				T0 = _mm256_maddubs_epi16(T0, coeff0);
				T1 = _mm256_maddubs_epi16(T1, coeff1);
				T2 = _mm256_maddubs_epi16(T2, coeff2);
				T3 = _mm256_maddubs_epi16(T3, coeff3);
				T4 = _mm256_maddubs_epi16(T4, coeff0);
				T5 = _mm256_maddubs_epi16(T5, coeff1);
				T6 = _mm256_maddubs_epi16(T6, coeff2);
				T7 = _mm256_maddubs_epi16(T7, coeff3);

				mVal1 = _mm256_add_epi16(T0, T1);
				mVal2 = _mm256_add_epi16(T4, T5);
				mVal1 = _mm256_add_epi16(mVal1, T2);
				mVal2 = _mm256_add_epi16(mVal2, T6);
				mVal1 = _mm256_add_epi16(mVal1, T3);
				mVal2 = _mm256_add_epi16(mVal2, T7);

				mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
				mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
				mVal1 = _mm256_srai_epi16(mVal1, shift);
				mVal2 = _mm256_srai_epi16(mVal2, shift);
				mVal1 = _mm256_packus_epi16(mVal1, mVal2);

				_mm256_storeu_si256((__m256i*)(dst), mVal1);

				src += i_src;
				dst += i_dst;
			}
	}
}

void com_if_filter_ver_8_w64_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[6]);
	const int i_src2 = i_src * 2;
	const int i_src3 = i_src * 3;
	const int i_src4 = i_src * 4;
	const int i_src5 = i_src * 5;
	const int i_src6 = i_src * 6;
	const int i_src7 = i_src * 7;

	src -= 3 * i_src;

	if (bsym)
	{
		__m256i mAddOffset = _mm256_set1_epi16(offset);
		__m256i coeff0 = _mm256_set1_epi8(coeff[0]);
		__m256i coeff1 = _mm256_set1_epi8(coeff[1]);
		__m256i coeff2 = _mm256_set1_epi8(coeff[2]);
		__m256i coeff3 = _mm256_set1_epi8(coeff[3]);
		__m256i T0, T1, T2, T3, T4, T5, T6, T7, mVal1, mVal2;

		for (row = 0; row < height; row++)
		{
			const pel_t *p = src + 32;
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));
			__m256i S5 = _mm256_loadu_si256((__m256i*)(src + i_src5));
			__m256i S6 = _mm256_loadu_si256((__m256i*)(src + i_src6));
			__m256i S7 = _mm256_loadu_si256((__m256i*)(src + i_src7));

			T0 = _mm256_unpacklo_epi8(S0, S7);
			T1 = _mm256_unpacklo_epi8(S1, S6);
			T2 = _mm256_unpacklo_epi8(S2, S5);
			T3 = _mm256_unpacklo_epi8(S3, S4);
			T4 = _mm256_unpackhi_epi8(S0, S7);
			T5 = _mm256_unpackhi_epi8(S1, S6);
			T6 = _mm256_unpackhi_epi8(S2, S5);
			T7 = _mm256_unpackhi_epi8(S3, S4);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu_si256((__m256i*)(dst), mVal1);

			S0 = _mm256_loadu_si256((__m256i*)(p));
			S1 = _mm256_loadu_si256((__m256i*)(p + i_src));
			S2 = _mm256_loadu_si256((__m256i*)(p + i_src2));
			S3 = _mm256_loadu_si256((__m256i*)(p + i_src3));
			S4 = _mm256_loadu_si256((__m256i*)(p + i_src4));
			S5 = _mm256_loadu_si256((__m256i*)(p + i_src5));
			S6 = _mm256_loadu_si256((__m256i*)(p + i_src6));
			S7 = _mm256_loadu_si256((__m256i*)(p + i_src7));
														   
			T0 = _mm256_unpacklo_epi8(S0, S7);			   
			T1 = _mm256_unpacklo_epi8(S1, S6);			   
			T2 = _mm256_unpacklo_epi8(S2, S5);			   
			T3 = _mm256_unpacklo_epi8(S3, S4);
			T4 = _mm256_unpackhi_epi8(S0, S7);
			T5 = _mm256_unpackhi_epi8(S1, S6);
			T6 = _mm256_unpackhi_epi8(S2, S5);
			T7 = _mm256_unpackhi_epi8(S3, S4);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu_si256((__m256i*)(dst + 32), mVal1);

			src += i_src;
			dst += i_dst;
		}
	}
	else
	{
		__m256i mAddOffset = _mm256_set1_epi16(offset);
		__m256i coeff0 = _mm256_set1_epi16(*(i32s_t*)coeff);
		__m256i coeff1 = _mm256_set1_epi16(*(i32s_t*)(coeff + 2));
		__m256i coeff2 = _mm256_set1_epi16(*(i32s_t*)(coeff + 4));
		__m256i coeff3 = _mm256_set1_epi16(*(i32s_t*)(coeff + 6));
		__m256i T0, T1, T2, T3, T4, T5, T6, T7, mVal1, mVal2;

		for (row = 0; row < height; row++)
		{
			const pel_t *p = src + 32;
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));
			__m256i S5 = _mm256_loadu_si256((__m256i*)(src + i_src5));
			__m256i S6 = _mm256_loadu_si256((__m256i*)(src + i_src6));
			__m256i S7 = _mm256_loadu_si256((__m256i*)(src + i_src7));

			T0 = _mm256_unpacklo_epi8(S0, S1);
			T1 = _mm256_unpacklo_epi8(S2, S3);
			T2 = _mm256_unpacklo_epi8(S4, S5);
			T3 = _mm256_unpacklo_epi8(S6, S7);
			T4 = _mm256_unpackhi_epi8(S0, S1);
			T5 = _mm256_unpackhi_epi8(S2, S3);
			T6 = _mm256_unpackhi_epi8(S4, S5);
			T7 = _mm256_unpackhi_epi8(S6, S7);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu_si256((__m256i*)(dst), mVal1);

			S0 = _mm256_loadu_si256((__m256i*)(p));
			S1 = _mm256_loadu_si256((__m256i*)(p + i_src));
			S2 = _mm256_loadu_si256((__m256i*)(p + i_src2));
			S3 = _mm256_loadu_si256((__m256i*)(p + i_src3));
			S4 = _mm256_loadu_si256((__m256i*)(p + i_src4));
			S5 = _mm256_loadu_si256((__m256i*)(p + i_src5));
			S6 = _mm256_loadu_si256((__m256i*)(p + i_src6));
			S7 = _mm256_loadu_si256((__m256i*)(p + i_src7));

			T0 = _mm256_unpacklo_epi8(S0, S1);
			T1 = _mm256_unpacklo_epi8(S2, S3);
			T2 = _mm256_unpacklo_epi8(S4, S5);
			T3 = _mm256_unpacklo_epi8(S6, S7);
			T4 = _mm256_unpackhi_epi8(S0, S1);
			T5 = _mm256_unpackhi_epi8(S2, S3);
			T6 = _mm256_unpackhi_epi8(S4, S5);
			T7 = _mm256_unpackhi_epi8(S6, S7);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu_si256((__m256i*)(dst + 32), mVal1);

			src += i_src;
			dst += i_dst;
		}
	}
}

void com_if_filter_ver_8_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[6]);
	const int i_src2 = i_src * 2;
	const int i_src3 = i_src * 3;
	const int i_src4 = i_src * 4;
	const int i_src5 = i_src * 5;
	const int i_src6 = i_src * 6;
	const int i_src7 = i_src * 7;
	const int i_src8 = i_src * 8;

	src -= 3 * i_src;

	if (bsym)
	{
		__m256i mAddOffset = _mm256_set1_epi16(offset);
		__m256i coeff0 = _mm256_set1_epi8(coeff[0]);
		__m256i coeff1 = _mm256_set1_epi8(coeff[1]);
		__m256i coeff2 = _mm256_set1_epi8(coeff[2]);
		__m256i coeff3 = _mm256_set1_epi8(coeff[3]);
		__m256i T0, T1, T2, T3, T4, T5, T6, T7, mVal1, mVal2;
		__m256i R0, R1, R2, R3, R4, R5, R6, R7;
		for (row = 0; row < height; row+=2)
		{
			__m128i S0 = _mm_loadu_si128((__m128i*)(src));
			__m128i S1 = _mm_loadu_si128((__m128i*)(src + i_src));
			__m128i S2 = _mm_loadu_si128((__m128i*)(src + i_src2));
			__m128i S3 = _mm_loadu_si128((__m128i*)(src + i_src3));
			__m128i S4 = _mm_loadu_si128((__m128i*)(src + i_src4));
			__m128i S5 = _mm_loadu_si128((__m128i*)(src + i_src5));
			__m128i S6 = _mm_loadu_si128((__m128i*)(src + i_src6));
			__m128i S7 = _mm_loadu_si128((__m128i*)(src + i_src7));
			__m128i S8 = _mm_loadu_si128((__m128i*)(src + i_src8));

			R0 = _mm256_set_m128i(S0, S1);
			R1 = _mm256_set_m128i(S1, S2);
			R2 = _mm256_set_m128i(S2, S3);
			R3 = _mm256_set_m128i(S3, S4);
			R4 = _mm256_set_m128i(S4, S5);
			R5 = _mm256_set_m128i(S5, S6);
			R6 = _mm256_set_m128i(S6, S7);
			R7 = _mm256_set_m128i(S7, S8);

			T0 = _mm256_unpacklo_epi8(R0, R7);
			T1 = _mm256_unpackhi_epi8(R0, R7);
			T2 = _mm256_unpacklo_epi8(R1, R6);
			T3 = _mm256_unpackhi_epi8(R1, R6);
			T4 = _mm256_unpacklo_epi8(R2, R5);
			T5 = _mm256_unpackhi_epi8(R2, R5);
			T6 = _mm256_unpacklo_epi8(R3, R4);
			T7 = _mm256_unpackhi_epi8(R3, R4);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff0);
			T2 = _mm256_maddubs_epi16(T2, coeff1);
			T3 = _mm256_maddubs_epi16(T3, coeff1);
			T4 = _mm256_maddubs_epi16(T4, coeff2);
			T5 = _mm256_maddubs_epi16(T5, coeff2);
			T6 = _mm256_maddubs_epi16(T6, coeff3);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T2);
			mVal2 = _mm256_add_epi16(T1, T3);
			mVal1 = _mm256_add_epi16(mVal1, T4);
			mVal2 = _mm256_add_epi16(mVal2, T5);
			mVal1 = _mm256_add_epi16(mVal1, T6);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu2_m128i((__m128i*)dst, (__m128i*)(dst + i_dst), mVal1);
			src += 2 * i_src;
			dst += 2 * i_dst;
		}
	}
	else
	{
		__m256i mAddOffset = _mm256_set1_epi16(offset);
		__m256i coeff0 = _mm256_set1_epi16(*(i16s_t*)coeff);
		__m256i coeff1 = _mm256_set1_epi16(*(i16s_t*)(coeff + 2));
		__m256i coeff2 = _mm256_set1_epi16(*(i16s_t*)(coeff + 4));
		__m256i coeff3 = _mm256_set1_epi16(*(i16s_t*)(coeff + 6));
		__m256i T0, T1, T2, T3, T4, T5, T6, T7, mVal1, mVal2;
		__m256i R0, R1, R2, R3, R4, R5, R6, R7;
		for (row = 0; row < height; row += 2)
		{
			__m128i S0 = _mm_loadu_si128((__m128i*)(src));
			__m128i S1 = _mm_loadu_si128((__m128i*)(src + i_src));
			__m128i S2 = _mm_loadu_si128((__m128i*)(src + i_src2));
			__m128i S3 = _mm_loadu_si128((__m128i*)(src + i_src3));
			__m128i S4 = _mm_loadu_si128((__m128i*)(src + i_src4));
			__m128i S5 = _mm_loadu_si128((__m128i*)(src + i_src5));
			__m128i S6 = _mm_loadu_si128((__m128i*)(src + i_src6));
			__m128i S7 = _mm_loadu_si128((__m128i*)(src + i_src7));
			__m128i S8 = _mm_loadu_si128((__m128i*)(src + i_src8));

			R0 = _mm256_set_m128i(S0, S1);
			R1 = _mm256_set_m128i(S1, S2);
			R2 = _mm256_set_m128i(S2, S3);
			R3 = _mm256_set_m128i(S3, S4);
			R4 = _mm256_set_m128i(S4, S5);
			R5 = _mm256_set_m128i(S5, S6);
			R6 = _mm256_set_m128i(S6, S7);
			R7 = _mm256_set_m128i(S7, S8);

			T0 = _mm256_unpacklo_epi8(R0, R1);
			T1 = _mm256_unpackhi_epi8(R0, R1);
			T2 = _mm256_unpacklo_epi8(R2, R3);
			T3 = _mm256_unpackhi_epi8(R2, R3);
			T4 = _mm256_unpacklo_epi8(R4, R5);
			T5 = _mm256_unpackhi_epi8(R4, R5);
			T6 = _mm256_unpacklo_epi8(R6, R7);
			T7 = _mm256_unpackhi_epi8(R6, R7);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff0);
			T2 = _mm256_maddubs_epi16(T2, coeff1);
			T3 = _mm256_maddubs_epi16(T3, coeff1);
			T4 = _mm256_maddubs_epi16(T4, coeff2);
			T5 = _mm256_maddubs_epi16(T5, coeff2);
			T6 = _mm256_maddubs_epi16(T6, coeff3);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T2);
			mVal2 = _mm256_add_epi16(T1, T3);
			mVal1 = _mm256_add_epi16(mVal1, T4);
			mVal2 = _mm256_add_epi16(mVal2, T5);
			mVal1 = _mm256_add_epi16(mVal1, T6);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu2_m128i((__m128i*)dst, (__m128i*)(dst + i_dst), mVal1);
			src += 2 * i_src;
			dst += 2 * i_dst;
		}
	}
}

void com_if_filter_ver_8_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[6]);
	__m256i mask24 = _mm256_setr_epi16(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0);
	const int i_src2 = i_src * 2;
	const int i_src3 = i_src * 3;
	const int i_src4 = i_src * 4;
	const int i_src5 = i_src * 5;
	const int i_src6 = i_src * 6;
	const int i_src7 = i_src * 7;

	src -= 3 * i_src;

	if (bsym)
	{
		__m256i mAddOffset = _mm256_set1_epi16(offset);
		__m256i coeff0 = _mm256_set1_epi8(coeff[0]);
		__m256i coeff1 = _mm256_set1_epi8(coeff[1]);
		__m256i coeff2 = _mm256_set1_epi8(coeff[2]);
		__m256i coeff3 = _mm256_set1_epi8(coeff[3]);
		__m256i T0, T1, T2, T3, T4, T5, T6, T7, mVal1, mVal2;

		for (row = 0; row < height; row++)
		{
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));
			__m256i S5 = _mm256_loadu_si256((__m256i*)(src + i_src5));
			__m256i S6 = _mm256_loadu_si256((__m256i*)(src + i_src6));
			__m256i S7 = _mm256_loadu_si256((__m256i*)(src + i_src7));

			T0 = _mm256_unpacklo_epi8(S0, S7);
			T1 = _mm256_unpacklo_epi8(S1, S6);
			T2 = _mm256_unpacklo_epi8(S2, S5);
			T3 = _mm256_unpacklo_epi8(S3, S4);
			T4 = _mm256_unpackhi_epi8(S0, S7);
			T5 = _mm256_unpackhi_epi8(S1, S6);
			T6 = _mm256_unpackhi_epi8(S2, S5);
			T7 = _mm256_unpackhi_epi8(S3, S4);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_maskstore_epi32((int*)(dst), mask24, mVal1);

			src += i_src;
			dst += i_dst;
		}
	}
	else
	{
		__m256i mAddOffset = _mm256_set1_epi16(offset);
		__m256i coeff0 = _mm256_set1_epi16(*(i32s_t*)coeff);
		__m256i coeff1 = _mm256_set1_epi16(*(i32s_t*)(coeff + 2));
		__m256i coeff2 = _mm256_set1_epi16(*(i32s_t*)(coeff + 4));
		__m256i coeff3 = _mm256_set1_epi16(*(i32s_t*)(coeff + 6));
		__m256i T0, T1, T2, T3, T4, T5, T6, T7, mVal1, mVal2;

		for (row = 0; row < height; row++)
		{
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));
			__m256i S5 = _mm256_loadu_si256((__m256i*)(src + i_src5));
			__m256i S6 = _mm256_loadu_si256((__m256i*)(src + i_src6));
			__m256i S7 = _mm256_loadu_si256((__m256i*)(src + i_src7));

			T0 = _mm256_unpacklo_epi8(S0, S1);
			T1 = _mm256_unpacklo_epi8(S2, S3);
			T2 = _mm256_unpacklo_epi8(S4, S5);
			T3 = _mm256_unpacklo_epi8(S6, S7);
			T4 = _mm256_unpackhi_epi8(S0, S1);
			T5 = _mm256_unpackhi_epi8(S2, S3);
			T6 = _mm256_unpackhi_epi8(S4, S5);
			T7 = _mm256_unpackhi_epi8(S6, S7);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_maskstore_epi32((int*)(dst), mask24, mVal1);

			src += i_src;
			dst += i_dst;
		}
	}
}

void com_if_filter_ver_8_w48_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[6]);
	__m256i mask16 = _mm256_setr_epi32(-1, -1, -1, -1, 0, 0, 0, 0);
	const int i_src2 = i_src * 2;
	const int i_src3 = i_src * 3;
	const int i_src4 = i_src * 4;
	const int i_src5 = i_src * 5;
	const int i_src6 = i_src * 6;
	const int i_src7 = i_src * 7;

	src -= 3 * i_src;

	if (bsym)
	{
		__m256i mAddOffset = _mm256_set1_epi16(offset);
		__m256i coeff0 = _mm256_set1_epi8(coeff[0]);
		__m256i coeff1 = _mm256_set1_epi8(coeff[1]);
		__m256i coeff2 = _mm256_set1_epi8(coeff[2]);
		__m256i coeff3 = _mm256_set1_epi8(coeff[3]);
		__m256i T0, T1, T2, T3, T4, T5, T6, T7, mVal1, mVal2;

		for (row = 0; row < height; row++)
		{
			const pel_t *p = src + 32;
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));
			__m256i S5 = _mm256_loadu_si256((__m256i*)(src + i_src5));
			__m256i S6 = _mm256_loadu_si256((__m256i*)(src + i_src6));
			__m256i S7 = _mm256_loadu_si256((__m256i*)(src + i_src7));

			T0 = _mm256_unpacklo_epi8(S0, S7);
			T1 = _mm256_unpacklo_epi8(S1, S6);
			T2 = _mm256_unpacklo_epi8(S2, S5);
			T3 = _mm256_unpacklo_epi8(S3, S4);
			T4 = _mm256_unpackhi_epi8(S0, S7);
			T5 = _mm256_unpackhi_epi8(S1, S6);
			T6 = _mm256_unpackhi_epi8(S2, S5);
			T7 = _mm256_unpackhi_epi8(S3, S4);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu_si256((__m256i*)(dst), mVal1);

			S0 = _mm256_loadu_si256((__m256i*)(p));
			S1 = _mm256_loadu_si256((__m256i*)(p + i_src));
			S2 = _mm256_loadu_si256((__m256i*)(p + i_src2));
			S3 = _mm256_loadu_si256((__m256i*)(p + i_src3));
			S4 = _mm256_loadu_si256((__m256i*)(p + i_src4));
			S5 = _mm256_loadu_si256((__m256i*)(p + i_src5));
			S6 = _mm256_loadu_si256((__m256i*)(p + i_src6));
			S7 = _mm256_loadu_si256((__m256i*)(p + i_src7));

			T0 = _mm256_unpacklo_epi8(S0, S7);
			T1 = _mm256_unpacklo_epi8(S1, S6);
			T2 = _mm256_unpacklo_epi8(S2, S5);
			T3 = _mm256_unpacklo_epi8(S3, S4);
			T4 = _mm256_unpackhi_epi8(S0, S7);
			T5 = _mm256_unpackhi_epi8(S1, S6);
			T6 = _mm256_unpackhi_epi8(S2, S5);
			T7 = _mm256_unpackhi_epi8(S3, S4);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_maskstore_epi32((int*)(dst + 32), mask16, mVal1);

			src += i_src;
			dst += i_dst;
		}
	}
	else
	{
		__m256i mAddOffset = _mm256_set1_epi16(offset);
		__m256i coeff0 = _mm256_set1_epi16(*(i32s_t*)coeff);
		__m256i coeff1 = _mm256_set1_epi16(*(i32s_t*)(coeff + 2));
		__m256i coeff2 = _mm256_set1_epi16(*(i32s_t*)(coeff + 4));
		__m256i coeff3 = _mm256_set1_epi16(*(i32s_t*)(coeff + 6));
		__m256i T0, T1, T2, T3, T4, T5, T6, T7, mVal1, mVal2;

		for (row = 0; row < height; row++)
		{
			const pel_t *p = src + 32;
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));
			__m256i S5 = _mm256_loadu_si256((__m256i*)(src + i_src5));
			__m256i S6 = _mm256_loadu_si256((__m256i*)(src + i_src6));
			__m256i S7 = _mm256_loadu_si256((__m256i*)(src + i_src7));

			T0 = _mm256_unpacklo_epi8(S0, S1);
			T1 = _mm256_unpacklo_epi8(S2, S3);
			T2 = _mm256_unpacklo_epi8(S4, S5);
			T3 = _mm256_unpacklo_epi8(S6, S7);
			T4 = _mm256_unpackhi_epi8(S0, S1);
			T5 = _mm256_unpackhi_epi8(S2, S3);
			T6 = _mm256_unpackhi_epi8(S4, S5);
			T7 = _mm256_unpackhi_epi8(S6, S7);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_storeu_si256((__m256i*)(dst), mVal1);

			S0 = _mm256_loadu_si256((__m256i*)(p));
			S1 = _mm256_loadu_si256((__m256i*)(p + i_src));
			S2 = _mm256_loadu_si256((__m256i*)(p + i_src2));
			S3 = _mm256_loadu_si256((__m256i*)(p + i_src3));
			S4 = _mm256_loadu_si256((__m256i*)(p + i_src4));
			S5 = _mm256_loadu_si256((__m256i*)(p + i_src5));
			S6 = _mm256_loadu_si256((__m256i*)(p + i_src6));
			S7 = _mm256_loadu_si256((__m256i*)(p + i_src7));

			T0 = _mm256_unpacklo_epi8(S0, S1);
			T1 = _mm256_unpacklo_epi8(S2, S3);
			T2 = _mm256_unpacklo_epi8(S4, S5);
			T3 = _mm256_unpacklo_epi8(S6, S7);
			T4 = _mm256_unpackhi_epi8(S0, S1);
			T5 = _mm256_unpackhi_epi8(S2, S3);
			T6 = _mm256_unpackhi_epi8(S4, S5);
			T7 = _mm256_unpackhi_epi8(S6, S7);

			T0 = _mm256_maddubs_epi16(T0, coeff0);
			T1 = _mm256_maddubs_epi16(T1, coeff1);
			T2 = _mm256_maddubs_epi16(T2, coeff2);
			T3 = _mm256_maddubs_epi16(T3, coeff3);
			T4 = _mm256_maddubs_epi16(T4, coeff0);
			T5 = _mm256_maddubs_epi16(T5, coeff1);
			T6 = _mm256_maddubs_epi16(T6, coeff2);
			T7 = _mm256_maddubs_epi16(T7, coeff3);

			mVal1 = _mm256_add_epi16(T0, T1);
			mVal2 = _mm256_add_epi16(T4, T5);
			mVal1 = _mm256_add_epi16(mVal1, T2);
			mVal2 = _mm256_add_epi16(mVal2, T6);
			mVal1 = _mm256_add_epi16(mVal1, T3);
			mVal2 = _mm256_add_epi16(mVal2, T7);

			mVal1 = _mm256_add_epi16(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi16(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi16(mVal1, shift);
			mVal2 = _mm256_srai_epi16(mVal2, shift);
			mVal1 = _mm256_packus_epi16(mVal1, mVal2);

			_mm256_maskstore_epi32((int*)(dst + 32), mask16, mVal1);

			src += i_src;
			dst += i_dst;
		}
	}
}

void com_if_filter_hor_ver_4_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
	ALIGNED_32(i16s_t tmp_res[(32 + 3) * 32]);
	i16s_t *tmp = tmp_res;
	const int i_tmp = 32;
	const int i_tmp2 = 64;
	const int i_tmp3 = 96;

	int row, col;
	int bsymy = (coef_y[1] == coef_y[6]);
	int shift = 12;
	__m256i mAddOffset = _mm256_set1_epi32(1 << 11);
	__m256i mCoef = _mm256_set1_epi32(*(i32s_t*)coef_x);
	__m256i mSwitch1 = _mm256_setr_epi8(0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6, 0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6);
	__m256i mSwitch2 = _mm256_setr_epi8(4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10, 4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10);
	__m256i T0, T1, S, S0, R1, R2, sum;

	//HOR
	src = src - i_src - 1;

	for (row = -1; row < height + 2; row++)
	{
		for (col = 0; col < width; col += 16)
		{
			S = _mm256_loadu_si256((__m256i*)(src + col));
			S0 = _mm256_permute4x64_epi64(S, 0x94);
			R1 = _mm256_shuffle_epi8(S0, mSwitch1);
			R2 = _mm256_shuffle_epi8(S0, mSwitch2);
			T0 = _mm256_maddubs_epi16(R1, mCoef);
			T1 = _mm256_maddubs_epi16(R2, mCoef);
			sum = _mm256_hadd_epi16(T0, T1);

			_mm256_storeu_si256((__m256i*)(tmp + col), sum);
		}
		src += i_src;
		tmp += i_tmp;
	}

	// VER
	tmp = tmp_res;
	if (bsymy)
	{
		__m256i mCoefy1 = _mm256_set1_epi16(coef_y[0]);
		__m256i mCoefy2 = _mm256_set1_epi16(coef_y[1]);
		__m256i mVal1, mVal2, mVal;
		__m256i T0, T1, T2, T3, S0, S1, S2, S3;


		for (row = 0; row < height; row++)
		{
			for (col = 0; col < width; col += 16)
			{
				S0 = _mm256_load_si256((__m256i*)(tmp + col));
				S1 = _mm256_load_si256((__m256i*)(tmp + col + i_tmp));
				S2 = _mm256_load_si256((__m256i*)(tmp + col + i_tmp2));
				S3 = _mm256_load_si256((__m256i*)(tmp + col + i_tmp3));

				T0 = _mm256_unpacklo_epi16(S0, S3);
				T1 = _mm256_unpacklo_epi16(S1, S2);
				T2 = _mm256_unpackhi_epi16(S0, S3);
				T3 = _mm256_unpackhi_epi16(S1, S2);

				T0 = _mm256_madd_epi16(T0, mCoefy1);
				T1 = _mm256_madd_epi16(T1, mCoefy2);
				T2 = _mm256_madd_epi16(T2, mCoefy1);
				T3 = _mm256_madd_epi16(T3, mCoefy2);

				mVal1 = _mm256_add_epi32(T0, T1);
				mVal2 = _mm256_add_epi32(T2, T3);

				mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm256_srai_epi32(mVal1, shift);
				mVal2 = _mm256_srai_epi32(mVal2, shift);

				mVal = _mm256_packs_epi32(mVal1, mVal2);
				mVal = _mm256_packus_epi16(mVal, mVal);

				mVal = _mm256_permute4x64_epi64(mVal, 0xd8);
				_mm_storeu_si128((__m128i*)(dst + col), _mm256_castsi256_si128(mVal));
			}
			tmp += i_tmp;
			dst += i_dst;
		}
	}
	else
	{
		__m128i mCoefy11 = _mm_set1_epi16(*(i16s_t*)coef_y);
		__m128i mCoefy22 = _mm_set1_epi16(*(i16s_t*)(coef_y + 2));
		__m256i mVal1, mVal2, mVal;
		__m256i T0, T1, T2, T3, S0, S1, S2, S3;

		__m256i mCoefy1 = _mm256_cvtepi8_epi16(mCoefy11);
		__m256i mCoefy2 = _mm256_cvtepi8_epi16(mCoefy22);

		for (row = 0; row < height; row++)
		{
			for (col = 0; col < width; col += 16)
			{
				S0 = _mm256_load_si256((__m256i*)(tmp + col));
				S1 = _mm256_load_si256((__m256i*)(tmp + col + i_tmp));
				S2 = _mm256_load_si256((__m256i*)(tmp + col + i_tmp2));
				S3 = _mm256_load_si256((__m256i*)(tmp + col + i_tmp3));

				T0 = _mm256_unpacklo_epi16(S0, S1);
				T1 = _mm256_unpacklo_epi16(S2, S3);
				T2 = _mm256_unpackhi_epi16(S0, S1);
				T3 = _mm256_unpackhi_epi16(S2, S3);

				T0 = _mm256_madd_epi16(T0, mCoefy1);
				T1 = _mm256_madd_epi16(T1, mCoefy2);
				T2 = _mm256_madd_epi16(T2, mCoefy1);
				T3 = _mm256_madd_epi16(T3, mCoefy2);

				mVal1 = _mm256_add_epi32(T0, T1);
				mVal2 = _mm256_add_epi32(T2, T3);

				mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm256_srai_epi32(mVal1, shift);
				mVal2 = _mm256_srai_epi32(mVal2, shift);

				mVal = _mm256_packs_epi32(mVal1, mVal2);
				mVal = _mm256_packus_epi16(mVal, mVal);

				mVal = _mm256_permute4x64_epi64(mVal, 0xd8);
				_mm_storeu_si128((__m128i*)(dst + col), _mm256_castsi256_si128(mVal));
			}
			tmp += i_tmp;
			dst += i_dst;
		}
	}
}

void com_if_filter_hor_ver_4_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
	ALIGNED_32(i16s_t tmp_res[(32 + 3) * 32]);
	i16s_t *tmp = tmp_res;
	const int i_tmp = 32;
	const int i_tmp2 = 64;
	const int i_tmp3 = 96;

	int row;
	int bsymy = (coef_y[1] == coef_y[6]);
	int shift = 12;
	__m256i mAddOffset = _mm256_set1_epi32(1 << 11);
	__m256i mCoef = _mm256_set1_epi32(*(i32s_t*)coef_x);
	__m256i mSwitch = _mm256_setr_epi8(0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6, 4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10);
	__m256i mSwitch1 = _mm256_setr_epi8(0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6, 0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6);
	__m256i mSwitch2 = _mm256_setr_epi8(4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10, 4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10);
	__m256i T0, T1, T2, S, S0, R1, R2, R3, sum1, sum2;
	__m256i mask24 = _mm256_setr_epi16(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0);
	//HOR
	src = src - i_src - 1;

	for (row = -1; row < height + 2; row++)
	{
		S = _mm256_loadu_si256((__m256i*)(src));
		S0 = _mm256_permute4x64_epi64(S, 0x99);
		R1 = _mm256_shuffle_epi8(S, mSwitch1);
		R2 = _mm256_shuffle_epi8(S, mSwitch2);
		R3 = _mm256_shuffle_epi8(S0, mSwitch);

		T0 = _mm256_maddubs_epi16(R1, mCoef);
		T1 = _mm256_maddubs_epi16(R2, mCoef);
		T2 = _mm256_maddubs_epi16(R3, mCoef);
		sum1 = _mm256_hadd_epi16(T0, T1);
		sum2 = _mm256_hadd_epi16(T2, T2);

		sum2 = _mm256_permute4x64_epi64(sum2, 0xd8);
		sum2 = _mm256_permute2x128_si256(sum1, sum2, 0x13);
		_mm_storeu_si128((__m128i*)(tmp), _mm256_castsi256_si128(sum1));
		_mm256_storeu_si256((__m256i*)(tmp + 8), sum2);
		src += i_src;
		tmp += i_tmp;
	}

	// VER
	tmp = tmp_res;
	if (bsymy)
	{
		__m256i mCoefy1 = _mm256_set1_epi16(coef_y[0]);
		__m256i mCoefy2 = _mm256_set1_epi16(coef_y[1]);
		__m256i mVal1, mVal2, mVal3, mVal4, mVal11, mVal22, mVal;
		__m256i T0, T1, T2, T3, S0, S1, S2, S3;
		__m256i T4, T5, T6, T7, S4, S5, S6, S7;

		for (row = 0; row < height; row++)
		{
			S0 = _mm256_load_si256((__m256i*)(tmp));
			S1 = _mm256_load_si256((__m256i*)(tmp + i_tmp));
			S2 = _mm256_load_si256((__m256i*)(tmp + i_tmp2));
			S3 = _mm256_load_si256((__m256i*)(tmp + i_tmp3));

			S4 = _mm256_load_si256((__m256i*)(tmp + 16));
			S5 = _mm256_load_si256((__m256i*)(tmp + 16 + i_tmp));
			S6 = _mm256_load_si256((__m256i*)(tmp + 16 + i_tmp2));
			S7 = _mm256_load_si256((__m256i*)(tmp + 16 + i_tmp3));

			T0 = _mm256_unpacklo_epi16(S0, S3);
			T1 = _mm256_unpacklo_epi16(S1, S2);
			T2 = _mm256_unpackhi_epi16(S0, S3);
			T3 = _mm256_unpackhi_epi16(S1, S2);

			T4 = _mm256_unpacklo_epi16(S4, S7);
			T5 = _mm256_unpacklo_epi16(S5, S6);
			T6 = _mm256_unpackhi_epi16(S4, S7);
			T7 = _mm256_unpackhi_epi16(S5, S6);

			T0 = _mm256_madd_epi16(T0, mCoefy1);
			T1 = _mm256_madd_epi16(T1, mCoefy2);
			T2 = _mm256_madd_epi16(T2, mCoefy1);
			T3 = _mm256_madd_epi16(T3, mCoefy2);

			T4 = _mm256_madd_epi16(T4, mCoefy1);
			T5 = _mm256_madd_epi16(T5, mCoefy2);
			T6 = _mm256_madd_epi16(T6, mCoefy1);
			T7 = _mm256_madd_epi16(T7, mCoefy2);

			mVal1 = _mm256_add_epi32(T0, T1);
			mVal2 = _mm256_add_epi32(T2, T3);

			mVal3 = _mm256_add_epi32(T4, T5);
			mVal4 = _mm256_add_epi32(T6, T7);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal3 = _mm256_add_epi32(mVal3, mAddOffset);
			mVal4 = _mm256_add_epi32(mVal4, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal3 = _mm256_srai_epi32(mVal3, shift);
			mVal4 = _mm256_srai_epi32(mVal4, shift);

			mVal11 = _mm256_packs_epi32(mVal1, mVal2);
			mVal22 = _mm256_packs_epi32(mVal3, mVal4);
			mVal = _mm256_packus_epi16(mVal11, mVal22);

			mVal = _mm256_permute4x64_epi64(mVal, 0xd8);
			_mm256_maskstore_epi32((int*)(dst), mask24, mVal);

			tmp += i_tmp;
			dst += i_dst;
		}
	}
	else
	{
		__m128i mCoefy11 = _mm_set1_epi16(*(i16s_t*)coef_y);
		__m128i mCoefy22 = _mm_set1_epi16(*(i16s_t*)(coef_y + 2));
		__m256i mVal1, mVal2, mVal3, mVal4, mVal11, mVal22, mVal;
		__m256i T0, T1, T2, T3, S0, S1, S2, S3;
		__m256i T4, T5, T6, T7, S4, S5, S6, S7;

		__m256i mCoefy1 = _mm256_cvtepi8_epi16(mCoefy11);
		__m256i mCoefy2 = _mm256_cvtepi8_epi16(mCoefy22);

		for (row = 0; row < height; row++)
		{
			S0 = _mm256_load_si256((__m256i*)(tmp));
			S1 = _mm256_load_si256((__m256i*)(tmp + i_tmp));
			S2 = _mm256_load_si256((__m256i*)(tmp + i_tmp2));
			S3 = _mm256_load_si256((__m256i*)(tmp + i_tmp3));

			S4 = _mm256_load_si256((__m256i*)(tmp + 16));
			S5 = _mm256_load_si256((__m256i*)(tmp + 16 + i_tmp));
			S6 = _mm256_load_si256((__m256i*)(tmp + 16 + i_tmp2));
			S7 = _mm256_load_si256((__m256i*)(tmp + 16 + i_tmp3));

			T0 = _mm256_unpacklo_epi16(S0, S1);
			T1 = _mm256_unpacklo_epi16(S2, S3);
			T2 = _mm256_unpackhi_epi16(S0, S1);
			T3 = _mm256_unpackhi_epi16(S2, S3);

			T4 = _mm256_unpacklo_epi16(S4, S5);
			T5 = _mm256_unpacklo_epi16(S6, S7);
			T6 = _mm256_unpackhi_epi16(S4, S5);
			T7 = _mm256_unpackhi_epi16(S6, S7);

			T0 = _mm256_madd_epi16(T0, mCoefy1);
			T1 = _mm256_madd_epi16(T1, mCoefy2);
			T2 = _mm256_madd_epi16(T2, mCoefy1);
			T3 = _mm256_madd_epi16(T3, mCoefy2);

			T4 = _mm256_madd_epi16(T4, mCoefy1);
			T5 = _mm256_madd_epi16(T5, mCoefy2);
			T6 = _mm256_madd_epi16(T6, mCoefy1);
			T7 = _mm256_madd_epi16(T7, mCoefy2);

			mVal1 = _mm256_add_epi32(T0, T1);
			mVal2 = _mm256_add_epi32(T2, T3);

			mVal3 = _mm256_add_epi32(T4, T5);
			mVal4 = _mm256_add_epi32(T6, T7);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal3 = _mm256_add_epi32(mVal3, mAddOffset);
			mVal4 = _mm256_add_epi32(mVal4, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal3 = _mm256_srai_epi32(mVal3, shift);
			mVal4 = _mm256_srai_epi32(mVal4, shift);

			mVal11 = _mm256_packs_epi32(mVal1, mVal2);
			mVal22 = _mm256_packs_epi32(mVal3, mVal4);
			mVal = _mm256_packus_epi16(mVal11, mVal22);

			mVal = _mm256_permute4x64_epi64(mVal, 0xd8);
			_mm256_maskstore_epi32((int*)(dst), mask24, mVal);

			tmp += i_tmp;
			dst += i_dst;
		}
	}
}

void com_if_filter_hor_ver_8_w16_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
	ALIGNED_32(i16s_t tmp_res[(64 + 7) * 64]);
	i16s_t *tmp = tmp_res;
	const int i_tmp = 64;
	const int i_tmp2 = 128;
	const int i_tmp3 = 192;
	const int i_tmp4 = 256;
	const int i_tmp5 = 320;
	const int i_tmp6 = 384;
	const int i_tmp7 = 448;

	int row, col;
	int bsymy = (coef_y[1] == coef_y[6]);
	int shift = 12;
	__m256i mAddOffset = _mm256_set1_epi32(1 << 11);
	__m256i mSwitch1 = _mm256_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8);
	__m256i mSwitch2 = _mm256_setr_epi8(2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10);
	__m256i mSwitch3 = _mm256_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12);
	__m256i mSwitch4 = _mm256_setr_epi8(6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14);
	__m256i mCoef;
	__m256i T0, T1, S0, sum, S, T2, T3;
	__m256i r0, r1, r2, r3;

	src = src - 3 * i_src - 3;

	//HOR
#if defined(_WIN64)
	mCoef = _mm256_set1_epi64x(*(long long*)coef_x);
#else
	mCoef = _mm256_loadu_si256((__m256i*)coef_x);
	mCoef = _mm256_permute4x64_epi64(mCoef, 0x0);
#endif

	for (row = -3; row < height + 4; row++)
	{
		for (col = 0; col < width; col += 16)
		{
			S = _mm256_loadu_si256((__m256i*)(src + col));
			S0 = _mm256_permute4x64_epi64(S, 0x94);

			r0 = _mm256_shuffle_epi8(S0, mSwitch1);
			r1 = _mm256_shuffle_epi8(S0, mSwitch2);
			r2 = _mm256_shuffle_epi8(S0, mSwitch3);
			r3 = _mm256_shuffle_epi8(S0, mSwitch4);

			T0 = _mm256_maddubs_epi16(r0, mCoef);
			T1 = _mm256_maddubs_epi16(r1, mCoef);
			T2 = _mm256_maddubs_epi16(r2, mCoef);
			T3 = _mm256_maddubs_epi16(r3, mCoef);

			T0 = _mm256_hadd_epi16(T0, T1);
			T1 = _mm256_hadd_epi16(T2, T3);
			sum = _mm256_hadd_epi16(T0, T1);

			_mm256_storeu_si256((__m256i*)(tmp + col), sum);
		}
		src += i_src;
		tmp += i_tmp;
	}

	// VER
	tmp = tmp_res;
	if (bsymy)
	{
		__m256i mCoefy1 = _mm256_set1_epi16(coef_y[0]);
		__m256i mCoefy2 = _mm256_set1_epi16(coef_y[1]);
		__m256i mCoefy3 = _mm256_set1_epi16(coef_y[2]);
		__m256i mCoefy4 = _mm256_set1_epi16(coef_y[3]);
		__m256i mVal1, mVal2, mVal;
		__m256i T0, T1, T2, T3, S0, S1, S2, S3;
		__m256i T4, T5, T6, T7, S4, S5, S6, S7;

		for (row = 0; row < height; row++)
		{
			for (col = 0; col < width; col += 16)
			{
				S0 = _mm256_loadu_si256((__m256i*)(tmp + col));
				S1 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp));
				S2 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp2));
				S3 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp3));
				S4 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp4));
				S5 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp5));
				S6 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp6));
				S7 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp7));

				T0 = _mm256_unpacklo_epi16(S0, S7);
				T1 = _mm256_unpacklo_epi16(S1, S6);
				T2 = _mm256_unpacklo_epi16(S2, S5);
				T3 = _mm256_unpacklo_epi16(S3, S4);
				T4 = _mm256_unpackhi_epi16(S0, S7);
				T5 = _mm256_unpackhi_epi16(S1, S6);
				T6 = _mm256_unpackhi_epi16(S2, S5);
				T7 = _mm256_unpackhi_epi16(S3, S4);


				T0 = _mm256_madd_epi16(T0, mCoefy1);
				T1 = _mm256_madd_epi16(T1, mCoefy2);
				T2 = _mm256_madd_epi16(T2, mCoefy3);
				T3 = _mm256_madd_epi16(T3, mCoefy4);
				T4 = _mm256_madd_epi16(T4, mCoefy1);
				T5 = _mm256_madd_epi16(T5, mCoefy2);
				T6 = _mm256_madd_epi16(T6, mCoefy3);
				T7 = _mm256_madd_epi16(T7, mCoefy4);

				mVal1 = _mm256_add_epi32(T0, T1);
				mVal2 = _mm256_add_epi32(T4, T5);
				mVal1 = _mm256_add_epi32(mVal1, T2);
				mVal2 = _mm256_add_epi32(mVal2, T6);
				mVal1 = _mm256_add_epi32(mVal1, T3);
				mVal2 = _mm256_add_epi32(mVal2, T7);

				mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm256_srai_epi32(mVal1, shift);
				mVal2 = _mm256_srai_epi32(mVal2, shift);

				mVal = _mm256_packs_epi32(mVal1, mVal2);
				mVal = _mm256_packus_epi16(mVal, mVal);

				mVal = _mm256_permute4x64_epi64(mVal, 0xd8);
				_mm_storeu_si128((__m128i*)(dst + col), _mm256_castsi256_si128(mVal));
			}
			tmp += i_tmp;
			dst += i_dst;
		}
	}
	else
	{
		__m128i mCoefy11 = _mm_set1_epi16(*(i16s_t*)coef_y);
		__m128i mCoefy22 = _mm_set1_epi16(*(i16s_t*)(coef_y + 2));
		__m128i mCoefy33 = _mm_set1_epi16(*(i16s_t*)(coef_y + 4));
		__m128i mCoefy44 = _mm_set1_epi16(*(i16s_t*)(coef_y + 6));
		__m256i mVal1, mVal2, mVal;
		__m256i T0, T1, T2, T3, S0, S1, S2, S3;
		__m256i T4, T5, T6, T7, S4, S5, S6, S7;

		__m256i mCoefy1 = _mm256_cvtepi8_epi16(mCoefy11);
		__m256i mCoefy2 = _mm256_cvtepi8_epi16(mCoefy22);
		__m256i mCoefy3 = _mm256_cvtepi8_epi16(mCoefy33);
		__m256i mCoefy4 = _mm256_cvtepi8_epi16(mCoefy44);

		for (row = 0; row < height; row++)
		{
			for (col = 0; col < width; col += 16)
			{
				S0 = _mm256_loadu_si256((__m256i*)(tmp + col));
				S1 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp));
				S2 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp2));
				S3 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp3));
				S4 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp4));
				S5 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp5));
				S6 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp6));
				S7 = _mm256_loadu_si256((__m256i*)(tmp + col + i_tmp7));

				T0 = _mm256_unpacklo_epi16(S0, S1);
				T1 = _mm256_unpacklo_epi16(S2, S3);
				T2 = _mm256_unpacklo_epi16(S4, S5);
				T3 = _mm256_unpacklo_epi16(S6, S7);
				T4 = _mm256_unpackhi_epi16(S0, S1);
				T5 = _mm256_unpackhi_epi16(S2, S3);
				T6 = _mm256_unpackhi_epi16(S4, S5);
				T7 = _mm256_unpackhi_epi16(S6, S7);


				T0 = _mm256_madd_epi16(T0, mCoefy1);
				T1 = _mm256_madd_epi16(T1, mCoefy2);
				T2 = _mm256_madd_epi16(T2, mCoefy3);
				T3 = _mm256_madd_epi16(T3, mCoefy4);
				T4 = _mm256_madd_epi16(T4, mCoefy1);
				T5 = _mm256_madd_epi16(T5, mCoefy2);
				T6 = _mm256_madd_epi16(T6, mCoefy3);
				T7 = _mm256_madd_epi16(T7, mCoefy4);

				mVal1 = _mm256_add_epi32(T0, T1);
				mVal2 = _mm256_add_epi32(T4, T5);
				mVal1 = _mm256_add_epi32(mVal1, T2);
				mVal2 = _mm256_add_epi32(mVal2, T6);
				mVal1 = _mm256_add_epi32(mVal1, T3);
				mVal2 = _mm256_add_epi32(mVal2, T7);

				mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm256_srai_epi32(mVal1, shift);
				mVal2 = _mm256_srai_epi32(mVal2, shift);

				mVal = _mm256_packs_epi32(mVal1, mVal2);
				mVal = _mm256_packus_epi16(mVal, mVal);

				mVal = _mm256_permute4x64_epi64(mVal, 0xd8);
				_mm_storeu_si128((__m128i*)(dst + col), _mm256_castsi256_si128(mVal));
			}
			tmp += i_tmp;
			dst += i_dst;
		}
	}
}

void com_if_filter_hor_ver_8_w24_sse256(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
	ALIGNED_32(i16s_t tmp_res[(64 + 7) * 64]);
	i16s_t *tmp = tmp_res;
	const int i_tmp = 64;
	const int i_tmp2 = 128;
	const int i_tmp3 = 192;
	const int i_tmp4 = 256;
	const int i_tmp5 = 320;
	const int i_tmp6 = 384;
	const int i_tmp7 = 448;

	int row;
	int bsymy = (coef_y[1] == coef_y[6]);
	int shift = 12;
	__m256i mAddOffset = _mm256_set1_epi32(1 << 11);
	__m256i mCoef;
	__m256i mask24 = _mm256_setr_epi16(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0);
	

	//HOR
	__m256i T0, T1, S1, sum1, sum2, S, T2, T3, T4, T5;
	__m256i r0, r1, r2, r3, r4, r5;
	__m256i mSwitch1 = _mm256_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12);
	__m256i mSwitch2 = _mm256_setr_epi8(2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14);
	__m256i mSwitch3 = _mm256_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8);
	__m256i mSwitch4 = _mm256_setr_epi8(2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10);
	__m256i mSwitch5 = _mm256_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12);
	__m256i mSwitch6 = _mm256_setr_epi8(6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14);
	src = src - 3 * i_src - 3;
#if defined(_WIN64)
	mCoef = _mm256_set1_epi64x(*(long long*)coef_x);
#else
	mCoef = _mm256_loadu_si256((__m256i*)coef_x);
	mCoef = _mm256_permute4x64_epi64(mCoef, 0x0);
#endif

	for (row = -3; row < height + 4; row++)
	{
		S = _mm256_loadu_si256((__m256i*)(src));
		S1 = _mm256_permute4x64_epi64(S, 0x99);

		r0 = _mm256_shuffle_epi8(S1, mSwitch1);
		r1 = _mm256_shuffle_epi8(S1, mSwitch2);
		r2 = _mm256_shuffle_epi8(S, mSwitch3);
		r3 = _mm256_shuffle_epi8(S, mSwitch4);
		r4 = _mm256_shuffle_epi8(S, mSwitch5);
		r5 = _mm256_shuffle_epi8(S, mSwitch6);

		T0 = _mm256_maddubs_epi16(r0, mCoef);
		T1 = _mm256_maddubs_epi16(r1, mCoef);
		T2 = _mm256_maddubs_epi16(r2, mCoef);
		T3 = _mm256_maddubs_epi16(r3, mCoef);
		T4 = _mm256_maddubs_epi16(r4, mCoef);
		T5 = _mm256_maddubs_epi16(r5, mCoef);

		T0 = _mm256_hadd_epi16(T0, T1);
		T1 = _mm256_hadd_epi16(T2, T3);
		T2 = _mm256_hadd_epi16(T4, T5);
		sum1 = _mm256_hadd_epi16(T1, T2);
		sum2 = _mm256_hadd_epi16(T0, T0);

		sum2 = _mm256_permute4x64_epi64(sum2, 0xd8);
		sum2 = _mm256_permute2x128_si256(sum1, sum2, 0x13);
		_mm_storeu_si128((__m128i*)(tmp), _mm256_castsi256_si128(sum1));
		_mm256_storeu_si256((__m256i*)(tmp + 8), sum2);

		src += i_src;
		tmp += i_tmp;
	}

	// VER
	tmp = tmp_res;
	if (bsymy)
	{
		__m256i mCoefy1 = _mm256_set1_epi16(coef_y[0]);
		__m256i mCoefy2 = _mm256_set1_epi16(coef_y[1]);
		__m256i mCoefy3 = _mm256_set1_epi16(coef_y[2]);
		__m256i mCoefy4 = _mm256_set1_epi16(coef_y[3]);
		__m256i mVal1, mVal2, mVal, mVal3, mVal4;
		__m256i T0, T1, T2, T3, S0, S1, S2, S3;
		__m256i T4, T5, T6, T7, S4, S5, S6, S7;
		__m256i T00, T11, T22, T33, S00, S11, S22, S33;
		__m256i T44, T55, T66, T77, S44, S55, S66, S77;

		for (row = 0; row < height; row++)
		{
			S0 = _mm256_loadu_si256((__m256i*)(tmp));
			S1 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp));
			S2 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp2));
			S3 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp3));
			S4 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp4));
			S5 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp5));
			S6 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp6));
			S7 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp7));

			S00 = _mm256_loadu_si256((__m256i*)(tmp + 16));
			S11 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp));
			S22 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp2));
			S33 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp3));
			S44 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp4));
			S55 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp5));
			S66 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp6));
			S77 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp7));

			T0 = _mm256_unpacklo_epi16(S0, S7);
			T1 = _mm256_unpacklo_epi16(S1, S6);
			T2 = _mm256_unpacklo_epi16(S2, S5);
			T3 = _mm256_unpacklo_epi16(S3, S4);
			T4 = _mm256_unpackhi_epi16(S0, S7);
			T5 = _mm256_unpackhi_epi16(S1, S6);
			T6 = _mm256_unpackhi_epi16(S2, S5);
			T7 = _mm256_unpackhi_epi16(S3, S4);

			T00 = _mm256_unpacklo_epi16(S00, S77);
			T11 = _mm256_unpacklo_epi16(S11, S66);
			T22 = _mm256_unpacklo_epi16(S22, S55);
			T33 = _mm256_unpacklo_epi16(S33, S44);
			T44 = _mm256_unpackhi_epi16(S00, S77);
			T55 = _mm256_unpackhi_epi16(S11, S66);
			T66 = _mm256_unpackhi_epi16(S22, S55);
			T77 = _mm256_unpackhi_epi16(S33, S44);

			T0 = _mm256_madd_epi16(T0, mCoefy1);
			T1 = _mm256_madd_epi16(T1, mCoefy2);
			T2 = _mm256_madd_epi16(T2, mCoefy3);
			T3 = _mm256_madd_epi16(T3, mCoefy4);
			T4 = _mm256_madd_epi16(T4, mCoefy1);
			T5 = _mm256_madd_epi16(T5, mCoefy2);
			T6 = _mm256_madd_epi16(T6, mCoefy3);
			T7 = _mm256_madd_epi16(T7, mCoefy4);

			T00 = _mm256_madd_epi16(T00, mCoefy1);
			T11 = _mm256_madd_epi16(T11, mCoefy2);
			T22 = _mm256_madd_epi16(T22, mCoefy3);
			T33 = _mm256_madd_epi16(T33, mCoefy4);
			T44 = _mm256_madd_epi16(T44, mCoefy1);
			T55 = _mm256_madd_epi16(T55, mCoefy2);
			T66 = _mm256_madd_epi16(T66, mCoefy3);
			T77 = _mm256_madd_epi16(T77, mCoefy4);

			mVal1 = _mm256_add_epi32(T0, T1);
			mVal2 = _mm256_add_epi32(T4, T5);
			mVal1 = _mm256_add_epi32(mVal1, T2);
			mVal2 = _mm256_add_epi32(mVal2, T6);
			mVal1 = _mm256_add_epi32(mVal1, T3);
			mVal2 = _mm256_add_epi32(mVal2, T7);

			mVal3 = _mm256_add_epi32(T00, T11);
			mVal4 = _mm256_add_epi32(T44, T55);
			mVal3 = _mm256_add_epi32(mVal3, T22);
			mVal4 = _mm256_add_epi32(mVal4, T66);
			mVal3 = _mm256_add_epi32(mVal3, T33);
			mVal4 = _mm256_add_epi32(mVal4, T77);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal3 = _mm256_add_epi32(mVal3, mAddOffset);
			mVal4 = _mm256_add_epi32(mVal4, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal3 = _mm256_srai_epi32(mVal3, shift);
			mVal4 = _mm256_srai_epi32(mVal4, shift);

			mVal1 = _mm256_packs_epi32(mVal1, mVal2);
			mVal3 = _mm256_packs_epi32(mVal3, mVal4);
			mVal = _mm256_packus_epi16(mVal1, mVal3);

			mVal = _mm256_permute4x64_epi64(mVal, 0xd8);
			_mm256_maskstore_epi32((int*)(dst), mask24, mVal);

			tmp += i_tmp;
			dst += i_dst;
		}
	}
	else
	{
		__m128i mCoefy11 = _mm_set1_epi16(*(i16s_t*)coef_y);
		__m128i mCoefy22 = _mm_set1_epi16(*(i16s_t*)(coef_y + 2));
		__m128i mCoefy33 = _mm_set1_epi16(*(i16s_t*)(coef_y + 4));
		__m128i mCoefy44 = _mm_set1_epi16(*(i16s_t*)(coef_y + 6));
		__m256i mVal1, mVal2, mVal, mVal3, mVal4;
		__m256i T0, T1, T2, T3, S0, S1, S2, S3;
		__m256i T4, T5, T6, T7, S4, S5, S6, S7;
		__m256i T00, T11, T22, T33, S00, S11, S22, S33;
		__m256i T44, T55, T66, T77, S44, S55, S66, S77;

		__m256i mCoefy1 = _mm256_cvtepi8_epi16(mCoefy11);
		__m256i mCoefy2 = _mm256_cvtepi8_epi16(mCoefy22);
		__m256i mCoefy3 = _mm256_cvtepi8_epi16(mCoefy33);
		__m256i mCoefy4 = _mm256_cvtepi8_epi16(mCoefy44);

		for (row = 0; row < height; row++)
		{
			S0 = _mm256_loadu_si256((__m256i*)(tmp));
			S1 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp));
			S2 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp2));
			S3 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp3));
			S4 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp4));
			S5 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp5));
			S6 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp6));
			S7 = _mm256_loadu_si256((__m256i*)(tmp + i_tmp7));

			S00 = _mm256_loadu_si256((__m256i*)(tmp + 16));
			S11 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp));
			S22 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp2));
			S33 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp3));
			S44 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp4));
			S55 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp5));
			S66 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp6));
			S77 = _mm256_loadu_si256((__m256i*)(tmp + 16 + i_tmp7));

			T0 = _mm256_unpacklo_epi16(S0, S1);
			T1 = _mm256_unpacklo_epi16(S2, S3);
			T2 = _mm256_unpacklo_epi16(S4, S5);
			T3 = _mm256_unpacklo_epi16(S6, S7);
			T4 = _mm256_unpackhi_epi16(S0, S1);
			T5 = _mm256_unpackhi_epi16(S2, S3);
			T6 = _mm256_unpackhi_epi16(S4, S5);
			T7 = _mm256_unpackhi_epi16(S6, S7);

			T00 = _mm256_unpacklo_epi16(S00, S11);
			T11 = _mm256_unpacklo_epi16(S22, S33);
			T22 = _mm256_unpacklo_epi16(S44, S55);
			T33 = _mm256_unpacklo_epi16(S66, S77);
			T44 = _mm256_unpackhi_epi16(S00, S11);
			T55 = _mm256_unpackhi_epi16(S22, S33);
			T66 = _mm256_unpackhi_epi16(S44, S55);
			T77 = _mm256_unpackhi_epi16(S66, S77);

			T0 = _mm256_madd_epi16(T0, mCoefy1);
			T1 = _mm256_madd_epi16(T1, mCoefy2);
			T2 = _mm256_madd_epi16(T2, mCoefy3);
			T3 = _mm256_madd_epi16(T3, mCoefy4);
			T4 = _mm256_madd_epi16(T4, mCoefy1);
			T5 = _mm256_madd_epi16(T5, mCoefy2);
			T6 = _mm256_madd_epi16(T6, mCoefy3);
			T7 = _mm256_madd_epi16(T7, mCoefy4);

			T00 = _mm256_madd_epi16(T00, mCoefy1);
			T11 = _mm256_madd_epi16(T11, mCoefy2);
			T22 = _mm256_madd_epi16(T22, mCoefy3);
			T33 = _mm256_madd_epi16(T33, mCoefy4);
			T44 = _mm256_madd_epi16(T44, mCoefy1);
			T55 = _mm256_madd_epi16(T55, mCoefy2);
			T66 = _mm256_madd_epi16(T66, mCoefy3);
			T77 = _mm256_madd_epi16(T77, mCoefy4);

			mVal1 = _mm256_add_epi32(T0, T1);
			mVal2 = _mm256_add_epi32(T4, T5);
			mVal1 = _mm256_add_epi32(mVal1, T2);
			mVal2 = _mm256_add_epi32(mVal2, T6);
			mVal1 = _mm256_add_epi32(mVal1, T3);
			mVal2 = _mm256_add_epi32(mVal2, T7);

			mVal3 = _mm256_add_epi32(T00, T11);
			mVal4 = _mm256_add_epi32(T44, T55);
			mVal3 = _mm256_add_epi32(mVal3, T22);
			mVal4 = _mm256_add_epi32(mVal4, T66);
			mVal3 = _mm256_add_epi32(mVal3, T33);
			mVal4 = _mm256_add_epi32(mVal4, T77);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal3 = _mm256_add_epi32(mVal3, mAddOffset);
			mVal4 = _mm256_add_epi32(mVal4, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal3 = _mm256_srai_epi32(mVal3, shift);
			mVal4 = _mm256_srai_epi32(mVal4, shift);

			mVal1 = _mm256_packs_epi32(mVal1, mVal2);
			mVal3 = _mm256_packs_epi32(mVal3, mVal4);
			mVal = _mm256_packus_epi16(mVal1, mVal3);

			mVal = _mm256_permute4x64_epi64(mVal, 0xd8);
			_mm256_maskstore_epi32((int*)(dst), mask24, mVal);

			tmp += i_tmp;
			dst += i_dst;
		}
	}
}

void com_if_filter_ver_4_w16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[2]);
	__m256i mAddOffset = _mm256_set1_epi32(offset);
	const int i_src2 = i_src << 1;
	const int i_src3 = i_src + i_src2;
	const int i_src4 = i_src << 2;

	__m256i max_val1 = _mm256_set1_epi16((pel_t)max_val);

	src -= i_src;

	if (bsym) {
		__m128i coeff1 = _mm_set1_epi16(((i16s_t*)coeff)[0]);
		__m256i coeff0 = _mm256_cvtepi8_epi16(coeff1);
		__m256i T0, T1, T2, T3, mVal1, mVal2, mVal3, mVal4;
		__m256i R0, R1, R2, R3;
		for (row = 0; row < height; row += 2) {
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));

			R0 = _mm256_add_epi16(S0, S3);
			R1 = _mm256_add_epi16(S1, S2);
			R2 = _mm256_add_epi16(S1, S4);
			R3 = _mm256_add_epi16(S2, S3);

			T0 = _mm256_unpacklo_epi16(R0, R1);
			T1 = _mm256_unpackhi_epi16(R0, R1);
			T2 = _mm256_unpacklo_epi16(R2, R3);
			T3 = _mm256_unpackhi_epi16(R2, R3);

			T0 = _mm256_madd_epi16(T0, coeff0);
			T1 = _mm256_madd_epi16(T1, coeff0);
			T2 = _mm256_madd_epi16(T2, coeff0);
			T3 = _mm256_madd_epi16(T3, coeff0);

			mVal1 = _mm256_add_epi32(T0, mAddOffset);
			mVal2 = _mm256_add_epi32(T1, mAddOffset);
			mVal3 = _mm256_add_epi32(T2, mAddOffset);
			mVal4 = _mm256_add_epi32(T3, mAddOffset);

			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal3 = _mm256_srai_epi32(mVal3, shift);
			mVal4 = _mm256_srai_epi32(mVal4, shift);

			mVal1 = _mm256_packus_epi32(mVal1, mVal2);
			mVal3 = _mm256_packus_epi32(mVal3, mVal4);

			mVal1 = _mm256_min_epu16(mVal1, max_val1);
			mVal3 = _mm256_min_epu16(mVal3, max_val1);
			_mm256_storeu_si256((__m256i*)dst, mVal1);
			_mm256_storeu_si256((__m256i*)(dst + i_dst), mVal3);

			src += i_src2;
			dst += 2 * i_dst;
		}
	}
	else {
		__m128i coeff00 = _mm_set1_epi16(*(i16s_t*)coeff);
		__m128i coeff11 = _mm_set1_epi16(*(i16s_t*)(coeff + 2));
		__m256i coeff0 = _mm256_cvtepi8_epi16(coeff00);
		__m256i coeff1 = _mm256_cvtepi8_epi16(coeff11);
		__m256i T0, T1, T2, T3, mVal1, mVal2;
		for (row = 0; row < height; row += 2) {
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));

			T0 = _mm256_unpacklo_epi16(S0, S1);
			T1 = _mm256_unpackhi_epi16(S0, S1);
			T2 = _mm256_unpacklo_epi16(S2, S3);
			T3 = _mm256_unpackhi_epi16(S2, S3);

			T0 = _mm256_madd_epi16(T0, coeff0);
			T1 = _mm256_madd_epi16(T1, coeff0);
			T2 = _mm256_madd_epi16(T2, coeff1);
			T3 = _mm256_madd_epi16(T3, coeff1);

			mVal1 = _mm256_add_epi32(T0, T2);
			mVal2 = _mm256_add_epi32(T1, T3);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal1 = _mm256_packus_epi32(mVal1, mVal2);

			mVal1 = _mm256_min_epu16(mVal1, max_val1);
			_mm256_storeu_si256((__m256i*)dst, mVal1);

			T0 = _mm256_unpacklo_epi16(S1, S2);
			T1 = _mm256_unpackhi_epi16(S1, S2);
			T2 = _mm256_unpacklo_epi16(S3, S4);
			T3 = _mm256_unpackhi_epi16(S3, S4);

			T0 = _mm256_madd_epi16(T0, coeff0);
			T1 = _mm256_madd_epi16(T1, coeff0);
			T2 = _mm256_madd_epi16(T2, coeff1);
			T3 = _mm256_madd_epi16(T3, coeff1);

			mVal1 = _mm256_add_epi32(T0, T2);
			mVal2 = _mm256_add_epi32(T1, T3);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal1 = _mm256_packus_epi32(mVal1, mVal2);

			mVal1 = _mm256_min_epu16(mVal1, max_val1);
			_mm256_storeu_si256((__m256i*)(dst + i_dst), mVal1);

			src += i_src2;
			dst += 2 * i_dst;
		}
	}
}

void com_if_filter_ver_4_w32_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
	int row;
	const int offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[2]);
	__m256i mAddOffset = _mm256_set1_epi32(offset);
	const int i_src2 = i_src << 1;
	const int i_src3 = i_src + i_src2;
	const int i_src4 = i_src << 2;

	__m256i max_val1 = _mm256_set1_epi16((pel_t)max_val);

	src -= i_src;

	if (bsym) {
		__m128i coeff1 = _mm_set1_epi16(((i16s_t*)coeff)[0]);
		__m256i coeff0 = _mm256_cvtepi8_epi16(coeff1);
		__m256i T0, T1, T2, T3, mVal1, mVal2, mVal3, mVal4;
		__m256i R0, R1, R2, R3;
		for (row = 0; row < height; row += 2) {
			const pel_t *src1 = src + 16;
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));

			R0 = _mm256_add_epi16(S0, S3);
			R1 = _mm256_add_epi16(S1, S2);
			R2 = _mm256_add_epi16(S1, S4);
			R3 = _mm256_add_epi16(S2, S3);

			T0 = _mm256_unpacklo_epi16(R0, R1);
			T1 = _mm256_unpackhi_epi16(R0, R1);
			T2 = _mm256_unpacklo_epi16(R2, R3);
			T3 = _mm256_unpackhi_epi16(R2, R3);

			T0 = _mm256_madd_epi16(T0, coeff0);
			T1 = _mm256_madd_epi16(T1, coeff0);
			T2 = _mm256_madd_epi16(T2, coeff0);
			T3 = _mm256_madd_epi16(T3, coeff0);

			mVal1 = _mm256_add_epi32(T0, mAddOffset);
			mVal2 = _mm256_add_epi32(T1, mAddOffset);
			mVal3 = _mm256_add_epi32(T2, mAddOffset);
			mVal4 = _mm256_add_epi32(T3, mAddOffset);

			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal3 = _mm256_srai_epi32(mVal3, shift);
			mVal4 = _mm256_srai_epi32(mVal4, shift);

			mVal1 = _mm256_packus_epi32(mVal1, mVal2);
			mVal3 = _mm256_packus_epi32(mVal3, mVal4);

			mVal1 = _mm256_min_epu16(mVal1, max_val1);
			mVal3 = _mm256_min_epu16(mVal3, max_val1);
			_mm256_storeu_si256((__m256i*)dst, mVal1);
			_mm256_storeu_si256((__m256i*)(dst + i_dst), mVal3);

			S0 = _mm256_loadu_si256((__m256i*)(src1));
			S1 = _mm256_loadu_si256((__m256i*)(src1 + i_src));
			S2 = _mm256_loadu_si256((__m256i*)(src1 + i_src2));
			S3 = _mm256_loadu_si256((__m256i*)(src1 + i_src3));
			S4 = _mm256_loadu_si256((__m256i*)(src1 + i_src4));

			R0 = _mm256_add_epi16(S0, S3);
			R1 = _mm256_add_epi16(S1, S2);
			R2 = _mm256_add_epi16(S1, S4);
			R3 = _mm256_add_epi16(S2, S3);

			T0 = _mm256_unpacklo_epi16(R0, R1);
			T1 = _mm256_unpackhi_epi16(R0, R1);
			T2 = _mm256_unpacklo_epi16(R2, R3);
			T3 = _mm256_unpackhi_epi16(R2, R3);

			T0 = _mm256_madd_epi16(T0, coeff0);
			T1 = _mm256_madd_epi16(T1, coeff0);
			T2 = _mm256_madd_epi16(T2, coeff0);
			T3 = _mm256_madd_epi16(T3, coeff0);

			mVal1 = _mm256_add_epi32(T0, mAddOffset);
			mVal2 = _mm256_add_epi32(T1, mAddOffset);
			mVal3 = _mm256_add_epi32(T2, mAddOffset);
			mVal4 = _mm256_add_epi32(T3, mAddOffset);

			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal3 = _mm256_srai_epi32(mVal3, shift);
			mVal4 = _mm256_srai_epi32(mVal4, shift);

			mVal1 = _mm256_packus_epi32(mVal1, mVal2);
			mVal3 = _mm256_packus_epi32(mVal3, mVal4);

			mVal1 = _mm256_min_epu16(mVal1, max_val1);
			mVal3 = _mm256_min_epu16(mVal3, max_val1);
			_mm256_storeu_si256((__m256i*)(dst + 16), mVal1);
			_mm256_storeu_si256((__m256i*)(dst + i_dst + 16), mVal3);

			src += i_src2;
			dst += 2 * i_dst;
		}
	}
	else {
		__m128i coeff00 = _mm_set1_epi16(*(i16s_t*)coeff);
		__m128i coeff11 = _mm_set1_epi16(*(i16s_t*)(coeff + 2));
		__m256i coeff0 = _mm256_cvtepi8_epi16(coeff00);
		__m256i coeff1 = _mm256_cvtepi8_epi16(coeff11);
		__m256i T0, T1, T2, T3, mVal1, mVal2;
		for (row = 0; row < height; row += 2) {
			const pel_t *src1 = src + 16;
			__m256i S0 = _mm256_loadu_si256((__m256i*)(src));
			__m256i S1 = _mm256_loadu_si256((__m256i*)(src + i_src));
			__m256i S2 = _mm256_loadu_si256((__m256i*)(src + i_src2));
			__m256i S3 = _mm256_loadu_si256((__m256i*)(src + i_src3));
			__m256i S4 = _mm256_loadu_si256((__m256i*)(src + i_src4));

			T0 = _mm256_unpacklo_epi16(S0, S1);
			T1 = _mm256_unpackhi_epi16(S0, S1);
			T2 = _mm256_unpacklo_epi16(S2, S3);
			T3 = _mm256_unpackhi_epi16(S2, S3);

			T0 = _mm256_madd_epi16(T0, coeff0);
			T1 = _mm256_madd_epi16(T1, coeff0);
			T2 = _mm256_madd_epi16(T2, coeff1);
			T3 = _mm256_madd_epi16(T3, coeff1);

			mVal1 = _mm256_add_epi32(T0, T2);
			mVal2 = _mm256_add_epi32(T1, T3);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal1 = _mm256_packus_epi32(mVal1, mVal2);

			mVal1 = _mm256_min_epu16(mVal1, max_val1);
			_mm256_storeu_si256((__m256i*)dst, mVal1);

			T0 = _mm256_unpacklo_epi16(S1, S2);
			T1 = _mm256_unpackhi_epi16(S1, S2);
			T2 = _mm256_unpacklo_epi16(S3, S4);
			T3 = _mm256_unpackhi_epi16(S3, S4);

			T0 = _mm256_madd_epi16(T0, coeff0);
			T1 = _mm256_madd_epi16(T1, coeff0);
			T2 = _mm256_madd_epi16(T2, coeff1);
			T3 = _mm256_madd_epi16(T3, coeff1);

			mVal1 = _mm256_add_epi32(T0, T2);
			mVal2 = _mm256_add_epi32(T1, T3);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal1 = _mm256_packus_epi32(mVal1, mVal2);

			mVal1 = _mm256_min_epu16(mVal1, max_val1);
			_mm256_storeu_si256((__m256i*)(dst + i_dst), mVal1);

			S0 = _mm256_loadu_si256((__m256i*)(src1));
			S1 = _mm256_loadu_si256((__m256i*)(src1 + i_src));
			S2 = _mm256_loadu_si256((__m256i*)(src1 + i_src2));
			S3 = _mm256_loadu_si256((__m256i*)(src1 + i_src3));
			S4 = _mm256_loadu_si256((__m256i*)(src1 + i_src4));

			T0 = _mm256_unpacklo_epi16(S0, S1);
			T1 = _mm256_unpackhi_epi16(S0, S1);
			T2 = _mm256_unpacklo_epi16(S2, S3);
			T3 = _mm256_unpackhi_epi16(S2, S3);

			T0 = _mm256_madd_epi16(T0, coeff0);
			T1 = _mm256_madd_epi16(T1, coeff0);
			T2 = _mm256_madd_epi16(T2, coeff1);
			T3 = _mm256_madd_epi16(T3, coeff1);

			mVal1 = _mm256_add_epi32(T0, T2);
			mVal2 = _mm256_add_epi32(T1, T3);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal1 = _mm256_packus_epi32(mVal1, mVal2);

			mVal1 = _mm256_min_epu16(mVal1, max_val1);
			_mm256_storeu_si256((__m256i*)(dst + 16), mVal1);

			T0 = _mm256_unpacklo_epi16(S1, S2);
			T1 = _mm256_unpackhi_epi16(S1, S2);
			T2 = _mm256_unpacklo_epi16(S3, S4);
			T3 = _mm256_unpackhi_epi16(S3, S4);

			T0 = _mm256_madd_epi16(T0, coeff0);
			T1 = _mm256_madd_epi16(T1, coeff0);
			T2 = _mm256_madd_epi16(T2, coeff1);
			T3 = _mm256_madd_epi16(T3, coeff1);

			mVal1 = _mm256_add_epi32(T0, T2);
			mVal2 = _mm256_add_epi32(T1, T3);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift);
			mVal2 = _mm256_srai_epi32(mVal2, shift);
			mVal1 = _mm256_packus_epi32(mVal1, mVal2);

			mVal1 = _mm256_min_epu16(mVal1, max_val1);
			_mm256_storeu_si256((__m256i*)(dst + i_dst + 16), mVal1);

			src += i_src2;
			dst += 2 * i_dst;
		}
	}
}

void com_if_filter_ver_4_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_4_w24_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
	com_if_filter_ver_4_w16_sse256_10bit(src, i_src, dst, i_dst, 16, height, coeff, max_val);
	com_if_filter_ver_4_sse128_10bit(src + 16, i_src, dst + 16, i_dst, 8, height, coeff, max_val);
}

void com_if_filter_hor_4_w8_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
	int row, col;
	const int offset = 32;
	const int shift = 6;

	__m128i mCoef1 = _mm_set1_epi32(*(i32s_t*)coeff);
	__m256i mCoef = _mm256_cvtepi8_epi16(mCoef1);
	__m256i mSwitch1 = _mm256_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 2, 3, 4, 5, 6, 7, 8, 9);
	__m256i mSwitch2 = _mm256_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 6, 7, 8, 9, 10, 11, 12, 13, 4, 5, 6, 7, 8, 9, 10, 11, 6, 7, 8, 9, 10, 11, 12, 13);
	__m256i mAddOffset = _mm256_set1_epi32((i16s_t)offset);
	__m256i T0, T1, S, S0, sum;
	__m256i max_val1 = _mm256_set1_epi16((pel_t)max_val);

    src -= 1;

	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col += 8) {
			S = _mm256_loadu_si256((__m256i*)(src + col));
			S0 = _mm256_permute4x64_epi64(S, 0x94);
			T0 = _mm256_madd_epi16(_mm256_shuffle_epi8(S0, mSwitch1), mCoef);
			T1 = _mm256_madd_epi16(_mm256_shuffle_epi8(S0, mSwitch2), mCoef);
			sum = _mm256_hadd_epi32(T0, T1);

			sum = _mm256_add_epi32(sum, mAddOffset);
			sum = _mm256_srai_epi32(sum, shift);
			sum = _mm256_packus_epi32(sum, sum);
			sum = _mm256_permute4x64_epi64(sum, 0xd8);

			sum = _mm256_min_epu16(sum, max_val1);
			_mm_storeu_si128((__m128i*)(dst + col), _mm256_castsi256_si128(sum));
		}
		src += i_src;
		dst += i_dst;
	}
}

void com_if_filter_hor_ver_4_w16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
	ALIGNED_32(i16s_t tmp_res[(32 + 3) * 32]);
	i16s_t *tmp = tmp_res;
	const int i_tmp = 32;
	const int i_tmp2 = 64;
	const int i_tmp3 = 96;
	__m256i mask8 = _mm256_setr_epi32(-1, -1, -1, -1, 0, 0, 0, 0);
	int row, col;
	int shift1, shift2;
    int add1, add2;

	__m128i mCoef1 = _mm_set1_epi32(*(i32s_t*)coef_x);
	__m256i mCoef = _mm256_cvtepi8_epi16(mCoef1);
	__m256i mSwitch1 = _mm256_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 2, 3, 4, 5, 6, 7, 8, 9);
	__m256i mSwitch2 = _mm256_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 6, 7, 8, 9, 10, 11, 12, 13, 4, 5, 6, 7, 8, 9, 10, 11, 6, 7, 8, 9, 10, 11, 12, 13);
	__m256i T0, T1, S, S0, sum;
    __m256i mAddOffset;
	__m256i max_val1 = _mm256_set1_epi16((pel_t)max_val);
    __m256i mVal1, mVal2, mVal;
    __m256i T2, T3, S1, S2, S3;
    __m128i mCoefy11, mCoefy22;
    __m256i mCoefy1, mCoefy2;

    if (max_val == 255) { // 8 bit_depth
        shift1 = 0;
        shift2 = 12;
    }
    else { // 10 bit_depth
        shift1 = 2;
        shift2 = 10;
    }

    add1 = (1 << (shift1)) >> 1;
    add2 = 1 << (shift2 - 1);

    mAddOffset = _mm256_set1_epi32(add1);
	//HOR
	src = src - i_src - 1;

	for (row = -1; row < height + 2; row++) {
		for (col = 0; col < width; col += 8) {
			S = _mm256_loadu_si256((__m256i*)(src + col));
			S0 = _mm256_permute4x64_epi64(S, 0x94);
			T0 = _mm256_madd_epi16(_mm256_shuffle_epi8(S0, mSwitch1), mCoef);
			T1 = _mm256_madd_epi16(_mm256_shuffle_epi8(S0, mSwitch2), mCoef);
			sum = _mm256_hadd_epi32(T0, T1);

			sum = _mm256_add_epi32(sum, mAddOffset);
			sum = _mm256_srai_epi32(sum, shift1);
			sum = _mm256_packus_epi32(sum, sum);
			sum = _mm256_permute4x64_epi64(sum, 0xd8);

			_mm256_maskstore_epi32((i32s_t*)(tmp + col), mask8, sum);
		}
		src += i_src;
		tmp += i_tmp;
	}

	// VER
	tmp = tmp_res;
	mAddOffset = _mm256_set1_epi32(add2);

	mCoefy11 = _mm_set1_epi16(*(i16s_t*)coef_y);
	mCoefy22 = _mm_set1_epi16(*(i16s_t*)(coef_y + 2));
	mCoefy1 = _mm256_cvtepi8_epi16(mCoefy11);
	mCoefy2 = _mm256_cvtepi8_epi16(mCoefy22);

	for (row = 0; row < height; row++)
	{
		for (col = 0; col < width; col += 16)
		{
			S0 = _mm256_load_si256((__m256i*)(tmp + col));
			S1 = _mm256_load_si256((__m256i*)(tmp + col + i_tmp));
			S2 = _mm256_load_si256((__m256i*)(tmp + col + i_tmp2));
			S3 = _mm256_load_si256((__m256i*)(tmp + col + i_tmp3));

			T0 = _mm256_unpacklo_epi16(S0, S1);
			T1 = _mm256_unpacklo_epi16(S2, S3);
			T2 = _mm256_unpackhi_epi16(S0, S1);
			T3 = _mm256_unpackhi_epi16(S2, S3);

			T0 = _mm256_madd_epi16(T0, mCoefy1);
			T1 = _mm256_madd_epi16(T1, mCoefy2);
			T2 = _mm256_madd_epi16(T2, mCoefy1);
			T3 = _mm256_madd_epi16(T3, mCoefy2);

			mVal1 = _mm256_add_epi32(T0, T1);
			mVal2 = _mm256_add_epi32(T2, T3);

			mVal1 = _mm256_add_epi32(mVal1, mAddOffset);
			mVal2 = _mm256_add_epi32(mVal2, mAddOffset);
			mVal1 = _mm256_srai_epi32(mVal1, shift2);
			mVal2 = _mm256_srai_epi32(mVal2, shift2);

			mVal = _mm256_packus_epi32(mVal1, mVal2);
			mVal = _mm256_min_epu16(mVal, max_val1);
			_mm256_storeu_si256((__m256i*)(dst + col), mVal);
		}
		tmp += i_tmp;
		dst += i_dst;
	}
}

void com_if_filter_hor_ver_4_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff_h, const char_t *coeff_v, int max_val);
void com_if_filter_hor_ver_4_w24_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
	com_if_filter_hor_ver_4_w16_sse256_10bit(src, i_src, dst, i_dst, 16, height, coef_x, coef_y, max_val);
	com_if_filter_hor_ver_4_sse128_10bit(src + 16, i_src, dst + 16, i_dst, 8, height, coef_x, coef_y, max_val);
}

void com_if_filter_hor_8_w16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	int i, j;
	__m256i max_val1 = _mm256_set1_epi16((pel_t)max_val);

	__m256i T0, T1, T2, T3, T4, T5, T6, T7;
	__m256i M0, M1, M2, M3, M4, M5, M6, M7;

	__m256i offset = _mm256_set1_epi32(32);

	i32s_t * coef = (i32s_t*)coeff;
	__m128i mCoef0 = _mm_setr_epi32(coef[0], coef[1], coef[0], coef[1]);
	__m256i mCoef = _mm256_cvtepi8_epi16(mCoef0);

	src -= 3;

	for (j = 0; j < height; j++) {
		const pel_t *p = src;
		for (i = 0; i < width; i += 16) {

			T0 = _mm256_loadu_si256((__m256i*)p++);
			T1 = _mm256_loadu_si256((__m256i*)p++);
			T2 = _mm256_loadu_si256((__m256i*)p++);
			T3 = _mm256_loadu_si256((__m256i*)p++);
			T4 = _mm256_loadu_si256((__m256i*)p++);
			T5 = _mm256_loadu_si256((__m256i*)p++);
			T6 = _mm256_loadu_si256((__m256i*)p++);
			T7 = _mm256_loadu_si256((__m256i*)p++);

			M0 = _mm256_madd_epi16(T0, mCoef);
			M1 = _mm256_madd_epi16(T1, mCoef);
			M2 = _mm256_madd_epi16(T2, mCoef);
			M3 = _mm256_madd_epi16(T3, mCoef);
			M4 = _mm256_madd_epi16(T4, mCoef);
			M5 = _mm256_madd_epi16(T5, mCoef);
			M6 = _mm256_madd_epi16(T6, mCoef);
			M7 = _mm256_madd_epi16(T7, mCoef);

			M0 = _mm256_hadd_epi32(M0, M1);
			M1 = _mm256_hadd_epi32(M2, M3);
			M2 = _mm256_hadd_epi32(M4, M5);
			M3 = _mm256_hadd_epi32(M6, M7);

			M0 = _mm256_hadd_epi32(M0, M1);
			M1 = _mm256_hadd_epi32(M2, M3);

			M2 = _mm256_add_epi32(M0, offset);
			M3 = _mm256_add_epi32(M1, offset);
			M2 = _mm256_srai_epi32(M2, 6);
			M3 = _mm256_srai_epi32(M3, 6);
			M2 = _mm256_packus_epi32(M2, M3);
			M2 = _mm256_min_epu16(M2, max_val1);
			_mm256_storeu_si256((__m256i*)(dst + i), M2);

			p += 8;
		}
		dst += i_dst;
		src += i_src;
	}

}

void com_if_filter_hor_8_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_hor_8_w24_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
	com_if_filter_hor_8_w16_sse256_10bit(src, i_src, dst, i_dst, 16, height, coeff, max_val);
	com_if_filter_hor_8_sse128_10bit(src + 16, i_src, dst + 16, i_dst, 8, height, coeff, max_val);
}


void com_if_filter_ver_8_w16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
	int i, j;
	__m256i max_val1 = _mm256_set1_epi16((pel_t)max_val);
	__m256i mAddOffset = _mm256_set1_epi32(32);
	__m256i T0, T1, T2, T3, T4, T5, T6, T7;
	__m256i M0, M1, M2, M3, M4, M5, M6, M7;
	__m256i N0, N1, N2, N3, N4, N5, N6, N7;

	src -= 3 * i_src;

	if (coeff[1] == coeff[6]){
		__m128i coeff0 = _mm_set1_epi16(*(i16s_t*)(coeff    ));
		__m128i coeff1 = _mm_set1_epi16(*(i16s_t*)(coeff + 2));
		__m256i coeff10 = _mm256_cvtepi8_epi16(coeff0);
		__m256i coeff11 = _mm256_cvtepi8_epi16(coeff1);

		for (j = 0; j < height; j++) {
			const pel_t *p = src;
			for (i = 0; i < width; i += 16) {
				T0 = _mm256_loadu_si256((__m256i*)(p));
				T1 = _mm256_loadu_si256((__m256i*)(p + i_src));
				T2 = _mm256_loadu_si256((__m256i*)(p + 2 * i_src));
				T3 = _mm256_loadu_si256((__m256i*)(p + 3 * i_src));
				T4 = _mm256_loadu_si256((__m256i*)(p + 4 * i_src));
				T5 = _mm256_loadu_si256((__m256i*)(p + 5 * i_src));
				T6 = _mm256_loadu_si256((__m256i*)(p + 6 * i_src));
				T7 = _mm256_loadu_si256((__m256i*)(p + 7 * i_src));

				T0 = _mm256_add_epi16(T0, T7);
				T1 = _mm256_add_epi16(T1, T6);
				T2 = _mm256_add_epi16(T2, T5);
				T3 = _mm256_add_epi16(T3, T4);

				M0 = _mm256_unpacklo_epi16(T0, T1);
				M1 = _mm256_unpacklo_epi16(T2, T3);
				M2 = _mm256_unpackhi_epi16(T0, T1);
				M3 = _mm256_unpackhi_epi16(T2, T3);

				N0 = _mm256_madd_epi16(M0, coeff10);
				N1 = _mm256_madd_epi16(M1, coeff11);
				N2 = _mm256_madd_epi16(M2, coeff10);
				N3 = _mm256_madd_epi16(M3, coeff11);

				N0 = _mm256_add_epi32(N0, N1);
				N1 = _mm256_add_epi32(N2, N3);

				N0 = _mm256_add_epi32(N0, mAddOffset);
				N1 = _mm256_add_epi32(N1, mAddOffset);
				N0 = _mm256_srai_epi32(N0, 6);
				N1 = _mm256_srai_epi32(N1, 6);
				N0 = _mm256_packus_epi32(N0, N1);
				N0 = _mm256_min_epu16(N0, max_val1);
				_mm256_storeu_si256((__m256i*)(dst + i), N0);

				p += 16;
			}
			dst += i_dst;
			src += i_src;
		}
	}
	else{
		__m128i coeff0 = _mm_set1_epi16(*(i16s_t*)(coeff));
		__m128i coeff1 = _mm_set1_epi16(*(i16s_t*)(coeff + 2));
		__m128i coeff2 = _mm_set1_epi16(*(i16s_t*)(coeff + 4));
		__m128i coeff3 = _mm_set1_epi16(*(i16s_t*)(coeff + 6));
		__m256i coeff00 = _mm256_cvtepi8_epi16(coeff0);
		__m256i coeff01 = _mm256_cvtepi8_epi16(coeff1);
		__m256i coeff02 = _mm256_cvtepi8_epi16(coeff2);
		__m256i coeff03 = _mm256_cvtepi8_epi16(coeff3);

		for (j = 0; j < height; j++) {
			const pel_t *p = src;
			for (i = 0; i < width; i += 16) {
				T0 = _mm256_loadu_si256((__m256i*)(p));
				T1 = _mm256_loadu_si256((__m256i*)(p + i_src));
				T2 = _mm256_loadu_si256((__m256i*)(p + 2 * i_src));
				T3 = _mm256_loadu_si256((__m256i*)(p + 3 * i_src));
				T4 = _mm256_loadu_si256((__m256i*)(p + 4 * i_src));
				T5 = _mm256_loadu_si256((__m256i*)(p + 5 * i_src));
				T6 = _mm256_loadu_si256((__m256i*)(p + 6 * i_src));
				T7 = _mm256_loadu_si256((__m256i*)(p + 7 * i_src));

				M0 = _mm256_unpacklo_epi16(T0, T1);
				M1 = _mm256_unpacklo_epi16(T2, T3);
				M2 = _mm256_unpacklo_epi16(T4, T5);
				M3 = _mm256_unpacklo_epi16(T6, T7);
				M4 = _mm256_unpackhi_epi16(T0, T1);
				M5 = _mm256_unpackhi_epi16(T2, T3);
				M6 = _mm256_unpackhi_epi16(T4, T5);
				M7 = _mm256_unpackhi_epi16(T6, T7);

				N0 = _mm256_madd_epi16(M0, coeff00);
				N1 = _mm256_madd_epi16(M1, coeff01);
				N2 = _mm256_madd_epi16(M2, coeff02);
				N3 = _mm256_madd_epi16(M3, coeff03);
				N4 = _mm256_madd_epi16(M4, coeff00);
				N5 = _mm256_madd_epi16(M5, coeff01);
				N6 = _mm256_madd_epi16(M6, coeff02);
				N7 = _mm256_madd_epi16(M7, coeff03);

				N0 = _mm256_add_epi32(N0, N1);
				N1 = _mm256_add_epi32(N2, N3);
				N2 = _mm256_add_epi32(N4, N5);
				N3 = _mm256_add_epi32(N6, N7);

				N0 = _mm256_add_epi32(N0, N1);
				N1 = _mm256_add_epi32(N2, N3);

				N0 = _mm256_add_epi32(N0, mAddOffset);
				N1 = _mm256_add_epi32(N1, mAddOffset);
				N0 = _mm256_srai_epi32(N0, 6);
				N1 = _mm256_srai_epi32(N1, 6);
				N0 = _mm256_packus_epi32(N0, N1);
				N0 = _mm256_min_epu16(N0, max_val1);
				_mm256_storeu_si256((__m256i*)(dst + i), N0);

				p += 16;
			}
			dst += i_dst;
			src += i_src;
		}
	}
}

void com_if_filter_ver_8_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_8_w24_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
	com_if_filter_ver_8_w16_sse256_10bit(src, i_src, dst, i_dst, 16, height, coeff, max_val);
	com_if_filter_ver_8_sse128_10bit(src + 16, i_src, dst + 16, i_dst, 8, height, coeff, max_val);
}

void com_if_filter_hor_ver_8_w16_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
	int add1, shift1;
	int add2, shift2;
    __m128i mCoef0;
    __m256i mCoef, offset;
	__m256i T0, T1, T2, T3, T4, T5, T6, T7;
	__m256i M0, M1, M2, M3, M4, M5, M6, M7;
	__m256i N0, N1, N2, N3, N4, N5, N6, N7;
	int i, j;
	__m256i max_val1 = _mm256_set1_epi16((pel_t)max_val);
	ALIGNED_16(i16s_t tmp_res[(64 + 7) * 64]);
	i16s_t *tmp;
	int i_tmp = 64;
    i32s_t * coef;
    __m128i coeff0, coeff1, coeff2, coeff3;
    __m256i coeff00, coeff01, coeff02, coeff03;

    if (max_val == 255) { // 8 bit_depth
        shift1 = 0;
        shift2 = 12;
    } else { // 10 bit_depth
        shift1 = 2;
        shift2 = 10;
    }

	add1 = (1 << (shift1)) >> 1;
	add2 = 1 << (shift2 - 1);

	src += -3 * i_src - 3;
	tmp = tmp_res;
	coef = (i32s_t*)coef_x;
	mCoef0 = _mm_setr_epi32(coef[0], coef[1], coef[0], coef[1]);
	mCoef = _mm256_cvtepi8_epi16(mCoef0);
	offset = _mm256_set1_epi32(add1);

	for (j = -3; j < height + 4; j++) {
		const pel_t *p = src;
		for (i = 0; i < width; i += 16) {

			T0 = _mm256_loadu_si256((__m256i*)p++);
			T1 = _mm256_loadu_si256((__m256i*)p++);
			T2 = _mm256_loadu_si256((__m256i*)p++);
			T3 = _mm256_loadu_si256((__m256i*)p++);
			T4 = _mm256_loadu_si256((__m256i*)p++);
			T5 = _mm256_loadu_si256((__m256i*)p++);
			T6 = _mm256_loadu_si256((__m256i*)p++);
			T7 = _mm256_loadu_si256((__m256i*)p++);

			M0 = _mm256_madd_epi16(T0, mCoef);
			M1 = _mm256_madd_epi16(T1, mCoef);
			M2 = _mm256_madd_epi16(T2, mCoef);
			M3 = _mm256_madd_epi16(T3, mCoef);
			M4 = _mm256_madd_epi16(T4, mCoef);
			M5 = _mm256_madd_epi16(T5, mCoef);
			M6 = _mm256_madd_epi16(T6, mCoef);
			M7 = _mm256_madd_epi16(T7, mCoef);

			M0 = _mm256_hadd_epi32(M0, M1);
			M1 = _mm256_hadd_epi32(M2, M3);
			M2 = _mm256_hadd_epi32(M4, M5);
			M3 = _mm256_hadd_epi32(M6, M7);

			M0 = _mm256_hadd_epi32(M0, M1);
			M1 = _mm256_hadd_epi32(M2, M3);

			M2 = _mm256_add_epi32(M0, offset);
			M3 = _mm256_add_epi32(M1, offset);
			M2 = _mm256_srai_epi32(M2, shift1);
			M3 = _mm256_srai_epi32(M3, shift1);
			M2 = _mm256_packs_epi32(M2, M3);
			_mm256_storeu_si256((__m256i*)(tmp + i), M2);

			p += 8;
		}
		tmp += i_tmp;
		src += i_src;
	}

	offset = _mm256_set1_epi32(add2);
	tmp = tmp_res;

	coeff0 = _mm_set1_epi16(*(i16s_t*)(coef_y));
	coeff1 = _mm_set1_epi16(*(i16s_t*)(coef_y + 2));
	coeff2 = _mm_set1_epi16(*(i16s_t*)(coef_y + 4));
	coeff3 = _mm_set1_epi16(*(i16s_t*)(coef_y + 6));
	coeff00 = _mm256_cvtepi8_epi16(coeff0);
	coeff01 = _mm256_cvtepi8_epi16(coeff1);
	coeff02 = _mm256_cvtepi8_epi16(coeff2);
	coeff03 = _mm256_cvtepi8_epi16(coeff3);

	for (j = 0; j < height; j++) {
		const pel_t *p = (pel_t*)tmp;
		for (i = 0; i < width; i += 16) {
			T0 = _mm256_loadu_si256((__m256i*)(p));
			T1 = _mm256_loadu_si256((__m256i*)(p + i_tmp));
			T2 = _mm256_loadu_si256((__m256i*)(p + 2 * i_tmp));
			T3 = _mm256_loadu_si256((__m256i*)(p + 3 * i_tmp));
			T4 = _mm256_loadu_si256((__m256i*)(p + 4 * i_tmp));
			T5 = _mm256_loadu_si256((__m256i*)(p + 5 * i_tmp));
			T6 = _mm256_loadu_si256((__m256i*)(p + 6 * i_tmp));
			T7 = _mm256_loadu_si256((__m256i*)(p + 7 * i_tmp));

			M0 = _mm256_unpacklo_epi16(T0, T1);
			M1 = _mm256_unpacklo_epi16(T2, T3);
			M2 = _mm256_unpacklo_epi16(T4, T5);
			M3 = _mm256_unpacklo_epi16(T6, T7);
			M4 = _mm256_unpackhi_epi16(T0, T1);
			M5 = _mm256_unpackhi_epi16(T2, T3);
			M6 = _mm256_unpackhi_epi16(T4, T5);
			M7 = _mm256_unpackhi_epi16(T6, T7);

			N0 = _mm256_madd_epi16(M0, coeff00);
			N1 = _mm256_madd_epi16(M1, coeff01);
			N2 = _mm256_madd_epi16(M2, coeff02);
			N3 = _mm256_madd_epi16(M3, coeff03);
			N4 = _mm256_madd_epi16(M4, coeff00);
			N5 = _mm256_madd_epi16(M5, coeff01);
			N6 = _mm256_madd_epi16(M6, coeff02);
			N7 = _mm256_madd_epi16(M7, coeff03);

			N0 = _mm256_add_epi32(N0, N1);
			N1 = _mm256_add_epi32(N2, N3);
			N2 = _mm256_add_epi32(N4, N5);
			N3 = _mm256_add_epi32(N6, N7);

			N0 = _mm256_add_epi32(N0, N1);
			N1 = _mm256_add_epi32(N2, N3);

			N0 = _mm256_add_epi32(N0, offset);
			N1 = _mm256_add_epi32(N1, offset);
			N0 = _mm256_srai_epi32(N0, shift2);
			N1 = _mm256_srai_epi32(N1, shift2);
			N0 = _mm256_packus_epi32(N0, N1);
			N0 = _mm256_min_epu16(N0, max_val1);
			_mm256_storeu_si256((__m256i*)(dst + i), N0);

			p += 16;
		}
		dst += i_dst;
		tmp += i_tmp;
	}

}

void com_if_filter_hor_ver_8_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);
void com_if_filter_hor_ver_8_w24_sse256_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
	com_if_filter_hor_ver_8_w16_sse256_10bit(src, i_src, dst, i_dst, 16, height, coef_x, coef_y, max_val);
	com_if_filter_hor_ver_8_sse128_10bit(src + 16, i_src, dst + 16, i_dst, 8, height, coef_x, coef_y, max_val);
}


