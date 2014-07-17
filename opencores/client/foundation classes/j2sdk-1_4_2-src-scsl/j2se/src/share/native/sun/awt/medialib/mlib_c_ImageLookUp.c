/*
 * @(#)mlib_c_ImageLookUp.c	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageLookUp.c	1.16	99/05/21 SMI"
#endif /* __SUNPRO_C */

/*
 * $RCSfile: mlib_c_ImageLookUp.c,v $
 * $Revision: 4.2 $
 * $Date: 1997/02/18 16:31:09 $
 * $Author: liang $
 */

/*
 * FUNCTION
 *      mlib_ImageLookUp - table lookup
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageLookUp(mlib_image *dst,
 *                                   mlib_image *src,
 *                                   void **table);
 *
 * ARGUMENT
 *      dst     pointer to output image
 *      src     pointer to input image
 *      table   lookup table
 *
 * DESCRIPTION
 *      dst = table[src]
 */


#include "mlib_image.h"
#include "mlib_ImageCheck.h"

/***************************************************************/
void mlib_ImageLookUp_U8_D64(mlib_u8  *src, mlib_s32 slb,
                             mlib_d64 *dst, mlib_s32 dlb,
                             mlib_s32 xsize, mlib_s32 ysize,
                             mlib_s32 csize,
                             mlib_d64 **table);

void mlib_ImageLookUp_S16_D64(mlib_s16 *src, mlib_s32 slb,
                              mlib_d64 *dst, mlib_s32 dlb,
                              mlib_s32 xsize, mlib_s32 ysize,
                              mlib_s32 csize,
                              mlib_d64 **table);

void mlib_ImageLookUp_S32_D64(mlib_s32 *src, mlib_s32 slb,
                              mlib_d64 *dst, mlib_s32 dlb,
                              mlib_s32 xsize, mlib_s32 ysize,
                              mlib_s32 csize,
                              mlib_d64 **table);

void mlib_ImageLookUpSI_U8_D64(mlib_u8  *src, mlib_s32 slb,
                               mlib_d64 *dst, mlib_s32 dlb,
                               mlib_s32 xsize, mlib_s32 ysize,
                               mlib_s32 csize,
                               mlib_d64 **table);

void mlib_ImageLookUpSI_S16_D64(mlib_s16 *src, mlib_s32 slb,
                                mlib_d64 *dst, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 csize,
                                mlib_d64 **table);

void mlib_ImageLookUpSI_S32_D64(mlib_s32 *src, mlib_s32 slb,
                                mlib_d64 *dst, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 csize,
                                mlib_d64 **table);

/***************************************************************/
#ifdef _MSC_VER

#define TABLE_SHIFT_S32 (mlib_u64)2147483648

#else

#define TABLE_SHIFT_S32 (mlib_u32)2147483648

#endif

/***************************************************************/
#define MLIB_C_IMAGELOOKUP(DTYPE, STYPE, TABLE)                         \
{                                                                       \
  mlib_s32 i, j, k;                                                     \
                                                                        \
  if (xsize < 2) {                                                      \
    for(j = 0; j < ysize; j++, dst += dlb, src += slb){                 \
      for(k = 0; k < csize; k++) {                                      \
        DTYPE *da = dst + k;                                            \
        STYPE *sa = src + k;                                            \
        DTYPE *tab = (DTYPE*) TABLE[k];                                 \
                                                                        \
        for(i = 0; i < xsize; i++, da += csize, sa += csize)            \
        *da=tab[*sa];                                                   \
      }                                                                 \
    }                                                                   \
  } else {                                                              \
    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {                \
      for(k = 0; k < csize; k++) {                                      \
        DTYPE    *da = dst + k;                                         \
        STYPE    *sa = src + k;                                         \
        DTYPE    *tab = (DTYPE*) TABLE[k];                              \
        mlib_s32 s0, t0, s1, t1;                                        \
                                                                        \
        s0 = (mlib_s32)sa[0];                                           \
        s1 = (mlib_s32)sa[csize];                                       \
        sa += 2*csize;                                                  \
                                                                        \
        for(i = 0; i < xsize - 3; i+=2, da += 2*csize, sa += 2*csize) { \
          t0 = (mlib_s32)tab[s0];                                       \
          t1 = (mlib_s32)tab[s1];                                       \
          s0 = (mlib_s32)sa[0];                                         \
          s1 = (mlib_s32)sa[csize];                                     \
          da[0] = (DTYPE)t0;                                            \
          da[csize] = (DTYPE)t1;                                        \
        }                                                               \
        t0 = (mlib_s32)tab[s0];                                         \
        t1 = (mlib_s32)tab[s1];                                         \
        da[0] = (DTYPE)t0;                                              \
        da[csize] = (DTYPE)t1;                                          \
        if (xsize & 1) da[2*csize] = tab[sa[0]];                        \
      }                                                                 \
    }                                                                   \
  }                                                                     \
}

/***************************************************************/
#define MLIB_C_IMAGELOOKUPSI(DTYPE, STYPE, TABLE)                       \
{                                                                       \
  mlib_s32 i, j, k;                                                     \
                                                                        \
  if (xsize < 2) {                                                      \
    for(j = 0; j < ysize; j++, dst += dlb, src += slb){                 \
      for(k = 0; k < csize; k++) {                                      \
        DTYPE *da = dst + k;                                            \
        STYPE *sa = src;                                                \
        DTYPE *tab = (DTYPE*) TABLE[k];                                 \
                                                                        \
        for(i = 0; i < xsize; i++, da += csize, sa ++)                  \
        *da=tab[*sa];                                                   \
      }                                                                 \
    }                                                                   \
  } else {                                                              \
    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {                \
      for(k = 0; k < csize; k++) {                                      \
        DTYPE    *da = dst + k;                                         \
        STYPE    *sa = src;                                             \
        DTYPE    *tab = (DTYPE*) TABLE[k];                              \
        mlib_s32 s0, t0, s1, t1;                                        \
                                                                        \
        s0 = (mlib_s32)sa[0];                                           \
        s1 = (mlib_s32)sa[1];                                           \
        sa += 2;                                                        \
                                                                        \
        for(i = 0; i < xsize - 3; i+=2, da += 2*csize, sa += 2) {       \
          t0 = (mlib_s32)tab[s0];                                       \
          t1 = (mlib_s32)tab[s1];                                       \
          s0 = (mlib_s32)sa[0];                                         \
          s1 = (mlib_s32)sa[1];                                         \
          da[0] = (DTYPE)t0;                                            \
          da[csize] = (DTYPE)t1;                                        \
        }                                                               \
        t0 = (mlib_s32)tab[s0];                                         \
        t1 = (mlib_s32)tab[s1];                                         \
        da[0] = (DTYPE)t0;                                              \
        da[csize] = (DTYPE)t1;                                          \
        if (xsize & 1) da[2*csize] = tab[sa[0]];                        \
      }                                                                 \
    }                                                                   \
  }                                                                     \
}
#ifdef _LITTLE_ENDIAN
/***************************************************************/
#define READ_U8_U8_ALIGN(table0, table1, table2, table3)      \
  t3 = table0[s0 & 0xFF];                                     \
  t2 = table1[s0>>8];                                         \
  t1 = table2[s1 & 0xFF];                                     \
  t0 = table3[s1>>8];
/***************************************************************/
#define READ_U8_U8_NOTALIGN(table0, table1, table2, table3)   \
  t3 = table0[s0 >> 8];                                       \
  t2 = table1[s1 & 0xFF];                                     \
  t1 = table2[s1 >> 8];                                       \
  t0 = table3[s2 & 0xFF];
/***************************************************************/
#define READ_U8_S16_ALIGN(table0, table1, table2, table3)     \
  t1 = *(mlib_u16*)((mlib_u8*)table0 + ((s0 << 1) & 0x1FE));  \
  t0 = *(mlib_u16*)((mlib_u8*)table1 + ((s0 >> 7) & 0x1FE));  \
  t3 = *(mlib_u16*)((mlib_u8*)table2 + ((s0 >> 15)  & 0x1FE));\
  t2 = *(mlib_u16*)((mlib_u8*)table3 + ((s0 >> 23)  & 0x1FE));
/***************************************************************/
#define READ_U8_S16_NOTALIGN(table0, table1, table2, table3)  \
  t1 = *(mlib_u16*)((mlib_u8*)table0 + ((s0 >> 7) & 0x1FE));  \
  t0 = *(mlib_u16*)((mlib_u8*)table1 + ((s0 >> 15)  & 0x1FE));\
  t3 = *(mlib_u16*)((mlib_u8*)table2 + ((s0 >> 23)  & 0x1FE));\
  t2 = *(mlib_u16*)((mlib_u8*)table3 + ((s1 << 1) & 0x1FE));
/***************************************************************/
#define ADD_READ_U8_S16_NOTALIGN(table0, table1, table2)      \
  t1 = *(mlib_u16*)((mlib_u8*)table0 + ((s1 >> 7) & 0x1FE));  \
  t0 = *(mlib_u16*)((mlib_u8*)table1 + ((s1 >> 15)  & 0x1FE));\
  t2 = *(mlib_u16*)((mlib_u8*)table2 + ((s1 >> 23)  & 0x1FE));
/***************************************************************/
#define READ_U8_S32(table0, table1, table2, table3)           \
  t0 = *(mlib_u32*)((mlib_u8*)table0 + ((s0 << 2) & 0x3FC));  \
  t1 = *(mlib_u32*)((mlib_u8*)table1 + ((s0 >> 6) & 0x3FC));  \
  t2 = *(mlib_u32*)((mlib_u8*)table2 + ((s0 >> 14)  & 0x3FC));\
  t3 = *(mlib_u32*)((mlib_u8*)table3 + ((s0 >> 22)  & 0x3FC));
/***************************************************************/
#else
/***************************************************************/
#define READ_U8_U8_ALIGN(table0, table1, table2, table3)      \
  t0 = table0[s0>>8];                                         \
  t1 = table1[s0 & 0xFF];                                     \
  t2 = table2[s1>>8];                                         \
  t3 = table3[s1 & 0xFF];                                     \
/***************************************************************/
#define READ_U8_U8_NOTALIGN(table0, table1, table2, table3)   \
  t0 = table0[s0 & 0xFF];                                     \
  t1 = table1[s1 >> 8];                                       \
  t2 = table2[s1 & 0xFF];                                     \
  t3 = table3[s2 >> 8];                                       \
/***************************************************************/
#define READ_U8_S16_ALIGN(table0, table1, table2, table3)     \
  t0 = *(mlib_u16*)((mlib_u8*)table0 + ((s0 >> 23) & 0x1FE)); \
  t1 = *(mlib_u16*)((mlib_u8*)table1 + ((s0 >> 15) & 0x1FE)); \
  t2 = *(mlib_u16*)((mlib_u8*)table2 + ((s0 >> 7)  & 0x1FE)); \
  t3 = *(mlib_u16*)((mlib_u8*)table3 + ((s0 << 1)  & 0x1FE));
/***************************************************************/
#define READ_U8_S16_NOTALIGN(table0, table1, table2, table3)  \
  t0 = *(mlib_u16*)((mlib_u8*)table0 + ((s0 >> 15) & 0x1FE)); \
  t1 = *(mlib_u16*)((mlib_u8*)table1 + ((s0 >> 7)  & 0x1FE)); \
  t2 = *(mlib_u16*)((mlib_u8*)table2 + ((s0 << 1)  & 0x1FE)); \
  t3 = *(mlib_u16*)((mlib_u8*)table3 + ((s1 >> 23) & 0x1FE));
/***************************************************************/
#define ADD_READ_U8_S16_NOTALIGN(table0, table1, table2)      \
  t0 = *(mlib_u16*)((mlib_u8*)table0 + ((s1 >> 15) & 0x1FE)); \
  t1 = *(mlib_u16*)((mlib_u8*)table1 + ((s1 >> 7)  & 0x1FE)); \
  t2 = *(mlib_u16*)((mlib_u8*)table2 + ((s1 << 1)  & 0x1FE));
/***************************************************************/
#define READ_U8_S32(table0, table1, table2, table3)           \
  t0 = *(mlib_u32*)((mlib_u8*)table0 + ((s0 >> 22) & 0x3FC)); \
  t1 = *(mlib_u32*)((mlib_u8*)table1 + ((s0 >> 14) & 0x3FC)); \
  t2 = *(mlib_u32*)((mlib_u8*)table2 + ((s0 >> 6)  & 0x3FC)); \
  t3 = *(mlib_u32*)((mlib_u8*)table3 + ((s0 << 2)  & 0x3FC));
/***************************************************************/
#endif
/***************************************************************/

void mlib_c_ImageLookUp_U8_U8(mlib_u8  *src, mlib_s32 slb,
                              mlib_u8  *dst, mlib_s32 dlb,
                              mlib_s32 xsize, mlib_s32 ysize,
                              mlib_s32 csize,
                              mlib_u8  **table)
{
  if (xsize*csize < 9) {
    MLIB_C_IMAGELOOKUP(mlib_u8, mlib_u8, table)
  } else if (csize == 1) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u16 *sa;
      mlib_u8 *tab = (mlib_u8*) table[0];
      mlib_u32 s0, s1, s2, t0, t1, t2, t3, t;
      mlib_s32 off;
      mlib_s32 size = xsize;
      mlib_u8  *dp = dst, *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)dst & 3)) & 3);

      for (i = 0; i < off; i++, sp++) {
        *dp++ = tab[sp[0]];
        size--;
      }

      da = (mlib_u32*)dp;

      if (((mlib_addr)sp & 1) == 0) {
        sa = (mlib_u16*)sp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 7; i+=4, da ++, sa += 2) {
          READ_U8_U8_ALIGN(tab, tab, tab, tab)
          t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
          s0 = sa[0];
          s1 = sa[1];
          da[0] = t;
        }
        READ_U8_U8_ALIGN(tab, tab, tab, tab)
        t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
        da[0] = t;
        da++;
        dp = (mlib_u8*)da;
        sp = (mlib_u8*)sa;
        i += 4;
        for (; i < size; i++, dp++, sp++) dp[0] = tab[sp[0]];

      } else {
        sa = (mlib_u16*)(sp - 1);

        s0 = sa[0];
        s1 = sa[1];
        s2 = sa[2];
        sa += 3;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 8; i+=4, da ++, sa += 2) {
          READ_U8_U8_NOTALIGN(tab, tab, tab, tab)
          t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
          s0 = s2;
          s1 = sa[0];
          s2 = sa[1];
          da[0] = t;
        }
        READ_U8_U8_NOTALIGN(tab, tab, tab, tab)
        t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
        da[0] = t;
        da++;
        dp = (mlib_u8*)da;
#ifdef _LITTLE_ENDIAN
        *dp++ = tab[s2 >> 8];
#else
        *dp++ = tab[s2 & 0xFF];
#endif
        sp = (mlib_u8*)sa;
        i += 5;
        for (; i < size; i++, dp++, sp++) dp[0] = tab[sp[0]];
      }
    }
  } else if (csize == 2) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u16 *sa;
      mlib_u8  *tab0 = (mlib_u8*) table[0];
      mlib_u8  *tab1 = (mlib_u8*) table[1];
      mlib_u8  *tab;
      mlib_u32 s0, s1, s2, t0, t1, t2, t3, t;
      mlib_s32 off;
      mlib_s32 size = xsize * 2;
      mlib_u8  *dp = dst, *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)dst & 3)) & 3);

      for (i = 0; i < off - 1; i+=2, sp += 2) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        size-=2;
      }

      if ((off & 1) != 0) {
        *dp++ = tab0[sp[0]];
        size--; sp++;
        tab = tab0; tab0 = tab1; tab1 = tab;
      }

      da = (mlib_u32*)dp;

      if (((mlib_addr)sp & 1) == 0) {
        sa = (mlib_u16*)sp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 7; i+=4, da ++, sa += 2) {
          READ_U8_U8_ALIGN(tab0, tab1, tab0, tab1)
          t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
          s0 = sa[0];
          s1 = sa[1];
          da[0] = t;
        }
        READ_U8_U8_ALIGN(tab0, tab1, tab0, tab1)
        t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
        da[0] = t;
        da++;
        dp = (mlib_u8*)da;
        sp = (mlib_u8*)sa;
        i += 4;

        for (; i < size - 1; i+=2, sp += 2) {
          *dp++ = tab0[sp[0]];
          *dp++ = tab1[sp[1]];
        }

        if (i < size) *dp = tab0[(*sp)];

      } else {
        sa = (mlib_u16*)(sp - 1);

        s0 = sa[0];
        s1 = sa[1];
        s2 = sa[2];
        sa += 3;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 8; i+=4, da ++, sa += 2) {
          READ_U8_U8_NOTALIGN(tab0, tab1, tab0, tab1)
          t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
          s0 = s2;
          s1 = sa[0];
          s2 = sa[1];
          da[0] = t;
        }
        READ_U8_U8_NOTALIGN(tab0, tab1, tab0, tab1)
        t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
        da[0] = t;
        da++;
        dp = (mlib_u8*)da;
#ifdef _LITTLE_ENDIAN
        *dp++ = tab0[s2>>8];
#else
        *dp++ = tab0[s2 & 0xFF];
#endif
        sp = (mlib_u8*)sa;
        i += 5;

        for (; i < size - 1; i+=2, sp += 2) {
          *dp++ = tab1[sp[0]];
          *dp++ = tab0[sp[1]];
        }

        if (i < size) *dp = tab1[(*sp)];
      }
    }
  } else if (csize == 3) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u16 *sa;
      mlib_u8  *tab0 = (mlib_u8*) table[0];
      mlib_u8  *tab1 = (mlib_u8*) table[1];
      mlib_u8  *tab2 = (mlib_u8*) table[2];
      mlib_u8  *tab;
      mlib_u32 s0, s1, s2, t0, t1, t2, t3, t;
      mlib_s32 off;
      mlib_s32 size = xsize * 3;
      mlib_u8  *dp = dst, *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)dst & 3)) & 3);

      if (off == 1) {
        *dp++ = tab0[sp[0]];
        tab = tab0; tab0 = tab1;
        tab1 = tab2; tab2 = tab;
        size--; sp++;
      } else if (off == 2) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        tab = tab2; tab2 = tab1;
        tab1 = tab0; tab0 = tab;
        size-=2; sp += 2;
      } else if (off == 3) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        *dp++ = tab2[sp[2]];
        size-=3; sp += 3;
      }

      da = (mlib_u32*)dp;

      if (((mlib_addr)sp & 1) == 0) {
        sa = (mlib_u16*)sp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 7; i+=4, da ++, sa += 2) {
          READ_U8_U8_ALIGN(tab0, tab1, tab2, tab0)
          t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
          tab = tab0; tab0 = tab1;
          tab1 = tab2; tab2 = tab;
          s0 = sa[0];
          s1 = sa[1];
          da[0] = t;
        }
        READ_U8_U8_ALIGN(tab0, tab1, tab2, tab0)
        t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
        da[0] = t;
        da++;
        dp = (mlib_u8*)da;
        sp = (mlib_u8*)sa;
        i += 4;

        if (i < size) {
          *dp++ = tab1[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp++ = tab2[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp++ = tab0[(*sp)];
        }

      } else {
        sa = (mlib_u16*)(sp - 1);

        s0 = sa[0];
        s1 = sa[1];
        s2 = sa[2];
        sa += 3;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 8; i+=4, da ++, sa += 2) {
          READ_U8_U8_NOTALIGN(tab0, tab1, tab2, tab0)
          t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
          tab = tab0; tab0 = tab1;
          tab1 = tab2; tab2 = tab;
          s0 = s2;
          s1 = sa[0];
          s2 = sa[1];
          da[0] = t;
        }
        READ_U8_U8_NOTALIGN(tab0, tab1, tab2, tab0)
        t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
        da[0] = t;
        da++;
        dp = (mlib_u8*)da;
#ifdef _LITTLE_ENDIAN
        *dp++ = tab1[s2>>8];
#else
        *dp++ = tab1[s2 & 0xFF];
#endif
        sp = (mlib_u8*)sa;
        i += 5;

        if (i < size) {
          *dp++ = tab2[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp++ = tab0[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp = tab1[(*sp)];
        }
      }
    }
  } else if (csize == 4) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u16 *sa;
      mlib_u8  *tab0 = (mlib_u8*) table[0];
      mlib_u8  *tab1 = (mlib_u8*) table[1];
      mlib_u8  *tab2 = (mlib_u8*) table[2];
      mlib_u8  *tab3 = (mlib_u8*) table[3];
      mlib_u8  *tab;
      mlib_u32 s0, s1, s2, t0, t1, t2, t3, t;
      mlib_s32 off;
      mlib_s32 size = xsize * 4;
      mlib_u8  *dp = dst, *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)dst & 3)) & 3);

      if (off == 1) {
        *dp++ = tab0[sp[0]];
        tab = tab0; tab0 = tab1;
        tab1 = tab2; tab2 = tab3; tab3 = tab;
        size--; sp++;
      } else if (off == 2) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        tab = tab0; tab0 = tab2; tab2 = tab;
        tab = tab1; tab1 = tab3; tab3 = tab;
        size-=2; sp += 2;
      } else if (off == 3) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        *dp++ = tab2[sp[2]];
        tab = tab3; tab3 = tab2;
        tab2 = tab1; tab1 = tab0; tab0 = tab;
        size-=3; sp += 3;
      }

      da = (mlib_u32*)dp;

      if (((mlib_addr)sp & 1) == 0) {
        sa = (mlib_u16*)sp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 7; i+=4, da ++, sa += 2) {
          READ_U8_U8_ALIGN(tab0, tab1, tab2, tab3)
          t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
          s0 = sa[0];
          s1 = sa[1];
          da[0] = t;
        }
        READ_U8_U8_ALIGN(tab0, tab1, tab2, tab3)
        t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
        da[0] = t;
        da++;
        dp = (mlib_u8*)da;
        sp = (mlib_u8*)sa;
        i += 4;

        if (i < size) {
          *dp++ = tab0[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp++ = tab1[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp = tab2[(*sp)];
        }

      } else {
        sa = (mlib_u16*)(sp - 1);

        s0 = sa[0];
        s1 = sa[1];
        s2 = sa[2];
        sa += 3;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 8; i+=4, da ++, sa += 2) {
          READ_U8_U8_NOTALIGN(tab0, tab1, tab2, tab3)
          t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
          s0 = s2;
          s1 = sa[0];
          s2 = sa[1];
          da[0] = t;
        }
        READ_U8_U8_NOTALIGN(tab0, tab1, tab2, tab3)
        t = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
        da[0] = t;
        da++;
        dp = (mlib_u8*)da;
#ifdef _LITTLE_ENDIAN
        *dp++ = tab0[s2>>8];
#else
        *dp++ = tab0[s2 & 0xFF];
#endif
        sp = (mlib_u8*)sa;
        i += 5;

        if (i < size) {
          *dp++ = tab1[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp++ = tab2[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp = tab3[(*sp)];
        }
      }
    }
  }
}

/***************************************************************/

void mlib_c_ImageLookUp_S16_U8(mlib_s16 *src, mlib_s32 slb,
                               mlib_u8  *dst, mlib_s32 dlb,
                               mlib_s32 xsize, mlib_s32 ysize,
                               mlib_s32 csize,
                               mlib_u8  **table)
{
  mlib_u8 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][32768];
  }

    MLIB_C_IMAGELOOKUP(mlib_u8, mlib_s16, table_base)
}

/***************************************************************/

void mlib_c_ImageLookUp_S32_U8(mlib_s32 *src, mlib_s32 slb,
                               mlib_u8  *dst, mlib_s32 dlb,
                               mlib_s32 xsize, mlib_s32 ysize,
                               mlib_s32 csize,
                               mlib_u8  **table)
{
  mlib_u8 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][TABLE_SHIFT_S32];
  }

    MLIB_C_IMAGELOOKUP(mlib_u8, mlib_s32, table_base)
}
/***************************************************************/

void mlib_c_ImageLookUp_U8_S16(mlib_u8  *src, mlib_s32 slb,
                               mlib_s16 *dst, mlib_s32 dlb,
                               mlib_s32 xsize, mlib_s32 ysize,
                               mlib_s32 csize,
                               mlib_s16 **table)
{
  if (xsize*csize < 12) {
    MLIB_C_IMAGELOOKUP(mlib_s16, mlib_u8, table)
  } else if (csize == 1) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *da;
      mlib_u16 *tab = (mlib_u16*) table[0];
      mlib_u32 s0, s1, t0, t1, t2, t3;
      mlib_u32 res1, res2;
      mlib_s32 off;
      mlib_s32 size = xsize;
      mlib_u16 *dp = (mlib_u16*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      for (i = 0; i < off; i++, sp++) {
        *dp++ = tab[sp[0]];
        size--;
      }

      sa = (mlib_u32*)sp;

      if (((mlib_addr)dp & 3) == 0) {
        da = (mlib_u32*)dp;

        s0 = sa[0];
        sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 7; i+=4, da+=2, sa++) {
          READ_U8_S16_ALIGN(tab, tab, tab, tab)
          res1 = (t0 << 16) + t1;
          res2 = (t2 << 16) + t3;
          s0 = sa[0];
          da[0] = res1;
          da[1] = res2;
        }
        READ_U8_S16_ALIGN(tab, tab, tab, tab)
        res1 = (t0 << 16) + t1;
        res2 = (t2 << 16) + t3;
        da[0] = res1;
        da[1] = res2;
        da += 2;
        dp = (mlib_u16*)da;
        sp = (mlib_u8*)sa;
        i += 4;
        for (; i < size; i++, dp++, sp++) dp[0] = tab[sp[0]];

      } else {

        *dp++ = tab[(*sp)];
        size--;
        da = (mlib_u32*)dp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 10; i+=4, da+=2, sa++) {
          READ_U8_S16_NOTALIGN(tab, tab, tab, tab)
          s0 = s1;
          res1 = (t0 << 16) + t1;
          res2 = (t2 << 16) + t3;
          s1 = sa[0];
          da[0] = res1;
          da[1] = res2;
        }
        READ_U8_S16_NOTALIGN(tab, tab, tab, tab)
        res1 = (t0 << 16) + t1;
        res2 = (t2 << 16) + t3;
        da[0] = res1;
        da[1] = res2;
        ADD_READ_U8_S16_NOTALIGN(tab, tab, tab)
        res1 = (t0 << 16) + t1;
        da[2] = res1;
        da += 3;
        dp = (mlib_u16*)da;
        *dp++ = (mlib_u16)t2;
        sp = (mlib_u8*)sa;
        i += 7;
        for (; i < size; i++, dp++, sp++) dp[0] = tab[sp[0]];
      }
    }
  } else if (csize == 2) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *da;
      mlib_u16 *tab0 = (mlib_u16*) table[0];
      mlib_u16 *tab1 = (mlib_u16*) table[1];
      mlib_u16 *tab;
      mlib_u32 s0, s1, t0, t1, t2, t3;
      mlib_u32 res1, res2;
      mlib_s32 off;
      mlib_s32 size = xsize*2;
      mlib_u16 *dp = (mlib_u16*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      for (i = 0; i < off - 1; i+=2, sp+=2) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        size-=2;
      }

      if ((off & 1) != 0) {
        *dp++ = tab0[*sp];
        size--; sp++;
        tab = tab0; tab0 = tab1; tab1 = tab;
      }

      sa = (mlib_u32*)sp;

      if (((mlib_addr)dp & 3) == 0) {
        da = (mlib_u32*)dp;

        s0 = sa[0];
        sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 7; i+=4, da+=2, sa++) {
          READ_U8_S16_ALIGN(tab0, tab1, tab0, tab1)
          res1 = (t0 << 16) + t1;
          res2 = (t2 << 16) + t3;
          s0 = sa[0];
          da[0] = res1;
          da[1] = res2;
        }
        READ_U8_S16_ALIGN(tab0, tab1, tab0, tab1)
        res1 = (t0 << 16) + t1;
        res2 = (t2 << 16) + t3;
        da[0] = res1;
        da[1] = res2;
        da += 2;
        dp = (mlib_u16*)da;
        sp = (mlib_u8*)sa;
        i += 4;

        for (; i < size - 1; i+=2, sp += 2) {
          *dp++ = tab0[sp[0]];
          *dp++ = tab1[sp[1]];
        }

        if (i < size) *dp = tab0[(*sp)];

      } else {

        *dp++ = tab0[(*sp)];
        size--;
        da = (mlib_u32*)dp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 10; i+=4, da+=2, sa++) {
          READ_U8_S16_NOTALIGN(tab1, tab0, tab1, tab0)
          s0 = s1;
          res1 = (t0 << 16) + t1;
          res2 = (t2 << 16) + t3;
          s1 = sa[0];
          da[0] = res1;
          da[1] = res2;
        }
        READ_U8_S16_NOTALIGN(tab1, tab0, tab1, tab0)
        res1 = (t0 << 16) + t1;
        res2 = (t2 << 16) + t3;
        da[0] = res1;
        da[1] = res2;
        ADD_READ_U8_S16_NOTALIGN(tab1, tab0, tab1)
        res1 = (t0 << 16) + t1;
        da[2] = res1;
        da += 3;
        dp = (mlib_u16*)da;
        *dp++ = (mlib_u16)t2;
        sp = (mlib_u8*)sa;
        i += 7;

        for (; i < size - 1; i+=2, sp += 2) {
          *dp++ = tab0[sp[0]];
          *dp++ = tab1[sp[1]];
        }

        if (i < size) *dp = tab0[(*sp)];
      }
    }
  } else if (csize == 3) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *da;
      mlib_u16 *tab0 = (mlib_u16*) table[0];
      mlib_u16 *tab1 = (mlib_u16*) table[1];
      mlib_u16 *tab2 = (mlib_u16*) table[2];
      mlib_u16 *tab;
      mlib_u32 s0, s1, t0, t1, t2, t3;
      mlib_u32 res1, res2;
      mlib_s32 off;
      mlib_s32 size = xsize*3;
      mlib_u16 *dp = (mlib_u16*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      if (off == 1) {
        *dp++ = tab0[(*sp)];
        tab = tab0; tab0 = tab1;
        tab1 = tab2; tab2 = tab;
        size--; sp++;
      } else if (off == 2) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        tab = tab2; tab2 = tab1;
        tab1 = tab0; tab0 = tab;
        size-=2; sp += 2;
      } else if (off == 3) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        *dp++ = tab2[sp[2]];
        size-=3; sp += 3;
      }

      sa = (mlib_u32*)sp;

      if (((mlib_addr)dp & 3) == 0) {
        da = (mlib_u32*)dp;

        s0 = sa[0];
        sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 7; i+=4, da+=2, sa++) {
          READ_U8_S16_ALIGN(tab0, tab1, tab2, tab0)
          res1 = (t0 << 16) + t1;
          res2 = (t2 << 16) + t3;
          tab = tab0; tab0 = tab1;
          tab1 = tab2; tab2 = tab;
          s0 = sa[0];
          da[0] = res1;
          da[1] = res2;
        }
        READ_U8_S16_ALIGN(tab0, tab1, tab2, tab0)
        res1 = (t0 << 16) + t1;
        res2 = (t2 << 16) + t3;
        da[0] = res1;
        da[1] = res2;
        da += 2;
        dp = (mlib_u16*)da;
        sp = (mlib_u8*)sa;
        i += 4;

        if (i < size) {
          *dp++ = tab1[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp++ = tab2[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp = tab0[(*sp)];
        }

      } else {

        *dp++ = tab0[(*sp)];
        size--;
        da = (mlib_u32*)dp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 10; i+=4, da+=2, sa++) {
          READ_U8_S16_NOTALIGN(tab1, tab2, tab0, tab1)
          s0 = s1;
          res1 = (t0 << 16) + t1;
          res2 = (t2 << 16) + t3;
          tab = tab0; tab0 = tab1;
          tab1 = tab2; tab2 = tab;
          s1 = sa[0];
          da[0] = res1;
          da[1] = res2;
        }
        READ_U8_S16_NOTALIGN(tab1, tab2, tab0, tab1)
        res1 = (t0 << 16) + t1;
        res2 = (t2 << 16) + t3;
        da[0] = res1;
        da[1] = res2;
        ADD_READ_U8_S16_NOTALIGN(tab2, tab0, tab1)
        res1 = (t0 << 16) + t1;
        da[2] = res1;
        da += 3;
        dp = (mlib_u16*)da;
        *dp++ = (mlib_u16)t2;
        sp = (mlib_u8*)sa;
        i += 7;

        if (i < size) {
          *dp++ = tab2[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp++ = tab0[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp = tab1[(*sp)];
        }
      }
    }
  } else if (csize == 4) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *da;
      mlib_u16 *tab0 = (mlib_u16*) table[0];
      mlib_u16 *tab1 = (mlib_u16*) table[1];
      mlib_u16 *tab2 = (mlib_u16*) table[2];
      mlib_u16 *tab3 = (mlib_u16*) table[3];
      mlib_u16 *tab;
      mlib_u32 s0, s1, t0, t1, t2, t3;
      mlib_u32 res1, res2;
      mlib_s32 off;
      mlib_s32 size = xsize*4;
      mlib_u16 *dp = (mlib_u16*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      if (off == 1) {
        *dp++ = tab0[(*sp)];
        tab = tab0; tab0 = tab1;
        tab1 = tab2; tab2 = tab3; tab3 = tab;
        size--; sp++;
      } else if (off == 2) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        tab = tab0; tab0 = tab2; tab2 = tab;
        tab = tab1; tab1 = tab3; tab3 = tab;
        size-=2; sp+=2;
      } else if (off == 3) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        *dp++ = tab2[sp[2]];
        tab = tab3; tab3 = tab2;
        tab2 = tab1; tab1 = tab0; tab0 = tab;
        size-=3; sp += 3;
      }

      sa = (mlib_u32*)sp;

      if (((mlib_addr)dp & 3) == 0) {
        da = (mlib_u32*)dp;

        s0 = sa[0];
        sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 7; i+=4, da+=2, sa++) {
          READ_U8_S16_ALIGN(tab0, tab1, tab2, tab3)
          res1 = (t0 << 16) + t1;
          res2 = (t2 << 16) + t3;
          s0 = sa[0];
          da[0] = res1;
          da[1] = res2;
        }
        READ_U8_S16_ALIGN(tab0, tab1, tab2, tab3)
        res1 = (t0 << 16) + t1;
        res2 = (t2 << 16) + t3;
        da[0] = res1;
        da[1] = res2;
        da += 2;
        dp = (mlib_u16*)da;
        sp = (mlib_u8*)sa;
        i += 4;

        if (i < size) {
          *dp++ = tab0[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp++ = tab1[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp = tab2[(*sp)];
        }

      } else {

        *dp++ = tab0[(*sp)];
        size--;
        da = (mlib_u32*)dp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 10; i+=4, da+=2, sa++) {
          READ_U8_S16_NOTALIGN(tab1, tab2, tab3, tab0)
          s0 = s1;
          res1 = (t0 << 16) + t1;
          res2 = (t2 << 16) + t3;
          s1 = sa[0];
          da[0] = res1;
          da[1] = res2;
        }
        READ_U8_S16_NOTALIGN(tab1, tab2, tab3, tab0)
        res1 = (t0 << 16) + t1;
        res2 = (t2 << 16) + t3;
        da[0] = res1;
        da[1] = res2;
        ADD_READ_U8_S16_NOTALIGN(tab1, tab2, tab3)
        res1 = (t0 << 16) + t1;
        da[2] = res1;
        da += 3;
        dp = (mlib_u16*)da;
        *dp++ = (mlib_u16)t2;
        sp = (mlib_u8*)sa;
        i += 7;

        if (i < size) {
          *dp++ = tab0[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp++ = tab1[(*sp)];
          i++; sp++;
        }

        if (i < size) {
          *dp = tab2[(*sp)];
        }
      }
    }
  }
}

/***************************************************************/

void mlib_c_ImageLookUp_S16_S16(mlib_s16 *src, mlib_s32 slb,
                                mlib_s16 *dst, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 csize,
                                mlib_s16 **table)
{
  mlib_s16 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][32768];
  }

  MLIB_C_IMAGELOOKUP(mlib_s16, mlib_s16, table_base)
}

/***************************************************************/

void mlib_c_ImageLookUp_S32_S16(mlib_s32 *src, mlib_s32 slb,
                                mlib_s16 *dst, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 csize,
                                mlib_s16 **table)
{
  mlib_s16 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][TABLE_SHIFT_S32];
  }

  MLIB_C_IMAGELOOKUP(mlib_s16, mlib_s32, table_base)
}

/***************************************************************/
void mlib_c_ImageLookUp_U8_S32(mlib_u8  *src, mlib_s32 slb,
                               mlib_s32 *dst, mlib_s32 dlb,
                               mlib_s32 xsize, mlib_s32 ysize,
                               mlib_s32 csize,
                               mlib_s32 **table)
{
  if (xsize*csize < 7) {
    MLIB_C_IMAGELOOKUP(mlib_s32, mlib_u8, table)
  } else if (csize == 1) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *tab = (mlib_u32*) table[0];
      mlib_u32 s0, t0, t1, t2, t3;
      mlib_s32 off;
      mlib_s32 size = xsize;
      mlib_u32 *dp = (mlib_u32*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      for (i = 0; i < off; i++, sp++) {
        *dp++ = tab[sp[0]];
        size--;
      }

      sa = (mlib_u32*)sp;

      s0 = sa[0];
      sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for(i = 0; i < size - 7; i+=4, dp+=4, sa++) {
        READ_U8_S32(tab, tab, tab, tab)
        s0 = sa[0];
        dp[0] = t0;
        dp[1] = t1;
        dp[2] = t2;
        dp[3] = t3;
      }
      READ_U8_S32(tab, tab, tab, tab)
      dp[0] = t0;
      dp[1] = t1;
      dp[2] = t2;
      dp[3] = t3;
      dp += 4;
      sp = (mlib_u8*)sa;
      i += 4;
      for (; i < size; i++, dp++, sp++) dp[0] = tab[sp[0]];
    }
  } else if (csize == 2) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *tab0 = (mlib_u32*) table[0];
      mlib_u32 *tab1 = (mlib_u32*) table[1];
      mlib_u32 *tab;
      mlib_u32 s0, t0, t1, t2, t3;
      mlib_s32 off;
      mlib_s32 size = xsize*2;
      mlib_u32 *dp = (mlib_u32*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      for (i = 0; i < off - 1; i+=2, sp+=2) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        size-=2;
      }

      if ((off & 1) != 0) {
        *dp++ = tab0[*sp];
        size--; sp++;
        tab = tab0; tab0 = tab1; tab1 = tab;
      }

      sa = (mlib_u32*)sp;

      s0 = sa[0];
      sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for(i = 0; i < size - 7; i+=4, dp+=4, sa++) {
        READ_U8_S32(tab0, tab1, tab0, tab1)
        s0 = sa[0];
        dp[0] = t0;
        dp[1] = t1;
        dp[2] = t2;
        dp[3] = t3;
      }
      READ_U8_S32(tab0, tab1, tab0, tab1)
      dp[0] = t0;
      dp[1] = t1;
      dp[2] = t2;
      dp[3] = t3;
      dp += 4;
      sp = (mlib_u8*)sa;
      i += 4;

      for (; i < size - 1; i+=2, sp += 2) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
      }

      if (i < size) *dp = tab0[(*sp)];
    }
  } else if (csize == 3) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *tab0 = (mlib_u32*) table[0];
      mlib_u32 *tab1 = (mlib_u32*) table[1];
      mlib_u32 *tab2 = (mlib_u32*) table[2];
      mlib_u32 *tab;
      mlib_u32 s0, t0, t1, t2, t3;
      mlib_s32 off;
      mlib_s32 size = xsize*3;
      mlib_u32 *dp = (mlib_u32*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      if (off == 1) {
        *dp++ = tab0[(*sp)];
        tab = tab0; tab0 = tab1;
        tab1 = tab2; tab2 = tab;
        size--; sp++;
      } else if (off == 2) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        tab = tab2; tab2 = tab1;
        tab1 = tab0; tab0 = tab;
        size-=2; sp += 2;
      } else if (off == 3) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        *dp++ = tab2[sp[2]];
        size-=3; sp += 3;
      }

      sa = (mlib_u32*)sp;

      s0 = sa[0];
      sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for(i = 0; i < size - 7; i+=4, dp+=4, sa++) {
        READ_U8_S32(tab0, tab1, tab2, tab0)
        tab = tab0; tab0 = tab1;
        tab1 = tab2; tab2 = tab;
        s0 = sa[0];
        dp[0] = t0;
        dp[1] = t1;
        dp[2] = t2;
        dp[3] = t3;
      }
      READ_U8_S32(tab0, tab1, tab2, tab0)
      dp[0] = t0;
      dp[1] = t1;
      dp[2] = t2;
      dp[3] = t3;
      dp += 4;
      sp = (mlib_u8*)sa;
      i += 4;

      if (i < size) {
        *dp++ = tab1[(*sp)];
        i++; sp++;
      }

      if (i < size) {
        *dp++ = tab2[(*sp)];
        i++; sp++;
      }

      if (i < size) {
        *dp = tab0[(*sp)];
      }
    }
  } else if (csize == 4) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *tab0 = (mlib_u32*) table[0];
      mlib_u32 *tab1 = (mlib_u32*) table[1];
      mlib_u32 *tab2 = (mlib_u32*) table[2];
      mlib_u32 *tab3 = (mlib_u32*) table[3];
      mlib_u32 *tab;
      mlib_u32 s0, t0, t1, t2, t3;
      mlib_s32 off;
      mlib_s32 size = xsize*4;
      mlib_u32 *dp = (mlib_u32*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      if (off == 1) {
        *dp++ = tab0[(*sp)];
        tab = tab0; tab0 = tab1;
        tab1 = tab2; tab2 = tab3; tab3 = tab;
        size--; sp++;
      } else if (off == 2) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        tab = tab0; tab0 = tab2; tab2 = tab;
        tab = tab1; tab1 = tab3; tab3 = tab;
        size-=2; sp+=2;
      } else if (off == 3) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[1]];
        *dp++ = tab2[sp[2]];
        tab = tab3; tab3 = tab2;
        tab2 = tab1; tab1 = tab0; tab0 = tab;
        size-=3; sp += 3;
      }

      sa = (mlib_u32*)sp;

      s0 = sa[0];
      sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for(i = 0; i < size - 7; i+=4, dp+=4, sa++) {
        READ_U8_S32(tab0, tab1, tab2, tab3)
        s0 = sa[0];
        dp[0] = t0;
        dp[1] = t1;
        dp[2] = t2;
        dp[3] = t3;
      }
      READ_U8_S32(tab0, tab1, tab2, tab3)
      dp[0] = t0;
      dp[1] = t1;
      dp[2] = t2;
      dp[3] = t3;
      dp += 4;
      sp = (mlib_u8*)sa;
      i += 4;

      if (i < size) {
        *dp++ = tab0[(*sp)];
        i++; sp++;
      }

      if (i < size) {
        *dp++ = tab1[(*sp)];
        i++; sp++;
      }

      if (i < size) {
        *dp = tab2[(*sp)];
      }
    }
  }
}

/***************************************************************/
void mlib_c_ImageLookUp_S16_S32(mlib_s16 *src, mlib_s32 slb,
                                mlib_s32 *dst, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 csize,
                                mlib_s32 **table)
{
  mlib_s32 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][32768];
  }

  MLIB_C_IMAGELOOKUP(mlib_s32, mlib_s16, table_base)
}

/***************************************************************/

void mlib_c_ImageLookUp_S32_S32(mlib_s32 *src, mlib_s32 slb,
                                mlib_s32 *dst, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 csize,
                                mlib_s32 **table)
{
  mlib_s32 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][TABLE_SHIFT_S32];
  }

  MLIB_C_IMAGELOOKUP(mlib_s32, mlib_s32, table_base)
}

/***************************************************************/

void mlib_c_ImageLookUpSI_U8_U8(mlib_u8  *src, mlib_s32 slb,
                                mlib_u8  *dst, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 csize,
                                mlib_u8  **table)
{
  if ((xsize < 8) || ((xsize*ysize) < 250)) {
   MLIB_C_IMAGELOOKUPSI(mlib_u8, mlib_u8, table)
  } else if (csize == 2) {

    mlib_u16 tab[256];
    mlib_u8  *tab0 = table[0];
    mlib_u8  *tab1 = table[1];
    mlib_s32 i, j, s0, s1, s2;

    s0 = tab0[0];
    s1 = tab1[0];
    for (i = 1; i < 256; i++) {
#ifdef _LITTLE_ENDIAN
      s2 = (s1 << 8) + s0;
#else
      s2 = (s0 << 8) + s1;
#endif
      s0 = tab0[i];
      s1 = tab1[i];
      tab[i-1] = (mlib_u16)s2;
    }
#ifdef _LITTLE_ENDIAN
    s2 = (s1 << 8) + s0;
#else
    s2 = (s0 << 8) + s1;
#endif
    tab[255] = (mlib_u16)s2;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_s32 *da;
      mlib_u8  *dp = dst;
      mlib_u8  *sa = src;
      mlib_s32 s0, t0, s1, t1, t, t2, off;
      mlib_s32 size = xsize;

      if (((mlib_addr)dp & 1) == 0) {

        if (((mlib_addr)dp & 3) != 0) {
          *((mlib_u16*)dp) = tab[sa[0]];
          sa++; size--; dp+=2;
        }
        da = (mlib_s32*)dp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 3; i+=2, da++, sa += 2) {
          t0 = tab[s0];
          t1 = tab[s1];
#ifdef _LITTLE_ENDIAN
          t = (t1 << 16) + t0;
#else
          t = (t0 << 16) + t1;
#endif
          s0 = sa[0];
          s1 = sa[1];
          da[0] = t;
        }
        t0 = tab[s0];
        t1 = tab[s1];
#ifdef _LITTLE_ENDIAN
          t = (t1 << 16) + t0;
#else
          t = (t0 << 16) + t1;
#endif
        da[0] = t;
        da++;
        if (size & 1) *((mlib_u16*)da) = tab[sa[0]];

      } else {

        off = (mlib_s32)(4 - ((mlib_addr)dp & 3));

        if (off > 1) {
          t0 = tab[sa[0]];
#ifdef _LITTLE_ENDIAN
          dp[1] = (t0 >> 8);
          dp[0] = t0;
#else
          dp[0] = (t0 >> 8);
          dp[1] = t0;
#endif
          sa++; size--; dp+=2;
        }

        t0 = tab[sa[0]]; sa++;
#ifdef _LITTLE_ENDIAN
        *dp++ = t0;
#else
        *dp++ = (t0 >> 8);
#endif

        da = (mlib_s32*)dp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 4; i+=2, da++, sa += 2) {
          t1 = tab[s0];
          t2 = tab[s1];
#ifdef _LITTLE_ENDIAN
          t = (t0 >> 8) + (t1 << 8) + (t2 << 24);
#else
          t = (t0 << 24) + (t1 << 8) + (t2 >> 8);
#endif
          t0 = t2;
          s0 = sa[0];
          s1 = sa[1];
          da[0] = t;
        }
        t1 = tab[s0];
        t2 = tab[s1];
#ifdef _LITTLE_ENDIAN
        t = (t0 >> 8) + (t1 << 8) + (t2 << 24);
#else
        t = (t0 << 24) + (t1 << 8) + (t2 >> 8);
#endif
        da[0] = t;
        da++;
        dp = (mlib_u8*)da;
#ifdef _LITTLE_ENDIAN
        dp[0] = (t2 >> 8);
#else
        dp[0] = t2;
#endif

        if ((size & 1) == 0) {
          t0 = tab[sa[0]];
#ifdef _LITTLE_ENDIAN
          dp[2] = (t0 >> 8);
          dp[1] = t0;
#else
          dp[1] = (t0 >> 8);
          dp[2] = t0;
#endif
        }
      }
    }
  } else if (csize == 3) {
    mlib_u32 tab[256];
    mlib_u8  *tab0 = table[0];
    mlib_u8  *tab1 = table[1];
    mlib_u8  *tab2 = table[2];
    mlib_s32 i, j;
    mlib_u32 s0, s1, s2, s3;


    s0 = tab0[0];
    s1 = tab1[0];
    s2 = tab2[0];
    for (i = 1; i < 256; i++) {
#ifdef _LITTLE_ENDIAN
      s3 = (s2 << 24) + (s1 << 16) + (s0 << 8);
#else
      s3 = (s0 << 16) + (s1 << 8) + s2;
#endif
      s0 = tab0[i];
      s1 = tab1[i];
      s2 = tab2[i];
      tab[i-1] = s3;
    }
#ifdef _LITTLE_ENDIAN
    s3 = (s2 << 24) + (s1 << 16) + (s0 << 8);
#else
    s3 = (s0 << 16) + (s1 << 8) + s2;
#endif
    tab[255] = s3;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u8  *dp = dst;
      mlib_u8  *sa = src, *ptr;
      mlib_u32 s0, s1, t0, t1;
      mlib_u32 res1, res2;
      mlib_s32 size = xsize, off;

      off = (mlib_s32)((mlib_addr)dp & 3);

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i < off; i++) {
        ptr = (mlib_u8*)(tab + sa[0]);
        dp[0] = ptr[1];
        dp[1] = ptr[2];
        dp[2] = ptr[3];
        dp += 3; sa ++;
      }

      size -= off;
      da = (mlib_u32*)dp;
      s0 = sa[0];
      s1 = sa[1];
      sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for(i = 0; i < size - 7; i+=4, da+=3, sa += 4) {
        t0 = tab[s0];
        t1 = tab[s1];
#ifdef _LITTLE_ENDIAN
        da[0] = (t0 >> 8) + (t1 << 16);
        res2 = (t1 >> 16);
#else
        da[0] = (t0 << 8) + (t1 >> 16);
        res2 = (t1 << 16);
#endif
        s0 = sa[0];
        s1 = sa[1];
        t0 = tab[s0];
        t1 = tab[s1];
#ifdef _LITTLE_ENDIAN
        res2 += (t0 << 8);
        res1 = (t0 >> 24) + t1;
#else
        res2 += (t0 >> 8);
        res1 = (t0 << 24) + t1;
#endif
        s0 = sa[2];
        s1 = sa[3];
        da[1] = res2;
        da[2] = res1;
      }
      t0 = tab[s0];
      t1 = tab[s1];
#ifdef _LITTLE_ENDIAN
      da[0] = (t0 >> 8) + (t1 << 16);
      res2 = (t1 >> 16);
#else
      da[0] = (t0 << 8) + (t1 >> 16);
      res2 = (t1 << 16);
#endif
      s0 = sa[0];
      s1 = sa[1];
      t0 = tab[s0];
      t1 = tab[s1];
#ifdef _LITTLE_ENDIAN
      res2 += (t0 << 8);
      res1 = (t0 >> 24) + t1;
#else
      res2 += (t0 >> 8);
      res1 = (t0 << 24) + t1;
#endif
      da[1] = res2;
      da[2] = res1;
      da += 3;
      sa += 2;
      dp = (mlib_u8*)da;
      i += 4;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (; i < size; i++) {
        ptr = (mlib_u8*)(tab + sa[0]);
        dp[0] = ptr[1];
        dp[1] = ptr[2];
        dp[2] = ptr[3];
        dp += 3; sa ++;
      }
    }
  } else if (csize == 4) {
     mlib_u32 tab[256];
     mlib_u8  *tab0 = table[0];
     mlib_u8  *tab1 = table[1];
     mlib_u8  *tab2 = table[2];
     mlib_u8  *tab3 = table[3];
     mlib_s32 i, j;
     mlib_u32 s0, s1, s2, s3, s4;

    s0 = tab0[0];
    s1 = tab1[0];
    s2 = tab2[0];
    s3 = tab3[0];
    for (i = 1; i < 256; i++) {
#ifdef _LITTLE_ENDIAN
      s4 = (s3 << 24) + (s2 << 16) + (s1 << 8) + s0;
#else
      s4 = (s0 << 24) + (s1 << 16) + (s2 << 8) + s3;
#endif
      s0 = tab0[i];
      s1 = tab1[i];
      s2 = tab2[i];
      s3 = tab3[i];
      tab[i-1] = s4;
    }
#ifdef _LITTLE_ENDIAN
    s4 = (s3 << 24) + (s2 << 16) + (s1 << 8) + s0;
#else
    s4 = (s0 << 24) + (s1 << 16) + (s2 << 8) + s3;
#endif
    tab[255] = s4;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u8  *dp = dst;
      mlib_u8  *sa = src;
      mlib_u32 s0, t0, s1, t1, t2;
      mlib_s32 size = xsize, off;
      mlib_u32 shift, shift1, res1, res2;

      if (((mlib_addr)dp & 3) == 0) {

        da = (mlib_u32*)dp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 3; i+=2, da += 2, sa += 2) {
          t0 = tab[s0];
          t1 = tab[s1];
          s0 = sa[0];
          s1 = sa[1];
          da[0] = t0;
          da[1] = t1;
        }
        t0 = tab[s0];
        t1 = tab[s1];
        da[0] = t0;
        da[1] = t1;
        if (size & 1) da[2] = tab[sa[0]];

      } else {

        off = (mlib_s32)(4 - ((mlib_addr)dp & 3));
        shift = 8*off;
        shift1 = 32 - shift;

        for (i = 0; i < off; i++) {
          dp[i] = table[i][sa[0]];
        }

        dp += i;
        t0 = tab[sa[0]]; sa++;

        da = (mlib_u32*)dp;

        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 4; i+=2, da += 2, sa += 2) {
          t1 = tab[s0];
          t2 = tab[s1];
#ifdef _LITTLE_ENDIAN
          res1 = (t0 >> shift) + (t1 << shift1);
          res2 = (t1 >> shift) + (t2 << shift1);
#else
          res1 = (t0 << shift) + (t1 >> shift1);
          res2 = (t1 << shift) + (t2 >> shift1);
#endif
          t0 = t2;
          s0 = sa[0];
          s1 = sa[1];
          da[0] = res1;
          da[1] = res2;
        }
        t1 = tab[s0];
        t2 = tab[s1];
#ifdef _LITTLE_ENDIAN
        res1 = (t0 >> shift) + (t1 << shift1);
        res2 = (t1 >> shift) + (t2 << shift1);
#else
        res1 = (t0 << shift) + (t1 >> shift1);
        res2 = (t1 << shift) + (t2 >> shift1);
#endif
        da[0] = res1;
        da[1] = res2;
#ifdef _LITTLE_ENDIAN
        t0 = (da[2] >> shift1);
        da[2] = (t2 >> shift) + (t0 << shift1);
#else
        t0 = (da[2] << shift1);
        da[2] = (t2 << shift) + (t0 >> shift1);
#endif
        da += 2;
        dp = (mlib_u8*)da + (4 - off);

        if ((size & 1) == 0) {
          t0 = tab[sa[0]];
#ifdef _LITTLE_ENDIAN
          dp[3] = (mlib_u8)(t0 >> 24);
          dp[2] = (mlib_u8)(t0 >> 16);
          dp[1] = (mlib_u8)(t0 >> 8);
          dp[0] = (mlib_u8)t0;
#else
          dp[0] = (mlib_u8)(t0 >> 24);
          dp[1] = (mlib_u8)(t0 >> 16);
          dp[2] = (mlib_u8)(t0 >> 8);
          dp[3] = (mlib_u8)t0;
#endif
        }
      }
    }
  }
}

/***************************************************************/

void mlib_c_ImageLookUpSI_S16_U8(mlib_s16 *src, mlib_s32 slb,
                                 mlib_u8  *dst, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s32 csize,
                                 mlib_u8  **table)
{
  mlib_u8 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][32768];
  }

  if ((xsize < 8) || (csize == 2)) {
    MLIB_C_IMAGELOOKUPSI(mlib_u8, mlib_s16, table_base)
  } else if (csize == 3) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u8  *dp = dst;
      mlib_s16 *sa = src;
      mlib_u8  *tab0 = table_base[0];
      mlib_u8  *tab1 = table_base[1];
      mlib_u8  *tab2 = table_base[2];
      mlib_s32 s0, s1;
      mlib_u32 t0, t1, t2, t3, t4, t5;
      mlib_u32 res1, res2;
      mlib_s32 size = xsize, off;

      off = (mlib_s32)((mlib_addr)dp & 3);

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = 0; i < off; i++) {
        s0 = *sa++;
        dp[0] = tab0[s0];
        dp[1] = tab1[s0];
        dp[2] = tab2[s0];
        dp += 3;
      }

      size -= off;
      da = (mlib_u32*)dp;
      s0 = sa[0];
      s1 = sa[1];
      sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for(i = 0; i < size - 7; i+=4, da+=3, sa += 4) {
        t0 = tab0[s0];
        t1 = tab1[s0];
        t2 = tab2[s0];
        t3 = tab0[s1];
        t4 = tab1[s1];
        t5 = tab2[s1];
#ifdef _LITTLE_ENDIAN
        da[0] = (t3 << 24) + (t2 << 16) + (t1 << 8) + t0;
        res2 = (t5 << 8) + t4;
#else
        da[0] = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
        res2 = (t4 << 24) + (t5 << 16);
#endif
        s0 = sa[0];
        s1 = sa[1];
        t0 = tab0[s0];
        t1 = tab1[s0];
        t2 = tab2[s0];
        t3 = tab0[s1];
        t4 = tab1[s1];
        t5 = tab2[s1];
#ifdef _LITTLE_ENDIAN
        res2 += ((t1 << 24) + (t0 << 16));
        res1 = (t5 << 24) + (t4 << 16) + (t3 << 8) + t2;
#else
        res2 += ((t0 << 8) + t1);
        res1 = (t2 << 24) + (t3 << 16) + (t4 << 8) + t5;
#endif
        s0 = sa[2];
        s1 = sa[3];
        da[1] = res2;
        da[2] = res1;
      }
      t0 = tab0[s0];
      t1 = tab1[s0];
      t2 = tab2[s0];
      t3 = tab0[s1];
      t4 = tab1[s1];
      t5 = tab2[s1];
#ifdef _LITTLE_ENDIAN
      da[0] = (t3 << 24) + (t2 << 16) + (t1 << 8) + t0;
      res2 = (t5 << 8) + t4;
#else
      da[0] = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
      res2 = (t4 << 24) + (t5 << 16);
#endif
      s0 = sa[0];
      s1 = sa[1];
      t0 = tab0[s0];
      t1 = tab1[s0];
      t2 = tab2[s0];
      t3 = tab0[s1];
      t4 = tab1[s1];
      t5 = tab2[s1];
#ifdef _LITTLE_ENDIAN
      res2 += ((t1 << 24) + (t0 << 16));
      res1 = (t5 << 24) + (t4 << 16) + (t3 << 8) + t2;
#else
      res2 += ((t0 << 8) + t1);
      res1 = (t2 << 24) + (t3 << 16) + (t4 << 8) + t5;
#endif
      da[1] = res2;
      da[2] = res1;
      da += 3;
      sa += 2;
      dp = (mlib_u8*)da;
      i += 4;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (; i < size; i++) {
        s0 = *sa++;
        dp[0] = tab0[s0];
        dp[1] = tab1[s0];
        dp[2] = tab2[s0];
        dp += 3;
      }
    }
  } else if (csize == 4) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u8  *dp = dst;
      mlib_s16 *sa = src;
      mlib_u8  *tab0 = table_base[0];
      mlib_u8  *tab1 = table_base[1];
      mlib_u8  *tab2 = table_base[2];
      mlib_u8  *tab3 = table_base[3];
      mlib_s32 s0;
      mlib_u32 t0, t1, t2, t3;
      mlib_s32 size = xsize, off;
      mlib_u32 shift, shift1, res1, res2, res;

      if (((mlib_addr)dp & 3) == 0) {

        da = (mlib_u32*)dp;

        s0 = sa[0];
        sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 1; i++, da++, sa ++) {
          t0 = tab0[s0];
          t1 = tab1[s0];
          t2 = tab2[s0];
          t3 = tab3[s0];
#ifdef _LITTLE_ENDIAN
          res = (t3 << 24) + (t2 << 16) + (t1 << 8) + t0;
#else
          res = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
#endif
          s0 = sa[0];
          da[0] = res;
        }
        t0 = tab0[s0];
        t1 = tab1[s0];
        t2 = tab2[s0];
        t3 = tab3[s0];
#ifdef _LITTLE_ENDIAN
        res = (t3 << 24) + (t2 << 16) + (t1 << 8) + t0;
#else
        res = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
#endif
        da[0] = res;

      } else {

        off = (mlib_s32)(4 - ((mlib_addr)dp & 3));
        shift = 8*off;
        shift1 = 32 - shift;

        s0 = *sa++;

        for (i = 0; i < off; i++) {
          dp[i] = table_base[i][s0];
        }

        dp += i;
        da = (mlib_u32*)dp;

        t0 = tab0[s0];
        t1 = tab1[s0];
        t2 = tab2[s0];
        t3 = tab3[s0];

#ifdef _LITTLE_ENDIAN
        res1 = (t3 << 24) + (t2 << 16) + (t1 << 8) + t0;
#else
        res1 = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
#endif

        s0 = sa[0];
        sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 2; i++, da++, sa++) {
          t0 = tab0[s0];
          t1 = tab1[s0];
          t2 = tab2[s0];
          t3 = tab3[s0];
#ifdef _LITTLE_ENDIAN
          res2 = (t3 << 24) + (t2 << 16) + (t1 << 8) + t0;
          res = (res1 >> shift) + (res2 << shift1);
#else
          res2 = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
          res = (res1 << shift) + (res2 >> shift1);
#endif
          res1 = res2;
          s0 = sa[0];
          da[0] = res;
        }
        t0 = tab0[s0];
        t1 = tab1[s0];
        t2 = tab2[s0];
        t3 = tab3[s0];
#ifdef _LITTLE_ENDIAN
          res2 = (t3 << 24) + (t2 << 16) + (t1 << 8) + t0;
          res = (res1 >> shift) + (res2 << shift1);
#else
          res2 = (t0 << 24) + (t1 << 16) + (t2 << 8) + t3;
          res = (res1 << shift) + (res2 >> shift1);
#endif
        da[0] = res;
#ifdef _LITTLE_ENDIAN
        res1 = (da[1] >> shift1);
        da[1] = (res2 >> shift) + (res1 << shift1);
#else
        res1 = (da[1] << shift1);
        da[1] = (res2 << shift) + (res1 >> shift1);
#endif

      }
    }
  }
}

/***************************************************************/

void mlib_c_ImageLookUpSI_S32_U8(mlib_s32 *src,  mlib_s32 slb,
                                 mlib_u8 *dst,   mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s32 csize,
                                 mlib_u8 **table)
{
  mlib_u8 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][TABLE_SHIFT_S32];
  }

  MLIB_C_IMAGELOOKUPSI(mlib_u8, mlib_s32, table_base)
}
/***************************************************************/

void mlib_c_ImageLookUpSI_U8_S16(mlib_u8  *src, mlib_s32 slb,
                                 mlib_s16 *dst, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s32 csize,
                                 mlib_s16 **table)
{
  if ((xsize < 4) || ((xsize*ysize) < 250)) {
   MLIB_C_IMAGELOOKUPSI(mlib_s16, mlib_u8, table)

  } else if (csize == 2) {
    mlib_u32 tab[256];
    mlib_u16 *tab0 = (mlib_u16*)table[0];
    mlib_u16 *tab1 = (mlib_u16*)table[1];
    mlib_s32 i, j;
    mlib_u32 s0, s1, s2;

    s0 = tab0[0];
    s1 = tab1[0];
    for (i = 1; i < 256; i++) {
#ifdef _LITTLE_ENDIAN
      s2 = (s1 << 16) + s0;
#else
      s2 = (s0 << 16) + s1;
#endif
      s0 = tab0[i];
      s1 = tab1[i];
      tab[i-1] = s2;
    }
#ifdef _LITTLE_ENDIAN
    s2 = (s1 << 16) + s0;
#else
    s2 = (s0 << 16) + s1;
#endif
    tab[255] = s2;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u16 *dp = (mlib_u16*)dst;
      mlib_u8  *sa = src;
      mlib_u32 s0, t0, s1, t1, t2;
      mlib_u32 res1, res2;
      mlib_s32 size = xsize;

      if (((mlib_addr)dp & 3) == 0) {

        da = (mlib_u32*)dp;
        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 3; i+=2, da+=2, sa+=2) {
          t0 = tab[s0];
          t1 = tab[s1];
          s0 = sa[0];
          s1 = sa[1];
          da[0] = t0;
          da[1] = t1;
        }
        t0 = tab[s0];
        t1 = tab[s1];
        da[0] = t0;
        da[1] = t1;
        if (size & 1) da[2] = tab[sa[0]];

      } else {

        t0 = tab[*sa++];
#ifdef _LITTLE_ENDIAN
        *dp++ = (mlib_u16)(t0);
#else
        *dp++ = (mlib_u16)(t0 >> 16);
#endif
        da = (mlib_u32*)dp;
        s0 = sa[0];
        s1 = sa[1];
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 4; i+=2, da+=2, sa+=2) {
          t1 = tab[s0];
          t2 = tab[s1];
#ifdef _LITTLE_ENDIAN
          res1 = (t0 >> 16) + (t1 << 16);
          res2 = (t1 >> 16) + (t2 << 16);
#else
          res1 = (t0 << 16) + (t1 >> 16);
          res2 = (t1 << 16) + (t2 >> 16);
#endif
          t0 = t2;
          s0 = sa[0];
          s1 = sa[1];
          da[0] = res1;
          da[1] = res2;
        }
        t1 = tab[s0];
        t2 = tab[s1];
#ifdef _LITTLE_ENDIAN
        res1 = (t0 >> 16) + (t1 << 16);
        res2 = (t1 >> 16) + (t2 << 16);
#else
        res1 = (t0 << 16) + (t1 >> 16);
        res2 = (t1 << 16) + (t2 >> 16);
#endif
        da[0] = res1;
        da[1] = res2;
        da += 2;
        dp = (mlib_u16*)da;
#ifdef _LITTLE_ENDIAN
        dp[0] = (mlib_u16)(t2 >> 16);
#else
        dp[0] = (mlib_u16)t2;
#endif
        if ((size & 1) == 0) {
          t0 = tab[sa[0]];
#ifdef _LITTLE_ENDIAN
          dp[2] = (mlib_u16)(t0 >> 16);
          dp[1] = (mlib_u16)t0;
#else
          dp[1] = (mlib_u16)(t0 >> 16);
          dp[2] = (mlib_u16)t0;
#endif
        }
      }
    }
  } else if (csize == 3) {
    mlib_u32 tab[512];
    mlib_u16 *tab0 = (mlib_u16*)table[0];
    mlib_u16 *tab1 = (mlib_u16*)table[1];
    mlib_u16 *tab2 = (mlib_u16*)table[2];
    mlib_s32 i, j;
    mlib_u32 s0, s1, s2, s3, s4;

    s0 = tab0[0];
    s1 = tab1[0];
    s2 = tab2[0];
    for (i = 1; i < 256; i++) {
#ifdef _LITTLE_ENDIAN
      s3 = (s0 << 16);
      s4 = (s2 << 16) + s1;
#else
      s3 = s0;
      s4 = (s1 << 16) + s2;
#endif
      s0 = tab0[i];
      s1 = tab1[i];
      s2 = tab2[i];
      tab[2*i-2] = s3;
      tab[2*i-1] = s4;
    }
#ifdef _LITTLE_ENDIAN
    s4 = (s2 << 16) + s1;
    tab[510] = s0 << 16;
#else
    s4 = (s1 << 16) + s2;
    tab[510] = s0;
#endif
    tab[511] = s4;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u16 *dp = (mlib_u16*)dst, *ptr;
      mlib_u8  *sa = src;
      mlib_u32 s0, s1, t0, t1, t2, t3;
      mlib_u32 res1, res2;
      mlib_s32 size = xsize, off;

      off = (mlib_s32)((mlib_addr)dp & 3);

      if (off != 0) {
        ptr = (mlib_u16*)(tab + 2*sa[0]);
        dp[0] = ptr[1];
        dp[1] = ptr[2];
        dp[2] = ptr[3];
        dp += 3; sa ++;
        size--;
      }

      da = (mlib_u32*)dp;
      s0 = sa[0] << 3;
      s1 = sa[1] << 3;
      sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for(i = 0; i < size - 3; i+=2, da+=3, sa += 2) {
        t0 = *(mlib_u32*)((mlib_u8*)tab + s0);
        t1 = *(mlib_u32*)((mlib_u8*)tab + s0 + 4);
        t2 = *(mlib_u32*)((mlib_u8*)tab + s1);
        t3 = *(mlib_u32*)((mlib_u8*)tab + s1 + 4);
#ifdef _LITTLE_ENDIAN
        res1 = (t0 >> 16) + (t1 << 16);
        res2 = (t1 >> 16) + t2;
#else
        res1 = (t0 << 16) + (t1 >> 16);
        res2 = (t1 << 16) + t2;
#endif
        s0 = sa[0] << 3;
        s1 = sa[1] << 3;
        da[0] = res1;
        da[1] = res2;
        da[2] = t3;
      }
      t0 = *(mlib_u32*)((mlib_u8*)tab + s0);
      t1 = *(mlib_u32*)((mlib_u8*)tab + s0 + 4);
      t2 = *(mlib_u32*)((mlib_u8*)tab + s1);
      t3 = *(mlib_u32*)((mlib_u8*)tab + s1 + 4);
#ifdef _LITTLE_ENDIAN
      res1 = (t0 >> 16) + (t1 << 16);
      res2 = (t1 >> 16) + t2;
#else
      res1 = (t0 << 16) + (t1 >> 16);
      res2 = (t1 << 16) + t2;
#endif
      da[0] = res1;
      da[1] = res2;
      da[2] = t3;
      da += 3;
      dp = (mlib_u16*)da;
      i += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      if (i < size) {
        ptr = (mlib_u16*)(tab + 2*sa[0]);
        dp[0] = ptr[1];
        dp[1] = ptr[2];
        dp[2] = ptr[3];
      }
    }
  } else if (csize == 4) {
    mlib_u32 tab[512];
    mlib_u16 *tab0 = (mlib_u16*)table[0];
    mlib_u16 *tab1 = (mlib_u16*)table[1];
    mlib_u16 *tab2 = (mlib_u16*)table[2];
    mlib_u16 *tab3 = (mlib_u16*)table[3];
    mlib_s32 i, j;
    mlib_u32 s0, s1, s2, s3, s4, s5;

    s0 = tab0[0];
    s1 = tab1[0];
    s2 = tab2[0];
    s3 = tab3[0];
    for (i = 1; i < 256; i++) {
#ifdef _LITTLE_ENDIAN
      s4 = (s1 << 16) + s0;
      s5 = (s3 << 16) + s2;
#else
      s4 = (s0 << 16) + s1;
      s5 = (s2 << 16) + s3;
#endif
      s0 = tab0[i];
      s1 = tab1[i];
      s2 = tab2[i];
      s3 = tab3[i];
      tab[2*i-2] = s4;
      tab[2*i-1] = s5;
    }
#ifdef _LITTLE_ENDIAN
    s4 = (s1 << 16) + s0;
    s5 = (s3 << 16) + s2;
#else
    s4 = (s0 << 16) + s1;
    s5 = (s2 << 16) + s3;
#endif
    tab[510] = s4;
    tab[511] = s5;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *da;
      mlib_u16 *dp = (mlib_u16*)dst;
      mlib_u8  *sa = src;
      mlib_u32 s0, t0, s1, t1, t2, t3, t4, t5;
      mlib_s32 size = xsize;
      mlib_u32 res1, res2, res3, res4;

      if (((mlib_addr)dp & 3) == 0) {

        da = (mlib_u32*)dp;

        s0 = sa[0] << 3;
        s1 = sa[1] << 3;
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 3; i+=2, da += 4, sa += 2) {
          t0 = *(mlib_u32*)((mlib_u8*)tab + s0);
          t1 = *(mlib_u32*)((mlib_u8*)tab + s0 + 4);
          t2 = *(mlib_u32*)((mlib_u8*)tab + s1);
          t3 = *(mlib_u32*)((mlib_u8*)tab + s1 + 4);
          s0 = sa[0] << 3;
          s1 = sa[1] << 3;
          da[0] = t0;
          da[1] = t1;
          da[2] = t2;
          da[3] = t3;
        }
        t0 = *(mlib_u32*)((mlib_u8*)tab + s0);
        t1 = *(mlib_u32*)((mlib_u8*)tab + s0 + 4);
        t2 = *(mlib_u32*)((mlib_u8*)tab + s1);
        t3 = *(mlib_u32*)((mlib_u8*)tab + s1 + 4);
        da[0] = t0;
        da[1] = t1;
        da[2] = t2;
        da[3] = t3;
        if (size & 1) {
          da[4] = tab[2*sa[0]];
          da[5] = tab[2*sa[0]+1];
        }
      } else {

        t4 = tab[2*sa[0]];
        t5 = tab[2*sa[0] + 1];
#ifdef _LITTLE_ENDIAN
        *dp++ = (mlib_u16)(t4);
#else
        *dp++ = (mlib_u16)(t4 >> 16);
#endif
        sa++;
        da = (mlib_u32*)dp;
#ifdef _LITTLE_ENDIAN
        *da++ = (t4 >> 16) + (t5 << 16);
#else
        *da++ = (t4 << 16) + (t5 >> 16);
#endif
        s0 = sa[0] << 3;
        s1 = sa[1] << 3;
        sa += 2;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
        for(i = 0; i < size - 4; i+=2, da += 4, sa += 2) {
          t0 = *(mlib_u32*)((mlib_u8*)tab + s0);
          t1 = *(mlib_u32*)((mlib_u8*)tab + s0 + 4);
          t2 = *(mlib_u32*)((mlib_u8*)tab + s1);
          t3 = *(mlib_u32*)((mlib_u8*)tab + s1 + 4);
#ifdef _LITTLE_ENDIAN
          res1 = (t5 >> 16) + (t0 << 16);
          res2 = (t0 >> 16) + (t1 << 16);
          res3 = (t1 >> 16) + (t2 << 16);
          res4 = (t2 >> 16) + (t3 << 16);
#else
          res1 = (t5 << 16) + (t0 >> 16);
          res2 = (t0 << 16) + (t1 >> 16);
          res3 = (t1 << 16) + (t2 >> 16);
          res4 = (t2 << 16) + (t3 >> 16);
#endif
          s0 = sa[0] << 3;
          s1 = sa[1] << 3;
          da[0] = res1;
          da[1] = res2;
          da[2] = res3;
          da[3] = res4;
          t5 = t3;
        }
        t0 = *(mlib_u32*)((mlib_u8*)tab + s0);
        t1 = *(mlib_u32*)((mlib_u8*)tab + s0 + 4);
        t2 = *(mlib_u32*)((mlib_u8*)tab + s1);
        t3 = *(mlib_u32*)((mlib_u8*)tab + s1 + 4);
#ifdef _LITTLE_ENDIAN
        res1 = (t5 >> 16) + (t0 << 16);
        res2 = (t0 >> 16) + (t1 << 16);
        res3 = (t1 >> 16) + (t2 << 16);
        res4 = (t2 >> 16) + (t3 << 16);
#else
        res1 = (t5 << 16) + (t0 >> 16);
        res2 = (t0 << 16) + (t1 >> 16);
        res3 = (t1 << 16) + (t2 >> 16);
        res4 = (t2 << 16) + (t3 >> 16);
#endif
        da[0] = res1;
        da[1] = res2;
        da[2] = res3;
        da[3] = res4;
        da += 4;
        dp = (mlib_u16*)da;
#ifdef _LITTLE_ENDIAN
        dp[0] = (mlib_u16)(t3 >> 16);
#else
        dp[0] = (mlib_u16)t3;
#endif
        if ((size & 1) == 0) {
          t0 = tab[2*sa[0]];
#ifdef _LITTLE_ENDIAN
          dp[2] = (mlib_u16)(t0 >> 16);
          dp[1] = (mlib_u16)t0;
#else
          dp[1] = (mlib_u16)(t0 >> 16);
          dp[2] = (mlib_u16)t0;
#endif
          t0 = tab[2*sa[0]+1];
#ifdef _LITTLE_ENDIAN
          dp[4] = (mlib_u16)(t0 >> 16);
          dp[3] = (mlib_u16)t0;
#else
          dp[3] = (mlib_u16)(t0 >> 16);
          dp[4] = (mlib_u16)t0;
#endif
        }
      }
    }
  }
}

/***************************************************************/

void mlib_c_ImageLookUpSI_S16_S16(mlib_s16 *src, mlib_s32 slb,
                                  mlib_s16 *dst, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s32 csize,
                                  mlib_s16 **table)
{
  mlib_s16 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][32768];
  }

  MLIB_C_IMAGELOOKUPSI(mlib_s16, mlib_s16, table_base)
}

/***************************************************************/

void mlib_c_ImageLookUpSI_S32_S16(mlib_s32 *src, mlib_s32 slb,
                                  mlib_s16 *dst, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s32 csize,
                                  mlib_s16 **table)
{
  mlib_s16 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][TABLE_SHIFT_S32];
  }

  MLIB_C_IMAGELOOKUPSI(mlib_s16, mlib_s32, table_base)
}

/***************************************************************/

void mlib_c_ImageLookUpSI_U8_S32(mlib_u8  *src, mlib_s32 slb,
                                 mlib_s32 *dst, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s32 csize,
                                 mlib_s32 **table)
{
  if (xsize < 7) {
    MLIB_C_IMAGELOOKUPSI(mlib_s32, mlib_u8, table)
  } else if (csize == 2) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *tab0 = (mlib_u32*) table[0];
      mlib_u32 *tab1 = (mlib_u32*) table[1];
      mlib_u32 s0, t0, t1, t2, t3;
      mlib_s32 off;
      mlib_s32 size = xsize;
      mlib_u32 *dp = (mlib_u32*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      for (i = 0; i < off; i++, sp++) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[0]];
        size--;
      }

      sa = (mlib_u32*)sp;

      s0 = sa[0];
      sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for(i = 0; i < size - 7; i+=4, dp+=8, sa++) {
#ifdef _LITTLE_ENDIAN
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2) & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2) & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6) & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6) & 0x3FC));
#else
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22) & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22) & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14) & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14) & 0x3FC));
#endif
        dp[0] = t0;
        dp[1] = t1;
        dp[2] = t2;
        dp[3] = t3;
#ifdef _LITTLE_ENDIAN
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14)  & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14)  & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22)  & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22)  & 0x3FC));
#else
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6)  & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6)  & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2)  & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2)  & 0x3FC));
#endif
        s0 = sa[0];
        dp[4] = t0;
        dp[5] = t1;
        dp[6] = t2;
        dp[7] = t3;
      }
#ifdef _LITTLE_ENDIAN
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2) & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2) & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6) & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6) & 0x3FC));
#else
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22) & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22) & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14) & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14) & 0x3FC));
#endif
      dp[0] = t0;
      dp[1] = t1;
      dp[2] = t2;
      dp[3] = t3;
#ifdef _LITTLE_ENDIAN
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14)  & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14)  & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22)  & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22)  & 0x3FC));
#else
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6)  & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6)  & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2)  & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2)  & 0x3FC));
#endif
      dp[4] = t0;
      dp[5] = t1;
      dp[6] = t2;
      dp[7] = t3;
      dp += 8;
      sp = (mlib_u8*)sa;
      i += 4;

      for (; i < size; i++, sp++) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[0]];
      }
    }
  } else if (csize == 3) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *tab0 = (mlib_u32*) table[0];
      mlib_u32 *tab1 = (mlib_u32*) table[1];
      mlib_u32 *tab2 = (mlib_u32*) table[2];
      mlib_u32 s0, t0, t1, t2, t3, t4, t5;
      mlib_s32 off;
      mlib_s32 size = xsize;
      mlib_u32 *dp = (mlib_u32*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      for (i = 0; i < off; i++, sp++) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[0]];
        *dp++ = tab2[sp[0]];
        size--;
      }

      sa = (mlib_u32*)sp;

      s0 = sa[0];
      sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for(i = 0; i < size - 7; i+=4, dp+=12, sa++) {
#ifdef _LITTLE_ENDIAN
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2) & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2) & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 << 2) & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6) & 0x3FC));
        t4 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6) & 0x3FC));
        t5 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 6) & 0x3FC));
#else
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22) & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22) & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 22) & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14) & 0x3FC));
        t4 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14) & 0x3FC));
        t5 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 14) & 0x3FC));
#endif
        dp[0] = t0;
        dp[1] = t1;
        dp[2] = t2;
        dp[3] = t3;
        dp[4] = t4;
        dp[5] = t5;
#ifdef _LITTLE_ENDIAN
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14)  & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14)  & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 14)  & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22)  & 0x3FC));
        t4 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22)  & 0x3FC));
        t5 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 22)  & 0x3FC));
#else
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6)  & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6)  & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 6)  & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2)  & 0x3FC));
        t4 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2)  & 0x3FC));
        t5 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 << 2)  & 0x3FC));
#endif
        s0 = sa[0];
        dp[6] = t0;
        dp[7] = t1;
        dp[8] = t2;
        dp[9] = t3;
        dp[10] = t4;
        dp[11] = t5;
      }
#ifdef _LITTLE_ENDIAN
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2) & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2) & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 << 2) & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6) & 0x3FC));
      t4 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6) & 0x3FC));
      t5 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 6) & 0x3FC));
#else
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22) & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22) & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 22) & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14) & 0x3FC));
      t4 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14) & 0x3FC));
      t5 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 14) & 0x3FC));
#endif
      dp[0] = t0;
      dp[1] = t1;
      dp[2] = t2;
      dp[3] = t3;
      dp[4] = t4;
      dp[5] = t5;
#ifdef _LITTLE_ENDIAN
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14)  & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14)  & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 14)  & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22)  & 0x3FC));
      t4 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22)  & 0x3FC));
      t5 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 22)  & 0x3FC));
#else
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6)  & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6)  & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 6)  & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2)  & 0x3FC));
      t4 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2)  & 0x3FC));
      t5 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 << 2)  & 0x3FC));
#endif
      dp[6] = t0;
      dp[7] = t1;
      dp[8] = t2;
      dp[9] = t3;
      dp[10] = t4;
      dp[11] = t5;
      dp += 12;
      sp = (mlib_u8*)sa;
      i += 4;

      for (; i < size; i++, sp++) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[0]];
        *dp++ = tab2[sp[0]];
      }
    }
  } else if (csize == 4) {
    mlib_s32 i, j;

    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
      mlib_u32 *sa;
      mlib_u32 *tab0 = (mlib_u32*) table[0];
      mlib_u32 *tab1 = (mlib_u32*) table[1];
      mlib_u32 *tab2 = (mlib_u32*) table[2];
      mlib_u32 *tab3 = (mlib_u32*) table[3];
      mlib_u32 s0, t0, t1, t2, t3;
      mlib_s32 off;
      mlib_s32 size = xsize;
      mlib_u32 *dp = (mlib_u32*)dst;
      mlib_u8  *sp = src;

      off = (mlib_s32)((4 - ((mlib_addr)src & 3)) & 3);

      for (i = 0; i < off; i++, sp++) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[0]];
        *dp++ = tab2[sp[0]];
        *dp++ = tab3[sp[0]];
        size--;
      }

      sa = (mlib_u32*)sp;

      s0 = sa[0];
      sa++;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for(i = 0; i < size - 7; i+=4, dp+=16, sa++) {
#ifdef _LITTLE_ENDIAN
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2) & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2) & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 << 2) & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 << 2) & 0x3FC));
#else
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22) & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22) & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 22) & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 22) & 0x3FC));
#endif
        dp[0] = t0;
        dp[1] = t1;
        dp[2] = t2;
        dp[3] = t3;
#ifdef _LITTLE_ENDIAN
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6)  & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6)  & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 6)  & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 6)  & 0x3FC));
#else
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14) & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14) & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 14) & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 14) & 0x3FC));
#endif
        dp[4] = t0;
        dp[5] = t1;
        dp[6] = t2;
        dp[7] = t3;
#ifdef _LITTLE_ENDIAN
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14)  & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14)  & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 14)  & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 14)  & 0x3FC));
#else
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6)  & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6)  & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 6)  & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 6)  & 0x3FC));
#endif
        dp[8] = t0;
        dp[9] = t1;
        dp[10] = t2;
        dp[11] = t3;
#ifdef _LITTLE_ENDIAN
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22)  & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22)  & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 22)  & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 22)  & 0x3FC));
#else
        t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2)  & 0x3FC));
        t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2)  & 0x3FC));
        t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 << 2)  & 0x3FC));
        t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 << 2)  & 0x3FC));
#endif
        s0 = sa[0];
        dp[12] = t0;
        dp[13] = t1;
        dp[14] = t2;
        dp[15] = t3;
      }
#ifdef _LITTLE_ENDIAN
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2) & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2) & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 << 2) & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 << 2) & 0x3FC));
#else
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22) & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22) & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 22) & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 22) & 0x3FC));
#endif
      dp[0] = t0;
      dp[1] = t1;
      dp[2] = t2;
      dp[3] = t3;
#ifdef _LITTLE_ENDIAN
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6)  & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6)  & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 6)  & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 6)  & 0x3FC));
#else
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14) & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14) & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 14) & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 14) & 0x3FC));
#endif
      dp[4] = t0;
      dp[5] = t1;
      dp[6] = t2;
      dp[7] = t3;
#ifdef _LITTLE_ENDIAN
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 14)  & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 14)  & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 14)  & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 14)  & 0x3FC));
#else
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 6)  & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 6)  & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 6)  & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 6)  & 0x3FC));
#endif
      dp[8] = t0;
      dp[9] = t1;
      dp[10] = t2;
      dp[11] = t3;
#ifdef _LITTLE_ENDIAN
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 >> 22)  & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 >> 22)  & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 >> 22)  & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 >> 22)  & 0x3FC));
#else
      t0 = *(mlib_u32*)((mlib_u8*)tab0 + ((s0 << 2)  & 0x3FC));
      t1 = *(mlib_u32*)((mlib_u8*)tab1 + ((s0 << 2)  & 0x3FC));
      t2 = *(mlib_u32*)((mlib_u8*)tab2 + ((s0 << 2)  & 0x3FC));
      t3 = *(mlib_u32*)((mlib_u8*)tab3 + ((s0 << 2)  & 0x3FC));
#endif
      dp[12] = t0;
      dp[13] = t1;
      dp[14] = t2;
      dp[15] = t3;
      dp += 16;
      sp = (mlib_u8*)sa;
      i += 4;

      for (; i < size; i++, sp++) {
        *dp++ = tab0[sp[0]];
        *dp++ = tab1[sp[0]];
        *dp++ = tab2[sp[0]];
        *dp++ = tab3[sp[0]];
      }
    }
  }
}

/***************************************************************/

void mlib_c_ImageLookUpSI_S16_S32(mlib_s16 *src, mlib_s32 slb,
                                  mlib_s32 *dst, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s32 csize,
                                  mlib_s32 **table)
{
  mlib_s32 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][32768];
  }

  MLIB_C_IMAGELOOKUPSI(mlib_s32, mlib_s16, table_base)
}

/***************************************************************/

void mlib_c_ImageLookUpSI_S32_S32(mlib_s32 *src, mlib_s32 slb,
                                  mlib_s32 *dst, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s32 csize,
                                  mlib_s32 **table)
{
  mlib_s32 *table_base[4];
  int c;

  for (c = 0; c < csize; c++) {
    table_base[c] = &table[c][TABLE_SHIFT_S32];
  }

  MLIB_C_IMAGELOOKUPSI(mlib_s32, mlib_s32, table_base)
}

/***************************************************************/

#ifdef MLIB_TEST

mlib_status mlib_c_ImageLookUp(mlib_image *dst,
                               mlib_image *src,
                               void **table)
#else

mlib_status mlib_ImageLookUp(mlib_image *dst,
                             mlib_image *src,
                             void **table)
#endif

{
  mlib_s32   slb, dlb, xsize, ysize, nchan, ichan;
  mlib_type  stype, dtype;
  void       *sa, *da;

  MLIB_IMAGE_CHECK(src);
  MLIB_IMAGE_CHECK(dst);
  MLIB_IMAGE_SIZE_EQUAL(src, dst);
  MLIB_IMAGE_CHAN_SRC1_OR_EQ(src, dst);

  stype = mlib_ImageGetType(src);
  dtype = mlib_ImageGetType(dst);
  ichan = mlib_ImageGetChannels(src);
  nchan = mlib_ImageGetChannels(dst);
  xsize = mlib_ImageGetWidth(src);
  ysize = mlib_ImageGetHeight(src);
  slb   = mlib_ImageGetStride(src);
  dlb   = mlib_ImageGetStride(dst);
  sa    = mlib_ImageGetData(src);
  da    = mlib_ImageGetData(dst);

  if (ichan == nchan) {
    if (dtype == MLIB_BYTE) {
      if (stype == MLIB_BYTE) {

        mlib_c_ImageLookUp_U8_U8(sa, slb,
                                 da, dlb,
                                 xsize, ysize, nchan,
                                 (mlib_u8 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_c_ImageLookUp_S16_U8(sa, slb/2,
                                  da, dlb,
                                  xsize, ysize, nchan,
                                  (mlib_u8 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_c_ImageLookUp_S32_U8(sa, slb/4,
                                  da, dlb,
                                  xsize, ysize, nchan,
                                  (mlib_u8 **) table);
        return MLIB_SUCCESS;
      }

    } else if (dtype == MLIB_SHORT) {

      if (stype == MLIB_BYTE) {

        mlib_c_ImageLookUp_U8_S16(sa, slb,
                                  da, dlb/2,
                                  xsize, ysize, nchan,
                                  (mlib_s16 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_c_ImageLookUp_S16_S16(sa, slb/2,
                                   da, dlb/2,
                                   xsize, ysize, nchan,
                                   (mlib_s16 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_c_ImageLookUp_S32_S16(sa, slb/4,
                                   da, dlb/2,
                                   xsize, ysize, nchan,
                                   (mlib_s16 **) table);
        return MLIB_SUCCESS;
      }

    } else if (dtype == MLIB_INT) {

      if (stype == MLIB_BYTE) {

        mlib_c_ImageLookUp_U8_S32(sa, slb,
                                  da, dlb/4,
                                  xsize, ysize, nchan,
                                  (mlib_s32 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_c_ImageLookUp_S16_S32(sa, slb/2,
                                   da, dlb/4,
                                   xsize, ysize, nchan,
                                   (mlib_s32 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_c_ImageLookUp_S32_S32(sa, slb/4,
                                   da, dlb/4,
                                   xsize, ysize, nchan,
                                   (mlib_s32 **) table);
        return MLIB_SUCCESS;
      }

    } else if (dtype == MLIB_FLOAT) {

      if (stype == MLIB_BYTE) {

        mlib_c_ImageLookUp_U8_S32(sa, slb,
                                  da, dlb/4,
                                  xsize, ysize, nchan,
                                  (mlib_s32 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_c_ImageLookUp_S16_S32(sa, slb/2,
                                   da, dlb/4,
                                   xsize, ysize, nchan,
                                   (mlib_s32 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_c_ImageLookUp_S32_S32(sa, slb/4,
                                   da, dlb/4,
                                   xsize, ysize, nchan,
                                   (mlib_s32 **) table);
        return MLIB_SUCCESS;
      }

    } else if (dtype == MLIB_DOUBLE) {

      if (stype == MLIB_BYTE) {

        mlib_ImageLookUp_U8_D64(sa, slb,
                                da, dlb/8,
                                xsize, ysize, nchan,
                                (mlib_d64 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_ImageLookUp_S16_D64(sa, slb/2,
                                 da, dlb/8,
                                 xsize, ysize, nchan,
                                 (mlib_d64 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_ImageLookUp_S32_D64(sa, slb/4,
                                 da, dlb/8,
                                 xsize, ysize, nchan,
                                 (mlib_d64 **) table);
        return MLIB_SUCCESS;
      }
    }

  } else if (ichan == 1) {

    if (dtype == MLIB_BYTE) {

      if (stype == MLIB_BYTE) {

        mlib_c_ImageLookUpSI_U8_U8(sa, slb,
                                   da, dlb,
                                   xsize, ysize, nchan,
                                   (mlib_u8 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_c_ImageLookUpSI_S16_U8(sa, slb/2,
                                    da, dlb,
                                    xsize, ysize, nchan,
                                    (mlib_u8 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_c_ImageLookUpSI_S32_U8(sa, slb/4,
                                    da, dlb,
                                    xsize, ysize, nchan,
                                    (mlib_u8 **) table);
        return MLIB_SUCCESS;
      }

    } else if (dtype == MLIB_SHORT) {

      if (stype == MLIB_BYTE) {

        mlib_c_ImageLookUpSI_U8_S16(sa, slb,
                                    da, dlb/2,
                                    xsize, ysize, nchan,
                                    (mlib_s16 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_c_ImageLookUpSI_S16_S16(sa, slb/2,
                                     da, dlb/2,
                                     xsize, ysize, nchan,
                                     (mlib_s16 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_c_ImageLookUpSI_S32_S16(sa, slb/4,
                                     da, dlb/2,
                                     xsize, ysize, nchan,
                                     (mlib_s16 **) table);
        return MLIB_SUCCESS;
      }

    } else if (dtype == MLIB_INT) {

      if (stype == MLIB_BYTE) {

        mlib_c_ImageLookUpSI_U8_S32(sa, slb,
                                    da, dlb/4,
                                    xsize, ysize, nchan,
                                    (mlib_s32 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_c_ImageLookUpSI_S16_S32(sa, slb/2,
                                     da, dlb/4,
                                     xsize, ysize, nchan,
                                     (mlib_s32 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_c_ImageLookUpSI_S32_S32(sa, slb/4,
                                     da, dlb/4,
                                     xsize, ysize, nchan,
                                     (mlib_s32 **) table);
        return MLIB_SUCCESS;
      }

    } else if (dtype == MLIB_FLOAT) {

      if (stype == MLIB_BYTE) {

        mlib_c_ImageLookUpSI_U8_S32(sa, slb,
                                    da, dlb/4,
                                    xsize, ysize, nchan,
                                    (mlib_s32 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_c_ImageLookUpSI_S16_S32(sa, slb/2,
                                     da, dlb/4,
                                     xsize, ysize, nchan,
                                     (mlib_s32 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_c_ImageLookUpSI_S32_S32(sa, slb/4,
                                     da, dlb/4,
                                     xsize, ysize, nchan,
                                     (mlib_s32 **) table);
        return MLIB_SUCCESS;
      }

    } else if (dtype == MLIB_DOUBLE) {

      if (stype == MLIB_BYTE) {

        mlib_ImageLookUpSI_U8_D64(sa, slb,
                                  da, dlb/8,
                                  xsize, ysize, nchan,
                                  (mlib_d64 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_ImageLookUpSI_S16_D64(sa, slb/2,
                                   da, dlb/8,
                                   xsize, ysize, nchan,
                                   (mlib_d64 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_ImageLookUpSI_S32_D64(sa, slb/4,
                                   da, dlb/8,
                                   xsize, ysize, nchan,
                                   (mlib_d64 **) table);
        return MLIB_SUCCESS;
      }
    }
  }

  return MLIB_FAILURE;
}

/***************************************************************/
