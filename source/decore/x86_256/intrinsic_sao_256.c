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

#include "intrinsic_256.h"


// for smb_width = 32 or 64
ALIGNED_16(pel_t sao_mask_32[7][16]) = {
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }//don't use
};

//for smb_width = 4
ALIGNED_16(pel_t sao_mask_4[2][2][16]) = {
	{
        { 0, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		{ -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	}
};

void SAO_on_block_sse256(com_pic_t *pic_src, com_pic_t *pic_dst, sap_param_t *saoBlkParam, int compIdx, int smb_index, int pix_y, int pix_x, int smb_pix_height, int smb_pix_width,
    int smb_available_left, int smb_available_right, int smb_available_up, int smb_available_down, int smb_available_upleft,
    int smb_available_upright, int smb_available_leftdown, int smb_available_rightdwon, int sample_bit_depth)
{
    int type;
    int start_x, end_x, start_y, end_y;
    int start_x_r0, end_x_r0, start_x_r, end_x_r, start_x_rn, end_x_rn;
    int x, y;
    pel_t *src, *dst;
    int i_src, i_dst;
    int reg = 0;

    if (compIdx == SAO_Y) {
        i_src = pic_src->i_stride;
        src = pic_src->p_y + pix_y * i_src + pix_x;
        i_dst = pic_dst->i_stride;
        dst = pic_dst->p_y + pix_y * i_dst + pix_x;
    }
    else {
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
        if (smb_pix_width == SAO_SHIFT_PIX_NUM){
            __m128i off;
            __m128i s0, s1;
            __m128i t0, t1, t2, t3, etype;
            __m128i mask;
            __m128i c2 = _mm_set1_epi16(2);
            __m128i zero = _mm_setzero_si128();

            off = _mm_loadu_si128((__m128i*)saoBlkParam->offset);
            off = _mm_packs_epi32(off, _mm_set_epi32(0, 0, 0, saoBlkParam->offset[4]));
            off = _mm_packs_epi16(off, _mm_setzero_si128());

			mask = _mm_load_si128((__m128i*)(sao_mask_4[smb_available_left][smb_available_right]));
            for (y = 0; y < smb_pix_height; y++) {
                s0 = _mm_loadl_epi64((__m128i*)&src[-1]);
				s0 = _mm_unpacklo_epi8(s0, zero);
                s1 = _mm_srli_si128(s0, 2);

                t3 = _mm_min_epu16(s0, s1);
                t1 = _mm_cmpeq_epi16(t3, s0);
                t2 = _mm_cmpeq_epi16(t3, s1);
                t0 = _mm_subs_epi16(t2, t1); //leftsign

				t3 = _mm_srli_si128(t0, 2);

                etype = _mm_subs_epi16(t0, t3); //edgetype

                etype = _mm_adds_epi16(etype, c2);

                t0 = _mm_shuffle_epi8(off, _mm_packus_epi16(etype, zero));//get offset

                //add 8 nums once for possible overflow
                t1 = _mm_cvtepi8_epi16(t0);

                t1 = _mm_adds_epi16(t1, s1);
                t0 = _mm_packus_epi16(t1, zero); //saturated

                _mm_maskmoveu_si128(t0, mask, (char_t*)(dst));

                dst += i_dst;
                src += i_src;
            }
        }
        else{
            __m256i off;
            __m256i s0, s1, s2;
            __m256i t0, t1, t2, t3, t4, etype;
            __m128i mask, offtmp;
            __m256i c2 = _mm256_set1_epi8(2);

            int end_x_32;

            offtmp = _mm_loadu_si128((__m128i*)saoBlkParam->offset);
            offtmp = _mm_packs_epi32(offtmp, _mm_set_epi32(0, 0, 0, saoBlkParam->offset[4]));
            offtmp = _mm_packs_epi16(offtmp, _mm_setzero_si128());

			off = _mm256_castsi128_si256(offtmp);
			off = _mm256_inserti128_si256(off, offtmp, 1);

            start_x = smb_available_left ? 0 : 1;
            end_x = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
            end_x_32 = end_x - ((end_x - start_x) & 0x1f);

            mask = _mm_loadu_si128((__m128i*)(sao_mask_32[end_x - end_x_32 - 26]));
            for (y = 0; y < smb_pix_height; y++) {
                //diff = src[start_x] - src[start_x - 1];
                //leftsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
                for (x = start_x; x < end_x; x += 32) {
					s0 = _mm256_lddqu_si256((__m256i*)&src[x - 1]);
					s1 = _mm256_loadu_si256((__m256i*)&src[x]);
                    s2 = _mm256_loadu_si256((__m256i*)&src[x + 1]);

                    t3 = _mm256_min_epu8(s0, s1);
                    t1 = _mm256_cmpeq_epi8(t3, s0);
                    t2 = _mm256_cmpeq_epi8(t3, s1);
                    t0 = _mm256_subs_epi8(t2, t1); //leftsign

                    t3 = _mm256_min_epu8(s1, s2);
                    t1 = _mm256_cmpeq_epi8(t3, s1);
                    t2 = _mm256_cmpeq_epi8(t3, s2);
                    t3 = _mm256_subs_epi8(t1, t2); //rightsign

                    etype = _mm256_adds_epi8(t0, t3); //edgetype

                    etype = _mm256_adds_epi8(etype, c2);

                    t0 = _mm256_shuffle_epi8(off, etype);//get offset

                    //convert byte to short for possible overflow
					t1 = _mm256_cvtepi8_epi16(_mm256_castsi256_si128(t0));
                    t2 = _mm256_cvtepi8_epi16(_mm256_extracti128_si256(t0, 1));
					t3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(s1));
					t4 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(s1, 1));

                    t1 = _mm256_adds_epi16(t1, t3);
                    t2 = _mm256_adds_epi16(t2, t4);
                    t0 = _mm256_packus_epi16(t1, t2); //saturated
					t0 = _mm256_permute4x64_epi64(t0, 0xd8);

                    if (x != end_x_32){
                        _mm256_storeu_si256((__m256i*)(dst + x), t0);
                    }
                    else{
						_mm_storeu_si128((__m128i*)(dst + x), _mm256_castsi256_si128(t0));
                        _mm_maskmoveu_si128(_mm256_extracti128_si256(t0, 1), mask, (char_t*)(dst + x + 16));
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
        if (smb_pix_width == SAO_SHIFT_PIX_NUM){
            __m128i off;
            __m128i s0, s1, s2;
            __m128i t0, t1, t2, t3, etype;
            __m128i c2 = _mm_set1_epi16(2);
            __m128i mask;
            __m128i zero = _mm_setzero_si128();

            off = _mm_loadu_si128((__m128i*)saoBlkParam->offset);
            off = _mm_packs_epi32(off, _mm_set_epi32(0, 0, 0, saoBlkParam->offset[4]));
            off = _mm_packs_epi16(off, _mm_setzero_si128());

            start_y = smb_available_up ? 0 : 1;
            end_y = smb_available_down ? smb_pix_height : (smb_pix_height - 1);

            dst += start_y * i_dst;
            src += start_y * i_src;

            mask = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1);

			s0 = _mm_loadl_epi64((__m128i*)&src[-i_src]);
			s1 = _mm_loadl_epi64((__m128i*)src);
			s0 = _mm_unpacklo_epi8(s0, zero);
			s1 = _mm_unpacklo_epi8(s1, zero);
            for (y = start_y; y < end_y; y++) {
				s2 = _mm_loadl_epi64((__m128i*)&src[i_src]);
				s2 = _mm_unpacklo_epi8(s2, zero);

                t3 = _mm_min_epu16(s0, s1);
                t1 = _mm_cmpeq_epi16(t3, s0);
                t2 = _mm_cmpeq_epi16(t3, s1);
                t0 = _mm_subs_epi16(t2, t1); //upsign

                t3 = _mm_min_epu16(s1, s2);
                t1 = _mm_cmpeq_epi16(t3, s1);
                t2 = _mm_cmpeq_epi16(t3, s2);
                t3 = _mm_subs_epi16(t1, t2); //downsign

                etype = _mm_adds_epi16(t0, t3); //edgetype

                etype = _mm_adds_epi16(etype, c2);

                t0 = _mm_shuffle_epi8(off, _mm_packus_epi16(etype, zero));//get offset

                //convert to short type for possible overflow
                t1 = _mm_cvtepi8_epi16(t0);

                t1 = _mm_adds_epi16(t1, s1);
                t0 = _mm_packus_epi16(t1, zero); //saturated

                _mm_maskmoveu_si128(t0, mask, (char_t*)(dst));
                
				s0 = s1;
				s1 = s2;

                dst += i_dst;
                src += i_src;
            }
        }
        else{
            __m256i off;
            __m256i s0, s1, s2;
            __m256i t0, t1, t2, t3, t4, etype;
            __m128i mask, offtmp;
            __m256i c2 = _mm256_set1_epi8(2);
            int end_x_32 = smb_pix_width - (smb_pix_width & 0x1f);

			offtmp = _mm_loadu_si128((__m128i*)saoBlkParam->offset);
			offtmp = _mm_packs_epi32(offtmp, _mm_set_epi32(0, 0, 0, saoBlkParam->offset[4]));
			offtmp = _mm_packs_epi16(offtmp, _mm_setzero_si128());

			off = _mm256_castsi128_si256(offtmp);
			off = _mm256_inserti128_si256(off, offtmp, 1);

            start_y = smb_available_up ? 0 : 1;
            end_y = smb_available_down ? smb_pix_height : (smb_pix_height - 1);

            dst += start_y * i_dst;
            src += start_y * i_src;

            mask = _mm_set_epi8(0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
            for (y = start_y; y < end_y; y++) {
                //diff = src[start_x] - src[start_x - 1];
                //leftsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
                for (x = 0; x < smb_pix_width; x += 32) {
					s0 = _mm256_lddqu_si256((__m256i*)&src[x - i_src]);
					s1 = _mm256_lddqu_si256((__m256i*)&src[x]);
					s2 = _mm256_lddqu_si256((__m256i*)&src[x + i_src]);

                    t3 = _mm256_min_epu8(s0, s1);
                    t1 = _mm256_cmpeq_epi8(t3, s0);
                    t2 = _mm256_cmpeq_epi8(t3, s1);
                    t0 = _mm256_subs_epi8(t2, t1); //leftsign

                    t3 = _mm256_min_epu8(s1, s2);
                    t1 = _mm256_cmpeq_epi8(t3, s1);
                    t2 = _mm256_cmpeq_epi8(t3, s2);
                    t3 = _mm256_subs_epi8(t1, t2); //rightsign

                    etype = _mm256_adds_epi8(t0, t3); //edgetype

                    etype = _mm256_adds_epi8(etype, c2);

                    t0 = _mm256_shuffle_epi8(off, etype);//get offset

					//convert byte to short for possible overflow
					t1 = _mm256_cvtepi8_epi16(_mm256_castsi256_si128(t0));
					t2 = _mm256_cvtepi8_epi16(_mm256_extracti128_si256(t0, 1));
					t3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(s1));
					t4 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(s1, 1));

					t1 = _mm256_adds_epi16(t1, t3);
					t2 = _mm256_adds_epi16(t2, t4);
					t0 = _mm256_packus_epi16(t1, t2); //saturated
					t0 = _mm256_permute4x64_epi64(t0, 0xd8);

                    if (x != end_x_32){
                        _mm256_storeu_si256((__m256i*)(dst + x), t0);
                    }
                    else{
						_mm_storeu_si128((__m128i*)(dst + x), _mm256_castsi256_si128(t0));
                        _mm_maskmoveu_si128(_mm256_extracti128_si256(t0, 1), mask, (char_t*)(dst + x + 16));
                        break;
                    }
                }
                dst += i_dst;
                src += i_src;
            }
        }
    }
        break;
    case SAO_TYPE_EO_135: {
        if (smb_pix_width == SAO_SHIFT_PIX_NUM){
            __m128i off;
            __m128i s0, s1, s2, s4;
            __m128i t0, t1, t2, t3, etype;
            __m128i c2 = _mm_set1_epi8(2);
            __m128i mask;
            __m128i zero = _mm_setzero_si128();

            off = _mm_loadu_si128((__m128i*)saoBlkParam->offset);
            off = _mm_packs_epi32(off, _mm_set_epi32(0, 0, 0, saoBlkParam->offset[4]));
            off = _mm_packs_epi16(off, _mm_setzero_si128());

            //first row
			s1 = _mm_loadl_epi64((__m128i*)(src - 1));
			s4 = _mm_loadl_epi64((__m128i*)&src[i_src - 1]);
			s2 = _mm_srli_si128(s4, 1);

			t3 = _mm_min_epu8(s1, s2);
			t1 = _mm_cmpeq_epi8(t3, s1);
			t2 = _mm_cmpeq_epi8(t3, s2);
			t3 = _mm_subs_epi8(t1, t2); //downsign
			if (smb_available_up){
				s0 = _mm_loadl_epi64((__m128i*)&src[-i_src - 1]);
				s1 = _mm_srli_si128(s1, 1);

				t0 = _mm_min_epu8(s0, s1);
				t1 = _mm_cmpeq_epi8(t0, s0);
				t2 = _mm_cmpeq_epi8(t0, s1);
				t0 = _mm_subs_epi8(t2, t1); //upsign

				t2 = _mm_srli_si128(t3, 1);
				etype = _mm_adds_epi8(t0, t2); //edgetype

				etype = _mm_adds_epi8(etype, c2);

				t0 = _mm_shuffle_epi8(off, etype);//get offset

				//add 8 nums once for possible overflow
				t1 = _mm_cvtepi8_epi16(t0);
				t2 = _mm_unpacklo_epi8(s1, zero);

				t1 = _mm_adds_epi16(t1, t2);
				t0 = _mm_packus_epi16(t1, zero); //saturated

				mask = _mm_loadu_si128((__m128i*)sao_mask_4[smb_available_upleft][smb_available_right]);
				_mm_maskmoveu_si128(t0, mask, (char_t*)(dst));
			}
            dst += i_dst;
            src += i_src;

            //middle rows
			mask = _mm_load_si128((__m128i*)(sao_mask_4[smb_available_left][smb_available_right]));
			s1 = s4;						//src[-1]
            for (y = 1; y < smb_pix_height - 1; y++) {
				s4 = _mm_loadl_epi64((__m128i*)&src[i_src - 1]);
				s2 = _mm_srli_si128(s4, 1);

				t0 = t3; //upsign

                t3 = _mm_min_epu8(s1, s2);
                t1 = _mm_cmpeq_epi8(t3, s1);
                t2 = _mm_cmpeq_epi8(t3, s2);
                t3 = _mm_subs_epi8(t1, t2); //downsign

				etype = _mm_subs_epi8(_mm_srli_si128(t3, 1), t0); //edgetype

                etype = _mm_adds_epi8(etype, c2);

                t0 = _mm_shuffle_epi8(off, etype);//get offset

                //add 8 nums once for possible overflow
                t1 = _mm_cvtepi8_epi16(t0);
				t2 = _mm_unpacklo_epi8(_mm_srli_si128(s1, 1), zero);

                t1 = _mm_adds_epi16(t1, t2);
                t0 = _mm_packus_epi16(t1, zero); //saturated

                _mm_maskmoveu_si128(t0, mask, (char_t*)(dst));
                
				s1 = s4;
                dst += i_dst;
                src += i_src;
            }
            //last row
			if (smb_available_down){
				s1 = _mm_srli_si128(s1, 1);
				s2 = _mm_loadl_epi64((__m128i*)&src[i_src + 1]);

				t0 = t3; //upsign

				t3 = _mm_min_epu8(s1, s2);
				t1 = _mm_cmpeq_epi8(t3, s1);
				t2 = _mm_cmpeq_epi8(t3, s2);
				t3 = _mm_subs_epi8(t1, t2); //downsign

				etype = _mm_subs_epi8(t3, t0); //edgetype

				etype = _mm_adds_epi8(etype, c2);

				t0 = _mm_shuffle_epi8(off, etype);//get offset

				//add 8 nums once for possible overflow
				t1 = _mm_cvtepi8_epi16(t0);
				t3 = _mm_unpacklo_epi8(s1, zero);

				t1 = _mm_adds_epi16(t1, t3);
				t0 = _mm_packus_epi16(t1, zero); //saturated

				mask = _mm_load_si128((__m128i*)(sao_mask_4[smb_available_left][smb_available_rightdwon]));
				_mm_maskmoveu_si128(t0, mask, (char_t*)(dst));
			}
        }
        else {
            __m256i off;
            __m256i s0, s1, s2;
            __m256i t0, t1, t2, t3, t4, etype;
            __m128i mask, offtmp;
            __m256i c2 = _mm256_set1_epi8(2);
            int end_x_r0_32, end_x_r_32, end_x_rn_32;

			offtmp = _mm_loadu_si128((__m128i*)saoBlkParam->offset);
			offtmp = _mm_packs_epi32(offtmp, _mm_set_epi32(0, 0, 0, saoBlkParam->offset[4]));
			offtmp = _mm_packs_epi16(offtmp, _mm_setzero_si128());

			off = _mm256_castsi128_si256(offtmp);
			off = _mm256_inserti128_si256(off, offtmp, 1);

            start_x_r0 = smb_available_upleft ? 0 : 1;
            end_x_r0 = smb_available_up ? (smb_available_right ? smb_pix_width : (smb_pix_width - 1)) : 1;
            start_x_r = smb_available_left ? 0 : 1;
            end_x_r = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
            start_x_rn = smb_available_down ? (smb_available_left ? 0 : 1) : (smb_pix_width - 1);
            end_x_rn = smb_available_rightdwon ? smb_pix_width : (smb_pix_width - 1);

            end_x_r0_32 = end_x_r0 - ((end_x_r0 - start_x_r0) & 0x1f);
            end_x_r_32 = end_x_r - ((end_x_r - start_x_r) & 0x1f);
            end_x_rn_32 = end_x_rn - ((end_x_rn - start_x_rn) & 0x1f);

            //first row
            for (x = start_x_r0; x < end_x_r0; x += 32) {
                s0 = _mm256_loadu_si256((__m256i*)&src[x - i_src - 1]);
                s1 = _mm256_loadu_si256((__m256i*)&src[x]);
                s2 = _mm256_loadu_si256((__m256i*)&src[x + i_src + 1]);

                t3 = _mm256_min_epu8(s0, s1);
                t1 = _mm256_cmpeq_epi8(t3, s0);
                t2 = _mm256_cmpeq_epi8(t3, s1);
                t0 = _mm256_subs_epi8(t2, t1); //upsign

                t3 = _mm256_min_epu8(s1, s2);
                t1 = _mm256_cmpeq_epi8(t3, s1);
                t2 = _mm256_cmpeq_epi8(t3, s2);
                t3 = _mm256_subs_epi8(t1, t2); //downsign

                etype = _mm256_adds_epi8(t0, t3); //edgetype

                etype = _mm256_adds_epi8(etype, c2);

                t0 = _mm256_shuffle_epi8(off, etype);//get offset
				//convert byte to short for possible overflow
				t1 = _mm256_cvtepi8_epi16(_mm256_castsi256_si128(t0));
				t2 = _mm256_cvtepi8_epi16(_mm256_extracti128_si256(t0, 1));
				t3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(s1));
				t4 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(s1, 1));

				t1 = _mm256_adds_epi16(t1, t3);
				t2 = _mm256_adds_epi16(t2, t4);
				t0 = _mm256_packus_epi16(t1, t2); //saturated
				t0 = _mm256_permute4x64_epi64(t0, 0xd8);

                if (x != end_x_r0_32){
                    _mm256_storeu_si256((__m256i*)(dst + x), t0);
                }
                else{
                    mask = _mm_loadu_si128((__m128i*)sao_mask_32[end_x_r0 - end_x_r0_32 - 26]);
					_mm_storeu_si128((__m128i*)(dst + x), _mm256_castsi256_si128(t0));
                    _mm_maskmoveu_si128(_mm256_extracti128_si256(t0, 1), mask, (char_t*)(dst + x + 16));
                    break;
                }
            }
            dst += i_dst;
            src += i_src;

            mask = _mm_load_si128((__m128i*)(sao_mask_32[end_x_r - end_x_r_32 - 26]));
            //middle rows
            for (y = 1; y < smb_pix_height - 1; y++) {
                for (x = start_x_r; x < end_x_r; x += 32) {
                    s0 = _mm256_loadu_si256((__m256i*)&src[x - i_src - 1]);
                    s1 = _mm256_loadu_si256((__m256i*)&src[x]);
                    s2 = _mm256_loadu_si256((__m256i*)&src[x + i_src + 1]);

                    t3 = _mm256_min_epu8(s0, s1);
                    t1 = _mm256_cmpeq_epi8(t3, s0);
                    t2 = _mm256_cmpeq_epi8(t3, s1);
                    t0 = _mm256_subs_epi8(t2, t1); //upsign

                    t3 = _mm256_min_epu8(s1, s2);
                    t1 = _mm256_cmpeq_epi8(t3, s1);
                    t2 = _mm256_cmpeq_epi8(t3, s2);
                    t3 = _mm256_subs_epi8(t1, t2); //downsign

                    etype = _mm256_adds_epi8(t0, t3); //edgetype

                    etype = _mm256_adds_epi8(etype, c2);

                    t0 = _mm256_shuffle_epi8(off, etype);//get offset

					//convert byte to short for possible overflow
					t1 = _mm256_cvtepi8_epi16(_mm256_castsi256_si128(t0));
					t2 = _mm256_cvtepi8_epi16(_mm256_extracti128_si256(t0, 1));
					t3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(s1));
					t4 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(s1, 1));

					t1 = _mm256_adds_epi16(t1, t3);
					t2 = _mm256_adds_epi16(t2, t4);
					t0 = _mm256_packus_epi16(t1, t2); //saturated
					t0 = _mm256_permute4x64_epi64(t0, 0xd8);

                    if (x != end_x_r_32){
                        _mm256_storeu_si256((__m256i*)(dst + x), t0);
                    }
                    else{
						_mm_storeu_si128((__m128i*)(dst + x), _mm256_castsi256_si128(t0));
                        _mm_maskmoveu_si128(_mm256_extracti128_si256(t0, 1), mask, (char_t*)(dst + x + 16));
                        break;
                    }
                }
                dst += i_dst;
                src += i_src;
            }
            //last row
            for (x = start_x_rn; x < end_x_rn; x += 32) {
                s0 = _mm256_loadu_si256((__m256i*)&src[x - i_src - 1]);
                s1 = _mm256_loadu_si256((__m256i*)&src[x]);
                s2 = _mm256_loadu_si256((__m256i*)&src[x + i_src + 1]);

                t3 = _mm256_min_epu8(s0, s1);
                t1 = _mm256_cmpeq_epi8(t3, s0);
                t2 = _mm256_cmpeq_epi8(t3, s1);
                t0 = _mm256_subs_epi8(t2, t1); //upsign

                t3 = _mm256_min_epu8(s1, s2);
                t1 = _mm256_cmpeq_epi8(t3, s1);
                t2 = _mm256_cmpeq_epi8(t3, s2);
                t3 = _mm256_subs_epi8(t1, t2); //downsign

                etype = _mm256_adds_epi8(t0, t3); //edgetype

                etype = _mm256_adds_epi8(etype, c2);

                t0 = _mm256_shuffle_epi8(off, etype);//get offset

				//convert byte to short for possible overflow
				t1 = _mm256_cvtepi8_epi16(_mm256_castsi256_si128(t0));
				t2 = _mm256_cvtepi8_epi16(_mm256_extracti128_si256(t0, 1));
				t3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(s1));
				t4 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(s1, 1));

				t1 = _mm256_adds_epi16(t1, t3);
				t2 = _mm256_adds_epi16(t2, t4);
				t0 = _mm256_packus_epi16(t1, t2); //saturated
				t0 = _mm256_permute4x64_epi64(t0, 0xd8);

                if (x != end_x_rn_32){
                    _mm256_storeu_si256((__m256i*)(dst + x), t0);
                }
                else{
                    mask = _mm_load_si128((__m128i*)(sao_mask_32[end_x_rn - end_x_rn_32 - 26]));
					_mm_storeu_si128((__m128i*)(dst + x), _mm256_castsi256_si128(t0));
                    _mm_maskmoveu_si128(_mm256_extracti128_si256(t0, 1), mask, (char_t*)(dst + x + 16));
                    break;
                }
            }
        }
    }
        break;
    case SAO_TYPE_EO_45: {
        if (smb_pix_width == SAO_SHIFT_PIX_NUM){
            __m128i off;
            __m128i s0, s1, s2;
            __m128i t0, t1, t2, t3, etype;
            __m128i c2 = _mm_set1_epi8(2);
            __m128i mask;
            __m128i zero = _mm_setzero_si128();

            off = _mm_loadu_si128((__m128i*)saoBlkParam->offset);
            off = _mm_packs_epi32(off, _mm_set_epi32(0, 0, 0, saoBlkParam->offset[4]));
            off = _mm_packs_epi16(off, zero);

            //first row
			s1 = _mm_loadl_epi64((__m128i*)(src));
			s2 = _mm_loadl_epi64((__m128i*)&src[i_src - 1]);

			t3 = _mm_min_epu8(s1, s2);
			t1 = _mm_cmpeq_epi8(t3, s1);
			t2 = _mm_cmpeq_epi8(t3, s2);
			t3 = _mm_subs_epi8(t1, t2); //downsign

			if (smb_available_up){
				s0 = _mm_loadl_epi64((__m128i*)&src[-i_src + 1]);

				t0 = _mm_min_epu8(s0, s1);
				t1 = _mm_cmpeq_epi8(t0, s0);
				t2 = _mm_cmpeq_epi8(t0, s1);
				t0 = _mm_subs_epi8(t2, t1); //upsign
				etype = _mm_adds_epi8(t0, t3); //edgetype

				etype = _mm_adds_epi8(etype, c2);

				t0 = _mm_shuffle_epi8(off, etype);//get offset

				//add 8 nums once for possible overflow
				t1 = _mm_cvtepi8_epi16(t0);
				t2 = _mm_unpacklo_epi8(s1, zero);

				t1 = _mm_adds_epi16(t1, t2);
				t0 = _mm_packus_epi16(t1, zero); //saturated

				mask = _mm_load_si128((__m128i*)(sao_mask_4[smb_available_left][smb_available_upright]));
				_mm_maskmoveu_si128(t0, mask, (char_t*)(dst));
			}

            dst += i_dst;
            src += i_src;

            //middle rows
			mask = _mm_load_si128((__m128i*)(sao_mask_4[smb_available_left][smb_available_right]));
            for (y = 1; y < smb_pix_height - 1; y++) {
				s1 = _mm_srli_si128(s2, 1);						//src[0]
				s2 = _mm_loadl_epi64((__m128i*)&src[i_src - 1]);

				t0 = _mm_srli_si128(t3, 1); //upsign 

                t3 = _mm_min_epu8(s1, s2);
                t1 = _mm_cmpeq_epi8(t3, s1);
                t2 = _mm_cmpeq_epi8(t3, s2);
                t3 = _mm_subs_epi8(t1, t2); //downsign

                etype = _mm_subs_epi8(t3, t0); //edgetype

                etype = _mm_adds_epi8(etype, c2);

                t0 = _mm_shuffle_epi8(off, etype);//get offset

                //add 8 nums once for possible overflow
                t1 = _mm_cvtepi8_epi16(t0);
                t2 = _mm_unpacklo_epi8(s1, zero);

                t1 = _mm_adds_epi16(t1, t2);
                t0 = _mm_packus_epi16(t1, zero); //saturated

                _mm_maskmoveu_si128(t0, mask, (char_t*)(dst));
                
                dst += i_dst;
                src += i_src;
            }

			if (smb_available_down){
				s1 = _mm_srli_si128(s2, 1);						//src[0]
				s2 = _mm_loadl_epi64((__m128i*)&src[i_src - 1]);

				t0 = _mm_srli_si128(t3, 1); //upsign

				t3 = _mm_min_epu8(s1, s2);
				t1 = _mm_cmpeq_epi8(t3, s1);
				t2 = _mm_cmpeq_epi8(t3, s2);
				t3 = _mm_subs_epi8(t1, t2); //downsign

				etype = _mm_subs_epi8(t3, t0); //edgetype

				etype = _mm_adds_epi8(etype, c2);

				t0 = _mm_shuffle_epi8(off, etype);//get offset

				//add 8 nums once for possible overflow
				t1 = _mm_cvtepi8_epi16(t0);
				t3 = _mm_unpacklo_epi8(s1, zero);

				t1 = _mm_adds_epi16(t1, t3);
				t0 = _mm_packus_epi16(t1, zero); //saturated

				mask = _mm_load_si128((__m128i*)(sao_mask_4[smb_available_leftdown][smb_available_right]));
				_mm_maskmoveu_si128(t0, mask, (char_t*)(dst));
			}
        }
		else{
			__m256i off;
			__m256i s0, s1, s2;
			__m256i t0, t1, t2, t3, t4, etype;
			__m128i mask, offtmp;
			__m256i c2 = _mm256_set1_epi8(2);
			int end_x_r0_32, end_x_r_32, end_x_rn_32;


			offtmp = _mm_loadu_si128((__m128i*)saoBlkParam->offset);
			offtmp = _mm_packs_epi32(offtmp, _mm_set_epi32(0, 0, 0, saoBlkParam->offset[4]));
			offtmp = _mm_packs_epi16(offtmp, _mm_setzero_si128());

			off = _mm256_castsi128_si256(offtmp);
			off = _mm256_inserti128_si256(off, offtmp, 1);

			start_x_r0 = smb_available_up ? (smb_available_left ? 0 : 1) : (smb_pix_width - 1);
			end_x_r0 = smb_available_upright ? smb_pix_width : (smb_pix_width - 1);
			start_x_r = smb_available_left ? 0 : 1;
			end_x_r = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
			start_x_rn = smb_available_leftdown ? 0 : 1;
			end_x_rn = smb_available_down ? (smb_available_right ? smb_pix_width : (smb_pix_width - 1)) : 1;

			end_x_r0_32 = end_x_r0 - ((end_x_r0 - start_x_r0) & 0x1f);
			end_x_r_32 = end_x_r - ((end_x_r - start_x_r) & 0x1f);
			end_x_rn_32 = end_x_rn - ((end_x_rn - start_x_rn) & 0x1f);

			//first row
			for (x = start_x_r0; x < end_x_r0; x += 32) {
				s0 = _mm256_loadu_si256((__m256i*)&src[x - i_src + 1]);
				s1 = _mm256_loadu_si256((__m256i*)&src[x]);
				s2 = _mm256_loadu_si256((__m256i*)&src[x + i_src - 1]);

				t3 = _mm256_min_epu8(s0, s1);
				t1 = _mm256_cmpeq_epi8(t3, s0);
				t2 = _mm256_cmpeq_epi8(t3, s1);
				t0 = _mm256_subs_epi8(t2, t1); //upsign

				t3 = _mm256_min_epu8(s1, s2);
				t1 = _mm256_cmpeq_epi8(t3, s1);
				t2 = _mm256_cmpeq_epi8(t3, s2);
				t3 = _mm256_subs_epi8(t1, t2); //downsign

				etype = _mm256_adds_epi8(t0, t3); //edgetype

				etype = _mm256_adds_epi8(etype, c2);

				t0 = _mm256_shuffle_epi8(off, etype);//get offset

				//convert byte to short for possible overflow
				t1 = _mm256_cvtepi8_epi16(_mm256_castsi256_si128(t0));
				t2 = _mm256_cvtepi8_epi16(_mm256_extracti128_si256(t0, 1));
				t3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(s1));
				t4 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(s1, 1));

				t1 = _mm256_adds_epi16(t1, t3);
				t2 = _mm256_adds_epi16(t2, t4);
				t0 = _mm256_packus_epi16(t1, t2); //saturated
				t0 = _mm256_permute4x64_epi64(t0, 0xd8);

				if (x != end_x_r0_32){
					_mm256_storeu_si256((__m256i*)(dst + x), t0);
				}
				else{
					mask = _mm_load_si128((__m128i*)(sao_mask_32[end_x_r0 - end_x_r0_32 - 26]));
					_mm_storeu_si128((__m128i*)(dst + x), _mm256_castsi256_si128(t0));
					_mm_maskmoveu_si128(_mm256_extracti128_si256(t0, 1), mask, (char_t*)(dst + x + 16));
					break;
				}
			}
			dst += i_dst;
			src += i_src;

			mask = _mm_load_si128((__m128i*)(sao_mask_32[end_x_r - end_x_r_32 - 26]));
			//middle rows
			for (y = 1; y < smb_pix_height - 1; y++) {
				for (x = start_x_r; x < end_x_r; x += 32) {
					s0 = _mm256_loadu_si256((__m256i*)&src[x - i_src + 1]);
					s1 = _mm256_loadu_si256((__m256i*)&src[x]);
					s2 = _mm256_loadu_si256((__m256i*)&src[x + i_src - 1]);

					t3 = _mm256_min_epu8(s0, s1);
					t1 = _mm256_cmpeq_epi8(t3, s0);
					t2 = _mm256_cmpeq_epi8(t3, s1);
					t0 = _mm256_subs_epi8(t2, t1); //upsign

					t3 = _mm256_min_epu8(s1, s2);
					t1 = _mm256_cmpeq_epi8(t3, s1);
					t2 = _mm256_cmpeq_epi8(t3, s2);
					t3 = _mm256_subs_epi8(t1, t2); //downsign

					etype = _mm256_adds_epi8(t0, t3); //edgetype

					etype = _mm256_adds_epi8(etype, c2);

					t0 = _mm256_shuffle_epi8(off, etype);//get offset

					//convert byte to short for possible overflow
					t1 = _mm256_cvtepi8_epi16(_mm256_castsi256_si128(t0));
					t2 = _mm256_cvtepi8_epi16(_mm256_extracti128_si256(t0, 1));
					t3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(s1));
					t4 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(s1, 1));

					t1 = _mm256_adds_epi16(t1, t3);
					t2 = _mm256_adds_epi16(t2, t4);
					t0 = _mm256_packus_epi16(t1, t2); //saturated
					t0 = _mm256_permute4x64_epi64(t0, 0xd8);

					if (x != end_x_r_32){
						_mm256_storeu_si256((__m256i*)(dst + x), t0);
					}
					else{
						_mm_storeu_si128((__m128i*)(dst + x), _mm256_castsi256_si128(t0));
						_mm_maskmoveu_si128(_mm256_extracti128_si256(t0, 1), mask, (char_t*)(dst + x + 16));
						break;
					}
				}
				dst += i_dst;
				src += i_src;
			}
			for (x = start_x_rn; x < end_x_rn; x += 32) {
				s0 = _mm256_loadu_si256((__m256i*)&src[x - i_src + 1]);
				s1 = _mm256_loadu_si256((__m256i*)&src[x]);
				s2 = _mm256_loadu_si256((__m256i*)&src[x + i_src - 1]);

				t3 = _mm256_min_epu8(s0, s1);
				t1 = _mm256_cmpeq_epi8(t3, s0);
				t2 = _mm256_cmpeq_epi8(t3, s1);
				t0 = _mm256_subs_epi8(t2, t1); //upsign

				t3 = _mm256_min_epu8(s1, s2);
				t1 = _mm256_cmpeq_epi8(t3, s1);
				t2 = _mm256_cmpeq_epi8(t3, s2);
				t3 = _mm256_subs_epi8(t1, t2); //downsign

				etype = _mm256_adds_epi8(t0, t3); //edgetype

				etype = _mm256_adds_epi8(etype, c2);

				t0 = _mm256_shuffle_epi8(off, etype);//get offset

				//convert byte to short for possible overflow
				t1 = _mm256_cvtepi8_epi16(_mm256_castsi256_si128(t0));
				t2 = _mm256_cvtepi8_epi16(_mm256_extracti128_si256(t0, 1));
				t3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(s1));
				t4 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(s1, 1));

				t1 = _mm256_adds_epi16(t1, t3);
				t2 = _mm256_adds_epi16(t2, t4);
				t0 = _mm256_packus_epi16(t1, t2); //saturated
				t0 = _mm256_permute4x64_epi64(t0, 0xd8);

				if (x != end_x_rn_32){
					_mm256_storeu_si256((__m256i*)(dst + x), t0);
				}
				else{
					mask = _mm_load_si128((__m128i*)(sao_mask_32[end_x_rn - end_x_rn_32 - 26]));
					_mm_storeu_si128((__m128i*)(dst + x), _mm256_castsi256_si128(t0));
					_mm_maskmoveu_si128(_mm256_extracti128_si256(t0, 1), mask, (char_t*)(dst + x + 16));
					break;
				}
			}
		}
    }
        break;
	case SAO_TYPE_BO: {
		if (smb_pix_width == SAO_SHIFT_PIX_NUM){
			__m128i r0, r1, r2, r3, off0, off1, off2, off3;
			__m128i t0, t1, t2, t3, src0, src1;
			__m128i mask;
			__m128i zero = _mm_setzero_si128();
			int shift_bo = sample_bit_depth - NUM_SAO_BO_CLASSES_IN_BIT;

			r0 = _mm_set1_epi16(saoBlkParam->startBand);
			r1 = _mm_set1_epi16((saoBlkParam->startBand + 1)%32);
			r2 = _mm_set1_epi16(saoBlkParam->startBand2);
			r3 = _mm_set1_epi16((saoBlkParam->startBand2 + 1)%32);
			off0 = _mm_set1_epi16(saoBlkParam->offset[0]);
			off1 = _mm_set1_epi16(saoBlkParam->offset[1]);
			off2 = _mm_set1_epi16(saoBlkParam->offset[2]);
			off3 = _mm_set1_epi16(saoBlkParam->offset[3]);

			mask = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1);

			for (y = 0; y < smb_pix_height; y++) {
				src0 = _mm_loadl_epi64((__m128i*)src);
				src0 = _mm_unpacklo_epi8(src0, zero);
				src1 = _mm_srai_epi16(src0, shift_bo);

				t0 = _mm_cmpeq_epi16(src1, r0);
				t1 = _mm_cmpeq_epi16(src1, r1);
				t2 = _mm_cmpeq_epi16(src1, r2);
				t3 = _mm_cmpeq_epi16(src1, r3);

				t0 = _mm_and_si128(t0, off0);
				t1 = _mm_and_si128(t1, off1);
				t2 = _mm_and_si128(t2, off2);
				t3 = _mm_and_si128(t3, off3);
				t0 = _mm_or_si128(t0, t1);
				t2 = _mm_or_si128(t2, t3);
				t0 = _mm_or_si128(t0, t2);      // get offsets

				t1 = _mm_adds_epi16(t0, src0);
				src0 = _mm_packus_epi16(t1, zero); //saturated

				_mm_maskmoveu_si128(src0, mask, (char_t*)(dst));

				dst += i_dst;
				src += i_src;
			}
		}
		else{
			__m256i r0, r1, r2, r3, off0, off1, off2, off3;
			__m256i t0, t1, t2, t3, t4, src0, src1;
			__m128i mask = _mm_setzero_si128();
            __m256i shift_mask = _mm256_set1_epi8(31);

            end_x = smb_pix_width - 31;

			r0 = _mm256_set1_epi8(saoBlkParam->startBand);
			r1 = _mm256_set1_epi8((saoBlkParam->startBand + 1)%32);
			r2 = _mm256_set1_epi8(saoBlkParam->startBand2);
			r3 = _mm256_set1_epi8((saoBlkParam->startBand2 + 1)%32);
			off0 = _mm256_set1_epi8(saoBlkParam->offset[0]);
			off1 = _mm256_set1_epi8(saoBlkParam->offset[1]);
			off2 = _mm256_set1_epi8(saoBlkParam->offset[2]);
			off3 = _mm256_set1_epi8(saoBlkParam->offset[3]);

            if (smb_pix_width > 32) {
                mask = _mm_load_si128((__m128i*)(sao_mask_32[smb_pix_width - 32 - 26]));
            }

			for (y = 0; y < smb_pix_height; y++) {
                for (x = 0; x < smb_pix_width; x += 32){
					src0 = _mm256_loadu_si256((__m256i*)&src[x]);
                    src1 = _mm256_srli_epi16(src0, 3);
                    src1 = _mm256_and_si256(src1, shift_mask);

					t0 = _mm256_cmpeq_epi8(src1, r0);
					t1 = _mm256_cmpeq_epi8(src1, r1);
					t2 = _mm256_cmpeq_epi8(src1, r2);
					t3 = _mm256_cmpeq_epi8(src1, r3);

					t0 = _mm256_and_si256(t0, off0);
					t1 = _mm256_and_si256(t1, off1);
					t2 = _mm256_and_si256(t2, off2);
					t3 = _mm256_and_si256(t3, off3);
					t0 = _mm256_or_si256(t0, t1);
					t2 = _mm256_or_si256(t2, t3);
					t0 = _mm256_or_si256(t0, t2);
					//src0 = _mm_adds_epi8(src0, t0);
					//convert byte to short for possible overflow
					t1 = _mm256_cvtepi8_epi16(_mm256_castsi256_si128(t0));
					t2 = _mm256_cvtepi8_epi16(_mm256_extracti128_si256(t0, 1));
					t3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(src0));
					t4 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(src0, 1));

					t1 = _mm256_add_epi16(t1, t3);
					t2 = _mm256_add_epi16(t2, t4);
					t0 = _mm256_packus_epi16(t1, t2); //saturated
					t0 = _mm256_permute4x64_epi64(t0, 0xd8);

                    if (smb_pix_width - x >= 32){
                        _mm256_storeu_si256((__m256i*)(dst + x), t0);
                    } else{
                        _mm_storeu_si128((__m128i*)(dst + x), _mm256_castsi256_si128(t0));
                        _mm_maskmoveu_si128(_mm256_extracti128_si256(t0, 1), mask, (char_t*)(dst + x + 16));
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