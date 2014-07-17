/*
 * @(#)mlib_v_ImageConvMxN_8.h	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#pragma ident	"@(#)mlib_v_ImageConvMxN_8.h	1.5	98/11/25 SMI"

#ifndef MLIB_IMAGECONVMXN_8_H
#define MLIB_IMAGECONVMXN_8_H

/*
 * FUNCTION
 *     Macros for mlib_conv7x7_16nw   (16-bit convolution routines,
 *                                     edge = no write)
 */

#include <mlib_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PXLS_PER_BLOCK  16

struct COEFF_REC {
  mlib_f32 coeff;
  mlib_s32 spoffs;
};

#ifdef __cplusplus
}
#endif

#endif  /* MLIB_IMAGECONVMXN_8_H */

/***************************************************************/
