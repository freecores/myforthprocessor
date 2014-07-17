/*
 * @(#)mlib_c_ImageAffineIndex_BL.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageAffineIndex_BL.c	1.15	00/02/07 SMI"
#endif /* __SUNPRO_C */

#include <stdlib.h>
#include "mlib_image.h"
#include "mlib_ImageColormap.h"
#include "mlib_ImageFilters.h"

/***************************************************************/

#define MLIB_LIMIT 512
#define MLIB_SHIFT  16
#define MLIB_PREC  (1 << MLIB_SHIFT)
#define MLIB_MASK  (MLIB_PREC - 1)

#ifdef MLIB_OS64BIT
#define MLIB_POINTER_SHIFT(P) (P >> (MLIB_SHIFT-3)) &~ 7
#define MLIB_POINTER_GET(A, P) (*(MLIB_TYPE **)((mlib_u8 *)A + P))
#else
#define MLIB_POINTER_SHIFT(P) (P >> (MLIB_SHIFT-2)) &~ 3
#define MLIB_POINTER_GET(A, P) (*(MLIB_TYPE **)((mlib_addr)A + P))
#endif

/***************************************************************/

#define DECLAREVAR()                            \
  mlib_s32  xLeft, xRight, X, Y;                \
  mlib_s32  xSrc, ySrc;                         \
  mlib_s32  yStart = sides[0];                  \
  mlib_s32  yFinish = sides[1];                 \
  mlib_s32  dX = sides[2];                      \
  mlib_s32  dY = sides[3];                      \
  mlib_s32  max_xsize = sides[4];               \
  MLIB_TYPE *srcIndexPtr0, *srcIndexPtr1;       \
  MLIB_TYPE *dstIndexPtr;                       \
  mlib_s32  i, j;                               \
  mlib_d64  scale = 1.0 / 65536.0;

/***************************************************************/

#define DECLARE_INTERNAL_VAR_3CH()              \
  mlib_d64  fdx, fdy;                           \
  mlib_d64  a00_0, a01_0, a10_0, a11_0;         \
  mlib_d64  a00_1, a01_1, a10_1, a11_1;         \
  mlib_d64  a00_2, a01_2, a10_2, a11_2;         \
  mlib_d64  pix0_0, pix1_0, res0;               \
  mlib_d64  pix0_1, pix1_1, res1;               \
  mlib_d64  pix0_2, pix1_2, res2;

/***************************************************************/

#define DECLARE_INTERNAL_VAR_4CH()              \
  mlib_d64  fdx, fdy;                           \
  mlib_d64  a00_0, a01_0, a10_0, a11_0;         \
  mlib_d64  a00_1, a01_1, a10_1, a11_1;         \
  mlib_d64  a00_2, a01_2, a10_2, a11_2;         \
  mlib_d64  a00_3, a01_3, a10_3, a11_3;         \
  mlib_d64  pix0_0, pix1_0, res0;               \
  mlib_d64  pix0_1, pix1_1, res1;               \
  mlib_d64  pix0_2, pix1_2, res2;               \
  mlib_d64  pix0_3, pix1_3, res3;

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

#define GET_PIXELS_POINTERS()                                          \
  fdx = (X & MLIB_MASK) * scale;                                       \
  fdy = (Y & MLIB_MASK) * scale;                                       \
  ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;                              \
  xSrc = X >> MLIB_SHIFT;  X += dX;                                    \
  srcIndexPtr0 = MLIB_POINTER_GET(lineAddr, ySrc) + xSrc;              \
  srcIndexPtr1 = (MLIB_TYPE *)((mlib_u8 *)srcIndexPtr0 + srcYStride);

/***************************************************************/

#define GET_COLOR_POINTERS(ind)                 \
  pcolor00 = (lut + srcIndexPtr0[0]*ind);       \
  pcolor10 = (lut + srcIndexPtr1[0]*ind);       \
  pcolor01 = (lut + srcIndexPtr0[1]*ind);       \
  pcolor11 = (lut + srcIndexPtr1[1]*ind);

/***************************************************************/

#define COUNT_BL_U8(ind)                                                \
  pix0_##ind = a00_##ind + fdy * (a10_##ind - a00_##ind);               \
  pix1_##ind = a01_##ind + fdy * (a11_##ind - a01_##ind);               \
  res##ind = pix0_##ind + fdx * (pix1_##ind - pix0_##ind) + 0.5;

/***************************************************************/

#define COUNT_BL_U8_3CH()       \
  COUNT_BL_U8(0)                \
  COUNT_BL_U8(1)                \
  COUNT_BL_U8(2)

/***************************************************************/

#define COUNT_BL_U8_4CH()       \
  COUNT_BL_U8_3CH();            \
  COUNT_BL_U8(3)

/***************************************************************/

#define COUNT_BL_S16(ind)                                       \
  pix0_##ind = a00_##ind + fdy * (a10_##ind - a00_##ind);       \
  pix1_##ind = a01_##ind + fdy * (a11_##ind - a01_##ind);       \
  res##ind = pix0_##ind + fdx * (pix1_##ind - pix0_##ind);

/***************************************************************/

#define COUNT_BL_S16_3CH()      \
  COUNT_BL_S16(0)               \
  COUNT_BL_S16(1)               \
  COUNT_BL_S16(2)

/***************************************************************/

#define COUNT_BL_S16_4CH()      \
  COUNT_BL_S16_3CH();           \
  COUNT_BL_S16(3)

/***************************************************************/

#define LOAD(ind)               \
  a00_##ind = pcolor00[ind];    \
  a01_##ind = pcolor01[ind];    \
  a10_##ind = pcolor10[ind];    \
  a11_##ind = pcolor11[ind];

/***************************************************************/

#define LOAD_3CH()      \
  LOAD(0)               \
  LOAD(1)               \
  LOAD(2)

/***************************************************************/

#define LOAD_4CH()      \
  LOAD_3CH()            \
  LOAD(3)

/***************************************************************/

#define STORE_INTO_INTERM_BUF_3CH()     \
  dstPixelPtr[0] = res0;                \
  dstPixelPtr[1] = res1;                \
  dstPixelPtr[2] = res2;

/***************************************************************/

#define STORE_INTO_INTERM_BUF_4CH()     \
  dstPixelPtr[0] = res0;                \
  dstPixelPtr[1] = res1;                \
  dstPixelPtr[2] = res2;                \
  dstPixelPtr[3] = res3;

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_u8

/***************************************************************/

#define mlib_U8  mlib_u8
#define mlib_S16 mlib_s16

#define FUNC_AFFINEINDEX_BL_0(ITYPE, LTYPE, NCHAN)                                                \
  mlib_status mlib_c_ImageAffineIndex_##ITYPE##_##LTYPE##_##NCHAN##CH_BL(mlib_s32   *leftEdges,   \
                                                                         mlib_s32   *rightEdges,  \
                                                                         mlib_s32   *xStarts,     \
                                                                         mlib_s32   *yStarts,     \
                                                                         mlib_s32   *sides,       \
                                                                         mlib_u8    *dstData,     \
                                                                         mlib_u8    **lineAddr,   \
                                                                         mlib_s32   dstYStride,   \
                                                                         mlib_s32   srcYStride,   \
                                                                         void       *colormap)    \
  {                                                                                               \
    DECLAREVAR()                                                                                  \
    mlib_##LTYPE  *dstPixelPtr, dstRowData[NCHAN * MLIB_LIMIT], *dstRowPtr = dstRowData;          \
    mlib_d64 *pcolor00, *pcolor10, *pcolor01, *pcolor11;                                          \
    mlib_d64 *lut = ((mlib_d64*)mlib_ImageGetLutDoubleData(colormap) -                            \
                      NCHAN * mlib_ImageGetLutOffset(colormap));                                  \
                                                                                                  \
    if (max_xsize > MLIB_LIMIT) {                                                                 \
      dstRowPtr = mlib_malloc(NCHAN * sizeof(mlib_##LTYPE) * max_xsize);                          \
      if (dstRowPtr == NULL) return MLIB_FAILURE;                                                 \
    }                                                                                             \
                                                                                                  \
    for (j = yStart; j <= yFinish; j++) {                                                         \
      DECLARE_INTERNAL_VAR_##NCHAN##CH()                                                          \
                                                                                                  \
      CLIP()                                                                                      \
      dstPixelPtr = dstRowPtr;                                                                    \
                                                                                                  \
      GET_PIXELS_POINTERS()                                                                       \
      GET_COLOR_POINTERS(NCHAN)                                                                   \
      LOAD_##NCHAN##CH()

    /* pragma pipeloop(0) must be here */

#define FUNC_AFFINEINDEX_BL_1(ITYPE, LTYPE, NCHAN)                                                \
      for (i = 0; i < (xRight - xLeft); i++, dstPixelPtr += NCHAN) {                              \
        COUNT_BL_##LTYPE##_##NCHAN##CH()                                                          \
                                                                                                  \
        GET_PIXELS_POINTERS()                                                                     \
        GET_COLOR_POINTERS(NCHAN)                                                                 \
        LOAD_##NCHAN##CH()                                                                        \
                                                                                                  \
        STORE_INTO_INTERM_BUF_##NCHAN##CH()                                                       \
      }                                                                                           \
                                                                                                  \
      COUNT_BL_##LTYPE##_##NCHAN##CH()                                                            \
      STORE_INTO_INTERM_BUF_##NCHAN##CH()                                                         \
                                                                                                  \
      mlib_ImageColorTrue2IndexLine_##LTYPE##_##ITYPE##_##NCHAN(dstRowPtr, dstIndexPtr,           \
                                                                xRight - xLeft + 1, colormap);    \
    }                                                                                             \
                                                                                                  \
    if (dstRowPtr != dstRowData) mlib_free(dstRowPtr);                                            \
                                                                                                  \
    return MLIB_SUCCESS;                                                                          \
  }

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_u8

FUNC_AFFINEINDEX_BL_0(U8, U8, 3)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BL_1(U8, U8, 3)

FUNC_AFFINEINDEX_BL_0(U8, S16, 3)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BL_1(U8, S16, 3)

FUNC_AFFINEINDEX_BL_0(U8, U8, 4)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BL_1(U8, U8, 4)

FUNC_AFFINEINDEX_BL_0(U8, S16, 4)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BL_1(U8, S16, 4)

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s16

FUNC_AFFINEINDEX_BL_0(S16, U8, 3)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BL_1(S16, U8, 3)

FUNC_AFFINEINDEX_BL_0(S16, S16, 3)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BL_1(S16, S16, 3)

FUNC_AFFINEINDEX_BL_0(S16, U8, 4)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BL_1(S16, U8, 4)

FUNC_AFFINEINDEX_BL_0(S16, S16, 4)
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
FUNC_AFFINEINDEX_BL_1(S16, S16, 4)

/***************************************************************/
