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

#ifndef _BIARIDECOD_H_
#define _BIARIDECOD_H_

#include "common.h"

typedef struct {
    uchar_t MPS;     // 1 bit
    uchar_t cycno;   // 2 bits
    i16u_t  LG_PMPS; //10 bits
} aec_ctx_t;

#define B_BITS  10
#define LG_PMPS_SHIFTNO 2
#define HALF      (1 << (B_BITS-1))
#define QUARTER   (1 << (B_BITS-2))

#define NUM_CuType_CTX              11+10
#define NUM_B8_TYPE_CTX              9
#define NUM_MVD_CTX                 15
#define NUM_PMV_IDX_CTX             10
#define NUM_REF_NO_CTX               6
#define NUM_DELTA_QP_CTX             4
#define NUM_INTER_DIR_CTX           18
#define NUM_INTER_DIR_DHP_CTX        3
#define NUM_B8_TYPE_DHP_CTX          1
#define NUM_AMP_CTX                  2
#define NUM_C_INTRA_MODE_CTX         4
#define NUM_CBP_CTX                  4
#define NUM_BCBP_CTX                 4
#define NUM_MAP_CTX                 17
#define NUM_LAST_CTX                17
#define NUM_INTRA_MODE_CTX           7
#define NUM_ABS_CTX                  5
#define NUM_TU_CTX                   3
#define NUM_SPLIT_CTX                8  
#define NUM_BRP_CTX                  8
#define NUM_LAST_CG_CTX_LUMA        12
#define NUM_LAST_CG_CTX_CHROMA       6
#define NUM_SIGCG_CTX_LUMA           2
#define NUM_SIGCG_CTX_CHROMA         1
#define NUM_LAST_POS_CTX_LUMA       56
#define NUM_LAST_POS_CTX_CHROMA     16
#define NUM_LAST_CG_CTX             (NUM_LAST_CG_CTX_LUMA + NUM_LAST_CG_CTX_CHROMA)
#define NUM_SIGCG_CTX               (NUM_SIGCG_CTX_LUMA + NUM_SIGCG_CTX_CHROMA)
#define NUM_LAST_POS_CTX            (NUM_LAST_POS_CTX_LUMA + NUM_LAST_POS_CTX_CHROMA)
#define NUM_SAO_MERGE_FLAG_CTX       3
#define NUM_SAO_MODE_CTX             1
#define NUM_SAO_OFFSET_CTX           2
#define NUM_INTER_DIR_MIN_CTX        2

typedef struct {
    aec_ctx_t cuType_contexts          [NUM_CuType_CTX];
    aec_ctx_t pdir_contexts            [NUM_INTER_DIR_CTX];
    aec_ctx_t amp_contexts             [NUM_AMP_CTX];
    aec_ctx_t b8_type_contexts         [NUM_B8_TYPE_CTX];
    aec_ctx_t pdir_dhp_contexts        [NUM_INTER_DIR_DHP_CTX];
    aec_ctx_t b8_type_dhp_contexts     [NUM_B8_TYPE_DHP_CTX];
    aec_ctx_t b_dir_skip_contexts      [DIRECTION];
    aec_ctx_t p_skip_mode_contexts     [MH_PSKIP_NUM];
    aec_ctx_t wpm_contexts             [WPM_NUM];
    aec_ctx_t mvd_contexts             [3][NUM_MVD_CTX];
    aec_ctx_t pmv_idx_contexts         [2][NUM_PMV_IDX_CTX];
    aec_ctx_t ref_no_contexts          [NUM_REF_NO_CTX];
    aec_ctx_t delta_qp_contexts        [NUM_DELTA_QP_CTX];
    aec_ctx_t l_intra_mode_contexts    [NUM_INTRA_MODE_CTX];
    aec_ctx_t c_intra_mode_contexts    [NUM_C_INTRA_MODE_CTX];
    aec_ctx_t cbp_contexts             [3][NUM_CBP_CTX];
    aec_ctx_t map_contexts             [NUM_BLOCK_TYPES][NUM_MAP_CTX];
    aec_ctx_t last_contexts            [NUM_BLOCK_TYPES][NUM_LAST_CTX];
    aec_ctx_t split_contexts           [NUM_SPLIT_CTX];
    aec_ctx_t tu_contexts              [NUM_TU_CTX];
    aec_ctx_t lastCG_contexts          [NUM_LAST_CG_CTX];
    aec_ctx_t sigCG_contexts           [NUM_SIGCG_CTX];
    aec_ctx_t lastPos_contexts         [NUM_LAST_POS_CTX];
    aec_ctx_t saomergeflag_context     [NUM_SAO_MERGE_FLAG_CTX];
    aec_ctx_t saomode_context          [NUM_SAO_MODE_CTX];
    aec_ctx_t saooffset_context        [NUM_SAO_OFFSET_CTX];
    aec_ctx_t m_cALFLCU_Enable_SCModel [1];
    aec_ctx_t brp_contexts             [NUM_BRP_CTX];
    aec_ctx_t pdirMin_contexts         [NUM_INTER_DIR_MIN_CTX];
} aec_contexts_t;

typedef struct {
    i32u_t t1, value_t;

    uchar_t s1, value_s;
    uchar_t is_value_bound;
    uchar_t is_value_domain;

    // buffer 
    i32u_t  buffer;
    i32u_t  prev_bytes;
    int     bits_to_go;
    uchar_t *p_buf;
    uchar_t *p_start;
    uchar_t *p_end;
    int     index;

    // contexts
    aec_contexts_t   syn_ctx;

} aec_core_t;


void biari_start_dec(aec_core_t * aec, uchar_t *start, uchar_t *end, i32u_t prev_bytes);
i32u_t biari_decode_symbol (aec_core_t * aec, aec_ctx_t * bi_ct);
i32u_t biari_decode_symbol_eq_prob(aec_core_t * aec);
i32u_t biari_decode_final(aec_core_t * aec);
i32u_t biari_decode_symbol_continu0(aec_core_t * aec, aec_ctx_t * bi_ct, int max_num);
i32u_t biari_decode_symbol_continu0_ext(aec_core_t * aec, aec_ctx_t * bi_ct, int max_ctx_inc, int max_num);

#define get_byte(aec){                                                            \
    (aec)->buffer = *aec->p_start++;                                              \
    (aec)->bits_to_go = 7;                                                        \
    (aec)->prev_bytes = (((aec)->prev_bytes << 8) | (aec)->buffer) & 0x00FFFFFF;  \
    if ((aec)->prev_bytes == 0x02) {                                              \
        (aec)->buffer >>= 2;                                                      \
        (aec)->bits_to_go = 5;                                                    \
    }                                                                             \
}

#define biari_decode_read_inline(aec,s1,value_s,value_t,domain,bound) {           \
    if (domain || (s1 == 254 && bound == 1)) {                                    \
        s1 = 0;                                                                   \
        value_s = 0;                                                              \
        domain = 0;                                                               \
        while (value_t < QUARTER && value_s < 254) {                              \
            if (--aec->bits_to_go < 0) {                                          \
                get_byte(aec);                                                    \
            }                                                                     \
            value_t = (value_t << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01); \
            value_s++;                                                            \
        }                                                                         \
        bound = (value_t < QUARTER) ? 1 : 0;                                      \
        value_t = value_t & 0xff;                                                 \
    }                                                                             \
} 

#define biari_decode_read(aec) biari_decode_read_inline(aec, aec->s1, aec->value_s, aec->value_t, aec->is_value_domain, aec->is_value_bound);

#endif  // BIARIDECOD_H_

