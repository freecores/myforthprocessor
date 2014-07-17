package MyJava;

/**
 * <p>Überschrift: Token des Scanners</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.io.*;

public class Token
 {
  public Keyword kind;     // type of token
  public String source;    // name of source file
  public int line;         // found in sourceline
  public int col;          // starting column
  public long val;          // his value
  public double fval;
  public String string;    // his string representation

  Token() {}

  Token(Token x)
   {
    kind = x.kind;
    source = x.source != null?new String(x.source):null;
    line = x.line;
    col = x.col;
    val = x.val;
    fval = x.fval;
    string = x.string != null?new String(x.string):null;
   }

  void serialize(DataOutputStream x)
   {
    try
     {
      x.writeInt(kind.value);
      x.writeUTF(source);
      x.writeInt(line);
      x.writeInt(col);
      x.writeLong(val);
      x.writeDouble(fval);
      x.writeUTF(string);
    }
    catch (IOException ex) { ex.printStackTrace(); }
   }

  Token(DataInputStream x)
   {
    try
     {
      kind = Keyword.get(x.readInt());
      source = x.readUTF();
      line = x.readInt();
      col = x.readInt();
      val = x.readLong();
      fval = x.readDouble();
      string = x.readUTF();
     }
    catch (IOException ex) { ex.printStackTrace(); }
   }
 }
