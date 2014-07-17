/*
 * @(#)mlib_v_ImageAffine_BC.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident  "@(#)mlib_v_ImageAffine_BC.c	1.9  99/12/20 SMI"

/*
 * FUNCTION
 *      mlib_v_ImageAffine_u8_1ch_bc
 *      mlib_v_ImageAffine_u8_2ch_bc
 *      mlib_v_ImageAffine_u8_3ch_bc
 *      mlib_v_ImageAffine_u8_4ch_bc
 *      mlib_v_ImageAffine_s16_1ch_bc
 *      mlib_v_ImageAffine_s16_2ch_bc
 *      mlib_v_ImageAffine_s16_3ch_bc
 *      mlib_v_ImageAffine_s16_4ch_bc
 *      mlib_v_ImageAffine_s32_1ch_bc
 *      mlib_v_ImageAffine_s32_2ch_bc
 *      mlib_v_ImageAffine_s32_3ch_bc
 *      mlib_v_ImageAffine_s32_4ch_bc
 *        - image affine transformation with Bicubic filtering
 * SYNOPSIS
 *      void mlib_v_ImageAffine_[u8|s16|s32]_?ch_bc
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

#include <vis_proto.h>
#include "mlib_image.h"
#include "mlib_v_ImageFilters.h"

/***************************************************************/

#define MLIB_SHIFT   16
#define MLIB_PREC        (1 << MLIB_SHIFT)
#define MLIB_MASK        (MLIB_PREC - 1)

/***************************************************************/

#define SAT32(DST) \
  DST = (mlib_s32)val0;

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

/***************************************************************/

#define MLIB_AFFINEDEFPARAM2_BC MLIB_AFFINEDEFPARAM, mlib_s32 srcYStride, mlib_filter filter

/***************************************************************/

#define DECLAREVAR               \
  mlib_s32  xLeft, xRight, X, Y; \
  mlib_s32  xSrc, ySrc;          \
  mlib_s32  yStart = sides[0];   \
  mlib_s32  yFinish = sides[1];  \
  mlib_s32  dX = sides[2];       \
  mlib_s32  dY = sides[3];       \
  MLIB_TYPE *sPtr;               \
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

#undef MLIB_TYPE
#define MLIB_TYPE mlib_u8
#define FILTER_SHIFT 5
#define FILTER_MASK  (((1 << 8) - 1) << 3)

/************************************************************************/

#define NEXT_PIXEL_1BC_U8()                                              \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  sPtr = (mlib_u8 *)lineAddr[ySrc] + xSrc;

/************************************************************************/

#define LOAD_BC_U8_1CH_1PIXEL( mlib_filters_u8 )                         \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row00 = vis_faligndata(data0, data1);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row20 = vis_faligndata(data0, data1);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  filterposy = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yFilter = *((mlib_d64 *) ((mlib_u8 *)mlib_filters_u8 + filterposy));   \
  filterposx = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xFilter = *((mlib_d64 *)((mlib_u8 *)mlib_filters_u8 + filterposx));    \
  X += dX;                                                               \
  Y += dY;

/************************************************************************/

#define RESULT_1BC_U8_1PIXEL(ind)                                        \
  v0 = vis_fmul8x16au(vis_read_hi(row0##ind), vis_read_hi(yFilter));     \
  v1 = vis_fmul8x16al(vis_read_hi(row1##ind), vis_read_hi(yFilter));     \
  sum = vis_fpadd16(v0, v1);                                             \
  v2 = vis_fmul8x16au(vis_read_hi(row2##ind), vis_read_lo(yFilter));     \
  sum = vis_fpadd16(sum, v2);                                            \
  v3 = vis_fmul8x16al(vis_read_hi(row3##ind), vis_read_lo(yFilter));     \
  sum = vis_fpadd16(sum, v3);                                            \
  v0 = vis_fmul8sux16(sum, xFilter);                                     \
  v1 = vis_fmul8ulx16(sum, xFilter);                                     \
  v3 = vis_fpadd16(v1, v0);                                              \
  vis_alignaddr((void*)2, 0);                                            \
  v0 = vis_faligndata(v3, v3);                                           \
  v2 = vis_fpadd16(v3, v0);                                              \
  v1 = vis_write_lo(v1, vis_fpadd16s(vis_read_hi(v2), vis_read_lo(v2))); \
  res = vis_write_lo(res, vis_fpack16(v1));

/************************************************************************/

#define BC_U8_1CH( index, ind1, ind2, mlib_filters_u8 )                  \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  v0 = vis_fmul8x16au(vis_read_hi(row0##ind1), vis_read_hi(yFilter));    \
  filterposy = (Y >> FILTER_SHIFT);                                      \
  data1 = dpSrc[1];                                                      \
  v1 = vis_fmul8x16al(vis_read_hi(row1##ind1), vis_read_hi(yFilter));    \
  row0##ind2 = vis_faligndata(data0, data1);                             \
  filterposx = (X >> FILTER_SHIFT);                                      \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  sum = vis_fpadd16(v0, v1);                                             \
  data0 = dpSrc[0];                                                      \
  v2 = vis_fmul8x16au(vis_read_hi(row2##ind1), vis_read_lo(yFilter));    \
  X += dX;                                                               \
  data1 = dpSrc[1];                                                      \
  row1##ind2 = vis_faligndata(data0, data1);                             \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  Y += dY;                                                               \
  sum = vis_fpadd16(sum, v2);                                            \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  v3 = vis_fmul8x16al(vis_read_hi(row3##ind1), vis_read_lo(yFilter));    \
  data0 = dpSrc[0];                                                      \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  sum = vis_fpadd16(sum, v3);                                            \
  data1 = dpSrc[1];                                                      \
  filterposy &= FILTER_MASK;                                             \
  v0 = vis_fmul8sux16(sum, xFilter);                                     \
  row2##ind2 = vis_faligndata(data0, data1);                             \
  sPtr += srcYStride;                                                    \
  v1 = vis_fmul8ulx16(sum, xFilter);                                     \
  filterposx &= FILTER_MASK;                                             \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  d##index = vis_fpadd16(v0, v1);                                        \
  data1 = dpSrc[1];                                                      \
  row3##ind2 = vis_faligndata(data0, data1);                             \
  yFilter = *((mlib_d64 *) ((mlib_u8 *)mlib_filters_u8 + filterposy));   \
  xFilter = *((mlib_d64 *)((mlib_u8 *)mlib_filters_u8 + filterposx));    \
  sPtr = (mlib_u8 *)lineAddr[ySrc] + xSrc;

/************************************************************************/

#define FADD_1BC_U8()                                                    \
  p0 = vis_fpadd16s(vis_read_hi(d0), vis_read_lo(d0));                   \
  p1 = vis_fpadd16s(vis_read_hi(d1), vis_read_lo(d1));                   \
  p2 = vis_fpadd16s(vis_read_hi(d2), vis_read_lo(d2));                   \
  p3 = vis_fpadd16s(vis_read_hi(d3), vis_read_lo(d3));                   \
  m02 = vis_fpmerge(p0, p2);                                             \
  m13 = vis_fpmerge(p1, p3);                                             \
  m0213 = vis_fpmerge(vis_read_hi(m02), vis_read_hi(m13));               \
  e0 = vis_fpmerge(vis_read_hi(m0213), vis_read_lo(m0213));              \
  m0213 = vis_fpmerge(vis_read_lo(m02), vis_read_lo(m13));               \
  e1 = vis_fpmerge(vis_read_hi(m0213), vis_read_lo(m0213));              \
  res = vis_fpadd16(e0, e1);

/************************************************************************/

void  mlib_v_ImageAffine_u8_1ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_s32  filterposx, filterposy;
  mlib_d64  data0, data1;
  mlib_d64  sum;
  mlib_d64  row00, row10, row20, row30;
  mlib_d64  row01, row11, row21, row31;
  mlib_f32  p0, p1, p2, p3;
  mlib_d64  xFilter, yFilter;
  mlib_d64  v0, v1, v2, v3;
  mlib_d64  d0, d1, d2, d3;
  mlib_d64  e0, e1;
  mlib_d64  m02, m13, m0213;
  mlib_d64  *dpSrc;
  mlib_s32  align, cols, i;
  mlib_d64  res;
  const union table *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = mlib_filters_u8_bc;
  } else {
    mlib_filters_table = mlib_filters_u8_bc2;
  }

  for (j = yStart; j <= yFinish; j++) {

    vis_write_gsr(3 << 3);

    CLIP(1)

    cols = xRight - xLeft + 1;
    align = (4 - ((mlib_addr)dstPixelPtr) & 3) & 3;
    align = (cols < align)? cols : align;

    for (i = 0; i < align; i++) {
      NEXT_PIXEL_1BC_U8()
      LOAD_BC_U8_1CH_1PIXEL( mlib_filters_table )
      RESULT_1BC_U8_1PIXEL(0)
      vis_st_u8(res, dstPixelPtr++);
    }

    if (i <= cols - 10) {

      NEXT_PIXEL_1BC_U8()
      LOAD_BC_U8_1CH_1PIXEL( mlib_filters_table )

      NEXT_PIXEL_1BC_U8()

      BC_U8_1CH(0, 0, 1, mlib_filters_table)
      BC_U8_1CH(1, 1, 0, mlib_filters_table)
      BC_U8_1CH(2, 0, 1, mlib_filters_table)
      BC_U8_1CH(3, 1, 0, mlib_filters_table)

      FADD_1BC_U8()

      BC_U8_1CH(0, 0, 1, mlib_filters_table)
      BC_U8_1CH(1, 1, 0, mlib_filters_table)
      BC_U8_1CH(2, 0, 1, mlib_filters_table)
      BC_U8_1CH(3, 1, 0, mlib_filters_table)

#pragma pipeloop(0)
      for (; i <= cols - 14; i+=4) {
        *(mlib_f32*)dstPixelPtr = vis_fpack16(res);
        FADD_1BC_U8()
        BC_U8_1CH(0, 0, 1, mlib_filters_table)
        BC_U8_1CH(1, 1, 0, mlib_filters_table)
        BC_U8_1CH(2, 0, 1, mlib_filters_table)
        BC_U8_1CH(3, 1, 0, mlib_filters_table)
        dstPixelPtr += 4;
      }
      *(mlib_f32*)dstPixelPtr = vis_fpack16(res);
      dstPixelPtr += 4;
      FADD_1BC_U8()
      *(mlib_f32*)dstPixelPtr = vis_fpack16(res);
      dstPixelPtr += 4;

      RESULT_1BC_U8_1PIXEL(0)
      vis_st_u8(res, dstPixelPtr++);

      LOAD_BC_U8_1CH_1PIXEL( mlib_filters_table )
      RESULT_1BC_U8_1PIXEL(0)
      vis_st_u8(res, dstPixelPtr++);
      i += 10;
    }

    for (; i < cols; i++) {
      NEXT_PIXEL_1BC_U8()
      LOAD_BC_U8_1CH_1PIXEL( mlib_filters_table )
      RESULT_1BC_U8_1PIXEL(0)
      vis_st_u8(res, dstPixelPtr++);
    }
  }
}

/************************************************************************/

#define FADD_2BC_U8()                                                    \
  d0 = vis_fpadd16(d00, d10);                                            \
  d1 = vis_fpadd16(d01, d11);                                            \
  d2 = vis_fpadd16(d02, d12);                                            \
  d3 = vis_fpadd16(d03, d13);                                            \
  p0 = vis_fpadd16s(vis_read_hi(d0), vis_read_lo(d0));                   \
  p1 = vis_fpadd16s(vis_read_hi(d1), vis_read_lo(d1));                   \
  p2 = vis_fpadd16s(vis_read_hi(d2), vis_read_lo(d2));                   \
  p3 = vis_fpadd16s(vis_read_hi(d3), vis_read_lo(d3));                   \
  e0 = vis_freg_pair(p0, p1);                                            \
  e1 = vis_freg_pair(p2, p3);                                            \
  res = vis_fpack16_pair(e0, e1);

/************************************************************************/

#define LOAD_BC_U8_2CH_1PIXEL( mlib_filters_u8 )                         \
  filterposy = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yFilter = *((mlib_d64 *) ((mlib_u8 *)mlib_filters_u8 + filterposy));   \
  filterposx = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xFilter = *((mlib_d64 *)((mlib_u8 *)mlib_filters_u8 + filterposx));    \
  X += dX;                                                               \
  Y += dY;                                                               \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row0 = vis_faligndata(data0, data1);                                   \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row1 = vis_faligndata(data0, data1);                                   \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row2 = vis_faligndata(data0, data1);                                   \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row3 = vis_faligndata(data0, data1);

/************************************************************************/

#define NEXT_PIXEL_2BC_U8()                                              \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  sPtr = (mlib_u8 *)lineAddr[ySrc] + (xSrc<<1);

/************************************************************************/

#define RESULT_2BC_U8_1PIXEL()                                           \
  v00 = vis_fmul8x16au(vis_read_hi(row0), vis_read_hi(yFilter));         \
  dr = vis_fpmerge(vis_read_hi(xFilter), vis_read_lo(xFilter));          \
  v01 = vis_fmul8x16au(vis_read_lo(row0), vis_read_hi(yFilter));         \
  dr = vis_fpmerge(vis_read_hi(dr), vis_read_lo(dr));                    \
  v10 = vis_fmul8x16al(vis_read_hi(row1), vis_read_hi(yFilter));         \
  dr1 = vis_fpmerge(vis_read_lo(dr), vis_read_lo(dr));                   \
  v11 = vis_fmul8x16al(vis_read_lo(row1), vis_read_hi(yFilter));         \
  dr = vis_fpmerge(vis_read_hi(dr), vis_read_hi(dr));                    \
  v20 = vis_fmul8x16au(vis_read_hi(row2), vis_read_lo(yFilter));         \
  xFilter0 = vis_fpmerge(vis_read_hi(dr), vis_read_hi(dr1));             \
  v21 = vis_fmul8x16au(vis_read_lo(row2), vis_read_lo(yFilter));         \
  xFilter1 = vis_fpmerge(vis_read_lo(dr), vis_read_lo(dr1));             \
  v30 = vis_fmul8x16al(vis_read_hi(row3), vis_read_lo(yFilter));         \
  sum0 = vis_fpadd16(v00, v10);                                          \
  v31 = vis_fmul8x16al(vis_read_lo(row3), vis_read_lo(yFilter));         \
  sum1 = vis_fpadd16(v01, v11);                                          \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  v00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  v01 = vis_fmul8sux16(sum1, xFilter1);                                  \
  v10 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  sum0 = vis_fpadd16(v00, v10);                                          \
  v11 = vis_fmul8ulx16(sum1, xFilter1);                                  \
  sum1 = vis_fpadd16(v01, v11);                                          \
  d0 = vis_fpadd16(sum0, sum1);                                          \
  v00 = vis_write_lo(v00, vis_fpadd16s(vis_read_hi(d0),                  \
                                       vis_read_lo(d0)));                \
  res = vis_write_lo(res, vis_fpack16(v00));

/************************************************************************/

#define BC_U8_2CH( index, mlib_filters_u8 )                              \
  v00 = vis_fmul8x16au(vis_read_hi(row0), vis_read_hi(yFilter));         \
  dr = vis_fpmerge(vis_read_hi(xFilter), vis_read_lo(xFilter));          \
  v01 = vis_fmul8x16au(vis_read_lo(row0), vis_read_hi(yFilter));         \
  dr = vis_fpmerge(vis_read_hi(dr), vis_read_lo(dr));                    \
  v10 = vis_fmul8x16al(vis_read_hi(row1), vis_read_hi(yFilter));         \
  dr1 = vis_fpmerge(vis_read_lo(dr), vis_read_lo(dr));                   \
  v11 = vis_fmul8x16al(vis_read_lo(row1), vis_read_hi(yFilter));         \
  dr = vis_fpmerge(vis_read_hi(dr), vis_read_hi(dr));                    \
  v20 = vis_fmul8x16au(vis_read_hi(row2), vis_read_lo(yFilter));         \
  xFilter0 = vis_fpmerge(vis_read_hi(dr), vis_read_hi(dr1));             \
  v21 = vis_fmul8x16au(vis_read_lo(row2), vis_read_lo(yFilter));         \
  xFilter1 = vis_fpmerge(vis_read_lo(dr), vis_read_lo(dr1));             \
  v30 = vis_fmul8x16al(vis_read_hi(row3), vis_read_lo(yFilter));         \
  v31 = vis_fmul8x16al(vis_read_lo(row3), vis_read_lo(yFilter));         \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  sum0 = vis_fpadd16(v00, v10);                                          \
  filterposy = (Y >> FILTER_SHIFT);                                      \
  data1 = dpSrc[1];                                                      \
  row0 = vis_faligndata(data0, data1);                                   \
  filterposx = (X >> FILTER_SHIFT);                                      \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  sum1 = vis_fpadd16(v01, v11);                                          \
  X += dX;                                                               \
  data1 = dpSrc[1];                                                      \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  row1 = vis_faligndata(data0, data1);                                   \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  Y += dY;                                                               \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  data0 = dpSrc[0];                                                      \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  data1 = dpSrc[1];                                                      \
  filterposy &= FILTER_MASK;                                             \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  v00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  row2 = vis_faligndata(data0, data1);                                   \
  v01 = vis_fmul8sux16(sum1, xFilter1);                                  \
  sPtr += srcYStride;                                                    \
  v10 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  filterposx &= FILTER_MASK;                                             \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  v11= vis_fmul8ulx16(sum1, xFilter1);                                   \
  data0 = dpSrc[0];                                                      \
  d0##index = vis_fpadd16(v00, v10);                                     \
  data1 = dpSrc[1];                                                      \
  row3 = vis_faligndata(data0, data1);                                   \
  yFilter = *((mlib_d64 *) ((mlib_u8 *)mlib_filters_u8 + filterposy));   \
  d1##index = vis_fpadd16(v01, v11);                                     \
  xFilter = *((mlib_d64 *)((mlib_u8 *)mlib_filters_u8 + filterposx));    \
  sPtr = (mlib_u8 *)lineAddr[ySrc] + (xSrc<<1);                          \

/************************************************************************/

void  mlib_v_ImageAffine_u8_2ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_s32  filterposx, filterposy;
  mlib_d64  data0, data1;
  mlib_d64  sum0, sum1;
  mlib_d64  row0, row1, row2, row3;
  mlib_f32  p0, p1, p2, p3;
  mlib_d64  xFilter;
  mlib_d64  xFilter0, xFilter1, yFilter;
  mlib_d64  v00, v10, v20, v30;
  mlib_d64  v01, v11, v21, v31;
  mlib_d64  d0, d1, d2, d3;
  mlib_d64  d00, d01, d02, d03;
  mlib_d64  d10, d11, d12, d13;
  mlib_d64  e0, e1;
  mlib_d64  *dpSrc;
  mlib_s32  cols, i, mask, off;
  mlib_d64  dr, dr1;
  mlib_d64  res, *dp;
  const union table *mlib_filters_table;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table = mlib_filters_u8_bc;
  } else {
    mlib_filters_table = mlib_filters_u8_bc2;
  }

  for (j = yStart; j <= yFinish; j++) {

    vis_write_gsr(3 << 3);

    CLIP(2)

    cols = xRight - xLeft + 1;
    dp = vis_alignaddr(dstPixelPtr, 0);
    off = dstPixelPtr - (mlib_u8*)dp;
    dstLineEnd += 1;
    mask = vis_edge8(dstPixelPtr, dstLineEnd);
    i = 0;

    if (i <= cols - 10) {

      NEXT_PIXEL_2BC_U8()
      LOAD_BC_U8_2CH_1PIXEL( mlib_filters_table )

      NEXT_PIXEL_2BC_U8()

      BC_U8_2CH( 0, mlib_filters_table )
      BC_U8_2CH( 1, mlib_filters_table )
      BC_U8_2CH( 2, mlib_filters_table )
      BC_U8_2CH( 3, mlib_filters_table )

      FADD_2BC_U8()

      BC_U8_2CH( 0, mlib_filters_table )
      BC_U8_2CH( 1, mlib_filters_table )
      BC_U8_2CH( 2, mlib_filters_table )
      BC_U8_2CH( 3, mlib_filters_table )

#pragma pipeloop(0)

      for (; i <= cols-14; i+=4) {
        vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
        res = vis_faligndata(res, res);
        vis_pst_8(res, dp++, mask);
        vis_pst_8(res, dp, ~mask);
        FADD_2BC_U8()
        BC_U8_2CH( 0, mlib_filters_table )
        BC_U8_2CH( 1, mlib_filters_table )
        BC_U8_2CH( 2, mlib_filters_table )
        BC_U8_2CH( 3, mlib_filters_table )
      }
      vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
      res = vis_faligndata(res, res);
      vis_pst_8(res, dp++, mask);
      vis_pst_8(res, dp, ~mask);

      FADD_2BC_U8()
      vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
      res = vis_faligndata(res, res);
      vis_pst_8(res, dp++, mask);
      vis_pst_8(res, dp, ~mask);

      dstPixelPtr = (mlib_u8*)dp + off;

      RESULT_2BC_U8_1PIXEL()
      vis_alignaddr((void *)7, 0);
      vis_st_u8(res, dstPixelPtr+1);
      res = vis_faligndata(res, res);
      vis_st_u8(res, dstPixelPtr);
      dstPixelPtr += 2;

      LOAD_BC_U8_2CH_1PIXEL( mlib_filters_table )
      RESULT_2BC_U8_1PIXEL()
      vis_alignaddr((void *)7, 0);
      vis_st_u8(res, dstPixelPtr+1);
      res = vis_faligndata(res, res);
      vis_st_u8(res, dstPixelPtr);
      dstPixelPtr += 2;
      i += 10;
    }

    for (; i < cols; i++) {
      NEXT_PIXEL_2BC_U8()
      LOAD_BC_U8_2CH_1PIXEL( mlib_filters_table )
      RESULT_2BC_U8_1PIXEL()
      vis_alignaddr((void *)7, 0);
      vis_st_u8(res, dstPixelPtr+1);
      res = vis_faligndata(res, res);
      vis_st_u8(res, dstPixelPtr);
      dstPixelPtr += 2;
    }
  }
}

/************************************************************************/

#define FADD_3BC_U8()                                                    \
  vis_alignaddr((void*)6, 0);                                            \
  d3 = vis_faligndata(d0, d1);                                           \
  vis_alignaddr((void*)2, 0);                                            \
  d4 = vis_faligndata(d1, d2);                                           \
  d0 = vis_fpadd16(d0, d3);                                              \
  d2 = vis_fpadd16(d2, d4);                                              \
  d1 = vis_faligndata(d2, d2);                                           \
  d0 = vis_fpadd16(d0, d1);                                              \
  f0.f = vis_fpack16(d0);

/************************************************************************/

#define LOAD_BC_U8_3CH_1PIXEL( mlib_filters_u8, mlib_filters_u8_3 )      \
  filterposy = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yFilter = *((mlib_d64 *) ((mlib_u8 *)mlib_filters_u8 + filterposy));   \
  filterposx = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xPtr=((mlib_d64 *)((mlib_u8 *)mlib_filters_u8_3+3*filterposx));        \
  xFilter0 = xPtr[0];                                                    \
  xFilter1 = xPtr[1];                                                    \
  xFilter2 = xPtr[2];                                                    \
  X += dX;                                                               \
  Y += dY;                                                               \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row00 = vis_faligndata(data0, data1);                                  \
  row01 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  row11 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row20 = vis_faligndata(data0, data1);                                  \
  row21 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  row31 = vis_faligndata(data1, data2);

/************************************************************************/

#define STORE_BC_U8_3CH_1PIXEL()                                         \
 dstPixelPtr[0] = f0.t[0];                                               \
 dstPixelPtr[1] = f0.t[1];                                               \
 dstPixelPtr[2] = f0.t[2];                                               \
 dstPixelPtr += 3;

/************************************************************************/

#define NEXT_PIXEL_3BC_U8()                                              \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  sPtr = (mlib_u8 *)lineAddr[ySrc] + (3*xSrc);

/************************************************************************/

#define RESULT_3BC_U8_1PIXEL()                                           \
  v00 = vis_fmul8x16au(vis_read_hi(row00), vis_read_hi(yFilter));        \
  v01 = vis_fmul8x16au(vis_read_lo(row00), vis_read_hi(yFilter));        \
  v02 = vis_fmul8x16au(vis_read_hi(row01), vis_read_hi(yFilter));        \
  v10 = vis_fmul8x16al(vis_read_hi(row10), vis_read_hi(yFilter));        \
  v11 = vis_fmul8x16al(vis_read_lo(row10), vis_read_hi(yFilter));        \
  v12 = vis_fmul8x16al(vis_read_hi(row11), vis_read_hi(yFilter));        \
  v20 = vis_fmul8x16au(vis_read_hi(row20), vis_read_lo(yFilter));        \
  sum0 = vis_fpadd16(v00, v10);                                          \
  v21 = vis_fmul8x16au(vis_read_lo(row20), vis_read_lo(yFilter));        \
  sum1 = vis_fpadd16(v01, v11);                                          \
  v22 = vis_fmul8x16au(vis_read_hi(row21), vis_read_lo(yFilter));        \
  sum2 = vis_fpadd16(v02, v12);                                          \
  v30 = vis_fmul8x16al(vis_read_hi(row30), vis_read_lo(yFilter));        \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  v31 = vis_fmul8x16al(vis_read_lo(row30), vis_read_lo(yFilter));        \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  v32 = vis_fmul8x16al(vis_read_hi(row31), vis_read_lo(yFilter));        \
  sum2 = vis_fpadd16(sum2, v22);                                         \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  v00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  sum2 = vis_fpadd16(sum2, v32);                                         \
  v01 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  v10 = vis_fmul8sux16(sum1, xFilter1);                                  \
  d0 = vis_fpadd16(v00, v01);                                            \
  v11 = vis_fmul8ulx16(sum1, xFilter1);                                  \
  v20 = vis_fmul8sux16(sum2, xFilter2);                                  \
  d1 = vis_fpadd16(v10, v11);                                            \
  v21 = vis_fmul8ulx16(sum2, xFilter2);                                  \
  d2 = vis_fpadd16(v20, v21);                                            \
  vis_alignaddr((void*)6, 0);                                            \
  d3 = vis_faligndata(d0, d1);                                           \
  vis_alignaddr((void*)2, 0);                                            \
  d4 = vis_faligndata(d1, d2);                                           \
  d0 = vis_fpadd16(d0, d3);                                              \
  d2 = vis_fpadd16(d2, d4);                                              \
  d1 = vis_faligndata(d2, d2);                                           \
  d0 = vis_fpadd16(d0, d1);                                              \
  f0.f = vis_fpack16(d0);

/************************************************************************/

#define BC_U8_3CH( mlib_filters_u8, mlib_filters_u8_3 )                  \
  v00 = vis_fmul8x16au(vis_read_hi(row00), vis_read_hi(yFilter));        \
  v01 = vis_fmul8x16au(vis_read_lo(row00), vis_read_hi(yFilter));        \
  v02 = vis_fmul8x16au(vis_read_hi(row01), vis_read_hi(yFilter));        \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  filterposy = (Y >> FILTER_SHIFT);                                      \
  v10 = vis_fmul8x16al(vis_read_hi(row10), vis_read_hi(yFilter));        \
  data1 = dpSrc[1];                                                      \
  v11 = vis_fmul8x16al(vis_read_lo(row10), vis_read_hi(yFilter));        \
  sum0 = vis_fpadd16(v00, v10);                                          \
  data2 = dpSrc[2];                                                      \
  row00 = vis_faligndata(data0, data1);                                  \
  v12 = vis_fmul8x16al(vis_read_hi(row11), vis_read_hi(yFilter));        \
  row01 = vis_faligndata(data1, data2);                                  \
  filterposx = (X >> FILTER_SHIFT);                                      \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  v20 = vis_fmul8x16au(vis_read_hi(row20), vis_read_lo(yFilter));        \
  sum1 = vis_fpadd16(v01, v11);                                          \
  data0 = dpSrc[0];                                                      \
  X += dX;                                                               \
  data1 = dpSrc[1];                                                      \
  v21 = vis_fmul8x16au(vis_read_lo(row20), vis_read_lo(yFilter));        \
  sum2 = vis_fpadd16(v02, v12);                                          \
  data2 = dpSrc[2];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  v22 = vis_fmul8x16au(vis_read_hi(row21), vis_read_lo(yFilter));        \
  row11 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  Y += dY;                                                               \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  v30 = vis_fmul8x16al(vis_read_hi(row30), vis_read_lo(yFilter));        \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  data0 = dpSrc[0];                                                      \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  data1 = dpSrc[1];                                                      \
  v31 = vis_fmul8x16al(vis_read_lo(row30), vis_read_lo(yFilter));        \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  data2 = dpSrc[2];                                                      \
  filterposy &= FILTER_MASK;                                             \
  row20 = vis_faligndata(data0, data1);                                  \
  v32 = vis_fmul8x16al(vis_read_hi(row31), vis_read_lo(yFilter));        \
  row21 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  filterposx &= FILTER_MASK;                                             \
  sum2 = vis_fpadd16(sum2, v22);                                         \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  data0 = dpSrc[0];                                                      \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  v00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  data1 = dpSrc[1];                                                      \
  sum2 = vis_fpadd16(sum2, v32);                                         \
  v01 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  data2 = dpSrc[2];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  v10 = vis_fmul8sux16(sum1, xFilter1);                                  \
  d0 = vis_fpadd16(v00, v01);                                            \
  row31 = vis_faligndata(data1, data2);                                  \
  yFilter = *((mlib_d64 *)((mlib_u8 *)mlib_filters_u8 + filterposy));    \
  v11 = vis_fmul8ulx16(sum1, xFilter1);                                  \
  xPtr=((mlib_d64 *)((mlib_u8 *)mlib_filters_u8_3+3*filterposx));        \
  xFilter0 = xPtr[0];                                                    \
  v20 = vis_fmul8sux16(sum2, xFilter2);                                  \
  d1 = vis_fpadd16(v10, v11);                                            \
  xFilter1 = xPtr[1];                                                    \
  v21 = vis_fmul8ulx16(sum2, xFilter2);                                  \
  xFilter2 = xPtr[2];                                                    \
  sPtr = (mlib_u8 *)lineAddr[ySrc] + (3*xSrc);                           \
  d2 = vis_fpadd16(v20, v21);                                            \

/************************************************************************/

void  mlib_v_ImageAffine_u8_3ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_s32  filterposx, filterposy;
  mlib_d64  data0, data1, data2;
  mlib_d64  sum0, sum1, sum2;
  mlib_d64  row00, row10, row20, row30;
  mlib_d64  row01, row11, row21, row31;
  mlib_d64  xFilter0, xFilter1, xFilter2, yFilter;
  mlib_d64  v00, v10, v20, v30;
  mlib_d64  v01, v11, v21, v31;
  mlib_d64  v02, v12, v22, v32;
  mlib_d64  d0, d1, d2, d3, d4;
  mlib_d64  *dpSrc;
  mlib_s32  cols, i;
  mlib_d64  *xPtr;
  union {
    mlib_u8 t[4];
    mlib_f32 f;
  } f0;
  const union table *mlib_filters_table  ;
  const union table *mlib_filters_table_3;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table   = mlib_filters_u8_bc;
    mlib_filters_table_3 = mlib_filters_u8_bc_3;
  } else {
    mlib_filters_table   = mlib_filters_u8_bc2;
    mlib_filters_table_3 = mlib_filters_u8_bc2_3;
  }

  for ( j = yStart; j <= yFinish; j ++ ) {

    vis_write_gsr(3 << 3);

    CLIP(3)

    cols = xRight - xLeft + 1;
    i = 0;

    if (i <= cols - 4) {

      NEXT_PIXEL_3BC_U8()
      LOAD_BC_U8_3CH_1PIXEL( mlib_filters_table, mlib_filters_table_3 )

      NEXT_PIXEL_3BC_U8()

      BC_U8_3CH( mlib_filters_table, mlib_filters_table_3 )
      FADD_3BC_U8()

      BC_U8_3CH( mlib_filters_table, mlib_filters_table_3 )

#pragma pipeloop(0)

      for (; i < cols-4; i++) {
        STORE_BC_U8_3CH_1PIXEL()

        FADD_3BC_U8()
        BC_U8_3CH( mlib_filters_table, mlib_filters_table_3 )
      }

      STORE_BC_U8_3CH_1PIXEL()

      FADD_3BC_U8()
      STORE_BC_U8_3CH_1PIXEL()

      RESULT_3BC_U8_1PIXEL()
      STORE_BC_U8_3CH_1PIXEL()

      LOAD_BC_U8_3CH_1PIXEL( mlib_filters_table, mlib_filters_table_3 )
      RESULT_3BC_U8_1PIXEL()
      STORE_BC_U8_3CH_1PIXEL()
      i += 4;
    }

    for (; i < cols; i++) {
      NEXT_PIXEL_3BC_U8()
      LOAD_BC_U8_3CH_1PIXEL( mlib_filters_table, mlib_filters_table_3 )
      RESULT_3BC_U8_1PIXEL()
      STORE_BC_U8_3CH_1PIXEL()
    }
  }
}

/************************************************************************/

#define FADD_4BC_U8()                                                    \
  d0 = vis_fpadd16(d00, d10);                                            \
  d1 = vis_fpadd16(d20, d30);                                            \
  d0 = vis_fpadd16(d0, d1);                                              \
  d2 = vis_fpadd16(d01, d11);                                            \
  d3 = vis_fpadd16(d21, d31);                                            \
  d2 = vis_fpadd16(d2, d3);                                              \
  res = vis_fpack16_pair(d0, d2);

/************************************************************************/

#define LOAD_BC_U8_4CH_1PIXEL( mlib_filters_u8, mlib_filters_u8_4 )      \
  filterposy = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yFilter = *((mlib_d64 *) ((mlib_u8 *)mlib_filters_u8 + filterposy));   \
  filterposx = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xPtr=((mlib_d64 *)((mlib_u8 *)mlib_filters_u8_4+4*filterposx));        \
  xFilter0 = xPtr[0];                                                    \
  xFilter1 = xPtr[1];                                                    \
  xFilter2 = xPtr[2];                                                    \
  xFilter3 = xPtr[3];                                                    \
  X += dX;                                                               \
  Y += dY;                                                               \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row00 = vis_faligndata(data0, data1);                                  \
  row01 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  row11 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row20 = vis_faligndata(data0, data1);                                  \
  row21 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  row31 = vis_faligndata(data1, data2);

/************************************************************************/

#define NEXT_PIXEL_4BC_U8()                                              \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  sPtr = (mlib_u8 *)lineAddr[ySrc] + (4*xSrc);

/************************************************************************/

#define RESULT_4BC_U8_1PIXEL(ind)                                        \
  v00 = vis_fmul8x16au(vis_read_hi(row00), vis_read_hi(yFilter));        \
  v01 = vis_fmul8x16au(vis_read_lo(row00), vis_read_hi(yFilter));        \
  v02 = vis_fmul8x16au(vis_read_hi(row01), vis_read_hi(yFilter));        \
  v03 = vis_fmul8x16au(vis_read_lo(row01), vis_read_hi(yFilter));        \
  v10 = vis_fmul8x16al(vis_read_hi(row10), vis_read_hi(yFilter));        \
  v11 = vis_fmul8x16al(vis_read_lo(row10), vis_read_hi(yFilter));        \
  sum0 = vis_fpadd16(v00, v10);                                          \
  v12 = vis_fmul8x16al(vis_read_hi(row11), vis_read_hi(yFilter));        \
  sum1 = vis_fpadd16(v01, v11);                                          \
  v13 = vis_fmul8x16al(vis_read_lo(row11), vis_read_hi(yFilter));        \
  sum2 = vis_fpadd16(v02, v12);                                          \
  v20 = vis_fmul8x16au(vis_read_hi(row20), vis_read_lo(yFilter));        \
  sum3 = vis_fpadd16(v03, v13);                                          \
  v21 = vis_fmul8x16au(vis_read_lo(row20), vis_read_lo(yFilter));        \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  v22 = vis_fmul8x16au(vis_read_hi(row21), vis_read_lo(yFilter));        \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  v23 = vis_fmul8x16au(vis_read_lo(row21), vis_read_lo(yFilter));        \
  sum2 = vis_fpadd16(sum2, v22);                                         \
  v30 = vis_fmul8x16al(vis_read_hi(row30), vis_read_lo(yFilter));        \
  sum3 = vis_fpadd16(sum3, v23);                                         \
  v31 = vis_fmul8x16al(vis_read_lo(row30), vis_read_lo(yFilter));        \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  v32 = vis_fmul8x16al(vis_read_hi(row31), vis_read_lo(yFilter));        \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  v33 = vis_fmul8x16al(vis_read_lo(row31), vis_read_lo(yFilter));        \
  sum2 = vis_fpadd16(sum2, v32);                                         \
  v00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  sum3 = vis_fpadd16(sum3, v33);                                         \
  v01 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  v10 = vis_fmul8sux16(sum1, xFilter1);                                  \
  d0##ind = vis_fpadd16(v00, v01);                                       \
  v11 = vis_fmul8ulx16(sum1, xFilter1);                                  \
  v20 = vis_fmul8sux16(sum2, xFilter2);                                  \
  d1##ind = vis_fpadd16(v10, v11);                                       \
  v21 = vis_fmul8ulx16(sum2, xFilter2);                                  \
  v30 = vis_fmul8sux16(sum3, xFilter3);                                  \
  d2##ind = vis_fpadd16(v20, v21);                                       \
  v31 = vis_fmul8ulx16(sum3, xFilter3);                                  \
  d3##ind = vis_fpadd16(v30, v31);

/************************************************************************/

#define BC_U8_4CH( ind, mlib_filters_u8, mlib_filters_u8_4 )             \
  v00 = vis_fmul8x16au(vis_read_hi(row00), vis_read_hi(yFilter));        \
  v01 = vis_fmul8x16au(vis_read_lo(row00), vis_read_hi(yFilter));        \
  v02 = vis_fmul8x16au(vis_read_hi(row01), vis_read_hi(yFilter));        \
  v03 = vis_fmul8x16au(vis_read_lo(row01), vis_read_hi(yFilter));        \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  filterposy = (Y >> FILTER_SHIFT);                                      \
  v10 = vis_fmul8x16al(vis_read_hi(row10), vis_read_hi(yFilter));        \
  data1 = dpSrc[1];                                                      \
  v11 = vis_fmul8x16al(vis_read_lo(row10), vis_read_hi(yFilter));        \
  sum0 = vis_fpadd16(v00, v10);                                          \
  data2 = dpSrc[2];                                                      \
  row00 = vis_faligndata(data0, data1);                                  \
  v12 = vis_fmul8x16al(vis_read_hi(row11), vis_read_hi(yFilter));        \
  row01 = vis_faligndata(data1, data2);                                  \
  filterposx = (X >> FILTER_SHIFT);                                      \
  v13 = vis_fmul8x16al(vis_read_lo(row11), vis_read_hi(yFilter));        \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  v20 = vis_fmul8x16au(vis_read_hi(row20), vis_read_lo(yFilter));        \
  sum1 = vis_fpadd16(v01, v11);                                          \
  data0 = dpSrc[0];                                                      \
  X += dX;                                                               \
  data1 = dpSrc[1];                                                      \
  v21 = vis_fmul8x16au(vis_read_lo(row20), vis_read_lo(yFilter));        \
  sum2 = vis_fpadd16(v02, v12);                                          \
  data2 = dpSrc[2];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  v22 = vis_fmul8x16au(vis_read_hi(row21), vis_read_lo(yFilter));        \
  row11 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  v23 = vis_fmul8x16au(vis_read_lo(row21), vis_read_lo(yFilter));        \
  sum3 = vis_fpadd16(v03, v13);                                          \
  Y += dY;                                                               \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  v30 = vis_fmul8x16al(vis_read_hi(row30), vis_read_lo(yFilter));        \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  data0 = dpSrc[0];                                                      \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  data1 = dpSrc[1];                                                      \
  v31 = vis_fmul8x16al(vis_read_lo(row30), vis_read_lo(yFilter));        \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  data2 = dpSrc[2];                                                      \
  filterposy &= FILTER_MASK;                                             \
  row20 = vis_faligndata(data0, data1);                                  \
  v32 = vis_fmul8x16al(vis_read_hi(row31), vis_read_lo(yFilter));        \
  row21 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  filterposx &= FILTER_MASK;                                             \
  v33 = vis_fmul8x16al(vis_read_lo(row31), vis_read_lo(yFilter));        \
  sum2 = vis_fpadd16(sum2, v22);                                         \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  sum3 = vis_fpadd16(sum3, v23);                                         \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  data0 = dpSrc[0];                                                      \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  v00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  data1 = dpSrc[1];                                                      \
  sum2 = vis_fpadd16(sum2, v32);                                         \
  v01 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  sum3 = vis_fpadd16(sum3, v33);                                         \
  data2 = dpSrc[2];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  v10 = vis_fmul8sux16(sum1, xFilter1);                                  \
  d0##ind = vis_fpadd16(v00, v01);                                       \
  row31 = vis_faligndata(data1, data2);                                  \
  yFilter = *((mlib_d64 *)((mlib_u8 *)mlib_filters_u8 + filterposy));    \
  v11 = vis_fmul8ulx16(sum1, xFilter1);                                  \
  xPtr=((mlib_d64 *)((mlib_u8 *)mlib_filters_u8_4+4*filterposx));        \
  xFilter0 = xPtr[0];                                                    \
  v20 = vis_fmul8sux16(sum2, xFilter2);                                  \
  d1##ind = vis_fpadd16(v10, v11);                                       \
  xFilter1 = xPtr[1];                                                    \
  v21 = vis_fmul8ulx16(sum2, xFilter2);                                  \
  xFilter2 = xPtr[2];                                                    \
  v30 = vis_fmul8sux16(sum3, xFilter3);                                  \
  d2##ind = vis_fpadd16(v20, v21);                                       \
  v31 = vis_fmul8ulx16(sum3, xFilter3);                                  \
  xFilter3 = xPtr[3];                                                    \
  sPtr = (mlib_u8 *)lineAddr[ySrc] + (4*xSrc);                           \
  d3##ind = vis_fpadd16(v30, v31);                                       \

/************************************************************************/

void  mlib_v_ImageAffine_u8_4ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_s32  filterposx, filterposy;
  mlib_d64  data0, data1, data2;
  mlib_d64  sum0, sum1, sum2, sum3;
  mlib_d64  row00, row10, row20, row30;
  mlib_d64  row01, row11, row21, row31;
  mlib_d64  xFilter0, xFilter1, xFilter2, xFilter3, yFilter;
  mlib_d64  v00, v10, v20, v30;
  mlib_d64  v01, v11, v21, v31;
  mlib_d64  v02, v12, v22, v32;
  mlib_d64  v03, v13, v23, v33;
  mlib_d64  d0, d1, d2, d3;
  mlib_d64  d00, d10, d20, d30;
  mlib_d64  d01, d11, d21, d31;
  mlib_d64  *dpSrc;
  mlib_s32  cols, i;
  mlib_d64  res, *dp, *xPtr;
  mlib_s32  mask, emask, gsrd;
  const union table *mlib_filters_table  ;
  const union table *mlib_filters_table_4;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table   = mlib_filters_u8_bc;
    mlib_filters_table_4 = mlib_filters_u8_bc_4;
  } else {
    mlib_filters_table   = mlib_filters_u8_bc2;
    mlib_filters_table_4 = mlib_filters_u8_bc2_4;
  }

  for (j = yStart; j <= yFinish; j++) {

    vis_write_gsr(3 << 3);

    CLIP(4)
    dstLineEnd += 3;
    dp = (mlib_d64*)vis_alignaddr(dstPixelPtr, 0);
    mask = vis_edge8(dstPixelPtr, dstLineEnd);
    gsrd = ((8 - (mlib_addr)dstPixelPtr) & 7);

    cols = xRight - xLeft + 1;
    i = 0;

    if (i <= cols - 6) {

      NEXT_PIXEL_4BC_U8()
      LOAD_BC_U8_4CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )

      NEXT_PIXEL_4BC_U8()

      BC_U8_4CH( 0, mlib_filters_table, mlib_filters_table_4 )
      BC_U8_4CH( 1, mlib_filters_table, mlib_filters_table_4 )
      FADD_4BC_U8()

      BC_U8_4CH( 0, mlib_filters_table, mlib_filters_table_4 )
      BC_U8_4CH( 1, mlib_filters_table, mlib_filters_table_4 )

#pragma pipeloop(0)

      for (; i <= cols-8; i+=2) {
        vis_alignaddr((void *)gsrd, 0);
        res = vis_faligndata(res, res);

        vis_pst_8(res, dp++, mask);
        vis_pst_8(res, dp, ~mask);

        FADD_4BC_U8()
        BC_U8_4CH( 0, mlib_filters_table, mlib_filters_table_4 )
        BC_U8_4CH( 1, mlib_filters_table, mlib_filters_table_4 )
      }

      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);

      vis_pst_8(res, dp++, mask);
      vis_pst_8(res, dp, ~mask);

      FADD_4BC_U8()
      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);

      vis_pst_8(res, dp++, mask);
      vis_pst_8(res, dp, ~mask);

      RESULT_4BC_U8_1PIXEL(0)
      LOAD_BC_U8_4CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_4BC_U8_1PIXEL(1)
      FADD_4BC_U8()

      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);

      vis_pst_8(res, dp++, mask);
      vis_pst_8(res, dp, ~mask);
      i += 6;
    }

    if (i <= cols-4) {
      NEXT_PIXEL_4BC_U8()
      LOAD_BC_U8_4CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )

      NEXT_PIXEL_4BC_U8()

      BC_U8_4CH( 0, mlib_filters_table, mlib_filters_table_4 )
      BC_U8_4CH( 1, mlib_filters_table, mlib_filters_table_4 )
      FADD_4BC_U8()
      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);

      vis_pst_8(res, dp++, mask);
      vis_pst_8(res, dp, ~mask);

      RESULT_4BC_U8_1PIXEL(0)
      LOAD_BC_U8_4CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_4BC_U8_1PIXEL(1)
      FADD_4BC_U8()

      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);

      vis_pst_8(res, dp++, mask);
      vis_pst_8(res, dp, ~mask);
      i += 4;
    }

    if (i <= cols-2) {
      NEXT_PIXEL_4BC_U8()
      LOAD_BC_U8_4CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_4BC_U8_1PIXEL(0)

      NEXT_PIXEL_4BC_U8()
      LOAD_BC_U8_4CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_4BC_U8_1PIXEL(1)
      FADD_4BC_U8()

      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);

      vis_pst_8(res, dp++, mask);
      vis_pst_8(res, dp, ~mask);
      i += 2;
    }

    if (i < cols) {
      NEXT_PIXEL_4BC_U8()
      LOAD_BC_U8_4CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_4BC_U8_1PIXEL(0)

      d0 = vis_fpadd16(d00, d10);
      d1 = vis_fpadd16(d20, d30);
      d0 = vis_fpadd16(d0, d1);
      res = vis_fpack16_pair(d0, d0);
      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);

      emask = vis_edge8(dp, dstLineEnd);
      vis_pst_8(res, dp++, emask & mask);
      if ((mlib_u8*)dp <= (mlib_u8*)dstLineEnd) {
        mask = vis_edge8(dp, dstLineEnd);
        vis_pst_8(res, dp, mask);
      }
    }
  }
}

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s16
#undef  FILTER_SHIFT
#define FILTER_SHIFT 4
#undef  FILTER_MASK
#define FILTER_MASK  (((1 << 9) - 1) << 3)

/************************************************************************/

#define NEXT_PIXEL_1BC_S16()                                             \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  sPtr = (mlib_s16 *)lineAddr[ySrc] + xSrc;

/************************************************************************/

#define LOAD_BC_S16_1CH_1PIXEL( mlib_filters_s16, mlib_filters_s16_4 )   \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row0 = vis_faligndata(data0, data1);                                   \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row1 = vis_faligndata(data0, data1);                                   \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row2 = vis_faligndata(data0, data1);                                   \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  row3 = vis_faligndata(data0, data1);                                   \
  filterposy = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yPtr = ((mlib_d64 *) ((mlib_u8 *)mlib_filters_s16_4+filterposy*4));    \
  yFilter0 = yPtr[0];                                                    \
  yFilter1 = yPtr[1];                                                    \
  yFilter2 = yPtr[2];                                                    \
  yFilter3 = yPtr[3];                                                    \
  filterposx = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xFilter = *((mlib_d64 *)((mlib_u8 *)mlib_filters_s16 + filterposx));   \
  X += dX;                                                               \
  Y += dY;

/************************************************************************/

#define RESULT_1BC_S16_1PIXEL()                                          \
  u0 = vis_fmul8sux16(row0, yFilter0);                                   \
  u1 = vis_fmul8ulx16(row0, yFilter0);                                   \
  u2 = vis_fmul8sux16(row1, yFilter1);                                   \
  v0 = vis_fpadd16(u0, u1);                                              \
  u3 = vis_fmul8ulx16(row1, yFilter1);                                   \
  u0 = vis_fmul8sux16(row2, yFilter2);                                   \
  v1 = vis_fpadd16(u2, u3);                                              \
  u1 = vis_fmul8ulx16(row2, yFilter2);                                   \
  sum = vis_fpadd16(v0, v1);                                             \
  u2 = vis_fmul8sux16(row3, yFilter3);                                   \
  v2 = vis_fpadd16(u0, u1);                                              \
  u3 = vis_fmul8ulx16(row3, yFilter3);                                   \
  sum = vis_fpadd16(sum, v2);                                            \
  v3 = vis_fpadd16(u2, u3);                                              \
  sum = vis_fpadd16(sum, v3);                                            \
  d00 = vis_fmul8sux16(sum, xFilter);                                    \
  d10 = vis_fmul8ulx16(sum, xFilter);                                    \
  d0 = vis_fpadd16(d00, d10);                                            \
  p0 = vis_fpadd16s(vis_read_hi(d0), vis_read_lo(d0));                   \
  d0 = vis_fmuld8sux16( f_x01000100, p0);                                \
  d1 = vis_write_lo(d1, vis_fpadd32s(vis_read_hi(d0), vis_read_lo(d0))); \
  res = vis_fpackfix_pair(d1, d1);

/************************************************************************/

#define BC_S16_1CH( ind, mlib_filters_s16, mlib_filters_s16_4 )          \
  u0 = vis_fmul8sux16(row0, yFilter0);                                   \
  u1 = vis_fmul8ulx16(row0, yFilter0);                                   \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u2 = vis_fmul8sux16(row1, yFilter1);                                   \
  v0 = vis_fpadd16(u0, u1);                                              \
  data0 = dpSrc[0];                                                      \
  filterposy = (Y >> FILTER_SHIFT);                                      \
  u3 = vis_fmul8ulx16(row1, yFilter1);                                   \
  data1 = dpSrc[1];                                                      \
  row0 = vis_faligndata(data0, data1);                                   \
  filterposx = (X >> FILTER_SHIFT);                                      \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u0 = vis_fmul8sux16(row2, yFilter2);                                   \
  v1 = vis_fpadd16(u2, u3);                                              \
  data0 = dpSrc[0];                                                      \
  u1 = vis_fmul8ulx16(row2, yFilter2);                                   \
  sum = vis_fpadd16(v0, v1);                                             \
  X += dX;                                                               \
  data1 = dpSrc[1];                                                      \
  row1 = vis_faligndata(data0, data1);                                   \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u2 = vis_fmul8sux16(row3, yFilter3);                                   \
  v2 = vis_fpadd16(u0, u1);                                              \
  Y += dY;                                                               \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  data0 = dpSrc[0];                                                      \
  u3 = vis_fmul8ulx16(row3, yFilter3);                                   \
  sum = vis_fpadd16(sum, v2);                                            \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  data1 = dpSrc[1];                                                      \
  filterposy &= FILTER_MASK;                                             \
  row2 = vis_faligndata(data0, data1);                                   \
  sPtr += srcYStride;                                                    \
  filterposx &= FILTER_MASK;                                             \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  v3 = vis_fpadd16(u2, u3);                                              \
  data1 = dpSrc[1];                                                      \
  row3 = vis_faligndata(data0, data1);                                   \
  yPtr = ((mlib_d64 *) ((mlib_u8 *)mlib_filters_s16_4+filterposy*4));    \
  yFilter0 = yPtr[0];                                                    \
  sum = vis_fpadd16(sum, v3);                                            \
  yFilter1 = yPtr[1];                                                    \
  d0 = vis_fmul8sux16(sum, xFilter);                                     \
  yFilter2 = yPtr[2];                                                    \
  d1 = vis_fmul8ulx16(sum, xFilter);                                     \
  yFilter3 = yPtr[3];                                                    \
  xFilter = *((mlib_d64 *)((mlib_u8 *)mlib_filters_s16 + filterposx));   \
  d0##ind = vis_fpadd16(d0, d1);                                         \
  sPtr = (mlib_s16 *)lineAddr[ySrc] + xSrc;

/************************************************************************/

#define FADD_1BC_S16()                                                   \
  p0 = vis_fpadd16s(vis_read_hi(d00), vis_read_lo(d00));                 \
  p1 = vis_fpadd16s(vis_read_hi(d01), vis_read_lo(d01));                 \
  p2 = vis_fpadd16s(vis_read_hi(d02), vis_read_lo(d02));                 \
  p3 = vis_fpadd16s(vis_read_hi(d03), vis_read_lo(d03));                 \
  d0 = vis_fmuld8sux16( f_x01000100, p0);                                \
  d1 = vis_fmuld8sux16( f_x01000100, p1);                                \
  d2 = vis_fmuld8sux16( f_x01000100, p2);                                \
  d3 = vis_fmuld8sux16( f_x01000100, p3);                                \
  d0 = vis_freg_pair(vis_fpadd32s(vis_read_hi(d0), vis_read_lo(d0)),     \
                     vis_fpadd32s(vis_read_hi(d1), vis_read_lo(d1)));    \
  d1 = vis_freg_pair(vis_fpadd32s(vis_read_hi(d2), vis_read_lo(d2)),     \
                     vis_fpadd32s(vis_read_hi(d3), vis_read_lo(d3)));    \
  res = vis_fpackfix_pair(d0, d1);

/************************************************************************/

void  mlib_v_ImageAffine_s16_1ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_s32  filterposx, filterposy;
  mlib_d64  data0, data1;
  mlib_d64  sum;
  mlib_d64  row0, row1, row2, row3;
  mlib_f32  p0, p1, p2, p3;
  mlib_d64  xFilter, yFilter0, yFilter1, yFilter2, yFilter3;
  mlib_d64  v0, v1, v2, v3;
  mlib_d64  u0, u1, u2, u3;
  mlib_d64  d0, d1, d2, d3;
  mlib_d64  d00, d10, d01, d02, d03;
  mlib_d64 *yPtr;
  mlib_d64 *dpSrc;
  mlib_s32  align, cols, i;
  mlib_d64  res;
  mlib_f32  f_x01000100 = vis_to_float(0x01000100);
  const union table *mlib_filters_table  ;
  const union table *mlib_filters_table_4;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table   = mlib_filters_s16_bc;
    mlib_filters_table_4 = mlib_filters_s16_bc_4;
  } else {
    mlib_filters_table   = mlib_filters_s16_bc2;
    mlib_filters_table_4 = mlib_filters_s16_bc2_4;
  }

  srcYStride >>= 1;

  for (j = yStart; j <= yFinish; j++) {

    vis_write_gsr(10 << 3);

    CLIP(1)

    cols = xRight - xLeft + 1;
    align = (8 - ((mlib_addr)dstPixelPtr) & 7) & 7;
    align >>= 1;
    align = (cols < align)? cols : align;

    for (i = 0; i < align; i++) {
      NEXT_PIXEL_1BC_S16()
      LOAD_BC_S16_1CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_1BC_S16_1PIXEL()
      vis_st_u16(res, dstPixelPtr++);
    }

    if (i <= cols - 10) {

      NEXT_PIXEL_1BC_S16()
      LOAD_BC_S16_1CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )

      NEXT_PIXEL_1BC_S16()

      BC_S16_1CH( 0, mlib_filters_table, mlib_filters_table_4 )
      BC_S16_1CH( 1, mlib_filters_table, mlib_filters_table_4 )
      BC_S16_1CH( 2, mlib_filters_table, mlib_filters_table_4 )
      BC_S16_1CH( 3, mlib_filters_table, mlib_filters_table_4 )

      FADD_1BC_S16()

      BC_S16_1CH( 0, mlib_filters_table, mlib_filters_table_4 )
      BC_S16_1CH( 1, mlib_filters_table, mlib_filters_table_4 )
      BC_S16_1CH( 2, mlib_filters_table, mlib_filters_table_4 )
      BC_S16_1CH( 3, mlib_filters_table, mlib_filters_table_4 )

#pragma pipeloop(0)
      for (; i <= cols - 14; i+=4) {
        *(mlib_d64*)dstPixelPtr = res;
        FADD_1BC_S16()
        BC_S16_1CH( 0, mlib_filters_table, mlib_filters_table_4 )
        BC_S16_1CH( 1, mlib_filters_table, mlib_filters_table_4 )
        BC_S16_1CH( 2, mlib_filters_table, mlib_filters_table_4 )
        BC_S16_1CH( 3, mlib_filters_table, mlib_filters_table_4 )
        dstPixelPtr += 4;
      }
      *(mlib_d64*)dstPixelPtr = res;
      dstPixelPtr += 4;
      FADD_1BC_S16()
      *(mlib_d64*)dstPixelPtr = res;
      dstPixelPtr += 4;

      RESULT_1BC_S16_1PIXEL()
      vis_st_u16(res, dstPixelPtr++);

      LOAD_BC_S16_1CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_1BC_S16_1PIXEL()
      vis_st_u16(res, dstPixelPtr++);
      i += 10;
    }

    for (; i < cols; i++) {
      NEXT_PIXEL_1BC_S16()
      LOAD_BC_S16_1CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_1BC_S16_1PIXEL()
      vis_st_u16(res, dstPixelPtr++);
    }
  }
}

/************************************************************************/

#define NEXT_PIXEL_2BC_S16()                                             \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  sPtr = (mlib_s16 *)lineAddr[ySrc] + (xSrc<<1);

/************************************************************************/

#define LOAD_BC_S16_2CH_1PIXEL( mlib_filters_s16, mlib_filters_s16_4 )   \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row00 = vis_faligndata(data0, data1);                                  \
  row01 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  row11 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row20 = vis_faligndata(data0, data1);                                  \
  row21 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  row31 = vis_faligndata(data1, data2);                                  \
  filterposy = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yPtr = ((mlib_d64 *) ((mlib_u8 *)mlib_filters_s16_4+filterposy*4));    \
  yFilter0 = yPtr[0];                                                    \
  yFilter1 = yPtr[1];                                                    \
  yFilter2 = yPtr[2];                                                    \
  yFilter3 = yPtr[3];                                                    \
  filterposx = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xFilter = *((mlib_d64 *)((mlib_u8 *)mlib_filters_s16 + filterposx));   \
  X += dX;                                                               \
  Y += dY;                                                               \

/************************************************************************/

#define RESULT_2BC_S16_1PIXEL()                                          \
  u00 = vis_fmul8sux16(row00, yFilter0);                                 \
  dr = vis_fpmerge(vis_read_hi(xFilter), vis_read_lo(xFilter));          \
  u01 = vis_fmul8ulx16(row00, yFilter0);                                 \
  dr = vis_fpmerge(vis_read_hi(dr), vis_read_lo(dr));                    \
  u10 = vis_fmul8sux16(row01, yFilter0);                                 \
  dr1 = vis_fpmerge(vis_read_lo(dr), vis_read_lo(dr));                   \
  u11 = vis_fmul8ulx16(row01, yFilter0);                                 \
  dr = vis_fpmerge(vis_read_hi(dr), vis_read_hi(dr));                    \
  u20 = vis_fmul8sux16(row10, yFilter1);                                 \
  v00 = vis_fpadd16(u00, u01);                                           \
  u21 = vis_fmul8ulx16(row10, yFilter1);                                 \
  v01 = vis_fpadd16(u10, u11);                                           \
  u00 = vis_fmul8sux16(row11, yFilter1);                                 \
  xFilter0 = vis_fpmerge(vis_read_hi(dr), vis_read_hi(dr1));             \
  u01 = vis_fmul8ulx16(row11, yFilter1);                                 \
  u10 = vis_fmul8sux16(row20, yFilter2);                                 \
  u11 = vis_fmul8ulx16(row20, yFilter2);                                 \
  v10 = vis_fpadd16(u20, u21);                                           \
  sum0 = vis_fpadd16(v00, v10);                                          \
  u20 = vis_fmul8sux16(row21, yFilter2);                                 \
  v11 = vis_fpadd16(u00, u01);                                           \
  u21 = vis_fmul8ulx16(row21, yFilter2);                                 \
  xFilter1 = vis_fpmerge(vis_read_lo(dr), vis_read_lo(dr1));             \
  u00 = vis_fmul8sux16(row30, yFilter3);                                 \
  v20 = vis_fpadd16(u10, u11);                                           \
  sum1 = vis_fpadd16(v01, v11);                                          \
  u01 = vis_fmul8ulx16(row30, yFilter3);                                 \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  v21 = vis_fpadd16(u20, u21);                                           \
  u10 = vis_fmul8sux16(row31, yFilter3);                                 \
  v30 = vis_fpadd16(u00, u01);                                           \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  u11 = vis_fmul8ulx16(row31, yFilter3);                                 \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  v31 = vis_fpadd16(u10, u11);                                           \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  d00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  d10 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  d20 = vis_fmul8sux16(sum1, xFilter1);                                  \
  d30 = vis_fmul8ulx16(sum1, xFilter1);                                  \
  d0 = vis_fpadd16(d00, d10);                                            \
  d1 = vis_fpadd16(d20, d30);                                            \
  d0 = vis_fpadd16(d0, d1);                                              \
  p0 = vis_fpadd16s(vis_read_hi(d0), vis_read_lo(d0));                   \
  d0 = vis_fmuld8sux16( f_x01000100, p0);                                \
  res = vis_fpackfix_pair(d0, d0);

/************************************************************************/

#define BC_S16_2CH( ind, mlib_filters_s16, mlib_filters_s16_4 )          \
  u00 = vis_fmul8sux16(row00, yFilter0);                                 \
  dr = vis_fpmerge(vis_read_hi(xFilter), vis_read_lo(xFilter));          \
  u01 = vis_fmul8ulx16(row00, yFilter0);                                 \
  dr = vis_fpmerge(vis_read_hi(dr), vis_read_lo(dr));                    \
  u10 = vis_fmul8sux16(row01, yFilter0);                                 \
  dr1 = vis_fpmerge(vis_read_lo(dr), vis_read_lo(dr));                   \
  u11 = vis_fmul8ulx16(row01, yFilter0);                                 \
  dr = vis_fpmerge(vis_read_hi(dr), vis_read_hi(dr));                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u20 = vis_fmul8sux16(row10, yFilter1);                                 \
  v00 = vis_fpadd16(u00, u01);                                           \
  u21 = vis_fmul8ulx16(row10, yFilter1);                                 \
  data0 = dpSrc[0];                                                      \
  filterposy = (Y >> FILTER_SHIFT);                                      \
  v01 = vis_fpadd16(u10, u11);                                           \
  data1 = dpSrc[1];                                                      \
  u00 = vis_fmul8sux16(row11, yFilter1);                                 \
  xFilter0 = vis_fpmerge(vis_read_hi(dr), vis_read_hi(dr1));             \
  data2 = dpSrc[2];                                                      \
  u01 = vis_fmul8ulx16(row11, yFilter1);                                 \
  row00 = vis_faligndata(data0, data1);                                  \
  u10 = vis_fmul8sux16(row20, yFilter2);                                 \
  row01 = vis_faligndata(data1, data2);                                  \
  filterposx = (X >> FILTER_SHIFT);                                      \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u11 = vis_fmul8ulx16(row20, yFilter2);                                 \
  v10 = vis_fpadd16(u20, u21);                                           \
  data0 = dpSrc[0];                                                      \
  sum0 = vis_fpadd16(v00, v10);                                          \
  X += dX;                                                               \
  data1 = dpSrc[1];                                                      \
  u20 = vis_fmul8sux16(row21, yFilter2);                                 \
  v11 = vis_fpadd16(u00, u01);                                           \
  data2 = dpSrc[2];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  u21 = vis_fmul8ulx16(row21, yFilter2);                                 \
  row11 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  xFilter1 = vis_fpmerge(vis_read_lo(dr), vis_read_lo(dr1));             \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u00 = vis_fmul8sux16(row30, yFilter3);                                 \
  v20 = vis_fpadd16(u10, u11);                                           \
  Y += dY;                                                               \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  sum1 = vis_fpadd16(v01, v11);                                          \
  data0 = dpSrc[0];                                                      \
  u01 = vis_fmul8ulx16(row30, yFilter3);                                 \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  data1 = dpSrc[1];                                                      \
  v21 = vis_fpadd16(u20, u21);                                           \
  u10 = vis_fmul8sux16(row31, yFilter3);                                 \
  data2 = dpSrc[2];                                                      \
  v30 = vis_fpadd16(u00, u01);                                           \
  filterposy &= FILTER_MASK;                                             \
  row20 = vis_faligndata(data0, data1);                                  \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  u11 = vis_fmul8ulx16(row31, yFilter3);                                 \
  row21 = vis_faligndata(data1, data2);                                  \
  sPtr += srcYStride;                                                    \
  filterposx &= FILTER_MASK;                                             \
  v31 = vis_fpadd16(u10, u11);                                           \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  data1 = dpSrc[1];                                                      \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  data2 = dpSrc[2];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  d0 = vis_fmul8sux16(sum0, xFilter0);                                   \
  row31 = vis_faligndata(data1, data2);                                  \
  yPtr = ((mlib_d64 *) ((mlib_u8 *)mlib_filters_s16_4+filterposy*4));    \
  d1 = vis_fmul8ulx16(sum0, xFilter0);                                   \
  yFilter0 = yPtr[0];                                                    \
  d2 = vis_fmul8sux16(sum1, xFilter1);                                   \
  yFilter1 = yPtr[1];                                                    \
  d3 = vis_fmul8ulx16(sum1, xFilter1);                                   \
  d0##ind = vis_fpadd16(d0, d1);                                         \
  yFilter2 = yPtr[2];                                                    \
  yFilter3 = yPtr[3];                                                    \
  d1##ind = vis_fpadd16(d2, d3);                                         \
  xFilter = *((mlib_d64 *)((mlib_u8 *)mlib_filters_s16 + filterposx));   \
  sPtr = (mlib_s16 *)lineAddr[ySrc] + (xSrc<<1);

/************************************************************************/

#define FADD_2BC_S16()                                                   \
  d0 = vis_fpadd16(d00, d10);                                            \
  d2 = vis_fpadd16(d01, d11);                                            \
  p0 = vis_fpadd16s(vis_read_hi(d0), vis_read_lo(d0));                   \
  p1 = vis_fpadd16s(vis_read_hi(d2), vis_read_lo(d2));                   \
  d0 = vis_fmuld8sux16( f_x01000100, p0);                                \
  d1 = vis_fmuld8sux16( f_x01000100, p1);                                \
  res = vis_fpackfix_pair(d0, d1);

/************************************************************************/

void  mlib_v_ImageAffine_s16_2ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_s32  filterposx, filterposy;
  mlib_d64  data0, data1, data2;
  mlib_d64  sum0, sum1;
  mlib_d64  row00, row10, row20, row30;
  mlib_d64  row01, row11, row21, row31;
  mlib_f32  p0, p1;
  mlib_d64  xFilter, xFilter0, xFilter1;
  mlib_d64  yFilter0, yFilter1, yFilter2, yFilter3;
  mlib_d64  v00, v01, v10, v11, v20, v21, v30, v31;
  mlib_d64  u00, u01, u10, u11, u20, u21;
  mlib_d64  d0, d1, d2, d3;
  mlib_d64  d00, d10, d20, d30, d01, d11;
  mlib_d64  *yPtr;
  mlib_d64  *dp, *dpSrc;
  mlib_s32  cols, i, mask, emask;
  mlib_d64  res, res1;
  mlib_d64  dr, dr1;
  mlib_f32 f_x01000100 = vis_to_float(0x01000100);
  const union table *mlib_filters_table  ;
  const union table *mlib_filters_table_4;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table   = mlib_filters_s16_bc;
    mlib_filters_table_4 = mlib_filters_s16_bc_4;
  } else {
    mlib_filters_table   = mlib_filters_s16_bc2;
    mlib_filters_table_4 = mlib_filters_s16_bc2_4;
  }

  srcYStride >>= 1;

  for (j = yStart; j <= yFinish; j++) {

    vis_write_gsr(10 << 3);

    CLIP(2)

    cols = xRight - xLeft + 1;
    dp = vis_alignaddr(dstPixelPtr, 0);
    dstLineEnd += 1;
    mask = vis_edge16(dstPixelPtr, dstLineEnd);
    i = 0;

    if (i <= cols - 6) {

      NEXT_PIXEL_2BC_S16()
      LOAD_BC_S16_2CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )

      NEXT_PIXEL_2BC_S16()

      BC_S16_2CH( 0, mlib_filters_table, mlib_filters_table_4 )
      BC_S16_2CH( 1, mlib_filters_table, mlib_filters_table_4 )

      FADD_2BC_S16()

      BC_S16_2CH( 0, mlib_filters_table, mlib_filters_table_4 )
      BC_S16_2CH( 1, mlib_filters_table, mlib_filters_table_4 )

#pragma pipeloop(0)

      for (; i <= cols-8; i+=2) {
        vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
        res = vis_faligndata(res, res);
        vis_pst_16(res, dp++, mask);
        vis_pst_16(res, dp, ~mask);
        FADD_2BC_S16()
        BC_S16_2CH( 0, mlib_filters_table, mlib_filters_table_4 )
        BC_S16_2CH( 1, mlib_filters_table, mlib_filters_table_4 )
      }
      vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);

      FADD_2BC_S16()
      vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);

      RESULT_2BC_S16_1PIXEL()
      res1 = res;

      LOAD_BC_S16_2CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_2BC_S16_1PIXEL()
      res = vis_write_hi(res, vis_read_hi(res1));
      vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);

      i += 6;
    }

    if (i <= cols - 4) {
      NEXT_PIXEL_2BC_S16()
      LOAD_BC_S16_2CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )

      NEXT_PIXEL_2BC_S16()

      BC_S16_2CH( 0, mlib_filters_table, mlib_filters_table_4 )
      BC_S16_2CH( 1, mlib_filters_table, mlib_filters_table_4 )

      FADD_2BC_S16()
      vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);

      RESULT_2BC_S16_1PIXEL()
      res1 = res;

      LOAD_BC_S16_2CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_2BC_S16_1PIXEL()
      res = vis_write_hi(res, vis_read_hi(res1));
      vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);

      i += 4;
    }

    if (i <= cols - 2) {
      NEXT_PIXEL_2BC_S16()
      LOAD_BC_S16_2CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_2BC_S16_1PIXEL()
      res1 = res;

      NEXT_PIXEL_2BC_S16()
      LOAD_BC_S16_2CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_2BC_S16_1PIXEL()
      res = vis_write_hi(res, vis_read_hi(res1));
      vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);

      i += 2;
    }

    if (i < cols) {
      NEXT_PIXEL_2BC_S16()
      LOAD_BC_S16_2CH_1PIXEL( mlib_filters_table, mlib_filters_table_4 )
      RESULT_2BC_S16_1PIXEL()
      vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
      res = vis_faligndata(res, res);
      emask = vis_edge16(dp, dstLineEnd);
      vis_pst_16(res, dp++, mask & emask);
      if ((mlib_s16*)dp <= dstLineEnd) {
        mask = vis_edge16(dp, dstLineEnd);
        vis_pst_16(res, dp, mask);
      }
    }
  }
}


/************************************************************************/

#define NEXT_PIXEL_3BC_S16()                                             \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  sPtr = (mlib_s16 *)lineAddr[ySrc] + (xSrc*3);

/************************************************************************/

#define LOAD_BC_S16_3CH_1PIXEL( mlib_filters_s16_3, mlib_filters_s16_4 ) \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  data3 = dpSrc[3];                                                      \
  row00 = vis_faligndata(data0, data1);                                  \
  row01 = vis_faligndata(data1, data2);                                  \
  row02 = vis_faligndata(data2, data3);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  data3 = dpSrc[3];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  row11 = vis_faligndata(data1, data2);                                  \
  row12 = vis_faligndata(data2, data3);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  data3 = dpSrc[3];                                                      \
  row20 = vis_faligndata(data0, data1);                                  \
  row21 = vis_faligndata(data1, data2);                                  \
  row22 = vis_faligndata(data2, data3);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  data3 = dpSrc[3];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  row31 = vis_faligndata(data1, data2);                                  \
  row32 = vis_faligndata(data2, data3);                                  \
  filterposy = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yPtr = ((mlib_d64 *) ((mlib_u8 *)mlib_filters_s16_4+filterposy*4)); \
  yFilter0 = yPtr[0];                                                    \
  yFilter1 = yPtr[1];                                                    \
  yFilter2 = yPtr[2];                                                    \
  yFilter3 = yPtr[3];                                                    \
  filterposx = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xPtr = ((mlib_d64 *)((mlib_u8 *)mlib_filters_s16_3+filterposx*3));  \
  xFilter0 = xPtr[0];                                                    \
  xFilter1 = xPtr[1];                                                    \
  xFilter2 = xPtr[2];                                                    \
  X += dX;                                                               \
  Y += dY;

/************************************************************************/

#define STORE_BC_S16_3CH_1PIXEL()                                        \
 dstPixelPtr[0] = f0.t[0];                                               \
 dstPixelPtr[1] = f0.t[1];                                               \
 dstPixelPtr[2] = f0.t[2];                                               \
 dstPixelPtr += 3;

/************************************************************************/

#define RESULT_3BC_S16_1PIXEL()                                          \
  u00 = vis_fmul8sux16(row00, yFilter0);                                 \
  u01 = vis_fmul8ulx16(row00, yFilter0);                                 \
  u10 = vis_fmul8sux16(row01, yFilter0);                                 \
  u11 = vis_fmul8ulx16(row01, yFilter0);                                 \
  v00 = vis_fpadd16(u00, u01);                                           \
  u20 = vis_fmul8sux16(row02, yFilter0);                                 \
  v01 = vis_fpadd16(u10, u11);                                           \
  u21 = vis_fmul8ulx16(row02, yFilter0);                                 \
  u00 = vis_fmul8sux16(row10, yFilter1);                                 \
  u01 = vis_fmul8ulx16(row10, yFilter1);                                 \
  v02 = vis_fpadd16(u20, u21);                                           \
  u10 = vis_fmul8sux16(row11, yFilter1);                                 \
  u11 = vis_fmul8ulx16(row11, yFilter1);                                 \
  v10 = vis_fpadd16(u00, u01);                                           \
  u20 = vis_fmul8sux16(row12, yFilter1);                                 \
  u21 = vis_fmul8ulx16(row12, yFilter1);                                 \
  u00 = vis_fmul8sux16(row20, yFilter2);                                 \
  v11 = vis_fpadd16(u10, u11);                                           \
  u01 = vis_fmul8ulx16(row20, yFilter2);                                 \
  v12 = vis_fpadd16(u20, u21);                                           \
  u10 = vis_fmul8sux16(row21, yFilter2);                                 \
  u11 = vis_fmul8ulx16(row21, yFilter2);                                 \
  v20 = vis_fpadd16(u00, u01);                                           \
  u20 = vis_fmul8sux16(row22, yFilter2);                                 \
  sum0 = vis_fpadd16(v00, v10);                                          \
  u21 = vis_fmul8ulx16(row22, yFilter2);                                 \
  u00 = vis_fmul8sux16(row30, yFilter3);                                 \
  u01 = vis_fmul8ulx16(row30, yFilter3);                                 \
  v21 = vis_fpadd16(u10, u11);                                           \
  sum1 = vis_fpadd16(v01, v11);                                          \
  u10 = vis_fmul8sux16(row31, yFilter3);                                 \
  sum2 = vis_fpadd16(v02, v12);                                          \
  v22 = vis_fpadd16(u20, u21);                                           \
  u11 = vis_fmul8ulx16(row31, yFilter3);                                 \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  u20 = vis_fmul8sux16(row32, yFilter3);                                 \
  v30 = vis_fpadd16(u00, u01);                                           \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  u21 = vis_fmul8ulx16(row32, yFilter3);                                 \
  v31 = vis_fpadd16(u10, u11);                                           \
  sum2 = vis_fpadd16(sum2, v22);                                         \
  v32 = vis_fpadd16(u20, u21);                                           \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  row30 = vis_faligndata(data0, data1);                                  \
  v00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  sum2 = vis_fpadd16(sum2, v32);                                         \
  v01 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  v10 = vis_fmul8sux16(sum1, xFilter1);                                  \
  v11 = vis_fmul8ulx16(sum1, xFilter1);                                  \
  d0 = vis_fpadd16(v00, v01);                                            \
  v20 = vis_fmul8sux16(sum2, xFilter2);                                  \
  v21 = vis_fmul8ulx16(sum2, xFilter2);                                  \
  d1 = vis_fpadd16(v10, v11);                                            \
  d2 = vis_fpadd16(v20, v21);                                            \
  vis_alignaddr((void*)6, 0);                                            \
  d3 = vis_faligndata(d0, d1);                                           \
  vis_alignaddr((void*)2, 0);                                            \
  d4 = vis_faligndata(d1, d2);                                           \
  d0 = vis_fpadd16(d0, d3);                                              \
  d2 = vis_fpadd16(d2, d4);                                              \
  d1 = vis_faligndata(d2, d2);                                           \
  d0 = vis_fpadd16(d0, d1);                                              \
  d2 = vis_fmuld8sux16( f_x01000100, vis_read_hi(d0));                   \
  d3 = vis_fmuld8sux16( f_x01000100, vis_read_lo(d0));                   \
  f0.d = vis_fpackfix_pair(d2, d3);

/************************************************************************/

#define BC_S16_3CH( mlib_filters_s16_3, mlib_filters_s16_4 )             \
  u00 = vis_fmul8sux16(row00, yFilter0);                                 \
  u01 = vis_fmul8ulx16(row00, yFilter0);                                 \
  u10 = vis_fmul8sux16(row01, yFilter0);                                 \
  u11 = vis_fmul8ulx16(row01, yFilter0);                                 \
  v00 = vis_fpadd16(u00, u01);                                           \
  u20 = vis_fmul8sux16(row02, yFilter0);                                 \
  v01 = vis_fpadd16(u10, u11);                                           \
  u21 = vis_fmul8ulx16(row02, yFilter0);                                 \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u00 = vis_fmul8sux16(row10, yFilter1);                                 \
  u01 = vis_fmul8ulx16(row10, yFilter1);                                 \
  data0 = dpSrc[0];                                                      \
  filterposy = (Y >> FILTER_SHIFT);                                      \
  v02 = vis_fpadd16(u20, u21);                                           \
  data1 = dpSrc[1];                                                      \
  u10 = vis_fmul8sux16(row11, yFilter1);                                 \
  data2 = dpSrc[2];                                                      \
  u11 = vis_fmul8ulx16(row11, yFilter1);                                 \
  v10 = vis_fpadd16(u00, u01);                                           \
  data3 = dpSrc[3];                                                      \
  u20 = vis_fmul8sux16(row12, yFilter1);                                 \
  row00 = vis_faligndata(data0, data1);                                  \
  u21 = vis_fmul8ulx16(row12, yFilter1);                                 \
  row01 = vis_faligndata(data1, data2);                                  \
  u00 = vis_fmul8sux16(row20, yFilter2);                                 \
  row02 = vis_faligndata(data2, data3);                                  \
  filterposx = (X >> FILTER_SHIFT);                                      \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  v11 = vis_fpadd16(u10, u11);                                           \
  u01 = vis_fmul8ulx16(row20, yFilter2);                                 \
  v12 = vis_fpadd16(u20, u21);                                           \
  data0 = dpSrc[0];                                                      \
  u10 = vis_fmul8sux16(row21, yFilter2);                                 \
  X += dX;                                                               \
  data1 = dpSrc[1];                                                      \
  u11 = vis_fmul8ulx16(row21, yFilter2);                                 \
  v20 = vis_fpadd16(u00, u01);                                           \
  data2 = dpSrc[2];                                                      \
  u20 = vis_fmul8sux16(row22, yFilter2);                                 \
  sum0 = vis_fpadd16(v00, v10);                                          \
  data3 = dpSrc[3];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  u21 = vis_fmul8ulx16(row22, yFilter2);                                 \
  row11 = vis_faligndata(data1, data2);                                  \
  u00 = vis_fmul8sux16(row30, yFilter3);                                 \
  row12 = vis_faligndata(data2, data3);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u01 = vis_fmul8ulx16(row30, yFilter3);                                 \
  v21 = vis_fpadd16(u10, u11);                                           \
  Y += dY;                                                               \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  sum1 = vis_fpadd16(v01, v11);                                          \
  data0 = dpSrc[0];                                                      \
  u10 = vis_fmul8sux16(row31, yFilter3);                                 \
  sum2 = vis_fpadd16(v02, v12);                                          \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  data1 = dpSrc[1];                                                      \
  v22 = vis_fpadd16(u20, u21);                                           \
  u11 = vis_fmul8ulx16(row31, yFilter3);                                 \
  data2 = dpSrc[2];                                                      \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  u20 = vis_fmul8sux16(row32, yFilter3);                                 \
  data3 = dpSrc[3];                                                      \
  v30 = vis_fpadd16(u00, u01);                                           \
  filterposy &= FILTER_MASK;                                             \
  row20 = vis_faligndata(data0, data1);                                  \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  u21 = vis_fmul8ulx16(row32, yFilter3);                                 \
  row21 = vis_faligndata(data1, data2);                                  \
  row22 = vis_faligndata(data2, data3);                                  \
  sPtr += srcYStride;                                                    \
  filterposx &= FILTER_MASK;                                             \
  v31 = vis_fpadd16(u10, u11);                                           \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  sum2 = vis_fpadd16(sum2, v22);                                         \
  data1 = dpSrc[1];                                                      \
  v32 = vis_fpadd16(u20, u21);                                           \
  data2 = dpSrc[2];                                                      \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  data3 = dpSrc[3];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  v00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  row31 = vis_faligndata(data1, data2);                                  \
  row32 = vis_faligndata(data2, data3);                                  \
  yPtr = ((mlib_d64 *) ((mlib_u8 *)mlib_filters_s16_4+filterposy*4));    \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  yFilter0 = yPtr[0];                                                    \
  sum2 = vis_fpadd16(sum2, v32);                                         \
  v01 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  yFilter1 = yPtr[1];                                                    \
  v10 = vis_fmul8sux16(sum1, xFilter1);                                  \
  yFilter2 = yPtr[2];                                                    \
  v11 = vis_fmul8ulx16(sum1, xFilter1);                                  \
  d0 = vis_fpadd16(v00, v01);                                            \
  yFilter3 = yPtr[3];                                                    \
  xPtr = ((mlib_d64 *)((mlib_u8 *)mlib_filters_s16_3+filterposx*3));     \
  v20 = vis_fmul8sux16(sum2, xFilter2);                                  \
  xFilter0 = xPtr[0];                                                    \
  v21 = vis_fmul8ulx16(sum2, xFilter2);                                  \
  d1 = vis_fpadd16(v10, v11);                                            \
  xFilter1 = xPtr[1];                                                    \
  d2 = vis_fpadd16(v20, v21);                                            \
  xFilter2 = xPtr[2];                                                    \
  sPtr = (mlib_s16 *)lineAddr[ySrc] + (xSrc*3);

/************************************************************************/

#define FADD_3BC_S16()                                                   \
  vis_alignaddr((void*)6, 0);                                            \
  d3 = vis_faligndata(d0, d1);                                           \
  vis_alignaddr((void*)2, 0);                                            \
  d4 = vis_faligndata(d1, d2);                                           \
  d0 = vis_fpadd16(d0, d3);                                              \
  d2 = vis_fpadd16(d2, d4);                                              \
  d1 = vis_faligndata(d2, d2);                                           \
  d0 = vis_fpadd16(d0, d1);                                              \
  d2 = vis_fmuld8sux16( f_x01000100, vis_read_hi(d0));                   \
  d3 = vis_fmuld8sux16( f_x01000100, vis_read_lo(d0));                   \
  f0.d = vis_fpackfix_pair(d2, d3);

/************************************************************************/

void  mlib_v_ImageAffine_s16_3ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_s32  filterposx, filterposy;
  mlib_d64  data0, data1, data2, data3;
  mlib_d64  sum0, sum1, sum2;
  mlib_d64  row00, row10, row20, row30;
  mlib_d64  row01, row11, row21, row31;
  mlib_d64  row02, row12, row22, row32;
  mlib_d64  xFilter0, xFilter1, xFilter2;
  mlib_d64  yFilter0, yFilter1, yFilter2, yFilter3;
  mlib_d64  v00, v01, v02, v10, v11, v12, v20, v21, v22, v30, v31, v32;
  mlib_d64  u00, u01, u10, u11, u20, u21;
  mlib_d64  d0, d1, d2, d3, d4;
  mlib_d64 *yPtr, *xPtr;
  mlib_d64 *dpSrc;
  mlib_s32  cols, i;
  mlib_f32  f_x01000100 = vis_to_float(0x01000100);
  union {
    mlib_s16 t[4];
    mlib_d64 d;
  } f0;
  const union table *mlib_filters_table_3;
  const union table *mlib_filters_table_4;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table_3 = mlib_filters_s16_bc_3;
    mlib_filters_table_4 = mlib_filters_s16_bc_4;
  } else {
    mlib_filters_table_3 = mlib_filters_s16_bc2_3;
    mlib_filters_table_4 = mlib_filters_s16_bc2_4;
  }

  srcYStride >>= 1;

  for (j = yStart; j <= yFinish; j++) {

    vis_write_gsr(10 << 3);

    CLIP(3)

    cols = xRight - xLeft + 1;

    i = 0;

    if (i <= cols - 4) {

      NEXT_PIXEL_3BC_S16()
      LOAD_BC_S16_3CH_1PIXEL( mlib_filters_table_3, mlib_filters_table_4 )

      NEXT_PIXEL_3BC_S16()

      BC_S16_3CH( mlib_filters_table_3, mlib_filters_table_4 )
      FADD_3BC_S16()

      BC_S16_3CH( mlib_filters_table_3, mlib_filters_table_4 )

#pragma pipeloop(0)

      for (; i < cols-4; i++) {
        STORE_BC_S16_3CH_1PIXEL()

        FADD_3BC_S16()
        BC_S16_3CH( mlib_filters_table_3, mlib_filters_table_4 )
      }

      STORE_BC_S16_3CH_1PIXEL()

      FADD_3BC_S16()
      STORE_BC_S16_3CH_1PIXEL()

      RESULT_3BC_S16_1PIXEL()
      STORE_BC_S16_3CH_1PIXEL()

      LOAD_BC_S16_3CH_1PIXEL( mlib_filters_table_3, mlib_filters_table_4 )
      RESULT_3BC_S16_1PIXEL()
      STORE_BC_S16_3CH_1PIXEL()
      i += 4;
    }

    for (; i < cols; i++) {
      NEXT_PIXEL_3BC_S16()
      LOAD_BC_S16_3CH_1PIXEL( mlib_filters_table_3, mlib_filters_table_4 )
      RESULT_3BC_S16_1PIXEL()
      STORE_BC_S16_3CH_1PIXEL()
    }
  }
}

/************************************************************************/

#define NEXT_PIXEL_4BC_S16()                                             \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  sPtr = (mlib_s16 *)lineAddr[ySrc] + (xSrc<<2);

/************************************************************************/

#define LOAD_BC_S16_4CH_1PIXEL( mlib_filters_s16_4 )                     \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  data3 = dpSrc[3];                                                      \
  data4 = dpSrc[4];                                                      \
  row00 = vis_faligndata(data0, data1);                                  \
  row01 = vis_faligndata(data1, data2);                                  \
  row02 = vis_faligndata(data2, data3);                                  \
  row03 = vis_faligndata(data3, data4);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  data3 = dpSrc[3];                                                      \
  data4 = dpSrc[4];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  row11 = vis_faligndata(data1, data2);                                  \
  row12 = vis_faligndata(data2, data3);                                  \
  row13 = vis_faligndata(data3, data4);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  data3 = dpSrc[3];                                                      \
  data4 = dpSrc[4];                                                      \
  row20 = vis_faligndata(data0, data1);                                  \
  row21 = vis_faligndata(data1, data2);                                  \
  row22 = vis_faligndata(data2, data3);                                  \
  row23 = vis_faligndata(data3, data4);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  data1 = dpSrc[1];                                                      \
  data2 = dpSrc[2];                                                      \
  data3 = dpSrc[3];                                                      \
  data4 = dpSrc[4];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  row31 = vis_faligndata(data1, data2);                                  \
  row32 = vis_faligndata(data2, data3);                                  \
  row33 = vis_faligndata(data3, data4);                                  \
  filterposy = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yPtr = ((mlib_d64 *) ((mlib_u8 *)mlib_filters_s16_4+filterposy*4));    \
  yFilter0 = yPtr[0];                                                    \
  yFilter1 = yPtr[1];                                                    \
  yFilter2 = yPtr[2];                                                    \
  yFilter3 = yPtr[3];                                                    \
  filterposx = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xPtr = ((mlib_d64 *)((mlib_u8 *)mlib_filters_s16_4+filterposx*4));     \
  xFilter0 = xPtr[0];                                                    \
  xFilter1 = xPtr[1];                                                    \
  xFilter2 = xPtr[2];                                                    \
  xFilter3 = xPtr[3];                                                    \
  X += dX;                                                               \
  Y += dY;

/************************************************************************/

#define RESULT_4BC_S16_1PIXEL()                                          \
  u00 = vis_fmul8sux16(row00, yFilter0);                                 \
  u01 = vis_fmul8ulx16(row00, yFilter0);                                 \
  u10 = vis_fmul8sux16(row01, yFilter0);                                 \
  u11 = vis_fmul8ulx16(row01, yFilter0);                                 \
  v00 = vis_fpadd16(u00, u01);                                           \
  u20 = vis_fmul8sux16(row02, yFilter0);                                 \
  v01 = vis_fpadd16(u10, u11);                                           \
  u21 = vis_fmul8ulx16(row02, yFilter0);                                 \
  u30 = vis_fmul8sux16(row03, yFilter0);                                 \
  u31 = vis_fmul8ulx16(row03, yFilter0);                                 \
  v02 = vis_fpadd16(u20, u21);                                           \
  u00 = vis_fmul8sux16(row10, yFilter1);                                 \
  u01 = vis_fmul8ulx16(row10, yFilter1);                                 \
  v03 = vis_fpadd16(u30, u31);                                           \
  u10 = vis_fmul8sux16(row11, yFilter1);                                 \
  u11 = vis_fmul8ulx16(row11, yFilter1);                                 \
  v10 = vis_fpadd16(u00, u01);                                           \
  u20 = vis_fmul8sux16(row12, yFilter1);                                 \
  v11 = vis_fpadd16(u10, u11);                                           \
  u21 = vis_fmul8ulx16(row12, yFilter1);                                 \
  u30 = vis_fmul8sux16(row13, yFilter1);                                 \
  u31 = vis_fmul8ulx16(row13, yFilter1);                                 \
  u00 = vis_fmul8sux16(row20, yFilter2);                                 \
  v12 = vis_fpadd16(u20, u21);                                           \
  u01 = vis_fmul8ulx16(row20, yFilter2);                                 \
  v13 = vis_fpadd16(u30, u31);                                           \
  u10 = vis_fmul8sux16(row21, yFilter2);                                 \
  u11 = vis_fmul8ulx16(row21, yFilter2);                                 \
  v20 = vis_fpadd16(u00, u01);                                           \
  u20 = vis_fmul8sux16(row22, yFilter2);                                 \
  sum0 = vis_fpadd16(v00, v10);                                          \
  u21 = vis_fmul8ulx16(row22, yFilter2);                                 \
  u30 = vis_fmul8sux16(row23, yFilter2);                                 \
  u31 = vis_fmul8ulx16(row23, yFilter2);                                 \
  u00 = vis_fmul8sux16(row30, yFilter3);                                 \
  u01 = vis_fmul8ulx16(row30, yFilter3);                                 \
  v21 = vis_fpadd16(u10, u11);                                           \
  sum1 = vis_fpadd16(v01, v11);                                          \
  u10 = vis_fmul8sux16(row31, yFilter3);                                 \
  sum2 = vis_fpadd16(v02, v12);                                          \
  sum3 = vis_fpadd16(v03, v13);                                          \
  v22 = vis_fpadd16(u20, u21);                                           \
  u11 = vis_fmul8ulx16(row31, yFilter3);                                 \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  u20 = vis_fmul8sux16(row32, yFilter3);                                 \
  u21 = vis_fmul8ulx16(row32, yFilter3);                                 \
  v23 = vis_fpadd16(u30, u31);                                           \
  v30 = vis_fpadd16(u00, u01);                                           \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  u30 = vis_fmul8sux16(row33, yFilter3);                                 \
  u31 = vis_fmul8ulx16(row33, yFilter3);                                 \
  v31 = vis_fpadd16(u10, u11);                                           \
  sum2 = vis_fpadd16(sum2, v22);                                         \
  sum3 = vis_fpadd16(sum3, v23);                                         \
  v32 = vis_fpadd16(u20, u21);                                           \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  v33 = vis_fpadd16(u30, u31);                                           \
  v00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  sum2 = vis_fpadd16(sum2, v32);                                         \
  v01 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  v10 = vis_fmul8sux16(sum1, xFilter1);                                  \
  sum3 = vis_fpadd16(sum3, v33);                                         \
  v11 = vis_fmul8ulx16(sum1, xFilter1);                                  \
  d0 = vis_fpadd16(v00, v01);                                            \
  v20 = vis_fmul8sux16(sum2, xFilter2);                                  \
  v21 = vis_fmul8ulx16(sum2, xFilter2);                                  \
  d1 = vis_fpadd16(v10, v11);                                            \
  v30 = vis_fmul8sux16(sum3, xFilter3);                                  \
  v31 = vis_fmul8ulx16(sum3, xFilter3);                                  \
  d2 = vis_fpadd16(v20, v21);                                            \
  d3 = vis_fpadd16(v30, v31);                                            \
  d0 = vis_fpadd16(d0, d1);                                              \
  d2 = vis_fpadd16(d2, d3);                                              \
  d0 = vis_fpadd16(d0, d2);                                              \
  d2 = vis_fmuld8sux16( f_x01000100, vis_read_hi(d0));                   \
  d3 = vis_fmuld8sux16( f_x01000100, vis_read_lo(d0));                   \
  res = vis_fpackfix_pair(d2, d3);

/************************************************************************/

#define BC_S16_4CH( mlib_filters_s16_4 )                                 \
  u00 = vis_fmul8sux16(row00, yFilter0);                                 \
  u01 = vis_fmul8ulx16(row00, yFilter0);                                 \
  u10 = vis_fmul8sux16(row01, yFilter0);                                 \
  u11 = vis_fmul8ulx16(row01, yFilter0);                                 \
  v00 = vis_fpadd16(u00, u01);                                           \
  u20 = vis_fmul8sux16(row02, yFilter0);                                 \
  v01 = vis_fpadd16(u10, u11);                                           \
  u21 = vis_fmul8ulx16(row02, yFilter0);                                 \
  u30 = vis_fmul8sux16(row03, yFilter0);                                 \
  u31 = vis_fmul8ulx16(row03, yFilter0);                                 \
  v02 = vis_fpadd16(u20, u21);                                           \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u00 = vis_fmul8sux16(row10, yFilter1);                                 \
  u01 = vis_fmul8ulx16(row10, yFilter1);                                 \
  data0 = dpSrc[0];                                                      \
  filterposy = (Y >> FILTER_SHIFT);                                      \
  v03 = vis_fpadd16(u30, u31);                                           \
  data1 = dpSrc[1];                                                      \
  u10 = vis_fmul8sux16(row11, yFilter1);                                 \
  data2 = dpSrc[2];                                                      \
  u11 = vis_fmul8ulx16(row11, yFilter1);                                 \
  v10 = vis_fpadd16(u00, u01);                                           \
  data3 = dpSrc[3];                                                      \
  u20 = vis_fmul8sux16(row12, yFilter1);                                 \
  v11 = vis_fpadd16(u10, u11);                                           \
  data4 = dpSrc[4];                                                      \
  u21 = vis_fmul8ulx16(row12, yFilter1);                                 \
  row00 = vis_faligndata(data0, data1);                                  \
  u30 = vis_fmul8sux16(row13, yFilter1);                                 \
  row01 = vis_faligndata(data1, data2);                                  \
  u31 = vis_fmul8ulx16(row13, yFilter1);                                 \
  row02 = vis_faligndata(data2, data3);                                  \
  u00 = vis_fmul8sux16(row20, yFilter2);                                 \
  row03 = vis_faligndata(data3, data4);                                  \
  filterposx = (X >> FILTER_SHIFT);                                      \
  sPtr += srcYStride;                                                    \
  v12 = vis_fpadd16(u20, u21);                                           \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u01 = vis_fmul8ulx16(row20, yFilter2);                                 \
  v13 = vis_fpadd16(u30, u31);                                           \
  data0 = dpSrc[0];                                                      \
  u10 = vis_fmul8sux16(row21, yFilter2);                                 \
  X += dX;                                                               \
  data1 = dpSrc[1];                                                      \
  u11 = vis_fmul8ulx16(row21, yFilter2);                                 \
  v20 = vis_fpadd16(u00, u01);                                           \
  data2 = dpSrc[2];                                                      \
  u20 = vis_fmul8sux16(row22, yFilter2);                                 \
  sum0 = vis_fpadd16(v00, v10);                                          \
  data3 = dpSrc[3];                                                      \
  u21 = vis_fmul8ulx16(row22, yFilter2);                                 \
  data4 = dpSrc[4];                                                      \
  row10 = vis_faligndata(data0, data1);                                  \
  u30 = vis_fmul8sux16(row23, yFilter2);                                 \
  row11 = vis_faligndata(data1, data2);                                  \
  u31 = vis_fmul8ulx16(row23, yFilter2);                                 \
  row12 = vis_faligndata(data2, data3);                                  \
  u00 = vis_fmul8sux16(row30, yFilter3);                                 \
  row13 = vis_faligndata(data3, data4);                                  \
  sPtr += srcYStride;                                                    \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  u01 = vis_fmul8ulx16(row30, yFilter3);                                 \
  v21 = vis_fpadd16(u10, u11);                                           \
  Y += dY;                                                               \
  xSrc = (X>>MLIB_SHIFT)-1;                                              \
  sum1 = vis_fpadd16(v01, v11);                                          \
  data0 = dpSrc[0];                                                      \
  u10 = vis_fmul8sux16(row31, yFilter3);                                 \
  sum2 = vis_fpadd16(v02, v12);                                          \
  sum3 = vis_fpadd16(v03, v13);                                          \
  ySrc = (Y>>MLIB_SHIFT)-1;                                              \
  data1 = dpSrc[1];                                                      \
  v22 = vis_fpadd16(u20, u21);                                           \
  u11 = vis_fmul8ulx16(row31, yFilter3);                                 \
  data2 = dpSrc[2];                                                      \
  sum0 = vis_fpadd16(sum0, v20);                                         \
  u20 = vis_fmul8sux16(row32, yFilter3);                                 \
  data3 = dpSrc[3];                                                      \
  u21 = vis_fmul8ulx16(row32, yFilter3);                                 \
  v23 = vis_fpadd16(u30, u31);                                           \
  data4 = dpSrc[4];                                                      \
  v30 = vis_fpadd16(u00, u01);                                           \
  filterposy &= FILTER_MASK;                                             \
  row20 = vis_faligndata(data0, data1);                                  \
  sum1 = vis_fpadd16(sum1, v21);                                         \
  u30 = vis_fmul8sux16(row33, yFilter3);                                 \
  row21 = vis_faligndata(data1, data2);                                  \
  u31 = vis_fmul8ulx16(row33, yFilter3);                                 \
  row22 = vis_faligndata(data2, data3);                                  \
  row23 = vis_faligndata(data3, data4);                                  \
  sPtr += srcYStride;                                                    \
  filterposx &= FILTER_MASK;                                             \
  v31 = vis_fpadd16(u10, u11);                                           \
  dpSrc = vis_alignaddr(sPtr, 0);                                        \
  data0 = dpSrc[0];                                                      \
  sum2 = vis_fpadd16(sum2, v22);                                         \
  sum3 = vis_fpadd16(sum3, v23);                                         \
  data1 = dpSrc[1];                                                      \
  v32 = vis_fpadd16(u20, u21);                                           \
  data2 = dpSrc[2];                                                      \
  sum0 = vis_fpadd16(sum0, v30);                                         \
  data3 = dpSrc[3];                                                      \
  v33 = vis_fpadd16(u30, u31);                                           \
  data4 = dpSrc[4];                                                      \
  row30 = vis_faligndata(data0, data1);                                  \
  v00 = vis_fmul8sux16(sum0, xFilter0);                                  \
  row31 = vis_faligndata(data1, data2);                                  \
  row32 = vis_faligndata(data2, data3);                                  \
  row33 = vis_faligndata(data3, data4);                                  \
  yPtr = ((mlib_d64 *) ((mlib_u8 *)mlib_filters_s16_4+filterposy*4));    \
  sum1 = vis_fpadd16(sum1, v31);                                         \
  yFilter0 = yPtr[0];                                                    \
  sum2 = vis_fpadd16(sum2, v32);                                         \
  v01 = vis_fmul8ulx16(sum0, xFilter0);                                  \
  yFilter1 = yPtr[1];                                                    \
  v10 = vis_fmul8sux16(sum1, xFilter1);                                  \
  sum3 = vis_fpadd16(sum3, v33);                                         \
  yFilter2 = yPtr[2];                                                    \
  v11 = vis_fmul8ulx16(sum1, xFilter1);                                  \
  d0 = vis_fpadd16(v00, v01);                                            \
  yFilter3 = yPtr[3];                                                    \
  xPtr = ((mlib_d64 *)((mlib_u8 *)mlib_filters_s16_4+filterposx*4));     \
  v20 = vis_fmul8sux16(sum2, xFilter2);                                  \
  xFilter0 = xPtr[0];                                                    \
  v21 = vis_fmul8ulx16(sum2, xFilter2);                                  \
  d1 = vis_fpadd16(v10, v11);                                            \
  xFilter1 = xPtr[1];                                                    \
  v30 = vis_fmul8sux16(sum3, xFilter3);                                  \
  v31 = vis_fmul8ulx16(sum3, xFilter3);                                  \
  d2 = vis_fpadd16(v20, v21);                                            \
  xFilter2 = xPtr[2];                                                    \
  d3 = vis_fpadd16(v30, v31);                                            \
  xFilter3 = xPtr[3];                                                    \
  sPtr = (mlib_s16 *)lineAddr[ySrc] + (xSrc<<2);

/************************************************************************/

#define FADD_4BC_S16()                                                   \
  d0 = vis_fpadd16(d0, d1);                                              \
  d2 = vis_fpadd16(d2, d3);                                              \
  d0 = vis_fpadd16(d0, d2);                                              \
  d2 = vis_fmuld8sux16( f_x01000100, vis_read_hi(d0));                   \
  d3 = vis_fmuld8sux16( f_x01000100, vis_read_lo(d0));                   \
  res = vis_fpackfix_pair(d2, d3);                                                               \

/************************************************************************/

void  mlib_v_ImageAffine_s16_4ch_bc (MLIB_AFFINEDEFPARAM2_BC)
{
  DECLAREVAR
  mlib_s32  filterposx, filterposy;
  mlib_d64  data0, data1, data2, data3, data4;
  mlib_d64  sum0, sum1, sum2, sum3;
  mlib_d64  row00, row10, row20, row30;
  mlib_d64  row01, row11, row21, row31;
  mlib_d64  row02, row12, row22, row32;
  mlib_d64  row03, row13, row23, row33;
  mlib_d64  xFilter0, xFilter1, xFilter2, xFilter3;
  mlib_d64  yFilter0, yFilter1, yFilter2, yFilter3;
  mlib_d64  v00, v01, v02, v03, v10, v11, v12, v13;
  mlib_d64  v20, v21, v22, v23, v30, v31, v32, v33;
  mlib_d64  u00, u01, u10, u11, u20, u21,u30, u31;
  mlib_d64  d0, d1, d2, d3;
  mlib_d64 *yPtr, *xPtr;
  mlib_d64 *dp, *dpSrc;
  mlib_s32  cols, i, mask, gsrd;
  mlib_d64  res;
  mlib_f32  f_x01000100 = vis_to_float(0x01000100);
  const union table *mlib_filters_table_4;

  if ( filter == MLIB_BICUBIC ) {
    mlib_filters_table_4 = mlib_filters_s16_bc_4;
  } else {
    mlib_filters_table_4 = mlib_filters_s16_bc2_4;
  }

  srcYStride >>= 1;

  for (j = yStart; j <= yFinish; j++) {

    vis_write_gsr(10 << 3);

    CLIP(4)

    cols = xRight - xLeft + 1;
    dp = vis_alignaddr(dstPixelPtr, 0);
    dstLineEnd += 3;
    mask = vis_edge16(dstPixelPtr, dstLineEnd);
    gsrd = ((8 - (mlib_addr)dstPixelPtr) & 7);

    i = 0;

    if (i <= cols - 4) {

      NEXT_PIXEL_4BC_S16()
      LOAD_BC_S16_4CH_1PIXEL( mlib_filters_table_4 )

      NEXT_PIXEL_4BC_S16()

      BC_S16_4CH( mlib_filters_table_4 )
      FADD_4BC_S16()

      BC_S16_4CH( mlib_filters_table_4 )

#pragma pipeloop(0)

      for (; i < cols-4; i++) {
        vis_alignaddr((void *)gsrd, 0);
        res = vis_faligndata(res, res);

        vis_pst_16(res, dp++, mask);
        vis_pst_16(res, dp, ~mask);

        FADD_4BC_S16()
        BC_S16_4CH( mlib_filters_table_4 )
      }

      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);

      FADD_4BC_S16()
      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);

      RESULT_4BC_S16_1PIXEL()
      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);

      LOAD_BC_S16_4CH_1PIXEL( mlib_filters_table_4 )
      RESULT_4BC_S16_1PIXEL()
      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);
      i += 4;
    }

#pragma pipeloop(0)
    for (; i < cols; i++) {
      NEXT_PIXEL_4BC_S16()
      LOAD_BC_S16_4CH_1PIXEL( mlib_filters_table_4 )
      RESULT_4BC_S16_1PIXEL()
      vis_alignaddr((void *)gsrd, 0);
      res = vis_faligndata(res, res);
      vis_pst_16(res, dp++, mask);
      vis_pst_16(res, dp, ~mask);
    }
  }
}

/***************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s32

/***************************************************************/

#define CREATE_COEF_BICUBIC( X, Y, OPERATOR ) \
  dx = (X & MLIB_MASK) * scale;               \
  dy = (Y & MLIB_MASK) * scale;               \
  dx_2  = 0.5  * dx;    dy_2  = 0.5  * dy;    \
  dx2   = dx   * dx;    dy2   = dy   * dy;    \
  dx3_2 = dx_2 * dx2;   dy3_2 = dy_2 * dy2;   \
  dx3_3 = 3.0  * dx3_2; dy3_3 = 3.0  * dy3_2; \
                                              \
  xf0 = dx2 - dx3_2 - dx_2;                   \
  xf1 = dx3_3 - 2.5 * dx2 + 1.0;              \
  xf2 = 2.0 * dx2 - dx3_3 + dx_2;             \
  xf3 = dx3_2 - 0.5 * dx2;                    \
                                              \
  OPERATOR;                                   \
                                              \
  yf0 = dy2 - dy3_2 - dy_2;                   \
  yf1 = dy3_3 - 2.5 * dy2 + 1.0;              \
  yf2 = 2.0 * dy2 - dy3_3 + dy_2;             \
  yf3 = dy3_2 - 0.5 * dy2;

/***************************************************************/

#define CREATE_COEF_BICUBIC_2( X, Y, OPERATOR ) \
  dx = (X & MLIB_MASK) * scale;                 \
  dy = (Y & MLIB_MASK) * scale;                 \
  dx2   = dx  * dx;    dy2   = dy  * dy;        \
  dx3_2 = dx  * dx2;   dy3_2 = dy  * dy2;       \
  dx3_3 = 2.0 * dx2;   dy3_3 = 2.0 * dy2;       \
                                                \
  xf0 = - dx3_2 + dx3_3 - dx;                   \
  xf1 =   dx3_2 - dx3_3 + 1.0;                  \
  xf2 = - dx3_2 + dx2   + dx;                   \
  xf3 =   dx3_2 - dx2;                          \
                                                \
  OPERATOR;                                     \
                                                \
  yf0 = - dy3_2 + dy3_3 - dy;                   \
  yf1 =   dy3_2 - dy3_3 + 1.0;                  \
  yf2 = - dy3_2 + dy2   + dy;                   \
  yf3 =   dy3_2 - dy2;

/***************************************************************/

void  mlib_v_ImageAffine_s32_1ch_bc (MLIB_AFFINEDEFPARAM2_BC)
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

    xSrc = (X>>MLIB_SHIFT)-1;
    ySrc = (Y>>MLIB_SHIFT)-1;

    sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
    s0 = sPtr[0]; s1 = sPtr[1];
    s2 = sPtr[2]; s3 = sPtr[3];

    sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
    s4 = sPtr[0]; s5 = sPtr[1];
    s6 = sPtr[2]; s7 = sPtr[3];

    if ( filter == MLIB_BICUBIC ) {
      for (; dstPixelPtr <= (dstLineEnd - 1); dstPixelPtr++)
      {
  
        X += dX;
        Y += dY;
  
        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
        sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
        c2 = (sPtr[0]*xf0 + sPtr[1]*xf1 +
              sPtr[2]*xf2 + sPtr[3]*xf3);
        sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
        c3 = (sPtr[0]*xf0 + sPtr[1]*xf1 +
              sPtr[2]*xf2 + sPtr[3]*xf3);
  
        CREATE_COEF_BICUBIC( X, Y, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )
  
        SAT32(dstPixelPtr[0])
  
        xSrc = (X>>MLIB_SHIFT)-1;
        ySrc = (Y>>MLIB_SHIFT)-1;
  
        sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
        s0 = sPtr[0]; s1 = sPtr[1];
        s2 = sPtr[2]; s3 = sPtr[3];
  
        sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
        s4 = sPtr[0]; s5 = sPtr[1];
        s6 = sPtr[2]; s7 = sPtr[3];
      }
     } else {
      for (; dstPixelPtr <= (dstLineEnd - 1); dstPixelPtr++)
      {
  
        X += dX;
        Y += dY;
  
        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
        sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
        c2 = (sPtr[0]*xf0 + sPtr[1]*xf1 +
              sPtr[2]*xf2 + sPtr[3]*xf3);
        sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
        c3 = (sPtr[0]*xf0 + sPtr[1]*xf1 +
              sPtr[2]*xf2 + sPtr[3]*xf3);
  
        CREATE_COEF_BICUBIC_2( X, Y, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )
  
        SAT32(dstPixelPtr[0])
  
        xSrc = (X>>MLIB_SHIFT)-1;
        ySrc = (Y>>MLIB_SHIFT)-1;
  
        sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + xSrc;
        s0 = sPtr[0]; s1 = sPtr[1];
        s2 = sPtr[2]; s3 = sPtr[3];
  
        sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
        s4 = sPtr[0]; s5 = sPtr[1];
        s6 = sPtr[2]; s7 = sPtr[3];
      }
    }

    c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
    c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
    sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
    c2 = (sPtr[0]*xf0 + sPtr[1]*xf1 +
          sPtr[2]*xf2 + sPtr[3]*xf3);
    sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
    c3 = (sPtr[0]*xf0 + sPtr[1]*xf1 +
          sPtr[2]*xf2 + sPtr[3]*xf3);

    val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
    SAT32(dstPixelPtr[0])
  }
}

/***************************************************************/

void  mlib_v_ImageAffine_s32_2ch_bc (MLIB_AFFINEDEFPARAM2_BC)
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

      xSrc = (X1>>MLIB_SHIFT)-1;
      ySrc = (Y1>>MLIB_SHIFT)-1;

      sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
      s0 = sPtr[0]; s1 = sPtr[2];
      s2 = sPtr[4]; s3 = sPtr[6];

      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      s4 = sPtr[0]; s5 = sPtr[2];
      s6 = sPtr[4]; s7 = sPtr[6];

      if ( filter == MLIB_BICUBIC ) {
        for (; dPtr <= (dstLineEnd - 1); dPtr+=2)
        {
          X1 += dX;
          Y1 += dY;
  
          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c2 = (sPtr[0]*xf0 + sPtr[2]*xf1 +
                sPtr[4]*xf2 + sPtr[6]*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c3 = (sPtr[0]*xf0 + sPtr[2]*xf1 +
                sPtr[4]*xf2 + sPtr[6]*xf3);
  
          CREATE_COEF_BICUBIC(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )
  
          SAT32(dPtr[0])
  
          xSrc = (X1>>MLIB_SHIFT)-1;
          ySrc = (Y1>>MLIB_SHIFT)-1;
  
          sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
          s0 = sPtr[0]; s1 = sPtr[2];
          s2 = sPtr[4]; s3 = sPtr[6];
  
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          s4 = sPtr[0]; s5 = sPtr[2];
          s6 = sPtr[4]; s7 = sPtr[6];
        }
      } else {
        for (; dPtr <= (dstLineEnd - 1); dPtr+=2)
        {
          X1 += dX;
          Y1 += dY;
  
          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c2 = (sPtr[0]*xf0 + sPtr[2]*xf1 +
                sPtr[4]*xf2 + sPtr[6]*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c3 = (sPtr[0]*xf0 + sPtr[2]*xf1 +
                sPtr[4]*xf2 + sPtr[6]*xf3);
  
          CREATE_COEF_BICUBIC_2(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )
  
          SAT32(dPtr[0])
  
          xSrc = (X1>>MLIB_SHIFT)-1;
          ySrc = (Y1>>MLIB_SHIFT)-1;
  
          sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
          s0 = sPtr[0]; s1 = sPtr[2];
          s2 = sPtr[4]; s3 = sPtr[6];
  
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          s4 = sPtr[0]; s5 = sPtr[2];
          s6 = sPtr[4]; s7 = sPtr[6];
        }
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      c2 = (sPtr[0]*xf0 + sPtr[2]*xf1 +
            sPtr[4]*xf2 + sPtr[6]*xf3);
      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      c3 = (sPtr[0]*xf0 + sPtr[2]*xf1 +
            sPtr[4]*xf2 + sPtr[6]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
      SAT32(dPtr[0])
    }
  }
}
/*
void  mlib_v_ImageAffine_s32_2ch_bc (MLIB_AFFINEDEFPARAM2_BC)
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

      dx =  (X1 & MLIB_MASK) * scale;  dy = (Y1 & MLIB_MASK) * scale;
      dx_2 = 0.5 * dx;    dy_2 = 0.5 * dy;
      dx2 = dx * dx;      dy2 = dy * dy;
      dx3_2 = dx_2 * dx2; dy3_2 = dy_2 * dy2;
      dx3_3 = 3.0 * dx3_2;  dy3_3 = 3.0 * dy3_2;

      xf0 = dx2 - dx3_2 - dx_2;
      xf1 = dx3_3 - 2.5 * dx2 + 1.0;
      xf2 = 2.0 * dx2 - dx3_3 + dx_2;
      xf3 = dx3_2 - 0.5 * dx2;

      yf0 = dy2 - dy3_2 - dy_2;
      yf1 = dy3_3 - 2.5 * dy2 + 1.0;
      yf2 = 2.0 * dy2 - dy3_3 + dy_2;
      yf3 = dy3_2 - 0.5 * dy2;

      xSrc = (X1>>MLIB_SHIFT)-1;
      ySrc = (Y1>>MLIB_SHIFT)-1;

      sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
      s0 = sPtr[0]; s1 = sPtr[2];
      s2 = sPtr[4]; s3 = sPtr[6];

      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      s4 = sPtr[0]; s5 = sPtr[2];
      s6 = sPtr[4]; s7 = sPtr[6];

      for (; dPtr <= (dstLineEnd - 1); dPtr+=2)
      {

        X1 += dX;
        Y1 += dY;

        c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
        c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
        sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
        c2 = (sPtr[0]*xf0 + sPtr[2]*xf1 +
              sPtr[4]*xf2 + sPtr[6]*xf3);
        sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
        c3 = (sPtr[0]*xf0 + sPtr[2]*xf1 +
              sPtr[4]*xf2 + sPtr[6]*xf3);

        dx =  (X1 & MLIB_MASK) * scale;  dy = (Y1 & MLIB_MASK) * scale;
        dx_2 = 0.5 * dx;    dy_2 = 0.5 * dy;
        dx2 = dx * dx;      dy2 = dy * dy;
        dx3_2 = dx_2 * dx2; dy3_2 = dy_2 * dy2;
        dx3_3 = 3.0 * dx3_2;  dy3_3 = 3.0 * dy3_2;

        xf0 = dx2 - dx3_2 - dx_2;
        xf1 = dx3_3 - 2.5 * dx2 + 1.0;
        xf2 = 2.0 * dx2 - dx3_3 + dx_2;
        xf3 = dx3_2 - 0.5 * dx2;

        val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);

        yf0 = dy2 - dy3_2 - dy_2;
        yf1 = dy3_3 - 2.5 * dy2 + 1.0;
        yf2 = 2.0 * dy2 - dy3_3 + dy_2;
        yf3 = dy3_2 - 0.5 * dy2;

        SAT32(dPtr[0])

        xSrc = (X1>>MLIB_SHIFT)-1;
        ySrc = (Y1>>MLIB_SHIFT)-1;

        sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 2*xSrc + k;
        s0 = sPtr[0]; s1 = sPtr[2];
        s2 = sPtr[4]; s3 = sPtr[6];

        sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
        s4 = sPtr[0]; s5 = sPtr[2];
        s6 = sPtr[4]; s7 = sPtr[6];
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      c2 = (sPtr[0]*xf0 + sPtr[2]*xf1 +
            sPtr[4]*xf2 + sPtr[6]*xf3);
      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      c3 = (sPtr[0]*xf0 + sPtr[2]*xf1 +
            sPtr[4]*xf2 + sPtr[6]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
      SAT32(dPtr[0])
    }
  }
}
*/
/***************************************************************/

void  mlib_v_ImageAffine_s32_3ch_bc (MLIB_AFFINEDEFPARAM2_BC)
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

      xSrc = (X1>>MLIB_SHIFT)-1;
      ySrc = (Y1>>MLIB_SHIFT)-1;

      sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
      s0 = sPtr[0]; s1 = sPtr[3];
      s2 = sPtr[6]; s3 = sPtr[9];

      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      s4 = sPtr[0]; s5 = sPtr[3];
      s6 = sPtr[6]; s7 = sPtr[9];

      if ( filter == MLIB_BICUBIC ) {
        for (; dPtr <= (dstLineEnd - 1); dPtr+=3)
        {
          X1 += dX;
          Y1 += dY;
  
          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c2 = (sPtr[0]*xf0 + sPtr[3]*xf1 +
                sPtr[6]*xf2 + sPtr[9]*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c3 = (sPtr[0]*xf0 + sPtr[3]*xf1 +
                sPtr[6]*xf2 + sPtr[9]*xf3);
  
          CREATE_COEF_BICUBIC(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )
  
          SAT32(dPtr[0])
  
          xSrc = (X1>>MLIB_SHIFT)-1;
          ySrc = (Y1>>MLIB_SHIFT)-1;
  
          sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
          s0 = sPtr[0]; s1 = sPtr[3];
          s2 = sPtr[6]; s3 = sPtr[9];
  
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          s4 = sPtr[0]; s5 = sPtr[3];
          s6 = sPtr[6]; s7 = sPtr[9];
        }
      } else {
        for (; dPtr <= (dstLineEnd - 1); dPtr+=3)
        {
          X1 += dX;
          Y1 += dY;
  
          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c2 = (sPtr[0]*xf0 + sPtr[3]*xf1 +
                sPtr[6]*xf2 + sPtr[9]*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c3 = (sPtr[0]*xf0 + sPtr[3]*xf1 +
                sPtr[6]*xf2 + sPtr[9]*xf3);
  
          CREATE_COEF_BICUBIC_2(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )
  
          SAT32(dPtr[0])
  
          xSrc = (X1>>MLIB_SHIFT)-1;
          ySrc = (Y1>>MLIB_SHIFT)-1;
  
          sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 3*xSrc + k;
          s0 = sPtr[0]; s1 = sPtr[3];
          s2 = sPtr[6]; s3 = sPtr[9];
  
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          s4 = sPtr[0]; s5 = sPtr[3];
          s6 = sPtr[6]; s7 = sPtr[9];
        }
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      c2 = (sPtr[0]*xf0 + sPtr[3]*xf1 +
            sPtr[6]*xf2 + sPtr[9]*xf3);
      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      c3 = (sPtr[0]*xf0 + sPtr[3]*xf1 +
            sPtr[6]*xf2 + sPtr[9]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
      SAT32(dPtr[0])
    }
  }
}

/***************************************************************/

void  mlib_v_ImageAffine_s32_4ch_bc (MLIB_AFFINEDEFPARAM2_BC)
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

      xSrc = (X1>>MLIB_SHIFT)-1;
      ySrc = (Y1>>MLIB_SHIFT)-1;

      sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
      s0 = sPtr[0]; s1 = sPtr[4];
      s2 = sPtr[8]; s3 = sPtr[12];

      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      s4 = sPtr[0]; s5 = sPtr[4];
      s6 = sPtr[8]; s7 = sPtr[12];

      if ( filter == MLIB_BICUBIC ) {
        for (; dPtr <= (dstLineEnd - 1); dPtr+=4)
        {
          X1 += dX;
          Y1 += dY;
  
          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c2 = (sPtr[0]*xf0 + sPtr[4]*xf1 +
                sPtr[8]*xf2 + sPtr[12]*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c3 = (sPtr[0]*xf0 + sPtr[4]*xf1 +
                sPtr[8]*xf2 + sPtr[12]*xf3);
  
          CREATE_COEF_BICUBIC(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )
  
          SAT32(dPtr[0])
  
          xSrc = (X1>>MLIB_SHIFT)-1;
          ySrc = (Y1>>MLIB_SHIFT)-1;
  
          sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
          s0 = sPtr[0]; s1 = sPtr[4];
          s2 = sPtr[8]; s3 = sPtr[12];
  
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          s4 = sPtr[0]; s5 = sPtr[4];
          s6 = sPtr[8]; s7 = sPtr[12];
        }
      } else {
        for (; dPtr <= (dstLineEnd - 1); dPtr+=4)
        {
          X1 += dX;
          Y1 += dY;
  
          c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
          c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c2 = (sPtr[0]*xf0 + sPtr[4]*xf1 +
                sPtr[8]*xf2 + sPtr[12]*xf3);
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          c3 = (sPtr[0]*xf0 + sPtr[4]*xf1 +
                sPtr[8]*xf2 + sPtr[12]*xf3);
  
          CREATE_COEF_BICUBIC_2(  X1, Y1, val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3) )
  
          SAT32(dPtr[0])
  
          xSrc = (X1>>MLIB_SHIFT)-1;
          ySrc = (Y1>>MLIB_SHIFT)-1;
  
          sPtr = ((MLIB_TYPE **)lineAddr)[ySrc] + 4*xSrc + k;
          s0 = sPtr[0]; s1 = sPtr[4];
          s2 = sPtr[8]; s3 = sPtr[12];
  
          sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
          s4 = sPtr[0]; s5 = sPtr[4];
          s6 = sPtr[8]; s7 = sPtr[12];
        }
      }

      c0 = (s0*xf0 + s1*xf1 + s2*xf2 + s3*xf3);
      c1 = (s4*xf0 + s5*xf1 + s6*xf2 + s7*xf3);
      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      c2 = (sPtr[0]*xf0 + sPtr[4]*xf1 +
            sPtr[8]*xf2 + sPtr[12]*xf3);
      sPtr = (MLIB_TYPE*)((mlib_addr)sPtr + srcYStride);
      c3 = (sPtr[0]*xf0 + sPtr[4]*xf1 +
            sPtr[8]*xf2 + sPtr[12]*xf3);

      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);
      SAT32(dPtr[0])
    }
  }
}

/***************************************************************/
