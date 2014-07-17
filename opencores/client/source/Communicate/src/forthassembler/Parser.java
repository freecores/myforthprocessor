package forthassembler;


/**
 * <p>Überschrift: der Parser</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */


import java.io.*;
import java.util.*;

public class Parser
{
 private static int labelno;
 private static Token t;     // last processed token
 private static Token la;    // current token
 private static int sy;
 private static int countToken;
 private static Hashtable labels, localVar;
 private static Vector catches;
 private static boolean rom, exitTrue;
 private static Vector crossreference;
 private static int stringStorage;

 /**
  *  parse the token stream
  *  @param romable as result vhdl-code is written, if true, a deck otherwise
  *  @param name    name of resulting file
  */
 public static void parse(boolean romable, String name)
  {    // Starten der Analyse
   int [] terminalsy = new int[3];
   terminalsy[0] = Scanner.none;
   localVar = new Hashtable();
   crossreference = new Vector();
   rom = romable;
   exitTrue = false;
   labels = new Hashtable(32767);
   countToken = 0;
   labelno = 0;
   stringStorage = 0;
   Code.setLocalOffset(0);
   scan();

   if (!romable)
    {  // insert module name
     String module = name.substring(name.lastIndexOf("\\") + 1);

     if (sy == Scanner.modName)
      { // name of modul defined
       scan();
       module = la.ident;
       scan();
      }

     Code.resident(true);
     Code.append((short)module.length());
     for(int i = 0; i < module.length(); i++)
      Code.append((byte)module.charAt(i));
    }

   // parse token
   do
    {
     Code.resident(false);
     terminalsy[1] = Scanner.colon;
     terminalsy[2] = Scanner.colonlocal;
     statements(terminalsy, null, null);
     if (sy == Scanner.colon || sy == Scanner.colonlocal)
      {
       Code.resident(true);
       terminalsy[2] = terminalsy[1] = terminalsy[0];
       colonDefinition(terminalsy);
       scan();
      }
    }
   while(sy != Scanner.none);

   removeLocals(localVar, Integer.MAX_VALUE);

   if (Code.localOffset() != 0)
    error("Return-stack not balanced, difference is " + Code.localOffset());

   if (Errors.count() == 0)
    {
     Code.resident(false);
     Code.appendCommand((byte)Scanner.exit);
     Code.appendText("EXIT");
     Code.resident(true);
     insertImmediate();
     // resolve open references
     resolveLabels();
     // write deck or vhdl code
     int unresolved = writeObjectDeck(name);
     // write listing
     try
      {
       FileWriter file = new FileWriter(new File(name + ".lst"));
       Code.writeDASM(file, romable, unresolved, stringStorage);
       printList(file, "\r\nreferences:", crossreference);
       file.close();
/*       FileOutputStream f = new FileOutputStream(new File(name + ".diff"));
       FileInputStream r1 = new FileInputStream(new File(name + ".rom"));
       FileInputStream r2 = new FileInputStream(new File(name + "old.rom"));
       int rb1, rb2;
       String rx;
       int lines = 0;
       do
       {
        rx = "";
        do
        {
         rb1 = r1.read();
         rx += (char)rb1;
         rb2 = r2.read();
        }
        while(rb1 == rb2 && rb1 >= 0 && rb2 >= 0 && (rb1 != '\n' || rb2 != '\n'));
        if (rb1 != rb2)
        {
         while(rb1 >= 0 && rb1 != '\n')
         {
          rb1 = r1.read();
          rx += (char)rb1;
         }
         rx = lines + " " + rx;
         f.write(rx.getBytes());
         while(rb2 >= 0 && rb2 != '\n')
          rb2 = r2.read();
        }
        lines++;
       }
       while(rb1 >= 0 && rb2 >= 0);
       r1.close();
       r2.close();
       rx = lines + "";
       f.write(rx.getBytes());
       f.close();*/
      }
     catch (Exception ex) { ex.printStackTrace(); }
    }
  }

 /**
  * remove locals
  * @param table Hashtable
  * @param count number of variables to remove
  */
 private static void removeLocals(Hashtable table, int count)
  {
   count = Math.min(table.size(), count);
   int size = 0;
   
   for(int i = 0; i < count; i++)
    {
     Iterator iter = table.entrySet().iterator();
     Map.Entry e = (Map.Entry)iter.next();

     while(iter.hasNext())
      {
       Map.Entry a = (Map.Entry)iter.next();

       if (Math.abs(((Integer)a.getValue()).intValue()) > Math.abs(((Integer)e.getValue()).intValue()))
        {
         e = a;
        }
      }

     size += ((Integer)e.getValue()).intValue() >= 0?1:2;
     table.remove((String)e.getKey());
     Code.appendText("remove " + (String)e.getKey());
    }

   // remove locals
   while(size > 1)
    {
     Code.appendCommand( (byte) (128 + Scanner.twodrop));
     Code.appendText("A:2DROP");
     size -= 2;
    }

   if (size > 0)
    {
     Code.appendCommand( (byte) (128 + Scanner.drop));
     Code.appendText("A:DROP");
    }
  }

  /** get immediate expression
   *  @return Token number or floating
   */
  private static Token build()
   {
    Vector v = new Vector();
    int count = 1;

    do
     {
      scan();

      if (sy == Scanner.build)
       count++;
      else if (sy == Scanner.does)
       count--;
      else
       v.add(0, la);
     }
    while(count > 0);

    return evaluate(v);
   }

   /**
    * evaluate expression stack
    * @param expression expression stack
    * @return calculated expression
    */
   private static Token evaluate(Vector expression)
    {
     Token operator;
     Token a, b;

     if (expression.isEmpty())
      {
       error(" no valid expression!");
       return new Token();
      }

     operator = (Token)expression.get(0);
     expression.remove(0);

     switch(operator.kind)
      {
       case Scanner.add:
        a = evaluate(expression);
        b = evaluate(expression);
        if (a.kind != Scanner.number && b.kind != Scanner.number)
         error("operands not both integer");
        b.val += a.val;
        return b;
       case Scanner.sub:
        a = evaluate(expression);
        b = evaluate(expression);
        if (a.kind != Scanner.number && b.kind != Scanner.number)
         error("operands not both integer");
        b.val -= a.val;
        return b;
       case Scanner.inc:
        b = evaluate(expression);
        if (b.kind != Scanner.number)
         error("operand not an integer");
        b.val++;
        return b;
       case Scanner.dec:
        b = evaluate(expression);
        if (b.kind != Scanner.number)
         error("operand not an integer");
        b.val--;
        return b;
       case Scanner.twomul:
        b = evaluate(expression);
        if (b.kind != Scanner.number)
         error("operand not an integer");
        b.val *= 2;
        return b;
       case Scanner.twodiv:
        b = evaluate(expression);
        if (b.kind != Scanner.number)
         error("operand not an integer");
        b.val /= 2;
        return b;
       case Scanner.and:
        a = evaluate(expression);
        b = evaluate(expression);
        if (a.kind != Scanner.number && b.kind != Scanner.number)
         error("operands not both integer");
        b.val &= a.val;
        return b;
       case Scanner.cellp:
        b = evaluate(expression);
        if (b.kind != Scanner.number)
         error("operand not an integer");
        b.val += 4;
        return b;
       case Scanner.halfp:
        b = evaluate(expression);
        if (b.kind != Scanner.number)
         error("operand not an integer");
        b.val += 2;
        return b;
       case Scanner.invert:
        b = evaluate(expression);
        if (b.kind != Scanner.number)
         error("operand not an integer");
        b.val = ~b.val;
        return b;
       case Scanner.lshift:
        a = evaluate(expression);
        b = evaluate(expression);
        if (a.kind != Scanner.number && b.kind != Scanner.number)
         error("operands not both integer");
        b.val <<= a.val;
        return b;
       case Scanner.rshift:
        a = evaluate(expression);
        b = evaluate(expression);
        if (a.kind != Scanner.number && b.kind != Scanner.number)
         error("operands not both integer");
        b.val >>= a.val;
        return b;
       case Scanner.or:
        a = evaluate(expression);
        b = evaluate(expression);
        if (a.kind != Scanner.number && b.kind != Scanner.number)
         error("operands not both integer");
        b.val |= a.val;
        return b;
       case Scanner.xor:
        a = evaluate(expression);
        b = evaluate(expression);
        if (a.kind != Scanner.number && b.kind != Scanner.number)
         error("operands not both integer");
        b.val ^= a.val;
        return b;
       case Scanner.number:
       case Scanner.floating:
        return operator;
       case Scanner.ident:
        if (operator.ident.compareTo("D>F") == 0)
         {
          b = evaluate(expression);
          if (b.kind != Scanner.number)
           error("operand not an integer");
          b.kind = Scanner.floating;
          b.d = (double)b.val;
          return b;
         }
        else if (operator.ident.compareTo("F>D") == 0)
         {
          b = evaluate(expression);
          if (b.kind != Scanner.floating)
           error("operand not a double");
          b.kind = Scanner.number;
          b.val = (long)b.d;
          return b;
         }
        else if (operator.ident.compareTo("FABS") == 0)
         {
          b = evaluate(expression);
          if (b.kind != Scanner.floating)
           error("operand not a double");
          b.kind = Scanner.floating;
          b.d = Math.abs(b.d);
          return b;
         }
        else if (operator.ident.compareTo("FLOOR") == 0)
         {
          b = evaluate(expression);
          if (b.kind != Scanner.floating)
           error("operand not a double");
          b.kind = Scanner.floating;
          b.d = Math.floor(b.d);
          return b;
         }
        else if (operator.ident.compareTo("FROUND") == 0)
         {
          b = evaluate(expression);
          if (b.kind != Scanner.floating)
           error("operand not a double");
          b.kind = Scanner.floating;
          b.d = Math.ceil(b.d);
          return b;
         }
        else if (operator.ident.compareTo("F**") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.floating && b.kind != Scanner.floating)
           error("operands not both double");
          b.d = Math.pow(b.d, a.d);
          return b;
         }
        else if (operator.ident.compareTo("F/") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.floating && b.kind != Scanner.floating)
           error("operands not both double");
          b.d /= a.d;
          return b;
         }
        else if (operator.ident.compareTo("F*") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.floating && b.kind != Scanner.floating)
           error("operands not both double");
          b.d *= a.d;
          return b;
         }
        else if (operator.ident.compareTo("F-") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.floating && b.kind != Scanner.floating)
           error("operands not both double");
          b.d -= a.d;
          return b;
         }
        else if (operator.ident.compareTo("F+") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.floating && b.kind != Scanner.floating)
           error("operands not both double");
          b.d += a.d;
          return b;
         }
        else if (operator.ident.compareTo("FMIN") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.floating && b.kind != Scanner.floating)
           error("operands not both double");
          b.d = Math.min(a.d, b.d);
          return b;
         }
        else if (operator.ident.compareTo("FMAX") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.floating && b.kind != Scanner.floating)
           error("operands not both double");
          b.d = Math.max(a.d, b.d);
          return b;
         }
        else if (operator.ident.compareTo("*") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.number && b.kind != Scanner.number)
           error("operands not both integer");
          b.val *= a.val;
          return b;
         }
        else if (operator.ident.compareTo("/") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.number && b.kind != Scanner.number)
           error("operands not both integer");
          b.val /= a.val;
          return b;
         }
        else if (operator.ident.compareTo("MOD") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.number && b.kind != Scanner.number)
           error("operands not both integer");
          b.val %= a.val;
          return b;
         }
        else if (operator.ident.compareTo("MIN") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.number && b.kind != Scanner.number)
           error("operands not both integer");
          b.val = Math.min(a.val, b.val);
          return b;
         }
        else if (operator.ident.compareTo("MAX") == 0)
         {
          a = evaluate(expression);
          b = evaluate(expression);
          if (a.kind != Scanner.number && b.kind != Scanner.number)
           error("operands not both integer");
          b.val = Math.max(a.val, b.val);
          return b;
         }
        else if (operator.ident.compareTo("ABS") == 0)
         {
          b = evaluate(expression);
          if (b.kind != Scanner.number)
           error("operand not an integer");
          b.val = Math.abs(b.val);
          return b;
         }
       default:
        error(operator.kind + " is no valid operator!" + operator.ident);
      }

     return new Token();
    }

 /**
  * get next token from stream
  * sy       new token symbol
  * la       new token
  */
 private static void scan()
  {
   t = la;
   la = Scanner.next(true);
   sy = la.kind;
   countToken++;
  }

 /**
  * process error message
  * @param msg the message
  */
 public static void error(String msg)
  {
   Errors.println(Scanner.source.sourceFile,la.line,la.col,msg, true);
  }

 /**
  * process error message
  * @param msg the message
  */
 public static void warning(String msg)
  {
   Errors.println(Scanner.source.sourceFile,la.line,la.col,msg, false);
  }

 /**
  * begin-statement
  * @param terminalsy list of terminating symbols
  * @param exitLabel exit label
  */
 private static void beginStatement(int [] terminalsy, String exitLabel)
  {
   int behind = 0, before = Code.localOffset();
   String t1, t2;
   boolean alternate = la.alternate;
   int [] beginterminalsy = new int[terminalsy.length + 3];

   beginterminalsy[2] = Scanner.until;
   beginterminalsy[1] = Scanner.again;
   beginterminalsy[0] = Scanner.whilesy;
   for(int i = 0; i < terminalsy.length; i++)
    beginterminalsy[i + 3] = terminalsy[i];

   t1 = "§" + labelno++;
   t2 = "§" + labelno++;
   Scanner.insert(false, t1 + " LABEL");

   scan();
   statements(beginterminalsy, t2, exitLabel);

   if (sy == Scanner.until)
    {
     Scanner.insert(alternate, t1 + " 0BRANCH " + t2 + " LABEL");
     behind = Code.localOffset() - (alternate?1:0);
    }
   else if (sy == Scanner.whilesy)
    {
     Scanner.insert(alternate, t2 + " 0BRANCH");
     beginterminalsy[2] = beginterminalsy[1] = beginterminalsy[0] = Scanner.repeat;

     scan();
     statements(beginterminalsy, t2, exitLabel);

     if (sy != Scanner.repeat)
      error("REPEAT expected");

     Scanner.insert(false, t1 + " BRANCH " + t2 + " LABEL");
     behind = Code.localOffset();
    }
   else if (sy == Scanner.again)
    {
     Scanner.insert(false, t1 + " BRANCH " + t2 + " LABEL");
     behind = Code.localOffset();
    }

   if (before != behind)
    error("unbalanced return stack in BEGIN-loop, difference is " + (behind - before));
  }

 /**
  * do-statement
  * @param terminalsy list of terminating symbols
  * @param exitLabel exit label
  */
 private static void doStatement(int [] terminalsy, String exitLabel)
  {
   String t1, t2, t3;
   boolean alternate = !la.alternate;
   int [] doterminalsy = new int[terminalsy.length + 2];
   int before = Code.localOffset() + (alternate?2:-2);

   doterminalsy[1] = Scanner.loop;
   doterminalsy[0] = Scanner.ploop;
   for(int i = 0; i < terminalsy.length; i++)
    doterminalsy[i + 2] = terminalsy[i];

   t1 = "§" + labelno++;
   t2 = "§" + labelno++;
   t3 = "§" + labelno++;

   // note: reversed insert order necessary!
   Scanner.insert(false, t1 + " LABEL");
   if (sy == Scanner.doq)
    Scanner.insert(false, t3 + " BRANCH");
   Scanner.insert(alternate, "R1@ R@ A:2DROP");

   scan();
   statements(doterminalsy, t2, exitLabel);

   // note: reversed insert order necessary!
   Scanner.insert(alternate, t3 + " LABEL >! INVERT " + t1 + " 0BRANCH 2DROP " + t2 + " LABEL");

   if (sy == Scanner.loop)
    Scanner.insert(alternate, "1+");
   else if (sy == Scanner.ploop)
    Scanner.insert(alternate, "R> +");
   else
    error("missing LOOP-statement");
   scan();

   if (before != Code.localOffset())
    error("unbalanced return stack in DO-loop, difference is " + (Code.localOffset() - before));
  }

 /**
  * if-statement
  * @param terminalsy list of terminating symbols
  * @param breakLabel break label
  * @param exitLabel exit label
  */
 private static void ifStatement(int [] terminalsy, String breakLabel, String exitLabel)
  {
   int afterThen, afterElse, before;
   boolean exitThen, exitElse = false;
   String t1;
   boolean alternate = la.alternate;
   int [] ifterminalsy = new int[terminalsy.length + 3];

   ifterminalsy[2] = Scanner.endif;
   ifterminalsy[1] = Scanner.then;
   ifterminalsy[0] = Scanner.elsesy;
   for(int i = 0; i < terminalsy.length; i++)
    ifterminalsy[i + 3] = terminalsy[i];

   afterElse = before = Code.localOffset() - (alternate?1:0);
   exitTrue = false;

   t1 = "§" + labelno++;
   Scanner.insert(alternate, t1 + " 0BRANCH");
   scan();
   statements(ifterminalsy, breakLabel, exitLabel);

   afterThen = Code.localOffset();
   exitThen = exitTrue;
   exitTrue = false;

   if (sy == Scanner.elsesy)
    {
     String t2;

     Code.setLocalOffset(before);

     t2 = "§" + labelno++;
     exitTrue = false;
     Scanner.insert(false, t2 + " BRANCH " + t1 + " LABEL");

     t1 = t2;
     ifterminalsy[0] = ifterminalsy[1];
     scan();
     statements(ifterminalsy, breakLabel, exitLabel);
     afterElse = Code.localOffset();
     exitElse = exitTrue;
     exitTrue = false;
    }

   if (exitThen && exitElse)
    Code.setLocalOffset(before);
   else if (exitThen && !exitElse)
    Code.setLocalOffset(afterElse);
   else if (!exitThen && exitElse)
    Code.setLocalOffset(afterThen);
   else if (afterThen != afterElse)
    error(alternate + "misbalanced return stack in IF-statement, difference is " + (afterThen - afterElse));

   Scanner.insert(false, t1 + " LABEL");

   if (sy != Scanner.then && sy != Scanner.endif)
    error("missing endif, respectively then");
   else
    scan();
  }

 /**
  * case-statement
  * @param terminalsy list of terminating symbols
  * @param breakLabel break label
  * @param exitLabel exit label
  */
 private static void caseStatement(int [] terminalsy, String breakLabel, String exitLabel)
  {
   int before, marker, offset;
   String t1, t2;
   boolean alternate = la.alternate;
   int [] caseterminalsy = new int[terminalsy.length + 3];

   caseterminalsy[2] = Scanner.endcase;
   caseterminalsy[1] = Scanner.elsesy;
   caseterminalsy[0] = Scanner.of;
   for(int i = 0; i < terminalsy.length; i++)
    caseterminalsy[i + 3] = terminalsy[i];

   t2 = "§" + labelno++;

   // placeholder for drop
   marker = Code.last();
   offset = 0;
   Scanner.insert(alternate, "NOP");

   scan();
   statements(caseterminalsy, breakLabel, exitLabel);

   while(sy != Scanner.endcase)
    {
     if (sy != Scanner.of && sy != Scanner.elsesy)
      error("OF expected, not " + sy);

     t1 = "§" + labelno++;

     if (sy == Scanner.of)
      {
       caseterminalsy[0] = Scanner.endof;
       Scanner.insert(alternate, "=! NIP " + t1 + " 0BRANCH DROP");
      }
     else
      caseterminalsy[0] = caseterminalsy[1] = Scanner.endcase;

     before = Code.localOffset();
     exitTrue = false;
     scan();
     statements(caseterminalsy, breakLabel, exitLabel);

     if (sy != caseterminalsy[0])
      error("ENDOF expected, not " + sy);

     if (Code.localOffset() != before && !exitTrue)
      error("return stack unbalanced in CASE-statement, difference is " + (Code.localOffset() - before));

     if (caseterminalsy[0] == Scanner.endcase)
      break;

     marker = Code.last();
     offset = 3;

     Code.setLocalOffset(before);
     Scanner.insert(alternate, t2 + " BRANCH " + t1 + " LABEL NOP");
     scan();

     caseterminalsy[0] = Scanner.of;
     statements(caseterminalsy, breakLabel, exitLabel);
    }

   // remove case parameter from stack
   Code.replace(marker, offset, (byte)((alternate?128:0) + Scanner.drop));
   if (alternate)
    Code.setLocalOffset(Code.localOffset() - 1);

   Scanner.insert(false, t2 + " LABEL");
  }

 /**
  * colon-definition
  * @param terminalsy list of terminating symbols
  */
 private static void colonDefinition(int [] terminalsy)
  {
   Hashtable saved = localVar;
   int oldOffset = Code.localOffset();
   String epilog = "", returns = "§return";
   Token t1 = new Token();
   int [] colonterminalsy = new int[terminalsy.length + 1];

   colonterminalsy[0] = Scanner.semicolon;

   for(int i = 0; i < terminalsy.length; i++)
    colonterminalsy[i + 1] = terminalsy[i];

   catches = new Vector();
   localVar = new Hashtable();
   localVar.put(returns, new Integer(Code.localOffset()));
   String exitLabel = "§" + labelno++;
   scan();

   if (sy != Scanner.ident)
    error("only named code definition allowed, not " + sy + " " + la.ident);
   else
    {
     Code.newLine();
     Code.newLine();
     Code.appendText(la.ident + " LABEL");
     insertLabel(Code.last(), la, true);

     if (t.kind == Scanner.colon)
      epilog = la.ident + " VAL PROCEDURE " + la.ident;
    }

   exitTrue = false;
   scan();

   statements(colonterminalsy, null, exitLabel);

   t1.kind = Scanner.ident;
   t1.ident = exitLabel;
   Code.newLine();
   Code.appendText(t1.ident + " LABEL");
   insertLabel(Code.last(), t1, false);

   localVar.remove(returns);
   removeLocals(localVar, Integer.MAX_VALUE);
   localVar = saved;

   int size = Code.localOffset() - oldOffset;

   if (size != 0)
    error("stack unbalanced, difference is " + size);

   if (sy != Scanner.semicolon)
    error("semicolon expected, not " + sy);
   else
    {
     Code.appendCommand((byte)((la.alternate?128:0) + Scanner.exit));
     Code.appendText((la.alternate?"A:":"") + "EXIT");
     insertImmediate();
    }

   exitTrue = false;
   if (!catches.isEmpty())
    error("dangling catch or missing throw");

   Scanner.insert(false, epilog);
  }

 /**
  * the grammar rules of the assembler
  * @param breakLabel label for loop leave instruction
  * @param terminalsy list of terminating symbols
  * @param exitLabel exit label
  */
 private static void statements(int [] terminalsy, String breakLabel, String exitLabel)
  {
   node y;
   int alternate;
   long longdouble;
   Integer n;

   for(;;)
    {
     // terminating symbol read?
     for(int i = 0; i < terminalsy.length; i++)
      if (sy == terminalsy[i])
       return;

     alternate = la.alternate?128:0;

     if (sy < 128 && sy >= 0)
      {
       // take credits to possibly declared locals
       if (sy == Scanner.exit)
        {
         exitTrue = true;
         if ((Integer)localVar.get("§return") != null && localVar.size() > 1)
          {
           sy = Scanner.branch;
           t.kind = Scanner.ident;
           t.ident = exitLabel;
          }
        }

       // simple instruction
       switch(sy)
        {
         case Scanner.trap:
          Code.appendCommand((byte)(alternate + sy));
          if (t.kind == Scanner.number)
           {
            Code.append((int)t.val);
            Code.appendText(t.val + (alternate != 0?" A:":" ") + "TRAP");
           }
          else if (t.kind == Scanner.ident)
           {
            Code.appendText(t.ident + (alternate != 0?" A:":" ") + "TRAP");
            insertLabelList(Code.last(), t, 4);
            Code.append(0);
           }
          else
           error("illegal operand");
          break;
         case Scanner.val:
          if (t.kind == Scanner.number)
           Code.append(alternate != 0, t.val);
          else if (t.kind == Scanner.ident)
           {
            Code.appendCommand((byte)(alternate + sy));
            Code.appendText(t.ident + (alternate != 0?" A:":" ") + "VAL");
            insertLabelList(Code.last(), t, 4);
            Code.append(0);
           }
          else
           error("illegal operand");
          break;
         case Scanner.zerobranchn:
         case Scanner.zerobranch:
         case Scanner.call:
         case Scanner.branch:
          Code.appendCommand((byte)(alternate + sy));
          if (t.kind == Scanner.number)
           {
            Code.append((short)t.val);
            Code.appendText(t.val + (alternate != 0?" A:":" ") + la.ident);
           }
          else if (t.kind == Scanner.ident)
           {
            Code.appendText(t.ident + (alternate != 0?" A:":" ") + la.ident);
            insertLabelList(Code.last(), t, 2);
            Code.append((short)0);
           }
          else
           error("invalid branch destinaton");
          break;
         case Scanner.switchsy:
         case Scanner.setpc:
          Code.appendCommand((byte)(alternate + sy));
          if (t.kind == Scanner.number)
           {
            Code.append((short)t.val);
            Code.appendText(t.val + (alternate != 0?" A:":" ") + la.ident);
           }
          else
           error("number expected");
          break;
         default:
          Code.appendCommand((byte)(alternate + sy));
          Code.appendText((alternate != 0?"A:":"") + la.ident);
          break;
        }
       scan();
      }
     else switch(sy)
      {
       case Scanner.local:
        // allocate a local variable
        scan();
        if ((Integer)localVar.get(la.ident) != null)
         warning("redeclaration of local variable " + la.ident);
        Code.appendCommand( (byte) (128 + Scanner.rat));
        Code.appendText("A:R@ ( " + la.ident + " declared )");
        Code.appendCommand( (byte) Scanner.drop);
        Code.appendText("DROP");
        localVar.put(la.ident, new Integer(Code.localOffset()));
        scan();
        break;
       case Scanner.locals:
        // allocate some local variables
        scan();
        while (sy == Scanner.ident && la.ident.compareTo("|") != 0)
         {
          if ((Integer)localVar.get(la.ident) != null)
           warning("redeclaration of local variable " + la.ident);
          Code.appendCommand( (byte) (128 + Scanner.rat));
          Code.appendText("A:R@ ( " + la.ident + " declared )");
          Code.appendCommand( (byte) Scanner.drop);
          Code.appendText("DROP");
          localVar.put(la.ident, new Integer(Code.localOffset()));
          scan();
         }
        if (la.ident.compareTo("|") != 0)
         error("delimiting \'|\' expected");
        else
         scan();
        break;
       case Scanner.twolocal:
        // allocate a local variable
        scan();
        if ((Integer)localVar.get(la.ident) != null)
         warning("redeclaration of local variable " + la.ident);
        Code.appendCommand( (byte) (128 + Scanner.r1at));
        Code.appendText("A:R1@ ( " + la.ident + " declared )");
        Code.appendCommand( (byte) (128 + Scanner.rat));
        Code.appendText("A:R@");
        Code.appendCommand( (byte) Scanner.twodrop);
        Code.appendText("2DROP");
        localVar.put(la.ident, new Integer(-Code.localOffset()));
        scan();
        break;
       case Scanner.twolocals:
        // allocate some local long variables
        scan();
        while (sy == Scanner.ident && la.ident.compareTo("|") != 0)
         {
          if ((Integer)localVar.get(la.ident) != null)
           warning("redeclaration of local variable " + la.ident);
          Code.appendCommand( (byte) (128 + Scanner.r1at));
          Code.appendText("A:R1@ ( " + la.ident + " declared )");
          Code.appendCommand( (byte) (128 + Scanner.rat));
          Code.appendText("A:R@");
          Code.appendCommand( (byte) Scanner.twodrop);
          Code.appendText("2DROP");
          localVar.put(la.ident, new Integer(-Code.localOffset()));
          scan();
         }
        if (la.ident.compareTo("|") != 0)
         error("delimiting \'|\' expected");
        else
         scan();
        break;
       case Scanner.purge:
        // remove last locals
        scan();
        if (sy != Scanner.number)
         error("number expected");
        else
         {
          removeLocals(localVar, (int) la.val);
          scan();
         }
        break;
       case Scanner.aCatch:
        if (catches.isEmpty())
         {
          catches.add(0, new String("§" + labelno));
          Scanner.insert(false, "§" + labelno++ + " LABEL");
         }
        else
         {
          Scanner.insert(false, (String)catches.get(0) + " LABEL");
          catches.remove(0);
         }
        scan();
        break;
       case Scanner.aThrow:
        if (catches.isEmpty())
         {
          catches.add(0, new String("§" + labelno));
          Scanner.insert(false, "§" + labelno++ + " BRANCH");
         }
        else
         {
          Scanner.insert(false, (String)catches.get(0) + " BRANCH");
          catches.remove(0);
         }
        scan();
        break;
       case Scanner.unicodeString:
        byte [] doubled = new byte[la.string.length() * 2];
        for(int j = 0; j < la.string.length(); j++)
         {
          char h = la.string.charAt(j);
          doubled[j * 2] = (byte)h;
          doubled[j * 2 + 1] = (byte)(h >> 8);
         }
        la.string = new String(doubled);
       case Scanner.imString:
       case Scanner.countedString:
        Code.appendCommand((byte)(alternate + Scanner.val));
        if ((y = searchText(la)) != null)
         la.ident = y.t.ident;
        else
         {
          la.ident = "§" + labelno++;
          stringStorage += la.string.length() + 2;
         }
        insertLabelList(Code.last(), la, 4);
        Code.append(0);
        Code.appendText(la.ident + (alternate != 0?" A:":" ") + "VAL");
        if (sy == Scanner.imString)
         Scanner.insert(alternate != 0, "COUNT");
        scan();
        break;
       case Scanner.floating:
        longdouble = Double.doubleToRawLongBits(la.d);
        Code.append(alternate != 0, (int)(longdouble >> 32));
        Code.append(alternate != 0, (int)(longdouble & 0xffffffffL));
        scan();
        break;
       case Scanner.to:
        // TO-statement
        scan();
        if (sy == Scanner.number)
         {
          Code.append(alternate != 0, la.val);
         }
        else if ((n = (Integer)localVar.get(la.ident)) != null)
         {
          Code.appendText("( " + la.ident + " will be modified! )");
          int off = n.intValue();

          int offset = Code.localOffset() - Math.abs(off);

          if (offset == 0)
           Scanner.insert(false, (off < 0)?"A:2DROP A:R1@ A:R@ 2DROP":"A:DROP A:R>");
          else if (off < 0)
           Scanner.insert(false, "A:R1@ A:" + (offset + 1) + " A:PUT A:R@ A:" + offset + " A:PUT 2DROP" );
          else
           Scanner.insert(false, "A:R@ A:" + offset + " A:PUT DROP" );

          scan();
          break;
         }
        else
         {
          la.kind = Scanner.ident;
          Code.appendCommand((byte)(Scanner.val + alternate));
          Code.appendText(la.ident + (alternate != 0?" A:":" ") + "VAL ( will be modified! )");
          insertLabelList(Code.last(), la, 4);
          Code.append(0);
         }
        Code.appendCommand((byte)(Scanner.store + alternate));
        Code.appendText((alternate != 0?"A:":"") + "!");
        scan();
        break;
       case Scanner.ident:
        if ((n = (Integer)localVar.get(la.ident)) != null)
         {
          Code.appendText("( " + la.ident + " will be fetched! )");
          int off = n.intValue();
          int offset = Code.localOffset() - Math.abs(off);

          if (offset == 0)
           Scanner.insert(false, (off < 0)?"R1@ R@":"R@");
          else if (off < 0)
           Scanner.insert(false, (offset + 1) + " A:VAL A:PICK R> " + offset + " A:VAL A:PICK R>" );
          else
           Scanner.insert(false, offset + " A:VAL A:PICK R>" );

          scan();
          break;
         }
       case Scanner.number:
        scan();

        // only part of next instruction
        if (sy < 128 && sy >= 0)
         if (sy == Scanner.trap || sy == Scanner.val || sy == Scanner.branch ||
             sy == Scanner.zerobranch || sy == Scanner.zerobranchn || sy == Scanner.call ||
             sy == Scanner.switchsy || sy == Scanner.setpc)
         break;

        if (sy == Scanner.label)
         {
          Code.newLine();
          Code.appendText(t.ident + " LABEL");
          insertLabel(Code.last(), t, true); // label
          scan();
         }
        else if (t.kind == Scanner.number)
         if (sy == Scanner.org)
          {
           if (rom)
            Code.origin((int)t.val);

           scan();
          }
         else
          {
           Code.append(alternate != 0, t.val);
          }
        else
         {                                    // linkable reference
          y = (node)labels.get(t.ident);

          if (y == null || !y.proc && y.location == Integer.MIN_VALUE)
           {                                  // forward reference ??????????????????????????????????
            Code.appendCommand((byte)(alternate + Scanner.val));
            Code.appendText(t.ident + (alternate != 0?" A:":" ") + "VAL ( will be modified! )");
            insertLabelList(Code.last(), t, 6);
            Code.append(0);
            Code.appendCommand((byte)(alternate + Scanner.nop));
            Code.appendText((alternate != 0?"A:":"") + "NOP ( will be modified! )");
           }
          else if (y.proc)
           {                                  // procedure reference
            Code.appendCommand((byte)(alternate + Scanner.call));
            Code.appendText(t.ident + (alternate != 0?" A:":" ") + "CALL");
            insertLabelList(Code.last(), t, 2);
            Code.append((short)0);
           }
          else
           {                                  // known reference
            Code.appendCommand((byte)(alternate + Scanner.val));
            Code.appendText(t.ident + (alternate != 0?" A:":" ") + "VAL");
            insertLabelList(Code.last(), t, 4);
            Code.append(0);
           }
         }
        break;
       case Scanner.imChar:
        Code.append(alternate != 0, (int)(la.string.charAt(0) & 0xff));
        scan();
        break;
       case Scanner.ifsy:
        ifStatement(terminalsy, breakLabel, exitLabel);
        break;
       case Scanner.casesy:
        caseStatement(terminalsy, breakLabel, exitLabel);
        scan();
        break;
       case Scanner.doq:
       case Scanner.dosy:
        doStatement(terminalsy, exitLabel);
        break;
       case Scanner.leave:
        if (breakLabel == null)
         error("no preceeding DO or BEGIN");
        else
         {
          Scanner.insert(false, breakLabel + " BRANCH");
          exitTrue = true;
         }
        scan();
        break;
       case Scanner.begin:
        beginStatement(terminalsy, exitLabel);
        scan();
        break;
       case Scanner.build:
        la = build();
        la.alternate = alternate != 0;
        sy = la.kind;
        break;
       default:
        error("misplaced token " + la.ident + " with illegal symbol " + sy);
        scan();
        break;
      }
    }
  }

 /**
  * insert a reference to label t
  * @param pos  position in code table
  * @param t the referenced label
  * @param size size of reference in byte
  */
 private static void insertLabelList(int pos, Token t, int size)
  {
   node x = (node)labels.get(t.ident);
   list y = new list(t, pos, size);

   if (x == null)
    {
     // a forward reference
     x = new node();
     x.t = t;
     labels.put(t.ident, x);
    }
   // add reference
   x.list.addElement(y);
  }

 /**
  * define a label
  * @param pos Code position
  * @param t token defining label
  * @param proc true, if name of a procedure
  */
 private static void insertLabel(int pos, Token t, boolean proc)
  {
   node x = (node)labels.get(t.ident);

   if (x == null)
    x = new node();

   if (x.location != Integer.MIN_VALUE)
    error("multiple defined label");
   else
    {
     x.location = pos;
     x.t = t;
    }

   x.proc |= proc;
   labels.put(t.ident, x);
  }

 /**
  * search a distinct string constant
  * @param string Token
  * @return Token, if exists
  */
 private static node searchText(Token string)
  {
   node x;
   String label;
   Enumeration key = labels.keys();

   while(key.hasMoreElements() && string.string != null)
    {
     label = (String)key.nextElement();
     x = (node)labels.get(label);

     if (x.t.string != null && x.t.string.compareTo(string.string) == 0)
      return x;
    }

   return null;
  }

 /**
  * complete all open immediate references of label list, means add them to Code table
  */
 private static void insertImmediate()
  {
   String label;
   Enumeration key = labels.keys();

   while(key.hasMoreElements())
    {
     node x;
     int loc;

     label = (String)key.nextElement();
     x = (node)labels.get(label);

     if (x.t.kind == Scanner.imString || x.t.kind == Scanner.countedString || x.t.kind == Scanner.unicodeString)
      if (x.location == Integer.MIN_VALUE)
       {
        insertLabel(Code.last(), x.t, false);

        Code.newLine();
        Code.appendText(x.t.ident + " LABEL");
        Code.append((short)x.t.string.length());
        Code.newLine();
        Code.appendText("\" " + x.t.string + " \"");
        for(loc = 0; loc < x.t.string.length(); loc++)
         Code.append((byte)x.t.string.charAt(loc));
       }
    }
  }

 /**
  * resolve all references of label list, means modify Code table
  */
 private static void resolveLabels()
  {
   String label;
   Enumeration key = labels.keys();
   Vector list = new Vector();

   while(key.hasMoreElements())
    {
     node x;
     int addr = 0, loc, dummy;

     label = (String)key.nextElement();
     x = (node)labels.get(label);
     x.max = x.list.size();

     if (x.list == null && x.t.ident.charAt(0) != '§')
      Errors.println(x.t.sourceFile,x.t.line,x.t.col,label + " never referenced", true);
     if (x.location == Integer.MIN_VALUE && rom)
      Errors.println(x.t.sourceFile,x.t.line,x.t.col,label + " unresolved", true);

     if (x.location != Integer.MIN_VALUE)
      {
       addr = Code.address(rom, x.location);

       if (x.t.ident.charAt(0) != '§')
        list.add(x);

       for(int i = 0; x.list != null && i < x.list.size();)
        {
         list y;
         y = (list)x.list.elementAt(i);
         loc = Code.address(rom, y.address);

         switch(y.size)
          {
           case 2:
            dummy = addr - (loc + 2);

            if (dummy < -32768 || dummy > 32767)
             Errors.println(y.t.sourceFile,y.t.line,y.t.col,label + " out of range", true);
            Code.replace(y.address, 0, (short)(dummy));
            x.list.remove(i);
            break;
           case 4:
            Code.replace(y.address, 0, addr);
            i++;
            break;
           case 6:
            int z = y.address - (y.address < 0?-1:1);
            Code.replace(z, 0, (byte)((x.proc?Scanner.call:Scanner.val) | (Code.get(z) & 0x80)));
            if (x.proc)
             {
              dummy = addr - (loc + 2);
              if (dummy < -32768 || dummy > 32767)
               Errors.println(y.t.sourceFile,y.t.line,y.t.col,label + " out of range", true);
              Code.replace(z, 1, (short)(dummy));
              Code.replace(z, 3, (byte)Scanner.nop);
              Code.replace(z, 4, (byte)Scanner.nop);
              Code.replace(z, 5, (byte)Scanner.nop);
              x.list.remove(i);
             }
            else
             x.list.set(i++, new list(y.t, y.address, 4));
            break;
           default:
            Errors.println(y.t.sourceFile,y.t.line,y.t.col,label + " undefined reference", true);
            i++;
          }
        }

       if (rom || x.list.size() == 0)
        {
         x.list = null;
         labels.remove(label);
        }
      }
    }

   crossreference.addAll(list);
  }

 /**
  * print reference list
  * @param file destination file
  * @param header title
  * @param list a list of references
  */
 private static void printList(FileWriter file, String header, Vector list)
  {
   try { file.write(header + "\r\n"); }
   catch (IOException ex1) { ex1.printStackTrace(); }

   while(!list.isEmpty())
    {
     int min;
     node a, b;
     int addr;

     min = 0;
     a = (node)list.get(0);
     for(int i = 1; i < list.size(); i++)
      {
       b = (node)list.get(i);
       if (b.t.ident.compareTo(a.t.ident) < 0)
        {
         a = b;
         min = i;
        }
      }

     addr = a.location == Integer.MIN_VALUE?-1:Code.address(rom, a.location);
     try { file.write(Integer.toHexString(addr) + '\t' + (a.max) + '\t' + a.t.ident + "\r\n"); }
     catch (IOException ex) { ex.printStackTrace(); }
     list.remove(min);
    }
  }

 /**
  * write linkable object deck
  * @param name trunc of filename, ".rom" or ".obj" will be added
  * @return count of unresolved references
  */
 private static int writeObjectDeck(String name)
  {
   int a, count = 0;
   Set key = labels.keySet();
   Iterator iter = key.iterator();
   Vector list = new Vector();

   while(iter.hasNext())
    list.add(labels.get((String)iter.next()));

   try
    {
     File dataOutFile = new File(name + (rom?".rom":".obj"));
     FileOutputStream o = new FileOutputStream(dataOutFile);

     if (!rom)
      {
       byte [] x = new byte[4];

       // Code table
       Code.writeObj(o);

       // globals
       iter = key.iterator();
       while(iter.hasNext())
        {
         node y;
         String label;

         label = (String)iter.next();
         y = (node)labels.get(label);

         // relative address
         a = Code.address(rom, y.location);
         x[0] = (byte)a;
         x[1] = (byte)(a >> 8);
         x[2] = (byte)(a >> 16);
         x[3] = (byte)(a >> 24);
         o.write(x, 0, 4);

         if (a == -1)
          { // label unresolved
           // write counted label name
           x[0] = (byte)label.length();
           x[1] = (byte)(label.length() / 256);
           o.write(x, 0, 2);
           for(int i = 0; i < label.length(); i++)
            {
             x[0] = (byte)label.charAt(i);
             o.write(x, 0, 1);
            }

           x[0] = x[1] = x[2] = (byte)0;
           a = (2 + label.length()) % 4;
           if (a > 0)
            o.write(x, 0, 4 - a);
          }

         // number of references
         a = y.list.size();
         count += a;
         x[0] = (byte)a;
         x[1] = (byte)(a >> 8);
         x[2] = (byte)(a >> 16);
         x[3] = (byte)(a >> 24);
         o.write(x, 0, 4);
         // references
         for(int i = 0; i < y.list.size(); i++)
          {
           list z;

           z = (list)y.list.elementAt(i);
           a = Code.address(rom, z.address) | (z.size == 6?0x80000000:0);
           x[0] = (byte)a;
           x[1] = (byte)(a >> 8);
           x[2] = (byte)(a >> 16);
           x[3] = (byte)(a >> 24);
           o.write(x, 0, 4);

           if (Math.abs(z.size) < 4)
            error(label + " unresolvable relative offset");
          }
        }

       // mark resident
       x[0] = (byte)(Scanner.resident?-1:0);
       o.write(x, 0, 1);

       crossreference.addAll(list);
       Object n;
       iter = list.iterator();
       while(iter.hasNext())
        if (!crossreference.contains(n = iter.next()))
         crossreference.add(n);
      }
     else
      Code.writeVHDL(o);

     o.close();
    }
   catch(Exception e) {}

   return count;
  }
}

 class list
  {
   public final Token t;
   public final int address;
   public final int size;

   public list(Token t, int address, int size)
    {
     this.t = t;
     this.address = address;
     this.size = size;
    }
  }

 class node
  {
   public Token t;
   public boolean proc;
   public int location;
   public Vector list;
   public int max;

   public node()
    {
     t = null;
     location = Integer.MIN_VALUE;
     proc = false;
     list = new Vector();
     max = 0;
    }
  }
