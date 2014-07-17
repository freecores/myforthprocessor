/*
 * @(#)debug.h	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// Debug.h  by Stanley Man-Kit Ho
//
//=---------------------------------------------------------------------------=
//
// Contains declaration of the debugging functions
//


#ifdef _DEBUG

#define TRACE(x)	    OutputDebugString(x)
#define TRACE2(x, y)	    TRACE(x);TRACE(y)
#define TRACE3(x, y, z)	    TRACE(x);TRACE(y);TRACE(z)

#else

#define TRACE(x)
#define TRACE2(x, y)
#define TRACE3(x, y, z)

#endif // _DEBUG 

