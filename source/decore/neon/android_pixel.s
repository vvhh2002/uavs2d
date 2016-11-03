.macro function name
     .global \name
\name:
     .hidden \name
     .type   \name, %function
     .func   \name
\name:
.endm

function add_pel_clip_neon128
@*src1-->r0, i_src1-->r1, *src2-->r2, i_src2-->r3, *dst-->r4, i_dst-->r5, width-->r6, height-->r7

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6, r7}
	sub sp, sp, #40

	lsl r3,r3,#1
	
	@offset
	vmov.u16 q3,#255
	vmov.u16 q4,#0
	
	and r9,r6,#7
	cmp r9,#0
	beq add_pel_clip8

add_pel_clip4:

add_pel_clip4_y:
	mov r9,r0
	mov r10,r2
	mov r12,r4


add_pel_clip4_x:
	vld1.32 {d0[0]},[r9]!
	vld1.16 {d2},[r10]!
	vmovl.u8 q0,d0
	vadd.s16 d2,d0,d2
	vmin.s16 d2,d2,d6
	vmax.s16 d2,d2,d8
	vmovn.i16 d2,q1
	vst1.32 {d2[0]},[r12]!

	add r0,r0,r1
	add r2,r2,r3
	add r4,r4,r5
	sub r7,r7,#1
	cmp r7,#0
	bne add_pel_clip4_y
	b end_add_pel_clip

add_pel_clip8:

add_pel_clip8_y:
	mov r9,r0
	mov r10,r2
	mov r8,r4
	mov r12,#0
	
add_pel_clip8_x:
	vld1.16 {d0},[r9]!
	vld1.16 {q1},[r10]!
	vmovl.u8 q0,d0
	vadd.s16 q2,q0,q1
	vmin.s16 q2,q2,q3
	vmax.s16 q2,q2,q4
	vmovn.i16 d2,q2
	vst1.16 {d2},[r8]!
	add r12,#8
	cmp r12,r6
	bne add_pel_clip8_x
	
	add r0,r0,r1
	add r2,r2,r3
	add r4,r4,r5
	sub r7,r7,#1
	cmp r7,#0
	bne add_pel_clip8_y

end_add_pel_clip:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc



function avg_pel_neon128
@*dst-->r0, i_dst-->r1, *src1-->r2, i_src1-->r3, *src2-->r4, i_src2-->r5, width-->r6, height-->r7
	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5, r6, r7}
	sub sp, sp, #40

	and r8,r6,#15
	cmp r8,#0
	beq avg_pel16

	and r8,r6,#7
	cmp r8,#0
	beq avg_pel8

	and r8,r6,#3
	cmp r8,#0
	beq avg_pel4

avg_pel2:

avg_pel2_y:
	mov r8,r0
	mov r9,r2
	mov r10,r4
	mov r12,#0

avg_pel2_x:
	vld1.16 {d0[0]},[r9]!
	vld1.16 {d1[0]},[r10]!
	vrhadd.u8 d2,d1,d0
	vst1.16 {d2[0]},[r8]!
	add r12,#2
	cmp r12,r6
	bne avg_pel2_x

	add r0,r0,r1
	add r2,r2,r3
	add r4,r4,r5
	sub r7,r7,#1
	cmp r7,#0
	bne avg_pel2_y

	b end_avg_pel


avg_pel4:

avg_pel4_y:
	mov r8,r0
	mov r9,r2
	mov r10,r4
	mov r12,#0

avg_pel4_x:
	vld1.32 {d0[0]},[r9]!
	vld1.32 {d1[0]},[r10]!
	vrhadd.u8 d2,d1,d0
	vst1.32 {d2[0]},[r8]!
	add r12,#4
	cmp r12,r6
	bne avg_pel4_x

	add r0,r0,r1
	add r2,r2,r3
	add r4,r4,r5
	sub r7,r7,#1
	cmp r7,#0
	bne avg_pel4_y

	b end_avg_pel

avg_pel8:

avg_pel8_y:
	mov r8,r0
	mov r9,r2
	mov r10,r4
	mov r12,#0

avg_pel8_x:
	vld1.8 {d0},[r9]!
	vld1.8 {d1},[r10]!
	vrhadd.u8 d2,d0,d1
	vst1.8 {d2},[r8]!
	add r12,#8
	cmp r12,r6
	bne avg_pel8_x

	add r0,r0,r1
	add r2,r2,r3
	add r4,r4,r5
	sub r7,r7,#1
	cmp r7,#0
	bne avg_pel8_y

	b end_avg_pel

avg_pel16:

avg_pel16_y:
	mov r8,r0
	mov r9,r2
	mov r10,r4
	mov r12,#0
avg_pel16_x:
	vld1.8 {q0},[r9]!
	vld1.8 {q1},[r10]!
	vrhadd.u8 q2,q0,q1
	vst1.8 {q2},[r8]!
	add r12,#16
	cmp r12,r6
	bne avg_pel16_x

	add r0,r0,r1
	add r2,r2,r3
	add r4,r4,r5
	sub r7,r7,#1
	cmp r7,#0
	bne avg_pel16_y

	b end_avg_pel

end_avg_pel:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function padding_rows_lr_neon128
@*src-->r0, i_src-->r1, width-->r2, height-->r3, start-->r4, rows-->r5,pad-->r6
@ 2*pad-->r9, p-->r7, p[-pad]-->r10, p[width]-->r8

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5,r6}
	sub sp, sp, #40

	cmp r4,#0
	movlt r4,#0

	and r7,r6,#15
	cmp r7,#0
	subne r6,r6,r7
	addne r6,#16

	add r7,r4,r5
	cmp r7,r3
	subgt r5,r3,r4

	mul r7,r4,r1
	add r7,r7,r0

padding_rows_lr_y:
	sub r10,r7,r6
	add r8,r7,r2
	ldrb r12,[r7]
	vdup.8 q0,r12
	ldrb r12,[r8,#-1]
	vdup.8 q1,r12
	mov r9,#0

padding_rows_lr_x:
	vst1.8 {q0},[r10]!
	vst1.8 {q1},[r8]!
	add r9,#16
	cmp r6,r9
	bne padding_rows_lr_x

	add r7,r7,r1
	sub r5,#1
	cmp r5,#0
	bne padding_rows_lr_y

	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc


function padding_rows_neon128
@*src-->r0, i_src-->r1, width-->r2, height-->r3, start-->r4, rows-->r5
@pad-->r6, p-->r7

	stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	add sp, sp, #40
	ldmia sp, {r4, r5,r6}
	sub sp, sp, #40

	push {r6}
	cmp r4,#0
	movlt r4,#0

	and r7,r6,#15
	cmp r7,#0
	subne r6,r6,r7
	addne r6,#16

	add r7,r4,r5
	cmp r7,r3
	subgt r5,r3,r4
	push {r5}
	mul r7,r4,r1
	add r7,r7,r0

padding_rows_y:
	sub r10,r7,r6
	add r8,r7,r2
	ldrb r12,[r7]
	vdup.8 q0,r12
	ldrb r12,[r8,#-1]
	vdup.8 q1,r12
	mov r9,#0

padding_rows_x:
	vst1.8 {q0},[r10]!
	vst1.8 {q1},[r8]!
	add r9,#16
	cmp r6,r9
	bne padding_rows_x

	add r7,r7,r1
	sub r5,#1
	cmp r5,#0
	bne padding_rows_y

memcpy1:
	pop {r6,r5}
	push {r3,r5,r6}
	mov r5,r6
	lsl r9,r6,#1
	add r7,r9,r2
	and r8,r7,#15
	cmp r8,#0
	lsrne r8,#1
	subne r6,r6,r8
	addne r6,r6,#8
	lsl r9,r6,#1

	cmp r4,#0
	bne memcpy2

	@ r7-->p,r8-->p-i_scr
	sub r7,r0,r6
	sub r8,r7,r1
	
memcpy1_y:
	@ r10--> width + 2 * pad
	add r10,r9,r2

	mov r3,r8
	mov r12,r7
	
memcpy1_x:
	vld1.8 {q0},[r12]!
	vst1.8 {q0},[r3]!
	sub r10,#16
	cmp r10,#0
	bne memcpy1_x

	sub r8,r8,r1
	sub r5,#1
	cmp r5,#0
	bne memcpy1_y

memcpy2:
	pop {r3,r5,r12}
	add r10,r4,r5
	cmp r10,r3
	bne end_padding_rows
memcpy2_t:
	sub r3,#1
	mul r10,r3,r1
	sub r7,r0,r6
	add r7,r7,r10
	add r8,r7,r1
	mov r5,r12

memcpy2_y:
	add r10,r9,r2
	mov r3,r8
	mov r12,r7

memcpy2_x:
	vld1.8 {q0},[r12]!
	vst1.8 {q0},[r3]!
	sub r10,#16
	cmp r10,#0
	bne memcpy2_x

	add r8,r8,r1
	sub r5,#1
	cmp r5,#0
	bne memcpy2_y


end_padding_rows:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	mov pc, lr
.endfunc



