/*
 * @(#)util5.h	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

extern "C" {
char* checked_malloc(int nbytes);

int wrap_dup2(int from, int to);

int read_PR_fully(const char* msg, PRFileDesc* pr, char* buffer, int length);

int write_PR_fully(const char* msg, PRFileDesc* pr, char* buff, int len);

int PRFileDesc_To_FD(PRFileDesc* pr);

void wrap_PR_CreatePipe(const char* msg, 
			PRFileDesc **readPipe, PRFileDesc **writePipe);

void wrap_PR_CreateSocketPair(const char* msg, PRFileDesc* fds[]);

}

