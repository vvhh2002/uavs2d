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

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <string.h>
#include <assert.h>

#include "global.h"
#include "commonVariables.h"
#include "header.h"

#include "AEC.h"
#include "biaridecod.h"
#include "loop-filter.h"

#include "ComAdaptiveLoopFilter.h"
#include "DecAdaptiveLoopFilter.h"


// Adaptive frequency weighting quantization
#include "wquant.h"


/* 08.16.2007--for user data after pic header */
extern void readParaSAO_one_SMB(avs2_dec_t *h_dec, int smb_index, int mb_y, int mb_x, int *slice_sao_on, sap_param_t *saoBlkParam, sap_param_t *rec_saoBlkParam);


static void init_rec_core_frame(avs2_dec_t *h_dec, com_rec_t *rec)
{
    rec->frm_cur       = h_dec->frm_cur;
    rec->ref_bwd       = h_dec->ref_bwd;
    rec->ref_fwd       = h_dec->ref_fwd;
    rec->type          = h_dec->type;
    rec->typeb         = h_dec->typeb;
    rec->i_refs        = h_dec->i_refs;
    rec->img_tr_ext    = h_dec->img_tr_ext;
    rec->dec_order_idx = h_dec->pts;
}


static void dec_all_loopfilter(com_rec_t *rec)
{
    const pic_hdr_t *pichdr = rec->pichdr;
    const seq_info_t *seq = rec->seq;
    int bfinished = rec->lcu_pix_y + rec->lcu_height == seq->img_height;
    int lcu_size = 1 << seq->g_uiMaxSizeInBit;
    int lcu_pix_y = rec->lcu_pix_y;
    int finished_line_num;
    
    com_pic_t *pic = rec->frm_cur->yuv_data;

    /* back up rec last row */
    if (!bfinished) {
        memcpy(rec->filter_bak_rec[0], pic->p_y + (lcu_pix_y + lcu_size - 1) * pic->i_stride, seq->img_width * sizeof(pel_t));
        memcpy(rec->filter_bak_rec[1], pic->p_u + ((lcu_pix_y + lcu_size) / 2 - 1) * pic->i_stridec, seq->img_widthc * sizeof(pel_t));
        memcpy(rec->filter_bak_rec[2], pic->p_v + ((lcu_pix_y + lcu_size) / 2 - 1) * pic->i_stridec, seq->img_widthc * sizeof(pel_t));
    }

    if (!pichdr->loop_filter_disable) {
        deblock_lcu_row(rec, rec->frm_cur->yuv_data);
    }

    if (seq->sao_enable && (lcu_pix_y || bfinished)) {
        int sao_cpy_rows = bfinished ? lcu_size * 2 : lcu_size;
        com_pic_yuv_copy_rows_ext(rec->g_pic_flt_tmp, rec->frm_cur->yuv_data, lcu_pix_y - lcu_size, lcu_pix_y - lcu_size + sao_cpy_rows, (int*)rec->slihdr->slice_sao_on);
        sao_lcu_row(rec, seq->g_uiMaxSizeInBit, seq->slice_set_enable, rec->sao_blk_params_rec, seq->sample_bit_depth, lcu_pix_y - lcu_size);
        if (bfinished) {
            sao_lcu_row(rec, seq->g_uiMaxSizeInBit, seq->slice_set_enable, rec->sao_blk_params_rec, seq->sample_bit_depth, lcu_pix_y);
        }
    }
    if (seq->alf_enable && (lcu_pix_y - lcu_size > 0 || bfinished)) {
        if (pichdr->m_alfPictureParam[ALF_Y].alf_flag != 0 || pichdr->m_alfPictureParam[ALF_Cb].alf_flag != 0 || pichdr->m_alfPictureParam[ALF_Cr].alf_flag != 0) {
            int cpy_flag[3];
            int alf_cpy_rows = bfinished ? lcu_size * 3 : lcu_size;
            cpy_flag[0] = pichdr->m_alfPictureParam[ALF_Y].alf_flag;
            cpy_flag[1] = pichdr->m_alfPictureParam[ALF_Cb].alf_flag;
            cpy_flag[2] = pichdr->m_alfPictureParam[ALF_Cr].alf_flag;
            com_pic_yuv_copy_rows_ext(rec->g_pic_flt_tmp, rec->frm_cur->yuv_data, lcu_pix_y - 2 * lcu_size, lcu_pix_y - 2 * lcu_size + alf_cpy_rows, cpy_flag);
            com_pic_yuv_padding_rows_lr(rec->g_pic_flt_tmp, lcu_pix_y - 2 * lcu_size, alf_cpy_rows, cpy_flag);
            if (lcu_pix_y - 2 * lcu_size >= 0) {
                alf_lcu_row(rec, pichdr->m_alfPictureParam, seq->sample_bit_depth, lcu_pix_y - 2 * lcu_size);
            }

            if (bfinished) {
                alf_lcu_row(rec, pichdr->m_alfPictureParam, seq->sample_bit_depth, lcu_pix_y - lcu_size);
                alf_lcu_row(rec, pichdr->m_alfPictureParam, seq->sample_bit_depth, lcu_pix_y);
            }
        }
    }

    if (bfinished) {
        finished_line_num = seq->img_height + seq->pad_size + 64;
    } else {
        finished_line_num = lcu_pix_y - lcu_size - 8;
    }

    if (lcu_pix_y - lcu_size > 0 || bfinished) {
        int pad_start = lcu_pix_y - 2 * lcu_size - 8;
        int pad_rows = finished_line_num - pad_start;
        com_pic_yuv_padding_rows(rec->frm_cur->yuv_data, pad_start, pad_rows);

        if (rec->pichdr->curr_RPS.referd_by_others) {
            avs2_pthread_mutex_lock(&rec->frm_cur->mutex);
            rec->frm_cur->linend = finished_line_num;
            avs2_pthread_cond_broadcast(&rec->frm_cur->cond);
            avs2_pthread_mutex_unlock(&rec->frm_cur->mutex);
        } else {
            rec->frm_cur->linend = finished_line_num;
        }
    }
}

static void dec_all_lcus(avs2_dec_t *h_dec)
{
    aec_core_t *aec = &h_dec->aec_core;
    bs_stream_t *bs = &h_dec->bs;
    uchar_t *p_next = h_dec->bs_start;
    uchar_t *end = h_dec->bs_end;
    int lcu_x, lcu_y;
    int lcu_idx;
    const seq_info_t *seq = h_dec->seq;

    int first_slice = 1;
    int N8_SizeScale = 1 << (seq->g_uiMaxSizeInBit - MIN_CU_SIZE_IN_BIT);
    int new_slice = 0;
    int lcu_cur_num;

    for (lcu_y = 0; lcu_y < seq->img_height_in_lcu; lcu_y++) {
        for (lcu_x = 0; lcu_x < seq->img_width_in_lcu; lcu_x++) {
            i32u_t prev_bytes;
            lcu_cur_num = lcu_y * seq->img_width_in_lcu + lcu_x;
            lcu_idx = (lcu_y * seq->img_width_in_mcu + lcu_x) * N8_SizeScale;
            if (first_slice) {
                bs_one_unit_init(bs, p_next, end);
                p_next = bs->p_end;
                prev_bytes = parse_slice_hdr(h_dec, bs);
                h_dec->current_slice_nr++;
                first_slice = 0;
                new_slice = 1;
            } else {
                if (bs->p_start_test == bs->p_end) {
                    bs_one_unit_init(bs, p_next, end);
                    p_next = bs->p_end;
                    prev_bytes = parse_slice_hdr(h_dec, bs);
                    h_dec->current_slice_nr++;
                    new_slice = 1;
                } else {
                    new_slice = 0;
                }
            }

            if (new_slice) {
                h_dec->last_dquant = 0;
                biari_start_dec(&h_dec->aec_core, bs->p_start_test, bs->p_end, prev_bytes);
            }

            init_lcu(h_dec, lcu_x, lcu_y);

            /* check co-local mv and ref informations are OK */
            if (h_dec->ref_list[0].frm && h_dec->i_thd_idx) {
                check_ref_avaliable(h_dec->ref_list[0].frm, h_dec->lcu_pix_y + h_dec->lcu_height - 1);
            }

            if (seq->sao_enable) {
                readParaSAO_one_SMB(h_dec, lcu_cur_num, h_dec->lcu_b8_y, h_dec->lcu_b8_x, h_dec->slice_hdr.slice_sao_on, h_dec->sao_blk_params[lcu_cur_num], h_dec->sao_blk_params_rec[lcu_cur_num]);
            }

            if (seq->alf_enable) {
                int compIdx;
                for (compIdx = 0; compIdx < NUM_ALF_COMPONENT; compIdx++) {
                    if (h_dec->pic_hdr.m_alfPictureParam[compIdx].alf_flag) {
                        h_dec->alf_params.m_AlfLCUEnabled[lcu_cur_num][compIdx] = (char_t) aec_alf_lcu_ctrl(h_dec, aec);
                    } else {
                        h_dec->alf_params.m_AlfLCUEnabled[lcu_cur_num][compIdx] = 0;
                    }
                }
            }

            if (!h_dec->i_rec_threads) {
                init_rec_core_lcu(h_dec->rec, &h_dec->lcu_array[0], lcu_x, lcu_y);
            }

            cu_decode(h_dec, (unsigned int) seq->g_uiMaxSizeInBit, (unsigned int) lcu_idx);

            AEC_startcode_follows(h_dec, 1);
        }

        if (!h_dec->i_rec_threads) {
            dec_all_loopfilter(h_dec->rec);
        }

        if (h_dec->i_rec_threads) {
            avs2_pthread_mutex_lock(&h_dec->mutex);
            h_dec->i_aec_finished_idx++;
            avs2_pthread_cond_broadcast(&h_dec->cond);
            avs2_pthread_mutex_unlock(&h_dec->mutex);
        }
    }
}

static void rec_one_lcu_row(com_rec_t *rec, int lcu_y, com_lcu_t *lcu_row)
{
    int lcu_idx;
    int lcu_x;
    const seq_info_t *seq = rec->seq;
    int N8_SizeScale = 1 << (seq->g_uiMaxSizeInBit - MIN_CU_SIZE_IN_BIT);

    for (lcu_x = 0; lcu_x < seq->img_width_in_lcu; lcu_x++) {
        lcu_idx = (lcu_y * seq->img_width_in_mcu + lcu_x) * N8_SizeScale;
        init_rec_core_lcu(rec, lcu_row + lcu_x, lcu_x, lcu_y);
        cu_decode_reconstruct(rec, (unsigned int) seq->g_uiMaxSizeInBit, lcu_idx);
    }
    dec_all_loopfilter(rec);
}

static void* rec_woker(void *r)
{
    com_rec_t *rec = (com_rec_t*)r;
    int lcu_y;
    const seq_info_t *seq = rec->seq;

    
    do  {
        com_lcu_t *lcu_row;

        avs2_pthread_mutex_lock(rec->pmutex);
        while (*rec->i_aec_finished_idx <= *rec->i_rec_finished_idx && *rec->i_rec_finished_idx < seq->img_height_in_lcu) {
            avs2_pthread_cond_wait(rec->pcond, rec->pmutex);
        }
        lcu_y = *rec->i_rec_finished_idx;
        (*rec->i_rec_finished_idx)++;
        avs2_pthread_mutex_unlock(rec->pmutex);

        if (lcu_y < seq->img_height_in_lcu) {
            lcu_row = rec->lcu_array + lcu_y * seq->img_width_in_lcu;
            rec_one_lcu_row(rec, lcu_y, lcu_row);
        }
        
    } while (lcu_y < seq->img_height_in_lcu);

    return NULL;
}

/*
*************************************************************************
* Function:decodes one picture
* Input:
* Output:
* Return:
* Attention:
*************************************************************************
*/

void* dec_one_frame(void *h)
{
    avs2_dec_t *h_dec = (avs2_dec_t*)h;
    const seq_info_t *seq = h_dec->seq;
    const int mb_nr = h_dec->cu_idx;
    int mb_width = h_dec->seq->img_width / 8;

    int i, j;
    com_rec_t *rec = h_dec->rec;

    int img_height, img_width, lcuHeight, lcuWidth, numLCUInPicWidth, numLCUInPicHeight, NumCUInFrame;
 
    lcuHeight         = 1 << seq->g_uiMaxSizeInBit;
    lcuWidth          = lcuHeight;
    img_height        = seq->img_height;
    img_width         = seq->img_width;
    numLCUInPicWidth  = img_width / lcuWidth ;
    numLCUInPicHeight = img_height / lcuHeight ;
    numLCUInPicWidth  += (img_width % lcuWidth) ? 1 : 0;
    numLCUInPicHeight += (img_height % lcuHeight) ? 1 : 0;
    NumCUInFrame = numLCUInPicHeight * numLCUInPicWidth;

    h_dec->i_aec_finished_idx = 0;
    h_dec->i_rec_finished_idx = 0;

    init_rec_core_frame(h_dec, rec);


    if (h_dec->i_rec_threads) {
        for (i = 0; i < h_dec->i_rec_threads; i++) {
            avs2_threadpool_run(h_dec->rec_thread_pool, rec_woker, rec, 1);
        }
    }

    dec_all_lcus(h_dec);

    if (h_dec->i_rec_threads) {
        for (i = 0; i < h_dec->i_rec_threads; i++) {
            avs2_threadpool_wait(h_dec->rec_thread_pool, rec);
        }
    }

    if ((h_dec->type == P_IMG || h_dec->type == F_IMG) && seq->background_picture_enable && h_dec->pic_hdr.background_reference_enable) {
        int i_b4 = seq->b4_info_stride;
        char_t *fwd_ref = h_dec->frm_cur->refbuf;

        for (j = 0; j < seq->img_height / MIN_BLOCK_SIZE; j++) {
            for (i = 0; i < seq->img_width / MIN_BLOCK_SIZE; i++) {
                if (fwd_ref[i] == h_dec->i_refs - 1) {
                    fwd_ref[i] = -1;
                }
            }
            fwd_ref += i_b4;
        }
    }
    if(h_dec->type == P_IMG && h_dec->typeb == BP_IMG && seq->background_picture_enable) {
        int i_b4 = seq->b4_info_stride;
        char_t *fwd_ref = h_dec->frm_cur->refbuf;

        for (j = 0; j < seq->img_height / MIN_BLOCK_SIZE; j++) {
            for (i = 0; i < seq->img_width / MIN_BLOCK_SIZE; i++) {
                fwd_ref[i] = -1;
            }
            fwd_ref += i_b4;
        }
    }
    return NULL;
}

void com_frm_create(avs2_dec_ctrl_t *ctrl, com_frm_t *frm, int pad)
{
    seq_info_t * seq = &ctrl->seq_info;
    char_t *pinfo;
    int info_size = seq->b4_info_size * sizeof(char_t)+ // refbuf
                    seq->b4_info_size * sizeof(i16s_t)* 2; // mvbuf

    frm->yuv_data = com_pic_yuv_create(seq->img_width, seq->img_height, pad);

    frm->coiref = -1;
    frm->imgtr  = -1;
    frm->bak_imgtr = -1;

    frm->b4_info_buf = (char_t*)com_malloc(info_size);

    ctrl->total_memory += frm->yuv_data->pic_memory + info_size;

    pinfo = frm->b4_info_buf;

    frm->refbuf = (char_t*)pinfo + seq->b4_info_stride + 1;
    pinfo += seq->b4_info_size * sizeof(char_t);

    frm->mvbuf = (i16s_t(*)[2])pinfo + seq->b4_info_stride + 1;
    pinfo += seq->b4_info_size * sizeof(i16s_t)* 2;

    avs2_pthread_mutex_init(&frm->mutex, NULL);
    avs2_pthread_cond_init(&frm->cond, NULL);
}

void com_frm_destroy(avs2_dec_ctrl_t *ctrl, com_frm_t *frm)
{
    avs2_pthread_mutex_destroy(&frm->mutex);
    avs2_pthread_cond_destroy(&frm->cond);
    com_pic_yuv_destroy(frm->yuv_data);
    com_free(frm->b4_info_buf);
}