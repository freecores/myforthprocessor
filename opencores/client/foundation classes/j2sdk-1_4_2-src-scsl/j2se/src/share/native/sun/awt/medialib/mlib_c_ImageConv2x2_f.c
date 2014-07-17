/*
 * @(#)mlib_c_ImageConv2x2_f.c	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageConv2x2_f.c	1.5	00/03/28 SMI"
#endif /* __SUNPRO_C */

#ifdef _MSC_VER
#pragma warning(disable:4068 4244 4101)
#endif /*_MSC_VER*/

/*
 * FUNCTION
 *      Internal functions for mlib_ImageConv2x2 on S16/U8 type and MLIB_EDGE_DST_NO_WRITE mask
 *
 */

#include <stdlib.h>
#include <mlib_image.h>

/***************************************************************/

#define LOAD_KERNEL_INTO_DOUBLE()                                         \
  while (scalef_expon > 30) {                                             \
    scalef /= (1 << 30);                                                  \
    scalef_expon -= 30;                                                   \
  }                                                                       \
                                                                          \
  scalef /= (1 << scalef_expon);                                          \
                                                                          \
  /* keep kernel in regs */                                               \
  k0 = scalef * kern[0];  k1 = scalef * kern[1];  k2 = scalef * kern[2];  \
  k3 = scalef * kern[3];

/***************************************************************/

#define GET_SRC_DST_PARAMETERS(type)                    \
  hgt = mlib_ImageGetHeight(src);                       \
  wid = mlib_ImageGetWidth(src);                        \
  nchannel = mlib_ImageGetChannels(src);                \
  sll = mlib_ImageGetStride(src) / sizeof(type);        \
  dll = mlib_ImageGetStride(dst) / sizeof(type);        \
  adr_src = (type *)mlib_ImageGetData(src);             \
  adr_dst = (type *)mlib_ImageGetData(dst);

/***************************************************************/

#ifndef MLIB_USE_FTOI_CLAMPING

#define CLAMP_S32(x)                                                                               \
  (((x) <= MLIB_S32_MIN) ? MLIB_S32_MIN : (((x) >= MLIB_S32_MAX) ? MLIB_S32_MAX : (mlib_s32)(x)))

#else

#define CLAMP_S32(x) ((mlib_s32)(x))

#endif

/***************************************************************/

#ifdef _LITTLE_ENDIAN

#define STORE2(res0, res1)      \
  dp[0    ] = res1;             \
  dp[chan1] = res0

#else

#define STORE2(res0, res1)      \
  dp[0    ] = res0;             \
  dp[chan1] = res1

#endif /* _LITTLE_ENDIAN */

/***************************************************************/

#ifdef _LITTLE_ENDIAN

#define LOAD_BUFF(buff)         \
  buff[i    ] = sp[0];          \
  buff[i + 1] = sp[chan1]

#else

#define LOAD_BUFF(buff)                                                                     \
  *(mlib_s64*)(buff + i) = (((mlib_s64)sp[0]) << 32) | ((mlib_s64)sp[chan1] & 0xffffffff);

#endif /* _LITTLE_ENDIAN */

/***************************************************************/

typedef union {
  mlib_d64 d64;
  struct {
    mlib_s32 i0;
    mlib_s32 i1;
  } i32s;
} d64_2x32;

/***************************************************************/

#define D_KER     1

#define BUFF_LINE 256

/***************************************************************/

#define XOR_80(x) x ^= 0x80

void mlib_ImageXor80_aa(mlib_u8  *dl,
                        mlib_s32 wid,
                        mlib_s32 hgt,
                        mlib_s32 str)
{
  mlib_u8  *dp, *dend;
  mlib_u64 cadd = MLIB_U64_CONST(0x8080808080808080);
  int j;

  if (wid == str) {
    wid *= hgt;
    hgt = 1;
  }

  for (j = 0; j < hgt; j++) {
    dend = dl + wid;

    for (dp = dl; ((mlib_addr)dp & 7) && (dp < dend); dp++) XOR_80(dp[0]);

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; dp <= (dend - 8); dp += 8) {
      *((mlib_s64*)dp) ^= cadd;
    }

    for (; (dp < dend); dp++) XOR_80(dp[0]);

    dl += str;
  }
}

/***************************************************************/

void mlib_ImageXor80(mlib_u8  *dl,
                     mlib_s32 wid,
                     mlib_s32 hgt,
                     mlib_s32 str,
                     mlib_s32 nchan,
                     mlib_s32 cmask)
{
  int i, j, c;

  for (j = 0; j < hgt; j++) {
    for (c = 0; c < nchan; c++) {
      if (cmask & (1 << (nchan - 1 - c))) {
        mlib_u8 *dp = dl + c;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for (i = 0; i < wid; i++) XOR_80(dp[i*nchan]);
      }
    }
    dl += str;
  }
}

/***************************************************************/

#define DTYPE mlib_s16

mlib_status mlib_conv2x2_16nw(mlib_image *dst,
                              mlib_image *src,
                              mlib_s32   *kern,
                              mlib_s32   scalef_expon,
                              mlib_s32   cmask)
{
  mlib_d64 buff_arr[2*BUFF_LINE];
  mlib_s32 *pbuff = (mlib_s32*)buff_arr, *buffo, *buff0, *buff1, *buff2, *buffT;
  DTYPE    *adr_src, *sl, *sp, *sl1;
  DTYPE    *adr_dst, *dl, *dp;
  mlib_d64 k0, k1, k2, k3, scalef = 65536.0;
  mlib_d64 p00, p01, p02,
           p10, p11, p12;
  mlib_s32 wid, hgt, sll, dll, wid1;
  mlib_s32 nchannel, chan1, chan2;
  mlib_s32 i, j, c;
  LOAD_KERNEL_INTO_DOUBLE()
  GET_SRC_DST_PARAMETERS(DTYPE)

  wid1 = (wid + 1) &~ 1;
  if (wid1 > BUFF_LINE) {
    pbuff = mlib_malloc(4*sizeof(mlib_s32)*wid1);
    if (pbuff == NULL) return MLIB_FAILURE;
  }

  buffo = pbuff;
  buff0 = buffo + wid1;
  buff1 = buff0 + wid1;
  buff2 = buff1 + wid1;

  chan1 = nchannel;
  chan2 = chan1 + chan1;

  wid -= D_KER;
  hgt -= D_KER;

  for (c = 0; c < nchannel; c++) {
    if (!(cmask & (1 << (nchannel - 1 - c)))) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    sl1 = sl + sll;
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < wid + D_KER; i++) {
      buff0[i - 1] = (mlib_s32)sl[i*chan1];
      buff1[i - 1] = (mlib_s32)sl1[i*chan1];
    }

    sl += (D_KER + 1)*sll;

    for (j = 0; j < hgt; j++) {
      sp = sl;
      dp = dl;

      buff2[-1] = (mlib_s32)sp[0];
      sp += chan1;

      p02 = buff0[-1];
      p12 = buff1[-1];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        mlib_s64 o64;
        d64_2x32 sd0, sd1, dd;

        p00 = p02; p10 = p12;

        sd0.d64 = *(mlib_d64*)(buff0 + i);
        sd1.d64 = *(mlib_d64*)(buff1 + i);
        p01 = (mlib_d64)sd0.i32s.i0;
        p02 = (mlib_d64)sd0.i32s.i1;
        p11 = (mlib_d64)sd1.i32s.i0;
        p12 = (mlib_d64)sd1.i32s.i1;

        LOAD_BUFF(buff2);

        dd.i32s.i0 = CLAMP_S32(p00 * k0 + p01 * k1 + p10 * k2 + p11 * k3);
        dd.i32s.i1 = CLAMP_S32(p01 * k0 + p02 * k1 + p11 * k2 + p12 * k3);
        *(mlib_d64*)(buffo + i) = dd.d64;

        o64 = *(mlib_s64*)(buffo + i);
        STORE2(o64 >> 48, o64 >> 16);

        sp += chan2;
        dp += chan2;
      }

      for (; i < wid; i++) {
        p00 = buff0[i - 1]; p10 = buff1[i - 1];
        p01 = buff0[i];     p11 = buff1[i];

        buff2[i] = (mlib_s32)sp[0];

        buffo[i] = CLAMP_S32(p00 * k0 + p01 * k1 + p10 * k2 + p11 * k3);
        dp[0] = buffo[i] >> 16;

        sp += chan1;
        dp += chan1;
      }

      sl += sll;
      dl += dll;

      buffT = buff0;
      buff0 = buff1;
      buff1 = buff2;
      buff2 = buffT;
    }
  }

  if (pbuff != (mlib_s32*)buff_arr) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

/***************************************************************/

mlib_status mlib_conv2x2_16ext(mlib_image *dst,
                               mlib_image *src,
                               mlib_s32   dx_l,
                               mlib_s32   dx_r,
                               mlib_s32   dy_t,
                               mlib_s32   dy_b,
                               mlib_s32   *kern,
                               mlib_s32   scalef_expon,
                               mlib_s32   cmask)
{
  mlib_d64 buff_arr[2*BUFF_LINE];
  mlib_s32 *pbuff = (mlib_s32*)buff_arr, *buffo, *buff0, *buff1, *buff2, *buffT;
  DTYPE    *adr_src, *sl, *sp, *sl1;
  DTYPE    *adr_dst, *dl, *dp;
  mlib_d64 k0, k1, k2, k3, scalef = 65536.0;
  mlib_d64 p00, p01, p02,
           p10, p11, p12;
  mlib_s32 wid, hgt, sll, dll, wid1;
  mlib_s32 nchannel, chan1, chan2;
  mlib_s32 i, j, c, swid;
  LOAD_KERNEL_INTO_DOUBLE()
  GET_SRC_DST_PARAMETERS(DTYPE)

  swid = wid + D_KER;

  wid1 = (swid + 1) &~ 1;
  if (wid1 > BUFF_LINE) {
    pbuff = mlib_malloc(4*sizeof(mlib_s32)*wid1);
    if (pbuff == NULL) return MLIB_FAILURE;
  }

  buffo = pbuff;
  buff0 = buffo + wid1;
  buff1 = buff0 + wid1;
  buff2 = buff1 + wid1;

  swid -= dx_r;

  chan1 = nchannel;
  chan2 = chan1 + chan1;

  for (c = 0; c < nchannel; c++) {
    if (!(cmask & (1 << (nchannel - 1 - c)))) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    if ((hgt - dy_b) > 0) sl1 = sl + sll;
    else sl1 = sl;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < swid; i++) {
      buff0[i - 1] = (mlib_s32)sl[i*chan1];
      buff1[i - 1] = (mlib_s32)sl1[i*chan1];
    }

    if (dx_r != 0) {
      buff0[swid - 1] = buff0[swid - 2];
      buff1[swid - 1] = buff1[swid - 2];
    }

    if ((hgt - dy_b) > 1) sl = sl1 + sll;
    else sl = sl1;

    for (j = 0; j < hgt; j++) {
      sp = sl;
      dp = dl;

      buff2[-1] = (mlib_s32)sp[0];
      sp += chan1;

      p02 = buff0[-1];
      p12 = buff1[-1];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        mlib_s64 o64;
        d64_2x32 sd0, sd1, dd;

        p00 = p02; p10 = p12;

        sd0.d64 = *(mlib_d64*)(buff0 + i);
        sd1.d64 = *(mlib_d64*)(buff1 + i);
        p01 = (mlib_d64)sd0.i32s.i0;
        p02 = (mlib_d64)sd0.i32s.i1;
        p11 = (mlib_d64)sd1.i32s.i0;
        p12 = (mlib_d64)sd1.i32s.i1;

        LOAD_BUFF(buff2);

        dd.i32s.i0 = CLAMP_S32(p00 * k0 + p01 * k1 + p10 * k2 + p11 * k3);
        dd.i32s.i1 = CLAMP_S32(p01 * k0 + p02 * k1 + p11 * k2 + p12 * k3);
        *(mlib_d64*)(buffo + i) = dd.d64;

        o64 = *(mlib_s64*)(buffo + i);
        STORE2(o64 >> 48, o64 >> 16);

        sp += chan2;
        dp += chan2;
      }

      for (; i < wid; i++) {
        p00 = buff0[i - 1]; p10 = buff1[i - 1];
        p01 = buff0[i];     p11 = buff1[i];

        buff2[i] = (mlib_s32)sp[0];

        buffo[i] = CLAMP_S32(p00 * k0 + p01 * k1 + p10 * k2 + p11 * k3);
        dp[0] = buffo[i] >> 16;

        sp += chan1;
        dp += chan1;
      }

      if (dx_r != 0) buff2[swid - 1] = buff2[swid - 2];

      if (j < hgt - dy_b - 2) sl += sll;
      dl += dll;

      buffT = buff0;
      buff0 = buff1;
      buff1 = buff2;
      buff2 = buffT;
    }
  }

  if (pbuff != (mlib_s32*)buff_arr) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

/***************************************************************/

#undef  DTYPE
#define DTYPE mlib_u8

mlib_status mlib_conv2x2_8nw(mlib_image *dst,
                             mlib_image *src,
                             mlib_s32   *kern,
                             mlib_s32   scalef_expon,
                             mlib_s32   cmask)
{
  mlib_d64 buff_arr[2*BUFF_LINE];
  mlib_s32 *pbuff = (mlib_s32*)buff_arr, *buffo, *buff0, *buff1, *buff2, *buffT;
  DTYPE    *adr_src, *sl, *sp, *sl1;
  DTYPE    *adr_dst, *dl, *dp;
  mlib_d64 k0, k1, k2, k3, scalef = (1 << 24);
  mlib_d64 p00, p01, p02,
           p10, p11, p12;
  mlib_s32 wid, hgt, sll, dll, wid1;
  mlib_s32 nchannel, chan1, chan2;
  mlib_s32 i, j, c;
  LOAD_KERNEL_INTO_DOUBLE()
  GET_SRC_DST_PARAMETERS(DTYPE)

  wid1 = (wid + 1) &~ 1;
  if (wid1 > BUFF_LINE) {
    pbuff = mlib_malloc(4*sizeof(mlib_s32)*wid1);
    if (pbuff == NULL) return MLIB_FAILURE;
  }

  buffo = pbuff;
  buff0 = buffo + wid1;
  buff1 = buff0 + wid1;
  buff2 = buff1 + wid1;

  chan1 = nchannel;
  chan2 = chan1 + chan1;

  wid -= D_KER;
  hgt -= D_KER;

  for (c = 0; c < nchannel; c++) {
    if (!(cmask & (1 << (nchannel - 1 - c)))) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    sl1 = sl + sll;
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < wid + D_KER; i++) {
      buff0[i - 1] = (mlib_s32)sl[i*chan1];
      buff1[i - 1] = (mlib_s32)sl1[i*chan1];
    }

    sl += (D_KER + 1)*sll;

    for (j = 0; j < hgt; j++) {
      sp = sl;
      dp = dl;

      buff2[-1] = (mlib_s32)sp[0];
      sp += chan1;

      p02 = buff0[-1];
      p12 = buff1[-1];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        mlib_s64 o64;
        d64_2x32 sd0, sd1, dd;

        p00 = p02; p10 = p12;

        sd0.d64 = *(mlib_d64*)(buff0 + i);
        sd1.d64 = *(mlib_d64*)(buff1 + i);
        p01 = (mlib_d64)sd0.i32s.i0;
        p02 = (mlib_d64)sd0.i32s.i1;
        p11 = (mlib_d64)sd1.i32s.i0;
        p12 = (mlib_d64)sd1.i32s.i1;

        LOAD_BUFF(buff2);

        dd.i32s.i0 = CLAMP_S32(p00 * k0 + p01 * k1 + p10 * k2 + p11 * k3 - (1u << 31));
        dd.i32s.i1 = CLAMP_S32(p01 * k0 + p02 * k1 + p11 * k2 + p12 * k3 - (1u << 31));
        *(mlib_d64*)(buffo + i) = dd.d64;

        o64 = *(mlib_s64*)(buffo + i);
        STORE2(o64 >> 56, o64 >> 24);

        sp += chan2;
        dp += chan2;
      }

      for (; i < wid; i++) {
        p00 = buff0[i - 1]; p10 = buff1[i - 1];
        p01 = buff0[i];     p11 = buff1[i];

        buff2[i] = (mlib_s32)sp[0];

        buffo[i] = CLAMP_S32(p00 * k0 + p01 * k1 + p10 * k2 + p11 * k3 - (1u << 31));
        dp[0] = (buffo[i] >> 24);

        sp += chan1;
        dp += chan1;
      }

      sl += sll;
      dl += dll;

      buffT = buff0;
      buff0 = buff1;
      buff1 = buff2;
      buff2 = buffT;
    }
  }

{
    mlib_s32 amask = (1 << nchannel) - 1;
    if ((cmask & amask) != amask) {
      mlib_ImageXor80(adr_dst, wid, hgt, dll, nchannel, cmask);
    } else {
      mlib_ImageXor80_aa(adr_dst, wid*nchannel, hgt, dll);
    }
  }

  if (pbuff != (mlib_s32*)buff_arr) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

/***************************************************************/

mlib_status mlib_conv2x2_8ext(mlib_image *dst,
                              mlib_image *src,
                              mlib_s32   dx_l,
                              mlib_s32   dx_r,
                              mlib_s32   dy_t,
                              mlib_s32   dy_b,
                              mlib_s32   *kern,
                              mlib_s32   scalef_expon,
                              mlib_s32   cmask)
{
  mlib_d64 buff_arr[4*BUFF_LINE];
  mlib_s32 *pbuff = (mlib_s32*)buff_arr, *buffo, *buff0, *buff1, *buff2, *buffT;
  DTYPE    *adr_src, *sl, *sp, *sl1;
  DTYPE    *adr_dst, *dl, *dp;
  mlib_d64 k0, k1, k2, k3, scalef = (1 << 24);
  mlib_d64 p00, p01, p02,
           p10, p11, p12;
  mlib_s32 wid, hgt, sll, dll, wid1;
  mlib_s32 nchannel, chan1, chan2;
  mlib_s32 i, j, c, swid;
  LOAD_KERNEL_INTO_DOUBLE()
  GET_SRC_DST_PARAMETERS(DTYPE)

  swid = wid + D_KER;

  wid1 = (swid + 1) &~ 1;
  if (wid1 > BUFF_LINE) {
    pbuff = mlib_malloc(4*sizeof(mlib_s32)*wid1);
    if (pbuff == NULL) return MLIB_FAILURE;
  }

  buffo = pbuff;
  buff0 = buffo + wid1;
  buff1 = buff0 + wid1;
  buff2 = buff1 + wid1;

  chan1 = nchannel;
  chan2 = chan1 + chan1;

  swid -= dx_r;

  for (c = 0; c < nchannel; c++) {
    if (!(cmask & (1 << (nchannel - 1 - c)))) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    if ((hgt - dy_b) > 0) sl1 = sl + sll;
    else sl1 = sl;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < swid; i++) {
      buff0[i - 1] = (mlib_s32)sl[i*chan1];
      buff1[i - 1] = (mlib_s32)sl1[i*chan1];
    }

    if (dx_r != 0) {
      buff0[swid - 1] = buff0[swid - 2];
      buff1[swid - 1] = buff1[swid - 2];
    }

    if ((hgt - dy_b) > 1) sl = sl1 + sll;
    else sl = sl1;

    for (j = 0; j < hgt; j++) {
      sp = sl;
      dp = dl;

      buff2[-1] = (mlib_s32)sp[0];
      sp += chan1;

      p02 = buff0[-1];
      p12 = buff1[-1];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        mlib_s64 o64;
        d64_2x32 sd0, sd1, dd;

        p00 = p02; p10 = p12;

        sd0.d64 = *(mlib_d64*)(buff0 + i);
        sd1.d64 = *(mlib_d64*)(buff1 + i);
        p01 = (mlib_d64)sd0.i32s.i0;
        p02 = (mlib_d64)sd0.i32s.i1;
        p11 = (mlib_d64)sd1.i32s.i0;
        p12 = (mlib_d64)sd1.i32s.i1;

        LOAD_BUFF(buff2);

        dd.i32s.i0 = CLAMP_S32(p00 * k0 + p01 * k1 + p10 * k2 + p11 * k3 - (1u << 31));
        dd.i32s.i1 = CLAMP_S32(p01 * k0 + p02 * k1 + p11 * k2 + p12 * k3 - (1u << 31));
        *(mlib_d64*)(buffo + i) = dd.d64;

        o64 = *(mlib_s64*)(buffo + i);
        STORE2(o64 >> 56, o64 >> 24);

        sp += chan2;
        dp += chan2;
      }

      for (; i < wid; i++) {
        p00 = buff0[i - 1]; p10 = buff1[i - 1];
        p01 = buff0[i];     p11 = buff1[i];

        buff2[i] = (mlib_s32)sp[0];

        buffo[i] = CLAMP_S32(p00 * k0 + p01 * k1 + p10 * k2 + p11 * k3 - (1u << 31));
        dp[0] = (buffo[i] >> 24);

        sp += chan1;
        dp += chan1;
      }

      if (dx_r != 0) buff2[swid - 1] = buff2[swid - 2];

      if (j < hgt - dy_b - 2) sl += sll;
      dl += dll;

      buffT = buff0;
      buff0 = buff1;
      buff1 = buff2;
      buff2 = buffT;
    }
  }

{
    mlib_s32 amask = (1 << nchannel) - 1;
    if ((cmask & amask) != amask) {
      mlib_ImageXor80(adr_dst, wid, hgt, dll, nchannel, cmask);
    } else {
      mlib_ImageXor80_aa(adr_dst, wid*nchannel, hgt, dll);
    }
  }

  if (pbuff != (mlib_s32*)buff_arr) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

/***************************************************************/
