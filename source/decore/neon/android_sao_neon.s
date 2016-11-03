.macro function name
     .global \name
\name:
     .hidden \name
     .type   \name, %function
     .func   \name
\name:
.endm

function SAO_EO_0_neon @--(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, int start_x, int end_x, int mb_height, char* mask)
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6, r7, r8}
	sub sp, sp, #40
	
	sub 	r10, r6 , r5
	mov 	r9 , #15
	and 	r9 , r9 , r10
	sub		r12, r9 , #1
	add		r12, r8 , r12, lsl#4
	vld1.32	{q15}, [r12]			@-- load mask: mask+(((end_x - start_x) & 0x0f) - 1)*16
	
	sub 	r10, r6 , r9			@-- r10 = end_x_16 = end_x - ((end_x - start_x) & 0x0f)
	
	@------- set offset table: q0 -----------
	vld1.32  {q10}, [r4]			@-- load offset[0-3]
	ldr		 r9 , [r4, #16]			@-- load offset4
	mov		 r8, #0
	vmov	 d4, r9, r8
	vmov.u32 d5, #0
	vmovn.s32 d0, q10
	vmovn.s32 d1, q2
	vmovn.s16 d0, q0				@-- convert int32 to byte
	
	vmov.u8	q1 , #2					@-- constant(save)
	
	mov 	r8 , #0					@-- y = 0
loop_y_eo_0:
	mov 	r9, r5					@-- x = start_x
loop_x_eo_0:
	add		r12, r0 , r9
	vld1.8	{q11}, [r12]				@-- load src[x]	(save)
	sub 	r12, r12, #1
	vld1.8	{q10}, [r12]				@-- load src[x-1]
	
	vmin.u8 q12, q10, q11
	vceq.u8 q13, q12, q10
	vceq.u8 q14, q12, q11
	vsub.s8 q10, q14, q13			@-- leftsign
	
	add 	r12, r12 , #2
	vld1.8 {q12}, [r12]				@-- load src[x+1]
	
	vmin.u8 q13, q11, q12
	vceq.u8	q14, q13, q11
	vceq.u8 q13, q13, q12
	vsub.s8 q12, q14, q13			@-- rightsign
	
	vadd.s8	q10, q12, q10			@-- edgetype
	
	vadd.s8 q10, q10, q1			@-- generate look-up indexs
	vtbl.8	d24, {d0} , d20
	vtbl.8  d25, {d0} , d21			@-- get offset
	
	vmovl.s8 q13, d24
	vmovl.s8 q14, d25
	vmovl.u8 q10, d22				@-- src[x] low 8 samples
	vmovl.u8 q12, d23
	
	vadd.s16 q13, q13, q10
	vadd.s16 q14, q14, q12
	
	vqmovun.s16 d20, q13
	vqmovun.s16 d21, q14			@-- results
	
	add		r12, r1, r9				@-- dst+x
	cmp		r9, r10
	beq		maskmove_eo_0
	vst1.8	{q10}, [r12]
	add 	r9, #16
	cmp		r9, r6
	blt		loop_x_eo_0
	b		loop_x_eo_0_end

maskmove_eo_0:
	@--- maskmove
	vld1.8	{q12}, [r12]			@-- load 16 pixels from dst+x 
	vbif	q10, q12, q15
	vst1.8	{q10}, [r12]

loop_x_eo_0_end:	
	add		r0, r0, r2				@-- src+=src_stride
	add		r1, r1, r3				@-- dst+=dst_stride
	
	add 	r8, #1					@-- y++
	cmp 	r8, r7					@-- y == mb_height?
	bne 	loop_y_eo_0
	
	ldmia 	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov 	pc, lr
.endfunc

function SAO_EO_90_neon @--(pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, int start_y, int end_y, int mb_width)
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6, r7}
	sub sp, sp, #40
	
	mul		r8 , r2 , r5
	mul		r9 , r3 , r5
	add		r0 , r0 , r8			@-- src -= start_y*src_stride
	add		r1 , r1 , r9			@-- dst -= start_y*dst_stride
	
	sub 	r10, r7 , #15			@-- r10 = end_x_16 = mb_width - 15
	
	@------- set offset table: q0 -----------
	vld1.32  {q10}, [r4]			@-- load offset[0-3]
	ldr		 r9 , [r4, #16]			@-- load offset4
	mov		 r8, #0
	vmov	 d4, r9, r8
	vmov.u32 d5, #0
	vmovn.s32 d0, q10
	vmovn.s32 d1, q2
	vmovn.s16 d0, q0				@-- convert int32 to byte
	
	vmov.u8	q1 , #2					@-- constant(save)
	
	mov		r8 , r5					@-- y = start_y
	
	mov		r12, #0
	mov		r9 , #-1
	cmp		r7 , #4
	beq		set_mask_width_4
	vmov.i8 d30, #255
	vmov	d31, r9, r12			@-- mask="-1 repeat 12, 0, 0, 0, 0"
	b		loop_y_eo_90
set_mask_width_4:
	vmov	d30, r9, r12
	vmov.i8 d31, #0					@-- mask="-1, -1, -1, -1, 0 repeat 12"

loop_y_eo_90:
	mov 	r9, #0					@-- x = 0
loop_x_eo_90:
	add		r12, r0 , r9
	vld1.8	{q11}, [r12]			@-- load src[x]	(save)
	sub 	r12, r12, r2
	vld1.8	{q10}, [r12]			@-- load src[x-src_stride]
	
	vmin.u8 q12, q10, q11
	vceq.u8 q13, q12, q10
	vceq.u8 q14, q12, q11
	vsub.s8 q10, q14, q13			@-- leftsign
	
	add 	r12, r12, r2, lsl #1
	vld1.8 {q12}, [r12]				@-- load src[x+src_stride]
	
	vmin.u8 q13, q11, q12
	vceq.u8	q14, q13, q11
	vceq.u8 q13, q13, q12
	vsub.s8 q12, q14, q13			@-- rightsign
	
	vadd.s8	q10, q12, q10			@-- edgetype
	
	vadd.s8 q10, q10, q1			@-- generate look-up indexs
	vtbl.8	d24, {d0} , d20
	vtbl.8  d25, {d0} , d21			@-- get offset
	
	vmovl.s8 q13, d24
	vmovl.s8 q14, d25
	vmovl.u8 q10, d22				@-- src[x] low 8 samples
	vmovl.u8 q12, d23
	
	vadd.s16 q13, q13, q10
	vadd.s16 q14, q14, q12
	
	vqmovun.s16 d20, q13
	vqmovun.s16 d21, q14			@-- results
	
	add		r12, r1, r9				@-- dst+x
	cmp		r9, r10
	bge		maskmove_eo_90
	vst1.8	{q10}, [r12]
	add 	r9, #16
	cmp		r9, r7
	blt		loop_x_eo_90
	b		loop_x_eo_90_end
maskmove_eo_90:
	@--- maskmove
	vld1.8	{q12}, [r12]			@-- load 16 pixels from dst+x 
	vbif	q10, q12, q15
	vst1.8	{q10}, [r12]

loop_x_eo_90_end:	
	add		r0, r0, r2				@-- src+=src_stride
	add		r1, r1, r3				@-- dst+=dst_stride
	
	add 	r8, #1					@-- y++
	cmp 	r8, r6					@-- y == end_y?
	bne 	loop_y_eo_90
	
	ldmia 	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov 	pc, lr
.endfunc

function SAO_EO_135_neon 
@--(pel_t* src, pel_t* dst, int src_stride, int dst_stride, 
@-- int* offset, char_t* mask, int mb_height, int start_x_r0, int end_x_r0, int start_x_r, int end_x_r, int start_x_rn, int end_x_rn)
@-- start_x_r0 for row0 that is first row
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp!, {r4, r5, r6, r7, r8} 	@--r4=offset; r5=mask; r6=mb_height; r7=start_x_r0; r8=end_x_r0
	
	sub		r9, r8, r7
	and		r9, r9, #15
	sub		r10, r8, r9				@-- r10:end_x_r0_16 = end_x_r0 - ((end_x_r0 - start_x_r0) & 0x0f);
	
	@------- set offset table: q0 -----------
	vld1.32  {q10}, [r4]			@-- load offset[0-3]
	ldr		 r9 , [r4, #16]			@-- load offset4
	mov		 r12, #0
	vmov	 d4, r9, r12
	vmov.u32 d5, #0
	vmovn.s32 d0, q10
	vmovn.s32 d1, q2
	vmovn.s16 d0, q0				@-- convert int32 to byte
	
	vmov.u8	q1 , #2					@-- constant(save)
	
@---------------------first row-------------------------
	mov 	r9 , r7					@-- x = start_x_r0
loop_x_eo_135_r0:
	cmp		r9 , r8
	bge		loop_x_eo_135_end_r0
	add		r12, r0 , r9
	vld1.8	{q11}, [r12]			@-- load src[x]	(save)
	sub 	r12, r12, r2
	sub		r12, r12, #1
	vld1.8	{q10}, [r12]			@-- load src[x-src_stride-1]
	
	vmin.u8 q12, q10, q11
	vceq.u8 q13, q12, q10
	vceq.u8 q14, q12, q11
	vsub.s8 q10, q14, q13			@-- leftsign
	
	add 	r12, r12, r2, lsl #1
	add		r12, r12, #2
	vld1.8 {q12}, [r12]				@-- load src[x+src_stride+1]
	
	vmin.u8 q13, q11, q12
	vceq.u8	q14, q13, q11
	vceq.u8 q13, q13, q12
	vsub.s8 q12, q14, q13			@-- rightsign
	
	vadd.s8	q10, q12, q10			@-- edgetype
	
	vadd.s8 q10, q10, q1			@-- generate look-up indexs
	vtbl.8	d24, {d0} , d20
	vtbl.8  d25, {d0} , d21			@-- get offset
	
	vmovl.s8 q13, d24
	vmovl.s8 q14, d25
	vmovl.u8 q10, d22				@-- src[x] low 8 samples
	vmovl.u8 q12, d23
	
	vadd.s16 q13, q13, q10
	vadd.s16 q14, q14, q12
	
	vqmovun.s16 d20, q13
	vqmovun.s16 d21, q14			@-- results
	
	add		r12, r1, r9				@-- dst+x
	cmp		r9, r10
	bge		maskmove_eo_135_r0
	vst1.8	{q10}, [r12]
	add 	r9, #16
	b		loop_x_eo_135_r0
maskmove_eo_135_r0:
	sub		r7, r8 , r10
	sub 	r7, r7, #1
	add		r7, r5 , r7, lsl #4		@-- offset = 16*rownum
	vld1.32 {q15}, [r7]				@-- load mask_r0
	vld1.8	{q12}, [r12]			@-- load 16 pixels from dst+x 
	vbif	q10, q12, q15
	vst1.8	{q10}, [r12]

loop_x_eo_135_end_r0:	
	add		r0, r0, r2				@-- src+=src_stride
	add		r1, r1, r3				@-- dst+=dst_stride

@--------------------------------middle rows--------------------------------
	ldmia	sp!, {r7, r8}			@-- r7=start_x_r; r8=end_x_r
	sub		r9 , r8, r7
	and		r9 , r9, #15
	sub		r12, r9, #1
	add		r12, r5, r12, lsl #4
	vld1.32 {q15}, [r12]			@-- mask_r
	
	sub		r10, r8, r9				@-- r10=end_x_r_16

	mov		r11, #1					@-- y = 1
loop_y_eo_135_r:
	mov 	r9, r7					@-- x = start_x_r
loop_x_eo_135_r:
	add		r12, r0 , r9
	vld1.8	{q11}, [r12]			@-- load src[x]	(save)
	sub 	r12, r12, r2
	sub		r12, r12, #1
	vld1.8	{q10}, [r12]			@-- load src[x-src_stride-1]
	
	vmin.u8 q12, q10, q11
	vceq.u8 q13, q12, q10
	vceq.u8 q14, q12, q11
	vsub.s8 q10, q14, q13			@-- leftsign
	
	add 	r12, r12, r2, lsl #1
	add		r12, r12, #2
	vld1.8 {q12}, [r12]				@-- load src[x+src_stride+1]
	
	vmin.u8 q13, q11, q12
	vceq.u8	q14, q13, q11
	vceq.u8 q13, q13, q12
	vsub.s8 q12, q14, q13			@-- rightsign
	
	vadd.s8	q10, q12, q10			@-- edgetype
	
	vadd.s8 q10, q10, q1			@-- generate look-up indexs
	vtbl.8	d24, {d0} , d20
	vtbl.8  d25, {d0} , d21			@-- get offset
	
	vmovl.s8 q13, d24
	vmovl.s8 q14, d25
	vmovl.u8 q10, d22				@-- src[x] low 8 samples
	vmovl.u8 q12, d23
	
	vadd.s16 q13, q13, q10
	vadd.s16 q14, q14, q12
	
	vqmovun.s16 d20, q13
	vqmovun.s16 d21, q14			@-- results
	
	add		r12, r1, r9				@-- dst+x
	cmp		r9, r10
	bge		maskmove_eo_135_r
	vst1.8	{q10}, [r12]
	add 	r9, #16
	cmp		r9, r8
	blt		loop_x_eo_135_r
	b		loop_x_eo_135_end_r
maskmove_eo_135_r:
	@--- maskmove
	vld1.8	{q12}, [r12]			@-- load 16 pixels from dst+x 
	vbif	q10, q12, q15
	vst1.8	{q10}, [r12]

loop_x_eo_135_end_r:	
	add		r0, r0, r2				@-- src+=src_stride
	add		r1, r1, r3				@-- dst+=dst_stride
	
	add 	r11, #1					@-- y++
	sub		r12, r6, #1			
	cmp 	r11, r12				@-- y == mb_height - 1?
	bne 	loop_y_eo_135_r
	
@---------------------------------last row--------------------------------
	ldmia	sp , {r6, r7}			@-- r6=start_x_rn; r7=end_x_rn;  don't update sp 
	sub		r8 , r7, r6
	and		r8 , r8, #15
	sub		r10, r7, r8				@-- r10=end_x_rn_16
	
	mov 	r9 , r6					@-- x = start_x_rn
loop_x_eo_135_rn:
	cmp		r9 , r7
	bge		loop_x_eo_135_end_rn
	add		r12, r0 , r9
	vld1.8	{q11}, [r12]			@-- load src[x]	(save)
	sub 	r12, r12, r2
	sub		r12, r12, #1
	vld1.8	{q10}, [r12]			@-- load src[x-src_stride-1]
	
	vmin.u8 q12, q10, q11
	vceq.u8 q13, q12, q10
	vceq.u8 q14, q12, q11
	vsub.s8 q10, q14, q13			@-- leftsign
	
	add 	r12, r12, r2, lsl #1
	add		r12, r12, #2
	vld1.8 {q12}, [r12]				@-- load src[x+src_stride+1]
	
	vmin.u8 q13, q11, q12
	vceq.u8	q14, q13, q11
	vceq.u8 q13, q13, q12
	vsub.s8 q12, q14, q13			@-- rightsign
	
	vadd.s8	q10, q12, q10			@-- edgetype
	
	vadd.s8 q10, q10, q1			@-- generate look-up indexs
	vtbl.8	d24, {d0} , d20
	vtbl.8  d25, {d0} , d21			@-- get offset
	
	vmovl.s8 q13, d24
	vmovl.s8 q14, d25
	vmovl.u8 q10, d22				@-- src[x] low 8 samples
	vmovl.u8 q12, d23
	
	vadd.s16 q13, q13, q10
	vadd.s16 q14, q14, q12
	
	vqmovun.s16 d20, q13
	vqmovun.s16 d21, q14			@-- results
	
	add		r12, r1, r9				@-- dst+x
	cmp		r9, r10
	bge		maskmove_eo_135_rn
	vst1.8	{q10}, [r12]
	add 	r9, #16
	b		loop_x_eo_135_rn
maskmove_eo_135_rn:
	sub		r6, r7 , r10
	sub 	r6, r6, #1
	add		r6, r5 , r6, lsl #4		@-- offset = 16*rownum
	vld1.32 {q15}, [r6]				@-- load mask_rn
	vld1.8	{q12}, [r12]			@-- load 16 pixels from dst+x 
	vbif	q10, q12, q15
	vst1.8	{q10}, [r12]
loop_x_eo_135_end_rn:	
	sub 	sp, sp, #68				@-- reserve stack point 
	ldmia 	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov 	pc, lr
.endfunc

function SAO_EO_45_neon 
@--(pel_t* src, pel_t* dst, int src_stride, int dst_stride, 
@-- int* offset, char_t* mask, int mb_height, int start_x_r0, int end_x_r0, int start_x_r, int end_x_r, int start_x_rn, int end_x_rn)
@-- start_x_r0 for row0 that is first row
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp!, {r4, r5, r6, r7, r8} 	@--r4=offset; r5=mask; r6=mb_height; r7=start_x_r0; r8=end_x_r0
	
	sub		r9, r8, r7
	and		r9, r9, #15
	sub		r10, r8, r9				@-- r10:end_x_r0_16 = end_x_r0 - ((end_x_r0 - start_x_r0) & 0x0f);
	
	@------- set offset table: q0 -----------
	vld1.32  {q10}, [r4]			@-- load offset[0-3]
	ldr		 r9 , [r4, #16]			@-- load offset4
	mov		 r12, #0
	vmov	 d4, r9, r12
	vmov.u32 d5, #0
	vmovn.s32 d0, q10
	vmovn.s32 d1, q2
	vmovn.s16 d0, q0				@-- convert int32 to byte
	
	vmov.u8	q1 , #2					@-- constant(save)
	
@---------------------first row-------------------------
	mov 	r9 , r7					@-- x = start_x_r0
loop_x_eo_45_r0:
	cmp		r9 , r8
	bge		loop_x_eo_45_end_r0
	add		r12, r0 , r9
	vld1.8	{q11}, [r12]			@-- load src[x]	(save)
	sub 	r12, r12, r2
	add		r12, r12, #1
	vld1.8	{q10}, [r12]			@-- load src[x-src_stride+1]
	
	vmin.u8 q12, q10, q11
	vceq.u8 q13, q12, q10
	vceq.u8 q14, q12, q11
	vsub.s8 q10, q14, q13			@-- leftsign
	
	add 	r12, r12, r2, lsl #1
	sub		r12, r12, #2
	vld1.8 {q12}, [r12]				@-- load src[x+src_stride-1]
	
	vmin.u8 q13, q11, q12
	vceq.u8	q14, q13, q11
	vceq.u8 q13, q13, q12
	vsub.s8 q12, q14, q13			@-- rightsign
	
	vadd.s8	q10, q12, q10			@-- edgetype
	
	vadd.s8 q10, q10, q1			@-- generate look-up indexs
	vtbl.8	d24, {d0} , d20
	vtbl.8  d25, {d0} , d21			@-- get offset
	
	vmovl.s8 q13, d24
	vmovl.s8 q14, d25
	vmovl.u8 q10, d22				@-- src[x] low 8 samples
	vmovl.u8 q12, d23
	
	vadd.s16 q13, q13, q10
	vadd.s16 q14, q14, q12
	
	vqmovun.s16 d20, q13
	vqmovun.s16 d21, q14			@-- results
	
	add		r12, r1, r9				@-- dst+x
	cmp		r9, r10
	bge		maskmove_eo_45_r0
	vst1.8	{q10}, [r12]
	add 	r9, #16
	b		loop_x_eo_45_r0
maskmove_eo_45_r0:
	sub		r7, r8 , r10
	sub 	r7, r7, #1
	add		r7, r5 , r7, lsl #4		@-- offset = 16*rownum
	vld1.32 {q15}, [r7]				@-- load mask_r0
	vld1.8	{q12}, [r12]			@-- load 16 pixels from dst+x 
	vbif	q10, q12, q15
	vst1.8	{q10}, [r12]

loop_x_eo_45_end_r0:	
	add		r0, r0, r2				@-- src+=src_stride
	add		r1, r1, r3				@-- dst+=dst_stride

@--------------------------------middle rows--------------------------------
	ldmia	sp!, {r7, r8}			@-- r7=start_x_r; r8=end_x_r
	sub		r9 , r8, r7
	and		r9 , r9, #15
	sub		r12, r9, #1
	add		r12, r5, r12, lsl #4
	vld1.32 {q15}, [r12]			@-- mask_r
	
	sub		r10, r8, r9				@-- r10=end_x_r_16

	mov		r11, #1					@-- y = 1
loop_y_eo_45_r:
	mov 	r9, r7					@-- x = start_x_r
loop_x_eo_45_r:
	add		r12, r0 , r9
	vld1.8	{q11}, [r12]			@-- load src[x]	(save)
	sub 	r12, r12, r2
	add		r12, r12, #1
	vld1.8	{q10}, [r12]			@-- load src[x-src_stride+1]
	
	vmin.u8 q12, q10, q11
	vceq.u8 q13, q12, q10
	vceq.u8 q14, q12, q11
	vsub.s8 q10, q14, q13			@-- leftsign
	
	add 	r12, r12, r2, lsl #1
	sub		r12, r12, #2
	vld1.8 {q12}, [r12]				@-- load src[x+src_stride-1]
	
	vmin.u8 q13, q11, q12
	vceq.u8	q14, q13, q11
	vceq.u8 q13, q13, q12
	vsub.s8 q12, q14, q13			@-- rightsign
	
	vadd.s8	q10, q12, q10			@-- edgetype
	
	vadd.s8 q10, q10, q1			@-- generate look-up indexs
	vtbl.8	d24, {d0} , d20
	vtbl.8  d25, {d0} , d21			@-- get offset
	
	vmovl.s8 q13, d24
	vmovl.s8 q14, d25
	vmovl.u8 q10, d22				@-- src[x] low 8 samples
	vmovl.u8 q12, d23
	
	vadd.s16 q13, q13, q10
	vadd.s16 q14, q14, q12
	
	vqmovun.s16 d20, q13
	vqmovun.s16 d21, q14			@-- results
	
	add		r12, r1, r9				@-- dst+x
	cmp		r9, r10
	bge		maskmove_eo_45_r
	vst1.8	{q10}, [r12]
	add 	r9, #16
	cmp		r9, r8
	blt		loop_x_eo_45_r
	b		loop_x_eo_45_end_r
maskmove_eo_45_r:
	@--- maskmove
	vld1.8	{q12}, [r12]			@-- load 16 pixels from dst+x 
	vbif	q10, q12, q15
	vst1.8	{q10}, [r12]

loop_x_eo_45_end_r:	
	add		r0, r0, r2				@-- src+=src_stride
	add		r1, r1, r3				@-- dst+=dst_stride
	
	add 	r11, #1					@-- y++
	sub		r12, r6, #1			
	cmp 	r11, r12				@-- y == mb_height - 1?
	bne 	loop_y_eo_45_r
	
@---------------------------------last row--------------------------------
	ldmia	sp , {r6, r7}			@-- r6=start_x_rn; r7=end_x_rn;  don't update sp 
	sub		r8 , r7, r6
	and		r8 , r8, #15
	sub		r10, r7, r8				@-- r10=end_x_rn_16
	
	mov 	r9 , r6					@-- x = start_x_rn
loop_x_eo_45_rn:
	cmp		r9 , r7
	bge		loop_x_eo_45_end_rn
	add		r12, r0 , r9
	vld1.8	{q11}, [r12]			@-- load src[x]	(save)
	sub 	r12, r12, r2
	add		r12, r12, #1
	vld1.8	{q10}, [r12]			@-- load src[x-src_stride+1]
	
	vmin.u8 q12, q10, q11
	vceq.u8 q13, q12, q10
	vceq.u8 q14, q12, q11
	vsub.s8 q10, q14, q13			@-- leftsign
	
	add 	r12, r12, r2, lsl #1
	sub		r12, r12, #2
	vld1.8 {q12}, [r12]				@-- load src[x+src_stride-1]
	
	vmin.u8 q13, q11, q12
	vceq.u8	q14, q13, q11
	vceq.u8 q13, q13, q12
	vsub.s8 q12, q14, q13			@-- rightsign
	
	vadd.s8	q10, q12, q10			@-- edgetype
	
	vadd.s8 q10, q10, q1			@-- generate look-up indexs
	vtbl.8	d24, {d0} , d20
	vtbl.8  d25, {d0} , d21			@-- get offset
	
	vmovl.s8 q13, d24
	vmovl.s8 q14, d25
	vmovl.u8 q10, d22				@-- src[x] low 8 samples
	vmovl.u8 q12, d23
	
	vadd.s16 q13, q13, q10
	vadd.s16 q14, q14, q12
	
	vqmovun.s16 d20, q13
	vqmovun.s16 d21, q14			@-- results
	
	add		r12, r1, r9				@-- dst+x
	cmp		r9, r10
	bge		maskmove_eo_45_rn
	vst1.8	{q10}, [r12]
	add 	r9, #16
	b		loop_x_eo_45_rn
maskmove_eo_45_rn:
	sub		r6, r7 , r10
	sub 	r6, r6, #1
	add		r6, r5 , r6, lsl #4		@-- offset = 16*rownum
	vld1.32 {q15}, [r6]				@-- load mask_rn
	vld1.8	{q12}, [r12]			@-- load 16 pixels from dst+x 
	vbif	q10, q12, q15
	vst1.8	{q10}, [r12]
loop_x_eo_45_end_rn:	
	sub 	sp, sp, #68				@-- reserve stack point 
	ldmia 	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov 	pc, lr
.endfunc

@---- (pel_t* src, pel_t* dst, int src_stride, int dst_stride, int* offset, int start_band, int start_band_2, int mb_width, int mb_height)
function SAO_BO_neon
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6, r7, r8} 		@-- r4=offset; r5=start_band; r6=start_band_2; r7=mb_width; r8=mb_height
	sub sp, sp, #40
	
	vmov.u8 q5, #1
	vdup.8	q0, r5					@-- constant: start_band
	vadd.u8 q1, q0, q5
	vmov.u8 q4, #31
	vand	q1, q1, q4				@-- q1: (start_band+1)%32
	vdup.8	q2, r6
	vadd.u8 q3, q2, q5
	vand	q3, q3, q4				@-- q3: (start_band_2 + 1)%32

	vld1.32	{q8}, [r4]

	vmov	r9, r10, d16
	vdup.8	q4, r9					@-- offset[0]
	vdup.8	q5, r10					@-- offset[1]

	vmov	r9, r10, d17
	vdup.8	q6, r9					@-- offset[2]
	vdup.8	q7, r10					@-- offset[3]

	mov  	r6 , #0					@-- y = 0
	sub		r10, r7 , #15			@-- r10 = mb_width - 15
	mov		r12, #0
	mov		r9 , #-1
	cmp		r7 , #4
	beq		set_mask_width_4_bo
	vmov.i8 d30, #255
	vmov	d31, r9, r12			@-- mask="-1 repeat 12, 0, 0, 0, 0"
	b		loop_y_bo
set_mask_width_4_bo:
	vmov	d30, r9, r12
	vmov.i8 d31, #0					@-- mask="-1, -1, -1, -1, 0 repeat 12"

loop_y_bo:
	mov 	r9, #0					@-- x = 0
loop_x_bo:
	add		r12, r0 , r9
	vld1.8	{q13}, [r12]			@-- load src[x]	(save)
	vshr.u8	q8 , q13, #3
	
	vceq.i8	q9 , q8, q0
	vceq.i8 q10, q8, q1
	vceq.i8 q11, q8, q2
	vceq.i8 q12, q8, q3

	vand	q9 , q9 , q4
	vand	q10, q10, q5
	vand	q11, q11, q6
	vand	q12, q12, q7

	vorr	q9 , q9 , q10
	vorr	q11, q11, q12
	vorr	q9 , q9 , q11			@-- get offsets

	vmovl.s8 q10, d18
	vmovl.s8 q11, d19
	vmovl.u8 q12, d26				@-- src[x] low 8 samples
	vmovl.u8 q13, d27
	
	vadd.s16 q10, q12, q10
	vadd.s16 q11, q11, q13
	
	vqmovun.s16 d20, q10
	vqmovun.s16 d21, q11			@-- results
	
	add		r12, r1, r9				@-- dst+x
	cmp		r9, r10
	bge		maskmove_bo
	vst1.8	{q10}, [r12]
	add 	r9, #16
	cmp		r9, r7
	blt		loop_x_bo
	b		loop_x_bo_end
maskmove_bo:
	@--- maskmove
	vld1.8	{q12}, [r12]			@-- load 16 pixels from dst+x 
	vbif	q10, q12, q15
	vst1.8	{q10}, [r12]

loop_x_bo_end:	
	add		r0, r0, r2				@-- src+=src_stride
	add		r1, r1, r3				@-- dst+=dst_stride
	
	add 	r6, #1					@-- y++
	cmp 	r6, r8					@-- y == mb_height?
	bne 	loop_y_bo

	ldmia 	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}
.endfunc