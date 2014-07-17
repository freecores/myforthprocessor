package MyJava;

/**
 * <p>Überschrift: </p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright (c) 2005</p>
 * <p>Organisation: </p>
 * @author unbekannt
 * @version 1.0
 */

import java.io.*;
import java.util.*;

public class Compiler
{
 private static Process p = null;

 /**
  * compile source named in arg and produce loader list
  * @param arg sourcename
  * @param jdk path of jdk sources
  * @param editor name of editor
  * @param applet true, if called from an applet
  * @param force true, force recompilation of all necessary sources
  * @param gc name of garbage collection strategie
  * @return true, if compiled successfully
  */
 public static boolean compile(String arg, String jdk, String editor, boolean applet, boolean force, String gc)
  {
   File out = null;
   Vector built = new Vector();
   Vector toBuild = new Vector();
   toBuild.add(arg);

/*   if (true)
   {
    new Pass(new File((String)toBuild.get(0)), new Vector(), true); return false;
   }*/
   int error = 0;
   Header[] list;

   Parser.jdkSource = jdk;

   if (editor == null || editor.length() == 0)
    editor = "notepad";

   if (p != null)
    p.destroy();

   PrintStream old = System.out;
   if (applet)
    {
     try { out = File.createTempFile("report", null); } catch (IOException ex) { }
     try { System.setOut(new PrintStream(new FileOutputStream(out))); } catch (FileNotFoundException ex1) { }
     //try { p = Runtime.getRuntime().exec(editor + " " + out.getAbsolutePath()); } catch (IOException ex2) { }
    }

   //Starte Compiler
   System.out.println("Compiling...");


   int k = 0;
   do
    {
     Scope.reset();
     // Starte Parser
     File file = new File((String)toBuild.get(0));
     toBuild.remove(0);
     list = new Pass(file, built, force, gc).compilationUnit(null, 0);
     Scope.reset();

     if ((error = Errors.count()) != 0 || list == null)
      break;

     System.out.println("Iteration " + k++ + " completed");

     // sort
     for(int i = 1; i < list.length; i++)
      {
       int min = i;

       for (int j = i + 1; j < list.length; j++)
        if (list[j].depth < list[min].depth)
         min = j;

       Header x = list[i];
       list[i] = list[min];
       list[min] = x;
      }

     if (list.length > 0)
     try
      {
       String body = list[0].name.substring(0, list[0].name.lastIndexOf('.') + 1);
       FileWriter writer = new FileWriter(new File(body + "ld"));

       String dummy = "", imp = "***\n";
       for(int i = 0; i < list.length; i++)
        if (list[i].depth == 0)
         {
          dummy += list[i].name + '\n';
          if (!built.contains(list[i].name))
           built.add(list[i].name);
          toBuild.remove(list[i].name);
         }
        else
         {
          if (list[i].depth == 1)
           imp += list[i].name + '\n';
          toBuild.remove(list[i].name);
          if (!built.contains(list[i].name))
           toBuild.add(list[i].name);
         }

       writer.write(dummy, 0, dummy.length());
       writer.write(imp, 0, imp.length());
       writer.close();
      }
     catch (IOException ex) { ex.printStackTrace(); }

     list = null;
    }
   while(toBuild.size() > 0);

   if (error != 0)
    {
     System.out.println(error + (error > 1 ? " errors" : " error") + " found");
     toBuild.clear();
    }
   else
    System.out.println("Built successfully");

   if (applet)
    {
     System.out.close();
     System.setOut(old);

     try { p = Runtime.getRuntime().exec(editor + " " + out.getAbsolutePath()); } catch (IOException ex2) { }
    }

   return error == 0;
  }


  public static void main(String [] args)
   {
    compile("C:\\unzipped\\j2sdk-1_4_2-src-scsl\\j2se\\src\\share\\classes\\java\\lang\\MyObject.java",
            "C:\\unzipped\\j2sdk-1_4_2-src-scsl\\j2se\\src\\share\\classes", null, true, true, "3color");
    //compile("C:\\unzipped\\j2sdk-1_4_2-src-scsl\\j2se\\src\\share\\classes\\java\\lang\\Runtime.java",
    //        "C:\\unzipped\\j2sdk-1_4_2-src-scsl\\j2se\\src\\share\\classes", null, false, true);
   }
}
