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

#include "commonVariables.h"
#include "defines.h"

void off_sao(sap_param_t *saoblkparam);
void copySAOParam_for_blk(sap_param_t *saopara_dst, sap_param_t *saopara_src);
void copySAOParam_for_blk_onecomponent(sap_param_t *saopara_dst, sap_param_t *saopara_src);
void getMergeNeighbor(avs2_dec_t *h_dec, int smb_index, int pix_y, int pix_x, int smb_pix_width, int smb_pix_height, int input_MaxsizeInBit, int input_slice_set_enable, sap_param_t(*rec_saoBlkParam)[3], int *MergeAvail, sap_param_t sao_merge_param[][3]);

void sao_lcu_row(com_rec_t *rec, int input_MaxSizeInBit, int input_slice_set_enable, sap_param_t(*rec_saoBlkParam)[3], int sample_bit_depth, int pix_y);