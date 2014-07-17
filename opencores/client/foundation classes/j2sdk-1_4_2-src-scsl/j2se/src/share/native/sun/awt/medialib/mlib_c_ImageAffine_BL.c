/*
 * @(#)mlib_c_ImageAffine_BL.c	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageAffine_BL.c	1.13	00/01/31 SMI"
#endif /* __SUNPRO_C */

/*
 * FUNCTION
 *      mlib_c_ImageAffine_u8_1ch_bl
 *      mlib_c_ImageAffine_u8_2ch_bl
 *      mlib_c_ImageAffine_u8_3ch_bl
 *      mlib_c_ImageAffine_u8_4ch_bl
 *      mlib_c_ImageAffine_s16_1ch_bl
 *      mlib_c_ImageAffine_s16_2ch_bl
 *      mlib_c_ImageAffine_s16_3ch_bl
 *      mlib_c_ImageAffine_s16_4ch_bl
 *      mlib_c_ImageAffine_s32_1ch_bl
 *      mlib_c_ImageAffine_s32_2ch_bl
 *      mlib_c_ImageAffine_s32_3ch_bl
 *      mlib_c_ImageAffine_s32_4ch_bl
 *        - image affine transformation with Bilinear filtering
 * SYNOPSIS
 *      void mlib_c_ImageAffine_[u8|s16|s32]_?ch_bl
 *                                  (mlib_s32 *leftEdges,
 *                                   mlib_s32 *rightEdges,
 *                                   mlib_s32 *xStarts,
 *                                   mlib_s32 *yStarts,
 *                                   mlib_s32 *sides,
 *                                   mlib_u8  *dstData,
 *                                   mlib_u8  **lineAddr,
 *                                   mlib_s32 dstYStride,
 *                                   mlib_s32 srcYStride)
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
 *      srcYStride stride of source image
 *
 * DESCRIPTION
 *      The functions step along the lines from xLeft to xRight and apply
 *      the bilinear filtering.
 *
 */

#include <stdlib.h>
#include "mlib_image.h"
#include "mlib_ImageFilters.h"

#ifdef MLIB_OS64BIT
#define MLIB_POINTER_SHIFT(P) (P >> (MLIB_SHIFT-3)) &~ 7
#define MLIB_POINTER_GET(A, P) (*(MLIB_TYPE **)((mlib_u8 *)A + P))
#else
#define MLIB_POINTER_SHIFT(P) (P >> (MLIB_SHIFT-2)) &~ 3
#define MLIB_POINTER_GET(A, P) (*(MLIB_TYPE **)((mlib_addr)A + P))
#endif

/***************************************************************/

#define MLIB_SHIFT   16
#define MLIB_PREC    (1 << MLIB_SHIFT)
#define MLIB_MASK    (MLIB_PREC - 1)
#define MLIB_ROUND   (MLIB_PREC >> 1)

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

#define MLIB_AFFINEDEFPARAM2 MLIB_AFFINEDEFPARAM, mlib_s32 srcYStride

/***************************************************************/

#define MLIB_AFFINECALLPARAM                            \
  leftEdges, rightEdges, xStarts, yStarts,              \
             sides, dstData, lineAddr, dstYStride

#define MLIB_AFFINECALLPARAM2 MLIB_AFFINECALLPARAM, srcYStride

/***************************************************************/

#define DECLAREVAR                      \
  mlib_s32  xLeft, xRight, X, Y;        \
  mlib_s32  xSrc, ySrc;                 \
  mlib_s32  yStart = sides[0];          \
  mlib_s32  yFinish = sides[1];         \
  mlib_s32  dX = sides[2];              \
  mlib_s32  dY = sides[3];              \
  MLIB_TYPE *srcPixelPtr;               \
  MLIB_TYPE *dstPixelPtr;               \
  MLIB_TYPE *dstLineEnd;                \
  mlib_s32  j;

/***************************************************************/

#define CLIP(N)                                         \
  dstData += dstYStride;                                \
  xLeft = leftEdges[j];                                 \
  xRight = rightEdges[j];                               \
  X = xStarts[j];                                       \
  Y = yStarts[j];                                       \
  if (xLeft > xRight) continue;                         \
  dstPixelPtr  = (MLIB_TYPE*)dstData + N * xLeft;       \
  dstLineEnd   = (MLIB_TYPE*)dstData + N * xRight;

/***************************************************************/

#define MLIB_TYPE mlib_u8

#ifdef __sparc /* for SPARC, using floating-point multiplies is faster */

/***************************************************************/

#define GET_POINTERS_U8(ind)                                          \
  fdx = (mlib_f32)(X & MLIB_MASK) * scale;                            \
  fdy = (mlib_f32)(Y & MLIB_MASK) * scale;                            \
  ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;                             \
  xSrc = X >> MLIB_SHIFT;  X += dX;                                   \
  srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + ind * xSrc;        \
  srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);

/***************************************************************/

#define COUNT_U8(ind)                                                   \
  pix0_##ind = a00_##ind + fdy * (a10_##ind - a00_##ind);               \
  pix1_##ind = a01_##ind + fdy * (a11_##ind - a01_##ind);               \
  res##ind = pix0_##ind + fdx * (pix1_##ind - pix0_##ind) + 0.5f;

/***************************************************************/

#define LOAD_U8(ind, ind1, ind2)                \
  a00_##ind = mlib_U82F32[srcPixelPtr[ind1]];   \
  a01_##ind = mlib_U82F32[srcPixelPtr[ind2]];   \
  a10_##ind = mlib_U82F32[srcPixelPtr2[ind1]];  \
  a11_##ind = mlib_U82F32[srcPixelPtr2[ind2]];

/***************************************************************/

void mlib_c_ImageAffine_u8_1ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;
  mlib_f32 scale = 1.f / (mlib_f32) MLIB_PREC;

  for (j = yStart; j <= yFinish; j++) {
    mlib_f32  fdx, fdy;
    mlib_f32  a00_0, a01_0, a10_0, a11_0;
    mlib_f32  pix0_0, pix1_0, res0;

    CLIP(1)

    GET_POINTERS_U8(1)
    LOAD_U8(0, 0, 1)

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dstPixelPtr < dstLineEnd; dstPixelPtr++) {
      COUNT_U8(0)
      GET_POINTERS_U8(1)
      LOAD_U8(0, 0, 1)
      dstPixelPtr[0] = (MLIB_TYPE) res0;
    }
    COUNT_U8(0)
    dstPixelPtr[0] = (MLIB_TYPE) res0;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_u8_2ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;
  mlib_f32 scale = 1.f / (mlib_f32) MLIB_PREC;

  for (j = yStart; j <= yFinish; j++) {
    mlib_f32  fdx, fdy;
    mlib_f32  a00_0, a01_0, a10_0, a11_0;
    mlib_f32  a00_1, a01_1, a10_1, a11_1;
    mlib_f32  pix0_0, pix1_0, res0;
    mlib_f32  pix0_1, pix1_1, res1;

    CLIP(2)

    GET_POINTERS_U8(2)
    LOAD_U8(0, 0, 2)
    LOAD_U8(1, 1, 3)

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 2) {
      COUNT_U8(0)
      COUNT_U8(1)
      GET_POINTERS_U8(2)
      LOAD_U8(0, 0, 2)
      LOAD_U8(1, 1, 3)
      dstPixelPtr[0] = (MLIB_TYPE) res0;
      dstPixelPtr[1] = (MLIB_TYPE) res1;
    }
    COUNT_U8(0)
    COUNT_U8(1)
    dstPixelPtr[0] = (MLIB_TYPE) res0;
    dstPixelPtr[1] = (MLIB_TYPE) res1;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_u8_3ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;
  mlib_f32 scale = 1.f / (mlib_f32) MLIB_PREC;

  for (j = yStart; j <= yFinish; j++) {
    mlib_f32  fdx, fdy;
    mlib_f32  a00_0, a01_0, a10_0, a11_0;
    mlib_f32  a00_1, a01_1, a10_1, a11_1;
    mlib_f32  a00_2, a01_2, a10_2, a11_2;
    mlib_f32  pix0_0, pix1_0, res0;
    mlib_f32  pix0_1, pix1_1, res1;
    mlib_f32  pix0_2, pix1_2, res2;

    CLIP(3)

    GET_POINTERS_U8(3)
    LOAD_U8(0, 0, 3)
    LOAD_U8(1, 1, 4)
    LOAD_U8(2, 2, 5)

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 3) {
      COUNT_U8(0)
      COUNT_U8(1)
      COUNT_U8(2)
      GET_POINTERS_U8(3)
      LOAD_U8(0, 0, 3)
      LOAD_U8(1, 1, 4)
      LOAD_U8(2, 2, 5)
      dstPixelPtr[0] = (MLIB_TYPE) res0;
      dstPixelPtr[1] = (MLIB_TYPE) res1;
      dstPixelPtr[2] = (MLIB_TYPE) res2;
    }
    COUNT_U8(0)
    COUNT_U8(1)
    COUNT_U8(2)
    dstPixelPtr[0] = (MLIB_TYPE) res0;
    dstPixelPtr[1] = (MLIB_TYPE) res1;
    dstPixelPtr[2] = (MLIB_TYPE) res2;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_u8_4ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;
  mlib_f32 scale = 1.f / (mlib_f32) MLIB_PREC;

  for (j = yStart; j <= yFinish; j++) {
    mlib_f32  fdx, fdy;
    mlib_f32  a00_0, a01_0, a10_0, a11_0;
    mlib_f32  a00_1, a01_1, a10_1, a11_1;
    mlib_f32  a00_2, a01_2, a10_2, a11_2;
    mlib_f32  a00_3, a01_3, a10_3, a11_3;
    mlib_f32  pix0_0, pix1_0, res0;
    mlib_f32  pix0_1, pix1_1, res1;
    mlib_f32  pix0_2, pix1_2, res2;
    mlib_f32  pix0_3, pix1_3, res3;

    CLIP(4)

    GET_POINTERS_U8(4)
    LOAD_U8(0, 0, 4)
    LOAD_U8(1, 1, 5)
    LOAD_U8(2, 2, 6)
    LOAD_U8(3, 3, 7)

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 4) {
      COUNT_U8(0)
      COUNT_U8(1)
      COUNT_U8(2)
      COUNT_U8(3)
      GET_POINTERS_U8(4)
      LOAD_U8(0, 0, 4)
      LOAD_U8(1, 1, 5)
      LOAD_U8(2, 2, 6)
      LOAD_U8(3, 3, 7)
      dstPixelPtr[0] = (MLIB_TYPE) res0;
      dstPixelPtr[1] = (MLIB_TYPE) res1;
      dstPixelPtr[2] = (MLIB_TYPE) res2;
      dstPixelPtr[3] = (MLIB_TYPE) res3;
    }
    COUNT_U8(0)
    COUNT_U8(1)
    COUNT_U8(2)
    COUNT_U8(3)
    dstPixelPtr[0] = (MLIB_TYPE) res0;
    dstPixelPtr[1] = (MLIB_TYPE) res1;
    dstPixelPtr[2] = (MLIB_TYPE) res2;
    dstPixelPtr[3] = (MLIB_TYPE) res3;
  }
}

/***************************************************************/

#else       /* for x86, using integer multiplies is faster */

#define GET_POINTERS_U8(ind)                                          \
  fdx = X & MLIB_MASK;                                                \
  fdy = Y & MLIB_MASK;                                                \
  ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;                             \
  xSrc = X >> MLIB_SHIFT;  X += dX;                                   \
  srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + ind * xSrc;        \
  srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);

/***************************************************************/

#define COUNT_U8(ind)                                                                      \
  pix0_##ind = a00_##ind + ((fdy * (a10_##ind - a00_##ind) + MLIB_ROUND) >> MLIB_SHIFT);   \
  pix1_##ind = a01_##ind + ((fdy * (a11_##ind - a01_##ind) + MLIB_ROUND) >> MLIB_SHIFT);   \
  res##ind = pix0_##ind + ((fdx * (pix1_##ind - pix0_##ind) + MLIB_ROUND) >> MLIB_SHIFT);

/***************************************************************/

#define LOAD_U8(ind, ind1, ind2)        \
  a00_##ind = srcPixelPtr[ind1];        \
  a01_##ind = srcPixelPtr[ind2];        \
  a10_##ind = srcPixelPtr2[ind1];       \
  a11_##ind = srcPixelPtr2[ind2];

/***************************************************************/

void mlib_c_ImageAffine_u8_1ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;

  for (j = yStart; j <= yFinish; j++) {
    mlib_s32  fdx, fdy;
    mlib_s32  a00_0, a01_0, a10_0, a11_0;
    mlib_s32  pix0_0, pix1_0, res0;

    CLIP(1)

    GET_POINTERS_U8(1)
    LOAD_U8(0, 0, 1)

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dstPixelPtr < dstLineEnd; dstPixelPtr++) {
      COUNT_U8(0)
      GET_POINTERS_U8(1)
      LOAD_U8(0, 0, 1)
      dstPixelPtr[0] = (MLIB_TYPE) res0;
    }
    COUNT_U8(0)
    dstPixelPtr[0] = (MLIB_TYPE) res0;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_u8_2ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;

  for (j = yStart; j <= yFinish; j++) {
    mlib_s32  fdx, fdy;
    mlib_s32  a00_0, a01_0, a10_0, a11_0;
    mlib_s32  a00_1, a01_1, a10_1, a11_1;
    mlib_s32  pix0_0, pix1_0, res0;
    mlib_s32  pix0_1, pix1_1, res1;

    CLIP(2)

    GET_POINTERS_U8(2)
    LOAD_U8(0, 0, 2)
    LOAD_U8(1, 1, 3)

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 2) {
      COUNT_U8(0)
      COUNT_U8(1)
      GET_POINTERS_U8(2)
      LOAD_U8(0, 0, 2)
      LOAD_U8(1, 1, 3)
      dstPixelPtr[0] = (MLIB_TYPE) res0;
      dstPixelPtr[1] = (MLIB_TYPE) res1;
    }
    COUNT_U8(0)
    COUNT_U8(1)
    dstPixelPtr[0] = (MLIB_TYPE) res0;
    dstPixelPtr[1] = (MLIB_TYPE) res1;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_u8_3ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;

  for (j = yStart; j <= yFinish; j++) {
    mlib_s32  fdx, fdy;
    mlib_s32  a00_0, a01_0, a10_0, a11_0;
    mlib_s32  a00_1, a01_1, a10_1, a11_1;
    mlib_s32  a00_2, a01_2, a10_2, a11_2;
    mlib_s32  pix0_0, pix1_0, res0;
    mlib_s32  pix0_1, pix1_1, res1;
    mlib_s32  pix0_2, pix1_2, res2;

    CLIP(3)

    GET_POINTERS_U8(3)
    LOAD_U8(0, 0, 3)
    LOAD_U8(1, 1, 4)
    LOAD_U8(2, 2, 5)

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 3) {
      COUNT_U8(0)
      COUNT_U8(1)
      COUNT_U8(2)
      GET_POINTERS_U8(3)
      LOAD_U8(0, 0, 3)
      LOAD_U8(1, 1, 4)
      LOAD_U8(2, 2, 5)
      dstPixelPtr[0] = (MLIB_TYPE) res0;
      dstPixelPtr[1] = (MLIB_TYPE) res1;
      dstPixelPtr[2] = (MLIB_TYPE) res2;
    }
    COUNT_U8(0)
    COUNT_U8(1)
    COUNT_U8(2)
    dstPixelPtr[0] = (MLIB_TYPE) res0;
    dstPixelPtr[1] = (MLIB_TYPE) res1;
    dstPixelPtr[2] = (MLIB_TYPE) res2;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_u8_4ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;

  for (j = yStart; j <= yFinish; j++) {
    mlib_s32  fdx, fdy;
    mlib_s32  a00_0, a01_0, a10_0, a11_0;
    mlib_s32  a00_1, a01_1, a10_1, a11_1;
    mlib_s32  a00_2, a01_2, a10_2, a11_2;
    mlib_s32  a00_3, a01_3, a10_3, a11_3;
    mlib_s32  pix0_0, pix1_0, res0;
    mlib_s32  pix0_1, pix1_1, res1;
    mlib_s32  pix0_2, pix1_2, res2;
    mlib_s32  pix0_3, pix1_3, res3;

    CLIP(4)

    GET_POINTERS_U8(4)
    LOAD_U8(0, 0, 4)
    LOAD_U8(1, 1, 5)
    LOAD_U8(2, 2, 6)
    LOAD_U8(3, 3, 7)

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 4) {
      COUNT_U8(0)
      COUNT_U8(1)
      COUNT_U8(2)
      COUNT_U8(3)
      GET_POINTERS_U8(4)
      LOAD_U8(0, 0, 4)
      LOAD_U8(1, 1, 5)
      LOAD_U8(2, 2, 6)
      LOAD_U8(3, 3, 7)
      dstPixelPtr[0] = (MLIB_TYPE) res0;
      dstPixelPtr[1] = (MLIB_TYPE) res1;
      dstPixelPtr[2] = (MLIB_TYPE) res2;
      dstPixelPtr[3] = (MLIB_TYPE) res3;
    }
    COUNT_U8(0)
    COUNT_U8(1)
    COUNT_U8(2)
    COUNT_U8(3)
    dstPixelPtr[0] = (MLIB_TYPE) res0;
    dstPixelPtr[1] = (MLIB_TYPE) res1;
    dstPixelPtr[2] = (MLIB_TYPE) res2;
    dstPixelPtr[3] = (MLIB_TYPE) res3;
  }
}

#endif /* #ifdef __sparc */

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s16

#ifdef __sparc /* for SPARC, using floating-point multiplies is faster */

void mlib_c_ImageAffine_s16_1ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 scale = 1. / (mlib_d64) MLIB_PREC;
  mlib_s32 srcYStride1 = srcYStride+2;

  srcYStride >>= 1;
  srcYStride1 >>= 1;
  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  t, u, k, k0;
    mlib_s32  a00_0, a01_0, a10_0, a11_0;

    CLIP(1)

    t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + xSrc;
    a00_0 = srcPixelPtr[0]; k = t * u;
    a01_0 = srcPixelPtr[1];
    a10_0 = srcPixelPtr[srcYStride]; k0 = (a01_0 - a00_0) * t;
    a11_0 = srcPixelPtr[srcYStride1]; k0 += (a10_0 - a00_0) * u;

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
    {
      mlib_d64 pix0;
      mlib_s32 t0;

      pix0 = k0 + (a00_0 - a01_0 + a11_0 - a10_0) * k;
      t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> MLIB_SHIFT;  X += dX;
      srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + ySrc) + xSrc;
      t0 = a00_0 + (mlib_s32)pix0;
      a00_0 = srcPixelPtr[0]; k = t * u;
      a01_0 = srcPixelPtr[1];
      a10_0 = srcPixelPtr[srcYStride]; k0 = (a01_0 - a00_0) * t;
      a11_0 = srcPixelPtr[srcYStride1]; k0 += (a10_0 - a00_0) * u;
      dstPixelPtr[0] = (mlib_s16)t0;
    }
    k0 += (a00_0 - a01_0 + a11_0 - a10_0) * k;
    dstPixelPtr[0] = a00_0 + (mlib_s32)k0;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_s16_2ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 scale = 1. / (mlib_d64) MLIB_PREC;

  for (j = yStart; j <= yFinish; j++)
  {
    MLIB_TYPE *srcPixelPtr2;
    mlib_d64  t, u, k, k0, k1;
    mlib_s32  a00_0, a01_0, a10_0, a11_0;
    mlib_s32  a00_1, a01_1, a10_1, a11_1;

    CLIP(2)

    t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
    srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
    a00_0 = srcPixelPtr[0];
    a00_1 = srcPixelPtr[1];  k = t * u;
    a01_0 = srcPixelPtr[2];
    a01_1 = srcPixelPtr[3];
    a10_0 = srcPixelPtr2[0]; k0 = (a01_0 - a00_0) * t;
    a10_1 = srcPixelPtr2[1]; k1 = (a01_1 - a00_1) * t;
    a11_0 = srcPixelPtr2[2]; k0 += (a10_0 - a00_0) * u;
    a11_1 = srcPixelPtr2[3]; k1 += (a10_1 - a00_1) * u;

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 2)
    {
      mlib_d64 pix0, pix1;
      mlib_s32 t0, t1;

      pix0 = (a00_0 - a01_0 + a11_0 - a10_0) * k;
      pix1 = (a00_1 - a01_1 + a11_1 - a10_1) * k;
      pix0 += k0;
      pix1 += k1;
      t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> MLIB_SHIFT;  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
      srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
      a01_0 = srcPixelPtr[2];
      a01_1 = srcPixelPtr[3];
      t0 = a00_0 + (mlib_s32)pix0;
      t1 = a00_1 + (mlib_s32)pix1;
      a00_0 = srcPixelPtr[0]; k = t * u;
      a00_1 = srcPixelPtr[1];
      a10_0 = srcPixelPtr2[0]; k0 = (a01_0 - a00_0) * t;
      a10_1 = srcPixelPtr2[1]; k1 = (a01_1 - a00_1) * t;
      a11_0 = srcPixelPtr2[2]; k0 += (a10_0 - a00_0) * u;
      a11_1 = srcPixelPtr2[3]; k1 += (a10_1 - a00_1) * u;
      dstPixelPtr[0] = (mlib_s16)t0;
      dstPixelPtr[1] = (mlib_s16)t1;
    }

    k0 += (a00_0 - a01_0 + a11_0 - a10_0) * k;
    k1 += (a00_1 - a01_1 + a11_1 - a10_1) * k;
    dstPixelPtr[0] = a00_0 + (mlib_s32)k0;
    dstPixelPtr[1] = a00_1 + (mlib_s32)k1;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_s16_3ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 scale = 1. / (mlib_d64) MLIB_PREC;

  for (j = yStart; j <= yFinish; j++)
  {
    MLIB_TYPE *srcPixelPtr2;
    mlib_d64  t, u, k, k0, k1, k2;
    mlib_s32  a00_0, a01_0, a10_0, a11_0;
    mlib_s32  a00_1, a01_1, a10_1, a11_1;
    mlib_s32  a00_2, a01_2, a10_2, a11_2;

    CLIP(3)

    t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) +3*xSrc;
    srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
    a00_0 = srcPixelPtr[0];
    a00_1 = srcPixelPtr[1];  k = t * u;
    a00_2 = srcPixelPtr[2];
    a01_0 = srcPixelPtr[3];
    a01_1 = srcPixelPtr[4];
    a01_2 = srcPixelPtr[5];
    a10_0 = srcPixelPtr2[0]; k0 = (a01_0 - a00_0) * t;
    a10_1 = srcPixelPtr2[1]; k1 = (a01_1 - a00_1) * t;
    a10_2 = srcPixelPtr2[2]; k2 = (a01_2 - a00_2) * t;
    a11_0 = srcPixelPtr2[3]; k0 += (a10_0 - a00_0) * u;
    a11_1 = srcPixelPtr2[4]; k1 += (a10_1 - a00_1) * u;
    a11_2 = srcPixelPtr2[5]; k2 += (a10_2 - a00_2) * u;

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 3)
    {
      mlib_d64 pix0, pix1, pix2;
      mlib_s32 t0, t1, t2;

      pix0 = (a00_0 - a01_0 + a11_0 - a10_0) * k;
      pix1 = (a00_1 - a01_1 + a11_1 - a10_1) * k;
      pix2 = (a00_2 - a01_2 + a11_2 - a10_2) * k;
      pix0 += k0;
      pix1 += k1;
      pix2 += k2;
      t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> MLIB_SHIFT;  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 3*xSrc;
      srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
      a01_0 = srcPixelPtr[3];
      a01_1 = srcPixelPtr[4];
      a01_2 = srcPixelPtr[5];
      t0 = a00_0 + (mlib_s32)pix0;
      t1 = a00_1 + (mlib_s32)pix1;
      t2 = a00_2 + (mlib_s32)pix2;
      a00_0 = srcPixelPtr[0]; k = t * u;
      a00_1 = srcPixelPtr[1];
      a00_2 = srcPixelPtr[2];
      a10_0 = srcPixelPtr2[0]; k0 = (a01_0 - a00_0) * t;
      a10_1 = srcPixelPtr2[1]; k1 = (a01_1 - a00_1) * t;
      a10_2 = srcPixelPtr2[2]; k2 = (a01_2 - a00_2) * t;
      a11_0 = srcPixelPtr2[3]; k0 += (a10_0 - a00_0) * u;
      a11_1 = srcPixelPtr2[4]; k1 += (a10_1 - a00_1) * u;
      a11_2 = srcPixelPtr2[5]; k2 += (a10_2 - a00_2) * u;
      dstPixelPtr[0] = (mlib_s16)t0;
      dstPixelPtr[1] = (mlib_s16)t1;
      dstPixelPtr[2] = (mlib_s16)t2;
    }

    k0 += (a00_0 - a01_0 + a11_0 - a10_0) * k;
    k1 += (a00_1 - a01_1 + a11_1 - a10_1) * k;
    k2 += (a00_2 - a01_2 + a11_2 - a10_2) * k;
    dstPixelPtr[0] = a00_0 + (mlib_s32)k0;
    dstPixelPtr[1] = a00_1 + (mlib_s32)k1;
    dstPixelPtr[2] = a00_2 + (mlib_s32)k2;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_s16_4ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 scale = 1. / (mlib_d64) MLIB_PREC;

  for (j = yStart; j <= yFinish; j++)
  {
    MLIB_TYPE *srcPixelPtr2;
    mlib_d64  t, u, k, k0, k1, k2, k3;
    mlib_s32  a00_0, a01_0, a10_0, a11_0;
    mlib_s32  a00_1, a01_1, a10_1, a11_1;
    mlib_s32  a00_2, a01_2, a10_2, a11_2;
    mlib_s32  a00_3, a01_3, a10_3, a11_3;
    mlib_d64  t3;

    CLIP(4)

    t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 4*xSrc;
    srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
    a00_0 = srcPixelPtr[0]; k = t * u;
    a00_1 = srcPixelPtr[1];
    a00_2 = srcPixelPtr[2];
    a00_3 = srcPixelPtr[3];
    a01_0 = srcPixelPtr[4];  k0 = a00_0;
    a01_1 = srcPixelPtr[5];  k1 = a00_1;
    a01_2 = srcPixelPtr[6];  k2 = a00_2;
    a01_3 = srcPixelPtr[7];  k3 = a00_3;
    a10_0 = srcPixelPtr2[0]; k0 += (a01_0 - a00_0) * t;
    a10_1 = srcPixelPtr2[1]; k1 += (a01_1 - a00_1) * t;
    a10_2 = srcPixelPtr2[2]; k2 += (a01_2 - a00_2) * t;
    a10_3 = srcPixelPtr2[3]; k3 += (a01_3 - a00_3) * t;
    a11_0 = srcPixelPtr2[4]; k0 += (a10_0 - a00_0) * u;
    a11_1 = srcPixelPtr2[5]; k1 += (a10_1 - a00_1) * u;
    a11_2 = srcPixelPtr2[6]; k2 += (a10_2 - a00_2) * u;
    a11_3 = srcPixelPtr2[7]; k3 += (a10_3 - a00_3) * u;
    t3 = a00_3 - a01_3 + a11_3 - a10_3;

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 4)
    {
      mlib_d64 pix0, pix1, pix2, pix3;

      pix0 = k0 + (a00_0 - a01_0 + a11_0 - a10_0) * k;
      pix1 = k1 + (a00_1 - a01_1 + a11_1 - a10_1) * k;
      pix2 = k2 + (a00_2 - a01_2 + a11_2 - a10_2) * k;
      t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> (MLIB_SHIFT - 2);  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + (xSrc &~ 3);
      srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
      pix3 = k3 + t3 * k;
      a00_3 = srcPixelPtr[3];
      a01_3 = srcPixelPtr[7];  k3 = a00_3;
      a10_3 = srcPixelPtr2[3]; k3 += (a01_3 - a00_3) * t;
      a11_3 = srcPixelPtr2[7]; k3 += (a10_3 - a00_3) * u;
      t3 = a00_3 - a01_3 + a11_3 - a10_3;
      dstPixelPtr[3] = (MLIB_TYPE) pix3;
      a00_0 = srcPixelPtr[0]; k = t * u;
      a00_1 = srcPixelPtr[1];
      a00_2 = srcPixelPtr[2];
      a01_0 = srcPixelPtr[4];  k0 = a00_0;
      a01_1 = srcPixelPtr[5];  k1 = a00_1;
      a01_2 = srcPixelPtr[6];  k2 = a00_2;
      a10_0 = srcPixelPtr2[0]; k0 += (a01_0 - a00_0) * t;
      a10_1 = srcPixelPtr2[1]; k1 += (a01_1 - a00_1) * t;
      a10_2 = srcPixelPtr2[2]; k2 += (a01_2 - a00_2) * t;
      a11_0 = srcPixelPtr2[4]; k0 += (a10_0 - a00_0) * u;
      a11_1 = srcPixelPtr2[5]; k1 += (a10_1 - a00_1) * u;
      a11_2 = srcPixelPtr2[6]; k2 += (a10_2 - a00_2) * u;
      dstPixelPtr[0] = (MLIB_TYPE) pix0;
      dstPixelPtr[1] = (MLIB_TYPE) pix1;
      dstPixelPtr[2] = (MLIB_TYPE) pix2;
    }

    k0 += (a00_0 - a01_0 + a11_0 - a10_0) * k;
    k1 += (a00_1 - a01_1 + a11_1 - a10_1) * k;
    k2 += (a00_2 - a01_2 + a11_2 - a10_2) * k;
    k3 += t3 * k;
    dstPixelPtr[0] = (MLIB_TYPE) k0;
    dstPixelPtr[1] = (MLIB_TYPE) k1;
    dstPixelPtr[2] = (MLIB_TYPE) k2;
    dstPixelPtr[3] = (MLIB_TYPE) k3;
  }
}

/***************************************************************/

#else       /* for x86, using integer multiplies is faster */

/* due to overflow in multiplies like fdy * (a10 - a00) */
#undef  MLIB_SHIFT
#define MLIB_SHIFT 15

#define INTERP_S16(ind)                                                                    \
  pix0_##ind = a00_##ind + ((fdy * (a10_##ind - a00_##ind) + MLIB_ROUND) >> MLIB_SHIFT);   \
  pix1_##ind = a01_##ind + ((fdy * (a11_##ind - a01_##ind) + MLIB_ROUND) >> MLIB_SHIFT);   \
  res##ind = pix0_##ind + ((fdx * (pix1_##ind - pix0_##ind) + MLIB_ROUND) >> MLIB_SHIFT);

/***************************************************************/

void mlib_c_ImageAffine_s16_1ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_s32  srcYStride1 = srcYStride+2;
  mlib_s32  fdx, fdy;
  mlib_s32  a00_0, a01_0, a10_0, a11_0;
  mlib_s32  pix0_0, pix1_0, res0;

  srcYStride >>= 1;
  srcYStride1 >>= 1;
  dX = (dX + 1) >> 1;
  dY = (dY + 1) >> 1;

  for (j = yStart; j <= yFinish; j++)
  {
    CLIP(1)

    X = X >> 1;
    Y = Y >> 1;

    fdx = X & MLIB_MASK;
    fdy = Y & MLIB_MASK;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + xSrc;
    a00_0 = srcPixelPtr[0];
    a01_0 = srcPixelPtr[1];
    a10_0 = srcPixelPtr[srcYStride];
    a11_0 = srcPixelPtr[srcYStride1];

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
    {
      INTERP_S16(0)
      dstPixelPtr[0] = (mlib_s16) res0;

      fdx = X & MLIB_MASK;
      fdy = Y & MLIB_MASK;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> MLIB_SHIFT;  X += dX;
      srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + ySrc) + xSrc;
      a00_0 = srcPixelPtr[0];
      a01_0 = srcPixelPtr[1];
      a10_0 = srcPixelPtr[srcYStride];
      a11_0 = srcPixelPtr[srcYStride1];
    }
    INTERP_S16(0)
    dstPixelPtr[0] = (mlib_s16) res0;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_s16_2ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;
  mlib_s32  fdx, fdy;
  mlib_s32  a00_0, a01_0, a10_0, a11_0;
  mlib_s32  a00_1, a01_1, a10_1, a11_1;
  mlib_s32  pix0_0, pix1_0, res0;
  mlib_s32  pix0_1, pix1_1, res1;

  dX = (dX + 1) >> 1;
  dY = (dY + 1) >> 1;

  for (j = yStart; j <= yFinish; j++)
  {
    CLIP(2)

    X = X >> 1;
    Y = Y >> 1;

    fdx = X & MLIB_MASK;
    fdy = Y & MLIB_MASK;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
    srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
    a00_0 = srcPixelPtr[0];
    a00_1 = srcPixelPtr[1];
    a01_0 = srcPixelPtr[2];
    a01_1 = srcPixelPtr[3];
    a10_0 = srcPixelPtr2[0];
    a10_1 = srcPixelPtr2[1];
    a11_0 = srcPixelPtr2[2];
    a11_1 = srcPixelPtr2[3];

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 2)
    {
      INTERP_S16(0)
      INTERP_S16(1)
      dstPixelPtr[0] = (mlib_s16)res0;
      dstPixelPtr[1] = (mlib_s16)res1;

      fdx = X & MLIB_MASK;
      fdy = Y & MLIB_MASK;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> MLIB_SHIFT;  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
      srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
      a01_0 = srcPixelPtr[2];
      a01_1 = srcPixelPtr[3];
      a00_0 = srcPixelPtr[0];
      a00_1 = srcPixelPtr[1];
      a10_0 = srcPixelPtr2[0];
      a10_1 = srcPixelPtr2[1];
      a11_0 = srcPixelPtr2[2];
      a11_1 = srcPixelPtr2[3];
    }

    INTERP_S16(0)
    INTERP_S16(1)
    dstPixelPtr[0] = (mlib_s16)res0;
    dstPixelPtr[1] = (mlib_s16)res1;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_s16_3ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;
  mlib_s32  fdx, fdy;
  mlib_s32  a00_0, a01_0, a10_0, a11_0;
  mlib_s32  a00_1, a01_1, a10_1, a11_1;
  mlib_s32  a00_2, a01_2, a10_2, a11_2;
  mlib_s32  pix0_0, pix1_0, res0;
  mlib_s32  pix0_1, pix1_1, res1;
  mlib_s32  pix0_2, pix1_2, res2;

  dX = (dX + 1) >> 1;
  dY = (dY + 1) >> 1;

  for (j = yStart; j <= yFinish; j++)
  {
    CLIP(3)

    X = X >> 1;
    Y = Y >> 1;

    fdx = X & MLIB_MASK;
    fdy = Y & MLIB_MASK;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) +3*xSrc;
    srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
    a00_0 = srcPixelPtr[0];
    a00_1 = srcPixelPtr[1];
    a00_2 = srcPixelPtr[2];
    a01_0 = srcPixelPtr[3];
    a01_1 = srcPixelPtr[4];
    a01_2 = srcPixelPtr[5];
    a10_0 = srcPixelPtr2[0];
    a10_1 = srcPixelPtr2[1];
    a10_2 = srcPixelPtr2[2];
    a11_0 = srcPixelPtr2[3];
    a11_1 = srcPixelPtr2[4];
    a11_2 = srcPixelPtr2[5];

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 3)
    {
      INTERP_S16(0)
      INTERP_S16(1)
      INTERP_S16(2)
      dstPixelPtr[0] = (mlib_s16)res0;
      dstPixelPtr[1] = (mlib_s16)res1;
      dstPixelPtr[2] = (mlib_s16)res2;

      fdx = X & MLIB_MASK;
      fdy = Y & MLIB_MASK;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> MLIB_SHIFT;  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 3*xSrc;
      srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
      a01_0 = srcPixelPtr[3];
      a01_1 = srcPixelPtr[4];
      a01_2 = srcPixelPtr[5];
      a00_0 = srcPixelPtr[0];
      a00_1 = srcPixelPtr[1];
      a00_2 = srcPixelPtr[2];
      a10_0 = srcPixelPtr2[0];
      a10_1 = srcPixelPtr2[1];
      a10_2 = srcPixelPtr2[2];
      a11_0 = srcPixelPtr2[3];
      a11_1 = srcPixelPtr2[4];
      a11_2 = srcPixelPtr2[5];
    }

    INTERP_S16(0)
    INTERP_S16(1)
    INTERP_S16(2)
    dstPixelPtr[0] = (mlib_s16)res0;
    dstPixelPtr[1] = (mlib_s16)res1;
    dstPixelPtr[2] = (mlib_s16)res2;
  }
}

/***************************************************************/

void mlib_c_ImageAffine_s16_4ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  MLIB_TYPE *srcPixelPtr2;
  mlib_s32  fdx, fdy;
  mlib_s32  a00_0, a01_0, a10_0, a11_0;
  mlib_s32  a00_1, a01_1, a10_1, a11_1;
  mlib_s32  a00_2, a01_2, a10_2, a11_2;
  mlib_s32  a00_3, a01_3, a10_3, a11_3;
  mlib_s32  pix0_0, pix1_0, res0;
  mlib_s32  pix0_1, pix1_1, res1;
  mlib_s32  pix0_2, pix1_2, res2;
  mlib_s32  pix0_3, pix1_3, res3;

  dX = (dX + 1) >> 1;
  dY = (dY + 1) >> 1;

  for (j = yStart; j <= yFinish; j++)
  {
    CLIP(4)

    X = X >> 1;
    Y = Y >> 1;

    fdx = X & MLIB_MASK;
    fdy = Y & MLIB_MASK;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 4*xSrc;
    srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
    a00_0 = srcPixelPtr[0];
    a00_1 = srcPixelPtr[1];
    a00_2 = srcPixelPtr[2];
    a00_3 = srcPixelPtr[3];
    a01_0 = srcPixelPtr[4];
    a01_1 = srcPixelPtr[5];
    a01_2 = srcPixelPtr[6];
    a01_3 = srcPixelPtr[7];
    a10_0 = srcPixelPtr2[0];
    a10_1 = srcPixelPtr2[1];
    a10_2 = srcPixelPtr2[2];
    a10_3 = srcPixelPtr2[3];
    a11_0 = srcPixelPtr2[4];
    a11_1 = srcPixelPtr2[5];
    a11_2 = srcPixelPtr2[6];
    a11_3 = srcPixelPtr2[7];

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 4)
    {
      INTERP_S16(0)
      INTERP_S16(1)
      INTERP_S16(2)
      INTERP_S16(3)
      dstPixelPtr[0] = (mlib_s16)res0;
      dstPixelPtr[1] = (mlib_s16)res1;
      dstPixelPtr[2] = (mlib_s16)res2;
      dstPixelPtr[3] = (mlib_s16)res3;

      fdx = X & MLIB_MASK;
      fdy = Y & MLIB_MASK;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> (MLIB_SHIFT - 2);  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + (xSrc &~ 3);
      srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
      a00_0 = srcPixelPtr[0];
      a00_1 = srcPixelPtr[1];
      a00_2 = srcPixelPtr[2];
      a00_3 = srcPixelPtr[3];
      a01_0 = srcPixelPtr[4];
      a01_1 = srcPixelPtr[5];
      a01_2 = srcPixelPtr[6];
      a01_3 = srcPixelPtr[7];
      a10_0 = srcPixelPtr2[0];
      a10_1 = srcPixelPtr2[1];
      a10_2 = srcPixelPtr2[2];
      a10_3 = srcPixelPtr2[3];
      a11_0 = srcPixelPtr2[4];
      a11_1 = srcPixelPtr2[5];
      a11_2 = srcPixelPtr2[6];
      a11_3 = srcPixelPtr2[7];
    }

    INTERP_S16(0)
    INTERP_S16(1)
    INTERP_S16(2)
    INTERP_S16(3)
    dstPixelPtr[0] = (mlib_s16)res0;
    dstPixelPtr[1] = (mlib_s16)res1;
    dstPixelPtr[2] = (mlib_s16)res2;
    dstPixelPtr[3] = (mlib_s16)res3;
  }
}

#endif /* #ifdef __sparc */

/***************************************************************/

#undef  MLIB_SHIFT
#define MLIB_SHIFT 16

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s32

void mlib_c_ImageAffine_s32_1ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 scale = 1. / (mlib_d64) MLIB_PREC;
  mlib_s32 srcYStride1 = srcYStride+4;

  srcYStride >>= 2;
  srcYStride1 >>= 2;
  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  t, u, k0, k1, k2, k3;
    mlib_d64  a00_0, a01_0, a10_0, a11_0;
    mlib_d64 pix0;

    CLIP(1)

    t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + xSrc;
    k3 = t * u; k2 = (1.0 - t) * u;
    k1 = t * (1.0 - u); k0 = (1.0 - t) * (1.0 - u);
    a00_0 = srcPixelPtr[0];
    a01_0 = srcPixelPtr[1];
    a10_0 = srcPixelPtr[srcYStride];
    a11_0 = srcPixelPtr[srcYStride1];

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
    {

      mlib_s32 t0;

      pix0 = k0 * a00_0  + k1 * a01_0 + k2 * a10_0 + k3 * a11_0;
      t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> MLIB_SHIFT;  X += dX;
      srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + ySrc) + xSrc;
      t0 = (mlib_s32)(pix0);
      k3 = t * u; k2 = (1.0 - t) * u;
      k1 = t * (1.0 - u); k0 = (1.0 - t) * (1.0 - u);
      a00_0 = srcPixelPtr[0];
      a01_0 = srcPixelPtr[1];
      a10_0 = srcPixelPtr[srcYStride];
      a11_0 = srcPixelPtr[srcYStride1];
      dstPixelPtr[0] = t0;
    }
    pix0 = k0 * a00_0  + k1 * a01_0 + k2 * a10_0 + k3 * a11_0;
    dstPixelPtr[0] = (mlib_s32)(pix0);
  }
}

/***************************************************************/

void mlib_c_ImageAffine_s32_2ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 scale = 1. / (mlib_d64) MLIB_PREC;

  for (j = yStart; j <= yFinish; j++)
  {
    MLIB_TYPE *srcPixelPtr2;
    mlib_d64  t, u, k0, k1, k2, k3;
    mlib_d64  a00_0, a01_0, a10_0, a11_0;
    mlib_d64  a00_1, a01_1, a10_1, a11_1;
    mlib_d64 pix0, pix1;

    CLIP(2)

    t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
    srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
    k3 = t * u; k2 = (1.0 - t) * u;
    k1 = t * (1.0 - u); k0 = (1.0 - t) * (1.0 - u);
    a00_0 = srcPixelPtr[0];
    a00_1 = srcPixelPtr[1];
    a01_0 = srcPixelPtr[2];
    a01_1 = srcPixelPtr[3];
    a10_0 = srcPixelPtr2[0];
    a10_1 = srcPixelPtr2[1];
    a11_0 = srcPixelPtr2[2];
    a11_1 = srcPixelPtr2[3];

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 2)
    {
      mlib_s32 t0, t1;

      pix0 = k0 * a00_0  + k1 * a01_0 + k2 * a10_0 + k3 * a11_0;
      pix1 = k0 * a00_1  + k1 * a01_1 + k2 * a10_1 + k3 * a11_1;
      t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> MLIB_SHIFT;  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 2*xSrc;
      srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
      k3 = t * u; k2 = (1.0 - t) * u;
      k1 = t * (1.0 - u); k0 = (1.0 - t) * (1.0 - u);
      a01_0 = srcPixelPtr[2];
      a01_1 = srcPixelPtr[3];
      t0 = (mlib_s32)(pix0);
      t1 = (mlib_s32)(pix1);
      a00_0 = srcPixelPtr[0];
      a00_1 = srcPixelPtr[1];
      a10_0 = srcPixelPtr2[0];
      a10_1 = srcPixelPtr2[1];
      a11_0 = srcPixelPtr2[2];
      a11_1 = srcPixelPtr2[3];
      dstPixelPtr[0] = t0;
      dstPixelPtr[1] = t1;
    }

    pix0 = k0 * a00_0  + k1 * a01_0 + k2 * a10_0 + k3 * a11_0;
    pix1 = k0 * a00_1  + k1 * a01_1 + k2 * a10_1 + k3 * a11_1;
    dstPixelPtr[0] = (mlib_s32)(pix0);
    dstPixelPtr[1] = (mlib_s32)(pix1);
  }
}

/***************************************************************/

void mlib_c_ImageAffine_s32_3ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 scale = 1. / (mlib_d64) MLIB_PREC;

  for (j = yStart; j <= yFinish; j++)
  {
    MLIB_TYPE *srcPixelPtr2;
    mlib_d64  t, u, k0, k1, k2, k3;
    mlib_d64  a00_0, a01_0, a10_0, a11_0;
    mlib_d64  a00_1, a01_1, a10_1, a11_1;
    mlib_d64  a00_2, a01_2, a10_2, a11_2;
    mlib_d64 pix0, pix1, pix2;

    CLIP(3)

    t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) +3*xSrc;
    srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
    k3 = t * u; k2 = (1.0 - t) * u;
    k1 = t * (1.0 - u); k0 = (1.0 - t) * (1.0 - u);
    a00_0 = srcPixelPtr[0];
    a00_1 = srcPixelPtr[1];
    a00_2 = srcPixelPtr[2];
    a01_0 = srcPixelPtr[3];
    a01_1 = srcPixelPtr[4];
    a01_2 = srcPixelPtr[5];
    a10_0 = srcPixelPtr2[0];
    a10_1 = srcPixelPtr2[1];
    a10_2 = srcPixelPtr2[2];
    a11_0 = srcPixelPtr2[3];
    a11_1 = srcPixelPtr2[4];
    a11_2 = srcPixelPtr2[5];

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 3)
    {
      mlib_s32 t0, t1, t2;

      pix0 = k0 * a00_0  + k1 * a01_0 + k2 * a10_0 + k3 * a11_0;
      pix1 = k0 * a00_1  + k1 * a01_1 + k2 * a10_1 + k3 * a11_1;
      pix2 = k0 * a00_2  + k1 * a01_2 + k2 * a10_2 + k3 * a11_2;
      t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> MLIB_SHIFT;  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 3*xSrc;
      srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
      k3 = t * u; k2 = (1.0 - t) * u;
      k1 = t * (1.0 - u); k0 = (1.0 - t) * (1.0 - u);
      a01_0 = srcPixelPtr[3];
      a01_1 = srcPixelPtr[4];
      a01_2 = srcPixelPtr[5];
      t0 = (mlib_s32)(pix0);
      t1 = (mlib_s32)(pix1);
      t2 = (mlib_s32)(pix2);
      a00_0 = srcPixelPtr[0];
      a00_1 = srcPixelPtr[1];
      a00_2 = srcPixelPtr[2];
      a10_0 = srcPixelPtr2[0];
      a10_1 = srcPixelPtr2[1];
      a10_2 = srcPixelPtr2[2];
      a11_0 = srcPixelPtr2[3];
      a11_1 = srcPixelPtr2[4];
      a11_2 = srcPixelPtr2[5];
      dstPixelPtr[0] = t0;
      dstPixelPtr[1] = t1;
      dstPixelPtr[2] = t2;
    }

    pix0 = k0 * a00_0  + k1 * a01_0 + k2 * a10_0 + k3 * a11_0;
    pix1 = k0 * a00_1  + k1 * a01_1 + k2 * a10_1 + k3 * a11_1;
    pix2 = k0 * a00_2  + k1 * a01_2 + k2 * a10_2 + k3 * a11_2;
    dstPixelPtr[0] = (mlib_s32)(pix0);
    dstPixelPtr[1] = (mlib_s32)(pix1);
    dstPixelPtr[2] = (mlib_s32)(pix2);
  }
}

/***************************************************************/

void mlib_c_ImageAffine_s32_4ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 scale = 1. / (mlib_d64) MLIB_PREC;

  for (j = yStart; j <= yFinish; j++)
  {
    MLIB_TYPE *srcPixelPtr2;
    mlib_d64  t, u, k0, k1, k2, k3;
    mlib_d64  a00_0, a01_0, a10_0, a11_0;
    mlib_d64  a00_1, a01_1, a10_1, a11_1;
    mlib_d64  a00_2, a01_2, a10_2, a11_2;
    mlib_d64  a00_3, a01_3, a10_3, a11_3;
    mlib_d64  pix0, pix1, pix2, pix3;

    CLIP(4)

    t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
    ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
    xSrc = X >> MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + 4*xSrc;
    srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
    k3 = t * u; k2 = (1.0 - t) * u;
    k1 = t * (1.0 - u); k0 = (1.0 - t) * (1.0 - u);
    a00_0 = srcPixelPtr[0];
    a00_1 = srcPixelPtr[1];
    a00_2 = srcPixelPtr[2];
    a00_3 = srcPixelPtr[3];
    a01_0 = srcPixelPtr[4];
    a01_1 = srcPixelPtr[5];
    a01_2 = srcPixelPtr[6];
    a01_3 = srcPixelPtr[7];
    a10_0 = srcPixelPtr2[0];
    a10_1 = srcPixelPtr2[1];
    a10_2 = srcPixelPtr2[2];
    a10_3 = srcPixelPtr2[3];
    a11_0 = srcPixelPtr2[4];
    a11_1 = srcPixelPtr2[5];
    a11_2 = srcPixelPtr2[6];
    a11_3 = srcPixelPtr2[7];

    for (; dstPixelPtr < dstLineEnd; dstPixelPtr += 4)
    {
      mlib_s32 t0, t1, t2, t3;

      pix0 = k0 * a00_0  + k1 * a01_0 + k2 * a10_0 + k3 * a11_0;
      pix1 = k0 * a00_1  + k1 * a01_1 + k2 * a10_1 + k3 * a11_1;
      pix2 = k0 * a00_2  + k1 * a01_2 + k2 * a10_2 + k3 * a11_2;
      pix3 = k0 * a00_3  + k1 * a01_3 + k2 * a10_3 + k3 * a11_3;
      t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X >> (MLIB_SHIFT - 2);  X += dX;
      srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + (xSrc &~ 3);
      srcPixelPtr2 = (MLIB_TYPE *)((mlib_u8 *)srcPixelPtr + srcYStride);
      k3 = t * u; k2 = (1.0 - t) * u;
      k1 = t * (1.0 - u); k0 = (1.0 - t) * (1.0 - u);
      a00_3 = srcPixelPtr[3];
      a01_3 = srcPixelPtr[7];
      a10_3 = srcPixelPtr2[3];
      a11_3 = srcPixelPtr2[7];
      t0 = (mlib_s32)(pix0);
      t1 = (mlib_s32)(pix1);
      t2 = (mlib_s32)(pix2);
      t3 = (mlib_s32)(pix3);
      a00_0 = srcPixelPtr[0];
      a00_1 = srcPixelPtr[1];
      a00_2 = srcPixelPtr[2];
      a01_0 = srcPixelPtr[4];
      a01_1 = srcPixelPtr[5];
      a01_2 = srcPixelPtr[6];
      a10_0 = srcPixelPtr2[0];
      a10_1 = srcPixelPtr2[1];
      a10_2 = srcPixelPtr2[2];
      a11_0 = srcPixelPtr2[4];
      a11_1 = srcPixelPtr2[5];
      a11_2 = srcPixelPtr2[6];
      dstPixelPtr[0] = t0;
      dstPixelPtr[1] = t1;
      dstPixelPtr[2] = t2;
      dstPixelPtr[3] = t3;
    }

    pix0 = k0 * a00_0  + k1 * a01_0 + k2 * a10_0 + k3 * a11_0;
    pix1 = k0 * a00_1  + k1 * a01_1 + k2 * a10_1 + k3 * a11_1;
    pix2 = k0 * a00_2  + k1 * a01_2 + k2 * a10_2 + k3 * a11_2;
    pix3 = k0 * a00_3  + k1 * a01_3 + k2 * a10_3 + k3 * a11_3;
    dstPixelPtr[0] = (mlib_s32)pix0;
    dstPixelPtr[1] = (mlib_s32)pix1;
    dstPixelPtr[2] = (mlib_s32)pix2;
    dstPixelPtr[3] = (mlib_s32)pix3;
  }
}

/***************************************************************/
