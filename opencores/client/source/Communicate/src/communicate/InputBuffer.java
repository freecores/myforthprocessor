package communicate;

/**
 * <p>Überschrift: Buffer</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

public class InputBuffer
 {
  int len, pos;
  byte [] buffer;

  public InputBuffer()
   {
    len = 0;
    pos = 0;
    buffer = new byte[256];
   }

  public InputBuffer(InputBuffer x)
   {
    len = x.len;
    pos = x.pos;
    buffer = new byte[x.buffer.length];

    for(int i = 0; i < buffer.length; i++)
     buffer[i] = x.buffer[i];
   }

   /**
    * enlarge buffer, if full
    */
   private void resize()
   {
    if (buffer.length == len)
     {
      byte [] x = new byte[buffer.length + 256];

      for(int i = 0; i < len; i++)
       x[i] = buffer[i];

      buffer = x;
     }
   }

   /**
    * reset index for reading
    */
   public void reset()
   {
    pos = 0;
   }

   /**
    * reset index for reading and truncate buffer length to len
    * @param len int
    */
   public void reset(int len)
   {
    reset();
    this.len = len;
   }

   /**
    * read next byte
    * @return int
    */
   public int getByte()
   {
    return (pos < len)?(int)(buffer[pos++] & 0xff):-1;
   }

   /**
    * append raw byte
    * @param x byte
    */
   public void appendRaw(byte x)
   {
    resize();
    buffer[len++] = x;
   }

   /**
    * append masked byte
    * @param x byte
    */
   public void append(byte x)
   {
    if (x < 28 || x > 240)
     appendRaw((byte)27);
    appendRaw(x);
   }

   /**
    * append integer
    * @param x int
    */
   public void append(int x)
   {
    append((byte)x);
    x >>= 8;
    append((byte)x);
    x >>= 8;
    append((byte)x);
    x >>= 8;
    append((byte)x);
   }

   /**
    * append long
    * @param x long
    */
   public void append(long x)
   {
    append((byte)x);
    x >>= 8;
    append((byte)x);
    x >>= 8;
    append((byte)x);
    x >>= 8;
    append((byte)x);
    x >>= 8;
    append((byte)x);
    x >>= 8;
    append((byte)x);
    x >>= 8;
    append((byte)x);
    x >>= 8;
    append((byte)x);
   }

   /**
    * append content of string
    * @param x String
    */
   public void append(String x)
   {
    for(int i = 0; i < x.length(); i++)
     append((byte)x.charAt(i));
   }

   /**
    * skip x byte
    * @param x int
    */
   public void skip(int x)
   {
    pos += x;
   }

   /**
    * read byte at position x
    * @param x int
    * @return int
    */
   public int byteAt(int x)
   {
    return (x < len)?(int)(buffer[x] & 0xff):-1;
   }
 }
