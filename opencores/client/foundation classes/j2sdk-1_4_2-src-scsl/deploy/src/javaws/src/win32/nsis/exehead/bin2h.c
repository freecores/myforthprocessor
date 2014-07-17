/*
 * Nullsoft "SuperPimp" Installation System 
 * version 1.0j - November 12th 2000
 *
 * Copyright (C) 1999-2000 Nullsoft, Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Justin Frankel
 * justin@nullsoft.com
 *
 * This source distribution includes portions of zlib. see zlib/zlib.h for
 * its license and so forth. Note that this license is also borrowed from zlib.
 */

/*
 * @(#)bin2h.c	1.2 02/01/24
 */


#include <stdio.h>

int main(int argc, char *argv[]) {
  FILE *in, *out;
  char *outfilename;
  char *token;
  int total_bytes=0;

  if (argc != 4) { 
    fprintf(stderr,"Usage: bin2s file.dat outfile.h token_name\n");
    return 1;
  }

  in = fopen(argv[1],"rb");

  if (!in) {
    fprintf(stderr,"Error: file not found\n");
    return 1;
  }
  out = fopen(argv[2],"wt");

  if (!out) {
    fclose(in);
    fprintf(stderr,"Error: could not open output file\n");
    return 1;
  }

  outfilename = argv[2];
  token = argv[3];
  fprintf(out,"unsigned char %s[] = {  \n",token);
  for (;;) {
    static int firsttime;
    static int linecount;
    int c;
    if (++linecount > 10) {
      linecount = 0;
      fprintf(out,",\n  ");
    }
    else if (firsttime) fprintf(out,", ");
    firsttime = 1;
    c = fgetc(in);
    if (feof(in)) break;
    total_bytes++;
    fprintf(out,"%i",c);
  }
  fprintf(out,"};\n",token,total_bytes);
  fclose(in);
  fclose(out);
  fprintf(stderr,"Success (%d bytes)!\n\n",total_bytes);
  return 0;
}
