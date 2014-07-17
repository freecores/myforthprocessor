/*
 * @(#)calcxtbl.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)calcxtbl.c	1.8 98/06/11

	Contains:	base on the FPU flag, chooses which routine to call
			at runtime. 

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "kcms_sys.h"

#if defined (KPMAC68K)
#include <Gestalt.h>
#else
#define gestaltNoFPU 0
#endif

#include <math.h>
#include "kcmptdef.h"
#include "kcmptlib.h"
#include "kcptmgr.h"
#include "fut.h"
#include "csmatrix.h"

static	KpInt32_t	kcpFPUType = gestaltNoFPU;

#if defined (KPMAC68K)
/************************************************************************/
/* For the macintosh find out if there is a floating point processor.	*/
/*	for everybody else there is not.									*/
/************************************************************************/
void
kcpGetFPU ()
{
	OSErr	myErr;
	
	myErr = Gestalt(gestaltFPUType, &kcpFPUType);
}
#endif

/************************************************************/
/*	Return gestalt status for whether or not FPU is present */
/************************************************************/
KpInt32_t
	kcpIsFPUpresent ()
{
	return (kcpFPUType);
}

#if !defined (KPMAC68K)
/************************************************************/
/* If this is not a KPMAC68K these functions do not exhist. */
/************************************************************/

PTErr_t
	makeOutputMatrixXformFPU (	KpF15d16_t FAR*	matrix,
							KpUInt32_t			gridsize,
							fut_p FAR*	theFut)
{
	if (matrix) {}
	if (gridsize) {}
	if (theFut) {}
	
	return (KCP_SYSERR_2);
}

PTErr_t makeProfileXformFPU (FixedXYZColor_p rXYZ,
							FixedXYZColor_p gXYZ,
							FixedXYZColor_p bXYZ, 
							ResponseRecord_p rTRC,
							ResponseRecord_p gTRC,
							ResponseRecord_p bTRC, 
							KpUInt32_t gridsize,
							KpBool_t invert,
							newMGmode_p newMGmodeP,
							fut_p FAR* theFut)
{
	if (rXYZ) {}
	if (gXYZ) {}
	if (bXYZ) {}
	if (rTRC) {}
	if (gTRC) {}
	if (bTRC) {}
	if (gridsize) {}
	if (invert) {}
	if (newMGmodeP) {}
	if (theFut) {}
	
	return (KCP_SYSERR_2);
}

PTErr_t makeForwardXformFromMatrixFPU (LPMATRIXDATA	mdata,
									KpUInt32_t			gridsize,
									KpUInt32_t		interpMode,
									fut_p *		theFut)
{
	if (mdata) {}
	if (gridsize) {}
	if (interpMode) {}
	if (theFut) {}
	
	return (KCP_SYSERR_2);
}

PTErr_t makeInverseXformFromMatrixFPU (LPMATRIXDATA	mdata,
									KpUInt32_t 		gridsize,
									KpUInt32_t		interpMode,
									fut_p *		theFut)
{
	if (mdata) {}
	if (gridsize) {}
	if (interpMode) {}
	if (theFut) {}
	
	return (KCP_SYSERR_2);
}

void calcGtbl3FPU (fut_gtbldat_p *table, KpInt32_t *gridSizes, 
					double **rows, KpBool_t xrange)
{
	if (table) {}
	if (gridSizes) {}
	if (rows) {}
	if (xrange) {}
}

void calcItbl1FPU (fut_itbldat_p table, KpInt32_t gridSize, double gamma)
{
	if (table) {}
	if (gridSize) {}
	if (gamma) {}
}

void calcItbl256FPU (fut_itbldat_p table, KpInt32_t gridSize, unsigned short lut[])
{
	if (table) {}
	if (gridSize) {}
	if (lut) {}
}

PTErr_t calcItblNFPU (fut_itbldat_p table, KpInt32_t gridSize, ResponseRecord* rrp, KpUInt32_t interpMode)
{
	if (table) {}
	if (gridSize) {}
	if (rrp) {}
	if (interpMode) {}
	return (KCP_SUCCESS);
}

void calcOtbl0FPU (fut_otbldat_p table)
{
	if (table) {}
}

void calcOtbl1FPU (fut_otbldat_p table, double fwdgamma)
{
	if (table) {}
	if (fwdgamma) {}
}

PTErr_t calcOtblNFPU (fut_otbldat_p table, ResponseRecord* rrp, KpUInt32_t interpMode)
{
	if (table) {}
	if (rrp) {}
	if (interpMode) {}
	return (KCP_SUCCESS);
}

double calcInvertTRCFPU (double p, KpUInt16_p data, KpUInt32_t length)
{
	if (p) {}
	if (data) {}
	if (length) {}
	return (0);
}

#endif

