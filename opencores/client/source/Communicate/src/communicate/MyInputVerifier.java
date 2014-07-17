package communicate;

import javax.swing.*;
import java.util.regex.*;


/**
 * <p>Überschrift: Input verifier</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

public class MyInputVerifier extends InputVerifier
 {
  Pattern rule;

  /**
   * set regular expression defining the verification
   * @param expr String
   */
  public MyInputVerifier(String expr)
   {
    super();
    rule = Pattern.compile(expr);
   }

   /**
    * checks input
    * @param input JComponent
    * @return boolean
    */
   public boolean verify(JComponent input)
   {
    Matcher m = rule.matcher(((JTextArea)input).getText());

    return m.matches();
   }

 }
