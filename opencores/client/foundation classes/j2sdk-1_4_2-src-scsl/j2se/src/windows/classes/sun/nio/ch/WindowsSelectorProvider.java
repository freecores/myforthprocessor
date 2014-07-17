/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)WindowsSelectorProvider.java	1.3 03/01/23
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.spi.AbstractSelector;

/*
 * SelectorProvider for sun.nio.ch.WindowsSelectorImpl.
 *
 * @author Konstantin Kladko
 * @version 1.3, 01/23/03
 * @since 1.4
 */

public class WindowsSelectorProvider extends SelectorProviderImpl {
    
    public AbstractSelector openSelector() throws IOException {
        return new WindowsSelectorImpl(this);
    }
}
