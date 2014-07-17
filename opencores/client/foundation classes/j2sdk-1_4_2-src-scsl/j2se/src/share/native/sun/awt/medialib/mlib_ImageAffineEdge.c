/*
 * @(#)mlib_ImageAffineEdge.c	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageAffineEdge.c	1.22	99/12/28 SMI"
#endif /* __SUNPRO_C */

#ifdef _MSC_VER
#pragma warning(disable:4068 4244 4101)
#endif /*_MSC_VER*/

/*
 * FUNCTION
 *      mlib_ImageAffineZeroEdge - implementation of MLIB_EDGE_DST_FILL_ZERO
 *                                 edge condition
 *      mlib_ImageAffineEdgeNearest - implementation of MLIB_EDGE_OP_NEAREST
 *                                    edge condition
 *      void mlib_ImageAffineEdgeExtend_BL - implementation of MLIB_EDGE_SRC_EXTEND
 *                                           edge condition for MLIB_BILINEAR filter
 *      void mlib_ImageAffineEdgeExtend_BC - implementation of MLIB_EDGE_SRC_EXTEND
 *                                           edge condition for MLIB_BICUBIC filter
 *      void mlib_ImageAffineEdgeExtend_BC2 - implementation of MLIB_EDGE_SRC_EXTEND
 *                                            edge condition for MLIB_BICUBIC2 filter
 *
 * SYNOPSIS
 *      void mlib_ImageAffineEdgeZero(mlib_image *dst,
 *                                    mlib_s32 *leftEdges,
 *                                    mlib_s32 *rightEdges,
 *                                    mlib_s32 *sides,
 *                                    mlib_s32 *leftEdgesE,
 *                                    mlib_s32 *rightEdgesE,
 *                                    mlib_s32 *sidesE);
 *      void mlib_ImageAffineEdgeNearest(mlib_image *dst,
 *                                       mlib_s32 *leftEdges,
 *                                       mlib_s32 *rightEdges,
 *                                       mlib_s32 *sides,
 *                                       mlib_s32 *leftEdgesE,
 *                                       mlib_s32 *rightEdgesE,
 *                                       mlib_s32 *xStartsE,
 *                                       mlib_s32 *yStartsE,
 *                                       mlib_s32 *sidesE
 *                                       mlib_u8  **lineAddr);
 *      void mlib_ImageAffineEdgeExtend_BL(mlib_image *dst,
 *                                         mlib_image *src,
 *                                         mlib_s32 *leftEdges,
 *                                         mlib_s32 *rightEdges,
 *                                         mlib_s32 *sides,
 *                                         mlib_s32 *leftEdgesE,
 *                                         mlib_s32 *rightEdgesE,
 *                                         mlib_s32 *xStartsE,
 *                                         mlib_s32 *yStartsE,
 *                                         mlib_s32 *sidesE,
 *                                         mlib_u8  **lineAddr);
 *      void mlib_ImageAffineEdgeExtend_BC(mlib_image *dst,
 *                                         mlib_image *src,
 *                                         mlib_s32 *leftEdges,
 *                                         mlib_s32 *rightEdges,
 *                                         mlib_s32 *sides,
 *                                         mlib_s32 *leftEdgesE,
 *                                         mlib_s32 *rightEdgesE,
 *                                         mlib_s32 *xStartsE,
 *                                         mlib_s32 *yStartsE,
 *                                         mlib_s32 *sidesE,
 *                                         mlib_u8  **lineAddr);
 *      void mlib_ImageAffineEdgeExtend_BC2(mlib_image *dst,
 *                                          mlib_image *src,
 *                                          mlib_s32 *leftEdges,
 *                                          mlib_s32 *rightEdges,
 *                                          mlib_s32 *sides,
 *                                          mlib_s32 *leftEdgesE,
 *                                          mlib_s32 *rightEdgesE,
 *                                          mlib_s32 *xStartsE,
 *                                          mlib_s32 *yStartsE,
 *                                          mlib_s32 *sidesE,
 *                                          mlib_u8  **lineAddr);
 *
 * ARGUMENTS
 *      dst         - pointer to the destination image
 *      src         - pointer to the source image
 *      leftEdges   - array[dstHeight] of xLeft coordinates for the
 *                    corresponding interpolation (bilinear or bicubic)
 *      rightEdges  - array[dstHeight] of xRight coordinates for the
 *                    corresponding interpolation (bilinear or bicubic)
 *      sides       - array[4]. sides[0] is yStart, sides[1] is yFinish,
 *                    sides[2] is dx * 65536, sides[3] is dy * 65536 for
 *                    the corresponding interpolation (bilinear or bicubic)
 *      leftEdgesE  - array[dstHeight] of xLeft coordinates for the
 *                    nearest neighbour interpolation
 *      rightEdgesE - array[dstHeight] of xRight coordinates for the
 *                    nearest neighbour interpolation
 *      xStartsE    - array[dstHeight] of xStart * 65536 coordinates
 *                    for the nearest neighbour interpolation
 *      yStartsE    - array[dstHeight] of yStart * 65536 coordinates
 *                    for the nearest neighbour interpolation
 *      sidesE      - array[4]. sides[0] is yStart, sides[1] is yFinish,
 *                    sides[2] is dx * 65536, sides[3] is dy * 65536 for
 *                    nearest neighbour interpolation
 *      lineAddr    - array[srcHeight] of pointers to the first pixel on
 *                    the corresponding lines in source image
 *
 * DESCRIPTION
 *      mlib_ImageAffineZeroEdge:
 *      This function fills the edge pixels (i.e. thouse one which can not
 *      be interpolated with given resampling filter because their prototypes
 *      in the source image lie too close to the border) in the destination
 *      image with zeroes.
 *
 *      mlib_ImageAffineEdgeNearest:
 *      This function fills the edge pixels (i.e. thouse one which can not
 *      be interpolated with given resampling filter because their prototypes
 *      in the source image lie too close to the border) in the destination
 *      image according to the nearest neighbour interpolation.
 *
 *      mlib_ImageAffineEdgeExtend_BL:
 *      This function fills the edge pixels (i.e. thouse one which can not
 *      be interpolated with given resampling filter because their prototypes
 *      in the source image lie too close to the border) in the destination
 *      image according to the bilinear interpolation with border pixels extend
 *      of source image.
 *
 *      mlib_ImageAffineEdgeExtend_BC:
 *      This function fills the edge pixels (i.e. thouse one which can not
 *      be interpolated with given resampling filter because their prototypes
 *      in the source image lie too close to the border) in the destination
 *      image according to the bicubic interpolation with border pixels extend
 *      of source image.
 *
 *      mlib_ImageAffineEdgeExtend_BC2:
 *      This function fills the edge pixels (i.e. thouse one which can not
 *      be interpolated with given resampling filter because their prototypes
 *      in the source image lie too close to the border) in the destination
 *      image according to the bicubic2 interpolation with border pixels extend
 *      of source image.
 */

#include <stdlib.h>
#include "mlib_image.h"
#include "mlib_ImageFilters.h"

/***************************************************************/

#define MLIB_SHIFT      16
#define MLIB_PREC       (1 << MLIB_SHIFT)
#define MLIB_MASK       (MLIB_PREC - 1)
#define MLIB_SIGN_SHIFT 31

#define FILTER_SHIFT_U8  4
#define FILTER_MASK_U8   (((1 << 8) - 1) << 4)
#define FILTER_SHIFT_S16 3
#define FILTER_MASK_S16  (((1 << 9) - 1) << 4)
#define sPtr srcPixelPtr

/***************************************************************/

extern mlib_d64 mlib_U82D64[];

#define D64mlib_u8(X)   mlib_U82D64[X]
#define D64mlib_s16(X)  ((mlib_d64)(X))
#define D64mlib_s32(X)  ((mlib_d64)(X))
#define D64mlib_f32(X)  ((mlib_d64)(X))
#define D64mlib_d64(X)  ((mlib_d64)(X))

/***************************************************************/

#ifdef MLIB_USE_FTOI_CLAMPING

#define SATmlib_u8(DST, val0)                   \
  DST = ((mlib_s32)(val0 - sat) >> 24) ^ 0x80;

#define SATmlib_s16(DST, val0)          \
  DST = ((mlib_s32)val0) >> 16;

#define SATmlib_s32(DST, val0)  \
  DST = val0;

#else

#define SATmlib_u8(DST, val0)                   \
  val0 -= sat;                                  \
  if (val0 >= MLIB_S32_MAX)                     \
    DST = MLIB_U8_MAX;                          \
  else if (val0 <= MLIB_S32_MIN)                \
    DST = MLIB_U8_MIN;                          \
  else                                          \
    DST = ((mlib_s32) val0 >> 24) ^ 0x80;

#define SATmlib_s16(DST, val0)          \
  if (val0 >= MLIB_S32_MAX)             \
    DST = MLIB_S16_MAX;                 \
  else if (val0 <= MLIB_S32_MIN)        \
    DST = MLIB_S16_MIN;                 \
  else                                  \
    DST = (mlib_s32)val0 >> 16;

#define SATmlib_s32(DST, val0)          \
  if (val0 >= MLIB_S32_MAX)             \
    DST = MLIB_S32_MAX;                 \
  else if (val0 <= MLIB_S32_MIN)        \
    DST = MLIB_S32_MIN;                 \
  else                                  \
    DST = (mlib_s32)val0;

#endif

#define SATmlib_f32(DST, val0)  \
  DST = val0;

#define SATmlib_d64(DST, val0)  \
  DST = val0;

/***************************************************************/

#define MLIB_EDGE_ZERO_LINE(TYPE, Left, Right)          \
  dstPixelPtr = (TYPE*)data + channels * Left;          \
  dstLineEnd  = (TYPE*)data + channels * Right;         \
                                                        \
  for (; dstPixelPtr < dstLineEnd; dstPixelPtr++) {     \
    *dstPixelPtr = 0;                                   \
  }

/***************************************************************/

#define MLIB_EDGE_NEAREST_LINE(TYPE, Left, Right)                       \
  dstPixelPtr = (TYPE*)data + channels * Left;                          \
  dstLineEnd  = (TYPE*)data + channels * Right;                         \
                                                                        \
  for (; dstPixelPtr < dstLineEnd; dstPixelPtr += channels) {           \
    ySrc = Y >> MLIB_SHIFT;                                             \
    xSrc = X >> MLIB_SHIFT;                                             \
    srcPixelPtr = (TYPE*)lineAddr[ySrc] + xSrc * channels;              \
                                                                        \
    for (j = 0; j < channels; j++) dstPixelPtr[j] = srcPixelPtr[j];     \
                                                                        \
    Y += dY;                                                            \
    X += dX;                                                            \
  }

/***************************************************************/

#define MLIB_EDGE_BL_LINE(TYPE, Left, Right)                             \
  dstPixelPtr = (TYPE*)data + channels * Left;                           \
  dstLineEnd  = (TYPE*)data + channels * Right;                          \
                                                                         \
  for (; dstPixelPtr < dstLineEnd; dstPixelPtr += channels) {            \
    ySrc = ((Y - 32768) >> MLIB_SHIFT);                                  \
    xSrc = ((X - 32768) >> MLIB_SHIFT);                                  \
                                                                         \
    t = ((X - 32768) & MLIB_MASK) * scale;                               \
    u = ((Y - 32768) & MLIB_MASK) * scale;                               \
                                                                         \
    xDelta = (((xSrc + 1 - srcWidth )) >> MLIB_SIGN_SHIFT) & channels;   \
    yDelta = (((ySrc + 1 - srcHeight)) >> MLIB_SIGN_SHIFT) & srcStride;  \
                                                                         \
    xFlag = (xSrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                    \
    xSrc = xSrc + (1 & xFlag);                                           \
    xDelta = xDelta &~ xFlag;                                            \
                                                                         \
    yFlag = (ySrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                    \
    ySrc = ySrc + (1 & yFlag);                                           \
    yDelta = yDelta &~ yFlag;                                            \
                                                                         \
    srcPixelPtr = (TYPE*)lineAddr[ySrc] + xSrc * channels;               \
                                                                         \
    for (j = 0; j < channels; j++) {                                     \
      a00   = *srcPixelPtr;                                              \
      a01   = *(srcPixelPtr + xDelta);                                   \
      a10   = *(srcPixelPtr + yDelta);                                   \
      a11   = *(srcPixelPtr + yDelta + xDelta);                          \
      pix0  = ((a00 * (1 - t) + a01 * t) * (1 - u) +                     \
               (a10 * (1 - t) + a11 * t) * u);                           \
                                                                         \
      dstPixelPtr[j] = (mlib_s32)pix0;                                   \
      srcPixelPtr++;                                                     \
    }                                                                    \
                                                                         \
    X += dX;                                                             \
    Y += dY;                                                             \
  }

/***************************************************************/

#define GET_FILTER_TBL_U8(X, xf0, xf1, xf2, xf3, FTBL)                  \
  filterpos = ((X - 32768) >> FILTER_SHIFT_U8) & FILTER_MASK_U8;        \
  fptr = (mlib_f32 *) ((mlib_u8 *)FTBL + filterpos);                    \
                                                                        \
  xf0 = fptr[0];                                                        \
  xf1 = fptr[1];                                                        \
  xf2 = fptr[2];                                                        \
  xf3 = fptr[3]

#define GET_FILTER_TBL_S16(X, xf0, xf1, xf2, xf3, FTBL)                 \
  filterpos = ((X - 32768) >> FILTER_SHIFT_S16) & FILTER_MASK_S16;      \
  fptr = (mlib_f32 *) ((mlib_u8 *)FTBL + filterpos);                    \
                                                                        \
  xf0 = fptr[0];                                                        \
  xf1 = fptr[1];                                                        \
  xf2 = fptr[2];                                                        \
  xf3 = fptr[3]

#define GET_TBL_U8_BC(X, xf0, xf1, xf2, xf3)                            \
  GET_FILTER_TBL_U8(X, xf0, xf1, xf2, xf3, mlib_filters_u8f_bc)

#define GET_TBL_U8_BC2(X, xf0, xf1, xf2, xf3)                           \
  GET_FILTER_TBL_U8(X, xf0, xf1, xf2, xf3, mlib_filters_u8f_bc2)

#define GET_TBL_S16_BC(X, xf0, xf1, xf2, xf3)                           \
  GET_FILTER_TBL_S16(X, xf0, xf1, xf2, xf3, mlib_filters_s16f_bc)

#define GET_TBL_S16_BC2(X, xf0, xf1, xf2, xf3)                          \
  GET_FILTER_TBL_S16(X, xf0, xf1, xf2, xf3, mlib_filters_s16f_bc2)

/***************************************************************/

#define GET_FILTER_BC(X, xf0, xf1, xf2, xf3)    \
  dx = ((X - 32768) & MLIB_MASK) * scale;       \
  dx_2  = 0.5 * dx;                             \
  dx2   = dx * dx;                              \
  dx3_2 = dx_2 * dx2;                           \
  dx3_3 = 3.0 * dx3_2;                          \
                                                \
  xf0 = dx2 - dx3_2 - dx_2;                     \
  xf1 = dx3_3 - 2.5 * dx2 + 1.0;                \
  xf2 = 2.0 * dx2 - dx3_3 + dx_2;               \
  xf3 = dx3_2 - 0.5 * dx2

#define GET_FILTER_BC2(X, xf0, xf1, xf2, xf3)   \
  dx =  ((X - 32768) & MLIB_MASK) * scale;      \
  dx2   = dx  * dx;                             \
  dx3_2 = dx  * dx2;                            \
  dx3_3 = 2.0 * dx2;                            \
                                                \
  xf0 = - dx3_2 + dx3_3 - dx;                   \
  xf1 =   dx3_2 - dx3_3 + 1.0;                  \
  xf2 = - dx3_2 + dx2   + dx;                   \
  xf3 =   dx3_2 - dx2

/***************************************************************/

#define MLIB_EDGE_BC_LINE(TYPE, Left, Right, GET_FILTER)                              \
  dstPixelPtr = (TYPE*)data + channels * Left;                                        \
  dstLineEnd  = (TYPE*)data + channels * Right;                                       \
                                                                                      \
  for (; dstPixelPtr < dstLineEnd; dstPixelPtr += channels) {                         \
    GET_FILTER(X, xf0, xf1, xf2, xf3);                                                \
    GET_FILTER(Y, yf0, yf1, yf2, yf3);                                                \
                                                                                      \
    xSrc = ((X - 32768) >> MLIB_SHIFT);                                               \
    ySrc = ((Y - 32768) >> MLIB_SHIFT);                                               \
                                                                                      \
    xDelta0 = ((~((xSrc - 1) >> MLIB_SIGN_SHIFT)) & (- channels));                    \
    yDelta0 = ((~((ySrc - 1) >> MLIB_SIGN_SHIFT)) & (- srcStride));                   \
    xDelta1 = ((xSrc + 1 - srcWidth) >> MLIB_SIGN_SHIFT) & (channels);                \
    yDelta1 = ((ySrc + 1 - srcHeight) >> MLIB_SIGN_SHIFT) & (srcStride);              \
    xDelta2 = xDelta1 + (((xSrc + 2 - srcWidth) >> MLIB_SIGN_SHIFT) & (channels));    \
    yDelta2 = yDelta1 + (((ySrc + 2 - srcHeight) >> MLIB_SIGN_SHIFT) & (srcStride));  \
                                                                                      \
    xFlag = (xSrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                                 \
    xSrc = xSrc + (1 & xFlag);                                                        \
    xDelta2 -= (xDelta1 & xFlag);                                                     \
    xDelta1 = (xDelta1 &~ xFlag);                                                     \
                                                                                      \
    yFlag = (ySrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                                 \
    ySrc = ySrc + (1 & yFlag);                                                        \
    yDelta2  -= (yDelta1 & yFlag);                                                    \
    yDelta1 = yDelta1 &~ yFlag;                                                       \
                                                                                      \
    sPtr = (TYPE*)lineAddr[ySrc] + xSrc * channels;                                   \
                                                                                      \
    for (j = 0; j < channels; j++) {                                                  \
      c0 = (D64##TYPE((*(sPtr + yDelta0 + xDelta0))) * xf0 +                          \
            D64##TYPE((*(sPtr + yDelta0          ))) * xf1 +                          \
            D64##TYPE((*(sPtr + yDelta0 + xDelta1))) * xf2 +                          \
            D64##TYPE((*(sPtr + yDelta0 + xDelta2))) * xf3);                          \
                                                                                      \
      c1 = (D64##TYPE((*(sPtr + xDelta0))) * xf0 +                                    \
            D64##TYPE((*sPtr)            ) * xf1 +                                    \
            D64##TYPE((*(sPtr + xDelta1))) * xf2 +                                    \
            D64##TYPE((*(sPtr + xDelta2))) * xf3);                                    \
                                                                                      \
      c2 = (D64##TYPE((*(sPtr + yDelta1 + xDelta0))) * xf0 +                          \
            D64##TYPE((*(sPtr + yDelta1          ))) * xf1 +                          \
            D64##TYPE((*(sPtr + yDelta1 + xDelta1))) * xf2 +                          \
            D64##TYPE((*(sPtr + yDelta1 + xDelta2))) * xf3);                          \
                                                                                      \
      c3 = (D64##TYPE((*(sPtr + yDelta2 + xDelta0))) * xf0 +                          \
            D64##TYPE((*(sPtr + yDelta2          ))) * xf1 +                          \
            D64##TYPE((*(sPtr + yDelta2 + xDelta1))) * xf2 +                          \
            D64##TYPE((*(sPtr + yDelta2 + xDelta2))) * xf3);                          \
                                                                                      \
      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                                     \
                                                                                      \
      SAT##TYPE(dstPixelPtr[j], val0);                                                \
                                                                                      \
      sPtr++;                                                                         \
    }                                                                                 \
                                                                                      \
    X += dX;                                                                          \
    Y += dY;                                                                          \
  }

/***************************************************************/

#define MLIB_EDGE_BC_u8(TYPE, Left, Right)              \
  MLIB_EDGE_BC_LINE(TYPE, Left, Right, GET_TBL_U8_BC)

#define MLIB_EDGE_BC2_u8(TYPE, Left, Right)             \
  MLIB_EDGE_BC_LINE(TYPE, Left, Right, GET_TBL_U8_BC2)

#define MLIB_EDGE_BC_s16(TYPE, Left, Right)             \
  MLIB_EDGE_BC_LINE(TYPE, Left, Right, GET_TBL_S16_BC)

#define MLIB_EDGE_BC2_s16(TYPE, Left, Right)                    \
  MLIB_EDGE_BC_LINE(TYPE, Left, Right, GET_TBL_S16_BC2)

#define MLIB_EDGE_BC(TYPE, Left, Right)                 \
  MLIB_EDGE_BC_LINE(TYPE, Left, Right, GET_FILTER_BC)

#define MLIB_EDGE_BC2(TYPE, Left, Right)                \
  MLIB_EDGE_BC_LINE(TYPE, Left, Right, GET_FILTER_BC2)

/***************************************************************/

#define MLIB_PROCESS_EDGES_ZERO(TYPE) {                 \
    TYPE *srcPixelPtr;                                  \
    TYPE *dstPixelPtr;                                  \
    TYPE *dstLineEnd;                                   \
                                                        \
    for (i = yStartE; i < yStart; i++) {                \
      xLeftE  = leftEdgesE[i];                          \
      xRightE = rightEdgesE[i] + 1;                     \
      data   += dstStride;                              \
                                                        \
      MLIB_EDGE_ZERO_LINE(TYPE, xLeftE, xRightE);       \
    }                                                   \
                                                        \
    for (; i <= yFinish; i++) {                         \
      xLeftE  = leftEdgesE[i];                          \
      xRightE = rightEdgesE[i] + 1;                     \
      xLeft   = leftEdges[i];                           \
      xRight  = rightEdges[i] + 1;                      \
      data   += dstStride;                              \
                                                        \
      if (xLeft < xRight) {                             \
        MLIB_EDGE_ZERO_LINE(TYPE, xLeftE, xLeft);       \
      } else {                                          \
        xRight = xLeftE;                                \
      }                                                 \
                                                        \
      MLIB_EDGE_ZERO_LINE(TYPE, xRight, xRightE);       \
    }                                                   \
                                                        \
    for (; i <= yFinishE; i++) {                        \
      xLeftE  = leftEdgesE[i];                          \
      xRightE = rightEdgesE[i] + 1;                     \
      data   += dstStride;                              \
                                                        \
      MLIB_EDGE_ZERO_LINE(TYPE, xLeftE, xRightE);       \
    }                                                   \
  }

/***************************************************************/

#define MLIB_PROCESS_EDGES(PROCESS_LINE, TYPE) {        \
    TYPE *srcPixelPtr;                                  \
    TYPE *dstPixelPtr;                                  \
    TYPE *dstLineEnd;                                   \
                                                        \
    for (i = yStartE; i < yStart; i++) {                \
      xLeftE  = leftEdgesE[i];                          \
      xRightE = rightEdgesE[i] + 1;                     \
      X       = xStartsE[i];                            \
      Y       = yStartsE[i];                            \
      data   += dstStride;                              \
                                                        \
      PROCESS_LINE(TYPE, xLeftE, xRightE);              \
    }                                                   \
                                                        \
    for (; i <= yFinish; i++) {                         \
      xLeftE  = leftEdgesE[i];                          \
      xRightE = rightEdgesE[i] + 1;                     \
      xLeft   = leftEdges[i];                           \
      xRight  = rightEdges[i] + 1;                      \
      X       = xStartsE[i];                            \
      Y       = yStartsE[i];                            \
      data   += dstStride;                              \
                                                        \
      if (xLeft < xRight) {                             \
        PROCESS_LINE(TYPE, xLeftE, xLeft);              \
      } else {                                          \
        xRight = xLeftE;                                \
      }                                                 \
                                                        \
      X = xStartsE[i] + dX * (xRight - xLeftE);         \
      Y = yStartsE[i] + dY * (xRight - xLeftE);         \
      PROCESS_LINE(TYPE, xRight, xRightE);              \
    }                                                   \
                                                        \
    for (; i <= yFinishE; i++) {                        \
      xLeftE  = leftEdgesE[i];                          \
      xRightE = rightEdgesE[i] + 1;                     \
      X       = xStartsE[i];                            \
      Y       = yStartsE[i];                            \
      data   += dstStride;                              \
                                                        \
      PROCESS_LINE(TYPE, xLeftE, xRightE);              \
    }                                                   \
  }

/***************************************************************/

void mlib_ImageAffineEdgeZero(mlib_image *dst,
                              mlib_s32 *leftEdges,
                              mlib_s32 *rightEdges,
                              mlib_s32 *sides,
                              mlib_s32 *leftEdgesE,
                              mlib_s32 *rightEdgesE,
                              mlib_s32 *sidesE)
{

  mlib_type type      = mlib_ImageGetType(dst);
  mlib_s32  dstStride    = mlib_ImageGetStride(dst);
  mlib_s32  channels  = mlib_ImageGetChannels(dst);
  mlib_u8   *data     = (mlib_u8 *)mlib_ImageGetData(dst);
  mlib_s32  yStart    = sides[0];
  mlib_s32  yFinish   = sides[1];
  mlib_s32  yStartE   = sidesE[0];
  mlib_s32  yFinishE  = sidesE[1];
  mlib_s32  xLeft, xRight, xLeftE, xRightE;
  mlib_s32  i;

  data += (yStartE - 1) * dstStride;

  switch (type) {
    case MLIB_BYTE:
      MLIB_PROCESS_EDGES_ZERO(mlib_u8)
      break;

    case MLIB_SHORT:
      MLIB_PROCESS_EDGES_ZERO(mlib_s16)
      break;

    case MLIB_INT:
    case MLIB_FLOAT:
      MLIB_PROCESS_EDGES_ZERO(mlib_s32)
      break;

    case MLIB_DOUBLE:
      MLIB_PROCESS_EDGES_ZERO(mlib_d64)
      break;
    default:
      break;
  }
}

/***************************************************************/

void mlib_ImageAffineEdgeNearest(mlib_image *dst,
                                 mlib_s32 *leftEdges,
                                 mlib_s32 *rightEdges,
                                 mlib_s32 *sides,
                                 mlib_s32 *leftEdgesE,
                                 mlib_s32 *rightEdgesE,
                                 mlib_s32 *xStartsE,
                                 mlib_s32 *yStartsE,
                                 mlib_s32 *sidesE,
                                 mlib_u8  **lineAddr)
{
  mlib_type type      = mlib_ImageGetType(dst);
  mlib_s32  channels  = mlib_ImageGetChannels(dst);
  mlib_s32  dstStride    = mlib_ImageGetStride(dst);
  mlib_u8   *data     = (mlib_u8 *)mlib_ImageGetData(dst);
  mlib_s32  yStart    = sides[0];
  mlib_s32  yFinish   = sides[1];
  mlib_s32  yStartE   = sidesE[0];
  mlib_s32  yFinishE  = sidesE[1];
  mlib_s32  dX        = sidesE[2];
  mlib_s32  dY        = sidesE[3];
  mlib_s32  xSrc, ySrc, X, Y;
  mlib_s32  xLeft, xRight, xLeftE, xRightE;
  mlib_s32  i, j;

  data += (yStartE - 1) * dstStride;

  switch (type) {
    case MLIB_BYTE:
      MLIB_PROCESS_EDGES(MLIB_EDGE_NEAREST_LINE, mlib_u8)
      break;

    case MLIB_SHORT:
      MLIB_PROCESS_EDGES(MLIB_EDGE_NEAREST_LINE, mlib_s16)
      break;

    case MLIB_INT:
    case MLIB_FLOAT:
      MLIB_PROCESS_EDGES(MLIB_EDGE_NEAREST_LINE, mlib_s32)
      break;

    case MLIB_DOUBLE:
      MLIB_PROCESS_EDGES(MLIB_EDGE_NEAREST_LINE, mlib_d64)
      break;
    default:
      break;
  }
}

/***************************************************************/

void mlib_ImageAffineEdgeExtend_BL(mlib_image *dst,
                                   mlib_image *src,
                                   mlib_s32 *leftEdges,
                                   mlib_s32 *rightEdges,
                                   mlib_s32 *sides,
                                   mlib_s32 *leftEdgesE,
                                   mlib_s32 *rightEdgesE,
                                   mlib_s32 *xStartsE,
                                   mlib_s32 *yStartsE,
                                   mlib_s32 *sidesE,
                                   mlib_u8  **lineAddr)
{
  mlib_type type      = mlib_ImageGetType(dst);
  mlib_s32  channels  = mlib_ImageGetChannels(dst);
  mlib_s32  dstStride = mlib_ImageGetStride(dst);
  mlib_s32  srcStride = mlib_ImageGetStride(src);
  mlib_s32  srcWidth  = mlib_ImageGetWidth(src);
  mlib_s32  srcHeight = mlib_ImageGetHeight(src);
  mlib_u8   *data     = (mlib_u8 *)mlib_ImageGetData(dst);
  mlib_s32  yStart    = sides[0];
  mlib_s32  yFinish   = sides[1];
  mlib_s32  yStartE   = sidesE[0];
  mlib_s32  yFinishE  = sidesE[1];
  mlib_s32  dX        = sidesE[2];
  mlib_s32  dY        = sidesE[3];
  mlib_d64  scale     = 1.0 / (mlib_d64) MLIB_PREC;
  mlib_s32  xSrc, ySrc, X, Y, xDelta, yDelta, xFlag, yFlag;
  mlib_s32  xLeft, xRight, xLeftE, xRightE;
  mlib_d64  t, u, pix0;
  mlib_s32  a00, a01, a10, a11;
  mlib_s32  i, j;

  data += (yStartE - 1) * dstStride;

  switch (type) {
    case MLIB_BYTE:
      MLIB_PROCESS_EDGES(MLIB_EDGE_BL_LINE, mlib_u8)
      break;

    case MLIB_SHORT:
      srcStride >>= 1;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BL_LINE, mlib_s16)
      break;

    case MLIB_INT:
      srcStride >>= 2;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BL_LINE, mlib_s32)
      break;

    case MLIB_FLOAT:
      srcStride >>= 2;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BL_LINE, mlib_f32)
      break;

    case MLIB_DOUBLE:
      srcStride >>= 3;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BL_LINE, mlib_d64)
      break;
    default :
      break;
  }
}

/***************************************************************/

void mlib_ImageAffineEdgeExtend_BC(mlib_image *dst,
                                   mlib_image *src,
                                   mlib_s32 *leftEdges,
                                   mlib_s32 *rightEdges,
                                   mlib_s32 *sides,
                                   mlib_s32 *leftEdgesE,
                                   mlib_s32 *rightEdgesE,
                                   mlib_s32 *xStartsE,
                                   mlib_s32 *yStartsE,
                                   mlib_s32 *sidesE,
                                   mlib_u8  **lineAddr)
{
  mlib_type type        = mlib_ImageGetType(dst);
  mlib_s32  channels    = mlib_ImageGetChannels(dst);
  mlib_s32  dstStride   = mlib_ImageGetStride(dst);
  mlib_s32  srcStride   = mlib_ImageGetStride(src);
  mlib_s32  srcWidth    = mlib_ImageGetWidth(src);
  mlib_s32  srcHeight   = mlib_ImageGetHeight(src);
  mlib_u8   *data       = (mlib_u8 *)mlib_ImageGetData(dst);
  mlib_s32  yStart      = sides[0];
  mlib_s32  yFinish     = sides[1];
  mlib_s32  yStartE     = sidesE[0];
  mlib_s32  yFinishE    = sidesE[1];
  mlib_s32  dX          = sidesE[2];
  mlib_s32  dY          = sidesE[3];
  mlib_s32  xSrc, ySrc, X, Y, xFlag, yFlag;
  mlib_s32  xLeft, xRight, xLeftE, xRightE;
  mlib_s32  i, j;

  mlib_d64  dx, dx_2, dx2, dx3_2, dx3_3;
  mlib_d64  dy, dy_2, dy2, dy3_2, dy3_3;
  mlib_d64  scale = 1.0 / (mlib_d64) MLIB_PREC;
  mlib_d64  xf0, xf1, xf2, xf3;
  mlib_d64  yf0, yf1, yf2, yf3;
  mlib_d64  c0, c1, c2, c3, val0;
  mlib_s32  filterpos;
  mlib_f32  *fptr;
  mlib_s32  xDelta0, xDelta1, xDelta2;
  mlib_s32  yDelta0, yDelta1, yDelta2;
  mlib_d64  sat = (mlib_d64)0x7F800000;

  data += (yStartE - 1) * dstStride;

  switch (type) {
    case MLIB_BYTE:
      MLIB_PROCESS_EDGES(MLIB_EDGE_BC_u8, mlib_u8)
      break;

    case MLIB_SHORT:
      srcStride >>= 1;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BC_s16, mlib_s16)
      break;

    case MLIB_INT:
      srcStride >>= 2;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BC, mlib_s32)
      break;

    case MLIB_FLOAT:
      srcStride >>= 2;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BC, mlib_f32)
      break;

    case MLIB_DOUBLE:
      srcStride >>= 3;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BC, mlib_d64)
      break;

   default:
     break;
  }
}

/***************************************************************/

void mlib_ImageAffineEdgeExtend_BC2(mlib_image *dst,
                                    mlib_image *src,
                                    mlib_s32 *leftEdges,
                                    mlib_s32 *rightEdges,
                                    mlib_s32 *sides,
                                    mlib_s32 *leftEdgesE,
                                    mlib_s32 *rightEdgesE,
                                    mlib_s32 *xStartsE,
                                    mlib_s32 *yStartsE,
                                    mlib_s32 *sidesE,
                                    mlib_u8  **lineAddr)
{
  mlib_type type        = mlib_ImageGetType(dst);
  mlib_s32  channels    = mlib_ImageGetChannels(dst);
  mlib_s32  dstStride   = mlib_ImageGetStride(dst);
  mlib_s32  srcStride   = mlib_ImageGetStride(src);
  mlib_s32  srcWidth    = mlib_ImageGetWidth(src);
  mlib_s32  srcHeight   = mlib_ImageGetHeight(src);
  mlib_u8   *data       = (mlib_u8 *)mlib_ImageGetData(dst);
  mlib_s32  yStart      = sides[0];
  mlib_s32  yFinish     = sides[1];
  mlib_s32  yStartE     = sidesE[0];
  mlib_s32  yFinishE    = sidesE[1];
  mlib_s32  dX          = sidesE[2];
  mlib_s32  dY          = sidesE[3];
  mlib_s32  xSrc, ySrc, X, Y, xFlag, yFlag;
  mlib_s32  xLeft, xRight, xLeftE, xRightE;
  mlib_s32  i, j;

  mlib_d64  dx, dx_2, dx2, dx3_2, dx3_3;
  mlib_d64  dy, dy_2, dy2, dy3_2, dy3_3;
  mlib_d64  scale = 1.0 / (mlib_d64) MLIB_PREC;
  mlib_d64  xf0, xf1, xf2, xf3;
  mlib_d64  yf0, yf1, yf2, yf3;
  mlib_d64  c0, c1, c2, c3, val0;
  mlib_s32  filterpos;
  mlib_f32  *fptr;
  mlib_s32  xDelta0, xDelta1, xDelta2;
  mlib_s32  yDelta0, yDelta1, yDelta2;
  mlib_d64  sat = (mlib_d64)0x7F800000;

  data += (yStartE - 1) * dstStride;

  switch (type) {
    case MLIB_BYTE:
      MLIB_PROCESS_EDGES(MLIB_EDGE_BC2_u8, mlib_u8)
      break;

    case MLIB_SHORT:
      srcStride >>= 1;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BC2_s16, mlib_s16)
      break;

    case MLIB_INT:
      srcStride >>= 2;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BC2, mlib_s32)
      break;

    case MLIB_FLOAT:
      srcStride >>= 2;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BC2, mlib_f32)
      break;

    case MLIB_DOUBLE:
      srcStride >>= 3;
      MLIB_PROCESS_EDGES(MLIB_EDGE_BC2, mlib_d64)
      break;
    default:
      break;
  }
}

/***************************************************************/
