/*
 * @(#)awt_PrintJob.h	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_PRINT_JOB_H
#define AWT_PRINT_JOB_H

#include "stdhdrs.h"

/************************************************************************
 * PrintJob class
 */

class AwtPrintJob {
public:
    static jfieldID pageDimensionID;
    static jfieldID pageResolutionID;
    static jfieldID truePageResolutionID;
    static jfieldID graphicsID;
};

#endif /* AWT_PRINT_JOB_H */x
