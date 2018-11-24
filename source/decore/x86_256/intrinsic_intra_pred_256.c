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

#include "intrinsic_256.h"

/*void xPredIntraVertAdi(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight)
{
	int x, y;
	pel_t *rpSrc = pSrc + 1;

	for (y = 0; y < iHeight; y++) {
		for (x = 0; x < iWidth; x++) {
			dst[x] = rpSrc[x];
		}
		dst += i_dst;
	}
}*/
void xPredIntraVertAdi_sse256(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight)
{
	pel_t *rpSrc = pSrc + 1;
	int i;
	__m256i S1;

	if (iWidth >= 32)
	{
		for (i = 0; i < iHeight; i++)
		{
			S1 = _mm256_loadu_si256((const __m256i*)(rpSrc));//32
			_mm256_storeu_si256((__m256i*)(dst), S1);

			if (32 < iWidth)
			{
				S1 = _mm256_loadu_si256((const __m256i*)(rpSrc + 32));//64
				_mm256_storeu_si256((__m256i*)(dst + 32), S1);
			}
			dst += i_dst;
		}
	}
	else
	{
		int i, j;
		__m128i S1;
		if (iWidth & 15) {//4/8
			__m128i mask = _mm_load_si128((const __m128i*)intrinsic_mask16[(iWidth & 15) - 1]);
			for (i = 0; i < iHeight; i++) {
				for (j = 0; j < iWidth - 15; j += 16) {
					S1 = _mm_loadu_si128((const __m128i*)(rpSrc+ j));
					_mm_storeu_si128((__m128i*)(dst + j), S1);
				}
				S1 = _mm_loadu_si128((const __m128i*)(rpSrc + j));
				_mm_maskmoveu_si128(S1, mask, (char *)&dst[j]);
				dst += i_dst;
			}
		}
		/*{//4/8
			for (i = 0; i < iHeight; i++) {
				for (j = 0; j < iWidth; j += 4) {
					S1 = _mm_loadu_si128((const __m128i*)(rpSrc + j));
					_mm_storeu_si128((__m128i*)(dst + j), S1);
				}
				dst += i_dst;
			}
		}*/
		else {
			for (i = 0; i < iHeight; i++) {//16
				S1 = _mm_loadu_si128((const __m128i*)rpSrc);
				_mm_storeu_si128((__m128i*)dst, S1);
				dst += i_dst;
			}
		}
	}
}
/*
void xPredIntraHorAdi(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight)
{
	int x, y;
	pel_t *rpSrc = pSrc - 1;

	for (y = 0; y < iHeight; y++) {
		for (x = 0; x < iWidth; x++) {
			dst[x] = rpSrc[-y];
		}
		dst += i_dst;
	}
}*/
void xPredIntraHorAdi_sse256(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight)
{
	int i;
	pel_t *rpSrc = pSrc - 1;
	__m256i S1;

	if (iWidth >= 32)
	{
		for (i = 0; i < iHeight; i++)
		{
			S1 = _mm256_set1_epi8((char)rpSrc[-i]);//32
			_mm256_storeu_si256((__m256i*)(dst), S1);

			if (32 < iWidth)
			{//64
			_mm256_storeu_si256((__m256i*)(dst + 32), S1);
			}
			dst += i_dst;
		}
	}
	else
	{
		int i, j;
		__m128i S1;
		if (iWidth & 15) 
		{//4/8
			__m128i mask = _mm_load_si128((const __m128i*)intrinsic_mask16[(iWidth & 15) - 1]);
			for (i = 0; i < iHeight; i++)
			{
				for (j = 0; j < iWidth - 15; j += 16) 
				{
				S1 = _mm_set1_epi8((char)rpSrc[-i]);
				_mm_storeu_si128((__m128i*)(dst + j), S1);
				}
				S1 = _mm_set1_epi8((char)rpSrc[-i]);
				_mm_maskmoveu_si128(S1, mask, (char*)&dst[j]);
				dst += i_dst;
			}
		}
		else {
			for (i = 0; i < iHeight; i++) 
			{//16
				S1 = _mm_set1_epi8((char)rpSrc[-i]);
				_mm_storeu_si128((__m128i*)dst, S1);
				dst += i_dst;
			}
		}
	}
}
/*
void xPredIntraDCAdi(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bAboveAvail, int bLeftAvail, int sample_bit_depth)
{
int   x, y;
int   iDCValue = 0;
pel_t  *rpSrc = pSrc - 1;

if (bLeftAvail) {
for (y = 0; y < iHeight; y++) {
iDCValue += rpSrc[-y];
}

rpSrc = pSrc + 1;
if (bAboveAvail) {
for (x = 0; x < iWidth; x++) {
iDCValue += rpSrc[x];
}

iDCValue += ((iWidth + iHeight) >> 1);
iDCValue = (iDCValue * (512 / (iWidth + iHeight))) >> 9;
} else {
iDCValue += iHeight / 2;
iDCValue /= iHeight;
}
} else {
rpSrc = pSrc + 1;
if (bAboveAvail) {
for (x = 0; x < iWidth; x++) {
iDCValue += rpSrc[x];
}

iDCValue += iWidth / 2;
iDCValue /= iWidth;
} else {
iDCValue = 1 << (sample_bit_depth - 1);
}
}

for (y = 0; y < iHeight; y++) {
for (x = 0; x < iWidth; x++) {
dst[x] = iDCValue;
}
dst += i_dst;
}
}
*/
void xPredIntraDCAdi_see256(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bAboveAvail, int bLeftAvail, int sample_bit_depth)
{
	int   x, y;
	int   iDCValue = 0;
	pel_t  *rpSrc = pSrc - 1;
	__m256i S1;
	int i, j;
	if (bLeftAvail) {
		for (y = 0; y < iHeight; y++) {
			iDCValue += rpSrc[-y];
		}

		rpSrc = pSrc + 1;
		if (bAboveAvail) {
			for (x = 0; x < iWidth; x++) {
				iDCValue += rpSrc[x];
			}

			iDCValue += ((iWidth + iHeight) >> 1);
			iDCValue = (iDCValue * (512 / (iWidth + iHeight))) >> 9;
		}
		else {
			iDCValue += iHeight / 2;
			iDCValue /= iHeight;
		}
	}
	else {
		rpSrc = pSrc + 1;
		if (bAboveAvail) {
			for (x = 0; x < iWidth; x++) {
				iDCValue += rpSrc[x];
			}

			iDCValue += iWidth / 2;
			iDCValue /= iWidth;
		}
		else {
			iDCValue = 1 << (sample_bit_depth - 1);
		}
	}
	/*
	for (y = 0; y < iHeight; y++) {
		for (x = 0; x < iWidth; x++) {
			dst[x] = iDCValue;
		}
		dst += i_dst;
	}
	*/

	S1 = _mm256_set1_epi8((char)iDCValue);
	if (iWidth >= 32)
	{
		for (i = 0; i < iHeight; i++)
		{
			_mm256_storeu_si256((__m256i*)(dst), S1);//32
			if (32 < iWidth)
			{//64
			_mm256_storeu_si256((__m256i*)(dst + 32), S1);
			}
			dst += i_dst;
		}
	}
	else
	{
		__m128i S1;
		S1 = _mm_set1_epi8((char)iDCValue);
		if (iWidth & 15)
		{//4/8
			__m128i mask = _mm_load_si128((const __m128i*)intrinsic_mask16[(iWidth & 15) - 1]);
			for (i = 0; i < iHeight; i++)
			{
				for (j = 0; j < iWidth - 15; j += 16)
				{
				_mm_storeu_si128((__m128i*)(dst + j), S1);
				}
				_mm_maskmoveu_si128(S1, mask, (char *)&dst[j]);
				dst += i_dst;
			}
		}
		else {
			for (i = 0; i < iHeight; i++)
			{//16
				_mm_storeu_si128((__m128i*)dst, S1);
				dst += i_dst;
			}
		}
	}
}