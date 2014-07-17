/**
 * @(#)Tokens.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.parser;

/**
 * An interface that defines codes for Java source tokens
 *  returned from lexical analysis.
 */
public interface Tokens {
    public static final int EOF = 0;
    public static final int ERROR = EOF + 1;
    public static final int IDENTIFIER = ERROR + 1;
    public static final int ABSTRACT = IDENTIFIER + 1;
    public static final int ASSERT = ABSTRACT + 1;
    public static final int BOOLEAN = ASSERT + 1;
    public static final int BREAK = BOOLEAN + 1;
    public static final int BYTE = BREAK + 1;
    public static final int CASE = BYTE + 1;
    public static final int CATCH = CASE + 1;
    public static final int CHAR = CATCH + 1;
    public static final int CLASS = CHAR + 1;
    public static final int CONST = CLASS + 1;
    public static final int CONTINUE = CONST + 1;
    public static final int DEFAULT = CONTINUE + 1;
    public static final int DO = DEFAULT + 1;
    public static final int DOUBLE = DO + 1;
    public static final int ELSE = DOUBLE + 1;
    public static final int EXTENDS = ELSE + 1;
    public static final int FINAL = EXTENDS + 1;
    public static final int FINALLY = FINAL + 1;
    public static final int FLOAT = FINALLY + 1;
    public static final int FOR = FLOAT + 1;
    public static final int GOTO = FOR + 1;
    public static final int IF = GOTO + 1;
    public static final int IMPLEMENTS = IF + 1;
    public static final int IMPORT = IMPLEMENTS + 1;
    public static final int INSTANCEOF = IMPORT + 1;
    public static final int INT = INSTANCEOF + 1;
    public static final int INTERFACE = INT + 1;
    public static final int LONG = INTERFACE + 1;
    public static final int NATIVE = LONG + 1;
    public static final int NEW = NATIVE + 1;
    public static final int PACKAGE = NEW + 1;
    public static final int PRIVATE = PACKAGE + 1;
    public static final int PROTECTED = PRIVATE + 1;
    public static final int PUBLIC = PROTECTED + 1;
    public static final int RETURN = PUBLIC + 1;
    public static final int SHORT = RETURN + 1;
    public static final int STATIC = SHORT + 1;
    public static final int STRICTFP = STATIC + 1;
    public static final int SUPER = STRICTFP + 1;
    public static final int SWITCH = SUPER + 1;
    public static final int SYNCHRONIZED = SWITCH + 1;
    public static final int THIS = SYNCHRONIZED + 1;
    public static final int THROW = THIS + 1;
    public static final int THROWS = THROW + 1;
    public static final int TRANSIENT = THROWS + 1;
    public static final int TRY = TRANSIENT + 1;
    public static final int VOID = TRY + 1;
    public static final int VOLATILE = VOID + 1;
    public static final int WHILE = VOLATILE + 1;
    public static final int INTLITERAL = WHILE + 1;
    public static final int LONGLITERAL = INTLITERAL + 1;
    public static final int FLOATLITERAL = LONGLITERAL + 1;
    public static final int DOUBLELITERAL = FLOATLITERAL + 1;
    public static final int CHARLITERAL = DOUBLELITERAL + 1;
    public static final int STRINGLITERAL = CHARLITERAL + 1;
    public static final int TRUE = STRINGLITERAL + 1;
    public static final int FALSE = TRUE + 1;
    public static final int NULL = FALSE + 1;
    public static final int LPAREN = NULL + 1;
    public static final int RPAREN = LPAREN + 1;
    public static final int LBRACE = RPAREN + 1;
    public static final int RBRACE = LBRACE + 1;
    public static final int LBRACKET = RBRACE + 1;
    public static final int RBRACKET = LBRACKET + 1;
    public static final int SEMI = RBRACKET + 1;
    public static final int COMMA = SEMI + 1;
    public static final int DOT = COMMA + 1;
    public static final int EQ = DOT + 1;
    public static final int GT = EQ + 1;
    public static final int LT = GT + 1;
    public static final int BANG = LT + 1;
    public static final int TILDE = BANG + 1;
    public static final int QUES = TILDE + 1;
    public static final int COLON = QUES + 1;
    public static final int EQEQ = COLON + 1;
    public static final int LTEQ = EQEQ + 1;
    public static final int GTEQ = LTEQ + 1;
    public static final int BANGEQ = GTEQ + 1;
    public static final int AMPAMP = BANGEQ + 1;
    public static final int BARBAR = AMPAMP + 1;
    public static final int PLUSPLUS = BARBAR + 1;
    public static final int SUBSUB = PLUSPLUS + 1;
    public static final int PLUS = SUBSUB + 1;
    public static final int SUB = PLUS + 1;
    public static final int STAR = SUB + 1;
    public static final int SLASH = STAR + 1;
    public static final int AMP = SLASH + 1;
    public static final int BAR = AMP + 1;
    public static final int CARET = BAR + 1;
    public static final int PERCENT = CARET + 1;
    public static final int LTLT = PERCENT + 1;
    public static final int GTGT = LTLT + 1;
    public static final int GTGTGT = GTGT + 1;
    public static final int PLUSEQ = GTGTGT + 1;
    public static final int SUBEQ = PLUSEQ + 1;
    public static final int STAREQ = SUBEQ + 1;
    public static final int SLASHEQ = STAREQ + 1;
    public static final int AMPEQ = SLASHEQ + 1;
    public static final int BAREQ = AMPEQ + 1;
    public static final int CARETEQ = BAREQ + 1;
    public static final int PERCENTEQ = CARETEQ + 1;
    public static final int LTLTEQ = PERCENTEQ + 1;
    public static final int GTGTEQ = LTLTEQ + 1;
    public static final int GTGTGTEQ = GTGTEQ + 1;
    public static final int TokenCount = GTGTGTEQ + 1;
}
