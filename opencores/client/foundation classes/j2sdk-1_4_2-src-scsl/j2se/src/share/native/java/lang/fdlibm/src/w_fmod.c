
 /* @(#)w_fmod.c	1.7 03/01/23           */
/*
 * @(#)w_fmod.c	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * wrapper fmod(x,y)
 */

#include "fdlibm.h"


#ifdef __STDC__
	double fmod(double x, double y)	/* wrapper fmod */
#else
	double fmod(x,y)		/* wrapper fmod */
	double x,y;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_fmod(x,y);
#else
	double z;
	z = __ieee754_fmod(x,y);
	if(_LIB_VERSION == _IEEE_ ||isnan(y)||isnan(x)) return z;
	if(y==0.0) {
	        return __kernel_standard(x,y,27); /* fmod(x,0) */
	} else
	    return z;
#endif
}
