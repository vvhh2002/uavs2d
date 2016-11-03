.macro function name
     .global \name
\name:
     .hidden \name
     .type   \name, %function
     .func   \name
\name:
.endm


//***********************************************************************
//Detail for IDCT -- Neon
//Function List:
//void idct_4x4_shift12_neon(coef_t *blk, int shift, int clip)
//void idct_8x8_shift12_neon(coef_t *blk, int shift, int clip) 
//void idct_8x8_shift12_neon2(coef_t *blk, int shift, int clip)
//void idct_16x16_shift12_neon(coef_t *blk, int shift, int clip)
//void idct_4x16_shift12_neon(coef_t *blk, int shift, int clip)
//void idct_16x4_shift12_neon(coef_t *blk, int shift, int clip)
//void idct_32x32_shift12_neon(coef_t *blk, int shift, int clip)
//void idct_32x32_shift11_neon(coef_t *blk, int shift, int clip)
//void idct_8x32_shift12_neon(coef_t *blk, int shift, int clip)
//void idct_8x32_shift11_neon(coef_t *blk, int shift, int clip)
//void idct_32x8_shift12_neon(coef_t *blk, int shift, int clip)
//void idct_32x8_shift11_neon(coef_t *blk, int shift, int clip)
//Notes:
//     The formal parameter "shift" in the function is not used. Its 
// existence is for the compatibility of the C versions. In fact, the 
// value "shift" is wirtten as immediate in the code, which is either 11
// or 12, as the name of the function indicates.
//     The other functions are used for tests during coding, such as 
// "void partialButterflyInverse16_shift5_clip16_neon(Short *src,Short *dst)",
// which is the first inverse transform of the matrix 16x16. And the
// transform of 16x16 idct is composed of the two functions, namely 
// "partialButterflyInverse16_shift5_clip16_neon","partialButterflyInverse16
// _shift12_clipx_neon".
//     And the idct8x8 has two versions, however, the first one has more 
// efficiency, as it does not have much memory access during the caculation.
//Author:
// CaoHongbin 2015.1.28
//***********************************************************************


//***********************************************************************
//void idct_4x4_shift12_neon(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 12 in code
//clip: clip
//***********************************************************************
function idct_4x4_shift12_neon //
	PUSH {R4-R9, LR}	

	VLD1.64 {D0, D1, D2, D3}, [R0]
		
	MOV R5, #32
	MOVT R5, #32 
	MOV R6, #42	
	MOVT R6, #17	
	VMOV D4, R5, R6 //D4[0]=32, D4[1]=32,D4[2]=42, D4[3]=17
	
	//O[0]
	VMULL.S16 Q8, D1, D4[2]
	VMLAL.S16 Q8, D3, D4[3]
	//O[1]
	VMULL.S16 Q9, D1, D4[3]
	VMLSL.S16 Q9, D3, D4[2]
	//E[0]
	VMULL.S16 Q10, D0, D4[0]
	VMLAL.S16 Q10, D2, D4[0]
	//E[1]
	VMULL.S16 Q11, D0, D4[0]
	VMLSL.S16 Q11, D2, D4[0]
	
	VQADD.S32 Q12, Q8, Q10
	VQRSHRN.S32 D0, Q12, #5
	
	VQADD.S32 Q13, Q11, Q9
	VQRSHRN.S32 D1, Q13, #5
	
	VQSUB.S32 Q14, Q11, Q9
	VQRSHRN.S32 D2, Q14, #5
	
	VQSUB.S32 Q15, Q10, Q8
	VQRSHRN.S32 D3, Q15, #5
	
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3	

	//second butterfly
	//O[0]
	VMULL.S16 Q8, D1, D4[2]
	VMLAL.S16 Q8, D3, D4[3]
	//O[1]
	VMULL.S16 Q9, D1, D4[3]
	VMLSL.S16 Q9, D3, D4[2]
	//E[0]
	VMULL.S16 Q10, D0, D4[0]
	VMLAL.S16 Q10, D2, D4[0]
	//E[1]
	VMULL.S16 Q11, D0, D4[0]
	VMLSL.S16 Q11, D2, D4[0]
	
	VQADD.S32 Q12, Q8, Q10
	VQRSHRN.S32 D0, Q12, #12
	
	VQADD.S32 Q13, Q11, Q9
	VQRSHRN.S32 D1, Q13, #12
	
	VQSUB.S32 Q14, Q11, Q9
	VQRSHRN.S32 D2, Q14, #12
	
	VQSUB.S32 Q15, Q10, Q8
	VQRSHRN.S32 D3, Q15, #12
	
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3	

	//clip
	MVN R7, #0
	SUB R2, R2, #1
	LSL R8, R7, R2 	//minvalue R8
	MVN R9, R8 		//maxvalue R9
	VDUP.16 Q8, R8	//minval vector Q8
	VDUP.16 Q9, R9  //maxval vector Q9
	VMAX.S16 Q0, Q0, Q8
	VMAX.s16 Q1, Q1, Q8
	VMIN.S16 Q0, Q0, Q9
	VMIN.S16 Q1, Q1, Q9

	//store to blk

	VST1.64 {D0, D1, D2, D3}, [R0]

	POP {R4-R9, PC}	   
.endfunc


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


//***********************************************************************
//void idct_8x8_shift12_neon(coef_t *blk, int shift, int clip) 
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 12 in code  
//clip: clip
//***********************************************************************
function idct_8x8_shift12_neon //
	PUSH {R4-R12, LR}

	MOV R10, R0 //save coeffs addr
	MOV R9, #16

	//load the first 4 column
	//VLD1.8  {D0, D1, D2, D3, D4, D5, D6, D7}, [R0]!
	VLD1.8 D2, [R0], R9  //SRC[0]
	VLD1.8 D3, [R0], R9  //SRC[LINE]
	VLD1.8 D4, [R0], R9  //SRC[2*LINE]
	VLD1.8 D5, [R0], R9  //SRC[3*LINE]
	VLD1.8 D6, [R0], R9  //SRC[4*LINE]
	VLD1.8 D7, [R0], R9  //SRC[5*LINE]
	VLD1.8 D8, [R0], R9  //SRC[6*LINE]
	VLD1.8 D9, [R0], R9  //SRC[7*LINE]

	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D0, R4, R5  //D0[0]=32, D0[1]=32, D0[2]=17, D0[3]=42

	//E[0]
	VMULL.S16 Q5, D2, D0[0]
	VMLAL.S16 Q5, D4, D0[3]
	VMLAL.S16 Q5, D6, D0[0]
	VMLAL.S16 Q5, D8, D0[2]
	//E[1]
	VMULL.S16 Q6, D2, D0[0]
	VMLAL.S16 Q6, D4, D0[2]
	VMLSL.S16 Q6, D6, D0[0]
	VMLSL.S16 Q6, D8, D0[3]
	//E[2]
	VMULL.S16 Q7, D2, D0[0]
	VMLSL.S16 Q7, D4, D0[2]
	VMLSL.S16 Q7, D6, D0[0]
	VMLAL.S16 Q7, D8, D0[3]
	//E[3]
	VMULL.S16 Q8, D2, D0[0]
	VMLSL.S16 Q8, D4, D0[3]
	VMLAL.S16 Q8, D6, D0[0]
	VMLSL.S16 Q8, D8, D0[2]

	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D1, R4, R5  //D1[0]=9, D1[1]=25, D1[2]=38, D1[3]=44

	//O[0]
	VMULL.S16 Q9, D3, D1[3]
	VMLAL.S16 Q9, D5, D1[2]
	VMLAL.S16 Q9, D7, D1[1]
	VMLAL.S16 Q9, D9, D1[0]
	//O[1]
	VMULL.S16 Q10, D3, D1[2]
	VMLSL.S16 Q10, D5, D1[0]
	VMLSL.S16 Q10, D7, D1[3]
	VMLSL.S16 Q10, D9, D1[1]
	//O[2]
	VMULL.S16 Q11, D3, D1[1]
	VMLSL.S16 Q11, D5, D1[3]
	VMLAL.S16 Q11, D7, D1[0]
	VMLAL.S16 Q11, D9, D1[2]
	//O[3]
	VMULL.S16 Q12, D3, D1[0]
	VMLSL.S16 Q12, D5, D1[1]
	VMLAL.S16 Q12, D7, D1[2]
	VMLSL.S16 Q12, D9, D1[3]

	//CALCULATE DCT1 result
	VADD.S32 Q1, Q5, Q9     //DST[0]
	VADD.S32 Q0, Q6, Q10    //DST[1]
	VADD.S32 Q2, Q7, Q11    //DST[2]
	VADD.S32 Q3, Q8, Q12    //DST[3]
	VSUB.S32 Q4, Q8, Q12    //DST[4]
	VSUB.S32 Q13, Q7, Q11    //DST[5]
	VSUB.S32 Q14, Q6, Q10    //DST[6]
	VSUB.S32 Q15, Q5, Q9    //DST[7]

	VQRSHRN.S32 D2, Q1, #5
	VQRSHRN.S32 D3, Q0, #5
	VQRSHRN.S32 D4, Q2, #5
	VQRSHRN.S32 D5, Q3, #5
	VQRSHRN.S32 D6, Q4, #5
	VQRSHRN.S32 D7, Q13, #5
	VQRSHRN.S32 D8, Q14, #5
	VQRSHRN.S32 D9, Q15, #5

	// D2 D6
	// D3 D7
	// D4 D8
	// D5 D9
	// inverse matrix
	VTRN.32 D2, D4
	VTRN.32 D3, D5
	VTRN.32 D6, D8
	VTRN.32 D7, D9
	VTRN.16 D2, D3
	VTRN.16 D4, D5
	VTRN.16 D6, D7
	VTRN.16 D8, D9

	//LAST FOUR ROWS
	ADD R0, R10, #8
	//load the last four columns
	VLD1.64 D10, [R0], R9  //SRC[0]
	VLD1.64 D11, [R0], R9  //SRC[LINE]
	VLD1.64 D12, [R0], R9  //SRC[2*LINE]
	VLD1.64 D13, [R0], R9  //SRC[3*LINE]
	VLD1.64 D14, [R0], R9  //SRC[4*LINE]
	VLD1.64 D15, [R0], R9  //SRC[5*LINE]
	VLD1.64 D16, [R0], R9  //SRC[6*LINE]
	VLD1.64 D17, [R0], R9  //SRC[7*LINE]
	//VLD1.8  {D10, D11, D12, D13, D14, D15, D16, D17}, [R0]!

	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D0, R4, R5  //D0[0]=32, D0[1]=32, D0[2]=17, D0[3]=42

	//E[0]
	VMULL.S16 Q9, D10, D0[0] //the third parameters: d0-d7
	VMLAL.S16 Q9, D12, D0[3]
	VMLAL.S16 Q9, D14, D0[0]
	VMLAL.S16 Q9, D16, D0[2]
	//E[1]
	VMULL.S16 Q10, D10, D0[0]
	VMLAL.S16 Q10, D12, D0[2]
	VMLSL.S16 Q10, D14, D0[0]
	VMLSL.S16 Q10, D16, D0[3]
	//E[2]
	VMULL.S16 Q11, D10, D0[0]
	VMLSL.S16 Q11, D12, D0[2]
	VMLSL.S16 Q11, D14, D0[0]
	VMLAL.S16 Q11, D16, D0[3]
	//E[3]
	VMULL.S16 Q12, D10, D0[0]
	VMLSL.S16 Q12, D12, D0[3]
	VMLAL.S16 Q12, D14, D0[0]
	VMLSL.S16 Q12, D16, D0[2]

	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D0, R4, R5  //D0[0]=9, D0[1]=25, D0[2]=38, D0[3]=44

	//O[0]
	VMULL.S16 Q13, D11, D0[3]
	VMLAL.S16 Q13, D13, D0[2]
	VMLAL.S16 Q13, D15, D0[1]
	VMLAL.S16 Q13, D17, D0[0]
	//O[1]
	VMULL.S16 Q14, D11, D0[2]
	VMLSL.S16 Q14, D13, D0[0]
	VMLSL.S16 Q14, D15, D0[3]
	VMLSL.S16 Q14, D17, D0[1]
	//O[2]
	VMULL.S16 Q15, D11, D0[1]
	VMLSL.S16 Q15, D13, D0[3]
	VMLAL.S16 Q15, D15, D0[0]
	VMLAL.S16 Q15, D17, D0[2]
	//O[3]
	VMULL.S16 Q5, D11, D0[0]
	VMLSL.S16 Q5, D13, D0[1]
	VMLAL.S16 Q5, D15, D0[2]
	VMLSL.S16 Q5, D17, D0[3]

	//CALCULATE DST
	VADD.S32 Q0, Q9, Q13     //DST[0]
	VADD.S32 Q6, Q10, Q14    //DST[1]
	VADD.S32 Q7, Q11, Q15    //DST[2]
	VADD.S32 Q8, Q12, Q5    //DST[3]
	VSUB.S32 Q12, Q12, Q5    //DST[4]
	VSUB.S32 Q11, Q11, Q15   //DST[5]
	VSUB.S32 Q10, Q10, Q14   //DST[6]
	VSUB.S32 Q9, Q9, Q13    //DST[7]

	VQRSHRN.S32 D10, Q0, #5
	VQRSHRN.S32 D11, Q6, #5
	VQRSHRN.S32 D12, Q7, #5
	VQRSHRN.S32 D13, Q8, #5
	VQRSHRN.S32 D14, Q12, #5
	VQRSHRN.S32 D15, Q11, #5
	VQRSHRN.S32 D16, Q10, #5
	VQRSHRN.S32 D17, Q9, #5

	// D10 D14
	// D11 D15
	// D12 D16
	// D13 D17
	VTRN.32 D10, D12
	VTRN.32 D11, D13
	VTRN.16 D10, D11
	VTRN.16 D12, D13
	VTRN.32 D14, D16
	VTRN.32 D15, D17
	VTRN.16 D14, D15
	VTRN.16 D16, D17

	//second butterfly
	//First four rows
	//the first four columns: D2, D3, d4, d5, D10, D11, D12, D13
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D0, R4, R5  //D0[0]=32, D0[1]=32, D0[2]=17, D0[3]=42

	//E[0]
	VMULL.S16 Q9, D2, D0[0]
	VMLAL.S16 Q9, D4, D0[3]
	VMLAL.S16 Q9, D10, D0[0]
	VMLAL.S16 Q9, D12, D0[2]
	//E[1]
	VMULL.S16 Q10, D2, D0[0]
	VMLAL.S16 Q10, D4, D0[2]
	VMLSL.S16 Q10, D10, D0[0]
	VMLSL.S16 Q10, D12, D0[3]
	//E[2]
	VMULL.S16 Q11, D2, D0[0]
	VMLSL.S16 Q11, D4, D0[2]
	VMLSL.S16 Q11, D10, D0[0]
	VMLAL.S16 Q11, D12, D0[3]
	//E[3]
	VMULL.S16 Q12, D2, D0[0]
	VMLSL.S16 Q12, D4, D0[3]
	VMLAL.S16 Q12, D10, D0[0]
	VMLSL.S16 Q12, D12, D0[2]

	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D1, R4, R5  //D1[0]=9, D1[1]=25, D1[2]=38, D1[3]=44

	//O[0]
	VMULL.S16 Q13, D3, D1[3]
	VMLAL.S16 Q13, D5, D1[2]
	VMLAL.S16 Q13, D11, D1[1]
	VMLAL.S16 Q13, D13, D1[0]
	//O[1]
	VMULL.S16 Q14, D3, D1[2]
	VMLSL.S16 Q14, D5, D1[0]
	VMLSL.S16 Q14, D11, D1[3]
	VMLSL.S16 Q14, D13, D1[1]
	//O[2]
	VMULL.S16 Q15, D3, D1[1]
	VMLSL.S16 Q15, D5, D1[3]
	VMLAL.S16 Q15, D11, D1[0]
	VMLAL.S16 Q15, D13, D1[2]
	//O[3]
	VMULL.S16 Q1, D3, D1[0]
	VMLSL.S16 Q1, D5, D1[1]
	VMLAL.S16 Q1, D11, D1[2]
	VMLSL.S16 Q1, D13, D1[3]

	//CALCULATE DST
	VADD.S32 Q0, Q9, Q13     //DST[0]
	VADD.S32 Q2, Q10, Q14    //DST[1]
	VADD.S32 Q5, Q11, Q15    //DST[2]
	VADD.S32 Q6, Q12, Q1    //DST[3]
	VSUB.S32 Q12, Q12, Q1    //DST[4]
	VSUB.S32 Q11, Q11, Q15    //DST[5]
	VSUB.S32 Q10, Q10, Q14    //DST[6]
	VSUB.S32 Q9, Q9, Q13    //DST[7]

	//store to q1, q2, q5, q6

	VQRSHRN.S32 D4,  Q2,  #12	//DST[1]
	VQRSHRN.S32 D10, Q5,  #12 	//DST[2]
	VQRSHRN.S32 D12, Q6,  #12	//DST[3]
	VQRSHRN.S32 D2,  Q0,  #12 	//DST[0]
	VQRSHRN.S32 D5,  Q11, #12	//DST[5]
	VQRSHRN.S32 D3,  Q12, #12	//DST[4]
	VQRSHRN.S32 D11, Q10, #12	//DST[6]
	VQRSHRN.S32 D13, Q9,  #12	//DST[7]



	VTRN.32 D2, D10
	VTRN.32 D4, D12
	VTRN.32 D3, D11
	VTRN.32 D5, D13
	VTRN.16 D2, D4
	VTRN.16 D10, D12
	VTRN.16 D3, D5
	VTRN.16 D11, D13


	//clip
	MVN R7, #0
	SUB R2, R2, #1
	LSL R8, R7, R2 	//minvalue R8
	MVN R9, R8 		//maxvalue R9
	VDUP.16 Q9,R8	//minval vector Q9
	VDUP.16 Q10,R9  //maxval vector Q10
	VMAX.S16 Q1, Q1, Q9
	VMAX.s16 Q2, Q2, Q9
	VMAX.S16 Q5, Q5, Q9
	VMAX.s16 Q6, Q6, Q9
	VMIN.S16 Q1, Q1, Q10
	VMIN.S16 Q2, Q2, Q10
	VMIN.S16 Q5, Q5, Q10
	VMIN.S16 Q6, Q6, Q10

	//store
	VST1.64 {D2, D3, D4, D5}, [R10]!



	// LAST FOUR ROWS
	//THE RIGHT FOUR COLUMNS: D6, D7, D8, D9, D14, D15, D16, D17
	MOV R6, #32
	MOVT R6, #32
	MOV R7, #17
	MOVT R7, #42
	VMOV D0, R6, R7  //D0[0]=32, D0[1]=32, D0[2]=17, D0[3]=42

	//store
	VST1.64 {D10, D11, D12, D13}, [R10]!

	//E[0]
	VMULL.S16 Q1, D6, D0[0]
	VMLAL.S16 Q1, D8, D0[3]
	VMLAL.S16 Q1, D14, D0[0]
	VMLAL.S16 Q1, D16, D0[2]
	//E[1]
	VMULL.S16 Q2, D6, D0[0]
	VMLAL.S16 Q2, D8, D0[2]
	VMLSL.S16 Q2, D14, D0[0]
	VMLSL.S16 Q2, D16, D0[3]
	//E[2]
	VMULL.S16 Q5, D6, D0[0]
	VMLSL.S16 Q5, D8, D0[2]
	VMLSL.S16 Q5, D14, D0[0]
	VMLAL.S16 Q5, D16, D0[3]
	//E[3]
	VMULL.S16 Q6, D6, D0[0]
	VMLSL.S16 Q6, D8, D0[3]
	VMLAL.S16 Q6, D14, D0[0]
	VMLSL.S16 Q6, D16, D0[2]

	MOV R6, #9
	MOVT R6, #25
	MOV R7, #38
	MOVT R7, #44
	VMOV D1, R6, R7  //D1[0]=9, D1[1]=25, D1[2]=38, D1[3]=44

	//O[0]
	VMULL.S16 Q9, D7, D1[3]
	VMLAL.S16 Q9, D9, D1[2]
	VMLAL.S16 Q9, D15, D1[1]
	VMLAL.S16 Q9, D17, D1[0]
	//O[1]
	VMULL.S16 Q10, D7, D1[2]
	VMLSL.S16 Q10, D9, D1[0]
	VMLSL.S16 Q10, D15, D1[3]
	VMLSL.S16 Q10, D17, D1[1]
	//O[2]
	VMULL.S16 Q11, D7, D1[1]
	VMLSL.S16 Q11, D9, D1[3]
	VMLAL.S16 Q11, D15, D1[0]
	VMLAL.S16 Q11, D17, D1[2]
	//O[3]
	VMULL.S16 Q12, D7, D1[0]
	VMLSL.S16 Q12, D9, D1[1]
	VMLAL.S16 Q12, D15, D1[2]
	VMLSL.S16 Q12, D17, D1[3]

	//CALCULATE DST
	VADD.S32 Q0, Q1, Q9     //DST[0]
	VADD.S32 Q3, Q2, Q10    //DST[1]
	VADD.S32 Q4, Q5, Q11    //DST[2]
	VADD.S32 Q7, Q6, Q12    //DST[3]
	VSUB.S32 Q8, Q6, Q12    //DST[4]
	VSUB.S32 Q13, Q5, Q11   //DST[5]
	VSUB.S32 Q14, Q2, Q10   //DST[6]
	VSUB.S32 Q15, Q1, Q9    //DST[7]

	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D2, Q3, #12
	VQRSHRN.S32 D4, Q4, #12
	VQRSHRN.S32 D6, Q7, #12
	VQRSHRN.S32 D1, Q8, #12
	VQRSHRN.S32 D3, Q13, #12
	VQRSHRN.S32 D5, Q14, #12
	VQRSHRN.S32 D7, Q15, #12

	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7

	//clip
	MVN R7, #0
	LSL R8, R7, R2 	//minvalue R8
	MVN R9, R8 		//maxvalue R9
	VDUP.16 Q9,R8	//minval vector Q9
	VDUP.16 Q10,R9  //maxval vector Q10
	VMAX.S16 Q0, Q0, Q9
	VMAX.s16 Q1, Q1, Q9
	VMAX.S16 Q2, Q2, Q9
	VMAX.s16 Q3, Q3, Q9
	VMIN.S16 Q0, Q0, Q10
	VMIN.S16 Q1, Q1, Q10
	VMIN.S16 Q2, Q2, Q10
	VMIN.S16 Q3, Q3, Q10

	//store
	VST1.64 {D0,D1,D2,D3}, [R10]!
	VST1.64 {D4,D5,D6,D7}, [R10]!
	POP {R4-R12, PC}
.endfunc


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


//************************************************************************
//macro: the first inverse transform of 8
//************************************************************************
.macro partialButterflyInverse8_4rows_shift5_clip16
	
	MOV R9, #16
	
	//FIRST FOUR ROWS
	
	VLD1.64 D0, [R0], R9  //SRC[0]
	VLD1.64 D1, [R0], R9  //SRC[LINE]
	VLD1.64 D2, [R0], R9  //SRC[2*LINE]
	VLD1.64 D3, [R0], R9  //SRC[3*LINE]
	VLD1.64 D4, [R0], R9  //SRC[4*LINE]
	VLD1.64 D5, [R0], R9  //SRC[5*LINE]
	VLD1.64 D6, [R0], R9  //SRC[6*LINE]
	VLD1.64 D7, [R0], R9  //SRC[7*LINE]
	VMOV D8, D7 //SAVE D7
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=44
	
	//E[0]
	VMULL.S16 Q5, D0, D7[0]
	VMLAL.S16 Q5, D2, D7[3]
	VMLAL.S16 Q5, D4, D7[0]
	VMLAL.S16 Q5, D6, D7[2]
	//E[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[2]
	VMLSL.S16 Q6, D4, D7[0]
	VMLSL.S16 Q6, D6, D7[3]
	//E[2]
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D4, D7[0]
	VMLAL.S16 Q7, D6, D7[3]	
	//E[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D2, D7[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D6, D7[2]	

	VMOV D7, D8
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D6, R4, R5  //D6[0]=9, D6[1]=25, D6[2]=38, D6[3]=44
	
	//O[0]
	VMULL.S16 Q9, D1, D6[3]
	VMLAL.S16 Q9, D3, D6[2]
	VMLAL.S16 Q9, D5, D6[1]
	VMLAL.S16 Q9, D7, D6[0]	
	//O[1]
	VMULL.S16 Q10, D1, D6[2]
	VMLSL.S16 Q10, D3, D6[0]
	VMLSL.S16 Q10, D5, D6[3]
	VMLSL.S16 Q10, D7, D6[1]	
	//O[2]
	VMULL.S16 Q11, D1, D6[1]
	VMLSL.S16 Q11, D3, D6[3]
	VMLAL.S16 Q11, D5, D6[0]
	VMLAL.S16 Q11, D7, D6[2]	
	//O[3]
	VMULL.S16 Q12, D1, D6[0]
	VMLSL.S16 Q12, D3, D6[1]
	VMLAL.S16 Q12, D5, D6[2]
	VMLSL.S16 Q12, D7, D6[3]
	
	//CALCULATE DST
	VADD.S32 Q0, Q5, Q9     //DST[0]
	VADD.S32 Q1, Q6, Q10    //DST[1]
	VADD.S32 Q2, Q7, Q11    //DST[2]
	VADD.S32 Q3, Q8, Q12    //DST[3]
	VSUB.S32 Q4, Q8, Q12    //DST[4]
	VSUB.S32 Q13, Q7, Q11    //DST[5]
	VSUB.S32 Q14, Q6, Q10    //DST[6]
	VSUB.S32 Q15, Q5, Q9    //DST[7]
	
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D2, Q1, #5
	VQRSHRN.S32 D4, Q2, #5
	VQRSHRN.S32 D6, Q3, #5
	VQRSHRN.S32 D1, Q4, #5
	VQRSHRN.S32 D3, Q13, #5
	VQRSHRN.S32 D5, Q14, #5
	VQRSHRN.S32 D7, Q15, #5
	
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7
	

	VST1.64 {D0, D1, D2, D3}, [R1]!
	VST1.64 {D4, D5, D6, D7}, [R1]!	
.endm

//************************************************************************
//macro: the second inverse transform of 8
//************************************************************************
.macro partialButterflyInverse8_4rows_shift12_clipx
	
	MOV R9, #16
	
	//FIRST FOUR ROWS
	
	VLD1.64 D0, [R0], R9  //SRC[0]
	VLD1.64 D1, [R0], R9  //SRC[LINE]
	VLD1.64 D2, [R0], R9  //SRC[2*LINE]
	VLD1.64 D3, [R0], R9  //SRC[3*LINE]
	VLD1.64 D4, [R0], R9  //SRC[4*LINE]
	VLD1.64 D5, [R0], R9  //SRC[5*LINE]
	VLD1.64 D6, [R0], R9  //SRC[6*LINE]
	VLD1.64 D7, [R0], R9  //SRC[7*LINE]
	VMOV D8, D7 //SAVE D7
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=44
	
	//E[0]
	VMULL.S16 Q5, D0, D7[0]
	VMLAL.S16 Q5, D2, D7[3]
	VMLAL.S16 Q5, D4, D7[0]
	VMLAL.S16 Q5, D6, D7[2]
	//E[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[2]
	VMLSL.S16 Q6, D4, D7[0]
	VMLSL.S16 Q6, D6, D7[3]
	//E[2]
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D4, D7[0]
	VMLAL.S16 Q7, D6, D7[3]	
	//E[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D2, D7[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D6, D7[2]	

	VMOV D7, D8
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D6, R4, R5  //D6[0]=9, D6[1]=25, D6[2]=38, D6[3]=44
	
	//O[0]
	VMULL.S16 Q9, D1, D6[3]
	VMLAL.S16 Q9, D3, D6[2]
	VMLAL.S16 Q9, D5, D6[1]
	VMLAL.S16 Q9, D7, D6[0]	
	//O[1]
	VMULL.S16 Q10, D1, D6[2]
	VMLSL.S16 Q10, D3, D6[0]
	VMLSL.S16 Q10, D5, D6[3]
	VMLSL.S16 Q10, D7, D6[1]	
	//O[2]
	VMULL.S16 Q11, D1, D6[1]
	VMLSL.S16 Q11, D3, D6[3]
	VMLAL.S16 Q11, D5, D6[0]
	VMLAL.S16 Q11, D7, D6[2]	
	//O[3]
	VMULL.S16 Q12, D1, D6[0]
	VMLSL.S16 Q12, D3, D6[1]
	VMLAL.S16 Q12, D5, D6[2]
	VMLSL.S16 Q12, D7, D6[3]
	
	//CALCULATE DST
	VADD.S32 Q0, Q5, Q9     //DST[0]
	VADD.S32 Q1, Q6, Q10    //DST[1]
	VADD.S32 Q2, Q7, Q11    //DST[2]
	VADD.S32 Q3, Q8, Q12    //DST[3]
	VSUB.S32 Q4, Q8, Q12    //DST[4]
	VSUB.S32 Q13, Q7, Q11    //DST[5]
	VSUB.S32 Q14, Q6, Q10    //DST[6]
	VSUB.S32 Q15, Q5, Q9    //DST[7]
	
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D2, Q1, #12
	VQRSHRN.S32 D4, Q2, #12
	VQRSHRN.S32 D6, Q3, #12
	VQRSHRN.S32 D1, Q4, #12
	VQRSHRN.S32 D3, Q13, #12
	VQRSHRN.S32 D5, Q14, #12
	VQRSHRN.S32 D7, Q15, #12
	
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7

	//clip
	MVN R6, #0
	LSL R7, R6, R2 	//minvalue R8
	MVN R8, R7 		//maxvalue R9
	VDUP.16 Q9,R7	//minval vector Q9
	VDUP.16 Q10,R8  //maxval vector Q10
	VMAX.S16 Q0, Q0, Q9
	VMAX.s16 Q1, Q1, Q9
	VMAX.S16 Q2, Q2, Q9
	VMAX.s16 Q3, Q3, Q9
	VMIN.S16 Q0, Q0, Q10
	VMIN.S16 Q1, Q1, Q10
	VMIN.S16 Q2, Q2, Q10
	VMIN.S16 Q3, Q3, Q10

	VST1.64 {D0, D1, D2, D3}, [R1]!
	VST1.64 {D4, D5, D6, D7}, [R1]!  
.endm

//***********************************************************************
//void partialButterflyInverse8_shift5_clip16_neon(Short *src,Short *dst)
//src: transform coefficients
//dst: the result   
//detail: shift=5 clip=16
//***********************************************************************
function partialButterflyInverse8_shift5_clip16_neon
	PUSH {R4-R12, LR}
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////

	partialButterflyInverse8_4rows_shift5_clip16

	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8_4rows_shift5_clip16

	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void partialButterflyInverse8_shift12_clipx_neon(Short *src,Short *dst,int clip)
//src: transform coefficients
//dst: the result   
//detail: shift=12 clip=x
//***********************************************************************
function partialButterflyInverse8_shift12_clipx_neon
	PUSH {R4-R12, LR}
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 
	SUB R2, R2, #1

	///////////////////1ST  4 ROWS///////////////////

	partialButterflyInverse8_4rows_shift12_clipx

	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8_4rows_shift12_clipx

	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void idct_8x8_shift12_neon2(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 12 in code  
//clip: clip
//ps: this is the second choice of the idct_8x8_shift12_neon
//***********************************************************************
function idct_8x8_shift12_neon2
	PUSH {R4-R12, LR}

	////////////////////alloc mem ///////////////////

	SUB R12, SP, #128
	MOV R1, R12

	//////////////////////////////////////////////////
	///////////////////first trans////////////////////
	//////////////////////////////////////////////////

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////

	partialButterflyInverse8_4rows_shift5_clip16

	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8_4rows_shift5_clip16

	
	//////////////////////////////////////////////////
	///////////////////second trans///////////////////
	//////////////////////////////////////////////////

	MOV R0, R12
	MOV R1, R10

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 
	SUB R2, R2, #1

	///////////////////1ST  4 ROWS///////////////////

	partialButterflyInverse8_4rows_shift12_clipx

	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8_4rows_shift12_clipx

	///////////////////    END    ///////////////////

	POP {R4-R12, PC}
.endfunc


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


//************************************************************************
//macro: the first inverse transform of 16
//************************************************************************
.macro partButterflyInverse16_4rows_shift5_clip16

	//CALCULATE EO
	ADD R0, R10, #64
	MOV R9, #128
	
	VLD1.64 D0, [R0], R9     //src[ 2*line]
	VLD1.64 D1, [R0], R9     //src[ 6*line]
	VLD1.64 D2, [R0], R9     //src[10*line]
	VLD1.64 D3, [R0], R9     //src[14*line]
	
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D7, R4, R5  //D7[0]=9, D7[1]=25, D7[2]=38, D7[3]=44
	
	VMULL.S16 Q12, D0, D7[3]
	VMLAL.S16 Q12, D1, D7[2]
	VMLAL.S16 Q12, D2, D7[1]
	VMLAL.S16 Q12, D3, D7[0]    //EO[0]
	
	VMULL.S16 Q13, D0, D7[2]
	VMLSL.S16 Q13, D1, D7[0]
	VMLSL.S16 Q13, D2, D7[3]
	VMLSL.S16 Q13, D3, D7[1]    //EO[1]
	
	VMULL.S16 Q14, D0, D7[1]
	VMLSL.S16 Q14, D1, D7[3]
	VMLAL.S16 Q14, D2, D7[0]
	VMLAL.S16 Q14, D3, D7[2]    //EO[2]
		
	VMULL.S16 Q15, D0, D7[0]
	VMLSL.S16 Q15, D1, D7[1]
	VMLAL.S16 Q15, D2, D7[2]
	VMLSL.S16 Q15, D3, D7[3]    //EO[3]
	
	//CALCULATE EE
	MOV R0, R10
	VLD1.64 D0, [R0], R9      //src[ 0      ]
	VLD1.64 D1, [R0], R9      //src[ 4*line ]
	VLD1.64 D2, [R0], R9      //src[ 8*line  ]
	VLD1.64 D3, [R0], R9      //src[ 12*line ]
	
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D4, R4, R5  //D4[0]=32, D4[1]=32, D4[2]=17, D4[3]=42

	VMULL.S16 Q4, D1, D4[3]
	VMLAL.S16 Q4, D3, D4[2]     //EEO[0]
	VMULL.S16 Q5, D0, D4[0]
	VMLAL.S16 Q5, D2, D4[0]     //EEE[0]
	VMULL.S16 Q6, D1, D4[2]
	VMLSL.S16 Q6, D3, D4[3]     //EEO[1]
	VMULL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D2, D4[0]     //EEE[1]
	
	VADD.S32 Q8, Q5, Q4         //EE[0]
	VADD.S32 Q9, Q6, Q7         //EE[1]
	VSUB.S32 Q10, Q7, Q6        //EE[2]
	VSUB.S32 Q11, Q5, Q4        //EE[3]
	////////////////////////////////////////////////////////////////////
	VADD.S32 Q0, Q8, Q12        //E[0]
	VADD.S32 Q1, Q9, Q13        //E[1]
	VADD.S32 Q2, Q10, Q14       //E[2]
	VADD.S32 Q3, Q11, Q15       //E[3]
	VSUB.S32 Q4, Q11, Q15       //E[4]
	VSUB.S32 Q5, Q10, Q14       //E[5]
	VSUB.S32 Q6, Q9, Q13        //E[6]
	VSUB.S32 Q7, Q8, Q12        //E[7]
	
	VPUSH {D0-D15}              //PUSH E[0]-E[7]
	////////////////////////////////////////////////////////////////////
	
	ADD R0, R10, #32
	MOV R9, #64
	
	VLD1.64 D0, [R0], R9        //src[   line]
	VLD1.64 D1, [R0], R9        //src[ 3*line]
	VLD1.64 D2, [R0], R9        //src[ 5*line]
	VLD1.64 D3, [R0], R9        //src[ 7*line]
	VLD1.64 D4, [R0], R9        //src[ 9*line]
	VLD1.64 D5, [R0], R9        //src[11*line]
	VLD1.64 D6, [R0], R9        //src[13*line]
	VLD1.64 D7, [R0], R9        //src[15*line]
	//SAVE 
	VMOV Q6, Q2
	VMOV Q7, Q3
	
	//LOAD G_AIT16  1 3 5 7 9 11 13 15
	MOV R7, #4
	MOVT R7, #13
	MOV R3, #21
	MOVT R3, #29
	MOV R4, #35
	MOVT R4, #40
	MOV R5, #43
	MOVT R5, #45
	VMOV D6, R7, R3 //D6[0]=4 ,D6[1]=13,D6[2]=21,D6[3]=29
	VMOV D7, R4, R5 //D7[0]=35,D7[1]=40,D7[2]=43,D7[3]=45
		
	//O[0]
	VMULL.S16 Q8, D0, D7[3]
	VMLAL.S16 Q8, D1, D7[2]
	VMLAL.S16 Q8, D2, D7[1]
	VMLAL.S16 Q8, D3, D7[0]
	VMLAL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q7
	VMLAL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[0]
	VMOV Q2, Q6
	//O[1]
	VMULL.S16 Q9, D0, D7[2]
	VMLAL.S16 Q9, D1, D6[3]
	VMLAL.S16 Q9, D2, D6[0]
	VMLSL.S16 Q9, D3, D6[2]
	VMLSL.S16 Q9, D4, D7[1]
	VMLSL.S16 Q9, D5, D7[3]
	VMOV Q2, Q7
	VMLSL.S16 Q9, D4, D7[0]
	VMLSL.S16 Q9, D5, D6[1]
	VMOV Q2, Q6
	//O[2]
	VMULL.S16 Q10, D0, D7[1]
	VMLAL.S16 Q10, D1, D6[0]
	VMLSL.S16 Q10, D2, D7[0]
	VMLSL.S16 Q10, D3, D7[2]
	VMLSL.S16 Q10, D4, D6[1]
	VMLAL.S16 Q10, D5, D6[3]
	VMOV Q2, Q7
	VMLAL.S16 Q10, D4, D7[3]
	VMLAL.S16 Q10, D5, D6[2]
	VMOV Q2, Q6
	//O[3]
	VMULL.S16 Q11, D0, D7[0]
	VMLSL.S16 Q11, D1, D6[2]
	VMLSL.S16 Q11, D2, D7[2]
	VMLAL.S16 Q11, D3, D6[0]
	VMLAL.S16 Q11, D4, D7[3]
	VMLAL.S16 Q11, D5, D6[1]
	VMOV Q2, Q7
	VMLSL.S16 Q11, D4, D7[1]
	VMLSL.S16 Q11, D5, D6[3]	
	VMOV Q2, Q6	
	//O[4]
	VMULL.S16 Q12, D0, D6[3]
	VMLSL.S16 Q12, D1, D7[1]
	VMLSL.S16 Q12, D2, D6[1]
	VMLAL.S16 Q12, D3, D7[3]
	VMLSL.S16 Q12, D4, D6[0]
	VMLSL.S16 Q12, D5, D7[2]
	VMOV Q2, Q7
	VMLAL.S16 Q12, D4, D6[2]
	VMLAL.S16 Q12, D5, D7[0]
	VMOV Q2, Q6	
	//O[5]
	VMULL.S16 Q13, D0, D6[2]
	VMLSL.S16 Q13, D1, D7[3]
	VMLAL.S16 Q13, D2, D6[3]
	VMLAL.S16 Q13, D3, D6[1]
	VMLSL.S16 Q13, D4, D7[2]
	VMLAL.S16 Q13, D5, D7[0]
	VMOV Q2, Q7
	VMLAL.S16 Q13, D4, D6[0]
	VMLSL.S16 Q13, D5, D7[1]
	VMOV Q2, Q6
	//O[6]
	VMULL.S16 Q14, D0, D6[1]
	VMLSL.S16 Q14, D1, D7[0]
	VMLAL.S16 Q14, D2, D7[3]
	VMLSL.S16 Q14, D3, D7[1]
	VMLAL.S16 Q14, D4, D6[2]
	VMLAL.S16 Q14, D5, D6[0]
	VMOV Q2, Q7
	VMLSL.S16 Q14, D4, D6[3]
	VMLAL.S16 Q14, D5, D7[2]
	VMOV Q2, Q6
	//O[7]
	VMULL.S16 Q15, D0, D6[0]
	VMLSL.S16 Q15, D1, D6[1]
	VMLAL.S16 Q15, D2, D6[2]
	VMLSL.S16 Q15, D3, D6[3]
	VMLAL.S16 Q15, D4, D7[0]
	VMLSL.S16 Q15, D5, D7[1]
	VMOV Q2, Q7
	VMLAL.S16 Q15, D4, D7[2]
	VMLSL.S16 Q15, D5, D7[3]
	
	VPUSH {D24-D31}              //LACK OF REGISTERS, SO STORE O[4]-D[7]
	////////////////////////////////////////////////////////////////////
	ADD SP, SP, #64
	VPOP {D24-D31}  //Q[12] - Q[15] : E[0] - E[3]
	VADD.S32 Q0, Q8, Q12   //DST[0]
	VADD.S32 Q1, Q9, Q13   //DST[1]
	VADD.S32 Q2, Q10, Q14  //DST[2]
	VADD.S32 Q3, Q11, Q15  //DST[3]
	VSUB.S32 Q7, Q12, Q8   //DST[15]
	VSUB.S32 Q6, Q13, Q9   //DST[14]
	VSUB.S32 Q5, Q14, Q10  //DST[13]
	VSUB.S32 Q4, Q15, Q11  //DST[12]
	
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	VQRSHRN.S32 D8, Q4, #5
	VQRSHRN.S32 D9, Q5, #5
	VQRSHRN.S32 D10, Q6, #5
	VQRSHRN.S32 D11, Q7, #5
	VTRN.32 D8, D10
	VTRN.32 D9, D11
	VTRN.16 D8, D9
	VTRN.16 D10, D11
	
	MOV R9, #32
	MOV R1, R11
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1]
	ADD R1, R11, #24
	VST1.64 D8, [R1], R9
	VST1.64 D9, [R1], R9
	VST1.64 D10, [R1], R9
	VST1.64 D11, [R1]
	////////////////////////////////////////////////////////////////////
	VPOP {D24-D31}         //E[4] - E[7]
	SUB SP, SP, #192
	VPOP {D16-D23}         //O[4] - O[7]

	VADD.S32 Q0, Q8, Q12   //DST[4]
	VADD.S32 Q1, Q9, Q13   //DST[5]
	VADD.S32 Q2, Q10, Q14  //DST[6]
	VADD.S32 Q3, Q11, Q15  //DST[7]
	VSUB.S32 Q7, Q12, Q8   //DST[11]
	VSUB.S32 Q6, Q13, Q9   //DST[10]
	VSUB.S32 Q5, Q14, Q10  //DST[9]
	VSUB.S32 Q4, Q15, Q11  //DST[8]
	
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D2, Q1, #5
	VQRSHRN.S32 D4, Q2, #5
	VQRSHRN.S32 D6, Q3, #5
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	
	VQRSHRN.S32 D1, Q4, #5
	VQRSHRN.S32 D3, Q5, #5
	VQRSHRN.S32 D5, Q6, #5
	VQRSHRN.S32 D7, Q7, #5
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7
	
	MOV R9, #32
	ADD R1, R11, #8
	VST1.64 {D0, D1}, [R1], R9
	VST1.64 {D2, D3}, [R1], R9
	VST1.64 {D4, D5}, [R1], R9
	VST1.64 {D6, D7}, [R1]

	ADD SP, SP, #128
	////////////////////////////////////////////////////////////////	
.endm

//************************************************************************
//macro: the second inverse transform of 16
//************************************************************************
.macro partButterflyInverse16_4rows_shift12_clipx

	//CALCULATE EO
	ADD R0, R10, #64
	MOV R9, #128
	
	VLD1.64 D0, [R0], R9     //src[ 2*line]
	VLD1.64 D1, [R0], R9     //src[ 6*line]
	VLD1.64 D2, [R0], R9     //src[10*line]
	VLD1.64 D3, [R0], R9     //src[14*line]
	
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D7, R4, R5  //D7[0]=9, D7[1]=25, D7[2]=38, D7[3]=44
	
	VMULL.S16 Q12, D0, D7[3]
	VMLAL.S16 Q12, D1, D7[2]
	VMLAL.S16 Q12, D2, D7[1]
	VMLAL.S16 Q12, D3, D7[0]    //EO[0]
	
	VMULL.S16 Q13, D0, D7[2]
	VMLSL.S16 Q13, D1, D7[0]
	VMLSL.S16 Q13, D2, D7[3]
	VMLSL.S16 Q13, D3, D7[1]    //EO[1]
	
	VMULL.S16 Q14, D0, D7[1]
	VMLSL.S16 Q14, D1, D7[3]
	VMLAL.S16 Q14, D2, D7[0]
	VMLAL.S16 Q14, D3, D7[2]    //EO[2]
		
	VMULL.S16 Q15, D0, D7[0]
	VMLSL.S16 Q15, D1, D7[1]
	VMLAL.S16 Q15, D2, D7[2]
	VMLSL.S16 Q15, D3, D7[3]    //EO[3]
	
	//CALCULATE EE
	MOV R0, R10
	VLD1.64 D0, [R0], R9      //src[ 0      ]
	VLD1.64 D1, [R0], R9      //src[ 4*line ]
	VLD1.64 D2, [R0], R9      //src[ 8*line  ]
	VLD1.64 D3, [R0], R9      //src[ 12*line ]
	
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D4, R4, R5  //D4[0]=32, D4[1]=32, D4[2]=17, D4[3]=42

	VMULL.S16 Q4, D1, D4[3]
	VMLAL.S16 Q4, D3, D4[2]     //EEO[0]
	VMULL.S16 Q5, D0, D4[0]
	VMLAL.S16 Q5, D2, D4[0]     //EEE[0]
	VMULL.S16 Q6, D1, D4[2]
	VMLSL.S16 Q6, D3, D4[3]     //EEO[1]
	VMULL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D2, D4[0]     //EEE[1]
	
	VADD.S32 Q8, Q5, Q4         //EE[0]
	VADD.S32 Q9, Q6, Q7         //EE[1]
	VSUB.S32 Q10, Q7, Q6        //EE[2]
	VSUB.S32 Q11, Q5, Q4        //EE[3]
	////////////////////////////////////////////////////////////////////
	VADD.S32 Q0, Q8, Q12        //E[0]
	VADD.S32 Q1, Q9, Q13        //E[1]
	VADD.S32 Q2, Q10, Q14       //E[2]
	VADD.S32 Q3, Q11, Q15       //E[3]
	VSUB.S32 Q4, Q11, Q15       //E[4]
	VSUB.S32 Q5, Q10, Q14       //E[5]
	VSUB.S32 Q6, Q9, Q13        //E[6]
	VSUB.S32 Q7, Q8, Q12        //E[7]
	
	VPUSH {D0-D15}              //PUSH E[0]-E[7]
	////////////////////////////////////////////////////////////////////
	
	ADD R0, R10, #32
	MOV R9, #64
	
	VLD1.64 D0, [R0], R9        //src[   line]
	VLD1.64 D1, [R0], R9        //src[ 3*line]
	VLD1.64 D2, [R0], R9        //src[ 5*line]
	VLD1.64 D3, [R0], R9        //src[ 7*line]
	VLD1.64 D4, [R0], R9        //src[ 9*line]
	VLD1.64 D5, [R0], R9        //src[11*line]
	VLD1.64 D6, [R0], R9        //src[13*line]
	VLD1.64 D7, [R0], R9        //src[15*line]
	//SAVE 
	VMOV Q6, Q2
	VMOV Q7, Q3
	
	//LOAD G_AIT16  1 3 5 7 9 11 13 15
	MOV R7, #4
	MOVT R7, #13
	MOV R3, #21
	MOVT R3, #29
	MOV R4, #35
	MOVT R4, #40
	MOV R5, #43
	MOVT R5, #45
	VMOV D6, R7, R3 //D6[0]=4 ,D6[1]=13,D6[2]=21,D6[3]=29
	VMOV D7, R4, R5 //D7[0]=35,D7[1]=40,D7[2]=43,D7[3]=45
		
	//O[0]
	VMULL.S16 Q8, D0, D7[3]
	VMLAL.S16 Q8, D1, D7[2]
	VMLAL.S16 Q8, D2, D7[1]
	VMLAL.S16 Q8, D3, D7[0]
	VMLAL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q7
	VMLAL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[0]
	VMOV Q2, Q6
	//O[1]
	VMULL.S16 Q9, D0, D7[2]
	VMLAL.S16 Q9, D1, D6[3]
	VMLAL.S16 Q9, D2, D6[0]
	VMLSL.S16 Q9, D3, D6[2]
	VMLSL.S16 Q9, D4, D7[1]
	VMLSL.S16 Q9, D5, D7[3]
	VMOV Q2, Q7
	VMLSL.S16 Q9, D4, D7[0]
	VMLSL.S16 Q9, D5, D6[1]
	VMOV Q2, Q6
	//O[2]
	VMULL.S16 Q10, D0, D7[1]
	VMLAL.S16 Q10, D1, D6[0]
	VMLSL.S16 Q10, D2, D7[0]
	VMLSL.S16 Q10, D3, D7[2]
	VMLSL.S16 Q10, D4, D6[1]
	VMLAL.S16 Q10, D5, D6[3]
	VMOV Q2, Q7
	VMLAL.S16 Q10, D4, D7[3]
	VMLAL.S16 Q10, D5, D6[2]
	VMOV Q2, Q6
	//O[3]
	VMULL.S16 Q11, D0, D7[0]
	VMLSL.S16 Q11, D1, D6[2]
	VMLSL.S16 Q11, D2, D7[2]
	VMLAL.S16 Q11, D3, D6[0]
	VMLAL.S16 Q11, D4, D7[3]
	VMLAL.S16 Q11, D5, D6[1]
	VMOV Q2, Q7
	VMLSL.S16 Q11, D4, D7[1]
	VMLSL.S16 Q11, D5, D6[3]	
	VMOV Q2, Q6	
	//O[4]
	VMULL.S16 Q12, D0, D6[3]
	VMLSL.S16 Q12, D1, D7[1]
	VMLSL.S16 Q12, D2, D6[1]
	VMLAL.S16 Q12, D3, D7[3]
	VMLSL.S16 Q12, D4, D6[0]
	VMLSL.S16 Q12, D5, D7[2]
	VMOV Q2, Q7
	VMLAL.S16 Q12, D4, D6[2]
	VMLAL.S16 Q12, D5, D7[0]
	VMOV Q2, Q6	
	//O[5]
	VMULL.S16 Q13, D0, D6[2]
	VMLSL.S16 Q13, D1, D7[3]
	VMLAL.S16 Q13, D2, D6[3]
	VMLAL.S16 Q13, D3, D6[1]
	VMLSL.S16 Q13, D4, D7[2]
	VMLAL.S16 Q13, D5, D7[0]
	VMOV Q2, Q7
	VMLAL.S16 Q13, D4, D6[0]
	VMLSL.S16 Q13, D5, D7[1]
	VMOV Q2, Q6
	//O[6]
	VMULL.S16 Q14, D0, D6[1]
	VMLSL.S16 Q14, D1, D7[0]
	VMLAL.S16 Q14, D2, D7[3]
	VMLSL.S16 Q14, D3, D7[1]
	VMLAL.S16 Q14, D4, D6[2]
	VMLAL.S16 Q14, D5, D6[0]
	VMOV Q2, Q7
	VMLSL.S16 Q14, D4, D6[3]
	VMLAL.S16 Q14, D5, D7[2]
	VMOV Q2, Q6
	//O[7]
	VMULL.S16 Q15, D0, D6[0]
	VMLSL.S16 Q15, D1, D6[1]
	VMLAL.S16 Q15, D2, D6[2]
	VMLSL.S16 Q15, D3, D6[3]
	VMLAL.S16 Q15, D4, D7[0]
	VMLSL.S16 Q15, D5, D7[1]
	VMOV Q2, Q7
	VMLAL.S16 Q15, D4, D7[2]
	VMLSL.S16 Q15, D5, D7[3]
	
	VPUSH {D24-D31}              //LACK OF REGISTERS, SO STORE O[4]-D[7]
	////////////////////////////////////////////////////////////////////
	ADD SP, SP, #64
	VPOP {D24-D31}  //Q[12] - Q[15] : E[0] - E[3]
	VADD.S32 Q0, Q8, Q12   //DST[0]
	VADD.S32 Q1, Q9, Q13   //DST[1]
	VADD.S32 Q2, Q10, Q14  //DST[2]
	VADD.S32 Q3, Q11, Q15  //DST[3]
	VSUB.S32 Q7, Q12, Q8   //DST[15]
	VSUB.S32 Q6, Q13, Q9   //DST[14]
	VSUB.S32 Q5, Q14, Q10  //DST[13]
	VSUB.S32 Q4, Q15, Q11  //DST[12]
	
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	VQRSHRN.S32 D8, Q4, #12
	VQRSHRN.S32 D9, Q5, #12
	VQRSHRN.S32 D10, Q6, #12
	VQRSHRN.S32 D11, Q7, #12
	VTRN.32 D8, D10
	VTRN.32 D9, D11
	VTRN.16 D8, D9
	VTRN.16 D10, D11
	
	////////////////////////////////////////////////////////////////////
	VPOP {D24-D31}         //E[4] - E[7]
	SUB SP, SP, #192
	VPOP {D16-D23}         //O[4] - O[7]

	VADD.S32 Q2, Q8, Q12   //DST[4]
	VADD.S32 Q3, Q9, Q13   //DST[5]
	VADD.S32 Q6, Q10, Q14  //DST[6]
	VADD.S32 Q7, Q11, Q15  //DST[7]
	VSUB.S32 Q12, Q12, Q8   //DST[11]
	VSUB.S32 Q13, Q13, Q9   //DST[10]
	VSUB.S32 Q14, Q14, Q10  //DST[9]
	VSUB.S32 Q15, Q15, Q11  //DST[8]
	
	//STORE TO Q6, Q7, Q8, Q9
	VQRSHRN.S32 D16, Q6, #12
	VQRSHRN.S32 D18, Q7, #12
	VQRSHRN.S32 D12, Q2, #12
	VQRSHRN.S32 D14, Q3, #12
	VTRN.32 D12, D16
	VTRN.32 D14, D18
	VTRN.16 D12, D14
	VTRN.16 D16, D18
	
	VQRSHRN.S32 D13, Q15, #12
	VQRSHRN.S32 D15, Q14, #12
	VQRSHRN.S32 D17, Q13, #12
	VQRSHRN.S32 D19, Q12, #12
	VTRN.32 D13, D17
	VTRN.32 D15, D19
	VTRN.16 D13, D15
	VTRN.16 D17, D19
	
	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q2, R5	//minval vector Q5
	VDUP.16 Q3, R6  //maxval vector Q6

	VMAX.S16 Q0, Q0, Q2
	VMAX.S16 Q1, Q1, Q2
	VMAX.S16 Q4, Q4, Q2
	VMAX.S16 Q5, Q5, Q2
	VMAX.S16 Q6, Q6, Q2
	VMAX.S16 Q7, Q7, Q2
	VMAX.S16 Q8, Q8, Q2
	VMAX.S16 Q9, Q9, Q2

	VMIN.S16 Q0, Q0, Q3
	VMIN.S16 Q1, Q1, Q3
	VMIN.S16 Q4, Q4, Q3
	VMIN.S16 Q5, Q5, Q3
	VMIN.S16 Q6, Q6, Q3
	VMIN.S16 Q7, Q7, Q3
	VMIN.S16 Q8, Q8, Q3
	VMIN.S16 Q9, Q9, Q3


	//D0, D12, D13, D8
	//D1, D14, D15, D9
	//D2, D16, D17, D10
	//D3, D18, D19, D11
	MOV R9, #32
	MOV R1, R11	
	
	//THE FISRT ROW
	VMOV.S16 D20,D0
	VMOV.S16 D21,D12
	VMOV.S16 D22,D13
	VMOV.S16 D23,D8
	VST1.64 {D20, D21, D22, D23}, [R1], R9
	
	//THE SECOND ROW
	VMOV.S16 D20,D1
	VMOV.S16 D21,D14
	VMOV.S16 D22,D15
	VMOV.S16 D23,D9
	VST1.64 {D20, D21, D22, D23}, [R1], R9
	
	//THE THIRD ROW
	VMOV.S16 D20,D2
	VMOV.S16 D21,D16
	VMOV.S16 D22,D17
	VMOV.S16 D23,D10
	VST1.64 {D20, D21, D22, D23}, [R1], R9
	
	//THE FOURTH ROW
	VMOV.S16 D20,D3
	VMOV.S16 D21,D18
	VMOV.S16 D22,D19
	VMOV.S16 D23,D11
	VST1.64 {D20, D21, D22, D23}, [R1]
	
	ADD SP, SP, #128
	////////////////////////////////////////////////////////////////	
.endm

//***********************************************************************
//void partialButterflyInverse16_shift5_clip16_neon(Short *src,Short *dst)
//src: transform coefficients
//dst: the result   
//detail: shift=5 clip=16
//***********************************************************************
function partialButterflyInverse16_shift5_clip16_neon

	PUSH {R4-R12, LR}
	MOV R10, R0
	MOV R11, R1
	///////////////////FIRST  4 ROWS///////////////////
	
	partButterflyInverse16_4rows_shift5_clip16
	
	///////////////////SECOND 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #128
	partButterflyInverse16_4rows_shift5_clip16
	
	///////////////////THIRD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #128
	partButterflyInverse16_4rows_shift5_clip16
	
	///////////////////FOURTH 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #128
	partButterflyInverse16_4rows_shift5_clip16
	
	////////////////////////   END  ///////////////////	
	
	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void partialButterflyInverse16_shift12_clipx_neon(coef_t *blk, int shift, int clip)
//src: transform coefficients
//dst: the result   
//detail: shift=12 clip=x
//***********************************************************************
function partialButterflyInverse16_shift12_clipx_neon

	PUSH {R4-R12, LR}
	MOV R10, R0
	MOV R11, R1
	SUB R2, R2, #1
	///////////////////FIRST  4 ROWS///////////////////
	
	partButterflyInverse16_4rows_shift12_clipX
	
	///////////////////SECOND 4 ROWS///////////////////
	ADD R10, R10, #8
	ADD R11, R11, #128
	
	partButterflyInverse16_4rows_shift12_clipX
	
	///////////////////THIRD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #128
	partButterflyInverse16_4rows_shift12_clipX
	
	///////////////////FOURTH 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #128
	partButterflyInverse16_4rows_shift12_clipX
	
	////////////////////////   END  ///////////////////	
	
	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void idct_16x16_shift12_neon(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 12 in code  
//clip: clip
//***********************************************************************
function idct_16x16_shift12_neon
	PUSH {R4-R12, LR}

	////////////////////alloc mem ///////////////////

	SUB R12, SP, #960

	MOV R8, R0
	MOV R1, R12

	//////////////////////////////////////////////////
	///////////////////first trans////////////////////
	//////////////////////////////////////////////////

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////FIRST  4 ROWS///////////////////
	
	partButterflyInverse16_4rows_shift5_clip16
	
	///////////////////SECOND 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #128
	partButterflyInverse16_4rows_shift5_clip16
	
	///////////////////THIRD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #128
	partButterflyInverse16_4rows_shift5_clip16
	
	///////////////////FOURTH 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #128
	partButterflyInverse16_4rows_shift5_clip16
	
	//////////////////////////////////////////////////
	///////////////////second trans///////////////////
	//////////////////////////////////////////////////

	MOV R0, R12
	MOV R1, R8
	SUB R2, R2, #1
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 
	///////////////////FIRST  4 ROWS///////////////////
	
	partButterflyInverse16_4rows_shift12_clipX
	
	///////////////////SECOND 4 ROWS///////////////////
	ADD R10, R10, #8
	ADD R11, R11, #128
	
	partButterflyInverse16_4rows_shift12_clipX
	
	///////////////////THIRD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #128
	partButterflyInverse16_4rows_shift12_clipX
	
	///////////////////FOURTH 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #128
	partButterflyInverse16_4rows_shift12_clipX
	
	////////////////////////   END  ///////////////////	
	
	POP {R4-R12, PC} 
.endfunc


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


//************************************************************************
//macro: the first inverse transform of 4x16
//************************************************************************
.macro partButterflyInverse16x4_4rows_shift5_clip16

	//CALCULATE EO
	ADD R0, R10, #16
	MOV R9, #32
	
	VLD1.64 D0, [R0], R9     //src[ 2*line]
	VLD1.64 D1, [R0], R9     //src[ 6*line]
	VLD1.64 D2, [R0], R9     //src[10*line]
	VLD1.64 D3, [R0], R9     //src[14*line]
	
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D7, R4, R5  //D7[0]=9, D7[1]=25, D7[2]=38, D7[3]=44
	
	VMULL.S16 Q12, D0, D7[3]
	VMLAL.S16 Q12, D1, D7[2]
	VMLAL.S16 Q12, D2, D7[1]
	VMLAL.S16 Q12, D3, D7[0]    //EO[0]
	
	VMULL.S16 Q13, D0, D7[2]
	VMLSL.S16 Q13, D1, D7[0]
	VMLSL.S16 Q13, D2, D7[3]
	VMLSL.S16 Q13, D3, D7[1]    //EO[1]
	
	VMULL.S16 Q14, D0, D7[1]
	VMLSL.S16 Q14, D1, D7[3]
	VMLAL.S16 Q14, D2, D7[0]
	VMLAL.S16 Q14, D3, D7[2]    //EO[2]
		
	VMULL.S16 Q15, D0, D7[0]
	VMLSL.S16 Q15, D1, D7[1]
	VMLAL.S16 Q15, D2, D7[2]
	VMLSL.S16 Q15, D3, D7[3]    //EO[3]
	
	//CALCULATE EE
	MOV R0, R10
	VLD1.64 D0, [R0], R9      //src[ 0      ]
	VLD1.64 D1, [R0], R9      //src[ 4*line ]
	VLD1.64 D2, [R0], R9      //src[ 8*line  ]
	VLD1.64 D3, [R0], R9      //src[ 12*line ]
	
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D4, R4, R5  //D4[0]=32, D4[1]=32, D4[2]=17, D4[3]=42

	VMULL.S16 Q4, D1, D4[3]
	VMLAL.S16 Q4, D3, D4[2]     //EEO[0]
	VMULL.S16 Q5, D0, D4[0]
	VMLAL.S16 Q5, D2, D4[0]     //EEE[0]
	VMULL.S16 Q6, D1, D4[2]
	VMLSL.S16 Q6, D3, D4[3]     //EEO[1]
	VMULL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D2, D4[0]     //EEE[1]
	
	VADD.S32 Q8, Q5, Q4         //EE[0]
	VADD.S32 Q9, Q6, Q7         //EE[1]
	VSUB.S32 Q10, Q7, Q6        //EE[2]
	VSUB.S32 Q11, Q5, Q4        //EE[3]
	////////////////////////////////////////////////////////////////////
	VADD.S32 Q0, Q8, Q12        //E[0]
	VADD.S32 Q1, Q9, Q13        //E[1]
	VADD.S32 Q2, Q10, Q14       //E[2]
	VADD.S32 Q3, Q11, Q15       //E[3]
	VSUB.S32 Q4, Q11, Q15       //E[4]
	VSUB.S32 Q5, Q10, Q14       //E[5]
	VSUB.S32 Q6, Q9, Q13        //E[6]
	VSUB.S32 Q7, Q8, Q12        //E[7]
	
	VPUSH {D0-D15}              //PUSH E[0]-E[7]
	////////////////////////////////////////////////////////////////////
	
	ADD R0, R10, #8
	MOV R9, #16
	
	VLD1.64 D0, [R0], R9        //src[   line]
	VLD1.64 D1, [R0], R9        //src[ 3*line]
	VLD1.64 D2, [R0], R9        //src[ 5*line]
	VLD1.64 D3, [R0], R9        //src[ 7*line]
	VLD1.64 D4, [R0], R9        //src[ 9*line]
	VLD1.64 D5, [R0], R9        //src[11*line]
	VLD1.64 D6, [R0], R9        //src[13*line]
	VLD1.64 D7, [R0], R9        //src[15*line]
	//SAVE 
	VMOV Q6, Q2
	VMOV Q7, Q3
	
	//LOAD G_AIT16  1 3 5 7 9 11 13 15
	MOV R7, #4
	MOVT R7, #13
	MOV R3, #21
	MOVT R3, #29
	MOV R4, #35
	MOVT R4, #40
	MOV R5, #43
	MOVT R5, #45
	VMOV D6, R7, R3 //D6[0]=4 ,D6[1]=13,D6[2]=21,D6[3]=29
	VMOV D7, R4, R5 //D7[0]=35,D7[1]=40,D7[2]=43,D7[3]=45
		
	//O[0]
	VMULL.S16 Q8, D0, D7[3]
	VMLAL.S16 Q8, D1, D7[2]
	VMLAL.S16 Q8, D2, D7[1]
	VMLAL.S16 Q8, D3, D7[0]
	VMLAL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q7
	VMLAL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[0]
	VMOV Q2, Q6
	//O[1]
	VMULL.S16 Q9, D0, D7[2]
	VMLAL.S16 Q9, D1, D6[3]
	VMLAL.S16 Q9, D2, D6[0]
	VMLSL.S16 Q9, D3, D6[2]
	VMLSL.S16 Q9, D4, D7[1]
	VMLSL.S16 Q9, D5, D7[3]
	VMOV Q2, Q7
	VMLSL.S16 Q9, D4, D7[0]
	VMLSL.S16 Q9, D5, D6[1]
	VMOV Q2, Q6
	//O[2]
	VMULL.S16 Q10, D0, D7[1]
	VMLAL.S16 Q10, D1, D6[0]
	VMLSL.S16 Q10, D2, D7[0]
	VMLSL.S16 Q10, D3, D7[2]
	VMLSL.S16 Q10, D4, D6[1]
	VMLAL.S16 Q10, D5, D6[3]
	VMOV Q2, Q7
	VMLAL.S16 Q10, D4, D7[3]
	VMLAL.S16 Q10, D5, D6[2]
	VMOV Q2, Q6
	//O[3]
	VMULL.S16 Q11, D0, D7[0]
	VMLSL.S16 Q11, D1, D6[2]
	VMLSL.S16 Q11, D2, D7[2]
	VMLAL.S16 Q11, D3, D6[0]
	VMLAL.S16 Q11, D4, D7[3]
	VMLAL.S16 Q11, D5, D6[1]
	VMOV Q2, Q7
	VMLSL.S16 Q11, D4, D7[1]
	VMLSL.S16 Q11, D5, D6[3]	
	VMOV Q2, Q6	
	//O[4]
	VMULL.S16 Q12, D0, D6[3]
	VMLSL.S16 Q12, D1, D7[1]
	VMLSL.S16 Q12, D2, D6[1]
	VMLAL.S16 Q12, D3, D7[3]
	VMLSL.S16 Q12, D4, D6[0]
	VMLSL.S16 Q12, D5, D7[2]
	VMOV Q2, Q7
	VMLAL.S16 Q12, D4, D6[2]
	VMLAL.S16 Q12, D5, D7[0]
	VMOV Q2, Q6	
	//O[5]
	VMULL.S16 Q13, D0, D6[2]
	VMLSL.S16 Q13, D1, D7[3]
	VMLAL.S16 Q13, D2, D6[3]
	VMLAL.S16 Q13, D3, D6[1]
	VMLSL.S16 Q13, D4, D7[2]
	VMLAL.S16 Q13, D5, D7[0]
	VMOV Q2, Q7
	VMLAL.S16 Q13, D4, D6[0]
	VMLSL.S16 Q13, D5, D7[1]
	VMOV Q2, Q6
	//O[6]
	VMULL.S16 Q14, D0, D6[1]
	VMLSL.S16 Q14, D1, D7[0]
	VMLAL.S16 Q14, D2, D7[3]
	VMLSL.S16 Q14, D3, D7[1]
	VMLAL.S16 Q14, D4, D6[2]
	VMLAL.S16 Q14, D5, D6[0]
	VMOV Q2, Q7
	VMLSL.S16 Q14, D4, D6[3]
	VMLAL.S16 Q14, D5, D7[2]
	VMOV Q2, Q6
	//O[7]
	VMULL.S16 Q15, D0, D6[0]
	VMLSL.S16 Q15, D1, D6[1]
	VMLAL.S16 Q15, D2, D6[2]
	VMLSL.S16 Q15, D3, D6[3]
	VMLAL.S16 Q15, D4, D7[0]
	VMLSL.S16 Q15, D5, D7[1]
	VMOV Q2, Q7
	VMLAL.S16 Q15, D4, D7[2]
	VMLSL.S16 Q15, D5, D7[3]
	
	VPUSH {D24-D31}              //LACK OF REGISTERS, SO STORE O[4]-D[7]
	////////////////////////////////////////////////////////////////////
	ADD SP, SP, #64
	VPOP {D24-D31}  //Q[12] - Q[15] : E[0] - E[3]
	VADD.S32 Q0, Q8, Q12   //DST[0]
	VADD.S32 Q1, Q9, Q13   //DST[1]
	VADD.S32 Q2, Q10, Q14  //DST[2]
	VADD.S32 Q3, Q11, Q15  //DST[3]
	VSUB.S32 Q7, Q12, Q8   //DST[15]
	VSUB.S32 Q6, Q13, Q9   //DST[14]
	VSUB.S32 Q5, Q14, Q10  //DST[13]
	VSUB.S32 Q4, Q15, Q11  //DST[12]
	
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	VQRSHRN.S32 D8, Q4, #5
	VQRSHRN.S32 D9, Q5, #5
	VQRSHRN.S32 D10, Q6, #5
	VQRSHRN.S32 D11, Q7, #5
	VTRN.32 D8, D10
	VTRN.32 D9, D11
	VTRN.16 D8, D9
	VTRN.16 D10, D11
	
	MOV R9, #32
	MOV R1, R11
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1]
	ADD R1, R11, #24
	VST1.64 D8, [R1], R9
	VST1.64 D9, [R1], R9
	VST1.64 D10, [R1], R9
	VST1.64 D11, [R1]
	////////////////////////////////////////////////////////////////////
	VPOP {D24-D31}         //E[4] - E[7]
	SUB SP, SP, #192
	VPOP {D16-D23}         //O[4] - O[7]

	VADD.S32 Q0, Q8, Q12   //DST[4]
	VADD.S32 Q1, Q9, Q13   //DST[5]
	VADD.S32 Q2, Q10, Q14  //DST[6]
	VADD.S32 Q3, Q11, Q15  //DST[7]
	VSUB.S32 Q7, Q12, Q8   //DST[11]
	VSUB.S32 Q6, Q13, Q9   //DST[10]
	VSUB.S32 Q5, Q14, Q10  //DST[9]
	VSUB.S32 Q4, Q15, Q11  //DST[8]
	
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D2, Q1, #5
	VQRSHRN.S32 D4, Q2, #5
	VQRSHRN.S32 D6, Q3, #5
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	
	VQRSHRN.S32 D1, Q4, #5
	VQRSHRN.S32 D3, Q5, #5
	VQRSHRN.S32 D5, Q6, #5
	VQRSHRN.S32 D7, Q7, #5
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7
	
	MOV R9, #32
	ADD R1, R11, #8
	VST1.64 {D0, D1}, [R1], R9
	VST1.64 {D2, D3}, [R1], R9
	VST1.64 {D4, D5}, [R1], R9
	VST1.64 {D6, D7}, [R1]

	ADD SP, SP, #128
	////////////////////////////////////////////////////////////////	
.endm

//************************************************************************
//macro: the second inverse transform of 4x16
//************************************************************************
.macro partButterflyInverse4x16_4rows_shift12_clipx

	MOV R0, R10
	MOV R9, #32

	VLD1.64 D0, [R0], R9     //src[ 1*line]
	VLD1.64 D1, [R0], R9     //src[ 2*line]
	VLD1.64 D2, [R0], R9     //src[ 3*line]
	VLD1.64 D3, [R0], R9     //src[ 4*line]
		
	MOV R5, #32
	MOVT R5, #32 
	MOV R6, #42	
	MOVT R6, #17	
	VMOV D4, R5, R6 //D4[0]=32, D4[1]=32,D4[2]=42, D4[3]=17
	
	//O[0]
	VMULL.S16 Q8, D1, D4[2]
	VMLAL.S16 Q8, D3, D4[3]
	//O[1]
	VMULL.S16 Q9, D1, D4[3]
	VMLSL.S16 Q9, D3, D4[2]
	//E[0]
	VMULL.S16 Q10, D0, D4[0]
	VMLAL.S16 Q10, D2, D4[0]
	//E[1]
	VMULL.S16 Q11, D0, D4[0]
	VMLSL.S16 Q11, D2, D4[0]
	
	VQADD.S32 Q12, Q8, Q10
	VQRSHRN.S32 D0, Q12, #12
	
	VQADD.S32 Q13, Q11, Q9
	VQRSHRN.S32 D1, Q13, #12
	
	VQSUB.S32 Q14, Q11, Q9
	VQRSHRN.S32 D2, Q14, #12
	
	VQSUB.S32 Q15, Q10, Q8
	VQRSHRN.S32 D3, Q15, #12
	
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3	

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6

	MOV R1, R11
	VST1.64 D0, [R1]!
	VST1.64 D1, [R1]!
	VST1.64 D2, [R1]!
	VST1.64 D3, [R1]!
.endm

//***********************************************************************
//void partialButterflyInverse16x4_shift5_clip16_neon(Short *src,Short *dst)
//src: transform coefficients
//dst: the result   
//detail: shift=5 clip=16
//***********************************************************************
function partialButterflyInverse16x4_shift5_clip16_neon

	PUSH {R4-R12, LR}
	MOV R10, R0
	MOV R11, R1
	///////////////////FIRST  4 ROWS///////////////////
	
	partButterflyInverse16x4_4rows_shift5_clip16
		
	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void partialButterflyInverse4x16_shift12_clipx_neon(coef_t *blk, int shift, int clip)
//src: transform coefficients
//dst: the result   
//detail: shift=12 clip=x
//***********************************************************************
function partialButterflyInverse4x16_shift12_clipx_neon
	
	PUSH {R4-R12, LR}	
	SUB R2, R2, #1
	MOV R10, R0
	MOV R11, R1
	///////////////////FIRST  4 ROWS///////////////////

	partButterflyInverse4x16_4rows_shift12_clipx


	///////////////////SECOND 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift12_clipx
	
	///////////////////THIRD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift12_clipx
	
	///////////////////FOURTH 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift12_clipx
	
	////////////////////////   END  ///////////////////	

	POP {R4-R12, PC}	   
.endfunc

//***********************************************************************
//void idct_4x16_shift12_neon(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 12 in code  
//clip: clip
//***********************************************************************
function idct_4x16_shift12_neon
	PUSH {R4-R12, LR}

	////////////////////alloc mem ///////////////////

	SUB R12, SP, #480

	MOV R8, R0
	MOV R1, R12

	//////////////////////////////////////////////////
	///////////////////first trans////////////////////
	//////////////////////////////////////////////////

	MOV R10, R0
	MOV R11, R1
	///////////////////FIRST  4 ROWS///////////////////
	
	partButterflyInverse16x4_4rows_shift5_clip16


	//////////////////////////////////////////////////
	///////////////////second trans///////////////////
	//////////////////////////////////////////////////

	MOV R0, R12
	MOV R1, R8
	SUB R2, R2, #1
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////FIRST  4 ROWS///////////////////

	partButterflyInverse4x16_4rows_shift12_clipx


	///////////////////SECOND 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift12_clipx
	
	///////////////////THIRD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift12_clipx
	
	///////////////////FOURTH 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift12_clipx
	
	////////////////////////   END  ///////////////////	

	POP {R4-R12, PC}	   
.endfunc


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


//************************************************************************
//macro: the first inverse transform of 16x4
//************************************************************************
.macro partButterflyInverse4x16_4rows_shift5_clip16
	
	MOV R0, R10
	MOV R9, #32

	VLD1.64 D0, [R0], R9     //src[ 1*line]
	VLD1.64 D1, [R0], R9     //src[ 2*line]
	VLD1.64 D2, [R0], R9     //src[ 3*line]
	VLD1.64 D3, [R0], R9     //src[ 4*line]
		
	MOV R5, #32
	MOVT R5, #32 
	MOV R6, #42	
	MOVT R6, #17	
	VMOV D4, R5, R6 //D4[0]=32, D4[1]=32,D4[2]=42, D4[3]=17
	
	//O[0]
	VMULL.S16 Q8, D1, D4[2]
	VMLAL.S16 Q8, D3, D4[3]
	//O[1]
	VMULL.S16 Q9, D1, D4[3]
	VMLSL.S16 Q9, D3, D4[2]
	//E[0]
	VMULL.S16 Q10, D0, D4[0]
	VMLAL.S16 Q10, D2, D4[0]
	//E[1]
	VMULL.S16 Q11, D0, D4[0]
	VMLSL.S16 Q11, D2, D4[0]
	
	VQADD.S32 Q12, Q8, Q10
	VQRSHRN.S32 D0, Q12, #5
	
	VQADD.S32 Q13, Q11, Q9
	VQRSHRN.S32 D1, Q13, #5
	
	VQSUB.S32 Q14, Q11, Q9
	VQRSHRN.S32 D2, Q14, #5
	
	VQSUB.S32 Q15, Q10, Q8
	VQRSHRN.S32 D3, Q15, #5
	
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3	

	MOV R1, R11
	VST1.64 D0, [R1]!
	VST1.64 D1, [R1]!
	VST1.64 D2, [R1]!
	VST1.64 D3, [R1]!

.endm

//************************************************************************
//macro: the second inverse transform of 16x4
//************************************************************************
.macro partButterflyInverse16x4_4rows_shift12_clipx
	//CALCULATE EO
	ADD R0, R10, #16
	MOV R9, #32
	
	VLD1.64 D0, [R0], R9     //src[ 2*line]
	VLD1.64 D1, [R0], R9     //src[ 6*line]
	VLD1.64 D2, [R0], R9     //src[10*line]
	VLD1.64 D3, [R0], R9     //src[14*line]
	
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D7, R4, R5  //D7[0]=9, D7[1]=25, D7[2]=38, D7[3]=44
	
	VMULL.S16 Q12, D0, D7[3]
	VMLAL.S16 Q12, D1, D7[2]
	VMLAL.S16 Q12, D2, D7[1]
	VMLAL.S16 Q12, D3, D7[0]    //EO[0]
	
	VMULL.S16 Q13, D0, D7[2]
	VMLSL.S16 Q13, D1, D7[0]
	VMLSL.S16 Q13, D2, D7[3]
	VMLSL.S16 Q13, D3, D7[1]    //EO[1]
	
	VMULL.S16 Q14, D0, D7[1]
	VMLSL.S16 Q14, D1, D7[3]
	VMLAL.S16 Q14, D2, D7[0]
	VMLAL.S16 Q14, D3, D7[2]    //EO[2]
		
	VMULL.S16 Q15, D0, D7[0]
	VMLSL.S16 Q15, D1, D7[1]
	VMLAL.S16 Q15, D2, D7[2]
	VMLSL.S16 Q15, D3, D7[3]    //EO[3]
	
	//CALCULATE EE
	MOV R0, R10
	VLD1.64 D0, [R0], R9      //src[ 0      ]
	VLD1.64 D1, [R0], R9      //src[ 4*line ]
	VLD1.64 D2, [R0], R9      //src[ 8*line  ]
	VLD1.64 D3, [R0], R9      //src[ 12*line ]
	
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D4, R4, R5  //D4[0]=32, D4[1]=32, D4[2]=17, D4[3]=42

	VMULL.S16 Q4, D1, D4[3]
	VMLAL.S16 Q4, D3, D4[2]     //EEO[0]
	VMULL.S16 Q5, D0, D4[0]
	VMLAL.S16 Q5, D2, D4[0]     //EEE[0]
	VMULL.S16 Q6, D1, D4[2]
	VMLSL.S16 Q6, D3, D4[3]     //EEO[1]
	VMULL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D2, D4[0]     //EEE[1]
	
	VADD.S32 Q8, Q5, Q4         //EE[0]
	VADD.S32 Q9, Q6, Q7         //EE[1]
	VSUB.S32 Q10, Q7, Q6        //EE[2]
	VSUB.S32 Q11, Q5, Q4        //EE[3]
	////////////////////////////////////////////////////////////////////
	VADD.S32 Q0, Q8, Q12        //E[0]
	VADD.S32 Q1, Q9, Q13        //E[1]
	VADD.S32 Q2, Q10, Q14       //E[2]
	VADD.S32 Q3, Q11, Q15       //E[3]
	VSUB.S32 Q4, Q11, Q15       //E[4]
	VSUB.S32 Q5, Q10, Q14       //E[5]
	VSUB.S32 Q6, Q9, Q13        //E[6]
	VSUB.S32 Q7, Q8, Q12        //E[7]
	
	VPUSH {D0-D15}              //PUSH E[0]-E[7]
	////////////////////////////////////////////////////////////////////
	
	ADD R0, R10, #8
	MOV R9, #16
	
	VLD1.64 D0, [R0], R9        //src[   line]
	VLD1.64 D1, [R0], R9        //src[ 3*line]
	VLD1.64 D2, [R0], R9        //src[ 5*line]
	VLD1.64 D3, [R0], R9        //src[ 7*line]
	VLD1.64 D4, [R0], R9        //src[ 9*line]
	VLD1.64 D5, [R0], R9        //src[11*line]
	VLD1.64 D6, [R0], R9        //src[13*line]
	VLD1.64 D7, [R0], R9        //src[15*line]
	//SAVE 
	VMOV Q6, Q2
	VMOV Q7, Q3
	
	//LOAD G_AIT16  1 3 5 7 9 11 13 15
	MOV R7, #4
	MOVT R7, #13
	MOV R3, #21
	MOVT R3, #29
	MOV R4, #35
	MOVT R4, #40
	MOV R5, #43
	MOVT R5, #45
	VMOV D6, R7, R3 //D6[0]=4 ,D6[1]=13,D6[2]=21,D6[3]=29
	VMOV D7, R4, R5 //D7[0]=35,D7[1]=40,D7[2]=43,D7[3]=45
		
	//O[0]
	VMULL.S16 Q8, D0, D7[3]
	VMLAL.S16 Q8, D1, D7[2]
	VMLAL.S16 Q8, D2, D7[1]
	VMLAL.S16 Q8, D3, D7[0]
	VMLAL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q7
	VMLAL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[0]
	VMOV Q2, Q6
	//O[1]
	VMULL.S16 Q9, D0, D7[2]
	VMLAL.S16 Q9, D1, D6[3]
	VMLAL.S16 Q9, D2, D6[0]
	VMLSL.S16 Q9, D3, D6[2]
	VMLSL.S16 Q9, D4, D7[1]
	VMLSL.S16 Q9, D5, D7[3]
	VMOV Q2, Q7
	VMLSL.S16 Q9, D4, D7[0]
	VMLSL.S16 Q9, D5, D6[1]
	VMOV Q2, Q6
	//O[2]
	VMULL.S16 Q10, D0, D7[1]
	VMLAL.S16 Q10, D1, D6[0]
	VMLSL.S16 Q10, D2, D7[0]
	VMLSL.S16 Q10, D3, D7[2]
	VMLSL.S16 Q10, D4, D6[1]
	VMLAL.S16 Q10, D5, D6[3]
	VMOV Q2, Q7
	VMLAL.S16 Q10, D4, D7[3]
	VMLAL.S16 Q10, D5, D6[2]
	VMOV Q2, Q6
	//O[3]
	VMULL.S16 Q11, D0, D7[0]
	VMLSL.S16 Q11, D1, D6[2]
	VMLSL.S16 Q11, D2, D7[2]
	VMLAL.S16 Q11, D3, D6[0]
	VMLAL.S16 Q11, D4, D7[3]
	VMLAL.S16 Q11, D5, D6[1]
	VMOV Q2, Q7
	VMLSL.S16 Q11, D4, D7[1]
	VMLSL.S16 Q11, D5, D6[3]	
	VMOV Q2, Q6	
	//O[4]
	VMULL.S16 Q12, D0, D6[3]
	VMLSL.S16 Q12, D1, D7[1]
	VMLSL.S16 Q12, D2, D6[1]
	VMLAL.S16 Q12, D3, D7[3]
	VMLSL.S16 Q12, D4, D6[0]
	VMLSL.S16 Q12, D5, D7[2]
	VMOV Q2, Q7
	VMLAL.S16 Q12, D4, D6[2]
	VMLAL.S16 Q12, D5, D7[0]
	VMOV Q2, Q6	
	//O[5]
	VMULL.S16 Q13, D0, D6[2]
	VMLSL.S16 Q13, D1, D7[3]
	VMLAL.S16 Q13, D2, D6[3]
	VMLAL.S16 Q13, D3, D6[1]
	VMLSL.S16 Q13, D4, D7[2]
	VMLAL.S16 Q13, D5, D7[0]
	VMOV Q2, Q7
	VMLAL.S16 Q13, D4, D6[0]
	VMLSL.S16 Q13, D5, D7[1]
	VMOV Q2, Q6
	//O[6]
	VMULL.S16 Q14, D0, D6[1]
	VMLSL.S16 Q14, D1, D7[0]
	VMLAL.S16 Q14, D2, D7[3]
	VMLSL.S16 Q14, D3, D7[1]
	VMLAL.S16 Q14, D4, D6[2]
	VMLAL.S16 Q14, D5, D6[0]
	VMOV Q2, Q7
	VMLSL.S16 Q14, D4, D6[3]
	VMLAL.S16 Q14, D5, D7[2]
	VMOV Q2, Q6
	//O[7]
	VMULL.S16 Q15, D0, D6[0]
	VMLSL.S16 Q15, D1, D6[1]
	VMLAL.S16 Q15, D2, D6[2]
	VMLSL.S16 Q15, D3, D6[3]
	VMLAL.S16 Q15, D4, D7[0]
	VMLSL.S16 Q15, D5, D7[1]
	VMOV Q2, Q7
	VMLAL.S16 Q15, D4, D7[2]
	VMLSL.S16 Q15, D5, D7[3]
	
	VPUSH {D24-D31}              //LACK OF REGISTERS, SO STORE O[4]-D[7]
	////////////////////////////////////////////////////////////////////
	ADD SP, SP, #64
	VPOP {D24-D31}  //Q[12] - Q[15] : E[0] - E[3]
	VADD.S32 Q0, Q8, Q12   //DST[0]
	VADD.S32 Q1, Q9, Q13   //DST[1]
	VADD.S32 Q2, Q10, Q14  //DST[2]
	VADD.S32 Q3, Q11, Q15  //DST[3]
	VSUB.S32 Q7, Q12, Q8   //DST[15]
	VSUB.S32 Q6, Q13, Q9   //DST[14]
	VSUB.S32 Q5, Q14, Q10  //DST[13]
	VSUB.S32 Q4, Q15, Q11  //DST[12]
	
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	VQRSHRN.S32 D8, Q4, #12
	VQRSHRN.S32 D9, Q5, #12
	VQRSHRN.S32 D10, Q6, #12
	VQRSHRN.S32 D11, Q7, #12
	VTRN.32 D8, D10
	VTRN.32 D9, D11
	VTRN.16 D8, D9
	VTRN.16 D10, D11

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q2, R5	//minval vector Q5
	VDUP.16 Q3, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q2
	VMAX.S16 Q1, Q1, Q2
	VMAX.S16 Q4, Q4, Q2
	VMAX.S16 Q5, Q5, Q2
	VMIN.S16 Q0, Q0, Q3
	VMIN.S16 Q1, Q1, Q3
	VMIN.S16 Q4, Q4, Q3
	VMIN.S16 Q5, Q5, Q3

	
	MOV R9, #32
	MOV R1, R11
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1]
	ADD R1, R11, #24
	VST1.64 D8, [R1], R9
	VST1.64 D9, [R1], R9
	VST1.64 D10, [R1], R9
	VST1.64 D11, [R1]




	////////////////////////////////////////////////////////////////////
	VPOP {D24-D31}         //E[4] - E[7]
	SUB SP, SP, #192
	VPOP {D16-D23}         //O[4] - O[7]

	VADD.S32 Q0, Q8, Q12   //DST[4]
	VADD.S32 Q1, Q9, Q13   //DST[5]
	VADD.S32 Q2, Q10, Q14  //DST[6]
	VADD.S32 Q3, Q11, Q15  //DST[7]
	VSUB.S32 Q7, Q12, Q8   //DST[11]
	VSUB.S32 Q6, Q13, Q9   //DST[10]
	VSUB.S32 Q5, Q14, Q10  //DST[9]
	VSUB.S32 Q4, Q15, Q11  //DST[8]
	
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D2, Q1, #12
	VQRSHRN.S32 D4, Q2, #12
	VQRSHRN.S32 D6, Q3, #12
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	
	VQRSHRN.S32 D1, Q4, #12
	VQRSHRN.S32 D3, Q5, #12
	VQRSHRN.S32 D5, Q6, #12
	VQRSHRN.S32 D7, Q7, #12
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMAX.S16 Q2, Q2, Q5
	VMAX.S16 Q3, Q3, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	VMIN.S16 Q2, Q2, Q6
	VMIN.S16 Q3, Q3, Q6

	
	MOV R9, #32
	ADD R1, R11, #8
	VST1.64 {D0, D1}, [R1], R9
	VST1.64 {D2, D3}, [R1], R9
	VST1.64 {D4, D5}, [R1], R9
	VST1.64 {D6, D7}, [R1]

	ADD SP, SP, #128
	////////////////////////////////////////////////////////////////	
.endm

//***********************************************************************
//void partialButterflyInverse4x16_shift5_clip16_neon(Short *src,Short *dst)
//src: transform coefficients
//dst: the result   
//detail: shift=5 clip=16
//***********************************************************************
function partialButterflyInverse4x16_shift5_clip16_neon

	PUSH {R4-R12, LR}	
	MOV R10, R0
	MOV R11, R1
	///////////////////FIRST  4 ROWS///////////////////

	partButterflyInverse4x16_4rows_shift5_clip16


	///////////////////SECOND 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift5_clip16
	
	///////////////////THIRD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift5_clip16
	
	///////////////////FOURTH 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift5_clip16
	
	////////////////////////   END  ///////////////////	

	POP {R4-R12, PC}	   
.endfunc

//***********************************************************************
//void partialButterflyInverse16x4_shift12_clipx_neon(coef_t *blk, int shift, int clip)
//src: transform coefficients
//dst: the result   
//detail: shift=12 clip=x
//***********************************************************************
function partialButterflyInverse16x4_shift12_clipx_neon
	
	PUSH {R4-R12, LR}
	SUB R2, R2, #1
	MOV R10, R0
	MOV R11, R1
	///////////////////FIRST  4 ROWS///////////////////
	
	partButterflyInverse16x4_4rows_shift12_clipx
		
	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void idct_16x4_shift12_neon(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 12 in code  
//clip: clip
//***********************************************************************
function idct_16x4_shift12_neon
	PUSH {R4-R12, LR}	

	////////////////////alloc mem ///////////////////

	SUB R12, SP, #480

	MOV R8, R0
	MOV R1, R12

	//////////////////////////////////////////////////
	///////////////////first trans////////////////////
	//////////////////////////////////////////////////

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////FIRST  4 ROWS///////////////////

	partButterflyInverse4x16_4rows_shift5_clip16


	///////////////////SECOND 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift5_clip16
	
	///////////////////THIRD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift5_clip16
	
	///////////////////FOURTH 4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #32
	partButterflyInverse4x16_4rows_shift5_clip16
	
	//////////////////////////////////////////////////
	///////////////////second trans///////////////////
	//////////////////////////////////////////////////


	MOV R0, R12
	MOV R1, R8
	SUB R2, R2, #1
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////FIRST  4 ROWS///////////////////
	
	partButterflyInverse16x4_4rows_shift12_clipx
		
	POP {R4-R12, PC} 
.endfunc


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

//************************************************************************
//macro: the first inverse transform of 32
//************************************************************************
.macro partButterflyInverse32_4rows_shift5_clip16

	//CALCULATE EEE
	MOV R0, R10
	MOV R9, #512
	VLD1.64 D0, [R0], R9   //src[ 0       ]
	VLD1.64 D1, [R0], R9   //src[ 8*line  ]
	VLD1.64 D2, [R0], R9   //src[ 16*line ]
	VLD1.64 D3, [R0], R9   //src[ 24*line ]
	
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=42
	
	
	VMULL.S16 Q4, D1, D7[3]
	VMLAL.S16 Q4, D3, D7[2]     //EEEO[0]
	VMULL.S16 Q5, D1, D7[2]
	VMLSL.S16 Q5, D3, D7[3]     //EEEO[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[0]     //EEEE[0]	
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[0]     //EEEE[1]
	
	VADD.S32 Q8, Q6, Q4         //EEE[0]
	VADD.S32 Q9, Q7, Q5         //EEE[1]
	VSUB.S32 Q10, Q7, Q5        //EEE[2]
	VSUB.S32 Q11, Q6, Q4        //EEE[3]
	
	//CALCULATE EEO
	ADD R0, R10, #256
	MOV R9, #512	
	VLD1.64 D0, [R0], R9   //src[ 4*line  ]
	VLD1.64 D1, [R0], R9   //src[ 12*line ]
	VLD1.64 D2, [R0], R9   //src[ 20*line ]
	VLD1.64 D3, [R0], R9   //src[ 28*line ]	
	
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D7, R4, R5  //D7[0]=9, D7[1]=25, D7[2]=38, D7[3]=44
	
	VMULL.S16 Q12, D0, D7[3]
	VMLAL.S16 Q12, D1, D7[2]
	VMLAL.S16 Q12, D2, D7[1]
	VMLAL.S16 Q12, D3, D7[0]    //EEO[0]
	
	VMULL.S16 Q13, D0, D7[2]
	VMLSL.S16 Q13, D1, D7[0]
	VMLSL.S16 Q13, D2, D7[3]
	VMLSL.S16 Q13, D3, D7[1]    //EEO[1]
	
	VMULL.S16 Q14, D0, D7[1]
	VMLSL.S16 Q14, D1, D7[3]
	VMLAL.S16 Q14, D2, D7[0]
	VMLAL.S16 Q14, D3, D7[2]    //EEO[2]
		
	VMULL.S16 Q15, D0, D7[0]
	VMLSL.S16 Q15, D1, D7[1]
	VMLAL.S16 Q15, D2, D7[2]
	VMLSL.S16 Q15, D3, D7[3]    //EEO[3]
	
	//CALCULATE EE
	VADD.S32 Q0, Q8, Q12         //EE[0]
	VADD.S32 Q1, Q9, Q13         //EE[1]
	VADD.S32 Q2, Q10, Q14        //EE[2]
	VADD.S32 Q3, Q11, Q15        //EE[3]
	VSUB.S32 Q4, Q11, Q15        //EE[4]
	VSUB.S32 Q5, Q10, Q14        //EE[5]
	VSUB.S32 Q6, Q9, Q13         //EE[6]
	VSUB.S32 Q7, Q8, Q12         //EE[7]
	
	VPUSH {D0-D15}  //PUSH EE
	
	//CALCULATE EO
	ADD R0, R10, #128
	MOV R9, #256
	VLD1.64 D0, [R0], R9   //src[ 2*line ]
	VLD1.64 D1, [R0], R9   //src[ 6*line ]
	VLD1.64 D2, [R0], R9   //src[10*line ]
	VLD1.64 D3, [R0], R9   //src[14*line ]
	VLD1.64 D4, [R0], R9   //src[18*line ]
	VLD1.64 D5, [R0], R9   //src[22*line ]
	VLD1.64 D6, [R0], R9   //src[26*line ]
	VLD1.64 D7, [R0], R9   //src[30*line ]
	
	VMOV Q10, Q2
	VMOV Q11, Q3
	
	MOV R7, #4
	MOVT R7, #13
	MOV R3, #21
	MOVT R3, #29
	MOV R4, #35
	MOVT R4, #40
	MOV R5, #43
	MOVT R5, #45
	VMOV D6, R7, R3 //D6[0]=4 ,D6[1]=13 ,D6[2]=21,D6[3]=29
	VMOV D7, R4, R5 //D7[0]=35,D7[1]=40 ,D7[2]=43,D7[3]=45

	//EO[7]
	VMULL.S16 Q8, D0, D6[0]
	VMLSL.S16 Q8, D1, D6[1]
	VMLAL.S16 Q8, D2, D6[2]
	VMLSL.S16 Q8, D3, D6[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[2]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[6]
	VMULL.S16 Q8, D0, D6[1]
	VMLSL.S16 Q8, D1, D7[0]
	VMLAL.S16 Q8, D2, D7[3]
	VMLSL.S16 Q8, D3, D7[1]
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D6[0]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D7[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[5]
	VMULL.S16 Q8, D0, D6[2]
	VMLSL.S16 Q8, D1, D7[3]
	VMLAL.S16 Q8, D2, D6[3]
	VMLAL.S16 Q8, D3, D6[1]
	VMLSL.S16 Q8, D4, D7[2]
	VMLAL.S16 Q8, D5, D7[0]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q10
	VPUSH {Q8}

	//EO[4]
	VMULL.S16 Q8, D0, D6[3]
	VMLSL.S16 Q8, D1, D7[1]
	VMLSL.S16 Q8, D2, D6[1]
	VMLAL.S16 Q8, D3, D7[3]
	VMLSL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D7[0]	
	VMOV Q2, Q10
	VPUSH {Q8}	

	//EO[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D1, D6[2]
	VMLSL.S16 Q8, D2, D7[2]
	VMLAL.S16 Q8, D3, D6[0]
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[1]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D6[3]	
	VMOV Q2, Q10	
	VPUSH {Q8}
	
	
	//EO[2]
	VMULL.S16 Q8, D0, D7[1]
	VMLAL.S16 Q8, D1, D6[0]
	VMLSL.S16 Q8, D2, D7[0]
	VMLSL.S16 Q8, D3, D7[2]
	VMLSL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[3]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[1]
	VMULL.S16 Q8, D0, D7[2]
	VMLAL.S16 Q8, D1, D6[3]
	VMLAL.S16 Q8, D2, D6[0]
	VMLSL.S16 Q8, D3, D6[2]
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D6[1]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	
	//EO[0]
	VMULL.S16 Q8, D0, D7[3]
	VMLAL.S16 Q8, D1, D7[2]
	VMLAL.S16 Q8, D2, D7[1]
	VMLAL.S16 Q8, D3, D7[0]
	VMLAL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[0]
	VPUSH {Q8}


	//CALCULATE E
	VPOP  {D16-D23}               //POP EO[0] - EO[3]
	ADD SP, SP, #64
	VPOP  {D24-D31}               //POP EE[0] - EE[3]
	VADD.S32 Q0, Q12, Q8          //E[0]
	VADD.S32 Q1, Q13, Q9          //E[1]
	VADD.S32 Q2, Q14, Q10         //E[2]
	VADD.S32 Q3, Q15, Q11         //E[3]
	VSUB.S32 Q4, Q15, Q11         //E[12] 
	VSUB.S32 Q5, Q14, Q10         //E[13] 
	VSUB.S32 Q6, Q13, Q9          //E[14] 
	VSUB.S32 Q7, Q12, Q8          //E[15] 
	
	SUB SP, SP, #192
	VPUSH {D0-D15}   //PUSH E[0]-E[3], E[12]-E[15]
	
	ADD SP, SP, #192
	VPOP {D16-D23}                //EO[4] - EO[7]
	ADD SP, SP, #64
	VPOP {D24-D31}                //EE[4] - EE[7]
	VADD.S32 Q0, Q12, Q8          //E[4]
	VADD.S32 Q1, Q13, Q9          //E[5]
	VADD.S32 Q2, Q14, Q10         //E[6]
	VADD.S32 Q3, Q15, Q11         //E[7]
	VSUB.S32 Q4, Q15, Q11         //E[8] 
	VSUB.S32 Q5, Q14, Q10         //E[9] 
	VSUB.S32 Q6, Q13, Q9          //E[10] 
	VSUB.S32 Q7, Q12, Q8          //E[11]
		
	SUB SP, SP, #384
	VPUSH {D0-D15}  //PUSH E[4] - E[11]



	//CALCULATE O
	ADD R0, R10, #64
	MOV R9, #128
	
	VLD1.64 D16, [R0], R9    //src[ 1*line ]
	VLD1.64 D17, [R0], R9    //src[ 3*line ]
	VLD1.64 D18, [R0], R9    //src[ 5*line ]
	VLD1.64 D19, [R0], R9    //src[ 7*line ]
	VLD1.64 D20, [R0], R9    //src[ 9*line ]
	VLD1.64 D21, [R0], R9    //src[11*line ]
	VLD1.64 D22, [R0], R9    //src[13*line ]
	VLD1.64 D23, [R0], R9    //src[15*line ]
	VLD1.64 D24, [R0], R9    //src[17*line ]
	VLD1.64 D25, [R0], R9    //src[19*line ]
	VLD1.64 D26, [R0], R9    //src[21*line ]
	VLD1.64 D27, [R0], R9    //src[23*line ]
	VLD1.64 D28, [R0], R9    //src[25*line ]
	VLD1.64 D29, [R0], R9    //src[27*line ]
	VLD1.64 D30, [R0], R9    //src[29*line ]
	VLD1.64 D31, [R0], R9    //src[31*line ]

	MOV R7,  #2
	MOVT R7, #7
	MOV R3, #11
	MOVT R3, #15
	MOV R4, #19
	MOVT R4, #23
	MOV R5, #27
	MOVT R5, #30
	VMOV D4, R7, R3   //D4 [0]=2 ,D4[1] =7,D4 [2]=11,D4[3] =15
	VMOV D5, R4, R5   //D5 [0]=19,D5[1] =23,D5 [0]=27,D5[1] =30

	MOV R7, #34
	MOVT R7, #36
	MOV R3, #39
	MOVT R3, #41
	MOV R4, #43
	MOVT R4, #44
	MOV R5, #45
	MOVT R5, #45
	VMOV D6, R7, R3   //D6[0]=34,D6[1]=36,D6[0]=39,D6[1]=41
	VMOV D7, R4, R5   //D7[0]=43,D7[1]=44,D7[0]=45,D7[1]=45
				
	//O[15]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D5[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q12
	VMOV Q1, Q13
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D6[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D3, D7[3]
	
	VPUSH {Q7}

	
	//O[14]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D4[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D7[3]
	VPUSH {Q7}
	
	//O[13]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D7[2]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D7[1]
	VPUSH {Q7}
	
	//O[12]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D7[0]
	VPUSH {Q7}

	//O[11]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D6[0]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D5[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D5[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D6[3]
	VPUSH {Q7}

	//O[10]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D6[2]
	VPUSH {Q7}

	//O[9]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D7[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D6[1]
	VPUSH {Q7}

	//O[8]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D7[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D5[2]
	VMLAL.S16 Q7, D3, D6[0]
	VPUSH {Q7}
	
	//O[7]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D7[3]
	VMLAL.S16 Q7, D3, D4[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D7[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D5[3]
	VPUSH {Q7}
	
	//O[6]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D5[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D5[2]
	VPUSH {Q7}


	//O[5]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D4[3]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[1]
	VPUSH {Q7}

	
	//O[4]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D7[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D5[0]
	VPUSH {Q7}

	
	//O[3]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[0]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D4[3]
	VPUSH {Q7}

	
	//O[2]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLAL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D4[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D6[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D6[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLAL.S16 Q7, D3, D4[2]
	VPUSH {Q7}

	//O[1]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D5[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D4[1]
	VPUSH {Q7}

		
	//O[0]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[2]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLAL.S16 Q7, D3, D6[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D4[0]
	VPUSH {Q7}


	//CALCULATE AND STORE DST
	/////////////////////////////////////////
	VPOP {D16-D23}   //Q[8] - Q[11] :O[0]-O[3]
	ADD SP, SP, #320
	VPOP {D24-D31}   //Q[12]- Q[15] :E[0]-E[3]
    //DST[0]-[3]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	MOV R1, R11
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[28]-[31]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	ADD R1, R11, #56
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	

	/////////////////////////////////////////
	SUB SP, SP, #384
	VPOP {D16-D23}   //Q[8] - Q[11] :O[4]-O[7]
	ADD SP, SP, #128
	VPOP {D24-D31}   //Q[12]- Q[15] :E[4]-E[7]
	//DST[4]-[7]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	ADD R1, R11, #8
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[24]-[27]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	ADD R1, R11, #48
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9


	/////////////////////////////////////////
	VPOP {D24-D31}   //Q[12]- Q[15] :E[8]-E[11]
	SUB SP, SP, #256
	VPOP {D16-D23}   //Q[8] - Q[11] :O[8]-O[11]
		
	//DST[8]-[11]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	ADD R1, R11, #16
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[20]-[23]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	ADD R1, R11, #40
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	

	/////////////////////////////////////////

	VPOP {D16-D23}   //Q[8] - Q[11] :O[12]-O[15]
	ADD SP, SP, #192	
	VPOP {D24-D31}   //Q[12]- Q[15] :E[12]-E[15]
	
	
	//DST[12]-[15]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D2, Q1, #5
	VQRSHRN.S32 D4, Q2, #5
	VQRSHRN.S32 D6, Q3, #5
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	
    //DST[16]-[19]
	VSUB.S32 Q7, Q12, Q8
	VSUB.S32 Q6, Q13, Q9
	VSUB.S32 Q5, Q14, Q10
	VSUB.S32 Q4, Q15, Q11
	VQRSHRN.S32 D1, Q4, #5
	VQRSHRN.S32 D3, Q5, #5
	VQRSHRN.S32 D5, Q6, #5
	VQRSHRN.S32 D7, Q7, #5
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7
	
	ADD R1, R11, #24
	MOV R9, #64
	VST1.64 {D0, D1}, [R1], R9
	VST1.64 {D2, D3}, [R1], R9
	VST1.64 {D4, D5}, [R1], R9
	VST1.64 {D6, D7}, [R1], R9
	
	ADD SP, SP, #256
.endm

//************************************************************************
//macro: the second inverse transform of 32
//************************************************************************
.macro partButterflyInverse32_4rows_shift12_clipx

	//CALCULATE EEE
	MOV R0, R10
	MOV R9, #512
	VLD1.64 D0, [R0], R9   //src[ 0       ]
	VLD1.64 D1, [R0], R9   //src[ 8*line  ]
	VLD1.64 D2, [R0], R9   //src[ 16*line ]
	VLD1.64 D3, [R0], R9   //src[ 24*line ]
	
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=42
	
	
	VMULL.S16 Q4, D1, D7[3]
	VMLAL.S16 Q4, D3, D7[2]     //EEEO[0]
	VMULL.S16 Q5, D1, D7[2]
	VMLSL.S16 Q5, D3, D7[3]     //EEEO[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[0]     //EEEE[0]	
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[0]     //EEEE[1]
	
	VADD.S32 Q8, Q6, Q4         //EEE[0]
	VADD.S32 Q9, Q7, Q5         //EEE[1]
	VSUB.S32 Q10, Q7, Q5        //EEE[2]
	VSUB.S32 Q11, Q6, Q4        //EEE[3]
	
	//CALCULATE EEO
	ADD R0, R10, #256
	MOV R9, #512	
	VLD1.64 D0, [R0], R9   //src[ 4*line  ]
	VLD1.64 D1, [R0], R9   //src[ 12*line ]
	VLD1.64 D2, [R0], R9   //src[ 20*line ]
	VLD1.64 D3, [R0], R9   //src[ 28*line ]	
	
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D7, R4, R5  //D7[0]=9, D7[1]=25, D7[2]=38, D7[3]=44
	
	VMULL.S16 Q12, D0, D7[3]
	VMLAL.S16 Q12, D1, D7[2]
	VMLAL.S16 Q12, D2, D7[1]
	VMLAL.S16 Q12, D3, D7[0]    //EEO[0]
	
	VMULL.S16 Q13, D0, D7[2]
	VMLSL.S16 Q13, D1, D7[0]
	VMLSL.S16 Q13, D2, D7[3]
	VMLSL.S16 Q13, D3, D7[1]    //EEO[1]
	
	VMULL.S16 Q14, D0, D7[1]
	VMLSL.S16 Q14, D1, D7[3]
	VMLAL.S16 Q14, D2, D7[0]
	VMLAL.S16 Q14, D3, D7[2]    //EEO[2]
		
	VMULL.S16 Q15, D0, D7[0]
	VMLSL.S16 Q15, D1, D7[1]
	VMLAL.S16 Q15, D2, D7[2]
	VMLSL.S16 Q15, D3, D7[3]    //EEO[3]
	
	//CALCULATE EE
	VADD.S32 Q0, Q8, Q12         //EE[0]
	VADD.S32 Q1, Q9, Q13         //EE[1]
	VADD.S32 Q2, Q10, Q14        //EE[2]
	VADD.S32 Q3, Q11, Q15        //EE[3]
	VSUB.S32 Q4, Q11, Q15        //EE[4]
	VSUB.S32 Q5, Q10, Q14        //EE[5]
	VSUB.S32 Q6, Q9, Q13         //EE[6]
	VSUB.S32 Q7, Q8, Q12         //EE[7]
	
	VPUSH {D0-D15}  //PUSH EE
	
	//CALCULATE EO
	ADD R0, R10, #128
	MOV R9, #256
	VLD1.64 D0, [R0], R9   //src[ 2*line ]
	VLD1.64 D1, [R0], R9   //src[ 6*line ]
	VLD1.64 D2, [R0], R9   //src[10*line ]
	VLD1.64 D3, [R0], R9   //src[14*line ]
	VLD1.64 D4, [R0], R9   //src[18*line ]
	VLD1.64 D5, [R0], R9   //src[22*line ]
	VLD1.64 D6, [R0], R9   //src[26*line ]
	VLD1.64 D7, [R0], R9   //src[30*line ]
	
	VMOV Q10, Q2
	VMOV Q11, Q3
	
	MOV R7, #4
	MOVT R7, #13
	MOV R3, #21
	MOVT R3, #29
	MOV R4, #35
	MOVT R4, #40
	MOV R5, #43
	MOVT R5, #45
	VMOV D6, R7, R3 //D6[0]=4 ,D6[1]=13 ,D6[2]=21,D6[3]=29
	VMOV D7, R4, R5 //D7[0]=35,D7[1]=40 ,D7[2]=43,D7[3]=45

	//EO[7]
	VMULL.S16 Q8, D0, D6[0]
	VMLSL.S16 Q8, D1, D6[1]
	VMLAL.S16 Q8, D2, D6[2]
	VMLSL.S16 Q8, D3, D6[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[2]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[6]
	VMULL.S16 Q8, D0, D6[1]
	VMLSL.S16 Q8, D1, D7[0]
	VMLAL.S16 Q8, D2, D7[3]
	VMLSL.S16 Q8, D3, D7[1]
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D6[0]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D7[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[5]
	VMULL.S16 Q8, D0, D6[2]
	VMLSL.S16 Q8, D1, D7[3]
	VMLAL.S16 Q8, D2, D6[3]
	VMLAL.S16 Q8, D3, D6[1]
	VMLSL.S16 Q8, D4, D7[2]
	VMLAL.S16 Q8, D5, D7[0]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q10
	VPUSH {Q8}

	//EO[4]
	VMULL.S16 Q8, D0, D6[3]
	VMLSL.S16 Q8, D1, D7[1]
	VMLSL.S16 Q8, D2, D6[1]
	VMLAL.S16 Q8, D3, D7[3]
	VMLSL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D7[0]	
	VMOV Q2, Q10
	VPUSH {Q8}	

	//EO[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D1, D6[2]
	VMLSL.S16 Q8, D2, D7[2]
	VMLAL.S16 Q8, D3, D6[0]
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[1]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D6[3]	
	VMOV Q2, Q10	
	VPUSH {Q8}
	
	
	//EO[2]
	VMULL.S16 Q8, D0, D7[1]
	VMLAL.S16 Q8, D1, D6[0]
	VMLSL.S16 Q8, D2, D7[0]
	VMLSL.S16 Q8, D3, D7[2]
	VMLSL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[3]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[1]
	VMULL.S16 Q8, D0, D7[2]
	VMLAL.S16 Q8, D1, D6[3]
	VMLAL.S16 Q8, D2, D6[0]
	VMLSL.S16 Q8, D3, D6[2]
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D6[1]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	
	//EO[0]
	VMULL.S16 Q8, D0, D7[3]
	VMLAL.S16 Q8, D1, D7[2]
	VMLAL.S16 Q8, D2, D7[1]
	VMLAL.S16 Q8, D3, D7[0]
	VMLAL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[0]
	VPUSH {Q8}


	//CALCULATE E
	VPOP  {D16-D23}               //POP EO[0] - EO[3]
	ADD SP, SP, #64
	VPOP  {D24-D31}               //POP EE[0] - EE[3]
	VADD.S32 Q0, Q12, Q8          //E[0]
	VADD.S32 Q1, Q13, Q9          //E[1]
	VADD.S32 Q2, Q14, Q10         //E[2]
	VADD.S32 Q3, Q15, Q11         //E[3]
	VSUB.S32 Q4, Q15, Q11         //E[12] 
	VSUB.S32 Q5, Q14, Q10         //E[13] 
	VSUB.S32 Q6, Q13, Q9          //E[14] 
	VSUB.S32 Q7, Q12, Q8          //E[15] 
	
	SUB SP, SP, #192
	VPUSH {D0-D15}   //PUSH E[0]-E[3], E[12]-E[15]
	
	ADD SP, SP, #192
	VPOP {D16-D23}                //EO[4] - EO[7]
	ADD SP, SP, #64
	VPOP {D24-D31}                //EE[4] - EE[7]
	VADD.S32 Q0, Q12, Q8          //E[4]
	VADD.S32 Q1, Q13, Q9          //E[5]
	VADD.S32 Q2, Q14, Q10         //E[6]
	VADD.S32 Q3, Q15, Q11         //E[7]
	VSUB.S32 Q4, Q15, Q11         //E[8] 
	VSUB.S32 Q5, Q14, Q10         //E[9] 
	VSUB.S32 Q6, Q13, Q9          //E[10] 
	VSUB.S32 Q7, Q12, Q8          //E[11]
		
	SUB SP, SP, #384
	VPUSH {D0-D15}  //PUSH E[4] - E[11]



	//CALCULATE O
	ADD R0, R10, #64
	MOV R9, #128
	
	VLD1.64 D16, [R0], R9    //src[ 1*line ]
	VLD1.64 D17, [R0], R9    //src[ 3*line ]
	VLD1.64 D18, [R0], R9    //src[ 5*line ]
	VLD1.64 D19, [R0], R9    //src[ 7*line ]
	VLD1.64 D20, [R0], R9    //src[ 9*line ]
	VLD1.64 D21, [R0], R9    //src[11*line ]
	VLD1.64 D22, [R0], R9    //src[13*line ]
	VLD1.64 D23, [R0], R9    //src[15*line ]
	VLD1.64 D24, [R0], R9    //src[17*line ]
	VLD1.64 D25, [R0], R9    //src[19*line ]
	VLD1.64 D26, [R0], R9    //src[21*line ]
	VLD1.64 D27, [R0], R9    //src[23*line ]
	VLD1.64 D28, [R0], R9    //src[25*line ]
	VLD1.64 D29, [R0], R9    //src[27*line ]
	VLD1.64 D30, [R0], R9    //src[29*line ]
	VLD1.64 D31, [R0], R9    //src[31*line ]

	MOV R7,  #2
	MOVT R7, #7
	MOV R3, #11
	MOVT R3, #15
	MOV R4, #19
	MOVT R4, #23
	MOV R5, #27
	MOVT R5, #30
	VMOV D4, R7, R3   //D4 [0]=2 ,D4[1] =7,D4 [2]=11,D4[3] =15
	VMOV D5, R4, R5   //D5 [0]=19,D5[1] =23,D5 [0]=27,D5[1] =30

	MOV R7, #34
	MOVT R7, #36
	MOV R3, #39
	MOVT R3, #41
	MOV R4, #43
	MOVT R4, #44
	MOV R5, #45
	MOVT R5, #45
	VMOV D6, R7, R3   //D6[0]=34,D6[1]=36,D6[0]=39,D6[1]=41
	VMOV D7, R4, R5   //D7[0]=43,D7[1]=44,D7[0]=45,D7[1]=45
				
	//O[15]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D5[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q12
	VMOV Q1, Q13
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D6[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D3, D7[3]
	
	VPUSH {Q7}

	
	//O[14]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D4[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D7[3]
	VPUSH {Q7}
	
	//O[13]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D7[2]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D7[1]
	VPUSH {Q7}
	
	//O[12]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D7[0]
	VPUSH {Q7}

	//O[11]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D6[0]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D5[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D5[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D6[3]
	VPUSH {Q7}

	//O[10]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D6[2]
	VPUSH {Q7}

	//O[9]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D7[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D6[1]
	VPUSH {Q7}

	//O[8]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D7[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D5[2]
	VMLAL.S16 Q7, D3, D6[0]
	VPUSH {Q7}
	
	//O[7]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D7[3]
	VMLAL.S16 Q7, D3, D4[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D7[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D5[3]
	VPUSH {Q7}
	
	//O[6]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D5[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D5[2]
	VPUSH {Q7}


	//O[5]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D4[3]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[1]
	VPUSH {Q7}

	
	//O[4]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D7[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D5[0]
	VPUSH {Q7}

	
	//O[3]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[0]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D4[3]
	VPUSH {Q7}

	
	//O[2]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLAL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D4[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D6[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D6[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLAL.S16 Q7, D3, D4[2]
	VPUSH {Q7}

	//O[1]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D5[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D4[1]
	VPUSH {Q7}

		
	//O[0]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[2]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLAL.S16 Q7, D3, D6[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D4[0]
	VPUSH {Q7}


	//CALCULATE AND STORE DST
	/////////////////////////////////////////
	VPOP {D16-D23}   //Q[8] - Q[11] :O[0]-O[3]
	ADD SP, SP, #320
	VPOP {D24-D31}   //Q[12]- Q[15] :E[0]-E[3]
    //DST[0]-[3]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6


	MOV R1, R11
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[28]-[31]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #56
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9

	

	/////////////////////////////////////////
	SUB SP, SP, #384
	VPOP {D16-D23}   //Q[8] - Q[11] :O[4]-O[7]
	ADD SP, SP, #128
	VPOP {D24-D31}   //Q[12]- Q[15] :E[4]-E[7]
	//DST[4]-[7]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #8
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[24]-[27]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #48
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9


	/////////////////////////////////////////
	VPOP {D24-D31}   //Q[12]- Q[15] :E[8]-E[11]
	SUB SP, SP, #256
	VPOP {D16-D23}   //Q[8] - Q[11] :O[8]-O[11]
		
	//DST[8]-[11]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3


	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #16
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[20]-[23]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #40
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	

	/////////////////////////////////////////

	VPOP {D16-D23}   //Q[8] - Q[11] :O[12]-O[15]
	ADD SP, SP, #192	
	VPOP {D24-D31}   //Q[12]- Q[15] :E[12]-E[15]
	
	
	//DST[12]-[15]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D2, Q1, #12
	VQRSHRN.S32 D4, Q2, #12
	VQRSHRN.S32 D6, Q3, #12
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	
    //DST[16]-[19]
	VSUB.S32 Q7, Q12, Q8
	VSUB.S32 Q6, Q13, Q9
	VSUB.S32 Q5, Q14, Q10
	VSUB.S32 Q4, Q15, Q11
	VQRSHRN.S32 D1, Q4, #12
	VQRSHRN.S32 D3, Q5, #12
	VQRSHRN.S32 D5, Q6, #12
	VQRSHRN.S32 D7, Q7, #12
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMAX.S16 Q2, Q2, Q5
	VMAX.S16 Q3, Q3, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	VMIN.S16 Q2, Q2, Q6
	VMIN.S16 Q3, Q3, Q6

	ADD R1, R11, #24
	MOV R9, #64
	VST1.64 {D0, D1}, [R1], R9
	VST1.64 {D2, D3}, [R1], R9
	VST1.64 {D4, D5}, [R1], R9
	VST1.64 {D6, D7}, [R1], R9
	
	ADD SP, SP, #256	
.endm

//************************************************************************
//macro: the second inverse transform of 32
//************************************************************************
.macro partButterflyInverse32_4rows_shift11_clipx

	//CALCULATE EEE
	MOV R0, R10
	MOV R9, #512
	VLD1.64 D0, [R0], R9   //src[ 0       ]
	VLD1.64 D1, [R0], R9   //src[ 8*line  ]
	VLD1.64 D2, [R0], R9   //src[ 16*line ]
	VLD1.64 D3, [R0], R9   //src[ 24*line ]
	
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=42
	
	
	VMULL.S16 Q4, D1, D7[3]
	VMLAL.S16 Q4, D3, D7[2]     //EEEO[0]
	VMULL.S16 Q5, D1, D7[2]
	VMLSL.S16 Q5, D3, D7[3]     //EEEO[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[0]     //EEEE[0]	
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[0]     //EEEE[1]
	
	VADD.S32 Q8, Q6, Q4         //EEE[0]
	VADD.S32 Q9, Q7, Q5         //EEE[1]
	VSUB.S32 Q10, Q7, Q5        //EEE[2]
	VSUB.S32 Q11, Q6, Q4        //EEE[3]
	
	//CALCULATE EEO
	ADD R0, R10, #256
	MOV R9, #512	
	VLD1.64 D0, [R0], R9   //src[ 4*line  ]
	VLD1.64 D1, [R0], R9   //src[ 12*line ]
	VLD1.64 D2, [R0], R9   //src[ 20*line ]
	VLD1.64 D3, [R0], R9   //src[ 28*line ]	
	
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D7, R4, R5  //D7[0]=9, D7[1]=25, D7[2]=38, D7[3]=44
	
	VMULL.S16 Q12, D0, D7[3]
	VMLAL.S16 Q12, D1, D7[2]
	VMLAL.S16 Q12, D2, D7[1]
	VMLAL.S16 Q12, D3, D7[0]    //EEO[0]
	
	VMULL.S16 Q13, D0, D7[2]
	VMLSL.S16 Q13, D1, D7[0]
	VMLSL.S16 Q13, D2, D7[3]
	VMLSL.S16 Q13, D3, D7[1]    //EEO[1]
	
	VMULL.S16 Q14, D0, D7[1]
	VMLSL.S16 Q14, D1, D7[3]
	VMLAL.S16 Q14, D2, D7[0]
	VMLAL.S16 Q14, D3, D7[2]    //EEO[2]
		
	VMULL.S16 Q15, D0, D7[0]
	VMLSL.S16 Q15, D1, D7[1]
	VMLAL.S16 Q15, D2, D7[2]
	VMLSL.S16 Q15, D3, D7[3]    //EEO[3]
	
	//CALCULATE EE
	VADD.S32 Q0, Q8, Q12         //EE[0]
	VADD.S32 Q1, Q9, Q13         //EE[1]
	VADD.S32 Q2, Q10, Q14        //EE[2]
	VADD.S32 Q3, Q11, Q15        //EE[3]
	VSUB.S32 Q4, Q11, Q15        //EE[4]
	VSUB.S32 Q5, Q10, Q14        //EE[5]
	VSUB.S32 Q6, Q9, Q13         //EE[6]
	VSUB.S32 Q7, Q8, Q12         //EE[7]
	
	VPUSH {D0-D15}  //PUSH EE
	
	//CALCULATE EO
	ADD R0, R10, #128
	MOV R9, #256
	VLD1.64 D0, [R0], R9   //src[ 2*line ]
	VLD1.64 D1, [R0], R9   //src[ 6*line ]
	VLD1.64 D2, [R0], R9   //src[10*line ]
	VLD1.64 D3, [R0], R9   //src[14*line ]
	VLD1.64 D4, [R0], R9   //src[18*line ]
	VLD1.64 D5, [R0], R9   //src[22*line ]
	VLD1.64 D6, [R0], R9   //src[26*line ]
	VLD1.64 D7, [R0], R9   //src[30*line ]
	
	VMOV Q10, Q2
	VMOV Q11, Q3
	
	MOV R7, #4
	MOVT R7, #13
	MOV R3, #21
	MOVT R3, #29
	MOV R4, #35
	MOVT R4, #40
	MOV R5, #43
	MOVT R5, #45
	VMOV D6, R7, R3 //D6[0]=4 ,D6[1]=13 ,D6[2]=21,D6[3]=29
	VMOV D7, R4, R5 //D7[0]=35,D7[1]=40 ,D7[2]=43,D7[3]=45

	//EO[7]
	VMULL.S16 Q8, D0, D6[0]
	VMLSL.S16 Q8, D1, D6[1]
	VMLAL.S16 Q8, D2, D6[2]
	VMLSL.S16 Q8, D3, D6[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[2]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[6]
	VMULL.S16 Q8, D0, D6[1]
	VMLSL.S16 Q8, D1, D7[0]
	VMLAL.S16 Q8, D2, D7[3]
	VMLSL.S16 Q8, D3, D7[1]
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D6[0]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D7[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[5]
	VMULL.S16 Q8, D0, D6[2]
	VMLSL.S16 Q8, D1, D7[3]
	VMLAL.S16 Q8, D2, D6[3]
	VMLAL.S16 Q8, D3, D6[1]
	VMLSL.S16 Q8, D4, D7[2]
	VMLAL.S16 Q8, D5, D7[0]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q10
	VPUSH {Q8}

	//EO[4]
	VMULL.S16 Q8, D0, D6[3]
	VMLSL.S16 Q8, D1, D7[1]
	VMLSL.S16 Q8, D2, D6[1]
	VMLAL.S16 Q8, D3, D7[3]
	VMLSL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D7[0]	
	VMOV Q2, Q10
	VPUSH {Q8}	

	//EO[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D1, D6[2]
	VMLSL.S16 Q8, D2, D7[2]
	VMLAL.S16 Q8, D3, D6[0]
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[1]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D6[3]	
	VMOV Q2, Q10	
	VPUSH {Q8}
	
	
	//EO[2]
	VMULL.S16 Q8, D0, D7[1]
	VMLAL.S16 Q8, D1, D6[0]
	VMLSL.S16 Q8, D2, D7[0]
	VMLSL.S16 Q8, D3, D7[2]
	VMLSL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[3]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[1]
	VMULL.S16 Q8, D0, D7[2]
	VMLAL.S16 Q8, D1, D6[3]
	VMLAL.S16 Q8, D2, D6[0]
	VMLSL.S16 Q8, D3, D6[2]
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D6[1]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	
	//EO[0]
	VMULL.S16 Q8, D0, D7[3]
	VMLAL.S16 Q8, D1, D7[2]
	VMLAL.S16 Q8, D2, D7[1]
	VMLAL.S16 Q8, D3, D7[0]
	VMLAL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[0]
	VPUSH {Q8}


	//CALCULATE E
	VPOP  {D16-D23}               //POP EO[0] - EO[3]
	ADD SP, SP, #64
	VPOP  {D24-D31}               //POP EE[0] - EE[3]
	VADD.S32 Q0, Q12, Q8          //E[0]
	VADD.S32 Q1, Q13, Q9          //E[1]
	VADD.S32 Q2, Q14, Q10         //E[2]
	VADD.S32 Q3, Q15, Q11         //E[3]
	VSUB.S32 Q4, Q15, Q11         //E[12] 
	VSUB.S32 Q5, Q14, Q10         //E[13] 
	VSUB.S32 Q6, Q13, Q9          //E[14] 
	VSUB.S32 Q7, Q12, Q8          //E[15] 
	
	SUB SP, SP, #192
	VPUSH {D0-D15}   //PUSH E[0]-E[3], E[12]-E[15]
	
	ADD SP, SP, #192
	VPOP {D16-D23}                //EO[4] - EO[7]
	ADD SP, SP, #64
	VPOP {D24-D31}                //EE[4] - EE[7]
	VADD.S32 Q0, Q12, Q8          //E[4]
	VADD.S32 Q1, Q13, Q9          //E[5]
	VADD.S32 Q2, Q14, Q10         //E[6]
	VADD.S32 Q3, Q15, Q11         //E[7]
	VSUB.S32 Q4, Q15, Q11         //E[8] 
	VSUB.S32 Q5, Q14, Q10         //E[9] 
	VSUB.S32 Q6, Q13, Q9          //E[10] 
	VSUB.S32 Q7, Q12, Q8          //E[11]
		
	SUB SP, SP, #384
	VPUSH {D0-D15}  //PUSH E[4] - E[11]



	//CALCULATE O
	ADD R0, R10, #64
	MOV R9, #128
	
	VLD1.64 D16, [R0], R9    //src[ 1*line ]
	VLD1.64 D17, [R0], R9    //src[ 3*line ]
	VLD1.64 D18, [R0], R9    //src[ 5*line ]
	VLD1.64 D19, [R0], R9    //src[ 7*line ]
	VLD1.64 D20, [R0], R9    //src[ 9*line ]
	VLD1.64 D21, [R0], R9    //src[11*line ]
	VLD1.64 D22, [R0], R9    //src[13*line ]
	VLD1.64 D23, [R0], R9    //src[15*line ]
	VLD1.64 D24, [R0], R9    //src[17*line ]
	VLD1.64 D25, [R0], R9    //src[19*line ]
	VLD1.64 D26, [R0], R9    //src[21*line ]
	VLD1.64 D27, [R0], R9    //src[23*line ]
	VLD1.64 D28, [R0], R9    //src[25*line ]
	VLD1.64 D29, [R0], R9    //src[27*line ]
	VLD1.64 D30, [R0], R9    //src[29*line ]
	VLD1.64 D31, [R0], R9    //src[31*line ]

	MOV R7,  #2
	MOVT R7, #7
	MOV R3, #11
	MOVT R3, #15
	MOV R4, #19
	MOVT R4, #23
	MOV R5, #27
	MOVT R5, #30
	VMOV D4, R7, R3   //D4 [0]=2 ,D4[1] =7,D4 [2]=11,D4[3] =15
	VMOV D5, R4, R5   //D5 [0]=19,D5[1] =23,D5 [0]=27,D5[1] =30

	MOV R7, #34
	MOVT R7, #36
	MOV R3, #39
	MOVT R3, #41
	MOV R4, #43
	MOVT R4, #44
	MOV R5, #45
	MOVT R5, #45
	VMOV D6, R7, R3   //D6[0]=34,D6[1]=36,D6[0]=39,D6[1]=41
	VMOV D7, R4, R5   //D7[0]=43,D7[1]=44,D7[0]=45,D7[1]=45
				
	//O[15]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D5[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q12
	VMOV Q1, Q13
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D6[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D3, D7[3]
	
	VPUSH {Q7}

	
	//O[14]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D4[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D7[3]
	VPUSH {Q7}
	
	//O[13]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D7[2]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D7[1]
	VPUSH {Q7}
	
	//O[12]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D7[0]
	VPUSH {Q7}

	//O[11]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D6[0]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D5[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D5[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D6[3]
	VPUSH {Q7}

	//O[10]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D6[2]
	VPUSH {Q7}

	//O[9]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D7[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D6[1]
	VPUSH {Q7}

	//O[8]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D7[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D5[2]
	VMLAL.S16 Q7, D3, D6[0]
	VPUSH {Q7}
	
	//O[7]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D7[3]
	VMLAL.S16 Q7, D3, D4[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D7[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D5[3]
	VPUSH {Q7}
	
	//O[6]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D5[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D5[2]
	VPUSH {Q7}


	//O[5]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D4[3]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[1]
	VPUSH {Q7}

	
	//O[4]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D7[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D5[0]
	VPUSH {Q7}

	
	//O[3]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[0]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D4[3]
	VPUSH {Q7}

	
	//O[2]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLAL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D4[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D6[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D6[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLAL.S16 Q7, D3, D4[2]
	VPUSH {Q7}

	//O[1]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D5[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D4[1]
	VPUSH {Q7}

		
	//O[0]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[2]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLAL.S16 Q7, D3, D6[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D4[0]
	VPUSH {Q7}


	//CALCULATE AND STORE DST
	/////////////////////////////////////////
	VPOP {D16-D23}   //Q[8] - Q[11] :O[0]-O[3]
	ADD SP, SP, #320
	VPOP {D24-D31}   //Q[12]- Q[15] :E[0]-E[3]
    //DST[0]-[3]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6


	MOV R1, R11
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[28]-[31]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #56
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9

	

	/////////////////////////////////////////
	SUB SP, SP, #384
	VPOP {D16-D23}   //Q[8] - Q[11] :O[4]-O[7]
	ADD SP, SP, #128
	VPOP {D24-D31}   //Q[12]- Q[15] :E[4]-E[7]
	//DST[4]-[7]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #8
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[24]-[27]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #48
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9


	/////////////////////////////////////////
	VPOP {D24-D31}   //Q[12]- Q[15] :E[8]-E[11]
	SUB SP, SP, #256
	VPOP {D16-D23}   //Q[8] - Q[11] :O[8]-O[11]
		
	//DST[8]-[11]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3


	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #16
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[20]-[23]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #40
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	

	/////////////////////////////////////////

	VPOP {D16-D23}   //Q[8] - Q[11] :O[12]-O[15]
	ADD SP, SP, #192	
	VPOP {D24-D31}   //Q[12]- Q[15] :E[12]-E[15]
	
	
	//DST[12]-[15]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D2, Q1, #11
	VQRSHRN.S32 D4, Q2, #11
	VQRSHRN.S32 D6, Q3, #11
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	
    //DST[16]-[19]
	VSUB.S32 Q7, Q12, Q8
	VSUB.S32 Q6, Q13, Q9
	VSUB.S32 Q5, Q14, Q10
	VSUB.S32 Q4, Q15, Q11
	VQRSHRN.S32 D1, Q4, #11
	VQRSHRN.S32 D3, Q5, #11
	VQRSHRN.S32 D5, Q6, #11
	VQRSHRN.S32 D7, Q7, #11
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMAX.S16 Q2, Q2, Q5
	VMAX.S16 Q3, Q3, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	VMIN.S16 Q2, Q2, Q6
	VMIN.S16 Q3, Q3, Q6

	ADD R1, R11, #24
	MOV R9, #64
	VST1.64 {D0, D1}, [R1], R9
	VST1.64 {D2, D3}, [R1], R9
	VST1.64 {D4, D5}, [R1], R9
	VST1.64 {D6, D7}, [R1], R9
	
	ADD SP, SP, #256	
.endm

//***********************************************************************
//void partialButterflyInverse32_shift5_clip16_neon(Short *src,Short *dst)
//src: transform coefficients
//dst: the result   
//detail: shift=5 clip=16
//***********************************************************************
function partialButterflyInverse32_shift5_clip16_neon
	PUSH {R4-R12, LR}	
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////3RD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////4TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////5TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////6TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////7TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////8TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void partialButterflyInverse32_shift12_clipx_neon(Short *src,Short *dst,int clip)
//src: transform coefficients
//dst: the result   
//detail: shift=12 clip=x
//***********************************************************************
function partialButterflyInverse32_shift12_clipx_neon
	PUSH {R4-R12, LR}	

	SUB R2, #1

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////3RD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////4TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////5TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////6TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////7TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////8TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void idct_32x32_shift12_neon(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 12 in code  
//clip: clip
//***********************************************************************
function idct_32x32_shift12_neon
	PUSH {R4-R12, LR}	

	////////////////////alloc mem ///////////////////

	SUB R12, SP, #2880

	MOV R8, R0
	MOV R1, R12

	//////////////////////////////////////////////////
	///////////////////first trans////////////////////
	//////////////////////////////////////////////////

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////3RD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////4TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////5TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////6TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////7TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////8TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16

	//////////////////////////////////////////////////
	///////////////////second trans///////////////////
	//////////////////////////////////////////////////

	MOV R0, R12
	MOV R1, R8
	SUB R2, R2, #1
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////3RD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////4TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////5TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////6TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////7TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////8TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift12_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void idct_32x32_shift11_neon(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 11 in code  
//clip: clip
//***********************************************************************
function idct_32x32_shift11_neon
	PUSH {R4-R12, LR}	

	////////////////////alloc mem ///////////////////

	SUB R12, SP, #2880

	MOV R8, R0
	MOV R1, R12

	//////////////////////////////////////////////////
	///////////////////first trans////////////////////
	//////////////////////////////////////////////////

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////3RD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////4TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////5TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////6TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////7TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16
	
	///////////////////8TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift5_clip16

	//////////////////////////////////////////////////
	///////////////////second trans///////////////////
	//////////////////////////////////////////////////

	MOV R0, R12
	MOV R1, R8
	SUB R2, R2, #1
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32_4rows_shift11_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift11_clipx
	
	///////////////////3RD  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift11_clipx
	
	///////////////////4TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift11_clipx
	
	///////////////////5TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift11_clipx
	
	///////////////////6TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift11_clipx
	
	///////////////////7TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift11_clipx
	
	///////////////////8TH  4 ROWS///////////////////
	
	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32_4rows_shift11_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


//************************************************************************
//macro: the first inverse transform of 8*32
//************************************************************************
.macro partButterflyInverse32x8_4rows_shift5_clip16

	//CALCULATE EEE
	MOV R0, R10
	MOV R9, #128
	VLD1.64 D0, [R0], R9   //src[ 0       ]
	VLD1.64 D1, [R0], R9   //src[ 8*line  ]
	VLD1.64 D2, [R0], R9   //src[ 16*line ]
	VLD1.64 D3, [R0], R9   //src[ 24*line ]
	
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=42
	
	
	VMULL.S16 Q4, D1, D7[3]
	VMLAL.S16 Q4, D3, D7[2]     //EEEO[0]
	VMULL.S16 Q5, D1, D7[2]
	VMLSL.S16 Q5, D3, D7[3]     //EEEO[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[0]     //EEEE[0]	
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[0]     //EEEE[1]
	
	VADD.S32 Q8, Q6, Q4         //EEE[0]
	VADD.S32 Q9, Q7, Q5         //EEE[1]
	VSUB.S32 Q10, Q7, Q5        //EEE[2]
	VSUB.S32 Q11, Q6, Q4        //EEE[3]
	
	//CALCULATE EEO
	ADD R0, R10, #64
	MOV R9, #128	
	VLD1.64 D0, [R0], R9   //src[ 4*line  ]
	VLD1.64 D1, [R0], R9   //src[ 12*line ]
	VLD1.64 D2, [R0], R9   //src[ 20*line ]
	VLD1.64 D3, [R0], R9   //src[ 28*line ]	
	
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D7, R4, R5  //D7[0]=9, D7[1]=25, D7[2]=38, D7[3]=44
	
	VMULL.S16 Q12, D0, D7[3]
	VMLAL.S16 Q12, D1, D7[2]
	VMLAL.S16 Q12, D2, D7[1]
	VMLAL.S16 Q12, D3, D7[0]    //EEO[0]
	
	VMULL.S16 Q13, D0, D7[2]
	VMLSL.S16 Q13, D1, D7[0]
	VMLSL.S16 Q13, D2, D7[3]
	VMLSL.S16 Q13, D3, D7[1]    //EEO[1]
	
	VMULL.S16 Q14, D0, D7[1]
	VMLSL.S16 Q14, D1, D7[3]
	VMLAL.S16 Q14, D2, D7[0]
	VMLAL.S16 Q14, D3, D7[2]    //EEO[2]
		
	VMULL.S16 Q15, D0, D7[0]
	VMLSL.S16 Q15, D1, D7[1]
	VMLAL.S16 Q15, D2, D7[2]
	VMLSL.S16 Q15, D3, D7[3]    //EEO[3]
	
	//CALCULATE EE
	VADD.S32 Q0, Q8, Q12         //EE[0]
	VADD.S32 Q1, Q9, Q13         //EE[1]
	VADD.S32 Q2, Q10, Q14        //EE[2]
	VADD.S32 Q3, Q11, Q15        //EE[3]
	VSUB.S32 Q4, Q11, Q15        //EE[4]
	VSUB.S32 Q5, Q10, Q14        //EE[5]
	VSUB.S32 Q6, Q9, Q13         //EE[6]
	VSUB.S32 Q7, Q8, Q12         //EE[7]
	
	VPUSH {D0-D15}  //PUSH EE
	
	//CALCULATE EO
	ADD R0, R10, #32
	MOV R9, #64
	VLD1.64 D0, [R0], R9   //src[ 2*line ]
	VLD1.64 D1, [R0], R9   //src[ 6*line ]
	VLD1.64 D2, [R0], R9   //src[10*line ]
	VLD1.64 D3, [R0], R9   //src[14*line ]
	VLD1.64 D4, [R0], R9   //src[18*line ]
	VLD1.64 D5, [R0], R9   //src[22*line ]
	VLD1.64 D6, [R0], R9   //src[26*line ]
	VLD1.64 D7, [R0], R9   //src[30*line ]
	
	VMOV Q10, Q2
	VMOV Q11, Q3
	
	MOV R7, #4
	MOVT R7, #13
	MOV R3, #21
	MOVT R3, #29
	MOV R4, #35
	MOVT R4, #40
	MOV R5, #43
	MOVT R5, #45
	VMOV D6, R7, R3 //D6[0]=4 ,D6[1]=13 ,D6[2]=21,D6[3]=29
	VMOV D7, R4, R5 //D7[0]=35,D7[1]=40 ,D7[2]=43,D7[3]=45

	//EO[7]
	VMULL.S16 Q8, D0, D6[0]
	VMLSL.S16 Q8, D1, D6[1]
	VMLAL.S16 Q8, D2, D6[2]
	VMLSL.S16 Q8, D3, D6[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[2]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[6]
	VMULL.S16 Q8, D0, D6[1]
	VMLSL.S16 Q8, D1, D7[0]
	VMLAL.S16 Q8, D2, D7[3]
	VMLSL.S16 Q8, D3, D7[1]
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D6[0]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D7[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[5]
	VMULL.S16 Q8, D0, D6[2]
	VMLSL.S16 Q8, D1, D7[3]
	VMLAL.S16 Q8, D2, D6[3]
	VMLAL.S16 Q8, D3, D6[1]
	VMLSL.S16 Q8, D4, D7[2]
	VMLAL.S16 Q8, D5, D7[0]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q10
	VPUSH {Q8}

	//EO[4]
	VMULL.S16 Q8, D0, D6[3]
	VMLSL.S16 Q8, D1, D7[1]
	VMLSL.S16 Q8, D2, D6[1]
	VMLAL.S16 Q8, D3, D7[3]
	VMLSL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D7[0]	
	VMOV Q2, Q10
	VPUSH {Q8}	

	//EO[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D1, D6[2]
	VMLSL.S16 Q8, D2, D7[2]
	VMLAL.S16 Q8, D3, D6[0]
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[1]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D6[3]	
	VMOV Q2, Q10	
	VPUSH {Q8}
	
	
	//EO[2]
	VMULL.S16 Q8, D0, D7[1]
	VMLAL.S16 Q8, D1, D6[0]
	VMLSL.S16 Q8, D2, D7[0]
	VMLSL.S16 Q8, D3, D7[2]
	VMLSL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[3]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[1]
	VMULL.S16 Q8, D0, D7[2]
	VMLAL.S16 Q8, D1, D6[3]
	VMLAL.S16 Q8, D2, D6[0]
	VMLSL.S16 Q8, D3, D6[2]
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D6[1]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	
	//EO[0]
	VMULL.S16 Q8, D0, D7[3]
	VMLAL.S16 Q8, D1, D7[2]
	VMLAL.S16 Q8, D2, D7[1]
	VMLAL.S16 Q8, D3, D7[0]
	VMLAL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[0]
	VPUSH {Q8}


	//CALCULATE E
	VPOP  {D16-D23}               //POP EO[0] - EO[3]
	ADD SP, SP, #64
	VPOP  {D24-D31}               //POP EE[0] - EE[3]
	VADD.S32 Q0, Q12, Q8          //E[0]
	VADD.S32 Q1, Q13, Q9          //E[1]
	VADD.S32 Q2, Q14, Q10         //E[2]
	VADD.S32 Q3, Q15, Q11         //E[3]
	VSUB.S32 Q4, Q15, Q11         //E[12] 
	VSUB.S32 Q5, Q14, Q10         //E[13] 
	VSUB.S32 Q6, Q13, Q9          //E[14] 
	VSUB.S32 Q7, Q12, Q8          //E[15] 
	
	SUB SP, SP, #192
	VPUSH {D0-D15}   //PUSH E[0]-E[3], E[12]-E[15]
	
	ADD SP, SP, #192
	VPOP {D16-D23}                //EO[4] - EO[7]
	ADD SP, SP, #64
	VPOP {D24-D31}                //EE[4] - EE[7]
	VADD.S32 Q0, Q12, Q8          //E[4]
	VADD.S32 Q1, Q13, Q9          //E[5]
	VADD.S32 Q2, Q14, Q10         //E[6]
	VADD.S32 Q3, Q15, Q11         //E[7]
	VSUB.S32 Q4, Q15, Q11         //E[8] 
	VSUB.S32 Q5, Q14, Q10         //E[9] 
	VSUB.S32 Q6, Q13, Q9          //E[10] 
	VSUB.S32 Q7, Q12, Q8          //E[11]
		
	SUB SP, SP, #384
	VPUSH {D0-D15}  //PUSH E[4] - E[11]



	//CALCULATE O
	ADD R0, R10, #16
	MOV R9, #32
	
	VLD1.64 D16, [R0], R9    //src[ 1*line ]
	VLD1.64 D17, [R0], R9    //src[ 3*line ]
	VLD1.64 D18, [R0], R9    //src[ 5*line ]
	VLD1.64 D19, [R0], R9    //src[ 7*line ]
	VLD1.64 D20, [R0], R9    //src[ 9*line ]
	VLD1.64 D21, [R0], R9    //src[11*line ]
	VLD1.64 D22, [R0], R9    //src[13*line ]
	VLD1.64 D23, [R0], R9    //src[15*line ]
	VLD1.64 D24, [R0], R9    //src[17*line ]
	VLD1.64 D25, [R0], R9    //src[19*line ]
	VLD1.64 D26, [R0], R9    //src[21*line ]
	VLD1.64 D27, [R0], R9    //src[23*line ]
	VLD1.64 D28, [R0], R9    //src[25*line ]
	VLD1.64 D29, [R0], R9    //src[27*line ]
	VLD1.64 D30, [R0], R9    //src[29*line ]
	VLD1.64 D31, [R0], R9    //src[31*line ]

	MOV R7,  #2
	MOVT R7, #7
	MOV R3, #11
	MOVT R3, #15
	MOV R4, #19
	MOVT R4, #23
	MOV R5, #27
	MOVT R5, #30
	VMOV D4, R7, R3   //D4 [0]=2 ,D4[1] =7,D4 [2]=11,D4[3] =15
	VMOV D5, R4, R5   //D5 [0]=19,D5[1] =23,D5 [0]=27,D5[1] =30

	MOV R7, #34
	MOVT R7, #36
	MOV R3, #39
	MOVT R3, #41
	MOV R4, #43
	MOVT R4, #44
	MOV R5, #45
	MOVT R5, #45
	VMOV D6, R7, R3   //D6[0]=34,D6[1]=36,D6[0]=39,D6[1]=41
	VMOV D7, R4, R5   //D7[0]=43,D7[1]=44,D7[0]=45,D7[1]=45
				
	//O[15]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D5[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q12
	VMOV Q1, Q13
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D6[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D3, D7[3]
	
	VPUSH {Q7}

	
	//O[14]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D4[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D7[3]
	VPUSH {Q7}
	
	//O[13]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D7[2]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D7[1]
	VPUSH {Q7}
	
	//O[12]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D7[0]
	VPUSH {Q7}

	//O[11]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D6[0]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D5[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D5[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D6[3]
	VPUSH {Q7}

	//O[10]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D6[2]
	VPUSH {Q7}

	//O[9]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D7[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D6[1]
	VPUSH {Q7}

	//O[8]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D7[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D5[2]
	VMLAL.S16 Q7, D3, D6[0]
	VPUSH {Q7}
	
	//O[7]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D7[3]
	VMLAL.S16 Q7, D3, D4[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D7[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D5[3]
	VPUSH {Q7}
	
	//O[6]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D5[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D5[2]
	VPUSH {Q7}


	//O[5]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D4[3]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[1]
	VPUSH {Q7}

	
	//O[4]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D7[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D5[0]
	VPUSH {Q7}

	
	//O[3]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[0]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D4[3]
	VPUSH {Q7}

	
	//O[2]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLAL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D4[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D6[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D6[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLAL.S16 Q7, D3, D4[2]
	VPUSH {Q7}

	//O[1]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D5[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D4[1]
	VPUSH {Q7}

		
	//O[0]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[2]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLAL.S16 Q7, D3, D6[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D4[0]
	VPUSH {Q7}


	//CALCULATE AND STORE DST
	/////////////////////////////////////////
	VPOP {D16-D23}   //Q[8] - Q[11] :O[0]-O[3]
	ADD SP, SP, #320
	VPOP {D24-D31}   //Q[12]- Q[15] :E[0]-E[3]
    //DST[0]-[3]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	MOV R1, R11
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[28]-[31]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	ADD R1, R11, #56
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	

	/////////////////////////////////////////
	SUB SP, SP, #384
	VPOP {D16-D23}   //Q[8] - Q[11] :O[4]-O[7]
	ADD SP, SP, #128
	VPOP {D24-D31}   //Q[12]- Q[15] :E[4]-E[7]
	//DST[4]-[7]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	ADD R1, R11, #8
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[24]-[27]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	ADD R1, R11, #48
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9


	/////////////////////////////////////////
	VPOP {D24-D31}   //Q[12]- Q[15] :E[8]-E[11]
	SUB SP, SP, #256
	VPOP {D16-D23}   //Q[8] - Q[11] :O[8]-O[11]
		
	//DST[8]-[11]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	ADD R1, R11, #16
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[20]-[23]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D1, Q1, #5
	VQRSHRN.S32 D2, Q2, #5
	VQRSHRN.S32 D3, Q3, #5
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3
	
	ADD R1, R11, #40
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	

	/////////////////////////////////////////

	VPOP {D16-D23}   //Q[8] - Q[11] :O[12]-O[15]
	ADD SP, SP, #192	
	VPOP {D24-D31}   //Q[12]- Q[15] :E[12]-E[15]
	
	
	//DST[12]-[15]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D2, Q1, #5
	VQRSHRN.S32 D4, Q2, #5
	VQRSHRN.S32 D6, Q3, #5
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	
    //DST[16]-[19]
	VSUB.S32 Q7, Q12, Q8
	VSUB.S32 Q6, Q13, Q9
	VSUB.S32 Q5, Q14, Q10
	VSUB.S32 Q4, Q15, Q11
	VQRSHRN.S32 D1, Q4, #5
	VQRSHRN.S32 D3, Q5, #5
	VQRSHRN.S32 D5, Q6, #5
	VQRSHRN.S32 D7, Q7, #5
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7
	
	ADD R1, R11, #24
	MOV R9, #64
	VST1.64 {D0, D1}, [R1], R9
	VST1.64 {D2, D3}, [R1], R9
	VST1.64 {D4, D5}, [R1], R9
	VST1.64 {D6, D7}, [R1], R9
	
	ADD SP, SP, #256
.endm

//************************************************************************
//macro: the second inverse transform of 8*32
//************************************************************************
.macro partialButterflyInverse8x32_4rows_shift12_clipx
	
	MOV R9, #64
	
	//FIRST FOUR ROWS
	
	VLD1.64 D0, [R0], R9  //SRC[0]
	VLD1.64 D1, [R0], R9  //SRC[LINE]
	VLD1.64 D2, [R0], R9  //SRC[2*LINE]
	VLD1.64 D3, [R0], R9  //SRC[3*LINE]
	VLD1.64 D4, [R0], R9  //SRC[4*LINE]
	VLD1.64 D5, [R0], R9  //SRC[5*LINE]
	VLD1.64 D6, [R0], R9  //SRC[6*LINE]
	VLD1.64 D7, [R0], R9  //SRC[7*LINE]
	VMOV D8, D7 //SAVE D7
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=44
	
	//E[0]
	VMULL.S16 Q5, D0, D7[0]
	VMLAL.S16 Q5, D2, D7[3]
	VMLAL.S16 Q5, D4, D7[0]
	VMLAL.S16 Q5, D6, D7[2]
	//E[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[2]
	VMLSL.S16 Q6, D4, D7[0]
	VMLSL.S16 Q6, D6, D7[3]
	//E[2]
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D4, D7[0]
	VMLAL.S16 Q7, D6, D7[3]	
	//E[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D2, D7[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D6, D7[2]	

	VMOV D7, D8
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D6, R4, R5  //D6[0]=9, D6[1]=25, D6[2]=38, D6[3]=44
	
	//O[0]
	VMULL.S16 Q9, D1, D6[3]
	VMLAL.S16 Q9, D3, D6[2]
	VMLAL.S16 Q9, D5, D6[1]
	VMLAL.S16 Q9, D7, D6[0]	
	//O[1]
	VMULL.S16 Q10, D1, D6[2]
	VMLSL.S16 Q10, D3, D6[0]
	VMLSL.S16 Q10, D5, D6[3]
	VMLSL.S16 Q10, D7, D6[1]	
	//O[2]
	VMULL.S16 Q11, D1, D6[1]
	VMLSL.S16 Q11, D3, D6[3]
	VMLAL.S16 Q11, D5, D6[0]
	VMLAL.S16 Q11, D7, D6[2]	
	//O[3]
	VMULL.S16 Q12, D1, D6[0]
	VMLSL.S16 Q12, D3, D6[1]
	VMLAL.S16 Q12, D5, D6[2]
	VMLSL.S16 Q12, D7, D6[3]
	
	//CALCULATE DST
	VADD.S32 Q0, Q5, Q9     //DST[0]
	VADD.S32 Q1, Q6, Q10    //DST[1]
	VADD.S32 Q2, Q7, Q11    //DST[2]
	VADD.S32 Q3, Q8, Q12    //DST[3]
	VSUB.S32 Q4, Q8, Q12    //DST[4]
	VSUB.S32 Q13, Q7, Q11    //DST[5]
	VSUB.S32 Q14, Q6, Q10    //DST[6]
	VSUB.S32 Q15, Q5, Q9    //DST[7]
	
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D2, Q1, #12
	VQRSHRN.S32 D4, Q2, #12
	VQRSHRN.S32 D6, Q3, #12
	VQRSHRN.S32 D1, Q4, #12
	VQRSHRN.S32 D3, Q13, #12
	VQRSHRN.S32 D5, Q14, #12
	VQRSHRN.S32 D7, Q15, #12
	
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7

	//clip
	MVN R6, #0
	LSL R7, R6, R2 	//minvalue R8
	MVN R8, R7 		//maxvalue R9
	VDUP.16 Q9,R7	//minval vector Q9
	VDUP.16 Q10,R8  //maxval vector Q10
	VMAX.S16 Q0, Q0, Q9
	VMAX.s16 Q1, Q1, Q9
	VMAX.S16 Q2, Q2, Q9
	VMAX.s16 Q3, Q3, Q9
	VMIN.S16 Q0, Q0, Q10
	VMIN.S16 Q1, Q1, Q10
	VMIN.S16 Q2, Q2, Q10
	VMIN.S16 Q3, Q3, Q10

	VST1.64 {D0, D1, D2, D3}, [R1]!
	VST1.64 {D4, D5, D6, D7}, [R1]!		  
.endm

//***********************************************************************
//void partialButterflyInverse32x8_shift5_clip16_neon(Short *src,Short *dst)
//src: transform coefficients
//dst: the result   
//detail: shift=5 clip=16
//***********************************************************************
function partialButterflyInverse32x8_shift5_clip16_neon
	PUSH {R4-R12, LR}	
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32x8_4rows_shift5_clip16
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32x8_4rows_shift5_clip16
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void partialButterflyInverse8x32_shift12_clipx_neon(Short *src,Short *dst,int clip)
//src: transform coefficients
//dst: the result   
//detail: shift=5 clip=16
//***********************************************************************
function partialButterflyInverse8x32_shift12_clipx_neon
	PUSH {R4-R12, LR}	
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST
	SUB R2, R2, #1 

	///////////////////1ST  4 ROWS///////////////////
	
	partialButterflyInverse8x32_4rows_shift12_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8x32_4rows_shift12_clipx

	///////////////////3ED  4 ROWS///////////////////

	ADD R0, R10, #16
	partialButterflyInverse8x32_4rows_shift12_clipx
	
	///////////////////4ED  4 ROWS///////////////////

	ADD R0, R10, #24
	partialButterflyInverse8x32_4rows_shift12_clipx
	

	///////////////////5ED  4 ROWS///////////////////

	ADD R0, R10, #32
	partialButterflyInverse8x32_4rows_shift12_clipx
	

	///////////////////6ED  4 ROWS///////////////////

	ADD R0, R10, #40
	partialButterflyInverse8x32_4rows_shift12_clipx
	

	///////////////////7ED  4 ROWS///////////////////

	ADD R0, R10, #48
	partialButterflyInverse8x32_4rows_shift12_clipx
	

	///////////////////8ED  4 ROWS///////////////////

	ADD R0, R10, #56
	partialButterflyInverse8x32_4rows_shift12_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void idct_8x32_shift12_neon(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 12 in code  
//clip: clip
//***********************************************************************
function idct_8x32_shift12_neon
	PUSH {R4-R12, LR}	

	////////////////////alloc mem ///////////////////

	SUB R12, SP, #1440

	MOV R8, R0
	MOV R1, R12

	//////////////////////////////////////////////////
	///////////////////first trans////////////////////
	//////////////////////////////////////////////////

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32x8_4rows_shift5_clip16
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32x8_4rows_shift5_clip16

	//////////////////////////////////////////////////
	///////////////////second trans///////////////////
	//////////////////////////////////////////////////

	MOV R0, R12
	MOV R1, R8
	SUB R2, R2, #1
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST

	///////////////////1ST  4 ROWS///////////////////
	
	partialButterflyInverse8x32_4rows_shift12_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8x32_4rows_shift12_clipx

	///////////////////3ED  4 ROWS///////////////////

	ADD R0, R10, #16
	partialButterflyInverse8x32_4rows_shift12_clipx
	
	///////////////////4ED  4 ROWS///////////////////

	ADD R0, R10, #24
	partialButterflyInverse8x32_4rows_shift12_clipx
	

	///////////////////5ED  4 ROWS///////////////////

	ADD R0, R10, #32
	partialButterflyInverse8x32_4rows_shift12_clipx
	

	///////////////////6ED  4 ROWS///////////////////

	ADD R0, R10, #40
	partialButterflyInverse8x32_4rows_shift12_clipx
	

	///////////////////7ED  4 ROWS///////////////////

	ADD R0, R10, #48
	partialButterflyInverse8x32_4rows_shift12_clipx
	

	///////////////////8ED  4 ROWS///////////////////

	ADD R0, R10, #56
	partialButterflyInverse8x32_4rows_shift12_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

//************************************************************************
//macro: the second inverse transform of 8*32
//************************************************************************
.macro partialButterflyInverse8x32_4rows_shift11_clipx
	
	MOV R9, #64
	
	//FIRST FOUR ROWS
	
	VLD1.64 D0, [R0], R9  //SRC[0]
	VLD1.64 D1, [R0], R9  //SRC[LINE]
	VLD1.64 D2, [R0], R9  //SRC[2*LINE]
	VLD1.64 D3, [R0], R9  //SRC[3*LINE]
	VLD1.64 D4, [R0], R9  //SRC[4*LINE]
	VLD1.64 D5, [R0], R9  //SRC[5*LINE]
	VLD1.64 D6, [R0], R9  //SRC[6*LINE]
	VLD1.64 D7, [R0], R9  //SRC[7*LINE]
	VMOV D8, D7 //SAVE D7
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=44
	
	//E[0]
	VMULL.S16 Q5, D0, D7[0]
	VMLAL.S16 Q5, D2, D7[3]
	VMLAL.S16 Q5, D4, D7[0]
	VMLAL.S16 Q5, D6, D7[2]
	//E[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[2]
	VMLSL.S16 Q6, D4, D7[0]
	VMLSL.S16 Q6, D6, D7[3]
	//E[2]
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D4, D7[0]
	VMLAL.S16 Q7, D6, D7[3]	
	//E[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D2, D7[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D6, D7[2]	

	VMOV D7, D8
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D6, R4, R5  //D6[0]=9, D6[1]=25, D6[2]=38, D6[3]=44
	
	//O[0]
	VMULL.S16 Q9, D1, D6[3]
	VMLAL.S16 Q9, D3, D6[2]
	VMLAL.S16 Q9, D5, D6[1]
	VMLAL.S16 Q9, D7, D6[0]	
	//O[1]
	VMULL.S16 Q10, D1, D6[2]
	VMLSL.S16 Q10, D3, D6[0]
	VMLSL.S16 Q10, D5, D6[3]
	VMLSL.S16 Q10, D7, D6[1]	
	//O[2]
	VMULL.S16 Q11, D1, D6[1]
	VMLSL.S16 Q11, D3, D6[3]
	VMLAL.S16 Q11, D5, D6[0]
	VMLAL.S16 Q11, D7, D6[2]	
	//O[3]
	VMULL.S16 Q12, D1, D6[0]
	VMLSL.S16 Q12, D3, D6[1]
	VMLAL.S16 Q12, D5, D6[2]
	VMLSL.S16 Q12, D7, D6[3]
	
	//CALCULATE DST
	VADD.S32 Q0, Q5, Q9     //DST[0]
	VADD.S32 Q1, Q6, Q10    //DST[1]
	VADD.S32 Q2, Q7, Q11    //DST[2]
	VADD.S32 Q3, Q8, Q12    //DST[3]
	VSUB.S32 Q4, Q8, Q12    //DST[4]
	VSUB.S32 Q13, Q7, Q11    //DST[5]
	VSUB.S32 Q14, Q6, Q10    //DST[6]
	VSUB.S32 Q15, Q5, Q9    //DST[7]
	
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D2, Q1, #11
	VQRSHRN.S32 D4, Q2, #11
	VQRSHRN.S32 D6, Q3, #11
	VQRSHRN.S32 D1, Q4, #11
	VQRSHRN.S32 D3, Q13, #11
	VQRSHRN.S32 D5, Q14, #11
	VQRSHRN.S32 D7, Q15, #11
	
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7

	//clip
	MVN R6, #0
	LSL R7, R6, R2 	//minvalue R8
	MVN R8, R7 		//maxvalue R9
	VDUP.16 Q9,R7	//minval vector Q9
	VDUP.16 Q10,R8  //maxval vector Q10
	VMAX.S16 Q0, Q0, Q9
	VMAX.s16 Q1, Q1, Q9
	VMAX.S16 Q2, Q2, Q9
	VMAX.s16 Q3, Q3, Q9
	VMIN.S16 Q0, Q0, Q10
	VMIN.S16 Q1, Q1, Q10
	VMIN.S16 Q2, Q2, Q10
	VMIN.S16 Q3, Q3, Q10

	VST1.64 {D0, D1, D2, D3}, [R1]!
	VST1.64 {D4, D5, D6, D7}, [R1]!		  
.endm


//***********************************************************************
//void partialButterflyInverse8x32_shift11_clipx_neon(Short *src,Short *dst,int clip)
//src: transform coefficients
//dst: the result   
//detail: shift=5 clip=16
//***********************************************************************
function partialButterflyInverse8x32_shift11_clipx_neon
	PUSH {R4-R12, LR}	
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST
	SUB R2, R2, #1 

	///////////////////1ST  4 ROWS///////////////////
	
	partialButterflyInverse8x32_4rows_shift11_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8x32_4rows_shift11_clipx

	///////////////////3ED  4 ROWS///////////////////

	ADD R0, R10, #16
	partialButterflyInverse8x32_4rows_shift11_clipx
	
	///////////////////4ED  4 ROWS///////////////////

	ADD R0, R10, #24
	partialButterflyInverse8x32_4rows_shift11_clipx
	

	///////////////////5ED  4 ROWS///////////////////

	ADD R0, R10, #32
	partialButterflyInverse8x32_4rows_shift11_clipx
	

	///////////////////6ED  4 ROWS///////////////////

	ADD R0, R10, #40
	partialButterflyInverse8x32_4rows_shift11_clipx
	

	///////////////////7ED  4 ROWS///////////////////

	ADD R0, R10, #48
	partialButterflyInverse8x32_4rows_shift11_clipx
	

	///////////////////8ED  4 ROWS///////////////////

	ADD R0, R10, #56
	partialButterflyInverse8x32_4rows_shift11_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void idct_8x32_shift11_neon(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code  
//clip: clip
//***********************************************************************
function idct_8x32_shift11_neon
	PUSH {R4-R12, LR}	

	////////////////////alloc mem ///////////////////

	SUB R12, SP, #1440

	MOV R8, R0
	MOV R1, R12

	//////////////////////////////////////////////////
	///////////////////first trans////////////////////
	//////////////////////////////////////////////////

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32x8_4rows_shift5_clip16
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32x8_4rows_shift5_clip16

	//////////////////////////////////////////////////
	///////////////////second trans///////////////////
	//////////////////////////////////////////////////

	MOV R0, R12
	MOV R1, R8
	SUB R2, R2, #1
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST

	///////////////////1ST  4 ROWS///////////////////
	
	partialButterflyInverse8x32_4rows_shift11_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8x32_4rows_shift11_clipx

	///////////////////3ED  4 ROWS///////////////////

	ADD R0, R10, #16
	partialButterflyInverse8x32_4rows_shift11_clipx
	
	///////////////////4ED  4 ROWS///////////////////

	ADD R0, R10, #24
	partialButterflyInverse8x32_4rows_shift11_clipx
	

	///////////////////5ED  4 ROWS///////////////////

	ADD R0, R10, #32
	partialButterflyInverse8x32_4rows_shift11_clipx
	

	///////////////////6ED  4 ROWS///////////////////

	ADD R0, R10, #40
	partialButterflyInverse8x32_4rows_shift11_clipx
	

	///////////////////7ED  4 ROWS///////////////////

	ADD R0, R10, #48
	partialButterflyInverse8x32_4rows_shift11_clipx
	

	///////////////////8ED  4 ROWS///////////////////

	ADD R0, R10, #56
	partialButterflyInverse8x32_4rows_shift11_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


//************************************************************************
//macro: the first inverse transform of 32*8
//************************************************************************
.macro partialButterflyInverse8x32_4rows_shift5_clip16
	
	MOV R9, #64
	
	//FIRST FOUR ROWS
	
	VLD1.64 D0, [R0], R9  //SRC[0]
	VLD1.64 D1, [R0], R9  //SRC[LINE]
	VLD1.64 D2, [R0], R9  //SRC[2*LINE]
	VLD1.64 D3, [R0], R9  //SRC[3*LINE]
	VLD1.64 D4, [R0], R9  //SRC[4*LINE]
	VLD1.64 D5, [R0], R9  //SRC[5*LINE]
	VLD1.64 D6, [R0], R9  //SRC[6*LINE]
	VLD1.64 D7, [R0], R9  //SRC[7*LINE]
	VMOV D8, D7 //SAVE D7
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=44
	
	//E[0]
	VMULL.S16 Q5, D0, D7[0]
	VMLAL.S16 Q5, D2, D7[3]
	VMLAL.S16 Q5, D4, D7[0]
	VMLAL.S16 Q5, D6, D7[2]
	//E[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[2]
	VMLSL.S16 Q6, D4, D7[0]
	VMLSL.S16 Q6, D6, D7[3]
	//E[2]
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D4, D7[0]
	VMLAL.S16 Q7, D6, D7[3]	
	//E[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D2, D7[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D6, D7[2]	

	VMOV D7, D8
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D6, R4, R5  //D6[0]=9, D6[1]=25, D6[2]=38, D6[3]=44
	
	//O[0]
	VMULL.S16 Q9, D1, D6[3]
	VMLAL.S16 Q9, D3, D6[2]
	VMLAL.S16 Q9, D5, D6[1]
	VMLAL.S16 Q9, D7, D6[0]	
	//O[1]
	VMULL.S16 Q10, D1, D6[2]
	VMLSL.S16 Q10, D3, D6[0]
	VMLSL.S16 Q10, D5, D6[3]
	VMLSL.S16 Q10, D7, D6[1]	
	//O[2]
	VMULL.S16 Q11, D1, D6[1]
	VMLSL.S16 Q11, D3, D6[3]
	VMLAL.S16 Q11, D5, D6[0]
	VMLAL.S16 Q11, D7, D6[2]	
	//O[3]
	VMULL.S16 Q12, D1, D6[0]
	VMLSL.S16 Q12, D3, D6[1]
	VMLAL.S16 Q12, D5, D6[2]
	VMLSL.S16 Q12, D7, D6[3]
	
	//CALCULATE DST
	VADD.S32 Q0, Q5, Q9     //DST[0]
	VADD.S32 Q1, Q6, Q10    //DST[1]
	VADD.S32 Q2, Q7, Q11    //DST[2]
	VADD.S32 Q3, Q8, Q12    //DST[3]
	VSUB.S32 Q4, Q8, Q12    //DST[4]
	VSUB.S32 Q13, Q7, Q11    //DST[5]
	VSUB.S32 Q14, Q6, Q10    //DST[6]
	VSUB.S32 Q15, Q5, Q9    //DST[7]
	
	VQRSHRN.S32 D0, Q0, #5
	VQRSHRN.S32 D2, Q1, #5
	VQRSHRN.S32 D4, Q2, #5
	VQRSHRN.S32 D6, Q3, #5
	VQRSHRN.S32 D1, Q4, #5
	VQRSHRN.S32 D3, Q13, #5
	VQRSHRN.S32 D5, Q14, #5
	VQRSHRN.S32 D7, Q15, #5
	
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7


	VST1.64 {D0, D1, D2, D3}, [R1]!
	VST1.64 {D4, D5, D6, D7}, [R1]!		  
.endm

//************************************************************************
//macro: the second inverse transform of 32*8
//************************************************************************
.macro partButterflyInverse32x8_4rows_shift12_clipx

	//CALCULATE EEE
	MOV R0, R10
	MOV R9, #128
	VLD1.64 D0, [R0], R9   //src[ 0       ]
	VLD1.64 D1, [R0], R9   //src[ 8*line  ]
	VLD1.64 D2, [R0], R9   //src[ 16*line ]
	VLD1.64 D3, [R0], R9   //src[ 24*line ]
	
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=42
	
	
	VMULL.S16 Q4, D1, D7[3]
	VMLAL.S16 Q4, D3, D7[2]     //EEEO[0]
	VMULL.S16 Q5, D1, D7[2]
	VMLSL.S16 Q5, D3, D7[3]     //EEEO[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[0]     //EEEE[0]	
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[0]     //EEEE[1]
	
	VADD.S32 Q8, Q6, Q4         //EEE[0]
	VADD.S32 Q9, Q7, Q5         //EEE[1]
	VSUB.S32 Q10, Q7, Q5        //EEE[2]
	VSUB.S32 Q11, Q6, Q4        //EEE[3]
	
	//CALCULATE EEO
	ADD R0, R10, #64
	MOV R9, #128	
	VLD1.64 D0, [R0], R9   //src[ 4*line  ]
	VLD1.64 D1, [R0], R9   //src[ 12*line ]
	VLD1.64 D2, [R0], R9   //src[ 20*line ]
	VLD1.64 D3, [R0], R9   //src[ 28*line ]	
	
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D7, R4, R5  //D7[0]=9, D7[1]=25, D7[2]=38, D7[3]=44
	
	VMULL.S16 Q12, D0, D7[3]
	VMLAL.S16 Q12, D1, D7[2]
	VMLAL.S16 Q12, D2, D7[1]
	VMLAL.S16 Q12, D3, D7[0]    //EEO[0]
	
	VMULL.S16 Q13, D0, D7[2]
	VMLSL.S16 Q13, D1, D7[0]
	VMLSL.S16 Q13, D2, D7[3]
	VMLSL.S16 Q13, D3, D7[1]    //EEO[1]
	
	VMULL.S16 Q14, D0, D7[1]
	VMLSL.S16 Q14, D1, D7[3]
	VMLAL.S16 Q14, D2, D7[0]
	VMLAL.S16 Q14, D3, D7[2]    //EEO[2]
		
	VMULL.S16 Q15, D0, D7[0]
	VMLSL.S16 Q15, D1, D7[1]
	VMLAL.S16 Q15, D2, D7[2]
	VMLSL.S16 Q15, D3, D7[3]    //EEO[3]
	
	//CALCULATE EE
	VADD.S32 Q0, Q8, Q12         //EE[0]
	VADD.S32 Q1, Q9, Q13         //EE[1]
	VADD.S32 Q2, Q10, Q14        //EE[2]
	VADD.S32 Q3, Q11, Q15        //EE[3]
	VSUB.S32 Q4, Q11, Q15        //EE[4]
	VSUB.S32 Q5, Q10, Q14        //EE[5]
	VSUB.S32 Q6, Q9, Q13         //EE[6]
	VSUB.S32 Q7, Q8, Q12         //EE[7]
	
	VPUSH {D0-D15}  //PUSH EE
	
	//CALCULATE EO
	ADD R0, R10, #32
	MOV R9, #64
	VLD1.64 D0, [R0], R9   //src[ 2*line ]
	VLD1.64 D1, [R0], R9   //src[ 6*line ]
	VLD1.64 D2, [R0], R9   //src[10*line ]
	VLD1.64 D3, [R0], R9   //src[14*line ]
	VLD1.64 D4, [R0], R9   //src[18*line ]
	VLD1.64 D5, [R0], R9   //src[22*line ]
	VLD1.64 D6, [R0], R9   //src[26*line ]
	VLD1.64 D7, [R0], R9   //src[30*line ]
	
	VMOV Q10, Q2
	VMOV Q11, Q3
	
	MOV R7, #4
	MOVT R7, #13
	MOV R3, #21
	MOVT R3, #29
	MOV R4, #35
	MOVT R4, #40
	MOV R5, #43
	MOVT R5, #45
	VMOV D6, R7, R3 //D6[0]=4 ,D6[1]=13 ,D6[2]=21,D6[3]=29
	VMOV D7, R4, R5 //D7[0]=35,D7[1]=40 ,D7[2]=43,D7[3]=45

	//EO[7]
	VMULL.S16 Q8, D0, D6[0]
	VMLSL.S16 Q8, D1, D6[1]
	VMLAL.S16 Q8, D2, D6[2]
	VMLSL.S16 Q8, D3, D6[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[2]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[6]
	VMULL.S16 Q8, D0, D6[1]
	VMLSL.S16 Q8, D1, D7[0]
	VMLAL.S16 Q8, D2, D7[3]
	VMLSL.S16 Q8, D3, D7[1]
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D6[0]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D7[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[5]
	VMULL.S16 Q8, D0, D6[2]
	VMLSL.S16 Q8, D1, D7[3]
	VMLAL.S16 Q8, D2, D6[3]
	VMLAL.S16 Q8, D3, D6[1]
	VMLSL.S16 Q8, D4, D7[2]
	VMLAL.S16 Q8, D5, D7[0]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q10
	VPUSH {Q8}

	//EO[4]
	VMULL.S16 Q8, D0, D6[3]
	VMLSL.S16 Q8, D1, D7[1]
	VMLSL.S16 Q8, D2, D6[1]
	VMLAL.S16 Q8, D3, D7[3]
	VMLSL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D7[0]	
	VMOV Q2, Q10
	VPUSH {Q8}	

	//EO[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D1, D6[2]
	VMLSL.S16 Q8, D2, D7[2]
	VMLAL.S16 Q8, D3, D6[0]
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[1]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D6[3]	
	VMOV Q2, Q10	
	VPUSH {Q8}
	
	
	//EO[2]
	VMULL.S16 Q8, D0, D7[1]
	VMLAL.S16 Q8, D1, D6[0]
	VMLSL.S16 Q8, D2, D7[0]
	VMLSL.S16 Q8, D3, D7[2]
	VMLSL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[3]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[1]
	VMULL.S16 Q8, D0, D7[2]
	VMLAL.S16 Q8, D1, D6[3]
	VMLAL.S16 Q8, D2, D6[0]
	VMLSL.S16 Q8, D3, D6[2]
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D6[1]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	
	//EO[0]
	VMULL.S16 Q8, D0, D7[3]
	VMLAL.S16 Q8, D1, D7[2]
	VMLAL.S16 Q8, D2, D7[1]
	VMLAL.S16 Q8, D3, D7[0]
	VMLAL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[0]
	VPUSH {Q8}


	//CALCULATE E
	VPOP  {D16-D23}               //POP EO[0] - EO[3]
	ADD SP, SP, #64
	VPOP  {D24-D31}               //POP EE[0] - EE[3]
	VADD.S32 Q0, Q12, Q8          //E[0]
	VADD.S32 Q1, Q13, Q9          //E[1]
	VADD.S32 Q2, Q14, Q10         //E[2]
	VADD.S32 Q3, Q15, Q11         //E[3]
	VSUB.S32 Q4, Q15, Q11         //E[12] 
	VSUB.S32 Q5, Q14, Q10         //E[13] 
	VSUB.S32 Q6, Q13, Q9          //E[14] 
	VSUB.S32 Q7, Q12, Q8          //E[15] 
	
	SUB SP, SP, #192
	VPUSH {D0-D15}   //PUSH E[0]-E[3], E[12]-E[15]
	
	ADD SP, SP, #192
	VPOP {D16-D23}                //EO[4] - EO[7]
	ADD SP, SP, #64
	VPOP {D24-D31}                //EE[4] - EE[7]
	VADD.S32 Q0, Q12, Q8          //E[4]
	VADD.S32 Q1, Q13, Q9          //E[5]
	VADD.S32 Q2, Q14, Q10         //E[6]
	VADD.S32 Q3, Q15, Q11         //E[7]
	VSUB.S32 Q4, Q15, Q11         //E[8] 
	VSUB.S32 Q5, Q14, Q10         //E[9] 
	VSUB.S32 Q6, Q13, Q9          //E[10] 
	VSUB.S32 Q7, Q12, Q8          //E[11]
		
	SUB SP, SP, #384
	VPUSH {D0-D15}  //PUSH E[4] - E[11]



	//CALCULATE O
	ADD R0, R10, #16
	MOV R9, #32
	
	VLD1.64 D16, [R0], R9    //src[ 1*line ]
	VLD1.64 D17, [R0], R9    //src[ 3*line ]
	VLD1.64 D18, [R0], R9    //src[ 5*line ]
	VLD1.64 D19, [R0], R9    //src[ 7*line ]
	VLD1.64 D20, [R0], R9    //src[ 9*line ]
	VLD1.64 D21, [R0], R9    //src[11*line ]
	VLD1.64 D22, [R0], R9    //src[13*line ]
	VLD1.64 D23, [R0], R9    //src[15*line ]
	VLD1.64 D24, [R0], R9    //src[17*line ]
	VLD1.64 D25, [R0], R9    //src[19*line ]
	VLD1.64 D26, [R0], R9    //src[21*line ]
	VLD1.64 D27, [R0], R9    //src[23*line ]
	VLD1.64 D28, [R0], R9    //src[25*line ]
	VLD1.64 D29, [R0], R9    //src[27*line ]
	VLD1.64 D30, [R0], R9    //src[29*line ]
	VLD1.64 D31, [R0], R9    //src[31*line ]

	MOV R7,  #2
	MOVT R7, #7
	MOV R3, #11
	MOVT R3, #15
	MOV R4, #19
	MOVT R4, #23
	MOV R5, #27
	MOVT R5, #30
	VMOV D4, R7, R3   //D4 [0]=2 ,D4[1] =7,D4 [2]=11,D4[3] =15
	VMOV D5, R4, R5   //D5 [0]=19,D5[1] =23,D5 [0]=27,D5[1] =30

	MOV R7, #34
	MOVT R7, #36
	MOV R3, #39
	MOVT R3, #41
	MOV R4, #43
	MOVT R4, #44
	MOV R5, #45
	MOVT R5, #45
	VMOV D6, R7, R3   //D6[0]=34,D6[1]=36,D6[0]=39,D6[1]=41
	VMOV D7, R4, R5   //D7[0]=43,D7[1]=44,D7[0]=45,D7[1]=45
				
	//O[15]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D5[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q12
	VMOV Q1, Q13
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D6[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D3, D7[3]
	
	VPUSH {Q7}

	
	//O[14]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D4[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D7[3]
	VPUSH {Q7}
	
	//O[13]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D7[2]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D7[1]
	VPUSH {Q7}
	
	//O[12]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D7[0]
	VPUSH {Q7}

	//O[11]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D6[0]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D5[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D5[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D6[3]
	VPUSH {Q7}

	//O[10]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D6[2]
	VPUSH {Q7}

	//O[9]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D7[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D6[1]
	VPUSH {Q7}

	//O[8]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D7[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D5[2]
	VMLAL.S16 Q7, D3, D6[0]
	VPUSH {Q7}
	
	//O[7]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D7[3]
	VMLAL.S16 Q7, D3, D4[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D7[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D5[3]
	VPUSH {Q7}
	
	//O[6]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D5[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D5[2]
	VPUSH {Q7}


	//O[5]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D4[3]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[1]
	VPUSH {Q7}

	
	//O[4]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D7[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D5[0]
	VPUSH {Q7}

	
	//O[3]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[0]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D4[3]
	VPUSH {Q7}

	
	//O[2]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLAL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D4[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D6[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D6[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLAL.S16 Q7, D3, D4[2]
	VPUSH {Q7}

	//O[1]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D5[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D4[1]
	VPUSH {Q7}

		
	//O[0]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[2]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLAL.S16 Q7, D3, D6[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D4[0]
	VPUSH {Q7}


	//CALCULATE AND STORE DST
	/////////////////////////////////////////
	VPOP {D16-D23}   //Q[8] - Q[11] :O[0]-O[3]
	ADD SP, SP, #320
	VPOP {D24-D31}   //Q[12]- Q[15] :E[0]-E[3]
    //DST[0]-[3]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	MOV R1, R11
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[28]-[31]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #56
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	

	/////////////////////////////////////////
	SUB SP, SP, #384
	VPOP {D16-D23}   //Q[8] - Q[11] :O[4]-O[7]
	ADD SP, SP, #128
	VPOP {D24-D31}   //Q[12]- Q[15] :E[4]-E[7]
	//DST[4]-[7]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #8
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[24]-[27]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #48
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9


	/////////////////////////////////////////
	VPOP {D24-D31}   //Q[12]- Q[15] :E[8]-E[11]
	SUB SP, SP, #256
	VPOP {D16-D23}   //Q[8] - Q[11] :O[8]-O[11]
		
	//DST[8]-[11]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #16
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[20]-[23]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D1, Q1, #12
	VQRSHRN.S32 D2, Q2, #12
	VQRSHRN.S32 D3, Q3, #12
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #40
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	

	/////////////////////////////////////////

	VPOP {D16-D23}   //Q[8] - Q[11] :O[12]-O[15]
	ADD SP, SP, #192	
	VPOP {D24-D31}   //Q[12]- Q[15] :E[12]-E[15]
	
	
	//DST[12]-[15]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #12
	VQRSHRN.S32 D2, Q1, #12
	VQRSHRN.S32 D4, Q2, #12
	VQRSHRN.S32 D6, Q3, #12
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	
    //DST[16]-[19]
	VSUB.S32 Q7, Q12, Q8
	VSUB.S32 Q6, Q13, Q9
	VSUB.S32 Q5, Q14, Q10
	VSUB.S32 Q4, Q15, Q11
	VQRSHRN.S32 D1, Q4, #12
	VQRSHRN.S32 D3, Q5, #12
	VQRSHRN.S32 D5, Q6, #12
	VQRSHRN.S32 D7, Q7, #12
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMAX.S16 Q2, Q2, Q5
	VMAX.S16 Q3, Q3, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	VMIN.S16 Q2, Q2, Q6
	VMIN.S16 Q3, Q3, Q6
	
	ADD R1, R11, #24
	MOV R9, #64
	VST1.64 {D0, D1}, [R1], R9
	VST1.64 {D2, D3}, [R1], R9
	VST1.64 {D4, D5}, [R1], R9
	VST1.64 {D6, D7}, [R1], R9
	
	ADD SP, SP, #256
.endm

//***********************************************************************
//void partialButterflyInverse8x32_shift5_clip16_neon(Short *src,Short *dst)
//src: transform coefficients
//dst: the result   
//detail: shift=5 clip=16
//***********************************************************************
function partialButterflyInverse8x32_shift5_clip16_neon
	PUSH {R4-R12, LR}	
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST

	///////////////////1ST  4 ROWS///////////////////
	
	partialButterflyInverse8x32_4rows_shift5_clip16
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8x32_4rows_shift5_clip16

	///////////////////3ED  4 ROWS///////////////////

	ADD R0, R10, #16
	partialButterflyInverse8x32_4rows_shift5_clip16
	
	///////////////////4ED  4 ROWS///////////////////

	ADD R0, R10, #24
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////5ED  4 ROWS///////////////////

	ADD R0, R10, #32
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////6ED  4 ROWS///////////////////

	ADD R0, R10, #40
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////7ED  4 ROWS///////////////////

	ADD R0, R10, #48
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////8ED  4 ROWS///////////////////

	ADD R0, R10, #56
	partialButterflyInverse8x32_4rows_shift5_clip16
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void partialButterflyInverse32x8_shift12_clipx_neon(Short *src,Short *dst,int clip)
//src: transform coefficients
//dst: the result   
//detail: shift=2 clip=x
//***********************************************************************
function partialButterflyInverse32x8_shift12_clipx_neon
	PUSH {R4-R12, LR}	
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 
	SUB R2, R2, #1

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32x8_4rows_shift12_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32x8_4rows_shift12_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void idct_32x8_shift12_neon(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 12 in code  
//clip: clip
//***********************************************************************
function idct_32x8_shift12_neon
	PUSH {R4-R12, LR}	

	////////////////////alloc mem ///////////////////

	SUB R12, SP, #1440

	MOV R8, R0
	MOV R1, R12

	//////////////////////////////////////////////////
	///////////////////first trans////////////////////
	//////////////////////////////////////////////////

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST

	///////////////////1ST  4 ROWS///////////////////
	
	partialButterflyInverse8x32_4rows_shift5_clip16
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8x32_4rows_shift5_clip16

	///////////////////3ED  4 ROWS///////////////////

	ADD R0, R10, #16
	partialButterflyInverse8x32_4rows_shift5_clip16
	
	///////////////////4ED  4 ROWS///////////////////

	ADD R0, R10, #24
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////5ED  4 ROWS///////////////////

	ADD R0, R10, #32
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////6ED  4 ROWS///////////////////

	ADD R0, R10, #40
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////7ED  4 ROWS///////////////////

	ADD R0, R10, #48
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////8ED  4 ROWS///////////////////

	ADD R0, R10, #56
	partialButterflyInverse8x32_4rows_shift5_clip16
	
	//////////////////////////////////////////////////
	///////////////////second trans///////////////////
	//////////////////////////////////////////////////

	MOV R0, R12
	MOV R1, R8
	SUB R2, R2, #1
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32x8_4rows_shift12_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32x8_4rows_shift12_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


//************************************************************************
//macro: the second inverse transform of 32*8
//************************************************************************
.macro partButterflyInverse32x8_4rows_shift11_clipx

	//CALCULATE EEE
	MOV R0, R10
	MOV R9, #128
	VLD1.64 D0, [R0], R9   //src[ 0       ]
	VLD1.64 D1, [R0], R9   //src[ 8*line  ]
	VLD1.64 D2, [R0], R9   //src[ 16*line ]
	VLD1.64 D3, [R0], R9   //src[ 24*line ]
	
	MOV R4, #32
	MOVT R4, #32
	MOV R5, #17
	MOVT R5, #42
	VMOV D7, R4, R5  //D7[0]=32, D7[1]=32, D7[2]=17, D7[3]=42
	
	
	VMULL.S16 Q4, D1, D7[3]
	VMLAL.S16 Q4, D3, D7[2]     //EEEO[0]
	VMULL.S16 Q5, D1, D7[2]
	VMLSL.S16 Q5, D3, D7[3]     //EEEO[1]
	VMULL.S16 Q6, D0, D7[0]
	VMLAL.S16 Q6, D2, D7[0]     //EEEE[0]	
	VMULL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D2, D7[0]     //EEEE[1]
	
	VADD.S32 Q8, Q6, Q4         //EEE[0]
	VADD.S32 Q9, Q7, Q5         //EEE[1]
	VSUB.S32 Q10, Q7, Q5        //EEE[2]
	VSUB.S32 Q11, Q6, Q4        //EEE[3]
	
	//CALCULATE EEO
	ADD R0, R10, #64
	MOV R9, #128	
	VLD1.64 D0, [R0], R9   //src[ 4*line  ]
	VLD1.64 D1, [R0], R9   //src[ 12*line ]
	VLD1.64 D2, [R0], R9   //src[ 20*line ]
	VLD1.64 D3, [R0], R9   //src[ 28*line ]	
	
	MOV R4, #9
	MOVT R4, #25
	MOV R5, #38
	MOVT R5, #44
	VMOV D7, R4, R5  //D7[0]=9, D7[1]=25, D7[2]=38, D7[3]=44
	
	VMULL.S16 Q12, D0, D7[3]
	VMLAL.S16 Q12, D1, D7[2]
	VMLAL.S16 Q12, D2, D7[1]
	VMLAL.S16 Q12, D3, D7[0]    //EEO[0]
	
	VMULL.S16 Q13, D0, D7[2]
	VMLSL.S16 Q13, D1, D7[0]
	VMLSL.S16 Q13, D2, D7[3]
	VMLSL.S16 Q13, D3, D7[1]    //EEO[1]
	
	VMULL.S16 Q14, D0, D7[1]
	VMLSL.S16 Q14, D1, D7[3]
	VMLAL.S16 Q14, D2, D7[0]
	VMLAL.S16 Q14, D3, D7[2]    //EEO[2]
		
	VMULL.S16 Q15, D0, D7[0]
	VMLSL.S16 Q15, D1, D7[1]
	VMLAL.S16 Q15, D2, D7[2]
	VMLSL.S16 Q15, D3, D7[3]    //EEO[3]
	
	//CALCULATE EE
	VADD.S32 Q0, Q8, Q12         //EE[0]
	VADD.S32 Q1, Q9, Q13         //EE[1]
	VADD.S32 Q2, Q10, Q14        //EE[2]
	VADD.S32 Q3, Q11, Q15        //EE[3]
	VSUB.S32 Q4, Q11, Q15        //EE[4]
	VSUB.S32 Q5, Q10, Q14        //EE[5]
	VSUB.S32 Q6, Q9, Q13         //EE[6]
	VSUB.S32 Q7, Q8, Q12         //EE[7]
	
	VPUSH {D0-D15}  //PUSH EE
	
	//CALCULATE EO
	ADD R0, R10, #32
	MOV R9, #64
	VLD1.64 D0, [R0], R9   //src[ 2*line ]
	VLD1.64 D1, [R0], R9   //src[ 6*line ]
	VLD1.64 D2, [R0], R9   //src[10*line ]
	VLD1.64 D3, [R0], R9   //src[14*line ]
	VLD1.64 D4, [R0], R9   //src[18*line ]
	VLD1.64 D5, [R0], R9   //src[22*line ]
	VLD1.64 D6, [R0], R9   //src[26*line ]
	VLD1.64 D7, [R0], R9   //src[30*line ]
	
	VMOV Q10, Q2
	VMOV Q11, Q3
	
	MOV R7, #4
	MOVT R7, #13
	MOV R3, #21
	MOVT R3, #29
	MOV R4, #35
	MOVT R4, #40
	MOV R5, #43
	MOVT R5, #45
	VMOV D6, R7, R3 //D6[0]=4 ,D6[1]=13 ,D6[2]=21,D6[3]=29
	VMOV D7, R4, R5 //D7[0]=35,D7[1]=40 ,D7[2]=43,D7[3]=45

	//EO[7]
	VMULL.S16 Q8, D0, D6[0]
	VMLSL.S16 Q8, D1, D6[1]
	VMLAL.S16 Q8, D2, D6[2]
	VMLSL.S16 Q8, D3, D6[3]
	VMLAL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[2]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[6]
	VMULL.S16 Q8, D0, D6[1]
	VMLSL.S16 Q8, D1, D7[0]
	VMLAL.S16 Q8, D2, D7[3]
	VMLSL.S16 Q8, D3, D7[1]
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D6[0]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D7[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[5]
	VMULL.S16 Q8, D0, D6[2]
	VMLSL.S16 Q8, D1, D7[3]
	VMLAL.S16 Q8, D2, D6[3]
	VMLAL.S16 Q8, D3, D6[1]
	VMLSL.S16 Q8, D4, D7[2]
	VMLAL.S16 Q8, D5, D7[0]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[1]
	VMOV Q2, Q10
	VPUSH {Q8}

	//EO[4]
	VMULL.S16 Q8, D0, D6[3]
	VMLSL.S16 Q8, D1, D7[1]
	VMLSL.S16 Q8, D2, D6[1]
	VMLAL.S16 Q8, D3, D7[3]
	VMLSL.S16 Q8, D4, D6[0]
	VMLSL.S16 Q8, D5, D7[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[2]
	VMLAL.S16 Q8, D5, D7[0]	
	VMOV Q2, Q10
	VPUSH {Q8}	

	//EO[3]
	VMULL.S16 Q8, D0, D7[0]
	VMLSL.S16 Q8, D1, D6[2]
	VMLSL.S16 Q8, D2, D7[2]
	VMLAL.S16 Q8, D3, D6[0]
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[1]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D6[3]	
	VMOV Q2, Q10	
	VPUSH {Q8}
	
	
	//EO[2]
	VMULL.S16 Q8, D0, D7[1]
	VMLAL.S16 Q8, D1, D6[0]
	VMLSL.S16 Q8, D2, D7[0]
	VMLSL.S16 Q8, D3, D7[2]
	VMLSL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[3]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D7[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	//EO[1]
	VMULL.S16 Q8, D0, D7[2]
	VMLAL.S16 Q8, D1, D6[3]
	VMLAL.S16 Q8, D2, D6[0]
	VMLSL.S16 Q8, D3, D6[2]
	VMLSL.S16 Q8, D4, D7[1]
	VMLSL.S16 Q8, D5, D7[3]
	VMOV Q2, Q11
	VMLSL.S16 Q8, D4, D7[0]
	VMLSL.S16 Q8, D5, D6[1]
	VMOV Q2, Q10
	VPUSH {Q8}
	
	
	//EO[0]
	VMULL.S16 Q8, D0, D7[3]
	VMLAL.S16 Q8, D1, D7[2]
	VMLAL.S16 Q8, D2, D7[1]
	VMLAL.S16 Q8, D3, D7[0]
	VMLAL.S16 Q8, D4, D6[3]
	VMLAL.S16 Q8, D5, D6[2]
	VMOV Q2, Q11
	VMLAL.S16 Q8, D4, D6[1]
	VMLAL.S16 Q8, D5, D6[0]
	VPUSH {Q8}


	//CALCULATE E
	VPOP  {D16-D23}               //POP EO[0] - EO[3]
	ADD SP, SP, #64
	VPOP  {D24-D31}               //POP EE[0] - EE[3]
	VADD.S32 Q0, Q12, Q8          //E[0]
	VADD.S32 Q1, Q13, Q9          //E[1]
	VADD.S32 Q2, Q14, Q10         //E[2]
	VADD.S32 Q3, Q15, Q11         //E[3]
	VSUB.S32 Q4, Q15, Q11         //E[12] 
	VSUB.S32 Q5, Q14, Q10         //E[13] 
	VSUB.S32 Q6, Q13, Q9          //E[14] 
	VSUB.S32 Q7, Q12, Q8          //E[15] 
	
	SUB SP, SP, #192
	VPUSH {D0-D15}   //PUSH E[0]-E[3], E[12]-E[15]
	
	ADD SP, SP, #192
	VPOP {D16-D23}                //EO[4] - EO[7]
	ADD SP, SP, #64
	VPOP {D24-D31}                //EE[4] - EE[7]
	VADD.S32 Q0, Q12, Q8          //E[4]
	VADD.S32 Q1, Q13, Q9          //E[5]
	VADD.S32 Q2, Q14, Q10         //E[6]
	VADD.S32 Q3, Q15, Q11         //E[7]
	VSUB.S32 Q4, Q15, Q11         //E[8] 
	VSUB.S32 Q5, Q14, Q10         //E[9] 
	VSUB.S32 Q6, Q13, Q9          //E[10] 
	VSUB.S32 Q7, Q12, Q8          //E[11]
		
	SUB SP, SP, #384
	VPUSH {D0-D15}  //PUSH E[4] - E[11]



	//CALCULATE O
	ADD R0, R10, #16
	MOV R9, #32
	
	VLD1.64 D16, [R0], R9    //src[ 1*line ]
	VLD1.64 D17, [R0], R9    //src[ 3*line ]
	VLD1.64 D18, [R0], R9    //src[ 5*line ]
	VLD1.64 D19, [R0], R9    //src[ 7*line ]
	VLD1.64 D20, [R0], R9    //src[ 9*line ]
	VLD1.64 D21, [R0], R9    //src[11*line ]
	VLD1.64 D22, [R0], R9    //src[13*line ]
	VLD1.64 D23, [R0], R9    //src[15*line ]
	VLD1.64 D24, [R0], R9    //src[17*line ]
	VLD1.64 D25, [R0], R9    //src[19*line ]
	VLD1.64 D26, [R0], R9    //src[21*line ]
	VLD1.64 D27, [R0], R9    //src[23*line ]
	VLD1.64 D28, [R0], R9    //src[25*line ]
	VLD1.64 D29, [R0], R9    //src[27*line ]
	VLD1.64 D30, [R0], R9    //src[29*line ]
	VLD1.64 D31, [R0], R9    //src[31*line ]

	MOV R7,  #2
	MOVT R7, #7
	MOV R3, #11
	MOVT R3, #15
	MOV R4, #19
	MOVT R4, #23
	MOV R5, #27
	MOVT R5, #30
	VMOV D4, R7, R3   //D4 [0]=2 ,D4[1] =7,D4 [2]=11,D4[3] =15
	VMOV D5, R4, R5   //D5 [0]=19,D5[1] =23,D5 [0]=27,D5[1] =30

	MOV R7, #34
	MOVT R7, #36
	MOV R3, #39
	MOVT R3, #41
	MOV R4, #43
	MOVT R4, #44
	MOV R5, #45
	MOVT R5, #45
	VMOV D6, R7, R3   //D6[0]=34,D6[1]=36,D6[0]=39,D6[1]=41
	VMOV D7, R4, R5   //D7[0]=43,D7[1]=44,D7[0]=45,D7[1]=45
				
	//O[15]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D5[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q12
	VMOV Q1, Q13
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D6[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D7[2]
	VMLSL.S16 Q7, D3, D7[3]
	
	VPUSH {Q7}

	
	//O[14]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D4[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D7[3]
	VPUSH {Q7}
	
	//O[13]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D7[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D7[2]
	VMLSL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D7[1]
	VPUSH {Q7}
	
	//O[12]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D7[0]
	VPUSH {Q7}

	//O[11]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D6[0]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D5[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D5[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D4[2]
	VMLSL.S16 Q7, D3, D6[3]
	VPUSH {Q7}

	//O[10]
	VMOV Q0, Q8
	VMOV Q1, Q9
	VMULL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D5[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLAL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D6[2]
	VPUSH {Q7}

	//O[9]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D4[0]
	VMLAL.S16 Q7, D3, D7[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D6[3]
	VMLAL.S16 Q7, D3, D4[1]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D5[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D4[2]
	VMLAL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D6[1]
	VPUSH {Q7}

	//O[8]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D6[3]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D7[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13	
	VMLSL.S16 Q7, D0, D4[0]
	VMLSL.S16 Q7, D1, D7[3]
	VMLAL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[0]
	VMLSL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D5[2]
	VMLAL.S16 Q7, D3, D6[0]
	VPUSH {Q7}
	
	//O[7]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[0]
	VMLSL.S16 Q7, D1, D5[2]
	VMLSL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D7[0]
	VMLSL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D7[3]
	VMLAL.S16 Q7, D3, D4[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLSL.S16 Q7, D2, D7[1]
	VMLSL.S16 Q7, D3, D4[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D6[1]
	VMLSL.S16 Q7, D3, D5[3]
	VPUSH {Q7}
	
	//O[6]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D4[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[2]
	VMLAL.S16 Q7, D1, D6[0]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D7[3]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D5[3]
	VMLSL.S16 Q7, D3, D5[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D7[0]
	VMLAL.S16 Q7, D3, D5[2]
	VPUSH {Q7}


	//O[5]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D6[3]
	VMLSL.S16 Q7, D3, D6[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[1]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D6[0]
	VMLSL.S16 Q7, D3, D4[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D7[1]
	VMLSL.S16 Q7, D1, D5[3]
	VMLAL.S16 Q7, D2, D4[3]
	VMLAL.S16 Q7, D3, D7[3]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D5[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D5[1]
	VPUSH {Q7}

	
	//O[4]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLSL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D7[2]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D4[1]
	VMLAL.S16 Q7, D2, D6[2]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D4[3]
	VMLSL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[0]
	VMLAL.S16 Q7, D1, D6[1]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D5[0]
	VPUSH {Q7}

	
	//O[3]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[0]
	VMLAL.S16 Q7, D1, D5[1]
	VMLSL.S16 Q7, D2, D4[1]
	VMLSL.S16 Q7, D3, D6[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D7[3]
	VMLSL.S16 Q7, D1, D6[1]
	VMLSL.S16 Q7, D2, D4[2]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D7[1]
	VMLAL.S16 Q7, D2, D5[2]
	VMLSL.S16 Q7, D3, D4[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D5[3]
	VMLSL.S16 Q7, D1, D7[3]
	VMLSL.S16 Q7, D2, D6[2]
	VMLSL.S16 Q7, D3, D4[3]
	VPUSH {Q7}

	
	//O[2]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[1]
	VMLAL.S16 Q7, D1, D6[0]
	VMLAL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D4[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLSL.S16 Q7, D0, D5[2]
	VMLSL.S16 Q7, D1, D6[3]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D6[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D5[1]
	VMLSL.S16 Q7, D1, D4[0]
	VMLAL.S16 Q7, D2, D5[0]
	VMLAL.S16 Q7, D3, D6[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[0]
	VMLAL.S16 Q7, D2, D5[3]
	VMLAL.S16 Q7, D3, D4[2]
	VPUSH {Q7}

	//O[1]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D6[3]
	VMLAL.S16 Q7, D2, D6[0]
	VMLAL.S16 Q7, D3, D5[1]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D4[2]
	VMLSL.S16 Q7, D1, D4[0]
	VMLSL.S16 Q7, D2, D4[3]
	VMLSL.S16 Q7, D3, D5[2]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLSL.S16 Q7, D0, D6[1]
	VMLSL.S16 Q7, D1, D7[0]
	VMLSL.S16 Q7, D2, D7[3]
	VMLSL.S16 Q7, D3, D7[1]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLSL.S16 Q7, D0, D6[2]
	VMLSL.S16 Q7, D1, D5[3]
	VMLSL.S16 Q7, D2, D5[0]
	VMLSL.S16 Q7, D3, D4[1]
	VPUSH {Q7}

		
	//O[0]
	VMOV Q0, Q8
	VMOV Q1, Q9	
	VMULL.S16 Q7, D0, D7[3]
	VMLAL.S16 Q7, D1, D7[2]
	VMLAL.S16 Q7, D2, D7[1]
	VMLAL.S16 Q7, D3, D7[0]
	VMOV Q0, Q10
	VMOV Q1, Q11	
	VMLAL.S16 Q7, D0, D6[3]
	VMLAL.S16 Q7, D1, D6[2]
	VMLAL.S16 Q7, D2, D6[1]
	VMLAL.S16 Q7, D3, D6[0]
	VMOV Q0, Q12
	VMOV Q1, Q13		
	VMLAL.S16 Q7, D0, D5[3]
	VMLAL.S16 Q7, D1, D5[2]
	VMLAL.S16 Q7, D2, D5[1]
	VMLAL.S16 Q7, D3, D5[0]
	VMOV Q0, Q14
	VMOV Q1, Q15
	VMLAL.S16 Q7, D0, D4[3]
	VMLAL.S16 Q7, D1, D4[2]
	VMLAL.S16 Q7, D2, D4[1]
	VMLAL.S16 Q7, D3, D4[0]
	VPUSH {Q7}


	//CALCULATE AND STORE DST
	/////////////////////////////////////////
	VPOP {D16-D23}   //Q[8] - Q[11] :O[0]-O[3]
	ADD SP, SP, #320
	VPOP {D24-D31}   //Q[12]- Q[15] :E[0]-E[3]
    //DST[0]-[3]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	MOV R1, R11
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[28]-[31]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #56
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	

	/////////////////////////////////////////
	SUB SP, SP, #384
	VPOP {D16-D23}   //Q[8] - Q[11] :O[4]-O[7]
	ADD SP, SP, #128
	VPOP {D24-D31}   //Q[12]- Q[15] :E[4]-E[7]
	//DST[4]-[7]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #8
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[24]-[27]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #48
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9


	/////////////////////////////////////////
	VPOP {D24-D31}   //Q[12]- Q[15] :E[8]-E[11]
	SUB SP, SP, #256
	VPOP {D16-D23}   //Q[8] - Q[11] :O[8]-O[11]
		
	//DST[8]-[11]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #16
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	
	 //DST[20]-[23]
	VSUB.S32 Q3, Q12, Q8
	VSUB.S32 Q2, Q13, Q9
	VSUB.S32 Q1, Q14, Q10
	VSUB.S32 Q0, Q15, Q11
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D1, Q1, #11
	VQRSHRN.S32 D2, Q2, #11
	VQRSHRN.S32 D3, Q3, #11
	VTRN.32 D0, D2
	VTRN.32 D1, D3
	VTRN.16 D0, D1
	VTRN.16 D2, D3

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	
	ADD R1, R11, #40
	MOV R9, #64
	VST1.64 D0, [R1], R9
	VST1.64 D1, [R1], R9
	VST1.64 D2, [R1], R9
	VST1.64 D3, [R1], R9
	

	/////////////////////////////////////////

	VPOP {D16-D23}   //Q[8] - Q[11] :O[12]-O[15]
	ADD SP, SP, #192	
	VPOP {D24-D31}   //Q[12]- Q[15] :E[12]-E[15]
	
	
	//DST[12]-[15]
	VADD.S32 Q0, Q8, Q12
	VADD.S32 Q1, Q9, Q13
	VADD.S32 Q2, Q10, Q14
	VADD.S32 Q3, Q11, Q15
	VQRSHRN.S32 D0, Q0, #11
	VQRSHRN.S32 D2, Q1, #11
	VQRSHRN.S32 D4, Q2, #11
	VQRSHRN.S32 D6, Q3, #11
	VTRN.32 D0, D4
	VTRN.32 D2, D6
	VTRN.16 D0, D2
	VTRN.16 D4, D6
	
    //DST[16]-[19]
	VSUB.S32 Q7, Q12, Q8
	VSUB.S32 Q6, Q13, Q9
	VSUB.S32 Q5, Q14, Q10
	VSUB.S32 Q4, Q15, Q11
	VQRSHRN.S32 D1, Q4, #11
	VQRSHRN.S32 D3, Q5, #11
	VQRSHRN.S32 D5, Q6, #11
	VQRSHRN.S32 D7, Q7, #11
	VTRN.32 D1, D5
	VTRN.32 D3, D7
	VTRN.16 D1, D3
	VTRN.16 D5, D7

	//clip
	MVN R4, #0
	LSL R5, R4, R2 	//minvalue R5
	MVN R6, R5 		//maxvalue R6
	VDUP.16 Q5, R5	//minval vector Q5
	VDUP.16 Q6, R6  //maxval vector Q6
	VMAX.S16 Q0, Q0, Q5
	VMAX.S16 Q1, Q1, Q5
	VMAX.S16 Q2, Q2, Q5
	VMAX.S16 Q3, Q3, Q5
	VMIN.S16 Q0, Q0, Q6
	VMIN.S16 Q1, Q1, Q6
	VMIN.S16 Q2, Q2, Q6
	VMIN.S16 Q3, Q3, Q6
	
	ADD R1, R11, #24
	MOV R9, #64
	VST1.64 {D0, D1}, [R1], R9
	VST1.64 {D2, D3}, [R1], R9
	VST1.64 {D4, D5}, [R1], R9
	VST1.64 {D6, D7}, [R1], R9
	
	ADD SP, SP, #256
.endm

//***********************************************************************
//void partialButterflyInverse32x8_shift11_clipx_neon(Short *src,Short *dst,int clip)
//src: transform coefficients
//dst: the result   
//detail: shift=5 clip=16
//***********************************************************************
function partialButterflyInverse32x8_shift11_clipx_neon
	PUSH {R4-R12, LR}	
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 
	SUB R2, R2, #1

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32x8_4rows_shift11_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32x8_4rows_shift11_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

//***********************************************************************
//void idct_32x8_shift11_neon(coef_t *blk, int shift, int clip)
//blk: transform coefficients
//shift: not use, the shift should change in_the code, shift 11 in code  
//clip: clip
//***********************************************************************
function idct_32x8_shift11_neon
	PUSH {R4-R12, LR}	

	////////////////////alloc mem ///////////////////

	SUB R12, SP, #1440

	MOV R8, R0
	MOV R1, R12

	//////////////////////////////////////////////////
	///////////////////first trans////////////////////
	//////////////////////////////////////////////////

	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST

	///////////////////1ST  4 ROWS///////////////////
	
	partialButterflyInverse8x32_4rows_shift5_clip16
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R0, R10, #8
	partialButterflyInverse8x32_4rows_shift5_clip16

	///////////////////3ED  4 ROWS///////////////////

	ADD R0, R10, #16
	partialButterflyInverse8x32_4rows_shift5_clip16
	
	///////////////////4ED  4 ROWS///////////////////

	ADD R0, R10, #24
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////5ED  4 ROWS///////////////////

	ADD R0, R10, #32
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////6ED  4 ROWS///////////////////

	ADD R0, R10, #40
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////7ED  4 ROWS///////////////////

	ADD R0, R10, #48
	partialButterflyInverse8x32_4rows_shift5_clip16
	

	///////////////////8ED  4 ROWS///////////////////

	ADD R0, R10, #56
	partialButterflyInverse8x32_4rows_shift5_clip16
	
	//////////////////////////////////////////////////
	///////////////////second trans///////////////////
	//////////////////////////////////////////////////

	MOV R0, R12
	MOV R1, R8
	SUB R2, R2, #1
	MOV R10, R0 //SAVE SRC
	MOV R11, R1 //SAVE DST 

	///////////////////1ST  4 ROWS///////////////////
	
	partButterflyInverse32x8_4rows_shift11_clipx
	
	///////////////////2ND  4 ROWS///////////////////

	ADD R10, R10, #8
	ADD R11, R11, #256
	partButterflyInverse32x8_4rows_shift11_clipx
	
	///////////////////    END    ///////////////////

	POP {R4-R12, PC} 
.endfunc

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


.macro	mul_col_s16	res_d, col_d, l1, l2			//used in 2nd hor & ver trans
    vmull.s16   q12, d16, \col_d[0]
    vmlal.s16   q12, d17, \col_d[1]
    vmlal.s16	q12, d18, \col_d[2]
    vmlal.s16   q12, d19, \col_d[3]

\l1:
	cmp 	r2, #0
	beq		\l2
	mov 	r4, #1
	sub		r5, r2, #1
	lsl		r12, r4, r5
	vmov	d9, r12, r12		//vdup
	vadd.s32	d24, d24, d9
	vadd.s32	d25, d25, d9
	vmov 	r4, r5, d24
	vmov	r6, r7, d25
	asr		r4, r4, r2
	asr		r5, r5, r2
	asr		r6, r6, r2
	asr		r7, r7, r2
	vmov 	d24, r4, r5
	vmov 	d25, r6, r7
\l2:
	vqmovn.s32	\res_d,	q12
.endm

.macro transpose_4x4_16b	line0, line1 line2, line3
    //transpose matrix, store by columns
    vtrn.32		\line0, \line2
	vtrn.32 	\line1, \line3
	vtrn.16 	\line0, \line1
	vtrn.16 	\line2, \line3
.endm

.macro	mul_col_s16_clip	res_d, col_d		//used in 2nd trans
    vmull.s16   q12, d4, \col_d[0]
    vmlal.s16   q12, d5, \col_d[1]
    vmlal.s16	q12, d6, \col_d[2]
    vmlal.s16   q12, d7, \col_d[3]

	//add rnd_factor & shift
	vadd.s32	d24, d24, d12
	vadd.s32	d25, d25, d12
	vmov 	r5, r6, d24
	vmov	r7, r8, d25
	asr		r5, r5, r2
	asr		r6, r6, r2
	asr		r7, r7, r2
	asr		r8, r8, r2
	//clip
	cmp		r5, r9
	movgt	r5, r9
	cmp		r5, r10
	movlt	r5, r10
	cmp		r6, r9
	movgt	r6, r9
	cmp		r6, r10
	movlt	r6, r10
	cmp		r7, r9
	movgt	r7, r9
	cmp		r7, r10
	movlt	r7, r10
	cmp		r8, r9
	movgt	r8, r9
	cmp		r8, r10
	movlt	r8, r10

	vmov 	d24, r5, r6
	vmov 	d25, r7, r8
	vqmovn.s32	\res_d,	q12
.endm
//*************************************************************************************
//void inv_2nd_trans_hor_neon(coef_t *blk, int i_blk, int shift, const i16s_t coef[4][4])
//r0	blk: 	block data
//r1	i_blk: 	blk stride
//r2	shift: 	shift number
//r3	coef: 	transform coefficients
//*************************************************************************************
function inv_2nd_trans_hor_neon
	PUSH {R4-R12, LR}

	//load & transpose coef_matrix
	vld4.16		{d0-d3}, [r3]				//cross load
	//vld1.16	{d0-d3}, [r3]
    //transpose_4x4_16b	d0, d1, d2, d3

    //load & transpose blk_matrix
    mov			r4, r0
    add			r1, r1, r1					//r1' = r1 * 2, 16 bit  = 2 Byte
	vld1.16		{d16}, [r4], r1
	vld1.16		{d17}, [r4], r1
	vld1.16		{d18}, [r4], r1
	vld1.16		{d19}, [r4], r1
	transpose_4x4_16b	d16, d17, d18, d19

	//matrix multiply, blk_matrix X coef_matrix
    mul_col_s16	d4, d0, A0, B0				// (blk_matrix) X (coef_matrix column 0)
    mul_col_s16 d5, d1, A1, B1				// (blk_matrix) X (coef_matrix column 1)
    mul_col_s16	d6, d2, A2, B2      		// (blk_matrix) X (coef_matrix column 2)
    mul_col_s16	d7, d3, A3, B3      		// (blk_matrix) X (coef_matrix column 3)

	//cross store blk data
	transpose_4x4_16b	d4, d5, d6, d7
    vst1.16		{d4}, [r0], r1
    vst1.16		{d5}, [r0], r1
    vst1.16		{d6}, [r0], r1
    vst1.16		{d7}, [r0], r1

	POP {R4-R12, PC}
.endfunc

//*************************************************************************************
//void inv_2nd_trans_ver_neon(coef_t *blk, int i_blk, int shift, const i16s_t coef[4][4])
//r0	blk: 	block data
//r1	i_blk: 	blk stride
//r2	shift: 	shift number
//r3	coef: 	transform coefficients
//*************************************************************************************
function inv_2nd_trans_ver_neon
	PUSH {R4-R12, LR}

	//load & transpose coef_matrix
	vld4.16		{d0-d3}, [r3]

    //load & transpose blk_matrix
    mov			r4, r0
    add			r1, r1, r1					//r1' = r1 * 2, 16 bit  = 2 Byte
	vld1.16		{d16}, [r4], r1
	vld1.16		{d17}, [r4], r1
	vld1.16		{d18}, [r4], r1
	vld1.16		{d19}, [r4], r1

	//matrix multiply, blk_matrix X coef_matrix
    mul_col_s16	d4, d0, AA0, BB0			// (blk_matrix) X (coef_matrix column 0)
    mul_col_s16 d5, d1, AA1, BB1			// (blk_matrix) X (coef_matrix column 1)
    mul_col_s16	d6, d2, AA2, BB2      		// (blk_matrix) X (coef_matrix column 2)
    mul_col_s16	d7, d3, AA3, BB3      		// (blk_matrix) X (coef_matrix column 3)

	//store blk data
    vst1.16		{d4}, [r0], r1
    vst1.16		{d5}, [r0], r1
    vst1.16		{d6}, [r0], r1
    vst1.16		{d7}, [r0], r1

	POP {R4-R12, PC}
.endfunc

//*************************************************************************************
//void inv_2nd_trans_neon(coef_t *blk, int i_blk, int shift, int clip_depth, const i16s_t coef[4][4])
//r0	blk:	block data
//r1	i_blk: 	blk stride
//r2	shift: 	shift number
//r3	clip_depth:	clip depth
//r4	coef: 	transform coefficients
//*************************************************************************************
function inv_2nd_trans_neon
	PUSH {R4-R12, LR}
	add	sp, sp, #40
	ldmia sp, {r4}
	sub	sp, sp, #40

	/*---ver trans---*/
	//load & transpose coef_matrix
	vld4.16		{d0-d3}, [r4]

    //load & transpose blk_matrix
    mov			r5, r0
    add			r1, r1, r1					//r1' = r1 * 2, 16 bit  = 2 Byte
	vld1.16		{d16}, [r5], r1
	vld1.16		{d17}, [r5], r1
	vld1.16		{d18}, [r5], r1
	vld1.16		{d19}, [r5], r1

	//matrix multiply, blk_matrix X coef_matrix
	vmull.s16   q12, d16, d0[0]
    vmlal.s16   q12, d17, d0[1]
    vmlal.s16	q12, d18, d0[2]
    vmlal.s16   q12, d19, d0[3]
    vqrshrn.s32	d4, q12, #5

    vmull.s16   q12, d16, d1[0]
    vmlal.s16   q12, d17, d1[1]
    vmlal.s16	q12, d18, d1[2]
    vmlal.s16   q12, d19, d1[3]
    vqrshrn.s32	d5, q12, #5

	vmull.s16   q12, d16, d2[0]
    vmlal.s16   q12, d17, d2[1]
    vmlal.s16	q12, d18, d2[2]
    vmlal.s16   q12, d19, d2[3]
    vqrshrn.s32	d6, q12, #5

    vmull.s16   q12, d16, d3[0]
    vmlal.s16   q12, d17, d3[1]
    vmlal.s16	q12, d18, d3[2]
    vmlal.s16   q12, d19, d3[3]
    vqrshrn.s32	d7, q12, #5

	//temp blk data {d4-d7}

    /*---hor trans---*/
	add	r2, r2, #2				//r2: shift value
	mov 	r6, #1
	sub		r5, r2, #1
	lsl		r12, r6, r5
	vmov	d12, r12, r12		//r12: rnd_factor value
	sub		r9, r3, #1
	lsl		r9, r6, r9
	sub		r9, r9, #1			//r9: max_val
	mov		r11, #0
	sub		r10, r11, r9
	sub		r10, r10, #1		//r10: min_val
//	vmov.16	d12[0], r9
//	vmov.16	d12[1], r10

	//matrix multiply, blk_matrix X coef_matrix
	transpose_4x4_16b	d4, d5, d6, d7
    mul_col_s16_clip	d8, d0				// (blk_matrix) X (coef_matrix column 0)
    mul_col_s16_clip 	d9, d1				// (blk_matrix) X (coef_matrix column 1)
    mul_col_s16_clip	d10, d2      		// (blk_matrix) X (coef_matrix column 2)
    mul_col_s16_clip	d11, d3      		// (blk_matrix) X (coef_matrix column 3)

	//store blk data
	transpose_4x4_16b	d8, d9, d10, d11
    vst1.16		{d8}, [r0], r1
    vst1.16		{d9}, [r0], r1
    vst1.16		{d10}, [r0], r1
    vst1.16		{d11}, [r0], r1

	POP {R4-R12, PC}
.endfunc

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

.macro rshift	M0, M1, M2, M3
	vshr.s16	\M0, \M0, #1
	vshr.s16	\M1, \M1, #1
	vshr.s16	\M2, \M2, #1
	vshr.s16	\M3, \M3, #1
.endm
.macro filter_a_new_row_32	M0,M1,M2,M3,M4,M5,M6,M7,M8,M9,M10,M11
	//row 1 M0 M1 M2 M3
	//row 2 M4 M5 M6 M7
	//new row: M8 M9 M10 M11
	vadd.s16	\M8, \M0, \M4
	vadd.s16	\M9, \M1, \M5
	vadd.s16	\M10, \M2, \M6
	vadd.s16	\M11, \M3, \M7
	vshr.s16	\M8, \M8, #1
	vshr.s16	\M9, \M9, #1
	vshr.s16	\M10, \M10, #1
	vshr.s16	\M11, \M11, #1
.endm
.macro filter_a_new_col_4	M0,M1,M2
	//col 1 M0
	//col 2 M1
	//new col: M2
	vadd.s16	\M2, \M0, \M1
	vshr.s16	\M2, \M2, #1
.endm
.macro store_col_4x4	M0,M1,M2,M3,R1
	mov	r3, #128
	transpose_4x4_16b	\M0,\M1,\M2,\M3
	vst1.16	{\M0}, [\R1], r3
	vst1.16	{\M1}, [\R1], r3
	vst1.16	{\M2}, [\R1], r3
	vst1.16	{\M3}, [\R1], r3
.endm
.macro store_col_4x4_ver	M0,M1,M2,M3,R1
	mov	r3, #32
	transpose_4x4_16b	\M0,\M1,\M2,\M3
	vst1.16	{\M0}, [\R1], r3
	vst1.16	{\M1}, [\R1], r3
	vst1.16	{\M2}, [\R1], r3
	vst1.16	{\M3}, [\R1], r3
.endm
//*************************************************************************************
//void inv_wavelet_NSQT_Hor_neon(coef_t *blk)
//r0	blk: block data (input 32x8, output 64x16)
//*************************************************************************************
function inv_wavelet_NSQT_Hor_neon
	PUSH {R4-R9, LR}

	/* ver transform, 32x8 -> 32x16 */
	mov	r1, r0

	//add first row
	vld1.16	{q0-q1}, [r1]!		//load row 0: q0 q1 q2 q3
	vld1.16	{q2-q3}, [r1]!
	rshift	q0,q1,q2,q3
	vld1.16	{q4-q5}, [r1]!		//load row 1: q4 q5 q6 q7
	vld1.16	{q6-q7}, [r1]!
	rshift	q4,q5,q6,q7
	filter_a_new_row_32	q0,q1,q2,q3,q4,q5,q6,q7,q8,q9,q10,q11 	//add a new row between row0&1:	q8 q9 q10 q11
	vpush	{q0-q3}
	vpush	{q8-q11}

	//loop: add 6 rows
	mov r2, #6
ver_filter_1:
	vld1.16	{q0-q1}, [r1]!		//load
	vld1.16	{q2-q3}, [r1]!
	rshift	q0,q1,q2,q3
	filter_a_new_row_32	q0,q1,q2,q3,q4,q5,q6,q7,q8,q9,q10,q11
	vpush	{q4-q7}
	vpush	{q8-q11}
	vmov	q4, q0
	vmov	q5, q1
	vmov	q6, q2
	vmov 	q7, q3
	sub	r2, r2, #1
	cmp	r2, #0
	bne	ver_filter_1

	//add last row
	vpush	{q4-q7}				//last two rows are the same
	vpush	{q4-q7}

	/* hor transform, 32x16 -> 64x16 */
	mov r4, #4
	//loop 32x4 -> 64x4
	add	sp, sp, #960			//1024 to sp0, 1024-64
hor_filter_1:
	cmp r4, #4
	subne sp, sp, #128
	addne r0, r0, #512
	@filter 1st 8x4->16x4
	vpop	{q0}
	sub	sp, sp, #80				// 64+16
	vpop	{q1}
	sub	sp, sp, #80
	vpop	{q2}
	sub	sp, sp, #80
	vpop	{q3}
	transpose_4x4_16b	d0,d2,d4,d6
	transpose_4x4_16b	d1,d3,d5,d7
	vmov	d8, d1
	vmov	d10, d3
	vmov	d12, d5
	vmov	d14, d7

	filter_a_new_col_4	d0,d2,d1
	filter_a_new_col_4	d2,d4,d3
	filter_a_new_col_4	d4,d6,d5
	filter_a_new_col_4	d6,d8,d7
	filter_a_new_col_4	d8,d10,d9
	filter_a_new_col_4	d10,d12,d11
	filter_a_new_col_4	d12,d14,d13

	add	sp, sp, #192			// 64x3
	vpop	{q12}
	sub	sp, sp, #80				// 64+16
	vpop	{q13}
	sub	sp, sp, #80
	vpop	{q14}
	sub	sp, sp, #80
	vpop	{q15}
	transpose_4x4_16b	d24,d26,d28,d30
	transpose_4x4_16b	d25,d27,d29,d31
	filter_a_new_col_4	d14,d24,d15

	//store d0-d15
	mov r1, r0
	store_col_4x4	d0,d1,d2,d3,r1
	add r1, r0, #8
	store_col_4x4	d4,d5,d6,d7,r1
	add r1, r0, #16
	store_col_4x4	d8,d9,d10,d11,r1
	add r1, r0, #24
	store_col_4x4	d12,d13,d14,d15,r1
	//add	sp, sp, #224

	@filter 2nd 8x4->16x4
	vswp	q0, q12
	vswp	q1, q13
	vswp	q2, q14
	vswp	q3, q15
	vmov	d8, d1
	vmov	d10, d3
	vmov	d12, d5
	vmov	d14, d7
	filter_a_new_col_4	d0,d2,d1
	filter_a_new_col_4	d2,d4,d3
	filter_a_new_col_4	d4,d6,d5
	filter_a_new_col_4	d6,d8,d7
	filter_a_new_col_4	d8,d10,d9
	filter_a_new_col_4	d10,d12,d11
	filter_a_new_col_4	d12,d14,d13

	add	sp, sp, #192			// 224 to sp0
	vpop	{q12}
	sub	sp, sp, #80				// 64+16
	vpop	{q13}
	sub	sp, sp, #80
	vpop	{q14}
	sub	sp, sp, #80
	vpop	{q15}
	transpose_4x4_16b	d24,d26,d28,d30
	transpose_4x4_16b	d25,d27,d29,d31
	filter_a_new_col_4	d14,d24,d15

	add r1, r0, #32
	store_col_4x4	d0,d1,d2,d3,r1
	add r1, r0, #40
	store_col_4x4	d4,d5,d6,d7,r1
	add r1, r0, #48
	store_col_4x4	d8,d9,d10,d11,r1
	add r1, r0, #56
	store_col_4x4	d12,d13,d14,d15,r1
//	add sp, sp, #208

	@filter 3rd 8x4->16x4
	vswp	q0, q12
	vswp	q1, q13
	vswp	q2, q14
	vswp	q3, q15
	vmov	d8, d1
	vmov	d10, d3
	vmov	d12, d5
	vmov	d14, d7
	filter_a_new_col_4	d0,d2,d1
	filter_a_new_col_4	d2,d4,d3
	filter_a_new_col_4	d4,d6,d5
	filter_a_new_col_4	d6,d8,d7
	filter_a_new_col_4	d8,d10,d9
	filter_a_new_col_4	d10,d12,d11
	filter_a_new_col_4	d12,d14,d13

	add	sp, sp, #192			// 224 to sp0
	vpop	{q12}
	sub	sp, sp, #80				// 64+16
	vpop	{q13}
	sub	sp, sp, #80
	vpop	{q14}
	sub	sp, sp, #80
	vpop	{q15}
	transpose_4x4_16b	d24,d26,d28,d30
	transpose_4x4_16b	d25,d27,d29,d31
	filter_a_new_col_4	d14,d24,d15

	add r1, r0, #64
	store_col_4x4	d0,d1,d2,d3,r1
	add r1, r0, #72
	store_col_4x4	d4,d5,d6,d7,r1
	add r1, r0, #80
	store_col_4x4	d8,d9,d10,d11,r1
	add r1, r0, #88
	store_col_4x4	d12,d13,d14,d15,r1
//	add sp, sp, #192

	@filter 4th 8x4->16x4
	vswp	q0, q12
	vswp	q1, q13
	vswp	q2, q14
	vswp	q3, q15
	vmov	d8, d1
	vmov	d10, d3
	vmov	d12, d5
	vmov	d14, d7
	filter_a_new_col_4	d0,d2,d1
	filter_a_new_col_4	d2,d4,d3
	filter_a_new_col_4	d4,d6,d5
	filter_a_new_col_4	d6,d8,d7
	filter_a_new_col_4	d8,d10,d9
	filter_a_new_col_4	d10,d12,d11
	filter_a_new_col_4	d12,d14,d13
	filter_a_new_col_4	d14,d14,d15

	add r1, r0, #96
	store_col_4x4	d0,d1,d2,d3,r1
	add r1, r0, #104
	store_col_4x4	d4,d5,d6,d7,r1
	add r1, r0, #112
	store_col_4x4	d8,d9,d10,d11,r1
	add r1, r0, #120
	store_col_4x4	d12,d13,d14,d15,r1
//	add sp, sp, #192

	sub r4, r4, #1
	cmp	r4, #0
	bne	hor_filter_1

	add sp, sp, #960

	POP {R4-R9, PC}
.endfunc

//*************************************************************************************
//void inv_wavelet_B64_neon(coef_t *blk)
//r0	blk: block data (input 32x32, output 64x64)
//*************************************************************************************
function inv_wavelet_B64_neon
	PUSH {R4-R9, LR}

	/* ver transform, 32x32 -> 32x64 */
	mov	r1, r0

	//add first row
	vld1.16	{q0-q1}, [r1]!		//load row 0: q0 q1 q2 q3
	vld1.16	{q2-q3}, [r1]!
	rshift	q0,q1,q2,q3
	vld1.16	{q4-q5}, [r1]!		//load row 1: q4 q5 q6 q7
	vld1.16	{q6-q7}, [r1]!
	rshift	q4,q5,q6,q7
	filter_a_new_row_32	q0,q1,q2,q3,q4,q5,q6,q7,q8,q9,q10,q11 	//add a new row between row0&1:	q8 q9 q10 q11
	vpush	{q0-q3}
	vpush	{q8-q11}

	//loop: add 30 rows
	mov r2, #30
ver_filter_B64:
	vld1.16	{q0-q1}, [r1]!		//load
	vld1.16	{q2-q3}, [r1]!
	rshift	q0,q1,q2,q3
	filter_a_new_row_32	q0,q1,q2,q3,q4,q5,q6,q7,q8,q9,q10,q11
	vpush	{q4-q7}
	vpush	{q8-q11}
	vmov	q4, q0
	vmov	q5, q1
	vmov	q6, q2
	vmov 	q7, q3
	sub	r2, r2, #1
	cmp	r2, #0
	bne	ver_filter_B64

	//add last row
	vpush	{q4-q7}				//last two rows are the same
	vpush	{q4-q7}

	/* hor transform, 32x64 -> 64x64 */
	mov r4, #16					// 64/4
	//loop 32x4 -> 64x4
	add	sp, sp, #4032			// 4096-64
hor_filter_B64:
	cmp r4, #16
	subne sp, sp, #128
	addne r0, r0, #512
	@filter 1st 8x4->16x4
	vpop	{q0}
	sub	sp, sp, #80				// 64+16
	vpop	{q1}
	sub	sp, sp, #80
	vpop	{q2}
	sub	sp, sp, #80
	vpop	{q3}
	transpose_4x4_16b	d0,d2,d4,d6
	transpose_4x4_16b	d1,d3,d5,d7
	vmov	d8, d1
	vmov	d10, d3
	vmov	d12, d5
	vmov	d14, d7

	filter_a_new_col_4	d0,d2,d1
	filter_a_new_col_4	d2,d4,d3
	filter_a_new_col_4	d4,d6,d5
	filter_a_new_col_4	d6,d8,d7
	filter_a_new_col_4	d8,d10,d9
	filter_a_new_col_4	d10,d12,d11
	filter_a_new_col_4	d12,d14,d13

	add	sp, sp, #192			// 64x3
	vpop	{q12}
	sub	sp, sp, #80				// 64+16
	vpop	{q13}
	sub	sp, sp, #80
	vpop	{q14}
	sub	sp, sp, #80
	vpop	{q15}
	transpose_4x4_16b	d24,d26,d28,d30
	transpose_4x4_16b	d25,d27,d29,d31
	filter_a_new_col_4	d14,d24,d15

	//store d0-d15
	mov r1, r0
	store_col_4x4	d0,d1,d2,d3,r1
	add r1, r0, #8
	store_col_4x4	d4,d5,d6,d7,r1
	add r1, r0, #16
	store_col_4x4	d8,d9,d10,d11,r1
	add r1, r0, #24
	store_col_4x4	d12,d13,d14,d15,r1
	//add	sp, sp, #224

	@filter 2nd 8x4->16x4
	vswp	q0, q12
	vswp	q1, q13
	vswp	q2, q14
	vswp	q3, q15
	vmov	d8, d1
	vmov	d10, d3
	vmov	d12, d5
	vmov	d14, d7
	filter_a_new_col_4	d0,d2,d1
	filter_a_new_col_4	d2,d4,d3
	filter_a_new_col_4	d4,d6,d5
	filter_a_new_col_4	d6,d8,d7
	filter_a_new_col_4	d8,d10,d9
	filter_a_new_col_4	d10,d12,d11
	filter_a_new_col_4	d12,d14,d13

	add	sp, sp, #192			// 224 to sp0
	vpop	{q12}
	sub	sp, sp, #80				// 64+16
	vpop	{q13}
	sub	sp, sp, #80
	vpop	{q14}
	sub	sp, sp, #80
	vpop	{q15}
	transpose_4x4_16b	d24,d26,d28,d30
	transpose_4x4_16b	d25,d27,d29,d31
	filter_a_new_col_4	d14,d24,d15

	add r1, r0, #32
	store_col_4x4	d0,d1,d2,d3,r1
	add r1, r0, #40
	store_col_4x4	d4,d5,d6,d7,r1
	add r1, r0, #48
	store_col_4x4	d8,d9,d10,d11,r1
	add r1, r0, #56
	store_col_4x4	d12,d13,d14,d15,r1
//	add sp, sp, #208

	@filter 3rd 8x4->16x4
	vswp	q0, q12
	vswp	q1, q13
	vswp	q2, q14
	vswp	q3, q15
	vmov	d8, d1
	vmov	d10, d3
	vmov	d12, d5
	vmov	d14, d7
	filter_a_new_col_4	d0,d2,d1
	filter_a_new_col_4	d2,d4,d3
	filter_a_new_col_4	d4,d6,d5
	filter_a_new_col_4	d6,d8,d7
	filter_a_new_col_4	d8,d10,d9
	filter_a_new_col_4	d10,d12,d11
	filter_a_new_col_4	d12,d14,d13

	add	sp, sp, #192			// 224 to sp0
	vpop	{q12}
	sub	sp, sp, #80				// 64+16
	vpop	{q13}
	sub	sp, sp, #80
	vpop	{q14}
	sub	sp, sp, #80
	vpop	{q15}
	transpose_4x4_16b	d24,d26,d28,d30
	transpose_4x4_16b	d25,d27,d29,d31
	filter_a_new_col_4	d14,d24,d15

	add r1, r0, #64
	store_col_4x4	d0,d1,d2,d3,r1
	add r1, r0, #72
	store_col_4x4	d4,d5,d6,d7,r1
	add r1, r0, #80
	store_col_4x4	d8,d9,d10,d11,r1
	add r1, r0, #88
	store_col_4x4	d12,d13,d14,d15,r1
//	add sp, sp, #192

	@filter 4th 8x4->16x4
	vswp	q0, q12
	vswp	q1, q13
	vswp	q2, q14
	vswp	q3, q15
	vmov	d8, d1
	vmov	d10, d3
	vmov	d12, d5
	vmov	d14, d7
	filter_a_new_col_4	d0,d2,d1
	filter_a_new_col_4	d2,d4,d3
	filter_a_new_col_4	d4,d6,d5
	filter_a_new_col_4	d6,d8,d7
	filter_a_new_col_4	d8,d10,d9
	filter_a_new_col_4	d10,d12,d11
	filter_a_new_col_4	d12,d14,d13
	filter_a_new_col_4	d14,d14,d15

	add r1, r0, #96
	store_col_4x4	d0,d1,d2,d3,r1
	add r1, r0, #104
	store_col_4x4	d4,d5,d6,d7,r1
	add r1, r0, #112
	store_col_4x4	d8,d9,d10,d11,r1
	add r1, r0, #120
	store_col_4x4	d12,d13,d14,d15,r1
//	add sp, sp, #192

	sub r4, r4, #1
	cmp	r4, #0
	bne	hor_filter_B64

	add sp, sp, #4032

	POP {R4-R9, PC}
.endfunc

//*************************************************************************************
//void inv_wavelet_NSQT_Ver_neon(coef_t *blk)
//r0	blk: block data (input 8x32, output 16x64)
//*************************************************************************************
function inv_wavelet_NSQT_Ver_neon
	PUSH {R4-R9, LR}

	/* ver transform, 8x32 -> 8x64 */
	mov	r1, r0

	//add first row
	vld1.16	{q0}, [r1]!		//load row 0: q0
	vshr.s16	q0, q0, #1
	vld1.16	{q1}, [r1]!		//load row 1: q1
	vshr.s16	q1, q1, #1
	vadd.s16	q2, q0, q1
	vshr.s16	q2, q2, #1
	vpush	{q0}
	vpush	{q2}

	//loop: add 30 rows
	mov r2, #30
ver_filter_2:
	vld1.16	{q0}, [r1]!		//load a new row
	vshr.s16	q0, q0, #1
	vadd.s16	q2, q0, q1
	vshr.s16	q2, q2, #1
	vpush	{q1}
	vpush	{q2}
	vmov	q1, q0
	sub	r2, r2, #1
	cmp	r2, #0
	bne	ver_filter_2

	//add last row
	vpush	{q1}
	vpush	{q1}			//last two rows are the same

	/* hor transform, 8x64 -> 16x64 */
	mov r4, #16
	//loop 8x4 -> 16x4
	add	sp, sp, #960		//1024-16x4
hor_filter_2:
	cmp r4, #16
	subne sp, sp, #128
	addne r0, r0, #128		// 32x4
	@filter 1st 8x8->8x16
	vpop	{q3}
	vpop	{q2}
	vpop	{q1}
	vpop	{q0}
	transpose_4x4_16b	d0,d2,d4,d6
	transpose_4x4_16b	d1,d3,d5,d7
	vmov	d8, d1
	vmov	d10, d3
	vmov	d12, d5
	vmov	d14, d7

	filter_a_new_col_4	d0,d2,d1
	filter_a_new_col_4	d2,d4,d3
	filter_a_new_col_4	d4,d6,d5
	filter_a_new_col_4	d6,d8,d7
	filter_a_new_col_4	d8,d10,d9
	filter_a_new_col_4	d10,d12,d11
	filter_a_new_col_4	d12,d14,d13
	filter_a_new_col_4	d14,d14,d15

	//store d0-d15
	mov r1, r0
	store_col_4x4_ver	d0,d1,d2,d3,r1
	add r1, r0, #8
	store_col_4x4_ver	d4,d5,d6,d7,r1
	add r1, r0, #16
	store_col_4x4_ver	d8,d9,d10,d11,r1
	add r1, r0, #24
	store_col_4x4_ver	d12,d13,d14,d15,r1

	sub r4, r4, #1
	cmp	r4, #0
	bne	hor_filter_2

	add sp, sp, #960

	POP {R4-R9, PC}
.endfunc

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////