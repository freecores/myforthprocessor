
package communicate;

import java.io.*;

/**
 * <p>Überschrift: Dateifilter</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

public final class MyFileFilter extends javax.swing.filechooser.FileFilter
{
  String [] endings;
  String description;

  /**
   * constructor
   * @param regex the regular expression
   */
  public MyFileFilter(String [] endings)
   {
    this.endings = endings;
   }

  /**
   * checks file, if his name matches the regular expression
   * @param f the file
   * @return true, if f is an directory or matches the regular expression
   */
  public boolean accept(File f)
   {
    for(int i = 0; i < endings.length; i++)
     if (f.getName().endsWith("." + endings[i]) || endings[i].endsWith("*"))
      return true;

    return false;
   }

   /**
    * @param text description of filter
    */
   public void setDescription(String text)
    {
     this.description = new String(text);
    }

   /**
   * @return the description
   */
  public String getDescription()
   {
    return description;
   }

}
