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
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include <sys/timeb.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#include <process.h>
#endif
 
#include "./source/decoder/uavs2d.h"

#ifdef _WIN32
#ifdef _WIN64
#define GL_DISPLAY    0
#elif defined(__ANDROID__)
#define GL_DISPLAY    0
#else
#define GL_DISPLAY    1
#endif
#else
#define GL_DISPLAY    0
#endif


#define TEST_SPEED    0
#define FRAME_THREADS 8
#define REC_THREADS   1 // 0: single thread for aec and rec; 1: one aec thread and one rec thread.

/* ---------------------------------------------------------------------------
 * buffer size 
 */
#define READ_BUF_SIZE   1024 * 100           /* size of reading in every time */
#define TOTAL_BUF_SIZE  1024 * 1024 * 5    /* must bigger than max size of a frame */

const char IMGTYPE[8][8] = {
    " X",
    " I",
    " G",
    "GB",
    " S",
    " P",
    " F",
    " B"
};

/**
* ===========================================================================
* local variables
* ===========================================================================
*/
static void *dec_handle = NULL;      /* decoder handle */
uavs2d_lib_create_t lib_create;
uavs2d_lib_decode_t lib_decode;
uavs2d_lib_destroy_t lib_destroy;
uavs2d_lib_flush_t lib_flush;

/* ---------------------------------------------------------------------------
* Function   : get current time (in mill-second)
* Parameters :
*      [in ] : none
*      [out] : none
* Return     : current time in mill-second
* ---------------------------------------------------------------------------
*/
static __inline long long get_mdate(void)
{
    struct timeb tb;
    ftime(&tb);
    return ((long long)tb.time * 1000 + (long long)tb.millitm) * 1000;
}

/* ---------------------------------------------------------------------------
* Function   : calculate the MAD of 2 frames
* Parameters :
*      [in ] : width      - width   of frame
*            : height     - height  of frame
*            : rec        - pointer to reconstructed frame buffer
*            : rec_stride - stride  of reconstructed frame
*            : dst        - pointer to decoded frame buffer
*            : dst_stride - stride  of decoded frame
*      [out] : none
* Return     : mad of 2 frames
* ---------------------------------------------------------------------------
*/
static __inline int
cal_mad(int width, int height, unsigned char *rec, int rec_stride, unsigned char *dst, int dst_stride, int pix_size)
{
    int is_error = 0;
    int d = 0;
    int i, j, t;

    width *= pix_size;

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            t = dst[i] - rec[i];
            d += t * t;
        }
        rec += rec_stride;
        dst += dst_stride;
    }

    if (is_error) {
        fprintf(stderr, "\n");
    }

    return d;
}

/* ---------------------------------------------------------------------------
* Function   : calculate and output the psnr (only for YUV 4:2:0)
* Parameters :
*      [in ] : rec    - pointer to buffer of reconstructed picture
*            : dst    - pointer to buffer of decoded picture
*            : width  - width  of picture
*            : height - height of picture
*      [out] : none
* Return     : void
* ---------------------------------------------------------------------------
*/
static float all_psnr[3] = {0.0, 0.0, 0.0};

static void
cal_psnr(unsigned char *rec, unsigned char *dst, int width, int height, int pix_size)
{
    int stride = width;         /* stride of frame/field  (luma) */
    int size_l = width * height;/* size   of frame/field  (luma) */
    int diff;                   /* difference between decoded and reconstructed picture */
    unsigned char *p1;          /* pointer to buffer of reconstructed picture */
    unsigned char *p2;          /* pointer to buffer of decoded picture */
    float fpsnr;

    /* Y */
    p1 = rec;
    p2 = dst;
    diff = cal_mad(width, height, p1, stride, p2, stride, pix_size);
    if (diff == 0) {
        fpsnr = 0.0f;
    }
    else {
        fpsnr = (float)(10 * log10(65025.0F * size_l / diff));
    }
    fprintf(stderr, "%7.4f,  ", fpsnr);
    all_psnr[0] += fpsnr;

    width >>= 1;               /* width  of frame/field  (chroma) */
    height >>= 1;               /* height of frame/field  (chroma, with padding) */
    stride >>= 1;               /* stride of frame/field  (chroma) */

    /* U */
    p1 += size_l;
    p2 += size_l;
    diff = cal_mad(width, height, p1, stride, p2, stride, pix_size);
    if (diff == 0) {
        fpsnr = 0.0f;
    } else {
        fpsnr = (float)(10 * log10(65025.0F * size_l / diff));
    }
    fprintf(stderr, "%7.4f,  ", fpsnr);
    all_psnr[1] += fpsnr;

    /* V */
    p1 += size_l / 4;
    p2 += size_l / 4;
    diff = cal_mad(width, height, p1, stride, p2, stride, pix_size);
    if (diff == 0) {
        fpsnr = 0.0f;
    } else {
        fpsnr = (float)(10 * log10(65025.0F * size_l / diff));
    }

    fprintf(stderr, "%7.4f\n", fpsnr);
    all_psnr[2] += fpsnr;
}


/* ---------------------------------------------------------------------------
* Function   : find the next start code (I/PB picture, or sequence header/end)
* Parameters :
*      [in ] : bs_data - pointer to the bitstream buffer
*            : left    - length of bs_data
*      [out] : left    - the rest length of bs_data
* Return     : 1 if a start code is found, otherwise 0
* ---------------------------------------------------------------------------
*/
static int
find_next_start_code(const unsigned char *bs_data, int bs_len, int *left)
{
    const unsigned char *data_ptr = bs_data + 4;
    int count = bs_len - 4;

    while (count >= 4 &&
        ((*(unsigned int *)data_ptr) != 0xB6010000) && /* P/B picture */
        ((*(unsigned int *)data_ptr) != 0xB3010000) && /* I   picture */
        ((*(unsigned int *)data_ptr) != 0xB0010000) && /* sequence header */
        ((*(unsigned int *)data_ptr) != 0xB1010000)) { /* sequence end */
        data_ptr++;
        count--;
    }

    if (count >= 4) {
        *left = count;
        return 1;
    }

    return 0;
}

/* ---------------------------------------------------------------------------
* Function   : find the sequence header (call first before decoding)
* Parameters :
*      [in ] : bs_data - pointer to the bitstream buffer
*            : left    - length of bs_data
*      [out] : left    - the rest length of bs_data
* Return     : 1 if a start code is found, otherwise 0
* ---------------------------------------------------------------------------
*/
static int
find_sequence_header(const unsigned char *bs_data, int bs_len, int *left)
{
    const unsigned char *data_ptr = bs_data;
    int count = bs_len;

    while (count >= 4 && (*(unsigned int *)data_ptr) != 0xB0010000) {
        data_ptr++;
        count--;
    }

    (*left) = count;
    return (count >= 4);
}

#if GL_DISPLAY

#include "glut.h"

int screen_w, screen_h;
int display_w, display_h;
char *rgb_buf = NULL;
unsigned char* display_yuv;
HANDLE sem_ready, sem_finish;

__inline unsigned char CONVERT_ADJUST(double tmp)
{
    return (unsigned char)((tmp >= 0 && tmp <= 255) ? tmp : (tmp < 0 ? 0 : 255));
}

void yuv_2_rgb(char *y, char *u, char *v, int i_luma, int i_chroma, int SizeW, int SizeH, char* rgb24, int i_rgb)
{
    int numOfPixel = SizeW * SizeH;

    const int R = 0;
    const int G = 1;
    const int B = 2;

    for (int i = 0; i < SizeH; i++){
        int startY = i * i_luma;
        int startU = (i / 2) * (i_chroma);
        int startV = (i / 2) * (i_chroma);
        int startRGB = i * i_rgb * 3;

        for (int j = 0; j < SizeW; j++){
            int Y = startY + j;
            int V = startV + j / 2;
            int U = startU + j / 2;
            int index = startRGB + j * 3;

            rgb24[index + B] = CONVERT_ADJUST(((y[Y] & 0xff) + 1.4075 * ((u[V] & 0xff) - 128)));
            rgb24[index + G] = CONVERT_ADJUST(((y[Y] & 0xff) - 0.3455 * ((v[U] & 0xff) - 128) - 0.7169*((u[V] & 0xff) - 128)));
            rgb24[index + R] = CONVERT_ADJUST(((y[Y] & 0xff) + 1.779  * ((v[U] & 0xff) - 128)));
        }
    }
}

void display(char *y, char *u, char *v, int i_luma, int i_chroma){
    //Make picture full of window  
    //Move to(-1.0,1.0)  
    glRasterPos3f(-1.0f, 1.0f, 0);
    //Zoom, Flip  
    //glPixelZoom((float)screen_w / (float)display_w, -(float)screen_h / (float)display_h);

    yuv_2_rgb(y, u, v, i_luma, i_chroma, display_w, display_h, rgb_buf, display_w);

    glDrawPixels(display_w, display_h, GL_RGB, GL_UNSIGNED_BYTE, rgb_buf);

    //GLUT_DOUBLE  
    glutSwapBuffers();

    //GLUT_SINGLE  
    glFlush();
}

void flush(void){
    //Make picture full of window  
    //Move to(-1.0,1.0)  
    glRasterPos3f(-1.0f, 1.0f, 0);
    //Zoom, Flip  
    glPixelZoom((float)screen_w / (float)display_w, -(float)screen_h / (float)display_h);

    if (rgb_buf == NULL) {
        rgb_buf = malloc(display_w * display_h * 3);
        memset(rgb_buf, 128, display_w * display_h * 3);
    }

    glDrawPixels(display_w, display_h, GL_RGB, GL_UNSIGNED_BYTE, rgb_buf);

    //GLUT_DOUBLE  
    glutSwapBuffers();

    //GLUT_SINGLE  
    glFlush();
}

void reshape(GLsizei w, GLsizei h)
{
    screen_w = w;
    screen_h = h;
}

void timeFunc(int value){
    int size = display_w * display_h;

    WaitForSingleObject(sem_ready, INFINITE);
    display(display_yuv, display_yuv + size, display_yuv + size + size / 4, display_w, display_w / 2);
    ReleaseSemaphore(sem_finish, 1, NULL);

    glutTimerFunc(40, timeFunc, 0);
}

static unsigned __stdcall gl_video_thread(void *arg)
{
    int argc = 1;
    char *msg = "uavs2d";

    glutInit(&argc, &msg);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(display_w, display_h);
    glutCreateWindow("Simplest Video Play OpenGL");
    glutDisplayFunc(&flush);
    glutReshapeFunc(reshape);
    glutTimerFunc(40, timeFunc, 0);

    glutMainLoop();

    return 0; 
}

#endif


static __inline void write_output_frame(FILE* fp, int width, int height, int pix_size, unsigned char *buf)
{
    if (fp) {
        if (pix_size == 2) {
            static char* tmp_buf = NULL;
            unsigned short* p = (unsigned short*)buf;
            int i;
            if (tmp_buf == NULL) {
                tmp_buf = malloc(width * height);
            }
            for (i = 0; i < height * width; i++) {
                tmp_buf[i] = p[i] >> 2;
            }
            fwrite(tmp_buf, width * height, 1, fp);

            p += width * height;
            width >>= 1;
            height >>= 1;

            for (i = 0; i < height * width; i++) {
                tmp_buf[i] = p[i] >> 2;
            }
            fwrite(tmp_buf, width * height, 1, fp);

            p += width * height;

            for (i = 0; i < height * width; i++) {
                tmp_buf[i] = p[i] >> 2;
            }
            fwrite(tmp_buf, width * height, 1, fp);

        } else {
            int size = width * height;
            fwrite(buf, size * 3 / 2 * pix_size, 1, fp);
        }
    }
#if GL_DISPLAY
    display_yuv = buf;
    ReleaseSemaphore(sem_ready, 1, NULL);
    WaitForSingleObject(sem_finish, INFINITE);

#endif
}

/* ---------------------------------------------------------------------------
* Function   : testing for AVS2 decoder library
* Parameters :
*      [in ] : in_file_name  - the input  file name for reading bitstream data
*            : rec_file_name - the rec    file name created by AVS2 encoder
*            : out_file_name - the output file name for writing decoded data
*      [out] : none
* Return     : void
* ---------------------------------------------------------------------------
*/
static int
test_avs2_decoder(char *in_file_name, char *rec_file_name, char *out_file_name)
{
    unsigned char *bs_data = NULL;    /* pointer to bitstream data buffer */
    unsigned char *bs_ptr  = NULL;    /* pointer to the position of bs_data */

    unsigned char *out_buf = NULL;    /* pointer to output buffer */
    unsigned char *rec_buf = NULL;    /* pointer to rec data buffer */

    long long total_time = 0;
    long long frame_time;
    int pix_size = 1;
    int width = 0;
    int height = 0;
    long long pts_test = 0;

    avs2_frame_t dec_frame;

    FILE *in_file;                    /* file handle of input file  */

    FILE *rec_file;                   /* file handle of rec file    */
    FILE *out_file;                   /* file handle of output file */

#if OUTPUT_BG_FRM
    FILE *gb_file = fopen("background_ref.yuv", "rb");
    FILE *gb_rec_file = NULL;
    if (gb_file) {
       gb_rec_file = fopen("background_dec.yuv", "wb");
    }
#endif

    int runing = 1;        /* decode loop flag */
    int frm_count = 0;     /* number of decoded frames */
    int found_seq_hdr = 0; /* seq flag */

    /* stream controler */
    int useful_bytes;
    int left_bytes;

    /* -------------------------------------------------------
    * open/create all files
    */
    /* open stream file */
    in_file = fopen(in_file_name, "rb");
    if (in_file == NULL) {
        fprintf(stderr, "Opening input file %s error.\n", in_file_name);
        exit(-1);
    }

    /* open rec file */
    rec_file = fopen(rec_file_name, "rb");
    if (rec_file == NULL) {
        fprintf(stderr, "Opening input file %s error.\n", rec_file_name);
    }

    /* create output file */
    out_file = fopen(out_file_name, "wb");
    if (out_file == NULL) {
        fprintf(stderr, "Creating output file %s error.\n", out_file_name);
    }

    /* ------------------------------------------------------
    * alloc all buffers
    */

    /* memory for stream buffer */
    bs_data = (unsigned char *)malloc(TOTAL_BUF_SIZE);

    if (!bs_data) {
        fprintf(stderr, "Alloc memory error.\n");
        exit(-1);
    }
    bs_ptr = bs_data;

    /* init decoder */
    dec_handle = lib_create(FRAME_THREADS, REC_THREADS);

    /* find the first sequence header */
    found_seq_hdr = 0;   
    useful_bytes = left_bytes = (int)fread(bs_data, 1, READ_BUF_SIZE, in_file);

    for (;;) {
        int read_bytes;
        if (find_sequence_header(bs_ptr, useful_bytes, &left_bytes)) {
            bs_ptr += useful_bytes - left_bytes;
            useful_bytes = left_bytes;
            found_seq_hdr = 1; /* set flag */
            break;
        }
        /* read more data */
        memcpy(bs_data, bs_ptr, useful_bytes);
        read_bytes = (int)fread(bs_data + useful_bytes, 1, READ_BUF_SIZE - useful_bytes, in_file);

        useful_bytes += read_bytes;
        if (feof(in_file)) {
            break;              /* no more data */
        }
        left_bytes = useful_bytes;
    }

    if (found_seq_hdr == 0) {
        fprintf(stderr, "no sequence header found.\n");
        goto finished;
    }

#if OUTPUT_BG_FRM
#define OUT_REC_AND_READ_REF                                                                    \
    if (dec_frame.frm_type == AVS2_GB_IMG) {                                                    \
        if (gb_rec_file) fwrite(out_buf, width * height * 3 / 2 * pix_size, 1, gb_rec_file);    \
        if (gb_file)  fread(rec_buf, width * height * 3 / 2 * pix_size, 1, gb_file);            \
        if (gb_file)  cal_psnr(rec_buf, out_buf, width, height, pix_size);                      \
    } else {                                                                                    \
        write_output_frame(out_file, width, height, pix_size, out_buf);                         \
        if (rec_file) fread(rec_buf, width * height * 3 / 2 * pix_size, 1, rec_file);           \
        if (rec_file) cal_psnr(rec_buf, out_buf, width, height, pix_size);                      \
    }                                                                             
#else
#define OUT_REC_AND_READ_REF                                                                    \
    write_output_frame(out_file, width, height, pix_size, out_buf);                             \
    if (rec_file) fread(rec_buf, width * height * 3 / 2 * pix_size, 1, rec_file);               \
    if (rec_file) cal_psnr(rec_buf, out_buf, width, height, pix_size);                    
#endif

    /* ------------------------------------------------------
     * decoding...
     */
    dec_frame.i_output_type = AVS2_OUT_I420;

    while (runing) {
        if (find_next_start_code(bs_ptr, useful_bytes, &left_bytes)) {
            /* decode this frame */
            dec_frame.bs_buf = bs_ptr;
            dec_frame.bs_len = useful_bytes - left_bytes;
            dec_frame.pts = pts_test;
            frame_time = get_mdate();

            lib_decode(dec_handle, &dec_frame);
            if (bs_ptr[3] == 0xb3 || bs_ptr[3] == 0xb6) {
                pts_test++;
            }

            frame_time = get_mdate() - frame_time;
            total_time += frame_time;

            /* update buffer pointers */
            bs_ptr += dec_frame.bs_len;
            useful_bytes -= dec_frame.bs_len;

#if TEST_SPEED
#define PRINT_INFO frm_count++;
#else
#define PRINT_INFO fprintf(stderr, "%4d   %4lld  (%s)  %2d  ", frm_count++, dec_frame.pts, IMGTYPE[dec_frame.frm_type], dec_frame.qp);
#endif
            switch (dec_frame.dec_stats) {
            case AVS2_TYPE_DECODED:   /* decode one frame */
                PRINT_INFO;
                OUT_REC_AND_READ_REF;
                break;
            case AVS2_TYPE_ERROR:     /* error, current or next frame was not decoded */
                PRINT_INFO;
                OUT_REC_AND_READ_REF;
                fprintf(stderr, "!!!ERROR !!!\n");
                break;
            case AVS2_TYPE_DROP:     /* error, current or next frame was not decoded */
                fprintf(stderr, "!!!DROP non-RA frame !!!\n");
                break;
            case AVS2_TYPE_SEQ:       /* sequence header was decoded */
                if (width != dec_frame.info.img_width || height != dec_frame.info.img_height) {
                    pix_size = dec_frame.info.output_bit_depth == 8 ? 1 : 2;
                    /* resize image buffer */
                    width  = dec_frame.info.img_width;
                    height = dec_frame.info.img_height;

#if GL_DISPLAY
                    display_w = width;
                    display_h = height;
                    sem_ready  = CreateSemaphore(NULL, 0, 1, NULL);
                    sem_finish = CreateSemaphore(NULL, 0, 1, NULL);

                    rgb_buf = malloc(display_w * display_h * 3);

                    _beginthreadex(NULL, 0, gl_video_thread, NULL, 0, NULL);
#endif
                    if (width <= 0 || height <= 0) {
                        fprintf(stderr, "Error frame size: %d x %d.\n", width, height);
                        goto finished;
                    }
                    fprintf(stderr, "Resized frame buffer to %dx%d\n", width, height);
                    fprintf(stderr, " index  pts  type  qp   psnr(Y)   psnr(U)   psnr(V)\n");
                    fprintf(stderr, "--------------------------------------------------\n");

                    /* free old output buffer and allocate the new buffer */
                    if (out_buf) {
                        free(out_buf);
                    }
                    out_buf = (unsigned char *)malloc(width * height * 3 / 2 * pix_size);

                    if (out_buf == NULL) {
                        fprintf(stderr, "Alloc memory error.\n");
                        goto finished;
                    } else {
                        dec_frame.p_buf_y = out_buf;
                        dec_frame.p_buf_u = dec_frame.p_buf_y + width * height * pix_size;
                        dec_frame.p_buf_v = dec_frame.p_buf_u + width * height / 4 * pix_size;
                        dec_frame.i_stride = width * pix_size;
                        dec_frame.i_stridec = width / 2 * pix_size;
                    }

                    if (rec_buf) {
                        free(rec_buf);
                    }
                    rec_buf = (unsigned char *)malloc(width * height * 3 / 2 * pix_size);

                    if (rec_buf == NULL) {
                        fprintf(stderr, "Alloc memory error.\n");
                        goto finished;
                    }
                }
                break;
            }
        } else {
            /* need more data from file */
readagain:
            if (!feof(in_file)) {
                int read_bytes;
                /* read more data */
                if (TOTAL_BUF_SIZE - useful_bytes > READ_BUF_SIZE && TOTAL_BUF_SIZE - useful_bytes - (bs_ptr - bs_data) < READ_BUF_SIZE) {
                    /* move data if needed */
                    memcpy(bs_data, bs_ptr, useful_bytes);
                    bs_ptr = bs_data;
                }
                read_bytes = (int)fread(bs_ptr + useful_bytes, 1, READ_BUF_SIZE, in_file);
                useful_bytes += read_bytes;
                left_bytes = useful_bytes;
            } else {
                /* this is the last frame data, and no sequence end code */
#if 0
                static loop_cnt = 0;
                loop_cnt++;

                if (rec_file == NULL && loop_cnt <= 10) {
                    fseek(in_file, 0, SEEK_SET);
                    goto readagain;
                } else 
#endif
                {
                    runing = 0; /* set flag to terminate */
                }
            }
        }
    }

    /* flush */
    dec_frame.bs_len = -1;

    do 
    {
        frame_time = get_mdate();
        lib_flush(dec_handle, &dec_frame);
        frame_time = get_mdate() - frame_time;
        total_time += frame_time;

        if (dec_frame.dec_stats == AVS2_TYPE_DECODED) {
            PRINT_INFO;
            OUT_REC_AND_READ_REF;
        }
    } while (dec_frame.dec_stats == AVS2_TYPE_DECODED);
    

    /* output total decoding time */
    fprintf(stderr, "--------------------------------------------------\n");
    fprintf(stderr, "total decoding time: %lld,  %6.2f fps\n", total_time / 1000, frm_count / (total_time / 1000000.0));

 finished:
    /* close decoder to release resources */
    lib_destroy(dec_handle);

    /* free memory spaces */
    if (out_buf != NULL) {
        free(out_buf);
        out_buf = NULL;
    }
    if (bs_data != NULL) {
        free(bs_data);
        bs_data = NULL;
    }
    if (rec_buf != NULL) {
        free(rec_buf);
        rec_buf = NULL;
    }

    /* close files */
    fclose(in_file);

    if (rec_file) {
        fclose(rec_file);
    }
    if (out_file) {
        fclose(out_file);
    }
#if OUTPUT_BG_FRM
    if (gb_file) {
        fclose(gb_file);
    }
    if (gb_rec_file) {
        fclose(gb_rec_file);
    }
#endif

#if GL_DISPLAY
    if (rgb_buf) {
        free(rgb_buf);
    }
    CloseHandle(sem_ready);
    CloseHandle(sem_finish);
#endif

#if 0
    {
        FILE* tmp = fopen("psnr.txt", "a+");
        fprintf(tmp, "%5.2f, %5.2f %5.2f\n", all_psnr[0], all_psnr[1], all_psnr[2]);
        fclose(tmp);
    }
#endif
    return frm_count;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "jtest avs_file rec_file output_file\n");
        return -1;
    } else {
        long long start_time;
        int total_frames;

#if defined(_WIN32)
        
        HMODULE inst_lib;
        if (sizeof(void*) == 8) {
            inst_lib = LoadLibrary("uavs2d_x64.dll");
        } else {
            inst_lib = LoadLibrary("uavs2d.dll");
        }

        if (inst_lib == NULL) {
            fprintf(stderr, "Load library error.\n");
            return -1;
        }

        lib_create = (uavs2d_lib_create_t)GetProcAddress(inst_lib, "uavs2d_lib_create");
        lib_decode = (uavs2d_lib_decode_t)GetProcAddress(inst_lib, "uavs2d_lib_decode");
        lib_destroy = (uavs2d_lib_destroy_t)GetProcAddress(inst_lib, "uavs2d_lib_destroy");
        lib_flush = (uavs2d_lib_flush_t)GetProcAddress(inst_lib, "uavs2d_lib_flush");
#else
        lib_create = uavs2d_lib_create;
        lib_decode = uavs2d_lib_decode;
        lib_destroy = uavs2d_lib_destroy;
        lib_flush = uavs2d_lib_flush;
#endif
        /* get start time */
        start_time = get_mdate();

        /* testing... */
        if (argc < 3 || TEST_SPEED) {
            total_frames = test_avs2_decoder(argv[1], "", "");
        } else if (argc < 4) {
            total_frames = test_avs2_decoder(argv[1], argv[2], "");
        } else {
            total_frames = test_avs2_decoder(argv[1], argv[2], argv[3]);
        }

#if defined(_WIN32)
        FreeLibrary(inst_lib);
#endif

        start_time = get_mdate() - start_time;
        fprintf(stderr, "\n:::::: TOTAL TIME: %lld, %5.2f fps ::::::\n", start_time / 1000, total_frames / (start_time * 1.0 / 1000000));
        fprintf(stderr, "\n:::::: TOTAL PSNR: %5.2f %5.2f %5.2f ::::::\n", all_psnr[0], all_psnr[1], all_psnr[2]);
    }

    return 0; 
}
