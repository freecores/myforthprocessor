/*
 * @(#)ConstructionException.java	1.4 03/01/23 11:44:44
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.lang.reflect.InvocationTargetException;

public class ConstructionException extends RuntimeException {
  private Exception exception;

  public ConstructionException() {
    super();
  }

  public ConstructionException(String detail) {
    super(detail);
  }

  public ConstructionException(Exception e) {
    this(e.getMessage());
    exception = e;
  }

  public Exception getException() {
    return exception;
  }

  public String toString() {
    String detail = null;

    if (exception != null) {
      detail = exception.toString();
      // Hack to get more useful information
      if (exception instanceof InvocationTargetException) {
        detail = detail + ": " + ((InvocationTargetException) exception).getTargetException().toString();
      }
    } else if (getMessage() != null) {
      detail = getMessage();
    }

    String prefix = "sun.jvm.hotspot.runtime.ConstructionException";

    if (detail != null) {
      return prefix + ": " + detail;
    } else {
      return prefix;
    }
  }
}
