/*
 * @(#)NativeGen.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.corba.se.idl;

// NOTES:

import java.io.PrintWriter;
import java.util.Hashtable;

public interface NativeGen extends Generator
{
  void generate (Hashtable symbolTable, NativeEntry entry, PrintWriter stream);
} // interface NativeGen

