package MyJava;

/**
 * <p>Überschrift: Quellcode Analysator</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

import java.io.*;

public class Scanner
{
 private int ch;
 private String unget;
 public int col;
 public int line;
 private long pos;
 private BufferedReader reader;
 private char [] buffer;
 private int bufferStart, bufferEnd;
 private boolean print = false;
 public final String sourceName;

 public void trace(boolean on)
  {
   print = on;
  }

 /**
  * read a single character ch from input stream
  */
 private void nextCh()
  {
   ch = -1;

   try
    {
     if (unget.length() > 0)
      {
       ch = unget.charAt(0);
       unget = unget.substring(1);
      }
     else
      {
       // fill buffer, assign new source file
       while(bufferStart >= bufferEnd && reader != null)
        {
         bufferEnd = reader.read(buffer, 0, buffer.length);
         bufferStart = 0;
         if (bufferEnd < 0)
          {
           reader.close();
           reader = null;
          }
        }
       // get next character
       if (bufferStart < bufferEnd)
        {
         ch = (int)buffer[bufferStart++];
         if (ch == '\n')
          {
           line++;
           col = 0;
          }
         else
          col++;
         pos++;
        }
       else if (reader != null)
        {
         reader.close();
         reader = null;
        }
      }
    }
   catch (IOException e) { }

//   if (print && ch >= 0)
//    System.out.print((char)ch);
  }

  /**
   * preempt input stream and insert macro into stream
   * @param macro String
   */
  void insert(String macro)
  {
   unget = macro + " " + ((char)ch) + unget;
   nextCh();
  }

 /**
   * write erroneous line to error logfile
   *
   * @param     line source line
   * @param     col error in this column
   * @param     msg explanation
   */
 private void error(int line, int col, String msg)
  {
   Errors.println(sourceName, line, col, msg);
  }

 /**
   * initialize scanner
   *
   * @param     s input stream of source to scan
   */
 public Scanner(File file)
  {
   sourceName = file.getPath();

   if (!file.exists())
    {
     System.out.println(sourceName + " does not exist\n");
     System.exit(-1);
    }
   if (!file.canRead())
    {
     System.out.println(sourceName + " not readable\n");
     System.exit(-1);
    }
   if (!file.isFile())
    {
     System.out.println(sourceName + " is no file\n");
     System.exit(-1);
    }

   try { reader = new BufferedReader(new FileReader(file)); } catch (Exception ex) { }

   System.out.println(sourceName + " will be read");

   buffer = new char[512];
   bufferEnd = -1;
   bufferStart = -1;

   ch = ' ';
   unget = "";
   line = 1;
   col = 0;
   pos = 0;
  }

 /**
   * scan next token
   *
   * @return  next token
   */
 public Token next()
  {
   StringBuffer input = new StringBuffer();
   Token t = new Token();

   while(ch >= 0 && ch <= ' ')
    nextCh();

   t.source = sourceName;
   t.line = line;
   t.col = col;
   t.kind = Keyword.NONESY;
   t.val = 0;
   t.string = "";

   switch(ch)
    {
     case '#':
      while(ch > ' ')
       {
        input.append((char)ch);
        nextCh();
       }
      t.kind = Keyword.get(new String(input));
      break;
     case '"':
      nextCh();
      while(ch != '"' && ch > -1)
       {
        int x = (int)character(t);
        if (x == '\b')
         {
          input.append((char)'\\');
          input.append((char)'b');
         }
        else if (x == '\f')
         {
          input.append((char)'\\');
          input.append((char)'f');
         }
        else if (x == '\t')
         {
          input.append((char)'\\');
          input.append((char)'t');
         }
        else if (x == '\r')
         {
          input.append((char)'\\');
          input.append((char)'r');
         }
        else if (x == '\n')
         {
          input.append((char)'\\');
          input.append((char)'n');
         }
        else
         input.append((char)x);
        nextCh();
       }
      nextCh();
      t.kind = Keyword.STRINGSY;
      t.string = new String(input);
      break;
     case '\'':
      nextCh();
      t.val = character(t);
      input.append((char)t.val);
      t.string = new String(input);
      nextCh();
      if (ch != '\'')
       error(t.line,t.col,"invalid charConst");
      else
       {
        t.kind = Keyword.CHARCONSTSY;
        nextCh();
       }
      break;
     case '.':
      nextCh();
      if (ch >= '0' && ch <= '9')
       {
        unget = ((char)ch) + unget;
        ch = '.';
        t = floatNumber(t);
       }
      else
       {
        t.string = ".";
        for(;ch == '.'; nextCh())
         t.string += ".";

        if (t.string.length() > 1)
         {
          Errors.warning(t.source, t.line, t.col, t.string + " detected!", false);
          t = next();
         }
        else
         t.kind = Keyword.DOTSY;
       }
      break;
     case '/':
      nextCh();
      if (ch == '/')
       {
        for(nextCh(); ch != '\n' && ch >= 0; nextCh())
         input.append((char)ch);

        nextCh();
        t.kind = Keyword.SIMPLECOMMENTSY;
        t.string = new String(input);
       }
      else if (ch == '*')
       {
        int old = -1;

        for(nextCh(); ch >= 0 && (old != '*' || ch != '/'); old = ch, nextCh())
         if (old >= 0)
          input.append((char)old);

        t.kind = Keyword.COMMENTSY;
        t.string = new String(input);
        nextCh();
       }
      else
       {
        input.append('/');
        t = maxSpan(t, input);
       }
      break;
     case '0':
      nextCh();
      if (ch == 'x' || ch == 'X')
       {
        nextCh();
        t = hexNumber(t);
        t.string = "0x" + t.string;
       }
      else if (ch != '.')
       {
        unget = ((char)ch) + unget;
        ch = '0';
        t = octNumber(t);
       }
      else
       {
        unget = ((char)ch) + unget;
        ch = '0';
        t = floatNumber(t);
       }
      break;
     default:
      if (ch >= '0' && ch <= '9')
       {                           // number
        t = number(t);
       }
      else if (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch == '_' || ch == '$')
       {                           // identifier
        while(ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' ||
              ch >= '0' && ch <= '9' || ch == '_' || ch == '$')
         {
          input.append((char)ch);
          nextCh();
         }
        t.string = new String(input);
        if ((t.kind = Keyword.get(t.string)) == null)
         t.kind = Keyword.IDENTSY;
       }
      else if (ch != -1)
       {
        input.append((char)ch);
        nextCh();
        t = maxSpan(t, input);     // get maximum span
       }
      else
       t.kind = Keyword.EOFSY;

      break;
    }

   if (print)
    System.out.println(t.kind.string + ':' + t.string + " next: " + ((char)ch));

   return t;
  }

  /**
   * complete current token
   * @param t Token preliminary
   * @param input StringBuffer
   * @return Token completed
   */
  private Token maxSpan(Token t, StringBuffer input)
  {
   t.string  = new String(input);
   input.append((char)ch);

   while(ch > ' ' && (t.kind = Keyword.get(new String(input))) != null)
    {                         // get maximum span
     t.string = new String(input);
     nextCh();
     input.append((char)ch);
    }

   if ((t.kind = Keyword.get(t.string)) == null)
    {
     error(t.line,t.col,"invalid symbol: " + t.string);
     t.kind = Keyword.NONESY;
    }

   return t;
  }

  /**
   * get character
   * @param t Token
   * @return long
   */
  private long character(Token t)
  {
   long x = -1;

   if (ch == '\\')
    {
     nextCh();
     switch(ch)
      {
       case 'b': x = '\b'; break;
       case 't': x = '\t'; break;
       case 'n': x = '\n'; break;
       case 'f': x = '\f'; break;
       case 'r': x = '\r'; break;
       case 'u':
        nextCh();
        x = -1;
        if (ch == 'u')
         nextCh();
        if (ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F' || ch >= 'a' || ch <= 'f')
         {
          x = ch - (ch < 'A'?'0':((ch < 'a'?'A':'a') - 10));
          nextCh();
          if (ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F' || ch >= 'a' || ch <= 'f')
           {
            x = x * 16 + ch - (ch < 'A' ? '0' : ((ch < 'a' ? 'A' : 'a') - 10));
            nextCh();
            if (ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F' || ch >= 'a' || ch <= 'f')
             {
              x = x * 16 + ch - (ch < 'A' ? '0' : ((ch < 'a' ? 'A' : 'a') - 10));
              nextCh();
              if (ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F' || ch >= 'a' || ch <= 'f')
               {
                x = x * 16 + ch - (ch < 'A' ? '0' : ((ch < 'a' ? 'A' : 'a') - 10));
                nextCh();
               }
              else
               x = -1;
             }
            else
             x = -1;
           }
          else
           x = -1;
         }
        unget = ((char)ch) + unget;
        if (x == -1)
         error(t.line,t.col,"invalid charConst");
        break;
       default:
        if (ch >= '0' && ch <= '3')
         {
          x = ch - '0';
          nextCh();
          if ( ch >= '0' && ch <= '7')
           {
            x = x * 8 + ch - '0';
            nextCh();
           }
          if ( ch >= '0' && ch <= '7')
           {
            x = x * 8 + ch - '0';
            nextCh();
           }
          unget = ((char)ch) + unget;
         }
        else if (ch >= '0' && ch <= '7')
         {
          x = ch - '0';
          nextCh();
          if ( ch >= '0' && ch <= '7')
           {
            x = x * 8 + ch - '0';
            nextCh();
           }
          unget = ((char)ch) + unget;
         }
        else
         x = ch;
       }
     }
    else
     x = ch;

    return x;
  }

  /**
   * get octal number
   * @param t Token
   * @return Token
   */
  private Token octNumber(Token t)
  {
   StringBuffer x = new StringBuffer();

   while(ch >= '0' && ch < '8')
    {
     x.append((char)ch);
     t.val = t.val * 8 + ch - '0';
     nextCh();
    }

   if (ch == 'l' || ch == 'L')
    {
     t.kind = Keyword.LNUMBERSY;
     nextCh();
    }
   else
    t.kind = Keyword.NUMBERSY;

   t.string = new String(x);

   return t;
  }

  /**
   * get hexadecimal number
   * @param t Token
   * @return Token
   */
  private Token hexNumber(Token t)
  {
   StringBuffer x = new StringBuffer();

   while(ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F' || ch >= 'a' && ch <= 'f')
    {
     x.append((char)ch);
     t.val = t.val * 16 + ch - (ch < 'A'?'0':((ch < 'a'?'A':'a') - 10));
     nextCh();
    }

   if (ch == 'l' || ch == 'L')
    {
     t.kind = Keyword.LNUMBERSY;
     nextCh();
    }
   else
    t.kind = Keyword.NUMBERSY;

   t.string = new String(x);

   return t;
  }

  /**
   * get floating point number
   * @param t Token
   * @return Token
   */
  private Token floatNumber(Token t)
   {
    StringBuffer buff = new StringBuffer(t.string);

    while(ch >= '0' && ch <= '9')
     {
      buff.append((char)ch);
      nextCh();
     }

    if (ch == '.')
     {
      buff.append('.');
      nextCh();
      while(ch >= '0' && ch <= '9')
       {
        buff.append((char)ch);
        nextCh();
       }
     }


    if (ch == 'E' || ch == 'e')
     {
      buff.append('e');
      nextCh();

      if (ch == '+' || ch == '-')
       {
        buff.append((char)ch);
        nextCh();
       }

      while(ch >= '0' && ch <= '9')
       {
        buff.append((char)ch);
        nextCh();
       }
     }

   if (ch == 'f' || ch == 'F' || ch == 'd' || ch == 'D')
    {
     nextCh();
    }

   t.string = new String(buff);
   try { t.fval = Double.parseDouble(t.string); t.kind = Keyword.DNUMBERSY; } catch(NumberFormatException ex) { }

   return t;
  }

  /**
   * get decimal number
   * @param t Token
   * @return Token
   */
  private Token number(Token t)
  {
   StringBuffer input = new StringBuffer();

   while (ch >= '0' && ch <= '9')
    {
     t.val = t.val * 10 + ch - '0';
     input.append((char)ch);
     nextCh();
    }

   if ((ch == 'l' || ch == 'L') && input.length() > 0)
    {
     t.kind = Keyword.LNUMBERSY;
     nextCh();
    }
   else if (ch == '.' || ch == 'e' || ch == 'E' || ch == 'f' || ch == 'F' || ch == 'd' || ch == 'D')
    {
     t.string = new String(input);
     t = floatNumber(t);
     input = new StringBuffer(t.string);
    }
   else
    t.kind = Keyword.NUMBERSY;

   t.string = new String(input);

   return t;
  }
}
