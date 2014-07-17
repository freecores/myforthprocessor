/*
 * @(#)mlib_v_ImageFilters.h	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident   "@(#)mlib_v_ImageFilters.h	1.18    99/10/08 SMI"

/*
 *    These tables are used by VIS versions
 *    of the following functions:
 *      mlib_ImageRotate(Index)
 *      mlib_ImageAffine(Index)
 *      mlib_ImageZoom(Index)
 *      mlib_ImageGridWarp
 *      mlib_ImagePolynomialWarp
 */

#ifndef MLIB_V_IMAGEFILTERS_H
#define MLIB_V_IMAGEFILTERS_H

#include "mlib_image.h"

union table {mlib_s16 s[4]; mlib_d64 d;};

extern const union table mlib_filters_u8_bl[],
                         mlib_filters_u8_bc[],
                         mlib_filters_u8_bc2[],
                         mlib_filters_u8_bc_3[],
                         mlib_filters_u8_bc2_3[],
                         mlib_filters_u8_bc_4[],
                         mlib_filters_u8_bc2_4[],
                         mlib_filters_s16_bc[],
                         mlib_filters_s16_bc2[],
                         mlib_filters_s16_bc_3[],
                         mlib_filters_s16_bc2_3[],
                         mlib_filters_s16_bc_4[],
                         mlib_filters_s16_bc2_4[];

#endif /* MLIB_V_IMAGEFILTERS_H */
