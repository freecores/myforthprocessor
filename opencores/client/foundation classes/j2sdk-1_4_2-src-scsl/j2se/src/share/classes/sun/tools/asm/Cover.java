/*
 * @(#)Cover.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

public class Cover {
public int Type;
public long Addr;
public int NumCommand;

     /**
      * Constructor
      */
     public Cover(int type, long addr, int command) {
        Type=type;
        Addr=addr;
        NumCommand=command;
     }
}
