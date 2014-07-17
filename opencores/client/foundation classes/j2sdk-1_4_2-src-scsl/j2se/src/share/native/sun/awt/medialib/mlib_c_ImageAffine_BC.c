/*
 * @(#)mlib_c_ImageAffine_BC.c	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageAffine_BC.c	1.14	00/01/31 SMI"
#endif /* __SUNPRO_C */

/*
 * FUNCTION
 *      mlib_c_ImageAffine_u8_1ch_bc
 *      mlib_c_ImageAffine_u8_2ch_bc
 *      mlib_c_ImageAffine_u8_3ch_bc
 *      mlib_c_ImageAffine_u8_4ch_bc
 *      mlib_c_ImageAffine_s16_1ch_bc
 *      mlib_c_ImageAffine_s16_2ch_bc
 *      mlib_c_ImageAffine_s16_3ch_bc
 *      mlib_c_ImageAffine_s16_4ch_bc
 *      mlib_c_ImageAffine_s32_1ch_bc
 *      mlib_c_ImageAffine_s32_2ch_bc
 *      mlib_c_ImageAffine_s32_3ch_bc
 *      mlib_c_ImageAffine_s32_4ch_bc
 *        - image affine transformation with Bicubic filtering
 * SYNOPSIS
 *      void mlib_c_ImageAffine_[u8|s16|s32]_?ch_bc
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
 *      the bicubic filtering.
 *
 */

#include "mlib_image.h"
#include "mlib_ImageFilters.h"

extern mlib_d64 mlib_U82D64[];

/***************************************************************/

#define MLIB_SHIFT       16
#define MLIB_PREC        (1 << MLIB_SHIFT)
#define MLIB_MASK        (MLIB_PREC - 1)

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

#define MLIB_AFFINEDEFPARAM2_BC MLIB_AFFINEDEFPARAM, mlib_s32 srcYStride, mlib_filter filter

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

#ifdef MLIB_USE_FTOI_CLAMPING

/***************************************************************/

#define SAT8(DST)                               \
  DST = ((mlib_s32)(val0 - sat) >> 24) ^ 0x80;

#define SAT16(DST)                      \
  DST = ((mlib_s32)val0) >> 16;

#define SAT32(DST)              \
  DST = (mlib_s32)val0;

/***************************************************************/

#else

/***************************************************************/

#define SAT8(DST)                               \
  val0 -= sat;                                  \
  if (val0 >= MLIB_S32_MAX)                     \
    DST = MLIB_U8_MAX;                          \
  else if (val0 <= MLIB_S32_MIN)                \
    DST = MLIB_U8_MIN;                          \
  else                                          \
    DST = ((mlib_s32)val0 >> 24) ^ 0x80;

#define SAT16(DST)                      \
  if (val0 >= MLIB_S32_MAX)             \
    DST = MLIB_S16_MAX;                 \
  else if (val0 <= MLIB_S32_MIN)        \
    DST = MLIB_S16_MIN;                 \
  else                                  \
    DST = ((mlib_s32)val0) >> 16;

#define SAT32(DST)                      \
  if (val0 >= MLIB_S32_MAX)             \
    DST = MLIB_S32_MAX;                 \
  else if (val0 <= MLIB_S32_MIN)        \
    DST = MLIB_S32_MIN;                 \
  else                                  \
    DST = (mlib_s32)val0;

#endif

/***************************************************************/

/* Test for the presence of any "1" bit in bits
   8 to 31 of val. If present, then val is either
   negative or >255. If over/underflows of 8 bits
   are uncommon, then this technique can be a win,
   since only a single test, rather than two, is
   necessary to determine if clamping is needed.
   On the other hand, if over/underflows are common,
   it adds an extra test.
*/
#define S32_TO_U8_SAT(DST)              \
  if (val0 & 0xffffff00) {              \
    if (val0 < MLIB_U8_MIN)             \
      DST = MLIB_U8_MIN;                \
    else                                \
      DST = MLIB_U8_MAX;                \
  } else {                              \
    DST = (mlib_u8)val0;                \
  }
  
#define S32_TO_S16_SAT(DST)             \
  if (val0 >= MLIB_S16_MAX)             \
    DST = MLIB_S16_MAX;                 \
  else if (val0 <= MLIB_S16_MIN)        \
    DST = MLIB_S16_MIN;                 \
  else                                  \
    DST = (mlib_s16)val0;

/***************************************************************/

#define MLIB_TYPE mlib_u8

#ifdef __sparc /* for SPARC, using floating-point multiplies is faster */

#define FILTER_SHIFT  4
#define FILTER_MASK   (((1 << 8) - 1) << 4)

/***************************************************************/

void  mlib_c_ImageAffine_u8_1ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_d64 sat = (mlib_d64)0x7F800000;
  const mlib_f32 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = mlib_filters_u8f_bc;
  } else {
    mlib_filters_table = mlib_filters_u8f_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_s32  filterpos;
    mlib_f32  *fptr;
    mlib_u8 s0, s1, s2, s3;

    CLIP(1)

    filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;
    fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

    xf0 = fptr[0];
    xf1 = fptr[1];
    xf2 = fptr[2];
    xf3 = fptr[3];

    filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;
    fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

    yf0 = fptr[0];
    yf1 = fptr[1];
    yf2 = fptr[2];
    yf3 = fptr[3];

    xSrc = (X >> MLIB_SHIFT)-1;
    ySrc = (Y >> MLIB_SHIFT)-1;

    srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
    s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
    s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dstPixelPtr <= (dstLineEnd-1); dstPixelPtr++)
  {
      X += dX;
      Y += dY;

      c0 = (mlib_U82D64[s0]*xf0 + mlib_U82D64[s1]*xf1 +
            mlib_U82D64[s2]*xf2 + mlib_U82D64[s3]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c1 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[1]]*xf1 +
            mlib_U82D64[srcPixelPtr[2]]*xf2 + mlib_U82D64[srcPixelPtr[3]]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[1]]*xf1 +
            mlib_U82D64[srcPixelPtr[2]]*xf2 + mlib_U82D64[srcPixelPtr[3]]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[1]]*xf1 +
            mlib_U82D64[srcPixelPtr[2]]*xf2 + mlib_U82D64[srcPixelPtr[3]]*xf3);

      filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

      filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      SAT8(dstPixelPtr[0])

      xSrc = (X >> MLIB_SHIFT)-1;
      ySrc = (Y >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
      s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

    }
    c0 = (mlib_U82D64[s0]*xf0 + mlib_U82D64[s1]*xf1 +
          mlib_U82D64[s2]*xf2 + mlib_U82D64[s3]*xf3);
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c1 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[1]]*xf1 +
          mlib_U82D64[srcPixelPtr[2]]*xf2 + mlib_U82D64[srcPixelPtr[3]]*xf3);
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c2 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[1]]*xf1 +
          mlib_U82D64[srcPixelPtr[2]]*xf2 + mlib_U82D64[srcPixelPtr[3]]*xf3);
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c3 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[1]]*xf1 +
          mlib_U82D64[srcPixelPtr[2]]*xf2 + mlib_U82D64[srcPixelPtr[3]]*xf3);

    val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

    SAT8(dstPixelPtr[0])
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_u8_2ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_d64 sat = (mlib_d64)0x7F800000;
  const mlib_f32 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = mlib_filters_u8f_bc;
  } else {
    mlib_filters_table = mlib_filters_u8f_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_f32  *fptr;
    mlib_u8 s0, s1, s2, s3;

    CLIP(2)

    for (k = 0; k < 2; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
      s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (; dPtr <= (dstLineEnd-1); dPtr += 2)
    {
        X1 += dX;
        Y1 += dY;

        c0 = (mlib_U82D64[s0]*xf0 + mlib_U82D64[s1]*xf1 +
              mlib_U82D64[s2]*xf2 + mlib_U82D64[s3]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c1 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[2]]*xf1 +
              mlib_U82D64[srcPixelPtr[4]]*xf2 + mlib_U82D64[srcPixelPtr[6]]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[2]]*xf1 +
              mlib_U82D64[srcPixelPtr[4]]*xf2 + mlib_U82D64[srcPixelPtr[6]]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[2]]*xf1 +
              mlib_U82D64[srcPixelPtr[4]]*xf2 + mlib_U82D64[srcPixelPtr[6]]*xf3);

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        SAT8(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
        s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

      }
      c0 = (mlib_U82D64[s0]*xf0 + mlib_U82D64[s1]*xf1 +
            mlib_U82D64[s2]*xf2 + mlib_U82D64[s3]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c1 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[2]]*xf1 +
            mlib_U82D64[srcPixelPtr[4]]*xf2 + mlib_U82D64[srcPixelPtr[6]]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[2]]*xf1 +
            mlib_U82D64[srcPixelPtr[4]]*xf2 + mlib_U82D64[srcPixelPtr[6]]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[2]]*xf1 +
            mlib_U82D64[srcPixelPtr[4]]*xf2 + mlib_U82D64[srcPixelPtr[6]]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

      SAT8(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_u8_3ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_d64 sat = (mlib_d64)0x7F800000;
  const mlib_f32 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = mlib_filters_u8f_bc;
  } else {
    mlib_filters_table = mlib_filters_u8f_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_f32  *fptr;
    mlib_u8 s0, s1, s2, s3;

    CLIP(3)

    for (k = 0; k < 3; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
      s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (; dPtr <= (dstLineEnd-1); dPtr += 3)
    {
        X1 += dX;
        Y1 += dY;

        c0 = (mlib_U82D64[s0]*xf0 + mlib_U82D64[s1]*xf1 +
              mlib_U82D64[s2]*xf2 + mlib_U82D64[s3]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c1 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[3]]*xf1 +
              mlib_U82D64[srcPixelPtr[6]]*xf2 + mlib_U82D64[srcPixelPtr[9]]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[3]]*xf1 +
              mlib_U82D64[srcPixelPtr[6]]*xf2 + mlib_U82D64[srcPixelPtr[9]]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[3]]*xf1 +
              mlib_U82D64[srcPixelPtr[6]]*xf2 + mlib_U82D64[srcPixelPtr[9]]*xf3);

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        SAT8(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
        s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

      }
      c0 = (mlib_U82D64[s0]*xf0 + mlib_U82D64[s1]*xf1 +
            mlib_U82D64[s2]*xf2 + mlib_U82D64[s3]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c1 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[3]]*xf1 +
            mlib_U82D64[srcPixelPtr[6]]*xf2 + mlib_U82D64[srcPixelPtr[9]]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[3]]*xf1 +
            mlib_U82D64[srcPixelPtr[6]]*xf2 + mlib_U82D64[srcPixelPtr[9]]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[3]]*xf1 +
            mlib_U82D64[srcPixelPtr[6]]*xf2 + mlib_U82D64[srcPixelPtr[9]]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

      SAT8(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_u8_4ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_d64 sat = (mlib_d64)0x7F800000;
  const mlib_f32 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = mlib_filters_u8f_bc;
  } else {
    mlib_filters_table = mlib_filters_u8f_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_f32  *fptr;
    mlib_u8 s0, s1, s2, s3;

    CLIP(4)

    for (k = 0; k < 4; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
      s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (; dPtr <= (dstLineEnd-1); dPtr += 4)
    {
        X1 += dX;
        Y1 += dY;

        c0 = (mlib_U82D64[s0]*xf0 + mlib_U82D64[s1]*xf1 +
              mlib_U82D64[s2]*xf2 + mlib_U82D64[s3]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c1 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[4]]*xf1 +
              mlib_U82D64[srcPixelPtr[8]]*xf2 + mlib_U82D64[srcPixelPtr[12]]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[4]]*xf1 +
              mlib_U82D64[srcPixelPtr[8]]*xf2 + mlib_U82D64[srcPixelPtr[12]]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[4]]*xf1 +
              mlib_U82D64[srcPixelPtr[8]]*xf2 + mlib_U82D64[srcPixelPtr[12]]*xf3);

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        SAT8(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
        s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

      }
      c0 = (mlib_U82D64[s0]*xf0 + mlib_U82D64[s1]*xf1 +
            mlib_U82D64[s2]*xf2 + mlib_U82D64[s3]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c1 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[4]]*xf1 +
            mlib_U82D64[srcPixelPtr[8]]*xf2 + mlib_U82D64[srcPixelPtr[12]]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[4]]*xf1 +
            mlib_U82D64[srcPixelPtr[8]]*xf2 + mlib_U82D64[srcPixelPtr[12]]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (mlib_U82D64[srcPixelPtr[0]]*xf0 + mlib_U82D64[srcPixelPtr[4]]*xf1 +
            mlib_U82D64[srcPixelPtr[8]]*xf2 + mlib_U82D64[srcPixelPtr[12]]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

      SAT8(dPtr[0])
    }
  }
}

/***************************************************************/

#else       /* for x86, using integer multiplies is faster */

#define FILTER_SHIFT  5
#define FILTER_MASK   (((1 << 8) - 1) << 3)

/***************************************************************/

#define SHIFT_X  12
#define ROUND_X  0 /* (1 << (SHIFT_X - 1)) */

#define SHIFT_Y  (14 + 14 - SHIFT_X)
#define ROUND_Y  (1 << (SHIFT_Y - 1))

/***************************************************************/

void  mlib_c_ImageAffine_u8_1ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_s16 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = (mlib_s16 *)mlib_filters_u8_bc;
  } else {
    mlib_filters_table = (mlib_s16 *)mlib_filters_u8_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32  xf0, xf1, xf2, xf3;
    mlib_s32  yf0, yf1, yf2, yf3;
    mlib_s32  c0, c1, c2, c3, val0;
    mlib_s32  filterpos;
    mlib_s16  *fptr;
    mlib_u8   s0, s1, s2, s3;

    CLIP(1)

    filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;
    fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

    xf0 = fptr[0];
    xf1 = fptr[1];
    xf2 = fptr[2];
    xf3 = fptr[3];

    filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;
    fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

    yf0 = fptr[0];
    yf1 = fptr[1];
    yf2 = fptr[2];
    yf3 = fptr[3];

    xSrc = (X >> MLIB_SHIFT)-1;
    ySrc = (Y >> MLIB_SHIFT)-1;

    srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
    s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
    s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dstPixelPtr <= (dstLineEnd-1); dstPixelPtr++)
  {
      X += dX;
      Y += dY;

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c1 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
            srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
            srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
            srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3 + ROUND_X) >> SHIFT_X;

      filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

      filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      S32_TO_U8_SAT(dstPixelPtr[0])

      xSrc = (X >> MLIB_SHIFT)-1;
      ySrc = (Y >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
      s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

    }
    c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c1 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
          srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3 + ROUND_X) >> SHIFT_X;
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
          srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3 + ROUND_X) >> SHIFT_X;
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
          srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3 + ROUND_X) >> SHIFT_X;

    val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

    S32_TO_U8_SAT(dstPixelPtr[0])
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_u8_2ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_s16 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = (mlib_s16 *)mlib_filters_u8_bc;
  } else {
    mlib_filters_table = (mlib_s16 *)mlib_filters_u8_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32  xf0, xf1, xf2, xf3;
    mlib_s32  yf0, yf1, yf2, yf3;
    mlib_s32  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_s16  *fptr;
    mlib_u8   s0, s1, s2, s3;

    CLIP(2)

    for (k = 0; k < 2; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
      s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (; dPtr <= (dstLineEnd-1); dPtr += 2)
    {
        X1 += dX;
        Y1 += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c1 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
              srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
              srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
              srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3 + ROUND_X) >> SHIFT_X;

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        S32_TO_U8_SAT(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
        s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

      }
      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c1 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
            srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
            srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
            srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3 + ROUND_X) >> SHIFT_X;

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

      S32_TO_U8_SAT(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_u8_3ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_s16 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = (mlib_s16 *)mlib_filters_u8_bc;
  } else {
    mlib_filters_table = (mlib_s16 *)mlib_filters_u8_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32  xf0, xf1, xf2, xf3;
    mlib_s32  yf0, yf1, yf2, yf3;
    mlib_s32  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_s16  *fptr;
    mlib_u8   s0, s1, s2, s3;

    CLIP(3)

    for (k = 0; k < 3; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
      s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (; dPtr <= (dstLineEnd-1); dPtr += 3)
    {
        X1 += dX;
        Y1 += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c1 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
              srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
              srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
              srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3 + ROUND_X) >> SHIFT_X;

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        S32_TO_U8_SAT(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
        s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

      }
      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c1 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
            srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
            srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
            srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3 + ROUND_X) >> SHIFT_X;

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

      S32_TO_U8_SAT(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_u8_4ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_s16 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = (mlib_s16 *)mlib_filters_u8_bc;
  } else {
    mlib_filters_table = (mlib_s16 *)mlib_filters_u8_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32  xf0, xf1, xf2, xf3;
    mlib_s32  yf0, yf1, yf2, yf3;
    mlib_s32  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_s16  *fptr;
    mlib_u8   s0, s1, s2, s3;

    CLIP(4)

    for (k = 0; k < 4; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
      s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (; dPtr <= (dstLineEnd-1); dPtr += 4)
    {
        X1 += dX;
        Y1 += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c1 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
              srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
              srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
              srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3 + ROUND_X) >> SHIFT_X;

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        S32_TO_U8_SAT(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
        s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

      }
      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c1 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
            srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
            srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
            srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3 + ROUND_X) >> SHIFT_X;

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

      S32_TO_U8_SAT(dPtr[0])
    }
  }
}

#endif /* #ifdef __sparc */

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s16

#ifdef __sparc /* for SPARC, using floating-point multiplies is faster */

#undef  FILTER_SHIFT
#define FILTER_SHIFT 3
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 9) - 1) << 4)

/***************************************************************/

void  mlib_c_ImageAffine_s16_1ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_f32 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = mlib_filters_s16f_bc;
  } else {
    mlib_filters_table = mlib_filters_s16f_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_s32  filterpos;
    mlib_f32  *fptr;
    mlib_s32  s0, s1, s2, s3;
    mlib_s32  s4, s5, s6, s7;

    CLIP(1)

    filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;
    fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

    xf0 = fptr[0];
    xf1 = fptr[1];
    xf2 = fptr[2];
    xf3 = fptr[3];

    filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;
    fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

    yf0 = fptr[0];
    yf1 = fptr[1];
    yf2 = fptr[2];
    yf3 = fptr[3];

    xSrc = (X >> MLIB_SHIFT)-1;
    ySrc = (Y >> MLIB_SHIFT)-1;

    srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
    s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
    s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    s4 = srcPixelPtr[0]; s5 = srcPixelPtr[1];
    s6 = srcPixelPtr[2]; s7 = srcPixelPtr[3];

    for (; dstPixelPtr <= (dstLineEnd - 1); dstPixelPtr++)
  {

      X += dX;
      Y += dY;

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
            srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
            srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3);

      filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

      filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      SAT16(dstPixelPtr[0])

      xSrc = (X >> MLIB_SHIFT)-1;
      ySrc = (Y >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
      s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[1];
      s6 = srcPixelPtr[2]; s7 = srcPixelPtr[3];
    }

    c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
    c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
          srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3);
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
          srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3);

    val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
    SAT16(dstPixelPtr[0])
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_s16_2ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_f32 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = mlib_filters_s16f_bc;
  } else {
    mlib_filters_table = mlib_filters_s16f_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_f32  *fptr;
    mlib_s32  s0, s1, s2, s3;
    mlib_s32  s4, s5, s6, s7;

    CLIP(2)

    for (k = 0; k < 2; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
      s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[2];
      s6 = srcPixelPtr[4]; s7 = srcPixelPtr[6];

      for (; dPtr <= (dstLineEnd - 1); dPtr += 2)
    {

        X1 += dX;
        Y1 += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
              srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
              srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3);

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        SAT16(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
        s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        s4 = srcPixelPtr[0]; s5 = srcPixelPtr[2];
        s6 = srcPixelPtr[4]; s7 = srcPixelPtr[6];
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
            srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
            srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
      SAT16(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_s16_3ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_f32 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = mlib_filters_s16f_bc;
  } else {
    mlib_filters_table = mlib_filters_s16f_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_f32  *fptr;
    mlib_s32  s0, s1, s2, s3;
    mlib_s32  s4, s5, s6, s7;

    CLIP(3)

    for (k = 0; k < 3; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
      s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[3];
      s6 = srcPixelPtr[6]; s7 = srcPixelPtr[9];

      for (; dPtr <= (dstLineEnd - 1); dPtr += 3)
    {

        X1 += dX;
        Y1 += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
              srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
              srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3);

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        SAT16(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
        s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        s4 = srcPixelPtr[0]; s5 = srcPixelPtr[3];
        s6 = srcPixelPtr[6]; s7 = srcPixelPtr[9];
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
            srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
            srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
      SAT16(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_s16_4ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_f32 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = mlib_filters_s16f_bc;
  } else {
    mlib_filters_table = mlib_filters_s16f_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_f32  *fptr;
    mlib_s32  s0, s1, s2, s3;
    mlib_s32  s4, s5, s6, s7;

    CLIP(4)

    for (k = 0; k < 4; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
      s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[4];
      s6 = srcPixelPtr[8]; s7 = srcPixelPtr[12];

      for (; dPtr <= (dstLineEnd - 1); dPtr += 4)
    {

        X1 += dX;
        Y1 += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
              srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
              srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3);

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        SAT16(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
        s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        s4 = srcPixelPtr[0]; s5 = srcPixelPtr[4];
        s6 = srcPixelPtr[8]; s7 = srcPixelPtr[12];
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
            srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
            srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
      SAT16(dPtr[0])
    }
  }
}

/***************************************************************/

#else       /* for x86, using integer multiplies is faster */

#undef  FILTER_SHIFT
#define FILTER_SHIFT 4
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 9) - 1) << 3)

/***************************************************************/

#undef SHIFT_X
#undef ROUND_X
#undef SHIFT_Y
#undef ROUND_Y

/***************************************************************/

#define SHIFT_X  15
#define ROUND_X  0 /* (1 << (SHIFT_X - 1)) */

#define SHIFT_Y  (15 + 15 - SHIFT_X)
#define ROUND_Y  (1 << (SHIFT_Y - 1))

/***************************************************************/

void  mlib_c_ImageAffine_s16_1ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_s16 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = (mlib_s16 *)mlib_filters_s16_bc;
  } else {
    mlib_filters_table = (mlib_s16 *)mlib_filters_s16_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32  xf0, xf1, xf2, xf3;
    mlib_s32  yf0, yf1, yf2, yf3;
    mlib_s32  c0, c1, c2, c3, val0;
    mlib_s32  filterpos;
    mlib_s16  *fptr;
    mlib_s32  s0, s1, s2, s3;
    mlib_s32  s4, s5, s6, s7;

    CLIP(1)

    filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;
    fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

    xf0 = fptr[0];
    xf1 = fptr[1];
    xf2 = fptr[2];
    xf3 = fptr[3];

    filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;
    fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

    yf0 = fptr[0];
    yf1 = fptr[1];
    yf2 = fptr[2];
    yf3 = fptr[3];

    xSrc = (X >> MLIB_SHIFT)-1;
    ySrc = (Y >> MLIB_SHIFT)-1;

    srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
    s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
    s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    s4 = srcPixelPtr[0]; s5 = srcPixelPtr[1];
    s6 = srcPixelPtr[2]; s7 = srcPixelPtr[3];

    for (; dstPixelPtr <= (dstLineEnd - 1); dstPixelPtr++)
  {

      X += dX;
      Y += dY;

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
            srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
            srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3 + ROUND_X) >> SHIFT_X;

      filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

      filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      S32_TO_S16_SAT(dstPixelPtr[0])

      xSrc = (X >> MLIB_SHIFT)-1;
      ySrc = (Y >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
      s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[1];
      s6 = srcPixelPtr[2]; s7 = srcPixelPtr[3];
    }

    c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
    c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3 + ROUND_X) >> SHIFT_X;
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
          srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3 + ROUND_X) >> SHIFT_X;
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
          srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3 + ROUND_X) >> SHIFT_X;

    val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;
    S32_TO_S16_SAT(dstPixelPtr[0])
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_s16_2ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_s16 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = (mlib_s16 *)mlib_filters_s16_bc;
  } else {
    mlib_filters_table = (mlib_s16 *)mlib_filters_s16_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32  xf0, xf1, xf2, xf3;
    mlib_s32  yf0, yf1, yf2, yf3;
    mlib_s32  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_s16  *fptr;
    mlib_s32  s0, s1, s2, s3;
    mlib_s32  s4, s5, s6, s7;

    CLIP(2)

    for (k = 0; k < 2; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
      s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[2];
      s6 = srcPixelPtr[4]; s7 = srcPixelPtr[6];

      for (; dPtr <= (dstLineEnd - 1); dPtr += 2)
    {

        X1 += dX;
        Y1 += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
              srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
              srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3 + ROUND_X) >> SHIFT_X;

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        S32_TO_S16_SAT(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
        s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        s4 = srcPixelPtr[0]; s5 = srcPixelPtr[2];
        s6 = srcPixelPtr[4]; s7 = srcPixelPtr[6];
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
            srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
            srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3 + ROUND_X) >> SHIFT_X;

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;
      S32_TO_S16_SAT(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_s16_3ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_s16 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = (mlib_s16 *)mlib_filters_s16_bc;
  } else {
    mlib_filters_table = (mlib_s16 *)mlib_filters_s16_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32  xf0, xf1, xf2, xf3;
    mlib_s32  yf0, yf1, yf2, yf3;
    mlib_s32  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_s16  *fptr;
    mlib_s32  s0, s1, s2, s3;
    mlib_s32  s4, s5, s6, s7;

    CLIP(3)

    for (k = 0; k < 3; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
      s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[3];
      s6 = srcPixelPtr[6]; s7 = srcPixelPtr[9];

      for (; dPtr <= (dstLineEnd - 1); dPtr += 3)
    {

        X1 += dX;
        Y1 += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
              srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
              srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3 + ROUND_X) >> SHIFT_X;

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        S32_TO_S16_SAT(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
        s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        s4 = srcPixelPtr[0]; s5 = srcPixelPtr[3];
        s6 = srcPixelPtr[6]; s7 = srcPixelPtr[9];
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
            srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
            srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3 + ROUND_X) >> SHIFT_X;

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;
      S32_TO_S16_SAT(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_s16_4ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  const mlib_s16 *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = (mlib_s16 *)mlib_filters_s16_bc;
  } else {
    mlib_filters_table = (mlib_s16 *)mlib_filters_s16_bc2;
  }

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_s32  xf0, xf1, xf2, xf3;
    mlib_s32  yf0, yf1, yf2, yf3;
    mlib_s32  c0, c1, c2, c3, val0;
    mlib_s32  filterpos, k;
    mlib_s16  *fptr;
    mlib_s32  s0, s1, s2, s3;
    mlib_s32  s4, s5, s6, s7;

    CLIP(4)

    for (k = 0; k < 4; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      xf0 = fptr[0];
      xf1 = fptr[1];
      xf2 = fptr[2];
      xf3 = fptr[3];

      filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
      fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

      yf0 = fptr[0];
      yf1 = fptr[1];
      yf2 = fptr[2];
      yf3 = fptr[3];

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
      s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[4];
      s6 = srcPixelPtr[8]; s7 = srcPixelPtr[12];

      for (; dPtr <= (dstLineEnd - 1); dPtr += 4)
    {

        X1 += dX;
        Y1 += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
              srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3 + ROUND_X) >> SHIFT_X;
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
              srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3 + ROUND_X) >> SHIFT_X;

        filterpos = (X1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        xf0 = fptr[0];
        xf1 = fptr[1];
        xf2 = fptr[2];
        xf3 = fptr[3];

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;

        filterpos = (Y1 >> FILTER_SHIFT) & FILTER_MASK;
        fptr = (mlib_s16 *) ((mlib_u8 *)mlib_filters_table + filterpos);

        yf0 = fptr[0];
        yf1 = fptr[1];
        yf2 = fptr[2];
        yf3 = fptr[3];

        S32_TO_S16_SAT(dPtr[0])

        xSrc = (X1 >> MLIB_SHIFT)-1;
        ySrc = (Y1 >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
        s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        s4 = srcPixelPtr[0]; s5 = srcPixelPtr[4];
        s6 = srcPixelPtr[8]; s7 = srcPixelPtr[12];
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3 + ROUND_X) >> SHIFT_X;
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
            srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3 + ROUND_X) >> SHIFT_X;
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
            srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3 + ROUND_X) >> SHIFT_X;

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3 + ROUND_Y) >> SHIFT_Y;
      S32_TO_S16_SAT(dPtr[0])
    }
  }
}

#endif /* #ifdef __sparc */

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s32

/***************************************************************/

#define CREATE_COEF_BICUBIC( X, Y, OPERATOR )   \
  dx = (X & MLIB_MASK) * scale;                 \
  dy = (Y & MLIB_MASK) * scale;                 \
  dx_2  = 0.5  * dx;    dy_2  = 0.5  * dy;      \
  dx2   = dx   * dx;    dy2   = dy   * dy;      \
  dx3_2 = dx_2 * dx2;   dy3_2 = dy_2 * dy2;     \
  dx3_3 = 3.0  * dx3_2; dy3_3 = 3.0  * dy3_2;   \
                                                \
  xf0 = dx2 - dx3_2 - dx_2;                     \
  xf1 = dx3_3 - 2.5 * dx2 + 1.0;                \
  xf2 = 2.0 * dx2 - dx3_3 + dx_2;               \
  xf3 = dx3_2 - 0.5 * dx2;                      \
                                                \
  OPERATOR;                                     \
                                                \
  yf0 = dy2 - dy3_2 - dy_2;                     \
  yf1 = dy3_3 - 2.5 * dy2 + 1.0;                \
  yf2 = 2.0 * dy2 - dy3_3 + dy_2;               \
  yf3 = dy3_2 - 0.5 * dy2;

/***************************************************************/

#define CREATE_COEF_BICUBIC_2( X, Y, OPERATOR )         \
  dx = (X & MLIB_MASK) * scale;                         \
  dy = (Y & MLIB_MASK) * scale;                         \
  dx2   = dx  * dx;    dy2   = dy  * dy;                \
  dx3_2 = dx  * dx2;   dy3_2 = dy  * dy2;               \
  dx3_3 = 2.0 * dx2;   dy3_3 = 2.0 * dy2;               \
                                                        \
  xf0 = - dx3_2 + dx3_3 - dx;                           \
  xf1 =   dx3_2 - dx3_3 + 1.0;                          \
  xf2 = - dx3_2 + dx2   + dx;                           \
  xf3 =   dx3_2 - dx2;                                  \
                                                        \
  OPERATOR;                                             \
                                                        \
  yf0 = - dy3_2 + dy3_3 - dy;                           \
  yf1 =   dy3_2 - dy3_3 + 1.0;                          \
  yf2 = - dy3_2 + dy2   + dy;                           \
  yf3 =   dy3_2 - dy2;

/***************************************************************/

void  mlib_c_ImageAffine_s32_1ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  dx, dx_2, dx2, dx3_2, dx3_3;
    mlib_d64  dy, dy_2, dy2, dy3_2, dy3_3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_d64  scale = 1. / 65536.;
    mlib_s32  s0, s1, s2, s3;
    mlib_s32  s4, s5, s6, s7;

    CLIP(1)

    if ( filter == MLIB_BICUBIC ) {
      CREATE_COEF_BICUBIC( X, Y, ; )
    } else {
      CREATE_COEF_BICUBIC_2( X, Y, ; )
    }

    xSrc = (X >> MLIB_SHIFT)-1;
    ySrc = (Y >> MLIB_SHIFT)-1;

    srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
    s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
    s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    s4 = srcPixelPtr[0]; s5 = srcPixelPtr[1];
    s6 = srcPixelPtr[2]; s7 = srcPixelPtr[3];

    if ( filter == MLIB_BICUBIC ) {
      for (; dstPixelPtr <= (dstLineEnd - 1); dstPixelPtr++)
    {

        X += dX;
        Y += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
              srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
              srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3);

        CREATE_COEF_BICUBIC( X, Y, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )

        SAT32(dstPixelPtr[0])

        xSrc = (X >> MLIB_SHIFT)-1;
        ySrc = (Y >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
        s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        s4 = srcPixelPtr[0]; s5 = srcPixelPtr[1];
        s6 = srcPixelPtr[2]; s7 = srcPixelPtr[3];
      }
    } else {
      for (; dstPixelPtr <= (dstLineEnd - 1); dstPixelPtr++)
    {

        X += dX;
        Y += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
              srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3);
        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
              srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3);

        CREATE_COEF_BICUBIC_2( X, Y, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )

        SAT32(dstPixelPtr[0])

        xSrc = (X >> MLIB_SHIFT)-1;
        ySrc = (Y >> MLIB_SHIFT)-1;

        srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
        s0 = srcPixelPtr[0]; s1 = srcPixelPtr[1];
        s2 = srcPixelPtr[2]; s3 = srcPixelPtr[3];

        srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
        s4 = srcPixelPtr[0]; s5 = srcPixelPtr[1];
        s6 = srcPixelPtr[2]; s7 = srcPixelPtr[3];
      }
    }

    c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
    c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
          srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3);
    srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
    c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[1]*xf1 +
          srcPixelPtr[2]*xf2 + srcPixelPtr[3]*xf3);

    val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
    SAT32(dstPixelPtr[0])
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_s32_2ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_d64  dx, dx_2, dx2, dx3_2, dx3_3;
    mlib_d64  dy, dy_2, dy2, dy3_2, dy3_3;
    mlib_d64  scale = 1. / 65536.;
    mlib_s32  s0, s1, s2, s3, k;
    mlib_s32  s4, s5, s6, s7;

    CLIP(2)

    for (k = 0; k < 2; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      if ( filter == MLIB_BICUBIC ) {
        CREATE_COEF_BICUBIC( X1, Y1, ;)
      } else {
        CREATE_COEF_BICUBIC_2( X1, Y1, ;)
      }

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
      s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[2];
      s6 = srcPixelPtr[4]; s7 = srcPixelPtr[6];

      if ( filter == MLIB_BICUBIC ) {
        for (; dPtr <= (dstLineEnd - 1); dPtr += 2)
      {
          X1 += dX;
          Y1 += dY;

          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
                srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
                srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3);

          CREATE_COEF_BICUBIC(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )

          SAT32(dPtr[0])

          xSrc = (X1 >> MLIB_SHIFT)-1;
          ySrc = (Y1 >> MLIB_SHIFT)-1;

          srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
          s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
          s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          s4 = srcPixelPtr[0]; s5 = srcPixelPtr[2];
          s6 = srcPixelPtr[4]; s7 = srcPixelPtr[6];
        }
      } else {
        for (; dPtr <= (dstLineEnd - 1); dPtr += 2)
      {
          X1 += dX;
          Y1 += dY;

          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
                srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
                srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3);

          CREATE_COEF_BICUBIC_2(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )

          SAT32(dPtr[0])

          xSrc = (X1 >> MLIB_SHIFT)-1;
          ySrc = (Y1 >> MLIB_SHIFT)-1;

          srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
          s0 = srcPixelPtr[0]; s1 = srcPixelPtr[2];
          s2 = srcPixelPtr[4]; s3 = srcPixelPtr[6];

          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          s4 = srcPixelPtr[0]; s5 = srcPixelPtr[2];
          s6 = srcPixelPtr[4]; s7 = srcPixelPtr[6];
        }
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
            srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[2]*xf1 +
            srcPixelPtr[4]*xf2 + srcPixelPtr[6]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
      SAT32(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_s32_3ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_d64  dx, dx_2, dx2, dx3_2, dx3_3;
    mlib_d64  dy, dy_2, dy2, dy3_2, dy3_3;
    mlib_d64  scale = 1. / 65536.;
    mlib_s32  s0, s1, s2, s3, k;
    mlib_s32  s4, s5, s6, s7;

    CLIP(3)

    for (k = 0; k < 3; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      if ( filter == MLIB_BICUBIC ) {
        CREATE_COEF_BICUBIC( X1, Y1, ; )
      } else {
        CREATE_COEF_BICUBIC_2( X1, Y1, ; )
      }

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
      s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[3];
      s6 = srcPixelPtr[6]; s7 = srcPixelPtr[9];

      if ( filter == MLIB_BICUBIC ) {
        for (; dPtr <= (dstLineEnd - 1); dPtr += 3)
      {
          X1 += dX;
          Y1 += dY;

          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
                srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
                srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3);

          CREATE_COEF_BICUBIC(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )

          SAT32(dPtr[0])

          xSrc = (X1 >> MLIB_SHIFT)-1;
          ySrc = (Y1 >> MLIB_SHIFT)-1;

          srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
          s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
          s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          s4 = srcPixelPtr[0]; s5 = srcPixelPtr[3];
          s6 = srcPixelPtr[6]; s7 = srcPixelPtr[9];
        }
      } else {
        for (; dPtr <= (dstLineEnd - 1); dPtr += 3)
      {
          X1 += dX;
          Y1 += dY;

          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
                srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
                srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3);

          CREATE_COEF_BICUBIC_2(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )

          SAT32(dPtr[0])

          xSrc = (X1 >> MLIB_SHIFT)-1;
          ySrc = (Y1 >> MLIB_SHIFT)-1;

          srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
          s0 = srcPixelPtr[0]; s1 = srcPixelPtr[3];
          s2 = srcPixelPtr[6]; s3 = srcPixelPtr[9];

          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          s4 = srcPixelPtr[0]; s5 = srcPixelPtr[3];
          s6 = srcPixelPtr[6]; s7 = srcPixelPtr[9];
        }
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
            srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[3]*xf1 +
            srcPixelPtr[6]*xf2 + srcPixelPtr[9]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
      SAT32(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_c_ImageAffine_s32_4ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR

  for (j = yStart; j <= yFinish; j++)
  {
    mlib_d64  xf0, xf1, xf2, xf3;
    mlib_d64  yf0, yf1, yf2, yf3;
    mlib_d64  c0, c1, c2, c3, val0;
    mlib_d64  dx, dx_2, dx2, dx3_2, dx3_3;
    mlib_d64  dy, dy_2, dy2, dy3_2, dy3_3;
    mlib_d64  scale = 1. / 65536.;
    mlib_s32  s0, s1, s2, s3, k;
    mlib_s32  s4, s5, s6, s7;

    CLIP(4)

    for (k = 0; k < 4; k++) {
      mlib_s32 X1 = X;
      mlib_s32 Y1 = Y;
      MLIB_TYPE *dPtr = dstPixelPtr + k;

      if ( filter == MLIB_BICUBIC ) {
        CREATE_COEF_BICUBIC( X1, Y1, ; )
      } else {
        CREATE_COEF_BICUBIC_2( X1, Y1, ; )
      }

      xSrc = (X1 >> MLIB_SHIFT)-1;
      ySrc = (Y1 >> MLIB_SHIFT)-1;

      srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
      s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
      s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      s4 = srcPixelPtr[0]; s5 = srcPixelPtr[4];
      s6 = srcPixelPtr[8]; s7 = srcPixelPtr[12];

      if ( filter == MLIB_BICUBIC ) {
        for (; dPtr <= (dstLineEnd - 1); dPtr += 4)
      {
          X1 += dX;
          Y1 += dY;

          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
                srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
                srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3);

          CREATE_COEF_BICUBIC(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )

          SAT32(dPtr[0])

          xSrc = (X1 >> MLIB_SHIFT)-1;
          ySrc = (Y1 >> MLIB_SHIFT)-1;

          srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
          s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
          s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          s4 = srcPixelPtr[0]; s5 = srcPixelPtr[4];
          s6 = srcPixelPtr[8]; s7 = srcPixelPtr[12];
        }
      } else {
        for (; dPtr <= (dstLineEnd - 1); dPtr += 4)
      {
          X1 += dX;
          Y1 += dY;

          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
                srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3);
          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
                srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3);

          CREATE_COEF_BICUBIC_2(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )

          SAT32(dPtr[0])

          xSrc = (X1 >> MLIB_SHIFT)-1;
          ySrc = (Y1 >> MLIB_SHIFT)-1;

          srcPixelPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
          s0 = srcPixelPtr[0]; s1 = srcPixelPtr[4];
          s2 = srcPixelPtr[8]; s3 = srcPixelPtr[12];

          srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
          s4 = srcPixelPtr[0]; s5 = srcPixelPtr[4];
          s6 = srcPixelPtr[8]; s7 = srcPixelPtr[12];
        }
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c2 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
            srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3);
      srcPixelPtr = (MLIB_TYPE*)((mlib_addr)srcPixelPtr + srcYStride);
      c3 = (srcPixelPtr[0]*xf0 + srcPixelPtr[4]*xf1 +
            srcPixelPtr[8]*xf2 + srcPixelPtr[12]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
      SAT32(dPtr[0])
    }
  }
}

/***************************************************************/
