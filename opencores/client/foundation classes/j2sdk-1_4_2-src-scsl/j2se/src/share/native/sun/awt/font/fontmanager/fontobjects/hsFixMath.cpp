/*
 * @(#)hsFixMath.cpp	1.6 03/01/23
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

#include "HeadSpin.h"
#include "hsFixedTypes.h"
#include "hsWide.h"

#if HS_BUILD_FOR_WIN32
#define HS_FLOAT_HACK		1
#endif

//////////////////////////////////////////////////////////////////////////////////////

#if HS_BUILD_FOR_MACPPC && !(HS_PIN_MATH_OVERFLOW)
asm hsFixed hsFixMul(hsFixed a, hsFixed b)
{
	mullw    r6,r3,r4
	mulhw    r5,r3,r4
	rlwinm	r3,r5,16,0,15
	rlwimi	r3,r6,16,16,31
 	blr
};
#elif HS_PIN_MATH_OVERFLOW || !(HS_BUILD_FOR_MAC) || HS_MP_SAFE
hsFixed hsFixMul(hsFixed a, hsFixed b)
{
#if HS_FLOAT_HACK
	return hsFixed(double(a) * double(b) / double(hsFixed1));
#else
	hsWide	w;

	return w.Mul(a, b)->AsFixed();
#endif
}
#endif

#if HS_PIN_MATH_OVERFLOW || !(HS_BUILD_FOR_MAC) || HS_MP_SAFE
hsFixed hsFixDiv(hsFixed a, hsFixed b)
{
#if HS_FLOAT_HACK
	return hsFixed(double(a) / double(b) * double(hsFixed1));
#else
	hsWide	w;

	return w.Set(a)->ShiftLeft(16)->Div(b)->AsLong();
#endif
}

hsFract hsFracMul(hsFract a, hsFract b)
{
#if HS_FLOAT_HACK
	return hsFract(double(a) * double(b) / double(hsFract1));
#else
	hsWide	w;

	return w.Mul(a, b)->AsFract();
#endif
}

hsFract hsFracDiv(hsFract a, hsFract b)
{
#if HS_FLOAT_HACK
	return hsFract(double(a) / double(b) * double(hsFract1));
#else
	hsWide	w;

	return w.Set(a)->ShiftLeft(30)->Div(b)->AsLong();
#endif
}
#endif

Int32 hsMulDiv(Int32 numer1, Int32 numer2, Int32 denom)
{
#if HS_FLOAT_HACK
	return Int32(double(numer1) * double(numer2) / double(denom));
#else
	hsWide	w;

	return w.Mul(numer1, numer2)->Div(denom)->AsLong();
#endif
}

Int32 hsMagnitude(Int32 x, Int32 y)
{
	hsWide	w, w2;

	return w.Mul(x, x)->Add(w2.Mul(y, y))->Sqrt();
}

///////////////////////////////////////////////////////////////////////////////////

#define kSinTableScale	10638985
#define kSinTableSize	255

hsFixed SinTable[256] = {
0, /* sin(0.000000)=0.000000 */
403, /* sin(0.006160)=0.006160 */
807, /* sin(0.012320)=0.012320 */
1211, /* sin(0.018480)=0.018479 */
1614, /* sin(0.024640)=0.024637 */
2018, /* sin(0.030800)=0.030795 */
2421, /* sin(0.036960)=0.036951 */
2825, /* sin(0.043120)=0.043107 */
3228, /* sin(0.049280)=0.049260 */
3631, /* sin(0.055440)=0.055411 */
4034, /* sin(0.061600)=0.061561 */
4437, /* sin(0.067760)=0.067708 */
4839, /* sin(0.073920)=0.073853 */
5242, /* sin(0.080080)=0.079994 */
5644, /* sin(0.086240)=0.086133 */
6046, /* sin(0.092400)=0.092268 */
6448, /* sin(0.098560)=0.098400 */
6850, /* sin(0.104720)=0.104528 */
7251, /* sin(0.110880)=0.110653 */
7652, /* sin(0.117040)=0.116773 */
8053, /* sin(0.123200)=0.122888 */
8454, /* sin(0.129360)=0.128999 */
8854, /* sin(0.135520)=0.135105 */
9254, /* sin(0.141680)=0.141206 */
9653, /* sin(0.147840)=0.147302 */
10052, /* sin(0.154000)=0.153392 */
10451, /* sin(0.160160)=0.159476 */
10849, /* sin(0.166320)=0.165554 */
11247, /* sin(0.172480)=0.171626 */
11645, /* sin(0.178640)=0.177691 */
12042, /* sin(0.184800)=0.183750 */
12438, /* sin(0.190960)=0.189801 */
12834, /* sin(0.197120)=0.195845 */
13230, /* sin(0.203280)=0.201882 */
13625, /* sin(0.209440)=0.207912 */
14020, /* sin(0.215599)=0.213933 */
14414, /* sin(0.221759)=0.219946 */
14807, /* sin(0.227919)=0.225951 */
15200, /* sin(0.234079)=0.231948 */
15593, /* sin(0.240239)=0.237935 */
15985, /* sin(0.246399)=0.243914 */
16376, /* sin(0.252559)=0.249883 */
16766, /* sin(0.258719)=0.255843 */
17156, /* sin(0.264879)=0.261793 */
17546, /* sin(0.271039)=0.267733 */
17934, /* sin(0.277199)=0.273663 */
18322, /* sin(0.283359)=0.279583 */
18709, /* sin(0.289519)=0.285492 */
19096, /* sin(0.295679)=0.291390 */
19482, /* sin(0.301839)=0.297277 */
19867, /* sin(0.307999)=0.303153 */
20251, /* sin(0.314159)=0.309017 */
20635, /* sin(0.320319)=0.314870 */
21018, /* sin(0.326479)=0.320710 */
21400, /* sin(0.332639)=0.326539 */
21781, /* sin(0.338799)=0.332355 */
22161, /* sin(0.344959)=0.338158 */
22541, /* sin(0.351119)=0.343949 */
22919, /* sin(0.357279)=0.349727 */
23297, /* sin(0.363439)=0.355491 */
23674, /* sin(0.369599)=0.361242 */
24050, /* sin(0.375759)=0.366979 */
24425, /* sin(0.381919)=0.372702 */
24799, /* sin(0.388079)=0.378411 */
25172, /* sin(0.394239)=0.384106 */
25545, /* sin(0.400399)=0.389786 */
25916, /* sin(0.406559)=0.395451 */
26286, /* sin(0.412719)=0.401102 */
26655, /* sin(0.418879)=0.406737 */
27024, /* sin(0.425039)=0.412356 */
27391, /* sin(0.431199)=0.417960 */
27757, /* sin(0.437359)=0.423549 */
28122, /* sin(0.443519)=0.429121 */
28486, /* sin(0.449679)=0.434676 */
28849, /* sin(0.455839)=0.440216 */
29211, /* sin(0.461999)=0.445738 */
29572, /* sin(0.468159)=0.451244 */
29932, /* sin(0.474319)=0.456733 */
30290, /* sin(0.480479)=0.462204 */
30648, /* sin(0.486639)=0.467658 */
31004, /* sin(0.492799)=0.473094 */
31359, /* sin(0.498959)=0.478512 */
31713, /* sin(0.505119)=0.483911 */
32066, /* sin(0.511279)=0.489293 */
32417, /* sin(0.517439)=0.494656 */
32768, /* sin(0.523599)=0.500000 */
33116, /* sin(0.529759)=0.505325 */
33464, /* sin(0.535919)=0.510631 */
33811, /* sin(0.542079)=0.515918 */
34156, /* sin(0.548239)=0.521185 */
34500, /* sin(0.554399)=0.526432 */
34842, /* sin(0.560559)=0.531659 */
35184, /* sin(0.566719)=0.536867 */
35524, /* sin(0.572879)=0.542053 */
35862, /* sin(0.579039)=0.547220 */
36199, /* sin(0.585199)=0.552365 */
36535, /* sin(0.591359)=0.557489 */
36870, /* sin(0.597519)=0.562593 */
37203, /* sin(0.603679)=0.567675 */
37534, /* sin(0.609839)=0.572735 */
37864, /* sin(0.615999)=0.577774 */
38193, /* sin(0.622159)=0.582791 */
38521, /* sin(0.628319)=0.587785 */
38846, /* sin(0.634479)=0.592758 */
39171, /* sin(0.640639)=0.597708 */
39494, /* sin(0.646798)=0.602635 */
39815, /* sin(0.652959)=0.607539 */
40135, /* sin(0.659118)=0.612420 */
40453, /* sin(0.665278)=0.617278 */
40770, /* sin(0.671438)=0.622113 */
41086, /* sin(0.677598)=0.626924 */
41399, /* sin(0.683758)=0.631711 */
41711, /* sin(0.689918)=0.636474 */
42022, /* sin(0.696078)=0.641213 */
42331, /* sin(0.702238)=0.645928 */
42638, /* sin(0.708398)=0.650618 */
42944, /* sin(0.714558)=0.655284 */
43248, /* sin(0.720718)=0.659925 */
43551, /* sin(0.726878)=0.664540 */
43852, /* sin(0.733038)=0.669131 */
44151, /* sin(0.739198)=0.673696 */
44448, /* sin(0.745358)=0.678235 */
44744, /* sin(0.751518)=0.682749 */
45038, /* sin(0.757678)=0.687237 */
45331, /* sin(0.763838)=0.691698 */
45621, /* sin(0.769998)=0.696134 */
45910, /* sin(0.776158)=0.700543 */
46198, /* sin(0.782318)=0.704926 */
46483, /* sin(0.788478)=0.709281 */
46767, /* sin(0.794638)=0.713610 */
47049, /* sin(0.800798)=0.717912 */
47329, /* sin(0.806958)=0.722186 */
47607, /* sin(0.813118)=0.726434 */
47884, /* sin(0.819278)=0.730653 */
48158, /* sin(0.825438)=0.734845 */
48431, /* sin(0.831598)=0.739009 */
48702, /* sin(0.837758)=0.743145 */
48971, /* sin(0.843918)=0.747253 */
49239, /* sin(0.850078)=0.751332 */
49504, /* sin(0.856238)=0.755383 */
49768, /* sin(0.862398)=0.759405 */
50030, /* sin(0.868558)=0.763398 */
50289, /* sin(0.874718)=0.767363 */
50547, /* sin(0.880878)=0.771298 */
50803, /* sin(0.887038)=0.775204 */
51057, /* sin(0.893198)=0.779081 */
51309, /* sin(0.899358)=0.782928 */
51560, /* sin(0.905518)=0.786745 */
51808, /* sin(0.911678)=0.790532 */
52054, /* sin(0.917838)=0.794290 */
52298, /* sin(0.923998)=0.798017 */
52541, /* sin(0.930158)=0.801714 */
52781, /* sin(0.936318)=0.805381 */
53019, /* sin(0.942478)=0.809017 */
53256, /* sin(0.948638)=0.812622 */
53490, /* sin(0.954798)=0.816197 */
53722, /* sin(0.960958)=0.819740 */
53952, /* sin(0.967118)=0.823253 */
54180, /* sin(0.973278)=0.826734 */
54406, /* sin(0.979438)=0.830184 */
54630, /* sin(0.985598)=0.833602 */
54852, /* sin(0.991758)=0.836989 */
55072, /* sin(0.997918)=0.840344 */
55290, /* sin(1.004078)=0.843667 */
55506, /* sin(1.010238)=0.846958 */
55719, /* sin(1.016398)=0.850217 */
55931, /* sin(1.022558)=0.853444 */
56140, /* sin(1.028718)=0.856638 */
56347, /* sin(1.034878)=0.859800 */
56552, /* sin(1.041038)=0.862929 */
56755, /* sin(1.047198)=0.866025 */
56956, /* sin(1.053358)=0.869089 */
57155, /* sin(1.059518)=0.872119 */
57351, /* sin(1.065678)=0.875117 */
57545, /* sin(1.071838)=0.878081 */
57738, /* sin(1.077997)=0.881012 */
57927, /* sin(1.084157)=0.883910 */
58115, /* sin(1.090317)=0.886774 */
58301, /* sin(1.096478)=0.889604 */
58484, /* sin(1.102637)=0.892401 */
58665, /* sin(1.108797)=0.895163 */
58844, /* sin(1.114957)=0.897892 */
59020, /* sin(1.121117)=0.900587 */
59195, /* sin(1.127277)=0.903247 */
59367, /* sin(1.133437)=0.905873 */
59537, /* sin(1.139597)=0.908465 */
59704, /* sin(1.145757)=0.911023 */
59870, /* sin(1.151917)=0.913545 */
60033, /* sin(1.158077)=0.916034 */
60193, /* sin(1.164237)=0.918487 */
60352, /* sin(1.170397)=0.920906 */
60508, /* sin(1.176557)=0.923289 */
60662, /* sin(1.182717)=0.925638 */
60814, /* sin(1.188877)=0.927951 */
60963, /* sin(1.195037)=0.930229 */
61110, /* sin(1.201197)=0.932472 */
61255, /* sin(1.207357)=0.934680 */
61397, /* sin(1.213517)=0.936852 */
61537, /* sin(1.219677)=0.938988 */
61675, /* sin(1.225837)=0.941089 */
61810, /* sin(1.231997)=0.943154 */
61943, /* sin(1.238157)=0.945184 */
62074, /* sin(1.244317)=0.947177 */
62202, /* sin(1.250477)=0.949135 */
62328, /* sin(1.256637)=0.951057 */
62452, /* sin(1.262797)=0.952942 */
62573, /* sin(1.268957)=0.954791 */
62692, /* sin(1.275117)=0.956604 */
62808, /* sin(1.281277)=0.958381 */
62922, /* sin(1.287437)=0.960122 */
63034, /* sin(1.293597)=0.961826 */
63143, /* sin(1.299757)=0.963493 */
63250, /* sin(1.305917)=0.965124 */
63354, /* sin(1.312077)=0.966718 */
63456, /* sin(1.318237)=0.968276 */
63556, /* sin(1.324397)=0.969797 */
63653, /* sin(1.330557)=0.971281 */
63748, /* sin(1.336717)=0.972728 */
63841, /* sin(1.342877)=0.974139 */
63931, /* sin(1.349037)=0.975512 */
64018, /* sin(1.355197)=0.976848 */
64103, /* sin(1.361357)=0.978148 */
64186, /* sin(1.367517)=0.979410 */
64266, /* sin(1.373677)=0.980635 */
64344, /* sin(1.379837)=0.981823 */
64420, /* sin(1.385997)=0.982973 */
64493, /* sin(1.392157)=0.984086 */
64563, /* sin(1.398317)=0.985162 */
64631, /* sin(1.404477)=0.986201 */
64697, /* sin(1.410637)=0.987202 */
64760, /* sin(1.416797)=0.988165 */
64821, /* sin(1.422957)=0.989092 */
64879, /* sin(1.429117)=0.989980 */
64935, /* sin(1.435277)=0.990831 */
64988, /* sin(1.441437)=0.991645 */
65039, /* sin(1.447597)=0.992420 */
65087, /* sin(1.453757)=0.993159 */
65133, /* sin(1.459917)=0.993859 */
65176, /* sin(1.466077)=0.994522 */
65217, /* sin(1.472237)=0.995147 */
65256, /* sin(1.478397)=0.995734 */
65292, /* sin(1.484557)=0.996284 */
65325, /* sin(1.490717)=0.996795 */
65357, /* sin(1.496876)=0.997269 */
65385, /* sin(1.503036)=0.997705 */
65411, /* sin(1.509197)=0.998103 */
65435, /* sin(1.515356)=0.998464 */
65456, /* sin(1.521516)=0.998786 */
65475, /* sin(1.527676)=0.999070 */
65491, /* sin(1.533836)=0.999317 */
65504, /* sin(1.539996)=0.999526 */
65516, /* sin(1.546156)=0.999696 */
65524, /* sin(1.552316)=0.999829 */
65531, /* sin(1.558476)=0.999924 */
65534, /* sin(1.564636)=0.999981 */
65536 /* sin(1.570796)=1.000000 */
};


hsFixed hsFixedCos(hsFixed s)
{
	return hsFixedSin(hsFixedPiOver2 + s);
}

hsFixed hsFixedSin(hsFixed s)
{
 	int index;
 	int swap = 0;
 	int rVal;
 	if( s < 0)
 	{
 		swap = 1;
 		s = -s;
 	}
 	
 	index = hsFixedRound(hsFixMul(s, kSinTableScale));
 	
 	if(index > kSinTableSize*4)
 		index %= kSinTableSize*4;
 	if(index > kSinTableSize*3)
 	{
 		index = kSinTableSize*4-index;
 		swap = !swap;
 	}
 	else if(index > kSinTableSize * 2)
 	{
 		index = index- kSinTableSize * 2;
 		swap = !swap;
 	}
 	else if(index > kSinTableSize)
 	{
 		index = kSinTableSize * 2 - index;
 	}
 	rVal = SinTable[index];
 	if(swap)
 		rVal = -rVal;
 	return rVal;
 	
}

///////////////////////////////////////////////////////////////////////////

UInt16 hsSqrt32(UInt32 value)
{
	UInt32	curr = 0;
	UInt32	root = 0;
	int		bits = 16;

	do {
		curr = (curr << 2) | TOP2BITS(value);
		value <<= 2;
		UInt32 guess = root << 2;
		root <<= 1;
		if (guess < curr)
		{	curr -= guess + 1;
			root |= 1;
		}
	} while (--bits);

	return UInt16(root);
}

hsFract hsFracSqrt(hsFract value)
{
	UInt32	curr = 0;
	UInt32	root = 0;
	int		bits = 31;

	do {
		curr = (curr << 2) | TOP2BITS(value);
		value <<= 2;
		UInt32 guess = root << 2;
		root <<= 1;
		if (guess < curr)
		{	curr -= guess + 1;
			root |= 1;
		}
	} while (--bits);

	return root;
}

UInt16 hsCubeRoot(UInt32 value)
{
	UInt32	curr;
	UInt32	root = 0;
	UInt32	bits = 11;

	curr = (unsigned)value >> 30;
	value <<= 2;
	do {
		root <<= 1;
		UInt32 guess = 3 * (root * root + root);
		if (guess < curr)
		{	curr -= guess + 1;
			root |= 1;
		}
		curr = (curr << 3) | TOP3BITS(value);
		value <<= 3;
	} while (--bits);

	return UInt16(root);
}

hsFract hsFracCubeRoot(hsFract value)
{
	int		bits = 31;
	UInt32	root = 0;
	UInt32	currH, currL;
	UInt32	guessH, guessL;
	Boolean	neg = false;

	if (value < 0)
	{	neg = true;
		value = -value;
	}

	currH = 0;
	currL = value >> 30;	// bit 30 (bit 31 is zero since we're positive)
	value <<= 2;
	do {
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
		WIDE_SHIFTLEFT(currH, currL, currH, currL, 3);
		currL |= TOP3BITS(value);
		value <<= 3;
	} while (--bits);

	if (neg)
		root = -Int32(root);
	return (Int32)root;
}

