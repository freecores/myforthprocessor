/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)QueueRunnable.h	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Contains declaration of the Runnable class needed to process
 * the workier and spontainious pipes
 */


#ifndef QUEUERUNNABLE_H
#define QUEUERUNNABLE_H

#include "nsIThreadManager.h"
#include "nspr.h"

typedef void (* pfnQueueProcessor)(void *);

class QueueRunnable : public nsIRunnable {
public:

    NS_DECL_ISUPPORTS

    NS_IMETHOD Run();
	
    QueueRunnable(void * vm, pfnQueueProcessor f, int fd, PRUint32 tid, PRMonitor * m, bool * pc, nsIThreadManager * tm);
    virtual	~QueueRunnable(void);

    void waitOnPipe(void);
    static void threadEntry(void * arg);

private:
    PRMonitor * mMonitor;
    bool * mIsPipeClean;
    void * mJVM;
    pfnQueueProcessor mFunction;
    int mFD;
    PRUint32 mThreadID;
    nsIThreadManager* mThreadManager;

};

#endif 
