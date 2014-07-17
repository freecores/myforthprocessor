/*
 * @(#)mlib_v_ImageConvMxN.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageConvMxN.c	1.20	00/04/17 SMI"

/*
 * FUNCTION
 *      mlib_ImageConvMxN - image convolution with edge condition
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageConvMxN(mlib_image *dst,
 *                                    mlib_image *src,
 *                                    mlib_s32   *kernel,
 *                                    mlib_s32   m,
 *                                    mlib_s32   n,
 *                                    mlib_s32   dm,
 *                                    mlib_s32   dn,
 *                                    mlib_s32   scale,
 *                                    mlib_s32   cmask,
 *                                    mlib_edge  edge)
 *
 * ARGUMENTS
 *      dst       Pointer to destination image.
 *      src       Pointer to source image.
 *      m         Kernel width (m must be larger than 1).
 *      n         Kernel height (n must be larger than 1).
 *      dn, dn    Position of key element in convolution kernel.
 *      kernel    Pointer to convolution kernel.
 *      scale     The scaling factor to convert the input integer
 *                coefficients into floating-point coefficients:
 *                floating-point coefficient = integer coefficient * 2^(-scale)
 *      cmask     Channel mask to indicate the channels to be convolved.
 *                Each bit of which represents a channel in the image. The
 *                channels corresponded to 1 bits are those to be processed.
 *      edge      Type of edge condition.
 *
 * DESCRIPTION
 *      2-D convolution, MxN kernel.
 *
 *      The center of the source image is mapped to the center of the
 *      destination image.
 *      The unselected channels are not overwritten. If both src and dst have
 *      just one channel, cmask is ignored.
 *
 *      The edge condition can be one of the following:
 *              MLIB_EDGE_DST_NO_WRITE  (default)
 *              MLIB_EDGE_DST_FILL_ZERO
 *              MLIB_EDGE_DST_COPY_SRC
 *              MLIB_EDGE_SRC_EXTEND
 *
 * RESTRICTION
 *      The src and the dst must be the same type and have same number
 *      of channels (1, 2, 3, or 4).
 *      m > 1.   n > 1.
 *      0 <= dm < m. 0 <= dn < n.
 *      For data type MLIB_BYTE - 16 <= scale <= 31.
 *      For data type MLIB_SHORT - 17 <= scale <= 32.
 *      For data type MLIB_INT - scale >= 0.
 *
 *      The kernel coefficients used in function mlib_ImageConvMxN
 *      has to be preprocessed in a special way.
 *      It is recommended to preprocess these coefficients
 *      using the mlib_ImageConvKernelConvert function.
 */

#include <stdlib.h>    /* defines NULL */
#include <mlib_image.h>
#include <mlib_ImageCheck.h>

/***************************************************************/

mlib_image *mlib_ImageSet(mlib_image *image,
                          mlib_type  type,
                          mlib_s32   channels,
                          mlib_s32   width,
                          mlib_s32   height,
                          mlib_s32   stride,
                          void       *data);

mlib_status mlib_ImageConvCopyEdge(mlib_image *dst, mlib_image *src, mlib_s32 dx_l,
                                   mlib_s32 dx_r, mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask);
mlib_status mlib_ImageConvClearEdge(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                    mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask);

/***************************************************************/

mlib_status mlib_v_convMxN_8nw( mlib_image *dst, mlib_image *src,
                                mlib_s32  kwid, mlib_s32 khgt,
                                mlib_s32 dm,    mlib_s32 dn,
                                mlib_s32 *kernel, mlib_s32 scalef_expon,
                                mlib_s32 cmask );

mlib_status mlib_v_convMxN_8ext( mlib_image *dst, mlib_image *src,
                                 mlib_s32  kwid, mlib_s32 khgt,
                                 mlib_s32  dx_l, mlib_s32 dx_r,
                                 mlib_s32  dy_t, mlib_s32 dy_b,
                                 mlib_s32 *kernel, mlib_s32 scalef_expon,
                                 mlib_s32 cmask );

/***************************************************************/

mlib_status mlib_ImageConvMxN_f(mlib_image *dst,
                                mlib_image *src,
                                mlib_s32   *kernel,
                                mlib_s32   m,
                                mlib_s32   n,
                                mlib_s32   dm,
                                mlib_s32   dn,
                                mlib_s32   scale,
                                mlib_s32   cmask,
                                mlib_edge  edge);

/***************************************************************/

void mlib_ImageConvMxN_U8(mlib_image *dst,
                          mlib_image *src,
                          mlib_d64   *k,
                          mlib_s32   m,
                          mlib_s32   n,
                          mlib_u8    *acmask)
{
}

/***************************************************************/

#ifdef MLIB_TEST

mlib_status mlib_v_ImageConvMxN(mlib_image *dst,
                                mlib_image *src,
                                mlib_s32   *kernel,
                                mlib_s32   m,
                                mlib_s32   n,
                                mlib_s32   dm,
                                mlib_s32   dn,
                                mlib_s32   scale,
                                mlib_s32   cmask,
                                mlib_edge  edge)
#else

mlib_status mlib_ImageConvMxN(mlib_image *dst,
                              mlib_image *src,
                              mlib_s32   *kernel,
                              mlib_s32   m,
                              mlib_s32   n,
                              mlib_s32   dm,
                              mlib_s32   dn,
                              mlib_s32   scale,
                              mlib_s32   cmask,
                              mlib_edge  edge)
#endif
{
  mlib_status stat;
  mlib_image tmp_src[1], tmp_dst[1];
  mlib_s32   slb, dlb,      /* line bytes (y stride) */
             src_wid, src_hgt,
             dst_wid, dst_hgt,
             src_nch, dst_nch;
  mlib_type  src_dtype, dst_dtype;
  mlib_s32   zero[4] = { 0, 0, 0, 0 };
  mlib_s32   dx, dy, dxd, dxs, dyd, dys, dw, dh;
  mlib_s32   dx_l, dx_r, dy_t, dy_b, conv_edge;
  mlib_u8    *sa, *da;

  if (n < 1 || m < 1 || dm < 0 || dm > m-1 || dn < 0 || dn > n-1) {
    return MLIB_FAILURE;
  }

  MLIB_IMAGE_CHECK(dst);
  MLIB_IMAGE_CHECK(src);
  if (kernel == NULL) return MLIB_NULLPOINTER;

  /* Special VIS version is only on images of U8 data type */
  if (mlib_ImageGetType(dst) != MLIB_BYTE) {
    return mlib_ImageConvMxN_f(dst, src, kernel, m, n, dm, dn, scale, cmask, edge);
  }

  MLIB_IMAGE_TYPE_EQUAL(dst, src);
  MLIB_IMAGE_CHAN_EQUAL(dst, src);

  MLIB_IMAGE_GET_ALL_PARAMS(dst, dst_dtype, dst_nch, dst_wid, dst_hgt, dlb, da);
  MLIB_IMAGE_GET_ALL_PARAMS(src, src_dtype, src_nch, src_wid, src_hgt, slb, sa);

  dw = (src_wid < dst_wid) ? src_wid : dst_wid;
  dh = (src_hgt < dst_hgt) ? src_hgt : dst_hgt;
  dx = (src_wid - dst_wid);
  dy = (src_hgt - dst_hgt);
  dxs = ((dx > 0) ? (dx+1) >> 1 : 0);
  dxd = ((dx > 0) ? 0 : (- dx) >> 1);
  dys = ((dy > 0) ? (dy+1) >> 1 : 0);
  dyd = ((dy > 0) ? 0 : (- dy) >> 1);

  dx_l = dm - dxs;
  dx_r = dxs - dx + (m - dm - 1);
  dy_t = dn - dys;
  dy_b = dys - dy + (n - dn - 1);
  dx_l = ((dx_l < 0) ? 0 : dx_l);
  dx_r = ((dx_r < 0) ? 0 : (dx_r > (m - dm - 1) ? (m - dm - 1) : dx_r));
  dy_t = ((dy_t < 0) ? 0 : dy_t);
  dy_b = ((dy_b < 0) ? 0 : (dy_b > (n - dn - 1) ? (n - dn - 1) : dy_b));

  conv_edge = dx_l + dx_r + dy_t + dy_b;
  if (conv_edge == 0) edge = MLIB_EDGE_DST_NO_WRITE;
  if (src_nch == 1) cmask = 1;

  if (scale < 16 || scale > 31) {
    stat = MLIB_FAILURE;
  } else {
    stat = MLIB_SUCCESS;
    mlib_ImageSet(tmp_dst, dst_dtype, dst_nch, (dw + m - 1 - dx_l - dx_r),
                  (dh + n - 1 - dy_t - dy_b), dlb,
                  (da + dlb*(dyd + dy_t - dn) + dst_nch*(dxd + dx_l - dm)));
    mlib_ImageSet(tmp_src, dst_dtype, dst_nch, (dw + m - 1 - dx_l - dx_r),
                  (dh + n - 1 - dy_t - dy_b), slb,
                  (sa + slb*(dys + dy_t - dn) + dst_nch*(dxs + dx_l - dm)));

    switch (edge) {
      case MLIB_EDGE_DST_FILL_ZERO:
        if (((dx_l + dx_r) < dw) && (dy_t + dy_b) < dh)
          stat = mlib_v_convMxN_8nw(tmp_dst, tmp_src, m, n, dm, dn, kernel, scale, cmask);
        if (stat != MLIB_SUCCESS) break;
        mlib_ImageSet(tmp_dst, dst_dtype, dst_nch, dw, dh, dlb,
                      (da + dlb*dyd + dst_nch*dxd));
        if ((dx_l + dx_r) > dw) {
          dx_l = dw; dx_r = 0;
        }
        if ((dy_t + dy_b) > dh) {
          dy_t = dh; dy_b = 0;
        }
        stat = mlib_ImageConvClearEdge(tmp_dst, dx_l, dx_r, dy_t, dy_b, zero, cmask);
        break;
      case MLIB_EDGE_DST_COPY_SRC:
        if (((dx_l + dx_r) < dw) && (dy_t + dy_b) < dh)
          stat = mlib_v_convMxN_8nw(tmp_dst, tmp_src, m, n, dm, dn, kernel, scale, cmask);
        if (stat != MLIB_SUCCESS) break;
        mlib_ImageSet(tmp_dst, dst_dtype, dst_nch, dw, dh, dlb,
                      (da + dlb*dyd + dst_nch*dxd));
        mlib_ImageSet(tmp_src, dst_dtype, dst_nch, dw, dh, slb,
                      (sa + slb*dys + dst_nch*dxs));
        if ((dx_l + dx_r) > dw) {
          dx_l = dw; dx_r = 0;
        }
        if ((dy_t + dy_b) > dh) {
          dy_t = dh; dy_b = 0;
        }
        stat = mlib_ImageConvCopyEdge(tmp_dst, tmp_src, dx_l, dx_r, dy_t, dy_b, cmask);
        break;
      case MLIB_EDGE_SRC_EXTEND:
        mlib_ImageSet(tmp_dst, dst_dtype, dst_nch, dw, dh, dlb,
                      (da + dlb*dyd + dst_nch*dxd));
        mlib_ImageSet(tmp_src, dst_dtype, dst_nch, dw, dh, slb,
                      (sa + slb*(dys + dy_t - dn) + dst_nch*(dxs + dx_l - dm)));
        stat = mlib_v_convMxN_8ext(tmp_dst, tmp_src, m, n, dx_l, dx_r, dy_t, dy_b, kernel, scale, cmask);
        break;
      case MLIB_EDGE_DST_NO_WRITE:
      default:
        if (((dx_l + dx_r) < dw) && (dy_t + dy_b) < dh)
          stat = mlib_v_convMxN_8nw(tmp_dst, tmp_src, m, n, dm, dn, kernel, scale, cmask);
        break;
    }
  }

  return (stat);
}

/***************************************************************/
