/*
 * @(#)MToolkit.java	1.168 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.*;
import java.awt.im.InputMethodHighlight;
import java.awt.im.spi.InputMethodDescriptor;
import java.awt.image.*;
import java.awt.peer.*;
import java.awt.datatransfer.Clipboard;
import java.awt.event.*;
import java.lang.reflect.*;
import java.lang.Math;
import java.io.*;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Properties;
import java.util.Map;

import sun.awt.image.ImageRepresentation;
import sun.awt.AppContext;
import sun.awt.AWTAutoShutdown;
import sun.awt.SunToolkit;
import sun.awt.GlobalCursorManager;
import sun.awt.DebugHelper;
import sun.awt.datatransfer.DataTransferer;
 
import java.awt.dnd.DragSource;
import java.awt.dnd.DragGestureListener;
import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.DragGestureRecognizer;
import java.awt.dnd.MouseDragGestureRecognizer;
import java.awt.dnd.InvalidDnDOperationException;
import java.awt.dnd.peer.DragSourceContextPeer;

import sun.awt.motif.X11InputMethod;
import sun.awt.X11GraphicsConfig;

import sun.awt.motif.MDragSourceContextPeer;

import sun.print.PrintJob2D;

import sun.misc.PerformanceLogger;

public class MToolkit extends SunToolkit implements Runnable {
    private static final DebugHelper dbg = DebugHelper.create(MToolkit.class);
    
    // the system clipboard - CLIPBOARD selection
    X11Clipboard clipboard;
    // the system selection - PRIMARY selection
    X11Clipboard selection;

    // Dynamic Layout Resize client code setting
    protected static boolean dynamicLayoutSetting = false;

    /**
     * True when the x settings have been loaded.
     */
    private boolean loadedXSettings;

    /**
     * Used to parse xsettings, created as needed.
     */
    private XSettingsParser xSettingsParser;

    /*
     * Note: The MToolkit object depends on the static initializer
     * of X11GraphicsEnvironment to initialize the connection to
     * the X11 server.
     */
    public static final X11GraphicsConfig config;
    static {
        if (GraphicsEnvironment.isHeadless()) {
            config = null;
        } else {
            config = (X11GraphicsConfig) (GraphicsEnvironment.
			     getLocalGraphicsEnvironment().
			     getDefaultScreenDevice().
			     getDefaultConfiguration());
	}
    }

    public static final String DATA_TRANSFERER_CLASS_NAME = "sun.awt.motif.MDataTransferer";

    public MToolkit() {
        super();
	if (PerformanceLogger.loggingEnabled()) {
	    PerformanceLogger.setTime("MToolkit construction");
	}
        if (!GraphicsEnvironment.isHeadless()) {
	    String mainClassName = null;

	    StackTraceElement trace[] = (new Throwable()).getStackTrace();
	    int bottom = trace.length - 1;
	    if (bottom >= 0) {
		mainClassName = trace[bottom].getClassName();
	    }
	    if (mainClassName == null || mainClassName.equals("")) {
		mainClassName = "AWT";
	    }

            init(mainClassName);
            SunToolkit.setDataTransfererClassName(DATA_TRANSFERER_CLASS_NAME);

            Thread toolkitThread = new Thread(this, "AWT-Motif");
            toolkitThread.setPriority(Thread.NORM_PRIORITY + 1);
            toolkitThread.setDaemon(true);
	    ThreadGroup mainTG = (ThreadGroup)AccessController.doPrivileged(
	    	new PrivilegedAction() {
	            public Object run() {
		        ThreadGroup currentTG =
		            Thread.currentThread().getThreadGroup();
			ThreadGroup parentTG = currentTG.getParent();
			while (parentTG != null) {
			    currentTG = parentTG;
			    parentTG = currentTG.getParent();
			}
			return currentTG;
		    }
	    });
	
	    Runtime.getRuntime().addShutdownHook(
	    	new Thread(mainTG, new Runnable() {
	            public void run() {
		        shutdown();
		    }
		}, "Shutdown-Thread")
	    );

            /*
             * Fix for 4701990.
             * AWTAutoShutdown state must be changed before the toolkit thread
             * starts to avoid race condition.
             */
            AWTAutoShutdown.notifyToolkitThreadBusy();

            toolkitThread.start();
        }
    }

    public native void init(String mainClassName);
    public native void run();
    private native void shutdown();

    /*
     * Create peer objects.
     */

    public ButtonPeer createButton(Button target) {
	ButtonPeer peer = new MButtonPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public TextFieldPeer createTextField(TextField target) {
	TextFieldPeer peer = new MTextFieldPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public LabelPeer createLabel(Label target) {
	LabelPeer peer = new MLabelPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public ListPeer createList(List target) {
	ListPeer peer = new MListPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public CheckboxPeer createCheckbox(Checkbox target) {
	CheckboxPeer peer = new MCheckboxPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public ScrollbarPeer createScrollbar(Scrollbar target) {
	ScrollbarPeer peer = new MScrollbarPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public ScrollPanePeer createScrollPane(ScrollPane target) {
	ScrollPanePeer peer = new MScrollPanePeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public TextAreaPeer createTextArea(TextArea target) {
	TextAreaPeer peer = new MTextAreaPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public ChoicePeer createChoice(Choice target) {
	ChoicePeer peer = new MChoicePeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public FramePeer  createFrame(Frame target) {
	FramePeer peer = new MFramePeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public CanvasPeer createCanvas(Canvas target) {
	CanvasPeer peer = new MCanvasPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public PanelPeer createPanel(Panel target) {
	PanelPeer peer = new MPanelPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public WindowPeer createWindow(Window target) {
	WindowPeer peer = new MWindowPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public DialogPeer createDialog(Dialog target) {
	DialogPeer peer = new MDialogPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public FileDialogPeer createFileDialog(FileDialog target) {
	FileDialogPeer peer = new MFileDialogPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public MenuBarPeer createMenuBar(MenuBar target) {
	MenuBarPeer peer = new MMenuBarPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public MenuPeer createMenu(Menu target) {
	MenuPeer peer = new MMenuPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public PopupMenuPeer createPopupMenu(PopupMenu target) {
	PopupMenuPeer peer = new MPopupMenuPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public MenuItemPeer createMenuItem(MenuItem target) {
	MenuItemPeer peer = new MMenuItemPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public CheckboxMenuItemPeer createCheckboxMenuItem(CheckboxMenuItem target) {
	CheckboxMenuItemPeer peer = new MCheckboxMenuItemPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public MEmbeddedFramePeer createEmbeddedFrame(MEmbeddedFrame target)
    {
        MEmbeddedFramePeer peer = new MEmbeddedFramePeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }


    public FontPeer getFontPeer(String name, int style){
	return new MFontPeer(name, style);
    }

    /*
     * On X, support for dynamic layout on resizing is governed by the
     * window manager.  If the window manager supports it, it happens
     * automatically.  The setter method for this property is
     * irrelevant on X.
     */
    public void setDynamicLayout(boolean b) {
        dynamicLayoutSetting = b;
    }

    protected boolean isDynamicLayoutSet() {
        return dynamicLayoutSetting;
    }

    /* Called from isDynamicLayoutActive() and from 
     * lazilyLoadDynamicLayoutSupportedProperty()
     */
    protected native boolean isDynamicLayoutSupportedNative();

    public boolean isDynamicLayoutActive() {
        return isDynamicLayoutSupportedNative();
    }

    public native boolean isFrameStateSupported(int state);

    static native ColorModel makeColorModel();
    static ColorModel screenmodel;

    static ColorModel getStaticColorModel() {
	if (screenmodel == null) {
	    screenmodel = config.getColorModel ();
	}
	return screenmodel;
    }

    public ColorModel getColorModel() {
	return getStaticColorModel();
    }

    public native int getScreenResolution();

    public Insets getScreenInsets(GraphicsConfiguration gc) {
        return new Insets(0,0,0,0);
    }

    protected native int getScreenWidth();
    protected native int getScreenHeight();

    public FontMetrics getFontMetrics(Font font) {
	/*
	// REMIND: platform font flag should be obsolete soon
        if (!RasterOutputManager.usesPlatformFont()) {
            return super.getFontMetrics(font);
        } else {
            return X11FontMetrics.getFontMetrics(font);
        }
	*/
	return super.getFontMetrics(font);
    }

    public native void sync();

    static boolean prepareScrImage(Image img, int w, int h, ImageObserver o) {
	if (w == 0 || h == 0) {
	    return true;
	}

        // Must be an OffScreenImage
        if (!(img instanceof X11Image)) {
            return true;
        }
            
	X11Image ximg = (X11Image) img;
	if (ximg.hasError()) {
	    if (o != null) {
		o.imageUpdate(img, ImageObserver.ERROR|ImageObserver.ABORT,
			      -1, -1, -1, -1);
	    }
	    return false;
	}
	ImageRepresentation ir = ximg.getImageRep();
	return ir.prepare(o);
    }

    static int checkScrImage(Image img, int w, int h, ImageObserver o) {

        if (!(img instanceof X11Image)) {
            return ImageObserver.ALLBITS;
        }
            
	X11Image ximg = (X11Image) img;
	int repbits;
	if (w == 0 || h == 0) {
	    repbits = ImageObserver.ALLBITS;
	} else {
	    repbits = ximg.getImageRep().check(o);
	}
	return ximg.check(o) | repbits;
    }

    public int checkImage(Image img, int w, int h, ImageObserver o) {
	return checkScrImage(img, w, h, o);
    }

    public boolean prepareImage(Image img, int w, int h, ImageObserver o) {
	return prepareScrImage(img, w, h, o);
    }

    public Image createImage(ImageProducer producer) {
	return new X11Image(producer);
    }

    public PrintJob getPrintJob(final Frame frame, final String doctitle,
				final Properties props) {
 
        if (GraphicsEnvironment.isHeadless()) {
            throw new IllegalArgumentException();
        }

        PrintJob2D printJob = new PrintJob2D(frame, doctitle, props);

        if (printJob.printDialog() == false) {
            printJob = null;
        }

        return printJob;   
    }

    public PrintJob getPrintJob(final Frame frame, final String doctitle,
				final JobAttributes jobAttributes,
				final PageAttributes pageAttributes) {


        if (GraphicsEnvironment.isHeadless()) {
            throw new IllegalArgumentException();
        }

        PrintJob2D printJob = new PrintJob2D(frame, doctitle,
                                             jobAttributes, pageAttributes);

        if (printJob.printDialog() == false) {
            printJob = null;
        }

        return printJob;
    }

    public native void beep();

    public  Clipboard getSystemClipboard() {
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
	  security.checkSystemClipboardAccess();
	}
        synchronized (this) {
            if (clipboard == null) {
                clipboard = new X11Clipboard("System", "CLIPBOARD");
            }
        }
	return clipboard;
    }
    
    public Clipboard getSystemSelection() {
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
            security.checkSystemClipboardAccess();
        }
        synchronized (this) {
            if (selection == null) {
                selection = new X11Clipboard("Selection", "PRIMARY");
            }
        }
        return selection;
    }
    
    public boolean getLockingKeyState(int key) {
        if (! (key == KeyEvent.VK_CAPS_LOCK || key == KeyEvent.VK_NUM_LOCK ||
               key == KeyEvent.VK_SCROLL_LOCK || key == KeyEvent.VK_KANA_LOCK)) {
            throw new IllegalArgumentException("invalid key for Toolkit.getLockingKeyState");
        }
        return getLockingKeyStateNative(key);
    }
    
    public native boolean getLockingKeyStateNative(int key);

    public native void loadSystemColors(int[] systemColors);

    /**
     * Give native peers the ability to query the native container 
     * given a native component (e.g. the direct parent may be lightweight).
     */
    public static Container getNativeContainer(Component c) {
	return Toolkit.getNativeContainer(c);
    }

    public static final Object targetToPeer(Object target) {
        return SunToolkit.targetToPeer(target);
    }

    public static final void targetDisposedPeer(Object target, Object peer) {
        SunToolkit.targetDisposedPeer(target, peer);
    }

    public DragSourceContextPeer createDragSourceContextPeer(DragGestureEvent dge) throws InvalidDnDOperationException {
	return MDragSourceContextPeer.createDragSourceContextPeer(dge);
    }

    public DragGestureRecognizer createDragGestureRecognizer(Class abstractRecognizerClass, DragSource ds, Component c, int srcActions, DragGestureListener dgl) {
	if (MouseDragGestureRecognizer.class.equals(abstractRecognizerClass))
	    return new MMouseDragGestureRecognizer(ds, c, srcActions, dgl);
	else
            return null;
    }

    /**
     * Returns a new input method adapter descriptor for native input methods.
     */
    public InputMethodDescriptor getInputMethodAdapterDescriptor() throws AWTException {
	return new X11InputMethodDescriptor();
    }

    /**
     * Returns a style map for the input method highlight.
     */
    public Map mapInputMethodHighlight(InputMethodHighlight highlight) {
	return X11InputMethod.mapInputMethodHighlight(highlight);
    }

    /**
     * Returns a new custom cursor.
     */
    public Cursor createCustomCursor(Image cursor, Point hotSpot, String name)
        throws IndexOutOfBoundsException {
        return new X11CustomCursor(cursor, hotSpot, name);
    }

    /**
     * Returns the supported cursor size
     */
    public Dimension getBestCursorSize(int preferredWidth, int preferredHeight) {
        return X11CustomCursor.getBestCursorSize(
            java.lang.Math.max(1,preferredWidth), java.lang.Math.max(1,preferredHeight));
    }

    public int getMaximumCursorColors() {
        return 2;  // Black and white.
    }

    private final static String prefix  = "DnD.Cursor.";
    private final static String postfix = ".32x32";

    protected Object lazilyLoadDesktopProperty(String name) {
	if (name.startsWith(prefix)) {
	    String cursorName = name.substring(prefix.length(), name.length()) + postfix;

	    try {
	        return Cursor.getSystemCustomCursor(cursorName);
	    } catch (AWTException awte) {
	 	System.err.println("cannot load system cursor: " + cursorName);
	
		return null;
	    }
	}

        if (name.equals("awt.dynamicLayoutSupported")) {
            return lazilyLoadDynamicLayoutSupportedProperty(name);
        }

        if (!loadedXSettings && name.startsWith("gnome.")) {
            loadedXSettings = true;
            if (!GraphicsEnvironment.isHeadless()) {
                loadXSettings();
                return desktopProperties.get(name);
            }
        }

	return super.lazilyLoadDesktopProperty(name);
    }

    /*
     * Called from lazilyLoadDesktopProperty because we may not know if 
     * the user has quit the previous window manager and started another. 
     */
    protected Boolean lazilyLoadDynamicLayoutSupportedProperty(String name) {
        boolean nativeDynamic = isDynamicLayoutSupportedNative();

        if (dbg.on) {
            dbg.print("In MTK.lazilyLoadDynamicLayoutSupportedProperty()" +
              "   nativeDynamic == " + nativeDynamic); 
        }

        return new Boolean(nativeDynamic);
    }

    private native int getMulticlickTime();


    protected void initializeDesktopProperties() {
	desktopProperties.put("DnD.Autoscroll.initialDelay",     new Integer(50));
	desktopProperties.put("DnD.Autoscroll.interval",         new Integer(50));
	desktopProperties.put("DnD.Autoscroll.cursorHysteresis", new Integer(5));

    /* As of 1.4, no wheel mice are supported on Solaris
     * however, they are on Linux, and there isn't a way to detect them,
     * so we leave this property unset to indicate we're not sure if there's
     * a wheel mouse or not.
     */
	//desktopProperties.put("awt.wheelMousePresent", new Boolean(false));

    // We don't want to call getMultilclickTime() if we're headless
    if (!GraphicsEnvironment.isHeadless()) {
        desktopProperties.put("awt.multiClickInterval",
                              new Integer(getMulticlickTime()));
    }
    }

    public RobotPeer createRobot(Robot target, GraphicsDevice screen) {
	/* 'target' is unused for now... */
	return new MRobotPeer(screen.getDefaultConfiguration());
    }

    //
    // The following support Gnome's equivalent of desktop properties.
    // A writeup of this can be found at:
    // http://www.freedesktop.org/standards/xsettings/xsettings.html
    //

    /**
     * Triggers a callback to parseXSettings with the x settings values
     * from the window server. Note that this will NOT call
     * parseXSettings if we are not running on a GNOME desktop.
     */
    private native void loadXSettings();

    /**
     * Callback from the native side indicating some, or all, of the
     * desktop properties have changed and need to be reloaded.
     * <code>data</code> is the byte array directly from the x server and
     * may be in little endian format.
     */
    private void parseXSettings(byte[] data) {
        // NOTE: This could be called from any thread.
        if (xSettingsParser == null) {
            xSettingsParser = new XSettingsParser();
        }
        xSettingsParser.parseXSettings(data);
    }


    /**
     * Used to parse xsettings. The resulting data is placed into desktop
     * properties.
     */
    private class XSettingsParser {
        private void parseXSettings(byte[] data) {
            int max = data.length;

            if (max > 12) {
                // First byte gives endianness and is either MSBFirst(0) or
                // LSBFirst(1)
                int[] index = new int[] { 0 };
                boolean isLittle = (getXSettingsUnsignedByte(data, index, 1)
                                    == 0);
                // next 3 bytes are unused, followed by CARD32 giving serial
                // then CARD32 giving number of settings
                index[0] = 8;

                int numberSettings = (int)getXSettingsUnsignedInt(data, index,
                                                                  12,isLittle);

                for (long settingCounter = 0; settingCounter <
                         numberSettings && index[0] < max; settingCounter++) {
                    parseXSetting(data, index, max, isLittle);
                }
            }
        }

        /**
         * Gets an unsigned short value from the byte array, throwing an
         * <code>IndexOutOfBoundsException</code> if there isn't enough space
         * in data for a short.
         *
         * @param data Array containing actual value
         * @param index First element gives index into data we should start at,
         *              on return index[0] is updated beyond the value.
         * @param isLittle Indicates if the data in data was created on a
         *        little endian machine.
         * @param max length of data
         * @return the unsigned short as an int
         */
        private int getXSettingsUnsignedShort(byte[] data, int[] index,
                                              int max, boolean isLittle)
                                    throws IndexOutOfBoundsException {
            if (index[0] + 2 > max) {
                throw new IndexOutOfBoundsException(
                    "getXSettingsUnsignedShort at " + index[0] + " max " +max);
            }
            int value;
            if (isLittle) {
                value = (0xFF & data[index[0]]) |
                    ((0xFF & data[index[0] + 1]) << 8);
            }
            else {
                value = ((0xFF & data[index[0]]) << 8) |
                    (0xFF & data[index[0] + 1]);
            }
            index[0] += 2;
            return value;
        }

        /**
         * Gets an unsigned byte value from the byte array, throwing an
         * <code>IndexOutOfBoundsException</code> if there isn't enough space
         * in data for a byte.
         *
         * @param data Array containing actual value
         * @param index First element gives index into data we should start at,
         *              on return index[0] is updated beyond the value.
         * @param max length of data
         * @return the unsigned byte as an int
         */
        private int getXSettingsUnsignedByte(byte[] data, int[] index, int max)
                                    throws IndexOutOfBoundsException {
            if (index[0] >= max) {
                throw new IndexOutOfBoundsException(
                    "getXSettingsUnsignedByte at " + index[0] + " max " + max);
            }
            int value = (0xFF & data[index[0]]);
            index[0]++;
            return value;
        }

        /**
         * Gets an unsigned int value from the byte array, throwing an
         * <code>IndexOutOfBoundsException</code> if there isn't enough space
         * in data for an int.
         *
         * @param data Array containing actual value
         * @param index First element gives index into data we should start at,
         *              on return index[0] is updated beyond the value.
         * @param max length of data
         * @return the unsigned int as a long
         */
        private long getXSettingsUnsignedInt(byte[] data, int[] index, int max,
                           boolean isLittle) throws IndexOutOfBoundsException {
            int offset = index[0];
            if (offset + 4 > max) {
                throw new IndexOutOfBoundsException(
                    "getXSettingsUnsignedInt at " + index[0] + " max " + max);
            }
            long value;

            if (isLittle) {
                value = (0xFF & data[offset++]) |
                        ((0xFF & data[offset++]) << 8) |
                        ((0xFF & data[offset++]) << 16) |
                        ((0xFF & data[offset++]) << 24);
            }
            else {
                value = ((0xFF & data[offset++]) << 24) |
                      ((0xFF & data[offset++]) << 16) |
                      ((0xFF & data[offset++]) << 8) | (0xFF & data[offset++]);
            }
            index[0] += 4;
            return value;
        }

        /**
         * Gets a String from the byte array, throwing an
         * <code>IndexOutOfBoundsException</code> if there isn't enough space
         * in data for a String. The first two, or four, bytes define the
         * size of the String followed by the actual String.
         *
         * @param data Array containing actual value
         * @param index First element gives index into data we should start at,
         *              on return index[0] is updated beyond the value.
         * @param max length of data
         * @param isFour Indicates if the length is given as a 4 byte int
         *        or two byte int.
         * @return Extracted String
         */
        private String getXSettingsString(byte[] data, int[] index, int max,
                                          boolean isLittle, boolean isFour)
                                          throws IndexOutOfBoundsException {
            int nameLength;

            if (isFour) {
                nameLength = (int)getXSettingsUnsignedInt(data, index, max,
                                                          isLittle);
            }
            else {
                nameLength = getXSettingsUnsignedShort(data, index, max,
                                                       isLittle);
            }

            if (index[0] + nameLength > max) {
                throw new IndexOutOfBoundsException("getXSettingsString " +
                                                    index[0] + " " + max);
            }
            String string = new String(data, 0, index[0], nameLength);
            index[0] += nameLength;
            // pad
            index[0] = (index[0] + 3) / 4 * 4;
            return string;
        }

        /**
         * Parses a particular x setting throwing an
         * <code>IndexOutOfBoundsException</code> if there was enough room
         * in the array for the setting.
         *
         * @param data Byte array to extract data from
         * @param index Starting index to parse from
         * @param max length of bytes
         * @param isLittle Indicates if the data in data was created on a
         *        little endian machine.
         */
        private void parseXSetting(byte[] data, int[] index, int max,
                           boolean isLittle) throws IndexOutOfBoundsException{
            int type = getXSettingsUnsignedByte(data, index, max);

            // pad
            index[0]++;

            String name = getXSettingsString(data, index, max, isLittle,false);
            long lastChanged = getXSettingsUnsignedInt(data, index, max,
                                                       isLittle);
            Object value = null;

            if (type == 0) {
                // Integer
                long lValue = getXSettingsUnsignedInt(data, index, max,
                                                      isLittle);

                value = new Integer((int)lValue);
            }
            else if (type == 1) {
                // String
                value = getXSettingsString(data, index, max, isLittle, true);
            }
            else if (type == 2) {
                // color
                int red = getXSettingsUnsignedShort(data, index, max,isLittle);
                int green = getXSettingsUnsignedShort(data, index, max,
                                                      isLittle);
                int blue = getXSettingsUnsignedShort(data, index, max,
                                                     isLittle);
                int alpha = getXSettingsUnsignedShort(data, index, max,
                                                      isLittle);

                value = new Color(red / 65535.0f, green  / 65535.0f,
                                  blue  / 65535.0f, alpha / 65535.0f);
            }
            else {
                throw new IndexOutOfBoundsException("Unknown type: " + type);
            }
            if (name != null) {
                // Push the value to the desktop properties, prefixing
                // it with 'gnome.' to avoid name collision.
                name = "gnome." + name;
                setDesktopProperty(name, value);
            }
        }
    } // class XSettingsParser
} // class MToolkit

