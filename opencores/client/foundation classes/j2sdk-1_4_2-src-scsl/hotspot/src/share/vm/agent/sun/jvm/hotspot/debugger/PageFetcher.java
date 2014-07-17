/*
 * @(#)PageFetcher.java	1.3 03/01/23 11:25:49
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

/** This interface specifies how a page is fetched by the PageCache. */

public interface PageFetcher {
  public Page fetchPage(long pageBaseAddress, long numBytes);
}
