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

#ifndef __HEADERS_H__
#define __HEADERS_H__

#include "defines.h"

#define NO_VAR_BINS            16
#define ALF_MAX_NUM_COEF       9

enum ALFComponentID {
    ALF_Y = 0,
    ALF_Cb,
    ALF_Cr,
    NUM_ALF_COMPONENT
};

typedef struct {
    int alf_flag;
    int filters_per_group;
    int filterPattern[NO_VAR_BINS];
    int coeffmulti[NO_VAR_BINS][ALF_MAX_NUM_COEF];
} ALFParam;

typedef struct reference_management {
    int referd_by_others;
    int num_of_ref;
    int ref_pic[MAXREF];
    int num_to_remove;
    int remove_pic[MAXREF];
} ref_man;

typedef struct {
    int profile_id;
    int level_id;
    int progressive_sequence;
    int is_field_sequence;
    int horizontal_size;
    int vertical_size;
    int chroma_format;
    int output_bit_depth;
    int sample_bit_depth;
    int sample_precision;
    int aspect_ratio_information;
    int frame_rate_code;
    int bit_rate_lower;
    int bit_rate_upper;
    int low_delay;
    int temporal_id_exist_flag;
    int b_pmvr_enabled;
    int b_mhpskip_enabled;
    int dhp_enabled;
    int wsm_enabled;
    int useNSQT;
    int useDQP;
    int useSDIP;
    int lf_cross_slice;
    int b_secT_enabled;
    int bbv_buf_size;
    int background_picture_enable;
    int g_uiMaxSizeInBit;
    int inter_amp_enable;
    int sao_enable;
    int alf_enable;
    int slice_set_enable;
    int gop_size;
    ref_man decod_RPS[MAXGOP];
    int picture_reorder_delay;
    int ROI_Coding;

    /* extend info */
    int img_width;
    int img_height;
    int img_widthc; 
    int img_heightc;
    int auto_crop_right;
    int auto_crop_bottom;

    int img_width_in_mcu;
    int img_height_in_mcu;
    int img_size_in_mcu;
   
    int img_width_in_lcu;
    int img_height_in_lcu;
    int img_size_in_lcu;

    int b4_info_stride;
    int b4_info_size;

    int lcu_size;

    int seq_weighting_quant_flag;
    i16s_t seq_wq_matrix[2][64];

    int pad_size;
    int bg_output_flag;

} seq_info_t;

typedef struct {
    int pic_type;
    int pic_typeb;

    i64s_t coding_order;
    int cur_layer;
    int displaydelay;
    
    int is_top_field;
    int fixed_picture_qp;
    int picture_qp;
    int loop_filter_disable;
    int loop_filter_parameter_flag;
    int alpha_c_offset;
    int beta_offset;
    int chroma_quant_param_disable;
    int chroma_quant_param_delta_u;
    int chroma_quant_param_delta_v;

    int weighting_quant_flag;
    i16s_t pic_wq_matrix[2][64];

    int picture_coding_type;
    int background_reference_enable;

    ref_man curr_RPS;
    ALFParam m_alfPictureParam[NUM_ALF_COMPONENT];

} pic_hdr_t;

typedef struct {
    int slice_vertical_position;
    int slice_vertical_position_extension;
    int slice_horizontal_positon;
    int slice_horizontal_positon_extension;
    int current_slice_set_index;
    int current_slice_header_flag;
    int fixed_slice_qp;
    int slice_qp;
    int slice_sao_on[3];
} slice_hdr_t;

#endif // #ifndef __HEADERS_H__