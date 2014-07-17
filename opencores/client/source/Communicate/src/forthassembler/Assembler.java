package forthassembler;

/**
 * <p>Überschrift: Schnittstelle des Assembler</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */


import java.io.*;

public class Assembler
 {
  private static Process p = null;

  /**
   * assemble forth source
   * @param option String, "linkable" or "romable"
   * @param args String sourcename
   * @param template String template file for romable deck
   * @param blocksize String size of BRAM-block in byte
   * @param editor String name of preferred editor
   * @param report String name of report file
   * @param applet boolean true, if started from applet
   * @return boolean true, if successful
   */
  public static boolean assemble(String option, String args, String template, String blocksize, String editor, String report, boolean applet)
   {
    String name = null;
    File out = null;
    PrintStream old = System.out;
    int error = 0;

    if (editor == null || editor.length() == 0)
     editor = "notepad";

    if (args != null)
     {
      if (p != null)
       p.destroy();

      int pos = args.lastIndexOf(".");
      pos = pos == -1?args.length():pos;
      name = args.substring(0, pos);

      if (report != null)
       {
        out = new File(report);
        try { out.createNewFile(); } catch (IOException ex) { out = null; }
        try { System.setOut(new PrintStream(new FileOutputStream(out))); }
        catch (FileNotFoundException ex1) { out = null; }
       }
      //Starte Compiler
      System.out.println("Compiling...");
      // Init Scanner, Symboltabelle, Errors
      String [] tmp = new String[1];
      tmp[0] = args;
      Scanner.init(tmp);
      Code.init();
      Errors.reset();
      // Starte Parser
      Parser.parse(option.startsWith("r"), name);

      if ((error = Errors.count()) != 0)
       System.out.println(error + (error > 1?" errors":" error") + " found");
      else
       {
    	if (option.startsWith("r") && template != null && template.length() > 0)
    	 {
    	  // build VHDL-Code file
    	  buildVHDL(template, blocksize, name);
    	 }
    	
        System.out.println("Built successfully");
       }

      if (out != null)
       {
        System.out.close();
        System.setOut(old);
       }

      if (applet && error == 0)
       try { p = Runtime.getRuntime().exec(editor + " " + name + ".lst"); } catch (IOException ex2) { }
     }

    return error == 0;
   }
  
  static void buildVHDL(String template, String blocksize, String name)
   {
	try
	 {
	  // open source file
	  File dataInFile = new File(name + ".rom");
	  BufferedReader in = new BufferedReader(new FileReader(dataInFile));
	  // open template
	  File tmpl = new File(template);
	  String s = tmpl.getName();
	  int n = s.lastIndexOf('.');
	  s = s.substring(0, n > 0?n:s.length());
	  BufferedReader intmpl = new BufferedReader(new FileReader(tmpl));
	  // create output file
	  File dataOutFile = new File(dataInFile.getParent() + "\\" + s + ".vhd");
	  BufferedWriter out = new BufferedWriter(new FileWriter(dataOutFile, false));
	  // convert blocksize
	  int blks = Integer.parseInt(blocksize);
	  final int lineCount = (lineCount(in) * 4 + blks - 1) / blks * blks;
	  in.close();
	  in = new BufferedReader(new FileReader(dataInFile));
	  int count = Integer.MAX_VALUE;
	  String trailer = getTrailer(intmpl, "#");
	  
	  // process template
	  while(trailer.length() > 0 && trailer.endsWith("#"))
	   {
		write(out, trailer, trailer.length() - 1);
		// get pragma
		String cmd = getPragma(intmpl, "#");
		
		if (cmd.compareTo("#loop") == 0)
		 {
		  trailer = getTrailer(intmpl, "#endloop");
		  cmd = getPragma(intmpl, "#endloop");
		  trailer = trailer.substring(0, trailer.length() - 8);
		  execute(trailer, cmd, lineCount, in, out);
		 }
		else
		 {
		  if (cmd.compareTo("#r") == 0)
		   {
		    trailer = getTrailer(intmpl, "#e");
		    cmd = getPragma(intmpl, "#e");
		    trailer = trailer.substring(0, trailer.length() - 2);
		   }
	      else
		   trailer = "";
		 
		  // execute pragma
		  count = execute(trailer, cmd, blks, count, in, out);
		 }
		
		trailer = getTrailer(intmpl, "#");
	   }
	   
	  write(out, trailer, trailer.length());
	  
	  in.close();
	  out.close();
	  intmpl.close();
	 }
	catch (Exception e)
	 {
		// TODO Automatisch erstellter Catch-Block
		e.printStackTrace(System.out);
	 }
    
   }
  
  static void execute(String trailer, String cmd, int lineCount, BufferedReader in, BufferedWriter out) throws Exception
   {
	int i = 0;
	String s = "";
	
	if (trailer.length() > 0)
	 {
	  String z = "";
	  
	  for(i = 30; i > 0; i--)
	   if ((lineCount & (1 << i)) != 0)
	    {
	     String b = new String(trailer);
		 int e = b.indexOf('#');
	    
	     while(e >= 0)
	      {
	       // write trailer
	       z += b.substring(0, e);
	       // execute command
	       String c = "";
	       while(e < b.length() && b.charAt(e) > ' ')
	        c += b.charAt(e++);
	    
	       String t = executecmd(c, 1 << i, i, in);

	       z += t;
	       b = b.substring(e);
	       e = b.indexOf('#');
	      }
	    
	     z += b;
	     write(out, z, z.length());
	     z = "";

	     if ((lineCount & ((1 << i) - 1)) != 0)
	      {
	       if (cmd.startsWith("#endloop"))
	        z += cmd.substring(8);
	      }
	    }
	 }
	
	s = executecmd(cmd, 0, 0, in);
    write(out, s, s.length());
   }
  
  static int execute(String trailer, String cmd, int blks, int count, BufferedReader in, BufferedWriter out) throws Exception
   {
	int i = 0;
	String s = "";
	
	if (trailer.length() > 0)
	 {
	  String z = "";
	  
	  for(i = 0; i < count; i++)
	   {
	    String b = new String(trailer);
		int e = b.indexOf('#');
	    
	    while(e >= 0)
	     {
	      // write trailer
	      z += b.substring(0, e);
	      // execute command
	      String c = "";
	      while(e < b.length() && b.charAt(e) > ' ')
	       c += b.charAt(e++);
	      
	      String t = executecmd(c, blks, i, in);
	      
	      if (t.length() == 0)
	       {
	        b = z = t;
	        count = i;
	        break;
	       }
	      
	      z += t;
	      b = b.substring(e);
	      e = b.indexOf('#');
	     }
	    
	    z += b;
	    write(out, z, z.length());
	    z = "";

	    if (i != count)
	     {
	      if (cmd.startsWith("#ep"))
	       z += cmd.substring(3);
	     }
	   }
	 }
	
	s = executecmd(cmd, blks, count, in);
    write(out, s, s.length());
    
	return count;
   }
  
  static String executecmd(String cmd, int blks, int count, BufferedReader in) throws Exception
   {
	String s = "";
	
	cmd = cmd.substring(1);
	
	if (cmd.length() > 0 && cmd.charAt(0) == 'p')
	 cmd = cmd.substring(1);
	
	if (cmd.compareTo("i") == 0)
	 {
	  s += count;
	 }
	else if (cmd.compareTo("ends") == 0)
	 {
	  int i = 0;
	  for(;(blks & (1 << i)) == 0; i++);
	  s += i;
	 }
	else if (cmd.compareTo("blksize") == 0)
	 s += blks;
	else if (cmd.compareTo("blocksize") == 0)
	 s += blks;
	else if (cmd.compareTo("block") == 0)
	 {
	  boolean empty = true;
	  
	  for(int i = 0; i < blks; i += 4)
	   {
		String b = getTrailer(in, ",").trim();
		if (b.length() < 30)
		 b = "\"00000000000000000000000000000000\",";
		if (b.compareTo("\"00000000000000000000000000000000\",") != 0)
		 empty = false;
		s += b + '\n';
	   }
	  
	  s = s.substring(0, s.length() - 2);
	  if (empty)
	   s = "";
	 }
	else if (cmd.startsWith("ld("))
	 {
	  int l = -1;
	  
	  for(int j = blks; j != 0; l++)
	   j >>>= 1;
	   
	  s += l;
	 }
	else if (!(cmd.startsWith("e") || cmd.startsWith("loopend")))
	 s = cmd;
	
	return s;
   }
  
  static void write(BufferedWriter out, String b, int l) throws Exception
   {
	char prev = 0;
	char [] buff = new char[2];
	
	buff[0] = '\r';
	
	for(int i = 0; i < l; i++)
	  {
	   char c = b.charAt(i);
			  
	   buff[1] = c;
			  
	   out.write(buff, (c == '\n' && prev != '\r')?0:1, (c == '\n' && prev != '\r')?2:1);
			  
	   prev = c;
	  }
   }
  
  static String getTrailer(BufferedReader in, String trailer) throws Exception
   {
    String b = "";
    char [] c = new char[1];
    
    while(!(b.length() >= trailer.length() && b.endsWith(trailer)) &&
    		in.read(c, 0, c.length) > 0)
     b += c[0];

    return b;	
   }
  
  static String getPragma(BufferedReader in, String trailer) throws Exception
   {
	String b = new String(trailer);
    char [] c = new char[1];
	    
	 {
	  while(in.read(c, 0, c.length) > 0 && c[0] > ' ')
	   b += c[0];
	 }
	    
	return b;	
   }
  
  static int lineCount(BufferedReader in)
   {
    int lines = 0, empty = 0;
    String line;
    
    try
     {
      while((line = in.readLine()).length() > 0)
       if (line.compareTo("\"00000000000000000000000000000000\",") == 0)
    	empty++;
       else
        {
    	 lines += empty + 1;
    	 empty = 0;
        }
     }
    catch(Exception e) {}
    
    return lines;
   }
 }
