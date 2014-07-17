/*
 * @(#)mlib_v_ImageColorTrue2Index.c	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageColorTrue2Index.c	1.45	99/10/18 SMI"

/*
 * FUNCTION
 *      mlib_ImageColorTrue2IndexInit - initialize the colormap structure 
 *      mlib_ImageColorTrue2Index - convert a true color image to an indexed 
 *                                  color image 
 *      mlib_ImageColorTrue2IndexFree - free the colormap structure 
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageColorTrue2IndexInit(void      **colormap,
 *                                                mlib_s32  bits,
 *                                                mlib_type intype,
 *                                                mlib_type outtype,
 *                                                mlib_s32  channels,
 *                                                mlib_s32  entries,
 *                                                mlib_s32  offset,
 *                                                void      **table)
 * 
 *      mlib_status mlib_ImageColorTrue2Index(mlib_image *dst,
 *                                            mlib_image *src,
 *                                            void       *colormap)
 *
 *      void mlib_ImageColorTrue2IndexFree(void *colormap)
 *
 * ARGUMENTS
 *      colormap  Internal data structure for inverse color mapping.
 *      bits      Number of bits per color component used in the colorcube 
 *                of the colormap structure. (If bits == 0, no colorcube will
 *                be created. But the inverse color mapping might be done by
 *                using the original lookup table.)
 *      intype    Input data type of the lookup table and source color image
 *      outtype   Output data type of the destination indexed image
 *      channels  Number of channels of the lookup table.
 *      entries   Number of entries of the lookup table.
 *      offset    The first entry offset of the lookup table.
 *      table     The lookup table (lut).
 *      dst       Pointer to destination image.
 *      src       Pointer to source image.
 *
 * DESCRIPTION
 *      Convert a true color image to a pseudo color image with the method
 *      of finding the nearest matched lut entry for each pixel. 
 *      Or, convert a true color image to a pseudo color image with the 
 *      method of error diffusion dithering or ordered dithering.
 *
 *      The src can be an MLIB_BYTE or MLIB_SHORT image with 3 or 4 channels.
 *      The dst must be a 1-channel MLIB_BYTE or MLIB_SHORT image. 
 *
 *      The lut might have either 3 or 4 channels. The type of the lut can be 
 *      one of the following:
 *              MLIB_BYTE in, MLIB_BYTE out (i.e., BYTE-to-BYTE)
 *              MLIB_SHORT in, MLIB_SHORT out (i.e., SHORT-to-SHORT)
 *              MLIB_SHORT in, MLIB_BYTE out (i.e., SHORT-to-BYTE)
 *
 *      The src image and the lut must have same number of channels.
 */

/***************************************************************/ 

#include "mlib_image.h"
#include "mlib_ImageColormap.h"
#include "vis_proto.h"
#include "mlib_ImageCheck.h"

/***************************************************************/ 

#define LUT_COLOR_CUBE_SEARCH 0
#define LUT_BINARY_TREE_SEARCH 1
#define LUT_STUPID_SEARCH 2

#define FIND_DISTANCE_3( x1, x2, y1, y2, z1, z2, SHIFT ) (      \
  ( ( ( ( x1 ) - ( x2 ) ) * ( ( x1 ) - ( x2 ) ) ) >> SHIFT ) +  \
  ( ( ( ( y1 ) - ( y2 ) ) * ( ( y1 ) - ( y2 ) ) ) >> SHIFT ) +  \
  ( ( ( ( z1 ) - ( z2 ) ) * ( ( z1 ) - ( z2 ) ) ) >> SHIFT ) )

#define FIND_DISTANCE_4( x1, x2, y1, y2, z1, z2, w1, w2, SHIFT ) (      \
  ( ( ( ( x1 ) - ( x2 ) ) * ( ( x1 ) - ( x2 ) ) ) >> SHIFT ) +          \
  ( ( ( ( y1 ) - ( y2 ) ) * ( ( y1 ) - ( y2 ) ) ) >> SHIFT ) +          \
  ( ( ( ( z1 ) - ( z2 ) ) * ( ( z1 ) - ( z2 ) ) ) >> SHIFT ) +          \
  ( ( ( ( w1 ) - ( w2 ) ) * ( ( w1 ) - ( w2 ) ) ) >> SHIFT ) )

/* Bit set in the tag denotes that the corresponding quadrant is a
   palette index, not node. If the bit is clear, this means that that
   is a pointer to the down level node. If the bit is clear and the
   corresponding quadrant is NULL, then there is no way down there and
   this quadrant is clear. */

struct lut_node_3
{
  mlib_u8 tag;

  union
  {
    struct lut_node_3 *quadrants[ 8 ];
    long index[ 8 ];
  } contents;
};

struct lut_node_4
{
  mlib_u16 tag;

  union
  {
    struct lut_node_4 *quadrants[ 16 ];
    long index[ 16 ];
  } contents;
};

/***************************************************************/ 

void *mlib_color_cube_init_U8_3( mlib_colormap *state );
void *mlib_color_cube_init_U8_4( mlib_colormap *state );

void *mlib_color_cube_init_S16_3( mlib_colormap *state );
void *mlib_color_cube_init_S16_4( mlib_colormap *state );

void *mlib_binary_tree_init_3( mlib_colormap *state );
void *mlib_binary_tree_init_4( mlib_colormap *state );

static void mlib_free_binary_tree_3( void *lut );
static void mlib_free_binary_tree_4( void *lut );

/***************************************************************/ 

#define MAIN_COLORTRUE2INDEX_LOOP( FROM_TYPE, TO_TYPE, NCHANNELS )       \
  for( y = 0; y < height; y++ )                                          \
  {                                                                      \
    mlib_ImageColorTrue2IndexLine_##FROM_TYPE##_##TO_TYPE##_##NCHANNELS( \
      sdata, ddata, width, colormap );                                   \
                                                                         \
    sdata += sstride;                                                    \
    ddata += dstride;                                                    \
  }

/***************************************************************/ 

#define COLOR_CUBE_3_INIT_THIRD_LOOP( POINTER_TYPE )            \
{                                                               \
  int detect;                                                   \
  mlib_u32 *dp;                                                 \
  POINTER_TYPE *rgbp;                                           \
  mlib_u32 dist2, xx2;                                          \
  mlib_s32 count, i = cindex;                                   \
  mlib_s32 lim;                                                 \
  mlib_s32 thismin, thismax;                                    \
                                                                \
  if( ret )                                                     \
  {                                                             \
    here2 = center2;                                            \
    min2 = 0;                                                   \
    max2 = colormax - 1;                                        \
    inc2 = cinc2;                                               \
    prevmin2 = colormax;                                        \
    prevmax2 = 0;                                               \
    dmin2 = 0;                                                  \
    dmax2 = 0;                                                  \
  }                                                             \
                                                                \
  detect = 0;                                                   \
  thismin = min2;                                               \
  thismax = max2;                                               \
                                                                \
  /* Third loop up */                                           \
  /* First loop just finds first applicable cell. */            \
  for( count = here2, dist2 = dist1,                            \
    xx2 = inc2, dp = dp1, rgbp = rgbp1, lim = max2;             \
    count <= lim;                                               \
    count++, dp++, rgbp++,                                      \
    dist2 += xx2, xx2 += txsqr )                                \
  {                                                             \
    if( *dp > dist2 )                                           \
    {                                                           \
      if( count > here2 )                                       \
      {                                                         \
        here2 = count;                                          \
        dp1 = dp;                                               \
        rgbp1 = rgbp;                                           \
        dist1 = dist2;                                          \
        inc2 = xx2;                                             \
        thismin = here2;                                        \
      }                                                         \
      detect = 1;                                               \
      break;                                                    \
    }                                                           \
  }                                                             \
                                                                \
  /* Second loop fills in a run of closer cells. */             \
  for( ; count <= lim; count++, dp++, rgbp++, dist2 += xx2,     \
    xx2 += txsqr )                                              \
  {                                                             \
    if( *dp > dist2 )                                           \
    {                                                           \
      *dp = dist2;                                              \
      *rgbp = i + offset;                                       \
    }                                                           \
    else                                                        \
    {                                                           \
      thismax = count - 1;                                      \
      break;                                                    \
    }                                                           \
  }                                                             \
                                                                \
  /* Third loop down */                                         \
  /* Do initializations here, since the 'find' loop             \
   * might not get executed.  */                                \
  lim = min2;                                                   \
  count = here2 - 1;                                            \
  xx2 = inc2 - txsqr;                                           \
  dist2 = dist1 - xx2;                                          \
  dp = dp1 - 1;                                                 \
  rgbp = rgbp1 - 1;                                             \
  /* The 'find' loop is executed only if we didn't already find \
   * something.                                                 \
   */                                                           \
  if( !detect )                                                 \
    for( ; count >= lim; count--, dp--, rgbp--, xx2 -= txsqr,   \
      dist2 -= xx2 )                                            \
    {                                                           \
      if( *dp > dist2 )                                         \
      {                                                         \
        here2 = count;                                          \
        dp1 = dp;                                               \
        rgbp1 = rgbp;                                           \
        dist1 = dist2;                                          \
        inc2 = xx2;                                             \
        thismax = here2;                                        \
        detect = 1;                                             \
        break;                                                  \
      }                                                         \
    }                                                           \
                                                                \
  /* The 'update' loop */                                       \
  for( ; count >= lim; count--, dp--, rgbp--, xx2 -= txsqr,     \
    dist2 -= xx2 )                                              \
  {                                                             \
    if( *dp > dist2 )                                           \
    {                                                           \
      *dp = dist2;                                              \
      *rgbp = i + offset;                                       \
    }                                                           \
    else                                                        \
    {                                                           \
      thismin = count + 1;                                      \
      break;                                                    \
    }                                                           \
  }                                                             \
                                                                \
  /* If we saw something, update the edge trackers. */          \
  if( detect )                                                  \
  {                                                             \
    /* Predictively track.  Not clear this is a win. */         \
    /* If there was a previous line, update the                 \
       dmin/dmax values. */                                     \
    if( prevmax2 >= prevmin2 )                                  \
    {                                                           \
      if( thismin > 0 )                                         \
        dmin2 = thismin - prevmin2 - 1;                         \
      else                                                      \
        dmin2 = 0;                                              \
                                                                \
      if( thismax < colormax - 1 )                              \
        dmax2 = thismax - prevmax2 + 1;                         \
      else                                                      \
        dmax2 = 0;                                              \
                                                                \
      /* Update the min and max values by the differences. */   \
      max2 = thismax + dmax2;                                   \
      if( max2 >= colormax )                                    \
        max2 = colormax - 1;                                    \
                                                                \
      min2 = thismin + dmin2;                                   \
      if( min2 < 0 )                                            \
        min2 = 0;                                               \
    }                                                           \
                                                                \
    prevmax2 = thismax;                                         \
    prevmin2 = thismin;                                         \
  }                                                             \
                                                                \
  ret = detect;                                                 \
}

/***************************************************************/ 

#define COLOR_CUBE_3_INIT_SECOND_LOOP( POINTER_TYPE )           \
{                                                               \
  int detect, first;                                            \
  mlib_s32 count;                                               \
  mlib_s32 thismin, thismax;                                    \
                                                                \
  if( ret )                                                     \
  {                                                             \
    here1 = center1;                                            \
    min1 = 0;                                                   \
    max1 = colormax - 1;                                        \
    inc1 = cinc1;                                               \
    prevmax1 = 0;                                               \
    prevmin1 = colormax;                                        \
  }                                                             \
                                                                \
  thismin = min1;                                               \
  thismax = max1;                                               \
  detect = 0;                                                   \
                                                                \
  for( count = here1, cdist1 = dist1 = dist0,                   \
    xx1 = inc1, cdp1 = dp1 = dp0,                               \
    crgbp1 = rgbp1 = rgbp0, first = 1;                          \
    count <= max1;                                              \
    count++, dp1 += stride1, cdp1 += stride1,                   \
    rgbp1 += stride1, crgbp1 += stride1, dist1 += xx1,          \
    cdist1 += xx1, xx1 += txsqr, first = 0 )                    \
  {                                                             \
    mlib_s32 ret = first;                                       \
                                                                \
    COLOR_CUBE_3_INIT_THIRD_LOOP( POINTER_TYPE );               \
                                                                \
    if( ret )                                                   \
    {                                                           \
      if( !detect )                                             \
      {                                                         \
        if( count > here1 )                                     \
        {                                                       \
          here1 = count;                                        \
          dp0 = cdp1;                                           \
          rgbp0 = crgbp1;                                       \
          dist0 = cdist1;                                       \
          inc1 = xx1;                                           \
          thismin = here1;                                      \
        }                                                       \
        detect = 1;                                             \
      }                                                         \
    }                                                           \
    else if( detect )                                           \
    {                                                           \
      thismax = count - 1;                                      \
      break;                                                    \
    }                                                           \
  }                                                             \
                                                                \
  /* Second loop down */                                        \
  for( count = here1 - 1, xx1 = inc1 - txsqr,                   \
    cdist1 = dist1 = dist0 - xx1,                               \
    cdp1 = dp1 = dp0 - stride1,                                 \
    crgbp1 = rgbp1 = rgbp0 - stride1, first = 1;                \
    count >= min1;                                              \
    count--, dp1 -= stride1, cdp1 -= stride1,                   \
    rgbp1 -= stride1, crgbp1 -= stride1,                        \
    xx1 -= txsqr, dist1 -= xx1, cdist1 -= xx1, first = 0 )      \
  {                                                             \
    mlib_s32 ret = first;                                       \
                                                                \
    COLOR_CUBE_3_INIT_THIRD_LOOP( POINTER_TYPE );               \
                                                                \
    if( ret )                                                   \
    {                                                           \
      if( !detect )                                             \
      {                                                         \
        here1 = count;                                          \
        dp0 = cdp1;                                             \
        rgbp0 = crgbp1;                                         \
        dist0 = cdist1;                                         \
        inc1 = xx1;                                             \
        thismax = here1;                                        \
        detect = 1;                                             \
      }                                                         \
    }                                                           \
    else if( detect )                                           \
    {                                                           \
      thismin = count + 1;                                      \
      break;                                                    \
    }                                                           \
  }                                                             \
                                                                \
  if( detect )                                                  \
  {                                                             \
    if( thismax < prevmax1 )                                    \
      max1 = thismax;                                           \
                                                                \
    prevmax1 = thismax;                                         \
                                                                \
    if( thismin > prevmin1 )                                    \
      min1 = thismin;                                           \
                                                                \
    prevmin1 = thismin;                                         \
  }                                                             \
                                                                \
  ret = detect;                                                 \
}

/***************************************************************/ 

#define COLOR_CUBE_3_INIT_FIRST_LOOP( POINTER_TYPE )    \
{                                                       \
  int detect;                                           \
  mlib_s32 count;                                       \
  int first;                                            \
                                                        \
  detect = 0;                                           \
  /* first loop up */                                   \
  for( count = center0, dist0 = cdist, xx0 = cinc0,     \
    dp0 = cdp, rgbp0 = crgbp, first = 1;                \
    count < colormax;                                   \
    count++, dp0 += stride0, rgbp0 += stride0,          \
    dist0 += xx0, xx0 += txsqr, first = 0 )             \
  {                                                     \
    mlib_s32 ret = first;                               \
                                                        \
    COLOR_CUBE_3_INIT_SECOND_LOOP( POINTER_TYPE );      \
                                                        \
    if( ret )                                           \
      detect = 1;                                       \
    else if( detect )                                   \
      break;                                            \
  }                                                     \
                                                        \
  /* first loop down */                                 \
  for( count = center0 - 1, xx0 = cinc0 - txsqr,        \
    dist0 = cdist - xx0, dp0 = cdp - stride0,           \
    rgbp0 = crgbp - stride0, first = 1;                 \
    count >= 0;                                         \
    count--, dp0 -= stride0, rgbp0 -= stride0,          \
    xx0 -= txsqr, dist0 -= xx0, first = 0 )             \
  {                                                     \
    mlib_s32 ret = first;                               \
                                                        \
    COLOR_CUBE_3_INIT_SECOND_LOOP( POINTER_TYPE );      \
                                                        \
    if( ret )                                           \
      detect = 1;                                       \
    else if( detect )                                   \
      break;                                            \
  }                                                     \
}

/***************************************************************/ 

#define COLOR_CUBE_3_INIT_MAIN_LOOP( POINTER_TYPE, SUBSTRACTION, SHIFT ) \
{                                                                        \
  POINTER_TYPE *rgbp0, *rgbp1, *crgbp, *crgbp1;                          \
                                                                         \
  c0 = ( base0[ 0 ] - SUBSTRACTION ) >> SHIFT;                           \
  c1 = ( base1[ 0 ] - SUBSTRACTION ) >> SHIFT;                           \
  c2 = ( base2[ 0 ] - SUBSTRACTION ) >> SHIFT;                           \
                                                                         \
  dist0 = c0 - x / 2;                                                    \
  dist1 = c1 - x / 2;                                                    \
  cdist = c2 - x / 2;                                                    \
  dist0 = dist0 * dist0 + dist1 * dist1 + cdist * cdist;                 \
                                                                         \
  if( nbits )                                                            \
  {                                                                      \
    cinc0 = 2 * ( xsqr - ( c0 << ( nbits - SHIFT ) ) );                  \
    cinc1 = 2 * ( xsqr - ( c1 << ( nbits - SHIFT ) ) );                  \
    cinc2 = 2 * ( xsqr - ( c2 << ( nbits - SHIFT ) ) );                  \
  }                                                                      \
  else                                                                   \
  {                                                                      \
    cinc0 = x - 2 * c0;                                                  \
    cinc1 = x - 2 * c1;                                                  \
    cinc2 = x - 2 * c2;                                                  \
  }                                                                      \
                                                                         \
  cdp = dist_buf;                                                        \
                                                                         \
  for( c0 = 0, xx0 = cinc0;                                              \
       c0 < colormax;                                                    \
       dist0 += xx0, c0++, xx0 += txsqr )                                \
    for( c1 = 0, dist1 = dist0, xx1 = cinc1;                             \
         c1 < colormax;                                                  \
         dist1 += xx1, c1++, xx1 += txsqr )                              \
      for( c2 = 0, cdist = dist1, xx2 = cinc2;                           \
           c2 < colormax;                                                \
           cdist += xx2, c2++, xx2 += txsqr, cdp++ )                     \
        *cdp = cdist;                                                    \
                                                                         \
  for( cindex = 0; cindex < memsize / 8; cindex++ )                      \
    *( ( ( mlib_d64* )table ) + cindex ) = doublefirst;                  \
                                                                         \
  for( cindex = 1; cindex < state->lutlength; cindex++ )                 \
  {                                                                      \
    c0 = ( base0[ cindex ] - SUBSTRACTION ) >> SHIFT;                    \
    c1 = ( base1[ cindex ] - SUBSTRACTION ) >> SHIFT;                    \
    c2 = ( base2[ cindex ] - SUBSTRACTION ) >> SHIFT;                    \
                                                                         \
    if( nbits )                                                          \
    {                                                                    \
      center0 = c0 >> ( nbits - SHIFT );                                 \
      center1 = c1 >> ( nbits - SHIFT );                                 \
      center2 = c2 >> ( nbits - SHIFT );                                 \
      cinc0 = xsqr - 2 * c0 * x +                                        \
        2 * ( ( center0 << ( nbits - SHIFT ) ) + x / 2 ) * x;            \
      cinc1 = xsqr - 2 * c1 * x +                                        \
        2 * ( ( center1 << ( nbits - SHIFT ) ) + x / 2 ) * x;            \
      cinc2 = xsqr - 2 * c2 * x +                                        \
        2 * ( ( center2 << ( nbits - SHIFT ) ) + x / 2 ) * x;            \
    }                                                                    \
    else                                                                 \
    {                                                                    \
      center0 = c0;                                                      \
      center1 = c1;                                                      \
      center2 = c2;                                                      \
      cinc0 = cinc1 = cinc2 = 1;                                         \
    }                                                                    \
                                                                         \
    dist0 = c0 - ( center0 * x + x / 2 );                                \
    dist1 = c1 - ( center1 * x + x / 2 );                                \
    cdist = c2 - ( center2 * x + x / 2 );                                \
    cdist = dist0 * dist0 + dist1 * dist1 + cdist * cdist;               \
                                                                         \
    cdp = dist_buf + center0 * stride0 +                                 \
      center1 * stride1 + center2;                                       \
    crgbp = table + center0 * stride0 +                                  \
      center1 * stride1 + center2;                                       \
                                                                         \
    COLOR_CUBE_3_INIT_FIRST_LOOP( POINTER_TYPE );                        \
  }                                                                      \
}

/***************************************************************/ 

#define COLOR_CUBE_4_INIT_FOURTH_LOOP( POINTER_TYPE )   \
{                                                       \
  int detect;                                           \
  mlib_u32 *dp;                                         \
  POINTER_TYPE *rgbp;                                   \
  mlib_u32 dist3, xx3;                                  \
  mlib_s32 count, i = cindex;                           \
  mlib_s32 lim;                                         \
  mlib_s32 thismin, thismax;                            \
                                                        \
  if( ret )                                             \
  {                                                     \
    here3 = center3;                                    \
    min3 = 0;                                           \
    max3 = colormax - 1;                                \
    inc3 = cinc3;                                       \
    prevmin3 = colormax;                                \
    prevmax3 = 0;                                       \
    dmin3 = 0;                                          \
    dmax3 = 0;                                          \
  }                                                     \
                                                        \
  detect = 0;                                           \
  thismin = min3;                                       \
  thismax = max3;                                       \
                                                        \
  /* Third loop up */                                   \
  /* First loop just finds first applicable cell. */    \
  for( count = here3, dist3 = dist2,                    \
    xx3 = inc3, dp = dp2, rgbp = rgbp2, lim = max3;     \
    count <= lim;                                       \
    count++, dp++, rgbp++,                              \
    dist3 += xx3, xx3 += txsqr )                        \
  {                                                     \
    if( *dp > dist3 )                                   \
    {                                                   \
      if( count > here3 )                               \
      {                                                 \
        here3 = count;                                  \
        dp2 = dp;                                       \
        rgbp2 = rgbp;                                   \
        dist2 = dist3;                                  \
        inc3 = xx3;                                     \
        thismin = here3;                                \
      }                                                 \
      detect = 1;                                       \
      break;                                            \
    }                                                   \
  }                                                     \
                                                        \
  /* Second loop fills in a run of closer cells. */     \
  for( ; count <= lim; count++, dp++, rgbp++,           \
    dist3 += xx3, xx3 += txsqr )                        \
  {                                                     \
    if( *dp > dist3 )                                   \
    {                                                   \
      *dp = dist3;                                      \
      *rgbp = i + offset;                               \
    }                                                   \
    else                                                \
    {                                                   \
      thismax = count - 1;                              \
      break;                                            \
    }                                                   \
  }                                                     \
                                                        \
  /* Third loop down */                                 \
  /* Do initializations here, since the 'find' loop     \
   * might not get executed.  */                        \
  lim = min3;                                           \
  count = here3 - 1;                                    \
  xx3 = inc3 - txsqr;                                   \
  dist3 = dist2 - xx3;                                  \
  dp = dp2 - 1;                                         \
  rgbp = rgbp2 - 1;                                     \
  /* The 'find' loop is executed only if we             \
   * didn't already find something.  */                 \
  if( !detect )                                         \
    for( ; count >= lim; count--, dp--, rgbp--,         \
      xx3 -= txsqr, dist3 -= xx3 )                      \
    {                                                   \
      if( *dp > dist3 )                                 \
      {                                                 \
        here3 = count;                                  \
        dp2 = dp;                                       \
        rgbp2 = rgbp;                                   \
        dist2 = dist3;                                  \
        inc3 = xx3;                                     \
        thismax = here3;                                \
        detect = 1;                                     \
        break;                                          \
      }                                                 \
    }                                                   \
                                                        \
  /* The 'update' loop */                               \
  for( ; count >= lim; count--, dp--, rgbp--,           \
    xx3 -= txsqr, dist3 -= xx3 )                        \
  {                                                     \
    if( *dp > dist3 )                                   \
    {                                                   \
      *dp = dist3;                                      \
      *rgbp = i + offset;                               \
    }                                                   \
    else                                                \
    {                                                   \
      thismin = count + 1;                              \
      break;                                            \
    }                                                   \
  }                                                     \
                                                        \
  /* If we saw something, update the edge trackers. */  \
  if( detect )                                          \
  {                                                     \
    /* Predictively track.  Not clear this is a win. */ \
    /* If there was a previous line, update the         \
       dmin/dmax values. */                             \
    if( prevmax3 >= prevmin3 )                          \
    {                                                   \
      if( thismin > 0 )                                 \
        dmin3 = thismin - prevmin3 - 1;                 \
      else                                              \
        dmin3 = 0;                                      \
                                                        \
      if( thismax < colormax - 1 )                      \
        dmax3 = thismax - prevmax3 + 1;                 \
      else                                              \
        dmax3 = 0;                                      \
                                                        \
      /* Update the min and max values by the           \
         differences. */                                \
      max3 = thismax + dmax3;                           \
      if( max3 >= colormax )                            \
        max3 = colormax - 1;                            \
                                                        \
      min3 = thismin + dmin3;                           \
      if( min3 < 0 )                                    \
        min3 = 0;                                       \
    }                                                   \
                                                        \
    prevmax3 = thismax;                                 \
    prevmin3 = thismin;                                 \
  }                                                     \
                                                        \
  ret = detect;                                         \
}

/***************************************************************/ 

#define COLOR_CUBE_4_INIT_THIRD_LOOP( POINTER_TYPE )            \
{                                                               \
  int detect, first;                                            \
  mlib_s32 count;                                               \
  mlib_s32 thismin, thismax;                                    \
                                                                \
  if( ret )                                                     \
  {                                                             \
    here2 = center2;                                            \
    min2 = 0;                                                   \
    max2 = colormax - 1;                                        \
    inc2 = cinc2;                                               \
    prevmax2 = 0;                                               \
    prevmin2 = colormax;                                        \
  }                                                             \
                                                                \
  thismin = min2;                                               \
  thismax = max2;                                               \
  detect = 0;                                                   \
                                                                \
  for( count = here2, cdist2 = dist2 = dist1,                   \
    xx2 = inc2, cdp2 = dp2 = dp1,                               \
    crgbp2 = rgbp2 = rgbp1, first = 1;                          \
    count <= max2;                                              \
    count++, dp2 += stride2, cdp2 += stride2,                   \
    rgbp2 += stride2, crgbp2 += stride2, dist2 += xx2,          \
    cdist2 += xx2, xx2 += txsqr, first = 0 )                    \
  {                                                             \
    mlib_s32 ret = first;                                       \
                                                                \
    COLOR_CUBE_4_INIT_FOURTH_LOOP( POINTER_TYPE );              \
                                                                \
    if( ret )                                                   \
    {                                                           \
      if( !detect )                                             \
      {                                                         \
        if( count > here2 )                                     \
        {                                                       \
          here2 = count;                                        \
          dp1 = cdp2;                                           \
          rgbp1 = crgbp2;                                       \
          dist1 = cdist2;                                       \
          inc2 = xx2;                                           \
          thismin = here2;                                      \
        }                                                       \
        detect = 1;                                             \
      }                                                         \
    }                                                           \
    else if( detect )                                           \
    {                                                           \
      thismax = count - 1;                                      \
      break;                                                    \
    }                                                           \
  }                                                             \
                                                                \
  /* Second loop down */                                        \
  for( count = here2 - 1, xx2 = inc2 - txsqr,                   \
    cdist2 = dist2 = dist1 - xx2,                               \
    cdp2 = dp2 = dp1 - stride2,                                 \
    crgbp2 = rgbp2 = rgbp1 - stride2, first = 1;                \
    count >= min2;                                              \
    count--, dp2 -= stride2, cdp2 -= stride2,                   \
    rgbp2 -= stride2, crgbp2 -= stride2,                        \
    xx2 -= txsqr, dist2 -= xx2, cdist2 -= xx2, first = 0 )      \
  {                                                             \
    mlib_s32 ret = first;                                       \
                                                                \
    COLOR_CUBE_4_INIT_FOURTH_LOOP( POINTER_TYPE );              \
                                                                \
    if( ret )                                                   \
    {                                                           \
      if( !detect )                                             \
      {                                                         \
        here2 = count;                                          \
        dp1 = cdp2;                                             \
        rgbp1 = crgbp2;                                         \
        dist1 = cdist2;                                         \
        inc2 = xx2;                                             \
        thismax = here2;                                        \
        detect = 1;                                             \
      }                                                         \
    }                                                           \
    else if( detect )                                           \
    {                                                           \
      thismin = count + 1;                                      \
      break;                                                    \
    }                                                           \
  }                                                             \
                                                                \
  if( detect )                                                  \
  {                                                             \
    if( thismax < prevmax2 )                                    \
      max2 = thismax;                                           \
                                                                \
    prevmax2 = thismax;                                         \
                                                                \
    if( thismin > prevmin2 )                                    \
      min2 = thismin;                                           \
                                                                \
    prevmin2 = thismin;                                         \
  }                                                             \
                                                                \
  ret = detect;                                                 \
}

/***************************************************************/ 

#define COLOR_CUBE_4_INIT_SECOND_LOOP( POINTER_TYPE )           \
{                                                               \
  int detect, first;                                            \
  mlib_s32 count;                                               \
  mlib_s32 thismin, thismax;                                    \
                                                                \
  if( ret )                                                     \
  {                                                             \
    here1 = center1;                                            \
    min1 = 0;                                                   \
    max1 = colormax - 1;                                        \
    inc1 = cinc1;                                               \
    prevmax1 = 0;                                               \
    prevmin1 = colormax;                                        \
  }                                                             \
                                                                \
  thismin = min1;                                               \
  thismax = max1;                                               \
  detect = 0;                                                   \
                                                                \
  for( count = here1, cdist1 = dist1 = dist0,                   \
    xx1 = inc1, cdp1 = dp1 = dp0,                               \
    crgbp1 = rgbp1 = rgbp0, first = 1;                          \
    count <= max1;                                              \
    count++, dp1 += stride1, cdp1 += stride1,                   \
    rgbp1 += stride1, crgbp1 += stride1, dist1 += xx1,          \
    cdist1 += xx1, xx1 += txsqr, first = 0 )                    \
  {                                                             \
    mlib_s32 ret = first;                                       \
                                                                \
    COLOR_CUBE_4_INIT_THIRD_LOOP( POINTER_TYPE );               \
                                                                \
    if( ret )                                                   \
    {                                                           \
      if( !detect )                                             \
      {                                                         \
        if( count > here1 )                                     \
        {                                                       \
          here1 = count;                                        \
          dp0 = cdp1;                                           \
          rgbp0 = crgbp1;                                       \
          dist0 = cdist1;                                       \
          inc1 = xx1;                                           \
          thismin = here1;                                      \
        }                                                       \
        detect = 1;                                             \
      }                                                         \
    }                                                           \
    else if( detect )                                           \
    {                                                           \
      thismax = count - 1;                                      \
      break;                                                    \
    }                                                           \
  }                                                             \
                                                                \
  /* Second loop down */                                        \
  for( count = here1 - 1, xx1 = inc1 - txsqr,                   \
    cdist1 = dist1 = dist0 - xx1,                               \
    cdp1 = dp1 = dp0 - stride1,                                 \
    crgbp1 = rgbp1 = rgbp0 - stride1, first = 1;                \
    count >= min1;                                              \
    count--, dp1 -= stride1, cdp1 -= stride1,                   \
    rgbp1 -= stride1, crgbp1 -= stride1,                        \
    xx1 -= txsqr, dist1 -= xx1, cdist1 -= xx1, first = 0 )      \
  {                                                             \
    mlib_s32 ret = first;                                       \
                                                                \
    COLOR_CUBE_4_INIT_THIRD_LOOP( POINTER_TYPE );               \
                                                                \
    if( ret )                                                   \
    {                                                           \
      if( !detect )                                             \
      {                                                         \
        here1 = count;                                          \
        dp0 = cdp1;                                             \
        rgbp0 = crgbp1;                                         \
        dist0 = cdist1;                                         \
        inc1 = xx1;                                             \
        thismax = here1;                                        \
        detect = 1;                                             \
      }                                                         \
    }                                                           \
    else if( detect )                                           \
    {                                                           \
      thismin = count + 1;                                      \
      break;                                                    \
    }                                                           \
  }                                                             \
                                                                \
  if( detect )                                                  \
  {                                                             \
    if( thismax < prevmax1 )                                    \
      max1 = thismax;                                           \
                                                                \
    prevmax1 = thismax;                                         \
                                                                \
    if( thismin > prevmin1 )                                    \
      min1 = thismin;                                           \
                                                                \
    prevmin1 = thismin;                                         \
  }                                                             \
                                                                \
  ret = detect;                                                 \
}

/***************************************************************/ 

#define COLOR_CUBE_4_INIT_FIRST_LOOP( POINTER_TYPE )    \
{                                                       \
  int detect;                                           \
  mlib_s32 count;                                       \
  int first;                                            \
                                                        \
  detect = 0;                                           \
  /* first loop up */                                   \
  for( count = center0, dist0 = cdist, xx0 = cinc0,     \
    dp0 = cdp, rgbp0 = crgbp, first = 1;                \
    count < colormax;                                   \
    count++, dp0 += stride0, rgbp0 += stride0,          \
    dist0 += xx0, xx0 += txsqr, first = 0 )             \
  {                                                     \
    mlib_s32 ret = first;                               \
                                                        \
    COLOR_CUBE_4_INIT_SECOND_LOOP( POINTER_TYPE );      \
                                                        \
    if( ret )                                           \
      detect = 1;                                       \
    else if( detect )                                   \
      break;                                            \
  }                                                     \
                                                        \
  /* first loop down */                                 \
  for( count = center0 - 1, xx0 = cinc0 - txsqr,        \
    dist0 = cdist - xx0, dp0 = cdp - stride0,           \
    rgbp0 = crgbp - stride0, first = 1;                 \
    count >= 0;                                         \
    count--, dp0 -= stride0, rgbp0 -= stride0,          \
    xx0 -= txsqr, dist0 -= xx0, first = 0 )             \
  {                                                     \
    mlib_s32 ret = first;                               \
                                                        \
    COLOR_CUBE_4_INIT_SECOND_LOOP( POINTER_TYPE );      \
                                                        \
    if( ret )                                           \
      detect = 1;                                       \
    else if( detect )                                   \
      break;                                            \
  }                                                     \
}

/***************************************************************/ 

#define COLOR_CUBE_4_INIT_MAIN_LOOP( POINTER_TYPE, SUBSTRACTION, SHIFT ) \
{                                                                        \
  POINTER_TYPE *rgbp0, *rgbp1, *rgbp2, *crgbp, *crgbp1, *crgbp2;         \
                                                                         \
  c0 = ( base0[ 0 ] - SUBSTRACTION ) >> SHIFT;                           \
  c1 = ( base1[ 0 ] - SUBSTRACTION ) >> SHIFT;                           \
  c2 = ( base2[ 0 ] - SUBSTRACTION ) >> SHIFT;                           \
  c3 = ( base3[ 0 ] - SUBSTRACTION ) >> SHIFT;                           \
                                                                         \
  dist0 = c0 - x / 2;                                                    \
  dist1 = c1 - x / 2;                                                    \
  dist2 = c2 - x / 2;                                                    \
  cdist = c3 - x / 2;                                                    \
  dist0 = dist0 * dist0 + dist1 * dist1 + dist2 * dist2 + cdist * cdist; \
                                                                         \
  if( nbits )                                                            \
  {                                                                      \
    cinc0 = 2 * ( xsqr - ( c0 << ( nbits - SHIFT ) ) );                  \
    cinc1 = 2 * ( xsqr - ( c1 << ( nbits - SHIFT ) ) );                  \
    cinc2 = 2 * ( xsqr - ( c2 << ( nbits - SHIFT ) ) );                  \
    cinc3 = 2 * ( xsqr - ( c3 << ( nbits - SHIFT ) ) );                  \
  }                                                                      \
  else                                                                   \
  {                                                                      \
    cinc0 = x - 2 * c0;                                                  \
    cinc1 = x - 2 * c1;                                                  \
    cinc2 = x - 2 * c2;                                                  \
    cinc3 = x - 2 * c3;                                                  \
  }                                                                      \
                                                                         \
  cdp = dist_buf;                                                        \
  for( c0 = 0, xx0 = cinc0;                                              \
    c0 < colormax;                                                       \
    dist0 += xx0, c0++, xx0 += txsqr )                                   \
    for( c1 = 0, dist1 = dist0, xx1 = cinc1;                             \
      c1 < colormax;                                                     \
      dist1 += xx1, c1++, xx1 += txsqr )                                 \
      for( c2 = 0, dist2 = dist1, xx2 = cinc2;                           \
        c2 < colormax;                                                   \
        dist2 += xx2, c2++, xx2 += txsqr )                               \
        for( c3 = 0, cdist = dist2, xx3 = cinc3;                         \
          c3 < colormax;                                                 \
          cdist += xx3, c3++, xx3 += txsqr, cdp++ )                      \
          *cdp = cdist;                                                  \
                                                                         \
  for( cindex = 0; cindex < memsize / 8; cindex++ )                      \
    *( ( ( mlib_d64* )table ) + cindex ) = doublefirst;                  \
                                                                         \
  for( cindex = 1; cindex < state->lutlength; cindex++ )                 \
  {                                                                      \
    c0 = ( base0[ cindex ] - SUBSTRACTION ) >> SHIFT;                    \
    c1 = ( base1[ cindex ] - SUBSTRACTION ) >> SHIFT;                    \
    c2 = ( base2[ cindex ] - SUBSTRACTION ) >> SHIFT;                    \
    c3 = ( base3[ cindex ] - SUBSTRACTION ) >> SHIFT;                    \
                                                                         \
    if( nbits )                                                          \
    {                                                                    \
      center0 = c0 >> ( nbits - SHIFT );                                 \
      center1 = c1 >> ( nbits - SHIFT );                                 \
      center2 = c2 >> ( nbits - SHIFT );                                 \
      center3 = c3 >> ( nbits - SHIFT );                                 \
      cinc0 = xsqr - 2 * c0 * x +                                        \
        2 * ( ( center0 << ( nbits - SHIFT ) ) + x / 2 ) * x;            \
      cinc1 = xsqr - 2 * c1 * x +                                        \
        2 * ( ( center1 << ( nbits - SHIFT ) ) + x / 2 ) * x;            \
      cinc2 = xsqr - 2 * c2 * x +                                        \
        2 * ( ( center2 << ( nbits - SHIFT ) ) + x / 2 ) * x;            \
      cinc3 = xsqr - 2 * c3 * x +                                        \
        2 * ( ( center3 << ( nbits - SHIFT ) ) + x / 2 ) * x;            \
    }                                                                    \
    else                                                                 \
    {                                                                    \
      center0 = c0;                                                      \
      center1 = c1;                                                      \
      center2 = c2;                                                      \
      center3 = c3;                                                      \
      cinc0 = cinc1 = cinc2 = cinc3 = 1;                                 \
    }                                                                    \
                                                                         \
    dist0 = c0 - ( center0 * x + x / 2 );                                \
    dist1 = c1 - ( center1 * x + x / 2 );                                \
    dist2 = c2 - ( center2 * x + x / 2 );                                \
    cdist = c3 - ( center3 * x + x / 2 );                                \
    cdist = dist0 * dist0 + dist1 * dist1 +                              \
      dist2 * dist2 + cdist * cdist;                                     \
                                                                         \
    cdp = dist_buf + center0 * stride0 + center1 * stride1 +             \
      center2 * stride2 + center3;                                       \
    crgbp = table + center0 * stride0 + center1 * stride1 +              \
      center2 * stride2 + center3;                                       \
                                                                         \
    COLOR_CUBE_4_INIT_FIRST_LOOP( POINTER_TYPE );                        \
  }                                                                      \
}

/***************************************************************/ 

#define COLOR_CUBE_U8_3_SEARCH( TABLE_POINTER_TYPE, SHIFT, STEP )       \
{                                                                       \
  mlib_u8 *c0, *c1, *c2;                                                \
  TABLE_POINTER_TYPE *table = s->table;                                 \
  mlib_s32 bits = s->bits;                                              \
  mlib_s32 nbits = 8 - bits;                                            \
  mlib_s32 mask = ~( ( 1 << nbits ) - 1 );                              \
  mlib_s32 j;                                                           \
                                                                        \
  c0 = src + SHIFT;                                                     \
  c1 = src + 1 + SHIFT;                                                 \
  c2 = src + 2 + SHIFT;                                                 \
                                                                        \
  switch( bits )                                                        \
  {                                                                     \
    case 1:                                                             \
    case 2:                                                             \
    {                                                                   \
      mlib_s32 bits0 = 8 - bits;                                        \
      mlib_s32 bits1 = bits0 - bits;                                    \
      mlib_s32 bits2 = bits1 - bits;                                    \
                                                                        \
      for( j = 0; j < length; j++ )                                     \
      {                                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) >> bits2 ) |                 \
          ( ( *c1 & mask ) >> bits1 ) |                                 \
          ( ( *c2 & mask ) >> bits0 ) ];                                \
                                                                        \
        c0 += STEP;                                                     \
        c1 += STEP;                                                     \
        c2 += STEP;                                                     \
      }                                                                 \
      break;                                                            \
    }                                                                   \
    case 3:                                                             \
    {                                                                   \
      for( j = 0; j < length; j++ )                                     \
      {                                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) << 1 ) |                     \
          ( ( *c1 & mask ) >> 2 ) |                                     \
          ( ( *c2 & mask ) >> 5 ) ];                                    \
                                                                        \
        c0 += STEP;                                                     \
        c1 += STEP;                                                     \
        c2 += STEP;                                                     \
      }                                                                 \
      break;                                                            \
    }                                                                   \
    case 4:                                                             \
    {                                                                   \
      for( j = 0; j < length; j++ )                                     \
      {                                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) << 4 ) |                     \
          ( *c1 & mask ) |                                              \
          ( ( *c2 & mask ) >> 4 ) ];                                    \
                                                                        \
        c0 += STEP;                                                     \
        c1 += STEP;                                                     \
        c2 += STEP;                                                     \
      }                                                                 \
      break;                                                            \
    }                                                                   \
    case 5:                                                             \
    case 6:                                                             \
    case 7:                                                             \
    {                                                                   \
      mlib_s32 bits0 = 8 - bits;                                        \
      mlib_s32 bits1 = bits * 2 - 8;                                    \
      mlib_s32 bits2 = bits1 + bits;                                    \
                                                                        \
      for( j = 0; j < length; j++ )                                     \
      {                                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) << bits2 ) |                 \
          ( ( *c1 & mask ) << bits1 ) |                                 \
          ( ( *c2 & mask ) >> bits0 ) ];                                \
                                                                        \
        c0 += STEP;                                                     \
        c1 += STEP;                                                     \
        c2 += STEP;                                                     \
      }                                                                 \
      break;                                                            \
    }                                                                   \
    case 8:                                                             \
    {                                                                   \
      for( j = 0; j < length; j++ )                                     \
      {                                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) << 16 ) |                    \
          ( ( *c1 & mask ) << 8 ) |                                     \
          ( *c2 & mask ) ];                                             \
                                                                        \
        c0 += STEP;                                                     \
        c1 += STEP;                                                     \
        c2 += STEP;                                                     \
      }                                                                 \
      break;                                                            \
    }                                                                   \
  }                                                                     \
}

/***************************************************************/ 

#define COLOR_CUBE_U8_4_SEARCH( TABLE_TYPE )            \
{                                                       \
  mlib_u8 *c0, *c1, *c2, *c3;                           \
  TABLE_TYPE *table = s->table;                         \
  mlib_s32 bits = s->bits;                              \
  mlib_s32 nbits = 8 - bits;                            \
  mlib_s32 mask = ~( ( 1 << nbits ) - 1 );              \
  mlib_s32 j;                                           \
                                                        \
  c0 = src;                                             \
  c1 = src + 1;                                         \
  c2 = src + 2;                                         \
  c3 = src + 3;                                         \
                                                        \
  switch( bits )                                        \
  {                                                     \
    case 1:                                             \
    {                                                   \
      for( j = 0; j < length; j++ )                     \
      {                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) >> 4 ) |     \
          ( ( *c1 & mask ) >> 5 ) |                     \
          ( ( *c2 & mask ) >> 6 ) |                     \
          ( ( *c3 & mask ) >> 7 ) ];                    \
                                                        \
        c0 += 4;                                        \
        c1 += 4;                                        \
        c2 += 4;                                        \
        c3 += 4;                                        \
      }                                                 \
      break;                                            \
    }                                                   \
    case 2:                                             \
    {                                                   \
      for( j = 0; j < length; j++ )                     \
      {                                                 \
        dst[ j ] = table[ ( *c0 & mask ) |              \
          ( ( *c1 & mask ) >> 2 ) |                     \
          ( ( *c2 & mask ) >> 4 ) |                     \
          ( ( *c3 & mask ) >> 6 ) ];                    \
                                                        \
        c0 += 4;                                        \
        c1 += 4;                                        \
        c2 += 4;                                        \
        c3 += 4;                                        \
          }                                             \
      break;                                            \
    }                                                   \
    case 3:                                             \
    {                                                   \
      for( j = 0; j < length; j++ )                     \
      {                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) << 4 ) |     \
          ( ( *c1 & mask ) << 1 ) |                     \
          ( ( *c2 & mask ) >> 2 ) |                     \
          ( ( *c3 & mask ) >> 5 ) ];                    \
                                                        \
        c0 += 4;                                        \
        c1 += 4;                                        \
        c2 += 4;                                        \
        c3 += 4;                                        \
      }                                                 \
      break;                                            \
    }                                                   \
    case 4:                                             \
    {                                                   \
      for( j = 0; j < length; j++ )                     \
      {                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) << 8 ) |     \
          ( ( *c1 & mask ) << 4 ) |                     \
          ( *c2 & mask ) |                              \
          ( ( *c3 & mask ) >> 4 ) ];                    \
                                                        \
        c0 += 4;                                        \
        c1 += 4;                                        \
        c2 += 4;                                        \
        c3 += 4;                                        \
      }                                                 \
      break;                                            \
    }                                                   \
    case 5:                                             \
    case 6:                                             \
    {                                                   \
      mlib_s32 bits3 = bits * 4 - 8;                    \
      mlib_s32 bits2 = bits3 - bits;                    \
      mlib_s32 bits1 = bits2 - bits;                    \
      mlib_s32 bits0 = 8 - bits;                        \
                                                        \
      for( j = 0; j < length; j++ )                     \
      {                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) << bits3 ) | \
          ( ( *c1 & mask ) << bits2 ) |                 \
          ( ( *c2 & mask ) << bits1 ) |                 \
          ( ( *c3 & mask ) >> bits0 ) ];                \
                                                        \
        c0 += 4;                                        \
        c1 += 4;                                        \
        c2 += 4;                                        \
        c3 += 4;                                        \
      }                                                 \
      break;                                            \
    }                                                   \
    case 7:                                             \
    {                                                   \
      for( j = 0; j < length; j++ )                     \
      {                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) << 20 ) |    \
          ( ( *c1 & mask ) << 13 ) |                    \
          ( ( *c2 & mask ) << 6 ) |                     \
          ( ( *c3 & mask ) >> 1 ) ];                    \
                                                        \
        c0 += 4;                                        \
        c1 += 4;                                        \
        c2 += 4;                                        \
        c3 += 4;                                        \
      }                                                 \
      break;                                            \
    }                                                   \
    case 8: /* will never be called */                  \
    {                                                   \
      for( j = 0; j < length; j++ )                     \
      {                                                 \
        dst[ j ] = table[ ( ( *c0 & mask ) << 24 ) |    \
          ( ( *c1 & mask ) << 16 ) |                    \
          ( ( *c2 & mask ) << 8 ) |                     \
          ( *c3 & mask ) ];                             \
                                                        \
        c0 += 4;                                        \
        c1 += 4;                                        \
        c2 += 4;                                        \
        c3 += 4;                                        \
      }                                                 \
      break;                                            \
    }                                                   \
  }                                                     \
}

/***************************************************************/ 

#define COLOR_CUBE_S16_3_SEARCH( TABLE_TYPE, SHIFT, STEP )                 \
{                                                                          \
  mlib_s16 *c0, *c1, *c2;                                                  \
  mlib_s32 bits = s->bits;                                                 \
  mlib_s32 nbits = 16 - bits;                                              \
  mlib_s32 mask = ~( ( 1 << nbits ) - 1 );                                 \
  mlib_s32 stride = 1 << bits;                                             \
  TABLE_TYPE *table = s->table;                                            \
  mlib_s32 j;                                                              \
                                                                           \
  c0 = src + SHIFT;                                                        \
  c1 = src + 1 + SHIFT;                                                    \
  c2 = src + 2 + SHIFT;                                                    \
                                                                           \
  switch( bits )                                                           \
  {                                                                        \
    case 1:                                                                \
    case 2:                                                                \
    case 3:                                                                \
    case 4:                                                                \
    case 5:                                                                \
    {                                                                      \
      mlib_s32 bits0 = 16 - bits;                                          \
      mlib_s32 bits1 = bits0 - bits;                                       \
      mlib_s32 bits2 = bits1 - bits;                                       \
                                                                           \
      for( j = 0; j < length; j++ )                                        \
      {                                                                    \
        dst[ j ] = table[ ( ( ( *c0 - MLIB_S16_MIN ) & mask ) >> bits2 ) | \
          ( ( ( *c1 - MLIB_S16_MIN ) & mask ) >> bits1 ) |                 \
          ( ( ( *c2 - MLIB_S16_MIN ) & mask ) >> bits0 ) ];                \
                                                                           \
        c0 += STEP;                                                        \
        c1 += STEP;                                                        \
        c2 += STEP;                                                        \
      }                                                                    \
      break;                                                               \
    }                                                                      \
    case 6:                                                                \
    case 7:                                                                \
    {                                                                      \
      mlib_s32 bits0 = 16 - bits;                                          \
      mlib_s32 bits1 = bits0 - bits;                                       \
      mlib_s32 bits2 = bits * 3 - 16;                                      \
                                                                           \
      for( j = 0; j < length; j++ )                                        \
      {                                                                    \
        dst[ j ] = table[ ( ( ( *c0 - MLIB_S16_MIN ) & mask ) << bits2 ) | \
          ( ( ( *c1 - MLIB_S16_MIN ) & mask ) >> bits1 ) |                 \
          ( ( ( *c2 - MLIB_S16_MIN ) & mask ) >> bits0 ) ];                \
                                                                           \
        c0 += STEP;                                                        \
        c1 += STEP;                                                        \
        c2 += STEP;                                                        \
      }                                                                    \
      break;                                                               \
    }                                                                      \
    case 8:                                                                \
    {                                                                      \
      for( j = 0; j < length; j++ )                                        \
      {                                                                    \
        dst[ j ] = table[ ( ( ( *c0 - MLIB_S16_MIN ) & mask ) << 8 ) |     \
          ( ( *c1 - MLIB_S16_MIN ) & mask ) |                              \
          ( ( ( *c2 - MLIB_S16_MIN ) & mask ) >> 8 ) ];                    \
                                                                           \
        c0 += STEP;                                                        \
        c1 += STEP;                                                        \
        c2 += STEP;                                                        \
      }                                                                    \
      break;                                                               \
    }                                                                      \
    case 9:                                                                \
    case 10:                                                               \
    {                                                                      \
      mlib_s32 bits0 = 16 - bits;                                          \
      mlib_s32 bits1 = 2 * bits - 16;                                      \
      mlib_s32 bits2 = bits1 + bits;                                       \
                                                                           \
      for( j = 0; j < length; j++ )                                        \
      {                                                                    \
        dst[ j ] = table[ ( ( ( *c0 - MLIB_S16_MIN ) & mask ) << bits2 ) | \
          ( ( ( *c1 - MLIB_S16_MIN ) & mask ) << bits1 ) |                 \
          ( ( ( *c2 - MLIB_S16_MIN ) & mask ) >> bits0 ) ];                \
                                                                           \
        c0 += STEP;                                                        \
        c1 += STEP;                                                        \
        c2 += STEP;                                                        \
      }                                                                    \
      break;                                                               \
    }                                                                      \
    /* Other cases may not be considered as the table size will be more    \
       than 2^32 */                                                        \
  }                                                                        \
}

#define COLOR_CUBE_S16_4_SEARCH( TABLE_TYPE )                              \
{                                                                          \
  mlib_s16 *c0, *c1, *c2, *c3;                                             \
  TABLE_TYPE *table = s->table;                                            \
  mlib_s32 bits = s->bits;                                                 \
  mlib_s32 nbits = 16 - bits;                                              \
  mlib_s32 mask = ~( ( 1 << nbits ) - 1 );                                 \
  mlib_s32 j;                                                              \
                                                                           \
  c0 = src;                                                                \
  c1 = src + 1;                                                            \
  c2 = src + 2;                                                            \
  c3 = src + 3;                                                            \
                                                                           \
  switch( bits )                                                           \
  {                                                                        \
    case 1:                                                                \
    case 2:                                                                \
    case 3:                                                                \
    {                                                                      \
      mlib_s32 bits0 = 16 - bits;                                          \
      mlib_s32 bits1 = bits0 - bits;                                       \
      mlib_s32 bits2 = bits1 - bits;                                       \
      mlib_s32 bits3 = bits2 - bits;                                       \
                                                                           \
      for( j = 0; j < length; j++ )                                        \
      {                                                                    \
        dst[ j ] = table[ ( ( ( *c0 - MLIB_S16_MIN ) & mask ) >> bits3 ) | \
          ( ( ( *c1 - MLIB_S16_MIN ) & mask ) >> bits2 ) |                 \
          ( ( ( *c2 - MLIB_S16_MIN ) & mask ) >> bits1 ) |                 \
          ( ( ( *c3 - MLIB_S16_MIN ) & mask ) >> bits0 ) ];                \
                                                                           \
        c0 += 4;                                                           \
        c1 += 4;                                                           \
        c2 += 4;                                                           \
        c3 += 4;                                                           \
      }                                                                    \
      break;                                                               \
    }                                                                      \
    case 4:                                                                \
    {                                                                      \
      for( j = 0; j < length; j++ )                                        \
      {                                                                    \
        dst[ j ] = table[ ( ( *c0 - MLIB_S16_MIN ) & mask ) |              \
          ( ( ( *c1 - MLIB_S16_MIN ) & mask ) >> 4 ) |                     \
          ( ( ( *c2 - MLIB_S16_MIN ) & mask ) >> 8 ) |                     \
          ( ( ( *c3 - MLIB_S16_MIN ) & mask ) >> 12 ) ];                   \
                                                                           \
        c0 += 4;                                                           \
        c1 += 4;                                                           \
        c2 += 4;                                                           \
        c3 += 4;                                                           \
      }                                                                    \
      break;                                                               \
    }                                                                      \
    case 5:                                                                \
    {                                                                      \
      for( j = 0; j < length; j++ )                                        \
      {                                                                    \
        dst[ j ] = table[ ( ( ( *c0 - MLIB_S16_MIN ) & mask ) << 4 ) |     \
          ( ( ( *c1 - MLIB_S16_MIN ) & mask ) >> 1 ) |                     \
          ( ( ( *c2 - MLIB_S16_MIN ) & mask ) >> 6 ) |                     \
          ( ( ( *c3 - MLIB_S16_MIN ) & mask ) >> 11 ) ];                   \
                                                                           \
        c0 += 4;                                                           \
        c1 += 4;                                                           \
        c2 += 4;                                                           \
        c3 += 4;                                                           \
      }                                                                    \
      break;                                                               \
    }                                                                      \
    case 6:                                                                \
    case 7:                                                                \
    {                                                                      \
      mlib_s32 bits0 = 16 - bits;                                          \
      mlib_s32 bits1 = bits0 - bits;                                       \
      mlib_s32 bits3 = bits * 4 - 16;                                      \
      mlib_s32 bits2 = bits3 - bits;                                       \
                                                                           \
      for( j = 0; j < length; j++ )                                        \
      {                                                                    \
        dst[ j ] = table[ ( ( ( *c0 - MLIB_S16_MIN ) & mask ) << bits3 ) | \
          ( ( ( *c1 - MLIB_S16_MIN ) & mask ) << bits2 ) |                 \
          ( ( ( *c2 - MLIB_S16_MIN ) & mask ) >> bits1 ) |                 \
          ( ( ( *c3 - MLIB_S16_MIN ) & mask ) >> bits0 ) ];                \
                                                                           \
        c0 += 4;                                                           \
        c1 += 4;                                                           \
        c2 += 4;                                                           \
        c3 += 4;                                                           \
      }                                                                    \
      break;                                                               \
    }                                                                      \
    case 8:                                                                \
    {                                                                      \
      for( j = 0; j < length; j++ )                                        \
      {                                                                    \
        dst[ j ] = table[ ( ( ( *c0 - MLIB_S16_MIN ) & mask ) << 16 ) |    \
          ( ( ( *c1 - MLIB_S16_MIN ) & mask ) << 8 ) |                     \
          ( ( *c2 - MLIB_S16_MIN ) & mask ) |                              \
          ( ( ( *c3 - MLIB_S16_MIN ) & mask ) >> 8 ) ];                    \
                                                                           \
        c0 += 4;                                                           \
        c1 += 4;                                                           \
        c2 += 4;                                                           \
        c3 += 4;                                                           \
      }                                                                    \
      break;                                                               \
    }                                                                      \
    /* Other cases may not be considered as the table size will be more    \
       than 2^32 */                                                        \
  }                                                                        \
}

/***************************************************************/ 

#define BINARY_TREE_SEARCH_RIGHT( POSITION, COLOR_MAX, SHIFT )  \
{                                                               \
  if( ( distance >= ( ( ( position[ POSITION ] + current_size - \
    c[ POSITION ] ) * ( position[ POSITION ] + current_size -   \
    c[ POSITION ] ) ) >> SHIFT ) ) &&                           \
    ( position[ POSITION ] + current_size != COLOR_MAX ) )      \
    continue_up = 1;                                            \
}

/***************************************************************/ 

#define BINARY_TREE_EXPLORE_RIGHT_3( POSITION, COLOR_MAX, IMAGE_TYPE,    \
  FIRST_NEIBOUR, SECOND_NEIBOUR, SUBSTRACTION, SHIFT )                   \
{                                                                        \
  if( distance >= ( ( ( position[ POSITION ] + current_size -            \
    c[ POSITION ] ) * ( position[ POSITION ] +                           \
      current_size - c[ POSITION ] ) ) >> SHIFT ) )                      \
  {                                                                      \
    if( distance < ( ( ( COLOR_MAX - c[ POSITION ] ) *                   \
      ( COLOR_MAX - c[ POSITION ] ) ) >> SHIFT ) )                       \
    {                                                                    \
      if( distance < ( ( ( position[ POSITION ] +                        \
        current_size * 2 - c[ POSITION ] ) *                             \
        ( position[ POSITION ] + current_size * 2 -                      \
          c[ POSITION ] ) ) >> SHIFT ) )                                 \
      {                                                                  \
        /* Check only a part of quadrant */                              \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 1;                            \
        check_neibours[ SECOND_NEIBOUR ] += 1;                           \
        check_corner += 1;                                               \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_3( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Only a part of quadrant needs checking */                   \
          distance =                                                     \
            mlib_search_quadrant_part_to_left_##IMAGE_TYPE##_3(          \
              node->contents.quadrants[ qq ],                            \
              distance, &found_color, c, p,                              \
              position[ POSITION ] + current_size, pass - 1, POSITION ); \
      }                                                                  \
      else /* Check whole quadrant */                                    \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 2;                            \
        check_neibours[ SECOND_NEIBOUR ] += 2;                           \
        check_corner += 2;                                               \
        continue_up = 1;                                                 \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_3( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Here is a full node. Just explore it */                     \
          distance = mlib_search_quadrant_##IMAGE_TYPE##_3(              \
            node->contents.quadrants[ qq ],                              \
            distance, &found_color, c[ 0 ], c[ 1 ], c[ 2 ], p );         \
      }                                                                  \
    }                                                                    \
    else /* Cell is on the edge of the space */                          \
    {                                                                    \
      if( position[ POSITION ] + current_size * 2 ==                     \
        COLOR_MAX )                                                      \
      {                                                                  \
        /* Check only a part of quadrant */                              \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 1;                            \
        check_neibours[ SECOND_NEIBOUR ] += 1;                           \
        check_corner += 1;                                               \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_3( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Only a part of quadrant needs checking */                   \
          distance =                                                     \
            mlib_search_quadrant_part_to_left_##IMAGE_TYPE##_3(          \
              node->contents.quadrants[ qq ],                            \
              distance, &found_color, c, p,                              \
              position[ POSITION ] + current_size,                       \
              pass - 1, POSITION );                                      \
      }                                                                  \
      else /* Check whole quadrant */                                    \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 2;                            \
        check_neibours[ SECOND_NEIBOUR ] += 2;                           \
        check_corner += 2;                                               \
        continue_up = 1;                                                 \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_3( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Here is a full node. Just explore it */                     \
          distance = mlib_search_quadrant_##IMAGE_TYPE##_3(              \
            node->contents.quadrants[ qq ],                              \
            distance, &found_color, c[ 0 ], c[ 1 ], c[ 2 ], p );         \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}

/***************************************************************/ 

#define BINARY_TREE_EXPLORE_RIGHT_4( POSITION, COLOR_MAX, IMAGE_TYPE,    \
  FIRST_NEIBOUR, SECOND_NEIBOUR, THIRD_NEIBOUR, SUBSTRACTION, SHIFT )    \
{                                                                        \
  if( distance >= ( ( ( position[ POSITION ] + current_size -            \
    c[ POSITION ] ) * ( position[ POSITION ] +                           \
      current_size - c[ POSITION ] ) ) >> SHIFT ) )                      \
  {                                                                      \
    if( distance < ( ( ( COLOR_MAX - c[ POSITION ] ) *                   \
      ( COLOR_MAX - c[ POSITION ] ) ) >> SHIFT ) )                       \
    {                                                                    \
      if( distance < ( ( ( position[ POSITION ] +                        \
        current_size * 2 - c[ POSITION ] ) *                             \
        ( position[ POSITION ] + current_size * 2 -                      \
          c[ POSITION ] ) ) >> SHIFT ) )                                 \
      {                                                                  \
        /* Check only a part of quadrant */                              \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 1;                            \
        check_neibours[ SECOND_NEIBOUR ] += 1;                           \
        check_neibours[ THIRD_NEIBOUR ] += 1;                            \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_4( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, c[ 3 ],            \
            p[ 3 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Only a part of quadrant needs checking */                   \
          distance =                                                     \
            mlib_search_quadrant_part_to_left_##IMAGE_TYPE##_4(          \
              node->contents.quadrants[ qq ],                            \
              distance, &found_color, c, p,                              \
              position[ POSITION ] + current_size, pass - 1, POSITION ); \
      }                                                                  \
      else /* Check whole quadrant */                                    \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 2;                            \
        check_neibours[ SECOND_NEIBOUR ] += 2;                           \
        check_neibours[ THIRD_NEIBOUR ] += 2;                            \
        continue_up = 1;                                                 \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_4( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, c[ 3 ],            \
            p[ 3 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Here is a full node. Just explore it */                     \
          distance = mlib_search_quadrant_##IMAGE_TYPE##_4(              \
            node->contents.quadrants[ qq ],                              \
            distance, &found_color, c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], p ); \
      }                                                                  \
    }                                                                    \
    else /* Cell is on the edge of the space */                          \
    {                                                                    \
      if( position[ POSITION ] + current_size * 2 ==                     \
        COLOR_MAX )                                                      \
      {                                                                  \
        /* Check only a part of quadrant */                              \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 1;                            \
        check_neibours[ SECOND_NEIBOUR ] += 1;                           \
        check_neibours[ THIRD_NEIBOUR ] += 1;                            \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_4( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, c[ 3 ],            \
            p[ 3 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Only a part of quadrant needs checking */                   \
          distance =                                                     \
            mlib_search_quadrant_part_to_left_##IMAGE_TYPE##_4(          \
              node->contents.quadrants[ qq ],                            \
              distance, &found_color, c, p,                              \
              position[ POSITION ] + current_size,                       \
              pass - 1, POSITION );                                      \
      }                                                                  \
      else /* Check whole quadrant */                                    \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 2;                            \
        check_neibours[ SECOND_NEIBOUR ] += 2;                           \
        check_neibours[ THIRD_NEIBOUR ] += 2;                            \
        continue_up = 1;                                                 \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_4( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, c[ 3 ],            \
            p[ 3 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Here is a full node. Just explore it */                     \
          distance = mlib_search_quadrant_##IMAGE_TYPE##_4(              \
            node->contents.quadrants[ qq ],                              \
            distance, &found_color, c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], p ); \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}

/***************************************************************/ 

#define BINARY_TREE_SEARCH_LEFT( POSITION, SHIFT )                      \
{                                                                       \
  if( ( distance > ( ( ( position[ POSITION ] - c[ POSITION ] ) *       \
    ( position[ POSITION ] - c[ POSITION ] ) ) >> SHIFT ) )  &&         \
    position[ POSITION ] )                                              \
    continue_up = 1;                                                    \
}

/***************************************************************/ 

#define BINARY_TREE_EXPLORE_LEFT_3( POSITION, IMAGE_TYPE,                \
  FIRST_NEIBOUR, SECOND_NEIBOUR, SUBSTRACTION, SHIFT )                   \
{                                                                        \
  if( distance >                                                         \
    ( ( ( c[ POSITION ] - position[ POSITION ] ) *                       \
    ( c[ POSITION ] - position[ POSITION ] ) ) >> SHIFT ) )              \
  {                                                                      \
    if( distance <= ( ( c[ POSITION ] * c[ POSITION ] ) >> SHIFT ) )     \
    {                                                                    \
      if( distance <= ( ( ( c[ POSITION ] + current_size -               \
        position[ POSITION ] ) *                                         \
        ( c[ POSITION ] + current_size -                                 \
          position[ POSITION ] ) ) >> SHIFT ) )                          \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 1;                            \
        check_neibours[ SECOND_NEIBOUR ] += 1;                           \
        check_corner += 1;                                               \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_3( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Only a part of quadrant needs checking */                   \
          distance =                                                     \
            mlib_search_quadrant_part_to_right_##IMAGE_TYPE##_3(         \
              node->contents.quadrants[ qq ],                            \
              distance, &found_color, c, p,                              \
              position[ POSITION ] - current_size, pass - 1, POSITION ); \
      }                                                                  \
      else /* Check whole quadrant */                                    \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 2;                            \
        check_neibours[ SECOND_NEIBOUR ] += 2;                           \
        check_corner += 2;                                               \
        continue_up = 1;                                                 \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_3( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Here is a full node. Just explore it */                     \
          distance = mlib_search_quadrant_##IMAGE_TYPE##_3(              \
            node->contents.quadrants[ qq ],                              \
            distance, &found_color, c[ 0 ], c[ 1 ], c[ 2 ], p );         \
      }                                                                  \
    }                                                                    \
    else                                                                 \
    {                                                                    \
      if( !( position[ POSITION ] - current_size ) )                     \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 1;                            \
        check_neibours[ SECOND_NEIBOUR ] += 1;                           \
        check_corner += 1;                                               \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_3( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Only a part of quadrant needs checking */                   \
          distance =                                                     \
            mlib_search_quadrant_part_to_right_##IMAGE_TYPE##_3(         \
              node->contents.quadrants[ qq ],                            \
              distance, &found_color, c, p,                              \
              position[ POSITION ] - current_size, pass - 1, POSITION ); \
      }                                                                  \
      else                                                               \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 2;                            \
        check_neibours[ SECOND_NEIBOUR ] += 2;                           \
        check_corner += 2;                                               \
        continue_up = 1;                                                 \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_3( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Here is a full node. Just explore it */                     \
          distance = mlib_search_quadrant_##IMAGE_TYPE##_3(              \
            node->contents.quadrants[ qq ],                              \
            distance, &found_color, c[ 0 ], c[ 1 ], c[ 2 ], p );         \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}

/***************************************************************/ 

#define BINARY_TREE_EXPLORE_LEFT_4( POSITION, IMAGE_TYPE,                \
  FIRST_NEIBOUR, SECOND_NEIBOUR, THIRD_NEIBOUR, SUBSTRACTION, SHIFT )    \
{                                                                        \
  if( distance >                                                         \
    ( ( ( c[ POSITION ] - position[ POSITION ] ) *                       \
    ( c[ POSITION ] - position[ POSITION ] ) ) >> SHIFT ) )              \
  {                                                                      \
    if( distance <= ( ( c[ POSITION ] * c[ POSITION ] ) >> SHIFT ) )     \
    {                                                                    \
      if( distance <= ( ( ( c[ POSITION ] + current_size -               \
        position[ POSITION ] ) *                                         \
        ( c[ POSITION ] + current_size -                                 \
          position[ POSITION ] ) ) >> SHIFT ) )                          \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 1;                            \
        check_neibours[ SECOND_NEIBOUR ] += 1;                           \
        check_neibours[ THIRD_NEIBOUR ] += 1;                            \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_4( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, c[ 3 ],            \
            p[ 3 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Only a part of quadrant needs checking */                   \
          distance =                                                     \
            mlib_search_quadrant_part_to_right_##IMAGE_TYPE##_4(         \
              node->contents.quadrants[ qq ],                            \
              distance, &found_color, c, p,                              \
              position[ POSITION ] - current_size, pass - 1, POSITION ); \
      }                                                                  \
      else /* Check whole quadrant */                                    \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 2;                            \
        check_neibours[ SECOND_NEIBOUR ] += 2;                           \
        check_neibours[ THIRD_NEIBOUR ] += 2;                            \
        continue_up = 1;                                                 \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_4( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, c[ 3 ],            \
            p[ 3 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Here is a full node. Just explore it */                     \
          distance = mlib_search_quadrant_##IMAGE_TYPE##_4(              \
            node->contents.quadrants[ qq ],                              \
            distance, &found_color, c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], p ); \
      }                                                                  \
    }                                                                    \
    else                                                                 \
    {                                                                    \
      if( !( position[ POSITION ] - current_size ) )                     \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 1;                            \
        check_neibours[ SECOND_NEIBOUR ] += 1;                           \
        check_neibours[ THIRD_NEIBOUR ] += 1;                            \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_4( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, c[ 3 ],            \
            p[ 3 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Only a part of quadrant needs checking */                   \
          distance =                                                     \
            mlib_search_quadrant_part_to_right_##IMAGE_TYPE##_4(         \
              node->contents.quadrants[ qq ],                            \
              distance, &found_color, c, p,                              \
              position[ POSITION ] - current_size, pass - 1, POSITION ); \
      }                                                                  \
      else                                                               \
      {                                                                  \
        mlib_s32 qq = q ^ ( 1 << POSITION );                             \
                                                                         \
        check_neibours[ FIRST_NEIBOUR ] += 2;                            \
        check_neibours[ SECOND_NEIBOUR ] += 2;                           \
        check_neibours[ THIRD_NEIBOUR ] += 2;                            \
        continue_up = 1;                                                 \
        if( node->tag & ( 1 << qq ) )                                    \
        {                                                                \
          /* Here is another color cell.                                 \
             Check the distance */                                       \
          mlib_s32 new_found_color =                                     \
            node->contents.index[ qq ];                                  \
          mlib_u32 newdistance = FIND_DISTANCE_4( c[ 0 ],                \
            p[ 0 ][ new_found_color ] - SUBSTRACTION, c[ 1 ],            \
            p[ 1 ][ new_found_color ] - SUBSTRACTION, c[ 2 ],            \
            p[ 2 ][ new_found_color ] - SUBSTRACTION, c[ 3 ],            \
            p[ 3 ][ new_found_color ] - SUBSTRACTION, SHIFT );           \
                                                                         \
          if( newdistance < distance )                                   \
          {                                                              \
            found_color = new_found_color;                               \
            distance = newdistance;                                      \
          }                                                              \
        }                                                                \
        else if( node->contents.quadrants[ qq ] )                        \
          /* Here is a full node. Just explore it */                     \
          distance = mlib_search_quadrant_##IMAGE_TYPE##_4(              \
            node->contents.quadrants[ qq ],                              \
            distance, &found_color, c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], p ); \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}

/***************************************************************/ 

#define BINARY_TREE_INIT_3( POINTER_TYPE, BITS, SUBTRACTION, SHIFT )      \
{                                                                         \
  POINTER_TYPE *base[ 3 ];                                                \
                                                                          \
  base[ 0 ] = state->lut[ 0 ];                                            \
  base[ 1 ] = state->lut[ 1 ];                                            \
  base[ 2 ] = state->lut[ 2 ];                                            \
                                                                          \
  table = mlib_malloc( sizeof( struct lut_node_3 ) );                     \
  if( !table )                                                            \
    return NULL;                                                          \
                                                                          \
  table->tag = 0;                                                         \
  table->contents.quadrants[ 0 ] = 0;                                     \
  table->contents.quadrants[ 1 ] = 0;                                     \
  table->contents.quadrants[ 2 ] = 0;                                     \
  table->contents.quadrants[ 3 ] = 0;                                     \
  table->contents.quadrants[ 4 ] = 0;                                     \
  table->contents.quadrants[ 5 ] = 0;                                     \
  table->contents.quadrants[ 6 ] = 0;                                     \
  table->contents.quadrants[ 7 ] = 0;                                     \
                                                                          \
  for( i = 0; i < state->lutlength; i++ )                                 \
  {                                                                       \
    mlib_s32 q;                                                           \
    mlib_s32 finish = 0;                                                  \
    mlib_s32 pass = BITS - 1;                                             \
    mlib_u32 c0, c1, c2;                                                  \
                                                                          \
    node = table;                                                         \
    c0 = base[ 0 ][ i ] - SUBTRACTION;                                    \
    c1 = base[ 1 ][ i ] - SUBTRACTION;                                    \
    c2 = base[ 2 ][ i ] - SUBTRACTION;                                    \
                                                                          \
    do                                                                    \
    {                                                                     \
      q = ( ( c0 >> pass ) & 1 ) |                                        \
        ( ( ( c1 << 1 ) >> pass ) & 2 ) |                                 \
        ( ( ( c2 << 2 ) >> pass ) & 4 );                                  \
                                                                          \
      if( node->tag & ( 1 << q ) )                                        \
      {                                                                   \
        /* change this cell to a node */                                  \
        mlib_s32 identical;                                               \
        mlib_s32 index = node->contents.index[ q ];                       \
        mlib_u32 palc0, palc1, palc2;                                     \
                                                                          \
        palc0 = base[ 0 ][ index ] - SUBTRACTION;                         \
        palc1 = base[ 1 ][ index ] - SUBTRACTION;                         \
        palc2 = base[ 2 ][ index ] - SUBTRACTION;                         \
                                                                          \
        identical = ( palc0 - c0 ) |                                      \
          ( palc1 - c1 ) |                                                \
          ( palc2 - c2 );                                                 \
                                                                          \
        if( !identical )                                                  \
        {                                                                 \
          /* Nodes are the same. Do we care ? Nope */                     \
          /* This index will be lost, so we leave */                      \
          finish = 1;                                                     \
        }                                                                 \
        else                                                              \
        {                                                                 \
          mlib_s32 still_in_the_same_cell = 1;                            \
          mlib_s32 oldq;                                                  \
                                                                          \
          if( BITS - pass >= bits )                                       \
          {                                                               \
            /* We found a color already on the bottom of the              \
               tree. Now we need to find out, what color is better        \
               here */                                                    \
            mlib_s32 mask = ~( ( 1 << pass ) - 1 );                       \
            mlib_u32 olddistance = FIND_DISTANCE_3(                       \
              palc0, palc0 & mask,                                        \
              palc1, palc1 & mask,                                        \
              palc2, palc2 & mask, SHIFT );                               \
            mlib_u32 newdistance = FIND_DISTANCE_3(                       \
              c0, c0 & mask,                                              \
              c1, c1 & mask,                                              \
              c2, c2 & mask, SHIFT );                                     \
                                                                          \
            if( newdistance < olddistance )                               \
            {                                                             \
              /* Change color index */                                    \
              node->contents.index[ q ] = i;                              \
            }                                                             \
            finish = 1;                                                   \
          }                                                               \
          else                                                            \
          {                                                               \
            do                                                            \
            {                                                             \
              mlib_s32 prevq = q;                                         \
                                                                          \
              pass--;                                                     \
              /*                                                          \
                split this cell to 2 quadrants if we are lucky. If        \
                not, do this several times untill we become lucky. It     \
                may not happen infinitelly since points are different     \
              */                                                          \
                                                                          \
              q = ( ( c0 >> pass ) & 1 ) |                                \
                ( ( ( c1 << 1 ) >> pass ) & 2 ) |                         \
                ( ( ( c2 << 2 ) >> pass ) & 4 );                          \
                                                                          \
              oldq = ( ( palc0 >> pass ) & 1 ) |                          \
                ( ( ( palc1 << 1 ) >> pass ) & 2 ) |                      \
                ( ( ( palc2 << 2 ) >> pass ) & 4 );                       \
                                                                          \
              if( oldq != q || BITS - pass >= bits )                      \
              {                                                           \
                /*                                                        \
                  we have found the place where we place both points in   \
                  different cells. That is what we were looking for.      \
                  I knew we are lucky :)                                  \
                */                                                        \
                struct lut_node_3 *new_node;                              \
                                                                          \
                new_node = mlib_malloc( sizeof( struct lut_node_3 ) );    \
                if( !new_node )                                           \
                {                                                         \
                  mlib_free_binary_tree_3( table );                       \
                  return NULL;                                            \
                }                                                         \
                                                                          \
                /* Set the tag mask for these new cells */                \
                new_node->tag = ( 1 << q ) | ( 1 << oldq );               \
                                                                          \
                new_node->contents.index[ 0 ] = 0;                        \
                new_node->contents.index[ 1 ] = 0;                        \
                new_node->contents.index[ 2 ] = 0;                        \
                new_node->contents.index[ 3 ] = 0;                        \
                new_node->contents.index[ 4 ] = 0;                        \
                new_node->contents.index[ 5 ] = 0;                        \
                new_node->contents.index[ 6 ] = 0;                        \
                new_node->contents.index[ 7 ] = 0;                        \
                                                                          \
                if( q != oldq )                                           \
                {                                                         \
                  new_node->contents.index[ q ] = i;                      \
                  new_node->contents.index[ oldq ] = index;               \
                }                                                         \
                else                                                      \
                {                                                         \
                  mlib_s32 mask = ~( ( 1 << pass ) - 1 );                 \
                  mlib_u32 olddistance = FIND_DISTANCE_3(                 \
                    palc0, palc0 & mask,                                  \
                    palc1, palc1 & mask,                                  \
                    palc2, palc2 & mask, SHIFT );                         \
                  mlib_u32 newdistance = FIND_DISTANCE_3(                 \
                    c0, c0 & mask,                                        \
                    c1, c1 & mask,                                        \
                    c2, c2 & mask, SHIFT );                               \
                                                                          \
                  /* Change color index */                                \
                  if( newdistance < olddistance )                         \
                    new_node->contents.index[ q ] = i;                    \
                  else                                                    \
                    new_node->contents.index[ q ] = index;                \
                }                                                         \
                                                                          \
                /* Here is not a color cell now. Here is a pointer */     \
                node->tag &= ~( 1 << prevq );                             \
                node->contents.quadrants[ prevq ] = new_node;             \
                                                                          \
                still_in_the_same_cell = 0;                               \
                finish = 1;                                               \
              }                                                           \
              else                                                        \
              {                                                           \
                /*                                                        \
                  we have to split this cell and continue splitting then, \
                  because points again in the same cell                   \
                */                                                        \
                struct lut_node_3 *new_node;                              \
                                                                          \
                new_node = mlib_malloc( sizeof( struct lut_node_3 ) );    \
                if( !new_node )                                           \
                {                                                         \
                  mlib_free_binary_tree_3( table );                       \
                  return NULL;                                            \
                }                                                         \
                                                                          \
                /* copy the information about colors down the tree */     \
                new_node->tag = 1 << q;                                   \
                                                                          \
                new_node->contents.index[ 0 ] = 0;                        \
                new_node->contents.index[ 1 ] = 0;                        \
                new_node->contents.index[ 2 ] = 0;                        \
                new_node->contents.index[ 3 ] = 0;                        \
                new_node->contents.index[ 4 ] = 0;                        \
                new_node->contents.index[ 5 ] = 0;                        \
                new_node->contents.index[ 6 ] = 0;                        \
                new_node->contents.index[ 7 ] = 0;                        \
                                                                          \
                new_node->contents.index[ q ] = index;                    \
                                                                          \
                /* attach the new node */                                 \
                /* Here is not a color cell now. Here is a pointer */     \
                node->tag &= ~( 1 << prevq );                             \
                node->contents.quadrants[ prevq ] = new_node;             \
                node = new_node;                                          \
                /* and continue */                                        \
              }                                                           \
            } while ( still_in_the_same_cell );                           \
          }                                                               \
        }                                                                 \
      }                                                                   \
      else                                                                \
      {                                                                   \
        if( node->contents.quadrants[ q ] )                               \
        {                                                                 \
          /* Down we go */                                                \
          node = node->contents.quadrants[ q ];                           \
        }                                                                 \
        else                                                              \
        {                                                                 \
          /* this quadrant is yet empty */                                \
          /* let's fill it with new color */                              \
          node->tag |= 1 << q;                                            \
          node->contents.index[ q ] = i;                                  \
                                                                          \
          /* and LEAVE ! */                                               \
          finish = 1;                                                     \
        }                                                                 \
      }                                                                   \
                                                                          \
      pass--;                                                             \
                                                                          \
    } while( !finish );                                                   \
  }                                                                       \
}

/***************************************************************/ 

#define CHECK_QUADRANT_U8_3( qq )                               \
{                                                               \
  if( node->tag & ( 1 << qq ) )                                 \
  {                                                             \
    /* Here is another color cell. Check the distance */        \
    mlib_s32 new_found_color = node->contents.index[ qq ];      \
    mlib_u32 newdistance = FIND_DISTANCE_3( c[ 0 ],             \
      p[ 0 ][ new_found_color ], c[ 1 ],                        \
      p[ 1 ][ new_found_color ], c[ 2 ],                        \
      p[ 2 ][ new_found_color ], 0 );                           \
                                                                \
    if( newdistance < distance )                                \
    {                                                           \
      found_color = new_found_color;                            \
      distance = newdistance;                                   \
    }                                                           \
  }                                                             \
  else if( node->contents.quadrants[ qq ] )                     \
    /* Here is a full node. Just explore it all */              \
    distance = mlib_search_quadrant_U8_3(                       \
      node->contents.quadrants[ qq ], distance, &found_color,   \
      c[ 0 ], c[ 1 ], c[ 2 ], p );                              \
/* Else there is just an empty cell */                          \
}

/***************************************************************/ 

#define CHECK_QUADRANT_S16_3( qq )                              \
{                                                               \
  if( node->tag & ( 1 << qq ) )                                 \
  {                                                             \
    /* Here is another color cell. Check the distance */        \
    mlib_s32 new_found_color = node->contents.index[ qq ];      \
    mlib_u32 palc0, palc1, palc2, newdistance;                  \
                                                                \
    palc0 = p[ 0 ][ new_found_color ] - MLIB_S16_MIN;           \
    palc1 = p[ 1 ][ new_found_color ] - MLIB_S16_MIN;           \
    palc2 = p[ 2 ][ new_found_color ] - MLIB_S16_MIN;           \
                                                                \
    newdistance = FIND_DISTANCE_3( c[ 0 ], palc0,               \
      c[ 1 ], palc1,                                            \
      c[ 2 ], palc2, 2 );                                       \
                                                                \
    if( newdistance < distance )                                \
    {                                                           \
      found_color = new_found_color;                            \
      distance = newdistance;                                   \
    }                                                           \
  }                                                             \
  else if( node->contents.quadrants[ qq ] )                     \
    /* Here is a full node. Just explore it all */              \
    distance = mlib_search_quadrant_S16_3(                      \
      node->contents.quadrants[ qq ], distance, &found_color,   \
      c[ 0 ], c[ 1 ], c[ 2 ], p );                              \
/* Else there is just an empty cell */                          \
}

/***************************************************************/ 

#define BINARY_TREE_SEARCH_3( SOURCE_IMAGE, POINTER_TYPE, BITS,              \
  COLOR_MAX, SUBTRACTION, POINTER_SHIFT, STEP, SHIFT )                       \
{                                                                            \
  POINTER_TYPE *channels[ 3 ], *p[ 3 ];                                      \
  mlib_u32 c[ 3 ];                                                           \
  mlib_s32 j;                                                                \
  mlib_s32 chann = s->channels;                                              \
                                                                             \
  p[ 0 ] = s->lut[ 0 ];                                                      \
  p[ 1 ] = s->lut[ 1 ];                                                      \
  p[ 2 ] = s->lut[ 2 ];                                                      \
  channels[ 0 ] = src + POINTER_SHIFT;                                       \
  channels[ 1 ] = src + 1 + POINTER_SHIFT;                                   \
  channels[ 2 ] = src + 2 + POINTER_SHIFT;                                   \
                                                                             \
  for( j = 0; j < length; j++ )                                              \
  {                                                                          \
    mlib_s32 pass = BITS - 1;                                                \
    mlib_u32 position[ 3 ] = { 0, 0, 0 };                                    \
    mlib_s32 we_found_it = 0;                                                \
    struct lut_node_3 *node = s->table;                                      \
    /* Stack pointer pointers to the first free element of stack. */         \
    /* The node we are in is in the `node' */                                \
    struct                                                                   \
    {                                                                        \
      struct lut_node_3 *node;                                               \
      mlib_s32 q;                                                            \
    } stack[ BITS ];                                                         \
    mlib_s32 stack_pointer = 0;                                              \
                                                                             \
    c[ 0 ] = *channels[ 0 ] - SUBTRACTION;                                   \
    c[ 1 ] = *channels[ 1 ] - SUBTRACTION;                                   \
    c[ 2 ] = *channels[ 2 ] - SUBTRACTION;                                   \
                                                                             \
    do                                                                       \
    {                                                                        \
      mlib_s32 q;                                                            \
      mlib_u32 current_size = 1 << pass;                                     \
                                                                             \
      q = ( ( c[ 0 ] >> pass ) & 1 ) |                                       \
        ( ( ( c[ 1 ] << 1 ) >> pass ) & 2 ) |                                \
        ( ( ( c[ 2 ] << 2 ) >> pass ) & 4 );                                 \
                                                                             \
      position[ 0 ] |= c[ 0 ] & current_size;                                \
      position[ 1 ] |= c[ 1 ] & current_size;                                \
      position[ 2 ] |= c[ 2 ] & current_size;                                \
                                                                             \
      if( node->tag & ( 1 << q ) )                                           \
      {                                                                      \
        /*                                                                   \
          Here is a cell with one color. We need to be sure it's             \
          the one that is the closest to our color                           \
        */                                                                   \
        mlib_s32 palindex = node->contents.index[ q ];                       \
        mlib_u32 palc[ 3 ];                                                  \
        mlib_s32 identical;                                                  \
                                                                             \
        palc[ 0 ] = p[ 0 ][ palindex ] - SUBTRACTION;                        \
        palc[ 1 ] = p[ 1 ][ palindex ] - SUBTRACTION;                        \
        palc[ 2 ] = p[ 2 ][ palindex ] - SUBTRACTION;                        \
                                                                             \
        identical = ( palc[ 0 ] - c[ 0 ] ) | ( palc[ 1 ] - c[ 1 ] ) |        \
          ( palc[ 2 ] - c[ 2 ] );                                            \
                                                                             \
        if( !identical || BITS - pass == bits )                              \
        {                                                                    \
          /* Oh, here it is :) */                                            \
          dst[ j ] = palindex + s->offset;                                   \
          we_found_it = 1;                                                   \
        }                                                                    \
        else                                                                 \
        {                                                                    \
          mlib_u32 distance;                                                 \
          /* First index is the channel, second is the number of the         \
             side */                                                         \
          mlib_s32 found_color;                                              \
          mlib_s32 continue_up;                                              \
                                                                             \
          distance = FIND_DISTANCE_3( c[ 0 ], palc[ 0 ],                     \
            c[ 1 ], palc[ 1 ], c[ 2 ], palc[ 2 ], SHIFT );                   \
          found_color = palindex;                                            \
                                                                             \
          do                                                                 \
          {                                                                  \
            mlib_s32 check_corner;                                           \
                                                                             \
            /*                                                               \
              Neibours are enumerated in a cicle:                            \
              0 - between quadrants 0 and 1,                                 \
              1 - between quadrants 1 and 2 and                              \
              2 - between quadrants 2 and 0                                  \
            */                                                               \
            mlib_s32 check_neibours[ 3 ];                                    \
                                                                             \
            /*                                                               \
              Others are three two neibour quadrants                         \
                                                                             \
              Side number is [ <number of the coordinate >][ <the bit        \
              in the quadrant number of the corner, corresponding to         \
              this coordinate> ], e.g. 2 is 0..010b, so the sides it has     \
              near are:                                                      \
              [ 0 (coordinate number) ][ 0 (bit 0 in the number) ]           \
              [ 1 (coordinate number) ][ 1 (bit 1 in the number) ]           \
                                                                             \
              Now we can look in the three nearest quadrants. Do             \
              we really need it ? Check it.                                  \
            */                                                               \
                                                                             \
            check_corner = check_neibours[ 0 ] = check_neibours[ 1 ] =       \
              check_neibours[ 2 ] = 0;                                       \
            continue_up = 0;                                                 \
                                                                             \
            if( q & 1 )                                                      \
            {                                                                \
              BINARY_TREE_EXPLORE_LEFT_3( 0, SOURCE_IMAGE, 2, 0,             \
                SUBTRACTION, SHIFT );                                        \
            }                                                                \
            else                                                             \
            {                                                                \
              BINARY_TREE_EXPLORE_RIGHT_3( 0, COLOR_MAX, SOURCE_IMAGE, 2, 0, \
                SUBTRACTION, SHIFT );                                        \
            }                                                                \
                                                                             \
            if( q & 2 )                                                      \
            {                                                                \
              BINARY_TREE_EXPLORE_LEFT_3( 1, SOURCE_IMAGE, 0, 1,             \
                SUBTRACTION, SHIFT );                                        \
            }                                                                \
            else                                                             \
            {                                                                \
              BINARY_TREE_EXPLORE_RIGHT_3( 1, COLOR_MAX, SOURCE_IMAGE, 0, 1, \
                SUBTRACTION, SHIFT );                                        \
            }                                                                \
                                                                             \
            if( q & 4 )                                                      \
            {                                                                \
              BINARY_TREE_EXPLORE_LEFT_3( 2, SOURCE_IMAGE, 1, 2,             \
                SUBTRACTION, SHIFT );                                        \
            }                                                                \
            else                                                             \
            {                                                                \
              BINARY_TREE_EXPLORE_RIGHT_3( 2, COLOR_MAX, SOURCE_IMAGE, 1, 2, \
                SUBTRACTION, SHIFT );                                        \
            }                                                                \
                                                                             \
            if( check_neibours[ 0 ] >= 2 )                                   \
            {                                                                \
              mlib_s32 qq = q ^ 3;                                           \
              CHECK_QUADRANT_##SOURCE_IMAGE##_3( qq );                       \
            }                                                                \
                                                                             \
            if( check_neibours[ 1 ] >= 2 )                                   \
            {                                                                \
              mlib_s32 qq = q ^ 6;                                           \
              CHECK_QUADRANT_##SOURCE_IMAGE##_3( qq );                       \
            }                                                                \
                                                                             \
            if( check_neibours[ 2 ] >= 2 )                                   \
            {                                                                \
              mlib_s32 qq = q ^ 5;                                           \
              CHECK_QUADRANT_##SOURCE_IMAGE##_3( qq );                       \
            }                                                                \
                                                                             \
            if( check_corner >= 3 )                                          \
            {                                                                \
              mlib_s32 qq = q ^ 7;                                           \
              CHECK_QUADRANT_##SOURCE_IMAGE##_3( qq );                       \
            }                                                                \
                                                                             \
            if( q & 1 )                                                      \
            {                                                                \
              BINARY_TREE_SEARCH_RIGHT( 0, COLOR_MAX, SHIFT );               \
            }                                                                \
            else                                                             \
            {                                                                \
              BINARY_TREE_SEARCH_LEFT( 0, SHIFT );                           \
            }                                                                \
                                                                             \
            if( q & 2 )                                                      \
            {                                                                \
              BINARY_TREE_SEARCH_RIGHT( 1, COLOR_MAX, SHIFT );               \
            }                                                                \
            else                                                             \
            {                                                                \
              BINARY_TREE_SEARCH_LEFT( 1, SHIFT );                           \
            }                                                                \
                                                                             \
            if( q & 4 )                                                      \
            {                                                                \
              BINARY_TREE_SEARCH_RIGHT( 2, COLOR_MAX, SHIFT );               \
            }                                                                \
            else                                                             \
            {                                                                \
              BINARY_TREE_SEARCH_LEFT( 2, SHIFT );                           \
            }                                                                \
                                                                             \
            position[ 0 ] &= ~( c[ 0 ] & current_size );                     \
            position[ 1 ] &= ~( c[ 1 ] & current_size );                     \
            position[ 2 ] &= ~( c[ 2 ] & current_size );                     \
                                                                             \
            current_size <<= 1;                                              \
                                                                             \
            pass++;                                                          \
                                                                             \
            stack_pointer--;                                                 \
            q = stack[ stack_pointer ].q;                                    \
            node = stack[ stack_pointer ].node;                              \
          } while( continue_up );                                            \
                                                                             \
          dst[ j ] = found_color + s->offset;                                \
                                                                             \
          we_found_it = 1;                                                   \
        }                                                                    \
      }                                                                      \
      else if( node->contents.quadrants[ q ] )                               \
      {                                                                      \
        /* Descend one level */                                              \
        stack[ stack_pointer ].node = node;                                  \
        stack[ stack_pointer++ ].q = q;                                      \
        node = node->contents.quadrants[ q ];                                \
      }                                                                      \
      else                                                                   \
      {                                                                      \
        /* Found the empty quadrant. Look around */                          \
        mlib_u32 distance = MLIB_U32_MAX;                                    \
        mlib_s32 found_color;                                                \
        mlib_s32 continue_up;                                                \
                                                                             \
        /*                                                                   \
          As we had come to this level, it is warranted that there           \
          are other points on this level near the empty quadrant             \
        */                                                                   \
        do                                                                   \
        {                                                                    \
          mlib_s32 check_corner;                                             \
          mlib_s32 check_neibours[ 3 ];                                      \
                                                                             \
          check_corner = check_neibours[ 0 ] = check_neibours[ 1 ] =         \
            check_neibours[ 2 ] = 0;                                         \
          continue_up = 0;                                                   \
                                                                             \
          if( q & 1 )                                                        \
          {                                                                  \
            BINARY_TREE_EXPLORE_LEFT_3( 0, SOURCE_IMAGE, 2, 0,               \
              SUBTRACTION, SHIFT );                                          \
          }                                                                  \
          else                                                               \
          {                                                                  \
            BINARY_TREE_EXPLORE_RIGHT_3( 0, COLOR_MAX, SOURCE_IMAGE, 2, 0,   \
              SUBTRACTION, SHIFT );                                          \
          }                                                                  \
                                                                             \
          if( q & 2 )                                                        \
          {                                                                  \
            BINARY_TREE_EXPLORE_LEFT_3( 1, SOURCE_IMAGE, 0, 1,               \
              SUBTRACTION, SHIFT );                                          \
          }                                                                  \
          else                                                               \
          {                                                                  \
            BINARY_TREE_EXPLORE_RIGHT_3( 1, COLOR_MAX, SOURCE_IMAGE, 0, 1,   \
              SUBTRACTION, SHIFT );                                          \
          }                                                                  \
                                                                             \
          if( q & 4 )                                                        \
          {                                                                  \
            BINARY_TREE_EXPLORE_LEFT_3( 2, SOURCE_IMAGE, 1, 2,               \
              SUBTRACTION, SHIFT );                                          \
          }                                                                  \
          else                                                               \
          {                                                                  \
            BINARY_TREE_EXPLORE_RIGHT_3( 2, COLOR_MAX, SOURCE_IMAGE, 1, 2,   \
              SUBTRACTION, SHIFT );                                          \
          }                                                                  \
                                                                             \
          if( check_neibours[ 0 ] >= 2 )                                     \
          {                                                                  \
            mlib_s32 qq = q ^ 3;                                             \
            CHECK_QUADRANT_##SOURCE_IMAGE##_3( qq );                         \
          }                                                                  \
                                                                             \
          if( check_neibours[ 1 ] >= 2 )                                     \
          {                                                                  \
            mlib_s32 qq = q ^ 6;                                             \
            CHECK_QUADRANT_##SOURCE_IMAGE##_3( qq );                         \
          }                                                                  \
                                                                             \
          if( check_neibours[ 2 ] >= 2 )                                     \
          {                                                                  \
            mlib_s32 qq = q ^ 5;                                             \
            CHECK_QUADRANT_##SOURCE_IMAGE##_3( qq );                         \
          }                                                                  \
                                                                             \
          if( check_corner >= 3 )                                            \
          {                                                                  \
            mlib_s32 qq = q ^ 7;                                             \
            CHECK_QUADRANT_##SOURCE_IMAGE##_3( qq );                         \
          }                                                                  \
                                                                             \
          if( q & 1 )                                                        \
          {                                                                  \
            BINARY_TREE_SEARCH_RIGHT( 0, COLOR_MAX, SHIFT );                 \
          }                                                                  \
          else                                                               \
          {                                                                  \
            BINARY_TREE_SEARCH_LEFT( 0, SHIFT );                             \
          }                                                                  \
                                                                             \
          if( q & 2 )                                                        \
          {                                                                  \
            BINARY_TREE_SEARCH_RIGHT( 1, COLOR_MAX, SHIFT );                 \
          }                                                                  \
          else                                                               \
          {                                                                  \
            BINARY_TREE_SEARCH_LEFT( 1, SHIFT );                             \
          }                                                                  \
                                                                             \
          if( q & 4 )                                                        \
          {                                                                  \
            BINARY_TREE_SEARCH_RIGHT( 2, COLOR_MAX, SHIFT );                 \
          }                                                                  \
          else                                                               \
          {                                                                  \
            BINARY_TREE_SEARCH_LEFT( 2, SHIFT );                             \
          }                                                                  \
                                                                             \
          position[ 0 ] &= ~( c[ 0 ] & current_size );                       \
          position[ 1 ] &= ~( c[ 1 ] & current_size );                       \
          position[ 2 ] &= ~( c[ 2 ] & current_size );                       \
                                                                             \
          current_size <<= 1;                                                \
                                                                             \
          pass++;                                                            \
                                                                             \
          stack_pointer--;                                                   \
          q = stack[ stack_pointer ].q;                                      \
          node = stack[ stack_pointer ].node;                                \
        } while( continue_up );                                              \
                                                                             \
        dst[ j ] = found_color + s->offset;                                  \
        we_found_it = 1;                                                     \
      }                                                                      \
                                                                             \
      pass--;                                                                \
                                                                             \
    } while( !we_found_it );                                                 \
                                                                             \
    channels[ 0 ] += STEP;                                                   \
    channels[ 1 ] += STEP;                                                   \
    channels[ 2 ] += STEP;                                                   \
  }                                                                          \
}

/***************************************************************/ 

#define BINARY_TREE_INIT_4( POINTER_TYPE, BITS, SUBTRACTION, SHIFT )      \
{                                                                         \
  POINTER_TYPE *base[ 4 ];                                                \
                                                                          \
  base[ 0 ] = state->lut[ 0 ];                                            \
  base[ 1 ] = state->lut[ 1 ];                                            \
  base[ 2 ] = state->lut[ 2 ];                                            \
  base[ 3 ] = state->lut[ 3 ];                                            \
                                                                          \
  table = mlib_malloc( sizeof( struct lut_node_4 ) );                     \
  if( !table )                                                            \
    return NULL;                                                          \
                                                                          \
  table->tag = 0;                                                         \
  table->contents.quadrants[ 0 ] = 0;                                     \
  table->contents.quadrants[ 1 ] = 0;                                     \
  table->contents.quadrants[ 2 ] = 0;                                     \
  table->contents.quadrants[ 3 ] = 0;                                     \
  table->contents.quadrants[ 4 ] = 0;                                     \
  table->contents.quadrants[ 5 ] = 0;                                     \
  table->contents.quadrants[ 6 ] = 0;                                     \
  table->contents.quadrants[ 7 ] = 0;                                     \
  table->contents.quadrants[ 8 ] = 0;                                     \
  table->contents.quadrants[ 9 ] = 0;                                     \
  table->contents.quadrants[ 10 ] = 0;                                    \
  table->contents.quadrants[ 11 ] = 0;                                    \
  table->contents.quadrants[ 12 ] = 0;                                    \
  table->contents.quadrants[ 13 ] = 0;                                    \
  table->contents.quadrants[ 14 ] = 0;                                    \
  table->contents.quadrants[ 15 ] = 0;                                    \
                                                                          \
  for( i = 0; i < state->lutlength; i++ )                                 \
  {                                                                       \
    mlib_s32 q;                                                           \
    mlib_s32 finish = 0;                                                  \
    mlib_s32 pass = BITS - 1;                                             \
    mlib_u32 c0, c1, c2, c3;                                              \
                                                                          \
    node = table;                                                         \
    c0 = base[ 0 ][ i ] - SUBTRACTION;                                    \
    c1 = base[ 1 ][ i ] - SUBTRACTION;                                    \
    c2 = base[ 2 ][ i ] - SUBTRACTION;                                    \
    c3 = base[ 3 ][ i ] - SUBTRACTION;                                    \
                                                                          \
    do                                                                    \
    {                                                                     \
      q = ( ( c0 >> pass ) & 1 ) |                                        \
        ( ( ( c1 << 1 ) >> pass ) & 2 ) |                                 \
        ( ( ( c2 << 2 ) >> pass ) & 4 ) |                                 \
        ( ( ( c3 << 3 ) >> pass ) & 8 );                                  \
                                                                          \
      if( node->tag & ( 1 << q ) )                                        \
      {                                                                   \
        /* change this cell to a node */                                  \
        mlib_s32 identical;                                               \
        mlib_s32 index = node->contents.index[ q ];                       \
        mlib_u32 palc0, palc1, palc2, palc3;                              \
                                                                          \
        palc0 = ( base[ 0 ][ index ] - SUBTRACTION );                     \
        palc1 = ( base[ 1 ][ index ] - SUBTRACTION );                     \
        palc2 = ( base[ 2 ][ index ] - SUBTRACTION );                     \
        palc3 = ( base[ 3 ][ index ] - SUBTRACTION );                     \
                                                                          \
        identical = ( palc0 - c0 ) |                                      \
          ( palc1 - c1 ) |                                                \
          ( palc2 - c2 ) |                                                \
          ( palc3 - c3 );                                                 \
                                                                          \
        if( !identical )                                                  \
        {                                                                 \
          /* Nodes are the same. Do we care ? Nope */                     \
          /* This index will be lost, so we leave */                      \
          finish = 1;                                                     \
        }                                                                 \
        else                                                              \
        {                                                                 \
          mlib_s32 still_in_the_same_cell = 1;                            \
          mlib_s32 oldq;                                                  \
                                                                          \
          if( BITS - pass >= bits )                                       \
          {                                                               \
            /* We found a color already on the bottom of the              \
               tree. Now we need to find out, what color is better        \
               here */                                                    \
            mlib_s32 mask = ~( ( 1 << pass ) - 1 );                       \
            mlib_u32 olddistance = FIND_DISTANCE_4(                       \
              palc0, palc0 & mask,                                        \
              palc1, palc1 & mask,                                        \
              palc2, palc2 & mask,                                        \
              palc3, palc3 & mask, SHIFT );                               \
            mlib_u32 newdistance = FIND_DISTANCE_4(                       \
              c0, c0 & mask,                                              \
              c1, c1 & mask,                                              \
              c2, c2 & mask,                                              \
              c3, c3 & mask, SHIFT );                                     \
                                                                          \
            if( newdistance < olddistance )                               \
            {                                                             \
              /* Change color index */                                    \
              node->contents.index[ q ] = i;                              \
            }                                                             \
            finish = 1;                                                   \
          }                                                               \
          else                                                            \
          {                                                               \
            do                                                            \
            {                                                             \
              mlib_s32 prevq = q;                                         \
                                                                          \
              pass--;                                                     \
              /*                                                          \
                split this cell to 2 quadrants if we are lucky. If        \
                not, do this several times untill we become lucky. It     \
                may not happen infinitelly since points are different     \
              */                                                          \
                                                                          \
              q = ( ( c0 >> pass ) & 1 ) |                                \
                ( ( ( c1 << 1 ) >> pass ) & 2 ) |                         \
                ( ( ( c2 << 2 ) >> pass ) & 4 ) |                         \
                ( ( ( c3 << 3 ) >> pass ) & 8 );                          \
                                                                          \
              oldq = ( ( palc0 >> pass ) & 1 ) |                          \
                ( ( ( palc1 << 1 ) >> pass ) & 2 ) |                      \
                ( ( ( palc2 << 2 ) >> pass ) & 4 ) |                      \
                ( ( ( palc3 << 3 ) >> pass ) & 8 );                       \
                                                                          \
              if( oldq != q || BITS - pass >= bits )                      \
              {                                                           \
                /*                                                        \
                  we have found the place where we place both points in   \
                  different cells. That is what we were looking for.      \
                  I knew we are lucky :)                                  \
                */                                                        \
                struct lut_node_4 *new_node;                              \
                                                                          \
                new_node = mlib_malloc( sizeof( struct lut_node_4 ) );    \
                if( !new_node )                                           \
                {                                                         \
                  mlib_free_binary_tree_4( table );                       \
                  return NULL;                                            \
                }                                                         \
                                                                          \
                /* Set the tag mask for these new cells */                \
                new_node->tag = ( 1 << q ) | ( 1 << oldq );               \
                                                                          \
                new_node->contents.index[ 0 ] = 0;                        \
                new_node->contents.index[ 1 ] = 0;                        \
                new_node->contents.index[ 2 ] = 0;                        \
                new_node->contents.index[ 3 ] = 0;                        \
                new_node->contents.index[ 4 ] = 0;                        \
                new_node->contents.index[ 5 ] = 0;                        \
                new_node->contents.index[ 6 ] = 0;                        \
                new_node->contents.index[ 7 ] = 0;                        \
                new_node->contents.index[ 8 ] = 0;                        \
                new_node->contents.index[ 9 ] = 0;                        \
                new_node->contents.index[ 10 ] = 0;                       \
                new_node->contents.index[ 11 ] = 0;                       \
                new_node->contents.index[ 12 ] = 0;                       \
                new_node->contents.index[ 13 ] = 0;                       \
                new_node->contents.index[ 14 ] = 0;                       \
                new_node->contents.index[ 15 ] = 0;                       \
                                                                          \
                if( q != oldq )                                           \
                {                                                         \
                  new_node->contents.index[ q ] = i;                      \
                  new_node->contents.index[ oldq ] = index;               \
                }                                                         \
                else                                                      \
                {                                                         \
                  mlib_s32 mask = ~( ( 1 << pass ) - 1 );                 \
                  mlib_u32 olddistance = FIND_DISTANCE_4(                 \
                    palc0, palc0 & mask,                                  \
                    palc1, palc1 & mask,                                  \
                    palc2, palc2 & mask,                                  \
                    palc3, palc3 & mask, SHIFT );                         \
                  mlib_u32 newdistance = FIND_DISTANCE_4(                 \
                    c0, c0 & mask,                                        \
                    c1, c1 & mask,                                        \
                    c2, c2 & mask,                                        \
                    c3, c3 & mask, SHIFT );                               \
                                                                          \
                  /* Change color index */                                \
                  if( newdistance < olddistance )                         \
                    new_node->contents.index[ q ] = i;                    \
                  else                                                    \
                    new_node->contents.index[ q ] = index;                \
                }                                                         \
                                                                          \
                /* Here is not a color cell now. Here is a pointer */     \
                node->tag &= ~( 1 << prevq );                             \
                node->contents.quadrants[ prevq ] = new_node;             \
                                                                          \
                still_in_the_same_cell = 0;                               \
                finish = 1;                                               \
              }                                                           \
              else                                                        \
              {                                                           \
                /*                                                        \
                  we have to split this cell and continue splitting then, \
                  because points again in the same cell                   \
                */                                                        \
                struct lut_node_4 *new_node;                              \
                                                                          \
                new_node = mlib_malloc( sizeof( struct lut_node_4 ) );    \
                if( !new_node )                                           \
                {                                                         \
                  mlib_free_binary_tree_4( table );                       \
                  return NULL;                                            \
                }                                                         \
                                                                          \
                /* copy the information about colors down the tree */     \
                new_node->tag = 1 << q;                                   \
                                                                          \
                new_node->contents.index[ 0 ] = 0;                        \
                new_node->contents.index[ 1 ] = 0;                        \
                new_node->contents.index[ 2 ] = 0;                        \
                new_node->contents.index[ 3 ] = 0;                        \
                new_node->contents.index[ 4 ] = 0;                        \
                new_node->contents.index[ 5 ] = 0;                        \
                new_node->contents.index[ 6 ] = 0;                        \
                new_node->contents.index[ 7 ] = 0;                        \
                new_node->contents.index[ 8 ] = 0;                        \
                new_node->contents.index[ 9 ] = 0;                        \
                new_node->contents.index[ 10 ] = 0;                       \
                new_node->contents.index[ 11 ] = 0;                       \
                new_node->contents.index[ 12 ] = 0;                       \
                new_node->contents.index[ 13 ] = 0;                       \
                new_node->contents.index[ 14 ] = 0;                       \
                new_node->contents.index[ 15 ] = 0;                       \
                                                                          \
                new_node->contents.index[ q ] = index;                    \
                                                                          \
                /* attach the new node */                                 \
                /* Here is not a color cell now. Here is a pointer */     \
                node->tag &= ~( 1 << prevq );                             \
                node->contents.quadrants[ prevq ] = new_node;             \
                node = new_node;                                          \
                /* and continue */                                        \
              }                                                           \
            } while ( still_in_the_same_cell );                           \
          }                                                               \
        }                                                                 \
      }                                                                   \
      else                                                                \
      {                                                                   \
        if( node->contents.quadrants[ q ] )                               \
        {                                                                 \
          /* Down we go */                                                \
          node = node->contents.quadrants[ q ];                           \
        }                                                                 \
        else                                                              \
        {                                                                 \
          /* this quadrant is yet empty */                                \
          /* let's fill it with new color */                              \
          node->tag |= 1 << q;                                            \
          node->contents.index[ q ] = i;                                  \
                                                                          \
          /* and LEAVE ! */                                               \
          finish = 1;                                                     \
        }                                                                 \
      }                                                                   \
                                                                          \
      pass--;                                                             \
                                                                          \
    } while( !finish );                                                   \
  }                                                                       \
}

/***************************************************************/ 

#define CHECK_QUADRANT_U8_4( qq )                               \
{                                                               \
  if( node->tag & ( 1 << qq ) )                                 \
  {                                                             \
    /* Here is another color cell. Check the distance */        \
    mlib_s32 new_found_color = node->contents.index[ qq ];      \
    mlib_s32 newdistance = FIND_DISTANCE_4( c[ 0 ],             \
      p[ 0 ][ new_found_color ], c[ 1 ],                        \
      p[ 1 ][ new_found_color ], c[ 2 ],                        \
      p[ 2 ][ new_found_color ], c[ 3 ],                        \
      p[ 3 ][ new_found_color ], 0 );                           \
                                                                \
    if( newdistance < distance )                                \
    {                                                           \
      found_color = new_found_color;                            \
      distance = newdistance;                                   \
    }                                                           \
  }                                                             \
  else if( node->contents.quadrants[ qq ] )                     \
    /* Here is a full node. Just explore it all */              \
    distance = mlib_search_quadrant_U8_4(                       \
      node->contents.quadrants[ qq ], distance, &found_color,   \
      c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], p );                      \
/* Else there is just an empty cell */                          \
}

/***************************************************************/ 

#define CHECK_QUADRANT_S16_4( qq )                              \
{                                                               \
  if( node->tag & ( 1 << qq ) )                                 \
  {                                                             \
    /* Here is another color cell. Check the distance */        \
    mlib_s32 new_found_color = node->contents.index[ qq ];      \
    mlib_u32 palc0, palc1, palc2, palc3, newdistance;           \
                                                                \
    palc0 = p[ 0 ][ new_found_color ] - MLIB_S16_MIN;           \
    palc1 = p[ 1 ][ new_found_color ] - MLIB_S16_MIN;           \
    palc2 = p[ 2 ][ new_found_color ] - MLIB_S16_MIN;           \
    palc3 = p[ 3 ][ new_found_color ] - MLIB_S16_MIN;           \
                                                                \
    newdistance = FIND_DISTANCE_4( c[ 0 ], palc0,               \
      c[ 1 ], palc1,                                            \
      c[ 2 ], palc2,                                            \
      c[ 3 ], palc3, 2 );                                       \
                                                                \
    if( newdistance < distance )                                \
    {                                                           \
      found_color = new_found_color;                            \
      distance = newdistance;                                   \
    }                                                           \
  }                                                             \
  else if( node->contents.quadrants[ qq ] )                     \
    /* Here is a full node. Just explore it all */              \
    distance = mlib_search_quadrant_S16_4(                      \
      node->contents.quadrants[ qq ], distance, &found_color,   \
      c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], p );                      \
/* Else there is just an empty cell */                          \
}

/***************************************************************/ 

#define BINARY_TREE_SEARCH_4( SOURCE_IMAGE, POINTER_TYPE, BITS,               \
  COLOR_MAX, SUBTRACTION, SHIFT )                                             \
{                                                                             \
  POINTER_TYPE *channels[ 4 ], *p[ 4 ];                                       \
  mlib_u32 c[ 4 ];                                                            \
  mlib_s32 j;                                                                 \
  mlib_s32 chann = s->channels;                                               \
                                                                              \
  p[ 0 ] = s->lut[ 0 ];                                                       \
  p[ 1 ] = s->lut[ 1 ];                                                       \
  p[ 2 ] = s->lut[ 2 ];                                                       \
  p[ 3 ] = s->lut[ 3 ];                                                       \
  channels[ 0 ] = src;                                                        \
  channels[ 1 ] = src + 1;                                                    \
  channels[ 2 ] = src + 2;                                                    \
  channels[ 3 ] = src + 3;                                                    \
                                                                              \
  for( j = 0; j < length; j++ )                                               \
  {                                                                           \
    mlib_s32 pass = BITS - 1;                                                 \
    mlib_u32 position[ 4 ] = { 0, 0, 0, 0 };                                  \
    mlib_s32 we_found_it = 0;                                                 \
    struct lut_node_4 *node = s->table;                                       \
    /* Stack pointer pointers to the first free element of stack. */          \
    /* The node we are in is in the `node' */                                 \
    struct                                                                    \
    {                                                                         \
      struct lut_node_4 *node;                                                \
      mlib_s32 q;                                                             \
    } stack[ BITS ];                                                          \
    mlib_s32 stack_pointer = 0;                                               \
                                                                              \
    c[ 0 ] = *channels[ 0 ] - SUBTRACTION;                                    \
    c[ 1 ] = *channels[ 1 ] - SUBTRACTION;                                    \
    c[ 2 ] = *channels[ 2 ] - SUBTRACTION;                                    \
    c[ 3 ] = *channels[ 3 ] - SUBTRACTION;                                    \
                                                                              \
    do                                                                        \
    {                                                                         \
      mlib_s32 q;                                                             \
      mlib_u32 current_size = 1 << pass;                                      \
                                                                              \
      q = ( ( c[ 0 ] >> pass ) & 1 ) |                                        \
        ( ( ( c[ 1 ] << 1 ) >> pass ) & 2 ) |                                 \
        ( ( ( c[ 2 ] << 2 ) >> pass ) & 4 ) |                                 \
        ( ( ( c[ 3 ] << 3 ) >> pass ) & 8 );                                  \
                                                                              \
      position[ 0 ] |= c[ 0 ] & current_size;                                 \
      position[ 1 ] |= c[ 1 ] & current_size;                                 \
      position[ 2 ] |= c[ 2 ] & current_size;                                 \
      position[ 3 ] |= c[ 3 ] & current_size;                                 \
                                                                              \
      if( node->tag & ( 1 << q ) )                                            \
      {                                                                       \
        /*                                                                    \
          Here is a cell with one color. We need to be sure it's              \
          the one that is the closest to our color                            \
        */                                                                    \
        mlib_s32 palindex = node->contents.index[ q ];                        \
        mlib_u32 palc[ 4 ];                                                   \
        mlib_s32 identical;                                                   \
                                                                              \
        palc[ 0 ] = p[ 0 ][ palindex ] - SUBTRACTION;                         \
        palc[ 1 ] = p[ 1 ][ palindex ] - SUBTRACTION;                         \
        palc[ 2 ] = p[ 2 ][ palindex ] - SUBTRACTION;                         \
        palc[ 3 ] = p[ 3 ][ palindex ] - SUBTRACTION;                         \
                                                                              \
        identical = ( palc[ 0 ] - c[ 0 ] ) | ( palc[ 1 ] - c[ 1 ] ) |         \
          ( palc[ 2 ] - c[ 2 ] ) | ( palc[ 3 ] - c[ 3 ] );                    \
                                                                              \
        if( !identical || BITS - pass == bits )                               \
        {                                                                     \
          /* Oh, here it is :) */                                             \
          dst[ j ] = palindex + s->offset;                                    \
          we_found_it = 1;                                                    \
        }                                                                     \
        else                                                                  \
        {                                                                     \
          mlib_u32 distance;                                                  \
          /* First index is the channel, second is the number of the          \
             side */                                                          \
          mlib_s32 found_color;                                               \
          mlib_s32 continue_up;                                               \
                                                                              \
          distance = FIND_DISTANCE_4( c[ 0 ], palc[ 0 ],                      \
            c[ 1 ], palc[ 1 ], c[ 2 ], palc[ 2 ], c[ 3 ], palc[ 3 ], SHIFT ); \
          found_color = palindex;                                             \
                                                                              \
          do                                                                  \
          {                                                                   \
            mlib_s32 check_corner;                                            \
            mlib_s32 check_neibours[ 6 ];                                     \
            mlib_s32 check_far_neibours[ 4 ];                                 \
                                                                              \
            /*                                                                \
              Check neibours: quadrants that are different by 2 bits          \
              from the quadrant, that we are in:                              \
              3 -  0                                                          \
              5 -  1                                                          \
              6 -  2                                                          \
              9 -  3                                                          \
              10 - 4                                                          \
              12 - 5                                                          \
              Far quadrants: different by 3 bits:                             \
              7  - 0                                                          \
              11 - 1                                                          \
              13 - 2                                                          \
              14 - 3                                                          \
            */                                                                \
                                                                              \
            check_neibours[ 0 ] = check_neibours[ 1 ] =                       \
              check_neibours[ 2 ] = check_neibours[ 3 ] =                     \
              check_neibours[ 4 ] = check_neibours[ 5 ] = 0;                  \
            continue_up = 0;                                                  \
                                                                              \
            if( q & 1 )                                                       \
            {                                                                 \
              BINARY_TREE_EXPLORE_LEFT_4( 0, SOURCE_IMAGE, 0, 1, 3,           \
                SUBTRACTION, SHIFT );                                         \
            }                                                                 \
            else                                                              \
            {                                                                 \
              BINARY_TREE_EXPLORE_RIGHT_4( 0, COLOR_MAX, SOURCE_IMAGE,        \
                0, 1, 3, SUBTRACTION, SHIFT );                                \
            }                                                                 \
                                                                              \
            if( q & 2 )                                                       \
            {                                                                 \
              BINARY_TREE_EXPLORE_LEFT_4( 1, SOURCE_IMAGE, 0, 2, 4,           \
                SUBTRACTION, SHIFT );                                         \
            }                                                                 \
            else                                                              \
            {                                                                 \
              BINARY_TREE_EXPLORE_RIGHT_4( 1, COLOR_MAX, SOURCE_IMAGE,        \
                0, 2, 4, SUBTRACTION, SHIFT );                                \
            }                                                                 \
                                                                              \
            if( q & 4 )                                                       \
            {                                                                 \
              BINARY_TREE_EXPLORE_LEFT_4( 2, SOURCE_IMAGE, 1, 2, 5,           \
                SUBTRACTION, SHIFT );                                         \
            }                                                                 \
            else                                                              \
            {                                                                 \
              BINARY_TREE_EXPLORE_RIGHT_4( 2, COLOR_MAX, SOURCE_IMAGE,        \
                1, 2, 5, SUBTRACTION, SHIFT );                                \
            }                                                                 \
                                                                              \
            if( q & 8 )                                                       \
            {                                                                 \
              BINARY_TREE_EXPLORE_LEFT_4( 3, SOURCE_IMAGE, 3, 4, 5,           \
                SUBTRACTION, SHIFT );                                         \
            }                                                                 \
            else                                                              \
            {                                                                 \
              BINARY_TREE_EXPLORE_RIGHT_4( 3, COLOR_MAX, SOURCE_IMAGE,        \
                3, 4, 5, SUBTRACTION, SHIFT );                                \
            }                                                                 \
                                                                              \
            check_far_neibours[ 0 ] = check_neibours[ 0 ] +                   \
              check_neibours[ 1 ] + check_neibours[ 2 ];                      \
            check_far_neibours[ 1 ] = check_neibours[ 0 ] +                   \
              check_neibours[ 3 ] + check_neibours[ 4 ];                      \
            check_far_neibours[ 2 ] = check_neibours[ 1 ] +                   \
              check_neibours[ 3 ] + check_neibours[ 5 ];                      \
            check_far_neibours[ 3 ] = check_neibours[ 2 ] +                   \
              check_neibours[ 4 ] + check_neibours[ 5 ];                      \
                                                                              \
            check_corner = check_far_neibours[ 0 ] +                          \
              check_far_neibours[ 1 ] +                                       \
              check_far_neibours[ 2 ] +                                       \
              check_far_neibours[ 3 ];                                        \
                                                                              \
            if( check_neibours[ 0 ] >= 2 )                                    \
            {                                                                 \
              mlib_s32 qq = q ^ 3;                                            \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( check_neibours[ 1 ] >= 2 )                                    \
            {                                                                 \
              mlib_s32 qq = q ^ 5;                                            \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( check_neibours[ 2 ] >= 2 )                                    \
            {                                                                 \
              mlib_s32 qq = q ^ 6;                                            \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( check_neibours[ 3 ] >= 2 )                                    \
            {                                                                 \
              mlib_s32 qq = q ^ 9;                                            \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( check_neibours[ 4 ] >= 2 )                                    \
            {                                                                 \
              mlib_s32 qq = q ^ 10;                                           \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( check_neibours[ 5 ] >= 2 )                                    \
            {                                                                 \
              mlib_s32 qq = q ^ 12;                                           \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( check_far_neibours[ 0 ] >= 3 )                                \
            {                                                                 \
              mlib_s32 qq = q ^ 7;                                            \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( check_far_neibours[ 1 ] >= 3 )                                \
            {                                                                 \
              mlib_s32 qq = q ^ 11;                                           \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( check_far_neibours[ 2 ] >= 3 )                                \
            {                                                                 \
              mlib_s32 qq = q ^ 13;                                           \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( check_far_neibours[ 3 ] >= 3 )                                \
            {                                                                 \
              mlib_s32 qq = q ^ 14;                                           \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( check_corner >= 4 )                                           \
            {                                                                 \
              mlib_s32 qq = q ^ 15;                                           \
              CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                        \
            }                                                                 \
                                                                              \
            if( q & 1 )                                                       \
            {                                                                 \
              BINARY_TREE_SEARCH_RIGHT( 0, COLOR_MAX, SHIFT );                \
            }                                                                 \
            else                                                              \
            {                                                                 \
              BINARY_TREE_SEARCH_LEFT( 0, SHIFT );                            \
            }                                                                 \
                                                                              \
            if( q & 2 )                                                       \
            {                                                                 \
              BINARY_TREE_SEARCH_RIGHT( 1, COLOR_MAX, SHIFT );                \
            }                                                                 \
            else                                                              \
            {                                                                 \
              BINARY_TREE_SEARCH_LEFT( 1, SHIFT );                            \
            }                                                                 \
                                                                              \
            if( q & 4 )                                                       \
            {                                                                 \
              BINARY_TREE_SEARCH_RIGHT( 2, COLOR_MAX, SHIFT );                \
            }                                                                 \
            else                                                              \
            {                                                                 \
              BINARY_TREE_SEARCH_LEFT( 2, SHIFT );                            \
            }                                                                 \
                                                                              \
            if( q & 8 )                                                       \
            {                                                                 \
              BINARY_TREE_SEARCH_RIGHT( 3, COLOR_MAX, SHIFT );                \
            }                                                                 \
            else                                                              \
            {                                                                 \
              BINARY_TREE_SEARCH_LEFT( 3, SHIFT );                            \
            }                                                                 \
                                                                              \
            position[ 0 ] &= ~( c[ 0 ] & current_size );                      \
            position[ 1 ] &= ~( c[ 1 ] & current_size );                      \
            position[ 2 ] &= ~( c[ 2 ] & current_size );                      \
            position[ 3 ] &= ~( c[ 3 ] & current_size );                      \
                                                                              \
            current_size <<= 1;                                               \
                                                                              \
            pass++;                                                           \
                                                                              \
            stack_pointer--;                                                  \
            q = stack[ stack_pointer ].q;                                     \
            node = stack[ stack_pointer ].node;                               \
          } while( continue_up );                                             \
                                                                              \
          dst[ j ] = found_color + s->offset;                                 \
          we_found_it = 1;                                                    \
        }                                                                     \
      }                                                                       \
      else if( node->contents.quadrants[ q ] )                                \
      {                                                                       \
        /* Descend one level */                                               \
        stack[ stack_pointer ].node = node;                                   \
        stack[ stack_pointer++ ].q = q;                                       \
        node = node->contents.quadrants[ q ];                                 \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        /* Found the empty quadrant. Look around */                           \
        mlib_u32 distance = MLIB_U32_MAX;                                     \
        mlib_s32 found_color;                                                 \
        mlib_s32 continue_up;                                                 \
                                                                              \
        /*                                                                    \
          As we had come to this level, it is warranted that there            \
          are other points on this level near the empty quadrant              \
        */                                                                    \
        do                                                                    \
        {                                                                     \
          mlib_s32 check_corner;                                              \
          mlib_s32 check_neibours[ 6 ];                                       \
          mlib_s32 check_far_neibours[ 4 ];                                   \
                                                                              \
          /*                                                                  \
            Check neibours: quadrants that are different by 2 bits            \
            from the quadrant, that we are in:                                \
            3 -  0                                                            \
            5 -  1                                                            \
            6 -  2                                                            \
            9 -  3                                                            \
            10 - 4                                                            \
            12 - 5                                                            \
            Far quadrants: different by 3 bits:                               \
            7  - 0                                                            \
            11 - 1                                                            \
            13 - 2                                                            \
            14 - 3                                                            \
          */                                                                  \
                                                                              \
          check_neibours[ 0 ] = check_neibours[ 1 ] =                         \
            check_neibours[ 2 ] = check_neibours[ 3 ] =                       \
            check_neibours[ 4 ] = check_neibours[ 5 ] = 0;                    \
          continue_up = 0;                                                    \
                                                                              \
          if( q & 1 )                                                         \
          {                                                                   \
            BINARY_TREE_EXPLORE_LEFT_4( 0, SOURCE_IMAGE, 0, 1, 3,             \
              SUBTRACTION, SHIFT );                                           \
          }                                                                   \
          else                                                                \
          {                                                                   \
            BINARY_TREE_EXPLORE_RIGHT_4( 0, COLOR_MAX, SOURCE_IMAGE,          \
              0, 1, 3, SUBTRACTION, SHIFT );                                  \
          }                                                                   \
                                                                              \
          if( q & 2 )                                                         \
          {                                                                   \
            BINARY_TREE_EXPLORE_LEFT_4( 1, SOURCE_IMAGE, 0, 2, 4,             \
              SUBTRACTION, SHIFT );                                           \
          }                                                                   \
          else                                                                \
          {                                                                   \
            BINARY_TREE_EXPLORE_RIGHT_4( 1, COLOR_MAX, SOURCE_IMAGE,          \
              0, 2, 4, SUBTRACTION, SHIFT );                                  \
          }                                                                   \
                                                                              \
          if( q & 4 )                                                         \
          {                                                                   \
            BINARY_TREE_EXPLORE_LEFT_4( 2, SOURCE_IMAGE, 1, 2, 5,             \
              SUBTRACTION, SHIFT );                                           \
          }                                                                   \
          else                                                                \
          {                                                                   \
            BINARY_TREE_EXPLORE_RIGHT_4( 2, COLOR_MAX, SOURCE_IMAGE,          \
              1, 2, 5, SUBTRACTION, SHIFT );                                  \
          }                                                                   \
                                                                              \
          if( q & 8 )                                                         \
          {                                                                   \
            BINARY_TREE_EXPLORE_LEFT_4( 3, SOURCE_IMAGE, 3, 4, 5,             \
              SUBTRACTION, SHIFT );                                           \
          }                                                                   \
          else                                                                \
          {                                                                   \
            BINARY_TREE_EXPLORE_RIGHT_4( 3, COLOR_MAX, SOURCE_IMAGE,          \
              3, 4, 5, SUBTRACTION, SHIFT );                                  \
          }                                                                   \
                                                                              \
          check_far_neibours[ 0 ] = check_neibours[ 0 ] +                     \
            check_neibours[ 1 ] + check_neibours[ 2 ];                        \
          check_far_neibours[ 1 ] = check_neibours[ 0 ] +                     \
            check_neibours[ 3 ] + check_neibours[ 4 ];                        \
          check_far_neibours[ 2 ] = check_neibours[ 1 ] +                     \
            check_neibours[ 3 ] + check_neibours[ 5 ];                        \
          check_far_neibours[ 3 ] = check_neibours[ 2 ] +                     \
            check_neibours[ 4 ] + check_neibours[ 5 ];                        \
                                                                              \
          check_corner = check_far_neibours[ 0 ] +                            \
            check_far_neibours[ 1 ] +                                         \
            check_far_neibours[ 2 ] +                                         \
            check_far_neibours[ 3 ];                                          \
                                                                              \
          if( check_neibours[ 0 ] >= 2 )                                      \
          {                                                                   \
            mlib_s32 qq = q ^ 3;                                              \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( check_neibours[ 1 ] >= 2 )                                      \
          {                                                                   \
            mlib_s32 qq = q ^ 5;                                              \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( check_neibours[ 2 ] >= 2 )                                      \
          {                                                                   \
            mlib_s32 qq = q ^ 6;                                              \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( check_neibours[ 3 ] >= 2 )                                      \
          {                                                                   \
            mlib_s32 qq = q ^ 9;                                              \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( check_neibours[ 4 ] >= 2 )                                      \
          {                                                                   \
            mlib_s32 qq = q ^ 10;                                             \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( check_neibours[ 5 ] >= 2 )                                      \
          {                                                                   \
            mlib_s32 qq = q ^ 12;                                             \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( check_far_neibours[ 0 ] >= 3 )                                  \
          {                                                                   \
            mlib_s32 qq = q ^ 7;                                              \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( check_far_neibours[ 1 ] >= 3 )                                  \
          {                                                                   \
            mlib_s32 qq = q ^ 11;                                             \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( check_far_neibours[ 2 ] >= 3 )                                  \
          {                                                                   \
            mlib_s32 qq = q ^ 13;                                             \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( check_far_neibours[ 3 ] >= 3 )                                  \
          {                                                                   \
            mlib_s32 qq = q ^ 14;                                             \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( check_corner >= 4 )                                             \
          {                                                                   \
            mlib_s32 qq = q ^ 15;                                             \
            CHECK_QUADRANT_##SOURCE_IMAGE##_4( qq );                          \
          }                                                                   \
                                                                              \
          if( q & 1 )                                                         \
          {                                                                   \
            BINARY_TREE_SEARCH_RIGHT( 0, COLOR_MAX, SHIFT );                  \
          }                                                                   \
          else                                                                \
          {                                                                   \
            BINARY_TREE_SEARCH_LEFT( 0, SHIFT );                              \
          }                                                                   \
                                                                              \
          if( q & 2 )                                                         \
          {                                                                   \
            BINARY_TREE_SEARCH_RIGHT( 1, COLOR_MAX, SHIFT );                  \
          }                                                                   \
          else                                                                \
          {                                                                   \
            BINARY_TREE_SEARCH_LEFT( 1, SHIFT );                              \
          }                                                                   \
                                                                              \
          if( q & 4 )                                                         \
          {                                                                   \
            BINARY_TREE_SEARCH_RIGHT( 2, COLOR_MAX, SHIFT );                  \
          }                                                                   \
          else                                                                \
          {                                                                   \
            BINARY_TREE_SEARCH_LEFT( 2, SHIFT );                              \
          }                                                                   \
                                                                              \
          if( q & 8 )                                                         \
          {                                                                   \
            BINARY_TREE_SEARCH_RIGHT( 3, COLOR_MAX, SHIFT );                  \
          }                                                                   \
          else                                                                \
          {                                                                   \
            BINARY_TREE_SEARCH_LEFT( 3, SHIFT );                              \
          }                                                                   \
                                                                              \
          position[ 0 ] &= ~( c[ 0 ] & current_size );                        \
          position[ 1 ] &= ~( c[ 1 ] & current_size );                        \
          position[ 2 ] &= ~( c[ 2 ] & current_size );                        \
          position[ 3 ] &= ~( c[ 3 ] & current_size );                        \
                                                                              \
          current_size <<= 1;                                                 \
                                                                              \
          pass++;                                                             \
                                                                              \
          stack_pointer--;                                                    \
          q = stack[ stack_pointer ].q;                                       \
          node = stack[ stack_pointer ].node;                                 \
        } while( continue_up );                                               \
                                                                              \
        dst[ j ] = found_color + s->offset;                                   \
        we_found_it = 1;                                                      \
      }                                                                       \
                                                                              \
      pass--;                                                                 \
                                                                              \
    } while( !we_found_it );                                                  \
                                                                              \
    channels[ 0 ] += 4;                                                       \
    channels[ 1 ] += 4;                                                       \
    channels[ 2 ] += 4;                                                       \
    channels[ 3 ] += 4;                                                       \
  }                                                                           \
}

/***************************************************************/ 

#define FSQR_S16_HI(dsrc)                                                   \
  vis_fpadd32( vis_fmuld8ulx16( vis_read_hi( dsrc ), vis_read_hi( dsrc ) ), \
    vis_fmuld8sux16( vis_read_hi( dsrc ), vis_read_hi( dsrc ) ) )

#define FSQR_S16_LO(dsrc)                                                  \
  vis_fpadd32( vis_fmuld8ulx16( vis_read_lo( dsrc ), vis_read_lo( dsrc) ), \
    vis_fmuld8sux16( vis_read_lo( dsrc ), vis_read_lo( dsrc ) ) )

/***************************************************************/ 

#define FIND_NEAREST_U8_3                                               \
{                                                                       \
  mlib_d64 *dpsrc, dsrc, dsrc1, ddist, ddist1, ddist2, ddist3;          \
  mlib_d64 dcolor, dind, dres, dres1, dpind[1], dpmin[1];               \
  mlib_d64 done = vis_to_double_dup( 1 ),                               \
           dmask = vis_to_double( 0xffff, 0xffffffff ),                 \
           dmax = vis_to_double_dup( MLIB_S32_MAX );                    \
  mlib_f32 *lut = ( mlib_f32 * )mlib_ImageGetLutNormalTable( s );       \
  mlib_f32 fone = vis_to_float( 0x100 );                                \
  int      i, k, mask;                                                  \
  int      gsr[1];                                                      \
  mlib_s32 offset = mlib_ImageGetLutOffset( s ) - 1;                    \
  mlib_s32 entries = s->lutlength;                                      \
                                                                        \
  gsr[0] = vis_read_gsr();                                              \
  for( i = 0; i <= ( length-2 ); i += 2 )                               \
  {                                                                     \
    dpsrc = vis_alignaddr( src, -1 );                                   \
    src += 6;                                                           \
    dsrc = dpsrc[ 0 ];                                                  \
    dsrc1 = dpsrc[ 1 ];                                                 \
    dsrc1 = vis_faligndata( dsrc, dsrc1 );                              \
    dsrc = vis_fmul8x16al( vis_read_hi( dsrc1 ), fone );                \
    dsrc = vis_fand( dsrc, dmask );                                     \
    vis_alignaddr( dpsrc, 3 );                                          \
    dsrc1 = vis_faligndata( dsrc1, dsrc1 );                             \
    dsrc1 = vis_fmul8x16al( vis_read_hi( dsrc1 ), fone );               \
    dsrc = vis_fand( dsrc, dmask );                                     \
    dpind[ 0 ] = dind = done;                                           \
    dpmin[ 0 ] = dmax;                                                  \
    dcolor = vis_fmul8x16al( lut[ 0 ], fone );                          \
    for( k = 1; k <= entries; k++ )                                     \
    {                                                                   \
      ddist1 = vis_fpsub16( dcolor, dsrc );                             \
      ddist = FSQR_S16_HI( ddist1 );                                    \
      ddist1 = FSQR_S16_LO( ddist1 );                                   \
      dres = vis_fpadd32( ddist, ddist1 );                              \
      ddist3 = vis_fpsub16( dcolor, dsrc1 );                            \
      ddist2 = FSQR_S16_HI( ddist3 );                                   \
      ddist3 = FSQR_S16_LO( ddist3 );                                   \
      dres1 = vis_fpadd32( ddist2, ddist3 );                            \
      dcolor = vis_fmul8x16al( lut[ k ], fone );                        \
      dres = vis_freg_pair(                                             \
        vis_fpadd32s( vis_read_hi( dres ), vis_read_lo( dres ) ),       \
        vis_fpadd32s( vis_read_hi( dres1 ), vis_read_lo( dres1 ) ) );   \
      mask = vis_fcmplt32( dres, dpmin[ 0 ] );                          \
      vis_pst_32( dind, ( void * )dpind, mask );                        \
      dind = vis_fpadd32( dind, done );                                 \
      vis_pst_32( dres, ( void * )dpmin, mask );                        \
    }                                                                   \
    dst[ i ] = ( ( mlib_s32 * )dpind )[ 0 ] + offset;                   \
    dst[ i + 1 ] = ( ( mlib_s32 * )dpind)[ 1 ] + offset;                \
  }                                                                     \
  if( i < length )                                                      \
  {                                                                     \
    dpsrc = vis_alignaddr( src, -1 );                                   \
    dsrc = dpsrc[ 0 ];                                                  \
    dsrc1 = dpsrc[ 1 ];                                                 \
    dsrc1 = vis_faligndata( dsrc, dsrc1 );                              \
    dsrc = vis_fmul8x16al( vis_read_hi( dsrc1 ), fone );                \
    dsrc = vis_fand( dsrc, dmask );                                     \
    dpind[ 0 ] = dind = done;                                           \
    dpmin[ 0 ] = dmax;                                                  \
    for( k = 0; k < entries; k++ )                                      \
    {                                                                   \
      dcolor = vis_fmul8x16al( lut[ k ], fone );                        \
      ddist1 = vis_fpsub16( dcolor, dsrc );                             \
      ddist = FSQR_S16_HI( ddist1 );                                    \
      ddist1 = FSQR_S16_LO( ddist1 );                                   \
      dres = vis_fpadd32( ddist, ddist1 );                              \
      dres = vis_write_lo( dres,                                        \
        vis_fpadd32s( vis_read_hi( dres ), vis_read_lo( dres ) ) );     \
      mask = vis_fcmplt32( dres, dpmin[ 0 ] );                          \
      vis_pst_32( dind, ( void * )dpind, mask );                        \
      dind = vis_fpadd32( dind, done );                                 \
      vis_pst_32( dres, ( void * )dpmin, mask );                        \
    }                                                                   \
    dst[ i ] = ( ( mlib_s32 * )dpind)[ 1 ] + offset;                    \
  }                                                                     \
  vis_write_gsr(gsr[0]);                                                \
}

/***************************************************************/ 

#define FIND_NEAREST_U8_3_IN4                                           \
{                                                                       \
  mlib_d64 *dpsrc, dsrc, dsrc1, ddist, ddist1, ddist2, ddist3;          \
  mlib_d64 dcolor, dind, dres, dres1, dpind[1], dpmin[1];               \
  mlib_d64 done = vis_to_double_dup( 1 ),                               \
           dmask = vis_to_double( 0xffff, 0xffffffff ),                 \
           dmax = vis_to_double_dup( MLIB_S32_MAX );                    \
  mlib_f32 *lut = ( mlib_f32 * )mlib_ImageGetLutNormalTable( s );       \
  mlib_f32 fone = vis_to_float( 0x100 );                                \
  int i, k, mask, gsr[1];                                               \
  mlib_s32 offset = mlib_ImageGetLutOffset( s ) - 1;                    \
  mlib_s32 entries = s->lutlength;                                      \
                                                                        \
  gsr[0] = vis_read_gsr();                                              \
  dpsrc = vis_alignaddr( src, 0 );                                      \
  for( i = 0; i <= ( length-2 ); i += 2 )                               \
  {                                                                     \
    dsrc = dpsrc[ 0 ];                                                  \
    dsrc1 = dpsrc[ 1 ];                                                 \
    dsrc1 = vis_faligndata( dsrc, dsrc1 );                              \
    dpsrc++;                                                            \
    dsrc = vis_fmul8x16al( vis_read_hi( dsrc1 ), fone );                \
    dsrc = vis_fand( dsrc, dmask );                                     \
    dsrc1 = vis_fmul8x16al( vis_read_lo( dsrc1 ), fone );               \
    dsrc = vis_fand( dsrc, dmask );                                     \
    dpind[ 0 ] = dind = done;                                           \
    dpmin[ 0 ] = dmax;                                                  \
    dcolor = vis_fmul8x16al( lut[ 0 ], fone );                          \
    for( k = 1; k <= entries; k++ )                                     \
    {                                                                   \
      ddist1 = vis_fpsub16( dcolor, dsrc );                             \
      ddist = FSQR_S16_HI( ddist1 );                                    \
      ddist1 = FSQR_S16_LO( ddist1 );                                   \
      dres = vis_fpadd32( ddist, ddist1 );                              \
      ddist3 = vis_fpsub16( dcolor, dsrc1 );                            \
      ddist2 = FSQR_S16_HI( ddist3 );                                   \
      ddist3 = FSQR_S16_LO( ddist3 );                                   \
      dres1 = vis_fpadd32( ddist2, ddist3 );                            \
      dcolor = vis_fmul8x16al( lut[ k ], fone );                        \
      dres = vis_freg_pair(                                             \
        vis_fpadd32s( vis_read_hi( dres ), vis_read_lo( dres ) ),       \
        vis_fpadd32s( vis_read_hi( dres1 ), vis_read_lo( dres1 ) ) );   \
      mask = vis_fcmplt32( dres, dpmin[ 0 ] );                          \
      vis_pst_32( dind, ( void * )dpind, mask );                        \
      dind = vis_fpadd32( dind, done );                                 \
      vis_pst_32( dres, ( void * )dpmin, mask );                        \
    }                                                                   \
    dst[ i ] = ( ( mlib_s32 * )dpind )[ 0 ] + offset;                   \
    dst[ i + 1 ] = ( ( mlib_s32 * )dpind)[ 1 ] + offset;                \
  }                                                                     \
  if( i < length )                                                      \
  {                                                                     \
    dsrc = dpsrc[ 0 ];                                                  \
    dsrc1 = dpsrc[ 1 ];                                                 \
    dsrc1 = vis_faligndata( dsrc, dsrc1 );                              \
    dsrc = vis_fmul8x16al( vis_read_hi( dsrc1 ), fone );                \
    dsrc = vis_fand( dsrc, dmask );                                     \
    dpind[ 0 ] = dind = done;                                           \
    dpmin[ 0 ] = dmax;                                                  \
    for( k = 0; k < entries; k++ )                                      \
    {                                                                   \
      dcolor = vis_fmul8x16al( lut[ k ], fone );                        \
      ddist1 = vis_fpsub16( dcolor, dsrc );                             \
      ddist = FSQR_S16_HI( ddist1 );                                    \
      ddist1 = FSQR_S16_LO( ddist1 );                                   \
      dres = vis_fpadd32( ddist, ddist1 );                              \
      dres = vis_write_lo( dres,                                        \
        vis_fpadd32s( vis_read_hi( dres ), vis_read_lo( dres ) ) );     \
      mask = vis_fcmplt32( dres, dpmin[ 0 ] );                          \
      vis_pst_32( dind, ( void * )dpind, mask );                        \
      dind = vis_fpadd32( dind, done );                                 \
      vis_pst_32( dres, ( void * )dpmin, mask );                        \
    }                                                                   \
    dst[ i ] = ( ( mlib_s32 * )dpind)[ 1 ] + offset;                    \
  }                                                                     \
  vis_write_gsr(gsr[0]);                                                \
}
   
/***************************************************************/ 

#define FIND_NEAREST_U8_4                                               \
{                                                                       \
  mlib_d64 *dpsrc, dsrc, dsrc1, ddist, ddist1, ddist2, ddist3;          \
  mlib_d64 dcolor, dind, dres, dres1, dpind[ 1 ], dpmin[ 1 ];           \
  mlib_d64 done = vis_to_double_dup( 1 ),                               \
           dmax = vis_to_double_dup( MLIB_S32_MAX );                    \
  mlib_f32 *lut = ( mlib_f32 * )mlib_ImageGetLutNormalTable( s );       \
  mlib_f32 fone = vis_to_float( 0x100 );                                \
  int i, k, mask, gsr[1];                                               \
  mlib_s32 offset = mlib_ImageGetLutOffset( s ) - 1;                    \
  mlib_s32 entries = s->lutlength;                                      \
                                                                        \
  gsr[0] = vis_read_gsr();                                              \
  dpsrc = vis_alignaddr( src, 0 );                                      \
  for( i = 0; i <= ( length-2 ); i += 2 )                               \
  {                                                                     \
    dsrc = dpsrc[ 0 ];                                                  \
    dsrc1 = dpsrc[ 1 ];                                                 \
    dsrc1 = vis_faligndata( dsrc, dsrc1 );                              \
    dpsrc++;                                                            \
    dsrc = vis_fmul8x16al( vis_read_hi( dsrc1 ), fone );                \
    dsrc1 = vis_fmul8x16al( vis_read_lo( dsrc1 ), fone );               \
    dpind[ 0 ] = dind = done;                                           \
    dpmin[ 0 ] = dmax;                                                  \
    dcolor = vis_fmul8x16al(lut[0], fone);                              \
    for( k = 1; k <= entries; k++ )                                     \
    {                                                                   \
      ddist1 = vis_fpsub16( dcolor, dsrc );                             \
      ddist = FSQR_S16_HI( ddist1 );                                    \
      ddist1 = FSQR_S16_LO( ddist1 );                                   \
      dres = vis_fpadd32( ddist, ddist1 );                              \
      ddist3 = vis_fpsub16( dcolor, dsrc1 );                            \
      ddist2 = FSQR_S16_HI( ddist3 );                                   \
      ddist3 = FSQR_S16_LO( ddist3 );                                   \
      dres1 = vis_fpadd32( ddist2, ddist3 );                            \
      dcolor = vis_fmul8x16al( lut[ k ], fone );                        \
      dres = vis_freg_pair(                                             \
        vis_fpadd32s( vis_read_hi( dres ), vis_read_lo( dres ) ),       \
        vis_fpadd32s( vis_read_hi( dres1 ), vis_read_lo( dres1 ) ) );   \
      mask = vis_fcmplt32( dres, dpmin[ 0 ] );                          \
      vis_pst_32( dind, ( void * )dpind, mask );                        \
      dind = vis_fpadd32( dind, done );                                 \
      vis_pst_32( dres, ( void * )dpmin, mask );                        \
    }                                                                   \
    dst[ i ] = ( ( mlib_s32 * )dpind )[ 0 ] + offset;                   \
    dst[ i + 1 ] = ( ( mlib_s32 * )dpind )[ 1 ] + offset;               \
  }                                                                     \
  if( i < length )                                                      \
  {                                                                     \
    dsrc = dpsrc[ 0 ];                                                  \
    dsrc1 = dpsrc[ 1 ];                                                 \
    dsrc1 = vis_faligndata( dsrc, dsrc1 );                              \
    dsrc = vis_fmul8x16al( vis_read_hi( dsrc1 ), fone );                \
    dpind[ 0 ] = dind = done;                                           \
    dpmin[ 0 ] = dmax;                                                  \
    for( k = 0; k < entries; k++ )                                      \
    {                                                                   \
      dcolor = vis_fmul8x16al( lut[ k ], fone );                        \
      ddist1 = vis_fpsub16( dcolor, dsrc );                             \
      ddist = FSQR_S16_HI( ddist1 );                                    \
      ddist1 = FSQR_S16_LO( ddist1 );                                   \
      dres = vis_fpadd32( ddist, ddist1 );                              \
      dres = vis_write_lo( dres,                                        \
        vis_fpadd32s( vis_read_hi( dres ), vis_read_lo( dres ) ) );     \
      mask = vis_fcmplt32( dres, dpmin[ 0 ] );                          \
      vis_pst_32( dind, ( void * )dpind, mask );                        \
      dind = vis_fpadd32( dind, done );                                 \
      vis_pst_32( dres, ( void * )dpmin, mask );                        \
    }                                                                   \
    dst[ i ] = ( ( mlib_s32 * )dpind )[ 1 ] + offset;                   \
  }                                                                     \
  vis_write_gsr(gsr[0]);                                                \
}

/***************************************************************/ 

#define FIND_NEAREST_S16_3( SHIFT, STEP )                       \
  int i, k, k_min, min_dist, diff, mask;                        \
  mlib_s32 offset = mlib_ImageGetLutOffset( s ) - 1;            \
  mlib_s32 entries = s->lutlength;                              \
  mlib_d64 *double_lut = mlib_ImageGetLutDoubleData( s );       \
  mlib_d64 col0, col1, col2;                                    \
  mlib_d64 dist, len0, len1, len2;                              \
                                                                \
  for( i = 0; i < length; i++ )                                 \
  {                                                             \
    col0 = src[ STEP * i + SHIFT ];                             \
    col1 = src[ STEP * i + 1 + SHIFT ];                         \
    col2 = src[ STEP * i + 2 + SHIFT ];                         \
    min_dist = MLIB_S32_MAX;                                    \
    k_min = 1;                                                  \
    len0 = double_lut[ 0 ] - col0;                              \
    len1 = double_lut[ 1 ] - col1;                              \
    len2 = double_lut[ 2 ] - col2;                              \
    for( k = 1; k <= entries; k++ )                             \
    {                                                           \
      dist = len0 * len0;                                       \
      len0 = double_lut[ 3 * k ] - col0;                        \
      dist += len1 * len1;                                      \
      len1 = double_lut[ 3 * k + 1 ] - col1;                    \
      dist += len2 * len2;                                      \
      len2 = double_lut[ 3 * k + 2 ] - col2;                    \
      diff = ( mlib_s32 )( dist * 0.125 ) - min_dist;           \
      mask = diff >> 31;                                        \
      min_dist += diff & mask;                                  \
      k_min += ( k - k_min ) & mask;                            \
    }                                                           \
    dst[ i ] = k_min + offset;                                  \
  }

/***************************************************************/ 

#define FIND_NEAREST_S16_4                                      \
  int i, k, k_min, min_dist, diff, mask;                        \
  mlib_s32 offset = mlib_ImageGetLutOffset( s ) - 1;            \
  mlib_s32 entries = s->lutlength;                              \
  mlib_d64 *double_lut = mlib_ImageGetLutDoubleData( s );       \
  mlib_d64 col0, col1, col2, col3;                              \
  mlib_d64 dist, len0, len1, len2, len3;                        \
                                                                \
  for( i = 0; i < length; i++ )                                 \
  {                                                             \
    col0 = src[ 4 * i ];                                        \
    col1 = src[ 4 * i + 1 ];                                    \
    col2 = src[ 4 * i + 2 ];                                    \
    col3 = src[ 4 * i + 3 ];                                    \
    min_dist = MLIB_S32_MAX;                                    \
    k_min = 1;                                                  \
    len0 = double_lut[ 0 ] - col0;                              \
    len1 = double_lut[ 1 ] - col1;                              \
    len2 = double_lut[ 2 ] - col2;                              \
    len3 = double_lut[ 3 ] - col3;                              \
    for( k = 1; k <= entries; k++ )                             \
    {                                                           \
      dist = len0 * len0;                                       \
      len0 =  double_lut[ 4 * k ] - col0;                       \
      dist += len1 * len1;                                      \
      len1 = double_lut[ 4 * k + 1 ] - col1;                    \
      dist += len2 * len2;                                      \
      len2 =  double_lut[ 4 * k + 2 ] - col2;                   \
      dist += len3 * len3;                                      \
      len3 =  double_lut[ 4 * k + 3 ] - col3;                   \
      diff = ( mlib_s32 )( dist * 0.125 ) - min_dist;           \
      mask = diff >> 31;                                        \
      min_dist += diff & mask;                                  \
      k_min += ( k - k_min ) & mask;                            \
    }                                                           \
    dst[ i ] = k_min + offset;                                  \
  }

/***************************************************************/ 

#ifdef MLIB_TEST

mlib_status mlib_v_ImageColorTrue2IndexInit(

#else

mlib_status mlib_ImageColorTrue2IndexInit(

#endif
                                          void **colormap,
                                          mlib_s32 bits,
                                          mlib_type intype,
                                          mlib_type outtype,
                                          mlib_s32 channels,
                                          mlib_s32 entries,
                                          mlib_s32 offset,
                                          void **table )
{
  mlib_s32 src_bits, method, outsize, pal_size;
  void *transformed_table;
  static mlib_s32 switch_table[ 2 ][ 2 ] =
  {
    { 1000, 3000 }, /*  U8_S16_3 */ /*  U8_S16_4 */
    { 1000, 1000 }  /* S16_S16_3 */ /* S16_S16_4 */
  };

  /* Check for obvious errors */
  if( !table )
    return MLIB_NULLPOINTER;

  if( entries <= 0 || channels > 4 || channels < 3 )
    return MLIB_OUTOFRANGE;

  if( !table[ 0 ] || !table[ 1 ] || !table[ 2 ] )
    return MLIB_NULLPOINTER;

  if( channels == 4 )
    if( !table[ 3 ] )
      return MLIB_NULLPOINTER;

  if( intype == MLIB_BYTE )
    src_bits = 8;
  else if( intype == MLIB_SHORT )
    src_bits = 16;
  else
    return MLIB_FAILURE;

  if( src_bits < bits )
    return MLIB_FAILURE;

  if( offset >= MLIB_U8_MIN && offset + entries - 1 <= MLIB_U8_MAX )
    outsize = 1;
  else
  {
    outsize = 2;
    if( offset < MLIB_S16_MIN || offset + entries - 1 > MLIB_S16_MAX )
      return MLIB_OUTOFRANGE;
  }

  if( outsize == 2 && outtype == MLIB_BYTE )
    return MLIB_FAILURE;

  if( channels * bits <= 24 && bits > 0 )
    method = LUT_COLOR_CUBE_SEARCH;
  else
  {
    if( entries <= switch_table[ src_bits / 8 - 1 ][ channels - 3 ] )
      method = LUT_STUPID_SEARCH;
    else
      method = LUT_BINARY_TREE_SEARCH;
    if( !bits )
      bits = src_bits;
  }

  *colormap = mlib_malloc( sizeof( mlib_colormap ) );

  if( !( *colormap ) )
    return MLIB_FAILURE;

  ( ( mlib_colormap * )*colormap )->lut = table;
  ( ( mlib_colormap * )*colormap )->intype = intype;
  ( ( mlib_colormap * )*colormap )->outtype = outtype;
  ( ( mlib_colormap * )*colormap )->indexsize = outsize;
  ( ( mlib_colormap * )*colormap )->bits = bits;
  ( ( mlib_colormap * )*colormap )->method = method;
  ( ( mlib_colormap * )*colormap )->channels = channels;
  ( ( mlib_colormap * )*colormap )->lutlength = entries;
  ( ( mlib_colormap * )*colormap )->offset = offset;

  pal_size = entries * 4;
  if( intype == MLIB_SHORT )
    pal_size *= 2;

  {
    mlib_s32 j;
    mlib_s32 *normal_table;
    mlib_d64 *double_lut;

    normal_table = mlib_malloc( pal_size );
    if( !normal_table )
    {
      mlib_free( *colormap );
      return MLIB_FAILURE;
    }
    ( ( mlib_colormap * )*colormap )->normal_table = ( mlib_u8 * )normal_table;

    double_lut = mlib_malloc( entries*channels*sizeof(mlib_d64) );
    if( !double_lut )
    {
      mlib_free( ( ( mlib_colormap * )*colormap )->normal_table );
      mlib_free( *colormap );
      return MLIB_FAILURE;
    }
    ( ( mlib_colormap * )*colormap )->double_lut = ( mlib_d64 * )double_lut;

    if( intype == MLIB_BYTE )
    {
      mlib_u8 **colors = ( mlib_u8 ** )table;

      if( channels == 3 )
      {
        for( j = 0; j < entries; j++ )
        {
          normal_table[ j ] = ( colors[ 0 ][ j ] << 16 ) |
            ( colors[ 1 ][ j ] << 8 ) |
            colors[ 2 ][ j ];
          double_lut[ 3 * j ] = colors[ 0 ][ j ];
          double_lut[ 3 * j + 1 ] = colors[ 1 ][ j ];
          double_lut[ 3 * j + 2 ] = colors[ 2 ][ j ];
        }
      }
      else
      { /* channels == 4 */
        for( j = 0; j < entries; j++ ) {
          normal_table[ j ] = ( colors[ 0 ][ j ] << 24 ) |
            ( colors[ 1 ][ j ] << 16 ) |
            ( colors[ 2 ][ j ] << 8 ) |
            colors[ 3 ][ j ];
          double_lut[ 4 * j ] = colors[ 0 ][ j ];
          double_lut[ 4 * j + 1 ] = colors[ 1 ][ j ];
          double_lut[ 4 * j + 2 ] = colors[ 2 ][ j ];
          double_lut[ 4 * j + 3 ] = colors[ 3 ][ j ];
        }
      }
    }
    else
    { /* intype == MLIB_SHORT */
      mlib_u16 **colors = ( mlib_u16 ** )table;

      if( channels == 3 )
      {
        for( j = 0; j < entries; j++ )
        {
          normal_table[ 2 * j ] = colors[ 0 ][ j ];
          normal_table[ 2 * j + 1 ] = ( colors[ 1 ][ j ] << 16 ) |
            colors[ 2 ][ j ];
          double_lut[ 3 * j ] = ( ( mlib_s16 ** )colors )[ 0 ][ j ];
          double_lut[ 3 * j + 1 ] = ( ( mlib_s16 ** )colors )[ 1 ][ j ];
          double_lut[ 3 * j + 2 ] = ( ( mlib_s16 ** )colors )[ 2 ][ j ];
        }
      }
      else
      { /* channels == 4 */
        for( j = 0; j < entries; j++ )
        {
          normal_table[ 2 * j ] = ( colors[ 0 ][ j ] << 16 ) |
            colors[ 1 ][ j ];
          normal_table[ 2 * j + 1 ] = ( colors[ 2 ][ j ] << 16 ) |
            colors[ 3 ][ j ];
          double_lut[ 4 * j ] = ( ( mlib_s16 ** )colors )[ 0 ][ j ];
          double_lut[ 4 * j + 1 ] = ( ( mlib_s16 ** )colors )[ 1 ][ j ];
          double_lut[ 4 * j + 2 ] = ( ( mlib_s16 ** )colors )[ 2 ][ j ];
          double_lut[ 4 * j + 3 ] = ( ( mlib_s16 ** )colors )[ 3 ][ j ];
        }
      }
    }
  }

  switch( method )
  {
    case LUT_COLOR_CUBE_SEARCH:
    {
      switch( intype )
      {
        case MLIB_BYTE:
        {
          switch( channels )
          {
            case 3:
              transformed_table =
                mlib_color_cube_init_U8_3( ( mlib_colormap * )*colormap );
              break;
            case 4:
              transformed_table =
                mlib_color_cube_init_U8_4( ( mlib_colormap * )*colormap );
              break;
            default:
              return MLIB_FAILURE;
          }
          break;
        }
        case MLIB_SHORT:
        {
          switch( channels )
          {
            case 3:
              transformed_table =
                mlib_color_cube_init_S16_3( ( mlib_colormap * )*colormap );
              break;
            case 4:
              transformed_table =
                mlib_color_cube_init_S16_4( ( mlib_colormap * )*colormap );
              break;
            default:
              return MLIB_FAILURE;
          }
          break;
        }
        default:
          return MLIB_FAILURE;
      }
      break;
    }
    case LUT_BINARY_TREE_SEARCH:
    {
      switch( channels )
      {
        case 3:
          transformed_table =
            mlib_binary_tree_init_3( ( mlib_colormap * )*colormap );
          break;
        case 4:
          transformed_table =
            mlib_binary_tree_init_4( ( mlib_colormap * )*colormap );
          break;
        default:
          return MLIB_FAILURE;
      }
      break;
    }
  }

  if( !transformed_table && method != LUT_STUPID_SEARCH )
  {
/*
    mlib_free( ( ( mlib_colormap * )*colormap )->normal_table );
    mlib_free( *colormap );
    return MLIB_FAILURE;
*/
    /* Better to run linear search if we don't have enough memory for
       anything faster */
    ( ( mlib_colormap * )*colormap )->method = LUT_STUPID_SEARCH;
  }

  ( ( mlib_colormap * )*colormap )->table = transformed_table;

  return MLIB_SUCCESS;
}

/***************************************************************/ 

#ifdef MLIB_TEST

mlib_status mlib_v_ImageColorTrue2Index( 

#else

mlib_status mlib_ImageColorTrue2Index(

#endif
                                       mlib_image *dst,
                                       mlib_image *src,
                                       void *colormap )
{
  mlib_s32 y, width, height, sstride, dstride, schann, dchann;
  mlib_colormap *s = colormap;
  mlib_s32 maplength, channels;
  mlib_type stype, dtype;

  MLIB_IMAGE_CHECK(src);
  MLIB_IMAGE_CHECK(dst);
  MLIB_IMAGE_SIZE_EQUAL(src, dst);
  MLIB_IMAGE_HAVE_CHAN(dst, 1);

  if(!colormap)
    return MLIB_NULLPOINTER;

  maplength = s->lutlength;
  channels = s->channels;
  stype = mlib_ImageGetType( src );
  dtype = mlib_ImageGetType( dst );
  width = mlib_ImageGetWidth( src );
  height = mlib_ImageGetHeight( src );
  sstride = mlib_ImageGetStride( src );
  dstride = mlib_ImageGetStride( dst );
  schann = mlib_ImageGetChannels( src );
  dchann = mlib_ImageGetChannels( dst );

  if( stype != s->intype || dtype != s->outtype )
    return MLIB_FAILURE;

  if( channels != schann )
    return MLIB_FAILURE;

  switch( stype )
  {
    case MLIB_BYTE:
    {
      mlib_u8 *sdata = mlib_ImageGetData( src );

      switch( dtype )
      {
        case MLIB_BYTE:
        {
          mlib_u8 *ddata = mlib_ImageGetData( dst );

          switch( channels )
          {
            case 3:
            {
              MAIN_COLORTRUE2INDEX_LOOP( U8, U8, 3 );
              return MLIB_SUCCESS;
            }
            case 4:
            {
              MAIN_COLORTRUE2INDEX_LOOP( U8, U8, 4 );
              return MLIB_SUCCESS;
            }
            default:
              return MLIB_FAILURE;
          }
        }
        case MLIB_SHORT:
        {
          mlib_s16 *ddata = mlib_ImageGetData( dst );

          dstride /= 2;
          switch( channels )
          {
            case 3:
            {
              MAIN_COLORTRUE2INDEX_LOOP( U8, S16, 3 );
              return MLIB_SUCCESS;
            }
            case 4:
            {
              MAIN_COLORTRUE2INDEX_LOOP( U8, S16, 4 );
              return MLIB_SUCCESS;
            }
            default:
              return MLIB_FAILURE;
          }
        }
      }
      break;
    }
    case MLIB_SHORT:
    {
      mlib_s16 *sdata = mlib_ImageGetData( src );

      sstride /= 2;
      switch( dtype )
      {
        case MLIB_BYTE:
        {
          mlib_u8 *ddata = mlib_ImageGetData( dst );

          switch( channels )
          {
            case 3:
            {
              MAIN_COLORTRUE2INDEX_LOOP( S16, U8, 3 );
              return MLIB_SUCCESS;
            }
            case 4:
            {
              MAIN_COLORTRUE2INDEX_LOOP( S16, U8, 4 );
              return MLIB_SUCCESS;
            }
            default:
              return MLIB_FAILURE;
          }
        }
        case MLIB_SHORT:
        {
          mlib_s16 *ddata = mlib_ImageGetData( dst );

          dstride /= 2;
          switch( channels )
          {
            case 3:
            {
              MAIN_COLORTRUE2INDEX_LOOP( S16, S16, 3 );
              return MLIB_SUCCESS;
            }
            case 4:
            {
              MAIN_COLORTRUE2INDEX_LOOP( S16, S16, 4 );
              return MLIB_SUCCESS;
            }
            default:
              return MLIB_FAILURE;
          }
        }
      }
      break;
    }
    default:
      return MLIB_FAILURE;
  }
}

/***************************************************************/ 

#ifdef MLIB_TEST

void mlib_v_ImageColorTrue2IndexFree( void *colormap )

#else

void mlib_ImageColorTrue2IndexFree( void *colormap )

#endif
{
  if( colormap )
  {
    mlib_colormap *s = colormap;
    mlib_type type = s->intype;
    void *table = s->table;
    mlib_s32 method = s->method;

    switch( method )
    {
      case LUT_COLOR_CUBE_SEARCH:
      {
        mlib_free( table );
        break;
      }
      case LUT_BINARY_TREE_SEARCH:
      {
        switch( s->channels )
        {
          case 3:
          {
            mlib_free_binary_tree_3( table );
            break;
          }
          case 4:
          {
            mlib_free_binary_tree_4( table );
            break;
          }
        }
        break;
      }
    }

    mlib_free( s->normal_table );
    mlib_free( s->double_lut );
    mlib_free( colormap );
  }
}

/***************************************************************/ 

void *mlib_color_cube_init_U8_3( mlib_colormap *state )
{
  mlib_s32 offset = state->offset;

  mlib_s32 bits = state->bits;
  mlib_s32 nbits = 8 - bits;
  mlib_u32 *dist_buf;
  mlib_s32 size;
  mlib_u8 *base0, *base1, *base2;
  mlib_s32 cindex;

  mlib_s32 center0, center1, center2;
  mlib_u32 dist0, dist1, cdist;
  mlib_s32 cinc0, cinc1, cinc2;
  mlib_u32 *dp0, *dp1, *cdp, *cdp1;
  mlib_s32 stride0, stride1;
  mlib_s32 x, xsqr, colormax;
  mlib_s32 xx0, xx1, xx2;
  mlib_s32 inc1, inc2, cdist1;
  mlib_s32 here1, min1, max1, prevmin1, prevmax1;
  mlib_s32 here2, min2, max2, prevmin2, prevmax2;
  mlib_s32 dmin2, dmax2;
  mlib_s32 txsqr;
  mlib_s32 c0, c1, c2;
  mlib_d64 doublefirst;
  mlib_s32 memsize;

  size = ( 1 << bits ) * ( 1 << bits ) * ( 1 << bits );
  memsize = size * state->indexsize;

  dist_buf = mlib_malloc( size * 4 );
  if( !dist_buf )
    return NULL;

  base0 = state->lut[ 0 ];
  base1 = state->lut[ 1 ];
  base2 = state->lut[ 2 ];

  colormax = 1 << bits;
  x = 1 << nbits;
  xsqr = 1 << ( 2 * nbits );
  txsqr = xsqr + xsqr;
  stride0 = colormax * colormax;
  stride1 = colormax;

  switch( state->indexsize )
  {
    case 1:
    {
      mlib_u8 *table;

      table = mlib_malloc( memsize );
      if( !table )
      {
        mlib_free( dist_buf );
        return NULL;
      }

      ( ( mlib_u8* )&doublefirst )[ 0 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 1 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 2 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 3 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 4 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 5 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 6 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 7 ] = offset;

      COLOR_CUBE_3_INIT_MAIN_LOOP( mlib_u8, 0, 0 );

      mlib_free( dist_buf );
      return table;
    }
    case 2:
    {
      mlib_s16 *table;

      table = mlib_malloc( memsize  );
      if( !table )
      {
        mlib_free( dist_buf );
        return NULL;
      }

      ( ( mlib_s16* )&doublefirst )[ 0 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 1 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 2 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 3 ] = offset;

      COLOR_CUBE_3_INIT_MAIN_LOOP( mlib_s16, 0, 0 );

      mlib_free( dist_buf );
      return table;
    }
  }
}

/***************************************************************/ 

void *mlib_color_cube_init_U8_4( mlib_colormap *state )
{
  mlib_s32 offset = state->offset;

  mlib_s32 bits = state->bits;
  mlib_s32 nbits = 8 - bits;
  mlib_u32 *dist_buf;
  mlib_s32 size;
  mlib_u8 *base0, *base1, *base2, *base3;
  mlib_s32 cindex;

  mlib_s32 center0, center1, center2, center3;
  mlib_u32 dist0, dist1, dist2, cdist;
  mlib_s32 cinc0, cinc1, cinc2, cinc3;
  mlib_u32 *dp0, *dp1, *dp2, *cdp, *cdp1, *cdp2;
  mlib_s32 stride0, stride1, stride2;
  mlib_s32 x, xsqr, colormax;
  mlib_s32 xx0, xx1, xx2, xx3;
  mlib_s32 inc1, inc2, inc3, cdist1, cdist2;
  mlib_s32 here1, min1, max1, prevmin1, prevmax1;
  mlib_s32 here2, min2, max2, prevmin2, prevmax2;
  mlib_s32 here3, min3, max3, prevmin3, prevmax3;
  mlib_s32 dmin3, dmax3;
  mlib_s32 txsqr;
  mlib_s32 c0, c1, c2, c3;
  mlib_d64 doublefirst;
  mlib_s32 memsize;

  size = ( 1 << bits ) * ( 1 << bits ) * ( 1 << bits ) * ( 1 << bits );
  memsize = size * state->indexsize;

  dist_buf = mlib_malloc( size * 4 );
  if ( !dist_buf )
    return NULL;

  base0 = state->lut[ 0 ];
  base1 = state->lut[ 1 ];
  base2 = state->lut[ 2 ];
  base3 = state->lut[ 3 ];

  colormax = 1 << bits;
  x = 1 << nbits;
  xsqr = 1 << ( 2 * nbits );
  txsqr = xsqr + xsqr;
  stride0 = colormax * colormax * colormax;
  stride1 = colormax * colormax;
  stride2 = colormax;

  switch( state->indexsize )
  {
    case 1:
    {
      mlib_u8 *table;

      table = mlib_malloc( memsize );
      if( !table )
      {
        mlib_free( dist_buf );
        return NULL;
      }

      ( ( mlib_u8* )&doublefirst )[ 0 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 1 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 2 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 3 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 4 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 5 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 6 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 7 ] = offset;

      COLOR_CUBE_4_INIT_MAIN_LOOP( mlib_u8, 0, 0 );

      mlib_free( dist_buf );
      return table;
    }
    case 2:
    {
      mlib_s16 *table;

      table = mlib_malloc( memsize );
      if( !table )
      {
        mlib_free( dist_buf );
        return NULL;
      }

      ( ( mlib_s16* )&doublefirst )[ 0 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 1 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 2 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 3 ] = offset;

      COLOR_CUBE_4_INIT_MAIN_LOOP( mlib_s16, 0, 0 );

      mlib_free( dist_buf );
      return table;
    }
  }
}

/***************************************************************/ 

void *mlib_color_cube_init_S16_3( mlib_colormap *state )
{
  mlib_s32 offset = state->offset;

  mlib_s32 bits = state->bits;
  mlib_s32 nbits = 16 - bits;
  mlib_u32 *dist_buf;
  mlib_s32 size;
  mlib_s16 *base0, *base1, *base2;
  mlib_s32 cindex;

  mlib_s32 center0, center1, center2;
  mlib_u32 dist0, dist1, cdist;
  mlib_s32 cinc0, cinc1, cinc2;
  mlib_u32 *dp0, *dp1, *cdp, *cdp1;
  mlib_s32 stride0, stride1;
  mlib_s32 x, xsqr, colormax;
  mlib_s32 xx0, xx1, xx2;
  mlib_s32 inc1, inc2, cdist1;
  mlib_s32 here1, min1, max1, prevmin1, prevmax1;
  mlib_s32 here2, min2, max2, prevmin2, prevmax2;
  mlib_s32 dmin2, dmax2;
  mlib_s32 txsqr;
  mlib_s32 c0, c1, c2;
  mlib_d64 doublefirst;
  mlib_s32 memsize;

  size = ( 1 << bits ) * ( 1 << bits ) * ( 1 << bits );
  memsize = size * state->indexsize;

  dist_buf = mlib_malloc( size * 4 );
  if ( !dist_buf )
    return NULL;

  base0 = state->lut[ 0 ];
  base1 = state->lut[ 1 ];
  base2 = state->lut[ 2 ];

  colormax = 1 << bits;
  x = 1 << ( nbits - 1 );
  xsqr = 1 << ( 2 * nbits - 2 );
  txsqr = xsqr + xsqr;
  stride0 = colormax * colormax;
  stride1 = colormax;

  switch( state->indexsize )
  {
    case 1:
    {
      mlib_u8 *table;

      table = mlib_malloc( memsize );
      if( !table )
      {
        mlib_free( dist_buf );
        return NULL;
      }

      ( ( mlib_u8* )&doublefirst )[ 0 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 1 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 2 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 3 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 4 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 5 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 6 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 7 ] = offset;

      COLOR_CUBE_3_INIT_MAIN_LOOP( mlib_u8, MLIB_S16_MIN, 1 );

      mlib_free( dist_buf );
      return table;
    }
    case 2:
    {
      mlib_s16 *table;

      table = mlib_malloc( memsize  );
      if( !table )
      {
        mlib_free( dist_buf );
        return NULL;
      }

      ( ( mlib_s16* )&doublefirst )[ 0 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 1 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 2 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 3 ] = offset;

      COLOR_CUBE_3_INIT_MAIN_LOOP( mlib_s16, MLIB_S16_MIN, 1 );

      mlib_free( dist_buf );
      return table;
    }
  }
}

/***************************************************************/ 

void *mlib_color_cube_init_S16_4( mlib_colormap *state )
{
  mlib_s32 offset = state->offset;

  mlib_s32 bits = state->bits;
  mlib_s32 nbits = 16 - bits;
  mlib_u32 *dist_buf;
  mlib_s32 size;
  mlib_s16 *base0, *base1, *base2, *base3;
  mlib_s32 cindex;

  mlib_s32 center0, center1, center2, center3;
  mlib_u32 dist0, dist1, dist2, cdist;
  mlib_s32 cinc0, cinc1, cinc2, cinc3;
  mlib_u32 *dp0, *dp1, *dp2, *cdp, *cdp1, *cdp2;
  mlib_s32 stride0, stride1, stride2;
  mlib_s32 x, xsqr, colormax;
  mlib_s32 xx0, xx1, xx2, xx3;
  mlib_s32 inc1, inc2, inc3, cdist1, cdist2;
  mlib_s32 here1, min1, max1, prevmin1, prevmax1;
  mlib_s32 here2, min2, max2, prevmin2, prevmax2;
  mlib_s32 here3, min3, max3, prevmin3, prevmax3;
  mlib_s32 dmin3, dmax3;
  mlib_s32 txsqr;
  mlib_s32 c0, c1, c2, c3;
  mlib_d64 doublefirst;
  mlib_s32 memsize;

  size = ( 1 << bits ) * ( 1 << bits ) * ( 1 << bits ) * ( 1 << bits );
  memsize = size * state->indexsize;

  dist_buf = mlib_malloc( size * 4 );
  if ( !dist_buf )
    return NULL;

  base0 = state->lut[ 0 ];
  base1 = state->lut[ 1 ];
  base2 = state->lut[ 2 ];
  base3 = state->lut[ 3 ];

  colormax = 1 << bits;
  x = 1 << ( nbits - 1 );
  xsqr = 1 << ( 2 * nbits - 2 );
  txsqr = xsqr + xsqr;
  stride0 = colormax * colormax * colormax;
  stride1 = colormax * colormax;
  stride2 = colormax;

  switch( state->indexsize )
  {
    case 1:
    {
      mlib_u8 *table;

      table = mlib_malloc( memsize );
      if( !table )
      {
        mlib_free( dist_buf );
        return NULL;
      }

      ( ( mlib_u8* )&doublefirst )[ 0 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 1 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 2 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 3 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 4 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 5 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 6 ] = offset;
      ( ( mlib_u8* )&doublefirst )[ 7 ] = offset;

      COLOR_CUBE_4_INIT_MAIN_LOOP( mlib_u8, MLIB_S16_MIN, 1 );

      mlib_free( dist_buf );
      return table;
    }
    case 2:
    {
      mlib_s16 *table;

      table = mlib_malloc( memsize );
      if( !table )
      {
        mlib_free( dist_buf );
        return NULL;
      }

      ( ( mlib_s16* )&doublefirst )[ 0 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 1 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 2 ] = offset;
      ( ( mlib_s16* )&doublefirst )[ 3 ] = offset;

      COLOR_CUBE_4_INIT_MAIN_LOOP( mlib_s16, MLIB_S16_MIN, 1 );

      mlib_free( dist_buf );
      return table;
    }
  }
}

/***************************************************************/ 

void *mlib_binary_tree_init_3( mlib_colormap *state )
{
  mlib_s32 offset = state->offset, bits = state->bits;
  struct lut_node_3 *table, *node;
  mlib_s32 i;

  switch( state->intype )
  {
    case MLIB_BYTE:
    {                                                                       
      BINARY_TREE_INIT_3( mlib_u8, 8, 0, 0 );
      break;
    }
    case MLIB_SHORT:
    {
      BINARY_TREE_INIT_3( mlib_s16, 16, MLIB_S16_MIN, 1 );
      break;
    }
  }

  return table;
}

/***************************************************************/ 

void *mlib_binary_tree_init_4( mlib_colormap *state )
{
  mlib_s32 offset = state->offset, bits = state->bits;
  struct lut_node_4 *table, *node;
  mlib_s32 i;

  switch( state->intype )
  {
    case MLIB_BYTE:
      BINARY_TREE_INIT_4( mlib_u8, 8, 0, 0 );
      break;
    case MLIB_SHORT:
      BINARY_TREE_INIT_4( mlib_s16, 16, MLIB_S16_MIN, 1 );
      break;
  }

  return table;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_U8_3( struct lut_node_3 *node,
                                    mlib_u32 distance, mlib_s32 *found_color,
                                    mlib_u32 c0, mlib_u32 c1, mlib_u32 c2, mlib_u8 **base )
{
  int i;

  for( i = 0; i < 8; i++ )
  {
    if( node->tag & ( 1 << i ) )
    {
      /* Here is alone color cell. Check the distance */
      mlib_s32 newindex = node->contents.index[ i ];
      mlib_u32 newpalc0, newpalc1, newpalc2;
      mlib_u32 newdistance;

      newpalc0 = base[ 0 ][ newindex ];
      newpalc1 = base[ 1 ][ newindex ];
      newpalc2 = base[ 2 ][ newindex ];
      newdistance = FIND_DISTANCE_3( c0, newpalc0,
        c1, newpalc1, c2, newpalc2, 0 );
      if( distance > newdistance )
      {
        *found_color = newindex;
        distance = newdistance;
      }
    }
    else if( node->contents.quadrants[ i ] )
      distance = mlib_search_quadrant_U8_3(
        node->contents.quadrants[ i ], distance, found_color,
        c0, c1, c2, base );
  }

  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_part_to_left_U8_3( struct lut_node_3 *node,
                                                 mlib_u32 distance, mlib_s32 *found_color,
                                                 mlib_u32 *c, mlib_u8 **base, mlib_u32 position,
                                                 mlib_s32 pass, mlib_s32 dir_bit )
{
  mlib_u32 current_size = 1 << pass;
  mlib_s32 i;
  mlib_s32 q = 1 << dir_bit;
  static mlib_s32 opposite_quadrants[ 3 ][ 4 ] = {
    { 0, 2, 4, 6 },
    { 0, 1, 4, 5 },
    { 0, 1, 2, 3 }
  };

/* Search only quadrant's half untill it is necessary to check the
  whole quadrant */

  if( distance < ( position + current_size - c[ dir_bit ] ) *
    ( position + current_size - c[ dir_bit ] ) )
  { /* Search half of quadrant */
    for( i = 0; i < 4; i++ )
    {
      mlib_s32 qq = opposite_quadrants[ dir_bit ][ i ];
      if( node->tag & ( 1 << qq ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ qq ];
        mlib_u32 newpalc0, newpalc1, newpalc2;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ];
        newpalc1 = base[ 1 ][ newindex ];
        newpalc2 = base[ 2 ][ newindex ];
        newdistance = FIND_DISTANCE_3( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, 0 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ qq ] )
        distance = mlib_search_quadrant_part_to_left_U8_3(
          node->contents.quadrants[ qq ], distance, found_color,
          c, base, position, pass - 1, dir_bit );
    }
  }
  else /* Search whole quadrant */
  {
    mlib_s32 mask = 1 << dir_bit;

    for( i = 0; i < 8; i++ )
    {
      if( node->tag & ( 1 << i ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ i ];
        mlib_u32 newpalc0, newpalc1, newpalc2;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ];
        newpalc1 = base[ 1 ][ newindex ];
        newpalc2 = base[ 2 ][ newindex ];
        newdistance = FIND_DISTANCE_3( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, 0 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ i ] )
      {
        if( i & mask )
          /* This quadrant may require partial checking */
          distance = mlib_search_quadrant_part_to_left_U8_3(
            node->contents.quadrants[ i ], distance, found_color,
            c, base, position + current_size, pass - 1, dir_bit );
        else
          /* Here we should check all */
          distance = mlib_search_quadrant_U8_3(
            node->contents.quadrants[ i ], distance, found_color,
            c[ 0 ], c[ 1 ], c[ 2 ], base );
      }
    }
  }

  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_part_to_right_U8_3( struct lut_node_3 *node,
                                                  mlib_u32 distance, mlib_s32 *found_color,
                                                  mlib_u32 *c, mlib_u8 **base, mlib_u32 position,
                                                  mlib_s32 pass, mlib_s32 dir_bit )
{
  mlib_u32 current_size = 1 << pass;
  mlib_s32 i;
  mlib_s32 q = 1 << dir_bit;
  static mlib_s32 opposite_quadrants[ 3 ][ 4 ] = {
    { 1, 3, 5, 7 },
    { 2, 3, 6, 7 },
    { 4, 5, 6, 7 }
  };

/* Search only quadrant's half untill it is necessary to check the
  whole quadrant */

  if( distance <= ( c[ dir_bit ] - position - current_size ) *
    ( c[ dir_bit ] - position - current_size ) )
  { /* Search half of quadrant */
    for( i = 0; i < 4; i++ )
    {
      mlib_s32 qq = opposite_quadrants[ dir_bit ][ i ];
      if( node->tag & ( 1 << qq ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ qq ];
        mlib_u32 newpalc0, newpalc1, newpalc2;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ];
        newpalc1 = base[ 1 ][ newindex ];
        newpalc2 = base[ 2 ][ newindex ];
        newdistance = FIND_DISTANCE_3( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, 0 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ qq ] )
        distance = mlib_search_quadrant_part_to_right_U8_3(
          node->contents.quadrants[ qq ], distance, found_color,
          c, base, position + current_size, pass - 1, dir_bit );
    }
  }
  else /* Search whole quadrant */
  {
    mlib_s32 mask = 1 << dir_bit;

    for( i = 0; i < 8; i++ )
    {
      if( node->tag & ( 1 << i ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ i ];
        mlib_u32 newpalc0, newpalc1, newpalc2;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ];
        newpalc1 = base[ 1 ][ newindex ];
        newpalc2 = base[ 2 ][ newindex ];
        newdistance = FIND_DISTANCE_3( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, 0 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ i ] )
      {
        if( i & mask )
          /* Here we should check all */
          distance = mlib_search_quadrant_U8_3(
            node->contents.quadrants[ i ], distance, found_color,
            c[ 0 ], c[ 1 ], c[ 2 ], base );
        else
          /* This quadrant may require partial checking */
          distance = mlib_search_quadrant_part_to_right_U8_3(
            node->contents.quadrants[ i ], distance, found_color,
            c, base, position, pass - 1, dir_bit );
      }
    }
  }
  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_S16_3( struct lut_node_3 *node,
                                     mlib_u32 distance, mlib_s32 *found_color,
                                     mlib_u32 c0, mlib_u32 c1, mlib_u32 c2, mlib_s16 **base )
{
  int i;

  for( i = 0; i < 8; i++ )
  {
    if( node->tag & ( 1 << i ) )
    {
      /* Here is alone color cell. Check the distance */
      mlib_s32 newindex = node->contents.index[ i ];
      mlib_u32 newpalc0, newpalc1, newpalc2;
      mlib_u32 newdistance;

      newpalc0 = base[ 0 ][ newindex ] - MLIB_S16_MIN;
      newpalc1 = base[ 1 ][ newindex ] - MLIB_S16_MIN;
      newpalc2 = base[ 2 ][ newindex ] - MLIB_S16_MIN;
      newdistance = FIND_DISTANCE_3( c0, newpalc0,
        c1, newpalc1, c2, newpalc2, 2 );
      if( distance > newdistance )
      {
        *found_color = newindex;
        distance = newdistance;
      }
    }
    else if( node->contents.quadrants[ i ] )
      distance = mlib_search_quadrant_S16_3(
        node->contents.quadrants[ i ], distance, found_color,
        c0, c1, c2, base );
  }
  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_part_to_left_S16_3( struct lut_node_3 *node,
                                                  mlib_u32 distance, mlib_s32 *found_color,
                                                  mlib_u32 *c, mlib_s16 **base, mlib_u32 position,
                                                  mlib_s32 pass, mlib_s32 dir_bit )
{
  mlib_u32 current_size = 1 << pass;
  mlib_s32 i;
  mlib_s32 q = 1 << dir_bit;
  static mlib_s32 opposite_quadrants[ 3 ][ 4 ] = {
    { 0, 2, 4, 6 },
    { 0, 1, 4, 5 },
    { 0, 1, 2, 3 }
  };

/* Search only quadrant's half untill it is necessary to check the
  whole quadrant */

  if( distance < ( ( ( position + current_size - c[ dir_bit ] ) *
    ( position + current_size - c[ dir_bit ] ) ) >> 2 ))
  { /* Search half of quadrant */
    for( i = 0; i < 4; i++ )
    {
      mlib_s32 qq = opposite_quadrants[ dir_bit ][ i ];
      if( node->tag & ( 1 << qq ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ qq ];
        mlib_u32 newpalc0, newpalc1, newpalc2;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ] - MLIB_S16_MIN;
        newpalc1 = base[ 1 ][ newindex ] - MLIB_S16_MIN;
        newpalc2 = base[ 2 ][ newindex ] - MLIB_S16_MIN;
        newdistance = FIND_DISTANCE_3( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, 2 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ qq ] )
        distance = mlib_search_quadrant_part_to_left_S16_3(
          node->contents.quadrants[ qq ], distance, found_color,
          c, base, position, pass - 1, dir_bit );
    }
  }
  else /* Search whole quadrant */
  {
    mlib_s32 mask = 1 << dir_bit;

    for( i = 0; i < 8; i++ )
    {
      if( node->tag & ( 1 << i ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ i ];
        mlib_u32 newpalc0, newpalc1, newpalc2;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ] - MLIB_S16_MIN;
        newpalc1 = base[ 1 ][ newindex ] - MLIB_S16_MIN;
        newpalc2 = base[ 2 ][ newindex ] - MLIB_S16_MIN;
        newdistance = FIND_DISTANCE_3( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, 2 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ i ] )
      {
        if( i & mask )
          /* This quadrant may require partial checking */
          distance = mlib_search_quadrant_part_to_left_S16_3(
            node->contents.quadrants[ i ], distance, found_color,
            c, base, position + current_size, pass - 1, dir_bit );
        else
          /* Here we should check all */
          distance = mlib_search_quadrant_S16_3(
            node->contents.quadrants[ i ], distance, found_color,
            c[ 0 ], c[ 1 ], c[ 2 ], base );
      }
    }
  }
  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_part_to_right_S16_3( struct lut_node_3 *node,
                                                   mlib_u32 distance, mlib_s32 *found_color,
                                                   mlib_u32 *c, mlib_s16 **base, mlib_u32 position,
                                                   mlib_s32 pass, mlib_s32 dir_bit )
{
  mlib_u32 current_size = 1 << pass;
  mlib_s32 i;
  mlib_s32 q = 1 << dir_bit;
  static mlib_s32 opposite_quadrants[ 3 ][ 4 ] = {
    { 1, 3, 5, 7 },
    { 2, 3, 6, 7 },
    { 4, 5, 6, 7 }
  };

/* Search only quadrant's half untill it is necessary to check the
  whole quadrant */

  if( distance <= ( ( ( c[ dir_bit ] - position - current_size ) *
    ( c[ dir_bit ] - position - current_size ) ) >> 2 ) )
  { /* Search half of quadrant */
    for( i = 0; i < 4; i++ )
    {
      mlib_s32 qq = opposite_quadrants[ dir_bit ][ i ];
      if( node->tag & ( 1 << qq ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ qq ];
        mlib_u32 newpalc0, newpalc1, newpalc2;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ] - MLIB_S16_MIN;
        newpalc1 = base[ 1 ][ newindex ] - MLIB_S16_MIN;
        newpalc2 = base[ 2 ][ newindex ] - MLIB_S16_MIN;
        newdistance = FIND_DISTANCE_3( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, 2 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ qq ] )
        distance = mlib_search_quadrant_part_to_right_S16_3(
          node->contents.quadrants[ qq ], distance, found_color,
          c, base, position + current_size, pass - 1, dir_bit );
    }
  }
  else /* Search whole quadrant */
  {
    mlib_s32 mask = 1 << dir_bit;

    for( i = 0; i < 8; i++ )
    {
      if( node->tag & ( 1 << i ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ i ];
        mlib_u32 newpalc0, newpalc1, newpalc2;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ] - MLIB_S16_MIN;
        newpalc1 = base[ 1 ][ newindex ] - MLIB_S16_MIN;
        newpalc2 = base[ 2 ][ newindex ] - MLIB_S16_MIN;
        newdistance = FIND_DISTANCE_3( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, 2 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ i ] )
      {
        if( i & mask )
          /* Here we should check all */
          distance = mlib_search_quadrant_S16_3(
            node->contents.quadrants[ i ], distance, found_color,
            c[ 0 ], c[ 1 ], c[ 2 ], base );
        else
          /* This quadrant may require partial checking */
          distance = mlib_search_quadrant_part_to_right_S16_3(
            node->contents.quadrants[ i ], distance, found_color,
            c, base, position, pass - 1, dir_bit );
      }
    }
  }

  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_U8_4( struct lut_node_4 *node,
                                    mlib_u32 distance, mlib_s32 *found_color,
                                    mlib_u32 c0, mlib_u32 c1, mlib_u32 c2, mlib_u32 c3,
                                    mlib_u8 **base )
{
  int i;

  for( i = 0; i < 16; i++ )
  {
    if( node->tag & ( 1 << i ) )
    {
      /* Here is alone color cell. Check the distance */
      mlib_s32 newindex = node->contents.index[ i ];
      mlib_u32 newpalc0, newpalc1, newpalc2, newpalc3;
      mlib_u32 newdistance;

      newpalc0 = base[ 0 ][ newindex ];
      newpalc1 = base[ 1 ][ newindex ];
      newpalc2 = base[ 2 ][ newindex ];
      newpalc3 = base[ 3 ][ newindex ];
      newdistance = FIND_DISTANCE_4( c0, newpalc0,
        c1, newpalc1, c2, newpalc2, c3, newpalc3, 0 );
      if( distance > newdistance )
      {
        *found_color = newindex;
        distance = newdistance;
      }
    }
    else if( node->contents.quadrants[ i ] )
      distance = mlib_search_quadrant_U8_4(
        node->contents.quadrants[ i ], distance, found_color,
        c0, c1, c2, c3, base );
  }

  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_part_to_left_U8_4( struct lut_node_4 *node,
                                                 mlib_u32 distance, mlib_s32 *found_color,
                                                 mlib_u32 *c, mlib_u8 **base, mlib_u32 position,
                                                 mlib_s32 pass, mlib_s32 dir_bit )
{
  mlib_u32 current_size = 1 << pass;
  mlib_s32 i;
  mlib_s32 q = 1 << dir_bit;
  static mlib_s32 opposite_quadrants[ 4 ][ 8 ] = {
    { 0, 2, 4, 6, 8, 10, 12, 14 },
    { 0, 1, 4, 5, 8, 9, 12, 13 },
    { 0, 1, 2, 3, 8, 9, 10, 11 },
    { 0, 1, 2, 3, 4, 5, 6, 7 }
  };

/* Search only quadrant's half untill it is necessary to check the
  whole quadrant */

  if( distance < ( position + current_size - c[ dir_bit ] ) *
    ( position + current_size - c[ dir_bit ] ) )
  { /* Search half of quadrant */
    for( i = 0; i < 8; i++ )
    {
      mlib_s32 qq = opposite_quadrants[ dir_bit ][ i ];
      if( node->tag & ( 1 << qq ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ qq ];
        mlib_u32 newpalc0, newpalc1, newpalc2, newpalc3;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ];
        newpalc1 = base[ 1 ][ newindex ];
        newpalc2 = base[ 2 ][ newindex ];
        newpalc3 = base[ 3 ][ newindex ];
        newdistance = FIND_DISTANCE_4( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, c[ 3 ], newpalc3, 0 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ qq ] )
        distance = mlib_search_quadrant_part_to_left_U8_4(
          node->contents.quadrants[ qq ], distance, found_color,
          c, base, position, pass - 1, dir_bit );
    }
  }
  else /* Search whole quadrant */
  {
    mlib_s32 mask = 1 << dir_bit;

    for( i = 0; i < 16; i++ )
    {
      if( node->tag & ( 1 << i ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ i ];
        mlib_u32 newpalc0, newpalc1, newpalc2, newpalc3;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ];
        newpalc1 = base[ 1 ][ newindex ];
        newpalc2 = base[ 2 ][ newindex ];
        newpalc3 = base[ 3 ][ newindex ];
        newdistance = FIND_DISTANCE_4( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, c[ 3 ], newpalc3, 0 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ i ] )
      {
        if( i & mask )
          /* This quadrant may require partial checking */
          distance = mlib_search_quadrant_part_to_left_U8_4(
            node->contents.quadrants[ i ], distance, found_color,
            c, base, position + current_size, pass - 1, dir_bit );
        else
          /* Here we should check all */
          distance = mlib_search_quadrant_U8_4(
            node->contents.quadrants[ i ], distance, found_color,
            c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], base );
      }
    }
  }
  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_part_to_right_U8_4( struct lut_node_4 *node,
                                                  mlib_u32 distance, mlib_s32 *found_color,
                                                  mlib_u32 *c, mlib_u8 **base, mlib_u32 position,
                                                  mlib_s32 pass, mlib_s32 dir_bit )
{
  mlib_u32 current_size = 1 << pass;
  mlib_s32 i;
  mlib_s32 q = 1 << dir_bit;
  static mlib_s32 opposite_quadrants[ 4 ][ 8 ] = {
    { 1, 3, 5, 7, 9, 11, 13, 15 },
    { 2, 3, 6, 7, 10, 11, 14, 15 },
    { 4, 5, 6, 7, 12, 13, 14, 15 },
    { 8, 9, 10, 11, 12, 13, 14, 15 }
  };

/* Search only quadrant's half untill it is necessary to check the
  whole quadrant */

  if( distance <= ( c[ dir_bit ] - position - current_size ) *
    ( c[ dir_bit ] - position - current_size ) )
  { /* Search half of quadrant */
    for( i = 0; i < 8; i++ )
    {
      mlib_s32 qq = opposite_quadrants[ dir_bit ][ i ];
      if( node->tag & ( 1 << qq ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ qq ];
        mlib_u32 newpalc0, newpalc1, newpalc2, newpalc3;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ];
        newpalc1 = base[ 1 ][ newindex ];
        newpalc2 = base[ 2 ][ newindex ];
        newpalc3 = base[ 3 ][ newindex ];
        newdistance = FIND_DISTANCE_4( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, c[ 3 ], newpalc3, 0 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ qq ] )
        distance = mlib_search_quadrant_part_to_right_U8_4(
          node->contents.quadrants[ qq ], distance, found_color,
          c, base, position + current_size, pass - 1, dir_bit );
    }
  }
  else /* Search whole quadrant */
  {
    mlib_s32 mask = 1 << dir_bit;

    for( i = 0; i < 16; i++ )
    {
      if( node->tag & ( 1 << i ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ i ];
        mlib_u32 newpalc0, newpalc1, newpalc2, newpalc3;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ];
        newpalc1 = base[ 1 ][ newindex ];
        newpalc2 = base[ 2 ][ newindex ];
        newpalc3 = base[ 3 ][ newindex ];
        newdistance = FIND_DISTANCE_4( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, c[ 3 ], newpalc3, 0 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ i ] )
      {
        if( i & mask )
          /* Here we should check all */
          distance = mlib_search_quadrant_U8_4(
            node->contents.quadrants[ i ], distance, found_color,
            c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], base );
        else
          /* This quadrant may require partial checking */
          distance = mlib_search_quadrant_part_to_right_U8_4(
            node->contents.quadrants[ i ], distance, found_color,
            c, base, position, pass - 1, dir_bit );
      }
    }
  }
  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_S16_4( struct lut_node_4 *node,
                                     mlib_u32 distance, mlib_s32 *found_color,
                                     mlib_u32 c0, mlib_u32 c1, mlib_u32 c2, mlib_u32 c3,
                                     mlib_s16 **base )
{
  int i;

  for( i = 0; i < 16; i++ )
  {
    if( node->tag & ( 1 << i ) )
    {
      /* Here is alone color cell. Check the distance */
      mlib_s32 newindex = node->contents.index[ i ];
      mlib_u32 newpalc0, newpalc1, newpalc2, newpalc3;
      mlib_u32 newdistance;

      newpalc0 = base[ 0 ][ newindex ] - MLIB_S16_MIN;
      newpalc1 = base[ 1 ][ newindex ] - MLIB_S16_MIN;
      newpalc2 = base[ 2 ][ newindex ] - MLIB_S16_MIN;
      newpalc3 = base[ 3 ][ newindex ] - MLIB_S16_MIN;
      newdistance = FIND_DISTANCE_4( c0, newpalc0,
        c1, newpalc1, c2, newpalc2, c3, newpalc3, 2 );
      if( distance > newdistance )
      {
        *found_color = newindex;
        distance = newdistance;
      }
    }
    else if( node->contents.quadrants[ i ] )
      distance = mlib_search_quadrant_S16_4(
        node->contents.quadrants[ i ], distance, found_color,
        c0, c1, c2, c3, base );
  }
  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_part_to_left_S16_4( struct lut_node_4 *node,
                                                  mlib_u32 distance, mlib_s32 *found_color,
                                                  mlib_u32 *c, mlib_s16 **base, mlib_u32 position,
                                                  mlib_s32 pass, mlib_s32 dir_bit )
{
  mlib_u32 current_size = 1 << pass;
  mlib_s32 i;
  mlib_s32 q = 1 << dir_bit;
  static mlib_s32 opposite_quadrants[ 4 ][ 8 ] = {
    { 0, 2, 4, 6, 8, 10, 12, 14 },
    { 0, 1, 4, 5, 8, 9, 12, 13 },
    { 0, 1, 2, 3, 8, 9, 10, 11 },
    { 0, 1, 2, 3, 4, 5, 6, 7 }
  };

/* Search only quadrant's half untill it is necessary to check the
  whole quadrant */

  if( distance < ( ( ( position + current_size - c[ dir_bit ] ) *
    ( position + current_size - c[ dir_bit ] ) ) >> 2 ) )
  { /* Search half of quadrant */
    for( i = 0; i < 8; i++ )
    {
      mlib_s32 qq = opposite_quadrants[ dir_bit ][ i ];
      if( node->tag & ( 1 << qq ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ qq ];
        mlib_u32 newpalc0, newpalc1, newpalc2, newpalc3;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ] - MLIB_S16_MIN;
        newpalc1 = base[ 1 ][ newindex ] - MLIB_S16_MIN;
        newpalc2 = base[ 2 ][ newindex ] - MLIB_S16_MIN;
        newpalc3 = base[ 3 ][ newindex ] - MLIB_S16_MIN;
        newdistance = FIND_DISTANCE_4( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, c[ 3 ], newpalc3, 2 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ qq ] )
        distance = mlib_search_quadrant_part_to_left_S16_4(
          node->contents.quadrants[ qq ], distance, found_color,
          c, base, position, pass - 1, dir_bit );
    }
  }
  else /* Search whole quadrant */
  {
    mlib_s32 mask = 1 << dir_bit;

    for( i = 0; i < 16; i++ )
    {
      if( node->tag & ( 1 << i ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ i ];
        mlib_u32 newpalc0, newpalc1, newpalc2, newpalc3;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ] - MLIB_S16_MIN;
        newpalc1 = base[ 1 ][ newindex ] - MLIB_S16_MIN;
        newpalc2 = base[ 2 ][ newindex ] - MLIB_S16_MIN;
        newpalc3 = base[ 3 ][ newindex ] - MLIB_S16_MIN;
        newdistance = FIND_DISTANCE_4( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, c[ 3 ], newpalc3, 2 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ i ] )
      {
        if( i & mask )
          /* This quadrant may require partial checking */
          distance = mlib_search_quadrant_part_to_left_S16_4(
            node->contents.quadrants[ i ], distance, found_color,
            c, base, position + current_size, pass - 1, dir_bit );
        else
          /* Here we should check all */
          distance = mlib_search_quadrant_S16_4(
            node->contents.quadrants[ i ], distance, found_color,
            c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], base );
      }
    }
  }

  return distance;
}

/***************************************************************/ 

mlib_u32 mlib_search_quadrant_part_to_right_S16_4( struct lut_node_4 *node,
                                                   mlib_u32 distance, mlib_s32 *found_color,
                                                   mlib_u32 *c, mlib_s16 **base, mlib_u32 position,
                                                   mlib_s32 pass, mlib_s32 dir_bit )
{
  mlib_u32 current_size = 1 << pass;
  mlib_s32 i;
  mlib_s32 q = 1 << dir_bit;
  static mlib_s32 opposite_quadrants[ 4 ][ 8 ] = {
    { 1, 3, 5, 7, 9, 11, 13, 15 },
    { 2, 3, 6, 7, 10, 11, 14, 15 },
    { 4, 5, 6, 7, 12, 13, 14, 15 },
    { 8, 9, 10, 11, 12, 13, 14, 15 }
  };

/* Search only quadrant's half untill it is necessary to check the
  whole quadrant */

  if( distance <= ( ( ( c[ dir_bit ] - position - current_size ) *
    ( c[ dir_bit ] - position - current_size ) ) >> 2 ) )
  { /* Search half of quadrant */
    for( i = 0; i < 8; i++ )
    {
      mlib_s32 qq = opposite_quadrants[ dir_bit ][ i ];
      if( node->tag & ( 1 << qq ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ qq ];
        mlib_u32 newpalc0, newpalc1, newpalc2, newpalc3;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ] - MLIB_S16_MIN;
        newpalc1 = base[ 1 ][ newindex ] - MLIB_S16_MIN;
        newpalc2 = base[ 2 ][ newindex ] - MLIB_S16_MIN;
        newpalc3 = base[ 3 ][ newindex ] - MLIB_S16_MIN;
        newdistance = FIND_DISTANCE_4( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, c[ 3 ], newpalc3, 2 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ qq ] )
        distance = mlib_search_quadrant_part_to_right_S16_4(
          node->contents.quadrants[ qq ], distance, found_color,
          c, base, position + current_size, pass - 1, dir_bit );
    }
  }
  else /* Search whole quadrant */
  {
    mlib_s32 mask = 1 << dir_bit;

    for( i = 0; i < 16; i++ )
    {
      if( node->tag & ( 1 << i ) )
      {
        /* Here is alone color cell. Check the distance */
        mlib_s32 newindex = node->contents.index[ i ];
        mlib_u32 newpalc0, newpalc1, newpalc2, newpalc3;
        mlib_u32 newdistance;

        newpalc0 = base[ 0 ][ newindex ] - MLIB_S16_MIN;
        newpalc1 = base[ 1 ][ newindex ] - MLIB_S16_MIN;
        newpalc2 = base[ 2 ][ newindex ] - MLIB_S16_MIN;
        newpalc3 = base[ 3 ][ newindex ] - MLIB_S16_MIN;
        newdistance = FIND_DISTANCE_4( c[ 0 ], newpalc0,
          c[ 1 ], newpalc1, c[ 2 ], newpalc2, c[ 3 ], newpalc3, 2 );
        if( distance > newdistance )
        {
          *found_color = newindex;
          distance = newdistance;
        }
      }
      else if( node->contents.quadrants[ i ] )
      {
        if( i & mask )
          /* Here we should check all */
          distance = mlib_search_quadrant_S16_4(
            node->contents.quadrants[ i ], distance, found_color,
            c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], base );
        else
          /* This quadrant may require partial checking */
          distance = mlib_search_quadrant_part_to_right_S16_4(
            node->contents.quadrants[ i ], distance, found_color,
            c, base, position, pass - 1, dir_bit );
      }
    }
  }
  return distance;
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_U8_U8_3(
                                           mlib_u8 *src,
                                           mlib_u8 *dst,
                                           mlib_s32 length,
                                           void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_3( U8, mlib_u8, 8, ( MLIB_U8_MAX + 1 ), 0, 0, 3, 0 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      COLOR_CUBE_U8_3_SEARCH( mlib_u8, 0, 3 );
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_U8_3;
      break;
    }
  }
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_U8_U8_3_in_4(
                                                mlib_u8 *src,
                                                mlib_u8 *dst,
                                                mlib_s32 length,
                                                void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_3( U8, mlib_u8, 8, ( MLIB_U8_MAX + 1 ), 0, 1, 4, 0 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      COLOR_CUBE_U8_3_SEARCH( mlib_u8, 1, 4 );
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_U8_3_IN4;
      break;
    }
  }
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_U8_U8_4(
                                           mlib_u8 *src,
                                           mlib_u8 *dst,
                                           mlib_s32 length,
                                           void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_4( U8, mlib_u8, 8, ( MLIB_U8_MAX + 1 ), 0, 0 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      COLOR_CUBE_U8_4_SEARCH( mlib_u8 );
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_U8_4;
      break;
    }
  }
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_U8_S16_3(
                                            mlib_u8  *src,
                                            mlib_s16 *dst,
                                            mlib_s32 length,
                                            void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_3( U8, mlib_u8, 8, ( MLIB_U8_MAX + 1 ), 0, 0, 3, 0 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      switch( s->indexsize )
      {
        case 1:
        {
          COLOR_CUBE_U8_3_SEARCH( mlib_u8, 0, 3 );
          break;
        }
        case 2:
        {
          COLOR_CUBE_U8_3_SEARCH( mlib_s16, 0, 3 );
          break;
        }
      }
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_U8_3;
      break;
    }
  }
}
/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_U8_S16_3_in_4(
                                                 mlib_u8  *src,
                                                 mlib_s16 *dst,
                                                 mlib_s32 length,
                                                 void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_3( U8, mlib_u8, 8, ( MLIB_U8_MAX + 1 ), 0, 1, 4, 0 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      switch( s->indexsize )
      {
        case 1:
        {
          COLOR_CUBE_U8_3_SEARCH( mlib_u8, 1, 4 );
          break;
        }
        case 2:
        {
          COLOR_CUBE_U8_3_SEARCH( mlib_s16, 1, 4 );
          break;
        }
      }
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_U8_3_IN4;
      break;
    }
  }
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_U8_S16_4(
                                            mlib_u8  *src,
                                            mlib_s16 *dst,
                                            mlib_s32 length,
                                            void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_4( U8, mlib_u8, 8, ( MLIB_U8_MAX + 1 ), 0, 0 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      switch( s->indexsize )
      {
        case 1:
        {
          COLOR_CUBE_U8_4_SEARCH( mlib_u8 );
          break;
        }
        case 2:
        {
          COLOR_CUBE_U8_4_SEARCH( mlib_s16 );
          break;
        }
      }
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_U8_4;
      break;
    }
  }
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_S16_S16_3(
                                             mlib_s16 *src,
                                             mlib_s16 *dst,
                                             mlib_s32 length,
                                             void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_3( S16, mlib_s16, 16, ( ( MLIB_S16_MAX + 1 ) * 2 ),
        MLIB_S16_MIN, 0, 3, 2 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      switch( s->indexsize )
      {
        case 1:
        {
          COLOR_CUBE_S16_3_SEARCH( mlib_u8, 0, 3 );
          break;
        }
        case 2:
        {
          COLOR_CUBE_S16_3_SEARCH( mlib_s16, 0, 3 );
          break;
        }
      }
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_S16_3( 0, 3 );
      break;
    }
  }
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_S16_S16_3_in_4(
                                                  mlib_s16 *src,
                                                  mlib_s16 *dst,
                                                  mlib_s32 length,
                                                  void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_3( S16, mlib_s16, 16, ( ( MLIB_S16_MAX + 1 ) * 2 ),
        MLIB_S16_MIN, 1, 4, 2 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      switch( s->indexsize )
      {
        case 1:
        {
          COLOR_CUBE_S16_3_SEARCH( mlib_u8, 1, 4 );
          break;
        }
        case 2:
        {
          COLOR_CUBE_S16_3_SEARCH( mlib_s16, 1, 4 );
          break;
        }
      }
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_S16_3( 1, 4 );
      break;
    }
  }
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_S16_S16_4(
                                             mlib_s16 *src,
                                             mlib_s16 *dst,
                                             mlib_s32 length,
                                             void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_4( S16, mlib_s16, 16, ( ( MLIB_S16_MAX + 1 ) * 2 ),
        MLIB_S16_MIN, 2 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      switch( s->indexsize )
      {
        case 1:
        {
          COLOR_CUBE_S16_4_SEARCH( mlib_u8 );
          break;
        }
        case 2:
        {
          COLOR_CUBE_S16_4_SEARCH( mlib_s16 );
          break;
        }
      }
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_S16_4;
      break;
    }
  }
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_S16_U8_3(
                                            mlib_s16 *src,
                                            mlib_u8  *dst,
                                            mlib_s32 length,
                                            void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_3( S16, mlib_s16, 16, ( ( MLIB_S16_MAX + 1 ) * 2 ),
        MLIB_S16_MIN, 0, 3, 2 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      COLOR_CUBE_S16_3_SEARCH( mlib_u8, 0, 3 );
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_S16_3( 0, 3 );
      break;
    }
  }
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_S16_U8_3_in_4(
                                                 mlib_s16 *src,
                                                 mlib_u8  *dst,
                                                 mlib_s32 length,
                                                 void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_3( S16, mlib_s16, 16, ( ( MLIB_S16_MAX + 1 ) * 2 ),
        MLIB_S16_MIN, 1, 4, 2 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      COLOR_CUBE_S16_3_SEARCH( mlib_u8, 1, 4 );
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_S16_3( 1, 4 );
      break;
    }
  }
}

/***************************************************************/ 

void mlib_ImageColorTrue2IndexLine_S16_U8_4(
                                            mlib_s16 *src,
                                            mlib_u8  *dst,
                                            mlib_s32 length,
                                            void *state )
{
  mlib_colormap *s = state;
  mlib_s32 bits = s->bits;

  switch( s->method )
  {
    case LUT_BINARY_TREE_SEARCH:
    {
      BINARY_TREE_SEARCH_4( S16, mlib_s16, 16, ( ( MLIB_S16_MAX + 1 ) * 2 ),
        MLIB_S16_MIN, 2 );
      break;
    }
    case LUT_COLOR_CUBE_SEARCH:
    {
      COLOR_CUBE_S16_4_SEARCH( mlib_u8 );
      break;
    }
    case LUT_STUPID_SEARCH:
    {
      FIND_NEAREST_S16_4;
    }
  }
}

/***************************************************************/ 

void mlib_free_binary_tree_3( void *table )
{
  struct lut_node_3 *node;

  node = table;
  if( node )
  {
    int j;
    mlib_s32 mask = 1;

    for( j = 0; j < 8; j++ )
    {
      if( !( node->tag & mask ) && node->contents.quadrants[ j ] )
        mlib_free_binary_tree_3( node->contents.quadrants[ j ] );
      mask <<= 1;
    }
    mlib_free( node );
  }
}

/***************************************************************/ 

void mlib_free_binary_tree_4( void *table )
{
  struct lut_node_4 *node;

  node = table;
  if( node )
  {
    int j;
    mlib_s32 mask = 1;

    for( j = 0; j < 16; j++ )
    {
      if( !( node->tag & mask ) && node->contents.quadrants[ j ] )
        mlib_free_binary_tree_4( node->contents.quadrants[ j ] );
      mask <<= 1;
    }
    mlib_free( node );
  }
}

/***************************************************************/ 

