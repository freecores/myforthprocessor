/*
 * @(#)mlib_c_ImageConvClearEdge.c	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageConvClearEdge.c	1.4	99/09/07 SMI"
#endif /* __SUNPRO_C */

/*
 * FILENAME: mlib_ImageConvClearEdge.c
 *
 * FUNCTIONS
 *      mlib_ImageConvClearEdge  - Set edge of an image to a specific
 *                                        color. (C version)
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageConvClearEdge(mlib_image *img,
 *                                            mlib_s32 dx_l, mlib_32 dx_r,
 *                                            mlib_s32 dy_t, mlib_32 dy_b,
 *                                            mlib_s32 *color,
 *                                            mlib_s32 cmask);
 *
 * ARGUMENT
 *      img       Pointer to an image.
 *      dx_l      Number of columns on the left side of the
 *                image to be cleared.
 *      dx_r      Number of columns on the right side of the
 *                image to be cleared.
 *      dy_t      Number of rows on the top edge of the
 *                image to be cleared.
 *      dy_b      Number of rows on the top edge of the
 *                image to be cleared.
 *      color     Pointer to the color that the edges are set to.
 *      cmask     Channel mask to indicate the channels to be convolved.
 *                Each bit of which represents a channel in the image. The
 *                channels corresponded to 1 bits are those to be processed.
 *
 * RESTRICTION
 *      img can have 1, 2, 3 or 4 channels of MLIB_BYTE or MLIB_SHORT or MLIB_INT
 *      data type.
 *
 * DESCRIPTION
 *      Set edge of an image to a specific color. (VIS version)
 *      The unselected channels are not overwritten.
 *      If src and dst have just one channel,
 *      cmask is ignored.
 */

#include <stdlib.h>
#include "mlib_image.h"

#define EDGES(chan, type, mask)                                                           \
{                                                                                         \
    type *pimg = (type *) mlib_ImageGetData(img); /* pointer to the data of img-image */  \
    type color_i;                                                                         \
    int img_height = mlib_ImageGetHeight(img);    /* height of source image */            \
    int img_width  = mlib_ImageGetWidth(img);     /* width of source image */             \
    int img_stride = mlib_ImageGetStride(img) / sizeof(type); /* elements to next row */  \
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
      color_i = (type)color[l];                                                           \
      for (j = 0; j < dx_l; j++) {                                                        \
        for (i = dy_t; i < (img_height - dy_b); i++) {                                    \
          pimg[i*img_stride + l + j*chan] = color_i;                                      \
        }                                                                                 \
      }                                                                                   \
      for (j = 0; j < dx_r; j++) {                                                        \
        for (i = dy_t; i < (img_height - dy_b); i++) {                                    \
          pimg[i*img_stride + l+(img_width-1 - j)*chan] = color_i;                        \
        }                                                                                 \
      }                                                                                   \
      for (i = 0; i < dy_t; i++) {                                                        \
        for (j = 0; j < img_width; j++) {                                                 \
          pimg[i*img_stride + l + j*chan] = color_i;                                      \
        }                                                                                 \
      }                                                                                   \
      for (i = 0; i < dy_b; i++) {                                                        \
        for (j = 0; j < img_width; j++) {                                                 \
          pimg[(img_height-1 - i)*img_stride + l + j*chan] = color_i;                     \
        }                                                                                 \
      }                                                                                   \
    }                                                                                     \
  }

/***************************************************************/

mlib_status mlib_ImageConvClearEdge(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                    mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)

{
  int channel;

#ifdef MLIB_TEST

  if (img == NULL || mlib_ImageGetData(img) == NULL || color == NULL) return MLIB_NULLPOINTER;

  if (mlib_ImageGetWidth(img) <= 0 || mlib_ImageGetHeight(img) <= 0 ||
      mlib_ImageGetStride(img) <= 0) return MLIB_FAILURE;

  if (dx_l <= 0 || dx_r <= 0 || dy_t <= 0 || dy_b <= 0) return MLIB_FAILURE;
  if (((dx_l + dx_r) > mlib_ImageGetWidth(img)) ||
      ((dy_t + dy_b) > mlib_ImageGetHeight(img))) return MLIB_FAILURE;

#endif
  channel = mlib_ImageGetChannels(img);
  if (channel == 1) cmask = 1;
  switch (mlib_ImageGetType(img)) {

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

