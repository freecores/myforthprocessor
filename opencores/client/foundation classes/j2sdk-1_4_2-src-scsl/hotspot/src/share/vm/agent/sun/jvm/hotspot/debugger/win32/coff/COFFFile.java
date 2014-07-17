/*
 * @(#)COFFFile.java	1.3 03/01/23 11:32:33
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Top-level interface modeling the information stored in either a
    Portable Executable or object file. */

public interface COFFFile {
  public COFFHeader getHeader();

  /** Indicates whether the file is an image (.EXE or .DLL) or not
      (.OBJ). */
  public boolean isImage();

  /** Closes this COFF file. All subordinate objects are invalid after
      it is closed. */
  public void close();
}
