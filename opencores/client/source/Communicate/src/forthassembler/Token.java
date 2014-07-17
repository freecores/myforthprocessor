package forthassembler;

/**
 * <p>Überschrift: Token des Scanners</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

public class Token
{
 String sourceFile;                   // filename
 public boolean alternate;            // marks alternative stack
 public int kind;                     // type of token
 public int line;                     // sourceline number
 public int col;                      // column in sourceline
 public long val;                     // number
 public String ident;                 // identifier
 public String string;                // string
 double d;                            // floating
}
