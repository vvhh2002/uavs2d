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

int scale_motion_vector(avs2_dec_t *h_dec, int motion_vector, int currblkref, int neighbourblkref, int ref);    //, int currsmbtype, int neighboursmbtype, int block_y_pos, int curr_block_y,  int direct_mv);
void set_ref_info(avs2_dec_t *h_dec);

void core_inter_get_luma(int x_pos, int y_pos, int step_h, int step_v, pel_t *dst, int i_dst, com_frm_t* p_ref_frm, int bit_depth, int check_ref);
void core_inter_get_chroma(pel_t *dst, int i_dst, com_frm_t *p_ref_frm, int uv, int start_x, int start_y, int width, int height, int bit_depth, int check_ref);