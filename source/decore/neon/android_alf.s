.macro function name
     .global \name
\name:
     .hidden \name
     .type   \name, %function
     .func   \name
\name:
.endm


function alf_flt_one_block_neon128
    stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12,lr}
    add sp, sp, #40
    ldmia sp,{r4, r5, r6, r7, r8, r9, r10, r11}
    sub sp, sp, #40

starPos:
    cmp r10,#0
    beq isAbove
    sub r10,r4,#4
    vmov.32 d0[0],r10
	b endPos

isAbove:
    vmov.32 d0[0],r4   //startPos:d0[0]
    
endPos:
    add r10,r4,r5
    cmp r11,#0
    beq isBelow
    sub r10,r10,#4
    vmov.32 d0[1],r10
	b xPosEnd


isBelow:
    vmov.32 d0[1],r10   //endPos:d0[1]

xPosEnd:
    add r10,r6,r7  
	vmov.32 d1[0],r10  //xPosEnd:d1[0]
 
	vmov.s32 r3,d0[0]
    mul r4,r3,r2
    add r1,r1,r4   //*******
    add r0,r0,r4

Loop1:
    //i:r3
	vmov.s32 r4,d0[1]
    cmp r3,r4
    beq xnd_filter
    sub r4,r4,#1  //endPos-1:r4
	vdup.32 d22,r4
    sub r5,r3,#1  //i-1:r5
    add r7,r3,#1  //i+1:r7
	vdup.32 d23,r5
	vdup.32 d24,r7
    vmov.s32 r10,d0[0]    //r10:startPos
	vdup.32 d25,r10
    vmax.s32 d23,d23,d25
    vmin.s32 d23,d23,d22   //yUp:r5
    vmax.s32 d24,d24,d25
    vmin.s32 d24,d24,d22   //yBottom:r7
	vmov.s32 r5,d23[0]
	vmov.s32 r7,d24[0]
    sub r7,r7,r3
    mul r7,r7,r2
    add r7,r7,r1   //imgPad1:r7
    sub r5,r5,r3
    mul r5,r5,r2
    add r5,r5,r1   //imgPad2:r5
    vmov.u32 d26[0],r7
    vmov.u32 d26[1],r5

    sub r5,r3,#2  //i-2:r5
    add r7,r3,#2  //i+2:r7
	vdup.32 d23,r5
	vdup.32 d24,r7
    vmax.s32 d23,d23,d25
    vmin.s32 d23,d23,d22   //yUp:r5
    vmax.s32 d24,d24,d25
    vmin.s32 d24,d24,d22   //yBottom:r7
	vmov.s32 r5,d23[0]
	vmov.s32 r7,d24[0]
    sub r7,r7,r3
    mul r7,r7,r2
    add r7,r7,r1   //imgPad3:r7
    sub r5,r5,r3
    mul r5,r5,r2
    ADD r5,r5,r1   //imgPad4:r5
    vmov.u32 d27[0],r7
    vmov.u32 d27[1],r5

    sub r5,r3,#3  //i-3:r6
    add r7,r3,#3  //i+3:r7
    vdup.32 d23,r5
	vdup.32 d24,r7
    vmax.s32 d23,d23,d25
    vmin.s32 d23,d23,d22   //yUp:r5
    vmax.s32 d24,d24,d25
    vmin.s32 d24,d24,d22   //yBottom:r7
	vmov.s32 r5,d23[0]
	vmov.s32 r7,d24[0]
    sub r7,r7,r3
    mul r7,r7,r2
    add r7,r7,r1   //imgPad5:r7
    sub r5,r5,r3
    mul r5,r5,r2
    add r5,r5,r1   //imgPad6:r5
    vmov.u32 d28[0],r7
    vmov.u32 d28[1],r5
	mov r4,r6  //r4:j  r6:xPos

Loop2:
    vmov.s32 r5,d1[0]  //r5:xPosEnd
    cmp r4,r5
    beq Exit

    ldr r5,[r8],#4  //r7:coef[0]
    vdup.16 q1,r5
    vmov.u32 r5,d28[1]
    add r5,r5,r4
    vld1.64 d4,[r5]  //imgPad6[j]
	vmovl.u8 q2,d4
    vmul.s16 q2,q2,q1

    vmov.u32 r5,d28[0]
    add r5,r5,r4
    vld1.64 d6,[r5]  //imgPad5[j]
    vmovl.u8 q3,d6
    vmul.s16 q3,q3,q1
    vadd.s16 q1,q3,q2   //Q1:(imgPad5[j]+imgPad6[j])*coef[0]

    ldr r5,[r8],#4  //r5:coef[1]
    vdup.16 q2,r5
    vmov.u32 r5,d27[1]
    add r5,r5,r4
    vld1.64 d6,[r5]  //imgPad4[j]
	vmovl.u8 q3,d6
    vmul.s16 q3,q2,q3

    vmov.u32 r5,d27[0]
    add r5,r5,r4
    vld1.64 d8,[r5]  //imgPad3[j]
	vmovl.u8 q4,d8
    vmul.s16 q4,q4,q2
    VADD.s16 q2,q3,q4   //Q2:(imgPad3[j]+imgPad4[j])*coef[1]

    ldr r5,[r8],#4  //r7:coef[2]
    vdup.16 q3,r5
    sub r7,r4,#1
    vmov.u32 r5,d26[1]  //r5:imgPad2
    add r7,r5,r7
    vld1.64 d8,[r7]  //imgPad2[j-1]
	vmovl.u8 q4,d8
    vmul.s16 q3,q3,q4  //Q3:
    
    add r7,r5,r4
    VLD1.64 d8,[r7]  //imgPad2[j]
	vmovl.u8 q4,d8
    ldr r7,[r8],#4  //r7:coef[3]
    VDUP.16 q5,r7
    vmul.s16 q4,q4,q5  //Q4:

    add r7,r4,#1
    add r7,r5,r7
    vld1.64 d10,[r7]  //imgPad2[j+1]
	vmovl.u8 q5,d10
    ldr r7,[r8]  //r7:coef[4]
    vdup.16 q6,r7
    vmul.s16 q5,q5,q6  //Q5 :

    SUB r7,r4,#1
    vmov.u32 r5,d26[0]  //r5:imgPad1
    add r7,r5,r7
    vld1.64 d14,[r7]  //imgPad1[j-1]
    vmovl.u8 q7,d14
    vmul.s16 q7,q7,q6  
    vadd.s16 q5,q5,q7  //Q5:imgPad*coef[4]

    add r7,r5,r4
    vld1.64 d12,[r7]  //imgPad1[j]
    vmovl.u8 q6,d12
    sub r8,r8,#4
    ldr r7,[r8]  //r7:coef[3]
    vdup.16 q7,r7
    vmul.s16 q6,q6,q7  
    VADD.s16 q4,q6,q4  //Q4:imgPad*coef[3]

    add r7,r4,#1
    add r7,r5,r7
    VLD1.64 d12,[r7]  //imgPad1[j+1]
    vmovl.u8 q6,d12
    SUB r8,r8,#4
    ldr r7,[r8]  //r7:coef[2]
    VDUP.16 q7,r7
    vmul.s16 q6,q7,q6  //Q5:
    VADD.s16 q3,q6,q3  //Q3:imgPad*coef[2]

    sub r7,r4,#3
    ADD r7,r1,r7   
    VLD1.64 d12,[r7]   //imgPad[j-3]
    vmovl.u8 q6,d12
    add r8,r8,#12
    ldr r5,[r8],#4  //r5:coef[5]
    vdup.16 q7,r5
    vmul.s16 q6,q6,q7

    add r7,r4,#3
    add r7,r1,r7   
    VLD1.64 d16,[r7]   //imgPad[j+3]
    vmovl.u8 q8,d16
    vmul.s16 q7,q7,q8
    VADD.s16 q6,q7,q6  //Q6:imgPad*coef[5]

    sub r5,r4,#2
    add r5,r1,r5   
    vld1.64 d14,[r5]   //imgPad[j-2]
    vmovl.u8 q7,d14
    ldr r5,[r8],#4  //r5:coef[6]
    vdup.16 q8,r5
    vmul.s16 q7,q7,q8

    add r5,r4,#2
    add r5,r1,r5    
    vld1.64 d18,[r5]   //imgPad[j+2]
    vmovl.u8 q9,d18
    vmul.s16 q8,q9,q8
    vadd.s16 q7,q7,q8 //Q7:imgPad*coef[6]

    sub r5,r4,#1
    add r5,r1,r5  
    VLD1.64 d16,[r5]   //imgPad[j-1]
    vmovl.u8 q8,d16
    ldr r5,[r8],#4   //r5:coef[7]
    VDUP.16 q9,r5
    VMUL.s16 q8,q8,q9

    add r5,r4,#1
    add r5,r1,r5  
    vld1.64 d20,[r5]   //imgPad[j+1]
    vmovl.u8 q10,d20
    vmul.s16 q9,q10,q9
    vadd.s16 q8,q9,q8  //Q8:imgPad*coef[7]

    add r5,r1,r4
    vld1.64 d18,[r5]   //imgPad[j]
    vmovl.u8 q9,d18
    ldr r5,[r8]   //r7:coef[8]
    vdup.16 q10,r5
    vmul.s16 q9,q9,q10  //Q9:imgPad*coef[8]
	sub r8,r8,#32

    vadd.s16 q1,q1,q2
    vadd.s16 q1,q1,q3
    vadd.s16 q1,q1,q4
    vadd.s16 q1,q1,q5
    vadd.s16 q1,q1,q6
    vadd.s16 q1,q1,q7
    vadd.s16 q1,q1,q8
    vadd.s16 q1,q1,q9

    mov r5,#32
    vdup.16 q2,r5
    vadd.s16 q1,q1,q2
    vshr.s16 q1,q1,#6

    //clip
    mov r5,#1
    lsl r5,r5,r9
    sub r5,r5,#1 //MAX
    mov r7,#0  //MIN
    vdup.16 q3,r5
    vdup.16 q4,r7
    vmax.s16 q1,q1,q4
    vmin.s16 q1,q1,q3
    vmovn.s16 d6,q1
	vmov.s32 r7,d1[0]
	sub r5,r7,r6
	mov r10,#15
	and r5,r5,r10
	cmp r5,#4
	beq store
	add r5,r0,r4
    vst1.64 d6,[r5]

store:
    add r5,r0,r4
    vst1.32 d6[0],[r5]

    add r4,r4,#8  //j+8
    b Loop2
    
Exit:
    add r1,r1,r2
    add r0,r0,r2
    add r3,r3,#1  //i++
    b Loop1

xnd_filter:
    ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12,lr}
	mov pc, lr
.endfunc
