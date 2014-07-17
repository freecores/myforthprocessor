
 /* @(#)s_fabs.c	1.7 03/01/23           */
/*
 * @(#)s_fabs.c	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * fabs(x) returns the absolute value of x.
 */

#include "fdlibm.h"

#ifdef __STDC__
	double fabs(double x)
#else
	double fabs(x)
	double x;
#endif
{
	__HI(x) &= 0x7fffffff;
        return x;
}
