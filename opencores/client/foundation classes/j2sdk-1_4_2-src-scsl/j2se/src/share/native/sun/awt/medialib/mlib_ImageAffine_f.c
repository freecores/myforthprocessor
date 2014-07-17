/*
 * @(#)mlib_ImageAffine_f.c	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageAffine_f.c	1.13	00/01/28 SMI"
#endif /* __SUNPRO_C */

/*
 * FUNCTION
 *      mlib_ImageAffine_alltypes - internal function for image affine
 *                                  transformation on all types
 */

#include <stdlib.h>
#include "mlib_image.h"
#include "mlib_ImageCheck.h"

/***************************************************************/

#define LIMIT 600

/***************************************************************/

extern mlib_s32 mlib_type_shift[];

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

#define MLIB_AFFINECALLPARAM_NN                         \
  leftEdges, rightEdges, xStarts, yStarts,              \
             sides, dstData, lineAddr, dstYStride

#define MLIB_AFFINECALLPARAM_BL MLIB_AFFINECALLPARAM_NN, srcYStride

#define MLIB_AFFINECALLPARAM_BC MLIB_AFFINECALLPARAM_NN, srcYStride, filter

/***************************************************************/

typedef void (*fun_type_nn)(MLIB_AFFINEDEFPARAM_NN);
typedef void (*fun_type_bl)(MLIB_AFFINEDEFPARAM_BL);
typedef void (*fun_type_bc)(MLIB_AFFINEDEFPARAM_BC);

/***************************************************************/

void mlib_c_ImageAffine_bit_1ch_nn(MLIB_AFFINEDEFPARAM_NN);

/***************************************************************/

mlib_status mlib_AffineEdges(mlib_s32 *leftEdges,
                             mlib_s32 *rightEdges,
                             mlib_s32 *xStarts,
                             mlib_s32 *yStarts,
                             mlib_s32 *sides,
                             mlib_d64 srcW,
                             mlib_d64 srcH,
                             mlib_d64 dstW,
                             mlib_d64 dstH,
                             mlib_s32 kw,
                             mlib_s32 kh,
                             mlib_s32 kw1,
                             mlib_s32 kh1,
                             mlib_s32 *borders,
                             mlib_edge edge,
                             mlib_d64 *mtx,
                             mlib_s32 shiftx,
                             mlib_s32 shifty);

/***************************************************************/

#define CALC_SRC_EDGES1(edge)                                           \
  if (mlib_AffineEdges(leftEdges, rightEdges, xStarts, yStarts, sides,  \
      srcWidth, srcHeight, dstWidth, dstHeight,                         \
      kw, kw, kw1, kw1, borders, edge, mtx, 16, 16) != MLIB_SUCCESS)    \
    return MLIB_FAILURE

#define CALC_SRC_EDGES2(S, edge)                                                         \
    if (mlib_AffineEdges(leftEdges##S, rightEdges##S, xStarts##S, yStarts##S, sides##S,  \
        srcWidth, srcHeight, dstWidth, dstHeight,                                        \
        kw, kw, kw1, kw1, borders, edge, mtx, 16, 16) != MLIB_SUCCESS)                   \
      return MLIB_FAILURE

/***************************************************************/

    void mlib_ImageAffineEdgeZero(mlib_image *dst,
                                  mlib_s32 *leftEdges,
                                  mlib_s32 *rightEdges,
                                  mlib_s32 *sides,
                                  mlib_s32 *leftEdgesE,
                                  mlib_s32 *rightEdgesE,
                                  mlib_s32 *sidesE);

/***************************************************************/

void mlib_ImageAffineEdgeNearest(mlib_image *dst,
                                 mlib_s32 *leftEdges,
                                 mlib_s32 *rightEdges,
                                 mlib_s32 *sides,
                                 mlib_s32 *leftEdgesE,
                                 mlib_s32 *rightEdgesE,
                                 mlib_s32 *xStartsE,
                                 mlib_s32 *yStartsE,
                                 mlib_s32 *sidesE,
                                 mlib_u8  **lineAddr);

/***************************************************************/

void mlib_ImageAffineEdgeExtend_BL(mlib_image *dst,
                                   mlib_image *src,
                                   mlib_s32 *leftEdges,
                                   mlib_s32 *rightEdges,
                                   mlib_s32 *sides,
                                   mlib_s32 *leftEdgesE,
                                   mlib_s32 *rightEdgesE,
                                   mlib_s32 *xStartsE,
                                   mlib_s32 *yStartsE,
                                   mlib_s32 *sidesE,
                                   mlib_u8  **lineAddr);

/***************************************************************/

void mlib_ImageAffineEdgeExtend_BC(mlib_image *dst,
                                   mlib_image *src,
                                   mlib_s32 *leftEdges,
                                   mlib_s32 *rightEdges,
                                   mlib_s32 *sides,
                                   mlib_s32 *leftEdgesE,
                                   mlib_s32 *rightEdgesE,
                                   mlib_s32 *xStartsE,
                                   mlib_s32 *yStartsE,
                                   mlib_s32 *sidesE,
                                   mlib_u8  **lineAddr);

/***************************************************************/

void mlib_ImageAffineEdgeExtend_BC2(mlib_image *dst,
                                    mlib_image *src,
                                    mlib_s32 *leftEdges,
                                    mlib_s32 *rightEdges,
                                    mlib_s32 *sides,
                                    mlib_s32 *leftEdgesE,
                                    mlib_s32 *rightEdgesE,
                                    mlib_s32 *xStartsE,
                                    mlib_s32 *yStartsE,
                                    mlib_s32 *sidesE,
                                    mlib_u8  **lineAddr);

/***************************************************************/

mlib_status mlib_ImageAffine_alltypes(mlib_image  *dst,
                                      mlib_image  *src,
                                      mlib_d64    *mtx,
                                      mlib_filter filter,
                                      mlib_edge   edge,
                                      fun_type_nn *mlib_AffineFunArr_nn,
                                      fun_type_bl *mlib_AffineFunArr_bl,
                                      fun_type_bc *mlib_AffineFunArr_bc)
{
  mlib_type  type;
  mlib_s32   nchan;
  mlib_s32   srcWidth,  dstWidth, srcHeight, dstHeight;
  mlib_s32   srcYStride, dstYStride;
  mlib_s32   sides[5], type_off;
  mlib_s32   *leftEdges, *rightEdges, *xStarts, *yStarts;
  mlib_s32   leArr[LIMIT], reArr[LIMIT], xsArr[LIMIT], ysArr[LIMIT];
  mlib_u8    *laArr[LIMIT + 4], **lineAddr, *memBuffer = NULL;
  mlib_u8    *srcLinePtr;
  mlib_u8    *srcData, *dstData;
  mlib_s32   kw=0, kw1=0, *borders;
  mlib_s32   i;

  /* check for obvious errors */
  MLIB_IMAGE_TYPE_EQUAL( src, dst );
  MLIB_IMAGE_CHAN_EQUAL( src, dst );

  srcData = mlib_ImageGetData(src);
  dstData = mlib_ImageGetData(dst);
  type    = mlib_ImageGetType(dst);
  nchan   = mlib_ImageGetChannels(dst);
  srcWidth     = mlib_ImageGetWidth(src);
  srcHeight    = mlib_ImageGetHeight(src);
  dstWidth     = mlib_ImageGetWidth(dst);
  dstHeight    = mlib_ImageGetHeight(dst);
  srcYStride   = mlib_ImageGetStride(src);
  dstYStride   = mlib_ImageGetStride(dst);
  borders = mlib_ImageGetBorders(src);

  if (srcHeight < LIMIT && dstHeight < LIMIT) {
    lineAddr    = laArr;
    leftEdges   = leArr;
    rightEdges  = reArr;
    xStarts     = xsArr;
    yStarts     = ysArr;
  } else {
    memBuffer = mlib_malloc(4*dstHeight*sizeof(mlib_s32) +
                            (srcHeight + 4)*sizeof(mlib_u8 *));
    if (memBuffer == NULL)  return MLIB_FAILURE;
    leftEdges   = (mlib_s32 *)(memBuffer);
    rightEdges  = (mlib_s32 *)(memBuffer + dstHeight * sizeof(mlib_s32));
    xStarts     = (mlib_s32 *)(memBuffer + 2 * dstHeight * sizeof(mlib_s32));
    yStarts     = (mlib_s32 *)(memBuffer + 3 * dstHeight * sizeof(mlib_s32));
    lineAddr    = (mlib_u8 **)(memBuffer + 4 * dstHeight * sizeof(mlib_s32));
  }

  switch(filter) {
    case MLIB_NEAREST:
      kw  = 1;
      kw1 = 0;
      break;

    case MLIB_BILINEAR:
      kw  = 2;
      kw1 = 0;
      break;

    case MLIB_BICUBIC:
    case MLIB_BICUBIC2:
      kw  = 4;
      kw1 = 1;
      break;
  }

  CALC_SRC_EDGES1(edge);

  srcLinePtr = (mlib_u8 *)srcData;
  lineAddr += 2;
  for (i = -2; i < srcHeight + 2; i++) {
    lineAddr[i] = srcLinePtr + i * srcYStride;
  }

  dstData += (sides[0]-1)*dstYStride;

  type_off = (type >= MLIB_FLOAT) ? MLIB_FLOAT : MLIB_BYTE;

  if (type == MLIB_BIT) {
    if (nchan != 1 || filter != MLIB_NEAREST) return MLIB_FAILURE;
    mlib_c_ImageAffine_bit_1ch_nn(MLIB_AFFINECALLPARAM_NN);
  } else {
    switch (filter) {
      case MLIB_NEAREST:
        mlib_AffineFunArr_nn[4*(type - type_off) + (nchan - 1)](MLIB_AFFINECALLPARAM_NN);
        break;
      case MLIB_BILINEAR:
        mlib_AffineFunArr_bl[4*(type - type_off) + (nchan - 1)](MLIB_AFFINECALLPARAM_BL);
        break;
      case MLIB_BICUBIC:
      case MLIB_BICUBIC2:
        mlib_AffineFunArr_bc[4*(type - type_off) + (nchan - 1)](MLIB_AFFINECALLPARAM_BC);
        break;
    }
  }

  if (filter != MLIB_NEAREST && edge != MLIB_EDGE_DST_NO_WRITE && edge != MLIB_EDGE_SRC_PADDED)
  {
    mlib_s32  *leftEdgesE, *rightEdgesE, *xStartsE, *yStartsE;
    mlib_s32  leArrE[LIMIT], reArrE[LIMIT], xsArrE[LIMIT], ysArrE[LIMIT];
    mlib_u8   *memBufferE = NULL;
    mlib_s32  sidesE[5];

    if (dstHeight < LIMIT) {
      leftEdgesE   = leArrE;
      rightEdgesE  = reArrE;
      xStartsE     = xsArrE;
      yStartsE     = ysArrE;
    } else {
      memBufferE = mlib_malloc( 4*dstHeight*sizeof(mlib_s32) );

      if (memBufferE == NULL) {
        if (memBuffer != NULL) mlib_free(memBuffer);
        return MLIB_FAILURE;
      }

      leftEdgesE   = (mlib_s32 *)(memBufferE);
      rightEdgesE  = (mlib_s32 *)(memBufferE + dstHeight * sizeof(mlib_s32));
      xStartsE     = (mlib_s32 *)(memBufferE + 2 * dstHeight * sizeof(mlib_s32));
      yStartsE     = (mlib_s32 *)(memBufferE + 3 * dstHeight * sizeof(mlib_s32));
    }

    CALC_SRC_EDGES2(E, -1);

    switch (edge) {
      case MLIB_EDGE_DST_FILL_ZERO:
        mlib_ImageAffineEdgeZero(dst, leftEdges, rightEdges, sides,
                                 leftEdgesE, rightEdgesE, sidesE);
        break;

      case MLIB_EDGE_OP_NEAREST:
        mlib_ImageAffineEdgeNearest(dst, leftEdges, rightEdges, sides,
                                    leftEdgesE, rightEdgesE,
                                    xStartsE, yStartsE, sidesE, lineAddr);
        break;

      case MLIB_EDGE_SRC_EXTEND:
        if (filter == MLIB_BILINEAR)
          mlib_ImageAffineEdgeExtend_BL(dst, src, leftEdges, rightEdges, sides,
                                        leftEdgesE, rightEdgesE, xStartsE, yStartsE,
                                        sidesE, lineAddr);
        else if (filter == MLIB_BICUBIC)
          mlib_ImageAffineEdgeExtend_BC(dst, src, leftEdges, rightEdges, sides,
                                        leftEdgesE, rightEdgesE, xStartsE, yStartsE,
                                        sidesE, lineAddr);
        else if (filter == MLIB_BICUBIC2)
          mlib_ImageAffineEdgeExtend_BC2(dst, src, leftEdges, rightEdges, sides,
                                         leftEdgesE, rightEdgesE, xStartsE, yStartsE,
                                         sidesE, lineAddr);
        break;
    default:
        break;
    }

    if (leftEdgesE != leArrE) {
      mlib_free(memBufferE);
    }
  }

  if (memBuffer != NULL) {
    mlib_free(memBuffer);
  }

  return MLIB_SUCCESS;
}

/***************************************************************/
