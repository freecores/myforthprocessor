/*
 * @(#)evalth14.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)evalth14.c	1.18 99/03/04
 *
 *	4 input evaluation functions using tetrahedral interpolation
 *
 *	Author:			George Pawle
 *
 *	Creation Date:	12/22/96
 *
 *    COPYRIGHT (c) 1996-1999 Eastman Kodak Company
 *    As an unpublished work pursuant to Title 17 of the United
 *    States Code.  All rights reserved.
 */

#include "kcpcache.h"

#define EVAL_HIGH_MASK (0xfffffe00)
#define EVAL_EXTENDED_BITS (8)

#define TH1_4D_OCHAN_VARS(bits, chan) \
KpUInt8_p			gridBase##chan; \
KpInt32_t			outStride##chan; \
KpUInt##bits##_p	outLut##chan, outp##chan; \
KpUInt##bits##_t	prevRes##chan;

#define TH1_4D_INIT_VARS \
KpUInt8_p	gridBaseP, thisOutLut; \
evalILut_p	inLut0, inLut1, inLut2, inLut3; \
KpUInt32_t	data0, data1, data2, data3, dataMask; \
KpInt32_t	i1, oChan, index, tResult, tvert1, tvert2, tvert3, tvert4, mullacc, temp, iLutEntries; \
KpInt32_t	inStride0 = inStride[0], inStride1 = inStride[1], inStride2 = inStride[2], inStride3 = inStride[3]; \
KpInt32_t	baseOffset, Xf, Yf, Zf, Tf, interp[4]; \
th1_4dControl_p	pentaInfo, finderP; \

#define TH1_4D_INIT_VARS_D8 \
KpInt32_t	tvertData, outLutSize = EVAL_OLUT_ENTRIESD8; \
KpUInt8_p	inp0 = inp[0].p8, inp1 = inp[1].p8, inp2 = inp[2].p8, inp3 = inp[3].p8; \
KpUInt32_t	thisColor, prevColor;

#define TH1_4D_INIT_VARS_D16 \
KpInt32_t	outLutSize = EVAL_OLUT_ENTRIESD16 * sizeof (KpUInt16_t); \
KpUInt16_p	inp0 = inp[0].p16, inp1 = inp[1].p16, inp2 = inp[2].p16, inp3 = inp[3].p16; \
KpUInt32_t	thisColor1, thisColor2, prevColor1, prevColor2 = 0; \
KpInt32_t	delta3, delta2, delta1, delta0, deltaHigh, mullaccH, highBits, tvert1Data, tvert2Data, tvert3Data, tvert4Data;

#define TH1_4D_INIT_TABLES_D8 \
	if (dataTypeI) {}  \
	if (dataTypeO) {}  \
	iLutEntries = 1 << 8; \
	inLut0 = PTTableP->etLuts[ET_I8].P; \
	gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G12].P; \
	thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O8].P; \
	prevColor = (~(*inp0)) << 24; 			/* make sure cache is not valid */

#define TH1_4D_INIT_TABLES_D16 \
	if (dataTypeI == KCM_USHORT_12) { \
		iLutEntries = 1 << 12; \
		inLut0 = PTTableP->etLuts[ET_I12].P; \
		gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G16].P; \
	} \
	else { \
		iLutEntries = 1 << 16; \
		inLut0 = PTTableP->etLuts[ET_I16].P; \
		gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G16].P; \
	} \
 \
	if (dataTypeO == KCM_USHORT_12) { \
		thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O12].P; \
	} \
	else { \
		thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O16].P; \
	} \
 \
	prevColor1 = (~(*inp0)) << 16; 			/* make sure cache is not valid */

#define TH1_4D_INIT_DATA \
	finderP = &PTTableP->etFinder[-1];	/* -1 due to Matlab indexing starting at 1 */ \
 \
	inLut1 = inLut0 + iLutEntries; \
	inLut2 = inLut1 + iLutEntries; \
	inLut3 = inLut2 + iLutEntries; \
	dataMask = iLutEntries -1;	/* set up data mask to prevent input table memory access violations */ \
	oChan = -1; \
	gridBaseP -= sizeof (mf2_tbldat_t); \
	thisOutLut -= outLutSize;

#define TH1_4D_OCHAN_TABLES(obits, chan) \
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

#define TH1_4D_INIT(bits) \
	TH1_4D_INIT_VARS \
	TH1_4D_INIT_VARS_D##bits \
	TH1_4D_INIT_TABLES_D##bits \
	TH1_4D_INIT_DATA

#define TH1_4D_GETDATA_D8 \
	data0 = *inp0; 					/* get channel 0 input data */ \
	inp0 += inStride0; \
	data1 = *inp1; 					/* get channel 1 input data */ \
	inp1 += inStride1; \
	data2 = *inp2; 					/* get channel 2 input data */ \
	inp2 += inStride2; \
	data3 = *inp3; 					/* get channel 2 input data */ \
	inp3 += inStride3; \
\
	thisColor = (data0 << 24) | (data1 << 16) | (data2 << 8) | (data3);	/* calc this color */

#define TH1_4D_GETDATA_D16 \
	data0 = *inp0; 					/* get channel 0 input data */ \
	data0 &= dataMask; \
	inp0 = (KpUInt16_p)((KpUInt8_p)inp0 + inStride0); \
	data1 = *inp1; 					/* get channel 1 input data */ \
	data1 &= dataMask; \
	inp1 = (KpUInt16_p)((KpUInt8_p)inp1 + inStride1); \
	data2 = *inp2; 					/* get channel 2 input data */ \
	data2 &= dataMask; \
	inp2 = (KpUInt16_p)((KpUInt8_p)inp2 + inStride2); \
	data3 = *inp3; 					/* get channel 2 input data */ \
	data3 &= dataMask; \
	inp3 = (KpUInt16_p)((KpUInt8_p)inp3 + inStride3); \
\
	thisColor1 = (data0 << 16) | (data1);	/* calc this color */  \
    thisColor2 = (data2 << 16) | (data3);

#define TH1_4D_FINDTETRA_D8 \
	prevColor = thisColor;  \
\
	baseOffset = inLut0[(0*FUT_INPTBL_ENT) + data0].index; 	/* pass input data through input tables */ \
	Xf = inLut0[(0*FUT_INPTBL_ENT) + data0].frac; \
	baseOffset += inLut0[(1*FUT_INPTBL_ENT) + data1].index; \
	Yf = inLut0[(1*FUT_INPTBL_ENT) + data1].frac; \
	baseOffset += inLut0[(2*FUT_INPTBL_ENT) + data2].index; \
	Zf = inLut0[(2*FUT_INPTBL_ENT) + data2].frac; \
	baseOffset += inLut0[(3*FUT_INPTBL_ENT) + data3].index; \
	Tf = inLut0[(3*FUT_INPTBL_ENT) + data3].frac; \
\
	TH1_4D_CALCTETRA

#define TH1_4D_FINDTETRA_D16 \
	prevColor1 = thisColor1;  \
	prevColor2 = thisColor2;  \
\
	baseOffset = inLut0[data0].index;		/* pass input data through input tables */ \
	Xf = inLut0[data0].frac; \
	baseOffset += inLut1[data1].index; \
	Yf = inLut1[data1].frac; \
	baseOffset += inLut2[data2].index; \
	Zf = inLut2[data2].frac; \
	baseOffset += inLut3[data3].index; \
	Tf = inLut3[data3].frac; \
\
	TH1_4D_CALCTETRA

#define TH1_4D_CALCTETRA \
	/* find the pentahedron in which the point is located */ \
	/* this builds a binary number based of the possible comparisons of the 4 interpolants */ \
	index = 0; \
	if (Xf > Yf) { \
		index += (1<<5); \
	} \
	if (Zf > Tf) { \
		index += (1<<4); \
	} \
	if (Xf > Zf) { \
		index += (1<<3); \
	} \
	if (Yf > Tf) { \
		index += (1<<2); \
	} \
	if (Yf > Zf) { \
		index += (1<<1); \
	} \
	if (Xf > Tf) { \
		index += (1<<0); \
	} \
\
	pentaInfo = finderP + pentahedron[index];	/* get pentahedron info */ \
\
	tvert1 = pentaInfo->tvert1;	/* offsets from the base grid point to the pentahedral corners */ \
	tvert2 = pentaInfo->tvert2; \
	tvert3 = pentaInfo->tvert3; \
	tvert4 = pentaInfo->tvert4; \
\
	interp[pentaInfo->dx] = Xf;	/* re-dorder the interpolants */ \
	interp[pentaInfo->dy] = Yf; \
	interp[pentaInfo->dz] = Zf; \
	interp[pentaInfo->dt] = Tf; \
	Tf = interp[0]; \
	Zf = interp[1]; \
	Yf = interp[2]; \
	Xf = interp[3];

#define COMP_TRESULT  \
		KCP_SHIFT_RIGHT_ROUND(mullacc, temp, EVAL_FRACBITS) \
		tResult += temp;

#define TH1_4D_TETRAINTERP_D8(chan) \
	gridBaseP = gridBase##chan + baseOffset; \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert4)) & MF2_TBL_MAXVAL; \
	tvertData = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert3)) & MF2_TBL_MAXVAL; \
	mullacc = Tf * (tResult - tvertData);						/* (tvert4 - tvert3) * t */ \
 \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert2)) & MF2_TBL_MAXVAL; \
	mullacc += (Zf * (tvertData - tResult));					/* (tvert3 - tvert2) * z */ \
 \
	tvertData = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert1)) & MF2_TBL_MAXVAL; \
	mullacc += (Yf * (tResult - tvertData));					/* (tvert2 - tvert1) * y */ \
 \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + 0)) & MF2_TBL_MAXVAL; \
	mullacc += (Xf * (tvertData - tResult));					/* (tvert1 - tvert0) * x */ \
 \
	COMP_TRESULT					/* tvert0 + (mullacc) */

#define TH1_TEST_DELTA(delta, chan) \
	highBits = delta & EVAL_HIGH_MASK; \
	if ((highBits != 0) && (highBits != EVAL_HIGH_MASK)) { \
		goto ExtendedPrecision##chan; \
	}

#define TH1_4D_TETRAINTERP_D16(chan) \
	gridBaseP = gridBase##chan + baseOffset; \
	tvert4Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert4)) & MF2_TBL_MAXVAL; \
	tvert3Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert3)) & MF2_TBL_MAXVAL; \
	delta3 = tvert4Data - tvert3Data; \
 \
	tvert2Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert2)) & MF2_TBL_MAXVAL; \
	delta2 = tvert3Data - tvert2Data; \
 \
	tvert1Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert1)) & MF2_TBL_MAXVAL; \
	delta1 = tvert2Data - tvert1Data; \
 \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + 0)) & MF2_TBL_MAXVAL; \
	delta0 = tvert1Data - tResult; \
 \
	TH1_TEST_DELTA(delta3, chan) \
	mullacc = Tf * delta3;						/* (tvert4 - tvert3) * t */ \
 \
	TH1_TEST_DELTA(delta2, chan) \
	mullacc += (Zf * delta2);					/* (tvert3 - tvert2) * z */ \
 \
	TH1_TEST_DELTA(delta1, chan) \
	mullacc += (Yf * delta1);					/* (tvert2 - tvert1) * y */ \
 \
	TH1_TEST_DELTA(delta0, chan) \
	mullacc += (Xf * delta0);					/* (tvert1 - tvert0) * x */ \
 \
	COMP_TRESULT		/* tvert0 + (mullacc) */ \
 \
 	goto evalDone##chan; \
 \
 ExtendedPrecision##chan: \
 	KCP_SHIFT_RIGHT(delta3, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta3 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH = Tf * deltaHigh;						/* (tvert4 - tvert3) * z */ \
	mullacc = Tf * delta3; \
 \
 	KCP_SHIFT_RIGHT(delta2, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta2 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH += Zf * deltaHigh;						/* (tvert3 - tvert2) * y */ \
	mullacc += Zf * delta2; \
 \
 	KCP_SHIFT_RIGHT(delta1, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta1 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH += Yf * deltaHigh;						/* (tvert2 - tvert1) * y */ \
	mullacc += Yf * delta1; \
 \
 	KCP_SHIFT_RIGHT(delta0, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta0 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH += Xf * deltaHigh;						/* (tvert1 - tvert0) * x */ \
	mullacc += Xf * delta0; \
 \
 	KCP_SHIFT_RIGHT(mullacc, mullacc, EVAL_EXTENDED_BITS)	/* discard unneeded low order bits */ \
 	mullacc += mullaccH;			/* add in high order bits */ \
 	mullacc += (((1 << (EVAL_FRACBITS - EVAL_EXTENDED_BITS)) -1) >> 1);	/* round */ \
 	KCP_SHIFT_RIGHT(mullacc, mullacc, EVAL_FRACBITS - EVAL_EXTENDED_BITS); \
 	tResult += mullacc; \
 \
 evalDone##chan:

#define TH1_4D_TETRAINTERP_AND_OLUT(chan, bits) \
	TH1_4D_TETRAINTERP_D##bits(chan)	/* tetrahedral interpolation for this channel */ \
	prevRes##chan = outLut##chan[tResult];

#define TH1_STORE_DATA(chan, bits) \
	*outp##chan = prevRes##chan;	/* write to buffer */ \
	outp##chan = (KpUInt##bits##_p)((KpUInt8_p)outp##chan + outStride##chan);	/* next location */


/* constant tables for finding pentahedral volumes and the corresponding coefficients and multipliers */

#define TH1_4D_COMBINATIONS 64

static KpUInt32_t	pentahedron[TH1_4D_COMBINATIONS] = { 24,  0, 23,  0,  0,  0, 12,  0,  0,  0, 22,  0,
															  0,  0,  9,  8, 18,  0,  0,  0, 17, 16, 11, 10,
															  0,  0,  0,  0,  0,  0,  0,  7, 21,  0,  0,  0,
															  0,  0,  0,  0, 20,  6, 19,  3,  0,  0,  0,  2,
															 15, 14,  0,  0,  0, 13,  0,  0,  0,  5,  0,  0,
															  0,  4,  0,  1};

/**************************************************************
 * evalTh1i4o1 ---- 8 BIT
 *  Evaluation routine for evaluating 4 channel to 1 channel.
 */

void
	evalTh1i4o1d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(8, 0)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GETDATA_D8
		
		if (thisColor != prevColor) {

			TH1_4D_FINDTETRA_D8
					
			TH1_4D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
	}
}

/**************************************************************
 * evalTh1i4o1 ---- 16 bit
 *  Evaluation routine for evaluating 4 channel to 1 channel.
 */

void
	evalTh1i4o1d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(16, 0)

	TH1_4D_INIT(16)
	TH1_4D_OCHAN_TABLES(16, 0)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GETDATA_D16
		
		if ((thisColor1 != prevColor1) || (thisColor2 != prevColor2)) {

			TH1_4D_FINDTETRA_D16

			TH1_4D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
	}
}

/**************************************************************
 * evalTh1i4o2 --- 8 bit
 **************************************************************/

void
	evalTh1i4o2d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GETDATA_D8
		
		if (thisColor != prevColor) {

			TH1_4D_FINDTETRA_D8
					
			TH1_4D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_4D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
	}
}

/**************************************************************
 * evalTh1i4o2  ----  16 bit
 **************************************************************/

void
	evalTh1i4o2d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(16, 0)
TH1_4D_OCHAN_VARS(16, 1)

	TH1_4D_INIT(16)
	TH1_4D_OCHAN_TABLES(16, 0)
	TH1_4D_OCHAN_TABLES(16, 1)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GETDATA_D16
		
		if ((thisColor1 != prevColor1) || (thisColor2 != prevColor2)) {

			TH1_4D_FINDTETRA_D16
					
			TH1_4D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_4D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
	}
}

/**************************************************************
 * evalTh1i4o3   -----   8 bit
 **************************************************************/
 
void
	evalTh1i4o3d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)
TH1_4D_OCHAN_VARS(8, 2)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)
	TH1_4D_OCHAN_TABLES(8, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GETDATA_D8
		
		if (thisColor != prevColor) {

			TH1_4D_FINDTETRA_D8
					
			TH1_4D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_4D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_4D_TETRAINTERP_AND_OLUT(2, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
	}
}


#if defined (KPMAC)

typedef union QDBuff_s {
	KpUInt8_t	cbuf[4];
	KpUInt32_t	lword;
} QDBuff_t;

/**************************************************************
 * evalTh1i4o3QD
 **************************************************************/
void
	evalTh1i4o3QD (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
KpUInt32_p	outpL;
QDBuff_t	QDoBuf;
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)
TH1_4D_OCHAN_VARS(8, 2)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)
	TH1_4D_OCHAN_TABLES(8, 2)

	if (outStride) {}

	outpL = (KpUInt32_p)(outp[0].p8 -1);

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GETDATA_D8
		
		if (thisColor != prevColor) {

			TH1_4D_FINDTETRA_D8
					
			TH1_4D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_4D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_4D_TETRAINTERP_AND_OLUT(2, 8)
		}

		QDoBuf.lword = *outpL;				/* preserve alpha channel */

		QDoBuf.cbuf[1] = prevRes0;			/* use result from previous color evaluation */
		QDoBuf.cbuf[2] = prevRes1;
		QDoBuf.cbuf[3] = prevRes2;
		
		*outpL++ = QDoBuf.lword;
	}
}

#endif 	/* if defined KPMAC */


/**************************************************************
 * evalTh1i4o3   ---- 16 bit
 **************************************************************/
 
void
	evalTh1i4o3d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(16, 0)
TH1_4D_OCHAN_VARS(16, 1)
TH1_4D_OCHAN_VARS(16, 2)

	TH1_4D_INIT(16)
	TH1_4D_OCHAN_TABLES(16, 0)
	TH1_4D_OCHAN_TABLES(16, 1)
	TH1_4D_OCHAN_TABLES(16, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GETDATA_D16
		
		if ((thisColor1 != prevColor1) || (thisColor2 != prevColor2)) {

			TH1_4D_FINDTETRA_D16
					
			TH1_4D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_4D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
			TH1_4D_TETRAINTERP_AND_OLUT(2, 16)
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
		TH1_STORE_DATA(2, 16);
	}
}


/**************************************************************
 * evalTh1i4o4   ----  8 bit
 *  Evaluation routine for evaluating 4 channel to 4 channels.
 **************************************************************/
void
	evalTh1i4o4d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)
TH1_4D_OCHAN_VARS(8, 2)
TH1_4D_OCHAN_VARS(8, 3)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)
	TH1_4D_OCHAN_TABLES(8, 2)
	TH1_4D_OCHAN_TABLES(8, 3)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GETDATA_D8
		
		if (thisColor != prevColor) {

			TH1_4D_FINDTETRA_D8
					
			TH1_4D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_4D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_4D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_4D_TETRAINTERP_AND_OLUT(3, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
	}
}


/**************************************************************
 * evalTh1i4oB32
 **************************************************************/

void
	evalTh1iB32oB32 (	imagePtr_p	inp,
						KpInt32_p	inStride,
						KpUInt32_t	dataTypeI,
						imagePtr_p	outp,
						KpInt32_p	outStride,
						KpUInt32_t	dataTypeO,
						KpInt32_t	n,
						PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)
TH1_4D_OCHAN_VARS(8, 2)
TH1_4D_OCHAN_VARS(8, 3)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)
	TH1_4D_OCHAN_TABLES(8, 2)
	TH1_4D_OCHAN_TABLES(8, 3)

	if (inStride || outStride) {}

	thisColor = ~inp0[3];		/* make sure cache is not valid */

	for (i1 = n; i1 > 0; i1--) {
		data0 = *inp0++; 					/* get channel 0 input data */
		data1 = *inp0++; 					/* get channel 1 input data */
		data2 = *inp0++; 					/* get channel 2 input data */
		data3 = *inp0++; 					/* get channel 3 input data */
		
		thisColor = (data0 << 24) | (data1 << 16) | (data2 << 8) | (data3);	/* calc this color */
		
		if (thisColor != prevColor) {

			TH1_4D_FINDTETRA_D8
					
			TH1_4D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_4D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_4D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_4D_TETRAINTERP_AND_OLUT(3, 8)
		}

		*outp0++ = prevRes0;				/* use result from previous color evaluation */
		*outp0++ = prevRes1;
		*outp0++ = prevRes2;
		*outp0++ = prevRes3;
	}
}

/**************************************************************
 * evalTh1i4oL32
 *  Evaluation routine for evaluating 4 channel to 4 channels,
 **************************************************************/
void
	evalTh1iL32oL32 (	imagePtr_p	inp,
						KpInt32_p	inStride,
						KpUInt32_t	dataTypeI,
						imagePtr_p	outp,
						KpInt32_p	outStride,
						KpUInt32_t	dataTypeO,
						KpInt32_t	n,
						PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)
TH1_4D_OCHAN_VARS(8, 2)
TH1_4D_OCHAN_VARS(8, 3)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)
	TH1_4D_OCHAN_TABLES(8, 2)
	TH1_4D_OCHAN_TABLES(8, 3)

	if (inStride || outStride) {}
	
	thisColor = ~inp0[0];		/* make sure cache is not valid */

	for (i1 = n; i1 > 0; i1--) {
		data3 = inp3[0]; 					/* get channel 3 input data */
		data2 = inp3[1]; 					/* get channel 2 input data */
		data1 = inp3[2]; 					/* get channel 1 input data */
		data0 = inp3[3]; 					/* get channel 0 input data */

		inp3 += 4;

		thisColor = (data0 << 24) | (data1 << 16) | (data2 << 8) | (data3);	/* calc this color */
		
		if (thisColor != prevColor) {

			TH1_4D_FINDTETRA_D8
					
			TH1_4D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_4D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_4D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_4D_TETRAINTERP_AND_OLUT(3, 8)
		}

		outp3[0] = prevRes3;		/* use result from previous color evaluation */
		outp3[1] = prevRes2;
		outp3[2] = prevRes1;
		outp3[3] = prevRes0;

		outp3 += 4;
	}

}

/**************************************************************
 * evalTh1i4o4   ---- 16 bit
 *  Evaluation routine for evaluating 4 channel to 4 channels.
 **************************************************************/
void
	evalTh1i4o4d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(16, 0)
TH1_4D_OCHAN_VARS(16, 1)
TH1_4D_OCHAN_VARS(16, 2)
TH1_4D_OCHAN_VARS(16, 3)

	TH1_4D_INIT(16)
	TH1_4D_OCHAN_TABLES(16, 0)
	TH1_4D_OCHAN_TABLES(16, 1)
	TH1_4D_OCHAN_TABLES(16, 2)
	TH1_4D_OCHAN_TABLES(16, 3)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GETDATA_D16
		
		if ((thisColor1 != prevColor1) || (thisColor2 != prevColor2)) {

			TH1_4D_FINDTETRA_D16
					
			TH1_4D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_4D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
			TH1_4D_TETRAINTERP_AND_OLUT(2, 16)
			TH1_4D_TETRAINTERP_AND_OLUT(3, 16)
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
		TH1_STORE_DATA(2, 16);
		TH1_STORE_DATA(3, 16);
	}
}
