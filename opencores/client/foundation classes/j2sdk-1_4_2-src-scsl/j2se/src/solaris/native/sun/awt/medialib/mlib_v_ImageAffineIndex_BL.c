/*
 * @(#)mlib_v_ImageAffineIndex_BL.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#pragma ident	"@(#)mlib_v_ImageAffineIndex_BL.c	1.10	00/02/07 SMI"

#include <stdlib.h>
#include <vis_proto.h>
#include "mlib_image.h"
#include "mlib_ImageColormap.h"
#include "mlib_v_ImageFilters.h"

/***************************************************************/

#define MLIB_LIMIT 512
#define MLIB_SHIFT 16
#define MLIB_PREC  (1 << MLIB_SHIFT)

#ifdef MLIB_OS64BIT
#define MLIB_POINTER_SHIFT(P) (P >> (MLIB_SHIFT-3)) &~ 7
#else
#define MLIB_POINTER_SHIFT(P) (P >> (MLIB_SHIFT-2)) &~ 3
#endif

#define FILTER_SHIFT 5
#define FILTER_MASK  (((1 << 8) - 1) << 3)
#define LGSTEPS 8
#define STEPS (1 << LGSTEPS)

/***************************************************************/

#define DECLAREVAR()                    \
  mlib_s32  xLeft, xRight, X, Y;        \
  mlib_s32  yStart = sides[0];          \
  mlib_s32  yFinish = sides[1];         \
  mlib_s32  dX = sides[2];              \
  mlib_s32  dY = sides[3];              \
  mlib_s32  max_xsize = sides[4];       \
  MLIB_TYPE *srcIndexPtr;               \
  MLIB_TYPE *dstIndexPtr;               \
  mlib_d64  *dstPixelPtr;               \
  mlib_s32  i, j;

/***************************************************************/

#define CLIP()                                  \
  dstData += dstYStride;                        \
  xLeft = leftEdges[j];                         \
  xRight = rightEdges[j];                       \
  X = xStarts[j];                               \
  Y = yStarts[j];                               \
  if (xLeft > xRight)                           \
    continue;                                   \
  dstIndexPtr = (MLIB_TYPE *)dstData + xLeft;   \
  dstPixelPtr = dstRowPtr;

/***************************************************************/

#define FPMIL16(src1, src2)                     \
  vis_fpadd16(vis_fmul8sux16((src1), (src2)),   \
              vis_fmul8ulx16((src1), (src2)))

/***************************************************************/

#define BL_U8_4CH(index)                                                   \
  filterpos = MLIB_POINTER_SHIFT(Y);                                       \
  srcIndexPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);          \
  srcIndexPtr += (X >> MLIB_SHIFT);                                        \
  filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;                           \
  xfilter = *((mlib_d64 *) ((mlib_u8 *) mlib_filters_u8_bl + filterpos));  \
  filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;                           \
  yfilter = *((mlib_d64 *) ((mlib_u8 *) mlib_filters_u8_bl + filterpos     \
              + STEPS*8));                                                 \
  filter = vis_fpadd16(vis_fmul8sux16(xfilter, yfilter),                   \
                       vis_fmul8ulx16(xfilter, yfilter));                  \
  hi_top = flut[srcIndexPtr[0]];                                           \
  lo_top = flut[srcIndexPtr[1]];                                           \
  hi_bot = flut[srcIndexPtr[srcYStride]];                                  \
  lo_bot = flut[srcIndexPtr[srcYStride+1]];                                \
  ulprod = vis_fmul8x16au(hi_top, vis_read_hi(filter));                    \
  urprod = vis_fmul8x16al(lo_top, vis_read_hi(filter));                    \
  llprod = vis_fmul8x16au(hi_bot, vis_read_lo(filter));                    \
  lrprod = vis_fmul8x16al(lo_bot, vis_read_lo(filter));                    \
  sm##index = vis_fpadd16(ulprod, urprod);                                 \
  sm##index = vis_fpadd16(sm##index, llprod);                              \
  sm##index = vis_fpadd16(sm##index, lrprod);                              \
  sm##index = vis_fpadd16(sm##index, k05);                                 \
  X += dX; Y += dY;

/***************************************************************/

#define PREPARE_FILTERS_4BL_S16()                                               \
  x0 = X; y0 = Y;                                                               \
                                                                                \
  hi_deltax = vis_to_double_dup(((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1));  \
  hi_deltay = vis_to_double_dup(((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1));  \
                                                                                \
  hi_delta1_x = vis_fpsub16(mask_7fff, hi_deltax);                              \
  hi_delta1_y = vis_fpsub16(mask_7fff, hi_deltay);                              \
                                                                                \
  x0 += dX; y0 += dY;                                                           \
                                                                                \
  lo_deltax = vis_to_double_dup(((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1));  \
  lo_deltay = vis_to_double_dup(((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1));  \
                                                                                \
  lo_delta1_x = vis_fpsub16(mask_7fff, lo_deltax);                              \
  lo_delta1_y = vis_fpsub16(mask_7fff, lo_deltay);

/***************************************************************/

#define NEXT_PIXELS_4BL_S16()                                           \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcIndexPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);       \
  srcIndexPtr += (X >> MLIB_SHIFT);                                     \
  X += dX; Y += dY;

/***************************************************************/

#define LOAD_4BL_S16()                          \
  ul0 = flut[srcIndexPtr[0]];                   \
  ur0 = flut[srcIndexPtr[1]];                   \
  ll0 = flut[srcIndexPtr[srcYStride]];          \
  lr0 = flut[srcIndexPtr[srcYStride+1]];

/***************************************************************/

#define RESULT_4BL_S16(pos)                                     \
  ul0 = FPMIL16(ul0, pos##_delta1_x);                           \
  ur0 = FPMIL16(ur0, pos##_deltax);                             \
  ul0 = vis_fpadd16(ul0, ur0);                                  \
  ul0 = vis_fpadd16(ul0, ul0);                                  \
  ul0 = FPMIL16(ul0, pos##_delta1_y);                           \
  ll0 = FPMIL16(ll0, pos##_delta1_x);                           \
  lr0 = FPMIL16(lr0, pos##_deltax);                             \
  ll0 = vis_fpadd16(ll0, lr0);                                  \
  ll0 = vis_fpadd16(ll0, ll0);                                  \
  ll0 = FPMIL16(ll0, pos##_deltay);                             \
  hi8 = vis_fpadd16(ll0, ul0);                                  \
  hi8 = vis_fpadd16(hi8, hi8);                                  \
  pos##_deltax = vis_fpadd16(pos##_deltax, dx64);               \
  pos##_deltay = vis_fpadd16(pos##_deltay, dy64);               \
  pos##_deltax = vis_fand(pos##_deltax, mask_7fff);             \
  pos##_deltay = vis_fand(pos##_deltay, mask_7fff);             \
  pos##_delta1_x = vis_fpsub16(mask_7fff, pos##_deltax);        \
  pos##_delta1_y = vis_fpsub16(mask_7fff, pos##_deltay);

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_u8

/***************************************************************/

mlib_status mlib_v_ImageAffineIndex_U8_U8_3CH_BL(mlib_s32   *leftEdges,
                                                 mlib_s32   *rightEdges,
                                                 mlib_s32   *xStarts,
                                                 mlib_s32   *yStarts,
                                                 mlib_s32   *sides,
                                                 mlib_u8    *dstData,
                                                 mlib_u8    **lineAddr,
                                                 mlib_s32   dstYStride,
                                                 mlib_s32   srcYStride,
                                                 void       *colormap)
{
  DECLAREVAR()
  mlib_f32  *flut   = (mlib_f32 *)mlib_ImageGetLutNormalTable(colormap) -
  mlib_ImageGetLutOffset(colormap);
  mlib_d64  dstRowData[MLIB_LIMIT/2];
  mlib_d64  *dstRowPtr = dstRowData;
  mlib_d64 k05 = vis_to_double_dup(0x00080008);

  if (max_xsize > MLIB_LIMIT) {
    dstRowPtr = mlib_malloc(sizeof(mlib_d64) * ((max_xsize + 1) >> 1));
    if (dstRowPtr == NULL) return MLIB_FAILURE;
  }

  vis_write_gsr(3 << 3);

  for (j = yStart; j <= yFinish; j++) {
    mlib_s32 cols, times;
    mlib_s32 filterpos;
    mlib_d64 filter, xfilter, yfilter;
    mlib_d64 sm0, sm1;
    mlib_f32 hi_top, lo_top, hi_bot, lo_bot;
    mlib_d64 ulprod, urprod, llprod, lrprod;

    CLIP()

    cols = xRight - xLeft + 1;
    times = cols >> 1;

    if (times > 0) {
      BL_U8_4CH(0)
      BL_U8_4CH(1)

#pragma pipeloop(0)
      for (i = 1; i < times; i++) {
        *dstPixelPtr++ = vis_fpack16_pair(sm0, sm1);
        BL_U8_4CH(0)
        BL_U8_4CH(1)
      }
      *dstPixelPtr++ = vis_fpack16_pair(sm0, sm1);
    }
    cols -= times << 1;

    if (cols == 1) {
      BL_U8_4CH(0)
      *dstPixelPtr++ = vis_fpack16_pair(sm0, sm0);
    }

    mlib_ImageColorTrue2IndexLine_U8_U8_3_in_4((mlib_u8 *)dstRowPtr,
                                               dstIndexPtr,
                                               xRight - xLeft + 1,
                                               colormap);
  }

  if (dstRowPtr != dstRowData) mlib_free(dstRowPtr);

  return MLIB_SUCCESS;
}

/***************************************************************/

mlib_status mlib_v_ImageAffineIndex_U8_S16_3CH_BL(mlib_s32   *leftEdges,
                                                  mlib_s32   *rightEdges,
                                                  mlib_s32   *xStarts,
                                                  mlib_s32   *yStarts,
                                                  mlib_s32   *sides,
                                                  mlib_u8    *dstData,
                                                  mlib_u8    **lineAddr,
                                                  mlib_s32   dstYStride,
                                                  mlib_s32   srcYStride,
                                                  void       *colormap)
{
  DECLAREVAR()
  mlib_d64 *flut   = (mlib_d64 *)mlib_ImageGetLutNormalTable(colormap) -
  mlib_ImageGetLutOffset(colormap);
  mlib_d64 dstRowData[MLIB_LIMIT];
  mlib_d64 *dstRowPtr = dstRowData;
  mlib_d64 dx64, dy64;
  mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);

  dx64 = vis_to_double_dup((((dX) & 0xFFFF) << 16) | ((dX) & 0xFFFF));
  dy64 = vis_to_double_dup((((dY) & 0xFFFF) << 16) | ((dY) & 0xFFFF));

  if (max_xsize > MLIB_LIMIT) {
    dstRowPtr = mlib_malloc(sizeof(mlib_d64) * max_xsize);
    if (dstRowPtr == NULL) return MLIB_FAILURE;
  }

  for (j = yStart; j <= yFinish; j++) {
    mlib_d64 ul0, ur0, ll0, lr0;
    mlib_s32 cols, filterpos;
    mlib_d64 hi_deltax, hi_deltay, hi_delta1_x, hi_delta1_y;
    mlib_d64 lo_deltax, lo_deltay, lo_delta1_x, lo_delta1_y;
    mlib_d64 hi8;
    mlib_s32 x0, y0;

    CLIP()

    cols = xRight - xLeft + 1;
    PREPARE_FILTERS_4BL_S16()
    NEXT_PIXELS_4BL_S16()

#pragma pipeloop(0)
    for (i = 0; i < cols - 2; i += 2) {
      LOAD_4BL_S16()
      NEXT_PIXELS_4BL_S16()
      RESULT_4BL_S16(hi)
      *dstPixelPtr++ = hi8;

      LOAD_4BL_S16()
      NEXT_PIXELS_4BL_S16()
      RESULT_4BL_S16(lo)
      *dstPixelPtr++ = hi8;
    }
    LOAD_4BL_S16()
    RESULT_4BL_S16(hi)
    *dstPixelPtr++ = hi8;
    i++;

    if (i < cols) {
      NEXT_PIXELS_4BL_S16()
      LOAD_4BL_S16()
      RESULT_4BL_S16(lo)
      *dstPixelPtr++ = hi8;
    }

    mlib_ImageColorTrue2IndexLine_S16_U8_3_in_4((mlib_s16 *)dstRowPtr,
                                                dstIndexPtr,
                                                xRight - xLeft + 1,
                                                colormap);
  }

  if (dstRowPtr != dstRowData) mlib_free(dstRowPtr);

  return MLIB_SUCCESS;
}

/***************************************************************/

mlib_status mlib_v_ImageAffineIndex_U8_U8_4CH_BL(mlib_s32   *leftEdges,
                                                 mlib_s32   *rightEdges,
                                                 mlib_s32   *xStarts,
                                                 mlib_s32   *yStarts,
                                                 mlib_s32   *sides,
                                                 mlib_u8    *dstData,
                                                 mlib_u8    **lineAddr,
                                                 mlib_s32   dstYStride,
                                                 mlib_s32   srcYStride,
                                                 void       *colormap)
{
  DECLAREVAR()
  mlib_f32  *flut   = (mlib_f32 *)mlib_ImageGetLutNormalTable(colormap) -
  mlib_ImageGetLutOffset(colormap);
  mlib_d64  dstRowData[MLIB_LIMIT/2];
  mlib_d64  *dstRowPtr = dstRowData;
  mlib_d64 k05 = vis_to_double_dup(0x00080008);

  if (max_xsize > MLIB_LIMIT) {
    dstRowPtr = mlib_malloc(sizeof(mlib_d64) * ((max_xsize + 1) >> 1));
    if (dstRowPtr == NULL) return MLIB_FAILURE;
  }

  vis_write_gsr(3 << 3);

  for (j = yStart; j <= yFinish; j++) {
    mlib_s32 cols, times;
    mlib_s32 filterpos;
    mlib_d64 filter, xfilter, yfilter;
    mlib_d64 sm0, sm1;
    mlib_f32 hi_top, lo_top, hi_bot, lo_bot;
    mlib_d64 ulprod, urprod, llprod, lrprod;

    CLIP()

    cols = xRight - xLeft + 1;
    times = cols >> 1;

    if (times > 0) {
      BL_U8_4CH(0)
      BL_U8_4CH(1)

#pragma pipeloop(0)
      for (i = 1; i < times; i++) {
        *dstPixelPtr++ = vis_fpack16_pair(sm0, sm1);
        BL_U8_4CH(0)
        BL_U8_4CH(1)
      }
      *dstPixelPtr++ = vis_fpack16_pair(sm0, sm1);
    }
    cols -= times << 1;

    if (cols == 1) {
      BL_U8_4CH(0)
      *dstPixelPtr++ = vis_fpack16_pair(sm0, sm0);
    }

    mlib_ImageColorTrue2IndexLine_U8_U8_4((mlib_u8 *)dstRowPtr,
                                          dstIndexPtr,
                                          xRight - xLeft + 1,
                                          colormap);
  }

  if (dstRowPtr != dstRowData) mlib_free(dstRowPtr);

  return MLIB_SUCCESS;
}

/***************************************************************/

mlib_status mlib_v_ImageAffineIndex_U8_S16_4CH_BL(mlib_s32   *leftEdges,
                                                  mlib_s32   *rightEdges,
                                                  mlib_s32   *xStarts,
                                                  mlib_s32   *yStarts,
                                                  mlib_s32   *sides,
                                                  mlib_u8    *dstData,
                                                  mlib_u8    **lineAddr,
                                                  mlib_s32   dstYStride,
                                                  mlib_s32   srcYStride,
                                                  void       *colormap)
{
  DECLAREVAR()
  mlib_d64 *flut   = (mlib_d64 *)mlib_ImageGetLutNormalTable(colormap) -
  mlib_ImageGetLutOffset(colormap);
  mlib_d64 dstRowData[MLIB_LIMIT];
  mlib_d64 *dstRowPtr = dstRowData;
  mlib_d64 dx64, dy64;
  mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);

  dx64 = vis_to_double_dup((((dX) & 0xFFFF) << 16) | ((dX) & 0xFFFF));
  dy64 = vis_to_double_dup((((dY) & 0xFFFF) << 16) | ((dY) & 0xFFFF));

  if (max_xsize > MLIB_LIMIT) {
    dstRowPtr = mlib_malloc(sizeof(mlib_d64) * max_xsize);
    if (dstRowPtr == NULL) return MLIB_FAILURE;
  }

  for (j = yStart; j <= yFinish; j++) {
    mlib_d64 ul0, ur0, ll0, lr0;
    mlib_s32 cols, filterpos;
    mlib_d64 hi_deltax, hi_deltay, hi_delta1_x, hi_delta1_y;
    mlib_d64 lo_deltax, lo_deltay, lo_delta1_x, lo_delta1_y;
    mlib_d64 hi8;
    mlib_s32 x0, y0;

    CLIP()

    cols = xRight - xLeft + 1;
    PREPARE_FILTERS_4BL_S16()
    NEXT_PIXELS_4BL_S16()

#pragma pipeloop(0)
    for (i = 0; i < cols - 2; i += 2) {
      LOAD_4BL_S16()
      NEXT_PIXELS_4BL_S16()
      RESULT_4BL_S16(hi)
      *dstPixelPtr++ = hi8;

      LOAD_4BL_S16()
      NEXT_PIXELS_4BL_S16()
      RESULT_4BL_S16(lo)
      *dstPixelPtr++ = hi8;
    }
    LOAD_4BL_S16()
    RESULT_4BL_S16(hi)
    *dstPixelPtr++ = hi8;
    i++;

    if (i < cols) {
      NEXT_PIXELS_4BL_S16()
      LOAD_4BL_S16()
      RESULT_4BL_S16(lo)
      *dstPixelPtr = hi8;
    }

    mlib_ImageColorTrue2IndexLine_S16_U8_4((mlib_s16 *)dstRowPtr,
                                           dstIndexPtr,
                                           xRight - xLeft + 1,
                                           colormap);
  }

  if (dstRowPtr != dstRowData) mlib_free(dstRowPtr);

  return MLIB_SUCCESS;
}

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s16

/***************************************************************/

mlib_status mlib_v_ImageAffineIndex_S16_U8_3CH_BL(mlib_s32  *leftEdges,
                                                  mlib_s32   *rightEdges,
                                                  mlib_s32   *xStarts,
                                                  mlib_s32   *yStarts,
                                                  mlib_s32   *sides,
                                                  mlib_u8    *dstData,
                                                  mlib_u8    **lineAddr,
                                                  mlib_s32   dstYStride,
                                                  mlib_s32   srcYStride,
                                                  void       *colormap)
{
  DECLAREVAR()
  mlib_f32  *flut   = (mlib_f32 *)mlib_ImageGetLutNormalTable(colormap) -
  mlib_ImageGetLutOffset(colormap);
  mlib_d64  dstRowData[MLIB_LIMIT/2];
  mlib_d64  *dstRowPtr = dstRowData;
  mlib_d64 k05 = vis_to_double_dup(0x00080008);

  if (max_xsize > MLIB_LIMIT) {
    dstRowPtr = mlib_malloc(sizeof(mlib_d64) * ((max_xsize + 1) >> 1));
    if (dstRowPtr == NULL) return MLIB_FAILURE;
  }

  vis_write_gsr(3 << 3);
  srcYStride >>= 1;

  for (j = yStart; j <= yFinish; j++) {
    mlib_s32 cols, times;
    mlib_s32 filterpos;
    mlib_d64 filter, xfilter, yfilter;
    mlib_d64 sm0, sm1;
    mlib_f32 hi_top, lo_top, hi_bot, lo_bot;
    mlib_d64 ulprod, urprod, llprod, lrprod;

    CLIP()

    cols = xRight - xLeft + 1;
    times = cols >> 1;

    if (times > 0) {
      BL_U8_4CH(0)
      BL_U8_4CH(1)

#pragma pipeloop(0)
      for (i = 1; i < times; i++) {
        *dstPixelPtr++ = vis_fpack16_pair(sm0, sm1);
        BL_U8_4CH(0)
        BL_U8_4CH(1)
      }
      *dstPixelPtr++ = vis_fpack16_pair(sm0, sm1);
    }
    cols -= times << 1;

    if (cols == 1) {
      BL_U8_4CH(0)
      *dstPixelPtr++ = vis_fpack16_pair(sm0, sm0);
    }

    mlib_ImageColorTrue2IndexLine_U8_S16_3_in_4((mlib_u8 *)dstRowPtr,
                                                dstIndexPtr,
                                                xRight - xLeft + 1,
                                                colormap);
  }

  if (dstRowPtr != dstRowData) mlib_free(dstRowPtr);

  return MLIB_SUCCESS;
}

/***************************************************************/

mlib_status mlib_v_ImageAffineIndex_S16_S16_3CH_BL(mlib_s32  *leftEdges,
                                                   mlib_s32   *rightEdges,
                                                   mlib_s32   *xStarts,
                                                   mlib_s32   *yStarts,
                                                   mlib_s32   *sides,
                                                   mlib_u8    *dstData,
                                                   mlib_u8    **lineAddr,
                                                   mlib_s32   dstYStride,
                                                   mlib_s32   srcYStride,
                                                   void       *colormap)
{
  DECLAREVAR()
  mlib_d64 *flut   = (mlib_d64 *)mlib_ImageGetLutNormalTable(colormap) -
  mlib_ImageGetLutOffset(colormap);
  mlib_d64 dstRowData[MLIB_LIMIT];
  mlib_d64 *dstRowPtr = dstRowData;
  mlib_d64 dx64, dy64;
  mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);

  dx64 = vis_to_double_dup((((dX) & 0xFFFF) << 16) | ((dX) & 0xFFFF));
  dy64 = vis_to_double_dup((((dY) & 0xFFFF) << 16) | ((dY) & 0xFFFF));

  if (max_xsize > MLIB_LIMIT) {
    dstRowPtr = mlib_malloc(sizeof(mlib_d64) * max_xsize);
    if (dstRowPtr == NULL) return MLIB_FAILURE;
  }

  srcYStride >>= 1;

  for (j = yStart; j <= yFinish; j++) {
    mlib_d64 ul0, ur0, ll0, lr0;
    mlib_s32 cols, filterpos;
    mlib_d64 hi_deltax, hi_deltay, hi_delta1_x, hi_delta1_y;
    mlib_d64 lo_deltax, lo_deltay, lo_delta1_x, lo_delta1_y;
    mlib_d64 hi8;
    mlib_s32 x0, y0;

    CLIP()

    cols = xRight - xLeft + 1;
    PREPARE_FILTERS_4BL_S16()
    NEXT_PIXELS_4BL_S16()

#pragma pipeloop(0)
    for (i = 0; i < cols - 2; i += 2) {
      LOAD_4BL_S16()
      NEXT_PIXELS_4BL_S16()
      RESULT_4BL_S16(hi)
      *dstPixelPtr++ = hi8;

      LOAD_4BL_S16()
      NEXT_PIXELS_4BL_S16()
      RESULT_4BL_S16(lo)
      *dstPixelPtr++ = hi8;
    }
    LOAD_4BL_S16()
    RESULT_4BL_S16(hi)
    *dstPixelPtr++ = hi8;
    i++;

    if (i < cols) {
      NEXT_PIXELS_4BL_S16()
      LOAD_4BL_S16()
      RESULT_4BL_S16(lo)
      *dstPixelPtr++ = hi8;
    }

    mlib_ImageColorTrue2IndexLine_S16_S16_3_in_4((mlib_s16 *)dstRowPtr,
                                                 dstIndexPtr,
                                                 xRight - xLeft + 1,
                                                 colormap);
  }

  if (dstRowPtr != dstRowData) mlib_free(dstRowPtr);

  return MLIB_SUCCESS;
}

/***************************************************************/

mlib_status mlib_v_ImageAffineIndex_S16_U8_4CH_BL(mlib_s32  *leftEdges,
                                                  mlib_s32   *rightEdges,
                                                  mlib_s32   *xStarts,
                                                  mlib_s32   *yStarts,
                                                  mlib_s32   *sides,
                                                  mlib_u8    *dstData,
                                                  mlib_u8    **lineAddr,
                                                  mlib_s32   dstYStride,
                                                  mlib_s32   srcYStride,
                                                  void       *colormap)
{
  DECLAREVAR()
  mlib_f32  *flut   = (mlib_f32 *)mlib_ImageGetLutNormalTable(colormap) -
  mlib_ImageGetLutOffset(colormap);
  mlib_d64  dstRowData[MLIB_LIMIT/2];
  mlib_d64  *dstRowPtr = dstRowData;
  mlib_d64 k05 = vis_to_double_dup(0x00080008);

  if (max_xsize > MLIB_LIMIT) {
    dstRowPtr = mlib_malloc(sizeof(mlib_d64) * ((max_xsize + 1) >> 1));
    if (dstRowPtr == NULL) return MLIB_FAILURE;
  }

  srcYStride >>= 1;

  vis_write_gsr(3 << 3);

  for (j = yStart; j <= yFinish; j++) {
    mlib_s32 cols, times;
    mlib_s32 filterpos;
    mlib_d64 filter, xfilter, yfilter;
    mlib_d64 sm0, sm1;
    mlib_f32 hi_top, lo_top, hi_bot, lo_bot;
    mlib_d64 ulprod, urprod, llprod, lrprod;

    CLIP()

    cols = xRight - xLeft + 1;
    times = cols >> 1;

    if (times > 0) {
      BL_U8_4CH(0)
      BL_U8_4CH(1)

#pragma pipeloop(0)
      for (i = 1; i < times; i++) {
        *dstPixelPtr++ = vis_fpack16_pair(sm0, sm1);
        BL_U8_4CH(0)
        BL_U8_4CH(1)
      }
      *dstPixelPtr++ = vis_fpack16_pair(sm0, sm1);
    }
    cols -= times << 1;

    if (cols == 1) {
      BL_U8_4CH(0)
      *dstPixelPtr++ = vis_fpack16_pair(sm0, sm0);
    }

    mlib_ImageColorTrue2IndexLine_U8_S16_4((mlib_u8 *)dstRowPtr,
                                           dstIndexPtr,
                                           xRight - xLeft + 1,
                                           colormap);
  }

  if (dstRowPtr != dstRowData) mlib_free(dstRowPtr);

  return MLIB_SUCCESS;
}

/***************************************************************/

mlib_status mlib_v_ImageAffineIndex_S16_S16_4CH_BL(mlib_s32  *leftEdges,
                                                   mlib_s32   *rightEdges,
                                                   mlib_s32   *xStarts,
                                                   mlib_s32   *yStarts,
                                                   mlib_s32   *sides,
                                                   mlib_u8    *dstData,
                                                   mlib_u8    **lineAddr,
                                                   mlib_s32   dstYStride,
                                                   mlib_s32   srcYStride,
                                                   void       *colormap)
{
  DECLAREVAR()
  mlib_d64 *flut   = (mlib_d64 *)mlib_ImageGetLutNormalTable(colormap) -
  mlib_ImageGetLutOffset(colormap);
  mlib_d64 dstRowData[MLIB_LIMIT];
  mlib_d64 *dstRowPtr = dstRowData;
  mlib_d64 dx64, dy64;
  mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);

  dx64 = vis_to_double_dup((((dX) & 0xFFFF) << 16) | ((dX) & 0xFFFF));
  dy64 = vis_to_double_dup((((dY) & 0xFFFF) << 16) | ((dY) & 0xFFFF));

  if (max_xsize > MLIB_LIMIT) {
    dstRowPtr = mlib_malloc(sizeof(mlib_d64) * max_xsize);
    if (dstRowPtr == NULL) return MLIB_FAILURE;
  }

  srcYStride >>= 1;

  for (j = yStart; j <= yFinish; j++) {
    mlib_d64 ul0, ur0, ll0, lr0;
    mlib_s32 cols, filterpos;
    mlib_d64 hi_deltax, hi_deltay, hi_delta1_x, hi_delta1_y;
    mlib_d64 lo_deltax, lo_deltay, lo_delta1_x, lo_delta1_y;
    mlib_d64 hi8;
    mlib_s32 x0, y0;

    CLIP()

    cols = xRight - xLeft + 1;
    PREPARE_FILTERS_4BL_S16()
    NEXT_PIXELS_4BL_S16()

#pragma pipeloop(0)
    for (i = 0; i < cols - 2; i += 2) {
      LOAD_4BL_S16()
      NEXT_PIXELS_4BL_S16()
      RESULT_4BL_S16(hi)
      *dstPixelPtr++ = hi8;

      LOAD_4BL_S16()
      NEXT_PIXELS_4BL_S16()
      RESULT_4BL_S16(lo)
      *dstPixelPtr++ = hi8;
    }
    LOAD_4BL_S16()
    RESULT_4BL_S16(hi)
    *dstPixelPtr++ = hi8;
    i++;

    if (i < cols) {
      NEXT_PIXELS_4BL_S16()
      LOAD_4BL_S16()
      RESULT_4BL_S16(lo)
      *dstPixelPtr++ = hi8;
    }

    mlib_ImageColorTrue2IndexLine_S16_S16_4((mlib_s16 *)dstRowPtr,
                                            dstIndexPtr,
                                            xRight - xLeft + 1,
                                            colormap);
  }

  if (dstRowPtr != dstRowData) mlib_free(dstRowPtr);

  return MLIB_SUCCESS;
}

/***************************************************************/
