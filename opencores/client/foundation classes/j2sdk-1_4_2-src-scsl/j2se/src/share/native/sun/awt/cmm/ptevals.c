/*
 * @(#)ptevals.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ptevals.c	2.17 98/12/03

	Contains:	PT evaluator table stuff

 *********************************************************************
 *    COPYRIGHT (c) 1991-1998 Eastman Kodak Company
 *    As an unpublished work pursuant to Title 17 of the United
 *    States Code.  All rights reserved.
 *********************************************************************
 */


#include "kcmptlib.h"
#include "kcptmgrd.h"
#include "kcptmgr.h"


	/* return number and type of all available evaluators */
PTErr_t
	PTEvaluators	(KpInt32_p nEval, evalList_t theList[])
{
initializedGlobals_p	iGP;

	iGP = getInitializedGlobals ();
	if (iGP == NULL) {
		return KCP_NO_PROCESS_GLOBAL_MEM;
	}

	if  (nEval == NULL) {
		return KCP_BAD_PTR;
	}
	
	if  (theList == NULL) {
		return KCP_BAD_PTR;
	}

	theList[0].evalID = KCP_EVAL_SW;	/* SW always available */
	theList[0].number = 1;
	*nEval = 1;

#if defined (KCP_ACCEL)
	if (iGP->haveCTE == 1) {				/* CTE is present */
		theList[1].evalID = KCP_EVAL_CTE;
		theList[1].number = 1;
		(*nEval)++;
	}
#endif

	return (KCP_SUCCESS);
}


/* return evaluator to use given desired type */
PTErr_t
	GetEval	(PTEvalTypes_t reqEval, PTEvalTypes_t *useEval)
{
#if defined (KCP_ACCEL)
initializedGlobals_p	iGP;

	iGP = getInitializedGlobals ();
	if (iGP == NULL) {
		return KCP_NO_PROCESS_GLOBAL_MEM;
	}

	if (reqEval == KCP_EVAL_DEFAULT) {		/* if default, use best available */
		reqEval = KCP_EVAL_CTE;
	}

	if (iGP->haveCTE == 0) {				/* if no CTE, must use SW */
#endif
		reqEval = KCP_EVAL_SW;
#if defined (KCP_ACCEL)
	}
#endif

	*useEval = reqEval;						/* assume requested evaluator is available */

	return KCP_SUCCESS;
}
