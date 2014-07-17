/*
 * @(#)EditorCommands.java	1.3 03/01/23 11:48:46
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui;

/** The debugger supplies an implementation of this interface to
    Editors it requests be opened by the EditorFactory. Using this
    object the Editor can communicate with the debugger. */

public interface EditorCommands {
  /** Notifies the debugger that the editing window has been closed. */
  public void windowClosed(Editor editor);

  /** Toggles a breakpoint at the given (one-based) line */
  public void toggleBreakpointAtLine(Editor editor, int lineNumber);
}
