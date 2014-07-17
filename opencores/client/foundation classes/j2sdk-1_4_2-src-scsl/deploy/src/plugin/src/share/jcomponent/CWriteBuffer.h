/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__CWriteBuffer__)
#define __CWriteBuffer__

class CWriteBuffer {
    char * m_buff;
    int m_buff_length;
    int m_data_length;
    const static int CHUNK = 1024;

public:
    CWriteBuffer(int size = CHUNK);
    ~CWriteBuffer();
    void putInt(int x);
    void putShort(short x);
    void putString(const char *);
    void putString(const char *, int length);
    void checkBuffSize(int size);
    int send(int fd) const;
    operator char*();
};

#endif
