package sort1;

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
public class Sort
{
 public Sort()
  {
  }

 void bubble(String s)
  {
   char [] ch = s.toCharArray();
   boolean goon = true;
   int start = 0;

   do
    {
     goon = false;
     BasicIO.print("Iteration ");
     BasicIO.print(++start);
     BasicIO.print(":");
     BasicIO.print(new String(ch));
     BasicIO.newLine();
     for(int i = 0; i < ch.length - start; i++)
      if (ch[i] > ch[i + 1])
       {
        char x = ch[i];
        ch[i] = ch[i + 1];
        ch[i + 1] = x;
        goon = true;
       }
    }
   while(goon);
  }

 void quick(char [] ch, int offset, final int length, int iteration)
  {
   if (ch[offset] > ch[offset + length - 1])
    {
     char x = ch[offset];
     ch[offset] = ch[offset + length - 1];
     ch[offset + length - 1] = x;
    }
   if (length > 2)
    {
     int mid = offset + length / 2;
     if (ch[offset] > ch[mid])
      {
       char x = ch[offset];
       ch[offset] = ch[mid];
       ch[mid] = x;
      }
     else if (ch[mid] > ch[offset + length - 1])
      {
       char x = ch[mid];
       ch[mid] = ch[offset + length - 1];
       ch[offset + length - 1] = x;
      }
     if (length > 3)
      {
       int start = offset, end = offset + length - 1;
       char x = ch[mid];
       boolean goon = false;

       while(start <= end)
        {
         while(ch[start] < x)
          start++;
         while(ch[end] > x)
          end--;
         if (start <= end)
          {
           char y = ch[start];
           ch[start++] = ch[end];
           ch[end--] = y;
           goon = true;
          }
        }
        if (offset < end && goon)
         quick(ch, offset, end - offset + 1, iteration + 1);
        if (end + 1 < length + offset && goon)
         quick(ch, end + 1, length - (end + 1 - offset), iteration + 1);
      }
    }
   BasicIO.print("Depth ");
   BasicIO.print(iteration);
   BasicIO.print(":");
   BasicIO.print(new String(ch));
   BasicIO.newLine();
  }

 public static void main(String[] args)
  {
   Sort sort = new Sort();
   BasicIO.print(args[1]);
   BasicIO.newLine();
   if (args[0].charAt(0) == 'b')
    sort.bubble(args[1]);
   else
    sort.quick(args[1].toCharArray(), 0, args[1].length(), 0);
  }

}
