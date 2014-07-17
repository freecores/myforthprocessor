/*
 * @(#)main.c	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Program to read a crunched JAR file and regenerate a normal JAR.
 *
 * The crunched format has a set of global constant pools.  We have
 * to regnerate the private constant pools for each classfile and
 * replace all the global connsta pool indexes with private constant
 * pool indexes.
 *
 *							KGH Nov 98
 */

#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <malloc.h>
#include <string.h>
#include "crunch.h"

// The file descriptor for the input file:
int input_fd;

// The file descriptor for the output file:
int output_fd;

static void
bcopy (uchar *src, uchar *dst, uint len)
{
    while (len--) {
	*dst++ = *src++;
    }
}

void
syntax()
{
    fprintf(stderr, "Syntax is:\n");
    fprintf(stderr, "        uncrunch <src> <dst>\n");
    fprintf(stderr, "    or\n");
    fprintf(stderr, "        uncrunch \"<src>\" \"<dst>\"\n");
    fprintf(stderr, "If you double-quote one name, you have to double quote both.");
    exit(3);
}

void
read_fully(void *buff, int len) 
{
    int rc;
    while (len > 0) {
	int rc = _read(input_fd, buff, len);
	if (rc <= 0) {
	    int err = GetLastError();
	    fprintf(stderr, "Read on input file failed, error=%d", err);
	    exit(4);
	}
	buff = ((char *)buff) + rc;
	len -= rc;
    }
}

uint output_file_offset = 0;

// Write data to the ZIP output stream.
void
write_data(void *buff, int len) 
{
    int rc;
    while (len > 0) {
	int rc = _write(output_fd, buff, len);
	if (rc <= 0) {
	    int err = GetLastError();
	    fprintf(stderr, "Write on output file failed, error=%d\n", err);
	    if (err == ERROR_DISK_FULL) {
	        fprintf(stderr, "Insufficient disk space\n");
	    }
	    exit(5);
	}
	output_file_offset += rc;
	buff = ((char *)buff) + rc;
	len -= rc;
    }
}

ushort
read_swapped_short()
{
    uchar raw[2];
    ushort cooked;
    read_fully(&raw, 2);
    cooked = (raw[0] << 8) | raw[1];
    return cooked;
}

uint
read_swapped_int()
{
    uchar raw[4];
    uint cooked;
    read_fully(&raw, 4);
    cooked = (raw[0] << 24) | (raw[1] << 16) | (raw[2] << 8) | raw[3];
    return cooked;
}

// Read a Java UTF8 string that is known to contaon only ASCII.
// Result is a null-terminated ASCII string.
// Limit is 1 KByte.
uchar *
read_utf_ascii(uchar *buff, int max)
{
    int i;
    uchar len = read_swapped_short();
    if (len >= max) {
	fprintf(stderr, "read_utf_ascii length exceeded");
	exit(10);
    }
    read_fully(buff, len);
    for (i= 0; i < len; i++) {
	if (buff[i] & 0x80) {
	    fprintf(stderr, "read_utf_ascii on non-ascii string\n");
	    exit(9);
	}
    }
    buff[len] = 0;
    return buff;
}

ushort
get_swapped_short(uchar *raw)
{
    ushort cooked;
    cooked = (raw[0] << 8) | raw[1];
    return cooked;
}

void
set_swapped_short(uchar *raw, ushort value)
{
    raw[0] = (value  >> 8) & 0xFF;
    raw[1] = value & 0xFF;
}

uint
get_swapped_int(uchar *raw)
{
    uint cooked;
    cooked = (raw[0] << 24) | (raw[1] << 16) | (raw[2] << 8) | raw[3];
    return cooked;
}

void
set_swapped_int(uchar *raw, uint value)
{ 
    raw[0] = (value  >> 24) & 0xFF;
    raw[1] = (value  >> 16) & 0xFF;
    raw[2] = (value  >> 8) & 0xFF;
    raw[3] = value & 0xFF;
}

// Data for ZIP central directory.
uchar central_directory[700000];
uchar *central_directory_limit = central_directory + sizeof(central_directory);
uchar *central_directory_ptr = central_directory;
ushort central_directory_count;

void
add_to_zip_directory(uchar *fname, int len)
{
    uint fname_length = strlen(fname);
    ushort header[500];

    if ((central_directory_ptr + 400) > central_directory_limit) {
	fprintf(stderr, "Ran out of ZIP central directory space\n");
	fprintf(stderr, "after creating %d entries.\n", central_directory_count);
	exit(122);
    }
    
    // ZIP central directory magic.
    header[0] = 0x4B50;
    header[1] = 0x0201;
    header[2] = 0xA;
    // required version
    header[3] = 0xA;
    // flags => not compressed
    header[4] = 0;
    // Compression method => none
    header[5] = 0;
    // Last modified date and time.
    header[6] = 0;
    header[7] = 0;
    // CRC
    header[8] = 0;
    header[9] = 0;
    // Compressed length:
    header[10] = len & 0xFFFF;
    header[11] = (len >> 16) & 0xFFFF;
    // Uncompressed length.  Same as compressed length.,
    header[12] = len & 0xFFFF;
    header[13] = (len >> 16) & 0xFFFF;
    // Filename length
    header[14] = fname_length;
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
    header[21] = output_file_offset & 0xFFFF;
    header[22] = (output_file_offset >> 16) & 0xFFFF;
    // Copy the fname to the header.
    bcopy(fname, (char *)(header+23), fname_length);

    // Copy the whole thing into the central directory.
    bcopy((uchar *)header, central_directory_ptr, 46+fname_length);
    central_directory_ptr += (46+fname_length);

    central_directory_count++;
}

void
write_zip_header(uchar *fname, int len) {
    uint fname_length = strlen(fname);
    ushort header[500];

    // ZIP LOC magic.
    header[0] = 0x4B50;
    header[1] = 0x0403;
    // Version
    header[2] = 0xA;
    // flags => not compressed
    header[3] = 0;
    // Compression method => none
    header[4] = 0;
    // Last modified date and time.
    header[5] = 0;
    header[6] = 0;
    // CRC
    header[7] = 0;
    header[8] = 0;
    // Compressed length:
    header[9] = len & 0xFFFF;
    header[10] = (len >> 16) & 0xFFFF;
    // Uncompressed length.  Same as compressed length.,
    header[11] = len & 0xFFFF;
    header[12] = (len >> 16) & 0xFFFF;
    // Filename length
    header[13] = fname_length;
    // So called "extra field" length.
    header[14] = 0;
    // Copy the fname to the header.
    bcopy(fname, (char *)(header+15), fname_length);

    // Write the LOC header to the output file.
    write_data(header, 30 + fname_length);
}

void
write_central_directory()
{
    ushort header[25];
    uint directory_len = central_directory_ptr - central_directory;

     // Create the End of Central Directory structure.
    header[0] = 0x4B50;
    header[1] = 0x0605;
    // disk numbers
    header[2] = 0;
    header[3] = 0;
    // Number of entries in central directory.
    header[4] = central_directory_count;
    header[5] = central_directory_count;
    // Size of the central directory}
    header[6] = directory_len & 0xFFFF;
    header[7] = (directory_len >> 16) & 0xFFFF;
    // Offset of central directory within disk. 
    header[8] = output_file_offset & 0xFFFF; 
    header[9] = (output_file_offset >> 16) & 0xFFFF; 
    // zipfile comment length;
    header [10] = 0;

    // Write the central directory.
    // printf("Central directory at %d\n", output_file_offset);
    write_data(central_directory, directory_len);

    // Write the End of Central Directory structure.
    // printf("end-of-directory at %d\n", output_file_offset);
    write_data(header, 22);
}

// Add a ZIP entry for a directory name
static void
write_directory_name(char *dir_name)
{
    add_to_zip_directory(dir_name, 0);
    write_zip_header(dir_name, 0);
}


static uchar header[] = { 0xCA, 0xFE, 0xBA, 0xBE, 0, 3, 0, 45 };
static uchar input_buffer[256 * 1024];
static uchar output_buffer[256 * 1024];

void
process_classfile(uchar *fname, uint input_length)
{
    uint output_length;
    uint total_length;

    if (input_length >= sizeof input_buffer) {
	fprintf(stderr, "classfile too big (%d)\n", input_length);
	exit(16);
    }
    read_fully(input_buffer, input_length);

    reset_local_pool();
    prescan(input_buffer, input_length);
    fixup_prescan_values();

    output_length = convert(input_buffer, output_buffer, input_length);

    total_length = 8 + get_local_pool_length() + output_length;

    add_to_zip_directory(fname, total_length);
    write_zip_header(fname, total_length);

    write_data(header, 8);
    write_local_pool();
    write_data(output_buffer, output_length);

}

void
process_data_file(uchar *fname, uint length)
{
    uint done = 0;

    while (length > done) {
	int rc = _read(input_fd, output_buffer+done, length);
	if (rc <= 0) {
	    fprintf(stderr, "Couldn't read data file\n");
	    exit(12);
	}
	done += rc;
    }

    add_to_zip_directory(fname, length);
    write_zip_header(fname, length);

    write_data(output_buffer, length);
}

static char dir_name[1024];

static void
read_file_name(uchar *fname)
{
    uchar rawlen;
    uchar buff[257];
    uchar *buffp, *dirp;

    read_fully(&rawlen, 1);

    read_fully(buff, rawlen);
    buff[rawlen] = 0;

    buffp = buff;
    // If the current filename starts with "../" back-up our idea
    // of the current directory.
    while (buffp[0] == '.' && buffp[1] == '.' && buffp[2] == '/') {
	buffp += 3;
	for (dirp = (dir_name + strlen(dir_name) - 2); 
		(dirp >= dir_name && *dirp != '/');
		dirp--) {
	}
	dirp[1] = 0;
    }

    // Append any new directory to dir_name.
    while (strchr(buffp, '/')) {
	uchar *cp = strchr(buffp, '/');
	*cp = 0;
	strcat(dir_name, buffp);
	strcat(dir_name, "/");
	write_directory_name(dir_name);
	buffp = cp + 1;
    }

    fname[0] = 0;
    strcat(fname, dir_name);
    strcat(fname, buffp);

    // printf("fname == \"%s\"\n", fname);
}

static uchar log_file_name[MAX_PATH];

// Redirect all stdout and stderr mesages to a log file.
static void
redirect_stdio() {
    uchar *tname;

    tname = getenv("TEMP");
    if (tname) {
	sprintf(log_file_name, "%s\\uncrunch.log", tname);
    } else {
	sprintf(log_file_name, "C:\\uncrunch.log");
    }

    freopen(log_file_name, "w+", stderr);
    _dup2(2,1);
    setbuf(stderr, 0);
    setbuf(stdout, 0);
}

#define OUR_CLASS_NAME "kgh-uncrunch"

HINSTANCE ourInstance;


// This function is called to process window messages
LRESULT CALLBACK window_proc(
	HWND window,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
    // fprintf(stderr, "window_proc: %d\n", msg);
}

// This function runs as a separate thread to manage our
// window state.
// We crate an invisible window simply so that InstallShield
// can detect when we exit.  Sigh.
DWORD WINAPI
window_thread(LPVOID arg) 
{
    WNDCLASSEX wex;
    ATOM our_class;
    HWND window;
    MSG msg;
    int rc;

    // fprintf(stderr, "Window thread started\n");

    // Initialize our window state.
    wex.cbSize = sizeof(wex);
    wex.style = CS_NOCLOSE;
    wex.lpfnWndProc = window_proc;
    wex.cbClsExtra = 0;
    wex.cbWndExtra = 0;
    wex.hInstance = ourInstance;
    wex.hIcon = NULL;
    wex.hCursor = NULL;
    wex.hbrBackground = NULL;
    wex.lpszMenuName = NULL;
    wex.lpszClassName = OUR_CLASS_NAME;
    wex.hIconSm = NULL;

    our_class = RegisterClassEx(&wex);
    
    if (our_class == 0) {
	fprintf(stderr, "RegisterClassEx error %d\n", GetLastError());
    }

    window = CreateWindow(OUR_CLASS_NAME,
		"Uncrunch.exe",
		WS_DLGFRAME,
		400, 300,	// Position
		300, 200,	//Sixe
		NULL,
		NULL,
		ourInstance,
		0);

    if (window == 0) {
	fprintf(stderr, "CreatwWindow error %d\n", GetLastError());
    }

    // Run the window message loop.
    while (GetMessage(&msg, NULL, 0, 0)) { 
        DispatchMessage(&msg);
    } 
}

// Main entry point.
int APIENTRY
WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
    int i, rc, magic, fcount, flength, version, wthid;
    HANDLE wthd;
    uchar fname[1024];
    uchar src[1024];
    uchar dst[1024];
    uchar tfile[1024];

    ourInstance = hInstance;

    redirect_stdio();

    fprintf(stderr, "Running uncrunch\n");
    fprintf(stderr, "lpCmdLine = %s\n", lpCmdLine);

    if (lpCmdLine[0] == '"') {
	// The line starts with a ".  We require that both filenames be quoted.
        // Our InstallShield script invokes us this way because it has to
        // be avle to use diretcories with spaces in their names.
	uchar *line = lpCmdLine+1;
	uchar *qp = strchr(line, '"');
	if (qp == 0) {
	   syntax();
	}
	*qp = 0;
	strcpy(src, line);
	line = qp+1;
	qp = strchr(line, '"');
	if (qp == 0) {
	   syntax();
	}
	line = qp+1;
	qp = strchr(line, '"');
	if (qp == 0) {
	   syntax();
	}
	*qp = 0;
	strcpy(dst, line);
    } else {
	rc = sscanf(lpCmdLine, "%s %s", src, dst);
	if (rc != 2) {
	    syntax();
	}
    }

    fprintf(stderr, "Crunching from %s to %s\n", src, dst);

    // We initially write our output to a temporary file.
    strcpy(tfile, dst);
    tfile[strlen(tfile)-4] = 0;
    strcat(tfile, ".tmp");
    fprintf(stderr, "Temporary file = %s\n", tfile);

    // Create a thread to manage our window state.
    wthd = CreateThread(NULL, 0, window_thread, 0, 0, &wthid);
    // fprintf(stderr, "window thread id == %d\n", wthid);

    input_fd = _open(src, _O_RDONLY|_O_BINARY, 0);
    if (input_fd <= 0) {
	fprintf(stderr,"Couldn't open input file \"%s\"\n", src);
	exit(1);
    }

    output_fd = _open(tfile, _O_WRONLY|_O_BINARY|_O_CREAT|_O_TRUNC, _S_IREAD|_S_IWRITE);
    if (output_fd <= 0) {
	fprintf(stderr, "Couldn't open output file \"%s\"\n", tfile);
	exit(11);
    }

    magic = read_swapped_int();
    if (magic != CRUNCH_MAGIC) {
	fprintf(stderr, "Input file isn't in java crunched format\n");
	exit(1);
    }

    version = read_swapped_int();
    if (version != CRUNCH_VERSION) {
	fprintf(stderr, "Input file has wrong crunch version\n");
	exit(2);
    }

    fcount = read_swapped_int();    

    // Read the global constant pools.
    read_global_pools();

    // Process each of the input files.
    for (i = 0; i < fcount; i++) {

	flength = read_swapped_int();

	read_file_name(fname);

	// printf("File \"%s\" length %d at %d\n", fname, flength, output_file_offset);

	if (flength >= 0) {
	    strcat(fname, ".class");
	    process_classfile(fname, flength);
	} else {
	    process_data_file(fname, -flength);
	}
    }

    write_central_directory();

    _close(output_fd);

    // OK, we seem to have processed everything without problems.
    // Rename the temporarty file to the output file.
    unlink(dst);
    rc = rename(tfile, dst);
    if (rc != 0) {
	fprintf(stderr, "File rename failed");
	exit(9);
    } 

    fprintf(stderr, "Uncrunch completed OK.\n");
    fflush(stderr);
    fclose(stderr);
    fclose(stdout);

    // Since everything was successful, delete our log file.
    unlink(log_file_name);

    exit(0);
}
