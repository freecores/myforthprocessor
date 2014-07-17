/*
 * @(#)mlib_ImageAffine_NN.c	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageAffine_NN.c	1.2	00/01/28 SMI"
#endif /* __SUNPRO_C */

#ifdef _MSC_VER
#pragma warning(disable:4068 4244 4101)
#endif /*_MSC_VER*/

/*
 * FUNCTION
 *      mlib_c_ImageAffine_bit_1ch_nn
 *        - image affine transformation on BIT image with Nearest Neighbor filtering
 * SYNOPSIS
 *      void mlib_c_ImageAffine_bit_1ch_nn
 *                                  (mlib_s32 *leftEdges,
 *                                   mlib_s32 *rightEdges,
 *                                   mlib_s32 *xStarts,
 *                                   mlib_s32 *yStarts,
 *                                   mlib_s32 *sides,
 *                                   mlib_u8  *dstData,
 *                                   mlib_u8  **lineAddr,
 *                                   mlib_s32 dstYStride)
 *
 * ARGUMENTS
 *      leftEdges  array[dstHeight] of xLeft coordinates
 *      RightEdges array[dstHeight] of xRight coordinates
 *      xStarts    array[dstHeight] of xStart * 65536 coordinates
 *      yStarts    array[dstHeight] of yStart * 65536 coordinates
 *      sides      output array[4]. sides[0] is yStart, sides[1] is yFinish,
 *                 sides[2] is dx * 65536, sides[3] is dy * 65536
 *      dstData    pointer to the first pixel on (yStart - 1) line
 *      lineAddr   array[srcHeight] of pointers to the first pixel on
 *                 the corresponding lines
 *      dstYStride stride of destination image
 *
 * DESCRIPTION
 *      The functions step along the lines from xLeft to xRight and get the
 *      nearest pixel values as being with the following coordinates
 *      ((xStart - (i - xLeft) * dx) >> 16, (yStart - (i - xLeft) * dy) >> 16).
 *
 */

#include "mlib_image.h"

/***************************************************************/

#ifdef MLIB_OS64BIT
#define MLIB_POINTER_SHIFT(P)  (((P) >> (MLIB_SHIFT-3)) &~ 7)
#define MLIB_POINTER_GET(A, P) (*(MLIB_TYPE **)((mlib_u8 *)(A) + (P)))
#else
#define MLIB_POINTER_SHIFT(P)  (((P) >> (MLIB_SHIFT-(sizeof(mlib_addr)/4+1))) &~ (sizeof(mlib_addr)-1))
#define MLIB_POINTER_GET(A, P) (*(MLIB_TYPE **)((mlib_addr)(A) + (P)))
#endif

#define MLIB_SHIFT   16

#define MLIB_TYPE mlib_u8

/***************************************************************/

#define MLIB_AFFINEDEFPARAM             \
  mlib_s32 *leftEdges,                  \
           mlib_s32 *rightEdges,        \
           mlib_s32 *xStarts,           \
           mlib_s32 *yStarts,           \
           mlib_s32 *sides,             \
           mlib_u8  *dstData,           \
           mlib_u8  **lineAddr,         \
           mlib_s32 dstYStride          \

/***************************************************************/

#define MLIB_AFFINECALLPARAM                            \
  leftEdges, rightEdges, xStarts, yStarts,              \
             sides, dstData, lineAddr, dstYStride

/***************************************************************/

#define DECLAREVAR                      \
  mlib_s32  xLeft, xRight, X, Y;        \
  mlib_s32  xSrc, ySrc;                 \
  mlib_s32  yStart = sides[0];          \
  mlib_s32  yFinish = sides[1];         \
  mlib_s32  dX = sides[2];              \
  mlib_s32  dY = sides[3];              \
  MLIB_TYPE *srcPixelPtr;               \
  MLIB_TYPE *srcPixelPtr0;              \
  MLIB_TYPE *srcPixelPtr1;              \
  MLIB_TYPE *srcPixelPtr2;              \
  MLIB_TYPE *srcPixelPtr3;              \
  MLIB_TYPE *srcPixelPtr4;              \
  MLIB_TYPE *srcPixelPtr5;              \
  MLIB_TYPE *srcPixelPtr6;              \
  MLIB_TYPE *srcPixelPtr7;              \
  MLIB_TYPE *dstPixelPtr;               \
  MLIB_TYPE *dstLineEnd;                \
  mlib_s32  j;

/***************************************************************/

#define CLIP()                                  \
  dstData += dstYStride;                        \
  xLeft = leftEdges[j];                         \
  xRight = rightEdges[j];                       \
  X = xStarts[j];                               \
  Y = yStarts[j];                               \
  if (xLeft > xRight) continue;                 \
  dstPixelPtr  = (MLIB_TYPE*)dstData + xLeft;   \
  dstLineEnd   = (MLIB_TYPE*)dstData + xRight

/***************************************************************/

void mlib_c_ImageAffine_bit_1ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR
  int i, bit, res;

  for (j = yStart; j <= yFinish; j++) {
    MLIB_TYPE pix0;

    CLIP();
    xRight++;

    i = xLeft;

    if (i & 7) {
      mlib_u8 *dp = dstData + (i >> 3);
      int res = dp[0];
      int i_end = i + (8 - (i & 7));

      if (i_end > xRight) i_end = xRight;

      for (; i < i_end; i++) {
        bit = 7 - (i & 7);
        ySrc = MLIB_POINTER_SHIFT(Y);
        srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc);

        res = (res &~ (1 << bit)) |
        (((srcPixelPtr[X >> (MLIB_SHIFT + 3)] >> (7 - (X >> MLIB_SHIFT) & 7)) & 1) << bit);

        X += dX;
        Y += dY;
      }

      dp[0] = res;
    }

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /*  __SUNPRO_C */
    for (; i <= (xRight - 8); i += 8) {
      srcPixelPtr0 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y)); Y += dY;
      res  = ((srcPixelPtr0[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT)    ) & 7)) & 0x0080); X += dX;

      srcPixelPtr1 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y)); Y += dY;
      res |= ((srcPixelPtr1[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 1) & 7)) & 0x4040); X += dX;

      srcPixelPtr2 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y)); Y += dY;
      res |= ((srcPixelPtr2[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 2) & 7)) & 0x2020); X += dX;

      srcPixelPtr3 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y)); Y += dY;
      res |= ((srcPixelPtr3[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 3) & 7)) & 0x1010); X += dX;

      srcPixelPtr4 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y)); Y += dY;
      res |= ((srcPixelPtr4[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 4) & 7)) & 0x0808); X += dX;

      srcPixelPtr5 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y)); Y += dY;
      res |= ((srcPixelPtr5[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 5) & 7)) & 0x0404); X += dX;

      srcPixelPtr6 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y)); Y += dY;
      res |= ((srcPixelPtr6[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 6) & 7)) & 0x0202); X += dX;

      srcPixelPtr7 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y)); Y += dY;
      res |= ((srcPixelPtr7[X >> (MLIB_SHIFT + 3)] >> (7 - ((X >> MLIB_SHIFT) & 7))) & 0x0001); X += dX;

      dstData[i >> 3] = res | (res >> 8);
    }

    if (i < xRight) {
      mlib_u8 *dp = dstData + (i >> 3);
      int res = dp[0];

      for (; i < xRight; i++) {
        bit = 7 - (i & 7);
        ySrc = MLIB_POINTER_SHIFT(Y);
        srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc);

        res = (res &~ (1 << bit)) |
        (((srcPixelPtr[X >> (MLIB_SHIFT + 3)] >> (7 - (X >> MLIB_SHIFT) & 7)) & 1) << bit);

        X += dX;
        Y += dY;
      }

      dp[0] = res;
    }
  }
}

/***************************************************************/
