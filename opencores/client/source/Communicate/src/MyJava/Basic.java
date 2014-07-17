package MyJava;

/**
 * <p>Überschrift: Klassen für die Datentypen von Java</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.util.*;
import java.io.*;

class Header
 {
  String name;                        // name of java sourcefile
  String [] imports;                  // import declarations
  String myPackage;                   // package
  Vector scopes;                      // starting scopes of classes
  Scope base;                         // root scope of scope tree
  final int depth;                    // priority: 0 ... sourcefile
  protected static Vector stack;

  /**
   * write header to file name
   * @param name String
   */
  void write(String name)
   {
    DataOutputStream x = null;
    try
     {
      x = new DataOutputStream(new FileOutputStream(name));
      x.writeBoolean(myPackage != null);
      if (myPackage != null)
       x.writeUTF(myPackage);
      x.writeInt(imports.length);

      for (int i = 0; i < imports.length; i++)
       x.writeUTF(imports[i]);

      base.serialize(x);
      x.close();
     }
    catch (Exception ex) { ex.printStackTrace(); }
   }

  /**
   * read header from file name
   * @param name String
   * @param source int source id
   */
  Header(String name, int source)
   {
    this.name = new File(name.substring(0, name.lastIndexOf('.') + 1) + "java").getAbsolutePath();
    this.depth = source;

    if (name.endsWith(".h"))
     {
      DataInputStream x = null;
      try
       {
        x = new DataInputStream(new FileInputStream(name));

        if (x.readBoolean())
         myPackage = x.readUTF();
        else
         myPackage = null;

        int dim = x.readInt();
        imports = new String[dim];

        for (int i = 0; i < dim; i++)
         imports[i] = x.readUTF();

        stack = new Vector();

        base = new Scope(null, Scope.classed, x.readUTF(), x.readUTF(), x);

        scopes = stack;
        stack = null;

        x.close();
       }
      catch (Exception ex) { ex.printStackTrace(); }
     }
   }

   /**
    * create header
    * @param jdk String jdk prefix
    * @param name String header name
    * @param myPackage String package prefix
    * @param source int source id
    */
   Header(String jdk, String name, String myPackage, int source)
   {
    scopes = new Vector();
    this.name = new File(name).getAbsolutePath();
    this.depth = source;

    String dummy = "";

    if (name.startsWith(jdk))
     {
      dummy = name.substring(jdk.length());
      dummy = dummy.substring(0, dummy.lastIndexOf('\\')).replace('\\', '.');
     }

    this.base = new Scope(null, Scope.classed, myPackage, dummy);
   }
 }

/**
 * base class holding name and modifiers only
*
 * <p>Überschrift: </p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright (c) 2005</p>
 * <p>Organisation: </p>
 * @author unbekannt
 * @version 1.0
 */
public class Basic
 {
  int modify;                     // modifier
  Token name;                     // name
  int version;                    // version number (overloading)

  /**
   * default constructor
   */
  Basic() { }

  /**
   * create with content from buffer
   * @param x DataInputStream
   */
  Basic(DataInputStream x)
   {
    try
     {
      modify = x.readInt();
      name = new Token();
      name.kind = Keyword.IDENTSY;
      name.string = x.readUTF();
      version = x.readInt();
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * serialize object
    * @param x DataOutputStream
    */
  void serialize(DataOutputStream x)
   {
    try
     {
      x.writeInt(modify);
      x.writeUTF(name.string);
      x.writeInt(version);
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * serialize a hashset
    * @param set HashSet
    * @return MyBuffer
    */
   void serialize(HashSet set, DataOutputStream x)
   {
    try
     {
      x.writeInt(set.size());

      Iterator iter = set.iterator();
      while(iter.hasNext())
       x.writeUTF((String)iter.next());
     }
     catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * create hashset from buffer
    * @param x DataInputStream
    * @return HashSet
    */
   HashSet read(DataInputStream x)
   {
    HashSet set = new HashSet();

    try
     {
      for (int i = x.readInt(); i > 0; i--)
       set.add(x.readUTF());
     }
    catch (IOException ex) { ex.printStackTrace(); }

    return set;
   }

   /**
    * calculate CRC 16
    * @param crc initial value
    * @param x String
    * @param modify int
    * @return int CRC 16 i upper half
    */
   int crc16(int crc, String x, int modify)
   {
    int flag = 0;
    int help;

    // extract relevant flags
    if ((modify & Keyword.CLASSSY.value) != 0)
     flag = 4;
    if ((modify & Keyword.PRIVATESY.value) != 0)
     flag |= 2;
    else if ((modify & Keyword.PROTECTEDSY.value) != 0)
     flag |= 1;

    for(int i = 0; i < x.length(); i++)
     for(int j = 0, input = x.charAt(i); j < 8; j++)
      {
       help = (crc ^ input) & 1;
       help = (help << 3) | (help << 10) | (help << 15);
       crc >>= 1;
       crc ^= help;
       input >>= 1;
      }

    return (crc << 16) | flag;
   }

   /**
    *
    * @return String
    */
   public String toString()
   {
    return name.string + "@" + version;
   }

   /**
    * convert modifiers into a string
    * @return String
    */
   public String modifier()
   {
    String x = "";
    for(int i = 65536 * 512; i != 0; i >>= 1)
     if ((modify & i) != 0)
      x += Keyword.get(i).string + ' ';

    return x;
   }

   /**
    * match against x
    * @param x Basic
    * @return boolean
    */
   boolean match(Basic x)
   {
    boolean flag = x.name.string.compareTo(name.string) == 0 || x.name.string.endsWith("." + name.string) || name.string.endsWith("." + x.name.string);
    return flag && (x.version == version || x.version == 0 || version == 0) && x.modify == modify;
   }
 }

class ClassType extends Basic
 {
  ClassType extend;              // superclass
  ClassType [] implement;        // abstract classes and interfaces
  Scope scope;                   // scope of class
  Vector statics;                // list of operator trees of static blocks
  String filename;
  HashSet unresolved;            // list of names of unresolved references
  String comment;                // trailing comment

  /**
   * default constructor
   */
  ClassType()
   {
    statics = new Vector();
    unresolved = new HashSet();
    modify = Keyword.CLASSSY.value;
    scope = null;
   }

   /**
    * constructor
    * @param name String name of class
    */
   ClassType(String name)
   {
    this.name = new Token();
    this.name.string = name;
    this.name.kind = Keyword.IDENTSY;
    modify = Keyword.CLASSSY.value;
    scope = null;
   }

   /**
    * constructor
    * @param scope Scope enclosing scope
    * @param x DataInputStream object is read from
    * @param base Basic super object
    */
   ClassType(Scope scope, DataInputStream x, Basic base)
   {
    this.modify = base.modify;
    this.name = base.name;
    this.version = base.version;

    String s = null;
    try
     {
      s = x.readUTF();
      extend = (s.length() > 0)?new ClassType(s):new ClassType();

      int dim1 = x.readInt();
      implement = new ClassType[dim1];

      for(int i = 0; i < dim1; i++)
       implement[i] = new ClassType(x.readUTF());

      unresolved = read(x);

      this.scope = new Scope(scope, (modify & Keyword.STATICSY.value) != 0?Scope.heap:Scope.classed, x.readUTF(), x.readUTF(), x);
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * serialize object
    * @param x DataOutputStream
    */
  void serialize(DataOutputStream x)
   {
    try
     {
      super.serialize(x);
      x.writeUTF(extend.name != null ? extend.name.string : "");

      x.writeInt(implement.length);
      for (int i = 0; i < implement.length; i++)
       x.writeUTF(implement[i].name.string);

      serialize(unresolved, x);
      scope.serialize(x);
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * check, if empty
    * @return boolean true, if no enclosing scope or name givven
    */
   boolean isEmpty()
   {
    return scope == null || name == null;
   }

  public String toString()
   {
    return modifier() + ((modify & Keyword.INTERFACESY.value) != 0?"interface ":"class ") + super.toString();
   }

   /**
    * save object to file name
    * @param name String
    */
   void write(String name)
   {
    try
     {
      File f;
      DataOutputStream x = new DataOutputStream(new FileOutputStream(f = new File(name + ".tmp")));
      filename = f.getPath();

      while (statics.size() > 0)
       {
        ((Operation)statics.get(0)).serialize(x);
        statics.remove(0);
       }

      x.close();

      statics = null;
     }
    catch (Exception ex) { ex.printStackTrace(); }
   }

   /**
    * read object from filename
    * @return Vector
    */
   Vector read()
   {
    Vector v = new Vector();
    File f;

    if (filename != null)
     {
      try
       {
        DataInputStream x = new DataInputStream(new FileInputStream(f = new File(filename)));
        Operation op = new Operation(x);
        while (x.available() > 0)
         {
          v.add(op);
          op = new Operation(x);
         }
        x.close();
        f.delete();
        filename = null;
       }
      catch (Exception ex) { ex.printStackTrace(); }
     }

    return v;
   }

   /**
    * hashcode of object
    * @return int
    */
   public int hashCode()
   {
    String name = this.name.string.substring(this.name.string.lastIndexOf('.') + 1);
    String x = name;

    return crc16(-1, x, 0) | ((name.charAt(0) << 8) & 0xff00);
   }
 }

class VariableType extends Basic
 {
  int offset;                    // relative position in object, for class variables
  boolean referenced = false;    // needed in code generation
  Type type;                     // type of variable

  /**
   * constructor
   * @param type Type
   * @param modify int modifiers
   */
  VariableType(Type type, int modify)
   {
    this.modify = modify | Keyword.VARIABLESY.value;
    this.type = type;
   }

   /**
    * constructor
    * @param x DataInputStream object is read from
    * @param base Basic super object
    */
   VariableType(DataInputStream x, Basic base)
   {
    try
     {
      this.modify = base.modify;
      this.name = base.name;
      this.version = base.version;

      offset = x.readInt();

      type = new Type(x);
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * serialize object
    * @param y DataOutputStream
    */
   void serialize(DataOutputStream y)
   {
    try
     {
      super.serialize(y);
      y.writeInt(offset);
      type.serialize(y);
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * hashcode of object
    * @return int
    */
   public int hashCode()
   {
    String name = this.name.string.substring(this.name.string.lastIndexOf('.') + 1);
    String x = type.toString() + name;
//if (name.length() == 0) { System.out.println(this.name.string); System.exit(0);}
    return crc16(-2, x, 0) | ((name.charAt(0) << 8) & 0xff00);
   }

  public String toString()
   {
    return modifier() + type.toString() + ' ' + super.toString();
   }

   /**
    * match against x
    * @param x VariableType
    * @return boolean
    */
   boolean match(VariableType x)
   {
    return x.type.match(type) && super.match(x);
   }
 }

class MethodType extends Basic
 {
  Type type;                 // type of result
  Parameter [] parameter;    // list of parameter
  String [] throwing;        // list of exceptions
  Scope scope;               // scope of method
  Vector operation;          // list of operator trees
  String filename;
  String comment;            // trailing comment

  /**
   * constructor
   * @param modify int modifiers
   */
  MethodType(int modify)
   {
    operation = new Vector();
    this.modify = modify | Keyword.METHODSY.value;
   }

   /**
    * constructor
    * @param x DataInputStream object read from
    * @param base Basic super object
    */
   MethodType(DataInputStream x, Basic base)
   {
    try
     {
      this.modify = base.modify;
      this.name = base.name;
      this.version = base.version;
      type = new Type(x);

      int dim1 = x.readInt();
      parameter = new Parameter[dim1];
      for (int i = 0; i < dim1; i++)
       parameter[i] = new Parameter(x);

      dim1 = x.readInt();
      throwing = new String[dim1];
      for (int i = 0; i < dim1; i++)
       throwing[i] = x.readUTF();

      scope = new Scope(Scope.cur, Scope.automatic, "");
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * serialize object
    * @param y DataOutputStream
    */
   void serialize(DataOutputStream y)
   {
    try
     {
      super.serialize(y);
      type.serialize(y);

      y.writeInt(parameter.length);
      for (int i = 0; i < parameter.length; i++)
       parameter[i].serialize(y);

      y.writeInt(throwing.length);
      for (int i = 0; i < throwing.length; i++)
       y.writeUTF(throwing[i]);
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * hashcode of object
    * @return int
    */
   public int hashCode()
   {
    String name = this.name.string.substring(this.name.string.lastIndexOf('.') + 1);
    String x = type.toString() + name + '(';

    if (parameter.length > 0)
     for(int i = parameter[0].name.string.compareTo("§this") == 0 || parameter[0].name.string.compareTo("§outer") == 0?1:0,
         end = parameter.length - (parameter[parameter.length - 1].type.type == Keyword.VOIDSY?1:0); i < end; i++)
      x += parameter[i].type.toString();

    return crc16(-3, x, 0) | ((name.charAt(0) << 8) & 0xff00);
   }

  public String toString()
   {
    String x = modifier() + type.toString() + ' ' + super.toString() + '(';
    for(int i = 0; i < parameter.length; i++)
     x += (i > 0?",":"") + parameter[i].type.toString();
    return x + ')';
   }

   /**
    * write operations to file name
    * @param name String
    */
   void write(String name)
   {
    try
     {
      File f;
      DataOutputStream x = new DataOutputStream(new FileOutputStream(f = new File(name + ".tmp")));
      filename = f.getPath();

      while (operation.size() > 0)
       {
        ((Operation)operation.get(0)).serialize(x);
        operation.remove(0);
       }

      x.close();

      operation = null;
     }
    catch (Exception ex) { ex.printStackTrace(); }
   }

   /**
    * read operations from filename
    * @return Vector
    */
   Vector read()
   {
    Vector v = new Vector();
    File f;

    if (filename != null)
     {
      try
       {
        DataInputStream x = new DataInputStream(new FileInputStream(f = new File(filename)));
        Operation op = new Operation(x);
        while (x.available() > 0)
         {
          v.add(op);
          op = new Operation(x);
         }
        x.close();
        f.delete();
        filename = null;
       }
      catch (Exception ex) { ex.printStackTrace(); }
     }

    return v;
   }

   /**
    * match against x
    * @param x MethodType
    * @return boolean
    */
   boolean match(MethodType x)
   {
    return x.type.match(type) && super.match(x);
   }
 }

class Type
 {
  Keyword type;
  Token ident;
  int version;
  int dim;
  int modify;

  Type() { type = Keyword.NONESY; ident = new Token(); version = 0; }

  /**
   * read object from x
   * @param x DataInputStream
   */
  Type(DataInputStream x)
   {
    try
     {
      type = Keyword.get(x.readInt());

      if (type == Keyword.NONESY)
       {
        ident = new Token();
        ident.string = x.readUTF();
        ident.kind = Keyword.IDENTSY;
       }

      version = x.readInt();
      dim = x.readInt();
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * serialize object
    * @param x DataOutputStream
    */
   void serialize(DataOutputStream x)
   {
    try
     {
      x.writeInt(type.value);

      if (type == Keyword.NONESY)
       x.writeUTF(ident.string);

      x.writeInt(version);
      x.writeInt(dim);
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * match against t
    * @param t Type
    * @return boolean
    */
   boolean match(Type t)
   {
    boolean flag = false;

    if (type == Keyword.NONESY && t.type == Keyword.NONESY)
     flag = t.ident.string.compareTo(ident.string) == 0 || t.ident.string.endsWith("." + ident.string) || ident.string.endsWith("." + t.ident.string);

    return t.dim == dim && t.type == type &&
           (((t.version ^ version) & -4) == 0 || (t.version & -8) == 0 || (version & -8) == 0) &&
           (type != Keyword.NONESY || flag);
   }

  public String toString()
   {
    String x = null;

    if (type != null)
     x = (type != Keyword.NONESY?type.string:(ident.string + " crc:" + version)) + " dimension:" + dim;

    return x;
   }

   /**
    * copy constructor
    * @param x Type
    */
   Type(Type x)
   {
    type = x.type;
    ident = x.ident != null?new Token(x.ident):null;
    version = x.version;
    dim = x.dim;
    modify = x.modify;
   }
 }

class Parameter
 {
  int modify;
  Type type;
  Token name;

  Parameter() { }

  /**
   * read object from x
   * @param x DataInputStream
   */
  Parameter(DataInputStream x)
   {
    try
     {
      modify = x.readInt();
      type = new Type(x);
      name = new Token();
      name.kind = Keyword.IDENTSY;
      name.string = x.readUTF();
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

   /**
    * serialize object
    * @param x DataOutputStream
    */
   void serialize(DataOutputStream x)
   {
    try
     {
      x.writeInt(modify);
      type.serialize(x);
      x.writeUTF(name.string);
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }
 }

class Scope
 {
  private static Vector table = new Vector();  // holds all scopes for indexing
  static final int automatic = 1,              // scope types
                   heap = 2,
                   classed = 3,
                   BLOCK = 1,
                   MAIN = 2,
                   LOOP = 3,
                   TRY = 4,
                   FINALLY = 5,
                   CATCH = 6,
                   SWITCH = 7,
                   SEQUENCE = 8,
                   DUMMY = 9,
                   SYNCHRONIZED = 10;
  final String prefix;                         // name of class, if starting scope
  int storageClass;                            // automatic, classed or heap for static scope
  final String trailer;                        // trailing path of scope
  int offset = 0;                              // offset for class variables
  int refcount = 0;                            // number of references
  private TreeMap map;                         // entries of scope
  private Vector follower;                     // scopes of inner classes
  Scope prev;                                  // enclosing scope
  Vector label;                                // list of labels (for jumps)
  int block;                                   // type of scope
  String exception = null;                     // for exception handling
  protected static Scope cur = null;           // for loading of header
  boolean continues = false;
  boolean breaks = false;
  
  /**
   * reset table of scopes
   */
  static void reset()
   {
    if (table != null)
     for(int i = 0; i < table.size(); i++)
      ((Scope)table.get(i)).remove();

    table = new Vector();
   }

   /**
    * get scope x
    * @param x int
    * @return Scope
    */
   static Scope get(int x)
   {
    return x >= 0 && x < table.size()?(Scope)table.get(x):null;
   }

   /**
    * get index of scope x
    * @param x Scope
    * @return int
    */
   static int indexOf(Scope x)
   {
    return table.indexOf(x);
   }

   /**
    * iterator over scope
    * @return Iterator
    */
   Iterator iterator()
   {
    return map.values().iterator();
   }

   /**
    * remove entry x of scope
    * @param x String
    */
   boolean remove(String x)
   {
    String trunc = x.substring(x.lastIndexOf('.') + 1);
    SortedMap l = map.headMap(trunc);
    SortedMap h = map.tailMap(trunc + '!');

    int diff = map.size() - l.size() - h.size();

    map = new TreeMap(l);
    map.putAll(h);

    return diff != 0;
   }

   /**
    * add x as follower into object
    * @param x Scope
    */
   private void add(Scope x)
   {
    if (x.prefix != null && x.prefix.length() > 0)
     follower.add(x);
   }

   /**
    * clear followers
    */
   private void remove()
   {
    follower = null;
   }

  Scope()
   {
    this(null, automatic, "");
   }

   /**
    * constructor
    * @param x Scope enclosing scope
    * @param storageClass int
    * @param prefix String
    * @param trailer String
    */
   Scope(Scope x, int storageClass, String prefix, String trailer)
   {
    map = new TreeMap();
    prev = x;
    this.prefix = prefix;
    this.storageClass = storageClass;
    this.trailer = trailer;
    this.follower = new Vector();

    table.add(this);
    if (x != null)
     x.add(this);
   }

   /**
    * constructor
    * @param x Scope enclosing scope
    * @param storageClass int
    * @param prefix String
    */
   Scope(Scope x, int storageClass, String prefix)
   {
    this(x, storageClass, prefix, "");
   }

   /**
    * constructor
    * @param x Scope enclosing scope
    * @param storageClass int
    * @param prefix String
    * @param label Vector labels of scope
    * @param block int type of scope
    */
   Scope(Scope x, int storageClass, String prefix, Vector label, int block)
   {
    this(x, storageClass, prefix);
    this.label = label;
    this.block = block;
   }

   /**
    * insert member into scope
    * @param member Basic
    */
   void declInsert(Basic member)
   {
    String trunc = member.name.string.substring(member.name.string.lastIndexOf('.') + 1);
    String modified = trunc + " " + member.version;

    map.put(modified, member);
   }

   /**
    * insert member into scope
    * @param member Basic
    */
   void declInsertOverload(Basic member)
   {
    String trunc = member.name.string.substring(member.name.string.lastIndexOf('.') + 1);
    String modified = trunc;

    if (member instanceof VariableType /*&& storageClass == Scope.classed*/ && (member.modify & Keyword.STATICSY.value) == 0)
     {
      VariableType x = (VariableType)member;

      if (x.type.dim > 0 || x.type.type == Keyword.NONESY)
       {
        x.offset = refcount++ * 4;
        offset += 4;
        for(Iterator iter = map.keySet().iterator(); iter.hasNext();)
         {
          Basic obj = (Basic)map.get(iter.next());
          if (obj instanceof VariableType)
           {
            x = (VariableType)obj;
            if (!(x.type.dim > 0 || x.type.type == Keyword.NONESY))
             x.offset += 4;
           }
         }
       }
      else
       {
        x.offset = offset;
        offset += x.type.type == Keyword.LONGSY || x.type.type == Keyword.DOUBLESY || x.type.type == Keyword.FLOATSY ? 8 : 4;
       }
     }

    member.version = member.hashCode();
    modified = trunc + " " + member.version;

    while(map.containsKey(modified))
     {
      member.version += 8;
      modified = trunc + " " + member.version;
     }

    map.put(modified, member);
   }

   /**
    * generate full path ending with x
    * @param x String
    * @return String
    */
   String buildPath(String x)
   {
    for(Scope y = this; y != null; y = y.prev)
     if (y.prefix != null && y.prefix.length() > 0)
      x = y.prefix + ((x.length() > 0 && !x.startsWith("."))?".":"") + x;

    return x;
   }

   /**
    * generate java path ending with x
    * @param x String
    * @return String
    */
   String javaPath(String x)
   {
    for(Scope y = this; y != null; y = y.prev)
     if (y.trailer != null && y.trailer.length() > 0)
      x = y.trailer + (x.length() > 0?".":"") + x;
     else if (y.prefix != null && y.prefix.length() > 0)
      x = y.prefix + (x.length() > 0?".":"") + x;

    return x;
   }

   /**
    * locate all scopes containing name
    * @param name String
    * @param invisible int
    * @return Scope[]
    */
  Scope [] locate(String name, int invisible, boolean isClass)
   {
    return locate(name, invisible, isClass, true, true);
   }

  private Scope [] locate(String name, int invisible, boolean isClass, boolean forward, boolean depth)
   {
    Vector found = new Vector();
    String [] part = name.split("\\.");

    Scope stop = null;
    for(Scope y = this; y != null && invisible != 0; y = y.prev)
     if (y.prefix != null && y.prefix.length() > 0)
      stop = y.prev;

    // is present in an open scope?
    for(Scope y = this; y != stop && found.size() == 0; y = y.prev)
     {
      String path = y.buildPath("");

      if (isClass && y.prefix != null && y.prefix.length() > 0)
       if (path.endsWith("." + name) || path.compareTo(name) == 0)
        if (y.prev != null)
         {
          found.add(y.prev);
          break;
         }

      path += "." + part[part.length - 1];
      Basic [] x = y.get(part[part.length - 1]);
      int count = x.length;

      for(int i = 0; i < x.length; i++)
       if ((x[i].modify & invisible) != 0)
        count--;

      if (count != 0)
       if (part.length == 1 ||
           y.prefix != null && y.prefix.length() > 0 && (path.endsWith("." + name) || path.compareTo(name) == 0))
        found.add(y);
     }

    // present in a follower
    if (forward && part.length > 1)
     for(int i = 0; i < follower.size() && found.size() == 0; i++)
      {
       Scope y = (Scope)follower.get(i);
       Scope [] s;

       for(int k = 0; k < part.length && found.size() == 0; k++)
        if (part[k].compareTo(y.prefix) == 0 && (s = y.locate(name, invisible, isClass, forward, false)).length > 0)
         for(int j = 0; j < s.length; j++)
          found.add(s[j]);
      }

    // is present in an interface or a super class?
    invisible = Keyword.PRIVATESY.value | ((invisible & Keyword.PRIVATESY.value) != 0?Keyword.PROTECTEDSY.value:0);

    for(Scope t = this; t != stop && depth; t = t.prev)
     if (t.prefix != null && t.prefix.length() > 0 && t.prev != null)
      {
       Basic [] b = t.prev.get(t.prefix);
       for(int j = 0; j < b.length; j++)
        if (b[j] instanceof ClassType && ((ClassType)b[j]).scope == t)
         {
          Scope [] z = new Scope[0];
          ClassType y = (ClassType)b[j];

          if (y.implement != null)
           for (int i = 0; z.length == 0 && i < y.implement.length; i++)
            if (y.implement[i].scope != null)
             z = y.implement[i].scope.locate(name, invisible, isClass, false, true);

          if (y.extend != null && y.extend.scope != null && z.length == 0)
           z = y.extend.scope.locate(name, invisible, isClass, false, true);

          for(int i = 0; i < z.length; i++)
           if (!found.contains(z[i]))
            found.add(z[i]);
         }
      }

    return (Scope [])found.toArray(new Scope[found.size()]);
   }

   /**
    * get all entries named name
    * @param name String
    * @return Basic[]
    */
   Basic [] get(String name)
   {
    String trunc = name.substring(name.lastIndexOf('.') + 1);
    SortedMap m = map.subMap(trunc + ' ', trunc + "!");;

    return (Basic [])m.values().toArray(new Basic[m.size()]);
   }

   /**
    * get scope labeled x
    * @param x String
    * @return Scope
    */
   Scope findLabel(String x)
   {
    if (label != null)
     for(int i = 0; i < label.size(); i++)
      if (x.compareTo((String)label.get(i)) == 0)
       return this;

    return (block != Scope.MAIN && prev != null)?prev.findLabel(x):null;
   }

   /**
    * read object from x
    * @param prev Scope enclosing scope
    * @param storageClass int
    * @param prefix String
    * @param trailer String
    * @param x MyBuffer
    */
   Scope(Scope prev, int storageClass, String prefix, String trailer, DataInputStream x)
   {
    this(prev, storageClass, prefix, trailer);
    Scope dummy = cur;
    cur = this;
    int size = 0;
    try
     {
      offset = x.readInt();
      size = x.readInt();
     }
    catch (IOException ex) { ex.printStackTrace(); }

    while(size-- > 0)
     {
      Basic base = new Basic(x);

      if ((base.modify & (Keyword.CLASSSY.value | Keyword.INTERFACESY.value)) != 0)
       {
        declInsert(new ClassType(this, x, base));
        if (!Header.stack.contains(this))
         Header.stack.add(this);
       }
      else if ((base.modify & Keyword.METHODSY.value) != 0)
       declInsert(new MethodType(x, base));
      else
       declInsert(new VariableType(x, base));
     }

    cur = dummy;
   }

   /**
    * serialize object
    */
   void serialize(DataOutputStream x)
   {
    Iterator iter = iterator();

    try
     {
      x.writeUTF(prefix);
      x.writeUTF(trailer);
      x.writeInt(offset);
      x.writeInt(map.values().size());
     }
    catch (IOException ex) { ex.printStackTrace(); }

    while(iter.hasNext())
     ((Basic)iter.next()).serialize(x);
   }
 }


