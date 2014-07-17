/*
 * @(#)mlib_v_ImageConvMxN_16.h	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#pragma ident	"@(#)mlib_v_ImageConvMxN_16.h	1.7	98/11/25 SMI"

#ifndef MLIB_IMAGECONVMXN_16_H
#define MLIB_IMAGECONVMXN_16_H

/*
 * FUNCTION
 *      Macros for mlib_convMxN_16(nw, ext)   (16-bit convolution routines)
 */

#include <mlib_types.h>
#include <vis_proto.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************/
#define CONV_16(dsrc, ka)                               \
  tmp0 = vis_fmuld8sux16(vis_read_hi(dsrc), ka);        \
  tmp1 = vis_fmuld8ulx16(vis_read_hi(dsrc), ka);        \
  tmp2 = vis_fmuld8sux16(vis_read_lo(dsrc), ka);        \
  tmp3 = vis_fmuld8ulx16(vis_read_lo(dsrc), ka);        \
  tmp0 = vis_fpadd32(tmp0, tmp1);                       \
  out0 = vis_fpadd32(out0, tmp0);                       \
  tmp2 = vis_fpadd32(tmp2, tmp3);                       \
  out1 = vis_fpadd32(out1, tmp2);
/***************************************************************/
#define CONV_16_MIX(dsrc1, dsrc2, ka)                   \
  tmp0 = vis_fmuld8sux16(vis_read_lo(dsrc1), ka);       \
  tmp1 = vis_fmuld8ulx16(vis_read_lo(dsrc1), ka);       \
  tmp2 = vis_fmuld8sux16(vis_read_hi(dsrc2), ka);       \
  tmp3 = vis_fmuld8ulx16(vis_read_hi(dsrc2), ka);       \
  tmp0 = vis_fpadd32(tmp0, tmp1);                       \
  out0 = vis_fpadd32(out0, tmp0);                       \
  tmp2 = vis_fpadd32(tmp2, tmp3);                       \
  out1 = vis_fpadd32(out1, tmp2);
/***************************************************************/
#define COUNT_8(adr_row, ka1, ka2, ka3, ka4, ka5, ka6, ka7, ka8)        \
  d1 = *adr_row;                                                        \
  d_1 = *(adr_row+1);                                                   \
  CONV_16(d1, ka1)                                                      \
  CONV_16_MIX(d1, d_1, ka3)                                             \
  CONV_16(d_1, ka5)                                                     \
  d21 = vis_faligndata(d1, d_1);                                        \
  d1 = *(adr_row+2);                                                    \
  CONV_16(d21, ka2)                                                     \
  dt1 = vis_faligndata(d_1, d1);                                        \
  CONV_16_MIX(d21, dt1, ka4)                                            \
  d21 = vis_faligndata(d1, d1);                                         \
  CONV_16(dt1, ka6)                                                     \
  CONV_16_MIX(d_1, d1, ka7)                                             \
  CONV_16_MIX(dt1, d21, ka8)
/***************************************************************/
#define COUNT_7(adr_row, ka1, ka2, ka3, ka4, ka5, ka6, ka7)     \
  d1 = *adr_row;                                                \
  d_1 = *(adr_row+1);                                           \
  CONV_16(d1, ka1)                                              \
  CONV_16_MIX(d1, d_1, ka3)                                     \
  CONV_16(d_1, ka5)                                             \
  d21 = vis_faligndata(d1, d_1);                                \
  d1 = *(adr_row+2);                                            \
  CONV_16(d21, ka2)                                             \
  dt1 = vis_faligndata(d_1, d1);                                \
  CONV_16_MIX(d21, dt1, ka4)                                    \
  CONV_16(dt1, ka6)                                             \
  CONV_16_MIX(d_1, d1, ka7)
/***************************************************************/
#define COUNT_6(adr_row, ka1, ka2, ka3, ka4, ka5, ka6)  \
  d1 = *adr_row;                                        \
  d_1 = *(adr_row+1);                                   \
  CONV_16(d1, ka1)                                      \
  CONV_16_MIX(d1, d_1, ka3)                             \
  CONV_16(d_1, ka5)                                     \
  d21 = vis_faligndata(d1, d_1);                        \
  d1 = *(adr_row+2);                                    \
  CONV_16(d21, ka2)                                     \
  dt1 = vis_faligndata(d_1, d1);                        \
  CONV_16_MIX(d21, dt1, ka4)                            \
  CONV_16(dt1, ka6)
/***************************************************************/
#define COUNT_5(adr_row, ka1, ka2, ka3, ka4, ka5)       \
  d1 = *adr_row;                                        \
  d_1 = *(adr_row+1);                                   \
  CONV_16(d1, ka1)                                      \
  CONV_16_MIX(d1, d_1, ka3)                             \
  CONV_16(d_1, ka5)                                     \
  d21 = vis_faligndata(d1, d_1);                        \
  dt1 = vis_faligndata(d_1, d_1);                       \
  CONV_16(d21, ka2)                                     \
  CONV_16_MIX(d21, dt1, ka4)
/***************************************************************/
#define COUNT_4(adr_row, ka1, ka2, ka3, ka4)    \
  d1 = *adr_row;                                \
  d_1 = *(adr_row+1);                           \
  CONV_16(d1, ka1)                              \
  CONV_16_MIX(d1, d_1, ka3)                     \
  d21 = vis_faligndata(d1, d_1);                \
  dt1 = vis_faligndata(d_1, d_1);               \
  CONV_16(d21, ka2)                             \
  CONV_16_MIX(d21, dt1, ka4)
/***************************************************************/
#define COUNT_3(adr_row, ka1, ka2, ka3)         \
  d1 = *adr_row;                                \
  d_1 = *(adr_row+1);                           \
  CONV_16(d1, ka1)                              \
  CONV_16_MIX(d1, d_1, ka3)                     \
  d21 = vis_faligndata(d1, d_1);                \
  CONV_16(d21, ka2)
/***************************************************************/
#define COUNT_2(adr_row, ka1, ka2)      \
  d1 = *adr_row;                        \
  d_1 = *(adr_row+1);                   \
  CONV_16(d1, ka1)                      \
  d21 = vis_faligndata(d1, d_1);        \
  CONV_16(d21, ka2)
/***************************************************************/
#define COUNT_1(adr_row, ka1)   \
  d1 = *adr_row;                \
  CONV_16(d1, ka1)
/***************************************************************/
#define ONE_ROW_CONV_MxN(_khgt, _kwid, _fk, _ibuffp, _intrm_buf, _dwid) {       \
  mlib_d64  *s1, *s2, *s3, *ibufh;                                              \
  mlib_f32  k1, k2, k3, k4, k5, k6, k7, k8;                                     \
  mlib_f32  k9, k10, k11, k12, k13, k14, k15, k16;                              \
  mlib_f32  k17, k18, k19, k20, k21, k22, k23, k24;                             \
  int       row, ikh, ikw, n1, n2, n3, offset, i;                               \
  mlib_d64  out0, out1, tmp0, tmp1, tmp2, tmp3;                                 \
  mlib_d64  d1, d_1, d21, dt1;                                                  \
                                                                                \
  row = 0;                                                                      \
  vis_alignaddr(0, 2);                                                          \
  for (ikh = _khgt; ikh >= 3; ikh -= 3) {                                       \
    n1 = row * _kwid; n2 = n1 + _kwid; n3 = n2 + _kwid;                         \
    offset = 0;                                                                 \
    for (ikw = _kwid; ikw >= 8; ikw -= 8) {                                     \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++]; k6 = _fk[n1++]; k7 = _fk[n1++]; k8 = _fk[n1++];           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++]; k12 = _fk[n2++];        \
      k13 = _fk[n2++]; k14 = _fk[n2++]; k15 = _fk[n2++]; k16 = _fk[n2++];       \
      k17 = _fk[n3++]; k18 = _fk[n3++]; k19 = _fk[n3++]; k20 = _fk[n3++];       \
      k21 = _fk[n3++]; k22 = _fk[n3++]; k23 = _fk[n3++]; k24 = _fk[n3++];       \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      s3 = _ibuffp[row+2] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_8(s1, k1, k2, k3, k4, k5, k6, k7, k8)                             \
        COUNT_8(s2, k9, k10, k11, k12, k13, k14, k15, k16)                      \
        COUNT_8(s3, k17, k18, k19, k20, k21, k22, k23, k24)                     \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++; s3++;                                                       \
      }                                                                         \
      offset += 2;                                                              \
    }                                                                           \
    if (ikw == 7) {                                                             \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++]; k6 = _fk[n1++]; k7 = _fk[n1++];                           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++]; k12 = _fk[n2++];        \
      k13 = _fk[n2++]; k14 = _fk[n2++]; k15 = _fk[n2++];                        \
      k17 = _fk[n3++]; k18 = _fk[n3++]; k19 = _fk[n3++]; k20 = _fk[n3++];       \
      k21 = _fk[n3++]; k22 = _fk[n3++]; k23 = _fk[n3++];                        \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      s3 = _ibuffp[row+2] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_7(s1, k1, k2, k3, k4, k5, k6, k7)                                 \
        COUNT_7(s2, k9, k10, k11, k12, k13, k14, k15)                           \
        COUNT_7(s3, k17, k18, k19, k20, k21, k22, k23)                          \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++; s3++;                                                       \
      }                                                                         \
    }                                                                           \
    else if (ikw == 6) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++]; k6 = _fk[n1++];                                           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++]; k12 = _fk[n2++];        \
      k13 = _fk[n2++]; k14 = _fk[n2++];                                         \
      k17 = _fk[n3++]; k18 = _fk[n3++]; k19 = _fk[n3++]; k20 = _fk[n3++];       \
      k21 = _fk[n3++]; k22 = _fk[n3++];                                         \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      s3 = _ibuffp[row+2] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_6(s1, k1, k2, k3, k4, k5, k6)                                     \
        COUNT_6(s2, k9, k10, k11, k12, k13, k14)                                \
        COUNT_6(s3, k17, k18, k19, k20, k21, k22)                               \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++; s3++;                                                       \
      }                                                                         \
    }                                                                           \
    else if (ikw == 5) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++];                                                           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++]; k12 = _fk[n2++];        \
      k13 = _fk[n2++];                                                          \
      k17 = _fk[n3++]; k18 = _fk[n3++]; k19 = _fk[n3++]; k20 = _fk[n3++];       \
      k21 = _fk[n3++];                                                          \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      s3 = _ibuffp[row+2] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_5(s1, k1, k2, k3, k4, k5)                                         \
        COUNT_5(s2, k9, k10, k11, k12, k13)                                     \
        COUNT_5(s3, k17, k18, k19, k20, k21)                                    \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++; s3++;                                                       \
      }                                                                         \
    }                                                                           \
    else if (ikw == 4) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++]; k12 = _fk[n2++];        \
      k17 = _fk[n3++]; k18 = _fk[n3++]; k19 = _fk[n3++]; k20 = _fk[n3++];       \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      s3 = _ibuffp[row+2] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_4(s1, k1, k2, k3, k4)                                             \
        COUNT_4(s2, k9, k10, k11, k12)                                          \
        COUNT_4(s3, k17, k18, k19, k20)                                         \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++; s3++;                                                       \
      }                                                                         \
    }                                                                           \
    else if (ikw == 3) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++];                           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++];                         \
      k17 = _fk[n3++]; k18 = _fk[n3++]; k19 = _fk[n3++];                        \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      s3 = _ibuffp[row+2] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_3(s1, k1, k2, k3)                                                 \
        COUNT_3(s2, k9, k10, k11)                                               \
        COUNT_3(s3, k17, k18, k19)                                              \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++; s3++;                                                       \
      }                                                                         \
    }                                                                           \
    else if (ikw == 2) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++];                                           \
      k9 = _fk[n2++]; k10 = _fk[n2++];                                          \
      k17 = _fk[n3++]; k18 = _fk[n3++];                                         \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      s3 = _ibuffp[row+2] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_2(s1, k1, k2)                                                     \
        COUNT_2(s2, k9, k10)                                                    \
        COUNT_2(s3, k17, k18)                                                   \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++; s3++;                                                       \
      }                                                                         \
    }                                                                           \
    else if (ikw == 1) {                                                        \
      k1 = _fk[n1++];                                                           \
      k9 = _fk[n2++];                                                           \
      k17 = _fk[n3++];                                                          \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      s3 = _ibuffp[row+2] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_1(s1, k1)                                                         \
        COUNT_1(s2, k9)                                                         \
        COUNT_1(s3, k17)                                                        \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++; s3++;                                                       \
      }                                                                         \
    }                                                                           \
    row += 3;                                                                   \
  }                                                                             \
  if (ikh == 2) {                                                               \
    n1 = row * _kwid; n2 = n1 + _kwid;                                          \
    offset = 0;                                                                 \
    for (ikw = _kwid; ikw >= 8; ikw -= 8) {                                     \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++]; k6 = _fk[n1++]; k7 = _fk[n1++]; k8 = _fk[n1++];           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++]; k12 = _fk[n2++];        \
      k13 = _fk[n2++]; k14 = _fk[n2++]; k15 = _fk[n2++]; k16 = _fk[n2++];       \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_8(s1, k1, k2, k3, k4, k5, k6, k7, k8)                             \
        COUNT_8(s2, k9, k10, k11, k12, k13, k14, k15, k16)                      \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++;                                                             \
      }                                                                         \
      offset += 2;                                                              \
    }                                                                           \
    if (ikw == 7) {                                                             \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++]; k6 = _fk[n1++]; k7 = _fk[n1++];                           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++]; k12 = _fk[n2++];        \
      k13 = _fk[n2++]; k14 = _fk[n2++]; k15 = _fk[n2++];                        \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_7(s1, k1, k2, k3, k4, k5, k6, k7)                                 \
        COUNT_7(s2, k9, k10, k11, k12, k13, k14, k15)                           \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++;                                                             \
      }                                                                         \
    }                                                                           \
    else if (ikw == 6) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++]; k6 = _fk[n1++];                                           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++]; k12 = _fk[n2++];        \
      k13 = _fk[n2++]; k14 = _fk[n2++];                                         \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_6(s1, k1, k2, k3, k4, k5, k6)                                     \
        COUNT_6(s2, k9, k10, k11, k12, k13, k14)                                \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++;                                                             \
      }                                                                         \
    }                                                                           \
    else if (ikw == 5) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++];                                                           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++]; k12 = _fk[n2++];        \
      k13 = _fk[n2++];                                                          \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_5(s1, k1, k2, k3, k4, k5)                                         \
        COUNT_5(s2, k9, k10, k11, k12, k13)                                     \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++;                                                             \
      }                                                                         \
    }                                                                           \
    else if (ikw == 4) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++]; k12 = _fk[n2++];        \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_4(s1, k1, k2, k3, k4)                                             \
        COUNT_4(s2, k9, k10, k11, k12)                                          \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++;                                                             \
      }                                                                         \
    }                                                                           \
    else if (ikw == 3) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++];                           \
      k9 = _fk[n2++]; k10 = _fk[n2++]; k11 = _fk[n2++];                         \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_3(s1, k1, k2, k3)                                                 \
        COUNT_3(s2, k9, k10, k11)                                               \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++;                                                             \
      }                                                                         \
    }                                                                           \
    else if (ikw == 2) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++];                                           \
      k9 = _fk[n2++]; k10 = _fk[n2++];                                          \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_2(s1, k1, k2)                                                     \
        COUNT_2(s2, k9, k10)                                                    \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++;                                                             \
      }                                                                         \
    }                                                                           \
    else if (ikw == 1) {                                                        \
      k1 = _fk[n1++];                                                           \
      k9 = _fk[n2++];                                                           \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_1(s1, k1)                                                         \
        COUNT_1(s2, k9)                                                         \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++; s2++;                                                             \
      }                                                                         \
    }                                                                           \
  }                                                                             \
  else if (ikh == 1) {                                                          \
    n1 = row * _kwid;                                                           \
    offset = 0;                                                                 \
    for (ikw = _kwid; ikw >= 8; ikw -= 8) {                                     \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++]; k6 = _fk[n1++]; k7 = _fk[n1++]; k8 = _fk[n1++];           \
      s1 = _ibuffp[row] + offset;                                               \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_8(s1, k1, k2, k3, k4, k5, k6, k7, k8)                             \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++;                                                                   \
      }                                                                         \
      offset += 2;                                                              \
    }                                                                           \
    if (ikw == 7) {                                                             \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++]; k6 = _fk[n1++]; k7 = _fk[n1++];                           \
      s1 = _ibuffp[row] + offset;                                               \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_7(s1, k1, k2, k3, k4, k5, k6, k7)                                 \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++;                                                                   \
      }                                                                         \
    }                                                                           \
    else if (ikw == 6) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++]; k6 = _fk[n1++];                                           \
      s1 = _ibuffp[row] + offset;                                               \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_6(s1, k1, k2, k3, k4, k5, k6)                                     \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++;                                                                   \
      }                                                                         \
    }                                                                           \
    else if (ikw == 5) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      k5 = _fk[n1++];                                                           \
      s1 = _ibuffp[row] + offset;                                               \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_5(s1, k1, k2, k3, k4, k5)                                         \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++;                                                                   \
      }                                                                         \
    }                                                                           \
    else if (ikw == 4) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++]; k4 = _fk[n1++];           \
      s1 = _ibuffp[row] + offset;                                               \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_4(s1, k1, k2, k3, k4)                                             \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++;                                                                   \
      }                                                                         \
    }                                                                           \
    else if (ikw == 3) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++]; k3 = _fk[n1++];                           \
      s1 = _ibuffp[row] + offset;                                               \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_3(s1, k1, k2, k3)                                                 \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++;                                                                   \
      }                                                                         \
    }                                                                           \
    else if (ikw == 2) {                                                        \
      k1 = _fk[n1++]; k2 = _fk[n1++];                                           \
      s1 = _ibuffp[row] + offset;                                               \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_2(s1, k1, k2)                                                     \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++;                                                                   \
      }                                                                         \
    }                                                                           \
    else if (ikw == 1) {                                                        \
      k1 = _fk[n1++];                                                           \
      s1 = _ibuffp[row] + offset;                                               \
      s2 = _ibuffp[row+1] + offset;                                             \
      ibufh = _intrm_buf;                                                       \
      for (i = 0; i < _dwid; i += 4) {                                          \
        out0 = ibufh[0];                                                        \
        out1 = ibufh[1];                                                        \
        COUNT_1(s1, k1)                                                         \
        ibufh[0] = out0;                                                        \
        ibufh[1] = out1;                                                        \
        ibufh += 2;                                                             \
        s1++;                                                                   \
      }                                                                         \
    }                                                                           \
  }                                                                             \
}

#ifdef __cplusplus
}
#endif

#endif  /* MLIB_IMAGECONVMXN_16_H */

/***************************************************************/
