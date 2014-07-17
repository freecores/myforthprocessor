/*
 * @(#)nrOutlines.h	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/* nrOutlines.h generated from nrOutlines.psw
   by unix pswrap V1.009  Wed Apr 19 17:50:24 PDT 1989
 */

#ifndef NROUTLINES_H
#define NROUTLINES_H

#if  defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern void OutlineInit( void );

extern void CheckForFont(const char *fontname, short int *found);

extern void DrawOutline(const char *fontname, float size, int upper, int lower, int *numOps, int *numPoints, int *numPaths);

extern void GetOutline(int numOps, int numPoints, int numPaths, int opsArray[], int pointsPerPathArray[], float pointsArray[]);

#if  defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* NROUTLINES_H */
