/*
 * @(#)mlib_ImageConv_16nw.c	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageConv_16nw.c	1.10	00/04/17 SMI"
#endif /* __SUNPRO_C */

#ifdef _MSC_VER
#pragma warning(disable:4068 4244 4101)
#endif /*_MSC_VER*/

/*
 * FUNCTION
 *   Internal functions for mlib_ImageConv* on S16/U8 type and
 *   MLIB_EDGE_DST_NO_WRITE mask
 */

/***************************************************************/

#include <stdlib.h>
#include <mlib_image.h>

/*
  This define switches between functions of S16 and U8 types:
  Files mlib_ImageConv_16nw.c and mlib_c_ImageConv_8nw.c
*/
#define TYPE_SHORT

/***************************************************************/

#define BUFF_SIZE   1600

#define CACHE_SIZE  (64*1024)

/***************************************************************/

#ifdef TYPE_SHORT

#define DTYPE mlib_s16

#define CONV_FUNC(KERN) mlib_conv##KERN##_16nw

#define DSCALE       65536.0

#define FROM_S32(x)  ((x) >> 16)

#define S64TOS32(x)  ((x) & 0xffffffff)

#define SAT_OFF

#else

#define DTYPE mlib_u8

#define CONV_FUNC(KERN) mlib_conv##KERN##_8nw

#define DSCALE       (1 << 24)

#define FROM_S32(x)  (((x) >> 24) ^ 128)

#define S64TOS32(x)  (x)

#define SAT_OFF      - (1u << 31)

#endif

/***************************************************************/

#define FTYPE mlib_d64

#ifndef MLIB_USE_FTOI_CLAMPING

#define CLAMP_S32(x)                                                                               \
  (((x) <= MLIB_S32_MIN) ? MLIB_S32_MIN : (((x) >= MLIB_S32_MAX) ? MLIB_S32_MAX : (mlib_s32)(x)))

#else

#define CLAMP_S32(x) ((mlib_s32)(x))

#endif

/***************************************************************/

#define D2I(x) CLAMP_S32((x) SAT_OFF)

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

#define LOAD_BUFF(buff)                                                                \
  *(mlib_s64*)(buff + i) = (((mlib_s64)sp[0]) << 32) | S64TOS32((mlib_s64)sp[chan1]);

#endif /* _LITTLE_ENDIAN */

/***************************************************************/

typedef union {
  mlib_d64 d64;
  struct {
    mlib_s32 i0;
    mlib_s32 i1;
  } i32s;
  struct {
    mlib_s32 f0;
    mlib_s32 f1;
  } f32s;
} d64_2x32;

/***************************************************************/

#define BUFF_LINE 256

/***************************************************************/

#define DEF_VARS(type)                  \
  type     *adr_src, *sl, *sp, *sl1;    \
  type     *adr_dst, *dl, *dp;          \
  FTYPE    *pbuff = buff;               \
  mlib_s32 *buffi, *buffo;              \
  mlib_s32 wid, hgt, sll, dll;          \
  mlib_s32 nchannel, chan1, chan2;      \
  mlib_s32 i, j, c;

/***************************************************************/

#define LOAD_KERNEL3()                                                    \
  FTYPE    scalef = DSCALE;                                               \
  FTYPE    k0, k1, k2, k3, k4, k5, k6, k7, k8;                            \
  FTYPE    p00, p01, p02, p03,                                            \
           p10, p11, p12, p13,                                            \
           p20, p21, p22, p23;                                            \
                                                                          \
  while (scalef_expon > 30) {                                             \
    scalef /= (1 << 30);                                                  \
    scalef_expon -= 30;                                                   \
  }                                                                       \
                                                                          \
  scalef /= (1 << scalef_expon);                                          \
                                                                          \
  /* keep kernel in regs */                                               \
  k0 = scalef * kern[0];  k1 = scalef * kern[1];  k2 = scalef * kern[2];  \
  k3 = scalef * kern[3];  k4 = scalef * kern[4];  k5 = scalef * kern[5];  \
  k6 = scalef * kern[6];  k7 = scalef * kern[7];  k8 = scalef * kern[8];

/***************************************************************/

#define LOAD_KERNEL(SIZE)                               \
  FTYPE    scalef = DSCALE;                             \
                                                        \
  while (scalef_expon > 30) {                           \
    scalef /= (1 << 30);                                \
    scalef_expon -= 30;                                 \
  }                                                     \
                                                        \
  scalef /= (1 << scalef_expon);                        \
                                                        \
  for (j = 0; j < SIZE; j++) k[j] = scalef * kern[j]

/***************************************************************/

#define GET_SRC_DST_PARAMETERS(type)                    \
    hgt = mlib_ImageGetHeight(src);                     \
  wid = mlib_ImageGetWidth(src);                        \
  nchannel = mlib_ImageGetChannels(src);                \
  sll = mlib_ImageGetStride(src) / sizeof(type);        \
  dll = mlib_ImageGetStride(dst) / sizeof(type);        \
  adr_src = (type *)mlib_ImageGetData(src);             \
  adr_dst = (type *)mlib_ImageGetData(dst);

/***************************************************************/

#ifndef __sparc

#ifdef TYPE_SHORT

#define CLAMP_STORE(dst, val)           \
  if (val >= MLIB_S16_MAX)              \
    dst = MLIB_S16_MAX;                 \
  else if (val <= MLIB_S16_MIN)         \
    dst = MLIB_S16_MIN;                 \
  else                                  \
    dst = (mlib_s16)val;

#else

/* Test for the presence of any "1" bit in bits
   8 to 31 of val. If present, then val is either
   negative or >255. If over/underflows of 8 bits
   are uncommon, then this technique can be a win,
   since only a single test, rather than two, is
   necessary to determine if clamping is needed.
   On the other hand, if over/underflows are common,
   it adds an extra test.
*/
#define CLAMP_STORE(dst, val)   \
  if (val & 0xffffff00) {       \
    if (val < MLIB_U8_MIN)      \
      dst = MLIB_U8_MIN;        \
    else                        \
      dst = MLIB_U8_MAX;        \
  } else {                      \
    dst = (mlib_u8)val;         \
  }

#endif

#endif

/***************************************************************/

void mlib_ImageXor80(mlib_u8  *dl,
                     mlib_s32 wid,
                     mlib_s32 hgt,
                     mlib_s32 str,
                     mlib_s32 nchan,
                     mlib_s32 cmask);

void mlib_ImageXor80_aa(mlib_u8  *dl,
                        mlib_s32 wid,
                        mlib_s32 hgt,
                        mlib_s32 str);

/***************************************************************/

#define KSIZE  3

#ifdef __sparc  /* for SPARC, using floating-point multiplies is faster */

mlib_status CONV_FUNC(3x3)(mlib_image *dst,
                           mlib_image *src,
                           mlib_s32   *kern,
                           mlib_s32   scalef_expon,
                           mlib_s32   cmask)
{
  FTYPE    buff[(KSIZE + 2)*BUFF_LINE], *buff0, *buff1, *buff2, *buff3, *buffT;
  DEF_VARS(DTYPE)
  DTYPE *sl2;
  LOAD_KERNEL3()
  GET_SRC_DST_PARAMETERS(DTYPE)

  if (wid > BUFF_LINE) {
    pbuff = mlib_malloc((KSIZE + 2)*sizeof(FTYPE)*wid);
    if (pbuff == NULL) return MLIB_FAILURE;
  }

  buff0 = pbuff;
  buff1 = buff0 + wid;
  buff2 = buff1 + wid;
  buff3 = buff2 + wid;
  buffo = (mlib_s32*)(buff3 + wid);
  buffi = buffo + (wid &~ 1);

  chan1 = nchannel;
  chan2 = chan1 + chan1;

  wid -= (KSIZE - 1);
  hgt -= (KSIZE - 1);

  adr_dst += ((KSIZE - 1)/2)*(dll + chan1);

  for (c = 0; c < nchannel; c++) {
    if (!(cmask & (1 << (nchannel - 1 - c)))) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    sl1 = sl  + sll;
    sl2 = sl1 + sll;
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < wid + (KSIZE - 1); i++) {
      buff0[i] = (FTYPE)sl[i*chan1];
      buff1[i] = (FTYPE)sl1[i*chan1];
      buff2[i] = (FTYPE)sl2[i*chan1];
    }

    sl += KSIZE*sll;

    for (j = 0; j < hgt; j++) {
      FTYPE    s0, s1;

      p02 = buff0[0];
      p12 = buff1[0];
      p22 = buff2[0];

      p03 = buff0[1];
      p13 = buff1[1];
      p23 = buff2[1];

      s0 = p02 * k0 + p03 * k1 + p12 * k3 + p13 * k4 + p22 * k6 + p23 * k7;
      s1 = p03 * k0 + p13 * k3 + p23 * k6;

      sp = sl;
      dp = dl;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        mlib_s64 o64;
        d64_2x32 dd;

        p02 = buff0[i + 2]; p12 = buff1[i + 2]; p22 = buff2[i + 2];
        p03 = buff0[i + 3]; p13 = buff1[i + 3]; p23 = buff2[i + 3];

        LOAD_BUFF(buffi);

        dd.d64 = *(FTYPE   *)(buffi + i);
        buff3[i    ] = (FTYPE)dd.i32s.i0;
        buff3[i + 1] = (FTYPE)dd.i32s.i1;

        dd.i32s.i0 = D2I(s0 + p02 * k2 + p12 * k5 + p22 * k8);
        dd.i32s.i1 = D2I(s1 + p02 * k1 + p03 * k2 + p12 * k4 + p13 * k5 + p22 * k7 + p23 * k8);
        *(FTYPE   *)(buffo + i) = dd.d64;

        s0 = p02 * k0 + p03 * k1 + p12 * k3 + p13 * k4 + p22 * k6 + p23 * k7;
        s1 = p03 * k0 + p13 * k3 + p23 * k6;

        o64 = *(mlib_s64*)(buffo + i);
#ifdef TYPE_SHORT
        STORE2(FROM_S32(o64 >> 32), FROM_S32(o64));
#else
        STORE2(o64 >> 56, o64 >> 24);
#endif

        sp += chan2;
        dp += chan2;
      }

      for (; i < wid; i++) {
        p00 = buff0[i];     p10 = buff1[i];     p20 = buff2[i];
        p01 = buff0[i + 1]; p11 = buff1[i + 1]; p21 = buff2[i + 1];
        p02 = buff0[i + 2]; p12 = buff1[i + 2]; p22 = buff2[i + 2];

        buffi[i] = (mlib_s32)sp[0];
        buff3[i] = (FTYPE)buffi[i];

        buffo[i] = D2I(p00 * k0 + p01 * k1 + p02 * k2 + p10 * k3 + p11 * k4 +
                       p12 * k5 + p20 * k6 + p21 * k7 + p22 * k8);
#ifdef TYPE_SHORT
        dp[0] = FROM_S32(buffo[i]);
#else
        dp[0] = buffo[i] >> 24;
#endif

        sp += chan1;
        dp += chan1;
      }

      buffi[wid] = (mlib_s32)sp[0];
      buff3[wid] = (FTYPE)buffi[wid];
      buffi[wid + 1] = (mlib_s32)sp[chan1];
      buff3[wid + 1] = (FTYPE)buffi[wid + 1];

      sl += sll;
      dl += dll;

      buffT = buff0;
      buff0 = buff1;
      buff1 = buff2;
      buff2 = buff3;
      buff3 = buffT;
    }
  }

#ifndef TYPE_SHORT
{
    mlib_s32 amask = (1 << nchannel) - 1;
    if ((cmask & amask) != amask) {
      mlib_ImageXor80(adr_dst, wid, hgt, dll, nchannel, cmask);
    } else {
      mlib_ImageXor80_aa(adr_dst, wid*nchannel, hgt, dll);
    }
  }
#endif

  if (pbuff != buff) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

/***************************************************************/

#else    /* for x86, using integer multiplies is faster */

#ifdef __ICL

#ifdef TYPE_SHORT
static mlib_status mlib_conv3x3_16nw_1chan(mlib_image *dst,
#else
       static mlib_status mlib_conv3x3_8nw_1chan(mlib_image *dst,
#endif
                                                 mlib_image *src,
                                                 mlib_s32 *kernel,
                                                 mlib_s32 scale);

#endif /* __ICL */

mlib_status CONV_FUNC(3x3)(mlib_image *dst,
                           mlib_image *src,
                           mlib_s32   *kern,
                           mlib_s32   scalef_expon,
                           mlib_s32   cmask)
{
  DTYPE    *adr_src, *sl, *sp0, *sp1, *sp2;
  DTYPE    *adr_dst, *dl, *dp;
  mlib_s32 wid, hgt, sll, dll;
  mlib_s32 nchannel, chan1, chan2;
  mlib_s32 i, j, c;
  mlib_s32 shift1, shift2;
  mlib_s32 k0, k1, k2, k3, k4, k5, k6, k7, k8;
  mlib_s32 p02, p03,
           p12, p13,
           p22, p23;

#ifdef TYPE_SHORT

#ifdef __ICL
{
    mlib_u8 *src_data = mlib_ImageGetData(src);
    mlib_u8 *dst_data   = mlib_ImageGetData(dst);
    int     src_stride  = mlib_ImageGetStride(src);
    int     dst_stride  = mlib_ImageGetStride(dst);

    nchannel = mlib_ImageGetChannels(src);

    if ((nchannel == 1) && (cmask != 0) &&
        (((mlib_addr)src_data & 0x7) == 0) &&
        (((mlib_addr)dst_data & 0x7) == 0) &&
        ((src_stride & 0x7) == 0) &&
        ((dst_stride & 0x7) == 0))
    {
        return mlib_conv3x3_16nw_1chan(dst, src, kern, scalef_expon);
      }
  }
#endif /* __ICL */

  shift1 = 16;
#else

#ifdef __ICL
{
    mlib_u8 *src_data = mlib_ImageGetData(src);
    mlib_u8 *dst_data   = mlib_ImageGetData(dst);
    int     src_stride  = mlib_ImageGetStride(src);
    int     dst_stride  = mlib_ImageGetStride(dst);

    nchannel = mlib_ImageGetChannels(src);

    if ((nchannel == 1) && (cmask != 0) &&
        (((mlib_addr)src_data & 0x7) == 0) &&
        (((mlib_addr)dst_data & 0x7) == 0) &&
        ((src_stride & 0x7) == 0) &&
        ((dst_stride & 0x7) == 0))
    {
        return mlib_conv3x3_8nw_1chan(dst, src, kern, scalef_expon);
      }
  }
#endif /* __ICL */

  shift1 = 8;
#endif /* TYPE_SHORT */

  shift2 = scalef_expon - shift1;

  /* keep kernel in regs */
  k0 = kern[0] >> shift1;  k1 = kern[1] >> shift1;  k2 = kern[2] >> shift1;
  k3 = kern[3] >> shift1;  k4 = kern[4] >> shift1;  k5 = kern[5] >> shift1;
  k6 = kern[6] >> shift1;  k7 = kern[7] >> shift1;  k8 = kern[8] >> shift1;

  GET_SRC_DST_PARAMETERS(DTYPE)

  chan1 = nchannel;
  chan2 = chan1 + chan1;

  wid -= (KSIZE - 1);
  hgt -= (KSIZE - 1);

  adr_dst += ((KSIZE - 1)/2)*(dll + chan1);

  for (c = 0; c < chan1; c++) {
    if (!(cmask & (1 << (chan1 - 1 - c)))) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    for (j = 0; j < hgt; j++) {
      mlib_s32 s0, s1;
      mlib_s32 pix0, pix1;

      dp  = dl;
      sp0 = sl;
      sp1 = sp0 + sll;
      sp2 = sp1 + sll;

      p02 = sp0[0];
      p12 = sp1[0];
      p22 = sp2[0];

      p03 = sp0[chan1];
      p13 = sp1[chan1];
      p23 = sp2[chan1];

      s0 = p02 * k0 + p03 * k1 + p12 * k3 + p13 * k4 + p22 * k6 + p23 * k7;
      s1 = p03 * k0 + p13 * k3 + p23 * k6;

      sp0 += chan2;
      sp1 += chan2;
      sp2 += chan2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        p02 = sp0[0];     p12 = sp1[0];     p22 = sp2[0];
        p03 = sp0[chan1]; p13 = sp1[chan1]; p23 = sp2[chan1];

        pix0 = (s0 + p02 * k2 + p12 * k5 + p22 * k8) >> shift2;
        pix1 = (s1 + p02 * k1 + p03 * k2 + p12 * k4 +
                p13 * k5 + p22 * k7 + p23 * k8) >> shift2;

        CLAMP_STORE(dp[0],     pix0)
        CLAMP_STORE(dp[chan1], pix1)

        s0 = p02 * k0 + p03 * k1 + p12 * k3 + p13 * k4 + p22 * k6 + p23 * k7;
        s1 = p03 * k0 + p13 * k3 + p23 * k6;

        sp0 += chan2;
        sp1 += chan2;
        sp2 += chan2;
        dp += chan2;
      }

      if (wid & 1) {
        p02 = sp0[0]; p12 = sp1[0]; p22 = sp2[0];
        pix0 = (s0 + p02 * k2 + p12 * k5 + p22 * k8) >> shift2;
        CLAMP_STORE(dp[0], pix0)
      }

      sl += sll;
      dl += dll;
    }
  }

  return MLIB_SUCCESS;
}

#endif  /* #ifdef __sparc */

/***************************************************************/

#undef  KSIZE
#define KSIZE 4

mlib_status CONV_FUNC(4x4)(mlib_image *dst,
                           mlib_image *src,
                           mlib_s32   *kern,
                           mlib_s32   scalef_expon,
                           mlib_s32   cmask)
{
  FTYPE    buff[(KSIZE + 3)*BUFF_LINE];
  FTYPE    *buff0, *buff1, *buff2, *buff3, *buff4, *buffd, *buffT;
  FTYPE    k[KSIZE*KSIZE];
  mlib_s32 d0, d1;
  FTYPE    k0, k1, k2, k3, k4, k5, k6, k7;
  FTYPE    p00, p01, p02, p03, p04,
           p10, p11, p12, p13, p14,
           p20, p21, p22, p23,
           p30, p31, p32, p33;
  DEF_VARS(DTYPE)
  DTYPE *sl2, *sl3;
  LOAD_KERNEL(KSIZE*KSIZE);
  GET_SRC_DST_PARAMETERS(DTYPE)

  if (wid > BUFF_LINE) {
    pbuff = mlib_malloc((KSIZE + 3)*sizeof(FTYPE)*wid);
    if (pbuff == NULL) return MLIB_FAILURE;
  }

  buff0 = pbuff;
  buff1 = buff0 + wid;
  buff2 = buff1 + wid;
  buff3 = buff2 + wid;
  buff4 = buff3 + wid;
  buffd = buff4 + wid;
  buffo = (mlib_s32*)(buffd + wid);
  buffi = buffo + (wid &~ 1);

  chan1 = nchannel;
  chan2 = chan1 + chan1;

  wid -= (KSIZE - 1);
  hgt -= (KSIZE - 1);

  adr_dst += ((KSIZE - 1)/2)*(dll + chan1);

  for (c = 0; c < nchannel; c++) {
    if (!(cmask & (1 << (nchannel - 1 - c)))) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    sl1 = sl  + sll;
    sl2 = sl1 + sll;
    sl3 = sl2 + sll;
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < wid + (KSIZE - 1); i++) {
      buff0[i] = (FTYPE)sl[i*chan1];
      buff1[i] = (FTYPE)sl1[i*chan1];
      buff2[i] = (FTYPE)sl2[i*chan1];
      buff3[i] = (FTYPE)sl3[i*chan1];
    }

    sl += KSIZE*sll;

    for (j = 0; j < hgt; j++) {
      d64_2x32 dd;

      /*
       *  First loop on two first lines of kernel
       */
      k0 = k[0]; k1 = k[1]; k2 = k[2]; k3 = k[3];
      k4 = k[4]; k5 = k[5]; k6 = k[6]; k7 = k[7];

      sp = sl;
      dp = dl;

      p02 = buff0[0];
      p12 = buff1[0];
      p03 = buff0[1];
      p13 = buff1[1];
      p04 = buff0[2];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        p00 = p02; p10 = p12;
        p01 = p03; p11 = p13;
        p02 = p04; p12 = buff1[i + 2];
        p03 = buff0[i + 3]; p13 = buff1[i + 3];
        p04 = buff0[i + 4]; p14 = buff1[i + 4];

        LOAD_BUFF(buffi);

        dd.d64 = *(FTYPE   *)(buffi + i);
        buff4[i    ] = (FTYPE)dd.i32s.i0;
        buff4[i + 1] = (FTYPE)dd.i32s.i1;

        buffd[i    ] = (p00 * k0 + p01 * k1 + p02 * k2 + p03 * k3 +
                        p10 * k4 + p11 * k5 + p12 * k6 + p13 * k7);
        buffd[i + 1] = (p01 * k0 + p02 * k1 + p03 * k2 + p04 * k3 +
                        p11 * k4 + p12 * k5 + p13 * k6 + p14 * k7);

        sp += chan2;
        dp += chan2;
      }

      /*
       *  Second loop on two last lines of kernel
       */
      k0 = k[ 8]; k1 = k[ 9]; k2 = k[10]; k3 = k[11];
      k4 = k[12]; k5 = k[13]; k6 = k[14]; k7 = k[15];

      sp = sl;
      dp = dl;

      p02 = buff2[0];
      p12 = buff3[0];
      p03 = buff2[1];
      p13 = buff3[1];
      p04 = buff2[2];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        p00 = p02; p10 = p12;
        p01 = p03; p11 = p13;
        p02 = p04; p12 = buff3[i + 2];
        p03 = buff2[i + 3]; p13 = buff3[i + 3];
        p04 = buff2[i + 4]; p14 = buff3[i + 4];

        d0 = D2I(p00 * k0 + p01 * k1 + p02 * k2 + p03 * k3 +
                 p10 * k4 + p11 * k5 + p12 * k6 + p13 * k7 + buffd[i]);
        d1 = D2I(p01 * k0 + p02 * k1 + p03 * k2 + p04 * k3 +
                 p11 * k4 + p12 * k5 + p13 * k6 + p14 * k7 + buffd[i + 1]);

        dp[0    ] = FROM_S32(d0);
        dp[chan1] = FROM_S32(d1);

        sp += chan2;
        dp += chan2;
      }

      /* last pixels */
      for (; i < wid; i++) {
        p00 = buff0[i];     p10 = buff1[i];     p20 = buff2[i];     p30 = buff3[i];
        p01 = buff0[i + 1]; p11 = buff1[i + 1]; p21 = buff2[i + 1]; p31 = buff3[i + 1];
        p02 = buff0[i + 2]; p12 = buff1[i + 2]; p22 = buff2[i + 2]; p32 = buff3[i + 2];
        p03 = buff0[i + 3]; p13 = buff1[i + 3]; p23 = buff2[i + 3]; p33 = buff3[i + 3];

        buff4[i] = (FTYPE)sp[0];

        buffo[i] = D2I(p00 * k[0] + p01 * k[1] + p02 * k[2] + p03 * k[3] +
                       p10 * k[4] + p11 * k[5] + p12 * k[6] + p13 * k[7] +
                       p20 * k[ 8] + p21 * k[ 9] + p22 * k[10] + p23 * k[11] +
                       p30 * k[12] + p31 * k[13] + p32 * k[14] + p33 * k[15]);

        dp[0] = FROM_S32(buffo[i]);

        sp += chan1;
        dp += chan1;
      }

      buff4[wid    ] = (FTYPE)sp[0];
      buff4[wid + 1] = (FTYPE)sp[chan1];
      buff4[wid + 2] = (FTYPE)sp[chan2];

      /* next line */
      sl += sll;
      dl += dll;

      buffT = buff0;
      buff0 = buff1;
      buff1 = buff2;
      buff2 = buff3;
      buff3 = buff4;
      buff4 = buffT;
    }
  }

  if (pbuff != buff) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

/***************************************************************/

#undef  KSIZE
#define KSIZE 5

#ifdef __sparc  /* for SPARC, using floating-point multiplies is faster */

mlib_status CONV_FUNC(5x5)(mlib_image *dst,
                           mlib_image *src,
                           mlib_s32   *kern,
                           mlib_s32   scalef_expon,
                           mlib_s32   cmask)
{
  FTYPE    buff[(KSIZE + 3)*BUFF_LINE];
  FTYPE    *buff0, *buff1, *buff2, *buff3, *buff4, *buff5, *buffd, *buffT;
  FTYPE    k[KSIZE*KSIZE];
  mlib_s32 d0, d1;
  FTYPE    k0, k1, k2, k3, k4, k5, k6, k7, k8, k9;
  FTYPE    p00, p01, p02, p03, p04, p05,
           p10, p11, p12, p13, p14, p15,
           p20, p21, p22, p23, p24,
           p30, p31, p32, p33, p34,
           p40, p41, p42, p43, p44;
  DEF_VARS(DTYPE)
  DTYPE *sl2, *sl3, *sl4;
  LOAD_KERNEL(KSIZE*KSIZE);
  GET_SRC_DST_PARAMETERS(DTYPE)

  if (wid > BUFF_LINE) {
    pbuff = mlib_malloc((KSIZE + 3)*sizeof(FTYPE)*wid);
    if (pbuff == NULL) return MLIB_FAILURE;
  }

  buff0 = pbuff;
  buff1 = buff0 + wid;
  buff2 = buff1 + wid;
  buff3 = buff2 + wid;
  buff4 = buff3 + wid;
  buff5 = buff4 + wid;
  buffd = buff5 + wid;
  buffo = (mlib_s32*)(buffd + wid);
  buffi = buffo + (wid &~ 1);

  chan1 = nchannel;
  chan2 = chan1 + chan1;

  wid -= (KSIZE - 1);
  hgt -= (KSIZE - 1);

  adr_dst += ((KSIZE - 1)/2)*(dll + chan1);

  for (c = 0; c < nchannel; c++) {
    if (!(cmask & (1 << (nchannel - 1 - c)))) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    sl1 = sl  + sll;
    sl2 = sl1 + sll;
    sl3 = sl2 + sll;
    sl4 = sl3 + sll;
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < wid + (KSIZE - 1); i++) {
      buff0[i] = (FTYPE)sl[i*chan1];
      buff1[i] = (FTYPE)sl1[i*chan1];
      buff2[i] = (FTYPE)sl2[i*chan1];
      buff3[i] = (FTYPE)sl3[i*chan1];
      buff4[i] = (FTYPE)sl4[i*chan1];
    }

    sl += KSIZE*sll;

    for (j = 0; j < hgt; j++) {
      d64_2x32 dd;

      /*
       *  First loop
       */
      k0 = k[0]; k1 = k[1]; k2 = k[2]; k3 = k[3]; k4 = k[4];
      k5 = k[5]; k6 = k[6]; k7 = k[7]; k8 = k[8]; k9 = k[9];

      sp = sl;
      dp = dl;

      p02 = buff0[0];
      p12 = buff1[0];
      p03 = buff0[1];
      p13 = buff1[1];
      p04 = buff0[2];
      p14 = buff1[2];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        p00 = p02; p10 = p12;
        p01 = p03; p11 = p13;
        p02 = p04; p12 = p14;

        LOAD_BUFF(buffi);

        p03 = buff0[i + 3]; p13 = buff1[i + 3];
        p04 = buff0[i + 4]; p14 = buff1[i + 4];
        p05 = buff0[i + 5]; p15 = buff1[i + 5];

        buffd[i    ] = (p00 * k0 + p01 * k1 + p02 * k2 + p03 * k3 + p04 * k4 +
                        p10 * k5 + p11 * k6 + p12 * k7 + p13 * k8 + p14 * k9);
        buffd[i + 1] = (p01 * k0 + p02 * k1 + p03 * k2 + p04 * k3 + p05 * k4 +
                        p11 * k5 + p12 * k6 + p13 * k7 + p14 * k8 + p15 * k9);

        sp += chan2;
        dp += chan2;
      }

      /*
       *  Second loop
       */
      k0 = k[10]; k1 = k[11]; k2 = k[12]; k3 = k[13]; k4 = k[14];
      k5 = k[15]; k6 = k[16]; k7 = k[17]; k8 = k[18]; k9 = k[19];

      sp = sl;
      dp = dl;

      p02 = buff2[0];
      p12 = buff3[0];
      p03 = buff2[1];
      p13 = buff3[1];
      p04 = buff2[2];
      p14 = buff3[2];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        p00 = p02; p10 = p12;
        p01 = p03; p11 = p13;

        p02 = buff2[i + 2]; p12 = buff3[i + 2];
        p03 = buff2[i + 3]; p13 = buff3[i + 3];
        p04 = buff2[i + 4]; p14 = buff3[i + 4];
        p05 = buff2[i + 5]; p15 = buff3[i + 5];

        dd.d64 = *(FTYPE   *)(buffi + i);
        buff5[i    ] = (FTYPE)dd.i32s.i0;
        buff5[i + 1] = (FTYPE)dd.i32s.i1;

        buffd[i    ] += (p00 * k0 + p01 * k1 + p02 * k2 + p03 * k3 + p04 * k4 +
                         p10 * k5 + p11 * k6 + p12 * k7 + p13 * k8 + p14 * k9);
        buffd[i + 1] += (p01 * k0 + p02 * k1 + p03 * k2 + p04 * k3 + p05 * k4 +
                         p11 * k5 + p12 * k6 + p13 * k7 + p14 * k8 + p15 * k9);

        sp += chan2;
        dp += chan2;
      }

      /*
       *  3 loop
       */
      k0 = k[20]; k1 = k[21]; k2 = k[22]; k3 = k[23]; k4 = k[24];

      sp = sl;
      dp = dl;

      p02 = buff4[0];
      p03 = buff4[1];
      p04 = buff4[2];
      p05 = buff4[3];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        p00 = p02; p01 = p03; p02 = p04; p03 = p05;

        p04 = buff4[i + 4]; p05 = buff4[i + 5];

        d0 = D2I(p00 * k0 + p01 * k1 + p02 * k2 + p03 * k3 + p04 * k4 + buffd[i]);
        d1 = D2I(p01 * k0 + p02 * k1 + p03 * k2 + p04 * k3 + p05 * k4 + buffd[i + 1]);

        dp[0    ] = FROM_S32(d0);
        dp[chan1] = FROM_S32(d1);

        sp += chan2;
        dp += chan2;
      }

      /* last pixels */
      for (; i < wid; i++) {
        p00 = buff0[i];     p10 = buff1[i];     p20 = buff2[i];     p30 = buff3[i];
        p01 = buff0[i + 1]; p11 = buff1[i + 1]; p21 = buff2[i + 1]; p31 = buff3[i + 1];
        p02 = buff0[i + 2]; p12 = buff1[i + 2]; p22 = buff2[i + 2]; p32 = buff3[i + 2];
        p03 = buff0[i + 3]; p13 = buff1[i + 3]; p23 = buff2[i + 3]; p33 = buff3[i + 3];
        p04 = buff0[i + 4]; p14 = buff1[i + 4]; p24 = buff2[i + 4]; p34 = buff3[i + 4];

        p40 = buff4[i];     p41 = buff4[i + 1]; p42 = buff4[i + 2];
        p43 = buff4[i + 3]; p44 = buff4[i + 4];

        buff5[i] = (FTYPE)sp[0];

        buffo[i] = D2I(p00 * k[0] + p01 * k[1] + p02 * k[2] + p03 * k[3] + p04 * k[4] +
                       p10 * k[5] + p11 * k[6] + p12 * k[7] + p13 * k[8] + p14 * k[9] +
                       p20 * k[10] + p21 * k[11] + p22 * k[12] + p23 * k[13] + p24 * k[14] +
                       p30 * k[15] + p31 * k[16] + p32 * k[17] + p33 * k[18] + p34 * k[19] +
                       p40 * k[20] + p41 * k[21] + p42 * k[22] + p43 * k[23] + p44 * k[24]);

        dp[0] = FROM_S32(buffo[i]);

        sp += chan1;
        dp += chan1;
      }

      buff5[wid    ] = (FTYPE)sp[0];
      buff5[wid + 1] = (FTYPE)sp[chan1];
      buff5[wid + 2] = (FTYPE)sp[chan2];
      buff5[wid + 3] = (FTYPE)sp[chan2 + chan1];

      /* next line */
      sl += sll;
      dl += dll;

      buffT = buff0;
      buff0 = buff1;
      buff1 = buff2;
      buff2 = buff3;
      buff3 = buff4;
      buff4 = buff5;
      buff5 = buffT;
    }
  }

  if (pbuff != buff) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

/***************************************************************/

#else    /* for x86, using integer multiplies is faster */

mlib_status CONV_FUNC(5x5)(mlib_image *dst,
                           mlib_image *src,
                           mlib_s32   *kern,
                           mlib_s32   scalef_expon,
                           mlib_s32   cmask)
{
  mlib_s32 buff[BUFF_LINE];
  mlib_s32 *buffd;
  mlib_s32 k[KSIZE*KSIZE];
  mlib_s32 d0, d1;
  mlib_s32 shift1, shift2;
  mlib_s32 k0, k1, k2, k3, k4, k5, k6, k7, k8, k9;
  mlib_s32 p00, p01, p02, p03, p04, p05,
           p10, p11, p12, p13, p14, p15,
           p20, p21, p22, p23, p24,
           p30, p31, p32, p33, p34,
           p40, p41, p42, p43, p44;
  DTYPE    *adr_src, *sl, *sp0, *sp1;
  DTYPE    *adr_dst, *dl, *dp;
  mlib_s32 *pbuff = buff;
  mlib_s32 wid, hgt, sll, dll;
  mlib_s32 nchannel, chan1, chan2, chan3, chan4;
  mlib_s32 i, j, c;

#ifdef TYPE_SHORT
  shift1 = 16;
#else
  shift1 = 8;
#endif

  shift2 = scalef_expon - shift1;

  for (j = 0; j < KSIZE*KSIZE; j++) k[j] = kern[j] >> shift1;

  GET_SRC_DST_PARAMETERS(DTYPE)

  if (wid > BUFF_LINE) {
    pbuff = mlib_malloc(sizeof(mlib_s32)*wid);
    if (pbuff == NULL) return MLIB_FAILURE;
  }

  buffd = pbuff;

  chan1 = nchannel;
  chan2 = chan1 + chan1;
  chan3 = chan2 + chan1;
  chan4 = chan3 + chan1;

  wid -= (KSIZE - 1);
  hgt -= (KSIZE - 1);

  adr_dst += ((KSIZE - 1)/2)*(dll + chan1);

  for (c = 0; c < chan1; c++) {
    if (!(cmask & (1 << (chan1 - 1 - c)))) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    for (j = 0; j < hgt; j++) {
      mlib_s32 pix0, pix1;
      /*
       *  First loop
       */
      sp0 = sl;
      sp1 = sp0 + sll;
      dp = dl;

      k0 = k[0]; k1 = k[1]; k2 = k[2]; k3 = k[3]; k4 = k[4];
      k5 = k[5]; k6 = k[6]; k7 = k[7]; k8 = k[8]; k9 = k[9];

      p02 = sp0[0];     p12 = sp1[0];
      p03 = sp0[chan1]; p13 = sp1[chan1];
      p04 = sp0[chan2]; p14 = sp1[chan2];
      p05 = sp0[chan3]; p15 = sp1[chan3];

      sp0 += chan4;
      sp1 += chan4;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        p00 = p02; p10 = p12;
        p01 = p03; p11 = p13;
        p02 = p04; p12 = p14;
        p03 = p05; p13 = p15;

        p04 = sp0[0];     p14 = sp1[0];
        p05 = sp0[chan1]; p15 = sp1[chan1];

        buffd[i    ] = (p00 * k0 + p01 * k1 + p02 * k2 + p03 * k3 + p04 * k4 +
                        p10 * k5 + p11 * k6 + p12 * k7 + p13 * k8 + p14 * k9);
        buffd[i + 1] = (p01 * k0 + p02 * k1 + p03 * k2 + p04 * k3 + p05 * k4 +
                        p11 * k5 + p12 * k6 + p13 * k7 + p14 * k8 + p15 * k9);

        sp0 += chan2;
        sp1 += chan2;
        dp += chan2;
      }

      if (wid & 1) {
        p00 = p02; p10 = p12;
        p01 = p03; p11 = p13;
        p02 = p04; p12 = p14;
        p03 = p05; p13 = p15;

        p04 = sp0[0];     p14 = sp1[0];

        buffd[i] = (p00 * k0 + p01 * k1 + p02 * k2 + p03 * k3 + p04 * k4 +
                    p10 * k5 + p11 * k6 + p12 * k7 + p13 * k8 + p14 * k9);
      }

      /*
       *  Second loop
       */
      sp0 = sl + 2*sll;
      sp1 = sp0 + sll;
      dp = dl;

      k0 = k[10]; k1 = k[11]; k2 = k[12]; k3 = k[13]; k4 = k[14];
      k5 = k[15]; k6 = k[16]; k7 = k[17]; k8 = k[18]; k9 = k[19];

      p02 = sp0[0];     p12 = sp1[0];
      p03 = sp0[chan1]; p13 = sp1[chan1];
      p04 = sp0[chan2]; p14 = sp1[chan2];
      p05 = sp0[chan3]; p15 = sp1[chan3];

      sp0 += chan4;
      sp1 += chan4;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        p00 = p02; p10 = p12;
        p01 = p03; p11 = p13;
        p02 = p04; p12 = p14;
        p03 = p05; p13 = p15;

        p04 = sp0[0];     p14 = sp1[0];
        p05 = sp0[chan1]; p15 = sp1[chan1];

        buffd[i    ] += (p00 * k0 + p01 * k1 + p02 * k2 + p03 * k3 + p04 * k4 +
                         p10 * k5 + p11 * k6 + p12 * k7 + p13 * k8 + p14 * k9);
        buffd[i + 1] += (p01 * k0 + p02 * k1 + p03 * k2 + p04 * k3 + p05 * k4 +
                         p11 * k5 + p12 * k6 + p13 * k7 + p14 * k8 + p15 * k9);

        sp0 += chan2;
        sp1 += chan2;
        dp += chan2;
      }

      if (wid & 1) {
        p00 = p02; p10 = p12;
        p01 = p03; p11 = p13;
        p02 = p04; p12 = p14;
        p03 = p05; p13 = p15;

        p04 = sp0[0];     p14 = sp1[0];

        buffd[i] += (p00 * k0 + p01 * k1 + p02 * k2 + p03 * k3 + p04 * k4 +
                     p10 * k5 + p11 * k6 + p12 * k7 + p13 * k8 + p14 * k9);
      }

      /*
       *  3 loop
       */
      dp = dl;
      sp0 = sl + 4*sll;

      k0 = k[20]; k1 = k[21]; k2 = k[22]; k3 = k[23]; k4 = k[24];

      p02 = sp0[0];
      p03 = sp0[chan1];
      p04 = sp0[chan2];
      p05 = sp0[chan3];

      sp0 += chan2 + chan2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i <= (wid - 2); i += 2) {
        p00 = p02; p01 = p03; p02 = p04; p03 = p05;

        p04 = sp0[0]; p05 = sp0[chan1];

        pix0 = (buffd[i    ] + p00 * k0 + p01 * k1 + p02 * k2 +
                p03 * k3 + p04 * k4) >> shift2;
        pix1 = (buffd[i + 1] + p01 * k0 + p02 * k1 + p03 * k2 +
                p04 * k3 + p05 * k4) >> shift2;

        CLAMP_STORE(dp[0],     pix0)
        CLAMP_STORE(dp[chan1], pix1)

        dp  += chan2;
        sp0 += chan2;
      }

      if (wid & 1) {
        p00 = p02; p01 = p03; p02 = p04; p03 = p05;

        p04 = sp0[0];

        pix0 = (buffd[i    ] + p00 * k0 + p01 * k1 + p02 * k2 +
                p03 * k3 + p04 * k4) >> shift2;
        CLAMP_STORE(dp[0],     pix0)
      }

      /* next line */
      sl += sll;
      dl += dll;
    }
  }

  if (pbuff != buff) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

#endif  /* #ifdef __sparc */

/***************************************************************/

#ifndef TYPE_SHORT

#undef  KSIZE
#define KSIZE 7

mlib_status CONV_FUNC(7x7)(mlib_image *dst,
                           mlib_image *src,
                           mlib_s32   *kern,
                           mlib_s32   scalef_expon,
                           mlib_s32   cmask)
{
  FTYPE    buff[(KSIZE + 3)*BUFF_LINE], *buffs[2*(KSIZE + 1)], *buffd;
  FTYPE    k[KSIZE*KSIZE];
  mlib_s32 l, m, buff_ind;
  mlib_s32 d0, d1;
  FTYPE    k0, k1, k2, k3, k4, k5, k6;
  FTYPE    p0, p1, p2, p3, p4, p5, p6, p7;
  DTYPE *sl2, *sl3, *sl4, *sl5, *sl6;
  DEF_VARS(DTYPE)
  LOAD_KERNEL(KSIZE*KSIZE);
  GET_SRC_DST_PARAMETERS(DTYPE)

  if (wid > BUFF_LINE) {
    pbuff = mlib_malloc((KSIZE + 3)*sizeof(FTYPE)*wid);
    if (pbuff == NULL) return MLIB_FAILURE;
  }

  for (l = 0; l < KSIZE + 1; l++) buffs[l] = pbuff + l*wid;
  for (l = 0; l < KSIZE + 1; l++) buffs[l + (KSIZE + 1)] = buffs[l];
  buffd = buffs[KSIZE] + wid;
  buffo = (mlib_s32*)(buffd + wid);
  buffi = buffo + (wid &~ 1);

  chan1 = nchannel;
  chan2 = chan1 + chan1;

  wid -= (KSIZE - 1);
  hgt -= (KSIZE - 1);

  adr_dst += ((KSIZE - 1)/2)*(dll + chan1);

  for (c = 0; c < nchannel; c++) {
    if (!(cmask & (1 << (nchannel - 1 - c)))) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    sl1 = sl  + sll;
    sl2 = sl1 + sll;
    sl3 = sl2 + sll;
    sl4 = sl3 + sll;
    sl5 = sl4 + sll;
    sl6 = sl5 + sll;
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < wid + (KSIZE - 1); i++) {
      buffs[0][i] = (FTYPE)sl[i*chan1];
      buffs[1][i] = (FTYPE)sl1[i*chan1];
      buffs[2][i] = (FTYPE)sl2[i*chan1];
      buffs[3][i] = (FTYPE)sl3[i*chan1];
      buffs[4][i] = (FTYPE)sl4[i*chan1];
      buffs[5][i] = (FTYPE)sl5[i*chan1];
      buffs[6][i] = (FTYPE)sl6[i*chan1];
    }
    buff_ind = 0;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < wid; i++) buffd[i] = 0.0;

    sl += KSIZE*sll;

    for (j = 0; j < hgt; j++) {
      FTYPE    **buffc = buffs + buff_ind;
      FTYPE    *buffn = buffc[KSIZE];
      FTYPE    *pk = k;

      for (l = 0; l < KSIZE; l++) {
        FTYPE    *buff = buffc[l];
        d64_2x32 dd;

        sp = sl;
        dp = dl;

        p2 = buff[0]; p3 = buff[1]; p4 = buff[2];
        p5 = buff[3]; p6 = buff[4]; p7 = buff[5];

        k0 = *pk++; k1 = *pk++; k2 = *pk++; k3 = *pk++;
        k4 = *pk++; k5 = *pk++; k6 = *pk++;

        if (l < (KSIZE - 1)) {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
          for (i = 0; i <= (wid - 2); i += 2) {
            p0 = p2; p1 = p3; p2 = p4; p3 = p5; p4 = p6; p5 = p7;

            p6 = buff[i + 6]; p7 = buff[i + 7];

            buffd[i    ] += p0*k0 + p1*k1 + p2*k2 + p3*k3 + p4*k4 + p5*k5 + p6*k6;
            buffd[i + 1] += p1*k0 + p2*k1 + p3*k2 + p4*k3 + p5*k4 + p6*k5 + p7*k6;
          }
        } else {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
          for (i = 0; i <= (wid - 2); i += 2) {
            p0 = p2; p1 = p3; p2 = p4; p3 = p5; p4 = p6; p5 = p7;

            p6 = buff[i + 6]; p7 = buff[i + 7];

            LOAD_BUFF(buffi);

            dd.d64 = *(FTYPE   *)(buffi + i);
            buffn[i    ] = (FTYPE)dd.i32s.i0;
            buffn[i + 1] = (FTYPE)dd.i32s.i1;

            d0 = D2I(p0*k0 + p1*k1 + p2*k2 + p3*k3 + p4*k4 + p5*k5 + p6*k6 + buffd[i    ]);
            d1 = D2I(p1*k0 + p2*k1 + p3*k2 + p4*k3 + p5*k4 + p6*k5 + p7*k6 + buffd[i + 1]);

            dp[0    ] = FROM_S32(d0);
            dp[chan1] = FROM_S32(d1);

            buffd[i    ] = 0.0;
            buffd[i + 1] = 0.0;

            sp += chan2;
            dp += chan2;
          }
        }
      }

      /* last pixels */
      for (; i < wid; i++) {
        FTYPE    *pk = k, s = 0;
        mlib_s32 d0;

        for (l = 0; l < KSIZE; l++) {
          FTYPE    *buff = buffc[l] + i;

          for (m = 0; m < KSIZE; m++) s += buff[m] * (*pk++);
        }

        d0 = D2I(s);
        dp[0] = FROM_S32(d0);

        buffn[i] = (FTYPE)sp[0];

        sp += chan1;
        dp += chan1;
      }

      for (l = 0; l < (KSIZE - 1); l++) buffn[wid + l] = sp[l*chan1];

      /* next line */
      sl += sll;
      dl += dll;

      buff_ind++;
      if (buff_ind >= KSIZE + 1) buff_ind = 0;
    }
  }

  if (pbuff != buff) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

#endif

/***************************************************************/

static mlib_status mlib_ImageConv1xN(mlib_image   *dst,
                                     mlib_image   *src,
                                     mlib_d64     *k,
                                     mlib_s32     n,
                                     mlib_u8      *acmask)
{
  FTYPE    buff[BUFF_SIZE];
  mlib_s32 off, kh;
  mlib_s32 d0, d1;
  FTYPE    *pk;
  FTYPE    k0, k1, k2, k3;
  FTYPE    p0, p1, p2, p3, p4;
  DEF_VARS(DTYPE)
  DTYPE    *sl_c, *dl_c, *sl0;
  mlib_s32 l, hsize, max_hsize;
  GET_SRC_DST_PARAMETERS(DTYPE)

  for (i = 0; i < n; i++) k[i] *= DSCALE;

  wid -= 1;
  hgt -= n;

  max_hsize = (CACHE_SIZE/sizeof(DTYPE))/sll;
  if (!max_hsize) max_hsize = 1;

  if (max_hsize > BUFF_SIZE) {
    pbuff = mlib_malloc(sizeof(FTYPE)*max_hsize);
  }

  chan1 = nchannel;

  sl_c = adr_src;
  dl_c = adr_dst;

  for (l = 0; l < hgt; l += hsize) {
    hsize = hgt - l;
    if (hsize > max_hsize) hsize = max_hsize;

    for (c = 0; c < nchannel; c++) {
      if (!acmask[c]) continue;

      sl = sl_c + c;
      dl = dl_c + c;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (j = 0; j < hsize; j++) pbuff[j] = 0.0;

      for (i = 0; i < wid; i++) {
        sl0 = sl;

        for (off = 0; off < (n - 4); off += 4) {
          pk = k + off;
          sp = sl0;

          k0 = pk[0]; k1 = pk[1]; k2 = pk[2]; k3 = pk[3];
          p2 = sp[0]; p3 = sp[sll]; p4 = sp[2*sll];
          sp += 3*sll;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
          for (j = 0; j < hsize; j += 2) {
            p0 = p2; p1 = p3; p2 = p4;
            p3 = sp[0];
            p4 = sp[sll];

            pbuff[j    ] += p0*k0 + p1*k1 + p2*k2 + p3*k3;
            pbuff[j + 1] += p1*k0 + p2*k1 + p3*k2 + p4*k3;

            sp += 2*sll;
          }

          sl0 += 4*sll;
        }

        pk = k + off;
        sp = sl0;

        k0 = pk[0]; k1 = pk[1]; k2 = pk[2]; k3 = pk[3];
        p2 = sp[0]; p3 = sp[sll]; p4 = sp[2*sll];

        dp = dl;
        kh = n - off;

        if (kh == 4) {
          sp += 3*sll;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
          for (j = 0; j <= (hsize - 2); j += 2) {
            p0 = p2; p1 = p3; p2 = p4;
            p3 = sp[0];
            p4 = sp[sll];

            d0 = D2I(p0*k0 + p1*k1 + p2*k2 + p3*k3 + pbuff[j]);
            d1 = D2I(p1*k0 + p2*k1 + p3*k2 + p4*k3 + pbuff[j + 1]);

            dp[0  ] = FROM_S32(d0);
            dp[dll] = FROM_S32(d1);

            pbuff[j] = 0;
            pbuff[j + 1] = 0;

            sp += 2*sll;
            dp += 2*dll;
          }

          if (j < hsize) {
            p0 = p2; p1 = p3; p2 = p4;
            p3 = sp[0];

            d0 = D2I(p0*k0 + p1*k1 + p2*k2 + p3*k3 + pbuff[j]);

            pbuff[j] = 0;

            dp[0] = FROM_S32(d0);
          }
        } else if (kh == 3) {
          sp += 2*sll;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
          for (j = 0; j <= (hsize - 2); j += 2) {
            p0 = p2; p1 = p3;
            p2 = sp[0];
            p3 = sp[sll];

            d0 = D2I(p0*k0 + p1*k1 + p2*k2 + pbuff[j]);
            d1 = D2I(p1*k0 + p2*k1 + p3*k2 + pbuff[j + 1]);

            dp[0  ] = FROM_S32(d0);
            dp[dll] = FROM_S32(d1);

            pbuff[j] = 0;
            pbuff[j + 1] = 0;

            sp += 2*sll;
            dp += 2*dll;
          }

          if (j < hsize) {
            p0 = p2; p1 = p3;
            p2 = sp[0];

            d0 = D2I(p0*k0 + p1*k1 + p2*k2 + pbuff[j]);

            pbuff[j] = 0;

            dp[0] = FROM_S32(d0);
          }
        } else if (kh == 2) {
          sp += sll;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
          for (j = 0; j <= (hsize - 2); j += 2) {
            p0 = p2;
            p1 = sp[0];
            p2 = sp[sll];

            d0 = D2I(p0*k0 + p1*k1 + pbuff[j]);
            d1 = D2I(p1*k0 + p2*k1 + pbuff[j + 1]);

            dp[0  ] = FROM_S32(d0);
            dp[dll] = FROM_S32(d1);

            pbuff[j] = 0;
            pbuff[j + 1] = 0;

            sp += 2*sll;
            dp += 2*dll;
          }

          if (j < hsize) {
            p0 = p2;
            p1 = sp[0];

            d0 = D2I(p0*k0 + p1*k1 + pbuff[j]);

            pbuff[j] = 0;

            dp[0] = FROM_S32(d0);
          }
        } else /* if (kh == 1) */ {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
          for (j = 0; j < hsize; j++) {
            p0 = sp[0];

            d0 = D2I(p0*k0 + pbuff[j]);

            dp[0] = FROM_S32(d0);

            pbuff[j] = 0;

            sp += sll;
            dp += dll;
          }
        }

        sl += chan1;
        dl += chan1;
      }
    }

    sl_c += max_hsize*sll;
    dl_c += max_hsize*dll;
  }

  if (pbuff != buff) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

/***************************************************************/

#define MAX_KER   7
#define MAX_N    15

#ifdef TYPE_SHORT
mlib_status  mlib_ImageConvMxN_S16(mlib_image *dst,
                                   mlib_image *src,
                                   mlib_d64   *k,
                                   mlib_s32   m,
                                   mlib_s32   n,
                                   mlib_u8    *acmask)
#else
mlib_status mlib_ImageConvMxN_U8(mlib_image *dst,
                                 mlib_image *src,
                                 mlib_d64   *k,
                                 mlib_s32   m,
                                 mlib_s32   n,
                                 mlib_u8    *acmask)
#endif
{
  FTYPE    buff[BUFF_SIZE], *buffs_arr[2*(MAX_N + 1)];
  FTYPE    **buffs = buffs_arr, *buffd;
  mlib_s32 l, off, kw, bsize, buff_ind;
  mlib_s32 d0, d1;
  FTYPE    k0, k1, k2, k3, k4, k5, k6;
  FTYPE    p0, p1, p2, p3, p4, p5, p6, p7;
  d64_2x32 dd;
  DEF_VARS(DTYPE)
  GET_SRC_DST_PARAMETERS(DTYPE)
  sp = NULL;
  dp = NULL;

  if (m == 1) return mlib_ImageConv1xN(dst, src, k, n, acmask);

  for (i = 0; i < m*n; i++) k[i] *= DSCALE;

  bsize = (n + 3)*wid;
  if ((bsize > BUFF_SIZE) || (n > MAX_N)) {
    pbuff = mlib_malloc(sizeof(FTYPE)*bsize + sizeof(FTYPE   *)*2*(n + 1));
    if (pbuff == NULL) return MLIB_FAILURE;
    buffs = (FTYPE   **)(pbuff + bsize);
  }

  for (l = 0; l < (n + 1); l++) buffs[l] = pbuff + l*wid;
  for (l = 0; l < (n + 1); l++) buffs[l + (n + 1)] = buffs[l];
  buffd = buffs[n] + wid;
  buffo = (mlib_s32*)(buffd + wid);
  buffi = buffo + (wid &~ 1);

  chan1 = nchannel;
  chan2 = chan1 + chan1;

  wid -= m;
  hgt -= n;

  for (c = 0; c < nchannel; c++) {
    if (!acmask[c]) continue;

    sl = adr_src + c;
    dl = adr_dst + c;

    for (l = 0; l < n; l++) {
      FTYPE    *buff = buffs[l];

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i < wid + (m - 1); i++) {
        buff[i] = (FTYPE)sl[i*chan1];
      }
      sl += sll;
    }
    buff_ind = 0;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (i = 0; i < wid; i++) buffd[i] = 0.0;

    for (j = 0; j < hgt; j++) {
      FTYPE    **buffc = buffs + buff_ind;
      FTYPE    *buffn = buffc[n];
      FTYPE    *pk = k;

      for (l = 0; l < n; l++) {
        FTYPE    *buff_l = buffc[l];

        for (off = 0; off < m;) {
          FTYPE    *buff = buff_l + off;

          kw = m - off;
          if (kw > 2*MAX_KER) kw = MAX_KER; else
            if (kw > MAX_KER) kw = kw/2;
          off += kw;

          sp = sl;
          dp = dl;

          p2 = buff[0]; p3 = buff[1]; p4 = buff[2];
          p5 = buff[3]; p6 = buff[4]; p7 = buff[5];

          k0 = pk[0]; k1 = pk[1]; k2 = pk[2]; k3 = pk[3];
          k4 = pk[4]; k5 = pk[5]; k6 = pk[6];
          pk += kw;

          if (kw == 7) {

            if (l < (n - 1) || off < m) {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2; p1 = p3; p2 = p4; p3 = p5; p4 = p6; p5 = p7;

                p6 = buff[i + 6]; p7 = buff[i + 7];

                buffd[i    ] += p0*k0 + p1*k1 + p2*k2 + p3*k3 + p4*k4 + p5*k5 + p6*k6;
                buffd[i + 1] += p1*k0 + p2*k1 + p3*k2 + p4*k3 + p5*k4 + p6*k5 + p7*k6;
              }
            } else {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2; p1 = p3; p2 = p4; p3 = p5; p4 = p6; p5 = p7;

                p6 = buff[i + 6]; p7 = buff[i + 7];

                LOAD_BUFF(buffi);

                dd.d64 = *(FTYPE   *)(buffi + i);
                buffn[i    ] = (FTYPE)dd.i32s.i0;
                buffn[i + 1] = (FTYPE)dd.i32s.i1;

                d0 = D2I(p0*k0 + p1*k1 + p2*k2 + p3*k3 + p4*k4 + p5*k5 + p6*k6 + buffd[i    ]);
                d1 = D2I(p1*k0 + p2*k1 + p3*k2 + p4*k3 + p5*k4 + p6*k5 + p7*k6 + buffd[i + 1]);

                dp[0    ] = FROM_S32(d0);
                dp[chan1] = FROM_S32(d1);

                buffd[i    ] = 0.0;
                buffd[i + 1] = 0.0;

                sp += chan2;
                dp += chan2;
              }
            }
          } else if (kw == 6) {

            if (l < (n - 1) || off < m) {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2; p1 = p3; p2 = p4; p3 = p5; p4 = p6;

                p5 = buff[i + 5]; p6 = buff[i + 6];

                buffd[i    ] += p0*k0 + p1*k1 + p2*k2 + p3*k3 + p4*k4 + p5*k5;
                buffd[i + 1] += p1*k0 + p2*k1 + p3*k2 + p4*k3 + p5*k4 + p6*k5;
              }
            } else {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2; p1 = p3; p2 = p4; p3 = p5; p4 = p6;

                p5 = buff[i + 5]; p6 = buff[i + 6];

                buffn[i    ] = (FTYPE)sp[0];
                buffn[i + 1] = (FTYPE)sp[chan1];

                d0 = D2I(p0*k0 + p1*k1 + p2*k2 + p3*k3 + p4*k4 + p5*k5 + buffd[i    ]);
                d1 = D2I(p1*k0 + p2*k1 + p3*k2 + p4*k3 + p5*k4 + p6*k5 + buffd[i + 1]);

                dp[0    ] = FROM_S32(d0);
                dp[chan1] = FROM_S32(d1);

                buffd[i    ] = 0.0;
                buffd[i + 1] = 0.0;

                sp += chan2;
                dp += chan2;
              }
            }
          } else if (kw == 5) {

            if (l < (n - 1) || off < m) {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2; p1 = p3; p2 = p4; p3 = p5;

                p4 = buff[i + 4]; p5 = buff[i + 5];

                buffd[i    ] += p0*k0 + p1*k1 + p2*k2 + p3*k3 + p4*k4;
                buffd[i + 1] += p1*k0 + p2*k1 + p3*k2 + p4*k3 + p5*k4;
              }
            } else {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2; p1 = p3; p2 = p4; p3 = p5;

                p4 = buff[i + 4]; p5 = buff[i + 5];

                buffn[i    ] = (FTYPE)sp[0];
                buffn[i + 1] = (FTYPE)sp[chan1];

                d0 = D2I(p0*k0 + p1*k1 + p2*k2 + p3*k3 + p4*k4 + buffd[i    ]);
                d1 = D2I(p1*k0 + p2*k1 + p3*k2 + p4*k3 + p5*k4 + buffd[i + 1]);

                dp[0    ] = FROM_S32(d0);
                dp[chan1] = FROM_S32(d1);

                buffd[i    ] = 0.0;
                buffd[i + 1] = 0.0;

                sp += chan2;
                dp += chan2;
              }
            }
          } else if (kw == 4) {

            if (l < (n - 1) || off < m) {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2; p1 = p3; p2 = p4;

                p3 = buff[i + 3]; p4 = buff[i + 4];

                buffd[i    ] += p0*k0 + p1*k1 + p2*k2 + p3*k3;
                buffd[i + 1] += p1*k0 + p2*k1 + p3*k2 + p4*k3;
              }
            } else {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2; p1 = p3; p2 = p4;

                p3 = buff[i + 3]; p4 = buff[i + 4];

                buffn[i    ] = (FTYPE)sp[0];
                buffn[i + 1] = (FTYPE)sp[chan1];

                d0 = D2I(p0*k0 + p1*k1 + p2*k2 + p3*k3 + buffd[i    ]);
                d1 = D2I(p1*k0 + p2*k1 + p3*k2 + p4*k3 + buffd[i + 1]);

                dp[0    ] = FROM_S32(d0);
                dp[chan1] = FROM_S32(d1);

                buffd[i    ] = 0.0;
                buffd[i + 1] = 0.0;

                sp += chan2;
                dp += chan2;
              }
            }
          } else if (kw == 3) {

            if (l < (n - 1) || off < m) {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2; p1 = p3;

                p2 = buff[i + 2]; p3 = buff[i + 3];

                buffd[i    ] += p0*k0 + p1*k1 + p2*k2;
                buffd[i + 1] += p1*k0 + p2*k1 + p3*k2;
              }
            } else {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2; p1 = p3;

                p2 = buff[i + 2]; p3 = buff[i + 3];

                buffn[i    ] = (FTYPE)sp[0];
                buffn[i + 1] = (FTYPE)sp[chan1];

                d0 = D2I(p0*k0 + p1*k1 + p2*k2 + buffd[i    ]);
                d1 = D2I(p1*k0 + p2*k1 + p3*k2 + buffd[i + 1]);

                dp[0    ] = FROM_S32(d0);
                dp[chan1] = FROM_S32(d1);

                buffd[i    ] = 0.0;
                buffd[i + 1] = 0.0;

                sp += chan2;
                dp += chan2;
              }
            }
          } else /*if (kw == 2)*/ {

            if (l < (n - 1) || off < m) {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2;

                p1 = buff[i + 1]; p2 = buff[i + 2];

                buffd[i    ] += p0*k0 + p1*k1;
                buffd[i + 1] += p1*k0 + p2*k1;
              }
            } else {
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
              for (i = 0; i <= (wid - 2); i += 2) {
                p0 = p2;

                p1 = buff[i + 1]; p2 = buff[i + 2];

                buffn[i    ] = (FTYPE)sp[0];
                buffn[i + 1] = (FTYPE)sp[chan1];

                d0 = D2I(p0*k0 + p1*k1 + buffd[i    ]);
                d1 = D2I(p1*k0 + p2*k1 + buffd[i + 1]);

                dp[0    ] = FROM_S32(d0);
                dp[chan1] = FROM_S32(d1);

                buffd[i    ] = 0.0;
                buffd[i + 1] = 0.0;

                sp += chan2;
                dp += chan2;
              }
            }
          }
        }
      }

      /* last pixels */
      for (; i < wid; i++) {
        FTYPE    *pk = k, s = 0;
        mlib_s32 x, d0;

        for (l = 0; l < n; l++) {
          FTYPE    *buff = buffc[l] + i;

          for (x = 0; x < m; x++) s += buff[x] * (*pk++);
        }

        d0 = D2I(s);
        dp[0] = FROM_S32(d0);

        buffn[i] = (FTYPE)sp[0];

        sp += chan1;
        dp += chan1;
      }

      for (l = 0; l < (m - 1); l++) buffn[wid + l] = sp[l*chan1];

      /* next line */
      sl += sll;
      dl += dll;

      buff_ind++;
      if (buff_ind >= n + 1) buff_ind = 0;
    }
  }

  if (pbuff != buff) mlib_free(pbuff);

  return MLIB_SUCCESS;
}

/***************************************************************/

#ifdef __ICL

#include <mmintrin.h>
#include <xmmintrin.h>

/***************************************************************/

typedef union
{
  __m64     u64;
  mlib_s32  s32[2];
  mlib_s16  s16[4];
  mlib_u8   u8[8];
} mlib_m64;

/***************************************************************/

#ifdef TYPE_SHORT

#define LOAD_KERNEL_2__M64                                      \
  km64[0].s32[0] =  ((kernel[0] >> 16) & 0xFFFF);               \
  km64[0].u64 = _mm_unpacklo_pi16(km64[0].u64, km64[0].u64);    \
  km64[0].u64 = _mm_unpacklo_pi16(km64[0].u64, km64[0].u64);    \
  km64[1].s32[0] =  ((kernel[1] >> 16) & 0xFFFF);               \
  km64[1].u64 = _mm_unpacklo_pi16(km64[1].u64, km64[1].u64);    \
  km64[1].u64 = _mm_unpacklo_pi16(km64[1].u64, km64[1].u64);    \
  km64[2].s32[0] =  ((kernel[2] >> 16) & 0xFFFF);               \
  km64[2].u64 = _mm_unpacklo_pi16(km64[2].u64, km64[2].u64);    \
  km64[2].u64 = _mm_unpacklo_pi16(km64[2].u64, km64[2].u64);    \
  km64[3].s32[0] =  ((kernel[3] >> 16) & 0xFFFF);               \
  km64[3].u64 = _mm_unpacklo_pi16(km64[3].u64, km64[3].u64);    \
  km64[3].u64 = _mm_unpacklo_pi16(km64[3].u64, km64[3].u64);    \
  km64[4].s32[0] =  ((kernel[4] >> 16) & 0xFFFF);               \
  km64[4].u64 = _mm_unpacklo_pi16(km64[4].u64, km64[4].u64);    \
  km64[4].u64 = _mm_unpacklo_pi16(km64[4].u64, km64[4].u64);    \
  km64[5].s32[0] =  ((kernel[5] >> 16) & 0xFFFF);               \
  km64[5].u64 = _mm_unpacklo_pi16(km64[5].u64, km64[5].u64);    \
  km64[5].u64 = _mm_unpacklo_pi16(km64[5].u64, km64[5].u64);    \
  km64[6].s32[0] =  ((kernel[6] >> 16) & 0xFFFF);               \
  km64[6].u64 = _mm_unpacklo_pi16(km64[6].u64, km64[6].u64);    \
  km64[6].u64 = _mm_unpacklo_pi16(km64[6].u64, km64[6].u64);    \
  km64[7].s32[0] =  ((kernel[7] >> 16) & 0xFFFF);               \
  km64[7].u64 = _mm_unpacklo_pi16(km64[7].u64, km64[7].u64);    \
  km64[7].u64 = _mm_unpacklo_pi16(km64[7].u64, km64[7].u64);    \
  km64[8].s32[0] =  ((kernel[8] >> 16) & 0xFFFF);               \
  km64[8].u64 = _mm_unpacklo_pi16(km64[8].u64, km64[8].u64);    \
  km64[8].u64 = _mm_unpacklo_pi16(km64[8].u64, km64[8].u64);    \
  k = (__m64 *)km64;

#define CONVOLVE(k0, k1, k2)                            \
  d0 = _mm_mullo_pi16(src0, k0);                        \
  d1 = _mm_mulhi_pi16(src0, k0);                        \
  mul_lo = _mm_unpacklo_pi16(d0, d1);                   \
  mul_hi = _mm_unpackhi_pi16(d0, d1);                   \
  sum_lo = _mm_add_pi32(sum_lo, mul_lo);                \
  sum_hi = _mm_add_pi32(sum_hi, mul_hi);                \
  src0 = _mm_srli_si64(src0, 0x10);                     \
  src0 = _mm_or_si64(src0, _mm_slli_si64(src1, 0x30));  \
  src1 = _mm_srli_si64(src1, 0x10);                     \
  d0 = _mm_mullo_pi16(src0, k1);                        \
  d1 = _mm_mulhi_pi16(src0, k1);                        \
  mul_lo = _mm_unpacklo_pi16(d0, d1);                   \
  mul_hi = _mm_unpackhi_pi16(d0, d1);                   \
  sum_lo = _mm_add_pi32(sum_lo, mul_lo);                \
  sum_hi = _mm_add_pi32(sum_hi, mul_hi);                \
  src0 = _mm_srli_si64(src0, 0x10);                     \
  src0 = _mm_or_si64(src0, _mm_slli_si64(src1, 0x30));  \
  src1 = _mm_srli_si64(src1, 0x10);                     \
  d0 = _mm_mullo_pi16(src0, k2);                        \
  d1 = _mm_mulhi_pi16(src0, k2);                        \
  mul_lo = _mm_unpacklo_pi16(d0, d1);                   \
  mul_hi = _mm_unpackhi_pi16(d0, d1);                   \
  sum_lo = _mm_add_pi32(sum_lo, mul_lo);                \
  sum_hi = _mm_add_pi32(sum_hi, mul_hi);                \
  src0 = _mm_srli_si64(src0, 0x10);                     \
  src0 = _mm_or_si64(src0, _mm_slli_si64(src1, 0x30));  \
  src1 = _mm_srli_si64(src1, 0x10);                     \

/***************************************************************/

static mlib_status mlib_conv3x3_16nw_1chan(mlib_image *dst,
                                           mlib_image *src,
                                           mlib_s32 *kernel,
                                           mlib_s32 scale)
{
  mlib_u8   *src_data = mlib_ImageGetData(src);
  mlib_u8   *dst_data = mlib_ImageGetData(dst);
  int       width       = mlib_ImageGetWidth(src);
  int       height      = mlib_ImageGetHeight(src);
  int       s_stride    = mlib_ImageGetStride(src);
  int       d_stride    = mlib_ImageGetStride(dst);
  int       src_stride  = (s_stride >> 3);
  int       dst_stride  = (d_stride >> 3);
  int       shift       = scale - 16;
  int       k0, k1, k2, k3, k4, k5, k6, k7, k8;

  mlib_u8   *src_row;
  mlib_u8   *dst_row;

  mlib_m64  km64[9];
  __m64     *k;
  int       loop;
  int       i, j;

  k0 = kernel[0] >> 16;  k1 = kernel[1] >> 16;  k2 = kernel[2] >> 16;
  k3 = kernel[3] >> 16;  k4 = kernel[4] >> 16;  k5 = kernel[5] >> 16;
  k6 = kernel[6] >> 16;  k7 = kernel[7] >> 16;  k8 = kernel[8] >> 16;

  src_row = src_data;
  dst_row = (dst_data + d_stride) + 2;

  for (j = 0; j < (height - 2); j++)
{
    mlib_s16 *s_ptr0 = (mlib_s16 *)src_row;
    mlib_s16 *s_ptr1 = (mlib_s16 *)(src_row + s_stride);
    mlib_s16 *s_ptr2 = (mlib_s16 *)(src_row + 2 * s_stride);
    mlib_s16 *d_ptr = (mlib_s16 *)dst_row;

    for (i = 0; i < 3; i++)
    {
      int p0, p1, p2, p3, p4, p5, p6, p7, p8;
      int d;

      p0 = s_ptr0[0];
      p1 = s_ptr0[1];
      p2 = s_ptr0[2];

      p3 = s_ptr1[0];
      p4 = s_ptr1[1];
      p5 = s_ptr1[2];

      p6 = s_ptr2[0];
      p7 = s_ptr2[1];
      p8 = s_ptr2[2];

      d = p0 * k0 + p1 * k1 + p2 * k2 +
      p3 * k3 + p4 * k4 + p5 * k5 +
      p6 * k6 + p7 * k7 + p8 * k8;

      d = (d >> (scale - 16));

      CLAMP_STORE(d_ptr[0], d)

      d_ptr++;
      s_ptr0++;
      s_ptr1++;
      s_ptr2++;
    }

    src_row += s_stride;
    dst_row += d_stride;
  }

  LOAD_KERNEL_2__M64

  src_row = src_data + 8;
  dst_row = (dst_data + d_stride) + 8;
  loop = (width - 8) >> 2;

  for (j = 0; j < (height - 2); j++) {
    __m64 *sptr;
    __m64 *dptr;
    __m64 src0, src1;
    __m64 d0, d1, d2;
    __m64 sum_lo, sum_hi;
    __m64 mul_lo, mul_hi;
    __m64 zero = 0;

    sptr = (__m64 *)src_row;
    dptr = (__m64 *)dst_row;

    for (i = 0; i < loop; i++)
    {
      src0 = sptr[0];
      src1 = sptr[1];

      sum_lo = 0;
      sum_hi = 0;

      CONVOLVE(k[0], k[1], k[2])

      src0 = (sptr + src_stride)[0];
      src1 = (sptr + src_stride)[1];

      CONVOLVE(k[3], k[4], k[5])

      src0 = (sptr + 2 * src_stride)[0];
      src1 = (sptr + 2 * src_stride)[1];

      CONVOLVE(k[6], k[7], k[8])

      sum_lo = _mm_srai_pi32(sum_lo, shift);
      sum_hi = _mm_srai_pi32(sum_hi, shift);

      *dptr = _mm_packs_pi32(sum_lo, sum_hi);

      dptr++;
      sptr++;
    }

    src_row += s_stride;
    dst_row += d_stride;
  }

  _mm_empty();

  src_row = src_data + (2 * width - 10);
  dst_row = (dst_data + d_stride) + (2 * width - 8);

  for (j = 0; j < (height - 2); j++) {
    mlib_s16 *s_ptr0 = (mlib_s16 *)src_row;
    mlib_s16 *s_ptr1 = (mlib_s16 *)(src_row + s_stride);
    mlib_s16 *s_ptr2 = (mlib_s16 *)(src_row + 2 * s_stride);
    mlib_s16 *d_ptr  = (mlib_s16 *)dst_row;

    for (i = 0; i < 3; i++)
    {
      int p0, p1, p2, p3, p4, p5, p6, p7, p8;
      int d;

      p0 = s_ptr0[0];
      p1 = s_ptr0[1];
      p2 = s_ptr0[2];

      p3 = s_ptr1[0];
      p4 = s_ptr1[1];
      p5 = s_ptr1[2];

      p6 = s_ptr2[0];
      p7 = s_ptr2[1];
      p8 = s_ptr2[2];

      d = p0 * k0 + p1 * k1 + p2 * k2 +
      p3 * k3 + p4 * k4 + p5 * k5 +
      p6 * k6 + p7 * k7 + p8 * k8;

      d = (d >> (scale - 16));

      CLAMP_STORE(d_ptr[0], d)

      d_ptr++;
      s_ptr0++;
      s_ptr1++;
      s_ptr2++;
    }

    src_row += s_stride;
    dst_row += d_stride;
  }

  return MLIB_SUCCESS;
}

/***************************************************************/

#else

static int mlib_round_8[16] =
{
  0x00400040, 0x00200020, 0x00100010, 0x00080008,
  0x00040004, 0x00020002, 0x00010001, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000
};

/***************************************************************/

#define LOAD_KERNEL_2__M64                                      \
  km64[0].s32[0] =  ((kernel[0] >> 16) & 0xFFFF);               \
  km64[0].u64 = _mm_unpacklo_pi16(km64[0].u64, km64[0].u64);    \
  km64[0].u64 = _mm_unpacklo_pi16(km64[0].u64, km64[0].u64);    \
  km64[1].s32[0] =  ((kernel[1] >> 16) & 0xFFFF);               \
  km64[1].u64 = _mm_unpacklo_pi16(km64[1].u64, km64[1].u64);    \
  km64[1].u64 = _mm_unpacklo_pi16(km64[1].u64, km64[1].u64);    \
  km64[2].s32[0] =  ((kernel[2] >> 16) & 0xFFFF);               \
  km64[2].u64 = _mm_unpacklo_pi16(km64[2].u64, km64[2].u64);    \
  km64[2].u64 = _mm_unpacklo_pi16(km64[2].u64, km64[2].u64);    \
  km64[3].s32[0] =  ((kernel[3] >> 16) & 0xFFFF);               \
  km64[3].u64 = _mm_unpacklo_pi16(km64[3].u64, km64[3].u64);    \
  km64[3].u64 = _mm_unpacklo_pi16(km64[3].u64, km64[3].u64);    \
  km64[4].s32[0] =  ((kernel[4] >> 16) & 0xFFFF);               \
  km64[4].u64 = _mm_unpacklo_pi16(km64[4].u64, km64[4].u64);    \
  km64[4].u64 = _mm_unpacklo_pi16(km64[4].u64, km64[4].u64);    \
  km64[5].s32[0] =  ((kernel[5] >> 16) & 0xFFFF);               \
  km64[5].u64 = _mm_unpacklo_pi16(km64[5].u64, km64[5].u64);    \
  km64[5].u64 = _mm_unpacklo_pi16(km64[5].u64, km64[5].u64);    \
  km64[6].s32[0] =  ((kernel[6] >> 16) & 0xFFFF);               \
  km64[6].u64 = _mm_unpacklo_pi16(km64[6].u64, km64[6].u64);    \
  km64[6].u64 = _mm_unpacklo_pi16(km64[6].u64, km64[6].u64);    \
  km64[7].s32[0] =  ((kernel[7] >> 16) & 0xFFFF);               \
  km64[7].u64 = _mm_unpacklo_pi16(km64[7].u64, km64[7].u64);    \
  km64[7].u64 = _mm_unpacklo_pi16(km64[7].u64, km64[7].u64);    \
  km64[8].s32[0] =  ((kernel[8] >> 16) & 0xFFFF);               \
  km64[8].u64 = _mm_unpacklo_pi16(km64[8].u64, km64[8].u64);    \
  km64[8].u64 = _mm_unpacklo_pi16(km64[8].u64, km64[8].u64);    \
  k = (__m64 *)km64;

#define CONVOLVE(k0, k1, k2, sum)       \
  d0 = src0;                            \
  d0 = _mm_unpacklo_pi8(d0, zero);      \
  d0 = _mm_sll_pi16(d0, 0x7);           \
  d1 = _mm_srli_si64(src0, 0x8);        \
  d0 = _mm_mulhi_pi16(d0, k0);          \
  sum = _mm_add_pi16(sum, d0);          \
  d1 = _mm_unpacklo_pi8(d1, zero);      \
  d1 = _mm_sll_pi16(d1, 0x7);           \
  d1 = _mm_mulhi_pi16(d1, k1);          \
  sum = _mm_add_pi16(sum, d1);          \
  d2 = _mm_srli_si64(src0, 0x10);       \
  d2 = _mm_unpacklo_pi8(d2, zero);      \
  d2 = _mm_sll_pi16(d2, 0x7);           \
  d2 = _mm_mulhi_pi16(d2, k2);          \
  sum = _mm_add_pi16(sum, d2);          \

/***************************************************************/

static mlib_status mlib_conv3x3_8nw_1chan(mlib_image *dst,
                                          mlib_image *src,
                                          mlib_s32 *kernel,
                                          mlib_s32 scale)
{
  mlib_u8   *src_data   = mlib_ImageGetData(src);
  mlib_u8   *dst_data   = mlib_ImageGetData(dst);
  int       width       = mlib_ImageGetWidth(src);
  int       height      = mlib_ImageGetHeight(src);
  int       s_stride    = mlib_ImageGetStride(src);
  int       d_stride    = mlib_ImageGetStride(dst);
  int       src_stride  = (s_stride >> 3);
  int       dst_stride  = (d_stride >> 3);
  int       gsr_scale   = 31 - scale;
  int       shift       = 6 - gsr_scale;
  int       k0, k1, k2, k3, k4, k5, k6, k7, k8;

  mlib_u8   *src_row    = src_data;
  mlib_u8   *dst_row    = dst_data + d_stride;
  mlib_u8   *s_ptr;
  mlib_u8   *d_ptr;

  mlib_m64  km64[9];
  mlib_m64  rnd;
  __m64     rnd_val;
  __m64     *k;
  int       loop, tail;
  int       i, j;

  LOAD_KERNEL_2__M64

  rnd.s32[0] = rnd.s32[1] = mlib_round_8[gsr_scale];
  rnd_val = rnd.u64;

  k0 = kernel[0] >> 16;  k1 = kernel[1] >> 16;  k2 = kernel[2] >> 16;
  k3 = kernel[3] >> 16;  k4 = kernel[4] >> 16;  k5 = kernel[5] >> 16;
  k6 = kernel[6] >> 16;  k7 = kernel[7] >> 16;  k8 = kernel[8] >> 16;

  for (j = 0; j < (height - 2); j++) {
    s_ptr = src_row;
    d_ptr = dst_row + 1;

    for (i = 0; i < 7; i++)
    {
      int p0, p1, p2, p3, p4, p5, p6, p7, p8;
      int s;

      p0 = s_ptr[0];
      p1 = s_ptr[1];
      p2 = s_ptr[2];

      p3 = (s_ptr + s_stride)[0];
      p4 = (s_ptr + s_stride)[1];
      p5 = (s_ptr + s_stride)[2];

      p6 = (s_ptr + 2 * s_stride)[0];
      p7 = (s_ptr + 2 * s_stride)[1];
      p8 = (s_ptr + 2 * s_stride)[2];

      s = p0 * k0 + p1 * k1 + p2 * k2 +
      p3 * k3 + p4 * k4 + p5 * k5 +
      p6 * k6 + p7 * k7 + p8 * k8;

      s = (s >> (scale - 16));

      CLAMP_STORE(d_ptr[0], s)

      d_ptr++;
      s_ptr++;
    }

    src_row += s_stride;
    dst_row += d_stride;
  }

  src_row = src_data + 8;
  dst_row = dst_data + d_stride + 8;
  loop = (width - 16) >> 3;

  for (j = 0; j < (height - 2); j++) {
    __m64 *sptr;
    __m64 *dptr;
    __m64 src0, src1;
    __m64 d0, d1, d2;
    __m64 sum_lo, sum_hi;
    __m64 zero;

    sptr = (__m64 *)src_row;
    dptr = (__m64 *)dst_row;

    for (i = 0; i < loop; i++)
    {
      src0 = sptr[0];
      src1 = sptr[1];

      sum_lo = rnd_val;
      sum_hi = rnd_val;

      CONVOLVE(k[0], k[1], k[2], sum_lo)

      src0 = _mm_or_si64(_mm_srli_si64(src0, 0x20), _mm_slli_si64(src1, 0x20));

      CONVOLVE(k[0], k[1], k[2], sum_hi)

      src0 = (sptr + src_stride)[0];
      src1 = (sptr + src_stride)[1];

      CONVOLVE(k[3], k[4], k[5], sum_lo)

      src0 = _mm_or_si64(_mm_srli_si64(src0, 0x20), _mm_slli_si64(src1, 0x20));

      CONVOLVE(k[3], k[4], k[5], sum_hi)

      src0 = (sptr + 2 * src_stride)[0];
      src1 = (sptr + 2 * src_stride)[1];

      CONVOLVE(k[6], k[7], k[8], sum_lo)

      src0 = _mm_or_si64(_mm_srli_si64(src0, 0x20), _mm_slli_si64(src1, 0x20));

      CONVOLVE(k[6], k[7], k[8], sum_hi)

      sum_lo = _mm_srai_pi16(sum_lo, shift);
      sum_hi = _mm_srai_pi16(sum_hi, shift);

      *dptr++ = _mm_packs_pu16(sum_lo, sum_hi);
      sptr++;
    }

    src_row += s_stride;
    dst_row += d_stride;
  }

  src_row = src_data + (width - 9);
  dst_row = (dst_data + d_stride) + (width - 8);

  for (j = 0; j < (height - 2); j++) {
    s_ptr = src_row;
    d_ptr = dst_row;

    for (i = 0; i < 7; i++)
    {
      int p0, p1, p2, p3, p4, p5, p6, p7, p8;
      int s;

      p0 = s_ptr[0];
      p1 = s_ptr[1];
      p2 = s_ptr[2];

      p3 = (s_ptr + s_stride)[0];
      p4 = (s_ptr + s_stride)[1];
      p5 = (s_ptr + s_stride)[2];

      p6 = (s_ptr + 2 * s_stride)[0];
      p7 = (s_ptr + 2 * s_stride)[1];
      p8 = (s_ptr + 2 * s_stride)[2];

      s = p0 * k0 + p1 * k1 + p2 * k2 +
      p3 * k3 + p4 * k4 + p5 * k5 +
      p6 * k6 + p7 * k7 + p8 * k8;

      s = (s >> (scale - 16));

      CLAMP_STORE(d_ptr[0], s)

      d_ptr++;
      s_ptr++;
    }

    src_row += s_stride;
    dst_row += d_stride;
  }

  _mm_empty();

  return MLIB_SUCCESS;
}

/***************************************************************/

#endif /* TYPE_SHORT */

#endif /* __ICL */
