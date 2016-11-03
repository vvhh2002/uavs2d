.macro function name
     .global \name
\name:
     .hidden \name
     .type   \name, %function
     .func   \name
\name:
.endm

function deblock_edge_ver_neon  @( *src, stride, alpha, beta, *flag[2])
		push {r4-r12, lr}
		add sp, sp, #40
		ldmia sp, {r4}
		sub sp, sp, #40
		vpush {q4-q7}
		
		vdup.16 q15 , r2   @alpha
		vdup.16 q14 , r3   @beta
		sub r0 , r0 , #3	@r0:src-3
		mov r2 , r0
		
		ldrb r5, [r4]				@ flag[0]
		ldrb r6, [r4, #1]			@
		cmp r5, #0					@ flag[0] ? -1 : 0;
		movne r5, #-1
		cmp r6, #0
		movne r6, #-1
		
		vdup.s32 d20, r5
		vdup.s32 d21, r6
		
		@src[-3]:src[4] --> l2, l1, l0, r0, r1, r2, r3, r4
		vld1.64 {d0} , [r0] , r1    @l2
		vld1.64 {d1} , [r0] , r1    @l1
		vld1.64 {d2} , [r0] , r1    @l0
		vld1.64 {d3} , [r0] , r1    @r0
		vld1.64 {d4} , [r0] , r1    @r1
		vld1.64 {d5} , [r0] , r1    @r2
		vld1.64 {d6} , [r0] , r1	@r3
		vld1.64 {d7} , [r0]		  	@r4
		
		@//////////transform q0.....q7
		vtrn.32         q0, q2
		vtrn.32         q1, q3
		
		vtrn.16         q0, q1
		vtrn.16         q2, q3
		
		vtrn.8         d0, d1
		vtrn.8         d2, d3
		vtrn.8         d4, d5
		vtrn.8         d6, d7
		
		vmovl.u8		q7, d7
		vmovl.u8		q6, d6
		vmovl.u8		q5, d5
		vmovl.u8		q4, d4
		vmovl.u8		q3, d3
		vmovl.u8		q2, d2
		vmovl.u8		q1, d1
		vmovl.u8		q0, d0
		
		@ABS(r0 - l0) < Alpha && abs(r0-l0) > 1
		vabd.u16 q8 , q3 , q2  
		vclt.u16 q12, q8 , q15
		vmov.i16 q13, #1
		vclt.u16 q13, q13, q8
		vand	 q13, q12, q13
		vand	 q13, q13, q10	@mask1(save)
		
		@FlatnessL
		vabd.u16 q12, q2 , q1	@abs(l0-l1) < beta
		vclt.u16 q12, q12, q14
		vmov.u16 q8 , #2
		vand	 q12, q12, q8
		
		vabd.u16 q11, q0 , q2 	@abs(l2-l0) < beta
		vclt.u16 q11, q11, q14
		vmov.u16 q9 , #1
		vand	 q11, q11, q9
		vadd.u16 q15, q12, q11	@FlatnessL++ (save)
		
		@FlatnessR
		vabd.u16 q11, q3 , q4	@abs(r0-r1) < beta
		vclt.u16 q11, q11, q14
		vand	 q11, q11, q8	@q8=2
		
		vabd.u16 q10, q3 , q5	@abs(r0-r2) < beta
		vclt.u16 q10, q10, q14
		vand	 q10, q10, q9	@q9=1
		vadd.u16 q11, q11, q10	@FlatnessR++
		
		vadd.u16 q12, q15, q11 @FLT: FlatnessL+FlatnessR(save)
		
		vceq.u16 q11, q3 , q4	@r0==r1
		vceq.u16 q10, q2 , q1	@l0==l1
		vand	 q11, q11, q10
		
		vsub.s16 q10, q12, q8	@flt-2
		vmov.u16 q9 , #3
		vsub.s16 q9 , q12, q9	@flt-3
		
		vbif	 q10, q9, q11	@fs56
		
		vmov.u16 q11, #0		@fs: init 0
		vmov.u16 q9 , #4
		vclt.u16 q9 , q9 , q12	@flt > 4
		vbit	 q11, q10, q9
		
		vceq.u16 q15, q15 , q8	@flatnessL == 2
		vmov.u16 q10, #1
		vbif	 q8 , q10, q15	@fs4
		
		vmov.u16 q9 , #4
		vceq.u16 q9 , q12, q9	@flt == 4
		vbit	 q11, q8, q9	@update fs
		
		vabd.u16 q8 , q1 , q4	@abs(l1-r1)<beta
		vclt.u16 q8 , q8 , q14
		vmov.u16 q9 , #0
		vbit	 q9 , q10, q8	@fs3
		
		vmov.u16 q8 , #3
		vceq.u16 q10, q12, q8
		vbit	 q11, q9 , q10	@update fs
		
		vand	 q12, q11, q13	@get fs (save)
		
		vmov.u16 q14, q6
		vmov.u16 q15, q7
		@fs == 1
		vadd.u16 q11, q2, q3	@L0+R0
		vshl.u16 q10, q2, #1
		vshl.u16 q9 , q3, #1
		vadd.u16 q10, q10, q11
		vadd.u16 q9 , q9 , q11
		
		@save tmp result L0 and R0 to q6 and q7
		vrshr.u16 q6 , q10, #2 @l0
		vrshr.u16 q7 , q9 , #2 @r0
		vmov.u16  q10, #1
		vceq.u16  q10, q12, q10 @fs==1
		vbif	  q6 , q2 , q10	@update L0_dst	
		vbif	  q7 , q3 , q10	@update R0_dst
		
		@fs == 2
		vshl.u16 q11, q11, #1 	@(L0 + R0)<<1
		vadd.u16 q10, q1 , q3	@L1+R0
		vadd.u16 q9 , q2 , q4	@L0+R1
		vadd.u16 q10, q11, q10
		vadd.u16 q9 , q11, q9
		vshl.u16 q8 , q1 , #1 	@L1<<1
		vshl.u16 q13, q4 , #1	@R1<<1
		vadd.u16 q10, q10, q8 
		vadd.u16 q9 , q9 , q13
		vshl.u16 q8 , q2 , #3	@L0<<3
		vshl.u16 q13, q3 , #3	@R0<<3
		vadd.u16 q10, q10, q8
		vadd.u16 q9 , q9 , q13
		vrshr.u16 q8, q10, #4
		vrshr.u16 q9, q9 , #4
		@update L0_dst and R0_dst
		vmov.u16 q10, #2
		vceq.u16 q10, q12, q10 @fs == 2
		vbit	 q6 , q8 , q10
		vbit	 q7 , q9 , q10
		
		@fs == 3
		vshl.u16 q11, q11, #1	@(L0+R0)<<2
		vadd.u16 q10, q0 , q4	@L2+R1
		vshl.u16 q9 , q2 , #1	@L0<<1
		vshl.u16 q8 , q1 , #2	@L1<<2
		vadd.u16 q10, q11, q10
		vadd.u16 q9 , q9 , q8
		vadd.u16 q8 , q10, q9 	@L0_dst_fs3
		
		vadd.u16 q10, q1 , q5	@L1+R2
		vshl.u16 q9 , q3 , #1	@R0<<1
		vshl.u16 q13, q4 , #2	@R1<<2
		vadd.u16 q10, q11, q10
		vadd.u16 q9 , q13, q9
		vadd.u16 q9 , q10, q9
		
		vrshr.u16 q8, q8 , #4
		vrshr.u16 q9, q9 , #4
		@update L0_dst and R0_dst
		vmov.u16 q10, #3
		vceq.u16 q13, q12, q10
		vbit	 q6 , q8 , q13
		vbit	 q7 , q9 , q13
		
		@calculate src[-2] L1
		vadd.u16 q8 , q0 , q3	@L2+R0
		vshl.u16 q9 , q0 , #1	@L2<<1
		vshl.u16 q10, q2 , #2	@L0<<2
		vadd.u16 q8 , q8 , q9
		vshl.u16 q9 , q1 , #3	@L1<<3
		vadd.u16 q10, q10, q9
		vadd.u16 q8 , q8 , q10
		@calculate src[1] R1
		vadd.u16 q9 , q5 , q2	@R2+L0
		vshl.u16 q10, q5 , #1	@R2<<1
		vadd.u16 q9 , q9 , q10
		vshl.u16 q10, q3 , #2	@R0<<2
		vadd.u16 q9 , q9 , q10
		vshl.u16 q10, q4 , #3	@R1<<3
		vadd.u16 q9 , q9 , q10
		
		vrshr.u16 q8, q8 , #4
		vrshr.u16 q9, q9 , #4
		@update L1_dst and R1_dst
		vbif	 q8 , q1 , q13
		vbif	 q9 , q4 , q13
		
		@fs == 4
		vmov.u16 q10, #4
		vceq.u16 q13, q12, q10
		vpadd.i32 d24, d26, d27
		vmov	 r4, r5, d24
		orr		 r4, r4, r5
		cmp		 r4, #0
		beq		 filtered
		
		@if(fs==4 exist)
		@calculate L0 and R0
		vshl.u16 q11, q11, #1	@R0+L0)<<3
		vadd.u16 q10, q0 , q2	@L0+L2
		vshl.u16 q12, q5 , #1
		vadd.u16 q10, q10, q11
		vadd.u16 q10, q12, q10
		vshl.u16 q12, q5 , #2
		vadd.u16 q10, q10, q12
		vshl.u16 q12, q0 , #3	@L2<<3
		vadd.u16 q10, q10, q12
		vrshr.u16 q10, q10, #5
		
		vadd.u16 q12, q3, q5	@R0+R2
		vadd.u16 q11, q11, q12
		vshl.u16 q12, q0, #1	@L2<<1
		vadd.u16 q11, q11, q12
		vshl.u16 q12, q0, #2	@
		vadd.u16 q11, q11, q12
		vshl.u16 q12, q5, #3	@R2<<3
		vadd.u16 q11, q11, q12
		vrshr.u16 q11, q11, #5
		
		vbit	 q6 , q10, q13
		vbit	 q7 , q11, q13
		
		@calculate L1
		vsub.s16 q11, q3 , q2	@R0-L0
		vadd.u16 q10, q3 , q0	@R0+L2
		vshl.u16 q10, q10, #1
		vshl.u16 q12, q0 , #2	@L2<<2
		vadd.s16 q10, q10, q11
		vadd.s16 q10, q10, q12
		vshl.u16 q12, q2 , #3	@L0<<3
		vadd.s16 q10, q10, q12
		vrshr.s16 q10, q10, #4
		vbit	 q8 , q10, q13
		
		@calculate R1
		vadd.u16 q10, q2 , q5	@(L0+R2)<<1
		vshl.u16 q12, q5 , #2	@R2<<2
		vshl.u16 q10, q10, #1
		vsub.s16 q12, q12, q11
		vshl.u16 q11, q3 , #3	@R0<<3
		vadd.s16 q10, q12, q10
		vadd.s16 q10, q11, q10
		vrshr.s16 q10,q10, #4
		vbit	  q9, q10, q13
		
		@calculate L2 and R2
		vadd.u16 q10, q0 , q3	@L2+R0
		vshl.u16 q11, q0 , #1	@L2<<1
		vshl.u16 q12, q2 , #2	@L0<<2
		vadd.u16 q10, q10, q11
		vadd.u16 q10, q10, q12
		
		vadd.u16 q11, q2 , q5	@L0+R2
		vshl.u16 q12, q5 , #1	@R2<<1
		vadd.u16 q11, q11, q12
		vshl.u16 q12, q3 , #2	@R0<<2
		vadd.u16 q11, q11, q12
		
		vrshr.u16 q10, q10, #3
		vrshr.u16 q11, q11, #3
	filtered:
		vbif	  q10, q0 , q13
		vbif	  q11, q5 , q13
		
		@@---order: q10, q8, q6, q7, q9, q11, q14, q15
		@@---for    L2 , L1, L0, R0, R1, R2 , R3 , R4
		
		vswp	d21, d18
		vswp	d17, d22
		vswp	d13, d28
		vswp	d15, d30
		
		vtrn.32         q10, q6
		vtrn.32         q8, q7
		vtrn.32         q9, q14
		vtrn.32         q11, q15
		vtrn.16         q10, q8
		vtrn.16         q6, q7
		vtrn.16         q9, q11
		vtrn.16         q14, q15
		
		vmovn.i16 d20, q10
		vmovn.i16 d16, q8
		vmovn.i16 d12, q6
		vmovn.i16 d14, q7
		vmovn.i16 d18, q9
		vmovn.i16 d22, q11
		vmovn.i16 d28, q14
		vmovn.i16 d30, q15
		
		vst1.32 {d20} , [r2] , r1
		vst1.32 {d16} , [r2] , r1
		vst1.32 {d12} , [r2] , r1
		vst1.32 {d14} , [r2] , r1
		vst1.32 {d18} , [r2] , r1
		vst1.32 {d22} , [r2] , r1
		vst1.32 {d28} , [r2] , r1
		vst1.32 {d30} , [r2] 
		vpop {q4-q7}
		ldmia  sp! , {r4, r5 , r6 , r7 , r8 , r9 , r10 , r11 , r12 , pc}	
.endfunc

function deblock_edge_hor_neon  @( *src, stride, alpha, beta, *flt_flag)
		push {r4-r12, lr}
		add sp, sp, #40
		ldmia sp, {r4}
		sub sp, sp, #40
		vpush {q4-q7}

		vdup.16 q15 , r2   			@alpha
		vdup.16 q14 , r3   			@beta
		sub r2 , r0 , r1 , lsl #1	@r2:src-2*stride 
		sub r0 , r2 , r1 			@r0:src-3*stride
		
		ldrb r5, [r4]				@ flt_flag[0]
		ldrb r6, [r4, #1]			@
		cmp r5, #0					@ flag[0] ? -1 : 0;
		movne r5, #-1
		cmp r6, #0
		movne r6, #-1
		
		vdup.s32 d12, r5
		vdup.s32 d13, r6
		
		@src[-3]:src[3] --> l2, l1, l0, r0, r1, r2, r3
		vld1.64 {d0} , [r0] , r1    @l2
		vld1.64 {d2} , [r0] , r1    @l1
		vld1.64 {d4} , [r0] , r1    @l0
		vld1.64 {d6} , [r0] , r1    @r0
		vld1.64 {d8} , [r0] , r1    @r1
		vld1.64 {d10} , [r0] , r1   @r2
		
		vmovl.u8 q0, d0
		vmovl.u8 q1, d2
		vmovl.u8 q2, d4
		vmovl.u8 q3, d6
		vmovl.u8 q4, d8
		vmovl.u8 q5, d10
		
		@ABS(r0 - l0) < Alpha && abs(r0-l0) > 1
		vabd.u16 q8 , q3 , q2  
		vclt.u16 q12, q8 , q15
		vmov.i16 q13, #1
		vclt.u16 q13, q13, q8
		vand	 q13, q12, q13 
		vand	 q13, q13, q6	@mask1(save)
		
		@FlatnessL
		vabd.u16 q12, q2 , q1	@abs(l0-l1) < beta
		vclt.u16 q12, q12, q14
		vmov.u16 q8 , #2
		vand	 q12, q12, q8
		
		vabd.u16 q11, q0 , q2 	@abs(l2-l0) < beta
		vclt.u16 q11, q11, q14
		vmov.u16 q9 , #1
		vand	 q11, q11, q9
		vadd.u16 q6, q12, q11	@FlatnessL++ (save)
		
		@FlatnessR
		vabd.u16 q11, q3 , q4	@abs(r0-r1) < beta
		vclt.u16 q11, q11, q14
		vand	 q11, q11, q8	@q8=2
		
		vabd.u16 q10, q3 , q5	@abs(r0-r2) < beta
		vclt.u16 q10, q10, q14
		vand	 q10, q10, q9	@q9=1
		vadd.u16 q11, q11, q10	@FlatnessR++
		
		vadd.u16 q12, q6, q11 @FLT: FlatnessL+FlatnessR(save)
		
		vceq.u16 q11, q3 , q4	@r0==r1
		vceq.u16 q10, q2 , q1	@l0==l1
		vand	 q11, q11, q10
		
		vsub.s16 q10, q12, q8	@flt-2
		vmov.u16 q9 , #3
		vsub.s16 q9 , q12, q9	@flt-3
		
		vbif	 q10, q9, q11	@fs56
		
		vmov.u16 q11, #0		@fs: init 0
		vmov.u16 q9 , #4
		vclt.u16 q7 , q9 , q12	@flt > 4
		vbit	 q11, q10, q7
		
		vceq.u16 q6 , q6 , q8	@flatnessL == 2
		vmov.u16 q7 , #1
		vbif	 q8 , q7 , q6	@fs4
		
		vceq.u16 q9 , q12, q9	@flt == 4
		vbit	 q11, q8, q9	@update fs
		
		vabd.u16 q10, q1 , q4	@abs(l1-r1)<beta
		vclt.u16 q10, q10, q14
		vmov.u16 q9 , #0
		vbit	 q9 , q7 , q10	@fs3
		
		vmov.u16 q8 , #3
		vceq.u16 q10, q12, q8
		vbit	 q11, q9 , q10	@update fs
		
		vand	 q12, q11, q13	@get fs (save)
		
		@fs == 1
		vadd.u16 q11, q2, q3	@L0+R0
		vshl.u16 q10, q2, #1
		vshl.u16 q9 , q3, #1
		vadd.u16 q10, q10, q11
		vadd.u16 q9 , q9 , q11
		
		@save tmp result L0 and R0 to q6 and q7
		vrshr.u16 q6 , q10, #2 @l0
		vrshr.u16 q7 , q9 , #2 @r0
		vmov.u16  q10, #1
		vceq.u16  q10, q12, q10 @fs==1
		vbif	  q6 , q2 , q10	@update L0_dst	
		vbif	  q7 , q3 , q10	@update R0_dst
		
		@fs == 2
		vshl.u16 q11, q11, #1 	@(L0 + R0)<<1
		vadd.u16 q10, q1 , q3	@L1+R0
		vadd.u16 q9 , q2 , q4	@L0+R1
		vadd.u16 q10, q11, q10
		vadd.u16 q9 , q11, q9
		vshl.u16 q15, q1 , #1 	@L1<<1
		vshl.u16 q14, q4 , #1	@R1<<1
		vadd.u16 q10, q10, q15 
		vadd.u16 q9 , q9 , q14
		vshl.u16 q15, q2 , #3	@L0<<3
		vshl.u16 q14, q3 , #3	@R0<<3
		vadd.u16 q10, q10, q15
		vadd.u16 q9 , q9 , q14
		vrshr.u16 q8, q10, #4
		vrshr.u16 q9, q9 , #4
		@update L0_dst and R0_dst
		vmov.u16 q10, #2
		vceq.u16 q10, q12, q10 @fs == 2
		vbit	 q6 , q8 , q10
		vbit	 q7 , q9 , q10
		
		@fs == 3
		vshl.u16 q11, q11, #1	@(L0+R0)<<2
		vadd.u16 q10, q0 , q4	@L2+R1
		vshl.u16 q9 , q2 , #1	@L0<<1
		vshl.u16 q8 , q1 , #2	@L1<<2
		vadd.u16 q10, q11, q10
		vadd.u16 q9 , q9 , q8
		vadd.u16 q8 , q10, q9 	@L0_dst_fs3
		
		vadd.u16 q10, q1 , q5	@L1+R2
		vshl.u16 q15, q3 , #1	@R0<<1
		vshl.u16 q14, q4 , #2	@R1<<2
		vadd.u16 q10, q11, q10
		vadd.u16 q14, q14, q15
		vadd.u16 q9 , q10, q14
		
		vrshr.u16 q8, q8 , #4
		vrshr.u16 q9, q9 , #4
		@update L0_dst and R0_dst
		vmov.u16 q10, #3
		vceq.u16 q14, q12, q10
		vbit	 q6 , q8 , q14
		vbit	 q7 , q9 , q14
		
		@calculate src[-2] L1
		vadd.u16 q8 , q0 , q3	@L2+R0
		vshl.u16 q9 , q0 , #1	@L2<<1
		vshl.u16 q10, q2 , #2	@L0<<2
		vshl.u16 q15, q1 , #3	@L1<<3
		vadd.u16 q8 , q8 , q9
		vadd.u16 q10, q10, q15
		vadd.u16 q8 , q8 , q10
		@calculate src[1] R1
		vadd.u16 q9 , q5 , q2	@R2+L0
		vshl.u16 q10, q5 , #1	@R2<<1
		vshl.u16 q15, q3 , #2	@R0<<2
		vadd.u16 q9 , q9 , q10
		vshl.u16 q10, q4 , #3	@R1<<3
		vadd.u16 q9 , q9 , q15
		vadd.u16 q9 , q9 , q10
		
		vrshr.u16 q8, q8 , #4
		vrshr.u16 q9, q9 , #4
		@update L1_dst and R1_dst
		vbif	 q8 , q1 , q14
		vbif	 q9 , q4 , q14
		
		@fs == 4
		vmov.u16 q10, #4
		vceq.u16 q15, q12, q10
		vpadd.i32 d24, d30, d31
		vmov	 r4, r5, d24
		orr		 r4, r4, r5
		cmp		 r4, #0
		beq		 filtered_hor
		
		@if(fs==4 exist)
		@calculate L0 and R0
		vshl.u16 q11, q11, #1	@(R0+L0)<<3
		vadd.u16 q10, q0 , q2	@L0+L2
		vshl.u16 q12, q5 , #1
		vshl.u16 q13, q5 , #2
		vshl.u16 q14, q0 , #3	@L2<<3
		vadd.u16 q10, q10, q11
		vadd.u16 q12, q12, q13
		vadd.u16 q10, q10, q14
		vadd.u16 q10, q10, q12
		vrshr.u16 q10, q10, #5
		
		vadd.u16 q12, q3, q5	@R0+R2
		vshl.u16 q13, q0, #1	@L2<<1
		vshl.u16 q14, q0, #2	@
		vadd.u16 q11, q11, q12
		vadd.u16 q13, q13, q14
		vshl.u16 q14, q5, #3	@R2<<3
		vadd.u16 q11, q11, q13
		vadd.u16 q11, q11, q14
		vrshr.u16 q11, q11, #5
		
		vbit	 q6 , q10, q15
		vbit	 q7 , q11, q15
		
		@calculate L1
		vsub.s16 q11, q3 , q2	@R0-L0
		vadd.u16 q10, q3 , q0	@R0+L2
		vshl.u16 q10, q10, #1
		vshl.u16 q12, q0 , #2	@L2<<2
		vshl.u16 q13, q2 , #3	@L0<<3
		vadd.s16 q10, q10, q11
		vadd.s16 q12, q12, q13
		vadd.s16 q10, q10, q12
		vrshr.s16 q10, q10, #4
		vbit	 q8 , q10, q15
		
		@calculate R1
		vadd.u16 q10, q2 , q5	@(L0+R2)<<1
		vshl.u16 q12, q5 , #2	@R2<<2
		vshl.u16 q10, q10, #1
		vshl.u16 q13, q3 , #3	@R0<<3
		vsub.s16 q10, q10, q11
		vadd.s16 q12, q12, q13
		vadd.s16 q10, q10, q12
		vrshr.s16 q10,q10, #4
		vbit	  q9, q10, q15
		
		@calculate L2 and R2
		vadd.u16 q10, q0 , q3	@L2+R0
		vshl.u16 q11, q0 , #1	@L2<<1
		vshl.u16 q12, q2 , #2	@L0<<2
		vadd.u16 q10, q10, q11
		vadd.u16 q10, q10, q12
		
		vadd.u16 q11, q2 , q5	@L0+R2
		vshl.u16 q12, q5 , #1	@R2<<1
		vshl.u16 q13, q3 , #2	@R0<<2
		vadd.u16 q11, q11, q12
		vadd.u16 q11, q11, q13
		
		vrshr.u16 q10, q10, #3
		vrshr.u16 q11, q11, #3
		
		vbif	  q10, q0 , q15
		vbif	  q11, q5 , q15
		
		@store L0,L1,L2,R0,R1,R2
		sub		r2, r2, r1
		
		vmovn.i16 d20, q10
		vmovn.i16 d16, q8
		vmovn.i16 d12, q6
		vmovn.i16 d14, q7
		vmovn.i16 d18, q9
		vmovn.i16 d22, q11
		
		vst1.32 {d20} , [r2] , r1
		vst1.32 {d16} , [r2] , r1
		vst1.32 {d12} , [r2] , r1
		vst1.32 {d14} , [r2] , r1
		vst1.32 {d18} , [r2] , r1
		vst1.32 {d22} , [r2] 
		b		end_hor
filtered_hor:
		@store L0,L1,R0,R1
		vmovn.i16 d16, q8
		vmovn.i16 d12, q6
		vmovn.i16 d14, q7
		vmovn.i16 d18, q9
		vst1.32 {d16} , [r2] , r1
		vst1.32 {d12} , [r2] , r1
		vst1.32 {d14} , [r2] , r1
		vst1.32 {d18} , [r2] 

end_hor:		
		vpop {q4-q7}
		ldmia  sp! , {r4,r5 , r6 , r7 , r8 , r9 , r10 , r11 , r12 , pc}	
.endfunc


function deblock_edge_ver_c_neon  @( *srcU, *srcV, stride, alpha, beta, *flag[2])
		push {r4-r9, lr}
		add sp, sp, #28
		ldmia sp, {r4, r5}
		sub sp, sp, #28
		
		vdup.16 q15 , r3	@alpha
		vdup.16 q14 , r4	@beta
		
		ldrb r8, [r5]				@ flag[0]
		ldrb r9, [r5, #1]			@
		cmp r8, #0					@ flag[0] ? -1 : 0;
		movne r8, #-1
		cmp r9, #0
		movne r9, #-1
		
		vmov	d20, r8, r9
		vmov	d21, d20
		
		sub		r0, #3
		sub		r1, #3
		mov		r3, r0
		mov		r4, r1
		@src[-3]:src[4] --> l2, l1, l0, r0, r1, r2, r3, r4
		vld1.64 {d0} , [r0] , r2    
		vld1.64 {d1} , [r0] , r2    
		vld1.64 {d2} , [r0] , r2    
		vld1.64 {d3} , [r0]    
		vld1.64 {d4} , [r1] , r2    
		vld1.64 {d5} , [r1] , r2    
		vld1.64 {d6} , [r1] , r2	
		vld1.64 {d7} , [r1]		  	

		@//////////transform q0.....q7
		vtrn.32         q0, q2
		vtrn.32         q1, q3
		
		vtrn.16         q0, q1
		vtrn.16         q2, q3
		
		vtrn.8         d0, d1
		vtrn.8         d2, d3
		vtrn.8         d4, d5
		vtrn.8         d6, d7
		
		vmovl.u8		q7, d7	   @r4
		vmovl.u8		q6, d6	   @r3
		vmovl.u8		q5, d5	   @r2
		vmovl.u8		q4, d4	   @r1
		vmovl.u8		q3, d3	   @r0
		vmovl.u8		q2, d2	   @l0
		vmovl.u8		q1, d1	   @l1
		vmovl.u8		q0, d0	   @l2
		
		@ABS(r0 - l0) < Alpha && abs(r0-l0) > 1
		vabd.u16 q8 , q3 , q2  
		vclt.u16 q12, q8 , q15
		vmov.i16 q13, #1
		vclt.u16 q13, q13, q8
		vand	 q13, q12, q13
		vand	 q13, q13, q10	@mask1(save)
		
		@FlatnessL
		vabd.u16 q12, q2 , q1	@abs(l0-l1) < beta
		vclt.u16 q12, q12, q14
		vmov.u16 q8 , #2
		vand	 q12, q12, q8
		
		vabd.u16 q11, q0 , q2 	@abs(l2-l0) < beta
		vclt.u16 q11, q11, q14
		vmov.u16 q9 , #1
		vand	 q11, q11, q9
		vadd.u16 q15, q12, q11	@FlatnessL++ (save)
		
		@FlatnessR
		vabd.u16 q11, q3 , q4	@abs(r0-r1) < beta
		vclt.u16 q11, q11, q14
		vand	 q11, q11, q8	@q8=2
		
		vabd.u16 q10, q3 , q5	@abs(r0-r2) < beta
		vclt.u16 q10, q10, q14
		vand	 q10, q10, q9	@q9=1
		vadd.u16 q11, q11, q10	@FlatnessR++
		
		vadd.u16 q12, q15, q11 @FLT: FlatnessL+FlatnessR(save)
		
		vceq.u16 q11, q3 , q4	@r0==r1
		vceq.u16 q10, q2 , q1	@l0==l1
		vand	 q11, q11, q10
		
		vmov.u16 q9 , #3
		vsub.s16 q10, q12, q9	@flt-3
		vmov.u16 q9 , #4
		vsub.s16 q8 , q12, q9	@flt-4
		
		vbif	 q10, q8, q11	@fs56
		
		vmov.u16 q11, #0		@fs: init 0
		vclt.u16 q8 , q9 , q12	@flt > 4
		vbit	 q11, q10, q8
		
		vmov.u16 q8 , #2
		vceq.u16 q15, q15, q8	@flatnessL == 2
		vmov.u16 q8 , #1
		vmov.u16 q10, #0
		vbif	 q8 , q10, q15	@fs4
		
		vceq.u16 q9 , q12, q9	@flt == 4
		vbit	 q11, q8, q9	@update fs
		
		vand	 q12, q11, q13	@get fs (save)
		
		vmov.u16 q14, q6
		vmov.u16 q15, q7
		@fs == 1
		vadd.u16 q11, q2, q3	@L0+R0
		vshl.u16 q10, q2, #1
		vshl.u16 q9 , q3, #1
		vadd.u16 q10, q10, q11
		vadd.u16 q9 , q9 , q11
		
		@save tmp result L0 and R0 to q6 and q7
		vrshr.u16 q6 , q10, #2 @l0
		vrshr.u16 q7 , q9 , #2 @r0
		vmov.u16  q10, #1
		vceq.u16  q10, q12, q10 @fs==1
		vbif	  q6 , q2 , q10	@update L0_dst	
		vbif	  q7 , q3 , q10	@update R0_dst
		
		@fs == 2
		vshl.u16 q11, q11, #1 	@(L0 + R0)<<1
		vadd.u16 q10, q1 , q3	@L1+R0
		vadd.u16 q9 , q2 , q4	@L0+R1
		vadd.u16 q10, q11, q10
		vadd.u16 q9 , q11, q9
		vshl.u16 q8 , q1 , #1 	@L1<<1
		vshl.u16 q13, q4 , #1	@R1<<1
		vadd.u16 q10, q10, q8 
		vadd.u16 q9 , q9 , q13
		vshl.u16 q8 , q2 , #3	@L0<<3
		vshl.u16 q13, q3 , #3	@R0<<3
		vadd.u16 q10, q10, q8
		vadd.u16 q9 , q9 , q13
		vrshr.u16 q8, q10, #4
		vrshr.u16 q9, q9 , #4
		@update L0_dst and R0_dst
		vmov.u16 q10, #2
		vceq.u16 q10, q12, q10 @fs == 2
		vbit	 q6 , q8 , q10
		vbit	 q7 , q9 , q10
		
		@fs == 3
		vshl.u16 q11, q11, #1	@(L0+R0)<<2
		vadd.u16 q10, q0 , q4	@L2+R1
		vshl.u16 q9 , q2 , #1	@L0<<1
		vshl.u16 q8 , q1 , #2	@L1<<2
		vadd.u16 q10, q11, q10
		vadd.u16 q9 , q9 , q8
		vadd.u16 q8 , q10, q9 	@L0_dst_fs3
		
		vadd.u16 q10, q1 , q5	@L1+R2
		vshl.u16 q9 , q3 , #1	@R0<<1
		vshl.u16 q13, q4 , #2	@R1<<2
		vadd.u16 q10, q11, q10
		vadd.u16 q9 , q13, q9
		vadd.u16 q9 , q10, q9
		
		vrshr.u16 q8, q8 , #4
		vrshr.u16 q9, q9 , #4
		@update L0_dst and R0_dst
		vmov.u16 q10, #3
		vceq.u16 q13, q12, q10
		vbit	 q6 , q8 , q13
		vbit	 q7 , q9 , q13
		
		@calculate src[-2] L1
		vadd.u16 q8 , q0 , q3	@L2+R0
		vshl.u16 q9 , q0 , #1	@L2<<1
		vshl.u16 q10, q2 , #2	@L0<<2
		vadd.u16 q8 , q8 , q9
		vshl.u16 q9 , q1 , #3	@L1<<3
		vadd.u16 q10, q10, q9
		vadd.u16 q8 , q8 , q10
		@calculate src[1] R1
		vadd.u16 q9 , q5 , q2	@R2+L0
		vshl.u16 q10, q5 , #1	@R2<<1
		vadd.u16 q9 , q9 , q10
		vshl.u16 q10, q3 , #2	@R0<<2
		vadd.u16 q9 , q9 , q10
		vshl.u16 q10, q4 , #3	@R1<<3
		vadd.u16 q9 , q9 , q10
		
		vrshr.u16 q8, q8 , #4
		vrshr.u16 q9, q9 , #4
		@update L1_dst and R1_dst
		vbif	 q8 , q1 , q13
		vbif	 q9 , q4 , q13
		
		@@---order: q0, q8, q6, q7, q9, q5, q14, q15
		@@---for    L2 , L1, L0, R0, R1, R2 , R3 , R4
		
		vswp	d1 , d18
		vswp	d17, d10
		vswp	d13, d28
		vswp	d15, d30
		
		vtrn.32         q0, q6
		vtrn.32         q8, q7
		vtrn.32         q9, q14
		vtrn.32         q5, q15
		vtrn.16         q0, q8
		vtrn.16         q6, q7
		vtrn.16         q9, q5
		vtrn.16         q14, q15
		
		vmovn.i16 d0, q0
		vmovn.i16 d1, q8
		vmovn.i16 d2, q6
		vmovn.i16 d3, q7
		vmovn.i16 d4, q9
		vmovn.i16 d5, q5
		vmovn.i16 d6, q14
		vmovn.i16 d7, q15
		
		vst1.64 {d0} , [r3] , r2    
		vst1.64 {d1} , [r3] , r2    
		vst1.64 {d2} , [r3] , r2    
		vst1.64 {d3} , [r3]    
		vst1.64 {d4} , [r4] , r2    
		vst1.64 {d5} , [r4] , r2    
		vst1.64 {d6} , [r4] , r2	
		vst1.64 {d7} , [r4]		  	

		ldmia  sp! , {r4, r5 , r6 , r7 , r8 , r9 , pc}	
.endfunc

function deblock_edge_hor_c_neon  @( *srcU, *srcV, stride, alpha, beta, *flt_flag)
		push {r4-r10, lr}
		add sp, sp, #32
		ldmia sp, {r4, r5}
		sub sp, sp, #32
		
		vdup.16 q15, r3   			@alpha
		vdup.16 q14, r4   			@beta

		lsl		r8, r2, #1			
		add		r9, r8, r2			@ 3*stride

		ldrb r6, [r5]				@ flt_flag[0]
		ldrb r7, [r5, #1]			@
		cmp r6, #0					@ flag[0] ? -1 : 0;
		movne r6, #-1
		cmp r7, #0
		movne r7, #-1
		
		vmov	d12, r6, r7
		vmov	d13, d12
		
		@src[-3]:src[3] --> l2, l1, l0, r0, r1, r2, r3
		ldr		r6 , [r0, -r9]
		ldr		r7 , [r1, -r9]
		vmov	d0 , r6, r7			@- L2
		ldr		r6 , [r0, -r8]
		ldr		r7 , [r1, -r8]
		vmov	d2 , r6, r7			@- L1
		ldr		r6 , [r0, -r2]
		ldr		r7 , [r1, -r2]
		vmov	d4 , r6, r7			@- L0
		ldr		r6 , [r0]
		ldr		r7 , [r1]
		vmov	d6 , r6, r7			@- R0
		ldr		r6 , [r0, r2]
		ldr		r7 , [r1, r2]
		vmov	d8 , r6, r7			@- R1
		ldr		r6 , [r0, r8]
		ldr		r7 , [r1, r8]
		vmov	d10, r6, r7			@- R2

		vmovl.u8 q0, d0
		vmovl.u8 q1, d2
		vmovl.u8 q2, d4
		vmovl.u8 q3, d6
		vmovl.u8 q4, d8
		vmovl.u8 q5, d10
		
		@ABS(r0 - l0) < Alpha && abs(r0-l0) > 1
		vabd.u16 q8 , q3 , q2  
		vclt.u16 q12, q8 , q15
		vmov.i16 q13, #1
		vclt.u16 q13, q13, q8
		vand	 q13, q12, q13 
		vand	 q13, q13, q6	@mask1(save)
		
		@FlatnessL
		vabd.u16 q12, q2 , q1	@abs(l0-l1) < beta
		vclt.u16 q12, q12, q14
		vmov.u16 q8 , #2
		vand	 q12, q12, q8
		
		vabd.u16 q11, q0 , q2 	@abs(l2-l0) < beta
		vclt.u16 q11, q11, q14
		vmov.u16 q9 , #1
		vand	 q11, q11, q9
		vadd.u16 q6, q12, q11	@FlatnessL++ (save)
		
		@FlatnessR
		vabd.u16 q11, q3 , q4	@abs(r0-r1) < beta
		vclt.u16 q11, q11, q14
		vand	 q11, q11, q8	@q8=2
		
		vabd.u16 q10, q3 , q5	@abs(r0-r2) < beta
		vclt.u16 q10, q10, q14
		vand	 q10, q10, q9	@q9=1
		vadd.u16 q11, q11, q10	@FlatnessR++
		
		vadd.u16 q12, q6, q11 @FLT: FlatnessL+FlatnessR(save)
		
		vceq.u16 q11, q3 , q4	@r0==r1
		vceq.u16 q10, q2 , q1	@l0==l1
		vand	 q11, q11, q10
		
		vmov.u16 q8 , #3
		vsub.s16 q10, q12, q8	@flt-3
		vmov.u16 q9 , #4
		vsub.s16 q7 , q12, q9	@flt-4
		
		vbif	 q10, q7, q11	@fs56
		
		vmov.u16 q11, #0		@fs: init 0
		vclt.u16 q7 , q9 , q12	@flt > 4
		vbit	 q11, q10, q7
		
		vmov.u16 q8 , #2
		vceq.u16 q6 , q6 , q8	@flatnessL == 2
		vmov.u16 q7 , #0
		vmov.u16 q8 , #1
		vbif	 q8 , q7 , q6	@fs4
		
		vceq.u16 q9 , q12, q9	@flt == 4
		vbit	 q11, q8, q9	@update fs
		
		vand	 q12, q11, q13	@get fs (save)
		
		@fs == 1
		vadd.u16 q11, q2, q3	@L0+R0
		vshl.u16 q10, q2, #1
		vshl.u16 q9 , q3, #1
		vadd.u16 q10, q10, q11
		vadd.u16 q9 , q9 , q11
		
		@save tmp result L0 and R0 to q6 and q7
		vrshr.u16 q6 , q10, #2 @l0
		vrshr.u16 q7 , q9 , #2 @r0
		vmov.u16  q10, #1
		vceq.u16  q10, q12, q10 @fs==1
		vbif	  q6 , q2 , q10	@update L0_dst	
		vbif	  q7 , q3 , q10	@update R0_dst
		
		@fs == 2
		vshl.u16 q11, q11, #1 	@(L0 + R0)<<1
		vadd.u16 q10, q1 , q3	@L1+R0
		vadd.u16 q9 , q2 , q4	@L0+R1
		vadd.u16 q10, q11, q10
		vadd.u16 q9 , q11, q9
		vshl.u16 q15, q1 , #1 	@L1<<1
		vshl.u16 q14, q4 , #1	@R1<<1
		vadd.u16 q10, q10, q15 
		vadd.u16 q9 , q9 , q14
		vshl.u16 q15, q2 , #3	@L0<<3
		vshl.u16 q14, q3 , #3	@R0<<3
		vadd.u16 q10, q10, q15
		vadd.u16 q9 , q9 , q14
		vrshr.u16 q8, q10, #4
		vrshr.u16 q9, q9 , #4
		@update L0_dst and R0_dst
		vmov.u16 q10, #2
		vceq.u16 q10, q12, q10 @fs == 2
		vbit	 q6 , q8 , q10
		vbit	 q7 , q9 , q10
		
		@fs == 3
		vshl.u16 q11, q11, #1	@(L0+R0)<<2
		vadd.u16 q10, q0 , q4	@L2+R1
		vshl.u16 q9 , q2 , #1	@L0<<1
		vshl.u16 q8 , q1 , #2	@L1<<2
		vadd.u16 q10, q11, q10
		vadd.u16 q9 , q9 , q8
		vadd.u16 q8 , q10, q9 	@L0_dst_fs3
		
		vadd.u16 q10, q1 , q5	@L1+R2
		vshl.u16 q15, q3 , #1	@R0<<1
		vshl.u16 q14, q4 , #2	@R1<<2
		vadd.u16 q10, q11, q10
		vadd.u16 q14, q14, q15
		vadd.u16 q9 , q10, q14
		
		vrshr.u16 q8, q8 , #4
		vrshr.u16 q9, q9 , #4
		@update L0_dst and R0_dst
		vmov.u16 q10, #3
		vceq.u16 q14, q12, q10
		vbit	 q6 , q8 , q14
		vbit	 q7 , q9 , q14
		
		@calculate src[-2] L1
		vadd.u16 q8 , q0 , q3	@L2+R0
		vshl.u16 q9 , q0 , #1	@L2<<1
		vshl.u16 q10, q2 , #2	@L0<<2
		vshl.u16 q15, q1 , #3	@L1<<3
		vadd.u16 q8 , q8 , q9
		vadd.u16 q10, q10, q15
		vadd.u16 q8 , q8 , q10
		@calculate src[1] R1
		vadd.u16 q9 , q5 , q2	@R2+L0
		vshl.u16 q10, q5 , #1	@R2<<1
		vshl.u16 q15, q3 , #2	@R0<<2
		vadd.u16 q9 , q9 , q10
		vshl.u16 q10, q4 , #3	@R1<<3
		vadd.u16 q9 , q9 , q15
		vadd.u16 q9 , q9 , q10
		
		vrshr.u16 q8, q8 , #4
		vrshr.u16 q9, q9 , #4
		@update L1_dst and R1_dst
		vbif	 q8 , q1 , q14
		vbif	 q9 , q4 , q14
		
		@store L1,L0,R0,R1
		vmovn.i16 d0, q8
		vmovn.i16 d2, q6
		vmovn.i16 d4, q7
		vmovn.i16 d6, q9

		vmov	r6 , r7, d0
		str		r6 , [r0, -r8]
		str		r7 , [r1, -r8]
		vmov	r9 , r10, d2
		str		r9 , [r0, -r2]
		str		r10, [r1, -r2]
		vmov	r6 , r7, d4
		str		r6 , [r0]
		str		r7 , [r1]
		vmov	r9 , r10, d6
		str		r9 , [r0, r2]
		str		r10, [r1, r2]
	
		ldmia  sp! , {r4, r5, r6, r7, r8, r9, r10, pc}	
.endfunc