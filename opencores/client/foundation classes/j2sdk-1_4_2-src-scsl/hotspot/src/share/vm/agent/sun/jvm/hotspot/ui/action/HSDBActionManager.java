/* 
 * @(#)HSDBActionManager.java	1.5 03/01/23 11:49:45
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui.action;

import com.sun.java.swing.action.ActionManager;

/**
 * An application specific implementation of an ActionManager
 */
public class HSDBActionManager extends ActionManager {
    
    public static ActionManager getInstance() {
	if (manager == null) {
	    manager = new HSDBActionManager();
	}
	return manager;
    }

    protected void addActions() {
	// actions for ObjectHistogramPanel 
	addAction(FindAction.VALUE_COMMAND, new FindAction());
	addAction(ShowAction.VALUE_COMMAND, new ShowAction());

	// Actions for Java Threads Panel
	addAction(InspectAction.VALUE_COMMAND, new InspectAction());
	addAction(MemoryAction.VALUE_COMMAND, new MemoryAction());
	addAction(ThreadInfoAction.VALUE_COMMAND, new ThreadInfoAction());
	addAction(FindCrashesAction.VALUE_COMMAND, new FindCrashesAction());
        addAction(JavaStackTraceAction.VALUE_COMMAND, new JavaStackTraceAction());

        // Action for ClassBrowserPanel
        addAction(FindClassesAction.VALUE_COMMAND, new FindClassesAction());
    }
}
