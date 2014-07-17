/*
 * @(#)nsprPrivate.h	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* These are copies of private data structures from the NSPR lib. We need
   them because NSPR does not reveal the internals of the PRFileDesc, which
   we need in order to pass the fd for the pipe to the Java process */

/* from nsprpub/pr/include/md/_unixos.h */
struct _MDFileDesc {
    int osfd;
};

/* From nsprpub/pr/include/private/primpl.h */
/* Mozilla changes: This struct has changed */
typedef struct _MDFileDesc _MDFileDesc;
struct PRFilePrivate {
    PRInt32 state;
    PRBool nonblocking;
    PRBool inheritable;
    PRFileDesc *next;
    PRIntn lockCount;
    _MDFileDesc md;
#ifdef _PR_PTHREADS
    PRIntn eventMask[1];
#endif
};
