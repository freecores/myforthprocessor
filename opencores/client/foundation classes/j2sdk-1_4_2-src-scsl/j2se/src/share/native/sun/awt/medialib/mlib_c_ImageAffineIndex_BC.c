/*
 * @(#)mlib_c_ImageAffineIndex_BC.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageAffineIndex_BC.c	1.13	00/02/07 SMI"
#endif /* __SUNPRO_C */

#include <stdlib.h>
#include "mlib_image.h"
#include "mlib_ImageColormap.h"
#include "mlib_ImageFilters.h"

/***************************************************************/

#define MLIB_LIMIT  512
#define MLIB_SHIFT   16
#define MLIB_PREC    (1 << MLIB_SHIFT)
#define MLIB_MASK    (MLIB_PREC - 1)

/***************************************************************/

#define DECLAREVAR()                    \
  mlib_s32  xLeft, xRight, X, Y;        \
  mlib_s32  xSrc, ySrc;                 \
  mlib_s32  yStart = sides[0];          \
  mlib_s32  yFinish = sides[1];         \
  mlib_s32  dX = sides[2];              \
  mlib_s32  dY = sides[3];              \
  mlib_s32  max_xsize = sides[4];       \
  MLIB_TYPE *srcIndexPtr;               \
  MLIB_TYPE *dstIndexPtr;               \
  mlib_s32  i, j;                       \
  mlib_d64  xf0, xf1, xf2, xf3;         \
  mlib_d64  yf0, yf1, yf2, yf3;         \
  mlib_d64  c0, c1, c2, c3, val0;       \
  mlib_s32  filterpos;                  \
  mlib_f32  *fptr;                      \
  mlib_d64 s0, s1, s2, s3;

/***************************************************************/

#define CLIP()                                  \
  dstData += dstYStride;                        \
  xLeft = leftEdges[j];                         \
  xRight = rightEdges[j];                       \
  X = xStarts[j];                               \
  Y = yStarts[j];                               \
  if (xLeft > xRight)                           \
    continue;                                   \
  dstIndexPtr = (MLIB_TYPE *)dstData + xLeft;

/***************************************************************/

#define GET_FILTERS_KOEF()                                              \
  filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);      \
                                                                        \
  xf0 = fptr[0];                                                        \
  xf1 = fptr[1];                                                        \
  xf2 = fptr[2];                                                        \
  xf3 = fptr[3];                                                        \
                                                                        \
  filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_table + filterpos);      \
                                                                        \
  yf0 = fptr[0];                                                        \
  yf1 = fptr[1];                                                        \
  yf2 = fptr[2];                                                        \
  yf3 = fptr[3];

/***************************************************************/

#define GET_POINTER()                                   \
  xSrc = (X >> MLIB_SHIFT)-1;                           \
  ySrc = (Y >> MLIB_SHIFT)-1;                           \
  srcIndexPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;

/***************************************************************/

#define LOAD_FIRST_ROW(nchan, chan)             \
  s0 = *(lut + srcIndexPtr[0]*nchan + chan);    \
  s1 = *(lut + srcIndexPtr[1]*nchan + chan);    \
  s2 = *(lut + srcIndexPtr[2]*nchan + chan);    \
  s3 = *(lut + srcIndexPtr[3]*nchan + chan);

/***************************************************************/

#define COUNT_NEXT_ROW(dst, nchan, chan)                                \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr + srcYStride);      \
  dst = ((*(lut + srcIndexPtr[0]*nchan + chan))*xf0 +                   \
         (*(lut + srcIndexPtr[1]*nchan + chan))*xf1 +                   \
         (*(lut + srcIndexPtr[2]*nchan + chan))*xf2 +                   \
         (*(lut + srcIndexPtr[3]*nchan + chan))*xf3);

/***************************************************************/

#ifdef MLIB_USE_FTOI_CLAMPING
#define STORE_SAT_VALUE_U8(ind)                         \
  dstPixelPtr[ind] = ((int)(val0 - sat) >> 24) ^ 0x80;
#else
#define STORE_SAT_VALUE_U8(ind)                         \
  val0 -= sat;                                          \
  if (val0 >= MLIB_S32_MAX)                             \
    dstPixelPtr[ind] = MLIB_U8_MAX;                     \
  else if (val0 <= MLIB_S32_MIN)                        \
    dstPixelPtr[ind] = MLIB_U8_MIN;                     \
  else                                                  \
    dstPixelPtr[ind] = ((mlib_s32)val0 >> 24) ^ 0x80;
#endif

/***************************************************************/

#ifdef MLIB_USE_FTOI_CLAMPING
#define STORE_SAT_VALUE_S16(ind)                \
  dstPixelPtr[ind] = ((int)(val0)) >> 16;
#else
#define STORE_SAT_VALUE_S16(ind)                \
  if (val0 >= MLIB_S32_MAX)                     \
    dstPixelPtr[ind] = MLIB_S16_MAX;            \
  else if (val0 <= MLIB_S32_MIN)                \
    dstPixelPtr[ind] = MLIB_S16_MIN;            \
  else                                          \
    dstPixelPtr[ind] = (mlib_s32)val0 >> 16;
#endif

/***************************************************************/

#define MAKE_BC_3CH(lut_format)                                       \
  X += dX;                                                            \
  Y += dY;                                                            \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 3, 0)                                            \
  COUNT_NEXT_ROW(c2, 3, 0)                                            \
  COUNT_NEXT_ROW(c3, 3, 0)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr - 3*srcYStride);  \
  LOAD_FIRST_ROW(3, 1)                                                \
  STORE_SAT_VALUE_##lut_format(0)                                     \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 3, 1)                                            \
  COUNT_NEXT_ROW(c2, 3, 1)                                            \
  COUNT_NEXT_ROW(c3, 3, 1)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr - 3*srcYStride);  \
  LOAD_FIRST_ROW(3, 2)                                                \
  STORE_SAT_VALUE_##lut_format(1)                                     \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 3, 2)                                            \
  COUNT_NEXT_ROW(c2, 3, 2)                                            \
  COUNT_NEXT_ROW(c3, 3, 2)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  GET_FILTERS_KOEF()                                                  \
  GET_POINTER()                                                       \
  LOAD_FIRST_ROW(3, 0)                                                \
  STORE_SAT_VALUE_##lut_format(2)

/***************************************************************/

#define MAKE_LAST_PIXEL_BC_3CH(lut_format)                            \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 3, 0)                                            \
  COUNT_NEXT_ROW(c2, 3, 0)                                            \
  COUNT_NEXT_ROW(c3, 3, 0)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr - 3*srcYStride);  \
  LOAD_FIRST_ROW(3, 1)                                                \
  STORE_SAT_VALUE_##lut_format(0)                                     \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 3, 1)                                            \
  COUNT_NEXT_ROW(c2, 3, 1)                                            \
  COUNT_NEXT_ROW(c3, 3, 1)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr - 3*srcYStride);  \
  LOAD_FIRST_ROW(3, 2)                                                \
  STORE_SAT_VALUE_##lut_format(1)                                     \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 3, 2)                                            \
  COUNT_NEXT_ROW(c2, 3, 2)                                            \
  COUNT_NEXT_ROW(c3, 3, 2)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  STORE_SAT_VALUE_##lut_format(2)

/***************************************************************/

#define MAKE_BC_4CH(lut_format)                                       \
  X += dX;                                                            \
  Y += dY;                                                            \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 4, 0)                                            \
  COUNT_NEXT_ROW(c2, 4, 0)                                            \
  COUNT_NEXT_ROW(c3, 4, 0)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr - 3*srcYStride);  \
  LOAD_FIRST_ROW(4, 1)                                                \
  STORE_SAT_VALUE_##lut_format(0)                                     \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 4, 1)                                            \
  COUNT_NEXT_ROW(c2, 4, 1)                                            \
  COUNT_NEXT_ROW(c3, 4, 1)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr - 3*srcYStride);  \
  LOAD_FIRST_ROW(4, 2)                                                \
  STORE_SAT_VALUE_##lut_format(1)                                     \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 4, 2)                                            \
  COUNT_NEXT_ROW(c2, 4, 2)                                            \
  COUNT_NEXT_ROW(c3, 4, 2)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr - 3*srcYStride);  \
  LOAD_FIRST_ROW(4, 3)                                                \
  STORE_SAT_VALUE_##lut_format(2)                                     \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 4, 3)                                            \
  COUNT_NEXT_ROW(c2, 4, 3)                                            \
  COUNT_NEXT_ROW(c3, 4, 3)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  GET_FILTERS_KOEF()                                                  \
  GET_POINTER()                                                       \
  LOAD_FIRST_ROW(4, 0)                                                \
  STORE_SAT_VALUE_##lut_format(3)

/***************************************************************/

#define MAKE_LAST_PIXEL_BC_4CH(lut_format)                            \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 4, 0)                                            \
  COUNT_NEXT_ROW(c2, 4, 0)                                            \
  COUNT_NEXT_ROW(c3, 4, 0)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr - 3*srcYStride);  \
  LOAD_FIRST_ROW(4, 1)                                                \
  STORE_SAT_VALUE_##lut_format(0)                                     \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 4, 1)                                            \
  COUNT_NEXT_ROW(c2, 4, 1)                                            \
  COUNT_NEXT_ROW(c3, 4, 1)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr - 3*srcYStride);  \
  LOAD_FIRST_ROW(4, 2)                                                \
  STORE_SAT_VALUE_##lut_format(1)                                     \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 4, 2)                                            \
  COUNT_NEXT_ROW(c2, 4, 2)                                            \
  COUNT_NEXT_ROW(c3, 4, 2)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  srcIndexPtr = (MLIB_TYPE*)((mlib_addr)srcIndexPtr - 3*srcYStride);  \
  LOAD_FIRST_ROW(4, 3)                                                \
  STORE_SAT_VALUE_##lut_format(2)                                     \
  c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);                           \
  COUNT_NEXT_ROW(c1, 4, 3)                                            \
  COUNT_NEXT_ROW(c2, 4, 3)                                            \
  COUNT_NEXT_ROW(c3, 4, 3)                                            \
  val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                         \
  STORE_SAT_VALUE_##lut_format(3)

/***************************************************************/

#define FILTER_U8  ((filter == MLIB_BICUBIC ) ? mlib_filters_u8f_bc  : mlib_filters_u8f_bc2)
#define FILTER_S16 ((filter == MLIB_BICUBIC ) ? mlib_filters_s16f_bc : mlib_filters_s16f_bc2)

/***************************************************************/

#define mlib_U8  mlib_u8
#define mlib_S16 mlib_s16

#define FUNC_AFFINEINDEX_BC_0(ITYPE, LTYPE, NCHAN)                                                 \
  mlib_status mlib_c_ImageAffineIndex_##ITYPE##_##LTYPE##_##NCHAN##CH_BC(mlib_s32   *leftEdges,    \
                                                                         mlib_s32    *rightEdges,  \
                                                                         mlib_s32    *xStarts,     \
                                                                         mlib_s32    *yStarts,     \
                                                                         mlib_s32    *sides,       \
                                                                         mlib_u8     *dstData,     \
                                                                         mlib_u8     **lineAddr,   \
                                                                         mlib_s32    dstYStride,   \
                                                                         mlib_s32    srcYStride,   \
                                                                         void        *colormap,    \
                                                                         mlib_filter filter)       \
  {                                                                                                \
    DECLAREVAR()                                                                                   \
    mlib_##LTYPE dstRowData[NCHAN * MLIB_LIMIT], *dstRowPtr = dstRowData, *dstPixelPtr;            \
    mlib_d64  *lut = ((mlib_d64*)mlib_ImageGetLutDoubleData(colormap) -                            \
                      NCHAN * mlib_ImageGetLutOffset(colormap));                                   \
    mlib_d64  sat = (mlib_d64)0x7F800000;                                                          \
    const mlib_f32 *mlib_filters_table = FILTER_##LTYPE;                                           \
                                                                                                   \
    if (max_xsize > MLIB_LIMIT) {                                                                  \
      dstRowPtr = mlib_malloc(NCHAN * sizeof(mlib_##LTYPE) * max_xsize);                           \
      if (dstRowPtr == NULL) return MLIB_FAILURE;                                                  \
    }                                                                                              \
                                                                                                   \
    for (j = yStart; j <= yFinish; j++) {                                                          \
                                                                                                   \
      CLIP()                                                                                       \
      dstPixelPtr = dstRowPtr;                                                                     \
                                                                                                   \
      GET_FILTERS_KOEF()                                                                           \
      GET_POINTER()                                                                                \
      LOAD_FIRST_ROW(NCHAN, 0)

    /* pragma pipeloop(0) must be here */

#define FUNC_AFFINEINDEX_BC_1(ITYPE, LTYPE, NCHAN)                                                 \
                                                                                                   \
      for (i = 0; i < (xRight - xLeft); i++, dstPixelPtr += NCHAN) {                               \
        MAKE_BC_##NCHAN##CH(LTYPE)                                                                 \
      }                                                                                            \
                                                                                                   \
      MAKE_LAST_PIXEL_BC_##NCHAN##CH(LTYPE)                                                        \
                                                                                                   \
      mlib_ImageColorTrue2IndexLine_##LTYPE##_##ITYPE##_##NCHAN(dstRowPtr, dstIndexPtr,            \
                                                                xRight - xLeft + 1, colormap);     \
    }                                                                                              \
                                                                                                   \
    if (dstRowPtr != dstRowData) mlib_free(dstRowPtr);                                             \
                                                                                                   \
    return MLIB_SUCCESS;                                                                           \
  }

/***************************************************************/

#undef MLIB_TYPE
#define MLIB_TYPE mlib_u8

/***************************************************************/

#undef  FILTER_SHIFT
#define FILTER_SHIFT 4
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 8) - 1) << 4)

FUNC_AFFINEINDEX_BC_0(U8, U8, 3)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BC_1(U8, U8, 3)

/***************************************************************/

#undef  FILTER_SHIFT
#define FILTER_SHIFT 3
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 9) - 1) << 4)

FUNC_AFFINEINDEX_BC_0(U8, S16, 3)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BC_1(U8, S16, 3)

/***************************************************************/

#undef  FILTER_SHIFT
#define FILTER_SHIFT 4
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 8) - 1) << 4)

FUNC_AFFINEINDEX_BC_0(U8, U8, 4)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BC_1(U8, U8, 4)

/***************************************************************/

#undef  FILTER_SHIFT
#define FILTER_SHIFT 3
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 9) - 1) << 4)

FUNC_AFFINEINDEX_BC_0(U8, S16, 4)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BC_1(U8, S16, 4)

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s16

/***************************************************************/

#undef  FILTER_SHIFT
#define FILTER_SHIFT 4
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 8) - 1) << 4)

FUNC_AFFINEINDEX_BC_0(S16, U8, 3)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BC_1(S16, U8, 3)

/***************************************************************/

#undef  FILTER_SHIFT
#define FILTER_SHIFT 3
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 9) - 1) << 4)

FUNC_AFFINEINDEX_BC_0(S16, S16, 3)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BC_1(S16, S16, 3)

/***************************************************************/

#undef  FILTER_SHIFT
#define FILTER_SHIFT 4
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 8) - 1) << 4)

FUNC_AFFINEINDEX_BC_0(S16, U8, 4)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BC_1(S16, U8, 4)

/***************************************************************/

#undef  FILTER_SHIFT
#define FILTER_SHIFT 3
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 9) - 1) << 4)

FUNC_AFFINEINDEX_BC_0(S16, S16, 4)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BC_1(S16, S16, 4)

/***************************************************************/
