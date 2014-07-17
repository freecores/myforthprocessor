/*
 * @(#)mlib_ImageConvMxNFunc.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageConvMxNFunc.c	1.7	00/04/17 SMI"
#endif /* __SUNPRO_C */

#ifdef _MSC_VER
#pragma warning(disable:4068 4244 4101)
#endif /*_MSC_VER*/

/*
 * FUNCTION
 *      mlib_ImageConvMxN_f - internal function: "C" version of image convolution
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageConvMxN_f(mlib_image *dst,
 *                                      mlib_image *src,
 *                                      mlib_s32   *kernel,
 *                                      mlib_s32   m,
 *                                      mlib_s32   n,
 *                                      mlib_s32   dm,
 *                                      mlib_s32   dn,
 *                                      mlib_s32   scale,
 *                                      mlib_s32   cmask,
 *                                      mlib_edge  edge)
 *
 * ARGUMENTS
 *      dst       Pointer to destination image.
 *      src       Pointer to source image.
 *      m         Kernel width (m must be larger than 1).
 *      n         Kernel height (n must be larger than 1).
 *      dm, dn    Position of key element in convolution kernel.
 *      kernel    Pointer to convolution kernel.
 *      scale     The scaling factor to convert the input integer
 *                coefficients into floating-point coefficients:
 *                floating-point coefficient = integer coefficient * 2^(-scale)
 *      cmask     Channel mask to indicate the channels to be convolved.
 *                Each bit of which represents a channel in the image. The
 *                channels corresponded to 1 bits are those to be processed.
 *      edge      Type of edge condition.
 *
 */

/***************************************************************/

#include <stdlib.h>
#include <mlib_image.h>
#include <mlib_ImageCheck.h>
#include <mlib_SysMath.h>

/***************************************************************/

#define MLIB_D2_16 65536.0f
#define MLIB_D2_24 16777216.0f
extern mlib_f32 mlib_U82F32[];

/***************************************************************/

#ifdef MLIB_USE_FTOI_CLAMPING

#define FLOAT2INT_CLAMP(X) ((mlib_s32)(X))

#else

#define FLOAT2INT_CLAMP(X) (((X) >= MLIB_S32_MAX) ? MLIB_S32_MAX :                     \
                            (((X) <= MLIB_S32_MIN) ? MLIB_S32_MIN : ((mlib_s32)(X))))

#endif

/***************************************************************/

mlib_image *mlib_ImageSet(mlib_image *image,
                          mlib_type  type,
                          mlib_s32   channels,
                          mlib_s32   width,
                          mlib_s32   height,
                          mlib_s32   stride,
                          void       *data);

mlib_status mlib_ImageConvCopyEdge(mlib_image *dst,
                                   mlib_image *src,
                                   mlib_s32   dx_l,
                                   mlib_s32   dx_r,
                                   mlib_s32   dy_t,
                                   mlib_s32   dy_b,
                                   mlib_s32   cmask);

mlib_status mlib_ImageConvClearEdge(mlib_image *img,
                                    mlib_s32   dx_l,
                                    mlib_s32   dx_r,
                                    mlib_s32   dy_t,
                                    mlib_s32   dy_b,
                                    mlib_s32   *color,
                                    mlib_s32   cmask);

/***************************************************************/

mlib_status mlib_ImageConvMxN_U8(mlib_image *dst,
                                 mlib_image *src,
                                 mlib_d64   *k,
                                 mlib_s32   m,
                                 mlib_s32   n,
                                 mlib_u8    *acmask);

mlib_status mlib_ImageConvMxN_S16(mlib_image *dst,
                                  mlib_image *src,
                                  mlib_d64   *k,
                                  mlib_s32   m,
                                  mlib_s32   n,
                                  mlib_u8    *acmask);

mlib_status mlib_ImageConvMxN_S32(mlib_image *dst,
                                  mlib_image *src,
                                  mlib_d64   *k,
                                  mlib_s32   m,
                                  mlib_s32   n,
                                  mlib_u8    *acmask);

/***************************************************************/

static void mlib_ImageConvMxNMulAdd_U8(mlib_f32 *dst,
                                       mlib_u8  *src,
                                       mlib_d64 *dkernel,
                                       mlib_s32 n,
                                       mlib_s32 m,
                                       mlib_s32 nch)
{
  mlib_f32 *dst1 = dst + 1;
  mlib_s32 i, j;

  for (j = 0; j < m; j += 3, src += 3*nch, dkernel += 3) {
    mlib_u8  *src2  = src  + 2*nch;
    mlib_f32 hval0  = (mlib_f32) dkernel[0];
    mlib_f32 hval1  = (mlib_f32) dkernel[1];
    mlib_f32 hval2  = (mlib_f32) dkernel[2];
    mlib_f32 val0   = mlib_U82F32[src[0]];
    mlib_f32 val1   = mlib_U82F32[src[nch]];
    mlib_f32 dval   = dst[0];

    if (j == m - 2) {
      hval2 = 0.f;
    } else if (j == m - 1) {
      hval1 = 0.f;
      hval2 = 0.f;
    }

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < n; i++) {
      mlib_f32 dval0 = val0*hval0 + dval;
      mlib_f32 val2  = mlib_U82F32[src2[i*nch]];

      dval      = dst1[i];
      dval0    += val1*hval1;
      dval0    += val2*hval2;
      val0      = val1;
      val1      = val2;

      dst[i]    = dval0;
    }
  }
}

/***************************************************************/

static void mlib_ImageConvMxNMedian_U8(mlib_u8  *dst,
                                       mlib_f32 *src,
                                       mlib_s32 n,
                                       mlib_s32 nch)
{
  mlib_s32 i;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for (i = 0; i < n; i++)
    ((mlib_s32*)src)[i] = FLOAT2INT_CLAMP(src[i]*MLIB_D2_24);

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for (i = 0; i < n; i++) {
    mlib_s32 detl = 128 + (((mlib_s32*)src)[i] >> 24);

    src[i]    = -128.f;
    dst[i*nch] = detl;
  }
}

/***************************************************************/

static void mlib_ImageConvMxNU82U8_ext(mlib_u8  *dst,
                                       mlib_u8  *src,
                                       mlib_s32 n,
                                       mlib_s32 nch,
                                       mlib_s32 dx_l,
                                       mlib_s32 dx_r)
{
  mlib_s32 i;
  mlib_s32 val = src[0];

  for (i = 0; i < dx_l; i++) dst[i] = val;
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for (; i < n - dx_r; i++) dst[i] = src[nch*(i - dx_l)];
  val = dst[n - dx_r - 1];
  for (; i < n; i++) dst[i] = val;
}

/***************************************************************/

void mlib_ImageConvMxN_U8_ext(mlib_image *dst,
                              mlib_image *src,
                              mlib_d64   *dkernel,
                              mlib_s32   m,
                              mlib_s32   n,
                              mlib_s32   dx_l,
                              mlib_s32   dx_r,
                              mlib_s32   dy_t,
                              mlib_s32   dy_b,
                              mlib_u8    *acmask,
                              mlib_d64   *dsa)
{
  mlib_f32   *fsa = (mlib_f32*)dsa, *fsh, *fsv;
  mlib_u8    *usa = (mlib_u8*)dsa;
  mlib_u8    *da = mlib_ImageGetData(dst);
  mlib_u8    *sa = mlib_ImageGetData(src);
  mlib_s32   dlb = mlib_ImageGetStride(dst);
  mlib_s32   slb = mlib_ImageGetStride(src);
  mlib_s32   dw  = mlib_ImageGetWidth(dst);
  mlib_s32   dh  = mlib_ImageGetHeight(dst);
  mlib_s32   nch = mlib_ImageGetChannels(dst);
  mlib_s32   i, j, j1, k;

  fsh  = fsa + dw + m;
  fsv  = fsh + dw;

  for (i = 0; i < dw; i++) {
    fsh[i] = -128.f;
    fsv[i] = -128.f;
  }

  for (j = 0; j < dh; j++, da += dlb) {
    for (k = 0; k < nch; k++)
      if (acmask[k]) {
        mlib_u8  *sa1      = sa + k;
        mlib_d64 *dkernel1 = dkernel;

        for (j1 = 0; j1 < n; j1++, dkernel1 += m) {
          mlib_ImageConvMxNU82U8_ext(usa, sa1, dw + m - 1, nch, dx_l, dx_r);
          mlib_ImageConvMxNMulAdd_U8(fsh, usa, dkernel1, dw, m, 1);
          if ((j + j1 >= dy_t) && (j + j1 < dh + n - dy_b - 2)) sa1 += slb;
        }

        mlib_ImageConvMxNMedian_U8(da + k, fsh, dw, nch);
      }
    if ((j >= dy_t) && (j < dh + n - dy_b - 2)) sa += slb;
  }
}

/***************************************************************/

static void mlib_ImageConvMxNS162S32(mlib_s32 *dst,
                                     mlib_s16 *src,
                                     mlib_s32 n,
                                     mlib_s32 nch)
{
  mlib_s32 i;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for (i = 0; i < n; i++) dst[i] = src[nch*i];
}

/***************************************************************/

static void mlib_ImageConvMxNMulAdd_S16(mlib_f32 *dst,
                                        mlib_s32 *src,
                                        mlib_d64 *dkernel,
                                        mlib_s32 n,
                                        mlib_s32 m)
{
  mlib_f32 *dst1 = dst + 1;
  mlib_s32 i, j;

  for (j = 0; j < m; j += 3, src += 3, dkernel += 3) {
    mlib_s32 *src2  = src  + 2;
    mlib_f32 hval0  = (mlib_f32) dkernel[0];
    mlib_f32 hval1  = (mlib_f32) dkernel[1];
    mlib_f32 hval2  = (mlib_f32) dkernel[2];
    mlib_f32 val0   = (mlib_f32) src[0];
    mlib_f32 val1   = (mlib_f32) src[1];
    mlib_f32 dval   = dst[0];

    if (j == m - 2) {
      hval2 = 0.f;
    } else if (j == m - 1) {
      hval1 = 0.f;
      hval2 = 0.f;
    }

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < n; i++) {
      mlib_f32 dval0 = val0*hval0 + dval;
      mlib_f32 val2  = (mlib_f32) src2[i];

      dval      = dst1[i];
      dval0    += val1*hval1;
      dval0    += val2*hval2;
      val0      = val1;
      val1      = val2;

      dst[i]    = dval0;
    }
  }
}

/***************************************************************/

static void mlib_ImageConvMxNMedian_S16(mlib_s16 *dst,
                                        mlib_f32 *src,
                                        mlib_s32 n,
                                        mlib_s32 nch)
{
  mlib_s32 i;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for (i = 0; i < n; i++)
    ((mlib_s32*)src)[i] = FLOAT2INT_CLAMP(src[i]*MLIB_D2_16);

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for (i = 0; i < n; i++) {
    mlib_s32 detl = ((mlib_s32*)src)[i];

    src[i]    = 0.5f;
    dst[i*nch] = detl >> 16;
  }
}

/***************************************************************/

static void mlib_ImageConvMxNS162S32_ext(mlib_s32 *dst,
                                         mlib_s16 *src,
                                         mlib_s32 n,
                                         mlib_s32 nch,
                                         mlib_s32 dx_l,
                                         mlib_s32 dx_r)
{
  mlib_s32 i;
  mlib_s32 val = src[0];

  for (i = 0; i < dx_l; i++) dst[i] = val;
  for (; i < n - dx_r; i++) dst[i] = src[nch*(i - dx_l)];
  val = dst[n - dx_r - 1];
  for (i = n - dx_r; i < n; i++) dst[i] = val;
}

/***************************************************************/

void mlib_ImageConvMxN_S16_ext(mlib_image *dst,
                               mlib_image *src,
                               mlib_d64   *dkernel,
                               mlib_s32   m,
                               mlib_s32   n,
                               mlib_s32   dx_l,
                               mlib_s32   dx_r,
                               mlib_s32   dy_t,
                               mlib_s32   dy_b,
                               mlib_u8    *acmask,
                               mlib_d64   *dsa)
{
  mlib_f32   *fsa = (mlib_f32*)dsa, *fsh, *fsv;
  mlib_s32   *isa = (mlib_s32*)dsa;
  mlib_s16   *da = mlib_ImageGetData(dst);
  mlib_s16   *sa = mlib_ImageGetData(src);
  mlib_s32   dlb = mlib_ImageGetStride(dst) >> 1;
  mlib_s32   slb = mlib_ImageGetStride(src) >> 1;
  mlib_s32   dw  = mlib_ImageGetWidth(dst);
  mlib_s32   dh  = mlib_ImageGetHeight(dst);
  mlib_s32   nch = mlib_ImageGetChannels(dst);
  mlib_s32   i, j, j1, k;

  fsh  = fsa + dw + m;
  fsv  = fsh + dw;

  for (i = 0; i < dw; i++) {
    fsh[i] = 0.5f;
    fsv[i] = 0.5f;
  }

  for (j = 0; j < dh; j++, da += dlb) {
    for (k = 0; k < nch; k++)
      if (acmask[k]) {
        mlib_s16 *sa1      = sa + k;
        mlib_d64 *dkernel1 = dkernel;

        for (j1 = 0; j1 < n; j1++, dkernel1 += m) {
          mlib_ImageConvMxNS162S32_ext(isa, sa1, dw + m - 1, nch, dx_l, dx_r);
          mlib_ImageConvMxNMulAdd_S16(fsh, isa, dkernel1, dw, m);
          if ((j + j1 >= dy_t) && (j + j1 < dh + n - dy_b - 2)) sa1 += slb;
        }

        mlib_ImageConvMxNMedian_S16(da + k, fsh, dw, nch);
      }
    if ((j >= dy_t) && (j < dh + n - dy_b - 2)) sa += slb;
  }
}

/***************************************************************/

static void mlib_ImageConvMxNMulAdd_S32(mlib_d64 *dst,
                                        mlib_s32 *src,
                                        mlib_d64 *dkernel,
                                        mlib_s32 n,
                                        mlib_s32 m,
                                        mlib_s32 nch)
{
  mlib_d64 *dst1 = dst + 1;
  mlib_s32 i, j;

  for (j = 0; j < m; j += 3, src += 3*nch, dkernel += 3) {
    mlib_s32 *src2  = src  + 2*nch;
    mlib_d64 hval0  = dkernel[0];
    mlib_d64 hval1  = dkernel[1];
    mlib_d64 hval2  = dkernel[2];
    mlib_d64 val0   = src[0];
    mlib_d64 val1   = src[nch];
    mlib_d64 dval   = dst[0];

    if (j == m - 2) {
      hval2 = 0.f;
    } else if (j == m - 1) {
      hval1 = 0.f;
      hval2 = 0.f;
    }

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < n; i++) {
      mlib_d64 dval0 = val0*hval0 + dval;
      mlib_d64 val2  = src2[i*nch];

      dval      = dst1[i];
      dval0    += val1*hval1;
      dval0    += val2*hval2;
      val0      = val1;
      val1      = val2;

      dst[i]    = dval0;
    }
  }
}

/***************************************************************/

static void mlib_ImageConvMxNMedian_S32(mlib_s32 *dst,
                                        mlib_d64 *src,
                                        mlib_s32 n,
                                        mlib_s32 nch)
{
  mlib_s32 i;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for (i = 0; i < n; i++) {
    mlib_s32 detl = FLOAT2INT_CLAMP(src[i]);

    src[i]    = 0.5;
    dst[i*nch] = detl;
  }
}

/***************************************************************/

static void mlib_ImageConvMxNS322S32_ext(mlib_s32 *dst,
                                         mlib_s32 *src,
                                         mlib_s32 n,
                                         mlib_s32 nch,
                                         mlib_s32 dx_l,
                                         mlib_s32 dx_r)
{
  mlib_s32 i;
  mlib_d64 val = src[0];

  for (i = 0; i < dx_l; i++) dst[i] = (mlib_s32) val;
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for (; i < n - dx_r; i++) dst[i] = src[nch*(i - dx_l)];
  val = dst[n - dx_r - 1];
  for (; i < n; i++) dst[i] = (mlib_s32) val;
}

/***************************************************************/

void mlib_ImageConvMxN_S32_ext(mlib_image *dst,
                               mlib_image *src,
                               mlib_d64   *dkernel,
                               mlib_s32   m,
                               mlib_s32   n,
                               mlib_s32   dx_l,
                               mlib_s32   dx_r,
                               mlib_s32   dy_t,
                               mlib_s32   dy_b,
                               mlib_u8    *acmask,
                               mlib_d64   *dsa)
{
  mlib_d64   *dsh, *dsv;
  mlib_s32   *isa = (mlib_s32*)dsa;
  mlib_s32   *da = mlib_ImageGetData(dst);
  mlib_s32   *sa = mlib_ImageGetData(src);
  mlib_s32   dlb = mlib_ImageGetStride(dst) >> 2;
  mlib_s32   slb = mlib_ImageGetStride(src) >> 2;
  mlib_s32   dw  = mlib_ImageGetWidth(dst);
  mlib_s32   dh  = mlib_ImageGetHeight(dst);
  mlib_s32   nch = mlib_ImageGetChannels(dst);
  mlib_s32   i, j, j1, k;

  dsh  = dsa + dw + m;
  dsv  = dsh + dw;

  for (i = 0; i < dw; i++) {
    dsh[i] = 0.5;
    dsv[i] = 0.5;
  }

  for (j = 0; j < dh; j++, da += dlb) {
    for (k = 0; k < nch; k++)
      if (acmask[k]) {
        mlib_s32 *sa1      = sa + k;
        mlib_d64 *dkernel1 = dkernel;

        for (j1 = 0; j1 < n; j1++, dkernel1 += m) {
          mlib_ImageConvMxNS322S32_ext(isa, sa1, dw + m - 1, nch, dx_l, dx_r);
          mlib_ImageConvMxNMulAdd_S32(dsh, isa, dkernel1, dw, m, 1);
          if ((j + j1 >= dy_t) && (j + j1 < dh + n - dy_b - 2)) sa1 += slb;
        }

        mlib_ImageConvMxNMedian_S32(da + k, dsh, dw, nch);
      }
    if ((j >= dy_t) && (j < dh + n - dy_b - 2)) sa += slb;
  }
}

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
                                mlib_edge  edge)
{
  mlib_image dst1[1], src1[1];
  mlib_type  dst_dtype, src_dtype;
  mlib_d64   akernel[256], *dkernel = akernel, fscale = 1.f;
  mlib_s32   scale_exp = scale;
  mlib_u8    acmask[4];
  mlib_status stat=0;

  mlib_u8   *da, *sa;
  mlib_s32  dx, dy, dxs, dxd, dys, dyd, dw, dh;
  mlib_s32  dx_l, dx_r, dy_t, dy_b, dw1, dh1;
  mlib_s32  src_wid, src_hgt, dst_wid, dst_hgt, src_nch, dst_nch, dlb, slb;
  mlib_s32  i, nchmul;

  if (m < 1 || n < 1 || dm < 0 || dm > m-1 || dn < 0 || dn > n-1) return MLIB_FAILURE;

  MLIB_IMAGE_CHECK(dst);
  MLIB_IMAGE_CHECK(src);
  if (kernel == NULL) return MLIB_NULLPOINTER;

  MLIB_IMAGE_TYPE_EQUAL(dst, src);
  MLIB_IMAGE_CHAN_EQUAL(dst, src);

  switch(mlib_ImageGetType(dst)) {
    default:
      break;
    case MLIB_BYTE:
      if (scale < 16 || scale > 31) return MLIB_FAILURE;
      break;
    case MLIB_SHORT:
      if (scale < 17 || scale > 32 ) return MLIB_FAILURE;
      break;
    case MLIB_INT:
      if (scale < 0) return MLIB_FAILURE;
  }

  while (scale_exp > 30) {
    fscale /= (1 << 30);
    scale_exp -= 30;
  }
  fscale /= (1 << scale_exp);

  MLIB_IMAGE_GET_ALL_PARAMS(dst, dst_dtype, dst_nch, dst_wid, dst_hgt, dlb, da);
  MLIB_IMAGE_GET_ALL_PARAMS(src, src_dtype, src_nch, src_wid, src_hgt, slb, sa);

  nchmul = dst_nch << ((dst_dtype == MLIB_BYTE) ? 0 :
                       (dst_dtype == MLIB_SHORT) ? 1 : 2);
  cmask  = (dst_nch == 1) ? 1 : cmask;

  for (i = 0; i < dst_nch; i++)
    acmask[i] = (cmask & (1 << (dst_nch - i - 1)));

  dw1 = (src_wid < dst_wid) ? src_wid : dst_wid;
  dh1 = (src_hgt < dst_hgt) ? src_hgt : dst_hgt;
  dx  = (src_wid - dst_wid);
  dy  = (src_hgt - dst_hgt);
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

  dw   = dw1 - dx_l - dx_r;
  dh   = dh1 - dy_t - dy_b;

  if (m*n > 256) {
    dkernel = mlib_malloc(m*n*sizeof(mlib_d64));
    if (dkernel == NULL) return MLIB_FAILURE;
  }

  for (i = 0; i < m*n; i++) dkernel[i] = kernel[i]*fscale;

  if (edge == MLIB_EDGE_SRC_EXTEND) {
    mlib_d64 dspace[1024], *dsa = dspace;

    if (dw1 <= 0 || dh1 <= 0) {
      if (dkernel != akernel) mlib_free(dkernel);
      return MLIB_SUCCESS;
    }

    mlib_ImageSet(dst1, dst_dtype, dst_nch, dw1, dh1, dlb,
                  da + nchmul*(dxd) + (dyd)*dlb);
    mlib_ImageSet(src1, src_dtype, src_nch, dw1, dh1, slb,
                  sa + nchmul*(dxs + dx_l - dm) + (dys + dy_t - dn)*slb);

    if (1024 < 3*dw1 + m) {
      dsa = mlib_malloc(sizeof(mlib_d64)*(3*dw1 + m));
      if (dsa == NULL) {
        if (dkernel != akernel) mlib_free(dkernel);
        return MLIB_FAILURE;
      }
    }

    switch(dst_dtype) {
      default:
	break;
      case MLIB_BYTE:
        mlib_ImageConvMxN_U8_ext(dst1, src1, dkernel, m, n,
                                 dx_l, dx_r, dy_t, dy_b, acmask, dsa);
        break;
      case MLIB_SHORT:
        mlib_ImageConvMxN_S16_ext(dst1, src1, dkernel, m, n,
                                  dx_l, dx_r, dy_t, dy_b, acmask, dsa);
        break;
      case MLIB_INT:
        mlib_ImageConvMxN_S32_ext(dst1, src1, dkernel, m, n,
                                  dx_l, dx_r, dy_t, dy_b, acmask, dsa);
    }
    if (dsa != dspace) mlib_free(dsa);
    if (dkernel != akernel) mlib_free(dkernel);

    return MLIB_SUCCESS;
  }

  if (edge != MLIB_EDGE_DST_NO_WRITE) {
    mlib_s32 color[4] = { 0, 0, 0, 0 };

    if ((dx_l + dx_r) > dw1) {
      dx_l = dw1; dx_r = 0;
    }
    if ((dy_t + dy_b) > dh1) {
      dy_t = dh1; dy_b = 0;
    }

    switch(edge) {
      default:
	break;
      case MLIB_EDGE_DST_FILL_ZERO:
        if (dw1 <= 0 || dh1 <= 0) break;

        mlib_ImageSet(dst1, dst_dtype, dst_nch, dw1, dh1, dlb,
                      da + nchmul*dxd + dyd*dlb);
        mlib_ImageConvClearEdge(dst1, dx_l, dx_r, dy_t, dy_b, color, cmask);
        break;
      case MLIB_EDGE_DST_COPY_SRC:
        if (dw1 <= 0 || dh1 <= 0) break;

        mlib_ImageSet(dst1, dst_dtype, dst_nch, dw1, dh1, dlb,
                      da + nchmul*dxd + dyd*dlb);
        mlib_ImageSet(src1, src_dtype, src_nch, dw1, dh1, slb,
                      sa + nchmul*dxs + dys*slb);
        mlib_ImageConvCopyEdge(dst1, src1, dx_l, dx_r, dy_t, dy_b, cmask);
        break;
    }
  }

  dxd += dx_l;
  dyd += dy_t;
  dxs += dx_l - dm;
  dys += dy_t - dn;

  if (((dx_l + dx_r) >= dw1) || (dy_t + dy_b) >= dh1 || (dw <= 0) || (dh <= 0)) {
    if (dkernel != akernel) mlib_free(dkernel);
    return MLIB_SUCCESS;
  }

  da += (nchmul*dxd + dyd*dlb);
  sa += (nchmul*dxs + dys*slb);

  mlib_ImageSet(dst1, dst_dtype, dst_nch, dw, dh, dlb, da);
  mlib_ImageSet(src1, src_dtype, src_nch, dw + m, dh + n, slb, sa);

  switch(dst_dtype) {
    default:
      break;
    case MLIB_BYTE:
      stat = mlib_ImageConvMxN_U8(dst1, src1, dkernel, m, n, acmask);
      break;
    case MLIB_SHORT:
      stat = mlib_ImageConvMxN_S16(dst1, src1, dkernel, m, n, acmask);
      break;
    case MLIB_INT:
      stat = mlib_ImageConvMxN_S32(dst1, src1, dkernel, m, n, acmask);
  }

  if (dkernel != akernel) mlib_free(dkernel);

  return stat;
}

/***************************************************************/
