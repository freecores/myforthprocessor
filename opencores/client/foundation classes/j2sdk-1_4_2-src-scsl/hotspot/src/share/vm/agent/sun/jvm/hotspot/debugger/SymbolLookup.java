/*
 * @(#)SymbolLookup.java	1.3 03/01/23 11:25:59
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

public interface SymbolLookup {
  /** Looks up the given symbol in the context of the given object.

      <P>

      FIXME: we may want to hide the objectName so the user does not
      have to specify it, but it isn't clear whether this will work
      transparently with dbx.

      <P>

      FIXME: what happens if the address is not found? Throw
      exception? Currently returns null. */
  public Address lookup(String objectName, String symbol);

  /** Looks up the given symbol in the context of the given object,
      assuming that symbol refers to a Java object.

      FIXME: still not sure whether this will be necessary. Seems that
      lookup of static fields with type "oop" already works, since the
      lookup routine returns the address of the oop (i.e., an
      oopDesc**).

      <P>

      FIXME: we may want to hide the objectName so the user does not
      have to specify it, but it isn't clear whether this will work
      transparently with dbx.

      <P>

      FIXME: what happens if the address is not found? Throw
      exception? Currently returns null. */
  public OopHandle lookupOop(String objectName, String symbol);
}
