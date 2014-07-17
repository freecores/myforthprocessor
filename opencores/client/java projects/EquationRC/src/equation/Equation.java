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
  {BasicIO.newLine();
   for(int i = 0; i < row.length; i++)
    {
     line = line.trim();
     int count = 0;
     while(count < line.length() && line.charAt(count) > ' ')
      count++;
     row[i] = Double.parseDouble(line.substring(0, count));
     line = line.substring(count);
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
       BasicIO.print("Solution: ");
       for(count = 0; count < b.length; count++)
        BasicIO.print(b[count] + " ");
       BasicIO.newLine();
       BasicIO.print("Enter right vector or empty line\n");
       line = BasicIO.readLine();
      }
    }
   catch(Exception e) { BasicIO.print(e.getMessage()); }
  }

 static
  {
   main(null);
  }
}
