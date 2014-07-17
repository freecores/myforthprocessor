/*
 * @(#)evalth1c.c	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)evalth1c.c	1.30 99/03/04

	Contains:	tetrahedral interpolation with optimized tables

	Author:		George Pawle

	COPYRIGHT (c) 1996-1999 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "fut_util.h" 
#include "attrib.h"
#include "kcpcache.h"

static evalTh1Proc_t getTh1EvalFuncOpt (evalControl_p, KpUInt32_t, KpUInt32_t, KpInt32_p);
static PTErr_t initGridInfo (KpInt32_t, KpInt32_t, PTTable_p, fut_gtbl_p);
static void th1MFtbl2InLut (mf2_tbldat_p, KpInt32_t, KpInt32_t, KpInt32_t, evalILut_p);
static void freeEtMem (etMem_p);
static void nullEtMem (etMem_p);
static void lockEtMem (etMem_p);
static void unlockEtMem (etMem_p);
static void allocEtMem (etMem_p, KpInt32_t);
static KpInt32_t hasEtMem (etMem_p);
#if defined KCP_SINGLE_EVAL_CACHE
static PTErr_t	getEvalStatePT (PTRefNum_p);
static PTErr_t	putEvalStatePT (PTRefNum_t);
#endif

/* constant tables for finding pentahedral volumes and
 * the corresponding coefficients and multipliers
 */
static KpUInt32_t	bceiBase[TH1_4D_PENTAHEDROA] =
	{ 9,  9,  9,  9,  9,  9,  5,  5,  5,  5, 5, 5,  3,  3,  3,  3, 3, 3,  2,  2,  2,  2, 2, 2};
static KpUInt32_t	dfgjkmBase[TH1_4D_PENTAHEDROA] =
	{13, 13, 10, 11, 11, 10, 13, 13,  6,  7, 7, 6, 11, 11,  4,  7, 7, 4, 10, 10,  4,  6, 6, 4};
static KpUInt32_t	hlnoBase[TH1_4D_PENTAHEDROA] =
	{15, 14, 14, 15, 12, 12, 15, 14, 14, 15, 8, 8, 15, 12, 12, 15, 8, 8, 14, 12, 12, 14, 8, 8};
static KpUInt32_t	dxBase[TH1_4D_PENTAHEDROA] =
	{ 3,  3,  3,  3,  3,  3,  2,  2,  1,  1, 0, 0,  2,  2,  1,  1, 0, 0,  2,  2,  1,  1, 0, 0};
static KpUInt32_t	dyBase[TH1_4D_PENTAHEDROA] =
	{ 2,  2,  1,  1,  0,  0,  3,  3,  3,  3, 3, 3,  1,  0,  0,  2, 2, 1,  1,  0,  0,  2, 2, 1};
static KpUInt32_t	dzBase[TH1_4D_PENTAHEDROA] =
	{ 1,  0,  0,  2,  2,  1,  1,  0,  0,  2, 2, 1,  3,  3,  3,  3, 3, 3,  0,  1,  2,  0, 1, 2};
static KpUInt32_t	dtBase[TH1_4D_PENTAHEDROA] =
	{ 0,  1,  2,  0,  1,  2,  0,  1,  2,  0, 1, 2,  0,  1,  2,  0, 1, 2,  3,  3,  3,  3, 3, 3};


/******************************************************************************/
/* get the evaluation function to use */
PTErr_t
	getEvalFunc (	KpInt32_t		nEvals,
					KpUInt32_t		ifmt,
					KpUInt32_t		ofmt,
					evalControl_p	evalControlP)
{
KpInt32_t	numOutputs, totalEvals;

	evalControlP->optimizedEval = 0;			/* assume that optimized tables can not be used */

	if (evalControlP->nFuts == 1) {
		evalControlP->evalFunc = getTh1EvalFuncOpt (evalControlP, ifmt, ofmt, &numOutputs);		/* try for optimized function */

		if (evalControlP->evalFunc != NULL) {
			totalEvals = numOutputs * nEvals;
				
			if ( !		/* if none of the following apply... */
				(((((evalControlP->evalDataTypeI == KCM_USHORT) || (evalControlP->evalDataTypeO == KCM_USHORT)) && (totalEvals < TH1_MIN_16BIT_EVALS))	/* verify sufficient # of evaluations */
				|| (((evalControlP->evalDataTypeI != KCM_USHORT) && (evalControlP->evalDataTypeO != KCM_USHORT)) && (totalEvals < TH1_MIN_EVALS)))
				&& (ifmt != FMT_QD) && (ofmt != FMT_QD))) {		/* still need to use iQDoQD for QD */

				evalControlP->optimizedEval = 1;	/* use optimized method */
			}
		}
	}

	if (evalControlP->optimizedEval == 0) {
		evalControlP->evalFunc = evalTh1gen;
	}	

	return KCP_SUCCESS;
}


/************************************************/
/* See if this evaluation has been optimized.
   Returns function address if available.
*/

static evalTh1Proc_t
	getTh1EvalFuncOpt (	evalControl_p	evalControlP,
						KpUInt32_t		ifmt,
						KpUInt32_t		ofmt,
						KpInt32_p		numOutputsP)
{
evalTh1Proc_t	func;
KpUInt32_t 	iomask, imask, omask;
KpInt32_t 	i2, nIn, o, nOut;
fut_chan_p	futChan;
KpHandle_t	futH;

/*	return NULL;	return null to force non-optimized evaluation */

	iomask = evalControlP->ioMaskList [0];
	imask = (KpInt32_t)FUT_IMASK(iomask);
	omask = (KpInt32_t)FUT_OMASK(iomask);

	futH = (KpHandle_t) evalControlP->evalList[0]->data;	/* check the fut */

	/* find number of output channels */
	for (nOut = 0, o = 0; o < FUT_NOCHAN; o++) {
		if (omask & FUT_BIT(o)) {
			for (nIn = 0, i2 = 0; i2 < FUT_NICHAN; i2++) {	/* make sure input tables are shared */
				if (imask & FUT_BIT(i2)) {
				
					futChan = FCHANP(FFUTP(futH)->chanHandle[o]);	/* get pointer to chan */

					if (futChan->itblHandle[i2] != FFUTP(futH)->itblHandle[i2]) {
						return NULL;		/* Input tables not shared */
					}
					nIn++;
				}
			}
			nOut++;
		}
	}

	*numOutputsP = nOut;

	switch (evalControlP->evalDataTypeI) {
	case KCM_UBYTE:
		switch (nIn) {	
		case 3:
			switch (nOut) {
				case 1:
					func = evalTh1i3o1d8;
					break;

				case 2:
					func = evalTh1i3o2d8;
					break;

				case 3:
					func = evalTh1i3o3d8;
				
		#if defined (KPMAC)
					if (ifmt == FMT_QD) {
						if (ofmt == FMT_QD) {
							func = evalTh1iQDoQD;
						}
						else {
							func = evalTh1iQDo3;
						}
					}
					else {
						if (ofmt == FMT_QD) {
							func = evalTh1i3oQD;
						}
						else {
		#endif
							/* look for 3-channel 8-bit to 3-channel 12-bit */
							if (FMT_GENERAL_12BIT == ofmt) {
								func = evalTh1i3o3d8to16;
							}
							else /* otherwise, perform nominal checks */
							{
								if ((ifmt == FMT_BIGENDIAN24) && (ofmt == FMT_BIGENDIAN24)) {
									func = evalTh1iB24oB24;
								}
								else {
									if ((ifmt == FMT_LITTLEENDIAN24) && (ofmt == FMT_LITTLEENDIAN24)) {
										func = evalTh1iL24oL24;
									}
								}
							}
		#if defined (KPMAC)
						}
					}
		#endif
					break;

				case 4:
					func = evalTh1i3o4d8;
					break;	  

				case 5:
					func = evalTh1i3o5d8;
					break;

				case 6:
					func = evalTh1i3o6d8;
					break;

				case 7:
					func = evalTh1i3o7d8;
					break;

				case 8:
					func = evalTh1i3o8d8;
					break;

				default:
					func = NULL;
					break;
			}
			
			break;
			
		case 4:
			switch (nOut) {
				case 1:
					func = evalTh1i4o1d8;
					break;

				case 2:
					func = evalTh1i4o2d8;
					break;

				case 3:
					func = evalTh1i4o3d8;

		#if defined (KPMAC)
					if (ofmt == FMT_QD) {
						func = evalTh1i4o3QD;
					}
		#endif
					break;

				case 4:
					func = evalTh1i4o4d8;

					if ((ifmt == FMT_BIGENDIAN32) && (ofmt == FMT_BIGENDIAN32)) {
						func = evalTh1iB32oB32;
					}
					else {
						if ((ifmt == FMT_LITTLEENDIAN32) && (ofmt == FMT_LITTLEENDIAN32)) {
							func = evalTh1iL32oL32;
						}
					}
					
					break;	  

				default:
					func = NULL;
					break;
			}
			break;

			default:
				func = NULL;
				break;
		}
		break;

	case KCM_USHORT_12:
	case KCM_USHORT:
		switch (nIn) {
		case 3:
			switch (nOut) {
				case 1:
					func = evalTh1i3o1d16;
					break;

				case 2:
					func = evalTh1i3o2d16;
					break;

				case 3:
					/* look for 3-channel 12/16-bit to 3-channel 8-bit */
					if (evalControlP->evalDataTypeO == KCM_UBYTE) {
						func = evalTh1i3o3d16to8;
					} else {
						func = evalTh1i3o3d16;
					}
					break;

				case 4:
					func = evalTh1i3o4d16;
					break;	  

				case 5:
					func = evalTh1i3o5d16;
					break;

				case 6:
					func = evalTh1i3o6d16;
					break;

				case 7:
					func = evalTh1i3o7d16;
					break;

				case 8:
					func = evalTh1i3o8d16;
					break;

				default:
					func = NULL;
					break;
			}
			break;
			
		case 4:
			switch (nOut) {
				case 1:
					func = evalTh1i4o1d16;
					break;

				case 2:
					func = evalTh1i4o2d16;
					break;

				case 3:
					func = evalTh1i4o3d16;
					break;

				case 4:
					func = evalTh1i4o4d16;
					break;	  

				default:
					func = NULL;
					break;
			}
			break;

		default:
			func = NULL;
			break;
		}
		break;

	default:
		func = NULL;
		break;
	}

	return (func);
}

 
/******************************************************************************/
/* initialize the evaluation tables */
PTErr_t
	initEvalTables (	evalControl_p	evalControlP)
{
KpInt32_t		theSizef, i1, i2, numInputs, nOutputChans, endBits, cvrtShft;
PTTable_p		PTTableP;
fut_p			fut;
fut_itbl_p		theITbl, iTblsP [FUT_NICHAN];
fut_chan_p		thisChan;
fut_gtbl_p		firstGTbl, aGTbl;
fut_otbl_p		oTblsP [FUT_NOCHAN];
mf2_tbldat_p	gTblsP [FUT_NOCHAN], iTblDat, interleavedGridP;
PTErr_t			PTErr = KCP_SUCCESS;
KpInt32_t		j, inputTableEntries, gridTblEntries, outputTableEntries, outputTableEntrySize, destTableMaxValue;
KpInt32_t		totalInputLutBytes, totalGridTableBytes, outputLutBytes, totalOutputLutBytes;
evalILut_p		iLut;
mf2_tbldat_t	tmp, cvrtRnd, identityTable[] = {0,0xffff};
etMem_p			etILutsP, etGLutsP, etOLutsP;
#if defined KCP_SINGLE_EVAL_CACHE
PTRefNum_t		evalStatePT;
#endif

	PTTableP = evalControlP->evalList[0];

	/* Input Tables controls */
	switch (evalControlP->evalDataTypeI) {
	case KCM_UBYTE:		
		inputTableEntries = 1 << 8;
		etILutsP = &PTTableP->etLuts[ET_I8];
		break;
		
	case KCM_USHORT_12:		
		inputTableEntries = 1 << 12;
		etILutsP = &PTTableP->etLuts[ET_I12];
		break;
		
	case KCM_USHORT:		
		inputTableEntries = 1 << 16;
		etILutsP = &PTTableP->etLuts[ET_I16];
		break;
		
	default:
		goto ErrOut1;
	}

	/* set up grid and output tables controls */
	/* based on the macro KCP_CONVERT_DOWN(data, startBits, endBits) */
	switch (evalControlP->evalDataTypeO) {
	case KCM_UBYTE:		
		endBits = 12;				/* use 12 bit grid table data */
		cvrtShft = MF2_TBL_BITS - endBits;
		cvrtRnd = 1 << (cvrtShft -1);
		etGLutsP = &PTTableP->etLuts[ET_G12];
		etOLutsP = &PTTableP->etLuts[ET_O8];
		outputTableEntries = 1 << 12;
		outputTableEntrySize = sizeof(KpUInt8_t);
		destTableMaxValue = 0xff;
		break;
		
	case KCM_USHORT_12:		
		endBits = 16;				/* use 16 bit grid table data */
		cvrtShft = 0;
		cvrtRnd = 0;
		etGLutsP = &PTTableP->etLuts[ET_G16];
		etOLutsP = &PTTableP->etLuts[ET_O12];
		outputTableEntries = 1 << 16;
		outputTableEntrySize = sizeof(KpUInt16_t);
		destTableMaxValue = 0xfff;
		break;
		
	case KCM_USHORT:		
		endBits = 16;				/* use 16 bit grid table data */
		cvrtShft = 0;
		cvrtRnd = 0;
		etGLutsP = &PTTableP->etLuts[ET_G16];
		etOLutsP = &PTTableP->etLuts[ET_O16];
		outputTableEntries = 1 << 16;
		outputTableEntrySize = sizeof(KpUInt16_t);
		destTableMaxValue = 0xffff;
		break;
		
	default:
		goto ErrOut1;
	}

	/* if not optimized or any table is not ready, must lock the futs */
	if ((evalControlP->optimizedEval == (KpUInt32_t) 0) ||
		(hasEtMem (etILutsP) != 1) || (hasEtMem (etGLutsP) != 1) || (hasEtMem (etOLutsP) != 1)) {

		/* lock all of the futs which will be used */
		for (i1 = 0; i1 < evalControlP->nFuts; i1++) {
			fut = fut_lock_fut (evalControlP->evalList[i1]->data);
			if ((fut == NULL) || (fut_to_mft (fut) != 1)) {
				PTErr = KCP_PTERR_2;
				goto GetOut;
			}

			evalControlP->evalList[i1]->dataP = fut;
		}		

		if (evalControlP->optimizedEval == (KpUInt32_t) 0) {	/* not optimized, do not build tables */
			return KCP_SUCCESS;
		}

		fut = PTTableP->dataP;
		if ( ! IS_FUT(fut)) return KCP_FAILURE;

		/* Find number of input variables */
		for (i2 = 0, numInputs = 0; i2 < FUT_NICHAN ; i2++) {
			if (fut->iomask.in & FUT_BIT(i2)) {
				iTblsP[numInputs] = fut->itbl[i2];	/* make a list of input tables */
				numInputs++;
			}
		}

		/* Find number of output channels */
		firstGTbl = NULL;
		for (i2 = 0, nOutputChans = 0; i2 < FUT_NOCHAN; i2++) {
			if (fut->iomask.out & FUT_BIT(i2)) {
				thisChan = fut->chan[i2];	/* this output channel is being evaluated */
				
				gTblsP[nOutputChans] = thisChan->gtbl->refTbl;	/* make a list of gtbls */
				oTblsP[nOutputChans] = thisChan->otbl;			/* and otbls */

				aGTbl = thisChan->gtbl;							/* shared input tables, so any gtbl will do */

				if (firstGTbl == NULL) {
					firstGTbl = aGTbl;							/* remember first gtbl */
				}

				/* all grids must have the same dimensionality */
				for (i1 = 0; i1 < FUT_NICHAN ; i1++) {
					if (aGTbl->size[i1] != firstGTbl->size[i1]) {
						return KCP_INVAL_EVAL;					/* sorry */
					}
				}

				nOutputChans++;
			}
		}

		PTErr = initGridInfo (numInputs, nOutputChans, PTTableP, firstGTbl);	/* set up grid table descriptors */
		if (PTErr != KCP_SUCCESS) {
			return	PTErr;
		}

		#if defined KCP_SINGLE_EVAL_CACHE
		PTErr = getEvalStatePT (&evalStatePT);
		if (PTErr != KCP_SUCCESS) {
			return	PTErr;
		}

		if (evalStatePT != PTTableP->refNum) {
			freeEvalTables (evalStatePT);			/* retain a single set of optimized tables */
		}
		#endif
	}

	if (hasEtMem (etILutsP) == 1) {
		lockEtMem (etILutsP);	/* lock the required table */
	}
	else {
		totalInputLutBytes = inputTableEntries * sizeof (evalILut_t) * numInputs;

		allocEtMem (etILutsP, totalInputLutBytes);	/* allocate necessary memory for the input tables */
		if (hasEtMem (etILutsP) != 1) {
			goto ErrOut2;
		}
		
		/* set up the input table for each variable
		 * this is done in 2 steps:
		 * expand (if necessary) the current input table to the size needed for the input data
		 * convert that table into a lut for evaluation
		 */

		theSizef = nOutputChans * sizeof (mf2_tbldat_t);
		
		for (i2 = numInputs-1; i2 >= 0 ; i2--) {
			theITbl = iTblsP[i2];				/* input table to convert */
			iLut = (evalILut_p)etILutsP->P + (i2 * inputTableEntries);	/* converted table */

				/* because an entry of the eval lut is larger than an entry of the input table data, */
				/* the end of the eval lut can be used as temporary memory to expand the input table data. */
			
			iTblDat = (mf2_tbldat_p)(iLut + inputTableEntries);	/* end of lut */
			iTblDat -= inputTableEntries;						/* back up to start of expanded table */

			convert1DTable (	theITbl->refTbl, sizeof (mf2_tbldat_t), theITbl->refTblEntries, MF2_TBL_MAXVAL,
							iTblDat, sizeof (mf2_tbldat_t), inputTableEntries, MF2_TBL_MAXVAL,
							KCP_MAP_END_POINTS, KCP_MAP_END_POINTS);

			th1MFtbl2InLut (iTblDat, inputTableEntries, theITbl->size, theSizef, iLut);	/* convert the input table to an eval lut */
			
			theSizef *= theITbl->size;
		}
	}
	
	/* Grid Tables */
	/* are the grid tables already interleaved? */
	if (hasEtMem (etGLutsP) == 1) {
		lockEtMem (etGLutsP);	/* lock the required table */
	}
	else {
		gridTblEntries = firstGTbl->tbl_size / sizeof (mf2_tbldat_t);
		totalGridTableBytes = gridTblEntries * nOutputChans * sizeof (mf2_tbldat_t);	/* size needed for this fut's grid tables */

		allocEtMem (etGLutsP, totalGridTableBytes);	/* allocate necessary memory for the input tables */
		if (hasEtMem (etGLutsP) != 1) {
			goto ErrOut2;
		}
		/* interleave the grid tables */
		interleavedGridP = etGLutsP->P;
		for (i2 = 0; i2 < gridTblEntries; i2++) {
			for (j = 0; j < nOutputChans; j++) {
				tmp = *gTblsP[j]++;
				tmp = (tmp + (cvrtRnd - (tmp >> endBits))) >> cvrtShft;
				*interleavedGridP++ = tmp;
			}
		}
	}

	/* Output Tables */
	if (hasEtMem (etOLutsP) == 1) {
		lockEtMem (etOLutsP);	/* lock the required table */
	}
	else {
		KpUInt8_p	oLuts;
	
		outputLutBytes = outputTableEntries * outputTableEntrySize;
		totalOutputLutBytes = outputLutBytes * nOutputChans;	/* size needed for this fut's output luts */

		allocEtMem (etOLutsP, totalOutputLutBytes);		/* allocate necessary memory for the output tables */
		if (hasEtMem (etOLutsP) != 1) {
			goto ErrOut2;
		}	

		/* set up the output table for each channel */
		oLuts = etOLutsP->P;

		for (i2 = 0; i2 < nOutputChans; i2++) {
			mf2_tbldat_p	srcTable;
			KpInt32_t		srcTableEntries;
			fut_otbl_p		otbl;

			otbl = oTblsP[i2];

			if ((IS_OTBL(otbl)) && (otbl->refTbl != NULL)) {
				srcTable = otbl->refTbl;
				srcTableEntries = otbl->refTblEntries;
			}
			else {
				srcTable = identityTable;
				srcTableEntries = 2;
			}
						
			convert1DTable (srcTable, sizeof (mf2_tbldat_t), srcTableEntries, MF2_TBL_MAXVAL,
							&oLuts [i2 * outputLutBytes], outputTableEntrySize, outputTableEntries, destTableMaxValue,
							KCP_MAP_END_POINTS, KCP_MAP_END_POINTS);
		}
	}

	/* were tables built? */
GetOut:
	if ((PTErr == KCP_NO_MEMORY) || (PTMemTest () == 0)) {	/* switch to general eval function */
		evalControlP->optimizedEval = 0;	/* not optimized */
		evalControlP->evalFunc = evalTh1gen;
		PTErr = KCP_SUCCESS;
	}
	#if defined KCP_SINGLE_EVAL_CACHE
	else {									/* Update evaluation state info */
		if (PTErr == KCP_SUCCESS) {
			PTErr = putEvalStatePT (PTTableP->refNum);	/* remember which PT has the optimized tables */
		}
	}
	#endif

	return PTErr;


ErrOut1:
	PTErr = KCP_INVAL_EVAL;
	goto ErrOut;
	
ErrOut2:	
	PTErr = KCP_NO_MEMORY;

ErrOut:	
	freeEvalTables (PTTableP->refNum);	/* free whatever may have been allocated */
	goto GetOut;
}



/******************************************************************************/
/* Convert fut itbl to input table in special format */
static PTErr_t
	initGridInfo (	KpInt32_t		numInputs,
					KpInt32_t		nOutputChans,
					PTTable_p		PTTableP,
					fut_gtbl_p		aGTbl)
{
KpInt32_t	theSizef, i2, dimx, dimy, dimz, index;

	/* set up offsets in grid */
	switch (numInputs) {
	case 1:
		dimx = 0;	/* not used */
		dimy = 0;
		dimz = 0;
		break;

	case 2:
		dimx = 0;	/* not used */
		dimy = 0;
		dimz = aGTbl->size[1];
		break;

	case 3:
		dimx = 0;	/* not used */
		dimy = aGTbl->size[1];
		dimz = aGTbl->size[2];
		break;

	case 4:
		dimx = aGTbl->size[1];
		dimy = aGTbl->size[2];
		dimz = aGTbl->size[3];
		break;

	default:
		return (KCP_NOT_COMPLETE);
	}		
		
	PTTableP->etGOffsets[0] = 0;													/* offset to 0000 */
	PTTableP->etGOffsets[1] = 1;													/* offset to 0001 */
	PTTableP->etGOffsets[2] = dimz;													/* offset to 0010 */
	PTTableP->etGOffsets[3] = PTTableP->etGOffsets[2] + 1;							/* offset to 0011 */
	PTTableP->etGOffsets[4] = dimy * PTTableP->etGOffsets[2];						/* offset to 0100 */
	PTTableP->etGOffsets[5] = PTTableP->etGOffsets[4] + 1;							/* offset to 0101 */
	PTTableP->etGOffsets[6] = PTTableP->etGOffsets[4] + PTTableP->etGOffsets[2];	/* offset to 0110 */
	PTTableP->etGOffsets[7] = PTTableP->etGOffsets[6] + 1;							/* offset to 0111 */
	PTTableP->etGOffsets[8] = dimx * PTTableP->etGOffsets[4];						/* offset to 1000 */
	PTTableP->etGOffsets[9] = PTTableP->etGOffsets[8] + 1;							/* offset to 1001 */
	PTTableP->etGOffsets[10] = PTTableP->etGOffsets[8] + PTTableP->etGOffsets[2];	/* offset to 1010 */
	PTTableP->etGOffsets[11] = PTTableP->etGOffsets[10] + 1;						/* offset to 1011 */
	PTTableP->etGOffsets[12] = PTTableP->etGOffsets[8] + PTTableP->etGOffsets[4];	/* offset to 1100 */
	PTTableP->etGOffsets[13] = PTTableP->etGOffsets[12] + 1;						/* offset to 1101 */
	PTTableP->etGOffsets[14] = PTTableP->etGOffsets[8] + PTTableP->etGOffsets[4] + PTTableP->etGOffsets[2];	/* offset to 1110 */
	PTTableP->etGOffsets[15] = PTTableP->etGOffsets[14] + 1;						/* offset to 1111 */

	theSizef = nOutputChans * sizeof (mf2_tbldat_t);
	for (i2 = 0; i2 < TH1_NUM_OFFSETS; i2++) {
		PTTableP->etGOffsets[i2] *= theSizef;	/* adjust for # and size of channels in interleaved grid tables */
	}

	for (i2 = 0; i2 < TH1_4D_PENTAHEDROA; i2++) {
		index = bceiBase[i2] -1;
		PTTableP->etFinder[i2].tvert1 = PTTableP->etGOffsets[index];	/* grid offset to bcei corner */
		index = dfgjkmBase[i2] -1;
		PTTableP->etFinder[i2].tvert2 = PTTableP->etGOffsets[index];	/* grid offset to dfgjkm corner */
		index = hlnoBase[i2] -1;
		PTTableP->etFinder[i2].tvert3 = PTTableP->etGOffsets[index];	/* grid offset to hlno corner */
		PTTableP->etFinder[i2].tvert4 = PTTableP->etGOffsets[15];			/* grid offset to bcei corner */
		PTTableP->etFinder[i2].dx = dxBase[i2];							/* dx multiplier */
		PTTableP->etFinder[i2].dy = dyBase[i2];							/* dy multiplier */
		PTTableP->etFinder[i2].dz = dzBase[i2];							/* dz multiplier */
		PTTableP->etFinder[i2].dt = dtBase[i2];							/* dt multiplier */
	}
	
	return KCP_SUCCESS;
}


/******************************************************************************/
/* Convert fut itbl to input table in special format */
static void
	th1MFtbl2InLut (	mf2_tbldat_p	futLut,
						KpInt32_t		nEntries,
						KpInt32_t		gridDim,
						KpInt32_t		sizef,
						evalILut_p		evalLut)
{
KpInt32_t	i2, index, v, maxDim;
KpFloat32_t	factor;
KpUInt32_t	frac;

	/* Convert the itbl value to values convenient for grid table interpolation. */
	maxDim = gridDim -1;

	factor = ((KpFloat32_t) ((gridDim -1) * (1 << EVAL_FRACBITS))) / MF2_TBL_MAXVAL;
	
	for (i2 = 0; i2 < nEntries; i2++) {
		v = (KpInt32_t) ((futLut[i2] * factor) + 0.5);	/* get fut lut value */
		
		index = v >> EVAL_FRACBITS;				/* get fut integer index value */
		frac = v & EVAL_FRACMASK;	 			/* get fut fractional value */

		if (index == gridDim -1) {				/* prevent addressing past end of table */
			frac = (1 << EVAL_FRACBITS) -1;
			index--;
		}

		evalLut[i2].index = index * sizef;		/* position index for offset into fut grid */
		evalLut[i2].frac = frac;
	}
}


/******************************************************************************/
/* Deallocate all allocated memory and null out handles. */
/* Note: will deallocate properly if only some of the memory got allocated. */
void
	freeEvalTables (	PTRefNum_t	PTRefNum)
{
PTTable_p	PTTableP;
KpInt32_t	i1;

#if defined KCP_SINGLE_EVAL_CACHE
	putEvalStatePT (NULL);
#endif

	PTTableP = lockPTTable (PTRefNum);

	if (PTTableP == NULL) {
		return;
	}

	for (i1 = 0; i1 < ET_NLUT; i1++) {
		freeEtMem (&PTTableP->etLuts[i1]);
	}

	nullEvalTables (PTTableP);			/* set evaluation state to null */

	unlockPTTable (PTRefNum);
}


/******************************************************************************/
/* Unlock the evaluation state */

void
	unlockEvalTables (	PTTable_p	PTTableP)
{
KpInt32_t	i1;

	if (PTTableP != NULL) {

		for (i1 = 0; i1 < ET_NLUT; i1++) {
			unlockEtMem (&PTTableP->etLuts[i1]);
		}
	}
}


/******************************************************************************/
/* Set the evaluation state to unused */
void
	nullEvalTables (	PTTable_p	PTTableP)
{
KpInt32_t	i1;

	if (PTTableP == NULL) {
		return;
	}

	for (i1 = 0; i1 < ET_NLUT; i1++) {
		nullEtMem (&PTTableP->etLuts[i1]);
	}

	return;
}


/******************************************************************************/
/* allocate evaluation state memory */
static void
	allocEtMem (etMem_p		theEtMemP,
				KpInt32_t	bytesNeeded)
{
	/* is memory already allocated and the correct size? */
	if (theEtMemP->bytes != bytesNeeded) {
		freeEtMem (theEtMemP);							/* free current allocation */
		
		theEtMemP->P = allocBufferPtr (bytesNeeded);	/* get new memory */
		if (theEtMemP->P != NULL) {
			theEtMemP->bytes = bytesNeeded;
		}
	}
}


/******************************************************************************/
/* free evaluation state memory */
static void
	freeEtMem (etMem_p	theEtMemP)
{
	if (theEtMemP->P != NULL) {
		freeBufferPtr (theEtMemP->P);
	}
	nullEtMem (theEtMemP);
}


/******************************************************************************/
/* check for evaluation state memory */
static KpInt32_t
	hasEtMem (etMem_p	theEtMemP)
{
	if (theEtMemP->P != NULL) {
		return 1;
	}
	else {
		return 0;
	}
}


/******************************************************************************/
/*  set evaluation state memory to null */
static void
	nullEtMem (etMem_p	theEtMemP)
{
	theEtMemP->H = NULL;
	theEtMemP->P = NULL;
	theEtMemP->bytes = 0;
}


/******************************************************************************/
/* lock evaluation state memory */
static void
	lockEtMem (etMem_p	theEtMemP)
{
	if (theEtMemP != NULL) {
		if (theEtMemP->H != NULL) {
			theEtMemP->P = (etMem_p) lockBuffer (theEtMemP->H);
			theEtMemP->H = NULL;	/* indicate locked */
		}
	}
}


/******************************************************************************/
/* unlock evaluation state memory */
static void
	unlockEtMem (etMem_p	theEtMemP)
{
	if (theEtMemP != NULL) {
		theEtMemP->H = getHandleFromPtr(theEtMemP->P);	/* get handle */
		(void) unlockBuffer (theEtMemP->H);
	}
}


#if defined KCP_SINGLE_EVAL_CACHE

/* get the current chaining state from thread globals */
static PTErr_t
	getEvalStatePT (	PTRefNum_p	evalStatePT)
{
processGlobals_p	pGP;

	pGP = loadProcessGlobals();
	if (pGP == NULL) {
		return	KCP_NO_PROCESS_GLOBAL_MEM;
	}

	*evalStatePT = pGP->evalStatePT;

	unloadProcessGlobals ();

	return KCP_SUCCESS;
}


/* put the current chaining state in the thread globals */
static PTErr_t
	putEvalStatePT (	PTRefNum_t	evalStatePT)
{
processGlobals_p	pGP;

	pGP = loadProcessGlobals();
	if (pGP == NULL) {
		return	KCP_NO_PROCESS_GLOBAL_MEM;
	}

	pGP->evalStatePT = evalStatePT;

	unloadProcessGlobals ();

	return KCP_SUCCESS;
}

#endif
