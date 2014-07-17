/*
 * @(#)mlib_ImageAffineEdgeIndex.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifdef __SUNPRO_C
#pragma ident   "@(#)mlib_ImageAffineEdgeIndex.c	1.12    99/10/22 SMI"
#endif /* __SUNPRO_C */

/*
 * FUNCTION
 *      mlib_ImageAffineEdgeZeroIndex - implementation of MLIB_EDGE_DST_FILL_ZERO
 *                                      edge condition
 *      void mlib_ImageAffineEdgeExtendIndex_BL  - implementation of MLIB_EDGE_SRC_EXTEND
 *                                                 edge condition for MLIB_BILINEAR filter
 *      void mlib_ImageAffineEdgeExtendIndex_BC  - implementation of MLIB_EDGE_SRC_EXTEND
 *                                                 edge condition for MLIB_BICUBIC filter
 *      void mlib_ImageAffineEdgeExtendIndex_BC2 - implementation of MLIB_EDGE_SRC_EXTEND
 *                                                 edge condition for MLIB_BICUBIC2 filter
 *
 * SYNOPSIS
 *      void mlib_ImageAffineEdgeZeroIndex(mlib_image *dst,
 *                                         mlib_s32 *leftEdges,
 *                                         mlib_s32 *rightEdges,
 *                                         mlib_s32 *sides,
 *                                         mlib_s32 *leftEdgesE,
 *                                         mlib_s32 *rightEdgesE,
 *                                         mlib_s32 *sidesE,
 *                                         void     *colormap);
 *      void mlib_ImageAffineEdgeExtendIndex_BL(mlib_image *dst,
 *                                              mlib_image *src,
 *                                              mlib_s32 *leftEdges,
 *                                              mlib_s32 *rightEdges,
 *                                              mlib_s32 *sides,
 *                                              mlib_s32 *leftEdgesE,
 *                                              mlib_s32 *rightEdgesE,
 *                                              mlib_s32 *xStartsE,
 *                                              mlib_s32 *yStartsE,
 *                                              mlib_s32 *sidesE,
 *                                              mlib_u8  **lineAddr,
 *                                              void     *colormap);
 *      void mlib_ImageAffineEdgeExtendIndex_BC(mlib_image *dst,
 *                                              mlib_image *src,
 *                                              mlib_s32 *leftEdges,
 *                                              mlib_s32 *rightEdges,
 *                                              mlib_s32 *sides,
 *                                              mlib_s32 *leftEdgesE,
 *                                              mlib_s32 *rightEdgesE,
 *                                              mlib_s32 *xStartsE,
 *                                              mlib_s32 *yStartsE,
 *                                              mlib_s32 *sidesE,
 *                                              mlib_u8  **lineAddr,
 *                                              void     *colormap);
 *      void mlib_ImageAffineEdgeExtendIndex_BC2(mlib_image *dst,
 *                                               mlib_image *src,
 *                                               mlib_s32 *leftEdges,
 *                                               mlib_s32 *rightEdges,
 *                                               mlib_s32 *sides,
 *                                               mlib_s32 *leftEdgesE,
 *                                               mlib_s32 *rightEdgesE,
 *                                               mlib_s32 *xStartsE,
 *                                               mlib_s32 *yStartsE,
 *                                               mlib_s32 *sidesE,
 *                                               mlib_u8  **lineAddr,
 *                                               void     *colormap);
 *
 * ARGUMENTS
 *      dst         - pointer to the destination image
 *      src         - pointer to the source image
 *      leftEdges   - array[dstHeight] of xLeft coordinates for the
 *                    corresponding interpolation (bilinear or bicubic)
 *      rightEdges  - array[dstHeight] of xRight coordinates for the
 *                    corresponding interpolation (bilinear or bicubic)
 *      sides       - array[4]. sides[0] is yStart, sides[1] is yFinish,
 *                    sides[2] is dx * 65536, sides[3] is dy * 65536 for
 *                    the corresponding interpolation (bilinear or bicubic)
 *      leftEdgesE  - array[dstHeight] of xLeft coordinates for the
 *                    nearest neighbour interpolation
 *      rightEdgesE - array[dstHeight] of xRight coordinates for the
 *                    nearest neighbour interpolation
 *      xStartsE    - array[dstHeight] of xStart * 65536 coordinates
 *                    for the nearest neighbour interpolation
 *      yStartsE    - array[dstHeight] of yStart * 65536 coordinates
 *                    for the nearest neighbour interpolation
 *      sidesE      - array[4]. sides[0] is yStart, sides[1] is yFinish,
 *                    sides[2] is dx * 65536, sides[3] is dy * 65536 for
 *                    nearest neighbour interpolation
 *      lineAddr    - array[srcHeight] of pointers to the first pixel on
 *                    the corresponding lines in source image
 *      colormap    - internal structure
 *
 * DESCRIPTION
 *      mlib_ImageAffineEdgeZeroIndex:
 *      This function fills the edge pixels (i.e. thouse one which can not
 *      be interpolated with given resampling filter because their prototypes
 *      in the source image lie too close to the border) in the destination
 *      image with zeroes.
 *
 *      mlib_ImageAffineEdgeExtendIndex_BL:
 *      This function fills the edge pixels (i.e. thouse one which can not
 *      be interpolated with given resampling filter because their prototypes
 *      in the source image lie too close to the border) in the destination
 *      image according to the bilinear interpolation with border pixels extend
 *      of source image.
 *
 *      mlib_ImageAffineEdgeExtendIndex_BC:
 *      This function fills the edge pixels (i.e. thouse one which can not
 *      be interpolated with given resampling filter because their prototypes
 *      in the source image lie too close to the border) in the destination
 *      image according to the bicubic interpolation with border pixels extend
 *      of source image.
 *
 *      mlib_ImageAffineEdgeExtendIndex_BC2:
 *      This function fills the edge pixels (i.e. thouse one which can not
 *      be interpolated with given resampling filter because their prototypes
 *      in the source image lie too close to the border) in the destination
 *      image according to the bicubic2 interpolation with border pixels extend
 *      of source image.
 */

#include <stdlib.h>
#include "mlib_image.h"
#include "mlib_ImageColormap.h"
#include "mlib_ImageFilters.h"

/***************************************************************/

#define MLIB_LIMIT 1024

/***************************************************************/

void mlib_ImageAffineEdgeZeroIndex(mlib_image *dst,
                                   mlib_s32 *leftEdges,
                                   mlib_s32 *rightEdges,
                                   mlib_s32 *sides,
                                   mlib_s32 *leftEdgesE,
                                   mlib_s32 *rightEdgesE,
                                   mlib_s32 *sidesE,
                                   void     *colormap)
{
  mlib_type type      = mlib_ImageGetType(dst);
  mlib_s32  stride    = mlib_ImageGetStride(dst);
  mlib_u8   *data     = (mlib_u8 *)mlib_ImageGetData(dst);
  mlib_s32  yStart    = sides[0];
  mlib_s32  yFinish   = sides[1];
  mlib_s32  yStartE   = sidesE[0];
  mlib_s32  yFinishE  = sidesE[1];
  mlib_s32  xLeft, xRight, xLeftE, xRightE;
  mlib_s32  j;

  mlib_type cmType    = mlib_ImageGetLutType(colormap);
  mlib_s32  cmChan    = mlib_ImageGetLutChannels(colormap);
  mlib_s32  cmOffset  = mlib_ImageGetLutOffset(colormap);

  data += (yStartE - 1) * stride;

  switch (type) {
    case MLIB_BYTE:
    {
        mlib_u8 *dstPixelPtr;
        mlib_u8 *dstLineEnd;

        for (j = yStartE; j < yStart; j++)
        {
          xLeftE  = leftEdgesE[j];
          xRightE = rightEdgesE[j];
          data   += stride;

          if (xLeftE > xRightE)
            continue;

          dstPixelPtr  = data + xLeftE;
          dstLineEnd   = data + (xRightE + 1);

          for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
            *dstPixelPtr = cmOffset;
        }

        for (; j <= yFinish; j++)
        {
          xLeftE  = leftEdgesE[j];
          xRightE = rightEdgesE[j];
          xLeft   = leftEdges[j];
          xRight  = rightEdges[j];
          data   += stride;

          if (xLeftE > xRightE)
            continue;

          if (xLeft > xRight)
          {
            dstPixelPtr  = data + xLeftE;
            dstLineEnd   = data + (xRightE + 1);

            for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
              *dstPixelPtr = cmOffset;
          }
          else
          {
            dstPixelPtr = data + xLeftE;
            dstLineEnd  = data + xLeft;

            for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
              *dstPixelPtr = cmOffset;

            dstPixelPtr = data + (xRight + 1);
            dstLineEnd  = data + (xRightE + 1);

            for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
              *dstPixelPtr = cmOffset;
          }
        }

        for (; j <= yFinishE; j++)
        {
          xLeftE  = leftEdgesE[j];
          xRightE = rightEdgesE[j];
          data   += stride;

          if (xLeftE > xRightE)
            continue;

          dstPixelPtr  = data + xLeftE;
          dstLineEnd   = data + (xRightE + 1);

          for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
            *dstPixelPtr = cmOffset;
        }
      }
      break;

    case MLIB_SHORT:
    {
        mlib_s16 *dstPixelPtr;
        mlib_s16 *dstLineEnd;

        for (j = yStartE; j < yStart; j++)
        {
          xLeftE  = leftEdgesE[j];
          xRightE = rightEdgesE[j];
          data   += stride;

          if (xLeftE > xRightE)
            continue;

          dstPixelPtr  = (mlib_s16 *)(data + 2 * xLeftE);
          dstLineEnd   = (mlib_s16 *)(data + 2 * (xRightE + 1));

          for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
            *dstPixelPtr = cmOffset;
        }

        for (; j <= yFinish; j++)
        {
          xLeftE  = leftEdgesE[j];
          xRightE = rightEdgesE[j];
          xLeft   = leftEdges[j];
          xRight  = rightEdges[j];
          data   += stride;

          if (xLeftE > xRightE)
            continue;

          if (xLeft > xRight)
          {
            dstPixelPtr  = (mlib_s16 *)(data + 2 * xLeftE);
            dstLineEnd   = (mlib_s16 *)(data + 2 * (xRightE + 1));

            for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
              *dstPixelPtr = cmOffset;
          }
          else
          {
            dstPixelPtr  = (mlib_s16 *)(data + 2 * xLeftE);
            dstLineEnd   = (mlib_s16 *)(data + 2 * xLeft);

            for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
              *dstPixelPtr = cmOffset;

            dstPixelPtr = (mlib_s16 *)(data + 2 * (xRight + 1));
            dstLineEnd  = (mlib_s16 *)(data + 2 * (xRightE + 1));

            for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
              *dstPixelPtr = cmOffset;
          }
        }

        for (; j <= yFinishE; j++)
        {
          xLeftE  = leftEdgesE[j];
          xRightE = rightEdgesE[j];
          data   += stride;

          if (xLeftE > xRightE)
            continue;

          dstPixelPtr  = (mlib_s16 *)(data + 2 * xLeftE);
          dstLineEnd   = (mlib_s16 *)(data + 2 * (xRightE + 1));

          for (; dstPixelPtr < dstLineEnd; dstPixelPtr++)
            *dstPixelPtr = cmOffset;
        }
      }
      break;
  }
}

/***************************************************************/

#define MLIB_SHIFT   16
#define MLIB_SIGN_SHIFT 31
#define MLIB_PREC       (1 << MLIB_SHIFT)
#define MLIB_MASK       (MLIB_PREC - 1)

/***************************************************************/

#define MLIB_EDGE_EXTEND_BL_U8(pixNum)                                   \
  dstPixelPtr  = (mlib_u8 *)dstRowPtr;                                   \
                                                                         \
  for (n = 0; n < pixNum; n++, dstPixelPtr += cmChan)                    \
  {                                                                      \
    ySrc = ((Y - 32768) >> MLIB_SHIFT);                                  \
    xSrc = ((X - 32768) >> MLIB_SHIFT);                                  \
                                                                         \
    t =  ((X - 32768) & MLIB_MASK) * scale;                              \
    u = ((Y - 32768) & MLIB_MASK) * scale;                               \
                                                                         \
    xDelta = (((xSrc + 1 - srcWidth)) >> MLIB_SIGN_SHIFT) & 1;           \
    yDelta = (((ySrc + 1 - srcHeight)) >> MLIB_SIGN_SHIFT) & srcStride;  \
                                                                         \
    xFlag = (xSrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                    \
    xSrc = xSrc + (1 & xFlag);                                           \
    xDelta = xDelta &~ xFlag;                                            \
                                                                         \
    yFlag = (ySrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                    \
    ySrc = ySrc + (1 & yFlag);                                           \
    yDelta = yDelta &~ yFlag;                                            \
                                                                         \
    srcIndexPtr = (IDX_TYPE *)lineAddr[ySrc] + xSrc;                     \
    idx00       = *srcIndexPtr;                                          \
    idx01       = *(srcIndexPtr + xDelta);                               \
    idx10       = *((IDX_TYPE *)((mlib_u8 *)srcIndexPtr + yDelta));      \
    idx11       = *((IDX_TYPE *)((mlib_u8 *)srcIndexPtr + yDelta) +      \
                    xDelta);                                             \
                                                                         \
    for (j = 0; j < cmChan; j++)                                         \
    {                                                                    \
      a00   = *(lut + idx00*cmChan + j);                                 \
      a01   = *(lut + idx01*cmChan + j);                                 \
      a10   = *(lut + idx10*cmChan + j);                                 \
      a11   = *(lut + idx11*cmChan + j);                                 \
      pix0 = a00 + u * (a10 - a00);                                      \
      pix1 = a01 + u * (a11 - a01);                                      \
      dstPixelPtr[j] = pix0 + 0.5 + t * (pix1 - pix0);                   \
    }                                                                    \
                                                                         \
    X += dX;                                                             \
    Y += dY;                                                             \
  }

#define MLIB_EDGE_EXTEND_BL_S16(pixNum)                                  \
  dstPixelPtr  = (mlib_s16 *)dstRowPtr;                                  \
                                                                         \
  for (n = 0; n < pixNum; n++, dstPixelPtr += cmChan)                    \
  {                                                                      \
    ySrc = ((Y - 32768) >> MLIB_SHIFT);                                  \
    xSrc = ((X - 32768) >> MLIB_SHIFT);                                  \
                                                                         \
    t =  ((X - 32768) & MLIB_MASK) * scale;                              \
    u = ((Y - 32768) & MLIB_MASK) * scale;                               \
                                                                         \
    xDelta = (((xSrc + 1 - srcWidth)) >> MLIB_SIGN_SHIFT) & 1;           \
    yDelta = (((ySrc + 1 - srcHeight)) >> MLIB_SIGN_SHIFT) & srcStride;  \
                                                                         \
    xFlag = (xSrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                    \
    xSrc = xSrc + (1 & xFlag);                                           \
    xDelta = xDelta &~ xFlag;                                            \
                                                                         \
    yFlag = (ySrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                    \
    ySrc = ySrc + (1 & yFlag);                                           \
    yDelta = yDelta &~ yFlag;                                            \
                                                                         \
    srcIndexPtr = (IDX_TYPE *)lineAddr[ySrc] + xSrc;                     \
    idx00       = *srcIndexPtr;                                          \
    idx01       = *(srcIndexPtr + xDelta);                               \
    idx10       = *((IDX_TYPE *)((mlib_u8 *)srcIndexPtr + yDelta));      \
    idx11       = *((IDX_TYPE *)((mlib_u8 *)srcIndexPtr + yDelta) +      \
                    xDelta);                                             \
                                                                         \
    for (j = 0; j < cmChan; j++)                                         \
    {                                                                    \
      a00   = *(lut + idx00*cmChan + j);                                 \
      a01   = *(lut + idx01*cmChan + j);                                 \
      a10   = *(lut + idx10*cmChan + j);                                 \
      a11   = *(lut + idx11*cmChan + j);                                 \
      pix0 = a00 + u * (a10 - a00);                                      \
      pix1 = a01 + u * (a11 - a01);                                      \
      pix0 += t * (pix1 - pix0);                                         \
      dstPixelPtr[j] = ((mlib_s32)(pix0 * 65536.) + 32768) >> 16;        \
    }                                                                    \
                                                                         \
    X += dX;                                                             \
    Y += dY;                                                             \
  }

/***************************************************************/

void mlib_ImageAffineEdgeExtendIndex_BL(mlib_image *dst,
                                        mlib_image *src,
                                        mlib_s32 *leftEdges,
                                        mlib_s32 *rightEdges,
                                        mlib_s32 *sides,
                                        mlib_s32 *leftEdgesE,
                                        mlib_s32 *rightEdgesE,
                                        mlib_s32 *xStartsE,
                                        mlib_s32 *yStartsE,
                                        mlib_s32 *sidesE,
                                        mlib_u8  **lineAddr,
                                        void     *colormap)
{
  mlib_type type      = mlib_ImageGetType(dst);
  mlib_s32  dstStride = mlib_ImageGetStride(dst);
  mlib_s32  srcStride = mlib_ImageGetStride(src);
  mlib_s32  srcWidth  = mlib_ImageGetWidth(src);
  mlib_s32  srcHeight = mlib_ImageGetHeight(src);
  mlib_u8   *data     = (mlib_u8 *)mlib_ImageGetData(dst);
  mlib_s32  yStart    = sides[0];
  mlib_s32  yFinish   = sides[1];
  mlib_s32  yStartE   = sidesE[0];
  mlib_s32  yFinishE  = sidesE[1];
  mlib_s32  dX        = sidesE[2];
  mlib_s32  dY        = sidesE[3];
  mlib_d64  scale     = 1.0 / 65536.0;
  mlib_s32  xSrc, ySrc, X, Y, xDelta, yDelta, xFlag, yFlag;
  mlib_s32  xLeft, xRight, xLeftE, xRightE;
  mlib_d64  t, u, pix0, pix1;
  mlib_d64  a00, a01, a10, a11;
  mlib_s32  idx00, idx01, idx10, idx11;
  int       i, j, n;

  mlib_type cmType    = mlib_ImageGetLutType(colormap);
  mlib_s32  cmChan    = mlib_ImageGetLutChannels(colormap);
  mlib_d64 *lut = (mlib_d64*)mlib_ImageGetLutDoubleData(colormap) -
  mlib_ImageGetLutOffset(colormap) * cmChan;

  data += (yStartE - 1) * dstStride;

  switch (type) {

#define IDX_TYPE mlib_u8

    case MLIB_BYTE:
    {
        mlib_u8 *srcIndexPtr;
        mlib_u8 *dstIndexPtr;

        switch (cmType)
        {

#define TRUE2INDEX(pixNum)                                                 \
          switch (cmChan)                                                  \
          {                                                                \
            case 3:                                                        \
              mlib_ImageColorTrue2IndexLine_U8_U8_3((mlib_u8 *)dstRowPtr,  \
                                                    dstIndexPtr,           \
                                                    pixNum,                \
                                                    colormap);             \
              break;                                                       \
                                                                           \
            case 4:                                                        \
              mlib_ImageColorTrue2IndexLine_U8_U8_4((mlib_u8 *)dstRowPtr,  \
                                                    dstIndexPtr,           \
                                                    pixNum,                \
                                                    colormap);             \
              break;                                                       \
            }                                                              \

        case MLIB_BYTE:
        {
            mlib_u8   *dstPixelPtr;
            mlib_d64  dstRowData[MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 2*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(4*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BL_U8((xRightE - xLeftE + 1))
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BL_U8((xRightE - xLeftE + 1))
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BL_U8((xLeft - xLeftE))
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_u8 *)(data + (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BL_U8((xRightE - xRight))
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BL_U8((xRightE - xLeftE + 1))
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 2*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                   \
          switch (cmChan)                                                    \
          {                                                                  \
            case 3:                                                          \
              mlib_ImageColorTrue2IndexLine_S16_U8_3((mlib_s16 *)dstRowPtr,  \
                                                     dstIndexPtr,            \
                                                     pixNum,                 \
                                                     colormap);              \
              break;                                                         \
                                                                             \
            case 4:                                                          \
              mlib_ImageColorTrue2IndexLine_S16_U8_4((mlib_s16 *)dstRowPtr,  \
                                                     dstIndexPtr,            \
                                                     pixNum,                 \
                                                     colormap);              \
              break;                                                         \
            }                                                                \

        case MLIB_SHORT:
        {
            mlib_s16  *dstPixelPtr;
            mlib_d64  dstRowData[2*MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 2*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(8*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BL_S16((xRightE - xLeftE + 1))
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BL_S16((xRightE - xLeftE + 1))
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BL_S16((xLeft - xLeftE))
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_u8 *)(data + (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BL_S16((xRightE - xRight))
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BL_S16((xRightE - xLeftE + 1))
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 2*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;
        }
      }
      break;

#undef IDX_TYPE
#define IDX_TYPE mlib_s16

    case MLIB_SHORT:
    {
        mlib_s16 *srcIndexPtr;
        mlib_s16 *dstIndexPtr;

        switch (cmType)
        {

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                  \
          switch (cmChan)                                                   \
          {                                                                 \
            case 3:                                                         \
              mlib_ImageColorTrue2IndexLine_U8_S16_3((mlib_u8 *)dstRowPtr,  \
                                                     dstIndexPtr,           \
                                                     pixNum,                \
                                                     colormap);             \
              break;                                                        \
                                                                            \
            case 4:                                                         \
              mlib_ImageColorTrue2IndexLine_U8_S16_4((mlib_u8 *)dstRowPtr,  \
                                                     dstIndexPtr,           \
                                                     pixNum,                \
                                                     colormap);             \
              break;                                                        \
            }                                                               \

        case MLIB_BYTE:
        {
            mlib_u8   *dstPixelPtr;
            mlib_d64  dstRowData[MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 4*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(2*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BL_U8((xRightE - xLeftE + 1))
              TRUE2INDEX(xRightE - xLeftE + 1)
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BL_U8((xRightE - xLeftE + 1))
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BL_U8((xLeft - xLeftE))
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_s16 *)(data + 2 * (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BL_U8((xRightE - xRight))
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BL_U8((xRightE - xLeftE + 1))
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 4*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                    \
          switch (cmChan)                                                     \
          {                                                                   \
            case 3:                                                           \
              mlib_ImageColorTrue2IndexLine_S16_S16_3((mlib_s16 *)dstRowPtr,  \
                                                      dstIndexPtr,            \
                                                      pixNum,                 \
                                                      colormap);              \
              break;                                                          \
                                                                              \
            case 4:                                                           \
              mlib_ImageColorTrue2IndexLine_S16_S16_4((mlib_s16 *)dstRowPtr,  \
                                                      dstIndexPtr,            \
                                                      pixNum,                 \
                                                      colormap);              \
              break;                                                          \
            }                                                                 \

        case MLIB_SHORT:
        {
            mlib_s16  *dstPixelPtr;
            mlib_d64  dstRowData[2*MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 4*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(4*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BL_S16((xRightE - xLeftE + 1))
              TRUE2INDEX(xRightE - xLeftE + 1)
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BL_S16((xRightE - xLeftE + 1))
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BL_S16((xLeft - xLeftE))
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_s16 *)(data + 2 * (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BL_S16((xRightE - xRight))
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BL_S16((xRightE - xLeftE + 1))
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 4*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;
        }
      }
      break;
  }
}

/***************************************************************/

#ifdef MLIB_USE_FTOI_CLAMPING

#define SAT8(DST) \
  DST = ((int)(val0 - sat) >> 24) ^ 0x80;

#define SAT16(DST) \
  DST = ((int)(val0)) >> 16;

/***************************************************************/

#else

/***************************************************************/

#define SAT8(DST)                               \
  val0 -= sat;                                  \
  if (val0 >= MLIB_S32_MAX)                     \
    DST = MLIB_U8_MAX;                          \
  else if (val0 <= MLIB_S32_MIN)                \
    DST = MLIB_U8_MIN;                          \
  else                                          \
    DST = ((mlib_s32)val0 >> 24) ^ 0x80;

#define SAT16(DST)                      \
  if (val0 >= MLIB_S32_MAX)             \
    DST = MLIB_S16_MAX;                 \
  else if (val0 <= MLIB_S32_MIN)        \
    DST = MLIB_S16_MIN;                 \
  else                                  \
    DST = (mlib_s32)val0 >> 16;

#endif

/***************************************************************/

#define FILTER_SHIFT_U8  4
#define FILTER_MASK_U8   (((1 << 8) - 1) << 4)
#define FILTER_SHIFT_S16 3
#define FILTER_MASK_S16  (((1 << 9) - 1) << 4)

/***************************************************************/

#define MLIB_EDGE_EXTEND_BC_U8(pixNum, mlib_filters_u8f)                        \
  dstPixelPtr  = (mlib_u8 *)dstRowPtr;                                          \
                                                                                \
  for (n = 0; n < pixNum; n++, dstPixelPtr += cmChan)                           \
  {                                                                             \
    filterpos = ((X - 32768) >> FILTER_SHIFT_U8) & FILTER_MASK_U8;              \
    fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_u8f + filterpos);              \
                                                                                \
    xf0 = fptr[0];                                                              \
    xf1 = fptr[1];                                                              \
    xf2 = fptr[2];                                                              \
    xf3 = fptr[3];                                                              \
                                                                                \
    filterpos = ((Y - 32768) >> FILTER_SHIFT_U8) & FILTER_MASK_U8;              \
    fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_u8f + filterpos);              \
                                                                                \
    yf0 = fptr[0];                                                              \
    yf1 = fptr[1];                                                              \
    yf2 = fptr[2];                                                              \
    yf3 = fptr[3];                                                              \
                                                                                \
    xSrc = ((X - 32768) >> MLIB_SHIFT);                                         \
    ySrc = ((Y - 32768) >> MLIB_SHIFT);                                         \
                                                                                \
    xDelta0 = ((~((xSrc - 1) >> MLIB_SIGN_SHIFT)) & (-1));                      \
    yDelta0 = ((~((ySrc - 1) >> MLIB_SIGN_SHIFT)) & (- srcStride));             \
    xDelta1 = ((xSrc + 1 - srcWidth) >> MLIB_SIGN_SHIFT) & (1);                 \
    yDelta1 = ((ySrc + 1 - srcHeight) >> MLIB_SIGN_SHIFT) &                     \
    (srcStride);                                                                \
    xDelta2 = xDelta1 + (((xSrc + 2 - srcWidth) >> MLIB_SIGN_SHIFT) &           \
                         (1));                                                  \
    yDelta2 = yDelta1 + (((ySrc + 2 - srcHeight) >> MLIB_SIGN_SHIFT) &          \
                         (srcStride));                                          \
                                                                                \
    xFlag = (xSrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                           \
    xSrc = xSrc + (1 & xFlag);                                                  \
    xDelta2 -= (xDelta1 & xFlag);                                               \
    xDelta1 = (xDelta1 &~ xFlag);                                               \
                                                                                \
    yFlag = (ySrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                           \
    ySrc = ySrc + (1 & yFlag);                                                  \
    yDelta2  -= (yDelta1 & yFlag);                                              \
    yDelta1 = yDelta1 &~ yFlag;                                                 \
                                                                                \
    srcIndexPtr = (IDX_TYPE *)lineAddr[ySrc] + xSrc;                            \
                                                                                \
    for (j = 0; j < cmChan; j++)                                                \
    {                                                                           \
                                                                                \
      sPtr = (IDX_TYPE *)((mlib_u8 *)srcIndexPtr + yDelta0);                    \
      idx00 = *(sPtr + xDelta0);                                                \
      idx01 = *(sPtr);                                                          \
      idx10 = *(sPtr + xDelta1);                                                \
      idx11 = *(sPtr + xDelta2);                                                \
                                                                                \
      c0 = (*(lut + idx00*cmChan + j))*xf0 + (*(lut + idx01*cmChan + j))*xf1 +  \
      (*(lut + idx10*cmChan + j))*xf2 + (*(lut + idx11*cmChan + j))*xf3;        \
                                                                                \
      sPtr = (IDX_TYPE *)(srcIndexPtr);                                         \
      idx00 = *(sPtr + xDelta0);                                                \
      idx01 = *(sPtr);                                                          \
      idx10 = *(sPtr + xDelta1);                                                \
      idx11 = *(sPtr + xDelta2);                                                \
                                                                                \
      c1 = (*(lut + idx00*cmChan + j))*xf0 + (*(lut + idx01*cmChan + j))*xf1 +  \
      (*(lut + idx10*cmChan + j))*xf2 + (*(lut + idx11*cmChan + j))*xf3;        \
                                                                                \
      sPtr = (IDX_TYPE *)((mlib_u8 *)srcIndexPtr + yDelta1);                    \
      idx00 = *(sPtr + xDelta0);                                                \
      idx01 = *(sPtr);                                                          \
      idx10 = *(sPtr + xDelta1);                                                \
      idx11 = *(sPtr + xDelta2);                                                \
                                                                                \
      c2 = (*(lut + idx00*cmChan + j))*xf0 + (*(lut + idx01*cmChan + j))*xf1 +  \
      (*(lut + idx10*cmChan + j))*xf2 + (*(lut + idx11*cmChan + j))*xf3;        \
                                                                                \
      sPtr = (IDX_TYPE *)((mlib_u8 *)srcIndexPtr + yDelta2);                    \
      idx00 = *(sPtr + xDelta0);                                                \
      idx01 = *(sPtr);                                                          \
      idx10 = *(sPtr + xDelta1);                                                \
      idx11 = *(sPtr + xDelta2);                                                \
                                                                                \
      c3 = (*(lut + idx00*cmChan + j))*xf0 + (*(lut + idx01*cmChan + j))*xf1 +  \
      (*(lut + idx10*cmChan + j))*xf2 + (*(lut + idx11*cmChan + j))*xf3;        \
                                                                                \
      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                               \
                                                                                \
      SAT8(dstPixelPtr[j])                                                      \
    }                                                                           \
                                                                                \
    X += dX;                                                                    \
    Y += dY;                                                                    \
  }

/***************************************************************/

#define MLIB_EDGE_EXTEND_BC_S16(pixNum, mlib_filters_s16f)                      \
  dstPixelPtr  = (mlib_s16 *)dstRowPtr;                                         \
                                                                                \
  for (n = 0; n < pixNum; n++, dstPixelPtr += cmChan)                           \
  {                                                                             \
    filterpos = ((X - 32768) >> FILTER_SHIFT_S16) & FILTER_MASK_S16;            \
    fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_s16f + filterpos);             \
                                                                                \
    xf0 = fptr[0];                                                              \
    xf1 = fptr[1];                                                              \
    xf2 = fptr[2];                                                              \
    xf3 = fptr[3];                                                              \
                                                                                \
    filterpos = ((Y - 32768) >> FILTER_SHIFT_S16) & FILTER_MASK_S16;            \
    fptr = (mlib_f32 *) ((mlib_u8 *)mlib_filters_s16f + filterpos);             \
                                                                                \
    yf0 = fptr[0];                                                              \
    yf1 = fptr[1];                                                              \
    yf2 = fptr[2];                                                              \
    yf3 = fptr[3];                                                              \
                                                                                \
    xSrc = ((X - 32768) >> MLIB_SHIFT);                                         \
    ySrc = ((Y - 32768) >> MLIB_SHIFT);                                         \
                                                                                \
    xDelta0 = ((~((xSrc - 1) >> MLIB_SIGN_SHIFT)) & (-1));                      \
    yDelta0 = ((~((ySrc - 1) >> MLIB_SIGN_SHIFT)) & (- srcStride));             \
    xDelta1 = ((xSrc + 1 - srcWidth) >> MLIB_SIGN_SHIFT) & (1);                 \
    yDelta1 = ((ySrc + 1 - srcHeight) >> MLIB_SIGN_SHIFT) &                     \
    (srcStride);                                                                \
    xDelta2 = xDelta1 + (((xSrc + 2 - srcWidth) >> MLIB_SIGN_SHIFT) &           \
                         (1));                                                  \
    yDelta2 = yDelta1 + (((ySrc + 2 - srcHeight) >> MLIB_SIGN_SHIFT) &          \
                         (srcStride));                                          \
                                                                                \
    xFlag = (xSrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                           \
    xSrc = xSrc + (1 & xFlag);                                                  \
    xDelta2 -= (xDelta1 & xFlag);                                               \
    xDelta1 = (xDelta1 &~ xFlag);                                               \
                                                                                \
    yFlag = (ySrc >> (MLIB_SIGN_SHIFT - MLIB_SHIFT));                           \
    ySrc = ySrc + (1 & yFlag);                                                  \
    yDelta2  -= (yDelta1 & yFlag);                                              \
    yDelta1 = yDelta1 &~ yFlag;                                                 \
                                                                                \
    srcIndexPtr = (IDX_TYPE *)lineAddr[ySrc] + xSrc;                            \
                                                                                \
    for (j = 0; j < cmChan; j++)                                                \
    {                                                                           \
                                                                                \
      sPtr = (IDX_TYPE *)((mlib_u8 *)srcIndexPtr + yDelta0);                    \
      idx00 = *(sPtr + xDelta0);                                                \
      idx01 = *(sPtr);                                                          \
      idx10 = *(sPtr + xDelta1);                                                \
      idx11 = *(sPtr + xDelta2);                                                \
                                                                                \
      c0 = (*(lut + idx00*cmChan + j))*xf0 + (*(lut + idx01*cmChan + j))*xf1 +  \
      (*(lut + idx10*cmChan + j))*xf2 + (*(lut + idx11*cmChan + j))*xf3;        \
                                                                                \
      sPtr = (IDX_TYPE *)(srcIndexPtr);                                         \
      idx00 = *(sPtr + xDelta0);                                                \
      idx01 = *(sPtr);                                                          \
      idx10 = *(sPtr + xDelta1);                                                \
      idx11 = *(sPtr + xDelta2);                                                \
                                                                                \
      c1 = (*(lut + idx00*cmChan + j))*xf0 + (*(lut + idx01*cmChan + j))*xf1 +  \
      (*(lut + idx10*cmChan + j))*xf2 + (*(lut + idx11*cmChan + j))*xf3;        \
                                                                                \
      sPtr = (IDX_TYPE *)((mlib_u8 *)srcIndexPtr + yDelta1);                    \
      idx00 = *(sPtr + xDelta0);                                                \
      idx01 = *(sPtr);                                                          \
      idx10 = *(sPtr + xDelta1);                                                \
      idx11 = *(sPtr + xDelta2);                                                \
                                                                                \
      c2 = (*(lut + idx00*cmChan + j))*xf0 + (*(lut + idx01*cmChan + j))*xf1 +  \
      (*(lut + idx10*cmChan + j))*xf2 + (*(lut + idx11*cmChan + j))*xf3;        \
                                                                                \
      sPtr = (IDX_TYPE *)((mlib_u8 *)srcIndexPtr + yDelta2);                    \
      idx00 = *(sPtr + xDelta0);                                                \
      idx01 = *(sPtr);                                                          \
      idx10 = *(sPtr + xDelta1);                                                \
      idx11 = *(sPtr + xDelta2);                                                \
                                                                                \
      c3 = (*(lut + idx00*cmChan + j))*xf0 + (*(lut + idx01*cmChan + j))*xf1 +  \
      (*(lut + idx10*cmChan + j))*xf2 + (*(lut + idx11*cmChan + j))*xf3;        \
                                                                                \
      val0 = (c0*yf0 + c1*yf1 + c2*yf2 + c3*yf3);                               \
                                                                                \
      SAT16(dstPixelPtr[j])                                                     \
    }                                                                           \
                                                                                \
    X += dX;                                                                    \
    Y += dY;                                                                    \
  }

/***************************************************************/

void mlib_ImageAffineEdgeExtendIndex_BC(mlib_image *dst,
                                        mlib_image *src,
                                        mlib_s32 *leftEdges,
                                        mlib_s32 *rightEdges,
                                        mlib_s32 *sides,
                                        mlib_s32 *leftEdgesE,
                                        mlib_s32 *rightEdgesE,
                                        mlib_s32 *xStartsE,
                                        mlib_s32 *yStartsE,
                                        mlib_s32 *sidesE,
                                        mlib_u8  **lineAddr,
                                        void     *colormap)
{
  mlib_type type      = mlib_ImageGetType(dst);
  mlib_s32  dstStride = mlib_ImageGetStride(dst);
  mlib_s32  srcStride = mlib_ImageGetStride(src);
  mlib_s32  srcWidth  = mlib_ImageGetWidth(src);
  mlib_s32  srcHeight = mlib_ImageGetHeight(src);
  mlib_u8   *data     = (mlib_u8 *)mlib_ImageGetData(dst);
  mlib_s32  yStart    = sides[0];
  mlib_s32  yFinish   = sides[1];
  mlib_s32  yStartE   = sidesE[0];
  mlib_s32  yFinishE  = sidesE[1];
  mlib_s32  dX        = sidesE[2];
  mlib_s32  dY        = sidesE[3];
  mlib_s32  xSrc, ySrc, X, Y, xFlag, yFlag;
  mlib_s32  xDelta0, xDelta1, xDelta2;
  mlib_s32  yDelta0, yDelta1, yDelta2;
  mlib_s32  xLeft, xRight, xLeftE, xRightE;
  mlib_s32  idx00, idx01, idx10, idx11;
  mlib_s32  i, j, n;
  mlib_d64 sat = (mlib_d64)0x7F800000;
  mlib_d64  scale = 1. / ( 1 << 8);

  mlib_d64  xf0, xf1, xf2, xf3;
  mlib_d64  yf0, yf1, yf2, yf3;
  mlib_d64  c0, c1, c2, c3, val0;
  mlib_s32  filterpos;
  mlib_f32  *fptr;

  mlib_type cmType    = mlib_ImageGetLutType(colormap);
  mlib_s32  cmChan    = mlib_ImageGetLutChannels(colormap);
  mlib_d64 *lut = (mlib_d64*)mlib_ImageGetLutDoubleData(colormap) -
  mlib_ImageGetLutOffset(colormap) * cmChan;

  data += (yStartE - 1) * dstStride;

  switch (type) {

#undef IDX_TYPE
#define IDX_TYPE mlib_u8

    case MLIB_BYTE:
    {
        mlib_u8 *srcIndexPtr;
        mlib_u8 *dstIndexPtr;
        mlib_u8 *sPtr;

        switch (cmType)
      {

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                 \
          switch (cmChan)                                                  \
          {                                                                \
            case 3:                                                        \
              mlib_ImageColorTrue2IndexLine_U8_U8_3((mlib_u8 *)dstRowPtr,  \
                                                    dstIndexPtr,           \
                                                    pixNum,                \
                                                    colormap);             \
              break;                                                       \
                                                                           \
            case 4:                                                        \
              mlib_ImageColorTrue2IndexLine_U8_U8_4((mlib_u8 *)dstRowPtr,  \
                                                    dstIndexPtr,           \
                                                    pixNum,                \
                                                    colormap);             \
              break;                                                       \
            }                                                              \

        case MLIB_BYTE:
        {
            mlib_u8   *dstPixelPtr;
            mlib_d64  dstRowData[MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 2*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(4*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1), mlib_filters_u8f_bc)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1),mlib_filters_u8f_bc)
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BC_U8((xLeft - xLeftE),mlib_filters_u8f_bc)
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_u8 *)(data + (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BC_U8((xRightE - xRight),mlib_filters_u8f_bc)
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1),mlib_filters_u8f_bc)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 2*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                   \
          switch (cmChan)                                                    \
          {                                                                  \
            case 3:                                                          \
              mlib_ImageColorTrue2IndexLine_S16_U8_3((mlib_s16 *)dstRowPtr,  \
                                                     dstIndexPtr,            \
                                                     pixNum,                 \
                                                     colormap);              \
              break;                                                         \
                                                                             \
            case 4:                                                          \
              mlib_ImageColorTrue2IndexLine_S16_U8_4((mlib_s16 *)dstRowPtr,  \
                                                     dstIndexPtr,            \
                                                     pixNum,                 \
                                                     colormap);              \
              break;                                                         \
            }                                                                \

        case MLIB_SHORT:
        {
            mlib_s16  *dstPixelPtr;
            mlib_d64  dstRowData[2*MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 2*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(8*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc)
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BC_S16((xLeft - xLeftE),mlib_filters_s16f_bc)
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_u8 *)(data + (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BC_S16((xRightE - xRight),mlib_filters_s16f_bc)
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 2*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;
        }
      }
      break;

#undef IDX_TYPE
#define IDX_TYPE mlib_s16

    case MLIB_SHORT:
    {
        mlib_s16 *srcIndexPtr;
        mlib_s16 *dstIndexPtr;
        mlib_s16 *sPtr;

        switch (cmType)
        {

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                  \
          switch (cmChan)                                                   \
          {                                                                 \
            case 3:                                                         \
              mlib_ImageColorTrue2IndexLine_U8_S16_3((mlib_u8 *)dstRowPtr,  \
                                                     dstIndexPtr,           \
                                                     pixNum,                \
                                                     colormap);             \
              break;                                                        \
                                                                            \
            case 4:                                                         \
              mlib_ImageColorTrue2IndexLine_U8_S16_4((mlib_u8 *)dstRowPtr,  \
                                                     dstIndexPtr,           \
                                                     pixNum,                \
                                                     colormap);             \
              break;                                                        \
            }                                                               \

        case MLIB_BYTE:
        {
            mlib_u8   *dstPixelPtr;
            mlib_d64  dstRowData[MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 4*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(2*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1),mlib_filters_u8f_bc)
              TRUE2INDEX(xRightE - xLeftE + 1)
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1),mlib_filters_u8f_bc)
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BC_U8((xLeft - xLeftE),mlib_filters_u8f_bc)
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_s16 *)(data + 2 * (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BC_U8((xRightE - xRight),mlib_filters_u8f_bc)
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1),mlib_filters_u8f_bc)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 4*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                    \
          switch (cmChan)                                                     \
          {                                                                   \
            case 3:                                                           \
              mlib_ImageColorTrue2IndexLine_S16_S16_3((mlib_s16 *)dstRowPtr,  \
                                                      dstIndexPtr,            \
                                                      pixNum,                 \
                                                      colormap);              \
              break;                                                          \
                                                                              \
            case 4:                                                           \
              mlib_ImageColorTrue2IndexLine_S16_S16_4((mlib_s16 *)dstRowPtr,  \
                                                      dstIndexPtr,            \
                                                      pixNum,                 \
                                                      colormap);              \
              break;                                                          \
            }                                                                 \

        case MLIB_SHORT:
        {
            mlib_s16  *dstPixelPtr;
            mlib_d64  dstRowData[2*MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 4*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(4*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc)
              TRUE2INDEX(xRightE - xLeftE + 1)
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc)
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BC_S16((xLeft - xLeftE),mlib_filters_s16f_bc)
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_s16 *)(data + 2 * (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BC_S16((xRightE - xRight),mlib_filters_s16f_bc)
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 4*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;
        }
      }
      break;
  }
}

/***************************************************************/

void mlib_ImageAffineEdgeExtendIndex_BC2(mlib_image *dst,
                                         mlib_image *src,
                                         mlib_s32   *leftEdges,
                                         mlib_s32   *rightEdges,
                                         mlib_s32   *sides,
                                         mlib_s32   *leftEdgesE,
                                         mlib_s32   *rightEdgesE,
                                         mlib_s32   *xStartsE,
                                         mlib_s32   *yStartsE,
                                         mlib_s32   *sidesE,
                                         mlib_u8    **lineAddr,
                                         void       *colormap)
{
  mlib_type type      = mlib_ImageGetType(dst);
  mlib_s32  dstStride = mlib_ImageGetStride(dst);
  mlib_s32  srcStride = mlib_ImageGetStride(src);
  mlib_s32  srcWidth  = mlib_ImageGetWidth(src);
  mlib_s32  srcHeight = mlib_ImageGetHeight(src);
  mlib_u8   *data     = (mlib_u8 *)mlib_ImageGetData(dst);
  mlib_s32  yStart    = sides[0];
  mlib_s32  yFinish   = sides[1];
  mlib_s32  yStartE   = sidesE[0];
  mlib_s32  yFinishE  = sidesE[1];
  mlib_s32  dX        = sidesE[2];
  mlib_s32  dY        = sidesE[3];
  mlib_s32  xSrc, ySrc, X, Y, xFlag, yFlag;
  mlib_s32  xDelta0, xDelta1, xDelta2;
  mlib_s32  yDelta0, yDelta1, yDelta2;
  mlib_s32  xLeft, xRight, xLeftE, xRightE;
  mlib_s32  idx00, idx01, idx10, idx11;
  mlib_s32  i, j, n;
  mlib_d64 sat = (mlib_d64)0x7F800000;
  mlib_d64  scale = 1. / ( 1 << 8);

  mlib_d64  xf0, xf1, xf2, xf3;
  mlib_d64  yf0, yf1, yf2, yf3;
  mlib_d64  c0, c1, c2, c3, val0;
  mlib_s32  filterpos;
  mlib_f32  *fptr;

  mlib_type cmType    = mlib_ImageGetLutType(colormap);
  mlib_s32  cmChan    = mlib_ImageGetLutChannels(colormap);
  mlib_d64 *lut = (mlib_d64*)mlib_ImageGetLutDoubleData(colormap) -
  mlib_ImageGetLutOffset(colormap) * cmChan;

  data += (yStartE - 1) * dstStride;

  switch (type) {

#undef IDX_TYPE
#define IDX_TYPE mlib_u8

    case MLIB_BYTE:
    {
        mlib_u8 *srcIndexPtr;
        mlib_u8 *dstIndexPtr;
        mlib_u8 *sPtr;

        switch (cmType)
      {

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                 \
          switch (cmChan)                                                  \
          {                                                                \
            case 3:                                                        \
              mlib_ImageColorTrue2IndexLine_U8_U8_3((mlib_u8 *)dstRowPtr,  \
                                                    dstIndexPtr,           \
                                                    pixNum,                \
                                                    colormap);             \
              break;                                                       \
                                                                           \
            case 4:                                                        \
              mlib_ImageColorTrue2IndexLine_U8_U8_4((mlib_u8 *)dstRowPtr,  \
                                                    dstIndexPtr,           \
                                                    pixNum,                \
                                                    colormap);             \
              break;                                                       \
            }                                                              \

        case MLIB_BYTE:
        {
            mlib_u8   *dstPixelPtr;
            mlib_d64  dstRowData[MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 2*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(4*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1), mlib_filters_u8f_bc2)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1),mlib_filters_u8f_bc2)
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BC_U8((xLeft - xLeftE),mlib_filters_u8f_bc2)
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_u8 *)(data + (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BC_U8((xRightE - xRight),mlib_filters_u8f_bc2)
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1),mlib_filters_u8f_bc2)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 2*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                   \
          switch (cmChan)                                                    \
          {                                                                  \
            case 3:                                                          \
              mlib_ImageColorTrue2IndexLine_S16_U8_3((mlib_s16 *)dstRowPtr,  \
                                                     dstIndexPtr,            \
                                                     pixNum,                 \
                                                     colormap);              \
              break;                                                         \
                                                                             \
            case 4:                                                          \
              mlib_ImageColorTrue2IndexLine_S16_U8_4((mlib_s16 *)dstRowPtr,  \
                                                     dstIndexPtr,            \
                                                     pixNum,                 \
                                                     colormap);              \
              break;                                                         \
            }                                                                \

        case MLIB_SHORT:
        {
            mlib_s16  *dstPixelPtr;
            mlib_d64  dstRowData[2*MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 2*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(8*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc2)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc2)
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

                MLIB_EDGE_EXTEND_BC_S16((xLeft - xLeftE),mlib_filters_s16f_bc2)
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_u8 *)(data + (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BC_S16((xRightE - xRight),mlib_filters_s16f_bc2)
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_u8 *)(data + xLeftE);

              MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc2)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 2*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;
        }
      }
      break;

#undef IDX_TYPE
#define IDX_TYPE mlib_s16

    case MLIB_SHORT:
    {
        mlib_s16 *srcIndexPtr;
        mlib_s16 *dstIndexPtr;
        mlib_s16 *sPtr;

        switch (cmType)
        {

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                  \
          switch (cmChan)                                                   \
          {                                                                 \
            case 3:                                                         \
              mlib_ImageColorTrue2IndexLine_U8_S16_3((mlib_u8 *)dstRowPtr,  \
                                                     dstIndexPtr,           \
                                                     pixNum,                \
                                                     colormap);             \
              break;                                                        \
                                                                            \
            case 4:                                                         \
              mlib_ImageColorTrue2IndexLine_U8_S16_4((mlib_u8 *)dstRowPtr,  \
                                                     dstIndexPtr,           \
                                                     pixNum,                \
                                                     colormap);             \
              break;                                                        \
            }                                                               \

        case MLIB_BYTE:
        {
            mlib_u8   *dstPixelPtr;
            mlib_d64  dstRowData[MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 4*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(2*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1),mlib_filters_u8f_bc2)
              TRUE2INDEX(xRightE - xLeftE + 1)
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1),mlib_filters_u8f_bc2)
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BC_U8((xLeft - xLeftE),mlib_filters_u8f_bc2)
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_s16 *)(data + 2 * (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BC_U8((xRightE - xRight),mlib_filters_u8f_bc2)
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BC_U8((xRightE - xLeftE + 1),mlib_filters_u8f_bc2)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 4*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;

#undef TRUE2INDEX
#define TRUE2INDEX(pixNum)                                                    \
          switch (cmChan)                                                     \
          {                                                                   \
            case 3:                                                           \
              mlib_ImageColorTrue2IndexLine_S16_S16_3((mlib_s16 *)dstRowPtr,  \
                                                      dstIndexPtr,            \
                                                      pixNum,                 \
                                                      colormap);              \
              break;                                                          \
                                                                              \
            case 4:                                                           \
              mlib_ImageColorTrue2IndexLine_S16_S16_4((mlib_s16 *)dstRowPtr,  \
                                                      dstIndexPtr,            \
                                                      pixNum,                 \
                                                      colormap);              \
              break;                                                          \
            }                                                                 \

        case MLIB_SHORT:
        {
            mlib_s16  *dstPixelPtr;
            mlib_d64  dstRowData[2*MLIB_LIMIT];
            mlib_d64  *dstRowPtr = dstRowData;

            if (dstStride > 4*MLIB_LIMIT)
              dstRowPtr = (mlib_d64 *)mlib_malloc(4*dstStride);

            if (dstRowPtr == NULL)
              return;

            for (i = yStartE; i < yStart; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc2)
              TRUE2INDEX(xRightE - xLeftE + 1)
            }

            for (; i <= yFinish; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              xLeft   = leftEdges[i];
              xRight  = rightEdges[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              if (xLeft > xRight)
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc2)
                TRUE2INDEX((xRightE - xLeftE + 1))
              }
              else
              {
                dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

                MLIB_EDGE_EXTEND_BC_S16((xLeft - xLeftE),mlib_filters_s16f_bc2)
                TRUE2INDEX((xLeft - xLeftE))

                dstIndexPtr = (mlib_s16 *)(data + 2 * (xRight + 1));

                X += dX * (xRight - xLeft + 1);
                Y += dY * (xRight - xLeft + 1);

                MLIB_EDGE_EXTEND_BC_S16((xRightE - xRight),mlib_filters_s16f_bc2)
                TRUE2INDEX((xRightE - xRight))
              }
            }

            for (; i <= yFinishE; i++)
            {
              xLeftE  = leftEdgesE[i];
              xRightE = rightEdgesE[i];
              X       = xStartsE[i];
              Y       = yStartsE[i];
              data   += dstStride;

              if (xLeftE > xRightE)
                continue;

              dstIndexPtr  = (mlib_s16 *)(data + 2 * xLeftE);

              MLIB_EDGE_EXTEND_BC_S16((xRightE - xLeftE + 1),mlib_filters_s16f_bc2)
              TRUE2INDEX((xRightE - xLeftE + 1))
            }

            if (dstStride > 4*MLIB_LIMIT)
              mlib_free(dstRowPtr);
          }
          break;
        }
      }
      break;
  }
}

/***************************************************************/
