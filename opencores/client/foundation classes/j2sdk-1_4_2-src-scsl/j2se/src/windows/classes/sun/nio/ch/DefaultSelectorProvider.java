/*
 * @(#)DefaultSelectorProvider.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.nio.channels.spi.SelectorProvider;


/**
 * Creates this platform's default SelectorProvider
 */

public class DefaultSelectorProvider {

    /**
     * Prevent instantiation.
     */
    private DefaultSelectorProvider() { }

    /**
     * Returns the default SelectorProvider.
     */
    public static SelectorProvider create() {
        return new sun.nio.ch.WindowsSelectorProvider();
    }

}
