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
#include <math.h>
#include <assert.h>

#include "biaridecod.h"

static tab_i32s_t lg_pmps_inc[] = { 46, 46, 46, 197, 95, 46 };

static void init_contexts(aec_contexts_t* contexts)
{
    int i;
    int numbers = sizeof(aec_contexts_t) / sizeof(aec_ctx_t);
    i32u_t *p;
    i32u_t tmp;
    aec_ctx_t *ctx = (aec_ctx_t*)&tmp;

    ctx->LG_PMPS = (QUARTER << LG_PMPS_SHIFTNO) - 1;   //10 bits precision
    ctx->MPS = 0;
    ctx->cycno = 0;

    assert(sizeof(aec_ctx_t) == 4);

    p = (i32u_t*)contexts;

    for (i = 0; i < numbers; i++) {
        p[i] = tmp;
    }
}

void biari_start_dec(aec_core_t * aec, uchar_t *start, uchar_t *end, i32u_t prev_bytes)
{
    int i;
    
    init_contexts(&aec->syn_ctx);

    aec->p_buf = start;
    aec->p_start = start;
    aec->p_end = end;
    aec->prev_bytes = prev_bytes;

    aec->s1 = 0;
    aec->t1 = QUARTER - 1; 
    aec->value_s = 0;
    aec->value_t = 0;

    aec->is_value_bound = 0;
    aec->is_value_domain = 1;

    aec->bits_to_go = 0;

    for (i = 0; i < B_BITS - 1; i++) {
        if (--aec->bits_to_go < 0) {
            get_byte(aec);
        }
        aec->value_t = (aec->value_t << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);
    }
}

i32u_t biari_decode_symbol(aec_core_t * aec, aec_ctx_t * bi_ct)
{
    register uchar_t bit;
    register uchar_t s_flag;
    register uchar_t cwr, cycno;
    register i32u_t lg_pmps;
    register i32u_t t_rlps;
    register i32u_t t2;
    register uchar_t s2;
    register i32u_t lg_pmps_shift;

    cycno = bi_ct->cycno;
    lg_pmps = bi_ct->LG_PMPS;
    lg_pmps_shift = (lg_pmps >> LG_PMPS_SHIFTNO);

    bit = bi_ct->MPS;

    cwr = (uchar_t) max(3, cycno + 2);
    biari_decode_read(aec);

    s_flag = (uchar_t) (aec->t1 < lg_pmps_shift);
    s2 = aec->s1 + s_flag;
    t2 = aec->t1 - lg_pmps_shift + (s_flag << 8);

    if (s2 < aec->value_s || (s2 == aec->value_s && aec->value_t < t2) || aec->is_value_bound) { //MPS
        i32u_t tmp_pmps = lg_pmps >> cwr;
        aec->s1 = s2;
        aec->t1 = t2;
        aec->is_value_domain = 0;
        if (cycno == 0) {
            bi_ct->cycno = 1;
        }
        bi_ct->LG_PMPS = (i16u_t) (lg_pmps - tmp_pmps - (tmp_pmps >> 2));
    } else {   //LPS
        int vt = aec->value_t;
        bit = (uchar_t) !bit; //LPS
        aec->is_value_domain = 1;

        t_rlps = (s_flag == 0) ? lg_pmps_shift : (aec->t1 + lg_pmps_shift);

        if (s2 == aec->value_s) {
            vt = (vt - t2);
        } else {
            if (--aec->bits_to_go < 0) {
                get_byte(aec);
            }

            vt = (vt << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);
            vt = 256 + vt - t2;
        }

        while (t_rlps < QUARTER) {
            t_rlps = t_rlps << 1;

            if (--aec->bits_to_go < 0) {
                get_byte(aec);
            }

            vt = (vt << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);
        }

        aec->value_t = (i32u_t) vt;
        aec->s1 = 0;
        aec->t1 = t_rlps & 0xff;

        // update bi_ct
        bi_ct->cycno = (uchar_t) min(cycno + 1, 3);
        lg_pmps += lg_pmps_inc[cwr];
        if (lg_pmps >= (256 << LG_PMPS_SHIFTNO)) {
            lg_pmps = (512 << LG_PMPS_SHIFTNO) - 1 - lg_pmps;
            bi_ct->MPS = (uchar_t) !(bi_ct->MPS);
        }
        bi_ct->LG_PMPS = (i16u_t) lg_pmps;
    }

    return (bit);
}

/*!
************************************************************************
* \brief
*    biari_decode_symbol_eq_prob():
* \return
*    the decoded symbol
************************************************************************
*/
i32u_t biari_decode_symbol_eq_prob(aec_core_t * aec)
{
    register uchar_t s_flag;
    register i32u_t t_rlps;
    register i32u_t t2;
    register uchar_t s2;

    biari_decode_read(aec);

    s_flag = (uchar_t) (aec->t1 < 256);
    s2 = aec->s1 + s_flag;
    t2 = aec->t1 - 256 + (s_flag << 8);

    if (s2 < aec->value_s || (s2 == aec->value_s && aec->value_t < t2) || aec->is_value_bound) { //MPS
        aec->s1 = s2;
        aec->t1 = t2;
        aec->is_value_domain = 0;
        return 0;
    } else {   //LPS
        int vt = aec->value_t;
        aec->is_value_domain = 1;

        t_rlps = 256 + ((-s_flag) & aec->t1);

        if (s2 == aec->value_s) {
            vt -= t2;
        } else {
            if (--aec->bits_to_go < 0) {
                get_byte(aec);
            }

            vt = (vt << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);
            vt += 256 - t2;
        }

        while (t_rlps < QUARTER) {
            t_rlps = t_rlps << 1;

            if (--aec->bits_to_go < 0) {
                get_byte(aec);
            }

            vt = (vt << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);
        }

        aec->value_t = (i32u_t) vt;
        aec->s1 = 0;
        aec->t1 = t_rlps & 0xff;

        return 1;
    }
}

i32u_t biari_decode_final(aec_core_t * aec)
{
    aec_ctx_t octx;
    aec_ctx_t * ctx = &octx;

    register uchar_t s_flag;
    register unsigned int t_rlps;
    register unsigned int t2;
    register uchar_t s2;

    biari_decode_read(aec);
    
    if (aec->t1) {
        s2 = aec->s1;
        t2 = aec->t1 - 1;
        s_flag = 0;
    } else {
        s2 = (uchar_t) (aec->s1 + 1);
        t2 = 255;
        s_flag = 1;
    }

    if (s2 < aec->value_s || (s2 == aec->value_s && aec->value_t < t2) || aec->is_value_bound) { //MPS
        aec->s1 = s2;
        aec->t1 = t2;
        aec->is_value_domain = 0;
        return 0;
    } else {
        if (s2 == aec->value_s) {
            aec->value_t = (aec->value_t - t2);
        } else {
            if (--aec->bits_to_go < 0) {
                get_byte(aec);
            }
            aec->value_t = (aec->value_t << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);      ////R
            aec->value_t = 256 + aec->value_t - t2;
        }
        t_rlps = 1;
        while (t_rlps < QUARTER) {
            t_rlps = t_rlps << 1;

            if (--aec->bits_to_go < 0) {
                get_byte(aec);
            }
            aec->value_t = (aec->value_t << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);
        }

        aec->s1 = 0;
        aec->t1 = 0;
        aec->is_value_domain = 1;
        return 1;
    }
}

i32u_t biari_decode_symbol_continu0(aec_core_t * aec, aec_ctx_t * bi_ct, int max_num)
{
    register uchar_t bit = 0;
    register uchar_t s_flag;
    register uchar_t cwr, cycno;
    register i32u_t lg_pmps;
    register i32u_t t_rlps;
    register i32u_t t2;
    register uchar_t s2;
    register i32u_t lg_pmps_shift;

    int i;

    uchar_t is_bnd    = aec->is_value_bound;
    uchar_t s1        = aec->s1;
    i32u_t  t1        = aec->t1;
    uchar_t is_domain = aec->is_value_domain;
    i32u_t  val_t     = aec->value_t;
    uchar_t val_s     = aec->value_s;
    uchar_t mps       = bi_ct->MPS;

    cycno = bi_ct->cycno;
    lg_pmps = bi_ct->LG_PMPS;

    for (i = 0; i < max_num && !bit; i++) {
        lg_pmps_shift = (lg_pmps >> LG_PMPS_SHIFTNO);
        bit = mps;
        cwr = (uchar_t) max(3, cycno + 2);

        biari_decode_read_inline(aec, s1, val_s, val_t, is_domain, is_bnd);

        s_flag = (uchar_t) (t1 < lg_pmps_shift);
        s2 = s1 + s_flag;
        t2 = t1 - lg_pmps_shift + (s_flag << 8);

        if (is_bnd || (s2 <= val_s && (s2 != val_s || val_t < t2))) { //MPS
            i32u_t tmp_pmps = lg_pmps >> cwr;
            s1 = s2;
            t1 = t2;
            is_domain = 0;
            if (cycno == 0) {
                cycno = 1;
            }
            lg_pmps = lg_pmps - tmp_pmps - (tmp_pmps >> 2);
        } else {   //LPS
            bit = (uchar_t) !bit; //LPS
            is_domain = 1;

            t_rlps = (s_flag == 0) ? lg_pmps_shift : (t1 + lg_pmps_shift);

            if (s2 == val_s) {
                val_t = (val_t - t2);
            } else {
                if (--aec->bits_to_go < 0) {
                    get_byte(aec);
                }

                val_t = (val_t << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);
                val_t = 256 + val_t - t2;
            }

            while (t_rlps < QUARTER) {
                t_rlps = t_rlps << 1;

                if (--aec->bits_to_go < 0) {
                    get_byte(aec);
                }

                val_t = (val_t << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);
            }

            s1 = 0;
            t1 = t_rlps & 0xff;

            // update bi_ct
            cycno = (uchar_t) min(cycno + 1, 3);
            lg_pmps += lg_pmps_inc[cwr];
            if (lg_pmps >= (256 << LG_PMPS_SHIFTNO)) {
                lg_pmps = (512 << LG_PMPS_SHIFTNO) - 1 - lg_pmps;
                mps = (uchar_t) !mps;
            }
        }
    }

    aec->t1              = t1;
    aec->s1              = s1;
    aec->value_t         = val_t;
    aec->value_s         = val_s;
    aec->is_value_bound  = is_bnd;
    aec->is_value_domain = is_domain;

    bi_ct->cycno = cycno;
    bi_ct->LG_PMPS = (i16u_t) lg_pmps;
    bi_ct->MPS = mps;

    return (i32u_t) (i - bit);
}

i32u_t biari_decode_symbol_continu0_ext(aec_core_t * aec, aec_ctx_t * bi_ct, int max_ctx_inc, int max_num)
{
    register uchar_t bit = 0;
    register uchar_t s_flag;
    register uchar_t cwr, cycno;
    register i32u_t lg_pmps;
    register i32u_t t_rlps;
    register i32u_t t2;
    register uchar_t s2;
    register i32u_t lg_pmps_shift;

    int i, ctx_add = 0;
    aec_ctx_t *ctx_base = bi_ct;

    uchar_t is_bnd    = aec->is_value_bound;
    uchar_t s1        = aec->s1;
    i32u_t  t1        = aec->t1;
    uchar_t is_domain = aec->is_value_domain;
    i32u_t  val_t     = aec->value_t;
    uchar_t val_s     = aec->value_s;

    for (i = 0; i < max_num && !bit; i++) {
        bi_ct = ctx_base + ctx_add;
        cycno = bi_ct->cycno;
        lg_pmps = bi_ct->LG_PMPS;
        lg_pmps_shift = (lg_pmps >> LG_PMPS_SHIFTNO);

        bit = bi_ct->MPS;

        cwr = (uchar_t) max(3, cycno + 2);

        biari_decode_read_inline(aec, s1, val_s, val_t, is_domain, is_bnd);

        s_flag = (uchar_t) (t1 < lg_pmps_shift);
        s2 = s1 + s_flag;
        t2 = t1 - lg_pmps_shift + (s_flag << 8);

        if (is_bnd || (s2 <= val_s && (s2 != val_s || val_t < t2))) { //MPS
            i32u_t tmp_pmps = lg_pmps >> cwr;
            s1 = s2;
            t1 = t2;
            is_domain = 0;
            if (cycno == 0) {
                bi_ct->cycno = 1;
            }
            bi_ct->LG_PMPS = (i16u_t) (lg_pmps - tmp_pmps - (tmp_pmps >> 2));
        } else {   //LPS
            bit = (uchar_t) !bit; //LPS
            is_domain = 1;

            t_rlps = (s_flag == 0) ? lg_pmps_shift : (t1 + lg_pmps_shift);

            if (s2 == val_s) {
                val_t = (val_t - t2);
            } else {
                if (--aec->bits_to_go < 0) {
                    get_byte(aec);
                }

                val_t = (val_t << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);
                val_t = 256 + val_t - t2;
            }

            while (t_rlps < QUARTER) {
                t_rlps = t_rlps << 1;

                if (--aec->bits_to_go < 0) {
                    get_byte(aec);
                }

                val_t = (val_t << 1) | ((aec->buffer >> aec->bits_to_go) & 0x01);
            }

            s1 = 0;
            t1 = t_rlps & 0xff;

            // update bi_ct
            bi_ct->cycno = (uchar_t) min(cycno + 1, 3);
            lg_pmps += lg_pmps_inc[cwr];
            if (lg_pmps >= (256 << LG_PMPS_SHIFTNO)) {
                lg_pmps = (512 << LG_PMPS_SHIFTNO) - 1 - lg_pmps;
                bi_ct->MPS = (uchar_t) !(bi_ct->MPS);
            }
            bi_ct->LG_PMPS = (i16u_t) lg_pmps;
        }

        ctx_add++;
        ctx_add = min(ctx_add, max_ctx_inc);
    }

    aec->t1              = t1;
    aec->s1              = s1;
    aec->value_t         = val_t;
    aec->value_s         = val_s;
    aec->is_value_bound  = is_bnd;
    aec->is_value_domain = is_domain;

    return (i32u_t) (i - bit);
}
