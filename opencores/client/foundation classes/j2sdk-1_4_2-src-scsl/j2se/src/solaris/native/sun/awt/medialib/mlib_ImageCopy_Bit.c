/*
 * @(#)mlib_ImageCopy_Bit.c	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#pragma ident	"@(#)mlib_ImageCopy_Bit.c	1.2	00/03/14 SMI"

/*
 * FUNCTIONS
 *      mlib_ImageCopy_bit_na     - BIT, non-aligned
 *      mlib_ImageCopy_bit_na_r   - BIT, non-aligned, reverse
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
 *      Direct copy from one image to another -- C version low level
 *      functions.
 */

#include <stdlib.h>
#include "mlib_image.h"

/***************************************************************/

/*
 * Bit offsets of source and distination are not the same
 */

void mlib_ImageCopy_bit_na(mlib_u8 *sa, mlib_u8 *da, int size, int s_offset, int d_offset)
{
#ifdef _NO_LONGLONG

  mlib_u32 *dp;          /* 4-byte aligned start points in dst */
  mlib_u32 *sp;          /* 4-byte aligned start point in src */
  mlib_s32 j;            /* offset of address in dst */
  mlib_u32 mask0 = 0xFFFFFFFF;
  mlib_u32 dmask;
  mlib_u32 src, src0, src1, dst;
  mlib_s32 ls_offset, ld_offset, shift;

  /* prepare the destination addresses */
  dp = (mlib_u32 *)((mlib_addr)da & (~3));
  sp = (mlib_u32 *)((mlib_addr)sa & (~3));
  ld_offset = (((mlib_addr)da & 3) << 3) + d_offset;     /* bit d_offset to first int */
  ls_offset = (((mlib_addr)sa & 3) << 3) + s_offset;     /* bit d_offset to first int */

  if (ld_offset > ls_offset) {
    if (ld_offset + size < 32) {
      dmask = (mask0 << (32 - size)) >> ld_offset;
      src0 = sp[0];
#ifdef _LITTLE_ENDIAN
      src0 = (src0 << 24) | ((src0 & 0xFF00) << 8) | ((src0 >> 8)& 0xFF00) | (src0 >> 24);
      src = (src0 >> (ld_offset - ls_offset));
      dst = dp[0];
      dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
      dst = (dst & (~dmask)) | (src & dmask);
      dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
      dp[0] = dst;
#else
      src = (src0 >> (ld_offset - ls_offset));
      dst = dp[0];
      dp[0] = (dst & (~dmask)) | (src & dmask);
#endif /* _LITTLE_ENDIAN */
      return;
    }
    dmask = mask0 >> ld_offset;
    src0 = sp[0];
#ifdef _LITTLE_ENDIAN
    src0 = (src0 << 24) | ((src0 & 0xFF00) << 8) | ((src0 >> 8)& 0xFF00) | (src0 >> 24);
    src = (src0 >> (ld_offset - ls_offset));
    dst = dp[0];
    dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
    dst = (dst & ~dmask) | (src & dmask);
    dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
    dp[0] = dst;
#else
    src = (src0 >> (ld_offset - ls_offset));
    dst = dp[0];
    dp[0] = (dst & ~dmask) | (src & dmask);
#endif /* _LITTLE_ENDIAN */
    j = 32 - ld_offset;
    dp++;
    ls_offset += j;
  } else {
    if (ld_offset + size < 32) {
      dmask = (mask0 << (32 - size)) >> ld_offset;
      src0 = sp[0];
      src1 = sp[1];
#ifdef _LITTLE_ENDIAN
      src0 = (src0 << 24) | ((src0 & 0xFF00) << 8) | ((src0 >> 8)& 0xFF00) | (src0 >> 24);
      src1 = (src1 << 24) | ((src1 & 0xFF00) << 8) | ((src1 >> 8)& 0xFF00) | (src1 >> 24);
      shift = ls_offset - ld_offset;
      src = (src0 << shift) | (src1 >> (32 - shift));
      dst = dp[0];
      dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
      dst = (dst & ~dmask) | (src & dmask);
      dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
      dp[0] = dst;
#else
      shift = ls_offset - ld_offset;
      src = (src0 << shift) | (src1 >> (32 - shift));
      dst = dp[0];
      dp[0] = (dst & ~dmask) | (src & dmask);
#endif /* _LITTLE_ENDIAN */
      return;
    }
    dmask = mask0 >> ld_offset;
    src0 = sp[0];
    src1 = sp[1];
#ifdef _LITTLE_ENDIAN
    src0 = (src0 << 24) | ((src0 & 0xFF00) << 8) | ((src0 >> 8)& 0xFF00) | (src0 >> 24);
    src1 = (src1 << 24) | ((src1 & 0xFF00) << 8) | ((src1 >> 8)& 0xFF00) | (src1 >> 24);
    shift = ls_offset - ld_offset;
    src = (src0 << shift) | (src1 >> (32 - shift));
    dst = dp[0];
    dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
    dst = (dst & ~dmask) | (src & dmask);
    dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
    dp[0] = dst;
#else
    shift = ls_offset - ld_offset;
    src = (src0 << shift) | (src1 >> (32 - shift));
    dst = dp[0];
    dp[0] = (dst & ~dmask) | (src & dmask);
#endif /* _LITTLE_ENDIAN */
    j = 32 - ld_offset;
    dp++;
    sp++;
    ls_offset = ls_offset + j - 32;
  }

  src1 = sp[0];
#ifdef _LITTLE_ENDIAN
  src1 = (src1 << 24) | ((src1 & 0xFF00) << 8) | ((src1 >> 8)& 0xFF00) | (src1 >> 24);
#endif /* _LITTLE_ENDIAN */
  for (; j <= size - 32; j += 32) {
    src0 = src1;
    src1 = sp[1];
#ifdef _LITTLE_ENDIAN
    src1 = (src1 << 24) | ((src1 & 0xFF00) << 8) | ((src1 >> 8)& 0xFF00) | (src1 >> 24);
    src = (src0 << ls_offset) | (src1 >> (32 - ls_offset));
    src = (src << 24) | ((src & 0xFF00) << 8) | ((src >> 8)& 0xFF00) | (src >> 24);
    dp[0] = src;
#else
    dp[0] = (src0 << ls_offset) | (src1 >> (32 - ls_offset));
#endif /* _LITTLE_ENDIAN */
    sp++;
    dp++;
  }
  
  if (j < size) {
   j = size - j;
   src0 = src1;
   src1 = sp[1];
#ifdef _LITTLE_ENDIAN
   src1 = (src1 << 24) | ((src1 & 0xFF00) << 8) | ((src1 >> 8)& 0xFF00) | (src1 >> 24);
   dmask = mask0 << (32 - j);
   src = (src0 << ls_offset) | (src1 >> (32 - ls_offset));
   dst = dp[0];
   dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
   dst = (dst & ~dmask) | (src & dmask);
   dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
   dp[0] = dst;
#else
   dmask = mask0 << (32 - j);
   src = (src0 << ls_offset) | (src1 >> (32 - ls_offset));
   dst = dp[0];
   dp[0] = (dst & ~dmask) | (src & dmask);
#endif /* _LITTLE_ENDIAN */
  }

#else /* _LONGLONG */

  mlib_u64 *dp;          /* 8-byte aligned start points in dst */
  mlib_u64 *sp;          /* 8-byte aligned start point in src */
  mlib_s32 j;            /* offset of address in dst */
  mlib_u64 lmask0 = 0xFFFFFFFFFFFFFFFF;
  mlib_u64 dmask;
  mlib_u64 lsrc, lsrc0, lsrc1, ldst;
  mlib_s32 ls_offset, ld_offset, shift;

  /* prepare the destination addresses */
  dp = (mlib_u64 *)((mlib_addr)da & (~7));
  sp = (mlib_u64 *)((mlib_addr)sa & (~7));
  ld_offset = (((mlib_addr)da & 7) << 3) + d_offset;     /* bit d_offset to first double */
  ls_offset = (((mlib_addr)sa & 7) << 3) + s_offset;     /* bit d_offset to first double */
  
  if (ld_offset > ls_offset) {
    if (ld_offset + size < 64) {
      dmask = (lmask0 << (64 - size)) >> ld_offset;
      lsrc0 = sp[0];
      lsrc = (lsrc0 >> (ld_offset - ls_offset));
      ldst = dp[0];
      dp[0] = (ldst & (~dmask)) | (lsrc & dmask);
      return;
    }
    dmask = lmask0 >> ld_offset;
    lsrc0 = sp[0];
    lsrc = (lsrc0 >> (ld_offset - ls_offset));
    ldst = dp[0];
    dp[0] = (ldst & ~dmask) | (lsrc & dmask);
    j = 64 - ld_offset;
    dp++;
    ls_offset += j;
  } else {
    if (ld_offset + size < 64) {
      dmask = (lmask0 << (64 - size)) >> ld_offset;
      lsrc0 = sp[0];
      lsrc1 = sp[1];
      shift = ls_offset - ld_offset;
      lsrc = (lsrc0 << shift) | (lsrc1 >> (64 - shift));
      ldst = dp[0];
      dp[0] = (ldst & ~dmask) | (lsrc & dmask);
      return;
    }
    dmask = lmask0 >> ld_offset;
    lsrc0 = sp[0];
    lsrc1 = sp[1];
    shift = ls_offset - ld_offset;
    lsrc = (lsrc0 << shift) | (lsrc1 >> (64 - shift));
    ldst = dp[0];
    dp[0] = (ldst & ~dmask) | (lsrc & dmask);
    j = 64 - ld_offset;
    dp++;
    sp++;
    ls_offset = ls_offset + j - 64;
  }

  lsrc1 = sp[0];
#pragma pipeloop(0)
  for (; j <= size - 64; j += 64) {
    lsrc0 = lsrc1;
    lsrc1 = sp[1];
    lsrc = (lsrc0 << ls_offset) | (lsrc1 >> (64 - ls_offset));
    dp[0] = lsrc;
    sp++;
    dp++;
  }
  
  if (j < size) {
   j = size - j;
   lsrc0 = lsrc1;
   lsrc1 = sp[1];
   dmask = lmask0 << (64 - j);
   lsrc = (lsrc0 << ls_offset) | (lsrc1 >> (64 - ls_offset));
   ldst = dp[0];
   dp[0] = (ldst & ~dmask) | (lsrc & dmask);
  }
#endif /* _NO_LONGLONG */

}

/***************************************************************/

/*
 * Bit offsets of source and distination are not the same
 * This function is both for C and VIS version (LONGLONG case)
 */

void mlib_ImageCopy_bit_na_r(mlib_u8 *sa, mlib_u8 *da, int size, int s_offset, int d_offset)
{
#ifdef _NO_LONGLONG

  mlib_u32 *dp;          /* 4-byte aligned start points in dst */
  mlib_u32 *sp;          /* 4-byte aligned start point in src */
  mlib_s32 j;            /* offset of address in dst */
  mlib_u32 lmask0 = 0xFFFFFFFF;
  mlib_u32 dmask;
  mlib_u32 src, src0, src1, dst;
  mlib_s32 ls_offset, ld_offset, shift;

  /* prepare the destination addresses */
  dp = (mlib_u32 *)((mlib_addr)da & (~3));
  sp = (mlib_u32 *)((mlib_addr)sa & (~3));
  ld_offset = (((mlib_addr)da & 3) << 3) + d_offset;     /* bit d_offset to first int */
  ls_offset = (((mlib_addr)sa & 3) << 3) + s_offset;     /* bit d_offset to first int */
  
  if (ld_offset < ls_offset) {
    if (ld_offset - size >= 0) { 
      dmask = (lmask0 << (32 - size)) >> (ld_offset - size);
      src0 = sp[0];
#ifdef _LITTLE_ENDIAN
      src0 = (src0 << 24) | ((src0 & 0xFF00) << 8) | ((src0 >> 8)& 0xFF00) | (src0 >> 24);
      src = (src0 << (ls_offset - ld_offset));
      dst = dp[0];
      dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
      dst = (dst & (~dmask)) | (src & dmask);
      dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
      dp[0] = dst;
#else
      src = (src0 << (ls_offset - ld_offset));
      dst = dp[0];
      dp[0] = (dst & (~dmask)) | (src & dmask);
#endif /* _LITTLE_ENDIAN */
      return;
    }
    dmask = lmask0 << (32 - ld_offset);
    src0 = sp[0];
#ifdef _LITTLE_ENDIAN
    src0 = (src0 << 24) | ((src0 & 0xFF00) << 8) | ((src0 >> 8)& 0xFF00) | (src0 >> 24);
    src = (src0 << (ls_offset - ld_offset));
    dst = dp[0];
    dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
    dst = (dst & ~dmask) | (src & dmask);
    dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
    dp[0] = dst;
#else
    src = (src0 << (ls_offset - ld_offset));
    dst = dp[0];
    dp[0] = (dst & ~dmask) | (src & dmask);
#endif /* _LITTLE_ENDIAN */
    j = ld_offset;
    dp--;
    ls_offset -= j;
  } else {
    if (ld_offset - size >= 0) {
      dmask = (lmask0 << (32 - size)) >> (ld_offset - size);
      src0 = sp[0];
      src1 = sp[-1];
#ifdef _LITTLE_ENDIAN
      src0 = (src0 << 24) | ((src0 & 0xFF00) << 8) | ((src0 >> 8)& 0xFF00) | (src0 >> 24);
      src1 = (src1 << 24) | ((src1 & 0xFF00) << 8) | ((src1 >> 8)& 0xFF00) | (src1 >> 24);
      shift = ld_offset - ls_offset;
      src = (src0 >> shift) | (src1 << (32 - shift));
      dst = dp[0];
      dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
      dst = (dst & ~dmask) | (src & dmask);
      dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
      dp[0] = dst;
#else
      shift = ld_offset - ls_offset;
      src = (src0 >> shift) | (src1 << (32 - shift));
      dst = dp[0];
      dp[0] = (dst & ~dmask) | (src & dmask);
#endif /* _LITTLE_ENDIAN */
      return;
    }
    dmask = lmask0 << (32 - ld_offset);
    src0 = sp[0];
    src1 = sp[-1];
#ifdef _LITTLE_ENDIAN
    src0 = (src0 << 24) | ((src0 & 0xFF00) << 8) | ((src0 >> 8)& 0xFF00) | (src0 >> 24);
    src1 = (src1 << 24) | ((src1 & 0xFF00) << 8) | ((src1 >> 8)& 0xFF00) | (src1 >> 24);
    shift = ld_offset - ls_offset;
    src = (src0 >> shift) | (src1 << (32 - shift));
    dst = dp[0];
    dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
    dst = (dst & ~dmask) | (src & dmask);
    dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
    dp[0] = dst;
#else
    shift = ld_offset - ls_offset;
    src = (src0 >> shift) | (src1 << (32 - shift));
    dst = dp[0];
    dp[0] = (dst & ~dmask) | (src & dmask);
#endif /* _LITTLE_ENDIAN */
    j = ld_offset;
    dp--;
    sp--;
    ls_offset = ls_offset - j + 32;
  }

  src1 = sp[0];
#ifdef _LITTLE_ENDIAN
  src1 = (src1 << 24) | ((src1 & 0xFF00) << 8) | ((src1 >> 8)& 0xFF00) | (src1 >> 24);
#endif /* _LITTLE_ENDIAN */
#pragma pipeloop(0)
  for (; j <= size - 32; j += 32) {
    src0 = src1;
    src1 = sp[-1];
#ifdef _LITTLE_ENDIAN
    src1 = (src1 << 24) | ((src1 & 0xFF00) << 8) | ((src1 >> 8)& 0xFF00) | (src1 >> 24);
    src = (src0 >> (32 - ls_offset)) | (src1 << ls_offset);
    src = (src << 24) | ((src & 0xFF00) << 8) | ((src >> 8)& 0xFF00) | (src >> 24);
    dp[0] = src;
#else
    dp[0] = (src0 >> (32 - ls_offset)) | (src1 << ls_offset);
#endif /* _LITTLE_ENDIAN */
    sp--;
    dp--;
  }
  
  if (j < size) {
   j = size - j;
   src0 = src1;
   src1 = sp[-1];
#ifdef _LITTLE_ENDIAN
   src1 = (src1 << 24) | ((src1 & 0xFF00) << 8) | ((src1 >> 8)& 0xFF00) | (src1 >> 24);
   dmask = lmask0 >> (32 - j);
   src = (src0 >> (32 - ls_offset)) | (src1 << ls_offset);
   dst = dp[0];
   dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
   dst = (dst & ~dmask) | (src & dmask);
   dst = (dst << 24) | ((dst & 0xFF00) << 8) | ((dst >> 8)& 0xFF00) | (dst >> 24);
   dp[0] = dst;
#else
   dmask = lmask0 >> (32 - j);
   src = (src0 >> (32 - ls_offset)) | (src1 << ls_offset);
   dst = dp[0];
   dp[0] = (dst & ~dmask) | (src & dmask);
#endif /* _LITTLE_ENDIAN */
  }

#else  /* _LONGLONG */

  mlib_u64 *dp;          /* 8-byte aligned start points in dst */
  mlib_u64 *sp;          /* 8-byte aligned start point in src */
  mlib_s32 j;            /* offset of address in dst */
  mlib_u64 lmask0 = 0xFFFFFFFFFFFFFFFF;
  mlib_u64 dmask;
  mlib_u64 lsrc, lsrc0, lsrc1, ldst;
  mlib_s32 ls_offset, ld_offset, shift;

  /* prepare the destination addresses */
  dp = (mlib_u64 *)((mlib_addr)da & (~7));
  sp = (mlib_u64 *)((mlib_addr)sa & (~7));
  ld_offset = (((mlib_addr)da & 7) << 3) + d_offset;     /* bit d_offset to first double */
  ls_offset = (((mlib_addr)sa & 7) << 3) + s_offset;     /* bit d_offset to first double */
  
  if (ld_offset < ls_offset) {
    if (ld_offset - size >= 0) { 
      dmask = (lmask0 << (64 - size)) >> (ld_offset - size);
      lsrc0 = sp[0];
      lsrc = (lsrc0 << (ls_offset - ld_offset));
      ldst = dp[0];
      dp[0] = (ldst & (~dmask)) | (lsrc & dmask);
      return;
    }
    dmask = lmask0 << (64 - ld_offset);
    lsrc0 = sp[0];
    lsrc = (lsrc0 << (ls_offset - ld_offset));
    ldst = dp[0];
    dp[0] = (ldst & ~dmask) | (lsrc & dmask);
    j = ld_offset;
    dp--;
    ls_offset -= j;
  } else {
    if (ld_offset - size >= 0) {
      dmask = (lmask0 << (64 - size)) >> (ld_offset - size);
      lsrc0 = sp[0];
      lsrc1 = sp[-1];
      shift = ld_offset - ls_offset;
      lsrc = (lsrc0 >> shift) | (lsrc1 << (64 - shift));
      ldst = dp[0];
      dp[0] = (ldst & ~dmask) | (lsrc & dmask);
      return;
    }
    dmask = lmask0 << (64 - ld_offset);
    lsrc0 = sp[0];
    lsrc1 = sp[-1];
    shift = ld_offset - ls_offset;
    lsrc = (lsrc0 >> shift) | (lsrc1 << (64 - shift));
    ldst = dp[0];
    dp[0] = (ldst & ~dmask) | (lsrc & dmask);
    j = ld_offset;
    dp--;
    sp--;
    ls_offset = ls_offset - j + 64;
  }

  lsrc1 = sp[0];
#pragma pipeloop(0)
  for (; j <= size - 64; j += 64) {
    lsrc0 = lsrc1;
    lsrc1 = sp[-1];
    dp[0] = (lsrc0 >> (64 - ls_offset)) | (lsrc1 << ls_offset);
    sp--;
    dp--;
  }
  
  if (j < size) {
   j = size - j;
   lsrc0 = lsrc1;
   lsrc1 = sp[-1];
   dmask = lmask0 >> (64 - j);
   lsrc = (lsrc0 >> (64 - ls_offset)) | (lsrc1 << ls_offset);
   ldst = dp[0];
   dp[0] = (ldst & ~dmask) | (lsrc & dmask);
  }

#endif /* _NO_LONGLONG */

}

/***************************************************************/
