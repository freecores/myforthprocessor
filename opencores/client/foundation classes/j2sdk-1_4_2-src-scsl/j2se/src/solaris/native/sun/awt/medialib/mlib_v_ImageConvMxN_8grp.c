/*
 * @(#)mlib_v_ImageConvMxN_8grp.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageConvMxN_8grp.c	1.8	98/11/05 SMI"


/*
 * FUNCTION
 *	mlib_conv8_algn_grp -- Do multiplies and accumulates for
 *				one alignment group.
 *
 * SYNOPSIS
 *	mlib_status mlib_conv8_algn_grp(
 *	   mlib_d64 *intrm_buf,		/ aligned intermed. rowbuf /
 *	   int dwid,			/ # pxls to wrt to dest. row /
 *	   struct COEFF_REC *coeff_tbl,	/ tbl of coeffs and offsets /
 *	   int coeff_idx,		/ 1st coeff_tbl record to use / 
 *	   int coeff_cnt,		/ # of coeff_tbl records to use /
 *	   mlib_u8 *srowaddr,		/ 1st row of src to do /
 *	   int algn_boffs		/ 8-byte alignment of this group /
 *	   )
 *
 * DESCRIPTION
 *	The processing for an alignment group begins with an align address.
 *	Then the 16-bit accumulators are read from the rowbuf in memory,
 *	the coefficients in that alignment group are multiplied by their
 *	respective coefficients (pulled sequentially from the coefficient
 *	table), and the products are added to the accumulators, which are
 *	then stored back into the row buffer in memory. Each loop works 
 *	across the image from left to right. 
 *
 *	An inner loop within each alignment group processes 16 coefficients
 *	(from the coefficient table) at a time, until all coefficients with 
 *	that alignment have been pulled from the table and processed.
 *
 *	When the pixels in an alignment group are being multiplied and
 *	accumulated for an output point, 4 pixels are used at once, since
 *	the 8x16 VIS multiply uses a 32-bit input holding 4 8-bit values.
 *	These 4 pixels being multiplied and accumulated in parallel are
 *	contributing to 4 output pixels.
 *
 *	Since the data I/O is done with 64-bit chunks, 8 8-bit input pixels 
 *	are processed as a group, contributing to 8 output pixels.
 *
 *	To maximize pipelining of instructions through the execution units,
 *	two such blocks of 8 pixels each are processed within each inner 
 *	loop. Thus each inner loop operates on a blocks of 16 pixels, so that
 *	16 16-bit accumulators in the intermediate row buffer in memory 
 *	are updated after each iteration of an inner loop.
 *
 *	Using 16 pixels per block provides a larger the "pool" of data for
 *	the inner loop to work with. This enables a larger number of
 *	independent calculations to avoid data dependency stalls.
 *
 *	Pseudo-code:
 *
 *	    set GSR
 *	    coeff_left_in_grp = coeffs in alignment group
 *
 *	    while (coeff_left_in_grp > 8) {
 *	      for (coeff=0; coeff<8; coeff++) {
 *		pull next coeff from table, store in a reg
 *		pull next coeff offset from table
 *		use offset to calculate aligned ptr to pixel
 *			to multiply by that coeff
 *		use ptr for coeff to get 1st 4 pixels for that coeff
 *	      for each block of 16 pixels in output row {
 *		load 16 16-bit accumulators from memory rowbuf
 *		for (coeff=0; coeff<8; coeff++) {
 *		  multiply next 8 pixels by coeff
 *		  add 8 products to accumulators 1-8
 *		  use ptr for coeff to get next 8 pixels for that coeff
 *		}
 *		for (coeff=0; coeff<8; coeff++) {
 *		  multiply next 8 pixels by coeff
 *		  add 8 products to accumulators 9-16
 *		  use ptr for coeff to get next 8 pixels for that coeff
 *		}
 *		store 16 16-bit accumulators to memory rowbuf
 *		coeff_left_in_grp = coeff_left_in_grp - 8
 *	      } / end block of 16 pixels /
 *	    } / end while /
 *
 *	    repeat while loop steps for remaining 0-7 coeffs
 *
 *************************************************************************
 */

#include <stdlib.h>
#include <vis_proto.h>
#include <mlib_image.h>
#include "mlib_v_ImageConvMxN_8.h"

#define DBG_8BIT	0

#define DBG_IBUF	0

#if DBG_IBUF
#include "myvisutil.h"
#endif


#define	SETUP_COEFF_AND_PTR( kN, spN, prevN, adatN )		\
	kN = coeff_tbl[coeff_idx].coeff;			\
	spN = (mlib_d64 *)((mlib_addr)				\
		(srowaddr + coeff_tbl[coeff_idx++].spoffs) &	\
		 (~ 7));					\
	curr = *spN++; prevN = *spN++;				\
	adatN = vis_faligndata( curr, prevN )

#define	MULT_ACCUM_1_COEFF( sptr, prev, adat, ka, accx, accy )	\
	dath = vis_fmul8x16al( vis_read_hi(adat), ka );		\
	datl = vis_fmul8x16al( vis_read_lo(adat), ka );		\
	accx = vis_fpadd16( accx, dath );			\
	accy = vis_fpadd16( accy, datl );			\
	curr = *sptr++;						\
	adat = vis_faligndata( prev, curr );			\
	prev = curr

/* intermediate accumulator buffer load/store */
#define	LOAD_ACCUMS_FROM_INTERMEDIATE_BUF()			\
	acc0 = ibuffh[0]; acc1 = ibuffh[1];			\
	acc2 = ibuffh[2]; acc3 = ibuffh[3]

#define	STORE_ACCUMS_TO_INTERMEDIATE_BUF()			\
	*ibuffh = acc0; ibuffh += 2;				\
	*ibuffl = acc1; ibuffl += 2;				\
	*ibuffh = acc2; ibuffh += 2;				\
	*ibuffl = acc3; ibuffl += 2


void mlib_conv8_algn_grp(
	mlib_d64 *intrm_buf,		/* aligned intermed. rowbuf */
	int dwid,			/* # pxls to wrt to dest. row */
	struct COEFF_REC *coeff_tbl,	/* tbl of coeffs and offsets */
	int coeff_idx,			/* 1st coeff_tbl record to use */ 
	int coeff_cnt,			/* # of coeff_tbl records to use */
	mlib_u8 *srowaddr,		/* 1st row of src to do */
	int algn_boffs			/* 8-byte alignment of this group */
	);

/************************************************************************/

void mlib_conv8_algn_grp(

	mlib_d64 *intrm_buf,		/* aligned intermed. rowbuf */
	int dwid,			/* # pxls to wrt to dest. row */
	struct COEFF_REC *coeff_tbl,	/* tbl of coeffs and offsets */
	int coeff_idx,			/* 1st coeff_tbl record to use */ 
	int coeff_cnt,			/* # of coeff_tbl records to use */
	mlib_u8 *srowaddr,		/* 1st row of src to do */
	int algn_boffs			/* 8-byte alignment of this group */
	)
{
  mlib_d64	*ibuffh,*ibuffl;	/* moving ptrs to intermediate buf */
  mlib_d64	*sp1, *sp2, *sp3,	/* src ptrs (increment along the row) */
		*sp4, *sp5, *sp6, *sp7, *sp8;
  mlib_d64	prev1, prev2, prev3, prev4, prev5, prev6, prev7, prev8, curr,
		adat1, adat2, adat3, adat4, adat5, adat6, adat7, adat8, 
		dath, datl,
		acc0, acc1, acc2, acc3;
  mlib_f32	k1, k2, k3, k4, k5, k6, k7, k8;	/* coefs in regs */
  int		i;
  
  sp1 = vis_alignaddr( srowaddr, algn_boffs );

  while ( coeff_cnt >= 8 ) {

    ibuffh = intrm_buf;		/* setup ibuff ptrs for this pass along rows */
    ibuffl = intrm_buf + 1;

    /* load coeff's into regs; set up "moving" ptrs to pxls;
     * get 1st aligned data from each ptr. (Use and incr coeff_idx.)
     */
#if DBG_8BIT
	k1 = coeff_tbl[coeff_idx].coeff;
	sp1 = (mlib_d64 *)((mlib_addr)
		(srowaddr + coeff_tbl[coeff_idx++].spoffs) &
		 (~ 7));
	curr = *sp1++; prev1 = *sp1++;
	adat1 = vis_faligndata( curr, prev1 );
#else
    SETUP_COEFF_AND_PTR( k1, sp1, prev1, adat1 );
#endif
    SETUP_COEFF_AND_PTR( k2, sp2, prev2, adat2 );
    SETUP_COEFF_AND_PTR( k3, sp3, prev3, adat3 );
    SETUP_COEFF_AND_PTR( k4, sp4, prev4, adat4 );
    SETUP_COEFF_AND_PTR( k5, sp5, prev5, adat5 );
    SETUP_COEFF_AND_PTR( k6, sp6, prev6, adat6 );
    SETUP_COEFF_AND_PTR( k7, sp7, prev7, adat7 );
    SETUP_COEFF_AND_PTR( k8, sp8, prev8, adat8 );

    for ( i = 0; i < dwid; i += PXLS_PER_BLOCK ) {
      LOAD_ACCUMS_FROM_INTERMEDIATE_BUF();
      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp4, prev4, adat4, k4, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp5, prev5, adat5, k5, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp6, prev6, adat6, k6, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp7, prev7, adat7, k7, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp8, prev8, adat8, k8, acc0, acc1 );

      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp4, prev4, adat4, k4, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp5, prev5, adat5, k5, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp6, prev6, adat6, k6, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp7, prev7, adat7, k7, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp8, prev8, adat8, k8, acc2, acc3 );
      STORE_ACCUMS_TO_INTERMEDIATE_BUF();
    }
#if DBG_IBUF
	printf("\n coeff_cnt=%d\n IBUFF:\n", coeff_cnt);
	ibuffh = intrm_buf;
	for ( i = 0; i < dwid; i += 4 ) {
	    acc0 = *ibuffh++;
	    ps4_64(" ",acc0);
	}
#endif
    coeff_cnt -= 8;
  }

  ibuffh = intrm_buf;		/* setup ibuff ptrs for last pass along rows */
  ibuffl = intrm_buf + 1;

  if ( coeff_cnt == 7 ) {

    /* load coeff's into regs; set up "moving" ptrs to pxls;
     * get 1st aligned data from each ptr. (Use and incr coeff_idx.)
     */
#if DBG_8BIT
	k1 = coeff_tbl[coeff_idx].coeff;
	sp1 = (mlib_d64 *)((mlib_addr)
		(srowaddr + coeff_tbl[coeff_idx++].spoffs) &
		 (~ 7));
	curr = *sp1++; prev1 = *sp1++;
	adat1 = vis_faligndata( curr, prev1 );
#else
    SETUP_COEFF_AND_PTR( k1, sp1, prev1, adat1 );
#endif
    SETUP_COEFF_AND_PTR( k2, sp2, prev2, adat2 );
    SETUP_COEFF_AND_PTR( k3, sp3, prev3, adat3 );
    SETUP_COEFF_AND_PTR( k4, sp4, prev4, adat4 );
    SETUP_COEFF_AND_PTR( k5, sp5, prev5, adat5 );
    SETUP_COEFF_AND_PTR( k6, sp6, prev6, adat6 );
    SETUP_COEFF_AND_PTR( k7, sp7, prev7, adat7 );

    for ( i = 0; i < dwid; i += PXLS_PER_BLOCK ) {
      LOAD_ACCUMS_FROM_INTERMEDIATE_BUF();
      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp4, prev4, adat4, k4, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp5, prev5, adat5, k5, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp6, prev6, adat6, k6, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp7, prev7, adat7, k7, acc0, acc1 );

      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp4, prev4, adat4, k4, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp5, prev5, adat5, k5, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp6, prev6, adat6, k6, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp7, prev7, adat7, k7, acc2, acc3 );
      STORE_ACCUMS_TO_INTERMEDIATE_BUF();
    }
  }

  else if ( coeff_cnt == 6 ) {

    /* load coeff's into regs; set up "moving" ptrs to pxls;
     * get 1st aligned data from each ptr. (Use and incr coeff_idx.)
     */
    SETUP_COEFF_AND_PTR( k1, sp1, prev1, adat1 );
    SETUP_COEFF_AND_PTR( k2, sp2, prev2, adat2 );
    SETUP_COEFF_AND_PTR( k3, sp3, prev3, adat3 );
    SETUP_COEFF_AND_PTR( k4, sp4, prev4, adat4 );
    SETUP_COEFF_AND_PTR( k5, sp5, prev5, adat5 );
    SETUP_COEFF_AND_PTR( k6, sp6, prev6, adat6 );

    for ( i = 0; i < dwid; i += PXLS_PER_BLOCK ) {
      LOAD_ACCUMS_FROM_INTERMEDIATE_BUF();
      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp4, prev4, adat4, k4, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp5, prev5, adat5, k5, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp6, prev6, adat6, k6, acc0, acc1 );

      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp4, prev4, adat4, k4, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp5, prev5, adat5, k5, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp6, prev6, adat6, k6, acc2, acc3 );
      STORE_ACCUMS_TO_INTERMEDIATE_BUF();
    }
  }

  else if ( coeff_cnt == 5 ) {

    /* load coeff's into regs; set up "moving" ptrs to pxls;
     * get 1st aligned data from each ptr. (Use and incr coeff_idx.)
     */
    SETUP_COEFF_AND_PTR( k1, sp1, prev1, adat1 );
    SETUP_COEFF_AND_PTR( k2, sp2, prev2, adat2 );
    SETUP_COEFF_AND_PTR( k3, sp3, prev3, adat3 );
    SETUP_COEFF_AND_PTR( k4, sp4, prev4, adat4 );
    SETUP_COEFF_AND_PTR( k5, sp5, prev5, adat5 );

    for ( i = 0; i < dwid; i += PXLS_PER_BLOCK ) {
      LOAD_ACCUMS_FROM_INTERMEDIATE_BUF();
      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp4, prev4, adat4, k4, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp5, prev5, adat5, k5, acc0, acc1 );

      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp4, prev4, adat4, k4, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp5, prev5, adat5, k5, acc2, acc3 );
      STORE_ACCUMS_TO_INTERMEDIATE_BUF();
    }
  }

  else if ( coeff_cnt == 4 ) {

    /* load coeff's into regs; set up "moving" ptrs to pxls;
     * get 1st aligned data from each ptr. (Use and incr coeff_idx.)
     */
    SETUP_COEFF_AND_PTR( k1, sp1, prev1, adat1 );
    SETUP_COEFF_AND_PTR( k2, sp2, prev2, adat2 );
    SETUP_COEFF_AND_PTR( k3, sp3, prev3, adat3 );
    SETUP_COEFF_AND_PTR( k4, sp4, prev4, adat4 );

    for ( i = 0; i < dwid; i += PXLS_PER_BLOCK ) {
      LOAD_ACCUMS_FROM_INTERMEDIATE_BUF();
      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp4, prev4, adat4, k4, acc0, acc1 );

      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp4, prev4, adat4, k4, acc2, acc3 );
      STORE_ACCUMS_TO_INTERMEDIATE_BUF();
    }
  }

  else if ( coeff_cnt == 3 ) {

    /* load coeff's into regs; set up "moving" ptrs to pxls;
     * get 1st aligned data from each ptr. (Use and incr coeff_idx.)
     */
    SETUP_COEFF_AND_PTR( k1, sp1, prev1, adat1 );
    SETUP_COEFF_AND_PTR( k2, sp2, prev2, adat2 );
    SETUP_COEFF_AND_PTR( k3, sp3, prev3, adat3 );

    for ( i = 0; i < dwid; i += PXLS_PER_BLOCK ) {
      LOAD_ACCUMS_FROM_INTERMEDIATE_BUF();
      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc0, acc1 );

      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp3, prev3, adat3, k3, acc2, acc3 );
      STORE_ACCUMS_TO_INTERMEDIATE_BUF();
    }
  }

  else if ( coeff_cnt == 2 ) {

    /* load coeff's into regs; set up "moving" ptrs to pxls;
     * get 1st aligned data from each ptr. (Use and incr coeff_idx.)
     */
    SETUP_COEFF_AND_PTR( k1, sp1, prev1, adat1 );
    SETUP_COEFF_AND_PTR( k2, sp2, prev2, adat2 );

    for ( i = 0; i < dwid; i += PXLS_PER_BLOCK ) {
      LOAD_ACCUMS_FROM_INTERMEDIATE_BUF();
      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc0, acc1 );

      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc2, acc3 );
      MULT_ACCUM_1_COEFF( sp2, prev2, adat2, k2, acc2, acc3 );
      STORE_ACCUMS_TO_INTERMEDIATE_BUF();
    }
  }

  else if ( coeff_cnt == 1 ) {

    /* load coeff's into regs; set up "moving" ptrs to pxls;
     * get 1st aligned data from each ptr. (Use and incr coeff_idx.)
     */
    SETUP_COEFF_AND_PTR( k1, sp1, prev1, adat1 );

    for ( i = 0; i < dwid; i += PXLS_PER_BLOCK ) {
      LOAD_ACCUMS_FROM_INTERMEDIATE_BUF();
      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc0, acc1 );
      MULT_ACCUM_1_COEFF( sp1, prev1, adat1, k1, acc2, acc3 );
      STORE_ACCUMS_TO_INTERMEDIATE_BUF();
    }
  }

#if DBG_IBUF
	printf("\n coeff_cnt=%d\n IBUFF:\n", coeff_cnt);
	ibuffh = intrm_buf;
	for ( i = 0; i < dwid; i += 4 ) {
	    acc0 = *ibuffh++;
	    ps4_64(" ",acc0);
	}
#endif
}

