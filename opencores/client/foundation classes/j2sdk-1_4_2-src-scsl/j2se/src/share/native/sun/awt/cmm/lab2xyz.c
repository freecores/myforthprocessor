/*
 * @(#)lab2xyz.c	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)lab2xyz.c	1.20 99/01/06

	Contains:	functions to create an lab->XYZ FuT.

	Created by lsh, November 11, 1993

	COPYRIGHT (c) 1993-1999 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "kcptmgr.h"
#include "makefuts.h"

#define KCP_THIS_FUT_CHANS (3)

typedef struct fData_s {
	fut_calcData_t	std;
	KpInt32_t	size[KCP_THIS_FUT_CHANS];
	double		dimSizeM1, dimSizeD2;
} fData_t, FAR* fData_p;


/* abifun -- input mappings:  Convert a* and b* to unsigned
 *---------------------------------------------------------------------
 */
static double
	abifun (double ab, fut_calcData_p dataP)
{
double		dimSizeM1, dimSizeD2;

	dimSizeM1 = ((fData_p) dataP)->dimSizeM1;
	dimSizeD2 = ((fData_p) dataP)->dimSizeD2;

	ab = 255.0 * ab;

	if (ab <= 128.0) {
		ab = (dimSizeD2 / dimSizeM1) * (ab / 128.0);
	}
	else {
		ab = 1.0 - (((dimSizeM1 - dimSizeD2) / dimSizeM1) * ((255.0 - ab) / 127.0));
	}

	return ab;
}


/*---------------------------------------------------------------------
 *  trifun -- grid-table functions:  Lab -> XYZ
 *
 * representing L* in [0, 100], a* and b* in [-128, 127] */

static fut_gtbldat_t
	gfun (double_p	args, fut_calcData_p dataP)
{
double	l, a, b, tristim, g;
KpInt32_t	ySize, zSize;

	l = args[0];
	a = args[1];
	b = args[2];

	ySize = ((fData_p) dataP)->size[1];
	zSize = ((fData_p) dataP)->size[2];

/* Undo grid mapping of a* and b*:  */
	if (a <= (double)(ySize / 2) / (double)(ySize - 1)) {
		a = (128.0 / 255.0) * ((double)(ySize - 1) / (double)(ySize / 2)) * a;
	}
	else {
		a = 1.0 - (127.0 / 255.0) * ((double)(ySize - 1) / (double)(ySize - 1 - (ySize / 2))) 
			* (1.0 - a);
	}
	
	if (b <= (double)(zSize / 2) / (double)(zSize - 1)) {
		b = (128.0 / 255.0) * ((double)(zSize - 1) / (double)(zSize / 2)) * b;
	}
	else {
		b = 1.0 - (127.0 / 255.0) * ((double)(zSize - 1) / (double)(zSize - 1 - (zSize / 2))) 
			* (1.0 - b);
	}

/* Shift and rescale a* and b*:  */
	a = 255.0 * a - 128.0;		/* CIE 1976 a* */
	a = 0.00232 * a;			/* H(X/X_n) - H(Y/Y_n), in [-0.297, 0.295] */
	b = 255.0 * b - 128.0;		/* CIE 1976 b* */
	b = 0.00580 * b;			/* H(Y/Y_n) - H(Z/Z_n), in [-0.742, 0.737] */

/* Separate XYZ channels:  */
	switch (((fData_p) dataP)->std.chan) {
	case 0:
		tristim = a + l;	/* H(X/X_n), in [-0.297, 1.295 */
		break;

	case 1:
		tristim = l;		/* H(Y/Y_n), in [0, 1] */
		break;

	case 2:
		tristim = l - b;	/* H(Z/Z_n), in [-0.742, 1.737] */
		break;
	}

/* Rescale & return:  */
	tristim = (tristim + 1.0) / 3.0;	/* in [0.086, 0.9123] */

	g = RESTRICT (tristim, 0.0, 1.0);

	return QUANT (g, FUT_GRD_MAXVAL);
}


/*------------------------------------------------------------------
 *  ofun -- quantization call-back functions for FuT library
 *------------------------------------------------------------------
 */
static fut_otbldat_t
	ofun (fut_gtbldat_t q, fut_calcData_p dataP)
{
double		s, p;

	p = DEQUANT (q, FUT_GRD_MAXVAL);

	p = (3.0 * p) - 1.0;		/* [0, 1] -> [-1, 2] */
	p = RESTRICT (p, 0.0, 2.0);	/* leave headroom */
	p = H_inverse (p);			/* X/X_n, Y/Y_n, Z/Z_n */

     /* Interpret according to white point and rescale for ICC:  */
	switch (dataP->chan) {
	case 0:
		p *= (KCP_D50_X * XYZSCALE);	/* X */
		break;

	case 1:
		p *= (KCP_D50_Y * XYZSCALE);	/* Y */
		break;

	case 2:
		p *= (KCP_D50_Z * XYZSCALE);	/* Z */
		break;
	}

	s = RESTRICT (p, 0.0, 1.0);	/* clip to valid range [0, 1] */

	return QUANT (s, FUT_MAX_PEL12);
}

/*----------------------------------------------------------------------
 *  get_lab2xyz_fut --	construct an Lab to XYZ FuT FuT
 *						to establish a grid for composition
 *----------------------------------------------------------------------
 */

#define KCP_THIS_FUT_CHANS (3)

fut_p
	get_lab2xyz (	KpInt32_t	size)
{
fut_p		futp;
KpInt32_t	iomask;
fut_ifunc_t	ifunArray[KCP_THIS_FUT_CHANS] = {NULL, abifun, abifun};
fut_gfunc_t	gfunArray[KCP_THIS_FUT_CHANS] = {gfun, gfun, gfun};
fut_ofunc_t	ofunArray[KCP_THIS_FUT_CHANS] = {ofun, ofun, ofun};
fData_t		fData;

	#if defined KCP_DIAG_LOG
	kcpDiagLog ("get_lab2xyz\n");
	#endif

	iomask = FUT_IN(FUT_XYZ) | FUT_OUT(FUT_XYZ);

	/* define neutral grid point */
	fData.dimSizeM1 = (double)(size - 1);
	fData.dimSizeD2 = (double)(size / 2);

	/* all dimensions are the same */
	fData.size[0] = size;
	fData.size[1] = size;
	fData.size[2] = size;

	/* Compute shared input tables, grid tables, and output tables:  */
	futp = constructfut (iomask, fData.size, &fData.std, ifunArray, gfunArray, ofunArray,
						KCP_LAB_PCS, KCP_XYZ_PCS);

	#if defined KCP_DIAG_LOG
	saveFut (futp, "CP18.fut");
	#endif

	return (futp);
}
