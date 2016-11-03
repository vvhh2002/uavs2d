.macro function name
     .global \name
\name:
     .hidden \name
     .type   \name, %function
     .func   \name
\name:
.endm

function com_if_filter_hor_4_neon128 
@ *src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, width-->r4, height-->r5, *coeff-->r6, max_val-->r7

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6, r7}
	sub sp, sp, #40

	@ set buffers for offset (1<<IF_FILTER_PREC - 1)
	vmov.i16 q15, #0x20
	
	@ load coefficients
	@q14: coeff[3] ... coeff[0] coeff[3] ... coeff[0]
	vld1.8 {d28}, [r6]
	vmovl.s8 q14,d28
	vmov d29, d28

	@ set buffers for clip(0, 255)
	vmov.s16 q13, #255
	vmov.s16 q12, #0

	@ branch
	and r7, r4, #7
	cmp r7, #0
	beq com_if_filter_hor_4_neon128_8

	and r7, r4, #3
	cmp r7, #0
	beq com_if_filter_hor_4_neon128_4

	b com_if_filter_hor_4_neon128_2

com_if_filter_hor_4_neon128_8:
	sub r7, r7, r7					@ y = 0
com_if_filter_hor_4_neon128_8_y:
	sub r8, r8, r8					@ x = 0
com_if_filter_hor_4_neon128_8_x:
	sub r9, r8, #1					@ x - 1
	add r10, r0, r9					@ &src[x - 1]
	vld1.8 {q0}, [r10]

	@ convert byte to short
	vmovl.u8 q1, d1					@ q1: p15 ... p12 p11 ... p8
	vmovl.u8 q0, d0					@ q0: p7 ... p4 p3 ... p0

	vmov.i16 d4, d0
	vext.i16 d5, d0, d1, #1 			@ q2: p4 ... p1 p3 ... p0
	vext.i16 d6, d0, d1, #2 
	vext.i16 d7, d0, d1, #3 			@ q3: p6 ... p3 p5 ... p2
	vmov.i16 d8, d1
	vext.i16 d9, d1, d2, #1 			@ q4: p8 ... p5 p7 ... p4
	vext.i16 d10, d1, d2, #2
	vext.i16 d11, d1, d2, #3 			@ q5: p10 ... p7 p9 ... p6

	@ multiply
	vmul.s16 q2, q2, q14
	vmul.s16 q3, q3, q14
	vmul.s16 q4, q4, q14
	vmul.s16 q5, q5, q14

	@ pair add
	vpadd.s16 d4, d4, d5
	vpadd.s16 d5, d6, d7
	vpadd.s16 d6, d8, d9
	vpadd.s16 d7, d10, d11
	
	vpadd.s16 d4, d4, d5
	vpadd.s16 d5, d6, d7
	
	@ offset and shift
	vadd.s16 q2, q2, q15
	vshr.s16 q1, q2, #6
	
	@ clip(0,255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d0, q1

	@ store results
	add r10, r2, r8
	vst1.8 {d0}, [r10]

	@--------------------------------
	@ loop control
	@--------------------------------
	add r8, r8, #8					@ x += 8
	cmp r8, r4
	blt com_if_filter_hor_4_neon128_8_x

	add r0, r0, r1					@ src += srcStride
	add r2, r2, r3					@ dst += dstStride

	add r7, r7, #1					@ y += 1
	cmp r7, r5
	blt com_if_filter_hor_4_neon128_8_y
	
	b com_if_filter_hor_4_neon128_end

com_if_filter_hor_4_neon128_4:
	sub r7, r7, r7					@ y = 0
com_if_filter_hor_4_neon128_4_y:
	sub r8, r8, r8					@ x = 0
com_if_filter_hor_4_neon128_4_x:
	sub r9, r8, #1					@ x - 1
	add r10, r0, r9					@ &src[x - 1]
	vld1.8 {d0}, [r10]

	@ convert byte to short
	vmovl.u8 q0, d0					@ q0: p8 ... p4 p3 ... p0

	@ prepare for sequences
	vmov.i16 d4, d0
	vext.i16 d5, d0, d1, #1 			@ q2: p4 ... p1 p3 ... p0
	vext.i16 d6, d0, d1, #2 
	vext.i16 d7, d0, d1, #3 			@ q3: p6 ... p3 p5 ... p2

	@ multiply
	vmul.s16 q2, q2, q14
	vmul.s16 q3, q3, q14

	@ pair add
	vpadd.s16 d4, d4, d5
	vpadd.s16 d5, d6, d7
	
	vpadd.s16 d4, d4, d5
	
	@ offset and shift
	vadd.s16 q2, q2, q15
	vshr.s16 q1, q2, #6
	
	@ clip(0,255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d0, q1

	@ store results
	add r10, r2, r8
	vst1.32 {d0[0]}, [r10]

	@--------------------------------
	@ loop control
	@--------------------------------
	add r8, r8, #4					@ x += 4
	cmp r8, r4
	blt com_if_filter_hor_4_neon128_4_x

	add r0, r0, r1					@ src += srcStride
	add r2, r2, r3					@ dst += dstStride

	add r7, r7, #1					@ y += 1
	cmp r7, r5
	blt com_if_filter_hor_4_neon128_4_y

	b com_if_filter_hor_4_neon128_end
	
com_if_filter_hor_4_neon128_2:
	sub r7, r7, r7					@ y = 0
com_if_filter_hor_4_neon128_2_y:
	sub r8, r8, r8					@ x = 0
com_if_filter_hor_4_neon128_2_x:
	sub r9, r8, #1					@ x - 1
	add r10, r0, r9					@ &src[x - 1]
	vld1.8 {d0}, [r10]

	@ convert byte to short
	vmovl.u8 q0, d0					@ q0: p3 ... p0

	@ prepare for sequences
	vmov.i16 d4, d0
	vext.i16 d5, d0, d1, #1 			@ q2: p4 ... p1 p3 ... p0

	@ multiply
	vmul.s16 q2, q2, q14

	@ pair add
	vpadd.s16 d4, d4, d5
	vpadd.s16 d4, d4, d5
	
	@ offset and shift
	vadd.s16 q2, q2, q15
	vshr.s16 q1, q2, #6
	
	@ clip(0,255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d0, q1

	@ store results
	add r10, r2, r8
	vst1.16 {d0[0]}, [r10]

	@--------------------------------
	@ loop control
	@--------------------------------
	add r8, r8, #2					@ x += 4
	cmp r8, r4
	blt com_if_filter_hor_4_neon128_2_x

	add r0, r0, r1					@ src += srcStride
	add r2, r2, r3					@ dst += dstStride

	add r7, r7, #1					@ y += 1
	cmp r7, r5
	blt com_if_filter_hor_4_neon128_2_y


com_if_filter_hor_4_neon128_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function com_if_filter_hor_8_neon128
	@r0: src; r1: srcStride; r2:dst; r3:dstStride; r4:width; r5:height; r6:coeff
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6}
	sub sp, sp, #40

	@ set buffers for offset (1<<IF_FILTER_PREC - 1)
	vmov.i16 q15, #0x20
	
	@ load coefficients
	vld1.s8 {d28}, [r6]
	vmovl.s8 q14,d28

	@ set buffers for clip(0, 255)
	vmov.s16 q13, #255
	vmov.s16 q12, #0

	@ branch
	and r7, r4, #7
	cmp r7, #0
	beq com_if_filter_hor_8_neon128_8
	
	b com_if_filter_hor_8_neon128_4

com_if_filter_hor_8_neon128_8:
	sub r7, r7, r7						@ y = 0
com_if_filter_hor_8_neon128_8_y:
	sub r8, r8, r8						@ x = 0
com_if_filter_hor_8_neon128_8_x:
	add r10, r0, #-3					@ address of src[x - 3]
	add r10, r10, r8
	vld1.8 {q0}, [r10]

	@ convert byte to short
	vmovl.u8 q10, d0
	vmovl.u8 q11, d1

	@ prepare for sequences
	vmov.i16 q3, q10
	vext.i16 q4, q10, q11, #1
	vext.i16 q5, q10, q11, #2
	vext.i16 q6, q10, q11, #3
	vext.i16 q7, q10, q11, #4
	vext.i16 q8, q10, q11, #5
	vext.i16 q9, q10, q11, #6
	vext.i16 q10, q10, q11, #7

	@ multiply
	vmul.s16 q0, q3, q14
	vmul.s16 q1, q4, q14
	vmul.s16 q2, q5, q14
	vmul.s16 q3, q6, q14
	vmul.s16 q4, q7, q14
	vmul.s16 q5, q8, q14
	vmul.s16 q6, q9, q14
	vmul.s16 q7, q10, q14

	@ pair add
	vpadd.s16 d16, d0, d1
	vpadd.s16 d17, d2, d3
	vpadd.s16 d18, d4, d5
	vpadd.s16 d19, d6, d7
	vpadd.s16 d20, d8, d9
	vpadd.s16 d21, d10, d11
	vpadd.s16 d22, d12, d13
	vpadd.s16 d23, d14, d15

	@ accumulate
	vpadd.s16 d0, d16, d17
	vpadd.s16 d1, d18, d19
	vpadd.s16 d2, d20, d21
	vpadd.s16 d3, d22, d23
	vpadd.s16 d4, d0, d1
	vpadd.s16 d5, d2, d3

	@ offset and shift
	vadd.s16 q0, q2, q15
	vshr.s16 q2, q0, #6

	@ clip(0, 255)
	vmax.s16 q0, q2, q12
	vmin.s16 q0, q0, q13

	@ convert short to byte
	vmovn.s16 d2, q0

	@ store results into &dst[x]
	add r10, r2, r8
	vst1.64 {d2}, [r10]

	@--------------------------------
	@ loop controls
	@--------------------------------
	add r8, r8, #8
	cmp r8, r4
	blt com_if_filter_hor_8_neon128_8_x

	add r0, r0, r1					@ src += srcStride
	add r2, r2, r3					@ dst += dstStride
	
	add r7, r7, #1
	cmp r7, r5
	blt com_if_filter_hor_8_neon128_8_y
	
	b com_if_filter_hor_8_neon128_end
	
com_if_filter_hor_8_neon128_4:
	sub r7, r7, r7					@ y = 0
com_if_filter_hor_8_neon128_4_y:
	sub r8, r8, r8					@ x = 0
com_if_filter_hor_8_neon128_4_x:
	add r10, r0, #-3				@ address of src[x - 3]
	add r10, r10, r8
	vld1.8 {q0}, [r10]

	@ convert byte to short
	vmovl.u8 q10, d0
	vmovl.u8 q11, d1

	@ prepare for sequences
	vmov.i16 q3, q10
	vext.i16 q4, q10, q11, #1
	vext.i16 q5, q10, q11, #2
	vext.i16 q6, q10, q11, #3

	@ multiply
	vmul.s16 q0, q3, q14
	vmul.s16 q1, q4, q14
	vmul.s16 q2, q5, q14
	vmul.s16 q3, q6, q14

	@ pair add
	vpadd.s16 d16, d0, d1
	vpadd.s16 d17, d2, d3
	vpadd.s16 d18, d4, d5
	vpadd.s16 d19, d6, d7

	@ accumulate
	vpadd.s16 d0, d16, d17
	vpadd.s16 d1, d18, d19
	vpadd.s16 d2, d0, d1

	@ offset and shift
	vadd.s16 d0, d2, d31
	vshr.s16 d1, d0, #6

	@ clip(0, 255)
	vmax.s16 d0, d1, d24
	vmin.s16 d0, d0, d26

	@ convert short to byte
	vmovn.s16 d2, q0

	@ store results into &dst[x]
	add r10, r2, r8
	vst1.32 {d2[0]}, [r10]

	@--------------------------------
	@ loop controls
	@--------------------------------
	add r8, r8, #4
	cmp r8, r4
	blt com_if_filter_hor_8_neon128_4_x

	add r0, r0, r1					@ src += srcStride
	add r2, r2, r3					@ dst += dstStride
	
	add r7, r7, #1
	cmp r7, r5
	blt com_if_filter_hor_8_neon128_4_y
	
com_if_filter_hor_8_neon128_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function com_if_filter_ver_4_neon128
	@r0: src; r1: srcStride; r2:dst; r3:dstStride; r4:width; r5:height; r6:coeff
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6}
	sub sp, sp, #40

	@ set buffers for offset (1<<IF_FILTER_PREC - 1)
	vmov.i16 q15, #0x20
		
	@ load coefficients(as scalar)
	vld1.8 {d0}, [r6]
	vmovl.s8 q0,d0

	@ set buffers for clip(0, 255)
	vmov.s16 q13, #255
	vmov.s16 q12, #0
	
	@ branch
	and r7, r4, #7
	cmp r7, #0
	beq com_if_filter_ver_4_neon128_8

	and r7, r4, #3
	cmp r7, #0
	beq com_if_filter_ver_4_neon128_4

	b com_if_filter_ver_4_neon128_2
	
com_if_filter_ver_4_neon128_8:
	sub r7, r7, r7					@ y = 0
com_if_filter_ver_4_neon128_8_y:
	sub r8, r8, r8					@ x = 0
com_if_filter_ver_4_neon128_8_x:
	@ load a block of pixels(4x8)
	sub r9, r8, r1
	add r10, r0, r9					@ &src[x - srcStride]
	vld1.64 {d2}, [r10]
	add r10, r1, r10
	vld1.64 {d3}, [r10]
	add r10, r1, r10
	vld1.64 {d4}, [r10]
	add r10, r1, r10
	vld1.64 {d5}, [r10]	

	@ convert byte to short
	vmovl.u8 q5, d2
	vmovl.u8 q6, d3
	vmovl.u8 q7, d4
	vmovl.u8 q8, d5

	@ multiply and accumulate
	vmul.s16 q1, q5, d0[0]
	vmla.s16 q1, q6, d0[1]
	vmla.s16 q1, q7, d0[2]
	vmla.s16 q1, q8, d0[3]
	
	@ offset and shift
	vadd.s16 q2, q1, q15
	vshr.s16 q1, q2, #6

	@ clip(0, 255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d4, q1

	@ store results into &dst[x]
	add r10, r2, r8
	vst1.64 {d4}, [r10]
	
	@--------------------------------
	@ loop controls
	@--------------------------------
	add r8, r8, #8
	cmp r8, r4
	blt com_if_filter_ver_4_neon128_8_x

	add r0, r0, r1					@ src += srcStride
	add r2, r2, r3					@ dst += dstStride
	
	add r7, r7, #1
	cmp r7, r5
	blt com_if_filter_ver_4_neon128_8_y
	
	b com_if_filter_ver_4_neon128_end
	
com_if_filter_ver_4_neon128_4:
	sub r7, r7, r7					@ y = 0
com_if_filter_ver_4_neon128_4_y:
	sub r8, r8, r8					@ x = 0
com_if_filter_ver_4_neon128_4_x:
	@ load a block of pixels(4x4)
	sub r9, r8, r1
	add r10, r0, r9					@ &src[x - srcStride]
	vld1.32 {d2[0]}, [r10]
	add r10, r1, r10
	vld1.32 {d3[0]}, [r10]
	add r10, r1, r10
	vld1.32 {d4[0]}, [r10]
	add r10, r1, r10
	vld1.32 {d5[0]}, [r10]	

	@ convert byte to short
	vmovl.u8 q5, d2
	vmovl.u8 q6, d3
	vmovl.u8 q7, d4
	vmovl.u8 q8, d5

	@ multiply and accumulate
	vmul.s16 q1, q5, d0[0]
	vmla.s16 q1, q6, d0[1]
	vmla.s16 q1, q7, d0[2]
	vmla.s16 q1, q8, d0[3]
	
	@ offset and shift
	vadd.s16 q2, q1, q15
	vshr.s16 q1, q2, #6

	@ clip(0, 255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d4, q1

	@ store results into &dst[x]
	add r10, r2, r8
	vst1.32 {d4[0]}, [r10]
	
	@--------------------------------
	@ loop controls
	@--------------------------------
	add r8, r8, #4
	cmp r8, r4
	blt com_if_filter_ver_4_neon128_4_x

	add r0, r0, r1					@ src += srcStride
	add r2, r2, r3					@ dst += dstStride
	
	add r7, r7, #1
	cmp r7, r5
	blt com_if_filter_ver_4_neon128_4_y

	b com_if_filter_ver_4_neon128_end
	
com_if_filter_ver_4_neon128_2:
	sub r7, r7, r7					@ y = 0
com_if_filter_ver_4_neon128_2_y:
	sub r8, r8, r8					@ x = 0
com_if_filter_ver_4_neon128_2_x:
	@ load a block of pixels(4x4)
	sub r9, r8, r1
	add r10, r0, r9					@ &src[x - srcStride]
	vld1.16 {d2[0]}, [r10]
	add r10, r1, r10
	vld1.16 {d3[0]}, [r10]
	add r10, r1, r10
	vld1.16 {d4[0]}, [r10]
	add r10, r1, r10
	vld1.16 {d5[0]}, [r10]	

	@ convert byte to short
	vmovl.u8 q5, d2
	vmovl.u8 q6, d3
	vmovl.u8 q7, d4
	vmovl.u8 q8, d5

	@ multiply and accumulate
	vmul.s16 q1, q5, d0[0]
	vmla.s16 q1, q6, d0[1]
	vmla.s16 q1, q7, d0[2]	
	vmla.s16 q1, q8, d0[3]
	
	@ offset and shift
	vadd.s16 q2, q1, q15
	vshr.s16 q1, q2, #6

	@ clip(0, 255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d4, q1

	@ store results into &dst[x]
	add r10, r2, r8
	vst1.16 {d4[0]}, [r10]
	
	@--------------------------------
	@ loop controls
	@--------------------------------
	add r8, r8, #2
	cmp r8, r4
	blt com_if_filter_ver_4_neon128_2_x

	add r0, r0, r1					@ src += srcStride
	add r2, r2, r3					@ dst += dstStride
	
	add r7, r7, #1
	cmp r7, r5
	blt com_if_filter_ver_4_neon128_2_y

com_if_filter_ver_4_neon128_end:	
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}	
.endfunc


function com_if_filter_ver_8_neon128
	@r0: src; r1: srcStride; r2:dst; r3:dstStride; r4:width; r5:height; r6:coeff
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6}
	sub sp, sp, #40

	@ set buffers for offset (1<<IF_FILTER_PREC - 1)
	vmov.i16 q15, #0x20
		
	@ load coefficients(as scalar)
	vld1.8 {d0}, [r6]
	vmovl.s8 q0,d0

	@ branch
	and r7, r4, #7
	cmp r7, #0
	beq com_if_filter_ver_8_neon128_8
	
	b com_if_filter_ver_8_neon128_4

com_if_filter_ver_8_neon128_8:
	sub r7, r7, r7					@ y = 0
com_if_filter_ver_8_neon128_8_y:
	sub r8, r8, r8					@ x = 0
com_if_filter_ver_8_neon128_8_x:
	mov r9, #3					@ &src[x - 3 * srcStride]
	mul r10, r1, r9
	sub r9, r8, r10

	@ load a block of pixels(8x8)
	add r10, r0, r9
	vld1.16 {d2}, [r10]
	add r10, r1, r10
	vld1.16 {d3}, [r10]
	add r10, r1, r10
	vld1.16 {d4}, [r10]
	add r10, r1, r10
	vld1.16 {d5}, [r10]	
	add r10, r1, r10
	vld1.16 {d6}, [r10]
	add r10, r1, r10
	vld1.16 {d7}, [r10]
	add r10, r1, r10
	vld1.16 {d8}, [r10]
	add r10, r1, r10
	vld1.16 {d9}, [r10]

	@ convert byte to short
	vmovl.u8 q5, d2
	vmovl.u8 q6, d3
	vmovl.u8 q7, d4
	vmovl.u8 q8, d5
	vmovl.u8 q9, d6
	vmovl.u8 q10, d7
	vmovl.u8 q11, d8
	vmovl.u8 q12, d9

	@ multiply and accumulate
	vmul.s16 q1, q5, d0[0]
	vmla.s16 q1, q6, d0[1]
	vmla.s16 q1, q7, d0[2]	
	vmla.s16 q1, q8, d0[3]
	vmla.s16 q1, q9, d1[0]
	vmla.s16 q1, q10, d1[1]	
	vmla.s16 q1, q11, d1[2]
	vmla.s16 q1, q12, d1[3]
	
	@ offset and shift
	vadd.s16 q1, q1, q15
	vshr.s16 q2, q1, #6

	@ clip(0, 255)
	vmov.i16 q1, #0x0
	vmax.s16 q2, q1, q2
	vmov.i16 q1, #0xff
	vmin.s16 q2, q1, q2

	@ convert short to byte
	vmovn.s16 d2, q2

	@ store results into &dst[x]
	add r10, r2, r8
	vst1.8 {d2}, [r10]
	
	@--------------------------------
	@ loop controls
	@--------------------------------
	add r8, r8, #8
	cmp r8, r4
	blt com_if_filter_ver_8_neon128_8_x

	add r0, r0, r1					@ src += srcStride
	add r2, r2, r3					@ dst += dstStride
	
	add r7, r7, #1
	cmp r7, r5
	blt com_if_filter_ver_8_neon128_8_y
	
	b com_if_filter_ver_8_neon128_end
	
com_if_filter_ver_8_neon128_4:
	sub r7, r7, r7					@ y = 0
com_if_filter_ver_8_neon128_4_y:
	sub r8, r8, r8					@ x = 0
com_if_filter_ver_8_neon128_4_x:
	mov r9, #3					@ &src[x - 3 * srcStride]
	mul r10, r1, r9
	sub r9, r8, r10

	@ load a block of pixels(4x8)
	add r10, r0, r9
	vld1.32 {d2[0]}, [r10]
	add r10, r1, r10
	vld1.32 {d3[0]}, [r10]
	add r10, r1, r10
	vld1.32 {d4[0]}, [r10]
	add r10, r1, r10
	vld1.32 {d5[0]}, [r10]	
	add r10, r1, r10
	vld1.32 {d6[0]}, [r10]
	add r10, r1, r10
	vld1.32 {d7[0]}, [r10]
	add r10, r1, r10
	vld1.32 {d8[0]}, [r10]
	add r10, r1, r10
	vld1.32 {d9[0]}, [r10]

	@ convert byte to short
	vmovl.u8 q5, d2
	vmovl.u8 q6, d3
	vmovl.u8 q7, d4
	vmovl.u8 q8, d5
	vmovl.u8 q9, d6
	vmovl.u8 q10, d7
	vmovl.u8 q11, d8
	vmovl.u8 q12, d9

	@ multiply and accumulate
	vmul.s16 q1, q5, d0[0]
	vmla.s16 q1, q6, d0[1]
	vmla.s16 q1, q7, d0[2]	
	vmla.s16 q1, q8, d0[3]
	vmla.s16 q1, q9, d1[0]
	vmla.s16 q1, q10, d1[1]	
	vmla.s16 q1, q11, d1[2]
	vmla.s16 q1, q12, d1[3]
	
	@ offset and shift
	vadd.s16 q1, q1, q15
	vshr.s16 q2, q1, #6

	@ clip(0, 255)
	vmov.i16 q1, #0x0
	vmax.s16 q2, q1, q2
	vmov.i16 q1, #0xff
	vmin.s16 q2, q1, q2

	@ convert short to byte
	vmovn.s16 d2, q2

	@ store results into &dst[x]
	add r10, r2, r8
	vst1.32 {d2[0]}, [r10]
	
	@--------------------------------
	@ loop controls
	@--------------------------------
	add r8, r8, #4
	cmp r8, r4
	blt com_if_filter_ver_8_neon128_4_x

	add r0, r0, r1					@ src += srcStride
	add r2, r2, r3					@ dst += dstStride
	
	add r7, r7, #1
	cmp r7, r5
	blt com_if_filter_ver_8_neon128_4_y
	
com_if_filter_ver_8_neon128_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc



function com_if_filter_hor_ver_4_neon128
@*src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, width-->r4, height-->r5, *coeff_h-->r6, *coeff_v-->r7, max_val-->r8
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6,r7,r8}
	sub sp, sp, #40
	
	@ align (r12)
	@ r12-->tmp
	and r12,sp,#15
	sub r12,sp,r12
	sub r12,#2240

	sub r0,r0,r1

	@ load coefficients_h
	@q14: coeff_h[3] ... coeff_h[0] coeff_h[3] ... coeff_h[0]
	vld1.8 {d28}, [r6]
	vmovl.s8 q14,d28
	vmov d29, d28

	@ set buffers for clip(0, 255)
	vmov.s16 q13, #255
	vmov.s16 q12, #0

@ Hor
	@ branch
	and r8, r4, #7
	cmp r8, #0
	beq com_if_filter_hor_4_8

	and r8, r4, #3
	cmp r8, #0
	beq com_if_filter_hor_4_4

	b com_if_filter_hor_4_2

com_if_filter_hor_4_8:
	mov r8, #-3					    @ y = -3
com_if_filter_hor_4_8_y:
	sub r9, r9, r9					@ x = 0	
com_if_filter_hor_4_8_x:
	sub r10, r9, #1					@ x - 1
	add r10, r0, r10					@ &src[x - 1]
	vld1.8 {q0}, [r10]

	@ convert byte to short
	vmovl.u8 q1, d1					@ q1: p15 ... p12 p11 ... p8
	vmovl.u8 q0, d0					@ q0: p7 ... p4 p3 ... p0

	vmov.i16 d4, d0
	vext.i16 d5, d0, d1, #1 			@ q2: p4 ... p1 p3 ... p0
	vext.i16 d6, d0, d1, #2 
	vext.i16 d7, d0, d1, #3 			@ q3: p6 ... p3 p5 ... p2
	vmov.i16 d8, d1
	vext.i16 d9, d1, d2, #1 			@ q4: p8 ... p5 p7 ... p4
	vext.i16 d10, d1, d2, #2
	vext.i16 d11, d1, d2, #3 			@ q5: p10 ... p7 p9 ... p6

	@ multiply
	vmul.s16 q2, q2, q14
	vmul.s16 q3, q3, q14
	vmul.s16 q4, q4, q14
	vmul.s16 q5, q5, q14

	@ pair add
	vpadd.s16 d4, d4, d5
	vpadd.s16 d5, d6, d7
	vpadd.s16 d6, d8, d9
	vpadd.s16 d7, d10, d11
	
	vpadd.s16 d4, d4, d5
	vpadd.s16 d5, d6, d7

	@ store results
	add r10, r12, r9
	add r10, r10, r9
	vst1.16 {q2}, [r10]

	@--------------------------------
	@ loop control
	@--------------------------------
	add r9, r9, #8					@ x += 8
	cmp r9, r4
	blt com_if_filter_hor_4_8_x

	add r0, r0, r1					@ src += srcStride
	add r12,#64 					@ tmp += 32

	add r8, r8, #1					@ y += 1
	cmp r8, r5
	blt com_if_filter_hor_4_8_y
	
	b com_if_filter_hor_4_end

com_if_filter_hor_4_4:
	mov r8, #-3					    @ y = -3
com_if_filter_hor_4_4_y:
	sub r9, r9, r9					@ x = 0	
com_if_filter_hor_4_4_x:
	sub r10, r9, #1					@ x - 1
	add r10, r0, r10					@ &src[x - 1]
	vld1.8 {d0}, [r10]

	@ convert byte to short
	vmovl.u8 q0, d0					@ q0: p7 ... p4 p3 ... p0

	@ prepare for sequences
	vmov.i16 d4, d0
	vext.i16 d5, d0, d1, #1 			@ q2: p4 ... p1 p3 ... p0
	vext.i16 d6, d0, d1, #2 
	vext.i16 d7, d0, d1, #3 			@ q3: p6 ... p3 p5 ... p2

	@ multiply
	vmul.s16 q2, q2, q14
	vmul.s16 q3, q3, q14

	@ pair add
	vpadd.s16 d4, d4, d5
	vpadd.s16 d5, d6, d7
	
	vpadd.s16 d4, d4, d5

	@ store results
	add r10, r12, r9
	add r10, r10, r9
	vst1.64 {d4}, [r10]

	@--------------------------------
	@ loop control
	@--------------------------------
	add r9, r9, #4					@ x += 4
	cmp r9, r4
	blt com_if_filter_hor_4_4_x

	add r0, r0, r1					@ src += srcStride
	add r12,#64 					@ tmp += 32

	add r8, r8, #1					@ y += 1
	cmp r8, r5
	blt com_if_filter_hor_4_4_y
	
	b com_if_filter_hor_4_end

com_if_filter_hor_4_2:
	mov r8, #-3					    @ y = -3
com_if_filter_hor_4_2_y:
	sub r9, r9, r9					@ x = 0	
com_if_filter_hor_4_2_x:
	sub r10, r9, #1					@ x - 1
	add r10, r0, r10				@ &src[x - 1]
	vld1.8 {d0}, [r10]

	@ convert byte to short
	vmovl.u8 q0, d0					@ q0: p7 ... p4 p3 ... p0

	@ prepare for sequences
	vmov.i16 d4, d0
	vext.i16 d5, d0, d1, #1 			@ q2: p4 ... p1 p3 ... p0

	@ multiply
	vmul.s16 q2, q2, q14

	@ pair add
	vpadd.s16 d4, d4, d5
	vpadd.s16 d4, d4, d5

	@ store results
	add r10, r12, r9
	add r10, r10, r9
	vst1.32 {d4[0]}, [r10]

	@--------------------------------
	@ loop control
	@--------------------------------
	add r9, r9, #2					@ x += 4
	cmp r9, r4
	blt com_if_filter_hor_4_2_x

	add r0, r0, r1					@ src += srcStride
	add r12,#64 					@ tmp += 32

	add r8, r8, #1					@ y += 1
	cmp r8, r5
	blt com_if_filter_hor_4_2_y
	
	b com_if_filter_hor_4_end

com_if_filter_hor_4_end:

@ Ver
	
	@ align (r12)
	@ r12-->tmp
	and r12,sp,#15
	sub r12,sp,r12
	sub r12,#2240
	add r12,#64                        @ tmp = tmp_res + 1 * 32;

	@ load coefficients_v(as scalar)
	vld1.8 {d0}, [r7]
	vmovl.s8 q0,d0
	
	@ set buffers for offset (1<<IF_FILTER_PREC - 1)
	vmov.i32 q15, #0x0800

	@ branch
	and r8, r4, #7
	cmp r8, #0
	beq com_if_filter_ver_4_8

	and r8, r4, #3
	cmp r8, #0
	beq com_if_filter_ver_4_4

	b com_if_filter_ver_4_2

com_if_filter_ver_4_8:
	sub r8, r8, r8				    @ y = 0
com_if_filter_ver_4_8_y:
	sub r9, r9, r9					@ x = 0	
com_if_filter_ver_4_8_x:
	@ load a block of pixels(4x8)
	sub r10, r9, #32
	lsl r10,#1
	add r10, r12, r10					// &src[x - srcStride]
	vld1.16 {q5}, [r10]
	add r10, #64
	vld1.16 {q6}, [r10]
	add r10, #64
	vld1.16 {q7}, [r10]
	add r10, #64
	vld1.16 {q8}, [r10]	

	@ multiply and accumulate
	vmull.s16 q1, d10, d0[0]
	vmull.s16 q2, d11, d0[0]

	vmlal.s16 q1, d12, d0[1]
	vmlal.s16 q2, d13, d0[1]
	vmlal.s16 q1, d14, d0[2]
	vmlal.s16 q2, d15, d0[2]
	vmlal.s16 q1, d16, d0[3]
	vmlal.s16 q2, d17, d0[3]

	@ offset and shift
	vadd.s32 q3, q1, q15
	vadd.s32 q4, q2, q15
	vshrn.i32 d2, q3, #12
	vshrn.i32 d3, q4, #12

	@ clip(0, 255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d4, q1

	@ store results into &dst[x]
	add r10, r2, r9
	vst1.64 {d4}, [r10]

	@--------------------------------
	@ loop controls
	@--------------------------------
	add r9, r9, #8
	cmp r9, r4
	blt com_if_filter_ver_4_8_x

	add r12, r12, #64				@ tmp += 32
	add r2, r2, r3					@ dst += dstStride
	
	add r8, r8, #1
	cmp r8, r5
	blt com_if_filter_ver_4_8_y
	
	b com_if_filter_ver_4_end

com_if_filter_ver_4_4:
	sub r8, r8, r8				    @ y = 0
com_if_filter_ver_4_4_y:
	sub r9, r9, r9					@ x = 0	
com_if_filter_ver_4_4_x:
	@ load a block of pixels(4x8)
	sub r10, r9, #32
	lsl r10,#1
	add r10, r12, r10					// &src[x - srcStride]
	vld1.16 {d4}, [r10]
	add r10, #64
	vld1.16 {d5}, [r10]
	add r10, #64
	vld1.16 {d6}, [r10]
	add r10, #64
	vld1.16 {d7}, [r10]	

	@ multiply and accumulate
	vmull.s16 q1, d4, d0[0]
	vmlal.s16 q1, d5, d0[1]
	vmlal.s16 q1, d6, d0[2]
	vmlal.s16 q1, d7, d0[3]

	@ offset and shift
	vadd.s32 q2, q1, q15
	vshrn.i32 d2, q2, #12

	@ clip(0, 255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d4, q1

	@ store results into &dst[x]
	add r10, r2, r9
	vst1.32 {d4[0]}, [r10]

	@--------------------------------
	@ loop controls
	@--------------------------------
	add r9, r9, #4
	cmp r9, r4
	blt com_if_filter_ver_4_4_x

	add r12, r12, #64				@ tmp += 32
	add r2, r2, r3					@ dst += dstStride
	
	add r8, r8, #1
	cmp r8, r5
	blt com_if_filter_ver_4_4_y
	
	b com_if_filter_ver_4_end

com_if_filter_ver_4_2:
	sub r8, r8, r8				    @ y = 0
com_if_filter_ver_4_2_y:
	sub r9, r9, r9					@ x = 0	
com_if_filter_ver_4_2_x:
	@ load a block of pixels(4x8)
	sub r10, r9, #32
	lsl r10,#1
	add r10, r12, r10					// &src[x - srcStride]
	vld1.16 {d4}, [r10]
	add r10, #64
	vld1.16 {d5}, [r10]
	add r10, #64
	vld1.16 {d6}, [r10]
	add r10, #64
	vld1.16 {d7}, [r10]	

	@ multiply and accumulate
	vmull.s16 q1, d4, d0[0]
	vmlal.s16 q1, d5, d0[1]
	vmlal.s16 q1, d6, d0[2]
	vmlal.s16 q1, d7, d0[3]

	@ offset and shift
	vadd.s32 q2, q1, q15
	vshrn.i32 d2, q2, #12

	@ clip(0, 255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d4, q1

	@ store results into &dst[x]
	add r10, r2, r9
	vst1.16 {d4[0]}, [r10]

	@--------------------------------
	@ loop controls
	@--------------------------------
	add r9, r9, #2
	cmp r9, r4
	blt com_if_filter_ver_4_2_x

	add r12, r12, #64				@ tmp += 32
	add r2, r2, r3					@ dst += dstStride
	
	add r8, r8, #1
	cmp r8, r5
	blt com_if_filter_ver_4_2_y
	
	b com_if_filter_ver_4_end


com_if_filter_ver_4_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function com_if_filter_hor_ver_8_neon128
@ *src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, width-->r4, height-->r5, *coeff_h-->r6, *coeff_v-->r7, max_val-->r8

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6,r7,r8}
	sub sp, sp, #40

	@ align (r12)
	@ r12-->tmp
	and r12, sp, #15
	sub r12, sp, r12
	sub r12, #9088

	mov r8,#3
	mul r8,r1,r8
	sub r0,r0,r8

	@ load coefficients_h
	@q14: coeff_h[7] ... coeff_h[4] coeff_h[3] ... coeff_h[0]
	vld1.s8 {d28}, [r6]
	vmovl.s8 q14,d28

	@ set buffers for clip(0, 255)
	vmov.s16 q13, #255
	vmov.s16 q12, #0

@ Hor
	@ branch
	and r8, r4, #7
	cmp r8, #0
	beq com_if_filter_hor_8_8

	b com_if_filter_hor_8_4

com_if_filter_hor_8_8:
	mov r8, #-7					    @ y = -7
com_if_filter_hor_8_8_y:
	sub r9, r9, r9					@ x = 0	
com_if_filter_hor_8_8_x:
	sub r10, r9, #3					@ x - 1
	add r10, r0, r10				@ &src[x - 1]
	vld1.8 {q0}, [r10]

	@ convert byte to short
	vmovl.u8 q10, d0
	vmovl.u8 q11, d1

	@ prepare for sequences
	vmov.i16 q3, q10
	vext.i16 q4, q10, q11, #1
	vext.i16 q5, q10, q11, #2
	vext.i16 q6, q10, q11, #3
	vext.i16 q7, q10, q11, #4
	vext.i16 q8, q10, q11, #5
	vext.i16 q9, q10, q11, #6
	vext.i16 q10, q10, q11, #7

	@ multiply
	vmul.s16 q0, q3, q14
	vmul.s16 q1, q4, q14
	vmul.s16 q2, q5, q14
	vmul.s16 q3, q6, q14
	vmul.s16 q4, q7, q14
	vmul.s16 q5, q8, q14
	vmul.s16 q6, q9, q14
	vmul.s16 q7, q10, q14

	@ pair add
	vpadd.s16 d16, d0, d1
	vpadd.s16 d17, d2, d3
	vpadd.s16 d18, d4, d5
	vpadd.s16 d19, d6, d7
	vpadd.s16 d20, d8, d9
	vpadd.s16 d21, d10, d11
	vpadd.s16 d22, d12, d13
	vpadd.s16 d23, d14, d15

	@ accumulate
	vpadd.s16 d0, d16, d17
	vpadd.s16 d1, d18, d19
	vpadd.s16 d2, d20, d21
	vpadd.s16 d3, d22, d23
	vpadd.s16 d4, d0, d1
	vpadd.s16 d5, d2, d3

	@ store results
	add r10, r12, r9
	add r10, r10, r9
	vst1.16 {q2}, [r10]

	@--------------------------------
	@ loop control
	@--------------------------------
	add r9, r9, #8					@ x += 8
	cmp r9, r4
	blt com_if_filter_hor_8_8_x

	add r0, r0, r1					@ src += srcStride
	add r12,#128 					@ tmp += 64

	add r8, r8, #1					@ y += 1
	cmp r8, r5
	blt com_if_filter_hor_8_8_y
	
	b com_if_filter_hor_8_end

com_if_filter_hor_8_4:
	mov r8, #-7					    @ y = -7
com_if_filter_hor_8_4_y:
	sub r9, r9, r9					@ x = 0
com_if_filter_hor_8_4_x:
	sub r10, r9, #3					@ x - 1
	add r10, r0, r10				@ &src[x - 1]
	vld1.8 {q0}, [r10]

	@ convert byte to short
	vmovl.u8 q10, d0
	vmovl.u8 q11, d1

	@ prepare for sequences
	vmov.i16 q3, q10
	vext.i16 q4, q10, q11, #1
	vext.i16 q5, q10, q11, #2
	vext.i16 q6, q10, q11, #3

	@ multiply
	vmul.s16 q0, q3, q14
	vmul.s16 q1, q4, q14
	vmul.s16 q2, q5, q14
	vmul.s16 q3, q6, q14

	@ pair add
	vpadd.s16 d16, d0, d1
	vpadd.s16 d17, d2, d3
	vpadd.s16 d18, d4, d5
	vpadd.s16 d19, d6, d7

	@ accumulate
	vpadd.s16 d0, d16, d17
	vpadd.s16 d1, d18, d19
	vpadd.s16 d2, d0, d1

	@ store results
	add r10, r12, r9
	add r10, r10, r9
	vst1.64 {d2}, [r10]

	@--------------------------------
	@ loop control
	@--------------------------------
	add r9, r9, #4					@ x += 4
	cmp r9, r4
	blt com_if_filter_hor_8_4_x

	add r0, r0, r1					@ src += srcStride
	add r12,#128 					@ tmp += 64

	add r8, r8, #1					@ y += 1
	cmp r8, r5
	blt com_if_filter_hor_8_4_y
	
	b com_if_filter_hor_8_end

com_if_filter_hor_8_end:

@ Ver
braek:
	@ align (r12)
	@ r12-->tmp
	and r12,sp,#15
	sub r12,sp,r12
	sub r12,#9088
	add r12,#384                        @ tmp = tmp_res + 3 * 64;

	@ load coefficients(as scalar)
	vld1.8 {d0}, [r7]
	vmovl.s8 q0,d0

	@ set buffers for offset (1<<IF_FILTER_PREC - 1)
	vmov.i32 q15, #0x0800

	@ branch
	and r8, r4, #7
	cmp r8, #0
	beq com_if_filter_ver_8_8

	b com_if_filter_ver_8_4

com_if_filter_ver_8_8:
	sub r8, r8, r8				    @ y = 0
com_if_filter_ver_8_8_y:
	sub r9, r9, r9					@ x = 0	
com_if_filter_ver_8_8_x:
	sub r10, r9, #192				@ x - 3 * 64
	lsl r10,#1

	@ load a block of pixels(8x8)
	add r10, r12, r10
	vld1.16 {q4}, [r10]
	add r10, #128
	vld1.16 {q5}, [r10]
	add r10, #128
	vld1.16 {q6}, [r10]
	add r10, #128
	vld1.16 {q7}, [r10]	
	add r10, #128
	vld1.16 {q8}, [r10]
	add r10, #128
	vld1.16 {q9}, [r10]
	add r10, #128
	vld1.16 {q10}, [r10]
	add r10, #128
	vld1.16 {q11}, [r10]

	@ multiply and accumulate
	vmull.s16 q1, d8, d0[0]
	vmull.s16 q2, d9, d0[0]

	vmlal.s16 q1, d10, d0[1]
	vmlal.s16 q2, d11, d0[1]
	vmlal.s16 q1, d12, d0[2]
	vmlal.s16 q2, d13, d0[2]
	vmlal.s16 q1, d14, d0[3]
	vmlal.s16 q2, d15, d0[3]

	vmlal.s16 q1, d16, d1[0]
	vmlal.s16 q2, d17, d1[0]
	vmlal.s16 q1, d18, d1[1]
	vmlal.s16 q2, d19, d1[1]
	vmlal.s16 q1, d20, d1[2]
	vmlal.s16 q2, d21, d1[2]
	vmlal.s16 q1, d22, d1[3]
	vmlal.s16 q2, d23, d1[3]

	@ offset and shift
	vadd.s32 q3, q1, q15
	vadd.s32 q4, q2, q15
	vshrn.i32 d2, q3, #12
	vshrn.i32 d3, q4, #12

	@ clip(0, 255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d4, q1

	@ store results into &dst[x]
	add r10, r2, r9
	vst1.64 {d4}, [r10]

	@--------------------------------
	@ loop controls
	@--------------------------------
	add r9, r9, #8
	cmp r9, r4
	blt com_if_filter_ver_8_8_x

	add r12, r12, #128				@ tmp += 64
	add r2, r2, r3					@ dst += dstStride
	
	add r8, r8, #1
	cmp r8, r5
	blt com_if_filter_ver_8_8_y
	
	b com_if_filter_ver_8_end

com_if_filter_ver_8_4:
	sub r8, r8, r8				    @ y = 0
com_if_filter_ver_8_4_y:
	sub r9, r9, r9					@ x = 0	
com_if_filter_ver_8_4_x:
	sub r10, r9, #192				@ x - 3 * 64
	lsl r10,#1

	@ load a block of pixels(8x4)
	add r10, r12, r10
	vld1.16 {d6}, [r10]
	add r10, #128
	vld1.16 {d7}, [r10]
	add r10, #128
	vld1.16 {d8}, [r10]
	add r10, #128
	vld1.16 {d9}, [r10]	
	add r10, #128
	vld1.16 {d10}, [r10]
	add r10, #128
	vld1.16 {d11}, [r10]
	add r10, #128
	vld1.16 {d12}, [r10]
	add r10, #128
	vld1.16 {d13}, [r10]

	@ multiply and accumulate
	vmull.s16 q1, d6, d0[0]

	vmlal.s16 q1, d7, d0[1]
	vmlal.s16 q1, d8, d0[2]
	vmlal.s16 q1, d9, d0[3]

	vmlal.s16 q1, d10, d1[0]
	vmlal.s16 q1, d11, d1[1]
	vmlal.s16 q1, d12, d1[2]
	vmlal.s16 q1, d13, d1[3]

	@ offset and shift
	vadd.s32 q2, q1, q15
	vshrn.i32 d2, q2, #12

	@ clip(0, 255)
	vmax.s16 q2, q1, q12
	vmin.s16 q1, q2, q13

	@ convert short to byte
	vmovn.s16 d4, q1

	@ store results into &dst[x]
	add r10, r2, r9
	vst1.32 {d4[0]}, [r10]

	@--------------------------------
	@ loop controls
	@--------------------------------
	add r9, r9, #4
	cmp r9, r4
	blt com_if_filter_ver_8_4_x

	add r12, r12, #128				@ tmp += 128
	add r2, r2, r3					@ dst += dstStride
	
	add r8, r8, #1
	cmp r8, r5
	blt com_if_filter_ver_8_4_y
	
	b com_if_filter_ver_8_end

com_if_filter_ver_8_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function com_if_filter_cpy_neon128
@*src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, *width-->r4, height-->r5
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

	sub r1, r4
	sub r3, r4

	and r12, r4, #15
	cmp r12, #0
	beq com_if_filter_cpy_loop16_y

com_if_filter_cpy_loop8_y:
	mov r7, #0
com_if_filter_cpy_loop8_x:	
	vld1.32 {d0}, [r0]!
	vst1.32 {d0}, [r2]!

	add r7, #8
	cmp r7, r4
	blt com_if_filter_cpy_loop8_x

	sub r5, #1
	add r0, r1
	add r2, r3
	cmp r5, #0
	bne com_if_filter_cpy_loop8_y
	b com_if_filter_cpy_end

com_if_filter_cpy_loop16_y:
	mov r7, #0
com_if_filter_cpy_loop16_x:	
	vld1.32 {q0}, [r0]!
	vst1.32 {q0}, [r2]!

	add r7, #16
	cmp r7, r4
	blt com_if_filter_cpy_loop16_x

	sub r5, #1
	add r0, r1
	add r2, r3
	cmp r5, #0
	bne com_if_filter_cpy_loop16_y

com_if_filter_cpy_end:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function com_if_filter_cpy2_neon128
@*src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, *width-->r4, height-->r5
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

com_if_filter_cpy2_loop:
	vld1.16 d0[0], [r0], r1
	vld1.16 d0[1], [r0], r1
	vst1.16 d0[0], [r2], r3
	vst1.16 d0[1], [r2], r3

	sub r5, #2
	cmp r5, #0
	bne com_if_filter_cpy2_loop

	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function com_if_filter_cpy4_neon128
@*src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, *width-->r4, height-->r5
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

com_if_filter_cpy4_loop:
	vld1.32 d0[0], [r0], r1
	vld1.32 d0[1], [r0], r1
	vst1.32 d0[0], [r2], r3
	vst1.32 d0[1], [r2], r3

	sub r5, #2
	cmp r5, #0
	bne com_if_filter_cpy4_loop

	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function com_if_filter_cpy6_neon128
@*src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, *width-->r4, height-->r5
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40
	add r7, r0, #4
	add r8, r2, #4

com_if_filter_cpy6_loop:
	vld1.32 d0[0], [r0], r1
	vst1.32 d0[0], [r2], r3
	vld1.16 d0[0], [r7], r1
	vst1.16 d0[0], [r8], r3

	vld1.32 d0[0], [r0], r1
	vst1.32 d0[0], [r2], r3
	vld1.16 d0[0], [r7], r1
	vst1.16 d0[0], [r8], r3

	sub r5, #2
	cmp r5, #0
	bne com_if_filter_cpy6_loop

	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function com_if_filter_cpy8_neon128
@*src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, *width-->r4, height-->r5
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

com_if_filter_cpy8_loop:
	vld1.32 d0, [r0], r1
	vld1.32 d1, [r0], r1
	vst1.32 d0, [r2], r3
	vst1.32 d1, [r2], r3

	sub r5, #2
	cmp r5, #0
	bne com_if_filter_cpy8_loop

	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function com_if_filter_cpy12_neon128
@*src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, *width-->r4, height-->r5
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40
	add r7, r0, #8
	add r8, r2, #8

com_if_filter_cpy12_loop:
	vld1.32 d0, [r0], r1
	vst1.32 d0, [r2], r3
	vld1.32 d1[0], [r7], r1
	vst1.32 d1[0], [r8], r3

	vld1.32 d0, [r0], r1
	vst1.32 d0, [r2], r3
	vld1.32 d1[0], [r7], r1
	vst1.32 d1[0], [r8], r3

	sub r5, #2
	cmp r5, #0
	bne com_if_filter_cpy12_loop

	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function com_if_filter_cpy16_neon128
@*src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, *width-->r4, height-->r5
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

com_if_filter_cpy16_loop:
	vld1.32 {q0}, [r0], r1
	vld1.32 {q1}, [r0], r1
	vst1.32 {q0}, [r2], r3
	vst1.32 {q1}, [r2], r3

	sub r5, #2
	cmp r5, #0
	bne com_if_filter_cpy16_loop

	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc

function com_if_filter_cpy32_neon128
@*src-->r0, i_src-->r1, *dst-->r2, i_dst-->r3, *width-->r4, height-->r5
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5}
	sub sp, sp, #40

com_if_filter_cpy32_loop:
	vld1.32 {q0, q1}, [r0], r1
	vld1.32 {q2, q3}, [r0], r1
	vst1.32 {q0, q1}, [r2], r3
	vst1.32 {q2, q3}, [r2], r3

	sub r5, #2
	cmp r5, #0
	bne com_if_filter_cpy32_loop

	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc
