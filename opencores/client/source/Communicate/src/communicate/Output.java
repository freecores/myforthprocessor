package communicate;


/**
 * <p>Überschrift: Consolenausgabe</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.io.*;
import java.util.*;

public class Output extends JScrollPane
 {
  static final long serialVersionUID = 1029;
  TitledBorder titledBorder4, titledBorder6;
  JTextPane jTextPane1 = new JTextPane();
  private int row, col;
  private String [] line = null;
  private boolean shift;
  boolean unicode;
  private byte hold;
  private final int incr;
  private final Dimension dim;

  /**
   * constructs Pane
   * @param width int
   * @param height int
   */
  public Output(int width, int height)
   {
    super();
    titledBorder4 = new TitledBorder("");
    titledBorder6 = new TitledBorder("");
    setViewportBorder(titledBorder6);
    getViewport().setBackground(Color.white);
    setBorder(titledBorder4);
    if (width > 0 && height > 0)
     setPreferredSize(dim = new Dimension(width, height));
    else
     dim = getViewport().getSize();
    jTextPane1.setEditable(false);
    getViewport().add(jTextPane1, null);
    incr = jTextPane1.getScrollableUnitIncrement(new Rectangle(new Point(0, 0), dim), SwingConstants.VERTICAL, -1);
    clear();
    shift = false;
    unicode = true;
    hold = 0;
   }

   /**
    * set editable on or off
    * @param x boolean
    */
   public void setEditable(boolean x)
   {
    jTextPane1.setEditable(x);
   }

   /**
    * append text to pane's text
    * @param text String
    */
   public synchronized void addText(String text)
   {
    Vector u = new Vector();
    String s = "";
    char curr;

    for(int i = 0; i < text.length(); i++)
     {
      curr = text.charAt(i);
      if (curr == '\n')
       {
        u.add(s);
        s = "";
       }
      else if (curr == '\t' || curr >= ' ')
       s += curr;
     }

    String [] z = new String[u.size() + 1];
    for(int i = 0; i < u.size(); i++)
     z[i] = (String)u.get(i);
    z[z.length - 1] = s;

    // additional empty lines necessary?
    if (row + z.length > line.length)
     {
      String [] v = new String[row + z.length];
      for(int i = 0; i < line.length; i++)
       v[i] = line[i];
      for(int i = line.length; i < v.length; i++)
       v[i] = "";
      line = v;
     }

    // additional spaces in cursor line necessary?
    while(line[row].length() < col)
     line[row] += ' ';

    // replace all finished lines
    String v;
    if (col == 0)
     v = z[0];
    else
     {
      if (col < line[row].length())
       v = line[row].substring(0, col);
      else
       v = line[row];

      v += z[0];
     }
    col = v.length();
    if (col < line[row].length())
     v +=  line[row].substring(col);
    line[row] = v;

    for(int i = 1; i < z.length; i++)
     {
      col = z[i].length();
      if (col < line[++row].length())
       line[row] = z[i] + line[row].substring(col);
      else
       line[row] = z[i];
     }

    while(row > 512)
     {
      int i;
      for(i = 0; i < line.length - 32; i++)
       line[i] = line[i + 32];
      for(;i < line.length; i++)
       line[i] = "";
      row -= 32;
     }

    // stringify updated line
    v = line[0];
    for(int i = 1; i < line.length; i++)
     v += "\r\n" + line[i];

    jTextPane1.setText(v);

    // scroll to the page containing the cursor
    if (u.size() > 0)
     jTextPane1.scrollRectToVisible(new Rectangle(
                                    new Point(dim.width, (int)((row + 1) * incr)),
                                    dim));
   }

   /**
    * append character to pane's text
    * @param text char
    */
   public synchronized void addText(char text)
   {
	if (text == 8)
	 {
	  if (line[row].length() > 0)
	   {
		line[row] = line[row].substring(0, line[row].length() - 1);
	   }
	  else
	   row = Math.max(row - 1, 0);
	  
      addText("");
	 }
	else
	 {
      char [] x = new char[1];
      x[0] = text;
      addText(new String(x));
	 }
   }

   /**
    * append a byte to pane's text
    * @param ascii boolean
    * @param binary byte
    */
   public synchronized void addText(byte binary)
   {
    if (!unicode)
     if ((shift = !shift))
      hold = binary;
     else
      addText((char)((binary << 8) + (hold & 0xff)));
    else
     addText((char)binary);
   }

   public synchronized void clearHold(boolean f)
   {
    if (!(unicode = f))
     shift = false;
   }

   /**
    * replace text of pane with line
    * @param line String
    */
   public synchronized void setText(String line)
   {
    jTextPane1.setText(line);
   }

   /**
    * clear pane
    */
   public synchronized void clear()
   {
    line = new String[0];
    jTextPane1.setText("");
    row = 0;
    col = 0;
   }

   /**
    * set cursor of pane to new position
    * @param row int
    * @param col int
    */
   public synchronized void setCursor(int row, int col)
   {
    this.row = row;
    this.col = col;
   }

   /**
    * save text of pane into file
    * @param file File
    */
   public synchronized void save(File file)
   {
    try
     {
      FileOutputStream out = new FileOutputStream(file, false);
      out.write(jTextPane1.getText().getBytes());
      out.close();
     }
    catch (Exception ex)
     {
     }
   }
 }
