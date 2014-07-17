package MyJava;

/**
 * <p>Überschrift: Codeerzeugung</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.io.*;
import java.util.*;

public class CodeRCmodified
 {
   private static String insert = "";
   static final int father = 0,
                    son = 4,
                    outer = 8,
                    table = 12,
                    name = 16,
                    coded = 20,
                    refcount = 24,
                    area = 28;
   private static final int LOADED = 1 << 30,
                            INDEXED = 1 << 29,
                            ISTHIS = 1 << 28;
   private static ClassType theClass = null;
   private static MethodType member = null;
   private static boolean exception = false;
   private static boolean isStatic = false;
   private static String statementException;
   private static Vector scopes = new Vector();
   private static Vector locals = new Vector();
   private static Vector sync = new Vector();
   private static Vector branches;
   private static boolean isFirst = false;
   private static String toAppend;

    /**
     * generate code referencing a class variable
     * @param x variable
     * @return code
     */
    public static String getOffset(VariableType x)
     {
      return (x.offset + CodeRCmodified.area) + " + ";
     }

    /**
     * replace content of variable
     * @param name String
     * @return String
     */
   static String toVariable(String name)
    {
     Scope scope = Find.find(name, Find.theScope, false)[0];
     VariableType b = (VariableType)scope.get(name)[0];

     int dist = 0;

     for(; scope != member.scope; scope = scope.prev)
      dist++;

     String s = "";

     if (b.type.dim > 0 || b.type.type == Keyword.NONESY)
      if (b.offset == 0)
       s = "DUP §base" + dist + " V! ";
      else
       s = "DUP " + (b.offset / 1) + " §base" + dist + " + V! ";

     return s + "TO " + name + " ";
    }

   /**
    * generate code for static variables and methods in scope
    * @param file FileOutputStream code target
    */
   private static void codeScope(FileOutputStream file)
    {
     Iterator iter = Find.theScope.iterator();
     while(iter.hasNext())
      {
       Basic y = (Basic)iter.next();
       if (y instanceof MethodType)
        {
         MethodType z = (MethodType) y;
         if (z.operation != null && z.operation.size() > 0)
          codeMethod(z, file);
        }
      }
     iter = Find.theScope.iterator();
     while(iter.hasNext())
      {
       Basic y = (Basic)iter.next();
       if (y instanceof VariableType && (y.modify & Keyword.STATICSY.value) != 0)
        {
         VariableType v = (VariableType) y;

         // allocate static variable
         String s = "VARIABLE ";
         if (v.type.dim == 0 && (v.type.type == Keyword.LONGSY || v.type.type == Keyword.FLOATSY || v.type.type == Keyword.DOUBLESY))
          s = '2' + s;
         s += Find.buildQualifiedName(Find.theScope, v, false, "");
         if (v.type.dim > 0 || v.type.type == Keyword.NONESY)
          s += " HERE 4 - SALLOCATE\n";
         append(s, file, true);
        }
      }
    }

   /**
    * generate code for a distinct class
    * @param x ClassType
    * @param file FileOutputStream code target
    * @param start FileOutputStream target for initial static code
    * @param module if true, insert module name
    */
   public static void codeClass(ClassType x, FileOutputStream file, FileOutputStream start, boolean module)
    {
     theClass = x;
     Find.theScope = x.scope;

     // anonymous class derived either from superclass or an interface
     if (x.implement.length == 1 && x.extend == x.implement[0])
      if ((x.extend.modify & Keyword.INTERFACESY.value) != 0)
       x.extend = new ClassType();
      else
       x.implement = new ClassType[0];

     // module name
     if (module)
      append("MODULENAME " + Find.theScope.buildPath(""), file, false);
     // write comment
     append(x.comment, file, false);
     x.comment = null;
     // code methods and variables
     codeScope(file);

     // append method table
     String codem = "";
     int count = 0;
     Iterator iter = x.scope.iterator();
     while(iter.hasNext())
      {
       Basic b = (Basic)iter.next();
       count++;
       if (b instanceof MethodType && (b.modify & (Keyword.STATICSY.value | Keyword.ABSTRACTSY.value)) == 0)
        {
      	 String z = ((MethodType)b).scope.buildPath(b.name.string + "§" + b.version);
         codem += b.version + " R@ ! A:CELL+ " + z.substring(z.indexOf('.') + 1) + " VAL R@ ! A:CELL+\n";
        }
       else
        count--;
      }

     append("\nA:HERE VARIABLE " + Find.buildQualifiedName(x.scope.prev, x, false, "").trim() + ".table " + count + " DUP 2* CELLS ALLOT R@ ! A:CELL+\n" + codem + "A:DROP\n",
            file, false);

     // static initial code
     //x.statics = x.read();
     isStatic = true;
     code(x.statics, start);
     isStatic = false;
     x.statics = null;
     Find.theScope = null;

     theClass = null;
    }

   /**
    * generate code for a method
    * @param x MethodType
    * @param file FileOutputStream code target
    */
   private static void codeMethod(MethodType x, FileOutputStream file)
    {
     member = x;
     append(x.comment, file, false);
     x.comment = null;
     // build function header
     String z = x.scope.buildPath(x.name.string + "§" + x.version);
     z = z.substring(z.indexOf('.') + 1);
     append(((x.modify & Keyword.STATICSY.value) == 0?"\n:LOCAL " + z:("\n: " + z)) + " ", file, true);

     if (isFirst = (x.scope.refcount > 0))
      append(x.scope.refcount + " VALLOCATE LOCAL §base" + scopes.size(), file, true);

     // allocate parameter
     if (x.parameter != null)
      {
       String code = "";
       int end = ((member.modify &
		           (Keyword.STATICSY.value | Keyword.CONSTRUCTORSY.value)) == 0 &&
		          member.parameter.length > 0 && member.parameter[0].name.string.compareTo("§this") == 0)? 1: 0;

       if (end != 0)
        {
         VariableType b = (VariableType)x.scope.get(x.parameter[0].name.string)[0];
         code += "DUP " + (b.offset / 1) + " §base" + scopes.size() + " + V! ";
         code += "LOCAL " + x.parameter[0].name.string + '\n';
        }
   
       for (int i = x.parameter.length; i-- > end; )
        {
         if (x.parameter[i].type.dim > 0 || x.parameter[i].type.type == Keyword.NONESY)
          {
           VariableType b = (VariableType)x.scope.get(x.parameter[i].name.string)[0];
           code += "DUP " + (b.offset / 1) + " §base" + scopes.size() + " + V! ";
          }
         else if (x.parameter[i].type.type == Keyword.LONGSY || x.parameter[i].type.type == Keyword.DOUBLESY || x.parameter[i].type.type == Keyword.FLOATSY)
          code += "2";

         code += "LOCAL " + x.parameter[i].name.string + '\n';
        }

       append(code, file, true);
      }

     // build code
     //x.operation = x.read();
     code(x.operation, file);
     x.operation = null;

     // terminate function
     append(";", file, true);

     member = null;

     // free obsolete data
     x.scope.label = null;
    }

   /**
    * create code for automatic variables of current scope
    * @return String code sequence
    */
   private static String createLocals()
    {
     String one = "", two = "", trailer = "";
     int small = 0, large = 0;

     if (!isFirst && Find.theScope.refcount > 0)
      trailer = Find.theScope.refcount + " VALLOCATE LOCAL §base" + (scopes.size() - 1) + "\n";

     isFirst = false;
     Iterator e = Find.theScope.iterator();
     while (e.hasNext())
      {
       boolean decl = false;
       Basic x = (Basic)e.next();

       // parameters are already declared
       if (member != null)
        for (int i = 0; member.parameter != null && i < member.parameter.length && !decl; i++)
         decl = member.parameter[i].name.string.compareTo(x.name.string) == 0;

       if (!decl && x instanceof VariableType && (x.modify & Keyword.STATICSY.value) == 0)
        {
         VariableType y = (VariableType) x;

         if (y.type.dim == 0 && (y.type.type == Keyword.LONGSY || y.type.type == Keyword.DOUBLESY || y.type.type == Keyword.FLOATSY))
          {
           two += y.name.string + ' ';
           large++;
          }
         else
          {
           one += y.name.string + ' ';
           small++;
          }
        }
      }

     if (small > 0)
      {
       String help = "0 ";
       while (--small > 0)
        help += "DUP ";

       one = help + "\nLOCALS " + one + "|\n";
      }
     if (two.length() > 0)
      {
       String help = "0 DUP ";
       while (--large > 0)
        help += "2DUP ";

       two = help + "\n2LOCALS " + two + "|\n";
      }

     return trailer + one + two;
    }

   /**
    * generate code for deletion of automatic variables of current scope
    * @return String code sequence
    */
   private static String purgeLocals()
    {
     String header = "";
     int count = Find.theScope.refcount > 0?1:0;

     Iterator e = Find.theScope.iterator();
     while(e.hasNext())
      {
       Basic x = (Basic) e.next();

       if (x instanceof VariableType && (x.modify & Keyword.STATICSY.value) == 0)
        {
         VariableType y = (VariableType) x;

         if (member == null || member.name.string.compareTo("~destructor") != 0 &&
             (member.name.string.compareTo("kill") != 0 || theClass.name.string.compareTo("JavaArray") != 0))
          if (y.name.string.compareTo("§this") != 0 &&  y.name.string.compareTo("§outer") != 0)
           header += freeVariable(y.type, y.name) + '\n';

         count++;
        }
      }

     if (Find.theScope.refcount > 0)
      header += "§base" + (scopes.size() - 1) + " SETVTOP\n";

     if (count > 0)
      header += "PURGE " + count + "\n";

     return header;
    }

   /**
    * generate code for deletion of a variable (for memorymanagement only)
    * @param t Type
    * @param name Token
    * @return String code sequence
    */
   private static String freeVariable(Type t, Token name)
    {
     String x = "", header = "";
     MethodType [] m = null;

     if (name != null)
      header = name.string + " ";

     if (t.type == Keyword.NONESY && t.dim == 0)
      {
       m = Find.getMethod("~destructor", null, null, findClass(t.ident.string, t.version));
       if (m.length > 0 && (m[0].modify & Keyword.STATICSY.value) != 0)
        x = header + "DUP HANDLEVALID IF " + Find.buildQualifiedName(m[0].scope, m[0], false, "") + "DUP ENDIF DROP ";
      }

     if (x.length() == 0)
      if (t.type == Keyword.NONESY || t.dim > 0)
       {
        m = Find.getMethod("kill", null, null, findClass("JavaArray", 0));
        x = header + "TRUE " + Find.buildQualifiedName(m[0].scope, m[0], false, "");
       }

     return x;
    }

    /**
     * increment reference (for memorymanagement only)
     * @param trailer String
     * @return String
     */
    private static String incrementReference(String trailer)
     {
      return (member == null || member.name.string.compareTo("~destructor") != 0)?trailer + " INCREFERENCE ":"";
     }

   /**
    * append a beautified version of x to generated code
    * @param x String code sequence
    * @param file FileOutputStream code target
    * @param check boolean beautify
    */
   private static void append(String x, FileOutputStream file, boolean check)
    {
     if (x == null)
      return;

     String part, a, b;
     boolean skip;

     try
      {
       while(x.length() > 0)
        {
         int k, n, j, i = x.indexOf('\n');
         if (i < 0)
          {
           part = x.trim();
           x = "";
          }
         else
          {
           part = x.substring(0, i).trim();
           x = x.substring(i + 1);
          }

         skip = false;
         do
          {
           k = j = i = -1;
           if (check)
            {
             String y = ' ' + part + ' ';
             String end, start;
             // locate compound token
             if ((k = y.indexOf(a = "\" ")) > 0 && y.charAt(k - 1) != ' ' && k > 1 && y.charAt(k - 2) != ' ')
              k = -1;
             if ((n = y.indexOf(b = "( ")) > 0 && y.charAt(n - 1) != ' ' && y.charAt(n - 1) != '.')
              n = -1;
             if (n >= 0 && n < k || k < 0)
              {
               k = n;
               a = b;
              }
             if ((n = y.indexOf(b = " CHAR ")) >= 0 && n < k || k < 0)
              {
               k = n;
               a = b;
              }
             if ((n = y.indexOf(b = " \\ ")) >= 0 && n < k || k < 0)
              {
               k = n;
               a = b;
              }
             // minor insert
             i = y.indexOf(end = " ELSE ");
             if ((n = y.indexOf(b = " ENDIF ")) >= 0 && (i < 0 || n < i))
              {
               i = n;
               end = b;
              }
             if ((n = y.indexOf(b = " THEN ")) >= 0 && (i < 0 || n < i))
              {
               i = n;
               end = b;
              }
             if ((n = y.indexOf(b = " REPEAT ")) >= 0 && (i < 0 || n < i))
              {
               i = n;
               end = b;
              }
             if ((n = y.indexOf(b = " UNTIL ")) >= 0 && (i < 0 || n < i))
              {
               i = n;
               end = b;
              }
             if ((n = y.indexOf(b = " AGAIN ")) >= 0 && (i < 0 || n < i))
              {
               i = n;
               end = b;
              }
             if ((n = y.indexOf(b = " ; ")) >= 0 && (i < 0 || n < i))
              {
               i = n;
               end = b;
              }
             if ((n = y.indexOf(b = " LOOP ")) >= 0 && (i < 0 || n < i))
              {
               i = n;
               end = b;
              }
             if ((n = y.indexOf(b = " LOOP+ ")) >= 0 && (i < 0 || n < i))
              {
               i = n;
               end = b;
              }
             if ((n = y.indexOf(b = " WHILE ")) >= 0 && (i < 0 || n < i))
              {
               i = n;
               end = b;
              }

             // major insert
             j = y.indexOf(start = " DO ");
             if ((n = y.indexOf(b = " ?DO ")) >= 0 && (j < 0 || n < j))
              {
               j = n;
               start = b;
              }
             if ((n = y.indexOf(b = " IF ")) >= 0 && (j < 0 || n < j))
              {
               j = n;
               start = b;
              }
             if ((n = y.indexOf(b = " BEGIN ")) >= 0 && (j < 0 || n < j))
              {
               j = n;
               start = b;
              }
             if ((n = y.indexOf(b = " :LOCAL ")) >= 0 && (j < 0 || n < j))
              {
               j = n;
               start = b;
              }
             if ((n = y.indexOf(b = " : ")) >= 0 && (j < 0 || n < j))
              {
               j = n;
               start = b;
              }


             if (k >= 0 && (i < 0 || k < i) && (j < 0 || k < j))
              {
               boolean old = skip;
               skip = true;
               b = " ";
               if (a.compareTo(" CHAR ") == 0)
                for(k += a.length(); y.charAt(k) != ' ' && k < y.length(); k++);
               else if (a.compareTo("\" ") == 0)
                k += y.substring(k + a.length()).indexOf(" \" ") + a.length() + 3;
               else if (a.compareTo("S\" ") == 0)
                k += y.substring(k + a.length()).indexOf(" \" ") + a.length() + 3;
               else if (a.compareTo("U\" ") == 0)
                k += y.substring(k + a.length()).indexOf(" \" ") + a.length() + 3;
               else if (a.compareTo("( ") == 0)
                k += y.substring(k + a.length()).indexOf(" ) ") + a.length() + 2;
               else if (a.compareTo(".( ") == 0)
                k += y.substring(k + a.length()).indexOf(" ) ") + a.length() + 2;
               else
                {
                 skip = false;
                 b = "\r\n";
                 k = y.length();
                }

               file.write(((old?"":insert) + y.substring(0, k).trim() + b).getBytes());
               part = y.substring(k).trim();
              }
             else if (i >= 0 && (i < j || j < 0))
              {
               if (i > 0)
                file.write(((skip?"":insert) + y.substring(0, i).trim() + "\r\n").getBytes());

               insert = (insert.length() > 3)?insert.substring(0, insert.length() - 3):"";

               file.write((insert + end.trim() + "\r\n").getBytes());

               part = y.substring(i + end.length()).trim();

               if (end.compareTo(" WHILE ") == 0 || end.compareTo(" ELSE ") == 0)
                insert += "   ";

               skip = false;
              }
             else if (j >= 0)
              { // major insert
               if (j > 0)
                file.write(((skip?"":insert) + y.substring(0, j).trim() + "\r\n").getBytes());

               if (start.charAt(1) == ':')
                {
                 for(i = j + start.length(); y.charAt(i) != ' '; i++);
                 file.write((insert + y.substring(j, i).trim() + "\r\n").getBytes());
                 part = y.substring(i).trim();
                }
               else
                {
                 file.write((insert + start.trim() + "\r\n").getBytes());
                 part = y.substring(j + start.length()).trim();
                }

               insert += "   ";

               skip = false;
              }
            }
           if (k < 0 && i < 0 && j < 0)
            {
             file.write(((skip?"":insert) + part.trim() + "\r\n").getBytes());
             part = "";
             skip = false;
            }
          }
         while(part.length() > 0);
        }
      }
     catch (IOException ex) { ex.printStackTrace(); }
    }

   /**
    * find class name in crossreference list
    * @param name String name of class
    * @param version int hashcode of class
    * @return ClassType[] list of found classes
    */
   private static ClassType [] findClass(String name, int version)
    {
     return Find.findClass(name, version, Find.theScope);
    }

   /**
    * generate code from operations compiled
    * @param operation Vector
    * @param file FileOutputStream target file
    */
   private static void code(Vector operation, FileOutputStream file)
    {
     Operation op = null, next = null;
     for(int n = 0; n < operation.size() || op != null; op = next)
      {
       next = (n < operation.size())?(Operation)operation.get(n++):null;

       if (op == null || op.operator == Keyword.CLEARSY)
        continue;

       append("\n\\ new statement\n", file, true);

       exception = false;
       statementException = "std" + Operation.newLabel();
       branches = new Vector();
       toAppend = null;
       try
        {
         traverse(op, false, false, next != null && next.operator == Keyword.CLEARSY, null, false, false);
        }
       catch(Exception e) { Find.error(e.getMessage()); }
       // entry for exception
       if (exception)
        op.code += "FALSE DUP IF " + statementException + " LABEL TRUE ENDIF ";

       // remove temporary objects
       String loc = "", st = "";
       int count;
       for (int i = count = locals.size(); i-- > 0; )
        {
         Token t = new Token();
         t.kind = Keyword.IDENTSY;
         t.string = i + "§";
         Type h = (Type)locals.get(i);
         if (h.dim > 0 || h.type == Keyword.NONESY)
          {
           op.code += freeVariable(h, t) + '\n';
           loc += "0 ";
          }
         else if (h.type == Keyword.LONGSY || h.type == Keyword.DOUBLESY)
          loc += "0 DUP";
         else
          loc += "0 ";
         st += i + "§ ";
         locals.remove(i);
        }
       // create temporary locals
       if (count > 0)
        append("\n" + loc + "\nLOCALS " + st + "|\n ", file, true);
       // write code
       write(op, file);
       // purge temporary locals
       if (count > 0)
        append("\nPURGE " + count + "\n ", file, true);
       // append trailer
       if (toAppend != null)
        append(toAppend, file, true);

       if (exception)
        { // leave scopes
         int operand = 0;
         Scope t;

         for(t = Find.theScope; t != null && t.block != Scope.MAIN && t.block != Scope.TRY; t = t.prev)
          operand++;

         operand = (operand << 16) | operand;

         if (t != null && t.block == Scope.TRY)
          {
           st = "IF " + operand + " §break" + Find.theScope.label.get(0) + " BRANCH ENDIF\n";

           for(t = Find.theScope; t.block != Scope.TRY; t.breaks = true, t = t.prev);

           t.breaks = true;
          }
         else if (t != null && member != null && member.throwing.length > 0)
          {
           st = "IF §exception TO §return\n" + operand + " §break" + Find.theScope.label.get(0) + " BRANCH ENDIF\n";

           for(t = Find.theScope; t.block != Scope.MAIN && t.block != Scope.TRY; t.breaks = true, t = t.prev);

           t.breaks = true;
          }
         else
          { // default handler
           MethodType [] m = Find.getMethod("handler", null, null, findClass("JavaArray", 0));
           st = "IF " + Find.buildQualifiedName(m[0].scope, m[0], false, "") + " ENDIF ";
          }

         append(st, file, true);
        }
      }
    }

   /**
    * combine code sequences of an operation to a generated code line
    * @param root Operation
    * @param file FileOutputStream
    */
   private static void write(Operation root, FileOutputStream file)
    {
     if (root.left != null)
      write(root.left, file);
     if (root.right != null)
      write(root.right, file);

     if (root.code.length() > 0)
      append(root.code, file, root.operator != Keyword.COMMENTSY);
    }

   /**
    * concate code of tree
    * @param root
    * @return concated code
    */
   private static String concatCode(Operation root)
    {
	 String s = "";
	 
	 if (root != null)
	  s = concatCode(root.left) + concatCode(root.right) + root.code;
	 
	 return s;
    }

   /**
    * traverse the operator tree of an operation
    * @param root Operation operator tree
    * @param lhs boolean true, if left son is left side of an assignment
    * @param statement boolean true, if no data should remain on stack after processing the tree
    * @param preorder initial operations
    * @param skipleft skip left son
    * @return bit 31 a variable the very first time referenced
    *         bit 30 a variable read out
    */
   private static int traverse(Operation root, boolean lhs, boolean rhs, boolean statement, Operation preorder,
                               boolean skipleft, boolean incrThis) throws Exception
    {
     String label = null;
     boolean added = false;
     int modify = 0;
     int rightside = 0;

     if (root == null)
      return modify;

     if (root.operator == Keyword.LEAFSY)
      {
       // primary, selector
       modify = primarySelector(root.left, lhs, rhs, preorder);

       // prefix, postfix
       if (root.right != null)
        {
         root.left.type = fixes(root.left, root.right, (lhs || statement));
         modify &= ~INDEXED;
        }

       root.type = root.left.type;
       root.name = root.left.name;

       if (!(lhs || statement) && !root.left.loaded)
        {
         root.code += insertLoad(root.type, root.left.name, modify & INDEXED);
         modify &= ~INDEXED;
        }

       root.loaded = !(lhs || statement) || root.left.loaded;
      }
     else if (root.operator == Keyword.LOGICALORSY)
      {
       if (branches.size() == 0 || !(label = (String)branches.lastElement()).startsWith("or_"))
        {
         label = "or_" + Operation.newLabel();
         added = true;
         branches.add(label);
        }

       traverse(root.left, false, false, false, preorder, false, false);
       root.left.code += " 0=! " + label + " 0BRANCH DROP ";
       if (root.left.type.type != Keyword.BOOLEANSY || root.left.type.dim != 0)
        Find.error("left expression must be boolean");
       traverse(root.right, false, false, false, preorder, false, false);
       if (root.right.type.type != Keyword.BOOLEANSY || root.right.type.dim != 0)
        Find.error("right expression must be boolean");

       if (added)
        {
         root.code += label + " LABEL ";
         branches.remove(label);
        }

       root.type = root.left.type;
       root.loaded = true;
      }
     else if (root.operator == Keyword.LOGICALANDSY)
      {
       if (branches.size() == 0 || !(label = (String)branches.lastElement()).startsWith("and_"))
        {
         label = "and_" + Operation.newLabel();
         added = true;
         branches.add(label);
        }

       traverse(root.left, false, false, false, preorder, false, false);
       root.left.code += label + " 0BRANCH! DROP ";
       if (root.left.type.type != Keyword.BOOLEANSY || root.left.type.dim != 0)
        Find.error("left expression must be boolean");
       traverse(root.right, false, false, false, preorder, false, false);
       if (root.right.type.type != Keyword.BOOLEANSY || root.right.type.dim != 0)
        Find.error("right expression must be boolean");

       if (added)
        {
         root.code += label + " LABEL ";
         branches.remove(label);
        }

       root.type = root.left.type;
       root.loaded = true;
      }
     else if (Parser.assign.contains(root.operator))
      {
       boolean compound = false;
       Operation myPreorder = new Operation();

       modify = traverse(root.left, true, false, false, myPreorder, false, false);
       if ((modify & Keyword.FINALSY.value) != 0 && root.operator != Keyword.FINALASSIGNSY)
        if (member != null && (member.modify & Keyword.CONSTRUCTORSY.value) == 0 && modify >= 0)
         Find.error("finals can not be modified"+((member.modify & Keyword.CONSTRUCTORSY.value) == 0));

       modify &= (Integer.MIN_VALUE | LOADED | INDEXED);

       if (root.operator != Keyword.ASSIGNSY && root.operator != Keyword.FINALASSIGNSY)
        { // decompose arithmetic operation and assign
         if (modify < 0)
          Find.error("left side has undefined value");
         Operation op = new Operation();
         op.operator = Keyword.get(root.operator.string.substring(0, root.operator.string.length() - 1));
         op.left = new Operation(root.left);
         op.left.loaded = true;
         op.left.code += insertLoad(root.left.type, root.left.name, modify & INDEXED);
         op.right = root.right;
         root.right = op;
         root.operator = Keyword.ASSIGNSY;
         compound = true;
        }

       modify &= ~INDEXED;

       if (root.right.operator == Keyword.LBRACESY)
        arrayInitializer(root.right, root.left.type, true);
       else
        rightside = traverse(root.right, false, true, false, null, compound, false);

       root.right.code += castElem(root.left.type, root.right.type, root.right);

       // right side datum longer needed?
       if (!statement)
        if ((root.left.type.type == Keyword.LONGSY || root.left.type.type == Keyword.DOUBLESY || root.left.type.type == Keyword.FLOATSY) &&
             root.left.type.dim == 0)
         root.right.code += "OVER OVER ";
        else
         root.right.code += "DUP ";

       String leftcode = concatCode(root.left).trim();
       String rightcode = concatCode(root.right).trim();

       // increment reference count of object to assign, decrement reference of currently assigned object
       if (root.left.type.type == Keyword.NONESY || root.left.type.dim > 0)
        {
         if ((rightside & LOADED) != 0)
          root.right.code += incrementReference("DUP");

         //if (modify >= 0)
          root.code += (!isStatic && root.left.name == null?"DUP ":"") +
                       (!isStatic && (member == null || member.name.string.compareTo("~destructor") != 0)?
                        insertLoad(root.left.type, root.left.name, 0) + freeVariable(root.left.type, null):"");
        }

       if (root.left.name != null)
        {
    	 String w;
         root.code += w = toVariable(root.left.name.string);
         String [] ss = w.split(" ");
         leftcode += ss[ss.length - 1];
        }
       else
        {
         String w = null;
         if (root.left.type.dim > 0 || root.left.type.type == Keyword.NONESY)
          w = ((root.left.type.modify & Keyword.STATICSY.value) == 0? "": "") + " ! ";
         else if (root.left.type.type == Keyword.BYTESY)
          w = "C! ";
         else if (root.left.type.type == Keyword.CHARSY || root.left.type.type == Keyword.SHORTSY)
          w = "H! ";
         else if (root.left.type.type == Keyword.LONGSY || root.left.type.type == Keyword.DOUBLESY || root.left.type.type == Keyword.FLOATSY)
          w = "2! ";
         if (w == null)
          w = "! ";

         root.code += w;
         
         if (rightcode.endsWith("2@"))
          rightcode = rightcode.substring(0, rightcode.length() - 2).trim();
         else if (rightcode.endsWith("@"))
          rightcode = rightcode.substring(0, rightcode.length() - 1).trim();
         else
          rightcode = "";
        }

       root.type = root.left.type;
       root.loaded = !statement;
       modify |= LOADED;

       // transform to postfix
       Operation op = new Operation();
       op.left = root.right;
       op.right = root.left;
       
       if (leftcode.compareTo(rightcode) == 0)
        {
    	 // remove self-assignment
    	 root.code = "";
         root.right = null;
         root.left = null;
        }
       else
        {
         root.right = op;
         root.left = myPreorder;
        }
      }
     else if (root.operator == Keyword.ANDSY)
      {
       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       if ( (root.left.type.type != Keyword.INTSY &&
             root.left.type.type != Keyword.SHORTSY &&
             root.left.type.type != Keyword.CHARSY &&
             root.left.type.type != Keyword.BYTESY &&
             root.left.type.type != Keyword.BOOLEANSY &&
             root.left.type.type != Keyword.LONGSY) ||
           root.left.type.dim != 0)
        Find.error("left operand must be (long) integer or boolean");
       if ( (root.right.type.type != Keyword.INTSY &&
             root.right.type.type != Keyword.SHORTSY &&
             root.right.type.type != Keyword.CHARSY &&
             root.right.type.type != Keyword.BYTESY &&
             root.right.type.type != Keyword.BOOLEANSY &&
             root.right.type.type != Keyword.LONGSY) ||
           root.right.type.dim != 0)
        Find.error("right operand must be (long) integer or boolean");
       castMax(root.left, root.right);
       root.type = root.left.type;
       root.loaded = true;
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (root.type.type == Keyword.BOOLEANSY)
        root.code = "AND ";
       else if (left != null && right != null)
        {
         String s;
         long result = (long)Find.getLong(left) & (long)Find.getLong(right);
         if (root.type.type == Keyword.LONGSY && Math.abs((long)result) <= Integer.MAX_VALUE)
          s = result + " S>D ";
         else
          s = result + " ";
         Find.toLiteral(root, s);
        }
       else if (root.type.type == Keyword.LONGSY)
        root.code = "A:R> A:R1@ NIP AND A:AND R> ";
       else
        root.code = "AND ";
      }
     else if (root.operator == Keyword.ORSY)
      {
       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       if ( (root.left.type.type != Keyword.INTSY &&
             root.left.type.type != Keyword.SHORTSY &&
             root.left.type.type != Keyword.CHARSY &&
             root.left.type.type != Keyword.BYTESY &&
             root.left.type.type != Keyword.BOOLEANSY &&
             root.left.type.type != Keyword.LONGSY) ||
           root.left.type.dim != 0)
        Find.error("left operand must be (long) integer or boolean");
       if (root.right.type.type != Keyword.INTSY &&
           root.right.type.type != Keyword.SHORTSY &&
           root.right.type.type != Keyword.CHARSY &&
           root.right.type.type != Keyword.BYTESY &&
           root.right.type.type != Keyword.BOOLEANSY &&
           root.right.type.type != Keyword.LONGSY ||
           root.right.type.dim != 0)
        Find.error("right operand must be (long) integer or boolean");
       castMax(root.left, root.right);
       root.type = root.left.type;
       root.loaded = true;
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (root.type.type == Keyword.BOOLEANSY)
        root.code = "OR ";
       else if (left != null && right != null)
        {
         String s;
         long result = (long)Find.getLong(left) | (long)Find.getLong(right);
         if (root.type.type == Keyword.LONGSY && Math.abs((long)result) <= Integer.MAX_VALUE)
          s = result + " S>D ";
         else
          s = result + " ";
         Find.toLiteral(root, s);
        }
       else if (root.type.type == Keyword.LONGSY)
        root.code = "A:R> A:R1@ NIP OR A:OR R> ";
       else
        root.code = "OR ";
      }
     else if (root.operator == Keyword.XORSY)
      {
       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       if (root.left.type.type != Keyword.INTSY &&
           root.left.type.type != Keyword.SHORTSY &&
           root.left.type.type != Keyword.CHARSY &&
           root.left.type.type != Keyword.BYTESY &&
           root.left.type.type != Keyword.BOOLEANSY &&
           root.left.type.type != Keyword.LONGSY ||
           root.left.type.dim != 0)
        Find.error("left operand must be (long) integer or boolean");
       if (root.right.type.type != Keyword.INTSY &&
           root.right.type.type != Keyword.SHORTSY &&
           root.right.type.type != Keyword.CHARSY &&
           root.right.type.type != Keyword.BYTESY &&
           root.right.type.type != Keyword.BOOLEANSY &&
           root.right.type.type != Keyword.LONGSY ||
           root.right.type.dim != 0)
        Find.error("right operand must be (long) integer or boolean");
       castMax(root.left, root.right);
       root.type = root.left.type;
       root.loaded = true;
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (root.type.type == Keyword.BOOLEANSY)
        root.code = "XOR ";
       else if (left != null && right != null)
        {
         String s;
         long result = (long)Find.getLong(left) ^ (long)Find.getLong(right);
         if (root.type.type == Keyword.LONGSY && Math.abs((long)result) <= Integer.MAX_VALUE)
          s = result + " S>D ";
         else
          s = result + " ";
         Find.toLiteral(root, s);
        }
       else if (root.type.type == Keyword.LONGSY)
        root.code = "A:R> A:R1@ NIP XOR A:XOR R> ";
       else
        root.code = "XOR ";
      }
     else if (root.operator == Keyword.INSTANCEOFSY)
      {
       traverse(root.left, false, false, false, preorder, false, false);
       if (root.left.type.type != Keyword.NONESY && root.left.type.dim == 0)
        Find.error("left operand of instanceof must be an object");
       if (root.right.type.type != Keyword.NONESY && root.right.type.dim == 0)
        Find.error("right operand of instanceof must be a class");
       root.type = new Type();
       root.type.type = Keyword.BOOLEANSY;
       root.loaded = true;

       ClassType [] s = findClass("JavaArray", 0);
       Basic [] b = s[0].scope.get("code");

       if (root.right.type.dim == 0)
        root.code = "\" " + root.right.type.ident.string.substring(root.right.type.ident.string.lastIndexOf('.') + 1) + " \" INSTANCEOF ";
       else
        root.code = " \" JavaArray \" CASTTO DUP IF DROP CELL+ @ " + getOffset((VariableType)b[0]) + " @ " + Find.crc(root.right.type) + " = ENDIF ";
      }
     else if (root.operator == Keyword.EQUALSY)
      {
       traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       castMax(root.left, root.right);
       root.type = new Type();
       root.type.type = Keyword.BOOLEANSY;
       root.loaded = true;
       Operation literal = Find.isLiteral(root.right);
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         if ((root.type.type == Keyword.FLOATSY || root.type.type == Keyword.DOUBLESY) && root.type.dim == 0)
          Find.toLiteral(root, Find.getValue(left) == Find.getValue(right)?"TRUE ":"FALSE ");
         else
          Find.toLiteral(root, Find.getLong(left) == Find.getLong(right)?"TRUE ":"FALSE ");
        }
       else if (root.left.type.dim != 0 || root.left.type.type == Keyword.INTSY || root.left.type.type == Keyword.BYTESY ||
           root.left.type.type == Keyword.SHORTSY || root.left.type.type == Keyword.CHARSY ||
           root.left.type.type == Keyword.BOOLEANSY || root.left.type.type == Keyword.NONESY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "0= ";
         }
        else
         root.code = "= ";
       else if (root.left.type.type == Keyword.LONGSY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "D0= ";
         }
        else
         root.code = "D= ";
      else if (literal != null && Find.getValue(literal) == 0.)
       {
        root.right = null;
        root.code = "F0= ";
       }
      else
       root.code = "F- F0= ";
      }
     else if (root.operator == Keyword.NOTEQUALSY)
      {
       traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       castMax(root.left, root.right);
       root.type = new Type();
       root.type.type = Keyword.BOOLEANSY;
       root.loaded = true;
       Operation literal = Find.isLiteral(root.right);
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         if ((root.type.type == Keyword.FLOATSY || root.type.type == Keyword.DOUBLESY) && root.type.dim == 0)
          Find.toLiteral(root, Find.getValue(left) != Find.getValue(right)?"TRUE ":"FALSE ");
         else
          Find.toLiteral(root, Find.getLong(left) != Find.getLong(right)?"TRUE ":"FALSE ");
        }
       else if (root.left.type.dim != 0 || root.left.type.type == Keyword.INTSY ||
           root.left.type.type == Keyword.BYTESY ||
           root.left.type.type == Keyword.SHORTSY ||
           root.left.type.type == Keyword.CHARSY ||
           root.left.type.type == Keyword.BOOLEANSY ||
           root.left.type.type == Keyword.NONESY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "0<> ";
         }
        else
         root.code = "<> ";
       else if (root.left.type.type == Keyword.LONGSY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "D0= 0= ";
         }
        else
         root.code = "D= 0= ";
       else if (literal != null && Find.getValue(literal) == 0.)
        {
         root.right = null;
         root.code = "F0= 0= ";
        }
       else
        root.code = "F- F0= 0= ";
      }
     else if (root.operator == Keyword.LESSSY)
      {
       traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       castMax(root.left, root.right);
       root.type = new Type();
       root.type.type = Keyword.BOOLEANSY;
       root.loaded = true;
       if (root.left.type.dim != 0 || root.left.type.type == Keyword.NONESY)
        Find.error("operands must be primitives");
       else if (root.left.type.type == Keyword.BOOLEANSY)
        Find.error("operation not defined for boolean");
       Operation literal = Find.isLiteral(root.right);
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         if ((root.type.type == Keyword.FLOATSY || root.type.type == Keyword.DOUBLESY) && root.type.dim == 0)
          Find.toLiteral(root, Find.getValue(left) < Find.getValue(right)?"TRUE ":"FALSE ");
         else
          Find.toLiteral(root, Find.getLong(left) < Find.getLong(right)?"TRUE ":"FALSE ");
        }
       else if (root.left.type.dim != 0 || root.left.type.type == Keyword.INTSY || root.left.type.type == Keyword.BYTESY ||
           root.left.type.type == Keyword.SHORTSY || root.left.type.type == Keyword.CHARSY ||
           root.left.type.type == Keyword.BOOLEANSY || root.left.type.type == Keyword.NONESY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "0< ";
         }
        else
         root.code = "< ";
       else if (root.left.type.type == Keyword.LONGSY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "D0< ";
         }
        else
         root.code = "D< ";
       else if (literal != null && Find.getValue(literal) == 0.)
        {
         root.right = null;
         root.code = "F0< ";
        }
       else
        root.code = "F< ";
      }
     else if (root.operator == Keyword.LESSEQUALSY)
      {
       traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       castMax(root.left, root.right);
       root.type = new Type();
       root.type.type = Keyword.BOOLEANSY;
       root.loaded = true;
       if (root.left.type.dim != 0 || root.left.type.type == Keyword.NONESY)
        Find.error("operands must be primitives");
       Operation literal = Find.isLiteral(root.right);
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         if ((root.type.type == Keyword.FLOATSY || root.type.type == Keyword.DOUBLESY) && root.type.dim == 0)
          Find.toLiteral(root, Find.getValue(left) <= Find.getValue(right)?"TRUE ":"FALSE ");
         else
          Find.toLiteral(root, Find.getLong(left) <= Find.getLong(right)?"TRUE ":"FALSE ");
        }
       else if (root.left.type.dim != 0 || root.left.type.type == Keyword.INTSY || root.left.type.type == Keyword.BYTESY ||
           root.left.type.type == Keyword.SHORTSY || root.left.type.type == Keyword.CHARSY ||
           root.left.type.type == Keyword.BOOLEANSY || root.left.type.type == Keyword.NONESY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "0> INVERT ";
         }
        else
         root.code = "> INVERT ";
       else if (root.left.type.type == Keyword.LONGSY)
        if (literal != null && Find.getLong(literal) == 0.)
         {
          root.right = null;
          root.code = "DNEGATE DROP 0< INVERT ";
         }
        else
         root.code = "D> INVERT ";
       else if (literal != null && Find.getValue(literal) == 0.)
        {
         root.right = null;
         root.code = "A:R1@ A:R@ A:F0= F0< R> OR ";
        }
       else
        root.code = "F- A:R1@ A:R@ A:F0= F0< R> OR ";
      }
     else if (root.operator == Keyword.GREATERSY)
      {
       traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       castMax(root.left, root.right);
       root.type = new Type();
       root.type.type = Keyword.BOOLEANSY;
       root.loaded = true;
       if (root.left.type.dim != 0 || root.left.type.type == Keyword.NONESY)
        Find.error("operands must be primitives");
       else if (root.left.type.type == Keyword.BOOLEANSY)
        Find.error("operation not defined for boolean");
       Operation literal = Find.isLiteral(root.right);
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         if ((root.type.type == Keyword.FLOATSY || root.type.type == Keyword.DOUBLESY) && root.type.dim == 0)
          Find.toLiteral(root, Find.getValue(left) > Find.getValue(right)?"TRUE ":"FALSE ");
         else
          Find.toLiteral(root, Find.getLong(left) > Find.getLong(right)?"TRUE ":"FALSE ");
        }
       else if (root.left.type.dim != 0 || root.left.type.type == Keyword.INTSY || root.left.type.type == Keyword.BYTESY ||
           root.left.type.type == Keyword.SHORTSY || root.left.type.type == Keyword.CHARSY ||
           root.left.type.type == Keyword.BOOLEANSY || root.left.type.type == Keyword.NONESY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "0> ";
         }
        else
         root.code = "> ";
       else if (root.left.type.type == Keyword.LONGSY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "DNEGATE D0< ";
         }
        else
         root.code = "D> ";
       else if (literal != null && Find.getValue(literal) == 0.)
        {
         root.right = null;
         root.code = "FNEGATE F0< ";
        }
       else
        root.code = "F- FNEGATE F0< ";
      }
     else if (root.operator == Keyword.GREATEREQUALSY)
      {
       traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       castMax(root.left, root.right);
       root.type = new Type();
       root.type.type = Keyword.BOOLEANSY;
       root.loaded = true;
       if (root.left.type.dim != 0 || root.left.type.type == Keyword.NONESY)
        Find.error("operands must be primitives");
       else if (root.left.type.type == Keyword.BOOLEANSY)
        Find.error("operation not defined for boolean");
       Operation literal = Find.isLiteral(root.right);
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         if ((root.type.type == Keyword.FLOATSY || root.type.type == Keyword.DOUBLESY) && root.type.dim == 0)
          Find.toLiteral(root, Find.getValue(left) >= Find.getValue(right)?"TRUE ":"FALSE ");
         else
          Find.toLiteral(root, Find.getLong(left) >= Find.getLong(right)?"TRUE ":"FALSE ");
        }
       else if (root.left.type.dim != 0 || root.left.type.type == Keyword.INTSY || root.left.type.type == Keyword.BYTESY ||
           root.left.type.type == Keyword.SHORTSY || root.left.type.type == Keyword.CHARSY ||
           root.left.type.type == Keyword.BOOLEANSY || root.left.type.type == Keyword.NONESY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "0< INVERT ";
         }
        else
         root.code = "< INVERT ";
       else if (root.left.type.type == Keyword.LONGSY)
        if (literal != null && Find.getLong(literal) == 0)
         {
          root.right = null;
          root.code = "D0< INVERT ";
         }
        else
         root.code = "D< INVERT ";
       else if (literal != null && Find.getValue(literal) == 0.)
        {
         root.right = null;
         root.code = "F0< INVERT ";
        }
       else
        root.code = "F< INVERT ";
      }
     else if (root.operator == Keyword.ASHIFTSY)
      {
       long result;
       Type dest = new Type();
       dest.type = Keyword.LONGSY;

       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       if (root.left.type.dim > 0 || root.left.type.type == Keyword.NONESY ||
           root.left.type.type == Keyword.BOOLEANSY ||
           root.left.type.type == Keyword.DOUBLESY ||
           root.left.type.type == Keyword.FLOATSY)
        Find.error("left operand must be a (long) integer");
       root.left.code += castElem(dest, root.left.type, root.left);
       traverse(root.right, false, false, false, preorder, false, false);
       if (root.right.type.dim > 0 || root.right.type.type == Keyword.NONESY ||
           root.right.type.type == Keyword.BOOLEANSY ||
           root.right.type.type == Keyword.DOUBLESY ||
           root.right.type.type == Keyword.FLOATSY ||
           root.right.type.type == Keyword.LONGSY)
        Find.error("left operand must be an integer");
       root.type = new Type(root.left.type);
       root.loaded = true;
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         String s;
         long r = (long)Find.getLong(left) >> (int)Find.getLong(right);
         if (root.type.type == Keyword.LONGSY && Math.abs((long)r) <= Integer.MAX_VALUE)
          s = r + " S>D ";
         else
          s = r + " ";
         Find.toLiteral(root, s);
        }
       else if (right != null && root.left.type.type == Keyword.INTSY && (result = Find.getLong(right)) >= 0 && result < 5)
        {
         for(int i = 0; i < result; i++)
          root.left.code += "2/ ";
         root.right = null;
        }
       else if (right != null && root.left.type.type == Keyword.INTSY && (result = Find.getLong(right)) < 0 && result >= -5)
        {
         for(int i = 0; i < -result; i++)
          root.left.code += "2* ";
         root.right = null;
        }
       else
        root.code += "ASHIFT " + castElem(root.left.type, dest, null);
      }
     else if (root.operator == Keyword.LSHIFTSY)
      {
       long result;
       Type dest = new Type();
       dest.type = Keyword.LONGSY;

       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       if (root.left.type.dim > 0 || root.left.type.type == Keyword.NONESY ||
           root.left.type.type == Keyword.BOOLEANSY ||
           root.left.type.type == Keyword.DOUBLESY ||
           root.left.type.type == Keyword.FLOATSY)
        Find.error("left operand must be a (long) integer");
       root.left.code += castElem(dest, root.left.type, root.left);
       traverse(root.right, false, false, false, preorder, false, false);
       if (root.right.type.dim > 0 || root.right.type.type == Keyword.NONESY ||
           root.right.type.type == Keyword.BOOLEANSY ||
           root.right.type.type == Keyword.DOUBLESY ||
           root.right.type.type == Keyword.FLOATSY ||
           root.right.type.type == Keyword.LONGSY)
        Find.error("left operand must be an integer");
       root.type = new Type(root.left.type);
       root.loaded = true;
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         String s;
         long r = (long)Find.getLong(left) << (int)Find.getLong(right);
         if (root.type.type == Keyword.LONGSY && Math.abs((long)r) <= Integer.MAX_VALUE)
          s = r + " S>D ";
         else
          s = r + " ";
         Find.toLiteral(root, s);
        }
       else if (right != null && root.left.type.type == Keyword.INTSY && (result = Find.getLong(right)) >= 0 && result < 5)
        {
         for(int i = 0; i < result; i++)
          root.left.code += "2* ";
         root.right = null;
        }
       else if (right != null && root.left.type.type == Keyword.INTSY && (result = Find.getLong(right)) < 0 && result >= -5)
        {
         for(int i = 0; i < -result; i++)
          root.left.code += "2/ ";
         root.right = null;
        }
       else
        root.code += "SHIFTL " + castElem(root.left.type, dest, null);
      }
     else if (root.operator == Keyword.RSHIFTSY)
      {
       long result;
       Type dest = new Type();
       dest.type = Keyword.LONGSY;

       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       if (root.left.type.dim > 0 || root.left.type.type == Keyword.NONESY ||
           root.left.type.type == Keyword.BOOLEANSY ||
           root.left.type.type == Keyword.DOUBLESY ||
           root.left.type.type == Keyword.FLOATSY)
        Find.error("left operand must be a (long) integer");
       root.left.code += castElem(dest, root.left.type, root.left);
       traverse(root.right, false, false, false, preorder, false, false);
       if (root.right.type.dim > 0 || root.right.type.type == Keyword.NONESY ||
           root.right.type.type == Keyword.BOOLEANSY ||
           root.right.type.type == Keyword.DOUBLESY ||
           root.right.type.type == Keyword.FLOATSY ||
           root.right.type.type == Keyword.LONGSY)
        Find.error("left operand must be an integer");
       root.type = root.left.type;
       root.loaded = true;
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         String s;
         long r = (long)Find.getValue(left) >> (int)Find.getValue(right);
         if (root.type.type == Keyword.LONGSY && Math.abs((long)r) <= Integer.MAX_VALUE)
          s = r + " S>D ";
         else
          s = r + " ";
         Find.toLiteral(root, s);
        }
       else if (right != null && root.left.type.type == Keyword.INTSY && (result = Find.getLong(right)) < 0 && result >= -5)
        {
         for(int i = 0; i < -result; i++)
          root.left.code += "2* ";
         root.right = null;
        }
       else
        root.code += "SHIFTR " + castElem(root.left.type, dest, null);
      }
     else if (root.operator == Keyword.MODULOSY)
      {
       long result;
       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       if (root.left.type.dim > 0 || root.left.type.type == Keyword.NONESY ||
           root.left.type.type == Keyword.BOOLEANSY ||
           root.left.type.type == Keyword.DOUBLESY ||
           root.left.type.type == Keyword.FLOATSY)
        Find.error("left operand must be a (long) integer");
       traverse(root.right, false, false, false, preorder, false, false);
       if (root.right.type.dim > 0 || root.right.type.type == Keyword.NONESY ||
           root.right.type.type == Keyword.BOOLEANSY ||
           root.right.type.type == Keyword.DOUBLESY ||
           root.right.type.type == Keyword.FLOATSY)
        Find.error("left operand must be a (long) integer");
       castMax(root.left, root.right);
       Type dest = root.left.type;

       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         String s;
         result = Find.getLong(left) % Find.getLong(right);
         if (dest.type == Keyword.LONGSY && Math.abs((long)result) <= Integer.MAX_VALUE)
          s = (long)result + " S>D ";
         else
          s = (int)result + " ";
         root.type = dest;
         Find.toLiteral(root, s);
        }
       else if (right != null && dest.type != Keyword.DOUBLESY &&
                (result = (long)(Math.log(Math.abs(Find.getValue(right))) / Math.log(2.))) == Math.log(Math.abs(Find.getValue(right))) / Math.log(2.))
        {
         root.operator = null;
         root.right = null;
         if (Find.getValue(right) < 0.)
          result = -result;

         if (dest.type == Keyword.LONGSY)
          root.code = "A:R1@ DABS A:" + (int)result + " A:XOR " + ((1L << (int)Math.abs(result)) - 1) +
                      " A:R> A:R1@ NIP AND A:AND R@ R1@ A:2DROP 0< IF DNEGATE ENDIF " + root.code;
         else
          root.code = "A:R@ ABS A:" + (int)result + " A:XOR " + ((1 << (int)Math.abs(result)) - 1) +
                      " AND R> 0< IF NEGATE ENDIF " + root.code;
        }
       else if (root.left.type.dim == 0 && root.left.type.type == Keyword.LONGSY)
        root.code += "DMOD ";
       else if (root.left.type.dim == 0 && (root.left.type.type == Keyword.INTSY ||
                root.left.type.type == Keyword.BYTESY ||
                root.left.type.type == Keyword.SHORTSY || root.left.type.type == Keyword.CHARSY))
        {
         root.code += "MOD ";
         dest = new Type();
         dest.type = Keyword.INTSY;
        }
       else
        Find.error("only integer types allowed");

       root.type = dest;
       root.loaded = true;
      }
     else if (root.operator == Keyword.SLASHSY)
      {
       long result;
       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       castMax(root.left, root.right);
       Type dest = root.left.type;
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         String s;
         result = Find.getLong(left) / Find.getLong(right);
         if (dest.type == Keyword.DOUBLESY)
          s = (Find.getValue(left) / Find.getValue(right)) + " ";
         else if (dest.type == Keyword.LONGSY && Math.abs((long)result) <= Integer.MAX_VALUE)
          s = (long)result + " S>D ";
         else
          s = (int)result + " ";
         root.type = dest;
         Find.toLiteral(root, s);
        }
       else if (right != null && Find.getValue(right) == 0.)
        {
         root.operator = null;
         root.right = null;
         if (dest.type == Keyword.DOUBLESY)
          root.code = "DROP 2* DROP 7fe00000H 1 RSHIFTC 0 " + root.code;
         else if (dest.type == Keyword.LONGSY)
          root.code = "2DROP 0 0 " + root.code;
         else
          root.code = "DROP 0 " + root.code;
        }
       else if (right != null && Math.abs(Find.getValue(right)) == 1.)
        {
         root.operator = null;
         root.right = null;
         if (Find.getValue(right) < 0.)
          if (dest.type == Keyword.DOUBLESY)
           root.code = "FNEGATE " + root.code;
          else if (dest.type == Keyword.LONGSY)
           root.code = "DNEGATE " + root.code;
          else
           root.code = "NEGATE " + root.code;
        }
       else if (right != null && dest.type != Keyword.DOUBLESY &&
                (result = (long)(Math.log(Math.abs(Find.getValue(right))) / Math.log(2.))) == Math.log(Math.abs(Find.getValue(right))) / Math.log(2.))
        {
         root.operator = null;
         root.right = null;
         if (Find.getValue(right) < 0.)
          result = -result;

         if (dest.type == Keyword.LONGSY)
          root.code = (int)result + " ASHIFT " + root.code;
         else if (result == 1.)
          root.code = "2/ " + root.code;
         else if (result == -1.)
          root.code = "2/ NEGATE " + root.code;
         else
          root.code = "S>D " + (int)result + " ASHIFT NIP " + root.code;
        }
       else if (root.left.type.dim == 0 && (root.left.type.type == Keyword.DOUBLESY || root.left.type.type == Keyword.FLOATSY))
        root.code += "F/ ";
       else if (root.left.type.dim == 0 && root.left.type.type == Keyword.LONGSY)
        root.code += "D/ ";
       else if (root.left.type.dim == 0 && (root.left.type.type == Keyword.INTSY ||
                root.left.type.type == Keyword.BYTESY ||
                root.left.type.type == Keyword.SHORTSY || root.left.type.type == Keyword.CHARSY))
        {
         root.code += "/ ";
         dest = new Type();
         dest.type = Keyword.INTSY;
        }
       else
        Find.error("only arithmetic types allowed");

       root.type = dest;
       root.loaded = true;
      }
     else if (root.operator == Keyword.MULTIPLYSY)
      {
       long result;
       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       castMax(root.left, root.right);
       Type dest = root.left.type;
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         String s;
         result = Find.getLong(left) * Find.getLong(right);
         if (dest.type == Keyword.DOUBLESY)
          s = (Find.getValue(left) * Find.getValue(right)) + " ";
         else if (dest.type == Keyword.LONGSY && Math.abs((long)result) <= Integer.MAX_VALUE)
          s = (long)result + " S>D ";
         else
          s = (int)result + " ";
         root.type = dest;
         Find.toLiteral(root, s);
        }
       else if (left != null && Find.getValue(left) == 0.)
        {
         root.operator = null;
         root.left = null;
         root.right = null;

         if (dest.type == Keyword.DOUBLESY)
          root.code = "0. " + root.code;
         else if (dest.type == Keyword.LONGSY)
          root.code = "0 0 " + root.code;
         else
          root.code = "0 " + root.code;
        }
       else if (left != null && Math.abs(Find.getValue(left)) == 1.)
        {
         root.operator = null;
         root.left = null;
         if (Find.getValue(left) < 0.)
          if (dest.type == Keyword.DOUBLESY)
           root.code = "FNEGATE " + root.code;
          else if (dest.type == Keyword.LONGSY)
           root.code = "DNEGATE " + root.code;
          else
           root.code = "NEGATE " + root.code;
        }
       else if (left != null && dest.type != Keyword.DOUBLESY &&
                (result = (long)(Math.log(Math.abs(Find.getValue(left))) / Math.log(2.))) == Math.log(Math.abs(Find.getValue(left))) / Math.log(2.))
        {
         root.operator = null;
         root.left = null;
         if (Find.getValue(left) < 0.)
          result = -result;

         if (dest.type == Keyword.LONGSY)
          root.code = (int)result + " SHIFTL " + root.code;
         else if (result == 1.)
          root.code = "2* " + root.code;
         else if (result == -1.)
          root.code = "2* NEGATE " + root.code;
         else
          root.code = "S>D " + (int)result + " SHIFTL NIP " + root.code;
        }
       else if (right != null && Find.getValue(right) == 0.)
        {
         root.operator = null;
         root.left = null;
         root.right = null;
         if (dest.type == Keyword.DOUBLESY)
          root.code = "0. " + root.code;
         else if (dest.type == Keyword.LONGSY)
          root.code = "0 0 " + root.code;
         else
          root.code = "0 " + root.code;
        }
       else if (right != null && Math.abs(Find.getValue(right)) == 1.)
        {
         root.operator = null;
         root.right = null;
         if (Find.getValue(right) < 0.)
          if (dest.type == Keyword.DOUBLESY)
           root.code = "FNEGATE " + root.code;
          else if (dest.type == Keyword.LONGSY)
           root.code = "DNEGATE " + root.code;
          else
           root.code = "NEGATE " + root.code;
        }
       else if (right != null && dest.type != Keyword.DOUBLESY &&
                (result = (long)(Math.log(Math.abs(Find.getValue(right))) / Math.log(2.))) == Math.log(Math.abs(Find.getValue(right))) / Math.log(2.))
        {
         root.operator = null;
         root.right = null;
         if (Find.getValue(right) < 0.)
          result = -result;

         if (dest.type == Keyword.LONGSY)
          root.code = (int)result + " SHIFTL " + root.code;
         else if (result == 1.)
          root.code = "2* " + root.code;
         else if (result == -1.)
          root.code = "2* NEGATE " + root.code;
         else
          root.code = "S>D " + (int)result + " SHIFTL NIP " + root.code;
        }
       else if (root.left.type.dim == 0 && (root.left.type.type == Keyword.DOUBLESY || root.left.type.type == Keyword.FLOATSY))
        root.code += "F* ";
       else if (root.left.type.dim == 0 && root.left.type.type == Keyword.LONGSY)
        root.code += "D* ";
       else if (root.left.type.dim == 0 && (root.left.type.type == Keyword.INTSY ||
                root.left.type.type == Keyword.BYTESY ||
                root.left.type.type == Keyword.SHORTSY || root.left.type.type == Keyword.CHARSY))
        {
         root.code += "* ";
         dest = new Type();
         dest.type = Keyword.INTSY;
        }
       else
        Find.error("only arithmetic types allowed");

       root.type = dest;
       root.loaded = true;
      }
     else if (root.operator == Keyword.MINUSSY)
      {
       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       castMax(root.left, root.right);
       Type dest = root.left.type;
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       if (left != null && right != null)
        {
         String s;
         double result = Find.getValue(left) - Find.getValue(right);
         if (dest.type == Keyword.DOUBLESY)
          s = result + " ";
         if (dest.type == Keyword.LONGSY && Math.abs((long)result) <= Integer.MAX_VALUE)
          s = (long)result + " S>D ";
         else
          s = (long)result + " ";
         root.type = dest;
         Find.toLiteral(root, s);
        }
       else if (root.left.type.dim == 0 && (root.left.type.type == Keyword.DOUBLESY || root.left.type.type == Keyword.FLOATSY))
        root.code += "F- ";
       else if (root.left.type.dim == 0 && root.left.type.type == Keyword.LONGSY)
        root.code += "D- ";
       else if (root.left.type.dim == 0 && (root.left.type.type == Keyword.INTSY ||
                root.left.type.type == Keyword.BYTESY ||
                root.left.type.type == Keyword.SHORTSY || root.left.type.type == Keyword.CHARSY))
        {
         root.code += "- ";
         dest = new Type();
         dest.type = Keyword.INTSY;
        }
       else
        Find.error("only arithmetic types allowed");

       root.type = dest;
       root.loaded = true;
      }
     else if (root.operator == Keyword.PLUSSY)
      {
       if (!skipleft)
        traverse(root.left, false, false, false, preorder, false, false);
       traverse(root.right, false, false, false, preorder, false, false);
       Operation left = Find.isLiteral(root.left);
       Operation right = Find.isLiteral(root.right);
       castMax(root.left, root.right);
       Type dest = root.left.type;

       if (left != null && right != null && (dest.type != Keyword.NONESY || dest.ident.string.compareTo("String") == 0))
        {
         String s;
         double result;

         if (dest.type == Keyword.NONESY)
          {
           if (left.type.type == Keyword.BYTESY || left.type.type == Keyword.CHARSY)
            s = "U\" " + (char)Integer.parseInt(left.code.trim()) + Find.getString(right) + " \" " + right.code;
           else if (left.type.type == Keyword.INTSY || left.type.type == Keyword.LONGSY ||
                    left.type.type == Keyword.DOUBLESY || left.type.type == Keyword.SHORTSY)
            s = "U\" " + left.code.substring(0, left.code.indexOf(' ')) + Find.getString(right) + " \" " + right.code;
           else if (left.type.type == Keyword.BOOLEANSY)
            s = "U\" " + left.code.trim().toLowerCase() + Find.getString(right) + " \" " + right.code;
           else if (right.type.type == Keyword.BYTESY || right.type.type == Keyword.CHARSY)
            s = "U\" " + Find.getString(left) + (char)Integer.parseInt(right.code.trim()) + " \" " + left.code;
           else if (right.type.type == Keyword.INTSY || right.type.type == Keyword.LONGSY ||
                    right.type.type == Keyword.DOUBLESY || right.type.type == Keyword.SHORTSY)
            s = "U\" " + Find.getString(left) + left.code.substring(0, left.code.indexOf(' ')) + " \" " + left.code;
           else if (right.type.type == Keyword.BOOLEANSY)
            s = "U\" " + Find.getString(left) + left.code.trim().toLowerCase() + " \" " + left.code;
           else
            s = "U\" " + Find.getString(left) + Find.getString(right) + left.code;

           locals.remove(locals.size() - 1);
           int x = s.lastIndexOf("DUP");
           if (rhs && x >= 0)
            {
             s = s.substring(0, x);
             locals.remove(locals.size() - 1);
            }
          }
         else if (dest.type == Keyword.DOUBLESY)
          s = (Find.getValue(left) + Find.getValue(right)) + " ";
         else if (dest.type == Keyword.LONGSY)
          s = (long)(result = Find.getValue(left) + Find.getValue(right)) + (Math.abs((long)result) <= Integer.MAX_VALUE?" S>D ":" ");
         else
          s = (long)(Find.getValue(left) + Find.getValue(right)) + " ";
         root.type = dest;
         Find.toLiteral(root, s);
        }
       else if (root.left.type.dim == 0 && (root.left.type.type == Keyword.DOUBLESY || root.left.type.type == Keyword.FLOATSY))
        root.code += "F+ ";
       else if (root.left.type.dim == 0 && root.left.type.type == Keyword.LONGSY)
        root.code += "D+ ";
       else if (root.left.type.dim == 0 && (root.left.type.type == Keyword.INTSY ||
                root.left.type.type == Keyword.BYTESY ||
                root.left.type.type == Keyword.SHORTSY || root.left.type.type == Keyword.CHARSY))
        {
         root.code += "+ ";
         dest = new Type();
         dest.type = Keyword.INTSY;
        }
       else if (root.left.type.dim == 0 && root.left.type.type == Keyword.NONESY && root.left.type.ident.string.endsWith("String"))
        {
         Type [] t = new Type[1];
         t[0] = root.left.type;
         MethodType [] m = Find.getMethod("concat", t[0], t, findClass("String", 0));
         root.code += incrementReference("DUP") + " SWAP " + Find.buildQualifiedName(m[0].scope, m[0], false, "") + (!rhs?"DUP TO " + locals.size() + "§ ":"");
         if (!rhs)
          locals.add(((MethodType)m[0]).type);
        }
       else
        Find.error("only arithmetic types and strings allowed");

       root.type = dest;
       root.loaded = true;
      }
     else if (root.operator == Keyword.QUESTIONMARKSY)
      {
       traverse(root.left, false, false, false, preorder, false, false);
       if (root.left.type.type != Keyword.BOOLEANSY)
        Find.error("leading expression must be boolean");
       root.left.code += "\nIF\n";
       int a = traverse(root.right.left, false, rhs, false, preorder, false, incrThis);
       int b = traverse(root.right.right, false, rhs, false, preorder, false, incrThis);
       castMax(root.right.left, root.right.right);
       root.type = root.right.type = root.right.left.type;
       root.loaded = true;
       if (rhs && (a & LOADED) == 0 && (b & LOADED) != 0 &&
           (root.right.type.dim > 0 || root.right.type.type == Keyword.NONESY))
        root.right.code += incrementReference("DUP");
       else if (incrThis && (b & ISTHIS) != 0)
        {
         root.right.code += incrementReference("DUP");
        }
       root.right.code += "\nENDIF\n";
       if (rhs && (a & LOADED) != 0 && (b & LOADED) == 0 &&
           (root.left.type.dim > 0 || root.left.type.type == Keyword.NONESY))
        root.left.code += incrementReference("DUP");
       else if (incrThis && (a & ISTHIS) != 0)
        {
         root.right.left.code += incrementReference("DUP");
        }
       root.right.left.code += "\nELSE\n";
       modify = a & b;
      }
     else if (root.operator == Keyword.ALLOCATESY)
      {
       if (root.left != null)
        traverse(root.left, lhs, rhs, false, preorder, false, false);
       if (root.right != null)
        traverse(root.right, lhs, rhs, false, preorder, false, false);

       root.code = (theClass.scope.offset + CodeRCmodified.area) +
                   " MALLOC DROP DUP " + toVariable("§this"); // allocate class storage
       root.code += "OVER IF OVER CELL+ @ " + Find.addOffset(CodeRCmodified.son) + "OVER OVER ! DROP ENDIF\n" + // store $this as son in superclass
                    "CELL+ @ OVER\n" +
                    "OVER " + Find.addOffset(CodeRCmodified.father) + "! " + "NIP\n"; // store father in object
       if (Find.theScope.get("§outer").length != 0)
        root.code += "§outer " + incrementReference("DUP") + "OVER " + Find.addOffset(CodeRCmodified.outer) + "!\n"; // store outer in object
       root.code += Find.buildQualifiedName(theClass.scope.prev, theClass, false, "").trim() + ".table OVER " + Find.addOffset(CodeRCmodified.table) + "!\n";
       root.code += Find.crc(member.type) + " OVER " + Find.addOffset(CodeRCmodified.coded) + "!\n"; // store code
       root.code += "\" " + theClass.name.string.substring(theClass.name.string.lastIndexOf('.') + 1) + " \" OVER " + Find.addOffset(CodeRCmodified.name) + "!\n"; // store class name
       root.code += theClass.scope.refcount + " OVER " + Find.addOffset(CodeRCmodified.refcount) + "! DROP\n"; // store code
      }
     else if (root.operator == Keyword.RETURNSY)
      {
       if (root.left != null)
        {
         if ((traverse(root.left, false, true, false, preorder, false, true) & (ISTHIS | LOADED)) != 0)
          if (root.left.type.dim != 0 || root.left.type.type == Keyword.NONESY)
           root.left.code += incrementReference("DUP");

         root.left.code += castElem(member.type, root.left.type, root.left);
        }
       else if ((member.modify & Keyword.CONSTRUCTORSY.value) != 0)
        {
         root.code += "§this ";
        }

       if (root.left != null)
        if (root.left.type.dim != 0 || root.left.type.type == Keyword.NONESY)
         root.code += "DUP 0 V! ";

       int operand = 0;
       Scope t;
       for(t = Find.theScope; t.block != Scope.MAIN; t.breaks = true, t = t.prev)
        operand++;

       t.breaks = true;
       operand = (operand << 16) | operand;

       toAppend = operand + " §break" + Find.theScope.label.get(0) + " BRANCH\n";
      }
     else if (root.operator == Keyword.BREAKSY)
      {
       int operand = 0;
       Scope t = null, s = Find.theScope;
       Scope last = null;

       if (root.name != null)
        {
         s = Find.theScope.findLabel(root.name.string);
         if (s == null)
          Find.error("undefined label " + root.name.string);
         else
          for(t = Find.theScope; t != s; t = t.prev)
           {
            operand++;
            last = t;
            t.breaks = true;
           }
         if (operand > 0)
          operand--;
        }
       else
        {
         for(t = Find.theScope; t != null && t.block != Scope.SWITCH && t.block != Scope.LOOP; t.breaks = true, t = t.prev)
          operand++;
        }

       if (t == null)
        Find.error("break not allowed in this context");
       else
       	t.breaks = true;

       operand = (operand << 16) | operand;
       root.code += (last != null && last.block == Scope.TRY?"0 ":"") + operand + " §break" + Find.theScope.label.get(0) + " BRANCH\n";
      }
     else if (root.operator == Keyword.CONTINUESY)
      {
       int operand = 0;
       Scope t = null, s = Find.theScope, last = null;

       if (root.name != null)
        {
         s = Find.theScope.findLabel(root.name.string);
         if (s == null)
          Find.error("undefined label " + root.name.string);
         else
          for(t = Find.theScope; t != s; t = t.prev)
           {
            operand++;
            last = t;
            t.breaks = true;
           }
         if (operand > 0)
          operand--;
        }
       else
        {
         for(t = Find.theScope; t != null && t.block != Scope.LOOP; t.breaks = true, t = t.prev)
          operand++;
        }

       if (t == null || last != null && last.block != Scope.DUMMY && last.block != Scope.LOOP)
        Find.error("continue not allowed in this context");
       else
    	t.continues = true;

       operand = ((operand - 1) << 16) | operand;
       if (operand < 0)
        root.code += operand + " §continue" + Find.theScope.label.get(0) + " BRANCH\n";
       else
        root.code += operand + " §break" + Find.theScope.label.get(0) + " BRANCH\n";
      }
     else if (root.operator == Keyword.PUSHSY)
      {
       if (root.left != null)
        traverse(root.left, lhs, rhs, false, preorder, false, false);
       if (root.right != null)
        traverse(root.right, lhs, rhs, false, preorder, false, false);
       scopes.add(Find.theScope);
       Find.theScope = root.scope;
       root.code += createLocals();
      }
     else if (root.operator == Keyword.POPSY)
      {
       if (root.left != null)
        traverse(root.left, lhs, rhs, false, preorder, false, false);
       if (root.right != null)
        traverse(root.right, lhs, rhs, false, preorder, false, false);
       root.code += purgeLocals();
       Find.theScope = (Scope) scopes.get(scopes.size() - 1);
       scopes.remove(scopes.size() - 1);
      }
     else if (root.operator == Keyword.LOCKSY)
      {
       label = "§synchronized" + sync.size();

       if (root.left != null)
        { // synchronized block
         traverse(root.left, false, false, false, preorder, false, false);

         ClassType [] c = findClass(root.left.type.ident.string, root.left.type.version);
         Basic [] b = c[0].scope.get("_staticThread");
         
         root.code += "LOCAL " + label + "\n" +
                      Find.buildQualifiedName(c[0].scope, b[0], false, "") + " @\n" +
                      label + " 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD\n";
        }
       else if ((member.modify & Keyword.STATICSY.value) == 0)
        { // synchronized method
         root.code = "§this ";

         Basic [] b = member.scope.prev.get("_staticThread");
         
         root.code += "LOCAL " + label + "\n" +
                      Find.buildQualifiedName(member.scope.prev, b[0], false, "") + " @\n" +
                      label + " 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD\n";
        }
       else
        { // synchronized class method
         Basic [] b = member.scope.prev.get("classMonitorEnter");
         root.code += Find.buildQualifiedName(member.scope.prev, b[0], false, "") + "\n";
        }

       sync.add(label);
      }
     else if (root.operator == Keyword.UNLOCKSY)
      {
       label = (String)sync.get(sync.size() - 1);

       if (root.left != null)
        { // synchronized block
         root.left = null;
        
         root.code += label + " 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD\n";

         root.code += "PURGE 1\n";
        }
       else if ((member.modify & Keyword.STATICSY.value) == 0)
        { // synchronized method
         root.code += label + " 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD\n";

         root.code += "PURGE 1\n";
        }
       else
        { // synchronized class method
         Basic [] b = member.scope.prev.get("classMonitorLeave");
         root.code += Find.buildQualifiedName(member.scope.prev, b[0], false, "") + "\n";
        }

       sync.remove(sync.size() - 1);
      }
     else if (root.operator == Keyword.THROWSY)
      {
       boolean isThrowing = member != null && member.throwing.length > 0;
       int a = traverse(root.left, false, true, false, preorder, false, false);

       int operand = 0;
       Scope t;
       for(t = Find.theScope; t.block != Scope.MAIN && t.block != Scope.TRY; t.breaks = true, t = t.prev)
        operand++;

       t.breaks = true;
       operand = (operand << 16) | operand;

       if (t.block == Scope.MAIN && !isThrowing)
        {
         MethodType[] m = Find.getMethod("handler", null, null, findClass("JavaArray", 0));
         root.code = Find.buildQualifiedName(m[0].scope, m[0], false, "");
        }
       else
        {
         if (t.block == Scope.MAIN)
          root.code = "§exception TO §return\n";
         if ((a & LOADED) != 0)
          root.left.code += incrementReference("DUP");
         //root.code += operand + " §break" + Find.theScope.label.get(0) + " BRANCH\n";
        }
      }
     else if (root.operator == Keyword.EXITSY)
      {
       boolean isThrowing = member != null && member.throwing.length > 0;
       int operand = 0;
       Scope t;
       for(t = Find.theScope; t.block != Scope.MAIN && t.block != Scope.TRY; t = t.prev)
        operand++;

       operand = (operand << 16) | operand;
       if (!(t.block == Scope.MAIN && !isThrowing))
        root.code += operand + " §break" + Find.theScope.label.get(0) + " BRANCH\n";
      }
     else if (root.operator == Keyword.BLOCKENDSY)
      {
       String s;
      
       Scope scope;
      
       for(scope = Find.theScope; scope != null; scope = scope.prev)
   	    if (root.code.compareTo((String)scope.label.get(0)) == 0)
   	     break;
      
       if (scope != null)
        {
    	 s = "";
         if (scope.block == Scope.LOOP)
          if (scope.continues)
           s = "0<! §break" + root.code + " 0BRANCH §continue" + root.code + " BRANCH ";
          else
           {
            if (scope.breaks)
       	     s = "0<! §break" + root.code + " 0BRANCH ";
           }
         else if (scope.breaks)
          s = "§break" + root.code + " BRANCH ";

         if (s.length() > 0)
          root.code = "0>! IF 10001H - " + s + "ENDIF DROP ";
         else
          root.code = "DROP ";
        }
       else
   	    Find.error("label " + root.code + " not found");
      }
     else if (root.operator == Keyword.LABELSY)
      {
       String s = "";
       
       if (!root.code.startsWith("§continue"))
        s = "0 "+ root.code;
       else if (Find.theScope.continues)
        s = "DUP " + root.code + "DROP ";

       root.code = s;
      }
     else
      { // process sons only, operator has no meaning
       if (root.left != null)
        traverse(root.left, lhs, rhs, false, preorder, false, false);
       if (root.right != null)
        traverse(root.right, lhs, rhs, false, preorder, false, false);
      }

     // clear stack?
     if (statement && root.loaded)
      {
       if (root.type.dim == 0 && (root.type.type == Keyword.LONGSY || root.type.type == Keyword.FLOATSY || root.type.type == Keyword.DOUBLESY))
        root.code += "2DROP ";
       else if (root.type.type != Keyword.VOIDSY)
        root.code += "DROP ";

       root.loaded = false;
      }

     return modify;
    }

   /**
    * process primary and selector of operand
    * @param root Operation primary, selector
    * @param lhs Operand is left hand side of expression
    * @param rhs Operand is right hand side of expression
    * @return bit 31 variable the very first time referenced
    *         bit 30 a variable read out
    */
   private static int primarySelector(Operation root, boolean lhs, boolean rhs, Operation preorder) throws Exception
    {
     Operation last = null, next = root.left, ptr = next.left;
     ClassType myClass = theClass;
     Scope scope = Find.theScope;
     Type type = new Type(), pushed = null;
     type.type = Keyword.VOIDSY;
     String qualified = "";
     String trailer = "";
     boolean loaded = false;
     boolean isSuper = false;
     boolean isThis = false;
     boolean rhsm = rhs && (root.right == null && next.right == null);
     Token lastLocal = null, previous = null;
     int modify = 0;
     int indexed = 0;

     try
     {
     if (ptr.operator == Keyword.LPARSY)
      { // expression
       modify = traverse(ptr.left, lhs, rhsm, false, preorder, false, false);

       scope = Find.theScope;
       type = new Type(ptr.left.type);
       loaded = ptr.left.loaded;
       pushed = new Type(type);

       if (type.type == Keyword.NONESY)
        {
         ClassType [] c = getNearest(Find.findClass(type.ident.string, type.version, scope), scope);

         if (c.length == 1)
          {
           myClass = c[0];
           scope = myClass.scope;
           //type.ident = myClass.name;
           type.version = myClass.version;
           type.modify = myClass.modify;
           type.type = Keyword.NONESY;
          }
         else if (c.length == 0)
          throw new CodeException("undefined type, looking for " + type.toString());
         else
          throw new CodeException("type " + type.toString() + " not unique");
        }

       next = next.right;
       last = ptr;
      }
     else if (ptr.operator == Keyword.LITERALSY)
      { // literal
       type = new Type(ptr.type);
       // String
       if (type.type == Keyword.NONESY && type.ident.string.compareTo("String") == 0)
        {
         MethodType [] m = Find.getMethod("createUnicode", null, null, findClass("JavaArray", 0));
         ptr.code += Find.buildQualifiedName(m[0].scope, m[0], false, "COUNT ");
         if (!rhsm)
          {
           ptr.code += "DUP TO " + locals.size() + "§ ";
           locals.add(new Type(type));
          }
         myClass = findClass("String", 0)[0];
         scope = myClass.scope;
         type.ident = myClass.name;
         type.version = myClass.version;
         type.modify = myClass.modify;
         type.type = Keyword.NONESY;
        }

       pushed = new Type(type);
       loaded = true;
       next = next.right;
       last = ptr;
      }
     else if (ptr.operator == Keyword.NEWSY)
      { //new
       type = new Type(ptr.left.type);

       if (ptr.left.operator == Keyword.LBRACESY)
        { // array initializer
         arrayInitializer(ptr.left, type, rhsm);
        }
       else if (ptr.left.left.operator == Keyword.LBRACKETSY)
        {
         arrayCreator(ptr.left, rhsm);
        }
       else
        {
         Find.setPosition(ptr.left.type.ident);
         ptr.left.left.type = ptr.left.type;
         myClass = classCreator(ptr.left.left, trailer, type, preorder);
         trailer = "";
         scope = myClass.scope;
         type = new Type(ptr.left.type);
         //type.ident = myClass.name;
         type.version = myClass.version;
         type.modify = myClass.modify;
         type.type = Keyword.NONESY;

         if (!rhsm)
          {
           // store in hidden local variable
           ptr.code += "DUP TO " + locals.size() + "§ ";
           locals.add(new Type(type));
          }
        }

       if (!Parser.typeKW.contains(type.type))
        {
         ClassType [] c = getNearest(Find.findClass(type.ident.string, type.version, scope), scope);

         if (c.length == 1)
          {
           myClass = c[0];
           scope = myClass.scope;
           //type.ident = myClass.name;
           type.version = myClass.version;
           type.modify = myClass.modify;
           type.type = Keyword.NONESY;
          }
         else if (c.length == 0)
          throw new CodeException("undefined type, looking for " + type.toString());
         else
          throw new CodeException("type " + type.toString() + " not unique");
        }

       pushed = new Type(type);
       loaded = true;

       next = next.right;
       last = ptr;
      }
     else if (ptr.operator == Keyword.THISSY)
      {
       myClass = theClass;
       ptr.type = new Type();
       ptr.type.type = Keyword.NONESY;
       ptr.type.ident = theClass.name;
       ptr.type.version = theClass.version;
       ptr.type.modify = theClass.modify;
       scope = myClass.scope;
       type = new Type(ptr.type);

       if (ptr.left != null && ptr.left.operator == Keyword.LPARSY)
        {
         ptr.left.type = type;
         classCreator(ptr.left, trailer, type, preorder);
         trailer = "";
         pushed = new Type(type);
         loaded = true;

         ptr.code += toVariable("§this");
        }
       else if ((member.modify & Keyword.STATICSY.value) != 0)
        {
         qualified = theClass.name.string;
        }
       else
        {
         Scope [] s = Find.theScope.locate("§this", Keyword.PRIVATESY.value + Keyword.PROTECTEDSY.value, false);
         Basic [] b;

         if (s.length == 0 || (b = s[0].get("§this")).length == 0)
          throw new CodeException("this not fetchable in " + member.toString());
         else
          lastLocal = b[0].name;

         isThis = ptr.left == null;
        }

       next = next.right;
       last = ptr;
      }
     else if (ptr.operator == Keyword.SUPERSY)
      {
       if (theClass.extend.name != null)
        {
         myClass = theClass.extend;
         ptr.type.ident = myClass.name;
         ptr.type.version = myClass.version;
         ptr.type.modify = myClass.modify;
         ptr.type.type = Keyword.NONESY;
         scope = myClass.scope;
         type = new Type(ptr.type);
        }

       loaded = false;

       if (ptr.left != null && ptr.left.operator == Keyword.LPARSY)
        if (theClass.name.string.compareTo("Object") == 0 || theClass.extend.name == null ||
            (theClass.extend.modify & Keyword.ABSTRACTSY.value) != 0)
         { // Object has no superclass, replace super by null
          ptr = root.left = new Operation();
          ptr.code = "0 ";
          loaded = true;
         }
        else
         {
          ptr.left.type = type;
          classCreator(ptr.left, trailer, type, preorder);
          trailer = "";
          pushed = new Type(type);
          // object super must remain on data stack!
          loaded = true;
         }
       else
        {
         trailer += "§this CELL+ " + Find.addOffset(CodeRCmodified.father);
         isSuper = true;
        }

       next = next.right;
       last = ptr;
      }

     while (next != null)
      {
       ptr = next.left;
       previous = lastLocal;
       lastLocal = null;
       Basic [] p = null;

       modify = 0;
       rhsm = rhs && (root.right == null && next.right == null);

       if (type.dim > 0)
        {
         ClassType [] c = findClass("JavaArray", 0);
         myClass = c[0];
         scope = c[0].scope;
         type = new Type();
         type.type = Keyword.NONESY;
         type.ident = myClass.name;
         type.version = myClass.version;
         type.modify = myClass.modify;
        }

       if (ptr.operator == Keyword.THISSY)
        {
         if (qualified.length() > 0)
          {
           String tail = qualified.substring(qualified.lastIndexOf('.') + 1);
           Scope [] s = Find.theScope.locate("§this", Keyword.PRIVATESY.value | Keyword.PROTECTEDSY.value, false);
           if (s.length == 1 && Find.theScope.buildPath("").indexOf(qualified) >= 0)
            {
             trailer = "§this ";
             Scope y = Find.theScope;
             while(y != null && (y.prefix == null || y.prefix.length() == 0))
              y = y.prev;
             for(String fetch = ""; y != null && (y.prefix == null || tail.compareTo(y.prefix) != 0); y = y.prev)
              if (y.prefix != null && y.prefix.length() > 0)
               {
                trailer += fetch + "CELL+ @ " + Find.addOffset(CodeRCmodified.outer);
                fetch = "@ ";
               }

             if (y != null)
              {
               Basic [] b = y.prev.get(tail);

               for(int i = 0; i < b.length; i++)
                if (b[i] instanceof ClassType && ((ClassType)b[i]).scope == y)
                 {
                  myClass = (ClassType)b[i];
                  break;
                 }

               type = new Type();
               type.type = Keyword.NONESY;
               type.ident = myClass.name;
               type.version = myClass.version;
               type.modify = myClass.modify;

               loaded = false;
               qualified = "";

               scope = y;
              }
             else
              throw new CodeException("path " + qualified + " not found");
            }
           else
            throw new CodeException("object not fetchable in static context");
          }
         else
          throw new CodeException("no path present");
        }
       else if (ptr.operator == Keyword.SUPERSY)
        if (type.type == Keyword.NONESY && myClass.extend != null && myClass.extend.name != null)
         {
          if ((trailer.length() > 0 || previous != null))
           trailer += insertLoad(type, previous, 0);

          myClass = myClass.extend;
          type = new Type();
          type.ident = myClass.name;
          type.type = Keyword.NONESY;
          type.version = myClass.version;
          type.modify = myClass.modify;

          trailer += "CELL+ " + Find.addOffset(CodeRCmodified.father);
          loaded = false;
          qualified = "";
          isSuper = true;
         }
        else
         throw new CodeException("primitive types have no superclass");
       else
        {
         // ident
         ClassType [] c = new ClassType[1];

         Find.setPosition(ptr.name);

         if (ptr.left != null && ptr.left.operator == Keyword.LPARSY)
          { // method
           if (qualified.length() > 0)
            {
             c = findClass(qualified, 0);
             if (c.length > 1)
              throw new CodeException("ambiguous path " + qualified);
             else if (c.length == 0)
              throw new CodeException("path " + qualified + " not found");
            }
           else if (type.type == Keyword.VOIDSY || Parser.typeKW.contains(type.type))
            c[0] = theClass;
           else
            c[0] = myClass;

           if (!loaded)
            {
             pushed = type;
             trailer += insertLoad(type, previous, 0);
            }
           MethodType z = methodCall(ptr, c[0], pushed, trailer, isSuper, rhsm, preorder);
           type = new Type(z.type);
           pushed = new Type(type);

           trailer = "";
           qualified = "";
           scope = Find.theScope;
           myClass = theClass;

           if (type.type == Keyword.NONESY)
            {
             c = getNearest(Find.findClass(type.ident.string, type.version, z.scope), z.scope);

             if (c.length == 0)
              throw new CodeException("class " + type.ident.string + " not found");
             else if (c.length > 1)
              throw new CodeException("ambiguous class " + type.ident.string);
             else
              {
               myClass = c[0];
               scope = myClass.scope;
              }
            }

           loaded = true;
          }
         else
          {
           Scope [] s = null;
           Basic x = null;

           if (qualified.length() > 0)
            {
             c = findClass(qualified, 0);
             s = new Scope[c.length];
             for(int i = 0; i < c.length; i++)
              s[i] = c[i].scope;
            }
           else if (type.dim != 0)
            {
             c = findClass("JavaArray", 0);
             s = c[0].scope.locate(ptr.name.string, 0, false);
             scope = s[0];
            }
           else if (pushed != null || type.type != Keyword.VOIDSY && !Parser.typeKW.contains(type.type) || (s = Find.find(ptr.name.string, Find.theScope, next.right != null || root.right != null)).length == 0)
            s = scope.locate(ptr.name.string, 0, false);

           Scope old = scope;
           for(int j = 0; j < s.length && x == null; j++)
            {
             p = s[j].get(ptr.name.string);
             scope = s[j];
             for(int i = 0; i < p.length; i++)
              if ((p[i] instanceof VariableType || p[i] instanceof ClassType && (next.right != null || root.right != null)))
               {
                x = p[i];
                break;
               }
            }

           if (x == null)
            if (pushed == null && type.type == Keyword.VOIDSY)
             {
              qualified += (qualified.length() == 0 || qualified.endsWith(".")?"":".") + ptr.name.string;
             }
            else
             throw new CodeException(ptr.name.string + " not found");
           else if (x instanceof ClassType)
             { // class or interface
              ClassType z = (ClassType)x;

              qualified = z.scope.buildPath("");
              trailer = "";
              scope = z.scope;
              type.ident = z.name;
              type.type = Keyword.NONESY;
              type.dim = 0;
              type.version = z.version;
              type.modify = z.modify;
              pushed = null;
              loaded = false;

              myClass = z;
             }
            else
             { // variable
              VariableType z = (VariableType) x;
              boolean referenced = z.referenced;

              for(Scope y = Find.theScope; !referenced && y != null && (y.prefix == null || y.prefix.length() == 0); y = (y == scope)?null:y.prev)
               referenced = y.block == Scope.LOOP || (y.prefix != null && y.prefix.length() > 0);

              modify = z.modify | (!referenced?Integer.MIN_VALUE:0);
              z.referenced = true;
              if (rhsm)
               modify |= LOADED;

              if ((trailer.length() > 0 || previous != null))
               trailer += insertLoad(type, previous, 0);

              type = new Type(z.type);
              type.modify = z.modify;
              loaded = false;

              if ((type.modify & Keyword.STATICSY.value) != 0 || scope.storageClass == Scope.heap)
               trailer += Find.buildQualifiedName(scope, z, false, "") + " ";
              else if (scope.storageClass == Scope.classed)
               {
                if ((pushed == null || pushed.type != Keyword.NONESY && pushed.dim == 0) && trailer.length() == 0)
                 if (member != null && (member.modify & Keyword.STATICSY.value) == 0)
                  trailer = "§this ";
                 else
                  throw new CodeException(z.toString() + " not fetchable in static context");
                String dist = getDistance(old, scope);
                trailer += dist + getOffset(z) + "( " + scope.buildPath(z.name.string) + " ) ";
                if (dist.length() == 0)
                 throw new CodeException("can not fetch " + ptr.name.string);
               }
              else
               {
                lastLocal = z.name;
                trailer = "";
               }

              qualified = "";

              if (type.type == Keyword.NONESY)
               {
                myClass = null;
                c = Find.findClass(type.ident.string, type.version, scope);
                if (c.length != 0)
                 {
                  myClass = c[0];
                  scope = myClass.scope;
                 }
                else
                 throw new CodeException("unresolved type " + type.toString());
               }
             }
          }
         isSuper = false;
        }

       next = next.right;
       last = ptr;
      }

     for (next = root.right; next != null; next = next.right)
      { // selector
       ptr = next.left;
       previous = lastLocal;
       lastLocal = null;
       Basic [] p = null;
       int oldindexed = indexed;

       indexed = 0;
       modify = 0;
       rhsm = rhs && next.right == null;

       if (type.dim > 0 && (ptr.operator == Keyword.IDENTSY || ptr.operator == Keyword.SUPERSY))
        {
         ClassType [] c = findClass("JavaArray", 0);
         myClass = c[0];
         scope = c[0].scope;
         type = new Type();
         type.type = Keyword.NONESY;
         type.ident = myClass.name;
         type.version = myClass.version;
         type.modify = myClass.modify;
        }

       if (ptr.operator == Keyword.IDENTSY)
        {
         ClassType [] c;
         Scope [] s;

         Find.setPosition(ptr.name);

         if (ptr.left != null && ptr.left.operator == Keyword.LPARSY)
          { // method
           if (!loaded)
            {
             pushed = type;
             trailer += insertLoad(type, previous, oldindexed);
            }

           MethodType z = methodCall(ptr, myClass, pushed, trailer, isSuper, rhsm, preorder);

           type = new Type(z.type);
           pushed = new Type(type);
           trailer = "";
           qualified = "";
           scope = Find.theScope;
           myClass = theClass;

           if (type.type == Keyword.NONESY)
            {
             c = getNearest(Find.findClass(type.ident.string, type.version, z.scope), z.scope);

             if (c.length == 0)
              throw new CodeException("class " + type.ident.string + " not found");
             else if (c.length > 1)
              throw new CodeException("ambiguous class " + type.ident.string);
             else
              {
               myClass = c[0];
               scope = myClass.scope;
              }
            }

           loaded = true;
          }
         else
          {
           Basic x = null;

           if (type.dim != 0)
            {
             c = findClass("JavaArray", 0);
             s = c[0].scope.locate(ptr.name.string, 0, false);
             scope = s[0];
            }
           else
            s = scope.locate(ptr.name.string, 0, false);

           Scope old = scope;
           for(int j = 0; j < s.length && x == null; j++)
            {
             p = s[j].get(ptr.name.string);
             scope = s[j];
             for(int i = 0; i < p.length && x == null; i++)
              if (p[i] instanceof VariableType)
               x = p[i];
            }

           if (x == null)
            throw new CodeException(ptr.name.string + " not found");
           else
            { // variable
             VariableType z = (VariableType) x;
             boolean referenced = z.referenced;

             for(Scope y = Find.theScope; !referenced && y != null && (y.prefix == null || y.prefix.length() == 0); y = (y == scope)?null:y.prev)
              referenced = y.block == Scope.LOOP || (y.prefix != null && y.prefix.length() > 0);

             modify = z.modify | (!referenced?Integer.MIN_VALUE:0);
             z.referenced = true;
             if (rhsm)
              modify |= LOADED;

             if ((trailer.length() > 0 || previous != null))
              trailer += insertLoad(type, previous, oldindexed);

             type = new Type(z.type);
             type.modify = z.modify;
             loaded = false;

             if ((type.modify & Keyword.STATICSY.value) != 0 || scope.storageClass == Scope.heap)
              trailer = Find.buildQualifiedName(scope, z, false, "") + " ";
             else if (s[0].storageClass == Scope.classed)
              {
               if ((pushed == null || pushed.type != Keyword.NONESY && pushed.dim == 0) && trailer.length() == 0)
                throw new CodeException("no object present!");
               String dist = getDistance(old, scope);
               trailer += dist + getOffset(z) + "( " + scope.buildPath(z.name.string) + " ) ";
               if (dist.length() == 0)
                throw new CodeException("can not fetch " + ptr.name.string);
              }
             else
              {
               lastLocal = z.name;
               trailer = "";
              }

             qualified = "";

             if (type.type == Keyword.NONESY)
              {
               myClass = null;
               c = Find.findClass(type.ident.string, type.version, scope);
               if (c.length != 0)
                {
                 myClass = c[0];
                 scope = myClass.scope;
                }
               else
                throw new CodeException("unresolved type " + type.toString());
              }
            }
          }
         isSuper = false;
        }
       else if (ptr.operator == Keyword.SUPERSY)
        if (myClass.extend.name == null)
         throw new CodeException("no superclass present!");
        else
         {
          myClass = myClass.extend;
          scope = myClass.scope;
          ptr.type.dim = 0;
          ptr.type.ident = myClass.name;
          ptr.type.type = Keyword.NONESY;
          ptr.type.version = myClass.version;
          ptr.type.modify = myClass.modify;
          ptr.name = myClass.name;

          classCreator(ptr, trailer, type, preorder);
          trailer = "";
          type = new Type(ptr.type);
          pushed = new Type(type);
          trailer = "";
          qualified = "";
          loaded = true;
          isSuper = true;

          // store in hidden local variable
          if (!rhsm)
           {
            ptr.code += "DUP TO " + locals.size() + "§ ";
            locals.add(new Type(type));
           }
         }
       else if (ptr.operator == Keyword.NEWSY)
        {
         Find.setPosition(ptr.left.type.ident);
         myClass = classCreator(ptr.left, trailer, type, preorder);
         trailer = "";
         scope = myClass.scope;
         type = new Type();
         type.ident = myClass.name;
         type.version = myClass.version;
         type.modify = myClass.modify;
         type.type = Keyword.NONESY;
         pushed = new Type(type);
         loaded = true;
         isSuper = false;
         trailer = "";
         qualified = "";

         // store in hidden local variable
         if (!rhsm)
          {
           ptr.code += "DUP TO " + locals.size() + "§ ";
           locals.add(new Type(type));
          }
        }
       else if (ptr.operator == Keyword.LBRACKETSY)
        { // '['
         if (trailer.length() > 0)
          {
           last.code += trailer;
           loaded = false;
          }

         if (!loaded || previous != null)
          last.code += insertLoad(type, previous, oldindexed);

         last.code += ">R";
         trailer = "";
         loaded = false;
         modify = LOADED;
         indexed = INDEXED;

         traverse(ptr.left, false, false, false, null, false, false);
         boolean l = ptr.left.loaded;

         if (ptr.left.type.dim > 0 || ptr.left.type.type != Keyword.INTSY && ptr.left.type.type != Keyword.SHORTSY && ptr.left.type.type != Keyword.CHARSY && ptr.left.type.type != Keyword.BYTESY)
          throw new CodeException("index must be integer, not " + ptr.left.type.toString());
         else if (type.dim <= 0)
          throw new CodeException("to many indices");
         else if (type.type == Keyword.VOIDSY)
          throw new CodeException("no array present");
         else
          {
           type.dim--;

           if (!l)
            ptr.left.code += insertLoad(ptr.left.type, ptr.left.name, 0);
           ptr.left.loaded = true;

           MethodType [] m = Find.getMethod("getElem", null, null, findClass("JavaArray", 0));
           ptr.code += Find.buildQualifiedName(m[0].scope, m[0], false, "R>");

           pushed = new Type(type);
          }

         isSuper = false;

         if (preorder != null && l)
          {
           Operation op;

           if (preorder.left == null)
            preorder.left = op = new Operation(ptr.left);
           else if (preorder.right == null)
            preorder.right = op = new Operation(ptr.left);
           else
            {
             op = new Operation();
             op.left = preorder.left;
             op.right = preorder.right;
             preorder.left = op;
             preorder.right = op = new Operation(ptr.left);
            }

           op.code += "TO " + locals.size() + "§ ";
           ptr.left.code = locals.size() + "§ ";
           ptr.left.name = null;

           locals.add(op.type);
           ptr.left.left = ptr.left.right = null;
          }
        }
       else
        throw new CodeException("misbalanced operator tree");

       last = ptr;
      }

    if (trailer.length() > 0)
     {
      root.code = trailer + root.code;
      loaded = false;
     }
    } catch(CodeException e) { Find.error(e.getMessage()); }

    root.name = lastLocal;
    root.loaded = loaded;
    root.type = new Type(type);
    root.scope = scope;

    return modify | indexed | (isThis?ISTHIS:0);
   }

   /**
    * select class closest to scope
    * @param c ClassType[]
    * @param scope Scope
    * @return ClassType[]
    */
   private static ClassType [] getNearest(ClassType [] c, Scope scope)
   {
    String a = scope.buildPath("");
    int max = -1, score = -1;

    for(int i = 0; i < c.length && c.length > 1; i++)
     {
      String b = c[i].scope.buildPath("");
      int cc = -1;
      for(int j = 0; j < a.length() && j < b.length() && a.charAt(j) == b.charAt(j); cc = j++);
      if (cc > score)
       {
        score = cc;
        max = i;
       }
     }

    if (max >= 0)
     {
      ClassType [] b = new ClassType[1];
      b[0] = c[max];
      c = b;
     }

    return c;
   }

  /**
   * generate code for a new empty array
   * @param ptr Operation operator tree
   * @param rhs is right hand side of expression
   */
  private static void arrayCreator(Operation ptr, boolean rhs) throws Exception
   {
    ClassType [] c;
    MethodType [] m = Find.getMethod("JavaArray", null, null, c = findClass("JavaArray", 0));
    String routine = ' ' + Find.buildQualifiedName(m[1].scope.prev.prev, c[0], false, "").trim() + ".table " + (m[1].version & -4) + " EXECUTE-NEW ";
    m = Find.getMethod("getElem", null, null, c);
    String getElem = ' ' + Find.buildQualifiedName(m[0].scope, m[0], false, "SWAP");

    Type t = new Type(ptr.type);

    for (Operation n = ptr.left; n != null; n = n.right)
     {
      traverse(n.left, false, false, false, null, false, false);
      if (n.left.type.type != Keyword.INTSY &&
          n.left.type.type != Keyword.SHORTSY &&
          n.left.type.type != Keyword.CHARSY ||
          n.left.type.dim != 0)
       Find.error("index not an integer!");

      if (n.right != null)
       {
        n.left.code += "A:R@ " + Find.crc(t) + routine + "R> 0\nDO\n";
        n.code = "OVER I " + getElem + " !\nLOOP\n" + n.code;
       }
      else
       n.left.code += Find.crc(t) + routine;
      t.dim--;
     }

    if (t.dim < 0)
     Find.error("too many indizes!");

    if (!rhs)
     {
      ptr.code += "DUP TO " + locals.size() + "§ ";
      locals.add(new Type(ptr.type));
     }
   }

  /**
   * generate code for instantiation of class
   * @param ptr Operation operator tree
   * @param trailer String leading code sequence
   * @param type Type type of leading code sequence
   * @return ClassType type of new instance
   * @throws CodeException
   */
  private static ClassType classCreator(Operation ptr, String trailer, Type type, Operation preorder) throws Exception
   {
    Basic x = null;
    Type [] t = arguments(ptr, false, preorder);
    Basic [] p = Find.getMethod(ptr.type.ident.string.substring(ptr.type.ident.string.lastIndexOf('.') + 1), null, t, findClass(ptr.type.ident.string, ptr.type.version));

    if (p.length == 0)
     throw new CodeException("constructor " + ptr.type.ident.string + " not found");
    else if (p.length > 1)
     throw new CodeException("multiple choice for " + ptr.type.ident.string);
    else if ((p[0].modify & Keyword.CONSTRUCTORSY.value) == 0)
     throw new CodeException("no constructor found");
    else
     { // new class instance
      MethodType z = (MethodType)p[0];

      if (z.parameter.length > 0 && z.parameter[0].name.string.startsWith("§outer"))
       {
        Scope [] s;

        if (trailer.length() > 0)
         {
          ptr.left = insertThis(ptr.left, trailer, type);
         }
        else if ((s = Find.theScope.locate("§outer", Keyword.PROTECTEDSY.value + Keyword.PRIVATESY.value, false)).length > 0)
         {
          VariableType b = (VariableType)s[0].get("§outer")[0];
          ptr.left = insertThis(ptr.left, "§outer ", b.type);
         }
        else if ((s = Find.theScope.locate("§this", Keyword.PROTECTEDSY.value + Keyword.PRIVATESY.value, false)).length > 0)
         {
          VariableType b = (VariableType)s[0].get("§this")[0];
          ptr.left = insertThis(ptr.left, "§this ", b.type);
         }
        else
         {
          ptr.left = insertThis(ptr.left, "0 ", z.parameter[0].type);
         }
       }

      exception = skipException(ptr, t, z, exception);

      if ((z.modify & Keyword.STATICSY.value) != 0)
       {
        ptr.code += Find.buildQualifiedName(z.scope, z, false, "");
       }
      else
       {
        String st = null;
        for(Iterator it = z.scope.prev.prev.iterator(); it.hasNext() && st == null;)
         {
          Basic b = (Basic)it.next();
          if (b instanceof ClassType && ((ClassType)b).scope == z.scope.prev)
           st = Find.buildQualifiedName(((ClassType)b).scope.prev, b, false, "").trim() + ".table ";
         }

        ptr.code += st + (z.version & -4) + " EXECUTE-NEW ";
       }

      // cast actuals
      Operation q = ptr.left;
      for(int i = 0; i < z.parameter.length && q != null; i++, q = q.right)
       q.left.code += castElem(z.parameter[i].type, q.left.type, q.left);

      Basic [] b = z.scope.prev.prev.get(ptr.type.ident.string.substring(ptr.type.ident.string.lastIndexOf('.') + 1));
      for(int i = 0; i < b.length; i++)
       if ((b[i].modify & Keyword.CLASSSY.value) != 0 && ((ClassType)b[i]).scope == z.scope.prev)
        x = b[i];
     }

    return (ClassType)x;
   }

  /**
   * generate code for a method call
   * @param ptr Operation operator tree
   * @param c ClassType class containing method
   * @param pushed Type type of data on top of stack
   * @param trailer String leading code sequence
   * @param isSuper boolean super call
   * @param rhs right hand side of expression
   * @return MethodType method
   * @throws CodeException
   */
  private static MethodType methodCall(Operation ptr, ClassType c, Type pushed, String trailer, boolean isSuper, boolean rhs, Operation preorder) throws Exception
   {
    Type [] t = arguments(ptr.left, c.name.string.compareTo("JavaArray") == 0 && ptr.name.string.compareTo("kill") == 0, preorder);
    Basic [] p = new Basic[0];
    ClassType [] a = new ClassType[1];
    a[0] = c;

    boolean empty;

    if (empty = (pushed == null || pushed.type == Keyword.VOIDSY))
     {
      pushed = new Type();
      pushed.type = Keyword.NONESY;
      pushed.ident = c.name;
      pushed.version = c.version;

      if (trailer.length() == 0)
       trailer = "§this ";
     }

    Find.setPosition(ptr.name);

    while(c != null)
     {
      // present in class or any superclass
      for(a[0] = c; a[0].scope != null && (p = Find.getMethod(ptr.name.string, null, t, a)).length == 0;)
       a[0] = a[0].extend;
      if (p.length > 0)
       break;

      // must be in outer class
      a[0] = null;
      trailer += "CELL+ @ " + Find.addOffset(CodeRCmodified.outer) + "@ ";

      for(Scope y = c.scope.prev; y != null && y.prev != null && a[0] == null; y = y.prev)
       if (y.prefix != null && y.prefix.length() > 0)
        for(Iterator iter = y.prev.iterator(); iter.hasNext() && a[0] == null;)
         {
          Basic b = (Basic)iter.next();
          if (b instanceof ClassType && ((ClassType)b).scope == y)
           {
            a[0] = (ClassType) b;

            pushed = new Type();
            pushed.type = Keyword.NONESY;
            pushed.ident = b.name;
            pushed.version = b.version;
           }
         }
      c = a[0];
     }

    MethodType z = null;

    if (p.length == 0)
     throw new CodeException("method " + ptr.name.string + " not present!");
    else if (p.length > 1)
     throw new CodeException("multiple choice for " + ptr.name.string + ", selection not decidable!");
    else if ((p[0].modify & Keyword.CONSTRUCTORSY.value) != 0)
     throw new CodeException("no constructor not allowed in this context");
    else
     {
      z = (MethodType) p[0];

      if (z.parameter.length > 0 && z.parameter[0].name.string.compareTo("§this") == 0)
       {
      	ptr.left.left = insertThis(ptr.left.left, trailer + ">R ", pushed);

        if (Find.theScope.locate("§this", Keyword.PROTECTEDSY.value + Keyword.PRIVATESY.value, false).length == 0 && empty)
         throw new CodeException("method not reachable in static context");
       }

      // fix possible exception
      exception = skipException(ptr.left, t, z, exception);

      // cast parameters
      Operation q = ptr.left.left;
      for(int i = 0; i < z.parameter.length && q != null; i++, q = q.right)
       q.left.code += castElem(z.parameter[i].type, q.left.type, q.left);

      ptr.code += Find.buildQualifiedName(z.scope, z, isSuper, "R>");

      // save result in temporary local
      if (!rhs)
       if (z.type.dim != 0 || z.type.type == Keyword.NONESY)
        {
         ptr.code += "DUP TO " + locals.size() + "§ ";
         locals.add(new Type(z.type));
        }
     }

    return z;
   }

  /**
   * generate code for actual parameter
   * @param root Operation operator tree for actuals
   * @param skip skip increment
   * @return Type[] list of types corresponding with actuals
   */
  private static Type [] arguments(Operation root, boolean skip, Operation preorder) throws Exception
   {
    Type [] parameter;

    int count = 0;
    for(Operation y = root.left; y != null; y = y.right)
     count++;
    parameter = new Type[count];
    count = 0;
    for(Operation y = root.left; y != null; y = y.right)
     {
      if ((traverse(y.left, false, true, false, preorder, false, false) & LOADED) != 0 && !skip && (y.left.type.dim > 0 || y.left.type.type == Keyword.NONESY))
       y.left.code += incrementReference("DUP");
      parameter[count++] = y.left.type;
     }

    return parameter;
   }

  /**
   * generate code for fetching a member of scope dest out of scope source
   * @param source Scope
   * @param dest Scope
   * @return String access path
   */
  private static String getDistance(Scope source, Scope dest)
   {
    if (source == null)
     return "";

    String x = "CELL+ @ ";
    String outer = "";

    while(source.prev != null && (source.prefix == null || source.prefix.length() == 0))
     source = source.prev;

    while(source.prev != null)
     {
      if (source == dest)
       return x;
      Iterator iter = source.prev.iterator();
      while(iter.hasNext() && outer.length() == 0)
       {
        Basic b = (Basic)iter.next();
        if (b instanceof ClassType && ((ClassType)b).scope == source)
         outer = getDistance(((ClassType)b).extend.scope, dest);
       }
      if (outer.length() > 0)
       return x + Find.addOffset(CodeRCmodified.father) + "@ " + outer;
      x += Find.addOffset(CodeRCmodified.outer) + "@ ";
      source = source.prev;
      while(source.prev != null && (source.prefix == null || source.prefix.length() == 0))
       source = source.prev;
     }

    return "";
   }

  /**
   * generate code for loading a variable onto stack
   * @param type Type
   * @param name Token
   * @param indexed int
   * @return String
   */
  private static String insertLoad(Type type, Token name, int indexed) throws Exception
   {
    if (name != null)
     return name.string + ' ';

    if (type == null || type.type == Keyword.VOIDSY)
     return "";

    String w = null;
    if (type.dim == 0)
     if (type.type == Keyword.BYTESY)
      w = "C@ ";
     else if (type.type == Keyword.CHARSY || type.type == Keyword.SHORTSY)
      w = "H@ ";
     else if (type.type == Keyword.LONGSY || type.type == Keyword.DOUBLESY ||
              type.type == Keyword.FLOATSY)
      w = "2@ ";
    if (w == null)
     w = "@ ";

    return w + (indexed == INDEXED?castElem(type, null, null):"");
   }

  /**
   * generate code for a new initialized array
   * @param root Operation operator tree for new array
   * @param t Type type of array
   * @param rhs right hand side of expression
   * @param unassigned true, if not assigned to a variable
   */
  private static void arrayInitializer(Operation root, Type t, boolean rhs) throws Exception
   {
    ClassType [] c;
    MethodType [] m = Find.getMethod("JavaArray", null, null, c = findClass("JavaArray", 0));
    String routine = ' ' + Find.buildQualifiedName(m[1].scope.prev.prev, c[0], false, "").trim() + ".table " + (m[1].version & -4) + " EXECUTE-NEW ";
    m = Find.getMethod("getElem", null, null, c);
    String getElem = ' ' + Find.buildQualifiedName(m[0].scope, m[0], false, "SWAP") + ' ';
    //int code = Find.crc(t);
    Type t1 = new Type(t);
    t1.dim--;

    root.code = "";

    String store = "! ";

    if (t1.dim == 0 && t1.type != Keyword.NONESY)
     if (t1.type == Keyword.BYTESY)
      {
       store = "C! ";
      }
     else if (t1.type == Keyword.CHARSY || t1.type == Keyword.SHORTSY)
      {
       store = "H! ";
      }
     else if (t1.type == Keyword.DOUBLESY || t1.type == Keyword.FLOATSY || t1.type == Keyword.LONGSY)
      {
       store = "2! ";
      }

    // allocate array
    root.left.code += Find.crc(t) + routine;
    if (!rhs)
     {
      root.left.code += "DUP TO " + locals.size() + "§ ";
      locals.add(new Type(t));
     }

    int i = 0;
    for(Operation ptr = root.right; ptr != null; ptr = ptr.right, i++)
     {
      int x = 0;

      if (ptr.left.operator == Keyword.LBRACESY)
       arrayInitializer(ptr.left, t1, true);
      else
       x = traverse(ptr.left, false, true, false, null, false, false);

      if (store.startsWith("2"))
       ptr.left.code += "2 PICK " + i + " " + getElem + store;
      else
       ptr.left.code += ((x & LOADED) != 0 && (ptr.left.type.dim > 0 || ptr.left.type.type == Keyword.NONESY)?incrementReference("DUP"):"") + "OVER " + i + " " + getElem + store;
     }
   }

  /**
   * insert leading parameter this into actual parameters
   * @param root Operation actual parameters list
   * @param code String code for leading parameter
   * @param type Type type of leading parameter
   * @return Operation updated list of actuals
   */
  private static Operation insertThis(Operation root, String code, Type type)
   {
    Operation z = new Operation();

    z.right = root;
    z.left = new Operation();
    z.left.code = code;

    if (type == null)
     {
      type = new Type();
      type.ident = theClass.name;
      type.type = Keyword.NONESY;
      type.version = theClass.version;
      type.modify = theClass.modify;
     }

    z.left.type = new Type(type);

    return z;
   }

  /**
   * generate code for last parameter exception
   * @param root Operation actual parameter list
   * @param t Type[] type list corresponding with actuals list
   * @param m MethodType aimed method
   * @param exception boolean true, if exception needed
   * @return boolean updated exception flag
   */
  private static boolean skipException(Operation root, Type [] t, MethodType m, boolean exception)
   {
    boolean hasException = (t.length > 0 && t[t.length - 1].type == Keyword.VOIDSY);

    if (!hasException && m.throwing.length > 0)
     { // insert a default exception return
      if (root.left == null)
       root = root.left = new Operation();
      else
       {
        for(root = root.left; root.right != null; root = root.right);
        root = root.right = new Operation();
       }

      root = root.left = new Operation();
      root.type = new Type();
      root.type.type = Keyword.VOIDSY;

      root.code = statementException + " VAL ";
     }
    else if (hasException && m.throwing.length == 0)
     { // remove exception from calling method
      if (root.left.right == null)
       root.left = null;
      else
       for(Operation ptr = root.left; ptr != null; ptr = ptr.right)
        if (ptr.right.right == null)
         ptr.right = null;
     }
    else if (hasException && m.throwing.length > 0)
     { // insert default exception return point
      for(root = root.left; root.right != null; root = root.right);
      root.left.code = statementException + " VAL ";
     }

    return exception || m.throwing.length > 0;
   }

  /**
   * generate code for typecast, prefix and postfix operations of an operand
   * @param primary Operation primary, selector
   * @param prepost Operation prefix, postfix
   * @param statement boolean true, if result not needed on data stack
   * @return Type
   */
  private static Type fixes(Operation primary, Operation prepost, boolean statement) throws Exception
   {
    Type t = new Type(primary.type);
    String trailer = "";
    boolean loaded = statement;

    if (!primary.loaded)
     {
      trailer = insertLoad(t, primary.name, 0);

      if (primary.name == null)
       {
    	int i;
    	
    	trailer = trailer.trim();
    	
    	for(i = trailer.length() - 1; i > 0 && trailer.charAt(i) != ' ';)
    	 i--;
    	
    	trailer = trailer.substring(0, i) + " A:R@ " + trailer.substring(i) + ' ';
       }

      if (prepost.left != null && prepost.right != null)
       if (primary.type.dim == 0 && (primary.type.type == Keyword.LONGSY || primary.type.type == Keyword.DOUBLESY || primary.type.type == Keyword.FLOATSY))
        trailer += "A:R1@ A:R@ A:ROT ";
       else
        trailer += "A:R@ A:SWAP ";
     }

    // prefix
    for(Operation ptr = prepost.left; ptr != null; ptr = ptr.right)
     {
      Operation root = ptr.left;
      root.code += trailer;
      // process operator
      if (root.operator == Keyword.CASTSY)
       {
        root.code += castElem(root.type, t, root);

        t = root.type;
       }
      else if (t.dim > 0)
       Find.error("prefix not allowed on array!");
      else if (root.operator == Keyword.INCSY)
       {
        if (primary.loaded)
         Find.error("++ operator not available for expression");

        String end = "";

        if (primary.name != null)
         end = "TO " + primary.name.string + ' ';
        else if (t.type == Keyword.BYTESY)
         end = "R@ C! ";
        else if (t.type == Keyword.SHORTSY || t.type == Keyword.CHARSY)
         end = "R@ H! ";
        else if (t.type == Keyword.INTSY || t.type == Keyword.BOOLEANSY)
         end = "R@ ! ";
        else
         end = "R@ 2! ";

        if ((t.type == Keyword.INTSY || t.type == Keyword.SHORTSY ||
             t.type == Keyword.CHARSY || t.type == Keyword.BYTESY))
         root.code += "1+ " + (!loaded?"DUP ":"") + end;
        else if (t.type == Keyword.LONGSY)
         root.code += "0 1 D+ " + (!loaded?"OVER OVER ":"") + end;
        else
         Find.error("prefix ++ not allowed here!");
       }
      else if (root.operator == Keyword.DECSY)
       {
        if (primary.loaded)
         Find.error("-- operator not available for expression");

        String end = "";

        if (primary.name != null)
         end = "TO " + primary.name.string + ' ';
        else if (t.type == Keyword.BYTESY)
         end = "R@ C! ";
        else if (t.type == Keyword.SHORTSY || t.type == Keyword.CHARSY)
         end = "R@ H! ";
        else if (t.type == Keyword.INTSY || t.type == Keyword.BOOLEANSY)
         end = "R@ ! ";
        else
         end = "R@ 2! ";

        if ((t.type == Keyword.INTSY || t.type == Keyword.SHORTSY ||
             t.type == Keyword.CHARSY || t.type == Keyword.BYTESY))
         root.code += "1- " + (!loaded?"DUP ":"") + end;
        else if (t.type == Keyword.LONGSY)
         root.code += "0 1 D- " + (!loaded?"OVER OVER ":"") + end;
        else
         Find.error("prefix -- not allowed here!");
       }
      else if (root.operator == Keyword.NOTSY)
       {
        statement = false;
        loaded = true;

        if (t.type == Keyword.BOOLEANSY)
         root.code += "0= ";
        else
         Find.error("prefix ! not allowed here!");
       }
      else if (root.operator == Keyword.INVERTSY)
       {
        statement = false;
        loaded = true;

        if ((t.type == Keyword.INTSY || t.type == Keyword.SHORTSY ||
             t.type == Keyword.CHARSY || t.type == Keyword.BYTESY))
         root.code += "INVERT ";
        else if (t.type == Keyword.LONGSY)
         root.code += "INVERT SWAP INVERT SWAP ";
        else
         Find.error("prefix ~ not allowed here!");
       }
      else if (root.operator == Keyword.PLUSSY)
       {
        statement = false;

        if (t.type != Keyword.INTSY && t.type != Keyword.SHORTSY && t.type != Keyword.CHARSY && t.type != Keyword.BYTESY &&
            t.type != Keyword.LONGSY && t.type != Keyword.FLOATSY && t.type != Keyword.DOUBLESY)
         Find.error("prefix + not allowed here!");

        loaded = true;
       }
      else if (root.operator == Keyword.MINUSSY)
       {
        statement = false;
        loaded = true;

        if ((t.type == Keyword.INTSY || t.type == Keyword.SHORTSY ||
             t.type == Keyword.CHARSY || t.type == Keyword.BYTESY))
         if (primary.left.left.operator == Keyword.LITERALSY)
          primary.left.left.code = '-' + primary.left.left.code;
         else
          root.code += "NEGATE ";
        else if (t.type == Keyword.LONGSY)
         if (primary.left.left.operator == Keyword.LITERALSY)
          primary.left.left.code = '-' + primary.left.left.code;
         else
          root.code += "DNEGATE ";
        else if (t.type == Keyword.FLOATSY || t.type == Keyword.DOUBLESY)
         if (primary.left.left.operator == Keyword.LITERALSY)
          primary.left.left.code = '-' + primary.left.left.code;
         else
          root.code += "FNEGATE ";
        else
         Find.error("prefix - not allowed here!");
       }

      trailer = "";
     }

    // postfix
    if (prepost.right != null)
     {
      Operation root = prepost.right;

      if (primary.loaded)
       Find.error("postfix operator not available for expression");
      else if (prepost.left != null)
       if (primary.type.dim == 0 && (primary.type.type == Keyword.LONGSY || primary.type.type == Keyword.DOUBLESY || primary.type.type == Keyword.FLOATSY))
        trailer += "A:ROT R> R1@ A:NIP ";
       else
        trailer += "R1@ A:NIP ";

      root.code = trailer;

      String end = "";

      if (primary.name != null)
       end = "TO " + primary.name.string + ' ';
      else if (primary.type.type == Keyword.BYTESY)
       end = "R@ C! ";
      else if (primary.type.type == Keyword.SHORTSY || primary.type.type == Keyword.CHARSY)
       end = "R@ H! ";
      else if (primary.type.type == Keyword.INTSY)
       end = "R@ ! ";
      else
       end = "R@ 2! ";

      if (t.dim > 0)
       Find.error("postfix not allowed on array!");
      else if (root.operator == Keyword.INCSY)
       {
        if ((primary.type.type == Keyword.INTSY || primary.type.type == Keyword.SHORTSY ||
             primary.type.type == Keyword.CHARSY || primary.type.type == Keyword.BYTESY))
         root.code += (!loaded?"DUP ":"") + "1+ " + end;
        else if (primary.type.type == Keyword.LONGSY)
         root.code += (!loaded?"OVER OVER ":"") + "0 1 D+ " + end;
        else
         Find.error("postfix ++ not allowed here!");
       }
      else if (root.operator == Keyword.DECSY)
       {
        if ((primary.type.type == Keyword.INTSY || primary.type.type == Keyword.SHORTSY ||
             primary.type.type == Keyword.CHARSY || primary.type.type == Keyword.BYTESY))
         root.code += (!loaded?"DUP ":"") + "1- " + end;
        else if (primary.type.type == Keyword.LONGSY)
         root.code += (!loaded?"OVER OVER ":"") + "0 1 D- " + end;
        else
         Find.error("postfix -- not allowed here!");
       }
     }

    if (!primary.loaded && primary.name == null)
     prepost.code += "A:DROP ";

    primary.loaded = !statement;

    return t;
   }

  /**
   * cast top element of data stack to type dest from type source
   * @param dest Type
   * @param source Type
   * @return String
   */
  private static String castElem(Type dest, Type source, Operation root) throws Exception
  {
   String x = "";

   if (source == null)
    return (dest.dim == 0 && dest.type == Keyword.NONESY)?" \" " + dest.ident.string.substring(dest.ident.string.lastIndexOf('.') + 1) + " \" CASTTO ":x;

   long val = 0;
   double fval = 0.;
   Operation literal = Find.isLiteral(root);
   
   if (literal != null)
    if (source.type == Keyword.BYTESY ||
   	 source.type == Keyword.CHARSY ||
   	 source.type == Keyword.SHORTSY ||
   	 source.type == Keyword.INTSY ||
   	 source.type == Keyword.LONGSY)
     val = Find.getLong(literal);
    else if (source.type == Keyword.FLOATSY ||
       	  source.type == Keyword.DOUBLESY)
     fval = Find.getValue(literal);
    else
     literal = null;
   
   if (dest.match(source) ||
       (dest.dim > 0 || dest.type == Keyword.NONESY) &&
       source.type == Keyword.NONESY && source.ident.string.compareTo("Object") == 0 ||
       dest.dim == 1 && dest.type == Keyword.NONESY && dest.ident.string.compareTo("Object") == 0 &&
       source.type == Keyword.NONESY &&
       source.ident.string.compareTo("JavaArray") == 0 ||
       source.dim > 0 && dest.type == Keyword.NONESY && dest.ident.string.compareTo("JavaArray") == 0)
    return x;

   if (dest.dim - source.dim == 1 && (source.dim > 0 || source.type == Keyword.NONESY))
    {
     ClassType [] c;
     MethodType [] m = Find.getMethod("JavaArray", null, null, c = findClass("JavaArray", 0));
     MethodType [] n = Find.getMethod("getElem", null, null, c);
     int code = Find.crc(dest);

     dest.dim--;
     if (dest.match(source))
      x = "1 " + code + " " + Find.buildQualifiedName(m[1].scope.prev.prev, c[0], false, "").trim() + ".table " + (m[1].version & -4) +
          " EXECUTE-NEW >R 0 " + Find.buildQualifiedName(n[0].scope, n[0], false, "R@") + " ! R> ";
     else
      Find.error("incompatible types "+"dest="+dest.toString()+" source=" + source.toString());
     dest.dim++;
    }
   else if (dest.type == Keyword.NONESY)
    if (source.type == Keyword.VOIDSY ||
        dest.ident.string.compareTo("Object") == 0 && dest.dim == 0 && (source.dim > 0 ||
        source.type == Keyword.NONESY) ||
        Find.isInterface(source, dest) || Find.isInterface(dest, source))
     return x;
    else if ((x = Find.isOuter(source, new Type(dest))).length() > 0 || Find.isSuperclass(dest, source))
     if (dest.dim != source.dim)
      Find.error("dimension does not match");
     else if (dest.dim == 0)
      x = " \" " + dest.ident.string.substring(dest.ident.string.lastIndexOf('.') + 1) + " \" CASTTO ";
     else
      x = "";
    else if (dest.ident.string.compareTo("String") == 0)
     {
      MethodType [] m = Find.getMethod("createString", null, null, findClass("JavaArray", 0));
      MethodType [] n = Find.getMethod("createUnicode", null, null, findClass("JavaArray", 0));

      String append = " DUP TO " + locals.size() + "§ ";
      locals.add(new Type(dest));

      if (source.type == Keyword.BYTESY)
       x = " <# HOLD DUP DUP #> " +Find.buildQualifiedName(m[0].scope, m[0], false, "") + append;
      else if (source.type == Keyword.INTSY)
       x = " <# 0 ..R " +Find.buildQualifiedName(m[0].scope, m[0], false, "") + append;
      else if (source.type == Keyword.SHORTSY)
       x = " 0<! IF 0FFFF0000H OR ELSE 0FFFFH AND ENDIF <# 0 ..R " +Find.buildQualifiedName(m[0].scope, m[0], false, "") + append;
      else if (source.type == Keyword.CHARSY)
       x = " <# DUP 8 RSHIFT HOLD HOLD DUP DUP #> " +Find.buildQualifiedName(n[0].scope, n[0], false, "") + append;
      else if (source.type == Keyword.LONGSY)
       x = " <# 0 D..R " +Find.buildQualifiedName(m[0].scope, m[0], false, "") + append;
      else if (source.type == Keyword.FLOATSY)
       x = " F.. " +Find.buildQualifiedName(m[0].scope, m[0], false, "") + append;
      else if (source.type == Keyword.DOUBLESY)
       x = " F.. " +Find.buildQualifiedName(m[0].scope, m[0], false, "") + append;
      else if (source.type == Keyword.BOOLEANSY)
       x = " IF \" true \" ELSE \" false \" ENDIF COUNT " +Find.buildQualifiedName(m[0].scope, m[0], false, "") + append;
     }
    else if (source.type == Keyword.NONESY && source.ident.string.compareTo("Object") == 0 && source.dim == 0)
     return x;
    else
     Find.error("incompatible types "+"dest="+dest.toString()+" source=" + source.toString());
   else if (dest.dim != 0 || source.dim != 0)
    ;//Find.error("primitive vectors of different type dest = " + dest.toString() + " source = " + source.toString());
   else if (dest.type == Keyword.BYTESY)
    if (literal != null)
     {
      if (source.type == Keyword.FLOATSY ||
   	   source.type == Keyword.DOUBLESY)
   	val = (long)fval;
      
      literal.code = (byte)val + " ";
      literal.type = dest;
     }
    else
     {
      if (source.type == Keyword.INTSY)
       x = " 127 U>! NIP IF 0FFFFFF00H OR ENDIF ";
      else if (source.type == Keyword.SHORTSY)
       x = " 127 U>! NIP IF 0FFFFFF00H OR ENDIF ";
      else if (source.type == Keyword.CHARSY)
       x = " 127 U>! NIP IF 0FFFFFF00H OR ENDIF ";
      else if (source.type == Keyword.LONGSY)
       x = " NIP 127 U>! NIP IF 0FFFFFF00H OR ENDIF ";
      else if (source.type == Keyword.FLOATSY)
       x = " F>D NIP 127 U>! NIP IF 0FFFFFF00H OR ENDIF ";
      else if (source.type == Keyword.DOUBLESY)
       x = " F>D NIP 127 U>! NIP IF 0FFFFFF00H OR ENDIF ";
      else if (source.type == Keyword.BOOLEANSY)
       Find.error("incompatible types "+"dest="+dest.toString()+" source=" + source.toString());
     }
   else if (dest.type == Keyword.SHORTSY)
    if (literal != null)
     {
      if (source.type == Keyword.FLOATSY ||
     	   source.type == Keyword.DOUBLESY)
     	val = (long)fval;
        
      literal.code = (short)val + " ";
      literal.type = dest;
     }
    else
     {
      if (source.type == Keyword.BYTESY)
       x = "";
      else if (source.type == Keyword.INTSY)
       x = " 32767 U>! NIP IF 0FFFF0000H OR ENDIF ";
      else if (source.type == Keyword.CHARSY)
       x = "";
      else if (source.type == Keyword.LONGSY)
       x = " NIP 32767 U>! NIP IF 0FFFF0000H OR ENDIF ";
      else if (source.type == Keyword.FLOATSY)
       x = " F>D NIP 32767 U>! NIP IF 0FFFF0000H OR ENDIF ";
      else if (source.type == Keyword.DOUBLESY)
       x = " F>D NIP 32767 U>! NIP IF 0FFFF0000H OR ENDIF ";
      else if (source.type == Keyword.BOOLEANSY)
       Find.error("incompatible types "+"dest="+dest.toString()+" source=" + source.toString());
     }
   else if (dest.type == Keyword.CHARSY)
    if (literal != null)
     {
      if (source.type == Keyword.FLOATSY ||
     	   source.type == Keyword.DOUBLESY)
     	val = (long)fval;
        
      literal.code = ((int)(char)val) + " ";
      literal.type = dest;
     }
    else
     {
      if (source.type == Keyword.BYTESY)
       x = "";
      else if (source.type == Keyword.SHORTSY)
       x = "";
      else if (source.type == Keyword.INTSY)
       x = " 0FFFFH AND ";
      else if (source.type == Keyword.LONGSY)
       x = " NIP 0FFFFH AND ";
      else if (source.type == Keyword.FLOATSY)
       x = " F>D NIP 0FFFFH AND ";
      else if (source.type == Keyword.DOUBLESY)
       x = " F>D NIP 0FFFFH AND ";
      else if (source.type == Keyword.BOOLEANSY)
       Find.error("incompatible types "+"dest="+dest.toString()+" source=" + source.toString());
     }
   else if (dest.type == Keyword.INTSY)
    if (literal != null)
     {
      if (source.type == Keyword.FLOATSY ||
    	   source.type == Keyword.DOUBLESY)
     	val = (long)fval;
        
      literal.code = (int)val + " ";
      literal.type = dest;
     }
    else
     {
      if (source.type == Keyword.BYTESY)
       x = "";
      else if (source.type == Keyword.SHORTSY)
       x = "";
      else if (source.type == Keyword.CHARSY)
       x = "";
      else if (source.type == Keyword.LONGSY)
       x = " D>S ";
      else if (source.type == Keyword.FLOATSY)
       x = " F>D D>S ";
      else if (source.type == Keyword.DOUBLESY)
       x = " F>D D>S ";
      else if (source.type == Keyword.BOOLEANSY)
       Find.error("incompatible types "+"dest="+dest.toString()+" source=" + source.toString());
     }
   else if (dest.type == Keyword.LONGSY)
    if (literal != null)
     {
      if (source.type == Keyword.FLOATSY ||
     	   source.type == Keyword.DOUBLESY)
     	val = (long)fval;
        
      literal.code = val + " ";
      if (Math.abs(val) <= Integer.MAX_VALUE)
       literal.code += "S>D ";
      literal.type = dest;
     }
    else
     {
      if (source.type == Keyword.BYTESY)
       x = " S>D ";
      else if (source.type == Keyword.SHORTSY)
       x = " S>D ";
      else if (source.type == Keyword.CHARSY)
       x = " S>D ";
      else if (source.type == Keyword.INTSY)
       x = " S>D ";
      else if (source.type == Keyword.FLOATSY)
       x = " F>D ";
      else if (source.type == Keyword.DOUBLESY)
       x = " F>D ";
      else if (source.type == Keyword.BOOLEANSY)
       Find.error("incompatible types "+"dest="+dest.toString()+" source=" + source.toString());
     }
   else if (dest.type == Keyword.FLOATSY || dest.type == Keyword.DOUBLESY)
    if (literal != null)
     {
      if (source.type != Keyword.FLOATSY &&
     	   source.type != Keyword.DOUBLESY)
     	fval = val;
        
      literal.code = fval + " ";
      literal.type = dest;
     }
    else
     {
      if (source.type == Keyword.BYTESY)
       x = " S>D D>F ";
      else if (source.type == Keyword.SHORTSY)
       x = " S>D D>F ";
      else if (source.type == Keyword.CHARSY)
       x = " S>D D>F ";
      else if (source.type == Keyword.INTSY)
       x = " S>D D>F ";
      else if (source.type == Keyword.LONGSY)
       x = " D>F ";
      else if (source.type == Keyword.BOOLEANSY)
       Find.error("incompatible types "+"dest="+dest.toString()+" source=" + source.toString());
     }
   else
    Find.error("incompatible types "+"dest="+dest.toString()+" source=" + source.toString());

   return x;
  }

 /**
  * cast operand with lower priority type to higher priority type of the other operand
  * @param left Operation operand
  * @param right Operation operand
  */
 private static void castMax(Operation left, Operation right) throws Exception
  {
   if (!left.type.match(right.type))
    if (left.type.type == Keyword.NONESY && left.type.ident.string.endsWith("String"))
     {
      right.code += castElem(left.type, right.type, left);
      right.type = left.type;
     }
    else if (right.type.type == Keyword.NONESY && right.type.ident.string.endsWith("String"))
     {
      left.code += castElem(right.type, left.type, left);
      left.type = right.type;
     }
    else if (left.type.type == Keyword.NONESY)
     {
      right.code += castElem(left.type, right.type, left);
      right.type = left.type;
     }
    else if (right.type.type == Keyword.NONESY)
     {
      left.code += castElem(right.type, left.type, left);
      left.type = right.type;
     }
    else if (left.type.type == Keyword.DOUBLESY || left.type.type == Keyword.FLOATSY)
     {
      right.code += castElem(left.type, right.type, left);
      right.type = left.type;
     }
    else if (right.type.type == Keyword.DOUBLESY || right.type.type == Keyword.FLOATSY)
     {
      left.code += castElem(right.type, left.type, left);
      left.type = right.type;
     }
    else if (left.type.type == Keyword.LONGSY)
     {
      right.code += castElem(left.type, right.type, left);
      right.type = left.type;
     }
    else if (right.type.type == Keyword.LONGSY)
     {
      left.code += castElem(right.type, left.type, left);
      left.type = right.type;
     }
    else if (left.type.type == Keyword.INTSY)
     {
      right.code += castElem(left.type, right.type, left);
      right.type = left.type;
     }
    else if (right.type.type == Keyword.INTSY)
     {
      left.code += castElem(right.type, left.type, left);
      left.type = right.type;
     }
    else if (left.type.type == Keyword.SHORTSY)
     {
      right.code += castElem(left.type, right.type, left);
      right.type = left.type;
     }
    else if (right.type.type == Keyword.SHORTSY)
     {
      left.code += castElem(right.type, left.type, left);
      left.type = right.type;
     }
    else if (left.type.type == Keyword.CHARSY)
     {
      right.code += castElem(left.type, right.type, left);
      right.type = left.type;
     }
    else if (right.type.type == Keyword.CHARSY)
     {
      left.code += castElem(right.type, left.type, left);
      left.type = right.type;
     }
    else if (left.type.type == Keyword.BYTESY)
     {
      right.code += castElem(left.type, right.type, left);
      right.type = left.type;
     }
    else if (right.type.type == Keyword.BYTESY)
     {
      left.code += castElem(right.type, left.type, left);
      left.type = right.type;
     }
    else if (left.type.type == Keyword.BOOLEANSY)
     {
      right.code += castElem(left.type, right.type, left);
      right.type = left.type;
     }
    else if (right.type.type == Keyword.BOOLEANSY)
     {
      left.code += castElem(right.type, left.type, left);
      left.type = right.type;
     }
  }
 }

