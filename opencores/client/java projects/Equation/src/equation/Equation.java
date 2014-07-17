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
public class Equation
{
 private static void fillRow(double [] row, String line)
  {
   int st, end = 0;

   for(int i = 0; i < row.length; i++)
    {
     for(st = end; st < line.length() && line.charAt(st) <= ' '; st++);
     for(end = st; end < line.length() && line.charAt(end) > ' '; end++);
     row[i] = Double.parseDouble(line.substring(st, end));
    }
  }

 public static void main(String[] args)
  {
   BasicIO.print("Enter coeffizients of first row of equation\n");
   String line = BasicIO.readLine();
   int count = 0;
   boolean blank = true;
   for(int i = 0; i < line.length(); i++)
    if (line.charAt(i) <= ' ')
     blank = true;
    else if (blank)
     {
      blank = false;
      count++;
     }
   double [][] mat = new double[count][count];
   double [] b = new double[count];
   fillRow(mat[0], line);
   for(count = 1; count < mat.length; count++)
    {
     BasicIO.print("Enter coeffizients of " + (count + 1) + ". row of equation\n");
     fillRow(mat[count], BasicIO.readLine());
    }

   try
    {
     Gauss g = new Gauss(mat);

     BasicIO.print("Enter right vector or empty line\n");
     line = BasicIO.readLine();
     while(line.length() > 0)
      {
       fillRow(b, line);
       b = g.solve(b);
       #ass "1 LOCKI/O";
       BasicIO.print("Solution: ");
       for(count = 0; count < b.length; count++)
        BasicIO.print(b[count] + " ");
       BasicIO.newLine();
       #ass "-1 LOCKI/O";
       BasicIO.print("Enter right vector or empty line\n");
       line = BasicIO.readLine();
      }
    }
   catch(Exception e) { BasicIO.print(e.getMessage()); }
  }
private static Thread x;
 static
  {
   class eq extends Thread
    {
     void run()
      {
       main(null);
       exit();
      }
    }

   (x = new eq()).start();

   //main(null);
  }
}
