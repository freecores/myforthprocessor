/*
 * @(#)calcitbl.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)calcitbl.c	1.25 98/12/03

	Contains:	calcItbl1, calcItbl256, calcItblN

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

/*
 *	General definitions
 */

#include <math.h>
#include "makefuts.h"

/*----------------------------------------------------------------------------
 *  calcItbl1, calcItbl256, calcItblN -- calculate an input table
 *		to linearize RGB inputs according to specified algorithm (power law, 
 *		256-entry LUT, or transfer table)
 *----------------------------------------------------------------------------
 */

PTErr_t
	PTCreateTRC (KpUInt16_p TRC, KpFloat32_t gamma)
{
	KpInt32_t	count;
	double		val;
	double		x;
	double		nument_inv;

	if (TRC == NULL)
		return KCP_BAD_PTR;
	if (gamma == (double)1.0)
		return KCP_BAD_ARG;

	nument_inv = 1.0 / (double)(FUT_INPTBL_ENT - 1);

	for (count = 0; count < FUT_INPTBL_ENT; count++) {
		x = (double)count * nument_inv;		/* in [0, 1] */

		/* Compute linearized value by power law:  */
		val = (double)pow (x, gamma);		/* in [0, 1] */
		val = MAX (x/SLOPE_LIMIT, val);		/* limit the slope */
		*TRC = (KpUInt16_t)(val * 65535);
		TRC++;
	}
	return KCP_SUCCESS;
}


/*---------------------  calcItbl1  --------------------------------*/
void
	calcItbl1 (fut_itbldat_p table, KpInt32_t gridSize, double gamma)
{
KpUInt16_t	TRC[FUT_INPTBL_ENT];
PTErr_t		PTError;

     /* Check input parameters and initialize:  */
	if (table == (fut_itbldat_p)NULL)
	   return;				/* just don't crash */
	if (gamma == (double)1.0)
	   return;				/* trivial case */

	PTError = PTCreateTRC(TRC, (KpFloat32_t)gamma);
	if (PTError != KCP_SUCCESS)
		return;

	calcItbl256(table, gridSize, TRC);
}


/*---------------------  calcItbl256  --------------------------------*/
void calcItbl256 (fut_itbldat_p table, KpInt32_t gridSize, KpUInt16_t lut[])
{
KpInt32_t		count;
double			val, norm;
fut_itbldat_t	lval, lmax;

     /* Check input and initialize:  */
	if (table == (fut_itbldat_p)NULL)	return;				/* just don't crash! */
	if (lut == (KpUInt16_p)NULL)	return;

	norm = (double)((gridSize - 1) << FUT_INP_DECIMAL_PT);
	lmax = ((fut_itbldat_t) norm) -1;

     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < FUT_INPTBL_ENT; count++) {	/* in [0, 255] */
	     /* Compute linearized value by table look-up:  */
		val = (double)lut[count] / SCALEDOT16;		/* in [0, 1] */

	     /* Rescale, clip, and quantize for input table:  */
		val = RESTRICT (val, 0.0, 1.0);		/* just to be sure */
		lval = (fut_itbldat_t) ((val * norm) + 0.5);
		if (lval > lmax) {
			lval = lmax;
		}

		table[count] = lval;
	}
}


/*---------------------  calcItblN  --------------------------------*/
/*
fut_itbldat_p	table;		pointer to input-table data
KpUInt32_t			gridSize;	dimension of grid table to be addressed
KpUInt32_t			length;		length of data table
KpUInt16_p			data;		table of linearization data
KpUInt32_t 			interpMode;	interpolation mode to use
*/

PTErr_t
	calcItblN (fut_itbldat_p table, KpInt32_t gridSize, ResponseRecord_p rrp, KpUInt32_t interpMode)
{
PTErr_t			PTErr;
KpUInt32_t		length;
KpUInt16_p		data;
KpUInt32_t		count, ix;
double			val, norm;
fut_itbldat_t	lval, lmax;
double			x, x1;
double			frac, nument_inv_li, nument_inv_l4;
KpInt32_t		hint = 1;
xfer_t			Xfer;
KpBool_t		decreasing;

	/* Check input and initialize:  */
	if (rrp == NULL) {
		return KCP_BAD_ARG;
	}
	
	length = rrp->count;
	data = rrp->data;

	if (data[0] > data[length-1]) {
		decreasing = KPTRUE;
	} else {
		decreasing = KPFALSE;
	}

	if (table == NULL)	return KCP_BAD_ARG;
	if (gridSize < 2)	return KCP_BAD_ARG;
	if (gridSize > FUT_GRD_MAXDIM)	return KCP_BAD_ARG;
	if (length == 0)	return KCP_BAD_ARG;
	if (data == NULL)	return KCP_BAD_ARG;
	   
	PTErr = init_xfer (&Xfer, rrp);
	if (PTErr != KCP_SUCCESS) {
	   return KCP_BAD_ARG;
	}
	
	PTErr = set_xfer (&Xfer, 0, 1);
	if (PTErr != KCP_SUCCESS) {
	   return KCP_BAD_ARG;
	}

	nument_inv_li = (double)(length - 1) / (double)(FUT_INPTBL_ENT - 1);
	nument_inv_l4 = 1.0 / (double)(FUT_INPTBL_ENT - 1);
	
	norm = (double)((gridSize - 1) << FUT_INP_DECIMAL_PT);
	lmax = ((fut_itbldat_t) norm) -1;

     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < FUT_INPTBL_ENT; count++) {
		
		/* Compute linearized value by interpolating in transfer table:  */
		x1 = (double)count * nument_inv_l4;	/* in [0, 1] */

		switch (interpMode) {
		case KCP_TRC_LINEAR_INTERP:

	 	    /* Compute linearized value by interpolating in transfer table:  */
			x = (double)count * nument_inv_li;		/* in [0, length - 1] */
			ix = (KpUInt32_t)x;						/* integer part */
			if (ix >= (length - 1)) {				/* off end of data table */
				val = (double)data[length - 1] / SCALEDOT16;	/* take last value */
			}
			else {								/* within data table, interpolate */
				frac = x - (double)ix;			/* fractional part */
				val = (double)data[ix] + frac * ((double)data[ix + 1] - (double)data[ix]);
				val /= SCALEDOT16;
			}						/* in [0, 1] */

			break;
		
		case KCP_TRC_LAGRANGE4_INTERP:
	
/* !!!!  LAGRANGIAN NOT WORK IF THE SOURCE DATA IS NON-MONOTONIC OR HAS FLAT REGIONS !!!! */
			val = xfer (&Xfer, x1, &hint);		/* in [0, 1] */

			break;
		
		default:
			return KCP_BAD_ARG;
		}

		if (length < SLOPE_COUNT) {
			if (decreasing == KPTRUE) {
				val = MIN (val, (1-x1)*SLOPE_LIMIT);	/* limit the decreasing slope */
			} else {
				val = MAX (x1/SLOPE_LIMIT, val);		/* limit the increasing slope */
			}
		}
	     /* Rescale, clip, and quantize for input table:  */
		val = RESTRICT (val, 0.0, 1.0);		/* just to be sure */
		lval = (fut_itbldat_t) ((val * norm) + 0.5);
		if (lval > lmax) {
			lval = lmax;
		}

		table[count] = lval;
	}
	
	return KCP_SUCCESS;
}
