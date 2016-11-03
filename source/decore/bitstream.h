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

#ifndef _VLC_H_
#define _VLC_H_

#include "common.h"

typedef struct bs_stream_t{
    int      bits_to_go;    //!< actual position in the codebuffer, bit-oriented, UVLC only
    uchar_t *p_start_test;
    uchar_t *p_end;
    uchar_t *bs_data;      //!< actual codebuffer for read bytes
} bs_stream_t;


int se_v(bs_stream_t *bs);
int ue_v(bs_stream_t *bs);
int u_v(bs_stream_t *bs, int bits);
int bs_one_unit_init(bs_stream_t *bs, uchar_t *start, uchar_t *end);

#define U_V(bs, bits, flag) u_v(bs, bits)
#define UE_V(bs, flag) ue_v(bs)
#define SE_V(bs, flag) se_v(bs)

#endif

