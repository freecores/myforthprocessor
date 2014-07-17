/*
 * @(#)nrOutlines.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
/* 
 * @author Joe Warzecha
 */ 

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#ifdef HAVE_DPS

/* NOTE: This file (nrOutlines.c) was generated from nrOutlines.psw
   by unix pswrap V1.009  Wed Apr 19 17:50:24 PDT 1989
 */

#include <DPS/dpsfriends.h>
#include <DPS/dpsclient.h>
#include <string.h>

#line 27 "nrOutlines.c"
void OutlineInit( void )
{
  typedef struct {
    unsigned char tokenType;
    unsigned char topLevelCount;
    unsigned short nBytes;

    DPSBinObjGeneric obj0;
    DPSBinObjGeneric obj1;
    DPSBinObjGeneric obj2;
    DPSBinObjGeneric obj3;
    DPSBinObjGeneric obj4;
    DPSBinObjGeneric obj5;
    DPSBinObjGeneric obj6;
    DPSBinObjGeneric obj7;
    DPSBinObjGeneric obj8;
    DPSBinObjGeneric obj9;
    DPSBinObjGeneric obj10;
    DPSBinObjGeneric obj11;
    DPSBinObjGeneric obj12;
    DPSBinObjGeneric obj13;
    DPSBinObjGeneric obj14;
    DPSBinObjGeneric obj15;
    DPSBinObjGeneric obj16;
    DPSBinObjGeneric obj17;
    DPSBinObjGeneric obj18;
    DPSBinObjGeneric obj19;
    DPSBinObjGeneric obj20;
    DPSBinObjGeneric obj21;
    DPSBinObjGeneric obj22;
    DPSBinObjGeneric obj23;
    DPSBinObjGeneric obj24;
    DPSBinObjGeneric obj25;
    DPSBinObjGeneric obj26;
    DPSBinObjGeneric obj27;
    DPSBinObjGeneric obj28;
    DPSBinObjGeneric obj29;
    DPSBinObjGeneric obj30;
    DPSBinObjGeneric obj31;
    DPSBinObjGeneric obj32;
    DPSBinObjGeneric obj33;
    DPSBinObjGeneric obj34;
    DPSBinObjGeneric obj35;
    DPSBinObjGeneric obj36;
    DPSBinObjGeneric obj37;
    DPSBinObjGeneric obj38;
    DPSBinObjGeneric obj39;
    DPSBinObjGeneric obj40;
    DPSBinObjGeneric obj41;
    DPSBinObjGeneric obj42;
    DPSBinObjGeneric obj43;
    DPSBinObjGeneric obj44;
    DPSBinObjGeneric obj45;
    DPSBinObjGeneric obj46;
    DPSBinObjGeneric obj47;
    DPSBinObjGeneric obj48;
    DPSBinObjGeneric obj49;
    DPSBinObjGeneric obj50;
    DPSBinObjGeneric obj51;
    DPSBinObjGeneric obj52;
    DPSBinObjGeneric obj53;
    DPSBinObjGeneric obj54;
    DPSBinObjGeneric obj55;
    DPSBinObjGeneric obj56;
    DPSBinObjGeneric obj57;
    DPSBinObjGeneric obj58;
    DPSBinObjGeneric obj59;
    DPSBinObjGeneric obj60;
    DPSBinObjGeneric obj61;
    DPSBinObjGeneric obj62;
    DPSBinObjGeneric obj63;
    DPSBinObjGeneric obj64;
    DPSBinObjGeneric obj65;
    DPSBinObjGeneric obj66;
    DPSBinObjGeneric obj67;
    DPSBinObjGeneric obj68;
    DPSBinObjGeneric obj69;
    DPSBinObjGeneric obj70;
    DPSBinObjGeneric obj71;
    DPSBinObjGeneric obj72;
    DPSBinObjGeneric obj73;
    DPSBinObjGeneric obj74;
    DPSBinObjGeneric obj75;
    DPSBinObjGeneric obj76;
    DPSBinObjGeneric obj77;
    DPSBinObjGeneric obj78;
    DPSBinObjGeneric obj79;
    DPSBinObjGeneric obj80;
    DPSBinObjGeneric obj81;
    DPSBinObjGeneric obj82;
    DPSBinObjGeneric obj83;
    DPSBinObjGeneric obj84;
    DPSBinObjGeneric obj85;
    DPSBinObjGeneric obj86;
    DPSBinObjGeneric obj87;
    DPSBinObjGeneric obj88;
    DPSBinObjGeneric obj89;
    DPSBinObjGeneric obj90;
    DPSBinObjGeneric obj91;
    DPSBinObjGeneric obj92;
    DPSBinObjGeneric obj93;
    DPSBinObjGeneric obj94;
    DPSBinObjGeneric obj95;
    DPSBinObjGeneric obj96;
    DPSBinObjGeneric obj97;
    DPSBinObjGeneric obj98;
    DPSBinObjGeneric obj99;
    DPSBinObjGeneric obj100;
    DPSBinObjGeneric obj101;
    DPSBinObjGeneric obj102;
    DPSBinObjGeneric obj103;
    DPSBinObjGeneric obj104;
    DPSBinObjGeneric obj105;
    DPSBinObjGeneric obj106;
    DPSBinObjGeneric obj107;
    DPSBinObjGeneric obj108;
    DPSBinObjGeneric obj109;
    DPSBinObjGeneric obj110;
    DPSBinObjGeneric obj111;
    DPSBinObjGeneric obj112;
    DPSBinObjGeneric obj113;
    DPSBinObjGeneric obj114;
    DPSBinObjGeneric obj115;
    DPSBinObjGeneric obj116;
    DPSBinObjGeneric obj117;
    DPSBinObjGeneric obj118;
    DPSBinObjGeneric obj119;
    DPSBinObjGeneric obj120;
    DPSBinObjGeneric obj121;
    DPSBinObjGeneric obj122;
    DPSBinObjGeneric obj123;
    DPSBinObjGeneric obj124;
    DPSBinObjGeneric obj125;
    DPSBinObjGeneric obj126;
    DPSBinObjGeneric obj127;
    DPSBinObjGeneric obj128;
    DPSBinObjGeneric obj129;
    DPSBinObjGeneric obj130;
    DPSBinObjGeneric obj131;
    DPSBinObjGeneric obj132;
    DPSBinObjGeneric obj133;
    DPSBinObjGeneric obj134;
    DPSBinObjGeneric obj135;
    DPSBinObjGeneric obj136;
    DPSBinObjGeneric obj137;
    DPSBinObjGeneric obj138;
    DPSBinObjGeneric obj139;
    DPSBinObjGeneric obj140;
    DPSBinObjGeneric obj141;
    DPSBinObjGeneric obj142;
    DPSBinObjGeneric obj143;
    DPSBinObjGeneric obj144;
    DPSBinObjGeneric obj145;
    DPSBinObjGeneric obj146;
    DPSBinObjGeneric obj147;
    DPSBinObjGeneric obj148;
    DPSBinObjGeneric obj149;
    DPSBinObjGeneric obj150;
    DPSBinObjGeneric obj151;
    DPSBinObjGeneric obj152;
    DPSBinObjGeneric obj153;
    DPSBinObjGeneric obj154;
    DPSBinObjGeneric obj155;
    DPSBinObjGeneric obj156;
    DPSBinObjGeneric obj157;
    DPSBinObjGeneric obj158;
    DPSBinObjGeneric obj159;
    DPSBinObjGeneric obj160;
    DPSBinObjGeneric obj161;
    DPSBinObjGeneric obj162;
    DPSBinObjGeneric obj163;
    DPSBinObjGeneric obj164;
    DPSBinObjGeneric obj165;
    DPSBinObjGeneric obj166;
    DPSBinObjGeneric obj167;
    DPSBinObjGeneric obj168;
    DPSBinObjGeneric obj169;
    DPSBinObjGeneric obj170;
    DPSBinObjGeneric obj171;
    DPSBinObjGeneric obj172;
    DPSBinObjGeneric obj173;
    DPSBinObjGeneric obj174;
    DPSBinObjGeneric obj175;
    DPSBinObjGeneric obj176;
    DPSBinObjGeneric obj177;
    DPSBinObjGeneric obj178;
    DPSBinObjGeneric obj179;
    DPSBinObjGeneric obj180;
    DPSBinObjGeneric obj181;
    DPSBinObjGeneric obj182;
    DPSBinObjGeneric obj183;
    DPSBinObjGeneric obj184;
    DPSBinObjGeneric obj185;
    DPSBinObjGeneric obj186;
    DPSBinObjGeneric obj187;
    DPSBinObjGeneric obj188;
    DPSBinObjGeneric obj189;
    DPSBinObjGeneric obj190;
    DPSBinObjGeneric obj191;
    DPSBinObjGeneric obj192;
    DPSBinObjGeneric obj193;
    DPSBinObjGeneric obj194;
    DPSBinObjGeneric obj195;
    DPSBinObjGeneric obj196;
    DPSBinObjGeneric obj197;
    DPSBinObjGeneric obj198;
    DPSBinObjGeneric obj199;
    DPSBinObjGeneric obj200;
    DPSBinObjGeneric obj201;
    DPSBinObjGeneric obj202;
    DPSBinObjGeneric obj203;
    DPSBinObjGeneric obj204;
    DPSBinObjGeneric obj205;
    DPSBinObjGeneric obj206;
    DPSBinObjGeneric obj207;
    DPSBinObjGeneric obj208;
    DPSBinObjGeneric obj209;
    DPSBinObjGeneric obj210;
    DPSBinObjGeneric obj211;
    DPSBinObjGeneric obj212;
    DPSBinObjGeneric obj213;
    DPSBinObjGeneric obj214;
    DPSBinObjGeneric obj215;
    DPSBinObjGeneric obj216;
    DPSBinObjGeneric obj217;
    DPSBinObjGeneric obj218;
    DPSBinObjGeneric obj219;
    DPSBinObjGeneric obj220;
    DPSBinObjGeneric obj221;
    DPSBinObjGeneric obj222;
    DPSBinObjGeneric obj223;
    DPSBinObjGeneric obj224;
    DPSBinObjGeneric obj225;
    DPSBinObjGeneric obj226;
    DPSBinObjGeneric obj227;
    DPSBinObjGeneric obj228;
    DPSBinObjGeneric obj229;
    DPSBinObjGeneric obj230;
    DPSBinObjGeneric obj231;
    DPSBinObjGeneric obj232;
    DPSBinObjGeneric obj233;
    } _dpsQ;
  static const _dpsQ _dpsStat = {
    DPS_DEF_TOKENTYPE, 65, 1876,
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* pointSize */
    {DPS_LITERAL|DPS_INT, 0, 0, 1000},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointSize */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 9},	/* array */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* opSize */
    {DPS_LITERAL|DPS_INT, 0, 0, 300},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* ops */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* opSize */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 9},	/* array */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* pathPointSize */
    {DPS_LITERAL|DPS_INT, 0, 0, 200},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* pathPoints */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pathPointSize */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 9},	/* array */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nPaths */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* singleByte */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 165},	/* string */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* doubleByte */
    {DPS_LITERAL|DPS_INT, 0, 0, 2},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 165},	/* string */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* movetoExecuted */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* linetoExecuted */
    {DPS_LITERAL|DPS_INT, 0, 0, 2},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* curvetoExecuted */
    {DPS_LITERAL|DPS_INT, 0, 0, 3},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* closepathExecuted */
    {DPS_LITERAL|DPS_INT, 0, 0, 4},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* movetoProc */
    {DPS_EXEC|DPS_ARRAY, 0, 41, 1536},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* linetoProc */
    {DPS_EXEC|DPS_ARRAY, 0, 36, 1248},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* curvetoProc */
    {DPS_EXEC|DPS_ARRAY, 0, 68, 704},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* closepathProc */
    {DPS_EXEC|DPS_ARRAY, 0, 23, 520},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 51},	/* def */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* ops */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* closepathExecuted */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pathPoints */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* nPaths */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* nPaths */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nPaths */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 8},
    {DPS_LITERAL|DPS_INT, 0, 0, -1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 135},	/* roll */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_INT, 0, 0, 7},
    {DPS_LITERAL|DPS_INT, 0, 0, -1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 135},	/* roll */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 2},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_INT, 0, 0, 6},
    {DPS_LITERAL|DPS_INT, 0, 0, -1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 135},	/* roll */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 3},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_INT, 0, 0, 5},
    {DPS_LITERAL|DPS_INT, 0, 0, -1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 135},	/* roll */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 4},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_INT, 0, 0, 4},
    {DPS_LITERAL|DPS_INT, 0, 0, -1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 135},	/* roll */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 5},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_INT, 0, 0, 3},
    {DPS_LITERAL|DPS_INT, 0, 0, -1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 135},	/* roll */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 6},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* ops */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* curvetoExecuted */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_LITERAL|DPS_INT, 0, 0, 3},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 4},
    {DPS_LITERAL|DPS_INT, 0, 0, -1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 135},	/* roll */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_INT, 0, 0, 3},
    {DPS_LITERAL|DPS_INT, 0, 0, -1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 135},	/* roll */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 2},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* ops */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* linetoExecuted */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 109},	/* ne */
    {DPS_EXEC|DPS_ARRAY, 0, 1, 1864},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 84},	/* if */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 4},
    {DPS_LITERAL|DPS_INT, 0, 0, -1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 135},	/* roll */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_INT, 0, 0, 3},
    {DPS_LITERAL|DPS_INT, 0, 0, -1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 135},	/* roll */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 2},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* ops */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* movetoExecuted */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 1},	/* add */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 62},	/* exch */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* closepathProc */
    }; /* _dpsQ */
  _dpsQ _dpsF;	/* local copy  */
  register DPSContext _dpsCurCtxt = DPSPrivCurrentContext();
  register DPSBinObjRec *_dpsP = (DPSBinObjRec *)&_dpsF.obj0;
  static int _dpsCodes[83] = {-1};
  {
if (_dpsCodes[0] < 0) {
    static const char * const _dps_names[] = {
	"pointSize",
	(char *) 0 ,
	"points",
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	"opSize",
	(char *) 0 ,
	"ops",
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	"pathPointSize",
	(char *) 0 ,
	"pathPoints",
	(char *) 0 ,
	"nPoints",
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	"nPaths",
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	"pointIndex",
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	"opIndex",
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	(char *) 0 ,
	"singleByte",
	"doubleByte",
	"movetoExecuted",
	(char *) 0 ,
	"linetoExecuted",
	(char *) 0 ,
	"curvetoExecuted",
	(char *) 0 ,
	"closepathExecuted",
	(char *) 0 ,
	"movetoProc",
	"linetoProc",
	"curvetoProc",
	"closepathProc",
	(char *) 0 };
    int *_dps_nameVals[83];
    _dps_nameVals[0] = &_dpsCodes[0];
    _dps_nameVals[1] = &_dpsCodes[1];
    _dps_nameVals[2] = &_dpsCodes[2];
    _dps_nameVals[3] = &_dpsCodes[3];
    _dps_nameVals[4] = &_dpsCodes[4];
    _dps_nameVals[5] = &_dpsCodes[5];
    _dps_nameVals[6] = &_dpsCodes[6];
    _dps_nameVals[7] = &_dpsCodes[7];
    _dps_nameVals[8] = &_dpsCodes[8];
    _dps_nameVals[9] = &_dpsCodes[9];
    _dps_nameVals[10] = &_dpsCodes[10];
    _dps_nameVals[11] = &_dpsCodes[11];
    _dps_nameVals[12] = &_dpsCodes[12];
    _dps_nameVals[13] = &_dpsCodes[13];
    _dps_nameVals[14] = &_dpsCodes[14];
    _dps_nameVals[15] = &_dpsCodes[15];
    _dps_nameVals[16] = &_dpsCodes[16];
    _dps_nameVals[17] = &_dpsCodes[17];
    _dps_nameVals[18] = &_dpsCodes[18];
    _dps_nameVals[19] = &_dpsCodes[19];
    _dps_nameVals[20] = &_dpsCodes[20];
    _dps_nameVals[21] = &_dpsCodes[21];
    _dps_nameVals[22] = &_dpsCodes[22];
    _dps_nameVals[23] = &_dpsCodes[23];
    _dps_nameVals[24] = &_dpsCodes[24];
    _dps_nameVals[25] = &_dpsCodes[25];
    _dps_nameVals[26] = &_dpsCodes[26];
    _dps_nameVals[27] = &_dpsCodes[27];
    _dps_nameVals[28] = &_dpsCodes[28];
    _dps_nameVals[29] = &_dpsCodes[29];
    _dps_nameVals[30] = &_dpsCodes[30];
    _dps_nameVals[31] = &_dpsCodes[31];
    _dps_nameVals[32] = &_dpsCodes[32];
    _dps_nameVals[33] = &_dpsCodes[33];
    _dps_nameVals[34] = &_dpsCodes[34];
    _dps_nameVals[35] = &_dpsCodes[35];
    _dps_nameVals[36] = &_dpsCodes[36];
    _dps_nameVals[37] = &_dpsCodes[37];
    _dps_nameVals[38] = &_dpsCodes[38];
    _dps_nameVals[39] = &_dpsCodes[39];
    _dps_nameVals[40] = &_dpsCodes[40];
    _dps_nameVals[41] = &_dpsCodes[41];
    _dps_nameVals[42] = &_dpsCodes[42];
    _dps_nameVals[43] = &_dpsCodes[43];
    _dps_nameVals[44] = &_dpsCodes[44];
    _dps_nameVals[45] = &_dpsCodes[45];
    _dps_nameVals[46] = &_dpsCodes[46];
    _dps_nameVals[47] = &_dpsCodes[47];
    _dps_nameVals[48] = &_dpsCodes[48];
    _dps_nameVals[49] = &_dpsCodes[49];
    _dps_nameVals[50] = &_dpsCodes[50];
    _dps_nameVals[51] = &_dpsCodes[51];
    _dps_nameVals[52] = &_dpsCodes[52];
    _dps_nameVals[53] = &_dpsCodes[53];
    _dps_nameVals[54] = &_dpsCodes[54];
    _dps_nameVals[55] = &_dpsCodes[55];
    _dps_nameVals[56] = &_dpsCodes[56];
    _dps_nameVals[57] = &_dpsCodes[57];
    _dps_nameVals[58] = &_dpsCodes[58];
    _dps_nameVals[59] = &_dpsCodes[59];
    _dps_nameVals[60] = &_dpsCodes[60];
    _dps_nameVals[61] = &_dpsCodes[61];
    _dps_nameVals[62] = &_dpsCodes[62];
    _dps_nameVals[63] = &_dpsCodes[63];
    _dps_nameVals[64] = &_dpsCodes[64];
    _dps_nameVals[65] = &_dpsCodes[65];
    _dps_nameVals[66] = &_dpsCodes[66];
    _dps_nameVals[67] = &_dpsCodes[67];
    _dps_nameVals[68] = &_dpsCodes[68];
    _dps_nameVals[69] = &_dpsCodes[69];
    _dps_nameVals[70] = &_dpsCodes[70];
    _dps_nameVals[71] = &_dpsCodes[71];
    _dps_nameVals[72] = &_dpsCodes[72];
    _dps_nameVals[73] = &_dpsCodes[73];
    _dps_nameVals[74] = &_dpsCodes[74];
    _dps_nameVals[75] = &_dpsCodes[75];
    _dps_nameVals[76] = &_dpsCodes[76];
    _dps_nameVals[77] = &_dpsCodes[77];
    _dps_nameVals[78] = &_dpsCodes[78];
    _dps_nameVals[79] = &_dpsCodes[79];
    _dps_nameVals[80] = &_dpsCodes[80];
    _dps_nameVals[81] = &_dpsCodes[81];
    _dps_nameVals[82] = &_dpsCodes[82];

    DPSMapNames(_dpsCurCtxt, 83, (char **) _dps_names, _dps_nameVals);
    }
  }

  _dpsF = _dpsStat;	/* assign automatic variable */

  _dpsP[0].val.nameVal = _dpsCodes[0];
  _dpsP[4].val.nameVal = _dpsCodes[1];
  _dpsP[3].val.nameVal = _dpsCodes[2];
  _dpsP[203].val.nameVal = _dpsCodes[3];
  _dpsP[197].val.nameVal = _dpsCodes[4];
  _dpsP[162].val.nameVal = _dpsCodes[5];
  _dpsP[156].val.nameVal = _dpsCodes[6];
  _dpsP[126].val.nameVal = _dpsCodes[7];
  _dpsP[118].val.nameVal = _dpsCodes[8];
  _dpsP[110].val.nameVal = _dpsCodes[9];
  _dpsP[102].val.nameVal = _dpsCodes[10];
  _dpsP[94].val.nameVal = _dpsCodes[11];
  _dpsP[88].val.nameVal = _dpsCodes[12];
  _dpsP[7].val.nameVal = _dpsCodes[13];
  _dpsP[11].val.nameVal = _dpsCodes[14];
  _dpsP[10].val.nameVal = _dpsCodes[15];
  _dpsP[217].val.nameVal = _dpsCodes[16];
  _dpsP[176].val.nameVal = _dpsCodes[17];
  _dpsP[140].val.nameVal = _dpsCodes[18];
  _dpsP[65].val.nameVal = _dpsCodes[19];
  _dpsP[14].val.nameVal = _dpsCodes[20];
  _dpsP[18].val.nameVal = _dpsCodes[21];
  _dpsP[17].val.nameVal = _dpsCodes[22];
  _dpsP[75].val.nameVal = _dpsCodes[23];
  _dpsP[21].val.nameVal = _dpsCodes[24];
  _dpsP[230].val.nameVal = _dpsCodes[25];
  _dpsP[227].val.nameVal = _dpsCodes[26];
  _dpsP[192].val.nameVal = _dpsCodes[27];
  _dpsP[189].val.nameVal = _dpsCodes[28];
  _dpsP[186].val.nameVal = _dpsCodes[29];
  _dpsP[153].val.nameVal = _dpsCodes[30];
  _dpsP[150].val.nameVal = _dpsCodes[31];
  _dpsP[85].val.nameVal = _dpsCodes[32];
  _dpsP[77].val.nameVal = _dpsCodes[33];
  _dpsP[24].val.nameVal = _dpsCodes[34];
  _dpsP[82].val.nameVal = _dpsCodes[35];
  _dpsP[79].val.nameVal = _dpsCodes[36];
  _dpsP[76].val.nameVal = _dpsCodes[37];
  _dpsP[27].val.nameVal = _dpsCodes[38];
  _dpsP[214].val.nameVal = _dpsCodes[39];
  _dpsP[211].val.nameVal = _dpsCodes[40];
  _dpsP[204].val.nameVal = _dpsCodes[41];
  _dpsP[198].val.nameVal = _dpsCodes[42];
  _dpsP[173].val.nameVal = _dpsCodes[43];
  _dpsP[170].val.nameVal = _dpsCodes[44];
  _dpsP[163].val.nameVal = _dpsCodes[45];
  _dpsP[157].val.nameVal = _dpsCodes[46];
  _dpsP[137].val.nameVal = _dpsCodes[47];
  _dpsP[134].val.nameVal = _dpsCodes[48];
  _dpsP[127].val.nameVal = _dpsCodes[49];
  _dpsP[119].val.nameVal = _dpsCodes[50];
  _dpsP[111].val.nameVal = _dpsCodes[51];
  _dpsP[103].val.nameVal = _dpsCodes[52];
  _dpsP[95].val.nameVal = _dpsCodes[53];
  _dpsP[89].val.nameVal = _dpsCodes[54];
  _dpsP[30].val.nameVal = _dpsCodes[55];
  _dpsP[224].val.nameVal = _dpsCodes[56];
  _dpsP[221].val.nameVal = _dpsCodes[57];
  _dpsP[218].val.nameVal = _dpsCodes[58];
  _dpsP[183].val.nameVal = _dpsCodes[59];
  _dpsP[180].val.nameVal = _dpsCodes[60];
  _dpsP[177].val.nameVal = _dpsCodes[61];
  _dpsP[147].val.nameVal = _dpsCodes[62];
  _dpsP[144].val.nameVal = _dpsCodes[63];
  _dpsP[141].val.nameVal = _dpsCodes[64];
  _dpsP[72].val.nameVal = _dpsCodes[65];
  _dpsP[69].val.nameVal = _dpsCodes[66];
  _dpsP[66].val.nameVal = _dpsCodes[67];
  _dpsP[33].val.nameVal = _dpsCodes[68];
  _dpsP[37].val.nameVal = _dpsCodes[69];
  _dpsP[41].val.nameVal = _dpsCodes[70];
  _dpsP[219].val.nameVal = _dpsCodes[71];
  _dpsP[44].val.nameVal = _dpsCodes[72];
  _dpsP[178].val.nameVal = _dpsCodes[73];
  _dpsP[47].val.nameVal = _dpsCodes[74];
  _dpsP[142].val.nameVal = _dpsCodes[75];
  _dpsP[50].val.nameVal = _dpsCodes[76];
  _dpsP[67].val.nameVal = _dpsCodes[77];
  _dpsP[53].val.nameVal = _dpsCodes[78];
  _dpsP[56].val.nameVal = _dpsCodes[79];
  _dpsP[59].val.nameVal = _dpsCodes[80];
  _dpsP[62].val.nameVal = _dpsCodes[81];
  _dpsP[233].val.nameVal = _dpsCodes[82];
  DPSBinObjSeqWrite(_dpsCurCtxt,(char *) &_dpsF,1876);
  DPSSYNCHOOK(_dpsCurCtxt)
}
#line 76 "nrOutlines.psw"

#line 776 "nrOutlines.c"
void CheckForFont(const char *fontname, short int *found)
{
  typedef struct {
    unsigned char tokenType;
    unsigned char sizeFlag;
    unsigned short topLevelCount;
    unsigned int nBytes;

    DPSBinObjGeneric obj0;
    DPSBinObjGeneric obj1;
    DPSBinObjGeneric obj2;
    DPSBinObjGeneric obj3;
    DPSBinObjGeneric obj4;
    DPSBinObjGeneric obj5;
    DPSBinObjGeneric obj6;
    DPSBinObjGeneric obj7;
    DPSBinObjGeneric obj8;
    DPSBinObjGeneric obj9;
    DPSBinObjGeneric obj10;
    DPSBinObjGeneric obj11;
    DPSBinObjGeneric obj12;
    DPSBinObjGeneric obj13;
    DPSBinObjGeneric obj14;
    DPSBinObjGeneric obj15;
    DPSBinObjGeneric obj16;
    DPSBinObjGeneric obj17;
    DPSBinObjGeneric obj18;
    DPSBinObjGeneric obj19;
    } _dpsQ;
  static const _dpsQ _dpsStat = {
    DPS_DEF_TOKENTYPE, 0, 14, 168,
    {DPS_LITERAL|DPS_NAME, 0, 0, 160},	/* param fontname */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 67},	/* findfont */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* FontName */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 75},	/* get */
    {DPS_LITERAL|DPS_STRING, 0, 0, 160},	/* param fontname */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 61},	/* eq */
    {DPS_EXEC|DPS_ARRAY, 0, 3, 136},
    {DPS_EXEC|DPS_ARRAY, 0, 3, 112},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 85},	/* ifelse */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 18},	/* clear */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 70},	/* flush */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    }; /* _dpsQ */
  _dpsQ _dpsF;	/* local copy  */
  register DPSContext _dpsCurCtxt = DPSPrivCurrentContext();
  register DPSBinObjRec *_dpsP = (DPSBinObjRec *)&_dpsF.obj0;
  static int _dpsCodes[1] = {-1};
  register int _dps_offset = 160;
  DPSResultsRec _dpsR[1];
  static const DPSResultsRec _dpsRstat[] = {
    { dps_tShort, -1 },
    };
    _dpsR[0] = _dpsRstat[0];
    _dpsR[0].value = (char *)found;

  {
if (_dpsCodes[0] < 0) {
    static const char * const _dps_names[] = {
	"FontName"};
    int *_dps_nameVals[1];
    _dps_nameVals[0] = &_dpsCodes[0];

    DPSMapNames(_dpsCurCtxt, 1, (char **) _dps_names, _dps_nameVals);
    }
  }

  _dpsF = _dpsStat;	/* assign automatic variable */

  _dpsP[0].length =
  _dpsP[4].length = strlen(fontname);
  _dpsP[2].val.nameVal = _dpsCodes[0];
  _dpsP[4].val.stringVal = _dps_offset;
  _dps_offset += _dpsP[4].length;
  _dpsP[0].val.stringVal = _dps_offset;
  _dps_offset += _dpsP[0].length;

  _dpsF.nBytes = _dps_offset+8;
  DPSSetResultTable(_dpsCurCtxt, _dpsR, 1);
  DPSBinObjSeqWrite(_dpsCurCtxt,(char *) &_dpsF,168);
  DPSWriteStringChars(_dpsCurCtxt, (char *)fontname, _dpsP[4].length);
  DPSWriteStringChars(_dpsCurCtxt, (char *)fontname, _dpsP[0].length);
  DPSAwaitReturnValues(_dpsCurCtxt);
}
#line 86 "nrOutlines.psw"
	
#line 871 "nrOutlines.c"
void DrawOutline(const char *fontname, float size, int upper, int lower, int *numOps, int *numPoints, int *numPaths)
{
  typedef struct {
    unsigned char tokenType;
    unsigned char sizeFlag;
    unsigned short topLevelCount;
    unsigned int nBytes;

    DPSBinObjGeneric obj0;
    DPSBinObjGeneric obj1;
    DPSBinObjGeneric obj2;
    DPSBinObjGeneric obj3;
    DPSBinObjGeneric obj4;
    DPSBinObjGeneric obj5;
    DPSBinObjGeneric obj6;
    DPSBinObjGeneric obj7;
    DPSBinObjGeneric obj8;
    DPSBinObjGeneric obj9;
    DPSBinObjGeneric obj10;
    DPSBinObjGeneric obj11;
    DPSBinObjGeneric obj12;
    DPSBinObjGeneric obj13;
    DPSBinObjGeneric obj14;
    DPSBinObjGeneric obj15;
    DPSBinObjGeneric obj16;
    DPSBinObjGeneric obj17;
    DPSBinObjGeneric obj18;
    DPSBinObjGeneric obj19;
    DPSBinObjGeneric obj20;
    DPSBinObjGeneric obj21;
    DPSBinObjReal obj22;
    DPSBinObjGeneric obj23;
    DPSBinObjGeneric obj24;
    DPSBinObjGeneric obj25;
    DPSBinObjGeneric obj26;
    DPSBinObjGeneric obj27;
    DPSBinObjGeneric obj28;
    DPSBinObjGeneric obj29;
    DPSBinObjGeneric obj30;
    DPSBinObjGeneric obj31;
    DPSBinObjGeneric obj32;
    DPSBinObjGeneric obj33;
    DPSBinObjGeneric obj34;
    DPSBinObjGeneric obj35;
    DPSBinObjGeneric obj36;
    DPSBinObjGeneric obj37;
    DPSBinObjGeneric obj38;
    DPSBinObjGeneric obj39;
    DPSBinObjGeneric obj40;
    DPSBinObjGeneric obj41;
    DPSBinObjGeneric obj42;
    DPSBinObjGeneric obj43;
    DPSBinObjGeneric obj44;
    DPSBinObjGeneric obj45;
    DPSBinObjGeneric obj46;
    DPSBinObjGeneric obj47;
    DPSBinObjGeneric obj48;
    DPSBinObjGeneric obj49;
    DPSBinObjGeneric obj50;
    DPSBinObjGeneric obj51;
    DPSBinObjGeneric obj52;
    DPSBinObjGeneric obj53;
    DPSBinObjGeneric obj54;
    DPSBinObjGeneric obj55;
    DPSBinObjGeneric obj56;
    DPSBinObjGeneric obj57;
    DPSBinObjGeneric obj58;
    DPSBinObjGeneric obj59;
    DPSBinObjGeneric obj60;
    DPSBinObjGeneric obj61;
    DPSBinObjGeneric obj62;
    DPSBinObjGeneric obj63;
    DPSBinObjGeneric obj64;
    DPSBinObjGeneric obj65;
    DPSBinObjGeneric obj66;
    DPSBinObjGeneric obj67;
    DPSBinObjGeneric obj68;
    DPSBinObjGeneric obj69;
    DPSBinObjGeneric obj70;
    DPSBinObjGeneric obj71;
    DPSBinObjGeneric obj72;
    DPSBinObjReal obj73;
    DPSBinObjReal obj74;
    DPSBinObjReal obj75;
    DPSBinObjReal obj76;
    DPSBinObjReal obj77;
    DPSBinObjReal obj78;
    } _dpsQ;
  static const _dpsQ _dpsStat = {
    DPS_DEF_TOKENTYPE, 0, 55, 640,
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nOps */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nPaths */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* nPoints */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_LITERAL|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 164},	/* store */
    {DPS_LITERAL|DPS_ARRAY, 0, 6, 584},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 156},	/* setmatrix */
    {DPS_LITERAL|DPS_NAME, 0, 0, 632},	/* param fontname */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 67},	/* findfont */
    {DPS_LITERAL|DPS_REAL, 0, 0, 0},	/* param: size */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 140},	/* scalefont */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 149},	/* setfont */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 111},	/* newpath */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 107},	/* moveto */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},	/* param: upper */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 80},	/* gt */
    {DPS_EXEC|DPS_ARRAY, 0, 9, 512},
    {DPS_EXEC|DPS_ARRAY, 0, 5, 472},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 85},	/* ifelse */
    {DPS_LITERAL|DPS_BOOL, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 17},	/* charpath */
    {DPS_EXEC|DPS_ARRAY, 0, 1, 464},
    {DPS_EXEC|DPS_ARRAY, 0, 1, 456},
    {DPS_EXEC|DPS_ARRAY, 0, 1, 448},
    {DPS_EXEC|DPS_ARRAY, 0, 1, 440},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 116},	/* pathforall */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* opIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pointIndex */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* nPaths */
    {DPS_LITERAL|DPS_INT, 0, 0, 2},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_LITERAL|DPS_INT, 0, 0, 3},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 70},	/* flush */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* closepathProc */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* curvetoProc */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* linetoProc */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* movetoProc */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* singleByte */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_LITERAL|DPS_INT, 0, 0, 0},	/* param: lower */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* singleByte */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* doubleByte */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_LITERAL|DPS_INT, 0, 0, 0},	/* param: upper */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* doubleByte */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_LITERAL|DPS_INT, 0, 0, 0},	/* param: lower */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 120},	/* put */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* doubleByte */
    {DPS_LITERAL|DPS_REAL, 0, 0, 1.0},
    {DPS_LITERAL|DPS_REAL, 0, 0, 0.0},
    {DPS_LITERAL|DPS_REAL, 0, 0, 0.0},
    {DPS_LITERAL|DPS_REAL, 0, 0, -1.0},
    {DPS_LITERAL|DPS_REAL, 0, 0, 0.0},
    {DPS_LITERAL|DPS_REAL, 0, 0, 0.0},
    }; /* _dpsQ */
  _dpsQ _dpsF;	/* local copy  */
  register DPSContext _dpsCurCtxt = DPSPrivCurrentContext();
  register DPSBinObjRec *_dpsP = (DPSBinObjRec *)&_dpsF.obj0;
  static int _dpsCodes[18] = {-1};
  register int _dps_offset = 632;
  DPSResultsRec _dpsR[3];
  static const DPSResultsRec _dpsRstat[] = {
    { dps_tInt, -1 },
    { dps_tInt, -1 },
    { dps_tInt, -1 },
    };
    _dpsR[0] = _dpsRstat[0];
    _dpsR[0].value = (char *)numOps;
    _dpsR[1] = _dpsRstat[1];
    _dpsR[1].value = (char *)numPoints;
    _dpsR[2] = _dpsRstat[2];
    _dpsR[2].value = (char *)numPaths;

  {
if (_dpsCodes[0] < 0) {
    static const char * const _dps_names[] = {
	"nPoints",
	(char *) 0 ,
	"nOps",
	"nPaths",
	(char *) 0 ,
	"pointIndex",
	(char *) 0 ,
	"opIndex",
	(char *) 0 ,
	"closepathProc",
	"curvetoProc",
	"linetoProc",
	"movetoProc",
	"singleByte",
	(char *) 0 ,
	"doubleByte",
	(char *) 0 ,
	(char *) 0 };
    int *_dps_nameVals[18];
    _dps_nameVals[0] = &_dpsCodes[0];
    _dps_nameVals[1] = &_dpsCodes[1];
    _dps_nameVals[2] = &_dpsCodes[2];
    _dps_nameVals[3] = &_dpsCodes[3];
    _dps_nameVals[4] = &_dpsCodes[4];
    _dps_nameVals[5] = &_dpsCodes[5];
    _dps_nameVals[6] = &_dpsCodes[6];
    _dps_nameVals[7] = &_dpsCodes[7];
    _dps_nameVals[8] = &_dpsCodes[8];
    _dps_nameVals[9] = &_dpsCodes[9];
    _dps_nameVals[10] = &_dpsCodes[10];
    _dps_nameVals[11] = &_dpsCodes[11];
    _dps_nameVals[12] = &_dpsCodes[12];
    _dps_nameVals[13] = &_dpsCodes[13];
    _dps_nameVals[14] = &_dpsCodes[14];
    _dps_nameVals[15] = &_dpsCodes[15];
    _dps_nameVals[16] = &_dpsCodes[16];
    _dps_nameVals[17] = &_dpsCodes[17];

    DPSMapNames(_dpsCurCtxt, 18, (char **) _dps_names, _dps_nameVals);
    }
  }

  _dpsF = _dpsStat;	/* assign automatic variable */

  _dpsP[20].length = strlen(fontname);
  _dpsP[22].val.realVal = size;
  _dpsP[29].val.integerVal =
  _dpsP[66].val.integerVal = upper;
  _dpsP[70].val.integerVal =
  _dpsP[61].val.integerVal = lower;
  _dpsP[0].val.nameVal = _dpsCodes[0];
  _dpsP[9].val.nameVal = _dpsCodes[1];
  _dpsP[3].val.nameVal = _dpsCodes[2];
  _dpsP[6].val.nameVal = _dpsCodes[3];
  _dpsP[48].val.nameVal = _dpsCodes[4];
  _dpsP[12].val.nameVal = _dpsCodes[5];
  _dpsP[45].val.nameVal = _dpsCodes[6];
  _dpsP[15].val.nameVal = _dpsCodes[7];
  _dpsP[42].val.nameVal = _dpsCodes[8];
  _dpsP[55].val.nameVal = _dpsCodes[9];
  _dpsP[56].val.nameVal = _dpsCodes[10];
  _dpsP[57].val.nameVal = _dpsCodes[11];
  _dpsP[58].val.nameVal = _dpsCodes[12];
  _dpsP[59].val.nameVal = _dpsCodes[13];
  _dpsP[63].val.nameVal = _dpsCodes[14];
  _dpsP[64].val.nameVal = _dpsCodes[15];
  _dpsP[72].val.nameVal = _dpsCodes[16];
  _dpsP[68].val.nameVal = _dpsCodes[17];
  _dpsP[20].val.stringVal = _dps_offset;
  _dps_offset += _dpsP[20].length;

  _dpsF.nBytes = _dps_offset+8;
  DPSSetResultTable(_dpsCurCtxt, _dpsR, 3);
  DPSBinObjSeqWrite(_dpsCurCtxt,(char *) &_dpsF,640);
  DPSWriteStringChars(_dpsCurCtxt, (char *)fontname, _dpsP[20].length);
  DPSAwaitReturnValues(_dpsCurCtxt);
}
#line 113 "nrOutlines.psw"

#line 1142 "nrOutlines.c"
void GetOutline(int numOps, int numPoints, int numPaths, int opsArray[], int pointsPerPathArray[], float pointsArray[])
{
  typedef struct {
    unsigned char tokenType;
    unsigned char topLevelCount;
    unsigned short nBytes;

    DPSBinObjGeneric obj0;
    DPSBinObjGeneric obj1;
    DPSBinObjGeneric obj2;
    DPSBinObjGeneric obj3;
    DPSBinObjGeneric obj4;
    DPSBinObjGeneric obj5;
    DPSBinObjGeneric obj6;
    DPSBinObjGeneric obj7;
    DPSBinObjGeneric obj8;
    DPSBinObjGeneric obj9;
    DPSBinObjGeneric obj10;
    DPSBinObjGeneric obj11;
    DPSBinObjGeneric obj12;
    } _dpsQ;
  static const _dpsQ _dpsStat = {
    DPS_DEF_TOKENTYPE, 13, 108,
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* ops */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* pathPoints */
    {DPS_LITERAL|DPS_INT, 0, 0, 1},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    {DPS_EXEC|DPS_NAME, 0, 0, 0},	/* points */
    {DPS_LITERAL|DPS_INT, 0, 0, 2},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    {DPS_LITERAL|DPS_INT, 0, 0, 0},
    {DPS_LITERAL|DPS_INT, 0, 0, 3},
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 119},	/* printobject */
    {DPS_EXEC|DPS_NAME, 0, DPSSYSNAME, 70},	/* flush */
    }; /* _dpsQ */
  _dpsQ _dpsF;	/* local copy  */
  register DPSContext _dpsCurCtxt = DPSPrivCurrentContext();
  register DPSBinObjRec *_dpsP = (DPSBinObjRec *)&_dpsF.obj0;
  static int _dpsCodes[3] = {-1};
  DPSResultsRec _dpsR[3];
  static const DPSResultsRec _dpsRstat[] = {
    { dps_tInt },
    { dps_tInt },
    { dps_tFloat },
    };
    _dpsR[0] = _dpsRstat[0];
    _dpsR[0].count = numOps;
    _dpsR[0].value = (char *)opsArray;
    _dpsR[1] = _dpsRstat[1];
    _dpsR[1].count = numPaths;
    _dpsR[1].value = (char *)pointsPerPathArray;
    _dpsR[2] = _dpsRstat[2];
    _dpsR[2].count = numPoints;
    _dpsR[2].value = (char *)pointsArray;

  {
if (_dpsCodes[0] < 0) {
    static const char * const _dps_names[] = {
	"ops",
	"pathPoints",
	"points"};
    int *_dps_nameVals[3];
    _dps_nameVals[0] = &_dpsCodes[0];
    _dps_nameVals[1] = &_dpsCodes[1];
    _dps_nameVals[2] = &_dpsCodes[2];

    DPSMapNames(_dpsCurCtxt, 3, (char **) _dps_names, _dps_nameVals);
    }
  }

  _dpsF = _dpsStat;	/* assign automatic variable */

  _dpsP[0].val.nameVal = _dpsCodes[0];
  _dpsP[3].val.nameVal = _dpsCodes[1];
  _dpsP[6].val.nameVal = _dpsCodes[2];
  DPSSetResultTable(_dpsCurCtxt, _dpsR, 3);
  DPSBinObjSeqWrite(_dpsCurCtxt,(char *) &_dpsF,108);
  DPSAwaitReturnValues(_dpsCurCtxt);
}
#line 121 "nrOutlines.psw"


#endif /* HAVE_DPS */
