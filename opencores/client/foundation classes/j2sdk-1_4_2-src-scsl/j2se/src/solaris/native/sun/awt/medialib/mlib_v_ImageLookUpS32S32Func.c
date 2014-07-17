/*
 * @(#)mlib_v_ImageLookUpS32S32Func.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#pragma ident	"@(#)mlib_v_ImageLookUpS32S32Func.c	1.2	98/09/11 SMI"

/*
 * $RCSfile: mlib_v_ImageLookUp32S32Func.c,v $
 * $Revision: 4.1 $
 * $Date: 1996/12/11 20:44:13 $
 * $Author: liang $
 */




#include <stdlib.h>
#include "mlib_image.h"

/***************************************************************/

void mlib_v_ImageLookUp_S32_S32(mlib_s32 *src, mlib_s32 slb,
                                mlib_s32 *dst, mlib_s32 dlb,
                                mlib_s32 xsize, mlib_s32 ysize,
                                mlib_s32 **table, mlib_s32 csize )
{
  mlib_s32 i, j, k;

  dlb >>= 2; slb >>= 2;

  if (xsize < 2) {
    for(j = 0; j < ysize; j++, dst += dlb, src += slb){
      for(k = 0; k < csize; k++) {
        mlib_s32 *da = dst + k;
        mlib_s32 *sa = src + k;
        mlib_s32 *tab = &table[k][(mlib_u32)2147483648];

        for(i = 0; i < xsize; i++, da += csize, sa += csize)
        *da=tab[*sa];
      }
    }
  } else {
    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
#pragma pipeloop(0)
      for(k = 0; k < csize; k++) {
        mlib_s32 *da = dst + k;
        mlib_s32 *sa = src + k;
        mlib_s32 *tab = &table[k][(mlib_u32)2147483648];
        mlib_s32 s0, t0, s1, t1;

        s0 = sa[0];
        s1 = sa[csize];
        sa += 2*csize;

        for(i = 0; i < xsize - 3; i+=2, da += 2*csize, sa += 2*csize) {
          t0 = tab[s0];
          t1 = tab[s1];
          s0 = sa[0];
          s1 = sa[csize];
          da[0] = t0;
          da[csize] = t1;
        }
        t0 = tab[s0];
        t1 = tab[s1];
        da[0] = t0;
        da[csize] = t1;
        if (xsize & 1) da[2*csize] = tab[sa[0]];
      }
    }
  }
}


