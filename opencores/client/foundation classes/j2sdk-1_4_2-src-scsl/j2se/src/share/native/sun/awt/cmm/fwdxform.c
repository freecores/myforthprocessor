/*
 * @(#)fwdxform.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)fwdxform.c	1.26 99/01/08

	Contains:	makeForwardXformFromMatrix

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-1999 Eastman Kodak Company
	As  an unpublished  work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include <math.h>
#include <string.h>
#include "makefuts.h"


/*---------------------------------------------------------------------------
 *  makeForwardXformFromMatrix -- make a fut of given gridsize from given matrix data
 *		for forward transform (RGB -> XYZ); return status code
 *---------------------------------------------------------------------------
 */
PTErr_t
	makeForwardXformFromMatrix (LPMATRIXDATA	mdata,
								KpUInt32_t		interpMode,
								KpInt32_p		dim,
								fut_p			theFut)
{
PTErr_t			PTErr;
ResponseRecord_p	rrp;
KpInt32_t		futReturn, i;
fut_itbldat_p	itblDat, prevItblDat;
fut_gtbldat_p	gtblDat[3];
KpUInt16_t		prevGamma = 0, thisGamma;
double			fwdgamma;

	for (i = 0; i < 3; i++) {
		 /* Get input table:  */
		futReturn = fut_get_itbl (theFut, (KpInt32_t)-1, i, &itblDat);
		if ((futReturn != 1) || (itblDat == (fut_itbldat_p)NULL)) {
		   return KCP_SYSERR_0;
		}

		 /* Get ResponseRecord:  */
		rrp = mdata->response[i];
		if ((rrp->count > 0) && (rrp->data == (KpUInt16_p)NULL)) {
		   return KCP_INCON_PT;
		}

		 /* Recompute input table:  */
		switch (rrp->count) {
		case 0:	/* linear response */
			/* no-op:  leave ramps alone */
			break;

		case 1:	/* power law */
			thisGamma = rrp->data[0];
			if (prevGamma == thisGamma) {	/* same gamma, just copy table */
				memcpy (itblDat, prevItblDat, sizeof (*itblDat) * FUT_INPTBL_ENT);
			}
			else {					
				prevGamma = thisGamma;
				prevItblDat = itblDat;

				fwdgamma = (double)thisGamma / SCALEDOT8;
				if (fwdgamma <= 0.0) {
					return KCP_INCON_PT;
				}
				calcItbl1 (itblDat, dim[i], fwdgamma);
			}
			break;

		case 256:	/* ready-to-use look-up table */
			makeMonotonic (rrp->count, rrp->data);
			calcItbl256 (itblDat, dim[i], rrp->data);
			break;

		default:	/* transfer table of arbitrary length */
			makeMonotonic (rrp->count, rrp->data);
			PTErr = calcItblN (itblDat, dim[i], rrp, interpMode);
			if (PTErr != KCP_SUCCESS) {
				return KCP_INCON_PT;
			}
			break;
		}
	
		/* Set the last entry to the value of the previous one.  This will perform
		automatic clipping of input greater than 4080 to the valid gridspace,
		which is defined only for input in the range [0, 255] or [0 << 4, 255 << 4]:  */
		itblDat[FUT_INPTBL_ENT] = itblDat[FUT_INPTBL_ENT -1];
	}

    /* Replace grid tables:  */
	for (i = 0; i < 3; i++) {
		futReturn = fut_get_gtbl (theFut, i, &gtblDat[i]);
		if ((futReturn != 1) || (gtblDat[i] == (fut_gtbldat_p)NULL)) {
			return KCP_INCON_PT;
		}
	}

	calcGtbl3 (gtblDat, dim, mdata->matrix, 0.0);	/* without offset */

	return KCP_SUCCESS;
}


/*-------------------------------------------------------------------------------
 *  makeMonotonic -- flatten reversals in data table
 *-------------------------------------------------------------------------------
 */
void
	makeMonotonic (KpUInt32_t count, KpUInt16_p table)
{
KpInt32_t	i;
KpUInt16_t	val;

     /* Check inputs:  */
	if ((table == (KpUInt16_p)NULL) || (count < 3)) {
		return;
	}

     /* Flatten from high end to low end, depending on polarity:  */
	if (table[0] <= table[count - 1]) {	/* globally non-decreasing */
		val = table[count - 1];
		for (i = count - 2; i >= 0; i--) {	/* from right to left */
			if (table[i] > val)	{	/* reversal? */
				table[i] = val;		/* flatten! */
			}
			else {					/* no reversal? */
				val = table[i];		/* update */
			}
		}
	}
	else {						/* globally decreasing */
		val = table[0];
		for (i = 1; i < (KpInt32_t)count; i++) {		/* from left to right */
			if (table[i] > val)	{	/* reversal? */
				table[i] = val;		/* flatten! */
			}
			else {					/* no reversal? */
				val = table[i];		/* update */
			}
		}
	}
}

