/**
 * @(#)FieldDocImpl.java	1.30 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.code.Flags;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import com.sun.tools.javac.v8.code.Symbol.VarSymbol;

import com.sun.tools.javac.v8.code.TypeTags;

import com.sun.tools.javac.v8.tree.Tree.VarDef;

import java.lang.reflect.Modifier;


/**
 * Represents a field in a java class.
 *
 * @see MemberDocImpl
 *
 * @since JDK1.2
 * @author Robert Field
 * @author Neal Gafter (rewrite)
 */
public class FieldDocImpl extends MemberDocImpl implements FieldDoc {
    protected final VarSymbol sym;
    VarDef tree = null;

    /**
     * Constructor.
     */
    public FieldDocImpl(DocEnv env, VarSymbol sym, String rawDocs) {
        super(env, rawDocs);
        this.sym = sym;
    }

    /**
      * Constructor.
      */
    public FieldDocImpl(DocEnv env, VarSymbol sym) {
        this(env, sym, null);
    }

    /**
      * Returns the flags in terms of javac's flags
      */
    protected long getFlags() {
        return sym.flags();
    }

    /**
      * Identify the containing class
      */
    protected ClassSymbol getContainingClass() {
        return sym.enclClass();
    }

    /**
      * Get type of this field.
      */
    public com.sun.javadoc.Type type() {
        return TypeMaker.getType(env, sym.type);
    }

    /**
      * Get the value of a constant field.
      *
      * @return the value of a constant field. The value is
      * automatically wrapped in an object if it has a primitive type.
      * If the field is not constant, returns null.
      */
    public Object constantValue() {
        if (sym.constValue != null)
            env.attr.evalInit(sym);
        Object result = sym.constValue;
        if (result != null && sym.type.tag == TypeTags.BOOLEAN)
            result = Boolean.valueOf(((Integer) result).intValue() != 0);
        return result;
    }

    /**
      * Get the value of a constant field.
      *
      * @return the text of a Java language expression whose value
      * is the value of the constant. The expression uses no identifiers
      * other than primitive literals. If the field is
      * not constant, returns null.
      */
    public String constantValueExpression() {
        Object cb = constantValue();
        if (cb == null)
            return null;
        if (cb instanceof Character)
            return sourceForm(((Character) cb).charValue());
        if (cb instanceof Byte)
            return sourceForm(((Byte) cb).byteValue());
        if (cb instanceof String)
            return sourceForm((String) cb);
        if (cb instanceof Double)
            return sourceForm(((Double) cb).doubleValue(), 'd');
        if (cb instanceof Float)
            return sourceForm(((Float) cb).doubleValue(), 'f');
        if (cb instanceof Long)
            return cb + "l";
        return cb.toString();
    }

    private String sourceForm(double v, char suffix) {
        if (Double.isNaN(v))
            return "0" + suffix + "/0" + suffix;
        if (v == Double.POSITIVE_INFINITY)
            return "1" + suffix + "/0" + suffix;
        if (v == Double.NEGATIVE_INFINITY)
            return "-1" + suffix + "/0" + suffix;
        return "" + v + suffix;
    }

    private String sourceForm(char c) {
        StringBuffer buf = new StringBuffer(8);
        buf.append('\'');
        sourceChar(c, buf);
        buf.append('\'');
        return buf.toString();
    }

    private String sourceForm(byte c) {
        return "0x" + Integer.toString(c & 255, 16);
    }

    private String sourceForm(String s) {
        StringBuffer buf = new StringBuffer(s.length() + 5);
        buf.append('\"');
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            sourceChar(c, buf);
        }
        buf.append('\"');
        return buf.toString();
    }

    private void sourceChar(char c, StringBuffer buf) {
        switch (c) {
        case '\b':
            buf.append("\\b");
            return;

        case '\t':
            buf.append("\\t");
            return;

        case '\n':
            buf.append("\\n");
            return;

        case '\f':
            buf.append("\\f");
            return;

        case '\r':
            buf.append("\\r");
            return;

        case '\"':
            buf.append("\\\"");
            return;

        case '\'':
            buf.append("\\\'");
            return;

        case '\\':
            buf.append("\\\\");
            return;

        default:
            if (isPrintableAscii(c)) {
                buf.append(c);
                return;
            }
            unicodeEscape(c, buf);
            return;

        }
    }

    private void unicodeEscape(char c, StringBuffer buf) {
        final String chars = "0123456789abcdef";
        buf.append("\\u");
        buf.append(chars.charAt(15 & (c >> 12)));
        buf.append(chars.charAt(15 & (c >> 8)));
        buf.append(chars.charAt(15 & (c >> 4)));
        buf.append(chars.charAt(15 & (c >> 0)));
    }

    private boolean isPrintableAscii(char c) {
        return c >= ' ' && c <= '~';
    }

    /**
      * Return true if this field is included in the active set.
      */
    public boolean isIncluded() {
        return containingClass().isIncluded() && env.shouldDocument(sym);
    }

    /**
      * Is this Doc item a field?
      *
      * @return true is it represents a field, and it does.
      */
    public boolean isField() {
        return true;
    }

    /**
      * Return true if this field is transient
      */
    public boolean isTransient() {
        return Modifier.isTransient(getModifiers());
    }

    /**
      * Return true if this field is volatile
      */
    public boolean isVolatile() {
        return Modifier.isVolatile(getModifiers());
    }

    /**
      *
      * Returns true if this field was synthesized by the compiler.
      */
    public boolean isSynthetic() {
        return ((sym.flags() & Flags.SYNTHETIC) != 0);
    }

    /**
      * Return the serialField tags in this FieldDocImpl item.
      *
      * @return an array of <tt>SerialFieldTagImpl<\tt> containing all
      *         <code>&#64serialField<\code> tags.
      */
    public SerialFieldTag[] serialFieldTags() {
        return comment().serialFieldTags();
    }

    public String name() {
        return sym.name.toString();
    }

    public String qualifiedName() {
        return sym.enclClass().fullName().toString() + '.' + name();
    }

    /**
      * Return the source position of the entity, or null if
      * no position is available.
      */
    public SourcePosition position() {
        if (sym.enclClass().sourcefile == null)
            return null;
        return SourcePositionImpl.make(sym.enclClass().sourcefile.toString(),
                (tree == null) ? 0 : tree.pos);
    }
}
