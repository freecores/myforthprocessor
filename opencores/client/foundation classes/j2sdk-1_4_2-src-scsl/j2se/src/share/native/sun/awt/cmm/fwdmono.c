/*
 * @(#)fwdmono.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)fwdmono.c	1.12 98/11/04

	Contains:	makeForwardXformMono

	COPYRIGHT (c) 1997-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include <math.h>
#include <string.h>
#include "makefuts.h"

#define FWD_MONO_OCHANS (3)

/*---------------------------------------------------------------------------
 *  makeForwardXformMono -- make a fut of given gridsize from given TRC data
 *		for forward transform (gray -> Lab); return status code
 *---------------------------------------------------------------------------
 */

PTErr_t
	makeForwardXformMono (	ResponseRecord_p	grayTRC,
							fut_p				theFut)
{
PTErr_t			PTErr = KCP_FAILURE;
KpInt32_t		futReturn, i1;
fut_otbldat_p	otblDat;
double			gamma;
fut_calcData_t	calcData;
KpUInt16_t		rrpData[2] = { 0, RRECORD_DATA_SIZE -1 };
ResponseRecord_t	rrt;

	/* compute new table entries */
	calcData.chan = 0;				/* always uses 1st input chan */

	for (i1 = 0; i1 < FWD_MONO_OCHANS; i1++) {
		if (( ! IS_CHAN(theFut->chan[i1])) ||
			!fut_calc_gtblEx (theFut->chan[i1]->gtbl, fut_grampEx, &calcData) ||
			!fut_calc_otblEx (theFut->chan[i1]->otbl, otblFunc, NULL)) {
			goto ErrOut0;
		}
	}

	/* get address of the first output table */
	futReturn = fut_get_otbl (theFut, 0, &otblDat);
	if ((futReturn != 1) || (otblDat == (fut_otbldat_p)NULL)) {
		goto ErrOut0;
	}

	/* setup the output table */
	switch (grayTRC->count)
	{
	case 0:
		/* setup the responseRecord struct */
		rrt.count = 2;
		rrt.data = rrpData;

		/* make the output table */
		PTErr = calcOtblLSN (otblDat, &rrt);
		break;

	case 1:
		gamma = (double)grayTRC->data[0] / SCALEDOT8;
		if (gamma <= 0.0) {
			goto ErrOut0;
		}

		/* make the output table */
		PTErr = calcOtblLS1 (otblDat, gamma);
		break;

	default:
		/* make the output table */
		makeMonotonic (grayTRC->count, grayTRC->data);
		PTErr = calcOtblLSN (otblDat, grayTRC);
	}

GetOut:
	return PTErr;


ErrOut0:
	PTErr = KCP_SYSERR_0;
	goto GetOut;
}
