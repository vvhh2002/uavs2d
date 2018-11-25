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

#include"transform.h"

#define absm(A) ((A)<(0) ? (-(A)):(A))


#define LOT_MAX_WLT_TAP             2           ///< number of wavelet transform tap, (5-3)


static tab_i32s_t tab_trans_core_4[4][4] = {
    {  32,    32,     32,     32 },
    {  42,    17,    -17,    -42 },
    {  32,   -32,    -32,     32 },
    {  17,   -42,     42,    -17 }
};
static tab_i32s_t tab_trans_core_8[8][8] = {
    {  32,    32,     32,     32,     32,     32,     32,     32    },
    {  44,    38,     25,      9,     -9,    -25,    -38,    -44    },
    {  42,    17,    -17,    -42,    -42,    -17,     17,     42    },
    {  38,    -9,    -44,    -25,     25,     44,      9,    -38    },
    {  32,   -32,    -32,     32,     32,    -32,    -32,     32    },
    {  25,   -44,      9,     38,    -38,     -9,     44,    -25    },
    {  17,   -42,     42,    -17,    -17,     42,    -42,     17    },
    {   9,   -25,     38,    -44,     44,    -38,     25,     -9    }
};
static tab_i32s_t tab_trans_core_16[16][16] = {
    {  32,    32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32    },
    {  45,    43,     40,     35,     29,     21,     13,      4,     -4,    -13,    -21,    -29,    -35,    -40,    -43,    -45    },
    {  44,    38,     25,      9,     -9,    -25,    -38,    -44,    -44,    -38,    -25,     -9,      9,     25,     38,     44    },
    {  43,    29,      4,    -21,    -40,    -45,    -35,    -13,     13,     35,     45,     40,     21,     -4,    -29,    -43    },
    {  42,    17,    -17,    -42,    -42,    -17,     17,     42,     42,     17,    -17,    -42,    -42,    -17,     17,     42    },
    {  40,     4,    -35,    -43,    -13,     29,     45,     21,    -21,    -45,    -29,     13,     43,     35,     -4,    -40    },
    {  38,    -9,    -44,    -25,     25,     44,      9,    -38,    -38,      9,     44,     25,    -25,    -44,     -9,     38    },
    {  35,   -21,    -43,      4,     45,     13,    -40,    -29,     29,     40,    -13,    -45,     -4,     43,     21,    -35    },
    {  32,   -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32    },
    {  29,   -40,    -13,     45,     -4,    -43,     21,     35,    -35,    -21,     43,      4,    -45,     13,     40,    -29    },
    {  25,   -44,      9,     38,    -38,     -9,     44,    -25,    -25,     44,     -9,    -38,     38,      9,    -44,     25    },
    {  21,   -45,     29,     13,    -43,     35,      4,    -40,     40,     -4,    -35,     43,    -13,    -29,     45,    -21    },
    {  17,   -42,     42,    -17,    -17,     42,    -42,     17,     17,    -42,     42,    -17,    -17,     42,    -42,     17    },
    {  13,   -35,     45,    -40,     21,      4,    -29,     43,    -43,     29,     -4,    -21,     40,    -45,     35,    -13    },
    {   9,   -25,     38,    -44,     44,    -38,     25,     -9,     -9,     25,    -38,     44,    -44,     38,    -25,      9    },
    {   4,   -13,     21,    -29,     35,    -40,     43,    -45,     45,    -43,     40,    -35,     29,    -21,     13,     -4    }
};
static tab_i32s_t tab_trans_core_32[32][32] = {
    {  32,    32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32    },
    {  45,    45,     44,     43,     41,     39,     36,     34,     30,     27,     23,     19,     15,     11,      7,      2,     -2,     -7,    -11,    -15,    -19,    -23,    -27,    -30,    -34,    -36,    -39,    -41,    -43,    -44,    -45,    -45    },
    {  45,    43,     40,     35,     29,     21,     13,      4,     -4,    -13,    -21,    -29,    -35,    -40,    -43,    -45,    -45,    -43,    -40,    -35,    -29,    -21,    -13,     -4,      4,     13,     21,     29,     35,     40,     43,     45    },
    {  45,    41,     34,     23,     11,     -2,    -15,    -27,    -36,    -43,    -45,    -44,    -39,    -30,    -19,     -7,      7,     19,     30,     39,     44,     45,     43,     36,     27,     15,      2,    -11,    -23,    -34,    -41,    -45    },
    {  44,    38,     25,      9,     -9,    -25,    -38,    -44,    -44,    -38,    -25,     -9,      9,     25,     38,     44,     44,     38,     25,      9,     -9,    -25,    -38,    -44,    -44,    -38,    -25,     -9,      9,     25,     38,     44    },
    {  44,    34,     15,     -7,    -27,    -41,    -45,    -39,    -23,     -2,     19,     36,     45,     43,     30,     11,    -11,    -30,    -43,    -45,    -36,    -19,      2,     23,     39,     45,     41,     27,      7,    -15,    -34,    -44    },
    {  43,    29,      4,    -21,    -40,    -45,    -35,    -13,     13,     35,     45,     40,     21,     -4,    -29,    -43,    -43,    -29,     -4,     21,     40,     45,     35,     13,    -13,    -35,    -45,    -40,    -21,      4,     29,     43    },
    {  43,    23,     -7,    -34,    -45,    -36,    -11,     19,     41,     44,     27,     -2,    -30,    -45,    -39,    -15,     15,     39,     45,     30,      2,    -27,    -44,    -41,    -19,     11,     36,     45,     34,      7,    -23,    -43    },
    {  42,    17,    -17,    -42,    -42,    -17,     17,     42,     42,     17,    -17,    -42,    -42,    -17,     17,     42,     42,     17,    -17,    -42,    -42,    -17,     17,     42,     42,     17,    -17,    -42,    -42,    -17,     17,     42    },
    {  41,    11,    -27,    -45,    -30,      7,     39,     43,     15,    -23,    -45,    -34,      2,     36,     44,     19,    -19,    -44,    -36,     -2,     34,     45,     23,    -15,    -43,    -39,     -7,     30,     45,     27,    -11,    -41    },
    {  40,     4,    -35,    -43,    -13,     29,     45,     21,    -21,    -45,    -29,     13,     43,     35,     -4,    -40,    -40,     -4,     35,     43,     13,    -29,    -45,    -21,     21,     45,     29,    -13,    -43,    -35,      4,     40    },
    {  39,    -2,    -41,    -36,      7,     43,     34,    -11,    -44,    -30,     15,     45,     27,    -19,    -45,    -23,     23,     45,     19,    -27,    -45,    -15,     30,     44,     11,    -34,    -43,     -7,     36,     41,      2,    -39    },
    {  38,    -9,    -44,    -25,     25,     44,      9,    -38,    -38,      9,     44,     25,    -25,    -44,     -9,     38,     38,     -9,    -44,    -25,     25,     44,      9,    -38,    -38,      9,     44,     25,    -25,    -44,     -9,     38    },
    {  36,   -15,    -45,    -11,     39,     34,    -19,    -45,     -7,     41,     30,    -23,    -44,     -2,     43,     27,    -27,    -43,      2,     44,     23,    -30,    -41,      7,     45,     19,    -34,    -39,     11,     45,     15,    -36    },
    {  35,   -21,    -43,      4,     45,     13,    -40,    -29,     29,     40,    -13,    -45,     -4,     43,     21,    -35,    -35,     21,     43,     -4,    -45,    -13,     40,     29,    -29,    -40,     13,     45,      4,    -43,    -21,     35    },
    {  34,   -27,    -39,     19,     43,    -11,    -45,      2,     45,      7,    -44,    -15,     41,     23,    -36,    -30,     30,     36,    -23,    -41,     15,     44,     -7,    -45,     -2,     45,     11,    -43,    -19,     39,     27,    -34    },
    {  32,   -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32    },
    {  30,   -36,    -23,     41,     15,    -44,     -7,     45,     -2,    -45,     11,     43,    -19,    -39,     27,     34,    -34,    -27,     39,     19,    -43,    -11,     45,      2,    -45,      7,     44,    -15,    -41,     23,     36,    -30    },
    {  29,   -40,    -13,     45,     -4,    -43,     21,     35,    -35,    -21,     43,      4,    -45,     13,     40,    -29,    -29,     40,     13,    -45,      4,     43,    -21,    -35,     35,     21,    -43,     -4,     45,    -13,    -40,     29    },
    {  27,   -43,     -2,     44,    -23,    -30,     41,      7,    -45,     19,     34,    -39,    -11,     45,    -15,    -36,     36,     15,    -45,     11,     39,    -34,    -19,     45,     -7,    -41,     30,     23,    -44,      2,     43,    -27    },
    {  25,   -44,      9,     38,    -38,     -9,     44,    -25,    -25,     44,     -9,    -38,     38,      9,    -44,     25,     25,    -44,      9,     38,    -38,     -9,     44,    -25,    -25,     44,     -9,    -38,     38,      9,    -44,     25    },
    {  23,   -45,     19,     27,    -45,     15,     30,    -44,     11,     34,    -43,      7,     36,    -41,      2,     39,    -39,     -2,     41,    -36,     -7,     43,    -34,    -11,     44,    -30,    -15,     45,    -27,    -19,     45,    -23    },
    {  21,   -45,     29,     13,    -43,     35,      4,    -40,     40,     -4,    -35,     43,    -13,    -29,     45,    -21,    -21,     45,    -29,    -13,     43,    -35,     -4,     40,    -40,      4,     35,    -43,     13,     29,    -45,     21    },
    {  19,   -44,     36,     -2,    -34,     45,    -23,    -15,     43,    -39,      7,     30,    -45,     27,     11,    -41,     41,    -11,    -27,     45,    -30,     -7,     39,    -43,     15,     23,    -45,     34,      2,    -36,     44,    -19    },
    {  17,   -42,     42,    -17,    -17,     42,    -42,     17,     17,    -42,     42,    -17,    -17,     42,    -42,     17,     17,    -42,     42,    -17,    -17,     42,    -42,     17,     17,    -42,     42,    -17,    -17,     42,    -42,     17    },
    {  15,   -39,     45,    -30,      2,     27,    -44,     41,    -19,    -11,     36,    -45,     34,     -7,    -23,     43,    -43,     23,      7,    -34,     45,    -36,     11,     19,    -41,     44,    -27,     -2,     30,    -45,     39,    -15    },
    {  13,   -35,     45,    -40,     21,      4,    -29,     43,    -43,     29,     -4,    -21,     40,    -45,     35,    -13,    -13,     35,    -45,     40,    -21,     -4,     29,    -43,     43,    -29,      4,     21,    -40,     45,    -35,     13    },
    {  11,   -30,     43,    -45,     36,    -19,     -2,     23,    -39,     45,    -41,     27,     -7,    -15,     34,    -44,     44,    -34,     15,      7,    -27,     41,    -45,     39,    -23,      2,     19,    -36,     45,    -43,     30,    -11    },
    {   9,   -25,     38,    -44,     44,    -38,     25,     -9,     -9,     25,    -38,     44,    -44,     38,    -25,      9,      9,    -25,     38,    -44,     44,    -38,     25,     -9,     -9,     25,    -38,     44,    -44,     38,    -25,      9    },
    {   7,   -19,     30,    -39,     44,    -45,     43,    -36,     27,    -15,      2,     11,    -23,     34,    -41,     45,    -45,     41,    -34,     23,    -11,     -2,     15,    -27,     36,    -43,     45,    -44,     39,    -30,     19,     -7    },
    {   4,   -13,     21,    -29,     35,    -40,     43,    -45,     45,    -43,     40,    -35,     29,    -21,     13,     -4,     -4,     13,    -21,     29,    -35,     40,    -43,     45,    -45,     43,    -40,     35,    -29,     21,    -13,      4    },
    {   2,    -7,     11,    -15,     19,    -23,     27,    -30,     34,    -36,     39,    -41,     43,    -44,     45,    -45,     45,    -45,     44,    -43,     41,    -39,     36,    -34,     30,    -27,     23,    -19,     15,    -11,      7,     -2    }
};

static tab_i16s_t tab_c4_trans[4][4] = {

    {    34,    58,    72,     81,},
    {    77,    69,    -7,    -75,},
    {    79,   -33,   -75,     58,},
    {    55,   -84,    73,    -28,}

};

static tab_i16s_t tab_c8_trans[4][4] = {
    {   123,   -35,    -8,    -3,},
    {   -32,  -120,    30,    10,},
    {    14,    25,   123,   -22,},
    {     8,    13,    19,   126,},
};

/////////////////////////////////////////////////////////////////////////////
/// local function declaration
/////////////////////////////////////////////////////////////////////////////
static void partialButterflyInverse4x4(coef_t *src, coef_t *dst, int shift, int clip_depth)
{
    int j;
    int E[2], O[2];

    int add = shift ? (1 << (shift - 1)) : 0;
    int max_val = (1 << (clip_depth - 1)) - 1;
    int min_val = -max_val - 1;

    for (j = 0; j < 4; j++) {
        E[0] = tab_trans_core_4[0][0] * src[0] + tab_trans_core_4[2][0] * src[8];
        E[1] = tab_trans_core_4[2][0] * src[0] - tab_trans_core_4[0][0] * src[8];
        O[0] = tab_trans_core_4[1][0] * src[4] + tab_trans_core_4[3][0] * src[12];
        O[1] = tab_trans_core_4[3][0] * src[4] - tab_trans_core_4[1][0] * src[12];

        dst[0] =  (coef_t)COM_CLIP3(min_val, max_val, (E[0] + O[0] + add) >> shift);
        dst[2] =  (coef_t)COM_CLIP3(min_val, max_val, (E[1] - O[1] + add) >> shift);
        dst[1] =  (coef_t)COM_CLIP3(min_val, max_val, (E[1] + O[1] + add) >> shift);
        dst[3] =  (coef_t)COM_CLIP3(min_val, max_val, (E[0] - O[0] + add) >> shift);
        dst += 4;
        src += 1;
    }
}

static void partialButterflyInverse4x16(coef_t *src, coef_t *dst, int shift, int clip_depth)
{
    int j;
    int E[2], O[2];

    int add = shift ? (1 << (shift - 1)) : 0;
    int max_val = (1 << (clip_depth - 1)) - 1;
    int min_val = -max_val - 1;

    for (j = 0; j < 16; j++) {
        E[0] = tab_trans_core_4[0][0] * src[0 * 16] + tab_trans_core_4[2][0] * src[2 * 16];
        E[1] = tab_trans_core_4[2][0] * src[0 * 16] - tab_trans_core_4[0][0] * src[2 * 16];
        O[0] = tab_trans_core_4[1][0] * src[1 * 16] + tab_trans_core_4[3][0] * src[3 * 16];
        O[1] = tab_trans_core_4[3][0] * src[1 * 16] - tab_trans_core_4[1][0] * src[3 * 16];

        dst[0] = (coef_t) COM_CLIP3(min_val, max_val, (E[0] + O[0] + add) >> shift);
        dst[2] = (coef_t) COM_CLIP3(min_val, max_val, (E[1] - O[1] + add) >> shift);
        dst[1] = (coef_t) COM_CLIP3(min_val, max_val, (E[1] + O[1] + add) >> shift);
        dst[3] = (coef_t) COM_CLIP3(min_val, max_val, (E[0] - O[0] + add) >> shift);
        dst += 4;
        src += 1;
    }
}

static void partialButterflyInverse8x8(coef_t *src, coef_t *dst, int shift, int clip_depth)
{
    int j, k;
    int E[4], O[4];
    int EE[2], EO[2];

    int add = shift ? (1 << (shift - 1)) : 0;
    int max_val = (1 << (clip_depth - 1)) - 1;
    int min_val = -max_val - 1;

    for (j = 0; j < 8; j++) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        for (k = 0; k < 4; k++) {
            O[k] = tab_trans_core_8[ 1][k] * src[8] + tab_trans_core_8[ 3][k] * src[24] + tab_trans_core_8[ 5][k] * src[40] + tab_trans_core_8[ 7][k] * src[56];
        }

        EO[0] = tab_trans_core_8[2][0] * src[ 16 ] + tab_trans_core_8[6][0] * src[ 48 ];
        EO[1] = tab_trans_core_8[2][1] * src[ 16 ] + tab_trans_core_8[6][1] * src[ 48 ];
        EE[0] = tab_trans_core_8[0][0] * src[ 0 ] + tab_trans_core_8[4][0] * src[ 32 ];
        EE[1] = tab_trans_core_8[0][1] * src[ 0 ] + tab_trans_core_8[4][1] * src[ 32 ];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        E[0] = EE[0] + EO[0];
        E[3] = EE[0] - EO[0];
        E[1] = EE[1] + EO[1];
        E[2] = EE[1] - EO[1];
        for (k = 0; k < 4; k++) {
            dst[k] = (coef_t) COM_CLIP3(min_val, max_val, (E[k] + O[k] + add) >> shift);
            dst[k + 4] = (coef_t) COM_CLIP3(min_val, max_val, (E[3 - k] - O[3 - k] + add) >> shift);
        }
        dst += 8;
        src += 1;
    }
}

static void partialButterflyInverse8x32(coef_t *src, coef_t *dst, int shift, int clip_depth)
{
    int j, k;
    int E[4], O[4];
    int EE[2], EO[2];

    int add = shift ? (1 << (shift - 1)) : 0;
    int max_val = (1 << (clip_depth - 1)) - 1;
    int min_val = -max_val - 1;

    for (j = 0; j < 32; j++) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        for (k = 0; k < 4; k++) {
            O[k] = tab_trans_core_8[1][k] * src[1 * 32] + tab_trans_core_8[3][k] * src[3 * 32] + tab_trans_core_8[5][k] * src[5 * 32] + tab_trans_core_8[7][k] * src[7 * 32];
        }

        EO[0] = tab_trans_core_8[2][0] * src[2 * 32] + tab_trans_core_8[6][0] * src[6 * 32];
        EO[1] = tab_trans_core_8[2][1] * src[2 * 32] + tab_trans_core_8[6][1] * src[6 * 32];
        EE[0] = tab_trans_core_8[0][0] * src[0 * 32] + tab_trans_core_8[4][0] * src[4 * 32];
        EE[1] = tab_trans_core_8[0][1] * src[0 * 32] + tab_trans_core_8[4][1] * src[4 * 32];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        E[0] = EE[0] + EO[0];
        E[3] = EE[0] - EO[0];
        E[1] = EE[1] + EO[1];
        E[2] = EE[1] - EO[1];
        for (k = 0; k < 4; k++) {
            dst[k] = (coef_t) COM_CLIP3(min_val, max_val, (E[k] + O[k] + add) >> shift);
            dst[k + 4] = (coef_t) COM_CLIP3(min_val, max_val, (E[3 - k] - O[3 - k] + add) >> shift);
        }
        dst += 8;
        src += 1;
    }
}

static void partialButterflyInverse16x16(coef_t *src, coef_t *dst, int shift, int clip_depth)
{
    int j, k;
    int E[8], O[8];
    int EE[4], EO[4];
    int EEE[2], EEO[2];

    int add = shift ? (1 << (shift - 1)) : 0;
    int max_val = (1 << (clip_depth - 1)) - 1;
    int min_val = -max_val - 1;

    for (j = 0; j < 16; j++) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        for (k = 0; k < 8; k++) {
            O[k] = tab_trans_core_16[1][k] * src[1 * 16] + tab_trans_core_16[ 3][k] * src[ 3 * 16] + tab_trans_core_16[ 5][k] * src[ 5 * 16] + tab_trans_core_16[ 7][k] * src[7 * 16] +
                   tab_trans_core_16[9][k] * src[9 * 16] + tab_trans_core_16[11][k] * src[11 * 16] + tab_trans_core_16[13][k] * src[13 * 16] + tab_trans_core_16[15][k] * src[15 * 16];
        }
        for (k = 0; k < 4; k++) {
            EO[k] = tab_trans_core_16[2][k] * src[2 * 16] + tab_trans_core_16[6][k] * src[6 * 16] + tab_trans_core_16[10][k] * src[10 * 16] + tab_trans_core_16[14][k] * src[14 * 16];
        }
        EEO[0] = tab_trans_core_16[4][0] * src[4 * 16] + tab_trans_core_16[12][0] * src[12 * 16];
        EEE[0] = tab_trans_core_16[0][0] * src[0 * 16] + tab_trans_core_16[ 8][0] * src[ 8 * 16];
        EEO[1] = tab_trans_core_16[4][1] * src[4 * 16] + tab_trans_core_16[12][1] * src[12 * 16];
        EEE[1] = tab_trans_core_16[0][1] * src[0 * 16] + tab_trans_core_16[ 8][1] * src[ 8 * 16];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        for (k = 0; k < 2; k++) {
            EE[k] = EEE[k] + EEO[k];
            EE[k + 2] = EEE[1 - k] - EEO[1 - k];
        }
        for (k = 0; k < 4; k++) {
            E[k] = EE[k] + EO[k];
            E[k + 4] = EE[3 - k] - EO[3 - k];
        }
        for (k = 0; k < 8; k++) {
            dst[k] = (coef_t) COM_CLIP3(min_val, max_val, (E[k] + O[k] + add) >> shift);
            dst[k + 8] = (coef_t) COM_CLIP3(min_val, max_val, (E[7 - k] - O[7 - k] + add) >> shift);
        }
        dst += 16;
        src += 1;
    }
}

static void partialButterflyInverse16x4(coef_t *src, coef_t *dst, int shift, int clip_depth)
{
    int j, k;
    int E[8], O[8];
    int EE[4], EO[4];
    int EEE[2], EEO[2];

    int add = shift ? (1 << (shift - 1)) : 0;
    int max_val = (1 << (clip_depth - 1)) - 1;
    int min_val = -max_val - 1;

    for (j = 0; j < 4; j++) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        for (k = 0; k < 8; k++) {
            O[k] = tab_trans_core_16[1][k] * src[1 * 4] + tab_trans_core_16[ 3][k] * src[ 3 * 4] + tab_trans_core_16[ 5][k] * src[ 5 * 4] + tab_trans_core_16[ 7][k] * src[ 7 * 4] +
                   tab_trans_core_16[9][k] * src[9 * 4] + tab_trans_core_16[11][k] * src[11 * 4] + tab_trans_core_16[13][k] * src[13 * 4] + tab_trans_core_16[15][k] * src[15 * 4];
        }
        for (k = 0; k < 4; k++) {
            EO[k] = tab_trans_core_16[2][k] * src[2 * 4] + tab_trans_core_16[6][k] * src[6 * 4] + tab_trans_core_16[10][k] * src[10 * 4] + tab_trans_core_16[14][k] * src[14 * 4];
        }
        EEO[0] = tab_trans_core_16[4][0] * src[4 * 4] + tab_trans_core_16[12][0] * src[12 * 4];
        EEE[0] = tab_trans_core_16[0][0] * src[0 * 4] + tab_trans_core_16[ 8][0] * src[ 8 * 4];
        EEO[1] = tab_trans_core_16[4][1] * src[4 * 4] + tab_trans_core_16[12][1] * src[12 * 4];
        EEE[1] = tab_trans_core_16[0][1] * src[0 * 4] + tab_trans_core_16[ 8][1] * src[ 8 * 4];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        for (k = 0; k < 2; k++) {
            EE[k] = EEE[k] + EEO[k];
            EE[k + 2] = EEE[1 - k] - EEO[1 - k];
        }
        for (k = 0; k < 4; k++) {
            E[k] = EE[k] + EO[k];
            E[k + 4] = EE[3 - k] - EO[3 - k];
        }
        for (k = 0; k < 8; k++) {
            dst[k] = (coef_t) COM_CLIP3(min_val, max_val, (E[k] + O[k] + add) >> shift);
            dst[k + 8] = (coef_t) COM_CLIP3(min_val, max_val, (E[7 - k] - O[7 - k] + add) >> shift);
        }
        dst += 16;
        src += 1;
    }
}

static void partialButterflyInverse32x32(coef_t *src, coef_t *dst, int shift, int clip_depth)
{
    int j, k;
    int E[16], O[16];
    int EE[8], EO[8];
    int EEE[4], EEO[4];
    int EEEE[2], EEEO[2];

    int add = shift ? (1 << (shift - 1)) : 0;
    int max_val = (1 << (clip_depth - 1)) - 1;
    int min_val = -max_val - 1;

    for (j = 0; j < 32; j++) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        for (k = 0; k < 16; k++) {
            O[k] = tab_trans_core_32[ 1][k] * src[ 1 * 32] + tab_trans_core_32[ 3][k] * src[ 3 * 32] + tab_trans_core_32[ 5][k] * src[ 5 * 32] + tab_trans_core_32[ 7][k] * src[ 7 * 32] +
                   tab_trans_core_32[ 9][k] * src[ 9 * 32] + tab_trans_core_32[11][k] * src[11 * 32] + tab_trans_core_32[13][k] * src[13 * 32] + tab_trans_core_32[15][k] * src[15 * 32] +
                   tab_trans_core_32[17][k] * src[17 * 32] + tab_trans_core_32[19][k] * src[19 * 32] + tab_trans_core_32[21][k] * src[21 * 32] + tab_trans_core_32[23][k] * src[23 * 32] +
                   tab_trans_core_32[25][k] * src[25 * 32] + tab_trans_core_32[27][k] * src[27 * 32] + tab_trans_core_32[29][k] * src[29 * 32] + tab_trans_core_32[31][k] * src[31 * 32];
        }
        for (k = 0; k < 8; k++) {
            EO[k] = tab_trans_core_32[ 2][k] * src[ 2 * 32] + tab_trans_core_32[ 6][k] * src[ 6 * 32] + tab_trans_core_32[10][k] * src[10 * 32] + tab_trans_core_32[14][k] * src[14 * 32] +
                    tab_trans_core_32[18][k] * src[18 * 32] + tab_trans_core_32[22][k] * src[22 * 32] + tab_trans_core_32[26][k] * src[26 * 32] + tab_trans_core_32[30][k] * src[30 * 32];
        }
        for (k = 0; k < 4; k++) {
            EEO[k] = tab_trans_core_32[4][k] * src[4 * 32] + tab_trans_core_32[12][k] * src[12 * 32] + tab_trans_core_32[20][k] * src[20 * 32] + tab_trans_core_32[28][k] * src[28 * 32];
        }
        EEEO[0] = tab_trans_core_32[8][0] * src[8 * 32] + tab_trans_core_32[24][0] * src[24 * 32];
        EEEO[1] = tab_trans_core_32[8][1] * src[8 * 32] + tab_trans_core_32[24][1] * src[24 * 32];
        EEEE[0] = tab_trans_core_32[0][0] * src[0 * 32] + tab_trans_core_32[16][0] * src[16 * 32];
        EEEE[1] = tab_trans_core_32[0][1] * src[0 * 32] + tab_trans_core_32[16][1] * src[16 * 32];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        EEE[0] = EEEE[0] + EEEO[0];
        EEE[3] = EEEE[0] - EEEO[0];
        EEE[1] = EEEE[1] + EEEO[1];
        EEE[2] = EEEE[1] - EEEO[1];
        for (k = 0; k < 4; k++) {
            EE[k] = EEE[k] + EEO[k];
            EE[k + 4] = EEE[3 - k] - EEO[3 - k];
        }
        for (k = 0; k < 8; k++) {
            E[k] = EE[k] + EO[k];
            E[k + 8] = EE[7 - k] - EO[7 - k];
        }
        for (k = 0; k < 16; k++) {
            dst[k] = (coef_t) COM_CLIP3(min_val, max_val, (E[k] + O[k] + add) >> shift);
            dst[k + 16] = (coef_t) COM_CLIP3(min_val, max_val, (E[15 - k] - O[15 - k] + add) >> shift);
        }
        dst += 32;
        src += 1;
    }
}

static void partialButterflyInverse32x8(coef_t *src, coef_t *dst, int shift, int clip_depth)
{
    int j, k;
    int E[16], O[16];
    int EE[8], EO[8];
    int EEE[4], EEO[4];
    int EEEE[2], EEEO[2];

    int add = shift ? (1 << (shift - 1)) : 0;
    int max_val = (1 << (clip_depth - 1)) - 1;
    int min_val = -max_val - 1;

    for (j = 0; j < 8; j++) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        for (k = 0; k < 16; k++) {
            O[k] = tab_trans_core_32[ 1][k] * src[ 1 * 8] + tab_trans_core_32[ 3][k] * src[ 3 * 8] + tab_trans_core_32[ 5][k] * src[ 5 * 8] + tab_trans_core_32[ 7][k] * src[ 7 * 8] +
                   tab_trans_core_32[ 9][k] * src[ 9 * 8] + tab_trans_core_32[11][k] * src[11 * 8] + tab_trans_core_32[13][k] * src[13 * 8] + tab_trans_core_32[15][k] * src[15 * 8] +
                   tab_trans_core_32[17][k] * src[17 * 8] + tab_trans_core_32[19][k] * src[19 * 8] + tab_trans_core_32[21][k] * src[21 * 8] + tab_trans_core_32[23][k] * src[23 * 8] +
                   tab_trans_core_32[25][k] * src[25 * 8] + tab_trans_core_32[27][k] * src[27 * 8] + tab_trans_core_32[29][k] * src[29 * 8] + tab_trans_core_32[31][k] * src[31 * 8];
        }
        for (k = 0; k < 8; k++) {
            EO[k] = tab_trans_core_32[ 2][k] * src[ 2 * 8] + tab_trans_core_32[ 6][k] * src[ 6 * 8] + tab_trans_core_32[10][k] * src[10 * 8] + tab_trans_core_32[14][k] * src[14 * 8] +
                    tab_trans_core_32[18][k] * src[18 * 8] + tab_trans_core_32[22][k] * src[22 * 8] + tab_trans_core_32[26][k] * src[26 * 8] + tab_trans_core_32[30][k] * src[30 * 8];
        }
        for (k = 0; k < 4; k++) {
            EEO[k] = tab_trans_core_32[4][k] * src[4 * 8] + tab_trans_core_32[12][k] * src[12 * 8] + tab_trans_core_32[20][k] * src[20 * 8] + tab_trans_core_32[28][k] * src[28 * 8];
        }
        EEEO[0] = tab_trans_core_32[8][0] * src[8 * 8] + tab_trans_core_32[24][0] * src[24 * 8];
        EEEO[1] = tab_trans_core_32[8][1] * src[8 * 8] + tab_trans_core_32[24][1] * src[24 * 8];
        EEEE[0] = tab_trans_core_32[0][0] * src[0 * 8] + tab_trans_core_32[16][0] * src[16 * 8];
        EEEE[1] = tab_trans_core_32[0][1] * src[0 * 8] + tab_trans_core_32[16][1] * src[16 * 8];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        EEE[0] = EEEE[0] + EEEO[0];
        EEE[3] = EEEE[0] - EEEO[0];
        EEE[1] = EEEE[1] + EEEO[1];
        EEE[2] = EEEE[1] - EEEO[1];
        for (k = 0; k < 4; k++) {
            EE[k] = EEE[k] + EEO[k];
            EE[k + 4] = EEE[3 - k] - EEO[3 - k];
        }
        for (k = 0; k < 8; k++) {
            E[k] = EE[k] + EO[k];
            E[k + 8] = EE[7 - k] - EO[7 - k];
        }
        for (k = 0; k < 16; k++) {
            dst[k] = (coef_t) COM_CLIP3(min_val, max_val, (E[k] + O[k] + add) >> shift);
            dst[k + 16] = (coef_t) COM_CLIP3(min_val, max_val, (E[15 - k] - O[15 - k] + add) >> shift);
        }
        dst += 32;
        src += 1;
    }
}

static void inv_wavelet_B64(coef_t *blk)
{
    coef_t iEBuff[64 + LOT_MAX_WLT_TAP * 2];
    coef_t *pExt = &iEBuff[LOT_MAX_WLT_TAP];
    int  i, n, x, y;
    int  ySize, y2, x2;
    coef_t pBuff[64 * 64];

    for (i = 0; i < 32; i++) {
        memcpy(&pBuff[i * 64], &blk[i * 32], 32 * sizeof(coef_t));
    }

    // step #1: vertical transform
    for (x = 0; x < 64; x++) {
        // copy
        for (y = 0, ySize = 0; y < 32; y++, ySize += 64) {
            y2 = y << 1;
            pExt[y2] = pBuff[x + ySize];
        }

        // reflection
        pExt[64] = pExt[64 - 2];

        // filtering (even pixel)
        for (n = 0; n <= 64; n += 2) {
            pExt[n] >>= 1;
        }

        // filtering (odd pixel)
        for (n = 1; n <= 64 - 1; n += 2) {
            pExt[n] = (pExt[n - 1] + pExt[n + 1]) >> 1;
        }

        // copy
        for (y = 0, ySize = 0; y < 64; y++, ySize += 64) {
            pBuff[x + ySize] = pExt[y];
        }
    }

    // step #2: horizontal transform
    for (y = 0, ySize = 0; y < 64; y++, ySize += 64) {
        // copy
        for (x = 0; x < 32; x++) {
            x2 = x << 1;
            pExt[x2] = pBuff[ySize + x];
        }

        // reflection
        pExt[64] = pExt[64 - 2];

        // filtering (odd pixel)
        for (n = 1; n <= 64 - 1; n += 2) {
            pExt[n] = (pExt[n - 1] + pExt[n + 1]) >> 1;
        }

        // copy
        memcpy(&pBuff[ySize], pExt, sizeof(coef_t)* 64);
    }

    memcpy(blk, pBuff, 64 * 64 * sizeof(coef_t));
}

static void inv_wavelet_NSQT_Hor(coef_t *blk)
{
    coef_t iEBuff[64 + LOT_MAX_WLT_TAP * 2];
    coef_t *pExt = &iEBuff[LOT_MAX_WLT_TAP];
    int  i, n, x, y;
    int  ySize, y2, x2;
    coef_t pBuff[64 * 16];

    for (i = 0; i < 8; i++) {
        memcpy(&pBuff[i * 64], &blk[i * 32], 32 * sizeof(coef_t));
    }


    // step #1: vertical transform
    for (x = 0; x < 32; x++) {
        // copy
        for (y = 0, ySize = 0; y < 8; y++, ySize += 64) {
            y2 = y << 1;
            pExt[y2] = pBuff[x + ySize];
        }

        // reflection
        pExt[16] = pExt[16 - 2];

        // filtering (even pixel)
        for (n = 0; n <= 16; n += 2) {
            pExt[n] >>= 1;
        }

        // filtering (odd pixel)
        for (n = 1; n <= 16 - 1; n += 2) {
            pExt[n] = (pExt[n - 1] + pExt[n + 1]) >> 1;
        }

        // copy
        for (y = 0, ySize = 0; y < 16; y++, ySize += 64) {
            pBuff[x + ySize] = pExt[y];
        }
    }

    // step #2: horizontal transform
    for (y = 0, ySize = 0; y < 16; y++, ySize += 64) {
        // copy
        for (x = 0; x < 32; x++) {
            x2 = x << 1;
            pExt[x2] = pBuff[ySize + x];
        }

        // reflection
        pExt[64] = pExt[64 - 2];

        // filtering (odd pixel)
        for (n = 1; n <= 64 - 1; n += 2) {
            pExt[n] = (pExt[n - 1] + pExt[n + 1]) >> 1;
        }

        // copy
        memcpy(&pBuff[ySize], pExt, sizeof(coef_t)* 64);
    }

    for (i = 0; i < 16; i++) {
        memcpy(&blk[i * 64], &pBuff[i * 64], 64 * sizeof(coef_t));
    }
}

static void inv_wavelet_NSQT_Ver(coef_t *blk)
{
    coef_t iEBuff[64 + LOT_MAX_WLT_TAP * 2];
    coef_t *pExt = &iEBuff[LOT_MAX_WLT_TAP];
    int  i, n, x, y;
    int  ySize, y2, x2;
    coef_t pBuff[16 * 64];

    for (i = 0; i < 32; i++) {
        memcpy(&pBuff[i * 16], &blk[i * 8], 8 * sizeof(coef_t));
    }

    // step #1: vertical transform
    for (x = 0; x < 8; x++) {
        // copy
        for (y = 0, ySize = 0; y < 32; y++, ySize += 16) {
            y2 = y << 1;
            pExt[y2] = pBuff[x + ySize];
        }

        // reflection
        pExt[64] = pExt[64 - 2];

        // filtering (even pixel)
        for (n = 0; n <= 64; n += 2) {
            pExt[n] >>= 1;
        }

        // filtering (odd pixel)
        for (n = 1; n <= 64 - 1; n += 2) {
            pExt[n] = (pExt[n - 1] + pExt[n + 1]) >> 1;
        }

        // copy
        for (y = 0, ySize = 0; y < 64; y++, ySize += 16) {
            pBuff[x + ySize] = pExt[y];
        }
    }

    // step #2: horizontal transform
    for (y = 0, ySize = 0; y < 64; y++, ySize += 16) {
        // copy
        for (x = 0; x < 8; x++) {
            x2 = x << 1;
            pExt[x2] = pBuff[ySize + x];
        }

        // reflection
        pExt[16] = pExt[16 - 2];

        // filtering (odd pixel)
        for (n = 1; n <= 16 - 1; n += 2) {
            pExt[n] = (pExt[n - 1] + pExt[n + 1]) >> 1;
        }

        // copy
        memcpy(&pBuff[ySize], pExt, sizeof(coef_t)* 16);
    }

    for (i = 0; i < 64; i++) {
        memcpy(&blk[i * 16], &pBuff[i * 16], 16 * sizeof(coef_t));
    }
}
// Functions for Secondary Transforms
static void inv_2nd_trans_hor(coef_t *blk, int i_blk, int shift, const i16s_t coef[4][4])
{
    int i, j, sum;
    int rnd_factor = shift == 0 ? 0 : 1 << (shift - 1);
    coef_t tmpSrc[4][4];
    coef_t *src = blk;
    coef_t *dst = blk;

    for (i = 0; i < 4; i++) {
        tmpSrc[i][0] = src[0];
        tmpSrc[i][1] = src[1];
        tmpSrc[i][2] = src[2];
        tmpSrc[i][3] = src[3];
        src += i_blk;
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            sum = rnd_factor
                + coef[0][i] * tmpSrc[j][0]
                + coef[1][i] * tmpSrc[j][1]
                + coef[2][i] * tmpSrc[j][2]
                + coef[3][i] * tmpSrc[j][3];

            dst[j * i_blk] = (coef_t) Clip3(-32768, 32767, sum >> shift);
        }
        dst += 1;
    }
}

static void inv_2nd_trans_ver(coef_t *blk, int i_blk, int shift, const i16s_t coef[4][4])
{
    int i, j, sum;
    int rnd_factor = shift == 0 ? 0 : 1 << (shift - 1);
    coef_t tmpSrc[4][4];
    coef_t *src = blk;
    coef_t *dst = blk;

    for (i = 0; i < 4; i++) {
        tmpSrc[i][0] = src[0];
        tmpSrc[i][1] = src[1];
        tmpSrc[i][2] = src[2];
        tmpSrc[i][3] = src[3];
        src += i_blk;
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            sum = rnd_factor
                + coef[0][i] * tmpSrc[0][j]
                + coef[1][i] * tmpSrc[1][j]
                + coef[2][i] * tmpSrc[2][j]
                + coef[3][i] * tmpSrc[3][j];

            dst[j] = Clip3(-32768, 32767, sum >> shift);
        }
        dst += i_blk;
    }
}

static void inv_2nd_trans(coef_t *blk, int i_blk, int shift, int clip_depth, const i16s_t coef[4][4])
{
    int i, j, sum;
    int rnd_factor = 16;
    coef_t tmpSrc[4][4];
    coef_t *src = blk;
    coef_t *dst = blk;
    int max_val = (1 << (clip_depth - 1)) - 1;
    int min_val = -max_val - 1;

    for (i = 0; i < 4; i++) {
        tmpSrc[i][0] = src[0];
        tmpSrc[i][1] = src[1];
        tmpSrc[i][2] = src[2];
        tmpSrc[i][3] = src[3];
        src += i_blk;
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            sum = rnd_factor
                + coef[0][i] * tmpSrc[0][j]
                + coef[1][i] * tmpSrc[1][j]
                + coef[2][i] * tmpSrc[2][j]
                + coef[3][i] * tmpSrc[3][j];

            dst[j] = (coef_t) Clip3(-32768, 32767, sum >> 5);
        }
        dst += i_blk;
    }

    shift += 2;
    rnd_factor = 1 << (shift - 1);
    src = blk;
    dst = blk;

    for (i = 0; i < 4; i++) {
        tmpSrc[i][0] = src[0];
        tmpSrc[i][1] = src[1];
        tmpSrc[i][2] = src[2];
        tmpSrc[i][3] = src[3];
        src += i_blk;
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            sum = rnd_factor
                + coef[0][i] * tmpSrc[j][0]
                + coef[1][i] * tmpSrc[j][1]
                + coef[2][i] * tmpSrc[j][2]
                + coef[3][i] * tmpSrc[j][3];

            dst[j * i_blk] = (coef_t) Clip3(min_val, max_val, sum >> shift);
        }
        dst += 1;
    }
}

static void idct_4x4(coef_t *blk, int shift, int clip)
{
    ALIGNED_16(coef_t trans_tmp[64 * 64]);
    partialButterflyInverse4x4(blk, trans_tmp, 5, 16);
    partialButterflyInverse4x4(trans_tmp, blk, shift, clip);
}

static void idct_8x8(coef_t *blk, int shift, int clip)
{
    ALIGNED_16(coef_t trans_tmp[64 * 64]);
    partialButterflyInverse8x8(blk, trans_tmp, 5, 16);
    partialButterflyInverse8x8(trans_tmp, blk, shift, clip);
}

static void idct_16x16(coef_t *blk, int shift, int clip)
{
    ALIGNED_16(coef_t trans_tmp[64 * 64]);
    partialButterflyInverse16x16(blk, trans_tmp, 5, 16);
    partialButterflyInverse16x16(trans_tmp, blk, shift, clip);
}

static void idct_32x32(coef_t *blk, int shift, int clip)
{
    ALIGNED_16(coef_t trans_tmp[64 * 64]);
    partialButterflyInverse32x32(blk, trans_tmp, 5, 16);
    partialButterflyInverse32x32(trans_tmp, blk, shift, clip);
}

void core_inv_trans(com_rec_t *rec, coef_t *blk, unsigned int trans_BitSize, int blk_id)
{
    int b_intra = IS_INTRA(rec->cu);
    const seq_info_t *seq = rec->seq;
    int secT_enabled = seq->b_secT_enabled;
    int sample_bit_depth = seq->sample_bit_depth;
    int shift, clip, tranSize;
    int isChroma = blk_id <= 3 ? 0 : 1;
    int vt = 1, ht = 1;
    int uiMode = rec->cu_loc_dat->intra_pred_modes[blk_id];
    int block_available_up = rec->block_available_up;
    int block_available_left = rec->block_available_left;

    shift = 20 - sample_bit_depth;
    clip = sample_bit_depth + 1;

    tranSize = (trans_BitSize == B64X64_IN_BIT) ? (1 << (trans_BitSize - 1)) : (1 << trans_BitSize);

    if (trans_BitSize == B64X64_IN_BIT) {
        shift--;
        clip++;
    }

    if (b_intra  &&  secT_enabled && (!isChroma)) {
        if (trans_BitSize >= 3) {
            vt = (uiMode >= 0 && uiMode <= 23);
            ht = (uiMode >= 13 && uiMode <= 32) || (uiMode >= 0 && uiMode <= 2);
            vt = vt && block_available_up;
            ht = ht && block_available_left;
            if (ht) {
                g_funs_handle.inv_2nd_hor(blk, tranSize, 7, tab_c8_trans);
            }
            if (vt) {
                g_funs_handle.inv_2nd_ver(blk, tranSize, 7, tab_c8_trans);
            }
        } else {
            g_funs_handle.inv_2nd(blk, tranSize, shift, clip, tab_c4_trans);
        }
    }

    if (trans_BitSize == B4X4_IN_BIT) {
        if (!b_intra || isChroma || !secT_enabled) {
            g_funs_handle.idct_sqt[0](blk, shift, clip);
        }
    } else if (trans_BitSize == B8X8_IN_BIT) {
        g_funs_handle.idct_sqt[1](blk, shift, clip);
    } else if (trans_BitSize == B16X16_IN_BIT) {
        g_funs_handle.idct_sqt[2](blk, shift, clip);
    } else if (trans_BitSize == B32X32_IN_BIT) {
        g_funs_handle.idct_sqt[3](blk, shift, clip);
    } else if (trans_BitSize == B64X64_IN_BIT) {
        g_funs_handle.idct_sqt[4](blk, shift, clip);
        g_funs_handle.inv_wavelet(blk);
    } 
}

static void idct_16x4(coef_t *blk, int shift, int clip)
{
    ALIGNED_16(coef_t trans_tmp[64 * 64]);
    partialButterflyInverse4x16(blk, trans_tmp, 5, 16);
    partialButterflyInverse16x4(trans_tmp, blk, shift, clip);
}

static void idct_4x16(coef_t *blk, int shift, int clip)
{
    ALIGNED_16(coef_t trans_tmp[64 * 64]);
    partialButterflyInverse16x4(blk, trans_tmp, 5, 16);
    partialButterflyInverse4x16(trans_tmp, blk, shift, clip);
}

static void idct_32x8(coef_t *blk, int shift, int clip)
{
    ALIGNED_16(coef_t trans_tmp[64 * 64]);
    partialButterflyInverse8x32(blk, trans_tmp, 5, 16);
    partialButterflyInverse32x8(trans_tmp, blk, shift, clip);
}

static void idct_8x32(coef_t *blk, int shift, int clip)
{
    ALIGNED_16(coef_t trans_tmp[64 * 64]);
    partialButterflyInverse32x8(blk, trans_tmp, 5, 16);
    partialButterflyInverse8x32(trans_tmp, blk, shift, clip);
}

void core_inv_trans_nsqt(com_rec_t *rec, coef_t *blk, unsigned int trans_BitSize, int blk_id)
{
    int b_intra = IS_INTRA(rec->cu);
    const seq_info_t *seq = rec->seq;
    int secT_enabled = seq->b_secT_enabled;
    int sample_bit_depth = seq->sample_bit_depth;
    int iSizeX=0, iSizeY=0;
    int iHor = 0;

    int shift, clip;
    int iBlockType = rec->cu->cuType;
    int vt = 1, ht = 1;
    int uiMode = rec->cu_loc_dat->intra_pred_modes[blk_id];
    int block_available_up = rec->block_available_up;
    int block_available_left = rec->block_available_left;

    shift = 20 - sample_bit_depth;
    clip = sample_bit_depth + 1;

    if (trans_BitSize == B32X32_IN_BIT) {
        shift--;
        clip++;
    }
    if (iBlockType == P2NXN || iBlockType == PHOR_UP || iBlockType == PHOR_DOWN || iBlockType == InNxNMB) {
        iHor = 1;
        iSizeX = (trans_BitSize == B32X32_IN_BIT) ? (1 << trans_BitSize) : (1 << (trans_BitSize + 1));
        iSizeY = (trans_BitSize == B32X32_IN_BIT) ? (1 << (trans_BitSize - 2)) : (1 << (trans_BitSize - 1));
    } else if (iBlockType == PNX2N || iBlockType == PVER_LEFT || iBlockType == PVER_RIGHT || iBlockType == INxnNMB) {
        iHor = 0;
        iSizeX = (trans_BitSize == B32X32_IN_BIT) ? (1 << (trans_BitSize - 2)) : (1 << (trans_BitSize - 1));
        iSizeY = (trans_BitSize == B32X32_IN_BIT) ? (1 << trans_BitSize) : (1 << (trans_BitSize + 1));
    }

    if (b_intra  &&  secT_enabled) {
        vt = (uiMode >= 0 && uiMode <= 23);
        ht = (uiMode >= 13 && uiMode <= 32) || (uiMode >= 0 && uiMode <= 2);
        vt = vt && block_available_up;
        ht = ht && block_available_left;
        if (ht) {
            g_funs_handle.inv_2nd_hor(blk, iSizeX, 7, tab_c8_trans);
        }
        if (vt) {
            g_funs_handle.inv_2nd_ver(blk, iSizeX, 7, tab_c8_trans);
        }
    }

    if (trans_BitSize == B8X8_IN_BIT) {
        if (iHor == 1) {
            g_funs_handle.idct_hor[0](blk, shift, clip);
        } else {
            g_funs_handle.idct_ver[0](blk, shift, clip);
        }
    } else if (trans_BitSize == B16X16_IN_BIT) {
        if (iHor == 1) {
            g_funs_handle.idct_hor[1](blk, shift, clip);
        } else {
            g_funs_handle.idct_ver[1](blk, shift, clip);
        }
    } else if (trans_BitSize == B32X32_IN_BIT) {
        if (iHor == 1) {
            g_funs_handle.idct_hor[2](blk, shift, clip);
        } else {
            g_funs_handle.idct_ver[2](blk, shift, clip);
        }
        if (iHor == 1) {
            g_funs_handle.inv_wavelet_hor(blk);
        } else {
            g_funs_handle.inv_wavelet_ver(blk);
        }
    }
}

void com_funs_init_dct()
{
    g_funs_handle.idct_sqt[0] = idct_4x4;
    g_funs_handle.idct_sqt[1] = idct_8x8;
    g_funs_handle.idct_sqt[2] = idct_16x16;
    g_funs_handle.idct_sqt[3] = idct_32x32;
    g_funs_handle.idct_sqt[4] = idct_32x32;

    g_funs_handle.idct_hor[0] = idct_16x4;
    g_funs_handle.idct_hor[1] = idct_32x8;
    g_funs_handle.idct_hor[2] = idct_32x8;

    g_funs_handle.idct_ver[0] = idct_4x16;
    g_funs_handle.idct_ver[1] = idct_8x32;
    g_funs_handle.idct_ver[2] = idct_8x32;

    g_funs_handle.inv_2nd_hor = inv_2nd_trans_hor;
    g_funs_handle.inv_2nd_ver = inv_2nd_trans_ver;
    g_funs_handle.inv_2nd     = inv_2nd_trans;

    g_funs_handle.inv_wavelet = inv_wavelet_B64;
    g_funs_handle.inv_wavelet_hor = inv_wavelet_NSQT_Hor;
    g_funs_handle.inv_wavelet_ver = inv_wavelet_NSQT_Ver;

}
