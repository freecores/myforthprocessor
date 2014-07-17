package MyJava;

import java.io.IOException;

/**
 * <p>Überschrift: </p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright (c) 2005</p>
 * <p>Organisation: </p>
 * @author unbekannt
 * @version 1.0
 */

public class MyObject
{
  Object a;
  int [] b;
  long d = 0;
  static long e = 0;

  static
  {
   System.out.println("hello world");
  }
  public MyObject(int x) { d = x; }
  public MyObject() { this(1); a = this; int b = (int)this.d;  }

  public int hashCode() { return 0; }
  public MyObject other()
   {
    return new MyObject() { int hashcode() { return 1; }};
   }

  void junk(CharSequence ss)
  {
    int [] a[] = { { 1, (int)(2 + 3L) }, new int[3] },
        b [][] = new int[2][2][3],
        c = new int[5];
    String s = "hello";
    s += ',' + " world";
    int len = ss.length();
    out:
    try {
    for(int i = 0; c != null && i < c.length; ++i)
     for(int j = 0; j < b[1].length; j++, i++)
     {
      int k = -1;

      switch(b[1][1][j])
      {
       case 1:
       case 2:
         k = i + j;
         c[k % c.length] = i + j;
         break out;
       case 3:
         k = i - j;
       default:
         k *= 2;
         throw new IOException();
      }
     }
    } catch(IOException ex) { ex.printStackTrace(); }
      catch(Exception ex) { c = new int[0]; }
      finally { System.exit(0); }
  }
}
