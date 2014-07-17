/*
 * @(#)mlib_v_ImageLookUpSIS32U8Func.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageLookUpSIS32U8Func.c	1.3	99/04/08 SMI"

/*
 * $RCSfile: mlib_v_ImageLookUpSIS32U8Func.c,v $
 * $Revision: 4.1 $
 * $Date: 1996/12/11 20:44:07 $
 * $Author: liang $
 */



#include <stdlib.h>
#include "vis_proto.h"
#include "mlib_image.h"

/***************************************************************/
void mlib_v_ImageLookUpSI_S32_U8_2_DstA8D1(mlib_s32 *src,
                                           mlib_u8 *dst,
                                           mlib_s32 xsize,
                                           mlib_u8 **table)
{
  mlib_s32 *sp;              /* pointer to source data */
  mlib_s32 s0, s1, s2, s3;   /* source data */
  mlib_u16 *dl;              /* pointer to start of destination */
  mlib_u16 *dend;            /* pointer to end of destination */
  mlib_d64 *dp;              /* aligned pointer to destination */
  mlib_d64 t0, t1, t2;       /* destination data */
  mlib_d64 t3, t4, t5;       /* destination data */
  mlib_d64 t6, t7, acc;      /* destination data */
  mlib_s32 emask;            /* edge mask */
  mlib_s32 i, num;           /* loop variable */
  mlib_u8  *tab0 = &table[0][(mlib_u32)2147483648];
  mlib_u8  *tab1 = &table[1][(mlib_u32)2147483648];

  sp   = src;
  dl   = (mlib_u16*)dst;
  dp   = (mlib_d64 *) dl;
  dend = dl + xsize - 1;

  vis_alignaddr((void *) 0, 7);

  if (xsize >= 4) {

    s0 = sp[0];
    s1 = sp[1];
    s2 = sp[2];
    s3 = sp[3];
    sp += 4;

#pragma pipeloop(0)
    for(i = 0; i <= xsize - 8; i+=4, sp+=4) {
      t7 = vis_ld_u8_i(tab1, s3);
      t6 = vis_ld_u8_i(tab0, s3);
      t5 = vis_ld_u8_i(tab1, s2);
      t4 = vis_ld_u8_i(tab0, s2);
      t3 = vis_ld_u8_i(tab1, s1);
      t2 = vis_ld_u8_i(tab0, s1);
      t1 = vis_ld_u8_i(tab1, s0);
      t0 = vis_ld_u8_i(tab0, s0);
      acc = vis_faligndata(t7, acc);
      acc = vis_faligndata(t6, acc);
      acc = vis_faligndata(t5, acc);
      acc = vis_faligndata(t4, acc);
      acc = vis_faligndata(t3, acc);
      acc = vis_faligndata(t2, acc);
      acc = vis_faligndata(t1, acc);
      acc = vis_faligndata(t0, acc);
      s0 = sp[0];
      s1 = sp[1];
      s2 = sp[2];
      s3 = sp[3];
      *dp++ = acc;
    }
    t7 = vis_ld_u8_i(tab1, s3);
    t6 = vis_ld_u8_i(tab0, s3);
    t5 = vis_ld_u8_i(tab1, s2);
    t4 = vis_ld_u8_i(tab0, s2);
    t3 = vis_ld_u8_i(tab1, s1);
    t2 = vis_ld_u8_i(tab0, s1);
    t1 = vis_ld_u8_i(tab1, s0);
    t0 = vis_ld_u8_i(tab0, s0);
    acc = vis_faligndata(t7, acc);
    acc = vis_faligndata(t6, acc);
    acc = vis_faligndata(t5, acc);
    acc = vis_faligndata(t4, acc);
    acc = vis_faligndata(t3, acc);
    acc = vis_faligndata(t2, acc);
    acc = vis_faligndata(t1, acc);
    acc = vis_faligndata(t0, acc);
    *dp++ = acc;
  }

  if ((mlib_addr) dp <= (mlib_addr) dend) {

    num = (mlib_s32)((mlib_u16*) dend - (mlib_u16*) dp);
    sp  += num;
    num ++;
#pragma pipeloop(0)
    for (i = 0; i < num; i ++) {
      s0 = *sp;
      sp --;

      t0  = vis_ld_u8_i(tab1, s0);
      acc = vis_faligndata(t0, acc);

      t0  = vis_ld_u8_i(tab0, s0);
      acc = vis_faligndata(t0, acc);
    }

    emask = vis_edge16(dp, dend);
    vis_pst_16(acc, dp, emask);
  }
}

/***************************************************************/
void mlib_v_ImageLookUpSI_S32_U8_2_D1(mlib_s32 *src,
                                      mlib_u8  *dst,
                                      mlib_s32 xsize,
                                      mlib_u8 **table)
{
  mlib_s32 *sp;                /* pointer to source data */
  mlib_s32 s0, s1, s2, s3, s4; /* source data */
  mlib_u8  *dl;                /* pointer to start of destination */
  mlib_u8  *dend;              /* pointer to end of destination */
  mlib_d64 *dp;                /* aligned pointer to destination */
  mlib_d64 t0, t1, t2;         /* destination data */
  mlib_d64 t3, t4, t5;         /* destination data */
  mlib_d64 t6, t7, acc;        /* destination data */
  mlib_s32 emask;              /* edge mask */
  mlib_s32 i, num;             /* loop variable */
  mlib_u8  *tab0 = &table[0][(mlib_u32)2147483648];
  mlib_u8  *tab1 = &table[1][(mlib_u32)2147483648];

  sp   = src;
  dl   = dst;

  dend = dl + 2 * xsize - 1;

  vis_alignaddr((void *) 0, 7);

  s0 = *sp++;
  *dl++ = tab0[s0];
  dp   = (mlib_d64 *) dl;
  xsize--;

  if (xsize >= 4) {

    s1 = sp[0];
    s2 = sp[1];
    s3 = sp[2];
    s4 = sp[3];
    sp += 4;

#pragma pipeloop(0)
    for(i = 0; i <= xsize - 8; i+=4, sp+=4) {
      t7 = vis_ld_u8_i(tab0, s4);
      t6 = vis_ld_u8_i(tab1, s3);
      t5 = vis_ld_u8_i(tab0, s3);
      t4 = vis_ld_u8_i(tab1, s2);
      t3 = vis_ld_u8_i(tab0, s2);
      t2 = vis_ld_u8_i(tab1, s1);
      t1 = vis_ld_u8_i(tab0, s1);
      t0 = vis_ld_u8_i(tab1, s0);
      acc = vis_faligndata(t7, acc);
      acc = vis_faligndata(t6, acc);
      acc = vis_faligndata(t5, acc);
      acc = vis_faligndata(t4, acc);
      acc = vis_faligndata(t3, acc);
      acc = vis_faligndata(t2, acc);
      acc = vis_faligndata(t1, acc);
      acc = vis_faligndata(t0, acc);
      s0 = s4;
      s1 = sp[0];
      s2 = sp[1];
      s3 = sp[2];
      s4 = sp[3];
      *dp++ = acc;
    }
    t7 = vis_ld_u8_i(tab0, s4);
    t6 = vis_ld_u8_i(tab1, s3);
    t5 = vis_ld_u8_i(tab0, s3);
    t4 = vis_ld_u8_i(tab1, s2);
    t3 = vis_ld_u8_i(tab0, s2);
    t2 = vis_ld_u8_i(tab1, s1);
    t1 = vis_ld_u8_i(tab0, s1);
    t0 = vis_ld_u8_i(tab1, s0);
    acc = vis_faligndata(t7, acc);
    acc = vis_faligndata(t6, acc);
    acc = vis_faligndata(t5, acc);
    acc = vis_faligndata(t4, acc);
    acc = vis_faligndata(t3, acc);
    acc = vis_faligndata(t2, acc);
    acc = vis_faligndata(t1, acc);
    acc = vis_faligndata(t0, acc);
    s0 = s4;
    *dp++ = acc;
  }

  num = (mlib_s32)(((mlib_u8*) dend - (mlib_u8*) dp) >> 1);
  sp  += num - 1;

#pragma pipeloop(0)
  for (i = 0; i < num; i ++) {
    s1 = *sp;
    sp --;

    t0  = vis_ld_u8_i(tab1, s1);
    acc = vis_faligndata(t0, acc);

    t0  = vis_ld_u8_i(tab0, s1);
    acc = vis_faligndata(t0, acc);
  }
  t0  = vis_ld_u8_i(tab1, s0);
  acc = vis_faligndata(t0, acc);
  emask = vis_edge8(dp, dend);
  vis_pst_8(acc, dp, emask);
}

/***************************************************************/

void mlib_v_ImageLookUpSI_S32_U8_2(mlib_s32 *src, mlib_s32 slb,
                                   mlib_u8  *dst, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_u8  **table)
{
  mlib_s32 *sl;
  mlib_u8  *dl;
  mlib_s32 i, j;
  mlib_u8  *tab0 = &table[0][(mlib_u32)2147483648];
  mlib_u8  *tab1 = &table[1][(mlib_u32)2147483648];

  sl = src;
  dl = dst;

  /* row loop */
  for (j = 0; j < ysize; j ++) {
    mlib_s32 *sp = sl;
    mlib_u8  *dp = dl;
    mlib_s32 off, s0, size = xsize;

    off = (mlib_s32)(((8 - ((mlib_addr)dp & 7)) & 7) >> 1);
    off = (off < size) ? off : size;

    for (i = 0; i < off; i++) {
      s0 = *sp++;
      *dp++ = tab0[s0];
      *dp++ = tab1[s0];
      size--;
    }

    if (size > 0) {

      if (((mlib_addr)dp & 1) == 0) {
        mlib_v_ImageLookUpSI_S32_U8_2_DstA8D1(sp, dp, size, table);
      } else {
        mlib_v_ImageLookUpSI_S32_U8_2_D1(sp, dp, size, table);
      }
    }

    sl = (mlib_s32 *) ((mlib_u8 *) sl + slb);
    dl = (mlib_u8 *) ((mlib_u8 *) dl + dlb);
  }
}

/***************************************************************/
void mlib_v_ImageLookUpSI_S32_U8_3_D1(mlib_s32 *src,
                                      mlib_u8  *dst,
                                      mlib_s32 xsize,
                                      mlib_u8 **table)
{
  mlib_s32 *sp;              /* pointer to source data */
  mlib_u8  *dl;              /* pointer to start of destination */
  mlib_d64 *dp;              /* aligned pointer to destination */
  mlib_d64 t0, t1, t2;       /* destination data */
  mlib_d64 t3, t4, t5;       /* destination data */
  mlib_d64 t6, t7;           /* destination data */
  mlib_d64 acc0, acc1, acc2; /* destination data */
  mlib_s32 i;                /* loop variable */
  mlib_u8  *tab0 = &table[0][(mlib_u32)2147483648];
  mlib_u8  *tab1 = &table[1][(mlib_u32)2147483648];
  mlib_u8  *tab2 = &table[2][(mlib_u32)2147483648];
  mlib_s32 s00, s01, s02, s03;
  mlib_s32 s10, s11, s12, s13;

  sp   = src;
  dl   = dst;
  dp   = (mlib_d64 *) dl;

  vis_alignaddr((void *) 0, 7);

  i = 0;

  if (xsize >= 8) {

    s00 = sp[0];
    s01 = sp[1];
    s02 = sp[2];
    s03 = sp[3];
    s10 = sp[4];
    s11 = sp[5];
    s12 = sp[6];
    s13 = sp[7];
    sp += 8;

#pragma pipeloop(0)
    for(i = 0; i <= xsize - 16; i+=8, sp+=8) {
      t7 = vis_ld_u8_i(tab1, s02);
      t6 = vis_ld_u8_i(tab0, s02);
      t5 = vis_ld_u8_i(tab2, s01);
      t4 = vis_ld_u8_i(tab1, s01);
      t3 = vis_ld_u8_i(tab0, s01);
      t2 = vis_ld_u8_i(tab2, s00);
      t1 = vis_ld_u8_i(tab1, s00);
      t0 = vis_ld_u8_i(tab0, s00);
      acc0 = vis_faligndata(t7, acc0);
      acc0 = vis_faligndata(t6, acc0);
      acc0 = vis_faligndata(t5, acc0);
      acc0 = vis_faligndata(t4, acc0);
      acc0 = vis_faligndata(t3, acc0);
      acc0 = vis_faligndata(t2, acc0);
      acc0 = vis_faligndata(t1, acc0);
      acc0 = vis_faligndata(t0, acc0);
      t7 = vis_ld_u8_i(tab0, s11);
      t6 = vis_ld_u8_i(tab2, s10);
      t5 = vis_ld_u8_i(tab1, s10);
      t4 = vis_ld_u8_i(tab0, s10);
      t3 = vis_ld_u8_i(tab2, s03);
      t2 = vis_ld_u8_i(tab1, s03);
      t1 = vis_ld_u8_i(tab0, s03);
      t0 = vis_ld_u8_i(tab2, s02);
      acc1 = vis_faligndata(t7, acc1);
      acc1 = vis_faligndata(t6, acc1);
      acc1 = vis_faligndata(t5, acc1);
      acc1 = vis_faligndata(t4, acc1);
      acc1 = vis_faligndata(t3, acc1);
      acc1 = vis_faligndata(t2, acc1);
      acc1 = vis_faligndata(t1, acc1);
      acc1 = vis_faligndata(t0, acc1);
      t7 = vis_ld_u8_i(tab2, s13);
      t6 = vis_ld_u8_i(tab1, s13);
      t5 = vis_ld_u8_i(tab0, s13);
      t4 = vis_ld_u8_i(tab2, s12);
      t3 = vis_ld_u8_i(tab1, s12);
      t2 = vis_ld_u8_i(tab0, s12);
      t1 = vis_ld_u8_i(tab2, s11);
      t0 = vis_ld_u8_i(tab1, s11);
      acc2 = vis_faligndata(t7, acc2);
      acc2 = vis_faligndata(t6, acc2);
      acc2 = vis_faligndata(t5, acc2);
      acc2 = vis_faligndata(t4, acc2);
      acc2 = vis_faligndata(t3, acc2);
      acc2 = vis_faligndata(t2, acc2);
      acc2 = vis_faligndata(t1, acc2);
      acc2 = vis_faligndata(t0, acc2);
      s00 = sp[0];
      s01 = sp[1];
      s02 = sp[2];
      s03 = sp[3];
      s10 = sp[4];
      s11 = sp[5];
      s12 = sp[6];
      s13 = sp[7];
      *dp++ = acc0;
      *dp++ = acc1;
      *dp++ = acc2;
    }
    t7 = vis_ld_u8_i(tab1, s02);
    t6 = vis_ld_u8_i(tab0, s02);
    t5 = vis_ld_u8_i(tab2, s01);
    t4 = vis_ld_u8_i(tab1, s01);
    t3 = vis_ld_u8_i(tab0, s01);
    t2 = vis_ld_u8_i(tab2, s00);
    t1 = vis_ld_u8_i(tab1, s00);
    t0 = vis_ld_u8_i(tab0, s00);
    acc0 = vis_faligndata(t7, acc0);
    acc0 = vis_faligndata(t6, acc0);
    acc0 = vis_faligndata(t5, acc0);
    acc0 = vis_faligndata(t4, acc0);
    acc0 = vis_faligndata(t3, acc0);
    acc0 = vis_faligndata(t2, acc0);
    acc0 = vis_faligndata(t1, acc0);
    acc0 = vis_faligndata(t0, acc0);
    t7 = vis_ld_u8_i(tab0, s11);
    t6 = vis_ld_u8_i(tab2, s10);
    t5 = vis_ld_u8_i(tab1, s10);
    t4 = vis_ld_u8_i(tab0, s10);
    t3 = vis_ld_u8_i(tab2, s03);
    t2 = vis_ld_u8_i(tab1, s03);
    t1 = vis_ld_u8_i(tab0, s03);
    t0 = vis_ld_u8_i(tab2, s02);
    acc1 = vis_faligndata(t7, acc1);
    acc1 = vis_faligndata(t6, acc1);
    acc1 = vis_faligndata(t5, acc1);
    acc1 = vis_faligndata(t4, acc1);
    acc1 = vis_faligndata(t3, acc1);
    acc1 = vis_faligndata(t2, acc1);
    acc1 = vis_faligndata(t1, acc1);
    acc1 = vis_faligndata(t0, acc1);
    t7 = vis_ld_u8_i(tab2, s13);
    t6 = vis_ld_u8_i(tab1, s13);
    t5 = vis_ld_u8_i(tab0, s13);
    t4 = vis_ld_u8_i(tab2, s12);
    t3 = vis_ld_u8_i(tab1, s12);
    t2 = vis_ld_u8_i(tab0, s12);
    t1 = vis_ld_u8_i(tab2, s11);
    t0 = vis_ld_u8_i(tab1, s11);
    acc2 = vis_faligndata(t7, acc2);
    acc2 = vis_faligndata(t6, acc2);
    acc2 = vis_faligndata(t5, acc2);
    acc2 = vis_faligndata(t4, acc2);
    acc2 = vis_faligndata(t3, acc2);
    acc2 = vis_faligndata(t2, acc2);
    acc2 = vis_faligndata(t1, acc2);
    acc2 = vis_faligndata(t0, acc2);
    *dp++ = acc0;
    *dp++ = acc1;
    *dp++ = acc2;
    i += 8;
  }

  dl = (mlib_u8*)dp;

#pragma pipeloop(0)
  for (; i < xsize; i++) {
    s00 = sp[0];
    dl[0] = tab0[s00];
    dl[1] = tab1[s00];
    dl[2] = tab2[s00];
    dl += 3; sp ++;
  }
}

/***************************************************************/

void mlib_v_ImageLookUpSI_S32_U8_3(mlib_s32 *src, mlib_s32 slb,
                                   mlib_u8  *dst, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_u8  **table)
{
  mlib_s32 *sl;
  mlib_u8  *dl;
  mlib_s32 i, j;
  mlib_u8  *tab0 = &table[0][(mlib_u32)2147483648];
  mlib_u8  *tab1 = &table[1][(mlib_u32)2147483648];
  mlib_u8  *tab2 = &table[2][(mlib_u32)2147483648];

  sl = src;
  dl = dst;

  /* row loop */
  for (j = 0; j < ysize; j ++) {
    mlib_s32 *sp = sl;
    mlib_u8  *dp = dl;
    mlib_s32 off, s0, size = xsize;


    off = (mlib_s32)((mlib_addr)dp & 7);
    off = (off * 5) & 7;
    off = (off < size) ? off : size;

    for (i = 0; i < off; i++) {
      s0 = *sp++;
      *dp++ = tab0[s0];
      *dp++ = tab1[s0];
      *dp++ = tab2[s0];
      size--;
    }

    if (size > 0) {
      mlib_v_ImageLookUpSI_S32_U8_3_D1(sp, dp, size, table);
    }

    sl = (mlib_s32 *) ((mlib_u8 *) sl + slb);
    dl = (mlib_u8 *) ((mlib_u8 *) dl + dlb);
  }
}

/***************************************************************/
void mlib_v_ImageLookUpSI_S32_U8_4_DstOff0_D1(mlib_s32 *src,
                                              mlib_u8  *dst,
                                              mlib_s32 xsize,
                                              mlib_u8 **table)
{
  mlib_s32 *sp;              /* pointer to source data */
  mlib_s32 s0, s1;           /* source data */
  mlib_u8  *dl;              /* pointer to start of destination */
  mlib_d64 *dp;              /* aligned pointer to destination */
  mlib_d64 t0, t1, t2;       /* destination data */
  mlib_d64 t3, t4, t5;       /* destination data */
  mlib_d64 t6, t7, acc;      /* destination data */
  mlib_s32 i;                /* loop variable */
  mlib_u8  *tab0 = &table[0][(mlib_u32)2147483648];
  mlib_u8  *tab1 = &table[1][(mlib_u32)2147483648];
  mlib_u8  *tab2 = &table[2][(mlib_u32)2147483648];
  mlib_u8  *tab3 = &table[3][(mlib_u32)2147483648];

  sp   = src;
  dl   = dst;
  dp   = (mlib_d64 *) dl;

  vis_alignaddr((void *) 0, 7);

  if (xsize >= 2) {

    s0 = sp[0];
    s1 = sp[1];
    sp += 2;

#pragma pipeloop(0)
    for(i = 0; i <= xsize - 4; i+=2, sp+=2) {
      t7 = vis_ld_u8_i(tab3, s1);
      t6 = vis_ld_u8_i(tab2, s1);
      t5 = vis_ld_u8_i(tab1, s1);
      t4 = vis_ld_u8_i(tab0, s1);
      t3 = vis_ld_u8_i(tab3, s0);
      t2 = vis_ld_u8_i(tab2, s0);
      t1 = vis_ld_u8_i(tab1, s0);
      t0 = vis_ld_u8_i(tab0, s0);
      acc = vis_faligndata(t7, acc);
      acc = vis_faligndata(t6, acc);
      acc = vis_faligndata(t5, acc);
      acc = vis_faligndata(t4, acc);
      acc = vis_faligndata(t3, acc);
      acc = vis_faligndata(t2, acc);
      acc = vis_faligndata(t1, acc);
      acc = vis_faligndata(t0, acc);
      s0 = sp[0];
      s1 = sp[1];
      *dp++ = acc;
    }
    t7 = vis_ld_u8_i(tab3, s1);
    t6 = vis_ld_u8_i(tab2, s1);
    t5 = vis_ld_u8_i(tab1, s1);
    t4 = vis_ld_u8_i(tab0, s1);
    t3 = vis_ld_u8_i(tab3, s0);
    t2 = vis_ld_u8_i(tab2, s0);
    t1 = vis_ld_u8_i(tab1, s0);
    t0 = vis_ld_u8_i(tab0, s0);
    acc = vis_faligndata(t7, acc);
    acc = vis_faligndata(t6, acc);
    acc = vis_faligndata(t5, acc);
    acc = vis_faligndata(t4, acc);
    acc = vis_faligndata(t3, acc);
    acc = vis_faligndata(t2, acc);
    acc = vis_faligndata(t1, acc);
    acc = vis_faligndata(t0, acc);
    *dp++ = acc;
  }

  if ((xsize & 1) != 0) {
    s0 = sp[0];
    t7 = vis_ld_u8_i(tab3, s0);
    t6 = vis_ld_u8_i(tab2, s0);
    t5 = vis_ld_u8_i(tab1, s0);
    t4 = vis_ld_u8_i(tab0, s0);
    acc = vis_faligndata(t7, acc);
    acc = vis_faligndata(t6, acc);
    acc = vis_faligndata(t5, acc);
    acc = vis_faligndata(t4, acc);
    *(mlib_f32*)dp = vis_read_hi(acc);
  }
}

/***************************************************************/
void mlib_v_ImageLookUpSI_S32_U8_4_DstOff1_D1(mlib_s32 *src,
                                              mlib_u8 *dst,
                                              mlib_s32 xsize,
                                              mlib_u8 **table)
{
  mlib_s32 *sp;              /* pointer to source data */
  mlib_s32 s0, s1, s2;       /* source data */
  mlib_u8  *dl;              /* pointer to start of destination */
  mlib_d64 *dp;              /* aligned pointer to destination */
  mlib_d64 t0, t1, t2;       /* destination data */
  mlib_d64 t3, t4, t5;       /* destination data */
  mlib_d64 t6, t7, acc;      /* destination data */
  mlib_s32 i;                /* loop variable */
  mlib_u8  *tab0 = &table[0][(mlib_u32)2147483648];
  mlib_u8  *tab1 = &table[1][(mlib_u32)2147483648];
  mlib_u8  *tab2 = &table[2][(mlib_u32)2147483648];
  mlib_u8  *tab3 = &table[3][(mlib_u32)2147483648];

  sp   = src;
  dl   = dst;
  dp   = (mlib_d64 *) dl;

  vis_alignaddr((void *) 0, 7);

  s0 = *sp++;

  if (xsize >= 2) {

    s1 = sp[0];
    s2 = sp[1];
    sp += 2;

#pragma pipeloop(0)
    for(i = 0; i <= xsize - 4; i+=2, sp+=2) {
      t7 = vis_ld_u8_i(tab0, s2);
      t6 = vis_ld_u8_i(tab3, s1);
      t5 = vis_ld_u8_i(tab2, s1);
      t4 = vis_ld_u8_i(tab1, s1);
      t3 = vis_ld_u8_i(tab0, s1);
      t2 = vis_ld_u8_i(tab3, s0);
      t1 = vis_ld_u8_i(tab2, s0);
      t0 = vis_ld_u8_i(tab1, s0);
      acc = vis_faligndata(t7, acc);
      acc = vis_faligndata(t6, acc);
      acc = vis_faligndata(t5, acc);
      acc = vis_faligndata(t4, acc);
      acc = vis_faligndata(t3, acc);
      acc = vis_faligndata(t2, acc);
      acc = vis_faligndata(t1, acc);
      acc = vis_faligndata(t0, acc);
      s0 = s2;
      s1 = sp[0];
      s2 = sp[1];
      *dp++ = acc;
    }
    t7 = vis_ld_u8_i(tab0, s2);
    t6 = vis_ld_u8_i(tab3, s1);
    t5 = vis_ld_u8_i(tab2, s1);
    t4 = vis_ld_u8_i(tab1, s1);
    t3 = vis_ld_u8_i(tab0, s1);
    t2 = vis_ld_u8_i(tab3, s0);
    t1 = vis_ld_u8_i(tab2, s0);
    t0 = vis_ld_u8_i(tab1, s0);
    acc = vis_faligndata(t7, acc);
    acc = vis_faligndata(t6, acc);
    acc = vis_faligndata(t5, acc);
    acc = vis_faligndata(t4, acc);
    acc = vis_faligndata(t3, acc);
    acc = vis_faligndata(t2, acc);
    acc = vis_faligndata(t1, acc);
    acc = vis_faligndata(t0, acc);
    s0 = s2;
    *dp++ = acc;
  }

  dl = (mlib_u8*)dp;

  if ((xsize & 1) != 0) {
    s1 = sp[0];
    t7 = vis_ld_u8_i(tab0, s1);
    t6 = vis_ld_u8_i(tab3, s0);
    t5 = vis_ld_u8_i(tab2, s0);
    t4 = vis_ld_u8_i(tab1, s0);
    acc = vis_faligndata(t7, acc);
    acc = vis_faligndata(t6, acc);
    acc = vis_faligndata(t5, acc);
    acc = vis_faligndata(t4, acc);
    *(mlib_f32*)dl = vis_read_hi(acc);
    dl += 4;
    s0 = s1;
  }

  dl[0] = tab1[s0];
  dl[1] = tab2[s0];
  dl[2] = tab3[s0];
}

/***************************************************************/
void mlib_v_ImageLookUpSI_S32_U8_4_DstOff2_D1(mlib_s32 *src,
                                              mlib_u8 *dst,
                                              mlib_s32 xsize,
                                              mlib_u8 **table)
{
  mlib_s32 *sp;              /* pointer to source data */
  mlib_s32 s0, s1, s2;       /* source data */
  mlib_u8  *dl;              /* pointer to start of destination */
  mlib_d64 *dp;              /* aligned pointer to destination */
  mlib_d64 t0, t1, t2;       /* destination data */
  mlib_d64 t3, t4, t5;       /* destination data */
  mlib_d64 t6, t7, acc;      /* destination data */
  mlib_s32 i;                /* loop variable */
  mlib_u8  *tab0 = &table[0][(mlib_u32)2147483648];
  mlib_u8  *tab1 = &table[1][(mlib_u32)2147483648];
  mlib_u8  *tab2 = &table[2][(mlib_u32)2147483648];
  mlib_u8  *tab3 = &table[3][(mlib_u32)2147483648];

  sp   = src;
  dl   = dst;
  dp   = (mlib_d64 *) dl;

  vis_alignaddr((void *) 0, 7);

  s0 = *sp++;

  if (xsize >= 2) {

    s1 = sp[0];
    s2 = sp[1];
    sp += 2;

#pragma pipeloop(0)
    for(i = 0; i <= xsize - 4; i+=2, sp+=2) {
      t7 = vis_ld_u8_i(tab1, s2);
      t6 = vis_ld_u8_i(tab0, s2);
      t5 = vis_ld_u8_i(tab3, s1);
      t4 = vis_ld_u8_i(tab2, s1);
      t3 = vis_ld_u8_i(tab1, s1);
      t2 = vis_ld_u8_i(tab0, s1);
      t1 = vis_ld_u8_i(tab3, s0);
      t0 = vis_ld_u8_i(tab2, s0);
      acc = vis_faligndata(t7, acc);
      acc = vis_faligndata(t6, acc);
      acc = vis_faligndata(t5, acc);
      acc = vis_faligndata(t4, acc);
      acc = vis_faligndata(t3, acc);
      acc = vis_faligndata(t2, acc);
      acc = vis_faligndata(t1, acc);
      acc = vis_faligndata(t0, acc);
      s0 = s2;
      s1 = sp[0];
      s2 = sp[1];
      *dp++ = acc;
    }
    t7 = vis_ld_u8_i(tab1, s2);
    t6 = vis_ld_u8_i(tab0, s2);
    t5 = vis_ld_u8_i(tab3, s1);
    t4 = vis_ld_u8_i(tab2, s1);
    t3 = vis_ld_u8_i(tab1, s1);
    t2 = vis_ld_u8_i(tab0, s1);
    t1 = vis_ld_u8_i(tab3, s0);
    t0 = vis_ld_u8_i(tab2, s0);
    acc = vis_faligndata(t7, acc);
    acc = vis_faligndata(t6, acc);
    acc = vis_faligndata(t5, acc);
    acc = vis_faligndata(t4, acc);
    acc = vis_faligndata(t3, acc);
    acc = vis_faligndata(t2, acc);
    acc = vis_faligndata(t1, acc);
    acc = vis_faligndata(t0, acc);
    s0 = s2;
    *dp++ = acc;
  }

  dl = (mlib_u8*)dp;

  if ((xsize & 1) != 0) {
    s1 = sp[0];
    t7 = vis_ld_u8_i(tab1, s1);
    t6 = vis_ld_u8_i(tab0, s1);
    t5 = vis_ld_u8_i(tab3, s0);
    t4 = vis_ld_u8_i(tab2, s0);
    acc = vis_faligndata(t7, acc);
    acc = vis_faligndata(t6, acc);
    acc = vis_faligndata(t5, acc);
    acc = vis_faligndata(t4, acc);
    *(mlib_f32*)dl = vis_read_hi(acc);
    dl += 4;
    s0 = s1;
  }

  dl[0] = tab2[s0];
  dl[1] = tab3[s0];
}

/***************************************************************/
void mlib_v_ImageLookUpSI_S32_U8_4_DstOff3_D1(mlib_s32 *src,
                                              mlib_u8 *dst,
                                              mlib_s32 xsize,
                                              mlib_u8 **table)
{
  mlib_s32 *sp;              /* pointer to source data */
  mlib_s32 s0, s1, s2;       /* source data */
  mlib_u8  *dl;              /* pointer to start of destination */
  mlib_d64 *dp;              /* aligned pointer to destination */
  mlib_d64 t0, t1, t2;       /* destination data */
  mlib_d64 t3, t4, t5;       /* destination data */
  mlib_d64 t6, t7, acc;      /* destination data */
  mlib_s32 i;                /* loop variable */
  mlib_u8  *tab0 = &table[0][(mlib_u32)2147483648];
  mlib_u8  *tab1 = &table[1][(mlib_u32)2147483648];
  mlib_u8  *tab2 = &table[2][(mlib_u32)2147483648];
  mlib_u8  *tab3 = &table[3][(mlib_u32)2147483648];

  sp   = src;
  dl   = dst;
  dp   = (mlib_d64 *) dl;

  vis_alignaddr((void *) 0, 7);

  s0 = *sp++;

  if (xsize >= 2) {

    s1 = sp[0];
    s2 = sp[1];
    sp += 2;

#pragma pipeloop(0)
    for(i = 0; i <= xsize - 4; i+=2, sp+=2) {
      t7 = vis_ld_u8_i(tab2, s2);
      t6 = vis_ld_u8_i(tab1, s2);
      t5 = vis_ld_u8_i(tab0, s2);
      t4 = vis_ld_u8_i(tab3, s1);
      t3 = vis_ld_u8_i(tab2, s1);
      t2 = vis_ld_u8_i(tab1, s1);
      t1 = vis_ld_u8_i(tab0, s1);
      t0 = vis_ld_u8_i(tab3, s0);
      acc = vis_faligndata(t7, acc);
      acc = vis_faligndata(t6, acc);
      acc = vis_faligndata(t5, acc);
      acc = vis_faligndata(t4, acc);
      acc = vis_faligndata(t3, acc);
      acc = vis_faligndata(t2, acc);
      acc = vis_faligndata(t1, acc);
      acc = vis_faligndata(t0, acc);
      s0 = s2;
      s1 = sp[0];
      s2 = sp[1];
      *dp++ = acc;
    }
    t7 = vis_ld_u8_i(tab2, s2);
    t6 = vis_ld_u8_i(tab1, s2);
    t5 = vis_ld_u8_i(tab0, s2);
    t4 = vis_ld_u8_i(tab3, s1);
    t3 = vis_ld_u8_i(tab2, s1);
    t2 = vis_ld_u8_i(tab1, s1);
    t1 = vis_ld_u8_i(tab0, s1);
    t0 = vis_ld_u8_i(tab3, s0);
    acc = vis_faligndata(t7, acc);
    acc = vis_faligndata(t6, acc);
    acc = vis_faligndata(t5, acc);
    acc = vis_faligndata(t4, acc);
    acc = vis_faligndata(t3, acc);
    acc = vis_faligndata(t2, acc);
    acc = vis_faligndata(t1, acc);
    acc = vis_faligndata(t0, acc);
    s0 = s2;
    *dp++ = acc;
  }

  dl = (mlib_u8*)dp;

  if ((xsize & 1) != 0) {
    s1 = sp[0];
    t7 = vis_ld_u8_i(tab2, s1);
    t6 = vis_ld_u8_i(tab1, s1);
    t5 = vis_ld_u8_i(tab0, s1);
    t4 = vis_ld_u8_i(tab3, s0);
    acc = vis_faligndata(t7, acc);
    acc = vis_faligndata(t6, acc);
    acc = vis_faligndata(t5, acc);
    acc = vis_faligndata(t4, acc);
    *(mlib_f32*)dl = vis_read_hi(acc);
    dl += 4;
    s0 = s1;
  }

  dl[0] = tab3[s0];
}
/***************************************************************/

void mlib_v_ImageLookUpSI_S32_U8_4(mlib_s32  *src, mlib_s32 slb,
                                   mlib_u8  *dst, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_u8  **table)
{
  mlib_s32 *sl;
  mlib_u8  *dl;
  mlib_s32 j;
  mlib_u8  *tab0 = &table[0][(mlib_u32)2147483648];
  mlib_u8  *tab1 = &table[1][(mlib_u32)2147483648];
  mlib_u8  *tab2 = &table[2][(mlib_u32)2147483648];
  mlib_u8  *tab3 = &table[3][(mlib_u32)2147483648];

  sl = src;
  dl = dst;

  /* row loop */
  for (j = 0; j < ysize; j ++) {
    mlib_s32 *sp = sl;
    mlib_u8  *dp = dl;
    mlib_s32 off, s0, size = xsize;


    off =  (mlib_s32)((8 - ((mlib_addr)dp & 7)) & 7);
    if ((off >= 4) && (size > 0)) {
      s0 = *sp++;
      *dp++ = tab0[s0];
      *dp++ = tab1[s0];
      *dp++ = tab2[s0];
      *dp++ = tab3[s0];
      size--;
    }

    if (size > 0) {
      off =  (mlib_s32)((4 - ((mlib_addr)dp & 3)) & 3);

      if (off == 0) {
        mlib_v_ImageLookUpSI_S32_U8_4_DstOff0_D1(sp, dp, size, table);
      } else if (off == 1) {
        s0 = *sp;
        *dp++ = tab0[s0];
        size--;
        mlib_v_ImageLookUpSI_S32_U8_4_DstOff1_D1(sp, dp, size, table);
      } else if (off == 2) {
        s0 = *sp;
        *dp++ = tab0[s0];
        *dp++ = tab1[s0];
        size--;
        mlib_v_ImageLookUpSI_S32_U8_4_DstOff2_D1(sp, dp, size, table);
      } else if (off == 3) {
        s0 = *sp;
        *dp++ = tab0[s0];
        *dp++ = tab1[s0];
        *dp++ = tab2[s0];
        size--;
        mlib_v_ImageLookUpSI_S32_U8_4_DstOff3_D1(sp, dp, size, table);
      }
    }

    sl = (mlib_s32 *) ((mlib_u8 *) sl + slb);
    dl = (mlib_u8 *) ((mlib_u8 *) dl + dlb);
  }
}

/***************************************************************/
