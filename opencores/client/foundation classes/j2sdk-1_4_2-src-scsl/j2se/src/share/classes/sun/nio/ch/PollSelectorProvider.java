/*
 * @(#)PollSelectorProvider.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.*;
import java.nio.channels.spi.*;

public class PollSelectorProvider
    extends SelectorProviderImpl
{
    public AbstractSelector openSelector() throws IOException {
	return new PollSelectorImpl(this);
    }
}
