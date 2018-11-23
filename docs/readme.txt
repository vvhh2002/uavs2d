1. 结构说明 

1.1 avs2_info_t 序列信息，解码序列头的时候会带出该结构，记录序列的基本信息

typedef struct {            
    int     profile_id;      // 暂时无用  
    int     level_id;        // 暂时无用  
    int     progressive_seq; // 暂时无用  
    int     img_width;       // 图像宽度
    int     img_height;      // 图像高度
    float   frame_rate;      // 帧率
} avs2_info_t;

1.2 avs2_frame_t 解码输入输出结构，送入码流，输出序列信息或图像

typedef struct avs2_frame_t {
    unsigned char     *bs_buf;       // [in ] 码流buffer
    int                bs_len;       // [in ] 码流长度

    int                dec_stats;    // [out] 解码状态，见后续说明

    unsigned long long pts;          // [in/out] 时间戳，播放ES流暂时不需要

    unsigned int       frm_type;     // [out] 当前输出图像的类型，暂时不需要
    unsigned int       qp;           // [out] 当前输出图像的QP，暂时不需要

    int                i_stride;     // [in ] 指定输出图像y分量的跨度
    int                i_stridec;    // [in ] 指定输出图像uv分量的跨度
    int                i_output_type;// [in ] 指定输出图像格式(I420/YUYV)
    unsigned char     *p_buf_y;      // [in: pointer; out: data] 指定输出图像存放的buffer，该buffer由调用者申请，避免输出之后的数据再做一次拷贝
    unsigned char     *p_buf_u;      // [in: pointer; out: data] 同上，存放u分量
    unsigned char     *p_buf_v;      // [in: pointer; out: data] 同上，存放v分量

    avs2_info_t        info;         // [out] 记录序列头信息，解码序列头时带出该信息

} avs2_frame_t;

注： A. dec_stats的值为

#define AVS2_TYPE_DECODED    1        // 正常解码，有图像输出
#define AVS2_TYPE_SEQ        2        // 正常解码，解码了一个序列头
#define AVS2_TYPE_ERROR     -1        // 解码出错
#define AVS2_TYPE_NEEDMORE  -2        // 需要更多数据 (由于解码需要对图像重排序，同时解码库使用的是帧级并行，因此一开始送入几帧图像后并不能马上输出，会返回该值，属正常情况)

     B. i_output_type表示输出图像类型，可以选择I420格式，也可以选择YUYV交织格式。
        选择YUYV时， i_stridec/p_buf_u/p_buf_v没有用，p_buf_y指向YUYV的buffer，i_stride表示按yuyv方式存储时, buffer的跨度

2. 函数说明  

2.1 void * avs2_lib_create(int threads)

    创建解码库。
    threads: 指定解码线程数目，4核8线程机器上，设为9合适
    返回值 : 解码库句柄

2.2 void  avs2_lib_decode(void *handle, avs2_frame_t *frm);

    解码图像或序列头。    
    handle: 解码库句柄
    frm   : 输入输出结构，送入码流，输出序列信息或图像

2.3 void  avs2_lib_flush(void *handle, avs2_frame_t *frm);

    输出延迟的图像。   
    handle: 解码库句柄
    frm   : 输入输出结构，输出序列信息或图像（码流指针不会被用到）

2.4 void  avs2_lib_destroy(void *handle);

    销毁解码库。
    handle: 待销毁的解码库句柄。


3. 调用程序切分码流方法

    每个解码单元是由码流中3字节（0x000001）的起始码前缀分隔，码流中一旦出现0x000001，就一定表示这个是一个新的解码单元(除非传输过程出现误码)
    每个0x000001之后的一个字节表示解码单元类型：
    B0 - 序列头
    B1 - 码流结束标记
    B2/B5/B7 - user data或其他一些数据，暂时不用关心
    B3 - I帧图像头
    B6 - P/B/F帧图像头
    00~AF - 条带头(一幅图像包含1个或多个条带)

    原则上，每个解码单元都是可以独立解码的，但是为了实现方便及调用方便，这里要求每次送入一个完整的帧，或序列头。
    
    扫描码流时，如果发现0x000001B0,则之后的数据是序列头，继续扫描后面的码流，直到下一个0x000001,之间这段数据的长度就是序列头的长度。
 
    解码图像时，如果发现0x000001B3或0x000001B6，则是图像头，继续扫描后面的码流， 直到下一个0x000001B3或0x000001B6或0x000001B0，之间的数据长度就是图像长度。

    注：A. 送入数据解码时，需要带着0x000001起始码前缀。

        B. 码流中B2/B5/B7等类型的解码单元可以包含在图像数据或序列头数据中一同送入，解码库中会自动丢弃这些数据。
           因此，切分序列头和图像帧起始就只需要看0x000001(B0/B3/B6)这三个分隔符，解码时只需要将连续两个分割符之间的数据送入解码库即可(数据要带着数据前的那个分割符一同送入)。

    




