package forthassembler;

/**
 * <p>Überschrift: Quellcode Analysator</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.lang.Exception;
import java.io.*;
import java.util.*;

public class Scanner
{
 public static final int eofCh = -1; //EOF

 public static final int
        nop = 0,
	store = 1,
	load = 2,
	storec = 3,
	loadc = 4,
	storeh = 5,
	loadh = 6,
	depth = 7,
	drop = 8,
	twodrop = 9,
	nip = 10,
	pick = 11,
	put = 12,
	val = 13,
	dup = 14,
	over = 15,
	rat = 16,
	r1at = 17,
        save = 18,
        halt = 19,
	storesp = 20,
	loadsp = 21,
	add = 22,
	sub = 23,
	inc = 24,
	dec = 25,
	twomul = 26,
	twodiv = 27,
	and = 28,
	cellp = 29,
	halfp = 30,
	invert = 31,
	lshift = 32,
	negate = 33,
	or = 34,
	rshift = 35,
	xor = 36,
	addc = 37,
	subc = 38,
        lshiftc = 39,
        rshiftc = 40,
        zeroltn = 41,
        zerolt = 42,
        zeroeqn = 43,
        zeroeq = 44,
        cmp = 45,
        const10m = 46,
        const15m = 47,
        const5m = 48,
        const14m = 49,
        const9m = 50,
        const13m = 51,
        const8m = 52,
        zeronen = 53,
        zerone = 54,
        zerogtn = 55,
        zerogt = 56,
        const12m = 57,
        const7m = 58,
        const11m = 59,
        const6m = 60,
        exit = 61,
        call = 62,
        trap = 63,
        branch = 64,
        zerobranchn = 65,
        zerobranch = 66,
        carryload = 67,
        carrystore = 68,
        abreak = 69,
        valueh = 70,
        const15 = 71,
        const14 = 72,
        const13 = 73,
        const12 = 74,
        const11 = 75,
        const10 = 76,
        const9 = 77,
        const8 = 78,
        const7 = 79,
        const6 = 80,
        const5 = 81,
        const4 = 82,
        const3 = 83,
        const2 = 84,
        const1 = 85,
        const0 = 86,
        const1m = 87,
        const2m = 88,
        const3m = 89,
        const4m = 90,
        uzerolt = 91,
        uzerogt = 92,
        fpu = 93,
        setpc = 94,
        switchsy = 95,
        getcoreid = 96,
        op64 = 97,
        prod32 = 98,

        none = 256,
        opar = 257,
        cpar = 258,
        ident = 259,
        number = 260,
        imString = 261,
        imChar = 262,
        colon = 263,
        semicolon = 264,
        dotString = 265,
        include = 266,
        ifsy = 267,
        then = 268,
        elsesy = 269,
        endif = 270,
        dosy = 271,
        loop = 272,
        ploop = 273,
        leave = 274,
        begin = 275,
        until = 276,
        whilesy = 277,
        repeat = 278,
        again = 279,
        casesy = 280,
        of = 281,
        endof = 282,
        endcase = 283,
        floating = 284,
        doq = 285,
        backslash = 286,
        btickb = 287,
        bcharb = 288,
        colonlocal = 289,
        label = 290,
        countedString = 291,
        to = 292,
        define = 293,
        create = 294,
        variable = 295,
        constant = 296,
        literal = 297,
        value = 298,
        procedure = 299,
        marker = 300,
        forget = 301,
        module = 302,
        build = 303,
        does = 304,
        abort = 305,
        local = 306,
        locals = 307,
        twovariable = 308,
        aThrow = 309,
        aCatch = 310,
        org = 311,
        twolocal = 312,
        twolocals = 313,
        unicodeString = 314,
        undefine = 315,
        modName = 316,
        purge = 317,
        dotLocal = 318;

 // static variable
 private static Vector stack;
 public static Source source;
 public static boolean resident;
 private static int ch;
 private static Hashtable refT = new Hashtable(307);
 private static Hashtable userMacro;
 private static File file;
 private static BufferedReader reader;
 private static char [] buffer;
 private static int bufferStart, bufferEnd;
 private static String parent = "";

 static
     {
      refT.put("FPU", new Integer(fpu));
      refT.put("64!", new Integer(op64));
      refT.put("32@", new Integer(prod32));
      refT.put("GETCOREID", new Integer(getcoreid));
      refT.put("SETPC", new Integer(setpc));
      refT.put("SWITCH", new Integer(switchsy));
      refT.put("CMP", new Integer(cmp));
      refT.put("NOP", new Integer(nop));
      refT.put("!", new Integer(store));
      refT.put("@", new Integer(load));
      refT.put("C!", new Integer(storec));
      refT.put("C@", new Integer(loadc));
      refT.put("H!", new Integer(storeh));
      refT.put("H@", new Integer(loadh));
      refT.put("DEPTH", new Integer(depth));
      refT.put("DROP", new Integer(drop));
      refT.put("2DROP", new Integer(twodrop));
      refT.put("NIP", new Integer(nip));
      refT.put("PICK", new Integer(pick));
      refT.put("PUT", new Integer(put));
      refT.put("VAL", new Integer(val));
      refT.put("LABEL", new Integer(label));
      refT.put("DUP", new Integer(dup));
      refT.put("OVER", new Integer(over));
      refT.put("R@", new Integer(rat));
      refT.put("R1@", new Integer(r1at));
      refT.put("SAVE", new Integer(save));
      refT.put("HALT", new Integer(halt));
      refT.put("SP@", new Integer(loadsp));
      refT.put("_SP!", new Integer(storesp));
      refT.put("+", new Integer(add));
      refT.put("-", new Integer(sub));
      refT.put("1+", new Integer(inc));
      refT.put("1-", new Integer(dec));
      refT.put("2*", new Integer(twomul));
      refT.put("2/", new Integer(twodiv));
      refT.put("AND", new Integer(and));
      refT.put("CELL+", new Integer(cellp));
      refT.put("HALF+", new Integer(halfp));
      refT.put("INVERT", new Integer(invert));
      refT.put("LSHIFT", new Integer(lshift));
      refT.put("RSHIFT", new Integer(rshift));
      refT.put("NEGATE", new Integer(negate));
      refT.put("OR", new Integer(or));
      refT.put("XOR", new Integer(xor));
      refT.put("+B", new Integer(addc));
      refT.put("-B", new Integer(subc));
      refT.put("LSHIFTC", new Integer(lshiftc));
      refT.put("RSHIFTC", new Integer(rshiftc));
      refT.put("0<!", new Integer(zeroltn));
      refT.put("0<", new Integer(zerolt));
      refT.put("U0<", new Integer(uzerolt));
      refT.put("0>!", new Integer(zerogtn));
      refT.put("0>", new Integer(zerogt));
      refT.put("U0>", new Integer(uzerogt));
      refT.put("0=!", new Integer(zeroeqn));
      refT.put("0=", new Integer(zeroeq));
      refT.put("0<>!", new Integer(zeronen));
      refT.put("0<>", new Integer(zerone));
      refT.put("EXIT", new Integer(exit));
      refT.put("CALL", new Integer(call));
      refT.put("TRAP", new Integer(trap));
      refT.put("BRANCH", new Integer(branch));
      refT.put("0BRANCH!", new Integer(zerobranchn));
      refT.put("0BRANCH", new Integer(zerobranch));
      refT.put("BREAK", new Integer(abreak));
      refT.put("B@", new Integer(carryload));
      refT.put("B!", new Integer(carrystore));
      refT.put(":LOCAL", new Integer(colonlocal));
      refT.put(":", new Integer(colon));
      refT.put(";", new Integer(semicolon));
      refT.put("(", new Integer(opar));
      refT.put(")", new Integer(cpar));
      refT.put("IF", new Integer(ifsy));
      refT.put("THEN", new Integer(then));
      refT.put("ELSE", new Integer(elsesy));
      refT.put("ENDIF", new Integer(endif));
      refT.put("?DO", new Integer(doq));
      refT.put("DO", new Integer(dosy));
      refT.put("LOOP", new Integer(loop));
      refT.put("+LOOP", new Integer(ploop));
      refT.put("LEAVE", new Integer(leave));
      refT.put("BEGIN", new Integer(begin));
      refT.put("UNTIL", new Integer(until));
      refT.put("WHILE", new Integer(whilesy));
      refT.put("REPEAT", new Integer(repeat));
      refT.put("AGAIN", new Integer(again));
      refT.put("CASE", new Integer(casesy));
      refT.put("OF", new Integer(of));
      refT.put("ENDOF", new Integer(endof));
      refT.put("ENDCASE", new Integer(endcase));
      refT.put("\\", new Integer(backslash));
      refT.put("[\']", new Integer(btickb));
      refT.put("\'", new Integer(btickb));
      refT.put("S\"", new Integer(imString));
      refT.put("\"", new Integer(countedString));
      refT.put("C\"", new Integer(imChar));
      refT.put("U\"", new Integer(unicodeString));
      refT.put("[CHAR]", new Integer(bcharb));
      refT.put(".(", new Integer(dotString));
      refT.put("TO", new Integer(to));
      refT.put("DEFINE", new Integer(define));
      refT.put("UNDEFINE", new Integer(undefine));
      refT.put("INCLUDE", new Integer(include));
      refT.put("CREATE", new Integer(create));
      refT.put("VARIABLE", new Integer(variable));
      refT.put("CONSTANT", new Integer(constant));
      refT.put("LITERAL", new Integer(literal));
      refT.put("VALUE", new Integer(value));
      refT.put("PROCEDURE", new Integer(procedure));
      refT.put("MARKER", new Integer(marker));
      refT.put("MODULE", new Integer(module));
      refT.put("2VARIABLE", new Integer(twovariable));
      refT.put("FORGET", new Integer(forget));
      refT.put("BUILD>", new Integer(build));
      refT.put("DOES>", new Integer(does));
      refT.put("ABORT\"", new Integer(abort));
      refT.put("LOCAL", new Integer(local));
      refT.put("LOCALS", new Integer(locals));
      refT.put("2LOCAL", new Integer(twolocal));
      refT.put("2LOCALS", new Integer(twolocals));
      refT.put("THROW", new Integer(aThrow));
      refT.put("CATCH", new Integer(aCatch));
      refT.put("$ORG", new Integer(org));
      refT.put("MODULENAME", new Integer(modName));
      refT.put("PURGE", new Integer(purge));
      refT.put(".L", new Integer(dotLocal));
    }

 private static Hashtable macro = new Hashtable(509);

 static
     {
      macro.put("=", "- 0=");
      macro.put("=!", "CMP 0=");
      macro.put("<", "- 0<");
      macro.put("<!", "CMP 0<");
      macro.put(">", "- 0>");
      macro.put(">!", "CMP 0>");
      macro.put("U<", "- U0<");
      macro.put("U<!", "CMP U0<");
      macro.put("U>", "- U0>");
      macro.put("U>!", "CMP U0>");
      macro.put("<>", "- 0<>");
      macro.put("<>!", "CMP 0<>");
      macro.put("EI", "0ffffffd4H !");
      macro.put("DI", "0ffffffd4H @");
      macro.put("QI", "DUP 0ffffffd8H !");
      macro.put("PI", "0ffffffd8H @");
      macro.put("SETIMASK", "0ffffffd0H !");
      macro.put("GETIMASK", "0ffffffd0H @");
      macro.put("SP!", "NOP _SP!");
      macro.put(",", "HERE DUP CELL+ TO HERE !");
      macro.put(">BODY", "CELL+ CELL+ @");
      macro.put("+!", "A:R@ @ + R> ! ");
      macro.put("C+!", "A:R@ C@ + R> C! ");
      macro.put("H+!", "A:R@ H@ + R> H! ");
      macro.put("2!", "A:R@ ! R> CELL+ !");
      macro.put("2@", "A:R@ CELL+ @ R> @");
      macro.put("2DUP", "OVER OVER");
      macro.put("2OVER", "3 PICK 3 PICK");
      macro.put("2SWAP", ">R A:R1@ NIP A:SWAP A:R1@ NIP R1@ A:NIP R@ R1@ A:2DROP");
      macro.put(">R", "A:R@ DROP");
      macro.put("?DUP", "1 0BRANCH! DUP");
      macro.put("ABS", "0<! IF NEGATE ENDIF");
      //macro.put("ALIGN", "HERE ALIGNED TO HERE");
      macro.put("ALIGNED", "3 + -4 AND");
      //macro.put("ALLOT", "HERE + TO HERE");
      macro.put("C,", "HERE 1 ALLOT C!");
      macro.put("BL", "32");
      macro.put("CELLS", "2* 2*");
      macro.put("CHAR+", "1+");
      macro.put("CHARS", " ");
      macro.put("HALVES", "2*");
      macro.put("COUNT", "HALF+ DUP 1- C@ 8 LSHIFT OVER 1- 1- C@ OR");
      macro.put("DECIMAL", "10 BASE !");
      macro.put("ERASE", "0 FILL");
      macro.put("FILL", "SWAP 0 ?DO A:R@ OVER C! CHAR+ R> LOOP 2DROP");
      macro.put("HEX", "16 BASE !");
      macro.put("I", "R@");
      macro.put("J", "R> R1@ A:R1@ NIP");
      macro.put("MAX", "<! IF SWAP ENDIF DROP");
      macro.put("MIN", ">! IF SWAP ENDIF DROP");
      macro.put("MOVE", "0 ?DO OVER I + C@ OVER I + C! LOOP 2DROP");
      macro.put("R>", "A:>R");
      macro.put("ROLL", "A:R@ PICK A:BEGIN A:0=! WHILE R@ PICK R@ 1+ PUT A:1- REPEAT NIP A:DROP");
      macro.put("ROT", "A:R> A:R1@ NIP R1@ R@ A:2DROP");
      macro.put("S>D", "A:R@ 0< R>");
      macro.put("SWAP", "A:R1@ NIP R>");
      macro.put("UNLOOP", "A:2DROP");
      macro.put("2>R", "A:R1@ A:R@ 2DROP");
      macro.put("2R>", "A:2>R");
      macro.put("2R@", "R1@ R@");
      macro.put("FALSE", "0");
      macro.put("TRUE", "-1");
      macro.put("TUCK", "SWAP OVER");
      macro.put("WITHIN", "<>! >R >! A:R@ A:R1@ 2DROP OVER > INVERT A:R1@ NIP A:> R> A:IF OR ELSE AND ENDIF R> AND");
      macro.put("D+", "A:R1@ NIP + R@ A:R1@ NIP +B R@ A:2DROP");
      macro.put("D-", "A:R1@ NIP - R@ A:R1@ NIP -B R@ A:2DROP");
      macro.put("D0<", "DROP 0<");
      macro.put("D0=", "OR 0=");
      macro.put("D2*", ">R A:2* DUP +B R>");
      macro.put("D2/", ">R 2/ R> 1 RSHIFTC");
      macro.put("D<", "D- D0<");
      macro.put("D>", "D- A:R1@ OR 0<> R> 0< INVERT AND");
      macro.put("D=", ">R A:R1@ NIP XOR A:XOR R> OR 0=");
      macro.put("D>S", "NIP");
      macro.put("DABS", "OVER 0< IF DNEGATE ENDIF");
      macro.put("DMAX", "A:R1@ A:R@ D- OVER 0> IF R1@ R@ D+ ELSE 2DROP R1@ R@ ENDIF A:2DROP");
      macro.put("DMIN", "A:R1@ A:R@ D- OVER 0< IF R1@ R@ D+ ELSE 2DROP R1@ R@ ENDIF A:2DROP");
      macro.put("DNEGATE", "NEGATE >R 0 OVER -B NIP R>");
      macro.put("2ROT", "2>R 2SWAP 2R> 2SWAP");
      macro.put("DU<", "D- DROP DUP -B");
      macro.put("DU>", "D- A:R1@ A:DUP A:-B D0= R> OR 0=");
      macro.put("F!", "!");
      macro.put("F@", "@");
      macro.put("FALIGNED", "ALIGNED");
      macro.put("FDEPTH", "DEPTH");
      macro.put("FDROP", "DROP");
      macro.put("FDUP", "DUP");
      macro.put("FLOAT+", "CELL+");
      macro.put("FLOATS", "CELLS");
      macro.put("F0<", "OVER 2* OR 0<> OVER 0< AND NIP");
      macro.put("F<", "F- F0<");
      macro.put("F0=", "OVER 2* OR NIP 0=");
      macro.put("FOVER", "OVER");
      macro.put("FSWAP", "SWAP");
      macro.put("DF!", "2!");
      macro.put("DF@", "2@");
      macro.put("DFALIGNED", "7 + -8 AND");
      macro.put("DFLOAT+", "CELL+ CELL+");
      macro.put("SFALIGNED", "FALIGNED");
      macro.put("SFLOAT+", "FLOAT+");
      macro.put("DFLOATS", "2* 2* 2*");
      macro.put("SFLOATS", "FLOATS");
      macro.put("FABS", ">R 7fffffffH AND R>");
      macro.put("TIB", "tib @");
      macro.put("#TIB", "#tib @");
      macro.put(">IN", ">in @");
      macro.put("SOURCE", ">IN #TIB A:R@ + 1023 AND TIB + R>");
     }

 /**
  * print error message
  * @param name name of source file
  * @param line line of error
  * @param col column of error
  * @param msg message
  */
 private static void error(String file,int line,int col,String msg)
  {
   Errors.println(file,line,col,msg, true);
  }

 /**
  * open next source file
  */
 private static void newFile()
  {
   try
    {
     if (reader != null)
      {
       reader.close();
      }

     reader = null;

     if (stack.isEmpty())
      return;

     source = (Source)stack.elementAt(0);

     if (source.sourceFile.charAt(0) == '.')
      source.sourceFile = parent + '\\' + source.sourceFile.substring(1);

     stack.remove(0);
     System.out.println(source.sourceFile + " will be read");

     file = new File(source.sourceFile);

     if (!file.exists())
      {
       System.out.println(source.sourceFile + " does not exist\n");
       System.exit(-1);
      }
     if (!file.canRead())
      {
       System.out.println(source.sourceFile + " not readable\n");
       System.exit(-1);
      }
     if (!file.isFile())
      {
       System.out.println(source.sourceFile + " is no file\n");
       System.exit(-1);
      }

     reader = new BufferedReader(new FileReader(file));
     reader.skip(source.pos);

     bufferEnd = -1;
     bufferStart = -1;
    }
   catch(Exception e) {}
  }

 /**
  * initialize scanner
  * @param args command line
  */
 public static void init(String [] args)
  {
   userMacro = new Hashtable(101);
   stack = new Vector();
   buffer = new char[512];

   for(int i = 0; i < args.length; i++)
    stack.add(i, new Source(args[i]));

   source = null;
   reader = null;
   ch = ' ';

   if (args.length > 0)
    parent = new File(args[0]).getParent();
   else
    parent = "";

   newFile();

   resident = false;
  }

 /**
  * retrieve next token t from input stream
  * @return a token
  */
 public static Token next(boolean expand)
  {
   StringBuffer x;
   Token u, t = new Token();
   boolean alternate;
   Integer n;
   String string;

   do
    {
     t.kind = none;
     t.line = source != null?source.line:-1;
     t.col = source != null?source.col:-1;
     t.val = -1;

     x = new StringBuffer();

     // skip leading whitespace
     while (ch <= ' ' && ch >= 0)
      nextCh();

     t.sourceFile = source != null?source.sourceFile:"";

     // extract compact sequence
     while(ch > ' ')
      {
       x.append((char)ch);
       nextCh();
      }

     t.ident = new String(x);

     // check stack modifier
     for(alternate = false; t.ident.startsWith("A:"); t.ident = t.ident.substring(2))
      alternate = !alternate;

     t.alternate = alternate;

     // expand user macro
     if (expand && (string = (String)userMacro.get(t.ident)) != null)
      {
       insert(alternate, string);
       return next(true);
      }
     // keyword
     else if ((n = (Integer)refT.get(t.ident)) != null)
      switch(n.intValue())
       {
        case dotLocal:
         Errors.println(source.sourceFile, source.line, source.col, "localOffset := " + Code.localOffset, false);
         t.kind = none;
         break;
        case opar:
         // skip comment
         extractDelimited(')');
         break;
        case backslash:
         // skip to end of line
         while(ch >= 0 && ch != '\n')
          nextCh();
         break;
        case imString:
        case imChar:
        case countedString:
        case unicodeString:
         // immediate String or Character
         t.string = extractDelimited('\"');
         t.kind = n.intValue();
         break;
        case abort:
         // abort message
         t.string = extractDelimited('\"');
         t.kind = imString;
         insert(false, "TYPE ABORT");
         break;
        case dotString:
         // display message
         t.string = extractDelimited(')');
         t.kind = imString;
         insert(false, " TYPE ");
         break;
        case bcharb:
         // [CHAR] takes first character of following string in source
         u = next(false);
         t.val = u.ident.charAt(0) & 0xff;
         t.kind = number;
         break;
        case include:
         // include a source file
         u = next(false);
         stack.add(0, source);
         stack.add(0, new Source(u.ident));
         newFile();
         break;
        case btickb:
         // retrieve an entry in dictionary
         u = next(false);
         insert(alternate, "\" " + u.ident + " \" FIND");
         break;
        case constant:
         // create a constant
         u = next(false);
         insert(alternate, "1 \" " + u.ident + " \" create");
         resident = true;
         break;
        case create:
         // create an entry in dictionary
         u = next(false);
         insert(alternate, "ALIGN HERE 2 \" " + u.ident + " \" create");
         resident = true;
         break;
        case variable:
         // create a variable
         u = next(false);
         insert(alternate, "CREATE " + u.ident + " 1 CELLS ALLOT");
         resident = true;
         break;
        case twovariable:
         // create a double variable
         u = next(false);
         insert(alternate, "ALIGN HERE 7 \" " + u.ident + " \" create 2 CELLS ALLOT");
         resident = true;
         break;
        case procedure:
         // create a procedure entry
         u = next(false);
         insert(alternate, "3 \" " + u.ident + " \" create");
         resident = true;
         break;
        case literal:
        case value:
         // create a value
         u = next(false);
         insert(alternate, "4 \" " + u.ident + " \" create");
         resident = true;
         break;
        case module:
         // create a module entry
         u = next(false);
         insert(alternate, "MODULETOP 5 \" " + u.ident + " \" create");
         resident = true;
         break;
        case marker:
         // create a marker
         u = next(false);
         insert(alternate, "HERE 6 \" " + u.ident + " \" create");
         resident = true;
         break;
        case forget:
         // forget an entry
         u = next(false);
         insert(alternate, "\" " + u.ident + " \" forget");
         break;
        case define:
         // save user macro
         u = next(false);
         x = new StringBuffer();
         while(ch >= 0 && ch != '\n')
          {
           x.append((char)ch);
           nextCh();
          }
         userMacro.put(u.ident, new String(x));
         break;
        case undefine:
         // remove user macro
         u = next(false);
         userMacro.remove(u.ident);
         break;
        default:
         t.kind = n.intValue();
       }
     else if ((string = (String)macro.get(t.ident)) != null)
     {
      insert(alternate, string);
     }
     else if (t.ident.length() > 0)
      parseInt(t);
    }
   while(ch >= 0 && t.kind == none);

   return t;
  }

 /**
  * convert a string to an integer
  * @param t token
  * @return a token identifier
  */
 private static void parseInt(Token t)
  {
   String string = t.ident.trim().toUpperCase();
   int radix = 10;
   int start = 0;
   int vz, diff, digit, begin;
   int end = string.length();
   long value = 0;
   char x = ' ', sign;
   boolean hasDigit = false;

   t.kind = ident;
   t.d = 0.;

   if (string.equals("."))
    return;

   if (string.charAt(end - 1) == 'H')
    {             // hexadecimal string
     radix = 16;
     end--;
    }

   start -= vz = (string.charAt(start) == '-')?-1:0;
   if (string.charAt(start) != '.' && (string.charAt(start) < '0' || string.charAt(start) > '9'))
    return;

   for(begin = start; begin < end; begin++)
    {
     x = string.charAt(begin);
     digit = (x >= '0' && x <= '9')?x - '0':((x >= 'A' && x <= 'F')?x - 'A' + 10:radix);
     if (digit >= radix)
      break;
     value *= radix;
     value += digit;
     hasDigit |= digit < 10;
    }

   if (radix == 16 && (value >> 32) == 0)
    {
     value = (long) (int) value;
     vz = (value < 0)?-1:0;
     value = Math.abs(value);
    }

   if (begin == start)
    if (x != '.' || begin >= end - 1)
     return;   // not a number

   if (begin >= end)
    {          // integer conversion completed
     if (vz < 0)
      value = -value;

     t.val = value;
     if (hasDigit)
      t.kind = number;

     return;
    }

   if (radix == 16)
    return; // no valid number or identifier

   t.d = (double)value;
   diff = 0;

   if (x == '.')
    for(begin++; begin < end; begin++)
     {
      x = string.charAt(begin);
      digit = (x >= '0' && x <= '9')?x - '0':radix;
      if (digit >= radix)
       break;
      t.d *= radix;
      t.d += digit;
      diff--;
     }

   if (begin++ < end)
    {
     if (x != 'E' && x != 'e')
      return; // an identifier

     x = string.charAt(begin);
     sign = (x == '-' || x == '+')?string.charAt(begin++):'+';

     if (begin >= end)
      return; // an identifier

     for(value = 0; begin < end; begin++)
      {
       x = string.charAt(begin);
       digit = (x >= '0' && x <= '9')?x - '0':radix;
       if (digit == radix)
        return;
       value *= radix;
       value += digit;
      }

     if (sign == '-')
      value = -value;

     diff += value;
    }

   t.d *= Math.pow(10., (double)diff);

   if (vz < 0)
    t.d = -t.d;

   t.kind = floating;
  }


 /**
  * extract a delimited string from input
  * @param delim delimiter
  */
 private static String extractDelimited(char delim)
  {
   StringBuffer x = new StringBuffer();
   int last;
   int actual;

   nextCh();
   last = ch;
   nextCh();
   actual = ch;

   if (!(last == delim && actual <= ' '))
    {
     nextCh();

     while(last != eofCh && !(last <= ' ' && actual == delim && ch <= ' '))
      {
       if (last == '\\')
        if (actual == 'n')
         {
          last = '\n';
          actual = ch;
          nextCh();
         }
        else if (actual == 'r')
         {
          last = '\r';
          actual = ch;
          nextCh();
         }
        else if (actual == 't')
         {
          last = '\t';
          actual = ch;
          nextCh();
         }
        else if (actual == 'f')
         {
          last = '\f';
          actual = ch;
          nextCh();
         }
        else if (actual == 'b')
         {
          last = '\b';
          actual = ch;
          nextCh();
         }
        else if (actual == delim)
         {
          last = ch;
          actual = ch;
          nextCh();
         }

       x.append((char)last);
       last = actual;
       actual = ch;
       nextCh();
      }
    }

   return new String(x);
  }

 /**
  * insert expanded macro into input stream
  */
 public static void insert(boolean alternate, String macro)
  {
   StringBuffer x = new StringBuffer();
   char delim, y;
   int i;

   while(macro.length() > 0)
    {
     for(i = 0; i < macro.length() && (y = macro.charAt(i)) <= ' '; i++)
      x.append(y);

     if (i == macro.length())
      break;

     macro = macro.substring(i);

     if (alternate)
      x.append("A:");

     if (macro.length() >= 2 && macro.charAt(0) == '.' && macro.charAt(1) == '(' && (macro.length() < 3 || macro.charAt(2) <= ' '))
      {
       delim = ')';
       x.append(".( ");
       macro = macro.substring(3);
      }
     else if (macro.length() >= 1 && macro.charAt(0) == '(' && (macro.length() < 2 || macro.charAt(1) <= ' '))
      {
       delim = ')';
       if (alternate)
        x.setLength(x.length() - 2);
       x.append("( ");
       macro = macro.substring(2);
      }
     else if (macro.length() >= 1 && macro.charAt(0) == '\\'&& (macro.length() < 2 || macro.charAt(1) <= ' '))
      {
       delim = ' ';
       if (alternate)
        x.setLength(x.length() - 2);
       macro = "";
      }
     else if (macro.length() >= 2 && macro.charAt(0) == 'S' && macro.charAt(1) == '"' && (macro.length() < 3 || macro.charAt(2) <= ' '))
      {
       delim = '\"';
       x.append("S\" ");
       macro = macro.substring(3);
      }
     else if (macro.length() >= 2 && macro.charAt(0) == 'C' && macro.charAt(1) == '"' && (macro.length() < 3 || macro.charAt(2) <= ' '))
      {
       delim = '\"';
       x.append("C\" ");
       macro = macro.substring(3);
      }
     else if (macro.length() >= 2 && macro.charAt(0) == 'U' && macro.charAt(1) == '"' && (macro.length() < 3 || macro.charAt(2) <= ' '))
      {
       delim = '\"';
       x.append("U\" ");
       macro = macro.substring(3);
      }
     else if (macro.length() >= 1 && macro.charAt(0) == '"' && (macro.length() < 2 || macro.charAt(1) <= ' '))
      {
       delim = '\"';
       x.append("\" ");
       macro = macro.substring(2);
      }
     else
      delim = ' ';

     for(i = 0, y = 0; i < macro.length() && y != delim; i++)
      {
       y = macro.charAt(i);
       x.append(y);
       y = (y < ' ')?' ':y;
      }

     macro = (i < macro.length())?macro.substring(i):"";
    }
   source.unget = new String(x) + ' ' + (source.unget == null?"":source.unget);
  }

 /**
  * read a single character ch from input stream
  */
 private static void nextCh()
  {
   int x = -1;

   try
    {
     if (source != null)
      if (source.unget != null)
       {
        x = source.unget.charAt(0);
        if (source.unget.length() > 1)
         source.unget = source.unget.substring(1);
        else
         source.unget = null;
       }
      else
       {
        // fill buffer, assign new source file
        while(bufferStart >= bufferEnd && reader != null)
         {
          bufferEnd = reader.read(buffer, 0, buffer.length);
          bufferStart = 0;
          if (bufferEnd < 0)
           newFile();
         }
        // get next character
        if (bufferStart < bufferEnd)
         x = buffer[bufferStart++];

        if (source != null)
         {
          if (x == '\n')
           {
            source.line++;
            source.col=0;
           }
          else
           source.col++;
          source.pos++;
         }
       }
    }
   catch (IOException e) {error(source.sourceFile, source.line, source.col, "problems reading from file");}

   ch = x;
  }
}
