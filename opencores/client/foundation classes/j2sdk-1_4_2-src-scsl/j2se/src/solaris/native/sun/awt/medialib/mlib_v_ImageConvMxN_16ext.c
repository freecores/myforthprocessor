/*
 * @(#)mlib_v_ImageConvMxN_16ext.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#pragma ident  "@(#)mlib_v_ImageConvMxN_16ext.c	1.7  98/10/20 SMI"


/*
 * FUNCTION
 *      mlib_v_convMxN_16ext - convolve a 16-bit image, MxN kernel,
 *                             edge = src extended
 *
 * SYNOPSIS
 *      mlib_status
 *      mlib_v_convMxN_16ext( mlib_image *srcimg, mlib_image *dstimg,
 *                            mlib_s32 kwid, mlib_s32 khgt,
 *                            mlib_s32 dx_l, mlib_s32 dx_r,
 *                            mlib_s32 dy_t, mlib_s32 dy_b,
 *                            mlib_s32 *skernel, mlib_s32 discardbits
 *                            mlib_s32 cmask)
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
 *      A 2-D convolution (MxN kernel) for 16-bit images.
 *
 *************************************************************************
 */

#include <stdlib.h>
#include <vis_proto.h>
#include <mlib_image.h>
#include "mlib_v_ImageConvMxN_16.h"

static
mlib_u32 mlib_round_16[16] = {
  0x00008000, 0x00004000, 0x00002000, 0x00001000,
  0x00000800, 0x00000400, 0x00000200, 0x00000100,
  0x00000080, 0x00000040, 0x00000020, 0x00000010,
  0x00000008, 0x00000004, 0x00000002, 0x00000001 };


void mlib_v_ImageCopy_na (mlib_u8*, mlib_u8*, mlib_s32);
void mlib_v_ImageChannelExtract_S16_21_D1 (mlib_s16*, mlib_s16*, mlib_s32, mlib_s32);
void mlib_v_ImageChannelExtract_S16_31_D1 (mlib_s16*, mlib_s16*, mlib_s32, mlib_s32);
void mlib_v_ImageChannelExtract_S16_41_D1 (mlib_s16*, mlib_s16*, mlib_s32, mlib_s32);
void mlib_v_ImageChannelInsert_S16_12_D1 (mlib_s16*, mlib_s16*, mlib_s32, mlib_s32);
void mlib_v_ImageChannelInsert_S16_13_D1 (mlib_s16*, mlib_s16*, mlib_s32, mlib_s32);
void mlib_v_ImageChannelInsert_S16_14_D1 (mlib_s16*, mlib_s16*, mlib_s32, mlib_s32);

static void mlib_my_load (mlib_s16* src, mlib_s16* dst, mlib_s32 size, mlib_s32 cmask)
{
  mlib_v_ImageCopy_na ((mlib_u8*)src, (mlib_u8*)dst, 2*size);
}
static void mlib_my_store (mlib_s16* src, mlib_s16* dst, mlib_s32 size, mlib_s32 cmask)
{
  mlib_v_ImageCopy_na ((mlib_u8*)src, (mlib_u8*)dst, 2*size);
}


/*-----------------------------------------------------------------------*/
mlib_status
mlib_v_convMxN_16ext( mlib_image *dstimg,              /* destination image */
                      mlib_image *srcimg,              /* source image */
                      mlib_s32 kwid,                   /* kernel height */
                      mlib_s32 khgt,                   /* kernel width */
                      mlib_s32 dx_l,
                      mlib_s32 dx_r,
                      mlib_s32 dy_t,
                      mlib_s32 dy_b,
                      mlib_s32 *kernel,                /* convolution kernel coeff array */
                      mlib_s32 discardbits,            /* LSBits discarded from 32-bit accum */
                      mlib_s32 cmask )
{
    mlib_s16  *srowaddr, *sa,                     /* src row addr */
              *drowaddr, *da,                     /* dst row addr */
              *buff_src,
              *sbuf;
    mlib_d64  *intrm_buf,                         /* fixed ptr to intermediate row buf */
              *ibuffh, *ddst;                     /* moving ptrs to intermediate buf */
    mlib_d64  round;
    mlib_u32  rval;
    int       dhgt, dwid, shgt, swid;
    int       spxl_stride, dpxl_stride, buf_slb;  /* y strides in pxls */
    int       i, j, n, ikw, ikh,
              gsr_scale;                          /* left pre-shift before fpack 16-bit R shift */
    void      (*p_proc_load)(mlib_s16*, mlib_s16*, mlib_s32, mlib_s32);
    void      (*p_proc_store)(mlib_s16*, mlib_s16*, mlib_s32, mlib_s32);
    int       nchannel, testchan, chan;
    mlib_d64  **ibuffp, *itmp;
    mlib_f32  *fk;



    /* calc L pre-shift before fpack 16-bit R shift */
    gsr_scale = 32 - discardbits;

    rval = mlib_round_16[gsr_scale];  /* get accum init value for roundoff */
    round = vis_to_double_dup(rval);

    srowaddr =    (mlib_s16 *)mlib_ImageGetData( srcimg );  /* src row addr */
    drowaddr =    (mlib_s16 *)mlib_ImageGetData( dstimg );  /* dst row addr */
    shgt =        mlib_ImageGetHeight( srcimg );
    swid =        mlib_ImageGetWidth( srcimg );
    spxl_stride = mlib_ImageGetStride( srcimg ) >> 1;  /* source stride pxls */
    dpxl_stride = mlib_ImageGetStride( dstimg ) >> 1;  /* dest. stride pxls */
    nchannel = mlib_ImageGetChannels( srcimg );

    /* adjust destination address, hgt, wid to account for border pixels */

    dwid = swid;
    dhgt = shgt;
    swid += (kwid-1);

    fk = (mlib_f32 *)mlib_malloc( sizeof(mlib_f32) * khgt * kwid );
    if ( fk==NULL ) return( MLIB_NULLPOINTER );

    n = 0;
    for ( ikw = 0; ikw < khgt; ikw++ ) {
      for ( ikh = 0; ikh < kwid; ikh++ ) {
         fk[n] = vis_to_float((kernel[n] & 0xffff0000) |
                             ((kernel[n] >> 16) & 0x0000ffff));
        n++;
      }
    }

    buf_slb = (((swid << 1)+24) & (~7)) >> 1;

    /* allocate intermediate (row) buf: */

    buff_src = (mlib_s16 *)mlib_malloc(sizeof(mlib_s16)*(khgt+2)* buf_slb + 8);
    sbuf = (mlib_s16*) ((mlib_addr)(buff_src +8) & (~7));
    if (buff_src == NULL) {
      mlib_free(fk);
      return( MLIB_FAILURE );
    }

    ibuffp = (mlib_d64 **)mlib_malloc(sizeof(mlib_d64 *)*khgt);
    if (ibuffp == NULL) {
      mlib_free(fk);
      mlib_free(buff_src);
      return( MLIB_FAILURE );
    }

    if (nchannel == 1) {
      p_proc_load = &mlib_my_load;
      p_proc_store = &mlib_my_store;
    } else if (nchannel == 2) {
      p_proc_load = &mlib_v_ImageChannelExtract_S16_21_D1;
      p_proc_store = &mlib_v_ImageChannelInsert_S16_12_D1;
    } else if (nchannel == 3) {
      p_proc_load = &mlib_v_ImageChannelExtract_S16_31_D1;
      p_proc_store = &mlib_v_ImageChannelInsert_S16_13_D1;
    } else {
      p_proc_load = &mlib_v_ImageChannelExtract_S16_41_D1;
      p_proc_store = &mlib_v_ImageChannelInsert_S16_14_D1;
    }

    swid -= (dx_l + dx_r);

    testchan = 1;
    for ( chan = 0; chan < nchannel; chan++) {
      sa = srowaddr;
      da  = drowaddr;
      if ((cmask & testchan) == 0) {
        testchan <<= 1;
        continue;
      }

      for (i = 0; i < khgt; i++)            /* init rowbuf ptrs */
      ibuffp[i] = (mlib_d64*) &sbuf[buf_slb * i];

      intrm_buf = (mlib_d64*) (sbuf + khgt * buf_slb);

      for (i = 0; i < khgt-1; i++) {
        (*p_proc_load)(sa, ((mlib_s16*)ibuffp[i] + dx_l), swid, testchan);  /* load khgt-1 (1 channel) rows src */
        for (j = 0; j < dx_l; j++) {
          *((mlib_s16*)ibuffp[i] + j) = *((mlib_s16*)ibuffp[i] + dx_l);      /* extend edges */
        }
        for (j = 0; j < dx_r; j++) {
          *((mlib_s16*)ibuffp[i] + swid + dx_l + j) = *((mlib_s16*)ibuffp[i] + swid + dx_l - 1);
        }

        if ((i > (dy_t - 1)) && (i < dhgt + khgt - dy_b - 2)) {
          sa += spxl_stride;
        }
      }

      /*******************************/
      /*      do each output row     */
      /*******************************/

      for (j = 0; j < dhgt; j++) {
        (*p_proc_load)(sa, ((mlib_s16*)ibuffp[khgt-1] + dx_l), swid, testchan);   /* load next row */
        for (i = 0; i < dx_l; i++) {
          *((mlib_s16*)ibuffp[khgt-1] + i) = *((mlib_s16*)ibuffp[khgt-1] + dx_l); /* extend edges */
        }
        for (i = 0; i < dx_r; i++) {
          *((mlib_s16*)ibuffp[khgt-1] + swid + dx_l + i) = *((mlib_s16*)ibuffp[khgt-1] + swid + dx_l - 1);
        }

        ibuffh = intrm_buf;                        /* init rowbuf to rounding value (near 0) */
        for ( i = 0; i < dwid; i += 4 ) {          /* 4 pixels at a time */
          ibuffh[0] = round;                       /* 1 d64 = 2 f32 intermed. pixels */
          ibuffh[1] = round;                       /* 1 d64 = 2 f32 intermed. pixels */
          ibuffh += 2;                             /* 2 more d64s done */
        }

        /**************************************************/
        /*    mult and accum to get one output row        */
        /**************************************************/

        ONE_ROW_CONV_MxN(khgt, kwid, fk, ibuffp, intrm_buf, dwid)

        /**************************************************/
        /*    write intermediate buffer to dest image     */
        /**************************************************/

        ddst = ibuffh = intrm_buf;
        vis_write_gsr((gsr_scale << 3));

        for (i = 0; i < dwid; i += 4 ) {
          *ddst++ = vis_freg_pair(vis_fpackfix(ibuffh[0]), vis_fpackfix(ibuffh[1])); /* prepare result */
          ibuffh += 2;
        }
        (*p_proc_store)((mlib_s16*)intrm_buf, da, dwid, testchan);                   /* store result in dst */

        /************************************************/
        /*          set up for next output row          */
        /************************************************/
        itmp = ibuffp[0];                    /* rotate rowbuf ptrs */
        for (i = 0; i < khgt-1; i++)
          ibuffp[i] = ibuffp[i+1];
        ibuffp[khgt-1] = itmp;
        if (j < dhgt - dy_b - 1) {
          sa += spxl_stride;
        }                                                              /* start of next src row */
        da +=  dpxl_stride;                                            /* start of next dest row */
      }
      testchan <<= 1;
    }
    mlib_free(buff_src);
    mlib_free(fk );
    mlib_free(ibuffp );
    return( MLIB_SUCCESS );
}
