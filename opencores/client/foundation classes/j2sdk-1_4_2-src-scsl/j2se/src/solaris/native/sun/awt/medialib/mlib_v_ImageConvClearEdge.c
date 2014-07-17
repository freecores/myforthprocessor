/*
 * @(#)mlib_v_ImageConvClearEdge.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageConvClearEdge.c	1.8	99/12/30 SMI"

/*
 * FILENAME: mlib_ImageConvClearEdge.c
 *
 * FUNCTIONS
 *      mlib_ImageConvClearEdge  - Set edge of an image to a specific
 *                                        color. (VIS version)
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageConvClearEdge(mlib_image *img,
 *                                          mlib_s32 dx_l, mlib_32 dx_r,
 *                                          mlib_s32 dy_t, mlib_32 dy_b,
 *                                          mlib_s32 *color,
 *                                          mlib_s32 cmask);
 *
 * ARGUMENT
 *      img       Pointer to an image.
 *      dx_l      Number of columns on the left side of the
 *                image to be cleared.
 *      dx_r      Number of columns on the right side of the
 *                image to be cleared.
 *      dy_t      Number of rows on the top edge of the
 *                image to be cleared.
 *      dy_b      Number of rows on the top edge of the
 *                image to be cleared.
 *      color     Pointer to the color that the edges are set to.
 *      cmask     Channel mask to indicate the channels to be convolved.
 *                Each bit of which represents a channel in the image. The
 *                channels corresponded to 1 bits are those to be processed.
 *
 * RESTRICTION
 *      img can have 1, 2, 3 or 4 channels of MLIB_BYTE or MLIB_SHORT or MLIB_INT
 *      data type.
 *
 * DESCRIPTION
 *      Set edge of an image to a specific color. (VIS version)
 *      The unselected channels are not overwritten.
 *      If src and dst have just one channel,
 *      cmask is ignored.
 */

#include "vis_proto.h"
#include "mlib_image.h"
#include <stdio.h>

/***************************************************************/

void mlib_ImageConvClearEdge_U8_1(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color);
void mlib_ImageConvClearEdge_U8_2(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask);
void mlib_ImageConvClearEdge_U8_3(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask);
void mlib_ImageConvClearEdge_U8_4(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask);
void mlib_ImageConvClearEdge_S16_1(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color);
void mlib_ImageConvClearEdge_S16_2(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask);
void mlib_ImageConvClearEdge_S16_3(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask);
void mlib_ImageConvClearEdge_S16_4(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask);
void mlib_ImageConvClearEdge_S32_1(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color);
void mlib_ImageConvClearEdge_S32_2(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask);
void mlib_ImageConvClearEdge_S32_3(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask);
void mlib_ImageConvClearEdge_S32_4(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask);

/***************************************************************/

#define VERT_EDGES(chan, type, mask)                                                    \
  type *pimg = (type *) mlib_ImageGetData(img); /* pointer to the data of img-image */  \
  type *pimg_row, *pimg_row_end;                                                        \
  type color_i;                                                                         \
  int img_height = mlib_ImageGetHeight(img);    /* height of source image */            \
  int img_width  = mlib_ImageGetWidth(img);     /* width of source image */             \
  int img_stride = mlib_ImageGetStride(img) / sizeof(type); /* elements to next row */  \
  int       i, j, l;                            /* indicies */                          \
  int emask, testchan;                                                                  \
  int img_width_t, img_width_b;                                                         \
  mlib_d64 dcolor, *dpimg;                      /* color for double's stores */         \
                                                                                        \
  testchan = 1;                                                                         \
  for (l = chan - 1; l >= 0; l--) {                                                     \
    if ((mask & testchan) == 0) {                                                       \
      testchan <<= 1;                                                                   \
      continue;                                                                         \
    }                                                                                   \
    testchan <<= 1;                                                                     \
    color_i = (type)color[l];                                                           \
    for (j = 0; j < dx_l; j++) {                                                        \
      for (i = dy_t; i < (img_height - dy_b); i++) {                                    \
        pimg[i*img_stride + l + j*chan] = color_i;                                      \
      }                                                                                 \
    }                                                                                   \
    for (j = 0; j < dx_r; j++) {                                                        \
      for (i = dy_t; i < (img_height - dy_b); i++) {                                    \
        pimg[i*img_stride + l+(img_width-1 - j)*chan] = color_i;                        \
      }                                                                                 \
    }                                                                                   \
  }                                                                                     \
                                                                                        \
  img_width_t = img_width;                                                              \
  img_width_b = img_width;                                                              \
  if ((img_width * chan) == img_stride) {                                               \
    img_width_t *= dy_t;                                                                \
    img_width_b *= dy_b;                                                                \
    img_stride *= (img_height - dy_b);                                                  \
    img_height = 2;                                                                     \
    dy_t = ((dy_t == 0) ? 0 : 1);                                                       \
    dy_b = ((dy_b == 0) ? 0 : 1);                                                       \
  }

/***************************************************************/

#define HORIZ_EDGES(chan, type, mask) {                                \
    testchan = 1;                                                      \
    for (l = chan - 1; l >= 0; l--) {                                  \
      if ((mask & testchan) == 0) {                                    \
        testchan <<= 1;                                                \
        continue;                                                      \
      }                                                                \
      testchan <<= 1;                                                  \
      color_i = (type)color[l];                                        \
      for (i = 0; i < dy_t; i++) {                                     \
        for (j = 0; j < img_width_t; j++) {                            \
          pimg[i*img_stride + l + j*chan] = color_i;                   \
        }                                                              \
      }                                                                \
      for (i = 0; i < dy_b; i++) {                                     \
        for (j = 0; j < img_width_b; j++) {                            \
          pimg[(img_height-1 - i)*img_stride + l + j*chan] = color_i;  \
        }                                                              \
      }                                                                \
    }                                                                  \
    return;                                                            \
  }

/***************************************************************/

mlib_status mlib_ImageConvClearEdge(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                    mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)

{

#ifdef MLIB_TEST

  if (img == NULL || mlib_ImageGetData(img) == NULL || color == NULL) return MLIB_NULLPOINTER;

  if (mlib_ImageGetWidth(img) <= 0 || mlib_ImageGetHeight(img) <= 0 ||
      mlib_ImageGetStride(img) <= 0) return MLIB_FAILURE;

  if (dx_l <= 0 || dx_r <= 0 || dy_t <= 0 || dy_b <= 0) return MLIB_FAILURE;
  if (((dx_l + dx_r) > mlib_ImageGetWidth(img)) ||
      ((dy_t + dy_b) > mlib_ImageGetHeight(img))) return MLIB_FAILURE;

#endif

  switch (mlib_ImageGetType(img)) {

    case MLIB_BYTE:
      switch (mlib_ImageGetChannels(img)) {

        case 1:
          mlib_ImageConvClearEdge_U8_1(img, dx_l, dx_r, dy_t, dy_b, color);
          break;

        case 2:
          mlib_ImageConvClearEdge_U8_2(img, dx_l, dx_r, dy_t, dy_b, color, cmask);
          break;

        case 3:
          mlib_ImageConvClearEdge_U8_3(img, dx_l, dx_r, dy_t, dy_b, color, cmask);
          break;

        case 4:
          mlib_ImageConvClearEdge_U8_4(img, dx_l, dx_r, dy_t, dy_b, color, cmask);
          break;

        default:
          return MLIB_FAILURE;
      }
      break;

    case MLIB_SHORT:
      switch (mlib_ImageGetChannels(img)) {

        case 1:
          mlib_ImageConvClearEdge_S16_1(img, dx_l, dx_r, dy_t, dy_b, color);
          break;

        case 2:
          mlib_ImageConvClearEdge_S16_2(img, dx_l, dx_r, dy_t, dy_b, color, cmask);
          break;

        case 3:
          mlib_ImageConvClearEdge_S16_3(img, dx_l, dx_r, dy_t, dy_b, color, cmask);
          break;

        case 4:
          mlib_ImageConvClearEdge_S16_4(img, dx_l, dx_r, dy_t, dy_b, color, cmask);
          break;

        default:
          return MLIB_FAILURE;
      }
      break;
    case MLIB_INT:
      switch (mlib_ImageGetChannels(img)) {

        case 1:
          mlib_ImageConvClearEdge_S32_1(img, dx_l, dx_r, dy_t, dy_b, color);
          break;

        case 2:
          mlib_ImageConvClearEdge_S32_2(img, dx_l, dx_r, dy_t, dy_b, color, cmask);
          break;

        case 3:
          mlib_ImageConvClearEdge_S32_3(img, dx_l, dx_r, dy_t, dy_b, color, cmask);
          break;

        case 4:
          mlib_ImageConvClearEdge_S32_4(img, dx_l, dx_r, dy_t, dy_b, color, cmask);
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

void mlib_ImageConvClearEdge_U8_1(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color)
{
  mlib_u32 color0 = color[0] & 0xFF;

  VERT_EDGES(1, mlib_u8, 1);

  if (img_width < 16)
    HORIZ_EDGES(1, mlib_u8, 1);

  color0 |= (color0 << 8);  color0 |= (color0 << 16);
  dcolor = vis_to_double_dup(color0);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    emask = vis_edge8(pimg_row, pimg_row_end);
    vis_pst_8(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_u8*)dpimg - pimg_row);
    for (; j < (img_width_t - 8); j += 8) *dpimg++ = dcolor;
    emask = vis_edge8(dpimg, pimg_row_end);
    vis_pst_8(dcolor, dpimg, emask);
  }

  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    emask = vis_edge8(pimg_row, pimg_row_end);
    vis_pst_8(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_u8*)dpimg - pimg_row);
    for (; j < (img_width_b - 8); j += 8) *dpimg++ = dcolor;
    emask = vis_edge8(dpimg, pimg_row_end);
    vis_pst_8(dcolor, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_U8_2(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)
{
  mlib_u32 color0 = color[0] & 0xFF,
           color1 = color[1] & 0xFF;
  mlib_d64 dcolor0;
  int      tmask = cmask & 3, mask1, offset;

  VERT_EDGES(2, mlib_u8, cmask);

  if (img_width < 8)
    HORIZ_EDGES(2, mlib_u8, cmask);

  tmask |= (tmask << 2); tmask |= (tmask << 4); tmask |= (tmask << 8);
  color0 = (color0 << 8) | color1;  color0 |= (color0 << 16);
  dcolor0 = vis_to_double_dup(color0);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t * 2 - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    offset = pimg_row - (mlib_u8*)dpimg;
    mask1 = (tmask >> offset);
    emask = vis_edge8(pimg_row, pimg_row_end) & mask1;
    dcolor = vis_faligndata(dcolor0, dcolor0);
    vis_pst_8(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_u8*)dpimg - pimg_row);
    for (; j < (img_width_t * 2 - 8); j += 8) vis_pst_8(dcolor, dpimg++, mask1);
    emask = vis_edge8(dpimg, pimg_row_end) & mask1;
    vis_pst_8(dcolor, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b * 2 - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    offset = pimg_row - (mlib_u8*)dpimg;
    mask1 = (tmask >> offset);
    emask = vis_edge8(pimg_row, pimg_row_end) & mask1;
    dcolor = vis_faligndata(dcolor0, dcolor0);
    vis_pst_8(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_u8*)dpimg - pimg_row);
    for (; j < (img_width_b * 2 - 8); j += 8) vis_pst_8(dcolor, dpimg++, mask1);
    emask = vis_edge8(dpimg, pimg_row_end) & mask1;
    vis_pst_8(dcolor, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_U8_3(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)
{
  mlib_u32 color0 = color[0] & 0xFF,
           color1 = color[1] & 0xFF,
           color2 = color[2] & 0xFF,
           col;
  mlib_d64 dcolor1, dcolor2, dcolor00, dcolor11, dcolor22;
  int      tmask = cmask & 7, mask0, mask1, mask2, offset;

  VERT_EDGES(3, mlib_u8, cmask);

  if (img_width < 16)
    HORIZ_EDGES(3, mlib_u8, cmask);

  tmask |= (tmask << 3); tmask |= (tmask << 6); tmask |= (tmask << 12);
  col = (color0 << 16) | (color1 << 8) | color2;
  color0 = (col << 8) | color0;
  color1 = (color0 << 8) | color1;
  color2 = (color1 << 8) | color2;
  dcolor = vis_to_double(color0, color1);
  dcolor1 = vis_to_double(color2, color0);
  dcolor2 = vis_to_double(color1, color2);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t * 3 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_u8*)dpimg;
    mask2 = (tmask >> (9 - ((8 - offset) & 7)));
    mask0 = mask2 >> 1;
    mask1 = mask0 >> 1;
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    dcolor22 = vis_faligndata(dcolor2, dcolor);
    dcolor00 = vis_faligndata(dcolor, dcolor1);
    dcolor11 = vis_faligndata(dcolor1, dcolor2);
    emask = vis_edge8(pimg_row, pimg_row_end) & mask2;
    if ((mlib_addr) pimg_row & 7) vis_pst_8(dcolor22, dpimg++, emask);
    j = (mlib_s32) ((mlib_u8*)dpimg - pimg_row);
    for (; j < (img_width_t * 3 - 24); j += 24) {
      vis_pst_8(dcolor00, dpimg, mask0);
      vis_pst_8(dcolor11, dpimg + 1, mask1);
      vis_pst_8(dcolor22, dpimg + 2, mask2);
      dpimg += 3;
    }
    if (j < (img_width_t * 3 - 8)) {
      vis_pst_8(dcolor00, dpimg++, mask0);
      if (j < (img_width_t * 3 - 16)) {
        vis_pst_8(dcolor11, dpimg++, mask1);
        dcolor00 = dcolor22;
        mask0 = mask2;
      }
      else {
        dcolor00 = dcolor11;
        mask0 = mask1;
      }
    }
    emask = vis_edge8(dpimg, pimg_row_end) & mask0;
    vis_pst_8(dcolor00, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b * 3 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_u8*)dpimg;
    mask2 = (tmask >> (9 - ((8 - offset) & 7)));
    mask0 = mask2 >> 1;
    mask1 = mask0 >> 1;
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    dcolor22 = vis_faligndata(dcolor2, dcolor);
    dcolor00 = vis_faligndata(dcolor, dcolor1);
    dcolor11 = vis_faligndata(dcolor1, dcolor2);
    emask = vis_edge8(pimg_row, pimg_row_end) & mask2;
    if ((mlib_addr) pimg_row & 7) vis_pst_8(dcolor22, dpimg++, emask);
    j = (mlib_s32) ((mlib_u8*)dpimg - pimg_row);
    for (; j < (img_width_b * 3 - 24); j += 24) {
      vis_pst_8(dcolor00, dpimg, mask0);
      vis_pst_8(dcolor11, dpimg + 1, mask1);
      vis_pst_8(dcolor22, dpimg + 2, mask2);
      dpimg += 3;
    }
    if (j < (img_width_b * 3 - 8)) {
      vis_pst_8(dcolor00, dpimg++, mask0);
      if (j < (img_width_b * 3 - 16)) {
        vis_pst_8(dcolor11, dpimg++, mask1);
        dcolor00 = dcolor22;
        mask0 = mask2;
      }
      else {
        dcolor00 = dcolor11;
        mask0 = mask1;
      }
    }
    emask = vis_edge8(dpimg, pimg_row_end) & mask0;
    vis_pst_8(dcolor00, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_U8_4(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                  mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)
{
  mlib_u32 color0 = color[0] & 0xFF,
           color1 = color[1] & 0xFF,
           color2 = color[2] & 0xFF,
           color3 = color[3] & 0xFF;
  mlib_d64 dcolor0;
  int      tmask = cmask & 0xF, mask1, offset;

  VERT_EDGES(4, mlib_u8, cmask);

  if (img_width < 4)
    HORIZ_EDGES(4, mlib_u8, cmask);

  tmask |= (tmask << 4); tmask |= (tmask << 8);
  color0 = (color0 << 24) | (color1 << 16) | (color2 << 8) | color3;
  dcolor0 = vis_to_double_dup(color0);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t * 4 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_u8*)dpimg;
    mask1 = (tmask >> offset);
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    emask = vis_edge8(pimg_row, pimg_row_end) & mask1;
    dcolor = vis_faligndata(dcolor0, dcolor0);
    vis_pst_8(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_u8*)dpimg - pimg_row);
    for (; j < (img_width_t * 4 - 8); j += 8) vis_pst_8(dcolor, dpimg++, mask1);
    emask = vis_edge8(dpimg, pimg_row_end) & mask1;
    vis_pst_8(dcolor, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b * 4 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_u8*)dpimg;
    mask1 = (tmask >> offset);
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    emask = vis_edge8(pimg_row, pimg_row_end) & mask1;
    dcolor = vis_faligndata(dcolor0, dcolor0);
    vis_pst_8(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_u8*)dpimg - pimg_row);
    for (; j < (img_width_b * 4 - 8); j += 8) vis_pst_8(dcolor, dpimg++, mask1);
    emask = vis_edge8(dpimg, pimg_row_end) & mask1;
    vis_pst_8(dcolor, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_S16_1(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color)
{
  mlib_u32 color0 = color[0] & 0xFFFF;

  VERT_EDGES(1, mlib_s16, 1);

  if (img_width < 8)
    HORIZ_EDGES(1, mlib_s16, 1);

  color0 |= (color0 << 16);
  dcolor = vis_to_double_dup(color0);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    emask = vis_edge16(pimg_row, pimg_row_end);
    vis_pst_16(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_s16*)dpimg - pimg_row);
    for (; j < (img_width_t - 4); j += 4) *dpimg++ = dcolor;
    emask = vis_edge16(dpimg, pimg_row_end);
    vis_pst_16(dcolor, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    emask = vis_edge16(pimg_row, pimg_row_end);
    vis_pst_16(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_s16*)dpimg - pimg_row);
    for (; j < (img_width_b - 4); j += 4) *dpimg++ = dcolor;
    emask = vis_edge16(dpimg, pimg_row_end);
    vis_pst_16(dcolor, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_S16_2(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)
{
  mlib_u32 color0 = color[0] & 0xFFFF,
           color1 = color[1] & 0xFFFF;
  mlib_d64 dcolor0;
  int      tmask = cmask & 3, mask1, offset;

  VERT_EDGES(2, mlib_s16, cmask);

  if (img_width < 4)
    HORIZ_EDGES(2, mlib_s16, cmask);

  tmask |= (tmask << 2); tmask |= (tmask << 4);
  color0 = (color0 << 16) | color1;
  dcolor0 = vis_to_double_dup(color0);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t * 2 - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    offset = pimg_row - (mlib_s16*)dpimg;
    mask1 = (tmask >> offset);
    emask = vis_edge16(pimg_row, pimg_row_end) & mask1;
    dcolor = vis_faligndata(dcolor0, dcolor0);
    vis_pst_16(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_s16*)dpimg - pimg_row);
    for (; j < (img_width_t * 2 - 4); j += 4) vis_pst_16(dcolor, dpimg++, mask1);
    emask = vis_edge16(dpimg, pimg_row_end) & mask1;
    vis_pst_16(dcolor, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b * 2 - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    offset = pimg_row - (mlib_s16*)dpimg;
    mask1 = (tmask >> offset);
    emask = vis_edge16(pimg_row, pimg_row_end) & mask1;
    dcolor = vis_faligndata(dcolor0, dcolor0);
    vis_pst_16(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_s16*)dpimg - pimg_row);
    for (; j < (img_width_b * 2 - 4); j += 4) vis_pst_16(dcolor, dpimg++, mask1);
    emask = vis_edge16(dpimg, pimg_row_end) & mask1;
    vis_pst_16(dcolor, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_S16_3(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)
{
  mlib_u32 color0 = color[0] & 0xFFFF,
           color1 = color[1] & 0xFFFF,
           color2 = color[2] & 0xFFFF,
           col0, col1, col2;
  mlib_d64 dcolor1, dcolor2, dcolor00, dcolor11, dcolor22;
  int      tmask = cmask & 7, mask0, mask1, mask2, offset;

  VERT_EDGES(3, mlib_s16, cmask);

  if (img_width < 8)
    HORIZ_EDGES(3, mlib_s16, cmask);

  tmask |= (tmask << 3); tmask |= (tmask << 6);  tmask |= (tmask << 12);
  col0 = (color0 << 16) | color1;
  col1 = (color2 << 16) | color0;
  col2 = (color1 << 16) | color2;
  dcolor = vis_to_double(col0, col1);
  dcolor1 = vis_to_double(col2, col0);
  dcolor2 = vis_to_double(col1, col2);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t * 3 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_s16*)dpimg;
    mask2 = (tmask >> (6 - ((4 - offset) & 3)));
    mask0 = mask2 >> 2;
    mask1 = mask0 >> 2;
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    dcolor22 = vis_faligndata(dcolor2, dcolor);
    dcolor00 = vis_faligndata(dcolor, dcolor1);
    dcolor11 = vis_faligndata(dcolor1, dcolor2);
    emask = vis_edge16(pimg_row, pimg_row_end) & mask2;
    if ((mlib_addr) pimg_row & 7) vis_pst_16(dcolor22, dpimg++, emask);
    j = (mlib_s32) ((mlib_s16*)dpimg - pimg_row);
    for (; j < (img_width_t * 3 - 12); j += 12) {
      vis_pst_16(dcolor00, dpimg, mask0);
      vis_pst_16(dcolor11, dpimg + 1, mask1);
      vis_pst_16(dcolor22, dpimg + 2, mask2);
      dpimg += 3;
    }
    if (j < (img_width_t * 3 - 4)) {
      vis_pst_16(dcolor00, dpimg++, mask0);
      if (j < (img_width_t * 3 - 8)) {
        vis_pst_16(dcolor11, dpimg++, mask1);
        dcolor00 = dcolor22;
        mask0 = mask2;
      }
      else {
        dcolor00 = dcolor11;
        mask0 = mask1;
      }
    }
    emask = vis_edge16(dpimg, pimg_row_end) & mask0;
    vis_pst_16(dcolor00, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b * 3 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_s16*)dpimg;
    mask2 = (tmask >> (6 - ((4 - offset) & 3)));
    mask0 = mask2 >> 2;
    mask1 = mask0 >> 2;
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    dcolor22 = vis_faligndata(dcolor2, dcolor);
    dcolor00 = vis_faligndata(dcolor, dcolor1);
    dcolor11 = vis_faligndata(dcolor1, dcolor2);
    emask = vis_edge16(pimg_row, pimg_row_end) & mask2;
    if ((mlib_addr) pimg_row & 7) vis_pst_16(dcolor22, dpimg++, emask);
    j = (mlib_s32) ((mlib_s16*)dpimg - pimg_row);
    for (; j < (img_width_b * 3 - 12); j += 12) {
      vis_pst_16(dcolor00, dpimg, mask0);
      vis_pst_16(dcolor11, dpimg + 1, mask1);
      vis_pst_16(dcolor22, dpimg + 2, mask2);
      dpimg += 3;
    }
    if (j < (img_width_b * 3 - 4)) {
      vis_pst_16(dcolor00, dpimg++, mask0);
      if (j < (img_width_b * 3 - 8)) {
        vis_pst_16(dcolor11, dpimg++, mask1);
        dcolor00 = dcolor22;
        mask0 = mask2;
      }
      else {
        dcolor00 = dcolor11;
        mask0 = mask1;
      }
    }
    emask = vis_edge16(dpimg, pimg_row_end) & mask0;
    vis_pst_16(dcolor00, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_S16_4(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)
{
  mlib_u32 color0 = color[0] & 0xFFFF,
           color1 = color[1] & 0xFFFF,
           color2 = color[2] & 0xFFFF,
           color3 = color[3] & 0xFFFF;
  mlib_d64 dcolor0;
  int      tmask = cmask & 0xF, mask1, offset;

  VERT_EDGES(4, mlib_s16, cmask);

  if (img_width < 4)
    HORIZ_EDGES(4, mlib_s16, cmask);

  tmask |= (tmask << 4);
  color0 = (color0 << 16) | color1;
  color1 = (color2 << 16) | color3;
  dcolor0 = vis_to_double(color0, color1);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t * 4 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_s16*)dpimg;
    mask1 = (tmask >> offset);
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    emask = vis_edge16(pimg_row, pimg_row_end) & mask1;
    dcolor = vis_faligndata(dcolor0, dcolor0);
    vis_pst_16(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_s16*)dpimg - pimg_row);
    for (; j < (img_width_t * 4 - 4); j += 4) vis_pst_16(dcolor, dpimg++, mask1);
    emask = vis_edge16(dpimg, pimg_row_end) & mask1;
    vis_pst_16(dcolor, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b * 4 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_s16*)dpimg;
    mask1 = (tmask >> offset);
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    emask = vis_edge16(pimg_row, pimg_row_end) & mask1;
    dcolor = vis_faligndata(dcolor0, dcolor0);
    vis_pst_16(dcolor, dpimg++, emask) ;
    j = (mlib_s32) ((mlib_s16*)dpimg - pimg_row);
    for (; j < (img_width_b * 4 - 4); j += 4) vis_pst_16(dcolor, dpimg++, mask1);
    emask = vis_edge16(dpimg, pimg_row_end) & mask1;
    vis_pst_16(dcolor, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_S32_1(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color)
{
  mlib_s32 color0 = color[0];

  VERT_EDGES(1, mlib_s32, 1);

  if (img_width < 8)
    HORIZ_EDGES(1, mlib_s32, 1);

  dcolor = vis_to_double_dup(color0);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    emask = vis_edge32(pimg_row, pimg_row_end);
    vis_pst_32(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_s32*)dpimg - pimg_row);
    for (; j < (img_width_t - 2); j += 2) *dpimg++ = dcolor;
    emask = vis_edge32(dpimg, pimg_row_end);
    vis_pst_32(dcolor, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    emask = vis_edge32(pimg_row, pimg_row_end);
    vis_pst_32(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_s32*)dpimg - pimg_row);
    for (; j < (img_width_b - 2); j += 2) *dpimg++ = dcolor;
    emask = vis_edge32(dpimg, pimg_row_end);
    vis_pst_32(dcolor, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_S32_2(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)
{
  mlib_s32 color0 = color[0],
           color1 = color[1];
  mlib_d64 dcolor0;
  int      tmask = cmask & 3, mask1, offset;

  VERT_EDGES(2, mlib_s32, cmask);

  if (img_width < 4)
    HORIZ_EDGES(2, mlib_s32, cmask);

  tmask |= (tmask << 2);
  dcolor0 = vis_to_double(color0, color1);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t * 2 - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    offset = pimg_row - (mlib_s32*)dpimg;
    mask1 = (tmask >> offset);
    emask = vis_edge32(pimg_row, pimg_row_end) & mask1;
    dcolor = vis_faligndata(dcolor0, dcolor0);
    vis_pst_32(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_s32*)dpimg - pimg_row);
    for (; j < (img_width_t * 2 - 2); j += 2) vis_pst_32(dcolor, dpimg++, mask1);
    emask = vis_edge32(dpimg, pimg_row_end) & mask1;
    vis_pst_32(dcolor, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b * 2 - 1;
    dpimg = (mlib_d64 *) vis_alignaddr(pimg_row, 0);
    offset = pimg_row - (mlib_s32*)dpimg;
    mask1 = (tmask >> offset);
    emask = vis_edge32(pimg_row, pimg_row_end) & mask1;
    dcolor = vis_faligndata(dcolor0, dcolor0);
    vis_pst_32(dcolor, dpimg++, emask);
    j = (mlib_s32) ((mlib_s32*)dpimg - pimg_row);
    for (; j < (img_width_b * 2 - 2); j += 2) vis_pst_32(dcolor, dpimg++, mask1);
    emask = vis_edge32(dpimg, pimg_row_end) & mask1;
    vis_pst_32(dcolor, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_S32_3(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)
{
  mlib_s32 color0 = color[0],
           color1 = color[1],
           color2 = color[2];
  mlib_d64 dcolor1, dcolor2, dcolor00, dcolor11, dcolor22;
  int      tmask = cmask & 7, mask0, mask1, mask2, offset;

  VERT_EDGES(3, mlib_s32, cmask);

  if (img_width < 8)
    HORIZ_EDGES(3, mlib_s32, cmask);

  tmask |= (tmask << 3); tmask |= (tmask << 6);
  dcolor = vis_to_double(color0, color1);
  dcolor1 = vis_to_double(color2, color0);
  dcolor2 = vis_to_double(color1, color2);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t * 3 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_s32*)dpimg;
    mask2 = (tmask >> (3 - ((2 - offset) & 1)));
    mask0 = mask2 >> 1;
    mask1 = mask0 >> 1;
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    dcolor22 = vis_faligndata(dcolor2, dcolor);
    dcolor00 = vis_faligndata(dcolor, dcolor1);
    dcolor11 = vis_faligndata(dcolor1, dcolor2);
    emask = vis_edge32(pimg_row, pimg_row_end) & mask2;
    if ((mlib_addr) pimg_row & 7) vis_pst_32(dcolor22, dpimg++, emask);
    j = (mlib_s32) ((mlib_s32*)dpimg - pimg_row);
    for (; j < (img_width_t * 3 - 6); j += 6) {
      vis_pst_32(dcolor00, dpimg, mask0);
      vis_pst_32(dcolor11, dpimg + 1, mask1);
      vis_pst_32(dcolor22, dpimg + 2, mask2);
      dpimg += 3;
    }
    if (j < (img_width_t * 3 - 2)) {
      vis_pst_32(dcolor00, dpimg++, mask0);
      if (j < (img_width_t * 3 - 4)) {
        vis_pst_32(dcolor11, dpimg++, mask1);
        dcolor00 = dcolor22;
        mask0 = mask2;
      }
      else {
        dcolor00 = dcolor11;
        mask0 = mask1;
      }
    }
    emask = vis_edge32(dpimg, pimg_row_end) & mask0;
    vis_pst_32(dcolor00, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b * 3 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_s32*)dpimg;
    mask2 = (tmask >> (3 - ((2 - offset) & 1)));
    mask0 = mask2 >> 1;
    mask1 = mask0 >> 1;
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    dcolor22 = vis_faligndata(dcolor2, dcolor);
    dcolor00 = vis_faligndata(dcolor, dcolor1);
    dcolor11 = vis_faligndata(dcolor1, dcolor2);
    emask = vis_edge32(pimg_row, pimg_row_end) & mask2;
    if ((mlib_addr) pimg_row & 7) vis_pst_32(dcolor22, dpimg++, emask);
    j = (mlib_s32) ((mlib_s32*)dpimg - pimg_row);
    for (; j < (img_width_b * 3 - 6); j += 6) {
      vis_pst_32(dcolor00, dpimg, mask0);
      vis_pst_32(dcolor11, dpimg + 1, mask1);
      vis_pst_32(dcolor22, dpimg + 2, mask2);
      dpimg += 3;
    }
    if (j < (img_width_b * 3 - 2)) {
      vis_pst_32(dcolor00, dpimg++, mask0);
      if (j < (img_width_b * 3 - 4)) {
        vis_pst_32(dcolor11, dpimg++, mask1);
        dcolor00 = dcolor22;
        mask0 = mask2;
      }
      else {
        dcolor00 = dcolor11;
        mask0 = mask1;
      }
    }
    emask = vis_edge32(dpimg, pimg_row_end) & mask0;
    vis_pst_32(dcolor00, dpimg, emask);
  }
}

/***************************************************************/

void mlib_ImageConvClearEdge_S32_4(mlib_image *img, mlib_s32 dx_l, mlib_s32 dx_r,
                                   mlib_s32 dy_t, mlib_s32 dy_b, mlib_s32 *color, mlib_s32 cmask)
{
  mlib_u32 color0 = color[0],
           color1 = color[1],
           color2 = color[2],
           color3 = color[3];
  mlib_d64 dcolor0, dcolor1, dcolor00, dcolor11;
  int      tmask = cmask & 0xF, mask0, mask1, offset;

  VERT_EDGES(4, mlib_s32, cmask);

  if (img_width < 4)
    HORIZ_EDGES(4, mlib_s32, cmask);

  tmask |= (tmask << 4);
  dcolor0 = vis_to_double(color0, color1);
  dcolor1 = vis_to_double(color2, color3);
  for (i = 0; i < dy_t; i++) {
    pimg_row = pimg + i * img_stride;
    pimg_row_end = pimg_row + img_width_t * 4 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_s32*)dpimg;
    mask1 = (tmask >> (4 - ((2 - offset) & 1)));
    mask0 = mask1 >> 2;
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    emask = vis_edge32(pimg_row, pimg_row_end) & mask1;
    dcolor00 = vis_faligndata(dcolor0, dcolor1);
    dcolor11 = vis_faligndata(dcolor1, dcolor0);
    if ((mlib_addr) pimg_row & 7) vis_pst_32(dcolor11, dpimg++, emask);
    j = (mlib_s32) ((mlib_s32*)dpimg - pimg_row);
    for (; j < (img_width_t * 4 - 4); j += 4) {
      vis_pst_32(dcolor00, dpimg, mask0);
      vis_pst_32(dcolor11, dpimg + 1, mask1);
      dpimg += 2;
    }
    if (j < (img_width_t * 4 - 2)) {
      vis_pst_32(dcolor00, dpimg++, mask0);
      dcolor00 = dcolor11;
      mask0 = mask1;
    }
    emask = vis_edge32(dpimg, pimg_row_end) & mask0;
    vis_pst_32(dcolor00, dpimg, emask);
  }
  for (i = 0; i < dy_b; i++) {
    pimg_row = pimg + (img_height-1 - i) * img_stride;
    pimg_row_end = pimg_row + img_width_b * 4 - 1;
    dpimg = (mlib_d64 *) ((mlib_addr) pimg_row & ~7);
    offset = pimg_row - (mlib_s32*)dpimg;
    mask1 = (tmask >> (4 - ((2 - offset) & 1)));
    mask0 = mask1 >> 2;
    vis_alignaddr((void *) (- (mlib_addr) pimg_row), 8);
    emask = vis_edge32(pimg_row, pimg_row_end) & mask1;
    dcolor00 = vis_faligndata(dcolor0, dcolor1);
    dcolor11 = vis_faligndata(dcolor1, dcolor0);
    if ((mlib_addr) pimg_row & 7) vis_pst_32(dcolor11, dpimg++, emask);
    j = (mlib_s32) ((mlib_s32*)dpimg - pimg_row);
    for (; j < (img_width_b * 4 - 4); j += 4) {
      vis_pst_32(dcolor00, dpimg, mask0);
      vis_pst_32(dcolor11, dpimg + 1, mask1);
      dpimg += 2;
    }
    if (j < (img_width_b * 4 - 2)) {
      vis_pst_32(dcolor00, dpimg++, mask0);
      dcolor00 = dcolor11;
      mask0 = mask1;
    }
    emask = vis_edge32(dpimg, pimg_row_end) & mask0;
    vis_pst_32(dcolor00, dpimg, emask);
  }
}

/***************************************************************/
