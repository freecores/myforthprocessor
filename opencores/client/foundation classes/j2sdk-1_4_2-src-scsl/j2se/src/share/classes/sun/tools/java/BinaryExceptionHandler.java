/*
 * @(#)BinaryExceptionHandler.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

/**
 * A single exception handler.  This class hangs off BinaryCode.
 */

public class BinaryExceptionHandler {
    public int startPC;		
    public int endPC;
    public int handlerPC;
    public ClassDeclaration exceptionClass;

    BinaryExceptionHandler(int start, int end, 
			   int handler, ClassDeclaration xclass) {
	startPC = start;
	endPC = end;
	handlerPC = handler;
	exceptionClass = xclass;
    }
}
