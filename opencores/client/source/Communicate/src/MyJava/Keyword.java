package MyJava;

/**
 * <p>Überschrift: Schlüsselworte</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.util.*;
import java.lang.reflect.*;

public class Keyword
{
 public final int value;
 public final String string;

 private static Hashtable table = new Hashtable();

 /**
  * assign instance a number and a descriptive string
  */
 private Keyword(int value, String string)
 {
  this.value = value;
  this.string = string;
  table.put(string, this);
 }

 public static final Keyword NONESY = new Keyword(65536 * 1024 + 0, " none");
 public static final Keyword NUMBERSY = new Keyword(65536 * 1024 + 1, " number");
 public static final Keyword PLUSSY = new Keyword(65536 * 1024 + 2, "+");
 public static final Keyword MINUSSY = new Keyword(65536 * 1024 + 3, "-");
 public static final Keyword MULTIPLYSY = new Keyword(65536 * 1024 + 4, "*");
 public static final Keyword SLASHSY = new Keyword(65536 * 1024 + 5, "/");
 public static final Keyword MODULOSY = new Keyword(65536 * 1024 + 6, "%");
 public static final Keyword EQUALSY = new Keyword(65536 * 1024 + 7, "==");
 public static final Keyword NOTEQUALSY = new Keyword(65536 * 1024 + 8, "!=");
 public static final Keyword GREATERSY = new Keyword(65536 * 1024 + 9, ">");
 public static final Keyword GREATEREQUALSY = new Keyword(65536 * 1024 + 10, ">=");
 public static final Keyword LESSSY = new Keyword(65536 * 1024 + 11, "<");
 public static final Keyword LESSEQUALSY = new Keyword(65536 * 1024 + 12, "<=");
 public static final Keyword ANDSY = new Keyword(65536 * 1024 + 13, "&");
 public static final Keyword ORSY = new Keyword(65536 * 1024 + 14, "|");
 public static final Keyword ASSIGNSY = new Keyword(65536 * 1024 + 15, "=");
 public static final Keyword INCSY = new Keyword(65536 * 1024 + 16, "++");
 public static final Keyword DECSY = new Keyword(65536 * 1024 + 17, "--");

 public static final Keyword SEMICOLONSY = new Keyword(65536 * 1024 + 18, ";");
 public static final Keyword COLONSY = new Keyword(65536 * 1024 + 19, ":");
 public static final Keyword LPARSY = new Keyword(65536 * 1024 + 20, "(");
 public static final Keyword RPARSY = new Keyword(65536 * 1024 + 21, ")");
 public static final Keyword LBRACKETSY = new Keyword(65536 * 1024 + 22, "[");
 public static final Keyword RBRACKETSY = new Keyword(65536 * 1024 + 23, "]");
 public static final Keyword LBRACESY = new Keyword(65536 * 1024 + 24, "{");
 public static final Keyword RBRACESY = new Keyword(65536 * 1024 + 25, "}");

 public static final Keyword DOTSY = new Keyword(65536 * 1024 + 26, ".");
 public static final Keyword BREAKSY = new Keyword(65536 * 1024 + 27, "break");
 public static final Keyword ELSESY = new Keyword(65536 * 1024 + 28, "else");
 public static final Keyword IFSY = new Keyword(65536 * 1024 + 29, "if");
 public static final Keyword NEWSY = new Keyword(65536 * 1024 + 30, "new");
 public static final Keyword RETURNSY = new Keyword(65536 * 1024 + 31, "return");
 public static final Keyword FORTHSY = new Keyword(65536 * 1024 + 32, "#ass");
 public static final Keyword WHILESY = new Keyword(65536 * 1024 + 33, "while");
 public static final Keyword FORSY = new Keyword(65536 * 1024 + 34, "for");
 public static final Keyword DOSY = new Keyword(65536 * 1024 + 35, "do");
 public static final Keyword NOTSY = new Keyword(65536 * 1024 + 36, "!");
 public static final Keyword CONTINUESY = new Keyword(65536 * 1024 + 37, "continue");
 public static final Keyword TRYSY = new Keyword(65536 * 1024 + 38, "try");
 public static final Keyword CATCHSY = new Keyword(65536 * 1024 + 39, "catch");
 public static final Keyword FINALLYSY = new Keyword(65536 * 1024 + 40, "finally");
 public static final Keyword SWITCHSY = new Keyword(65536 * 1024 + 41, "switch");
 public static final Keyword THROWSY = new Keyword(65536 * 1024 + 42, "throw");
 public static final Keyword COMMASY = new Keyword(65536 * 1024 + 43, ",");
 public static final Keyword CASESY = new Keyword(65536 * 1024 + 44, "case");
 public static final Keyword DEFAULTSY = new Keyword(65536 * 1024 + 45, "default");
 public static final Keyword QUESTIONMARKSY = new Keyword(65536 * 1024 + 46, "?");
 public static final Keyword INSTANCEOFSY = new Keyword(65536 * 1024 + 47, "instanceof");
 public static final Keyword THISSY = new Keyword(65536 * 1024 + 48, "this");
 public static final Keyword SUPERSY = new Keyword(65536 * 1024 + 49, "super");
 public static final Keyword TRUESY = new Keyword(65536 * 1024 + 50, "true");
 public static final Keyword FALSESY = new Keyword(65536 * 1024 + 51, "false");
 public static final Keyword NULLSY = new Keyword(65536 * 1024 + 52, "null");
 public static final Keyword PLUSASSIGNSY = new Keyword(65536 * 1024 + 53, "+=");
 public static final Keyword MINUSASSIGNSY = new Keyword(65536 * 1024 + 54, "-=");
 public static final Keyword MULTASSIGNSY = new Keyword(65536 * 1024 + 55, "*=");
 public static final Keyword DIVASSIGNSY = new Keyword(65536 * 1024 + 56, "/=");
 public static final Keyword MODASSIGNSY = new Keyword(65536 * 1024 + 57, "%=");
 public static final Keyword ANDASSIGNSY = new Keyword(65536 * 1024 + 58, "&=");
 public static final Keyword ORASSIGNSY = new Keyword(65536 * 1024 + 59, "|=");
 public static final Keyword XORASSIGNSY = new Keyword(65536 * 1024 + 60, "^=");
 public static final Keyword RSHIFTASSIGNSY = new Keyword(65536 * 1024 + 61, ">>>=");
 public static final Keyword LSHIFTASSIGNSY = new Keyword(65536 * 1024 + 62, "<<=");
 public static final Keyword ASHIFTASSIGNSY = new Keyword(65536 * 1024 + 63, ">>=");
 public static final Keyword LOGICALORSY = new Keyword(65536 * 1024 + 64, "||");
 public static final Keyword LOGICALANDSY = new Keyword(65536 * 1024 + 65, "&&");
 public static final Keyword RSHIFTSY = new Keyword(65536 * 1024 + 66, ">>>");
 public static final Keyword LSHIFTSY = new Keyword(65536 * 1024 + 67, "<<");
 public static final Keyword ASHIFTSY = new Keyword(65536 * 1024 + 68, ">>");
 public static final Keyword INVERTSY = new Keyword(65536 * 1024 + 69, "~");
 public static final Keyword DNUMBERSY = new Keyword(65536 * 1024 + 70, " floating number");
 public static final Keyword STRINGSY = new Keyword(65536 * 1024 + 71, " string");
 public static final Keyword CHARCONSTSY = new Keyword(65536 * 1024 + 72, " character constant");
 public static final Keyword LNUMBERSY = new Keyword(65536 * 1024 + 73, " long number");
 public static final Keyword XORSY = new Keyword(65536 * 1024 + 74, "^");
 public static final Keyword EOFSY = new Keyword(65536 * 1024 + 75, " EOF");
 public static final Keyword PACKAGESY = new Keyword(65536 * 1024 + 76, "package");
 public static final Keyword IMPORTSY = new Keyword(65536 * 1024 + 77, "import");
 public static final Keyword EXTENDSSY = new Keyword(65536 * 1024 + 78, "extends");
 public static final Keyword IMPLEMENTSSY = new Keyword(65536 * 1024 + 79, "implements");
 public static final Keyword LEAFSY = new Keyword(65536 * 1024 + 80, "leaf");
 public static final Keyword ASSERTSY = new Keyword(65536 * 1024 + 81, "assert");
 public static final Keyword CASTSY = new Keyword(65536 * 1024 + 82, "type cast");
 public static final Keyword EXCEPTIONSY = new Keyword(65536 * 1024 + 83, "exception parameter");
 public static final Keyword LITERALSY = new Keyword(65536 * 1024 + 84, "literal");
 public static final Keyword CLEARSY = new Keyword(65536 * 1024 + 85, "clear data stack");
 public static final Keyword EXITSY = new Keyword(65536 * 1024 + 86, "purge local variables");
 public static final Keyword PUSHSY = new Keyword(65536 * 1024 + 87, "push scope");
 public static final Keyword POPSY = new Keyword(65536 * 1024 + 88, "pop scope");
 public static final Keyword ALLOCATESY = new Keyword(65536 * 1024 + 89, "allocate object");
 public static final Keyword IDENTSY = new Keyword(65536 * 1024 + 90, " ident");
 public static final Keyword THROWSSY = new Keyword(65536 * 1024 + 91, "throws");
 public static final Keyword COMMENTSY = new Keyword(65536 * 1024 + 92, " comment");
 public static final Keyword SIMPLECOMMENTSY = new Keyword(65536 * 1024 + 93, " simple comment");
 public static final Keyword LOCKSY = new Keyword(65536 * 1024 + 94, " lock");
 public static final Keyword UNLOCKSY = new Keyword(65536 * 1024 + 95, " unlock");
 public static final Keyword FINALASSIGNSY = new Keyword(65536 * 1024 + 96, " =");
 public static final Keyword BLOCKENDSY = new Keyword(65536 * 1024 + 97, " block end");
 public static final Keyword LABELSY = new Keyword(65536 * 1024 + 98, " label");

 public static final Keyword BYTESY = new Keyword(65536, "byte");
 public static final Keyword SHORTSY = new Keyword(65536 * 2, "short");
 public static final Keyword CHARSY = new Keyword(65536 * 4, "char");
 public static final Keyword INTSY = new Keyword(65536 * 8, "int");
 public static final Keyword LONGSY = new Keyword(65536 * 16, "long");
 public static final Keyword FLOATSY = new Keyword(65536 * 32, "float");
 public static final Keyword DOUBLESY = new Keyword(65536 * 64, "double");
 public static final Keyword BOOLEANSY = new Keyword(65536 * 128, "boolean");
 public static final Keyword VOIDSY = new Keyword(65536 * 256, "void");
 public static final Keyword CONSTRUCTORSY = new Keyword(65536 * 512, " constructor");

 public static final Keyword PUBLICSY = new Keyword(Modifier.PUBLIC, "public");
 public static final Keyword PRIVATESY = new Keyword(Modifier.PRIVATE, "private");
 public static final Keyword PROTECTEDSY = new Keyword(Modifier.PROTECTED, "protected");
 public static final Keyword STATICSY = new Keyword(Modifier.STATIC, "static");
 public static final Keyword FINALSY = new Keyword(Modifier.FINAL, "final");
 public static final Keyword SYNCHRONIZEDSY = new Keyword(Modifier.SYNCHRONIZED, "synchronized");
 public static final Keyword VOLATILESY = new Keyword(Modifier.VOLATILE, "volatile");
 public static final Keyword TRANSIENTSY = new Keyword(Modifier.TRANSIENT, "transient");
 public static final Keyword NATIVESY = new Keyword(Modifier.NATIVE, "native");
 public static final Keyword ABSTRACTSY = new Keyword(Modifier.ABSTRACT, "abstract");
 public static final Keyword STRICTFPSY = new Keyword(Modifier.STRICT, "strictfp");
 public static final Keyword VARIABLESY = new Keyword(4096, " variable");
 public static final Keyword METHODSY = new Keyword(8192, " method");
 public static final Keyword INTERFACESY = new Keyword(16384, "interface");
 public static final Keyword CLASSSY = new Keyword(32768, "class");


 /**
  * @return the string table for this enumeration value's enumeration class
  */
 protected Set getStringTable()
  {
   return table.keySet();
  }

 /**
  * @return the enumeration value table for this enumeration value's enumeration class
  */
 protected Enumeration getEnumValueTable()
  {
   return table.elements();
  }

  /**
   * get keyword matching parameter
   * @param x String
   * @return Keyword if present, otherwise null
   */
 public static Keyword get(String x)
  {
   if (x.compareTo("!<") == 0)
    return GREATEREQUALSY;
   else if (x.compareTo("!>") == 0)
    return LESSEQUALSY;
   else
    return (Keyword)table.get(x);
  }

  /**
   * get Keyword associated with given value
   * @param value
   * @return Keyword
   */
  public static Keyword get(int value)
   {
    Iterator iter = table.values().iterator();

    while(iter.hasNext())
     {
      Keyword x = (Keyword)iter.next();
      if (x.value == value)
       return x;
     }

    return null;
   }
 }
