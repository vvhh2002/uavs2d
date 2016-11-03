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
#include "../inter-prediction.h"

ALIGNED_16(char intrinsic_mask16[15][16]) = {
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

ALIGNED_32(char intrinsic_mask32[32][32]) = {
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0 }
};

void com_init_intrinsic_256()
{
	g_funs_handle.add_pel_clip    = add_pel_clip_sse256;
	g_funs_handle.avg_pel         = avg_pel_sse256;
	g_funs_handle.padding_rows    = padding_rows_sse256;
	g_funs_handle.padding_rows_lr = padding_rows_lr_sse256;

    //g_funs_handle.sao_flt = SAO_on_block_sse256;
    g_funs_handle.alf_flt = alf_flt_one_block_sse256;

	g_funs_handle.ipflt[IPFILTER_H_4][ 7] = com_if_filter_hor_4_w16_sse256;
    g_funs_handle.ipflt[IPFILTER_H_4][11] = com_if_filter_hor_4_w24_sse256;
	g_funs_handle.ipflt[IPFILTER_H_4][15] = com_if_filter_hor_4_w16_sse256;
	
	g_funs_handle.ipflt[IPFILTER_H_8][ 3] = com_if_filter_hor_8_w16_sse256;
    g_funs_handle.ipflt[IPFILTER_H_8][ 5] = com_if_filter_hor_8_w24_sse256;
	g_funs_handle.ipflt[IPFILTER_H_8][ 7] = com_if_filter_hor_8_w16_sse256;
	g_funs_handle.ipflt[IPFILTER_H_8][11] = com_if_filter_hor_8_w16_sse256;
	g_funs_handle.ipflt[IPFILTER_H_8][15] = com_if_filter_hor_8_w16_sse256;
	
    g_funs_handle.ipflt[IPFILTER_V_4][ 7] = com_if_filter_ver_4_w16_sse256;
    g_funs_handle.ipflt[IPFILTER_V_4][11] = com_if_filter_ver_4_w24_sse256;
	g_funs_handle.ipflt[IPFILTER_V_4][15] = com_if_filter_ver_4_w32_sse256;
	
	g_funs_handle.ipflt[IPFILTER_V_8][ 3] = com_if_filter_ver_8_w16_sse256;
	g_funs_handle.ipflt[IPFILTER_V_8][ 5] = com_if_filter_ver_8_w24_sse256;
	g_funs_handle.ipflt[IPFILTER_V_8][ 7] = com_if_filter_ver_8_w32_sse256;
	g_funs_handle.ipflt[IPFILTER_V_8][11] = com_if_filter_ver_8_w48_sse256;
	g_funs_handle.ipflt[IPFILTER_V_8][15] = com_if_filter_ver_8_w64_sse256;

	g_funs_handle.ipflt_ext[IPFILTER_EXT_4][ 7] = com_if_filter_hor_ver_4_w16_sse256;
    g_funs_handle.ipflt_ext[IPFILTER_EXT_4][11] = com_if_filter_hor_ver_4_w24_sse256;
	g_funs_handle.ipflt_ext[IPFILTER_EXT_4][15] = com_if_filter_hor_ver_4_w16_sse256;
	
	g_funs_handle.ipflt_ext[IPFILTER_EXT_8][ 3] = com_if_filter_hor_ver_8_w16_sse256;
    g_funs_handle.ipflt_ext[IPFILTER_EXT_8][ 5] = com_if_filter_hor_ver_8_w24_sse256;
	g_funs_handle.ipflt_ext[IPFILTER_EXT_8][ 7] = com_if_filter_hor_ver_8_w16_sse256;
	g_funs_handle.ipflt_ext[IPFILTER_EXT_8][11] = com_if_filter_hor_ver_8_w16_sse256;
	g_funs_handle.ipflt_ext[IPFILTER_EXT_8][15] = com_if_filter_hor_ver_8_w16_sse256;
	

	g_funs_handle.deblock_edge  [0] = deblock_edge_ver_sse256;
	g_funs_handle.deblock_edge  [1] = deblock_edge_hor_sse256;
	g_funs_handle.deblock_edge_c[0] = deblock_edge_ver_c_sse256;
	g_funs_handle.deblock_edge_c[1] = deblock_edge_hor_c_sse256;

	// the idct module for 256 is much slower than that of 128
	g_funs_handle.idct_sqt[1] = idct_8x8_sse256;			    //huge delay --> much slower than 128
	g_funs_handle.idct_sqt[2] = idct_16x16_sse256;				
	g_funs_handle.idct_sqt[3] = idct_32x32_sse256;			
	g_funs_handle.idct_sqt[4] = idct_32x32_sse256;			

	g_funs_handle.inv_wavelet = inv_wavelet_B64_sse256;
	g_funs_handle.inv_wavelet_hor = inv_wavelet_NSQT_Hor_sse256;
	g_funs_handle.inv_wavelet_ver = inv_wavelet_NSQT_Ver_sse256;	//slower than 128

	//g_funs_handle.intra_pred_plane = xPredIntraPlaneAdi;
	g_funs_handle.intra_pred_hor = xPredIntraHorAdi_sse256;
	g_funs_handle.intra_pred_ver = xPredIntraVertAdi_sse256;
    //g_funs_handle.intra_pred_dc = xPredIntraDCAdi_see256;        //release模式下此函数存在bug（debug模式运行正常）
	//g_funs_handle.intra_pred_bi = xPredIntraBiAdi;

}

void com_init_intrinsic_256_10bit()
{
    printf("com_init_intrinsic_256_10bit()\n");

    g_funs_handle.add_pel_clip    = add_pel_clip_sse256_10bit;
    g_funs_handle.avg_pel         = avg_pel_sse256_10bit;
    g_funs_handle.padding_rows    = padding_rows_sse256_10bit;
    g_funs_handle.padding_rows_lr = padding_rows_lr_sse256_10bit;

    ////g_funs_handle.sao_flt = SAO_on_block_sse256;
    g_funs_handle.alf_flt = alf_flt_one_block_sse256_10bit;

    g_funs_handle.ipcpy[IPFILTER_EXT_4][ 7] = com_if_filter_cpy16_sse256_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_4][15] = com_if_filter_cpy_sse256_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_8][ 3] = com_if_filter_cpy16_sse256_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_8][ 7] = com_if_filter_cpy_sse256_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_8][11] = com_if_filter_cpy_sse256_10bit;
    g_funs_handle.ipcpy[IPFILTER_EXT_8][15] = com_if_filter_cpy_sse256_10bit;

    g_funs_handle.ipflt[IPFILTER_H_4][ 7] = com_if_filter_hor_4_w8_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_H_4][11] = com_if_filter_hor_4_w8_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_H_4][15] = com_if_filter_hor_4_w8_sse256_10bit;

    g_funs_handle.ipflt[IPFILTER_H_8][ 3] = com_if_filter_hor_8_w16_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_H_8][ 5] = com_if_filter_hor_8_w24_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_H_8][ 7] = com_if_filter_hor_8_w16_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_H_8][11] = com_if_filter_hor_8_w16_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_H_8][15] = com_if_filter_hor_8_w16_sse256_10bit;

    g_funs_handle.ipflt[IPFILTER_V_4][ 7] = com_if_filter_ver_4_w16_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_V_4][11] = com_if_filter_ver_4_w24_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_V_4][15] = com_if_filter_ver_4_w32_sse256_10bit;

    g_funs_handle.ipflt[IPFILTER_V_8][ 3] = com_if_filter_ver_8_w16_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_V_8][ 5] = com_if_filter_ver_8_w24_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_V_8][ 7] = com_if_filter_ver_8_w16_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_V_8][11] = com_if_filter_ver_8_w16_sse256_10bit;
    g_funs_handle.ipflt[IPFILTER_V_8][15] = com_if_filter_ver_8_w16_sse256_10bit;

    g_funs_handle.ipflt_ext[IPFILTER_EXT_4][ 7] = com_if_filter_hor_ver_4_w16_sse256_10bit;
    g_funs_handle.ipflt_ext[IPFILTER_EXT_4][11] = com_if_filter_hor_ver_4_w24_sse256_10bit;
    g_funs_handle.ipflt_ext[IPFILTER_EXT_4][15] = com_if_filter_hor_ver_4_w16_sse256_10bit;


    g_funs_handle.ipflt_ext[IPFILTER_EXT_8][ 3] = com_if_filter_hor_ver_8_w16_sse256_10bit;
    g_funs_handle.ipflt_ext[IPFILTER_EXT_8][ 5] = com_if_filter_hor_ver_8_w24_sse256_10bit;
    g_funs_handle.ipflt_ext[IPFILTER_EXT_8][ 7] = com_if_filter_hor_ver_8_w16_sse256_10bit;
    g_funs_handle.ipflt_ext[IPFILTER_EXT_8][11] = com_if_filter_hor_ver_8_w16_sse256_10bit;
    g_funs_handle.ipflt_ext[IPFILTER_EXT_8][15] = com_if_filter_hor_ver_8_w16_sse256_10bit;

    //g_funs_handle.deblock_edge[0] = deblock_edge_ver_sse256;
    //g_funs_handle.deblock_edge[1] = deblock_edge_hor_sse256;
    //g_funs_handle.deblock_edge_c[0] = deblock_edge_ver_c_sse256;
    //g_funs_handle.deblock_edge_c[1] = deblock_edge_hor_c_sse256;

    g_funs_handle.idct_sqt[1] = idct_8x8_sse256;			
    g_funs_handle.idct_sqt[2] = idct_16x16_sse256;
    g_funs_handle.idct_sqt[3] = idct_32x32_sse256;
    g_funs_handle.idct_sqt[4] = idct_32x32_sse256;

    g_funs_handle.inv_wavelet = inv_wavelet_B64_sse256;
    g_funs_handle.inv_wavelet_hor = inv_wavelet_NSQT_Hor_sse256;
    g_funs_handle.inv_wavelet_ver = inv_wavelet_NSQT_Ver_sse256;	

    ////g_funs_handle.intra_pred_plane = xPredIntraPlaneAdi;
    //g_funs_handle.intra_pred_hor = xPredIntraHorAdi_sse256;
    //g_funs_handle.intra_pred_ver = xPredIntraVertAdi_sse256;
    ////g_funs_handle.intra_pred_dc = xPredIntraDCAdi_see256;        //release模式下此函数存在bug（debug模式运行正常）
    ////g_funs_handle.intra_pred_bi = xPredIntraBiAdi;

}
