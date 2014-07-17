/*
 * @(#)chnstub.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)chnstub.c	1.6 99/01/06

        Contains:       KCMS PT chaining function stubs

 *********************************************************************
 *    COPYRIGHT (c) 1994-1999 Eastman Kodak Company
 *    As an unpublished work pursuant to Title 17 of the United
 *    States Code.  All rights reserved.
 *********************************************************************
*/

#include "kcmsos.h"
#include "kcmptlib.h"
#include "kcptmgr.h"

PTErr_t PTChain (PTRefNum_t PTRefNum){
	if (PTRefNum){}
	return (KCP_SYSERR_1);
}

PTErr_t PTChainInit
		(KpInt32_t nPT, PTRefNum_p PTList, KpInt32_t validate, KpInt32_p index){
	if (nPT){}
	if (PTList){}
	if (validate){}
	if (index){}
	return (KCP_SYSERR_1);
}

PTErr_t PTChainInitM
		(KpInt32_t nPT, PTRefNum_p PTList, KpInt32_t compMode, KpInt32_t rulesKey){
	if (nPT){}
	if (PTList){}
	if (compMode){}
	if (rulesKey){}
	return (KCP_SYSERR_1);
}

PTErr_t PTChainEnd (PTRefNum_p PTRefNum){
	if (PTRefNum){}
	return (KCP_SYSERR_1);
}

PTErr_t PTChainValidate (KpInt32_t nPT, PTRefNum_p PTList, KpInt32_p index){
	if (nPT){}
	if (PTList){}
	if (index){}
	return (KCP_SYSERR_1);
}

void
	addCompType (KpInt32_p kcpFlavor) {
	if (kcpFlavor) {}
}

void
	KCPChainSetup (	initializedGlobals_p	iGP) {
	if (iGP) {}
}

PTErr_t ComposeAttr(PTRefNum_t PTRefNum1, PTRefNum_t PTRefNum2, KpInt32_t mode, PTRefNum_t PTRefNumR){
	if (PTRefNum1) {}
	if (PTRefNum2) {}
	if (mode) {}
	if (PTRefNumR) {}
	return (KCP_SUCCESS);
}
