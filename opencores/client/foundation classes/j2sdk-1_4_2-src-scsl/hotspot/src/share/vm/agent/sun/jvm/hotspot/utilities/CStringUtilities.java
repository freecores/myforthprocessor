/*
 * @(#)CStringUtilities.java	1.3 03/01/23 11:51:14
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.debugger.*;

/** A utility class encapsulating useful operations on C strings
    represented as Addresses */

public class CStringUtilities {
  /** Return the length of a null-terminated ASCII string in the
      remote process */
  public static int getStringLength(Address addr) {
    int i = 0;
    while (addr.getCIntegerAt(i, 1, false) != 0) {
      i++;
    }
    return i;
  }

  /** Fetch a null-terminated ASCII string from the remote process.
      Returns null if the argument is null, otherwise returns a
      non-null string (for example, returns an empty string if the
      first character fetched is the null terminator). */
  public static String getString(Address addr) {
    if (addr == null) {
      return null;
    }

    List data = new ArrayList();
    byte val = 0;
    long i = 0;
    do {
      val = (byte) addr.getCIntegerAt(i, 1, false);
      if (val != 0) {
        data.add(new Byte(val));
      }
      ++i;
    } while (val != 0);

    // Convert to byte[] and from there to String
    byte[] bytes = new byte[data.size()];
    for (i = 0; i < data.size(); ++i) {
      bytes[(int) i] = ((Byte) data.get((int) i)).byteValue();
    }
    try {
      return new String(bytes, "US-ASCII");
    }
    catch (UnsupportedEncodingException e) {
      throw new RuntimeException("Error converting bytes to String using ASCII encoding");
    }
  }
}
