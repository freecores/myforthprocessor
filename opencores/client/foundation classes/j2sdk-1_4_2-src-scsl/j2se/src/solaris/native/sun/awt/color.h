/*
 * @(#)color.h	1.25 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _COLOR_H_
#define _COLOR_H_

#include "awt.h"
#include "colordata.h"

#ifndef HEADLESS
typedef struct {
    unsigned int Depth;
    XPixmapFormatValues wsImageFormat;
    ImgColorData clrdata;
    ImgConvertFcn *convert[NUM_IMGCV];
} awtImageData;
#endif /* !HEADLESS */

#endif           /* _COLOR_H_ */
