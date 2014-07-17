#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bytes_solaris_i486.inline.hpp	1.5 03/01/23 11:09:50 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Efficient swapping of data bytes from Java byte
// ordering to native byte ordering and vice versa.
inline u2   Bytes::swap_u2(u2 x) {
  u2 ret;
  ret = x & 0xff;
  ret <<= 8; x >>= 8;
  ret |= x & 0xff;
  return ret;
// TODO - Need to check this code out! kbt
//    u2 ret;
//    asm("  mov ax, x");
//    asm("  xchg al, ah");
//    asm("  mov ret, ax");
//    return ret;
}

inline u4   Bytes::swap_u4(u4 x) {
  u4 ret;
  ret = x & 0xff;
  ret <<= 8; x >>= 8;
  ret |= x & 0xff;
  ret <<= 8; x >>= 8;
  ret |= x & 0xff;
  ret <<= 8; x >>= 8;
  ret |= x & 0xff;
  return ret;
// TODO - Need to check this code out! kbt
//    u4 ret;
//    asm("  mov eax, x");
//    asm("  bswap eax");
//    asm("  mov ret, eax");
//    return ret;
}

// Helper function for swap_u8
inline u8   Bytes::swap_u8_base(u4 x, u4 y) {
  u8 ret;
  ret = x & 0xff;
  ret <<= 8; x >>= 8;
  ret |= x & 0xff;
  ret <<= 8; x >>= 8;
  ret |= x & 0xff;
  ret <<= 8; x >>= 8;
  ret |= x & 0xff;
  ret <<= 8;
  ret |= y & 0xff;
  ret <<= 8; y >>= 8;
  ret |= y & 0xff;
  ret <<= 8; y >>= 8;
  ret |= y & 0xff;
  ret <<= 8; y >>= 8;
  ret |= y & 0xff;
  return ret;
// TODO - Need to check this code out! kbt
//    u8 ret;
//    asm("  mov eax, y");
//    asm("  mov edx, x");
//    asm("  bswap eax");
//    asm("  bswap edx");
//    asm("  mov ret, edx:eax");
//    return ret;
}

