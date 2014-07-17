
 /* @(#)s_finite.c	1.7 03/01/23           */
/*
 * @(#)s_finite.c	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * finite(x) returns 1 is x is finite, else 0;
 * no branching!
 */

#include "fdlibm.h"

#ifdef __STDC__
	int finite(double x)
#else
	int finite(x)
	double x;
#endif
{
	int hx; 
	hx = __HI(x);
	return  (unsigned)((hx&0x7fffffff)-0x7ff00000)>>31;
}
