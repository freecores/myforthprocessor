package communicate;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/**
 * <p>Überschrift: Choice</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

public class MyChoice extends JDialog implements ActionListener
 {
  static final long serialVersionUID = 1026;
  JPanel panel1 = new JPanel();
  BorderLayout borderLayout1 = new BorderLayout();
  JComboBox box;

  /**
   * close dialog
   * @param e ActionEvent
   */
  public void actionPerformed(ActionEvent e)
   {
    this.dispose();
    this.processWindowEvent(new WindowEvent(this, WindowEvent.WINDOW_CLOSING));
   }

   /**
    * get selected text
    * @return String
    */
   public String getText()
   {
    return (String)box.getSelectedItem();
   }

   /**
    * construct choice
    * @param frame Frame
    * @param title String
    * @param modal boolean
    * @param item String[]
    */
   public MyChoice(Frame frame, String title, boolean modal, String [] item)
   {
    super(frame, title, modal);
    box = new JComboBox(item);
    box.addActionListener(this);
    //box.setPopupVisible(true);
    box.setVisible(true);
    box.setBounds(0, 0, 300, 30);
    box.setBackground(Color.WHITE);

    panel1.add(box);
    panel1.setPreferredSize(new Dimension(300, 30));
    panel1.setLayout(borderLayout1);
    getContentPane().add(panel1);
    pack();
   }

   /**
    * preselect an item
    * @param item String
    */
   public void show(String item)
   {
    box.setSelectedItem(item);
    super.setVisible(true);
   }
 }
