/*
 * @(#)NoSuchSymbolException.java	1.3 03/01/23 11:25:36
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

public class NoSuchSymbolException extends RuntimeException {
  private String symbol;

  public NoSuchSymbolException(String symbol) {
    super();
    this.symbol = symbol;
  }

  public NoSuchSymbolException(String symbol, String detail) {
    super(detail);
    this.symbol = symbol;
  }

  public String getSymbol() {
    return symbol;
  }
}
