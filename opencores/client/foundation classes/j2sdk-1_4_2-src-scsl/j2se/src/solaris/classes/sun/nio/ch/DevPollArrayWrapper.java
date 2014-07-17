/*
 * @(#)DevPollArrayWrapper.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import sun.misc.*;
import java.io.IOException;


/**
 * Manipulates a native array of pollfd structs on Solaris:
 *
 * typedef struct pollfd {
 *    int fd;
 *    short events;
 *    short revents;
 * } pollfd_t;
 *
 * @author Mike McCloskey
 * @version 1.3, 01/05/06
 * @since 1.4
 */

class DevPollArrayWrapper {

    // Event masks
    static final short POLLIN       = 0x0001;
    static final short POLLPRI      = 0x0002;
    static final short POLLOUT	    = 0x0004;
    static final short POLLRDNORM   = 0x0040;
    static final short POLLWRNORM   = POLLOUT;
    static final short POLLRDBAND   = 0x0080;
    static final short POLLWRBAND   = 0x0100;
    static final short POLLNORM	    = POLLRDNORM;
    static final short POLLERR	    = 0x0008;
    static final short POLLHUP	    = 0x0010;
    static final short POLLNVAL	    = 0x0020;
    static final short POLLREMOVE   = 0x0800;
    static final short POLLCONN     = POLLOUT;

    // Miscellaneous constants
    static final short SIZE_POLLFD   = 8;
    static final short FD_OFFSET     = 0;
    static final short EVENT_OFFSET  = 4;
    static final short REVENT_OFFSET = 6;
    static final int   NUM_POLLFDS   = 8192;

    // Base address of the native pollArray
    private long pollArrayAddress;

    DevPollArrayWrapper() {
        int allocationSize = NUM_POLLFDS * SIZE_POLLFD;
        pollArray = new AllocatedNativeObject(allocationSize, true);
        pollArrayAddress = pollArray.address();
        wfd = init();

        for (int i=0; i<NUM_POLLFDS; i++) {
            putDescriptor(i, 0);
            putEventOps(i, 0);
            putReventOps(i, 0);
        }
    }

    // The pollfd array for results from devpoll driver
    private AllocatedNativeObject pollArray;

    // The fd of the devpoll driver
    int wfd;

    // The fd of the interrupt line going out
    int outgoingInterruptFD;

    // The fd of the interrupt line coming in
    int incomingInterruptFD;

    // The index of the interrupt FD
    int interruptedIndex;

    // Number of updated pollfd entries
    int updated;

    void initInterrupt(int fd0, int fd1) {
        outgoingInterruptFD = fd1;
        incomingInterruptFD = fd0;
        register(wfd, fd0, POLLIN);
    }

    void putEventOps(int i, int event) {
        int offset = SIZE_POLLFD * i + EVENT_OFFSET;
        pollArray.putShort(offset, (short)event);
    }

    void putReventOps(int i, int revent) {
        int offset = SIZE_POLLFD * i + REVENT_OFFSET;
        pollArray.putShort(offset, (short)revent);
    }

    void putDescriptor(int i, int fd) {
        int offset = SIZE_POLLFD * i + FD_OFFSET;
        pollArray.putInt(offset, fd);
    }

    int getEventOps(int i) {
        int offset = SIZE_POLLFD * i + EVENT_OFFSET;
        return pollArray.getShort(offset);
    }

    int getReventOps(int i) {
        int offset = SIZE_POLLFD * i + REVENT_OFFSET;
        return pollArray.getShort(offset);
    }

    int getDescriptor(int i) {
        int offset = SIZE_POLLFD * i + FD_OFFSET;
        return pollArray.getInt(offset);
    }

    void setInterest(int fd, int mask) {
        register(wfd, fd, mask);
    }

    void release(int fd) {
        register(wfd, fd, POLLREMOVE);
    }

    void closeDevPollFD() throws IOException {
        FileDispatcher.closeIntFD(wfd);
        pollArray.free();
    }

    int poll(long timeout) {
        updated = poll0(pollArrayAddress, NUM_POLLFDS, timeout, wfd);
        for (int i=0; i<updated; i++) {
            if (getDescriptor(i) == incomingInterruptFD) {
                interruptedIndex = i;
                interrupted = true;
                break;
            }
        }
        return updated;
    }

    boolean interrupted = false;

    public void interrupt() {
        interrupt(outgoingInterruptFD);
    }

    public int interruptedIndex() {
        return interruptedIndex;
    }

    boolean interrupted() {
        return interrupted;
    }

    void clearInterrupted() {
        interrupted = false;
    }

    private native int init();
    private native void register(int wfd, int fd, int mask);
    private native int poll0(long pollAddress, int numfds, long timeout,
                             int wfd);
    private static native void interrupt(int fd);

}
