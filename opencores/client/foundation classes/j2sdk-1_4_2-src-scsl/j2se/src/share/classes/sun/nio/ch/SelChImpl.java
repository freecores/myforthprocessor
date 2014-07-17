/*
 * @(#)SelChImpl.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.FileDescriptor;
import java.io.IOException;


/**
 * An interface that allows translation (and more!).
 *
 * @version 1.12 03/01/23
 * @since 1.4
 */

interface SelChImpl {

    FileDescriptor getFD();

    int getFDVal();

    /**
     * Adds the specified ops if present in interestOps. The specified
     * ops are turned on without affecting the other ops.
     *
     * @return  true iff the new value of sk.readyOps() set by this method
     *          contains at least one bit that the previous value did not
     *          contain
     */
    public boolean translateAndUpdateReadyOps(int ops, SelectionKeyImpl sk);

    /**
     * Sets the specified ops if present in interestOps. The specified
     * ops are turned on, and all other ops are turned off.
     *
     * @return  true iff the new value of sk.readyOps() set by this method
     *          contains at least one bit that the previous value did not
     *          contain
     */
    public boolean translateAndSetReadyOps(int ops, SelectionKeyImpl sk);

    void translateAndSetInterestOps(int ops, SelectionKeyImpl sk);

    int validOps();

    void kill() throws IOException;

}
