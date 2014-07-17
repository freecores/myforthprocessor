package MyJava;

/**
 * <p>Überschrift: Operatorknoten</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.io.*;

class Operation
 {
  Operation left;               // left son
  Operation right;              // right son
  Keyword operator;             // operator
  static int labelno = 0;

  Token name;
  Type type;

  Scope scope;                  // scope
  String code = "";             // translated FORTH-Code
  boolean loaded = false;       // resulting value on stack

  static int newLabel()
   {
    return labelno++;
   }

  Operation()
   {
    left = null;
    right = null;
    type = new Type();
    scope = null;
   }

  Operation(Operation x)
   {
    name = x.name;
    type = new Type(x.type);
    scope = x.scope;
    code = new String(x.code);
    operator = x.operator;
    if (x.left != null)
     left = new Operation(x.left);
    if (x.right != null)
     right = new Operation(x.right);
   }

  Operation(DataInputStream x)
   {
    try
     {
      if (x.readBoolean())
       name = new Token(x);

      if (x.readBoolean())
       type = new Type(x);

      scope = Scope.get(x.readInt());
      code = x.readUTF();
      loaded = x.readBoolean();
      operator = Keyword.get(x.readUTF());
      if (x.readBoolean())
       left = new Operation(x);
      else
       left = null;

      if (x.readBoolean())
       right = new Operation(x);
      else
       right = null;
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

  void serialize(DataOutputStream x)
   {
    try
     {
      x.writeBoolean(name != null);
      if (name != null)
       name.serialize(x);
      x.writeBoolean(type != null);
      if (type != null)
       type.serialize(x);
      x.writeInt(Scope.indexOf(scope));
      x.writeUTF(code);
      x.writeBoolean(loaded);
      x.writeUTF(operator != null ? operator.string : "");
      x.writeBoolean(left != null);
      if (left != null)
       left.serialize(x);
      x.writeBoolean(right != null);
      if (right != null)
       right.serialize(x);
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }

  public String toString()
   {
    String s = name != null?name.string:"";
    s += type != null?type.toString():"";
    return s + code + loaded + (operator != null?operator.string:"") +
         (left != null?left.toString():"") + (right != null?right.toString():"");
   }
 }
