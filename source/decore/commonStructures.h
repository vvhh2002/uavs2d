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

#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdio.h>                              //!< for FILE
#include "defines.h"
#include "common.h"
#include "biaridecod.h"
#include "bitstream.h"
#include "threadpool.h"

#define ANG_X_OFFSET   3
#define ANG_XY_OFFSET 13
#define ANG_Y_OFFSET  25

#define LCU_SIZE 64

enum {
    INTRA_PRED_VER = 0,
    INTRA_PRED_HOR,
    INTRA_PRED_DC_DIAG
};

#define MAX_AMVP_SET_ARRAY 10

typedef struct {
    i16s_t pmv_cand[MAX_AMVP_SET_ARRAY][2];
    i16s_t pmv_num;
    i16s_t pmv_index;
} AMVP_SET;

enum SAOComponentIdx {
    SAO_Y = 0,
    SAO_Cb,
    SAO_Cr
};

enum SAOMode { //mode
    SAO_MODE_OFF = 0,
    SAO_MODE_MERGE,
    SAO_MODE_NEW,
    NUM_SAO_MODES
};

enum SAOModeMergeTypes {
    SAO_MERGE_LEFT = 0,
    SAO_MERGE_ABOVE,
    NUM_SAO_MERGE_TYPES
};


enum SAOModeNewTypes { //NEW: types
    SAO_TYPE_EO_0,
    SAO_TYPE_EO_90,
    SAO_TYPE_EO_135,
    SAO_TYPE_EO_45,
    SAO_TYPE_BO ,
    NUM_SAO_NEW_TYPES
};

enum SAOEOClasses { // EO Groups, the assignments depended on how you implement the edgeType calculation
    SAO_CLASS_EO_FULL_VALLEY = 0,
    SAO_CLASS_EO_HALF_VALLEY = 1,
    SAO_CLASS_EO_PLAIN       = 2,
    SAO_CLASS_EO_HALF_PEAK   = 3,
    SAO_CLASS_EO_FULL_PEAK   = 4,
    SAO_CLASS_BO             = 5,
    NUM_SAO_EO_CLASSES = SAO_CLASS_BO,
    NUM_SAO_OFFSET
};

/* global info for cu, stored by 8x8, shared by neighbors */
typedef struct com_cu_t {
    uchar_t ui_MbBitSize;
    char_t  c_ipred_mode;
    char_t  qp;
    char_t  cuType;
    char_t  b_decoded;
    i16s_t  slice_nr;
    i16s_t  slice_set_index;
    int     cbp;
} com_cu_t;

/* local info for LCU, didn't shared by neighbors */
typedef struct com_cu_local_t {
    char_t trans_size;
    char_t intra_pred_modes[4];
    char_t weighted_skipmode;
    char_t md_directskip_mode;
    char_t dmh_mode;
    char_t b8pdir[4];
} com_cu_local_t;

typedef struct com_lcu_t {
    com_cu_local_t cus[64];
    ALIGNED_16(coef_t coef_luma[64 * 64]); //!< coef & resi
    ALIGNED_16(coef_t coef_chroma[2][32 * 32]); //!< coef & resi
} com_lcu_t;

typedef struct {
    int modeIdc; //NEW, MERGE, OFF
    int typeIdc; //NEW: EO_0, EO_90, EO_135, EO_45, BO. MERGE: left, above
    int startBand; //BO: starting band index
    int startBand2;
    int deltaband;
    int offset[MAX_NUM_SAO_CLASSES];
} sap_param_t;


typedef struct {
    int     m_filterCoeffSym[16][9];
    int     m_varIndTab[16];
    char_t *m_varImg;
    char_t (*m_AlfLCUEnabled)[3];
} alf_param_t;


typedef struct {
    int type;
    int typeb;
    i64s_t   framenum;
    i64s_t   tr;
    int   qp;
    int   curr_frame_bits;
    int   emulate_bits;
    i64u_t pts;
} STDOUT_DATA;

typedef struct {
    STDOUT_DATA stdoutdata[8];
    int buffer_num;
} outdata;

typedef struct {
    i64s_t ref_imgtr;
    i64s_t ref_reftrs[4];
    int dist[2]; // 0: fwd 1 : bwd
    com_frm_t *frm;
} com_ref_t;

typedef struct {
    /* copy from h_dec when start a frame, read only, inited once*/
    const seq_info_t *seq;
    const pic_hdr_t* pichdr;
    const slice_hdr_t *slihdr;
    char_t *ipredmode;
    char_t *snd_ref;
    char_t *bw_ref;
    i16s_t(*bw_mv)[2];
    com_ref_t *ref_list;
    com_cu_t  *cu_array;
    com_lcu_t *lcu_array;
    com_pic_t *g_pic_flt_tmp;
    uchar_t *deblock_edge_flag[2];
    uchar_t *deblock_skip_flag[2];
    pel_t* filter_bak_rec[3];
    sap_param_t(*sao_blk_params_rec)[3];
    alf_param_t *alf_params;
    int check_ref;
    int* i_aec_finished_idx;
    int* i_rec_finished_idx;
    avs2_pthread_mutex_t* pmutex;
    avs2_pthread_cond_t*  pcond;

    /* copy from h_dec when start a frame, read only*/
    i64u_t dec_order_idx;
    com_frm_t *frm_cur;
    com_ref_t *ref_fwd;
    com_ref_t *ref_bwd;
    int type;
    int typeb;
    int i_refs;
    i64s_t img_tr_ext;

    /* lcu level info, updated when start every LCU */
    int lcu_pix_x;
    int lcu_pix_y;
    int lcu_width;
    int lcu_height;
    int lcu_b8_x;
    int lcu_b8_y;
    com_lcu_t *lcu;

    /* cu level info, updated when start every CU */
    int cu_zig_offset;
    int block_available_up;
    int block_available_left;
    com_cu_t *cu;
    com_cu_local_t *cu_loc_dat;

    int cu_idx;
    int cu_pix_x;
    int cu_pix_y;
    int cu_size;
    int cu_bitsize;
    int cu_b8size;
    int cu_b8num;
    int cu_b8_x;
    int cu_b8_y;
    int cu_b4_x;
    int cu_b4_y;
    int cu_pix_c_y;
    int cu_pix_c_x;
} com_rec_t;

typedef struct {
    /* seq-level shared info */
    const seq_info_t *seq;

    /* thread param */
    int i_thd_idx; // idx of this frame thread
    int i_rec_threads;
    int i_rec_finished_idx;
    int i_aec_finished_idx;
    avs2_threadpool_t *rec_thread_pool;
    avs2_pthread_mutex_t mutex;
    avs2_pthread_cond_t  cond;

    /* stream info */
    uchar_t *bs_start;
    uchar_t *bs_end;
    i64u_t pts;

    /* frame level info & buf */
    pic_hdr_t pic_hdr;
    slice_hdr_t slice_hdr;
    int type;
    int typeb;
    i64s_t tr;
    int i_refs;
    int i_frame_num;
    i64s_t imgtr_next_P;
    i64s_t img_tr_ext;

    int PrevPicDistanceLsb;
    i64s_t CurrPicDistanceMsb;

    bs_stream_t bs;

    com_lcu_t *lcu_array;
    com_cu_t  *cu_array;
    com_pic_t *g_pic_flt_tmp;

    com_frm_t *frm_cur;
    com_ref_t  ref_list[REF_MAXBUFFER];
    com_ref_t *ref_fwd;
    com_ref_t *ref_bwd;
    aec_core_t aec_core;    
    alf_param_t alf_params;

    i16s_t cur_wq_matrix16[256];
    i16s_t cur_wq_matrix32[1024];
    i16s_t *cur_wq_matrix[4];

    pel_t* filter_bak_rec[3];

    char_t *dec_info_buffer;
    char_t *ipredmode;
    char_t *nz;
    char_t *snd_ref;
    char_t *bw_ref;
    char_t *pdir;
    i16s_t(*bw_mv)[2];

    uchar_t *deblock_skip_flag[2];
    uchar_t *deblock_edge_flag[2];
    sap_param_t(*sao_blk_params)[3];
    sap_param_t(*sao_blk_params_rec)[3];


    /* rec core */
    com_rec_t *rec;

    /* lcu level info, only for aec */
    int lcu_cur_num;
    int lcu_pix_x;
    int lcu_pix_y;
    int lcu_width;
    int lcu_height;
    int lcu_b8_x;
    int lcu_b8_y;
    
    com_lcu_t *lcu;

    /* cu level info */
    int cu_zig_offset;
    com_cu_t *cu;
    com_cu_local_t *cu_loc_dat;

    int nsqt_shape;
    int cu_idx;
    int cu_pix_x;
    int cu_pix_y;
    int cu_size;
    int cu_bitsize;
    int cu_b8size;
    int cu_b8num;
    int cu_b8_x;
    int cu_b8_y;
    int cu_b4_x;
    int cu_b4_y;
    int cu_pix_c_y;
    int cu_pix_c_x;
    int current_slice_nr;
    int last_dquant;
} avs2_dec_t;

typedef struct avs2_dec_ctrl_t{
    /* work decoders */
    avs2_dec_t *p_dec;

    /* seq level info, shared by every threads */
    seq_info_t seq_info;

    /* below: seq level data,  mustn't be accessed by work threads */
    int i_inited_flag;
    int found_seqhdr;
    int found_pichdr;
    int new_hdr;
    i64s_t ip_cnt;

    com_frm_t  frm_bg;

    com_frm_t  frm_data_buf[REF_MAXBUFFER + MT_FRM_MAXTHREADS];
    com_frm_t *frm_buf[REF_MAXBUFFER + MT_FRM_MAXTHREADS];

    outdata outprint;

    i64s_t curr_IDRcoi;
    i64s_t curr_IDRtr;
    i64s_t next_IDRtr;
    i64s_t next_IDRcoi;
    i64s_t min_coding_order;
    i64s_t last_output;

    avs2_threadpool_t *thread_pool;
    int frame_threads;
    int active_threads;
    int idx_old_frm;
    int ref_buf_frames;
    int next_frm_num;

    i64s_t total_memory;

} avs2_dec_ctrl_t;


/***********************************************************************
* D a t a    t y p e s   f o r  A E C
************************************************************************/

//! struct for context management


typedef struct pix_pos {
    int available;
    int mb_addr;
    int blk_x_in_cu;
    int blk_y_in_cu;
    int b4x_in_pic; 
    int b4y_in_pic;
} PixelPos;

extern tab_char_t tab_log2[65];

#endif // #ifndef _TYPES_H_

