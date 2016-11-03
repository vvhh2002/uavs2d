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

#ifndef _WEIGHT_QUANT_H_
#define _WEIGHT_QUANT_H_

#include "common.h"
#include "commonStructures.h"

#define PARAM_NUM  6
#define WQ_MODEL_NUM 3

#define UNDETAILED 0
#define DETAILED   1

#define WQ_MODE_F  0  //M2331 2008-04
#define WQ_MODE_U  1  //M2331 2008-04
#define WQ_MODE_D  2  //M2331 2008-04

extern tab_i32s_t tab_wq_param_default[2][6];

extern tab_i16s_t tab_WqMDefault4x4[16];
extern tab_i16s_t tab_WqMDefault8x8[64];

void drive_pic_wq_matrix_param(i32s_t *param_vector, int mode, i16s_t pic_wq_matrix[2][64]);
void cal_all_size_wq_matrix(i16s_t *cur_wq_matrix[4], i16s_t *pic_wq_matrix);

#endif
