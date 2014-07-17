/*
 * @(#)mlib_c_ImageConvCopyEdge.c	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageConvCopyEdge.c	1.8	99/09/07 SMI"
#endif /* __SUNPRO_C */

/*
 * FILENAME: mlib_ImageConvCopyEdge.c
 *
 * FUNCTIONS
 *      mlib_ImageConvCopyEdge  - Copy src edges  to dst edges
 *
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageConvCopyEdge(mlib_image *dst,
 *                                           mlib_image *src,
 *                                           mlib_s32 dx_l, mlib_32 dx_r,
 *                                           mlib_s32 dy_t, mlib_32 dy_b,
 *                                           mlib_s32 cmask);
 *
 * ARGUMENT
 *      dst       Pointer to an dst image.
 *      src       Pointer to an src image.
 *      dx_l      Number of columns on the left side of the
 *                image to be copyed.
 *      dx_r      Number of columns on the right side of the
 *                image to be copyed.
 *      dy_t      Number of rows on the top edge of the
 *                image to be copyed.
 *      dy_b      Number of rows on the top edge of the
 *                image to be copyed.
 *      cmask     Channel mask to indicate the channels to be convolved.
 *                Each bit of which represents a channel in the image. The
 *                channels corresponded to 1 bits are those to be processed.
 *
 * RESTRICTION
 *      The src and the dst must be the same type, same width, same height and have same number
 *      of channels (1, 2, 3, or 4). The unselected channels are not
 *      overwritten. If both src and dst have just one channel,
 *      cmask is ignored.
 *
 * DESCRIPTION
 *      Copy src edges  to dst edges.
 *
 *      The unselected channels are not overwritten.
 *      If src and dst have just one channel,
 *      cmask is ignored.
 */

#include <stdlib.h>
#include "mlib_image.h"

#define EDGES(chan, type, mask)                                                           \
{                                                                                         \
    type *pdst = (type *) mlib_ImageGetData(dst); /* pointer to the data of img-image */  \
    type *psrc = (type *) mlib_ImageGetData(src); /* pointer to the data of img-image */  \
    int img_height = mlib_ImageGetHeight(dst);    /* height of source image */            \
    int img_width  = mlib_ImageGetWidth(dst);     /* width of source image */             \
    int dst_stride = mlib_ImageGetStride(dst) / sizeof(type); /* elements to next row */  \
    int src_stride = mlib_ImageGetStride(src) / sizeof(type); /* elements to next row */  \
    int       i, j, l;                            /* indicies */                          \
    int testchan;                                                                         \
                                                                                          \
    testchan = 1;                                                                         \
    for (l = chan - 1; l >= 0; l--) {                                                     \
      if ((mask & testchan) == 0) {                                                       \
        testchan <<= 1;                                                                   \
        continue;                                                                         \
      }                                                                                   \
      testchan <<= 1;                                                                     \
      for (j = 0; j < dx_l; j++) {                                                        \
        for (i = dy_t; i < (img_height - dy_b); i++) {                                    \
          pdst[i*dst_stride + l + j*chan] = psrc[i*src_stride + l + j*chan];              \
        }                                                                                 \
      }                                                                                   \
      for (j = 0; j < dx_r; j++) {                                                        \
        for (i = dy_t; i < (img_height - dy_b); i++) {                                    \
          pdst[i*dst_stride + l+(img_width-1 - j)*chan] =                                 \
          psrc[i*src_stride + l+(img_width-1 - j)*chan];                                  \
        }                                                                                 \
      }                                                                                   \
      for (i = 0; i < dy_t; i++) {                                                        \
        for (j = 0; j < img_width; j++) {                                                 \
          pdst[i*dst_stride + l + j*chan] = psrc[i*src_stride + l + j*chan];              \
        }                                                                                 \
      }                                                                                   \
      for (i = 0; i < dy_b; i++) {                                                        \
        for (j = 0; j < img_width; j++) {                                                 \
          pdst[(img_height-1 - i)*dst_stride + l + j*chan] =                              \
          psrc[(img_height-1 - i)*src_stride + l + j*chan];                               \
        }                                                                                 \
      }                                                                                   \
    }                                                                                     \
  }

/***************************************************************/

mlib_status mlib_ImageConvCopyEdge(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask)

{
  int channel;

#ifdef MLIB_TEST

  if (dst == NULL || mlib_ImageGetData(dst) == NULL) return MLIB_NULLPOINTER;
  if (dst == NULL || mlib_ImageGetData(dst) == NULL) return MLIB_NULLPOINTER;

  if (mlib_ImageGetWidth(dst) <= 0 || mlib_ImageGetHeight(dst) <= 0 ||
      mlib_ImageGetStride(dst) <= 0) return MLIB_FAILURE;

  if (mlib_ImageGetWidth(src) <= 0 || mlib_ImageGetHeight(src) <= 0 ||
      mlib_ImageGetStride(src) <= 0) return MLIB_FAILURE;

  if (mlib_ImageGetWidth(src)    !=  mlib_ImageGetWidth(dst) ||
      mlib_ImageGetHeight(src)   !=  mlib_ImageGetHeight(dst) ||
      mlib_ImageGetChannels(src) !=  mlib_ImageGetChannels(dst) ||
      mlib_ImageGetType(src)     !=  mlib_ImageGetType(dst))return MLIB_FAILURE;

  if (dx_l <= 0 || dx_r <= 0 || dy_t <= 0 || dy_b <= 0) return MLIB_FAILURE;
  if (((dx_l + dx_r) > mlib_ImageGetWidth(dst)) ||
      ((dy_t + dy_b) > mlib_ImageGetHeight(dst))) return MLIB_FAILURE;

#endif
  channel = mlib_ImageGetChannels(src);
  if (channel == 1) cmask = 1;
  switch (mlib_ImageGetType(src)) {

    case MLIB_BYTE:
      EDGES(channel, mlib_u8, cmask)
      break;
    case MLIB_SHORT:
      EDGES(channel, mlib_s16, cmask)
      break;
    case MLIB_INT:
      EDGES(channel, mlib_s32, cmask)
      break;
    default:
      return MLIB_FAILURE;
  }
  return MLIB_SUCCESS;
}

/***************************************************************/
