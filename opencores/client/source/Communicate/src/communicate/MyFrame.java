package communicate;

import java.util.*;
import java.io.*;
import javax.swing.*;
import java.awt.*;
import javax.swing.border.*;
import java.awt.event.*;

/**
 * <p>Überschrift: Oberfläche des FORTH-Client</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

public final class MyFrame extends JFrame implements ActionListener
 {
  static final long serialVersionUID = 1028;
  static MyFrame myframe;
  final String propertyFile = "myProperties";
  File myProject = new File("C:/");
  File home = new File("C:/");
  int dynmem = 1000000;
  String myEditor = "notepad";
  String baudrate = "9600";
  String stopbits = "1";
  String parity = "none";
  String option = "linkable";
  String template = "";
  String blocksize = "2048";
  String com = "COM1";
  String control = "NONE";
  String translate = "translate all newly";
  String gc = "tricolor marking";
  double oscilator = 50.0;
  String dcm = "DCM";
  File sun = new File("C:/");

  Output consoleOut = new Output(876, 325);
  Output status = new Output(876, 47);
  Input consoleIn = new Input(876, 325);
  Serial myport = null;

  Properties properties;
  boolean bounce = false;

  LayoutManager verticalFlowLayout1 = new FlowLayout(FlowLayout.LEADING, 0, 0);

  JMenuBar TheMenueBar = new JMenuBar();

  JMenu FileMenue1 = new JMenu();
  JMenu SerialMenue1 = new JMenu();
  JMenu CompileMenue1 = new JMenu();
  JMenu JavaMenue1 = new JMenu();
  JMenu CommandMenue1 = new JMenu();
  JMenu ConsoleMenue1 = new JMenu();
  JMenu DictionaryMenue1 = new JMenu();
  JMenu GCMenue1 = new JMenu();
  JMenu TestMenue1 = new JMenu();
  JMenu MiscMenue1 = new JMenu();

  JMenuItem FileMenueItem12 = new JMenuItem();
  JMenuItem FileMenueItem13 = new JMenuItem();
  JMenuItem FileMenueItem14 = new JMenuItem();
  JMenuItem FileMenueItem15 = new JMenuItem();
  JMenuItem FileMenueItem16 = new JMenuItem();
  JMenuItem FileMenueItem111 = new JMenuItem();
  JMenuItem FileMenueItem112 = new JMenuItem();

  JMenuItem SerialMenueItem11 = new JMenuItem();
  JMenuItem SerialMenueItem12 = new JMenuItem();
  JMenuItem SerialMenueItem13 = new JMenuItem();
  JMenuItem SerialMenueItem14 = new JMenuItem();
  JMenuItem SerialMenueItem15 = new JMenuItem();
  JMenuItem SerialMenueItem16 = new JMenuItem();
  JMenuItem SerialMenueItem17 = new JMenuItem();

  JMenuItem CompileMenueItem11 = new JMenuItem();
  JMenuItem CompileMenueItem12 = new JMenuItem();
  JMenuItem CompileMenueItem13 = new JMenuItem();
  JMenuItem CompileMenueItem14 = new JMenuItem();
  JMenuItem CompileMenueItem15 = new JMenuItem();

  JMenuItem JavaMenueItem0 = new JMenuItem();
  JMenuItem JavaMenueItem1 = new JMenuItem();
  JMenuItem JavaMenueItem2 = new JMenuItem();
  JMenuItem JavaMenueItem3 = new JMenuItem();
  JMenuItem JavaMenueItem4 = new JMenuItem();

  JMenuItem DictionaryMenueItem1 = new JMenuItem();
  JMenuItem DictionaryMenueItem2 = new JMenuItem();
  JMenuItem DictionaryMenueItem3 = new JMenuItem();
  JMenuItem DictionaryMenueItem4 = new JMenuItem();
  JMenuItem DictionaryMenueItem5 = new JMenuItem();

  JMenuItem CommandMenue11 = new JMenuItem();
  JMenuItem CommandMenue12 = new JMenuItem();

  JMenuItem OutMenue11 = new JMenuItem();
  JMenuItem OutMenue12 = new JMenuItem();

  JMenuItem InMenue11 = new JMenuItem();
  JMenuItem InMenue12 = new JMenuItem();
  JMenuItem InMenue13 = new JMenuItem();
  JMenuItem InMenue14 = new JMenuItem();

  JMenuItem GCMenueItem11 = new JMenuItem();
  JMenuItem GCMenueItem12 = new JMenuItem();
  JMenuItem GCMenueItem13 = new JMenuItem();
  JMenuItem GCMenueItem14 = new JMenuItem();
  JMenuItem GCMenueItem15 = new JMenuItem();

  JMenuItem testMenue = new JMenuItem();
  JMenuItem MiscMenueItem1 = new JMenuItem();
  JMenuItem MiscMenueItem2 = new JMenuItem();
  JMenuItem MiscMenueItem3 = new JMenuItem();
  JMenuItem MiscMenueItem4 = new JMenuItem();

  TitledBorder titledBorder1;
  TitledBorder titledBorder2;
  TitledBorder titledBorder3;
  TitledBorder titledBorder4;
  TitledBorder titledBorder5;
  TitledBorder titledBorder6;
  TitledBorder titledBorder7;
  TitledBorder titledBorder8;
  TitledBorder titledBorder9;
  TitledBorder titledBorder10;
  TitledBorder titledBorder11;
  TitledBorder titledBorder12;

 public MyFrame()
   { myframe = this;
    titledBorder1 = new TitledBorder("");
    titledBorder2 = new TitledBorder("");
    titledBorder3 = new TitledBorder("");
    titledBorder4 = new TitledBorder("");
    titledBorder5 = new TitledBorder("");
    titledBorder6 = new TitledBorder("");
    titledBorder7 = new TitledBorder("");
    titledBorder8 = new TitledBorder("");
    titledBorder9 = new TitledBorder("");
    titledBorder10 = new TitledBorder("");
    titledBorder11 = new TitledBorder("");
    titledBorder12 = new TitledBorder("");

    this.setJMenuBar(TheMenueBar);
    this.getContentPane().add(consoleOut);
    this.getContentPane().add(consoleIn);
    this.getContentPane().add(status);
    this.getContentPane().setLayout(verticalFlowLayout1);

    TheMenueBar.setBackground(Color.lightGray);
    TheMenueBar.setFont(new java.awt.Font("Tahoma", 0, 18));
    TheMenueBar.setBorder(titledBorder2);

    TestMenue1.setBackground(Color.lightGray);
    TestMenue1.setFont(new java.awt.Font("Tahoma", 0, 18));
    TestMenue1.setBorder(titledBorder12);
    TestMenue1.setText("Test");

    testMenue.setFont(new java.awt.Font("Tahoma", 0, 16));
    testMenue.setFocusPainted(false);
    testMenue.setText("test serial");
    testMenue.addActionListener(this);

    TestMenue1.add(testMenue);

    FileMenueItem111.setFont(new java.awt.Font("Tahoma", 0, 16));
    FileMenueItem111.setFocusPainted(false);
    FileMenueItem111.setText("New project");
    FileMenueItem111.addActionListener(this);
    FileMenueItem112.setFont(new java.awt.Font("Tahoma", 0, 16));
    FileMenueItem112.setText("New file");
    FileMenueItem112.addActionListener(this);
    FileMenueItem112.addActionListener(this);
    FileMenueItem12.setFont(new java.awt.Font("Tahoma", 0, 16));
    FileMenueItem12.setText("Open project");
    FileMenueItem12.addActionListener(this);
    FileMenueItem13.setFont(new java.awt.Font("Tahoma", 0, 16));
    FileMenueItem13.setText("Open file");
    FileMenueItem13.addActionListener(this);
    FileMenueItem14.setFont(new java.awt.Font("Tahoma", 0, 16));
    FileMenueItem14.setText("Close project");
    FileMenueItem14.addActionListener(this);
    FileMenueItem15.setFont(new java.awt.Font("Tahoma", 0, 16));
    FileMenueItem15.setText("Select editor");
    FileMenueItem15.addActionListener(this);
    FileMenueItem16.setFont(new java.awt.Font("Tahoma", 0, 16));
    FileMenueItem16.setText("Exit");
    FileMenueItem16.addActionListener(this);
    FileMenue1.setBackground(Color.lightGray);
    FileMenue1.setFont(new java.awt.Font("Tahoma", 0, 18));
    FileMenue1.setBorder(titledBorder4);
    FileMenue1.setText("File");

    SerialMenue1.setBackground(Color.lightGray);
    SerialMenue1.setFont(new java.awt.Font("Tahoma", 0, 18));
    SerialMenue1.setBorder(titledBorder6);
    SerialMenue1.setText("Serial");

    SerialMenueItem11.setFont(new java.awt.Font("Tahoma", 0, 16));
    SerialMenueItem11.setText("Port");
    SerialMenueItem11.addActionListener(this);
    SerialMenueItem12.setFont(new java.awt.Font("Tahoma", 0, 16));
    SerialMenueItem12.setText("Stopbits");
    SerialMenueItem12.addActionListener(this);
    SerialMenueItem13.setFont(new java.awt.Font("Tahoma", 0, 16));
    SerialMenueItem13.setText("Parity");
    SerialMenueItem13.addActionListener(this);
    SerialMenueItem14.setFont(new java.awt.Font("Tahoma", 0, 16));
    SerialMenueItem14.setText("Baudrate");
    SerialMenueItem14.addActionListener(this);
    SerialMenueItem15.setFont(new java.awt.Font("Tahoma", 0, 16));
    SerialMenueItem15.setText("Force changes");
    SerialMenueItem15.addActionListener(this);
    SerialMenueItem16.setFont(new java.awt.Font("Tahoma", 0, 16));
    SerialMenueItem16.setText("Default values");
    SerialMenueItem16.addActionListener(this);
    SerialMenueItem17.setFont(new java.awt.Font("Tahoma", 0, 16));
    SerialMenueItem17.setText("Control");
    SerialMenueItem17.addActionListener(this);

    CompileMenue1.setBackground(Color.lightGray);
    CompileMenue1.setFont(new java.awt.Font("Tahoma", 0, 18));
    CompileMenue1.setBorder(titledBorder8);
    CompileMenue1.setText("FORTH");

    CompileMenueItem11.setFont(new java.awt.Font("Tahoma", 0, 16));
    CompileMenueItem11.setText("Option");
    CompileMenueItem11.addActionListener(this);
    CompileMenueItem12.setFont(new java.awt.Font("Tahoma", 0, 16));
    CompileMenueItem12.setText("File");
    CompileMenueItem12.addActionListener(this);
    CompileMenueItem13.setFont(new java.awt.Font("Tahoma", 0, 16));
    CompileMenueItem13.setText("File list");
    CompileMenueItem13.addActionListener(this);
    CompileMenueItem14.setFont(new java.awt.Font("Tahoma", 0, 16));
    CompileMenueItem14.setText("Template");
    CompileMenueItem14.addActionListener(this);
    CompileMenueItem15.setFont(new java.awt.Font("Tahoma", 0, 16));
    CompileMenueItem15.setText("Blocksize");
    CompileMenueItem15.addActionListener(this);

    JavaMenue1.setBackground(Color.lightGray);
    JavaMenue1.setFont(new java.awt.Font("Tahoma", 0, 18));
    JavaMenue1.setBorder(titledBorder10);
    JavaMenue1.setText("Java");

    JavaMenueItem0.setFont(new java.awt.Font("Tahoma", 0, 16));
    JavaMenueItem0.setText("Option");
    JavaMenueItem0.addActionListener(this);
    JavaMenueItem1.setFont(new java.awt.Font("Tahoma", 0, 16));
    JavaMenueItem1.setText("To Forth");
    JavaMenueItem1.addActionListener(this);
    JavaMenueItem2.setFont(new java.awt.Font("Tahoma", 0, 16));
    JavaMenueItem2.setText("Sun sources");
    JavaMenueItem2.addActionListener(this);
    JavaMenueItem3.setFont(new java.awt.Font("Tahoma", 0, 16));
    JavaMenueItem3.setText("home");
    JavaMenueItem3.addActionListener(this);
    JavaMenueItem4.setFont(new java.awt.Font("Tahoma", 0, 16));
    JavaMenueItem4.setText("gc");
    JavaMenueItem4.addActionListener(this);

    DictionaryMenue1.setBackground(Color.lightGray);
    DictionaryMenue1.setFont(new java.awt.Font("Tahoma", 0, 18));
    DictionaryMenue1.setBorder(titledBorder10);
    DictionaryMenue1.setText("Dictionary");
    //DictionaryMenue1.setEnabled(false);

    DictionaryMenueItem1.setFont(new java.awt.Font("Tahoma", 0, 16));
    DictionaryMenueItem1.setText("list");
    DictionaryMenueItem1.addActionListener(this);
    DictionaryMenueItem2.setFont(new java.awt.Font("Tahoma", 0, 16));
    DictionaryMenueItem2.setText("load Forth module");
    DictionaryMenueItem2.addActionListener(this);
    DictionaryMenueItem3.setFont(new java.awt.Font("Tahoma", 0, 16));
    DictionaryMenueItem3.setText("load Java modules");
    DictionaryMenueItem3.addActionListener(this);
    DictionaryMenueItem4.setFont(new java.awt.Font("Tahoma", 0, 16));
    DictionaryMenueItem4.setText("initialize Java modules");
    DictionaryMenueItem4.addActionListener(this);
    DictionaryMenueItem5.setFont(new java.awt.Font("Tahoma", 0, 16));
    DictionaryMenueItem5.setText("unload");
    DictionaryMenueItem5.addActionListener(this);

    CommandMenue1.setBackground(Color.lightGray);
    CommandMenue1.setFont(new java.awt.Font("Tahoma", 0, 18));
    CommandMenue1.setBorder(titledBorder12);
    CommandMenue1.setText("Command");
    //CommandMenue1.setEnabled(false);

    CommandMenue11.setFont(new java.awt.Font("Tahoma", 0, 16));
    CommandMenue11.setText("Reset");
    CommandMenue11.addActionListener(this);
    CommandMenue12.setFont(new java.awt.Font("Tahoma", 0, 16));
    CommandMenue12.setText("Abort");
    CommandMenue12.addActionListener(this);

    ConsoleMenue1.setBackground(Color.lightGray);
    ConsoleMenue1.setFont(new java.awt.Font("Tahoma", 0, 18));
    ConsoleMenue1.setBorder(titledBorder12);
    ConsoleMenue1.setText("Console");

    OutMenue11.setFont(new java.awt.Font("Tahoma", 0, 16));
    OutMenue11.setText("Clear output");
    OutMenue11.addActionListener(this);
    OutMenue12.setFont(new java.awt.Font("Tahoma", 0, 16));
    OutMenue12.setText("Save output as");
    OutMenue12.addActionListener(this);
    InMenue11.setFont(new java.awt.Font("Tahoma", 0, 16));
    InMenue11.setText("Clear input");
    InMenue11.addActionListener(this);
    InMenue12.setFont(new java.awt.Font("Tahoma", 0, 16));
    InMenue12.setText("Save input as");
    InMenue12.addActionListener(this);
    InMenue13.setFont(new java.awt.Font("Tahoma", 0, 16));
    InMenue13.setText("Execute input");
    InMenue13.addActionListener(this);
    InMenue13.setEnabled(false);
    InMenue14.setFont(new java.awt.Font("Tahoma", 0, 16));
    InMenue14.setText("Load file");
    InMenue14.addActionListener(this);

    GCMenueItem11.setFont(new java.awt.Font("Tahoma", 0, 16));
    GCMenueItem11.setText("available");
    GCMenueItem11.addActionListener(this);
    GCMenueItem12.setFont(new java.awt.Font("Tahoma", 0, 16));
    GCMenueItem12.setText("allocate");
    GCMenueItem12.addActionListener(this);
    GCMenueItem13.setFont(new java.awt.Font("Tahoma", 0, 16));
    GCMenueItem13.setText("free");
    GCMenueItem13.addActionListener(this);
    GCMenueItem14.setFont(new java.awt.Font("Tahoma", 0, 16));
    GCMenueItem14.setText("reset");
    GCMenueItem14.addActionListener(this);
    GCMenueItem15.setFont(new java.awt.Font("Tahoma", 0, 16));
    GCMenueItem15.setText("default size");
    GCMenueItem15.addActionListener(this);
    GCMenue1.setBackground(Color.lightGray);
    GCMenue1.setFont(new java.awt.Font("Tahoma", 0, 18));
    GCMenue1.setBorder(titledBorder4);
    GCMenue1.setText("Dynamic Memory");
    //GCMenue1.setEnabled(false);

    MiscMenueItem1.setFont(new java.awt.Font("Tahoma", 0, 16));
    MiscMenueItem1.setText("About");
    MiscMenueItem1.addActionListener(this);
    MiscMenueItem2.setFont(new java.awt.Font("Tahoma", 0, 16));
    MiscMenueItem2.setText("DCM or PLL");
    MiscMenueItem2.addActionListener(this);
    MiscMenueItem3.setFont(new java.awt.Font("Tahoma", 0, 16));
    MiscMenueItem3.setText("oscilator");
    MiscMenueItem3.addActionListener(this);
    MiscMenueItem4.setFont(new java.awt.Font("Tahoma", 0, 16));
    MiscMenueItem4.setText("frequency");
    MiscMenueItem4.addActionListener(this);

    TheMenueBar.add(FileMenue1);
    TheMenueBar.add(SerialMenue1);
    TheMenueBar.add(CompileMenue1);
    TheMenueBar.add(JavaMenue1);
    TheMenueBar.add(DictionaryMenue1);
    TheMenueBar.add(CommandMenue1);
    TheMenueBar.add(GCMenue1);
    TheMenueBar.add(ConsoleMenue1);
    //TheMenueBar.add(testMenue);
    TheMenueBar.add(MiscMenue1);

    FileMenue1.add(FileMenueItem111);
    FileMenue1.add(FileMenueItem112);
    FileMenue1.add(FileMenueItem12);
    FileMenue1.add(FileMenueItem13);
    FileMenue1.add(FileMenueItem14);
    FileMenue1.add(FileMenueItem15);
    FileMenue1.add(FileMenueItem16);

    SerialMenue1.add(SerialMenueItem11);
    SerialMenue1.add(SerialMenueItem12);
    SerialMenue1.add(SerialMenueItem13);
    SerialMenue1.add(SerialMenueItem14);
    SerialMenue1.add(SerialMenueItem17);
    SerialMenue1.add(SerialMenueItem15);
    SerialMenue1.add(SerialMenueItem16);

    CompileMenue1.add(CompileMenueItem11);
    CompileMenue1.add(CompileMenueItem14);
    CompileMenue1.add(CompileMenueItem15);
    CompileMenue1.add(CompileMenueItem12);
    CompileMenue1.add(CompileMenueItem13);

    JavaMenue1.add(JavaMenueItem0);
    JavaMenue1.add(JavaMenueItem4);
    JavaMenue1.add(JavaMenueItem1);
    JavaMenue1.add(JavaMenueItem2);
    JavaMenue1.add(JavaMenueItem3);

    DictionaryMenue1.add(DictionaryMenueItem1);
    DictionaryMenue1.add(DictionaryMenueItem2);
    DictionaryMenue1.add(DictionaryMenueItem3);
    DictionaryMenue1.add(DictionaryMenueItem4);
    DictionaryMenue1.add(DictionaryMenueItem5);

    CommandMenue1.add(CommandMenue11);
    CommandMenue1.add(CommandMenue12);

    ConsoleMenue1.add(OutMenue11);
    ConsoleMenue1.add(OutMenue12);
    ConsoleMenue1.add(InMenue11);
    ConsoleMenue1.add(InMenue12);
    ConsoleMenue1.add(InMenue14);
    ConsoleMenue1.add(InMenue13);

    GCMenue1.add(GCMenueItem11);
    GCMenue1.add(GCMenueItem12);
    GCMenue1.add(GCMenueItem13);
    GCMenue1.add(GCMenueItem14);
    GCMenue1.add(GCMenueItem15);

    MiscMenue1.setBackground(Color.lightGray);
    MiscMenue1.setFont(new java.awt.Font("Tahoma", 0, 18));
    MiscMenue1.setBorder(titledBorder8);
    MiscMenue1.setText("Misc");

    MiscMenue1.add(MiscMenueItem2);
    MiscMenue1.add(MiscMenueItem3);
    MiscMenue1.add(MiscMenueItem4);
    MiscMenue1.add(MiscMenueItem1);

    properties = new Properties();

    try
     {
      FileInputStream stream = new FileInputStream(new File(propertyFile));
      properties.load(stream);

      String x = properties.getProperty("myProject");
      if (x != null)
       myProject = new File(x);
      x = properties.getProperty("home");
      if (x != null)
       home = new File(x);
      x = properties.getProperty("myEditor");
      if (x != null)
       myEditor = x;
      x = properties.getProperty("option");
      if (x != null)
       option = x;
      x = properties.getProperty("template");
      if (x != null)
       template = x;
      x = properties.getProperty("blocksize");
      if (x != null)
       blocksize = x;
      x = properties.getProperty("translate");
      if (x != null)
       translate = x;
      x = properties.getProperty("gc");
      if (x != null)
       gc = x;
      x = properties.getProperty("sun");
      if (x != null)
       sun = new File(x);
      x = properties.getProperty("com");
      if (x != null)
       com = x;
      x = properties.getProperty("parity");
      if (x != null)
       parity = x;
      x = properties.getProperty("stopbits");
      if (x != null)
       stopbits = x;
      x = properties.getProperty("baudrate");
      if (x != null)
       baudrate = x;
      x = properties.getProperty("control");
      if (x != null)
       control = x;
      x = properties.getProperty("dynmem");
      if (x != null)
       dynmem = Integer.parseInt(x);
      x = properties.getProperty("dcm");
      if (x != null)
       dcm = x;
      x = properties.getProperty("oscilator");
      if (x != null)
       oscilator = Double.parseDouble(x);
    }
    catch (Exception ex)
     {
      consoleOut.addText("Exception: " + ex.getMessage());
     }


    status.setText("ready ");
    this.setPreferredSize(new Dimension(890, 800));
   }

  /**
   * build list of dependent files involved in a distinct application
   * @param name name of source
   * @return list of involved java files
   */
  private Vector buildLoaderList(String name)
   {
    Vector map = new Vector();
    Vector toLoad = new Vector();

    while(true)
     {
      String trunc = name.substring(0, name.lastIndexOf('.') + 1);

      try
       {
        BufferedReader r = new BufferedReader(new FileReader(trunc + "ld"));
        // package
        for(String line = r.readLine(); line != null && !line.startsWith("***"); line = r.readLine())
         {
          if (map.contains(line))
           map.remove(line);
          map.add(line);
          toLoad.remove(line);
         }
        // imports
        for(String line = r.readLine(); line != null; line = r.readLine())
         {
          if (map.contains(line))
           {
            map.remove(line);
            map.add(line);
            toLoad.remove(line);
           }
          else
           {
            toLoad.remove(line);
            toLoad.add(line);
           }
         }
       }
      catch (Exception ex) { ex.printStackTrace(); }

      if (toLoad.size() == 0)
       break;

      name = (String)toLoad.get(0);
      toLoad.remove(0);
     }

    return map;
   }

  /**
   * assemble a FORTH source file
   * @param option romable or linkable
   * @param path source name
   * @param hide hide report, if assembled successfully
   * @param show show listing or not
   * @return true, if assembled successfully
   */
  private boolean assemble(String option, String path, boolean hide, boolean show)
   {
    String res = "report.tmp";
    status.setText("assembling...");

    boolean done = forthassembler.Assembler.assemble(option, path, template, blocksize, myEditor, res, show);

    File f = new File(res);
    long size = f.length();
    byte [] buff = new byte[(int)size];
    FileInputStream stream = null;
    try
     {
      stream = new FileInputStream(f);
      stream.read(buff);
      if (!done || !hide)
       consoleOut.addText(new String(buff));
      stream.close();
      f.delete();
     }
    catch (Exception ex5) { consoleOut.addText("Exception: " + ex5.getMessage()); }

    status.setText(done?"done":"aborted");

    return done;
   }

  /**
   * assemble all dependent FORTH-files of an application, if necessary
   * @param todo list of sources
   * @return true, if all assembled successfully
   */
  private boolean assembleAll(Vector todo)
   {
    for(int i = 0; i < todo.size(); i++)
     {
      String name = (String)todo.get(i);
      String trunc = name.substring(0, name.lastIndexOf('.') + 1);

      File ass = new File(trunc + "fs");
      File obj = new File(trunc + "obj");

      if (ass.exists() && ass.length() > 0)
       if (!obj.exists() || obj.lastModified() < ass.lastModified())
        if (!assemble("linkable", ass.getAbsolutePath(), false, false))
         return false;

      ass = new File(trunc + "start.fs");
      obj = new File(trunc + "start.obj");
      if (ass.exists() && ass.length() > 0)
       if (!obj.exists() || obj.lastModified() < ass.lastModified())
        if (!assemble("linkable", ass.getAbsolutePath(), false, false))
         return false;
     }

    return true;
   }

  /** enter input line
   * @return the line
   */
  public String inputDialog()
   {
    MyInputVerifier verify = new MyInputVerifier("[^\n]*");
    String text = null;

    while(text == null)
     {
      MyDialog dialog = new MyDialog(this, 800, 40, "enter a line", true, "", "enter any character", verify);

      dialog.setVisible(true);
      text = dialog.getText();
     }

    return text;
   }

  /** calculate best frequency
   * @param desired double
   */
  private void bestFrequency(double desired)
   {
	desired *= 1000000.0;
	double base = oscilator * 1000000.0;
	int clkdiv = 1;
	int multiplier = 1;
	int divider = 1;
	double res = Double.MAX_VALUE;
	
	for(int k = 1; k <= (dcm.charAt(0) == 'D'? 1: 52); k++)
	 for(int i = 2; i <= (dcm.charAt(0) == 'D'? 32: 64); i++)
	  for(int j = 1; j <= (dcm.charAt(0) == 'D'? 32: 128); j++)
	  {
	   double x = base / k * i / j - desired;
	   
	   if (dcm.charAt(0) == 'D' || (base / k * i <= 1.0e9 && base / k * i >= 400.0e6))
	   if (Math.abs(x) < res)
	    {
		 clkdiv = k;
		 multiplier = i;
		 divider = j;
		 
		 res = Math.abs(x);
	    }
	  }
	
	consoleOut.addText("" + base + (dcm.charAt(0) == 'P'? " / " + clkdiv: "") + " * " + multiplier + " / " +
			           divider + " = " + (base / clkdiv * multiplier / divider) + "\n");
   }
  
  public void actionPerformed(ActionEvent e)
   {
    boolean skip = false;

    if (e.getSource() == MiscMenueItem1)
     {
      consoleOut.clear();
      consoleOut.setCursor(5, 40);
      consoleOut.addText("Client for FORTH-System\n");
      consoleOut.setCursor(7, 40);
      consoleOut.addText("Copyright © 2004-2008, by Gerhard Hohner\n");
      consoleOut.setCursor(8, 40);
      consoleOut.addText("Email: gerhard.hohner@drei.at\n");
      consoleOut.addText("\n");
     }
    else if (e.getSource() == MiscMenueItem2)
     {                                                        // select DCM or PLL
      String [] item = { "DCM", "PLL" };
      MyChoice dialog = new MyChoice(this, "DCM or PLL", true, item);

      dialog.show(dcm);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       {
        dcm = text;
       }
     }
    else if (e.getSource() == MiscMenueItem3)
     {                                                        // oscilator
      MyInputVerifier verify = new MyInputVerifier("[0123456789]+(\\.[0123456789]*)?");
      MyDialog dialog = new MyDialog(this, "enter oscilator frequency[MHz]", true, "", "enter number only", verify);

      dialog.setVisible(true);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       {
        try { double a = Double.parseDouble(text); oscilator = a; }
        catch (Exception ex2) { consoleOut.addText("Exception: " + ex2.getMessage()); }
       }
     }
    else if (e.getSource() == MiscMenueItem4)
     {                                                        // frequency
        MyInputVerifier verify = new MyInputVerifier("[0123456789]+(\\.[0123456789]*)?");
        MyDialog dialog = new MyDialog(this, "enter desired frequency[MHz]", true, "", "enter number only", verify);

        dialog.setVisible(true);
        String text = dialog.getText();
        if (text != null && text.length() > 0)
         {
          try { double a = Double.parseDouble(text); bestFrequency(a); }
          catch (Exception ex2) { consoleOut.addText("Exception: " + ex2.getMessage()); }
         }
     }
    else if (e.getSource() == FileMenueItem111 && !bounce)
     {                                                        // create a new project
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(myProject);
      panel.setDialogTitle("select destination");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);          // disable multiselection
      panel.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {
        File x = new File(panel.getCurrentDirectory(), panel.getSelectedFile().getName());
        if (!x.exists())
         {
          try { x.createNewFile(); }
          catch (IOException ex1) { consoleOut.addText("Exception: " + ex1.getMessage()); }
          x.mkdir();
          myProject = x;
         }
        else if (x.isDirectory())
         myProject = x;
       }
      skip = true;
     }
    else if (e.getSource() == FileMenueItem112 && !bounce)
     {                                                        // create a new file
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(myProject);
      panel.setDialogTitle("select file");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);          // disable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {
        File x = new File(panel.getCurrentDirectory(), panel.getSelectedFile().getName());
        if (!x.exists())
         {
          try { x.createNewFile(); }
          catch (IOException ex1) { consoleOut.addText("Exception: " + ex1.getMessage()); }
         }
        else if (!x.isFile())
         x = null;
        try { Runtime.getRuntime().exec(myEditor + ' ' + x.getPath()); }
        catch (IOException ex2) { consoleOut.addText("Exception: " + ex2.getMessage()); }
       }
      skip = true;
     }
    else if (e.getSource() == FileMenueItem12)
     {                                                        // open a project
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(myProject);
      panel.setDialogTitle("open an existing project");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);         // disable multiselection
      panel.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // change working directory
         myProject = panel.getSelectedFile();
       }
     }
    else if (e.getSource() == FileMenueItem13)
     {                                                        // open some files
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(myProject);
      panel.setDialogTitle("open some existing files");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(true);          // enable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // open files
        File [] files = panel.getSelectedFiles();
        try
         {
          for(int i = 0; i < files.length; i++)
           Runtime.getRuntime().exec(myEditor + ' ' + files[i].getPath());
         }
        catch (IOException ex) { consoleOut.addText("Exception: " + ex.getMessage()); }
       }
     }
    else if (e.getSource() == FileMenueItem14)
     {                                                        // close current project
      myProject = myProject.getParentFile();
     }
    else if (e.getSource() == FileMenueItem15)
     {                                                        // set editor
      JFileChooser panel = new JFileChooser();

      String trunc = myEditor.substring(0, myEditor.lastIndexOf('\\'));
      if (trunc.length() == 0)
       trunc = "C:\\";
      panel.setCurrentDirectory(new File(trunc));
      panel.setDialogTitle("choose an editor");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);         // disable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "exe" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("EXE files");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // change editor
         myEditor = panel.getSelectedFile().getAbsolutePath();
       }
     }
    else if (e.getSource() == FileMenueItem16)
     {                                                        // exit frame
      properties.setProperty("myProject", myProject.getPath());
      properties.setProperty("home", home.getPath());
      properties.setProperty("myEditor", myEditor);
      properties.setProperty("option", option);
      properties.setProperty("template", template);
      properties.setProperty("blocksize", blocksize);
      properties.setProperty("translate", translate);
      properties.setProperty("gc", gc);
      properties.setProperty("sun", sun.getPath());
      properties.setProperty("com", com);
      properties.setProperty("parity", parity);
      properties.setProperty("stopbits", stopbits);
      properties.setProperty("baudrate", baudrate);
      properties.setProperty("control", control);
      properties.setProperty("dynmem", "" + dynmem);
      properties.setProperty("dcm", dcm);
      properties.setProperty("oscilator", "" + oscilator);

      File file = new File(propertyFile);
      try { file.createNewFile(); } catch (IOException ex7) { consoleOut.addText("Exception: " + ex7.getMessage()); }

      try { properties.store(new FileOutputStream(file), "properties for communicate"); }
      catch (IOException ex6) { consoleOut.addText("Exception: " + ex6.getMessage()); }

      if (myport != null)
       myport.close();

      this.dispose();
      this.processWindowEvent(new WindowEvent(this, WindowEvent.WINDOW_CLOSING));
     }
    else if (e.getSource() == SerialMenueItem11)
     {                                                        // select COM
      String [] item = Serial.getPortChoices();
      MyChoice dialog = new MyChoice(this, "select port", true, item);

      dialog.show(com);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       {
        com = text;
        SerialMenueItem15.setEnabled(true);
       }
     }
    else if (e.getSource() == SerialMenueItem12)
     {                                                        // select stopbits
      String [] item = { "1", "1.5", "2" };
      MyChoice dialog = new MyChoice(this, "select stopbits", true, item);

      dialog.show(stopbits);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       {
        stopbits = text;
        SerialMenueItem15.setEnabled(true);
       }
     }
    else if (e.getSource() == SerialMenueItem13)
     {                                                        // select parity
      String [] item = { "none", "even", "odd" };
      MyChoice dialog = new MyChoice(this, "select parity", true, item);

      dialog.show(parity);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       {
        parity = text;
        SerialMenueItem15.setEnabled(true);
       }
     }
    else if (e.getSource() == SerialMenueItem14)
     {                                                        // select baudrate
      String [] item = { "300", "1200", "2400", "9600", "14400", "28800", "38400", "57600", "152000" };
      MyChoice dialog = new MyChoice(this, "select baudrate", true, item);

      dialog.show(baudrate);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       {
        baudrate = text;
        SerialMenueItem15.setEnabled(true);
       }
     }
    else if (e.getSource() == CompileMenueItem11)
     {                                                        // option
      String [] item = { "linkable", "romable" };
      MyChoice dialog = new MyChoice(this, "select option", true, item);

      dialog.show(option);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       option = text;
     }
    else if (e.getSource() == CompileMenueItem14)
     {                                                        // get file to compile
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(myProject);
      panel.setDialogTitle("select template");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);          // disable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "t" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("template files");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // get file
        File file = panel.getSelectedFile();
        template = file.getAbsolutePath();
       }
     }
    else if (e.getSource() == CompileMenueItem15)
     {                                                        // select blocksize
      String [] item = { "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" };
      MyChoice dialog = new MyChoice(this, "select blocksize of BRAM", true, item);

      dialog.show(blocksize);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       blocksize = text;
     }
    else if (e.getSource() == CompileMenueItem12)
     {                                                        // get file to compile
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(myProject);
      panel.setDialogTitle("select FORTH-file");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);          // disable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "fs", "txt" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("FORTH files");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // compile files
        File file = panel.getSelectedFile();
        assemble(option, file.getAbsolutePath(), false, true);
       }
     }
    else if (e.getSource() == CompileMenueItem13)
     {                                                        // assamble precompiled files
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(home);
      panel.setDialogTitle("select loader file");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);          // disable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "ld" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("loader lists");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // open files
        File files = panel.getSelectedFile();
        Vector compiled = buildLoaderList(files.getAbsolutePath());
        status.addText(assembleAll(compiled)?"done":"aborted");
       }
     }
    else if (e.getSource() == JavaMenueItem0)
     {                                                        // get option to convert to FORTH
      String [] item = { "only modified", "translate all newly" };
      MyChoice dialog = new MyChoice(this, "select translate option", true, item);

      dialog.show(translate);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       translate = text;
     }
    else if (e.getSource() == JavaMenueItem1)
     {                                                        // get file to convert to FORTH
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(home);
      panel.setDialogTitle("select JAVA-file");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);         // no multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "java" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("JAVA files");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // open files
        status.setText("Compiling...");
        File files = panel.getSelectedFile();
        if (files != null && MyJava.Compiler.compile(files.getAbsolutePath(), sun.getAbsolutePath(), myEditor, true, translate.startsWith("translate"), gc))
         {
          status.setText("done");
          Vector compiled = buildLoaderList(files.getAbsolutePath());
          if (!assembleAll(compiled))
           compiled = null;
         }
        else
         status.setText("aborted");
       }
     }
    else if (e.getSource() == JavaMenueItem2)
     {                                                        // get root directory of suns sources
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(sun);
      panel.setDialogTitle("select directory");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);         // no multiselection
      panel.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // open files
        sun = panel.getSelectedFile();
       }
     }
    else if (e.getSource() == JavaMenueItem3)
     {                                                        // get root directory of java sources
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(home);
      panel.setDialogTitle("select home directory");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);         // no multiselection
      panel.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // open files
        File files = panel.getSelectedFile();
        home = files;
       }
     }
    else if (e.getSource() == JavaMenueItem4)
     {
      String [] item = { "tricolor marking", "modified reference counting" };
      MyChoice dialog = new MyChoice(this, "select gc option", true, item);

      dialog.show(gc);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       gc = text;
     }
    else if (e.getSource() == DictionaryMenueItem1 && myport != null)
     {                                                        // list dictionary
      myport.dictionary(true);
     }
    else if (e.getSource() == DictionaryMenueItem2 && myport != null && myport.isReady())
     {                                                        // load assembled file
      if (!Dictionary.isReady())
       myport.dictionary(false);
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(myProject);
      panel.setDialogTitle("select object file");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(true);          // enable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "obj" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("linkable object files");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // open files
        File [] files = panel.getSelectedFiles();
        Vector v = new Vector();
        for(int i = 0; i < files.length; i++)
         {
          String named = "";
          try
          {
           FileInputStream f = new FileInputStream(files[i].getAbsolutePath());
           f.skip(4L);
           byte [] b = new byte[f.read() + (f.read() << 8)];
           f.read(b);
           if (f.read() >= 0)
            {
             named = new String(b);
            }
          }
          catch (Exception ex4)
          {
          }
          Dictionary dict = Dictionary.search(named);
          if (dict == null || dict.name.compareTo(named) != 0)
           v.add(files[i].getAbsolutePath());
         }
        if (v.size() > 0)
         {
          myport.loadObjects(v, false, files.length > 1, -1);
          Dictionary.dictionary = null;
         }
       }
     }
    else if (e.getSource() == DictionaryMenueItem3 && myport != null && myport.isReady())
     {                                                        // load compiled files
      if (!Dictionary.isReady())
       myport.dictionary(false);
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(home);
      panel.setDialogTitle("select loader file");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);          // disable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "ld" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("loader lists");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // open files
        File files = panel.getSelectedFile();
        Vector dummy = new Vector();
        Vector compiled = buildLoaderList(files.getAbsolutePath());
        for (int i = compiled.size(); i-- > 0; )
         {
          String name = (String) compiled.get(i);
          String trunc = name.substring(0, name.lastIndexOf('.'));
          String named = "";
          try
          {
           FileInputStream f = new FileInputStream(trunc + ".obj");
           f.skip(4L);
           byte [] b = new byte[f.read() + (f.read() << 8)];
           f.read(b);
           if (f.read() >= 0)
            {
             named = new String(b);
            }
          }
          catch (Exception ex4)
          {
          }
          Dictionary dict = Dictionary.search(named);
          if (dict == null || dict.name.compareTo(named) != 0)
           dummy.add(compiled.get(i));
         }
        compiled = new Vector();
        for (int i = 0; i < dummy.size(); i++)
         {
          String name = (String) dummy.get(i);
          String trunc = name.substring(0, name.lastIndexOf('.') + 1) + "obj";
          compiled.add(trunc);
         }
        if (compiled.size() > 0)
         {
          myport.loadObjects(compiled, false, compiled.size() > 1, dynmem);
          Dictionary.dictionary = null;
         }
       }
     }
    else if (e.getSource() == DictionaryMenueItem4 && myport != null && myport.isReady())
     {                                                        // initialize loaded modules
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(home);
      panel.setDialogTitle("select loader file");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);          // disable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "ld" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("loader lists");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {                                             // open files
        File files = panel.getSelectedFile();
        Vector compiled = new Vector();
        Vector dummy = buildLoaderList(files.getAbsolutePath());
        while (dummy.size() > 0)
         {
          String name = (String) dummy.lastElement();
          String trunc = name.substring(0, name.lastIndexOf('.') + 1) + "start.obj";
          compiled.add(trunc);
          dummy.remove(name);
         }
        myport.loadObjects(compiled, false, false, -1);
       }
     }
    else if (e.getSource() == DictionaryMenueItem5 && myport != null)
     {                                                        // unload module
      if (!Dictionary.isReady())
       myport.dictionary(false);
      Vector dummy = new Vector();
      for(int i = 0; i < Dictionary.dictionary.size(); i++)
       {
        Dictionary dict = (Dictionary)Dictionary.dictionary.get(i);
        if (dict.type == Dictionary.MODULE)
         dummy.add(dict.name);
       }

      MyChoice dialog = new MyChoice(this, "select module", true, (String [])dummy.toArray(new String[dummy.size()]));

      dialog.show("");
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       {
        myport.unloadObject(text);
        Dictionary d = Dictionary.search(text);
        Dictionary.dictionary.setSize(Dictionary.dictionary.indexOf(d));
       }
     }
    else if (e.getSource() == SerialMenueItem15)
     {                                                        // change speed
      if (myport != null)
       {
        myport.redefineSIO(baudrate, parity, stopbits);
        myport.close();
       }

      try
       {
        myport = new Serial(com, baudrate, parity, stopbits, control, this);
        SerialMenueItem15.setEnabled(false);
        InMenue13.setEnabled(true);
        CommandMenue1.setEnabled(true);
        DictionaryMenue1.setEnabled(true);
        GCMenue1.setEnabled(true);
       }
      catch (Exception ex3)
       {
        consoleOut.addText("Exception: " + ex3.getMessage());
        InMenue13.setEnabled(false);
        CommandMenue1.setEnabled(false);
        DictionaryMenue1.setEnabled(false);
       }
     }
    else if (e.getSource() == SerialMenueItem17)
     {                                                        // control
      String [] item = { "RTS/CTS", "NONE" };
      MyChoice dialog = new MyChoice(this, "select control", true, item);

      dialog.show(control);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       {
        control = text;
        SerialMenueItem15.setEnabled(true);
       }
     }
    else if (e.getSource() == SerialMenueItem16)
     {
      baudrate = "9600";
      stopbits = "1";
      parity = "none";
      control = "NONE";
     }
    else if (e.getSource() == CommandMenue11 && myport != null)
     {                                                        // reset processor
      myport.restart();
     }
    else if (e.getSource() == CommandMenue12 && myport != null)
     {                                                        // abort
      myport.abort();
     }
    else if (e.getSource() == OutMenue11)
     {                                                        // clear output
      consoleOut.clear();
     }
    else if (e.getSource() == OutMenue12)
     {                                                        // save output
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(myProject);
      panel.setDialogTitle("select destination file");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);          // disable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "fs", "txt", "java" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("text files");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {
        File x = new File(myProject, panel.getSelectedFile().getName());
        if (!x.exists())
         {
          try { x.createNewFile(); } catch (IOException ex1) { consoleOut.addText("Exception: " + ex1.getMessage()); }
         }
        consoleOut.save(x);
       }
     }
    else if (e.getSource() == InMenue11)
     {                                                        // clear input
      consoleIn.clear();
     }
    else if (e.getSource() == InMenue12)
     {                                                        // save input
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(myProject);
      panel.setDialogTitle("select destination file");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);          // disable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "fs", "txt", "java" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("text files");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       {
        File x = new File(myProject, panel.getSelectedFile().getName());
        if (!x.exists())
         {
          try { x.createNewFile(); } catch (IOException ex1) { consoleOut.addText("Exception: " + ex1.getMessage()); }
         }
        consoleIn.save(x);
       }
     }
    else if (e.getSource() == InMenue13 && consoleIn.jEditorPane1.getText().length() > 0 && myport != null && myport.isReady())
     {                                                        // execute
      File file = new File(myProject, "scratch.§§§");
      consoleIn.save(file);
      if (assemble("linkable", file.getAbsolutePath(), true, false))
       {
        String name = file.getAbsolutePath();
        name = name.substring(0, name.lastIndexOf('.') + 1);
        Vector v = new Vector();
        v.add(name + "obj");
        myport.loadObjects(v, true, false, -1);
       }
      file.delete();
     }
    else if (e.getSource() == InMenue14)
     {                                                        // load source
      JFileChooser panel = new JFileChooser();

      panel.setCurrentDirectory(myProject);
      panel.setDialogTitle("select source file");
      panel.setDialogType(JFileChooser.OPEN_DIALOG); // open selected files
      panel.setMultiSelectionEnabled(false);          // disable multiselection
      panel.setFileSelectionMode(JFileChooser.FILES_ONLY);
      panel.resetChoosableFileFilters();             // no standard filters
      String [] endings = { "fs", "txt", "java" };
      MyFileFilter filter = new MyFileFilter(endings);
      filter.setDescription("text files");
      panel.setFileFilter(filter);

      if (panel.showDialog(null, "Attach") == JFileChooser.APPROVE_OPTION)
       consoleIn.load(panel.getSelectedFile());
     }
    else if (e.getSource() == GCMenueItem11 && myport != null)
     {                                                        // unused memory
      myport.unused(0);
     }
    else if (e.getSource() == GCMenueItem12 && myport != null)
     {                                                        // allocate memory
      MyInputVerifier verify = new MyInputVerifier("[0123456789]+");
      MyDialog dialog = new MyDialog(this, "enter size of dynamic memory to create", true, "", "enter number only", verify);

      dialog.setVisible(true);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       {
        try { long a = Long.parseLong(text); myport.allocate((int)a); }
        catch (Exception ex2) { consoleOut.addText("Exception: " + ex2.getMessage()); }
       }
     }
    else if (e.getSource() == GCMenueItem13 && myport != null)
     {                                                        // free memory
      myport.free();
     }
    else if (e.getSource() == GCMenueItem14 && myport != null)
     {                                                        // reset memory
      myport.resetPool();
     }
    else if (e.getSource() == GCMenueItem15 && myport != null)
     {                                                        // memory size
      MyInputVerifier verify = new MyInputVerifier("[0123456789]+");
      MyDialog dialog = new MyDialog(this, "enter default size of dynamic memory", true, "" + dynmem, "enter number only", verify);

      dialog.setVisible(true);
      String text = dialog.getText();
      if (text != null && text.length() > 0)
       {
        try { long a = Long.parseLong(text); dynmem = (int)a; }
        catch (Exception ex2) { consoleOut.addText("Exception: " + ex2.getMessage()); }
       }
     }
    else if (e.getSource() == testMenue)
     {
      InputBuffer input = new InputBuffer();
      input.appendRaw((byte)15);
      input.append((byte)Serial.CREATEF);
      input.append((byte)16);
      input.append("juchei.txt");
      input.appendRaw((byte)26);
      input.appendRaw((byte)1);
      input.append("hello, this is a test, good bye\n");
      input.appendRaw((byte)26);
      input.appendRaw((byte)15);
      input.append((byte)Serial.CLOSE);
      input.append((byte)16);
      input.appendRaw((byte)26);
      input.appendRaw((byte)15);
      input.append((byte)Serial.INCLUDEF);
      input.append("juchei.txt");
      input.appendRaw((byte)26);
      input.appendRaw((byte)15);
      input.append((byte)Serial.DELETE);
      input.append("juchei.txt");
      input.appendRaw((byte)26);
      input.appendRaw((byte)0);
      input.append("juchei");
      input.appendRaw((byte)26);
      myport.showStatus();
      myport.sendData(input);
     }

    bounce = skip;
   }


  public static void main(String[] args)
   {
    try
     {
      //UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
      UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
     }
    catch (Exception e)
     {
      e.printStackTrace();
     }

    MyFrame app = new MyFrame();

    app.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    app.pack();
    app.setVisible(true);
   }

 static
  {
   if (MyFrame.class.getResource("/win32com.dll") != null)
    {
     File dll = null, comm = null, properties = null;
     InputStream inputStream;
     FileOutputStream outputStream;
     byte[] array = new byte[8192];
     int read = 0;
     try
     {
      dll = new File(System.getProperty("java.home") + "\\bin", "win32com.dll");
      if (!dll.exists())
       {
        inputStream = MyFrame.class.getResource("/win32com.dll").openStream();
        // Copy resource to filesystem in a temp folder with a unique name
        outputStream = new FileOutputStream(dll);
        while ( (read = inputStream.read(array)) > 0)
        {
         outputStream.write(array, 0, read);
        }
        outputStream.close();
       }
      // write dummy
      comm = new File(System.getProperty("java.home") + "\\lib", "comm.jar");
      if (!comm.exists())
       {
        inputStream = MyFrame.class.getResource("/comm.jar").openStream();
        outputStream = new FileOutputStream(comm);
        while ( (read = inputStream.read(array)) > 0)
        {
         outputStream.write(array, 0, read);
        }
        outputStream.close();
       }
      // write properties
      properties = new File(System.getProperty("java.home") + "\\lib", "javax.comm.properties");
      if (!properties.exists())
       {
        inputStream = MyFrame.class.getResource("/javax.comm.properties").openStream();
        outputStream = new FileOutputStream(properties);
        while ( (read = inputStream.read(array)) > 0)
        {
         outputStream.write(array, 0, read);
        }
        outputStream.close();
       }
     }
     catch (FileNotFoundException ex)
     {
     }
     catch (IOException ex)
     {
     }
     System.setProperty("java.class.path", System.getProperty("java.home") + "\\bin;" +
                        System.getProperty("java.home") + "\\lib\\comm.jar;" + System.getProperty("java.library.path"));
    }
  }
}
