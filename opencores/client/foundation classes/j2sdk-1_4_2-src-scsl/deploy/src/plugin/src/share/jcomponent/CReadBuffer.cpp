/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "CReadBuffer.h"
#include <unistd.h>
#include <stdlib.h>

CReadBuffer::CReadBuffer(int fd) {
    m_fd = fd;
}

int CReadBuffer::getInt(int * x) {

    char buff[4];
    int rv = getIt(buff, 4);
    int result = (buff[0] << 24) |
		 ((buff[1] << 16) & 0xFF0000) |
		 ((buff[2] << 8) & 0xFF00) |
		 (buff[3] & 0xFF);
    *x = result;
    return rv;
}

int CReadBuffer::getShort(short * x) {

    char buff[2];
    int rv = getIt(buff, 2);
    short result = (buff[0] << 8) |
		   (buff[1] & 0xFF);
    *x = result;
    return rv;
}

int CReadBuffer::getString(char ** x) {

    int rc = 0;
    int length;
    
    rc = getInt(&length);
    if (rc <= 0) {
        *x = NULL;
        return rc;
    }
    char * buff;

    buff = (char *) malloc(length + 1);

    getIt(buff,length); 

    buff[length] = 0;
    *x = buff;

    return length;
}

int CReadBuffer::getByte(char * x) {

    return getIt((char *) x, 1);
}


void CReadBuffer::free(char * buf) {
    ::free(buf);
}

int CReadBuffer::getIt(char * buff,int length) {

    int rc;
    int offset = 0;
    while(offset != length) {
	rc = read(m_fd, buff+offset, length-offset);
	if (rc <= 0) return rc;
	offset = offset + rc;
    }
    return length;
}
