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

#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include <assert.h>
#include <math.h>

#include "global.h"
#include "commonVariables.h"

#include "ComAdaptiveLoopFilter.h"
#include "DecAdaptiveLoopFilter.h"
#include "header.h"
#include "bitstream.h"
#include "wquant.h"
#include "inter-prediction.h"
#include "uavs2d.h"

#ifndef MAXUINT64
#define MAXUINT64   ((i64u_t)~((i64u_t)0))
#define MAXINT64    ((i64s_t)(MAXUINT64 >> 1))
#endif

void init_seq_buffers(avs2_dec_ctrl_t *ctrl)
{
    int i;
    int pad = ctrl->seq_info.pad_size;

    com_frm_create(ctrl, &ctrl->frm_bg, pad);

    for (i = 0; i < ctrl->ref_buf_frames; i++) {
        com_frm_create(ctrl, &ctrl->frm_data_buf[i], pad);
        ctrl->frm_buf[i] = &ctrl->frm_data_buf[i];
    }
}

void free_seq_buffers(avs2_dec_ctrl_t *ctrl)
{
    int i;

    com_frm_destroy(ctrl, &ctrl->frm_bg);

    for (i = 0; i < ctrl->ref_buf_frames; i++) {
        com_frm_destroy(ctrl, &ctrl->frm_data_buf[i]);
    }
}

void rec_core_init(avs2_dec_t *h_dec)
{
    int i;
    com_rec_t *rec = h_dec->rec;

    rec->seq           = h_dec->seq;
    rec->pichdr        = &h_dec->pic_hdr;
    rec->slihdr        = &h_dec->slice_hdr;
    rec->bw_mv         = h_dec->bw_mv;
    rec->bw_ref        = h_dec->bw_ref;
    rec->snd_ref       = h_dec->snd_ref;
    rec->ipredmode     = h_dec->ipredmode;
    rec->ref_list      = h_dec->ref_list;
    rec->cu_array      = h_dec->cu_array;
    rec->lcu_array     = h_dec->lcu_array;
    rec->check_ref     = h_dec->i_thd_idx;
    rec->g_pic_flt_tmp = h_dec->g_pic_flt_tmp;
    rec->pmutex        = &h_dec->mutex;
    rec->pcond         = &h_dec->cond;
    rec->alf_params    = &h_dec->alf_params;

    rec->sao_blk_params_rec = h_dec->sao_blk_params_rec;

    rec->i_aec_finished_idx = &h_dec->i_aec_finished_idx;
    rec->i_rec_finished_idx = &h_dec->i_rec_finished_idx;

    rec->deblock_edge_flag[0] = h_dec->deblock_edge_flag[0];
    rec->deblock_edge_flag[1] = h_dec->deblock_edge_flag[1];

    rec->deblock_skip_flag[0] = h_dec->deblock_skip_flag[0];
    rec->deblock_skip_flag[1] = h_dec->deblock_skip_flag[1];

    rec->filter_bak_rec[0] = h_dec->filter_bak_rec[0];
    rec->filter_bak_rec[1] = h_dec->filter_bak_rec[1];
    rec->filter_bak_rec[2] = h_dec->filter_bak_rec[2];

    for (i = 1; i < h_dec->i_rec_threads; i++) {
        memcpy(&h_dec->rec[i], rec, sizeof(com_rec_t));
    }

}

i64s_t dec_core_buf_create(avs2_dec_t *h_dec, seq_info_t * seq)
{
    int total_mem_size;
    uchar_t *buf;
    int lcu_array_size = 1;
    int rec_cores = 1;

    seq->b4_info_size = (seq->img_height / MIN_BLOCK_SIZE + 2) * (seq->img_width / MIN_BLOCK_SIZE + 2);
    seq->b4_info_stride = (seq->img_width / MIN_BLOCK_SIZE + 2);

    if (h_dec->i_rec_threads) {
        lcu_array_size = seq->img_size_in_lcu;
        rec_cores = h_dec->i_rec_threads;
    }

    /* alloc buffers */
    total_mem_size = seq->img_size_in_mcu * sizeof(com_cu_t)       +     ALIGN_MASK +  // cu_array
                     sizeof(com_lcu_t)* lcu_array_size             +     ALIGN_MASK +  // lcu_array
                     sizeof(com_rec_t)* rec_cores                  +     ALIGN_MASK +  // rec
                     seq->img_width *sizeof(pel_t) * 2             + 3 * ALIGN_MASK +  // filter_bak_rec[3]
                     seq->b4_info_size * sizeof(char_t)            +     ALIGN_MASK +  // ipredmode
                     seq->b4_info_size * sizeof(char_t)            +     ALIGN_MASK +  // nz
                     seq->b4_info_size * sizeof(char_t)            +     ALIGN_MASK +  // pdir
                     seq->b4_info_size * sizeof(char_t)            +     ALIGN_MASK +  // bw_ref
                     seq->b4_info_size * sizeof(char_t)            +     ALIGN_MASK +  // snd_ref
                     seq->b4_info_size * sizeof(i16s_t)* 2         + 2 * ALIGN_MASK +  // bw_mv
                     sizeof(sap_param_t)* seq->img_size_in_lcu * 3 +     ALIGN_MASK +  // sao_blk_params
                     sizeof(sap_param_t)* seq->img_size_in_lcu * 3 +     ALIGN_MASK +  // sao_blk_params_rec
                     seq->img_size_in_mcu * sizeof(uchar_t) * 2        + 2 * ALIGN_MASK +  // deblock_edge_flag
                     seq->img_size_in_mcu * sizeof(uchar_t) * 4        + 2 * ALIGN_MASK +  // deblock_skip_flag
                     0;


#define GIVE_BUFFER(d, p, s, t) \
    (d) = (t)(p); \
    p += s; \
    p = ALIGN_POINTER(p);

    h_dec->dec_info_buffer = (char_t*)com_malloc(total_mem_size);
    buf = (uchar_t *) h_dec->dec_info_buffer;

    GIVE_BUFFER(h_dec->cu_array,  buf, seq->img_size_in_mcu * sizeof(com_cu_t), com_cu_t*);
    GIVE_BUFFER(h_dec->lcu_array, buf, lcu_array_size * sizeof(com_lcu_t), com_lcu_t*);
    GIVE_BUFFER(h_dec->rec,       buf, rec_cores * sizeof(com_rec_t), com_rec_t*);
    GIVE_BUFFER(h_dec->filter_bak_rec[0], buf, seq->img_width * sizeof(pel_t), pel_t*);
    GIVE_BUFFER(h_dec->filter_bak_rec[1], buf, seq->img_width / 2 * sizeof(pel_t), pel_t*);
    GIVE_BUFFER(h_dec->filter_bak_rec[2], buf, seq->img_width / 2 * sizeof(pel_t), pel_t*);


    GIVE_BUFFER(h_dec->ipredmode, buf, seq->b4_info_size * sizeof(char_t), char_t*);
    h_dec->ipredmode += seq->b4_info_stride + 1;

    GIVE_BUFFER(h_dec->nz, buf, seq->b4_info_size * sizeof(char_t), char_t*);
    h_dec->nz += seq->b4_info_stride + 1;

    GIVE_BUFFER(h_dec->bw_ref, buf, seq->b4_info_size * sizeof(char_t), char_t*);
    h_dec->bw_ref += seq->b4_info_stride + 1;

    GIVE_BUFFER(h_dec->pdir, buf, seq->b4_info_size * sizeof(char_t), char_t*);
    h_dec->pdir += seq->b4_info_stride + 1;

    GIVE_BUFFER(h_dec->snd_ref, buf, seq->b4_info_size * sizeof(char_t), char_t*);
    h_dec->snd_ref += seq->b4_info_stride + 1;

    GIVE_BUFFER(h_dec->bw_mv, buf, seq->b4_info_size * sizeof(i16s_t)* 2, i16s_t(*)[2]);
    h_dec->bw_mv += seq->b4_info_stride + 1;

    GIVE_BUFFER(h_dec->sao_blk_params, buf, sizeof(sap_param_t)* seq->img_size_in_lcu * 3, sap_param_t(*)[3]);
    GIVE_BUFFER(h_dec->sao_blk_params_rec, buf, sizeof(sap_param_t)* seq->img_size_in_lcu * 3, sap_param_t(*)[3]);
    GIVE_BUFFER(h_dec->deblock_edge_flag[0], buf, seq->img_size_in_mcu * sizeof(uchar_t), uchar_t*);
    GIVE_BUFFER(h_dec->deblock_edge_flag[1], buf, seq->img_size_in_mcu * sizeof(uchar_t), uchar_t*);
    GIVE_BUFFER(h_dec->deblock_skip_flag[0], buf, seq->img_size_in_mcu * 2 * sizeof(uchar_t), uchar_t*);
    GIVE_BUFFER(h_dec->deblock_skip_flag[1], buf, seq->img_size_in_mcu * 2 * sizeof(uchar_t), uchar_t*);

#undef GIVE_BUFFER

    h_dec->g_pic_flt_tmp = com_pic_yuv_create(seq->img_width, seq->img_height, seq->pad_size);

    total_mem_size += (int)h_dec->g_pic_flt_tmp->pic_memory;
    total_mem_size += alf_buf_create(h_dec, seq);

    if (h_dec->i_rec_threads) {
        avs2_threadpool_init(&h_dec->rec_thread_pool, h_dec->i_rec_threads, NULL, NULL);
        avs2_pthread_mutex_init(&h_dec->mutex, NULL);
        avs2_pthread_cond_init(&h_dec->cond, NULL);
    }

    rec_core_init(h_dec);

    return total_mem_size;
}

void dec_core_buf_free(avs2_dec_t *h_dec)
{
    if (h_dec->i_rec_threads) {
        avs2_threadpool_delete(h_dec->rec_thread_pool);
        avs2_pthread_mutex_destroy(&h_dec->mutex);
        avs2_pthread_cond_destroy(&h_dec->cond);
    }
    com_free(h_dec->dec_info_buffer);
    com_pic_yuv_destroy(h_dec->g_pic_flt_tmp);
    alf_buf_destroy(h_dec);
}


void cpy_seq_info(avs2_frame_t *frm, seq_info_t *hdr)
{
    frm->info.img_width = hdr->horizontal_size;
    frm->info.img_height = hdr->vertical_size;
    frm->info.profile_id = hdr->profile_id;
    frm->info.level_id = hdr->level_id;
    frm->info.progressive_seq = hdr->progressive_sequence;
    frm->info.output_bit_depth = hdr->output_bit_depth;
    frm->info.frame_rate_code = hdr->frame_rate_code;


    //add by Victor
    frm->info.seq_info.profile_id=hdr->profile_id;
    frm->info.seq_info.level_id=hdr->level_id;
    frm->info.seq_info.progressive_sequence=hdr->progressive_sequence;
    frm->info.seq_info.sample_bit_depth=hdr->sample_bit_depth;

}

/*
*************************************************************************
* Function:Write decoded frame to output file
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/
// Fix by Sunil for RD5.0 test in Linux (2013.11.06)
void write_frame(avs2_dec_ctrl_t *ctrl, avs2_frame_t *frm, i64s_t pos)          //!< filestream to output file
{
    int j;
    seq_info_t *seq = &ctrl->seq_info;
    int img_width     = (seq->img_width - seq->auto_crop_right);
    int img_height    = (seq->img_height - seq->auto_crop_bottom);
    int img_width_cr  = (img_width / 2);
    int img_height_cr = (img_height / 2);

    pel_t *src=NULL, *srcu=NULL, *srcv=NULL;
    int i_src=0, i_srcc=0;
    uchar_t *dst=NULL, *dstu=NULL, *dstv=NULL;
    int i_dst=0, i_dstc=0;

    dst    = frm->p_buf_y;
    dstu   = frm->p_buf_u;
    dstv   = frm->p_buf_v;
    i_dst  = frm->i_stride;
    i_dstc = frm->i_stridec;

    if (pos < 0) {
        com_pic_t *pic = ctrl->frm_bg.yuv_data;
        src    = pic->p_y;
        i_src  = pic->i_stride;
        srcu   = pic->p_u;
        srcv   = pic->p_v;
        i_srcc = pic->i_stridec;
    } else {
        for (j = 0; j < ctrl->ref_buf_frames; j++) {
            if (ctrl->frm_buf[j]->bak_imgtr == pos) {
                com_frm_t *frm_l = ctrl->frm_buf[j];
                com_pic_t *pic = frm_l->yuv_data;

                src    = pic->p_y;
                i_src  = pic->i_stride;
                srcu   = pic->p_u;
                srcv   = pic->p_v;
                i_srcc = pic->i_stridec;

                frm_l->usecnt--;

                if (frm_l->b_refed == 0) {
                    assert(frm_l->usecnt == 0);
                    frm_l->coiref = -1;
                    frm_l->layer = -1;
                }
                break;
            }
        }
    }
    
    if (frm->i_output_type == AVS2_OUT_I420) {

        //ffmpeg cli has a bug can't process yuv420p10le input

        //printf("output 10bit!i_dst %d,"
        //       " i_src %d,"
        //       "i_srcc %d "
        //       "seq->sample_bit_depth %d"
        //       "\n",
        //       i_dst,
        //       i_src,
        //       i_srcc,
        //       seq->sample_bit_depth
        //       );
        //g_funs_handle.cpy_pel_to_uchar(src, i_src, dst, i_dst, img_width, img_height, seq->sample_bit_depth);
        //g_funs_handle.cpy_pel_to_uchar(srcu, i_srcc, dstu, i_dstc, img_width_cr, img_height_cr, seq->sample_bit_depth);
        //g_funs_handle.cpy_pel_to_uchar(srcv, i_srcc, dstv, i_dstc, img_width_cr, img_height_cr, seq->sample_bit_depth);


        /* below only for 10-bit to 8-bit output

        for (int i = 0; i < img_height; i++) {
            for (j = 0; j < img_width; j++) {
                dst[j] = (uchar_t)COM_CLIP3(0, 255, (src[j] + 2) >> 2);
            }
            src += i_src;
            dst += i_dst;
        }

        for (int i = 0; i < img_height_cr; i++) {
            for (j = 0; j < img_width_cr; j++) {
                dstu[j] = (uchar_t)COM_CLIP3(0, 255, (srcu[j] + 2) >> 2);
                dstv[j] = (uchar_t)COM_CLIP3(0, 255, (srcv[j] + 2) >> 2);
            }
            srcu += i_srcc;
            dstu += i_dstc;
            srcv += i_srcc;
            dstv += i_dstc;
        }

         */

        if (seq->output_bit_depth == 8) {
            g_funs_handle.cpy_pel_to_uchar(src, i_src, dst, i_dst, img_width, img_height, seq->sample_bit_depth);
            g_funs_handle.cpy_pel_to_uchar(srcu, i_srcc, dstu, i_dstc, img_width_cr, img_height_cr, seq->sample_bit_depth);
            g_funs_handle.cpy_pel_to_uchar(srcv, i_srcc, dstv, i_dstc, img_width_cr, img_height_cr, seq->sample_bit_depth);
        } else { // output 10bit
            assert(seq->sample_bit_depth == 10);
        //    printf("output 10bit!i_dst %d, i_src %d,i_srcc %d \n",i_dst,i_src,i_srcc);
            for (j = 0; j < img_height; j++) {
                memcpy(dst, src, img_width * 2);
                dst += i_dst;
                src += i_src;
            }
            for (j = 0; j < img_height_cr; j++) {
                memcpy(dstu, srcu, img_width_cr * 2 );
                memcpy(dstv, srcv, img_width_cr * 2 );
                dstu += i_dstc;
                dstv += i_dstc;
                srcu += i_srcc;
                srcv += i_srcc;
            }
        }
    } else if (frm->i_output_type == AVS2_OUT_YUY2){
        g_funs_handle.cpy_pel_I420_to_uchar_YUY2(src, srcu, srcv, i_src, i_srcc, dst, i_dst, img_width, img_height, seq->sample_bit_depth);
    } else {
        assert(0);
    }
}

void report_frame(avs2_dec_t *h_dec, avs2_frame_t *frm, outdata *out, int pos)
{
    frm->qp = (unsigned int) out->stdoutdata[pos].qp;
    frm->pts = out->stdoutdata[pos].pts;

    if (out->stdoutdata[pos].type == I_IMG) { // I picture
        if (out->stdoutdata[pos].typeb == BACKGROUND_IMG) {
            if (h_dec->seq->bg_output_flag) {
                frm->frm_type = AVS2_G_IMG;
            } else {
                frm->frm_type = AVS2_GB_IMG;
            }
        } else {
            frm->frm_type = AVS2_I_IMG;
        }
    } else if (out->stdoutdata[pos].type == P_IMG && out->stdoutdata[pos].typeb != BP_IMG) {
        frm->frm_type = AVS2_P_IMG;
    } else if (out->stdoutdata[pos].type == P_IMG && out->stdoutdata[pos].typeb == BP_IMG) {
        frm->frm_type = AVS2_S_IMG;
    } else if (out->stdoutdata[pos].type == F_IMG) { // F pictures
        frm->frm_type = AVS2_F_IMG;
    } else { // B pictures
        frm->frm_type = AVS2_B_IMG;
    }
}

void delete_trbuffer(outdata *out, int pos)
{
    int i;
    for (i = pos; i < out->buffer_num - 1; i++) {
        out->stdoutdata[i] = out->stdoutdata[i + 1];
    }
    out->buffer_num--;
}

int out_put_frame(avs2_dec_ctrl_t *ctrl, avs2_dec_t *h_dec, avs2_frame_t *frm)
{
    outdata *outprint = &ctrl->outprint;
    int pointer_tmp = outprint->buffer_num;
    int i;

    assert(pointer_tmp < 8);

    h_dec->PrevPicDistanceLsb = (int) (h_dec->pic_hdr.coding_order % 256);

    outprint->stdoutdata[pointer_tmp].type = h_dec->type;
    outprint->stdoutdata[pointer_tmp].typeb = h_dec->typeb;
    outprint->stdoutdata[pointer_tmp].framenum = h_dec->tr;
    outprint->stdoutdata[pointer_tmp].tr = h_dec->tr;
    outprint->stdoutdata[pointer_tmp].qp = h_dec->pic_hdr.picture_qp;
    outprint->stdoutdata[pointer_tmp].pts = h_dec->pts;
    outprint->buffer_num++;

    if (h_dec->pic_hdr.coding_order >= h_dec->seq->picture_reorder_delay + ctrl->min_coding_order) {
        i64s_t tmp_min;
        int pos = -1;
        tmp_min = MAXINT64;

        for (i = 0; i < outprint->buffer_num; i++) {
            if (outprint->stdoutdata[i].tr < tmp_min && outprint->stdoutdata[i].tr >= ctrl->last_output) {
                pos = i;
                tmp_min = outprint->stdoutdata[i].tr;

            }
        }

        if (pos != -1) {
            ctrl->last_output = outprint->stdoutdata[pos].tr;
            report_frame(h_dec, frm, outprint, pos);
            if (outprint->stdoutdata[pos].typeb == BACKGROUND_IMG && h_dec->seq->bg_output_flag == 0) {
#if OUTPUT_BG_FRM
                write_frame(ctrl, frm, -1);
                delete_trbuffer(outprint, pos);
                return 1;
#else
                delete_trbuffer(outprint, pos);
                return 0;
#endif
            } else {
                write_frame(ctrl, frm, outprint->stdoutdata[pos].tr);
                delete_trbuffer(outprint, pos);
                return 1;
            }
        }
    }
    return 0;
}


void prepare_RefInfo(avs2_dec_ctrl_t *ctrl, avs2_dec_t *h_dec)
{
    int i, j;
    com_frm_t* tmp_com_pic;
    int flag = 0;
    int i_refs;
    int del_frm_before_IDR = 0;

    /* update IDR idx */
    if (h_dec->tr > ctrl->next_IDRtr && ctrl->curr_IDRtr != ctrl->next_IDRtr) {
        ctrl->curr_IDRtr  = ctrl->next_IDRtr;
        ctrl->curr_IDRcoi = ctrl->next_IDRcoi;
        del_frm_before_IDR = 1;
    }
    
    i_refs = h_dec->pic_hdr.curr_RPS.num_of_ref;

    /* re-order reference buffer for current frame */
    for (i = 0; i < h_dec->pic_hdr.curr_RPS.num_of_ref; i++) {
        int idx = (int)(h_dec->pic_hdr.coding_order - h_dec->pic_hdr.curr_RPS.ref_pic[i]);
        tmp_com_pic = ctrl->frm_buf[i];

        for (j = i; j < ctrl->ref_buf_frames; j++) {
            if (ctrl->frm_buf[j]->coiref == idx) {
                break;
            }
        }
        if (j == ctrl->ref_buf_frames) {
            i_refs--;
        }
        if (j != ctrl->ref_buf_frames) {
            ctrl->frm_buf[i] = ctrl->frm_buf[j];
            ctrl->frm_buf[j] = tmp_com_pic;
        }
    }

    /* delete the frame that will never be used */
    if (del_frm_before_IDR) {
        for (i = 0; i < ctrl->ref_buf_frames; i++) {
            if (ctrl->frm_buf[i]->coiref >=0 && ctrl->frm_buf[i]->imgtr < ctrl->curr_IDRtr && ctrl->frm_buf[i]->b_refed) {
                ctrl->frm_buf[i]->usecnt--;
                ctrl->frm_buf[i]->coiref = -1;
                ctrl->frm_buf[i]->layer = -1;
            }
        }
    } else {
        for (i = 0; i < h_dec->pic_hdr.curr_RPS.num_to_remove; i++) {
            for (j = 0; j < ctrl->ref_buf_frames; j++) {
                int remove_idx = (int)(h_dec->pic_hdr.coding_order - h_dec->pic_hdr.curr_RPS.remove_pic[i]);
                if (remove_idx >= 0 && ctrl->frm_buf[j]->coiref == remove_idx) {
                    break;
                }
            }
            if (j < ctrl->ref_buf_frames) {
                assert(ctrl->frm_buf[j]->b_refed);
                ctrl->frm_buf[j]->usecnt--;
                ctrl->frm_buf[j]->coiref = -1;
                ctrl->frm_buf[j]->layer = -1;
            }
        }
    }

    /* init reference list */
    if (h_dec->type != I_IMG) {
        if (h_dec->type == P_IMG && h_dec->typeb == BP_IMG) { // S frame
            i_refs = 1;
            h_dec->ref_list[0].frm = &ctrl->frm_bg;
            h_dec->ref_list[0].frm->imgtr = ctrl->frm_buf[0]->imgtr;
        } else {
            for (j = 0; j < i_refs; j++) {
                h_dec->ref_list[j].frm = ctrl->frm_buf[j];
            }
            if (h_dec->pic_hdr.background_reference_enable && i_refs >= 2) {
                int bg_idx = i_refs - 1;
                h_dec->ref_list[bg_idx].frm = &ctrl->frm_bg;
                h_dec->ref_list[bg_idx].frm->imgtr = ctrl->frm_buf[bg_idx]->imgtr;
            }
        }

        h_dec->imgtr_next_P = h_dec->type == B_IMG ? ctrl->frm_buf[0]->imgtr : h_dec->tr;

        if (h_dec->type == B_IMG) {
            h_dec->ref_fwd = &h_dec->ref_list[1];
            h_dec->ref_bwd = &h_dec->ref_list[0];
        } else {
            h_dec->ref_fwd = NULL;
            h_dec->ref_bwd = NULL;
        }
    } else {
        h_dec->ref_list[0].frm = NULL;
        i_refs = 0;
    }
    h_dec->i_refs = i_refs;

    for (i = 0; i < h_dec->i_refs; i++) {
        h_dec->ref_list[i].frm->usecnt++;
    }

    /* find a free img buffer */
    for (i = 0; i < ctrl->ref_buf_frames; i++) {
        if (ctrl->frm_buf[i]->usecnt == 0) {
            break;
        }
    }

    assert(i < ctrl->ref_buf_frames);

    h_dec->frm_cur = ctrl->frm_buf[i];
    
    /* init current frame */
    h_dec->frm_cur->imgtr = h_dec->tr;
    h_dec->frm_cur->bak_imgtr = h_dec->frm_cur->imgtr;
    h_dec->frm_cur->coiref = h_dec->pic_hdr.coding_order;
    h_dec->frm_cur->layer = h_dec->pic_hdr.cur_layer;
    h_dec->frm_cur->b_refed = h_dec->pic_hdr.curr_RPS.referd_by_others;

    if (h_dec->frm_cur->b_refed) {
        h_dec->frm_cur->usecnt = 2;
    } else {
        h_dec->frm_cur->usecnt = 1;
    }

    if (h_dec->type != B_IMG) {
        for (j = 0; j < i_refs; j++) {
            h_dec->frm_cur->reftrs[j] = ctrl->frm_buf[j]->imgtr;
        }

        for (j = i_refs; j < 4; j++) {
            h_dec->frm_cur->reftrs[j] = 0;
        }
    } else {
        h_dec->frm_cur->reftrs[0] = ctrl->frm_buf[1]->imgtr;
        h_dec->frm_cur->reftrs[1] = ctrl->frm_buf[0]->imgtr;
        h_dec->frm_cur->reftrs[2] = 0;
        h_dec->frm_cur->reftrs[3] = 0;
    }

    if (h_dec->type == I_IMG) {
        for (j = 0; j < 4; j++) {
            h_dec->frm_cur->reftrs[j] = h_dec->tr;
        }
    }
}


void init_frame_decoder(avs2_dec_ctrl_t *ctrl, avs2_dec_t* h_dec, pic_hdr_t *pichdr, uchar_t *start, uchar_t *end, i64u_t pts)
{
    int i;
    seq_info_t *seq = &ctrl->seq_info;
    h_dec->type = pichdr->pic_type;
    h_dec->typeb = pichdr->pic_typeb;
    h_dec->bs_start = start;
    h_dec->bs_end = end;
    h_dec->pts = pts;
    h_dec->i_frame_num = ctrl->next_frm_num;

    ctrl->next_frm_num++;

    memcpy(&h_dec->pic_hdr, pichdr, sizeof(pic_hdr_t));

    calc_picture_distance(h_dec);

    if (ctrl->min_coding_order == MAXINT64 - 256 && h_dec->type == I_IMG) {
        ctrl->min_coding_order = h_dec->pic_hdr.coding_order + h_dec->pic_hdr.displaydelay - h_dec->seq->picture_reorder_delay;
    }

    if (h_dec->type != B_IMG) {
        ctrl->ip_cnt++;
    }

    if (ctrl->new_hdr) {
        ctrl->new_hdr = 0;
        ctrl->next_IDRtr = h_dec->tr;
        ctrl->next_IDRcoi = h_dec->pic_hdr.coding_order;
    }

    for (i = 0; i < h_dec->seq->img_size_in_mcu; i++) {
        h_dec->cu_array[i].slice_nr = -1;
        h_dec->cu_array[i].c_ipred_mode = 1;
        h_dec->cu_array[i].b_decoded = 0;
    }

    if (h_dec->typeb == BACKGROUND_IMG && h_dec->seq->bg_output_flag == 0) {
        h_dec->frm_cur = &ctrl->frm_bg;
        h_dec->i_refs = 0;
        h_dec->ref_list[0].frm = NULL;
    } else {
        prepare_RefInfo(ctrl, h_dec);
    }

    memset(h_dec->deblock_edge_flag[0], 0, seq->img_size_in_mcu);
    memset(h_dec->deblock_edge_flag[1], 0, seq->img_size_in_mcu);
    memset(h_dec->deblock_skip_flag[0], 0, seq->img_size_in_mcu * 2);
    memset(h_dec->deblock_skip_flag[1], 0, seq->img_size_in_mcu * 2);
    memset(h_dec->ipredmode       - 1 - seq->b4_info_stride,  0, seq->b4_info_size * sizeof(char_t));
    memset(h_dec->nz              - 1 - seq->b4_info_stride,  0, seq->b4_info_size * sizeof(char_t));
    memset(h_dec->bw_ref          - 1 - seq->b4_info_stride, -1, seq->b4_info_size * sizeof(char_t));
    memset(h_dec->snd_ref         - 1 - seq->b4_info_stride, -1, seq->b4_info_size * sizeof(char_t));
    memset(h_dec->frm_cur->refbuf - 1 - seq->b4_info_stride, -1, seq->b4_info_size * sizeof(char_t));
    memset(h_dec->bw_mv           - 1 - seq->b4_info_stride,  0, seq->b4_info_size * sizeof(i16s_t) * 2);
    memset(h_dec->frm_cur->mvbuf  - 1 - seq->b4_info_stride,  0, seq->b4_info_size * sizeof(i16s_t) * 2);
    memset(h_dec->pdir            - 1 - seq->b4_info_stride, -1, seq->b4_info_size * sizeof(char_t));

    set_ref_info(h_dec);

    h_dec->frm_cur->linend = -h_dec->seq->pad_size;

    if (h_dec->pic_hdr.weighting_quant_flag) {
        h_dec->cur_wq_matrix[0] = h_dec->pic_hdr.pic_wq_matrix[0];
        h_dec->cur_wq_matrix[1] = h_dec->pic_hdr.pic_wq_matrix[1];
        cal_all_size_wq_matrix(h_dec->cur_wq_matrix, h_dec->pic_hdr.pic_wq_matrix[1]);
    }
}

void finish_frame_decoder(avs2_dec_ctrl_t *ctrl, avs2_dec_t* h_dec, avs2_frame_t *frm)
{
    int i;

    for (i = 0; i < h_dec->i_refs; i++) {
        h_dec->ref_list[i].frm->usecnt--;
    }

    if (h_dec->typeb == BACKGROUND_IMG && h_dec->seq->background_picture_enable && (&ctrl->frm_bg != h_dec->frm_cur)) {
        com_pic_yuv_copy_ext(ctrl->frm_bg.yuv_data, h_dec->frm_cur->yuv_data);
        ctrl->frm_bg.linend = h_dec->frm_cur->linend;
        ctrl->frm_bg.usecnt = 0;
    }

    if (out_put_frame(ctrl, h_dec, frm)){
        frm->dec_stats = AVS2_TYPE_DECODED;
    } else {
        frm->dec_stats = AVS2_TYPE_NEEDMORE;
    }
}

AVS2_API void *__cdecl uavs2d_lib_create(int frm_threads, int rec_threads)
{
    avs2_dec_ctrl_t *ctrl;
    int i;

    ctrl = (avs2_dec_ctrl_t *)com_malloc(sizeof(avs2_dec_ctrl_t));

    ctrl->total_memory = sizeof(avs2_dec_ctrl_t);
    ctrl->found_seqhdr = 0;
    ctrl->found_pichdr = 0;
    ctrl->outprint.buffer_num = 0;
    ctrl->curr_IDRtr = 0;
    ctrl->curr_IDRcoi = 0;
    ctrl->next_IDRtr = 0;
    ctrl->next_IDRcoi = 0;
    ctrl->i_inited_flag = 0;
    ctrl->last_output = -1;
    ctrl->min_coding_order = MAXINT64 - 256;

    ctrl->next_frm_num = 0;
    ctrl->idx_old_frm = 0;
    ctrl->active_threads = 0;
    ctrl->frame_threads = 1;

    ctrl->frame_threads = COM_CLIP3(1, MT_FRM_MAXTHREADS, frm_threads);
    ctrl->ref_buf_frames = REF_MAXBUFFER + ctrl->frame_threads;

    ctrl->p_dec = (avs2_dec_t *)com_malloc(sizeof(avs2_dec_t)* ctrl->frame_threads);
    ctrl->total_memory += sizeof(avs2_dec_t)* ctrl->frame_threads;

    rec_threads = COM_CLIP3(0, MT_REC_MATTHREADS, rec_threads);

    if (ctrl->frame_threads > 1) {
        avs2_threadpool_init(&ctrl->thread_pool, ctrl->frame_threads, NULL, NULL);
        for (i = 0; i < ctrl->frame_threads; i++) {
            ctrl->p_dec[i].seq = &ctrl->seq_info;
            ctrl->p_dec[i].i_thd_idx = i + 1;
            ctrl->p_dec[i].i_rec_threads = rec_threads;

            ctrl->p_dec[i].cur_wq_matrix[2] = ctrl->p_dec[i].cur_wq_matrix16;
            ctrl->p_dec[i].cur_wq_matrix[3] = ctrl->p_dec[i].cur_wq_matrix32;
        }
    } else {
        ctrl->p_dec[0].seq = &ctrl->seq_info;
        ctrl->p_dec[0].i_thd_idx = 0;
        ctrl->p_dec[0].i_rec_threads = rec_threads;

        ctrl->p_dec[0].cur_wq_matrix[2] = ctrl->p_dec[0].cur_wq_matrix16;
        ctrl->p_dec[0].cur_wq_matrix[3] = ctrl->p_dec[0].cur_wq_matrix32;
    }

    /* init functions handle */
    com_funs_init_intra_pred();
    com_funs_init_ip_filter();
    com_funs_init_pixel_opt();
    com_funs_init_deblock_filter();
    com_funs_init_dct();
    com_funs_init_alf_filter();

#if COMPILE_10BIT
    #if defined(__x86_64__)
    
    com_init_intrinsic_10bit();
    if (simd_avx_level(NULL) >= 2) {
        com_init_intrinsic_256_10bit();
    }
    
    #elif defined(__aarch64__)
    
    com_init_neon128();

    #else
    
    #endif
#endif


    return (void *)ctrl;
}

AVS2_API void __cdecl uavs2d_lib_decode(void *handle, avs2_frame_t *frm)
{
    int i;
    pic_hdr_t pic_hdr;
    avs2_dec_t *curr_dec;
    avs2_dec_ctrl_t *ctrl = (avs2_dec_ctrl_t*)handle;
    bs_stream_t tmp_bs;
    i64u_t pts = frm->pts;

    uchar_t *p_next = frm->bs_buf;
    uchar_t *p_all_end = frm->bs_buf + frm->bs_len;
    int RA_flag = 1;
    int start_slice_data = 0;

    while (!start_slice_data && bs_one_unit_init(&tmp_bs, p_next, p_all_end)) {
        p_next = tmp_bs.p_end;

        if (tmp_bs.bs_data[3] == SEQUENCE_HEADER_CODE) {
            ctrl->found_seqhdr = parse_seq_hdr(ctrl, &tmp_bs);
            ctrl->new_hdr = 1;

            if (ctrl->i_inited_flag == 0 && ctrl->found_seqhdr) {
                ctrl->i_inited_flag = 1;
                init_seq_buffers(ctrl);
                /* init every worker buffer */
                for (i = 0; i < ctrl->frame_threads; i++) {
                    ctrl->total_memory += dec_core_buf_create(&ctrl->p_dec[i], &ctrl->seq_info);
                }

                com_log(COM_LOG_INFO, (char_t *)"total memory: %.2f MB\n", ctrl->total_memory * 1.0 / 1024 / 1024);


            }

            if (ctrl->found_seqhdr) {
                cpy_seq_info(frm, &ctrl->seq_info);
                frm->dec_stats = AVS2_TYPE_SEQ;
            } else { // seq header parse error!
                frm->dec_stats = AVS2_TYPE_NEEDMORE;
            }
            return;
        }  
        else if (tmp_bs.bs_data[3] == I_PICTURE_START_CODE) {
            parse_i_hdr(&ctrl->seq_info, &pic_hdr, &tmp_bs);
            ctrl->found_pichdr = ctrl->found_seqhdr ? 1 : 0;
        } 
        else if (tmp_bs.bs_data[3] == PB_PICTURE_START_CODE) {
            RA_flag = parse_pb_hdr(&ctrl->seq_info, &pic_hdr, &tmp_bs);
            ctrl->found_pichdr = ctrl->found_seqhdr ? 1 : 0;
        } 
        else if (tmp_bs.bs_data[3] >= SLICE_START_CODE_MIN && tmp_bs.bs_data[3] <= SLICE_START_CODE_MAX) {
            start_slice_data = ctrl->found_pichdr ? 1 : 0;
        } 
        else if (tmp_bs.bs_data[3] == SEQUENCE_END_CODE) {
            printf("Sequence End!\n");
        } 
        else {
            /* EXTENSION_START_CODE 
               USER_DATA_START_CODE 
               VIDEO_EDIT_CODE */
        }
    }

    if (!start_slice_data) {
        frm->dec_stats = AVS2_TYPE_NEEDMORE;
        return;
    }
    if (!RA_flag && ctrl->ip_cnt == 1) {
        frm->dec_stats = AVS2_TYPE_DROP;
        return;
    }

    ctrl->found_pichdr = 0;

    if (ctrl->active_threads == ctrl->frame_threads) {
        curr_dec = &ctrl->p_dec[ctrl->idx_old_frm];

        if (ctrl->frame_threads > 1) {
            avs2_threadpool_wait(ctrl->thread_pool, curr_dec);
        }

        finish_frame_decoder(ctrl, curr_dec, frm);
        ctrl->idx_old_frm = (ctrl->idx_old_frm + 1) % ctrl->frame_threads;
    } else {
        int idx = (ctrl->idx_old_frm + ctrl->active_threads) % ctrl->frame_threads;
        curr_dec = &ctrl->p_dec[idx];
        ctrl->active_threads++;
    }

    init_frame_decoder(ctrl, curr_dec, &pic_hdr, tmp_bs.bs_data, p_all_end, pts);

    if (ctrl->frame_threads > 1) {
        avs2_threadpool_run(ctrl->thread_pool, dec_one_frame, curr_dec, 1);
    } else {
        dec_one_frame(curr_dec);
    }

}

AVS2_API void __cdecl  uavs2d_lib_flush(void *handle, avs2_frame_t *frm)
{
    avs2_dec_ctrl_t *ctrl = (avs2_dec_ctrl_t *)handle;
    outdata *out = &ctrl->outprint;

    frm->dec_stats = AVS2_TYPE_NEEDMORE;

    /* 1. flush active threads */

    while (frm->dec_stats != AVS2_TYPE_DECODED && ctrl->active_threads) {
        avs2_dec_t *curr_dec = &ctrl->p_dec[ctrl->idx_old_frm];

        if (ctrl->frame_threads > 1) {
            avs2_threadpool_wait(ctrl->thread_pool, curr_dec);
        }
        finish_frame_decoder(ctrl, curr_dec, frm);
        ctrl->active_threads--;
        ctrl->idx_old_frm = (ctrl->idx_old_frm + 1) % ctrl->frame_threads;
    }
    

    /* 2. flush frames in buffer */
    if (frm->dec_stats != AVS2_TYPE_DECODED) {
        int i, pos = -1;
        i64s_t tmp_min = MAXINT64;

        for (i = 0; i < out->buffer_num; i++) {
            if (out->stdoutdata[i].tr < tmp_min && out->stdoutdata[i].tr > ctrl->last_output) {
                pos = i;
                tmp_min = out->stdoutdata[i].tr;
            }
        }
        if (pos != -1) {
            ctrl->last_output = out->stdoutdata[pos].tr;
            report_frame(&ctrl->p_dec[0], frm, out, pos);
            write_frame(ctrl, frm, out->stdoutdata[pos].tr);
            frm->dec_stats = AVS2_TYPE_DECODED;
        } else {
            frm->dec_stats = AVS2_TYPE_NEEDMORE;
        }
    }
}

AVS2_API void __cdecl uavs2d_lib_destroy(void *handle)
{
    int i;
    avs2_dec_ctrl_t *ctrl = (avs2_dec_ctrl_t *)handle;

    if (ctrl->i_inited_flag) {
        for (i = 0; i < ctrl->frame_threads; i++) {
            dec_core_buf_free(&ctrl->p_dec[i]);
        }
        free_seq_buffers(ctrl);
    }

    com_free(ctrl->p_dec);
    
    if (ctrl->frame_threads > 1) {
        avs2_threadpool_delete(ctrl->thread_pool);
    }
    com_free(ctrl);

    return;
}