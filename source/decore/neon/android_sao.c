#include "android_sao.h"
#include "android.h"


ALIGNED_32(pel_t sao_mask[15 * 16]) = {
    -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0
};

void SAO_EO_0_c(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, int start_x, int end_x, int mb_height, char_t* mask){
    int x, y;
    int diff, leftsign, rightsign, edgetype;

    for (y = 0; y < mb_height; y++) {
        diff = src[start_x] - src[start_x - 1];
        leftsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
        for (x = start_x; x < end_x; x++) {
            diff = src[x] - src[x + 1];
            rightsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
            edgetype = leftsign + rightsign;
            leftsign = -rightsign;
            dst[x] = Clip3(0, 255, src[x] + offset[edgetype + 2]);
        }
        dst += dst_stride;
        src += src_stride;
    }
}

void SAO_EO_90_c(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, int start_y, int end_y, int mb_width){
    int x, y;
    int diff, upsign, downsign, edgetype;
    pel_t *dst_base = dst;
    pel_t *src_base = src;
    for (x = 0; x < mb_width; x++) {
        src = src_base + start_y * src_stride;
        diff = src[0] - src[-src_stride];
        upsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
        dst = dst_base + start_y * dst_stride;
        for (y = start_y; y < end_y; y++) {
            diff = src[0] - src[src_stride];
            downsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
            edgetype = downsign + upsign;
            upsign = -downsign;
            *dst = Clip3(0, 255, src[0] + offset[edgetype + 2]);
            dst += dst_stride;
            src += src_stride;
        }
        dst_base++;
        src_base++;
    }
}

void SAO_EO_135_c(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, char_t* mask, int mb_height, int start_x_r0, int end_x_r0, int start_x_r, int end_x_r, int start_x_rn, int end_x_rn){
    int diff, upsign, downsign, edgetype;
    int x, y;
    char_t signupline[65];
    int reg = 0;

    //init the line buffer
    for (x = start_x_r + 1; x < end_x_r + 1; x++) {
        diff = src[x + src_stride] - src[x - 1];
        upsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
        signupline[x] = upsign;
    }
    //first row
    for (x = start_x_r0; x < end_x_r0; x++) {
        diff = src[x] - src[x - 1 - src_stride];
        upsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
        edgetype = upsign - signupline[x + 1];
        dst[x] = Clip3(0, 255, src[x] + offset[edgetype + 2]);
    }
    dst += dst_stride;
    src += src_stride;

    //middle rows
    for (y = 1; y < mb_height - 1; y++) {
        for (x = start_x_r; x < end_x_r; x++) {
            if (x == start_x_r) {
                diff = src[x] - src[x - 1 - src_stride];
                upsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
                signupline[x] = upsign;
            }
            diff = src[x] - src[x + 1 + src_stride];
            downsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
            edgetype = downsign + signupline[x];
            dst[x] = Clip3(0, 255, src[x] + offset[edgetype + 2]);
            signupline[x] = reg;
            reg = -downsign;
        }
        dst += dst_stride;
        src += src_stride;
    }
    //last row
    for (x = start_x_rn; x < end_x_rn; x++) {
        if (x == start_x_r) {
            diff = src[x] - src[x - 1 - src_stride];
            upsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
            signupline[x] = upsign;
        }
        diff = src[x] - src[x + 1 + src_stride];
        downsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
        edgetype = downsign + signupline[x];
        dst[x] = Clip3(0, 255, src[x] + offset[edgetype + 2]);
    }
}

void SAO_EO_45_c(pel_t* src, pel_t* dst, int i_src, int i_dst, int* offset, char_t* mask, int mb_height, int start_x_r0, int end_x_r0, int start_x_r, int end_x_r, int start_x_rn, int end_x_rn){
    int diff, upsign, downsign, edgetype;
    int x, y;
    char_t signupline[65], *signupline1;
    int reg = 0;

    signupline1 = signupline + 1;
    for (x = start_x_r - 1; x < end_x_r - 1; x++) {
        diff = src[x + i_src] - src[x + 1];
        upsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
        signupline1[x] = upsign;
    }
    //first row
    for (x = start_x_r0; x < end_x_r0; x++) {
        diff = src[x] - src[x + 1 - i_src];
        upsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
        edgetype = upsign - signupline1[x - 1];
        dst[x] = Clip3(0, 255, src[x] + offset[edgetype + 2]);
    }
    dst += i_dst;
    src += i_src;

    //middle rows
    for (y = 1; y < mb_height - 1; y++) {
        for (x = start_x_r; x < end_x_r; x++) {
            if (x == end_x_r - 1) {
                diff = src[x] - src[x + 1 - i_src];
                upsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
                signupline1[x] = upsign;
            }
            diff = src[x] - src[x - 1 + i_src];
            downsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
            edgetype = downsign + signupline1[x];
            dst[x] = Clip3(0, 255, src[x] + offset[edgetype + 2]);
            signupline1[x - 1] = -downsign;
        }
        dst += i_dst;
        src += i_src;
    }
    for (x = start_x_rn; x < end_x_rn; x++) {
        if (x == end_x_r - 1) {
            diff = src[x] - src[x + 1 - i_src];
            upsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
            signupline1[x] = upsign;
        }
        diff = src[x] - src[x - 1 + i_src];
        downsign = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
        edgetype = downsign + signupline1[x];
        dst[x] = Clip3(0, 255, src[x] + offset[edgetype + 2]);
    }
}

void SAO_BO_c(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, int start_band, int start_band_2, int mb_width, int mb_height){
    pel_t *dst_base = dst;
    pel_t *src_base = src;
    int x, y;

    for (x = 0; x < mb_width; x++) {
        dst = dst_base;
        src = src_base;
        for (y = 0; y < mb_height; y++) {
            int tmp = src[0] >> (8 - NUM_SAO_BO_CLASSES_IN_BIT);
            if (tmp == start_band){
                *dst = Clip3(0, 255, src[0] + offset[0]);
            }
            else if (tmp == start_band + 1)
            {
                *dst = Clip3(0, 255, src[0] + offset[1]);
            }
            else if (tmp == start_band_2)
            {
                *dst = Clip3(0, 255, src[0] + offset[2]);
            }
            else if (tmp == start_band_2 + 1)
            {
                *dst = Clip3(0, 255, src[0] + offset[3]);
            }

            dst += dst_stride;
            src += src_stride;
        }
        dst_base++;
        src_base++;
    }
}

void SAO_on_block_neon(com_pic_t *pic_src, com_pic_t *pic_dst, sap_param_t *saoBlkParam, int compIdx, int smb_index, int pix_y, int pix_x, int smb_pix_height, int smb_pix_width,
    int smb_available_left, int smb_available_right, int smb_available_up, int smb_available_down, int smb_available_upleft,
    int smb_available_upright, int smb_available_leftdown, int smb_available_rightdwon, int sample_bit_depth){
    int type;
    int start_x, end_x, start_y, end_y;
    int start_x_r0, end_x_r0, start_x_r, end_x_r, start_x_rn, end_x_rn;
    pel_t *src, *dst;
    int i_src, i_dst;

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
        start_x = smb_available_left ? 0 : 1;
        end_x = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
        SAO_EO_0_neon(src, dst, i_src, i_dst, saoBlkParam->offset, start_x, end_x, smb_pix_height, sao_mask);
        break;
    }
    case SAO_TYPE_EO_90: {
        start_y = smb_available_up ? 0 : 1;
        end_y = smb_available_down ? smb_pix_height : (smb_pix_height - 1);
        SAO_EO_90_neon(src, dst, i_src, i_dst, saoBlkParam->offset, start_y, end_y, smb_pix_width);
        break;
    }
    case SAO_TYPE_EO_135: {
        start_x_r0 = smb_available_upleft ? 0 : 1;
        end_x_r0 = smb_available_up ? (smb_available_right ? smb_pix_width : (smb_pix_width - 1)) : 1;
        start_x_r = smb_available_left ? 0 : 1;
        end_x_r = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
        start_x_rn = smb_available_down ? (smb_available_left ? 0 : 1) : (smb_pix_width - 1);
        end_x_rn = smb_available_rightdwon ? smb_pix_width : (smb_pix_width - 1);
        SAO_EO_135_neon(src, dst, i_src, i_dst, saoBlkParam->offset, sao_mask, smb_pix_height, start_x_r0, end_x_r0, start_x_r, end_x_r, start_x_rn, end_x_rn);
        break;
    }
    case SAO_TYPE_EO_45: {
        start_x_r0 = smb_available_up ? (smb_available_left ? 0 : 1) : (smb_pix_width - 1);
        end_x_r0 = smb_available_upright ? smb_pix_width : (smb_pix_width - 1);
        start_x_r = smb_available_left ? 0 : 1;
        end_x_r = smb_available_right ? smb_pix_width : (smb_pix_width - 1);
        start_x_rn = smb_available_leftdown ? 0 : 1;
        end_x_rn = smb_available_down ? (smb_available_right ? smb_pix_width : (smb_pix_width - 1)) : 1;
        SAO_EO_45_neon(src, dst, i_src, i_dst, saoBlkParam->offset, sao_mask, smb_pix_height, start_x_r0, end_x_r0, start_x_r, end_x_r, start_x_rn, end_x_rn);
        break;
    }
    case SAO_TYPE_BO: {
        SAO_BO_neon(src, dst, i_src, i_dst, saoBlkParam->offset, saoBlkParam->startBand, saoBlkParam->startBand2, smb_pix_width, smb_pix_height);
        break;
    }
    default:{
        printf("Not a supported SAO types\n");
        assert(0);
        exit(-1);
    }
    }
}