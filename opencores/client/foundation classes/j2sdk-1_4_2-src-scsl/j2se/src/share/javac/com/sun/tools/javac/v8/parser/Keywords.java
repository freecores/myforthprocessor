/**
 * @(#)Keywords.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.parser;
import com.sun.tools.javac.v8.util.Context;

import com.sun.tools.javac.v8.util.Log;

import com.sun.tools.javac.v8.util.Name;


/**
 * Map from Name to Tokens and Tokens to String.
 */
public class Keywords implements Tokens {
    private static final Context.Key keywordsKey = new Context.Key();

    public static Keywords instance(Context context) {
        Keywords instance = (Keywords) context.get(keywordsKey);
        if (instance == null)
            instance = new Keywords(context);
        return instance;
    }
    private final Log log;
    private final Name.Table names;

    private Keywords(Context context) {
        super();
        context.put(keywordsKey, this);
        log = Log.instance(context);
        names = Name.Table.instance(context);
        for (int j = 0; j < Tokens.TokenCount; j++)
            tokenName[j] = null;
        enterKeyword("+", PLUS);
        enterKeyword("-", SUB);
        enterKeyword("!", BANG);
        enterKeyword("%", PERCENT);
        enterKeyword("^", CARET);
        enterKeyword("&", AMP);
        enterKeyword("*", STAR);
        enterKeyword("|", BAR);
        enterKeyword("~", TILDE);
        enterKeyword("/", SLASH);
        enterKeyword(">", GT);
        enterKeyword("<", LT);
        enterKeyword("?", QUES);
        enterKeyword(":", COLON);
        enterKeyword("=", EQ);
        enterKeyword("++", PLUSPLUS);
        enterKeyword("--", SUBSUB);
        enterKeyword("==", EQEQ);
        enterKeyword("<=", LTEQ);
        enterKeyword(">=", GTEQ);
        enterKeyword("!=", BANGEQ);
        enterKeyword("<<", LTLT);
        enterKeyword(">>", GTGT);
        enterKeyword(">>>", GTGTGT);
        enterKeyword("+=", PLUSEQ);
        enterKeyword("-=", SUBEQ);
        enterKeyword("*=", STAREQ);
        enterKeyword("/=", SLASHEQ);
        enterKeyword("&=", AMPEQ);
        enterKeyword("|=", BAREQ);
        enterKeyword("^=", CARETEQ);
        enterKeyword("%=", PERCENTEQ);
        enterKeyword("<<=", LTLTEQ);
        enterKeyword(">>=", GTGTEQ);
        enterKeyword(">>>=", GTGTGTEQ);
        enterKeyword("||", BARBAR);
        enterKeyword("&&", AMPAMP);
        enterKeyword("abstract", ABSTRACT);
        enterKeyword("assert", ASSERT);
        enterKeyword("boolean", BOOLEAN);
        enterKeyword("break", BREAK);
        enterKeyword("byte", BYTE);
        enterKeyword("case", CASE);
        enterKeyword("catch", CATCH);
        enterKeyword("char", CHAR);
        enterKeyword("class", CLASS);
        enterKeyword("const", CONST);
        enterKeyword("continue", CONTINUE);
        enterKeyword("default", DEFAULT);
        enterKeyword("do", DO);
        enterKeyword("double", DOUBLE);
        enterKeyword("else", ELSE);
        enterKeyword("extends", EXTENDS);
        enterKeyword("final", FINAL);
        enterKeyword("finally", FINALLY);
        enterKeyword("float", FLOAT);
        enterKeyword("for", FOR);
        enterKeyword("goto", GOTO);
        enterKeyword("if", IF);
        enterKeyword("implements", IMPLEMENTS);
        enterKeyword("import", IMPORT);
        enterKeyword("instanceof", INSTANCEOF);
        enterKeyword("int", INT);
        enterKeyword("interface", INTERFACE);
        enterKeyword("long", LONG);
        enterKeyword("native", NATIVE);
        enterKeyword("new", NEW);
        enterKeyword("package", PACKAGE);
        enterKeyword("private", PRIVATE);
        enterKeyword("protected", PROTECTED);
        enterKeyword("public", PUBLIC);
        enterKeyword("return", RETURN);
        enterKeyword("short", SHORT);
        enterKeyword("static", STATIC);
        enterKeyword("strictfp", STRICTFP);
        enterKeyword("super", SUPER);
        enterKeyword("switch", SWITCH);
        enterKeyword("synchronized", SYNCHRONIZED);
        enterKeyword("this", THIS);
        enterKeyword("throw", THROW);
        enterKeyword("throws", THROWS);
        enterKeyword("transient", TRANSIENT);
        enterKeyword("try", TRY);
        enterKeyword("void", VOID);
        enterKeyword("volatile", VOLATILE);
        enterKeyword("while", WHILE);
        enterKeyword("true", TRUE);
        enterKeyword("false", FALSE);
        enterKeyword("null", NULL);
        key = new byte[maxKey + 1];
        for (int i = 0; i <= maxKey; i++)
            key[i] = IDENTIFIER;
        for (byte j = 0; j < Tokens.TokenCount; j++)
            if (tokenName[j] != null)
                key[tokenName[j].index] = j;
    }

    public int key(Name name) {
        return (name.index > maxKey) ? IDENTIFIER : key[name.index];
    }

    /**
      * Keyword array. Maps name indices to Tokens.
      */
    private final byte[] key;

    /**
     * The number of the last entered keyword.
     */
    private int maxKey = 0;

    /**
     * The names of all tokens.
     */
    private Name[] tokenName = new Name[Tokens.TokenCount];

    public String token2string(int token) {
        switch (token) {
        case IDENTIFIER:
            return log.getLocalizedString("token.identifier");

        case CHARLITERAL:
            return log.getLocalizedString("token.character");

        case STRINGLITERAL:
            return log.getLocalizedString("token.string");

        case INTLITERAL:
            return log.getLocalizedString("token.integer");

        case LONGLITERAL:
            return log.getLocalizedString("token.long-integer");

        case FLOATLITERAL:
            return log.getLocalizedString("token.float");

        case DOUBLELITERAL:
            return log.getLocalizedString("token.double");

        case DOT:
            return "\'.\'";

        case COMMA:
            return "\',\'";

        case SEMI:
            return "\';\'";

        case LPAREN:
            return "\'(\'";

        case RPAREN:
            return "\')\'";

        case LBRACKET:
            return "\'[\'";

        case RBRACKET:
            return "\']\'";

        case LBRACE:
            return "\'{\'";

        case RBRACE:
            return "\'}\'";

        case ERROR:
            return log.getLocalizedString("token.bad-symbol");

        case EOF:
            return log.getLocalizedString("token.end-of-input");

        default:
            return tokenName[token].toJava();

        }
    }

    private void enterKeyword(String s, int token) {
        Name n = names.fromString(s);
        tokenName[token] = n;
        if (n.index > maxKey)
            maxKey = n.index;
    }
}
