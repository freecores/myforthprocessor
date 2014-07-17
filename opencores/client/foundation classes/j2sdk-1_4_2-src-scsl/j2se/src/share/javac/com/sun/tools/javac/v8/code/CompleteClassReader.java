/**
 * @(#)CompleteClassReader.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.Type.*;

import com.sun.tools.javac.v8.code.Symbol.*;


/**
 * This class contains some minor modifications wrt ClassReader
 *  which read complete constant pool and code sections with the class that is read in.
 *  Usually, these sections are omitted when reading class files.
 */
public class CompleteClassReader extends ClassReader {

    public CompleteClassReader(Context context) {
        super(context, false);
    }

    /**
      * As in ClassReader except that all of constant pool is immediately read in.
      */
    void indexPool() {
        super.indexPool();
        for (int i = 1; i < poolObj.length; i++)
            readPool(i);
    }

    /**
      * As in ClassReader except that constant pool is stored in class `c'.
      */
    void readClass(ClassSymbol c) {
        c.pool = new Pool(poolObj.length, poolObj);
        super.readClass(c);
    }

    /**
      * As in ClassReader except that code is read in and entered
      *  into method symbol.
      */
    void readMemberAttr(Symbol sym, Name attrName, int attrLen) {
        if (attrName == names.Code) {
            ((MethodSymbol) sym).code = readCode(sym);
        } else {
            super.readMemberAttr(sym, attrName, attrLen);
        }
    }

    /**
      * Read code block.
      */
    Code readCode(Symbol owner) {
        Code code = new Code(false, true, true, null);
        code.max_stack = nextChar();
        code.max_locals = nextChar();
        code.cp = nextInt();
        code.code = new byte[code.cp];
        System.arraycopy(buf, bp, code.code, 0, code.cp);
        bp += code.cp;
        char ncatches = nextChar();
        for (int i = 0; i < ncatches; i++) {
            code.addCatch(nextChar(), nextChar(), nextChar(), nextChar());
        }
        char ac = nextChar();
        for (int i = 0; i < ac; i++) {
            Name attrName = readName(nextChar());
            int attrLen = nextInt();
            if (attrName == names.LineNumberTable) {
                char nlines = nextChar();
                for (int j = 0; j < nlines; j++) {
                    code.addLineNumber(nextChar(), nextChar());
                }
            } else if (attrName == names.LocalVariableTable) {
                int count = nextChar();
                for (int j = 0; j < count; j++) {
                    char start_pc = nextChar();
                    char length = nextChar();
                    VarSymbol sym = new VarSymbol(0, readName(nextChar()),
                            readType(nextChar()), owner);
                    sym.adr = nextChar();
                    Code.LocalVar v = new Code.LocalVar(sym);
                    v.start_pc = start_pc;
                    v.length = length;
                    code.putVar(v);
                }
            } else {
                unrecogized(attrName);
                bp = bp + attrLen;
            }
        }
        return code;
    }
}
