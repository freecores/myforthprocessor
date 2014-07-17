/*
 * @(#)mlib_v_ImageAffine_BL.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident  "@(#)mlib_v_ImageAffine_BL.c	1.10  99/09/03 SMI"

/*
 * FUNCTION
 *      mlib_v_ImageAffine_u8_1ch_bl
 *      mlib_v_ImageAffine_u8_2ch_bl
 *      mlib_v_ImageAffine_u8_3ch_bl
 *      mlib_v_ImageAffine_u8_4ch_bl
 *      mlib_v_ImageAffine_s16_1ch_bl
 *      mlib_v_ImageAffine_s16_2ch_bl
 *      mlib_v_ImageAffine_s16_3ch_bl
 *      mlib_v_ImageAffine_s16_4ch_bl
 *      mlib_v_ImageAffine_s32_1ch_bl
 *      mlib_v_ImageAffine_s32_2ch_bl
 *      mlib_v_ImageAffine_s32_3ch_bl
 *      mlib_v_ImageAffine_s32_4ch_bl
 *        - image affine transformation with Bilinear filtering
 * SYNOPSIS
 *      void mlib_v_ImageAffine_[u8|s16|s32]_?ch_bl
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

#include <stdio.h>
#include <vis_proto.h>
#include "mlib_image.h"
#include "mlib_v_ImageFilters.h"

#ifdef MLIB_OS64BIT
#define MLIB_POINTER_SHIFT(P) (P >> (MLIB_SHIFT-3)) &~ 7
#else
#define MLIB_POINTER_SHIFT(P) (P >> (MLIB_SHIFT-2)) &~ 3
#endif

/***********************************************************************/
#define MLIB_SHIFT   16
#define MLIB_PREC    (1 << MLIB_SHIFT)
#define MLIB_MASK    (MLIB_PREC - 1)
#define FILTER_SHIFT 5
#define FILTER_MASK  (((1 << 8) - 1) << 3)
#define LGSTEPS 8
#define STEPS (1 << LGSTEPS)
#define MLIB_POINTER_GET(A,P) (*(MLIB_TYPE **)((mlib_u8 *)A + P))
/***********************************************************************/
#define MLIB_AFFINEDEFPARAM                                             \
  mlib_s32 *leftEdges,                                                  \
  mlib_s32 *rightEdges,                                                 \
  mlib_s32 *xStarts,                                                    \
  mlib_s32 *yStarts,                                                    \
  mlib_s32 *sides,                                                      \
  mlib_u8  *dstData,                                                    \
  mlib_u8  **lineAddr,                                                  \
  mlib_s32 dstYStride                                                   \

#define MLIB_AFFINEDEFPARAM2 MLIB_AFFINEDEFPARAM, mlib_s32 srcYStride
/***********************************************************************/
#define MLIB_AFFINECALLPARAM                                            \
  leftEdges, rightEdges, xStarts, yStarts,                              \
  sides, dstData, lineAddr, dstYStride

#define MLIB_AFFINECALLPARAM2 MLIB_AFFINECALLPARAM, srcYStride
/***********************************************************************/
#define DECLAREVAR                                                      \
  mlib_s32  xLeft, xRight, X, Y;                                        \
  mlib_s32  yStart = sides[0];                                          \
  mlib_s32  yFinish = sides[1];                                         \
  mlib_s32  dX = sides[2];                                              \
  mlib_s32  dY = sides[3];                                              \
  MLIB_TYPE *srcPixelPtr;                                               \
  MLIB_TYPE *dstPixelPtr;                                               \
  MLIB_TYPE *dstLineEnd;                                                \
  mlib_s32  j;
/***********************************************************************/
#define CLIP(N)                                                         \
  dstData += dstYStride;                                                \
  xLeft = leftEdges[j];                                                 \
  xRight = rightEdges[j];                                               \
  X = xStarts[j];                                                       \
  Y = yStarts[j];                                                       \
  if (xLeft > xRight) continue;                                         \
  dstPixelPtr  = (MLIB_TYPE*)dstData + N * xLeft;                       \
  dstLineEnd   = (MLIB_TYPE*)dstData + N * xRight;
/***********************************************************************/
#undef MLIB_TYPE
#define MLIB_TYPE mlib_u8
/***********************************************************************/
#define FPMIL16(src1, src2)                                             \
  vis_fpadd16(vis_fmul8sux16((src1), (src2)),                           \
              vis_fmul8ulx16((src1), (src2)))
/***********************************************************************/
#define NEXT_PIXELS_1BL_U8(ind)                                         \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr0##ind = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos); \
  srcPixelPtr0##ind += X >> MLIB_SHIFT;                                 \
  X += dX; Y += dY;                                                     \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr1##ind = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos); \
  srcPixelPtr1##ind += X >> MLIB_SHIFT;                                 \
  X += dX; Y += dY;                                                     \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr2##ind = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos); \
  srcPixelPtr2##ind += X >> MLIB_SHIFT;                                 \
  X += dX; Y += dY;                                                     \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr3##ind = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos); \
  srcPixelPtr3##ind += X >> MLIB_SHIFT;                                 \
  X += dX; Y += dY;
/***********************************************************************/
#define NEXT_PIXELS_2BL_U8(ind)                                         \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr0##ind = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos); \
  srcPixelPtr0##ind += (X >> (MLIB_SHIFT-1)) &~ 1;                      \
  X += dX; Y += dY;                                                     \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr1##ind = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos); \
  srcPixelPtr1##ind += (X >> (MLIB_SHIFT-1)) &~ 1;                      \
  X += dX; Y += dY;                                                     \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr2##ind = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos); \
  srcPixelPtr2##ind += (X >> (MLIB_SHIFT-1)) &~ 1;                      \
  X += dX; Y += dY;                                                     \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr3##ind = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos); \
  srcPixelPtr3##ind += (X >> (MLIB_SHIFT-1)) &~ 1;                      \
  X += dX; Y += dY;
/***********************************************************************/
#define LOAD_1BL_U81(accum, ind, a)                                     \
  accum = vis_faligndata(vis_ld_u8_i(srcPixelPtr3##a, ind), accum);     \
  accum = vis_faligndata(vis_ld_u8_i(srcPixelPtr2##a, ind), accum);     \
  accum = vis_faligndata(vis_ld_u8_i(srcPixelPtr1##a, ind), accum);     \
  accum = vis_faligndata(vis_ld_u8_i(srcPixelPtr0##a, ind), accum);
/***********************************************************************/
#define LOAD_1BL_U8(ind)                                                \
  LOAD_1BL_U81(ul##ind, 0, ind)                                         \
  LOAD_1BL_U81(ur##ind, 1, ind)                                         \
  LOAD_1BL_U81(ll##ind, srcYStride, ind)                                \
  LOAD_1BL_U81(lr##ind,srcYStride1, ind)
/***********************************************************************/
#define LOAD_2BL_U8_AL1(accum, ind, a)                                  \
  accum = vis_faligndata(vis_ld_u16_i(srcPixelPtr3##a, ind), accum);    \
  accum = vis_faligndata(vis_ld_u16_i(srcPixelPtr2##a, ind), accum);    \
  accum = vis_faligndata(vis_ld_u16_i(srcPixelPtr1##a, ind), accum);    \
  accum = vis_faligndata(vis_ld_u16_i(srcPixelPtr0##a, ind), accum);
/***********************************************************************/
#define LOAD_2BL_U8_AL(ind)                                             \
  LOAD_2BL_U8_AL1(ul##ind, 0, ind)                                      \
  LOAD_2BL_U8_AL1(ur##ind, 2, ind)                                      \
  LOAD_2BL_U8_AL1(ll##ind, srcYStride, ind)                             \
  LOAD_2BL_U8_AL1(lr##ind,srcYStride1, ind)
/***********************************************************************/
#define LOAD_2BL_U8(accum1, accum2, ind, a)                             \
  ptr = (mlib_d64*)vis_alignaddr(srcPixelPtr0##a, ind);                 \
  hi = ptr[0];                                                          \
  lo = ptr[1];                                                          \
  h1 = vis_faligndata(hi, lo);                                          \
  ptr = (mlib_d64*)vis_alignaddr(srcPixelPtr2##a, ind);                 \
  hi = ptr[0];                                                          \
  lo = ptr[1];                                                          \
  h2 = vis_faligndata(hi, lo);                                          \
  dr = vis_fpmerge(vis_read_hi(h1), vis_read_hi(h2));                   \
  ptr = (mlib_d64*)vis_alignaddr(srcPixelPtr1##a, ind);                 \
  hi = ptr[0];                                                          \
  lo = ptr[1];                                                          \
  h1 = vis_faligndata(hi, lo);                                          \
  ptr = (mlib_d64*)vis_alignaddr(srcPixelPtr3##a, ind);                 \
  hi = ptr[0];                                                          \
  lo = ptr[1];                                                          \
  h2 = vis_faligndata(hi, lo);                                          \
  dr1 = vis_fpmerge(vis_read_hi(h1), vis_read_hi(h2));                  \
  accum1 = vis_fpmerge(vis_read_hi(dr), vis_read_hi(dr1));              \
  accum2 = vis_fpmerge(vis_read_lo(dr), vis_read_lo(dr1));              \
  accum1 = vis_fpmerge(vis_read_hi(accum1), vis_read_lo(accum1));       \
  accum2 = vis_fpmerge(vis_read_hi(accum2), vis_read_lo(accum2));
/***********************************************************************/
#define STORE_2BL_U8()                                                  \
  vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);               \
  res = vis_fpack16_pair(hi8, lo8);                                     \
  res = vis_faligndata(res, res);                                       \
  vis_pst_8(res, dp++, mask);                                           \
  vis_pst_8(res, dp, ~mask);

/***********************************************************************/
#define RESULT_1BL_U8(pos, ind)                                         \
  pos##0 = vis_fmul8x16(vis_read_##pos(ul##ind), delta1_x);             \
  pos##1 = vis_fmul8x16(vis_read_##pos(ur##ind), deltax);               \
  pos##4 = vis_fpadd16(pos##0, pos##1);                                 \
  pos##6 = FPMIL16(pos##4, delta1_y);                                   \
  pos##2 = vis_fmul8x16(vis_read_##pos(ll##ind), delta1_x);             \
  pos##3 = vis_fmul8x16(vis_read_##pos(lr##ind), deltax);               \
  pos##5 = vis_fpadd16(pos##2, pos##3);                                 \
  pos##7 = FPMIL16(pos##5, deltay);                                     \
  pos##8 = vis_fpadd16(pos##6, pos##7);                                 \
  pos##8 = vis_fpadd16(pos##8, k05);                                    \
  deltax = vis_fpadd16(deltax, dx64);                                   \
  deltay = vis_fpadd16(deltay, dy64);                                   \
  deltax = vis_fand(deltax, mask_7fff);                                 \
  deltay = vis_fand(deltay, mask_7fff);                                 \
  delta1_x = vis_fpsub16(mask_7fff,deltax);                             \
  delta1_y = vis_fpsub16(mask_7fff,deltay);
/***********************************************************************/
#define BL_U8_3CH_MAKE(index)                                           \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);       \
  srcPixelPtr += (X >> MLIB_SHIFT)*3;                                   \
  filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xfilter = *((mlib_d64 *) ((mlib_u8 *) mlib_filters_u8_bl+filterpos)); \
  filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yfilter = *((mlib_d64 *) ((mlib_u8 *) mlib_filters_u8_bl + filterpos  \
                                                           + STEPS*8)); \
  filter = vis_fpadd16(vis_fmul8sux16(xfilter, yfilter),                \
                       vis_fmul8ulx16(xfilter, yfilter));               \
  ptr = vis_alignaddr(srcPixelPtr, -1);                                 \
  hi = ptr[0];                                                          \
  lo = ptr[1];                                                          \
  top = vis_faligndata(hi, lo);                                         \
  ptr = vis_alignaddr(srcPixelPtr, srcYStride-1);                       \
  hi = ptr[0];                                                          \
  lo = ptr[1];                                                          \
  bot = vis_faligndata(hi, lo);                                         \
  ulprod = vis_fmul8x16au(vis_read_hi(top), vis_read_hi(filter));       \
  urprod = vis_fmul8x16al(vis_read_lo(top), vis_read_hi(filter));       \
  llprod = vis_fmul8x16au(vis_read_hi(bot), vis_read_lo(filter));       \
  lrprod = vis_fmul8x16al(vis_read_lo(bot), vis_read_lo(filter));       \
  ulprod = vis_fpadd16(ulprod, llprod);                                 \
  urprod = vis_fpadd16(urprod, lrprod);                                 \
  vis_alignaddr((void*)2, 0);                                           \
  ulprod = vis_faligndata(ulprod, ulprod);                              \
  sm##index = vis_fpadd16(urprod, ulprod);                              \
  sm##index = vis_fpadd16(sm##index, k05);                              \
  X += dX; Y += dY;
/***********************************************************************/
#define BL_U8_4CH_MAKE(index)                                           \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);       \
  srcPixelPtr += (X >> (MLIB_SHIFT - 2)) &~ 3;                          \
  filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xfilter = *((mlib_d64 *) ((mlib_u8 *) mlib_filters_u8_bl+filterpos)); \
  filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yfilter = *((mlib_d64 *) ((mlib_u8 *) mlib_filters_u8_bl + filterpos  \
                                                           + STEPS*8)); \
  filter = vis_fpadd16(vis_fmul8sux16(xfilter, yfilter),                \
                       vis_fmul8ulx16(xfilter, yfilter));               \
  ptr = vis_alignaddr(srcPixelPtr, 0);                                  \
  hi = ptr[0];                                                          \
  lo = ptr[1];                                                          \
  top = vis_faligndata(hi, lo);                                         \
  ptr = vis_alignaddr(srcPixelPtr, srcYStride);                         \
  hi = ptr[0];                                                          \
  lo = ptr[1];                                                          \
  bot = vis_faligndata(hi, lo);                                         \
  ulprod = vis_fmul8x16au(vis_read_hi(top), vis_read_hi(filter));       \
  urprod = vis_fmul8x16al(vis_read_lo(top), vis_read_hi(filter));       \
  llprod = vis_fmul8x16au(vis_read_hi(bot), vis_read_lo(filter));       \
  lrprod = vis_fmul8x16al(vis_read_lo(bot), vis_read_lo(filter));       \
  sm##index = vis_fpadd16(ulprod, urprod);                              \
  sm##index = vis_fpadd16(sm##index, llprod);                           \
  sm##index = vis_fpadd16(sm##index, lrprod);                           \
  sm##index = vis_fpadd16(sm##index, k05);                              \
  X += dX; Y += dY;
/***********************************************************************/
#define BL_U8_4CH_MAKE_AL(index)                                        \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);       \
  srcPixelPtr += (X >> (MLIB_SHIFT - 2)) &~ 3;                          \
  filterpos = (X >> FILTER_SHIFT) & FILTER_MASK;                        \
  xfilter = *((mlib_d64 *) ((mlib_u8 *) mlib_filters_u8_bl+filterpos)); \
  filterpos = (Y >> FILTER_SHIFT) & FILTER_MASK;                        \
  yfilter = *((mlib_d64 *) ((mlib_u8 *) mlib_filters_u8_bl + filterpos  \
                                                           + STEPS*8)); \
  filter = vis_fpadd16(vis_fmul8sux16(xfilter, yfilter),                \
                       vis_fmul8ulx16(xfilter, yfilter));               \
  fptr = (mlib_f32*)srcPixelPtr;                                        \
  hi_top = fptr[0];                                                     \
  lo_top = fptr[1];                                                     \
  hi_bot = fptr[srcYStride];                                            \
  lo_bot = fptr[srcYStride+1];                                          \
  ulprod = vis_fmul8x16au(hi_top, vis_read_hi(filter));                 \
  urprod = vis_fmul8x16al(lo_top, vis_read_hi(filter));                 \
  llprod = vis_fmul8x16au(hi_bot, vis_read_lo(filter));                 \
  lrprod = vis_fmul8x16al(lo_bot, vis_read_lo(filter));                 \
  sm##index = vis_fpadd16(ulprod, urprod);                              \
  sm##index = vis_fpadd16(sm##index, llprod);                           \
  sm##index = vis_fpadd16(sm##index, lrprod);                           \
  sm##index = vis_fpadd16(sm##index, k05);                              \
  X += dX; Y += dY;
/***********************************************************************/

void mlib_v_ImageAffine_u8_1ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_s32 srcYStride1 = srcYStride+1;
  MLIB_TYPE *srcPixelPtr00, *srcPixelPtr10, *srcPixelPtr20, *srcPixelPtr30;
  MLIB_TYPE *srcPixelPtr01, *srcPixelPtr11, *srcPixelPtr21, *srcPixelPtr31;
  mlib_d64 dx64, dy64;
  mlib_d64 k05 = vis_to_double_dup(0x00200020);

  dx64 = vis_to_double_dup((((dX << 1) & 0xFFFF) << 16) | ((dX << 1) & 0xFFFF));
  dy64 = vis_to_double_dup((((dY << 1) & 0xFFFF) << 16) | ((dY << 1) & 0xFFFF));


  vis_write_gsr((1 << 3));

  for (j = yStart; j <= yFinish; j++) {
    mlib_d64 dd, *dp;
    mlib_d64 ul1, ur1, ll1, lr1;
    mlib_d64 ul0, ur0, ll0, lr0;
    mlib_s32 cols, offset, times, filterpos;
    mlib_d64 deltax, deltay, delta1_x, delta1_y;
    mlib_d64 hi0, hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8;
    mlib_d64 lo0, lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8;
    mlib_s32 x0, x1, x2, x3, y0, y1, y2, y3, X1, Y1;
    mlib_s32 i, mask;
    mlib_f32 *fp;
    mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);

    CLIP(1)

    cols = xRight - xLeft + 1;
    dp = (mlib_d64*)vis_alignaddr(dstPixelPtr, 0);
    offset = (MLIB_TYPE*)dp - dstPixelPtr;
    x0 = X + dX*offset; y0 = Y + dY*offset;
    x1 = (x0+dX); y1 = (y0+dY); x2 = (x1+dX); y2 = (y1+dY);
    x3 = (x2+dX); y3 = (y2+dY);

    deltax = vis_to_double((((x0 & 0xFFFE) << 15) | ((x1 & 0xFFFE) >> 1)),
                           (((x2 & 0xFFFE) << 15) | ((x3 & 0xFFFE) >> 1)));
    deltay = vis_to_double((((y0 & 0xFFFE) << 15) | ((y1 & 0xFFFE) >> 1)),
                           (((y2 & 0xFFFE) << 15) | ((y3 & 0xFFFE) >> 1)));

    delta1_x = vis_fpsub16(mask_7fff,deltax);
    delta1_y = vis_fpsub16(mask_7fff,deltay);

    mask = vis_edge8(dstPixelPtr, dstLineEnd);
    vis_alignaddr((void*)7, 0);

    offset = (8 + offset) & 7;

    i = 0;

    times = (cols < offset) ? cols : offset;

    if (times > 0) {

      X += (dX * (times-1)); X1 = X;
      Y += (dY * (times-1)); Y1 = Y;

      for (; i < times; i++) {
        filterpos = MLIB_POINTER_SHIFT(Y1);
        srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);
        srcPixelPtr += X1 >> MLIB_SHIFT;
        X1 -= dX; Y1 -= dY;
        ul0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, 0), ul0);
        ur0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, 1), ur0);
        ll0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, srcYStride), ll0);
        lr0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, srcYStride1), lr0);
      }
      X += dX; Y += dY;
      vis_alignaddr((void*)(offset), 0);
      ul0 = vis_faligndata(ul0, ul0);
      ur0 = vis_faligndata(ur0, ur0);
      ll0 = vis_faligndata(ll0, ll0);
      lr0 = vis_faligndata(lr0, lr0);
      vis_alignaddr((void*)7, 0);
      RESULT_1BL_U8(hi, 0)
      RESULT_1BL_U8(lo, 0)
      dd = vis_fpack16_pair(hi8, lo8);
      vis_pst_8(dd, dp++, mask);
    }
    fp = (mlib_f32*)dp;

    if (i <= cols - 16) {
      NEXT_PIXELS_1BL_U8(0)
      LOAD_1BL_U8(0)
      RESULT_1BL_U8(hi, 0)
      NEXT_PIXELS_1BL_U8(0)
      LOAD_1BL_U8(0)
      NEXT_PIXELS_1BL_U8(1)

#pragma pipeloop(0)
      for (; i <= cols - 24; i+= 8) {
        *fp++ = vis_fpack16(hi8);
        RESULT_1BL_U8(hi, 0)
        LOAD_1BL_U8(1)
        NEXT_PIXELS_1BL_U8(0)
        *fp++ = vis_fpack16(hi8);
        RESULT_1BL_U8(hi, 1)
        LOAD_1BL_U8(0)
        NEXT_PIXELS_1BL_U8(1)
      }

      *fp++ = vis_fpack16(hi8);
      RESULT_1BL_U8(hi, 0)
      LOAD_1BL_U8(1)
      NEXT_PIXELS_1BL_U8(0)
      *fp++ = vis_fpack16(hi8);
      RESULT_1BL_U8(hi, 1)
      LOAD_1BL_U8(0)
      *fp++ = vis_fpack16(hi8);
      RESULT_1BL_U8(hi, 0)
      *fp++ = vis_fpack16(hi8);
      i += 16;
    }

    if (i <= cols - 8) {
      NEXT_PIXELS_1BL_U8(0)
      LOAD_1BL_U8(0)
      RESULT_1BL_U8(hi, 0)
      *fp++ = vis_fpack16(hi8);
      NEXT_PIXELS_1BL_U8(0)
      LOAD_1BL_U8(0)
      RESULT_1BL_U8(hi, 0)
      *fp++ = vis_fpack16(hi8);
      i += 8;
    }

    if (i < cols) {
      dp = (mlib_d64*) fp;
      mask = vis_edge8(dp, dstLineEnd);
      X += (dX * (cols - i -1));
      Y += (dY * (cols - i -1));

      for (; i < cols; i++) {
        filterpos = MLIB_POINTER_SHIFT(Y);
        srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);
        srcPixelPtr += X >> MLIB_SHIFT;
        X -= dX; Y -= dY;
        ul0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, 0), ul0);
        ur0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, 1), ur0);
        ll0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, srcYStride), ll0);
        lr0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, srcYStride1), lr0);
      }
      RESULT_1BL_U8(hi, 0)
      RESULT_1BL_U8(lo, 0)
      dd = vis_fpack16_pair(hi8, lo8);
      vis_pst_8(dd, dp++, mask);
    }
  }
}

/***********************************************************************/

void mlib_v_ImageAffine_u8_2ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_s32 srcYStride1 = srcYStride+2;
  MLIB_TYPE *srcPixelPtr00, *srcPixelPtr10, *srcPixelPtr20, *srcPixelPtr30;
  MLIB_TYPE *srcPixelPtr01, *srcPixelPtr11, *srcPixelPtr21, *srcPixelPtr31;
  mlib_d64 dx64, dy64;
  mlib_d64 k05 = vis_to_double_dup(0x00200020);

  dx64 = vis_to_double_dup((((dX) & 0xFFFF) << 16) | ((dX) & 0xFFFF));
  dy64 = vis_to_double_dup((((dY) & 0xFFFF) << 16) | ((dY) & 0xFFFF));

  vis_write_gsr((1 << 3));

  if ((((mlib_addr)lineAddr[0] | (mlib_addr)dstData |
         srcYStride | dstYStride) & 1) == 0) {

    for (j = yStart; j <= yFinish; j++) {
      mlib_d64 dd, *dp;
      mlib_d64 ul1, ur1, ll1, lr1;
      mlib_d64 ul0, ur0, ll0, lr0;
      mlib_s32 cols, offset, times, filterpos;
      mlib_d64 deltax, deltay, delta1_x, delta1_y;
      mlib_d64 hi0, hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8;
      mlib_d64 lo0, lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8;
      mlib_s32 x0, x1, y0, y1, X1, Y1;
      mlib_s32 i, mask;
      mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);

      CLIP(2)

      dstLineEnd += 1;

      cols = xRight - xLeft + 1;
      dp = (mlib_d64*)vis_alignaddr(dstPixelPtr, 0);
      offset = (mlib_s16*)dp - (mlib_s16*)dstPixelPtr;
      x0 = X + dX*offset; y0 = Y + dY*offset;
      x1 = (x0+dX); y1 = (y0+dY);

      deltax = vis_to_double((((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1)),
                             (((x1 & 0xFFFE) << 15) | ((x1 & 0xFFFE) >> 1)));
      deltay = vis_to_double((((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1)),
                             (((y1 & 0xFFFE) << 15) | ((y1 & 0xFFFE) >> 1)));

      delta1_x = vis_fpsub16(mask_7fff,deltax);
      delta1_y = vis_fpsub16(mask_7fff,deltay);

      mask = vis_edge8(dstPixelPtr, dstLineEnd);
      vis_alignaddr((void*)6, 0);

      offset = (4 + offset) & 3;

      i = 0;

      times = (cols < offset) ? cols : offset;

      if (times > 0) {

        X += (dX * (times-1)); X1 = X;
        Y += (dY * (times-1)); Y1 = Y;

        for (; i < times; i++) {
          filterpos = MLIB_POINTER_SHIFT(Y1);
          srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);
          srcPixelPtr += (X1 >> (MLIB_SHIFT-1)) &~ 1;
          X1 -= dX; Y1 -= dY;
          ul0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 0), ul0);
          ur0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 2), ur0);
          ll0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride), ll0);
          lr0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride1), lr0);
        }
        X += dX; Y += dY;
        vis_alignaddr((void*)(2*offset), 0);
        ul0 = vis_faligndata(ul0, ul0);
        ur0 = vis_faligndata(ur0, ur0);
        ll0 = vis_faligndata(ll0, ll0);
        lr0 = vis_faligndata(lr0, lr0);
        vis_alignaddr((void*)6, 0);
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        dd = vis_fpack16_pair(hi8, lo8);
        vis_pst_8(dd, dp++, mask);
      }

      if (i <= cols - 12) {
        NEXT_PIXELS_2BL_U8(0)
        LOAD_2BL_U8_AL(0)
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        NEXT_PIXELS_2BL_U8(0)
        LOAD_2BL_U8_AL(0)
        NEXT_PIXELS_2BL_U8(1)

#pragma pipeloop(0)
        for (; i <= cols - 20; i+= 8) {
          *dp++ = vis_fpack16_pair(hi8, lo8);
          RESULT_1BL_U8(hi, 0)
          LOAD_2BL_U8_AL(1)
          RESULT_1BL_U8(lo, 0)
          NEXT_PIXELS_2BL_U8(0)
          *dp++ = vis_fpack16_pair(hi8, lo8);
          RESULT_1BL_U8(hi, 1)
          LOAD_2BL_U8_AL(0)
          RESULT_1BL_U8(lo, 1)
          NEXT_PIXELS_2BL_U8(1)
        }

        *dp++ = vis_fpack16_pair(hi8, lo8);
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        LOAD_2BL_U8_AL(1)
        *dp++ = vis_fpack16_pair(hi8, lo8);
        RESULT_1BL_U8(hi, 1)
        RESULT_1BL_U8(lo, 1)
        *dp++ = vis_fpack16_pair(hi8, lo8);
        i += 12;
      }

      if (i <= cols - 8) {
        NEXT_PIXELS_2BL_U8(0)
        LOAD_2BL_U8_AL(0)
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        *dp++ = vis_fpack16_pair(hi8, lo8);
        NEXT_PIXELS_2BL_U8(0)
        LOAD_2BL_U8_AL(0)
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        *dp++ = vis_fpack16_pair(hi8, lo8);
        i += 8;
      }

      if (i <= cols - 4) {
        NEXT_PIXELS_2BL_U8(0)
        LOAD_2BL_U8_AL(0)
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        *dp++ = vis_fpack16_pair(hi8, lo8);
        i += 4;
      }

      if (i < cols) {
        mask = vis_edge8(dp, dstLineEnd);
        X += (dX * (cols - i -1));
        Y += (dY * (cols - i -1));

        for (; i < cols; i++) {
          filterpos = MLIB_POINTER_SHIFT(Y);
          srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);
          srcPixelPtr += (X >> (MLIB_SHIFT-1)) &~ 1;
          X -= dX; Y -= dY;
          ul0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 0), ul0);
          ur0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 2), ur0);
          ll0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride), ll0);
          lr0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride1), lr0);
        }
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        dd = vis_fpack16_pair(hi8, lo8);
        vis_pst_8(dd, dp++, mask);
      }
    }
  } else {

    for (j = yStart; j <= yFinish; j++) {
      mlib_d64 *dp;
      mlib_d64 ul1, ur1, ll1, lr1;
      mlib_d64 ul0, ur0, ll0, lr0;
      mlib_s32 cols, filterpos;
      mlib_d64 deltax, deltay, delta1_x, delta1_y;
      mlib_d64 hi0, hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8;
      mlib_d64 lo0, lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8;
      mlib_s32 x0, x1, y0, y1;
      mlib_s32 i, mask, emask;
      mlib_d64 *ptr;
      mlib_d64 hi, h1, h2, lo, dr, dr1, res;
      mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);

      CLIP(2)

      dstLineEnd += 1;

      cols = xRight - xLeft + 1;
      dp = (mlib_d64*)vis_alignaddr(dstPixelPtr, 0);
      x0 = X; y0 = Y;
      x1 = (x0+dX); y1 = (y0+dY);

      deltax = vis_to_double((((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1)),
                             (((x1 & 0xFFFE) << 15) | ((x1 & 0xFFFE) >> 1)));
      deltay = vis_to_double((((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1)),
                             (((y1 & 0xFFFE) << 15) | ((y1 & 0xFFFE) >> 1)));

      delta1_x = vis_fpsub16(mask_7fff,deltax);
      delta1_y = vis_fpsub16(mask_7fff,deltay);

      mask = vis_edge8(dstPixelPtr, dstLineEnd);

      i = 0;

      if (i <= cols - 12) {
        NEXT_PIXELS_2BL_U8(0)
        LOAD_2BL_U8(ul0, ur0, 0, 0)
        LOAD_2BL_U8(ll0, lr0, srcYStride, 0)
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        NEXT_PIXELS_2BL_U8(0)
        LOAD_2BL_U8(ul0, ur0, 0, 0)
        LOAD_2BL_U8(ll0, lr0, srcYStride, 0)
        NEXT_PIXELS_2BL_U8(1)

#pragma pipeloop(0)
        for (; i <= cols - 20; i+= 8) {
          STORE_2BL_U8()
          RESULT_1BL_U8(hi, 0)
          LOAD_2BL_U8(ul1, ur1, 0, 1)
          LOAD_2BL_U8(ll1, lr1, srcYStride, 1)
          RESULT_1BL_U8(lo, 0)
          NEXT_PIXELS_2BL_U8(0)
          STORE_2BL_U8()
          RESULT_1BL_U8(hi, 1)
          LOAD_2BL_U8(ul0, ur0, 0, 0)
          LOAD_2BL_U8(ll0, lr0, srcYStride, 0)
          RESULT_1BL_U8(lo, 1)
          NEXT_PIXELS_2BL_U8(1)
        }

        STORE_2BL_U8()
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        LOAD_2BL_U8(ul1, ur1, 0, 1)
        LOAD_2BL_U8(ll1, lr1, srcYStride, 1)
        STORE_2BL_U8()
        RESULT_1BL_U8(hi, 1)
        RESULT_1BL_U8(lo, 1)
        STORE_2BL_U8()
        i += 12;
      }

      if (i <= cols - 8) {
        NEXT_PIXELS_2BL_U8(0)
        LOAD_2BL_U8(ul0, ur0, 0, 0)
        LOAD_2BL_U8(ll0, lr0, srcYStride, 0)
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        STORE_2BL_U8()
        NEXT_PIXELS_2BL_U8(0)
        LOAD_2BL_U8(ul0, ur0, 0, 0)
        LOAD_2BL_U8(ll0, lr0, srcYStride, 0)
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        STORE_2BL_U8()
        i += 8;
      }

      if (i <= cols - 4) {
        NEXT_PIXELS_2BL_U8(0)
        LOAD_2BL_U8(ul0, ur0, 0, 0)
        LOAD_2BL_U8(ll0, lr0, srcYStride, 0)
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        STORE_2BL_U8()
        i += 4;
      }

      if (i < cols) {
        vis_alignaddr((void*)7, 0);
        X += (dX * (cols - i -1));
        Y += (dY * (cols - i -1));

        for (; i < cols; i++) {
          filterpos = MLIB_POINTER_SHIFT(Y);
          srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);
          srcPixelPtr += (X >> (MLIB_SHIFT-1)) &~ 1;
          X -= dX; Y -= dY;
          ul0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, 1), ul0);
          ul0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, 0), ul0);
          ur0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, 3), ur0);
          ur0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, 2), ur0);
          ll0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, srcYStride+1), ll0);
          ll0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, srcYStride), ll0);
          lr0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, srcYStride1+1), lr0);
          lr0 = vis_faligndata(vis_ld_u8_i(srcPixelPtr, srcYStride1), lr0);
        }
        RESULT_1BL_U8(hi, 0)
        RESULT_1BL_U8(lo, 0)
        vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
        res = vis_fpack16_pair(hi8, lo8);
        res = vis_faligndata(res, res);
        emask = vis_edge8(dp, dstLineEnd);
        emask &= mask;
        vis_pst_8(res, dp++, emask);
        if ((mlib_u8*)dp <= (mlib_u8*)dstLineEnd) {
          emask = vis_edge8(dp, dstLineEnd);
          vis_pst_8(res, dp++, emask);
        }
      }
    }
  }
}

/***********************************************************************/

void mlib_v_ImageAffine_u8_3ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 k05 = vis_to_double_dup(0x00080008);

  vis_write_gsr(3 << 3);


    for (j = yStart; j <= yFinish; j++) {
      mlib_s32 col, cols, times;
      mlib_s32 filterpos;
      mlib_d64 filter, xfilter, yfilter;
      mlib_d64 sm0, sm1;
      mlib_d64 hi, lo, top, bot, *ptr;
      mlib_d64 ulprod, urprod, llprod, lrprod;
      union {
        mlib_u8 t[8];
        mlib_d64 f;
      } f0;

      CLIP(3)

      cols = xRight - xLeft+1;

      times = cols >> 1;

      if (times > 0) {
        BL_U8_3CH_MAKE(0)
        BL_U8_3CH_MAKE(1)

#pragma pipeloop(0)
        for(col = 1; col < times; col++) {
          f0.f = vis_fpack16_pair(sm0,sm1);
          BL_U8_3CH_MAKE(0)
          BL_U8_3CH_MAKE(1)
          dstPixelPtr[0] = f0.t[0];
          dstPixelPtr[1] = f0.t[1];
          dstPixelPtr[2] = f0.t[2];
          dstPixelPtr[3] = f0.t[4];
          dstPixelPtr[4] = f0.t[5];
          dstPixelPtr[5] = f0.t[6];
          dstPixelPtr += 6;
        }
        f0.f = vis_fpack16_pair(sm0,sm1);
        dstPixelPtr[0] = f0.t[0];
        dstPixelPtr[1] = f0.t[1];
        dstPixelPtr[2] = f0.t[2];
        dstPixelPtr[3] = f0.t[4];
        dstPixelPtr[4] = f0.t[5];
        dstPixelPtr[5] = f0.t[6];
        dstPixelPtr += 6;
      }
      cols -= times << 1;

      if (cols == 1) {
        BL_U8_3CH_MAKE(0)
        f0.f = vis_fpack16_pair(sm0,sm1);
        dstPixelPtr[0] = f0.t[0];
        dstPixelPtr[1] = f0.t[1];
        dstPixelPtr[2] = f0.t[2];
      }
    }
}



/***********************************************************************/

void mlib_v_ImageAffine_u8_4ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 k05 = vis_to_double_dup(0x00080008);

  vis_write_gsr(3 << 3);

  if ((((mlib_addr)lineAddr[0] | srcYStride) & 3) == 0) {

    srcYStride >>=2;

    for (j = yStart; j <= yFinish; j++) {
      mlib_d64 *dpDst;
      mlib_s32 col, cols, times, emask, mask;
      mlib_s32 filterpos;
      mlib_d64 accum, filter, xfilter, yfilter;
      mlib_d64 sm0, sm1;
      mlib_f32 hi_top, lo_top, hi_bot, lo_bot, *fptr;
      mlib_d64 ulprod, urprod, llprod, lrprod;

      CLIP(4)

      cols = xRight - xLeft+1;

      dstLineEnd += 3;
      dpDst = (mlib_d64*)vis_alignaddr(dstPixelPtr, 0);

      emask = vis_edge8(dstPixelPtr, dstLineEnd);
      times = cols >> 1;
      vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);

      if (times > 0) {
        BL_U8_4CH_MAKE_AL(0)
        BL_U8_4CH_MAKE_AL(1)

#pragma pipeloop(0)
        for(col = 1; col < times; col++) {
          accum = vis_fpack16_pair(sm0,sm1);
          accum = vis_faligndata(accum, accum);
          BL_U8_4CH_MAKE_AL(0)
          BL_U8_4CH_MAKE_AL(1)
          vis_pst_8(accum, dpDst++, emask);
          vis_pst_8(accum, dpDst, ~emask);
        }
        accum = vis_fpack16_pair(sm0,sm1);
        accum = vis_faligndata(accum, accum);
        vis_pst_8(accum, dpDst++, emask);
        vis_pst_8(accum, dpDst, ~emask);
      }
      cols -= times << 1;

      if (cols == 1) {
        BL_U8_4CH_MAKE_AL(0)
        accum = vis_fpack16_to_hi(accum, sm0);
        accum = vis_faligndata(accum, accum);
        mask = vis_edge8(dpDst, dstLineEnd);
        vis_pst_8(accum, dpDst++, emask & mask);
        if ((mlib_u8*)dpDst <= (mlib_u8*)dstLineEnd) {
          mask = vis_edge8(dpDst, dstLineEnd);
          vis_pst_8(accum, dpDst, mask);
        }
      }
    }
  } else {

    for (j = yStart; j <= yFinish; j++) {
      mlib_d64 *dpDst;
      mlib_s32 col, cols, times, emask, mask;
      mlib_s32 filterpos;
      mlib_d64 accum, filter, xfilter, yfilter;
      mlib_d64 sm0, sm1;
      mlib_d64 hi, lo, top, bot, *ptr;
      mlib_d64 ulprod, urprod, llprod, lrprod;

      CLIP(4)

      cols = xRight - xLeft+1;

      dstLineEnd += 3;
      dpDst = (mlib_d64*)vis_alignaddr(dstPixelPtr, 0);

      emask = vis_edge8(dstPixelPtr, dstLineEnd);
      times = cols >> 1;

      if (times > 0) {
        BL_U8_4CH_MAKE(0)
        BL_U8_4CH_MAKE(1)

#pragma pipeloop(0)
        for(col = 1; col < times; col++) {
          accum = vis_fpack16_pair(sm0,sm1);
          vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
          accum = vis_faligndata(accum, accum);
          BL_U8_4CH_MAKE(0)
          BL_U8_4CH_MAKE(1)
          vis_pst_8(accum, dpDst++, emask);
          vis_pst_8(accum, dpDst, ~emask);
        }
        accum = vis_fpack16_pair(sm0,sm1);
        vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
        accum = vis_faligndata(accum, accum);
        vis_pst_8(accum, dpDst++, emask);
        vis_pst_8(accum, dpDst, ~emask);
      }
      cols -= times << 1;

      if (cols == 1) {
        BL_U8_4CH_MAKE(0)
        accum = vis_fpack16_to_hi(accum, sm0);
        vis_alignaddr((void *)(8 - (mlib_addr)dstPixelPtr), 0);
        accum = vis_faligndata(accum, accum);
        mask = vis_edge8(dpDst, dstLineEnd);
        vis_pst_8(accum, dpDst++, emask & mask);
        if ((mlib_u8*)dpDst <= (mlib_u8*)dstLineEnd) {
          mask = vis_edge8(dpDst, dstLineEnd);
          vis_pst_8(accum, dpDst, mask);
        }
      }
    }
  }
}

/***********************************************************************/

#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s16

/***********************************************************************/
#define NEXT_PIXELS_2BL_S16()                                           \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr00 = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);     \
  srcPixelPtr00 += (X >> (MLIB_SHIFT-1)) &~ 1;                          \
  X += dX; Y += dY;                                                     \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr10 = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);     \
  srcPixelPtr10 += (X >> (MLIB_SHIFT-1)) &~ 1;                          \
  X += dX; Y += dY;
/***********************************************************************/
#define NEXT_PIXELS_3BL_S16()                                           \
  xSrc = X >> MLIB_SHIFT;                                               \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);       \
  srcPixelPtr += ((xSrc<<1)+xSrc);                                      \
  X += dX; Y += dY;
/***********************************************************************/
#define NEXT_PIXELS_4BL_S16()                                           \
  filterpos = MLIB_POINTER_SHIFT(Y);                                    \
  srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);       \
  srcPixelPtr += (X >> (MLIB_SHIFT-2)) &~ 3;                            \
  X += dX; Y += dY;
/***********************************************************************/
#define LOAD_2BL_S16_AL()                                               \
  ptr1 = (mlib_f32*)srcPixelPtr00;                                      \
  ptr2 = (mlib_f32*)srcPixelPtr10;                                      \
  ul0 = vis_freg_pair(ptr1[0], ptr2[0]);                                \
  ur0 = vis_freg_pair(ptr1[1], ptr2[1]);                                \
  ll0 = vis_freg_pair(ptr1[srcYStride1], ptr2[srcYStride1]);            \
  lr0 = vis_freg_pair(ptr1[srcYStride1+1], ptr2[srcYStride1+1]);
/***********************************************************************/
#define LOAD_2BL_S16()                                                  \
  ul0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr10, 2), ul0);            \
  ul0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr10, 0), ul0);            \
  ul0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr00, 2), ul0);            \
  ul0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr00, 0), ul0);            \
  ur0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr10, 6), ur0);            \
  ur0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr10, 4), ur0);            \
  ur0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr00, 6), ur0);            \
  ur0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr00, 4), ur0);            \
  ll0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr10, srcYStride+2), ul0); \
  ll0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr10, srcYStride), ll0);   \
  ll0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr00, srcYStride+2), ll0); \
  ll0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr00, srcYStride), ll0);   \
  lr0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr10, srcYStride+6), lr0); \
  lr0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr10, srcYStride+4), lr0); \
  lr0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr00, srcYStride+6), lr0); \
  lr0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr00, srcYStride+4), lr0);
/***********************************************************************/
#define LOAD_3BL_S16()                                                  \
  psr = vis_alignaddr((void *)srcPixelPtr, 0);                          \
  a0 = psr[0]; a1 = psr[1]; a2 = psr[2];                                \
  ul0 = vis_faligndata(a0, a1);                                         \
  ur0 = vis_faligndata(a1, a2);                                         \
  vis_alignaddr((void *)6, 0);                                          \
  ur0 = vis_faligndata(ul0, ur0);                                       \
                                                                        \
  psr = vis_alignaddr((void *)srcPixelPtr, srcYStride);                 \
  a0 = psr[0]; a1 = psr[1]; a2 = psr[2];                                \
  ll0 = vis_faligndata(a0, a1);                                         \
  lr0 = vis_faligndata(a1, a2);                                         \
  vis_alignaddr((void *)6, 0);                                          \
  lr0 = vis_faligndata(ll0, lr0);
/***********************************************************************/
#define LOAD_4BL_S16()                                                  \
  psr = vis_alignaddr((void *)srcPixelPtr, 0);                          \
  a0 = psr[0]; a1 = psr[1]; a2 = psr[2];                                \
  ul0 = vis_faligndata(a0, a1);                                         \
  ur0 = vis_faligndata(a1, a2);                                         \
                                                                        \
  psr = vis_alignaddr((void *)srcPixelPtr, srcYStride);                 \
  a0 = psr[0]; a1 = psr[1]; a2 = psr[2];                                \
  ll0 = vis_faligndata(a0, a1);                                         \
  lr0 = vis_faligndata(a1, a2);
/***********************************************************************/
#define LOAD_4BL_S16_AL()                                               \
  psr = (mlib_d64*)srcPixelPtr;                                         \
  ul0 = psr[0];                                                         \
  ur0 = psr[1];                                                         \
  ll0 = psr[srcYStride1];                                               \
  lr0 = psr[srcYStride1+1];
/***********************************************************************/
#define STORE_3BL_S16()                                                 \
  vis_alignaddr((void *)gsrd, 0);                                       \
  dstPixelPtr += 3;                                                     \
  hi8 = vis_faligndata(hi8, hi8);                                       \
  vis_pst_8(hi8, dp, mask);                                             \
  vis_pst_8(hi8, dp + 1, rmask);                                        \
                                                                        \
  mask = 0xfc >> ((mlib_addr)dstPixelPtr & 7);                          \
  rmask = (~mask << 2) & 0xf0;                                          \
  dp = (mlib_d64 *)((mlib_addr)dstPixelPtr & ~7);                       \
  gsrd = ((8 - (mlib_addr)dstPixelPtr) & 7);
/***********************************************************************/
#define RESULT_1BL_S16(pos)                                             \
  ul0 = FPMIL16(ul0, pos##_delta1_x);                                   \
  ur0 = FPMIL16(ur0, pos##_deltax);                                     \
  ul0 = vis_fpadd16(ul0, ur0);                                          \
  ul0 = vis_fpadd16(ul0, ul0);                                          \
  ul0 = FPMIL16(ul0, pos##_delta1_y);                                   \
  ll0 = FPMIL16(ll0, pos##_delta1_x);                                   \
  lr0 = FPMIL16(lr0, pos##_deltax);                                     \
  ll0 = vis_fpadd16(ll0, lr0);                                          \
  ll0 = vis_fpadd16(ll0, ll0);                                          \
  ll0 = FPMIL16(ll0, pos##_deltay);                                     \
  hi8 = vis_fpadd16(ll0, ul0);                                          \
  hi8 = vis_fpadd16(hi8, hi8);                                          \
  pos##_deltax = vis_fpadd16(pos##_deltax, dx64);                       \
  pos##_deltay = vis_fpadd16(pos##_deltay, dy64);                       \
  pos##_deltax = vis_fand(pos##_deltax, mask_7fff);                     \
  pos##_deltay = vis_fand(pos##_deltay, mask_7fff);                     \
  pos##_delta1_x = vis_fpsub16(mask_7ffe,pos##_deltax);                 \
  pos##_delta1_y = vis_fpsub16(mask_7ffe,pos##_deltay);
/***********************************************************************/

void mlib_v_ImageAffine_s16_1ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_s32 srcYStride1 = srcYStride+2;
  MLIB_TYPE *srcPixelPtr00, *srcPixelPtr10, *srcPixelPtr20, *srcPixelPtr30;
  mlib_d64 dx64, dy64;

  dx64 = vis_to_double_dup((((dX << 1) & 0xFFFF) << 16) | ((dX << 1) & 0xFFFF));
  dy64 = vis_to_double_dup((((dY << 1) & 0xFFFF) << 16) | ((dY << 1) & 0xFFFF));

  for (j = yStart; j <= yFinish; j++) {
    mlib_d64 *dp;
    mlib_d64 ul0, ur0, ll0, lr0;
    mlib_s32 cols, offset, times, filterpos;
    mlib_d64 hi_deltax, hi_deltay, hi_delta1_x, hi_delta1_y;
    mlib_d64 hi8;
    mlib_s32 x0, x1, x2, x3, y0, y1, y2, y3, X1, Y1;
    mlib_s32 i, mask;
    mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);
    mlib_d64 mask_7ffe = vis_to_double_dup(0x7FFE7FFE);

    CLIP(1)

    cols = xRight - xLeft + 1;
    dp = (mlib_d64*)vis_alignaddr(dstPixelPtr, 0);
    offset = (mlib_s16*)dp - (mlib_s16*)dstPixelPtr;
    x0 = X + dX*offset; y0 = Y + dY*offset;
    x1 = (x0+dX); y1 = (y0+dY);
    x2 = (x1+dX); y2 = (y1+dY);
    x3 = (x2+dX); y3 = (y2+dY);

    hi_deltax = vis_to_double((((x0 & 0xFFFE) << 15) | ((x1 & 0xFFFE) >> 1)),
                              (((x2 & 0xFFFE) << 15) | ((x3 & 0xFFFE) >> 1)));
    hi_deltay = vis_to_double((((y0 & 0xFFFE) << 15) | ((y1 & 0xFFFE) >> 1)),
                              (((y2 & 0xFFFE) << 15) | ((y3 & 0xFFFE) >> 1)));

    hi_delta1_x = vis_fpsub16(mask_7ffe, hi_deltax);
    hi_delta1_y = vis_fpsub16(mask_7ffe, hi_deltay);

    mask = vis_edge16(dstPixelPtr, dstLineEnd);
    vis_alignaddr((void*)6, 0);

    offset = (4 + offset) & 3;

    i = 0;

    times = (cols < offset) ? cols : offset;

    if (times > 0) {

      X += (dX * (times-1)); X1 = X;
      Y += (dY * (times-1)); Y1 = Y;

      for (; i < times; i++) {
        filterpos = MLIB_POINTER_SHIFT(Y1);
        srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);
        srcPixelPtr += (X1 >> MLIB_SHIFT);
        X1 -= dX; Y1 -= dY;
        ul0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 0), ul0);
        ur0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 2), ur0);
        ll0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride), ll0);
        lr0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride1), lr0);
      }
      X += dX; Y += dY;
      vis_alignaddr((void*)(2*offset), 0);
      ul0 = vis_faligndata(ul0, ul0);
      ur0 = vis_faligndata(ur0, ur0);
      ll0 = vis_faligndata(ll0, ll0);
      lr0 = vis_faligndata(lr0, lr0);
      vis_alignaddr((void*)6, 0);
      RESULT_1BL_S16(hi)
      vis_pst_16(hi8, dp++, mask);
    }

    if (i <= cols - 4) {
      NEXT_PIXELS_1BL_U8(0)

#pragma pipeloop(0)
      for (; i <= cols - 8; i+= 4) {
        LOAD_2BL_U8_AL(0)
        NEXT_PIXELS_1BL_U8(0)
        RESULT_1BL_S16(hi)
        *dp++ = hi8;
      }
      LOAD_2BL_U8_AL(0)
      RESULT_1BL_S16(hi)
      *dp++ = hi8;
      i += 4;
    }

    if (i < cols) {
      mask = vis_edge16(dp, dstLineEnd);
      X += (dX * (cols - i -1));
      Y += (dY * (cols - i -1));

      for (; i < cols; i++) {
        filterpos = MLIB_POINTER_SHIFT(Y);
        srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);
        srcPixelPtr += (X >> MLIB_SHIFT);
        X -= dX; Y -= dY;
        ul0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 0), ul0);
        ur0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 2), ur0);
        ll0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride), ll0);
        lr0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride1), lr0);
      }
      RESULT_1BL_S16(hi)
      vis_pst_16(hi8, dp++, mask);
    }
  }
}

/***********************************************************************/

void mlib_v_ImageAffine_s16_2ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_s32 srcYStride1 = (srcYStride>>2);
  MLIB_TYPE *srcPixelPtr00, *srcPixelPtr10;
  mlib_d64 dx64, dy64;

  dx64 = vis_to_double_dup((((dX) & 0xFFFF) << 16) | ((dX) & 0xFFFF));
  dy64 = vis_to_double_dup((((dY) & 0xFFFF) << 16) | ((dY) & 0xFFFF));

  if ((((mlib_addr)lineAddr[0] | srcYStride) & 3) == 0) {

    for (j = yStart; j <= yFinish; j++) {
      mlib_d64 *dp;
      mlib_d64 ul0, ur0, ll0, lr0;
      mlib_s32 cols, filterpos;
      mlib_d64 hi_deltax, hi_deltay, hi_delta1_x, hi_delta1_y;
      mlib_d64 hi8;
      mlib_s32 x0, x1, y0, y1;
      mlib_s32 i, mask, emask, gsrd;
      mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);
      mlib_d64 mask_7ffe = vis_to_double_dup(0x7FFE7FFE);
      mlib_f32 *ptr1, *ptr2;

      CLIP(2)

      cols = xRight - xLeft + 1;
      dp = (mlib_d64*)vis_alignaddr(dstPixelPtr, 0);
      x0 = X; y0 = Y;
      x1 = (x0+dX); y1 = (y0+dY);

      hi_deltax = vis_to_double((((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1)),
                                (((x1 & 0xFFFE) << 15) | ((x1 & 0xFFFE) >> 1)));
      hi_deltay = vis_to_double((((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1)),
                                (((y1 & 0xFFFE) << 15) | ((y1 & 0xFFFE) >> 1)));

      hi_delta1_x = vis_fpsub16(mask_7ffe, hi_deltax);
      hi_delta1_y = vis_fpsub16(mask_7ffe, hi_deltay);

      dstLineEnd += 1;

      mask = vis_edge16(dstPixelPtr, dstLineEnd);
      gsrd = ((8 - (mlib_addr)dstPixelPtr) & 7);
      vis_alignaddr((void*)gsrd, 0);

      i = 0;

      if (i <= cols - 2) {
        NEXT_PIXELS_2BL_S16()

#pragma pipeloop(0)
        for (; i <= cols - 4; i+= 2) {
          LOAD_2BL_S16_AL()
          NEXT_PIXELS_2BL_S16()
          RESULT_1BL_S16(hi)
          hi8 = vis_faligndata(hi8, hi8);
          vis_pst_16(hi8, dp++, mask);
          vis_pst_16(hi8, dp, ~mask);
        }
        LOAD_2BL_S16_AL()
        RESULT_1BL_S16(hi)
        hi8 = vis_faligndata(hi8, hi8);
        vis_pst_16(hi8, dp++, mask);
        vis_pst_16(hi8, dp, ~mask);
        i += 2;
      }

      if (i < cols) {
        filterpos = MLIB_POINTER_SHIFT(Y);
        srcPixelPtr00 = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);
        srcPixelPtr00 += (X >> (MLIB_SHIFT-1)) &~ 1;

        ptr1 = (mlib_f32*)srcPixelPtr00;
        ul0 = vis_write_hi(ul0, ptr1[0]);
        ur0 = vis_write_hi(ur0, ptr1[1]);
        ll0 = vis_write_hi(ll0, ptr1[srcYStride1]);
        lr0 = vis_write_hi(lr0, ptr1[srcYStride1+1]);
        RESULT_1BL_S16(hi)
        hi8 = vis_faligndata(hi8, hi8);
        emask = vis_edge16(dp, dstLineEnd);
        vis_pst_16(hi8, dp++, mask & emask);
        if ((mlib_u8*)dp <= (mlib_u8*)dstLineEnd) {
          emask = vis_edge16(dp, dstLineEnd);
          vis_pst_16(hi8, dp, emask);
        }
      }
    }
  } else {
    for (j = yStart; j <= yFinish; j++) {
      mlib_d64 *dp;
      mlib_d64 ul0, ur0, ll0, lr0;
      mlib_s32 cols, filterpos;
      mlib_d64 hi_deltax, hi_deltay, hi_delta1_x, hi_delta1_y;
      mlib_d64 hi8;
      mlib_s32 x0, x1, y0, y1;
      mlib_s32 i, mask, emask, gsrd;
      mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);
      mlib_d64 mask_7ffe = vis_to_double_dup(0x7FFE7FFE);

      CLIP(2)

      cols = xRight - xLeft + 1;
      dp = (mlib_d64*)vis_alignaddr(dstPixelPtr, 0);
      x0 = X; y0 = Y;
      x1 = (x0+dX); y1 = (y0+dY);

      hi_deltax = vis_to_double((((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1)),
                                (((x1 & 0xFFFE) << 15) | ((x1 & 0xFFFE) >> 1)));
      hi_deltay = vis_to_double((((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1)),
                                (((y1 & 0xFFFE) << 15) | ((y1 & 0xFFFE) >> 1)));

      hi_delta1_x = vis_fpsub16(mask_7ffe, hi_deltax);
      hi_delta1_y = vis_fpsub16(mask_7ffe, hi_deltay);

      dstLineEnd += 1;

      mask = vis_edge16(dstPixelPtr, dstLineEnd);
      gsrd = ((8 - (mlib_addr)dstPixelPtr) & 7);

      i = 0;

      if (i <= cols - 2) {
        NEXT_PIXELS_2BL_S16()

#pragma pipeloop(0)
        for (; i <= cols - 4; i+= 2) {
          vis_alignaddr((void*)6, 0);
          LOAD_2BL_S16()
          NEXT_PIXELS_2BL_S16()
          RESULT_1BL_S16(hi)
          vis_alignaddr((void*)gsrd, 0);
          hi8 = vis_faligndata(hi8, hi8);
          vis_pst_16(hi8, dp++, mask);
          vis_pst_16(hi8, dp, ~mask);
        }
        vis_alignaddr((void*)6, 0);
        LOAD_2BL_S16()
        RESULT_1BL_S16(hi)
        vis_alignaddr((void*)gsrd, 0);
        hi8 = vis_faligndata(hi8, hi8);
        vis_pst_16(hi8, dp++, mask);
        vis_pst_16(hi8, dp, ~mask);
        i += 2;
      }

      if (i < cols) {
        vis_alignaddr((void*)6, 0);
        filterpos = MLIB_POINTER_SHIFT(Y);
        srcPixelPtr = *(MLIB_TYPE **)((mlib_u8 *)lineAddr + filterpos);
        srcPixelPtr += (X >> (MLIB_SHIFT-1)) &~ 1;

        ul0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 2), ul0);
        ul0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 0), ul0);
        ur0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 6), ur0);
        ur0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, 4), ur0);
        ll0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride+2), ll0);
        ll0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride), ll0);
        lr0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride+6), lr0);
        lr0 = vis_faligndata(vis_ld_u16_i(srcPixelPtr, srcYStride+4), lr0);
        RESULT_1BL_S16(hi)
        vis_alignaddr((void*)gsrd, 0);
        hi8 = vis_faligndata(hi8, hi8);
        emask = vis_edge16(dp, dstLineEnd);
        vis_pst_16(hi8, dp++, mask & emask);

        if ((mlib_u8*)dp <= (mlib_u8*)dstLineEnd) {
          emask = vis_edge16(dp, dstLineEnd);
          vis_pst_16(hi8, dp, emask);
        }
      }
    }
  }
}

/***********************************************************************/

void mlib_v_ImageAffine_s16_3ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_s32 xSrc;
  mlib_d64 dx64, dy64;

  dx64 = vis_to_double_dup((((dX) & 0xFFFF) << 16) | ((dX) & 0xFFFF));
  dy64 = vis_to_double_dup((((dY) & 0xFFFF) << 16) | ((dY) & 0xFFFF));

  for (j = yStart; j <= yFinish; j++) {
    mlib_d64 *dp, *psr;
    mlib_d64 ul0, ur0, ll0, lr0;
    mlib_s32 cols, filterpos;
    mlib_d64 hi_deltax, hi_deltay, hi_delta1_x, hi_delta1_y;
    mlib_d64 lo_deltax, lo_deltay, lo_delta1_x, lo_delta1_y;
    mlib_d64 hi8;
    mlib_s32 x0, y0;
    mlib_s32 i, mask, rmask, gsrd;
    mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);
    mlib_d64 mask_7ffe = vis_to_double_dup(0x7FFE7FFE);
    mlib_d64 a0, a1, a2;

    CLIP(3)

    cols = xRight - xLeft + 1;
    dp = (mlib_d64 *)((mlib_addr)dstPixelPtr & ~7);
    x0 = X; y0 = Y;

    hi_deltax = vis_to_double_dup(((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1));
    hi_deltay = vis_to_double_dup(((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1));

    hi_delta1_x = vis_fpsub16(mask_7ffe, hi_deltax);
    hi_delta1_y = vis_fpsub16(mask_7ffe, hi_deltay);

    x0 += dX; y0 += dY;

    lo_deltax = vis_to_double_dup(((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1));
    lo_deltay = vis_to_double_dup(((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1));

    lo_delta1_x = vis_fpsub16(mask_7ffe, lo_deltax);
    lo_delta1_y = vis_fpsub16(mask_7ffe, lo_deltay);

    mask = 0xfc >> ((mlib_addr)dstPixelPtr & 7),
    rmask = (~mask << 2) & 0xf0;
    gsrd = ((8 - (mlib_addr)dstPixelPtr) & 7);

    NEXT_PIXELS_3BL_S16()

#pragma pipeloop(0)
    for (i = 0; i < cols - 2; i += 2) {
      LOAD_3BL_S16()
      NEXT_PIXELS_3BL_S16()
      RESULT_1BL_S16(hi)
      STORE_3BL_S16()

      LOAD_3BL_S16()
      NEXT_PIXELS_3BL_S16()
      RESULT_1BL_S16(lo)
      STORE_3BL_S16()
    }
    LOAD_3BL_S16()
    RESULT_1BL_S16(hi)
    STORE_3BL_S16()
    i++;

    if (i < cols) {

      NEXT_PIXELS_3BL_S16()
      LOAD_3BL_S16()
      RESULT_1BL_S16(lo)

      vis_alignaddr((void *)gsrd, 0);
      hi8 = vis_faligndata(hi8, hi8);
      vis_pst_8(hi8, dp, mask);
      vis_pst_8(hi8, dp + 1, rmask);
    }
  }
}

/***********************************************************************/

void mlib_v_ImageAffine_s16_4ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_s32 srcYStride1 = (srcYStride >> 3);
  mlib_d64 dx64, dy64;

  dx64 = vis_to_double_dup((((dX) & 0xFFFF) << 16) | ((dX) & 0xFFFF));
  dy64 = vis_to_double_dup((((dY) & 0xFFFF) << 16) | ((dY) & 0xFFFF));

  if ((((mlib_addr)lineAddr[0] | srcYStride) & 7) == 0) {

    for (j = yStart; j <= yFinish; j++) {
      mlib_d64 *dp, *psr;
      mlib_d64 ul0, ur0, ll0, lr0;
      mlib_s32 cols, filterpos;
      mlib_d64 hi_deltax, hi_deltay, hi_delta1_x, hi_delta1_y;
      mlib_d64 lo_deltax, lo_deltay, lo_delta1_x, lo_delta1_y;
      mlib_d64 hi8;
      mlib_s32 x0, y0;
      mlib_s32 i, mask, gsrd;
      mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);
      mlib_d64 mask_7ffe = vis_to_double_dup(0x7FFE7FFE);

      CLIP(4)

      cols = xRight - xLeft + 1;
      dp = (mlib_d64 *)((mlib_addr)dstPixelPtr & ~7);
      x0 = X; y0 = Y;

      hi_deltax = vis_to_double_dup(((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1));
      hi_deltay = vis_to_double_dup(((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1));

      hi_delta1_x = vis_fpsub16(mask_7ffe, hi_deltax);
      hi_delta1_y = vis_fpsub16(mask_7ffe, hi_deltay);

      x0 += dX; y0 += dY;

      lo_deltax = vis_to_double_dup(((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1));
      lo_deltay = vis_to_double_dup(((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1));

      lo_delta1_x = vis_fpsub16(mask_7ffe, lo_deltax);
      lo_delta1_y = vis_fpsub16(mask_7ffe, lo_deltay);

      dstLineEnd += 3;

      mask = vis_edge16(dstPixelPtr, dstLineEnd);
      gsrd = ((8 - (mlib_addr)dstPixelPtr) & 7);
      vis_alignaddr((void *)gsrd, 0);

      NEXT_PIXELS_4BL_S16()

#pragma pipeloop(0)
      for (i = 0; i < cols - 2; i += 2) {
        LOAD_4BL_S16_AL()
        NEXT_PIXELS_4BL_S16()
        RESULT_1BL_S16(hi)
        hi8 = vis_faligndata(hi8, hi8);
        vis_pst_16(hi8, dp++, mask);
        vis_pst_16(hi8, dp, ~mask);

        LOAD_4BL_S16_AL()
        NEXT_PIXELS_4BL_S16()
        RESULT_1BL_S16(lo)
        hi8 = vis_faligndata(hi8, hi8);
        vis_pst_16(hi8, dp++, mask);
        vis_pst_16(hi8, dp, ~mask);
      }
      LOAD_4BL_S16_AL()
      RESULT_1BL_S16(hi)
      hi8 = vis_faligndata(hi8, hi8);
      vis_pst_16(hi8, dp++, mask);
      vis_pst_16(hi8, dp, ~mask);
      i++;

      if (i < cols) {
        NEXT_PIXELS_4BL_S16()
        LOAD_4BL_S16_AL()
        RESULT_1BL_S16(lo)
        hi8 = vis_faligndata(hi8, hi8);
        vis_pst_16(hi8, dp++, mask);
        vis_pst_16(hi8, dp, ~mask);
      }
    }
  } else {

    for (j = yStart; j <= yFinish; j++) {
      mlib_d64 *dp, *psr;
      mlib_d64 ul0, ur0, ll0, lr0;
      mlib_s32 cols, filterpos;
      mlib_d64 hi_deltax, hi_deltay, hi_delta1_x, hi_delta1_y;
      mlib_d64 lo_deltax, lo_deltay, lo_delta1_x, lo_delta1_y;
      mlib_d64 hi8;
      mlib_s32 x0, y0;
      mlib_s32 i, mask, gsrd;
      mlib_d64 mask_7fff = vis_to_double_dup(0x7FFF7FFF);
      mlib_d64 mask_7ffe = vis_to_double_dup(0x7FFE7FFE);
      mlib_d64 a0, a1, a2;

      CLIP(4)

      cols = xRight - xLeft + 1;
      dp = (mlib_d64 *)((mlib_addr)dstPixelPtr & ~7);
      x0 = X; y0 = Y;

      hi_deltax = vis_to_double_dup(((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1));
      hi_deltay = vis_to_double_dup(((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1));

      hi_delta1_x = vis_fpsub16(mask_7ffe, hi_deltax);
      hi_delta1_y = vis_fpsub16(mask_7ffe, hi_deltay);

      x0 += dX; y0 += dY;

      lo_deltax = vis_to_double_dup(((x0 & 0xFFFE) << 15) | ((x0 & 0xFFFE) >> 1));
      lo_deltay = vis_to_double_dup(((y0 & 0xFFFE) << 15) | ((y0 & 0xFFFE) >> 1));

      lo_delta1_x = vis_fpsub16(mask_7ffe, lo_deltax);
      lo_delta1_y = vis_fpsub16(mask_7ffe, lo_deltay);

      dstLineEnd += 3;

      mask = vis_edge16(dstPixelPtr, dstLineEnd);
      gsrd = ((8 - (mlib_addr)dstPixelPtr) & 7);

      NEXT_PIXELS_4BL_S16()

#pragma pipeloop(0)
      for (i = 0; i < cols - 2; i += 2) {
        LOAD_4BL_S16()
        NEXT_PIXELS_4BL_S16()
        RESULT_1BL_S16(hi)
        vis_alignaddr((void *)gsrd, 0);
        hi8 = vis_faligndata(hi8, hi8);
        vis_pst_16(hi8, dp++, mask);
        vis_pst_16(hi8, dp, ~mask);

        LOAD_4BL_S16()
        NEXT_PIXELS_4BL_S16()
        RESULT_1BL_S16(lo)
        vis_alignaddr((void *)gsrd, 0);
        hi8 = vis_faligndata(hi8, hi8);
        vis_pst_16(hi8, dp++, mask);
        vis_pst_16(hi8, dp, ~mask);
      }
      LOAD_4BL_S16()
      RESULT_1BL_S16(hi)
      vis_alignaddr((void *)gsrd, 0);
      hi8 = vis_faligndata(hi8, hi8);
      vis_pst_16(hi8, dp++, mask);
      vis_pst_16(hi8, dp, ~mask);
      i++;

      if (i < cols) {
        NEXT_PIXELS_4BL_S16()
        LOAD_4BL_S16()
        RESULT_1BL_S16(lo)

        vis_alignaddr((void *)gsrd, 0);
        hi8 = vis_faligndata(hi8, hi8);
        vis_pst_16(hi8, dp++, mask);
        vis_pst_16(hi8, dp, ~mask);
      }
    }
  }
}

/***********************************************************************/
#undef  MLIB_TYPE
#define MLIB_TYPE mlib_s32

void mlib_v_ImageAffine_s32_1ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64 scale = 1. / 65536.;
  mlib_s32 srcYStride1 = srcYStride+4;
  mlib_s32 xSrc, ySrc;

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
    xSrc = X>>MLIB_SHIFT;  X += dX;
    srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc) + xSrc;
    k3 = t * u; k2 = (1.0 - t) * u;
    k1 = t * (1.0 - u); k0 = (1.0 - t) * (1.0 - u);
    a00_0 = srcPixelPtr[0];
    a01_0 = srcPixelPtr[1];
    a10_0 = srcPixelPtr[srcYStride];
    a11_0 = srcPixelPtr[srcYStride1];

    for (;dstPixelPtr < dstLineEnd; dstPixelPtr++)
    {

      mlib_s32 t0;

      pix0 = k0 * a00_0  + k1 * a01_0 + k2 * a10_0 + k3 * a11_0;
      t =  (X & MLIB_MASK) * scale;  u = (Y & MLIB_MASK) * scale;
      ySrc = MLIB_POINTER_SHIFT(Y);  Y += dY;
      xSrc = X>>MLIB_SHIFT;  X += dX;
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

void mlib_v_ImageAffine_s32_2ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64  scale = 1. / 65536.;
  mlib_s32 xSrc, ySrc;

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
    xSrc = X>>MLIB_SHIFT;  X += dX;
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
      xSrc = X>>MLIB_SHIFT;  X += dX;
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

void mlib_v_ImageAffine_s32_3ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64  scale = 1. / 65536.;
  mlib_s32 xSrc, ySrc;

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
    xSrc = X>>MLIB_SHIFT;  X += dX;
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
      xSrc = X>>MLIB_SHIFT;  X += dX;
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

void mlib_v_ImageAffine_s32_4ch_bl(MLIB_AFFINEDEFPARAM2)
{
  DECLAREVAR
  mlib_d64  scale = 1. / 65536.;
  mlib_s32 xSrc, ySrc;

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
    xSrc = X>>MLIB_SHIFT;  X += dX;
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
      xSrc = X>>(MLIB_SHIFT - 2);  X += dX;
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
