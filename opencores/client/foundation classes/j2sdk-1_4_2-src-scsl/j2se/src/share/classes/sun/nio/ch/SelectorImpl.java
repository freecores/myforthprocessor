/*
 * @(#)SelectorImpl.java	1.40 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.*;
import java.nio.channels.spi.*;
import java.util.*;
import sun.misc.*;


/**
 * An implementation of Selector for Solaris.
 */

abstract class SelectorImpl
    extends AbstractSelector
{

    // The set of keys with data ready for an operation
    protected Set selectedKeys;

    // The set of keys registered with this Selector
    protected HashSet keys;

    // Gate for selection operations, so that close can avoid racing
    // to lock the selector itself and the key sets
    private static class SelectGate extends Object { }
    private SelectGate selectGate = new SelectGate();

    protected SelectorImpl(SelectorProvider sp) {
	super(sp);
        selectedKeys = new HashSet();
        keys = new HashSet();
    }

    public Set keys() {
        return keys;
    }

    public Set selectedKeys() {
        return selectedKeys;
    }

    protected abstract int doSelect(long timeout) throws IOException;

    private int lockAndDoSelect(long timeout) throws IOException {
	synchronized (this) {
	    if (!isOpen())
		throw new ClosedSelectorException();
	    synchronized (selectGate) { }
	    synchronized (keys) {
		synchronized (selectedKeys) {
		    return doSelect(timeout);
		}
	    }
        }
    }

    public int select(long timeout)
        throws IOException
    {
        if (timeout < 0)
            throw new IllegalArgumentException("Negative timeout");
	return lockAndDoSelect((timeout == 0) ? -1 : timeout);
    }

    public int select() throws IOException {
        return select(0);
    }

    public int selectNow() throws IOException {
	return lockAndDoSelect(0);
    }

    public void implCloseSelector() throws IOException {
	// Precondition: Already synchronized on this selector
	synchronized (selectGate) {
	    wakeup();
	    synchronized (keys) {
		synchronized (selectedKeys) {
		    implClose();
		}
	    }
	}
    }

    protected abstract void implClose() throws IOException;

    void putEventOps(SelectionKeyImpl sk, int ops) { }

    protected final SelectionKey register(AbstractSelectableChannel ch,
					  int ops,
					  Object attachment)
    {
	if (!(ch instanceof SelChImpl))
	    throw new IllegalSelectorException();
	SelectionKeyImpl k = new SelectionKeyImpl((SelChImpl)ch, this);
	k.attach(attachment);
	synchronized (keys) {
	    implRegister(k);
	}
        k.interestOps(ops);
	return k;
    }

    protected abstract void implRegister(SelectionKeyImpl ski);

    void processDeregisterQueue() throws IOException {
	// Precondition: Synchronized on this, keys, and selectedKeys
	Set cks = cancelledKeys();
        synchronized (cks) {
            Iterator i = cks.iterator();
            while (i.hasNext()) {
		SelectionKeyImpl ski;
                implDereg(ski = (SelectionKeyImpl)i.next());
		i.remove();
            }
        }
    }

    protected abstract void implDereg(SelectionKeyImpl ski) throws IOException;

    abstract public Selector wakeup();

}
