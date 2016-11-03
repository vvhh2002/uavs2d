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

#include "intrinsic.h"


void SAO_on_block_sse128(void *p1, void *p2, void *p3, int compIdx, int smb_index, int pix_y, int pix_x, int smb_pix_height, 
    int smb_pix_width, int smb_available_left, int smb_available_right, int smb_available_up, int smb_available_down, 
	int smb_available_upleft, int smb_available_upright, int smb_available_leftdown, int smb_available_rightdwon, int sample_bit_depth)
{
    com_pic_t *pic_src = (com_pic_t*)p1;
    com_pic_t *pic_dst = (com_pic_t*)p2;
    sap_param_t *saoBlkParam = (sap_param_t*)p3;
    int type;
    int start_x, end_x, start_y, end_y;
    int start_x_r0, end_x_r0, start_x_r, end_x_r, start_x_rn, end_x_rn;
    int x, y;
    pel_t *src, *dst;
    int i_src, i_dst;
    int reg = 0;
    __m128i clipMin = _mm_setzero_si128();

    if (compIdx == SAO_Y) {
        i_src = pic_src->i_stride;
        src = pic_src->p_y + pix_y * i_src + pix_x;
        i_dst = pic_dst->i_stride;
        dst = pic_dst->p_y + pix_y * i_dst + pix_x;
    } else {
        i_src = pic_src->i_stridec;
        src = (compIdx == SAO_Cb) ? pic_src->p_u : pic_src->p_v;
        src += pix_y * i_src + pix_x;
        i_dst = pic_dst->i_stridec;
        dst = (compIdx == SAO_Cb) ? pic_dst->p_u : pic_dst->p_v;
        dst += pix_y * i_dst + pix_x;
    }

    assert(saoBlkParam->modeIdc == SAO_MODE_NEW);
    type = saoBlkParam->typeIdc;

    switch (type) {
    case SAO_TYPE_EO_0: {
        __m128i off0, off1, off2, off3, off4;
        __m128i s0, s1, s2;
        __m128i t0, t1, t2, t3, t4, etype;
        __m128i c0, c1, c2, c3, c4;
        __m128i mask;
        int end_x_16;

        c0 = _mm_set1_epi8(-2);
        c1 = _mm_set1_epi8(-1);
        c2 = _mm_set1_epi8(0);
        c3 = _mm_set1_epi8(1);
        c4 = _mm_set1_epi8(2);

        off0 = _mm_set1_epi8(saoBlkParam->offset[0]);
        off1 = _mm_set1_epi8(saoBlkParam->offset[1]);
        off2 = _mm_set1_epi8(saoBlkParam->offset[2]);
        off3 = _mm_set1_epi8(saoBlkParam->offset[3]);
        off4 = _mm_set1_epi8(saoBlkParam->offset[4]);

        start_x = smb_available_left ? 0 : 1;
        end_x = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
        end_x_16 = end_x - ((end_x - start_x) & 0x0f);

        mask = _mm_load_si128((__m128i*)(intrinsic_mask[end_x - end_x_16 - 1]));
        for (y = 0; y < smb_pix_height; y++) {
            //diff = src[start_x] - src[start_x - 1];
            //leftsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
            for (x = start_x; x < end_x; x += 16) {
                s0 = _mm_loadu_si128((__m128i*)&src[x - 1]);
                s1 = _mm_loadu_si128((__m128i*)&src[x]);
                s2 = _mm_loadu_si128((__m128i*)&src[x + 1]);

                t3 = _mm_min_epu8(s0, s1);
                t1 = _mm_cmpeq_epi8(t3, s0);
                t2 = _mm_cmpeq_epi8(t3, s1);
                t0 = _mm_subs_epi8(t2, t1); //leftsign

                t3 = _mm_min_epu8(s1, s2);
                t1 = _mm_cmpeq_epi8(t3, s1);
                t2 = _mm_cmpeq_epi8(t3, s2);
                t3 = _mm_subs_epi8(t1, t2); //rightsign

                etype = _mm_adds_epi8(t0, t3); //edgetype

                t0 = _mm_cmpeq_epi8(etype, c0);
                t1 = _mm_cmpeq_epi8(etype, c1);
                t2 = _mm_cmpeq_epi8(etype, c2);
                t3 = _mm_cmpeq_epi8(etype, c3);
                t4 = _mm_cmpeq_epi8(etype, c4);

                t0 = _mm_and_si128(t0, off0);
                t1 = _mm_and_si128(t1, off1);
                t2 = _mm_and_si128(t2, off2);
                t3 = _mm_and_si128(t3, off3);
                t4 = _mm_and_si128(t4, off4);

                t0 = _mm_adds_epi8(t0, t1);
                t2 = _mm_adds_epi8(t2, t3);
                t0 = _mm_adds_epi8(t0, t4);
                t0 = _mm_adds_epi8(t0, t2);//get offset

                //add 8 nums once for possible overflow
                t1 = _mm_cvtepi8_epi16(t0);
                t0 = _mm_srli_si128(t0, 8);
                t2 = _mm_cvtepi8_epi16(t0);
                t3 = _mm_unpacklo_epi8(s1, clipMin);
                t4 = _mm_unpackhi_epi8(s1, clipMin);

                t1 = _mm_adds_epi16(t1, t3);
                t2 = _mm_adds_epi16(t2, t4);
                t0 = _mm_packus_epi16(t1, t2); //saturated

                if (x != end_x_16){
                    _mm_storeu_si128((__m128i*)(dst + x), t0);
                }
                else{
                    _mm_maskmoveu_si128(t0, mask, (char_t*)(dst + x));
                    break;
                }
            }
            dst += i_dst;
            src += i_src;
        }

    }
        break;
    case SAO_TYPE_EO_90: {
        __m128i off0, off1, off2, off3, off4;
        __m128i s0, s1, s2;
        __m128i t0, t1, t2, t3, t4, etype;
        __m128i c0, c1, c2, c3, c4;
        __m128i mask = _mm_loadu_si128((__m128i*)(intrinsic_mask[(smb_pix_width & 15) - 1]));
        int end_x_16 = smb_pix_width - 15;

        c0 = _mm_set1_epi8(-2);
        c1 = _mm_set1_epi8(-1);
        c2 = _mm_set1_epi8(0);
        c3 = _mm_set1_epi8(1);
        c4 = _mm_set1_epi8(2);

        off0 = _mm_set1_epi8(saoBlkParam->offset[0]);
        off1 = _mm_set1_epi8(saoBlkParam->offset[1]);
        off2 = _mm_set1_epi8(saoBlkParam->offset[2]);
        off3 = _mm_set1_epi8(saoBlkParam->offset[3]);
        off4 = _mm_set1_epi8(saoBlkParam->offset[4]);

        start_y = smb_available_up ? 0 : 1;
        end_y = smb_available_down ? smb_pix_height : (smb_pix_height - 1);

        dst += start_y * i_dst;
        src += start_y * i_src;

        for (y = start_y; y < end_y; y++) {
            for (x = 0; x < smb_pix_width; x += 16) {
                s0 = _mm_loadu_si128((__m128i*)&src[x - i_src]);
                s1 = _mm_loadu_si128((__m128i*)&src[x]);
                s2 = _mm_loadu_si128((__m128i*)&src[x + i_src]);

                t3 = _mm_min_epu8(s0, s1);
                t1 = _mm_cmpeq_epi8(t3, s0);
                t2 = _mm_cmpeq_epi8(t3, s1);
                t0 = _mm_subs_epi8(t2, t1); //upsign

                t3 = _mm_min_epu8(s1, s2);
                t1 = _mm_cmpeq_epi8(t3, s1);
                t2 = _mm_cmpeq_epi8(t3, s2);
                t3 = _mm_subs_epi8(t1, t2); //downsign

                etype = _mm_adds_epi8(t0, t3); //edgetype

                t0 = _mm_cmpeq_epi8(etype, c0);
                t1 = _mm_cmpeq_epi8(etype, c1);
                t2 = _mm_cmpeq_epi8(etype, c2);
                t3 = _mm_cmpeq_epi8(etype, c3);
                t4 = _mm_cmpeq_epi8(etype, c4);

                t0 = _mm_and_si128(t0, off0);
                t1 = _mm_and_si128(t1, off1);
                t2 = _mm_and_si128(t2, off2);
                t3 = _mm_and_si128(t3, off3);
                t4 = _mm_and_si128(t4, off4);

                t0 = _mm_adds_epi8(t0, t1);
                t2 = _mm_adds_epi8(t2, t3);
                t0 = _mm_adds_epi8(t0, t4);
                t0 = _mm_adds_epi8(t0, t2);//get offset

                //add 8 nums once for possible overflow
                t1 = _mm_cvtepi8_epi16(t0);
                t0 = _mm_srli_si128(t0, 8);
                t2 = _mm_cvtepi8_epi16(t0);
                t3 = _mm_unpacklo_epi8(s1, clipMin);
                t4 = _mm_unpackhi_epi8(s1, clipMin);

                t1 = _mm_adds_epi16(t1, t3);
                t2 = _mm_adds_epi16(t2, t4);
                t0 = _mm_packus_epi16(t1, t2); //saturated

                if (x < end_x_16){
                    _mm_storeu_si128((__m128i*)(dst + x), t0);
                }
                else{
                    _mm_maskmoveu_si128(t0, mask, (char_t*)(dst + x));
                    break;
                }
            }
            dst += i_dst;
            src += i_src;
        }
    }
        break;
    case SAO_TYPE_EO_135: {
        __m128i off0, off1, off2, off3, off4;
        __m128i s0, s1, s2;
        __m128i t0, t1, t2, t3, t4, etype;
        __m128i c0, c1, c2, c3, c4;
        __m128i mask_r0, mask_r, mask_rn;
        int end_x_r0_16, end_x_r_16, end_x_rn_16;

        c0 = _mm_set1_epi8(-2);
        c1 = _mm_set1_epi8(-1);
        c2 = _mm_set1_epi8(0);
        c3 = _mm_set1_epi8(1);
        c4 = _mm_set1_epi8(2);

        off0 = _mm_set1_epi8(saoBlkParam->offset[0]);
        off1 = _mm_set1_epi8(saoBlkParam->offset[1]);
        off2 = _mm_set1_epi8(saoBlkParam->offset[2]);
        off3 = _mm_set1_epi8(saoBlkParam->offset[3]);
        off4 = _mm_set1_epi8(saoBlkParam->offset[4]);

        start_x_r0 = smb_available_upleft ? 0 : 1;
        end_x_r0 = smb_available_up ? (smb_available_right ? smb_pix_width : (smb_pix_width - 1)) : 1;
        start_x_r = smb_available_left ? 0 : 1;
        end_x_r = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
        start_x_rn = smb_available_down ? (smb_available_left ? 0 : 1) : (smb_pix_width - 1);
        end_x_rn = smb_available_rightdwon ? smb_pix_width : (smb_pix_width - 1);

        end_x_r0_16 = end_x_r0 - ((end_x_r0 - start_x_r0) & 0x0f);
        end_x_r_16 = end_x_r - ((end_x_r - start_x_r) & 0x0f);
        end_x_rn_16 = end_x_rn - ((end_x_rn - start_x_rn) & 0x0f);


        //first row
        for (x = start_x_r0; x < end_x_r0; x += 16) {
            s0 = _mm_loadu_si128((__m128i*)&src[x - i_src - 1]);
            s1 = _mm_loadu_si128((__m128i*)&src[x]);
            s2 = _mm_loadu_si128((__m128i*)&src[x + i_src + 1]);

            t3 = _mm_min_epu8(s0, s1);
            t1 = _mm_cmpeq_epi8(t3, s0);
            t2 = _mm_cmpeq_epi8(t3, s1);
            t0 = _mm_subs_epi8(t2, t1); //upsign

            t3 = _mm_min_epu8(s1, s2);
            t1 = _mm_cmpeq_epi8(t3, s1);
            t2 = _mm_cmpeq_epi8(t3, s2);
            t3 = _mm_subs_epi8(t1, t2); //downsign

            etype = _mm_adds_epi8(t0, t3); //edgetype

            t0 = _mm_cmpeq_epi8(etype, c0);
            t1 = _mm_cmpeq_epi8(etype, c1);
            t2 = _mm_cmpeq_epi8(etype, c2);
            t3 = _mm_cmpeq_epi8(etype, c3);
            t4 = _mm_cmpeq_epi8(etype, c4);

            t0 = _mm_and_si128(t0, off0);
            t1 = _mm_and_si128(t1, off1);
            t2 = _mm_and_si128(t2, off2);
            t3 = _mm_and_si128(t3, off3);
            t4 = _mm_and_si128(t4, off4);

            t0 = _mm_adds_epi8(t0, t1);
            t2 = _mm_adds_epi8(t2, t3);
            t0 = _mm_adds_epi8(t0, t4);
            t0 = _mm_adds_epi8(t0, t2);//get offset

            //add 8 nums once for possible overflow
            t1 = _mm_cvtepi8_epi16(t0);
            t0 = _mm_srli_si128(t0, 8);
            t2 = _mm_cvtepi8_epi16(t0);
            t3 = _mm_unpacklo_epi8(s1, clipMin);
            t4 = _mm_unpackhi_epi8(s1, clipMin);

            t1 = _mm_adds_epi16(t1, t3);
            t2 = _mm_adds_epi16(t2, t4);
            t0 = _mm_packus_epi16(t1, t2); //saturated

            if (x != end_x_r0_16){
                _mm_storeu_si128((__m128i*)(dst + x), t0);
            }
            else{
                mask_r0 = _mm_load_si128((__m128i*)(intrinsic_mask[end_x_r0 - end_x_r0_16 - 1]));
                _mm_maskmoveu_si128(t0, mask_r0, (char_t*)(dst + x));
                break;
            }
        }
        dst += i_dst;
        src += i_src;

        mask_r = _mm_load_si128((__m128i*)(intrinsic_mask[end_x_r - end_x_r_16 - 1]));
        //middle rows
        for (y = 1; y < smb_pix_height - 1; y++) {
            for (x = start_x_r; x < end_x_r; x += 16) {
                s0 = _mm_loadu_si128((__m128i*)&src[x - i_src - 1]);
                s1 = _mm_loadu_si128((__m128i*)&src[x]);
                s2 = _mm_loadu_si128((__m128i*)&src[x + i_src + 1]);

                t3 = _mm_min_epu8(s0, s1);
                t1 = _mm_cmpeq_epi8(t3, s0);
                t2 = _mm_cmpeq_epi8(t3, s1);
                t0 = _mm_subs_epi8(t2, t1); //upsign

                t3 = _mm_min_epu8(s1, s2);
                t1 = _mm_cmpeq_epi8(t3, s1);
                t2 = _mm_cmpeq_epi8(t3, s2);
                t3 = _mm_subs_epi8(t1, t2); //downsign

                etype = _mm_adds_epi8(t0, t3); //edgetype

                t0 = _mm_cmpeq_epi8(etype, c0);
                t1 = _mm_cmpeq_epi8(etype, c1);
                t2 = _mm_cmpeq_epi8(etype, c2);
                t3 = _mm_cmpeq_epi8(etype, c3);
                t4 = _mm_cmpeq_epi8(etype, c4);

                t0 = _mm_and_si128(t0, off0);
                t1 = _mm_and_si128(t1, off1);
                t2 = _mm_and_si128(t2, off2);
                t3 = _mm_and_si128(t3, off3);
                t4 = _mm_and_si128(t4, off4);

                t0 = _mm_adds_epi8(t0, t1);
                t2 = _mm_adds_epi8(t2, t3);
                t0 = _mm_adds_epi8(t0, t4);
                t0 = _mm_adds_epi8(t0, t2);//get offset

                //add 8 nums once for possible overflow
                t1 = _mm_cvtepi8_epi16(t0);
                t0 = _mm_srli_si128(t0, 8);
                t2 = _mm_cvtepi8_epi16(t0);
                t3 = _mm_unpacklo_epi8(s1, clipMin);
                t4 = _mm_unpackhi_epi8(s1, clipMin);

                t1 = _mm_adds_epi16(t1, t3);
                t2 = _mm_adds_epi16(t2, t4);
                t0 = _mm_packus_epi16(t1, t2); //saturated

                if (x != end_x_r_16){
                    _mm_storeu_si128((__m128i*)(dst + x), t0);
                }
                else{
                    _mm_maskmoveu_si128(t0, mask_r, (char_t*)(dst + x));
                    break;
                }
            }
            dst += i_dst;
            src += i_src;
        }
        //last row
        for (x = start_x_rn; x < end_x_rn; x += 16) {
            s0 = _mm_loadu_si128((__m128i*)&src[x - i_src - 1]);
            s1 = _mm_loadu_si128((__m128i*)&src[x]);
            s2 = _mm_loadu_si128((__m128i*)&src[x + i_src + 1]);

            t3 = _mm_min_epu8(s0, s1);
            t1 = _mm_cmpeq_epi8(t3, s0);
            t2 = _mm_cmpeq_epi8(t3, s1);
            t0 = _mm_subs_epi8(t2, t1); //upsign

            t3 = _mm_min_epu8(s1, s2);
            t1 = _mm_cmpeq_epi8(t3, s1);
            t2 = _mm_cmpeq_epi8(t3, s2);
            t3 = _mm_subs_epi8(t1, t2); //downsign

            etype = _mm_adds_epi8(t0, t3); //edgetype

            t0 = _mm_cmpeq_epi8(etype, c0);
            t1 = _mm_cmpeq_epi8(etype, c1);
            t2 = _mm_cmpeq_epi8(etype, c2);
            t3 = _mm_cmpeq_epi8(etype, c3);
            t4 = _mm_cmpeq_epi8(etype, c4);

            t0 = _mm_and_si128(t0, off0);
            t1 = _mm_and_si128(t1, off1);
            t2 = _mm_and_si128(t2, off2);
            t3 = _mm_and_si128(t3, off3);
            t4 = _mm_and_si128(t4, off4);

            t0 = _mm_adds_epi8(t0, t1);
            t2 = _mm_adds_epi8(t2, t3);
            t0 = _mm_adds_epi8(t0, t4);
            t0 = _mm_adds_epi8(t0, t2);//get offset

            //add 8 nums once for possible overflow
            t1 = _mm_cvtepi8_epi16(t0);
            t0 = _mm_srli_si128(t0, 8);
            t2 = _mm_cvtepi8_epi16(t0);
            t3 = _mm_unpacklo_epi8(s1, clipMin);
            t4 = _mm_unpackhi_epi8(s1, clipMin);

            t1 = _mm_adds_epi16(t1, t3);
            t2 = _mm_adds_epi16(t2, t4);
            t0 = _mm_packus_epi16(t1, t2); //saturated

            if (x != end_x_rn_16){
                _mm_storeu_si128((__m128i*)(dst + x), t0);
            }
            else{
                mask_rn = _mm_load_si128((__m128i*)(intrinsic_mask[end_x_rn - end_x_rn_16 - 1]));
                _mm_maskmoveu_si128(t0, mask_rn, (char_t*)(dst + x));
                break;
            }
        }
    }
        break;
    case SAO_TYPE_EO_45: {
        __m128i off0, off1, off2, off3, off4;
        __m128i s0, s1, s2;
        __m128i t0, t1, t2, t3, t4, etype;
        __m128i c0, c1, c2, c3, c4;
        __m128i mask_r0, mask_r, mask_rn;
        int end_x_r0_16, end_x_r_16, end_x_rn_16;

        c0 = _mm_set1_epi8(-2);
        c1 = _mm_set1_epi8(-1);
        c2 = _mm_set1_epi8(0);
        c3 = _mm_set1_epi8(1);
        c4 = _mm_set1_epi8(2);

        off0 = _mm_set1_epi8(saoBlkParam->offset[0]);
        off1 = _mm_set1_epi8(saoBlkParam->offset[1]);
        off2 = _mm_set1_epi8(saoBlkParam->offset[2]);
        off3 = _mm_set1_epi8(saoBlkParam->offset[3]);
        off4 = _mm_set1_epi8(saoBlkParam->offset[4]);

        start_x_r0 = smb_available_up ? (smb_available_left ? 0 : 1) : (smb_pix_width - 1);
        end_x_r0 = smb_available_upright ? smb_pix_width : (smb_pix_width - 1);
        start_x_r = smb_available_left ? 0 : 1;
        end_x_r = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
        start_x_rn = smb_available_leftdown ? 0 : 1;
        end_x_rn = smb_available_down ? (smb_available_right ? smb_pix_width : (smb_pix_width - 1)) : 1;

        end_x_r0_16 = end_x_r0 - ((end_x_r0 - start_x_r0) & 0x0f);
        end_x_r_16 = end_x_r - ((end_x_r - start_x_r) & 0x0f);
        end_x_rn_16 = end_x_rn - ((end_x_rn - start_x_rn) & 0x0f);


        //first row
        for (x = start_x_r0; x < end_x_r0; x += 16) {
            s0 = _mm_loadu_si128((__m128i*)&src[x - i_src + 1]);
            s1 = _mm_loadu_si128((__m128i*)&src[x]);
            s2 = _mm_loadu_si128((__m128i*)&src[x + i_src - 1]);

            t3 = _mm_min_epu8(s0, s1);
            t1 = _mm_cmpeq_epi8(t3, s0);
            t2 = _mm_cmpeq_epi8(t3, s1);
            t0 = _mm_subs_epi8(t2, t1); //upsign

            t3 = _mm_min_epu8(s1, s2);
            t1 = _mm_cmpeq_epi8(t3, s1);
            t2 = _mm_cmpeq_epi8(t3, s2);
            t3 = _mm_subs_epi8(t1, t2); //downsign

            etype = _mm_adds_epi8(t0, t3); //edgetype

            t0 = _mm_cmpeq_epi8(etype, c0);
            t1 = _mm_cmpeq_epi8(etype, c1);
            t2 = _mm_cmpeq_epi8(etype, c2);
            t3 = _mm_cmpeq_epi8(etype, c3);
            t4 = _mm_cmpeq_epi8(etype, c4);

            t0 = _mm_and_si128(t0, off0);
            t1 = _mm_and_si128(t1, off1);
            t2 = _mm_and_si128(t2, off2);
            t3 = _mm_and_si128(t3, off3);
            t4 = _mm_and_si128(t4, off4);

            t0 = _mm_adds_epi8(t0, t1);
            t2 = _mm_adds_epi8(t2, t3);
            t0 = _mm_adds_epi8(t0, t4);
            t0 = _mm_adds_epi8(t0, t2);//get offset

            //add 8 nums once for possible overflow
            t1 = _mm_cvtepi8_epi16(t0);
            t0 = _mm_srli_si128(t0, 8);
            t2 = _mm_cvtepi8_epi16(t0);
            t3 = _mm_unpacklo_epi8(s1, clipMin);
            t4 = _mm_unpackhi_epi8(s1, clipMin);

            t1 = _mm_adds_epi16(t1, t3);
            t2 = _mm_adds_epi16(t2, t4);
            t0 = _mm_packus_epi16(t1, t2); //saturated

            if (x != end_x_r0_16){
                _mm_storeu_si128((__m128i*)(dst + x), t0);
            }
            else{
                mask_r0 = _mm_load_si128((__m128i*)(intrinsic_mask[end_x_r0 - end_x_r0_16 - 1]));
                _mm_maskmoveu_si128(t0, mask_r0, (char_t*)(dst + x));
                break;
            }
        }
        dst += i_dst;
        src += i_src;

        mask_r = _mm_load_si128((__m128i*)(intrinsic_mask[end_x_r - end_x_r_16 - 1]));
        //middle rows
        for (y = 1; y < smb_pix_height - 1; y++) {
            for (x = start_x_r; x < end_x_r; x += 16) {
                s0 = _mm_loadu_si128((__m128i*)&src[x - i_src + 1]);
                s1 = _mm_loadu_si128((__m128i*)&src[x]);
                s2 = _mm_loadu_si128((__m128i*)&src[x + i_src - 1]);

                t3 = _mm_min_epu8(s0, s1);
                t1 = _mm_cmpeq_epi8(t3, s0);
                t2 = _mm_cmpeq_epi8(t3, s1);
                t0 = _mm_subs_epi8(t2, t1); //upsign

                t3 = _mm_min_epu8(s1, s2);
                t1 = _mm_cmpeq_epi8(t3, s1);
                t2 = _mm_cmpeq_epi8(t3, s2);
                t3 = _mm_subs_epi8(t1, t2); //downsign

                etype = _mm_adds_epi8(t0, t3); //edgetype

                t0 = _mm_cmpeq_epi8(etype, c0);
                t1 = _mm_cmpeq_epi8(etype, c1);
                t2 = _mm_cmpeq_epi8(etype, c2);
                t3 = _mm_cmpeq_epi8(etype, c3);
                t4 = _mm_cmpeq_epi8(etype, c4);

                t0 = _mm_and_si128(t0, off0);
                t1 = _mm_and_si128(t1, off1);
                t2 = _mm_and_si128(t2, off2);
                t3 = _mm_and_si128(t3, off3);
                t4 = _mm_and_si128(t4, off4);

                t0 = _mm_adds_epi8(t0, t1);
                t2 = _mm_adds_epi8(t2, t3);
                t0 = _mm_adds_epi8(t0, t4);
                t0 = _mm_adds_epi8(t0, t2);//get offset

                //add 8 nums once for possible overflow
                t1 = _mm_cvtepi8_epi16(t0);
                t0 = _mm_srli_si128(t0, 8);
                t2 = _mm_cvtepi8_epi16(t0);
                t3 = _mm_unpacklo_epi8(s1, clipMin);
                t4 = _mm_unpackhi_epi8(s1, clipMin);

                t1 = _mm_adds_epi16(t1, t3);
                t2 = _mm_adds_epi16(t2, t4);
                t0 = _mm_packus_epi16(t1, t2); //saturated

                if (x != end_x_r_16){
                    _mm_storeu_si128((__m128i*)(dst + x), t0);
                }
                else{
                    _mm_maskmoveu_si128(t0, mask_r, (char_t*)(dst + x));
                    break;
                }
            }
            dst += i_dst;
            src += i_src;
        }
        for (x = start_x_rn; x < end_x_rn; x += 16) {
            s0 = _mm_loadu_si128((__m128i*)&src[x - i_src + 1]);
            s1 = _mm_loadu_si128((__m128i*)&src[x]);
            s2 = _mm_loadu_si128((__m128i*)&src[x + i_src - 1]);

            t3 = _mm_min_epu8(s0, s1);
            t1 = _mm_cmpeq_epi8(t3, s0);
            t2 = _mm_cmpeq_epi8(t3, s1);
            t0 = _mm_subs_epi8(t2, t1); //upsign

            t3 = _mm_min_epu8(s1, s2);
            t1 = _mm_cmpeq_epi8(t3, s1);
            t2 = _mm_cmpeq_epi8(t3, s2);
            t3 = _mm_subs_epi8(t1, t2); //downsign

            etype = _mm_adds_epi8(t0, t3); //edgetype

            t0 = _mm_cmpeq_epi8(etype, c0);
            t1 = _mm_cmpeq_epi8(etype, c1);
            t2 = _mm_cmpeq_epi8(etype, c2);
            t3 = _mm_cmpeq_epi8(etype, c3);
            t4 = _mm_cmpeq_epi8(etype, c4);

            t0 = _mm_and_si128(t0, off0);
            t1 = _mm_and_si128(t1, off1);
            t2 = _mm_and_si128(t2, off2);
            t3 = _mm_and_si128(t3, off3);
            t4 = _mm_and_si128(t4, off4);

            t0 = _mm_adds_epi8(t0, t1);
            t2 = _mm_adds_epi8(t2, t3);
            t0 = _mm_adds_epi8(t0, t4);
            t0 = _mm_adds_epi8(t0, t2);//get offset

            //add 8 nums once for possible overflow
            t1 = _mm_cvtepi8_epi16(t0);
            t0 = _mm_srli_si128(t0, 8);
            t2 = _mm_cvtepi8_epi16(t0);
            t3 = _mm_unpacklo_epi8(s1, clipMin);
            t4 = _mm_unpackhi_epi8(s1, clipMin);

            t1 = _mm_adds_epi16(t1, t3);
            t2 = _mm_adds_epi16(t2, t4);
            t0 = _mm_packus_epi16(t1, t2); //saturated

            if (x != end_x_rn_16){
                _mm_storeu_si128((__m128i*)(dst + x), t0);
            }
            else{
                mask_rn = _mm_load_si128((__m128i*)(intrinsic_mask[end_x_rn - end_x_rn_16 - 1]));
                _mm_maskmoveu_si128(t0, mask_rn, (char_t*)(dst + x));
                break;
            }
        }
    }
        break;
    case SAO_TYPE_BO: {
        __m128i r0, r1, r2, r3, off0, off1, off2, off3;
        __m128i t0, t1, t2, t3, t4, src0, src1;
        __m128i mask = _mm_load_si128((const __m128i*)intrinsic_mask[(smb_pix_width & 15) - 1]);
        __m128i shift_mask = _mm_set1_epi8(31);
        int shift_bo = sample_bit_depth - NUM_SAO_BO_CLASSES_IN_BIT;
        int end_x_16 = smb_pix_width - 15;
        
        r0 = _mm_set1_epi8(saoBlkParam->startBand);
        r1 = _mm_set1_epi8((saoBlkParam->startBand + 1)%32);
        r2 = _mm_set1_epi8(saoBlkParam->startBand2);
        r3 = _mm_set1_epi8((saoBlkParam->startBand2 + 1)%32);
        off0 = _mm_set1_epi8(saoBlkParam->offset[0]);
        off1 = _mm_set1_epi8(saoBlkParam->offset[1]);
        off2 = _mm_set1_epi8(saoBlkParam->offset[2]);
        off3 = _mm_set1_epi8(saoBlkParam->offset[3]);

        for (y = 0; y < smb_pix_height; y++) {
            for (x = 0; x < smb_pix_width; x += 16) {
                src0 = _mm_loadu_si128((__m128i*)&src[x]);
                src1 = _mm_and_si128(_mm_srai_epi16(src0, 3), shift_mask);

                t0 = _mm_cmpeq_epi8(src1, r0);
                t1 = _mm_cmpeq_epi8(src1, r1);
                t2 = _mm_cmpeq_epi8(src1, r2);
                t3 = _mm_cmpeq_epi8(src1, r3);

                t0 = _mm_and_si128(t0, off0);
                t1 = _mm_and_si128(t1, off1);
                t2 = _mm_and_si128(t2, off2);
                t3 = _mm_and_si128(t3, off3);
                t0 = _mm_or_si128(t0, t1);
                t2 = _mm_or_si128(t2, t3);
                t0 = _mm_or_si128(t0, t2);
                //src0 = _mm_adds_epi8(src0, t0);
                //add 8 nums once for possible overflow
                t1 = _mm_cvtepi8_epi16(t0);
                t0 = _mm_srli_si128(t0, 8);
                t2 = _mm_cvtepi8_epi16(t0);
                t3 = _mm_unpacklo_epi8(src0, clipMin);
                t4 = _mm_unpackhi_epi8(src0, clipMin);

                t1 = _mm_adds_epi16(t1, t3);
                t2 = _mm_adds_epi16(t2, t4);
                src0 = _mm_packus_epi16(t1, t2); //saturated
                
                if (x < end_x_16) {
                    _mm_storeu_si128((__m128i*)&dst[x], src0);
                }
                else {
                    _mm_maskmoveu_si128(src0, mask, (char_t*)(dst + x));
                }

            }
            /* // for width == 4
            src0 = _mm_loadu_si128((__m128i*)&src[x]);
            src1 = _mm_srai_epi8(src0, shift_bo);

            t0 = _mm_cmpeq_epi8(src1, r0);
            t1 = _mm_cmpeq_epi8(src1, r1);
            t2 = _mm_cmpeq_epi8(src1, r2);
            t3 = _mm_cmpeq_epi8(src1, r3);

            t0 = _mm_and_si128(t0, off0);
            t1 = _mm_and_si128(t1, off1);
            t2 = _mm_and_si128(t2, off2);
            t3 = _mm_and_si128(t3, off3);
            t0 = _mm_or_si128(t0, t1);
            t2 = _mm_or_si128(t2, t3);
            t0 = _mm_or_si128(t0, t2);
            //add 8 nums once for possible overflow
            t1 = _mm_cvtepi8_epi16(t0);
            t0 = _mm_srli_si128(t0, 8);
            t2 = _mm_cvtepi8_epi16(t0);
            t3 = _mm_unpacklo_epi8(src0, clipMin);
            t4 = _mm_unpackhi_epi8(src0, clipMin);

            t1 = _mm_adds_epi16(t1, t3);
            t2 = _mm_adds_epi16(t2, t4);
            src0 = _mm_packus_epi16(t1, t2); //saturated
            */

            dst += i_dst;
            src += i_src;
        }

    }
        break;
    default: {
        printf("Not a supported SAO types\n");
        assert(0);
        exit(-1);
    }
    }
}

void SAO_on_block_sse128_10bit(void *p1, void *p2, void *p3, int compIdx, int smb_index, int pix_y, int pix_x, int smb_pix_height,
    int smb_pix_width, int smb_available_left, int smb_available_right, int smb_available_up, int smb_available_down,
    int smb_available_upleft, int smb_available_upright, int smb_available_leftdown, int smb_available_rightdwon, int sample_bit_depth)
{
    com_pic_t *pic_src = (com_pic_t*)p1;
    com_pic_t *pic_dst = (com_pic_t*)p2;
    sap_param_t *saoBlkParam = (sap_param_t*)p3;
    int type;
    int start_x, end_x, start_y, end_y;
    int start_x_r0, end_x_r0, start_x_r, end_x_r, start_x_rn, end_x_rn;
    int x, y;
    pel_t *src, *dst;
    int i_src, i_dst;
    int max_pixel = (1 << sample_bit_depth) - 1;
    __m128i off0, off1, off2, off3, off4;
    __m128i s0, s1, s2;
    __m128i t0, t1, t2, t3, t4, etype;
    __m128i c0, c1, c2, c3, c4;
    __m128i mask;
    __m128i min_val = _mm_setzero_si128();
    __m128i max_val = _mm_set1_epi16(max_pixel);

    if (compIdx == SAO_Y) {
        i_src = pic_src->i_stride;
        src   = pic_src->p_y + pix_y * i_src + pix_x;
        i_dst = pic_dst->i_stride;
        dst   = pic_dst->p_y + pix_y * i_dst + pix_x;
    } else {
        i_src = pic_src->i_stridec;
        src   = (compIdx == SAO_Cb) ? pic_src->p_u : pic_src->p_v;
        src  += pix_y * i_src + pix_x;
        i_dst = pic_dst->i_stridec;
        dst   = (compIdx == SAO_Cb) ? pic_dst->p_u : pic_dst->p_v;
        dst  += pix_y * i_dst + pix_x;
    }

    type = saoBlkParam->typeIdc;

    switch (type) {
    case SAO_TYPE_EO_0: {
                            int end_x_8;
                            c0 = _mm_set1_epi16(-2);
                            c1 = _mm_set1_epi16(-1);
                            c2 = _mm_set1_epi16(0);
                            c3 = _mm_set1_epi16(1);
                            c4 = _mm_set1_epi16(2);

                            off0 = _mm_set1_epi16((pel_t)saoBlkParam->offset[0]);
                            off1 = _mm_set1_epi16((pel_t)saoBlkParam->offset[1]);
                            off2 = _mm_set1_epi16((pel_t)saoBlkParam->offset[2]);
                            off3 = _mm_set1_epi16((pel_t)saoBlkParam->offset[3]);
                            off4 = _mm_set1_epi16((pel_t)saoBlkParam->offset[4]);
                            start_x = smb_available_left ? 0 : 1;
                            end_x = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
                            end_x_8 = end_x - ((end_x - start_x) & 0x07);

                            mask = _mm_load_si128((__m128i*)(intrinsic_mask_10bit[end_x - end_x_8 - 1]));
                            if (smb_pix_width == 4){
                                

                                for (y = 0; y < smb_pix_height; y++) {
                                    //diff = src[start_x] - src[start_x - 1];
                                    //leftsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
                                    s0 = _mm_loadu_si128((__m128i*)&src[start_x - 1]);
                                    s1 = _mm_srli_si128(s0, 2);
                                    s2 = _mm_srli_si128(s0, 4);

                                    t3 = _mm_min_epu16(s0, s1);
                                    t1 = _mm_cmpeq_epi16(t3, s0);
                                    t2 = _mm_cmpeq_epi16(t3, s1);
                                    t0 = _mm_subs_epi16(t2, t1); //leftsign

                                    t3 = _mm_min_epu16(s1, s2);
                                    t1 = _mm_cmpeq_epi16(t3, s1);
                                    t2 = _mm_cmpeq_epi16(t3, s2);
                                    t3 = _mm_subs_epi16(t1, t2); //rightsign

                                    etype = _mm_adds_epi16(t0, t3); //edgetype

                                    t0 = _mm_cmpeq_epi16(etype, c0);
                                    t1 = _mm_cmpeq_epi16(etype, c1);
                                    t2 = _mm_cmpeq_epi16(etype, c2);
                                    t3 = _mm_cmpeq_epi16(etype, c3);
                                    t4 = _mm_cmpeq_epi16(etype, c4);

                                    t0 = _mm_and_si128(t0, off0);
                                    t1 = _mm_and_si128(t1, off1);
                                    t2 = _mm_and_si128(t2, off2);
                                    t3 = _mm_and_si128(t3, off3);
                                    t4 = _mm_and_si128(t4, off4);

                                    t0 = _mm_adds_epi16(t0, t1);
                                    t2 = _mm_adds_epi16(t2, t3);
                                    t0 = _mm_adds_epi16(t0, t4);
                                    t0 = _mm_adds_epi16(t0, t2);//get offset

                                    t1 = _mm_adds_epi16(t0, s1);
                                    t1 = _mm_min_epi16(t1, max_val);
                                    t1 = _mm_max_epi16(t1, min_val);
                                    _mm_maskmoveu_si128(t1, mask, (char_t*)(dst));

                                    dst += i_dst;
                                    src += i_src;
                                }
                            }
                            else{

                                for (y = 0; y < smb_pix_height; y++) {
                                    //diff = src[start_x] - src[start_x - 1];
                                    //leftsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
                                    for (x = start_x; x < end_x; x += 8) {
                                        s0 = _mm_loadu_si128((__m128i*)&src[x - 1]);
                                        s1 = _mm_loadu_si128((__m128i*)&src[x]);
                                        s2 = _mm_loadu_si128((__m128i*)&src[x + 1]);

                                        t3 = _mm_min_epu16(s0, s1);
                                        t1 = _mm_cmpeq_epi16(t3, s0);
                                        t2 = _mm_cmpeq_epi16(t3, s1);
                                        t0 = _mm_subs_epi16(t2, t1); //leftsign

                                        t3 = _mm_min_epu16(s1, s2);
                                        t1 = _mm_cmpeq_epi16(t3, s1);
                                        t2 = _mm_cmpeq_epi16(t3, s2);
                                        t3 = _mm_subs_epi16(t1, t2); //rightsign

                                        etype = _mm_adds_epi16(t0, t3); //edgetype

                                        t0 = _mm_cmpeq_epi16(etype, c0);
                                        t1 = _mm_cmpeq_epi16(etype, c1);
                                        t2 = _mm_cmpeq_epi16(etype, c2);
                                        t3 = _mm_cmpeq_epi16(etype, c3);
                                        t4 = _mm_cmpeq_epi16(etype, c4);

                                        t0 = _mm_and_si128(t0, off0);
                                        t1 = _mm_and_si128(t1, off1);
                                        t2 = _mm_and_si128(t2, off2);
                                        t3 = _mm_and_si128(t3, off3);
                                        t4 = _mm_and_si128(t4, off4);

                                        t0 = _mm_adds_epi16(t0, t1);
                                        t2 = _mm_adds_epi16(t2, t3);
                                        t0 = _mm_adds_epi16(t0, t4);
                                        t0 = _mm_adds_epi16(t0, t2);//get offset

                                        t1 = _mm_adds_epi16(t0, s1);
                                        t1 = _mm_min_epi16(t1, max_val);
                                        t1 = _mm_max_epi16(t1, min_val);

                                        if (x != end_x_8){
                                            _mm_storeu_si128((__m128i*)(dst + x), t1);
                                        }
                                        else{
                                            _mm_maskmoveu_si128(t1, mask, (char_t*)(dst + x));
                                            break;
                                        }
                                    }
                                    dst += i_dst;
                                    src += i_src;
                                }
                            }
    }
        break;
    case SAO_TYPE_EO_90: {
                             int end_x_8 = smb_pix_width - 7;
                             c0 = _mm_set1_epi16(-2);
                             c1 = _mm_set1_epi16(-1);
                             c2 = _mm_set1_epi16(0);
                             c3 = _mm_set1_epi16(1);
                             c4 = _mm_set1_epi16(2);

                             off0 = _mm_set1_epi16((pel_t)saoBlkParam->offset[0]);
                             off1 = _mm_set1_epi16((pel_t)saoBlkParam->offset[1]);
                             off2 = _mm_set1_epi16((pel_t)saoBlkParam->offset[2]);
                             off3 = _mm_set1_epi16((pel_t)saoBlkParam->offset[3]);
                             off4 = _mm_set1_epi16((pel_t)saoBlkParam->offset[4]);
                             start_y = smb_available_up ? 0 : 1;
                             end_y = smb_available_down ? smb_pix_height : (smb_pix_height - 1);

                             dst += start_y * i_dst;
                             src += start_y * i_src;

                             if (smb_pix_width == 4){
                                 mask = _mm_set_epi32(0, 0, -1, -1);

                                 for (y = start_y; y < end_y; y++) {
                                     s0 = _mm_loadu_si128((__m128i*)(src - i_src));
                                     s1 = _mm_loadu_si128((__m128i*)src);
                                     s2 = _mm_loadu_si128((__m128i*)(src + i_src));

                                     t3 = _mm_min_epu16(s0, s1);
                                     t1 = _mm_cmpeq_epi16(t3, s0);
                                     t2 = _mm_cmpeq_epi16(t3, s1);
                                     t0 = _mm_subs_epi16(t2, t1); //upsign

                                     t3 = _mm_min_epu16(s1, s2);
                                     t1 = _mm_cmpeq_epi16(t3, s1);
                                     t2 = _mm_cmpeq_epi16(t3, s2);
                                     t3 = _mm_subs_epi16(t1, t2); //downsign

                                     etype = _mm_adds_epi16(t0, t3); //edgetype

                                     t0 = _mm_cmpeq_epi16(etype, c0);
                                     t1 = _mm_cmpeq_epi16(etype, c1);
                                     t2 = _mm_cmpeq_epi16(etype, c2);
                                     t3 = _mm_cmpeq_epi16(etype, c3);
                                     t4 = _mm_cmpeq_epi16(etype, c4);

                                     t0 = _mm_and_si128(t0, off0);
                                     t1 = _mm_and_si128(t1, off1);
                                     t2 = _mm_and_si128(t2, off2);
                                     t3 = _mm_and_si128(t3, off3);
                                     t4 = _mm_and_si128(t4, off4);

                                     t0 = _mm_adds_epi16(t0, t1);
                                     t2 = _mm_adds_epi16(t2, t3);
                                     t0 = _mm_adds_epi16(t0, t4);
                                     t0 = _mm_adds_epi16(t0, t2);//get offset

                                     //add 8 nums once for possible overflow
                                     t1 = _mm_adds_epi16(t0, s1);
                                     t1 = _mm_min_epi16(t1, max_val);
                                     t1 = _mm_max_epi16(t1, min_val);

                                     _mm_maskmoveu_si128(t1, mask, (char_t*)(dst));

                                     dst += i_dst;
                                     src += i_src;
                                 }
                             }
                             else{
                                 if (smb_pix_width & 0x07){
                                     mask = _mm_set_epi32(0, 0, -1, -1);

                                     for (y = start_y; y < end_y; y++) {
                                         for (x = 0; x < smb_pix_width; x += 8) {
                                             s0 = _mm_loadu_si128((__m128i*)&src[x - i_src]);
                                             s1 = _mm_loadu_si128((__m128i*)&src[x]);
                                             s2 = _mm_loadu_si128((__m128i*)&src[x + i_src]);

                                             t3 = _mm_min_epu16(s0, s1);
                                             t1 = _mm_cmpeq_epi16(t3, s0);
                                             t2 = _mm_cmpeq_epi16(t3, s1);
                                             t0 = _mm_subs_epi16(t2, t1); //upsign

                                             t3 = _mm_min_epu16(s1, s2);
                                             t1 = _mm_cmpeq_epi16(t3, s1);
                                             t2 = _mm_cmpeq_epi16(t3, s2);
                                             t3 = _mm_subs_epi16(t1, t2); //downsign

                                             etype = _mm_adds_epi16(t0, t3); //edgetype

                                             t0 = _mm_cmpeq_epi16(etype, c0);
                                             t1 = _mm_cmpeq_epi16(etype, c1);
                                             t2 = _mm_cmpeq_epi16(etype, c2);
                                             t3 = _mm_cmpeq_epi16(etype, c3);
                                             t4 = _mm_cmpeq_epi16(etype, c4);

                                             t0 = _mm_and_si128(t0, off0);
                                             t1 = _mm_and_si128(t1, off1);
                                             t2 = _mm_and_si128(t2, off2);
                                             t3 = _mm_and_si128(t3, off3);
                                             t4 = _mm_and_si128(t4, off4);

                                             t0 = _mm_adds_epi16(t0, t1);
                                             t2 = _mm_adds_epi16(t2, t3);
                                             t0 = _mm_adds_epi16(t0, t4);
                                             t0 = _mm_adds_epi16(t0, t2);//get offset

                                             t1 = _mm_adds_epi16(t0, s1);
                                             t1 = _mm_min_epi16(t1, max_val);
                                             t1 = _mm_max_epi16(t1, min_val);

                                             if (x < end_x_8){
                                                 _mm_storeu_si128((__m128i*)(dst + x), t1);
                                             }
                                             else{
                                                 _mm_maskmoveu_si128(t1, mask, (char_t*)(dst + x));
                                                 break;
                                             }
                                         }
                                         dst += i_dst;
                                         src += i_src;
                                     }
                                 }
                                 else{
                                     for (y = start_y; y < end_y; y++) {
                                         for (x = 0; x < smb_pix_width; x += 8) {
                                             s0 = _mm_loadu_si128((__m128i*)&src[x - i_src]);
                                             s1 = _mm_loadu_si128((__m128i*)&src[x]);
                                             s2 = _mm_loadu_si128((__m128i*)&src[x + i_src]);

                                             t3 = _mm_min_epu16(s0, s1);
                                             t1 = _mm_cmpeq_epi16(t3, s0);
                                             t2 = _mm_cmpeq_epi16(t3, s1);
                                             t0 = _mm_subs_epi16(t2, t1); //upsign

                                             t3 = _mm_min_epu16(s1, s2);
                                             t1 = _mm_cmpeq_epi16(t3, s1);
                                             t2 = _mm_cmpeq_epi16(t3, s2);
                                             t3 = _mm_subs_epi16(t1, t2); //downsign

                                             etype = _mm_adds_epi16(t0, t3); //edgetype

                                             t0 = _mm_cmpeq_epi16(etype, c0);
                                             t1 = _mm_cmpeq_epi16(etype, c1);
                                             t2 = _mm_cmpeq_epi16(etype, c2);
                                             t3 = _mm_cmpeq_epi16(etype, c3);
                                             t4 = _mm_cmpeq_epi16(etype, c4);

                                             t0 = _mm_and_si128(t0, off0);
                                             t1 = _mm_and_si128(t1, off1);
                                             t2 = _mm_and_si128(t2, off2);
                                             t3 = _mm_and_si128(t3, off3);
                                             t4 = _mm_and_si128(t4, off4);

                                             t0 = _mm_adds_epi16(t0, t1);
                                             t2 = _mm_adds_epi16(t2, t3);
                                             t0 = _mm_adds_epi16(t0, t4);
                                             t0 = _mm_adds_epi16(t0, t2);//get offset

                                             t1 = _mm_adds_epi16(t0, s1);
                                             t1 = _mm_min_epi16(t1, max_val);
                                             t1 = _mm_max_epi16(t1, min_val);

                                             _mm_storeu_si128((__m128i*)(dst + x), t1);
                                         }
                                         dst += i_dst;
                                         src += i_src;
                                     }
                                 }
                             }
    }
        break;
    case SAO_TYPE_EO_135: {
                              __m128i mask_r0, mask_r, mask_rn;
                              int end_x_r0_8, end_x_r_8, end_x_rn_8;

                              c0 = _mm_set1_epi16(-2);
                              c1 = _mm_set1_epi16(-1);
                              c2 = _mm_set1_epi16(0);
                              c3 = _mm_set1_epi16(1);
                              c4 = _mm_set1_epi16(2);

                              off0 = _mm_set1_epi16((pel_t)saoBlkParam->offset[0]);
                              off1 = _mm_set1_epi16((pel_t)saoBlkParam->offset[1]);
                              off2 = _mm_set1_epi16((pel_t)saoBlkParam->offset[2]);
                              off3 = _mm_set1_epi16((pel_t)saoBlkParam->offset[3]);
                              off4 = _mm_set1_epi16((pel_t)saoBlkParam->offset[4]);

                              start_x_r0 = smb_available_upleft ? 0 : 1;
                              end_x_r0 = smb_available_up ? (smb_available_right ? smb_pix_width : (smb_pix_width - 1)) : 1;
                              start_x_r = smb_available_left ? 0 : 1;
                              end_x_r = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
                              start_x_rn = smb_available_down ? (smb_available_left ? 0 : 1) : (smb_pix_width - 1);
                              end_x_rn = smb_available_rightdwon ? smb_pix_width : (smb_pix_width - 1);

                              end_x_r0_8 = end_x_r0 - ((end_x_r0 - start_x_r0) & 0x07);
                              end_x_r_8 = end_x_r - ((end_x_r - start_x_r) & 0x07);
                              end_x_rn_8 = end_x_rn - ((end_x_rn - start_x_rn) & 0x07);


                              //first row
                              for (x = start_x_r0; x < end_x_r0; x += 8) {
                                  s0 = _mm_loadu_si128((__m128i*)&src[x - i_src - 1]);
                                  s1 = _mm_loadu_si128((__m128i*)&src[x]);
                                  s2 = _mm_loadu_si128((__m128i*)&src[x + i_src + 1]);

                                  t3 = _mm_min_epu16(s0, s1);
                                  t1 = _mm_cmpeq_epi16(t3, s0);
                                  t2 = _mm_cmpeq_epi16(t3, s1);
                                  t0 = _mm_subs_epi16(t2, t1); //upsign

                                  t3 = _mm_min_epu16(s1, s2);
                                  t1 = _mm_cmpeq_epi16(t3, s1);
                                  t2 = _mm_cmpeq_epi16(t3, s2);
                                  t3 = _mm_subs_epi16(t1, t2); //downsign

                                  etype = _mm_adds_epi16(t0, t3); //edgetype

                                  t0 = _mm_cmpeq_epi16(etype, c0);
                                  t1 = _mm_cmpeq_epi16(etype, c1);
                                  t2 = _mm_cmpeq_epi16(etype, c2);
                                  t3 = _mm_cmpeq_epi16(etype, c3);
                                  t4 = _mm_cmpeq_epi16(etype, c4);

                                  t0 = _mm_and_si128(t0, off0);
                                  t1 = _mm_and_si128(t1, off1);
                                  t2 = _mm_and_si128(t2, off2);
                                  t3 = _mm_and_si128(t3, off3);
                                  t4 = _mm_and_si128(t4, off4);

                                  t0 = _mm_adds_epi16(t0, t1);
                                  t2 = _mm_adds_epi16(t2, t3);
                                  t0 = _mm_adds_epi16(t0, t4);
                                  t0 = _mm_adds_epi16(t0, t2);//get offset


                                  t1 = _mm_adds_epi16(t0, s1);
                                  t1 = _mm_min_epi16(t1, max_val);
                                  t1 = _mm_max_epi16(t1, min_val);

                                  if (x != end_x_r0_8){
                                      _mm_storeu_si128((__m128i*)(dst + x), t1);
                                  }
                                  else{
                                      mask_r0 = _mm_load_si128((__m128i*)(intrinsic_mask_10bit[end_x_r0 - end_x_r0_8 - 1]));
                                      _mm_maskmoveu_si128(t1, mask_r0, (char_t*)(dst + x));
                                      break;
                                  }
                              }
                              dst += i_dst;
                              src += i_src;

                              mask_r = _mm_load_si128((__m128i*)(intrinsic_mask_10bit[end_x_r - end_x_r_8 - 1]));
                              //middle rows
                              for (y = 1; y < smb_pix_height - 1; y++) {
                                  for (x = start_x_r; x < end_x_r; x += 8) {
                                      s0 = _mm_loadu_si128((__m128i*)&src[x - i_src - 1]);
                                      s1 = _mm_loadu_si128((__m128i*)&src[x]);
                                      s2 = _mm_loadu_si128((__m128i*)&src[x + i_src + 1]);

                                      t3 = _mm_min_epu16(s0, s1);
                                      t1 = _mm_cmpeq_epi16(t3, s0);
                                      t2 = _mm_cmpeq_epi16(t3, s1);
                                      t0 = _mm_subs_epi16(t2, t1); //upsign

                                      t3 = _mm_min_epu16(s1, s2);
                                      t1 = _mm_cmpeq_epi16(t3, s1);
                                      t2 = _mm_cmpeq_epi16(t3, s2);
                                      t3 = _mm_subs_epi16(t1, t2); //downsign

                                      etype = _mm_adds_epi16(t0, t3); //edgetype

                                      t0 = _mm_cmpeq_epi16(etype, c0);
                                      t1 = _mm_cmpeq_epi16(etype, c1);
                                      t2 = _mm_cmpeq_epi16(etype, c2);
                                      t3 = _mm_cmpeq_epi16(etype, c3);
                                      t4 = _mm_cmpeq_epi16(etype, c4);

                                      t0 = _mm_and_si128(t0, off0);
                                      t1 = _mm_and_si128(t1, off1);
                                      t2 = _mm_and_si128(t2, off2);
                                      t3 = _mm_and_si128(t3, off3);
                                      t4 = _mm_and_si128(t4, off4);

                                      t0 = _mm_adds_epi16(t0, t1);
                                      t2 = _mm_adds_epi16(t2, t3);
                                      t0 = _mm_adds_epi16(t0, t4);
                                      t0 = _mm_adds_epi16(t0, t2);//get offset

                                      t1 = _mm_adds_epi16(t0, s1);
                                      t1 = _mm_min_epi16(t1, max_val);
                                      t1 = _mm_max_epi16(t1, min_val);

                                      if (x != end_x_r_8){
                                          _mm_storeu_si128((__m128i*)(dst + x), t1);
                                      }
                                      else{
                                          _mm_maskmoveu_si128(t1, mask_r, (char_t*)(dst + x));
                                          break;
                                      }
                                  }
                                  dst += i_dst;
                                  src += i_src;
                              }
                              //last row
                              for (x = start_x_rn; x < end_x_rn; x += 8) {
                                  s0 = _mm_loadu_si128((__m128i*)&src[x - i_src - 1]);
                                  s1 = _mm_loadu_si128((__m128i*)&src[x]);
                                  s2 = _mm_loadu_si128((__m128i*)&src[x + i_src + 1]);

                                  t3 = _mm_min_epu16(s0, s1);
                                  t1 = _mm_cmpeq_epi16(t3, s0);
                                  t2 = _mm_cmpeq_epi16(t3, s1);
                                  t0 = _mm_subs_epi16(t2, t1); //upsign

                                  t3 = _mm_min_epu16(s1, s2);
                                  t1 = _mm_cmpeq_epi16(t3, s1);
                                  t2 = _mm_cmpeq_epi16(t3, s2);
                                  t3 = _mm_subs_epi16(t1, t2); //downsign

                                  etype = _mm_adds_epi16(t0, t3); //edgetype

                                  t0 = _mm_cmpeq_epi16(etype, c0);
                                  t1 = _mm_cmpeq_epi16(etype, c1);
                                  t2 = _mm_cmpeq_epi16(etype, c2);
                                  t3 = _mm_cmpeq_epi16(etype, c3);
                                  t4 = _mm_cmpeq_epi16(etype, c4);

                                  t0 = _mm_and_si128(t0, off0);
                                  t1 = _mm_and_si128(t1, off1);
                                  t2 = _mm_and_si128(t2, off2);
                                  t3 = _mm_and_si128(t3, off3);
                                  t4 = _mm_and_si128(t4, off4);

                                  t0 = _mm_adds_epi16(t0, t1);
                                  t2 = _mm_adds_epi16(t2, t3);
                                  t0 = _mm_adds_epi16(t0, t4);
                                  t0 = _mm_adds_epi16(t0, t2);//get offset

                                  t1 = _mm_adds_epi16(t0, s1);
                                  t1 = _mm_min_epi16(t1, max_val);
                                  t1 = _mm_max_epi16(t1, min_val);

                                  if (x != end_x_rn_8){
                                      _mm_storeu_si128((__m128i*)(dst + x), t1);
                                  }
                                  else{
                                      mask_rn = _mm_load_si128((__m128i*)(intrinsic_mask_10bit[end_x_rn - end_x_rn_8 - 1]));
                                      _mm_maskmoveu_si128(t1, mask_rn, (char_t*)(dst + x));
                                      break;
                                  }
                              }
    }
        break;
    case SAO_TYPE_EO_45: {
                             __m128i mask_r0, mask_r, mask_rn;
                             int end_x_r0_8, end_x_r_8, end_x_rn_8;

                             c0 = _mm_set1_epi16(-2);
                             c1 = _mm_set1_epi16(-1);
                             c2 = _mm_set1_epi16(0);
                             c3 = _mm_set1_epi16(1);
                             c4 = _mm_set1_epi16(2);

                             off0 = _mm_set1_epi16((pel_t)saoBlkParam->offset[0]);
                             off1 = _mm_set1_epi16((pel_t)saoBlkParam->offset[1]);
                             off2 = _mm_set1_epi16((pel_t)saoBlkParam->offset[2]);
                             off3 = _mm_set1_epi16((pel_t)saoBlkParam->offset[3]);
                             off4 = _mm_set1_epi16((pel_t)saoBlkParam->offset[4]);

                             start_x_r0 = smb_available_up ? (smb_available_left ? 0 : 1) : (smb_pix_width - 1);
                             end_x_r0 = smb_available_upright ? smb_pix_width : (smb_pix_width - 1);
                             start_x_r = smb_available_left ? 0 : 1;
                             end_x_r = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
                             start_x_rn = smb_available_leftdown ? 0 : 1;
                             end_x_rn = smb_available_down ? (smb_available_right ? smb_pix_width : (smb_pix_width - 1)) : 1;

                             end_x_r0_8 = end_x_r0 - ((end_x_r0 - start_x_r0) & 0x07);
                             end_x_r_8 = end_x_r - ((end_x_r - start_x_r) & 0x07);
                             end_x_rn_8 = end_x_rn - ((end_x_rn - start_x_rn) & 0x07);


                             //first row
                             for (x = start_x_r0; x < end_x_r0; x += 8) {
                                 s0 = _mm_loadu_si128((__m128i*)&src[x - i_src + 1]);
                                 s1 = _mm_loadu_si128((__m128i*)&src[x]);
                                 s2 = _mm_loadu_si128((__m128i*)&src[x + i_src - 1]);

                                 t3 = _mm_min_epu16(s0, s1);
                                 t1 = _mm_cmpeq_epi16(t3, s0);
                                 t2 = _mm_cmpeq_epi16(t3, s1);
                                 t0 = _mm_subs_epi16(t2, t1); //upsign

                                 t3 = _mm_min_epu16(s1, s2);
                                 t1 = _mm_cmpeq_epi16(t3, s1);
                                 t2 = _mm_cmpeq_epi16(t3, s2);
                                 t3 = _mm_subs_epi16(t1, t2); //downsign

                                 etype = _mm_adds_epi16(t0, t3); //edgetype

                                 t0 = _mm_cmpeq_epi16(etype, c0);
                                 t1 = _mm_cmpeq_epi16(etype, c1);
                                 t2 = _mm_cmpeq_epi16(etype, c2);
                                 t3 = _mm_cmpeq_epi16(etype, c3);
                                 t4 = _mm_cmpeq_epi16(etype, c4);

                                 t0 = _mm_and_si128(t0, off0);
                                 t1 = _mm_and_si128(t1, off1);
                                 t2 = _mm_and_si128(t2, off2);
                                 t3 = _mm_and_si128(t3, off3);
                                 t4 = _mm_and_si128(t4, off4);

                                 t0 = _mm_adds_epi16(t0, t1);
                                 t2 = _mm_adds_epi16(t2, t3);
                                 t0 = _mm_adds_epi16(t0, t4);
                                 t0 = _mm_adds_epi16(t0, t2);//get offset

                                 t1 = _mm_adds_epi16(t0, s1);
                                 t1 = _mm_min_epi16(t1, max_val);
                                 t1 = _mm_max_epi16(t1, min_val);

                                 if (x != end_x_r0_8){
                                     _mm_storeu_si128((__m128i*)(dst + x), t1);
                                 }
                                 else{
                                     mask_r0 = _mm_load_si128((__m128i*)(intrinsic_mask_10bit[end_x_r0 - end_x_r0_8 - 1]));
                                     _mm_maskmoveu_si128(t1, mask_r0, (char_t*)(dst + x));
                                     break;
                                 }
                             }
                             dst += i_dst;
                             src += i_src;

                             mask_r = _mm_load_si128((__m128i*)(intrinsic_mask_10bit[end_x_r - end_x_r_8 - 1]));
                             //middle rows
                             for (y = 1; y < smb_pix_height - 1; y++) {
                                 for (x = start_x_r; x < end_x_r; x += 8) {
                                     s0 = _mm_loadu_si128((__m128i*)&src[x - i_src + 1]);
                                     s1 = _mm_loadu_si128((__m128i*)&src[x]);
                                     s2 = _mm_loadu_si128((__m128i*)&src[x + i_src - 1]);

                                     t3 = _mm_min_epu16(s0, s1);
                                     t1 = _mm_cmpeq_epi16(t3, s0);
                                     t2 = _mm_cmpeq_epi16(t3, s1);
                                     t0 = _mm_subs_epi16(t2, t1); //upsign

                                     t3 = _mm_min_epu16(s1, s2);
                                     t1 = _mm_cmpeq_epi16(t3, s1);
                                     t2 = _mm_cmpeq_epi16(t3, s2);
                                     t3 = _mm_subs_epi16(t1, t2); //downsign

                                     etype = _mm_adds_epi16(t0, t3); //edgetype

                                     t0 = _mm_cmpeq_epi16(etype, c0);
                                     t1 = _mm_cmpeq_epi16(etype, c1);
                                     t2 = _mm_cmpeq_epi16(etype, c2);
                                     t3 = _mm_cmpeq_epi16(etype, c3);
                                     t4 = _mm_cmpeq_epi16(etype, c4);

                                     t0 = _mm_and_si128(t0, off0);
                                     t1 = _mm_and_si128(t1, off1);
                                     t2 = _mm_and_si128(t2, off2);
                                     t3 = _mm_and_si128(t3, off3);
                                     t4 = _mm_and_si128(t4, off4);

                                     t0 = _mm_adds_epi16(t0, t1);
                                     t2 = _mm_adds_epi16(t2, t3);
                                     t0 = _mm_adds_epi16(t0, t4);
                                     t0 = _mm_adds_epi16(t0, t2);//get offset

                                     t1 = _mm_adds_epi16(t0, s1);
                                     t1 = _mm_min_epi16(t1, max_val);
                                     t1 = _mm_max_epi16(t1, min_val);

                                     if (x != end_x_r_8){
                                         _mm_storeu_si128((__m128i*)(dst + x), t1);
                                     }
                                     else{
                                         _mm_maskmoveu_si128(t1, mask_r, (char_t*)(dst + x));
                                         break;
                                     }
                                 }
                                 dst += i_dst;
                                 src += i_src;
                             }
                             for (x = start_x_rn; x < end_x_rn; x += 8) {
                                 s0 = _mm_loadu_si128((__m128i*)&src[x - i_src + 1]);
                                 s1 = _mm_loadu_si128((__m128i*)&src[x]);
                                 s2 = _mm_loadu_si128((__m128i*)&src[x + i_src - 1]);

                                 t3 = _mm_min_epu16(s0, s1);
                                 t1 = _mm_cmpeq_epi16(t3, s0);
                                 t2 = _mm_cmpeq_epi16(t3, s1);
                                 t0 = _mm_subs_epi16(t2, t1); //upsign

                                 t3 = _mm_min_epu16(s1, s2);
                                 t1 = _mm_cmpeq_epi16(t3, s1);
                                 t2 = _mm_cmpeq_epi16(t3, s2);
                                 t3 = _mm_subs_epi16(t1, t2); //downsign

                                 etype = _mm_adds_epi16(t0, t3); //edgetype

                                 t0 = _mm_cmpeq_epi16(etype, c0);
                                 t1 = _mm_cmpeq_epi16(etype, c1);
                                 t2 = _mm_cmpeq_epi16(etype, c2);
                                 t3 = _mm_cmpeq_epi16(etype, c3);
                                 t4 = _mm_cmpeq_epi16(etype, c4);

                                 t0 = _mm_and_si128(t0, off0);
                                 t1 = _mm_and_si128(t1, off1);
                                 t2 = _mm_and_si128(t2, off2);
                                 t3 = _mm_and_si128(t3, off3);
                                 t4 = _mm_and_si128(t4, off4);

                                 t0 = _mm_adds_epi16(t0, t1);
                                 t2 = _mm_adds_epi16(t2, t3);
                                 t0 = _mm_adds_epi16(t0, t4);
                                 t0 = _mm_adds_epi16(t0, t2);//get offset

                                 t1 = _mm_adds_epi16(t0, s1);
                                 t1 = _mm_min_epi16(t1, max_val);
                                 t1 = _mm_max_epi16(t1, min_val);

                                 if (x != end_x_rn_8){
                                     _mm_storeu_si128((__m128i*)(dst + x), t1);
                                 }
                                 else{
                                     mask_rn = _mm_load_si128((__m128i*)(intrinsic_mask_10bit[end_x_rn - end_x_rn_8 - 1]));
                                     _mm_maskmoveu_si128(t1, mask_rn, (char_t*)(dst + x));
                                     break;
                                 }
                             }
    }
        break;
    case SAO_TYPE_BO: {
                          __m128i r0, r1, r2, r3;
                          int shift_bo = sample_bit_depth - NUM_SAO_BO_CLASSES_IN_BIT;
                          int end_x_8 = smb_pix_width - 7;
                          int id0 = saoBlkParam->startBand;
                          int id1 = (id0 + 1) & 0x1f;
                          int id2 = saoBlkParam->startBand2;
                          int id3 = (id2 + 1) & 0x1f;

                          r0   = _mm_set1_epi16(saoBlkParam->startBand);
                          r1   = _mm_set1_epi16((saoBlkParam->startBand + 1) % 32);
                          r2   = _mm_set1_epi16(saoBlkParam->startBand2);
                          r3   = _mm_set1_epi16((saoBlkParam->startBand2 + 1) % 32);
                          off0 = _mm_set1_epi16(saoBlkParam->offset[0]);
                          off1 = _mm_set1_epi16(saoBlkParam->offset[1]);
                          off2 = _mm_set1_epi16(saoBlkParam->offset[2]);
                          off3 = _mm_set1_epi16(saoBlkParam->offset[3]);

                          if (smb_pix_width == 4){
                              mask = _mm_set_epi32(0, 0, -1, -1);
                              
                              for (y = 0; y < smb_pix_height; y++) {
                                  s0 = _mm_loadu_si128((__m128i*)src);

                                  s1 = _mm_srai_epi16(s0, shift_bo);

                                  t0 = _mm_cmpeq_epi16(s1, r0);
                                  t1 = _mm_cmpeq_epi16(s1, r1);
                                  t2 = _mm_cmpeq_epi16(s1, r2);
                                  t3 = _mm_cmpeq_epi16(s1, r3);

                                  t0 = _mm_and_si128(t0, off0);
                                  t1 = _mm_and_si128(t1, off1);
                                  t2 = _mm_and_si128(t2, off2);
                                  t3 = _mm_and_si128(t3, off3);
                                  t0 = _mm_or_si128(t0, t1);
                                  t2 = _mm_or_si128(t2, t3);
                                  t0 = _mm_or_si128(t0, t2);

                                  t1 = _mm_adds_epi16(s0, t0);
                                  t1 = _mm_min_epi16(t1, max_val);
                                  t1 = _mm_max_epi16(t1, min_val);

                                  _mm_maskmoveu_si128(t1, mask, (char_t*)(dst));

                                  dst += i_dst;
                                  src += i_src;
                              }
                          }
                          else{
                              mask = _mm_load_si128((const __m128i*)intrinsic_mask_10bit[(smb_pix_width & 7) - 1]);

                              for (y = 0; y < smb_pix_height; y++) {
                                  for (x = 0; x < smb_pix_width; x += 8) {
                                      s0 = _mm_loadu_si128((__m128i*)&src[x]);

                                      s1 = _mm_srai_epi16(s0, shift_bo);

                                      t0 = _mm_cmpeq_epi16(s1, r0);
                                      t1 = _mm_cmpeq_epi16(s1, r1);
                                      t2 = _mm_cmpeq_epi16(s1, r2);
                                      t3 = _mm_cmpeq_epi16(s1, r3);

                                      t0 = _mm_and_si128(t0, off0);
                                      t1 = _mm_and_si128(t1, off1);
                                      t2 = _mm_and_si128(t2, off2);
                                      t3 = _mm_and_si128(t3, off3);
                                      t0 = _mm_or_si128(t0, t1);
                                      t2 = _mm_or_si128(t2, t3);
                                      t0 = _mm_or_si128(t0, t2);
                                      //src0 = _mm_adds_epi8(src0, t0);

                                      //add 8 nums once for possible overflow
                                      t1 = _mm_adds_epi16(s0, t0);
                                      t1 = _mm_min_epi16(t1, max_val);
                                      t1 = _mm_max_epi16(t1, min_val);

                                      if (x < end_x_8) {
                                          _mm_storeu_si128((__m128i*)&dst[x], t1);
                                      }
                                      else {
                                          _mm_maskmoveu_si128(t1, mask, (char_t*)(dst + x));
                                      }

                                  }

                                  dst += i_dst;
                                  src += i_src;
                              }
                          }
    }
        break;
    default: {
                 printf("Not a supported SAO types\n");
                 assert(0);
                 exit(-1);
    }
    }
}

