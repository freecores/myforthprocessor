/*
 * @(#)DatagramSocketAdaptor.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.*;
import java.net.*;
import java.nio.*;
import java.nio.channels.*;


// Make a datagram-socket channel look like a datagram socket.
//
// The methods in this class are defined in exactly the same order as in
// java.net.DatagramSocket so as to simplify tracking future changes to that
// class.
//

public class DatagramSocketAdaptor
    extends DatagramSocket
{

    // The channel being adapted
    private final DatagramChannelImpl dc;

    // Option adaptor object, created on demand
    private volatile OptionAdaptor opts = null;

    // Timeout "option" value for receives
    private volatile int timeout = 0;

    // Traffic-class/Type-of-service 
    private volatile int trafficClass = 0;


    // ## super will create a useless impl
    private DatagramSocketAdaptor(DatagramChannelImpl dc) throws IOException {
	// Invoke the DatagramSocketAdaptor(SocketAddress) constructor,
	// passing a null address, in order to avoid an invocation of
	// our bind method before the dc field is initialized.
	super((SocketAddress)null);
	this.dc = dc;
    }

    public static DatagramSocket create(DatagramChannelImpl dc) {
	try {
	    return new DatagramSocketAdaptor(dc);
	} catch (IOException x) {
	    throw new Error(x);
	}
    }

    private void connectInternal(SocketAddress remote)
	throws SocketException
    {
	InetSocketAddress isa = Net.asInetSocketAddress(remote);
	int port = isa.getPort();
        if (port < 0 || port > 0xFFFF)
            throw new IllegalArgumentException("connect: " + port);
        if (remote == null)
            throw new IllegalArgumentException("connect: null address");
        if (!isClosed())
            return;
	try {
	    dc.connect(remote);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public void bind(SocketAddress local) throws SocketException {
	try {
	    if (local == null)
		local = new InetSocketAddress(0);
	    dc.bind(local);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public void connect(InetAddress address, int port) {
	try {
	    connectInternal(new InetSocketAddress(address, port));
	} catch (SocketException x) {
	    // Yes, j.n.DatagramSocket really does this
	}
    }

    public void connect(SocketAddress remote) throws SocketException {
	if (remote == null)
	    throw new IllegalArgumentException("Address can't be null");
	connectInternal(remote);
    }

    public void disconnect() {
	try {
	    dc.disconnect();
	} catch (IOException x) {
	    throw new Error(x);
	}
    }

    public boolean isBound() {
	return dc.isBound();
    }

    public boolean isConnected() {
	return dc.isConnected();
    }

    public InetAddress getInetAddress() {
	return (isConnected()
		? Net.asInetSocketAddress(dc.remoteAddress()).getAddress()
		: null);
    }

    public int getPort() {
	return (isConnected()
		? Net.asInetSocketAddress(dc.remoteAddress()).getPort()
		: -1);
    }

    public void send(DatagramPacket p) throws IOException {
	synchronized (dc.blockingLock()) {
	    if (!dc.isBlocking())
		throw new IllegalBlockingModeException();
	    try {
		synchronized (p) {
		    ByteBuffer bb = ByteBuffer.wrap(p.getData(),
						    p.getOffset(),
						    p.getLength());
		    dc.send(bb, p.getSocketAddress());
		}
	    } catch (IOException x) {
		Net.translateException(x);
	    }
	}
    }

    // Must hold dc.blockingLock()
    //
    private void receive(ByteBuffer bb) throws IOException {
	if (timeout == 0) {
	    dc.receive(bb);
	    return;
	}

	// Implement timeout with a selector
	SelectionKey sk = null;
	dc.configureBlocking(false);
	try {
	    int n;
	    if (dc.receive(bb) != null)
		return;
	    Selector sel = Util.getTemporarySelector(dc);
	    sk = dc.register(sel, SelectionKey.OP_READ);
	    long to = timeout;
	    for (;;) {
		long st = System.currentTimeMillis();
		sel.select(to);
		if (sk.isReadable()) {
		    if (dc.receive(bb) != null)
			return;
		}
		sel.selectedKeys().remove(sk);
		to -= System.currentTimeMillis() - st;
		if (to <= 0)
		    throw new SocketTimeoutException();
	    }
	} finally {
	    if (sk != null)
		sk.cancel();
	    if (dc.isOpen())
		dc.configureBlocking(true);
	}
    }
			
    public void receive(DatagramPacket p) throws IOException {
	synchronized (dc.blockingLock()) {
	    if (!dc.isBlocking())
		throw new IllegalBlockingModeException();
	    try {
		synchronized (p) {
		    ByteBuffer bb = ByteBuffer.wrap(p.getData(),
						    p.getOffset(),
						    p.getLength());
		    receive(bb);
		    p.setLength(bb.position() - p.getOffset());
		}
	    } catch (IOException x) {
		Net.translateException(x);
	    }
	}
    }

    public InetAddress getLocalAddress() {
	if (isClosed())
	    return null;
	try {
	    return Net.asInetSocketAddress(dc.localAddress()).getAddress();
	} catch (Exception x) {
	    return new InetSocketAddress(0).getAddress();
	}
    }

    public int getLocalPort() {
	if (isClosed())
	    return -1;
	try {
	    return Net.asInetSocketAddress(dc.localAddress()).getPort();
	} catch (Exception x) {
	    return 0;
	}
    }

    public void setSoTimeout(int timeout) throws SocketException {
	this.timeout = timeout;
    }

    public int getSoTimeout() throws SocketException {
	return timeout;
    }

    private OptionAdaptor opts() {
	if (opts == null)
	    opts = new OptionAdaptor(dc);
	return opts;
    }

    public void setSendBufferSize(int size) throws SocketException {
	opts().setSendBufferSize(size);
    }

    public int getSendBufferSize() throws SocketException {
	return opts().getSendBufferSize();
    }

    public void setReceiveBufferSize(int size) throws SocketException {
	opts().setReceiveBufferSize(size);
    }

    public int getReceiveBufferSize() throws SocketException {
	return opts().getReceiveBufferSize();
    }

    public void setReuseAddress(boolean on) throws SocketException {
	opts().setReuseAddress(on);
    }

    public boolean getReuseAddress() throws SocketException {
	return opts().getReuseAddress();
    }

    public void setBroadcast(boolean on) throws SocketException {
	opts().setBroadcast(on);
    }

    public boolean getBroadcast() throws SocketException {
	return opts().getBroadcast();
    }

    public void setTrafficClass(int tc) throws SocketException {
	opts().setTrafficClass(tc);
	trafficClass = tc;
    }

    public int getTrafficClass() throws SocketException {
	int tc = opts().getTrafficClass();
	if (tc < 0) {
	    tc = trafficClass;
	}
	return tc;
    }

    public void close() {
	try {
	    dc.close();
	} catch (IOException x) {
	    throw new Error(x);
	}
    }

    public boolean isClosed() {
	return !dc.isOpen();
    }

    public DatagramChannel getChannel() {
	return dc;
    }

}