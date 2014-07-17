/*
 * @(#)mlib_v_ImageConvCopyEdge.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageConvCopyEdge.c	1.11	99/09/07 SMI"

/*
 * FILENAME: mlib_ImageConvCopyEdge.c
 *
 * FUNCTIONS
 *      mlib_ImageConvCopyEdge  - Copy src edges  to dst edges
 *
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageConvCopyEdge(mlib_image *dst,
 *                                           mlib_image *src,
 *                                           mlib_s32 dx_l, mlib_32 dx_r,
 *                                           mlib_s32 dy_t, mlib_32 dy_b,
 *                                           mlib_s32 cmask);
 *
 * ARGUMENT
 *      dst       Pointer to an dst image.
 *      src       Pointer to an src image.
 *      dx_l      Number of columns on the left side of the
 *                image to be copyed.
 *      dx_r      Number of columns on the right side of the
 *                image to be copyed.
 *      dy_t      Number of rows on the top edge of the
 *                image to be copyed.
 *      dy_b      Number of rows on the top edge of the
 *                image to be copyed.
 *      cmask     Channel mask to indicate the channels to be convolved.
 *                Each bit of which represents a channel in the image. The
 *                channels corresponded to 1 bits are those to be processed.
 *
 * RESTRICTION
 *      The src and the dst must be the same type, same width, same height and have same number
 *      of channels (1, 2, 3, or 4). The unselected channels are not
 *      overwritten. If both src and dst have just one channel,
 *      cmask is ignored.
 *
 * DESCRIPTION
 *      Copy src edges  to dst edges.

 *      The unselected channels are not overwritten.
 *      If src and dst have just one channel,
 *      cmask is ignored.
 */

#include "vis_proto.h"
#include "mlib_image.h"
#include <stdlib.h>

/***************************************************************/

void mlib_ImageConvCopyEdge_U8(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                               mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask, mlib_s32 nchannel);
void mlib_ImageConvCopyEdge_U8_3(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                 mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask);
void mlib_ImageConvCopyEdge_S16(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask, mlib_s32 nchannel);
void mlib_ImageConvCopyEdge_S16_3(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask);
void mlib_ImageConvCopyEdge_S32(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask, mlib_s32 nchannel);
void mlib_ImageConvCopyEdge_S32_3(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask);
void mlib_ImageConvCopyEdge_S32_4(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask);

/***************************************************************/

#define VERT_EDGES(chan, type, mask)                                                    \
  type *pdst = (type *) mlib_ImageGetData(dst); /* pointer to the data of img-image */  \
  type *psrc = (type *) mlib_ImageGetData(src); /* pointer to the data of img-image */  \
  type *pdst_row, *psrc_row, *pdst_row_end;                                             \
  int img_height = mlib_ImageGetHeight(dst);    /* height of source image */            \
  int img_width  = mlib_ImageGetWidth(dst);     /* width of source image */             \
  int dst_stride = mlib_ImageGetStride(dst) / sizeof(type); /* elements to next row */  \
  int src_stride = mlib_ImageGetStride(src) / sizeof(type); /* elements to next row */  \
  int       i, j, l;                            /* indicies */                          \
  int emask, testchan;                                                                  \
  int img_width_t, img_width_b;                                                         \
  mlib_d64 *dpdst, *dpsrc, data0, data1;                                                \
                                                                                        \
  testchan = 1;                                                                         \
  for (l = chan - 1; l >= 0; l--) {                                                     \
    if ((mask & testchan) == 0) {                                                       \
      testchan <<= 1;                                                                   \
      continue;                                                                         \
    }                                                                                   \
    testchan <<= 1;                                                                     \
    for (j = 0; j < dx_l; j++) {                                                        \
      for (i = dy_t; i < (img_height - dy_b); i++) {                                    \
        pdst[i*dst_stride + l + j*chan] = psrc[i*src_stride + l + j*chan];              \
      }                                                                                 \
    }                                                                                   \
    for (j = 0; j < dx_r; j++) {                                                        \
      for (i = dy_t; i < (img_height - dy_b); i++) {                                    \
        pdst[i*dst_stride + l+(img_width-1 - j)*chan] =                                 \
        psrc[i*src_stride + l+(img_width-1 - j)*chan];                                  \
      }                                                                                 \
    }                                                                                   \
  }                                                                                     \
  img_width_t = img_width;                                                              \
  img_width_b = img_width;                                                              \
  if (((img_width * chan) == dst_stride) && ((img_width * chan) == src_stride)) {       \
    img_width_t *= dy_t;                                                                \
    img_width_b *= dy_b;                                                                \
    dst_stride *= (img_height - dy_b);                                                  \
    src_stride *= (img_height - dy_b);                                                  \
    img_height = 2;                                                                     \
    dy_t = ((dy_t == 0) ? 0 : 1);                                                       \
    dy_b = ((dy_b == 0) ? 0 : 1);                                                       \
  }

/***************************************************************/

#define HORIZ_EDGES(chan, type, mask) {                                        \
    testchan = 1;                                                              \
    for (l = chan - 1; l >= 0; l--) {                                          \
      if ((mask & testchan) == 0) {                                            \
        testchan <<= 1;                                                        \
        continue;                                                              \
      }                                                                        \
      testchan <<= 1;                                                          \
      for (i = 0; i < dy_t; i++) {                                             \
        for (j = 0; j < img_width_t; j++) {                                    \
          pdst[i*dst_stride + l + j*chan] =  psrc[i*src_stride + l + j*chan];  \
        }                                                                      \
      }                                                                        \
      for (i = 0; i < dy_b; i++) {                                             \
        for (j = 0; j < img_width_b; j++) {                                    \
          pdst[(img_height-1 - i)*dst_stride + l + j*chan] =                   \
          psrc[(img_height-1 - i)*src_stride + l + j*chan];                    \
        }                                                                      \
      }                                                                        \
    }                                                                          \
    return;                                                                    \
  }

/***************************************************************/

mlib_status mlib_ImageConvCopyEdge(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask)

{

#ifdef MLIB_TEST

  if (dst == NULL || mlib_ImageGetData(dst) == NULL) return MLIB_NULLPOINTER;
  if (dst == NULL || mlib_ImageGetData(dst) == NULL) return MLIB_NULLPOINTER;

  if (mlib_ImageGetWidth(dst) <= 0 || mlib_ImageGetHeight(dst) <= 0 ||
      mlib_ImageGetStride(dst) <= 0) return MLIB_FAILURE;

  if (mlib_ImageGetWidth(src) <= 0 || mlib_ImageGetHeight(src) <= 0 ||
      mlib_ImageGetStride(src) <= 0) return MLIB_FAILURE;

  if (mlib_ImageGetWidth(src)    !=  mlib_ImageGetWidth(dst) ||
      mlib_ImageGetHeight(src)   !=  mlib_ImageGetHeight(dst) ||
      mlib_ImageGetChannels(src) !=  mlib_ImageGetChannels(dst) ||
      mlib_ImageGetType(src)     !=  mlib_ImageGetType(dst))return MLIB_FAILURE;

  if (dx_l <= 0 || dx_r <= 0 || dy_t <= 0 || dy_b <= 0) return MLIB_FAILURE;
  if (((dx_l + dx_r) > mlib_ImageGetWidth(dst)) ||
      ((dy_t + dy_b) > mlib_ImageGetHeight(dst))) return MLIB_FAILURE;

#endif

  switch (mlib_ImageGetType(dst)) {

    case MLIB_BYTE:
      switch (mlib_ImageGetChannels(dst)) {

        case 1:
          mlib_ImageConvCopyEdge_U8(dst, src, dx_l, dx_r, dy_t, dy_b, 1, 1);
          break;

        case 2:
          mlib_ImageConvCopyEdge_U8(dst, src, dx_l, dx_r, dy_t, dy_b, cmask, 2);
          break;

        case 3:
          mlib_ImageConvCopyEdge_U8_3(dst, src, dx_l, dx_r, dy_t, dy_b, cmask);
          break;

        case 4:
          mlib_ImageConvCopyEdge_U8(dst, src, dx_l, dx_r, dy_t, dy_b, cmask, 4);
          break;

        default:
          return MLIB_FAILURE;
      }
      break;

    case MLIB_SHORT:
      switch (mlib_ImageGetChannels(dst)) {

        case 1:
          mlib_ImageConvCopyEdge_S16(dst, src, dx_l, dx_r, dy_t, dy_b, 1, 1);
          break;

        case 2:
          mlib_ImageConvCopyEdge_S16(dst, src, dx_l, dx_r, dy_t, dy_b, cmask, 2);
          break;

        case 3:
          mlib_ImageConvCopyEdge_S16_3(dst, src, dx_l, dx_r, dy_t, dy_b, cmask);
          break;

        case 4:
          mlib_ImageConvCopyEdge_S16(dst, src, dx_l, dx_r, dy_t, dy_b, cmask, 4);
          break;

        default:
          return MLIB_FAILURE;
      }
      break;

    case MLIB_INT:
      switch (mlib_ImageGetChannels(dst)) {

        case 1:
          mlib_ImageConvCopyEdge_S32(dst, src, dx_l, dx_r, dy_t, dy_b, 1, 1);
          break;

        case 2:
          mlib_ImageConvCopyEdge_S32(dst, src, dx_l, dx_r, dy_t, dy_b, cmask, 2);
          break;

        case 3:
          mlib_ImageConvCopyEdge_S32_3(dst, src, dx_l, dx_r, dy_t, dy_b, cmask);
          break;

        case 4:
          mlib_ImageConvCopyEdge_S32_4(dst, src, dx_l, dx_r, dy_t, dy_b, cmask);
          break;

        default:
          return MLIB_FAILURE;
      }
      break;

    default:
      return MLIB_FAILURE;
  }
  return MLIB_SUCCESS;
}

/***************************************************************/

void mlib_ImageConvCopyEdge_U8(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                               mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask, mlib_s32 nchan)
{
  int  tmask = cmask & ((1 << nchan)-1), mask1, offset;
  VERT_EDGES(nchan, mlib_u8, cmask);

  if (img_width < 16/nchan)
    HORIZ_EDGES(nchan, mlib_u8, cmask);
  if (nchan == 1) tmask = 0xFFFF;
  else if (nchan == 2) {
    tmask |= (tmask << 2); tmask |= (tmask << 4); tmask |= (tmask << 8);
  }
  else if (nchan == 4) {
    tmask |= (tmask << 4); tmask |= (tmask << 8);
  }

  for (i = 0; i < dy_t; i++) {
    pdst_row = pdst + i * dst_stride,
             psrc_row = psrc + i * src_stride,
             pdst_row_end = pdst_row + img_width_t * nchan - 1;
    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_u8*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, - offset);
    mask1 = (tmask >> offset);
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge8(pdst_row, pdst_row_end) & mask1;
    vis_pst_8(vis_faligndata(data0, data1), dpdst++, emask);
    j = (mlib_s32) ((mlib_u8*)dpdst - pdst_row);
    data0 = data1;
    for (; j < (img_width_t * nchan - 8); j += 8) {
      data1 = *dpsrc++;
      vis_pst_8(vis_faligndata(data0, data1), dpdst++, mask1);
      data0 = data1;
    }
    data1 = *dpsrc++;
    emask = vis_edge8(dpdst, pdst_row_end) & mask1;
    vis_pst_8(vis_faligndata(data0, data1), dpdst++, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pdst_row = pdst + (img_height-1 - i) * dst_stride;
    psrc_row = psrc + (img_height-1 - i) * src_stride;
    pdst_row_end = pdst_row + img_width_b * nchan - 1;
    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_u8*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, - offset);
    mask1 = (tmask >> offset);
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge8(pdst_row, pdst_row_end) & mask1;
    vis_pst_8(vis_faligndata(data0, data1), dpdst++, emask);
    j = (mlib_s32) ((mlib_u8*)dpdst - pdst_row);
    data0 = data1;
    for (; j < (img_width_b * nchan - 8); j += 8) {
      data1 = *dpsrc++;
      vis_pst_8(vis_faligndata(data0, data1), dpdst++, mask1);
      data0 = data1;
    }
    data1 = *dpsrc++;
    emask = vis_edge8(dpdst, pdst_row_end) & mask1;
    vis_pst_8(vis_faligndata(data0, data1), dpdst++, emask);
  }
}

/***************************************************************/

void mlib_ImageConvCopyEdge_U8_3(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                 mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask)
{
  int      tmask = cmask & 7, mask0, mask1, mask2, offset;

  VERT_EDGES(3, mlib_u8, cmask);

  if (img_width < 16)
    HORIZ_EDGES(3, mlib_u8, cmask);

  tmask |= (tmask << 3); tmask |= (tmask << 6); tmask |= (tmask << 12);
  for (i = 0; i < dy_t; i++) {
    pdst_row = pdst + i * dst_stride,
             psrc_row = psrc + i * src_stride,
             pdst_row_end = pdst_row + img_width_t * 3 - 1;
    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_u8*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, - offset);
    mask2 = (tmask >> (offset+1));
    mask0 = mask2 >> 1;
    mask1 = mask0 >> 1;
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge8(pdst_row, pdst_row_end) & mask2;
    vis_pst_8(vis_faligndata(data0, data1), dpdst++, emask);
    data0 = data1;
    j = (mlib_s32) ((mlib_u8*)dpdst - pdst_row);
    for (; j < (img_width_t * 3 - 24); j += 24) {
      data1 = *dpsrc++;
      vis_pst_8(vis_faligndata(data0, data1), dpdst, mask0);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_8(vis_faligndata(data0, data1), dpdst + 1, mask1);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_8(vis_faligndata(data0, data1), dpdst + 2, mask2);
      data0 = data1;
      dpdst += 3;
    }
    if (j < (img_width_t * 3 - 8)) {
      data1 = *dpsrc++;
      vis_pst_8(vis_faligndata(data0, data1), dpdst++, mask0);
      data0 = data1;
      if (j < (img_width_t * 3 - 16)) {
        data1 = *dpsrc++;
        vis_pst_8(vis_faligndata(data0, data1), dpdst++, mask1);
        data0 = data1;
        mask0 = mask2;
      }
      else {
        mask0 = mask1;
      }
    }
    data1 = *dpsrc++;
    emask = vis_edge8(dpdst, pdst_row_end) & mask0;
    vis_pst_8(vis_faligndata(data0, data1), dpdst, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pdst_row = pdst + (img_height-1 - i) * dst_stride;
    psrc_row = psrc + (img_height-1 - i) * src_stride;
    pdst_row_end = pdst_row + img_width_b * 3 - 1;

    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_u8*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, - offset);
    mask2 = (tmask >> (offset+1));
    mask0 = mask2 >> 1;
    mask1 = mask0 >> 1;
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge8(pdst_row, pdst_row_end) & mask2;
    vis_pst_8(vis_faligndata(data0, data1), dpdst++, emask);
    data0 = data1;
    j = (mlib_s32) ((mlib_u8*)dpdst - pdst_row);
    for (; j < (img_width_b * 3 - 24); j += 24) {
      data1 = *dpsrc++;
      vis_pst_8(vis_faligndata(data0, data1), dpdst, mask0);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_8(vis_faligndata(data0, data1), dpdst + 1, mask1);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_8(vis_faligndata(data0, data1), dpdst + 2, mask2);
      data0 = data1;
      dpdst += 3;
    }
    if (j < (img_width_b * 3 - 8)) {
      data1 = *dpsrc++;
      vis_pst_8(vis_faligndata(data0, data1), dpdst++, mask0);
      data0 = data1;
      if (j < (img_width_b * 3 - 16)) {
        data1 = *dpsrc++;
        vis_pst_8(vis_faligndata(data0, data1), dpdst++, mask1);
        data0 = data1;
        mask0 = mask2;
      }
      else {
        mask0 = mask1;
      }
    }
    data1 = *dpsrc++;
    emask = vis_edge8(dpdst, pdst_row_end) & mask0;
    vis_pst_8(vis_faligndata(data0, data1), dpdst, emask);
  }
}

/***************************************************************/

void mlib_ImageConvCopyEdge_S16(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask, mlib_s32 nchan)
{
  int  tmask = cmask & ((1 << nchan)-1), mask1, offset;
  VERT_EDGES(nchan, mlib_s16, cmask);

  if (img_width < 16/nchan)
    HORIZ_EDGES(nchan, mlib_s16, cmask);
  if (nchan == 1) tmask = 0xFFFF;
  else if (nchan == 2) {
    tmask |= (tmask << 2); tmask |= (tmask << 4);
  }
  else if (nchan == 4) tmask |= (tmask << 4);

  for (i = 0; i < dy_t; i++) {
    pdst_row = pdst + i * dst_stride,
             psrc_row = psrc + i * src_stride,
             pdst_row_end = pdst_row + img_width_t * nchan - 1;
    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_s16*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, -(offset << 1));
    mask1 = (tmask >> offset);
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge16(pdst_row, pdst_row_end) & mask1;
    vis_pst_16(vis_faligndata(data0, data1), dpdst++, emask);
    j = (mlib_s32) ((mlib_s16*)dpdst - pdst_row);
    data0 = data1;
    for (; j < (img_width_t * nchan - 4); j += 4) {
      data1 = *dpsrc++;
      vis_pst_16(vis_faligndata(data0, data1), dpdst++, mask1);
      data0 = data1;
    }
    data1 = *dpsrc++;
    emask = vis_edge16(dpdst, pdst_row_end) & mask1;
    vis_pst_16(vis_faligndata(data0, data1), dpdst++, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pdst_row = pdst + (img_height-1 - i) * dst_stride;
    psrc_row = psrc + (img_height-1 - i) * src_stride;
    pdst_row_end = pdst_row + img_width_b * nchan - 1;
    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_s16*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, -(offset << 1));
    mask1 = (tmask >> offset);
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge16(pdst_row, pdst_row_end) & mask1;
    vis_pst_16(vis_faligndata(data0, data1), dpdst++, emask);
    j = (mlib_s32) ((mlib_s16*)dpdst - pdst_row);
    data0 = data1;
    for (; j < (img_width_b * nchan - 4); j += 4) {
      data1 = *dpsrc++;
      vis_pst_16(vis_faligndata(data0, data1), dpdst++, mask1);
      data0 = data1;
    }
    data1 = *dpsrc++;
    emask = vis_edge16(dpdst, pdst_row_end) & mask1;
    vis_pst_16(vis_faligndata(data0, data1), dpdst++, emask);
  }
}

/***************************************************************/

void mlib_ImageConvCopyEdge_S16_3(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask)
{
  int tmask = cmask & 7, mask0, mask1, mask2, offset;

  VERT_EDGES(3, mlib_s16, cmask);

  if (img_width < 16)
    HORIZ_EDGES(3, mlib_s16, cmask);

  tmask |= (tmask << 3); tmask |= (tmask << 6); tmask |= (tmask << 12);
  for (i = 0; i < dy_t; i++) {
    pdst_row = pdst + i * dst_stride,
             psrc_row = psrc + i * src_stride,
             pdst_row_end = pdst_row + img_width_t * 3 - 1;
    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_s16*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, -(offset << 1));
    mask2 = (tmask >> (offset+2));
    mask0 = mask2 >> 2;
    mask1 = mask0 >> 2;
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge16(pdst_row, pdst_row_end) & mask2;
    vis_pst_16(vis_faligndata(data0, data1), dpdst++, emask);
    data0 = data1;
    j = (mlib_s32) ((mlib_s16*)dpdst - pdst_row);
    for (; j < (img_width_t * 3 - 12); j += 12) {
      data1 = *dpsrc++;
      vis_pst_16(vis_faligndata(data0, data1), dpdst, mask0);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_16(vis_faligndata(data0, data1), dpdst + 1, mask1);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_16(vis_faligndata(data0, data1), dpdst + 2, mask2);
      data0 = data1;
      dpdst += 3;
    }
    if (j < (img_width_t * 3 - 4)) {
      data1 = *dpsrc++;
      vis_pst_16(vis_faligndata(data0, data1), dpdst++, mask0);
      data0 = data1;
      if (j < (img_width_t * 3 - 8)) {
        data1 = *dpsrc++;
        vis_pst_16(vis_faligndata(data0, data1), dpdst++, mask1);
        data0 = data1;
        mask0 = mask2;
      }
      else {
        mask0 = mask1;
      }
    }
    data1 = *dpsrc++;
    emask = vis_edge16(dpdst, pdst_row_end) & mask0;
    vis_pst_16(vis_faligndata(data0, data1), dpdst, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pdst_row = pdst + (img_height-1 - i) * dst_stride;
    psrc_row = psrc + (img_height-1 - i) * src_stride;
    pdst_row_end = pdst_row + img_width_b * 3 - 1;

    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_s16*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, -(offset  << 1));
    mask2 = (tmask >> (offset+2));
    mask0 = mask2 >> 2;
    mask1 = mask0 >> 2;
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge16(pdst_row, pdst_row_end) & mask2;
    vis_pst_16(vis_faligndata(data0, data1), dpdst++, emask);
    data0 = data1;
    j = (mlib_s32) ((mlib_s16*)dpdst - pdst_row);
    for (; j < (img_width_b * 3 - 12); j += 12) {
      data1 = *dpsrc++;
      vis_pst_16(vis_faligndata(data0, data1), dpdst, mask0);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_16(vis_faligndata(data0, data1), dpdst + 1, mask1);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_16(vis_faligndata(data0, data1), dpdst + 2, mask2);
      data0 = data1;
      dpdst += 3;
    }
    if (j < (img_width_b * 3 - 4)) {
      data1 = *dpsrc++;
      vis_pst_16(vis_faligndata(data0, data1), dpdst++, mask0);
      data0 = data1;
      if (j < (img_width_b * 3 - 8)) {
        data1 = *dpsrc++;
        vis_pst_16(vis_faligndata(data0, data1), dpdst++, mask1);
        data0 = data1;
        mask0 = mask2;
      }
      else {
        mask0 = mask1;
      }
    }
    data1 = *dpsrc++;
    emask = vis_edge16(dpdst, pdst_row_end) & mask0;
    vis_pst_16(vis_faligndata(data0, data1), dpdst, emask);
  }
}

/***************************************************************/

void mlib_ImageConvCopyEdge_S32(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask, mlib_s32 nchan)
{
  int  tmask = cmask & ((1 << nchan)-1), mask1, offset;
  VERT_EDGES(nchan, mlib_s32, cmask);

  if (img_width < 16/nchan)
    HORIZ_EDGES(nchan, mlib_s32, cmask);
  if (nchan == 1) tmask = 0xFFFF;
  else if (nchan == 2) {
    tmask |= (tmask << 2); tmask |= (tmask << 4);
  }

  for (i = 0; i < dy_t; i++) {
    pdst_row = pdst + i * dst_stride,
             psrc_row = psrc + i * src_stride,
             pdst_row_end = pdst_row + img_width_t * nchan - 1;
    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_s32*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, -(offset << 2));
    mask1 = (tmask >> offset);
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge32(pdst_row, pdst_row_end) & mask1;
    vis_pst_32(vis_faligndata(data0, data1), dpdst++, emask);
    j = (mlib_s32) ((mlib_s32*)dpdst - pdst_row);
    data0 = data1;
    for (; j < (img_width_t * nchan - 2); j += 2) {
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst++, mask1);
      data0 = data1;
    }
    data1 = *dpsrc++;
    emask = vis_edge32(dpdst, pdst_row_end) & mask1;
    vis_pst_32(vis_faligndata(data0, data1), dpdst++, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pdst_row = pdst + (img_height-1 - i) * dst_stride;
    psrc_row = psrc + (img_height-1 - i) * src_stride;
    pdst_row_end = pdst_row + img_width_b * nchan - 1;
    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_s32*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, -(offset << 2));
    mask1 = (tmask >> offset);
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge32(pdst_row, pdst_row_end) & mask1;
    vis_pst_32(vis_faligndata(data0, data1), dpdst++, emask);
    j = (mlib_s32) ((mlib_s32*)dpdst - pdst_row);
    data0 = data1;
    for (; j < (img_width_b * nchan - 2); j += 2) {
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst++, mask1);
      data0 = data1;
    }
    data1 = *dpsrc++;
    emask = vis_edge32(dpdst, pdst_row_end) & mask1;
    vis_pst_32(vis_faligndata(data0, data1), dpdst++, emask);
  }
}

/***************************************************************/

void mlib_ImageConvCopyEdge_S32_3(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask)
{
  int tmask = cmask & 7, mask0, mask1, mask2, offset;

  VERT_EDGES(3, mlib_s32, cmask);

  if (img_width < 16)
    HORIZ_EDGES(3, mlib_s32, cmask);

  tmask |= (tmask << 3); tmask |= (tmask << 6); tmask |= (tmask << 12);
  for (i = 0; i < dy_t; i++) {
    pdst_row = pdst + i * dst_stride,
             psrc_row = psrc + i * src_stride,
             pdst_row_end = pdst_row + img_width_t * 3 - 1;
    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_s32*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, -(offset << 2));
    mask2 = (tmask >> (offset+1));
    mask0 = mask2 >> 1;
    mask1 = mask0 >> 1;
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge32(pdst_row, pdst_row_end) & mask2;
    vis_pst_32(vis_faligndata(data0, data1), dpdst++, emask);
    data0 = data1;
    j = (mlib_s32) ((mlib_s32*)dpdst - pdst_row);
    for (; j < (img_width_t * 3 - 6); j += 6) {
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst, mask0);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst + 1, mask1);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst + 2, mask2);
      data0 = data1;
      dpdst += 3;
    }
    if (j < (img_width_t * 3 - 2)) {
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst++, mask0);
      data0 = data1;
      if (j < (img_width_t * 3 - 4)) {
        data1 = *dpsrc++;
        vis_pst_32(vis_faligndata(data0, data1), dpdst++, mask1);
        data0 = data1;
        mask0 = mask2;
      }
      else {
        mask0 = mask1;
      }
    }
    data1 = *dpsrc++;
    emask = vis_edge32(dpdst, pdst_row_end) & mask0;
    vis_pst_32(vis_faligndata(data0, data1), dpdst, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pdst_row = pdst + (img_height-1 - i) * dst_stride;
    psrc_row = psrc + (img_height-1 - i) * src_stride;
    pdst_row_end = pdst_row + img_width_b * 3 - 1;

    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_s32*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, -(offset  << 2));
    mask2 = (tmask >> (offset+1));
    mask0 = mask2 >> 1;
    mask1 = mask0 >> 1;
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge32(pdst_row, pdst_row_end) & mask2;
    vis_pst_32(vis_faligndata(data0, data1), dpdst++, emask);
    data0 = data1;
    j = (mlib_s32) ((mlib_s32*)dpdst - pdst_row);
    for (; j < (img_width_b * 3 - 6); j += 6) {
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst, mask0);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst + 1, mask1);
      data0 = data1;
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst + 2, mask2);
      data0 = data1;
      dpdst += 3;
    }
    if (j < (img_width_b * 3 - 2)) {
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst++, mask0);
      data0 = data1;
      if (j < (img_width_b * 3 - 4)) {
        data1 = *dpsrc++;
        vis_pst_32(vis_faligndata(data0, data1), dpdst++, mask1);
        data0 = data1;
        mask0 = mask2;
      }
      else {
        mask0 = mask1;
      }
    }
    data1 = *dpsrc++;
    emask = vis_edge32(dpdst, pdst_row_end) & mask0;
    vis_pst_32(vis_faligndata(data0, data1), dpdst, emask);
  }
}

/***************************************************************/

void mlib_ImageConvCopyEdge_S32_4(mlib_image *dst, mlib_image *src, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 cmask)
{
  int tmask = cmask & 15, mask0, mask1, offset;

  VERT_EDGES(4, mlib_s32, cmask);

  if (img_width < 16)
    HORIZ_EDGES(4, mlib_s32, cmask);

  tmask |= (tmask << 4); tmask |= (tmask << 8);
  for (i = 0; i < dy_t; i++) {
    pdst_row = pdst + i * dst_stride,
             psrc_row = psrc + i * src_stride,
             pdst_row_end = pdst_row + img_width_t * 4 - 1;
    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_s32*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, -(offset << 2));
    mask1 = (tmask >> (offset+2));
    mask0 = mask1 >> 2;
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge32(pdst_row, pdst_row_end) & mask1;
    vis_pst_32(vis_faligndata(data0, data1), dpdst++, emask);
    data0 = data1;
    j = (mlib_s32) ((mlib_s32*)dpdst - pdst_row);
    for (; j < (img_width_t * 4 - 4); j += 4) {
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst, mask0);
      data0 = *dpsrc++;
      vis_pst_32(vis_faligndata(data1, data0), dpdst + 1, mask1);
      dpdst += 2;
    }
    if (j < (img_width_t * 4 - 2)) {
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst++, mask0);
      data0 = data1;
      mask0 = mask1;
    }
    data1 = *dpsrc++;
    emask = vis_edge32(dpdst, pdst_row_end) & mask0;
    vis_pst_32(vis_faligndata(data0, data1), dpdst, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pdst_row = pdst + (img_height-1 - i) * dst_stride;
    psrc_row = psrc + (img_height-1 - i) * src_stride;
    pdst_row_end = pdst_row + img_width_b * 4 - 1;

    dpdst = (mlib_d64 *) ((mlib_addr) pdst_row & ~7);
    offset = pdst_row - (mlib_s32*)dpdst;
    dpsrc = (mlib_d64 *) vis_alignaddr(psrc_row, -(offset << 2));
    mask1 = (tmask >> (offset+2));
    mask0 = mask1 >> 2;
    data0 = *dpsrc++;
    data1 = *dpsrc++;
    emask = vis_edge32(pdst_row, pdst_row_end) & mask1;
    vis_pst_32(vis_faligndata(data0, data1), dpdst++, emask);
    data0 = data1;
    j = (mlib_s32) ((mlib_s32*)dpdst - pdst_row);
    for (; j < (img_width_b * 4 - 4); j += 4) {
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst, mask0);
      data0 = *dpsrc++;
      vis_pst_32(vis_faligndata(data1, data0), dpdst + 1, mask1);
      dpdst += 2;
    }
    if (j < (img_width_b * 4 - 2)) {
      data1 = *dpsrc++;
      vis_pst_32(vis_faligndata(data0, data1), dpdst++, mask0);
      data0 = data1;
      mask0 = mask1;
    }
    data1 = *dpsrc++;
    emask = vis_edge32(dpdst, pdst_row_end) & mask0;
    vis_pst_32(vis_faligndata(data0, data1), dpdst, emask);
  }
}

/***************************************************************/
