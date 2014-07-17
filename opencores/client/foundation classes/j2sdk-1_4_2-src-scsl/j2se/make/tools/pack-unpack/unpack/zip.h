/*
 * @(#)zip.h	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

void openJarFile(char *fname);
void addJarEntry(char *fname, int nsegs, bytes* segs);
void addDirectoryToJarFile(char *dir_name);
void closeJarFile(bool central);
void write2JarFile(bytes b);
