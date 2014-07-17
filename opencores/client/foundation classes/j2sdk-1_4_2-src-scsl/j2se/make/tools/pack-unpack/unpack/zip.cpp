/*
 * @(#)zip.cpp	1.12 03/06/18
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
/**
 * Note: Lifted from uncrunch.c from jdk sources
 */ 
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <stdlib.h>

#ifndef _MSC_VER 
#include <strings.h>
#endif


#include "defines.h"
#include "bytes.h"
#include "utils.h"

#define ushort unsigned short
#define uint   unsigned int
#define uchar  unsigned char

#include "zip.h"

uLong unix2dostime(time_t *t);
uLong updcrc(uchar *s, int n);

#ifdef sparc
#define SWAP_BYTES(a) \
    ((((a) << 8) & 0xff00) | 0x00ff) & (((a) >> 8) | 0xff00)
#else
#define SWAP_BYTES(a)  (a)
#endif

#define GET_INT_LO(a) \
    SWAP_BYTES(a & 0xFFFF)

#define GET_INT_HI(a) \
    SWAP_BYTES((a >> 16) & 0xFFFF);

// Data for ZIP central directory.
uchar central_directory[1024*1024];
uchar *central_directory_limit = central_directory + sizeof(central_directory);
uchar *central_directory_ptr = central_directory;
ushort central_directory_count;
FILE *jarfp;


static uint output_file_offset = 0;

// Write data to the ZIP output stream.
void
write_data(void *buff, int len) 
{
    while (len > 0) {
	int rc = fwrite(buff, 1, len, jarfp);
	if (rc <= 0) {
	    fprintf(errstrm, "Write on output file failed err=%d\n",errno);
	    unpack_abort("ERROR: Write on output failed");
	}
	output_file_offset += rc;
	buff = ((char *)buff) + rc;
	len -= rc;
    }
}

static void
add_to_zip_directory(char *fname, int len, uLong crc, uLong mtime){
    uint fname_length = strlen(fname);
    ushort header[500];

    if ((central_directory_ptr + 400) > central_directory_limit) {
	fprintf(errstrm, "Ran out of ZIP central directory space\n");
	fprintf(errstrm, "after creating %d entries.\n", central_directory_count);
	exit(122);
    }
    
    header[0] = SWAP_BYTES(0x4B50);
    header[1] = SWAP_BYTES(0x0201);
    header[2] = SWAP_BYTES(0xA);
    // required version
    header[3] = SWAP_BYTES(0xA);
    // flags => not compressed
    header[4] = 0;
    // Compression method => none
    header[5] = 0;
    // Last modified date and time.
    header[6] = GET_INT_LO(mtime);
    header[7] = GET_INT_HI(mtime);
    // CRC
    header[8] = GET_INT_LO(crc);
    header[9] = GET_INT_HI(crc);

    // Compressed length:
    header[10] = GET_INT_LO(len);
    header[11] = GET_INT_HI(len);
    // Uncompressed length.  Same as compressed length.,
    header[12] = GET_INT_LO(len);
    header[13] = GET_INT_HI(len);

    // Filename length
    header[14] = SWAP_BYTES(fname_length);
    // So called "extra field" length.
    header[15] = 0;
    // So called "comment" length.
    header[16] = 0;
    // Disk number start
    header[17] = 0;
    // File flags => binary
    header[18] = 0;
    // More file flags
    header[19] = 0;
    header[20] = 0;
    // Offset within ZIP file.
    header[21] = GET_INT_LO(output_file_offset);
    header[22] = GET_INT_HI(output_file_offset);
    // Copy the fname to the header.
    memcpy((char *)(header+23), fname, fname_length);

    // Copy the whole thing into the central directory.
    memcpy((char *)central_directory_ptr, (char *)header, 46+fname_length);
    central_directory_ptr += (46+fname_length);

    central_directory_count++;
}

static void
write_zip_header(char *fname, int len, unsigned long crc, unsigned long mtime) {
    uint fname_length = strlen(fname);
    ushort header[500];

    // ZIP LOC magic.
    header[0] = SWAP_BYTES(0x4B50);
    header[1] = SWAP_BYTES(0x0403);
    
    // Version
    header[2] = SWAP_BYTES(0xA);
    // flags => not compressed
    header[3] = 0;
    // Compression method => none
    header[4] = 0;

    // Last modified date and time.
    header[5] = GET_INT_LO(mtime);
    header[6] = GET_INT_HI(mtime);

    // CRC
    header[7] = GET_INT_LO(crc & 0xFFFF);
    header[8] = GET_INT_HI(crc);
 
    // Compressed length:
    header[9] = GET_INT_LO(len);
    header[10] = GET_INT_HI(len);

    // Uncompressed length.  Same as compressed length.,
    header[11] = GET_INT_LO(len);
    header[12] = GET_INT_HI(len);
    // Filename length
    header[13] = SWAP_BYTES(fname_length);
    // So called "extra field" length.
    header[14] = 0;
    // Copy the fname to the header.
    memcpy((char *)(header+15), fname, fname_length);

    // Write the LOC header to the output file.
    write_data(header, 30 + fname_length);
}

static void
write_central_directory(){
    ushort header[25];
    uint directory_len = central_directory_ptr - central_directory;

    // Create the End of Central Directory structure.
    header[0] = SWAP_BYTES(0x4B50);
    header[1] = SWAP_BYTES(0x0605);
    // disk numbers
    header[2] = 0;
    header[3] = 0;
    // Number of entries in central directory.
    header[4] = SWAP_BYTES(central_directory_count);
    header[5] = SWAP_BYTES(central_directory_count);
    // Size of the central directory}
    header[6] = GET_INT_LO(directory_len);
    header[7] = GET_INT_HI(directory_len);
    // Offset of central directory within disk. 
    header[8] = GET_INT_LO(output_file_offset);
    header[9] = GET_INT_HI(output_file_offset);
    // zipfile comment length;
    header [10] = 0;
    

    // Write the central directory.
    printcr(2, "Central directory at %d\n", output_file_offset);
    write_data(central_directory, directory_len);

    // Write the End of Central Directory structure.
    printcr(2, "end-of-directory at %d\n", output_file_offset);
    write_data(header, 22);
}

// Public API

//Open a Jar file and initialize.
void
openJarFile(char *fname) {
    if (!jarfp) {
    	jarfp = fopen(fname, "wb");
    	if (!jarfp) {
    	    perror("fopen");
    	    unpack_abort("ERROR: Could not open jar file");
    	}
    }
}

// Add a ZIP entry and copy the file data
void
addJarEntry(char *fname, int nsegs, bytes* segs) {
    int len = 0;
    uint c = updcrc((uchar *)NULL,0); //Initialize it

    time_t now = 0L;
#ifndef _MSC_VER
    struct tm btm;
    memset((void *)&btm,0,sizeof(btm));
    static const char* tm_data[] =  { BUILD_DATE };
    btm.tm_sec  = atoi(tm_data[5]) - 1;
    btm.tm_min  = atoi(tm_data[4]) - 1;
    btm.tm_hour = atoi(tm_data[3]) - 1;
    btm.tm_mday = atoi(tm_data[2]);
    btm.tm_mon  = atoi(tm_data[1]) - 1;
    btm.tm_year = 100 + atoi(tm_data[0]);
    now = mktime((struct tm *)&btm);
#endif
    
    assert(sizeof(uLong) == 4);

    int i = 0;
    for (i = 0; i < nsegs; i++) {
        len += segs[i].len;
        c = updcrc((uchar *)segs[i].ptr, segs[i].len);
    }

    add_to_zip_directory(fname, len, c, unix2dostime(&now));
    write_zip_header(fname, len, c, unix2dostime(&now));
    for (i = 0; i < nsegs; i++) {
        write_data(segs[i].ptr, segs[i].len);
    }
}

// Add a ZIP entry for a directory name no data
void
addDirectoryToJarFile(char *dir_name) {
    add_to_zip_directory((char *)dir_name, 0, 0, 0);
    write_zip_header((char *)dir_name, 0, 0, 0);
}

// Write out the central directory and close the jar file.
void closeJarFile(bool central) {
    if (jarfp) {
    	fflush(jarfp);
	if (central) write_central_directory();
	fflush(jarfp);
	fclose(jarfp);
	jarfp=null;
    }
}

//Write input contents of bytes to the jar file previously opened
void write2JarFile(bytes b) {
    write_data(b.ptr, b.len);
}


// The code fragments below are copied from 
// j2se/src/share/native/java/util/zip//zlib-1.1.3/zcrc32.c

/* Table of CRC-32's of all single byte values (made by makecrc.c) */
static uLong crctab[] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

/* Return the CRC-32 c updated with the eight bits in b. */
uLong crc32(uLong c, int b)
{
  return crctab[((int)c ^ b) & 0xff] ^ (c >> 8);
}

/* Run a set of bytes through the crc shift register.  If s is a NULL
   pointer, then initialize the crc shift register contents instead.
   Return the current crc in either case. */
uLong updcrc(uchar *s, int n) {
  register uLong c;       /* temporary variable */

  static uLong crc = 0xffffffffL; /* shift register contents */

  if (s == NULL) {
    c = 0xffffffffL;
  } else {
    c = crc;
    while (n--)
      c = crctab[((int)c ^ (*s++)) & 0xff] ^ (c >> 8);
  }
  crc = c;
  return c ^ 0xffffffffL;       /* (instead of ~c for 64-bit machines) */
}

/* Convert the date y/n/d and time h:m:s to a four byte DOS date and
   time (date in high two bytes, time in low two bytes allowing magnitude
   comparison). */

uLong dostime(int y, int n, int d, int h, int m, int s) {
  return y < 1980 ? dostime(1980, 1, 1, 0, 0, 0) :
        (((uLong)y - 1980) << 25) | ((uLong)n << 21) | ((uLong)d << 16) |
        ((uLong)h << 11) | ((uLong)m << 5) | ((uLong)s >> 1);
}


/* Return the Unix time t in DOS format, rounded up to the next two
   second boundary. */
uLong unix2dostime(time_t *t) {
  struct tm *s;         /* result of localtime() */

  s = localtime(t);
  return dostime(s->tm_year + 1900, s->tm_mon + 1, s->tm_mday,
                 s->tm_hour, s->tm_min, s->tm_sec);
}
