/*
 * @(#)hsRectangle.cpp	1.6 03/01/23
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

#include "hsGeometry.h"
#include "hsFixedTypes.h"
#if HS_SCALAR_IS_FIXED
	#include "hsWide.h"
#endif

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

hsRect* hsRect::Set(const hsPoint2* p1, const hsPoint2* p2)
{
	if (p1->fX < p2->fX)
	{	fLeft		= p1->fX;
		fRight	= p2->fX;
	}
	else
	{	fLeft		= p2->fX;
		fRight	= p1->fX;
	}
	
	if (p1->fY < p2->fY)
	{	fTop		= p1->fY;
		fBottom	= p2->fY;
	}
	else
	{	fTop		= p2->fY;
		fBottom	= p1->fY;
	}

	return this;
}

hsRect* hsRect::Set(UInt32 count, const hsPoint2 p[])
{
	if (count > 0)
	{	fLeft = fRight = p->fX;
		fTop = fBottom = p->fY;

		(void)this->Union(count - 1, &p[1]);
	}
	return this;
}

////////////////////////////////////////////////////////////////////////////////

hsRect* hsRect::Move(hsScalar dx, hsScalar dy)
{
	fLeft 	+= dx;
	fTop		+= dy;
	fRight	+= dx;
	fBottom	+= dy;
	
	return this;
}

hsRect* hsRect::Inset(hsScalar dx, hsScalar dy)
{
	fLeft		+= dx;
	fTop		+= dy;
	fRight	-= dx;
	fBottom	-= dy;
	
	return this;
}

Boolean hsRect::Intersect(const hsRect *r)
{
	if (r->fLeft < fRight && r->fTop < fBottom && fLeft < r->fRight && fTop < r->fBottom)
	{
		if (r->fLeft > fLeft)		fLeft = r->fLeft;
		if (r->fTop > fTop)		fTop = r->fTop;
		if (r->fRight < fRight)	fRight = r->fRight;
		if (r->fBottom < fBottom)	fBottom = r->fBottom;

		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////

hsRect *hsRect::Union(const hsRect *r)
{
	if (r->fLeft < fLeft)		fLeft = r->fLeft;
	if (r->fTop < fTop)		fTop = r->fTop;
	if (r->fRight > fRight)	fRight = r->fRight;
	if (r->fBottom > fBottom)	fBottom = r->fBottom;

	return this;
}

hsRect* hsRect::Union(const hsPoint2* p)
{
	if (p->fX < fLeft)	fLeft = p->fX;
	if (p->fX > fRight)	fRight = p->fX;
	if (p->fY < fTop)	fTop = p->fY;
	if (p->fY> fBottom)	fBottom = p->fY;
	return this;	
}

/*
	Expands this to include all of the points
	It treats the first point differently, checking all four sides without the else clause
*/
hsRect* hsRect::Union(UInt32 count, const hsPoint2 p[])
{
	if (count > 0)
	{	hsScalar	left = fLeft;
		hsScalar	top = fTop;
		hsScalar	right = fRight;
		hsScalar	bottom = fBottom;
		
		hsScalar	value = p->fX;
		if (value < left)		left = value;
		if (value > right)	right = value;

		value = p->fY;
		if (value < top)		top = value;
		if (value > bottom)	bottom = value;
		
		for (++p, --count; count > 0; ++p, --count)
		{	hsScalar	value = p->fX;
			if (value < left)			left = value;
			else if (value > right)	right = value;
			
			value = p->fY;
			if (value < top)			top = value;
			else if (value > bottom)	bottom = value;
		}
		
		(void)this->Set(left, top, right, bottom);
	}
	return this;
}

////////////////////////////////////////////////////////////////////////////////////

hsPoint2* hsRect::Center(hsPoint2* center) const
{
	center->fX	= this->CenterX();
	center->fY	= this->CenterY();
	
	return center;
}

hsIntRect *hsRect::Truncate(hsIntRect* r) const
{
	r->fLeft		= Int32(fLeft);
	r->fTop		= Int32(fTop);
	r->fRight		= Int32(fRight);
	r->fBottom	= Int32(fBottom);
	return r;
}

hsIntRect *hsRect::RoundOut(hsIntRect* r) const
{
	r->fLeft		= hsScalarToInt(hsFloor(fLeft));
	r->fTop		= hsScalarToInt(hsFloor(fTop));
	r->fRight		= hsScalarToInt(hsCeil(fRight));
	r->fBottom	= hsScalarToInt(hsCeil(fBottom));
	return r;
}

hsIntRect* hsRect::Round(hsIntRect* r) const
{
	r->fLeft		= hsScalarRound(fLeft);
	r->fTop		= hsScalarRound(fTop);
	r->fRight		= hsScalarRound(fRight);
	r->fBottom	= hsScalarRound(fBottom);
	return r;
}

////////////////////////////////////////////////////////////////

Boolean operator==(const hsRect& r1, const hsRect& r2)
{
	return r1.fLeft == r2.fLeft && r1.fTop == r2.fTop && r1.fRight == r2.fRight && r1.fBottom == r2.fBottom;
}

Boolean operator!=(const hsRect& r1, const hsRect& r2)
{
	return !(r1.fLeft == r2.fLeft && r1.fTop == r2.fTop && r1.fRight == r2.fRight && r1.fBottom == r2.fBottom);
}

////////////////////////////////////////////////////////////////

// clockwise to == hsFixedRect::ToQuad
hsPoint2* hsRect::ToQuad(hsPoint2 quad[4]) const
{
	quad[0].fX = fLeft;	quad[0].fY = fTop;
	quad[1].fX = fRight;	quad[1].fY = fTop;
	quad[2].fX = fRight;	quad[2].fY = fBottom;
	quad[3].fX = fLeft;	quad[3].fY = fBottom;
	return quad;
}

hsIntRect *hsIntRect::Union(const hsIntRect *r)
{
	if (r->fLeft < fLeft)		fLeft = r->fLeft;
	if (r->fTop < fTop)		fTop = r->fTop;
	if (r->fRight > fRight)	fRight = r->fRight;
	if (r->fBottom > fBottom)	fBottom = r->fBottom;

	return this;
}

Boolean hsIntRect::Intersect(const hsIntRect *r)
{
	if (r->fLeft < fRight && r->fTop < fBottom && fLeft < r->fRight && fTop < r->fBottom)
	{
		if (r->fLeft > fLeft)		fLeft = r->fLeft;
		if (r->fTop > fTop)		fTop = r->fTop;
		if (r->fRight < fRight)	fRight = r->fRight;
		if (r->fBottom < fBottom)	fBottom = r->fBottom;

		return true;
	}
	return false;
}

Boolean hsIntRect::Intersect(Int32 left, Int32 top, Int32 right, Int32 bottom)
{
	if (left < fRight && top < fBottom && fLeft < right && fTop < bottom)
	{
		if (left > fLeft)			fLeft = left;
		if (top > fTop)			fTop = top;
		if (right < fRight)		fRight = right;
		if (bottom < fBottom)	fBottom = bottom;

		return true;
	}
	return false;
}

hsIntRect* hsIntRect::Set(const hsRect* r)
{
	fLeft		= hsScalarToInt(r->fLeft);
	fTop		= hsScalarToInt(r->fTop);
	fRight	= hsScalarToInt(r->fRight);
	fBottom	= hsScalarToInt(r->fBottom);

	return this;
}

hsIntRect* hsIntRect::Inset(Int32 dx, Int32 dy)
{
	fLeft += dx;
	fTop += dy;
	fRight -= dx;
	fBottom -= dy;
	return this;
}

