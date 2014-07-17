package equation;

/**
 * <p>Überschrift: </p>
 *
 * <p>Beschreibung: </p>
 *
 * <p>Copyright: Copyright (c) 2005</p>
 *
 * <p>Organisation: </p>
 *
 * @author unbekannt
 * @version 1.0
 */

//import java.io.PrintStream;

public class BasicIO
{
 public static void print(char y)
  {
   //System.out.print(y);
   JavaArray.print("" + y);

int a = 1 * 3. / 2 - 2 + 1, b;
a = a + 1 + 3 * (1 / 2) * a - 1;
b = 1 + 3 * a / 2 % 3 % 2 * 1. - 1 * (4 % 2) - 1;
  }

 public static void print(String s)
  {
   //System.out.print(s);
   JavaArray.print(s);
  }

 public static void print(int a)
  {
   print(a + "");
  }

 public static void newLine()
  {
   //System.out.println();
   #ass "CR";
  }

 public static String readLine()
  {
   byte [] buffer = new byte[1024];
   int bytes = ((JavaArray)buffer).getElem(0);
   int length = buffer.length - 1;
   #ass "bytes length ACCEPT TO length";
   return JavaArray.createString(bytes, length);
  }
}
