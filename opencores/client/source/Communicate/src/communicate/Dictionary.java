package communicate;

/**
 * <p>Überschrift: Verwaltung des FORTH-Dictionary</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.util.*;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.File;

public class Dictionary
 {
  static boolean show = false;
  static final int CONSTANT = 1,
                   VARIABLE = 2,
                   PROCEDURE = 3,
                   VALUE = 4,
                   MODULE = 5,
                   MARKER = 6,
                   TWOVARIABLE = 7;
  String name;
  int type;
  int address;

  static volatile Vector dictionary = null;

  static boolean isReady()
   {
    return dictionary != null;
   }

  /**
   * extract dictionary from buffer
   * @param buffer InputBuffer
   * @param len int
   */
  static synchronized void build(InputBuffer buffer, int len)
   {
    Dictionary dict;
    Vector dictionary = new Vector();
    buffer.reset();
    buffer.skip(2);

    while(buffer.pos < len)
     {
      StringBuffer x;
      int length;

      dict = new Dictionary();
      x = new StringBuffer();

      length = buffer.getByte() + (buffer.getByte() << 8);
      //x.append((char)'"');
      while(length-- != 0)
       x.append((char)buffer.getByte());
      //x.append((char)'"');

      dict.name = new String(x);
      dict.type = buffer.getByte() + (buffer.getByte() << 8);
      buffer.getByte();
      buffer.getByte();
      dict.address = buffer.getByte() + (buffer.getByte() << 8) + (buffer.getByte() << 16) + (buffer.getByte() << 24);

      dictionary.add(dict);
     }

    Dictionary.dictionary = dictionary;

    if (show)
     try
      {
       File f;
       FileOutputStream file = new FileOutputStream(f = File.createTempFile("dictionary", null));

       for(int i = 0; i < Dictionary.dictionary.size(); i++)
        {
         String s = ((Dictionary)Dictionary.dictionary.get(i)).toString() + "\r\n";
         file.write(s.getBytes());
        }

       file.close();
       Runtime.getRuntime().exec("notepad " + f.getAbsolutePath());
      }
     catch (IOException ex4) { }
   }

   /**
    * find entry with name closest to desired name
    * @param name String
    * @return Dictionary
    */
   static synchronized Dictionary search(String name)
   {
    Dictionary dict, found = null;
    int min = 999999;

    for(int i = 0; dictionary != null && i < dictionary.size(); i++)
     {
      dict = (Dictionary)dictionary.get(i);

      int j = dict.name.length();

      if (dict.name.endsWith(name) && j < min)
       {
        min = j;
        found = dict;
       }
     }

    return found;
   }

  public String toString()
   {
    StringBuffer x = new StringBuffer();

    switch(type)
     {
      case CONSTANT: x.append("constant"); break;
      case VARIABLE: x.append("variable"); break;
      case PROCEDURE: x.append("word"); break;
      case VALUE: x.append("value"); break;
      case MODULE: x.append("module"); break;
      case MARKER: x.append("marker"); break;
      case TWOVARIABLE: x.append("2variable"); break;
     }

    x.append(' ');
    x.append(name);

    return new String(x);
   }
 }
