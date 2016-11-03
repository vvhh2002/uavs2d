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

#include "intrinsic.h"


void com_if_filter_cpy_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    int col;

    while (height--) {
        for (col = 0; col < width; col += 16) {
            CP128(dst + col, src + col);
        }
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_cpy_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    int col;
 
    while (height--) {
        for (col = 0; col < width; col += 8) {
            CP128(dst + col, src + col);
        }
        src += i_src;
        dst += i_dst;
    }
}


void com_if_filter_cpy2_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
     while (height--) {
        CP16(dst, src);
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_cpy4_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
     while (height--) {
        CP32(dst, src);
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_cpy6_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
     while (height--) {
        CP32(dst, src);
        CP16(dst + 4, src + 4);
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_cpy8_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    while (height--) {
        CP64(dst, src);
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_cpy12_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    while (height--) {
        CP64(dst, src);
        CP32(dst + 8, src + 8);
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_cpy16_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    while (height--) {
        CP128(dst, src);
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_cpy6_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    while (height--) {
        CP64(dst, src);
        CP32(dst + 4, src + 4);
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_cpy12_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    while (height--) {
        CP128(dst, src);
        CP64(dst + 8, src + 8);
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_cpy16_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height)
{
    while (height--) {
        CP128(dst, src);
        CP128(dst + 8, src + 8);
        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_hor_4_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
    int row, col;
    const int offset = 32;
    const int shift = 6;

    __m128i mAddOffset = _mm_set1_epi16(offset);

    __m128i mSwitch1 = _mm_setr_epi8(0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6);
    __m128i mSwitch2 = _mm_setr_epi8(4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10);
    __m128i mCoef = _mm_set1_epi32(*(i32s_t*)coeff);
    __m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask[(width & 7) - 1]));

    src -= 1;

    for (row = 0; row < height; row++) {
        __m128i mT20, mT40, mSum, mVal;

        for (col = 0; col < width - 7; col += 8) {

            __m128i mSrc = _mm_loadu_si128((__m128i*)(src + col));
            
            mT20 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoef);
            mT40 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoef);
       
            mSum = _mm_hadd_epi16(mT20, mT40);
            mVal = _mm_add_epi16(mSum, mAddOffset);

            mVal = _mm_srai_epi16(mVal, shift);
			mVal = _mm_packus_epi16(mVal, mVal);

			_mm_storel_epi64((__m128i*)&dst[col], mVal);
        }

        if (col < width) { 

            __m128i mSrc = _mm_loadu_si128((__m128i*)(src + col));

            __m128i mT20 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoef);
            __m128i mT40 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoef);

            __m128i mSum = _mm_hadd_epi16(mT20, mT40);
            __m128i mVal = _mm_add_epi16(mSum, mAddOffset);

            mVal = _mm_srai_epi16(mVal, shift);
			mVal = _mm_packus_epi16(mVal, mVal);

            _mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
        }

        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_hor_8_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
    int row, col=0;
    const int offset = 32;
    const int shift = 6;

    __m128i mAddOffset = _mm_set1_epi16(offset);

    __m128i mSwitch1 = _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8);
    __m128i mSwitch2 = _mm_setr_epi8(2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10);
    __m128i mSwitch3 = _mm_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12);
    __m128i mSwitch4 = _mm_setr_epi8(6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14);

	__m128i mCoef = _mm_loadl_epi64((__m128i*)coeff);

    __m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask[(width & 7) - 1]));
	mCoef = _mm_unpacklo_epi64(mCoef, mCoef);

    src -= 3;
    for (row = 0; row < height; row++) {

        __m128i T20, T40, T60, T80, s1, s2, sum, val;

        for (col = 0; col < width - 7; col += 8) {

			__m128i srcCoeff = _mm_loadu_si128((__m128i*)(src + col));

            T20 = _mm_maddubs_epi16(_mm_shuffle_epi8(srcCoeff, mSwitch1), mCoef);
            T40 = _mm_maddubs_epi16(_mm_shuffle_epi8(srcCoeff, mSwitch2), mCoef);
            T60 = _mm_maddubs_epi16(_mm_shuffle_epi8(srcCoeff, mSwitch3), mCoef);
            T80 = _mm_maddubs_epi16(_mm_shuffle_epi8(srcCoeff, mSwitch4), mCoef);

            s1 = _mm_hadd_epi16(T20, T40);
            s2 = _mm_hadd_epi16(T60, T80);
            sum = _mm_hadd_epi16(s1, s2);

            val = _mm_add_epi16(sum, mAddOffset);

            val = _mm_srai_epi16(val, shift);
			val = _mm_packus_epi16(val, val);

			_mm_storel_epi64((__m128i*)&dst[col], val);
        }

        if (col < width) {  

			__m128i srcCoeff = _mm_loadu_si128((__m128i*)(src + col));

            __m128i T20 = _mm_maddubs_epi16(_mm_shuffle_epi8(srcCoeff, mSwitch1), mCoef);
            __m128i T40 = _mm_maddubs_epi16(_mm_shuffle_epi8(srcCoeff, mSwitch2), mCoef);
            __m128i T60 = _mm_maddubs_epi16(_mm_shuffle_epi8(srcCoeff, mSwitch3), mCoef);
            __m128i T80 = _mm_maddubs_epi16(_mm_shuffle_epi8(srcCoeff, mSwitch4), mCoef);

            __m128i s1 = _mm_hadd_epi16(T20, T40);
            __m128i s2 = _mm_hadd_epi16(T60, T80);
            __m128i sum = _mm_hadd_epi16(s1, s2);

            __m128i val = _mm_add_epi16(sum, mAddOffset);

            val = _mm_srai_epi16(val, shift);
			val = _mm_packus_epi16(val, val);

            _mm_maskmoveu_si128(val, mask, (char_t*)&dst[col]);
        }

        src += i_src;
        dst += i_dst;
    }
}

void com_if_filter_ver_4_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
    int row, col;
    const int offset = 32;
    const int shift = 6;
    int bsym = (coeff[1] == coeff[2]);
    __m128i mAddOffset = _mm_set1_epi16(offset);

    pel_t const *p;

    __m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask[(width & 7) - 1]));
    
    src -= i_src; 

    if (bsym) {
        __m128i coeff0 = _mm_set1_epi8(coeff[0]);
        __m128i coeff1 = _mm_set1_epi8(coeff[1]);
        __m128i mVal;

        for (row = 0; row < height; row++) {
            p = src;
            for (col = 0; col < width - 7; col += 8) {
                __m128i T00 = _mm_loadu_si128((__m128i*)(p));
                __m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
                __m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
                __m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

				T00 = _mm_unpacklo_epi8(T00, T30);
				T10 = _mm_unpacklo_epi8(T10, T20);

				T00 = _mm_maddubs_epi16(T00, coeff0);
				T10 = _mm_maddubs_epi16(T10, coeff1);

                mVal = _mm_add_epi16(T00, T10);

                mVal = _mm_add_epi16(mVal, mAddOffset);
				mVal = _mm_srai_epi16(mVal, shift);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_storel_epi64((__m128i*)&dst[col], mVal);

                p += 8;
            }

            if (col < width) { // store either 1, 2, 3, 4, 5, 6, or 7 8-bit results in dst
                __m128i T00 = _mm_loadu_si128((__m128i*)(p));
                __m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
                __m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
                __m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

                T00 = _mm_unpacklo_epi8(T00, T30);
				T10 = _mm_unpacklo_epi8(T10, T20);

				T00 = _mm_maddubs_epi16(T00, coeff0);
				T10 = _mm_maddubs_epi16(T10, coeff1);

                mVal = _mm_add_epi16(T00, T10);

                mVal = _mm_add_epi16(mVal, mAddOffset);
				mVal = _mm_srai_epi16(mVal, shift);
				mVal = _mm_packus_epi16(mVal, mVal);

                _mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
            }

            src += i_src;
            dst += i_dst;
        }
    } else {
		__m128i coeff0 = _mm_set1_epi16(*(i32s_t*)coeff);
		__m128i coeff1 = _mm_set1_epi16(*(i32s_t*)(coeff + 2));
		__m128i mVal;
        for (row = 0; row < height; row++) {
            p = src;
            for (col = 0; col < width - 7; col += 8) {
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

				T00 = _mm_unpacklo_epi8(T00, T10);
				T10 = _mm_unpacklo_epi8(T20, T30);

				T00 = _mm_maddubs_epi16(T00, coeff0);
				T10 = _mm_maddubs_epi16(T10, coeff1);

				mVal = _mm_add_epi16(T00, T10);

				mVal = _mm_add_epi16(mVal, mAddOffset);
				mVal = _mm_srai_epi16(mVal, shift);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_storel_epi64((__m128i*)&dst[col], mVal);

                p += 8;
            }

            if (col < width) { // store either 1, 2, 3, 4, 5, 6, or 7 8-bit results in dst
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

				T00 = _mm_unpacklo_epi8(T00, T10);
				T10 = _mm_unpacklo_epi8(T20, T30);

				T00 = _mm_maddubs_epi16(T00, coeff0);
				T10 = _mm_maddubs_epi16(T10, coeff1);

				mVal = _mm_add_epi16(T00, T10);

				mVal = _mm_add_epi16(mVal, mAddOffset);
				mVal = _mm_srai_epi16(mVal, shift);
				mVal = _mm_packus_epi16(mVal, mVal);

                _mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
            }

            src += i_src;
            dst += i_dst;
        }
    }
}

void com_if_filter_ver_8_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff, int max_val)
{
    int row, col;
    const int offset = 32;
    const int shift = 6;
    int bsym = (coeff[1] == coeff[6]);

    __m128i mAddOffset = _mm_set1_epi16(offset);

    pel_t const *p;

    __m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask[(width & 7) - 1]));

    src -= 3 * i_src; 

    if (bsym) {
		__m128i coeff0 = _mm_set1_epi8(coeff[0]);
        __m128i coeff1 = _mm_set1_epi8(coeff[1]);
        __m128i coeff2 = _mm_set1_epi8(coeff[2]);
        __m128i coeff3 = _mm_set1_epi8(coeff[3]);
        __m128i mVal;

        for (row = 0; row < height; row++) {
            p = src;
            for (col = 0; col < width - 7; col += 8) {
                __m128i T00 = _mm_loadu_si128((__m128i*)(p));
                __m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
                __m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
                __m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));
                __m128i T40 = _mm_loadu_si128((__m128i*)(p + 4 * i_src));
                __m128i T50 = _mm_loadu_si128((__m128i*)(p + 5 * i_src));
                __m128i T60 = _mm_loadu_si128((__m128i*)(p + 6 * i_src));
                __m128i T70 = _mm_loadu_si128((__m128i*)(p + 7 * i_src));

				T00 = _mm_unpacklo_epi8(T00, T70);
				T10 = _mm_unpacklo_epi8(T10, T60);
				T20 = _mm_unpacklo_epi8(T20, T50);
				T30 = _mm_unpacklo_epi8(T30, T40);

				T00 = _mm_maddubs_epi16(T00, coeff0);
				T10 = _mm_maddubs_epi16(T10, coeff1);
				T20 = _mm_maddubs_epi16(T20, coeff2);
				T30 = _mm_maddubs_epi16(T30, coeff3);

				mVal = _mm_add_epi16(T00, T10);
				mVal = _mm_add_epi16(mVal, T20);
				mVal = _mm_add_epi16(mVal, T30);

                mVal = _mm_add_epi16(mVal, mAddOffset);
                mVal = _mm_srai_epi16(mVal, shift);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_storel_epi64((__m128i*)&dst[col], mVal);

                p += 8;
            }

            if (col < width) { // store either 1, 2, 3, 4, 5, 6, or 7 8-bit results in dst
                __m128i T00 = _mm_loadu_si128((__m128i*)(p));
                __m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
                __m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
                __m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));
                __m128i T40 = _mm_loadu_si128((__m128i*)(p + 4 * i_src));
                __m128i T50 = _mm_loadu_si128((__m128i*)(p + 5 * i_src));
                __m128i T60 = _mm_loadu_si128((__m128i*)(p + 6 * i_src));
                __m128i T70 = _mm_loadu_si128((__m128i*)(p + 7 * i_src));

				T00 = _mm_unpacklo_epi8(T00, T70);
				T10 = _mm_unpacklo_epi8(T10, T60);
				T20 = _mm_unpacklo_epi8(T20, T50);
				T30 = _mm_unpacklo_epi8(T30, T40);

				T00 = _mm_maddubs_epi16(T00, coeff0);
				T10 = _mm_maddubs_epi16(T10, coeff1);
				T20 = _mm_maddubs_epi16(T20, coeff2);
				T30 = _mm_maddubs_epi16(T30, coeff3);

				mVal = _mm_add_epi16(T00, T10);
				mVal = _mm_add_epi16(mVal, T20);
				mVal = _mm_add_epi16(mVal, T30);

				mVal = _mm_add_epi16(mVal, mAddOffset);
				mVal = _mm_srai_epi16(mVal, shift);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
            }

            src += i_src;
            dst += i_dst;
        }
    } else {
		__m128i coeff0 = _mm_set1_epi16(*(i32s_t*)coeff);
		__m128i coeff1 = _mm_set1_epi16(*(i32s_t*)(coeff + 2));
		__m128i coeff2 = _mm_set1_epi16(*(i32s_t*)(coeff + 4));
		__m128i coeff3 = _mm_set1_epi16(*(i32s_t*)(coeff + 6));
		__m128i mVal;
        for (row = 0; row < height; row++) {
            p = src;
            for (col = 0; col < width - 7; col += 8) {
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));
				__m128i T40 = _mm_loadu_si128((__m128i*)(p + 4 * i_src));
				__m128i T50 = _mm_loadu_si128((__m128i*)(p + 5 * i_src));
				__m128i T60 = _mm_loadu_si128((__m128i*)(p + 6 * i_src));
				__m128i T70 = _mm_loadu_si128((__m128i*)(p + 7 * i_src));

				T00 = _mm_unpacklo_epi8(T00, T10);
				T10 = _mm_unpacklo_epi8(T20, T30);
				T20 = _mm_unpacklo_epi8(T40, T50);
				T30 = _mm_unpacklo_epi8(T60, T70);

				T00 = _mm_maddubs_epi16(T00, coeff0);
				T10 = _mm_maddubs_epi16(T10, coeff1);
				T20 = _mm_maddubs_epi16(T20, coeff2);
				T30 = _mm_maddubs_epi16(T30, coeff3);

				mVal = _mm_add_epi16(T00, T10);
				mVal = _mm_add_epi16(mVal, T20);
				mVal = _mm_add_epi16(mVal, T30);

				mVal = _mm_add_epi16(mVal, mAddOffset);
				mVal = _mm_srai_epi16(mVal, shift);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_storel_epi64((__m128i*)&dst[col], mVal);

                p += 8;
            }

            if (col < width) { // store either 1, 2, 3, 4, 5, 6, or 7 8-bit results in dst
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));
				__m128i T40 = _mm_loadu_si128((__m128i*)(p + 4 * i_src));
				__m128i T50 = _mm_loadu_si128((__m128i*)(p + 5 * i_src));
				__m128i T60 = _mm_loadu_si128((__m128i*)(p + 6 * i_src));
				__m128i T70 = _mm_loadu_si128((__m128i*)(p + 7 * i_src));

				T00 = _mm_unpacklo_epi8(T00, T10);
				T10 = _mm_unpacklo_epi8(T20, T30);
				T20 = _mm_unpacklo_epi8(T40, T50);
				T30 = _mm_unpacklo_epi8(T60, T70);

				T00 = _mm_maddubs_epi16(T00, coeff0);
				T10 = _mm_maddubs_epi16(T10, coeff1);
				T20 = _mm_maddubs_epi16(T20, coeff2);
				T30 = _mm_maddubs_epi16(T30, coeff3);

				mVal = _mm_add_epi16(T00, T10);
				mVal = _mm_add_epi16(mVal, T20);
				mVal = _mm_add_epi16(mVal, T30);

				mVal = _mm_add_epi16(mVal, mAddOffset);
				mVal = _mm_srai_epi16(mVal, shift);
				mVal = _mm_packus_epi16(mVal, mVal);

                _mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
            }

            src += i_src;
            dst += i_dst;
        }
    }
}

void com_if_filter_hor_ver_4_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
    int row, col;
    int shift;
    i16s_t const *p;
 
    int bsymy = (coef_y[1] == coef_y[6]);

    ALIGNED_16(i16s_t tmp_res[(32 + 3) * 32]);
    i16s_t *tmp = tmp_res;
    const int i_tmp = 32;

    __m128i mAddOffset;

    __m128i mSwitch1 = _mm_setr_epi8(0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5,  6);
    __m128i mSwitch2 = _mm_setr_epi8(4, 5, 6, 7, 5, 6, 7, 8, 6, 7, 8, 9, 7, 8, 9, 10);

    __m128i mCoefx = _mm_set1_epi32(*(i32s_t*)coef_x);

    __m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask[(width & 7) - 1]));

    // HOR
    src = src - 1 * i_src - 1;
    
    if (width > 4) {
        for (row = -1; row < height + 2; row++) {
            __m128i mT0, mT1, mVal;
            for (col = 0; col < width; col += 8) {
                __m128i mSrc = _mm_loadu_si128((__m128i*)(src + col));
                mT0 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoefx);
                mT1 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoefx);

                mVal = _mm_hadd_epi16(mT0, mT1);
                _mm_store_si128((__m128i*)&tmp[col], mVal);
            }
            src += i_src;
            tmp += i_tmp;
        }
    } else {
        for (row = -1; row < height + 2; row++) {
            __m128i mSrc = _mm_loadu_si128((__m128i*)src);
            __m128i mT0 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoefx);
            __m128i mVal = _mm_hadd_epi16(mT0, mT0);
            _mm_storel_epi64((__m128i*)tmp, mVal);
            src += i_src;
            tmp += i_tmp;
        }
    }
    

    // VER
    shift = 12;
    mAddOffset = _mm_set1_epi32(1 << 11);

    tmp = tmp_res;
    if (bsymy) {
        __m128i mCoefy1 = _mm_set1_epi16(coef_y[0]);
        __m128i mCoefy2 = _mm_set1_epi16(coef_y[1]);
        __m128i mVal1, mVal2, mVal;

		for (row = 0; row < height; row++) {
			p = tmp;
			for (col = 0; col < width - 7; col += 8) {
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));

				__m128i M0 = _mm_unpacklo_epi16(T00, T30);
				__m128i M1 = _mm_unpacklo_epi16(T10, T20);
				__m128i M2 = _mm_unpackhi_epi16(T00, T30);
				__m128i M3 = _mm_unpackhi_epi16(T10, T20);

				M0 = _mm_madd_epi16(M0, mCoefy1);
				M1 = _mm_madd_epi16(M1, mCoefy2);
				M2 = _mm_madd_epi16(M2, mCoefy1);
				M3 = _mm_madd_epi16(M3, mCoefy2);

				mVal1 = _mm_add_epi32(M0, M1);
				mVal2 = _mm_add_epi32(M2, M3);

				mVal1 = _mm_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm_srai_epi32(mVal1, shift);
				mVal2 = _mm_srai_epi32(mVal2, shift);
				mVal = _mm_packs_epi32(mVal1, mVal2);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_storel_epi64((__m128i*)&dst[col], mVal);

				p += 8;
			}

			if (col < width) { // store either 1, 2, 3, 4, 5, 6, or 7 8-bit results in dst
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));

				__m128i M0 = _mm_unpacklo_epi16(T00, T30);
				__m128i M1 = _mm_unpacklo_epi16(T10, T20);
				__m128i M2 = _mm_unpackhi_epi16(T00, T30);
				__m128i M3 = _mm_unpackhi_epi16(T10, T20);

				M0 = _mm_madd_epi16(M0, mCoefy1);
				M1 = _mm_madd_epi16(M1, mCoefy2);
				M2 = _mm_madd_epi16(M2, mCoefy1);
				M3 = _mm_madd_epi16(M3, mCoefy2);

				mVal1 = _mm_add_epi32(M0, M1);
				mVal2 = _mm_add_epi32(M2, M3);

				mVal1 = _mm_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm_srai_epi32(mVal1, shift);
				mVal2 = _mm_srai_epi32(mVal2, shift);
				mVal = _mm_packs_epi32(mVal1, mVal2);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
			}

			tmp += i_tmp;
			dst += i_dst;
		}
    } else {
		__m128i coeff0 = _mm_set1_epi16(*(i32s_t*)coef_y);
		__m128i coeff1 = _mm_set1_epi16(*(i32s_t*)(coef_y + 2));
		__m128i mVal1, mVal2, mVal;
		coeff0 = _mm_cvtepi8_epi16(coeff0);
		coeff1 = _mm_cvtepi8_epi16(coeff1);
		
		for (row = 0; row < height; row++) {
			p = tmp;
			for (col = 0; col < width - 7; col += 8) {
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));

				__m128i M0 = _mm_unpacklo_epi16(T00, T10);
				__m128i M1 = _mm_unpacklo_epi16(T20, T30);
				__m128i M2 = _mm_unpackhi_epi16(T00, T10);
				__m128i M3 = _mm_unpackhi_epi16(T20, T30);

				M0 = _mm_madd_epi16(M0, coeff0);
				M1 = _mm_madd_epi16(M1, coeff1);
				M2 = _mm_madd_epi16(M2, coeff0);
				M3 = _mm_madd_epi16(M3, coeff1);

				mVal1 = _mm_add_epi32(M0, M1);
				mVal2 = _mm_add_epi32(M2, M3);

				mVal1 = _mm_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm_srai_epi32(mVal1, shift);
				mVal2 = _mm_srai_epi32(mVal2, shift);
				mVal = _mm_packs_epi32(mVal1, mVal2);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_storel_epi64((__m128i*)&dst[col], mVal);

				p += 8;
			}

			if (col < width) { // store either 1, 2, 3, 4, 5, 6, or 7 8-bit results in dst
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));

				__m128i M0 = _mm_unpacklo_epi16(T00, T10);
				__m128i M1 = _mm_unpacklo_epi16(T20, T30);
				__m128i M2 = _mm_unpackhi_epi16(T00, T10);
				__m128i M3 = _mm_unpackhi_epi16(T20, T30);

				M0 = _mm_madd_epi16(M0, coeff0);
				M1 = _mm_madd_epi16(M1, coeff1);
				M2 = _mm_madd_epi16(M2, coeff0);
				M3 = _mm_madd_epi16(M3, coeff1);

				mVal1 = _mm_add_epi32(M0, M1);
				mVal2 = _mm_add_epi32(M2, M3);

				mVal1 = _mm_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm_srai_epi32(mVal1, shift);
				mVal2 = _mm_srai_epi32(mVal2, shift);
				mVal = _mm_packs_epi32(mVal1, mVal2);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
			}

			tmp += i_tmp;
			dst += i_dst;
		}
	}
}

void com_if_filter_hor_ver_8_sse128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
    int row, col;
    int shift;
    i16s_t const *p;

    ALIGNED_16(i16s_t tmp_res[(64 + 7) * 64]);
    i16s_t *tmp = tmp_res;
    const int i_tmp = 64;

    int bsymy = (coef_y[1] == coef_y[6]);

    __m128i mAddOffset;

    __m128i mSwitch1 = _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8);
    __m128i mSwitch2 = _mm_setr_epi8(2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10);
    __m128i mSwitch3 = _mm_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12);
    __m128i mSwitch4 = _mm_setr_epi8(6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14);

    __m128i mCoefx  = _mm_loadl_epi64((__m128i*)coef_x);
    __m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask[(width & 7) - 1]));

    mCoefx = _mm_unpacklo_epi64(mCoefx, mCoefx);

    // HOR
    src = src - 3 * i_src - 3;

    for (row = -3; row < height + 4; row++) {
        __m128i mT0, mT1, mT2, mT3, s1, s2, mVal;

        for (col = 0; col < width; col += 8) {

            __m128i mSrc = _mm_loadu_si128((__m128i*)(src + col));

            mT0 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoefx);
            mT1 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoefx);
            mT2 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch3), mCoefx);
            mT3 = _mm_maddubs_epi16(_mm_shuffle_epi8(mSrc, mSwitch4), mCoefx);

            s1 = _mm_hadd_epi16(mT0, mT1);
            s2 = _mm_hadd_epi16(mT2, mT3);
            mVal = _mm_hadd_epi16(s1, s2);

            _mm_store_si128((__m128i*)&tmp[col], mVal);
        }

        src += i_src;
        tmp += i_tmp;
    }

    // VER
	shift = 12;
    mAddOffset = _mm_set1_epi32(1 << (shift - 1));
    tmp = tmp_res;

    if (bsymy) {
		__m128i mCoefy1 = _mm_set1_epi16(coef_y[0]);
		__m128i mCoefy2 = _mm_set1_epi16(coef_y[1]);
        __m128i mCoefy3 = _mm_set1_epi16(coef_y[2]);
        __m128i mCoefy4 = _mm_set1_epi16(coef_y[3]);
        __m128i mVal1, mVal2, mVal;

		for (row = 0; row < height; row++) {
			p = tmp;
			for (col = 0; col < width - 7; col += 8) {
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));
				__m128i T40 = _mm_loadu_si128((__m128i*)(p + 4 * i_tmp));
				__m128i T50 = _mm_loadu_si128((__m128i*)(p + 5 * i_tmp));
				__m128i T60 = _mm_loadu_si128((__m128i*)(p + 6 * i_tmp));
				__m128i T70 = _mm_loadu_si128((__m128i*)(p + 7 * i_tmp));

				__m128i T0 = _mm_unpacklo_epi16(T00, T70);
				__m128i T1 = _mm_unpacklo_epi16(T10, T60);
				__m128i T2 = _mm_unpacklo_epi16(T20, T50);
				__m128i T3 = _mm_unpacklo_epi16(T30, T40);
				__m128i T4 = _mm_unpackhi_epi16(T00, T70);
				__m128i T5 = _mm_unpackhi_epi16(T10, T60);
				__m128i T6 = _mm_unpackhi_epi16(T20, T50);
				__m128i T7 = _mm_unpackhi_epi16(T30, T40);

				T0 = _mm_madd_epi16(T0, mCoefy1);
				T1 = _mm_madd_epi16(T1, mCoefy2);
				T2 = _mm_madd_epi16(T2, mCoefy3);
				T3 = _mm_madd_epi16(T3, mCoefy4);
				T4 = _mm_madd_epi16(T4, mCoefy1);
				T5 = _mm_madd_epi16(T5, mCoefy2);
				T6 = _mm_madd_epi16(T6, mCoefy3);
				T7 = _mm_madd_epi16(T7, mCoefy4);

				mVal1 = _mm_add_epi32(T0, T1);
				mVal1 = _mm_add_epi32(mVal1, T2);
				mVal1 = _mm_add_epi32(mVal1, T3);

				mVal2 = _mm_add_epi32(T4, T5);
				mVal2 = _mm_add_epi32(mVal2, T6);
				mVal2 = _mm_add_epi32(mVal2, T7);

				mVal1 = _mm_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm_srai_epi32(mVal1, shift);
				mVal2 = _mm_srai_epi32(mVal2, shift);
				mVal = _mm_packs_epi32(mVal1, mVal2);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_storel_epi64((__m128i*)&dst[col], mVal);

				p += 8;
			}

			if (col < width) { // store either 1, 2, 3, 4, 5, 6, or 7 8-bit results in dst
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));
				__m128i T40 = _mm_loadu_si128((__m128i*)(p + 4 * i_tmp));
				__m128i T50 = _mm_loadu_si128((__m128i*)(p + 5 * i_tmp));
				__m128i T60 = _mm_loadu_si128((__m128i*)(p + 6 * i_tmp));
				__m128i T70 = _mm_loadu_si128((__m128i*)(p + 7 * i_tmp));

				__m128i T0 = _mm_unpacklo_epi16(T00, T70);
				__m128i T1 = _mm_unpacklo_epi16(T10, T60);
				__m128i T2 = _mm_unpacklo_epi16(T20, T50);
				__m128i T3 = _mm_unpacklo_epi16(T30, T40);
				__m128i T4 = _mm_unpackhi_epi16(T00, T70);
				__m128i T5 = _mm_unpackhi_epi16(T10, T60);
				__m128i T6 = _mm_unpackhi_epi16(T20, T50);
				__m128i T7 = _mm_unpackhi_epi16(T30, T40);

				T0 = _mm_madd_epi16(T0, mCoefy1);
				T1 = _mm_madd_epi16(T1, mCoefy2);
				T2 = _mm_madd_epi16(T2, mCoefy3);
				T3 = _mm_madd_epi16(T3, mCoefy4);
				T4 = _mm_madd_epi16(T4, mCoefy1);
				T5 = _mm_madd_epi16(T5, mCoefy2);
				T6 = _mm_madd_epi16(T6, mCoefy3);
				T7 = _mm_madd_epi16(T7, mCoefy4);

				mVal1 = _mm_add_epi32(T0, T1);
				mVal1 = _mm_add_epi32(mVal1, T2);
				mVal1 = _mm_add_epi32(mVal1, T3);

				mVal2 = _mm_add_epi32(T4, T5);
				mVal2 = _mm_add_epi32(mVal2, T6);
				mVal2 = _mm_add_epi32(mVal2, T7);

				mVal1 = _mm_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm_srai_epi32(mVal1, shift);
				mVal2 = _mm_srai_epi32(mVal2, shift);
				mVal = _mm_packs_epi32(mVal1, mVal2);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
			}
			tmp += i_tmp;
			dst += i_dst;
		}
	}
	else
	{
		__m128i mCoefy1 = _mm_set1_epi16(*(i16s_t*)coef_y);
		__m128i mCoefy2 = _mm_set1_epi16(*(i16s_t*)(coef_y + 2));
		__m128i mCoefy3 = _mm_set1_epi16(*(i16s_t*)(coef_y + 4));
		__m128i mCoefy4 = _mm_set1_epi16(*(i16s_t*)(coef_y + 6));
		__m128i mVal1, mVal2, mVal;
		mCoefy1 = _mm_cvtepi8_epi16(mCoefy1);
		mCoefy2 = _mm_cvtepi8_epi16(mCoefy2);
		mCoefy3 = _mm_cvtepi8_epi16(mCoefy3);
		mCoefy4 = _mm_cvtepi8_epi16(mCoefy4);

		for (row = 0; row < height; row++) {
			p = tmp;
			for (col = 0; col < width - 7; col += 8) {
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));
				__m128i T40 = _mm_loadu_si128((__m128i*)(p + 4 * i_tmp));
				__m128i T50 = _mm_loadu_si128((__m128i*)(p + 5 * i_tmp));
				__m128i T60 = _mm_loadu_si128((__m128i*)(p + 6 * i_tmp));
				__m128i T70 = _mm_loadu_si128((__m128i*)(p + 7 * i_tmp));

				__m128i T0 = _mm_unpacklo_epi16(T00, T10);
				__m128i T1 = _mm_unpacklo_epi16(T20, T30);
				__m128i T2 = _mm_unpacklo_epi16(T40, T50);
				__m128i T3 = _mm_unpacklo_epi16(T60, T70);
				__m128i T4 = _mm_unpackhi_epi16(T00, T10);
				__m128i T5 = _mm_unpackhi_epi16(T20, T30);
				__m128i T6 = _mm_unpackhi_epi16(T40, T50);
				__m128i T7 = _mm_unpackhi_epi16(T60, T70);

				T0 = _mm_madd_epi16(T0, mCoefy1);
				T1 = _mm_madd_epi16(T1, mCoefy2);
				T2 = _mm_madd_epi16(T2, mCoefy3);
				T3 = _mm_madd_epi16(T3, mCoefy4);
				T4 = _mm_madd_epi16(T4, mCoefy1);
				T5 = _mm_madd_epi16(T5, mCoefy2);
				T6 = _mm_madd_epi16(T6, mCoefy3);
				T7 = _mm_madd_epi16(T7, mCoefy4);

				mVal1 = _mm_add_epi32(T0, T1);
				mVal1 = _mm_add_epi32(mVal1, T2);
				mVal1 = _mm_add_epi32(mVal1, T3);

				mVal2 = _mm_add_epi32(T4, T5);
				mVal2 = _mm_add_epi32(mVal2, T6);
				mVal2 = _mm_add_epi32(mVal2, T7);

				mVal1 = _mm_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm_srai_epi32(mVal1, shift);
				mVal2 = _mm_srai_epi32(mVal2, shift);
				mVal = _mm_packs_epi32(mVal1, mVal2);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_storel_epi64((__m128i*)&dst[col], mVal);

				p += 8;
			}

			if (col < width) {
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));
				__m128i T40 = _mm_loadu_si128((__m128i*)(p + 4 * i_tmp));
				__m128i T50 = _mm_loadu_si128((__m128i*)(p + 5 * i_tmp));
				__m128i T60 = _mm_loadu_si128((__m128i*)(p + 6 * i_tmp));
				__m128i T70 = _mm_loadu_si128((__m128i*)(p + 7 * i_tmp));

				__m128i T0 = _mm_unpacklo_epi16(T00, T10);
				__m128i T1 = _mm_unpacklo_epi16(T20, T30);
				__m128i T2 = _mm_unpacklo_epi16(T40, T50);
				__m128i T3 = _mm_unpacklo_epi16(T60, T70);
				__m128i T4 = _mm_unpackhi_epi16(T00, T10);
				__m128i T5 = _mm_unpackhi_epi16(T20, T30);
				__m128i T6 = _mm_unpackhi_epi16(T40, T50);
				__m128i T7 = _mm_unpackhi_epi16(T60, T70);

				T0 = _mm_madd_epi16(T0, mCoefy1);
				T1 = _mm_madd_epi16(T1, mCoefy2);
				T2 = _mm_madd_epi16(T2, mCoefy3);
				T3 = _mm_madd_epi16(T3, mCoefy4);
				T4 = _mm_madd_epi16(T4, mCoefy1);
				T5 = _mm_madd_epi16(T5, mCoefy2);
				T6 = _mm_madd_epi16(T6, mCoefy3);
				T7 = _mm_madd_epi16(T7, mCoefy4);

				mVal1 = _mm_add_epi32(T0, T1);
				mVal1 = _mm_add_epi32(mVal1, T2);
				mVal1 = _mm_add_epi32(mVal1, T3);

				mVal2 = _mm_add_epi32(T4, T5);
				mVal2 = _mm_add_epi32(mVal2, T6);
				mVal2 = _mm_add_epi32(mVal2, T7);

				mVal1 = _mm_add_epi32(mVal1, mAddOffset);
				mVal2 = _mm_add_epi32(mVal2, mAddOffset);
				mVal1 = _mm_srai_epi32(mVal1, shift);
				mVal2 = _mm_srai_epi32(mVal2, shift);
				mVal = _mm_packs_epi32(mVal1, mVal2);
				mVal = _mm_packus_epi16(mVal, mVal);

				_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
			}

			tmp += i_tmp;
			dst += i_dst;
		}
	}
}

void com_if_filter_hor_4_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
	int row, col;
	const i16s_t offset = 32;
	const int shift = 6;

	__m128i mAddOffset = _mm_set1_epi32(offset);

	__m128i mSwitch1 = _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 2, 3, 4, 5, 6, 7, 8, 9);
	__m128i mSwitch2 = _mm_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 6, 7, 8, 9, 10, 11, 12, 13);
	__m128i mCoef = _mm_set1_epi32(*(i32s_t*)coeff);
	__m128i max_val1 = _mm_set1_epi16((pel_t)max_val);

	mCoef = _mm_cvtepi8_epi16(mCoef);
	src -= 1;

	if (!(width & 7)){
		for (row = 0; row < height; row++) {
			__m128i mT20, mT40, mVal, mT10, mT30;

			for (col = 0; col < width; col += 8) {
				__m128i mSrc = _mm_loadu_si128((__m128i*)(src + col));
				mT20 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoef);
				mT40 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoef);

				mSrc = _mm_loadu_si128((__m128i*)(src + col + 4));
				mT10 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoef);
				mT30 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoef);

				mT20 = _mm_hadd_epi32(mT20, mT40);
				mT10 = _mm_hadd_epi32(mT10, mT30);

				mT20 = _mm_add_epi32(mT20, mAddOffset);
				mT10 = _mm_add_epi32(mT10, mAddOffset);
				mT20 = _mm_srai_epi32(mT20, shift);
				mT10 = _mm_srai_epi32(mT10, shift);

				mVal = _mm_packus_epi32(mT20, mT10);
				mVal = _mm_min_epu16(mVal, max_val1);
				_mm_storeu_si128((__m128i*)&dst[col], mVal);
			}

			src += i_src;
			dst += i_dst;
		}
	}
	else{
		__m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask_10bit[(width & 7) - 1]));
		for (row = 0; row < height; row++) {
			__m128i mT20, mT40, mVal, mT10, mT30;

			for (col = 0; col < width - 7; col += 8) {
				__m128i mSrc = _mm_loadu_si128((__m128i*)(src + col));
				mT20 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoef);
				mT40 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoef);

				mSrc = _mm_loadu_si128((__m128i*)(src + col + 4));
				mT10 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoef);
				mT30 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoef);

				mT20 = _mm_hadd_epi32(mT20, mT40);
				mT10 = _mm_hadd_epi32(mT10, mT30);

				mT20 = _mm_add_epi32(mT20, mAddOffset);
				mT10 = _mm_add_epi32(mT10, mAddOffset);
				mT20 = _mm_srai_epi32(mT20, shift);
				mT10 = _mm_srai_epi32(mT10, shift);

				mVal = _mm_packus_epi32(mT20, mT10);
				mVal = _mm_min_epu16(mVal, max_val1);
				_mm_storeu_si128((__m128i*)&dst[col], mVal);
			}


			if (width - col > 4){
				__m128i mSrc = _mm_loadu_si128((__m128i*)(src + col));
				mT20 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoef);
				mT40 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoef);

				mSrc = _mm_loadu_si128((__m128i*)(src + col + 4));
				mT10 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoef);
				mT30 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoef);

				mT20 = _mm_hadd_epi32(mT20, mT40);
				mT10 = _mm_hadd_epi32(mT10, mT30);

				mT20 = _mm_add_epi32(mT20, mAddOffset);
				mT10 = _mm_add_epi32(mT10, mAddOffset);
				mT20 = _mm_srai_epi32(mT20, shift);
				mT10 = _mm_srai_epi32(mT10, shift);

				mVal = _mm_packus_epi32(mT20, mT10);
			}
			else
			{
				__m128i mSrc = _mm_loadu_si128((__m128i*)(src + col));
				mT20 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoef);
				mT40 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoef);
				mT20 = _mm_hadd_epi32(mT20, mT40);

				mT20 = _mm_add_epi32(mT20, mAddOffset);
				mT20 = _mm_srai_epi32(mT20, shift);

				mVal = _mm_packus_epi32(mT20, mT20);

			}
			mVal = _mm_min_epu16(mVal, max_val1);
			_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);

			src += i_src;
			dst += i_dst;
		}
	}
}

void com_if_filter_ver_4_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
	int row, col;
	const i16s_t offset = 32;
	const int shift = 6;
	int bsym = (coeff[1] == coeff[2]);
	__m128i mAddOffset = _mm_set1_epi32(offset);

	pel_t const *p;
	__m128i max_val1 = _mm_set1_epi16((pel_t)max_val);

	src -= i_src;

	if (bsym) {
		if (!(width & 7)){
            __m128i mVal;
			__m128i coeff0 = _mm_set1_epi16(*(i16s_t*)coeff);
			coeff0 = _mm_cvtepi8_epi16(coeff0);
			
			for (row = 0; row < height; row++) {
				p = src;
				for (col = 0; col < width; col += 8) {
					__m128i T00 = _mm_loadu_si128((__m128i*)(p));
					__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
					__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

					T00 = _mm_add_epi16(T00, T30);
					T20 = _mm_add_epi16(T10, T20);

					T10 = _mm_unpackhi_epi16(T00, T20);
					T00 = _mm_unpacklo_epi16(T00, T20);

					T00 = _mm_madd_epi16(T00, coeff0);
					T10 = _mm_madd_epi16(T10, coeff0);

					T00 = _mm_add_epi32(T00, mAddOffset);
					T00 = _mm_srai_epi32(T00, shift);
					T10 = _mm_add_epi32(T10, mAddOffset);
					T10 = _mm_srai_epi32(T10, shift);

					mVal = _mm_packus_epi32(T00, T10);
					mVal = _mm_min_epu16(mVal, max_val1);
					_mm_storeu_si128((__m128i*)&dst[col], mVal);

					p += 8;
				}

				src += i_src;
				dst += i_dst;
			}
		}
		else
		{
			__m128i coeff0 = _mm_cvtepi8_epi16(_mm_set1_epi16(*(i16s_t*)coeff));
			__m128i mVal;
			__m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask_10bit[(width & 7) - 1]));
			for (row = 0; row < height; row++) {
				p = src;
				for (col = 0; col < width - 7; col += 8) {
					__m128i T00 = _mm_loadu_si128((__m128i*)(p));
					__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
					__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

					T00 = _mm_add_epi16(T00, T30);
					T20 = _mm_add_epi16(T10, T20);

					T10 = _mm_unpackhi_epi16(T00, T20);
					T00 = _mm_unpacklo_epi16(T00, T20);

					T00 = _mm_madd_epi16(T00, coeff0);
					T10 = _mm_madd_epi16(T10, coeff0);

					T00 = _mm_add_epi32(T00, mAddOffset);
					T00 = _mm_srai_epi32(T00, shift);
					T10 = _mm_add_epi32(T10, mAddOffset);
					T10 = _mm_srai_epi32(T10, shift);

					mVal = _mm_packus_epi32(T00, T10);
					mVal = _mm_min_epu16(mVal, max_val1);
					_mm_storeu_si128((__m128i*)&dst[col], mVal);

					p += 8;
				}

				if (width - col > 4) {
					__m128i T00 = _mm_loadu_si128((__m128i*)(p));
					__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
					__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

					T00 = _mm_add_epi16(T00, T30);
					T20 = _mm_add_epi16(T10, T20);

					T10 = _mm_unpackhi_epi16(T00, T20);
					T00 = _mm_unpacklo_epi16(T00, T20);

					T00 = _mm_madd_epi16(T00, coeff0);
					T10 = _mm_madd_epi16(T10, coeff0);

					T00 = _mm_add_epi32(T00, mAddOffset);
					T00 = _mm_srai_epi32(T00, shift);
					T10 = _mm_add_epi32(T10, mAddOffset);
					T10 = _mm_srai_epi32(T10, shift);

					mVal = _mm_packus_epi32(T00, T10);
					mVal = _mm_min_epu16(mVal, max_val1);
					_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);

				}
				else
				{
					__m128i T00 = _mm_loadu_si128((__m128i*)(p));
					__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
					__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

					T00 = _mm_add_epi16(T00, T30);
					T20 = _mm_add_epi16(T10, T20);

					T00 = _mm_unpacklo_epi16(T00, T20);
					T00 = _mm_madd_epi16(T00, coeff0);

					T00 = _mm_add_epi32(T00, mAddOffset);
					T00 = _mm_srai_epi32(T00, shift);

					mVal = _mm_packus_epi32(T00, T00);
					mVal = _mm_min_epu16(mVal, max_val1);
					_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
				}

				src += i_src;
				dst += i_dst;
			}
		}
	}
	else {
		if (!(width & 7)){
			__m128i coeff0 = _mm_cvtepi8_epi16(_mm_set1_epi16(*(i16s_t*)coeff));
			__m128i coeff1 = _mm_cvtepi8_epi16(_mm_set1_epi16(*(i16s_t*)(coeff + 2)));
			__m128i mVal;
			__m128i M0, M1, M2, M3;
			for (row = 0; row < height; row++) {
				p = src;
				for (col = 0; col < width; col += 8) {
					__m128i T00 = _mm_loadu_si128((__m128i*)(p));
					__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
					__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

					M0 = _mm_unpacklo_epi16(T00, T10);
					M1 = _mm_unpacklo_epi16(T20, T30);
					M2 = _mm_unpackhi_epi16(T00, T10);
					M3 = _mm_unpackhi_epi16(T20, T30);

					M0 = _mm_madd_epi16(M0, coeff0);
					M1 = _mm_madd_epi16(M1, coeff1);
					M2 = _mm_madd_epi16(M2, coeff0);
					M3 = _mm_madd_epi16(M3, coeff1);

					M0 = _mm_add_epi32(M0, M1);
					M2 = _mm_add_epi32(M2, M3);

					M0 = _mm_add_epi32(M0, mAddOffset);
					M2 = _mm_add_epi32(M2, mAddOffset);
					M0 = _mm_srai_epi32(M0, shift);
					M2 = _mm_srai_epi32(M2, shift);

					mVal = _mm_packus_epi32(M0, M2);
					mVal = _mm_min_epu16(mVal, max_val1);
					_mm_storeu_si128((__m128i*)&dst[col], mVal);

					p += 8;
				}

				src += i_src;
				dst += i_dst;

			}
		}
		else{
			__m128i coeff0 = _mm_cvtepi8_epi16(_mm_set1_epi16(*(i16s_t*)coeff));
			__m128i coeff1 = _mm_cvtepi8_epi16(_mm_set1_epi16(*(i16s_t*)(coeff + 2)));
			__m128i mVal;
			__m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask_10bit[(width & 7) - 1]));
			__m128i M0, M1, M2, M3;
			for (row = 0; row < height; row++) {
				p = src;
				for (col = 0; col < width - 7; col += 8) {
					__m128i T00 = _mm_loadu_si128((__m128i*)(p));
					__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
					__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

					M0 = _mm_unpacklo_epi16(T00, T10);
					M1 = _mm_unpacklo_epi16(T20, T30);
					M2 = _mm_unpackhi_epi16(T00, T10);
					M3 = _mm_unpackhi_epi16(T20, T30);

					M0 = _mm_madd_epi16(M0, coeff0);
					M1 = _mm_madd_epi16(M1, coeff1);
					M2 = _mm_madd_epi16(M2, coeff0);
					M3 = _mm_madd_epi16(M3, coeff1);

					M0 = _mm_add_epi32(M0, M1);
					M2 = _mm_add_epi32(M2, M3);

					M0 = _mm_add_epi32(M0, mAddOffset);
					M2 = _mm_add_epi32(M2, mAddOffset);
					M0 = _mm_srai_epi32(M0, shift);
					M2 = _mm_srai_epi32(M2, shift);

					mVal = _mm_packus_epi32(M0, M2);
					mVal = _mm_min_epu16(mVal, max_val1);
					_mm_storeu_si128((__m128i*)&dst[col], mVal);

					p += 8;
				}

				if (width - col > 4) {
					__m128i T00 = _mm_loadu_si128((__m128i*)(p));
					__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
					__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

					M0 = _mm_unpacklo_epi16(T00, T10);
					M1 = _mm_unpacklo_epi16(T20, T30);
					M2 = _mm_unpackhi_epi16(T00, T10);
					M3 = _mm_unpackhi_epi16(T20, T30);

					M0 = _mm_madd_epi16(M0, coeff0);
					M1 = _mm_madd_epi16(M1, coeff1);
					M2 = _mm_madd_epi16(M2, coeff0);
					M3 = _mm_madd_epi16(M3, coeff1);

					M0 = _mm_add_epi32(M0, M1);
					M2 = _mm_add_epi32(M2, M3);

					M0 = _mm_add_epi32(M0, mAddOffset);
					M2 = _mm_add_epi32(M2, mAddOffset);
					M0 = _mm_srai_epi32(M0, shift);
					M2 = _mm_srai_epi32(M2, shift);

					mVal = _mm_packus_epi32(M0, M2);
					mVal = _mm_min_epu16(mVal, max_val1);

					_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
				}
				else
				{
					__m128i T00 = _mm_loadu_si128((__m128i*)(p));
					__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_src));
					__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));

					M0 = _mm_unpacklo_epi16(T00, T10);
					M1 = _mm_unpacklo_epi16(T20, T30);

					M0 = _mm_madd_epi16(M0, coeff0);
					M1 = _mm_madd_epi16(M1, coeff1);

					M0 = _mm_add_epi32(M0, M1);

					M0 = _mm_add_epi32(M0, mAddOffset);
					M0 = _mm_srai_epi32(M0, shift);

					mVal = _mm_packus_epi32(M0, M0);
					mVal = _mm_min_epu16(mVal, max_val1);

					_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);
				}

				src += i_src;
				dst += i_dst;

			}
		}
	}
}

void com_if_filter_hor_ver_4_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff_h, const char_t *coeff_v, int max_val)
{
	int row, col;
	i16s_t const *p;
	int shift1, shift2;
    int add1, add2;

	ALIGNED_16(i16s_t tmp_res[(32 + 3) * 32]);
	i16s_t *tmp = tmp_res;
	const int i_tmp = 32;

	__m128i mAddOffset1, mAddOffset2;

	__m128i mSwitch1 = _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 2, 3, 4, 5, 6, 7, 8, 9);
	__m128i mSwitch2 = _mm_setr_epi8(4, 5, 6, 7, 8, 9, 10, 11, 6, 7, 8, 9, 10, 11, 12, 13);

	__m128i mCoefx = _mm_cvtepi8_epi16(_mm_set1_epi32(*(i32s_t*)coeff_h));

	__m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask_10bit[(width & 7) - 1]));
	__m128i max_val1 = _mm_set1_epi16((pel_t)max_val);
    __m128i coeff0, coeff1, mVal;
    __m128i M0, M1, M2, M3;

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

    mAddOffset1 = _mm_set1_epi32(add1);
    mAddOffset2 = _mm_set1_epi32(add2);

	// HOR
	src = src - 1 * i_src - 1;

	if (width > 4) {
		for (row = -1; row < height + 2; row++) {
			__m128i mT0, mT1, mVal, mT2, mT3;
			for (col = 0; col < width; col += 8) {
				__m128i mSrc = _mm_loadu_si128((__m128i*)(src + col));
				mT0 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoefx);
				mT1 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoefx);

				mSrc = _mm_loadu_si128((__m128i*)(src + col + 4));
				mT2 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoefx);
				mT3 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoefx);

				mT0 = _mm_hadd_epi32(mT0, mT1);
				mT2 = _mm_hadd_epi32(mT2, mT3);

				mT0 = _mm_add_epi32(mT0, mAddOffset1);
				mT2 = _mm_add_epi32(mT2, mAddOffset1);
				mT0 = _mm_srai_epi32(mT0, shift1);
				mT2 = _mm_srai_epi32(mT2, shift1);

				mVal = _mm_packus_epi32(mT0, mT2);
				_mm_store_si128((__m128i*)&tmp[col], mVal);
			}
			src += i_src;
			tmp += i_tmp;
		}
	}
	else {
		for (row = -1; row < height + 2; row++) {
			__m128i mSrc = _mm_loadu_si128((__m128i*)(src));
			__m128i mT0 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch1), mCoefx);
			__m128i mT1 = _mm_madd_epi16(_mm_shuffle_epi8(mSrc, mSwitch2), mCoefx);

			mT0 = _mm_hadd_epi32(mT0, mT1);
			mT0 = _mm_add_epi32(mT0, mAddOffset1);
			mT0 = _mm_srai_epi32(mT0, shift1);

			mT0 = _mm_hadd_epi16(mT0, mT0);
			_mm_storel_epi64((__m128i*)tmp, mT0);
			src += i_src;
			tmp += i_tmp;
		}
	}


	// VER

	tmp = tmp_res;

	coeff0 = _mm_set1_epi16(*(i16s_t*)coeff_v);
	coeff1 = _mm_set1_epi16(*(i16s_t*)(coeff_v + 2));

	coeff0 = _mm_cvtepi8_epi16(coeff0);
	coeff1 = _mm_cvtepi8_epi16(coeff1);
	
	if (!(width & 7)){
		for (row = 0; row < height; row++) {
			p = tmp;
			for (col = 0; col < width; col += 8) {
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));

				M0 = _mm_unpacklo_epi16(T00, T10);
				M1 = _mm_unpacklo_epi16(T20, T30);
				M2 = _mm_unpackhi_epi16(T00, T10);
				M3 = _mm_unpackhi_epi16(T20, T30);

				M0 = _mm_madd_epi16(M0, coeff0);
				M1 = _mm_madd_epi16(M1, coeff1);
				M2 = _mm_madd_epi16(M2, coeff0);
				M3 = _mm_madd_epi16(M3, coeff1);

				M0 = _mm_add_epi32(M0, M1);
				M2 = _mm_add_epi32(M2, M3);

				M0 = _mm_add_epi32(M0, mAddOffset2);
				M2 = _mm_add_epi32(M2, mAddOffset2);
				M0 = _mm_srai_epi32(M0, shift2);
				M2 = _mm_srai_epi32(M2, shift2);

				mVal = _mm_packus_epi32(M0, M2);
				mVal = _mm_min_epu16(mVal, max_val1);
				_mm_storeu_si128((__m128i*)&dst[col], mVal);

				p += 8;
			}

			tmp += i_tmp;
			dst += i_dst;
		}
	}
	else{
		for (row = 0; row < height; row++) {
            __m128i T00, T10, T20, T30;
			p = tmp;
			for (col = 0; col < width - 7; col += 8) {
				__m128i T00 = _mm_loadu_si128((__m128i*)(p));
				__m128i T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				__m128i T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				__m128i T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));

				M0 = _mm_unpacklo_epi16(T00, T10);
				M1 = _mm_unpacklo_epi16(T20, T30);
				M2 = _mm_unpackhi_epi16(T00, T10);
				M3 = _mm_unpackhi_epi16(T20, T30);

				M0 = _mm_madd_epi16(M0, coeff0);
				M1 = _mm_madd_epi16(M1, coeff1);
				M2 = _mm_madd_epi16(M2, coeff0);
				M3 = _mm_madd_epi16(M3, coeff1);

				M0 = _mm_add_epi32(M0, M1);
				M2 = _mm_add_epi32(M2, M3);

				M0 = _mm_add_epi32(M0, mAddOffset2);
				M2 = _mm_add_epi32(M2, mAddOffset2);
				M0 = _mm_srai_epi32(M0, shift2);
				M2 = _mm_srai_epi32(M2, shift2);

				mVal = _mm_packus_epi32(M0, M2);
				mVal = _mm_min_epu16(mVal, max_val1);
				_mm_storeu_si128((__m128i*)&dst[col], mVal);

				p += 8;
			}

			T00 = _mm_loadu_si128((__m128i*)(p));
			T10 = _mm_loadu_si128((__m128i*)(p + i_tmp));
			T20 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
			T30 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));

			M0 = _mm_unpacklo_epi16(T00, T10);
			M1 = _mm_unpacklo_epi16(T20, T30);
			M2 = _mm_unpackhi_epi16(T00, T10);
			M3 = _mm_unpackhi_epi16(T20, T30);

			M0 = _mm_madd_epi16(M0, coeff0);
			M1 = _mm_madd_epi16(M1, coeff1);
			M2 = _mm_madd_epi16(M2, coeff0);
			M3 = _mm_madd_epi16(M3, coeff1);

			M0 = _mm_add_epi32(M0, M1);
			M2 = _mm_add_epi32(M2, M3);

			M0 = _mm_add_epi32(M0, mAddOffset2);
			M2 = _mm_add_epi32(M2, mAddOffset2);
			M0 = _mm_srai_epi32(M0, shift2);
			M2 = _mm_srai_epi32(M2, shift2);

			mVal = _mm_packus_epi32(M0, M2);
			mVal = _mm_min_epu16(mVal, max_val1);
			_mm_maskmoveu_si128(mVal, mask, (char_t*)&dst[col]);

			tmp += i_tmp;
			dst += i_dst;
		}
	}
}

void com_if_filter_hor_8_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
	int i, j;
	__m128i max_val1 = _mm_set1_epi16((pel_t)max_val);
	__m128i mCoef = _mm_loadl_epi64((__m128i*)coeff);

	__m128i T0, T1, T2, T3, T4, T5, T6, T7;
	__m128i M0, M1, M2, M3, M4, M5, M6, M7;
	__m128i offset = _mm_set1_epi32(32);
	mCoef = _mm_cvtepi8_epi16(mCoef);

	src -= 3;
	if (width & 7){
		__m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask_10bit[(width & 7) - 1]));
		for (j = 0; j < height; j++) {
			const pel_t *p = src;
			for (i = 0; i < width - 7; i += 8) {

				T0 = _mm_loadu_si128((__m128i*)p++);
				T1 = _mm_loadu_si128((__m128i*)p++);
				T2 = _mm_loadu_si128((__m128i*)p++);
				T3 = _mm_loadu_si128((__m128i*)p++);
				T4 = _mm_loadu_si128((__m128i*)p++);
				T5 = _mm_loadu_si128((__m128i*)p++);
				T6 = _mm_loadu_si128((__m128i*)p++);
				T7 = _mm_loadu_si128((__m128i*)p++);

				M0 = _mm_madd_epi16(T0, mCoef);
				M1 = _mm_madd_epi16(T1, mCoef);
				M2 = _mm_madd_epi16(T2, mCoef);
				M3 = _mm_madd_epi16(T3, mCoef);
				M4 = _mm_madd_epi16(T4, mCoef);
				M5 = _mm_madd_epi16(T5, mCoef);
				M6 = _mm_madd_epi16(T6, mCoef);
				M7 = _mm_madd_epi16(T7, mCoef);

				M0 = _mm_hadd_epi32(M0, M1);
				M1 = _mm_hadd_epi32(M2, M3);
				M2 = _mm_hadd_epi32(M4, M5);
				M3 = _mm_hadd_epi32(M6, M7);

				M0 = _mm_hadd_epi32(M0, M1);
				M1 = _mm_hadd_epi32(M2, M3);

				M2 = _mm_add_epi32(M0, offset);
				M3 = _mm_add_epi32(M1, offset);
				M2 = _mm_srai_epi32(M2, 6);
				M3 = _mm_srai_epi32(M3, 6);
				M2 = _mm_packus_epi32(M2, M3);
				M2 = _mm_min_epu16(M2, max_val1);
				_mm_storeu_si128((__m128i*)(dst + i), M2);

			}

			T0 = _mm_loadu_si128((__m128i*)p++);
			T1 = _mm_loadu_si128((__m128i*)p++);
			T2 = _mm_loadu_si128((__m128i*)p++);
			T3 = _mm_loadu_si128((__m128i*)p++);
			T4 = _mm_loadu_si128((__m128i*)p++);
			T5 = _mm_loadu_si128((__m128i*)p++);
			T6 = _mm_loadu_si128((__m128i*)p++);
			T7 = _mm_loadu_si128((__m128i*)p++);

			M0 = _mm_madd_epi16(T0, mCoef);
			M1 = _mm_madd_epi16(T1, mCoef);
			M2 = _mm_madd_epi16(T2, mCoef);
			M3 = _mm_madd_epi16(T3, mCoef);
			M4 = _mm_madd_epi16(T4, mCoef);
			M5 = _mm_madd_epi16(T5, mCoef);
			M6 = _mm_madd_epi16(T6, mCoef);
			M7 = _mm_madd_epi16(T7, mCoef);

			M0 = _mm_hadd_epi32(M0, M1);
			M1 = _mm_hadd_epi32(M2, M3);
			M2 = _mm_hadd_epi32(M4, M5);
			M3 = _mm_hadd_epi32(M6, M7);

			M0 = _mm_hadd_epi32(M0, M1);
			M1 = _mm_hadd_epi32(M2, M3);

			M2 = _mm_add_epi32(M0, offset);
			M3 = _mm_add_epi32(M1, offset);
			M2 = _mm_srai_epi32(M2, 6);
			M3 = _mm_srai_epi32(M3, 6);
			M2 = _mm_packus_epi32(M2, M3);
			M2 = _mm_min_epu16(M2, max_val1);
			_mm_maskmoveu_si128(M2, mask, (char_t*)&dst[i]);

			dst += i_dst;
			src += i_src;
		}
	}
	else{
		for (j = 0; j < height; j++) {
			const pel_t *p = src;
			for (i = 0; i < width; i += 8) {

				T0 = _mm_loadu_si128((__m128i*)p++);
				T1 = _mm_loadu_si128((__m128i*)p++);
				T2 = _mm_loadu_si128((__m128i*)p++);
				T3 = _mm_loadu_si128((__m128i*)p++);
				T4 = _mm_loadu_si128((__m128i*)p++);
				T5 = _mm_loadu_si128((__m128i*)p++);
				T6 = _mm_loadu_si128((__m128i*)p++);
				T7 = _mm_loadu_si128((__m128i*)p++);

				M0 = _mm_madd_epi16(T0, mCoef);
				M1 = _mm_madd_epi16(T1, mCoef);
				M2 = _mm_madd_epi16(T2, mCoef);
				M3 = _mm_madd_epi16(T3, mCoef);
				M4 = _mm_madd_epi16(T4, mCoef);
				M5 = _mm_madd_epi16(T5, mCoef);
				M6 = _mm_madd_epi16(T6, mCoef);
				M7 = _mm_madd_epi16(T7, mCoef);

				M0 = _mm_hadd_epi32(M0, M1);
				M1 = _mm_hadd_epi32(M2, M3);
				M2 = _mm_hadd_epi32(M4, M5);
				M3 = _mm_hadd_epi32(M6, M7);

				M0 = _mm_hadd_epi32(M0, M1);
				M1 = _mm_hadd_epi32(M2, M3);

				M2 = _mm_add_epi32(M0, offset);
				M3 = _mm_add_epi32(M1, offset);
				M2 = _mm_srai_epi32(M2, 6);
				M3 = _mm_srai_epi32(M3, 6);
				M2 = _mm_packus_epi32(M2, M3);
				M2 = _mm_min_epu16(M2, max_val1);
				_mm_storeu_si128((__m128i*)(dst + i), M2);

			}
			dst += i_dst;
			src += i_src;
		}
	}

}

void com_if_filter_ver_8_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val)
{
	int i, j;
	__m128i max_val1 = _mm_set1_epi16((pel_t)max_val);
	__m128i mAddOffset = _mm_set1_epi32(32);
	__m128i T0, T1, T2, T3, T4, T5, T6, T7;
	__m128i M0, M1, M2, M3, M4, M5, M6, M7;
	__m128i N0, N1, N2, N3, N4, N5, N6, N7;

	if (coeff[3] != coeff[4]){
		
		__m128i coeff00 = _mm_set1_epi16(*(i16s_t*)coeff);
		__m128i coeff01 = _mm_set1_epi16(*(i16s_t*)(coeff + 2));
		__m128i coeff02 = _mm_set1_epi16(*(i16s_t*)(coeff + 4));
		__m128i coeff03 = _mm_set1_epi16(*(i16s_t*)(coeff + 6));
		coeff00 = _mm_cvtepi8_epi16(coeff00);
		coeff01 = _mm_cvtepi8_epi16(coeff01);
		coeff02 = _mm_cvtepi8_epi16(coeff02);
		coeff03 = _mm_cvtepi8_epi16(coeff03);
        src -= 3 * i_src;

		if (width & 7){
			__m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask_10bit[(width & 7) - 1]));
			for (j = 0; j < height; j++) {
				const pel_t *p = src;
				for (i = 0; i < width - 7; i += 8) {
					T0 = _mm_loadu_si128((__m128i*)(p));
					T1 = _mm_loadu_si128((__m128i*)(p + i_src));
					T2 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					T3 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));
					T4 = _mm_loadu_si128((__m128i*)(p + 4 * i_src));
					T5 = _mm_loadu_si128((__m128i*)(p + 5 * i_src));
					T6 = _mm_loadu_si128((__m128i*)(p + 6 * i_src));
					T7 = _mm_loadu_si128((__m128i*)(p + 7 * i_src));

					M0 = _mm_unpacklo_epi16(T0, T1);
					M1 = _mm_unpacklo_epi16(T2, T3);
					M2 = _mm_unpacklo_epi16(T4, T5);
					M3 = _mm_unpacklo_epi16(T6, T7);
					M4 = _mm_unpackhi_epi16(T0, T1);
					M5 = _mm_unpackhi_epi16(T2, T3);
					M6 = _mm_unpackhi_epi16(T4, T5);
					M7 = _mm_unpackhi_epi16(T6, T7);

					N0 = _mm_madd_epi16(M0, coeff00);
					N1 = _mm_madd_epi16(M1, coeff01);
					N2 = _mm_madd_epi16(M2, coeff02);
					N3 = _mm_madd_epi16(M3, coeff03);
					N4 = _mm_madd_epi16(M4, coeff00);
					N5 = _mm_madd_epi16(M5, coeff01);
					N6 = _mm_madd_epi16(M6, coeff02);
					N7 = _mm_madd_epi16(M7, coeff03);

					N0 = _mm_add_epi32(N0, N1);
					N1 = _mm_add_epi32(N2, N3);
					N2 = _mm_add_epi32(N4, N5);
					N3 = _mm_add_epi32(N6, N7);

					N0 = _mm_add_epi32(N0, N1);
					N1 = _mm_add_epi32(N2, N3);

					N0 = _mm_add_epi32(N0, mAddOffset);
					N1 = _mm_add_epi32(N1, mAddOffset);
					N0 = _mm_srai_epi32(N0, 6);
					N1 = _mm_srai_epi32(N1, 6);
					N0 = _mm_packus_epi32(N0, N1);
					N0 = _mm_min_epu16(N0, max_val1);
					_mm_storeu_si128((__m128i*)(dst + i), N0);

					p += 8;
				}
				T0 = _mm_loadu_si128((__m128i*)(p));
				T1 = _mm_loadu_si128((__m128i*)(p + i_src));
				T2 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
				T3 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));
				T4 = _mm_loadu_si128((__m128i*)(p + 4 * i_src));
				T5 = _mm_loadu_si128((__m128i*)(p + 5 * i_src));
				T6 = _mm_loadu_si128((__m128i*)(p + 6 * i_src));
				T7 = _mm_loadu_si128((__m128i*)(p + 7 * i_src));

				M0 = _mm_unpacklo_epi16(T0, T1);
				M1 = _mm_unpacklo_epi16(T2, T3);
				M2 = _mm_unpacklo_epi16(T4, T5);
				M3 = _mm_unpacklo_epi16(T6, T7);
				M4 = _mm_unpackhi_epi16(T0, T1);
				M5 = _mm_unpackhi_epi16(T2, T3);
				M6 = _mm_unpackhi_epi16(T4, T5);
				M7 = _mm_unpackhi_epi16(T6, T7);

				N0 = _mm_madd_epi16(M0, coeff00);
				N1 = _mm_madd_epi16(M1, coeff01);
				N2 = _mm_madd_epi16(M2, coeff02);
				N3 = _mm_madd_epi16(M3, coeff03);
				N4 = _mm_madd_epi16(M4, coeff00);
				N5 = _mm_madd_epi16(M5, coeff01);
				N6 = _mm_madd_epi16(M6, coeff02);
				N7 = _mm_madd_epi16(M7, coeff03);

				N0 = _mm_add_epi32(N0, N1);
				N1 = _mm_add_epi32(N2, N3);
				N2 = _mm_add_epi32(N4, N5);
				N3 = _mm_add_epi32(N6, N7);

				N0 = _mm_add_epi32(N0, N1);
				N1 = _mm_add_epi32(N2, N3);

				N0 = _mm_add_epi32(N0, mAddOffset);
				N1 = _mm_add_epi32(N1, mAddOffset);
				N0 = _mm_srai_epi32(N0, 6);
				N1 = _mm_srai_epi32(N1, 6);
				N0 = _mm_packus_epi32(N0, N1);
				N0 = _mm_min_epu16(N0, max_val1);
				_mm_maskmoveu_si128(N0, mask, (char_t*)&dst[i]);

				dst += i_dst;
				src += i_src;
			}
		}
		else{
			for (j = 0; j < height; j++) {
				const pel_t *p = src;
				for (i = 0; i < width; i += 8) {
					T0 = _mm_loadu_si128((__m128i*)(p));
					T1 = _mm_loadu_si128((__m128i*)(p + i_src));
					T2 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					T3 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));
					T4 = _mm_loadu_si128((__m128i*)(p + 4 * i_src));
					T5 = _mm_loadu_si128((__m128i*)(p + 5 * i_src));
					T6 = _mm_loadu_si128((__m128i*)(p + 6 * i_src));
					T7 = _mm_loadu_si128((__m128i*)(p + 7 * i_src));

					M0 = _mm_unpacklo_epi16(T0, T1);
					M1 = _mm_unpacklo_epi16(T2, T3);
					M2 = _mm_unpacklo_epi16(T4, T5);
					M3 = _mm_unpacklo_epi16(T6, T7);
					M4 = _mm_unpackhi_epi16(T0, T1);
					M5 = _mm_unpackhi_epi16(T2, T3);
					M6 = _mm_unpackhi_epi16(T4, T5);
					M7 = _mm_unpackhi_epi16(T6, T7);

					N0 = _mm_madd_epi16(M0, coeff00);
					N1 = _mm_madd_epi16(M1, coeff01);
					N2 = _mm_madd_epi16(M2, coeff02);
					N3 = _mm_madd_epi16(M3, coeff03);
					N4 = _mm_madd_epi16(M4, coeff00);
					N5 = _mm_madd_epi16(M5, coeff01);
					N6 = _mm_madd_epi16(M6, coeff02);
					N7 = _mm_madd_epi16(M7, coeff03);

					N0 = _mm_add_epi32(N0, N1);
					N1 = _mm_add_epi32(N2, N3);
					N2 = _mm_add_epi32(N4, N5);
					N3 = _mm_add_epi32(N6, N7);

					N0 = _mm_add_epi32(N0, N1);
					N1 = _mm_add_epi32(N2, N3);

					N0 = _mm_add_epi32(N0, mAddOffset);
					N1 = _mm_add_epi32(N1, mAddOffset);
					N0 = _mm_srai_epi32(N0, 6);
					N1 = _mm_srai_epi32(N1, 6);
					N0 = _mm_packus_epi32(N0, N1);
					N0 = _mm_min_epu16(N0, max_val1);
					_mm_storeu_si128((__m128i*)(dst + i), N0);

					p += 8;
				}
				dst += i_dst;
				src += i_src;
			}
		}
	}
	else{ //ÏµÊý¶Ô³Æ
		__m128i coeff10 = _mm_set1_epi16(*(i16s_t*)coeff);
		__m128i coeff11 = _mm_set1_epi16(*(i16s_t*)(coeff + 2));
		coeff10 = _mm_cvtepi8_epi16(coeff10);
		coeff11 = _mm_cvtepi8_epi16(coeff11);

		src -= 3 * i_src;
		if (width & 7){
			__m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask_10bit[(width & 7) - 1]));
			for (j = 0; j < height; j++) {
				const pel_t *p = src;
				for (i = 0; i < width - 7; i += 8) {
					T0 = _mm_loadu_si128((__m128i*)(p));
					T1 = _mm_loadu_si128((__m128i*)(p + i_src));
					T2 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					T3 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));
					T4 = _mm_loadu_si128((__m128i*)(p + 4 * i_src));
					T5 = _mm_loadu_si128((__m128i*)(p + 5 * i_src));
					T6 = _mm_loadu_si128((__m128i*)(p + 6 * i_src));
					T7 = _mm_loadu_si128((__m128i*)(p + 7 * i_src));

					T0 = _mm_add_epi16(T0, T7);
					T1 = _mm_add_epi16(T1, T6);
					T2 = _mm_add_epi16(T2, T5);
					T3 = _mm_add_epi16(T3, T4);

					M0 = _mm_unpacklo_epi16(T0, T1);
					M1 = _mm_unpacklo_epi16(T2, T3);
					M2 = _mm_unpackhi_epi16(T0, T1);
					M3 = _mm_unpackhi_epi16(T2, T3);

					N0 = _mm_madd_epi16(M0, coeff10);
					N1 = _mm_madd_epi16(M1, coeff11);
					N2 = _mm_madd_epi16(M2, coeff10);
					N3 = _mm_madd_epi16(M3, coeff11);

					N0 = _mm_add_epi32(N0, N1);
					N1 = _mm_add_epi32(N2, N3);

					N0 = _mm_add_epi32(N0, mAddOffset);
					N1 = _mm_add_epi32(N1, mAddOffset);
					N0 = _mm_srai_epi32(N0, 6);
					N1 = _mm_srai_epi32(N1, 6);
					N0 = _mm_packus_epi32(N0, N1);
					N0 = _mm_min_epu16(N0, max_val1);
					_mm_storeu_si128((__m128i*)(dst + i), N0);

					p += 8;
				}
				T0 = _mm_loadu_si128((__m128i*)(p));
				T1 = _mm_loadu_si128((__m128i*)(p + i_src));
				T2 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
				T3 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));
				T4 = _mm_loadu_si128((__m128i*)(p + 4 * i_src));
				T5 = _mm_loadu_si128((__m128i*)(p + 5 * i_src));
				T6 = _mm_loadu_si128((__m128i*)(p + 6 * i_src));
				T7 = _mm_loadu_si128((__m128i*)(p + 7 * i_src));

				T0 = _mm_add_epi16(T0, T7);
				T1 = _mm_add_epi16(T1, T6);
				T2 = _mm_add_epi16(T2, T5);
				T3 = _mm_add_epi16(T3, T4);

				M0 = _mm_unpacklo_epi16(T0, T1);
				M1 = _mm_unpacklo_epi16(T2, T3);
				M2 = _mm_unpackhi_epi16(T0, T1);
				M3 = _mm_unpackhi_epi16(T2, T3);

				N0 = _mm_madd_epi16(M0, coeff10);
				N1 = _mm_madd_epi16(M1, coeff11);
				N2 = _mm_madd_epi16(M2, coeff10);
				N3 = _mm_madd_epi16(M3, coeff11);

				N0 = _mm_add_epi32(N0, N1);
				N1 = _mm_add_epi32(N2, N3);

				N0 = _mm_add_epi32(N0, mAddOffset);
				N1 = _mm_add_epi32(N1, mAddOffset);
				N0 = _mm_srai_epi32(N0, 6);
				N1 = _mm_srai_epi32(N1, 6);
				N0 = _mm_packus_epi32(N0, N1);
				N0 = _mm_min_epu16(N0, max_val1);
				_mm_maskmoveu_si128(N0, mask, (char_t*)&dst[i]);

				dst += i_dst;
				src += i_src;
			}
		}
		else{
			for (j = 0; j < height; j++) {
				const pel_t *p = src;
				for (i = 0; i < width; i += 8) {
					T0 = _mm_loadu_si128((__m128i*)(p));
					T1 = _mm_loadu_si128((__m128i*)(p + i_src));
					T2 = _mm_loadu_si128((__m128i*)(p + 2 * i_src));
					T3 = _mm_loadu_si128((__m128i*)(p + 3 * i_src));
					T4 = _mm_loadu_si128((__m128i*)(p + 4 * i_src));
					T5 = _mm_loadu_si128((__m128i*)(p + 5 * i_src));
					T6 = _mm_loadu_si128((__m128i*)(p + 6 * i_src));
					T7 = _mm_loadu_si128((__m128i*)(p + 7 * i_src));

					T0 = _mm_add_epi16(T0, T7);
					T1 = _mm_add_epi16(T1, T6);
					T2 = _mm_add_epi16(T2, T5);
					T3 = _mm_add_epi16(T3, T4);

					M0 = _mm_unpacklo_epi16(T0, T1);
					M1 = _mm_unpacklo_epi16(T2, T3);
					M2 = _mm_unpackhi_epi16(T0, T1);
					M3 = _mm_unpackhi_epi16(T2, T3);

					N0 = _mm_madd_epi16(M0, coeff10);
					N1 = _mm_madd_epi16(M1, coeff11);
					N2 = _mm_madd_epi16(M2, coeff10);
					N3 = _mm_madd_epi16(M3, coeff11);

					N0 = _mm_add_epi32(N0, N1);
					N1 = _mm_add_epi32(N2, N3);

					N0 = _mm_add_epi32(N0, mAddOffset);
					N1 = _mm_add_epi32(N1, mAddOffset);
					N0 = _mm_srai_epi32(N0, 6);
					N1 = _mm_srai_epi32(N1, 6);
					N0 = _mm_packus_epi32(N0, N1);
					N0 = _mm_min_epu16(N0, max_val1);
					_mm_storeu_si128((__m128i*)(dst + i), N0);

					p += 8;
				}
				dst += i_dst;
				src += i_src;
			}
		}
	}
}

void com_if_filter_hor_ver_8_sse128_10bit(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val)
{
	int add1, shift1;
	int add2, shift2;
	__m128i T0, T1, T2, T3, T4, T5, T6, T7;
	__m128i M0, M1, M2, M3, M4, M5, M6, M7;
	__m128i N0, N1, N2, N3, N4, N5, N6, N7;
    __m128i mCoef, offset, max_val1;
	int i, j;

	ALIGNED_16(i16s_t tmp_res[(64 + 7) * 64]);
	i16s_t *tmp;
	int i_tmp = 64;
    __m128i coeff00, coeff01, coeff02, coeff03;

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

	src += -3 * i_src - 3;
	tmp = tmp_res;

	mCoef = _mm_loadl_epi64((__m128i*)coef_x);
	offset = _mm_set1_epi32(add1);
	max_val1 = _mm_set1_epi16((pel_t)max_val);
	mCoef = _mm_cvtepi8_epi16(mCoef);

	for (j = -3; j < height + 4; j++) {
		const pel_t *p = src;
		for (i = 0; i < width; i += 8) {

			T0 = _mm_loadu_si128((__m128i*)p++);
			T1 = _mm_loadu_si128((__m128i*)p++);
			T2 = _mm_loadu_si128((__m128i*)p++);
			T3 = _mm_loadu_si128((__m128i*)p++);
			T4 = _mm_loadu_si128((__m128i*)p++);
			T5 = _mm_loadu_si128((__m128i*)p++);
			T6 = _mm_loadu_si128((__m128i*)p++);
			T7 = _mm_loadu_si128((__m128i*)p++);

			M0 = _mm_madd_epi16(T0, mCoef);
			M1 = _mm_madd_epi16(T1, mCoef);
			M2 = _mm_madd_epi16(T2, mCoef);
			M3 = _mm_madd_epi16(T3, mCoef);
			M4 = _mm_madd_epi16(T4, mCoef);
			M5 = _mm_madd_epi16(T5, mCoef);
			M6 = _mm_madd_epi16(T6, mCoef);
			M7 = _mm_madd_epi16(T7, mCoef);

			M0 = _mm_hadd_epi32(M0, M1);
			M1 = _mm_hadd_epi32(M2, M3);
			M2 = _mm_hadd_epi32(M4, M5);
			M3 = _mm_hadd_epi32(M6, M7);

			M0 = _mm_hadd_epi32(M0, M1);
			M1 = _mm_hadd_epi32(M2, M3);

			M2 = _mm_add_epi32(M0, offset);
			M3 = _mm_add_epi32(M1, offset);
			M2 = _mm_srai_epi32(M2, shift1);
			M3 = _mm_srai_epi32(M3, shift1);
			M2 = _mm_packs_epi32(M2, M3);
			_mm_storeu_si128((__m128i*)(tmp + i), M2);

		}
		tmp += i_tmp;
		src += i_src;
	}

	offset = _mm_set1_epi32(add2);
	tmp = tmp_res;

	coeff00 = _mm_set1_epi16(*(i16s_t*)coef_y);
	coeff01 = _mm_set1_epi16(*(i16s_t*)(coef_y + 2));
	coeff02 = _mm_set1_epi16(*(i16s_t*)(coef_y + 4));
	coeff03 = _mm_set1_epi16(*(i16s_t*)(coef_y + 6));
	coeff00 = _mm_cvtepi8_epi16(coeff00);
	coeff01 = _mm_cvtepi8_epi16(coeff01);
	coeff02 = _mm_cvtepi8_epi16(coeff02);
	coeff03 = _mm_cvtepi8_epi16(coeff03);
	if (width & 7){
		__m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask_10bit[(width & 7) - 1]));
		for (j = 0; j < height; j++) {
			const pel_t *p = (pel_t*)tmp;
			for (i = 0; i < width - 7; i += 8) {
				T0 = _mm_loadu_si128((__m128i*)(p));
				T1 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				T2 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				T3 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));
				T4 = _mm_loadu_si128((__m128i*)(p + 4 * i_tmp));
				T5 = _mm_loadu_si128((__m128i*)(p + 5 * i_tmp));
				T6 = _mm_loadu_si128((__m128i*)(p + 6 * i_tmp));
				T7 = _mm_loadu_si128((__m128i*)(p + 7 * i_tmp));

				M0 = _mm_unpacklo_epi16(T0, T1);
				M1 = _mm_unpacklo_epi16(T2, T3);
				M2 = _mm_unpacklo_epi16(T4, T5);
				M3 = _mm_unpacklo_epi16(T6, T7);
				M4 = _mm_unpackhi_epi16(T0, T1);
				M5 = _mm_unpackhi_epi16(T2, T3);
				M6 = _mm_unpackhi_epi16(T4, T5);
				M7 = _mm_unpackhi_epi16(T6, T7);

				N0 = _mm_madd_epi16(M0, coeff00);
				N1 = _mm_madd_epi16(M1, coeff01);
				N2 = _mm_madd_epi16(M2, coeff02);
				N3 = _mm_madd_epi16(M3, coeff03);
				N4 = _mm_madd_epi16(M4, coeff00);
				N5 = _mm_madd_epi16(M5, coeff01);
				N6 = _mm_madd_epi16(M6, coeff02);
				N7 = _mm_madd_epi16(M7, coeff03);

				N0 = _mm_add_epi32(N0, N1);
				N1 = _mm_add_epi32(N2, N3);
				N2 = _mm_add_epi32(N4, N5);
				N3 = _mm_add_epi32(N6, N7);

				N0 = _mm_add_epi32(N0, N1);
				N1 = _mm_add_epi32(N2, N3);

				N0 = _mm_add_epi32(N0, offset);
				N1 = _mm_add_epi32(N1, offset);
				N0 = _mm_srai_epi32(N0, shift2);
				N1 = _mm_srai_epi32(N1, shift2);
				N0 = _mm_packus_epi32(N0, N1);
				N0 = _mm_min_epu16(N0, max_val1);
				_mm_storeu_si128((__m128i*)(dst + i), N0);

				p += 8;
			}
			T0 = _mm_loadu_si128((__m128i*)(p));
			T1 = _mm_loadu_si128((__m128i*)(p + i_tmp));
			T2 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
			T3 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));
			T4 = _mm_loadu_si128((__m128i*)(p + 4 * i_tmp));
			T5 = _mm_loadu_si128((__m128i*)(p + 5 * i_tmp));
			T6 = _mm_loadu_si128((__m128i*)(p + 6 * i_tmp));
			T7 = _mm_loadu_si128((__m128i*)(p + 7 * i_tmp));

			M0 = _mm_unpacklo_epi16(T0, T1);
			M1 = _mm_unpacklo_epi16(T2, T3);
			M2 = _mm_unpacklo_epi16(T4, T5);
			M3 = _mm_unpacklo_epi16(T6, T7);
			M4 = _mm_unpackhi_epi16(T0, T1);
			M5 = _mm_unpackhi_epi16(T2, T3);
			M6 = _mm_unpackhi_epi16(T4, T5);
			M7 = _mm_unpackhi_epi16(T6, T7);

			N0 = _mm_madd_epi16(M0, coeff00);
			N1 = _mm_madd_epi16(M1, coeff01);
			N2 = _mm_madd_epi16(M2, coeff02);
			N3 = _mm_madd_epi16(M3, coeff03);
			N4 = _mm_madd_epi16(M4, coeff00);
			N5 = _mm_madd_epi16(M5, coeff01);
			N6 = _mm_madd_epi16(M6, coeff02);
			N7 = _mm_madd_epi16(M7, coeff03);

			N0 = _mm_add_epi32(N0, N1);
			N1 = _mm_add_epi32(N2, N3);
			N2 = _mm_add_epi32(N4, N5);
			N3 = _mm_add_epi32(N6, N7);

			N0 = _mm_add_epi32(N0, N1);
			N1 = _mm_add_epi32(N2, N3);

			N0 = _mm_add_epi32(N0, offset);
			N1 = _mm_add_epi32(N1, offset);
			N0 = _mm_srai_epi32(N0, shift2);
			N1 = _mm_srai_epi32(N1, shift2);
			N0 = _mm_packus_epi32(N0, N1);
			N0 = _mm_min_epu16(N0, max_val1);
			_mm_maskmoveu_si128(N0, mask, (char_t*)&dst[i]);

			dst += i_dst;
			tmp += i_tmp;
		}
	}
	else{
		for (j = 0; j < height; j++) {
			const pel_t *p = (pel_t*)tmp;
			for (i = 0; i < width; i += 8) {
				T0 = _mm_loadu_si128((__m128i*)(p));
				T1 = _mm_loadu_si128((__m128i*)(p + i_tmp));
				T2 = _mm_loadu_si128((__m128i*)(p + 2 * i_tmp));
				T3 = _mm_loadu_si128((__m128i*)(p + 3 * i_tmp));
				T4 = _mm_loadu_si128((__m128i*)(p + 4 * i_tmp));
				T5 = _mm_loadu_si128((__m128i*)(p + 5 * i_tmp));
				T6 = _mm_loadu_si128((__m128i*)(p + 6 * i_tmp));
				T7 = _mm_loadu_si128((__m128i*)(p + 7 * i_tmp));

				M0 = _mm_unpacklo_epi16(T0, T1);
				M1 = _mm_unpacklo_epi16(T2, T3);
				M2 = _mm_unpacklo_epi16(T4, T5);
				M3 = _mm_unpacklo_epi16(T6, T7);
				M4 = _mm_unpackhi_epi16(T0, T1);
				M5 = _mm_unpackhi_epi16(T2, T3);
				M6 = _mm_unpackhi_epi16(T4, T5);
				M7 = _mm_unpackhi_epi16(T6, T7);

				N0 = _mm_madd_epi16(M0, coeff00);
				N1 = _mm_madd_epi16(M1, coeff01);
				N2 = _mm_madd_epi16(M2, coeff02);
				N3 = _mm_madd_epi16(M3, coeff03);
				N4 = _mm_madd_epi16(M4, coeff00);
				N5 = _mm_madd_epi16(M5, coeff01);
				N6 = _mm_madd_epi16(M6, coeff02);
				N7 = _mm_madd_epi16(M7, coeff03);

				N0 = _mm_add_epi32(N0, N1);
				N1 = _mm_add_epi32(N2, N3);
				N2 = _mm_add_epi32(N4, N5);
				N3 = _mm_add_epi32(N6, N7);

				N0 = _mm_add_epi32(N0, N1);
				N1 = _mm_add_epi32(N2, N3);

				N0 = _mm_add_epi32(N0, offset);
				N1 = _mm_add_epi32(N1, offset);
				N0 = _mm_srai_epi32(N0, shift2);
				N1 = _mm_srai_epi32(N1, shift2);
				N0 = _mm_packus_epi32(N0, N1);
				N0 = _mm_min_epu16(N0, max_val1);
				_mm_storeu_si128((__m128i*)(dst + i), N0);

				p += 8;
			}
			dst += i_dst;
			tmp += i_tmp;
		}
	}
}
