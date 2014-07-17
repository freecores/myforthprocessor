/*
 * @(#)invxform.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)invxform.c	1.24 98/11/04

	Contains:	makeInverseXformFromMatrix

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-1998 Eastman Kodak Company
	As  an unpublished  work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include <math.h>
#include <string.h>
#include "makefuts.h"


/*---------------------------------------------------------------------------
 *  makeInverseXformFromMatrix -- make a fut of given gridsize from given
 *	matrix data for inverse transform (XYZ -> RGB); return status code
 *---------------------------------------------------------------------------
 */
PTErr_t
	makeInverseXformFromMatrix (LPMATRIXDATA	mdata,
								KpUInt32_t		interpMode,
								KpInt32_p		dim,
								fut_p			theFut)
{
PTErr_t			PTErr;
ResponseRecord_p	rrp;
KpInt32_t		futReturn, i;
fut_gtbldat_p	gtblDat[3];
fut_otbldat_p	otblDat, prevOtblDat;
KpUInt16_t		prevGamma = 0, thisGamma;
double			fwdgamma, one[3];

	for (i = 0; i < 3; i++) {
		 /* Get output table:  */
		futReturn = fut_get_otbl (theFut, i, &otblDat);
		if ((futReturn != 1) || (otblDat == (fut_otbldat_p)NULL)) {
			return KCP_SYSERR_0;
		}

		 /* Get ResponseRecord:  */
		rrp = mdata->response[i];
		if ((rrp->count > 0) && (rrp->data == (KpUInt16_p)NULL)) {
			return KCP_INCON_PT;
		}

		 /* Recompute output table:  */
		switch (rrp->count) {
		case 0:	/* linear response, with clipping */
			calcOtbl0 (otblDat);
			break;
			
		case 1:	/* power law */
			thisGamma = rrp->data[0];
			if (prevGamma == thisGamma) {	/* same gamma, just copy table */
				memcpy (otblDat, prevOtblDat, sizeof (*otblDat) * FUT_OUTTBL_ENT);
			}
			else {					
				prevGamma = thisGamma;
				prevOtblDat = otblDat;

				fwdgamma = (double)thisGamma / SCALEDOT8;
				if (fwdgamma <= 0.0) {
					return KCP_INCON_PT;
				}
				calcOtbl1 (otblDat, fwdgamma);
			}
			break;
			
		default:	/* look-up table of arbitrary length */
			makeInverseMonotonic (rrp->count, rrp->data);
			PTErr = calcOtblN (otblDat, rrp, interpMode);
			if (PTErr != KCP_SUCCESS) {
				return KCP_INCON_PT;
			}
			break;
		}
	}

	/* Compute inverse matrix (XYZ -> RGB):  */
	one[0] = one[1] = one[2] = 1.0;			/* arbitrary vector */

	 /* replaces matrix with inverse */
	if (solvemat (3, mdata->matrix, one) != 0) {
		return KCP_INCON_PT;				/* singular matrix passed in */
	}

	/* Rescale given matrix by factor of 3 for extended range:  */
	for (i = 0; i < 3; i++) {
		KpInt32_t	j;

		for (j = 0; j < 3; j++) {
			mdata->matrix[i][j] /= 3.0;
		}
	}

     /* Replace grid tables:  */
	for (i = 0; i < 3; i++) {
		futReturn = fut_get_gtbl (theFut, i, &gtblDat[i]);
		if ((futReturn != 1) || (gtblDat[i] == (fut_gtbldat_p)NULL)) {
			return KCP_INCON_PT;
		}
	}

	calcGtbl3 (gtblDat, dim, mdata->matrix, 1.0 / 3.0);	/* with offset */

	return KCP_SUCCESS;
}


/*-------------------------------------------------------------------------------
 *  makeInverseMonotonic -- flatten reversals in data table
 *-------------------------------------------------------------------------------
 */
void
	makeInverseMonotonic (KpUInt32_t count, KpUInt16_p table)
{
KpInt32_t	i;
KpUInt16_t	val;

     /* Check inputs:  */
	if ((table == (KpUInt16_p)NULL) || (count < 3)) {
		return;
	}

     /* Flatten from high end to low end, depending on polarity:  */
	if (table[0] <= table[count - 1]) {		/* globally non-decreasing */
		val = table[count - 1];
		for (i = count - 2; i >= 0; i--) {	/* from right to left */
			if (table[i] > val) {			/* reversal? */
				table[i] = val;				/* flatten! */
			}
			else {							/* no reversal? */
				val = table[i];				/* update */
			}
		}
	}
	else {									/* globally decreasing */
		val = table[0];
		for (i = 1; i < (KpInt32_t)count; i++)	{		/* from left to right */
			if (table[i] > val)	{			/* reversal? */
				table[i] = val;				/* flatten! */
			}
			else {							/* no reversal? */
				val = table[i];				/* update */
			}
		}
	}
}

