/*****************************************************************************
*  Copyright (C) 2016 uavs2dec project,
*  National Engineering Laboratory for Video Technology(Shenzhen),
*  Digital Media R&D Center at Peking University Shenzhen Graduate School, China
*  Project Leader: Ronggang Wang <rgwang@pkusz.edu.cn>
*
*  Main Authors: Zhenyu Wang <wangzhenyu@pkusz.edu.cn>, Kui Fan <kuifan@pku.edu.cn>
*               Shenghao Zhang <1219759986@qq.com>�� Bingjie Han, Kaili Yao, Hongbin Cao,  Yueming Wang,
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

#ifndef _DEFINES_H_
#define _DEFINES_H_


#define MT_FRM_MAXTHREADS 8
#define MT_REC_MATTHREADS 1

#define COMPILE_10BIT    1

/* ---------------------------------------------------------------------------
* basic type define
*/
typedef signed long long   i64s_t;
typedef signed int         i32s_t;
typedef signed short       i16s_t;
typedef signed char        char_t;
typedef unsigned long long i64u_t;
typedef unsigned int       i32u_t;
typedef unsigned short     i16u_t;
typedef unsigned char      uchar_t;

#if defined(__ANDROID__)
#else
#ifndef _INTPTR_T_DEFINED
//typedef i64s_t intptr_t;
#endif
#endif

#if COMPILE_10BIT
typedef i16u_t     pel_t;
#else
typedef uchar_t    pel_t;
#endif

typedef i16s_t    coef_t;
typedef i16s_t    resi_t;

typedef const i32s_t tab_i32s_t;
typedef const i16s_t tab_i16s_t;
typedef const i16u_t tab_i16u_t;
typedef const char_t tab_char_t;
typedef const uchar_t tab_uchar_t;

#define RESERVED_PROFILE_ID      0x24
#define BASELINE_PROFILE         64
#define BASELINE10_PROFILE       82

//////////////////// prediction techniques /////////////////////////////

#define DIRECTION                4
#define DS_FORWARD               4
#define DS_BACKWARD              2
#define DS_SYM                   3
#define DS_BID                   1

#define MH_PSKIP                 1     // M3256
#define MH_PSKIP_NUM             4
#define NUM_OFFSET               0
#define BID_P_FST                1
#define BID_P_SND                2
#define FW_P_FST                 3
#define FW_P_SND                 4
#define WPM_NUM                  3

#define DMH_MODE_NUM             5     // Number of DMH mode


//////////////////// reference picture management /////////////////////////////
#define REF_MAXBUFFER            7

//////////////////// coefficient coding /////////////////////////////
#define CG_SIZE                  16    // M3035 size of an coefficient group, 4x4

#define SWAP(x,y)                {(y)=(y)^(x);(x)=(y)^(x);(y)=(x)^(y);}
               
//////////////////// encoder optimization //////////////////////////////////////////////

#define TH                       2

////////////////////////SAO//////////////////////////////////////////////////////////////

#define NUM_BO_OFFSET             32
#define MAX_NUM_SAO_CLASSES       5
#define NUM_SAO_BO_CLASSES_LOG2   5
#define NUM_SAO_BO_CLASSES_IN_BIT 5
#define NUM_SAO_EO_TYPES_LOG2 2
#define NUM_SAO_BO_CLASSES (1<<NUM_SAO_BO_CLASSES_LOG2)
#define SAO_RATE_THR            0.75
#define SAO_SHIFT_PIX_NUM         4

/*
*************************************************************************************
* AVS2 macros end
*
*************************************************************************************
*/

#define CHROMA                    1
#define LUMA_8x8                  2
#define NUM_BLOCK_TYPES           8

#define clamp(a,b,c) ( (a)<(b) ? (b) : ((a)>(c)?(c):(a)) )    //!< clamp a to the range of [b;c]

// ---------------------------------------------------------------------------------
// FLAGS and DEFINES for new chroma intra prediction, Dzung Hoang
// Threshold values to zero out quantized transform coefficients.
// Recommend that _CHROMA_COEFF_COST_ be low to improve chroma quality
#define _LUMA_COEFF_COST_         4 //!< threshold for luma coeffs

#define IMG_PAD_SIZE              64   //!< Number of pixels padded around the reference frame (>=4)



#define absm(A) ((A)<(0) ? (-(A)):(A)) //!< abs macro, faster than procedure
#define MAX_VALUE                999999   //!< used for start value for some variables

#define Clip1(a)            ((a)>255?255:((a)<0?0:(a)))
#define Clip3(min,max,val)  (((val)<(min))?(min):(((val)>(max))?(max):(val)))

// ---------------------------------------------------------------------------------

// block size of block transformed by AVS
#define SKIPDIRECT                0
#define P2NX2N                    1
#define P2NXN                     2
#define PNX2N                     3
#define PHOR_UP                   4
#define PHOR_DOWN                 5
#define PVER_LEFT                 6
#define PVER_RIGHT                7
#define PNXN                      8
#define I8MB                      9
#define I16MB                     10
#define InNxNMB                   12
#define INxnNMB                   13
#define MAXMODE                   14   // add yuqh 20130824

#define IS_INTRA(MB)                  ((MB)->cuType==I8MB||(MB)->cuType==I16MB||(MB)->cuType==InNxNMB ||(MB)->cuType==INxnNMB)
#define IS_INTER(MB)                  ((MB)->cuType!=I8MB && (MB)->cuType!=I16MB&&(MB)->cuType!=InNxNMB &&(MB)->cuType!=INxnNMB)
#define IS_INTERMV(MB)                ((MB)->cuType!=I8MB && (MB)->cuType!=I16MB &&(MB)->cuType!=InNxNMB &&(MB)->cuType!=INxnNMB&& (MB)->cuType!=0)


#define IS_B_SKIP(MB)                ((MB)->cuType==SKIPDIRECT     && (h_dec->type==B_IMG))
#define IS_P_SKIP(MB)                ((MB)->cuType==SKIPDIRECT     && (((h_dec->type==F_IMG))||((h_dec->type==P_IMG))))

#define IS_B_SKIP_REC(MB)                ((MB)->cuType==SKIPDIRECT     && (rec->type==B_IMG))
#define IS_P_SKIP_REC(MB)                ((MB)->cuType==SKIPDIRECT     && (((rec->type==F_IMG))||((rec->type==P_IMG))))

// Quantization parameter range
#define MAX_QP                       79
#define SHIFT_QP                     11

// Picture types
#define B_IMG                        2   //!< B frame
#define I_IMG                        0   //!< I frame
#define P_IMG                        1   //!< P frame
#define F_IMG                        4  //!< F frame

#define BACKGROUND_IMG               3
#define BP_IMG                       5


// Direct Mode types
#define MIN_BLOCK_SIZE               4
#define MIN_CU_SIZE_IN_BIT           3
#define MIN_BLOCK_SIZE_IN_BIT        2

#define MAX_CU_SIZE                  64
#define MAX_CU_SIZE_IN_BIT           6
#define B4X4_IN_BIT                  2
#define B8X8_IN_BIT                  3
#define B16X16_IN_BIT                4
#define B32X32_IN_BIT                5
#define B64X64_IN_BIT                6
#define NUM_INTRA_PMODE              33        //!< # luma intra prediction modes
#define NUM_MODE_FULL_RD             9         // number of luma modes for full RD search
#define NUM_INTRA_PMODE_CHROMA       5         //!< #chroma intra prediction modes

// luma intra prediction modes

#define DC_PRED                      0
#define PLANE_PRED                   1
#define BI_PRED                      2
#define VERT_PRED                    12
#define HOR_PRED                     24


// chroma intra prediction modes
#define DM_PRED_C                    0
#define DC_PRED_C                    1
#define HOR_PRED_C                   2
#define VERT_PRED_C                  3
#define BI_PRED_C                    4

#define EOS                          1         //!< End Of Sequence
#define SOP                          2                       //!< Start Of Picture

#define DECODE_MB                    1

#ifndef _WIN32
#define max(a, b)                   ((a) > (b) ? (a) : (b))  //!< Macro returning max value
#define min(a, b)                   ((a) < (b) ? (a) : (b))  //!< Macro returning min value
#endif

#define XY_MIN_PMV                   1
#define MVPRED_xy_MIN                0
#define MVPRED_L                     1
#define MVPRED_U                     2
#define MVPRED_UR                    3


#define FORWARD                      0
#define BACKWARD                     1
#define SYM                          2
#define BID                          3
#define DUAL                         4

#define INTRA                        -1

#define MULTI                        16384
#define HALF_MULTI                   8192
#define OFFSET                       14

#define MAXREF    4
#define MAXGOP    32

#endif // #if _DEFINES_H_
