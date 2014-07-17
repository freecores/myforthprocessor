/*
 * @(#)NativeSeedGenerator.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider;

import java.io.IOException;

/**
 * Native seed generator for Unix systems. Inherit everything from
 * URLSeedGenerator.
 *
 * @version 1.2, 01/23/03
 */
class NativeSeedGenerator extends SeedGenerator.URLSeedGenerator {

    NativeSeedGenerator() throws IOException {
	super();
    }

}
