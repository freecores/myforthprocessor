/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)QueueRunnable.cpp	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Contains definition of the Runnable class needed to process
 * the worker and spontainious pipes
 */

#include <poll.h>
#include <errno.h>

#include "QueueRunnable.h"
#include "nsIServiceManager.h"
#include "nsIJVMManager.h"

#include "Debug.h"

NS_IMPL_ISUPPORTS1(QueueRunnable, nsIRunnable)

QueueRunnable::QueueRunnable(void * vm, pfnQueueProcessor f, int fd, PRUint32 tid, PRMonitor * m, bool * pc, nsIThreadManager * tm )
{
    NS_INIT_ISUPPORTS();

    mJVM = vm;
    mFunction = f;
    mFD = fd;
    mThreadID = tid;
    mMonitor = m;
    mIsPipeClean = pc;
    mThreadManager = tm;
    mThreadManager->AddRef();

/*
    nsIServiceManager * theServiceManager;
    nsServiceManager::GetGlobalServiceManager(&theServiceManager);
    theServiceManager->GetService(nsIJVMManager::GetCID(), 
                                      NS_GET_IID(nsIThreadManager),
                                      (nsISupports**)&mThreadManager);
*/
}

QueueRunnable::~QueueRunnable()
{
    if(mThreadManager) {
        NS_RELEASE(mThreadManager);
    }
}

NS_IMETHODIMP QueueRunnable::Run()
{
    if (mFunction) {
        mFunction((void *) mJVM);
    }
    return NS_OK;
}

void QueueRunnable::waitOnPipe()
{
    int rv;
    struct pollfd fds[1];
    fds[0].fd = mFD;
    fds[0].events = POLLRDNORM;
 
    for(;;) {
        fds[0].revents = 0;
        rv = poll(fds,1,-1);
        if(rv == -1) {
           // Deal with error case here
           if(errno != EINTR) {
               break;
           }
        } else {
           // Make sure we are out of poll for the right reason and
           // Put us one the Event Que of the main thread
            if(fds[0].revents & POLLRDNORM) {
                if(mThreadManager) {
                    trace("QueueRunnable: Posting Event: pipe %d activity\n",mFD);
                    PR_EnterMonitor(mMonitor);
                    *mIsPipeClean = false;
                    mThreadManager->PostEvent(mThreadID, this, PR_TRUE);
                    while(!*mIsPipeClean) PR_Wait(mMonitor,
                                          PR_INTERVAL_NO_TIMEOUT);
                    PR_ExitMonitor(mMonitor);
                }
            }
        }
    }   
}

void QueueRunnable::threadEntry(void *arg) {

    QueueRunnable * qr = (QueueRunnable *) arg;

    qr->AddRef();
    qr->waitOnPipe();
    qr->Release();
}
