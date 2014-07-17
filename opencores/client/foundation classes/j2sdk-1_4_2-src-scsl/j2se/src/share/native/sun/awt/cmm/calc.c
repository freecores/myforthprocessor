/*
 * @(#)calc.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)calc.c	1.4 99/01/06

	functions to calculate input, output, and grid tables from user
	defined functions.

	COPYRIGHT (c) 1991-1999 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include <string.h>
#include <stdio.h>
#include "kcptmgr.h"
#include "fut_util.h"		/* internal interface file */

/* Add a little extra precision to the itable ramp */
#define	PRECISION 8


/* construct a fut from a set of [iog] functions
 * each array has the functions in order corresponding to the channel position in iomask
 * the first element of fData must be a KpInt32_t and will contain the zero-based channel #
 * being built when the function is called.  The remaining elements are caller defined.
 * If any of the input function arrays are NULL, identity functions are used.
 */

fut_p
	constructfut (	KpInt32_t		iomask,
					KpInt32_p		sizeArray,
					fut_calcData_p	fData,
					fut_ifunc_p		ifunArray,
					fut_gfunc_p		gfunArray,
					fut_ofunc_p		ofunArray,
					PTDataClass_t	iClass,
					PTDataClass_t	oClass)
{
fut_p		futp;
KpInt32_t	i1, imask, omask;
fut_itbl_p	itbls[FUT_NICHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_gtbl_p	gtbls[FUT_NOCHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_otbl_p	otbls[FUT_NOCHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_ifunc_t	ifun;
fut_gfunc_t	gfun;
fut_ofunc_t	ofun;
fut_calcData_t	fDataL;
fut_calcData_p	fDataP;

	if (sizeArray == NULL) {
		return NULL;
	}

	if (fData == NULL) {
		fDataP = &fDataL;
	}
	else {
		fDataP = fData;
	}
	
	imask = FUT_IMASK(iomask);
	omask = FUT_OMASK(iomask);

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256], str2[256];
	KpInt32_t	i1;
	sprintf (string, "constructfut\n iomask %x, sizeArray[]", iomask);
	for (i1 = 0; i1 < FUT_NICHAN; i1++) {
		if ((FUT_BIT(i1) & imask) != 0) {
			sprintf (str2, " %d", sizeArray[i1]);
			strcat (string, str2);
		}
	}
	sprintf (str2, ", fData %x, ifunArray %x, gfunArray %x, ofunArray %x, iClass %d, oClass %d\n",
					fData, ifunArray, gfunArray, ofunArray, iClass, oClass);
	strcat (string, str2);
	kcpDiagLog (string);}
	#endif
	
	/* Compute shared input tables:  */
	for (i1 = 0; i1 < FUT_NICHAN; i1++) {
		if ((imask & FUT_BIT(i1)) != 0) {
			if ((ifunArray == NULL) || (ifunArray[i1] == NULL)) {
				ifun = fut_irampEx;
			}
			else {
				ifun = ifunArray[i1];
			}

			fDataP->chan = i1;	/* define the channel # */

			itbls[i1] = fut_new_itblEx (sizeArray[i1], ifun, fDataP);
			itbls[i1]->id = fut_unique_id ();
			itbls[i1]->dataClass = iClass;
		}
	}

	/* Compute grid tables and output tables:  */
	for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
		if ((omask & FUT_BIT(i1)) != 0) {
			if ((gfunArray == NULL) || (gfunArray[i1] == NULL)) {
				gfun = fut_grampEx;
			}
			else {
				gfun = gfunArray[i1];
			}

			fDataP->chan = i1;	/* define the channel # */

			gtbls[i1] = fut_new_gtblEx (iomask, gfun, fDataP, sizeArray);
			gtbls[i1]->id = fut_unique_id();

			if ((ofunArray == NULL) || (ofunArray[i1] == NULL)) {
				ofun = fut_orampEx;
			}
			else {
				ofun = ofunArray[i1];
			}

			otbls[i1] = fut_new_otblEx (ofun, fDataP);
			otbls[i1]->id = fut_unique_id();
			otbls[i1]->dataClass = oClass;
		}
	}

	/* Assemble FuT:  */
	futp = fut_new (iomask, itbls, gtbls, otbls);

	fut_free_tbls (FUT_NICHAN, (KpGenericPtr_t *)itbls);
	fut_free_tbls (FUT_NOCHAN, (KpGenericPtr_t *)gtbls);
	fut_free_tbls (FUT_NOCHAN, (KpGenericPtr_t *)otbls);

	if (fut_to_mft (futp) != 1) {		/* convert to reference tables */
		fut_free (futp);
		futp = NULL;
	}

	return (futp);
}


/* fut_calc_itbl computes the values of an input table from a user
 * defined function.  Ifun must be a pointer to a function accepting a
 * double and returning a double, both in the range (0.0,1.0).  (NULL is
 * a legal value - it returns leaving the table uninitialized)
 * fut_calc_itbl returns 0 (FALSE) if an error occurs (ifun returned
 * value out of range) and  1 (TRUE) otherwise.
 *
 * Another NOTE:
 *    To avoid referencing off-grid values (possibly resulting in a
 *    memory violation error) we clip the input table entries to the
 *    value ((grid size-1)<<FUT_INP_DECIMAL_PT)-1.  Thus, when
 *    interpolating a value at the very last grid point, we will get:
 *
 *        val = g[n-2] + (1-1/64K)*(g[n-1]-g[n-2])
 *            = g[n-1] + (1/64K) * (g[n-2]-g[n-1])
 *
 *    instead of:
 *
 *        val = g[n-2] + 0 * (g[n]-g[n-1])
 *            = g[n-1] (or error because g[n] is not defined)
 *
 *    Since the grid table entries are 12-bits, this causes an
 *    interpolation * error of at most 1/16 in the * result,
 *    which when rounded, disappears completely.
 */
KpInt32_t
	fut_calc_itblEx (	fut_itbl_p		itbl,
						fut_ifunc_t		ifun,
						fut_calcData_p	data)
{
fut_itbldat_p	theItbl;
KpInt32_t		i;
double			val, dataNorm, indexNorm;
fut_itbldat_t	dataMax, itblData;
	
	if ( ! IS_ITBL(itbl) ) {
		return (0);
	}

	if (ifun != NULL) {
		itbl->id = fut_unique_id ();		/* new table values, get new unique id */

		dataMax = ((itbl->size - 1) << FUT_INP_DECIMAL_PT) -1;
		dataNorm = (double) (dataMax +1);
		indexNorm = (double) (FUT_INPTBL_ENT-1);
		theItbl = itbl->tbl;

		for (i = 0; i < FUT_INPTBL_ENT; i++) {
			val = (*ifun) ((double) i / indexNorm, data);

			if ((val < 0.0) || (val > 1.0)) {
				return (0);
			}

			itblData = (fut_itbldat_t) ((val * dataNorm) + 0.5);

			if (itblData > dataMax) {
				itblData = dataMax;		/* clip value to 1 less than norm ( see note above) */
			}

			theItbl[i] = itblData;
		}

		/* set the very last (generally invisible) input table entry to the
		 * value of the previous one.  This will perform automatic clipping
		 * of input greater than 4080 to the valid gridspace, which is defined
		 * only for input in the range (0,255) or (0<<4,255<<4).
		 */

		theItbl[FUT_INPTBL_ENT] = theItbl[FUT_INPTBL_ENT -1];
	}

	return (1);
}


/* fut_calc_gtblEx computes the values of a grid table from a user
 * defined function.  Gfun must be a pointer to a function accepting
 * doubles in the range (0.0,1.0) and returning a fut_gtbldat_t in the
 * interval (0,FUT_GRD_MAXVAL). (NULL is a legal value - it just returns
 * leaving the table uninitialized).
 * fut_calc_gtblEx returns 0 (FALSE) if an error occurs (gfun returned
 * value outof range), 1 (TRUE) otherwise.
 */

#define GCLOOP(x) for (i[x] = 0; i[x] < n[x]; i[x]++ ) { \
		    cList[x] = i[x] * norm[x];

KpInt32_t
	fut_calc_gtblEx (	fut_gtbl_p		gtbl,
						fut_gfunc_t		gfun,
						fut_calcData_p	data)
{
KpInt32_t		index, n[FUT_NICHAN], i[FUT_NICHAN];
double			norm[FUT_NICHAN], cList[FUT_NICHAN];
fut_gtbldat_p	grid;
fut_gtbldat_t	val;

	if ( ! IS_GTBL(gtbl) ) {
		return (0);
	}

	if (gfun != NULL) {
		/* set up grid size in each dimension */
		for (index = 0; index < FUT_NICHAN; index++) {
			n[index] = gtbl->size[index];

			if (n[index] == 1) {
				norm[index] = 0.0;
			}
			else {
				norm[index] = 1.0 / (double) (n[index] -1);
			}
		}

		gtbl->id = fut_unique_id();	/* new table data, new id */

		/* construct function of 1 to 8 input variables */
		grid = gtbl->tbl;

		GCLOOP(0)
			GCLOOP(1)
				GCLOOP(2)
					GCLOOP(3)
						GCLOOP(4)
							GCLOOP(5)
								GCLOOP(6)
									GCLOOP(7)
										val = (*gfun)(cList, data);
										if ( (unsigned) val > FUT_GRD_MAXVAL ) {
											return (0);
										}
										*grid++ = val;
									}
								}
							}
						}
				    }
				}
		    }
		}
	}

	return (1);
}


/* fut_calc_otbl computes the values of an output table from a user defined
 * function.  Ofun must be a pointer to a function accepting a fut_gtbldat_t
 * in the range (0,FUT_GRD_MAXVAL) and returning a fut_otbldat_t in
 * the same interval (NULL is a legal value - it just returns
 * leaving the table uninitialized).
 * fut_calc_otbl returns 0 (FALSE) if an error occurs (ofun returned
 * value out of range) and 1 (TRUE) otherwise.
 */
KpInt32_t
	fut_calc_otblEx (	fut_otbl_p		otbl,
						fut_ofunc_t		ofun,
						fut_calcData_p	data)
{
KpInt32_t		i;
fut_otbldat_p	theOtbl;
fut_otbldat_t	val;

	if ( ! IS_OTBL(otbl) ) {
		return (0);
	}

	if (ofun != NULL) {
		theOtbl = otbl->tbl;

		otbl->id = fut_unique_id();

		for ( i=0; i<FUT_OUTTBL_ENT; i++ ) {
			val = ((*ofun) ((fut_gtbldat_t) i, data));

			if (val > FUT_GRD_MAXVAL) {
				return (0);
			}

			theOtbl[i] = val;
		}
	}

	return (1);
}


/* identity functions for initializing and calculating tables. */

double
	fut_irampEx	(double x, fut_calcData_p data)
{
	if (data) {}
	
	return (x);
}

fut_gtbldat_t
	fut_grampEx	(double_p dP, fut_calcData_p data)
{
KpInt32_t		chan;
fut_gtbldat_t	gdat;

	chan = data->chan;

	gdat = (fut_gtbldat_t) ((FUT_GRD_MAXVAL * dP[chan]) + 0.499999);

	return gdat;
}

fut_otbldat_t
	fut_orampEx	(fut_gtbldat_t x, fut_calcData_p data)
{
KpInt32_t	odat;

	if (data) {}

	odat = (KpInt32_t) x;
	odat *= FUT_MAX_PEL12;
	odat += FUT_GRD_MAXVAL >> 1;
	odat /= FUT_GRD_MAXVAL;
	
	return ((fut_otbldat_t) odat);
}
