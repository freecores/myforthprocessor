/*
 * @(#)mlib_v_ImageCopy_f.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageCopy_f.c	1.16	00/03/14 SMI"

/*
 * FUNCTIONS
 *      mlib_v_ImageCopy_a1         - 1-D, Aligned8, size 8x
 *      mlib_v_ImageCopy_a2         - 2-D, Aligned8, width 8x
 *      mlib_ImageCopy_na           - BYTE, non-aligned
 *      mlib_v_ImageCopy_bit_al     - BIT, aligned
 *
 * SYNOPSIS
 *
 * ARGUMENT
 *      sp       pointer to source image data
 *      dp       pointer to destination image data
 *      size     size in 8-bytes, bytes, or SHORTs
 *      width    image width in 8-bytes
 *      height   image height in lines
 *      stride   source image line stride in 8-bytes
 *      dstride  destination image line stride in 8-bytes
 *      s_offset source image line bit offset
 *      d_offset destination image line bit offset
 *
 * DESCRIPTION
 *      Direct copy from one image to another -- VIS version low level
 *      functions.
 *
 * NOTE
 *      These functions are separated from mlib_v_ImageCopy.c for loop
 *      unrolling and structure clarity.
 */

#include <stdlib.h>
#include "vis_proto.h"
#include "mlib_image.h"

/***************************************************************/

void mlib_v_ImageCopy_a1(mlib_d64 *sp, mlib_d64 *dp, mlib_s32 size);
void mlib_v_ImageCopy_a2(mlib_d64 *sp, mlib_d64 *dp,
                         mlib_s32 width, mlib_s32 height,
                         mlib_s32 stride, mlib_s32 dstride);
void mlib_ImageCopy_na(mlib_u8 *sa, mlib_u8 *da, mlib_s32 size);
void mlib_ImageCopy_bit_al(mlib_u8 *sa, mlib_u8 *da, mlib_s32 size, mlib_s32 offset);

/***************************************************************/

/*
 * Both source and destination image data are 1-d vectors and
 * 8-byte aligned. And size is in 8-bytes.
 */

void mlib_v_ImageCopy_a1(mlib_d64 *sp, mlib_d64 *dp, mlib_s32 size)
{
  mlib_s32 i;

#pragma pipeloop(0)
  for (i = 0; i < size; i++) {
    *dp++ = *sp++;
  }
}

/***************************************************************/

/*
 * Either source or destination image data are not 1-d vectors, but
 * they are 8-byte aligned. And stride and width are in 8-bytes.
 */

void mlib_v_ImageCopy_a2(mlib_d64 *sp, mlib_d64 *dp,
                         mlib_s32 width, mlib_s32 height,
                         mlib_s32 stride, mlib_s32 dstride)
{
  mlib_d64   *spl;         /* 8-byte aligned pointer for line */
  mlib_d64   *dpl;         /* 8-byte aligned pointer for line */
  mlib_s32   i, j;         /* indices for x, y */

  spl = sp;
  dpl = dp;

  /* row loop */
  for (j = 0; j < height; j++) {
    /* 8-byte column loop */
#pragma pipeloop(0)
    for (i = 0; i < width; i++) {
      *dp++ = *sp++;
    }
    sp = spl += stride;
    dp = dpl += dstride;
  }
}

/***************************************************************/

/*
 * Both bit offsets of source and distination are the same
 */

void mlib_ImageCopy_bit_al(mlib_u8 *sa, mlib_u8 *da, mlib_s32 size, mlib_s32 offset)
{
  mlib_u8  *dend;        /* end points in dst */
  mlib_d64 *dp;          /* 8-byte aligned start points in dst */
  mlib_d64 *sp;          /* 8-byte aligned start point in src */
  mlib_d64 s0, s1;       /* 8-byte source data */
  mlib_s32 j;            /* offset of address in dst */
  mlib_s32 emask;        /* edge mask */
  mlib_s32 b_size;        /* edge mask */
  mlib_u8  mask0 = 0xFF;
  mlib_u8  src, mask;

  if (size < (8 - offset)) {
    mask = mask0 << (8 - size);
    mask >>= offset;
    src = da[0];
    da[0] = (src & (~mask)) | (sa[0] & mask);
    return;
  }

  mask = mask0 >> offset;
  src = da[0];
  da[0] = (src & (~mask)) | (sa[0] & mask);
  da++;
  sa++;
  size = size - 8 + offset;
  b_size = size >> 3;   /* size in bytes */
  
  /* prepare the destination addresses */
  dp = (mlib_d64 *)((mlib_addr) da & (~7));
  j = (mlib_addr) dp - (mlib_addr) da;
  dend = da + b_size - 1;

  /* prepare the source address */
  sp = (mlib_d64 *)vis_alignaddr(sa, j);
  /* generate edge mask for the start point */
  emask = vis_edge8(da, dend);

  s1 = sp[0];
  if (emask != 0xff) {
    s0 = s1;
    s1 = sp[1];
    s0 = vis_faligndata(s0, s1);
    vis_pst_8(s0, dp++, emask);
    sp++;
    j += 8;
  }
#pragma pipeloop(0)
  for (; j <= (b_size - 8); j += 8) {
    s0 = s1;
    s1 = sp[1];
    *dp++ = vis_faligndata(s0, s1);
    sp++;
  }

  if (j < b_size)  {
    s0 = vis_faligndata(s1, sp[1]);
    emask = vis_edge8(dp, dend);
    vis_pst_8(s0, dp, emask);
  }
  
  j = size & 7;
  
  if (j > 0) {
    mask = mask0 << (8 - j);
    src = dend[1];
    dend[1] = (src & (~mask)) | (sa[b_size] & mask);
    
  }
}

/***************************************************************/

/*
 * Either source or destination data are not 8-byte aligned.
 * And size is is in bytes.
 */

void mlib_ImageCopy_na(mlib_u8 *sa, mlib_u8 *da, mlib_s32 size)
{
  mlib_u8  *dend;        /* end points in dst */
  mlib_d64 *dp;          /* 8-byte aligned start points in dst */
  mlib_d64 *sp;          /* 8-byte aligned start point in src */
  mlib_d64 s0, s1;       /* 8-byte source data */
  int      j;            /* offset of address in dst */
  int      emask;        /* edge mask */

  /* prepare the destination addresses */
  dp = (mlib_d64 *)((mlib_addr) da & (~7));
  j = (mlib_addr) dp - (mlib_addr) da;
  dend = da + size - 1;

  /* prepare the source address */
  sp = (mlib_d64 *)vis_alignaddr(sa, j);
  /* generate edge mask for the start point */
  emask = vis_edge8(da, dend);

  s1 = sp[0];
  if (emask != 0xff) {
    s0 = s1;
    s1 = sp[1];
    s0 = vis_faligndata(s0, s1);
    vis_pst_8(s0, dp++, emask);
    sp++;
    j += 8;
  }
#pragma pipeloop(0)
  for (; j <= (size - 8); j += 8) {
    s0 = s1;
    s1 = sp[1];
    *dp++ = vis_faligndata(s0, s1);
    sp++;
  }

  if (j < size)  {
    s0 = vis_faligndata(s1, sp[1]);
    emask = vis_edge8(dp, dend);
    vis_pst_8(s0, dp, emask);
  }
}

/***************************************************************/
