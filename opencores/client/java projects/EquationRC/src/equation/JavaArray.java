package equation;

/**
 * <p>Überschrift: </p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright (c) 2005</p>
 * <p>Organisation: </p>
 * @author unbekannt
 * @version 1.0
 */

public class JavaArray
 {
  public final int length;      // length of array
  public final int shift;       // ld(wordlength)
  public int array;             // reference to physical array

  /**
   * constructor
   * @param length number of elements
   * @param code lower 16 bit are the dimension, high part is the type
   */
  public JavaArray(int length, int code)
   {
    int x = code >>> 16;

    switch(x)
     {
      case 1:
      case 2:
       x = shift = x - 1;
       break;
      case 5:
      case 6:
       x = shift = 3;
       break;
      default:
       x = shift = 2;
     }

    if (code & 0xffff > 1)
     x = 2;

    this.length = length;
    #ass "code §this CELL+ @ 20 + !"; // overwrite code field

    #ass "length x LSHIFT MALLOC DROP TO x ";

    this.array = x;
   }

  /**
   * clone an array
   * @return a new reference to the array
   */
  public JavaArray clone()
   {
    int code;
    #ass "§this CELL+ @ 20 + @ TO code";
    JavaArray b = new JavaArray(length, code);

    int x = b.array;

    #ass "x DECREFERENCE";

    x = array;

    #ass "x INCREFERENCE";

    b.array = x;

    return b;
   }

  /**
   * fetch an element of the array
   * @param pos index
   * @return a reference to the element
   */
  public int getElem(int pos)
   {
    int x = array;
    #ass "x CELL+ @ TO x";
    int code;
    #ass "§this CELL+ @ 20 + @ TO code";
    int shift = (code & 0xffff > 1)?2:this.shift;

     if (pos < length && pos >= 0)
      #ass "x pos shift LSHIFT + TO x ";
     else if (pos != 0)
      #ass "S\" index out of range:  \" TYPE pos . CR";

    return x;
   }

  /** destructor
   */
  public void ~destructor()
   {
    int y = shift, x = array, l = length;
    int code;
    #ass "§this CELL+ @ 20 + @ TO code";
    int depth = code & 0xffff;
    int hash = code >>> 16;
    int c;
    Object obj;
    #ass "§this @ 1- 0= IF §this CELL+ @ @ TO obj";
    kill(obj, 0);
    #ass "§this CELL+ @ 8 + @ TO obj";
    kill(obj, 0);
    #ass "x @ TO c";

    if (c == 1 && (depth > 1 || hash > 6))
     {
      x = getElem(0);
      #ass "x l 0 ?DO DUP @ TO obj";
      kill(obj, -1);
      #ass "CELL+ LOOP DROP";
     }

    x = array;
    #ass "x FREE DROP";
    #ass "ENDIF §this DECREFERENCE";
   }

  /**
   * create an standard string
   * @param length stringlength
   * @param bytestring initial content
   * @return an initialized string object
   */
  public static String createString(int bytestring, int length)
   {
    byte [] body = new byte[length];
    int target = ((JavaArray)body).getElem(0);
    #ass "bytestring target length 0 ?DO OVER I + C@ OVER I + C! LOOP 2DROP";
    return new String(body);
   }

  /**
   * create an unicode string
   * @param length stringlength
   * @param forthstring initial content
   * @return an initialized string object
   */
  public static String createUnicode(int forthstring, int length)
   {
    char [] body = new char[length /= 2];
    int target = ((JavaArray)body).getElem(0);
    #ass "forthstring target length 2* 0 ?DO OVER I + C@ OVER I + C! LOOP 2DROP";
    return new String(body);
   }

  /**
   * decrement (and remove) an object
   * @param obj the object
   * @param polymorph the complete object, if true, only this part otherwise
   */
  public static void kill(Object obj, int polymorph)
   {
    #ass "obj HANDLEVALID";
    #ass "IF obj DUP -1086882304 polymorph EXECUTE-METHOD ENDIF";
   }

  /**
   * print unicode string to console
   * @param s String
   */
  public static void print(String s)
   {
    int length = s.length();
    int a = ((JavaArray)s.value).getElem(0);

    #ass "a length 2* PRINT";
   }

  /**
   * default exception handler
   * @param e Exception
   */
  public static void handler(Exception e)
   {
    print(e.getMessage());
    #ass "ABORT\" program aborted \" ";
   }
 }
