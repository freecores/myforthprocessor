/*
 * @(#)CompilerMember.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javac;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

/**
 * This class is used to represents fields while they are
 * being compiled
 */
final
class CompilerMember implements Comparable {
    MemberDefinition field;
    Assembler asm;
    Object value;
    String name;
    String sig;
    String key;

    CompilerMember(MemberDefinition field, Assembler asm) {
	this.field = field;
	this.asm = asm;
	name = field.getName().toString();
	sig = field.getType().getTypeSignature();
    }

    public int compareTo(Object o) {
	CompilerMember cm = (CompilerMember) o;
	return getKey().compareTo(cm.getKey());
    }

    String getKey() {
	if (key==null) 
	    key = name+sig;
	return key;
    }

}
