/*
 * @(#)Util.java	1.20 03/03/07
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.lang.ref.SoftReference;
import java.lang.reflect.*;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.nio.channels.*;
import java.nio.channels.spi.*;
import java.security.AccessController;
import java.security.PrivilegedAction;
import sun.misc.Unsafe;


class Util {


    // -- Caches --

    // The number of temp buffers in our pool
    private static final int TEMP_BUF_POOL_SIZE = 3;

    // Per-thread soft cache of the last temporary direct buffer
    private static ThreadLocal[] bufferPool;

    static {
        bufferPool = new ThreadLocal[TEMP_BUF_POOL_SIZE];
        for (int i=0; i<TEMP_BUF_POOL_SIZE; i++)
            bufferPool[i] = new ThreadLocal();
    }

    static ByteBuffer getTemporaryDirectBuffer(int size) {
        ByteBuffer buf = null;
        // Grab a buffer if available
        for (int i=0; i<TEMP_BUF_POOL_SIZE; i++) {
            SoftReference ref = (SoftReference)(bufferPool[i].get());
            if ((ref != null) && ((buf = (ByteBuffer)ref.get()) != null) &&
                (buf.capacity() >= size)) {
                buf.rewind();
                buf.limit(size);
                bufferPool[i].set(null);
                return buf;
            }
        }

        // Make a new one
        return ByteBuffer.allocateDirect(size);
    }

    static void releaseTemporaryDirectBuffer(ByteBuffer buf) {
        // Find an empty slot
        for (int i=0; i<TEMP_BUF_POOL_SIZE; i++) {
            SoftReference ref = (SoftReference)(bufferPool[i].get());
            if ((ref == null) || ((buf = (ByteBuffer)ref.get()) == null)) {
                bufferPool[i].set(new SoftReference(buf));
            }
        }
    }

    // Per-thread cached selector
    private static ThreadLocal localSelector = new ThreadLocal();

    // When finished, invoker must ensure that selector is empty
    // by cancelling any related keys
    //
    static Selector getTemporarySelector(SelectableChannel sc)
	throws IOException
    {
	SoftReference ref = (SoftReference)localSelector.get();
	Selector sel = null;
	if ((ref == null) || ((sel = (Selector)ref.get()) == null)
	    || (sel.provider() != sc.provider())) {
	    sel = sc.provider().openSelector();
	    localSelector.set(new SoftReference(sel));
	} else {
	    // Selector should be empty
	    sel.selectNow();		// Flush cancelled keys
	    assert sel.keys().isEmpty() : "Temporary selector not empty";
	}
	return sel;
    }


    // -- Random stuff --

    static ByteBuffer[] subsequence(ByteBuffer[] bs, int offset, int length) {
	if ((offset == 0) && (length == bs.length))
	    return bs;
	int n = length;
	ByteBuffer[] bs2 = new ByteBuffer[n];
	for (int i = 0; i < n; i++)
	    bs2[i] = bs[offset + i];
	return bs2;
    }


    // -- Unsafe access --

    private static Unsafe unsafe = Unsafe.getUnsafe();

    private static byte _get(long a) {
	return unsafe.getByte(a);
    }

    private static void _put(long a, byte b) {
	unsafe.putByte(a, b);
    }

    static void erase(ByteBuffer bb) {
	unsafe.setMemory(((DirectBuffer)bb).address(), bb.capacity(), (byte)0);
    }

    static Unsafe unsafe() {
	return unsafe;
    }


    private static int pageSize = -1;

    static int pageSize() {
	if (pageSize == -1)
	    pageSize = unsafe().pageSize();
	return pageSize;
    }

    private static Constructor directByteBufferConstructor = null;

    private static void initDBBConstructor() {
	AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
		    try {
			Class th = Class.forName("java.nio.DirectByteBuffer");
			directByteBufferConstructor
			    = th.getDeclaredConstructor(
					new Class[] { int.class,
                                                      long.class,
						      Runnable.class });
			directByteBufferConstructor.setAccessible(true);
		    } catch (ClassNotFoundException x) {
			throw new InternalError();
		    } catch (NoSuchMethodException x) {
			throw new InternalError();
		    } catch (IllegalArgumentException x) {
			throw new InternalError();
		    } catch (ClassCastException x) {
			throw new InternalError();
		    }
                    return null;
		}});
    }

    static MappedByteBuffer newMappedByteBuffer(int size, long addr,
						Runnable unmapper)
    {
        MappedByteBuffer dbb;
        if (directByteBufferConstructor == null)
            initDBBConstructor();
        try {
            dbb = (MappedByteBuffer)directByteBufferConstructor.newInstance(
              new Object[] { new Integer(size),
                             new Long(addr),
			     unmapper });
        } catch (InstantiationException e) {
            throw new InternalError();
        } catch (IllegalAccessException e) {
            throw new InternalError();
        } catch (InvocationTargetException e) {
            throw new InternalError();
        }
        return dbb;
    }

    private static Constructor directByteBufferRConstructor = null;

    private static void initDBBRConstructor() {
	AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
		    try {
			Class th = Class.forName("java.nio.DirectByteBufferR");
			directByteBufferRConstructor
			    = th.getDeclaredConstructor(
					new Class[] { int.class,
                                                      long.class,
						      Runnable.class });
			directByteBufferRConstructor.setAccessible(true);
		    } catch (ClassNotFoundException x) {
			throw new InternalError();
		    } catch (NoSuchMethodException x) {
			throw new InternalError();
		    } catch (IllegalArgumentException x) {
			throw new InternalError();
		    } catch (ClassCastException x) {
			throw new InternalError();
		    }
                    return null;
		}});
    }

    static MappedByteBuffer newMappedByteBufferR(int size, long addr,
						 Runnable unmapper)
    {
        MappedByteBuffer dbb;
        if (directByteBufferRConstructor == null)
            initDBBRConstructor();
        try {
            dbb = (MappedByteBuffer)directByteBufferRConstructor.newInstance(
              new Object[] { new Integer(size),
                             new Long(addr),
			     unmapper });
        } catch (InstantiationException e) {
            throw new InternalError();
        } catch (IllegalAccessException e) {
            throw new InternalError();
        } catch (InvocationTargetException e) {
            throw new InternalError();
        }
        return dbb;
    }


    // -- Initialization --

    private static boolean loaded = false;

    static void load() {
	synchronized (Util.class) {
	    if (loaded)
		return;
            loaded = true;
	    java.security.AccessController
		.doPrivileged(new sun.security.action.LoadLibraryAction("net"));
	    java.security.AccessController
		.doPrivileged(new sun.security.action.LoadLibraryAction("nio"));
            // IOUtil must be initialized; Its native methods are called from
            // other places in native nio code so they must be set up.
            IOUtil.initIDs();
	}
    }

}
