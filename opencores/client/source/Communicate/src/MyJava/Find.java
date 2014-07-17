package MyJava;

import java.util.Iterator;
import java.util.Vector;

public class Find
 {
  private static Header[] cross;
  private static Pass pass = null;
  private static final Keyword [] hierarchy = { Keyword.BYTESY, Keyword.CHARSY, Keyword.SHORTSY, Keyword.INTSY, Keyword.LONGSY, Keyword.FLOATSY, Keyword.DOUBLESY };
  private static final int [] hierarchyVal = { 0, 1, 1, 2, 3, 4, 4 };
  public static Scope theScope = null;

  public static String source = "";
  public static int line = 0;
  public static int col = 0;

  /**
   * note Pass object for retrieval
   * @param x the object
   */
  static void notePass(Pass x)
   {
    pass = x;
   }

  /**
   * assign list of loaded headers
   * @param list Header[]
   */
  static void setCrossreference(Header [] list)
   {
    cross = list;
    //insert = "";
   }

  /**
   * generate code referencing a class member
   * @param offset
   * @return code
   */
  public static String addOffset(int offset)
   {
    return offset != 0?offset + " + ":"";
   }

  /**
   * set position of currently processed token
   * @param t Token
   */
  public static void setPosition(Token t)
   {
    if (t != null)
     {
      line = t.line;
      col = t.col;
      source = t.source;
     }
   }

  /**
   * write error message
   * @param msg String
   */
  public static void error(String msg)
   {
    Errors.println(source, line, col, msg);
   }

  /**
   * write error message
   * @param msg String
   */
  public static void warning(String msg)
   {
    Errors.warning(source, line, col, msg, false);
   }

  /**
   * find class name in crossreference list
   * @param name String name of class
   * @param version int hashcode of class
   * @param scope Scope favorite scope to look for
   * @return ClassType[] list of found classes
   */
  public static ClassType [] findClass(String name, int version, Scope scope)
   {
    Vector x = new Vector();
    // search possible candidates
    Scope [] s = find(name, scope, true);

    // extract classes only
    for(int i = 0; i < s.length; i++)
     {
      Basic [] b = s[i].get(name);

      for(int j = 0; j < b.length; j++)
       if (b[j] instanceof ClassType)
        if (((b[j].version ^ version) & -8) == 0 || version == 0)
         if (!x.contains(b[j]))
          x.add(b[j]);
     }

    return (ClassType [])x.toArray(new ClassType[x.size()]);
   }

  /**
   * find a name in the crossreference list
   * @param name String
   * @param scope Scope favorite scope
   * @param isClass boolean name identifies a class
   * @return Scope[] list of scopes, each containing name
   */
  public static Scope [] find(String name, Scope scope, boolean isClass)
   {
    String tail = name.substring(name.lastIndexOf('.') + 1);
    // in favorite scope
    Scope [] result = scope.locate(name, 0, isClass);

    for(int k = 0; result.length == 0 && k < 2;
        cross = (k == 0 && result.length == 0 && pass != null)?pass.addHeader(cross, tail):cross, k++)
     { // crossreference list
      Vector v = new Vector();
      for(int i = 0; i < cross.length; i++)
       {
        Scope [] x = cross[i].base.locate(name, 0, isClass);

        for(int j = 0; j < x.length; j++)
         if (!v.contains(x[j]))
          v.add(x[j]);
       }

      result = (Scope [])v.toArray(new Scope[v.size()]);
     }

    return result;
   }

  /**
   * find class name in crossreference list
   * @param name String name of class
   * @param version int hashcode of class
   * @return ClassType[] list of found classes
   */
  static ClassType [] findClass(String name, int version)
   {
    return Find.findClass(name, version, theScope);
   }

  /**
   * find method name in crossreference list
   * @param name String name of method
   * @param res Type type of method
   * @param types Type[] parameter types
   * @param classes ClassType[] classes to search for
   * @return MethodType[] list of matching methods
   */
  public static MethodType [] getMethod(String name, Type res, Type [] types, ClassType [] classes)
   {
    Vector v = new Vector();
    int max = 0;
    name = name.substring(name.lastIndexOf('.') + 1);

    for(int i = 0; i < classes.length; i++)
     {
      for(Iterator iter = classes[i].scope.iterator(); iter.hasNext();)
       {
        Basic x = (Basic)iter.next();
        Vector w = new Vector();
        // match type of method
        if (x instanceof MethodType && (name.endsWith("." + x.name.string) || name.compareTo(x.name.string) == 0) && (res == null || ((MethodType)x).type.match(res)))
         {
          int start = 0, a = 0, b = 0;
          MethodType z = (MethodType)x;

          if (z.parameter.length > 0)
           {
            start = (z.parameter[0].name.string.compareTo("§this") == 0 || z.parameter[0].name.string.compareTo("§outer") == 0) ? 1 : 0;
            a = z.parameter[z.parameter.length - 1].type.type == Keyword.VOIDSY?1:0;
           }
          // match parameters
          if (types != null && types.length > 0)
           b = (types[types.length - 1].type == Keyword.VOIDSY)?1:0;

          int count = 0;
          if (types != null)
           if ((z.parameter.length - start - a) != (types.length - b))
            z = null;
           else
            for(int l = 0; l < types.length - b; l++)
             if (types[l].match(z.parameter[l + start].type))
              count++;
             else if (!(types[l].dim == 0 && z.parameter[l + start].type.dim == 0 &&
                      Parser.typeKW.contains(types[l].type) && types[l].type != Keyword.BOOLEANSY &&
                      Parser.typeKW.contains(z.parameter[l + start].type.type) && z.parameter[l + start].type.type != Keyword.BOOLEANSY ||
                	  isInterface(types[l], z.parameter[l + start].type) ||
                      isSuperclass(types[l], z.parameter[l + start].type)))
              {
               z = null;
               break;
              }
          // accept only, if match is not lower than best match made
          if (z != null && count >= max)
           {
            if (count > max)
             {
              v = new Vector();
              w = new Vector();
              max = count;
             }
            w.add(z);
           }
         }
        while(w.size() > 0)
         {
          MethodType u = (MethodType)w.get(0);
          w.remove(0);
          if (!v.contains(u))
           v.add(u);
         }
       }
     }

    int high = 0, low = Integer.MAX_VALUE;
    Vector w = new Vector();
    // select closest methods
    for(int i = 0; i < v.size(); i++)
     {
      MethodType m = (MethodType)v.get(i);
      int h = 0, l = 0;
      if (m.parameter.length > 0 && types != null)
       for(int k = 0, j = (m.parameter[0].name.string.compareTo("§this") == 0 || m.parameter[0].name.string.compareTo("§outer") == 0) ? 1 : 0;
           j < m.parameter.length && k < types.length; j++, k++)
        if (m.parameter[j].type.type != Keyword.NONESY && m.parameter[j].type.type != Keyword.BOOLEANSY && types[k].type != Keyword.VOIDSY)
         {
          int x = 0, y = 0;
          while(hierarchy[x] != m.parameter[j].type.type)
           x++;
          while(hierarchy[y] != types[k].type)
           y++;
          if (hierarchyVal[x] >= hierarchyVal[y])
           h++;
          else
           l++;
         }
      
      if (h >= high && l <= low)
       {
        if (h > high || l < low)
         w.clear();
        w.add(m);
        high = h;
        low = l;
       }
     }

    return (MethodType [])w.toArray(new MethodType[w.size()]);
   }

  /**
   * check, if derived is a superclass of given
   * @param given Type
   * @param derived Type
   * @return boolean
   */
  public static boolean isSuperclass(Type given, Type derived)
   {
    ClassType [] s;
    if (given.match(derived))
     return true;
    
    // Object matches every class and array
    if (given.type == Keyword.NONESY && given.ident.string.compareTo("Object") == 0)
     if (derived.dim > 0 || derived.type == Keyword.NONESY)
      return true;
     
    // looking for superclass?
    if (derived.type != Keyword.NONESY)
     return false;
    // Object is superclass of any array, and any object
    if (given.dim != derived.dim)
     if (derived.type != Keyword.NONESY || derived.ident.string.compareTo("Object") != 0 || derived.dim != 0)
      return false;
    // retrieve superclass
    if (given.type != Keyword.NONESY)
     if (given.dim > 0)
      s = findClass("JavaArray", 0);
     else
      return false;
    else if (derived.ident.string.compareTo("Object") == 0 && (derived.dim == 0 || derived.dim == given.dim))
        return true;
    else
     s = findClass(given.ident.string, given.version);

    for(int k = 0; k < s.length; k++)
     for (ClassType r = s[k].extend; r != null && r.name != null && r.scope != null; r = r.extend)
      if (r.name.string.compareTo(derived.ident.string) == 0)
      {
       //warning(r.toString() + derived.toString());
       return true;
      }
    /*{
       String path = r.scope.buildPath("");
       if (path.endsWith("." + derived.ident.string) || path.compareTo(derived.ident.string) == 0)
        return true;
      }*/

    if (derived.ident.string.compareTo("Object") == 0)
     for(int k = 0; k < s.length; k++)
      if ((s[k].modify & Keyword.ABSTRACTSY.value) != 0)
       {
        //warning(s[k].toString() + derived.toString());
        return true;
       }

    return false;
   }

  /**
   * check, if given is an implementation of set
   * @param given Type
   * @param set Type
   * @return boolean
   */
  public static boolean isInterface(Type given, Type set)
   {
    if (given.match(set))
     return true;

    if (given.dim != set.dim || given.type != Keyword.NONESY || set.type != Keyword.NONESY)
     return false;

    ClassType [] s = findClass(set.ident.string, set.version);

    Vector x = new Vector();

    for(int i = 0; i < s.length; i++)
     if ((s[i].modify & Keyword.INTERFACESY.value) != 0)
      x.add(s[i]);

    s = findClass(given.ident.string, given.version);

    while(x.size() > 0)
     {
      ClassType interfaces = (ClassType)x.get(0);
      x.remove(0);

      for(int i = 0; i < s.length; i++)
       for(ClassType c = s[i]; c.name != null; c = c.extend)
        for(int k = 0; k < c.implement.length; k++)
         if (isImplemented(c.implement[k], interfaces.name.string))
          return true;
     }
    
    return false;
   }

  public static boolean isImplemented(ClassType inter, String name)
   {
    if (inter.name.string.compareTo(name) == 0)
     return true;

    if (inter.implement != null)
     for(int i = 0; i < inter.implement.length; i++)
      if (isImplemented(inter.implement[i], name))
       return true;

    return false;
   }

  /**
   * check, if given is an inner class of desired
   * @param given Type
   * @param desired Type
   * @return String access path
   */
  public static String isOuter(Type given, Type desired)
   {
    String x = "";

    if (given.type != Keyword.NONESY)
     return x;

    ClassType [] c = findClass(given.ident.string, given.version);

    if (c.length != 1)
     {
      if (c.length != 0)
       error("type " + given.toString() + " ambiguous");

      return x;
     }

    given.version = c[0].version;

    while(c[0] != null && !isSuperclass(given, desired))
     {
      x += "CELL+ @ " + addOffset(Code3color.outer) + "@ ";
      Scope y = c[0].scope.prev;
      c[0] = null;
      while(y != null && (y.prefix == null || y.prefix.length() == 0))
       y = y.prev;

      if (y != null && y.prev != null)
       for(Iterator iter = y.prev.iterator(); iter.hasNext() && c[0] == null;)
        {
         Basic b = (Basic)iter.next();
         if (b instanceof ClassType && ((ClassType)b).scope == y)
          {
           c[0] = (ClassType)b;
           given = new Type(given);
           given.ident = c[0].name;
           given.version = c[0].version;
          }
        }
     }

    if (x.length() == 0)
     x = " ";

    return c[0] != null?x:"";
   }

  /**
   * check, if operand a literal
   * @param root Operation operand
   * @return Operation operand or null
   */
  public static Operation isLiteral(Operation root)
   {
	if (root != null)
     if (root.operator != Keyword.LEAFSY || (root = root.left.left.left) == null ||
         root.operator != Keyword.LITERALSY ||
         root.type.type != Keyword.INTSY && root.type.type != Keyword.SHORTSY &&
         root.type.type != Keyword.LONGSY && root.type.type != Keyword.CHARSY &&
         root.type.type != Keyword.DOUBLESY && root.type.type != Keyword.FLOATSY &&
         !(root.type.type == Keyword.NONESY && (root.type.ident.string.compareTo("Object") == 0 ||
                                                root.type.ident.string.compareTo("String") == 0)))
      root = null;

    return root;
   }

  /**
   * get value of literal
   * @param root Operation literal
   * @return double
   */
  public static double getValue(Operation root)
   {
    double x;

    try
     {
      String y = root.code.trim();
      int z = y.indexOf(' ');
      x = Double.parseDouble(z >= 0?y.substring(0, z):y);
     }
    catch(NumberFormatException ex) { x = Math.E; }

    return x;
   }

  /**
   * get value of literal
   * @param root Operation literal
   * @return double
   */
  public static long getLong(Operation root)
   {
    long x;

    try
     {
      String y = root.code.trim();
      int z = y.indexOf(' ');
      x = Long.parseLong(z >= 0?y.substring(0, z):y);
     }
    catch(Exception ex) { x = Long.MIN_VALUE; }

    return x;
   }

   /**
    * get value of literal
    * @param root Operation
    * @return String
    */
  public static String getString(Operation root)
   {
    int start, end;
    String x = "";
    start = root.code.indexOf("U\" ");
    end = root.code.indexOf(" \" ");
    x = root.code.substring(start + 3, end);
    root.code = root.code.substring(end + 3);
    return x;
   }

  /**
   * transform operand to a literal
   * @param root Operation operand
   * @param code String
   */
  public static void toLiteral(Operation root, String code)
   {
    root.operator = Keyword.LEAFSY;
    root.right = null;

    Operation node = root.left = new Operation();
    node = node.left = new Operation();
    node = node.left = new Operation();
    node.operator = Keyword.LITERALSY;
    node.type = root.type;
    node.code = new String(code);
   }

  public static int crc(Type t)
   {
    int x;

    if (t.type != Keyword.NONESY)
     if (t.type == Keyword.BYTESY)
      x = 1 << 16;
     else if (t.type == Keyword.CHARSY || t.type == Keyword.SHORTSY)
      x = 2 << 16;
     else if (t.type == Keyword.BOOLEANSY)
      x = 3 << 16;
     else if (t.type == Keyword.INTSY)
      x = 4 << 16;
     else if (t.type == Keyword.LONGSY)
      x = 5 << 16;
     else if (t.type == Keyword.DOUBLESY || t.type == Keyword.FLOATSY)
      x = 6 << 16;
     else
      x = 0;
    else
     x = new Basic().crc16(-1, t.ident.string, 0) & 0xffff0000;

    return x + t.dim;
   }

  /**
   * generate code for fetching member
   * @param scope Scope
   * @param member Basic
   * @param isSuper boolean supress polymorphic search
   * @param prefix String leading code for polymorphic access
   * @return String code
   */
  public static String buildQualifiedName(Scope scope, Basic member, boolean isSuper, String prefix)
   {
    String x = member.name.string.substring(member.name.string.lastIndexOf('.') + 1) +
               (member.version != 0 && !(member instanceof VariableType) && (member.modify & Keyword.NATIVESY.value) == 0? ("§" + member.version) : "");

    if ((member.modify & Keyword.STATICSY.value) != 0 ||
   	 (member.modify & Keyword.NATIVESY.value) != 0 ||
   	 member instanceof ClassType)
     {
      x = scope.buildPath(x) + " ";
      
      x = x.substring(x.indexOf(".") + 1);
     }
    else if (member instanceof MethodType)
     return prefix.substring(prefix.indexOf('.') + 1) + " " + member.version + " " + (isSuper?"FALSE":"TRUE") + " ( " + scope.buildPath(x) + " ) EXECUTE-METHOD ";

    return x;
   }

 }
