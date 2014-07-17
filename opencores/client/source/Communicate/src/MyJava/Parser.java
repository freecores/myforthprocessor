package MyJava;

/**
 * <p>Überschrift: Basisklasse von Pass</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.io.*;
import java.util.*;

public class Parser implements FilenameFilter
 {
  static String jdkSource = null;
  protected Vector prefetch;
  protected Token token;        // last token
  protected Token nextToken;    // lookahead token
  protected Keyword nextSymbol; // holds la.kind
  protected int countToken;     // number of processed token
  private int oldCount;
  protected boolean main_found; // main method found
  protected int peekPos = 0;
  protected Scanner scanner;
  protected String comment;
  protected boolean acceptComment;
  protected final File sources;
  protected Vector records;

  static HashSet typeKW = new HashSet();

  static
   {
    typeKW.add(Keyword.BYTESY);
    typeKW.add(Keyword.SHORTSY);
    typeKW.add(Keyword.CHARSY);
    typeKW.add(Keyword.INTSY);
    typeKW.add(Keyword.LONGSY);
    typeKW.add(Keyword.FLOATSY);
    typeKW.add(Keyword.DOUBLESY);
    typeKW.add(Keyword.BOOLEANSY);
   }

  static HashSet castFollower = new HashSet();

  static
   {
    castFollower.add(Keyword.IDENTSY);
    castFollower.add(Keyword.NEWSY);
    castFollower.add(Keyword.SUPERSY);
    castFollower.add(Keyword.THISSY);
    castFollower.add(Keyword.VOIDSY);
    castFollower.add(Keyword.NUMBERSY);
    castFollower.add(Keyword.DNUMBERSY);
    castFollower.add(Keyword.CHARCONSTSY);
    castFollower.add(Keyword.STRINGSY);
    castFollower.add(Keyword.TRUESY);
    castFollower.add(Keyword.FALSESY);
    castFollower.add(Keyword.NULLSY);
    castFollower.add(Keyword.LPARSY);
    castFollower.add(Keyword.NOTSY);
    castFollower.add(Keyword.INVERTSY);
   }

  static HashSet prefix = new HashSet();

  static
   {
    prefix.add(Keyword.INCSY);
    prefix.add(Keyword.DECSY);
    prefix.add(Keyword.NOTSY);
    prefix.add(Keyword.INVERTSY);
    prefix.add(Keyword.PLUSSY);
    prefix.add(Keyword.MINUSSY);
   }

  static HashSet infix = new HashSet();

  static
   {
    infix.add(Keyword.LOGICALORSY);
    infix.add(Keyword.LOGICALANDSY);
    infix.add(Keyword.INSTANCEOFSY);
    infix.add(Keyword.ORSY);
    infix.add(Keyword.ANDSY);
    infix.add(Keyword.XORSY);
    infix.add(Keyword.EQUALSY);
    infix.add(Keyword.NOTEQUALSY);
    infix.add(Keyword.LESSSY);
    infix.add(Keyword.GREATERSY);
    infix.add(Keyword.LESSEQUALSY);
    infix.add(Keyword.GREATEREQUALSY);
    infix.add(Keyword.LSHIFTSY);
    infix.add(Keyword.RSHIFTSY);
    infix.add(Keyword.ASHIFTSY);
    infix.add(Keyword.PLUSSY);
    infix.add(Keyword.MINUSSY);
    infix.add(Keyword.MULTIPLYSY);
    infix.add(Keyword.SLASHSY);
    infix.add(Keyword.MODULOSY);
   }

  static HashSet postfix = new HashSet();

  static
   {
    postfix.add(Keyword.INCSY);
    postfix.add(Keyword.DECSY);
   }

  static HashSet assign = new HashSet();

  static
   {
    assign.add(Keyword.ASSIGNSY);
    assign.add(Keyword.PLUSASSIGNSY);
    assign.add(Keyword.MINUSASSIGNSY);
    assign.add(Keyword.MULTASSIGNSY);
    assign.add(Keyword.DIVASSIGNSY);
    assign.add(Keyword.ANDASSIGNSY);
    assign.add(Keyword.ORASSIGNSY);
    assign.add(Keyword.XORASSIGNSY);
    assign.add(Keyword.MODASSIGNSY);
    assign.add(Keyword.LSHIFTASSIGNSY);
    assign.add(Keyword.RSHIFTASSIGNSY);
    assign.add(Keyword.ASHIFTASSIGNSY);
    assign.add(Keyword.FINALASSIGNSY);
   }

   /**
    * accept only java source files
    * @param dir File
    * @param name String
    * @return boolean
    */
   public boolean accept(File dir, String name)
   {
    return name.endsWith(".java");
   }

   /**
    * constructor
    * @param input File source
    */
   public Parser(File input)
   {
    prefetch = new Vector();
    oldCount = 0;
    comment = "";
    acceptComment = false;

    sources = input;
    scanner = new Scanner(sources);
    records = new Vector();
   }

   /**
    * append comment to comment buffer
    * @param x Token
    */
   protected void appendComment(Token x)
   {
    if (x.kind == Keyword.COMMENTSY)
     comment += " ( " + x.string + " ) ";
    else if (x.kind == Keyword.SIMPLECOMMENTSY)
     comment += " \\ " + x.string + '\n';
   }

   /**
    * clear comment buffer
    */
   protected void resetComment()
   {
    comment = "";
   }

   /**
    * record token into vector
    * @param v Vector
    */
   protected void traceOn(Vector v)
   {
    records.add(v);
   }

   /**
    * remove recording vector
    * @param v Vector
    */
   protected void traceOff(Vector v)
   {
    records.remove(v);
   }

  /**
    * look ahead for next token and store it in nextToken
    */
  protected void lookAhead()
   {    // Vorauslesen um ein Symbol
    token = nextToken;
    for(int i = 0; i < records.size(); i++)
     {
      Vector v = (Vector)records.get(i);
      v.add(nextToken);
     }

    do
     {
      if (prefetch.size() > 0)
       {
        nextToken = (Token)prefetch.get(0);
        prefetch.remove(0);
       }
      else
       nextToken = scanner.next();

      nextSymbol = nextToken.kind;

      if (acceptComment && (nextSymbol == Keyword.COMMENTSY || nextSymbol == Keyword.SIMPLECOMMENTSY))
       appendComment(nextToken);
     }
    while(nextSymbol == Keyword.COMMENTSY || nextSymbol == Keyword.SIMPLECOMMENTSY);
    countToken++;
    //Testausgabe um Programmablauf zu prüfen
    //System.out.println(nextToken.line+"\t"+nextToken.col+"\t"+nextSymbol.string);
    //Errors.warning(null,nextToken.line,nextToken.col,nextToken.string,false);
   }

   /**
    * poke token
    * @param pt Token
    */
   protected void poke(Token pt)
   {
    prefetch.add(0, pt);
   }

   /**
    * prefetch distinct token
    * @param nr int
    * @return Token
    */
   protected Token peek(int nr)
   {
    int i;
    Token t = null;

    for(i = 0; i <= nr; i++)
     {
      if (i < prefetch.size())
       t = (Token)prefetch.get(i);
      else
       prefetch.add(i, t = scanner.next());

      if (t.kind == Keyword.COMMENTSY || t.kind == Keyword.SIMPLECOMMENTSY)
       nr++;
     }
    
    return t;
   }

   /**
    * reset prefetch counter
    */
   protected void peekReset()
   {
    peekPos = 0;
   }

   /**
    * prefetch next token
    * @return Token
    */
   protected Token peek()
   {
    return peek(peekPos++);
   }

  /**
    * found keyword must match
    *
    * @param expected the desired keyword
    * @return true it matches
    */
  protected boolean matchKeyword(Keyword expected)
   {
    boolean x;

    if ((x = nextSymbol == expected))
     lookAhead();
    else
     error(expected.string + " expected, not \"" + nextSymbol.string + "\"");

    return x;
   }

  /**
    * print precise warning
    *
    * @param msg the message
    */
  public void warning(Keyword op, String msg)
   {
	msg = (op == null?"null":op.string) + " " + msg;
    Errors.warning(nextToken.source, nextToken.line, nextToken.col, msg, false);
   }

  public void warning(String msg)
  {
   Errors.warning(nextToken.source, nextToken.line, nextToken.col, msg, false);
  }
  
  /**
    * print precise error message
    *
    * @param msg the message
    */
  public void error(String msg)
   {
    if (countToken - oldCount > 3)
     {
      oldCount = countToken;
      Errors.println(nextToken.source, nextToken.line, nextToken.col, msg);
     }
   }

  /**
    * print error at code generation
    *
    * @param msg the message
    */
  public void codeError(String msg)
   {  // Fehlermeldung
    Errors.println(token.source, token.line,token.col,msg);
   }

  /**
    * get source linenumber of current token
    *
    * @return  linenumber
    */
  public int getLine()
   {
    return token.line;
   }

  /**
    * get starting column of token
    *
    * @return  the column
    */
  public int getCol()
   {
    return token.col;
   }

  // ( "this" | "super" ) "("
  protected boolean isThisOrSuperCall()
   {
    peekReset();
    Token pt = nextToken;
    return (pt.kind == Keyword.THISSY || pt.kind == Keyword.SUPERSY) && peek().kind == Keyword.LPARSY;
   }

  // "(" BasicType {"[""]"} ")"
  protected boolean isSimpleTypeCast()
   {
    // assert: la.kind == _lpar
    peekReset();
    if (typeKW.contains(peek().kind))
     {
      Token pt = skipDims(peek());
      if (pt != null)
       return pt.kind == Keyword.RPARSY;
     }
    return false;
   }

  // "(" Qualident {"[" "]"} ")" castFollower
  protected boolean guessTypeCast()
   {
    // assert: la.kind == _lpar
    peekReset();
    Token pt = rdQualident(peek());
    if (pt != null)
     {
      pt = skipDims(pt);
      if (pt != null)
       return pt.kind == Keyword.RPARSY && castFollower.contains(peek().kind);
     }
    return false;
   }

  // "[" "]"
  protected Token skipDims(Token pt)
   {
    if (pt.kind != Keyword.LBRACKETSY) return pt;
    do
     {
      pt = peek();
      if (pt.kind != Keyword.RBRACKETSY) return null;
      pt = peek();
     }
    while(pt.kind == Keyword.LBRACKETSY);

    return pt;
   }

  /* Checks whether the next sequence of tokens is a qualident *
   * and returns the qualident string                          *
   * !!! Proceeds from current peek position !!!               */
  protected Token rdQualident(Token pt)
   {
    String qualident = "";

    if (pt.kind == Keyword.IDENTSY)
     {
      qualident = pt.string;
      pt = peek();
      while (pt.kind == Keyword.DOTSY)
       {
        pt = peek();
        if (pt.kind != Keyword.IDENTSY) return null;
        qualident += "." + pt.string;
        pt = peek();
       }
      return pt;
     }
    else
     return null;
   }

  /*-----------------------------------------------------------------*
   * Resolver routines to resolve LL(1) conflicts:                   *
   * These routines return a boolean value that indicates            *
   * whether the alternative at hand shall be choosen or not.        *
   * They are used in IF ( ... ) expressions.                        *
   *-----------------------------------------------------------------*/

  // ',' (no '}')
  protected boolean commaAndNoRBrace()
   {
    return (nextSymbol == Keyword.COMMASY && peek(0).kind != Keyword.RBRACESY);
   }

  // '.' ident
  protected boolean dotAndIdent()
   {
    return nextSymbol == Keyword.DOTSY && peek(0).kind == Keyword.IDENTSY;
   }

  // ident '('
  protected boolean identAndLPar()
   {
    return nextSymbol == Keyword.IDENTSY && peek(0).kind == Keyword.LPARSY;
   }

  // ident ':'
  protected boolean isLabel()
   {
    return nextSymbol == Keyword.IDENTSY && peek(0).kind == Keyword.COLONSY;
   }

  // '[' (no ']')
  protected boolean nonEmptyBracket()
   {
    return (nextSymbol == Keyword.LBRACKETSY && peek(0).kind != Keyword.RBRACKETSY);
   }

  // '['']'
  protected boolean emptyBracket()
   {
    return (nextSymbol == Keyword.LBRACKETSY && peek(0).kind == Keyword.RBRACKETSY);
   }

  // final or Type ident
  protected boolean isLocalVarDecl(boolean finalIsSuccess)
   {
    Token pt = nextToken;
	peekReset();

    if (nextSymbol == Keyword.FINALSY)
     if (finalIsSuccess)
      return true;
     else
      pt = peek();

    // basicType | ident
    if (typeKW.contains(pt.kind))
     pt = peek();
    else
     pt = rdQualident(pt);

    if (pt != null)
     {
      pt = skipDims(pt);
      if (pt != null)
       return pt.kind == Keyword.IDENTSY;
     }
    return false;
   }

  protected boolean isTypeCast()
   {
    if (nextSymbol != Keyword.LPARSY) return false;
    if (isSimpleTypeCast()) return true;
    return guessTypeCast();
   }

  // '.' ("class" | "this") | '(' | '['']'
  protected boolean isIdentSuffix()
   {
    if (nextSymbol == Keyword.DOTSY)
     {
      peekReset();
      Token pt = peek();
      return pt.kind == Keyword.CLASSSY || pt.kind == Keyword.THISSY;
     }
    return (nextSymbol == Keyword.LPARSY || emptyBracket());
   }

  protected int classModifier()
   {
    int x = 0;

    if (nextSymbol == Keyword.PUBLICSY || nextSymbol == Keyword.PROTECTEDSY ||
        nextSymbol == Keyword.PRIVATESY || nextSymbol == Keyword.ABSTRACTSY ||
        nextSymbol == Keyword.STATICSY || nextSymbol == Keyword.FINALSY ||
        nextSymbol == Keyword.STRICTFPSY)
     {
      x = nextSymbol.value;
      lookAhead();
     }

    return x;
   }

  protected int modifier()
   {
    int x;

    if (nextSymbol == Keyword.STATICSY)
     {
      x = nextSymbol.value;
      lookAhead();
     }
    else
     x = modifier1();

    return x;
   }

  protected int modifier1()
   {
    int x = 0;

    if (nextSymbol == Keyword.PUBLICSY || nextSymbol == Keyword.PROTECTEDSY ||
        nextSymbol == Keyword.PRIVATESY || nextSymbol == Keyword.ABSTRACTSY ||
        nextSymbol == Keyword.FINALSY || nextSymbol == Keyword.NATIVESY ||
        nextSymbol == Keyword.SYNCHRONIZEDSY || nextSymbol == Keyword.TRANSIENTSY ||
        nextSymbol == Keyword.VOLATILESY || nextSymbol == Keyword.STRICTFPSY)
     {
      x = nextSymbol.value;
      lookAhead();
     }

    return x;
   }



   /**
    * @return number of processed token
    */
   public int getCountToken()
    {
     return countToken;
    }
  }
