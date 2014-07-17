package communicate;

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.event.*;

/**
 * <p>Überschrift: Dialog</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

public class MyDialog extends JDialog implements KeyListener
 {
  static final long serialVersionUID = 1027;
  JPanel panel1 = new JPanel();
  BorderLayout borderLayout1 = new BorderLayout();
  JTextArea jTextArea1 = new JTextArea();
  TitledBorder titledBorder1;
  TitledBorder titledBorder2;

  public MyDialog(Frame frame, String title, boolean modal, String text, String tip, InputVerifier verify)
   {
    this(frame, 450, 30, title, modal, text, tip, verify);
   }

  /**
   * construct dialog
   * @param frame Frame
   * @param title String
   * @param modal boolean
   * @param text String
   * @param tip String
   * @param verify InputVerifier
   */
  public MyDialog(Frame frame, int width, int height, String title, boolean modal, String text, String tip, InputVerifier verify)
   {
    super(frame, title, modal);
    titledBorder1 = new TitledBorder("");
    titledBorder2 = new TitledBorder("");
    panel1.setLayout(borderLayout1);
    panel1.setPreferredSize(new Dimension(width, height));
    jTextArea1.addKeyListener(this);
    jTextArea1.setLineWrap(false);
    jTextArea1.setFont(new java.awt.Font("Dialog", 0, 16));
    jTextArea1.setBorder(titledBorder2);
    jTextArea1.setToolTipText(tip);
    jTextArea1.setText(text);
    jTextArea1.setInputVerifier(verify);
    jTextArea1.setEditable(true);
    getContentPane().add(panel1);
    panel1.add(jTextArea1, BorderLayout.SOUTH);
    pack();
   }

   /**
    * get valid verfied text
    * @return String
    */
  public String getText()
  {
   String x = jTextArea1.getText();
   if (x.endsWith("\n") || x.endsWith("\r"))
    x = x.substring(0, x.length() - 1);
   jTextArea1.setText(x);
   InputVerifier y = jTextArea1.getInputVerifier();

   return y.verify(jTextArea1)?x:"";
  }

  /**
   * close dialog when return entered
   * @param e KeyEvent
   */
  public void keyTyped(KeyEvent e)
   {
    //if (e.getKeyChar() == '\n' || e.getKeyChar() == '\r')
    if (e.getKeyChar() < ' ')
     {
      this.dispose();
      this.processWindowEvent(new WindowEvent(this, WindowEvent.WINDOW_CLOSING));
     }
   }

  public void keyPressed(KeyEvent e)
   {

   }
  public void keyReleased(KeyEvent e)
   {

   }
 }
