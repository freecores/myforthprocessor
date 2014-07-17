/*
 * @(#)NamedFieldIdentifier.java	1.3 03/01/23 11:27:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

/** Named fields in compound types */

public interface NamedFieldIdentifier extends FieldIdentifier {
  public String getName();
}
