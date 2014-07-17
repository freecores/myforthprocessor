/*
 * @(#)mlib_ImageScanPoly.c	1.22 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageScanPoly.c	1.29	99/12/27 SMI"
#endif /* __SUNPRO_C */

/*
 *  FUNCTIONS
 *    mlib_AffineEdges
 *
 *  SYNOPSIS
 *    mlib_status mlib_AffineEdges(mlib_s32 *leftEdges,
 *                                 mlib_s32 *rightEdges,
 *                                 mlib_s32 *xStarts,
 *                                 mlib_s32 *yStarts,
 *                                 mlib_s32 *sides,
 *                                 mlib_d64 xClip,
 *                                 mlib_d64 yClip,
 *                                 mlib_d64 wClip,
 *                                 mlib_d64 hClip,
 *                                 mlib_d64 dstW,
 *                                 mlib_d64 dstH,
 *                                 mlib_d64 *mtx,
 *                                 mlib_d64 delta)
 *
 *  ARGUMENTS
 *    leftEdges   - output array[dstHeight] of xLeft coordinates
 *    rightEdges  - output array[dstHeight] of xRight coordinates
 *    xStarts     - output array[dstHeight] of xStart * 65536 coordinates
 *    yStarts     - output array[dstHeight] of yStart * 65536 coordinates
 *    sides       - output array[4]. sides[0] is yStart, sides[1] is yFinish,
 *                  sides[2] is dx * 65536, sides[3] is dy * 65536
 *    xClip       - input parameter of left clipping value
 *    yClip       - input parameter of top clipping value
 *    wClip       - input parameter of right clipping value
 *    hClip       - input parameter of bottom clipping value
 *    dstW        - width of destination image
 *    dstH        - height of destination image
 *    mtx         - input transformation matrix, see mlib_ImageAffine function
 *    delta       - must be 0. when the MLIB_NEAREST filter is used and -0.5
 *                  when another filter is applied
 *  RETURNS
 *    MLIB_FAILURE or
 *    MLIB_SUCCESS
 *
 *  DESCRIPTION
 *    This function calculates five output arrays of boundary coordinates
 *    for the mlib_ImageAffine function case depends on clipping values
 *    destination width and height, mtx and delta value.
 *
 *  COMMENTS
 *    First four arrays must have length same with width of destination
 *    image. Function writes values in these arrays with yStart to yFinish
 *    indexes.
 *    Used in C and VIS versions of mlib_ImageAffine function.
 *
 */

#include <stdlib.h>
#include <mlib_SysMath.h>

#include "mlib_image.h"

/***************************************************************/

mlib_status mlib_AffineEdges(mlib_s32 *leftEdges,
                             mlib_s32 *rightEdges,
                             mlib_s32 *xStarts,
                             mlib_s32 *yStarts,
                             mlib_s32 *sides,
                             mlib_d64 srcWidth,
                             mlib_d64 srcHeight,
                             mlib_d64 dstWidth,
                             mlib_d64 dstHeight,
                             mlib_s32 kw,
                             mlib_s32 kh,
                             mlib_s32 kw1,
                             mlib_s32 kh1,
                             mlib_s32 *borders,
                             mlib_edge edge,
                             mlib_d64 *mtx,
                             mlib_s32 shiftx,
                             mlib_s32 shifty)
{
  mlib_d64 xClip, yClip, wClip, hClip;
  mlib_d64 delta = 0.;
  mlib_d64 minX, minY, maxX, maxY;

  mlib_d64 coords[4][2];
  mlib_d64 a = mtx[0], b = mtx[1], tx = mtx[2],
           c = mtx[3], d = mtx[4], ty = mtx[5];
  mlib_d64 a2, b2, tx2, c2, d2, ty2;
  mlib_d64 dx, dy, div;
  mlib_s32 sdx, sdy;
  mlib_d64 dTop;
  mlib_s32 top, bot;
  mlib_s32 topIdx, max_xsize = 0;
  mlib_s32 i, j, t;

  if ((int)edge < 0) { /* process edges */
    minX = 0;
    minY = 0;
    maxX = srcWidth;
    maxY = srcHeight;

  } else {
    if (kw > 1) delta = -0.5; /* for MLIB_NEAREST filter delta = 0. */

    minX = (kw1 - delta);
    minY = (kh1 - delta);
    maxX = srcWidth  - ((kw - 1) - (kw1 - delta));
    maxY = srcHeight - ((kh - 1) - (kh1 - delta));

    if (edge == MLIB_EDGE_SRC_PADDED) {
      if (minX < borders[0]) minX = borders[0];
      if (minY < borders[1]) minY = borders[1];
      if (maxX > borders[2]) maxX = borders[2];
      if (maxY > borders[3]) maxY = borders[3];
    }
  }

  xClip = minX; yClip = minY; wClip = maxX; hClip = maxY;

  sides[4] = 0;

  div = a * d - c * b;
  if (div == 0.) return MLIB_FAILURE;

  if ((xClip >= wClip) || (yClip >= hClip)) {
    sides[0] = 0;
    sides[1] = -1;
    sides[2] = 1;
    sides[3] = 1;
    return MLIB_SUCCESS;
  }

  a2 = d; b2 = - b; tx2 = (- d * tx + b * ty);
  c2 = - c; d2 = a; ty2 = ( c * tx  - a * ty);

  dx = a2;
  dy = c2;

  tx -= 0.5;
  ty -= 0.5;

  coords[0][0] = xClip * a + yClip * b + tx;
  coords[0][1] = xClip * c + yClip * d + ty;

  coords[2][0] = wClip * a + hClip * b + tx;
  coords[2][1] = wClip * c + hClip * d + ty;

  if (div > 0.) {
    coords[1][0] = wClip * a + yClip * b + tx;
    coords[1][1] = wClip * c + yClip * d + ty;

    coords[3][0] = xClip * a + hClip * b + tx;
    coords[3][1] = xClip * c + hClip * d + ty;
  } else {
    coords[3][0] = wClip * a + yClip * b + tx;
    coords[3][1] = wClip * c + yClip * d + ty;

    coords[1][0] = xClip * a + hClip * b + tx;
    coords[1][1] = xClip * c + hClip * d + ty;
  }

  topIdx = 0;
  for (i = 1; i < 4; i++)
  {
    if (coords[i][1] < coords[topIdx][1])
      topIdx = i;
  }

  dTop = coords[topIdx][1];
  top = (mlib_s32)dTop; bot = -1;

  if (top >= dstHeight) {
    sides[0] = 0;
    sides[1] = -1;
    sides[2] = 1;
    sides[3] = 1;
    return MLIB_SUCCESS;
  }

  if (dTop >= 0.)
  {
    mlib_d64 xLeft, xRight, x;
    mlib_s32 nextIdx;

    if (dTop == top) {
      xLeft = coords[topIdx][0];
      xRight = coords[topIdx][0];
      nextIdx = (topIdx + 1) & 0x3;
      if (dTop == coords[nextIdx][1]) {
        x = coords[nextIdx][0];
        xLeft = (xLeft <= x) ? xLeft : x;
        xRight = (xRight >= x) ? xRight : x;
      }
      nextIdx = (topIdx - 1) & 0x3;
      if (dTop == coords[nextIdx][1]) {
        x = coords[nextIdx][0];
        xLeft = (xLeft <= x) ? xLeft : x;
        xRight = (xRight >= x) ? xRight : x;
      }
      leftEdges[top] = ((t = (mlib_s32)xLeft) >= xLeft) ? t : ++t;
      rightEdges[top] = (mlib_s32)xRight;

    } else top++;
  } else top = 0;

  for (i = 0; i < 2; i++) {
    mlib_d64 dY1 = coords[(topIdx - i) & 0x3][1];
    mlib_d64 dX1 = coords[(topIdx - i) & 0x3][0];
    mlib_d64 dY2 = coords[(topIdx - i - 1) & 0x3][1];
    mlib_d64 dX2 = coords[(topIdx - i - 1) & 0x3][0];
    mlib_d64 x = dX1, slope = (dX2 - dX1) / (dY2 - dY1);
    mlib_s32 y1 = (mlib_s32)dY1;
    mlib_s32 y2 = (mlib_s32)dY2;

    if (dY1 == dY2) continue;

    if (dY1 < 0.) y1 = 0; else y1++;
    if (y2 >= dstHeight) y2 = (mlib_s32)(dstHeight - 1);

    x += slope * (y1 - dY1);
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (j = y1; j <= y2; j++) {
      ((t = (mlib_s32)x) >= x) ? t : ++t;
      x += slope;
      leftEdges[j] = t;
    }
  }

  for (i = 0; i < 2; i++) {
    mlib_d64 dY1 = coords[(topIdx + i) & 0x3][1];
    mlib_d64 dX1 = coords[(topIdx + i) & 0x3][0];
    mlib_d64 dY2 = coords[(topIdx + i + 1) & 0x3][1];
    mlib_d64 dX2 = coords[(topIdx + i + 1) & 0x3][0];
    mlib_d64 x = dX1, slope = (dX2 - dX1) / (dY2 - dY1);
    mlib_s32 y1 = (mlib_s32)dY1;
    mlib_s32 y2 = (mlib_s32)dY2;

    if (dY1 == dY2) continue;
    if (dY1 < 0.) y1 = 0; else y1++;
    if (y2 >= dstHeight) y2 = (mlib_s32)(dstHeight - 1);

    x += slope * (y1 - dY1);
#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (j = y1; j <= y2; j++) {

      t = (mlib_s32)x;
      x += slope;
      rightEdges[j] = t;
    }
    bot = y2;
  }

  {
    mlib_d64 dxCl = xClip * div;
    mlib_d64 dyCl = yClip * div;
    mlib_d64 dwCl = wClip * div;
    mlib_d64 dhCl = hClip * div;

    mlib_s32 xCl = (mlib_s32)(xClip + delta);
    mlib_s32 yCl = (mlib_s32)(yClip + delta);
    mlib_s32 wCl = (mlib_s32)(wClip + delta);
    mlib_s32 hCl = (mlib_s32)(hClip + delta);

    /*
    mlib_s32 xCl = (mlib_s32)(xClip + delta);
    mlib_s32 yCl = (mlib_s32)(yClip + delta);
    mlib_s32 wCl = (mlib_s32)(wClip);
    mlib_s32 hCl = (mlib_s32)(hClip);
    */

    if (edge == MLIB_EDGE_SRC_PADDED) {
      xCl = kw1;
      yCl = kh1;
      wCl = (mlib_s32) (srcWidth  - ((kw - 1) - kw1));
      hCl = (mlib_s32) (srcHeight - ((kh - 1) - kh1));
    }

    div = 1.0/div;

    sdx = (mlib_s32)(a2 * div * (1 << shiftx));
    sdy = (mlib_s32)(c2 * div * (1 << shifty));

    if (div > 0) {

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = top; i <= bot; i++) {
        mlib_s32 xLeft = leftEdges[i];
        mlib_s32 xRight = rightEdges[i];
        mlib_s32 xs, ys, x_e, y_e, x_s, y_s;
        mlib_d64 dxs, dys, dxe, dye;
        mlib_d64 xl, ii, xr;

        xLeft = (xLeft < 0) ? 0 : xLeft;
        xRight = (xRight >= dstWidth) ? (mlib_s32)(dstWidth - 1) : xRight;

        xl = xLeft + 0.5;
        ii = i + 0.5;
        xr = xRight + 0.5;
        dxs = xl * a2 + ii * b2 + tx2;
        dys = xl * c2 + ii * d2 + ty2;
        if ((dxs < dxCl) || (dxs >= dwCl) || (dys < dyCl) || (dys >= dhCl)) {
          dxs += dx;
          dys += dy;
          xLeft++;
          if ((dxs < dxCl) || (dxs >= dwCl) || (dys < dyCl) || (dys >= dhCl)) xRight = -1;
        }
        dxe = xr * a2 + ii * b2 + tx2;
        dye = xr * c2 + ii * d2 + ty2;
        if ((dxe < dxCl) || (dxe >= dwCl) || (dye < dyCl) || (dye >= dhCl)) {
          dxe -= dx;
          dye -= dy;
          xRight--;
          if ((dxe < dxCl) || (dxe >= dwCl) || (dye < dyCl) || (dye >= dhCl)) xRight = -1;
        }

        xs = (mlib_s32)((dxs * div + delta) * (1 << shiftx));
        x_s = xs >> shiftx;

        ys = (mlib_s32)((dys * div + delta) * (1 << shifty));
        y_s = ys >> shifty;

        if (x_s < xCl) xs = (xCl << shiftx);
        else if (x_s >= wCl) xs = ((wCl << shiftx) - 1);

        if (y_s < yCl) ys = (yCl << shifty);
        else if (y_s >= hCl) ys = ((hCl << shifty) - 1);

        if (xRight >= xLeft) {
          x_e = ((xRight - xLeft) * sdx + xs) >> shiftx;
          y_e = ((xRight - xLeft) * sdy + ys) >> shifty;

          if ((x_e < xCl) || (x_e >= wCl)) {
            if (sdx > 0) sdx -= 1;
            else sdx += 1;
          }

          if ((y_e < yCl) || (y_e >= hCl)) {
            if (sdy > 0) sdy -= 1;
            else sdy += 1;
          }
        }

        leftEdges[i] = xLeft;
        rightEdges[i] = xRight;
        xStarts[i] = xs;
        yStarts[i] = ys;

        if ((xRight - xLeft + 1) > max_xsize) max_xsize = (xRight - xLeft + 1);
      }
    } else {

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
      for (i = top; i <= bot; i++) {
        mlib_s32 xLeft = leftEdges[i];
        mlib_s32 xRight = rightEdges[i];
        mlib_s32 xs, ys, x_e, y_e, x_s, y_s;
        mlib_d64 dxs, dys, dxe, dye;
        mlib_d64 xl, ii, xr;

        xLeft = (xLeft < 0) ? 0 : xLeft;
        xRight = (xRight >= dstWidth) ? (mlib_s32)(dstWidth - 1) : xRight;

        xl = xLeft + 0.5;
        ii = i + 0.5;
        xr = xRight + 0.5;
        dxs = xl * a2 + ii * b2 + tx2;
        dys = xl * c2 + ii * d2 + ty2;
        if ((dxs > dxCl) || (dxs <= dwCl) || (dys > dyCl) || (dys <= dhCl)) {
          dxs += dx;
          dys += dy;
          xLeft++;
          if ((dxs > dxCl) || (dxs <= dwCl) || (dys > dyCl) || (dys <= dhCl)) xRight = -1;
        }
        dxe = xr * a2 + ii * b2 + tx2;
        dye = xr * c2 + ii * d2 + ty2;
        if ((dxe > dxCl) || (dxe <= dwCl) || (dye > dyCl) || (dye <= dhCl)) {
          dxe -= dx;
          dye -= dy;
          xRight--;
          if ((dxe > dxCl) || (dxe <= dwCl) || (dye > dyCl) || (dye <= dhCl)) xRight = -1;
        }

        xs = (mlib_s32)((dxs * div + delta) * (1 << shiftx));
        x_s = xs >> shiftx;

        if (x_s < xCl) xs = (xCl << shiftx);
        else if (x_s >= wCl) xs = ((wCl << shiftx) - 1);

        ys = (mlib_s32)((dys * div + delta) * (1 << shifty));
        y_s = ys >> shifty;

        if (y_s < yCl) ys = (yCl << shifty);
        else if (y_s >= hCl) ys = ((hCl << shifty) - 1);

        if (xRight >= xLeft) {
          x_e = ((xRight - xLeft) * sdx + xs) >> shiftx;
          y_e = ((xRight - xLeft) * sdy + ys) >> shifty;

          if ((x_e < xCl) || (x_e >= wCl)) {
            if (sdx > 0) sdx -= 1;
            else sdx += 1;
          }

          if ((y_e < yCl) || (y_e >= hCl)) {
            if (sdy > 0) sdy -= 1;
            else sdy += 1;
          }
        }

        leftEdges[i] = xLeft;
        rightEdges[i] = xRight;
        xStarts[i] = xs;
        yStarts[i] = ys;

        if ((xRight - xLeft + 1) > max_xsize) max_xsize = (xRight - xLeft + 1);
      }
    }
  }

  while (leftEdges[top] > rightEdges[top] && top <= bot) top++;
  if (top < bot) while (leftEdges[bot] > rightEdges[bot]) bot--;

  sides[0] = top;
  sides[1] = bot;
  sides[2] = sdx;
  sides[3] = sdy;
  sides[4] = max_xsize;

  return MLIB_SUCCESS;
}

/***************************************************************/
