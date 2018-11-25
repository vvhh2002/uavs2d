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
#include <string.h>

#include "commonVariables.h"
#include "block_info.h"
#include "ComAdaptiveLoopFilter.h"
#include "DecAdaptiveLoopFilter.h"
#include "global.h"
#include "AEC.h"
#include "biaridecod.h"
#include "math.h"
#include "assert.h"
#include "bitstream.h"

static unsigned int unary_bin_max_decode(aec_core_t * aec, aec_ctx_t * ctx, int ctx_offset, unsigned int max_symbol)
{
    unsigned int symbol;
    aec_ctx_t * ictx;

    symbol = biari_decode_symbol(aec, ctx);

    if (symbol == 1) {
        return 0;
    } else {
        assert(max_symbol != 1);

        symbol = 1;
        ictx = ctx + ctx_offset;

        symbol += biari_decode_symbol_continu0(aec, ictx, max_symbol - symbol);

        return symbol;
    }
}


static unsigned int unary_bin_decode(aec_core_t * aec, aec_ctx_t * ctx, int ctx_offset)
{
    unsigned int symbol;
    aec_ctx_t * ictx;

    symbol = (unsigned int) !biari_decode_symbol(aec, ctx);

    if (symbol == 0) {
        return 0;
    } else {
        symbol = 1;
        ictx = ctx + ctx_offset;
        symbol += biari_decode_symbol_continu0(aec, ictx, 32768);
        return symbol;
    }
}


int aec_dmh_mode(aec_core_t * aec, int cu_bitsize)
{
    static tab_i32s_t iDecMapTab[9] = { 0, 3, 4, 7, 8, 1, 2, 5, 6 };

    aec_ctx_t *ctx = aec->syn_ctx.mvd_contexts[2];
    int offset = (cu_bitsize - 3) * 4;
    int iSymbol;

    if (biari_decode_symbol(aec, &ctx[offset]) == 0) {
        iSymbol = 0;
    } else {
        if (biari_decode_symbol(aec, &ctx[offset + 1]) == 0) {
            iSymbol = biari_decode_symbol_eq_prob(aec) + 1;
        } else {
            if (biari_decode_symbol(aec, &ctx[offset + 2]) == 0) {
                iSymbol = biari_decode_symbol_eq_prob(aec) + 3;
            } else {
                int t1 = biari_decode_symbol_eq_prob(aec);
                int t2 = biari_decode_symbol_eq_prob(aec);
                iSymbol = 5 + (t1 << 1) + t2;
            }
        }
    }

    return iDecMapTab[iSymbol];
}


/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the motion
*    vector data of a B-frame MB.
************************************************************************
*/
int aec_mvd(avs2_dec_t *h_dec, aec_core_t * aec, int xy)
{
    int act_sym;
    int binary_symbol;
    int golomb_order = 0;

    aec_ctx_t *ctx = aec->syn_ctx.mvd_contexts[xy];

    binary_symbol = 0;

    if (!biari_decode_symbol(aec, ctx)) {
        act_sym = 0;
    } else if (!biari_decode_symbol(aec, ctx + 3)) {
        act_sym = 1;
    } else if (!biari_decode_symbol(aec, ctx + 4)) {
        act_sym = 2;
    } else { 
        int add_one = biari_decode_symbol_eq_prob(aec);

        act_sym = 0;

        while (!biari_decode_symbol_eq_prob(aec)) {
            act_sym += (1 << golomb_order);
            golomb_order++;
        }

        while (golomb_order--) {
            if (biari_decode_symbol_eq_prob(aec)) {
                binary_symbol |= (1 << golomb_order);
            }
        }

        act_sym += binary_symbol;
        act_sym = 3 + act_sym * 2 + add_one;

    }

    if (act_sym != 0) {
        act_sym = (biari_decode_symbol_eq_prob(aec)) ? -act_sym : act_sym;
    }

    return act_sym;
}


/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the 8x8 block type.
************************************************************************
*/
int aec_weight_skip_mode(avs2_dec_t *h_dec, aec_core_t * aec)
{
    aec_ctx_t * pCTX = aec->syn_ctx.wpm_contexts;
    return biari_decode_symbol_continu0_ext(aec, pCTX, 2, h_dec->i_refs - 1);
}

int aec_direct_skip(aec_core_t * aec)
{
    aec_ctx_t * pCTX = aec->syn_ctx.b_dir_skip_contexts;
    int act_sym =  biari_decode_symbol_continu0_ext(aec, pCTX, 32768, DIRECTION - 1);
    if (act_sym == DIRECTION - 1) {
        act_sym += (!biari_decode_symbol(aec, pCTX + DIRECTION - 1));
    }
    return act_sym;
}

int aec_pskip_mode(aec_core_t * aec)
{
    aec_ctx_t * pCTX = aec->syn_ctx.p_skip_mode_contexts;
    int act_sym = biari_decode_symbol_continu0_ext(aec, pCTX, 32768, MH_PSKIP_NUM - 1);
    if (act_sym == MH_PSKIP_NUM - 1) {
        act_sym += (!biari_decode_symbol(aec, pCTX + MH_PSKIP_NUM - 1));
    }
    return act_sym;
}

int aec_trans_size(aec_core_t * aec, int cu_bitsize, int b_sdip)
{
    if (cu_bitsize == B64X64_IN_BIT) {
        return 0;
    } else if (cu_bitsize == B8X8_IN_BIT) {
        return biari_decode_symbol(aec, aec->syn_ctx.tu_contexts + 1);
    } else if (b_sdip){
        return biari_decode_symbol(aec, aec->syn_ctx.tu_contexts + 2);
    } else {
        return 0;
    }
}


/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the com_cu_t
*    type info of a given MB.
************************************************************************
*/
int aec_cu_type(avs2_dec_t *h_dec, aec_core_t * aec)
{
    int cu_bitsize = h_dec->cu_bitsize;
    static tab_i32s_t MapCUType[7]    = { -1, 0, 1, 2, 3, PNXN, 9 };
    static tab_i32s_t MapCUTypeMin[6] = { -1, 0, 1, 2, 3, 9 };

    int act_ctx;
    int act_sym;
    int curr_cuType;
    aec_contexts_t *ctx = &aec->syn_ctx;

    aec_ctx_t * pCTX = ctx->cuType_contexts;
    aec_ctx_t * pAMPCTX = ctx->amp_contexts;

    int symbol;
    int max_bit = 6 - (cu_bitsize == B8X8_IN_BIT);

    act_ctx = 0;
    act_sym = 0;

    do {
        if ((act_ctx == 5) && (cu_bitsize != MIN_CU_SIZE_IN_BIT)) {
            symbol = biari_decode_final(aec);
        } else {
            symbol = biari_decode_symbol(aec, pCTX + act_ctx);
        }
        act_ctx++;

    } while (!symbol && ++act_sym < max_bit);

    if (cu_bitsize == B8X8_IN_BIT) {
        act_sym = MapCUTypeMin[act_sym];
    } else {
        act_sym = MapCUType[act_sym];
    }

    curr_cuType = act_sym;

    if (cu_bitsize >= B16X16_IN_BIT && h_dec->seq->inter_amp_enable && (curr_cuType == 2 || curr_cuType == 3)) {
        if (!biari_decode_symbol(aec, pAMPCTX + 0)) {
            curr_cuType = curr_cuType * 2 + (!biari_decode_symbol(aec, pAMPCTX + 1));
        }
    }

    return curr_cuType;
}

int aec_cu_type_sfrm(avs2_dec_t *h_dec, aec_core_t * aec)
{
    int act_ctx;
    int act_sym;
    int curr_cuType;
    const int MapSCUType[7] = { -1, 0, 9 };
    aec_contexts_t *ctx = &aec->syn_ctx;
    aec_ctx_t * pCTX = ctx->cuType_contexts;

    int binidx = 0, symbol;
    int max_bit = 0;

    act_ctx = 0;
    act_sym = 0;
    max_bit = 2;

    while (1) {
        symbol = biari_decode_symbol(aec, pCTX + act_ctx);
        binidx++;

        if (symbol == 0) {
            act_sym++;
            act_ctx++;
        } else {
            break;
        }
        if (act_sym >= max_bit) {
            break;
        }
    }

    act_sym = MapSCUType[act_sym];
    curr_cuType = act_sym;
    return curr_cuType;
}


int aec_sdip(avs2_dec_t *h_dec, aec_core_t * aec)
{
    int cu_bitsize = h_dec->cu_bitsize;
    int curr_cuType = I8MB;

    if (h_dec->cu_loc_dat->trans_size == 1 && (cu_bitsize == 5 || cu_bitsize == 4)) {
        aec_contexts_t *ctx = &aec->syn_ctx;
        aec_ctx_t * pCTX = ctx->cuType_contexts;
        int symbol = biari_decode_symbol(aec, pCTX + 15);

        if (cu_bitsize == MIN_CU_SIZE_IN_BIT) {
            if (symbol) {
                if (biari_decode_symbol(aec, pCTX + 16)) {
                    curr_cuType = I8MB;
                } else {
                    curr_cuType = InNxNMB;
                }
            } else {
                curr_cuType = INxnNMB;
            }
        } else {
            curr_cuType = (symbol == 1 ? InNxNMB : INxnNMB);
        }
        
    }

    return curr_cuType;
}

int aec_bpred_dir(avs2_dec_t *h_dec, aec_core_t * aec)
{
    static tab_i32s_t new_pdir[4] = { 3, 1, 0, 2 };
    static tab_i32s_t dir2offset[4][4] = { 
        {  0,  2,  4,  9 },
        {  3,  1,  5, 10 },
        {  6,  7,  8, 11 },
        { 12, 13, 14, 15 }
    };

    int act_sym;
    int pdir0, pdir1;
    int cu_bitsize = h_dec->cu_bitsize;
    int cuType = h_dec->cu->cuType;
    aec_contexts_t *ctx = &aec->syn_ctx;
    aec_ctx_t * pCTX;

    if ((cuType >= P2NXN && cuType <= PVER_RIGHT) && cu_bitsize == B8X8_IN_BIT) {
        pCTX = ctx->pdirMin_contexts;
    } else {
        pCTX = ctx->pdir_contexts;
    }

    if (cuType == P2NX2N) {
        act_sym = biari_decode_symbol_continu0_ext(aec, pCTX, 32768, 2);
        if (act_sym == 2) {
            act_sym += (!biari_decode_symbol(aec, pCTX + 2));
        }
        pdir0 = act_sym;
    } 
    else if ((cuType >= P2NXN && cuType <= PVER_RIGHT) && cu_bitsize == B8X8_IN_BIT) {
        pdir0 = !biari_decode_symbol(aec, pCTX);
        pdir1 = pdir0 ^ (!biari_decode_symbol(aec, pCTX + 1));
        act_sym = dir2offset[pdir0][pdir1];
    } 
    else if (cuType >= P2NXN || cuType <= PVER_RIGHT) {
        act_sym = biari_decode_symbol_continu0_ext(aec, pCTX + 4, 32768, 2);
        if (act_sym == 2) {
            act_sym += (!biari_decode_symbol(aec, pCTX + 2 + 4));
        }
        pdir0 = act_sym;

        if (biari_decode_symbol(aec, pCTX + 8)) {
            pdir1 = pdir0;
        } else {
            switch (pdir0) {
            case 0:
                if (biari_decode_symbol(aec, pCTX + 9)) {
                    pdir1 = 1;
                } else {
                    if (biari_decode_symbol(aec, pCTX + 10)) {
                        pdir1 = 2;
                    } else {
                        pdir1 = 3;
                    }
                }
                break;
            case 1:
                if (biari_decode_symbol(aec, pCTX + 11)) {
                    pdir1 = 0;
                } else {
                    if (biari_decode_symbol(aec, pCTX + 12)) {
                        pdir1 = 2;
                    } else {
                        pdir1 = 3;
                    }
                }
                break;

            case 2:
                if (biari_decode_symbol(aec, pCTX + 13)) {
                    pdir1 = 0;
                } else {
                    if (biari_decode_symbol(aec, pCTX + 14)) {
                        pdir1 = 1;
                    } else {
                        pdir1 = 3;
                    }
                }
                break;
            case 3:
                if (biari_decode_symbol(aec, pCTX + 15)) {
                    pdir1 = 0;
                } else {
                    if (biari_decode_symbol(aec, pCTX + 16)) {
                        pdir1 = 1;
                    } else {
                        pdir1 = 2;
                    }
                }
                break;
            }

        }
        pdir0 = new_pdir[pdir0];
        pdir1 = new_pdir[pdir1];
        act_sym = dir2offset[pdir0][pdir1];
    }

    return act_sym;
}


/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the PU type.
************************************************************************
*/
int aec_p_dhp_dir(aec_core_t * aec, com_cu_t *cu)
{
    static tab_i32s_t dir2offset[2][2] = { 
        { 0, 1 }, 
        { 2, 3 } 
    };

    aec_ctx_t * pCTX = aec->syn_ctx.pdir_dhp_contexts;

    if (cu->cuType == P2NX2N) {
        return biari_decode_symbol(aec, pCTX) ;
    } else {
        int pdir0, pdir1;
        pdir0 = biari_decode_symbol(aec, pCTX + 1);
        pdir1 = pdir0 ^ (!biari_decode_symbol(aec, pCTX + 2));
        return dir2offset[pdir0][pdir1];
    }
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode a pair of
*    intra prediction modes of a given MB.
************************************************************************
*/
int aec_intra_pred_mode(aec_core_t * aec)
{
    aec_ctx_t * pCTX;
    
    pCTX = aec->syn_ctx.l_intra_mode_contexts;

    if (biari_decode_symbol(aec, pCTX) == 1) {
        return biari_decode_symbol(aec, pCTX + 6) - 2;
    } else {
        int symbol;
        symbol  = biari_decode_symbol(aec, pCTX + 1) << 4;
        symbol += biari_decode_symbol(aec, pCTX + 2) << 3;
        symbol += biari_decode_symbol(aec, pCTX + 3) << 2;
        symbol += biari_decode_symbol(aec, pCTX + 4) << 1;
        symbol += biari_decode_symbol(aec, pCTX + 5);
        return symbol;
    }
}
/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the reference
*    parameter of a given MB.
************************************************************************
*/

int aec_ref_index(avs2_dec_t *h_dec, aec_core_t * aec, int start_x, int start_y)
{
    aec_contexts_t *ctx = &aec->syn_ctx;
    int act_sym;

    if (biari_decode_symbol(aec, ctx->ref_no_contexts)) {
        return 0;
    } else {
        aec_ctx_t * pCTX = ctx->ref_no_contexts + 4;
        int i_refs = h_dec->i_refs - 1;

        if (1 >= i_refs || biari_decode_symbol(aec, pCTX)) {
            return 1;
        } else {
            act_sym = 2;
            pCTX++;
            act_sym += biari_decode_symbol_continu0(aec, pCTX, i_refs - act_sym);
            return act_sym;
        }
    }
}



/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the delta qp
*     of a given MB.
************************************************************************
*/
int aec_delta_qp(avs2_dec_t *h_dec, aec_core_t * aec)
{
    aec_contexts_t *ctx = &aec->syn_ctx;

    int act_ctx;
    int act_sym;
    int dquant;

    act_ctx = ((h_dec->last_dquant != 0) ? 1 : 0);
    act_sym = !biari_decode_symbol(aec, ctx->delta_qp_contexts + act_ctx);

    if (act_sym != 0) {
        act_ctx = 2;
        act_sym = unary_bin_decode(aec, ctx->delta_qp_contexts + act_ctx, 1);
        act_sym++;
    }

    dquant = (act_sym + 1) / 2;

    if ((act_sym & 0x01) == 0) {                      // lsb is signed bit
        dquant = -dquant;
    }

    h_dec->last_dquant = dquant;
    return dquant;
}
/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the coded
*    block pattern of a given MB.
************************************************************************
*/
int readCBP_BIT_AEC(avs2_dec_t *h_dec, int b8, aec_core_t * aec)
{
    if (b8 == 4) {
        return biari_decode_symbol(aec, aec->syn_ctx.cbp_contexts[2]);
    } else {
        aec_ctx_t * pCTX;
        com_cu_t *cu = h_dec->cu;
        int cu_b4_size = (1 << cu->ui_MbBitSize) / 4;
        int tu_b4_x, tu_b4_y;
        char_t *nz;
        int i_b4 = h_dec->seq->b4_info_stride;
        int nz_left = 0, nz_top = 0, a, b;
        int cbp = cu->cbp;

        if (h_dec->nsqt_shape == 1) {
            tu_b4_x = h_dec->cu_b4_x;
            tu_b4_y = h_dec->cu_b4_y + b8 * cu_b4_size / 4;
        } else if (h_dec->nsqt_shape == 2) {
            tu_b4_x = h_dec->cu_b4_x + b8 * cu_b4_size / 4;
            tu_b4_y = h_dec->cu_b4_y;
        } else {
            tu_b4_x = h_dec->cu_b4_x + (b8 % 2) * cu_b4_size / 2;
            tu_b4_y = h_dec->cu_b4_y + (b8 / 2) * cu_b4_size / 2;
        }
    
        nz = h_dec->nz + tu_b4_y * i_b4 + tu_b4_x;

        if (tu_b4_x == h_dec->cu_b4_x) {
            nz_left = nz[-1];
        } else {
            nz_left = cbp & (1 << (b8 - 1));
        }

        if (tu_b4_y == h_dec->cu_b4_y) {
            nz_top = nz[-i_b4];
        } else if (h_dec->nsqt_shape == 0) {
            nz_top = cbp & (1 << (b8 - 2));
        } else {
            nz_top = cbp & (1 << (b8 - 1));
        }

        a = nz_left ? 1 : 0;
        b = nz_top  ? 1 : 0;

        pCTX = aec->syn_ctx.cbp_contexts[0] + a + 2 * b;

        return biari_decode_symbol(aec, pCTX);
    }
}

int aec_cbp(avs2_dec_t *h_dec, aec_core_t * aec)
{
    int cu_idx = h_dec->cu_idx;
    com_cu_t *cu = h_dec->cu;
    aec_ctx_t *ctx = aec->syn_ctx.cbp_contexts[1];
    int cbp = 0;
    int cbp_bit = 0;
    com_cu_local_t *cu_loc_dat = h_dec->cu_loc_dat;

    if (!IS_INTRA(cu)) {
        if (!(IS_B_SKIP(cu) || IS_P_SKIP(cu))) {
            cbp_bit = readCBP_BIT_AEC(h_dec, 4, aec);
        }
        if (cbp_bit == 0) {
            cu_loc_dat->trans_size = (char_t) biari_decode_symbol(aec, aec->syn_ctx.tu_contexts);

            if (cu_loc_dat->trans_size == 0) {
                if (biari_decode_symbol(aec, ctx) == 0) {
                    cbp = 15;
                } else { 
                    /* chroma */
                    if (biari_decode_symbol(aec, ctx + 2)) {
                        cbp += 48;
                    } else {
                        cbp += biari_decode_symbol(aec, ctx + 2) ? 32 : 16;
                    }
                    /* luma */
                    cbp += readCBP_BIT_AEC(h_dec, 0, aec) ? 15 : 0;
                }
            } else {
                /* chroma */
                if (biari_decode_symbol(aec, ctx)) {
                    if (biari_decode_symbol(aec, ctx + 2)) {
                        cbp += 48;
                    } else {
                        cbp += biari_decode_symbol(aec, ctx + 2) ? 32 : 16;
                    }
                }

                /* luma */
                cu->cbp  = (readCBP_BIT_AEC(h_dec, 0, aec));
                cu->cbp += (readCBP_BIT_AEC(h_dec, 1, aec) << 1);
                cu->cbp += (readCBP_BIT_AEC(h_dec, 2, aec) << 2);
                cu->cbp += (readCBP_BIT_AEC(h_dec, 3, aec) << 3);
                cbp += cu->cbp;
            }
            
        } else {
            cu_loc_dat->trans_size = 1;
            cbp = 0;
        }
    } else {
        cu_loc_dat->trans_size = (char_t) (cu->cuType == I16MB ? 0 : 1);

        /* luma */
        if (cu->cuType == I16MB) {
            cbp = readCBP_BIT_AEC(h_dec, 0, aec) ? 15 : 0;
        } else {
            cu->cbp  = (readCBP_BIT_AEC(h_dec, 0, aec));
            cu->cbp += (readCBP_BIT_AEC(h_dec, 1, aec) << 1);
            cu->cbp += (readCBP_BIT_AEC(h_dec, 2, aec) << 2);
            cu->cbp += (readCBP_BIT_AEC(h_dec, 3, aec) << 3);
            cbp = cu->cbp;
        }
        
        /* chroma */
        if (biari_decode_symbol(aec, ctx + 1)) {
            if (biari_decode_symbol(aec, ctx + 3)) {
                cbp += 48;
            } else {
                cbp_bit = biari_decode_symbol(aec, ctx + 3);
                cbp += (cbp_bit == 1) ? 32 : 16;
            }
        }
    }

    return cbp;
}

int aec_intra_pred_mode_c(avs2_dec_t *h_dec, aec_core_t * aec)
{
    aec_contexts_t *ctx = &aec->syn_ctx;
    com_cu_t *cu = h_dec->cu;
    int act_ctx;
    int act_sym; 
    int lmode = 0;
    int is_redundant = 0;
    int b4y = h_dec->cu_b8_y << 1;
    int b4x = h_dec->cu_b8_x << 1;
    int LumaMode = h_dec->ipredmode[b4y * h_dec->seq->b4_info_stride + b4x];

    switch (LumaMode)
    {
    case VERT_PRED:
        lmode = VERT_PRED_C;
        is_redundant = 1;
        break;
    case HOR_PRED:
        lmode = HOR_PRED_C;
        is_redundant = 1;
        break;
    case DC_PRED:
        lmode = DC_PRED_C;
        is_redundant = 1;
        break;
    case BI_PRED:
        lmode = BI_PRED_C;
        is_redundant = 1;
        break;
    default:
        break;
    }

    act_ctx = h_dec->cu_pix_x > 0 && cu->slice_nr == cu[-1].slice_nr && cu[-1].c_ipred_mode;

    act_sym = !biari_decode_symbol(aec, ctx->c_intra_mode_contexts + act_ctx);

    if (act_sym != 0) {
        // check point
        act_sym = unary_bin_max_decode(aec, ctx->c_intra_mode_contexts + 3, 0, 3) + 1;
        if (is_redundant && act_sym >= lmode) {
            act_sym++;
        }
    }

    return act_sym;
}



// 0: INTRA_PRED_VER
// 1: INTRA_PRED_HOR
// 2: INTRA_PRED_DC_DIAG

tab_i32s_t tab_intraModeClassified[NUM_INTRA_PMODE] = { 
    2, 2, 2, 1, 1, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 0
};

static tab_i32s_t tab_raster2ZZ_4x4[] = {
    0, 1, 5, 6,
    2, 4, 7, 12,
    3, 8, 11, 13,
    9, 10, 14, 15
};
static tab_i32s_t tab_raster2ZZ_8x8[] = {
    0, 1, 5, 6, 14, 15, 27, 28,
    2, 4, 7, 13, 16, 26, 29, 42,
    3, 8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

static tab_i32s_t tab_raster2ZZ_2x8[] = {
    0, 1, 4, 5, 8, 9, 12, 13,
    2, 3, 6, 7, 10, 11, 14, 15
};

static tab_i32s_t tab_raster2ZZ_8x2[] = {
    0, 1,
    2, 4,
    3, 5,
    6, 8,
    7, 9,
    10, 12,
    11, 13,
    14, 15
};

static tab_i32s_t tab_zigzag[4][4] = {
    { 0, 1, 5, 6 }, 
    { 2, 4, 7, 12 }, 
    { 3, 8, 11, 13 }, 
    { 9, 10, 14, 15 } 
};

static tab_i32s_t level_rank[6][5] = {
    { 0, 1, 2, 3, 4 },
    { 1, 1, 2, 3, 4 },
    { 2, 2, 2, 3, 4 },
    { 3, 3, 3, 3, 4 },
    { 3, 3, 3, 3, 4 },
    { 4, 4, 4, 4, 4 }
};

static tab_i32s_t ctx_pos_tab[2][2][16] = { { { -1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3 }, { -1, 1, 1, 2, 2, 2, 1, 1, 2, 2, 3, 3, 2, 2, 2, 3 } },
                                          { { -1, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3 }, { -1, 0, 1, 2, 2, 2, 1, 1, 2, 2, 3, 3, 2, 2, 2, 3 } } };

static tab_i32s_t T_Chr[5] = { 0, 1, 2, 4, 3000 };

int aec_run_level(avs2_dec_t *h_dec, aec_core_t * aec, com_cu_t *cu, int b8, int *run_buf, int*level_buf, int* cg_flag, int* pairs_in_cg, int* cg_num, int plane)
{
    int pairs, rank, pos;
    int Run, absLevel;
    int sigCGFlag = 1, firstCG = 0;
    int iCG, numOfCG;
    int i;
    int CGLastX = 0;
    int CGLastY = 0;
    int CGLast = 0;
    int CGx = 0, CGy = 0;
    int prevCGFlagSum;
    int cu_bitsize = h_dec->cu_bitsize;
    int cuType = cu->cuType;
    int xx, yy;
    int numOfCoeff = (1 << (cu_bitsize - 1)) * (1 << (cu_bitsize - 1));
    int bitSize = cu_bitsize - MIN_CU_SIZE_IN_BIT;

    int isChroma = 0;
    int ctxmode = INTRA_PRED_DC_DIAG;
    int golomb_order = 0, binary_symbol = 0;

    int pairs_prev;

    com_cu_local_t *cu_loc_dat = h_dec->cu_loc_dat;

    run_buf[0] = 0;

    if (plane == LUMA_8x8 && IS_INTRA(cu)) {
        ctxmode = tab_intraModeClassified[cu_loc_dat->intra_pred_modes[b8]];
        if (ctxmode == INTRA_PRED_HOR) {
            ctxmode = INTRA_PRED_VER;
        }
    }

    if (cu_loc_dat->trans_size == 0 && cu_bitsize != B64X64_IN_BIT && plane == LUMA_8x8) {
        numOfCoeff = (1 << cu_bitsize) * (1 << cu_bitsize);
        bitSize = bitSize + 1;
    }

    if (h_dec->seq->useNSQT && cu_bitsize > B8X8_IN_BIT && ((cu_loc_dat->trans_size == 1) && (cu_bitsize == B64X64_IN_BIT) && (IS_INTER(cu)) && (cuType >= P2NXN && cuType <= PVER_RIGHT) && (plane == LUMA_8x8))) {
        numOfCoeff = (1 << (cu_bitsize - 2)) * (1 << (cu_bitsize - 2));
        bitSize -= 1;
    }

    numOfCG = (numOfCoeff >> 4);

    //--- read coefficients for whole block ---
    {
        int iVer = 0, iHor = 0;
        int nsqt_sdip_sw;
        int numCGminus1X, numCGminus1Y;

        aec_ctx_t(*Primary) [NUM_MAP_CTX];
        aec_ctx_t * pCTX;
        aec_ctx_t * pCTXSigCG;
        aec_ctx_t * pCTXLastCG;
        aec_ctx_t * pCTXLastPosInCG;
        int offset, sigCGctx;

        if (plane == LUMA_8x8) {
            Primary = aec->syn_ctx.map_contexts;
            pCTXSigCG = aec->syn_ctx.sigCG_contexts;
            pCTXLastCG = aec->syn_ctx.lastCG_contexts;
            pCTXLastPosInCG = aec->syn_ctx.lastPos_contexts;
        } else {
            isChroma = 1;
            Primary = aec->syn_ctx.last_contexts;
            pCTXSigCG = aec->syn_ctx.sigCG_contexts + NUM_SIGCG_CTX_LUMA;
            pCTXLastCG = aec->syn_ctx.lastCG_contexts + NUM_LAST_CG_CTX_LUMA;
            pCTXLastPosInCG = aec->syn_ctx.lastPos_contexts + NUM_LAST_POS_CTX_LUMA;
        }

        //! Decode
        rank = 0;
        pairs = 0;

        *cg_num = 1;
        cg_flag[ 0 ] = 1;

        if (cu_loc_dat->trans_size == 1 && !isChroma) {
            if (h_dec->seq->useNSQT && cu_bitsize > B8X8_IN_BIT && IS_INTER(cu)) {
                if (cuType == P2NXN || cuType == PHOR_UP || cuType == PHOR_DOWN) {
                    iHor = 1;
                    nsqt_sdip_sw = 1;
                } else if (cuType == PNX2N || cuType == PVER_LEFT || cuType == PVER_RIGHT) {
                    iVer = 1;
                    nsqt_sdip_sw = 1;
                }
            } else if (h_dec->seq->useSDIP && IS_INTRA(cu)) {
                if (cuType == InNxNMB) {
                    iHor = 1;
                    nsqt_sdip_sw = 0;
                } else if (cuType == INxnNMB) {
                    iVer = 1;
                    nsqt_sdip_sw = 0;
                }
            }
        }

        if (iHor){
            numCGminus1X = (1 << (bitSize + 1)) - 1;
            numCGminus1Y = (1 << (bitSize - 1)) - 1;
        } else if (iVer) {
            numCGminus1X = (1 << (bitSize - 1)) - 1;
            numCGminus1Y = (1 << (bitSize + 1)) - 1;
        } else {
            numCGminus1X = numCGminus1Y = (1 << bitSize) - 1;
        }

        if (plane == LUMA_8x8 && ctxmode == INTRA_PRED_DC_DIAG) {
            SWAP(numCGminus1X, numCGminus1Y);
        }

        for (iCG = 0; iCG < numOfCG && !firstCG; iCG++) {
            pairs_in_cg[ iCG ] = 0;

            //! Last CG POSITION
            if (rank == 0 && bitSize) {
                int numCGminus1 = numOfCG - 1;

                if (bitSize == 0) { //4x4
                    // no need to decode LastCG0Flag
                    CGLast = 0;
                    CGLastX = 0;
                    CGLastY = 0;
                } else if (bitSize == 1) { // 8x8
                    CGLast = 0;
                    CGLast += biari_decode_symbol_continu0_ext(aec, pCTXLastCG, 2, 3);
                    
                    if (iHor) {
                        CGLastX = CGLast;
                        CGLastY = 0;
                    } else if (iVer) {
                        CGLastX = 0;
                        CGLastY = CGLast;
                    } else {
                        CGLastX = CGLast & 1;
                        CGLastY = CGLast >> 1;
                    }
                } else { // 16x16 and 32x32
                    int offset = isChroma ? 3 : 9;
                    if (biari_decode_symbol(aec, pCTXLastCG + offset) == 0) {
                        CGLastX = 0;
                        CGLastY = 0;
                        CGLast  = 0;
                    } else {
                        offset = isChroma ? 4 : 10;
                        CGLastX += biari_decode_symbol_continu0(aec, pCTXLastCG + offset, numCGminus1X - CGLastX);
                        offset++;
                        if (CGLastX == 0) {
                            if (numCGminus1Y != 1) {
                                CGLastY += biari_decode_symbol_continu0(aec, pCTXLastCG + offset, numCGminus1Y - 1 - CGLastY);
                            }
                            CGLastY ++;
                        } else {
                            CGLastY += biari_decode_symbol_continu0(aec, pCTXLastCG + offset, numCGminus1Y - CGLastY);
                        }

                    }
                    if (plane == LUMA_8x8 && ctxmode == INTRA_PRED_DC_DIAG) {
                        SWAP(CGLastX, CGLastY);
                    }

                    if ((iHor || iVer) && (bitSize != 2) && nsqt_sdip_sw == 0) {
                        printf("erro decoder run level \n");
                    } else {
                        if (iHor) {
                            CGLast = tab_raster2ZZ_2x8[CGLastY * 8 + CGLastX];
                        } else if (iVer) {
                            CGLast = tab_raster2ZZ_8x2[CGLastY * 2 + CGLastX];
                        } else {
                            if (bitSize == 2) {
                                CGLast = tab_raster2ZZ_4x4[CGLastY * 4 + CGLastX];
                            } else {
                                CGLast = tab_raster2ZZ_8x8[CGLastY * 8 + CGLastX];
                            }
                        }
                    }
                }
                CGx = CGLastX;
                CGy = CGLastY;
            }

            firstCG = (CGLast == 0);


            //! Sig CG Flag
            if (rank > 0) {
                prevCGFlagSum = (iCG < 1 ? 0 : cg_flag[iCG - 1 ]) * 2 + (iCG < 2 ? 0 : cg_flag[ iCG - 2 ]);

                if (bitSize == 1) {
                    if (iHor) {
                        CGx = CGLast;
                        CGy = 0;
                    } else if (iVer) {
                        CGx = 0;
                        CGy = CGLast;
                    } else {
                        CGx = CGLast & 1;
                        CGy = CGLast >> 1;
                    }
                } else if (bitSize == 2) {
                    if (iHor) {
                        CGx = tab_scan_2x8[CGLast][0];
                        CGy = tab_scan_2x8[CGLast][1];
                    } else if (iVer) {
                        CGx = tab_scan_8x2[CGLast][0];
                        CGy = tab_scan_8x2[CGLast][1];
                    } else {
                        CGx = tab_scan_4x4[CGLast][0];
                        CGy = tab_scan_4x4[CGLast][1];
                    }
                } else if (bitSize == 3) {
                    if ((iHor || iVer) && nsqt_sdip_sw == 0) {
                        printf("erro decoder CGx  \n");
                    } else {
                        if (iHor) {
                            CGx = tab_scan_2x8[CGLast][0];
                            CGy = tab_scan_2x8[CGLast][1];
                        } else if (iVer) {
                            CGx = tab_scan_8x2[CGLast][0];
                            CGy = tab_scan_8x2[CGLast][1];
                        } else {
                            CGx = tab_scan_8x8[CGLast][0];
                            CGy = tab_scan_8x8[CGLast][1];
                        }
                    }
                }
                sigCGctx = isChroma ? 0 : ((CGLast == 0) ?  0 : 1);
                sigCGFlag  = biari_decode_symbol(aec, pCTXSigCG + sigCGctx);
                cg_flag[ iCG ]  = sigCGFlag;
                *cg_num += 1;
            }

            if (sigCGFlag || rank == 0) {
                int offset_add;
                int pairsInCG = 0;

                pairs_prev = pairs;

                if (plane == LUMA_8x8) {
                    if (bitSize) {
                        offset = ((CGx > 0 && CGy > 0) ? 0 : (((ctxmode >> 1) << 2) + (firstCG ? 4 : 12))) + 8;
                    } else {
                        offset = (ctxmode >> 1) << 2;
                    }
                    offset_add = bitSize ? 4 : 0;
                } else {
                    offset     = bitSize ? 4 : 0;
                    offset_add = bitSize ? 3 : 0;
                }

                if (iCG) {
                    offset += (isChroma ? NUM_LAST_POS_CTX_CHROMA / 2 : NUM_LAST_POS_CTX_LUMA / 2);
                }

                xx = biari_decode_symbol_continu0_ext(aec, pCTXLastPosInCG + offset    , 1, 3);
                yy = biari_decode_symbol_continu0_ext(aec, pCTXLastPosInCG + offset + 2, 1, 3);

                if (CGx == 0 && CGy > 0 && ctxmode == 2) {
                    yy = yy ^ xx;
                    xx = yy ^ xx;
                    yy = xx ^ yy;
                }

                if (rank) {
                    xx = 3 - xx;
                    if (ctxmode) {
                        yy = 3 - yy;
                    }
                }
                pos = 15 - tab_zigzag[yy][xx];

                for (; pos < 16; pairs++, pairsInCG++) {
                    /* read level */
                    absLevel = 0;

                    if (biari_decode_final(aec)) {
                        golomb_order = 0;
                        binary_symbol = 0;
                        while (!biari_decode_symbol_eq_prob(aec)) {
                            absLevel += (1 << golomb_order);
                            golomb_order++;
                        }
                        
                        while (golomb_order--) {
                            if (biari_decode_symbol_eq_prob(aec)) {
                                binary_symbol |= (1 << golomb_order);
                            }
                        }

                        absLevel += binary_symbol;
                        absLevel += 32;
                    } else {
                        int indiv = ((pairsInCG + 1) >> 1) + 2;
                        indiv = min(4, indiv);
                        offset = ((firstCG && pos > 12) ? 0 : 3) + indiv + 6;

                        if (!isChroma) {
                            offset += 3;
                        }
                        pCTX = Primary[min(rank, indiv)];
                        absLevel += biari_decode_symbol_continu0(aec, pCTX + offset, 31 - absLevel);
                    }

                    absLevel++;

                    /* read run */
                    Run = 0;

                    if (pos < 15) {
                        int pos_left = 15 - pos;
                        int absSum5 = absLevel;
                        int n, k;
                        int *p_run = run_buf + pairs - 1;
                        int *p_lev = level_buf + pairs - 1;
                 
                        n = *(p_run--) + 1;

                        for (k = 0; k < pairsInCG && n < 7; k++) {
                            absSum5 += *(p_lev--);
                            n += *(p_run--) + 1;
                        }

                        absSum5 >>= 1;
                        pCTX = Primary[min(absSum5, 2)] + offset_add + (firstCG ? 0 : 3);

                        if (plane == LUMA_8x8) {
                            tab_i32s_t *offset_tab = ctx_pos_tab[firstCG][ctxmode == INTRA_PRED_VER] + pos_left;
                            while (!biari_decode_symbol(aec, pCTX + *(offset_tab--)) && --pos_left) {}
                        } else {
                            if (firstCG) {
                                do {
                                    offset = (pos_left > 1) + (pos_left >= 6);
                                } while (!biari_decode_symbol(aec, pCTX + offset) && --pos_left);
                            } else {
                                do {
                                    offset = (pos_left >= 6);
                                } while (!biari_decode_symbol(aec, pCTX + offset) && --pos_left);
                            }
                        }
                        Run = 15 - pos - pos_left;
                    }

                    level_buf[pairs] = absLevel;
                    run_buf  [pairs] = Run;

                    pairs_in_cg[iCG] ++;

                    if (rank < 4) {
                        rank = level_rank[min(absLevel, 5)][rank];
                    }

                    pos += (Run + 1);
                }

                //Sign of Level
                pairs = pairs_prev;
                for (i = pairs_in_cg[ iCG ]; i > 0; i--, pairs++) {
                    if (biari_decode_symbol_eq_prob(aec)) {
                        level_buf[pairs] = -level_buf[pairs];
                    }
                }
            }

            CGLast --;
        }

    }
    return pairs;
}


/*!
************************************************************************
* \brief
*    finding end of a slice in case this is not the end of a frame
*
* Unsure whether the "correction" below actually solves an off-by-one
* problem or whether it introduces one in some cases :-(  Anyway,
* with this change the bit stream format works with AEC again.
* StW, 8.7.02
************************************************************************
*/
int AEC_startcode_follows(avs2_dec_t *h_dec, int eos_bit)
{
    unsigned int  bit;
    aec_core_t *aec = &(h_dec->aec_core);

    if (eos_bit) {
        bit = biari_decode_final(aec);    //GB
    } else {
        bit = 0;
    }

    return (bit == 1 ? 1 : 0);
}

int aec_cu_split_flag(avs2_dec_t *h_dec, int uiBitSize)
{
    aec_core_t  *aec = &h_dec->aec_core;
    aec_ctx_t   *ctx = aec->syn_ctx.split_contexts + uiBitSize;
    return biari_decode_symbol(aec, ctx);
}

int aec_sao_merge_flag(avs2_dec_t *h_dec, int mergeleft_avail, int mergeup_avail, int cu_idx)
{
    aec_core_t  *aec = &h_dec->aec_core;
    int MergeLeft = 0;
    int MergeUp = 0;
    aec_contexts_t  *ctx = &aec->syn_ctx;
    int act_ctx = mergeleft_avail + mergeup_avail;
    int act_sym;

    if (act_ctx == 1) {
        act_sym = biari_decode_symbol(aec, &ctx->saomergeflag_context[0]);
    } else if (act_ctx == 2) {
        act_sym  = biari_decode_symbol(aec, &ctx->saomergeflag_context[1]);
        if (act_sym != 1) {
            act_sym += (biari_decode_symbol(aec, &ctx->saomergeflag_context[2]) << 1);
        }
    }

    if (mergeleft_avail) {
        MergeLeft = act_sym & 0x01;
        act_sym = act_sym >> 1;
    }

    if (mergeup_avail && !MergeLeft) {
        MergeUp = act_sym & 0x01;
    }
    return (MergeLeft << 1) + MergeUp;
}

int aec_sao_mode(avs2_dec_t *h_dec, int cu_idx)
{
    aec_core_t  *aec = &h_dec->aec_core;
    aec_contexts_t  *ctx = &aec->syn_ctx;
    int act_sym = 0;
    int t;

    if (!biari_decode_symbol(aec, ctx->saomode_context)) {
        t = !biari_decode_symbol_eq_prob(aec);
        act_sym = 1 + (t << 1);
    }
    return act_sym;
}

int sao_offset(aec_core_t * aec, com_cu_t *cu, int cu_idx, int offset_type)
{
    static tab_i32s_t EO_OFFSET_INV__MAP[] = { 1, 0, 2, -1, 3, 4, 5, 6 };
    static tab_i32s_t saoclip[NUM_SAO_OFFSET][3] = { //EO
        { -1, 6, 7 }, //low bound, upper bound, threshold
        {  0, 1, 1 },
        {  0, 0, 0 },
        { -1, 0, 1 },
        { -6, 1, 7 },
        { -7, 7, 7 }
    };

    aec_contexts_t  *ctx = &aec->syn_ctx;
    int  act_sym, sym, cnt ;
    int signsymbol = 0;
    int maxvalue;

    maxvalue = saoclip[offset_type][2];
    cnt = 0;

    if (offset_type == SAO_CLASS_BO) {
        sym = !biari_decode_symbol(aec,  &ctx->saooffset_context[0]);
    } else {
        sym  = !biari_decode_symbol_eq_prob(aec);
    }
    while (sym) {
        cnt++;
        if (cnt == maxvalue) {
            break;
        }
        sym = !biari_decode_symbol_eq_prob(aec);
    }

    if (offset_type == SAO_CLASS_EO_FULL_VALLEY) {
        act_sym = EO_OFFSET_INV__MAP[cnt];
    } else if (offset_type == SAO_CLASS_EO_FULL_PEAK) {
        act_sym = -EO_OFFSET_INV__MAP[cnt];
    } else if (offset_type == SAO_CLASS_EO_HALF_PEAK) {
        act_sym = -cnt;
    } else {
        act_sym = cnt;
    }

    if (offset_type == SAO_CLASS_BO && act_sym) {
        signsymbol = biari_decode_symbol_eq_prob(aec);
    }
    if (signsymbol) {
        act_sym = -act_sym;
    }
    return act_sym;
}

void aec_sao_offsets(avs2_dec_t *h_dec, sap_param_t *saoBlkParam, int cu_idx, int offsetTh, int *offset)
{
    aec_core_t* aec;
    int i;
    int offset_type;
    aec = &(h_dec->aec_core);
    assert(saoBlkParam->modeIdc == SAO_MODE_NEW);

    for (i = 0; i < 4; i++) {
        if (saoBlkParam->typeIdc == SAO_TYPE_BO) {
            offset_type = SAO_CLASS_BO;
        }
        else {
            offset_type = (i >= 2) ? (i + 1) : i;
        }
        offset[i] = sao_offset(aec, NULL, cu_idx, offset_type);
    }
}

int read_sao_type_base(aec_core_t * aec, com_cu_t *cu, int cu_idx, int index)
{
    aec_contexts_t  *ctx = &aec->syn_ctx;
    int  act_sym = 0;
    int temp;
    int i, length;
    int golomb_order, rest;

    temp = 0;

    golomb_order = 1;

    if (index == 0) {
        length = NUM_SAO_EO_TYPES_LOG2;
    }
    else if (index == 1) {
        length = NUM_SAO_BO_CLASSES_LOG2;
    }
    else {
        length = NUM_SAO_BO_CLASSES_LOG2 - 1;
        assert(index == 2);
    }

    if (index == 2) {
        do {
            temp = biari_decode_symbol_eq_prob(aec);

            if (temp == 0) {
                act_sym += (1 << golomb_order);
                golomb_order++;
            }
            if (golomb_order == 4) {
                golomb_order = 0;
                temp = 1;
            }
        } while (temp != 1);
        rest = 0;
        while (golomb_order--) {
            //next binary part
            temp = biari_decode_symbol_eq_prob(aec);

            if (temp == 1) {
                rest |= (temp << golomb_order);
            }
        }

        act_sym += rest;

    }
    else {
        for (i = 0; i < length; i++) {
            act_sym = act_sym + (biari_decode_symbol_eq_prob(aec) << i);
        }
    }
    return act_sym;
}


int aec_sao_type(avs2_dec_t *h_dec, sap_param_t *saoBlkParam, int cu_idx)
{
    aec_core_t  *aec;
    int stBnd[2];
    aec = &(h_dec->aec_core);
    assert(saoBlkParam->modeIdc == SAO_MODE_NEW);

    if (saoBlkParam->typeIdc == SAO_TYPE_BO) {
        stBnd[0] = read_sao_type_base(aec, NULL, cu_idx, 1);
        stBnd[1] = read_sao_type_base(aec, NULL, cu_idx, 2) + 2;
        return (stBnd[0] + (stBnd[1] << NUM_SAO_BO_CLASSES_LOG2));
    } else {
        return read_sao_type_base(aec, NULL, cu_idx, 0);
    }
}

int aec_alf_lcu_ctrl(avs2_dec_t *h_dec, aec_core_t * aec)
{
    int ruiVal;
    aec_contexts_t  *ctx = &aec->syn_ctx;
    ruiVal = biari_decode_symbol(aec, ctx->m_cALFLCU_Enable_SCModel);

    return ruiVal;

}