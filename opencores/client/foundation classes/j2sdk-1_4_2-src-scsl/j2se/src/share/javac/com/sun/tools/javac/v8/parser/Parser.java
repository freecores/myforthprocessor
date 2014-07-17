/**
 * @(#)Parser.java	1.39 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.parser;
import com.sun.tools.javac.v8.tree.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.tree.Tree.*;


/**
 * The parser maps a token sequence into an abstract syntax
 *  tree. It operates by recursive descent, with code derived
 *  systematically from an LL(1) grammar. For efficiency reasons, an
 *  operator precedence scheme is used for parsing binary operation
 *  expressions.
 */
public class Parser implements Tokens {

    /**
     * The number of precedence levels of infix operators.
     */
    private static final int infixPrecedenceLevels = 10;

    /**
     * The scanner used for lexical analysis.
     */
    private Scanner S;

    /**
     * The factory to be used for abstract syntax tree construction.
     */
    private TreeMaker F;

    /**
     * The log to be used for error diagnostics.
     */
    private Log log;

    /**
     * The keyword table.
     */
    private Keywords keywords;

    /**
     * The Source language setting.
     */
    private Source source;

    /**
     * The name table.
     */
    private Name.Table names;

    /**
     * Construct a parser from a given scanner, tree factory and log.
     *  @param genEndPos Should endPositions be generated?
     */
    public Parser(Context context, Scanner S, boolean keepDocComments,
            boolean genEndPos) {
        super();
        this.S = S;
        this.F = TreeMaker.instance(context);
        this.log = Log.instance(context);
        this.names = Name.Table.instance(context);
        this.keywords = Keywords.instance(context);
        this.keepDocComments = keepDocComments;
        this.source = Source.instance(context);
        this.allowAsserts = source.ordinal >= Source.JDK1_4.ordinal;
        this.genEndPos = genEndPos;
        if (keepDocComments)
            docComments = new Hashtable();
        if (genEndPos)
            endPositions = new Hashtable();
    }

    /**
      * Switch: should we keep docComments?
      */
    boolean keepDocComments;

    /**
     * Switch: should we recognize assert statements, or just give a warning?
     */
    boolean allowAsserts;

    /**
     * Switch: should we store the ending positions?
     */
    boolean genEndPos;

    /**
     * When terms are parsed, the mode determines which is expected:
     *     mode = EXPR        : an expression
     *     mode = TYPE        : a type
     *     mode = NOPARAMS    : no parameters allowed for type
     */
    static final int EXPR = 1;
    static final int TYPE = 2;
    static final int NOPARAMS = 4;

    /**
     * The current mode.
     */
    private int mode = 0;

    /**
     * The mode of the term that was parsed last.
     */
    private int lastmode = 0;
    static Tree errorTree = new Tree.Erroneous();

    /**
     * Skip forward until a suitable stop token is found.
     */
    private void skip() {
        int nbraces = 0;
        int nparens = 0;
        while (true) {
            switch (S.token) {
            case EOF:

            case CLASS:

            case INTERFACE:
                return;

            case SEMI:
                if (nbraces == 0 && nparens == 0)
                    return;
                break;

            case RBRACE:
                if (nbraces == 0)
                    return;
                nbraces--;
                break;

            case RPAREN:
                if (nparens > 0)
                    nparens--;
                break;

            case LBRACE:
                nbraces++;
                break;

            case LPAREN:
                nparens++;
                break;

            default:

            }
            S.nextToken();
        }
    }

    /**
      * Generate a syntax error at given position using the given argument
      *  unless one was already reported at the same position, then skip.
      */
    private Tree syntaxError(int pos, String key, String arg) {
        if (pos != S.errPos)
            log.error(pos, key, arg);
        skip();
        S.errPos = pos;
        return errorTree;
    }

    /**
      * Generate a syntax error at given position unless one was already
      *  reported at the same position, then skip.
      */
    private Tree syntaxError(int pos, String key) {
        return syntaxError(pos, key, null);
    }

    /**
      * Generate a syntax error at current position unless one was already reported
      *  at the same position, then skip.
      */
    private Tree syntaxError(String key) {
        return syntaxError(S.pos, key, null);
    }

    /**
      * Generate a syntax error at current position unless one was already reported
      *  at the same position, then skip.
      */
    private Tree syntaxError(String key, String arg) {
        return syntaxError(S.pos, key, arg);
    }

    /**
      * If next input token matches given token, skip it, otherwise report an error.
      */
    private void accept(int token) {
        if (S.token == token) {
            S.nextToken();
        } else {
            int pos = Position.line(S.pos) > Position.line(S.prevEndPos + 1) ?
                    S.prevEndPos + 1 : S.pos;
            syntaxError(pos, "expected", keywords.token2string(token));
            if (S.token == token)
                S.nextToken();
        }
    }

    /**
      * Report an illegal start of expression/type error at given position.
      */
    Tree illegal(int pos) {
        if ((mode & EXPR) != 0)
            return syntaxError(pos, "illegal.start.of.expr");
        else
            return syntaxError(pos, "illegal.start.of.type");
    }

    /**
      * Report an illegal start of expression/type error at current position.
      */
    Tree illegal() {
        return illegal(S.pos);
    }

    /**
      * A hashtable to store all documentation comments
      *  indexed by the tree nodes they refer to.
      *  defined only if option flag keepDocComment is set.
      */
    Hashtable docComments;

    /**
     * Make an entry into docComments hashtable,
     *  provided flag keepDocComments is set and given doc comment is non-null.
     *  @param tree   The tree to be used as index in the hashtable
     *  @param dc     The doc comment to associate with the tree, or null.
     */
    void attach(Tree tree, String dc) {
        if (keepDocComments && dc != null) {
            docComments.put(tree, dc);
        }
    }

    /**
      * A hashtable to store ending positions
      *  of source ranges indexed by the tree nodes.
      *  Defined only if option flag genEndPos is set.
      */
    Hashtable endPositions;

    /**
     * Make an entry into endPositions hashtable, provided flag
     *  genEndPos is set. Note that this method is usually hand-inlined.
     *  @param tree   The tree to be used as index in the hashtable
     *  @param endPos The ending position to associate with the tree.
     */
    void storeEnd(Tree tree, int endpos) {
        if (genEndPos)
            endPositions.put(tree, new Integer(endpos));
    }

    /**
      * Ident = IDENTIFIER
      */
    Name ident() {
        if (S.token == IDENTIFIER) {
            Name name = S.name;
            S.nextToken();
            return name;
        }
        if (S.token == ASSERT) {
            if (allowAsserts) {
                log.error(S.pos, "assert.as.identifier");
                S.nextToken();
                return names.error;
            } else {
                log.warning(S.pos, "assert.as.identifier");
                Name name = S.name;
                S.nextToken();
                return name;
            }
        } else {
            accept(IDENTIFIER);
            return names.error;
        }
    }

    /**
      * Qualident = Ident { DOT Ident }
      */
    Tree qualident() {
        Tree t = F.at(S.pos).Ident(ident());
        while (S.token == DOT) {
            int pos = S.pos;
            S.nextToken();
            t = F.at(pos).Select(t, ident());
        }
        return t;
    }

    /**
      * Literal =
      *	   INTLITERAL
      *	 | LONGLITERAL
      *	 | FLOATLITERAL
      *	 | DOUBLELITERAL
      *	 | CHARLITERAL
      *	 | STRINGLITERAL
      *       | TRUE
      *       | FALSE
      *       | NULL
      */
    Tree literal(Name prefix) {
        int pos = S.pos;
        Tree t = errorTree;
        switch (S.token) {
        case INTLITERAL:
            try {
                t = F.at(pos).Literal(Type.INT,
                        new Integer(Convert.string2int(strval(prefix), S.radix)));
            } catch (NumberFormatException ex) {
                log.error(S.pos, "int.number.too.large", strval(prefix));
            }
            break;

        case LONGLITERAL:
            try {
                t = F.at(pos).Literal(Type.LONG,
                        new Long(Convert.string2long(strval(prefix), S.radix)));
            } catch (NumberFormatException ex) {
                log.error(S.pos, "int.number.too.large", strval(prefix));
            }
            break;

        case FLOATLITERAL:
            {
                Float n = Float.valueOf(S.stringVal());
                if (n.floatValue() == 0.0F && !isZero(S.stringVal()))
                    log.error(S.pos, "fp.number.too.small");
                else if (n.floatValue() == Float.POSITIVE_INFINITY)
                    log.error(S.pos, "fp.number.too.large");
                else
                    t = F.at(pos).Literal(Type.FLOAT, n);
                break;
            }

        case DOUBLELITERAL:
            {
                Double n = Double.valueOf(S.stringVal());
                if (n.doubleValue() == 0.0 && !isZero(S.stringVal()))
                    log.error(S.pos, "fp.number.too.small");
                else if (n.doubleValue() == Double.POSITIVE_INFINITY)
                    log.error(S.pos, "fp.number.too.large");
                else
                    t = F.at(pos).Literal(Type.DOUBLE, n);
                break;
            }

        case CHARLITERAL:
            t = F.at(pos).Literal(Type.CHAR, new Integer(S.stringVal().charAt(0)));
            break;

        case STRINGLITERAL:
            t = F.at(pos).Literal(Type.CLASS, S.stringVal());
            break;

        case TRUE:

        case FALSE:

        case NULL:
            t = F.at(pos).Ident(S.name);
            break;

        default:
            assert false;

        }
        S.nextToken();
        return t;
    }

    boolean isZero(String s) {
        char[] cs = s.toCharArray();
        int i = 0;
        while (i < cs.length && (cs[i] == '0' || cs[i] == '.'))
            i++;
        return !(i < cs.length && ('1' <= cs[i] && cs[i] <= '9'));
    }

    String strval(Name prefix) {
        String s = S.stringVal();
        return (prefix.len == 0) ? s : prefix + s;
    }

    /**
      * terms can be either expressions or types.
      */
    Tree expression() {
        return term(EXPR);
    }

    Tree type() {
        return term(TYPE);
    }

    Tree term(int newmode) {
        int prevmode = mode;
        mode = newmode;
        Tree t = term();
        lastmode = mode;
        mode = prevmode;
        return t;
    }

    /**
      *  Expression = Expression1 [ExpressionRest]
      *  ExpressionRest = [AssignmentOperator Expression1]
      *  AssignmentOperator = "=" | "+=" | "-=" | "*=" | "/=" |  "&=" | "|=" | "^=" |
      *                       "%=" | "<<=" | ">>=" | ">>>="
      *  Type = Type1
      *  TypeNoParams = TypeNoParams1
      *  StatementExpression = Expression
      *  ConstantExpression = Expression
      */
    Tree term() {
        Tree t = term1();
        if ((mode & EXPR) != 0 && S.token == EQ || PLUSEQ <= S.token &&
                S.token <= GTGTGTEQ)
            return termRest(t);
        else
            return t;
    }

    Tree termRest(Tree t) {
        switch (S.token) {
        case EQ:
            {
                int pos = S.pos;
                S.nextToken();
                mode = EXPR;
                Tree t1 = term();
                return F.at(pos).Assign(t, t1);
            }

        case PLUSEQ:

        case SUBEQ:

        case STAREQ:

        case SLASHEQ:

        case PERCENTEQ:

        case AMPEQ:

        case BAREQ:

        case CARETEQ:

        case LTLTEQ:

        case GTGTEQ:

        case GTGTGTEQ:
            int pos = S.pos;
            int token = S.token;
            S.nextToken();
            mode = EXPR;
            Tree t1 = term();
            return F.at(pos).Assignop(optag(token), t, t1);

        default:
            return t;

        }
    }

    /**
      * Expression1   = Expression2 [Expression1Rest]
      *  Type1         = Type2
      *  TypeNoParams1 = TypeNoParams2
      */
    Tree term1() {
        Tree t = term2();
        if ((mode & EXPR) != 0 & S.token == QUES) {
            mode = EXPR;
            return term1Rest(t);
        } else {
            return t;
        }
    }

    /**
      * Expression1Rest = ["?" Expression ":" Expression1]
      */
    Tree term1Rest(Tree t) {
        if (S.token == QUES) {
            int pos = S.pos;
            S.nextToken();
            Tree t1 = term();
            accept(COLON);
            Tree t2 = term1();
            return F.at(pos).Conditional(t, t1, t2);
        } else {
            return t;
        }
    }

    /**
      * Expression2   = Expression3 [Expression2Rest]
      *  Type2         = Type3
      *  TypeNoParams2 = TypeNoParams3
      */
    Tree term2() {
        Tree t = term3();
        if ((mode & EXPR) != 0 && prec(S.token) >= TreeInfo.orPrec) {
            mode = EXPR;
            return term2Rest(t, TreeInfo.orPrec);
        } else {
            return t;
        }
    }

    Tree term2Rest(Tree t, int minprec) {
        List savedOd = odStackSupply.elems;
        Tree[] odStack = newOdStack();
        List savedOp = opStackSupply.elems;
        int[] opStack = newOpStack();
        int top = 0;
        odStack[0] = t;
        int startPos = S.pos;
        int topOp = ERROR;
        while (prec(S.token) >= minprec) {
            opStack[top] = topOp;
            top++;
            topOp = S.token;
            int pos = S.pos;
            S.nextToken();
            odStack[top] = topOp == INSTANCEOF ? type() : term3();
            while (top > 0 && prec(topOp) >= prec(S.token)) {
                odStack[top - 1] = makeOp(pos, topOp, odStack[top - 1], odStack[top]);
                top--;
                topOp = opStack[top];
            }
        }
        assert top == 0;
        t = odStack[0];
        if (t.tag == Tree.PLUS) {
            StringBuffer buf = foldStrings(t);
            if (buf != null) {
                t = F.at(startPos).Literal(Type.CLASS, buf.toString());
            }
        }
        odStackSupply.elems = savedOd;
        opStackSupply.elems = savedOp;
        return t;
    }

    /**
      * Construct a binary or type test node.
      */
    private Tree makeOp(int pos, int topOp, Tree od1, Tree od2) {
        if (topOp == INSTANCEOF) {
            return F.at(pos).TypeTest(od1, od2);
        } else {
            return F.at(pos).Binary(optag(topOp), od1, od2);
        }
    }

    /**
      * The empty List<String>.
      */
    private static List emptyStringList = new List();

    /**
     * If tree is a concatenation of string literals, replace it
     *  by a single literal representing the concatenated string.
     */
    private static StringBuffer foldStrings(Tree tree) {
        List buf = emptyStringList;
        while (true) {
            if (tree.tag == Tree.LITERAL) {
                Literal lit = (Literal) tree;
                if (lit.typetag == Type.CLASS) {
                    StringBuffer sbuf = new StringBuffer((String) lit.value);
                    while (buf.nonEmpty()) {
                        sbuf.append((String) buf.head);
                        buf = buf.tail;
                    }
                    return sbuf;
                }
            } else if (tree.tag == Tree.PLUS) {
                Binary op = (Binary) tree;
                if (op.rhs.tag == Tree.LITERAL) {
                    Literal lit = (Literal) op.rhs;
                    if (lit.typetag == Type.CLASS) {
                        buf = buf.prepend((String) lit.value);
                        tree = op.lhs;
                        continue;
                    }
                }
            }
            return null;
        }
    }

    /**
      * optimization: To save allocating a new operand/operator stack
      *  for every binary operation, we use supplys.
      */
    ListBuffer odStackSupply = new ListBuffer();
    ListBuffer opStackSupply = new ListBuffer();

    private Tree[] newOdStack() {
        if (odStackSupply.elems == odStackSupply.last)
            odStackSupply.append(new Tree[infixPrecedenceLevels + 1]);
        Tree[] odStack = (Tree[]) odStackSupply.elems.head;
        odStackSupply.elems = odStackSupply.elems.tail;
        return odStack;
    }

    private int[] newOpStack() {
        if (opStackSupply.elems == opStackSupply.last)
            opStackSupply.append(new int[infixPrecedenceLevels + 1]);
        int[] opStack = (int[]) opStackSupply.elems.head;
        opStackSupply.elems = opStackSupply.elems.tail;
        return opStack;
    }

    /**
      * Expression3    = PrefixOp Expression3
      *                 | "(" Expr | TypeNoParams ")" Expression3
      *                 | Primary {Selector} {PostfixOp}
      *  Primary        = "(" Expression ")"
      *                 | THIS [Arguments]
      *                 | SUPER SuperSuffix
      *                 | Literal
      *                 | NEW Creator
      *                 | Ident { "." Ident }
      *                   [ "[" ( "]" BracketsOpt "." CLASS | Expression "]" )
      *                   | Arguments
      *                   | "." ( CLASS | THIS | SUPER Arguments | NEW InnerCreator )
      *                   ]
      *                 | BasicType BracketsOpt "." CLASS
      *  PrefixOp       = "++" | "--" | "!" | "~" | "+" | "-"
      *  PostfixOp      = "++" | "--"
      *  Type3          = Ident { "." Ident } [TypeArguments] {TypeSelector} BracketsOpt
      *                 | BasicType
      *  TypeNoParams3  = Ident { "." Ident } BracketsOpt
      *  Selector       = "." Ident [Arguments]
      *                 | "." THIS
      *                 | "." SUPER SuperSuffix
      *                 | "." NEW InnerCreator
      *                 | "[" Expression "]"
      *  TypeSelector   = "." Ident [TypeArguments]
      *  SuperSuffix    = Arguments | "." Ident [Arguments]
      */
    Tree term3() {
        int pos = S.pos;
        Tree t;
        switch (S.token) {
        case PLUSPLUS:

        case SUBSUB:

        case BANG:

        case TILDE:

        case PLUS:

        case SUB:
            if ((mode & EXPR) != 0) {
                mode = EXPR;
                int token = S.token;
                S.nextToken();
                if (token == SUB &&
                        (S.token == INTLITERAL || S.token == LONGLITERAL) &&
                        S.radix == 10) {
                    t = literal(names.hyphen);
                } else {
                    t = term3();
                    return F.at(pos).Unary(unoptag(token), t);
                }
            } else
                return illegal();
            break;

        case LPAREN:
            if ((mode & EXPR) != 0) {
                S.nextToken();
                mode = EXPR | TYPE | NOPARAMS;
                t = term3();
                t = termRest(term1Rest(term2Rest(t, TreeInfo.orPrec)));
                accept(RPAREN);
                lastmode = mode;
                mode = EXPR;
                if ((lastmode & EXPR) == 0) {
                    Tree t1 = term3();
                    return F.at(pos).TypeCast(t, t1);
                } else if ((lastmode & TYPE) != 0) {
                    switch (S.token) {
                    case BANG:

                    case TILDE:

                    case LPAREN:

                    case THIS:

                    case SUPER:

                    case INTLITERAL:

                    case LONGLITERAL:

                    case FLOATLITERAL:

                    case DOUBLELITERAL:

                    case CHARLITERAL:

                    case STRINGLITERAL:

                    case TRUE:

                    case FALSE:

                    case NULL:

                    case NEW:

                    case IDENTIFIER:

                    case ASSERT:

                    case BYTE:

                    case SHORT:

                    case CHAR:

                    case INT:

                    case LONG:

                    case FLOAT:

                    case DOUBLE:

                    case BOOLEAN:

                    case VOID:
                        Tree t1 = term3();
                        return F.at(pos).TypeCast(t, t1);

                    }
                }
            } else
                return illegal();
            t = F.at(pos).Parens(t);
            break;

        case THIS:
            if ((mode & EXPR) != 0) {
                mode = EXPR;
                t = F.at(pos).Ident(names._this);
                S.nextToken();
                t = argumentsOpt(t);
            } else
                return illegal();
            break;

        case SUPER:
            if ((mode & EXPR) != 0) {
                mode = EXPR;
                t = superSuffix(F.at(pos).Ident(names._super));
            } else
                return illegal();
            break;

        case INTLITERAL:

        case LONGLITERAL:

        case FLOATLITERAL:

        case DOUBLELITERAL:

        case CHARLITERAL:

        case STRINGLITERAL:

        case TRUE:

        case FALSE:

        case NULL:
            if ((mode & EXPR) != 0) {
                mode = EXPR;
                t = literal(names.empty);
            } else
                return illegal();
            break;

        case NEW:
            if ((mode & EXPR) != 0) {
                mode = EXPR;
                S.nextToken();
                t = creator(pos);
            } else
                return illegal();
            break;

        case IDENTIFIER:

        case ASSERT:
            t = F.at(S.pos).Ident(ident());
            loop:
            while (true) {
                pos = S.pos;
                switch (S.token) {
                case LBRACKET:
                    S.nextToken();
                    if (S.token == RBRACKET) {
                        S.nextToken();
                        t = bracketsSuffix(bracketsOpt(F.at(pos).TypeArray(t)));
                    } else {
                        if ((mode & EXPR) != 0) {
                            mode = EXPR;
                            Tree t1 = term();
                            t = F.at(pos).Indexed(t, t1);
                        }
                        accept(RBRACKET);
                    }
                    break loop;

                case LPAREN:
                    if ((mode & EXPR) != 0) {
                        mode = EXPR;
                        t = arguments(t);
                    }
                    break loop;

                case DOT:
                    S.nextToken();
                    if ((mode & EXPR) != 0) {
                        switch (S.token) {
                        case CLASS:
                            mode = EXPR;
                            t = F.at(pos).Select(t, names._class);
                            S.nextToken();
                            break loop;

                        case THIS:
                            mode = EXPR;
                            t = F.at(pos).Select(t, names._this);
                            S.nextToken();
                            break loop;

                        case SUPER:
                            mode = EXPR;
                            t = superSuffix(F.at(pos).Select(t, names._super));
                            break loop;

                        case NEW:
                            mode = EXPR;
                            int pos1 = S.pos;
                            S.nextToken();
                            t = innerCreator(pos1, t);
                            break loop;

                        }
                    }
                    t = F.at(pos).Select(t, ident());
                    break;

                default:
                    break loop;

                }
            }
            break;

        case BYTE:

        case SHORT:

        case CHAR:

        case INT:

        case LONG:

        case FLOAT:

        case DOUBLE:

        case BOOLEAN:
            t = bracketsSuffix(bracketsOpt(basicType()));
            break;

        case VOID:
            if ((mode & EXPR) != 0) {
                S.nextToken();
                if (S.token == DOT) {
                    t = bracketsSuffix(F.at(pos).TypeIdent(Type.VOID));
                } else {
                    return illegal(pos);
                }
            } else {
                return illegal();
            }
            break;

        default:
            return illegal();

        }
        while (true) {
            int pos1 = S.pos;
            if (S.token == LBRACKET) {
                S.nextToken();
                if (S.token == RBRACKET && (mode & TYPE) != 0) {
                    mode = TYPE;
                    S.nextToken();
                    return bracketsOpt(F.at(pos1).TypeArray(t));
                } else {
                    if ((mode & EXPR) != 0) {
                        mode = EXPR;
                        Tree t1 = term();
                        t = F.at(pos1).Indexed(t, t1);
                    }
                    accept(RBRACKET);
                }
            } else if (S.token == DOT) {
                S.nextToken();
                if (S.token == SUPER && (mode & EXPR) != 0) {
                    mode = EXPR;
                    t = F.at(pos).Select(t, names._super);
                    S.nextToken();
                    t = arguments(t);
                } else if (S.token == NEW && (mode & EXPR) != 0) {
                    mode = EXPR;
                    int pos2 = S.pos;
                    S.nextToken();
                    t = innerCreator(pos2, t);
                } else {
                    t = argumentsOpt(F.at(pos).Select(t, ident()));
                }
            } else {
                break;
            }
        }
        while ((S.token == PLUSPLUS || S.token == SUBSUB) && (mode & EXPR) != 0) {
            mode = EXPR;
            t = F.at(S.pos).Unary(S.token == PLUSPLUS ? Tree.POSTINC :
                    Tree.POSTDEC, t);
            S.nextToken();
        }
        if (genEndPos)
            endPositions.put(t, new Integer(S.prevEndPos));
        return t;
    }

    /**
      * SuperSuffix = Arguments | "." Ident [Arguments]
      */
    Tree superSuffix(Tree t) {
        S.nextToken();
        if (S.token == LPAREN) {
            t = arguments(t);
        } else {
            int pos = S.pos;
            accept(DOT);
            t = argumentsOpt(F.at(pos).Select(t, ident()));
        }
        return t;
    }

    /**
      * BasicType = BYTE | SHORT | CHAR | INT | LONG | FLOAT | DOUBLE | BOOLEAN
      */
    Tree basicType() {
        Tree t = F.at(S.pos).TypeIdent(typetag(S.token));
        S.nextToken();
        return t;
    }

    /**
      * ArgumentsOpt = [ Arguments ]
      */
    Tree argumentsOpt(Tree t) {
        if ((mode & EXPR) != 0 && S.token == LPAREN) {
            mode = EXPR;
            return arguments(t);
        } else {
            return t;
        }
    }

    /**
      * Arguments = "(" [Expression { COMMA Expression }] ")"
      */
    List arguments() {
        int pos = S.pos;
        ListBuffer args = new ListBuffer();
        if (S.token == LPAREN) {
            S.nextToken();
            if (S.token != RPAREN) {
                args.append(expression());
                while (S.token == COMMA) {
                    S.nextToken();
                    args.append(expression());
                }
            }
            accept(RPAREN);
        } else {
            syntaxError(S.pos, "expected", keywords.token2string(LPAREN));
        }
        return args.toList();
    }

    Tree arguments(Tree t) {
        int pos = S.pos;
        List args = arguments();
        return F.at(pos).Apply(t, args);
    }

    /**
      * BracketsOpt = {"[" "]"}
      */
    private Tree bracketsOpt(Tree t) {
        while (S.token == LBRACKET) {
            int pos = S.pos;
            S.nextToken();
            accept(RBRACKET);
            t = F.at(pos).TypeArray(t);
        }
        return t;
    }

    /**
      * BracketsSuffixExpr = "." CLASS
      *  BracketsSuffixType =
      */
    Tree bracketsSuffix(Tree t) {
        if ((mode & EXPR) != 0 && S.token == DOT) {
            mode = EXPR;
            int pos = S.pos;
            S.nextToken();
            accept(CLASS);
            t = F.at(pos).Select(t, names._class);
        } else if ((mode & TYPE) != 0) {
            mode = TYPE;
        } else {
            syntaxError(S.pos, "dot.class.expected");
        }
        return t;
    }

    /**
      * Creator = Qualident [TypeArguments] ( ArrayCreatorRest | ClassCreatorRest )
      */
    Tree creator(int newpos) {
        switch (S.token) {
        case BYTE:

        case SHORT:

        case CHAR:

        case INT:

        case LONG:

        case FLOAT:

        case DOUBLE:

        case BOOLEAN:
            return arrayCreatorRest(newpos, basicType());

        default:
            Tree t = qualident();
            if (S.token == LBRACKET)
                return arrayCreatorRest(newpos, t);
            else if (S.token == LPAREN)
                return classCreatorRest(newpos, null, t);
            else
                return syntaxError("left-paren.or.left-square-bracket.expected");

        }
    }

    /**
      * InnerCreator = Ident [TypeArguments] ClassCreatorRest
      */
    Tree innerCreator(int newpos, Tree encl) {
        Tree t = F.at(S.pos).Ident(ident());
        return classCreatorRest(newpos, encl, t);
    }

    /**
      * ArrayCreatorRest = "[" ( "]" BracketsOpt ArrayInitializer
      *                         | Expression "]" {"[" Expression "]"} BracketsOpt )
      */
    Tree arrayCreatorRest(int newpos, Tree elemtype) {
        accept(LBRACKET);
        if (S.token == RBRACKET) {
            S.nextToken();
            elemtype = bracketsOpt(elemtype);
            if (S.token == LBRACE) {
                return arrayInitializer(elemtype);
            } else {
                syntaxError(S.pos, "array.dimension.missing");
                return errorTree;
            }
        } else {
            ListBuffer dims = new ListBuffer();
            dims.append(expression());
            accept(RBRACKET);
            while (S.token == LBRACKET) {
                int pos = S.pos;
                S.nextToken();
                if (S.token == RBRACKET) {
                    S.nextToken();
                    elemtype = bracketsOpt(F.at(pos).TypeArray(elemtype));
                } else {
                    dims.append(expression());
                    accept(RBRACKET);
                }
            }
            return F.at(newpos).NewArray(elemtype, dims.toList(), null);
        }
    }

    /**
      * ClassCreatorRest = Arguments [ClassBody]
      */
    Tree classCreatorRest(int newpos, Tree encl, Tree t) {
        List args = arguments();
        ClassDef body = null;
        if (S.token == LBRACE)
            body = F.at(S.pos).ClassDef(0, names.empty, TypeParameter.emptyList,
                    null, Tree.emptyList, classOrInterfaceBody(names.empty, false));
        return F.at(newpos).NewClass(encl, t, args, body);
    }

    /**
      * ArrayInitializer = "{" [VariableInitializer {"," VariableInitializer}] [","] "}"
      */
    Tree arrayInitializer(Tree t) {
        int pos = S.pos;
        accept(LBRACE);
        ListBuffer elems = new ListBuffer();
        if (S.token == COMMA) {
            S.nextToken();
        } else if (S.token != RBRACE) {
            elems.append(variableInitializer());
            while (S.token == COMMA) {
                S.nextToken();
                if (S.token == RBRACE)
                    break;
                elems.append(variableInitializer());
            }
        }
        accept(RBRACE);
        return F.at(pos).NewArray(t, Tree.emptyList, elems.toList());
    }

    /**
      * VariableInitializer = ArrayInitializer | Expression
      */
    Tree variableInitializer() {
        return S.token == LBRACE ? arrayInitializer(null) : expression();
    }

    /**
      * ParExpression = "(" Expression ")"
      */
    Tree parExpression() {
        int pos = S.pos;
        accept(LPAREN);
        Tree t = expression();
        accept(RPAREN);
        return genEndPos ? F.at(pos).Parens(t) : t;
    }

    /**
      * Block = "{" BlockStatements "}"
      */
    Block block(long flags) {
        int pos = S.pos;
        accept(LBRACE);
        List stats = blockStatements();
        Block t = F.at(pos).Block(flags, stats);
        while (S.token == CASE || S.token == DEFAULT) {
            syntaxError("orphaned", keywords.token2string(S.token));
            blockStatements();
        }
        t.endpos = S.pos;
        accept(RBRACE);
        return t;
    }

    Block block() {
        return block(0);
    }

    /**
      * BlockStatements = { BlockStatement }
      *  BlockStatement  = LocalVariableDeclarationStatement
      *                  | ClassOrInterfaceDeclaration
      *                  | [Ident ":"] Statement
      *  LocalVariableDeclarationStatement
      *                  = [FINAL] Type VariableDeclarators ";"
      */
    List blockStatements() {
        ListBuffer stats = new ListBuffer();
        while (true) {
            int pos = S.pos;
            switch (S.token) {
            case RBRACE:

            case CASE:

            case DEFAULT:

            case EOF:
                return stats.toList();

            case LBRACE:

            case IF:

            case FOR:

            case WHILE:

            case DO:

            case TRY:

            case SWITCH:

            case SYNCHRONIZED:

            case RETURN:

            case THROW:

            case BREAK:

            case CONTINUE:

            case SEMI:

            case ELSE:

            case FINALLY:

            case CATCH:
                stats.append(statement());
                break;

            case FINAL:
                {
                    String dc = S.docComment;
                    long flags = modifiersOpt();
                    if (S.token == INTERFACE || S.token == CLASS) {
                        stats.append(classOrInterfaceDeclaration(flags, dc));
                    } else {
                        pos = S.pos;
                        Name name = S.name;
                        Tree t = type();
                        stats.appendList(variableDeclarators(flags, t));
                        accept(SEMI);
                    }
                    break;
                }

            case ABSTRACT:

            case STRICTFP:
                {
                    String dc = S.docComment;
                    long flags = modifiersOpt();
                    stats.append(classOrInterfaceDeclaration(flags, dc));
                    break;
                }

            case INTERFACE:

            case CLASS:
                stats.append(classOrInterfaceDeclaration(0, S.docComment));
                break;

            case ASSERT:
                if (allowAsserts) {
                    stats.append(statement());
                    break;
                }

            default:
                Name name = S.name;
                Tree t = term(EXPR | TYPE);
                if (S.token == COLON && t.tag == Tree.IDENT) {
                    S.nextToken();
                    Tree stat = statement();
                    stats.append(F.at(pos).Labelled(name, stat));
                } else if ((lastmode & TYPE) != 0 &&
                        (S.token == IDENTIFIER || S.token == ASSERT)) {
                    stats.appendList(variableDeclarators(0, t));
                    accept(SEMI);
                } else {
                    stats.append(F.at(pos).Exec(checkExprStat(t)));
                    accept(SEMI);
                }

            }
        }
    }

    /**
      * Statement =
      *       Block
      *     | IF ParExpression Statement [ELSE Statement]
      *     | FOR "(" ForInitOpt ";" [Expression] ";" ForUpdateOpt ")" Statement
      *     | WHILE ParExpression Statement
      *     | DO Statement WHILE ParExpression ";"
      *     | TRY Block ( Catches | [Catches] FinallyPart )
      *     | SWITCH ParExpression "{" SwitchBlockStatementGroups "}"
      *     | SYNCHRONIZED ParExpression Block
      *     | RETURN [Expression] ";"
      *     | THROW Expression ";"
      *     | BREAK [Ident] ";"
      *     | CONTINUE [Ident] ";"
      *     | ASSERT Expression [ ":" Expression ] ";"
      *     | ";"
      *     | ExpressionStatement
      *     | Ident ":" Statement
      */
    Tree statement() {
        int pos = S.pos;
        switch (S.token) {
        case LBRACE:
            return block();

        case IF:
            {
                S.nextToken();
                Tree cond = parExpression();
                Tree thenpart = statement();
                Tree elsepart = null;
                if (S.token == ELSE) {
                    S.nextToken();
                    elsepart = statement();
                }
                return F.at(pos).If(cond, thenpart, elsepart);
            }

        case FOR:
            {
                S.nextToken();
                accept(LPAREN);
                List inits = S.token == SEMI ? Tree.emptyList : forInit();
                accept(SEMI);
                Tree cond = S.token == SEMI ? null : expression();
                accept(SEMI);
                List steps = S.token == RPAREN ? Tree.emptyList : forUpdate();
                accept(RPAREN);
                Tree body = statement();
                return F.at(pos).ForLoop(inits, cond, steps, body);
            }

        case WHILE:
            {
                S.nextToken();
                Tree cond = parExpression();
                Tree body = statement();
                return F.at(pos).WhileLoop(cond, body);
            }

        case DO:
            {
                S.nextToken();
                Tree body = statement();
                accept(WHILE);
                Tree cond = parExpression();
                Tree t = F.at(pos).DoLoop(body, cond);
                if (genEndPos)
                    endPositions.put(t, new Integer(S.endPos));
                accept(SEMI);
                return t;
            }

        case TRY:
            {
                S.nextToken();
                Tree body = block();
                ListBuffer catchers = new ListBuffer();
                Tree finalizer = null;
                if (S.token == CATCH || S.token == FINALLY) {
                    while (S.token == CATCH)
                        catchers.append(catchClause());
                    if (S.token == FINALLY) {
                        S.nextToken();
                        finalizer = block();
                    }
                } else {
                    log.error(pos, "try.without.catch.or.finally");
                }
                return F.at(pos).Try(body, catchers.toList(), finalizer);
            }

        case SWITCH:
            {
                S.nextToken();
                Tree selector = parExpression();
                accept(LBRACE);
                List cases = switchBlockStatementGroups();
                Tree t = F.at(pos).Switch(selector, cases);
                if (genEndPos)
                    endPositions.put(t, new Integer(S.endPos));
                accept(RBRACE);
                return t;
            }

        case SYNCHRONIZED:
            {
                S.nextToken();
                Tree lock = parExpression();
                Tree body = block();
                return F.at(pos).Synchronized(lock, body);
            }

        case RETURN:
            {
                S.nextToken();
                Tree result = S.token == SEMI ? null : expression();
                Tree t = F.at(pos).Return(result);
                if (genEndPos)
                    endPositions.put(t, new Integer(S.endPos));
                accept(SEMI);
                return t;
            }

        case THROW:
            {
                S.nextToken();
                Tree exc = expression();
                Tree t = F.at(pos).Throw(exc);
                if (genEndPos)
                    endPositions.put(t, new Integer(S.endPos));
                accept(SEMI);
                return t;
            }

        case BREAK:
            {
                S.nextToken();
                Name label = (S.token == IDENTIFIER || S.token == ASSERT) ? ident() :
                        null;
                Tree t = F.at(pos).Break(label);
                if (genEndPos)
                    endPositions.put(t, new Integer(S.prevEndPos));
                accept(SEMI);
                return t;
            }

        case CONTINUE:
            {
                S.nextToken();
                Name label = (S.token == IDENTIFIER || S.token == ASSERT) ? ident() :
                        null;
                Tree t = F.at(pos).Continue(label);
                if (genEndPos)
                    endPositions.put(t, new Integer(S.prevEndPos));
                accept(SEMI);
                return t;
            }

        case SEMI:
            S.nextToken();
            return F.at(pos).Skip();

        case ELSE:
            return syntaxError("else.without.if");

        case FINALLY:
            return syntaxError("finally.without.try");

        case CATCH:
            return syntaxError("catch.without.try");

        case ASSERT:
            {
                if (allowAsserts) {
                    S.nextToken();
                    Tree assertion = expression();
                    Tree message = null;
                    if (S.token == COLON) {
                        S.nextToken();
                        message = expression();
                    }
                    Tree t = F.at(pos).Assert(assertion, message);
                    accept(SEMI);
                    return t;
                }
            }

        default:
            Name name = S.name;
            Tree expr = expression();
            if (S.token == COLON && expr.tag == Tree.IDENT) {
                S.nextToken();
                Tree stat = statement();
                return F.at(pos).Labelled(name, stat);
            } else {
                Tree stat = F.at(pos).Exec(checkExprStat(expr));
                accept(SEMI);
                return stat;
            }

        }
    }

    /**
      * CatchClause	= CATCH "(" FormalParameter ")" Block
      */
    Catch catchClause() {
        int pos = S.pos;
        accept(CATCH);
        accept(LPAREN);
        VarDef formal =
                variableDeclaratorId(optFinal() | Flags.PARAMETER, qualident());
        accept(RPAREN);
        Tree body = block();
        return F.at(pos).Catch(formal, body);
    }

    /**
      * SwitchBlockStatementGroups = { SwitchBlockStatementGroup }
      *  SwitchBlockStatementGroup = SwitchLabel BlockStatements
      *  SwitchLabel = CASE ConstantExpression ":" | DEFAULT ":"
      */
    List switchBlockStatementGroups() {
        ListBuffer cases = new ListBuffer();
        while (true) {
            int pos = S.pos;
            switch (S.token) {
            case CASE:
                {
                    S.nextToken();
                    Tree pat = expression();
                    accept(COLON);
                    List stats = blockStatements();
                    cases.append(F.at(pos).Case(pat, stats));
                    break;
                }

            case DEFAULT:
                {
                    S.nextToken();
                    accept(COLON);
                    List stats = blockStatements();
                    cases.append(F.at(pos).Case(null, stats));
                    break;
                }

            case RBRACE:

            case EOF:
                return cases.toList();

            default:
                S.nextToken();
                syntaxError(pos, "case.default.or.right-brace.expected");

            }
        }
    }

    /**
      * MoreStatementExpressions = { COMMA StatementExpression }
      */
    List moreStatementExpressions(int pos, Tree first) {
        ListBuffer stats = new ListBuffer();
        stats.append(F.at(pos).Exec(checkExprStat(first)));
        while (S.token == COMMA) {
            S.nextToken();
            pos = S.pos;
            Tree t = expression();
            stats.append(F.at(pos).Exec(checkExprStat(t)));
        }
        return stats.toList();
    }

    /**
      * ForInit = StatementExpression MoreStatementExpressions
      *           |  [FINAL] Type VariableDeclarators
      */
    List forInit() {
        int pos = S.pos;
        if (S.token == FINAL) {
            S.nextToken();
            return variableDeclarators(Flags.FINAL, type());
        } else {
            Tree t = term(EXPR | TYPE);
            if ((lastmode & TYPE) != 0 &&
                    (S.token == IDENTIFIER || S.token == ASSERT))
                return variableDeclarators(0, t);
            else
                return moreStatementExpressions(pos, t);
        }
    }

    /**
      * ForUpdate = StatementExpression MoreStatementExpressions
      */
    List forUpdate() {
        return moreStatementExpressions(S.pos, expression());
    }

    /**
      * ModifiersOpt = { Modifier }
      *  Modifier = PUBLIC | PROTECTED | PRIVATE | STATIC | ABSTRACT | FINAL
      *           | NATIVE | SYNCHRONIZED | TRANSIENT | VOLATILE
      */
    long modifiersOpt() {
        long flags = 0;
        if (S.deprecatedFlag) {
            flags = Flags.DEPRECATED;
            S.deprecatedFlag = false;
        }
        while (true) {
            int flag;
            switch (S.token) {
            case PRIVATE:
                flag = Flags.PRIVATE;
                break;

            case PROTECTED:
                flag = Flags.PROTECTED;
                break;

            case PUBLIC:
                flag = Flags.PUBLIC;
                break;

            case STATIC:
                flag = Flags.STATIC;
                break;

            case TRANSIENT:
                flag = Flags.TRANSIENT;
                break;

            case FINAL:
                flag = Flags.FINAL;
                break;

            case ABSTRACT:
                flag = Flags.ABSTRACT;
                break;

            case NATIVE:
                flag = Flags.NATIVE;
                break;

            case VOLATILE:
                flag = Flags.VOLATILE;
                break;

            case SYNCHRONIZED:
                flag = Flags.SYNCHRONIZED;
                break;

            case STRICTFP:
                flag = Flags.STRICTFP;
                break;

            default:
                return flags;

            }
            if ((flags & flag) != 0)
                log.error(S.pos, "repeated.modifier");
            flags = flags | flag;
            S.nextToken();
        }
    }

    /**
      * VariableDeclarators = VariableDeclarator { "," VariableDeclarator }
      */
    List variableDeclarators(long flags, Tree type) {
        return variableDeclaratorsRest(S.pos, flags, type, ident(), false, null);
    }

    /**
      * VariableDeclaratorsRest = VariableDeclaratorRest { "," VariableDeclarator }
      *  ConstantDeclaratorsRest = ConstantDeclaratorRest { "," ConstantDeclarator }
      *
      *  @param reqInit  Is an initializer always required?
      *  @param dc       The documentation comment for the variable declarations, or null.
      */
    List variableDeclaratorsRest(int pos, long flags, Tree type, Name name,
            boolean reqInit, String dc) {
        ListBuffer vdefs = new ListBuffer();
        vdefs.append(variableDeclaratorRest(pos, flags, type, name, reqInit, dc));
        while (S.token == COMMA) {
            S.nextToken();
            vdefs.append(variableDeclarator(flags, type, reqInit, dc));
        }
        return vdefs.toList();
    }

    /**
      * VariableDeclarator = Ident VariableDeclaratorRest
      *  ConstantDeclarator = Ident ConstantDeclaratorRest
      */
    VarDef variableDeclarator(long flags, Tree type, boolean reqInit, String dc) {
        return variableDeclaratorRest(S.pos, flags, type, ident(), reqInit, dc);
    }

    /**
      * VariableDeclaratorRest = BracketsOpt ["=" VariableInitializer]
      *  ConstantDeclaratorRest = BracketsOpt "=" VariableInitializer
      *
      *  @param reqInit  Is an initializer always required?
      *  @param dc       The documentation comment for the variable declarations, or null.
      */
    VarDef variableDeclaratorRest(int pos, long flags, Tree type, Name name,
            boolean reqInit, String dc) {
        type = bracketsOpt(type);
        Tree init = null;
        if (S.token == EQ) {
            S.nextToken();
            init = variableInitializer();
        } else if (reqInit)
            syntaxError(S.pos, "expected", keywords.token2string(EQ));
        VarDef result = F.at(pos).VarDef(flags, name, type, init);
        if (genEndPos)
            endPositions.put(result, new Integer(S.prevEndPos));
        attach(result, dc);
        return result;
    }

    /**
      * VariableDeclaratorId = Ident BracketsOpt
      */
    VarDef variableDeclaratorId(long flags, Tree type) {
        int pos = S.pos;
        Name name = ident();
        type = bracketsOpt(type);
        return F.at(pos).VarDef(flags, name, type, null);
    }

    /**
      * CompilationUnit = [PACKAGE Qualident ";"] {ImportDeclaration} {TypeDeclaration}
      */
    public Tree.TopLevel compilationUnit() {
        int pos = S.pos;
        Tree pid = null;
        String dc = S.docComment;
        if (S.token == PACKAGE) {
            S.nextToken();
            pid = qualident();
            accept(SEMI);
        }
        ListBuffer defs = new ListBuffer();
        while (S.token == IMPORT)
            defs.append(importDeclaration());
        while (S.token != EOF)
            defs.append(typeDeclaration());
        Tree.TopLevel toplevel = F.at(pos).TopLevel(pid, defs.toList());
        attach(toplevel, dc);
        if (keepDocComments)
            toplevel.docComments = docComments;
        if (genEndPos)
            toplevel.endPositions = endPositions;
        return toplevel;
    }

    /**
      * ImportDeclaration = IMPORT Ident { "." Ident } [ "." "*" ] ";"
      */
    Tree importDeclaration() {
        int pos = S.pos;
        S.nextToken();
        Tree pid = F.at(S.pos).Ident(ident());
        do {
            accept(DOT);
            if (S.token == STAR) {
                pid = F.at(S.pos).Select(pid, names.asterisk);
                S.nextToken();
                break;
            } else {
                pid = F.at(S.pos).Select(pid, ident());
            }
        } while (S.token == DOT)
            ;
        accept(SEMI);
        return F.at(pos).Import(pid);
    }

    /**
      * TypeDeclaration = ClassOrInterfaceDeclaration
      *                  | ";"
      */
    Tree typeDeclaration() {
        long flags = 0;
        if (S.pos == S.errPos) {
            flags = modifiersOpt();
            while (S.token != CLASS && S.token != INTERFACE && S.token != EOF) {
                S.nextToken();
                flags = modifiersOpt();
            }
        }
        int pos = S.pos;
        if (S.token == SEMI) {
            S.nextToken();
            return F.at(pos).Block(0, Tree.emptyList);
        } else {
            String dc = S.docComment;
            flags = flags | modifiersOpt();
            return classOrInterfaceDeclaration(flags, dc);
        }
    }

    /**
      * ClassOrInterfaceDeclaration = ModifiersOpt
      *                                (ClassDeclaration | InterfaceDeclaration)
      *  @param flags    Any modifiers starting the class or interface declaration
      *                  which are not in ModifiersOpt
      *  @param dc       The documentation comment for the class, or null.
      */
    Tree classOrInterfaceDeclaration(long flags, String dc) {
        flags = flags | modifiersOpt();
        if (S.token == CLASS)
            return classDeclaration(flags, dc);
        else if (S.token == INTERFACE)
            return interfaceDeclaration(flags, dc);
        else
            return syntaxError("class.or.intf.expected");
    }

    /**
      * ClassDeclaration = CLASS Ident TypeParametersOpt [EXTENDS Type]
      *                     [IMPLEMENTS TypeList] ClassBody
      *  @param flags    The modifiers starting the class declaration
      *  @param dc       The documentation comment for the class, or null.
      */
    Tree classDeclaration(long flags, String dc) {
        int pos = S.pos;
        accept(CLASS);
        Name name = ident();
        List typarams = TypeParameter.emptyList;
        Tree extending = null;
        if (S.token == EXTENDS) {
            S.nextToken();
            extending = type();
        }
        List implementing = Tree.emptyList;
        if (S.token == IMPLEMENTS) {
            S.nextToken();
            implementing = typeList();
        }
        List defs = classOrInterfaceBody(name, false);
        Tree result = F.at(pos).ClassDef(flags, name, typarams, extending,
                implementing, defs);
        attach(result, dc);
        return result;
    }

    /**
      * InterfaceDeclaration = INTERFACE Ident TypeParametersOpt
      *                         [EXTENDS TypeList] InterfaceBody
      *  @param flags    The modifiers starting the interface declaration
      *  @param dc       The documentation comment for the interface, or null.
      */
    Tree interfaceDeclaration(long flags, String dc) {
        int pos = S.pos;
        accept(INTERFACE);
        Name name = ident();
        List typarams = TypeParameter.emptyList;
        List extending = Tree.emptyList;
        if (S.token == EXTENDS) {
            S.nextToken();
            extending = typeList();
        }
        List defs = classOrInterfaceBody(name, true);
        Tree result = F.at(pos).ClassDef(flags | Flags.INTERFACE, name, typarams,
                null, extending, defs);
        attach(result, dc);
        return result;
    }

    /**
      * TypeList = Type {"," Type}
      */
    List typeList() {
        ListBuffer ts = new ListBuffer();
        ts.append(type());
        while (S.token == COMMA) {
            S.nextToken();
            ts.append(type());
        }
        return ts.toList();
    }

    /**
      * ClassBody     = "{" {ClassBodyDeclaration} "}"
      *  InterfaceBody = "{" {InterfaceBodyDeclaration} "}"
      */
    List classOrInterfaceBody(Name className, boolean isInterface) {
        int pos = S.pos;
        accept(LBRACE);
        ListBuffer defs = new ListBuffer();
        while (S.token != RBRACE && S.token != EOF) {
            defs.appendList(classOrInterfaceBodyDeclaration(className, isInterface));
        }
        accept(RBRACE);
        return defs.toList();
    }

    /**
      * ClassBodyDeclaration =
      *      ";"
      *    | [STATIC] Block
      *    | ModifiersOpt
      *      ( Type Ident
      *        ( VariableDeclaratorsRest ";" | MethodDeclaratorRest )
      *      | VOID Ident MethodDeclaratorRest
      *      | TypeParameters (Type | VOID) Ident MethodDeclaratorRest
      *      | Ident ConstructorDeclaratorRest
      *      | TypeParameters Ident ConstructorDeclaratorRest
      *      | ClassOrInterfaceDeclaration
      *      )
      *  InterfaceBodyDeclaration =
      *      ";"
      *    | ModifiersOpt Type Ident
      *      ( ConstantDeclaratorsRest | InterfaceMethodDeclaratorRest ";" )
      */
    List classOrInterfaceBodyDeclaration(Name className, boolean isInterface) {
        int pos = S.pos;
        if (S.token == SEMI) {
            S.nextToken();
            return Tree.emptyList.prepend(F.at(pos).Block(0, Tree.emptyList));
        } else {
            String dc = S.docComment;
            long flags = modifiersOpt();
            if (S.token == CLASS || S.token == INTERFACE) {
                return Tree.emptyList.prepend(
                        classOrInterfaceDeclaration(flags, dc));
            } else if (S.token == LBRACE && !isInterface &&
                    (flags & Flags.StandardFlags & ~Flags.STATIC) == 0) {
                return Tree.emptyList.prepend(block(flags));
            } else {
                List typarams = TypeParameter.emptyList;
                int token = S.token;
                Name name = S.name;
                pos = S.pos;
                Tree type;
                boolean isVoid = S.token == VOID;
                if (isVoid) {
                    type = F.at(pos).TypeIdent(Type.VOID);
                    S.nextToken();
                } else {
                    type = type();
                }
                if (S.token == LPAREN && !isInterface && type.tag == Tree.IDENT) {
                    if (isInterface || name != className)
                        log.error(pos, "invalid.meth.decl.ret.type.req");
                    return Tree.emptyList.prepend(
                            methodDeclaratorRest(pos, flags, null, names.init,
                            typarams, isInterface, true, dc));
                } else {
                    pos = S.pos;
                    name = ident();
                    if (S.token == LPAREN) {
                        return Tree.emptyList.prepend(
                                methodDeclaratorRest(pos, flags, type, name,
                                typarams, isInterface, isVoid, dc));
                    } else if (!isVoid && typarams.isEmpty()) {
                        List defs = variableDeclaratorsRest(pos, flags, type, name,
                                isInterface, dc);
                        accept(SEMI);
                        return defs;
                    } else {
                        syntaxError(S.pos, "expected", keywords.token2string(LPAREN));
                        return Tree.emptyList;
                    }
                }
            }
        }
    }

    /**
      * MethodDeclaratorRest =
      *      FormalParameters BracketsOpt [Throws TypeList] ( MethodBody | ";")
      *  VoidMethodDeclaratorRest =
      *      FormalParameters [Throws TypeList] ( MethodBody | ";")
      *  InterfaceMethodDeclaratorRest =
      *      FormalParameters BracketsOpt [THROWS TypeList] ";"
      *  VoidInterfaceMethodDeclaratorRest =
      *      FormalParameters [THROWS TypeList] ";"
      *  ConstructorDeclaratorRest =
      *      "(" FormalParameterListOpt ")" [THROWS TypeList] MethodBody
      */
    Tree methodDeclaratorRest(int pos, long flags, Tree type, Name name,
            List typarams, boolean isInterface, boolean isVoid, String dc) {
        List params = formalParameters();
        if (!isVoid)
            type = bracketsOpt(type);
        List thrown = Tree.emptyList;
        if (S.token == THROWS) {
            S.nextToken();
            thrown = qualidentList();
        }
        Block body;
        if (S.token == LBRACE) {
            body = block();
        } else {
            accept(SEMI);
            body = null;
        }
        Tree result = F.at(pos).MethodDef(flags, name, type, typarams, params, thrown,
                body);
        attach(result, dc);
        return result;
    }

    /**
      * QualidentList = Qualident {"," Qualident}
      */
    List qualidentList() {
        ListBuffer ts = new ListBuffer();
        ts.append(qualident());
        while (S.token == COMMA) {
            S.nextToken();
            ts.append(qualident());
        }
        return ts.toList();
    }

    /**
      * FormalParameters = "(" [FormalParameter {"," FormalParameter}] ")"
      */
    List formalParameters() {
        ListBuffer params = new ListBuffer();
        accept(LPAREN);
        if (S.token != RPAREN) {
            params.append(formalParameter());
            while (S.token == COMMA) {
                S.nextToken();
                params.append(formalParameter());
            }
        }
        accept(RPAREN);
        return params.toList();
    }

    int optFinal() {
        if (S.token == FINAL) {
            S.nextToken();
            return Flags.FINAL;
        } else {
            return 0;
        }
    }

    /**
      * FormalParameter = [FINAL] Type VariableDeclaratorId
      */
    VarDef formalParameter() {
        return variableDeclaratorId(optFinal() | Flags.PARAMETER, type());
    }

    /**
      * Share the terminator when making lists of trees.
      *  This is am optimized implementation of List.make(a, b).
      */
    private List makeList(Tree a, Tree b) {
        return new List(a, new List(b, Tree.emptyList));
    }

    /**
      * Share the terminator when making lists of trees.
      *  This is am optimized implementation of List.make(a).
      */
    private List makeList(Tree a) {
        return new List(a, Tree.emptyList);
    }

    /**
      * Check that given tree is a legal expression statement.
      */
    Tree checkExprStat(Tree t) {
        switch (t.tag) {
        case Tree.PREINC:

        case Tree.PREDEC:

        case Tree.POSTINC:

        case Tree.POSTDEC:

        case Tree.ASSIGN:

        case Tree.BITOR_ASG:

        case Tree.BITXOR_ASG:

        case Tree.BITAND_ASG:

        case Tree.SL_ASG:

        case Tree.SR_ASG:

        case Tree.USR_ASG:

        case Tree.PLUS_ASG:

        case Tree.MINUS_ASG:

        case Tree.MUL_ASG:

        case Tree.DIV_ASG:

        case Tree.MOD_ASG:

        case Tree.APPLY:

        case Tree.NEWCLASS:

        case Tree.ERRONEOUS:
            return t;

        default:
            log.error(t.pos, "not.stmt");
            return errorTree;

        }
    }

    /**
      * Return precedence of operator represented by token,
      *  -1 if token is not a binary operator. @see TreeInfo.opPrec
      */
    static int prec(int token) {
        int oc = optag(token);
        return (oc >= 0) ? TreeInfo.opPrec(oc) : -1;
    }

    /**
      * Return operation tag of binary operator represented by token,
      *  -1 if token is not a binary operator.
      */
    static int optag(int token) {
        switch (token) {
        case BARBAR:
            return Tree.OR;

        case AMPAMP:
            return Tree.AND;

        case BAR:
            return Tree.BITOR;

        case BAREQ:
            return Tree.BITOR_ASG;

        case CARET:
            return Tree.BITXOR;

        case CARETEQ:
            return Tree.BITXOR_ASG;

        case AMP:
            return Tree.BITAND;

        case AMPEQ:
            return Tree.BITAND_ASG;

        case EQEQ:
            return Tree.EQ;

        case BANGEQ:
            return Tree.NE;

        case LT:
            return Tree.LT;

        case GT:
            return Tree.GT;

        case LTEQ:
            return Tree.LE;

        case GTEQ:
            return Tree.GE;

        case LTLT:
            return Tree.SL;

        case LTLTEQ:
            return Tree.SL_ASG;

        case GTGT:
            return Tree.SR;

        case GTGTEQ:
            return Tree.SR_ASG;

        case GTGTGT:
            return Tree.USR;

        case GTGTGTEQ:
            return Tree.USR_ASG;

        case PLUS:
            return Tree.PLUS;

        case PLUSEQ:
            return Tree.PLUS_ASG;

        case SUB:
            return Tree.MINUS;

        case SUBEQ:
            return Tree.MINUS_ASG;

        case STAR:
            return Tree.MUL;

        case STAREQ:
            return Tree.MUL_ASG;

        case SLASH:
            return Tree.DIV;

        case SLASHEQ:
            return Tree.DIV_ASG;

        case PERCENT:
            return Tree.MOD;

        case PERCENTEQ:
            return Tree.MOD_ASG;

        case INSTANCEOF:
            return Tree.TYPETEST;

        default:
            return -1;

        }
    }

    /**
      * Return operation tag of unary operator represented by token,
      *  -1 if token is not a binary operator.
      */
    static int unoptag(int token) {
        switch (token) {
        case PLUS:
            return Tree.POS;

        case SUB:
            return Tree.NEG;

        case BANG:
            return Tree.NOT;

        case TILDE:
            return Tree.COMPL;

        case PLUSPLUS:
            return Tree.PREINC;

        case SUBSUB:
            return Tree.PREDEC;

        default:
            return -1;

        }
    }

    /**
      * Return type tag of basic type represented by token,
      *  -1 if token is not a basic type identifier.
      */
    static int typetag(int token) {
        switch (token) {
        case BYTE:
            return Type.BYTE;

        case CHAR:
            return Type.CHAR;

        case SHORT:
            return Type.SHORT;

        case INT:
            return Type.INT;

        case LONG:
            return Type.LONG;

        case FLOAT:
            return Type.FLOAT;

        case DOUBLE:
            return Type.DOUBLE;

        case BOOLEAN:
            return Type.BOOLEAN;

        default:
            return -1;

        }
    }
}
