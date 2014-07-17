/*
 * @(#)mlib_v_ImageConvMxN_8fill.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageConvMxN_8fill.c	1.7	98/09/02 SMI"


/*
 * FUNCTION
 *	mlib_conv8_fill_coeff_tbl -- Classify coefficients by alignment,
 *			fill in table coefficients and offsets of
 *			corresponding pixels from upper left pixel.
 *
 * SYNOPSIS
 *	void mlib_conv8_fill_coeff_tbl(
 *	   int khgt, int kwid,	      / kernel dimensions /
 *	   mlib_s32 *kernel,	      / coefficients /
 *	   struct COEFF_REC *coeff_tbl,  / table to fill w/ coeffs, offsets /
 *	   int coeff_cnt[],	      / fill with # coeffs in each algn grp /
 *	   int spxl_stride);	      / src stride in pxls /
 *
 * DESCRIPTION
 *	A helper routine for 2-D MxN convolution for 8-bit images.
 *
 *	The routine breaks the data into 8 alignment groups based on each
 *	pixel's offset from an 8-byte boundary (0, 1, 2,.., or 7 bytes of
 *	offset). Alignment group 0 is defined as the coeff in row 0, col 0,
 *	and all coeff's whose corresponding pixels have the same offset
 *	from an 8-byte bound as that for this upper left coeff. Group 1 is
 *	defined as the coeff in row 0, col 1, and all other coeff's whose
 *	pixels have the same alignment. Group 2 is defined by the coeff in
 *	row 0, col 2, and Group 3 is defined by the coeff in row 0, col 3,
 *	etc. The coefficients are stored in a run-time table, ordered by
 *	alignment group: first the Group 0 coeffs, then Group 1, etc.
 *
 *	Along with each coefficient, the table stores offsets to the pixel
 *	corresponding to each coefficient. These offsets are in pixels, and
 *	relative to the pixel corresponding to the coefficient in kernel
 *	row 0, col 0. The offsets are used to set up one pointer for each
 *	coefficient. Each pointer is used to access the pixels to be
 *	multiplied by that coefficient, as the convolution proceeds along
 *	the row.
 *
 *	The heart of this routine is the formula to calculate the offsets
 *	for each coefficient:
 *
 *	offset = (kernel row * source stride in pixels) +
 *		 (((kernel row & 0x07) * (8-rel_algn) + algn_grp) & 0x07);
 *	where:
 *	    1)	1st kernel row is row zero.
 *	    2)	rel_algn = spxl_stride & 0x07;	/ relative alignment in pxls /
 *	    3)	algn_grp: as defined above.
 *
 *	NOTE:	To illustrate the concept of relative alignment, the diagram
 *		below shows alignment of pixels corresponding to a 7x7 kernel
 *		for a source image with a relative alignment of one pixel.
 *		The vertical lines at 8-pixel intervals are 8-byte boundaries.
 *		The upper left pixel just happens to start on an 8-byte bound.
 *
 *	***************************************************************
 *			    Relative Alignment = 1 pixel
 *		  (successive rows have a relative offset = 1 pixel)
 *
 *				alignment group
 *	     A   B   C   D    E   F   G   H     A  B  C  D  E
 *	   | 1   2   3   4    5   6   7   .  |  .  .  .  .  .
 *	   | .   8   9  10   11  12  13  14  |  .  .  .  .  .
 *	   | .   .  15  16   17  18  19  20  | 21  .  .  .  .
 *	   | .   .   .  22   23  24  25  26  | 27 28  .  .  .
 *	   | .   .   .   .   29  30  31  32  | 33 34 35  .  .
 *	   | .   .   .   .    .  36  37  38  | 39 40 41 42  .
 *	   | .   .   .   .    .   .  43  44  | 45 46 47 48 49
 *
 *
 *************************************************************************
 */

#include <vis_proto.h>
#include <mlib_image.h>
#include "mlib_v_ImageConvMxN_8.h"

void mlib_conv8_fill_coeff_tbl( int khgt, int kwid, mlib_s32 *kernel,
				  struct COEFF_REC *coeff_tbl,
				  int *coeff_cnt, int spxl_stride );


/*-------------------------------------------------------------------------*/

void mlib_conv8_fill_coeff_tbl(

	int khgt, int kwid,	      /* kernel dimensions */
	mlib_s32 *kernel,	      /* coefficients */
	struct COEFF_REC *coeff_tbl,  /* fill with coeffs and pxl offsets */
	int coeff_cnt[],	      /* fill with # coeffs in each algn grp */
	int spxl_stride		      /* src stride in pxls */
	)
{
  /*  Fill table of coeff's and offsets:
   *
   *  (Divide coeff's into the 8 alignment groups -- 8 possible offsets
   *   from 8-byte bound. All group 0 coeffs go into coeff_tbl 1st, then
   *   all group 1 coeffs, then group 2, then group 3, etc.)
   */
  int	rel_algn,		/* relative alignment (row-to-row) */
	eight_minus_rel_algn,	/* used repeatedly */
	algn_grp,		/* alignment group */
	kcnt,			/* count coeff's in alignment group */
	coeff_idx,		/* index into coeff_tbl */
	sloffs,			/* offset of src line from 1st of khgt rows */
	krow,			/* kernel row */
	kbase,			/* index into kernel[] of 1st coeff in row */
	koffs;			/* offset of coeff from 1st coeff in row */
  mlib_u16 kku16;		/* u16 form of coeff */

  rel_algn = spxl_stride & 0x07;	/* relative alignment (row-to-row) */
  eight_minus_rel_algn = 8 - rel_algn;	/* used repeatedly */
  coeff_idx = 0;			/* start at beginning of coeff_tbl */

  for ( algn_grp = 0; algn_grp < 8; algn_grp++ ) {

    kcnt = 0;			/* no coeff's in this algn grp yet */
    /* start with 1st kernel row, with kernel at left edge of image: */
    sloffs = 0;			/* offset of curr src line from pxl
				   corresponding to uppr left coeff */

    for ( krow = 0; krow < khgt; krow++ ) {

      /* find 1st coeff in this row that's in this alignment group:
       * (find offset from very 1st coeff in this kernel row)
       */
      koffs = (  ((krow & 0x07) * eight_minus_rel_algn) + algn_grp  ) & 0x07;

      kbase = kwid * krow;		/* 1st coeff in current kernel row */

      while ( koffs < kwid ) {		/* while in current kernel row */

	/* get next coeff in current algn_grp; caste for shift: */
	kku16 = (mlib_u16)(kernel[kbase + koffs] >> 16);

	/* store f32 coeff pair, and offset to corresponding src pixel: */
	coeff_tbl[coeff_idx].coeff = vis_to_float( (kku16 << 16) | (kku16 & 0x0000ffff ));
	coeff_tbl[coeff_idx].spoffs = sloffs + koffs; /* offs to+within line */
	coeff_idx++;

	koffs += 8;		/* next coeff in algn_grp is 8 pxls away */
	kcnt++;			/* count coeff in algn_grp */
      }

      sloffs += spxl_stride;	/* inc offs to 1st pxl of next src row */

    } /* end for (krow..) */

    coeff_cnt[algn_grp] = kcnt;

  } /* end for (algn_grp..) */
}

