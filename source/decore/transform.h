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


#define shift_trans 10   // 8-pt trans: 9 + 17 - 16 = 10
#define shift_out   15   // 8-pt inv_trans: 13 + (28-17) - 9 = 15#

#define PIXEL_BIT       8
#define RESID_BIT       (PIXEL_BIT + 1)
#define LIMIT_BIT       16
#define FACTO_BIT       5

void core_inv_trans     (com_rec_t *rec, coef_t *blk, unsigned int trans_BitSize, int blk_id);
void core_inv_trans_nsqt(com_rec_t *rec, coef_t *blk, unsigned int trans_BitSize, int blk_id);
