package forthassembler;

/**
 * <p>Überschrift: Quelldatei des Analysators</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */


public class Source
 {
  String sourceFile;
  String unget;
  int col;
  int line;
  int pos;

  public Source(String sourceFile)
   {
    this.sourceFile = sourceFile;
    unget = null;
    col = 0;
    line = 1;
    pos = 0;
   }
 }
