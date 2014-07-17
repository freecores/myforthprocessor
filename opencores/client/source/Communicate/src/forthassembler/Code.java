package forthassembler;

/**
 * <p>Überschrift: Codeerzeugung</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.io.*;
import java.util.*;

public class Code
 {
  private static Vector [] dasm = new Vector[2];
  private static byte [][] memory = new byte[2][];
  private static int [] pos = new int[2];
  private static int [] memsize = new int[2];
  // start address of internal ROM
  private static int startAddress;
  private static Line line = null;
  static int localOffset;
  private static boolean resident;
  // some statistics
  private static int loadstore;
  private static int val;
  private static int valh;
  private static int consts;
  private static int arithmetic;
  private static int relop;
  private static int flow;
  private static int misc;

  /**
   * initialize code table
   */
  public static void init()
   {
    dasm[0] = new Vector();
    dasm[1] = new Vector();
    startAddress = 0;
    memory[0] = new byte[256];
    memsize[0] = 256;
    pos[0] = 0;
    memory[1] = new byte[256];
    memsize[1] = 256;
    pos[1] = 0;
    resident = false;
    loadstore = 0;
    val = 0;
    valh = 0;
    consts = 0;
    arithmetic = 0;
    relop = 0;
    flow = 0;
    misc = 0;
   }

  /**
   * set local offset
   * @param x value
   */
  public static void setLocalOffset(int x)
   {
    localOffset = x;
   }

  /**
   * get local offset
   * @return int
   */
  public static int localOffset()
   {
    return localOffset;
   }

  /**
   * set starting address of code
   * @param addr startaddress
   */
  public static void origin(int addr)
   {
    startAddress = addr;
   }

   /**
    * write listing to file
    * @param file FileWriter
    * @param rom boolean
    * @param unresolved int
    * @param stringStorage int
    */
   public static void writeDASM(FileWriter file, boolean rom, int unresolved, int stringStorage)
   {
    int index = rom?0:1;

    try
     {
      for(int i = 0; i < 2; i++, index ^= 1)
       if (dasm[index].size() > 0)
        {
         int start = startAddress + (i == 0?0:((pos[index == 0?1:0] + 3) / 4 * 4));
         Line first, second = (Line)dasm[index].get(0);
         String x;

         for(int j = 1; j < dasm[index].size(); j++)
          {
           first = second;
           second = (Line)dasm[index].get(j);
           x = first.toString(index != 0?-first.addr - 1:first.addr, start, second.addr - first.addr) + "\r\n";
           file.write(x);
          }

         x = second.toString(index != 0?-second.addr - 1:second.addr, start, pos[index] - second.addr) + "\r\n";
         file.write(x);
        }
      double total = loadstore + val + valh + consts + arithmetic + relop + flow + misc;
      file.write("\r\ninstruction usage:\r\n");
      file.write("load/store:\t\t" + loadstore + "\t(" + (loadstore / total * 100) + "%)\r\n");
      file.write("32-bit values:\t\t" + val + "\t(" + (val / total * 100) + "%)\r\n");
      file.write("16-bit values:\t\t" + valh + "\t(" + (valh / total * 100) + "%)\r\n");
      file.write("5-bit values:\t\t" + consts + "\t(" + (consts / total * 100) + "%)\r\n");
      file.write("values total:\t\t" + (consts + valh + val) + "\t(" + ((consts + valh + val) / total * 100) + "%)\r\n");
      file.write("arithmetic/logical:\t" + arithmetic + "\t(" + (arithmetic / total * 100) + "%)\r\n");
      file.write("relations:\t\t" + relop + "\t(" + (relop / total * 100) + "%)\r\n");
      file.write("flow control:\t\t" + flow + "\t(" + (flow / total * 100) + "%)\r\n");
      file.write("other:\t\t\t" + misc + "\t(" + (misc / total * 100) + "%)\r\n");
      file.write("total:\t\t\t" + ((int)total) + "\r\n");
      file.write("\r\naverage bytes per instruction: " + ((double)(Code.getSize(0) + Code.getSize(1) - stringStorage) / total) + "\r\n");
      file.write("\r\nstring storage in byte:\t" + stringStorage + "\r\n");
      file.write("unresolved references:\t" + unresolved + "\r\n");
      file.write("\r\nSize of static code: " + Code.getSize(1) + "\r\n");
      file.write("Size of initial code: " + Code.getSize(0) + "\r\n");
     }
    catch (Exception ex) { ex.printStackTrace(); }
   }

  /**
   * write segments and code table
   * @param out output file stream
   */
  public static void writeObj(OutputStream out)
   {
    int length;
    byte [] x = new byte[4];

    try
     {
      // length of module
      length = (pos[1] + 3) / 4 * 4;
      x[0] = (byte)length;
      x[1] = (byte)(length >> 8);
      x[2] = (byte)(length >> 16);
      x[3] = (byte)(length >> 24);
      out.write(x, 0, 4);
      // module
      if (length > 0)
       out.write(memory[1], 0, length);
      // length of initial code
      length = (pos[0] + 3) / 4 * 4;
      x[0] = (byte)length;
      x[1] = (byte)(length >> 8);
      x[2] = (byte)(length >> 16);
      x[3] = (byte)(length >> 24);
      out.write(x, 0, 4);
      // initial code
      if (length > 0)
       out.write(memory[0], 0, length);
     }
    catch(Exception e)
     {System.out.println("output file corrupted!");};
   }

  /**
   * write a single word as vhdl code
   * @param out output file stream
   * @param buffer holds bytes to code
   */
  private static void write4Bytes(OutputStream out, byte [] buffer)
   {
    byte [] quote = new byte[1];
    byte [] comma = new byte[2];
    byte [] zero = new byte[1];
    byte [] one = new byte[1];

    quote[0] = (byte)'\"';
    comma[0] = (byte)',';
    comma[1] = (byte)'\n';
    zero[0] = (byte)'0';
    one[0] = (byte)'1';

    try
     {
      out.write(quote);

      for(int i = 3; i >= 0; i--)
       for(int j = 128, x = (int)buffer[i]; j > 0; j /= 2)
        if ((x & j) == 0)
         out.write(zero);
        else
         out.write(one);

      out.write(quote);
      out.write(comma);
     }
    catch(Exception e)
     {System.out.println("output file corupted!");};
   }

  /**
   * write code table vhdl coded to file
   * @param out the file stream
   */
  public static void writeVHDL(OutputStream out)
   {
    byte [] buffer = new byte[4];
    int linecount = 0;

    for(int i = 0; i <= 1; i++)
     for(int j = 0; j < pos[i];)
      {
       buffer[0] = memory[i][j++];
       buffer[1] = memory[i][j++];
       buffer[2] = memory[i][j++];
       buffer[3] = memory[i][j++];
       write4Bytes(out, buffer);
       linecount++;
      }

    buffer[0] = buffer[1] = buffer[2] = buffer[3] = (byte)Scanner.nop;

    while((linecount & (linecount - 1)) != 0)
     {
      write4Bytes(out, buffer);
      linecount++;
     }
   }

  /**
   * switch between resident and initial tables
   * @param f boolean
   */
  public static void resident(boolean f)
   {
    resident = f;
   }

  /**
   * get current entry position of code table
   * @return position
   */
  public static int last()
   {
    return resident?-pos[1] - 1:pos[0];
   }

  /**
   * get a code byte
   * @param index index into memory
   * @return single code byte
   */
  public static int get(int index)
   {
    return (int)memory[index < 0?1:0][index < 0?-index - 1:index];
   }

  /**
   * convert code table position to an absolute address
   * @param romable code romable or not
   * @param loc code table position
   * @return address
   */
  public static int address(boolean romable, int loc)
   {
    if (loc == Integer.MIN_VALUE)
     return -1;
    else if (loc < 0)
     return (-loc - 1) + (romable?(pos[0] + 3) / 4 * 4:4) + startAddress;
    else
     return loc + (!romable?((pos[1] + 3) / 4 * 4 + 8):0) + startAddress;
   }

  /**
   * get size of a distinct code segment
   * @param index the segment
   * @return size
   */
  public static int getSize(int index)
   {
    return pos[index];
   }

  /**
   * resize a distinct code buffer
   * @param index index of desired buffer
   * @return resized buffer
   */
  private static byte [] realloc(int index)
   {
    byte [] x = new byte[(memsize[index] += 256)];

    for(int i = 0; i < pos[index]; i++)
     x[i] = memory[index][i];

    return x;
   }

  /**
   * append to code table
   * @param x value to append
   */
  public static void appendCommand(byte x)
   {
    switch(x & 0x7f)
     {
      case Scanner.load:
      case Scanner.loadc:
      case Scanner.loadh:
      case Scanner.store:
      case Scanner.storec:
      case Scanner.storeh:
       loadstore++;
       break;
      case Scanner.val:
       val++;
       break;
      case Scanner.valueh:
       valh++;
       break;
      case Scanner.const0:
      case Scanner.const1:
      case Scanner.const2:
      case Scanner.const3:
      case Scanner.const4:
      case Scanner.const5:
      case Scanner.const6:
      case Scanner.const7:
      case Scanner.const8:
      case Scanner.const9:
      case Scanner.const10:
      case Scanner.const11:
      case Scanner.const12:
      case Scanner.const13:
      case Scanner.const14:
      case Scanner.const15:
      case Scanner.const1m:
      case Scanner.const2m:
      case Scanner.const3m:
      case Scanner.const4m:
      case Scanner.const5m:
      case Scanner.const6m:
      case Scanner.const7m:
      case Scanner.const8m:
      case Scanner.const9m:
      case Scanner.const10m:
      case Scanner.const11m:
      case Scanner.const12m:
      case Scanner.const13m:
      case Scanner.const14m:
      case Scanner.const15m:
       consts++;
       break;
      case Scanner.add:
      case Scanner.addc:
      case Scanner.and:
      case Scanner.cellp:
      case Scanner.dec:
      case Scanner.halfp:
      case Scanner.inc:
      case Scanner.invert:
      case Scanner.lshift:
      case Scanner.lshiftc:
      case Scanner.negate:
      case Scanner.or:
      case Scanner.rshift:
      case Scanner.rshiftc:
      case Scanner.sub:
      case Scanner.subc:
      case Scanner.cmp:
      case Scanner.twodiv:
      case Scanner.twomul:
      case Scanner.xor:
      case Scanner.fpu:
      case Scanner.op64:
      case Scanner.prod32:
       arithmetic++;
       break;
      case Scanner.zeroeq:
      case Scanner.zeroeqn:
      case Scanner.zerogt:
      case Scanner.zerolt:
      case Scanner.uzerogt:
      case Scanner.uzerolt:
      case Scanner.zerogtn:
      case Scanner.zeroltn:
      case Scanner.zerone:
      case Scanner.zeronen:
       relop++;
       break;
      case Scanner.branch:
      case Scanner.call:
      case Scanner.exit:
      case Scanner.trap:
      case Scanner.zerobranch:
      case Scanner.zerobranchn:
       flow++;
       break;
      default:
       misc++;
     }

    if ((x & 0x80) != 0)
     switch(x & 0x7f)
      {
       case Scanner.store:
       case Scanner.storec:
       case Scanner.storeh:
       case Scanner.twodrop:
       case Scanner.put:
       //case Scanner.storesp:
       case Scanner.op64:
        localOffset -= 2;
        break;
       case Scanner.drop:
       case Scanner.nip:
       case Scanner.add:
       case Scanner.sub:
       case Scanner.and:
       case Scanner.lshift:
       case Scanner.or:
       case Scanner.rshift:
       case Scanner.xor:
       case Scanner.addc:
       case Scanner.subc:
       case Scanner.lshiftc:
       case Scanner.rshiftc:
       case Scanner.zerobranch:
       case Scanner.zerobranchn:
       case Scanner.carrystore:
       case Scanner.setpc:
       case Scanner.fpu:
        localOffset -= 1;
        break;
       case Scanner.depth:
       case Scanner.val:
       case Scanner.valueh:
       case Scanner.const15:
       case Scanner.const14:
       case Scanner.const13:
       case Scanner.const12:
       case Scanner.const11:
       case Scanner.const10:
       case Scanner.const9:
       case Scanner.const8:
       case Scanner.const7:
       case Scanner.const6:
       case Scanner.const5:
       case Scanner.const4:
       case Scanner.const3:
       case Scanner.const2:
       case Scanner.const1:
       case Scanner.const0:
       case Scanner.const15m:
       case Scanner.const14m:
       case Scanner.const13m:
       case Scanner.const12m:
       case Scanner.const11m:
       case Scanner.const10m:
       case Scanner.const9m:
       case Scanner.const8m:
       case Scanner.const7m:
       case Scanner.const6m:
       case Scanner.const5m:
       case Scanner.const4m:
       case Scanner.const3m:
       case Scanner.const2m:
       case Scanner.const1m:
       case Scanner.dup:
       case Scanner.over:
       case Scanner.rat:
       case Scanner.r1at:
       case Scanner.zeroltn:
       case Scanner.zeroeqn:
       case Scanner.zeronen:
       case Scanner.zerogtn:
       case Scanner.cmp:
       case Scanner.carryload:
       case Scanner.loadsp:
       case Scanner.getcoreid:
       case Scanner.prod32:
        localOffset += 1;
        break;
       case Scanner.storesp:
        localOffset = 0;
       default:
        break;
      }

    newLine();
    append(x);
   }

   /**
    * new line in listing
    */
   static void newLine()
   {
    int index = resident?1:0;
    line = new Line(pos[index]);
    dasm[index].add(line);
   }

   /**
    * append text to listing
    * @param x String
    */
   static void appendText(String x)
   {
    line.appendText(x);
   }

   /**
    * append to code table
    * @param x value to append
    */
   public static void append(byte x)
    {
     int index = resident?1:0;

     if ((memsize[index] - pos[index]) < 1)
      memory[index] = realloc(index);

     memory[index][pos[index]++] = x;
    }

  /**
   * append to code table
   * @param x value to append
   */
  public static void append(short x)
   {
    append((byte)(x % 256));
    append((byte)(x >> 8));
   }

  /**
   * append to code table
   * @param x value to append
   */
  public static void append(int x)
   {
    append((byte)(x % 256));
    append((byte)(x >> 8));
    append((byte)(x >> 16));
    append((byte)(x >> 24));
   }

 /**
  * append to code table
  * @param alternate stack id
  * @param x value to append
  */
 public static void append(boolean alternate, long x)
  {
   int a = (int)(x >> 32);
   int b = (int)(x & 0xffffffffL);

   if (x > Integer.MAX_VALUE || x < Integer.MIN_VALUE)
    append(alternate, a);

   append(alternate, b);
  }

  /**
   * append to code table
   * @param alternate stack id
   * @param x value to append
   */
  public static void append(boolean alternate, int x)
   {
    int pre;

    switch(x)
     {
      case 15: pre = Scanner.const15; break;
      case 14: pre = Scanner.const14; break;
      case 13: pre = Scanner.const13; break;
      case 12: pre = Scanner.const12; break;
      case 11: pre = Scanner.const11; break;
      case 10: pre = Scanner.const10; break;
      case 9: pre = Scanner.const9; break;
      case 8: pre = Scanner.const8; break;
      case 7: pre = Scanner.const7; break;
      case 6: pre = Scanner.const6; break;
      case 5: pre = Scanner.const5; break;
      case 4: pre = Scanner.const4; break;
      case 3: pre = Scanner.const3; break;
      case 2: pre = Scanner.const2; break;
      case 1: pre = Scanner.const1; break;
      case 0: pre = Scanner.const0; break;
      case -15: pre = Scanner.const15m; break;
      case -14: pre = Scanner.const14m; break;
      case -13: pre = Scanner.const13m; break;
      case -12: pre = Scanner.const12m; break;
      case -11: pre = Scanner.const11m; break;
      case -10: pre = Scanner.const10m; break;
      case -9: pre = Scanner.const9m; break;
      case -8: pre = Scanner.const8m; break;
      case -7: pre = Scanner.const7m; break;
      case -6: pre = Scanner.const6m; break;
      case -5: pre = Scanner.const5m; break;
      case -4: pre = Scanner.const4m; break;
      case -3: pre = Scanner.const3m; break;
      case -2: pre = Scanner.const2m; break;
      case -1: pre = Scanner.const1m; break;
      default: pre = (((x >> 15) + 1) / 2 == 0)?Scanner.valueh:Scanner.val;
     }

    appendCommand((byte)((alternate?128:0) + pre));
    line.appendText(x + (alternate?" A:":" ") + "VAL");

    if (pre == Scanner.val)
     append(x);
    else if (pre == Scanner.valueh)
     append((short)x);
   }

  /**
   * replace in code table
   * @param pos code table position
   * @param offset offset in segment
   * @param x value to append
   */
  public static void replace(int pos, int offset, byte x)
   {
    int index = pos < 0?1:0;
    pos = (pos < 0?-pos - 1:pos) + offset;
    memory[index][pos] = (byte)x;
   }

  /**
   * replace in code table
   * @param pos code table position
   * @param offset an offset to position
   * @param x value to append
   */
  public static void replace(int pos, int offset, short x)
   {
    int index = pos < 0?1:0;
    pos = (pos < 0?-pos - 1:pos) + offset;
    memory[index][pos++] = (byte)x;
    memory[index][pos] = (byte)(x >> 8);
   }

  /**
   * replace in code table
   * @param pos code table position
   * @param offset offset in segment
   * @param x value to append
   */
  public static void replace(int pos, int offset, int x)
   {
    int index = pos < 0?1:0;
    pos = (pos < 0?-pos - 1:pos) + offset;
    memory[index][pos++] = (byte)(x % 256);
    memory[index][pos++] = (byte)(x >> 8);
    memory[index][pos++] = (byte)(x >> 16);
    memory[index][pos] = (byte)(x >> 24);
   }
 }

 class Line
  {
   int addr;
   String code;
   String text;

   Line(int x)
    {
     addr = x;
     code = "";
     text = "";
    }

   void appendText(String x)
    {
     text += x + ' ';
    }

   public String toString(int index, int offset, int length)
    {
     int a = addr + offset;
     String x = toHex((byte)(a >> 24)) + toHex((byte)(a >> 16)) + toHex((byte)(a >> 8)) + toHex((byte)(a));
     String y = "";
     for(int i = 0; i < length; i++)
      {
       int z = i + (index < 0?-index - 1:index);
       y += toHex((byte)Code.get(index < 0?-z - 1:z));
      }

     while(x.length() < 10)
      x += " ";
     while(y.length() < 12)
      y += " ";

     return x + y + " " + text;
    }

   private static String toHex(byte x)
    {
     final String alpha = "0123456789abcdef";

     return ("" + alpha.charAt((x >> 4) & 0xf)) + alpha.charAt(x & 0xf);
    }
  }
