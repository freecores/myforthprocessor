/*
 * @(#)evalth13.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)evalth13.c	1.13 99/03/04
*
*	tetrahedral interpolation evaluation functions
*
*	Author:			George Pawle
*
*	Creation Date:	20 Dec 96
*
*    COPYRIGHT (c) 1996-1999 Eastman Kodak Company
*    As an unpublished work pursuant to Title 17 of the United
*    States Code.  All rights reserved.
*/

#include "kcpcache.h"

#define EVAL_HIGH_MASK (0xfffffc00)
#define EVAL_EXTENDED_BITS (8)

#define TH1_3D_OCHAN_VARS(bits, chan) \
KpUInt8_p			gridBase##chan; \
KpInt32_t			outStride##chan; \
KpUInt##bits##_p	outLut##chan, outp##chan; \
KpUInt##bits##_t	prevRes##chan;

#define TH1_3D_INIT_VARS \
KpUInt8_p	gridBaseP, thisOutLut; \
evalILut_p	inLut0, inLut1, inLut2; \
KpUInt32_t	data0, data1, data2, dataMask; \
KpInt32_t	i1, oChan, tResult, tvert1, tvert2, tvert3, mullacc, temp, iLutEntries; \
KpInt32_t	inStride0 = inStride[0], inStride1 = inStride[1], inStride2 = inStride[2]; \
KpInt32_t	baseOffset, Xf, Yf, Zf, xf, yf, zf; \
KpUInt32_t	a001, a010, a011, a100, a101, a110;

#define TH1_3D_INIT_INVARS_D8 \
KpUInt8_p	inp0 = inp[0].p8, inp1 = inp[1].p8, inp2 = inp[2].p8; \
KpUInt32_t	thisColor, prevColor = 0xffffffff;

#define TH1_3D_INIT_INVARS_D16 \
KpUInt16_p	inp0 = inp[0].p16, inp1 = inp[1].p16, inp2 = inp[2].p16; \
KpUInt32_t	ColorPart1, prevPart1 = 0, prevPart2 = 0xffffffff;

#define TH1_3D_INIT_GRIDVARS_D8 \
KpInt32_t	tvertData;

#define TH1_3D_INIT_GRIDVARS_D16 \
KpInt32_t	delta2, delta1, delta0, deltaHigh, mullaccH, highBits; \
KpInt32_t	tvert1Data, tvert2Data, tvert3Data;

#define TH1_3D_INIT_OUTVARS(bits) \
KpInt32_t	outLutSize = EVAL_OLUT_ENTRIESD##bits * sizeof (KpUInt##bits##_t);

#define TH1_3D_INIT_IN_D8 \
	if (dataTypeI) {} \
	iLutEntries = 1 << 8; \
	inLut0 = PTTableP->etLuts[ET_I8].P;

#define TH1_3D_INIT_IN_D16 \
	if (dataTypeI == KCM_USHORT_12) { \
		iLutEntries = 1 << 12; \
		inLut0 = PTTableP->etLuts[ET_I12].P; \
	} \
	else { \
		iLutEntries = 1 << 16; \
		inLut0 = PTTableP->etLuts[ET_I16].P; \
	}

#define TH1_3D_INIT_OUT_D8 \
	if (dataTypeO) {} \
	gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G12].P; \
	thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O8].P; \

#define TH1_3D_INIT_OUT_D16 \
	gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G16].P; \
	if (dataTypeO == KCM_USHORT_12) { \
		thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O12].P; \
	} \
	else { \
		thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O16].P; \
	}

#define TH1_3D_INIT_DATA(obits) \
	a001 = PTTableP->etGOffsets[1];		/* copy grid offsets into locals */ \
	a010 = PTTableP->etGOffsets[2]; \
	a011 = PTTableP->etGOffsets[3]; \
	a100 = PTTableP->etGOffsets[4]; \
	a101 = PTTableP->etGOffsets[5]; \
	a110 = PTTableP->etGOffsets[6]; \
	tvert3 = PTTableP->etGOffsets[7]; \
 \
	inLut1 = inLut0 + iLutEntries; \
	inLut2 = inLut1 + iLutEntries; \
	dataMask = iLutEntries -1;	/* set up data mask to prevent input table memory access violations */ \
	oChan = -1; \
	gridBaseP -= sizeof (mf2_tbldat_t); \
	thisOutLut -= outLutSize;

#define TH1_3D_INIT(bits) \
	TH1_3D_INIT_VARS \
	TH1_3D_INIT_INVARS_D##bits \
	TH1_3D_INIT_GRIDVARS_D##bits \
	TH1_3D_INIT_OUTVARS(bits) \
	TH1_3D_INIT_IN_D##bits \
	TH1_3D_INIT_OUT_D##bits \
	TH1_3D_INIT_DATA(bits)

#define TH1_3D_OCHAN_TABLES(obits, chan) \
	do {	/* set up output tables */ \
		oChan++; \
		gridBaseP += sizeof (mf2_tbldat_t); \
		thisOutLut += outLutSize; \
		if (outp[oChan].p##obits != NULL) {	/* this output channel is being evaluated */ \
			break; \
		} \
	} while (1); \
	outp##chan = outp[oChan].p##obits; \
	outStride##chan = outStride[oChan]; \
	gridBase##chan = gridBaseP; \
	outLut##chan = (KpUInt##obits##_p)thisOutLut;

#define TH1_3D_GET_DATA_D8 \
	data0 = *inp0; 								/* get channel 0 input data */ \
	inp0 += inStride0; \
	data1 = *inp1; 								/* get channel 1 input data */ \
	inp1 += inStride1; \
	data2 = *inp2; 								/* get channel 2 input data */ \
	inp2 += inStride2; \
 \
	thisColor = (data0 << 16) | (data1 << 8) | (data2);	/* calc this color   */ 


#define TH1_3D_GET_DATA_D16 \
	data0 = *inp0; 								/* get channel 0 input data */ \
	data0 &= dataMask; \
	inp0 = (KpUInt16_p)((KpUInt8_p)inp0 + inStride0); \
	data1 = *inp1; 								/* get channel 1 input data */ \
	data1 &= dataMask; \
	inp1 = (KpUInt16_p)((KpUInt8_p)inp1 + inStride1); \
	data2 = *inp2; 								/* get channel 2 input data */ \
	data2 &= dataMask; \
	inp2 = (KpUInt16_p)((KpUInt8_p)inp2 + inStride2); \
\
	ColorPart1 = (data0 << 16) | (data1);	/* calc this color */

	
#define TH1_3D_FIND_TETRA_D8    \
	prevColor = thisColor;    \
\
	baseOffset = inLut0[(0*FUT_INPTBL_ENT) + data0].index; 	/* pass input data through input tables */ \
	Xf = inLut0[(0*FUT_INPTBL_ENT) + data0].frac; \
	baseOffset += inLut0[(1*FUT_INPTBL_ENT) + data1].index; \
	Yf = inLut0[(1*FUT_INPTBL_ENT) + data1].frac; \
	baseOffset += inLut0[(2*FUT_INPTBL_ENT) + data2].index; \
	Zf = inLut0[(2*FUT_INPTBL_ENT) + data2].frac; \
\
	TH1_3D_CALC_TETRA


#define TH1_3D_FIND_TETRA_D16 \
	prevPart1 = ColorPart1;  \
	prevPart2 = data2;  \
\
	baseOffset = inLut0[data0].index;		/* pass input data through input tables */ \
	Xf = inLut0[data0].frac; \
	baseOffset += inLut1[data1].index; \
	Yf = inLut1[data1].frac; \
	baseOffset += inLut2[data2].index; \
	Zf = inLut2[data2].frac; \
\
	TH1_3D_CALC_TETRA 	


#define	TH1_3D_CALC_TETRA   \
	/* find the tetrahedron in which the point is located */ \
	if (Xf > Yf) { \
		if (Yf > Zf) {	/* AHEG */ \
			xf = Xf;		/* unchanged order */ \
			yf = Yf; \
			zf = Zf; \
\
			tvert1 = a110; \
			tvert2 = a100; \
		} \
		else { \
			zf = Yf;		/* y into z */ \
			tvert1 = a101; \
			if (Xf > Zf) {	/* AHEF */ \
				xf = Xf;		/* x does not change */ \
				yf = Zf;		/* z into y */ \
\
				tvert2 = a100; \
			} \
			else {			/* AHBF */ \
				xf = Zf;		/* z into x */ \
				yf = Xf;		/* x into y */ \
\
				tvert2 = a001; \
			} \
		} \
	} \
	else { \
		if (Yf > Zf) { \
			xf = Yf;		/* y into x */ \
			tvert2 = a010; \
			if (Xf > Zf) {	/* AHCG */ \
				yf = Xf;		/* x into y */ \
				zf = Zf;		/* z into z */ \
\
				tvert1 = a110; \
			} \
			else {			/* AHCD */ \
				yf = Zf;		/* z into y */ \
				zf = Xf;		/* x into z */ \
\
				tvert1 = a011; \
			} \
		} \
		else {			/* AHDB */ \
			xf = Zf;		/* z into x */ \
			yf = Yf;		/* y into y */ \
			zf = Xf;		/* x into z */ \
\
			tvert1 = a011; \
			tvert2 = a001; \
		} \
	}

#define COMP_TRESULT  \
		KCP_SHIFT_RIGHT_ROUND(mullacc, temp, EVAL_FRACBITS) \
		tResult += temp;

#define TH1_3D_TETRAINTERP_D8(chan) \
	gridBaseP = gridBase##chan + baseOffset; \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert1)) & MF2_TBL_MAXVAL; \
	tvertData = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert3)) & MF2_TBL_MAXVAL; \
	mullacc = zf * (tvertData - tResult);	/* (tvert3 - tvert1) * z */ \
 \
	tvertData = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert2)) & MF2_TBL_MAXVAL; \
	mullacc += (yf * (tResult - tvertData));					/* (tvert1 - tvert2) * y */ \
 \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + 0)) & MF2_TBL_MAXVAL; \
	mullacc += (xf * (tvertData - tResult));					/* (tvert2 - A) * x */ \
 \
	COMP_TRESULT		/* A + (mults) */

#define TH1_TEST_DELTA(delta, chan) \
	highBits = delta & EVAL_HIGH_MASK; \
	if ((highBits != 0) && (highBits != EVAL_HIGH_MASK)) { \
		goto ExtendedPrecision##chan; \
	}

#define TH1_3D_TETRAINTERP_D16(chan) \
	gridBaseP = gridBase##chan + baseOffset; \
	tvert1Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert1)) & MF2_TBL_MAXVAL; \
	tvert3Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert3)) & MF2_TBL_MAXVAL; \
	delta2 = tvert3Data - tvert1Data; \
 \
	tvert2Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert2)) & MF2_TBL_MAXVAL; \
	delta1 = tvert1Data - tvert2Data; \
 \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + 0)) & MF2_TBL_MAXVAL; \
	delta0 = tvert2Data - tResult; \
 \
	TH1_TEST_DELTA(delta2, chan) \
	mullacc = zf * delta2;						/* (tvert3 - tvert1) * z */ \
 \
	TH1_TEST_DELTA(delta1, chan) \
	mullacc += (yf * delta1);					/* (tvert1 - tvert2) * y */ \
 \
	TH1_TEST_DELTA(delta0, chan) \
	mullacc += (xf * delta0);					/* (tvert2 - A) * x */ \
 \
	COMP_TRESULT		/* A + (mults) */ \
 \
 	goto evalDone##chan; \
 \
 ExtendedPrecision##chan: \
 	KCP_SHIFT_RIGHT(delta2, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta2 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH = zf * deltaHigh;						/* (tvert3 - tvert1) * z */ \
	mullacc = zf * delta2; \
 \
 	KCP_SHIFT_RIGHT(delta1, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta1 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH += yf * deltaHigh;						/* (tvert1 - tvert2) * y */ \
	mullacc += yf * delta1; \
 \
 	KCP_SHIFT_RIGHT(delta0, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta0 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH += xf * deltaHigh;						/* (tvert2 - A) * x */ \
	mullacc += xf * delta0; \
 \
 	KCP_SHIFT_RIGHT(mullacc, mullacc, EVAL_EXTENDED_BITS)	/* discard unneeded low order bits */ \
 	mullacc += mullaccH;			/* add in high order bits */ \
 	mullacc += (((1 << (EVAL_FRACBITS - EVAL_EXTENDED_BITS)) -1) >> 1);	/* round */ \
 	KCP_SHIFT_RIGHT(mullacc, mullacc, EVAL_FRACBITS - EVAL_EXTENDED_BITS); \
 	tResult += mullacc; \
 \
 evalDone##chan:

#define TH1_3D_TETRAINTERP_AND_OLUT(chan, bits) \
	TH1_3D_TETRAINTERP_D##bits(chan)	/* tetrahedral interpolation for this channel */ \
	prevRes##chan = outLut##chan[tResult];

#define TH1_STORE_DATA(chan, bits) \
	*outp##chan = prevRes##chan;	/* write to buffer */ \
	outp##chan = (KpUInt##bits##_p)((KpUInt8_p)outp##chan + outStride##chan);	/* next location */



/******************************************************************************/

void
	evalTh1i3o1d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
	}
}


/******************************************************************************/

void
	evalTh1i3o1d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(16, 0)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D16
		
		if ((ColorPart1 != prevPart1) || (data2 != prevPart2)) {

			TH1_3D_FIND_TETRA_D16
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
	}
}


/******************************************************************************/

void
	evalTh1i3o2d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o2d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D16
		
		if ((ColorPart1 != prevPart1) || (data2 != prevPart2)) {

			TH1_3D_FIND_TETRA_D16
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
	}
}


/******************************************************************************/

void
	evalTh1i3o3d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
	}
}


/******************************************************************************/

void
	evalTh1iB24oB24 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	if (outStride) {}

	for (i1 = n; i1 > 0; i1--) {
		data0 = *inp0++; 					/* get channel 0 input data */
		data1 = *inp0++; 					/* get channel 1 input data */
		data2 = *inp0++; 					/* get channel 2 input data */

		thisColor = (data0 << 16) | (data1 << 8) | (data2);	/* calc this color */
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		*outp0++ = prevRes0;	/* use result from previous color evaluation */
		*outp0++ = prevRes1;
		*outp0++ = prevRes2;
	}
}


/******************************************************************************/

void
	evalTh1iL24oL24 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	if (outStride) {}
	
	for (i1 = n; i1 > 0; i1--) {
		data2 = inp2[0]; 					/* get channel 2 input data */
		data1 = inp2[1]; 					/* get channel 1 input data */
		data0 = inp2[2]; 					/* get channel 0 input data */

		inp2 += 3;

		thisColor = (data0 << 16) | (data1 << 8) | (data2);	/* calc this color */
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}
		
		outp2[0] = prevRes2;		/* use result from previous color evaluation */
		outp2[1] = prevRes1;
		outp2[2] = prevRes0;

		outp2 += 3;
	}
}


#if defined (KPMAC)
/**************************** Quick Draw Formats **********************************/

void
	evalTh1iQDoQD (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
KpUInt32_p	inpqd, outpqd;
KpUInt32_t	alphaData;

TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	if (inStride || outStride) {}
	
	inpqd = (KpUInt32_p)(inp[0].p8 -1);
	outpqd = (KpUInt32_p)(outp[0].p8 -1);

	for (i1 = n; i1 > 0; i1--) {
		thisColor = *inpqd++;
		alphaData = thisColor & 0xff000000;		/* get alpha channel input data */
		data0 = (thisColor >> 16) & 0xff;		/* get channel 0 input data */
		data1 = (thisColor >> 8) & 0xff;		/* get channel 1 input data */
		data2 = (thisColor) & 0xff;				/* get channel 2 input data */

		thisColor &= 0xffffff;					/* clear off alpha channel */

		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		alphaData |= prevRes0 << 16;
		alphaData |= prevRes1 << 8;
		alphaData |= prevRes2;
		*outpqd++ = alphaData;
	}
}


/******************************************************************************/

void
	evalTh1iQDo3 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
KpUInt32_p	inpqd;

TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
		
	if (inStride) {}
	
	inpqd = (KpUInt32_p)(inp[0].p8 -1);

	for (i1 = n; i1 > 0; i1--) {
		thisColor = *inpqd++;
		data0 = (thisColor >> 16) & 0xff;		/* get channel 0 input data */
		data1 = (thisColor >> 8) & 0xff;		/* get channel 1 input data */
		data2 = (thisColor) & 0xff;				/* get channel 2 input data */
		
		thisColor &= 0xffffff;					/* clear off alpha channel */

		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3oQD (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
KpUInt32_p	outpqd;
KpUInt32_t	qdData;

TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	if (outStride) {}
	
	outpqd = (KpUInt32_p)(outp[0].p8 -1);

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8

		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		qdData  = prevRes0 << 16;
		qdData |= prevRes1 << 8;
		qdData |= prevRes2;
		*outpqd++ = qdData;
	}
}

#endif 	/* KPMAC */


/*********************************************************************************************/

void
	evalTh1i3o3d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D16
		
		if ((ColorPart1 != prevPart1) || (data2 != prevPart2)) {

			TH1_3D_FIND_TETRA_D16
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 16)
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
		TH1_STORE_DATA(2, 16);
	}
}


/* Tetrahedral Interpolation evaluation of 3-channel 8-bit to 3-channel 12/16-bit data */
void
	evalTh1i3o3d8to16 (	imagePtr_p	inp,
						KpInt32_p	inStride,
						KpUInt32_t	dataTypeI,
						imagePtr_p	outp,
						KpInt32_p	outStride,
						KpUInt32_t	dataTypeO,
						KpInt32_t	n,
						PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)

	TH1_3D_INIT_VARS
	TH1_3D_INIT_INVARS_D8
	TH1_3D_INIT_GRIDVARS_D16 \
	TH1_3D_INIT_OUTVARS(16) \
	TH1_3D_INIT_IN_D8
	TH1_3D_INIT_OUT_D16
	TH1_3D_INIT_DATA(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 16)
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
		TH1_STORE_DATA(2, 16);
	}
}

/* Tetrahedral Interpolation evaluation of 3-channel 12/16-bit to 3-channel 8-bit data */
void
	evalTh1i3o3d16to8 (	imagePtr_p	inp,
						KpInt32_p	inStride,
						KpUInt32_t	dataTypeI,
						imagePtr_p	outp,
						KpInt32_p	outStride,
						KpUInt32_t	dataTypeO,
						KpInt32_t	n,
						PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT_VARS
	TH1_3D_INIT_INVARS_D16
	TH1_3D_INIT_GRIDVARS_D8 \
	TH1_3D_INIT_OUTVARS(8) \
	TH1_3D_INIT_IN_D16
	TH1_3D_INIT_OUT_D8
	TH1_3D_INIT_DATA(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D16
		
		if ((ColorPart1 != prevPart1) || (data2 != prevPart2)) {

			TH1_3D_FIND_TETRA_D16
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o4d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)
TH1_3D_OCHAN_VARS(8, 3)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
	TH1_3D_OCHAN_TABLES(8, 3)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o4d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)
	TH1_3D_OCHAN_TABLES(16, 3)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D16
		
		if ((ColorPart1 != prevPart1) || (data2 != prevPart2)) {

			TH1_3D_FIND_TETRA_D16
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 16)
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
		TH1_STORE_DATA(2, 16);
		TH1_STORE_DATA(3, 16);
	}
}


/******************************************************************************/

void
	evalTh1i3o5d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)
TH1_3D_OCHAN_VARS(8, 3)
TH1_3D_OCHAN_VARS(8, 4)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
	TH1_3D_OCHAN_TABLES(8, 3)
	TH1_3D_OCHAN_TABLES(8, 4)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
		TH1_STORE_DATA(4, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o5d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)
TH1_3D_OCHAN_VARS(16, 4)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)
	TH1_3D_OCHAN_TABLES(16, 3)
	TH1_3D_OCHAN_TABLES(16, 4)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D16
		
		if ((ColorPart1 != prevPart1) || (data2 != prevPart2)) {

			TH1_3D_FIND_TETRA_D16
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 16)
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
		TH1_STORE_DATA(2, 16);
		TH1_STORE_DATA(3, 16);
		TH1_STORE_DATA(4, 16);
	}
}


/******************************************************************************/

void
	evalTh1i3o6d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)
TH1_3D_OCHAN_VARS(8, 3)
TH1_3D_OCHAN_VARS(8, 4)
TH1_3D_OCHAN_VARS(8, 5)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
	TH1_3D_OCHAN_TABLES(8, 3)
	TH1_3D_OCHAN_TABLES(8, 4)
	TH1_3D_OCHAN_TABLES(8, 5)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(5, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
		TH1_STORE_DATA(4, 8);
		TH1_STORE_DATA(5, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o6d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)
TH1_3D_OCHAN_VARS(16, 4)
TH1_3D_OCHAN_VARS(16, 5)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)
	TH1_3D_OCHAN_TABLES(16, 3)
	TH1_3D_OCHAN_TABLES(16, 4)
	TH1_3D_OCHAN_TABLES(16, 5)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D16
		
		if ((ColorPart1 != prevPart1) || (data2 != prevPart2)) {

			TH1_3D_FIND_TETRA_D16
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(5, 16)
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
		TH1_STORE_DATA(2, 16);
		TH1_STORE_DATA(3, 16);
		TH1_STORE_DATA(4, 16);
		TH1_STORE_DATA(5, 16);
	}
}

/******************************************************************************/

void
	evalTh1i3o7d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)
TH1_3D_OCHAN_VARS(8, 3)
TH1_3D_OCHAN_VARS(8, 4)
TH1_3D_OCHAN_VARS(8, 5)
TH1_3D_OCHAN_VARS(8, 6)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
	TH1_3D_OCHAN_TABLES(8, 3)
	TH1_3D_OCHAN_TABLES(8, 4)
	TH1_3D_OCHAN_TABLES(8, 5)
	TH1_3D_OCHAN_TABLES(8, 6)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(5, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(6, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
		TH1_STORE_DATA(4, 8);
		TH1_STORE_DATA(5, 8);
		TH1_STORE_DATA(6, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o7d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)
TH1_3D_OCHAN_VARS(16, 4)
TH1_3D_OCHAN_VARS(16, 5)
TH1_3D_OCHAN_VARS(16, 6)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)
	TH1_3D_OCHAN_TABLES(16, 3)
	TH1_3D_OCHAN_TABLES(16, 4)
	TH1_3D_OCHAN_TABLES(16, 5)
	TH1_3D_OCHAN_TABLES(16, 6)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D16
		
		if ((ColorPart1 != prevPart1) || (data2 != prevPart2)) {

			TH1_3D_FIND_TETRA_D16
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(5, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(6, 16)
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
		TH1_STORE_DATA(2, 16);
		TH1_STORE_DATA(3, 16);
		TH1_STORE_DATA(4, 16);
		TH1_STORE_DATA(5, 16);
		TH1_STORE_DATA(6, 16);
	}
}


/******************************************************************************/

void
	evalTh1i3o8d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)
TH1_3D_OCHAN_VARS(8, 3)
TH1_3D_OCHAN_VARS(8, 4)
TH1_3D_OCHAN_VARS(8, 5)
TH1_3D_OCHAN_VARS(8, 6)
TH1_3D_OCHAN_VARS(8, 7)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
	TH1_3D_OCHAN_TABLES(8, 3)
	TH1_3D_OCHAN_TABLES(8, 4)
	TH1_3D_OCHAN_TABLES(8, 5)
	TH1_3D_OCHAN_TABLES(8, 6)
	TH1_3D_OCHAN_TABLES(8, 7)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(5, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(6, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(7, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
		TH1_STORE_DATA(4, 8);
		TH1_STORE_DATA(5, 8);
		TH1_STORE_DATA(6, 8);
		TH1_STORE_DATA(7, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o8d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)
TH1_3D_OCHAN_VARS(16, 4)
TH1_3D_OCHAN_VARS(16, 5)
TH1_3D_OCHAN_VARS(16, 6)
TH1_3D_OCHAN_VARS(16, 7)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)
	TH1_3D_OCHAN_TABLES(16, 3)
	TH1_3D_OCHAN_TABLES(16, 4)
	TH1_3D_OCHAN_TABLES(16, 5)
	TH1_3D_OCHAN_TABLES(16, 6)
	TH1_3D_OCHAN_TABLES(16, 7)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D16
		
		if ((ColorPart1 != prevPart1) || (data2 != prevPart2)) {

			TH1_3D_FIND_TETRA_D16
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(5, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(6, 16)
			TH1_3D_TETRAINTERP_AND_OLUT(7, 16)
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
		TH1_STORE_DATA(2, 16);
		TH1_STORE_DATA(3, 16);
		TH1_STORE_DATA(4, 16);
		TH1_STORE_DATA(5, 16);
		TH1_STORE_DATA(6, 16);
		TH1_STORE_DATA(7, 16);
	}
}
