/*
 * @(#)SPARCRegisterType.java	1.3 03/01/23 11:18:08
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

/** A class providing the enums describing SPARC register types */

public class SPARCRegisterType {
  private SPARCRegisterType() {
  }

  public static final SPARCRegisterType GLOBAL = new SPARCRegisterType();
  public static final SPARCRegisterType OUT    = new SPARCRegisterType();
  public static final SPARCRegisterType IN     = new SPARCRegisterType();
  public static final SPARCRegisterType LOCAL  = new SPARCRegisterType();
}
