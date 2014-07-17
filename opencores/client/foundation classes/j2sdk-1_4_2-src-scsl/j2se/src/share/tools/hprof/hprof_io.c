/*
 * @(#)hprof_io.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>
#include "hprof.h"

/* Output buffer */
#define WRITE_BUF_SIZE 2048
static int buf_index = 0;
static char *write_buf;


static char *cur_ptr_in_dump;

void hprof_dump_seek(char *ptr)
{
    cur_ptr_in_dump = ptr;
}

char * hprof_dump_cur(void)
{
    return cur_ptr_in_dump;
}

void hprof_dump_read(void *buf, int size)
{
    memcpy(buf, cur_ptr_in_dump, (SIZE_T)size);
    cur_ptr_in_dump += size;
}

void * hprof_dump_read_id(void)
{
    void *p;
    hprof_dump_read(&p, sizeof(void *));
    return p;
}

unsigned int hprof_dump_read_u4(void)
{
    jint u4;
    hprof_dump_read(&u4, sizeof(jint));
    return ntohl(u4);
}

unsigned short hprof_dump_read_u2(void)
{
    unsigned short u2;
    hprof_dump_read(&u2, sizeof(unsigned short));
    return ntohs(u2);
}

unsigned char hprof_dump_read_u1(void)
{
    unsigned char u1;
    hprof_dump_read(&u1, sizeof(unsigned char));
    return u1;
}

void hprof_io_setup(void)
{
    write_buf = HPROF_CALLOC(ALLOC_TYPE_ARRAY, WRITE_BUF_SIZE);
}

#ifdef WATCH_ALLOCS
void hprof_io_free(void) {
    hprof_free(write_buf);
    buf_index = 0;	/* keep freed buffer from being used */
}
#endif /* WATCH_ALLOCS */

void 
hprof_write_header(unsigned char type, jint length)
{
    hprof_write_u1(type);
    hprof_write_current_ticks();
    hprof_write_u4(length);
}

void 
hprof_write_dev(void *buf, int len)
{
    int res;
    if (hprof_socket_p) {
        res = hprof_send(hprof_fd, buf, len, 0);
    } else {
        res = hprof_write(hprof_fd, buf, len);
    }
    
    if (res < 0) {
        if (hprof_is_on) {
	    fprintf(stderr,
		    "HPROF ERROR: failed to write results, exiting .....\n");
	    hprof_is_on = FALSE;
	}
	CALL(ProfilerExit)((jint)1);
    }
}

void 
hprof_write_raw(void *buf, int len)
{
    if (buf_index + len > WRITE_BUF_SIZE) {
        hprof_flush();
	if (len > WRITE_BUF_SIZE) {
	    hprof_write_dev(buf, len);
	    return;
	}
    }
    memcpy(write_buf + buf_index, buf, len);
    buf_index += len;
}

void 
hprof_flush(void)
{
    if (buf_index) {
        hprof_write_dev(write_buf, buf_index);
	buf_index = 0;
    }
}

void 
hprof_printf(char *fmt, ...)
{
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    hprof_write_raw(buf, strlen(buf));
    va_end(args);
}

void 
hprof_write_current_ticks(void)
{
    hprof_write_u4(hprof_get_milliticks() * 1000 - micro_sec_ticks);
}

void
hprof_write_u4(unsigned int i)
{
    i = htonl(i);
    hprof_write_raw(&i, sizeof(unsigned int));
}

void
hprof_write_u2(unsigned short i)
{
    i = htons(i);
    hprof_write_raw(&i, sizeof(unsigned short));
}

void
hprof_write_u1(unsigned char i)
{
    hprof_write_raw(&i, sizeof(unsigned char));
}

void
hprof_write_id(void *p)
{
    hprof_write_raw(&p, sizeof(void *));
}

