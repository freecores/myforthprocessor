/*
 * @(#)ConsoleWindow.java	1.76 02/08/20
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/*
 * Open a new Java Console used to display the Syste.out and System.err
 * error messages sent by the running Applets or JavaBeans components
 *
 * @version 1.0
 * @Author Jerome Dochez
 * @author Stanley Man-Kit Ho
 * @Author Devananda Jayaraman
 */

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.EventQueue;
import java.awt.FlowLayout;
import java.awt.Insets;
import java.awt.Frame;
import java.awt.SystemColor;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.InvocationEvent;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRootPane;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JViewport;
import javax.swing.KeyStroke;
import javax.swing.WindowConstants;
import java.text.MessageFormat;
import java.util.TreeSet;
import java.util.Collection;
import java.util.Iterator;
import java.security.Policy;
import sun.awt.AppContext;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.security.TrustDecider;
import sun.plugin.security.X509PluginTrustManager;
import sun.plugin.security.CertificateHostnameVerifier;
import sun.plugin.util.Trace;
import sun.plugin.util.PluginLogger;
import sun.plugin.cache.JarCache;
import sun.plugin.cache.FileCache;
import sun.plugin.services.BrowserService;
import sun.plugin.util.PluginSysUtil;

class ConsoleWindow extends JFrame
{
    {
	// Change metal theme - make all fonts PLAIN instead of
	// BOLD - this would only affect the plugin dialogs.
	//

	sun.plugin.util.PluginUIManager.setTheme();
    }

    /*
     * Create the new console window
     */
    ConsoleWindow()
    {
    	super(ResourceHandler.getMessage("console.caption"));

	try
	{
	    // Determine the platform that we are on
	    //
            String osName = (String) java.security.AccessController.doPrivileged(
                                    new sun.security.action.GetPropertyAction("os.name"));

	    if (osName.indexOf("Windows") != -1)
		onWindows = true;

            String workaround = (String) java.security.AccessController.doPrivileged(
                                         new sun.security.action.GetPropertyAction("mozilla.workaround", "false"));

	    if (workaround != null && workaround.equalsIgnoreCase("true"))
		isMozilla = true;

	    BrowserService service = sun.plugin.services.ServiceProvider.getService();

	    iconifiedOnClose = service.isConsoleIconifiedOnClose();
	}
	catch (Exception e)
	{
	    e.printStackTrace();
	}


	setSize(450,400);
	setResizable(true);

	// Use default look and feel for decoration
//	setUndecorated(true);
//	getRootPane().setWindowDecorationStyle(JRootPane.FRAME);

	// Make sure we don't do anything exception in the listener.
	//

	setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);

	/* WORKAROUND>>>
	 * There seems to be a major problem with double buffering in swing and Netscape 6.
	 * These two lines are here to turn off buffering until the problem with Netscape 6
	 * or swing can be resolved
	 */
	if (onWindows == false && isMozilla == true)
	{
	    getRootPane().setDoubleBuffered(false);
	    ((JPanel) getContentPane()).setDoubleBuffered(false);
	}

	getContentPane().setLayout(new BorderLayout());

	textArea = new JTextArea();
	textArea.setEditable(false);
	textArea.setMargin(new Insets(0, 5, 0, 0));
	scroller = new JScrollPane(textArea);
	JViewport viewport = scroller.getViewport();
	viewport.setScrollMode(JViewport.BLIT_SCROLL_MODE);

	sbVer = scroller.getVerticalScrollBar();
	sbHor = scroller.getHorizontalScrollBar();
	getContentPane().add(scroller, BorderLayout.CENTER);

	ActionListener dumpThreadStack = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		StringBuffer buffer = new StringBuffer();

		buffer.append(ResourceHandler.getMessage("console.dump.stack"));
		buffer.append(ResourceHandler.getMessage("console.menu.text.top"));
		buffer.append(JavaRunTime.dumpAllStacks());
		buffer.append(ResourceHandler.getMessage("console.menu.text.tail"));
		buffer.append(ResourceHandler.getMessage("console.done"));

		System.out.println(buffer.toString());
	    }
	};

	ActionListener showThreads = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		StringBuffer buffer = new StringBuffer();

		buffer.append(ResourceHandler.getMessage("console.dump.thread"));

		Thread t = Thread.currentThread();
		ThreadGroup tg = JavaRunTime.getPluginThreadGroup().getParent();
		dumpThreadGroup(tg, buffer);

		buffer.append(ResourceHandler.getMessage("console.done"));

		System.out.println(buffer.toString());
	    }
	};

	ActionListener reloadPolicyConfig = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Policy policy = Policy.getPolicy();

		System.out.print(ResourceHandler.getMessage("console.reload.policy"));
		policy.refresh();
		System.out.println(ResourceHandler.getMessage("console.completed"));
	    }
	};

	ActionListener reloadProxyConfig = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		System.out.println(ResourceHandler.getMessage("console.reload.proxy"));
		sun.plugin.net.proxy.PluginProxyManager.reset();
		System.out.println(ResourceHandler.getMessage("console.done"));
	    }
	};

	ActionListener showSystemProperties = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		displaySystemProperties();
	    }
	};

	ActionListener showHelp = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		String helpString = displayHelp();
		System.out.print(helpString);
	    }
	};

	ActionListener showClassLoaderCache = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		ClassLoaderInfo.dumpClassLoaderCache(System.out);
	    }
	};

	ActionListener clearClassLoaderCache = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		//clear cache files and jars in memory
		JarCache.clearLoadedJars();
		FileCache.clearLoadedFiles();

		ClassLoaderInfo.clearClassLoaderCache();

		// We MUST reset the trust decider so newer classes
		// with different cert may be loaded
		TrustDecider.reset();

		// Reset trust manager for HTTPS
		X509PluginTrustManager.reset();

		// Reset the certificate hostname verifier for HTTPS
		CertificateHostnameVerifier.reset();

    		System.out.println(ResourceHandler.getMessage("console.clear.classloader"));
	    }
	};

	ActionListener clearConsole = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		// Add header in the beginning of Console window
		String strVersion = displayVersion();
		String strHelp = displayHelp();
		String strHeader = strVersion + "\n" + strHelp;
		textArea.setText(strHeader);

 		//textArea.setText("");
		textArea.revalidate();

		// Display version info
		//displayVersion();

		// Display help information after the screen is cleared.
		//displayHelp();
	    }
	};

	ActionListener copyConsole = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		// Obtain current text selections
		int selectionStart = textArea.getSelectionStart();
		int selectionEnd = textArea.getSelectionEnd();

		// If no selection, copy all
		if (selectionEnd - selectionStart <= 0)
		    textArea.selectAll();

		// Copy text to clipboard
		textArea.copy();
	    }
	};

	ActionListener closeConsole = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		if (iconifiedOnClose)
		{
		    // On other platform with Netscape 4.x, closing the window
		    // will only iconified the console.
		    //
		    setState(Frame.ICONIFIED);
		}
		else
		{
		    setVisible(false);
		}
	    }
	};

	final ActionListener showMemory = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		long freeMemory = Runtime.getRuntime().freeMemory() / 1024;
		long totalMemory = Runtime.getRuntime().totalMemory() / 1024;
		long percentFree = (long) (100.0 / (((double)totalMemory) / freeMemory));

		MessageFormat mf = new MessageFormat(ResourceHandler.getMessage("console.memory"));
		Object[] args = { new Long(totalMemory), new Long(freeMemory), new Long(percentFree) };

    		System.out.print(mf.format(args));
		System.out.println(ResourceHandler.getMessage("console.completed"));
	    }
	};


	ActionListener runFinalize = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		System.out.print(ResourceHandler.getMessage("console.finalize"));
		System.runFinalization();
		System.out.println(ResourceHandler.getMessage("console.completed"));

		// Display memory after finalization
		showMemory.actionPerformed(e);
	    }
	};


	ActionListener runGC = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
    		System.out.print(ResourceHandler.getMessage("console.gc"));
		System.gc();
		System.out.println(ResourceHandler.getMessage("console.completed"));

		// Display memory after garbage collection
		showMemory.actionPerformed(e);
	    }
	};

	ActionListener traceLevel0 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(false);
		Trace.setNetTrace(false);
		Trace.setSecurityTrace(false);
		Trace.setExtTrace(false);
		Trace.setLiveConnectTrace(false);
    		System.out.println(ResourceHandler.getMessage("console.trace.level.0"));
	    }
	};

	ActionListener traceLevel1 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(true);
		Trace.setNetTrace(false);
		Trace.setSecurityTrace(false);
		Trace.setExtTrace(false);
		Trace.setLiveConnectTrace(false);
    		System.out.println(ResourceHandler.getMessage("console.trace.level.1"));
	    }
	};

	ActionListener traceLevel2 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(true);
		Trace.setNetTrace(true);
		Trace.setSecurityTrace(false);
		Trace.setExtTrace(false);
		Trace.setLiveConnectTrace(false);
    		System.out.println(ResourceHandler.getMessage("console.trace.level.2"));
	    }
	};

	ActionListener traceLevel3 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(true);
		Trace.setNetTrace(true);
		Trace.setSecurityTrace(true);
		Trace.setExtTrace(false);
		Trace.setLiveConnectTrace(false);
    		System.out.println(ResourceHandler.getMessage("console.trace.level.3"));
	    }
	};

	ActionListener traceLevel4 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(true);
		Trace.setNetTrace(true);
		Trace.setSecurityTrace(true);
		Trace.setExtTrace(true);
		Trace.setLiveConnectTrace(false);
    		System.out.println(ResourceHandler.getMessage("console.trace.level.4"));
	    }
	};

	ActionListener traceLevel5 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(true);
		Trace.setNetTrace(true);
		Trace.setSecurityTrace(true);
		Trace.setExtTrace(true);
		Trace.setLiveConnectTrace(true);
    		System.out.println(ResourceHandler.getMessage("console.trace.level.5"));
	    }
	};

	ActionListener logging = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		PluginLogger.setLogging(!PluginLogger.getLogging());
    		System.out.println(ResourceHandler.getMessage("console.log")
				   + PluginLogger.getLogging()
				   + ResourceHandler.getMessage("console.completed"));
	    }
	};

	if (onWindows)
	{
	    String runjcovOption = System.getProperty("javaplugin.vm.options");
	    if (runjcovOption.indexOf("-Xrunjcov") != -1)
	    {
	        ActionListener runJcov = new ActionListener()
		{
		    public void actionPerformed(ActionEvent e)
	            {
		        boolean ret = WJcovUtil.dumpJcovData();
			if (ret == false)
			{
			    System.out.println(ResourceHandler.getMessage("console.jcov.error"));
			}
			else
			{
			    System.out.println(ResourceHandler.getMessage("console.jcov.info"));
			 }
		     }
		};
		textArea.registerKeyboardAction(    runJcov,
					    	    KeyStroke.getKeyStroke(KeyEvent.VK_J, 0),
					    	    JComponent.WHEN_IN_FOCUSED_WINDOW	);
		}
	}

	textArea.registerKeyboardAction(    dumpThreadStack,
					    KeyStroke.getKeyStroke(KeyEvent.VK_V, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    showThreads,
					    KeyStroke.getKeyStroke(KeyEvent.VK_T, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    reloadProxyConfig,
					    KeyStroke.getKeyStroke(KeyEvent.VK_P, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    reloadPolicyConfig,
					    KeyStroke.getKeyStroke(KeyEvent.VK_R, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    showSystemProperties,
					    KeyStroke.getKeyStroke(KeyEvent.VK_S, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    showHelp,
					    KeyStroke.getKeyStroke(KeyEvent.VK_H, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    showMemory,
					    KeyStroke.getKeyStroke(KeyEvent.VK_M, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    clearConsole,
					    KeyStroke.getKeyStroke(KeyEvent.VK_C, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    runGC,
					    KeyStroke.getKeyStroke(KeyEvent.VK_G, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    runFinalize,
					    KeyStroke.getKeyStroke(KeyEvent.VK_F, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    closeConsole,
					    KeyStroke.getKeyStroke(KeyEvent.VK_Q, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    traceLevel0,
					    KeyStroke.getKeyStroke(KeyEvent.VK_0, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    traceLevel1,
					    KeyStroke.getKeyStroke(KeyEvent.VK_1, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    traceLevel2,
					    KeyStroke.getKeyStroke(KeyEvent.VK_2, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    traceLevel3,
					    KeyStroke.getKeyStroke(KeyEvent.VK_3, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    traceLevel4,
					    KeyStroke.getKeyStroke(KeyEvent.VK_4, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    traceLevel5,
					    KeyStroke.getKeyStroke(KeyEvent.VK_5, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    clearClassLoaderCache,
					    KeyStroke.getKeyStroke(KeyEvent.VK_X, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    showClassLoaderCache,
					    KeyStroke.getKeyStroke(KeyEvent.VK_L, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(    logging,
					    KeyStroke.getKeyStroke(KeyEvent.VK_O, 0),
					    JComponent.WHEN_IN_FOCUSED_WINDOW	);

	/* Get our resource strings.  We are guaranteed that ResourceHandler
         * has already loaded its resource because the same method that
         * did the resource loading subsequently calls showConsoleWindow.
         */
	JButton clear = new JButton(ResourceHandler.getMessage("console.clear"));
	clear.setMnemonic(ResourceHandler.getAcceleratorKey("console.clear"));
	JButton copy = new JButton(ResourceHandler.getMessage("console.copy"));
	copy.setMnemonic(ResourceHandler.getAcceleratorKey("console.copy"));
	JButton close = new JButton(ResourceHandler.getMessage("console.close"));
	close.setMnemonic(ResourceHandler.getAcceleratorKey("console.close"));

	JPanel panel = new JPanel();
	panel.setLayout(new FlowLayout(FlowLayout.CENTER));
	panel.add(clear);
	panel.add(new JLabel("    "));
	panel.add(copy);
	panel.add(new JLabel("    "));
	panel.add(close);

	// We use the background RGB from the SystemColor.control
//	int rgb = SystemColor.control.getRGB();
//	panel.setBackground(new Color(rgb));
	getContentPane().add(panel, BorderLayout.SOUTH);

        addWindowListener(  new WindowAdapter()
	{
 		public void windowClosing(WindowEvent e)
		{
		    if (iconifiedOnClose)
		    {
			// On other platform, closing the window
			// will only iconified the console.
			//
			setState(Frame.ICONIFIED);
		    }
		    else
		    {
			setVisible(false);
		    }
		}
	});

	clear.addActionListener(clearConsole);
	copy.addActionListener(copyConsole);
	close.addActionListener(closeConsole);
    }

    /**
     * <p>
     * Prints an output message to the console window
     * </p>
     *
     * @param text text to be printed
     */
    public void append(final String text)
    {
	// We need to post the event to the swing component's event queue
	PluginSysUtil.invokeLater(new Runnable(){
	    public void run() {
		// If the size exceed limit, it wipe the required
		// portion of text from the beginning.
		int diff = textArea.getText().length() + text.length() - TEXT_LIMIT;
		if (diff > 0){
		    textArea.replaceRange("",0,diff);
		}

		textArea.append(text);
		textArea.revalidate();
		setScrollPosition();
	    }
	 });
    }

    /**
     * Dump threads in threadgroup recursively.
     *
     * @param tg Root of thread group tree.
     * @param buffer String buffer to append output
     */
    private void dumpThreadGroup(ThreadGroup tg, StringBuffer sb)
    {
	if (tg != null)
	{
	    // Clean up thread group if necessary.
	    try
	    {
		if (tg.activeCount() == 0 && tg.activeGroupCount() == 0 && tg.isDestroyed() == false)
		    tg.destroy();
	    }
	    catch (Throwable e)
	    {
	    }

	    // Dump thread group
	    sb.append("Group " + tg.getName());
	    sb.append(",ac=" + tg.activeCount());
	    sb.append(",agc=" + tg.activeGroupCount());
	    sb.append(",pri=" + tg.getMaxPriority());
	    if (tg.isDestroyed())
		sb.append(",destoyed");
	    if (tg.isDaemon())
		sb.append(",daemon");
	    sb.append("\n");

	    Thread[] tt = new Thread[1000];
	    tg.enumerate(tt, false);

	    // Dump threads within thread group
	    for (int i=0; i < tt.length; i++)
	    {
		if (tt[i] != null)
		{
		    sb.append("    ");
		    sb.append(tt[i].getName());
		    sb.append(",");
		    sb.append(tt[i].getPriority());

		    if (tt[i].isAlive())
			sb.append(",alive");
		    else
			sb.append(",not alive");

		    if (tt[i].isDaemon())
			sb.append(",dameon");

		    if (tt[i].isInterrupted())
			sb.append(",interrupted");

		    sb.append("\n");
		}
	    }

	    // Obtain sub-thread group
	    ThreadGroup[] tgArray = new ThreadGroup[1000];
	    tg.enumerate(tgArray, false);

	    // Dump sub-thread group
	    for (int i=0; i < tgArray.length; i++)
	    {
		if (tgArray[i] != null)
		    dumpThreadGroup(tgArray[i], sb);
	    }
	}
    }


    /**
     * Display help information.
     */
    public static String displayHelp()
    {
	StringBuffer buffer = new StringBuffer();

	buffer.append(ResourceHandler.getMessage("console.menu.text.top"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.c"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.f"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.g"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.h"));

	String os = (String) java.security.AccessController.doPrivileged(
                              new sun.security.action.GetPropertyAction("os.name"));

	if (os.indexOf("Windows") != -1)
	{
	    if (System.getProperty("javaplugin.vm.options").indexOf("-Xrunjcov") != -1)
	        buffer.append(ResourceHandler.getMessage("console.menu.text.j"));
	}
	buffer.append(ResourceHandler.getMessage("console.menu.text.l"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.m"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.o"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.p"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.q"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.r"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.s"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.t"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.v"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.x"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.0"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.tail"));

	//System.out.print(buffer.toString());
	return (buffer.toString());
    }

    public static String displayVersion()
    {
	// Tell people a little about ourselves.  This is partly so we can
	// get better information when people report problems.

	StringBuffer buffer = new StringBuffer();

	buffer.append(ResourceHandler.getMessage("product_name"));

	String javapluginVersion = System.getProperty("javaplugin.version");

	if (javapluginVersion != null)
	{
	    buffer.append(": " + ResourceHandler.getMessage("version") + " "
			  + System.getProperty("javaplugin.version"));
	}
	buffer.append("\n");
	buffer.append(ResourceHandler.getMessage("using_jre_version"));
	buffer.append(" ");
	buffer.append(System.getProperty("java.version"));
	buffer.append(" ");
	buffer.append(System.getProperty("java.vm.name"));
	buffer.append("\n");
	buffer.append(ResourceHandler.getMessage("user_home_dir"));
	buffer.append(" = ");
	buffer.append(System.getProperty("user.home"));

	//System.out.println(buffer.toString());
	return (buffer.toString());
    }


    public static void displaySystemProperties()
    {
	// Obtain all properties name and sort them first
	//
	TreeSet treeSet = new TreeSet();

	for (java.util.Enumeration e = System.getProperties().propertyNames(); e.hasMoreElements();)
	{
	    treeSet.add(e.nextElement());
	}


	StringBuffer buffer = new StringBuffer();


	buffer.append(ResourceHandler.getMessage("console.dump.properties"));
	buffer.append(ResourceHandler.getMessage("console.menu.text.top"));

	// Iterate the sorted properties
	//
	for (Iterator iter = treeSet.iterator(); iter.hasNext();)
	{
	    String key = (String) iter.next();
	    String value = System.getProperty(key);

	    if (value != null)
    	    {
		if (value.equals("\n"))
		    value = "\\n";
		else if (value.equals("\r"))
		    value = "\\r";
		else if (value.equals("\r\n"))
		    value = "\\r\\n";
		else if (value.equals("\n\r"))
		    value = "\\n\\r";
		else if (value.equals("\n\n"))
		    value = "\\n\\n";
		else if (value.equals("\r\r"))
		    value = "\\r\\r";
	    }

	    buffer.append(key + " = " + value + "\n");
	}

	buffer.append(ResourceHandler.getMessage("console.menu.text.tail"));
	buffer.append(ResourceHandler.getMessage("console.done"));

	System.out.println(buffer.toString());
    }

    /**
     * Set the horizontal and vertical scroll bar positions
     */
    public void setScrollPosition()
    {
 	scroller.validate();
	sbVer.setValue(sbVer.getMaximum());
	sbHor.setValue(sbHor.getMinimum());
    }


    /**
     * Show Java Console Window
     */
    public void showConsole(boolean visible)
    {
	if (iconifiedOnClose)
	{
	    // For iconified state, visible means normal
	    // windows. invisible means iconified windows.
	    //
	    if (visible)
	    {
		setState(Frame.NORMAL);
	    }
	    else
	    {
		setState(Frame.ICONIFIED);
	    }

	    setVisible(true);
	}
	else
	{
	    // Caution: changing the order of the following
	    // call may cause deadlock in the browser because
	    // of the way taskbar notification is handled in
	    // the platform.
	    //
	    if (isVisible() != visible)
	    {
		setVisible(visible);
	    }

	    if (visible)
		toFront();
	}
    }


    /**
     * Return true if console is visible.
     */
    public boolean isConsoleVisible()
    {
	if (iconifiedOnClose)
	{
	    // For iconified state, visible means normal
	    // windows. invisible means iconified windows.
	    return (getState() == Frame.NORMAL);
	}
	else
	{
	    // Otherwise, visible means the windows is actually
	    // visible.
	    return isVisible();
	}
    }

    private JTextArea textArea;
    private JScrollPane scroller;
    private JScrollBar sbHor, sbVer;
    private boolean onWindows = false;
    private boolean isMozilla = false;
    private boolean iconifiedOnClose = false;

    /* Text limited size to show on console window */
    private static final int TEXT_LIMIT = 0xFFFFF;
}
