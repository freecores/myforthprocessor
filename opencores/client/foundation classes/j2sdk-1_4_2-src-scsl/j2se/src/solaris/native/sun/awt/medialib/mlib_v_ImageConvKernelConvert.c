/*
 * @(#)mlib_v_ImageConvKernelConvert.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageConvKernelConvert.c	1.12	00/03/09 SMI"

/*
 * FUNCTION
 *      mlib_ImageConvKernelConvert - Convert convolution kernel from
 *                                    floating point version to integer
 *                                    version.
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageConvKernelConvert(mlib_s32  *ikernel,
 *                                              mlib_s32  *iscale,
 *                                              mlib_d64  *fkernel,
 *                                              mlib_s32  m,
 *                                              mlib_s32  n,
 *                                              mlib_type type);
 *
 * ARGUMENT
 *      ikernel  integer kernel
 *      iscale   scaling factor of the integer kernel
 *      fkernel  floating-point kernel
 *      m        width of the convolution kernel
 *      n        height of the convolution kernel
 *      type     image type
 *
 * DESCRIPTION
 *      Convert a floating point convolution kernel to integer kernel
 *      with scaling factor. The result integer kernel and scaling factor
 *      can be used in convolution functions directly without overflow.
 */

/***********************************************************************/

#include <stdlib.h>
#include <mlib_image.h>
#include <mlib_SysMath.h>

/***********************************************************************/

#ifdef MLIB_TEST

mlib_status mlib_v_ImageConvKernelConvert(mlib_s32 * ikernel,
                                          mlib_s32 * iscale,
                                          mlib_d64 * fkernel,
                                          mlib_s32 m,
                                          mlib_s32 n,
                                          mlib_type type)
#else

mlib_status mlib_ImageConvKernelConvert(mlib_s32 * ikernel,
                                        mlib_s32 * iscale,
                                        mlib_d64 * fkernel,
                                        mlib_s32 m,
                                        mlib_s32 n,
                                        mlib_type type)
#endif
{
  mlib_d64 sum_pos, sum_neg, sum, norm, max, f;
  mlib_s32 isum_pos, isum_neg, isum, test;
  mlib_s32 i, scale, scale1, *tmp_ikernel;
  mlib_s32 iskernel[256], *idkernel;
  mlib_s32 ismalloc;

  if (ikernel == NULL ||
      iscale == NULL ||
      fkernel == NULL ||
      ((type != MLIB_BYTE) &&
       (type != MLIB_SHORT) &&
       (type != MLIB_INT) &&
       (type != MLIB_BIT)) || m < 1 || n < 1)
    return MLIB_FAILURE;

  if ((type == MLIB_BYTE) || (type == MLIB_SHORT)) {

    tmp_ikernel = iskernel;
    ismalloc = 0;

    if (m * n > 256) {
      idkernel = (mlib_s32 *) mlib_malloc(m * n * sizeof(mlib_s32));
      if (idkernel == NULL)
        return MLIB_FAILURE;
      tmp_ikernel = idkernel;
      ismalloc = 1;
    }

    if (type == MLIB_BYTE) {
      sum_pos = 0;
      sum_neg = 0;

      for (i = 0; i < m * n; i++) {
        if (fkernel[i] > 0)
          sum_pos += fkernel[i];
        else
          sum_neg += fkernel[i];
      }

      sum_neg = mlib_fabs(sum_neg);
      sum = (sum_pos > sum_neg) ? sum_pos : sum_neg;
      scale = mlib_ilogb(sum);
      if (scale > 13)
        return MLIB_FAILURE;
      scale = 14 - scale;
    }
    else {
      sum = 0;
      max = 0;

      for (i = 0; i < m * n; i++) {
        f = mlib_fabs(fkernel[i]);
        sum += f;
        max = (max > f) ? max : f;
      }
      scale1 = mlib_ilogb(max) + 1;
      scale = mlib_ilogb(sum);
      scale = (scale > scale1) ? scale : scale1;
      if (scale > 14)
        return MLIB_FAILURE;
      scale = 15 - scale;
    }

    if (scale > 15)
      scale = 15;

    norm = (32768 >> (15 - scale));

    for (i = 0; i < m * n; i++) {
      if (fkernel[i] > 0)
        tmp_ikernel[i] = (mlib_s32) (fkernel[i] * norm + 0.5);
      else
        tmp_ikernel[i] = (mlib_s32) (fkernel[i] * norm - 0.5);
    }

    isum_pos = 0;
    isum_neg = 0;
    test = 0;

    for (i = 0; i < m * n; i++) {
      if (tmp_ikernel[i] > 0)
        isum_pos += tmp_ikernel[i];
      else
        isum_neg += tmp_ikernel[i];
    }

    isum_neg = abs(isum_neg);

    if (type == MLIB_BYTE) {
      isum = (isum_pos > isum_neg) ? isum_pos : isum_neg;
      if (isum > 32767) {
        test = 1;
      }
    }
    else {
      isum = isum_pos + isum_neg;
      if (isum > 65535) {
        test = 1;
      }
      if (test == 0) {
        isum = 0;
        for (i = 0; i < m * n; i++) {
          isum =
                  (isum >
                  abs(tmp_ikernel[i])) ? isum :
                  abs(tmp_ikernel[i]);
        }
        if (isum > 32767)
          test = 1;
      }
    }

    if (test == 1) {
      for (i = 0; i < m * n; i++)
        ikernel[i] = (mlib_s32) (fkernel[i] * norm) << 16;
    }
    else {
      for (i = 0; i < m * n; i++)
        ikernel[i] = tmp_ikernel[i] << 16;
    }

    *iscale = scale + 16;

    if (ismalloc != 0)
      mlib_free(idkernel);
    return MLIB_SUCCESS;

  }
  else { /* type == MLIB_INT || type == MLIB_BIT */
    max = 0;

    for (i = 0; i < m * n; i++) {
      f = mlib_fabs(fkernel[i]);
      max = (max > f) ? max : f;
    }

    scale = mlib_ilogb(max);
    if (scale > 29)
      return MLIB_FAILURE;
    if (scale < -100)
      scale = -100;

    *iscale = 29 - scale;
    scale = 29 - scale;

    norm = 1.0;
    while (scale > 30) {
      norm *= (1 << 30);
      scale -= 30;
    }

    norm *= (1 << scale);

    for (i = 0; i < m * n; i++) {
      if (fkernel[i] > 0)
        ikernel[i] = (mlib_s32) (fkernel[i] * norm + 0.5);
      else
        ikernel[i] = (mlib_s32) (fkernel[i] * norm - 0.5);
    }

    return MLIB_SUCCESS;
  }
}

/***************************************************************/
