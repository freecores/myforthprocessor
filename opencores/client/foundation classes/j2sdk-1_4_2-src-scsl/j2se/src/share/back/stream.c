/*
 * @(#)stream.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include "stream.h"
#include "util.h"

jfloat 
stream_encodeFloat(jfloat theFloat) 
{
    union {
        jfloat f;
        jint i;
    } sF;

    sF.f = theFloat;

    sF.i = HOST_TO_JAVA_INT(sF.i);

    return sF.f;
}

jdouble 
stream_encodeDouble(jdouble d)
{
    union {
	jdouble d;
	jlong l;
    } sD;

    sD.d = d;

    sD.l = HOST_TO_JAVA_LONG(sD.l);

    return sD.d;
}


