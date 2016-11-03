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

#ifndef _GLOBAL_COM_H_
#define _GLOBAL_COM_H_


#include <stdio.h>                              //!< for FILE
#include "defines.h"
#include "commonStructures.h"

extern tab_i32s_t tab_Left_Down_Avail_Matrix64[16][16];
extern tab_i32s_t tab_Up_Right_Avail_Matrix64[16][16];


extern tab_i32s_t tab_b8_xy_cvto_zigzag[8][8];
extern tab_i32s_t tab_blk_size[10][4];
extern tab_i32s_t tab_pu_pix_info[10][4];
extern tab_i32s_t tab_pu_blk_num[10];
extern tab_i32s_t tab_pu_2_blk_idx[10][4];

#endif
