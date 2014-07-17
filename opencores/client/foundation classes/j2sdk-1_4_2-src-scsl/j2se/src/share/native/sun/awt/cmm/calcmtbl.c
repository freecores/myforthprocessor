/*
 * @(#)calcmtbl.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)calcmtbl.c	1.13 98/12/03

	Contains:	sets up the ouput tables for monochrome	transforms

	Written by:	Color Processor group

	COPYRIGHT (c) 1997-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "kcms_sys.h"

#include <math.h>
#include "makefuts.h"

/*---------------------------------------------------------------------------
 *  calcOtblLSN -- calculate an output table by doing a device to L(TRC) & L to
 					L*  conversion.
 *---------------------------------------------------------------------------
 */

PTErr_t calcOtblLSN (fut_otbldat_p table, ResponseRecord_p rrp)
{
KpUInt32_t	length;
KpUInt16_p	data;
KpUInt32_t	count, ix;
double		val;
double		x;
double		frac, nument_inv_li;
KpBool_t	decreasing;

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

	if ((table == NULL) ||
		(length == 0) ||			/* bad size */
		(data == NULL)) {			/* no transfer table */
			return KCP_BAD_ARG;
	}

	nument_inv_li = (double)(length - 1) / (double)(FUT_OUTTBL_ENT - 1);
	
     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < FUT_OUTTBL_ENT; count++) {
		
	 	/* Compute linearized value by interpolating in transfer table:  */
		x = (double)count * nument_inv_li;		/* in [0, length - 1] */
		ix = (KpUInt32_t)x;						/* integer part */
		if (ix >= (length - 1)) {				/* off end of data table */
			val = (double)data[length - 1];	/* take last value */
		}
		else {								/* within data table, interpolate */
			frac = x - (double)ix;			/* fractional part */
			val = (double)data[ix] + frac * ((double)data[ix + 1] - (double)data[ix]);
		}						/* in [0, 1] */

		/* scale and calculate L to L* */
		val /= RRECORD_DATA_SIZE - 1;
		val = H (val);
	
		if (length < SLOPE_COUNT) {
			if (decreasing == KPTRUE) {
				val = MAX (val, (1-x)/SLOPE_LIMIT);			/* limit the decreasing slope */
			} else {
				val = MIN (val, x*SLOPE_LIMIT);				/* limit the increasing slope */
			}
		}

		/* Rescale to 4080 (= 255 << 4):  */
		*(table++) = (fut_otbldat_t)QUANT (val, FUT_MAX_PEL12);
	}
	
	return KCP_SUCCESS;
}

/*---------------------------------------------------------------------------
 *  calcOtblLS1 -- calculate an output table gamma value by doing a power law & L to
 					L*  conversion.
 *---------------------------------------------------------------------------
 */

PTErr_t calcOtblLS1 (fut_otbldat_p table, double gamma)
{
	KpUInt32_t	count;
	double		val;
	double		x;
	double		nument_inv;

     /* Check input parameters and initialize:  */
	if (table == (fut_otbldat_p)NULL)
	   return KCP_BAD_ARG;				/* just don't crash */

	nument_inv = 1.0 / (double)(FUT_OUTTBL_ENT - 1);

     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < FUT_OUTTBL_ENT; count++)
	{
		x = (double)count * nument_inv;		/* in [0, 1] */

		/* Compute linearized value by power law:  */
		val = pow (x, gamma);				/* in [0, 1] */
		val = MIN (val, x*SLOPE_LIMIT);		/* limit the slope */

		if (1.0 < val)
			val = 1.0;

		/* calculate L to L* */
		val = H (val);
	
		/* Rescale to 4080 (= 255 << 4):  */
		*(table++) = (fut_otbldat_t)QUANT (val, FUT_MAX_PEL12);

	}

	return KCP_SUCCESS;
}

/*---------------------------------------------------------------------------
 *  calcOtblLN -- calculate an output table by doing a L* to L & 
 					L to device(inverted TRC) conversion.
 *---------------------------------------------------------------------------
 */

PTErr_t calcOtblLN (fut_otbldat_p table, ResponseRecord_p rrp)
{
KpUInt32_t	length;
KpUInt16_p	data;
KpUInt32_t	count;
double		val, p, p1;
KpBool_t	decreasing;

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

	if ((table == NULL) ||
		(length == 0) ||
		(data[length - 1] == data[0]) ||						/* empty domain */											/* bad size */
		(data == NULL)) {	/* no transfer table */
			return KCP_BAD_ARG;
	}
	
     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < FUT_OUTTBL_ENT; count++) {
		
		/* scale and calcuate L* to L */
		p = DEQUANT ((fut_gtbldat_t)count, FUT_GRD_MAXVAL);
		p1 = H_inverse (p);

		/* rescale to TRC length */
		p = RESTRICT (p1, 0.0, 1.0);
		p *= RRECORD_DATA_SIZE;

		/* Find value relative to data table:  */
		val = calcInvertTRC (p, data, length);

		/* rescale and clip to [0, 1] */
		val /= (double) (length - 1);

		if (length < SLOPE_COUNT) {
			if (decreasing == KPTRUE) {
				val = MAX (val, (1-p1)/SLOPE_LIMIT);			/* limit the decreasing slope */
			} else {
				val = MIN (val, p1*SLOPE_LIMIT);				/* limit the increasing slope */
			}
		}

		val = RESTRICT (val, 0.0, 1.0);

		/* Rescale to 4080 (= 255 << 4):  */
		*(table++) = (fut_otbldat_t)QUANT (val, FUT_MAX_PEL12);
	}
	
	return KCP_SUCCESS;
}

PTErr_t calcOtblL1 (fut_otbldat_p table, double gamma)
{
	KpUInt32_t	count;
	double		val, invgamma;
	double		x;
	double		nument_inv;

     /* Check input parameters and initialize:  */
	if (table == (fut_otbldat_p)NULL)
	   return KCP_BAD_ARG;				/* just don't crash */
	if (gamma == 0.0F)
	   return KCP_BAD_ARG;

	/* invert the gamma */
	invgamma = 1.0F / gamma;

     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < FUT_OUTTBL_ENT; count++)
	{
		nument_inv = (double)count / (double)(FUT_OUTTBL_ENT - 1);

		/* calculate L* to L */
		x = H_inverse (nument_inv);		/* in [0, 1] */

		/* Compute linearized value by power law:  */
		val = pow (x, invgamma);				/* in [0, 1] */
		val = MIN (val, x*SLOPE_LIMIT);		/* limit the slope */

		if (1.0 < val)
			val = 1.0;
	
		/* Rescale to 4080 (= 255 << 4):  */
		*(table++) = (fut_otbldat_t)QUANT (val, FUT_MAX_PEL12);

	}

	return KCP_SUCCESS;
}


/* output table function */
fut_otbldat_t otblFunc
	(fut_gtbldat_t q, fut_calcData_p dataP)
{
	if (q) {}
	if (dataP) {}

	return (0x800);
}
