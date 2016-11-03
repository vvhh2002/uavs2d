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

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdio.h>                              //!< for FILE
#include "defines.h"
#include "commonStructures.h"
#include "commonVariables.h"
#include "biaridecod.h"

int simd_avx_level(int* phwavx);

void* dec_one_frame(void *h);

void init_lcu(avs2_dec_t *h_dec, int lcu_x, int lcu_y);
void init_rec_core_lcu(com_rec_t *rec, com_lcu_t *lcu, int lcu_x, int lcu_y);

int  cu_decode(avs2_dec_t *h_dec, unsigned int uiBitSize, unsigned int cu_idx);
int  cu_decode_reconstruct(com_rec_t *rec, unsigned int uiBitSize, unsigned int cu_idx);


void com_frm_create(avs2_dec_ctrl_t *ctrl, com_frm_t *frm, int pad);
void com_frm_destroy(avs2_dec_ctrl_t *ctrl, com_frm_t *frm);


#define PAYLOAD_TYPE_IDERP 8

#define I_PICTURE_START_CODE    0xB3
#define PB_PICTURE_START_CODE   0xB6
#define SLICE_START_CODE_MIN    0x00
#define SLICE_START_CODE_MAX    0xAF
#define USER_DATA_START_CODE    0xB2
#define SEQUENCE_HEADER_CODE    0xB0
#define EXTENSION_START_CODE    0xB5
#define SEQUENCE_END_CODE       0xB1
#define VIDEO_EDIT_CODE         0xB7

#define LOCATION_DATA_EXTENSION_ID               15

void deblock_lcu_row(com_rec_t *rec, com_pic_t *pic);
void deblock_set_cu_edge(avs2_dec_t *h_dec, unsigned int uiBitSize, unsigned int cu_idx);

int  AEC_startcode_follows(avs2_dec_t *h_dec, int eos_bit);


#endif

