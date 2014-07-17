package communicate;

/**
 * <p>Überschrift: Dateiverarbeitung</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.io.*;

public class MyFile
 {
  public final File file;
  RandomAccessFile ra;
  String mode;
  boolean delete;
  long rpos, wpos;

  /**
   * open random access file
   * @param file File
   * @param id int
   * @param append boolean
   * @param delete boolean
   * @throws Exception
   */
  public MyFile(File file, int id, boolean append, boolean delete) throws Exception
   {
    try
     {
      this.file = file;
      this.delete = delete;

      if (!file.exists())
       file.createNewFile();

      mode = ((id & 4) != 0)?"rw":"r";
      ra = new RandomAccessFile(file, mode);

      if (append)
       rpos = wpos = ra.length();
      else
       wpos = rpos = 0;
     }
    catch(Exception e) { throw e; }
   }

   /**
    * get length of file
    * @return long
    */
   public long size()
   {
    long pos = 0;

    try { pos = ra.length(); } catch (IOException ex) { }

    return pos;
   }

   /**
    * set file size
    * @param size long
    */
   public void setSize(long size)
   {
    try
     {
      ra.setLength(size);
      if (rpos > size)
       rpos = size;
      if (wpos > size)
       wpos = size;
     }
    catch (IOException ex)
     {
     }
   }

   /**
    * get file position
    * @return long
    */
   public long getFilePos()
   {
    return mode.equals("r")?rpos:(mode.equals("w")?wpos:Math.max(rpos, wpos));
   }

   /**
    * set file position
    * @param x long
    */
   public void setFilePos(long x)
   {
    try { ra.seek(x); rpos = x; wpos = x; } catch (IOException ex) { }
   }

   /**
    * get next byte
    * @return int
    */
   public int read()
   {
    int x = -1;

    try { x = ra.read(); rpos += (x != -1)?1:0; } catch (IOException ex) { }

    return x;
   }

   /**
    * get next line
    * @param length int
    * @return String
    */
   public String readLine(int length)
   {
    StringBuffer x = new StringBuffer();
    int z;

    do
     {
      z = -1;
      try
      {
       z = ra.read();
       if (z != -1)
        {
         rpos++;
         if (z != '\n')
          x.append((char)z);
        }
      }
      catch (IOException ex)
      {}
     }
    while(z != -1 && (char)z != '\n' && x.length() < length);

    return new String(x);
   }

   /**
    * write byte
    * @param x int
    */
   public void write(int x)
   {
    try { ra.write(x); wpos++; } catch (IOException ex) { }
   }

   /**
    * close file
    */
   public void close()
   {
    try
     {
      ra.close();
      if (delete)
       file.delete();
     }
    catch (IOException ex) { }
   }
 }
