/*
 * @(#)iomf.c	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)iomf.c	1.14 99/02/04

	Contains:	functions to read and write binary matrix fut files.

	Author:		George Pawle

	All opens, closes, reads and writes are performed with the functions
	KpOpen, Kp_close, Kp_read, and Kp_write, respectively
	to provide an "operating system independent" i/o interface.	These
	functions are implemented differently for each operating system, and are
	defined in the library kcms_sys.

	COPYRIGHT (c) 1993-1999 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include <string.h>
#include <stdio.h>
#include "kcptmgr.h"
#include "attrib.h"
#include "makefuts.h"

#define FUT_MATRIX_ZERO (0x0)
#define FUT_MATRIX_ONE (0x10000)
#define GBUFFER_SIZE (MF1_TBL_ENT*2)

/* constants for CP version 0 */
#define KCP_V0_ITABLE_SIZE	(256)
#define KCP_V0_OTABLE_SIZE	(4096)
#define KCP_9TO6_MASK	(0x3c0)
#define KCP_3TO0_MASK	(0xf)

/* data types for various flavors of mft2's */
typedef enum mft2Type_e {
	KCP_UNKNOWN_TYPE	=0,
	KCP_V0_TYPE			=1,		/* mft2 with version 0 transform */
	KCP_MFT2_TYPE_1		=3,		/* mft2 with lutType clarification */
	KCP_MFT2_FROM_MFT1	=4		/* mft2 generated from mft1 */
} mft2Type_t, FAR* mft2Type_p;

/* ICC mft2 info
 * used for reading in ICC profile matrix-fut tables
 */
typedef struct mft2_s {
	KpInt32_t		nIChan;						/* Number of input channels */
	KpInt32_t		nOChan;						/* Number of output channels */
    KpInt32_t		clutDimSize;				/* Number of grid points */
    KpInt32_t		iTblEntries;				/* Number of input table entries */
    KpInt32_t		clutEntries;				/* Number of clut table entries */
    KpInt32_t		oTblEntries;				/* Number of output table entries */
    mft2Type_t		type;						/* type of data in tables */
	mf2_tbldat_p	inputTable [FUT_NICHAN];	/* addresses of input tables */
	mf2_tbldat_p	clutTable;					/* address of clut table, which is interleaved grid tables */
	mf2_tbldat_p	outputTable [FUT_NOCHAN];	/* addresses of output tables */
} mft2_t, FAR* mft2_p;

static void			fut_free_mft (mft2_p);
static KpUInt32_t	fut_read_mft_data (KpFd_p, fut_hdr_p, Fixed_p, mft2_p);
static mft2Type_t	checkCPv0Gen (mft2_p);
static mft2Type_t	checkT2DGen (mft2_p);
static fut_p		futFromMFutTbls	(mft2_p, fut_hdr_p);
static KpInt32_t	calcNextGBufSize (KpInt32_t, KpInt32_p);
static KpDouble64_t	getValueRatio (PTDataMap_t, KpInt32_t, KpInt32_t);
static KpDouble64_t	getIndexRatio (PTDataMap_t, KpInt32_t, KpInt32_t);


/* fut_readMFutHdr reads the header of a matrix fut from
 * an open file descriptor and stores it in a fut I/O header.
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_read error
 * -2 to -4 on a table specific error
 */

KpInt32_t
	fut_readMFutHdr(	KpFd_p		fd,
						fut_hdr_p	futHdr)
{
KpInt32_t	status;
KpUInt32_t	dummy;
KpUInt8_t	inVars, outChans, gridDim;
		
	futHdr->iDataClass = KCP_UNKNOWN;
	futHdr->oDataClass = KCP_UNKNOWN;

	/* read in the common matrix fut stuff */
	status = Kp_read (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt32_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&inVars, sizeof(KpUInt8_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&outChans, sizeof(KpUInt8_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&gridDim, sizeof(KpUInt8_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt8_t));

	if (status == 1) {
		futHdr->version = 1;					/* save in fut locations */
		futHdr->order = 0;

		if ((inVars < 1) || (inVars > FUT_NICHAN)) {
			return (-2);
		}
		futHdr->icode[0] = (KpInt32_t) inVars;

		if ((outChans < 1) || (outChans > FUT_NOCHAN)) {
			return (-3);
		}
		futHdr->icode[1] = (KpInt32_t) outChans;

		if (gridDim < 2) {
			return (-4);
		}
		futHdr->icode[2] = (KpInt32_t) gridDim;
	}
	else {
		status = -1;
	}

	return status;
}


/* fut_readMFutTbls reads the tables of a matrix fut
 * from an open file descriptor and puts them into the supplied fut.
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_read error
 * -2 to -4 on a table specific error
 */
fut_p
	fut_readMFutTbls (	KpFd_p			fd,
						fut_hdr_p		futHdr,
						Fixed_p			matrix)
{
KpInt32_t	status;
KpUInt16_t	iTblEntries = 0, oTblEntries = 0;
mft2_t		theMft2;
fut_p		fut = NULL;

	theMft2.nIChan = futHdr->icode[0];				/* get the size info */
	theMft2.nOChan = futHdr->icode[1];
	theMft2.clutDimSize = futHdr->icode[2];

	status = fut_read_mft_data (fd, futHdr, matrix, &theMft2);	/* get the data for the mft */

	if (status == 1) {	/* determine the type of data in the mft */
		if ((futHdr->spaceIn == KCM_ADOBE_LAB) || (futHdr->spaceOut == KCM_ADOBE_LAB)) {
			fut = get_linlab_fut (KCP_LINLAB_GRID_SIZE, KCP_LAB8_NEUTRAL, KCP_LAB_PCS, KCP_LAB_PCS);
		}
		else {
			if (futHdr->magic == PTTYPE_MFT1) {		/* if transform is mft1 */
				theMft2.type = KCP_MFT2_FROM_MFT1;	/* in differentiated PCS/device mode */
			}
			else {

				/* fix a bug in OCS */
				if ((futHdr->spaceOut == KCM_CMYK) && (theMft2.nOChan == 3)) {
					futHdr->spaceOut = KCM_CIE_LAB;
					futHdr->oDataClass = KCP_LAB_PCS;
				}

				theMft2.type = checkCPv0Gen (&theMft2);				/* mft2 made by V0 CP? */
				
				if (theMft2.type != KCP_V0_TYPE) {
					if (futHdr->profileType != KCM_ICC_TYPE_0) {	/* from non-version 0 profile? */
						theMft2.type = KCP_MFT2_TYPE_1;				/* in differentiated PCS/device mode */
					}
					else {
						theMft2.type = KCP_V0_TYPE;					/* default to version 0 type */
					}
				}
			}

			#if defined KCP_DIAG_LOG
			{KpChar_t	string[256];
			sprintf (string, "fut_readMFutTbls\n iDataClass %d, oDataClass %d, profileType %d, spaceIn %d, spaceOut %d, theMft2.type %d\n",
						futHdr->iDataClass, futHdr->oDataClass, futHdr->profileType, futHdr->spaceIn, futHdr->spaceOut, theMft2.type);
			kcpDiagLog (string);}
			#endif

			fut = futFromMFutTbls (&theMft2, futHdr);				/* convert the tables into a fut */
		}
	}

	fut_free_mft (&theMft2);	/* free the mft memory */

	return fut;
}


/* get the data for the mft */
/* read the data and convert mft1 to mft2 format if needed */
KpUInt32_t
	fut_read_mft_data (	KpFd_p			fd,
						fut_hdr_p		futHdr,
						Fixed_p			matrix,
						mft2_p			theMft2)
{
KpUInt16_t		tmpUI16;
KpInt32_t		srciTblEntries, i1, status, srcEntryBytes;
KpInt32_t		srciTblSize, srcCLutSize, srcoTblSize, mft2iTblSize, mft2CLutSize, mft2oTblSize;
KpUInt8_p		mf1dataP;
KpUInt32_t		tmpData;
PTDataMap_t		tableMap;
mf2_tbldat_t	gData, tmpTbl [MF2_MAX_TBL_ENT];
mf2_tbldat_p	gDataP;
Fixed_t			lMatrix [MF_MATRIX_DIM * MF_MATRIX_DIM];
Fixed_p			lMatrixP;

	theMft2->inputTable[0] = NULL;		/* in case of error */
	theMft2->clutTable = NULL;
	theMft2->outputTable[0] = NULL;

	/* read the matrix tables */
	if (matrix != NULL) {
		lMatrixP = matrix;				/* return matrix data */
	}
	else {
		lMatrixP = (Fixed_p)&lMatrix;	/* discard matrix data */
	}

	status = Kp_read (fd, (KpGenericPtr_t) lMatrixP, sizeof(Fixed_t) * MF_MATRIX_DIM * MF_MATRIX_DIM);
	if (status != 1) {
		goto ErrOutM1;
	}

#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)lMatrixP, MF_MATRIX_DIM * MF_MATRIX_DIM);
#endif

	/* get the number of input and output table entries */
	switch (futHdr->magic) {
	case PTTYPE_MFT1:
		srcEntryBytes = sizeof (mf1_tbldat_t);	/* size of each entry in this table */
		srciTblEntries = MF1_TBL_ENT;

		if ((futHdr->iDataClass == KCP_VARIABLE_RANGE) || (futHdr->iDataClass == KCP_XYZ_PCS)) {
			theMft2->iTblEntries = MF2_STD_ITBL_SIZE;
		}
		else {
			theMft2->iTblEntries = MF1_TBL_ENT;
		}

		theMft2->oTblEntries = MF1_TBL_ENT;
		break;

	case PTTYPE_MFT2:
		srcEntryBytes = sizeof (mf2_tbldat_t);	/* size of each entry in this table */

		status = Kp_read (fd, (KpGenericPtr_t)&tmpUI16, sizeof (KpUInt16_t));
		if (status != 1) {
			goto ErrOutM1;
		}
	#if (FUT_MSBF == 0)
		Kp_swab16 ((KpGenericPtr_t)&tmpUI16, 1);
	#endif

		if ((tmpUI16 < MF2_MIN_TBL_ENT) || (tmpUI16 > MF2_MAX_TBL_ENT)) {
			goto ErrOut0;
		}

		srciTblEntries = tmpUI16;
		theMft2->iTblEntries = srciTblEntries;

		status = Kp_read (fd, (KpGenericPtr_t)&tmpUI16, sizeof (KpUInt16_t));
		if (status != 1) {
			goto ErrOutM1;
		}
	#if (FUT_MSBF == 0)
		Kp_swab16 ((KpGenericPtr_t)&tmpUI16, 1);
	#endif

		if ((tmpUI16 < MF2_MIN_TBL_ENT) || (tmpUI16 > MF2_MAX_TBL_ENT)) {
			goto ErrOut0;
		}

		theMft2->oTblEntries = tmpUI16;
		break;

	default:
		goto ErrOutM2;	/* unknown type */
	}
	
	theMft2->clutEntries = theMft2->nOChan;
	for (i1 = 0; i1 < theMft2->nIChan; i1++) {	/* calc total entries in the clut */
		theMft2->clutEntries *= theMft2->clutDimSize;
	}

	srciTblSize = srciTblEntries * srcEntryBytes;				/* size in bytes of each table */
	mft2iTblSize = theMft2->iTblEntries * sizeof (mf2_tbldat_t);
	srcCLutSize = theMft2->clutEntries * srcEntryBytes;
	mft2CLutSize = theMft2->clutEntries * sizeof (mf2_tbldat_t);
	srcoTblSize = theMft2->oTblEntries * srcEntryBytes;
	mft2oTblSize = theMft2->oTblEntries * sizeof (mf2_tbldat_t);

	theMft2->inputTable[0] = allocBufferPtr (mft2iTblSize * theMft2->nIChan);	/* get the needed memory for input */
	if (theMft2->inputTable[0] == NULL) {
		goto ErrOut0;
	}

	theMft2->clutTable = allocBufferPtr (mft2CLutSize);		/* clut */
	if (theMft2->clutTable == NULL) {
		goto ErrOut0;
	}

	theMft2->outputTable[0] = allocBufferPtr (mft2oTblSize * theMft2->nOChan);	/* and output tables */
	if (theMft2->outputTable[0] == NULL) {
		goto ErrOut0;
	}

	/* get the input table data */
	for (i1 = 0; i1 < theMft2->nIChan; i1++) {

		status = Kp_read (fd, (KpGenericPtr_t)tmpTbl, srciTblSize);	/* read the input table */
		if (status != 1) {
			goto ErrOutM1;
		}

		theMft2->inputTable[i1] = theMft2->inputTable[0] + (i1 * theMft2->iTblEntries);

		if (futHdr->magic == PTTYPE_MFT1) {			/* convert mft1 to 16 bit reference */
			if ((futHdr->iDataClass == KCP_VARIABLE_RANGE) || (futHdr->iDataClass == KCP_XYZ_PCS)) {
				tableMap = KCP_BASE_MAX_TO_REF16;	/* 8 to 16 bit PCS */
			}
			else {
				tableMap = KCP_MAP_END_POINTS;		/* 8 to 16 bit device */
			}

			convert1DTable (tmpTbl, sizeof (mf1_tbldat_t), srciTblEntries, MF1_TBL_MAXVAL,
							theMft2->inputTable[i1], sizeof (mf2_tbldat_t), theMft2->iTblEntries, MF2_TBL_MAXVAL,
							tableMap, KCP_MAP_END_POINTS);
		}
		else {	/* already mft2 format, just copy */
		#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)tmpTbl, theMft2->iTblEntries);
		#endif
			KpMemCpy (theMft2->inputTable[i1], tmpTbl, mft2iTblSize);
		}
	}

	/* get the clut data */
	mf1dataP = (KpUInt8_p) theMft2->clutTable;
	if (futHdr->magic == PTTYPE_MFT1) {
		mf1dataP += (mft2CLutSize - srcCLutSize);	/* load into end of buffer */
	}

	status = Kp_read (fd, (KpGenericPtr_t) mf1dataP, srcCLutSize);	
	if (status != 1) {
		goto ErrOutM1;
	}

	if (futHdr->magic == PTTYPE_MFT1) {		/* convert to mft2 using map end points */
		gDataP = theMft2->clutTable;
		for (i1 = 0; i1 < theMft2->clutEntries; i1++) {
			tmpData = (KpUInt32_t) mf1dataP [i1];
			gData = (mf2_tbldat_t) (((tmpData * MF2_TBL_MAXVAL) + (MF1_TBL_MAXVAL >> 1)) / MF1_TBL_MAXVAL);
			gDataP[i1] = gData;				/* store each clut entry */
		}
	}
	else {
	#if (FUT_MSBF == 0)
		Kp_swab16 ((KpGenericPtr_t)theMft2->clutTable, theMft2->clutEntries);
	#endif
	}

	/* get the output table data */
	for (i1 = 0; i1 < theMft2->nOChan; i1++) {

		status = Kp_read (fd, (KpGenericPtr_t)tmpTbl, srcoTblSize);	/* read the output table */
		if (status != 1) {
			goto ErrOutM1;
		}

		theMft2->outputTable[i1] = theMft2->outputTable[0] + (i1 * theMft2->oTblEntries);

		if (futHdr->magic == PTTYPE_MFT1) {	/* convert source to 16 bit reference */
			if ((futHdr->oDataClass == KCP_VARIABLE_RANGE) || (futHdr->oDataClass == KCP_XYZ_PCS)) {
				tableMap = KCP_BASE_MAX_TO_REF16;	/* 8 to 16 bit Lab or XYZ */
			}
			else {
				tableMap = KCP_MAP_END_POINTS;		/* 8 to 16 bit device */
			}

			convert1DTable (tmpTbl, sizeof (mf1_tbldat_t), theMft2->oTblEntries, MF1_TBL_MAXVAL,
							theMft2->outputTable[i1], sizeof (mf2_tbldat_t), theMft2->oTblEntries, MF2_TBL_MAXVAL,
							KCP_MAP_END_POINTS, tableMap);
		}
		else {	/* already 16 bits, just copy */
		#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)tmpTbl, theMft2->oTblEntries);
		#endif
			KpMemCpy (theMft2->outputTable[i1], tmpTbl, mft2oTblSize);
		}
	}

GetOut:
	return status;


ErrOutM2:
	status = -2;
	goto GetOut;

ErrOutM1:
	status = -1;
	goto GetOut;

ErrOut0:
	status = 0;
	goto GetOut;
}


void
	fut_free_mft (mft2_p	theMft2)
{
	freeBufferPtr (theMft2->inputTable[0]);	/* release all mft2 memory */
	theMft2->inputTable[0] = NULL;
	freeBufferPtr (theMft2->clutTable);
	theMft2->clutTable = NULL;
	freeBufferPtr (theMft2->outputTable[0]);
	theMft2->outputTable[0] = NULL;
}


/* futFromMFutTbls the tables of an mft2 to build a fut with mft2 reference tables
 * It checks for special cases and handles them appropriately.
 * Returns: 
 * 1 on success
 * -2 on memory allocation error
 */
static fut_p
	futFromMFutTbls (	mft2_p		theMft2,
						fut_hdr_p	futHdr)
{
fut_p			fut = NULL;
KpInt32_t		i1, i2, srcOTableMax, iTblEntries, gTblEntries;
KpUInt32_t		tmpData;
PTDataMap_t		inputMap, outputMap;
mf2_tbldat_t	gData, tmpMftTable [MF2_MAX_TBL_ENT];
mf2_tbldat_p	mftDataP, gDataP;
KpInt32_t		iomask, dimTbl[FUT_NICHAN] = {1, 1, 1, 1, 1, 1, 1, 1};
fut_itbl_p		itbl, itbls[FUT_NICHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_gtbl_p		gtbl, gtbls[FUT_NOCHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_otbl_p		otbl, otbls[FUT_NOCHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

	/* set up the input mapping mode */
	if ((theMft2->type == KCP_V0_TYPE) && ((futHdr->iDataClass == KCP_VARIABLE_RANGE) || (futHdr->iDataClass == KCP_XYZ_PCS))) {
		if (theMft2->iTblEntries == KCP_V0_ITABLE_SIZE) {
			iTblEntries = MF2_STD_ITBL_SIZE;		/* switch to a better size */
		}
		else {
			iTblEntries = theMft2->iTblEntries;		/* retain size */
		}
		inputMap = KCP_BASE_MAX_TO_REF16;
	}
	else {
		iTblEntries = theMft2->iTblEntries;		/* this results in copying tables */
		inputMap = KCP_MAP_END_POINTS;
	}

	/* convert each input table to mft2 */
	iomask = 0;
	for (i1 = 0; i1 < theMft2->nIChan; i1++) {
		if ((itbl = fut_alloc_itbl ()) == NULL) {	/* get an input table */
			goto GetOut;
		}

		itbls[i1] = itbl;					/* build itbl list */

		if (fut_alloc_imftdat (itbl, iTblEntries) == NULL) {
			goto GetOut;
		}

		itbl->id = fut_unique_id ();		/* this table has its own data */

		iomask |= FUT_IN(FUT_BIT(i1));		/* build i/o mask */
		dimTbl[i1] = theMft2->clutDimSize;	/* and grid table dimensions */		

		itbl->size = dimTbl[i1];
		itbl->dataClass = futHdr->iDataClass;

		convert1DTable (theMft2->inputTable [i1], sizeof (mf2_tbldat_t), theMft2->iTblEntries, MF2_TBL_MAXVAL,
						itbl->refTbl, sizeof (mf2_tbldat_t), iTblEntries, MF2_TBL_MAXVAL,
						inputMap, KCP_MAP_END_POINTS);
	}

	/* calc # of grid table entries */
	gTblEntries = theMft2->clutEntries / theMft2->nOChan;

	/* set up the output mapping mode */
	if (theMft2->type == KCP_V0_TYPE) {
		if ((futHdr->oDataClass == KCP_VARIABLE_RANGE) || (futHdr->oDataClass == KCP_XYZ_PCS)) {
			outputMap = KCP_BASE_MAX_TO_REF16;
			srcOTableMax = FUT_OUT_MAXVAL;
		}
		else {
			outputMap = KCP_MAP_END_POINTS;
			srcOTableMax = FUT_MAX_PEL12;
		}
	}
	else {
		srcOTableMax = MF2_TBL_MAXVAL;		/* this results in copying tables */
		outputMap = KCP_MAP_END_POINTS;
	}

	/* convert each channel */
	for (i1 = 0; i1 < theMft2->nOChan; i1++) {
		iomask |= FUT_OUT(FUT_BIT(i1));		/* build i/o mask */

		/* convert the grid tables */
		if ((gtbl = fut_alloc_gtbl ()) == NULL) {	/* get a grid table */
			goto GetOut;
		}

		gtbls[i1] = gtbl;					/* build gtbl list */

		gtbl->id = fut_unique_id ();		/* this table has its own data */

		gtbl->tbl_size = sizeof(fut_gtbldat_t);

		for (i2 = 0; i2 < FUT_NICHAN; i2++) {
			gtbl->size[i2] = (KpInt16_t)dimTbl[i2];		/* insert size of each dimension */
			gtbl->tbl_size *= gtbl->size[i2];	/* calc total grid size */
		}

		if (fut_alloc_gmftdat (gtbl) == NULL) {
			goto GetOut;
		}

		mftDataP = theMft2->clutTable;		/* source data */
		mftDataP += i1;						/* offset for this channel */
		gDataP = gtbl->refTbl;				/* destination grid table pointer */
		
		for (i2 = 0; i2 < gTblEntries; i2++) {
			gData = *mftDataP;
			mftDataP += theMft2->nOChan;	/* next channel data */

			if (theMft2->type == KCP_V0_TYPE) {
				tmpData = (KpUInt32_t) gData >> (MF2_TBL_BITS - FUT_GRD_BITS);
				gData = (mf2_tbldat_t) (((tmpData * MF2_TBL_MAXVAL) + (FUT_GRD_MAXVAL >> 1)) / FUT_GRD_MAXVAL);
			}

			gDataP[i2] = gData;	/* store each grid table entry */
		}

		/* convert the output tables to mft2 */
		if ((otbl = fut_alloc_otbl ()) == NULL) {	/* get an output table */
			goto GetOut;
		}

		otbls[i1] = otbl;							/* build otbl list */

		if (fut_alloc_omftdat (otbl, theMft2->oTblEntries) == NULL) {	/* get memory for table data */
			goto GetOut;
		}

		otbl->id = fut_unique_id ();				/* this table has its own data */
		otbl->dataClass = futHdr->oDataClass;		/* define data class */

		mftDataP = theMft2->outputTable [i1];		/* source data */

		if (theMft2->type == KCP_V0_TYPE) {
			for (i2 = 0; i2 < theMft2->oTblEntries; i2++) {
				tmpMftTable [i2] = mftDataP[i2] >> (MF2_TBL_BITS - FUT_GRD_BITS);	/* extract actual data from high 12 bits */
			}
			
			mftDataP = tmpMftTable;					/* this is now the source data */
		}

		/* convert otbl data into reference table data */
		convert1DTable (mftDataP, sizeof (mf2_tbldat_t), theMft2->oTblEntries, srcOTableMax,
						otbl->refTbl, sizeof (mf2_tbldat_t), theMft2->oTblEntries, MF2_TBL_MAXVAL,
						KCP_MAP_END_POINTS, outputMap);
	}

	/* Assemble FuT:  */
	fut = fut_new (iomask, itbls, gtbls, otbls);
	if (fut != NULL) {
		KpInt32_t	isTwoCubedILabCS;

		/* test for image Lab input and PCS Lab output with 2 grid points in each dimension */
		isTwoCubedILabCS = 0;
		if (((futHdr->spaceIn == KCM_IMAGE_LAB) ||(futHdr->spaceIn == KCM_RGB)) && (futHdr->spaceOut == KCM_CIE_LAB)) {
			KpInt32_t	i1;

			isTwoCubedILabCS = 1;

			for (i1 = 0; i1 < 3; i1++) {
				fut_itbl_p	itbl;
				fut_chan_p	chan;

				chan = fut->chan[i1];
				if ((! IS_CHAN(chan)) || (chan->imask != FUT_XYZ)) {	/* must have 3 output channels, each a function of 3 inputs */
					isTwoCubedILabCS = 0;
				}

				itbl = fut->itbl[i1];						/* every grid dimension must be 2 */
				if (! IS_ITBL(itbl) || (itbl->size != 2)) {
					isTwoCubedILabCS = 0;
				}
			}
		}

		if (isTwoCubedILabCS == 1) {
			PTDataClass_t	dataClass = KCP_FIXED_RANGE;
			fut_p			resizeFut, futResized;
			KpInt32_t		lab8diff, lab16diff;
			mf2_tbldat_t	lNeutral, aNeutral, bNeutral, lab8a, lab16a;
			mf2_tbldat_p	indat[3], outdat[1];

			if (theMft2->type != KCP_MFT2_FROM_MFT1) {
				/* evaluate a* channel with lab16 neutral and lab8 neutral
				 * if the result is closest to lab16 neutral, assumr lab16 but clip to lab8
				 */
				indat[0] = &lNeutral; indat[1] = &aNeutral; indat[2] = &bNeutral;
				outdat[0] = &lab8a;

				lNeutral = KCP_LAB8_L_NEUTRAL; aNeutral = KCP_LAB8_AB_NEUTRAL; bNeutral = KCP_LAB8_AB_NEUTRAL;
				evaluateFut (fut, FUT_Y, KCM_USHORT, 1, (KpGenericPtr_t FAR*) indat, (KpGenericPtr_t FAR*) outdat);
				lab8diff = lab8a & 0xffff;
				lab8diff -= KCP_LAB16_AB_NEUTRAL;
				if (lab8diff < 0) {
					lab8diff = -lab8diff;			/* absolute value */
				}

				outdat[0] = &lab16a;
				lNeutral = KCP_LAB16_L_NEUTRAL; aNeutral = KCP_LAB16_AB_NEUTRAL; bNeutral = KCP_LAB16_AB_NEUTRAL;
				evaluateFut (fut, FUT_Y, KCM_USHORT, 1, (KpGenericPtr_t FAR*) indat, (KpGenericPtr_t FAR*) outdat);
				lab16diff = lab16a &0xffff;
				lab16diff -= KCP_LAB16_AB_NEUTRAL;
				if (lab16diff < 0) {
					lab16diff = -lab16diff;			/* absolute value */
				}

				if (lab16diff < lab8diff) {
					dataClass = KCP_LAB_PCS;
				}
			}

			resizeFut = get_linlab_fut (KCP_LINLAB_GRID_SIZE, KCP_LAB8_NEUTRAL, dataClass, dataClass);

			if (resizeFut != NULL) {
				futResized = fut_comp (fut, resizeFut, iomask);	/* resize the fut */
				fut_free (resizeFut);
				if (futResized != NULL) {
					fut_free (fut);			/* free small fut */
					fut = futResized;		/* use resized fut */
				}
			}
		}
	}

GetOut:
	fut_free_tbls (FUT_NICHAN, (void *)itbls);
	fut_free_tbls (FUT_NOCHAN, (void *)gtbls);
	fut_free_tbls (FUT_NOCHAN, (void *)otbls);

	return fut;
}


/* mf_store_fp stores fut to the file named "filename", performing the
 * open and close automatically.  Returns 1 on success, 0 or negative
 * on error.
 */
KpInt32_t
	mf_store_fp (	fut_p			fut,
					KpChar_p		filename,
					KpFileProps_t	fileProps,
					KpInt32_t		MFutType)
{
KpFd_t		fd;
KpInt32_t	ret = 0;

	/* Open with the new e mode for exclusive.  The file must be closed, or at least unlocked when done */
	if (KpOpen (filename, "e", &fd, &fileProps) ) {
		if ((ret = makeMftTblDat (fut)) == 1) {
			ret = fut_writeMFut_Kp (&fd, fut, NULL, MFutType);
		}

		(void) Kp_close (&fd);
	}

	return (ret);
}


/* fut_writeMFut_Kp writes a fut in the specified matrix fut format
 * to an open file descriptor.
 * Returns: 
 * 1 on success
 * 0 on invalid fut error
 * -1 on header, id string, or Kp_write error
 * -2 to -5 on a table specific error
 */
KpInt32_t
	fut_writeMFut_Kp (	KpFd_p		fd,
						fut_p		fut,
						Fixed_p		matrix,
						KpInt32_t	MFutType)
{
KpInt32_t	status;
KpUInt32_t	dummy = 0;
KpUInt8_t	inputChans, outputChans, LUTDimensions;
KpInt32_t	inputChansT, outputChansT, LUTDimensionsT;
KpInt32_t	i1, i2, i3, wrtEntryBytes, outCount, outBytes, gData, dstData;
KpInt32_t	wrtiTblEntries, wrtiTableMaxValue, gTblEntries, wrtgTableMaxValue;
KpInt32_t	wrtoTblEntries, wrtoTableMaxValue, totalGSize;
PTDataMap_t	inputMap, outputMap;
Fixed_t		lMatrix[MF_MATRIX_DIM * MF_MATRIX_DIM];
KpUInt16_t	tmpUI16;
fut_chan_p	chan;
KpUInt8_p	mf1dataP;
KpUInt16_p	mf2dataP;
mf2_tbldat_p	gDataP[FUT_NOCHAN];
mf2_tbldat_t	tmpTbl[MF2_MAX_TBL_ENT];
KpInt32_t	tagType;

	status = fut_mfutInfo (fut, &LUTDimensionsT, &inputChansT, &outputChansT, MFutType,
								&wrtiTblEntries, &gTblEntries, &wrtoTblEntries);
	if (status != 1) {
		goto GetOut;
	}
	
	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "fut_writeMFut_Kp\n MFutType %x\n", MFutType);
	kcpDiagLog (string);}
	#endif

	LUTDimensions = (KpUInt8_t)LUTDimensionsT; /* type conversion */
	inputChans = (KpUInt8_t)inputChansT;
	outputChans = (KpUInt8_t)outputChansT;
	
	if (MFutType == PTTYPE_MFT2_VER_0) {		/* for header of LUT tag */
		tagType = PTTYPE_MFT2;
	}
	else {
		tagType = MFutType;
	}

#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)&tagType, 1);	
#endif

	/* write out the common matrix fut stuff */
	status = Kp_write (fd, (KpGenericPtr_t)&tagType, sizeof(KpInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&inputChans, sizeof(KpUInt8_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&outputChans, sizeof(KpUInt8_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&LUTDimensions, sizeof(KpUInt8_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt8_t));

	if (status != 1) {
		goto ErrOutM1;
	}
	
	/* get the matrix to write */
	if (matrix != NULL) {	/* copy the matrix */
		for (i1 = 0; i1 < (MF_MATRIX_DIM * MF_MATRIX_DIM); i1++) {
			lMatrix[i1] = matrix[i1];
		}
	}
	else {	/* create an identity matrix when there is no matrix */
		for (i1 = 0, i3 = 0; i1 < MF_MATRIX_DIM; i1++) {
			for (i2 = 0; i2 < MF_MATRIX_DIM; i2++, i3++) {
				if (i1 == i2) {
					lMatrix[i3] = FUT_MATRIX_ONE;
				}
				else {
					lMatrix[i3] = FUT_MATRIX_ZERO;
				}
			}
		}
	}
	
#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)lMatrix, MF_MATRIX_DIM * MF_MATRIX_DIM);
#endif

	/* write out the matrix */
	status = Kp_write (fd, (KpGenericPtr_t) lMatrix, sizeof(Fixed_t) * MF_MATRIX_DIM * MF_MATRIX_DIM);
	if (status != 1) {
		goto ErrOutM1;
	}

	/* set up to write standard mft2 tables, which may be modifided by the other cases */
	wrtEntryBytes = sizeof (mf2_tbldat_t);				/* bytes in each table entry */

	wrtiTableMaxValue = MF2_TBL_MAXVAL;
	inputMap = KCP_MAP_END_POINTS;

	wrtgTableMaxValue = MF2_TBL_MAXVAL;

	wrtoTableMaxValue = MF2_TBL_MAXVAL;
	outputMap = KCP_MAP_END_POINTS;

	switch (MFutType) {
		case PTTYPE_MFT1:
			wrtEntryBytes = sizeof (mf1_tbldat_t);
			
			wrtiTableMaxValue = MF1_TBL_MAXVAL;
			if ((fut->itbl[0]->dataClass == KCP_VARIABLE_RANGE) || (fut->itbl[0]->dataClass == KCP_XYZ_PCS)) {
				inputMap = KCP_REF16_TO_BASE_MAX;
			}

			wrtgTableMaxValue = MF1_TBL_MAXVAL;

			wrtoTableMaxValue = MF1_TBL_MAXVAL;
			if ((fut->chan[0]->otbl->dataClass == KCP_VARIABLE_RANGE) || (fut->chan[0]->otbl->dataClass == KCP_XYZ_PCS)) {
				outputMap = KCP_REF16_TO_BASE_MAX;
			}

			break;

		case PTTYPE_MFT2_VER_0:
			if ((fut->itbl[0]->dataClass == KCP_VARIABLE_RANGE) || (fut->itbl[0]->dataClass == KCP_XYZ_PCS)) {
				inputMap = KCP_REF16_TO_BASE_MAX;
			}

			wrtgTableMaxValue = FUT_GRD_MAXVAL;

			if ((fut->chan[0]->otbl->dataClass == KCP_VARIABLE_RANGE) || (fut->chan[0]->otbl->dataClass == KCP_XYZ_PCS)) {
				outputMap = KCP_REF16_TO_BASE_MAX;
				wrtoTableMaxValue = FUT_GRD_MAXVAL;
			}
			else {
				wrtoTableMaxValue = FUT_MAX_PEL12;
			}

		/* no changes for standard mft2
		 * write the additional info for mft2's
		 */
		case PTTYPE_MFT2:
			tmpUI16 = (KpUInt16_t) wrtiTblEntries;

		#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)&tmpUI16, 1);
		#endif
		
			status = Kp_write (fd, (KpGenericPtr_t) &tmpUI16, sizeof (KpUInt16_t));
			if (status != 1) {
				goto ErrOutM1;
			}

			tmpUI16 = (KpUInt16_t) wrtoTblEntries;
		
		#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)&tmpUI16, 1);
		#endif
		
			status = Kp_write (fd, (KpGenericPtr_t) &tmpUI16, sizeof (KpUInt16_t));
			if (status != 1) {
				goto ErrOutM1;
			}
		
			break;

		default:
			goto ErrOutM2;	/* unknown type */
	}

/* input table conversion */
	/* convert each input table to required precision */
	for (i1 = 0; (fut->itbl[i1] != NULL) && (i1 < FUT_NICHAN); i1++) {
		convert1DTable (fut->itbl[i1]->refTbl, sizeof (mf2_tbldat_t), fut->itbl[0]->refTblEntries, MF2_TBL_MAXVAL,
						tmpTbl, wrtEntryBytes, wrtiTblEntries, wrtiTableMaxValue,
						inputMap, KCP_MAP_END_POINTS);

	#if (FUT_MSBF == 0)
		if (MFutType != PTTYPE_MFT1)
			Kp_swab16 ((KpGenericPtr_t)tmpTbl, wrtiTblEntries);
	#endif
		status = Kp_write (fd, (KpGenericPtr_t)tmpTbl, wrtiTblEntries * wrtEntryBytes); /* write the input table */
		if (status != 1) {
			goto ErrOutM1;
		}
	}

/* grid table conversion */
	for (outputChans = 0; outputChans < FUT_NOCHAN; outputChans++) {
		if ((chan = fut->chan[outputChans]) == NULL) {
			break;
		}

		gDataP[outputChans] = chan->gtbl->refTbl;	/* get each grid table pointer */
	}

	totalGSize = gTblEntries * outputChans * wrtEntryBytes;
	outCount = 0;												/* count bytes written to buffer */
	outBytes = calcNextGBufSize (GBUFFER_SIZE, &totalGSize);	/* set up for first write */

	mf1dataP = (KpUInt8_p) tmpTbl;
	mf2dataP = (KpUInt16_p) tmpTbl;
	
	for (i1 = 0; i1 < gTblEntries; i1++) {
		for (i2 = 0; i2 < (KpInt32_t)outputChans; i2++) {
			gData = (KpUInt32_t) *(gDataP[i2])++;	/* get each grid table entry */

			dstData = ((gData * wrtgTableMaxValue) + (MF2_TBL_MAXVAL >> 1)) / MF2_TBL_MAXVAL;	/* scale for output */

			if (MFutType == PTTYPE_MFT1) {
				*mf1dataP++ = (KpUInt8_t)dstData;
			}
			else {
				if (MFutType == PTTYPE_MFT2_VER_0) {
					KpInt32_t   noise;

					noise = (dstData >> 2) & (FUT_BIT(MF2_TBL_BITS - FUT_GRD_BITS) -1);
					gData = (dstData << (MF2_TBL_BITS - FUT_GRD_BITS)) | noise;	/* 12 bit data with psuedo-noise in low 4 bits */
				}
			#if (FUT_MSBF == 0)
				Kp_swab16 ((KpGenericPtr_t)&gData, 1);
			#endif
				*mf2dataP++ = (KpUInt16_t) gData;
			}

			outCount += wrtEntryBytes;				/* count bytes in buffer */

			if (outCount == outBytes) {
				outCount = 0;					/* reset bytes written */
				mf1dataP = (KpUInt8_p) tmpTbl;
				mf2dataP = (KpUInt16_p) tmpTbl;

				status = Kp_write (fd, (KpGenericPtr_t)tmpTbl, outBytes);
				if (status != 1) {
					goto ErrOutM1;
				}

				outBytes = calcNextGBufSize (outBytes, &totalGSize);	/* set up for next time */
			}
		}
	}

/* output table conversion */
	for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
		if ((chan = fut->chan[i1]) == NULL) {
			break;										/* all done */
		}

		convert1DTable (fut->chan[i1]->otbl->refTbl, sizeof (mf2_tbldat_t), fut->chan[i1]->otbl->refTblEntries, MF2_TBL_MAXVAL,
						tmpTbl, wrtEntryBytes, wrtoTblEntries, wrtoTableMaxValue,
						KCP_MAP_END_POINTS, outputMap);

		if (MFutType == PTTYPE_MFT2_VER_0) {
			mf2dataP = (KpUInt16_p) tmpTbl;

			for (i2 = 0; i2 < wrtoTblEntries; i2++) {
				KpUInt16_t   noise, srcData;

				srcData = mf2dataP[i2];

				noise = (srcData >> 2) & (FUT_BIT(MF2_TBL_BITS - FUT_GRD_BITS) -1);
				tmpUI16 = (srcData << (MF2_TBL_BITS - FUT_GRD_BITS)) | noise;	/* 12 bit data with psuedo-noise in low 4 bits */
				mf2dataP[i2] = tmpUI16;
 			}
		}

		#if (FUT_MSBF == 0)
		if (MFutType != PTTYPE_MFT1) {
			Kp_swab16 ((KpGenericPtr_t)tmpTbl, wrtoTblEntries);
		}
		#endif

		status = Kp_write (fd, (KpGenericPtr_t)tmpTbl, wrtoTblEntries * wrtEntryBytes);
		if (status != 1) {
			goto ErrOutM1;
		}
	}

GetOut:
	return status;


ErrOutM2:
	status = -2;
	goto GetOut;

ErrOutM1:
	status = -1;
	goto GetOut;
}


static KpInt32_t
	calcNextGBufSize ( KpInt32_t curBufSize, KpInt32_p totalBytesRemaining)
{
KpInt32_t	bytesToWrite;

	*totalBytesRemaining -= curBufSize;		/* bytes to write after this buffer is written */
	if (*totalBytesRemaining <= 0) {
		bytesToWrite = *totalBytesRemaining + curBufSize;	/* last buffer to write, adjust size */
	}
	else {
		bytesToWrite = curBufSize;			/* not last buffer to write */
	}

	return bytesToWrite;
}


/* fut_mfutInfo returns matrix fut information of a fut.
 *
 * Returns: 
 *	1	on success
 *	0	invalid fut
 * -1	grid dimensions too large
 * -2	grid dimensions are not the same
 * -3	input channels not contiguous and starting at 0 or too many inputs
 * -4	output channels not contiguous and starting at 0 or too many outputs
 */
KpInt32_t
	fut_mfutInfo (	fut_p		fut,				/* get info of this fut */
					KpInt32_p	LUTDimensionsP,		/* # points in each grid dimension */
					KpInt32_p	inputChansP,		/* # of input channels */
					KpInt32_p	outputChansP,		/* # of input channels */
					KpInt32_t	MFutType,			/* type of intended table output */
					KpInt32_p	inputEntriesP,		/* # of input table entries */
					KpInt32_p	gridEntriesP,		/* # of grid table entries */
					KpInt32_p	outputEntriesP)		/* # of output table entries */
{
KpInt32_t	LUTDimensions, inputChans, outputChans, iTblEntries, gTblEntries, oTblEntries;
KpInt32_t	imask, omask, thisGridDim;
KpInt32_t	status = 1;								/* assume success */

	if ( ! IS_FUT(fut)  || ! IS_ITBL(fut->itbl[0])) {
		return (0);
	}

/* get the # of input channels, # of output channels, and the grid dimensions */

	/* input tables must be common and in first n contiguous input channels */
	imask = fut->iomask.in;							/* get the fut's input mask */
	LUTDimensions = fut->itbl[0]->size;				/* initialize the size */

	if (LUTDimensions > MF_GRD_MAXDIM) {
		status = -1;								/* this fut can not be made into a matrix fut */
	}
	
	for (inputChans = 0; inputChans < FUT_NICHAN; inputChans++, imask >>= 1) {
		if ( ! IS_ITBL(fut->itbl[inputChans]) || ((imask & 1) == 0)) {
			break;
		}
		
		thisGridDim = fut->itbl[inputChans]->size;

		if (LUTDimensions != thisGridDim) {			/* sizes must be the same */
			if (LUTDimensions < thisGridDim) {		/* always return largest size */
				LUTDimensions = thisGridDim;
			}

			if (status == 1) {
				status = -2;						/* this fut can not be made into a matrix fut */
			}
		}
	}

	if (imask != 0) {		/* input channels not contiguous */
		if (status == 1) {
			status = -3;	/* this fut can not be made into a matrix fut */
		}
	}
	
	/* output tables must be in first n contiguous output channels */
	omask = fut->iomask.out;				/* get the fut's output mask */
	outputChans = 0;

	for (outputChans = 0; outputChans < FUT_NOCHAN; outputChans++, omask >>= 1) {
		if ( ! IS_CHAN(fut->chan[outputChans]) || ((omask & 1) == 0)) {
			break;
		}
	}

	if (omask != 0) {
		if (status == 1) {
			status = -4;	/* this fut can not be made into a matrix fut */
		}
	}

	/* get # entries in input, grid, and output tables */
	iTblEntries = 0;	/* in case type not specified */
	gTblEntries = 0;
	oTblEntries = 0;

	if (( ! IS_GTBL(fut->chan[0]->gtbl)) || ( ! IS_OTBL(fut->chan[0]->otbl))) {
		status = -4;		/* this fut can not be made into a matrix fut */
	}
	else {
		gTblEntries = fut->chan[0]->gtbl->tbl_size / sizeof (fut_gtbldat_t);

		switch (MFutType) {
		case PTTYPE_MFT1:
			iTblEntries = MF1_TBL_ENT;
			oTblEntries = MF1_TBL_ENT;
			break;

		case PTTYPE_MFT2:
			iTblEntries = fut->itbl[0]->refTblEntries;
			oTblEntries = fut->chan[0]->otbl->refTblEntries;
			break;

		case PTTYPE_MFT2_VER_0:
			if (fut->itbl[0]->refTblEntries == MF2_STD_ITBL_SIZE) {
				iTblEntries = KCP_V0_ITABLE_SIZE;			/* return original size size */
			}
			else {
				iTblEntries = fut->itbl[0]->refTblEntries;	/* retain size */
			}
			oTblEntries = KCP_V0_OTABLE_SIZE;
			break;
		
		default:
			break;
		}
	}

	*LUTDimensionsP = LUTDimensions;				/* return info */
	*inputChansP = inputChans;
	*outputChansP = outputChans;
	*inputEntriesP = iTblEntries;
	*gridEntriesP = gTblEntries;
	*outputEntriesP = oTblEntries;

	return (status);
}


/* return 1 if the matrix is an identity matrix, 0 if not */
KpInt32_t
	isIdentityMatrix (	Fixed_p		matrix,
						KpInt32_t	matrixSize)
{
KpInt32_t	i1, i2, i3;

	for (i1 = 0, i3 = 0; i1 < matrixSize; i1++) {
		for (i2 = 0; i2 < matrixSize; i2++, i3++) {
			if (i1 == i2) {
				if (matrix[i3] != FUT_MATRIX_ONE) {
					return (0);
				}
			}
			else {
				if (matrix[i3] != FUT_MATRIX_ZERO) {
					return (0);
				}
			}
		}
	}

	return (1);
}


/* Convert a table to a table with differing size and/or precision
 * The modes using REF8 and REF16 assume that "END_POINTS" tables
 * are referenced to 8 and 16 bit ICC PCS definitions, respectively.
 */
KpInt32_t
	convert1DTable (KpGenericPtr_t srcTable, KpInt32_t srcBytes, KpInt32_t srcEntries, KpUInt32_t srcMaxValue,
					KpGenericPtr_t destTable, KpInt32_t destBytes, KpInt32_t destEntries, KpUInt32_t destMaxValue,
					PTDataMap_t inputMode, PTDataMap_t outputMode)
{
KpInt32_t	dIndex, sTableIndex, sTableIndexNext;
KpUInt32_t	intDestData, copyIndex, copyValue;
KpDouble64_t	sIndex, sTableFrac, srcData, srcData1, destData;
KpDouble64_t indexRatio, valueRatio;

	indexRatio = getIndexRatio (inputMode, srcEntries, destEntries);
	
	if (fabs(indexRatio - 1.0) < .00001) {
		copyIndex = 1;	/* just copy */
	}
	else {
		copyIndex = 0;
	}

	valueRatio = getValueRatio (outputMode, srcMaxValue, destMaxValue);
	
	if (fabs(valueRatio - 1.0) < .00001) {
		copyValue = 1;	/* just copy */
	}
	else {
		copyValue = 0;
	}
	
	for (dIndex = 0; dIndex < destEntries; dIndex++) {
		
		if (copyIndex == 1) {
			sTableIndex = dIndex;							/* source table position */
			sTableIndexNext = sTableIndex;
			sTableFrac = (KpDouble64_t)0;					/* Set for compiler - not used in this path */
		}
		else {
			sIndex = (KpDouble64_t) dIndex * indexRatio;			/* calculate the source table position */
			sTableIndex = (KpInt32_t) sIndex;					/* the source index */
			sTableFrac = sIndex - (KpDouble64_t) sTableIndex;	/* and the source interpolant */

			if (sTableIndex >= srcEntries) {	/* make sure we're in range for interpolation */
				sTableIndex = srcEntries -1;	/* 1st source is past end */
				sTableIndexNext = sTableIndex;
			}
			else {
				sTableIndexNext = sTableIndex +1;

				if (sTableIndexNext == srcEntries) {
					sTableIndexNext = sTableIndex;	/* 1st source is at end */
				}
			}
		}

		switch (srcBytes) {			/* get the table values to interpolate */
		case 1:
			srcData = (KpDouble64_t) ((KpUInt8_p) srcTable) [sTableIndex];
			srcData1 = (KpDouble64_t) ((KpUInt8_p) srcTable) [sTableIndexNext];
			break;
		
		case 2:
			srcData = (KpDouble64_t) ((KpUInt16_p) srcTable) [sTableIndex];
			srcData1 = (KpDouble64_t) ((KpUInt16_p) srcTable) [sTableIndexNext];
			break;
		
		case 4:
			srcData = (KpDouble64_t) ((KpUInt32_p) srcTable) [sTableIndex];
			srcData1 = (KpDouble64_t) ((KpUInt32_p) srcTable) [sTableIndexNext];
			break;
		
		default:
			srcData = (KpDouble64_t)0;
			srcData1 = (KpDouble64_t)0;
			break;
		}

		if (copyIndex != 1) {
			srcData += (sTableFrac * (srcData1 - srcData));	/* interpolate */
		}

		if (copyValue == 1) {
			intDestData = (KpUInt32_t)(srcData  + 0.5);
		}
		else {
			destData = srcData * valueRatio;					/* convert for output */

			intDestData = (KpUInt32_t)(destData + 0.5);			/* round and convert to integer */
		}

		if (intDestData > destMaxValue) {
			intDestData = destMaxValue;
		}

		switch (destBytes) {		/* store each entry */
		case 1:
			((KpUInt8_p) destTable) [dIndex] = (KpUInt8_t)intDestData;
			break;
		
		case 2:
			((KpUInt16_p) destTable) [dIndex] = (KpUInt16_t)intDestData;
			break;
		
		case 4:
			((KpUInt32_p) destTable) [dIndex] = intDestData;
			break;
		
		default:
			((KpUInt32_p) destTable) [dIndex] = 0;
			break;
		}
	}
	
	return 0;
}


static KpDouble64_t
	getIndexRatio (	PTDataMap_t	inputMode,
					KpInt32_t	srcEntries,
					KpInt32_t	destEntries)
{	
KpDouble64_t sStepFloat, dStepFloat;
KpDouble64_t indexRatio;

	switch (inputMode) {
	case KCP_MAP_END_POINTS:
		sStepFloat = (KpDouble64_t) (srcEntries -1);
		dStepFloat = (KpDouble64_t) (destEntries -1);
		break;
	
	case KCP_REF16_TO_BASE_MAX:
		sStepFloat = (KpDouble64_t) (srcEntries -1)  * 65536;
		dStepFloat = (KpDouble64_t) destEntries * 65535;
		break;
	
	case KCP_BASE_MAX_TO_REF16:
		sStepFloat = (KpDouble64_t) srcEntries * 65535;
		dStepFloat = (KpDouble64_t) (destEntries -1) * 65536;
		break;
	
	default:
		sStepFloat = 0.0;
		dStepFloat = 1.0;
		break;
	}

	indexRatio = sStepFloat / dStepFloat;
	
	return (indexRatio);
}	

	
static KpDouble64_t
	getValueRatio (	PTDataMap_t	outputMode,
					KpInt32_t	srcMaxValue,
					KpInt32_t	destMaxValue)
{
KpDouble64_t valueRatio, srcTempMaxValue, destTempMaxValue;

	switch (outputMode) {
	case KCP_MAP_END_POINTS:
		srcTempMaxValue = (KpDouble64_t) srcMaxValue;
		destTempMaxValue = (KpDouble64_t) destMaxValue;
		break;
	
	case KCP_REF16_TO_BASE_MAX:
		srcTempMaxValue = (KpDouble64_t) srcMaxValue *  65536;
		destTempMaxValue = (KpDouble64_t) (destMaxValue + 1) * 65535;
		break;
	
	case KCP_BASE_MAX_TO_REF16:
		srcTempMaxValue = (KpDouble64_t) (srcMaxValue + 1) * 65535;
		destTempMaxValue = (KpDouble64_t) destMaxValue * 65536;
		break;
	
	default:
		srcTempMaxValue = 1.0;
		destTempMaxValue = 0.0;
		break;
	}

	valueRatio = destTempMaxValue / srcTempMaxValue;
	
	return (valueRatio);
}


/* Check the the given mft2 to see if it was generated by a version 0 CP.
 * This is the case when every output table entry has bits 9:6 equal to bits 3:0.
 */

static mft2Type_t
	checkCPv0Gen (mft2_p	theMft2)
{
mft2Type_t		status;
KpInt32_t		i1, i2, bits9to6, bits3to0;
mf2_tbldat_p	theOTbl;
mf2_tbldat_t	theOTblData;

	if ((theMft2->iTblEntries != KCP_V0_ITABLE_SIZE) || (theMft2->oTblEntries != KCP_V0_OTABLE_SIZE)) {
		goto notCPv0Gen;
	}

	for (i1 = 0; i1 < theMft2->nOChan; i1++) {	/* check each entry of each output table */
		theOTbl = theMft2->outputTable [i1];

		for (i2 = 0; i2 < KCP_V0_OTABLE_SIZE; i2++) {
			theOTblData = theOTbl[i2];
			bits9to6 = (KpInt32_t)(theOTblData & KCP_9TO6_MASK) >> 6;
			bits3to0 = theOTblData & KCP_3TO0_MASK;
			
			if (bits9to6 != bits3to0) {		/* do they match? */
				goto notCPv0Gen;			/* this was not generated by CP version 0 */
			}
		}		
	}

	status = KCP_V0_TYPE;

	return status;


notCPv0Gen:
	status = KCP_UNKNOWN_TYPE;

	return status;
}
