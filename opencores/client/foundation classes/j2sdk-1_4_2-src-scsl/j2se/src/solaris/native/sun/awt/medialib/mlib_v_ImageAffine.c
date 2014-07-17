/*
 * @(#)mlib_v_ImageAffine.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageAffine.c	1.44	00/01/31 SMI"

/*
 * FUNCTION
 *      mlib_ImageAffine - image affine transformation with edge condition
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageAffine(mlib_image  *dst,
 *                                   mlib_image  *src,
 *                                   mlib_d64    *mtx,
 *                                   mlib_filter filter,
 *                                   mlib_edge   edge)
 *
 * ARGUMENTS
 *      dst       Pointer to destination image
 *      src       Pointer to source image
 *      mtx       Transformation matrix, where
 *                  mtx[0] holds a;  mtx[1] holds b;
 *                  mtx[2] holds tx; mtx[3] holds c;
 *                  mtx[4] holds d;  mtx[5] holds ty.
 *      filter    Type of resampling filter.
 *      edge      Type of edge condition.
 *
 * DESCRIPTION
 *                      xd = a*xs + b*ys + tx
 *                      yd = c*xs + d*ys + ty
 *
 *  The upper-left corner pixel of an image is located at (0.5, 0.5).
 *
 *  The resampling filter can be one of the following:
 *    MLIB_NEAREST
 *    MLIB_BILINEAR
 *    MLIB_BICUBIC
 *    MLIB_BICUBIC2
 *
 *  The edge condition can be one of the following:
 *    MLIB_EDGE_DST_NO_WRITE  (default)
 *    MLIB_EDGE_DST_FILL_ZERO
 *    MLIB_EDGE_OP_NEAREST
 *    MLIB_EDGE_SRC_EXTEND
 *    MLIB_EDGE_SRC_PADDED
 */

#include <stdlib.h>
#include "mlib_image.h"
#include "mlib_ImageCheck.h"

/***************************************************************/

#define MLIB_AFFINEDEFPARAM_NN          \
  mlib_s32 *leftEdges,                  \
           mlib_s32 *rightEdges,        \
           mlib_s32 *xStarts,           \
           mlib_s32 *yStarts,           \
           mlib_s32 *sides,             \
           mlib_u8  *dstData,           \
           mlib_u8  **lineAddr,         \
           mlib_s32 dstYStride          \

#define MLIB_AFFINEDEFPARAM_BL MLIB_AFFINEDEFPARAM_NN, mlib_s32 srcYStride

#define MLIB_AFFINEDEFPARAM_BC MLIB_AFFINEDEFPARAM_NN, mlib_s32 srcYStride, mlib_filter filter

/***************************************************************/

typedef void (*fun_type_nn)(MLIB_AFFINEDEFPARAM_NN);
typedef void (*fun_type_bl)(MLIB_AFFINEDEFPARAM_BL);
typedef void (*fun_type_bc)(MLIB_AFFINEDEFPARAM_BC);

/***************************************************************/

mlib_status mlib_ImageAffine_alltypes(mlib_image  *dst,
                                      mlib_image  *src,
                                      mlib_d64    *mtx,
                                      mlib_filter filter,
                                      mlib_edge   edge,
                                      fun_type_nn *mlib_AffineFunArr_nn,
                                      fun_type_bl *mlib_AffineFunArr_bl,
                                      fun_type_bc *mlib_AffineFunArr_bc);

/***************************************************************/

void mlib_v_ImageAffine_u8_1ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_v_ImageAffine_u8_2ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_v_ImageAffine_u8_3ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_v_ImageAffine_u8_4ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_v_ImageAffine_u8_1ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_u8_2ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_u8_3ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_u8_4ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_u8_1ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_v_ImageAffine_u8_2ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_v_ImageAffine_u8_3ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_v_ImageAffine_u8_4ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_v_ImageAffine_s16_1ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_v_ImageAffine_s16_2ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_v_ImageAffine_s16_3ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_v_ImageAffine_s16_4ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_v_ImageAffine_s16_1ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_s16_2ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_s16_3ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_s16_4ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_s16_1ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_v_ImageAffine_s16_2ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_v_ImageAffine_s16_3ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_v_ImageAffine_s16_4ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_ImageAffine_s32_1ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_ImageAffine_s32_2ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_ImageAffine_s32_3ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_ImageAffine_s32_4ch_nn(MLIB_AFFINEDEFPARAM_NN);
void mlib_v_ImageAffine_s32_1ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_s32_2ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_s32_3ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_s32_4ch_bl(MLIB_AFFINEDEFPARAM_BL);
void mlib_v_ImageAffine_s32_1ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_v_ImageAffine_s32_2ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_v_ImageAffine_s32_3ch_bc(MLIB_AFFINEDEFPARAM_BC);
void mlib_v_ImageAffine_s32_4ch_bc(MLIB_AFFINEDEFPARAM_BC);

/***************************************************************/

static fun_type_nn mlib_AffineFunArr_nn[] = {
  mlib_v_ImageAffine_u8_1ch_nn,  mlib_v_ImageAffine_u8_2ch_nn,
  mlib_v_ImageAffine_u8_3ch_nn,  mlib_v_ImageAffine_u8_4ch_nn,
  mlib_v_ImageAffine_s16_1ch_nn, mlib_v_ImageAffine_s16_2ch_nn,
  mlib_v_ImageAffine_s16_3ch_nn, mlib_v_ImageAffine_s16_4ch_nn,
  mlib_ImageAffine_s32_1ch_nn,   mlib_ImageAffine_s32_2ch_nn,
  mlib_ImageAffine_s32_3ch_nn,   mlib_ImageAffine_s32_4ch_nn,
};

/***************************************************************/

static fun_type_bl mlib_AffineFunArr_bl[] = {
  mlib_v_ImageAffine_u8_1ch_bl,  mlib_v_ImageAffine_u8_2ch_bl,
  mlib_v_ImageAffine_u8_3ch_bl,  mlib_v_ImageAffine_u8_4ch_bl,
  mlib_v_ImageAffine_s16_1ch_bl, mlib_v_ImageAffine_s16_2ch_bl,
  mlib_v_ImageAffine_s16_3ch_bl, mlib_v_ImageAffine_s16_4ch_bl,
  mlib_v_ImageAffine_s32_1ch_bl, mlib_v_ImageAffine_s32_2ch_bl,
  mlib_v_ImageAffine_s32_3ch_bl, mlib_v_ImageAffine_s32_4ch_bl,
};

/***************************************************************/

static fun_type_bc mlib_AffineFunArr_bc[] = {
  mlib_v_ImageAffine_u8_1ch_bc,  mlib_v_ImageAffine_u8_2ch_bc,
  mlib_v_ImageAffine_u8_3ch_bc,  mlib_v_ImageAffine_u8_4ch_bc,
  mlib_v_ImageAffine_s16_1ch_bc, mlib_v_ImageAffine_s16_2ch_bc,
  mlib_v_ImageAffine_s16_3ch_bc, mlib_v_ImageAffine_s16_4ch_bc,
  mlib_v_ImageAffine_s32_1ch_bc, mlib_v_ImageAffine_s32_2ch_bc,
  mlib_v_ImageAffine_s32_3ch_bc, mlib_v_ImageAffine_s32_4ch_bc,
};

/***************************************************************/

mlib_status mlib_ImageAffine(mlib_image  *dst,
                             mlib_image  *src,
                             mlib_d64    *mtx,
                             mlib_filter filter,
                             mlib_edge   edge)
{
  mlib_type type;

  MLIB_IMAGE_CHECK(src);
  MLIB_IMAGE_CHECK(dst);

  type = mlib_ImageGetType(dst);

  if (type != MLIB_BIT && type != MLIB_BYTE && type != MLIB_SHORT && type != MLIB_INT) {
    return MLIB_FAILURE;
  }

  return mlib_ImageAffine_alltypes(dst, src, mtx, filter, edge,
                                   mlib_AffineFunArr_nn,
                                   mlib_AffineFunArr_bl,
                                   mlib_AffineFunArr_bc);
}

/***************************************************************/
