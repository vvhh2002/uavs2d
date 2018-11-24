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

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


#include "global.h"
#include "commonVariables.h"
#include "transform.h"
#include "defines.h"
#include "bitstream.h"
#include "header.h"
#include "AEC.h"
#include "math.h"
#include "common.h"
#include "DecAdaptiveLoopFilter.h"

// Adaptive frequency weighting quantization
#include "wquant.h"

void read_weight_quant_matrix(bs_stream_t *bs, i16s_t seq_wq_matrix[2][64], int user_define)
{
    if (user_define) {
        int i;
        for (i = 0; i < 16; i++) {
            seq_wq_matrix[0][i] = (i16s_t) UE_V(bs, "weight_quant_coeff");
        }
        for (i = 0; i < 64; i++) {
            seq_wq_matrix[1][i] = (i16s_t) UE_V(bs, "weight_quant_coeff");
        }
    } else {
        memcpy(seq_wq_matrix[0], tab_WqMDefault4x4, sizeof(tab_WqMDefault4x4));
        memcpy(seq_wq_matrix[1], tab_WqMDefault8x8, sizeof(tab_WqMDefault8x8));
    }
}

int parse_seq_hdr(avs2_dec_ctrl_t *ctrl, bs_stream_t *bs)
{
    int i, j;
    int marker_bit;
    int lcu_size;
    seq_info_t *seq_info = &ctrl->seq_info;

    seq_info->profile_id           = U_V(bs, 8, "profile_id");
    seq_info->level_id             = U_V(bs, 8, "level_id");
    seq_info->progressive_sequence = U_V(bs, 1, "progressive_sequence");
    seq_info->is_field_sequence    = U_V(bs, 1, "is_field_sequence");
    seq_info->horizontal_size      = U_V(bs, 14, "horizontal_size");
    seq_info->vertical_size        = U_V(bs, 14, "vertical_size");

    U_V(bs, 2, "chroma_format");

    seq_info->output_bit_depth = 8;
    seq_info->sample_bit_depth = 8;
    seq_info->sample_precision = 1;

    if (seq_info->profile_id == 34) { // 10bit profile (0x22)
        seq_info->output_bit_depth = 6 + (U_V(bs, 3, "output_bitdepth")) * 2;
        seq_info->sample_bit_depth = 6 + (U_V(bs, 3, "encoding_precision")) * 2;
    } else { // other profile
        seq_info->sample_precision = U_V(bs, 3, "sample_precision");
    }

    //seq_info->output_bit_depth = 8;

#if !COMPILE_10BIT
    if (seq_info->sample_bit_depth != 8) {
        com_log(COM_LOG_ERROR, "This dec lib is compiled for 8bit stream !\n");
        return 0;
    }
#endif

    seq_info->aspect_ratio_information    = U_V(bs, 4, "aspect_ratio_information");
    seq_info->frame_rate_code             = U_V(bs, 4, "frame_rate_code");

    seq_info->bit_rate_lower              = U_V(bs, 18, "bit_rate_lower");
    marker_bit                            = U_V(bs, 1,  "marker bit");
    //CHECKMARKERBIT
    seq_info->bit_rate_upper              = U_V(bs, 12, "bit_rate_upper");
    seq_info->low_delay                   = U_V(bs, 1, "low_delay");
    marker_bit                            = U_V(bs, 1, "marker bit");
    //CHECKMARKERBIT
    seq_info->temporal_id_exist_flag      = U_V(bs, 1, "cur_layer exsit flag");                           //get Extention Flag

    U_V(bs, 18, "bbv buffer size");

    seq_info->g_uiMaxSizeInBit            = U_V(bs, 3, "MaxSizeInBit");
    seq_info->seq_weighting_quant_flag    = U_V(bs, 1, "weighting_quant_flag");

    if (seq_info->seq_weighting_quant_flag) {
        int user_define_wq = U_V(bs, 1, "load_seq_weight_quant_data_flag");
        read_weight_quant_matrix(bs, seq_info->seq_wq_matrix, user_define_wq);
    }

    seq_info->background_picture_enable = 0x01 ^ U_V(bs, 1, "background_picture_disable");
    seq_info->b_mhpskip_enabled         = U_V(bs, 1, "mhpskip enabled");
    seq_info->dhp_enabled               = U_V(bs, 1, "dhp enabled");
    seq_info->wsm_enabled               = U_V(bs, 1, "wsm enabled");
    seq_info->inter_amp_enable          = U_V(bs, 1, "bbv buffer size");
    seq_info->useNSQT                   = U_V(bs, 1, "useNSQT");
    seq_info->useSDIP                   = U_V(bs, 1, "useSDIP");
    seq_info->b_secT_enabled            = U_V(bs, 1, "secT enabled");
    seq_info->sao_enable                = U_V(bs, 1, "SAO Enable Flag");
    seq_info->alf_enable                = U_V(bs, 1, "ALF Enable Flag");
    seq_info->b_pmvr_enabled            = U_V(bs, 1, "pmvr enabled");

    U_V(bs, 1, "marker bit");

    seq_info->gop_size                  = U_V(bs, 6, "num_of_RPS");
    
    for (i = 0; i < seq_info->gop_size; i++) {
        seq_info->decod_RPS[i].referd_by_others = U_V(bs, 1, "refered by others");
        seq_info->decod_RPS[i].num_of_ref = U_V(bs, 3, "num of reference picture");

        for (j = 0; j < seq_info->decod_RPS[i].num_of_ref; j++) {
            seq_info->decod_RPS[i].ref_pic[j] = U_V(bs, 6, "delta COI of ref pic");
        }
        seq_info->decod_RPS[i].num_to_remove = U_V(bs, 3, "num of removed picture");

        for (j = 0; j < seq_info->decod_RPS[i].num_to_remove; j++) {
            seq_info->decod_RPS[i].remove_pic[j] = U_V(bs, 6, "delta COI of removed pic");
        }
        U_V(bs, 1, "marker bit");
    }

    if (seq_info->low_delay == 0) {
        seq_info->picture_reorder_delay = U_V(bs, 5, "picture_reorder_delay");
    }

    seq_info->lf_cross_slice = U_V(bs, 1, "loop filter cross slice");

    if (seq_info->profile_id == RESERVED_PROFILE_ID) {
        seq_info->slice_set_enable = /*slice_set_enable =*/ 0x01 ^ U_V(bs, 1, "slice set flag");            //added by mz, 2008.04
    } else {
        U_V(bs, 2, "reseved bits");
    }

    seq_info->img_width = seq_info->horizontal_size;
    seq_info->img_height = seq_info->vertical_size;

    if (seq_info->horizontal_size % (8) != 0) {
        seq_info->auto_crop_right = (8)-(seq_info->horizontal_size % (8));
    } else {
        seq_info->auto_crop_right = 0;
    }

    if (seq_info->vertical_size % (8) != 0) {
        seq_info->auto_crop_bottom = (8)-(seq_info->vertical_size % (8));
    } else {
        seq_info->auto_crop_bottom = 0;
    }

    seq_info->img_width = (seq_info->horizontal_size + seq_info->auto_crop_right);
    seq_info->img_height = (seq_info->vertical_size + seq_info->auto_crop_bottom);
    seq_info->img_widthc = (seq_info->img_width >> 1);
    seq_info->img_heightc = (seq_info->img_height >> 1);

    seq_info->img_width_in_mcu = seq_info->img_width / 8;
    seq_info->img_height_in_mcu = seq_info->img_height / 8;
    seq_info->img_size_in_mcu = seq_info->img_width_in_mcu * seq_info->img_height_in_mcu;

    lcu_size = 1 << seq_info->g_uiMaxSizeInBit;
    seq_info->lcu_size = lcu_size;
    seq_info->img_width_in_lcu = (seq_info->img_width + lcu_size - 1) / lcu_size;
    seq_info->img_height_in_lcu = (seq_info->img_height + lcu_size - 1) / lcu_size;
    seq_info->img_size_in_lcu = seq_info->img_width_in_lcu * seq_info->img_height_in_lcu;

    seq_info->pad_size = 64 + 4 + ALIGN_BASIC;
    seq_info->pad_size -= seq_info->pad_size & ALIGN_MASK;

    seq_info->b4_info_stride = (seq_info->img_width  / 4 + 2);
    seq_info->b4_info_size   = (seq_info->img_height / 4 + 2) * seq_info->b4_info_stride;

    return 1;
}

void pic_hdr_read_rps(pic_hdr_t *pichdr, bs_stream_t *bs, seq_info_t *seq_info)
{
    {
        int RPS_idx;
        int predict;
        predict = U_V(bs, 1, "use RPS in SPS");
        if (predict) {
            RPS_idx = U_V(bs, 5, "predict for RPS");
            pichdr->curr_RPS = seq_info->decod_RPS[RPS_idx];
        }
        else {
            //gop size16
            int j;
            pichdr->curr_RPS.referd_by_others = U_V(bs, 1, "refered by others");
            pichdr->curr_RPS.num_of_ref = U_V(bs, 3, "num of reference picture");
            for (j = 0; j < pichdr->curr_RPS.num_of_ref; j++) {
                pichdr->curr_RPS.ref_pic[j] = U_V(bs, 6, "delta COI of ref pic");
            }
            pichdr->curr_RPS.num_to_remove = U_V(bs, 3, "num of removed picture");
            for (j = 0; j < pichdr->curr_RPS.num_to_remove; j++) {
                pichdr->curr_RPS.remove_pic[j] = U_V(bs, 6, "delta COI of removed pic");
            }
            U_V(bs, 1, "marker bit");
        }
    }
}

void pic_hdr_read_deblk_info(pic_hdr_t *pichdr, bs_stream_t *bs)
{
    pichdr->loop_filter_disable = U_V(bs, 1, "loop_filter_disable");

    if (!pichdr->loop_filter_disable) {
        pichdr->loop_filter_parameter_flag = U_V(bs, 1, "loop_filter_parameter_flag");

        if (pichdr->loop_filter_parameter_flag) {
            pichdr->alpha_c_offset = SE_V(bs, "alpha_offset");
            pichdr->beta_offset = SE_V(bs, "beta_offset");
        }
        else { // 20071009
            pichdr->alpha_c_offset = 0;
            pichdr->beta_offset = 0;
        }
    }
}

void pic_hdr_read_chroma_quant_param(pic_hdr_t *pichdr, bs_stream_t *bs)
{
    pichdr->chroma_quant_param_disable = U_V(bs, 1, "chroma_quant_param_disable");
    if (!pichdr->chroma_quant_param_disable) {
        pichdr->chroma_quant_param_delta_u = SE_V(bs, "chroma_quant_param_delta_u");
        pichdr->chroma_quant_param_delta_v = SE_V(bs, "chroma_quant_param_delta_v");
    }
    else {
        pichdr->chroma_quant_param_delta_u = 0;
        pichdr->chroma_quant_param_delta_v = 0;
    }
}

void read_pic_wq_params(seq_info_t * seq_info, pic_hdr_t *pichdr, bs_stream_t *bs)
{
    pichdr->weighting_quant_flag = U_V(bs, 1, "weighting_quant_flag");

    if (pichdr->weighting_quant_flag) {
        int wq_type = U_V(bs, 2, "pic_weight_quant_data_index");
        if (wq_type == 0) {
            memcpy(pichdr->pic_wq_matrix, seq_info->seq_wq_matrix, sizeof(pichdr->pic_wq_matrix));
        }
        else if (wq_type == 1) {
            int i, wq_param, wq_model;
            i32s_t wq_param_vector[6];

            U_V(bs, 1, "reserved_bits");
            wq_param = U_V(bs, 2, "weighting_quant_param_index");
            wq_model = U_V(bs, 2, "weighting_quant_model");

            if (wq_param == 0) {
                memcpy(wq_param_vector, tab_wq_param_default[1], sizeof(wq_param_vector));
            }
            else if (wq_param == 1) {
                for (i = 0; i < 6; i++) {
                    wq_param_vector[i] = SE_V(bs, "quant_param_delta_u") + tab_wq_param_default[0][i];
                }
            }
            else if (wq_param == 2) {
                for (i = 0; i < 6; i++) {
                    wq_param_vector[i] = SE_V(bs, "quant_param_delta_u") + tab_wq_param_default[1][i];
                }
            }
            else {
                assert(0);
            }
            drive_pic_wq_matrix_param(wq_param_vector, wq_model, pichdr->pic_wq_matrix);
        }
        else if (wq_type == 2) {
            read_weight_quant_matrix(bs, pichdr->pic_wq_matrix, 1);
        }
        else {
            assert(0);
        }
    }
}

void parse_i_hdr(seq_info_t * seq_info, pic_hdr_t *pichdr, bs_stream_t *bs)
{
    U_V(bs, 32, "bbv_delay");

    if (U_V(bs, 1, "time_code_flag")) {
        U_V(bs, 24, "time_code");
    }

    if (seq_info->background_picture_enable) {
        if (U_V(bs, 1, "background_picture_flag")) {
            pichdr->pic_typeb = BACKGROUND_IMG;
        } else {
            pichdr->pic_typeb = 0;
        }

        if (pichdr->pic_typeb == BACKGROUND_IMG) {
            seq_info->bg_output_flag = U_V(bs, 1, "background_picture_output_flag");
        }
    }

    pichdr->coding_order = U_V(bs, 8, "coding_order");
    if (seq_info->temporal_id_exist_flag == 1) {
        pichdr->cur_layer = U_V(bs, 3, "layer");
    } else {
        pichdr->cur_layer = 0;
    }

    if (seq_info->low_delay == 0 && !(seq_info->background_picture_enable && seq_info->bg_output_flag == 0)) {
        pichdr->displaydelay = UE_V(bs, "displaydelay");
    } else {
        pichdr->displaydelay = 0;
    }

    pic_hdr_read_rps(pichdr, bs, seq_info);

    //xyji 12.23
    if (seq_info->low_delay) {
        UE_V(bs, "bbv check times");
    }

    if (!U_V(bs, 1, "progressive_frame")) {
        int ps = U_V(bs, 1, "picture_structure");
        assert(ps == 1);
    }

    U_V(bs, 1, "top_field_first");
    U_V(bs, 1, "repeat_first_field");

    if (seq_info->is_field_sequence) {
        pichdr->is_top_field = U_V(bs, 1, "is_top_field");
        U_V(bs, 1, "reserved bit for interlace coding");
    }
    pichdr->fixed_picture_qp = U_V(bs, 1, "fixed_picture_qp");
    pichdr->picture_qp = U_V(bs, 7, "picture_qp");

    pic_hdr_read_deblk_info(pichdr, bs);
    pic_hdr_read_chroma_quant_param(pichdr, bs);

    if (seq_info->seq_weighting_quant_flag) {
        read_pic_wq_params(seq_info, pichdr, bs);
    } else {
        pichdr->weighting_quant_flag = 0;
    }

    pichdr->pic_type = I_IMG;

    if (seq_info->alf_enable) {
        Read_ALF_param(pichdr, bs);
    }
}

/*
*************************************************************************
* Function:pb picture header
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
int parse_pb_hdr(seq_info_t * seq_info, pic_hdr_t *pichdr, bs_stream_t *bs)
{
    int random_access_flag;

    U_V(bs, 32, "bbv delay");

    pichdr->picture_coding_type = U_V(bs, 2, "picture_coding_type");

    if (seq_info->background_picture_enable && (pichdr->picture_coding_type == 1 || pichdr->picture_coding_type == 3)) {
        int bg_pref_flag = 0;
        if (pichdr->picture_coding_type == 1) {
            bg_pref_flag = U_V(bs, 1, "background_pred_flag");
        } 

        if (bg_pref_flag == 0) {
            pichdr->background_reference_enable = U_V(bs, 1, "background_reference_enable");
            pichdr->pic_typeb = 0;
        } else {
            pichdr->background_reference_enable = 0;
            if (pichdr->picture_coding_type == 1) {
                pichdr->pic_typeb = BP_IMG;
            } else {
                pichdr->pic_typeb = 0;
            }
        }
    } else {
        pichdr->background_reference_enable = 0;
        pichdr->pic_typeb = 0;
    }

    pichdr->coding_order = U_V(bs, 8, "coding_order");
    if (seq_info->temporal_id_exist_flag == 1) {
        pichdr->cur_layer = U_V(bs, 3, "layer");
    } else {
        pichdr->cur_layer = 0;
    }

    if (seq_info->low_delay == 0) {
        pichdr->displaydelay = UE_V(bs, "displaydelay");
    } else {
        pichdr->displaydelay = 0;
    }

    pic_hdr_read_rps(pichdr, bs, seq_info);

    //xyji 12.23
    if (seq_info->low_delay) {
        UE_V(bs, "bbv check times");
    }

    if (!U_V(bs, 1, "progressive_frame")) {
        int ps = U_V(bs, 1, "picture_structure");
        assert(ps == 1);
    }

    U_V(bs, 1, "top_field_first");
    U_V(bs, 1, "repeat_first_field");

    if (seq_info->is_field_sequence) {
        pichdr->is_top_field = U_V(bs, 1, "is_top_field");
        U_V(bs, 1, "reserved bit for interlace coding");
    }

    pichdr->fixed_picture_qp = U_V(bs, 1, "fixed_picture_qp");
    pichdr->picture_qp = U_V(bs, 7, "picture_qp");

    if (2 != pichdr->picture_coding_type) {
        U_V(bs, 1, "reserved bit");
    }

    random_access_flag = U_V(bs, 1, "random_access_decodable_flag");

    pic_hdr_read_deblk_info(pichdr, bs);
    pic_hdr_read_chroma_quant_param(pichdr, bs);

    if (seq_info->seq_weighting_quant_flag) {
        read_pic_wq_params(seq_info, pichdr, bs);
    } else {
        pichdr->weighting_quant_flag = 0;
    }

    if (pichdr->picture_coding_type == 1) {
        pichdr->pic_type = P_IMG;
    } else if (pichdr->picture_coding_type == 3) {
        pichdr->pic_type = F_IMG;
    } else {
        pichdr->pic_type = B_IMG;
    }

    if (seq_info->alf_enable) {
        Read_ALF_param(pichdr, bs);
    }

    return random_access_flag;
}

/*
*************************************************************************
* Function:To calculate the poc values
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/

void calc_picture_distance(avs2_dec_t *h_dec)
{
    // for POC mode 0:
    if (h_dec->pic_hdr.coding_order  <  h_dec->PrevPicDistanceLsb) {
        h_dec->CurrPicDistanceMsb += 256;
    }
    h_dec->pic_hdr.coding_order += h_dec->CurrPicDistanceMsb;

    if (h_dec->seq->low_delay == 0) {
        h_dec->tr = h_dec->pic_hdr.coding_order + h_dec->pic_hdr.displaydelay - h_dec->seq->picture_reorder_delay;
    } else {
        h_dec->tr = h_dec->pic_hdr.coding_order;
    }

    h_dec->img_tr_ext = h_dec->tr % 256;
}

i32u_t parse_slice_hdr(avs2_dec_t *h_dec, bs_stream_t *bs)
{
    int mb_row;
    int mb_column;
    int mb_width;
    const seq_info_t *seq = h_dec->seq;
    slice_hdr_t *slihdr = &h_dec->slice_hdr;
    bs->p_start_test--;

    slihdr->slice_vertical_position = U_V(bs, 8, "slice vertical position");

    if (seq->vertical_size > (144 * (1 << seq->g_uiMaxSizeInBit))) {
        slihdr->slice_vertical_position_extension = U_V(bs, 3, "slice vertical position extension");
    }

    if (seq->vertical_size > (144 * (1 << seq->g_uiMaxSizeInBit))) {
        mb_row = (slihdr->slice_vertical_position_extension << 7) + slihdr->slice_vertical_position;
    } else {
        mb_row = slihdr->slice_vertical_position;
    }

    slihdr->slice_horizontal_positon = U_V(bs, 8, "slice horizontal position");
    if (seq->horizontal_size > (255 * (1 << seq->g_uiMaxSizeInBit))) {
        slihdr->slice_horizontal_positon_extension = U_V(bs, 2, "slice horizontal position extension");
    }

    if (seq->profile_id == RESERVED_PROFILE_ID) {
        if (seq->slice_set_enable) {
            slihdr->slice_horizontal_positon = U_V(bs, 8, "slice horizontal position");

            if (seq->horizontal_size > 4080) {
                slihdr->slice_horizontal_positon_extension = U_V(bs, 2, "slice horizontal position extension");
            }

            slihdr->current_slice_set_index = U_V(bs, 6, "slice set index");
            slihdr->current_slice_header_flag = U_V(bs, 1, "slice header flag");

            if (seq->horizontal_size > 4080) {
                mb_column = (slihdr->slice_horizontal_positon_extension << 8) + slihdr->slice_horizontal_positon;
            } else {
                mb_column = slihdr->slice_horizontal_positon;
            }
        }
    }

    mb_width  = (seq->horizontal_size + 8 - 1) / 8;

    if (!seq->slice_set_enable || (seq->slice_set_enable && slihdr->current_slice_header_flag)) {   //added by mz, 2008.04
        if (!h_dec->pic_hdr.fixed_picture_qp) {
            slihdr->fixed_slice_qp = U_V(bs, 1, "fixed_slice_qp");
            slihdr->slice_qp = U_V(bs, 7, "slice_qp");
        } else {
            slihdr->fixed_slice_qp = 1;
            slihdr->slice_qp = h_dec->pic_hdr.picture_qp;
        }
    } else {
        slihdr->fixed_slice_qp = 1;
    }

    if (seq->sao_enable) {
        slihdr->slice_sao_on[0] = U_V(bs, 1, "sao_slice_flag_Y");
        slihdr->slice_sao_on[1] = U_V(bs, 1, "sao_slice_flag_Cb");
        slihdr->slice_sao_on[2] = U_V(bs, 1, "sao_slice_flag_Cr");
    }

    // align bit pos
    if (bs->bits_to_go != 7) {
        bs->p_start_test++;
        bs->bits_to_go = 7;
    }
    return (bs->p_start_test[-2] << 8) | bs->p_start_test[-1];
}

/*
*************************************************************************
* Function:Error handling procedure. Print error message to stderr and exit
with supplied code.
* Input:text
Error message
* Output:
* Return:
* Attention:
*************************************************************************
*/

void error(char *text, int code)
{
    fprintf(stderr, "%s\n", text);
    exit(code);
}
