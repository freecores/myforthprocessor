/*
 * @(#)hsMatrix33.cpp	1.8 03/01/23
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

#include "hsMatrix33.h"
#include "hsMemory.h"
#include "hsFixedTypes.h"

////////////////////////////////////////////////////////////////////////////////////

hsPoint2* hsPoint2::Grid(hsScalar period)
{
	hsScalar	periodAdd = hsScalarDiv2(period);
	hsScalar	invPeriod = hsScalarInvert(period);

	hsScalar newX = hsScalarMul(hsScalarMul(fX + periodAdd, invPeriod), period);
	hsScalar newY = hsScalarMul(hsScalarMul(fY + periodAdd, invPeriod), period);
#if HS_SCALAR_IS_FIXED
	fX = (newX >> 16) << 16;
	fY = (newY >> 16) << 16;
#else
	fX = hsScalar(Int32(newX));
	fY = hsScalar(Int32(newY));
#endif
	return this;
}

#if !(HS_NEVER_USE_FLOAT)
	hsPolar* hsPoint2::ToPolar(hsPolar* polar) const
	{
		polar->fRadius	= hsPoint2::Magnitude(fX, fY);
		polar->fAngle	= hsATan2(fY, fX);
		return polar;
	}
#endif

Boolean hsPoint2::CloseEnough(const hsPoint2* p, hsScalar tolerance) const
{
	return hsABS(p->fX - this->fX) <= tolerance && hsABS(p->fY - this->fY) <= tolerance;
}

hsScalar hsPoint2::Distance(const hsPoint2& p1, const hsPoint2& p2)
{
	return hsPoint2::Magnitude(p2.fX - p1.fX, p2.fY - p1.fY);
}

hsPoint2 hsPoint2::Average(const hsPoint2& a, const hsPoint2& b)
{
	hsPoint2	result;

	return *result.Set((a.fX + b.fX) * hsScalar(0.5), (a.fY + b.fY) * hsScalar(0.5));
}

#if HS_CAN_USE_FLOAT
	hsScalar hsPoint2::ComputeAngle(const hsPoint2& a, const hsPoint2& b, const hsPoint2& c)
	{
		hsPoint2	v1 = a - b;
		hsPoint2	v2 = c - b;
		hsPolar	polar1, polar2;

		return v2.ToPolar(&polar2)->fAngle - v1.ToPolar(&polar1)->fAngle;
	}
#endif

#if HS_SCALAR_IS_FIXED
	hsScalar hsPoint2::Magnitude(hsScalar x, hsScalar y)
	{
		hsWide	w1, w2;

		return w1.Mul(x, x)->Add(w2.Mul(y, y))->Sqrt();
	}
#endif

////////////////////////////////////////////////////////////////////////////////////

hsMatrixType hsMatrix33::GetType() const
{
	hsMatrixType	matType = kIdentityMatrixType;

	if (fMap[0][2] != 0 || fMap[1][2] != 0)
		matType |= kTranslateMatrixType;
	if (fMap[0][0] != hsScalar1 || fMap[1][1] != hsScalar1)
		matType |= kScaleMatrixType;
	if (fMap[0][1] != 0 || fMap[1][0] != 0)
		matType |= kRotateMatrixType;
	if (fMap[2][0] != 0 || fMap[2][1] != 0 || fMap[2][2] != hsScalar1)
		matType |= kPerspectiveMatrixType;
	
	return matType;
}

//////////////////////////////////////////////////////////////////////////////

hsMatrix33* hsMatrix33::Reset()
{
        static const hsMatrix33 gIdentity = {
	    {
		{ hsScalar1, 0,             0         },
		{ 0,         hsScalar1,     0         },
		{ 0,         0,             hsScalar1 },
	    }
	};

	*this = gIdentity;
	return this;
}

hsMatrix33* hsMatrix33::Normalize()
{
	if (fMap[2][2] != hsScalar1)
	{	hsScalar	mul = hsScalar1 / fMap[2][2];
		
		fMap[0][0] *= mul;
		fMap[0][1] *= mul;
		fMap[0][2] *= mul;

		fMap[1][0] *= mul;		
		fMap[1][1] *= mul;
		fMap[1][2] *= mul;
			
		fMap[2][0] *= mul;
		fMap[2][1] *= mul;
		fMap[2][2] = hsScalar1;
	}
	return this;
}

//////////////////////////////////////////////////////////////////////////////

hsMatrix33* hsMatrix33::SetTranslate(hsScalar x, hsScalar y)
{
	(void)this->Reset();
	fMap[0][2] = x;
	fMap[1][2] = y;
	return this;
}

hsMatrix33* hsMatrix33::Translate(hsScalar dx, hsScalar dy)
{
	hsScalar	px = fMap[2][0];
	hsScalar	py = fMap[2][1];
	
	if (px != 0 || py != 0)
	{	fMap[0][0] += px * dx;
		fMap[0][1] += py * dx;
		fMap[1][0] += px * dy;
		fMap[1][1] += py * dy;
	}
	fMap[0][2] += dx;
	fMap[1][2] += dy;

	return this;
}

hsMatrix33* hsMatrix33::SetScale(hsScalar sx, hsScalar sy, hsScalar px, hsScalar py)
{
	fMap[0][0] = sx;
	fMap[0][1] = 0;
	fMap[0][2] = px - px * sx;

	fMap[1][0] = 0;
	fMap[1][1] = sy;
	fMap[1][2] = py - py * sy;

	fMap[2][0] = fMap[2][1] = 0;
	fMap[2][2] = hsScalar1;

	return this;
}

hsMatrix33* hsMatrix33::Scale(hsScalar sx, hsScalar sy, hsScalar px, hsScalar py)
{
	hsMatrix33	map;

	*this = *map.SetScale(sx, sy, px, py) * (*this);
	
	return this;
}

hsMatrix33* hsMatrix33::SetRotate(hsScalar degrees, hsScalar px, hsScalar py)
{
	hsScalar	radians = degrees * HS_PI / hsScalar(180);
	hsScalar	sinValue = hsSine(radians);
	hsScalar	cosValue = hsCosine(radians);

	fMap[0][0] = cosValue;
	fMap[0][1] = -sinValue;
	fMap[0][2] = -cosValue * px + sinValue * py + px;

	fMap[1][0] = sinValue;
	fMap[1][1] = cosValue;
	fMap[1][2] = -sinValue * px - cosValue * py + py;

	fMap[2][0] = fMap[2][1] = 0;
	fMap[2][2] = hsScalar1;

	return this;
}

hsMatrix33* hsMatrix33::Rotate(hsScalar degrees, hsScalar px, hsScalar py)
{
	hsMatrix33	map;

	*this = *map.SetRotate(degrees, px, py) * (*this);

	return this;
}

////////////////////////////////////////////////////////////////////////////////////

inline void Swap(hsScalar& a, hsScalar& b)
{
	hsScalar	c = a;
	a = b;
	b = c;
}

hsRect* hsMatrix33::MapRect(const hsRect* src, hsRect* dst, hsMatrixType matType) const
{
	if (matType == kUnknownMatrixType)
		matType = this->GetType();

	if (matType & (kRotateMatrixType | kPerspectiveMatrixType))
	{	hsPoint2	quad[4];

		dst->Set(4, this->MapPoints(4, src->ToQuad(quad), quad, matType));
	}
	else
	{	hsScalar		dx = this->TranslateX();
		hsScalar		dy = this->TranslateY();
		hsScalar		sx = fMap[0][0];
		hsScalar		sy = fMap[1][1];

		hsScalar min = sx * src->fLeft + dx;
		hsScalar max = sx * src->fRight + dx;
		if (min > max)
			Swap(min, max);
		dst->fLeft	= min;
		dst->fRight = max;

		min = sy * src->fTop + dy;
		max = sy * src->fBottom + dy;
		if (min > max)
			Swap(min, max);
		dst->fTop = min;
		dst->fBottom = max;
	}
	return dst;
}

hsRect* hsMatrix33::MapRect(hsRect* rect, hsMatrixType matType) const
{
	return this->MapRect(rect, rect, matType);
}

hsPoint2* hsMatrix33::MapPoints(UInt32 count, const hsPoint2 src[], hsPoint2 dst[], hsMatrixType matType) const
{
	hsPoint2*	origDst = dst;

	hsAssert(matType == kUnknownMatrixType || matType == this->GetType(), "bad matType");

	if (matType == kUnknownMatrixType)
		matType = this->GetType();

	if (matType == kIdentityMatrixType)
	{	if (src != dst)
			HSMemory::BlockMove(src, dst, count * sizeof(hsPoint2));
	}
	else
	{	hsScalar	translateX = fMap[0][2];
		hsScalar	translateY = fMap[1][2];

		if (matType & kPerspectiveMatrixType)
		{	hsScalar	m22 = fMap[2][2];
	
			for (; count--; ++src, ++dst)
			{	
				hsScalar	newX = fMap[0][0] * src->fX + fMap[0][1] * src->fY + translateX;
				hsScalar	newY = fMap[1][0] * src->fX + fMap[1][1] * src->fY + translateY;

				hsScalar	perspect = src->fX * fMap[2][0] + src->fY * fMap[2][1] + m22;

				if (perspect != 0)
				{	dst->fX = newX / perspect;
					dst->fY = newY / perspect;
				}
			}
		}
		else if (matType & kRotateMatrixType)
			for (; count--; ++src, ++dst)
			{	hsScalar	newX = fMap[0][0] * src->fX + fMap[0][1] * src->fY + translateX;
				hsScalar	newY = fMap[1][0] * src->fX + fMap[1][1] * src->fY + translateY;
				dst->fX = newX;
				dst->fY = newY;
			}
		else if (matType & kScaleMatrixType)
			for (; count--; ++src, ++dst)
			{	dst->fX = fMap[0][0] * src->fX + translateX;
				dst->fY = fMap[1][1] * src->fY + translateY;
			}
		else	// kTranslateMatrixType
			for (; count--; ++src, ++dst)
			{	dst->fX = src->fX + translateX;
				dst->fY = src->fY + translateY;
			}
	}
	return origDst;
}

hsPoint2* hsMatrix33::MapPoints(UInt32 count, hsPoint2 points[], hsMatrixType matType) const
{
	return this->MapPoints(count, points, points, matType);
}

hsPoint2* hsMatrix33::MapVectors(UInt32 count, const hsPoint2 src[], hsPoint2 dst[], hsMatrixType matType) const
{
	if (matType == kUnknownMatrixType)
		matType = this->GetType();

	if ((matType & kPerspectiveMatrixType) || (fMap[0][2] == 0 && fMap[1][2] == 0))
		return this->MapPoints(count, src, dst, matType);

	hsMatrix33	tmp = *this;

	tmp.fMap[0][2] = tmp.fMap[1][2] = 0;
	
	return tmp.MapPoints(count, src, dst, matType & ~kTranslateMatrixType);
}

hsPoint2* hsMatrix33::MapVectors(UInt32 count, hsPoint2 points[], hsMatrixType matType) const
{
	return this->MapVectors(count, points, points, matType);
}

////////////////////////////////////////////////////////////////////////////////////

static hsScalar Determinant(const hsMatrix33* m)
{
	return	m->fMap[0][0] * m->fMap[1][1] * m->fMap[2][2] +
			m->fMap[0][1] * m->fMap[1][2] * m->fMap[2][0] +
			m->fMap[0][2] * m->fMap[1][0] * m->fMap[2][1] -
			m->fMap[0][2] * m->fMap[1][1] * m->fMap[2][0] -
			m->fMap[0][0] * m->fMap[1][2] * m->fMap[2][1] -
			m->fMap[0][1] * m->fMap[1][0] * m->fMap[2][2];
}

hsMatrix33* hsMatrix33::Invert(hsMatrix33* inverse) const
{
	hsScalar	det_over_1 = Determinant(this);

	if (det_over_1 != 0)
	{	det_over_1 = hsScalar1 / det_over_1;
	
		inverse->fMap[0][0] = (fMap[1][1] * fMap[2][2] - fMap[1][2] * fMap[2][1]) * det_over_1;
		inverse->fMap[1][0] = (fMap[1][2] * fMap[2][0] - fMap[1][0] * fMap[2][2]) * det_over_1;
		inverse->fMap[2][0] = (fMap[1][0] * fMap[2][1] - fMap[1][1] * fMap[2][0]) * det_over_1;
			
		inverse->fMap[0][1] = (fMap[0][2] * fMap[2][1] - fMap[0][1] * fMap[2][2]) * det_over_1;
		inverse->fMap[1][1] = (fMap[0][0] * fMap[2][2] - fMap[0][2] * fMap[2][0]) * det_over_1;
		inverse->fMap[2][1] = (fMap[0][1] * fMap[2][0] - fMap[0][0] * fMap[2][1]) * det_over_1;
		
		inverse->fMap[0][2] = (fMap[0][1] * fMap[1][2] - fMap[0][2] * fMap[1][1]) * det_over_1;
		inverse->fMap[1][2] = (fMap[0][2] * fMap[1][0] - fMap[0][0] * fMap[1][2]) * det_over_1;
		inverse->fMap[2][2] = (fMap[0][0] * fMap[1][1] - fMap[0][1] * fMap[1][0]) * det_over_1;

		return inverse;
	}
	return nil;
}

////////////////////////////////////////////////////////////////////////////////////

/*
	c = a * b
*/
hsMatrix33* hsMatrix33::SetConcat(const hsMatrix33* a, const hsMatrix33* b)
{
	hsMatrix33	tmpMatrix;
	hsMatrixType	aType, bType;
	hsMatrix33*	c;

	if ((aType = a->GetType()) == kIdentityMatrixType)
	{	c = (hsMatrix33*)b;
		goto DONE;
	}
	if ((bType = b->GetType()) == kIdentityMatrixType)
	{	c = (hsMatrix33*)a;
		goto DONE;
	}

	//	If this was passed as one of the params, make c point to a temp matrix and then copy it into this
	c = this;
	if (this == a || this == b)
		c = &tmpMatrix;

	c->fMap[0][0] = a->fMap[0][0] * b->fMap[0][0] + a->fMap[0][1] * b->fMap[1][0] + a->fMap[0][2] * b->fMap[2][0];
	c->fMap[0][1] = a->fMap[0][0] * b->fMap[0][1] + a->fMap[0][1] * b->fMap[1][1] + a->fMap[0][2] * b->fMap[2][1];
	c->fMap[0][2] = a->fMap[0][0] * b->fMap[0][2] + a->fMap[0][1] * b->fMap[1][2] + a->fMap[0][2] * b->fMap[2][2];

	c->fMap[1][0] = a->fMap[1][0] * b->fMap[0][0] + a->fMap[1][1] * b->fMap[1][0] + a->fMap[1][2] * b->fMap[2][0];
	c->fMap[1][1] = a->fMap[1][0] * b->fMap[0][1] + a->fMap[1][1] * b->fMap[1][1] + a->fMap[1][2] * b->fMap[2][1];
	c->fMap[1][2] = a->fMap[1][0] * b->fMap[0][2] + a->fMap[1][1] * b->fMap[1][2] + a->fMap[1][2] * b->fMap[2][2];

	if ((aType | bType) & kPerspectiveMatrixType)
	{	
		c->fMap[2][0] = a->fMap[2][0] * b->fMap[0][0] + a->fMap[2][1] * b->fMap[1][0] + a->fMap[2][2] * b->fMap[2][0];
		c->fMap[2][1] = a->fMap[2][0] * b->fMap[0][1] + a->fMap[2][1] * b->fMap[1][1] + a->fMap[2][2] * b->fMap[2][1];
		c->fMap[2][2] = a->fMap[2][0] * b->fMap[0][2] + a->fMap[2][1] * b->fMap[1][2] + a->fMap[2][2] * b->fMap[2][2];
	}
	else
	{	c->fMap[2][0] = 0;
		c->fMap[2][1] = 0;
		c->fMap[2][2] = hsScalar1;
	}

DONE:
	if (this != c)
		*this = *c;
	return this;
}

hsMatrix33 operator*(const hsMatrix33& a, const hsMatrix33& b)
{
	hsMatrix33	c;

	(void)c.SetConcat(&a, &b);
	
	return c;
}

////////////////////////////////////////////////////////////////////////////////////

hsFixedMatrix33* hsMatrix33::ToFixed33(hsFixedMatrix33* matrix) const
{
	hsMatrix33		normalMat;
	const hsMatrix33*	src = this;

	if (this->fMap[2][2] != hsScalar1)
	{	normalMat = *this;
		src = normalMat.Normalize();
	}

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++)
		{	hsAssert(src->fMap[i][j] >= -32768.0 && src->fMap[i][j] < 32768.0, "mat out of range");
			matrix->fMap[i][j] = hsScalarToFixed(src->fMap[i][j]);
		}
	for (int k = 0; k < 3; k++)
		matrix->fMap[2][k] = hsScalarToFract(src->fMap[2][k]);

	return matrix;
}

////////////////////////////////////////////////////////////////////////////////////

static void QuadToPoint(const hsPoint2 quad[], hsPoint2* pt)
{
	hsScalar	dx = quad[1].fX - quad[0].fX;
	hsScalar	dy = quad[1].fY - quad[0].fY;

	pt->fY = hsPoint2::Magnitude(dx, dy);

#if 1
	pt->fX = (dx * (quad[0].fY - quad[3].fY) + dy * (quad[3].fX - quad[0].fX)) / pt->fY;
#else
	hsWide	wide1, wide2;

	wide1.Mul(dx, quad[0].fY - quad[3].fY);
	wide2.Mul(dy, quad[3].fX - quad[0].fX);
	wide1.Add(&wide2);
	wide1.Div(pt->fY);

	pt->fX = wide1.AsLong();
#endif
}

static void Map4Pt(const hsPoint2 source[], hsMatrix33* dest, hsScalar scaleX, hsScalar scaleY)
{
	hsScalar	a1, a2;
	hsScalar	x0, y0, x1, y1, x2, y2;

	x0 = source[2].fX - source[0].fX;
	y0 = source[2].fY - source[0].fY;
	x1 = source[2].fX - source[1].fX;
	y1 = source[2].fY - source[1].fY;
	x2 = source[2].fX - source[3].fX;
	y2 = source[2].fY - source[3].fY;

	/* check if abs(x2) > abs(y2) */
	if ( x2 > 0 ? y2 > 0 ? x2 > y2 : x2 > -y2 : y2 > 0 ? -x2 > y2 : x2 < y2)
	//	a1 = hsFracDiv(hsMulDiv(x0 - x1, y2, x2) - y0 + y1, hsMulDiv(x1, y2, x2) - y1);
		a1 = ((x0 - x1) * y2 / x2 - y0 + y1) / (x1 * y2 / x2 - y1);
	else
	//	a1 = hsFracDiv(x0 - x1 - hsMulDiv(y0 - y1, x2, y2), x1 - hsMulDiv(y1, x2, y2));
		a1 = (x0 - x1 - (y0 - y1) * x2 / y2) / (x1 - y1 * x2 / y2);

	/* check if abs(x1) > abs(y1) */
	if ( x1 > 0 ? y1 > 0 ? x1 > y1 : x1 > -y1 : y1 > 0 ? -x1 > y1 : x1 < y1)
	//	a2 = hsFracDiv(y0 - y2 - hsMulDiv(x0 - x2, y1, x1), y2 - hsMulDiv(x2, y1, x1));
		a2 = (y0 - y2 - (x0 - x2) * y1 / x1) / (y2 - x2 * y1 / x1);
	else
	//	a2 = hsFracDiv(hsMulDiv(y0 - y2, x1, y1) - x0 + x2, hsMulDiv(y2, x1, y1) - x2);
		a2 = ((y0 - y2) * x1 / y1 - x0 + x2) / (y2 * x1 / y1 - x2);

	dest->fMap[0][0] = (a2 * source[3].fX + source[3].fX - source[0].fX) / scaleX;
	dest->fMap[1][0] = (a2 * source[3].fY + source[3].fY - source[0].fY) / scaleX;
	dest->fMap[2][0] = a2 / scaleX;
	dest->fMap[0][1] = (a1 * source[1].fX + source[1].fX - source[0].fX) / scaleY;
	dest->fMap[1][1] = (a1 * source[1].fY + source[1].fY - source[0].fY) / scaleY;
	dest->fMap[2][1] = a1 / scaleY;
	dest->fMap[0][2] = source[0].fX;
	dest->fMap[1][2] = source[0].fY;
	dest->fMap[2][2] = hsScalar1;
}

hsMatrix33* hsMatrix33::SetQuadToQuad(const hsPoint2 srcQuad[], const hsPoint2 dstQuad[])
{
	hsPoint2		tempPt;
	hsMatrix33	tempMap;

	QuadToPoint(srcQuad, &tempPt);

	Map4Pt(srcQuad, &tempMap, tempPt.fX, tempPt.fY);
	if (tempMap.Invert(this) == nil)
		return nil;

	Map4Pt(dstQuad, &tempMap, tempPt.fX, tempPt.fY);
	*this = *this * tempMap;
	return this;
}

