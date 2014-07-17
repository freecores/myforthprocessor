/*
 * @(#)mlib_v_ImageLookUp.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageLookUp.c	1.11	00/03/28 SMI"

/*
 * $RCSfile: mlib_v_ImageLookUp.c,v $
 * $Revision: 4.1 $
 * $Date: 1996/12/11 20:43:36 $
 * $Author: liang $
 */

/*
 * FUNCTION
 *      mlib_ImageLookUp - table lookup
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageLookUp(mlib_image *dst,
 *                                   mlib_image *src,
 *                                   void **table);
 *
 * ARGUMENT
 *      dst     pointer to output image
 *      src     pointer to input image
 *      table   lookup table
 *
 * DESCRIPTION
 *      dst = table[src]
 */


#include "mlib_image.h"
#include "mlib_ImageCheck.h"

/***************************************************************/

void mlib_v_ImageLookUp_U8_U8_1(mlib_u8  *sa, mlib_s32 slb,
                                mlib_u8  *da, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_u8  **table);
void mlib_v_ImageLookUp_U8_U8_2(mlib_u8  *sa, mlib_s32 slb,
                                mlib_u8  *da, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_u8  **table);
void mlib_v_ImageLookUp_U8_U8_3(mlib_u8  *sa, mlib_s32 slb,
                                mlib_u8  *da, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_u8  **table);
void mlib_v_ImageLookUp_U8_U8_4(mlib_u8  *sa, mlib_s32 slb,
                                mlib_u8  *da, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_u8  **table);

void mlib_v_ImageLookUp_S16_U8_1(mlib_s16 *sa, mlib_s32 slb,
                                 mlib_u8  *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_u8  **table);
void mlib_v_ImageLookUp_S16_U8_2(mlib_s16 *sa, mlib_s32 slb,
                                 mlib_u8  *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_u8  **table);
void mlib_v_ImageLookUp_S16_U8_3(mlib_s16 *sa, mlib_s32 slb,
                                 mlib_u8  *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_u8  **table);
void mlib_v_ImageLookUp_S16_U8_4(mlib_s16 *sa, mlib_s32 slb,
                                 mlib_u8  *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_u8  **table);

void mlib_v_ImageLookUp_S32_U8_1(mlib_s32 *sa, mlib_s32 slb,
                                 mlib_u8  *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_u8  **table);
void mlib_v_ImageLookUp_S32_U8_2(mlib_s32 *sa, mlib_s32 slb,
                                 mlib_u8  *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_u8  **table);
void mlib_v_ImageLookUp_S32_U8_3(mlib_s32 *sa, mlib_s32 slb,
                                 mlib_u8  *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_u8  **table);
void mlib_v_ImageLookUp_S32_U8_4(mlib_s32 *sa, mlib_s32 slb,
                                 mlib_u8  *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_u8  **table);

void mlib_v_ImageLookUp_U8_S16_1(mlib_u8  *sa, mlib_s32 slb,
                                 mlib_s16 *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s16 **table);
void mlib_v_ImageLookUp_U8_S16_2(mlib_u8  *sa, mlib_s32 slb,
                                 mlib_s16 *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s16 **table);
void mlib_v_ImageLookUp_U8_S16_3(mlib_u8  *sa, mlib_s32 slb,
                                 mlib_s16 *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s16 **table);
void mlib_v_ImageLookUp_U8_S16_4(mlib_u8  *sa, mlib_s32 slb,
                                 mlib_s16 *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s16 **table);

void mlib_v_ImageLookUp_S16_S16_1(mlib_s16 *sa, mlib_s32 slb,
                                  mlib_s16 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s16 **table);
void mlib_v_ImageLookUp_S16_S16_2(mlib_s16 *sa, mlib_s32 slb,
                                  mlib_s16 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s16 **table);
void mlib_v_ImageLookUp_S16_S16_3(mlib_s16 *sa, mlib_s32 slb,
                                  mlib_s16 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s16 **table);
void mlib_v_ImageLookUp_S16_S16_4(mlib_s16 *sa, mlib_s32 slb,
                                  mlib_s16 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s16 **table);

void mlib_v_ImageLookUp_S32_S16_1(mlib_s32 *sa, mlib_s32 slb,
                                  mlib_s16 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s16 **table);
void mlib_v_ImageLookUp_S32_S16_2(mlib_s32 *sa, mlib_s32 slb,
                                  mlib_s16 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s16 **table);
void mlib_v_ImageLookUp_S32_S16_3(mlib_s32 *sa, mlib_s32 slb,
                                  mlib_s16 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s16 **table);
void mlib_v_ImageLookUp_S32_S16_4(mlib_s32 *sa, mlib_s32 slb,
                                  mlib_s16 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s16 **table);

void mlib_v_ImageLookUp_U8_S32_1(mlib_u8  *sa, mlib_s32 slb,
                                 mlib_s32 *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s32 **table);
void mlib_v_ImageLookUp_U8_S32_2(mlib_u8  *sa, mlib_s32 slb,
                                 mlib_s32 *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s32 **table);
void mlib_v_ImageLookUp_U8_S32_3(mlib_u8  *sa, mlib_s32 slb,
                                 mlib_s32 *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s32 **table);
void mlib_v_ImageLookUp_U8_S32_4(mlib_u8  *sa, mlib_s32 slb,
                                 mlib_s32 *da, mlib_s32 dlb,
                                 mlib_s32 xsize, mlib_s32 ysize,
                                 mlib_s32 **table);

void mlib_v_ImageLookUp_S16_S32_1(mlib_s16 *sa, mlib_s32 slb,
                                  mlib_s32 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s32 **table);
void mlib_v_ImageLookUp_S16_S32_2(mlib_s16 *sa, mlib_s32 slb,
                                  mlib_s32 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s32 **table);
void mlib_v_ImageLookUp_S16_S32_3(mlib_s16 *sa, mlib_s32 slb,
                                  mlib_s32 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s32 **table);
void mlib_v_ImageLookUp_S16_S32_4(mlib_s16 *sa, mlib_s32 slb,
                                  mlib_s32 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s32 **table);

void mlib_v_ImageLookUp_S32_S32(mlib_s32 *sa, mlib_s32 slb,
                                mlib_s32 *da, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 **table, mlib_s32 csize);

void mlib_ImageLookUp_U8_D64(mlib_u8  *src, mlib_s32 slb,
                             mlib_d64 *dst, mlib_s32 dlb,
                             mlib_s32 xsize, mlib_s32 ysize,
                             mlib_s32 csize,
                             mlib_d64 **table);

void mlib_ImageLookUp_S16_D64(mlib_s16 *src, mlib_s32 slb,
                              mlib_d64 *dst, mlib_s32 dlb,
                              mlib_s32 xsize, mlib_s32 ysize,
                              mlib_s32 csize,
                              mlib_d64 **table);

void mlib_ImageLookUp_S32_D64(mlib_s32 *src, mlib_s32 slb,
                              mlib_d64 *dst, mlib_s32 dlb,
                              mlib_s32 xsize, mlib_s32 ysize,
                              mlib_s32 csize,
                              mlib_d64 **table);

mlib_status mlib_ImageLookUp_Bit_U8_1(mlib_u8 *src, mlib_s32 slb,
                                      mlib_u8 *dst, mlib_s32 dlb,
                                      mlib_s32 xsize, mlib_s32 ysize,
                                      mlib_s32 csize, mlib_s32 bitoff,
                                      mlib_u8  **table);

mlib_status mlib_ImageLookUp_Bit_U8_2(mlib_u8 *src, mlib_s32 slb,
                                      mlib_u8 *dst, mlib_s32 dlb,
                                      mlib_s32 xsize, mlib_s32 ysize,
                                      mlib_s32 csize, mlib_s32 bitoff,
                                      mlib_u8  **table);

mlib_status mlib_ImageLookUp_Bit_U8_3(mlib_u8 *src, mlib_s32 slb,
                                      mlib_u8 *dst, mlib_s32 dlb,
                                      mlib_s32 xsize, mlib_s32 ysize,
                                      mlib_s32 csize, mlib_s32 bitoff,
                                      mlib_u8  **table);

mlib_status mlib_ImageLookUp_Bit_U8_4(mlib_u8 *src, mlib_s32 slb,
                                      mlib_u8 *dst, mlib_s32 dlb,
                                      mlib_s32 xsize, mlib_s32 ysize,
                                      mlib_s32 csize, mlib_s32 bitoff,
                                      mlib_u8  **table);

/***************************************************************/

void mlib_v_ImageLookUpSI_U8_U8_2(mlib_u8  *sa, mlib_s32 slb,
                                  mlib_u8  *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_u8  **table);
void mlib_v_ImageLookUpSI_U8_U8_3(mlib_u8  *sa, mlib_s32 slb,
                                  mlib_u8  *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_u8  **table);
void mlib_v_ImageLookUpSI_U8_U8_4(mlib_u8  *sa, mlib_s32 slb,
                                  mlib_u8  *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_u8  **table);

void mlib_v_ImageLookUpSI_S16_U8_2(mlib_s16 *sa, mlib_s32 slb,
                                   mlib_u8  *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_u8  **table);
void mlib_v_ImageLookUpSI_S16_U8_3(mlib_s16 *sa, mlib_s32 slb,
                                   mlib_u8  *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_u8  **table);
void mlib_v_ImageLookUpSI_S16_U8_4(mlib_s16 *sa, mlib_s32 slb,
                                   mlib_u8  *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_u8  **table);

void mlib_v_ImageLookUpSI_S32_U8_2(mlib_s32 *sa, mlib_s32 slb,
                                   mlib_u8  *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_u8  **table);
void mlib_v_ImageLookUpSI_S32_U8_3(mlib_s32 *sa, mlib_s32 slb,
                                   mlib_u8  *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_u8  **table);
void mlib_v_ImageLookUpSI_S32_U8_4(mlib_s32 *sa, mlib_s32 slb,
                                   mlib_u8  *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_u8  **table);

void mlib_v_ImageLookUpSI_U8_S16_2(mlib_u8  *sa, mlib_s32 slb,
                                   mlib_s16 *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_s16 **table);
void mlib_v_ImageLookUpSI_U8_S16_3(mlib_u8  *sa, mlib_s32 slb,
                                   mlib_s16 *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_s16 **table);
void mlib_v_ImageLookUpSI_U8_S16_4(mlib_u8  *sa, mlib_s32 slb,
                                   mlib_s16 *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_s16 **table);

void mlib_v_ImageLookUpSI_S16_S16_2(mlib_s16 *sa, mlib_s32 slb,
                                    mlib_s16 *da, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s16 **table);
void mlib_v_ImageLookUpSI_S16_S16_3(mlib_s16 *sa, mlib_s32 slb,
                                    mlib_s16 *da, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s16 **table);
void mlib_v_ImageLookUpSI_S16_S16_4(mlib_s16 *sa, mlib_s32 slb,
                                    mlib_s16 *da, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s16 **table);

void mlib_v_ImageLookUpSI_S32_S16_2(mlib_s32 *sa, mlib_s32 slb,
                                    mlib_s16 *da, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s16 **table);
void mlib_v_ImageLookUpSI_S32_S16_3(mlib_s32 *sa, mlib_s32 slb,
                                    mlib_s16 *da, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s16 **table);
void mlib_v_ImageLookUpSI_S32_S16_4(mlib_s32 *sa, mlib_s32 slb,
                                    mlib_s16 *da, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s16 **table);

void mlib_v_ImageLookUpSI_U8_S32_2(mlib_u8  *sa, mlib_s32 slb,
                                   mlib_s32 *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_s32 **table);
void mlib_v_ImageLookUpSI_U8_S32_3(mlib_u8  *sa, mlib_s32 slb,
                                   mlib_s32 *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_s32 **table);
void mlib_v_ImageLookUpSI_U8_S32_4(mlib_u8  *sa, mlib_s32 slb,
                                   mlib_s32 *da, mlib_s32 dlb,
                                   mlib_s32 xsize, mlib_s32 ysize,
                                   mlib_s32 **table);

void mlib_v_ImageLookUpSI_S16_S32_2(mlib_s16 *sa, mlib_s32 slb,
                                    mlib_s32 *da, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s32 **table);
void mlib_v_ImageLookUpSI_S16_S32_3(mlib_s16 *sa, mlib_s32 slb,
                                    mlib_s32 *da, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s32 **table);
void mlib_v_ImageLookUpSI_S16_S32_4(mlib_s16 *sa, mlib_s32 slb,
                                    mlib_s32 *da, mlib_s32 dlb,
                                    mlib_s32 xsize, mlib_s32 ysize,
                                    mlib_s32 **table);

void mlib_v_ImageLookUpSI_S32_S32(mlib_s32 *sa, mlib_s32 slb,
                                  mlib_s32 *da, mlib_s32 dlb,
                                  mlib_s32 xsize, mlib_s32 ysize,
                                  mlib_s32 **table, mlib_s32 csize);

void mlib_ImageLookUpSI_U8_D64(mlib_u8  *src, mlib_s32 slb,
                               mlib_d64 *dst, mlib_s32 dlb,
                               mlib_s32 xsize, mlib_s32 ysize,
                               mlib_s32 csize,
                               mlib_d64 **table);

void mlib_ImageLookUpSI_S16_D64(mlib_s16 *src, mlib_s32 slb,
                                mlib_d64 *dst, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 csize,
                                mlib_d64 **table);

void mlib_ImageLookUpSI_S32_D64(mlib_s32 *src, mlib_s32 slb,
                                mlib_d64 *dst, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 csize,
                                mlib_d64 **table);

/***************************************************************/

#ifdef MLIB_TEST

mlib_status mlib_v_ImageLookUp(mlib_image *dst,
                               mlib_image *src,
                               void **table)
#else

mlib_status mlib_ImageLookUp(mlib_image *dst,
                             mlib_image *src,
                             void **table)
#endif

{
  mlib_s32   slb, dlb, xsize, ysize, nchan, ichan, bitoff_src;
  mlib_type  stype, dtype;
  void       *sa, *da;

  MLIB_IMAGE_CHECK(src);
  MLIB_IMAGE_CHECK(dst);
  MLIB_IMAGE_SIZE_EQUAL(src, dst);
  MLIB_IMAGE_CHAN_SRC1_OR_EQ(src, dst);

  stype = mlib_ImageGetType(src);
  dtype = mlib_ImageGetType(dst);
  ichan = mlib_ImageGetChannels(src);
  nchan = mlib_ImageGetChannels(dst);
  xsize = mlib_ImageGetWidth(src);
  ysize = mlib_ImageGetHeight(src);
  slb   = mlib_ImageGetStride(src);
  dlb   = mlib_ImageGetStride(dst);
  sa    = mlib_ImageGetData(src);
  da    = mlib_ImageGetData(dst);

  if (ichan == nchan) {

    if (dtype == MLIB_BYTE) {

      if (stype == MLIB_BYTE) {

      switch (nchan) {
        case 1:
          mlib_v_ImageLookUp_U8_U8_1(sa, slb,
                                     da, dlb,
                                     xsize, ysize,
                                     (mlib_u8 **) table);
          break;
        case 2:
          mlib_v_ImageLookUp_U8_U8_2(sa, slb,
                                     da, dlb,
                                     xsize, ysize,
                                     (mlib_u8 **) table);
          break;
        case 3:
          mlib_v_ImageLookUp_U8_U8_3(sa, slb,
                                     da, dlb,
                                     xsize, ysize,
                                     (mlib_u8 **) table);
          break;
        case 4:
          mlib_v_ImageLookUp_U8_U8_4(sa, slb,
                                     da, dlb,
                                     xsize, ysize,
                                     (mlib_u8 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

      switch (nchan) {
        case 1:
          mlib_v_ImageLookUp_S16_U8_1(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_u8 **) table);
          break;
        case 2:
          mlib_v_ImageLookUp_S16_U8_2(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_u8 **) table);
          break;
        case 3:
          mlib_v_ImageLookUp_S16_U8_3(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_u8 **) table);
          break;
        case 4:
          mlib_v_ImageLookUp_S16_U8_4(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_u8 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

      switch (nchan) {
        case 1:
          mlib_v_ImageLookUp_S32_U8_1(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_u8 **) table);
          break;
        case 2:
          mlib_v_ImageLookUp_S32_U8_2(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_u8 **) table);
          break;
        case 3:
          mlib_v_ImageLookUp_S32_U8_3(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_u8 **) table);
          break;
        case 4:
          mlib_v_ImageLookUp_S32_U8_4(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_u8 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_BIT) {

        if (nchan != 1) return MLIB_FAILURE;

        bitoff_src = mlib_ImageGetBitOffset(src);   /* bits to first byte */
        return mlib_ImageLookUp_Bit_U8_1(sa, slb,
                                da, dlb,
                                xsize, ysize, nchan, bitoff_src,
                                (mlib_u8 **) table);
      }

    } else if (dtype == MLIB_SHORT) {

      if (stype == MLIB_BYTE) {

      switch (nchan) {
        case 1:
          mlib_v_ImageLookUp_U8_S16_1(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s16 **) table);
          break;
        case 2:
          mlib_v_ImageLookUp_U8_S16_2(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s16 **) table);
          break;
        case 3:
          mlib_v_ImageLookUp_U8_S16_3(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s16 **) table);
          break;
        case 4:
          mlib_v_ImageLookUp_U8_S16_4(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s16 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

      switch (nchan) {
        case 1:
          mlib_v_ImageLookUp_S16_S16_1(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s16 **) table);
          break;
        case 2:
          mlib_v_ImageLookUp_S16_S16_2(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s16 **) table);
          break;
        case 3:
          mlib_v_ImageLookUp_S16_S16_3(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s16 **) table);
          break;
        case 4:
          mlib_v_ImageLookUp_S16_S16_4(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s16 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

      switch (nchan) {
        case 1:
          mlib_v_ImageLookUp_S32_S16_1(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s16 **) table);
          break;
        case 2:
          mlib_v_ImageLookUp_S32_S16_2(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s16 **) table);
          break;
        case 3:
          mlib_v_ImageLookUp_S32_S16_3(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s16 **) table);
          break;
        case 4:
          mlib_v_ImageLookUp_S32_S16_4(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s16 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;
      }
    } else if (dtype == MLIB_INT) {

      if (stype == MLIB_BYTE) {

      switch (nchan) {
        case 1:
          mlib_v_ImageLookUp_U8_S32_1(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s32 **) table);
          break;
        case 2:
          mlib_v_ImageLookUp_U8_S32_2(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s32 **) table);
          break;
        case 3:
          mlib_v_ImageLookUp_U8_S32_3(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s32 **) table);
          break;
        case 4:
          mlib_v_ImageLookUp_U8_S32_4(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s32 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

      switch (nchan) {
        case 1:
          mlib_v_ImageLookUp_S16_S32_1(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s32 **) table);
          break;
        case 2:
          mlib_v_ImageLookUp_S16_S32_2(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s32 **) table);
          break;
        case 3:
          mlib_v_ImageLookUp_S16_S32_3(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s32 **) table);
          break;
        case 4:
          mlib_v_ImageLookUp_S16_S32_4(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s32 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {
        if ((nchan >= 1) && (nchan <= 4)) {

          mlib_v_ImageLookUp_S32_S32(sa, slb,
                                     da, dlb,
                                     xsize, ysize,
                                     (mlib_s32 **) table, nchan);
          return MLIB_SUCCESS;
        } else {
          return MLIB_FAILURE;
        }
      }
    } else if (dtype == MLIB_FLOAT) {

      if (stype == MLIB_BYTE) {

      switch (nchan) {
        case 1:
          mlib_v_ImageLookUp_U8_S32_1(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s32 **) table);
          break;
        case 2:
          mlib_v_ImageLookUp_U8_S32_2(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s32 **) table);
          break;
        case 3:
          mlib_v_ImageLookUp_U8_S32_3(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s32 **) table);
          break;
        case 4:
          mlib_v_ImageLookUp_U8_S32_4(sa, slb,
                                      da, dlb,
                                      xsize, ysize,
                                      (mlib_s32 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

      switch (nchan) {
        case 1:
          mlib_v_ImageLookUp_S16_S32_1(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s32 **) table);
          break;
        case 2:
          mlib_v_ImageLookUp_S16_S32_2(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s32 **) table);
          break;
        case 3:
          mlib_v_ImageLookUp_S16_S32_3(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s32 **) table);
          break;
        case 4:
          mlib_v_ImageLookUp_S16_S32_4(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s32 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {
        if ((nchan >= 1) && (nchan <= 4)) {

          mlib_v_ImageLookUp_S32_S32(sa, slb,
                                     da, dlb,
                                     xsize, ysize,
                                     (mlib_s32 **) table, nchan);
          return MLIB_SUCCESS;
        } else {
          return MLIB_FAILURE;
        }
      }
    } else if (dtype == MLIB_DOUBLE) {

      if (stype == MLIB_BYTE) {

        mlib_ImageLookUp_U8_D64(sa, slb,
                                da, dlb/8,
                                xsize, ysize, nchan,
                                (mlib_d64 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_ImageLookUp_S16_D64(sa, slb/2,
                                 da, dlb/8,
                                 xsize, ysize, nchan,
                                 (mlib_d64 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_ImageLookUp_S32_D64(sa, slb/4,
                                 da, dlb/8,
                                 xsize, ysize, nchan,
                                 (mlib_d64 **) table);
        return MLIB_SUCCESS;
      }
    }

  } else if (ichan == 1) {

    if (dtype == MLIB_BYTE) {

      if (stype == MLIB_BYTE) {

      switch (nchan) {
        case 2:
          mlib_v_ImageLookUpSI_U8_U8_2(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_u8 **) table);
          break;
        case 3:
          mlib_v_ImageLookUpSI_U8_U8_3(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_u8 **) table);
          break;
        case 4:
          mlib_v_ImageLookUpSI_U8_U8_4(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_u8 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

      switch (nchan) {
        case 2:
          mlib_v_ImageLookUpSI_S16_U8_2(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_u8 **) table);
          break;
        case 3:
          mlib_v_ImageLookUpSI_S16_U8_3(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_u8 **) table);
          break;
        case 4:
          mlib_v_ImageLookUpSI_S16_U8_4(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_u8 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;
      } else if (stype == MLIB_INT) {

      switch (nchan) {
        case 2:
          mlib_v_ImageLookUpSI_S32_U8_2(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_u8 **) table);
          break;
        case 3:
          mlib_v_ImageLookUpSI_S32_U8_3(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_u8 **) table);
          break;
        case 4:
          mlib_v_ImageLookUpSI_S32_U8_4(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_u8 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_BIT) {

        bitoff_src = mlib_ImageGetBitOffset(src);   /* bits to first byte */

        if (nchan == 2) {

        return mlib_ImageLookUp_Bit_U8_2(sa, slb,
                                da, dlb,
                                xsize, ysize, nchan, bitoff_src,
                                (mlib_u8 **) table);
        } else  if (nchan == 3) {

        return mlib_ImageLookUp_Bit_U8_3(sa, slb,
                                da, dlb,
                                xsize, ysize, nchan, bitoff_src,
                                (mlib_u8 **) table);

        } else /* (nchan == 4) */ {

        return mlib_ImageLookUp_Bit_U8_4(sa, slb,
                                da, dlb,
                                xsize, ysize, nchan, bitoff_src,
                                (mlib_u8 **) table);
        }
      }

    } else if (dtype == MLIB_SHORT) {

      if (stype == MLIB_BYTE) {

      switch (nchan) {
        case 2:
          mlib_v_ImageLookUpSI_U8_S16_2(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_s16 **) table);
          break;
        case 3:
          mlib_v_ImageLookUpSI_U8_S16_3(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_s16 **) table);
          break;
        case 4:
          mlib_v_ImageLookUpSI_U8_S16_4(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_s16 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

      switch (nchan) {
        case 2:
          mlib_v_ImageLookUpSI_S16_S16_2(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s16 **) table);
          break;
        case 3:
          mlib_v_ImageLookUpSI_S16_S16_3(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s16 **) table);
          break;
        case 4:
          mlib_v_ImageLookUpSI_S16_S16_4(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s16 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

      switch (nchan) {
        case 2:
          mlib_v_ImageLookUpSI_S32_S16_2(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s16 **) table);
          break;
        case 3:
          mlib_v_ImageLookUpSI_S32_S16_3(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s16 **) table);
          break;
        case 4:
          mlib_v_ImageLookUpSI_S32_S16_4(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s16 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;
      }
    } else if (dtype == MLIB_INT) {

      if (stype == MLIB_BYTE) {

      switch (nchan) {
        case 2:
          mlib_v_ImageLookUpSI_U8_S32_2(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_s32 **) table);
          break;
        case 3:
          mlib_v_ImageLookUpSI_U8_S32_3(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_s32 **) table);
          break;
        case 4:
          mlib_v_ImageLookUpSI_U8_S32_4(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_s32 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

      switch (nchan) {
        case 2:
          mlib_v_ImageLookUpSI_S16_S32_2(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s32 **) table);
          break;
        case 3:
          mlib_v_ImageLookUpSI_S16_S32_3(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s32 **) table);
          break;
        case 4:
          mlib_v_ImageLookUpSI_S16_S32_4(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s32 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        if ((nchan >= 1) && (nchan <= 4)) {

          mlib_v_ImageLookUpSI_S32_S32(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s32 **) table, nchan);
          return MLIB_SUCCESS;
        } else {
          return MLIB_FAILURE;
        }
      }
    } else if (dtype == MLIB_FLOAT) {

      if (stype == MLIB_BYTE) {

      switch (nchan) {
        case 2:
          mlib_v_ImageLookUpSI_U8_S32_2(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_s32 **) table);
          break;
        case 3:
          mlib_v_ImageLookUpSI_U8_S32_3(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_s32 **) table);
          break;
        case 4:
          mlib_v_ImageLookUpSI_U8_S32_4(sa, slb,
                                        da, dlb,
                                        xsize, ysize,
                                        (mlib_s32 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

      switch (nchan) {
        case 2:
          mlib_v_ImageLookUpSI_S16_S32_2(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s32 **) table);
          break;
        case 3:
          mlib_v_ImageLookUpSI_S16_S32_3(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s32 **) table);
          break;
        case 4:
          mlib_v_ImageLookUpSI_S16_S32_4(sa, slb,
                                         da, dlb,
                                         xsize, ysize,
                                         (mlib_s32 **) table);
          break;
        default:
          return MLIB_FAILURE;
        }
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        if ((nchan >= 1) && (nchan <= 4)) {

          mlib_v_ImageLookUpSI_S32_S32(sa, slb,
                                       da, dlb,
                                       xsize, ysize,
                                       (mlib_s32 **) table, nchan);
          return MLIB_SUCCESS;
        } else {
          return MLIB_FAILURE;
        }
      }
    } else if (dtype == MLIB_DOUBLE) {

      if (stype == MLIB_BYTE) {

        mlib_ImageLookUpSI_U8_D64(sa, slb,
                                  da, dlb/8,
                                  xsize, ysize, nchan,
                                  (mlib_d64 **) table);

        return MLIB_SUCCESS;

      } else if (stype == MLIB_SHORT) {

        mlib_ImageLookUpSI_S16_D64(sa, slb/2,
                                   da, dlb/8,
                                   xsize, ysize, nchan,
                                   (mlib_d64 **) table);
        return MLIB_SUCCESS;

      } else if (stype == MLIB_INT) {

        mlib_ImageLookUpSI_S32_D64(sa, slb/4,
                                   da, dlb/8,
                                   xsize, ysize, nchan,
                                   (mlib_d64 **) table);
        return MLIB_SUCCESS;
      }
    }
  }
  return MLIB_FAILURE;
}

/***************************************************************/
