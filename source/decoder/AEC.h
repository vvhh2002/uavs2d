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

#ifndef _AEC_H_
#define _AEC_H_

#include "global.h"
#include "bitstream.h"
#include "commonVariables.h"
#include "commonStructures.h"

int aec_cu_type(avs2_dec_t *h_dec, aec_core_t * aec);
int aec_cu_type_sfrm(avs2_dec_t *h_dec, aec_core_t * aec);
int aec_sdip(avs2_dec_t *h_dec, aec_core_t * aec);
int aec_bpred_dir(avs2_dec_t *h_dec, aec_core_t * aec);
int aec_p_dhp_dir(aec_core_t * aec, com_cu_t *cun);
int aec_intra_pred_mode(aec_core_t * aec);
int aec_ref_index(avs2_dec_t *h_dec, aec_core_t * aec, int start_x, int start_y);
int aec_dmh_mode(aec_core_t * aec, int cu_bitsize);
int aec_mvd(avs2_dec_t *h_dec, aec_core_t * aec, int xy);
int aec_weight_skip_mode(avs2_dec_t *h_dec, aec_core_t * aec);
int aec_direct_skip(aec_core_t * aec);
int aec_pskip_mode(aec_core_t * aec);
int aec_cbp(avs2_dec_t *h_dec, aec_core_t * aec);
int aec_delta_qp(avs2_dec_t*h_dec, aec_core_t * aec);
int aec_intra_pred_mode_c(avs2_dec_t *h_dec, aec_core_t * aec);
int aec_cu_split_flag(avs2_dec_t *h_dec, int uiBitSize);
int aec_trans_size(aec_core_t * aec, int cu_bitsize, int b_sdip);

int aec_run_level(avs2_dec_t *h_dec, aec_core_t * aec, com_cu_t *cu, int b8, int *run_buf, int*level_buf, int* cg_flag, int* pairs_in_cg, int* cg_num, int plane);

int aec_sao_merge_flag(avs2_dec_t *h_dec, int mergeleft_avail, int mergeup_avail, int cu_idx);
int aec_sao_mode(avs2_dec_t *h_dec, int cu_idx);
int aec_sao_type(avs2_dec_t *h_dec, sap_param_t *saoBlkParam, int cu_idx);

void aec_sao_offsets(avs2_dec_t *h_dec, sap_param_t *saoBlkParam, int cu_idx, int offsetTh, int *offset);

int aec_alf_lcu_ctrl(avs2_dec_t *h_dec, aec_core_t * aec);

#endif