/*
 * @(#)mlib_c_ImageConvMxN.c	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageConvMxN.c	1.25	99/08/12 SMI"
#endif /* __SUNPRO_C */

/*
 * FUNCTION
 *      mlib_ImageConvMxN - image convolution with edge condition
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageConvMxN(mlib_image *dst,
 *                                    mlib_image *src,
 *                                    mlib_s32   *kernel,
 *                                    mlib_s32   m,
 *                                    mlib_s32   n,
 *                                    mlib_s32   dm,
 *                                    mlib_s32   dn,
 *                                    mlib_s32   scale,
 *                                    mlib_s32   cmask,
 *                                    mlib_edge  edge)
 *
 * ARGUMENTS
 *      dst       Pointer to destination image.
 *      src       Pointer to source image.
 *      m         Kernel width (m must be larger than 1).
 *      n         Kernel height (n must be larger than 1).
 *      dm, dn    Position of key element in convolution kernel.
 *      kernel    Pointer to convolution kernel.
 *      scale     The scaling factor to convert the input integer
 *                coefficients into floating-point coefficients:
 *                floating-point coefficient = integer coefficient * 2^(-scale)
 *      cmask     Channel mask to indicate the channels to be convolved.
 *                Each bit of which represents a channel in the image. The
 *                channels corresponded to 1 bits are those to be processed.
 *      edge      Type of edge condition.
 *
 * DESCRIPTION
 *      2-D convolution, MxN kernel.
 *
 *      The center of the source image is mapped to the center of the
 *      destination image.
 *      The unselected channels are not overwritten. If both src and dst have
 *      just one channel, cmask is ignored.
 *
 *      The edge condition can be one of the following:
 *              MLIB_EDGE_DST_NO_WRITE  (default)
 *              MLIB_EDGE_DST_FILL_ZERO
 *              MLIB_EDGE_DST_COPY_SRC
 *              MLIB_EDGE_SRC_EXTEND
 *
 * RESTRICTION
 *      The src and the dst must be the same type and have same number
 *      of channels (1, 2, 3, or 4).
 *      m > 1.   n > 1.
 *      0 <= dm < m. 0 <= dn < n.
 *      For data type MLIB_BYTE - 16 <= scale <= 31 (to be compatible with VIS
 *      version).
 *      For data type MLIB_SHORT - 17 <= scale <= 32 (to be compatible with VIS
 *      version).
 *      For data type MLIB_INT - scale >= 0.
 */

#include <mlib_image.h>

/***************************************************************/

mlib_status mlib_ImageConvMxN_f(mlib_image *dst,
                                mlib_image *src,
                                mlib_s32   *kernel,
                                mlib_s32   m,
                                mlib_s32   n,
                                mlib_s32   dm,
                                mlib_s32   dn,
                                mlib_s32   scale,
                                mlib_s32   cmask,
                                mlib_edge  edge);

/***************************************************************/

/* call common C version (file mlib_ImageConvMxNFunc.c) */

mlib_status mlib_ImageConvMxN(mlib_image *dst,
                              mlib_image *src,
                              mlib_s32   *kernel,
                              mlib_s32   m,
                              mlib_s32   n,
                              mlib_s32   dm,
                              mlib_s32   dn,
                              mlib_s32   scale,
                              mlib_s32   cmask,
                              mlib_edge  edge)
{
  return mlib_ImageConvMxN_f(dst, src, kernel, m, n, dm, dn, scale, cmask, edge);
}

/***************************************************************/
