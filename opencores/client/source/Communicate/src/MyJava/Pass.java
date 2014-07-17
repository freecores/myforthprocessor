package MyJava;

/**
 * <p>Überschrift: Pass 1 des Compilers</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.io.*;
import java.util.*;

public class Pass extends Parser
 {
  private final boolean force;
  private final String gc;
  private Vector scopeStack;
  private int exceptionStack;
  private Vector follower;
  private Vector built;
  private HashSet unresolved;
  private Header [] list;
  private static int anonymous = 0;
  private Type returns = null;
  private int depth;
  private MethodType member = null;

  /**
   * constructor
   * @param input File java source file
   * @param built Vector contains the names of in previous passes compiled java sources
   * @param force boolean
   * @param gc String name of garbage collection strategie
   */
  public Pass(File input, Vector built, boolean force, String gc)
   {
    super(input);
    this.built = built;
    this.force = force;
    this.gc = gc;

    acceptComment = true;
    Errors.reset();
    exceptionStack = 0;

    // initialize list of terminal followers
    follower = new Vector();
    follower.add(Keyword.LBRACESY);
    follower.add(Keyword.SEMICOLONSY);
    follower.add(Keyword.LBRACKETSY);
    follower.add(Keyword.IFSY);
    follower.add(Keyword.FORSY);
    follower.add(Keyword.WHILESY);
    follower.add(Keyword.DOSY);
    follower.add(Keyword.TRYSY);
    follower.add(Keyword.SWITCHSY);
    follower.add(Keyword.SYNCHRONIZEDSY);
    follower.add(Keyword.RETURNSY);
    follower.add(Keyword.THROWSY);
    follower.add(Keyword.BREAKSY);
    follower.add(Keyword.CONTINUESY);
   }

   /**
    * compiles a single java source file, and resolve references
    * @param loaded Header[] list of all previously compiled sources in this pass
    * @param depth int identification for source
    * @return Header[] updated list of loaded headers
    */
   public Header [] compilationUnit(Header [] loaded, final int depth)
   {
	try
	{
    this.depth = depth;
    String trunc = sources.getPath();
    list = loaded == null?new Header[0]:loaded;
    countToken = 0;
    
    Find.notePass(null);
    
    lookAhead();

    // read package name
    String myPackage = null;
    if (nextSymbol == Keyword.PACKAGESY)
     {
      lookAhead();
      myPackage = qualident();
      matchKeyword(Keyword.SEMICOLONSY);
     }

    Header header = new Header(new File(Parser.jdkSource).getPath(), trunc, myPackage, depth);

    Vector imported = new Vector();
    imported.add("java.lang.");

    // read import statements
    while (nextSymbol == Keyword.IMPORTSY)
     imported.add(importDeclaration());

    header.imports = new String[imported.size()];
    for(int l = 0; l < imported.size(); l++)
     header.imports[l] = (String)imported.get(l);

    trunc = trunc.substring(0, sources.getPath().lastIndexOf(".java"));
    String basename = trunc.substring(trunc.lastIndexOf('\\') + 1);

    scopeStack = header.scopes;

    while(nextSymbol != Keyword.EOFSY)
     typeDeclaration(header.base, basename);

    matchKeyword(Keyword.EOFSY);

    if (Errors.count() != 0)
     return null;

    // write valid header
    header.write(trunc + ".h");

    // append compiled file to list
    Header [] newList = new Header[(list != null?list.length:0) + 1];

    for(int i = 0; i < newList.length - 1; i++)
     newList[i] = list[i];

    newList[newList.length - 1] = header;
    list = newList;
    connectClasses(list);

    // resolve superclasses and interfaces
    for(int i = 0; i < header.scopes.size() && list != null; i++)
     {
      Vector v = new Vector();
      Scope scope = (Scope)header.scopes.get(i);

      for(Iterator iter = scope.iterator(); iter.hasNext();)
       {
        Basic b = (Basic)iter.next();
        if (b instanceof ClassType)
         v = getExtendImplement((ClassType)b);
       }

      while(v.size() > 0 && list != null)
       {
        connectClasses(list);
        list = loadSource((String)v.get(0), list, header, force);
        v.remove(0);
       }
     }

    if (Errors.count() != 0 || header.depth > 0 || list == null)
     return list;

    // process imports
    for(int i = 0; i < imported.size() && list != null; i++)
     {
      String st = (String)imported.get(i);
      if (!st.endsWith("."))
       {
        connectClasses(list);
        list = loadSource(st.substring(st.lastIndexOf('.') + 1), list, header, force);
       }
     }

    if (Errors.count() != 0 || header.depth > 0 || list == null)
     return list;

    connectClasses(list);
    // process unresolved references
    for(int i = 0; i < header.scopes.size() && list != null; i++)
     {
      Scope scope = (Scope)header.scopes.get(i);
      Iterator step = scope.iterator();
      while(step.hasNext() && list != null)
       {
        Basic x = (Basic)step.next();

        if (x instanceof ClassType)
         {
          boolean inner = false;
          ClassType y = (ClassType)x;

          while(y.unresolved.size() > 0 && list != null)
           {
            Iterator it = y.unresolved.iterator();
            if (!it.hasNext())
             break;
            String name = (String)it.next();
            it = null;
            y.unresolved.remove(name);
            String [] s = name.split("\\.");
            Scope [] z = null;

            z = Find.find(s[0], scope, true);

            if (z.length == 0)
             {
              list = loadSource(s[0], list, header, force);
              connectClasses(list);
             }
            try {
            if (s.length > 1)
             {
              ClassType [] classes = null;
               
              classes = Find.findClass(s[0], 0, scope);
              
              if (classes.length == 0)
               {
                classes = new ClassType[1];
                classes[0] = y;
               }
              for(int k = 1; k < s.length && classes.length > 0; k++)
               {
                Basic[] b = classes[0].scope.get(s[k]);
                for (int j = 0; j < b.length; j++)
                 if (b[j] instanceof VariableType)
                  {
                   VariableType v = (VariableType) b[j];
                   if (v.type.type == Keyword.NONESY)
                    {
                     classes = Find.findClass(v.type.ident.string, 0, scope);
                     
                     if (classes.length == 0)
                      {
                       y.unresolved.add(v.type.ident.string);
                       //y.unresolved.add(name);
                      }
                    }
                   else
                    classes = new ClassType[0];
                   break;
                  }
                 else if (b[j] instanceof MethodType)
                  {
                   MethodType v = (MethodType) b[j];
                   if (v.type.type == Keyword.NONESY)
                    {
                     classes = Find.findClass(v.type.ident.string, 0, scope);
                     
                     if (classes.length == 0)
                      {
                       y.unresolved.add(v.type.ident.string);
                       //y.unresolved.add(name);
                      }
                    }
                   else
                    classes = new ClassType[0];
                   break;
                  }
                 else if (b[j] instanceof ClassType)
                  {
                   classes = new ClassType[1];
                   classes[0] = (ClassType) b[j];
                   break;
                  }
               }
             }
            } catch(Exception ee){error("nullpointer " + s[0] + '.' + s[1]);}
           }
         }
       }
     }

    if (depth > 0)
     return list;

    if (Errors.count() != 0 || list == null)
     return null;

    connectClasses(list);
    
    Find.notePass(this);

    // resolve operator tree to Forth-code
    FileOutputStream file = null, starter = null;
    try
     {
      file = new FileOutputStream(header.name.substring(0, header.name.lastIndexOf('.')) + ".fs");
      starter = new FileOutputStream(header.name.substring(0, header.name.lastIndexOf('.')) + ".start.fs");
      System.out.println("translate " + header.name);

      for(int k = 0; k < header.scopes.size(); k++)
       {
        Scope scope = (Scope)header.scopes.get(k);
        boolean module = (k == 0);
        for(Iterator iter = scope.iterator(); iter.hasNext();)
         {
          Basic b = (Basic)iter.next();
          if (b instanceof ClassType && (b.modify & Keyword.INTERFACESY.value) == 0)
           {
            if (gc.startsWith("m"))
             CodeRCmodified.codeClass( (ClassType) b, file, starter, module);
            else
             Code3color.codeClass( (ClassType) b, file, starter, module);
            module = false;
           }
         }
       }

      file.close();
      starter.close();
     }
    catch(IOException ex1) { ex1.printStackTrace(); }
    
    Find.notePass(this);
	}
	catch(Exception failed)
	{ error(failed.getMessage() + " aborted"); }
    
    return (Errors.count() != 0)?null:list;
   }

   /**
    * extract names of superclass and implemented interfaces
    * @param c ClassType the class
    * @return Vector extracted names
    */
   private Vector getExtendImplement(ClassType c)
   {
    String st;
    Vector v = new Vector();

    if (c.extend.name != null && c.extend.scope == null)
     {
      st = c.extend.name.string;
      if (!c.unresolved.contains(st))
       for(Iterator iter = c.unresolved.iterator(); iter.hasNext();)
        {
         String s = (String)iter.next();
         if (s.endsWith('.' + st))
          {
           st = s;
           break;
          }
        }

      v.add(st);
     }

    for(int j = 0; j < c.implement.length; j++)
     if (c.implement[j].scope == null)
      {
       st = c.implement[j].name.string;

       if (!c.unresolved.contains(st))
        for(Iterator iter = c.unresolved.iterator(); iter.hasNext();)
         {
          String s = (String)iter.next();
          if (s.endsWith('.' + st))
           {
            st = s;
            break;
           }
         }

       if (!v.contains(st))
        v.add(st);
      }

    return v;
   }

   /**
    * load header of dependent file or compile dependend file
    * @param name String filename
    * @param list Header[] header of already processed files
    * @param header Header parent of dependent
    * @param force boolean
    * @return Header[] updated list of headers
    */
   private Header [] loadSource(String name, Header [] list, Header header, final boolean force)
   {
    int pos;
    int depth = header.depth;
    File file = null;

    name = name.trim();

    for(pos = 0; pos < list.length && list[pos] != header; pos++);

    String path = list[pos].name.substring(0, list[pos].name.lastIndexOf('\\'));

    // look in sdk's root directory
    if ((file = getFile(Parser.jdkSource, name)) != null)
     {
      if (Parser.jdkSource.compareToIgnoreCase(path) != 0)
       depth++;
     }

    // look in package
    if (file == null)
     {
      file = getFile(path, name);
     }

    // look in imports
    for (int j = 0; j < header.imports.length && file == null; j++)
     {
      String st = header.imports[j];
      int last = st.lastIndexOf('.') + 1;

      if (st.substring(last).compareToIgnoreCase(name) == 0)
       st = st.substring(0, last);

      if (st.endsWith("."))
       st = st.substring(0, st.length() - 1);

      for (int k = -1; (k = st.indexOf('.')) >= 0; )
       st = st.substring(0, k) + '\\' + st.substring(k + 1);

      st = jdkSource + '\\' + st;

      file = getFile(st, name);

      if (file != null)
       depth++;
     }

    if (file != null)
     {
      String st = file.getAbsolutePath();

      // already loaded?
      for(int k = 0; k < list.length; k++)
       if (list[k].name.compareToIgnoreCase(st) == 0)
        {
         file = null;
         return list;
        }

      if (file != null)
       {
        File desc = new File(st.substring(0, st.lastIndexOf('.') + 1) + 'h');
        File fs = new File(st.substring(0, st.lastIndexOf('.') + 1) + "fs");

        st = file.getAbsolutePath();
        for(int i = 0; i < built.size(); i++)
         if (((String)built.get(i)).compareToIgnoreCase(st) == 0)
          {
           file = desc;
           break;
          }

        if ((!desc.exists() || !fs.exists() || fs.length() == 0 || fs.lastModified() < file.lastModified() || force) && file != desc)
         {
          Header [] hh 	= new Pass(file, built, force, gc).compilationUnit(list, depth); // compile

          return hh;
         }
        else
         {
          Header [] newList = new Header[list.length + 1];

          for (int k = 0; k < newList.length - 1; k++)
           newList[k] = list[k];
          System.out.println(desc.getAbsolutePath() + " will be read!");
          newList[newList.length - 1] = new Header(desc.getAbsolutePath(), depth); // load header
          list = newList;
         }
       }
     }

    return list;
   }

   /**
    * open specified file, if exists
    * @param path String
    * @param name String
    * @return File
    */
   private File getFile(String path, String name)
   {
    File file = null;
    boolean found = false;
    String ext = "";

    for (int i = 0; i < name.length(); i++)
     ext += "" + ( (name.charAt(i) == '.') ? '\\' : name.charAt(i));

    while(ext.length() > 0 && !found)
     {
      file = new File(path + "\\" + ext + ".java");
      found = file != null && file.exists() && file.isFile();
      ext = ext.substring(0, Math.max(0, ext.lastIndexOf('\\')));
     }

    return (found)?file:null;
   }

   /**
    * load header named name
    * @param list Header[]
    * @param name String
    * @return Header[]
    */
   public Header [] addHeader(Header [] list, String name)
   {
    String st = null;
    for(int i = 0; i < built.size(); i++, st = null)
     {
      st = (String)built.get(i);
      String tail = st.substring(st.lastIndexOf('\\') + 1);
      tail = tail.substring(0, tail.indexOf('.'));
      if (tail.compareToIgnoreCase(name) == 0)
       break;
     }
    if (st != null)
     {
      Header [] newList = new Header[list.length + 1];

      for (int k = 0; k < newList.length - 1; k++)
       newList[k] = list[k];

      st = st.substring(0, st.lastIndexOf('.')) + ".h";
      System.out.println("loading " + st);
      newList[newList.length - 1] = new Header(st, depth + 1); // load header
      list = newList;

      connectClasses(list);
     }

    return list;
   }

   /**
    * try to reduce number of unresolved references in loaded and compiled sources
    * @param list Header[]
    */
   private void connectClasses(Header [] list)
   {
    Vector queue;
    Vector garbage = new Vector();

    Find.setCrossreference(list);

    for(int i = 0; list != null && i < list.length; i++)
     {
      queue = list[i].scopes;

      for(int j = 0; j < queue.size(); j++)
       for(Iterator iter = ((Scope)queue.get(j)).iterator(); iter.hasNext();)
        {
         Iterator a = null;
         Basic x = (Basic)iter.next();

         if (x instanceof ClassType)
          {
           ClassType y = (ClassType)x;
           ClassType [] z;
           boolean done = false;
           String st = null;

           if (y.extend != null && y.extend.name != null && y.extend.scope == null)
            { // look for superclass
             st = y.extend.name.string;
             for(a = y.unresolved.iterator(); a.hasNext();)
              {
               String s = (String)a.next();
               if (s.endsWith('.' + st))
                {
                 st = s;
                 break;
                }
              }

             z = Find.findClass(st, 0, y.scope);
             
             for(int k = 0; k < z.length; k++)
              if (z[k].scope.javaPath("").endsWith(st) || z[k].scope.buildPath("").endsWith(st))
               {
                y.extend = z[k];
                garbage.add(st);
                done = true;
               }
            }

           for(int k = 0; k < y.implement.length; k++)
            if (y.implement[k].name != null && y.implement[k].scope == null)
             { // look for interface
              st = y.implement[k].name.string;
              for(a = y.unresolved.iterator(); a.hasNext();)
               {
                String s = (String)a.next();
                if (s.endsWith('.' + st))
                 {
                  st = s;
                  break;
                 }
               }
              done = false;
              
              z = Find.findClass(st, 0, y.scope);
              
              for(int l = 0; l < z.length && !done; l++)
               if (z[l].scope.javaPath("").endsWith(st) || z[l].scope.buildPath("").endsWith(st))
                {
                 y.implement[k] = z[l];
                 garbage.add(st);
                 done = true;
                 break;
                }
             }

           a = null;
           while(garbage.size() > 0)
            {
             st = (String)garbage.get(0);
             garbage.remove(0);
             y.unresolved.remove(st);
            }
          }
        }
     }
   }

   /**
    * ident { '.' ident }
    * @return String
    */
   private String qualident()
   {
    String ident = nextToken.string;
    matchKeyword(Keyword.IDENTSY);

    while(nextSymbol == Keyword.DOTSY)
     {
      lookAhead();
      ident += "." + nextToken.string;
      matchKeyword(Keyword.IDENTSY);
     }

    return ident;
   }

   /**
    * "import" ident qualifiedImport ';'
    * @return String
    */
   private String importDeclaration()
    {
     String handle;

     matchKeyword(Keyword.IMPORTSY);
     handle = nextToken.string;
     matchKeyword(Keyword.IDENTSY);
     handle = qualifiedImport(handle);
     matchKeyword(Keyword.SEMICOLONSY);

     return handle;
    }

    /**
     * { '.' ident } [ ".*" ]
     * @param handle String
     * @return String
     */
    private String qualifiedImport(String handle)
    {
     while(nextSymbol == Keyword.DOTSY)
      {
       handle += ".";
       lookAhead();
       if (nextSymbol == Keyword.MULTIPLYSY)
        {
         lookAhead();
         break;
        }
       handle += nextToken.string;
       if (!matchKeyword(Keyword.IDENTSY))
        break;
      }
     return handle;
    }

    /**
     * ';' | classOrInterfaceDeclaration
     * @param scope Scope
     * @param basename String
     */
    private void typeDeclaration(Scope scope, String basename)
   {
    if (nextSymbol == Keyword.SEMICOLONSY)
     {
      lookAhead();
      return;
     }

    classOrInterfaceDeclaration(new Modifier(), scope, basename);
   }

   /**
    * [ modifiers ] ( classDeclaration | interfaceDeclaration )
    * @param modifier Modifier
    * @param scope Scope
    * @param basename String
    */
   private void classOrInterfaceDeclaration(Modifier modifier, Scope scope, String basename)
   {
    for(int x; (x = classModifier()) != 0; modifier.add(x));

    if (nextSymbol == Keyword.CLASSSY)
     classDeclaration(modifier, scope, basename);
    else if (nextSymbol == Keyword.INTERFACESY)
     interfaceDeclaration(modifier, scope, basename);
    else
     {
      error("Keyword " + Keyword.CLASSSY.string + " or " + Keyword.INTERFACESY.string + " expected, not " + nextToken.string);
      lookAhead();
     }
   }

  /**
   * ( basicType | qualident ) bracketsOpt
   * @return Type
   */
  private Type type()
   {
    Type t = new Type();

    t.ident = nextToken;
    t.type = basicType();
    if (t.type == Keyword.NONESY)
     {
      unresolved.add(t.ident.string = qualident());
     }
    else
     t.ident.string = null;

    t.dim = bracketsOpt();

    return t;
   }

   /**
    * ( NONESY | typeKW )
    * @return Keyword
    */
   private Keyword basicType()
   {
    Keyword x = Keyword.NONESY;

    if (typeKW.contains(nextSymbol))
     {
      x = nextSymbol;
      lookAhead();
     }

    return x;
   }

   /**
    * { '[' ']' }
    * @return int
    */
   private int bracketsOpt()
   {
    int x;

    for(x = 0; nextSymbol == Keyword.LBRACKETSY; x++)
     {
      lookAhead();
      unresolved.add("JavaArray");
      matchKeyword(Keyword.RBRACKETSY);
     }

    return x;
   }

   /**
    * type { ',' type }
    * @return ClassType[]
    */
   private ClassType [] typeList()
   {
    Vector x = new Vector();

    x.add(type());
    while(nextSymbol == Keyword.COMMASY)
     {
      lookAhead();
      x.add(type());
     }

    ClassType [] dummy = new ClassType[x.size()];
    for(int i = 0; i < x.size(); i++)
     {
      Type t = (Type)x.get(i);
      dummy[i] = new ClassType(t.ident.string.substring(t.ident.string.lastIndexOf('.') + 1));
      dummy[i].modify = (dummy[i].modify & ~Keyword.CLASSSY.value) | Keyword.INTERFACESY.value;
     }

    return dummy;
   }

   /**
    * [ FINALSY ] type variableDeclaratorId
    * @return Parameter
    */
   private Parameter formalParameter()
   {
    Parameter x = new Parameter();

    x.modify = 0;
    if (nextSymbol == Keyword.FINALSY)
     {
      x.modify = nextSymbol.value;
      lookAhead();
     }

    x.type = type();
    variableDeclaratorId(x);

    return x;
   }

   /**
    * qualident { ',' qualident }
    * @return String[]
    */
   private String [] qualidentList()
   {
    Vector x = new Vector();

    x.add(qualident());
    while(nextSymbol == Keyword.COMMASY)
     {
      lookAhead();
      x.add(qualident());
     }

    String [] dummy = new String[x.size()];
    for(int i = 0; i < x.size(); i++)
     dummy[i] = (String)x.get(i);

    return dummy;
   }

   /**
    * ident variableDeclaratorRest
    * @param x VariableType
    * @param scope Scope
    * @param queue Vector
    */
   private void variableDeclarator(VariableType x, Scope scope, Vector queue)
   {
    x.name = nextToken;
    matchKeyword(Keyword.IDENTSY);
    variableDeclaratorRest(x, scope, queue);
   }

   /**
    * ident bracketsOpt
    * @param x Parameter
    */
   private void variableDeclaratorId(Parameter x)
   {
    x.name = nextToken;
    matchKeyword(Keyword.IDENTSY);
    x.type.dim += bracketsOpt();
   }

   /**
    * bracketsOpt [ '=' variableIntitializer ]
    * @param x VariableType
    * @param scope Scope
    * @param queue Vector
    */
   private void variableDeclaratorRest(VariableType x, Scope scope, Vector queue)
   {
    x.type.dim += bracketsOpt();
    if (nextSymbol == Keyword.ASSIGNSY)
     {
      Operation node, root = new Operation();
      root.operator = (x.modify & Keyword.FINALSY.value) == 0?Keyword.ASSIGNSY:Keyword.FINALASSIGNSY;

      node = root.left = new Operation();

      node.operator = Keyword.LEAFSY;
      node = node.left = new Operation();
      node = node.left = new Operation();
      node = node.left = new Operation();

      node.name = x.name;
      node.type = x.type;

      lookAhead();

      root.right = variableIntitializer(x.type, x.type.dim, scope, queue);

      queue.add(root);

      root = new Operation();
      root.operator = Keyword.CLEARSY;
      queue.add(root);
     }
   }

   /**
    * arrayInitializer | expression
    * @param t Type
    * @param dim int
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation variableIntitializer(Type t, int dim, Scope scope, Vector queue)
   {
    return (nextSymbol == Keyword.LBRACESY)?arrayInitializer(t, dim, scope, queue):expression(scope, queue);
   }

   /**
    * "class" ident [ "extend" type ] [ "implement" typeList ] classBody
    * @param modifier Modifier
    * @param scope Scope
    * @param basename String
    */
   private void classDeclaration(Modifier modifier, Scope scope, String basename)
   {
    Vector queue = new Vector();
    HashSet dummy = unresolved;
    ClassType x = new ClassType();
    unresolved = x.unresolved;

    if (comment != null && comment.length() > 0)
     {
      x.comment = comment + '\n';
      resetComment();
     }

    matchKeyword(Keyword.CLASSSY);

    if (!scopeStack.contains(scope))
     scopeStack.add(scope);

    x.name = nextToken;

    //if (basename.compareTo(nextToken.string) != 0 && basename.length() > 0)
    // modifier.access &= ~Keyword.PUBLICSY.value;

    modifier.check(modifier.classes | modifier.access & (basename.compareToIgnoreCase(nextToken.string) != 0 && basename.length() > 0?~Keyword.PUBLICSY.value:-1));

    if ((modifier.cur & Keyword.ABSTRACTSY.value) == 0)
     modifier.methods &= ~Keyword.ABSTRACTSY.value;
    else
     modifier.methods |= Keyword.ABSTRACTSY.value;

    x.modify |= modifier.cur;

    matchKeyword(Keyword.IDENTSY);

    declMember(scope, x);

    x.scope = new Scope(scope, Scope.classed, x.name.string);

    if (nextSymbol == Keyword.EXTENDSSY)
     {
      lookAhead();
      Type t = type();
      x.extend = new ClassType(t.ident.string.substring(t.ident.string.lastIndexOf('.') + 1));
     }
    else if (((x.modify & Keyword.STATICSY.value) != 0 || (Scope)scopeStack.get(0) == scope) && x.name.string.compareTo("Object") != 0)
     {
      x.extend = new ClassType("Object");
      unresolved.add(x.extend.name.string);
     }
    else
     x.extend = new ClassType();

    if (nextSymbol == Keyword.IMPLEMENTSSY)
     {
      lookAhead();
      x.implement = typeList();
     }
    else
     x.implement = new ClassType[0];

    if ((modifier.cur & Keyword.ABSTRACTSY.value) != 0 && (modifier.cur & modifier.constructors) == 0)
     modifier.cur |= Keyword.PUBLICSY.value ;

    modifier.cur &= modifier.constructors;

    classBody(x, new Modifier(), "", queue);

    Iterator iter = x.scope.iterator();

    while(iter.hasNext())
     {
      Basic b = (Basic)iter.next();

      if (b instanceof MethodType)
       if ((b.modify & Keyword.STATICSY.value) != 0)
        { // remove this from static method
         MethodType m = (MethodType)b;
         m.scope.remove("§this");
         if (m.parameter.length > 0 && m.parameter[0].name.string.compareTo("§this") == 0)
          {
           Parameter [] p = new Parameter[m.parameter.length - 1];

           for(int i = 0; i < p.length; i++)
            p[i] = m.parameter[i + 1];

           m.parameter = p;
          }
        }
     }

    // add queue to constructors
    addToConstructor(x, queue);
    unresolved = dummy;

    writeList(x);
   }

   private void addToConstructor(ClassType x, Vector queue)
    {
       Basic [] b = x.scope.get(x.name.string);
       for(int i = 0; i < b.length; i++)
        if ((b[i].modify & Keyword.CONSTRUCTORSY.value) != 0)
         {
          MethodType z = (MethodType)b[i];
          Vector inter = new Vector();

          // remove §outer from static member class
          if ((x.modify & Keyword.STATICSY.value) != 0 || x.extend.name != null)
           if (z.parameter.length > 0 && z.parameter[0].name.string.compareTo("§outer") == 0)
            {
             z.scope.remove(z.parameter[0].name.string);

             Parameter [] p = new Parameter[z.parameter.length - 1];

             for(int j = 0; j < p.length; j++)
              p[j] = z.parameter[j + 1];

             z.parameter = p;
            }

          for(int j = 0; j < z.operation.size(); j++)
           {
            Operation op = (Operation)z.operation.get(j);
            inter.add(op);
            if (op.operator == Keyword.ALLOCATESY)
             for(int k = 0; k < queue.size(); k++)
              {
               op = new Operation((Operation)queue.get(k));
               inter.add(op);
              }
           }
          z.operation = inter;
         }
    }

   /**
    * '{' classBodyDeclaration '}'
    * @param object ClassType
    * @param modifier Modifier
    * @param supercall String
    * @param queue Vector
    */
   private void classBody(ClassType object, Modifier modifier, String supercall, Vector queue)
   {
    String s = "public static int _staticBlocking = 0; " +
               "public static Thread _staticThread = null; ";
    
    if ((object.modify & Keyword.ABSTRACTSY.value) == 0)
     scanner.insert(s);

    matchKeyword(Keyword.LBRACESY);

    while(nextSymbol != Keyword.RBRACESY && nextSymbol != Keyword.EOFSY)
     classBodyDeclaration(object, modifier, queue);

    // default constructor
    Basic [] bs = object.scope.get(object.name.string);
    boolean present = (object.modify & Keyword.ABSTRACTSY.value) != 0;

    for(int i = 0; i < bs.length && !present; i++)
     if (bs[i] instanceof MethodType && (bs[i].modify & Keyword.CONSTRUCTORSY.value) != 0)
      present = ((MethodType)bs[i]).parameter.length == 0;

    if (!present)
     {
      s = " public " + object.name.string + " () {" + supercall + "} } ";
      scanner.insert(s);
      matchKeyword(Keyword.RBRACESY);

      classBodyDeclaration(object, modifier, queue);
     }

    // default destructor
    if (gc.startsWith("m") &&
             object.scope.get("~destructor").length == 0 && (object.modify & Keyword.ABSTRACTSY.value) == 0)
     {
      s = "public void ~destructor (){Object obj;" +
          "#ass \"§this @ 1- 0= IF §this CELL+ @ " + Find.addOffset(CodeRCmodified.father) + " @ TO obj \" ; " +
          "JavaArray.kill(obj, 0);" +
          "#ass \"§this CELL+ @ " + Find.addOffset(CodeRCmodified.outer) + " @ TO obj\"; JavaArray.kill(obj, 0);";
      for(Iterator i = object.scope.iterator(); i.hasNext();)
       {
        Basic b = (Basic)i.next();
        if (b instanceof VariableType && (b.modify & Keyword.STATICSY.value) == 0)
         {
          VariableType v = (VariableType)b;
          if (v.type.dim > 0 || v.type.type == Keyword.NONESY)
           s += "obj = (Object)" + v.name.string + "; JavaArray.kill(obj, -1);";
         }
       }
       s += "#ass \"ENDIF §this DECREFERENCE\";} }";
       scanner.insert(s);
       matchKeyword(Keyword.RBRACESY);
       classBodyDeclaration(object, modifier, queue);
      }

    for(Iterator i = object.scope.iterator(); i.hasNext();)
    {
     Basic b = (Basic)i.next();
     if (b instanceof MethodType &&
    	 (b.modify & Keyword.STATICSY.value) != 0 &&
    	 (b.modify & Keyword.SYNCHRONIZEDSY.value) != 0)
      {
       s = " private static void classMonitorEnter() " +
       "{ " +
       "#ass \"DI \";" +
       "if (ThreadState.getState(_staticThread) == 0)" +
       "{ " +
       " _staticThread = null;" +
       " _staticBlocking = 0;" +
       "} " +

       "Thread t = Thread.currentThread();" +

       "while(t != _staticThread && _staticThread != null)" +
       " {" +
       "  ThreadState.switchTask(ThreadState.BLOCKED, " + object.hashCode() + ", 0L);" +
       " }" +

       "_staticThread = t;" +
       "_staticBlocking++;" +

       "#ass \"EI \";" +
       "} }";
       
       scanner.insert(s);
       matchKeyword(Keyword.RBRACESY);
       classBodyDeclaration(object, modifier, queue);
       
       s = "private static void classMonitorLeave() " +
       "{ " +
       "#ass \"DI \";" +

       "if (--_staticBlocking == 0)" +
       " {" +
       "  ThreadState.unblockTask(ThreadState.BLOCKED, " + object.hashCode() + ", false);" +
       "  _staticThread = null;" +
       " }" +

       "#ass \"EI \";" +
       "} }";
       
       scanner.insert(s);
       matchKeyword(Keyword.RBRACESY);
       classBodyDeclaration(object, modifier, queue);
       
       break;
      }
    }
    
    matchKeyword(Keyword.RBRACESY);
   }

   /**
    * ';' | [ STATICSY ] ( block | [ modifiers ] memberDecl )
    * @param object ClassType
    * @param modifier Modifier
    * @param queue Vector
    */
   private void classBodyDeclaration(ClassType object, Modifier modifier, Vector queue)
   {
    Modifier x = new Modifier(modifier);
    x.cur = 0;

    if (nextSymbol == Keyword.SEMICOLONSY)
     {
      lookAhead();
      return;
     }

    if (nextSymbol == Keyword.STATICSY)
     {
      x.add(modifier());
      queue = object.statics;
     }

    if (nextSymbol == Keyword.LBRACESY)
     {
      Vector label = new Vector();
      //label.add("" + Operation.newLabel());
      block(null, null, object.scope, label, Scope.MAIN, null, queue);
     }
    else
     {
      for(int y = modifier1(); y != 0; y = modifier())
       x.add(y);

      if ((x.cur & Keyword.NATIVESY.value) != 0)
       Errors.warning(nextToken.source, nextToken.line, nextToken.col, "native detected!", false);

      if ((x.cur & x.constructors) == 0)
       x.cur |= modifier.cur & modifier.constructors;

      if ((x.cur & Keyword.STATICSY.value) != 0)
       queue = object.statics;

      memberDecl(x, object, queue);
     }
   }

   /**
    * constructorDeclaratorRest | "void" voidMethodDeclaratorRest | classDeclaration | interfaceDeclaration | methodOrFieldDeclaration
    * @param modify Modifier
    * @param object ClassType
    * @param queue Vector
    */
   private void memberDecl(Modifier modify, ClassType object, Vector queue)
   {
    Type dummy = returns;

    if (identAndLPar())
     {
      if ((modify.cur & modify.constructors) == 0)
       modify.cur |= Keyword.PUBLICSY.value ;

      modify.check(modify.constructors | modify.methods);
      MethodType x = member = new MethodType(modify.cur | Keyword.CONSTRUCTORSY.value);

      x.comment = comment + '\n';
      resetComment();

      x.type = new Type();
      x.type.type = Keyword.NONESY;
      x.type.ident = object.name;
      x.type.version = object.version;
      returns = x.type;
      x.name = nextToken;
      matchKeyword(Keyword.IDENTSY);
      
      if (x.name.string.compareTo(object.name.string) != 0)
       error("missing type of method");
      
      constructorDeclaratorRest(x, object);
      member = null;
     }
    else if (nextSymbol == Keyword.VOIDSY)
     {
      modify.check(modify.methods | modify.access);
      MethodType x = member = new MethodType(modify.cur);

      x.comment = comment + '\n';
      resetComment();

      x.type = new Type();
      x.type.type = Keyword.VOIDSY;
      returns = x.type;
      lookAhead();
      if (nextSymbol == Keyword.INVERTSY)
       { // allow prefix ~ for destructor method
        lookAhead();
        nextToken.string = '~' + nextToken.string;
       }
      x.name = nextToken;
      matchKeyword(Keyword.IDENTSY);
      voidMethodDeclaratorRest(modify, x, object);
      member = null;
     }
    else if (nextSymbol == Keyword.CLASSSY)
     {
      modify.check(modify.classes | modify.access);
      classDeclaration(modify, object.scope, "");
     }
    else if (nextSymbol == Keyword.INTERFACESY)
     {
      modify.check(modify.interfaces | modify.access);
      interfaceDeclaration(modify, object.scope, "");
     }
    else
     methodOrFieldDeclaration(modify, object, queue);

    returns = dummy;
   }

   /**
    * ident methodOrFieldRest
    * @param modify Modifier
    * @param object ClassType
    * @param queue Vector
    */
   private void methodOrFieldDeclaration(Modifier modify, ClassType object, Vector queue)
   {
    Type t = type();
    Token u = nextToken;
    matchKeyword(Keyword.IDENTSY);
    methodOrFieldRest(modify, t, u, object, queue);
   }

   /**
    * methodDeclaratorRest | variableDeclaratorsRest
    * @param modify Modifier
    * @param t Type
    * @param name Token
    * @param object ClassType
    * @param queue Vector
    */
   private void methodOrFieldRest(Modifier modify, Type t, Token name, ClassType object, Vector queue)
   {
    Modifier m = new Modifier(modify);

    if (nextSymbol == Keyword.LBRACKETSY || nextSymbol == Keyword.ASSIGNSY || nextSymbol == Keyword.SEMICOLONSY || nextSymbol == Keyword.COMMASY)
     {
      if ((object.modify & Keyword.ABSTRACTSY.value) != 0)
       {
        // implicit public
        m.cur &= ~Keyword.ABSTRACTSY.value;
        if ((m.cur & m.access) == 0)
         m.cur |= Keyword.PUBLICSY.value ;
       }

      if (object.scope.prev.prev != null)
       if ((modify.classes & Keyword.STATICSY.value) == 0 && (m.cur & Keyword.STATICSY.value) != 0 && (m.cur & Keyword.FINALSY.value) == 0)
        error("must be final!");

      variableDeclaratorsRest(m, t, name, object.scope, queue);
      matchKeyword(Keyword.SEMICOLONSY);
     }
    else
     methodDeclaratorRest(m, t, name, object);
   }

   /**
    * variableDeclaratorRest { ',' variableDeclarator }
    * @param modify Modifier
    * @param t Type
    * @param name Token
    * @param scope Scope
    * @param queue Vector
    */
   private void variableDeclaratorsRest(Modifier modify, Type t, Token name, Scope scope, Vector queue)
   {
    if (comment != null && comment.length() > 0)
     {
      Operation op = new Operation();
      op.operator = Keyword.COMMENTSY;
      op.code = comment + '\n';
      queue.add(op);
      resetComment();
     }

    modify.check(modify.fields | modify.access);

    VariableType x = new VariableType(new Type(t), modify.cur);
    x.name = name;
    variableDeclaratorRest(x, scope, queue);
    scope.declInsertOverload(x);
    follower.add(Keyword.COMMASY);

    while(nextSymbol == Keyword.COMMASY)
     {
      lookAhead();
      x = new VariableType(new Type(t), modify.cur);

      variableDeclarator(x, scope, queue);
      scope.declInsertOverload(x);
     }

    follower.remove(follower.size() - 1);
   }

   /**
    * '{' [ variableIntitializer { ',' variableIntitializer } ] [ ',' ] '}'
    * @param t Type
    * @param dim int
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation arrayInitializer(Type t, int dim, Scope scope, Vector queue)
   {
    Operation base = new Operation(), next;
    int count = 0;

    base.operator = nextSymbol;
    base.type = new Type(t);

    unresolved.add("JavaArray");
    matchKeyword(Keyword.LBRACESY);
    follower.add(Keyword.RBRACESY);

    if (nextSymbol != Keyword.COMMASY && nextSymbol != Keyword.RBRACESY)
     {
      dim--;
      follower.add(Keyword.COMMASY);
      next = (base.right = new Operation());
      next.left = variableIntitializer(t, dim, scope, queue);
      count++;

      while(commaAndNoRBrace())
       {
        next = (next.right = new Operation());
        matchKeyword(Keyword.COMMASY);

        next.left = variableIntitializer(t, dim, scope, queue);
        count++;
       }

      follower.remove(follower.size() - 1);
     }

    follower.remove(follower.size() - 1);

    if (nextSymbol == Keyword.COMMASY)
     lookAhead();

    Operation lit, root = new Operation();
    root.operator = Keyword.LEAFSY;
    root.left = new Operation();
    lit = root.left.left = new Operation();
    lit.operator = Keyword.LITERALSY;
    lit.type = new Type();
    lit.type.type = Keyword.INTSY;
    lit.code = count + " ";
    base.left = root;

    matchKeyword(Keyword.RBRACESY);

    return base;
   }

   /**
    * formalParameters bracketsOpt [ "throws" qualidentList ] ( ';' | block )
    * @param modify Modifier
    * @param t Type
    * @param name Token
    * @param object ClassType
    */
   private void methodDeclaratorRest(Modifier modify, Type t, Token name, ClassType object)
   {
    modify.check(modify.methods | modify.access);
    MethodType x = member = new MethodType(modify.cur);

    x.comment = comment + '\n';
    resetComment();

    x.type = t;
    returns = x.type;
    x.name = name;
    x.parameter = formalParameters();
    x.type.dim += bracketsOpt();

    if (nextSymbol == Keyword.THROWSSY)
     {
      lookAhead();
      x.throwing = qualidentList();
     }
    else
     x.throwing = new String[0];

    if (nextSymbol == Keyword.SEMICOLONSY)
     {
      if ((modify.cur & Keyword.NATIVESY.value) == 0)
       {
        if ( (object.modify & Keyword.ABSTRACTSY.value) == 0)
         error("missing modifier abstract");

        if ( (modify.cur & modify.access) == 0)
         modify.add(Keyword.PUBLICSY.value);
       }

      x.scope = new Scope(object.scope, Scope.automatic, "");
      insertThis(object, x);
      lookAhead();
     }
    else
     {
      if ((object.modify & Keyword.ABSTRACTSY.value) == 0)
       {
        if ((x.modify & new Modifier().constructors) == 0)
         x.modify |= Keyword.PUBLICSY.value ;
       }

      Vector label = new Vector();
      label.add(Operation.newLabel() + "");
      x.scope = block(x, null, object.scope, label, Scope.MAIN, null, x.operation);
     }

    object.scope.declInsertOverload(x);
    
    member = null;
   }

   /**
    * formalParameters [ "throws" qualidentList ] ( ';' | block )
    * @param modify Modifier
    * @param x MethodType
    * @param object ClassType
    */
   private void voidMethodDeclaratorRest(Modifier modify, MethodType x, ClassType object)
   {
    Scope scope = object.scope;

    x.parameter = formalParameters();
    if (nextSymbol == Keyword.THROWSSY)
     {
      lookAhead();
      x.throwing = qualidentList();
     }
    else
     x.throwing = new String[0];

    if (nextSymbol == Keyword.SEMICOLONSY)
     {
      if ((modify.cur & Keyword.NATIVESY.value) == 0)
       {
        if ( (x.modify & Keyword.ABSTRACTSY.value) == 0)
         error("missing modifier abstract");

        if ( (modify.cur & modify.access) == 0)
         modify.add(Keyword.PUBLICSY.value);
       }

      x.scope = new Scope(scope, Scope.automatic, "");
      insertThis(object, x);
      lookAhead();
     }
    else
     {
      if ((x.modify & Keyword.ABSTRACTSY.value) == 0)
       {
        if ((x.modify & new Modifier().constructors) == 0)
         x.modify |= Keyword.PUBLICSY.value ;
       }

      Vector label = new Vector();
      label.add(Operation.newLabel() + "");
      x.scope = block(x, null, scope, label, Scope.MAIN, null, x.operation);
     }

    scope.declInsertOverload(x);
   }

   /**
    * formalParameters [ "throws" qualidentList ] block
    * @param x MethodType
    * @param object ClassType
    */
   private void constructorDeclaratorRest(MethodType x, ClassType object)
   {
    Scope scope = object.scope;

    x.parameter = formalParameters();
    if (nextSymbol == Keyword.THROWSSY)
     {
      lookAhead();
      x.throwing = qualidentList();
     }
    else
     x.throwing = new String[0];

    Vector label = new Vector();
    label.add(Operation.newLabel() + "");
    x.scope = block(x, null, scope, label, Scope.MAIN, null, x.operation);

    scope.declInsertOverload(x);
   }

   /**
    * '(' [ formalParameter { ',' formalParameter } ] ')'
    * @return Parameter[]
    */
   private Parameter [] formalParameters()
   {
    Vector x = new Vector();

    matchKeyword(Keyword.LPARSY);

    if (nextSymbol != Keyword.RPARSY)
     {
      x.add(formalParameter());
      while(nextSymbol == Keyword.COMMASY)
       {
        lookAhead();
        x.add(formalParameter());
       }
     }

    matchKeyword(Keyword.RPARSY);

    Parameter [] dummy = new Parameter[Math.max(0, x.size())];
    for(int i = 0; i < x.size(); i++)
     dummy[i] = (Parameter)x.get(i);

    return dummy;
   }

   /**
    * "interface" ident [ "extends" typeList ] interfaceBody
    * @param modify Modifier
    * @param scope Scope
    * @param basename String
    */
   private void interfaceDeclaration(Modifier modify, Scope scope, String basename)
   {
    HashSet dummy = unresolved;
    ClassType x = new ClassType();
    x.modify = (x.modify & ~Keyword.CLASSSY.value) | Keyword.INTERFACESY.value;
    unresolved = x.unresolved;

    if (comment != null && comment.length() > 0)
     {
      x.comment = comment + '\n';
      resetComment();
     }

    if (!scopeStack.contains(scope))
     scopeStack.add(scope);

    matchKeyword(Keyword.INTERFACESY);

    x.name = nextToken;

    //if (basename.compareTo(nextToken.string) != 0 && basename.length() > 0)
    // modify.access &= ~Keyword.PUBLICSY.value;

    modify.check(modify.interfaces | modify.access & (basename.compareToIgnoreCase(nextToken.string) != 0 && basename.length() > 0?~Keyword.PUBLICSY.value:-1));

    matchKeyword(Keyword.IDENTSY);

    declMember(scope, x);

    if (nextSymbol == Keyword.EXTENDSSY)
     {
      lookAhead();
      x.implement = typeList();
     }
    else
     x.implement = new ClassType[0];

    x.extend = new ClassType("Object");//!!!!!!!!!!!!!!!!!!!
    x.scope = new Scope(scope, Scope.heap, x.name.string);

    if ((modify.cur & modify.constructors) == 0)
     modify.cur |= Keyword.PUBLICSY.value & modify.access;

    interfaceBody(modify, x);

    unresolved = dummy;
    writeList(x);
   }

   /**
    * '{' interfaceBodyDeclaration '}'
    * @param modify Modifier
    * @param object ClassType
    */
   private void interfaceBody(Modifier modify, ClassType object)
   {
    // implicite default destructor
	if (!gc.startsWith("t"))
     scanner.insert("public void ~destructor ();");
    matchKeyword(Keyword.LBRACESY);

    while(nextSymbol != Keyword.RBRACESY && nextSymbol != Keyword.EOFSY)
     interfaceBodyDeclaration(modify, object);

    matchKeyword(Keyword.RBRACESY);
   }

   /**
    * ';' | [ modifiers ] interfaceMemberDecl
    * @param modify Modifier
    * @param object ClassType
    */
   private void interfaceBodyDeclaration(Modifier modify, ClassType object)
   {
    if (nextSymbol == Keyword.SEMICOLONSY)
     {
      lookAhead();
      return;
     }

    Modifier m = new Modifier(modify);
    m.cur = 0;

    for(int x; (x = modifier()) != 0; m.add(x));

    if ((m.cur & m.constructors) == 0)
     m.cur |= modify.cur & modify.constructors;

    if ((m.cur & Keyword.NATIVESY.value) != 0)
     Errors.warning(nextToken.source, nextToken.line, nextToken.col, "native detected!", false);

    interfaceMemberDecl(m, object);
   }

   /**
    * "void" voidInterfaceMethodDeclaratorRest | classDeclaration | interfaceDeclaration | interfaceMethodOrFieldDeclaration
    * @param modify Modifier
    * @param object ClassType
    */
   private void interfaceMemberDecl(Modifier modify, ClassType object)
   {
    if (comment != null && comment.length() > 0)
     {
      Operation op = new Operation();
      op.code = comment + '\n';
      op.operator = Keyword.COMMENTSY;
      object.statics.add(op);
      resetComment();
     }

    if (nextSymbol == Keyword.VOIDSY)
     {
      modify.check(modify.methods | modify.access);
      MethodType x = new MethodType(modify.cur);

      x.type = new Type();
      x.type.type = Keyword.VOIDSY;
      x.scope = new Scope(object.scope, Scope.automatic, "");

      lookAhead();
      if (nextSymbol == Keyword.INVERTSY)
       { // allow prefix ~ for destructor method
        lookAhead();
        nextToken.string = '~' + nextToken.string;
       }
      x.name = nextToken;
      matchKeyword(Keyword.IDENTSY);
      voidInterfaceMethodDeclaratorRest(x);
      insertThis(object, x);
      object.scope.declInsertOverload(x);
     }
    else if (nextSymbol == Keyword.CLASSSY)
     {
      modify.check(modify.classes | modify.access);
      Modifier m = new Modifier(modify);
      m.cur = modify.cur | Keyword.STATICSY.value;
      classDeclaration(m, object.scope, "");
     }
    else if (nextSymbol == Keyword.INTERFACESY)
     {
      modify.check(modify.interfaces | modify.access);
      interfaceDeclaration(modify, object.scope, "");
     }
    else
     interfaceMethodOrFieldDeclaration(modify, object);
   }

   /**
    * ident interfaceMethodOrFieldRest
    * @param modify Modifier
    * @param object ClassType
    */
   private void interfaceMethodOrFieldDeclaration(Modifier modify, ClassType object)
   {
    Token dummy;

    Type t = type();
    dummy = nextToken;
    matchKeyword(Keyword.IDENTSY);

    interfaceMethodOrFieldRest(new Modifier(modify), t, dummy, object);
   }

   /**
    * constantDeclaratorsRest | interfaceMethodDeclaratorRest
    * @param modify Modifier
    * @param t Type
    * @param ident Token
    * @param object ClassType
    */
   private void interfaceMethodOrFieldRest(Modifier modify, Type t, Token ident, ClassType object)
   {
    // implicit public
    if ((modify.cur & modify.access) == 0)
     modify.add(Keyword.PUBLICSY.value);

   if (nextSymbol == Keyword.LBRACKETSY || nextSymbol == Keyword.ASSIGNSY)
     {
      // implicit static
      modify.cur |= Keyword.STATICSY.value ;

      constantDeclaratorsRest(modify, t, ident, object.scope, object.statics);
      matchKeyword(Keyword.SEMICOLONSY);
     }
    else
     {
      // implicit abstract
      modify.cur |= Keyword.ABSTRACTSY.value ;

      interfaceMethodDeclaratorRest(modify, t, ident, object);
     }
   }

   /**
    * constantDeclaratorRest { ',' constantDeclarator }
    * @param modify Modifier
    * @param t Type
    * @param ident Token
    * @param scope Scope
    * @param queue Vector
    */
   private void constantDeclaratorsRest(Modifier modify, Type t, Token ident, Scope scope, Vector queue)
   {
    modify.check(modify.constants | modify.access);
    VariableType x = new VariableType(new Type(t), modify.cur | Keyword.FINALSY.value);
    x.name = ident;
    declMember(scope, x);
    constantDeclaratorRest(x, scope, queue);
    while(nextSymbol == Keyword.COMMASY)
     {
      lookAhead();
      x = new VariableType(new Type(t), modify.cur | Keyword.FINALSY.value);
      constantDeclarator(x, scope, queue);
     }
   }

   /**
    * bracketsOpt '=' variableIntitializer
    * @param x VariableType
    * @param scope Scope
    * @param queue Vector
    */
   private void constantDeclaratorRest(VariableType x, Scope scope, Vector queue)
   {
    if ((x.modify & Keyword.FINALSY.value) == 0)
     error("missing keyword final");

    x.type.dim += bracketsOpt();

    Operation node, root = new Operation();
    root.operator = Keyword.FINALASSIGNSY;

    matchKeyword(Keyword.ASSIGNSY);

    node = root.left = new Operation();
    node.operator = Keyword.LEAFSY;
    node = node.left = new Operation();
    node = node.left = new Operation();
    node = node.left = new Operation();

    node.name = x.name;
    node.type = x.type;

    root.right = variableIntitializer(x.type, x.type.dim, scope, queue);

    queue.add(root);

    root = new Operation();
    root.operator = Keyword.CLEARSY;
    queue.add(root);
   }

   /**
    * ident constantDeclaratorRest
    * @param x VariableType
    * @param scope Scope
    * @param queue Vector
    */
   private void constantDeclarator(VariableType x, Scope scope, Vector queue)
   {
    x.name = nextToken;
    declMember(scope, x);
    matchKeyword(Keyword.IDENTSY);
    constantDeclaratorRest(x, scope, queue);
   }

   /**
    * formalParameters bracketsOpt [ "throws" qualidentList ] ';'
    * @param modify Modifier
    * @param t Type
    * @param ident Token
    * @param object ClassType
    */
   private void interfaceMethodDeclaratorRest(Modifier modify, Type t, Token ident, ClassType object)
   {
    modify.check(modify.methods | modify.access);
    MethodType x = new MethodType(modify.cur);

    x.name = ident;
    x.type = t;
    x.scope = new Scope(object.scope, Scope.automatic, "");
    x.parameter = formalParameters();
    x.type.dim += bracketsOpt();
    object.scope.declInsertOverload(x);

    if (nextSymbol == Keyword.THROWSSY)
     {
      lookAhead();
      x.throwing = qualidentList();
     }
    else
     x.throwing = new String[0];

    insertThis(object, x);
    matchKeyword(Keyword.SEMICOLONSY);
   }

   /**
    * [ "throws" qualidentList ] ';'
    * @param x MethodType
    */
   private void voidInterfaceMethodDeclaratorRest(MethodType x)
   {
    x.parameter = formalParameters();
    if (nextSymbol == Keyword.THROWSSY)
     {
      lookAhead();
      x.throwing = qualidentList();
     }
    else
     x.throwing = new String[0];

    matchKeyword(Keyword.SEMICOLONSY);
   }

   /**
    * insert leading parameter this in parameter list
    * @param object ClassType
    * @param x MethodType
    */
   private void insertThis(ClassType object, MethodType x)
   {
    if ((x.modify & Keyword.STATICSY.value) == 0)
     {
      Parameter p = new Parameter();
      p.name = new Token();
      p.name.string = "§this";
      p.type = new Type();
      p.type.version = object.version;
      p.type.ident.string = object.name.string;
      p.type.type = Keyword.NONESY;

      Parameter [] q = new Parameter[x.parameter.length + 1];

      q[0] = p;
      for(int i = 1; i < q.length; i++)
       q[i] = x.parameter[i - 1];

      x.parameter = q;
     }

    if (x.throwing.length > 0)
     {
      Parameter p = new Parameter();
      p.name = new Token();
      p.name.string = "§exception";
      p.name.kind = Keyword.IDENTSY;
      p.type = new Type();
      p.type.type = Keyword.VOIDSY;

      Parameter[] q = new Parameter[x.parameter.length + 1];
      for (int i = 0; i < x.parameter.length; i++)
       q[i] = x.parameter[i];

      q[q.length - 1] = p;
      x.parameter = q;
     }
   }

   /**
    * { ident ':' } ( Block
    * | "assert" Expression [ ':' Expression ] ';'
    * | "if" ParExpression Statement ["else" Statement]
    * | "for" '(' [ForInit] ';' [Expression] ';' [ForUpdate] ')' Statement
    * | "while" ParExpression Statement
    * | "do" Statement "while" ParExpression ';'
    * | "try" Block (Catches ["finally" Block] | "finally" Block)
    * | "switch" ParExpression '{' SwitchBlockStatementGroups '}'
    * | "synchronized" ParExpression Block
    * | "return" [Expression] ';'
    * | "throw" Expression ';'
    * | "break" [ident] ';'
    * | "continue" [ident] ';'
    * | ';'
    * | StatementExpression ';'
    * | "#ass" StringLiteral { '+' StringLiteral } ';' )
    * @param modify Modifier
    * @param scope Scope
    * @param block int
    * @param continues Operation
    * @param queue Vector
    * @param clear boolean
    */
   private void statement(Modifier modify, Scope scope, int block, Operation continues, Vector queue, boolean clear)
   {
    Operation ptr;
    Vector newLabel = new Vector();

    while(isLabel())
     {
      String x = nextToken.string;
      matchKeyword(Keyword.IDENTSY);
      matchKeyword(Keyword.COLONSY);
      newLabel.add(x);
      scope.label.add(x);
     }

    if (nextSymbol == Keyword.ASSERTSY)
     { // recognized, but not implemented!
      lookAhead();
      follower.add(Keyword.COLONSY);
      expression(scope, queue);
      follower.remove(follower.size() - 1);
      if (nextSymbol == Keyword.COLONSY)
       {
        lookAhead();
        expression(scope, queue);
       }
      matchKeyword(Keyword.SEMICOLONSY);
     }
    else if (nextSymbol == Keyword.LBRACESY)
     {
      if (block != Scope.LOOP)
       {
        if (newLabel.size() == 0)
         block = Scope.SEQUENCE;
        else
         block = Scope.BLOCK;

        continues = null;
       }

      Vector label = new Vector();
      label.add(Operation.newLabel() + "");

      block(null, null, scope, label, block, continues, queue);
     }
    else if (nextSymbol == Keyword.IFSY)
     {
      int size = queue.size();
      lookAhead();

      Operation clause = parExpression(scope, queue);
      boolean skip = clause != null && clause.operator == Keyword.LITERALSY && clause.code.charAt(0) == 'T';

      if (clause == null || clause.operator != Keyword.LITERALSY)
       {
        ptr = new Operation();
        ptr.code = "IF\n";
        queue.add(ptr);
       }

      statement(modify, scope, Scope.SEQUENCE, null, queue, true);

      if (clause != null && clause.operator == Keyword.LITERALSY)
       if (!skip)
        queue.setSize(size);
       else
    	queue.remove(size);
      
      if (nextSymbol == Keyword.ELSESY)
       {
        lookAhead();

        if (clause == null || clause.operator != Keyword.LITERALSY)
         {
          ptr = new Operation();
          ptr.code = "ELSE\n";
          queue.add(ptr);
         }

        size = queue.size();
        statement(modify, scope, Scope.SEQUENCE, null, queue, true);

        if (clause != null && skip)
         queue.setSize(size);
       }

      if (clause == null || clause.operator != Keyword.LITERALSY)
       {
        ptr = new Operation();
        ptr.code = "ENDIF\n";
        queue.add(ptr);
       }
     }
    else if (nextSymbol == Keyword.FORSY)
     {
      boolean brace;
      int size = queue.size();
      
      Vector label = new Vector();
      label.add(Operation.newLabel() + "");
      Scope inter = new Scope(scope, Scope.automatic, "", label, Scope.LOOP);

      ptr = new Operation();
      ptr.operator = Keyword.PUSHSY;
      ptr.scope = inter;
      queue.add(ptr);

      lookAhead();
      matchKeyword(Keyword.LPARSY);

      if (nextSymbol != Keyword.SEMICOLONSY)
       forInit(modify, inter, queue);

      ptr = new Operation();
      ptr.code = "BEGIN\n";
      queue.add(ptr);

      matchKeyword(Keyword.SEMICOLONSY);

      Type t = new Type();
      t.type = Keyword.BOOLEANSY;

      boolean skip = false, always = false;

      if (nextSymbol != Keyword.SEMICOLONSY)
       {
        ptr = castExpression(inter, t, true, queue);
        
        ptr = ptr.left.left.left;
        if (ptr.operator == Keyword.LPARSY &&
        	ptr.left.operator == Keyword.LEAFSY)
         ptr = ptr.left.left.left.left;
        skip = ptr != null && ptr.operator == Keyword.LITERALSY && ptr.code.charAt(0) == 'F';
        always = ptr != null && ptr.operator == Keyword.LITERALSY && ptr.code.charAt(0) == 'T';
        if (always)
         queue.remove(queue.size() - 1);
       }
      else
       always = true;

      if (!always)
       {
        ptr = new Operation();
        ptr.code = "WHILE\n";
        queue.add(ptr);
       }

      matchKeyword(Keyword.SEMICOLONSY);
      Operation z = (nextSymbol != Keyword.RPARSY)?forUpdate(inter, queue):null;

      matchKeyword(Keyword.RPARSY);

      continues = new Operation();
      brace = nextSymbol == Keyword.LBRACESY;
      statement(modify, inter, Scope.LOOP, continues, queue, true);

      if (!brace)
       {
        // add continue label
        ptr = insertLabels("§continue", label);
        queue.add(ptr);
        queue.add(continues);
       }

      int i = queue.indexOf(continues);
      if (i < 0)
       i = queue.size() - 1;
      
      while(z != null)
       { // insert update statements
        if (z.left != null)
         queue.add(++i, z.left);
        z = z.right;

        ptr = new Operation();
        ptr.operator = Keyword.CLEARSY;
        queue.add(++i, ptr);
       }

      ptr = new Operation();
      ptr.code = always?"AGAIN\n":"REPEAT\n";
      queue.add(++i, ptr);

      queue.remove(continues);

      if (!brace)
       {
        // add break labels
        ptr = insertLabels("§break", label);
        queue.add(ptr);

        // deallocate locals
        ptr = new Operation();
        ptr.operator = Keyword.POPSY;
        queue.add(ptr);
       }

      ptr = blockEnd(scope);
      queue.add(ptr);
      
      if (skip)
       queue.setSize(size);
     }
    else if (nextSymbol == Keyword.WHILESY)
     {
      boolean brace;
      int size = queue.size();
      
      Vector label = new Vector();
      label.add(Operation.newLabel() + "");
      Scope inter = new Scope(scope, Scope.automatic, "", label, Scope.LOOP);

      ptr = new Operation();
      ptr.operator = Keyword.PUSHSY;
      ptr.scope = inter;
      queue.add(ptr);

      lookAhead();

      ptr = new Operation();
      ptr.code = "BEGIN\n";
      queue.add(ptr);

      Operation clause = parExpression(scope, queue);
      
      if (clause != null && clause.operator == Keyword.LITERALSY &&
    	  clause.code.charAt(0) == 'T')
       queue.remove(queue.size() - 1);
      else
       {
        ptr = new Operation();
        ptr.code = "WHILE\n";
        queue.add(ptr);
       }

      continues = new Operation();
      if (clause != null && clause.operator == Keyword.LITERALSY &&
          clause.code.charAt(0) == 'T')
       continues.code = "AGAIN\n";
      else
       continues.code = "REPEAT\n";
      brace = nextSymbol == Keyword.LBRACESY;
      statement(modify, inter, Scope.LOOP, continues, queue, true);

      if (!brace)
       {
        // add continue label
        ptr = insertLabels("§continue", label);
        queue.add(ptr);
        queue.add(continues);

        // add break labels
        ptr = insertLabels("§break", label);
        queue.add(ptr);

        // deallocate locals
        ptr = new Operation();
        ptr.operator = Keyword.POPSY;
        queue.add(ptr);
       }

      ptr = blockEnd(scope);
      queue.add(ptr);
      
      if (clause != null && clause.operator == Keyword.LITERALSY &&
          clause.code.charAt(0) == 'F')
       queue.setSize(size);
     }
    else if (nextSymbol == Keyword.DOSY)
     {
      boolean brace;
      
      Vector label = new Vector();
      label.add(Operation.newLabel() + "");
      Scope inter = new Scope(scope, Scope.automatic, "", label, Scope.LOOP);

      ptr = new Operation();
      ptr.operator = Keyword.PUSHSY;
      ptr.scope = inter;
      queue.add(ptr);

      lookAhead();

      ptr = new Operation();
      ptr.code = "BEGIN\n";
      queue.add(ptr);

      continues = new Operation();
      brace = nextSymbol == Keyword.LBRACESY;
      statement(modify, inter, Scope.LOOP, continues, queue, true);

      if (!brace)
       {
        // add continue label
        ptr = insertLabels("§continue", label);
        queue.add(ptr);
        queue.add(continues);
       }

      matchKeyword(Keyword.WHILESY);

      Vector help = new Vector();
      int i = queue.indexOf(continues);
      if (i++ >= 0)
       while(i < queue.size())
        {
         help.add(queue.get(i));
         queue.remove(i);
        }

      parExpression(scope, queue);

      ptr = new Operation();
      ptr.code = "0= UNTIL\n";
      queue.add(ptr);

      queue.remove(continues);

      for(i = 0; i < help.size(); i++)
       queue.add(help.get(i));

      if (!brace)
       {
        // add break labels
        ptr = insertLabels("§break", label);
        queue.add(ptr);

        // deallocate locals
        ptr = new Operation();
        ptr.operator = Keyword.POPSY;
        queue.add(ptr);
       }

      ptr = blockEnd(scope);
      queue.add(ptr);

      matchKeyword(Keyword.SEMICOLONSY);
     }
    else if (nextSymbol == Keyword.TRYSY)
     {
      exceptionStack++;
      lookAhead();

      Vector label = new Vector();
      label.add(Operation.newLabel() + "");

      block(null, null, scope, label, Scope.TRY, null, queue);
      exceptionStack--;

      String _label = "§out" + Operation.newLabel();

      if (nextSymbol != Keyword.FINALLYSY)
       catches(_label, scope, queue);

      // remove null from data stack
      ptr = new Operation();
      ptr.code = _label + " LABEL DROP ";
      queue.add(ptr);

      if (nextSymbol == Keyword.FINALLYSY)
       {
        label.clear();
        label.add(Operation.newLabel() + "");

        lookAhead();
        block(null, null, scope, label, Scope.FINALLY, null, queue);
       }
     }
    else if (nextSymbol == Keyword.SWITCHSY)
     {
      lookAhead();
      Vector label = new Vector();
      label.add(Operation.newLabel() + "");

      Scope inter = new Scope(scope, Scope.automatic, "", label, Scope.SWITCH);

      ptr = new Operation();
      ptr.operator = Keyword.PUSHSY;
      ptr.scope = inter;
      queue.add(ptr);

      Type t = new Type();
      t.type = Keyword.INTSY;
      matchKeyword(Keyword.LPARSY);
      follower.add(Keyword.RPARSY);
      castExpression(scope, t, true, queue);
      follower.remove(follower.size() - 1);
      matchKeyword(Keyword.RPARSY);

      matchKeyword(Keyword.LBRACESY);

      switchBlockStatementGroups(inter, queue);

      matchKeyword(Keyword.RBRACESY);

      ptr = insertLabels("§break", label);
      queue.add(ptr);
      ptr = new Operation();
      ptr.operator = Keyword.POPSY;
      queue.add(ptr);

      // jump to next block
      ptr = blockEnd(scope);
      queue.add(ptr);
     }
    else if (nextSymbol == Keyword.SYNCHRONIZEDSY)
     {
      lookAhead();

      Vector label = new Vector();
      label.add("" + Operation.newLabel());

      // currently only interrupts will be blocked
      matchKeyword(Keyword.LPARSY);
      ptr = new Operation();
      ptr.operator = Keyword.LOCKSY;
      ptr.left = expression(scope, queue);
      matchKeyword(Keyword.RPARSY);

      block(null, null, scope, label, Scope.SYNCHRONIZED, ptr, queue);
     }
    else if (nextSymbol == Keyword.RETURNSY)
     {
      ptr = new Operation();
      ptr.operator = nextSymbol;
      lookAhead();

      if (nextSymbol != Keyword.SEMICOLONSY)
       {
        ptr.left = expression(scope, queue);
       }
      queue.add(ptr);

      matchKeyword(Keyword.SEMICOLONSY);
     }
    else if (nextSymbol == Keyword.THROWSY)
     {
      ptr = new Operation();
      ptr.operator = nextSymbol;
      lookAhead();

      ptr.left = expression(scope, queue);

      queue.add(ptr);

      ptr = new Operation();
      ptr.operator = Keyword.EXITSY;
      queue.add(ptr);

      matchKeyword(Keyword.SEMICOLONSY);
     }
    else if (nextSymbol == Keyword.BREAKSY || nextSymbol == Keyword.CONTINUESY)
     {
      ptr = new Operation();
      ptr.operator = nextSymbol;
      lookAhead();

      if (nextSymbol == Keyword.IDENTSY)
       {
        ptr.name = nextToken;
        lookAhead();
       }

      matchKeyword(Keyword.SEMICOLONSY);
      queue.add(ptr);
     }
    else if (nextSymbol == Keyword.SEMICOLONSY)
     {
      lookAhead();
     }
    else if (nextSymbol == Keyword.FORTHSY)
     {
      ptr = new Operation();

      ptr.type.type = Keyword.VOIDSY;

      matchKeyword(Keyword.FORTHSY);
      ptr.code = nextToken.string;
      matchKeyword(Keyword.STRINGSY);

      while(nextSymbol == Keyword.PLUSSY)
       {
        lookAhead();
        ptr.code += ' ' + nextToken.string;
        matchKeyword(Keyword.STRINGSY);
       }

      ptr.code += '\n';

      matchKeyword(Keyword.SEMICOLONSY);

      queue.add(ptr);
     }
    else
     {
      ptr = statementExpression(scope, queue);
      if (ptr != null)
       {
        queue.add(ptr);

        ptr = new Operation();
        ptr.operator = Keyword.CLEARSY;
        queue.add(ptr);
       }

      matchKeyword(Keyword.SEMICOLONSY);
     }

    if (newLabel.size() > 0)
     {
      ptr = insertLabels("§break", newLabel);
      ptr.code = "0 " + ptr.code + "DROP ";
      ptr.operator = null;
      queue.add(ptr);
     }
   }

   /**
    * '{' blockStatement '}'
    * @param m MethodType
    * @param parameter VariableType
    * @param scope Scope
    * @param label Vector
    * @param blk int
    * @param continues Operation
    * @param queue Vector
    * @return Scope
    */
   private Scope block(MethodType m, VariableType parameter, Scope scope, Vector label, int blk,
                      Operation continues, Vector queue)
   {
    boolean first = false, constructor = false;
    Operation op;
    Scope x = blk == Scope.LOOP?scope:new Scope(scope, Scope.automatic, "", new Vector(label), blk);

    if (m != null)
     {
      // add hidden parameter exception to parameters
      if (m.throwing.length > 0)
       {
        Parameter p = new Parameter();
        p.name = new Token();
        p.name.string = "§exception";
        p.name.kind = Keyword.IDENTSY;
        p.type = new Type();
        p.type.type = Keyword.VOIDSY;

        Parameter[] q = new Parameter[m.parameter.length + 1];
        for (int i = 0; i < m.parameter.length; i++)
         q[i] = m.parameter[i];

        q[q.length - 1] = p;
        m.parameter = q;
       }

       Scope t;
       for(t = scope; t.prefix == null || t.prefix.length() == 0; t = t.prev);

       // constructor
       first = constructor = (m.modify & Keyword.CONSTRUCTORSY.value) != 0;

       // check, if inner class
       boolean inner = false;
       for (Scope y = t.prev; y != null && y.prev != null && !inner && first; y = y.prev)
        if ((inner = y.prefix != null && y.prefix.length() > 0))
         t = y;

       int version = 0;
       Basic [] b = t.prev.get(t.prefix);
       for (int i = 0; i < b.length; i++)
        if (b[i] instanceof ClassType && ((ClassType)b[i]).scope == t)
         version = b[i].version;

       // add hidden parameter §this to locals
       if ((m.modify & (Keyword.STATICSY.value | Keyword.CONSTRUCTORSY.value)) == 0 || inner)
        {
         Parameter p = new Parameter();
         p.name = new Token();
         p.name.string = inner ? "§outer" : "§this";
         p.type = new Type();
         p.type.version = version;
         p.type.ident.string = t.prefix;
         p.type.type = Keyword.NONESY;

         Parameter[] q = new Parameter[m.parameter.length + 1];
         q[0] = p;

         for (int i = 0; i < m.parameter.length; i++)
          q[i + 1] = m.parameter[i];

         m.parameter = q;
        }

       if (constructor)
        {
         Type z = new Type();
         z.version = version;
         z.ident.string = t.prefix;
         z.type = Keyword.NONESY;
         VariableType a = new VariableType(z, 0);

         a.name = new Token();
         a.name.string = "§this";
         a.name.kind = Keyword.IDENTSY;

         declInsert(x, a);
        }

       // parameter are local variables
       for (int i = m.parameter.length - 1; i >= 0; i--)
        {
         VariableType a = new VariableType(m.parameter[i].type, m.parameter[i].modify);
         a.name = m.parameter[i].name;
         a.referenced = true;
         declInsert(x, a);
        }
     }

    // add parameter of catch clause
    if (parameter != null)
     declInsert(x, parameter);

    // save current scope and update scope
    if (blk != Scope.LOOP)
     {
      op = new Operation();
      op.operator = Keyword.PUSHSY;
      op.scope = x;
      queue.add(op);
      
      if (blk == Scope.SYNCHRONIZED)
       {
    	continues.scope = x;
        queue.add(new Operation(continues));
       }
     }
    else
     label = scope.label;

    // assign parameter of catch clause
    if (parameter != null)
     {
      op = new Operation();
      op.code = "DUP 0 V! TO " + parameter.name.string + ' ';
      queue.add(op);
     }

    // set lock for synchronized method
    if (m != null && (m.modify & Keyword.SYNCHRONIZEDSY.value) != 0)
     {
      op = new Operation();
      op.operator = Keyword.LOCKSY;
      op.scope = x;
      queue.add(op);
     }

    if (constructor)
     { // insert "super();" into token stream
      scanner.insert("super();");
     }

    matchKeyword(Keyword.LBRACESY);

    if (constructor)
     {
      blockStatement(x, queue);
      boolean isThis = peek(0).kind == Keyword.LPARSY;
      boolean isSuper = isThis && nextSymbol == Keyword.SUPERSY;
      isThis = isThis && nextSymbol == Keyword.THISSY;

      if (isThis || isSuper)
       {
        // remove default super and clear data stack
        queue.remove(queue.size() - 1);
        queue.remove(queue.size() - 1);
        blockStatement(x, queue);
       }
      // remove clear data stack
      queue.remove(queue.size() - 1);

      if (!isThis)
       {
        op = new Operation();
        op.operator = Keyword.ALLOCATESY;
        queue.add(op);
       }
     }

    while(nextSymbol != Keyword.RBRACESY && nextSymbol != Keyword.EOFSY)
     {
      blockStatement(x, queue);
     }

    if (comment != null && comment.length() > 0)
     {
      op = new Operation();
      op.code = comment + '\n';
      op.operator = Keyword.COMMENTSY;
      queue.add(op);
      resetComment();
     }

    if (constructor)
     {
      // add return to constructors
      op = new Operation();
      op.code = "§this DUP 0 V! ( return object ) ";
      queue.add(op);
     }

    if (blk == Scope.LOOP)
     {
      // add continue label
      op = insertLabels("§continue", label);
      queue.add(op);
      queue.add(continues);
      continues = null;
     }
    else if (blk == Scope.TRY)
     {
      // add exception label
      Vector v = new Vector();
      v.add("" + Operation.newLabel());
      x.exception = "§except" + v.get(0);
      op = insertLabels("§except", v);
      queue.add(op);
     }

    // add break labels
    op = insertLabels("§break", label);
    if (continues != null && blk != Scope.SYNCHRONIZED)
     op.code += continues.code;
    queue.add(op);

    if (blk == Scope.SYNCHRONIZED)
     {
      op = new Operation(continues);
      op.operator = Keyword.UNLOCKSY;
      queue.add(op);
     }
    
    // set unlock for synchronized method
    if (m != null && (m.modify & Keyword.SYNCHRONIZEDSY.value) != 0)
     {
      op = new Operation();
      op.operator = Keyword.UNLOCKSY;
      op.scope = x;
      queue.add(op);
     }

    // deallocate locals
    op = new Operation();
    op.operator = Keyword.POPSY;
    queue.add(op);

    if (blk != Scope.LOOP)
     {
      if (blk == Scope.MAIN)
       {
    	op = new Operation();
        op.code = "DROP ";
       }
      else
       op = blockEnd(scope);
      
      queue.add(op);
     }

    matchKeyword(Keyword.RBRACESY);

    return x;
   }

   /**
    * localVariableDeclaration ';' | classOrInterfaceDeclaration | statement
    * @param scope Scope
    * @param queue Vector
    */
   private void blockStatement(Scope scope, Vector queue)
   {
    Modifier modify = new Modifier();

    modify.access = 0;
    modify.classes &= Keyword.FINALSY.value | Keyword.STRICTFPSY.value;
    modify.fields &= Keyword.FINALSY.value | Keyword.TRANSIENTSY.value | Keyword.VOLATILESY.value;
    modify.methods &= Keyword.FINALSY.value | Keyword.SYNCHRONIZEDSY.value | Keyword.NATIVESY.value | Keyword.STRICTFPSY.value;
    modify.constants &= Keyword.FINALSY.value;

    Token t;

    if (comment != null && comment.length() > 0)
     {
      Operation op = new Operation();
      op.code = comment + '\n';
      op.operator = Keyword.COMMENTSY;
      queue.add(op);
      resetComment();
     }

    if (isLocalVarDecl(false))
     {
      localVariableDeclaration(modify, scope, queue);
      matchKeyword(Keyword.SEMICOLONSY);
     }
    else
     {
      peekReset();
      
      t = nextToken;

      while(t.kind == Keyword.PUBLICSY || t.kind == Keyword.PROTECTEDSY ||
            t.kind == Keyword.PRIVATESY || t.kind == Keyword.ABSTRACTSY ||
            t.kind == Keyword.STATICSY || t.kind == Keyword.FINALSY ||
            t.kind == Keyword.STRICTFPSY)
       t = peek();
      
      if (t.kind == Keyword.CLASSSY || t.kind == Keyword.INTERFACESY)
       classOrInterfaceDeclaration(modify, scope, "");
      else
       statement(modify, scope, Scope.SEQUENCE, null, queue, true);
     }
   }

   /**
    * generate administrative code at the end of a block
    * @param scope scope of block
    * @return code
    */
   private Operation blockEnd(Scope scope)
   {
	Operation root = new Operation();
	
	root.operator = Keyword.BLOCKENDSY;
	root.code = (String)scope.label.get(0);

    return root;
   }

   /**
    * generate code for labels
    * @param prefix §continue or §break
    * @param label list of labels
    * @return code
    */
   private Operation insertLabels(String prefix, Vector label)
   {
    String s = "";

    for(int i = 0; i < label.size(); i++)
     {
      String l = (String) label.get(i);
      if (l.charAt(0) >= '0' && l.charAt(0) <= '9')
       s += prefix + l + " LABEL ";
     }

    Operation root = new Operation();
    root.operator = Keyword.LABELSY;
    root.code = s;
    
    return root;
   }

   /**
    * [ FINALSY ] type variableDeclarators
    * @param modify Modifier
    * @param scope Scope
    * @param queue Vector
    */
   private void localVariableDeclaration(Modifier modify, Scope scope, Vector queue)
   {
    if (nextSymbol == Keyword.FINALSY)
     {
      modify.add(nextSymbol.value);
      lookAhead();
     }

    modify.check(modify.fields | modify.access);
    Type t = type();

    variableDeclarators(modify, t, scope, queue);
   }

   /**
    * variableDeclarator { ',' variableDeclarator }
    * @param modify Modifier
    * @param t Type
    * @param scope Scope
    * @param queue Vector
    */
   private void variableDeclarators(Modifier modify, Type t, Scope scope, Vector queue)
   {
    VariableType x = new VariableType(new Type(t), modify.cur);

    follower.add(Keyword.COMMASY);
    variableDeclarator(x, scope, queue);
    declInsert(scope, x);

    while(nextSymbol == Keyword.COMMASY)
     {
      lookAhead();
      x = new VariableType(new Type(t), modify.cur);
      variableDeclarator(x, scope, queue);
      declInsert(scope, x);
     }
    follower.remove(follower.size() - 1);
   }

   /**
    * localVariableDeclaration | statementExpression moreStatementExpression
    * @param x Modifier
    * @param scope Scope
    * @param queue Vector
    */
   private void forInit(Modifier x, Scope scope, Vector queue)
   {
    if (isLocalVarDecl(true))
     localVariableDeclaration(new Modifier(x), scope, queue);
    else
     {
      Operation ret = moreStatementExpression(statementExpression(scope, queue), scope, queue);

      while(ret != null)
       {
        if (ret.left != null)
         queue.add(ret.left);

        ret = ret.right;

        Operation ptr = new Operation();
        ptr.operator = Keyword.CLEARSY;
        queue.add(ptr);
       }
     }
   }

   /**
    * statementExpression moreStatementExpression
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation forUpdate(Scope scope, Vector queue)
   {
    follower.add(Keyword.RPARSY);
    Operation op = moreStatementExpression(statementExpression(scope, queue), scope, queue);
    follower.remove(follower.size() - 1);
    return op;
   }

   /**
    * expression
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation statementExpression(Scope scope, Vector queue)
   {
    Operation ret = expression(scope, queue);

    return ret;
   }

   /**
    * { ',' statementExpression }
    * @param first Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation moreStatementExpression(Operation first, Scope scope, Vector queue)
   {
    Operation root = new Operation(), next = root;
    root.left = first;

    follower.add(Keyword.COMMASY);
    while(nextSymbol == Keyword.COMMASY)
     {
      lookAhead();
      Operation op = statementExpression(scope, queue);

      if (next.left != null)
       next = (next.right = new Operation());

      next.left = op;
     }

    follower.remove(follower.size() - 1);

    return root;
   }

   /**
    * catchClause { catchClause }
    * @param label String
    * @param scope Scope
    * @param queue Vector
    */
   private void catches(String label, Scope scope, Vector queue)
   {
    catchClause(label, scope, queue);

    while(nextSymbol == Keyword.CATCHSY)
     catchClause(label, scope, queue);
   }

   /**
    * "catch" '(' formalParameter ')' block
    * @param label String
    * @param scope Scope
    * @param queue Vector
    */
   private void catchClause(String label, Scope scope, Vector queue)
   {
    matchKeyword(Keyword.CATCHSY);
    matchKeyword(Keyword.LPARSY);

    Parameter parameter = formalParameter();

    Operation root = new Operation();
    root.code = "DUP \" " + parameter.type.ident.string.substring(parameter.type.ident.string.lastIndexOf('.') + 1) + " \" INSTANCEOF\n IF\n";
    queue.add(root);

    matchKeyword(Keyword.RPARSY);

    VariableType a = new VariableType(parameter.type, 0);
    a.name = parameter.name;

    Vector v = new Vector();
    v.add("" + Operation.newLabel());
    block(null, a, scope, v, Scope.CATCH, null, queue);

    root = new Operation();
    root.code = "DUP " + label + " BRANCH\nENDIF\n";
    queue.add(root);
   }

   /**
    * { switchBlockStatementGroup }
    * @param scope Scope
    * @param queue Vector
    */
   private void switchBlockStatementGroups(Scope scope, Vector queue)
   {
    String defaults = null;
    Vector code = new Vector();
    Operation op;

    while(nextSymbol == Keyword.CASESY || nextSymbol == Keyword.DEFAULTSY)
     defaults = switchBlockStatementGroup(defaults, scope, code, queue);

    op = new Operation();
    op.code = ((defaults != null)?defaults:("DROP §break" + (String)scope.label.get(0))) + " BRANCH\n";
    queue.add(op);

    for(int i = 0; i < code.size(); i++)
     queue.add(code.get(i));
   }

   /**
    * switchLabel { switchLabel } blockStatement
    * @param defaults String
    * @param scope Scope
    * @param code Vector
    * @param queue Vector
    * @return String
    */
   private String switchBlockStatementGroup(String defaults, Scope scope, Vector code, Vector queue)
   {
    Operation op;
    String label = "§§group_" + Operation.newLabel();

    do
     if (switchLabel(scope, queue))
      if (defaults != null)
       error("multiple default in switch block!");
      else
       defaults = label;
     else
      {
       op = new Operation();
       op.code = "OVER <> " + label + " 0BRANCH\n";
       queue.add(op);
      }
    while(nextSymbol == Keyword.CASESY || nextSymbol == Keyword.DEFAULTSY);

    Vector dummy = queue;
    queue = code;

    op = new Operation();
    op.code = "DUP " + label + " LABEL DROP\n";
    queue.add(op);

    while(nextSymbol != Keyword.CASESY && nextSymbol != Keyword.DEFAULTSY &&
          nextSymbol != Keyword.RBRACESY && nextSymbol != Keyword.EOFSY)
     blockStatement(scope, queue);

    queue = dummy;

    return defaults;
   }

   /**
    * ( "default" | "case" type ) ':'
    * @param scope Scope
    * @param queue Vector
    * @return boolean
    */
   private boolean switchLabel(Scope scope, Vector queue)
   {
    boolean x;

    if (x = nextSymbol == Keyword.DEFAULTSY)
     {
      lookAhead();
     }
    else
     {
      matchKeyword(Keyword.CASESY);

      Type t = new Type();
      t.type = Keyword.INTSY;

      follower.add(Keyword.COLONSY);
      castExpression(scope, t, true, queue);
      follower.remove(follower.size() - 1);
     }

    matchKeyword(Keyword.COLONSY);

    return x;
   }

   /**
    * expression1 [ assign expression ]
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation expression(Scope scope, Vector queue)
   {
    Operation root = expression1(scope, queue);

    if (assign.contains(nextSymbol))
     {
      Operation op = new Operation();

      op.left = root;

      op.operator = assignmentOperator();
      op.right = expression(scope, queue);

      root = op;
     }

    return root;
   }

   /**
    * expression2 [ conditionalExpr ]
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation expression1(Scope scope, Vector queue)
   {
    follower.add(Keyword.QUESTIONMARKSY);
    Operation op = expression2(scope, queue);
    follower.remove(follower.size() - 1);

    if (nextSymbol == Keyword.QUESTIONMARKSY)
     {
      Operation a = new Operation();
      a.operator = nextSymbol;
      a.right = conditionalExpr(scope, queue);
      a.left = op;
      op = a;
     }

    return op;
   }

   /**
    * '?' expression ':' expression1
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation conditionalExpr(Scope scope, Vector queue)
   {
    Operation root = new Operation();

    matchKeyword(Keyword.QUESTIONMARKSY);
    follower.add(Keyword.COLONSY);
    root.left = expression(scope, queue);
    follower.remove(follower.size() - 1);
    matchKeyword(Keyword.COLONSY);
    root.right = expression1(scope, queue);

    return root;
   }

   /**
    * expression3 [ expression2Rest ]
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation expression2(Scope scope, Vector queue)
   {
    Operation root = expression3(scope, queue);

    if (infix.contains(nextSymbol))
     root = expression2Rest(root, scope, queue);

    return root;
   }

   /**
    * logicalOr
    * @param root Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation expression2Rest(Operation root, Scope scope, Vector queue)
   {
    root = logicalOr(root, scope, queue);

    if (follower.contains(nextSymbol))
     return root;

    String skip = "skipping: ";
    while(!follower.contains(nextSymbol))
     {
      skip += nextToken.string + ' ';
      lookAhead();
     }
    error(skip);

    return root;
   }

   /**
    * [LogicalAnd] {„||“ Expression3 [LogicalAnd]}
    * @param root Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation logicalOr(Operation root, Scope scope, Vector queue)
   {
	boolean one = false;
	
    if (nextSymbol != Keyword.LOGICALORSY)
     root = logicalAnd(root, scope, queue);
    
    Operation literal = getLiteral(root);
    
    if (literal != null)
     one = literal.code.charAt(0) == 'T';

    while(nextSymbol == Keyword.LOGICALORSY)
     {
      Operation x = new Operation();
      
      x.operator = nextSymbol;
      lookAhead();
      x.left = root;
      x.right = expression3(scope, queue);
      root = x;
      
      if (nextSymbol != Keyword.LOGICALORSY)
       root.right = logicalAnd(root.right, scope, queue);
      
      if (one)
       root = root.left;
      else if ((literal = getLiteral(root.right)) != null)
       if (literal.code.charAt(0) == 'F')
    	root = root.left;
       else
        one = true;
     }

    return root;
   }

   /**
    * [Relop] {„&&“ Expression3 [Relop]}
    * @param root Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation logicalAnd(Operation root, Scope scope, Vector queue)
   {
	boolean zero = false;
		
    if (nextSymbol != Keyword.LOGICALANDSY)
     root = relop(root, scope, queue);
    
    Operation literal = getLiteral(root);
    
    if (literal != null)
     zero = literal.code.charAt(0) == 'F';

    while(nextSymbol == Keyword.LOGICALANDSY)
     {
      Operation x = new Operation();

      x.operator = nextSymbol;
      lookAhead();
      x.left = root;
      x.right = expression3(scope, queue);
      root = x;

      if (nextSymbol != Keyword.LOGICALANDSY)
       root.right = relop(root.right, scope, queue);
      
      if (zero)
       root = root.left;
      else if ((literal = getLiteral(root.right)) != null)
       if (literal.code.charAt(0) == 'T')
    	root = root.left;
       else
        zero = true;
     }

    return root;
   }

   /**
    * InstanceOf [(„==“ | „!=“) InstanceOf] | [or] [(„<=“ | „<“ | „>=“ | „>“) Expression3 [or]]
    * @param root Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation relop(Operation root, Scope scope, Vector queue)
   {
    Operation x, literall, literalr;
    boolean i;

    if (i = nextSymbol == Keyword.INSTANCEOFSY)
     root = instanceOf(root, scope, queue);
    else if (nextSymbol != Keyword.NOTEQUALSY && nextSymbol != Keyword.EQUALSY &&
             nextSymbol != Keyword.LESSEQUALSY && nextSymbol != Keyword.LESSSY &&
             nextSymbol != Keyword.GREATEREQUALSY && nextSymbol != Keyword.GREATERSY)
     root = or(root, scope, queue);

    if (nextSymbol == Keyword.NOTEQUALSY || nextSymbol == Keyword.EQUALSY)
     {
      x = new Operation();
      x.operator = nextSymbol;
      lookAhead();
      x.left = root;
      x.right = instanceOf(expression3(scope, queue), scope, queue);

      literall = getLiteral(x.left);
      literalr = getLiteral(x.right);
      
      if (literall != null && literalr != null)
       {
    	long vall = 0, valr = 0;
    	double fvall = 0., fvalr = 0.;
    	boolean res = false, bvall = false, bvalr = false;
    	
    	bvall = literall.code.charAt(0) == 'T';
    	String w = literall.code.substring(0, literall.code.indexOf(' '));
    	if (literall.type.type == Keyword.LONGSY)
    	 fvall = vall = Long.parseLong(w);
    	else
    	 vall = (int)(fvall = Double.parseDouble(w));
    	
    	bvalr = literalr.code.charAt(0) == 'T';
    	w = literalr.code.substring(0, literalr.code.indexOf(' '));
    	if (literalr.type.type == Keyword.LONGSY)
    	 fvalr = valr = Long.parseLong(w);
    	else
    	 valr = (int)(fvalr = Double.parseDouble(w));
    	
    	if (literalr.type.type == Keyword.BOOLEANSY)
    	 if (x.operator == Keyword.EQUALSY)
    	  res = bvall == bvalr;
    	 else
    	  res = bvall != bvalr;
    	else if (literalr.type.type == Keyword.DOUBLESY)
    	 if (x.operator == Keyword.EQUALSY)
    	  res = fvall == fvalr;
    	 else
    	  res = fvall != fvalr;
    	else
       	 if (x.operator == Keyword.EQUALSY)
       	  res = vall == valr;
       	 else
       	  res = vall != valr;
    	
    	literalr.code = res?"TRUE ":"FALSE ";
    	literalr.type.type = Keyword.BOOLEANSY;
    	
    	root = x.right;
       }
      else if (x.right != null)
       root = x;
     }
    else if (nextSymbol == Keyword.LESSEQUALSY || nextSymbol == Keyword.LESSSY ||
             nextSymbol == Keyword.GREATEREQUALSY || nextSymbol == Keyword.GREATERSY)
     {
      x = new Operation();
      x.operator = nextSymbol;
      lookAhead();
      x.left = root;
      x.right = or(expression3(scope, queue), scope, queue);

      literall = getLiteral(x.left);
      literalr = getLiteral(x.right);
      
      if (i)
       error("\"==\" or \"!=\" expected, not \"" + x.operator.string + "\"");
      else if (literall != null && literalr != null)
       {
       	long vall = 0, valr = 0;
       	double fvall = 0., fvalr = 0.;
       	boolean res = false;
        	
        String w = literall.code.substring(0, literall.code.indexOf(' '));
        	
        w = literalr.code.substring(0, literalr.code.indexOf(' '));
        if (literalr.type.type == Keyword.LONGSY)
         fvalr = valr = Long.parseLong(w);
        else
         valr = (int)(fvalr = Double.parseDouble(w));
        	
        if (literalr.type.type == Keyword.DOUBLESY)
         if (x.operator == Keyword.LESSEQUALSY)
          res = fvall <= fvalr;
         else if (x.operator == Keyword.LESSSY)
          res = fvall < fvalr;
         else if (x.operator == Keyword.GREATEREQUALSY)
          res = fvall >= fvalr;
         else
          res = fvall > fvalr;
        else
         if (x.operator == Keyword.LESSEQUALSY)
          res = vall <= valr;
         else if (x.operator == Keyword.LESSSY)
          res = vall < valr;
         else if (x.operator == Keyword.GREATEREQUALSY)
          res = vall >= valr;
         else
          res = vall > valr;
          
        literalr.code = res?"TRUE ":"FALSE ";
        literalr.type.type = Keyword.BOOLEANSY;
        	
        root = x.right;
       }
      else if (x.right != null)
       root = x;
     }

    return root;
   }

   /**
    * (Or | „instanceof“ Type)
    * @param root Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation instanceOf(Operation root, Scope scope, Vector queue)
   {
    if (nextSymbol == Keyword.INSTANCEOFSY)
     {
      Operation literal, x = new Operation();

      x.operator = nextSymbol;
      lookAhead();
      x.left = root;
      x.right = new Operation();
      x.right.type = type();
      
      literal = getLiteral(root);
      
      if (literal != null &&
    	  literal.type.type == Keyword.NONESY &&
    	  literal.type.ident.string.compareTo("Object") == 0)
       {
    	literal.code = "FALSE ";
    	literal.type.type = Keyword.BOOLEANSY;
       }
      else
       root = x;
     }
    else
     root = or(root, scope, queue);

    return root;
   }

   /**
    * [And] {('|' | '^') Expression3 [And]}
    * @param root Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation or(Operation root, Scope scope, Vector queue)
   {
    if (nextSymbol != Keyword.ORSY && nextSymbol != Keyword.XORSY)
     root = and(root, scope, queue);

    while(nextSymbol == Keyword.ORSY || nextSymbol == Keyword.XORSY)
     {
      Operation next = null, x = new Operation();
      boolean leftliteral = false, rightliteral = false;
      String w;
      long vall = 0, valr = 0;
  	  boolean isone = false;
  	  Keyword t = Keyword.INTSY;

      x.operator = nextSymbol;
      lookAhead();
      x.left = root;
      x.right = expression3(scope, queue);
      root = x;

      if (nextSymbol != Keyword.ORSY && nextSymbol != Keyword.XORSY)
       root.right = and(root.right, scope, queue);
      
      Operation literal = getLiteral(root.left);
      Operation literalr = getLiteral(root.right);
      
      if (literal == null)
       if (root.left.right != null &&
    	   (root.left.operator == Keyword.XORSY ||
    		root.left.operator == Keyword.ORSY) &&
       	   getLiteral(root.left.right) != null)
       if (root.operator == root.left.operator)
    	literal = getLiteral(root.left.right);
  	 
 	  if (literal != null)
       {
   	    next = literal;
	    w = next.code.substring(0, next.code.indexOf(' '));
	    vall = Long.parseLong(w);
	    
	    if (vall == 0)
	     root = root.right;
	    else
	     leftliteral = true;
	    
	    isone = vall == -1 && root.operator == Keyword.ORSY;
	    t = next.type.type;
       }
 	 
 	  if (literalr != null)
       {
   	    next = literalr;
	    w = next.code.substring(0, next.code.indexOf(' '));
	    valr = Long.parseLong(w);
	    
	    if (valr == 0)
	     root = root.left;
	    else
	     rightliteral = true;
	    
	    isone = valr == -1 && root.operator == Keyword.ORSY;
	    
	    if (next.type.type == Keyword.LONGSY)
	     t = Keyword.LONGSY;
       }
    	 
 	  if (isone)
       root = createLiteral(t, "-1 " + (t == Keyword.LONGSY?"S>D ":""));
 	  else if (leftliteral)
       if (rightliteral)
    	{
    	 if (root.operator == Keyword.XORSY)
    	  vall ^= valr;
    	 else
    	  vall |= valr;
    	   
    	 if (t == Keyword.INTSY)
    	  literal.code = (int)vall + " ";
    	 else
    	  {
    	   literal.code = vall + " ";
    	   if (Integer.MAX_VALUE >= Math.abs(vall))
    		literal.code += "S>D ";
    	  }
    	   
    	 root = root.left;
        }
       else if (literal == root.left.left.left.left)
        {
    	 next = root.left;
    	 root.left = root.right;
    	 root.right = next;
        }
       else
        {
    	 next = root.left.right;
    	 root.left.right = root.right;
    	 root.right = next;
        }
     }
    
    return root;
   }

   /**
    * [shift] {'&' Expression3 [shift]}
    * @param root Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation and(Operation root, Scope scope, Vector queue)
   {
	boolean iszero = false;
	Keyword t = Keyword.INTSY;
	
    if (nextSymbol != Keyword.ANDSY)
     root = shift(root, scope, queue);

    while(nextSymbol == Keyword.ANDSY)
     {
      Operation next = null, x = new Operation();
      String w;
      long vall = 0, valr = 0;

      x.operator = nextSymbol;
      lookAhead();
      x.left = root;
      x.right = expression3(scope, queue);
      root = x;

      if (nextSymbol != Keyword.ANDSY)
       root.right = shift(root.right, scope, queue);
    	       
      Operation literal = getLiteral(root.left);
      Operation literalr = getLiteral(root.right);
      
      if (literal == null)
       if (root.left.operator == Keyword.ANDSY)
    	literal = getLiteral(root.left.right);
      
 	  if (literal != null)
 	   {
 	  	next = literal;
 		w = next.code.substring(0, next.code.indexOf(' '));
 		vall = Long.parseLong(w);
 		
 		if (literal.type.type == Keyword.LONGSY)
 		 t = Keyword.LONGSY;
        
        iszero |= vall == 0;
        
        if (vall == -1)
	     root = root.right;
	    else if (literalr != null)
         {
          next = root.right.left.left.left;
          w = next.code.substring(0, next.code.indexOf(' '));
          valr = Long.parseLong(w);
          
          iszero |= valr == 0;
 		
 		  if (next.type.type == Keyword.LONGSY)
 		   t = Keyword.LONGSY;
        
          if (valr != -1)
           {
       	    vall &= valr;
       	   
       	    if (t != Keyword.LONGSY)
       	     literal.code = (int)vall + " ";
       	    else
       	     {
       	      literal.code = vall + " ";
       		  if (Integer.MAX_VALUE >= Math.abs(vall))
       		   literal.code += "S>D ";
       	     }
       	    literal.type.type = t;
       	   }
          
       	  root = root.left;
       	 }
        else if (literal == root.left.left.left.left)
         {
    	  next = root.left;
    	  root.left = root.right;
    	  root.right = next;
         }
        else
         {
    	  next = root.left.right;
    	  root.left.right = root.right;
    	  root.right = next;
         }
       }
     }

    if (iszero)
     root = createLiteral(Keyword.INTSY, "0 ");
    
    return root;
   }

   /**
    * [LowArith] {(„>>“ | „<<“ | „>>>“) Expression3 [LowArith]}
    * @param root Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation shift(Operation root, Scope scope, Vector queue)
   {
    if (nextSymbol != Keyword.RSHIFTSY && nextSymbol != Keyword.LSHIFTSY &&
        nextSymbol != Keyword.ASHIFTSY)
     root = lowArith(root, scope, queue);

    while(nextSymbol == Keyword.RSHIFTSY || nextSymbol == Keyword.LSHIFTSY ||
          nextSymbol == Keyword.ASHIFTSY)
     {
      Operation next, x = new Operation();

      x.operator = nextSymbol;
      lookAhead();
      x.left = root;
      x.right = expression3(scope, queue);
      root = x;

      if (nextSymbol != Keyword.RSHIFTSY && nextSymbol != Keyword.LSHIFTSY &&
          nextSymbol != Keyword.ASHIFTSY)
       root.right = lowArith(root.right, scope, queue);
 	 
      long valr = 0;
      String w;
      Operation literal = getLiteral(root.left);
      Operation literalr = getLiteral(root.right);
    	
      if (literalr != null)
       {
        next = literalr;
        w = next.code.substring(0, next.code.indexOf(' '));
        valr = Long.parseLong(w);
          
        if (valr != 0 && literal != null)
    	 {
    	  next = literal;
    	  w = next.code.substring(0, next.code.indexOf(' '));
    	  long vall = Long.parseLong(w);
    	   
    	  if (root.operator == Keyword.RSHIFTSY)
    	   vall >>= valr;
    	  else if (root.operator == Keyword.LSHIFTSY)
    	   vall <<= valr;
    	  else
       	   vall >>>= valr;
    	   
    	  if (next.type.type != Keyword.LONGSY)
    	   next.code = (int)vall + " ";
    	  else
    	   {
        	next.code = vall + " ";
    		if (Integer.MAX_VALUE >= Math.abs(vall))
    		 next.code += "S>D ";
    	   }
    	   
    	  root = root.left;
    	 }
       }
     }

    return root;
   }

   /**
    * [Arith] {('+' | '-') Expression3 [Arith]}
    * @param root Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation lowArith(Operation root, Scope scope, Vector queue)
   {
	long val = 0;
	double fval = 0.;
	Operation next = null, ptr = null, last = null;
	Keyword t = Keyword.INTSY, operator = null;
	
    if (nextSymbol != Keyword.PLUSSY && nextSymbol != Keyword.MINUSSY)
     root = arith(root, scope, queue);

    while(nextSymbol == Keyword.PLUSSY || nextSymbol == Keyword.MINUSSY)
     {
      Operation x = new Operation();
      String w = null, svalr = null, svall = null;
      long valr = 0;
      double fvalr = 0.;
      
      x.operator = nextSymbol;
      lookAhead();
      x.left = root;
      x.right = expression3(scope, queue);
      root = x;

      if (nextSymbol != Keyword.PLUSSY &&
    	  nextSymbol != Keyword.MINUSSY)
       root.right = arith(root.right, scope, queue);
       
      Operation literal = getLiteral(root.left);
      Operation literalr = getLiteral(root.right);
      
      if (literal == null)
       if (root.left.operator == Keyword.PLUSSY ||
    	   root.left.operator == Keyword.MINUSSY)
        literal = getLiteral(root.left.right);
      
 	  if (literal != null)
 	   {
 	  	next = literal;
 		w = next.code.substring(0, next.code.indexOf(' '));
 		
 		if (next.type.type == Keyword.NONESY)
 		 {
 		  svall = next.code.trim();
 		  svall = svall.substring(svall.indexOf('"') + 2);
 		  svall = svall.substring(0, svall.lastIndexOf('"') - 1);
 		 }
 		else if (ptr == null)
 	     if (next.type.type == Keyword.DOUBLESY)
 		  {
 		   fval = Double.parseDouble(w);
 		   val = 0;
 		   t = Keyword.DOUBLESY;
 		   operator = Keyword.PLUSSY;
 		   ptr = literal;
 		   last = root;
 		  }
 		 else
 		  {
 		   val = Long.parseLong(w);
 		   fval = 0.;
 		   if (next.type.type == Keyword.LONGSY)
 		    t = Keyword.LONGSY;
 		   operator = Keyword.PLUSSY;
 		   ptr = literal;
 		   last = root;
 		  }
 	   }
	 
	  if (literalr != null)
       {
  	    next = literalr;
	    w = next.code.substring(0, next.code.indexOf(' '));
	   
	    if ((next.type.type == Keyword.NONESY || svall != null) &&
	    	literal != null)
	     {
	      if (next.type.type == Keyword.NONESY)
	       {
		    svalr = next.code.trim();
		    svalr = svalr.substring(svalr.indexOf('"') + 2);
		    svalr = svalr.substring(0, svalr.lastIndexOf('"') - 1);
	       }
	      else if (next.type.type == Keyword.DOUBLESY)
	 	   fvalr = Double.parseDouble(w);
	 	  else
	 	   valr = Long.parseLong(w);

		  if (svall != null)
	  	   if (svalr != null)
	  	    svall = "U\" " + svall + svalr + " \" COUNT ";
	  	   else if (fvalr != 0.)
	  	    svall = "U\" " + svall + fvalr + " \" COUNT ";
	  	   else
	  	    svall = "U\" " + svall + valr + " \" COUNT ";
	  	  else if (fval != 0.)
		   svall = "U\" " + fval + svalr + " \" COUNT ";
	  	  else
	  	   svall = "U\" " + val + svalr + " \" COUNT ";
	  	    		 
		  literal.code = svall;
	  	  literal.type = new Type();
		  literal.type.type = Keyword.NONESY;
		  literal.type.ident = new Token();
		  literal.type.ident.kind = Keyword.IDENTSY;
		  literal.type.ident.string = "String";
		  root = root.left;
			
		  if (ptr == literal)
		   {
			t = Keyword.INTSY;
			operator = null;
			fval = val = 0;
			ptr = null;
			last = null;
		   }
	     }
	    else if (next.type.type == Keyword.DOUBLESY)
	     {
	      fvalr = Double.parseDouble(w);
	      
	      if (fvalr == 0.)
	       root = root.left;
	      else if (ptr != null)
	       {
	    	if (t == Keyword.DOUBLESY)
	    	 if (operator == root.operator)
	    	  fval = fval + fvalr;
	    	 else
	    	  fval = fval - fvalr;
	    	else if (operator == root.operator)
		     fval = val + fvalr;
	    	else
	    	 fval = val - fvalr;
	    	
		    t = Keyword.DOUBLESY;
		    val = 0;
		      
		    root = root.left;
	       }
	      else
	       {
		    ptr = next;
		    operator = root.operator;
		    fval = fvalr;
		    t = Keyword.DOUBLESY;
		    last = root;
		   }
	     }
	    else if (next.type.type != Keyword.NONESY)
	     {
	      valr = Long.parseLong(w);

	      if (valr == 0)
	       root = root.left;
	      else if (ptr != null)
	       {
	    	if (t == Keyword.DOUBLESY)
	    	 if (operator == root.operator)
	    	  fval = fval + valr;
	    	 else
	    	  fval = fval - valr;
	    	else if (operator == root.operator)
		     val = val + valr;
	    	else
	    	 val = val - valr;
	    	
		    if (t != Keyword.DOUBLESY)
			 if (t != Keyword.LONGSY)
			  t = next.type.type == Keyword.LONGSY?Keyword.LONGSY:Keyword.INTSY;

			root = root.left;
	       }
	      else
	       {
		    ptr = next;
		    operator = root.operator;
		    val = valr;
			t = next.type.type == Keyword.LONGSY?Keyword.LONGSY:Keyword.INTSY;
			last = root;
		   }
	     }
       }
     }

    if (ptr != null)
     {
      if (t == Keyword.DOUBLESY)
       ptr.code = fval + " ";
      else if (t == Keyword.INTSY)
       ptr.code = (int)val + " ";
      else
       {
        ptr.code = val + " ";
	    if (Integer.MAX_VALUE >= Math.abs(val))
	     ptr.code += "S>D ";
	   }
      
      ptr.type.type = t;
      
      next = last.left;
      
      if (t == Keyword.DOUBLESY && fval == 0. || val == 0)
       if (last == root)
        {
    	 if (root.right != null)
    	  if (ptr == root.right.left.left.left)
    	   root = root.left;
    	  else if (root.operator == Keyword.PLUSSY)
    	   root = root.right;
        }
       else if (next != null && next.right != null)
    	if (ptr == next.right.left.left.left)
         last.left = next.left;
        else if (next.operator == Keyword.PLUSSY)
    	 last.left = next.right;
     }
    
    return root;
   }

   /**
    * {('*' | '/' | '%') Expression3}
    * @param root Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation arith(Operation root, Scope scope, Vector queue)
   {
	Operation next;
	boolean iszero = false;
		
    while(nextSymbol == Keyword.MULTIPLYSY || nextSymbol == Keyword.SLASHSY ||
          nextSymbol == Keyword.MODULOSY)
     {
      Operation x = new Operation(), ptr = null;
      String w = null;
      long valr = 0;
      double fvalr = 0.;
      double fvall = 0.;
      long vall = 0;
      Keyword operator = null;
  	  Keyword t = Keyword.INTSY;

      x.operator = nextSymbol;
      lookAhead();
      x.left = root;
      x.right = expression3(scope, queue);
      root = x;
      
      if (getLiteral(root.left) != null)
       {
        ptr = root.left;
        operator = root.operator;
       }
      else if (getLiteral(root.left.right) != null)
       if (root.left.operator == root.operator)
        if (root.operator != Keyword.MODULOSY)
         {
          ptr = root.left.right;
          operator = Keyword.MULTIPLYSY;
         }
 	 
      if (ptr != null)
       {
        next = ptr.left.left.left;
        w = next.code.substring(0, next.code.indexOf(' '));
     	   
        if ((t = next.type.type) == Keyword.DOUBLESY)
         fvall = Double.parseDouble(w);
        else
         {
     	  vall = Long.parseLong(w);
     	  if (t != Keyword.LONGSY)
     	   t = Keyword.INTSY;
         }
    	   
        iszero |= fvall == 0. && vall == 0;
       }
      
 	  boolean rightliteral = false;
  	  Keyword tt = Keyword.INTSY;
	 
      if (getLiteral(x.right) != null)
       {
        next = getLiteral(x.right);
        w = next.code.substring(0, next.code.indexOf(' '));
     	   
        if (next.type.type == Keyword.DOUBLESY)
         {
     	  fvalr = Double.parseDouble(w);
     	  tt = Keyword.DOUBLESY;
         }
        else
         {
     	  valr = Long.parseLong(w);
     	  if (next.type.type == Keyword.LONGSY)
     	   tt = Keyword.LONGSY;
         }
        
        if (rightliteral = fvalr == 1. || valr == 1)
         if (x.operator == Keyword.MODULOSY)
          iszero = true;
         else
          root = x.left;
        
        rightliteral = !rightliteral;
    	
        iszero |= fvalr == 0. && valr == 0 &&
                  root.operator == Keyword.MULTIPLYSY;
       }
      
      if (!iszero && ptr != null)
       if (rightliteral)
        {
         next = ptr.left.left.left;
        
       	 if (fvall != 0. || fvalr != 0.)
       	  {
       	   if (fvall == 0.)
       	    fvall = vall;
       	   if (fvalr == 0.)
       	    fvalr = valr;
       		  
       	   if (operator == Keyword.MULTIPLYSY)
       	    fvall = fvall * fvalr;
       	   else if (operator == Keyword.SLASHSY)
            fvall = fvall / fvalr;
       	   else
       	    fvall = fvall % fvalr;
       		  
           iszero |= fvall == 0.;
       	   next.code = fvall + " ";
       	   t = Keyword.DOUBLESY;
       	  }
       	 else
       	  {
           if (operator == Keyword.MULTIPLYSY)
            vall = vall * valr;
           else if (operator == Keyword.SLASHSY)
            vall = vall / valr;
           else
            vall = vall % valr;
       		  
           if (tt == Keyword.LONGSY)
        	t = Keyword.LONGSY;
           
           iszero |= vall == 0;
       	   next.code = vall + " ";
           if (t == Keyword.LONGSY)
            if (Integer.MAX_VALUE >= Math.abs(vall))
        	 next.code += "S>D ";
       	  }
       	   
         next.type.type = t;
       	 root = root.left;
        }
       else if (root.operator == Keyword.MULTIPLYSY)
        {
    	 if (ptr == root.left)
    	  root.left = root.right;
    	 else
    	  root.left.right = root.right;
    	 
         root.right = ptr;
        	
         if (vall == 1 || fvall == 1.)
          root = root.left;
         
         fvall = vall = 0;
        }
     }

    if (iszero)
     root = createLiteral(Keyword.INTSY, "0 ");
    
    return root;
   }

   private Operation createLiteral(Keyword t, String code)
    {
	 Operation root;
	 
	 root = new Operation();
	 root.operator = Keyword.LEAFSY;

	 root.left = new Operation();
	 root.left.left = new Operation();
	 root.left.left.left = new Operation();

	 root.left.left.left.operator = Keyword.LITERALSY;
	 root.left.left.left.type = new Type();
	 root.left.left.left.type.type = t;
	 root.left.left.left.code = code;
	 
	 return root;
    }
   
   /**
    * { prefix | '(' type ')' } primary { selector } postfix
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation expression3(Scope scope, Vector queue)
   {
    Operation root, op, next, prepost = null;
    int j = 0;

    root = new Operation();
    root.operator = Keyword.LEAFSY;

    while(prefix.contains(nextSymbol) || isTypeCast())
     {
      op = new Operation();

      if (prefix.contains(nextSymbol))
       {
        op.operator = prefixOp();

        if (op.operator == Keyword.INCSY || op.operator == Keyword.DECSY)
         {
          j++;
         }
       }
      else
       {
        matchKeyword(Keyword.LPARSY);

        op.type = type();
        op.operator = Keyword.CASTSY;

        matchKeyword(Keyword.RPARSY);
       }

      next = new Operation();
      next.left = op;
      next.right = prepost;
      prepost = next;
     }

    if (j > 1)
     error("multiple pre increment/decrement operators");

    Operation p = next = (root.left = new Operation());
    next.left = primary(scope, queue);
    
    while(nextSymbol == Keyword.LBRACKETSY || nextSymbol == Keyword.DOTSY)
     {
      next = (next.right = new Operation());
      next.left = selector(scope, queue);
     }

    String qualified = "";
    for(Operation q = p.left; q != null && q.operator == null && q.left.name != null; q = q.right)
     qualified += q.left.name.string + '.';
    for(Operation q = p.right; q != null && q.operator == null && q.left.name != null; q = q.right)
     qualified += q.left.name.string + '.';
    if (qualified.length() > 0 && qualified.indexOf('.') != qualified.lastIndexOf('.'))
     unresolved.add(qualified.substring(0, qualified.length() - 1));

    op = new Operation();
    if ((op.operator = postfixOp()) == null)
     op = null;

    if (op != null && j > 0)
     error("concurring prefix and postfix operators!");

    if (prepost != null || op != null)
     {
      root.right = new Operation();
      root.right.left = prepost;
      root.right.right = op;
     }

    if (root.left.left == null)
     root = null;
    else
     root = resolvePrefix(root);

    return root;
   }

   /**
    * '(' Expression ')'
    * | "this" ArgumentsOpt
    * | "super" SuperSuffix
    * | Literal
    * | "new" Creator
    * | ident { '.' ident } [ IdentifierSuffix ]
    * | BasicType BracketsOpt '.' "class"
    * | "void" '.' "class"
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation primary(Scope scope, Vector queue)
   {
    Operation root = new Operation();

    root.left = new Operation();

    if (nextSymbol == Keyword.LPARSY)
     {
      Operation next;
      
      root.left.operator = nextSymbol;
      lookAhead();
      follower.add(Keyword.RPARSY);
      root.left.left = next = expression(scope, queue);
      follower.remove(follower.size() - 1);
      matchKeyword(Keyword.RPARSY);
      
      if (next.operator == Keyword.LEAFSY)
       if (next.left.left.left.operator == Keyword.LITERALSY)
        root.left = next.left.left.left;
      
     }
    else if (nextSymbol == Keyword.THISSY)
     {
      root.left.operator = nextSymbol;
      lookAhead();
      root.left.left = argumentsOpt(scope, null, queue);
     }
    else if (nextSymbol == Keyword.SUPERSY)
     {
      root.left.operator = nextSymbol;
      lookAhead();
      superSuffix(root, scope, queue);
     }
    else if (nextSymbol == Keyword.NEWSY)
     {
      root.left.operator = nextSymbol;
      lookAhead();
      root.left.left = creator(scope, queue);
     }
    else if (nextSymbol == Keyword.IDENTSY)
     {
      Operation next = root;
      root.left.name = nextToken;

      lookAhead();

      while(dotAndIdent())
       {
        matchKeyword(Keyword.DOTSY);
        next = (next.right = new Operation());
        next.left = new Operation();
        next.left.name = nextToken;
        matchKeyword(Keyword.IDENTSY);
       }

      if (isIdentSuffix())
       root = identifierSuffix(root, next, scope, queue);
     }
    else if (typeKW.contains(nextSymbol) || nextSymbol == Keyword.VOIDSY)
     {
      root = null;
      lookAhead();
      bracketsOpt();
      matchKeyword(Keyword.DOTSY);
      matchKeyword(Keyword.CLASSSY);
     }
    else
     {
      literal(root.left);
     }

    return root;
   }

   /**
    * [ arguments ]
    * @param scope Scope
    * @param v Vector
    * @param queue Vector
    * @return Operation
    */
   private Operation argumentsOpt(Scope scope, Vector v, Vector queue)
   {
    Operation x = null;

    if (nextSymbol == Keyword.LPARSY)
     {
      x = arguments(scope, v, queue);
     }

    return x;
   }

   /**
    * '(' [ expression { ',' expression } ] ')'
    * @param scope Scope
    * @param v Vector
    * @param queue Vector
    * @return Operation
    */
   private Operation arguments(Scope scope, Vector v, Vector queue)
   {
    Operation root = new Operation(), next = null;

    root.operator = nextSymbol;

    matchKeyword(Keyword.LPARSY);

    follower.add(Keyword.RPARSY);
    follower.add(Keyword.COMMASY);

    if (nextSymbol != Keyword.RPARSY)
     {
      next = (root.left = new Operation());
      next.left = expression(scope, queue);
      while(nextSymbol == Keyword.COMMASY)
       {
        lookAhead();
        next = (next.right = new Operation());
        next.left = expression(scope, queue);
       }
     }

    matchKeyword(Keyword.RPARSY);

    if (v != null)
     traceOff(v);

    if (exceptionStack > 0)
     {
      if (next == null)
       next = (root.left = new Operation());
      else
       next = (next.right = new Operation());

      next = (next.left = new Operation());
      next.type = new Type();
      next.type.type = Keyword.VOIDSY;
     }

    follower.remove(follower.size() - 1);
    follower.remove(follower.size() - 1);

    return root;
   }

   /**
    * arguments | '.' ident argumentsOpt
    * @param base Operation
    * @param scope Scope
    * @param queue Vector
    */
   private void superSuffix(Operation base, Scope scope, Vector queue)
   {
    if (nextSymbol == Keyword.LPARSY)
     base.left.left = arguments(scope, null, queue);
    else
     {
      matchKeyword(Keyword.DOTSY);

      base = base.right = new Operation();
      base.left = new Operation();
      base.left.name = nextToken;
      matchKeyword(Keyword.IDENTSY);

      base.left.left = argumentsOpt(scope, null, queue);
     }
   }

  private Operation resolvePrefix(Operation root)
   {
	Operation next = root.left.left.left;
	
	if (root.operator == Keyword.LEAFSY)
	 {
	  //warning(next.operator, "reduce");
	  if (next.operator == Keyword.LITERALSY &&
		  root.right != null)
	   {
	    Operation prepost = root.right.left;
	    Operation s = prepost;
	    long val = next.code.charAt(0);
	    double fval = 0.;
	    boolean bval = next.code.charAt(0) == 'T';
	    String w = next.code.substring(0, next.code.indexOf(' '));
	
	    if (next.type.type == Keyword.DOUBLESY)
	     fval = Double.parseDouble(w);
	    else if (next.type.type != Keyword.NONESY &&
			     next.type.type != Keyword.BOOLEANSY)
	     val = Long.parseLong(w);
	    else if (next.type.type != Keyword.BOOLEANSY)
	     prepost = null;
	
	    while(prepost != null)
	     {
		  s = null;

	      if (prepost.left.operator == Keyword.CASTSY)
	       {
	        if (next.type.type == Keyword.DOUBLESY)
	         val = (long)fval;
	        else
	         fval = val;
	      
	        next.type.type = prepost.left.type.type;
	       }
	      else if (prepost.left.operator == Keyword.NOTSY)
	       {
	        bval = !bval;
	       }
	      else if (prepost.left.operator == Keyword.INVERTSY)
	       {
	        val = ~val;
	       }
	      else if (prepost.left.operator == Keyword.MINUSSY)
	       {
	        val = -val;
	        fval = -fval;
	       }
	      prepost = prepost.right;
	     }
	    	  
        if (next.type.type == Keyword.BYTESY ||
    	    next.type.type == Keyword.SHORTSY ||
    	    next.type.type == Keyword.INTSY ||
    	    next.type.type == Keyword.CHARSY)
         next.code = (int)val + " ";
		    
        if (next.type.type == Keyword.LONGSY)
         {
          next.code = val + " ";
          if (Math.abs(val) <= Integer.MAX_VALUE)
           next.code += "S>D ";
         }
		    
        if (next.type.type == Keyword.FLOATSY ||
    	    next.type.type == Keyword.DOUBLESY)
         {
          next.code = fval + " ";
          next.type.type = Keyword.DOUBLESY;
         }
		    
        if (next.type.type == Keyword.BOOLEANSY)
         next.code = bval?"TRUE ":"FALSE ";
      
        root.right.left = s;
   	   }
	 }
	
	return root;
   }
  
  private void literal(Operation node)
   {
    node.operator = Keyword.LITERALSY;
    
    if (nextSymbol == Keyword.CHARCONSTSY)
     {
      node.code = (int)nextToken.val + " ";
      node.type.type = Keyword.CHARSY;
     }
    else if (nextSymbol == Keyword.NUMBERSY)
     {
      node.code = (int)nextToken.val + " ";
      node.type.type = Keyword.INTSY;
     }
    else if (nextSymbol == Keyword.LNUMBERSY)
     {
      node.code = nextToken.val + " ";
      if (Math.abs(nextToken.val) <= Integer.MAX_VALUE)
       node.code += "S>D ";
      node.type.type = Keyword.LONGSY;
     }
    else if (nextSymbol == Keyword.STRINGSY)
     {
      StringBuffer b = new StringBuffer();
      for(int i = 0; i < nextToken.string.length(); i++)
       {
        char a = nextToken.string.charAt(i);
        if (a == '"')
         b.append('\\');
        b.append(a);
       }
      node.code = "U\" " + b + " \" COUNT ";
      node.type.type = Keyword.NONESY;
      node.type.ident = new Token();
      node.type.ident.kind = Keyword.IDENTSY;
      node.type.ident.string = "String";

      unresolved.add(node.type.ident.string);
      unresolved.add("JavaArray");
     }
    else if (nextSymbol == Keyword.DNUMBERSY)
     {
      node.code = nextToken.fval + " ";
      node.type.type = Keyword.DOUBLESY;
     }
    else if (nextSymbol == Keyword.FALSESY)
     {
      node.code = "FALSE ";
      node.type.type = Keyword.BOOLEANSY;
     }
    else if (nextSymbol == Keyword.TRUESY)
     {
      node.code = "TRUE ";
      node.type.type = Keyword.BOOLEANSY;
     }
    else if (nextSymbol == Keyword.NULLSY)
     {
      node.code = "0 ";
      node.type.type = Keyword.NONESY;
      node.type.ident = new Token();
      node.type.ident.string = "Object";
      node.type.ident.kind = Keyword.IDENTSY;
      unresolved.add(node.type.ident.string);
     }
    else
     {
      error("literal expected, found " + nextSymbol.string + " named " + nextToken.string);
     }

    lookAhead();
   }

   /**
    * basicType arrayCreatorRest | qualident ( arrayCreatorRest | classCreatorRest )
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation creator(Scope scope, Vector queue)
   {
    Operation root = null;
    Type t = new Type();

    if ((t.type = basicType()) != Keyword.NONESY)
     {
      root = arrayCreatorRest(t, scope, queue);
     }
    else
     {
      Token x = nextToken;

      t.ident = new Token(nextToken);
      t.ident.string = x.string = qualident();
      unresolved.add(x.string);

      if (nextSymbol == Keyword.LBRACKETSY)
       {
        root = arrayCreatorRest(t, scope, queue);
       }
      else
       {
        root = classCreatorRest(x, scope, queue);
       }
     }

    return root;
   }

   /**
    * '[' ( ']' bracketsOpt arrayInitializer | expression ']' { '[' expression ']' } bracketsOpt )
    * @param t Type
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation arrayCreatorRest(Type t, Scope scope, Vector queue)
   {
    Operation root = null;

    matchKeyword(Keyword.LBRACKETSY);
    unresolved.add("JavaArray");

    t.dim++;

    if (nextSymbol == Keyword.RBRACKETSY)
     {
      lookAhead();

      t.dim += bracketsOpt();

      root = arrayInitializer(t, t.dim, scope, queue);

      root.type = new Type();
      root.type.ident = t.ident;
      root.type.type = t.type;
      root.type.dim = t.dim;
     }
    else
     {
      root = new Operation();
      root.type = new Type(t);

      follower.add(Keyword.RBRACKETSY);
      Operation next = (root.left = new Operation());
      next.operator = Keyword.LBRACKETSY;
      next.left = expression(scope, queue);
      matchKeyword(Keyword.RBRACKETSY);

      while(nonEmptyBracket())
       {
        matchKeyword(Keyword.LBRACKETSY);
        root.type.dim++;
        next = (next.right = new Operation());
        next.operator = Keyword.LBRACKETSY;
        next.left = expression(scope, queue);
        matchKeyword(Keyword.RBRACKETSY);
       }

      while(emptyBracket())
       {
        matchKeyword(Keyword.LBRACKETSY);
        root.type.dim++;
        matchKeyword(Keyword.RBRACKETSY);
       }

      follower.remove(follower.size() - 1);
     }

    return root;
   }

   /**
    * arguments [ classBody ]
    * @param t Token
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation classCreatorRest(Token t, Scope scope, Vector queue)
   {
    Vector v = new Vector();
    String s = "super";
    Operation root = new Operation();

    root.type.type = Keyword.NONESY;
    root.type.ident = t;
    traceOn(v);
    root.left = arguments(scope, v, queue);
    traceOff(v);

    for(int i = 0; i < v.size(); i++)
     {
      Token pt = (Token)v.get(i);

      if (pt.kind == Keyword.NUMBERSY)
       s += pt.val;
      else if (pt.kind == Keyword.LNUMBERSY)
       s += pt.val + "L";
      else if (pt.kind == Keyword.DNUMBERSY)
       s += pt.fval;
      else if (pt.kind == Keyword.IDENTSY)
       s += pt.string;
      else
       s += pt.kind.string;
     }
    if (s.endsWith("()"))
     s = "";
    else
     s += ';';

    if (nextSymbol == Keyword.LBRACESY)
     {
      ClassType x = new ClassType();
      x.name = new Token();
      x.name.kind = Keyword.IDENTSY;
      x.name.string = "Anonymous" + anonymous++;
      x.scope = new Scope(scope, Scope.classed, x.name.string);
      x.extend = new ClassType(t.string.substring(t.string.lastIndexOf('.') + 1));
      x.implement = new ClassType[1];
      x.implement[0] = x.extend;
      declMember(scope, x);

      if (!scopeStack.contains(scope))
       scopeStack.add(scope);

      root.type.ident = x.name;
      root.type.type = Keyword.NONESY;
      root.left.left = null;
      Vector old = queue;
      queue = new Vector();
      HashSet dummy = unresolved;
      unresolved = x.unresolved;
      unresolved.add(t.string);
      classBody(x, new Modifier(), s, queue);
      addToConstructor(x, queue);
      queue = old;
      unresolved = dummy;
      writeList(x);
     }

    return root;
   }

  private void writeList(ClassType x)
   {
    if (depth > 0)
     {
      x.statics = null;

      for(Iterator iter = x.scope.iterator(); iter.hasNext();)
       {
        Basic b = (Basic)iter.next();
        if (b instanceof MethodType)
         ((MethodType)b).operation = null;
       }
     }
/*    else
     {
      String trunc = scanner.sourceName.substring(0, scanner.sourceName.lastIndexOf('\\') + 1);
      x.write(trunc + x.version);

      for(Iterator iter = x.scope.iterator(); iter.hasNext();)
       {
        Basic b = (Basic)iter.next();
        if (b instanceof MethodType && ((MethodType)b).operation.size() > 0)
         ((MethodType)b).write(trunc + x.name.string + b.name.string + b.version);
       }
     }*/
   }

   /**
    * '[' ']' BracketsOpt '.' "class"
    * | ArgumentsOpt
    * | '.' ("class" | "this")
    * @param root Operation
    * @param next Operation
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation identifierSuffix(Operation root, Operation next, Scope scope, Vector queue)
   {
    if (nextSymbol == Keyword.LBRACKETSY)
     {
      lookAhead();
      lookAhead();
      unresolved.add("JavaArray");

      bracketsOpt();

      matchKeyword(Keyword.DOTSY);

      matchKeyword(Keyword.CLASSSY);

      next.left.operator = Keyword.CLASSSY;
     }
    else if (nextSymbol == Keyword.DOTSY)
     {
      lookAhead();

      if (nextSymbol == Keyword.CLASSSY)
       {
        lookAhead();
        next.left.operator = Keyword.CLASSSY;
       }
      else
       {
        next = (next.right = new Operation());
        next.left = new Operation();
        next.left.operator = nextSymbol;
        matchKeyword(Keyword.THISSY);
       }
     }
    else
     {
      next.left.left = argumentsOpt(scope, null, queue);
     }

    return root;
   }

   /**
    * '.' ( ident ArgumentsOpt
    * | "super" Arguments
    * | "new" InnerCreator
    * )
    * | '[' Expression ']'
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation selector(Scope scope, Vector queue)
   {
    Operation root;

    root = new Operation();

    if (nextSymbol == Keyword.DOTSY)
     {
      lookAhead();

      root.operator = nextSymbol;

      if (nextSymbol == Keyword.IDENTSY)
       {
        root.name = nextToken;
        lookAhead();
        root.left = argumentsOpt(scope, null, queue);
       }
      else if (nextSymbol == Keyword.SUPERSY)
       {
        matchKeyword(Keyword.SUPERSY);
        root.left = arguments(scope, null, queue);
       }
      else
       {
        matchKeyword(Keyword.NEWSY);
        root.left = innerCreator(scope, queue);
       }
     }
    else
     {
      root.operator = nextSymbol;
      matchKeyword(Keyword.LBRACKETSY);
      follower.add(Keyword.RBRACKETSY);
      unresolved.add("JavaArray");
      root.left = expression(scope, queue);
      follower.remove(follower.size() - 1);
      matchKeyword(Keyword.RBRACKETSY);
     }

    return root;
   }

   /**
    * ident classCreatorRest
    * @param scope Scope
    * @param queue Vector
    * @return Operation
    */
   private Operation innerCreator(Scope scope, Vector queue)
   {
    Token t = nextToken;

    matchKeyword(Keyword.IDENTSY);

    return classCreatorRest(t, scope, queue);
   }

   /**
    * expression
    * @param scope Scope
    * @param t Type
    * @param save boolean
    * @param queue Vector
    * @return Operation
    */
   private Operation castExpression(Scope scope, Type t, boolean save, Vector queue)
   {
    Operation next, root = new Operation();

    if (save)
     queue.add(root);
    root.operator = Keyword.LEAFSY;
    next = (root.left = new Operation());
    next = (next.left = new Operation());
    next = (next.left = new Operation());
    next.operator = Keyword.LPARSY;

    next.left = expression(scope, queue);

    next = (root.right = new Operation());
    next = (next.left = new Operation());
    next = (next.left = new Operation());
    next.operator = Keyword.CASTSY;
    next.type = t;

    return root;
   }

   /**
    * '(' castExpression ')'
    * @param scope Scope
    * @param queue Vector
    */
   private Operation parExpression(Scope scope, Vector queue)
   {
    matchKeyword(Keyword.LPARSY);

    Type t = new Type();
    t.type = Keyword.BOOLEANSY;

    follower.add(Keyword.RPARSY);
    Operation root = castExpression(scope, t, true, queue);
    follower.remove(follower.size() - 1);
    matchKeyword(Keyword.RPARSY);
    
    root = root.left.left.left;
    if (root.operator == Keyword.LPARSY &&
    	root.left.operator == Keyword.LEAFSY)
     root = root.left.left.left.left;
    
    return root;
   }

  private Keyword assignmentOperator()
   {
    Keyword op = nextSymbol;

    if (assign.contains(nextSymbol))
     lookAhead();
    else
     error("assign operator expected");

    return op;
   }

  private Keyword prefixOp()
   {
    Keyword op = nextSymbol;

    if (prefix.contains(op))
     lookAhead();
    else
     error("prefix operator expected");

    return op;
   }

  private Keyword postfixOp()
   {
    Keyword op = null;

    if (postfix.contains(nextSymbol))
     {
      op = nextSymbol;
      lookAhead();
     }

    return op;
   }

  private void declMember(Scope scope, Basic member)
   {
    if (scope.get(member.name.string).length != 0)
     error(member.name.string + " already declared");
    else
     scope.declInsertOverload(member);
   }

  private void declInsert(Scope scope, Basic member)
   {
    boolean present = false;

    if (member instanceof VariableType)
     for(Scope help = scope; help != null && !present && (help.prefix == null || help.prefix.length() == 0); help = help.prev)
      present = help.get(member.name.string).length != 0;

    if (present)
     error(member.name.string + " already declared, overloading inhibited");
    else
     scope.declInsertOverload(member);
   }

  private Operation getLiteral(Operation root)
   {
	Operation r = null;
	
	if (root != null && root.operator == Keyword.LEAFSY &&
	    root.left.left.left.operator == Keyword.LITERALSY)
	 r = root.left.left.left;
	
	return r;
   }
  
   class Modifier
    {
     int none, access, classes, fields, methods, constructors, interfaces, constants;

     public int cur;

     public Modifier()
      {
       access = Keyword.PUBLICSY.value | Keyword.PROTECTEDSY.value | Keyword.PRIVATESY.value;

       cur = 0;

       none = 0;
       classes = (Keyword.ABSTRACTSY.value | Keyword.STATICSY.value | Keyword.FINALSY.value | Keyword.STRICTFPSY.value);
       fields = (Keyword.STATICSY.value | Keyword.FINALSY.value | Keyword.TRANSIENTSY.value | Keyword.VOLATILESY.value);
       methods = (Keyword.ABSTRACTSY.value | Keyword.STATICSY.value | Keyword.FINALSY.value | Keyword.SYNCHRONIZEDSY.value |
                  Keyword.NATIVESY.value | Keyword.STRICTFPSY.value);
       constructors = Keyword.PUBLICSY.value | Keyword.PROTECTEDSY.value | Keyword.PRIVATESY.value;
       interfaces = (Keyword.ABSTRACTSY.value | Keyword.STATICSY.value | Keyword.STRICTFPSY.value);
       constants = (Keyword.PUBLICSY.value | Keyword.STATICSY.value | Keyword.FINALSY.value);
      }

     Modifier(Modifier m)
      {
       classes = m.classes;
       fields = m.fields;
       methods = m.methods;
       interfaces = m.interfaces;
       constants = m.constants;
       cur = m.cur;
       none = m.none;
       access = m.access;
       fields = m.fields;
       methods = m.methods;
       constructors = m.constructors;
       interfaces = m.interfaces;
       constants = m.constants;
      }

     public void add (int m)
      {
       if ((cur & m) == 0) cur |= m;
        else error("repeated modifier " + toString(m));
      }

     public void check(int allowed)
      {
       int wrong = cur & ~allowed;

       if (wrong != none)
        error("modifier(s) " + toString(wrong) + "not allowed here");
      }

     public String toString(int m)
      {
       String s = "";

       if ((m & Keyword.PUBLICSY.value) != 0) s += "public ";
       if ((m & Keyword.PRIVATESY.value) != 0) s += "private ";
       if ((m & Keyword.PROTECTEDSY.value) != 0) s += "protected ";
       if ((m & Keyword.STATICSY.value) != 0) s += "static ";
       if ((m & Keyword.FINALSY.value) != 0) s += "final ";
       if ((m & Keyword.SYNCHRONIZEDSY.value) != 0) s += "synchronized ";
       if ((m & Keyword.VOLATILESY.value) != 0) s += "volatile ";
       if ((m & Keyword.TRANSIENTSY.value) != 0) s += "transient ";
       if ((m & Keyword.NATIVESY.value) != 0) s += "native ";
       if ((m & Keyword.ABSTRACTSY.value) != 0) s += "abstract ";
       if ((m & Keyword.STRICTFPSY.value) != 0) s += "strictfp ";

       return s;
      }
    }

 }
