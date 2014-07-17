/*
 * @(#)SelectionKeyImpl.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.*;
import java.nio.channels.spi.*;


/**
 * An implementation of SelectionKey for Solaris.
 */

class SelectionKeyImpl
    extends AbstractSelectionKey
{

    final SelChImpl channel;				// package-private
    final SelectorImpl selector;			// package-private

    // Index for a pollfd array in Selector that this key is registered with
    private int index;

    private int interestOps;
    private int readyOps;

    SelectionKeyImpl(SelChImpl ch, SelectorImpl sel) {
	channel = ch;
	selector = sel;
    }

    public SelectableChannel channel() {
	return (SelectableChannel)channel;
    }

    public Selector selector() {
	return selector;
    }

    int getIndex() {					// package-private
        return index;
    }

    void setIndex(int i) {				// package-private
        index = i;
    }

    private void ensureValid() {
        if (!isValid())
            throw new CancelledKeyException();
    }

    public int interestOps() {
        ensureValid();
        synchronized (selector.selectedKeys()) {
            return interestOps;
        }
    }

    public SelectionKey interestOps(int ops) {
        ensureValid();
        synchronized (selector.selectedKeys()) {
            if ((ops & ~channel().validOps()) != 0)
                throw new IllegalArgumentException();
            channel.translateAndSetInterestOps(ops, this);
            interestOps = ops;
        }
	return this;
    }

    public int readyOps() {				// package-private
	ensureValid();
        return readyOps;
    }

    void readyOps(int ops) {				// package-private
        readyOps = ops;
    }

}
