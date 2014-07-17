/*
 * @(#)mlib_ImageCreate.c	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __SUNPRO_C
#pragma ident  "@(#)mlib_ImageCreate.c	1.31  00/01/06 SMI"
#endif /* __SUNPRO_C */

/*
 * FUNCTION
 *      mlib_ImageCreateStruct   - create image data structure
 *      mlib_ImageCreate         - create image data structure and allocate
 *                                 memory for image data
 *      mlib_ImageDelete         - delete image
 *      mlib_ImageCreateSubimage - create sub-image
 *
 *      mlib_ImageCreateRowTable - create row starts pointer table
 *      mlib_ImageDeleteRowTable - delete row starts pointer table
 *
 *      mlib_ImageSetBorder      - set the clipping box borders
 *
 * SYNOPSIS
 *        mlib_image *mlib_ImageCreateStruct(mlib_type type, 
 *                                           mlib_s32  channels, 
 *                                           mlib_s32  width, 
 *                                           mlib_s32  height,
 *                                           mlib_s32  stride, 
 *                                           void      *data)
 *
 *        mlib_image *mlib_ImageCreate(mlib_type type, 
 *                                     mlib_s32  channels, 
 *                                     mlib_s32  width, 
 *                                     mlib_s32  height)
 *
 *        void mlib_ImageDelete(mlib_image *img)
 *
 *        mlib_image *mlib_ImageCreateSubimage(mlib_image *img,
 *                                             mlib_s32 x, mlib_s32 y,
 *                                             mlib_s32 w, mlib_s32 h)
 *
 *        void *mlib_ImageCreateRowTable(mlib_image *img)
 *
 *        void mlib_ImageDeleteRowTable(mlib_image *img)
 *
 *        mlib_status mlib_ImageSetBorder(mlib_image *img,
 *                                        mlib_s32   minX,
 *                                        mlib_s32   minY,
 *                                        mlib_s32   maxX,
 *                                        mlib_s32   maxY)
 *        
 * ARGUMENT
 *      img       pointer to image data structure
 *      type      image data type, one of MLIB_BIT, MLIB_BYTE, MLIB_SHORT,
 *                MLIB_INT, MLIB_FLOAT or MLIB_DOUBLE
 *      channels  number of image channels
 *      width     image width in pixels
 *      height    image height in pixels
 *      stride    linebytes( bytes to next row) of the image
 *      data      pointer to image data allocated by user
 *      x         x coordinate of the left border in the source image
 *      y         y coordinate of the top border in the source image
 *      w         width of the sub-image
 *      h         height of the sub-image
 *      minX      x coordinate of the clipping box left border
 *      minY      y coordinate of the clipping box top border
 *      maxX      x coordinate of the clipping box rigth border
 *      maxY      y coordinate of the clipping box bottom border
 *      bitoffset the offset in bits from the beginning of the data buffer
 *                to the first pixel
 *
 * DESCRIPTION
 *      mlib_ImageCreateStruct() creates a mediaLib image data structure 
 *      using parameter supplied by user.
 *
 *      mlib_ImageCreate() creates a mediaLib image data structure and 
 *      allocates memory space for image data.
 *
 *      mlib_ImageDelete() deletes the mediaLib image data structure
 *      and frees the memory space of the image data if it is allocated
 *      through mlib_ImageCreate().
 *
 *      mlib_ImageCreateSubimage() creates a mediaLib image structure 
 *      for a sub-image based on a source image.
 *
 *      mlib_ImageCreateRowTable() creates row starts pointer table and
 *      puts it into mlib_image->state field.
 *
 *      mlib_ImageDeleteRowTable() deletes row starts pointer table from
 *      image and puts NULL into mlib_image->state field.
 *
 *      mlib_ImageSetBorder() sets a new values for the clipping box borders
 */

#include <stdlib.h>
#include <mlib_image.h>

/***************************************************************/

mlib_image* mlib_ImageSet(mlib_image *image,
                          mlib_type  type,
                          mlib_s32   channels, 
                          mlib_s32   width, 
                          mlib_s32   height,
                          mlib_s32   stride, 
                          void       *data)
{
  int        wb;                /* width in bytes */
  int        mask;              /* mask for check of stride */

  if (image == NULL) return NULL;
  
  /* for some ugly functions calling with uncorrect parameters */
  image->type     = type;
  image->channels = channels;
  image->width    = width;
  image->height   = height;
  image->stride   = stride;
  image->data     = data;
  image->state    = NULL;

  image->borders[0] = 0;
  image->borders[1] = 0;
  image->borders[2] = width;
  image->borders[3] = height;

  image->bitoffset = 0;

  /* sanity check */
  if (data == NULL) return NULL;

  if (width  <= 0 || height  <= 0 ||
      stride <= 0 || channels < 1 || channels > 4) {
    return NULL;
  }

  /* Check if stride == width
   * If it is then image can be treated as a 1-D vector
   */
  switch (type) {
    case MLIB_DOUBLE:
      wb = width * channels * 8;
      mask = 7;
      break;
    case MLIB_FLOAT:
    case MLIB_INT:
      wb = width * channels * 4;
      mask = 3;
      break;
    case MLIB_SHORT:
      wb = width * channels * 2;
      mask = 1;
      break;
    case MLIB_BYTE:
      wb = width * channels;
      mask = 0;
      break;
    case MLIB_BIT:
      wb = (width * channels + 7) / 8;
      mask = 0;
      break;
    default:
      return NULL;
  }
  if (stride & mask) {
      return NULL;
  }

  image->flags    = ((width & 0xf) << 8);          /* set width field  */
  image->flags   |= ((stride & 0xf) << 16);        /* set stride field */
  image->flags   |= ((height & 0xf) << 12);        /* set height field */
  image->flags   |= (mlib_addr)data & 0xff;
  image->flags   |= MLIB_IMAGE_USERALLOCATED;      /* user allocated data */

  if ((stride != wb) ||
      ((type == MLIB_BIT) && (stride * 8 != width * channels))) {
    image->flags |= MLIB_IMAGE_ONEDVECTOR;
  } 
 
  image->flags &= MLIB_IMAGE_ATTRIBUTESET;
    
  return image;
} 

/***************************************************************/

mlib_image *mlib_ImageCreateStruct(mlib_type type, 
                                   mlib_s32  channels, 
                                   mlib_s32  width, 
                                   mlib_s32  height,
                                   mlib_s32  stride, 
                                   void      *data)
{
  mlib_image *image;
  image = (mlib_image *)mlib_malloc(sizeof(mlib_image));
  if (image == NULL) {
    return NULL;
  }
  if (mlib_ImageSet(image, type, channels, width, height, stride, data) 
      == NULL) {
    mlib_free(image);
    image = NULL;
  }
   
  return image;
}

/***************************************************************/

mlib_image *mlib_ImageCreate(mlib_type type, 
                             mlib_s32  channels, 
                             mlib_s32  width, 
                             mlib_s32  height)
{
  mlib_image *image;
  int        wb;                /* width in bytes */
  void       *data;

  /* sanity check */
  if (width <= 0 || height <= 0 || channels < 1 || channels > 4) {
    return NULL;
  };

  switch (type) {
    case MLIB_DOUBLE:
      wb = width * channels * 8;
      break;
    case MLIB_FLOAT:
    case MLIB_INT:
      wb = width * channels * 4;
      break;
    case MLIB_SHORT:
      wb = width * channels * 2;
      break;
    case MLIB_BYTE:
      wb = width * channels;
      break;
    case MLIB_BIT:
      wb = (width * channels + 7) / 8;
      break;
    default:
      return NULL;
  }

  data = mlib_malloc(wb * height);
  if (data == NULL) {
    return NULL;
  }

  image = (mlib_image *)mlib_malloc(sizeof(mlib_image));
  if (image == NULL) {
    mlib_free(data);
    return NULL;
  };

  image->type     = type;
  image->channels = channels;
  image->width    = width;
  image->height   = height;
  image->stride   = wb;
  image->data     = data;
  image->flags    = ((width & 0xf) << 8);        /* set width field  */
  image->flags   |= ((height & 0xf) << 12);      /* set height field */
  image->flags   |= ((wb & 0xf) << 16);          /* set stride field */
  image->flags   |= (mlib_addr)data & 0xff;

  image->borders[0] = 0;
  image->borders[1] = 0;
  image->borders[2] = width;
  image->borders[3] = height;

  image->bitoffset = 0;

  if ((type == MLIB_BIT) && (wb * 8 != width * channels)) {
    image->flags |= MLIB_IMAGE_ONEDVECTOR;       /* not 1-d vector */
  }

  image->flags &= MLIB_IMAGE_ATTRIBUTESET;
  image->state  = NULL;
    
  return image;
} 

/***************************************************************/

void mlib_ImageDelete(mlib_image *img)
{
  if (img == NULL) return;
  if ((img->flags & MLIB_IMAGE_USERALLOCATED) == 0) {
    mlib_free(img->data);
  }
  mlib_ImageDeleteRowTable(img);
  mlib_free(img);
} 

/***************************************************************/

mlib_image *mlib_ImageCreateSubimage(mlib_image *img,
                                     mlib_s32 x, mlib_s32 y,
                                     mlib_s32 w, mlib_s32 h)
{
  mlib_image     *subimage;
  mlib_type      type;
  mlib_s32       channels;
  mlib_s32       width;                 /* for parent image */
  mlib_s32       height;                /* for parent image */
  mlib_s32       stride;
  mlib_s32       bitoffset=0;
  void           *data;

  /* sanity check */
  if (w <= 0 || h <= 0 || img == NULL) return NULL;

  type     = img->type;
  channels = img->channels;
  width    = img->width;
  height   = img->height;
  stride   = img->stride;

  /* clip the sub-image with respect to the parent image */
  if (((x + w) <= 0) || ((y + h) <= 0) ||
      (x >= width) || (y >= height)) {
    return NULL;
  }
  else {
    if (x < 0) {
      w += x;                                        /* x is negative */
      x = 0;
    }
    if (y < 0) {
      h += y;                                        /* y is negative */
      y = 0;
    }
    if ((x + w) > width) {
      w = width - x;
    }
    if ((y + h) > height) {
      h = height - y;
    }
  }

  /* compute sub-image origin */
  data = (mlib_u8 *)(img->data) + y * stride;

  switch (type) {
    case MLIB_DOUBLE:
      data = (mlib_u8 *)data + x * channels * 8;
      break;
    case MLIB_FLOAT:
    case MLIB_INT:
      data = (mlib_u8 *)data + x * channels * 4;
      break;
    case MLIB_SHORT:
      data = (mlib_u8 *)data + x * channels * 2;
      break;
    case MLIB_BYTE:
      data = (mlib_u8 *)data + x * channels;
      break;
    case MLIB_BIT:
      bitoffset = img->bitoffset;
      data = (mlib_u8 *)data + (x * channels + bitoffset) / 8;
      bitoffset = (x * channels + bitoffset) & 7;
      break;
    default:
      return NULL;
  }

  subimage = mlib_ImageCreateStruct(type, 
                                    channels, 
                                    w,
                                    h,
                                    stride,
                                    data);

  if (subimage != NULL && type == MLIB_BIT)
    subimage->bitoffset = bitoffset;

  return subimage;
}

/***************************************************************/

void *mlib_ImageCreateRowTable(mlib_image *img) {
  mlib_u8  **rtable, *tline;
  mlib_s32 i, im_height, im_stride;
  
  if (img == NULL) return NULL;
  if (img->state)  return img->state;
  
  im_height = mlib_ImageGetHeight(img);
  im_stride = mlib_ImageGetStride(img);
  tline     = mlib_ImageGetData(img);
  rtable    = mlib_malloc((3+im_height)*sizeof(mlib_u8 *));

  if (rtable == NULL || tline == NULL) return NULL;
  
  rtable[0] = 0;
  rtable[1] = (mlib_u8*)((void **)rtable + 1);
  rtable[2+im_height] = (mlib_u8*)((void **)rtable + 1);
  for (i = 0; i < im_height; i++) {
    rtable[i+2] = tline;
    tline    += im_stride;
  }
  img->state = ((void **)rtable + 2);
  return img->state;
}

/***************************************************************/

void mlib_ImageDeleteRowTable(mlib_image *img)
{
  void **state;

  if (img == NULL) return;

  state = img->state;
  if (!state) return;
  
  mlib_free(state - 2);
  img->state = 0;
}

/***************************************************************/

mlib_status mlib_ImageSetBorder(mlib_image *img,
                                mlib_s32   minX,
                                mlib_s32   minY,
                                mlib_s32   maxX,
                                mlib_s32   maxY)
{
  mlib_s32 width;
  mlib_s32 height;

  if (img == NULL) return MLIB_FAILURE;

  width    = img->width;

  if (minX < 0 || minX > width ||
      maxX < 0 || maxX > width ||
      minX >= maxX) return MLIB_OUTOFRANGE;

  height   = img->height;

  if (minY < 0 || minY > height ||
      maxY < 0 || maxY > height ||
      minY >= maxY) return MLIB_OUTOFRANGE;

  img->borders[0] = minX;
  img->borders[1] = minY;
  img->borders[2] = maxX;
  img->borders[3] = maxY;

  return MLIB_SUCCESS;
}

/***************************************************************/
