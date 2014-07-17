package MyJava;

/**
 * <p>Überschrift: Fehlermeldungen</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */


public class Errors
 {
  private static int counter;
  private static int oldLine = -1;

  /**
    * add line to error log
    *
    * @param     line number of sourceline
    * @param     col startting column
    * @param     msg explanation
    * @param     w, if true an error message
    */
  public static void warning(String source, int line, int col, String msg, boolean w)
   {
    System.out.println("-- file: " + source + " ,line: " + line + " ,col: " + col + (w?" error: ":" warning: ") + msg);
    if (w && line != oldLine)
     counter++;
    oldLine = line;
   }

  /**
    * add line to error log and accumulate error count
    *
    * @param     line number of sourceline
    * @param     col startting column
    * @param     msg explanation
    */
  public static void println(String source, int line, int col, String msg)
   {
    warning(source, line, col, msg, true);
   }

  /**
    * reset error logfile
    *
    */
  public static void reset()
   {
    counter = 0;
   }

  /**
    * get count of reporting lines in error logfile
    *
    * @return    number of lines
    */
  public static int count()
   {
    return counter;
   }
 }
