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
#include "../inter-prediction.h"

ALIGNED_16(char intrinsic_mask[15][16]) = {
    { -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    { -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    { -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    { -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    { -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    { -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    { -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    { -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0 },
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0 },
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0 },
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0 },
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0 },
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0 },
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0 },
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0 }
};

ALIGNED_32(i16s_t intrinsic_mask_10bit[15][16]) = {
        { -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        { -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        { -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0 }
};

// CPUIDFIELD  

#define  CPUIDFIELD_MASK_POS    0x0000001F  // 位偏移. 0~31.  
#define  CPUIDFIELD_MASK_LEN    0x000003E0  // 位长. 1~32  
#define  CPUIDFIELD_MASK_REG    0x00000C00  // 寄存器. 0=EAX, 1=EBX, 2=ECX, 3=EDX.  
#define  CPUIDFIELD_MASK_FIDSUB 0x000FF000  // 子功能号(低8位).  
#define  CPUIDFIELD_MASK_FID    0xFFF00000  // 功能号(最高4位 和 低8位).  

#define CPUIDFIELD_SHIFT_POS    0  
#define CPUIDFIELD_SHIFT_LEN    5  
#define CPUIDFIELD_SHIFT_REG    10  
#define CPUIDFIELD_SHIFT_FIDSUB 12  
#define CPUIDFIELD_SHIFT_FID    20  

#define CPUIDFIELD_MAKE(fid,fidsub,reg,pos,len) (((fid)&0xF0000000)     \
    | ((fid) << CPUIDFIELD_SHIFT_FID & 0x0FF00000)                      \
    | ((fidsub) << CPUIDFIELD_SHIFT_FIDSUB & CPUIDFIELD_MASK_FIDSUB)    \
    | ((reg) << CPUIDFIELD_SHIFT_REG & CPUIDFIELD_MASK_REG)             \
    | ((pos) << CPUIDFIELD_SHIFT_POS & CPUIDFIELD_MASK_POS)             \
    | (((len)-1) << CPUIDFIELD_SHIFT_LEN & CPUIDFIELD_MASK_LEN)         \
    )

#define CPUIDFIELD_FID(cpuidfield)  ( ((cpuidfield)&0xF0000000) | (((cpuidfield) & 0x0FF00000)>>CPUIDFIELD_SHIFT_FID) )  
#define CPUIDFIELD_FIDSUB(cpuidfield)   ( ((cpuidfield) & CPUIDFIELD_MASK_FIDSUB)>>CPUIDFIELD_SHIFT_FIDSUB )  
#define CPUIDFIELD_REG(cpuidfield)  ( ((cpuidfield) & CPUIDFIELD_MASK_REG)>>CPUIDFIELD_SHIFT_REG )  
#define CPUIDFIELD_POS(cpuidfield)  ( ((cpuidfield) & CPUIDFIELD_MASK_POS)>>CPUIDFIELD_SHIFT_POS )  
#define CPUIDFIELD_LEN(cpuidfield)  ( (((cpuidfield) & CPUIDFIELD_MASK_LEN)>>CPUIDFIELD_SHIFT_LEN) + 1 )  

// 取得位域  
#ifndef __GETBITS32  
#define __GETBITS32(src,pos,len)    ( ((src)>>(pos)) & (((unsigned int)-1)>>(32-len)) )  
#endif  


#define CPUF_SSE4A  CPUIDFIELD_MAKE(0x80000001,0,2,6,1)  
#define CPUF_AES    CPUIDFIELD_MAKE(1,0,2,25,1)  
#define CPUF_PCLMULQDQ  CPUIDFIELD_MAKE(1,0,2,1,1)  

#define CPUF_AVX    CPUIDFIELD_MAKE(1,0,2,28,1)  
#define CPUF_AVX2   CPUIDFIELD_MAKE(7,0,1,5,1)  
#define CPUF_OSXSAVE    CPUIDFIELD_MAKE(1,0,2,27,1)  
#define CPUF_XFeatureSupportedMaskLo    CPUIDFIELD_MAKE(0xD,0,0,0,32)  
#define CPUF_F16C   CPUIDFIELD_MAKE(1,0,2,29,1)  
#define CPUF_FMA    CPUIDFIELD_MAKE(1,0,2,12,1)  
#define CPUF_FMA4   CPUIDFIELD_MAKE(0x80000001,0,2,16,1)  
#define CPUF_XOP    CPUIDFIELD_MAKE(0x80000001,0,2,11,1)  


// SSE系列指令集的支持级别. simd_sse_level 函数的返回值。  
#define SIMD_SSE_NONE   0   // 不支持  
#define SIMD_SSE_1  1   // SSE  
#define SIMD_SSE_2  2   // SSE2  
#define SIMD_SSE_3  3   // SSE3  
#define SIMD_SSE_3S 4   // SSSE3  
#define SIMD_SSE_41 5   // SSE4.1  
#define SIMD_SSE_42 6   // SSE4.2  

const char* simd_sse_names[] = {
    "None",
    "SSE",
    "SSE2",
    "SSE3",
    "SSSE3",
    "SSE4.1",
    "SSE4.2",
};


// AVX系列指令集的支持级别. simd_avx_level 函数的返回值。  
#define SIMD_AVX_NONE   0   // 不支持  
#define SIMD_AVX_1  1   // AVX  
#define SIMD_AVX_2  2   // AVX2  

const char* simd_avx_names[] = {
    "None",
    "AVX",
    "AVX2"
};


// 根据CPUIDFIELD从缓冲区中获取字段.  
unsigned int  getcpuidfield_buf(const int dwBuf[4], int cpuf)
{
    return __GETBITS32(dwBuf[CPUIDFIELD_REG(cpuf)], CPUIDFIELD_POS(cpuf), CPUIDFIELD_LEN(cpuf));
}

// 根据CPUIDFIELD获取CPUID字段.  
unsigned int  getcpuidfield(int cpuf)
{
#ifdef _WIN32
    int dwBuf[4];
    __cpuidex(dwBuf, CPUIDFIELD_FID(cpuf), CPUIDFIELD_FIDSUB(cpuf));
    return getcpuidfield_buf(dwBuf, cpuf);
#else
    return 6;
#endif
}

// 检测AVX系列指令集的支持级别.  
int simd_avx_level(int* phwavx)
{
    int rt = SIMD_AVX_NONE; // result  

    // check processor support  
    if (0 != getcpuidfield(CPUF_AVX))
    {
        rt = SIMD_AVX_1;
        if (0 != getcpuidfield(CPUF_AVX2))
        {
            rt = SIMD_AVX_2;
        }
    }
    if (NULL != phwavx)   *phwavx = rt;

    // check OS support  
    if (0 != getcpuidfield(CPUF_OSXSAVE)) // XGETBV enabled for application use.  
    {
        unsigned int n = getcpuidfield(CPUF_XFeatureSupportedMaskLo); // XCR0: XFeatureSupportedMask register.  
        if (6 == (n & 6))   // XCR0[2:1] = ‘11b’ (XMM state and YMM state are enabled by OS).  
        {
            return rt;
        }
    }
    return SIMD_AVX_NONE;
}


static void cpy_pel_to_uchar_sse128(const pel_t *src, int i_src, uchar_t *dst, int i_dst, int width, int height, int bit_size)
{
    int i;
    for (i = 0; i < height; i++) {
        memcpy(dst, src, width * sizeof(uchar_t));
        dst += i_dst;
        src += i_src;
    }
}

static void cpy_pel_to_uchar_sse128_10bit(const pel_t *src, int i_src, uchar_t *dst, int i_dst, int width, int height, int bit_size)
{
    int i;
    __m128i add = _mm_set1_epi16(2);

    if (bit_size == 10) {
        while (height--) {
            for (i = 0; i < width - 16; i += 16) {
                __m128i T1 = _mm_loadu_si128((const __m128i*)(src + i));
                __m128i T2 = _mm_loadu_si128((const __m128i*)(src + i + 8));
                T1 = _mm_srli_epi16(_mm_add_epi16(T1, add), 2);
                T2 = _mm_srli_epi16(_mm_add_epi16(T2, add), 2);
                T1 = _mm_packus_epi16(T1, T2);
                _mm_storeu_si128((__m128i*)(dst + i), T1);
            }
            if (i < width - 8) {
                __m128i T1 = _mm_loadu_si128((const __m128i*)(src + i));
                T1 = _mm_srli_epi16(_mm_add_epi16(T1, add), 2);
                T1 = _mm_packus_epi16(T1, T1);
                _mm_storel_epi64((__m128i*)(dst + i), T1);
                i += 8;
            }
            for (; i < width; i++) {
                dst[i] = (uchar_t)COM_CLIP3(0, 255, (src[i] + 2) >> 2);
            }
            dst += i_dst;
            src += i_src;
        }
    } else {
        while (height--) {
            for (i = 0; i < width - 16; i += 16) {
                __m128i T1 = _mm_loadu_si128((const __m128i*)(src + i));
                __m128i T2 = _mm_loadu_si128((const __m128i*)(src + i + 8));
                T1 = _mm_packus_epi16(T1, T2);
                _mm_storeu_si128((__m128i*)(dst + i), T1);
            }
            if (i < width - 8) {
                __m128i T1 = _mm_loadu_si128((const __m128i*)(src + i));
                T1 = _mm_packus_epi16(T1, T1);
                _mm_storel_epi64((__m128i*)(dst + i), T1);
                i += 8;
            }
            for (; i < width; i++) {
                dst[i] = (uchar_t)COM_CLIP3(0, 255, src[i]);
            }
            dst += i_dst;
            src += i_src;
        }
    }
}

void com_init_intrinsic() 
{
    int i;

    for (i = 0; i < 16; i++) {
        g_funs_handle.ipcpy[IPFILTER_EXT_4][i] = com_if_filter_cpy_sse128;
        g_funs_handle.ipcpy[IPFILTER_EXT_8][i] = com_if_filter_cpy_sse128;
        g_funs_handle.ipflt[IPFILTER_H_4][i] = com_if_filter_hor_4_sse128;
        g_funs_handle.ipflt[IPFILTER_H_8][i] = com_if_filter_hor_8_sse128;
        g_funs_handle.ipflt[IPFILTER_V_4][i] = com_if_filter_ver_4_sse128;
        g_funs_handle.ipflt[IPFILTER_V_8][i] = com_if_filter_ver_8_sse128;
        g_funs_handle.ipflt_ext[IPFILTER_EXT_4][i] = com_if_filter_hor_ver_4_sse128;
        g_funs_handle.ipflt_ext[IPFILTER_EXT_8][i] = com_if_filter_hor_ver_8_sse128;
    }

    g_funs_handle.ipcpy[IPFILTER_EXT_4][0] = com_if_filter_cpy2_sse128;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][1] = com_if_filter_cpy4_sse128;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][2] = com_if_filter_cpy6_sse128;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][3] = com_if_filter_cpy8_sse128;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][5] = com_if_filter_cpy12_sse128;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][7] = com_if_filter_cpy16_sse128;

    g_funs_handle.ipcpy[IPFILTER_EXT_8][0] = com_if_filter_cpy4_sse128;
    g_funs_handle.ipcpy[IPFILTER_EXT_8][1] = com_if_filter_cpy8_sse128;
    g_funs_handle.ipcpy[IPFILTER_EXT_8][2] = com_if_filter_cpy12_sse128;
    g_funs_handle.ipcpy[IPFILTER_EXT_8][3] = com_if_filter_cpy16_sse128;

    g_funs_handle.deblock_edge  [0] = deblock_edge_ver_sse128;
    g_funs_handle.deblock_edge  [1] = deblock_edge_hor_sse128;
    g_funs_handle.deblock_edge_c[0] = deblock_edge_ver_c_sse128;
    g_funs_handle.deblock_edge_c[1] = deblock_edge_hor_c_sse128;

    g_funs_handle.idct_sqt[0] = idct_4x4_sse128;
    g_funs_handle.idct_sqt[1] = idct_8x8_sse128;
    g_funs_handle.idct_sqt[2] = idct_16x16_sse128;
    g_funs_handle.idct_sqt[3] = idct_32x32_sse128;
    g_funs_handle.idct_sqt[4] = idct_32x32_sse128;

    g_funs_handle.idct_hor[0] = idct_16x4_sse128;
    g_funs_handle.idct_hor[1] = idct_32x8_sse128;
    g_funs_handle.idct_hor[2] = idct_32x8_sse128;

    g_funs_handle.idct_ver[0] = idct_4x16_sse128;
    g_funs_handle.idct_ver[1] = idct_8x32_sse128;
    g_funs_handle.idct_ver[2] = idct_8x32_sse128;

    g_funs_handle.inv_2nd_hor = inv_2nd_trans_hor_sse128;
    g_funs_handle.inv_2nd_ver = inv_2nd_trans_ver_sse128;
    g_funs_handle.inv_2nd     = inv_2nd_trans_sse128;

    g_funs_handle.inv_wavelet     = inv_wavelet_B64_sse128;
    g_funs_handle.inv_wavelet_hor = inv_wavelet_NSQT_Hor_sse128;
	g_funs_handle.inv_wavelet_ver = inv_wavelet_NSQT_Ver_sse128;

    g_funs_handle.alf_flt = alf_flt_one_block_sse128;
    g_funs_handle.sao_flt = SAO_on_block_sse128;

    g_funs_handle.cpy_pel_I420_to_uchar_YUY2 = cpy_pel_I420_to_uchar_YUY2_sse128;

    g_funs_handle.cpy_pel_to_uchar = cpy_pel_to_uchar_sse128;
    g_funs_handle.add_pel_clip     = add_pel_clip_sse128;
    g_funs_handle.avg_pel          = avg_pel_sse128;
    g_funs_handle.padding_rows     = padding_rows_sse128;
    g_funs_handle.padding_rows_lr  = padding_rows_lr_sse128;
    
	g_funs_handle.intra_pred_ang_x[ 3 - ANG_X_OFFSET] = xPredIntraAngAdi_X_3_sse128;
    g_funs_handle.intra_pred_ang_x[ 4 - ANG_X_OFFSET] = xPredIntraAngAdi_X_4_sse128;
    g_funs_handle.intra_pred_ang_x[ 5 - ANG_X_OFFSET] = xPredIntraAngAdi_X_5_sse128;
    g_funs_handle.intra_pred_ang_x[ 6 - ANG_X_OFFSET] = xPredIntraAngAdi_X_6_sse128;
    g_funs_handle.intra_pred_ang_x[ 7 - ANG_X_OFFSET] = xPredIntraAngAdi_X_7_sse128;
    g_funs_handle.intra_pred_ang_x[ 8 - ANG_X_OFFSET] = xPredIntraAngAdi_X_8_sse128;
    g_funs_handle.intra_pred_ang_x[ 9 - ANG_X_OFFSET] = xPredIntraAngAdi_X_9_sse128;
    g_funs_handle.intra_pred_ang_x[10 - ANG_X_OFFSET] = xPredIntraAngAdi_X_10_sse128;
    g_funs_handle.intra_pred_ang_x[11 - ANG_X_OFFSET] = xPredIntraAngAdi_X_11_sse128;

    g_funs_handle.intra_pred_ang_xy[14 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_14_sse128;
    g_funs_handle.intra_pred_ang_xy[16 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_16_sse128;
    g_funs_handle.intra_pred_ang_xy[18 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_18_sse128;
    g_funs_handle.intra_pred_ang_xy[20 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_20_sse128;
    g_funs_handle.intra_pred_ang_xy[22 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_22_sse128;

    g_funs_handle.intra_pred_ang_y[25 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_25_sse128;
    g_funs_handle.intra_pred_ang_y[26 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_26_sse128;
    g_funs_handle.intra_pred_ang_y[27 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_27_sse128;
    g_funs_handle.intra_pred_ang_y[28 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_28_sse128;
    g_funs_handle.intra_pred_ang_y[29 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_29_sse128;
    g_funs_handle.intra_pred_ang_y[30 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_30_sse128;
    g_funs_handle.intra_pred_ang_y[31 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_31_sse128;
    g_funs_handle.intra_pred_ang_y[32 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_32_sse128;

    g_funs_handle.intra_pred_plane = xPredIntraPlaneAdi_sse128;
    g_funs_handle.intra_pred_hor   = xPredIntraHorAdi_sse128;
    g_funs_handle.intra_pred_ver   = xPredIntraVertAdi_sse128;
    g_funs_handle.intra_pred_dc    = xPredIntraDCAdi_sse128;
    g_funs_handle.intra_pred_bi    = xPredIntraBiAdi_sse128;
}

void com_init_intrinsic_10bit()
{
    int i;

    printf("com_init_intrinsic_10bit()\n");

    for (i = 0; i < 16; i++) {
        g_funs_handle.ipcpy[IPFILTER_EXT_4][i] = com_if_filter_cpy_sse128_10bit;
        g_funs_handle.ipcpy[IPFILTER_EXT_8][i] = com_if_filter_cpy_sse128_10bit;
		g_funs_handle.ipflt[IPFILTER_H_4][i] = com_if_filter_hor_4_sse128_10bit;
        g_funs_handle.ipflt[IPFILTER_H_8][i] = com_if_filter_hor_8_sse128_10bit;
        g_funs_handle.ipflt[IPFILTER_V_4][i] = com_if_filter_ver_4_sse128_10bit;
        g_funs_handle.ipflt[IPFILTER_V_8][i] = com_if_filter_ver_8_sse128_10bit;
        g_funs_handle.ipflt_ext[IPFILTER_EXT_4][i] = com_if_filter_hor_ver_4_sse128_10bit;
        g_funs_handle.ipflt_ext[IPFILTER_EXT_8][i] = com_if_filter_hor_ver_8_sse128_10bit;
    }

    g_funs_handle.ipcpy[IPFILTER_EXT_4][0] = com_if_filter_cpy2_sse128_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][1] = com_if_filter_cpy4_sse128_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][2] = com_if_filter_cpy6_sse128_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][3] = com_if_filter_cpy8_sse128_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][5] = com_if_filter_cpy12_sse128_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][7] = com_if_filter_cpy16_sse128_10bit;

    g_funs_handle.ipcpy[IPFILTER_EXT_8][0] = com_if_filter_cpy4_sse128_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_8][1] = com_if_filter_cpy8_sse128_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_8][2] = com_if_filter_cpy12_sse128_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_8][3] = com_if_filter_cpy16_sse128_10bit;

    g_funs_handle.deblock_edge  [0] = deblock_edge_ver_sse128_10bit;
    g_funs_handle.deblock_edge  [1] = deblock_edge_hor_sse128_10bit;
    g_funs_handle.deblock_edge_c[0] = deblock_edge_ver_c_sse128_10bit;
    g_funs_handle.deblock_edge_c[1] = deblock_edge_hor_c_sse128_10bit;

    g_funs_handle.idct_sqt[0] = idct_4x4_sse128;
    g_funs_handle.idct_sqt[1] = idct_8x8_sse128;
    g_funs_handle.idct_sqt[2] = idct_16x16_sse128;
    g_funs_handle.idct_sqt[3] = idct_32x32_sse128;
    g_funs_handle.idct_sqt[4] = idct_32x32_sse128;

    g_funs_handle.idct_hor[0] = idct_16x4_sse128;
    g_funs_handle.idct_hor[1] = idct_32x8_sse128;
    g_funs_handle.idct_hor[2] = idct_32x8_sse128;

    g_funs_handle.idct_ver[0] = idct_4x16_sse128;
    g_funs_handle.idct_ver[1] = idct_8x32_sse128;
    g_funs_handle.idct_ver[2] = idct_8x32_sse128;

    g_funs_handle.inv_2nd_hor = inv_2nd_trans_hor_sse128;
    g_funs_handle.inv_2nd_ver = inv_2nd_trans_ver_sse128;
    g_funs_handle.inv_2nd = inv_2nd_trans_sse128;

    g_funs_handle.inv_wavelet = inv_wavelet_B64_sse128;
    g_funs_handle.inv_wavelet_hor = inv_wavelet_NSQT_Hor_sse128;
    g_funs_handle.inv_wavelet_ver = inv_wavelet_NSQT_Ver_sse128;

    g_funs_handle.alf_flt = alf_flt_one_block_sse128_10bit;
    g_funs_handle.sao_flt = SAO_on_block_sse128_10bit;

    g_funs_handle.cpy_pel_I420_to_uchar_YUY2 = cpy_pel_I420_to_uchar_YUY2_sse128_10bit;

    g_funs_handle.cpy_pel_to_uchar = cpy_pel_to_uchar_sse128_10bit;
    g_funs_handle.add_pel_clip     = add_pel_clip_sse128_10bit;
    g_funs_handle.avg_pel          = avg_pel_sse128_10bit;
    g_funs_handle.padding_rows     = padding_rows_sse128_10bit;
    g_funs_handle.padding_rows_lr  = padding_rows_lr_sse128_10bit;

    g_funs_handle.intra_pred_ang_x[ 3 - ANG_X_OFFSET] = xPredIntraAngAdi_X_3_sse128_10bit;
    g_funs_handle.intra_pred_ang_x[ 4 - ANG_X_OFFSET] = xPredIntraAngAdi_X_4_sse128_10bit;
    g_funs_handle.intra_pred_ang_x[ 5 - ANG_X_OFFSET] = xPredIntraAngAdi_X_5_sse128_10bit;
    g_funs_handle.intra_pred_ang_x[ 6 - ANG_X_OFFSET] = xPredIntraAngAdi_X_6_sse128_10bit;
    g_funs_handle.intra_pred_ang_x[ 7 - ANG_X_OFFSET] = xPredIntraAngAdi_X_7_sse128_10bit;
    g_funs_handle.intra_pred_ang_x[ 8 - ANG_X_OFFSET] = xPredIntraAngAdi_X_8_sse128_10bit;
    g_funs_handle.intra_pred_ang_x[ 9 - ANG_X_OFFSET] = xPredIntraAngAdi_X_9_sse128_10bit;
    g_funs_handle.intra_pred_ang_x[10 - ANG_X_OFFSET] = xPredIntraAngAdi_X_10_sse128_10bit;
    g_funs_handle.intra_pred_ang_x[11 - ANG_X_OFFSET] = xPredIntraAngAdi_X_11_sse128_10bit;

    g_funs_handle.intra_pred_ang_xy[14 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_14_sse128_10bit;
    g_funs_handle.intra_pred_ang_xy[16 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_16_sse128_10bit;
    g_funs_handle.intra_pred_ang_xy[18 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_18_sse128_10bit;
    g_funs_handle.intra_pred_ang_xy[20 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_20_sse128_10bit;
    g_funs_handle.intra_pred_ang_xy[22 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_22_sse128_10bit;

    g_funs_handle.intra_pred_ang_y[25 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_25_sse128_10bit;
    g_funs_handle.intra_pred_ang_y[26 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_26_sse128_10bit;
    g_funs_handle.intra_pred_ang_y[27 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_27_sse128_10bit;
    g_funs_handle.intra_pred_ang_y[28 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_28_sse128_10bit;
    g_funs_handle.intra_pred_ang_y[29 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_29_sse128_10bit;
    g_funs_handle.intra_pred_ang_y[30 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_30_sse128_10bit;
#if !COMPILE_10BIT
    g_funs_handle.intra_pred_ang_y[31 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_31_sse128_10bit;
#endif
    g_funs_handle.intra_pred_ang_y[32 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_32_sse128_10bit;

    g_funs_handle.intra_pred_plane = xPredIntraPlaneAdi_sse128_10bit;
    g_funs_handle.intra_pred_hor   = xPredIntraHorAdi_sse128_10bit;
    g_funs_handle.intra_pred_ver   = xPredIntraVertAdi_sse128_10bit;
    g_funs_handle.intra_pred_dc    = xPredIntraDCAdi_sse128_10bit;
    g_funs_handle.intra_pred_bi    = xPredIntraBiAdi_sse128_10bit;
}