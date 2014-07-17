/*
 * @(#)CheckContext.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;

public
class CheckContext extends Context {
    public Vset vsBreak = Vset.DEAD_END;
    public Vset vsContinue = Vset.DEAD_END;

    // Accumulate (join) all DA/DU state prior to
    // any abnormal exit from a try-statement.
    // This field is ignored unless this
    // context is associated with a try-statement.
    public Vset vsTryExit = Vset.DEAD_END;

    /**
     * Create a new nested context, for a block statement
     */
    CheckContext(Context ctx, Statement stat) {
	super(ctx, stat);
    }
}
