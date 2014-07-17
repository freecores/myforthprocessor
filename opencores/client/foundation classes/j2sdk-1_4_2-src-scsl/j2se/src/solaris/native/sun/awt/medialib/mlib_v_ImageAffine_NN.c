/*
 * @(#)mlib_v_ImageAffine_NN.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident  "@(#)mlib_v_ImageAffine_NN.c	1.6  99/03/26 SMI"

/*
 * FUNCTION
 *      mlib_v_ImageAffine_u8_1ch_nn
 *      mlib_v_ImageAffine_u8_2ch_nn
 *      mlib_v_ImageAffine_u8_3ch_nn
 *      mlib_v_ImageAffine_u8_4ch_nn
 *      mlib_v_ImageAffine_s16_1ch_nn
 *      mlib_v_ImageAffine_s16_2ch_nn
 *      mlib_v_ImageAffine_s16_3ch_nn
 *      mlib_v_ImageAffine_s16_4ch_nn
 *      mlib_ImageAffine_s32_1ch_nn
 *      mlib_ImageAffine_s32_2ch_nn
 *      mlib_ImageAffine_s32_3ch_nn
 *      mlib_ImageAffine_s32_4ch_nn
 *        - image affine transformation with Nearest Neighbor filtering
 * SYNOPSIS
 *      void mlib_v_ImageAffine_[u8|s16|s32]_?ch_nn
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
 *      ((xStart - (i - xLeft) * dx) >> 16, (yStart - (i - xLeft) * dy) >> 16)
 *      .
 *
 */

#include "mlib_image.h"

/***************************************************************/

#ifdef MLIB_OS64BIT
#define MLIB_POINTER_SHIFT(P) (P >> (MLIB_SHIFT-3)) &~ 7
#define MLIB_POINTER_GET(A,P) (*(MLIB_TYPE **)((mlib_u8 *)A + P))
#else
#define MLIB_POINTER_SHIFT(P) (P >> (MLIB_SHIFT-2)) &~ 3
#define MLIB_POINTER_GET(A,P) (*(MLIB_TYPE **)((mlib_addr)A + P))
#endif

#define MLIB_SHIFT   16

/***************************************************************/

#define MLIB_AFFINEDEFPARAM \
  mlib_s32 *leftEdges,      \
  mlib_s32 *rightEdges,     \
  mlib_s32 *xStarts,        \
  mlib_s32 *yStarts,        \
  mlib_s32 *sides,          \
  mlib_u8  *dstData,        \
  mlib_u8  **lineAddr,      \
  mlib_s32 dstYStride       \

void mlib_v_ImageAffine_u8_1ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_v_ImageAffine_u8_2ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_v_ImageAffine_u8_3ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_v_ImageAffine_u8_4ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_v_ImageAffine_s16_1ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_v_ImageAffine_s16_2ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_v_ImageAffine_s16_3ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_v_ImageAffine_s16_4ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_ImageAffine_s32_1ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_ImageAffine_s32_2ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_ImageAffine_s32_3ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_ImageAffine_s32_4ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_ImageAffine_d64_1ch_nn(MLIB_AFFINEDEFPARAM);
void mlib_ImageAffine_d64_2ch_nn(MLIB_AFFINEDEFPARAM);

/***************************************************************/

#define MLIB_AFFINECALLPARAM               \
  leftEdges, rightEdges, xStarts, yStarts, \
  sides, dstData, lineAddr, dstYStride

/***************************************************************/

#define DECLAREVAR               \
  mlib_s32  xLeft, xRight, X, Y; \
  mlib_s32  xSrc, ySrc;          \
  mlib_s32  yStart = sides[0];   \
  mlib_s32  yFinish = sides[1];  \
  mlib_s32  dX = sides[2];       \
  mlib_s32  dY = sides[3];       \
  MLIB_TYPE *srcPixelPtr;        \
  MLIB_TYPE *dstPixelPtr;        \
  MLIB_TYPE *dstLineEnd;         \
  mlib_s32  j;

/***************************************************************/

#define CLIP(N)                                   \
  dstData += dstYStride;                          \
  xLeft = leftEdges[j];                           \
  xRight = rightEdges[j];                         \
  X = xStarts[j];                                 \
  Y = yStarts[j];                                 \
  if (xLeft > xRight) continue;                   \
  dstPixelPtr  = (MLIB_TYPE*)dstData + N * xLeft; \
  dstLineEnd   = (MLIB_TYPE*)dstData + N * xRight;


/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_u8

void mlib_v_ImageAffine_u8_1ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  for (j = yStart; j <= yFinish; j++)
  {
    MLIB_TYPE pix0;

    CLIP(1)

#pragma pipeloop(0)
    for (;dstPixelPtr <= dstLineEnd; dstPixelPtr++)
    {
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc);
      xSrc = X>>MLIB_SHIFT;  X += dX;
      pix0 = srcPixelPtr[xSrc];
      dstPixelPtr[0] = pix0;
    }
  }
}

/***************************************************************/

void mlib_v_ImageAffine_u8_2ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  mlib_u8 *srcData = lineAddr[0];
  mlib_s32 srcYStride = lineAddr[1] - srcData;
  if ((((mlib_addr)srcData | (mlib_addr) dstData |
       srcYStride | dstYStride) & 1) == 0) {
    mlib_v_ImageAffine_s16_1ch_nn(MLIB_AFFINECALLPARAM);
    return;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    MLIB_TYPE pix0, pix1;

    CLIP(2)

    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
    pix0 = srcPixelPtr[0];
    pix1 = srcPixelPtr[1];
    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
#pragma pipeloop(0)
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr+=2)
    {
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
      ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
      xSrc = X>>MLIB_SHIFT; X += dX;
      dstPixelPtr[0] = pix0;
      dstPixelPtr[1] = pix1;
      pix0 = srcPixelPtr[0];
      pix1 = srcPixelPtr[1];
    }
    dstPixelPtr[0] = pix0;
    dstPixelPtr[1] = pix1;
  }
}

/***************************************************************/

void mlib_v_ImageAffine_u8_3ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  for (j = yStart; j <= yFinish; j++)
  {
    MLIB_TYPE pix0, pix1, pix2;

    CLIP(3)

    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 3*xSrc;
    pix0 = srcPixelPtr[0];
    pix1 = srcPixelPtr[1];
    pix2 = srcPixelPtr[2];
    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
#pragma pipeloop(0)
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr+=3)
    {
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 3*xSrc;
      ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
      xSrc = X>>MLIB_SHIFT; X += dX;
      dstPixelPtr[0] = pix0;
      dstPixelPtr[1] = pix1;
      dstPixelPtr[2] = pix2;
      pix0 = srcPixelPtr[0];
      pix1 = srcPixelPtr[1];
      pix2 = srcPixelPtr[2];
    }
    dstPixelPtr[0] = pix0;
    dstPixelPtr[1] = pix1;
    dstPixelPtr[2] = pix2;
  }
}

/***************************************************************/

void mlib_v_ImageAffine_u8_4ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  mlib_u8 *srcData = lineAddr[0];
  mlib_s32 srcYStride = lineAddr[1] - srcData;
  if ((((mlib_addr)srcData | (mlib_addr) dstData |
       srcYStride | dstYStride) & 1) == 0) {
    mlib_v_ImageAffine_s16_2ch_nn(MLIB_AFFINECALLPARAM);
    return;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    MLIB_TYPE pix0, pix1, pix2, pix3;
    CLIP(4)

    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 4*xSrc;
    pix0 = srcPixelPtr[0];
    pix1 = srcPixelPtr[1];
    pix2 = srcPixelPtr[2];
    pix3 = srcPixelPtr[3];
    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr+=4)
    {
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 4*xSrc;
      ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
      xSrc = X>>MLIB_SHIFT; X += dX;
      dstPixelPtr[0] = pix0;
      dstPixelPtr[1] = pix1;
      dstPixelPtr[2] = pix2;
      dstPixelPtr[3] = pix3;
      pix0 = srcPixelPtr[0];
      pix1 = srcPixelPtr[1];
      pix2 = srcPixelPtr[2];
      pix3 = srcPixelPtr[3];
    }
    dstPixelPtr[0] = pix0;
    dstPixelPtr[1] = pix1;
    dstPixelPtr[2] = pix2;
    dstPixelPtr[3] = pix3;
  }
}

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_u16

void mlib_v_ImageAffine_s16_1ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32 pix0;

    CLIP(1)

    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X>>MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc);
    pix0 = srcPixelPtr[xSrc];
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr++)
    {
      xSrc = X>>MLIB_SHIFT;  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc);
      dstPixelPtr[0] = pix0;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      pix0 = srcPixelPtr[xSrc];
    }
    dstPixelPtr[0] = pix0;
  }
}

/***************************************************************/

void mlib_v_ImageAffine_s16_2ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  mlib_u8 *srcData = lineAddr[0];
  mlib_s32 srcYStride = lineAddr[1] - srcData;
  if ((((mlib_addr)srcData | (mlib_addr) dstData |
       srcYStride | dstYStride) & 3) == 0) {
    mlib_ImageAffine_s32_1ch_nn(MLIB_AFFINECALLPARAM);
    return;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32 pix0, pix1;

    CLIP(2)

    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
    pix0 = srcPixelPtr[0];
    pix1 = srcPixelPtr[1];
    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
#pragma pipeloop(0)
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr+=2)
    {
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
      ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
      xSrc = X>>MLIB_SHIFT; X += dX;
      dstPixelPtr[0] = pix0;
      dstPixelPtr[1] = pix1;
      pix0 = srcPixelPtr[0];
      pix1 = srcPixelPtr[1];
    }
    dstPixelPtr[0] = pix0;
    dstPixelPtr[1] = pix1;
  }
}

/***************************************************************/

void mlib_v_ImageAffine_s16_3ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32 pix0, pix1, pix2;

    CLIP(3)

    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 3*xSrc;
    pix0 = srcPixelPtr[0];
    pix1 = srcPixelPtr[1];
    pix2 = srcPixelPtr[2];
    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
#pragma pipeloop(0)
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr+=3)
    {
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 3*xSrc;
      ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
      xSrc = X>>MLIB_SHIFT; X += dX;
      dstPixelPtr[0] = pix0;
      dstPixelPtr[1] = pix1;
      dstPixelPtr[2] = pix2;
      pix0 = srcPixelPtr[0];
      pix1 = srcPixelPtr[1];
      pix2 = srcPixelPtr[2];
    }
    dstPixelPtr[0] = pix0;
    dstPixelPtr[1] = pix1;
    dstPixelPtr[2] = pix2;
  }
}

/***************************************************************/

void mlib_v_ImageAffine_s16_4ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  mlib_u8 *srcData = lineAddr[0];
  mlib_s32 srcYStride = lineAddr[1] - srcData;
  if ((((mlib_addr)srcData | (mlib_addr) dstData |
       srcYStride | dstYStride) & 3) == 0) {
    mlib_ImageAffine_s32_2ch_nn(MLIB_AFFINECALLPARAM);
    return;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32 pix0, pix1, pix2, pix3;
    CLIP(4)

    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 4*xSrc;
    pix0 = srcPixelPtr[0];
    pix1 = srcPixelPtr[1];
    pix2 = srcPixelPtr[2];
    pix3 = srcPixelPtr[3];
    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr+=4)
    {
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 4*xSrc;
      ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
      xSrc = X>>MLIB_SHIFT; X += dX;
      dstPixelPtr[0] = pix0;
      dstPixelPtr[1] = pix1;
      dstPixelPtr[2] = pix2;
      dstPixelPtr[3] = pix3;
      pix0 = srcPixelPtr[0];
      pix1 = srcPixelPtr[1];
      pix2 = srcPixelPtr[2];
      pix3 = srcPixelPtr[3];
    }
    dstPixelPtr[0] = pix0;
    dstPixelPtr[1] = pix1;
    dstPixelPtr[2] = pix2;
    dstPixelPtr[3] = pix3;
  }
}

/***************************************************************/
#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s32

void mlib_ImageAffine_s32_1ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32 pix0;

    CLIP(1)

    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X>>MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc);
    pix0 = srcPixelPtr[xSrc];
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;

#pragma pipeloop(0)
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr++)
    {
      xSrc = X>>MLIB_SHIFT;  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc);
      dstPixelPtr[0] = pix0;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      pix0 = srcPixelPtr[xSrc];
    }
    dstPixelPtr[0] = pix0;
  }
}

/***************************************************************/

void mlib_ImageAffine_s32_2ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  mlib_u8 *srcData = lineAddr[0];
  mlib_s32 srcYStride = lineAddr[1] - srcData;
  if ((((mlib_addr)srcData | (mlib_addr) dstData |
       srcYStride | dstYStride) & 7) == 0) {
    mlib_ImageAffine_d64_1ch_nn(MLIB_AFFINECALLPARAM);
    return;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32 pix0, pix1;

    CLIP(2)

    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
    pix0 = srcPixelPtr[0];
    pix1 = srcPixelPtr[1];
    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;

#pragma pipeloop(0)
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr+=2)
    {
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
      ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
      xSrc = X>>MLIB_SHIFT; X += dX;
      dstPixelPtr[0] = pix0;
      dstPixelPtr[1] = pix1;
      pix0 = srcPixelPtr[0];
      pix1 = srcPixelPtr[1];
    }
    dstPixelPtr[0] = pix0;
    dstPixelPtr[1] = pix1;
  }
}

/***************************************************************/

void mlib_ImageAffine_s32_3ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32 pix0, pix1, pix2;

    CLIP(3)

    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 3*xSrc;
    pix0 = srcPixelPtr[0];
    pix1 = srcPixelPtr[1];
    pix2 = srcPixelPtr[2];
    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;

#pragma pipeloop(0)
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr+=3)
    {
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 3*xSrc;
      ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
      xSrc = X>>MLIB_SHIFT; X += dX;
      dstPixelPtr[0] = pix0;
      dstPixelPtr[1] = pix1;
      dstPixelPtr[2] = pix2;
      pix0 = srcPixelPtr[0];
      pix1 = srcPixelPtr[1];
      pix2 = srcPixelPtr[2];
    }
    dstPixelPtr[0] = pix0;
    dstPixelPtr[1] = pix1;
    dstPixelPtr[2] = pix2;
  }
}

/***************************************************************/

void mlib_ImageAffine_s32_4ch_nn(MLIB_AFFINEDEFPARAM)
{
  DECLAREVAR

  mlib_u8 *srcData = lineAddr[0];
  mlib_s32 srcYStride = lineAddr[1] - srcData;
  if ((((mlib_addr)srcData | (mlib_addr) dstData |
       srcYStride | dstYStride) & 7) == 0) {
    mlib_ImageAffine_d64_2ch_nn(MLIB_AFFINECALLPARAM);
    return;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32 pix0, pix1, pix2, pix3;
    CLIP(4)

    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 4*xSrc;
    pix0 = srcPixelPtr[0];
    pix1 = srcPixelPtr[1];
    pix2 = srcPixelPtr[2];
    pix3 = srcPixelPtr[3];
    ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
    xSrc = X>>MLIB_SHIFT; X += dX;
    for (;dstPixelPtr < dstLineEnd; dstPixelPtr+=4)
    {
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 4*xSrc;
      ySrc = MLIB_POINTER_SHIFT(Y); Y += dY;
      xSrc = X>>MLIB_SHIFT; X += dX;
      dstPixelPtr[0] = pix0;
      dstPixelPtr[1] = pix1;
      dstPixelPtr[2] = pix2;
      dstPixelPtr[3] = pix3;
      pix0 = srcPixelPtr[0];
      pix1 = srcPixelPtr[1];
      pix2 = srcPixelPtr[2];
      pix3 = srcPixelPtr[3];
    }
    dstPixelPtr[0] = pix0;
    dstPixelPtr[1] = pix1;
    dstPixelPtr[2] = pix2;
    dstPixelPtr[3] = pix3;
  }
}

/***************************************************************/
