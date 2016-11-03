#include "android.h"

static void cpy_pel_to_uchar_neon(const pel_t *src, int i_src, uchar_t *dst, int i_dst, int width, int height, int bit_size)
{
    int i;
    for (i = 0; i < height; i++) {
        memcpy(dst, src, width * sizeof(uchar_t));
        dst += i_dst;
        src += i_src;
    }
}

void com_init_neon128()
{
    int i;

    g_funs_handle.cpy_pel_to_uchar = cpy_pel_to_uchar_neon;
    g_funs_handle.add_pel_clip     = add_pel_clip_neon128;
	g_funs_handle.avg_pel          = avg_pel_neon128;
	g_funs_handle.padding_rows     = padding_rows_neon128;
	g_funs_handle.padding_rows_lr  = padding_rows_lr_neon128;
    g_funs_handle.sao_flt          = SAO_on_block_neon;
	g_funs_handle.alf_flt          = alf_flt_one_block_neon128;

	g_funs_handle.idct_sqt[0] = idct_4x4_shift12_neon;
	g_funs_handle.idct_sqt[1] = idct_8x8_shift12_neon;
	g_funs_handle.idct_sqt[2] = idct_16x16_shift12_neon;
	g_funs_handle.idct_sqt[3] = idct_32x32_shift12_neon;
	g_funs_handle.idct_sqt[4] = idct_32x32_shift11_neon;

	g_funs_handle.idct_hor[0] = idct_16x4_shift12_neon;
	g_funs_handle.idct_hor[1] = idct_32x8_shift12_neon;
	g_funs_handle.idct_hor[2] = idct_32x8_shift11_neon;

	g_funs_handle.idct_ver[0] = idct_4x16_shift12_neon;
	g_funs_handle.idct_ver[1] = idct_8x32_shift12_neon;
	g_funs_handle.idct_ver[2] = idct_8x32_shift11_neon;
	
	g_funs_handle.inv_2nd_hor = inv_2nd_trans_hor_neon;
	g_funs_handle.inv_2nd_ver = inv_2nd_trans_ver_neon;
	g_funs_handle.inv_2nd = inv_2nd_trans_neon;

	g_funs_handle.inv_wavelet = inv_wavelet_B64_neon;
	g_funs_handle.inv_wavelet_hor = inv_wavelet_NSQT_Hor_neon;
	g_funs_handle.inv_wavelet_ver = inv_wavelet_NSQT_Ver_neon;

    for (i = 0; i < 16; i++) {
		g_funs_handle.ipcpy[IPFILTER_EXT_4][i] = com_if_filter_cpy_neon128;
		g_funs_handle.ipcpy[IPFILTER_EXT_8][i] = com_if_filter_cpy_neon128;
        g_funs_handle.ipflt[IPFILTER_H_4][i] = com_if_filter_hor_4_neon128;
        g_funs_handle.ipflt[IPFILTER_H_8][i] = com_if_filter_hor_8_neon128;
        g_funs_handle.ipflt[IPFILTER_V_4][i] = com_if_filter_ver_4_neon128;
        g_funs_handle.ipflt[IPFILTER_V_8][i] = com_if_filter_ver_8_neon128;
        g_funs_handle.ipflt_ext[IPFILTER_EXT_4][i] = com_if_filter_hor_ver_4_neon128;
        g_funs_handle.ipflt_ext[IPFILTER_EXT_8][i] = com_if_filter_hor_ver_8_neon128;
    }

	g_funs_handle.ipcpy[IPFILTER_EXT_4][0] = com_if_filter_cpy2_neon128;
	g_funs_handle.ipcpy[IPFILTER_EXT_4][1] = com_if_filter_cpy4_neon128;
	g_funs_handle.ipcpy[IPFILTER_EXT_4][2] = com_if_filter_cpy6_neon128;
	g_funs_handle.ipcpy[IPFILTER_EXT_4][3] = com_if_filter_cpy8_neon128;
	g_funs_handle.ipcpy[IPFILTER_EXT_4][5] = com_if_filter_cpy12_neon128;
	g_funs_handle.ipcpy[IPFILTER_EXT_4][7] = com_if_filter_cpy16_neon128;

	g_funs_handle.ipcpy[IPFILTER_EXT_8][0] = com_if_filter_cpy4_neon128;
	g_funs_handle.ipcpy[IPFILTER_EXT_8][1] = com_if_filter_cpy8_neon128;
	g_funs_handle.ipcpy[IPFILTER_EXT_8][2] = com_if_filter_cpy12_neon128;
	g_funs_handle.ipcpy[IPFILTER_EXT_8][3] = com_if_filter_cpy16_neon128;

    g_funs_handle.deblock_edge  [0] = deblock_edge_ver_neon;
    g_funs_handle.deblock_edge  [1] = deblock_edge_hor_neon;
    g_funs_handle.deblock_edge_c[0] = deblock_edge_ver_c_neon;
    g_funs_handle.deblock_edge_c[1] = deblock_edge_hor_c_neon;

	g_funs_handle.intra_pred_ver = xPredIntraVertAdi_neon128;
	g_funs_handle.intra_pred_hor = xPredIntraHorAdi_neon128;
	g_funs_handle.intra_pred_dc = xPredIntraDCAdi_neon128;
	g_funs_handle.intra_pred_plane = xPredIntraPlaneAdi_neon128;
	g_funs_handle.intra_pred_bi = xPredIntraBiAdi_neon128;

	g_funs_handle.intra_pred_ang_x[3 - ANG_X_OFFSET] = xPredIntraAngAdi_X_3_neon128;
	g_funs_handle.intra_pred_ang_x[5 - ANG_X_OFFSET] = xPredIntraAngAdi_X_5_neon128;
	g_funs_handle.intra_pred_ang_x[7 - ANG_X_OFFSET] = xPredIntraAngAdi_X_7_neon128;
	g_funs_handle.intra_pred_ang_x[9 - ANG_X_OFFSET] = xPredIntraAngAdi_X_9_neon128;
	g_funs_handle.intra_pred_ang_x[11 - ANG_X_OFFSET] = xPredIntraAngAdi_X_11_neon128;

	g_funs_handle.intra_pred_ang_x[4 - ANG_X_OFFSET] = xPredIntraAngAdi_X_4_neon128;
	g_funs_handle.intra_pred_ang_x[6 - ANG_X_OFFSET] = xPredIntraAngAdi_X_6_neon128;
	g_funs_handle.intra_pred_ang_x[8 - ANG_X_OFFSET] = xPredIntraAngAdi_X_8_neon128;
	g_funs_handle.intra_pred_ang_x[10 - ANG_X_OFFSET] = xPredIntraAngAdi_X_10_neon128;

	g_funs_handle.intra_pred_ang_y[25 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_25_neon128;
	g_funs_handle.intra_pred_ang_y[27 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_27_neon128;
	g_funs_handle.intra_pred_ang_y[29 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_29_neon128;
	g_funs_handle.intra_pred_ang_y[31 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_31_neon128;

	g_funs_handle.intra_pred_ang_y[26 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_26_neon128;
	g_funs_handle.intra_pred_ang_y[28 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_28_neon128;
	g_funs_handle.intra_pred_ang_y[30 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_30_neon128;
	g_funs_handle.intra_pred_ang_y[32 - ANG_Y_OFFSET] = xPredIntraAngAdi_Y_32_neon128;

	g_funs_handle.intra_pred_ang_xy[14 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_14_neon128;
	g_funs_handle.intra_pred_ang_xy[16 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_16_neon128;
	g_funs_handle.intra_pred_ang_xy[18 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_18_neon128;
	g_funs_handle.intra_pred_ang_xy[20 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_20_neon128;
	g_funs_handle.intra_pred_ang_xy[22 - ANG_XY_OFFSET] = xPredIntraAngAdi_XY_22_neon128;

}

