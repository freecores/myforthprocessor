package communicate;

import javax.comm.*;
import java.util.*;
import java.io.*;

import javax.swing.event.*;
import java.awt.event.*;

/**
 * <p>Überschrift: Protokoll zur Kommunikation mit dem Server</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright © 2004-2008 by Gerhard Hohner</p>
 * <p>Organisation: </p>
 * @author Gerhard Hohner
 * @version 1.0
 */

public class Serial implements SerialPortEventListener, CaretListener, KeyListener
 {
  private CommPortIdentifier portId;
  private SerialPort sPort;
  private InputStream is;
  private OutputStream os;
  private final Output consoleOut;
  private Output status;
  private int oldDot;


  static  final int OPEN = 0,
                    CREATEF = 1,
                    READ = 2,
                    LINE = 3,
                    LISTDICT = 4,
                    ATXY = 5,
                    GETFILEPOS = 6,
                    SETFILEPOS = 7,
                    GETFILESIZE = 8,
                    SETFILESIZE = 9,
                    INCLUDEF = 10,
                    CLEAR = 11,
                    STATUS = 12,
                    RENAME = 13,
                    DELETE = 14,
                    CLOSE = 15,
                    ABSOLUTE = 16,
                    RELATIVE = 17,
                    SETDATE = 18,
                    GETDATE = 19,
                    MODULPROCESSED = 20,
                    MEMORYUNUSED = 21,
                    READBUFFERED = 22,
                    ASCII = 23,
                    ERRORREC = 24,
                    INCCONSOLE = 28,
                    DECCONSOLE = 29;

  private Vector inputBuffers;
  private Vector stdInputStack;
  private Vector fileStack;
  private int handleIn;
  private InputBuffer inputbuffer;
  private Vector toSend;
  private boolean escape;
  private Tuppel sendTop;
  private int nextToSend;
  private volatile int sfifo = 8;
  private Automaton server;
  private InputBuffer preparedBuffer = null;
  private boolean receiveload = false;
  private boolean xon;
  private MyFrame frame = null;
  private InputBuffer errorRecords;
  private Vector loadList;
  private boolean deleteObj;
  private boolean errrecs;
  private volatile boolean forceSend = true;
  private int [] used = new int[3];
  private int noConsole = 0;
  private Vector rFiFo = new Vector();
  private volatile boolean rfifo = false;

  private MyFile [] files;

  private boolean open = false;

  /**
   * listen
   * @param a CaretEvent
   */
  public synchronized void caretUpdate(CaretEvent a)
   {
    String s = frame.consoleIn.getText();
    char t = (s.length() > 0?s.charAt(s.length() - 1):'\r');

    if (a.getDot() > oldDot && t != '\r')
     {
      InputBuffer b = new InputBuffer();
      b.appendRaw((byte)0);
      b.append((byte)t);
      sendData(b);
     }

    oldDot = a.getDot();
   }

  /**
   * listen
   * @param a KeyEvent
   */
  public void keyTyped(KeyEvent a)
   {
	char t = a.getKeyChar();
	
	if (t > 7 && t < 256)
     {
      InputBuffer b = new InputBuffer();
      b.appendRaw((byte)0);
      b.append((byte)(t == 13? 10: t));
      sendData(b);
     }
   }

  public void keyPressed(KeyEvent a) {}
  
  public void keyReleased(KeyEvent a) {}
  
   /**
    * restore console input
    * @param decr int decr decrement counter
    */
  private void restoreInput(int decr)
   {
	if ((noConsole -= decr) == 0)
	 {
      //frame.consoleIn.removeCaretListener(this);
      frame.consoleIn.removeKeyListener(this);
      frame.consoleIn.clear();
	  oldDot = -1;
      frame.consoleIn.load(new File(frame.myProject ,"Input.sav"));
	 }
   }

  /**
   * save console input
   */
  private void saveInput()
   {
	if (noConsole++ == 0)
	 {
	  frame.consoleIn.save(new File(frame.myProject, "Input.sav"));
	  frame.consoleIn.clear();
	  oldDot = -1;
	  //frame.consoleIn.addCaretListener(this);
	  frame.consoleIn.addKeyListener(this);
	 }
   }

  /**
   * reopen serial port
   * @param baudrate String
   * @param parity String
   * @param stopbits String
   * @param control String
   * @throws Exception
   */
  public void reassign(String baudrate, String parity, String stopbits, String control) throws Exception
   {
    // Set the parameters of the connection. If they won't set, close the
    // port before throwing an exception.
    int baud, stop, par;
    try { baud = Integer.parseInt(baudrate); }
    catch (NumberFormatException ex) { throw ex; }
    try { stop = stopbits.startsWith("1") ? (stopbits.endsWith("5") ? SerialPort.STOPBITS_1_5 :SerialPort.STOPBITS_1) : SerialPort.STOPBITS_2; }
    catch (Exception ex1) { throw ex1; }
    try { par = parity.startsWith("e") ? SerialPort.PARITY_EVEN :(parity.startsWith("o") ? SerialPort.PARITY_ODD : SerialPort.PARITY_NONE); }
    catch (Exception ex2) { throw ex2; }

    try { sPort.setSerialPortParams(baud, SerialPort.DATABITS_8, stop, par); }
    catch (UnsupportedCommOperationException ex3) { sPort.close(); throw new Exception("illegal port parameter"); }
    try
     {
      if (control.charAt(0) == 'X')
       sPort.setFlowControlMode(SerialPort.FLOWCONTROL_XONXOFF_IN |
                                SerialPort.FLOWCONTROL_XONXOFF_OUT);
      else if (control.charAt(0) == 'R')
       sPort.setFlowControlMode(SerialPort.FLOWCONTROL_RTSCTS_IN |
                                SerialPort.FLOWCONTROL_RTSCTS_OUT);
      else
       sPort.setFlowControlMode(SerialPort.FLOWCONTROL_NONE);
      
      sPort.setDTR(true);
      if (control.charAt(0) != 'R')
       sPort.setRTS(true);
     }
    catch (UnsupportedCommOperationException ex4) { sPort.close(); throw new Exception("unknown handshake"); }
    server.reset();
    restoreInput(noConsole);
   }

   /**
    * open serial connection
    * @param com String
    * @param baudrate String
    * @param parity String
    * @param stopbits String
    * @param control String
    * @param consoleOut Output
    * @param status Output
    * @throws Exception
    */
   public Serial(String com, String baudrate, String parity, String stopbits, String control, MyFrame frame) throws Exception
   {
    server = new Automaton();

    files = new MyFile[16];

    inputbuffer = new InputBuffer();
    escape = false;
    handleIn = 0;
    inputbuffer.appendRaw((byte)0);
    sendTop = null;

    nextToSend = -1;
    xon = true;

    inputBuffers = new Vector();
    stdInputStack = new Vector();
    fileStack = new Vector();
    toSend = new Vector();
    this.frame = frame;
    this.status = frame.status;
    this.consoleOut = frame.consoleOut;
    loadList = null;
    errorRecords = null;

    try { assign(com, baudrate, parity, stopbits, control); } catch(Exception e) { throw e;}
   }

   /**
    * open serial port
    * @param com String
    * @param baudrate String
    * @param parity String
    * @param stopbits String
    * @param control String
    * @throws Exception
    */
   public void assign(String com, String baudrate, String parity, String stopbits, String control) throws Exception
   {
    // Obtain a CommPortIdentifier object for the port you want to open.
    try { portId = CommPortIdentifier.getPortIdentifier(com); }
    catch (NoSuchPortException e) { throw new Exception("unknown port"); }

    // Open the port represented by the CommPortIdentifier object. Give
    // the open call a relatively long timeout of 30 seconds to allow
    // a different application to reliquish the port if the user
    // wants to.
    try { sPort = (SerialPort)portId.open(com, 30000); }
    catch (PortInUseException e) { throw e; }

    try { reassign(baudrate, parity, stopbits, control); }
    catch (Exception e) { throw e; }

    // Open the input and output streams for the connection. If they won't
    // open, close the port before throwing an exception.
    try {
        os = sPort.getOutputStream();
        is = sPort.getInputStream();
    } catch (IOException e) {
        sPort.close();
        throw new Exception("Error opening i/o streams");
    }

    // Add this object as an event listener for the serial port.
    try { sPort.addEventListener(this); }
    catch (TooManyListenersException e)
     {
      sPort.close();
      throw new Exception("too many listeners added");
     }

    sPort.notifyOnDataAvailable(true);
    sPort.notifyOnOutputEmpty(true);
    sPort.notifyOnBreakInterrupt(false);
    sPort.notifyOnFramingError(true);
    sPort.notifyOnOverrunError(true);
    sPort.notifyOnParityError(true);

    // Set receive timeout to allow breaking out of polling loop during
    // input handling.
    try { sPort.enableReceiveTimeout(30); }
    catch (UnsupportedCommOperationException e) { }

    while(is.available() > 0) try { is.read(); } catch(Exception e) { }

    open = true;
   }

   /**
   * Close the port and clean up associated elements.
   */
  public void close()
   {
    // If port is alread closed just return.
    if (!open) { return; }

    // Check to make sure sPort has reference to avoid a NPE.
    if (sPort != null)
     {
      try { os.close(); is.close(); } catch (IOException ex) { }
      // Close the port.
      sPort.close();
     }
    open = false;
   }

   /**
    * get names of installed serial ports
    * @return String[]
    */
   public static String [] getPortChoices()
   {
    CommPortIdentifier portId;
    Vector x = new Vector();

    Enumeration en = CommPortIdentifier.getPortIdentifiers();

    // iterate through the ports.
    while (en.hasMoreElements())
     {
      portId = (CommPortIdentifier)en.nextElement();
      if (portId.getPortType() == CommPortIdentifier.PORT_SERIAL)
       x.add(portId.getName());
     }

    String [] y = new String[x.size()];

    for(int i = 0; i < y.length; i++)
     y[i] = (String)x.get(i);

    return y;
   }

   /**
    * write binary files to serial port
    * @param name String
    * @param delete boolean
    * @param errrec boolean
    * @return boolean
    */
   public void loadObjects(Vector list, boolean delete, boolean errrec, int dynmem)
    {
     if (list != null && list.size() > 0)
      {
       unused(dynmem); // force dynamic memory of size of property dynmem
       loadList = list;
       deleteObj = delete;
       errrecs = errrec;
       if (!loadNext())
        loadList = null;
      }
    }

   private boolean loadNext()
    {
     boolean done = false;
     while(!done && loadList.size() > 0)
      {
       String s = (String)loadList.get(0);
       loadList.remove(0);
       done = loadObject(s, deleteObj, errrecs);
      }
     return done;
    }

   public boolean isReady()
    {
     return loadList == null;
    }

   /**
    * write binary file to serial port
    * @param name String
    * @param delete boolean
    * @param errrec boolean
    * @return boolean
    */
   private boolean loadObject(String name, boolean delete, boolean errrec)
   {
    boolean done = false, load = false;
    File file = makeFile(name);

    //while(!server.isReady());

    if (load = file.exists() && file.isFile() && file.length() > 0)
     {
      if (errrec && errorRecords == null)
       {
        errorRecords = new InputBuffer();
        errorRecords.appendRaw((byte)15);
       }

      InputBuffer x = new InputBuffer();
      x.appendRaw((byte)(errrec?243:254)); // load file
      try
       {
        while(!server.assignDeck());
        if (file.getName().compareToIgnoreCase("scratch.obj") != 0)
         consoleOut.addText("load " + file.getName() + "\n");
        files[15] = new MyFile(file, 2, false, delete); // read file only, no append
        done = true;
        sendData(x);
        status.setText("upload ");
       }
      catch (Exception ex) { consoleOut.addText(ex.getMessage() + '\n'); server.reset(); }
     }

    if (!done)
     status.setText("failed");
    else
     saveInput();

    return load;
   }
   
   /**
    * unload module
    * @param module String
    */
   public void unloadObject(String module)
   {
    while(!server.prepared(false));

    preparedBuffer = new InputBuffer();
    preparedBuffer.appendRaw((byte)15);
    preparedBuffer.append((byte)module.length());
    preparedBuffer.append((byte)(module.length() >> 8));
    preparedBuffer.append(module);
    preparedBuffer.appendRaw((byte)26);

    InputBuffer x = new InputBuffer();

    x.appendRaw((byte)245); // unload

    sendData(x);
    status.setText("unload");
   }

   /**
    * restart connected slave
    */
   public void restart()
   {
    InputBuffer x = new InputBuffer();

    loadList = null;
    errorRecords = null;
    this.inputbuffer = new InputBuffer();
    escape = false;
    handleIn = 0;
    this.inputbuffer.appendRaw((byte)0);
    
    sendTop = null;
    toSend.clear();
    inputBuffers.clear();
    stdInputStack.clear();
    fileStack.clear();
    nextToSend = -1;
    xon = true;

    server.reset();
    restoreInput(noConsole);

    for(int i = 0; i < files.length; files[i++] = null)
     if (files[i] != null)
      files[i].close();
    
    x.appendRaw((byte)255); // reset

    consoleOut.clearHold(true);
    try { os.flush(); }
    catch (IOException e) { consoleOut.addText(e.getMessage());	}
    sendData(x);
    status.setText("processor reset");
   }

   /**
    * abort operating upload
    */
   public void abort()
   {
    InputBuffer x = new InputBuffer();

    x.appendRaw((byte)251); // abort

    sendData(x);
    status.setText("stacks cleared");
    consoleOut.clearHold(true);

    server.abort();
    restoreInput(noConsole);

    for(int i = 0; i < files.length; files[i++] = null)
     if (files[i] != null)
      files[i].close();
   }

   /**
    * receive dictionary from slave
    * @param show display dictionary
    */
   public void dictionary(boolean show)
   {
    Dictionary.show = show;
    Dictionary.dictionary = null;

    InputBuffer x = new InputBuffer();
    x.appendRaw((byte)253); // read dictionary

    while(!server.readDictionary(30000));

    sendData(x);
    status.setText("dictionary will be read");

    while(!server.isReady());
   }

  /**
   * redefine serial port of server
   * @param baudrate String
   * @param parity String
   * @param stopbits String
   */
  public void redefineSIO(String baudrate, String parity, String stopbits)
  {
   int baud = 0, stop, par;
   try { baud = Integer.parseInt(baudrate); }
   catch (NumberFormatException ex) { }
   stop = stopbits.startsWith("1") ? (stopbits.endsWith("5") ? 3 :2) : 4;
   par = parity.startsWith("e") ? 2 :(parity.startsWith("o") ? 1 : 0);

   while(!server.prepared(false));

   InputBuffer x = new InputBuffer();
   x.appendRaw((byte)250); // redefine

   preparedBuffer = new InputBuffer();
   preparedBuffer.appendRaw((byte)15);
   preparedBuffer.append((byte)stop);
   preparedBuffer.append((byte)par);
   preparedBuffer.append((byte)8);
   preparedBuffer.append(baud);
   preparedBuffer.appendRaw((byte)26);

   sendData(x);
   status.setText("UART redefined");
  }

  /**
   * get size of unused memory and size of dynamic memory
   */
  public void unused(int dynmem)
   {
    InputBuffer x = new InputBuffer();

    x.appendRaw((byte)249); // unused

    while(!server.readUnused(15000));

    sendData(x);

    while(!server.isReady());

    if (dynmem > 0 && used[1] == 0)
     allocate(dynmem);
    else if (dynmem == 0)
     consoleOut.addText(used[0] + " byte unused, " + used[1] + " byte dynamic memory, " + used[2] + " byte free program storage\r\n");
   }

   /**
    * allocate dynamic memory
    * @param size int
    */
   public void allocate(int size)
   {
    while(!server.prepared(false));

    preparedBuffer = new InputBuffer();
    preparedBuffer.appendRaw((byte)15);
    preparedBuffer.append(size);
    preparedBuffer.appendRaw((byte)26);

    InputBuffer x = new InputBuffer();

    x.appendRaw((byte)248); // allocate dynamic memory

    sendData(x);
    status.setText("allocate dynamic memory");
   }

   /**
    * free dynamic memory
    */
   public void free()
    {
     InputBuffer x = new InputBuffer();

     x.appendRaw((byte)247); // free dynamic memory

     sendData(x);
     status.setText("no dynamic memory");
    }

    /**
     * reset dynamic memory
     */
    public void resetPool()
    {
     InputBuffer x = new InputBuffer();

     x.appendRaw((byte)246); // reset dynamic memory

     sendData(x);
     status.setText("dynamic memory cleared");
    }

   /**
    * process serial event
    * @param e SerialPortEvent
    */
   public void serialEvent(SerialPortEvent e)
   {
    int x, count;

    try
    {
    // Determine type of event.
    switch (e.getEventType())
     {
      case SerialPortEvent.FE:
                               try { consoleOut.addText("frame error\n"); is.read(); } catch (IOException ex1) { }
                               break;
      case SerialPortEvent.OE:
                               try { consoleOut.addText("overrun error\n"); is.read(); } catch (IOException ex1) { }
                               break;
      case SerialPortEvent.PE:
                              try { consoleOut.addText("parity error\n"); is.read(); } catch (IOException ex1) { }
                              break;
      case SerialPortEvent.DATA_AVAILABLE:
    	                                   count = 0;
                                           try
                                            {
                                             while(is.available() > 0)
                                              {
                                               try
                                                {
                                                 x = is.read();
                                                 //consoleOut.addText(x + " ");
                                                 rFiFo.add(new Integer(x & 0xff));
                                                 count++;
                                                }
                                               catch(Exception ex) { consoleOut.addText("read error\n"); }
                                              }
                                            }
                                           catch(Exception ex) { consoleOut.addText("receiver error\n"); }
                                           
                                           if (!rfifo && rFiFo.size() == count)
                                        	processInput();
                                           
                                           break;
      case SerialPortEvent.OUTPUT_BUFFER_EMPTY:
    	                                        sfifo = 8;
    	                                        if (forceSend)
                                                 sendData();
    	                                        break;
      default:
     }
    }
    catch(Exception ex) { consoleOut.addText("error:" + ex.getMessage() + "\n"); }
   }

   /**
    * diplay status of serial port
    */
   public void showStatus()
   {
    consoleOut.addText("port: " + sPort.getName() + '\n');
    consoleOut.addText("baudrate: " + sPort.getBaudRate() + "\n");
    consoleOut.addText("stopbits: " + sPort.getStopBits() + "\n");
    consoleOut.addText("databits: " + sPort.getDataBits() + "\n");
    String x = (sPort.getFlowControlMode() & SerialPort.FLOWCONTROL_RTSCTS_IN) != 0?"RTS/CTS IN":"";
    x += (sPort.getFlowControlMode() & SerialPort.FLOWCONTROL_RTSCTS_OUT) != 0?", RTS/CTS OUT":"";
    consoleOut.addText("mode: " + x + '\n');
    if (sPort.getParity() == SerialPort.PARITY_EVEN)
     x = "even";
    else if (sPort.getParity() == SerialPort.PARITY_ODD)
     x = "odd";
    else if (sPort.getParity() == SerialPort.PARITY_NONE)
     x = "none";
    else
     x = "mark";
    consoleOut.addText("parity: " + x + '\n');
    x = sPort.isCD()?"CD ":"";
    x += sPort.isCTS()?"CTS ":"";
    x += sPort.isDSR()?"DSR ":"";
    x += sPort.isDTR()?"DTR ":"";
    x += sPort.isRTS()?"RTS ":"";
    x += sPort.isRI()?"RI":"";
    // x += ((Object)x).length() + " " + x.hashCode();
    consoleOut.addText("status: " + x + '\n');
   }

   /**
    * process serial input character queue
    * @param input character to process
    */
   private void processInput()
   {
	while(rFiFo.size() > 0)
	 {
	  rfifo = true;
	  
	  int x = ((Integer)rFiFo.get(0)).intValue();
	  
	  processInput(x);
	  rFiFo.remove(0);
	  
	  rfifo = false;
	 }
   }

   /**
    * process serial input character
    * @param input character to process
    */
   private synchronized void processInput(int input)
   {
     if (input >= 0)
     { // valid input
      boolean help = false;

      if (!escape)
       { // not stuffed
        if (input < 23)
         { // new handle
          if (handleIn != input)
           {
            // save currently open handle
            inputBuffers.add(inputbuffer);
            inputbuffer = null;
            // reopen of a preempted handle?
            for (int i = inputBuffers.size() - 1; i >= 0; i--)
             {
              InputBuffer x = (InputBuffer) inputBuffers.get(i);
              if (x.byteAt(0) == input)
               { // reopen
                inputbuffer = x;
                inputBuffers.remove(i);
                break;
               }
             }
            if (inputbuffer == null)
             { // assign new buffer to new handle
              inputbuffer = new InputBuffer();
              inputbuffer.appendRaw( (byte) input);
             }
           }
          // replace current handle
          handleIn = input;
         }
        else if (input < 27)
         { // terminates handle
          if (input == 23)
           { // X on
            xon = true;
        	if (forceSend)
             sendData();
           }
          else if (input == 24)
           xon = false; // X off
          else if (handleIn >= 15)
           { // decode supervisor call
            int fileID, length, id;
            long pos;
            StringBuffer filename = new StringBuffer();
            File file, file2, f;
            InputBuffer buffer;

            if ( (int) inputbuffer.byteAt(0) == 16)
             {
              status.setText(((((int) inputbuffer.buffer[2] & 0xff) << 8) + (((int) inputbuffer.buffer[1] & 0xff))) +
                            " byte in input buffer free");
              if (inputbuffer.buffer[3] != 0)
               if (stdInputStack.size() > 0)
                {
            	 if (forceSend)
                  sendData();
                }
               else if (noConsole == 0)
                {
                 buffer = new InputBuffer();
                 buffer.appendRaw((byte)0);
                 buffer.append(frame.inputDialog());
                 buffer.append((byte)10);
                 //buffer.appendRaw((byte)26);
                 sendData(buffer);
                }
             }
            else if (receiveload)
             { // quit load
              buffer = new InputBuffer();
              buffer.appendRaw( (byte) 15);
              buffer.append( (byte) MODULPROCESSED);
              buffer.appendRaw( (byte) 26);
              sendData(buffer);
              receiveload = false;
             }
            else if ( (int) inputbuffer.byteAt(0) == 15) // && !server.terminated())
             switch ( (int) inputbuffer.byteAt(1))
              {
               case CLOSE:
                           fileID = ( (int) inputbuffer.byteAt(2));
                           if ((fileID & 8) == 0)
                            {
                             fileID >>= 4;
                             status.setText(files[fileID].file.getName() + " closed");
                             files[fileID].close();
                            }
                           buffer = new InputBuffer();
                           buffer.appendRaw( (byte) 15);
                           buffer.appendRaw( (byte) 26);
                           sendData(buffer);
                           break;
               case OPEN:
                         fileID = ( (int) inputbuffer.byteAt(2));
                         for (int i = 3; i < inputbuffer.len; i++)
                          filename.append( (char) inputbuffer.byteAt(i));
                         id = fileID >> 4;
                         try
                          {
                           files[id] = new MyFile(makeFile(new String(filename)), fileID, true, false);
                           status.setText(new String(filename) + " opened");
                           buffer = new InputBuffer();
                           buffer.appendRaw( (byte) 15);
                           buffer.appendRaw( (byte) 26);
                           sendData(buffer);
                          }
                         catch (Exception ex)
                          {
                           consoleOut.addText(ex.getMessage() + '\n');
                          }
                         break;
               case CREATEF:
                            fileID = ( (int) inputbuffer.byteAt(2));
                            for (int i = 3; i < inputbuffer.len; i++)
                             filename.append( (char) inputbuffer.byteAt(i));
                            try
                             {
                              f = makeFile(new String(filename));
                              if ( (fileID & 8) != 0)
                               f.mkdirs();
                              else
                               {
                                id = fileID >> 4;
                                files[id] = new MyFile(f, fileID, false, false);
                               }
                              status.setText(new String(filename) + " created");
                              buffer = new InputBuffer();
                              buffer.appendRaw( (byte) 15);
                              buffer.appendRaw( (byte) 26);
                              sendData(buffer);
                             }
                            catch (Exception ex1)
                             {
                              consoleOut.addText(ex1.getMessage() + '\n');
                             }
                            break;
               case DELETE:
                            for (int i = 2; i < inputbuffer.len; i++)
                             filename.append( (char) inputbuffer.byteAt(i));
                            file = makeFile(new String(filename));
                            if (file.exists())
                             {
                              file.delete();
                              status.setText(new String(filename) + " deleted");
                             }
                            buffer = new InputBuffer();
                            buffer.appendRaw( (byte) 15);
                            buffer.appendRaw( (byte) 26);
                            sendData(buffer);
                            break;
               case GETFILEPOS:
                               fileID = ( (int) inputbuffer.byteAt(2)) >> 4;
                               buffer = new InputBuffer();
                               buffer.appendRaw( (byte) 15);
                               buffer.append(files[fileID].getFilePos());
                               buffer.appendRaw( (byte) 26);
                               sendData(buffer);
                               break;
               case GETFILESIZE:
                                fileID = ( (int) inputbuffer.byteAt(2)) >> 4;
                                buffer = new InputBuffer();
                                buffer.appendRaw( (byte) 15);
                                buffer.append(files[fileID].size());
                                buffer.appendRaw( (byte) 26);
                                sendData(buffer);
                                break;
               case INCLUDEF:
                              for (int i = 2; i < inputbuffer.len; i++)
                               filename.append( (char) inputbuffer.byteAt(i));
                              try
                               {
                                stdInputStack.add(stdInputStack.size(), new MyFile(makeFile(new String(filename)), 2, false, false));
                                status.setText(new String(filename) + " opened");
                            	if (forceSend)
                                 sendData();
                                buffer = new InputBuffer();
                                buffer.appendRaw( (byte) 15);
                                buffer.appendRaw( (byte) 26);
                                sendData(buffer);
                               }
                              catch (Exception ex2)
                               {
                                consoleOut.addText(ex2.getMessage() + '\n');
                               }
                              break;
               case READ:
                          fileID = ( (int) inputbuffer.byteAt(2)) >> 4;
                          length = ( (int) inputbuffer.byteAt(6) << 24) +
                                   ( (int) inputbuffer.byteAt(5) << 16) +
                                   ( (int) inputbuffer.byteAt(4) << 8) + ( (int) inputbuffer.byteAt(3));
                          if (fileID != 0)
                           sendData(fileID, length);
                          break;
               case LINE:
                          fileID = ( (int) inputbuffer.byteAt(2)) >> 4;
                          length = ( (int) inputbuffer.byteAt(6) << 24) +
                                   ( (int) inputbuffer.byteAt(5) << 16) +
                                   ( (int) inputbuffer.byteAt(4) << 8) + ( (int) inputbuffer.byteAt(3));
                          buffer = new InputBuffer();
                          buffer.appendRaw( (byte) fileID);
                          buffer.append(files[fileID].readLine(length));
                          buffer.appendRaw( (byte) 26);
                          sendData(buffer);
                          break;
               case SETFILEPOS:
                                fileID = ( (int) inputbuffer.byteAt(2)) >> 4;
                                pos = ( (long) inputbuffer.byteAt(10) << 56) +
                                      ( (long) inputbuffer.byteAt(9) << 48) +
                                      ( (long) inputbuffer.byteAt(8) << 40) +
                                      ( (long) inputbuffer.byteAt(7) << 32) +
                                      ( (long) inputbuffer.byteAt(6) << 24) +
                                      ( (long) inputbuffer.byteAt(5) << 16) +
                                      ( (long) inputbuffer.byteAt(4) << 8) +
                                      ( (long) inputbuffer.byteAt(3));
                                files[fileID].setFilePos(pos);
                                buffer = new InputBuffer();
                                buffer.appendRaw( (byte) 15);
                                buffer.appendRaw( (byte) 26);
                                sendData(buffer);
                                break;
               case SETFILESIZE:
                                 fileID = ( (int) inputbuffer.byteAt(2)) >> 4;
                                 pos = ( (long) inputbuffer.byteAt(10) << 56) +
                                       ( (long) inputbuffer.byteAt(9) << 48) +
                                       ( (long) inputbuffer.byteAt(8) << 40) +
                                       ( (long) inputbuffer.byteAt(7) << 32) +
                                       ( (long) inputbuffer.byteAt(6) << 24) +
                                       ( (long) inputbuffer.byteAt(5) << 16) +
                                       ( (long) inputbuffer.byteAt(4) << 8) +
                                       ( (long) inputbuffer.byteAt(3));
                                 files[fileID].setFilePos(pos);
                                 files[fileID].setSize(pos);
                                 buffer = new InputBuffer();
                                 buffer.appendRaw( (byte) 15);
                                 buffer.appendRaw( (byte) 26);
                                 sendData(buffer);
                                 break;
               case STATUS:
                            for (int i = 2; i < inputbuffer.len; i++)
                             filename.append( (char) inputbuffer.byteAt(i));
                            f = makeFile(new String(filename));
                            int mask = 0;
                            if (f.exists())
                             mask = 1;
                            if (f.isFile())
                             mask |= 2;
                            if (f.isDirectory())
                             mask |= 4;
                            if (f.isHidden())
                             mask |= 8;
                            if (f.canWrite())
                             mask |= 16;
                            if (f.canRead())
                             mask |= 32;
                            buffer = new InputBuffer();
                            buffer.appendRaw( (byte) 15);
                            buffer.append(mask);
                            buffer.appendRaw( (byte) 26);
                            sendData(buffer);
                            break;
               case RENAME:
                            inputbuffer.skip(2);
                            length = (int) inputbuffer.getByte() + (inputbuffer.getByte() << 8);
                            for (int i = 0; i < length; i++)
                             filename.append( (char) inputbuffer.getByte());
                            file = makeFile(new String(filename));
                            filename = new StringBuffer();
                            int j = length + 3;
                            length = (int) inputbuffer.getByte() + (inputbuffer.getByte() << 8) + j;
                            for (j++; j <= length; j++)
                             filename.append( (char) inputbuffer.getByte());
                            file2 = makeFile(new String(filename));
                            file.renameTo(file2);
                            status.setText(file.getName() + " renamed to " + file2.getName());
                            buffer = new InputBuffer();
                            buffer.appendRaw( (byte) 15);
                            buffer.appendRaw( (byte) 26);
                            sendData(buffer);
                            break;
               case ABSOLUTE:
                              for (int i = 3; i < inputbuffer.len; i++)
                               filename.append( (char) inputbuffer.byteAt(i));
                              buffer = new InputBuffer();
                              buffer.appendRaw( (byte) 15);
                              buffer.append(makeFile(new String(filename)).getAbsolutePath());
                              buffer.appendRaw( (byte) 26);
                              sendData(buffer);
                              break;
               case RELATIVE:
                              for (int i = 3; i < inputbuffer.len; i++)
                               filename.append( (char) inputbuffer.byteAt(i));
                              buffer = new InputBuffer();
                              buffer.appendRaw( (byte) 15);
                              f = makeFile(new String(filename));
                              String[] l = (f.isDirectory()?f.list():new String[0]);
                              for (int n = 0; n < l.length; n++)
                               {
                                buffer.append((String)l[n]);
                                buffer.append((byte)'\n');
                               }
                              buffer.appendRaw( (byte) (!f.isDirectory()?25:26));
                              sendData(buffer);
                              break;
               case SETDATE:
                             int r;
                             for (r = 2; r < inputbuffer.len - 8; r++)
                              filename.append( (char) inputbuffer.byteAt(r));
                             f = makeFile(new String(filename));
                             pos = 0;
                             for (int i = 0; i < 8; i++)
                              pos |= (long) inputbuffer.byteAt(r++) << (i * 8);
                             buffer = new InputBuffer();
                             buffer.appendRaw( (byte) 15);
                             buffer.appendRaw( (byte) (f.setLastModified(pos)?26:25));
                             sendData(buffer);
                             break;
               case GETDATE:
                             for (int i = 2; i < inputbuffer.len; i++)
                              filename.append( (char) inputbuffer.byteAt(i));
                             f = makeFile(new String(filename));
                             buffer = new InputBuffer();
                             buffer.appendRaw( (byte) 15);
                             buffer.append(f.lastModified());
                             buffer.appendRaw( (byte) 26);
                             sendData(buffer);
                             break;
               case LISTDICT:
                              Dictionary.build(inputbuffer, inputbuffer.len);
                              status.setText("Dictionary read");
                              server.dictionaryRead();
                              break;
               case CLEAR:
                           consoleOut.clear();
                           status.setText("Console cleared");
                           break;
               case ATXY:
                          consoleOut.setCursor(inputbuffer.byteAt(2), inputbuffer.byteAt(3));
                          break;
               case MODULPROCESSED:
                                    length = ( (int) inputbuffer.byteAt(5) << 24) +
                                             ( (int) inputbuffer.byteAt(4) << 16) +
                                             ( (int) inputbuffer.byteAt(3) << 8) + ( (int) inputbuffer.byteAt(2));
                                    if (input == 26)
                                     status.setText("load done");
                                    else if (length == 0)
                                     status.setText("link failed");
                                    else
                                     status.setText(length + " characters erroneous received");

                                    consoleOut.addText(input == 26?"\nOK>":"\nFAILED>");

                                    restoreInput(1);

                                    server.terminated();

                                    if (loadList != null)
                                     {
                                      if (loadList.size() > 0)
                                       loadNext();
                                      else if (loadList.size() == 0)
                                       {
                                        preparedBuffer = (errorRecords != null && errorRecords.len > 2)? errorRecords: null;
                                        errorRecords = null;
                                        loadList = null;
                                      
                                        if (preparedBuffer != null)
                                         {
                                    	  saveInput();
                                          consoleOut.addText("late binding\n");
                                          preparedBuffer.appendRaw( (byte) 26);

                                          InputBuffer x = new InputBuffer();
                                          x.appendRaw( (byte) 244); // process error records

                                          server.prepared(true);
                                          sendData(x);
                                          server.terminated();
                                         }
                                       }
                                     }
                                    break;
               case MEMORYUNUSED:
                                  used[0] = inputbuffer.byteAt(2) + (inputbuffer.byteAt(3) << 8) +
                                            (inputbuffer.byteAt(4) << 16) + (inputbuffer.byteAt(5) << 24);
                                  used[1] = inputbuffer.byteAt(6) + (inputbuffer.byteAt(7) << 8) +
                                            (inputbuffer.byteAt(8) << 16) + (inputbuffer.byteAt(9) << 24);
                                  used[2] = inputbuffer.byteAt(10) + (inputbuffer.byteAt(11) << 8) +
                                            (inputbuffer.byteAt(12) << 16) + (inputbuffer.byteAt(13) << 24);
                                  server.unusedRead();
                                  break;
               case READBUFFERED:
                                  sendData(preparedBuffer);
                                  server.preparedSent();
                                  break;
               case ASCII:
                           consoleOut.clearHold(inputbuffer.byteAt(2) != 0);
                           status.setText(inputbuffer.byteAt(2) != 0?"ASCII":"Unicode");
                           break;
               case ERRORREC:
                             if (errorRecords != null)
                              { // queue record
                               for(int i = 2; i < inputbuffer.len; i++)
                                errorRecords.append((byte)inputbuffer.byteAt(i));
                              }
                             break;
               case INCCONSOLE:
            	               saveInput();
            	               break;
               case DECCONSOLE:
            	               restoreInput(1);
            	               break;
               default:
                        inputbuffer.appendRaw( (byte) input);
                        break;
              }
            }
           // content of associated buffer obsolete
           inputbuffer.reset(1);

           if (inputBuffers.size() > 0)
            { // reassign preempted handle
             inputbuffer = (InputBuffer) inputBuffers.lastElement();
             inputBuffers.remove(inputBuffers.size() - 1);
             handleIn = (int) inputbuffer.buffer[0];
            }
           else
            { // standard input is default
             handleIn = 0;
             inputbuffer = new InputBuffer();
             inputbuffer.appendRaw( (byte) 0);
            }
           }
          else
           help = input == 27;
         }
      if (escape || input > 27)
       {
        if (handleIn == 0)
         if (consoleOut.unicode)
          consoleOut.addText((char) input); // send to console output
         else
          consoleOut.addText((byte) input);
        else if (handleIn < 15)
         toFile( (int) inputbuffer.buffer[0], input); // save to file
        else
         inputbuffer.appendRaw( (byte) input); // append
       }
      escape = help;
     }
   }

   /**
    * compose file
    * @param name String
    * @return File
    */
   private File makeFile(String name)
   {
    if (name.indexOf(':') < 0)
     return new File(frame.myProject, name);
    else
     return new File(name);
   }

   /**
    * write input to output device
    * @param handle int
    * @param input int
    */
   private void toFile(int handle, int input)
   {
    if (handle == 0)
     consoleOut.addText((char)input);
    else if (files[handle] != null)
     files[handle].write(input);
   }

   /**
    * put x into output queue
    * @param x InputBuffer
    */
   public synchronized void sendData(InputBuffer x)
   {
	if (x != null)
     toSend.add(x);

	if (forceSend)
     sendData();
   }

   /**
    * put file into queue
    * @param handle int
    * @param size int
    */
   private synchronized void sendData(int handle, int size)
   {
    fileStack.add(new Tuppel(handle, size >= 0?(long)size:Long.MAX_VALUE));

	if (forceSend)
     sendData();
   }

   /**
    * send next character from output queue
    * @return int
    */
   private synchronized int sendData()
   {
	int y;
	
    forceSend = false;
    
	while((y = send()) >= 0)
	 {
	  sfifo = Math.max(sfifo - 1, 0);
	  if (sfifo == 0)
	   break;
	 }
    
    forceSend = true;
	
	return y;
   }
   
   /**
    * send next character from output queue
    * @return int
    */
   private synchronized int send()
   {
    int x = -1;

    if ((sPort.getFlowControlMode() != SerialPort.FLOWCONTROL_RTSCTS_OUT || sPort.isCTS()) && xon)
     { // port is ready to send
      boolean pass = false;
      x = -1;

      if (nextToSend == -1)
       {
        // any datum in queued buffers?
        while (toSend.size() > 0 && x == -1)
         {
          InputBuffer y = (InputBuffer) toSend.get(0);
          x = y.getByte();
          if (x == -1)
           {
            toSend.remove(0);
           }
          else
           {
            pass = true;
            if (x == 27)
             nextToSend = y.getByte();
            if (sendTop != null)
             sendTop.preempted = true;
           }
         }
        // transfer a file?
        while (x == -1 && (fileStack.size() > 0 || stdInputStack.size() > 0))
         {
          if (fileStack.size() > 0)
           {
            Tuppel top = (Tuppel) fileStack.lastElement();
            if (sendTop != top && sendTop != null && sendTop.handle > 0)
             sendTop.preempted = true;
            sendTop = top;
           }
          else if (stdInputStack.size() > 0)
           {
            Tuppel top = (sendTop == null || sendTop.handle > 0)?new Tuppel(0, Long.MAX_VALUE):sendTop;
            files[0] = (MyFile) stdInputStack.lastElement();
            if (sendTop != null && sendTop.handle > 0)
             sendTop.preempted = true;
            sendTop = top;
           }

          // preempted, then send handle
          if (sendTop.preempted)
           {
            x = sendTop.handle;
            pass = true;
            sendTop.preempted = false;
           }
          else if (sendTop.length >= 0)
           { // requested blocklength unsatisfied
            x = files[sendTop.handle].read();
            if (x != -1)
             sendTop.length--;
           }

          // file already completely sent, then send ACK (26)
          if (x == -1)
           if (sendTop.handle == 0)
            {
             files[sendTop.handle].close();
             files[sendTop.handle] = null;
             stdInputStack.remove(stdInputStack.size() - 1);
             if (stdInputStack.size() == 0)
              {
               sendTop = null;
              }
            }
           else
            {
             fileStack.remove(fileStack.size() - 1);
             if (sendTop.handle == 15)
              {
               files[sendTop.handle].close();
               files[sendTop.handle] = null;
              }
             x = 26;
             pass = true;
             sendTop = null;
             if (fileStack.size() > 0)
              {
               sendTop = (Tuppel) fileStack.lastElement();
               sendTop.preempted = true;
              }
            }
         }

        // stuffing necessary?
        if (!pass)
         if (x > -1 && (x < 28 || x > 240))
          {
           nextToSend = x;
           x = 27;
          }
       }
      else
       {
        x = nextToSend;
        nextToSend = -1;
       }

      if (x != -1)
       try { os.write(x); } catch (Exception ex) { consoleOut.addText(ex.getMessage() + "********\n"); }
     }
    
    return x;
   }

  class Tuppel
   {
    int handle;
    long length;
    boolean preempted = true;

    Tuppel()
     {
      handle = 0;
      length = Long.MAX_VALUE;
     }

    Tuppel(int handle, long length)
     {
      this.handle = handle;
      this.length = length;
     }
   }

  class Automaton
   {
    private int state;
    class MyTask extends TimerTask
     {
      public void run()
       {
        reset();
       }
     }
    MyTask task = null;
    java.util.Timer t;

    Automaton() { t = new java.util.Timer(); reset(); }

    int getState()
     {
      return state;
     }

    void reset()
     {
      state = 1;
      if (task != null)
       task.cancel();
      task = null;
     }

    void abort()
     {
      if (state != 1)
       reset();
     }

    boolean assignDeck()
     {
      boolean p = state == 1;

      if (p)
       state = 2;

      return p;
     }

    boolean terminated()
     {
      boolean p = state != 1;

      if (p)
       state = 1;

      return p;
     }

    boolean prepared(boolean link)
     {
      boolean p = state == 1;

      if (p)
       state = link?2:3;

      return p;
     }

    boolean preparedSent()
     {
      boolean p = state == 3;

      if (p)
       state = 1;

      return p;
     }

    boolean readDictionary(int time)
     {
      boolean p = state == 1;

      if (p)
       {
        state = 4;
        task = new MyTask();
        t.schedule(task, time);
       }

      return p;
     }

    boolean dictionaryRead()
     {
      boolean p = state == 4;

      if (p)
       {
        state = 1;
        task.cancel();
       }

      return p;
     }

    boolean readUnused(int time)
     {
      boolean p = state == 1;

      if (p)
       {
        state = 5;
        task = new MyTask();
        t.schedule(task, time);
       }

      return p;
     }

    boolean unusedRead()
     {
      boolean p = state == 5;

      if (p)
       {
        state = 1;
        task.cancel();
       }

      return p;
     }

    boolean isReady()
     {
      return state == 1;
     }
   }
 }
