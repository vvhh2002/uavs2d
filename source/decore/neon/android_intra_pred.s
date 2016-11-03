.macro function name
     .global \name
\name:
     .hidden \name
     .type   \name, %function
     .func   \name
\name:
.endm

function xPredIntraVertAdi_neon128

@r0-->*psrc, r1-->*dst, r2-->i_dst, r3-->width, r4-->height

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4}
	sub sp, sp, #40

	and r5,r3,#7  //是否为8的倍数
	cmp r5,#0
	beq xPredIntraVertAdi_neon128_8

xPredIntraVertAdi_neon128_4:
    mov r5,#0
looph_Vert_4:
    mov r6,#0
	add r7,r0,#1     // pel_t *rpSrc = pSrc + 1;
	mov r8,r1
loopw_Vert_4:
    vld1.32 {d0[0]},[r7]     //32位
	vst1.32 {d0[0]},[r8]
	
	add r1,r1,r2
	add r5,r5,#1  //1
	cmp r5,r4
	bne looph_Vert_4
	b end_xPredIntraVertAdi_neon128	    //跳转结束

xPredIntraVertAdi_neon128_8:
    cmp r3, #15
	bgt xPredIntraVertAdi_neon128_16

    mov r5,#0            //外层循环计数
looph_Vert_8:
    mov r6,#0            //内层循环计数
	add r7,r0,#1  // pel_t *rpSrc = pSrc + 1;
	mov r8,r1
loopw_Vert_8:
    vld1.8 {d0},[r7]!
	vst1.8 {d0},[r8]!
	add r6,#8
	cmp r6,r3
	bne loopw_Vert_8

	add r1,r1,r2
	add r5,r5,#1
	cmp r5,r4
	bne looph_Vert_8
	b end_xPredIntraVertAdi_neon128
	
xPredIntraVertAdi_neon128_16:	

    mov r5,#0            //外层循环计数
looph_Vert_16:
    mov r6,#0            //内层循环计数
	add r7,r0,#1  // pel_t *rpSrc = pSrc + 1;
	mov r8,r1
loopw_Vert_16:
    vld1.8 {q0},[r7]!
	vst1.8 {q0},[r8]!
	add r6,#16
	cmp r6,r3
	bne loopw_Vert_16

	add r1,r1,r2
	add r5,r5,#1
	cmp r5,r4
	bne looph_Vert_16
	
end_xPredIntraVertAdi_neon128:	 
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraHorAdi_neon128

@r0-->*psrc, r1-->*dst, r2-->i_dst, r3-->width, r4-->height

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4}
	sub sp, sp, #40

	and r5,r3,#7  //是否为8的倍数
	cmp r5,#0
	beq xPredIntraHorAdi_neon128_8

xPredIntraHorAdi_neon128_4:
    mov r5,#0     //外层计数器
	sub r7,r0,#1  // pel_t *rpSrc = pSrc - 1;

looph_Hor_4:
	
	mov r8,r1  //r8->dst

	ldrsb r9,[r7]
	VDUP.8 d0,r9
	vst1.32 {d0[0]},[r8]
	
	add r1,r1,r2
	sub r7,r7,#1
	add r5,r5,#1
	cmp r5,r4
	bne looph_Hor_4
	b end_xPredIntraHorAdi_neon128	    //跳转结束

xPredIntraHorAdi_neon128_8:
    cmp r3,#15
    bgt xPredIntraHorAdi_neon128_16
	
    mov r5,#0            //外层循环计数
	sub r7,r0,#1         // pel_t *rpSrc = pSrc - 1;
looph_Hor_8:
    mov r6,#0            //内层循环计数
	mov r8,r1
	ldrsb r9,[r7]
	VDUP.8 d0,r9
loopw_Hor_8:
	vst1.8 {d0},[r8]!
	add r6,r6,#8
	cmp r6,r3
	bne loopw_Hor_8

	add r1,r1,r2
	sub r7,r7,#1
	add r5,r5,#1
	cmp r5,r4
	bne looph_Hor_8
	b end_xPredIntraHorAdi_neon128	    //跳转结束
	
xPredIntraHorAdi_neon128_16:	
	
	mov r5,#0            //外层循环计数
	sub r7,r0,#1         // pel_t *rpSrc = pSrc - 1;
looph_Hor_16:
    mov r6,#0            //内层循环计数
	mov r8,r1
	ldrsb r9,[r7]
	VDUP.8 q0,r9
loopw_Hor_16:
	vst1.8 {q0},[r8]!
	add r6,r6,#16
	cmp r6,r3
	bne loopw_Hor_16

	add r1,r1,r2
	sub r7,r7,#1
	add r5,r5,#1
	cmp r5,r4
	bne looph_Hor_16
	
end_xPredIntraHorAdi_neon128:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function xPredIntraDCAdi_neon128

@r0-->*psrc, r1-->*dst, r2-->i_dst, r3-->iWidth, r4-->iHeight,r5-->bAboveAvail, r6-->bLeftAvail
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4,r5,r6}
	sub sp, sp, #40

	mov r8,#0   //iDCValue	
	cmp r6,#0
	beq bLeftAvail_0

bLeftAvail_1:    // if (bLeftAvail) 
    sub r11,r0,r4   //r11为rpSrc[-iHeight+1]
    and r10,r4,#7  //是否为8的倍数
	cmp r10,#0
	beq iHeight_8

iHeight_4:
     vld1.32 {d0[0]},[r11]     //32位
     vld1.32 {d0[1]},[r11]     //32位
	 //扩充为16位再相加
	 //vmovl.U16 q0, d0：将d0的每个16位数据赋值到q0的每个32位数据中
	 //vmovl.U8 q4, d0
	 vpaddl.u8 d1, d0
	 vpadd.u16 d2, d1, d1
	 vmov.U16 r12, d2[0]
	 add r8,r8,r12    //r8=iDCValue
	 b nextstep1

iHeight_8:
     mov r10,#0 //循环计数器

loop_iHeight_8:
	 vld1.8 {d0},[r11]!
	 vpaddl.u8 d1, d0
	 vpadd.u16 d2, d1, d1
	 vpadd.u16 d3, d2, d2
	 vmov.U16 r12, d3[0]
	 add r8,r8,r12    //r8=iDCValue
	 add r10,r10,#8
	 cmp r10,r4
	 bne loop_iHeight_8

nextstep1:
     add r9,r0,#1 //*rpSrc

	 cmp r5,#0      //bAboveAvail
	 beq bAboveAvail_0

bAboveAvail_1:
    and r10,r3,#7  //是否为8的倍数
	cmp r10,#0
	beq iWidth_8
iWidth_4:
     vld1.32 {d0[0]},[r9]     //32位
     vld1.32 {d0[1]},[r9]     //32位
	 vpaddl.u8 d1, d0
	 vpadd.u16 d2, d1, d1
	 vmov.U16 r12, d2[0]
	 add r8,r8,r12    //r8=iDCValue
	 b nextstep2

iWidth_8:
     mov r10,#0 //循环计数器

loop_iWidth_8:
	 vld1.8 {d0},[r9]!
	 vpaddl.u8 d1, d0
	 vpadd.u16 d2, d1, d1
	 vpadd.u16 d3, d2, d2
	 vmov.U16 r12, d3[0]
	 add r8,r8,r12    //r8=iDCValue
	 add r10,r10,#8
	 cmp r10,r3
	 bne loop_iWidth_8

nextstep2:
     add r10,r3,r4
	 lsr r10,r10,#1
	 add r8,r8,r10
	
	 mov r10,#512
	 add r9,r3,r4 
	//vdiv r10,r10,r9

	//div
	push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r11,r12,lr}
	mov r0,r10
	mov r1,r9
	bl __aeabi_idiv
	mov r10,r0
	pop {r0,r1,r2,r3,r4,r5,r6,r7,r8,r11,r12,lr}
	//div

	 mul r8,r8,r10
	 lsr r8,r8,#9                              //1-iDCValue
	 b nextstep4

bAboveAvail_0:
     lsr r10,r4,#1
	 add r8,r8,r10
	 //vdiv r8,r8,r4                             //2-iDCValue

	 //div
	mov r10,r3
	mov r11,r4
	push {r0,r1,r2,r5,r6,r7,r9,r10,r11,r12,lr}
	mov r0,r8
	mov r1,r4
	bl __aeabi_idiv
	mov r8,r0
	pop {r0,r1,r2,r5,r6,r7,r9,r10,r11,r12,lr}
	mov r3,r10
	mov r4,r11
	//div

	b nextstep4

bLeftAvail_0:    // if (bLeftAvail) else
     add r9,r0,#1 //*rpSrc

	 cmp r5,#0      //bAboveAvail
	 beq bAboveAvail_00

bAboveAvail_11:
    and r10,r3,#7  //是否为8的倍数
	cmp r10,#0
	beq iWidth_88
iWidth_44:
     vld1.32 {d0[0]},[r9]     //32位
     vld1.32 {d0[1]},[r9]     //32位
	 vpaddl.u8 d1, d0
	 vpadd.u16 d2, d1, d1
	 vmov.U16 r12, d2[0]
	 add r8,r8,r12    //r8=iDCValue
	 b nextstep3

iWidth_88:
     mov r10,#0 //循环计数器

loop_iWidth_88:
	 vld1.8 {d0},[r9]!
	 vpaddl.u8 d1, d0
	 vpadd.u16 d2, d1, d1
	 vpadd.u16 d3, d2, d2
	 vmov.U16 r12, d3[0]
	 add r8,r8,r12    //r8=iDCValue
	 add r10,r10,#8
	 cmp r10,r3
	 bne loop_iWidth_88
	 b nextstep3

nextstep3:
     lsr r10,r3,#1
	 add r8,r8,r10
	 //vdiv r8,r8,r3                         //3-iDCValue

	//div
	mov r10,r3
	mov r11,r4
	push {r0,r1,r2,r5,r6,r7,r9,r10,r11,r12,lr}
	mov r0,r8
	mov r1,r3
	bl __aeabi_idiv
	mov r8,r0
	pop {r0,r1,r2,r5,r6,r7,r9,r10,r11,r12,lr}
	mov r3,r10
	mov r4,r11
	//div

	 b nextstep4

bAboveAvail_00:
	 mov r12,#128
	 mov r8,r12                        //4-iDCValue

nextstep4:

    //计算dst

	and r10,r3,#7  //是否为8的倍数
	cmp r10,#0
	beq iWidth_888

iWidth_444:
    mov r11,#0
loop_iHeight_444:
	mov r9,r1
	VDUP.8  d0,r8        //32bit
	vst1.32 {d0[0]},[r9]
	
	add r1,r1,r2
	add r11,r11,#1
	cmp r11,r4
	bne loop_iHeight_444
	b end_xPredIntraDCAdi_neon128	    //跳转结束

iWidth_888:
    mov r11,#0            //外层循环计数
loop_iHeight_888:
    mov r12,#0            //内层循环计数
	mov r9,r1
	VDUP.8 d0,r8
loop_iWidhth_888:
	vst1.8 {d0},[r9]!
	add r12,r12,#8
	cmp r12,r3
	bne loop_iWidhth_888

	add r1,r1,r2
	add r11,r11,#1

	cmp r11,r4
	bne loop_iHeight_888

end_xPredIntraDCAdi_neon128:

	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraBiAdi_neon128

@r0-->*psrc, r1-->*dst, r2-->i_dst, r3-->iWidth, r4-->iHeight
  
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp,{r4}
	sub sp,sp,#40

	vpush {q4,q5}
	//计算d0[0]=ishift_x,d0[1]=ishift_y
	//计算ishift_x
	mov r9,#16
	cmp r3,r9
	bgt iWidth_32_64_Bi
	cmp r3,r9
	blt iWidth_4_8_Bi
	
	//iWidth==16
	mov r9,#4
	vmov.U32 d0[0], r9    //d0[0]存储ishift_x
	b s_ishift_y

 iWidth_32_64_Bi:

    mov r9,#32
	cmp r3,r9
	bgt iWidth_64_Bi

	//iWidth==32
	mov r9,#5
	vmov.U32 d0[0], r9    //d0[0]存储ishift_x
	b s_ishift_y

iWidth_64_Bi:
    //iWidth==64
	mov r9,#6
	vmov.U32 d0[0], r9    //d0[0]存储ishift_x
	b s_ishift_y

iWidth_4_8_Bi:
    
	mov r9,#4
	cmp r3,r9
	bgt iWidth_8_Bi

	//iWidth==4
	mov r9,#2
	vmov.U32 d0[0], r9    //d0[0]存储ishift_x
	b s_ishift_y

iWidth_8_Bi:
	//iWidth==8
	mov r9,#3
	vmov.U32 d0[0], r9    //d0[0]存储ishift_x

//计算ishift_y
s_ishift_y:

	mov r9,#16
	cmp r4,r9
	bgt iHeight_32_64_Bi
	cmp r4,r9
	blt iHeight_4_8_Bi
	
	//iHeight==16
	mov r9,#4
	vmov.U32 d0[1], r9    //d0[1]存储ishift_y
	b ishift_xy

 iHeight_32_64_Bi:

    mov r9,#32
	cmp r4,r9
	bgt iHeight_64_Bi

	//iHeight==32
	mov r9,#5
	vmov.U32 d0[1], r9    //d0[1]存储ishift_y
	b ishift_xy

iHeight_64_Bi:
    //iHeight==64
	mov r9,#6
	vmov.U32 d0[1], r9    //d0[1]存储ishift_y
	b ishift_xy

iHeight_4_8_Bi:
    
	mov r9,#4
	cmp r4,r9
	bgt iHeight_8_Bi

	//iHeight==4
	mov r9,#2
	vmov.U32 d0[1], r9    //d0[1]存储ishift_y
	b ishift_xy

iHeight_8_Bi:
	//iHeight==8
	mov r9,#3
	vmov.U32 d0[1], r9    //d0[1]存储ishift_y

	//计算ishift,ishift_xy,offset
ishift_xy:

    vmov.U32 r9,d0[0]     //d0[0]存储ishift_x
	vmov.U32 r10,d0[1]     //d0[1]存储ishift_y
	cmp r9,r10
	bgt ishift_yy
	vmov.U32 d1[0], r9    //d1[0]存储ishift
	b next_ishift_xy
ishift_yy:
    vmov.U32 d1[0], r10    //d1[0]存储ishift

next_ishift_xy:
	add r11,r9,r10
	add r11,r11,#1
	vmov.U32 d1[1], r11    //d1[1]存储ishift_xy
	
	add r11,r9,r10
	mov r12,#1
	lsl r12,r12,r11
	vmov.U32 d2[0], r12    //d2[0]存储offset

 nextstep_abcw:
    //计算a
    add r9,r0,r3
	ldrb r10,[r9]
	vmov.U32 d4[0], r10    //d4[0]存储a

	//计算b
    sub r9,r0,r4
	ldrb r10,[r9]
	vmov.U32 d4[1], r10    //d4[1]存储b

	//计算c
	cmp r3,r4
	beq equal_c
	
	vmov.U32 r9,d4[0]  //a
	vmov.U32 r10,d0[0]  //ishift_x
	lsl r9,r9,r10

	vmov.U32 r10,d4[1]  //b
	vmov.U32 r11,d0[1]  //ishift_y
	lsl r10,r10,r11

	add r9,r9,r10
	mov r10,#13
	mul r9,r9,r10     

	vmov.U32 r10,d1[0]  //ishift
	add r10,r10,#5
	mov r11,#1
	lsl r11,r11,r10

	add r9,r9,r11

	vmov.U32 r10,d1[0]  //ishift
	add r10,r10,#6
	asr r9,r9,r10
	vmov.U32 d5[0], r9   //d5[0]存储c

	b nextstep_w

equal_c:
    vmov.U32 r9,d4[0]     //d4[0]存储a
	vmov.U32 r10,d4[1]     //d4[1]存储b
	add r9,r9,r10
	add r9,r9,#1
	asr r9,r9,#1
	vmov.U32 d5[0], r9    //d5[0]存储c

nextstep_w:
    vmov.U32 r9,d5[0]     //d5[0]存储c
	vmov.U32 r10,d4[0]     //d4[0]存储a
	vmov.U32 r11,d4[1]     //d4[1]存储b
	lsl r12,r9,#1
	sub r12,r12,r10
	sub r12,r12,r11
	vmov.U32 d5[1], r12    //d5[1]存储w

	//PT[],PL[],wy[]
	mov r12,#0  //循环计数器
	mov r10,r0
	add r10,r10,#1
	vmov.U32 r9,d4[1]     //d4[1]存储b
	VDUP.16 d6, r9
	sub r6,sp,#2400
	
loop_ptx:
	vld1.32 d7[0],[r10]!
	vmovl.U8 q4,d7
	vsub.i16 d10,d6,d8

	vst1.16 d10,[r6]!

	add r12,r12,#4
	cmp r12,r3
	bne loop_ptx

	sub r6,r6,r3
	sub r6,r6,r3//pT[x]指针归位

	//PL[]
	mov r12,#0  //循环计数器
	mov r10,r0
	vmov.U32 r9,d4[0]     //d4[0]存储a
	VDUP.16 d6, r9
	sub r7,sp,#2560

loop_ply:
	sub r10,r10,#4
	vld1.32 d7[0],[r10]
	vmovl.U8 q4,d7   //d8存储[-4][-3][-2][-1]
	vrev64.16 q5,q4     //d10存储[-1][-2][-3][-4]

	vsub.i16 d12,d6,d10

	vst1.16 d12,[r7]!

	add r12,r12,#4
	cmp r12,r4
	bne loop_ply

	sub r7,r7,r4
	sub r7,r7,r4//pT[x]指针归位

	//wyL[]
	mov r12,#0  //循环计数器
	
	vmov.U32 r9,d5[1]     //d5[1]存储w
	mov r10,#4
	mul r10,r10,r9  
	VDUP.16 d6, r10       //d6存4w
	
	mov r10,#0
	vmov.U32 d8[0], r10
	add r10,r10,r9
	vmov.U32 d8[1], r10

	add r10,r10,r9
	vmov.U32 d9[0], r10
	add r10,r10,r9
	vmov.U32 d9[1], r10

	vmovn.I32 d10, q4    //d10存0/w/2w/3w
	sub r8,sp,#2720
loop_wy:
	vst1.16 d10,[r8]!

	vadd.s16 d10,d10,d6
	add r12,r12,#4
	cmp r12,r4
	bne loop_wy

	sub r8,r8,r4
	sub r8,r8,r4//pT[x]指针归位

	//dst[]赋值
	mov r12,#0 //外层循环计数器
	
loop_iHeight_Bi:	
	sub r9,r0,#1
	sub r9,r9,r12
	ldrb r10,[r9]
	vmov.U32 r9,d0[0]     //d0[0]存储ishift_x
	lsl  r10,r10,r9       //r10=predx
	vmov.U32 d12[0], r10  //d12[0]==predx
	mov r10,#0
	vmov.U32 d12[1], r10   //d12[1]==wxy 
	
	mov r11,#0 //内层循环计数器

loop_iWidth_Bi:
    vmov.U32 r10,d12[0]
	ldrsh r9,[r7]
	//add r7,r7,#2
	add r10,r10,r9
	vmov.U32 d12[0], r10

	mov r9,r0
	add r9,r9,#1
	add r9,r9,r11
	ldrb r10,[r9]
	vmov.U32 r9,d0[1]     //d0[1]存储ishift_y
	lsl  r10,r10,r9       //r10=pTop[x]
	
	
    ldrsh r9,[r6]
	add r6,r6,#2
	add r12,r12,#1
	mul r9,r9,r12
	sub r12,r12,#1
	add r10,r10,r9

	vmov.U32 r9,d0[0]
	lsl r10,r10,r9

	vmov.U32 d13[0],r10

	vmov.U32 r10,d12[0]
	vmov.U32 r9,d0[1]
	lsl r10,r10,r9

	vmov.U32 r9,d13[0]
	add r10,r10,r9
	
	vmov.U32 r9,d12[1]
	add r10,r10,r9

	vmov.U32 r9,d2[0]
	add r10,r10,r9

	vmov.U32 r9,d1[1]
	asr r10,r10,r9

	VDUP.8 d20, r10   //将r10复制到d0中，8位
	

	vmov.U8 d22, #0
	vmov.U8 d23, #255
	vmax.U8 d20,d20,d22
	vmin.U8 d20,d20,d23

	vst1.8 d20[0],[r1]!

	vmov.U32 r10,d12[1]
	ldrsh r9,[r8]
	//add r8,r8,#2
	add r10,r10,r9
	vmov.U32 d12[1], r10


	add r11,r11,#1
	cmp r11,r3
	bne loop_iWidth_Bi

	sub r6,r6,r3
	sub r6,r6,r3
	
	add r7,r7,#2
	add r8,r8,#2
	
	sub r1,r1,r3
	add r1,r1,r2

	add r12,r12,#1
	cmp r12,r4
	bne loop_iHeight_Bi

	vpop {q4,q5}
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_X_4_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	sub r12, sp, #256
	mov r10, r12

	lsl r5, #1				@ 2 * iHeight

	add r6, r4, r5			@ line_size = iWidth + (iHeight - 1) * 2
	sub r6, #2

	lsl r7, r4, #1			@ iWidth * 2 - 1
	sub r7, #1

	cmp r6, r7				@ real_size = min(line_size, iWidth * 2 - 1)
	movlt r7, r6

	add r0, #2				@ pSrc += 2

	mov r8, #0				@ i = 0
X_4_interpolation:
	vld1.8 {q0}, [r0]
	add r0, #8
	vext.8 d2, d0, d1, #1	@ pSrc[0]
	vext.8 d3, d0, d1, #2	@ pSrc[1]

	vaddl.u8 q0, d0, d3		@ pSrc[-1] + pSrc[1]
	vshll.u8 q1, d2, #1		@ 2 * pSrc[0]

	vadd.u16 q0, q0, q1		@ (pSrc[-1] + pSrc[0] * 2 + pSrc[1] + 2) >> 2
	vrshrn.u16 d0, q0, #2

	vst1.8 d0, [r12]!

	add r8, #8
	cmp r8, r7
	blt X_4_interpolation

	cmp r6, r7
	beq X_4_memcpy
X_4_padding:
	add r12, r10, r7		@ &first_line[real_size]

	ldrb r9, [r12, #-1]		@ first_line[real_size - 1]

	vdup.8 q0, r9

	vst1.8 {q0}, [r12]!		@ padding

	add r7, #16
	cmp r7, r6
	blt X_4_padding

X_4_memcpy:

	cmp r4, #15
	bgt X_4_memcpy_16

	cmp r4, #8
	beq X_4_memcpy_8

X_4_memcpy_4:
	mov r9, #8
X_4_memcpy_4_y:
	vld1.32 {q0}, [r10], r9
	vst1.32	d0[0], [r1], r2
	vext.8 d2, d0, d1, #2
	vst1.32	d2[0], [r1], r2
	vst1.32	d0[1], [r1], r2
	vst1.32	d2[1], [r1], r2

	sub r5, #8
	cmp r5, #0
	bne X_4_memcpy_4_y
	b X_4_end

X_4_memcpy_8:
	mov r9, #16
X_4_memcpy_8_y:
	vld1.32 {q0, q1}, [r10], r9
	vst1.32	d0, [r1], r2
	vext.8 q2, q0, q1, #2
	vst1.32	d4, [r1], r2
	vext.8 q3, q0, q1, #4
	vst1.32	d6, [r1], r2
	vext.8 q8, q0, q1, #6
	vst1.32	d16, [r1], r2
	vst1.32	d1, [r1], r2
	vst1.32	d5, [r1], r2
	vst1.32	d7, [r1], r2
	vst1.32	d17, [r1], r2

	sub r5, #16
	cmp r5, #0
	bne X_4_memcpy_8_y
	b X_4_end

X_4_memcpy_16:
	sub r2, r4
X_4_memcpy_16_y:
	mov r12, r10
	mov r8, #0
X_4_memcpy_16_x:
	vld1.32 {q0}, [r12]!
	vst1.32	{q0}, [r1]!	

	add r8, #16
	cmp r8, r4
	bne X_4_memcpy_16_x

	add r10, #2
	add r1, r2
	sub r5, #2
	cmp r5, #0
	bne X_4_memcpy_16_y

X_4_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function xPredIntraAngAdi_X_6_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	sub r12, sp, #256
	mov r10, r12

	sub r8, r4, #1				@ iWidth - 1
	add r6, r8, r5				@ line_size = iWidth + iHeight - 1

	add r7, r4, r4				@ iWidth * 2

	cmp r6, r7					@ real_size = min(line_size, iWidth * 2)
	movlt r7, r6

	add r0, #1					@ pSrc + = 1

	mov r8, #0					@ i = 0
X_6_interpolation:
	vld1.8 {q0}, [r0]
	add r0, #8
	vext.8 d2, d0, d1, #1	@ pSrc[0]
	vext.8 d3, d0, d1, #2	@ pSrc[1]	

	vaddl.u8 q0, d0, d3		@ pSrc[-1] + pSrc[1]
	vshll.u8 q1, d2, #1		@ 2 * pSrc[0]

	vadd.u16 q0, q0, q1		@ (pSrc[-1] + pSrc[0] * 2 + pSrc[1] + 2) >> 2
	vrshrn.u16 d0, q0, #2

	vst1.8 d0, [r12]!

	add r8, #8
	cmp r8, r7
	blt X_6_interpolation

	cmp r6, r7
	beq X_6_memcpy

X_6_padding:
	add r12, r10, r7		@ &first_line[real_size]

	ldrb r9, [r12, #-1]		@ first_line[real_size - 1]

	vdup.8 q0, r9

	vst1.8 {q0}, [r12]!		@ padding

	add r7, #16
	cmp r7, r6
	blt X_6_padding	

X_6_memcpy:

	cmp r4, #15
	bgt X_6_memcpy_16

	cmp r4, #8
	beq X_6_memcpy_8	

X_6_memcpy_4:
	mov r9, #4
X_6_memcpy_4_y:
	vld1.8 d0, [r10], r9
	vst1.32	d0[0], [r1], r2
	vext.8 d2, d0, d0, #1
	vst1.32	d2[0], [r1], r2
	vext.8 d2, d0, d0, #2
	vst1.32	d2[0], [r1], r2
	vext.8 d2, d0, d0, #3
	vst1.32	d2[0], [r1], r2

	sub r5, #4
	cmp r5, #0
	bne X_6_memcpy_4_y
	b X_6_end

X_6_memcpy_8:
	mov r9, #8
X_6_memcpy_8_y:
	vld1.32 {q0}, [r10], r9
	vst1.32	d0, [r1], r2
	vext.8 d2, d0, d1, #1
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #2
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #3
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #4
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #5
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #6
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #7
	vst1.32	d2, [r1], r2

	sub r5, #8
	cmp r5, #0
	bne X_6_memcpy_8_y
	b X_6_end

X_6_memcpy_16:
	sub r2, r4
X_6_memcpy_16_y:
	mov r12, r10
	mov r8, #0
X_6_memcpy_16_x:
	vld1.32 {q0}, [r12]!
	vst1.32	{q0}, [r1]!	

	add r8, #16
	cmp r8, r4
	bne X_6_memcpy_16_x

	add r10, #1
	add r1, r2
	sub r5, #1
	cmp r5, #0
	bne X_6_memcpy_16_y

X_6_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function xPredIntraAngAdi_X_8_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	sub r12, sp, #256
	mov r10, r12

	lsr r5, #1				@ iHeight / 2

	add r6, r4, r5			@ line_size = iWidth + iHeight / 2 - 1
	sub r6, #1

	lsl r7, r4, #1			@ iWidth * 2 + 1
	add r7, #1

	cmp r6, r7				@ real_size = min(line_size, iWidth * 2 + 1)
	movlt r7, r6

	add r9, r12, #128		@ pfirst[1] = first_line + aligned_line_size
	mov r8, r9

	vmov.u16 q15, #3
	mov r3, #0				@ i = 0
X_8_interpolation:
	vld1.8 {q0}, [r0]		@ d0-->pSrc[0]
	add r0, #8
	vext.8 d2, d0, d1, #1	@ pSrc[1]
	vext.8 d3, d0, d1, #2	@ pSrc[2]
	vext.8 d4, d0, d1, #3	@ pSrc[3]	

	vaddl.u8 q8, d2, d3		@ pSrc[1] + pSrc[2]
	vmul.u16 q10, q8, q15	@ (pSrc[1] + pSrc[2]) * 3

	vaddl.u8 q9, d0, d4		@ pSrc[0] + pSrc[3]

	vaddl.u8 q11, d3, d4	@ pSrc[2] + pSrc[3]

	vadd.u16 q11, q11, q8	@ pSrc[1] + pSrc[2]  * 2 + pSrc[3]
	vadd.u16 q10, q10, q9	@ pSrc[0] + (pSrc[1] + pSrc[2]) * 3 + pSrc[3]

	vrshrn.u16 d0, q10, #3	@ (pSrc[0] + (pSrc[1] + pSrc[2]) * 3 + pSrc[3] + 4) >> 3
	vrshrn.u16 d1, q11, #2	@ (pSrc[1] + pSrc[2]  * 2 + pSrc[3] + 2) >> 2

	vst1.8 d0, [r12]!
	vst1.8 d1, [r9]!

	add r3, #8
	cmp r3, r7
	blt X_8_interpolation

	cmp r6, r7
	beq X_8_memcpy

X_8_padding:
	add r9, r8, r7			@ & pfirst[1][real_size]
	add r12, r10, r7		@ & pfirst[0][real_size]

	ldrb r3, [r9, #-2]		@ load pfirst[1][real_size - 2]
	strb r3, [r9, #-1]

	vdup.8 q15, r3

	ldrb r3, [r12, #-1]		@ load pfirst[0][real_size - 1]

	vdup.8 q14, r3

	vst1.8 {q14}, [r12]!	@ padding
	vst1.8 {q15}, [r9]!

	add r7, #16
	cmp r7, r6
	blt X_6_padding	

X_8_memcpy:

	cmp r4, #15
	bgt X_8_memcpy_16

	cmp r4, #8
	beq X_8_memcpy_8	

X_8_memcpy_4:
	mov r6, #2
X_8_memcpy_4_y:
	vld1.8 {d0}, [r10], r6
	vld1.8 {d1}, [r8], r6
	vst1.32 d0[0], [r1], r2
	vst1.32 d1[0], [r1], r2
	vext.8 d2, d0, d0, #1
	vext.8 d3, d1, d1, #1
	vst1.32 d2[0], [r1], r2
	vst1.32 d3[0], [r1], r2

	sub r5, #2
	cmp r5, #0
	bne X_8_memcpy_4_y
	b X_8_end

X_8_memcpy_8:
	mov r6, #4
X_8_memcpy_8_y:
	vld1.8 {q0}, [r10], r6
	vld1.8 {q1}, [r8], r6	
	vst1.32 d0, [r1], r2
	vst1.32 d2, [r1], r2
	vext.8 d4, d0, d1, #1
	vext.8 d5, d2, d3, #1
	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2
	vext.8 d4, d0, d1, #2
	vext.8 d5, d2, d3, #2
	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2
	vext.8 d4, d0, d1, #3
	vext.8 d5, d2, d3, #3
	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2

	sub r5, #4
	cmp r5, #0
	bne X_8_memcpy_8_y
	b X_8_end

X_8_memcpy_16:
	sub r3, r2, r4
X_8_memcpy_16_y:	
	mov r12, r10
	mov r9, r8
	mov r6, #0
	add r7, r1, r2
X_8_memcpy_16_x:
	vld1.8 {q0}, [r12]!
	vld1.8 {q1}, [r9]!
	vst1.32 {q0}, [r1]!
	vst1.32 {q1}, [r7]!

	add r6, #16
	cmp r6, r4
	bne X_8_memcpy_16_x

	add r10, #1
	add r8, #1
	add r1, r7, r3
	sub r5, #1
	cmp r5, #0
	bne X_8_memcpy_16_y

X_8_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function xPredIntraAngAdi_X_10_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	vmov.u8 d26, #3
	vmov.u16 q14, #3
	vmov.u8 d30, #5
	vmov.u8 d31, #7

	cmp r5, #4
	beq xPredIntraAngAdi_X_10_H4

	sub r7, sp, #512			@ pfirst[0]
	add r8, r7, #128			@ pfirst[1]
	add r9, r8, #128			@ pfirst[2]
	add r10, r9, #128			@ pfirst[3]
	mov r12, r7

	lsr r5, #2					@ iHeight / 4
	
	add r6, r4, r5				@ line_size = iWidth + iHeight / 4 - 1
	sub r6, #1

	mov r3, #0					@ i = 0
X_10_interpolation:
	vld1.8 {q0}, [r0]			@ d0-->pSrc[0]
	add r0, #8
	vext.8 d2, d0, d1, #2		@ pSrc[2]
	vext.8 d3, d0, d1, #3		@ pSrc[3]	
	vext.8 d1, d0, d1, #1		@ pSrc[1]

	vmull.u8 q8, d0, d26				@ pSrc[0] * 3
	vmull.u8 q9, d1, d31				@ pSrc[1] * 7
	vmull.u8 q10, d2, d30				@ pSrc[2] * 5

	vaddw.u8 q8, q8, d3					@ pSrc[0] * 3 +  pSrc[1] * 7 + pSrc[2] * 5 + pSrc[3]
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d22, q8, #4

	vst1.32 d22, [r7]!					@ store &pfirst[0]

	vaddl.u8 q8, d1, d2					@ pSrc[1] + pSrc[2]  * 2 + pSrc[3]
	vaddl.u8 q9, d2, d3
	vadd.u16 q9, q8, q9

	vmul.u16 q8, q8, q14				@ pSrc[0] + (pSrc[1] + pSrc[2]) * 3 + pSrc[3]
	vaddl.u8 q10, d0, d3
	vadd.u16 q8, q8, q10

	vrshrn.u16 d23, q8, #3
	vrshrn.u16 d25, q9, #2

	vst1.32 d23, [r8]!					@ store &pfirst[1]
	vst1.32 d25, [r10]!					@ store &pfirst[3]

	vmull.u8 q8, d1, d30				@ pSrc[1] * 5
	vmull.u8 q9, d2, d31				@ pSrc[2] * 7
	vmull.u8 q10, d3, d26				@ pSrc[3] * 3

	vaddw.u8 q8, q8, d0					@ pSrc[0] + pSrc[1] * 5 + pSrc[2] * 7 + pSrc[3] * 3
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d23, q8, #4

	vst1.32 d23, [r9]!					@ store &pfirst[2]

	add r3, #8
	cmp r3, r6
	blt X_10_interpolation

	add r8, r12, #128			@ pfirst[1]
	add r9, r8, #128			@ pfirst[2]
	add r10, r9, #128			@ pfirst[3]

X_10_memcpy:

	cmp r4, #15
	bgt X_10_memcpy_16

	cmp r4, #8
	beq X_10_memcpy_8

X_10_memcpy_4:
	vld1.8 d0, [r12]
	vld1.8 d1, [r8]
	vld1.8 d2, [r9]
	vld1.8 d3, [r10]

	vst1.32 d0[0], [r1], r2
	vst1.32 d1[0], [r1], r2
	vst1.32 d2[0], [r1], r2
	vst1.32 d3[0], [r1], r2

	vext.8 d4, d0, d0, #1
	vext.8 d5, d1, d1, #1
	vext.8 d6, d2, d2, #1
	vext.8 d7, d3, d3, #1

	vst1.32 d4[0], [r1], r2
	vst1.32 d5[0], [r1], r2
	vst1.32 d6[0], [r1], r2
	vst1.32 d7[0], [r1], r2

	vext.8 d4, d0, d0, #2
	vext.8 d5, d1, d1, #2
	vext.8 d6, d2, d2, #2
	vext.8 d7, d3, d3, #2

	vst1.32 d4[0], [r1], r2
	vst1.32 d5[0], [r1], r2
	vst1.32 d6[0], [r1], r2
	vst1.32 d7[0], [r1], r2

	vext.8 d4, d0, d0, #3
	vext.8 d5, d1, d1, #3
	vext.8 d6, d2, d2, #3
	vext.8 d7, d3, d3, #3

	vst1.32 d4[0], [r1], r2
	vst1.32 d5[0], [r1], r2
	vst1.32 d6[0], [r1], r2
	vst1.32 d7[0], [r1], r2

	b X_10_end

X_10_memcpy_8:
	mov r6, #2
X_10_memcpy_8_y:
	vld1.8 {q0}, [r12], r6
	vld1.8 {q1}, [r8], r6
	vld1.8 {q2}, [r9], r6
	vld1.8 {q3}, [r10], r6

	vst1.32 d0, [r1], r2
	vst1.32 d2, [r1], r2
	vst1.32 d4, [r1], r2
	vst1.32 d6, [r1], r2

	vext.8 d0, d0, d1, #1
	vext.8 d2, d2, d3, #1
	vext.8 d4, d4, d5, #1
	vext.8 d6, d6, d7, #1

	vst1.32 d0, [r1], r2
	vst1.32 d2, [r1], r2
	vst1.32 d4, [r1], r2
	vst1.32 d6, [r1], r2

	sub r5, #2
	cmp r5, #0
	bne X_10_memcpy_8_y
	b X_10_end

X_10_memcpy_16:
	sub r0, r4, #1
	lsl r7, r2, #2
	sub r3, r7, r4
	sub r7, #16
X_10_memcpy_16_y:
	mov r6, #0
X_10_memcpy_16_x:
	vld1.8 {q0}, [r12]!
	vld1.8 {q1}, [r8]!
	vld1.8 {q2}, [r9]!
	vld1.8 {q3}, [r10]!

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2

	sub r1, r7

	add r6, #16
	cmp r6, r4
	bne X_10_memcpy_16_x

	add r1, r3

	sub r12, r0
	sub r8, r0
	sub r9, r0
	sub r10, r0
	sub r5, #1
	cmp r5, #0
	bne X_10_memcpy_16_y

	b X_10_end

xPredIntraAngAdi_X_10_H4:
	cmp	r4, #4
	beq xPredIntraAngAdi_X_10_H4_W4

xPredIntraAngAdi_X_10_H4_W16:
	vld1.8 {q0, q1}, [r0]				@ q0:pSrc[0]
	vext.8 q2, q0, q1, #2				@ pSrc[2]
	vext.8 q3, q0, q1, #3				@ pSrc[3]
	vext.8 q1, q0, q1, #1				@ pSrc[1]

	vmull.u8 q8, d0, d26				@ pSrc[0] * 3
	vmull.u8 q9, d2, d31				@ pSrc[1] * 7
	vmull.u8 q10, d4, d30				@ pSrc[2] * 5

	vaddw.u8 q8, q8, d6					@ pSrc[0] * 3 +  pSrc[1] * 7 + pSrc[2] * 5 + pSrc[3]
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d22, q8, #4

	vmull.u8 q8, d1, d26				@ pSrc[0] * 3
	vmull.u8 q9, d3, d31				@ pSrc[1] * 7
	vmull.u8 q10, d5, d30				@ pSrc[2] * 5

	vaddw.u8 q8, q8, d7					@ pSrc[0] * 3 +  pSrc[1] * 7 + pSrc[2] * 5 + pSrc[3]
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d23, q8, #4

	vst1.8 {q11}, [r1], r2				@ store &dst1

	vaddl.u8 q8, d2, d4					@ pSrc[1] + pSrc[2]  * 2 + pSrc[3]
	vaddl.u8 q9, d4, d6
	vadd.u16 q9, q8, q9

	vmul.u16 q8, q8, q14				@ pSrc[0] + (pSrc[1] + pSrc[2]) * 3 + pSrc[3]
	vaddl.u8 q10, d0, d6
	vadd.u16 q8, q8, q10

	vrshrn.u16 d22, q8, #3
	vrshrn.u16 d24, q9, #2

	vaddl.u8 q8, d3, d5					@ pSrc[1] + pSrc[2]  * 2 + pSrc[3]
	vaddl.u8 q9, d5, d7
	vadd.u16 q9, q8, q9

	vmul.u16 q8, q8, q14				@ pSrc[0] + (pSrc[1] + pSrc[2]) * 3 + pSrc[3]
	vaddl.u8 q10, d1, d7
	vadd.u16 q8, q8, q10

	vrshrn.u16 d23, q8, #3
	vrshrn.u16 d25, q9, #2

	vst1.8 {q11}, [r1], r2				@ store &dst2
	add r6, r1, r2
	vst1.8 {q12}, [r6]					@ store &dst4

	vmull.u8 q8, d2, d30				@ pSrc[1] * 5
	vmull.u8 q9, d4, d31				@ pSrc[2] * 7
	vmull.u8 q10, d6, d26				@ pSrc[3] * 3

	vaddw.u8 q8, q8, d0					@ pSrc[0] + pSrc[1] * 5 + pSrc[2] * 7 + pSrc[3] * 3
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d22, q8, #4

	vmull.u8 q8, d3, d30				@ pSrc[1] * 5
	vmull.u8 q9, d5, d31				@ pSrc[2] * 7
	vmull.u8 q10, d7, d26				@ pSrc[3] * 3

	vaddw.u8 q8, q8, d1					@ pSrc[0] + pSrc[1] * 5 + pSrc[2] * 7 + pSrc[3] * 3
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d23, q8, #4

	vst1.8 {q11}, [r1]					@ store &dst3

	b X_10_end

xPredIntraAngAdi_X_10_H4_W4:
	vld1.8 {d0}, [r0]					@ d0[0]:pSrc[0]
	vext.8 d2, d0, d0, #2				@ pSrc[2]
	vext.8 d3, d0, d0, #3				@ pSrc[3]
	vext.8 d1, d0, d0, #1				@ pSrc[1]	

	vmull.u8 q8, d0, d26				@ pSrc[0] * 3
	vmull.u8 q9, d1, d31				@ pSrc[1] * 7
	vmull.u8 q10, d2, d30				@ pSrc[2] * 5

	vaddw.u8 q8, q8, d3					@ pSrc[0] * 3 +  pSrc[1] * 7 + pSrc[2] * 5 + pSrc[3]
	vadd.u16 d16, d16, d18
	vadd.u16 d16, d16, d20

	vrshrn.u16 d22, q8, #4

	vst1.32 d22[0], [r1], r2			@ store &dst1

	vaddl.u8 q8, d1, d2					@ pSrc[1] + pSrc[2]  * 2 + pSrc[3]
	vaddl.u8 q9, d2, d3
	vadd.u16 d18, d16, d18

	vmul.u16 d16, d16, d28				@ pSrc[0] + (pSrc[1] + pSrc[2]) * 3 + pSrc[3]
	vaddl.u8 q10, d0, d3
	vadd.u16 d16, d16, d20

	vrshrn.u16 d23, q8, #3
	vrshrn.u16 d25, q9, #2

	vst1.32 d23[0], [r1], r2			@ store &dst2
	add r6, r1, r2
	vst1.32 d25[0], [r6]				@ store &dst4

	vmull.u8 q8, d1, d30				@ pSrc[1] * 5
	vmull.u8 q9, d2, d31				@ pSrc[2] * 7
	vmull.u8 q10, d3, d26				@ pSrc[3] * 3

	vaddw.u8 q8, q8, d0					@ pSrc[0] + pSrc[1] * 5 + pSrc[2] * 7 + pSrc[3] * 3
	vadd.u16 d16, d16, d18
	vadd.u16 d16, d16, d20

	vrshrn.u16 d23, q8, #4

	vst1.32 d23[0], [r1]					@ store &dst3

X_10_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_Y_30_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40
	
	sub r12, sp, #256
	mov r10, r12

	add r6, r4, r5			@ line_size = iWidth + iHeight - 1
	sub r6, #1

	lsl r7, r5, #1			@ iHeight * 2

	cmp r6, r7				@ real_size = min(line_size, iHeight * 2)
	movlt r7, r6

	sub r0, #2	  		    @ pSrc -= 2

	mov r8, #0				@ i = 0
	
	sub r0, #8				@ pSrc[-8]
Y_30_interpolation:
	vld1.8 {q0}, [r0]       @ pSrc[-8]
	sub r0, #8
	vext.8 d2, d0, d1, #1	@ pSrc[-7]
	vext.8 d3, d0, d1, #2	@ pSrc[-6]

	vaddl.u8 q0, d0, d3		@ pSrc[-8] + pSrc[-6]
	vshll.u8 q1, d2, #1		@ 2 * pSrc[-7]

	vadd.u16 q0, q0, q1		@ (pSrc[-8] + pSrc[-7] * 2 + pSrc[-6] + 2) >> 2
	vrshrn.u16 d2, q0, #2   
	vrshrn.u16 d3, q0, #2

    vrev64.8 q0,q1          @ 反序
	
	vst1.8 d0, [r12]!

	add r8, #8
	cmp r8, r7
	blt Y_30_interpolation

	cmp r6, r7
	beq Y_30_memcpy
	
Y_30_padding:
	add r12, r10, r7		@ &first_line[real_size]

	ldrb r9, [r12, #-1]		@ first_line[real_size - 1]

	vdup.8 q0, r9

	vst1.8 {q0}, [r12]!		@ padding

	add r7, #16
	cmp r7, r6
	blt Y_30_padding

Y_30_memcpy:	
	cmp r4, #15
	bgt Y_30_memcpy_16

	cmp r4, #8
	beq Y_30_memcpy_8

Y_30_memcpy_4:
	mov r9, #4
Y_30_memcpy_4_y:
	vld1.8 d0, [r10], r9
	vst1.32	d0[0], [r1], r2
	vext.8 d2, d0, d0, #1
	vst1.32	d2[0], [r1], r2
	vext.8 d2, d0, d0, #2
	vst1.32	d2[0], [r1], r2
	vext.8 d2, d0, d0, #3
	vst1.32	d2[0], [r1], r2

	sub r5, #4
	cmp r5, #0
	bne Y_30_memcpy_4_y
	b Y_30_end

Y_30_memcpy_8:
	mov r9, #8
Y_30_memcpy_8_y:
    vld1.32 {q0}, [r10], r9
	vst1.32	d0, [r1], r2
	vext.8 d2, d0, d1, #1
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #2
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #3
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #4
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #5
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #6
	vst1.32	d2, [r1], r2
	vext.8 d2, d0, d1, #7
	vst1.32	d2, [r1], r2

	sub r5, #8
	cmp r5, #0
	bne Y_30_memcpy_8_y
	b Y_30_end

Y_30_memcpy_16:
	sub r2, r4
Y_30_memcpy_16_y:
	mov r12, r10
	mov r8, #0
Y_30_memcpy_16_x:
	vld1.32 {q0}, [r12]!
	vst1.32	{q0}, [r1]!	

	add r8, #16
	cmp r8, r4
	bne Y_30_memcpy_16_x

	add r10, #1
	add r1, r2
	sub r5, #1
	cmp r5, #0
	bne Y_30_memcpy_16_y

Y_30_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_Y_32_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	sub r12, sp, #256
	mov r10, r12

	lsr r7, r5, #1			@ iHeight / 2

	add r6, r4, r7			@ line_size = iWidth + iHeight / 2 - 1
	sub r6, #1              

	mov r7, r5				@ iHeight-1

	cmp r6, r7				@ real_size = min(line_size, iHeight-1)
	movlt r7, r6

	add r9, r12, #128		@ pfirst[1] = first_line + aligned_line_size
	mov r8, r9

	mov r3, #0				@ i = 0
	sub r0, #3				@ pSrc -= 3
	
	sub r0, #16				@ pSrc[-16]
Y_32_interpolation:
	vld2.8 {d0,d1}, [r0]	@ d0-->pSrc[-16]-pSrc[-2],d1-->pSrc[-15]-pSrc[-1]
	add r0, #2
	vld2.8 {d2,d3}, [r0]	@ d2-->pSrc[-14]-pSrc[-0],d3-->pSrc[-13]-pSrc[1]
	sub r0, #18             @ pSrc[-32]	

	vaddl.u8 q8, d1, d2		@ pSrc[-1] + pSrc[0]
	vaddl.u8 q9, d2, d3	    @ pSrc[0] + pSrc[1]
	vadd.u16 q9, q9, q8	    @ pSrc[-1] + pSrc[0]  * 2 + pSrc[1]
	
	vaddl.u8 q10, d0, d1	@ pSrc[-2] + pSrc[-1]
	vaddl.u8 q11, d1, d2	@ pSrc[-1] + pSrc[0]
	vadd.u16 q11, q11, q10	@ pSrc[-2] + pSrc[-1]  * 2 + pSrc[0]

	vrshrn.u16 d2, q9, #2	@ (pSrc[1] + pSrc[0]  * 2 + pSrc[-1] + 2) >> 2
	vrshrn.u16 d3, q9, #2	@ (pSrc[1] + pSrc[0]  * 2 + pSrc[-1] + 2) >> 2
	vrev64.8 q0,q1          @ 反序
	vst1.8 d0, [r12]!
	
	vrshrn.u16 d2, q11, #2	@ (pSrc[0] + pSrc[-1]  * 2 + pSrc[-2] + 2) >> 2
	vrshrn.u16 d3, q11, #2	@ (pSrc[0] + pSrc[-1]  * 2 + pSrc[-2] + 2) >> 2
	vrev64.8 q0,q1          @ 反序
	vst1.8 d0, [r9]!
	
	add r3, #8
	cmp r3, r7
	blt Y_32_interpolation

	cmp r6, r7
	beq Y_32_memcpy
	
Y_32_padding:
	add r9, r8, r7			@ & pfirst[1][real_size]
	add r12, r10, r7		@ & pfirst[0][real_size]
	ldrb r3,[r9, #-1]		@ load pfirst[1][real_size - 1]
    vdup.8 q14, r3

Y_32_padding_1:
	vst1.8 {q14}, [r12]!	@ padding
	vst1.8 {q14}, [r9]!

	add r7, #16
	cmp r7, r6
	blt Y_32_padding_1	
	
Y_32_memcpy:

	cmp r4, #15
	bgt Y_32_memcpy_16

	cmp r4, #8
	beq Y_32_memcpy_8	

Y_32_memcpy_4:
	mov r6, #2
Y_32_memcpy_4_y:
	vld1.8 {d0}, [r10], r6
	vld1.8 {d1}, [r8], r6
	vst1.32 d0[0], [r1], r2
	vst1.32 d1[0], [r1], r2
	vext.8 d2, d0, d0, #1
	vext.8 d3, d1, d1, #1
	vst1.32 d2[0], [r1], r2
	vst1.32 d3[0], [r1], r2

	sub r5, #2
	cmp r5, #0
	bne Y_32_memcpy_4_y
	b Y_32_end

Y_32_memcpy_8:
	mov r6, #4
Y_32_memcpy_8_y:
	vld1.8 {q0}, [r10], r6
	vld1.8 {q1}, [r8], r6	
	vst1.32 d0, [r1], r2
	vst1.32 d2, [r1], r2
	vext.8 d4, d0, d1, #1
	vext.8 d5, d2, d3, #1
	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2
	vext.8 d4, d0, d1, #2
	vext.8 d5, d2, d3, #2
	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2
	vext.8 d4, d0, d1, #3
	vext.8 d5, d2, d3, #3
	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2

	sub r5, #4
	cmp r5, #0
	bne Y_32_memcpy_8_y
	b Y_32_end

Y_32_memcpy_16:
	sub r3, r2, r4
Y_32_memcpy_16_y:	
	mov r12, r10
	mov r9, r8
	mov r6, #0
	add r7, r1, r2
Y_32_memcpy_16_x:
	vld1.8 {q0}, [r12]!
	vld1.8 {q1}, [r9]!
	vst1.32 {q0}, [r1]!
	vst1.32 {q1}, [r7]!

	add r6, #16
	cmp r6, r4
	bne Y_32_memcpy_16_x

	add r10, #1
	add r8, #1
	add r1, r7, r3
	sub r5, #1
	cmp r5, #0
	bne Y_32_memcpy_16_y

Y_32_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_Y_28_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40
	
	sub r12, sp, #256
	mov r10, r12

	lsl r5, #1				@ 2 * iHeight

	add r6, r4, r5			@ line_size = iWidth + (iHeight - 1) * 2
	sub r6, #2

	lsl r7, r5, #1			@ 4 * iHeight + 1
	add r7, #1
	
	cmp r6, r7				@ real_size = min(line_size, 4 * iHeight + 1)
	movlt r7, r6

	vmov.u16 q15, #3
	mov r8, #0				@ i = 0	
	sub r0, #10
Y_28_interpolation:
	vld1.8 {q0}, [r0]		@ d0-->pSrc[-10]-->pSrc[-3]
	sub r0, #8
	vext.8 d2, d0, d1, #1	@ d2-->pSrc[-9]-->pSrc[-2]
	vext.8 d3, d0, d1, #2	@ d3-->pSrc[-8]-->pSrc[-1]
	vext.8 d4, d0, d1, #3	@ d4-->pSrc[-7]-->pSrc[0]	

	vaddl.u8 q8, d2, d3		@ pSrc[-1] + pSrc[-2]
	vmul.u16 q10, q8, q15	@ (pSrc[-1] + pSrc[-2]) * 3

	vaddl.u8 q9, d0, d4		@ pSrc[0] + pSrc[-3]

	vaddl.u8 q11, d0, d2	@ pSrc[-2] + pSrc[-3]

	vadd.u16 q11, q11, q8	@ pSrc[-1] + pSrc[-2]  * 2 + pSrc[-3]
	vadd.u16 q10, q10, q9	@ pSrc[0] + (pSrc[-1] + pSrc[-2]) * 3 + pSrc[-3]

	vrshrn.u16 d2, q10, #3	@ (pSrc[0] + (pSrc[-1] + pSrc[-2]) * 3 + pSrc[-3] + 4) >> 3
	vrshrn.u16 d3, q11, #2	@ (pSrc[-1] + pSrc[-2]  * 2 + pSrc[-3] + 2) >> 2
	vrev64.8 q0,q1          @ 反序

	vst2.8 {d0,d1}, [r12]!  @交叉存储

	add r8, #16
	cmp r8, r7
	blt Y_28_interpolation

	cmp r6, r7
	beq Y_28_memcpy
	
Y_28_padding:
	add r7, #1
	add r12, r10, r7		@ &pfirst_line[real_size]

	ldrb r3, [r12, #-3]		@ load pfirst_line[real_size - 3]
	strb r3, [r12, #-1]
	vdup.8 q15, r3          @pad2

	ldrb r3, [r12, #-2]		@ load pfirst_line[real_size - 2]
	vdup.8 q14, r3          @pad1

	vst2.8 {d28,d30}, [r12]!	@ padding

	add r7, #16
	cmp r7, r6
	blt Y_28_padding	
	
Y_28_memcpy:

	cmp r4, #15
	bgt Y_28_memcpy_16

	cmp r4, #8
	beq Y_28_memcpy_8

Y_28_memcpy_4:
	mov r9, #8
Y_28_memcpy_4_y:
	vld1.32 {q0}, [r10], r9
	vst1.32	d0[0], [r1], r2
	vext.8 d2, d0, d1, #2            
	vst1.32	d2[0], [r1], r2
	vst1.32	d0[1], [r1], r2
	vst1.32	d2[1], [r1], r2

	sub r5, #8
	cmp r5, #0
	bne Y_28_memcpy_4_y
	b Y_28_end

Y_28_memcpy_8:
	mov r9, #16
Y_28_memcpy_8_y:
	vld1.32 {q0, q1}, [r10], r9
	vst1.32	d0, [r1], r2
	vext.8 q2, q0, q1, #2
	vst1.32	d4, [r1], r2
	vext.8 q3, q0, q1, #4
	vst1.32	d6, [r1], r2
	vext.8 q8, q0, q1, #6
	vst1.32	d16, [r1], r2
	vst1.32	d1, [r1], r2
	vst1.32	d5, [r1], r2
	vst1.32	d7, [r1], r2
	vst1.32	d17, [r1], r2

	sub r5, #16
	cmp r5, #0
	bne Y_28_memcpy_8_y
	b Y_28_end

Y_28_memcpy_16:
	sub r2, r4
Y_28_memcpy_16_y:
	mov r12, r10
	mov r8, #0
Y_28_memcpy_16_x:
	vld1.32 {q0}, [r12]!
	vst1.32	{q0}, [r1]!	

	add r8, #16
	cmp r8, r4
	bne Y_28_memcpy_16_x

	add r10, #2
	add r1, r2
	sub r5, #2
	cmp r5, #0
	bne Y_28_memcpy_16_y

Y_28_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_Y_26_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

    stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	vmov.u8 d26, #3
	vmov.u16 q14, #3
	vmov.u8 d30, #5
	vmov.u8 d31, #7
	
	cmp r4, #4
	beq xPredIntraAngAdi_Y_26_W4
	
	sub r7, sp, #512			@ pfirst[0]
	mov r12, r7
	
	sub r8,r5,#1				@ iHeight-1
	lsl r8,#2					@ (iHeight-1)*4
	
	add r6, r4, r8				@ line_size = iWidth + (iHeight-1)*4
	
	lsl r5,#2					@ iHeight*4
	
	mov r3, #0					@ i = 0
    sub r0, #10                 @ pSrc[-10]
Y_26_interpolation:
	vld1.8 {q0}, [r0]		    @ d0-->pSrc[-10]-->pSrc[-3]
	sub r0, #8
	vext.8 d2, d0, d1, #1	    @ d2-->pSrc[-9]-->pSrc[-2]
	vext.8 d3, d0, d1, #2	    @ d3-->pSrc[-8]-->pSrc[-1]
	vext.8 d4, d0, d1, #3	    @ d4-->pSrc[-7]-->pSrc[0]	

	vmull.u8 q8, d4, d26		@ pSrc[0] * 3
	vmull.u8 q9, d3, d31		@ pSrc[-1] * 7
	vmull.u8 q10, d2, d30		@ pSrc[-2] * 5

	vaddw.u8 q8, q8, d0			@ pSrc[0] * 3 +  pSrc[-1] * 7 + pSrc[-2] * 5 + pSrc[-3]
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vaddl.u8 q9, d2, d3		    @ pSrc[-1] + pSrc[-2]  * 2 + pSrc[-3]
	vaddl.u8 q10, d2, d0
	vadd.u16 q9, q9, q10

	vaddl.u8 q10, d2, d3        @ pSrc[0] + (pSrc[-1] + pSrc[-2]) * 3 + pSrc[-3]
	vmul.u16 q10, q10, q14		
	vaddl.u8 q11, d0, d4
	vadd.u16 q10, q10, q11

	vmull.u8 q11, d3, d30		@ pSrc[-1] * 5
	vmull.u8 q12, d2, d31		@ pSrc[-2] * 7
	vadd.u16 q11, q11, q12
	vmull.u8 q12, d0, d26		@ pSrc[-3] * 3
    vadd.u16 q11, q11, q12
	vaddw.u8 q11, q11, d4	    @ pSrc[0] + pSrc[-1] * 5 + pSrc[-2] * 7 + pSrc[-3] * 3

	vrshrn.u16 d24, q8, #4	    @ (pSrc[0] * 3 +  pSrc[-1] * 7 + pSrc[-2] * 5 + pSrc[-3] + 8) >> 4
	vrshrn.u16 d25, q10, #3	    @ (pSrc[0] + (pSrc[-1] + pSrc[-2]) * 3 + pSrc[-3] + 4) >> 3
	vrev64.8 q8,q12             @ 反序

	vrshrn.u16 d24, q11, #4	    @ (pSrc[0] + pSrc[-1] * 5 + pSrc[-2] * 7 + pSrc[-3] * 3 + 8) >> 4
	vrshrn.u16 d25, q9, #2	    @ (pSrc[-1] + pSrc[-2]  * 2 + pSrc[-3] + 2) >> 2
	vrev64.8 q9,q12             @ 反序

	vst4.8 {d16, d17, d18, d19}, [r7]!  @交叉存储

	add r3, #32
	cmp r3, r6
	blt Y_26_interpolation
	
Y_26_memcpy:

	cmp r4, #15
	bgt Y_26_memcpy_16

Y_26_memcpy_8:
	mov r9, #16
Y_26_memcpy_8_y:
	vld1.32 {q0, q1}, [r12], r9
	vst1.32	d0, [r1], r2
	vext.8 q2, q0, q1, #4
	vst1.32	d4, [r1], r2
	vext.8 q3, q0, q1, #8
	vst1.32	d6, [r1], r2
	vext.8 q8, q0, q1, #12
	vst1.32	d16, [r1], r2
	
	sub r5, #16
	cmp r5, #0
	bne Y_26_memcpy_8_y
	b Y_26_end

Y_26_memcpy_16:
	sub r2, r4
Y_26_memcpy_16_y:
	mov r7, r12
	mov r8, #0
Y_26_memcpy_16_x:
	vld1.32 {q0},[r7]!
	vst1.32	{q0},[r1]!	

	add r8, #16
	cmp r8, r4
	bne Y_26_memcpy_16_x

	add r12, #4
	add r1, r2
	sub r5, #4
	cmp r5, #0
	bne Y_26_memcpy_16_y
	b Y_26_end
	
xPredIntraAngAdi_Y_26_W4:
	
	mov r3, #0					@ i = 0
    sub r0, #10
Y_26_interpolation_W4:
	vld1.8 {q0}, [r0]		    @ d0-->pSrc[-10]-->pSrc[-3]
	sub r0, #4
	vext.8 d2, d0, d1, #1	    @ d2-->pSrc[-9]-->pSrc[-2]
	vext.8 d3, d0, d1, #2	    @ d3-->pSrc[-8]-->pSrc[-1]
	vext.8 d4, d0, d1, #3	    @ d4-->pSrc[-7]-->pSrc[0]	

	vmull.u8 q8, d4, d26		@ pSrc[0] * 3
	vmull.u8 q9, d3, d31		@ pSrc[-1] * 7
	vmull.u8 q10, d2, d30		@ pSrc[-2] * 5

	vaddw.u8 q8, q8, d0			@ pSrc[0] * 3 +  pSrc[-1] * 7 + pSrc[-2] * 5 + pSrc[-3]
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vaddl.u8 q9, d2, d3		    @ pSrc[-1] + pSrc[-2]  * 2 + pSrc[-3]
	vaddl.u8 q10, d2, d0
	vadd.u16 q9, q9, q10

	vaddl.u8 q10, d2, d3        @ pSrc[0] + (pSrc[-1] + pSrc[-2]) * 3 + pSrc[-3]
	vmul.u16 q10, q10, q14		
	vaddl.u8 q11, d0, d4
	vadd.u16 q10, q10, q11

	vmull.u8 q11, d3, d30		@ pSrc[-1] * 5
	vmull.u8 q12, d2, d31		@ pSrc[-2] * 7
	vadd.u16 q11, q11, q12
	vmull.u8 q12, d0, d26		@ pSrc[-3] * 3
    vadd.u16 q11, q11, q12
	vaddw.u8 q11, q11, d4	    @ pSrc[0] + pSrc[-1] * 5 + pSrc[-2] * 7 + pSrc[-3] * 3
	
	vrshrn.u16 d24, q8, #4	    @ (pSrc[0] * 3 +  pSrc[-1] * 7 + pSrc[-2] * 5 + pSrc[-3] + 8) >> 4
	vrev64.8 q8,q12             @ 反序
	
	vrshrn.u16 d24, q10, #3	    @ (pSrc[0] + (pSrc[-1] + pSrc[-2]) * 3 + pSrc[-3] + 4) >> 3
	vrev64.8 q10,q12            @ 反序
	
	vrshrn.u16 d24, q11, #4	    @ (pSrc[0] + pSrc[-1] * 5 + pSrc[-2] * 7 + pSrc[-3] * 3 + 8) >> 4
	vrev64.8 q11,q12            @ 反序
	
	vrshrn.u16 d24, q9, #2	    @ (pSrc[-1] + pSrc[-2]  * 2 + pSrc[-3] + 2) >> 2
	vrev64.8 q9,q12             @ 反序
	
    vmovl.u8 q8,d16             @扩展
	vmovl.u8 q10,d20 
	vmovl.u8 q11,d22 
	vmovl.u8 q9,d18 
	
	vmov d17,d20
	vmov d23,d18
	
	vtrn.16 d16,d17             @转置
	vtrn.16 d22,d23
	vtrn.32 q8,q11
	
	vmovn.u16 d16, q8
	vmovn.u16 d22, q11
	
	vst1.32	d16[0], [r1], r2
	vst1.32	d16[1], [r1], r2
	vst1.32	d22[0], [r1], r2
	vst1.32	d22[1], [r1], r2
	
	add r3, #4
	cmp r3, r5
	blt Y_26_interpolation_W4

Y_26_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

.global epel_coeffs_XY_14
epel_coeffs_XY_14:
.byte 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15

function xPredIntraAngAdi_XY_14_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	vmov.u8 d26, #3
	vmov.u16 q14, #3
	vmov.u8 d30, #5
	vmov.u8 d31, #7

	cmp r5, #4
	beq xPredIntraAngAdi_XY_14_H4

	adr r12, epel_coeffs_XY_14			@ shuffle
	vld1.8 {q12}, [r12]

	sub r7, sp, #512					@ pfirst[0]
	add r8, r7, #128					@ pfirst[1]
	add r9, r8, #128					@ pfirst[2]
	add r10, r9, #128					@ pfirst[3]
	mov r12, r7
	
	sub r3, r0, r5						@ pSrc - iHeight + 3
	add r3, #3

	lsr r5, #2							@ iHeight / 4
	sub r6, r5, #1						@ left_size = line_size - iWidth = iHeight / 4 - 1

XY_14_interpolation_leftsize:
	vld1.8 {q0, q1}, [r3]
	add r3, #16
	vext.8 q2, q0, q1, #1
	vext.8 q3, q0, q1, #2

	vshll.u8 q8, d4, #1					@ pSrc[x - 1] + pSrc[x] + pSrc[x + 1]
	vaddl.u8 q9, d0, d6

	vadd.u16 q8, q8, q9

	vrshrn.u16 d22, q8, #2

	vshll.u8 q8, d5, #1					@ pSrc[x - 1] + pSrc[x] + pSrc[x + 1]
	vaddl.u8 q9, d1, d7

	vadd.u16 q8, q8, q9

	vrshrn.u16 d23, q8, #2

	vtbl.8 d0, {q11}, d24
	vtbl.8 d1, {q11}, d25

	vst1.32 d0[0], [r10]!
	vst1.32 d0[1], [r9]!
	vst1.32 d1[0], [r8]!
	vst1.32 d1[1], [r7]!

	sub r6, #4
	cmp r6, #0
	bgt XY_14_interpolation_leftsize

	sub r6, r5, #1						@ left_size = line_size - iWidth = iHeight / 4 - 1
	add r7, r12, r6
	add r8, r7, #128
	add r9, r8, #128
	add r10, r9, #128
	mov r3, #0
	sub r0, #1	
XY_14_interpolation_linesize:
	vld1.8 {d0, d1}, [r0]				@ d0:pSrc[-1]
	add r0, #8
	vext.8 d2, d0, d1, #2				@ pSrc[1]
	vext.8 d3, d0, d1, #3				@ pSrc[2]
	vext.8 d1, d0, d1, #1				@ pSrc[0]
	
	vmull.u8 q8, d1, d30				@ pSrc[0] * 5
	vmull.u8 q9, d2, d31				@ pSrc[1] * 7
	vmull.u8 q10, d3, d26				@ pSrc[2] * 3

	vaddw.u8 q8, q8, d0					@ (pSrc[-1] + pSrc[0] * 5 + pSrc[1] * 7 + pSrc[2] * 3 + 8) >> 4
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d22, q8, #4

	vst1.32 d22, [r7]!					@ store &dst1	
	
	vaddl.u8 q8, d1, d2					@ pSrc[0] + pSrc[1]

	vmul.u16 q9, q8, q14				@ pSrc[-1] + (pSrc[0] + pSrc[1]) * 3 + pSrc[2]
	vaddl.u8 q10, d0, d3
	vadd.u16 q9, q9, q10

	vaddl.u8 q10, d0, d1				@ pSrc[-1] + pSrc[0] * 2 + pSrc[1]
	vadd.u16 q8, q8, q10

	vrshrn.u16 d24, q8, #2
	vrshrn.u16 d22, q9, #3

	vst1.32 d22, [r8]!					@ store &dst2
	vst1.32 d24, [r10]!					@ store &dst4

	vmull.u8 q8, d0, d26				@ pSrc[-1] * 3
	vmull.u8 q9, d1, d31				@ pSrc[0] * 7
	vmull.u8 q10, d2, d30				@ pSrc[1] * 5

	vaddw.u8 q8, q8, d3					@ (pSrc[-1] * 3 + pSrc[0] * 7 + pSrc[1] * 5 + pSrc[2] + 8) >> 4
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d22, q8, #4

	vst1.32 d22, [r9]!					@ store &dst3	
	
	add r3, #8
	cmp r3, r4
	blt XY_14_interpolation_linesize
		
XY_14_memcpy:

	cmp r4, #15
	bgt XY_14_memcpy_16

	cmp r4, #8
	beq XY_14_memcpy_8

XY_14_memcpy_4:

	mov r7, r12
	add r8, r7, #128
	add r9, r8, #128
	add r10, r9, #128

	lsl r3, r2, #4
	sub r3, r2
	add r1, r3
	mov r6, #0
	sub r2, r6, r2

	vld1.8 {d0}, [r7]
	vld1.8 {d1}, [r8]
	vld1.8 {d2}, [r9]
	vld1.8 {d3}, [r10]

	vst1.32 d3[0], [r1], r2
	vst1.32 d2[0], [r1], r2
	vst1.32 d1[0], [r1], r2
	vst1.32 d0[0], [r1], r2

	vext.8 d4, d0, d0, #1
	vext.8 d5, d1, d1, #1
	vext.8 d6, d2, d2, #1
	vext.8 d7, d3, d3, #1

	vst1.32 d7[0], [r1], r2
	vst1.32 d6[0], [r1], r2
	vst1.32 d5[0], [r1], r2
	vst1.32 d4[0], [r1], r2

	vext.8 d4, d0, d0, #2
	vext.8 d5, d1, d1, #2
	vext.8 d6, d2, d2, #2
	vext.8 d7, d3, d3, #2

	vst1.32 d7[0], [r1], r2
	vst1.32 d6[0], [r1], r2
	vst1.32 d5[0], [r1], r2
	vst1.32 d4[0], [r1], r2

	vext.8 d4, d0, d0, #3
	vext.8 d5, d1, d1, #3
	vext.8 d6, d2, d2, #3
	vext.8 d7, d3, d3, #3

	vst1.32 d7[0], [r1], r2
	vst1.32 d6[0], [r1], r2
	vst1.32 d5[0], [r1], r2
	vst1.32 d4[0], [r1]
	
	b XY_14_end

XY_14_memcpy_8:
	
	mov r7, r12
	add r8, r7, #128
	add r9, r8, #128
	add r10, r9, #128

	cmp r5, #8
	beq XY_14_memcpy_8_H32

	lsl r3, r2, #3
	sub r3, r2
	add r1, r3
	mov r6, #0
	sub r2, r6, r2

	vld1.8 {q0}, [r7]
	vld1.8 {q1}, [r8]
	vld1.8 {q2}, [r9]
	vld1.8 {q3}, [r10]

	vst1.32 d6, [r1], r2
	vst1.32 d4, [r1], r2
	vst1.32 d2, [r1], r2
	vst1.32 d0, [r1], r2

	vext.8 d16, d0, d1, #1
	vext.8 d17, d2, d3, #1
	vext.8 d18, d4, d5, #1
	vext.8 d19, d6, d7, #1

	vst1.32 d19, [r1], r2
	vst1.32 d18, [r1], r2
	vst1.32 d17, [r1], r2
	vst1.32 d16, [r1]

	b XY_14_end

XY_14_memcpy_8_H32:

	lsl r3, r2, #5
	sub r3, r2
	add r1, r3
	mov r6, #0
	sub r2, r6, r2

	vld1.8 {q0}, [r7]
	vld1.8 {q1}, [r8]
	vld1.8 {q2}, [r9]
	vld1.8 {q3}, [r10]

	vst1.32 d6, [r1], r2
	vst1.32 d4, [r1], r2
	vst1.32 d2, [r1], r2
	vst1.32 d0, [r1], r2

	vext.8 d16, d0, d1, #1
	vext.8 d17, d2, d3, #1
	vext.8 d18, d4, d5, #1
	vext.8 d19, d6, d7, #1

	vst1.32 d19, [r1], r2
	vst1.32 d18, [r1], r2
	vst1.32 d17, [r1], r2
	vst1.32 d16, [r1], r2

	vext.8 d16, d0, d1, #2
	vext.8 d17, d2, d3, #2
	vext.8 d18, d4, d5, #2
	vext.8 d19, d6, d7, #2

	vst1.32 d19, [r1], r2
	vst1.32 d18, [r1], r2
	vst1.32 d17, [r1], r2
	vst1.32 d16, [r1], r2

	vext.8 d16, d0, d1, #3
	vext.8 d17, d2, d3, #3
	vext.8 d18, d4, d5, #3
	vext.8 d19, d6, d7, #3

	vst1.32 d19, [r1], r2
	vst1.32 d18, [r1], r2
	vst1.32 d17, [r1], r2
	vst1.32 d16, [r1], r2

	vext.8 d16, d0, d1, #4
	vext.8 d17, d2, d3, #4
	vext.8 d18, d4, d5, #4
	vext.8 d19, d6, d7, #4

	vst1.32 d19, [r1], r2
	vst1.32 d18, [r1], r2
	vst1.32 d17, [r1], r2
	vst1.32 d16, [r1], r2

	vext.8 d16, d0, d1, #5
	vext.8 d17, d2, d3, #5
	vext.8 d18, d4, d5, #5
	vext.8 d19, d6, d7, #5

	vst1.32 d19, [r1], r2
	vst1.32 d18, [r1], r2
	vst1.32 d17, [r1], r2
	vst1.32 d16, [r1], r2

	vext.8 d16, d0, d1, #6
	vext.8 d17, d2, d3, #6
	vext.8 d18, d4, d5, #6
	vext.8 d19, d6, d7, #6

	vst1.32 d19, [r1], r2
	vst1.32 d18, [r1], r2
	vst1.32 d17, [r1], r2
	vst1.32 d16, [r1], r2

	vext.8 d16, d0, d1, #7
	vext.8 d17, d2, d3, #7
	vext.8 d18, d4, d5, #7
	vext.8 d19, d6, d7, #7

	vst1.32 d19, [r1], r2
	vst1.32 d18, [r1], r2
	vst1.32 d17, [r1], r2
	vst1.32 d16, [r1], r2

	b XY_14_end

XY_14_memcpy_16:
	
	add r12, r6					@ pfirst[x] += left_size
	add r8, r12, #128
	add r9, r8, #128
	add r10, r9, #128

	add r0, r4, #1
	lsl r7, r2, #2
	sub r3, r7, r4
	sub r7, #16
XY_14_memcpy_16_y:
	mov r6, #0
XY_14_memcpy_16_x:
	vld1.8 {q0}, [r12]!
	vld1.8 {q1}, [r8]!
	vld1.8 {q2}, [r9]!
	vld1.8 {q3}, [r10]!

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2

	sub r1, r7

	add r6, #16
	cmp r6, r4
	bne XY_14_memcpy_16_x

	add r1, r3

	sub r12, r0
	sub r8, r0
	sub r9, r0
	sub r10, r0
	sub r5, #1
	cmp r5, #0
	bne XY_14_memcpy_16_y

	b XY_14_end

xPredIntraAngAdi_XY_14_H4:

	cmp	r4, #4
	beq xPredIntraAngAdi_XY_14_H4_W4

xPredIntraAngAdi_XY_14_H4_W16:
	sub r0, #1
	vld1.8 {q0, q1}, [r0]				@ q0:pSrc[-1]
	vext.8 q2, q0, q1, #2				@ pSrc[1]
	vext.8 q3, q0, q1, #3				@ pSrc[2]
	vext.8 q1, q0, q1, #1				@ pSrc[0]	

	vmull.u8 q8, d2, d30				@ pSrc[0] * 5
	vmull.u8 q9, d4, d31				@ pSrc[1] * 7
	vmull.u8 q10, d6, d26				@ pSrc[2] * 3

	vaddw.u8 q8, q8, d0					@ (pSrc[-1] + pSrc[0] * 5 + pSrc[1] * 7 + pSrc[2] * 3 + 8) >> 4
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d22, q8, #4

	vmull.u8 q8, d3, d30				@ pSrc[0] * 5
	vmull.u8 q9, d5, d31				@ pSrc[1] * 7
	vmull.u8 q10, d7, d26				@ pSrc[2] * 3

	vaddw.u8 q8, q8, d1					@ (pSrc[-1] + pSrc[0] * 5 + pSrc[1] * 7 + pSrc[2] * 3 + 8) >> 4
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d23, q8, #4

	vst1.8 {q11}, [r1], r2				@ store &dst1

	vaddl.u8 q8, d2, d4					@ pSrc[0] + pSrc[1]

	vmul.u16 q9, q8, q14				@ pSrc[-1] + (pSrc[0] + pSrc[1]) * 3 + pSrc[2]
	vaddl.u8 q10, d0, d6
	vadd.u16 q9, q9, q10

	vaddl.u8 q10, d0, d2				@ pSrc[-1] + pSrc[0] * 2 + pSrc[1]
	vadd.u16 q8, q8, q10

	vrshrn.u16 d24, q8, #2
	vrshrn.u16 d22, q9, #3

	vaddl.u8 q8, d3, d5					@ pSrc[0] + pSrc[1]

	vmul.u16 q9, q8, q14				@ pSrc[-1] + (pSrc[0] + pSrc[1]) * 3 + pSrc[2]
	vaddl.u8 q10, d1, d7
	vadd.u16 q9, q9, q10

	vaddl.u8 q10, d1, d3				@ pSrc[-1] + pSrc[0] * 2 + pSrc[1]
	vadd.u16 q8, q8, q10

	vrshrn.u16 d25, q8, #2
	vrshrn.u16 d23, q9, #3

	vst1.8 {q11}, [r1], r2				@ store &dst2
	add r6, r1, r2
	vst1.8 {q12}, [r6]					@ store &dst4

	vmull.u8 q8, d0, d26				@ pSrc[-1] * 3
	vmull.u8 q9, d2, d31				@ pSrc[0] * 7
	vmull.u8 q10, d4, d30				@ pSrc[1] * 5

	vaddw.u8 q8, q8, d6					@ (pSrc[-1] * 3 + pSrc[0] * 7 + pSrc[1] * 5 + pSrc[2] + 8) >> 4
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d22, q8, #4

	vmull.u8 q8, d1, d26				@ pSrc[-1] * 3
	vmull.u8 q9, d3, d31				@ pSrc[0] * 7
	vmull.u8 q10, d5, d30				@ pSrc[1] * 5

	vaddw.u8 q8, q8, d7					@ (pSrc[-1] * 3 + pSrc[0] * 7 + pSrc[1] * 5 + pSrc[2] + 8) >> 4
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d23, q8, #4

	vst1.32 {q11}, [r1]					@ store &dst3

	b XY_14_end

xPredIntraAngAdi_XY_14_H4_W4:
	sub r0, #1	
	vld1.8 {d0}, [r0]					@ d0:pSrc[-1]
	vext.8 d2, d0, d0, #2				@ pSrc[1]
	vext.8 d3, d0, d0, #3				@ pSrc[2]
	vext.8 d1, d0, d0, #1				@ pSrc[0]	

	vmull.u8 q8, d1, d30				@ pSrc[0] * 5
	vmull.u8 q9, d2, d31				@ pSrc[1] * 7
	vmull.u8 q10, d3, d26				@ pSrc[2] * 3

	vaddw.u8 q8, q8, d0					@ (pSrc[-1] + pSrc[0] * 5 + pSrc[1] * 7 + pSrc[2] * 3 + 8) >> 4
	vadd.u16 d16, d16, d18
	vadd.u16 d16, d16, d20

	vrshrn.u16 d22, q8, #4

	vst1.32 d22[0], [r1], r2			@ store &dst1

	vaddl.u8 q8, d1, d2					@ pSrc[0] + pSrc[1]

	vmul.u16 d18, d16, d28				@ pSrc[-1] + (pSrc[0] + pSrc[1]) * 3 + pSrc[2]
	vaddl.u8 q10, d0, d3
	vadd.u16 d18, d18, d20

	vaddl.u8 q10, d0, d1				@ pSrc[-1] + pSrc[0] * 2 + pSrc[1]
	vadd.u16 d16, d16, d20

	vrshrn.u16 d24, q8, #2
	vrshrn.u16 d22, q9, #3

	vst1.32 d22[0], [r1], r2			@ store &dst2
	add r6, r1, r2
	vst1.32 d24[0], [r6]				@ store &dst4

	vmull.u8 q8, d0, d26				@ pSrc[-1] * 3
	vmull.u8 q9, d1, d31				@ pSrc[0] * 7
	vmull.u8 q10, d2, d30				@ pSrc[1] * 5

	vaddw.u8 q8, q8, d3					@ (pSrc[-1] * 3 + pSrc[0] * 7 + pSrc[1] * 5 + pSrc[2] + 8) >> 4
	vadd.u16 d16, d16, d18
	vadd.u16 d16, d16, d20

	vrshrn.u16 d22, q8, #4

	vst1.32 d22[0], [r1]				@ store &dst3

XY_14_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function xPredIntraAngAdi_XY_16_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	vmov.u16 q14, #3

	sub r7, sp, #256			@ pfirst[0]
	add r8, r7, #128			@ pfirst[1]
	mov r12, r7

	sub r3, r0, r5				@ pSrc - iHeight + 1
	add r3, #1

	lsr r5, #1					@ iHeight / 2

	sub r6, r5, #1				@ left_size = line_size - iWidth = iHeight / 2 - 1

XY_16_interpolation_leftsize:
	vld1.8 {q0, q1}, [r3]
	add r3, #16
	vext.8 q2, q0, q1, #1
	vext.8 q3, q0, q1, #2	

	vshll.u8 q8, d4, #1					@ pSrc[x - 1] + pSrc[x]*2 + pSrc[x + 1]
	vaddl.u8 q9, d0, d6

	vadd.u16 q8, q8, q9

	vrshrn.u16 d22, q8, #2

	vshll.u8 q8, d5, #1					@ pSrc[x - 1] + pSrc[x] + pSrc[x + 1]
	vaddl.u8 q9, d1, d7

	vadd.u16 q8, q8, q9

	vrshrn.u16 d23, q8, #2

	vuzp.8 d22, d23

	vst1.32 d22, [r8]!
	vst1.32 d23, [r7]!

	sub r6, #8
	cmp r6, #0
	bgt XY_16_interpolation_leftsize

	sub r6, r5, #1						@ left_size = line_size - iWidth = iHeight / 4 - 1
	add r7, r12, r6
	add r8, r7, #128
	mov r3, #0
	sub r0, #1	
XY_16_interpolation_linesize:
	vld1.8 {d0, d1}, [r0]				@ d0:pSrc[-1]
	add r0, #8
	vext.8 d2, d0, d1, #2				@ pSrc[1]
	vext.8 d3, d0, d1, #3				@ pSrc[2]
	vext.8 d1, d0, d1, #1				@ pSrc[0]

	vaddl.u8 q8, d1, d2					@ pSrc[0] + pSrc[1]

	vmul.u16 q9, q8, q14				@ pSrc[-1] + (pSrc[0] + pSrc[1]) * 3 + pSrc[2]
	vaddl.u8 q10, d0, d3
	vadd.u16 q9, q9, q10

	vaddl.u8 q10, d0, d1				@ pSrc[-1] + pSrc[0] * 2 + pSrc[1]
	vadd.u16 q8, q8, q10

	vrshrn.u16 d24, q8, #2
	vrshrn.u16 d22, q9, #3

	vst1.32 d22, [r7]!					@ store &dst1
	vst1.32 d24, [r8]!					@ store &dst2

	add r3, #8
	cmp r3, r4
	blt XY_16_interpolation_linesize

XY_16_memcpy:

	cmp r4, #15
	bgt XY_16_memcpy_16

	cmp r4, #8
	beq XY_16_memcpy_8

XY_16_memcpy_4:

	mov r7, r12
	add r8, r7, #128

	cmp r5, #2
	bne XY_16_memcpy_4_H16

XY_16_memcpy_4_H4:
	vld1.8 d0, [r7]
	vld1.8 d1, [r8]

	vext.8 d2, d0, d0, #1
	vext.8 d3, d1, d1, #1

	vst1.32 d2[0], [r1], r2
	vst1.32 d3[0], [r1], r2
	vst1.32 d0[0], [r1], r2
	vst1.32 d1[0], [r1]

	b XY_16_end

XY_16_memcpy_4_H16:
	vld1.8 {q0}, [r7]
	vld1.8 {q1}, [r8]

	vext.8 d4, d0, d1, #7
	vext.8 d5, d2, d3, #7

	vst1.32 d4[0], [r1], r2
	vst1.32 d5[0], [r1], r2

	vext.8 d4, d0, d1, #6
	vext.8 d5, d2, d3, #6

	vst1.32 d4[0], [r1], r2
	vst1.32 d5[0], [r1], r2

	vext.8 d4, d0, d1, #5
	vext.8 d5, d2, d3, #5

	vst1.32 d4[0], [r1], r2
	vst1.32 d5[0], [r1], r2

	vext.8 d4, d0, d1, #4
	vext.8 d5, d2, d3, #4

	vst1.32 d4[0], [r1], r2
	vst1.32 d5[0], [r1], r2

	vext.8 d4, d0, d1, #3
	vext.8 d5, d2, d3, #3

	vst1.32 d4[0], [r1], r2
	vst1.32 d5[0], [r1], r2

	vext.8 d4, d0, d1, #2
	vext.8 d5, d2, d3, #2

	vst1.32 d4[0], [r1], r2
	vst1.32 d5[0], [r1], r2

	vext.8 d4, d0, d1, #1
	vext.8 d5, d2, d3, #1

	vst1.32 d4[0], [r1], r2
	vst1.32 d5[0], [r1], r2

	vst1.32 d0[0], [r1], r2
	vst1.32 d2[0], [r1]

	b XY_16_end

XY_16_memcpy_8:
	mov r7, r12
	add r8, r7, #128

	cmp r5, #16
	beq XY_16_memcpy_8_H32

XY_16_memcpy_8_H8:
	vld1.8 {q0}, [r7]
	vld1.8 {q1}, [r8]

	vext.8 d4, d0, d1, #3
	vext.8 d5, d2, d3, #3

	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2

	vext.8 d4, d0, d1, #2
	vext.8 d5, d2, d3, #2

	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2

	vext.8 d4, d0, d1, #1
	vext.8 d5, d2, d3, #1

	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2

	vst1.32 d0, [r1], r2
	vst1.32 d2, [r1]
	
	b XY_16_end

XY_16_memcpy_8_H32:
	vld1.8 {q0, q1}, [r7]
	vld1.8 {q2, q3}, [r8]

	vext.8 d16, d1, d2, #7
	vext.8 d17, d5, d6, #7

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d1, d2, #6
	vext.8 d17, d5, d6, #6

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d1, d2, #5
	vext.8 d17, d5, d6, #5

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d1, d2, #4
	vext.8 d17, d5, d6, #4

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d1, d2, #3
	vext.8 d17, d5, d6, #3

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d1, d2, #2
	vext.8 d17, d5, d6, #2

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d1, d2, #1
	vext.8 d17, d5, d6, #1

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vst1.32 d1, [r1], r2
	vst1.32 d5, [r1], r2

	vext.8 d16, d0, d1, #7
	vext.8 d17, d4, d5, #7

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d0, d1, #6
	vext.8 d17, d4, d5, #6

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d0, d1, #5
	vext.8 d17, d4, d5, #5

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d0, d1, #4
	vext.8 d17, d4, d5, #4

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d0, d1, #3
	vext.8 d17, d4, d5, #3

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d0, d1, #2
	vext.8 d17, d4, d5, #2

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vext.8 d16, d0, d1, #1
	vext.8 d17, d4, d5, #1

	vst1.32 d16, [r1], r2
	vst1.32 d17, [r1], r2

	vst1.32 d0, [r1], r2
	vst1.32 d4, [r1]

	b XY_16_end

XY_16_memcpy_16:
	
	add r12, r6					@ pfirst[x] += left_size
	add r8, r12, #128

	add r0, r4, #1
	lsl r7, r2, #1
	sub r3, r7, r4
	sub r7, #16
XY_16_memcpy_16_y:
	mov r6, #0
XY_16_memcpy_16_x:
	vld1.8 {q0}, [r12]!
	vld1.8 {q1}, [r8]!

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2

	sub r1, r7

	add r6, #16
	cmp r6, r4
	bne XY_16_memcpy_16_x

	add r1, r3

	sub r12, r0
	sub r8, r0
	sub r5, #1
	cmp r5, #0
	bne XY_16_memcpy_16_y

XY_16_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_XY_18_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	sub r7, sp, #256
	mov r12, r7

	add r6, r4, r5              @ line_size = iWidth + iHeight

	sub r0, r0, r5				@ pSrc -= iHeight

XY_18_interpolation_linesize:
	vld1.8 {q0, q1}, [r0]
	add r0, #16
	vext.8 q2, q0, q1, #1
	vext.8 q3, q0, q1, #2	

	vshll.u8 q8, d4, #1					@ pSrc[x - 1] + pSrc[x]*2 + pSrc[x + 1]
	vaddl.u8 q9, d0, d6

	vadd.u16 q8, q8, q9

	vrshrn.u16 d22, q8, #2

	vshll.u8 q8, d5, #1					@ pSrc[x - 1] + pSrc[x] + pSrc[x + 1]
	vaddl.u8 q9, d1, d7

	vadd.u16 q8, q8, q9

	vrshrn.u16 d23, q8, #2

	vst1.8 {q11}, [r7]!

	sub r6, #16
	cmp r6, #0
	bgt XY_18_interpolation_linesize

XY_18_memcpy:

	cmp r4, #15
	bgt XY_18_memcpy_16

	cmp r4, #8
	beq XY_18_memcpy_8

XY_18_memcpy_4:

	mov r7, r12

	cmp r5, #4
	bne XY_18_memcpy_4_H16	

XY_18_memcpy_4_H4:
	vld1.8 d0, [r7]
	
	vext.8 d1, d0, d0 ,#3
	vst1.32 d1[0], [r1], r2

	vext.8 d1, d0, d0 ,#2
	vst1.32 d1[0], [r1], r2

	vext.8 d1, d0, d0 ,#1
	vst1.32 d1[0], [r1], r2

	vst1.32 d0[0], [r1]

	b XY_18_end

XY_18_memcpy_4_H16:
	vld1.8 {q0, q1}, [r7]

	vext.8 q8, q0, q1, #1
	vext.8 q9, q0, q1, #2
	vext.8 q10, q0, q1, #3

	vst1.32 d21[1], [r1], r2
	vst1.32 d19[1], [r1], r2
	vst1.32 d17[1], [r1], r2
	vst1.32 d1[1], [r1], r2

	vst1.32 d21[0], [r1], r2
	vst1.32 d19[0], [r1], r2
	vst1.32 d17[0], [r1], r2
	vst1.32 d1[0], [r1], r2

	vst1.32 d20[1], [r1], r2
	vst1.32 d18[1], [r1], r2
	vst1.32 d16[1], [r1], r2
	vst1.32 d0[1], [r1], r2

	vst1.32 d20[0], [r1], r2
	vst1.32 d18[0], [r1], r2
	vst1.32 d16[0], [r1], r2
	vst1.32 d0[0], [r1]

	b XY_18_end

XY_18_memcpy_8:

	mov r7, r12

	cmp r5, #8
	bne XY_18_memcpy_8_H32
	
XY_18_memcpy_8_H8:
	vld1.8 {q0}, [r7]

	vext.8 d2, d0, d1 ,#7
	vst1.32 d2, [r1], r2

	vext.8 d2, d0, d1 ,#6
	vst1.32 d2, [r1], r2

	vext.8 d2, d0, d1 ,#5
	vst1.32 d2, [r1], r2

	vext.8 d2, d0, d1 ,#4
	vst1.32 d2, [r1], r2

	vext.8 d2, d0, d1 ,#3
	vst1.32 d2, [r1], r2

	vext.8 d2, d0, d1 ,#2
	vst1.32 d2, [r1], r2

	vext.8 d2, d0, d1 ,#1
	vst1.32 d2, [r1], r2

	vst1.32 d0, [r1]

	b XY_18_end

XY_18_memcpy_8_H32:
	vld1.8 {q0, q1}, [r7]!
	vld1.8 {q2}, [r7]

	vext.8 q8, q1, q2, #1
	vext.8 q9, q1, q2, #2
	vext.8 q10, q1, q2, #3
	vext.8 q11, q1, q2, #4
	vext.8 q12, q1, q2, #5
	vext.8 q13, q1, q2, #6
	vext.8 q14, q1, q2, #7

	vst1.8 d29, [r1], r2
	vst1.8 d27, [r1], r2
	vst1.8 d25, [r1], r2
	vst1.8 d23, [r1], r2
	vst1.8 d21, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d3, [r1], r2

	vst1.8 d28, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d24, [r1], r2
	vst1.8 d22, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d2, [r1], r2

	vext.8 q8, q0, q1, #1
	vext.8 q9, q0, q1, #2
	vext.8 q10, q0, q1, #3
	vext.8 q11, q0, q1, #4
	vext.8 q12, q0, q1, #5
	vext.8 q13, q0, q1, #6
	vext.8 q14, q0, q1, #7

	vst1.8 d29, [r1], r2
	vst1.8 d27, [r1], r2
	vst1.8 d25, [r1], r2
	vst1.8 d23, [r1], r2
	vst1.8 d21, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d1, [r1], r2

	vst1.8 d28, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d24, [r1], r2
	vst1.8 d22, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d0, [r1]

	b XY_18_end

XY_18_memcpy_16:

	add r12, r5
	sub r12, #1

	add r0, r4, #1
	sub r3, r2, r4
	sub r7, r2, #16
XY_18_memcpy_16_y:
	mov r6, #0
XY_18_memcpy_16_x:
	vld1.8 {q0}, [r12]!

	vst1.32 {q0}, [r1], r2

	sub r1, r7

	add r6, #16
	cmp r6, r4
	bne XY_18_memcpy_16_x

	add r1, r3

	sub r12, r0
	sub r5, #1
	cmp r5, #0
	bne XY_18_memcpy_16_y

XY_18_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_XY_20_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40
	
	vmov.u16 q14, #3

	sub r12, sp, #256
	mov r10, r12

	lsl r6, r5, #1					@ left_size = (iHeight - 1) * 2 + 1
	sub r6, #1

	sub r7, r4, #1					@ top_size = iWidth - 1

	add r8, r7, r6					@ line_size = left_size + top_size

	sub r0, #1						@  pSrc - iHeight - 1
	sub r3, r0, r5
	mov r9, #0
XY_20_interpolation_leftsize:
	vld1.8 {q0}, [r3]				@ d0:pSrc[-1]
	add r3, #8
	vext.8 d2, d0, d1, #2			@ d2:pSrc[1]
	vext.8 d3, d0, d1, #3			@ d3:pSrc[2]
	vext.8 d1, d0, d1, #1			@ d1:pSrc[0]

	vaddl.u8 q8, d1, d2				@ pSrc[-1] + (pSrc[0] + pSrc[1]) * 3 + pSrc[2]
	vmul.u16 q9, q8, q14
	vaddl.u8 q10, d0, d3
	vadd.u16 q9, q9, q10

	vaddl.u8 q11, d2, d3			@ pSrc[0] + pSrc[1] * 2 + pSrc[2] 
	vadd.u16 q8, q8, q11

	vrshrn.u16 d23, q8, #2
	vrshrn.u16 d22, q9, #3

	vst2.8 {d22, d23}, [r12]!

	add r9, #16
	cmp r9, r6
	blt XY_20_interpolation_leftsize

	add r12, r10, r6
XY_20_interpolation_linesize:
	vld1.8 {q0}, [r0]				@ d0:pSrc[-1]
	add r0, #8
	vext.8 d2, d0, d1, #2			@ d2:pSrc[1]
	vext.8 d1, d0, d1, #1			@ d1:pSrc[0]

	vaddl.u8 q8, d0, d1				@ (pSrc[-1] + pSrc[0] * 2 + pSrc[1] + 2) >> 2
	vaddl.u8 q9, d2, d1
	vadd.u16 q8, q8, q9

	vrshrn.u16 d0, q8, #2

	vst1.8 d0, [r12]!

	sub r7, #8
	cmp r7, #0
	bgt XY_20_interpolation_linesize

XY_20_memcpy:

	cmp r4, #15
	bgt XY_20_memcpy_16

	cmp r4, #8
	beq XY_20_memcpy_8

XY_20_memcpy_4:
	cmp r5, #4
	bne XY_20_memcpy_4_H16

XY_20_memcpy_4_H4:
	vld1.8 {q0}, [r10]

	vext.8 q1, q0, q0, #2

	vst1.32 d2[1], [r1], r2
	vst1.32 d0[1], [r1], r2
	vst1.32 d2[0], [r1], r2
	vst1.32 d0[0], [r1]
	
	b XY_20_end

XY_20_memcpy_4_H16:
	vld1.8 {q0, q1}, [r10]!	
	vld1.8 {q2}, [r10]

	vext.8 q3, q1, q2, #2

	vst1.32 d7[1], [r1], r2
	vst1.32 d3[1], [r1], r2
	vst1.32 d7[0], [r1], r2
	vst1.32 d3[0], [r1], r2
	vst1.32 d6[1], [r1], r2
	vst1.32 d2[1], [r1], r2
	vst1.32 d6[0], [r1], r2
	vst1.32 d2[0], [r1], r2

	vext.8 q3, q0, q1, #2

	vst1.32 d7[1], [r1], r2
	vst1.32 d1[1], [r1], r2
	vst1.32 d7[0], [r1], r2
	vst1.32 d1[0], [r1], r2
	vst1.32 d6[1], [r1], r2
	vst1.32 d0[1], [r1], r2
	vst1.32 d6[0], [r1], r2
	vst1.32 d0[0], [r1]

	b XY_20_end

XY_20_memcpy_8:
	cmp r5, #8
	bne XY_20_memcpy_8_H32

XY_20_memcpy_8_H8:
	vld1.8 {q0, q1}, [r10]

	vext.8 q8, q0, q1, #2
	vext.8 q9, q0, q1, #4
	vext.8 q10, q0, q1, #6

	vst1.8 d21, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d1, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d0, [r1], r2

	b XY_20_end

XY_20_memcpy_8_H32:
	vld1.8 {q0, q1}, [r10]!
	vld1.8 {q2, q3}, [r10]!
	vld1.8 {q11}, [r10]!

	vext.8 q8, q3, q11, #2
	vext.8 q9, q3, q11, #4
	vext.8 q10, q3, q11, #6

	vst1.8 d21, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d7, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d6, [r1], r2

	vext.8 q8, q2, q3, #2
	vext.8 q9, q2, q3, #4
	vext.8 q10, q2, q3, #6

	vst1.8 d21, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d5, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d4, [r1], r2

	vext.8 q8, q1, q2, #2
	vext.8 q9, q1, q2, #4
	vext.8 q10, q1, q2, #6

	vst1.8 d21, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d3, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d2, [r1], r2

	vext.8 q8, q0, q1, #2
	vext.8 q9, q0, q1, #4
	vext.8 q10, q0, q1, #6

	vst1.8 d21, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d1, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d0, [r1], r2

	b XY_20_end

XY_20_memcpy_16:

	add r12, r10, r6
	sub r12, #1

	add r0, r4, #2
	sub r3, r2, r4
XY_20_memcpy_16_y:
	mov r6, #0
XY_20_memcpy_16_x:
	vld1.8 {q0}, [r12]!

	vst1.32 {q0}, [r1]!

	add r6, #16
	cmp r6, r4
	bne XY_20_memcpy_16_x

	add r1, r3

	sub r12, r0
	sub r5, #1
	cmp r5, #0
	bne XY_20_memcpy_16_y

XY_20_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function xPredIntraAngAdi_XY_22_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	vmov.u8 d26, #3
	vmov.u16 q14, #3
	vmov.u8 d30, #5
	vmov.u8 d31, #7

	cmp r4, #4
	beq xPredIntraAngAdi_XY_22_W4

	sub r0, #1
	sub r3, r0, r5

	sub r12, sp, #512
	mov r10, r12

	lsl r6, r5, #2					@ left_size = (iHeight - 1) * 4 + 3
	sub r6, #1

	sub r7, r4, #3					@ top_size = iWidth - 3

	add r8, r6, r7					@ line_size = left_size + top_size

	mov r9, #0
XY_22_interpolation_leftsize:
	vld1.8 {q0}, [r3]				@ d0:pSrc[-1]
	add r3, #8
	vext.8 d2, d0, d1, #2			@ d2:pSrc[1]
	vext.8 d3, d0, d1, #3			@ d3:pSrc[2]
	vext.8 d1, d0, d1, #1			@ d1:pSrc[0]

	vmull.u8 q8, d0, d26			@ (pSrc[-1] * 3 + pSrc[0] * 7 + pSrc[1] * 5 + pSrc[2] + 8) >> 4
	vmull.u8 q9, d1, d31
	vmull.u8 q10, d2, d30

	vaddw.u8 q8, q8, d3
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d4, q8, #4

	vaddl.u8 q8, d1, d2				@ (pSrc[-1] + (pSrc[0] + pSrc[1]) * 3 + pSrc[2] + 4) >> 3
	vmul.u16 q9, q8, q14
	vaddl.u8 q10, d0, d3

	vadd.u16 q9, q9, q10

	vrshrn.u16 d5, q9, #3

	vaddl.u8 q9, d2, d3				@ (pSrc[0] + pSrc[1] * 2 + pSrc[2] + 2) >> 2
	vadd.u16 q9, q8, q9

	vrshrn.u16 d7, q9, #2

	vmull.u8 q8, d3, d26			@ (pSrc[-1] + pSrc[0] * 5 + pSrc[1] * 7 + pSrc[2] * 3 + 8) >> 4
	vmull.u8 q9, d1, d30
	vmull.u8 q10, d2, d31

	vaddw.u8 q8, q8, d0
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d6, q8, #4

	vst4.8 {d4, d5, d6, d7}, [r12]!

	add r9, #32
	cmp r9, r6
	blt XY_22_interpolation_leftsize

	add r12, r10, r6
XY_22_interpolation_linesize:
	vld1.8 {q0}, [r0]				@ d0:pSrc[-1]
	add r0, #8
	vext.8 d2, d0, d1, #2			@ d2:pSrc[1]
	vext.8 d1, d0, d1, #1			@ d1:pSrc[0]

	vaddl.u8 q8, d0, d1				@ (pSrc[-1] + pSrc[0] * 2 + pSrc[1] + 2) >> 2
	vaddl.u8 q9, d2, d1
	vadd.u16 q8, q8, q9

	vrshrn.u16 d0, q8, #2

	vst1.8 d0, [r12]!

	sub r7, #8
	cmp r7, #0
	bgt XY_22_interpolation_linesize
	
XY_22_memcpy:

	cmp r4, #15
	bgt XY_22_memcpy_16

XY_22_memcpy_8:
	
	cmp r5, #8
	bne XY_22_memcpy_8_H32

XY_22_memcpy_8_H8:
	
	vld1.8 {q0, q1}, [r10]!
	vld1.8 {d4}, [r10]

	vext.8 d6, d3, d4, #4
	vst1.8 d6, [r1], r2
	vst1.8 d3, [r1], r2

	vext.8 d6, d2, d3, #4
	vst1.8 d6, [r1], r2
	vst1.8 d2, [r1], r2

	vext.8 d6, d1, d2, #4
	vst1.8 d6, [r1], r2
	vst1.8 d1, [r1], r2

	vext.8 d6, d0, d1, #4
	vst1.8 d6, [r1], r2
	vst1.8 d0, [r1]

	b XY_22_end

XY_22_memcpy_8_H32:
	vld1.8 {q0, q1}, [r10]!
	vld1.8 {q2, q3}, [r10]!
	vld1.8 {q8, q9}, [r10]!
	vld1.8 {q10, q11}, [r10]!
	vld1.8 {q12}, [r10]!

	vext.8 q13, q11, q12, #4
	vst1.8 d27, [r1], r2
	vst1.8 d23, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d22, [r1], r2

	vext.8 q13, q10, q11, #4
	vst1.8 d27, [r1], r2
	vst1.8 d21, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d20, [r1], r2

	vext.8 q13, q9, q10, #4
	vst1.8 d27, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d18, [r1], r2

	vext.8 q13, q8, q9, #4
	vst1.8 d27, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d16, [r1], r2

	vext.8 q13, q3, q8, #4
	vst1.8 d27, [r1], r2
	vst1.8 d7, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d6, [r1], r2

	vext.8 q13, q2, q3, #4
	vst1.8 d27, [r1], r2
	vst1.8 d5, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d4, [r1], r2

	vext.8 q13, q1, q2, #4
	vst1.8 d27, [r1], r2
	vst1.8 d3, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d2, [r1], r2

	vext.8 q13, q0, q1, #4
	vst1.8 d27, [r1], r2
	vst1.8 d1, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d0, [r1], r2

	b XY_22_end

XY_22_memcpy_16:

	add r12, r10, r6
	sub r12, #3

	add r0, r4, #4
	sub r3, r2, r4
XY_22_memcpy_16_y:
	mov r6, #0
XY_22_memcpy_16_x:
	vld1.8 {q0}, [r12]!

	vst1.32 {q0}, [r1]!

	add r6, #16
	cmp r6, r4
	bne XY_22_memcpy_16_x

	add r1, r3

	sub r12, r0
	sub r5, #1
	cmp r5, #0
	bne XY_22_memcpy_16_y

	b XY_22_end

xPredIntraAngAdi_XY_22_W4:
	cmp r5, #4
	bne xPredIntraAngAdi_XY_22_W4_H16

xPredIntraAngAdi_XY_22_W4_H4:
	sub r0, #1
	sub r0, r5
	vld1.8 {q0}, [r0]				@ d0:pSrc[-1]
	vext.8 d2, d0, d1, #2			@ d2:pSrc[1]
	vext.8 d3, d0, d1, #3			@ d3:pSrc[2]
	vext.8 d1, d0, d1, #1			@ d1:pSrc[0]

	vmull.u8 q8, d0, d26			@ (pSrc[-1] * 3 + pSrc[0] * 7 + pSrc[1] * 5 + pSrc[2] + 8) >> 4
	vmull.u8 q9, d1, d31
	vmull.u8 q10, d2, d30

	vaddw.u8 q8, q8, d3
	vadd.u16 d16, d16, d18
	vadd.u16 d16, d16, d20

	vrshrn.u16 d4, q8, #4

	vaddl.u8 q8, d1, d2				@ (pSrc[-1] + (pSrc[0] + pSrc[1]) * 3 + pSrc[2] + 4) >> 3
	vmul.u16 q9, q8, q14
	vaddl.u8 q10, d0, d3

	vadd.u16 d18, d20, d18

	vrshrn.u16 d5, q9, #3

	vaddl.u8 q9, d2, d3				@ (pSrc[0] + pSrc[1] * 2 + pSrc[2] + 2) >> 2
	vadd.u16 d18, d16, d18

	vrshrn.u16 d7, q9, #2

	vmull.u8 q8, d3, d26			@ (pSrc[-1] + pSrc[0] * 5 + pSrc[1] * 7 + pSrc[2] * 3 + 8) >> 4
	vmull.u8 q9, d1, d30
	vmull.u8 q10, d2, d31

	vaddw.u8 q8, q8, d0
	vadd.u16 d16, d16, d18
	vadd.u16 d16, d16, d20

	vrshrn.u16 d6, q8, #4

	vzip.8 q2, q3
	vzip.8 d4, d6

	vst1.32 d6[1], [r1], r2
	vst1.32 d6[0], [r1], r2
	vst1.32 d4[1], [r1], r2
	vst1.32 d4[0], [r1]

	b XY_22_end

xPredIntraAngAdi_XY_22_W4_H16:
	sub r0, #1
	sub r0, r5
	vld1.8 {q0, q1}, [r0]			@ d0:pSrc[-1]
	vext.8 q2, q0, q1, #2			@ d2:pSrc[1]
	vext.8 q3, q0, q1, #3			@ d3:pSrc[2]
	vext.8 q1, q0, q1, #1			@ d1:pSrc[0]	

	vmull.u8 q8, d1, d26			@ (pSrc[-1] * 3 + pSrc[0] * 7 + pSrc[1] * 5 + pSrc[2] + 8) >> 4
	vmull.u8 q9, d3, d31
	vmull.u8 q10, d5, d30

	vaddw.u8 q8, q8, d7
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d22, q8, #4

	vaddl.u8 q8, d3, d5				@ (pSrc[-1] + (pSrc[0] + pSrc[1]) * 3 + pSrc[2] + 4) >> 3
	vmul.u16 q9, q8, q14
	vaddl.u8 q10, d1, d7

	vadd.u16 q9, q9, q10

	vrshrn.u16 d23, q9, #3

	vaddl.u8 q9, d5, d7				@ (pSrc[0] + pSrc[1] * 2 + pSrc[2] + 2) >> 2
	vadd.u16 q9, q9, q8

	vrshrn.u16 d25, q9, #2

	vmull.u8 q8, d7, d26			@ (pSrc[-1] + pSrc[0] * 5 + pSrc[1] * 7 + pSrc[2] * 3 + 8) >> 4
	vmull.u8 q9, d3, d30
	vmull.u8 q10, d5, d31

	vaddw.u8 q8, q8, d1
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d24, q8, #4

	vzip.8 q11, q12
	vzip.8 q11, q12

	vst1.32 d25[1], [r1], r2
	vst1.32 d25[0], [r1], r2
	vst1.32 d24[1], [r1], r2
	vst1.32 d24[0], [r1], r2
	vst1.32 d23[1], [r1], r2
	vst1.32 d23[0], [r1], r2
	vst1.32 d22[1], [r1], r2
	vst1.32 d22[0], [r1], r2

	vmull.u8 q8, d0, d26			@ (pSrc[-1] * 3 + pSrc[0] * 7 + pSrc[1] * 5 + pSrc[2] + 8) >> 4
	vmull.u8 q9, d2, d31
	vmull.u8 q10, d4, d30

	vaddw.u8 q8, q8, d6
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d22, q8, #4

	vaddl.u8 q8, d2, d4				@ (pSrc[-1] + (pSrc[0] + pSrc[1]) * 3 + pSrc[2] + 4) >> 3
	vmul.u16 q9, q8, q14
	vaddl.u8 q10, d0, d6

	vadd.u16 q9, q9, q10

	vrshrn.u16 d23, q9, #3

	vaddl.u8 q9, d4, d6				@ (pSrc[0] + pSrc[1] * 2 + pSrc[2] + 2) >> 2
	vadd.u16 q9, q9, q8

	vrshrn.u16 d25, q9, #2

	vmull.u8 q8, d6, d26			@ (pSrc[-1] + pSrc[0] * 5 + pSrc[1] * 7 + pSrc[2] * 3 + 8) >> 4
	vmull.u8 q9, d2, d30
	vmull.u8 q10, d4, d31

	vaddw.u8 q8, q8, d0
	vadd.u16 q8, q8, q9
	vadd.u16 q8, q8, q10

	vrshrn.u16 d24, q8, #4

	vzip.8 q11, q12
	vzip.8 q11, q12

	vst1.32 d25[1], [r1], r2
	vst1.32 d25[0], [r1], r2
	vst1.32 d24[1], [r1], r2
	vst1.32 d24[0], [r1], r2
	vst1.32 d23[1], [r1], r2
	vst1.32 d23[0], [r1], r2
	vst1.32 d22[1], [r1], r2
	vst1.32 d22[0], [r1]

XY_22_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_X_3_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	lsl r6, r4, #1				@ iWidth2 = iWidth << 1
	mov r8, #0

	and r12, r4, #15
	cmp r12, #0
	beq xPredIntraAngAdi_X_3_Y_W16

	and r12, r4, #7
	cmp r12, #0
	beq xPredIntraAngAdi_X_3_Y_W8


xPredIntraAngAdi_X_3_Y_W4:

	add r8, r8, #11				@ iTempDn = iTempD * imult
	lsr r9, r8, #2				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #2
	lsr r12, r8, #2
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_3_lezero_W4

xPredIntraAngAdi_X_3_gtzero_W4:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 d4, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d4, d4, #1
	vext.8 d6, d4, d4, #2
	vext.8 d7, d4, d4, #3

	vmull.u8 q8, d0, d4			@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0[0], [r1]

	cmp r12, r4
	beq xPredIntraAngAdi_X_3_gtzero_W4_end

	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0[0], [r7]

xPredIntraAngAdi_X_3_gtzero_W4_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_3_Y_W4
	b xPredIntraAngAdi_X_3_Y_end

xPredIntraAngAdi_X_3_lezero_W4:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2] - pSrc[iWidth2 + 3]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2] * c1 + pSrc[iWidth2 + 1] * c2 + pSrc[iWidth2 + 2] * c3 + pSrc[iWidth2 + 3] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0[0], [r1]

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_3_Y_W4
	b xPredIntraAngAdi_X_3_Y_end

xPredIntraAngAdi_X_3_Y_W8:

	add r8, r8, #11				@ iTempDn = iTempD * imult
	lsr r9, r8, #2				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #2
	lsr r12, r8, #2
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_3_lezero_W8

xPredIntraAngAdi_X_3_gtzero_W8:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 {q10}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d20, d21, #1
	vext.8 d6, d20, d21, #2
	vext.8 d7, d20, d21, #3

	vmull.u8 q8, d0, d20		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0, [r1]

	cmp r12, r4
	beq xPredIntraAngAdi_X_3_gtzero_W8_end

	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0, [r7]

xPredIntraAngAdi_X_3_gtzero_W8_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_3_Y_W8
	b xPredIntraAngAdi_X_3_Y_end

xPredIntraAngAdi_X_3_lezero_W8:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2] * c1 + pSrc[iWidth2 + 1] * c2 + pSrc[iWidth2 + 2] * c3 + pSrc[iWidth2 + 3] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0, [r1]

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_3_Y_W8
	b xPredIntraAngAdi_X_3_Y_end

	
xPredIntraAngAdi_X_3_Y_W16:

	add r8, r8, #11				@ iTempDn = iTempD * imult
	lsr r9, r8, #2				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #2
	lsr r12, r8, #2
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_3_lezero_W16

xPredIntraAngAdi_X_3_gtzero_W16:

	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	mov r10, #0					@ x = 0

xPredIntraAngAdi_X_3_gtzero_W16_LoopX:
	add r7, r0, r9
	vld1.32 {q8, q9}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 q10, q8, q9, #1
	vext.8 q11, q8, q9, #2
	vext.8 q12, q8, q9, #3

	vmull.u8 q13, d0, d16		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d20
	vmlal.u8 q13, d2, d22
	vmlal.u8 q13, d3, d24
	vrshrn.u16 d28, q13, #7

	vmull.u8 q13, d0, d17		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d21
	vmlal.u8 q13, d2, d23
	vmlal.u8 q13, d3, d25
	vrshrn.u16 d29, q13, #7

	add r7, r1, r10
	vst1.32 {q14}, [r7]

	add r10, #16
	add r9, #16
	cmp r10, r12
	blt xPredIntraAngAdi_X_3_gtzero_W16_LoopX

	cmp r12, r4
	beq xPredIntraAngAdi_X_3_gtzero_W16_end


	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 q0, r11

xPredIntraAngAdi_X_3_gtzero_W16_LoopX2:
	vst1.32 {q0}, [r7]!
	
	add r12, #16
	cmp r12, r4
	blt xPredIntraAngAdi_X_3_gtzero_W16_LoopX2

xPredIntraAngAdi_X_3_gtzero_W16_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_3_Y_W16
	b xPredIntraAngAdi_X_3_Y_end

xPredIntraAngAdi_X_3_lezero_W16:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2] * c1 + pSrc[iWidth2 + 1] * c2 + pSrc[iWidth2 + 2] * c3 + pSrc[iWidth2 + 3] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vmov d1, d0
	
	mov r7, r1
	mov r10, #0	
xPredIntraAngAdi_X_3_lezero_W16_loop:

	vst1.32 {q0}, [r7]!

	add r10, #16
	cmp r10, r4
	blt xPredIntraAngAdi_X_3_lezero_W16_loop

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_3_Y_W16

xPredIntraAngAdi_X_3_Y_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_X_5_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	lsl r6, r4, #1				@ iWidth2 = iWidth << 1
	mov r8, #0

	and r12, r4, #15
	cmp r12, #0
	beq xPredIntraAngAdi_X_5_Y_W16

	and r12, r4, #7
	cmp r12, #0
	beq xPredIntraAngAdi_X_5_Y_W8


xPredIntraAngAdi_X_5_Y_W4:

	add r8, r8, #11				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_5_lezero_W4

xPredIntraAngAdi_X_5_gtzero_W4:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 d4, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d4, d4, #1
	vext.8 d6, d4, d4, #2
	vext.8 d7, d4, d4, #3

	vmull.u8 q8, d0, d4			@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0[0], [r1]

	cmp r12, r4
	beq xPredIntraAngAdi_X_5_gtzero_W4_end

	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0[0], [r7]

xPredIntraAngAdi_X_5_gtzero_W4_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_5_Y_W4
	b xPredIntraAngAdi_X_5_Y_end

xPredIntraAngAdi_X_5_lezero_W4:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2] * c1 + pSrc[iWidth2 + 1] * c2 + pSrc[iWidth2 + 2] * c3 + pSrc[iWidth2 + 3] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0[0], [r1]

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_5_Y_W4
	b xPredIntraAngAdi_X_5_Y_end

xPredIntraAngAdi_X_5_Y_W8:

	add r8, r8, #11				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_5_lezero_W8

xPredIntraAngAdi_X_5_gtzero_W8:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 {q10}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d20, d21, #1
	vext.8 d6, d20, d21, #2
	vext.8 d7, d20, d21, #3

	vmull.u8 q8, d0, d20		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0, [r1]

	cmp r12, r4
	beq xPredIntraAngAdi_X_5_gtzero_W8_end

	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0, [r7]

xPredIntraAngAdi_X_5_gtzero_W8_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_5_Y_W8
	b xPredIntraAngAdi_X_5_Y_end

xPredIntraAngAdi_X_5_lezero_W8:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2] * c1 + pSrc[iWidth2 + 1] * c2 + pSrc[iWidth2 + 2] * c3 + pSrc[iWidth2 + 3] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0, [r1]

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_5_Y_W8
	b xPredIntraAngAdi_X_5_Y_end

	
xPredIntraAngAdi_X_5_Y_W16:

	add r8, r8, #11				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_5_lezero_W16

xPredIntraAngAdi_X_5_gtzero_W16:

	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	mov r10, #0					@ x = 0

xPredIntraAngAdi_X_5_gtzero_W16_LoopX:
	add r7, r0, r9
	vld1.32 {q8, q9}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 q10, q8, q9, #1
	vext.8 q11, q8, q9, #2
	vext.8 q12, q8, q9, #3

	vmull.u8 q13, d0, d16		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d20
	vmlal.u8 q13, d2, d22
	vmlal.u8 q13, d3, d24
	vrshrn.u16 d28, q13, #7

	vmull.u8 q13, d0, d17		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d21
	vmlal.u8 q13, d2, d23
	vmlal.u8 q13, d3, d25
	vrshrn.u16 d29, q13, #7

	add r7, r1, r10
	vst1.32 {q14}, [r7]

	add r10, #16
	add r9, #16
	cmp r10, r12
	blt xPredIntraAngAdi_X_5_gtzero_W16_LoopX

	cmp r12, r4
	beq xPredIntraAngAdi_X_5_gtzero_W16_end


	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 q0, r11

xPredIntraAngAdi_X_5_gtzero_W16_LoopX2:
	vst1.32 {q0}, [r7]!
	
	add r12, #16
	cmp r12, r4
	blt xPredIntraAngAdi_X_5_gtzero_W16_LoopX2

xPredIntraAngAdi_X_5_gtzero_W16_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_5_Y_W16
	b xPredIntraAngAdi_X_5_Y_end

xPredIntraAngAdi_X_5_lezero_W16:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2] * c1 + pSrc[iWidth2 + 1] * c2 + pSrc[iWidth2 + 2] * c3 + pSrc[iWidth2 + 3] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vmov d1, d0
	
	mov r7, r1
	mov r10, #0	
xPredIntraAngAdi_X_5_lezero_W16_loop:

	vst1.32 {q0}, [r7]!

	add r10, #16
	cmp r10, r4
	blt xPredIntraAngAdi_X_5_lezero_W16_loop

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_5_Y_W16

xPredIntraAngAdi_X_5_Y_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_X_7_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	lsl r6, r4, #1				@ iWidth2 = iWidth << 1
	mov r8, #0

	and r12, r4, #15
	cmp r12, #0
	beq xPredIntraAngAdi_X_7_Y_W16

	and r12, r4, #7
	cmp r12, #0
	beq xPredIntraAngAdi_X_7_Y_W8


xPredIntraAngAdi_X_7_Y_W4:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #7				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #7
	lsr r12, r8, #7
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_7_lezero_W4

xPredIntraAngAdi_X_7_gtzero_W4:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 d4, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d4, d4, #1
	vext.8 d6, d4, d4, #2
	vext.8 d7, d4, d4, #3

	vmull.u8 q8, d0, d4			@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0[0], [r1]

	cmp r12, r4
	beq xPredIntraAngAdi_X_7_gtzero_W4_end

	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0[0], [r7]

xPredIntraAngAdi_X_7_gtzero_W4_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_7_Y_W4
	b xPredIntraAngAdi_X_7_Y_end

xPredIntraAngAdi_X_7_lezero_W4:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0[0], [r1]

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_7_Y_W4
	b xPredIntraAngAdi_X_7_Y_end

xPredIntraAngAdi_X_7_Y_W8:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #7				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #7
	lsr r12, r8, #7
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_7_lezero_W8

xPredIntraAngAdi_X_7_gtzero_W8:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 {q10}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d20, d21, #1
	vext.8 d6, d20, d21, #2
	vext.8 d7, d20, d21, #3

	vmull.u8 q8, d0, d20		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0, [r1]

	cmp r12, r4
	beq xPredIntraAngAdi_X_7_gtzero_W8_end

	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0, [r7]

xPredIntraAngAdi_X_7_gtzero_W8_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_7_Y_W8
	b xPredIntraAngAdi_X_7_Y_end

xPredIntraAngAdi_X_7_lezero_W8:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0, [r1]

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_7_Y_W8
	b xPredIntraAngAdi_X_7_Y_end

	
xPredIntraAngAdi_X_7_Y_W16:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #7				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #7
	lsr r12, r8, #7
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_7_lezero_W16

xPredIntraAngAdi_X_7_gtzero_W16:

	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	mov r10, #0					@ x = 0

xPredIntraAngAdi_X_7_gtzero_W16_LoopX:
	add r7, r0, r9
	vld1.32 {q8, q9}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 q10, q8, q9, #1
	vext.8 q11, q8, q9, #2
	vext.8 q12, q8, q9, #3

	vmull.u8 q13, d0, d16		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d20
	vmlal.u8 q13, d2, d22
	vmlal.u8 q13, d3, d24
	vrshrn.u16 d28, q13, #7

	vmull.u8 q13, d0, d17		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d21
	vmlal.u8 q13, d2, d23
	vmlal.u8 q13, d3, d25
	vrshrn.u16 d29, q13, #7

	add r7, r1, r10
	vst1.32 {q14}, [r7]

	add r10, #16
	add r9, #16
	cmp r10, r12
	blt xPredIntraAngAdi_X_7_gtzero_W16_LoopX

	cmp r12, r4
	beq xPredIntraAngAdi_X_7_gtzero_W16_end


	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 q0, r11

xPredIntraAngAdi_X_7_gtzero_W16_LoopX2:
	vst1.32 {q0}, [r7]!
	
	add r12, #16
	cmp r12, r4
	blt xPredIntraAngAdi_X_7_gtzero_W16_LoopX2

xPredIntraAngAdi_X_7_gtzero_W16_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_7_Y_W16
	b xPredIntraAngAdi_X_7_Y_end

xPredIntraAngAdi_X_7_lezero_W16:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vmov d1, d0
	
	mov r7, r1
	mov r10, #0	
xPredIntraAngAdi_X_7_lezero_W16_loop:

	vst1.32 {q0}, [r7]!

	add r10, #16
	cmp r10, r4
	blt xPredIntraAngAdi_X_7_lezero_W16_loop

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_7_Y_W16

xPredIntraAngAdi_X_7_Y_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function xPredIntraAngAdi_X_9_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	lsl r6, r4, #1				@ iWidth2 = iWidth << 1
	mov r8, #0

	and r12, r4, #15
	cmp r12, #0
	beq xPredIntraAngAdi_X_9_Y_W16

	and r12, r4, #7
	cmp r12, #0
	beq xPredIntraAngAdi_X_9_Y_W8


xPredIntraAngAdi_X_9_Y_W4:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #8				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #8
	lsr r12, r8, #8
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_9_lezero_W4

xPredIntraAngAdi_X_9_gtzero_W4:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 d4, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d4, d4, #1
	vext.8 d6, d4, d4, #2
	vext.8 d7, d4, d4, #3

	vmull.u8 q8, d0, d4			@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0[0], [r1]

	cmp r12, r4
	beq xPredIntraAngAdi_X_9_gtzero_W4_end

	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0[0], [r7]

xPredIntraAngAdi_X_9_gtzero_W4_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_9_Y_W4
	b xPredIntraAngAdi_X_9_Y_end

xPredIntraAngAdi_X_9_lezero_W4:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0[0], [r1]

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_9_Y_W4
	b xPredIntraAngAdi_X_9_Y_end

xPredIntraAngAdi_X_9_Y_W8:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #8				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #8
	lsr r12, r8, #8
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_9_lezero_W8

xPredIntraAngAdi_X_9_gtzero_W8:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 {q10}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d20, d21, #1
	vext.8 d6, d20, d21, #2
	vext.8 d7, d20, d21, #3

	vmull.u8 q8, d0, d20		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0, [r1]

	cmp r12, r4
	beq xPredIntraAngAdi_X_9_gtzero_W8_end

	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0, [r7]

xPredIntraAngAdi_X_9_gtzero_W8_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_9_Y_W8
	b xPredIntraAngAdi_X_9_Y_end

xPredIntraAngAdi_X_9_lezero_W8:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0, [r1]

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_9_Y_W8
	b xPredIntraAngAdi_X_9_Y_end

	
xPredIntraAngAdi_X_9_Y_W16:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #8				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #8
	lsr r12, r8, #8
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_9_lezero_W16

xPredIntraAngAdi_X_9_gtzero_W16:

	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	mov r10, #0					@ x = 0

xPredIntraAngAdi_X_9_gtzero_W16_LoopX:
	add r7, r0, r9
	vld1.32 {q8, q9}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 q10, q8, q9, #1
	vext.8 q11, q8, q9, #2
	vext.8 q12, q8, q9, #3

	vmull.u8 q13, d0, d16		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d20
	vmlal.u8 q13, d2, d22
	vmlal.u8 q13, d3, d24
	vrshrn.u16 d28, q13, #7

	vmull.u8 q13, d0, d17		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d21
	vmlal.u8 q13, d2, d23
	vmlal.u8 q13, d3, d25
	vrshrn.u16 d29, q13, #7

	add r7, r1, r10
	vst1.32 {q14}, [r7]

	add r10, #16
	add r9, #16
	cmp r10, r12
	blt xPredIntraAngAdi_X_9_gtzero_W16_LoopX

	cmp r12, r4
	beq xPredIntraAngAdi_X_9_gtzero_W16_end


	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 q0, r11

xPredIntraAngAdi_X_9_gtzero_W16_LoopX2:
	vst1.32 {q0}, [r7]!
	
	add r12, #16
	cmp r12, r4
	blt xPredIntraAngAdi_X_9_gtzero_W16_LoopX2

xPredIntraAngAdi_X_9_gtzero_W16_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_9_Y_W16
	b xPredIntraAngAdi_X_9_Y_end

xPredIntraAngAdi_X_9_lezero_W16:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vmov d1, d0
	
	mov r7, r1
	mov r10, #0	
xPredIntraAngAdi_X_9_lezero_W16_loop:

	vst1.32 {q0}, [r7]!

	add r10, #16
	cmp r10, r4
	blt xPredIntraAngAdi_X_9_lezero_W16_loop

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_9_Y_W16

xPredIntraAngAdi_X_9_Y_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_X_11_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	lsl r6, r4, #1				@ iWidth2 = iWidth << 1
	mov r8, #0

	and r12, r4, #15
	cmp r12, #0
	beq xPredIntraAngAdi_X_11_Y_W16

	and r12, r4, #7
	cmp r12, #0
	beq xPredIntraAngAdi_X_11_Y_W8


xPredIntraAngAdi_X_11_Y_W4:

	add r8, r8, #1				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_11_lezero_W4

xPredIntraAngAdi_X_11_gtzero_W4:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 d4, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d4, d4, #1
	vext.8 d6, d4, d4, #2
	vext.8 d7, d4, d4, #3

	vmull.u8 q8, d0, d4			@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0[0], [r1]

	cmp r12, r4
	beq xPredIntraAngAdi_X_11_gtzero_W4_end

	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0[0], [r7]

xPredIntraAngAdi_X_11_gtzero_W4_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_11_Y_W4
	b xPredIntraAngAdi_X_11_Y_end

xPredIntraAngAdi_X_11_lezero_W4:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0[0], [r1]

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_11_Y_W4
	b xPredIntraAngAdi_X_11_Y_end

xPredIntraAngAdi_X_11_Y_W8:

	add r8, r8, #1				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_11_lezero_W8

xPredIntraAngAdi_X_11_gtzero_W8:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 {q10}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d20, d21, #1
	vext.8 d6, d20, d21, #2
	vext.8 d7, d20, d21, #3

	vmull.u8 q8, d0, d20		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0, [r1]

	cmp r12, r4
	beq xPredIntraAngAdi_X_11_gtzero_W8_end

	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0, [r7]

xPredIntraAngAdi_X_11_gtzero_W8_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_11_Y_W8
	b xPredIntraAngAdi_X_11_Y_end

xPredIntraAngAdi_X_11_lezero_W8:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0, [r1]

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_11_Y_W8
	b xPredIntraAngAdi_X_11_Y_end

	
xPredIntraAngAdi_X_11_Y_W16:

	add r8, r8, #1				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r4, r12
	movlt r12, r4

	cmp r12, #0
	ble xPredIntraAngAdi_X_11_lezero_W16

xPredIntraAngAdi_X_11_gtzero_W16:

	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	mov r10, #0					@ x = 0

xPredIntraAngAdi_X_11_gtzero_W16_LoopX:
	add r7, r0, r9
	vld1.32 {q8, q9}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 q10, q8, q9, #1
	vext.8 q11, q8, q9, #2
	vext.8 q12, q8, q9, #3

	vmull.u8 q13, d0, d16		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d20
	vmlal.u8 q13, d2, d22
	vmlal.u8 q13, d3, d24
	vrshrn.u16 d28, q13, #7

	vmull.u8 q13, d0, d17		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d21
	vmlal.u8 q13, d2, d23
	vmlal.u8 q13, d3, d25
	vrshrn.u16 d29, q13, #7

	add r7, r1, r10
	vst1.32 {q14}, [r7]

	add r10, #16
	add r9, #16
	cmp r10, r12
	blt xPredIntraAngAdi_X_11_gtzero_W16_LoopX

	cmp r12, r4
	beq xPredIntraAngAdi_X_11_gtzero_W16_end


	add r7, r1, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 q0, r11

xPredIntraAngAdi_X_11_gtzero_W16_LoopX2:
	vst1.32 {q0}, [r7]!
	
	add r12, #16
	cmp r12, r4
	blt xPredIntraAngAdi_X_11_gtzero_W16_LoopX2

xPredIntraAngAdi_X_11_gtzero_W16_end:
	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_11_Y_W16
	b xPredIntraAngAdi_X_11_Y_end

xPredIntraAngAdi_X_11_lezero_W16:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vmov d1, d0
	
	mov r7, r1
	mov r10, #0	
xPredIntraAngAdi_X_11_lezero_W16_loop:

	vst1.32 {q0}, [r7]!

	add r10, #16
	cmp r10, r4
	blt xPredIntraAngAdi_X_11_lezero_W16_loop

	add r1, r1, r2
	sub r5, #1
	cmp r5, #0
	bne xPredIntraAngAdi_X_11_Y_W16

xPredIntraAngAdi_X_11_Y_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_Y_25_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	lsl r6, r5, #1				@ iWidth2 = iWidth << 1
	mov r8, #0

	sub r3, sp, #8192
	push {r2, r4}
	add r0, #144

	mov r2, r4
	cmp r4, r5
	movlt r2, r5

	and r12, r5, #15
	cmp r12, #0
	beq xPredIntraAngAdi_Y_25_Y_H16

	and r12, r5, #7
	cmp r12, #0
	beq xPredIntraAngAdi_Y_25_Y_H8


xPredIntraAngAdi_Y_25_Y_H4:

	add r8, r8, #1				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_25_lezero_H4

xPredIntraAngAdi_Y_25_gtzero_H4:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 d4, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d4, d4, #1
	vext.8 d6, d4, d4, #2
	vext.8 d7, d4, d4, #3

	vmull.u8 q8, d0, d4			@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0[0], [r3]

	cmp r12, r5
	beq xPredIntraAngAdi_Y_25_gtzero_H4_end

	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0[0], [r7]

xPredIntraAngAdi_Y_25_gtzero_H4_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_25_Y_H4
	b xPredIntraAngAdi_Y_25_H4_transpose

xPredIntraAngAdi_Y_25_lezero_H4:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0[0], [r3]

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_25_Y_H4

xPredIntraAngAdi_Y_25_H4_transpose:
	pop {r2, r4}
	cmp r4, #4
	bne xPredIntraAngAdi_Y_25_H4W16_transpose

xPredIntraAngAdi_Y_25_H4W4_transpose:

	sub r3, sp, #8192
	vld1.8 {q0}, [r3]
	vuzp.8 d0, d1
	vuzp.8 d0, d1
	vst1.32 d0[0], [r1], r2
	vst1.32 d0[1], [r1], r2
	vst1.32 d1[0], [r1], r2
	vst1.32 d1[1], [r1]
	b xPredIntraAngAdi_Y_25_Y_end

xPredIntraAngAdi_Y_25_H4W16_transpose:

	sub r3, sp, #8192
	mov r9, #16
	vld1.32 d0[0], [r3], r9
	vld1.32 d2[0], [r3], r9
	vld1.32 d4[0], [r3], r9
	vld1.32 d6[0], [r3], r9

	vld1.32 d0[1], [r3], r9
	vld1.32 d2[1], [r3], r9
	vld1.32 d4[1], [r3], r9
	vld1.32 d6[1], [r3], r9

	vld1.32 d1[0], [r3], r9
	vld1.32 d3[0], [r3], r9
	vld1.32 d5[0], [r3], r9
	vld1.32 d7[0], [r3], r9

	vld1.32 d1[1], [r3], r9
	vld1.32 d3[1], [r3], r9
	vld1.32 d5[1], [r3], r9
	vld1.32 d7[1], [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3

	vtrn.16 q0, q2
	vtrn.16 q1, q3

	vst1.8 {q0}, [r1], r2
	vst1.8 {q1}, [r1], r2
	vst1.8 {q2}, [r1], r2
	vst1.8 {q3}, [r1], r2
	b xPredIntraAngAdi_Y_25_Y_end

xPredIntraAngAdi_Y_25_Y_H8:

	add r8, r8, #1				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_25_lezero_H8

xPredIntraAngAdi_Y_25_gtzero_H8:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 {q10}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d20, d21, #1
	vext.8 d6, d20, d21, #2
	vext.8 d7, d20, d21, #3

	vmull.u8 q8, d0, d20		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0, [r3]

	cmp r12, r5
	beq xPredIntraAngAdi_Y_25_gtzero_H8_end

	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0, [r7]

xPredIntraAngAdi_Y_25_gtzero_H8_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_25_Y_H8
	b xPredIntraAngAdi_Y_25_H8_transpose

xPredIntraAngAdi_Y_25_lezero_H8:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0, [r3]

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_25_Y_H8

xPredIntraAngAdi_Y_25_H8_transpose:
	pop {r2, r4}
	cmp r4, #8
	bne xPredIntraAngAdi_Y_25_H8W32_transpose	

xPredIntraAngAdi_Y_25_H8W8_transpose:

	sub r3, sp, #8192
	vld1.32 {q0, q1}, [r3]!	
	vld1.32 {q2, q3}, [r3]
	
	vtrn.8 d0, d1
	vtrn.8 d2, d3
	vtrn.8 d4, d5
	vtrn.8 d6, d7

	vtrn.16 d0, d2
	vtrn.16 d1, d3
	vtrn.16 d4, d6
	vtrn.16 d5, d7

	vtrn.32 d0, d4
	vtrn.32 d1, d5
	vtrn.32 d2, d6
	vtrn.32 d3, d7

	vst1.32 {d0}, [r1], r2
	vst1.32 {d1}, [r1], r2
	vst1.32 {d2}, [r1], r2
	vst1.32 {d3}, [r1], r2
	vst1.32 {d4}, [r1], r2
	vst1.32 {d5}, [r1], r2
	vst1.32 {d6}, [r1], r2
	vst1.32 {d7}, [r1]
	
	b xPredIntraAngAdi_Y_25_Y_end

xPredIntraAngAdi_Y_25_H8W32_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	mov r9, #32
	vld1.32 d0, [r3], r9
	vld1.32 d4, [r3], r9
	vld1.32 d8, [r3], r9
	vld1.32 d12, [r3], r9
	vld1.32 d16, [r3], r9
	vld1.32 d20, [r3], r9
	vld1.32 d24, [r3], r9
	vld1.32 d28, [r3], r9

	vld1.32 d1, [r3], r9
	vld1.32 d5, [r3], r9
	vld1.32 d9, [r3], r9
	vld1.32 d13, [r3], r9
	vld1.32 d17, [r3], r9
	vld1.32 d21, [r3], r9
	vld1.32 d25, [r3], r9
	vld1.32 d29, [r3], r9

	vld1.32 d2, [r3], r9
	vld1.32 d6, [r3], r9
	vld1.32 d10, [r3], r9
	vld1.32 d14, [r3], r9
	vld1.32 d18, [r3], r9
	vld1.32 d22, [r3], r9
	vld1.32 d26, [r3], r9
	vld1.32 d30, [r3], r9

	vld1.32 d3, [r3], r9
	vld1.32 d7, [r3], r9
	vld1.32 d11, [r3], r9
	vld1.32 d15, [r3], r9
	vld1.32 d19, [r3], r9
	vld1.32 d23, [r3], r9
	vld1.32 d27, [r3], r9
	vld1.32 d31, [r3], r9

	vtrn.8 q0, q2
	vtrn.8 q4, q6
	vtrn.8 q8, q10
	vtrn.8 q12, q14
	vtrn.8 q1, q3
	vtrn.8 q5, q7
	vtrn.8 q9, q11
	vtrn.8 q13, q15

	vtrn.16 q0, q4
	vtrn.16 q2, q6
	vtrn.16 q8, q12
	vtrn.16 q10, q14
	vtrn.16 q1, q5
	vtrn.16 q3, q7
	vtrn.16 q9, q13
	vtrn.16 q11, q15

	vtrn.32 q0, q8
	vtrn.32 q2, q10
	vtrn.32 q4, q12
	vtrn.32 q6, q14
	vtrn.32 q1, q9
	vtrn.32 q3, q11
	vtrn.32 q5, q13
	vtrn.32 q7, q15

	vst1.8 {q0, q1}, [r1], r2
	vst1.8 {q2, q3}, [r1], r2
	vst1.8 {q4, q5}, [r1], r2
	vst1.8 {q6, q7}, [r1], r2
	vst1.8 {q8, q9}, [r1], r2
	vst1.8 {q10, q11}, [r1], r2
	vst1.8 {q12, q13}, [r1], r2
	vst1.8 {q14, q15}, [r1], r2

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_25_Y_end

xPredIntraAngAdi_Y_25_Y_H16:

	add r8, r8, #1				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_25_lezero_H16

xPredIntraAngAdi_Y_25_gtzero_H16:

	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	mov r10, #0					@ x = 0

xPredIntraAngAdi_Y_25_gtzero_H16_LoopX:
	add r7, r0, r9
	vld1.32 {q8, q9}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 q10, q8, q9, #1
	vext.8 q11, q8, q9, #2
	vext.8 q12, q8, q9, #3

	vmull.u8 q13, d0, d16		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d20
	vmlal.u8 q13, d2, d22
	vmlal.u8 q13, d3, d24
	vrshrn.u16 d28, q13, #7

	vmull.u8 q13, d0, d17		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d21
	vmlal.u8 q13, d2, d23
	vmlal.u8 q13, d3, d25
	vrshrn.u16 d29, q13, #7

	add r7, r3, r10
	vst1.32 {q14}, [r7]

	add r10, #16
	add r9, #16
	cmp r10, r12
	blt xPredIntraAngAdi_Y_25_gtzero_H16_LoopX

	cmp r12, r5
	beq xPredIntraAngAdi_Y_25_gtzero_H16_end


	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 q0, r11

xPredIntraAngAdi_Y_25_gtzero_H16_LoopX2:
	vst1.32 {q0}, [r7]!
	
	add r12, #16
	cmp r12, r5
	blt xPredIntraAngAdi_Y_25_gtzero_H16_LoopX2

xPredIntraAngAdi_Y_25_gtzero_H16_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_25_Y_H16
	b xPredIntraAngAdi_Y_25_H16n_transpose

xPredIntraAngAdi_Y_25_lezero_H16:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vmov d1, d0
	
	mov r7, r3
	mov r10, #0	
xPredIntraAngAdi_Y_25_lezero_H16_loop:

	vst1.32 {q0}, [r7]!

	add r10, #16
	cmp r10, r5
	blt xPredIntraAngAdi_Y_25_lezero_H16_loop

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_25_Y_H16

xPredIntraAngAdi_Y_25_H16n_transpose:
	pop {r2, r4}

	cmp r5, #16
	beq xPredIntraAngAdi_Y_25_H16_transpose

	cmp r5, #32
	beq xPredIntraAngAdi_Y_25_H32_transpose

xPredIntraAngAdi_Y_25_H64W64_transpose:

	sub r3, sp, #8192
	mov r7, r1
	mov r8, r3
	vpush {q4-q7}
	mov r9, #64
	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #16
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #32
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #48
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_25_Y_end

xPredIntraAngAdi_Y_25_H32_transpose:

	cmp r4, #32
	bne xPredIntraAngAdi_Y_25_H32W8_transpose

xPredIntraAngAdi_Y_25_H32W32_transpose:

	sub r3, sp, #8192
	mov r7, r1
	mov r8, r3
	vpush {q4-q7}

	mov r9, #32
	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #16
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_25_Y_end

xPredIntraAngAdi_Y_25_H32W8_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	vld1.8 {q0, q1}, [r3]!
	vld1.8 {q2, q3}, [r3]!
	vld1.8 {q4, q5}, [r3]!
	vld1.8 {q6, q7}, [r3]!
	vld1.8 {q8, q9}, [r3]!
	vld1.8 {q10, q11}, [r3]!
	vld1.8 {q12, q13}, [r3]!
	vld1.8 {q14, q15}, [r3]!

	vtrn.8 q0, q2
	vtrn.8 q4, q6
	vtrn.8 q8, q10
	vtrn.8 q12, q14
	vtrn.8 q1, q3
	vtrn.8 q5, q7
	vtrn.8 q9, q11
	vtrn.8 q13, q15

	vtrn.16 q0, q4
	vtrn.16 q2, q6
	vtrn.16 q8, q12
	vtrn.16 q10, q14
	vtrn.16 q1, q5
	vtrn.16 q3, q7
	vtrn.16 q9, q13
	vtrn.16 q11, q15

	vtrn.32 q0, q8
	vtrn.32 q2, q10
	vtrn.32 q4, q12
	vtrn.32 q6, q14
	vtrn.32 q1, q9
	vtrn.32 q3, q11
	vtrn.32 q5, q13
	vtrn.32 q7, q15

	vst1.8 d0, [r1], r2
	vst1.8 d4, [r1], r2
	vst1.8 d8, [r1], r2
	vst1.8 d12, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d24, [r1], r2
	vst1.8 d28, [r1], r2

	vst1.8 d1, [r1], r2
	vst1.8 d5, [r1], r2
	vst1.8 d9, [r1], r2
	vst1.8 d13, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d21, [r1], r2
	vst1.8 d25, [r1], r2
	vst1.8 d29, [r1], r2

	vst1.8 d2, [r1], r2
	vst1.8 d6, [r1], r2
	vst1.8 d10, [r1], r2
	vst1.8 d14, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d22, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d30, [r1], r2

	vst1.8 d3, [r1], r2
	vst1.8 d7, [r1], r2
	vst1.8 d11, [r1], r2
	vst1.8 d15, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d23, [r1], r2
	vst1.8 d27, [r1], r2
	vst1.8 d31, [r1], r2

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_25_Y_end

xPredIntraAngAdi_Y_25_H16_transpose:

	cmp r4, #16
	bne xPredIntraAngAdi_Y_25_H16W4_transpose

xPredIntraAngAdi_Y_25_H16W16_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	vld1.32 {q0}, [r3]!
	vld1.32 {q1}, [r3]!
	vld1.32 {q2}, [r3]!
	vld1.32 {q3}, [r3]!
	vld1.32 {q4}, [r3]!
	vld1.32 {q5}, [r3]!
	vld1.32 {q6}, [r3]!
	vld1.32 {q7}, [r3]!
	vld1.32 {q8}, [r3]!
	vld1.32 {q9}, [r3]!
	vld1.32 {q10}, [r3]!
	vld1.32 {q11}, [r3]!
	vld1.32 {q12}, [r3]!
	vld1.32 {q13}, [r3]!
	vld1.32 {q14}, [r3]!
	vld1.32 {q15}, [r3]

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_25_Y_end

xPredIntraAngAdi_Y_25_H16W4_transpose:

	sub r3, sp, #8192

	vld1.8 {q0, q1}, [r3]!
	vld1.8 {q2, q3}, [r3]

	vtrn.8 q0, q1
	vtrn.8 q2, q3

	vtrn.16 q0, q2
	vtrn.16 q1, q3

	vst1.32 d0[0], [r1], r2
	vst1.32 d2[0], [r1], r2
	vst1.32 d4[0], [r1], r2
	vst1.32 d6[0], [r1], r2

	vst1.32 d0[1], [r1], r2
	vst1.32 d2[1], [r1], r2
	vst1.32 d4[1], [r1], r2
	vst1.32 d6[1], [r1], r2

	vst1.32 d1[0], [r1], r2
	vst1.32 d3[0], [r1], r2
	vst1.32 d5[0], [r1], r2
	vst1.32 d7[0], [r1], r2

	vst1.32 d1[1], [r1], r2
	vst1.32 d3[1], [r1], r2
	vst1.32 d5[1], [r1], r2
	vst1.32 d7[1], [r1], r2

xPredIntraAngAdi_Y_25_Y_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_Y_27_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	lsl r6, r5, #1				@ iWidth2 = iWidth << 1
	mov r8, #0

	sub r3, sp, #8192
	push {r2, r4}
	add r0, #144

	mov r2, r4
	cmp r4, r5
	movlt r2, r5

	and r12, r5, #15
	cmp r12, #0
	beq xPredIntraAngAdi_Y_27_Y_H16

	and r12, r5, #7
	cmp r12, #0
	beq xPredIntraAngAdi_Y_27_Y_H8


xPredIntraAngAdi_Y_27_Y_H4:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #8				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #8
	lsr r12, r8, #8
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_27_lezero_H4

xPredIntraAngAdi_Y_27_gtzero_H4:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 d4, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d4, d4, #1
	vext.8 d6, d4, d4, #2
	vext.8 d7, d4, d4, #3

	vmull.u8 q8, d0, d4			@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0[0], [r3]

	cmp r12, r5
	beq xPredIntraAngAdi_Y_27_gtzero_H4_end

	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0[0], [r7]

xPredIntraAngAdi_Y_27_gtzero_H4_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_27_Y_H4
	b xPredIntraAngAdi_Y_27_H4_transpose

xPredIntraAngAdi_Y_27_lezero_H4:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0[0], [r3]

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_27_Y_H4

xPredIntraAngAdi_Y_27_H4_transpose:
	pop {r2, r4}
	cmp r4, #4
	bne xPredIntraAngAdi_Y_27_H4W16_transpose

xPredIntraAngAdi_Y_27_H4W4_transpose:

	sub r3, sp, #8192
	vld1.8 {q0}, [r3]
	vuzp.8 d0, d1
	vuzp.8 d0, d1
	vst1.32 d0[0], [r1], r2
	vst1.32 d0[1], [r1], r2
	vst1.32 d1[0], [r1], r2
	vst1.32 d1[1], [r1]
	b xPredIntraAngAdi_Y_27_Y_end

xPredIntraAngAdi_Y_27_H4W16_transpose:

	sub r3, sp, #8192
	mov r9, #16
	vld1.32 d0[0], [r3], r9
	vld1.32 d2[0], [r3], r9
	vld1.32 d4[0], [r3], r9
	vld1.32 d6[0], [r3], r9

	vld1.32 d0[1], [r3], r9
	vld1.32 d2[1], [r3], r9
	vld1.32 d4[1], [r3], r9
	vld1.32 d6[1], [r3], r9

	vld1.32 d1[0], [r3], r9
	vld1.32 d3[0], [r3], r9
	vld1.32 d5[0], [r3], r9
	vld1.32 d7[0], [r3], r9

	vld1.32 d1[1], [r3], r9
	vld1.32 d3[1], [r3], r9
	vld1.32 d5[1], [r3], r9
	vld1.32 d7[1], [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3

	vtrn.16 q0, q2
	vtrn.16 q1, q3

	vst1.8 {q0}, [r1], r2
	vst1.8 {q1}, [r1], r2
	vst1.8 {q2}, [r1], r2
	vst1.8 {q3}, [r1], r2
	b xPredIntraAngAdi_Y_27_Y_end

xPredIntraAngAdi_Y_27_Y_H8:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #8				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #8
	lsr r12, r8, #8
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_27_lezero_H8

xPredIntraAngAdi_Y_27_gtzero_H8:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 {q10}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d20, d21, #1
	vext.8 d6, d20, d21, #2
	vext.8 d7, d20, d21, #3

	vmull.u8 q8, d0, d20		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0, [r3]

	cmp r12, r5
	beq xPredIntraAngAdi_Y_27_gtzero_H8_end

	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0, [r7]

xPredIntraAngAdi_Y_27_gtzero_H8_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_27_Y_H8
	b xPredIntraAngAdi_Y_27_H8_transpose

xPredIntraAngAdi_Y_27_lezero_H8:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0, [r3]

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_27_Y_H8

xPredIntraAngAdi_Y_27_H8_transpose:
	pop {r2, r4}
	cmp r4, #8
	bne xPredIntraAngAdi_Y_27_H8W32_transpose	

xPredIntraAngAdi_Y_27_H8W8_transpose:

	sub r3, sp, #8192
	vld1.32 {q0, q1}, [r3]!	
	vld1.32 {q2, q3}, [r3]
	
	vtrn.8 d0, d1
	vtrn.8 d2, d3
	vtrn.8 d4, d5
	vtrn.8 d6, d7

	vtrn.16 d0, d2
	vtrn.16 d1, d3
	vtrn.16 d4, d6
	vtrn.16 d5, d7

	vtrn.32 d0, d4
	vtrn.32 d1, d5
	vtrn.32 d2, d6
	vtrn.32 d3, d7
	
	vst1.32 d0, [r1], r2
	vst1.32 d1, [r1], r2
	vst1.32 d2, [r1], r2
	vst1.32 d3, [r1], r2
	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2
	vst1.32 d6, [r1], r2
	vst1.32 d7, [r1]
	b xPredIntraAngAdi_Y_27_Y_end

xPredIntraAngAdi_Y_27_H8W32_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	mov r9, #32
	vld1.32 d0, [r3], r9
	vld1.32 d4, [r3], r9
	vld1.32 d8, [r3], r9
	vld1.32 d12, [r3], r9
	vld1.32 d16, [r3], r9
	vld1.32 d20, [r3], r9
	vld1.32 d24, [r3], r9
	vld1.32 d28, [r3], r9

	vld1.32 d1, [r3], r9
	vld1.32 d5, [r3], r9
	vld1.32 d9, [r3], r9
	vld1.32 d13, [r3], r9
	vld1.32 d17, [r3], r9
	vld1.32 d21, [r3], r9
	vld1.32 d25, [r3], r9
	vld1.32 d29, [r3], r9

	vld1.32 d2, [r3], r9
	vld1.32 d6, [r3], r9
	vld1.32 d10, [r3], r9
	vld1.32 d14, [r3], r9
	vld1.32 d18, [r3], r9
	vld1.32 d22, [r3], r9
	vld1.32 d26, [r3], r9
	vld1.32 d30, [r3], r9

	vld1.32 d3, [r3], r9
	vld1.32 d7, [r3], r9
	vld1.32 d11, [r3], r9
	vld1.32 d15, [r3], r9
	vld1.32 d19, [r3], r9
	vld1.32 d23, [r3], r9
	vld1.32 d27, [r3], r9
	vld1.32 d31, [r3], r9

	vtrn.8 q0, q2
	vtrn.8 q4, q6
	vtrn.8 q8, q10
	vtrn.8 q12, q14
	vtrn.8 q1, q3
	vtrn.8 q5, q7
	vtrn.8 q9, q11
	vtrn.8 q13, q15

	vtrn.16 q0, q4
	vtrn.16 q2, q6
	vtrn.16 q8, q12
	vtrn.16 q10, q14
	vtrn.16 q1, q5
	vtrn.16 q3, q7
	vtrn.16 q9, q13
	vtrn.16 q11, q15

	vtrn.32 q0, q8
	vtrn.32 q2, q10
	vtrn.32 q4, q12
	vtrn.32 q6, q14
	vtrn.32 q1, q9
	vtrn.32 q3, q11
	vtrn.32 q5, q13
	vtrn.32 q7, q15

	vst1.8 {q0, q1}, [r1], r2
	vst1.8 {q2, q3}, [r1], r2
	vst1.8 {q4, q5}, [r1], r2
	vst1.8 {q6, q7}, [r1], r2
	vst1.8 {q8, q9}, [r1], r2
	vst1.8 {q10, q11}, [r1], r2
	vst1.8 {q12, q13}, [r1], r2
	vst1.8 {q14, q15}, [r1], r2

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_27_Y_end

xPredIntraAngAdi_Y_27_Y_H16:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #8				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #8
	lsr r12, r8, #8
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_27_lezero_H16

xPredIntraAngAdi_Y_27_gtzero_H16:

	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	mov r10, #0					@ x = 0

xPredIntraAngAdi_Y_27_gtzero_H16_LoopX:
	add r7, r0, r9
	vld1.32 {q8, q9}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 q10, q8, q9, #1
	vext.8 q11, q8, q9, #2
	vext.8 q12, q8, q9, #3

	vmull.u8 q13, d0, d16		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d20
	vmlal.u8 q13, d2, d22
	vmlal.u8 q13, d3, d24
	vrshrn.u16 d28, q13, #7

	vmull.u8 q13, d0, d17		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d21
	vmlal.u8 q13, d2, d23
	vmlal.u8 q13, d3, d25
	vrshrn.u16 d29, q13, #7

	add r7, r3, r10
	vst1.32 {q14}, [r7]

	add r10, #16
	add r9, #16
	cmp r10, r12
	blt xPredIntraAngAdi_Y_27_gtzero_H16_LoopX

	cmp r12, r5
	beq xPredIntraAngAdi_Y_27_gtzero_H16_end


	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 q0, r11

xPredIntraAngAdi_Y_27_gtzero_H16_LoopX2:
	vst1.32 {q0}, [r7]!
	
	add r12, #16
	cmp r12, r5
	blt xPredIntraAngAdi_Y_27_gtzero_H16_LoopX2

xPredIntraAngAdi_Y_27_gtzero_H16_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_27_Y_H16
	b xPredIntraAngAdi_Y_27_H16n_transpose

xPredIntraAngAdi_Y_27_lezero_H16:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vmov d1, d0
	
	mov r7, r3
	mov r10, #0	
xPredIntraAngAdi_Y_27_lezero_H16_loop:

	vst1.32 {q0}, [r7]!

	add r10, #16
	cmp r10, r5
	blt xPredIntraAngAdi_Y_27_lezero_H16_loop

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_27_Y_H16

xPredIntraAngAdi_Y_27_H16n_transpose:
	pop {r2, r4}

	cmp r5, #16
	beq xPredIntraAngAdi_Y_27_H16_transpose

	cmp r5, #32
	beq xPredIntraAngAdi_Y_27_H32_transpose

xPredIntraAngAdi_Y_27_H64W64_transpose:

	sub r3, sp, #8192
	mov r7, r1
	mov r8, r3
	vpush {q4-q7}
	mov r9, #64
	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #16
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #32
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #48
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_27_Y_end

xPredIntraAngAdi_Y_27_H32_transpose:

	cmp r4, #32
	bne xPredIntraAngAdi_Y_27_H32W8_transpose

xPredIntraAngAdi_Y_27_H32W32_transpose:

	sub r3, sp, #8192
	mov r7, r1
	mov r8, r3
	vpush {q4-q7}

	mov r9, #32
	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #16
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_27_Y_end

xPredIntraAngAdi_Y_27_H32W8_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	vld1.8 {q0, q1}, [r3]!
	vld1.8 {q2, q3}, [r3]!
	vld1.8 {q4, q5}, [r3]!
	vld1.8 {q6, q7}, [r3]!
	vld1.8 {q8, q9}, [r3]!
	vld1.8 {q10, q11}, [r3]!
	vld1.8 {q12, q13}, [r3]!
	vld1.8 {q14, q15}, [r3]!

	vtrn.8 q0, q2
	vtrn.8 q4, q6
	vtrn.8 q8, q10
	vtrn.8 q12, q14
	vtrn.8 q1, q3
	vtrn.8 q5, q7
	vtrn.8 q9, q11
	vtrn.8 q13, q15

	vtrn.16 q0, q4
	vtrn.16 q2, q6
	vtrn.16 q8, q12
	vtrn.16 q10, q14
	vtrn.16 q1, q5
	vtrn.16 q3, q7
	vtrn.16 q9, q13
	vtrn.16 q11, q15

	vtrn.32 q0, q8
	vtrn.32 q2, q10
	vtrn.32 q4, q12
	vtrn.32 q6, q14
	vtrn.32 q1, q9
	vtrn.32 q3, q11
	vtrn.32 q5, q13
	vtrn.32 q7, q15

	vst1.8 d0, [r1], r2
	vst1.8 d4, [r1], r2
	vst1.8 d8, [r1], r2
	vst1.8 d12, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d24, [r1], r2
	vst1.8 d28, [r1], r2

	vst1.8 d1, [r1], r2
	vst1.8 d5, [r1], r2
	vst1.8 d9, [r1], r2
	vst1.8 d13, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d21, [r1], r2
	vst1.8 d25, [r1], r2
	vst1.8 d29, [r1], r2

	vst1.8 d2, [r1], r2
	vst1.8 d6, [r1], r2
	vst1.8 d10, [r1], r2
	vst1.8 d14, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d22, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d30, [r1], r2

	vst1.8 d3, [r1], r2
	vst1.8 d7, [r1], r2
	vst1.8 d11, [r1], r2
	vst1.8 d15, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d23, [r1], r2
	vst1.8 d27, [r1], r2
	vst1.8 d31, [r1], r2

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_27_Y_end

xPredIntraAngAdi_Y_27_H16_transpose:

	cmp r4, #16
	bne xPredIntraAngAdi_Y_27_H16W4_transpose

xPredIntraAngAdi_Y_27_H16W16_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	vld1.32 {q0}, [r3]!
	vld1.32 {q1}, [r3]!
	vld1.32 {q2}, [r3]!
	vld1.32 {q3}, [r3]!
	vld1.32 {q4}, [r3]!
	vld1.32 {q5}, [r3]!
	vld1.32 {q6}, [r3]!
	vld1.32 {q7}, [r3]!
	vld1.32 {q8}, [r3]!
	vld1.32 {q9}, [r3]!
	vld1.32 {q10}, [r3]!
	vld1.32 {q11}, [r3]!
	vld1.32 {q12}, [r3]!
	vld1.32 {q13}, [r3]!
	vld1.32 {q14}, [r3]!
	vld1.32 {q15}, [r3]

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_27_Y_end

xPredIntraAngAdi_Y_27_H16W4_transpose:

	sub r3, sp, #8192

	vld1.8 {q0, q1}, [r3]!
	vld1.8 {q2, q3}, [r3]

	vtrn.8 q0, q1
	vtrn.8 q2, q3

	vtrn.16 q0, q2
	vtrn.16 q1, q3

	vst1.32 d0[0], [r1], r2
	vst1.32 d2[0], [r1], r2
	vst1.32 d4[0], [r1], r2
	vst1.32 d6[0], [r1], r2

	vst1.32 d0[1], [r1], r2
	vst1.32 d2[1], [r1], r2
	vst1.32 d4[1], [r1], r2
	vst1.32 d6[1], [r1], r2

	vst1.32 d1[0], [r1], r2
	vst1.32 d3[0], [r1], r2
	vst1.32 d5[0], [r1], r2
	vst1.32 d7[0], [r1], r2

	vst1.32 d1[1], [r1], r2
	vst1.32 d3[1], [r1], r2
	vst1.32 d5[1], [r1], r2
	vst1.32 d7[1], [r1], r2

xPredIntraAngAdi_Y_27_Y_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function xPredIntraAngAdi_Y_29_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	lsl r6, r5, #1				@ iWidth2 = iWidth << 1
	mov r8, #0

	sub r3, sp, #8192
	push {r2, r4}
	add r0, #144

	mov r2, r4
	cmp r4, r5
	movlt r2, r5

	and r12, r5, #15
	cmp r12, #0
	beq xPredIntraAngAdi_Y_29_Y_H16

	and r12, r5, #7
	cmp r12, #0
	beq xPredIntraAngAdi_Y_29_Y_H8


xPredIntraAngAdi_Y_29_Y_H4:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #7				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #7
	lsr r12, r8, #7
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_29_lezero_H4

xPredIntraAngAdi_Y_29_gtzero_H4:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 d4, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d4, d4, #1
	vext.8 d6, d4, d4, #2
	vext.8 d7, d4, d4, #3

	vmull.u8 q8, d0, d4			@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0[0], [r3]

	cmp r12, r5
	beq xPredIntraAngAdi_Y_29_gtzero_H4_end

	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0[0], [r7]

xPredIntraAngAdi_Y_29_gtzero_H4_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_29_Y_H4
	b xPredIntraAngAdi_Y_29_H4_transpose

xPredIntraAngAdi_Y_29_lezero_H4:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0[0], [r3]

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_29_Y_H4

xPredIntraAngAdi_Y_29_H4_transpose:
	pop {r2, r4}
	cmp r4, #4
	bne xPredIntraAngAdi_Y_29_H4W16_transpose

xPredIntraAngAdi_Y_29_H4W4_transpose:

	sub r3, sp, #8192
	vld1.8 {q0}, [r3]
	vuzp.8 d0, d1
	vuzp.8 d0, d1
	vst1.32 d0[0], [r1], r2
	vst1.32 d0[1], [r1], r2
	vst1.32 d1[0], [r1], r2
	vst1.32 d1[1], [r1]
	b xPredIntraAngAdi_Y_29_Y_end

xPredIntraAngAdi_Y_29_H4W16_transpose:

	sub r3, sp, #8192
	mov r9, #16
	vld1.32 d0[0], [r3], r9
	vld1.32 d2[0], [r3], r9
	vld1.32 d4[0], [r3], r9
	vld1.32 d6[0], [r3], r9

	vld1.32 d0[1], [r3], r9
	vld1.32 d2[1], [r3], r9
	vld1.32 d4[1], [r3], r9
	vld1.32 d6[1], [r3], r9

	vld1.32 d1[0], [r3], r9
	vld1.32 d3[0], [r3], r9
	vld1.32 d5[0], [r3], r9
	vld1.32 d7[0], [r3], r9

	vld1.32 d1[1], [r3], r9
	vld1.32 d3[1], [r3], r9
	vld1.32 d5[1], [r3], r9
	vld1.32 d7[1], [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3

	vtrn.16 q0, q2
	vtrn.16 q1, q3

	vst1.8 {q0}, [r1], r2
	vst1.8 {q1}, [r1], r2
	vst1.8 {q2}, [r1], r2
	vst1.8 {q3}, [r1], r2
	b xPredIntraAngAdi_Y_29_Y_end

xPredIntraAngAdi_Y_29_Y_H8:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #7				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #7
	lsr r12, r8, #7
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_29_lezero_H8

xPredIntraAngAdi_Y_29_gtzero_H8:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 {q10}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d20, d21, #1
	vext.8 d6, d20, d21, #2
	vext.8 d7, d20, d21, #3

	vmull.u8 q8, d0, d20		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0, [r3]

	cmp r12, r5
	beq xPredIntraAngAdi_Y_29_gtzero_H8_end

	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0, [r7]

xPredIntraAngAdi_Y_29_gtzero_H8_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_29_Y_H8
	b xPredIntraAngAdi_Y_29_H8_transpose

xPredIntraAngAdi_Y_29_lezero_H8:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0, [r3]

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_29_Y_H8

xPredIntraAngAdi_Y_29_H8_transpose:
	pop {r2, r4}
	cmp r4, #8
	bne xPredIntraAngAdi_Y_29_H8W32_transpose	

xPredIntraAngAdi_Y_29_H8W8_transpose:

	sub r3, sp, #8192
	vld1.32 {q0, q1}, [r3]!	
	vld1.32 {q2, q3}, [r3]
	
	vtrn.8 d0, d1
	vtrn.8 d2, d3
	vtrn.8 d4, d5
	vtrn.8 d6, d7

	vtrn.16 d0, d2
	vtrn.16 d1, d3
	vtrn.16 d4, d6
	vtrn.16 d5, d7

	vtrn.32 d0, d4
	vtrn.32 d1, d5
	vtrn.32 d2, d6
	vtrn.32 d3, d7
	
	vst1.32 d0, [r1], r2
	vst1.32 d1, [r1], r2
	vst1.32 d2, [r1], r2
	vst1.32 d3, [r1], r2
	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2
	vst1.32 d6, [r1], r2
	vst1.32 d7, [r1], r2
	b xPredIntraAngAdi_Y_29_Y_end

xPredIntraAngAdi_Y_29_H8W32_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	mov r9, #32
	vld1.32 d0, [r3], r9
	vld1.32 d4, [r3], r9
	vld1.32 d8, [r3], r9
	vld1.32 d12, [r3], r9
	vld1.32 d16, [r3], r9
	vld1.32 d20, [r3], r9
	vld1.32 d24, [r3], r9
	vld1.32 d28, [r3], r9

	vld1.32 d1, [r3], r9
	vld1.32 d5, [r3], r9
	vld1.32 d9, [r3], r9
	vld1.32 d13, [r3], r9
	vld1.32 d17, [r3], r9
	vld1.32 d21, [r3], r9
	vld1.32 d25, [r3], r9
	vld1.32 d29, [r3], r9

	vld1.32 d2, [r3], r9
	vld1.32 d6, [r3], r9
	vld1.32 d10, [r3], r9
	vld1.32 d14, [r3], r9
	vld1.32 d18, [r3], r9
	vld1.32 d22, [r3], r9
	vld1.32 d26, [r3], r9
	vld1.32 d30, [r3], r9

	vld1.32 d3, [r3], r9
	vld1.32 d7, [r3], r9
	vld1.32 d11, [r3], r9
	vld1.32 d15, [r3], r9
	vld1.32 d19, [r3], r9
	vld1.32 d23, [r3], r9
	vld1.32 d27, [r3], r9
	vld1.32 d31, [r3], r9

	vtrn.8 q0, q2
	vtrn.8 q4, q6
	vtrn.8 q8, q10
	vtrn.8 q12, q14
	vtrn.8 q1, q3
	vtrn.8 q5, q7
	vtrn.8 q9, q11
	vtrn.8 q13, q15

	vtrn.16 q0, q4
	vtrn.16 q2, q6
	vtrn.16 q8, q12
	vtrn.16 q10, q14
	vtrn.16 q1, q5
	vtrn.16 q3, q7
	vtrn.16 q9, q13
	vtrn.16 q11, q15

	vtrn.32 q0, q8
	vtrn.32 q2, q10
	vtrn.32 q4, q12
	vtrn.32 q6, q14
	vtrn.32 q1, q9
	vtrn.32 q3, q11
	vtrn.32 q5, q13
	vtrn.32 q7, q15

	vst1.8 {q0, q1}, [r1], r2
	vst1.8 {q2, q3}, [r1], r2
	vst1.8 {q4, q5}, [r1], r2
	vst1.8 {q6, q7}, [r1], r2
	vst1.8 {q8, q9}, [r1], r2
	vst1.8 {q10, q11}, [r1], r2
	vst1.8 {q12, q13}, [r1], r2
	vst1.8 {q14, q15}, [r1], r2

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_29_Y_end

xPredIntraAngAdi_Y_29_Y_H16:

	add r8, r8, #93				@ iTempDn = iTempD * imult
	lsr r9, r8, #7				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #7
	lsr r12, r8, #7
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_29_lezero_H16

xPredIntraAngAdi_Y_29_gtzero_H16:

	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	mov r10, #0					@ x = 0

xPredIntraAngAdi_Y_29_gtzero_H16_LoopX:
	add r7, r0, r9
	vld1.32 {q8, q9}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 q10, q8, q9, #1
	vext.8 q11, q8, q9, #2
	vext.8 q12, q8, q9, #3

	vmull.u8 q13, d0, d16		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d20
	vmlal.u8 q13, d2, d22
	vmlal.u8 q13, d3, d24
	vrshrn.u16 d28, q13, #7

	vmull.u8 q13, d0, d17		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d21
	vmlal.u8 q13, d2, d23
	vmlal.u8 q13, d3, d25
	vrshrn.u16 d29, q13, #7

	add r7, r3, r10
	vst1.32 {q14}, [r7]

	add r10, #16
	add r9, #16
	cmp r10, r12
	blt xPredIntraAngAdi_Y_29_gtzero_H16_LoopX

	cmp r12, r5
	beq xPredIntraAngAdi_Y_29_gtzero_H16_end


	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 q0, r11

xPredIntraAngAdi_Y_29_gtzero_H16_LoopX2:
	vst1.32 {q0}, [r7]!
	
	add r12, #16
	cmp r12, r5
	blt xPredIntraAngAdi_Y_29_gtzero_H16_LoopX2

xPredIntraAngAdi_Y_29_gtzero_H16_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_29_Y_H16
	b xPredIntraAngAdi_Y_29_H16n_transpose

xPredIntraAngAdi_Y_29_lezero_H16:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vmov d1, d0
	
	mov r7, r3
	mov r10, #0	
xPredIntraAngAdi_Y_29_lezero_H16_loop:

	vst1.32 {q0}, [r7]!

	add r10, #16
	cmp r10, r5
	blt xPredIntraAngAdi_Y_29_lezero_H16_loop

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_29_Y_H16

xPredIntraAngAdi_Y_29_H16n_transpose:
	pop {r2, r4}

	cmp r5, #16
	beq xPredIntraAngAdi_Y_29_H16_transpose

	cmp r5, #32
	beq xPredIntraAngAdi_Y_29_H32_transpose

xPredIntraAngAdi_Y_29_H64W64_transpose:

	sub r3, sp, #8192
	mov r7, r1
	mov r8, r3
	vpush {q4-q7}
	mov r9, #64
	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #16
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #32
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #48
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_29_Y_end

xPredIntraAngAdi_Y_29_H32_transpose:

	cmp r4, #32
	bne xPredIntraAngAdi_Y_29_H32W8_transpose

xPredIntraAngAdi_Y_29_H32W32_transpose:

	sub r3, sp, #8192
	mov r7, r1
	mov r8, r3
	vpush {q4-q7}

	mov r9, #32
	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #16
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_29_Y_end

xPredIntraAngAdi_Y_29_H32W8_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	vld1.8 {q0, q1}, [r3]!
	vld1.8 {q2, q3}, [r3]!
	vld1.8 {q4, q5}, [r3]!
	vld1.8 {q6, q7}, [r3]!
	vld1.8 {q8, q9}, [r3]!
	vld1.8 {q10, q11}, [r3]!
	vld1.8 {q12, q13}, [r3]!
	vld1.8 {q14, q15}, [r3]!

	vtrn.8 q0, q2
	vtrn.8 q4, q6
	vtrn.8 q8, q10
	vtrn.8 q12, q14
	vtrn.8 q1, q3
	vtrn.8 q5, q7
	vtrn.8 q9, q11
	vtrn.8 q13, q15

	vtrn.16 q0, q4
	vtrn.16 q2, q6
	vtrn.16 q8, q12
	vtrn.16 q10, q14
	vtrn.16 q1, q5
	vtrn.16 q3, q7
	vtrn.16 q9, q13
	vtrn.16 q11, q15

	vtrn.32 q0, q8
	vtrn.32 q2, q10
	vtrn.32 q4, q12
	vtrn.32 q6, q14
	vtrn.32 q1, q9
	vtrn.32 q3, q11
	vtrn.32 q5, q13
	vtrn.32 q7, q15

	vst1.8 d0, [r1], r2
	vst1.8 d4, [r1], r2
	vst1.8 d8, [r1], r2
	vst1.8 d12, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d24, [r1], r2
	vst1.8 d28, [r1], r2

	vst1.8 d1, [r1], r2
	vst1.8 d5, [r1], r2
	vst1.8 d9, [r1], r2
	vst1.8 d13, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d21, [r1], r2
	vst1.8 d25, [r1], r2
	vst1.8 d29, [r1], r2

	vst1.8 d2, [r1], r2
	vst1.8 d6, [r1], r2
	vst1.8 d10, [r1], r2
	vst1.8 d14, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d22, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d30, [r1], r2

	vst1.8 d3, [r1], r2
	vst1.8 d7, [r1], r2
	vst1.8 d11, [r1], r2
	vst1.8 d15, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d23, [r1], r2
	vst1.8 d27, [r1], r2
	vst1.8 d31, [r1], r2

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_29_Y_end

xPredIntraAngAdi_Y_29_H16_transpose:

	cmp r4, #16
	bne xPredIntraAngAdi_Y_29_H16W4_transpose

xPredIntraAngAdi_Y_29_H16W16_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	vld1.32 {q0}, [r3]!
	vld1.32 {q1}, [r3]!
	vld1.32 {q2}, [r3]!
	vld1.32 {q3}, [r3]!
	vld1.32 {q4}, [r3]!
	vld1.32 {q5}, [r3]!
	vld1.32 {q6}, [r3]!
	vld1.32 {q7}, [r3]!
	vld1.32 {q8}, [r3]!
	vld1.32 {q9}, [r3]!
	vld1.32 {q10}, [r3]!
	vld1.32 {q11}, [r3]!
	vld1.32 {q12}, [r3]!
	vld1.32 {q13}, [r3]!
	vld1.32 {q14}, [r3]!
	vld1.32 {q15}, [r3]

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_29_Y_end

xPredIntraAngAdi_Y_29_H16W4_transpose:

	sub r3, sp, #8192

	vld1.8 {q0, q1}, [r3]!
	vld1.8 {q2, q3}, [r3]

	vtrn.8 q0, q1
	vtrn.8 q2, q3

	vtrn.16 q0, q2
	vtrn.16 q1, q3

	vst1.32 d0[0], [r1], r2
	vst1.32 d2[0], [r1], r2
	vst1.32 d4[0], [r1], r2
	vst1.32 d6[0], [r1], r2

	vst1.32 d0[1], [r1], r2
	vst1.32 d2[1], [r1], r2
	vst1.32 d4[1], [r1], r2
	vst1.32 d6[1], [r1], r2

	vst1.32 d1[0], [r1], r2
	vst1.32 d3[0], [r1], r2
	vst1.32 d5[0], [r1], r2
	vst1.32 d7[0], [r1], r2

	vst1.32 d1[1], [r1], r2
	vst1.32 d3[1], [r1], r2
	vst1.32 d5[1], [r1], r2
	vst1.32 d7[1], [r1], r2

xPredIntraAngAdi_Y_29_Y_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function xPredIntraAngAdi_Y_31_neon128
@ pSrc-->r0, dst-->r1, i_dst-->r2, uiDirMode-->r3, iWidth-->r4, iHeight-->r5

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	lsl r6, r5, #1				@ iWidth2 = iWidth << 1
	mov r8, #0

	sub r3, sp, #8192
	push {r2, r4}
	add r0, #144

	mov r2, r4
	cmp r4, r5
	movlt r2, r5

	and r12, r5, #15
	cmp r12, #0
	beq xPredIntraAngAdi_Y_31_Y_H16

	and r12, r5, #7
	cmp r12, #0
	beq xPredIntraAngAdi_Y_31_Y_H8


xPredIntraAngAdi_Y_31_Y_H4:

	add r8, r8, #11				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_31_lezero_H4

xPredIntraAngAdi_Y_31_gtzero_H4:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 d4, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d4, d4, #1
	vext.8 d6, d4, d4, #2
	vext.8 d7, d4, d4, #3

	vmull.u8 q8, d0, d4			@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0[0], [r3]

	cmp r12, r5
	beq xPredIntraAngAdi_Y_31_gtzero_H4_end

	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0[0], [r7]

xPredIntraAngAdi_Y_31_gtzero_H4_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_31_Y_H4
	b xPredIntraAngAdi_Y_31_H4_transpose

xPredIntraAngAdi_Y_31_lezero_H4:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0[0], [r3]

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_31_Y_H4

xPredIntraAngAdi_Y_31_H4_transpose:
	pop {r2, r4}
	cmp r4, #4
	bne xPredIntraAngAdi_Y_31_H4W16_transpose

xPredIntraAngAdi_Y_31_H4W4_transpose:

	sub r3, sp, #8192
	vld1.8 {q0}, [r3]
	vuzp.8 d0, d1
	vuzp.8 d0, d1
	vst1.32 d0[0], [r1], r2
	vst1.32 d0[1], [r1], r2
	vst1.32 d1[0], [r1], r2
	vst1.32 d1[1], [r1], r2
	b xPredIntraAngAdi_Y_31_Y_end

xPredIntraAngAdi_Y_31_H4W16_transpose:

	sub r3, sp, #8192
	mov r9, #16
	vld1.32 d0[0], [r3], r9
	vld1.32 d2[0], [r3], r9
	vld1.32 d4[0], [r3], r9
	vld1.32 d6[0], [r3], r9

	vld1.32 d0[1], [r3], r9
	vld1.32 d2[1], [r3], r9
	vld1.32 d4[1], [r3], r9
	vld1.32 d6[1], [r3], r9

	vld1.32 d1[0], [r3], r9
	vld1.32 d3[0], [r3], r9
	vld1.32 d5[0], [r3], r9
	vld1.32 d7[0], [r3], r9

	vld1.32 d1[1], [r3], r9
	vld1.32 d3[1], [r3], r9
	vld1.32 d5[1], [r3], r9
	vld1.32 d7[1], [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3

	vtrn.16 q0, q2
	vtrn.16 q1, q3

	vst1.8 {q0}, [r1], r2 
	vst1.8 {q1}, [r1], r2 
	vst1.8 {q2}, [r1], r2 
	vst1.8 {q3}, [r1], r2 
	b xPredIntraAngAdi_Y_31_Y_end

xPredIntraAngAdi_Y_31_Y_H8:

	add r8, r8, #11				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_31_lezero_H8

xPredIntraAngAdi_Y_31_gtzero_H8:
	
	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	add r7, r0, r9
	vld1.32 {q10}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 d5, d20, d21, #1
	vext.8 d6, d20, d21, #2
	vext.8 d7, d20, d21, #3

	vmull.u8 q8, d0, d20		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q8, d1, d5
	vmlal.u8 q8, d2, d6
	vmlal.u8 q8, d3, d7

	vrshrn.u16 d0, q8, #7
	vst1.32 d0, [r3]

	cmp r12, r5
	beq xPredIntraAngAdi_Y_31_gtzero_H8_end

	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 d0, r11
	vst1.32 d0, [r7]

xPredIntraAngAdi_Y_31_gtzero_H8_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_31_Y_H8
	b xPredIntraAngAdi_Y_31_H8_transpose

xPredIntraAngAdi_Y_31_lezero_H8:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vst1.32 d0, [r3]

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_31_Y_H8

xPredIntraAngAdi_Y_31_H8_transpose:
	pop {r2, r4}
	cmp r4, #8
	bne xPredIntraAngAdi_Y_31_H8W32_transpose	

xPredIntraAngAdi_Y_31_H8W8_transpose:

	sub r3, sp, #8192
	vld1.32 {q0, q1}, [r3]!	
	vld1.32 {q2, q3}, [r3]
	
	vtrn.8 d0, d1
	vtrn.8 d2, d3
	vtrn.8 d4, d5
	vtrn.8 d6, d7

	vtrn.16 d0, d2
	vtrn.16 d1, d3
	vtrn.16 d4, d6
	vtrn.16 d5, d7

	vtrn.32 d0, d4
	vtrn.32 d1, d5
	vtrn.32 d2, d6
	vtrn.32 d3, d7
	
	vst1.32 d0, [r1], r2
	vst1.32 d1, [r1], r2
	vst1.32 d2, [r1], r2
	vst1.32 d3, [r1], r2
	vst1.32 d4, [r1], r2
	vst1.32 d5, [r1], r2
	vst1.32 d6, [r1], r2
	vst1.32 d7, [r1], r2
	b xPredIntraAngAdi_Y_31_Y_end

xPredIntraAngAdi_Y_31_H8W32_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	mov r9, #32
	vld1.32 d0, [r3], r9
	vld1.32 d4, [r3], r9
	vld1.32 d8, [r3], r9
	vld1.32 d12, [r3], r9
	vld1.32 d16, [r3], r9
	vld1.32 d20, [r3], r9
	vld1.32 d24, [r3], r9
	vld1.32 d28, [r3], r9

	vld1.32 d1, [r3], r9
	vld1.32 d5, [r3], r9
	vld1.32 d9, [r3], r9
	vld1.32 d13, [r3], r9
	vld1.32 d17, [r3], r9
	vld1.32 d21, [r3], r9
	vld1.32 d25, [r3], r9
	vld1.32 d29, [r3], r9

	vld1.32 d2, [r3], r9
	vld1.32 d6, [r3], r9
	vld1.32 d10, [r3], r9
	vld1.32 d14, [r3], r9
	vld1.32 d18, [r3], r9
	vld1.32 d22, [r3], r9
	vld1.32 d26, [r3], r9
	vld1.32 d30, [r3], r9

	vld1.32 d3, [r3], r9
	vld1.32 d7, [r3], r9
	vld1.32 d11, [r3], r9
	vld1.32 d15, [r3], r9
	vld1.32 d19, [r3], r9
	vld1.32 d23, [r3], r9
	vld1.32 d27, [r3], r9
	vld1.32 d31, [r3], r9

	vtrn.8 q0, q2
	vtrn.8 q4, q6
	vtrn.8 q8, q10
	vtrn.8 q12, q14
	vtrn.8 q1, q3
	vtrn.8 q5, q7
	vtrn.8 q9, q11
	vtrn.8 q13, q15

	vtrn.16 q0, q4
	vtrn.16 q2, q6
	vtrn.16 q8, q12
	vtrn.16 q10, q14
	vtrn.16 q1, q5
	vtrn.16 q3, q7
	vtrn.16 q9, q13
	vtrn.16 q11, q15

	vtrn.32 q0, q8
	vtrn.32 q2, q10
	vtrn.32 q4, q12
	vtrn.32 q6, q14
	vtrn.32 q1, q9
	vtrn.32 q3, q11
	vtrn.32 q5, q13
	vtrn.32 q7, q15

	vst1.8 {q0, q1}, [r1], r2
	vst1.8 {q2, q3}, [r1], r2
	vst1.8 {q4, q5}, [r1], r2
	vst1.8 {q6, q7}, [r1], r2
	vst1.8 {q8, q9}, [r1], r2
	vst1.8 {q10, q11}, [r1], r2
	vst1.8 {q12, q13}, [r1], r2
	vst1.8 {q14, q15}, [r1], r2

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_31_Y_end

xPredIntraAngAdi_Y_31_Y_H16:

	add r8, r8, #11				@ iTempDn = iTempD * imult
	lsr r9, r8, #3				@ idx = iTempDn >> ishift

	lsl r10, r8, #5				@ *offset = ((iTempDn << 5) >> ishift) - ((iTempDn >> ishift) << 5)
	lsr r10, r10, #3
	lsr r12, r8, #3
	lsl r12, r12, #5
	sub r10, r10, r12

	sub r12, r6, r9				@ real_width = min(iWidth, iWidth2 - idx + 1)
	add r12, #1
	cmp r5, r12
	movlt r12, r5

	cmp r12, #0
	ble xPredIntraAngAdi_Y_31_lezero_H16

xPredIntraAngAdi_Y_31_gtzero_H16:

	mov r11, #32
	sub r7, r11, r10			@ c1 = 32 - offset
	vdup.i8 d0, r7
	add r7, r11, r10			@ c3 = 32 + offset
	vdup.i8 d2, r7
	mov r11, #64
	sub r7, r11, r10			@ c2 = 64 - offset
	vdup.i8 d1, r7
	vdup.i8 d3, r10				@ c4 = offset

	mov r10, #0					@ x = 0

xPredIntraAngAdi_Y_31_gtzero_H16_LoopX:
	add r7, r0, r9
	vld1.32 {q8, q9}, [r7]			@ load pSrc[idx] - pSrc[idx + 7]
	vext.8 q10, q8, q9, #1
	vext.8 q11, q8, q9, #2
	vext.8 q12, q8, q9, #3

	vmull.u8 q13, d0, d16		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d20
	vmlal.u8 q13, d2, d22
	vmlal.u8 q13, d3, d24
	vrshrn.u16 d28, q13, #7

	vmull.u8 q13, d0, d17		@ dst[i] = (pel_t)((pSrc[idx] * c1 + pSrc[idx + 1] * c2 + pSrc[idx + 2] * c3 + pSrc[idx + 3] * c4 + 64) >> 7)
	vmlal.u8 q13, d1, d21
	vmlal.u8 q13, d2, d23
	vmlal.u8 q13, d3, d25
	vrshrn.u16 d29, q13, #7

	add r7, r3, r10
	vst1.32 {q14}, [r7]

	add r10, #16
	add r9, #16
	cmp r10, r12
	blt xPredIntraAngAdi_Y_31_gtzero_H16_LoopX

	cmp r12, r5
	beq xPredIntraAngAdi_Y_31_gtzero_H16_end


	add r7, r3, r12			@ dst[real_width]
	ldrb r11, [r7, #-1]		@ dst[real_width - 1]
	vdup.8 q0, r11

xPredIntraAngAdi_Y_31_gtzero_H16_LoopX2:
	vst1.32 {q0}, [r7]!
	
	add r12, #16
	cmp r12, r5
	blt xPredIntraAngAdi_Y_31_gtzero_H16_LoopX2

xPredIntraAngAdi_Y_31_gtzero_H16_end:
	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_31_Y_H16
	b xPredIntraAngAdi_Y_31_H16n_transpose

xPredIntraAngAdi_Y_31_lezero_H16:

	mov r11, #32
	sub r7, r11, r10		@ c1 = 32 - offset
	vmov.i8 d0[0], r7
	add r7, r11, r10		@ c3 = 32 + offset
	vmov.i8 d0[2], r7
	mov r11, #64
	sub r7, r11, r10		@ c2 = 64 - offset
	vmov.i8 d0[1], r7
	vmov.i8 d0[3], r10		@ c4 = offset
	
	add r7, r0, r6
	vld1.32 d1[0], [r7]		@ load pSrc[iWidth2 - 1] - pSrc[iWidth2 + 2]

	vmull.u8 q0, d0, d1		@ val = (pel_t)((pSrc[iWidth2 - 1] * c1 + pSrc[iWidth2] * c2 + pSrc[iWidth2 + 1] * c3 + pSrc[iWidth2 + 2] * c4 + 64) >> 7)
	vpadd.u16 d0, d0, d0
	vpadd.u16 d0, d0, d0
	vmov d1, d0
	vrshrn.u16 d0, q0, #7
	vmov d1, d0
	
	mov r7, r3
	mov r10, #0	
xPredIntraAngAdi_Y_31_lezero_H16_loop:

	vst1.32 {q0}, [r7]!

	add r10, #16
	cmp r10, r5
	blt xPredIntraAngAdi_Y_31_lezero_H16_loop

	add r3, r3, r2
	sub r4, #1
	cmp r4, #0
	bne xPredIntraAngAdi_Y_31_Y_H16

xPredIntraAngAdi_Y_31_H16n_transpose:
	pop {r2, r4}

	cmp r5, #16
	beq xPredIntraAngAdi_Y_31_H16_transpose

	cmp r5, #32
	beq xPredIntraAngAdi_Y_31_H32_transpose

xPredIntraAngAdi_Y_31_H64W64_transpose:

	sub r3, sp, #8192
	mov r7, r1
	mov r8, r3
	vpush {q4-q7}
	mov r9, #64
	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #16
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #32
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #48
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #32

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #48

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_31_Y_end

xPredIntraAngAdi_Y_31_H32_transpose:

	cmp r4, #32
	bne xPredIntraAngAdi_Y_31_H32W8_transpose

xPredIntraAngAdi_Y_31_H32W32_transpose:

	sub r3, sp, #8192
	mov r7, r1
	mov r8, r3
	vpush {q4-q7}

	mov r9, #32
	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r3, r8, #16
	lsl r1, r2, #4
	add r7, r1
	mov r1, r7

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	add r1, r7, #16

	vld1.32 {q0}, [r3], r9
	vld1.32 {q1}, [r3], r9
	vld1.32 {q2}, [r3], r9
	vld1.32 {q3}, [r3], r9
	vld1.32 {q4}, [r3], r9
	vld1.32 {q5}, [r3], r9
	vld1.32 {q6}, [r3], r9
	vld1.32 {q7}, [r3], r9
	vld1.32 {q8}, [r3], r9
	vld1.32 {q9}, [r3], r9
	vld1.32 {q10}, [r3], r9
	vld1.32 {q11}, [r3], r9
	vld1.32 {q12}, [r3], r9
	vld1.32 {q13}, [r3], r9
	vld1.32 {q14}, [r3], r9
	vld1.32 {q15}, [r3], r9

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_31_Y_end

xPredIntraAngAdi_Y_31_H32W8_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	vld1.8 {q0, q1}, [r3]!
	vld1.8 {q2, q3}, [r3]!
	vld1.8 {q4, q5}, [r3]!
	vld1.8 {q6, q7}, [r3]!
	vld1.8 {q8, q9}, [r3]!
	vld1.8 {q10, q11}, [r3]!
	vld1.8 {q12, q13}, [r3]!
	vld1.8 {q14, q15}, [r3]!

	vtrn.8 q0, q2
	vtrn.8 q4, q6
	vtrn.8 q8, q10
	vtrn.8 q12, q14
	vtrn.8 q1, q3
	vtrn.8 q5, q7
	vtrn.8 q9, q11
	vtrn.8 q13, q15

	vtrn.16 q0, q4
	vtrn.16 q2, q6
	vtrn.16 q8, q12
	vtrn.16 q10, q14
	vtrn.16 q1, q5
	vtrn.16 q3, q7
	vtrn.16 q9, q13
	vtrn.16 q11, q15

	vtrn.32 q0, q8
	vtrn.32 q2, q10
	vtrn.32 q4, q12
	vtrn.32 q6, q14
	vtrn.32 q1, q9
	vtrn.32 q3, q11
	vtrn.32 q5, q13
	vtrn.32 q7, q15

	vst1.8 d0, [r1], r2
	vst1.8 d4, [r1], r2
	vst1.8 d8, [r1], r2
	vst1.8 d12, [r1], r2
	vst1.8 d16, [r1], r2
	vst1.8 d20, [r1], r2
	vst1.8 d24, [r1], r2
	vst1.8 d28, [r1], r2

	vst1.8 d1, [r1], r2
	vst1.8 d5, [r1], r2
	vst1.8 d9, [r1], r2
	vst1.8 d13, [r1], r2
	vst1.8 d17, [r1], r2
	vst1.8 d21, [r1], r2
	vst1.8 d25, [r1], r2
	vst1.8 d29, [r1], r2

	vst1.8 d2, [r1], r2
	vst1.8 d6, [r1], r2
	vst1.8 d10, [r1], r2
	vst1.8 d14, [r1], r2
	vst1.8 d18, [r1], r2
	vst1.8 d22, [r1], r2
	vst1.8 d26, [r1], r2
	vst1.8 d30, [r1], r2

	vst1.8 d3, [r1], r2
	vst1.8 d7, [r1], r2
	vst1.8 d11, [r1], r2
	vst1.8 d15, [r1], r2
	vst1.8 d19, [r1], r2
	vst1.8 d23, [r1], r2
	vst1.8 d27, [r1], r2
	vst1.8 d31, [r1], r2

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_31_Y_end

xPredIntraAngAdi_Y_31_H16_transpose:

	cmp r4, #16
	bne xPredIntraAngAdi_Y_31_H16W4_transpose

xPredIntraAngAdi_Y_31_H16W16_transpose:

	sub r3, sp, #8192
	vpush {q4-q7}

	vld1.32 {q0}, [r3]!
	vld1.32 {q1}, [r3]!
	vld1.32 {q2}, [r3]!
	vld1.32 {q3}, [r3]!
	vld1.32 {q4}, [r3]!
	vld1.32 {q5}, [r3]!
	vld1.32 {q6}, [r3]!
	vld1.32 {q7}, [r3]!
	vld1.32 {q8}, [r3]!
	vld1.32 {q9}, [r3]!
	vld1.32 {q10}, [r3]!
	vld1.32 {q11}, [r3]!
	vld1.32 {q12}, [r3]!
	vld1.32 {q13}, [r3]!
	vld1.32 {q14}, [r3]!
	vld1.32 {q15}, [r3]

	vtrn.8 q0, q1
	vtrn.8 q2, q3
	vtrn.8 q4, q5
	vtrn.8 q6, q7
	vtrn.8 q8, q9
	vtrn.8 q10, q11
	vtrn.8 q12, q13
	vtrn.8 q14, q15

	vtrn.16 q0, q2
	vtrn.16 q1, q3
	vtrn.16 q4, q6
	vtrn.16 q5, q7
	vtrn.16 q8, q10
	vtrn.16 q9, q11
	vtrn.16 q12, q14
	vtrn.16 q13, q15

	vtrn.32 q0, q4
	vtrn.32 q1, q5
	vtrn.32 q2, q6
	vtrn.32 q3, q7
	vtrn.32 q8, q12
	vtrn.32 q9, q13
	vtrn.32 q10, q14
	vtrn.32 q11, q15

	vswp d1, d16
	vswp d3, d18
	vswp d5, d20
	vswp d7, d22
	vswp d9, d24
	vswp d11, d26
	vswp d13, d28
	vswp d15, d30

	vst1.32 {q0}, [r1], r2
	vst1.32 {q1}, [r1], r2
	vst1.32 {q2}, [r1], r2
	vst1.32 {q3}, [r1], r2
	vst1.32 {q4}, [r1], r2
	vst1.32 {q5}, [r1], r2
	vst1.32 {q6}, [r1], r2
	vst1.32 {q7}, [r1], r2
	vst1.32 {q8}, [r1], r2
	vst1.32 {q9}, [r1], r2
	vst1.32 {q10}, [r1], r2
	vst1.32 {q11}, [r1], r2
	vst1.32 {q12}, [r1], r2
	vst1.32 {q13}, [r1], r2
	vst1.32 {q14}, [r1], r2
	vst1.32 {q15}, [r1]

	vpop {q4-q7}
	b xPredIntraAngAdi_Y_31_Y_end

xPredIntraAngAdi_Y_31_H16W4_transpose:

	sub r3, sp, #8192

	vld1.8 {q0, q1}, [r3]!
	vld1.8 {q2, q3}, [r3]

	vtrn.8 q0, q1
	vtrn.8 q2, q3

	vtrn.16 q0, q2
	vtrn.16 q1, q3

	vst1.32 d0[0], [r1], r2
	vst1.32 d2[0], [r1], r2
	vst1.32 d4[0], [r1], r2
	vst1.32 d6[0], [r1], r2

	vst1.32 d0[1], [r1], r2
	vst1.32 d2[1], [r1], r2
	vst1.32 d4[1], [r1], r2
	vst1.32 d6[1], [r1], r2

	vst1.32 d1[0], [r1], r2
	vst1.32 d3[0], [r1], r2
	vst1.32 d5[0], [r1], r2
	vst1.32 d7[0], [r1], r2

	vst1.32 d1[1], [r1], r2
	vst1.32 d3[1], [r1], r2
	vst1.32 d5[1], [r1], r2
	vst1.32 d7[1], [r1], r2

xPredIntraAngAdi_Y_31_Y_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


.global Plane_tab_log2_sub2
Plane_tab_log2_sub2:
.byte -1, -2,  -1, -1,  0, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  3,-1, -1, -1, -1, -1, -1, -1, -1,-1, -1, -1, -1, -1, -1, -1, -1,-1, -1, -1, -1, -1, -1, -1, -1,-1, -1, -1, -1, -1, -1, -1,  4, -1

.global Plane_tab_mul_shift
Plane_tab_mul_shift:
.byte 13, 17, 5, 11, 23, 7, 10, 11, 15, 19

function xPredIntraPlaneAdi_neon128
@r0-->*psrc, r1-->*dst, r2-->i_dst, r3-->width, r4-->height

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4}
	sub sp, sp, #40

	adr r12, Plane_tab_log2_sub2
	add r10, r12, r3					@ tab_log2[iWidth] - 2
	ldrb r8, [r10]

	add r10, r12, r4					@ tab_log2[iHeight] - 2
	ldrb r9, [r10]

	adr r12, Plane_tab_mul_shift
	add r10, r8, r12					@ im_h = ib_mult[tab_log2[iWidth] - 2]
	ldrb r5, [r10]

	add r10, #5							@ is_h = ib_shift[tab_log2[iWidth] - 2]
	ldrb r6, [r10]

	add r10, r9, r12					@ im_v = ib_mult[tab_log2[iHeight] - 2]
	ldrb r7, [r10]

	add r10, #5							@ is_v = ib_shift[tab_log2[iHeight] - 2]
	ldrb r8, [r10]

	vmov.i32 d30, r5, r7				@ im_h, im_v
	vmov.i32 d31, r6, r8				@ is_h, is_v

	lsr r9, r3, #1						@ iW2
	lsr r10, r4, #1						@ iH2

	cmp r9, #4
	blt Plane_iH_lt4
	beq Plane_iH_eq4

Plane_iH_gt4:
	mov r6, #0x0304
	movt r6, #0x0102
	mov r7, #0x08
	add r7, #0x0700
	movt r7, #0x0506
	vmov.i32 d28, r7, r6				@ d28[0]: 8, 7, 6, 5, 4, 3, 2, 1(char)
	vmovl.u8 q13, d28					@ q13: 8, 7, 6, 5, 4, 3, 2, 1(short)
	vmov.i16 q12, #8
	vmov.i32 q0, #0
	
	mov r8, #0
	add r5, r0, r9						@ rpSrc = pSrc + iW2
	sub r6, r5, #8
Plane_iH_gt4_loop:
	 
	vld1.8 {q1}, [r5]
	vld1.8 d4, [r6]
	vtbl.8 d2, {q1}, d28

	vsubl.u8 q1, d2, d4					@ x * (rpSrc[x] - rpSrc[-x])
	vmull.s16 q2, d2, d26
	vmull.s16 q3, d3, d27
	vadd.s32 q2, q2, q3
	vadd.s32 q0, q0, q2

	add r8, #8
	vadd.u16 q13, q13, q12
	add r5, #8
	sub r6, #8
	cmp r8, r9
	blt Plane_iH_gt4_loop

	vadd.s32 d0, d0, d1
	b Plane_iH_end

Plane_iH_lt4:

	add r5, r0, r9						@ rpSrc = pSrc + iW2
	ldrb r6, [r5, #1]
	ldrb r7, [r5, #-1]
	sub r8, r6, r7
 	ldrb r6, [r5, #2]
	ldrb r7, [r5, #-2]
	sub r12, r6, r7
	lsl r12, #1
	add r8, r12
	mov r12, #0
	vmov.i32 d0, r8, r12

	b Plane_iH_end

Plane_iH_eq4:
	mov r6, #0x0304
	movt r6, #0x0102
	vmov.i32 d28[0], r6					@ d28[0]: 4, 3, 2, 1(char)
	vmovl.u8 q13, d28					@ d26: 4, 3, 2, 1(short)

	add r5, r0, r9						@ rpSrc = pSrc + iW2
	sub r6, r5, #4

	vld1.8 d0, [r5]
	vld1.8 d1, [r6]

	vtbl.8 d0, {d0}, d28

	vsubl.u8 q0, d0, d1					@ x * (rpSrc[x] - rpSrc[-x])
	vmull.s16 q0, d0, d26

	vadd.s32 d0, d0, d1

Plane_iH_end:

	cmp r10, #4
	blt Plane_iV_lt4
	beq Plane_iV_eq4

Plane_iV_gt4:
	
	mov r6, #0x0304
	movt r6, #0x0102
	mov r7, #0x08
	add r7, #0x0700
	movt r7, #0x0506
	vmov.i32 d28, r7, r6				@ d28[0]: 8, 7, 6, 5, 4, 3, 2, 1(char)
	vmovl.u8 q13, d28					@ q13: 8, 7, 6, 5, 4, 3, 2, 1(short)
	vmov.i16 q12, #8
	vmov.i32 q8, #0

	mov r8, #0
	sub r5, r0, r10						@ rpSrc = pSrc - iH2
	sub r6, r5, #8
Plane_iV_gt4_loop:

	vld1.8 {q1}, [r5]
	vld1.8 d4, [r6]
	vtbl.8 d2, {q1}, d28

	vsubl.u8 q1, d4, d2					@ y * (rpSrc[-y] - rpSrc[y])
	vmull.s16 q2, d2, d26
	vmull.s16 q3, d3, d27
	vadd.s32 q2, q2, q3
	vadd.s32 q8, q8, q2

	add r8, #8
	vadd.u16 q13, q13, q12
	add r5, #8
	sub r6, #8
	cmp r8, r10
	blt Plane_iV_gt4_loop

	vadd.s32 d1, d16, d17
	b Plane_iV_end

Plane_iV_lt4:

	sub r5, r0, r10						@ rpSrc = pSrc - iH2
	ldrb r6, [r5, #1]
	ldrb r7, [r5, #-1]
	sub r8, r7, r6
 	ldrb r6, [r5, #2]
	ldrb r7, [r5, #-2]
	sub r12, r7, r6
	lsl r12, #1
	add r8, r12
	mov r12, #0
	vmov.i32 d1, r8, r12

	b Plane_iV_end

Plane_iV_eq4:

	mov r6, #0x0304
	movt r6, #0x0102
	vmov.i32 d28[0], r6					@ d28[0]: 4, 3, 2, 1(char)
	vmovl.u8 q13, d28					@ d26: 4, 3, 2, 1(short)

	sub r5, r0, r10						@ rpSrc = pSrc - iH2
	sub r6, r5, #4

	vld1.8 d2, [r5]
	vld1.8 d3, [r6]

	vtbl.8 d2, {d2}, d28

	vsubl.u8 q1, d3, d2					@ x * (rpSrc[x] - rpSrc[-x])
	vmull.s16 q1, d2, d26

	vadd.s32 d1, d2, d3

Plane_iV_end:
	vpadd.s32 d0, d0, d1				@ iH and iV

	sub r5, r0, r4						@ iA = (pSrc[-1 - (iHeight - 1)] + pSrc[1 + iWidth - 1]) << 4
	ldrb r6, [r5]
	add r5, r0, r3
	ldrb r7, [r5]
	add r6, r7
	lsl r6, #4

	vshl.s32 d0, #5						@ iB = ((iH << 5) * im_h + (1 << (is_h - 1))) >> is_h
	vmul.s32 d0, d0, d30				@ iC = ((iV << 5) * im_v + (1 << (is_v - 1))) >> is_v
	vmov.i32 d2, #1
	vsub.s32 d1, d31, d2
	vshl.s32 d1, d2, d1
	vadd.s32 d0, d1
	vneg.s32 d31, d31
	vshl.s32 d0, d31
	vmov.i32 r8, r12, d0				@ iB, iC
	vdup.16 q15, r12
	vdup.16 q14, r8

	vmov.i32 d1, r9, r10				@ iTmp = iA - (iH2 - 1) * iC - (iW2 - 1) * iB + 16
	vsub.s32 d1, d2
	vmul.s32 d0, d0, d1
	vmov.i32 r7, r8, d0
	sub r6, r7
	sub r6, r8
	add r6, #16
	vdup.16 q13, r6

	mov r6, #0x0100
	movt r6, #0x0302
	mov r7, #0x04
	add r7, #0x0500
	movt r7, #0x0706
	vmov.i32 d24, r6, r7
	vmovl.u8 q12, d24					@ q13: 0, 1, 2, 3, 4, 5, 6, 7(short)

	vmul.s16 q0, q14, q12
	vadd.s16 q0, q0, q13

	cmp r3, #8
	ble Plane_le8

Plane_gt8:
	vmov.i16 q11, #0
	vmov.i16 q10, #255
	mov r5, #0
	sub r2, r3
	vshl.s16 q14, #3
	vmov q3, q0
Plane_gt8_y:
	mov r6, #0
	vmov q0, q3
Plane_gt8_x:

	vshr.s16 q1, q0, #5
	vmax.s16 q1, q1, q11
	vmin.s16 q1, q1, q10
	vmovn.u16 d4, q1
	vadd.s16 q0, q14
	vshr.s16 q1, q0, #5
	vmax.s16 q1, q1, q11
	vmin.s16 q1, q1, q10
	vmovn.u16 d5, q1
	vadd.s16 q0, q14

	vst1.8 {q2}, [r1]!
	add r6, #16
	cmp r6, r3
	blt Plane_gt8_x

	vadd.s16 q3, q3, q15
	add r1, r2
	add r5, #1
	cmp r5, r4
	blt Plane_gt8_y

	b Plane_end

Plane_le8:
	vmov.i16 q11, #0
	vmov.i16 q10, #255
	mov r5, #0
Plane_le8_y:
	vshr.s16 q1, q0, #5
	vmax.s16 q1, q1, q11
	vmin.s16 q1, q1, q10
	vmovn.u16 d2, q1
	vst1.8 d2, [r1]
	add r1, r2
	vadd.s16 q0, q0, q15
	add r5, #1
	cmp r5, r4
	blt Plane_le8_y

Plane_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

