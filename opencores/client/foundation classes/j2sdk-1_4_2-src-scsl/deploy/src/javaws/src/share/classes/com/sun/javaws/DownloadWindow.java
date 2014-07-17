/*
 * @(#)DownloadWindow.java	1.74 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 *  DownloadWindow
 *
 * Implementation of the standard accept/loading-progress
 * window shown when JavaWS starts up
 *
 * Call sequence:
 *
 *      DownloadWindow window = new DownloadWindow(launchDesc);
 *      jsw.buildIntroScreen(...)
 *      jsw.getFrame().show();
 *      ...
 *      jsw.showProgressScreen();
 *      boolean r = jsw.waitForLoading(loader, name);
 *      ...
 *      jsw.cleanWindow()  / jsw.disposeWindow()
 *
 */
package com.sun.javaws;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import javax.swing.*;
import javax.swing.border.*;
import java.security.*;
import java.net.URL;
import java.io.File;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.InformationDesc;
import com.sun.javaws.jnl.IconDesc;
import com.sun.javaws.cache.CacheImageLoader;
import com.sun.javaws.cache.CacheImageLoaderCallback;

public class DownloadWindow extends WindowAdapter implements ActionListener, LaunchDownload.DownloadProgress, CacheImageLoaderCallback  {
    
    /**  Reference to window */
    private JFrame _frame = null;
    
    /** App. info */
    private String _title;
    private String _vendor;
    
    // Download statistics
    private long _estimatedDownloadSize = 0;
    private long _totalDownloadedBytes = 0;
    private URL _currentUrl = null;
    
    // Estimated download time timer
    static final int TIMER_UPDATE_RATE = 1000;  // Update every second
    static final int TIMER_INITIAL_DELAY = 10;  // Don't show up the first 10 sec.
    static final int TIMER_AVERAGE_SIZE  = 10;  // Length of average
    Timer    _timerObject =  null;
    long[]   _timerDownloadAverage = new long[TIMER_AVERAGE_SIZE]; // Sliding average of download
    int      _timerCount = 0;
    long     _timerLastBytesCount = 0;
    boolean  _timerOn = false;   // Time is turned on if file found in cache
    
    // Heart-beat controls
    static final int HEART_BEAT_RATE = 250;
    static final boolean[] HEART_BEAT_RYTHM = { false, false, false, true, false, true };
    Timer    _heartbeatTimer = null;
    Object   _heartbeatLock  = new Object(); // To synchronize on
    int      _heartbeatCount = 0;
    boolean  _heartbeatOn = false;
    
    // Cancel does not do a System.exit(-1) if it invoked from the DownloadService
    boolean _isCanceled = false;
    boolean _exitOnCancel = true;
    
    /**
     * The image for the app.
     */
    private Image _appImage;
    
    /**
     * Contains text and any pertanent widgets (ok/cancel buttons, or
     * progress bar).
     */
    private JButton      _cancelButton      = null;
    private JLabel       _titleLabel        = null;
    private JLabel       _vendorLabel       = null;
    private JLabel       _infoStatus        = null;
    private JLabel       _infoProgressTxt   = null;
    private JLabel       _infoEstimatedTime = null;
    private JProgressBar _infoProgressBar   = null;
    private JLabel       _imageLabel        = null;
    
    private static final int _yRestriction = 20;
    
    private static final int MAX_DISPLAY = 20;

    private static final String LEAD = "...";

    /**
     * Model of the progress bar.
     */
    private DefaultBoundedRangeModel _loadingModel;
    /**
     * Listener on the cancelButton.
     */
    private ActionListener _cancelActionListener;
    
    /** Create main window */
    public DownloadWindow(LaunchDesc ld, boolean existOnCancel) {
        setLaunchDesc(ld, existOnCancel);
    }
    
    public DownloadWindow() {
	// Nothing to do. setLaunchDesc should be called
    }
    
    /** This is called when e.g. installing exensions */
    void setLaunchDesc(LaunchDesc ld, boolean existOnCancel) {
        InformationDesc id = ld.getInformation();
        _title = id.getTitle();
        _vendor = id.getVendor();
        if (_titleLabel != null) {
	    _titleLabel.setText(_title);
	    _vendorLabel.setText(_vendor);
        }
        
	// Cancel button behavior
	_isCanceled = false;
	_exitOnCancel = existOnCancel;
	
        if (id != null) {
	    IconDesc icon = id.getIconLocation(InformationDesc.ICON_SIZE_LARGE,
					       IconDesc.ICON_KIND_DEFAULT);
	    if (icon != null) {
		CacheImageLoader.getInstance().loadImage(icon, this);
	    }
        }
        
    }
    // implementation of the two methods in CachedImageLoaderCallback:
    public void imageAvailable(IconDesc id, Image image, File file) {
	DownloadWindow.this.updateImage(image, true);
    }

    public void finalImageAvailable(IconDesc id, Image image, File file) {
    }

    
    /** Returns a reference to the frame */
    public JFrame getFrame() { return _frame; }
    
    /** setup and show accept screen */
    public void buildIntroScreen() {
        Insets  ins; 
        _frame = GeneralUtilities.createFrame(Resources.getString("appname"));
        _frame.setResizable(false);
        _frame.addWindowListener(this);
        
        // Add background widget
        // JPanel p = new BackgroundImagePanel(Resources.getIcon("launch.image"));
        JLabel p = new JLabel();
        p.setBorder(null);
        p.setLayout(null);
        Container parent = _frame.getContentPane();
        parent.setLayout(new BorderLayout()); // Fixed size widget
        parent.add(p, BorderLayout.CENTER);
        
        int width  = 455; // Window size
	int pad = 8;
        int height = 215 + 2 * pad;
	int boxStart = pad;
	int bottomStart = 162 + boxStart;
	parent.setBounds(0, 0, width, height);
	_frame.addNotify(); // alow us to get top level windows insets
	ins = _frame.getInsets();


	JLabel box = new JLabel();
	box.setBorder(new CompoundBorder(new LineBorder(Color.black),
					 new BevelBorder(BevelBorder.LOWERED)));
	box.setLayout(null);
	box.setBackground(Color.white);
	box.setOpaque(true);
	box.setBounds(5, boxStart, 433, 156);
	// Add box last since it is behinig
	p.add(box);
	
        Color headingColor = Color.black;
        Font headingFont   = new Font("SansSerif", Font.BOLD, 22);
        Font vendorFont    = new Font("SansSerif", Font.BOLD, 18);
        
        Color msgColor = Color.black;
        Font msgFont   = new Font("SansSerif", Font.PLAIN, 14);
        
        // App info
        _titleLabel = new JLabel(_title);
        _titleLabel.setVerticalTextPosition(SwingConstants.TOP);
        _titleLabel.setFont(headingFont);
        _titleLabel.setForeground(headingColor);
        _titleLabel.setOpaque(false);
        _titleLabel.setBounds(86, 14, width - _yRestriction - 86, 30);
        box.add(_titleLabel);
        
        _vendorLabel = new JLabel(_vendor);
        _vendorLabel.setVerticalTextPosition(SwingConstants.TOP);
        _vendorLabel.setFont(vendorFont);
        _vendorLabel.setForeground(headingColor);
        _vendorLabel.setOpaque(false);
        _vendorLabel.setBounds(86, 44, width - _yRestriction - 86, 30);
        box.add(_vendorLabel);
        
        // Cancel button
        _cancelButton = new JButton(Resources.getString
					("launch.cancel"));
        _cancelActionListener = new ActionListener() {
	    public void actionPerformed(ActionEvent ae) {
		cancelAction();
	    }
        };
        _cancelButton.addActionListener(_cancelActionListener);
        Dimension r = _cancelButton.getPreferredSize();
        _cancelButton.setBounds(366, bottomStart, r.width, r.height);
        p.add(_cancelButton);
        
        // Create other widgets
        _loadingModel = new DefaultBoundedRangeModel(0, 1, 0, 100);
        _infoStatus = new JLabel(Resources.getString("launch.initializing", _title, _vendor));
        _infoStatus.setBounds(10, 79, width - 10 - _yRestriction, 22);
        _infoStatus.setForeground(msgColor);
        _infoStatus.setOpaque(false);
        _infoStatus.setFont(msgFont);
        
        _infoProgressTxt = new JLabel(" ");
        _infoProgressTxt.setOpaque(false);
        _infoProgressTxt.setForeground(msgColor);
        _infoProgressTxt.setFont(msgFont);
        _infoProgressTxt.setBounds(10, 101, width - 10 - _yRestriction, 22);
        
        _infoEstimatedTime = new JLabel("");
        _infoEstimatedTime.setOpaque(false);
        _infoEstimatedTime.setForeground(msgColor);
        _infoEstimatedTime.setFont(msgFont);
        _infoEstimatedTime.setBounds(10, 123, width - 10 - _yRestriction, 22);
        
        _infoProgressBar = new JProgressBar(_loadingModel);
        _infoProgressBar.setBackground(Color.white);
        _infoProgressBar.setForeground(new Color(131, 171, 239));
        _infoProgressBar.setOpaque(true);
        _infoProgressBar.setBorderPainted(true);
        Dimension d = _infoProgressBar.getPreferredSize();
        _infoProgressBar.setBounds(5, bottomStart + 6, 350, (int)d.getHeight());
        
        box.add(_infoStatus);
        box.add(_infoProgressTxt);
        box.add(_infoEstimatedTime);
        p.add(_infoProgressBar);
	
        _infoProgressBar.setVisible(true);
        
	// Get default icon image
        _imageLabel = new JLabel();
        _imageLabel.setOpaque(false);
	_imageLabel.setBackground(Color.white);
        _imageLabel.setBounds(10, 10, 64, 64);
        updateImage(GeneralUtilities.getDefaultImage().getImage(), false);
        box.add(_imageLabel);
	
        // Apply any defaults the user may have, constraining to the size
        // of the screen, and default (packed) size.
        Rectangle size =  new Rectangle(0, 0, width, height);
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        size.width = Math.min(screenSize.width, size.width);
        size.height = Math.min(screenSize.height, size.height);
        // Center the window
        _frame.setBounds((screenSize.width - size.width) / 2,
			     (screenSize.height - size.height) / 2,
			 size.width, size.height);
        
        setIndeterminedProgressBar(true);
    }
    
    public void showLoadingProgressScreen() {
        setStatus(Resources.getString("launch.progressScreen"));
	
        // Setup timer to calculate download time
        _timerObject = new Timer(TIMER_UPDATE_RATE, this);
        _timerObject.start();
    }
    
    public void setStatus(final String text) {
        Runnable action = new Runnable() {
	    public void run() {
		// The window might get cleared before this is invoked. Everything is
		// synchronized on the event-dispatcher thread
		if (_infoStatus != null) {		  
		    _infoStatus.setVisible((text != null));
		    _infoStatus.setText(text);
		}
	    }
	};
	if (_infoStatus.isShowing()) {
            SwingUtilities.invokeLater(action);
	} else {
	    action.run();
	}
    }
    
    public void setProgressText(final String text) {
	SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			// The window might get cleared before this is invoked. Everything is
			// synchronized on the event-dispatcher thread
			if (_infoProgressTxt != null) {
			    _infoProgressTxt.setVisible((text != null));
			    _infoProgressTxt.setText(text);
			}
		    }
		});
    }
    
    
    /**
     * Sets the visibility of the progress bar.
     */
    public void setProgressBarVisible(final boolean isVisible) {
	SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			// The window might get cleared before this is invoked
			if (_infoProgressBar != null) {
			    _infoProgressBar.setVisible(isVisible);
			}
		    }
		});
    }
    
    public void setProgressBarValue(int value) {
	if (_heartbeatOn) {
	    setIndeterminedProgressBar(false);
	}
	_loadingModel.setValue(value);
	setProgressBarVisible(value != 0);
    }
    
    /** Changes the progressbar to show a heart-beat */
    void setIndeterminedProgressBar(boolean on) {
	// Make sure to create timer
	if (_heartbeatTimer == null) {
	    // Setup timer to calculate download time
	    // It is important not to synchronize on the _heartbeatTimer
	    // object, since it might lead to deadlocks.
	    _heartbeatTimer = new Timer(HEART_BEAT_RATE, new ActionListener() {
			public void actionPerformed(ActionEvent e) {
			    synchronized(_heartbeatLock) {
				if (_heartbeatOn && _heartbeatTimer != null) {
				    _heartbeatCount = (_heartbeatCount + 1) % HEART_BEAT_RYTHM.length;
				    boolean state = HEART_BEAT_RYTHM[_heartbeatCount];
				    if (state) {
					_loadingModel.setValue(100);
				    } else {
					_loadingModel.setValue(0);
				    }
				}
			    }
			}
		    });
	}
	synchronized(_heartbeatLock) {
	    if (on) {
		setProgressBarVisible(true);
		_loadingModel.setValue(0);
		_heartbeatTimer.start();
		_heartbeatOn = true;
	    } else {
		setProgressBarVisible(false);
		_heartbeatTimer.stop();
		_heartbeatOn = false;
	    }
	}
    }
    
    void showLaunchingApplication(final String title) {
	SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			// The window might get cleared before this is invoked. Everything is
			// synchronized on the event-dispatcher thread
			if (_loadingModel != null) {
			    _infoStatus.setText(title);
			    _infoProgressTxt.setVisible(false);
			    _infoEstimatedTime.setVisible(false);
			    _loadingModel.setValue(0);
			}
		    }
		});
    }
    
    private void setEstimatedTime(final String title) {
	SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			// The window might get cleared before this is invoked. Everything is
			// synchronized on the event-dispatcher thread
			if (_infoEstimatedTime != null) {
			    if (title != null) _infoEstimatedTime.setText(title);
			    _infoEstimatedTime.setVisible(title != null);
			}
		    }
		});
    }
    
    /** Removes all content from the window, but keeps the default
     *  windowClose listener. Make sure to serialize this on the event-dispatching
     *  thread. Using synchronized might deadlock.
     */
    void clearWindow() {
	if (SwingUtilities.isEventDispatchThread()) {
	    clearWindowHelper();
	} else {
	    try {		
		SwingUtilities.invokeAndWait(new Runnable() {
			    public void run() {
				clearWindowHelper();
			    }});
	    } catch(Exception e) {
		Debug.ignoredException(e);
	    }	    
	} 
    }
    
    private void clearWindowHelper() {	
	if (_timerObject != null) {
	    _timerObject.stop();
	    _timerObject = null;
	    _timerDownloadAverage = null;
	}
	if (_heartbeatTimer != null) {
	    synchronized (_heartbeatLock) {
		_heartbeatTimer.stop();
		_heartbeatTimer= null;
	    }
	}
	if (_frame != null) {
	    _infoStatus = null;
	    _infoProgressTxt = null;
	    _infoProgressBar = null;
	    _loadingModel = null;
	    _infoEstimatedTime = null;
	    _cancelButton.removeActionListener(_cancelActionListener);
	    _cancelButton = null;
	    _cancelActionListener = null;
	    _frame.getContentPane().removeAll();
	}	
    }
    
    
    /** Removes window from the screen */
    void disposeWindow() {
	if (_frame != null) {
	    clearWindow();
	    _frame.removeWindowListener(this);
	    _frame.setVisible(false);
	    _frame.dispose();
	    _frame = null;
	}
    }
    
    /** Resets the status's displayed in the window to empty. */
    void reset() {
	setStatus(null);
	setProgressText(null);
	setProgressBarVisible(false);
    }
    
    /** This event is called by the timer */
    public void actionPerformed(ActionEvent e) {
	// Check if timer should be shown
	if (!_timerOn) return;
	if (_estimatedDownloadSize <= 0) return;
	
	// Calculate delta and put it into the sliding average array.
	long delta = _totalDownloadedBytes - _timerLastBytesCount;
	_timerLastBytesCount = _totalDownloadedBytes;
	_timerDownloadAverage[_timerCount % TIMER_AVERAGE_SIZE] = delta;
	
	// Adjust estimate if it was to low
	if (_totalDownloadedBytes > _estimatedDownloadSize) _estimatedDownloadSize = _totalDownloadedBytes;
	
	// Check if we should update the text
	if (_timerCount > TIMER_INITIAL_DELAY) {
	    // Calculate current average
	    float average = 0;
	    for(int i = 0; i < TIMER_AVERAGE_SIZE; i++) average += _timerDownloadAverage[i];
	    average /= TIMER_AVERAGE_SIZE; // Average bytes per tick
	    average /= (((float)TIMER_UPDATE_RATE) / 1000); // Average bytes/s
	    
	    if (average == 0) {
		// Stalled download
		setEstimatedTime(Resources.getString("launch.stalledDownload"));
	    } else if (_estimatedDownloadSize > 0) {
		int totalSecsLeft = (int)((_estimatedDownloadSize - _totalDownloadedBytes) / average);
		int hours = totalSecsLeft / 3600;
		totalSecsLeft -= (hours * 3600);
		int mins  = totalSecsLeft  / 60;
		totalSecsLeft -= (mins * 60);
		int secs  = totalSecsLeft;
		
		setEstimatedTime(Resources.getString("launch.estimatedTimeLeft", hours, mins, secs));
	    }
	}
	_timerCount++;
    }
    
    /* Resets the download time estimation */
    public void resetDownloadTimer() {
	_timerCount = 0;
	_timerLastBytesCount = 0;	
    }
    
    // -----------------
    
    public void progress(URL rc, String version, long readSoFar, long totalSize, int percent) {
	// Turn timer on and update download values
	_timerOn = true;
	_totalDownloadedBytes = Math.max(0,readSoFar);	// 4485537:  may be -1
	_estimatedDownloadSize = totalSize;
	
	// Update text fields if neccesary
	if (rc != _currentUrl && rc != null) {
	    // New Jar file
	    String host = rc.getHost();
	    String filename = rc.getFile();
	    int idx = filename.lastIndexOf('/');
	    if (idx != -1) filename = filename.substring(idx + 1);

	    if ((filename.length() + host.length()) > (2 * MAX_DISPLAY)) {
		filename = maxDisplay(filename);
		host = maxDisplay(host);
	    }

	    setStatus(Resources.getString("launch.loadingNetStatus", filename, host));
	    _currentUrl = rc;
	}
	
	if (totalSize == -1) {
	    setProgressText(Resources.getString("launch.loadingNetProgress",
						bytesToString(_totalDownloadedBytes)));
	} else {
	    setProgressText(Resources.getString("launch.loadingNetProgressPercent",
						bytesToString(_totalDownloadedBytes),
						bytesToString(totalSize),
						new Long(Math.max(0,percent)).toString()));
	    
	    setProgressBarValue(percent);
	}
    }
    
    /**
     * Invoked while an old version is being patched to a new version.
     */
    public void patching(URL rc, String version, int patchPercent,
			 int percent) {
	// Turn timer off
	_timerOn = false;
	setEstimatedTime(null); // hide
	
	if (_currentUrl != rc || patchPercent == 0) {
	    String host = rc.getHost();
	    String filename = rc.getFile();
	    int idx = filename.lastIndexOf('/');
	    if (idx != -1) filename = filename.substring(idx + 1);

	    if ((filename.length() + host.length()) > (2 * MAX_DISPLAY)) {
		filename = maxDisplay(filename);
		host = maxDisplay(host);
	    }
	    
	    setStatus(Resources.getString
			  ("launch.patchingStatus", filename, host));
	    _currentUrl  = rc;
	}
	setProgressText(null);
	setProgressBarValue(percent);
    }
    
    // fix for 4724977
    // only allows hostname and filename to be length of 20
    private String maxDisplay(String text) {

	int flen = text.length();

	if (flen > MAX_DISPLAY) {

	    text = LEAD + text.substring(flen - (MAX_DISPLAY - LEAD.length()) , flen);
	}

	return text;
    }

    /**
     * This is invoked when resource identified by <code>rc</code> is being
     * verified, i.e., scanned for certificates and checked that potential signing
     * is coherent
     */
    public void validating(URL rc, String version, long count, long total, int percent) {
	// Turn timer off
	_timerOn = false;
	setEstimatedTime(null); // hide
	
	// Update display
	long percentValidating = (total == 0) ? 0 : (count * 100) / total;
	
	if (_currentUrl != rc || count == 0) {
	    String host = rc.getHost();
	    String filename = rc.getFile();
	    int idx = filename.lastIndexOf('/');
	    if (idx != -1) filename = filename.substring(idx + 1);

	    if ((filename.length() + host.length()) > (2 * MAX_DISPLAY)) {
		filename = maxDisplay(filename);
		host = maxDisplay(host);
	    }

	    setStatus(Resources.getString("launch.validatingStatus", filename, host));
	    _currentUrl  = rc;
	}
	if (count != 0) {
	    setProgressText(Resources.getString("launch.validatingProgress", (int)percentValidating));
	} else {
	    setProgressText(null);
	}
	setProgressBarValue(percent);
    }
    
    
    /**
     * This is invoked when resource identified by <code>rc</code> failed to be
     * loaded. The loadingFromNet will always been called before this.
     */
    public void downloadFailed(URL url, String version) {
	// Turn timer off
	_timerOn = false;
	setEstimatedTime(null); // hide
	// Update display
	setStatus(Resources.getString("launch.loadingResourceFailedSts", url.toString()));
	setProgressText(Resources.getString("launch.loadingResourceFailed"));
	setProgressBarVisible(false);
    }
    
    /**
     * This is invoked when extension-descs are downloaded
     */
    public void extensionDownload(String name, int remaining) {
	// Turn timer off
	_timerOn = false;
	setEstimatedTime(null); // hide
	// Update display
	if (name != null) {
	    setStatus(Resources.getString("launch.extensiondownload-name", name, remaining));
	} else {
	    setStatus(Resources.getString("launch.extensiondownload", name, remaining));
	}
    }
    
    /**
     * This is invoked when a JRE is downloaded
     */
    public void jreDownload(String version, URL location) {
	// Turn timer off
	_timerOn = false;
	setEstimatedTime(null); // hide
	// Update display

	String host = location.getHost();

	host = maxDisplay(host);

	setStatus(Resources.getString("launch.downloadingJRE", version, host));
    }
    
    //----------------------
    
    /**
     * This is invoked as a resource, indicated by <code>owner</code>, is
     * downloaded from the network, i.e., not found in the cache.
     */
    private void loadingFromNet(URL rc, int readSoFar, int estimatedSize) {
	
    }
    
    /**
     * Forces the app image to dispaly <code>image</code>.
     */
    private void setAppImage(Image image) {
	updateImage(image, true);
    }
    
    private void updateImage(Image image, boolean forceUpdate) {
	ImageIcon ii;
	
	if (image != null) {
	  int w = image.getWidth(null);
 	  int h = image.getHeight(null);
	  if ((w != 64) || (h != 64)) {

	    int width = 64;
	    // if original image is slightly taller than wide, 
	    // try to preserve it's aspect ratio.
	    if ((h > w) && (h < 2*w)) {
		width = (64 * w) / h;
	    }

	    Image sImage = new BufferedImage(64, 64, BufferedImage.
						 TYPE_INT_RGB);
	    Graphics g = sImage.getGraphics();
	    
	    try {
		g.setColor(Color.white);
		g.fillRect(0, 0, 64, 64);
		g.drawImage(image, (64 - width)/2, 0, width, 64, null);
	    }
	    finally {
		g.dispose();
	    }
	    image = sImage;
	  }
	}
	synchronized(this) {
	    if (_appImage == null || forceUpdate) {
		_appImage = image;
	    }
	}
	if (_imageLabel != null) {
	    if (_appImage != null) {
		_imageLabel.setIcon(new ImageIcon(_appImage));
		_imageLabel.setOpaque(true);
	    }
	    else {
		_imageLabel.setIcon(null);
		_imageLabel.setOpaque(false);
	    }
	    _imageLabel.repaint();
	}
    }
    
    /** Converts a bytes to a nice string, using K, M, or G as appropriate */
    private String bytesToString(long bytes) {
	final int K = 1024;
	final int M = K * K;
	final int G = K * K * K;
	
	String kind = "";
	double value = bytes;
	int digits = 0;
	if (bytes > G) {
	    value /= G;
	    kind = "G";
	    digits = 1;
	} else if (bytes > M) {
	    value /= M;
	    kind = "M";
	    digits = 1;
	} else if (bytes > K) {
	    value /= K;
	    kind = "K";
	    digits = 0;
	}
	return Resources.formatDouble(value, digits) + kind;
    }
    
    /**
     * WindowListener method. Indicates user doesn't want the app to
     * be launched.
     */
    public void windowClosing(WindowEvent we) {
	cancelAction();
    }
    
    private void cancelAction() {
	if (_exitOnCancel) {
	    // This might be executed with a security manager turned on
	    AccessController.doPrivileged(new PrivilegedAction() {
			public Object run() {
			    Main.systemExit(-1); 
			    return null; // Nothing to return
			}
		    });
	} else {
	    _isCanceled = true;	    
	}
    }
    
    /* Use by the DownloadService impl. */
    public boolean isCanceled() { return _isCanceled; }
    
    /* Resets all arguments used in window for a second download. This is mostly for
     * the estimated time and cancel state. */
    public void resetCancled() { 
	_isCanceled = false; 	
    }
}



