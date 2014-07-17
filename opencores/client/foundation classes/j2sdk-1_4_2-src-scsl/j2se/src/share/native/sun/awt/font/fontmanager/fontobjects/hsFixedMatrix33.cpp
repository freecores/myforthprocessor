/*
 * @(#)hsFixedMatrix33.cpp	1.7 03/01/23
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

#include "hsFixedTypes.h"
#include "hsWide.h"
#include "hsMemory.h"

#ifdef _MSC_VER
#pragma optimize("", off)
#endif /* _MSC_VER */
////////////////////////////////////////////////////////////////////////////////////

hsMatrixType hsFixedMatrix33::GetType() const
{
	hsMatrixType	matType = kIdentityMatrixType;

	if (fMap[0][2] != 0 || fMap[1][2] != 0)
		matType |= kTranslateMatrixType;
	if (fMap[0][0] != hsFixed1 || fMap[1][1] != hsFixed1)
		matType |= kScaleMatrixType;
	if (fMap[0][1] != 0 || fMap[1][0] != 0)
		matType |= kRotateMatrixType;
	if (fMap[2][0] != 0 || fMap[2][1] != 0 || fMap[2][2] != hsFract1)
		matType |= kPerspectiveMatrixType;
	
	return matType;
}

//////////////////////////////////////////////////////////////////////////////

hsFixedMatrix33* hsFixedMatrix33::Reset()
{
	static const hsFixedMatrix33 gIdentity = { hsIntToFixed(1), 0, 0, 0, hsIntToFixed(1), 0, 0, 0, 0x40000000 };

	*this = gIdentity;
	return this;
}

hsFixedMatrix33* hsFixedMatrix33::SetTranslate(hsFixed dx, hsFixed dy)
{
	(void)this->Reset();
	fMap[0][2] = dx;
	fMap[1][2] = dy;
	return this;
}

hsFixedMatrix33* hsFixedMatrix33::Translate(hsFixed dx, hsFixed dy)
{
	hsFract	px = fMap[2][0];
	hsFract	py = fMap[2][1];
	
	if (px != 0 || py != 0)
	{	fMap[0][0] += hsFracMul(px, dx);
		fMap[0][1] += hsFracMul(py, dx);
		fMap[1][0] += hsFracMul(px, dy);
		fMap[1][1] += hsFracMul(py, dy);
	}
	fMap[0][2] += dx;
	fMap[1][2] += dy;

	return this;
}

hsFixedMatrix33* hsFixedMatrix33::SetScale(hsFixed sx, hsFixed sy, hsFixed px, hsFixed py)
{
	fMap[0][0] = sx;
	fMap[0][1] = 0;
	fMap[0][2] = px - hsFixMul(px, sx);

	fMap[1][0] = 0;
	fMap[1][1] = sy;
	fMap[1][2] = py - hsFixMul(py, sy);

	fMap[2][0] = fMap[2][1] = 0;
	fMap[2][2] = hsFract1;

	return this;
}

hsFixedMatrix33* hsFixedMatrix33::Scale(hsFixed sx, hsFixed sy, hsFixed px, hsFixed py)
{
	hsFixedMatrix33	map;

	*this = *map.SetScale(sx, sy, px, py) * (*this);
	
	return this;
}

hsFixedMatrix33* hsFixedMatrix33::SetRotate(hsFixed degrees, hsFixed px, hsFixed py)
{
	hsFixed	radians = hsFixMul(degrees, hsScalarToFixed(hsScalarPI)) / 180;
	hsFixed	sinValue = hsFixedSin(radians);
	hsFixed	cosValue = hsFixedCos(radians);

	fMap[0][0] = cosValue;
	fMap[0][1] = -sinValue;
	fMap[0][2] = hsFixMul(-cosValue, px) + hsFixMul(sinValue, py) + px;

	fMap[1][0] = sinValue;
	fMap[1][1] = cosValue;
	fMap[1][2] = hsFixMul(-sinValue, px) - hsFixMul(cosValue, py) + py;

	fMap[2][0] = fMap[2][1] = 0;
	fMap[2][2] = hsFract1;

	return this;
}

hsFixedMatrix33* hsFixedMatrix33::Rotate(hsFixed degrees, hsFixed px, hsFixed py)
{
	hsFixedMatrix33	map;

	*this = *map.SetRotate(degrees, px, py) * (*this);

	return this;
}

////////////////////////////////////////////////////////////////////////////////////

hsFixedPoint2* hsFixedMatrix33::MapPoints(long count, const hsFixedPoint2 src[], hsFixedPoint2 dst[], hsMatrixType matType) const
{
	hsFixedPoint2*	origDst = dst;

	hsAssert(matType == kUnknownMatrixType || matType == this->GetType(), "bad matType");

	if (matType == kUnknownMatrixType)
		matType = this->GetType();

	if (matType == kIdentityMatrixType)
	{	if (src != dst)
			HSMemory::BlockMove(src, dst, count * sizeof(hsFixedPoint2));
	}
	else
	{	hsFixed	translateX = fMap[0][2];
		hsFixed	translateY = fMap[1][2];

		if (matType & kPerspectiveMatrixType)
		{	hsFixed	m22 = hsFractToFixed(fMap[2][2]);
	
			for (; count--; ++src, ++dst)
			{	
#if 1
				hsFixed newX = hsFixMul(fMap[0][0], src->fX) + hsFixMul(fMap[0][1], src->fY) + translateX;
				hsFixed newY = hsFixMul(fMap[1][0], src->fX) + hsFixMul(fMap[1][1], src->fY) + translateY;

				hsFixed perspect = hsFracMul(src->fX, fMap[2][0]) + hsFracMul(src->fY, fMap[2][1]) + m22;

				if (perspect != 0)
				{	dst->fX = hsFixDiv(newX, perspect);
					dst->fY = hsFixDiv(newY, perspect);
				}
#else
				double newX = (double)hsFixedToScalar(fMap[0][0])* hsFixedToScalar(src->fX)+ hsFixedToScalar(fMap[0][1])*  hsFixedToScalar(src->fY) 
					+ hsFixedToScalar(translateX);
				double newY = (double)hsFixedToScalar(fMap[1][0])* hsFixedToScalar(src->fX) + hsFixedToScalar(fMap[1][1])*hsFixedToScalar(src->fY) 	
						+ hsFixedToScalar(translateY);

				double perspect = (double)hsFixedToScalar(src->fX)* hsFractToScalar(fMap[2][0]) +
					 hsFixedToScalar(src->fY) * hsFractToScalar(fMap[2][1]) + hsFixedToScalar(m22);

				if (perspect != 0)
				{	dst->fX = hsScalarToFixed(newX/perspect);
					dst->fY = hsScalarToFixed(newY/ perspect);
				}
#endif

			}
		}
		else if (matType & kRotateMatrixType)
			for (; count--; ++src, ++dst)
			{	hsFixed newX = hsFixMul(fMap[0][0], src->fX) + hsFixMul(fMap[0][1], src->fY) + translateX;
				hsFixed newY = hsFixMul(fMap[1][0], src->fX) + hsFixMul(fMap[1][1], src->fY) + translateY;
				dst->fX = newX;
				dst->fY = newY;
			}
		else if (matType & kScaleMatrixType)
			for (; count--; ++src, ++dst)
			{	dst->fX = hsFixMul(fMap[0][0], src->fX) + translateX;
				dst->fY = hsFixMul(fMap[1][1], src->fY) + translateY;
			}
		else	// kTranslateMatrixType
			for (; count--; ++src, ++dst)
			{	dst->fX = src->fX + translateX;
				dst->fY = src->fY + translateY;
			}
	}
	return origDst;
}

hsFixedPoint2* hsFixedMatrix33::MapPoints(long count, hsFixedPoint2 points[], hsMatrixType matType) const
{
	return this->MapPoints(count, points, points, matType);
}

////////////////////////////////////////////////////////////////////////////////////

#define kSafeMaxForLinearElement			16383.0
#define kSafeMaxForPerspectiveElement		1.9

static void Normalize(hsFixedMatrix33 *mat)
{
	float	m20 = hsFixedToScalar(mat->fMap[2][0]);
	float	m21 = hsFixedToScalar(mat->fMap[2][1]);
	float	d22 = float(1) / hsFixedToScalar(mat->fMap[2][2]);
	float sign = float(1);

	if (m20 < 0) m20 = -m20;
	if (m21 < 0) m21 = -m21;
	if (d22 < 0)
	{	sign = float(-1);
		d22 = -d22;
	}

	while (m20 * d22 > kSafeMaxForPerspectiveElement || m21 * d22 > kSafeMaxForPerspectiveElement)
		d22 *= float(0.5);

	d22 *= sign;	// restore the sign of d22

	mat->fMap[0][0] = hsFixed(mat->fMap[0][0] * d22);
	mat->fMap[0][1] = hsFixed(mat->fMap[0][1] * d22);
	mat->fMap[0][2] = hsFixed(mat->fMap[0][2] * d22);

	mat->fMap[1][0] = hsFixed(mat->fMap[1][0] * d22);
	mat->fMap[1][1] = hsFixed(mat->fMap[1][1] * d22);	
	mat->fMap[1][2] = hsFixed(mat->fMap[1][2] * d22);
		
	mat->fMap[2][0] = hsScalarToFract(hsFixedToScalar(mat->fMap[2][0]) * d22);
	mat->fMap[2][1] = hsScalarToFract(hsFixedToScalar(mat->fMap[2][1]) * d22);
	mat->fMap[2][2] = hsScalarToFract(hsFixedToScalar(mat->fMap[2][2]) * d22);
}

static inline hsFixed Determinant(const hsFixedMatrix33* matrix)
{
	if (matrix->GetType() & kPerspectiveMatrixType)
		return	hsFixMul(matrix->fMap[0][0], matrix->fMap[1][1]) +
				hsFixMul(hsFracMul(matrix->fMap[0][1], matrix->fMap[1][2]), matrix->fMap[2][0]) +
				hsFixMul(hsFracMul(matrix->fMap[0][2], matrix->fMap[1][0]), matrix->fMap[2][1]) -
				hsFixMul(hsFracMul(matrix->fMap[0][2], matrix->fMap[1][1]), matrix->fMap[2][0]) -
				hsFixMul(hsFracMul(matrix->fMap[0][0], matrix->fMap[1][2]), matrix->fMap[2][1]) -
				hsFixMul(matrix->fMap[0][1], matrix->fMap[1][0]);
	else
		return	hsFixMul(matrix->fMap[0][0], matrix->fMap[1][1]) -
				hsFixMul(matrix->fMap[0][1], matrix->fMap[1][0]);
}

static  double doubleDeterminant( double matrix[3][3])
{
		return	matrix[0][0]*matrix[1][1] +
				((matrix[0][1]*matrix[1][2])* matrix[2][0]) +
				((matrix[0][2]*matrix[1][0])*matrix[2][1]) -
				((matrix[0][2]*matrix[1][1])*matrix[2][0]) -
				((matrix[0][0]* matrix[1][2])* matrix[2][1]) -
				matrix[0][1]*matrix[1][0];
	
}

static void floatInvert(const hsFixedMatrix33* mat, hsFixedMatrix33* inverse)
{
	double m[3][3];
	double inv[3][3];
	int i,j;
	double det_over_1;
	for(i=0; i < 3; i++)
		for(j = 0; j < 3; j++)
			m[i][j] = (i == 2) ? hsFractToScalar(mat->fMap[i][j]) :  hsFixedToScalar(mat->fMap[i][j]);
	det_over_1 = 1.0/doubleDeterminant(m);
	
	inv[0][0] = (m[1][1]*m[2][2] - (m[1][2]* m[2][1]))*det_over_1;
	inv[1][0] = (m[1][2]*m[2][0] - (m[1][0]* m[2][2]))* det_over_1;
	inv[2][0] =(m[1][0]* m[2][1] - (m[1][1]* m[2][0]))*det_over_1;
		
	inv[0][1] = (m[0][2]*m[2][1] - (m[0][1]* m[2][2]))* det_over_1;
	inv[1][1] = (m[0][0]* m[2][2] - (m[0][2]* m[2][0]))* det_over_1;
	inv[2][1] = (m[0][1]* m[2][0] - (m[0][0]* m[2][1]))*det_over_1;
	
	inv[0][2] = (m[0][1]*m[1][2] - (m[0][2]* m[1][1]))* det_over_1;
	inv[1][2] = (m[0][2]* m[1][0] - (m[0][0]* m[1][2]))* det_over_1;
	inv[2][2] = (m[0][0]* m[1][1] - (m[0][1]* m[1][0]))* det_over_1;
	
	double d22 = 1;

	if (mat->GetType() & kPerspectiveMatrixType)
	{	
		double sign = 1;
		double	maxLinearElement = inv[0][0];
		double	maxPerspectiveElement = inv[2][0];
		
		for (i = 0; i < 2; i++)
			for (j = 0; j < 3; j++)
			{	double tmp = inv[i][j];
				if (tmp < 0)
					tmp = -tmp;
				if (tmp > maxLinearElement)
					maxLinearElement = tmp;
			}
		for (j = 1; j < 3; j++)
		{	double tmp = inv[2][j];
			if (tmp < 0)
				tmp = -tmp;
			if (tmp > maxPerspectiveElement)
				maxPerspectiveElement = tmp;
		}
		
		d22 = 1 / inv[2][2];
		if (d22 < 0)
		{	sign = -1;
			d22 = -d22;
		}

		while (	maxLinearElement * d22 > kSafeMaxForLinearElement ||
				maxPerspectiveElement * d22 > kSafeMaxForPerspectiveElement)
			d22 *= 0.5;

		d22 *= sign;	// restore the sign of d22
	}

	inverse->fMap[0][0] = hsScalarToFixed(inv[0][0] * d22); 
	inverse->fMap[0][1] = hsScalarToFixed(inv[0][1] * d22); 
	inverse->fMap[0][2] = hsScalarToFixed(inv[0][2] * d22); 

	inverse->fMap[1][0] = hsScalarToFixed(inv[1][0] * d22); 
	inverse->fMap[1][1] = hsScalarToFixed(inv[1][1] * d22); 
	inverse->fMap[1][2] = hsScalarToFixed(inv[1][2] * d22); 
		
	inverse->fMap[2][0] = hsScalarToFract(inv[2][0] * d22);
	inverse->fMap[2][1] = hsScalarToFract(inv[2][1] * d22);
	inverse->fMap[2][2] = hsScalarToFract(inv[2][2] * d22);
}

hsFixedMatrix33* hsFixedMatrix33::Invert(hsFixedMatrix33* inverse) const
{
#if 1
	floatInvert(this, inverse);
	return inverse;
#endif

	hsMatrixType matType = this->GetType();
	
//	hsAssert(matType & kPerspectiveMatrixType) == 0, "can't invert perspective yet");

	hsFixed	det_over_1;

	inverse->Reset();

	det_over_1 = Determinant(this);
	if (det_over_1 == 0)
		return nil;

	det_over_1 = hsFixDiv(hsFixed1, det_over_1);

	if(!( matType  & kPerspectiveMatrixType) )
	{
		inverse->fMap[0][0] = hsFixMul(fMap[1][1], det_over_1);
		inverse->fMap[0][1] = -hsFixMul(fMap[0][1], det_over_1);
		inverse->fMap[0][2] = hsFixMul(hsFixMul(fMap[0][1], fMap[1][2]) - hsFixMul(fMap[0][2], fMap[1][1]), det_over_1);

		inverse->fMap[1][0] = -hsFixMul(fMap[1][0], det_over_1);
		inverse->fMap[1][1] = hsFixMul(fMap[0][0], det_over_1);
		inverse->fMap[1][2] = hsFixMul(hsFixMul(fMap[1][0], fMap[0][2]) - hsFixMul(fMap[1][2], fMap[0][0]), det_over_1);
	}
	else
	{
#if 1
		inverse->fMap[0][0] = hsFixMul(hsFracMul(fMap[1][1], fMap[2][2]) - hsFracMul(fMap[1][2], fMap[2][1]), det_over_1);
		inverse->fMap[1][0] = hsFixMul(hsFracMul(fMap[1][2], fMap[2][0]) - hsFracMul(fMap[1][0], fMap[2][2]), det_over_1);
		inverse->fMap[2][0] = hsFixMul(hsFracMul(fMap[1][0], fMap[2][1]) - hsFracMul(fMap[1][1], fMap[2][0]), det_over_1);
			
		inverse->fMap[0][1] = hsFixMul(hsFracMul(fMap[0][2], fMap[2][1]) - hsFracMul(fMap[0][1], fMap[2][2]), det_over_1);
		inverse->fMap[1][1] = hsFixMul(hsFracMul(fMap[0][0], fMap[2][2]) - hsFracMul(fMap[0][2], fMap[2][0]), det_over_1);
		inverse->fMap[2][1] = hsFixMul(hsFracMul(fMap[0][1], fMap[2][0]) - hsFracMul(fMap[0][0], fMap[2][1]), det_over_1);
		
		inverse->fMap[0][2] = hsFixMul(hsFixMul(fMap[0][1], fMap[1][2]) - hsFixMul(fMap[0][2], fMap[1][1]), det_over_1);
		inverse->fMap[1][2] = hsFixMul(hsFixMul(fMap[0][2], fMap[1][0]) - hsFixMul(fMap[0][0], fMap[1][2]), det_over_1);
		inverse->fMap[2][2] = hsFixMul(hsFixMul(fMap[0][0], fMap[1][1]) - hsFixMul(fMap[0][1], fMap[1][0]), det_over_1);
		Normalize(inverse);
#else	
		floatInvert(this, inverse);
#endif
		
	}
	
	return inverse;
}

////////////////////////////////////////////////////////////////////////////////////

hsFixedMatrix33 operator*(const hsFixedMatrix33& a, const hsFixedMatrix33& b)
{
	hsFixedMatrix33	c;
	hsMatrixType		aType, bType;

	if ((aType = a.GetType()) == kIdentityMatrixType)
		return b;
	if ((bType = b.GetType()) == kIdentityMatrixType)
		return a;

	c.fMap[0][0] = hsFixMul(a.fMap[0][0], b.fMap[0][0]) + hsFixMul(a.fMap[0][1], b.fMap[1][0]) + hsFracMul(a.fMap[0][2], b.fMap[2][0]);
	c.fMap[0][1] = hsFixMul(a.fMap[0][0], b.fMap[0][1]) + hsFixMul(a.fMap[0][1], b.fMap[1][1]) + hsFracMul(a.fMap[0][2], b.fMap[2][1]);
	c.fMap[0][2] = hsFixMul(a.fMap[0][0], b.fMap[0][2]) + hsFixMul(a.fMap[0][1], b.fMap[1][2]) + hsFracMul(a.fMap[0][2], b.fMap[2][2]);

	c.fMap[1][0] = hsFixMul(a.fMap[1][0], b.fMap[0][0]) + hsFixMul(a.fMap[1][1], b.fMap[1][0]) + hsFracMul(a.fMap[1][2], b.fMap[2][0]);
	c.fMap[1][1] = hsFixMul(a.fMap[1][0], b.fMap[0][1]) + hsFixMul(a.fMap[1][1], b.fMap[1][1]) + hsFracMul(a.fMap[1][2], b.fMap[2][1]);
	c.fMap[1][2] = hsFixMul(a.fMap[1][0], b.fMap[0][2]) + hsFixMul(a.fMap[1][1], b.fMap[1][2]) + hsFracMul(a.fMap[1][2], b.fMap[2][2]);

	if ((aType | bType) & kPerspectiveMatrixType)
	{	
		c.fMap[2][0] = hsFracMul(a.fMap[2][0], b.fMap[0][0]) + hsFracMul(a.fMap[2][1], b.fMap[1][0]) + hsFractToFixed(hsFracMul(a.fMap[2][2], b.fMap[2][0]));
		c.fMap[2][1] = hsFracMul(a.fMap[2][0], b.fMap[0][1]) + hsFracMul(a.fMap[2][1], b.fMap[1][1]) + hsFractToFixed(hsFracMul(a.fMap[2][2], b.fMap[2][1]));
		c.fMap[2][2] = hsFracMul(a.fMap[2][0], b.fMap[0][2]) + hsFracMul(a.fMap[2][1], b.fMap[1][2]) + hsFractToFixed(hsFracMul(a.fMap[2][2], b.fMap[2][2]));
		Normalize(&c);

	}
	else
	{	c.fMap[2][0] = 0;
		c.fMap[2][1] = 0;
		c.fMap[2][2] = hsFract1;
	}
	return c;
}

////////////////////////////////////////////////////////////////////////////////////

static void QuadToPoint(const hsFixedPoint2 quad[], hsFixedPoint2* pt)
{
	hsFixed	dx = quad[1].fX - quad[0].fX;
	hsFixed	dy = quad[1].fY - quad[0].fY;

	pt->fY = hsMagnitude(dx, dy);
	
	hsWide	wide1, wide2;

	wide1.Mul(dx, quad[0].fY - quad[3].fY);
	wide2.Mul(dy, quad[3].fX - quad[0].fX);
	wide1.Add(&wide2);
	wide1.Div(pt->fY);

	pt->fX = wide1.AsLong();
}

static void Map4Pt(const hsFixedPoint2 source[], hsFixedMatrix33& dest, hsFixed scaleX, hsFixed scaleY)
{
	hsFract	a1, a2;
	hsFixed	x0, y0, x1, y1, x2, y2;

	x0 = source[2].fX - source[0].fX;
	y0 = source[2].fY - source[0].fY;
	x1 = source[2].fX - source[1].fX;
	y1 = source[2].fY - source[1].fY;
	x2 = source[2].fX - source[3].fX;
	y2 = source[2].fY - source[3].fY;

	/* check if abs(x2) > abs(y2) */
	if ( x2 > 0 ? y2 > 0 ? x2 > y2 : x2 > -y2 : y2 > 0 ? -x2 > y2 : x2 < y2)
		a1 = hsFracDiv(hsMulDiv(x0 - x1, y2, x2) - y0 + y1, hsMulDiv(x1, y2, x2) - y1);
	else
		a1 = hsFracDiv(x0 - x1 - hsMulDiv(y0 - y1, x2, y2), x1 - hsMulDiv(y1, x2, y2));

	/* check if abs(x1) > abs(y1) */
	if ( x1 > 0 ? y1 > 0 ? x1 > y1 : x1 > -y1 : y1 > 0 ? -x1 > y1 : x1 < y1)
		a2 = hsFracDiv(y0 - y2 - hsMulDiv(x0 - x2, y1, x1), y2 - hsMulDiv(x2, y1, x1));
	else
		a2 = hsFracDiv(hsMulDiv(y0 - y2, x1, y1) - x0 + x2, hsMulDiv(y2, x1, y1) - x2);

	dest.fMap[0][0] = hsFixDiv(hsFracMul(a2, source[3].fX) + source[3].fX - source[0].fX, scaleX);
	dest.fMap[1][0]  = hsFixDiv(hsFracMul(a2, source[3].fY) + source[3].fY - source[0].fY, scaleX);
	dest.fMap[2][0]  = hsFixDiv(a2, scaleX);
	dest.fMap[0][1]  = hsFixDiv(hsFracMul(a1, source[1].fX) + source[1].fX - source[0].fX, scaleY);
	dest.fMap[1][1]  = hsFixDiv(hsFracMul(a1, source[1].fY) + source[1].fY - source[0].fY, scaleY);
	dest.fMap[2][1]  = hsFixDiv(a1, scaleY);
	dest.fMap[0][2]  = source[0].fX;
	dest.fMap[1][2]  = source[0].fY;
	dest.fMap[2][2]  = hsFract1;
}

hsFixedMatrix33* hsFixedMatrix33::QuadToQuad(const hsFixedPoint2 srcQuad[], const hsFixedPoint2 dstQuad[])
{
	hsFixedPoint2		tempPt;
	hsFixedMatrix33	tempMap;

	QuadToPoint(srcQuad, &tempPt);

	Map4Pt(srcQuad, tempMap, tempPt.fX, tempPt.fY);
	if (tempMap.Invert(this) == nil)
		return nil;

	Map4Pt(dstQuad, tempMap, tempPt.fX, tempPt.fY);
	*this = *this * tempMap;
	return this;
}
