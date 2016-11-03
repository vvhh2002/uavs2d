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


void cpy_pel_I420_to_uchar_YUY2_sse128(const pel_t *srcy, const pel_t *srcu, const pel_t *srcv, int i_src, int i_srcc, uchar_t *dst, int i_dst, int width, int height, int bit_size)
{
    int i, j, l;
    const pel_t *src1 = srcy + i_src;
    uchar_t     *dst1 = dst + i_dst;

    __m128i TY00, TY01, TY10, TY11, TU, TV, TC0, TC1;
    __m128i TD00, TD01, TD10, TD11;

    if (!(width & 31)) {
        for (i = 0; i < height; i += 2) {
            for (j = 0, l = 0; j < width - 31; j += 32, l += 64) {
                TU = _mm_load_si128((const __m128i*)(srcu + (j >> 1)));
                TV = _mm_load_si128((const __m128i*)(srcv + (j >> 1)));

                TC0 = _mm_unpacklo_epi8(TU, TV);
                TY00 = _mm_load_si128((const __m128i*)(srcy + j));
                TY10 = _mm_load_si128((const __m128i*)(src1 + j));

                TD00 = _mm_unpacklo_epi8(TY00, TC0);
                TD01 = _mm_unpackhi_epi8(TY00, TC0);
                TD10 = _mm_unpacklo_epi8(TY10, TC0);
                TD11 = _mm_unpackhi_epi8(TY10, TC0);

                _mm_storeu_si128((__m128i*)(dst + l), TD00);
                _mm_storeu_si128((__m128i*)(dst + l + 16), TD01);
                _mm_storeu_si128((__m128i*)(dst1 + l), TD10);
                _mm_storeu_si128((__m128i*)(dst1 + l + 16), TD11);

                TC1 = _mm_unpackhi_epi8(TU, TV);
                TY01 = _mm_load_si128((const __m128i*)(srcy + j + 16));
                TY11 = _mm_load_si128((const __m128i*)(src1 + j + 16));

                TD00 = _mm_unpacklo_epi8(TY01, TC1);
                TD01 = _mm_unpackhi_epi8(TY01, TC1);
                TD10 = _mm_unpacklo_epi8(TY11, TC1);
                TD11 = _mm_unpackhi_epi8(TY11, TC1);

                _mm_storeu_si128((__m128i*)(dst + l + 32), TD00);
                _mm_storeu_si128((__m128i*)(dst + l + 48), TD01);
                _mm_storeu_si128((__m128i*)(dst1 + l + 32), TD10);
                _mm_storeu_si128((__m128i*)(dst1 + l + 48), TD11);
            }
            srcy += 2 * i_src;
            src1 += 2 * i_src;
            dst  += 2 * i_dst;
            dst1 += 2 * i_dst;
            srcu += i_srcc;
            srcv += i_srcc;
        }
    } else {
        for (i = 0; i < height; i += 2) {
            for (j = 0, l = 0; j < width - 15; j += 16, l += 32) {
                TU = _mm_loadl_epi64((const __m128i*)(srcu + (j >> 1)));
                TV = _mm_loadl_epi64((const __m128i*)(srcv + (j >> 1)));

                TC0 = _mm_unpacklo_epi8(TU, TV);
                TY00 = _mm_load_si128((const __m128i*)(srcy + j));
                TY10 = _mm_load_si128((const __m128i*)(src1 + j));

                TD00 = _mm_unpacklo_epi8(TY00, TC0);
                TD01 = _mm_unpackhi_epi8(TY00, TC0);
                TD10 = _mm_unpacklo_epi8(TY10, TC0);
                TD11 = _mm_unpackhi_epi8(TY10, TC0);

                _mm_storeu_si128((__m128i*)(dst  + l     ), TD00);
                _mm_storeu_si128((__m128i*)(dst  + l + 16), TD01);
                _mm_storeu_si128((__m128i*)(dst1 + l     ), TD10);
                _mm_storeu_si128((__m128i*)(dst1 + l + 16), TD11);
            }

            if (j < width) {
                assert(j + 8 == width);
                TU = _mm_loadl_epi64((const __m128i*)(srcu + (j >> 1)));
                TV = _mm_loadl_epi64((const __m128i*)(srcv + (j >> 1)));

                TC0 = _mm_unpacklo_epi8(TU, TV);
                TY00 = _mm_load_si128((const __m128i*)(srcy + j));
                TY10 = _mm_load_si128((const __m128i*)(src1 + j));

                TD00 = _mm_unpacklo_epi8(TY00, TC0);
                TD10 = _mm_unpacklo_epi8(TY10, TC0);

                _mm_storeu_si128((__m128i*)(dst  + l), TD00);
                _mm_storeu_si128((__m128i*)(dst1 + l), TD10);
            }

            srcy += 2 * i_src;
            src1 += 2 * i_src;
            dst += 2 * i_dst;
            dst1 += 2 * i_dst;
            srcu += i_srcc;
            srcv += i_srcc;
        }
    }
}

void cpy_pel_I420_to_uchar_YUY2_sse128_10bit(const pel_t *srcy, const pel_t *srcu, const pel_t *srcv, int i_src, int i_srcc, uchar_t *dst, int i_dst, int width, int height, int bit_size)
{
    int i, j, l;
    const pel_t *src1 = srcy + i_src;
    uchar_t     *dst1 = dst + i_dst;

    __m128i TY00, TY10, TU, TV, TC0;
    __m128i TD00, TD01, TD10, TD11;
    __m128i YO00, YO01, YO10, YO11;

    if (bit_size == 10) {
        __m128i add = _mm_set1_epi16(2);

        for (i = 0; i < height; i += 2) {
            for (j = 0, l = 0; j < width - 15; j += 16, l += 32) {
                TU = _mm_srli_epi16(_mm_add_epi16(_mm_load_si128((const __m128i*)(srcu + (j >> 1))), add), 2);
                TV = _mm_srli_epi16(_mm_add_epi16(_mm_load_si128((const __m128i*)(srcv + (j >> 1))), add), 2);
                TU = _mm_packus_epi16(TU, TU);
                TV = _mm_packus_epi16(TV, TV);
                TC0 = _mm_unpacklo_epi8(TU, TV);

                YO00 = _mm_srli_epi16(_mm_add_epi16(_mm_load_si128((const __m128i*)(srcy + j    )), add), 2);
                YO01 = _mm_srli_epi16(_mm_add_epi16(_mm_load_si128((const __m128i*)(srcy + j + 8)), add), 2);
                YO10 = _mm_srli_epi16(_mm_add_epi16(_mm_load_si128((const __m128i*)(src1 + j    )), add), 2);
                YO11 = _mm_srli_epi16(_mm_add_epi16(_mm_load_si128((const __m128i*)(src1 + j + 8)), add), 2);

                TY00 = _mm_packus_epi16(YO00, YO01);
                TY10 = _mm_packus_epi16(YO10, YO11);

                TD00 = _mm_unpacklo_epi8(TY00, TC0);
                TD01 = _mm_unpackhi_epi8(TY00, TC0);
                TD10 = _mm_unpacklo_epi8(TY10, TC0);
                TD11 = _mm_unpackhi_epi8(TY10, TC0);

                _mm_storeu_si128((__m128i*)(dst  + l     ), TD00);
                _mm_storeu_si128((__m128i*)(dst  + l + 16), TD01);
                _mm_storeu_si128((__m128i*)(dst1 + l     ), TD10);
                _mm_storeu_si128((__m128i*)(dst1 + l + 16), TD11);
            }

            if (j < width) {
                assert(j + 8 == width);
                TU = _mm_srli_epi16(_mm_add_epi16(_mm_load_si128((const __m128i*)(srcu + (j >> 1))), add), 2);
                TV = _mm_srli_epi16(_mm_add_epi16(_mm_load_si128((const __m128i*)(srcv + (j >> 1))), add), 2);
                TU = _mm_packus_epi16(TU, TU);
                TV = _mm_packus_epi16(TV, TV);
                TC0 = _mm_unpacklo_epi8(TU, TV);

                YO00 = _mm_srli_epi16(_mm_add_epi16(_mm_load_si128((const __m128i*)(srcy + j)), add), 2);
                YO10 = _mm_srli_epi16(_mm_add_epi16(_mm_load_si128((const __m128i*)(src1 + j)), add), 2);
                TY00 = _mm_packus_epi16(YO00, YO00);
                TY10 = _mm_packus_epi16(YO10, YO10);

                TD00 = _mm_unpacklo_epi8(TY00, TC0);
                TD10 = _mm_unpacklo_epi8(TY10, TC0);

                _mm_storeu_si128((__m128i*)(dst  + l), TD00);
                _mm_storeu_si128((__m128i*)(dst1 + l), TD10);
            }

            srcy += 2 * i_src;
            src1 += 2 * i_src;
            dst  += 2 * i_dst;
            dst1 += 2 * i_dst;
            srcu += i_srcc;
            srcv += i_srcc;
        }
    } else {
        for (i = 0; i < height; i += 2) {
            for (j = 0, l = 0; j < width - 15; j += 16, l += 32) {
                TU = _mm_load_si128((const __m128i*)(srcu + (j >> 1)));
                TV = _mm_load_si128((const __m128i*)(srcv + (j >> 1)));
                TU = _mm_packus_epi16(TU, TU);
                TV = _mm_packus_epi16(TV, TV);
                TC0 = _mm_unpacklo_epi8(TU, TV);

                YO00 = _mm_load_si128((const __m128i*)(srcy + j    ));
                YO01 = _mm_load_si128((const __m128i*)(srcy + j + 8));
                YO10 = _mm_load_si128((const __m128i*)(src1 + j    ));
                YO11 = _mm_load_si128((const __m128i*)(src1 + j + 8));

                TY00 = _mm_packus_epi16(YO00, YO01);
                TY10 = _mm_packus_epi16(YO10, YO11);

                TD00 = _mm_unpacklo_epi8(TY00, TC0);
                TD01 = _mm_unpackhi_epi8(TY00, TC0);
                TD10 = _mm_unpacklo_epi8(TY10, TC0);
                TD11 = _mm_unpackhi_epi8(TY10, TC0);

                _mm_storeu_si128((__m128i*)(dst + l), TD00);
                _mm_storeu_si128((__m128i*)(dst + l + 16), TD01);
                _mm_storeu_si128((__m128i*)(dst1 + l), TD10);
                _mm_storeu_si128((__m128i*)(dst1 + l + 16), TD11);
            }

            if (j < width) {
                assert(j + 8 == width);
                TU = _mm_load_si128((const __m128i*)(srcu + (j >> 1)));
                TV = _mm_load_si128((const __m128i*)(srcv + (j >> 1)));
                TU = _mm_packus_epi16(TU, TU);
                TV = _mm_packus_epi16(TV, TV);
                TC0 = _mm_unpacklo_epi8(TU, TV);

                YO00 = _mm_load_si128((const __m128i*)(srcy + j));
                YO10 = _mm_load_si128((const __m128i*)(src1 + j));
                TY00 = _mm_packus_epi16(YO00, YO00);
                TY10 = _mm_packus_epi16(YO10, YO10);

                TD00 = _mm_unpacklo_epi8(TY00, TC0);
                TD10 = _mm_unpacklo_epi8(TY10, TC0);

                _mm_storeu_si128((__m128i*)(dst + l), TD00);
                _mm_storeu_si128((__m128i*)(dst1 + l), TD10);
            }

            srcy += 2 * i_src;
            src1 += 2 * i_src;
            dst += 2 * i_dst;
            dst1 += 2 * i_dst;
            srcu += i_srcc;
            srcv += i_srcc;
        }
    }
}


void add_pel_clip_sse128(const pel_t *src1, int i_src1, const resi_t *src2, int i_src2, pel_t *dst, int i_dst, int width, int height, int bit_depth)
{
    int i, j;
    __m128i zero = _mm_setzero_si128();
    __m128i S, S1, S2, R1, R2, D;
    
    if (width & 15) {
        __m128i mask = _mm_load_si128((const __m128i*)intrinsic_mask[(width & 15) - 1]);

        for (i = 0; i < height; i++) {
            for (j = 0; j < width - 15; j += 16) {
                S  = _mm_loadu_si128((const __m128i*)(src1 + j));
                R1 = _mm_loadu_si128((const __m128i*)(src2 + j));
                R2 = _mm_loadu_si128((const __m128i*)(src2 + j + 8));
                S1 = _mm_unpacklo_epi8(S, zero);
                S2 = _mm_unpackhi_epi8(S, zero);
                S1 = _mm_add_epi16(R1, S1);
                S2 = _mm_add_epi16(R2, S2);
                D = _mm_packus_epi16(S1, S2);
                _mm_store_si128((__m128i*)(dst + j), D);
            }

            S  = _mm_loadu_si128((const __m128i*)(src1 + j));
            R1 = _mm_loadu_si128((const __m128i*)(src2 + j));
            R2 = _mm_loadu_si128((const __m128i*)(src2 + j + 8));
            S1 = _mm_unpacklo_epi8(S, zero);
            S2 = _mm_unpackhi_epi8(S, zero);
            S1 = _mm_add_epi16(R1, S1);
            S2 = _mm_add_epi16(R2, S2);
            D = _mm_packus_epi16(S1, S2);
            _mm_maskmoveu_si128(D, mask, (char_t*)&dst[j]);

            src1 += i_src1;
            src2 += i_src2;
            dst += i_dst;
        }
    } else {
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j += 16) {
                S = _mm_loadu_si128((const __m128i*)(src1 + j));
                R1 = _mm_loadu_si128((const __m128i*)(src2 + j));
                R2 = _mm_loadu_si128((const __m128i*)(src2 + j + 8));
                S1 = _mm_unpacklo_epi8(S, zero);
                S2 = _mm_unpackhi_epi8(S, zero);
                S1 = _mm_add_epi16(R1, S1);
                S2 = _mm_add_epi16(R2, S2);
                D = _mm_packus_epi16(S1, S2);
                _mm_storeu_si128((__m128i*)(dst + j), D);
            }
            src1 += i_src1;
            src2 += i_src2;
            dst += i_dst;
        }
    }
}

void add_pel_clip_sse128_10bit(const pel_t *src1, int i_src1, const resi_t *src2, int i_src2, pel_t *dst, int i_dst, int width, int height, int bit_depth)
{
    int j;
    __m128i zero = _mm_setzero_si128();
    __m128i D;
    __m128i max_val = _mm_set1_epi16((1 << bit_depth) - 1);

    if (width & 7) {
        __m128i mask = _mm_load_si128((const __m128i*)intrinsic_mask_10bit[(width & 7) - 1]);

        while (height--) {
            for (j = 0; j < width - 7; j += 8) {
                D = _mm_add_epi16(_mm_loadu_si128((const __m128i*)(src1 + j)), _mm_loadu_si128((const __m128i*)(src2 + j)));
                D = _mm_min_epi16(D, max_val);
                D = _mm_max_epi16(D, zero);
                _mm_store_si128((__m128i*)(dst + j), D);
            }

            D = _mm_add_epi16(_mm_loadu_si128((const __m128i*)(src1 + j)), _mm_loadu_si128((const __m128i*)(src2 + j)));
            D = _mm_min_epi16(D, max_val);
            D = _mm_max_epi16(D, zero);
            _mm_maskmoveu_si128(D, mask, (char_t*)&dst[j]);

            src1 += i_src1;
            src2 += i_src2;
            dst += i_dst;
        }
    } else {
        while (height--) {
            for (j = 0; j < width; j += 8) {
                D = _mm_add_epi16(_mm_loadu_si128((const __m128i*)(src1 + j)), _mm_loadu_si128((const __m128i*)(src2 + j)));
                D = _mm_min_epi16(D, max_val);
                D = _mm_max_epi16(D, zero);
                _mm_store_si128((__m128i*)(dst + j), D);
            }
            src1 += i_src1;
            src2 += i_src2;
            dst  += i_dst;
        }
    }
}

void avg_pel_sse128(pel_t *dst, int i_dst, pel_t *src1, int i_src1, pel_t *src2, int i_src2, int width, int height)
{
    int i, j;
    __m128i S1, S2, D;

    if (width & 15) {
        __m128i mask = _mm_load_si128((const __m128i*)intrinsic_mask[(width & 15) - 1]);

        for (i = 0; i < height; i++) {
            for (j = 0; j < width - 15; j += 16) {
                S1 = _mm_loadu_si128((const __m128i*)(src1 + j));
                S2 = _mm_load_si128((const __m128i*)(src2 + j));
                D = _mm_avg_epu8(S1, S2);
                _mm_storeu_si128((__m128i*)(dst + j), D);
            }

            S1 = _mm_loadu_si128((const __m128i*)(src1 + j));
            S2 = _mm_load_si128((const __m128i*)(src2 + j));
            D = _mm_avg_epu8(S1, S2);
            _mm_maskmoveu_si128(D, mask, (char_t*)&dst[j]);

            src1 += i_src1;
            src2 += i_src2;
            dst += i_dst;
        }
    } else {
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j += 16) {
                S1 = _mm_loadu_si128((const __m128i*)(src1 + j));
                S2 = _mm_load_si128((const __m128i*)(src2 + j));
                D = _mm_avg_epu8(S1, S2);
                _mm_storeu_si128((__m128i*)(dst + j), D);
            }
            src1 += i_src1;
            src2 += i_src2;
            dst += i_dst;
        }
    }
}

void avg_pel_sse128_10bit(pel_t *dst, int i_dst, pel_t *src1, int i_src1, pel_t *src2, int i_src2, int width, int height)
{
    int j;
    __m128i D;

    if (width & 7) {
        __m128i mask = _mm_load_si128((const __m128i*)intrinsic_mask_10bit[(width & 7) - 1]);

        while (height--) {
            for (j = 0; j < width - 7; j += 8) {
                D = _mm_avg_epu16(_mm_loadu_si128((const __m128i*)(src1 + j)), _mm_loadu_si128((const __m128i*)(src2 + j)));
                _mm_storeu_si128((__m128i*)(dst + j), D);
            }

            D = _mm_avg_epu16(_mm_loadu_si128((const __m128i*)(src1 + j)), _mm_loadu_si128((const __m128i*)(src2 + j)));
            _mm_maskmoveu_si128(D, mask, (char_t*)&dst[j]);

            src1 += i_src1;
            src2 += i_src2;
            dst += i_dst;
        }
    }
    else {
        while (height--) {
            for (j = 0; j < width; j += 8) {
                D = _mm_avg_epu16(_mm_loadu_si128((const __m128i*)(src1 + j)), _mm_loadu_si128((const __m128i*)(src2 + j)));
                _mm_storeu_si128((__m128i*)(dst + j), D);
            }
            src1 += i_src1;
            src2 += i_src2;
            dst += i_dst;
        }
    }
}

void padding_rows_sse128(pel_t *src, int i_src, int width, int height, int start, int rows, int pad)
{
    int i, j;
    pel_t *p, *p1, *p2;
    int pad_lr = pad + 16 - (pad & 0xF);
    start = max(start, 0);

    if (start + rows > height) {
        rows = height - start;
    }

    p = src + start * i_src;

    // left & right
    for (i = 0; i < rows; i++) {
        __m128i Val1 = _mm_set1_epi8((char)p[0]);
        __m128i Val2 = _mm_set1_epi8((char)p[width - 1]);
        p1 = p - pad_lr;
        p2 = p + width;
        for (j = 0; j < pad_lr; j += 16) {
            _mm_storeu_si128((__m128i*)(p1 + j), Val1);
            _mm_storeu_si128((__m128i*)(p2 + j), Val2);
        }

        p += i_src;
    }

    if (start == 0) {
        p = src - pad;
        for (i = 1; i <= pad; i++) {
            memcpy(p - i_src * i, p, (width + 2 * pad) * sizeof(pel_t));
        }
    }

    if (start + rows == height) {
        p = src + i_src * (height - 1) - pad;
        for (i = 1; i <= pad; i++) {
            memcpy(p + i_src * i, p, (width + 2 * pad) * sizeof(pel_t));
        }
    }
}

void padding_rows_sse128_10bit(pel_t *src, int i_src, int width, int height, int start, int rows, int pad)
{
    int i, j;
    pel_t *p, *p1, *p2;
    int pad_lr = pad + 16 - (pad & 0xF);
    start = max(start, 0);

    if (start + rows > height) {
        rows = height - start;
    }

    p = src + start * i_src;

    // left & right
    for (i = 0; i < rows; i++) {
        __m128i Val1 = _mm_set1_epi16((i16s_t)p[0]);
        __m128i Val2 = _mm_set1_epi16((i16s_t)p[width - 1]);
        p1 = p - pad_lr;
        p2 = p + width;
        for (j = 0; j < pad_lr; j += 8) {
            _mm_storeu_si128((__m128i*)(p1 + j), Val1);
            _mm_storeu_si128((__m128i*)(p2 + j), Val2);
        }

        p += i_src;
    }

    if (start == 0) {
        p = src - pad;
        for (i = 1; i <= pad; i++) {
            memcpy(p - i_src * i, p, (width + 2 * pad) * sizeof(pel_t));
        }
    }

    if (start + rows == height) {
        p = src + i_src * (height - 1) - pad;
        for (i = 1; i <= pad; i++) {
            memcpy(p + i_src * i, p, (width + 2 * pad) * sizeof(pel_t));
        }
    }
}

void padding_rows_lr_sse128(pel_t *src, int i_src, int width, int height, int start, int rows, int pad)
{
    int i, j;
    pel_t *p, *p1, *p2;

    start = max(start, 0);

    if (start + rows > height) {
        rows = height - start;
    }

    p = src + start * i_src;

    pad = pad + 16 - (pad & 0xF);

    // left & right
    for (i = 0; i < rows; i++) {
        __m128i Val1 = _mm_set1_epi8((char)p[0]);
        __m128i Val2 = _mm_set1_epi8((char)p[width - 1]);
        p1 = p - pad;
        p2 = p + width;
        for (j = 0; j < pad; j += 16) {
            _mm_storeu_si128((__m128i*)(p1 + j), Val1);
            _mm_storeu_si128((__m128i*)(p2 + j), Val2);
        }

        p += i_src;
    }
}

void padding_rows_lr_sse128_10bit(pel_t *src, int i_src, int width, int height, int start, int rows, int pad)
{
    int i, j;
    pel_t *p, *p1, *p2;

    start = max(start, 0);

    if (start + rows > height) {
        rows = height - start;
    }

    p = src + start * i_src;

    pad = pad + 16 - (pad & 0xF);

    // left & right
    for (i = 0; i < rows; i++) {
        __m128i Val1 = _mm_set1_epi16((i16s_t)p[0]);
        __m128i Val2 = _mm_set1_epi16((i16s_t)p[width - 1]);
        p1 = p - pad;
        p2 = p + width;
        for (j = 0; j < pad; j += 8) {
            _mm_storeu_si128((__m128i*)(p1 + j), Val1);
            _mm_storeu_si128((__m128i*)(p2 + j), Val2);
        }

        p += i_src;
    }
}