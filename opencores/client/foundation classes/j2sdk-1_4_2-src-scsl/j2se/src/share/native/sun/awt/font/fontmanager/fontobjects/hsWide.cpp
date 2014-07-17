/*
 * @(#)hsWide.cpp	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Copyright (C) 1996-1997 all rights reserved by HeadSpin Technology Inc. Chapel Hill, NC USA
 *
 * This software is the property of HeadSpin Technology Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * HEADSPIN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY
 * OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 *
 * This information in this software is subject to change without notice
*/

#include "hsWide.h"

/////////////////////////////////////////////////////////////////////////

static inline Boolean OverflowAdd(UInt32* sum, UInt32 a, UInt32 b)
{
	*sum = a + b;

	return (a | b) > *sum;	// true if overflow
}

/*
	Return the overflow from adding the three longs into a signed-wide
	
	wide = (high << 32) + (middle << 16) + low
*/
static inline Boolean SetWide3(hsWide* target, Int32 high, UInt32 middle, UInt32 low)
{
	hsAssert(high >= 0, "high is neg");

	target->fLo = low + (middle << 16);
	target->fHi = high + (middle >> 16) + (((low >> 16) + (UInt16)middle) >> 16);

	return target->fHi < 0;	// true if overflow
}

/////////////////////////////////////////////////////////////////////////

hsWide* hsWide::Mul(Int32 src1, Int32 src2)
{
	int neg = 0;
	
	if (src1 < 0)
	{	src1 = -src1;
		neg = ~0;
	}
	if (src2 < 0)
	{	src2 = -src2;
		neg = ~neg;
	}
	
	UInt32	a = src1 >> 16;
	UInt32	b = (UInt16)src1;
	UInt32	c = src2 >> 16;
	UInt32	d = (UInt16)src2;
	
	(void)SetWide3(this, a * c, a * d + c * b, b * d);

	if (neg)
		this->Negate();
	return this;
}

hsWide* hsWide::Mul(Int32 A)
{
	int		neg = 0;
	UInt32	B = fLo;
	Int32	C = fHi;
	Int32	tmp;
	UInt32	clo,blo,bhi,alo;

	if (A < 0)
	{	A = -A;
		neg = ~0;
	}
	if (WIDE_ISNEG(C, B))
	{	WIDE_NEGATE(C, B);
		neg = ~neg;
	}

	UInt32	ahi = A >> 16;
	UInt32	chi = C >> 16;
	if (ahi != 0 && chi != 0)
		goto OVER_FLOW;

	alo = (UInt16)A;
	bhi = B >> 16;
	blo = (UInt16)B;
	clo = (UInt16)C;

	tmp = alo * clo;
	if (tmp < 0 || SetWide3(this, tmp, alo * bhi, alo * blo))
		goto OVER_FLOW;

	if (chi != 0)
	{	UInt32	Vh = alo * chi;
		if (Vh >> 15)
			goto OVER_FLOW;
		if (((this->fHi >> 16) + (UInt16)Vh) >> 15)
			goto OVER_FLOW;
		this->fHi += Vh << 16;
	}
	else							// ahi != 0 && chi == 0
	{	hsWide	w;
		UInt32	Vh = ahi * clo;
		if (Vh >> 16)
			goto OVER_FLOW;
		tmp = ahi * bhi;
		if (tmp < 0 || SetWide3(&w, tmp, ahi * blo, 0))
			goto OVER_FLOW;
		if (((w.fHi >> 16) + (UInt16)Vh) >> 15)
			goto OVER_FLOW;
		w.fHi += Vh << 16;
		this->Add(&w);
	}
	
	if (neg)
		this->Negate();
	return this;

OVER_FLOW:
	*this = neg ? kNegInfinity64 : kPosInfinity64;
	return this;
}

hsWide* hsWide::Div(Int32 denom)
{
	if (denom == 0)
	{	if (this->IsNeg())
		{	hsSignalMathUnderflow();
			*this = kNegInfinity64;
		}
		else
		{	hsSignalMathOverflow();
			*this = kPosInfinity64;
		}
		return this;
	}

	int		neg = 0;
	Int32	resultH = 0;
	UInt32	resultL = 0;
	Int32	numerH = this->fHi;
	UInt32	numerL = this->fLo;

	if (denom < 0)
	{	denom = -denom;
		neg = ~0;
	}
	if (WIDE_ISNEG(numerH, numerL))
	{	WIDE_NEGATE(numerH, numerL);
		neg = ~neg;
	}
	
	WIDE_ADDPOS(numerH, numerL, denom >> 1);	// add denom/2 to get a round result

	UInt32	curr = (UInt32)numerH >> 31;

	for (int i = 0; i < 64; i++)
	{
		WIDE_SHIFTLEFT(resultH, resultL, resultH, resultL, 1);
		if (UInt32(denom) <= curr)
		{
			resultL |= 1;
			curr -= denom;
		}
		WIDE_SHIFTLEFT(numerH, numerL, numerH, numerL, 1);
		curr = (curr << 1) | ((UInt32)numerH >> 31);
	}

	if (neg)
		WIDE_NEGATE(resultH, resultL);
	return this->Set(resultH, resultL);
}

Int32 hsWide::Sqrt() const
{
	int		bits = 32;
	UInt32	root = 0;
	UInt32	valueH = (UInt32)fHi;
	UInt32	valueL = fLo;
	UInt32	currH = 0;
	UInt32	currL = 0;
	UInt32	guessH, guessL;
	
	do {
		WIDE_SHIFTLEFT(currH, currL, currH, currL, 2);
		currL |= TOP2BITS(valueH);
		WIDE_SHIFTLEFT(valueH, valueL, valueH, valueL, 2);		
		WIDE_SHIFTLEFT(guessH, guessL, 0, root, 2);
		root <<= 1;
		if (WIDE_LESSTHAN(guessH, guessL, currH, currL))
		{	WIDE_ADDPOS(guessH, guessL, 1);
			WIDE_SUBWIDE(currH, currL, guessH, guessL);
			root |= 1;
		}
	} while (--bits);

#if HS_PIN_MATH_OVERFLOW
	if ((Int32)root < 0)
		return kPosInfinity32;
#endif
	return (Int32)root;
}

Int32 hsWide::CubeRoot() const
{
	int		bits = 21;
	UInt32	root = 0;
	UInt32	valueH = (UInt32)fHi;
	UInt32	valueL = fLo;
	UInt32	currH, currL;
	UInt32	guessH, guessL;
	Boolean	neg = false;

	if (WIDE_ISNEG(valueH, valueL))
	{	neg = true;
		WIDE_NEGATE(valueH, valueL);
	}

	currH = currL = 0;
	WIDE_SHIFTLEFT(valueH, valueL, valueH, valueL, 1);	
	do {
		WIDE_SHIFTLEFT(currH, currL, currH, currL, 3);
		currL |= TOP3BITS(valueH);
		WIDE_SHIFTLEFT(valueH, valueL, valueH, valueL, 3);		

		root <<= 1;

		hsWide w;
		w.Mul(root, root)->Add(root);
	#if 0
		w.Mul(3);
	#else
		hsWide w2 = w;
		w.ShiftLeft(1)->Add(&w2);
	#endif
		guessH = (UInt32)w.fHi;
		guessL = w.fLo;

		if (WIDE_LESSTHAN(guessH, guessL, currH, currL))
		{	WIDE_ADDPOS(guessH, guessL, 1);
			WIDE_SUBWIDE(currH, currL, guessH, guessL);
			root |= 1;
		}
	} while (--bits);

	if (neg)
		root = -Int32(root);
	return (Int32)root;
}

