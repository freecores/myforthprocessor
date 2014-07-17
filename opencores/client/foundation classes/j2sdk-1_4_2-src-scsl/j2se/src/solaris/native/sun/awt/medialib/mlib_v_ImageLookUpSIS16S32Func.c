/*
 * @(#)mlib_v_ImageLookUpSIS16S32Func.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageLookUpSIS16S32Func.c	1.1	98/07/28 SMI"

/*
 * $RCSfile: mlib_v_ImageLookUpSIS16S32Func.c,v $
 * $Revision: 4.1 $
 * $Date: 1996/12/11 20:44:05 $
 * $Author: liang $
 */




#include <stdlib.h>
#include "vis_proto.h"
#include "mlib_image.h"

/***************************************************************/

void mlib_v_ImageLookUpSI_S16_S32_2_D1(mlib_s16  *src,
                                       mlib_f32 *dst,
                                       mlib_s32 xsize,
                                       mlib_s32 **table)
{
  mlib_s32 *sa;          /* aligned pointer to source data */
  mlib_s16 *sp;          /* pointer to source data */
  mlib_s32 s0;           /* source data */
  mlib_f32 *dp;          /* aligned pointer to destination */
  mlib_f32 acc0, acc1;   /* destination data */
  mlib_f32 acc2, acc3;   /* destination data */
  mlib_s32 i;            /* loop variable */
  mlib_f32 *table0 = (mlib_f32*)(&table[0][32768]);
  mlib_f32 *table1 = (mlib_f32*)(&table[1][32768]);
  mlib_s32 s00, s01;

  sa   = (mlib_s32*)src;
  dp   = dst;

  i = 0;

  if (xsize >= 2) {

    s0 = *sa++;

#pragma pipeloop(0)
    for(i = 0; i <= xsize - 4; i+=2, dp += 4) {
      s00 = (s0 >> 14) & (~3);
      s01 = ((s0 << 16) >> 14);
      acc0 = *(mlib_f32*)((mlib_u8*)table0 + s00);
      acc1 = *(mlib_f32*)((mlib_u8*)table1 + s00);
      acc2 = *(mlib_f32*)((mlib_u8*)table0 + s01);
      acc3 = *(mlib_f32*)((mlib_u8*)table1 + s01);
      s0 = *sa++;
      dp[0] = acc0;
      dp[1] = acc1;
      dp[2] = acc2;
      dp[3] = acc3;
    }
    s00 = (s0 >> 14) & (~3);
    s01 = ((s0 << 16) >> 14);
    acc0 = *(mlib_f32*)((mlib_u8*)table0 + s00);
    acc1 = *(mlib_f32*)((mlib_u8*)table1 + s00);
    acc2 = *(mlib_f32*)((mlib_u8*)table0 + s01);
    acc3 = *(mlib_f32*)((mlib_u8*)table1 + s01);
    dp[0] = acc0;
    dp[1] = acc1;
    dp[2] = acc2;
    dp[3] = acc3;
    dp += 4;
    i += 2;
  }

  sp = (mlib_s16*)sa;

  if ( i < xsize ) {
    *dp++ = table0[sp[0]];
    *dp++ = table1[sp[0]];
  }
}

/***************************************************************/

void mlib_v_ImageLookUpSI_S16_S32_2(mlib_s16 *src, mlib_s32 slb,
                                    mlib_s32 *dst, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s32 **table)
{
  mlib_s16 *sl;
  mlib_s32 *dl;
  mlib_s32 j;
  mlib_s32 *tab0 = &table[0][32768];
  mlib_s32 *tab1 = &table[1][32768];

  sl = src;
  dl = dst;

  /* row loop */
  for (j = 0; j < ysize; j ++) {
    mlib_s16 *sp = sl;
    mlib_s32 *dp = dl;
    mlib_s32 s0, size = xsize;

    if (((mlib_addr)sp & 3) != 0) {
      s0 = *sp++;
      *dp++ = tab0[s0];
      *dp++ = tab1[s0];
      size--;
    }

    if (size > 0) {
      mlib_v_ImageLookUpSI_S16_S32_2_D1(sp, (mlib_f32*)dp, size, table);
    }

    sl = (mlib_s16 *) ((mlib_u8 *) sl + slb);
    dl = (mlib_s32 *) ((mlib_u8 *) dl + dlb);
  }
}

/***************************************************************/

void mlib_v_ImageLookUpSI_S16_S32_3_D1(mlib_s16  *src,
                                       mlib_f32 *dst,
                                       mlib_s32 xsize,
                                       mlib_s32 **table)
{
  mlib_s32 *sa;          /* aligned pointer to source data */
  mlib_s16 *sp;          /* pointer to source data */
  mlib_s32 s0;           /* source data */
  mlib_f32 *dp;          /* aligned pointer to destination */
  mlib_f32 acc0, acc1;   /* destination data */
  mlib_f32 acc2, acc3;   /* destination data */
  mlib_f32 acc4, acc5;   /* destination data */
  mlib_s32 i;            /* loop variable */
  mlib_f32 *table0 = (mlib_f32*)(&table[0][32768]);
  mlib_f32 *table1 = (mlib_f32*)(&table[1][32768]);
  mlib_f32 *table2 = (mlib_f32*)(&table[2][32768]);
  mlib_s32 s00, s01;

  sa   = (mlib_s32*)src;
  dp   = dst;

  i = 0;

  if (xsize >= 2) {

    s0 = *sa++;

#pragma pipeloop(0)
    for(i = 0; i <= xsize - 4; i+=2, dp += 6) {
      s00 = (s0 >> 14) & (~3);
      s01 = ((s0 << 16) >> 14);
      acc0 = *(mlib_f32*)((mlib_u8*)table0 + s00);
      acc1 = *(mlib_f32*)((mlib_u8*)table1 + s00);
      acc2 = *(mlib_f32*)((mlib_u8*)table2 + s00);
      acc3 = *(mlib_f32*)((mlib_u8*)table0 + s01);
      acc4 = *(mlib_f32*)((mlib_u8*)table1 + s01);
      acc5 = *(mlib_f32*)((mlib_u8*)table2 + s01);
      s0 = *sa++;
      dp[0] = acc0;
      dp[1] = acc1;
      dp[2] = acc2;
      dp[3] = acc3;
      dp[4] = acc4;
      dp[5] = acc5;
    }
    s00 = (s0 >> 14) & (~3);
    s01 = ((s0 << 16) >> 14);
    acc0 = *(mlib_f32*)((mlib_u8*)table0 + s00);
    acc1 = *(mlib_f32*)((mlib_u8*)table1 + s00);
    acc2 = *(mlib_f32*)((mlib_u8*)table2 + s00);
    acc3 = *(mlib_f32*)((mlib_u8*)table0 + s01);
    acc4 = *(mlib_f32*)((mlib_u8*)table1 + s01);
    acc5 = *(mlib_f32*)((mlib_u8*)table2 + s01);
    dp[0] = acc0;
    dp[1] = acc1;
    dp[2] = acc2;
    dp[3] = acc3;
    dp[4] = acc4;
    dp[5] = acc5;
    dp += 6;
    i += 2;
  }

  sp = (mlib_s16*)sa;

  if ( i < xsize ) {
    *dp++ = table0[sp[0]];
    *dp++ = table1[sp[0]];
    *dp++ = table2[sp[0]];
  }
}

/***************************************************************/

void mlib_v_ImageLookUpSI_S16_S32_3(mlib_s16 *src, mlib_s32 slb,
                                    mlib_s32 *dst, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s32 **table)
{
  mlib_s16 *sl;
  mlib_s32 *dl;
  mlib_s32 j;
  mlib_s32 *tab0 = &table[0][32768];
  mlib_s32 *tab1 = &table[1][32768];
  mlib_s32 *tab2 = &table[2][32768];

  sl = src;
  dl = dst;

  /* row loop */
  for (j = 0; j < ysize; j ++) {
    mlib_s16 *sp = sl;
    mlib_s32 *dp = dl;
    mlib_s32 s0, size = xsize;

    if (((mlib_addr)sp & 3) != 0) {
      s0 = *sp++;
      *dp++ = tab0[s0];
      *dp++ = tab1[s0];
      *dp++ = tab2[s0];
      size--;
    }

    if (size > 0) {
      mlib_v_ImageLookUpSI_S16_S32_3_D1(sp, (mlib_f32*)dp, size, table);
    }

    sl = (mlib_s16 *) ((mlib_u8 *) sl + slb);
    dl = (mlib_s32 *) ((mlib_u8 *) dl + dlb);
  }
}

/***************************************************************/

void mlib_v_ImageLookUpSI_S16_S32_4_D1(mlib_s16  *src,
                                       mlib_f32 *dst,
                                       mlib_s32 xsize,
                                       mlib_s32 **table)
{
  mlib_s32 *sa;          /* aligned pointer to source data */
  mlib_s16 *sp;          /* pointer to source data */
  mlib_s32 s0;           /* source data */
  mlib_f32 *dp;          /* aligned pointer to destination */
  mlib_f32 acc0, acc1;   /* destination data */
  mlib_f32 acc2, acc3;   /* destination data */
  mlib_f32 acc4, acc5;   /* destination data */
  mlib_f32 acc6, acc7;   /* destination data */
  mlib_s32 i;            /* loop variable */
  mlib_f32 *table0 = (mlib_f32*)(&table[0][32768]);
  mlib_f32 *table1 = (mlib_f32*)(&table[1][32768]);
  mlib_f32 *table2 = (mlib_f32*)(&table[2][32768]);
  mlib_f32 *table3 = (mlib_f32*)(&table[3][32768]);
  mlib_s32 s00, s01;

  sa   = (mlib_s32*)src;
  dp   = dst;

  i = 0;

  if (xsize >= 2) {

    s0 = *sa++;

#pragma pipeloop(0)
    for(i = 0; i <= xsize - 4; i+=2, dp += 8) {
      s00 = (s0 >> 14) & (~3);
      s01 = ((s0 << 16) >> 14);
      acc0 = *(mlib_f32*)((mlib_u8*)table0 + s00);
      acc1 = *(mlib_f32*)((mlib_u8*)table1 + s00);
      acc2 = *(mlib_f32*)((mlib_u8*)table2 + s00);
      acc3 = *(mlib_f32*)((mlib_u8*)table3 + s00);
      acc4 = *(mlib_f32*)((mlib_u8*)table0 + s01);
      acc5 = *(mlib_f32*)((mlib_u8*)table1 + s01);
      acc6 = *(mlib_f32*)((mlib_u8*)table2 + s01);
      acc7 = *(mlib_f32*)((mlib_u8*)table3 + s01);
      s0 = *sa++;
      dp[0] = acc0;
      dp[1] = acc1;
      dp[2] = acc2;
      dp[3] = acc3;
      dp[4] = acc4;
      dp[5] = acc5;
      dp[6] = acc6;
      dp[7] = acc7;
    }
    s00 = (s0 >> 14) & (~3);
    s01 = ((s0 << 16) >> 14);
    acc0 = *(mlib_f32*)((mlib_u8*)table0 + s00);
    acc1 = *(mlib_f32*)((mlib_u8*)table1 + s00);
    acc2 = *(mlib_f32*)((mlib_u8*)table2 + s00);
    acc3 = *(mlib_f32*)((mlib_u8*)table3 + s00);
    acc4 = *(mlib_f32*)((mlib_u8*)table0 + s01);
    acc5 = *(mlib_f32*)((mlib_u8*)table1 + s01);
    acc6 = *(mlib_f32*)((mlib_u8*)table2 + s01);
    acc7 = *(mlib_f32*)((mlib_u8*)table3 + s01);
    dp[0] = acc0;
    dp[1] = acc1;
    dp[2] = acc2;
    dp[3] = acc3;
    dp[4] = acc4;
    dp[5] = acc5;
    dp[6] = acc6;
    dp[7] = acc7;
    dp += 8;
    i += 2;
  }

  sp = (mlib_s16*)sa;

  if ( i < xsize ) {
    *dp++ = table0[sp[0]];
    *dp++ = table1[sp[0]];
    *dp++ = table2[sp[0]];
    *dp++ = table3[sp[0]];
  }
}

/***************************************************************/

void mlib_v_ImageLookUpSI_S16_S32_4(mlib_s16 *src, mlib_s32 slb,
                                    mlib_s32 *dst, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s32 **table)
{
  mlib_s16 *sl;
  mlib_s32 *dl;
  mlib_s32 j;
  mlib_s32 *tab0 = &table[0][32768];
  mlib_s32 *tab1 = &table[1][32768];
  mlib_s32 *tab2 = &table[2][32768];
  mlib_s32 *tab3 = &table[3][32768];

  sl = src;
  dl = dst;

  /* row loop */
  for (j = 0; j < ysize; j ++) {
    mlib_s16 *sp = sl;
    mlib_s32 *dp = dl;
    mlib_s32 s0, size = xsize;

    if (((mlib_addr)sp & 3) != 0) {
      s0 = *sp++;
      *dp++ = tab0[s0];
      *dp++ = tab1[s0];
      *dp++ = tab2[s0];
      *dp++ = tab3[s0];
      size--;
    }

    if (size > 0) {
      mlib_v_ImageLookUpSI_S16_S32_4_D1(sp, (mlib_f32*)dp, size, table);
    }

    sl = (mlib_s16 *) ((mlib_u8 *) sl + slb);
    dl = (mlib_s32 *) ((mlib_u8 *) dl + dlb);
  }
}

/***************************************************************/
