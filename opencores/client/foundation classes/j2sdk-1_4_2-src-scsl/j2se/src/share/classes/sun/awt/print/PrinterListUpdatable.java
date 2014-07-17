/*
 * @(#)PrinterListUpdatable.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.print;

/**
 * PrinterListUpdatable is used by the PrintDialog to query asynchronously the
 * list of available printers. A synchronous design would be impractical on
 * a system which does not maintain a local list of all network printers.
 */
public interface PrinterListUpdatable {
    void updatePrinterList(String[] sortedNames);
}
