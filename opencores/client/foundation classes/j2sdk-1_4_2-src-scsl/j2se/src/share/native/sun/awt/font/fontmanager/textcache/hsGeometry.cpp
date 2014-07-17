/*
 * @(#)hsGeometry.cpp	1.9 03/01/23
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
#include "hsMemory.h"

///////////////////////////////////////////////////////////////////////////

void hsPolygon::Transform(const hsMatrix33* matrix)
{
	hsMatrixType	mType = matrix->GetType();

	for (UInt32 i = 0; i < fContourCount; i++)
		matrix->MapPoints(fContours[i].fPointCount, fContours[i].fPoints, mType);
}

hsRect* hsPolygon::ComputeBounds(hsRect* bounds) const
{
	Boolean	firstTime = true;

	for (UInt32 i = 0; i < fContourCount; i++)
	{	if (fContours[i].fPointCount > 0)
		{	if (firstTime)
			{	bounds->Set(	fContours[i].fPoints[0].fX, fContours[i].fPoints[0].fY,
							fContours[i].fPoints[0].fX, fContours[i].fPoints[0].fY);
				firstTime = false;
			}
			bounds->Union(fContours[i].fPointCount, fContours[i].fPoints);
		}
	}
	return bounds;
}

hsPolygon* hsPolygon::Copy(hsPolygon* dstOrNil) const
{
	hsPolygon* dst = dstOrNil;
	if (dst == nil)
		dst = new hsPolygon;

	dst->fContourCount	= 0;
	dst->fContours		= nil;
	if (this->fContourCount > 0)
	{
#ifndef CC_NOEX
		try {
#endif // !CC_NOEX
			dst->fContours = new hsPolyContour[this->fContourCount];
			for (UInt32 i = 0; i < this->fContourCount; i++)
			{	dst->fContours[i].fPointCount	= this->fContours[i].fPointCount;
				dst->fContours[i].fPoints		= nil;
				if (dst->fContours[i].fPointCount > 0)
				{	dst->fContours[i].fPoints = new hsPoint2[dst->fContours[i].fPointCount];
					HSMemory::BlockMove(this->fContours[i].fPoints, dst->fContours[i].fPoints,
										dst->fContours[i].fPointCount * sizeof(hsPoint2));
				}
				dst->fContourCount += 1;
			}
#ifndef CC_NOEX
		}
		catch (...) {
			hsPolygon::Delete(dst);
			if (dstOrNil == nil)
				delete dst;
			throw;
		}
#endif // !CC_NOEX
	}
	return dst;
}

void hsPolygon::Delete(hsPolygon* poly)
{
	if (poly->fContourCount > 0)
	{	for (UInt32 i = 0; i < poly->fContourCount; i++)
			delete[] poly->fContours[i].fPoints;
		delete[] poly->fContours;
	}
	hsPolygon::Init(poly);
}

void hsPolygon::Read(hsStream* stream)
{
	hsAssert(fContourCount == 0, "overwriting polygon");

	fContourCount	= stream->ReadSwap32();
	fContours		= nil;

	if (fContourCount)
	{	fContours = new hsPolyContour[fContourCount];
		for (UInt32 i = 0; i < fContourCount; i++)
		{	fContours[i].fPointCount	= stream->ReadSwap32();
			fContours[i].fPoints		= nil;
			if (fContours[i].fPointCount)
			{	fContours[i].fPoints = new hsPoint2[fContours[i].fPointCount];
				stream->ReadSwapFloat(fContours[i].fPointCount * 2, &fContours[i].fPoints[0].fX);
			}
		}
	}
}

void hsPolygon::Write(hsStream* stream) const
{
	stream->WriteSwap32(fContourCount);
	for (UInt32 i = 0; i < fContourCount; i++)
	{	stream->WriteSwap32(fContours[i].fPointCount);
		stream->WriteSwapFloat(fContours[i].fPointCount * 2, &fContours[i].fPoints[0].fX);
	}
}

///////////////////////////////////////////////////////////////////////////

/*
	Compute the t-value where the curve has a yMax (or yMin). If the value is outside of 0..1 return false
*/
Boolean hsDivideQuadAtMax(hsScalar a, hsScalar b, hsScalar c, hsScalar* t)
{
	hsScalar numer = a - b;
	hsScalar denom = a - (b + b) + c;

	if (numer < 0)
	{	numer = -numer;
		denom = -denom;
	}
	if (numer == 0 || denom <= 0 || numer >= denom)	// we only want 0 < t < 1
		return false;
	
	*t = numer / denom;
	return true;
}

inline void UnionRange(hsScalar& min, hsScalar& max, hsScalar value)
{
	if (value < min)
		min = value;
	else if (value > max)
		max = value;
}

inline void UnionExactQuadContour(const hsQuadContour* contour, hsRect* bounds)
{
	CurveWalker	walker;

	walker.Init(contour, true);

	hsScalar	left		= bounds->fLeft;
	hsScalar	top		= bounds->fTop;
	hsScalar	right		= bounds->fRight;
	hsScalar	bottom	= bounds->fBottom;

	while (walker.NextQuad())
	{	UnionRange(left, right, walker.a.fX);
		UnionRange(top, bottom, walker.a.fY);
		if (walker.drawLine == false)
		{	hsScalar	t;
			if (hsDivideQuadAtMax(walker.a.fX, walker.b.fX, walker.c.fX, &t))
				UnionRange(left, right, ScalarAverage(walker.a.fX, walker.b.fX, t));
			if (hsDivideQuadAtMax(walker.a.fY, walker.b.fY, walker.c.fY, &t))
				UnionRange(top, bottom, ScalarAverage(walker.a.fY, walker.b.fY, t));
		}
	}
	bounds->Set(left, top, right, bottom);
}

hsRect* hsQuadraticSpline::ComputeBounds(hsRect* bounds, Boolean exactBounds) const
{
	Boolean				firstTime = true;
	const hsQuadContour*	contour = fContours;

	/*
	bounds->Set(	hsScalar(kPosInfinity32),
				hsScalar(kPosInfinity32),
				hsScalar(kNegInfinity32),
				hsScalar(kNegInfinity32));
	*/

	for (UInt32 i = 0; i < fContourCount; i++)
	{	if (contour->fPointCount > 0)
		{	if (firstTime)
			{	bounds->Set(	contour->fPoints[0].fX, contour->fPoints[0].fY,
							contour->fPoints[0].fX, contour->fPoints[0].fY);
				firstTime = false;
			}

			if (exactBounds)
				UnionExactQuadContour(contour, bounds);
			else
				bounds->Union(contour->fPointCount, contour->fPoints);
		}
		++contour;
	}
	
	return bounds;
}

void hsPathSpline::Transform(const hsMatrix33* matrix)
{
	hsMatrixType	mType = matrix->GetType();

	for (UInt32 i = 0; i < fContourCount; i++)
		matrix->MapPoints(fContours[i].fPointCount, fContours[i].fPoints, mType);
}

hsPathSpline* hsPathSpline::Copy(hsPathSpline* dstOrNil) const
{
	hsPathSpline* dst = dstOrNil;
	if (dst == nil)
		dst = new hsPathSpline;

	dst->fContourCount	= 0;
	dst->fContours		= nil;
	if (this->fContourCount > 0)
	{
#ifndef CC_NOEX
		try {
#endif // !CC_NOEX
			dst->fContours = new hsPathContour[this->fContourCount];
			for (UInt32 i = 0; i < this->fContourCount; i++)
			{	UInt32	pointCount			= this->fContours[i].fPointCount;
				dst->fContours[i].fPointCount	= pointCount;
				dst->fContours[i].fPoints		= nil;
				dst->fContours[i].fControlBits	= nil;
				if (pointCount > 0)
				{	dst->fContours[i].fPoints = new hsPoint2[pointCount];
					HSMemory::BlockMove(this->fContours[i].fPoints, dst->fContours[i].fPoints,
										pointCount * sizeof(hsPoint2));

					if (this->fContours[i].fControlBits != nil)
					{	dst->fContours[i].fControlBits = new UInt32[hsPathContour::ControlBitLongs(pointCount)];
						HSMemory::BlockMove(this->fContours[i].fControlBits, dst->fContours[i].fControlBits,
											hsPathContour::ControlBitLongs(pointCount) * sizeof(UInt32));
					}
				}
				dst->fContourCount += 1;
			}
#ifndef CC_NOEX
		}
		catch (...) {
			hsPathSpline::Delete(dst);
			if (dstOrNil == nil)
				delete dst;
			throw;
		}
#endif // !CC_NOEX
	}
	return dst;
}

hsPathSpline* hsPathSpline::SoftCopy(hsPathSpline* dst) const
{
	if (dst)
	{	dst->fContourCount = this->fContourCount;
		if (dst->fContours)
		{	for (unsigned i = 0; i < this->fContourCount; i++)
			{	dst->fContours[i].fPointCount = this->fContours[i].fPointCount;
				if (dst->fContours[i].fPoints)
					HSMemory::BlockMove(this->fContours[i].fPoints, dst->fContours[i].fPoints,
										this->fContours[i].fPointCount * sizeof(hsPoint2));
				if (dst->fContours[i].fControlBits)
					HSMemory::BlockMove(this->fContours[i].fControlBits, dst->fContours[i].fControlBits,
										hsPathContour::ControlBitBytes(this->fContours[i].fPointCount));
			}
		}
	}
	return dst;
}

void hsPathSpline::Delete(hsPathSpline* path)
{
	if (path->fContourCount > 0)
	{	for (UInt32 i = 0; i < path->fContourCount; i++)
		{	delete[] path->fContours[i].fPoints;
			delete[] path->fContours[i].fControlBits;
		}
		delete[] path->fContours;
	}
	hsPathSpline::Init(path);
}

void hsPathSpline::Read(hsStream* stream)
{
	hsAssert(fContourCount == 0, "overwriting polygon");

	fContourCount	= stream->ReadSwap32();
	fContours		= nil;

	if (fContourCount)
	{	fContours = new hsPathContour[fContourCount];
		for (UInt32 i = 0; i < fContourCount; i++)
		{	fContours[i].fPointCount	= stream->ReadSwap32();
			fContours[i].fPoints		= nil;
			fContours[i].fControlBits	= nil;

			if (fContours[i].fPointCount)
			{	fContours[i].fPoints = new hsPoint2[fContours[i].fPointCount];
				stream->ReadSwapFloat(fContours[i].fPointCount * 2, &fContours[i].fPoints[0].fX);

				UInt32	controlBitLongs = hsPathContour::ControlBitLongs(fContours[i].fPointCount);
				if (controlBitLongs)
				{	hsAssert(controlBitLongs == hsPathContour::ControlBitLongs(fContours[i].fPointCount), "bad ctrlLong count");
					fContours[i].fControlBits = new UInt32[controlBitLongs];
					stream->ReadSwap32(controlBitLongs, fContours[i].fControlBits);
				}
			}
		}
	}
}

void hsPathSpline::Write(hsStream* stream) const
{
	stream->WriteSwap32(fContourCount);
	for (UInt32 i = 0; i < fContourCount; i++)
	{	stream->WriteSwap32(fContours[i].fPointCount);
		if (fContours[i].fPointCount)
		{	stream->WriteSwapFloat(fContours[i].fPointCount * 2, &fContours[i].fPoints[0].fX);

			UInt32	controlBitLongs = 0;
			if (fContours[i].fControlBits)
				controlBitLongs = hsPathContour::ControlBitLongs(fContours[i].fPointCount);
			stream->WriteSwap32(controlBitLongs, fContours[i].fControlBits);
		}
	}
}

///////////////////////////////////////////////////////////////////////////

/*
	fControlBits[]	at least 1 UInt32
	fPoints[]		at least 8 hsPoint2
*/
void hsPathContour::SetQuadOval(const hsRect* r)
{
	const hsScalar	scale = tanf(HS_PI/8) * hsScalar(0.5);

	hsScalar	cx = r->CenterX();
	hsScalar	cy = r->CenterY();
	hsScalar	h = r->Width() * scale;
	hsScalar	v = r->Height() * scale;

	this->fPointCount = kQuadOvalPointCount;
	this->fControlBits[0] = 0xFF000000;
	hsScalar*	p = &this->fPoints[0].fX;

	*p++ = r->fLeft;	*p++ = cy - v;
	*p++ = cx - h;		*p++ = r->fTop;
	*p++ = cx + h;		*p++ = r->fTop;
	*p++ = r->fRight;	*p++ = cy - v;

	*p++ = r->fRight;	*p++ = cy + v;
	*p++ = cx + h;		*p++ = r->fBottom;
	*p++ = cx - h;		*p++ = r->fBottom;
	*p++ = r->fLeft;	*p = cy + v;
}

/*
	fControlBits[]	at least 1 UInt32
	fPoints[]		at least 12 hsPoint2
*/
void hsPathContour::SetCubicOval(const hsRect* r)
{
	const hsScalar	scale = tanf(HS_PI/8) * hsScalar(2) / hsScalar(3);

	hsScalar	midWidth = r->CenterX();
	hsScalar	midHeight = r->CenterY();
	hsScalar	h = r->Width() * scale;
	hsScalar	v = r->Height() * scale;

	this->fPointCount = kCubicOvalPointCount;
	this->fControlBits[0] = 0x6DB00000;	// 0110110110110
	hsScalar* p = &this->fPoints[0].fX;

	*p++ = r->fLeft;		*p++ = midHeight;
	*p++ = r->fLeft;		*p++ = midHeight - v;
	*p++ = midWidth - h;	*p++ = r->fTop;

	*p++ = midWidth;		*p++ = r->fTop;
	*p++ = midWidth + h;	*p++ = r->fTop;
	*p++ = r->fRight;		*p++ = midHeight - v;

	*p++ = r->fRight;		*p++ = midHeight;
	*p++ = r->fRight;		*p++ = midHeight + v;
	*p++ = midWidth + h;	*p++ = r->fBottom;

	*p++ = midWidth;		*p++ = r->fBottom;
	*p++ = midWidth - h;	*p++ = r->fBottom;
	*p++ = r->fLeft;		*p = midHeight + v;
}

//////////////////////////////////////////////////////////////////////

void CurveWalker::Init(const hsPathContour* contour, Boolean aClosed)
{
	hsAssert(contour->fPointCount > 1, "bad point count for walker");

	this->count		= contour->fPointCount;
	this->points		= contour->fPoints;
	this->controlBits	= contour->fControlBits;
	this->closed		= aClosed;

	this->index_0		= 0;
	this->index_1		= 1;
	this->timeToStop	= false;
}

inline void	CurveWalker::NextIndex()
{
	this->index_0 = this->index_1;
	if (++this->index_1 == this->count)
	{	this->index_1 = 0;
		if (this->closed == false)
			this->timeToStop = true;
	}
	if (this->index_0 == 0)
		this->timeToStop = true;
}

inline int CurveWalker::OnCurve(int index) const
{
	return this->controlBits == nil || (this->controlBits[index >> 5] & (1L << 31 - (index & 31))) == 0;
}

inline void CurveWalker::SetAverage(hsPoint2* ave)
{
	const hsPoint2* aa = &this->points[this->index_0];
	const hsPoint2* bb = &this->points[this->index_1];
	
	ave->Set(ScalarAverage(aa->fX, bb->fX), ScalarAverage(aa->fY, bb->fY));
}

/*
	If NextPath returns true, then the fields [drawLine, a, b, c] have the answers.
		If drawLine == true, then [a b] define a line segment
		if drawLine == false, then [a b c] define a curve segment
*/
Boolean CurveWalker::NextQuad()
{
	if (this->timeToStop)
		return false;

	if (this->controlBits == nil)
	{	this->a = this->points[this->index_0];
		this->NextIndex();
		this->b = this->points[this->index_0];
		this->drawLine = true;
		return true;
	}

	if (this->OnCurve(this->index_0))
		this->a = this->points[this->index_0];
	else
	{	if (this->OnCurve(this->index_1))
		{	this->a = this->points[this->index_1];
			this->NextIndex();
		}
		else
			this->SetAverage(&this->a);
	}
	this->NextIndex();

	if (this->OnCurve(this->index_0))
	{	this->b = this->points[this->index_0];
		this->drawLine = true;
		return true;
	}
	this->b = this->points[this->index_0];

	if (this->OnCurve(this->index_1))
	{	this->c = this->points[this->index_1];
		this->NextIndex();
	}
	else
	{	this->SetAverage(&this->c);
	}

	this->drawLine = false;
	return true;
}

Boolean CurveWalker::NextCubic()
{
	if (this->timeToStop)
		return false;

	while (this->OnCurve(this->index_0) == false)
		this->NextIndex();
	this->a = this->points[this->index_0];

	this->NextIndex();

	this->b = this->points[this->index_0];
	if ((this->drawLine = this->OnCurve(this->index_0)) == false)
	{	this->NextIndex();
		this->c = this->points[this->index_0];
		this->NextIndex();
		this->d = this->points[this->index_0];
	}
	return true;
}

