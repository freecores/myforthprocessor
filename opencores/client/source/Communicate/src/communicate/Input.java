package communicate;

/**
 * <p>Überschrift: Eingabeconsole</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.io.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;

public class Input extends JScrollPane
 {
  static final long serialVersionUID = 1024;
  TitledBorder titledBorder4, titledBorder6;
  JEditorPane jEditorPane1 = new JEditorPane();

  /**
   * constructs Pane
   * @param width int
   * @param height int
   */
  public Input(int width, int height)
   {
    super();
    titledBorder4 = new TitledBorder("");
    titledBorder6 = new TitledBorder("");
    setViewportBorder(titledBorder6);
    getViewport().setBackground(Color.white);
    setBorder(titledBorder4);
    if (width > 0 && height > 0)
     setPreferredSize(new Dimension(width, height));
    getViewport().add(jEditorPane1, null);
   }

   /**
    * clear text
    */
   public void clear()
   {
    jEditorPane1.setText("");
   }

   /**
    * save text of pane into file
    * @param file File
    */
   public void save(File file)
   {
    try
     {
      FileOutputStream out = new FileOutputStream(file, false);
      out.write(jEditorPane1.getText().getBytes());
      out.close();
     }
    catch (Exception ex)
     {
     }
   }

   /**
    * append content of file to pane's text
    * @param file File
    */
   public void load(File file)
   {
    try
     {
      byte [] buff = new byte[1];
      FileInputStream in = new FileInputStream(file);
      StringBuffer x = new StringBuffer();
      for(int i = in.read(buff); i >= 0; i = in.read(buff))
       x.append((char)buff[0]);
      jEditorPane1.setText(jEditorPane1.getText() + x);
     }
    catch (Exception ex)
     {
     }
   }

   /**
    * get text
    * @return String
    */
   public synchronized String getText()
   {
    return jEditorPane1.getText();
   }

   /**
    * add listener to pane
    * @param listener CaretListener
    */
   public void addCaretListener(CaretListener listener)
   {
    jEditorPane1.addCaretListener(listener);
   }

   /**
    * remove listener from pane
    * @param listener CaretListener
    */
   public void removeCaretListener(CaretListener listener)
   {
    jEditorPane1.removeCaretListener(listener);
   }

   /**
    * add listener to pane
    * @param listener CaretListener
    */
   public void addKeyListener(KeyListener listener)
   {
    jEditorPane1.addKeyListener(listener);
   }

   /**
    * remove listener from pane
    * @param listener CaretListener
    */
   public void removeKeyListener(KeyListener listener)
   {
    jEditorPane1.removeKeyListener(listener);
   }
 }
