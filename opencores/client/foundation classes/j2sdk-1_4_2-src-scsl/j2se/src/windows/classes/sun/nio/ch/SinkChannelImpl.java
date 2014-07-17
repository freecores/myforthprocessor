/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SinkChannelImpl.java	1.8 03/01/23
 */

package sun.nio.ch;

import java.io.IOException;
import java.io.FileDescriptor;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.nio.channels.spi.*;


/** 
 * Pipe.SinkChannel implementation based on socket connection.
 */

class SinkChannelImpl
    extends Pipe.SinkChannel
    implements SelChImpl
{
    // The SocketChannel assoicated with this pipe
    SocketChannel sc;

    public FileDescriptor getFD() {
	return ((SocketChannelImpl)sc).getFD();
    }

    public int getFDVal() {
	return ((SocketChannelImpl)sc).getFDVal();
    }

    SinkChannelImpl(SelectorProvider sp, SocketChannel sc) {
	super(sp);
	this.sc = sc;
    }

    protected void implCloseSelectableChannel() throws IOException {
	if (!isRegistered())
	    kill();
    }

    public void kill() throws IOException {
	sc.close();
    }

    protected void implConfigureBlocking(boolean block) throws IOException {
	sc.configureBlocking(block);
    }

    public boolean translateReadyOps(int ops, int initialOps,
                                     SelectionKeyImpl sk) {
        int intOps = sk.interestOps();    // Do this just once, it synchronizes
        int oldOps = sk.readyOps();
        int newOps = initialOps;

        if ((ops & PollArrayWrapper.POLLNVAL) != 0)
            throw new Error("POLLNVAL detected");

        if ((ops & (PollArrayWrapper.POLLERR
                    | PollArrayWrapper.POLLHUP)) != 0) {
            newOps = intOps;
            sk.readyOps(newOps);
            return (newOps & ~oldOps) != 0;
        }

        if (((ops & PollArrayWrapper.POLLOUT) != 0) &&
            ((intOps & SelectionKey.OP_WRITE) != 0))
            newOps |= SelectionKey.OP_WRITE;

        sk.readyOps(newOps);
        return (newOps & ~oldOps) != 0;
    }

    public boolean translateAndUpdateReadyOps(int ops, SelectionKeyImpl sk) {
        return translateReadyOps(ops, sk.readyOps(), sk);
    }

    public boolean translateAndSetReadyOps(int ops, SelectionKeyImpl sk) {
        return translateReadyOps(ops, 0, sk);
    }

    public void translateAndSetInterestOps(int ops, SelectionKeyImpl sk) {
        if ((ops & SelectionKey.OP_WRITE) != 0)
            ops = PollArrayWrapper.POLLOUT;
        sk.selector.putEventOps(sk, ops);
    }

    public int write(ByteBuffer src) throws IOException {
	try {
	    return sc.write(src);
	} catch (AsynchronousCloseException x) {
	    close();
	    throw x;
	}
    }

    public long write(ByteBuffer[] srcs) throws IOException {
	try {
	    return sc.write(srcs);
	} catch (AsynchronousCloseException x) {
	    close();
	    throw x;
	}
    }

    public long write(ByteBuffer[] srcs, int offset, int length)
	throws IOException
    {
	try {
	    return write(Util.subsequence(srcs, offset, length));
	} catch (AsynchronousCloseException x) {
	    close();
	    throw x;
	}
    }
}
