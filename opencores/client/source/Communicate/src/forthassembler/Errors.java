package forthassembler;

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

  /**
   * print an error message in a single line
   * @param name name of source file
   * @param line line of error
   * @param col column of error
   * @param msg message
   */
  public static void println(String name, int line, int col, String msg, boolean error)
   {
    System.out.println(name+" -- line: "+line+" ,col: "+col+"\t" + (error? "error: ": " warning: ") + msg);
    if (error)
     counter++;
   }

  /**
   * initialize
   */
  public static void reset()
   {
    counter = 0;
   }

  /**
   * @return error count
   */
  public static int count()
   {
    return counter;
   }
 }
