#ifndef __NEON_H__
#define __NEON_H__

#include "../common.h"
#include "../commonStructures.h"

void cpy_pel_to_uchar_neon128(const pel_t *src, int i_src, uchar_t *dst, int i_dst, int width, int height);
void add_pel_clip_neon128(const pel_t *src1, int i_src1, const resi_t *src2, int i_src2, pel_t *dst, int i_dst, int width, int height ,int bit_depth);
void avg_pel_neon128(pel_t *dst, int i_dst, pel_t *src1, int i_src1, pel_t *src2, int i_src2, int width, int height);
void padding_rows_neon128(pel_t *src, int i_src, int width, int height, int start, int rows,int pad);
void padding_rows_lr_neon128(pel_t *src, int i_src, int width, int height, int start, int rows,int pad);
void com_if_filter_hor_4_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_hor_8_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_4_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_ver_8_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, char_t const *coeff, int max_val);
void com_if_filter_hor_ver_4_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coef_x, const char_t *coef_y, int max_val);
void com_if_filter_hor_ver_8_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height, const char_t *coeff_h, const char_t *coeff_v, int max_val);
void alf_flt_one_block_neon128(pel_t *imgRes, pel_t *imgPad, int stride, int isChroma, int yPos, int lcu_height, int xPos, int lcu_width, int *coef, int sample_bit_depth, int isAboveAvail, int isBelowAvail);

void SAO_on_block_neon(com_pic_t *pic_src, com_pic_t *pic_dst, sap_param_t *saoBlkParam, int compIdx, int smb_index, int pix_y, int pix_x, int smb_pix_height, int smb_pix_width,
    int smb_available_left, int smb_available_right, int smb_available_up, int smb_available_down, int smb_available_upleft,
    int smb_available_upright, int smb_available_leftdown, int smb_available_rightdwon, int sample_bit_depth);

void idct_4x4_shift12_neon(coef_t *blk, int shift, int clip);
void idct_8x8_shift12_neon(coef_t *blk, int shift, int clip);
void idct_16x16_shift12_neon(coef_t *blk, int shift, int clip);
void idct_4x16_shift12_neon(coef_t *blk, int shift, int clip);	
void idct_16x4_shift12_neon(coef_t *blk, int shift, int clip);
void idct_32x32_shift12_neon(coef_t *blk, int shift, int clip);
void idct_32x32_shift11_neon(coef_t *blk, int shift, int clip);
void idct_8x32_shift12_neon(coef_t *blk, int shift, int clip);
void idct_8x32_shift11_neon(coef_t *blk, int shift, int clip);
void idct_32x8_shift12_neon(coef_t *blk, int shift, int clip);
void idct_32x8_shift11_neon(coef_t *blk, int shift, int clip);

void inv_2nd_trans_hor_neon(coef_t *blk, int i_blk, int shift, const i16s_t coef[4][4]);
void inv_2nd_trans_ver_neon(coef_t *blk, int i_blk, int shift, const i16s_t coef[4][4]);
void inv_2nd_trans_neon(coef_t *blk, int i_blk, int shift, int clip_depth, const i16s_t coef[4][4]);

void inv_wavelet_B64_neon(coef_t *blk);
void inv_wavelet_NSQT_Hor_neon(coef_t *blk);
void inv_wavelet_NSQT_Ver_neon(coef_t *blk);

void deblock_edge_ver_neon(pel_t *SrcPtr, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_hor_neon(pel_t *SrcPtr, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_ver_c_neon(pel_t *SrcPtrU, pel_t *SrcPtrV, int stride, int Alpha, int Beta, uchar_t *flt_flag);
void deblock_edge_hor_c_neon(pel_t *SrcPtrU, pel_t *SrcPtrV, int stride, int Alpha, int Beta, uchar_t *flt_flag);

void xPredIntraVertAdi_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight);
void xPredIntraHorAdi_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight);
void xPredIntraDCAdi_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int bAboveAvail, int bLeftAvail, int sample_bit_depth);
void xPredIntraPlaneAdi_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int sample_bit_depth);
void xPredIntraBiAdi_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int iWidth, int iHeight, int sample_bit_depth);

void xPredIntraAngAdi_X_3_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_5_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_7_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_9_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_11_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void xPredIntraAngAdi_X_4_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_6_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_8_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_X_10_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void xPredIntraAngAdi_Y_25_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_27_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_29_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_31_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void xPredIntraAngAdi_Y_26_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_28_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_30_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_Y_32_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void xPredIntraAngAdi_XY_14_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_16_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_18_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_20_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);
void xPredIntraAngAdi_XY_22_neon128(pel_t *pSrc, pel_t *dst, int i_dst, int uiDirMode, int iWidth, int iHeight);

void com_if_filter_cpy_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy2_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy4_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy6_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy8_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy12_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy32_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);
void com_if_filter_cpy16_neon128(const pel_t *src, int i_src, pel_t *dst, int i_dst, int width, int height);

#endif // #ifndef __NEON_H__