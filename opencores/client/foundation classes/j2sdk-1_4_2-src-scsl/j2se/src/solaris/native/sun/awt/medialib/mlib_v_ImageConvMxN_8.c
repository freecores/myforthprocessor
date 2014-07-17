/*
 * @(#)mlib_v_ImageConvMxN_8.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageConvMxN_8.c	1.15	00/02/07 SMI"

/*
 * FUNCTION
 *      mlib_v_convMxN_8nw - convolve a 8-bit image, MxN kernel,
 *                         edge = no write
 *
 * SYNOPSIS
 *      mlib_status
 *      mlib_v_convMxN_8nw( mlib_image *srcimg, mlib_image *dstimg,
 *                          mlib_s32 kwid, mlib_s32 khgt,
 *                          mlib_s32 khw, mlib_s32 khh,
 *                          mlib_s32 *skernel, mlib_s32 discardbits
 *                          mlib_s32 cmask)
 *
 * ARGUMENT
 *      srcimg       Ptr to source image structure
 *      dstimg       Ptr to destination image structure
 *      khgt         Kernel height (# of rows)
 *      kwid         Kernel width (# of cols)
 *      skernel      Ptr to convolution kernel
 *      discardbits  The number of LSBits of the 32-bit accumulator that
 *                   are discarded when the 32-bit accumulator is converted
 *                   to 16-bit output data; discardbits must be 1-15 (it
 *                   cannot be zero). Same as exponent N for scalefac=2**N.
 *      cmask        Channel mask to indicate the channels to be convolved.
 *                   Each bit of which represents a channel in the image. The
 *                   channels corresponded to 1 bits are those to be processed.
 *
 * DESCRIPTION
 *      A 2-D convolution (MxN kernel) for 8-bit images.
 *
 *  The routine breaks the data into 8 alignment groups based on each
 *  pixel's offset from an 8-byte boundary (0, 1, 2,.., or 7 bytes of
 *  offset). The coefficients are stored in a run-time table, ordered
 *  by alignment group.
 *
 *  Along with each coefficient, the table stores offsets to the pixel
 *  corresponding to each coefficient. These offsets are in pixels, and
 *  relative to the pixel corresponding to the coefficient in kernel
 *  row 0, col 0. The offsets are used to set up one pointer for each
 *  coefficient. Each pointer is used to access the pixels to be
 *  multiplied by that coefficient, as the convolution proceeds along
 *  the row.
 *
 *  The bulk of this convolution implementation is a simple multiply/
 *  accumulate engine. Most of the intelligence and complexity in this
 *  implementation is concentrated in the formula for calculating the
 *  offset for each coefficient in the coefficient table.
 *
 *  The multiplies and accumulates are done for an output row with a
 *  loop that iterates 8 times, once for each alignment group.
 *
 *  The processing for an alignment group is handled in a subroutine;
 *  it begins with an align address. Then the 16-bit accumulators are
 *  read from the rowbuf in memory, the coefficients in that alignment
 *  group are multiplied by their respective coefficients (pulled
 *  sequentially from the coefficient table), and the products are added
 *  to the accumulators, which are then stored back into the row buffer
 *  in memory. Each loop works across the image from left to right.
 *
 *  An inner loop within each alignment group processes 16 coefficients
 *  (from the coefficient table) at a time, until all coefficients with
 *  that alignment have been pulled from the table and processed.
 *
 *  After the 8 alignment group function calls, all input pixels for
 *  that output row have been processed. The intermediate row buffer is
 *  then read back, and each 16-bit accumulator value in the buffer is
 *  divided by the scale factor, and packed into an 8-bit value in the
 *  destination image.
 *
 *  When the pixels in an alignment group are being multiplied and
 *  accumulated for an output point, 4 pixels are used at once, since
 *  the 8x16 VIS multiply uses a 32-bit input holding 4 8-bit values.
 *  These 4 pixels being multiplied and accumulated in parallel are
 *  contributing to 4 output pixels.
 *
 *  Since the data I/O is done with 64-bit chunks, 8 8-bit input pixels
 *  are processed as a group, contributing to 8 output pixels.
 *
 *  To maximize pipelining of instructions through the execution units,
 *  two such blocks of 8 pixels each are processed within each inner
 *  loop. Thus each inner loop operates on a blocks of 16 pixels, so that
 *  16 16-bit accumulators in the intermediate row buffer in memory
 *  are updated after each iteration of an inner loop.
 *
 *  Using 16 pixels per block provides a larger the "pool" of data for
 *  the inner loop to work with. This enables a larger number of
 *  independent calculations to avoid data dependency stalls.
 *
 *  Pseudo-code:
 *
 *  allocate aligned rowbuf of 16-bit accumulators
 *  allocate and fill coefficient table:
 *    sort coeffs into alignment groups
 *    calculate each coeff's offset from upper left coeff
 *    (based on source image stride)
 *
 *  for each output row {
 *    init rowbuf accumulators to rounding value
 *    (to yield correct roundoff when bits are
 *     truncated to produce 8-bit result)
 *    for (alignment group = 0 to 7 ) {
 *      set GSR
 *      coeff_left_in_grp = coeffs in alignment group
 *      while (coeff_left_in_grp > 8) {
 *        for (coeff=0; coeff<8; coeff++) {
 *    pull next coeff from table, store in a reg
 *    pull next coeff offset from table
 *    use offset to calculate aligned ptr to pixel
 *      to multiply by that coeff
 *    use ptr for coeff to get 1st 8 pixels for that coeff
 *        for each block of 16 pixels in output row {
 *    load 16 16-bit accumulators from memory rowbuf
 *    for (coeff=0; coeff<8; coeff++) {
 *      multiply next 8 pixels by coeff
 *      add 8 products to accumulators 1-8
 *      use ptr for coeff to get next 8 pixels for that coeff
 *
 *      multiply next 8 pixels by coeff
 *      add 8 products to accumulators 9-16
 *      use ptr for coeff to get next 8 pixels for that coeff
 *    }
 *    store 16 16-bit accumulators to memory rowbuf
 *    coeff_left_in_grp = coeff_left_in_grp - 8
 *        } / end block of 8 pixels /
 *      } / end while /
 *      repeat while loop steps for remaining 0-7 coeffs
 *    } / end alignment group /
 *    pack rowbuf accumulators to 8-bit destination buf
 *  } / end output row /
 *
 *  (NOTE:  Actual code for the processing of an alignment group
 *    is in a separate source code file.)
 *
 *************************************************************************
 */

#include <stdlib.h>
#include <vis_proto.h>
#include <mlib_image.h>
#include "mlib_v_ImageConvMxN_8.h"

static mlib_u32 mlib_round_8[16] = {
  0x00400040, 0x00200020, 0x00100010, 0x00080008,
  0x00040004, 0x00020002, 0x00010001, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000 };

extern void mlib_conv8_fill_coeff_tbl( int khgt, int kwid, mlib_s32 *kernel,
                                       struct COEFF_REC *coeff_tbl,
                                       int *coeff_cnt, int spxl_stride );

extern void mlib_conv8_algn_grp( mlib_d64 *intrm_buf,         /* aligned intermed. rowbuf */
                                 int dwid,                    /* # pxls to wrt to dest. row */
                                 struct COEFF_REC *coeff_tbl, /* tbl of coeffs and offsets */
                                 int coeff_idx,               /* 1st coeff_tbl record to use */
                                 int coeff_cnt,               /* # of coeff_tbl records to use */
                                 mlib_u8 *srowaddr,           /* 1st row of src to do */
                                 int algn_boffs  );           /* 8-byte alignment of this group */

void mlib_ImageCopy_na (mlib_u8*, mlib_u8*, mlib_s32);
void mlib_v_ImageChannelExtract_U8_21_D1 (mlib_u8*, mlib_u8*, mlib_s32, mlib_s32);
void mlib_v_ImageChannelExtract_U8_31_D1 (mlib_u8*, mlib_u8*, mlib_s32, mlib_s32);
void mlib_v_ImageChannelExtract_U8_41_D1 (mlib_u8*, mlib_u8*, mlib_s32, mlib_s32);
void mlib_v_ImageChannelInsert_U8_12_D1 (mlib_u8*, mlib_u8*, mlib_s32, mlib_s32);
void mlib_v_ImageChannelInsert_U8_13_D1 (mlib_u8*, mlib_u8*, mlib_s32, mlib_s32);
void mlib_v_ImageChannelInsert_U8_14_D1 (mlib_u8*, mlib_u8*, mlib_s32, mlib_s32);

static void mlib_my_load (mlib_u8* src, mlib_u8* dst, mlib_s32 size, mlib_s32 cmask)
{
  mlib_ImageCopy_na (src, dst, size);
}

static void mlib_my_store (mlib_u8* src, mlib_u8* dst, mlib_s32 size, mlib_s32 cmask)
{
  mlib_ImageCopy_na (src, dst, size);
}

/***************************************************************/

mlib_status mlib_convMxN_8nw_f(mlib_image *dst,
                               mlib_image *src,
                               mlib_s32   m,
                               mlib_s32   n,
                               mlib_s32   dm,
                               mlib_s32   dn,
                               mlib_s32   *kern,
                               mlib_s32   scale);

/***************************************************************/

mlib_status mlib_v_convMxN_8nw(mlib_image *dstimg,              /* destination image */
                               mlib_image *srcimg,              /* source image */
                               mlib_s32 kwid,                   /* kernel height */
                               mlib_s32 khgt,                   /* kernel width */
                               mlib_s32 khw,
                               mlib_s32 khh,
                               mlib_s32 *kernel,                /* convolution kernel coeff array */
                               mlib_s32 discardbits,            /* LSBits discarded from 32-bit accum */
                               mlib_s32 cmask )
{
  mlib_u8   *srowaddr, *sa,                      /* src row addr */
            *drowaddr, *da,                      /* dst row addr */
            *buff_src,
            *sbuf,
            *s_buf;
  mlib_d64  *intrm_buf,                          /* fixed ptr to intermediate row buf */
            *ibuffh, *ddst;                      /* moving ptrs to intermediate buf */
  mlib_d64  round;
  mlib_u32  rval;
  int       dhgt, dwid, shgt, swid, amask;
  int       spxl_stride, dpxl_stride,  buf_slb;  /* y stride in pxls */
  int       algn_grp, i, j,
            gsr_scale;                           /* left pre-shift before fpack 16-bit R shift */

  struct    COEFF_REC *coeff_tbl;                /* tbl of coeffs & offsets to corres. pxls */
  int       coeff_cnt[8],                        /* num of coeffs in each alignment group */
            next_coeff;                          /* update after do each alignment group */
  void      (*p_proc_load)(mlib_u8*, mlib_u8*, mlib_s32, mlib_s32);
  void      (*p_proc_store)(mlib_u8*, mlib_u8*, mlib_s32, mlib_s32);
  int       nchannel, testchan, chan;

  /*
     *  The 8x16 mult has built-in 8-bit R shift, and fpack16 has 7-bit
     *  fixed R shift (preceded by variable-bit L shift controlled by GSR
     *  scalefactor field). Thus net R shift = (8+7)-(GSR.scalefactor_field),
     *  so GSR.scalefactor_field = 15-(net R shift):
     */
  gsr_scale = 31 - discardbits;

  rval = mlib_round_8[gsr_scale];  /* get accum init value for roundoff */
  round = vis_to_double_dup(rval);

  srowaddr =    (mlib_u8 *)mlib_ImageGetData( srcimg );  /* src row addr */
  drowaddr =    (mlib_u8 *)mlib_ImageGetData( dstimg );  /* dst row addr */
  shgt =        mlib_ImageGetHeight( srcimg );
  swid =        mlib_ImageGetWidth( srcimg );
  spxl_stride = mlib_ImageGetStride( srcimg );  /* source stride pxls */
  dpxl_stride = mlib_ImageGetStride( dstimg );  /* dest. stride pxls */
  nchannel = mlib_ImageGetChannels( srcimg );

  if (nchannel == 1) cmask = 1;
  amask = (1 << nchannel) - 1;
  if ((cmask & amask) == amask) {
    return mlib_convMxN_8nw_f(dstimg, srcimg, kwid, khgt, khw, khh, kernel, discardbits);
  }

  /* adjust destination address, hgt, wid to account for border pixels */

  drowaddr = drowaddr + (dpxl_stride * khh) + khw*nchannel;
  dwid = swid - (kwid-1);
  dhgt = shgt - (khgt-1);

  buf_slb = (swid+24) & (~7);

  /* allocate intermediate (row) buf: */

  buff_src = (mlib_u8 *)mlib_malloc(sizeof(mlib_u8)*(khgt+2)* buf_slb + 8);
  if (buff_src == NULL) {
    return( MLIB_FAILURE );
  }
  sbuf = (mlib_u8*) ((mlib_addr)(buff_src +8) & (~7));

  /* allocate & fill table of coeff's and offsets to corresponding pxls */

  coeff_tbl = (struct COEFF_REC *) mlib_malloc( sizeof(struct COEFF_REC) * (khgt * kwid) );
  if ( coeff_tbl == NULL ) {
    mlib_free(buff_src);
    return( MLIB_FAILURE );
  }

  mlib_conv8_fill_coeff_tbl( khgt, kwid, kernel,  /* kernel size, coeff's */
                             coeff_tbl,           /* table to fill in */
                             coeff_cnt,           /* # coeffs in each algn grp */
                             buf_slb );           /* buf stride in pxls */

  if (nchannel == 1) {
    p_proc_load = & mlib_my_load;
    p_proc_store = & mlib_my_store;
  } else if (nchannel == 2) {
    p_proc_load = & mlib_v_ImageChannelExtract_U8_21_D1;
    p_proc_store = & mlib_v_ImageChannelInsert_U8_12_D1;
  } else if (nchannel == 3) {
    p_proc_load = & mlib_v_ImageChannelExtract_U8_31_D1;
    p_proc_store = & mlib_v_ImageChannelInsert_U8_13_D1;
  } else {
    p_proc_load = & mlib_v_ImageChannelExtract_U8_41_D1;
    p_proc_store = & mlib_v_ImageChannelInsert_U8_14_D1;
  }

  testchan = 1;
  for ( chan = 0; chan < nchannel; chan++) {
    sa = srowaddr;
    da  = drowaddr;
    if ((cmask & testchan) == 0) {
      testchan <<= 1;
      continue;
    }

    intrm_buf = (mlib_d64*) (sbuf + khgt * buf_slb);
    s_buf = sbuf;

    for (i = 0; i < khgt-1; i++) {
      (*p_proc_load)(sa, s_buf, swid, testchan);  /* load khdt-1 (1 channel) rows from src */
      sa += spxl_stride;
      s_buf += buf_slb;
    }

    /*******************************/
    /*      do each output row     */
    /*******************************/

    for (j = 0; j < dhgt; j++) {
      (*p_proc_load)(sa, s_buf, swid, testchan); /* load next row */
      ibuffh = intrm_buf;                        /* init rowbuf to rounding value (near 0) */
      for ( i = 0; i < dwid; i += 8 ) {          /* 8 pixels at a time */
        ibuffh[0] = round;                       /* 1 d64 = 4 s16 intermed. pixels */
        ibuffh[1] = round;                       /* 1 d64 = 4 s16 intermed. pixels */
        ibuffh += 2;                             /* 2 more d64s done */
      }

      /**************************************************/
      /*    mult and accum to get one output row        */
      /**************************************************/

      next_coeff = 0;

      for ( algn_grp = 0; algn_grp < 8; algn_grp++ ) {

        mlib_conv8_algn_grp( intrm_buf,            /* aligned intermed. rowbuf */
                             dwid,                 /* # pxls to wrt to dest. row */
                             coeff_tbl,            /* tbl of coeffs and offsets */
                             next_coeff,           /* 1st coeff_tbl record to use */
                             coeff_cnt[algn_grp],  /* # coeff_tbl records in align grp */
                             sbuf,                 /* ptr to src row for 1st kernel row */
                             algn_grp );           /* 8-byte alignment of this group */

        next_coeff += coeff_cnt[algn_grp];         /* add coeff's just processed */
      }

      /**************************************************/
      /*    write intermediate buffer to dest image     */
      /**************************************************/

      ddst = ibuffh = intrm_buf;
      vis_write_gsr((gsr_scale << 3));

      for (i = 0; i < dwid; i += 8 ) {
        *ddst++ = vis_freg_pair(vis_fpack16(ibuffh[0]), vis_fpack16(ibuffh[1])); /* prepare result */
        ibuffh += 2;
      }
      (*p_proc_store)((mlib_u8*)intrm_buf, da, dwid, testchan);                  /* store result in dst */

      /**************************************************/
      /*          set up for next output row            */
      /**************************************************/

      for (i = 0; i < buf_slb*(khgt-1); i += 8) {
        *(mlib_d64 *)(sbuf + i) = *(mlib_d64*)(sbuf + i +  buf_slb); /* shift data in sbuf */
      }
      sa += spxl_stride;                                             /* start of next src row */
      da +=  dpxl_stride;                                            /* start of next dest row */
    }
    testchan <<= 1;
  }

  mlib_free(buff_src);
  mlib_free( coeff_tbl );
  return( MLIB_SUCCESS );
}

/***************************************************************/
