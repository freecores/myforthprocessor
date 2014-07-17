/*
 * @(#)FileLockImpl.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.FileLock;
import java.nio.channels.FileChannel;

public class FileLockImpl
    extends FileLock
{
    boolean valid;

    FileLockImpl(FileChannel channel, long position, long size, boolean shared)
    {
        super(channel, position, size, shared);
        this.valid = true;
    }

    public synchronized boolean isValid() {
        return valid;
    }

    synchronized void invalidate() {
        valid = false;
    }

    public synchronized void release() throws IOException {
        if (valid) {
            ((FileChannelImpl)channel()).release(this);
            valid = false;
        }
    }

}
