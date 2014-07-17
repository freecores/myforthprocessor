/*
 * @(#)HTMLEntryManager.java	1.7 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.ui.player;

import com.sun.javaws.*;
import com.sun.javaws.debug.*;
import com.sun.javaws.jnl.*;
import java.io.*;
import java.net.*;
import java.util.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.text.html.*;
import javax.swing.text.html.parser.*;
import com.sun.javaws.cache.InstallCache;
import com.sun.javaws.net.*;

/**
 * An EntryManager that will obtain its list of JNLEntries from a web
 * page. The web page can be specified in the constructor, and defaults
 * to http://ignoramus.eng.sun.com/jump/index.html. The web page is
 * parsed and each ANCHOR (A) entry with a href that ends with .jnlp
 * is considered a JNL file and is parsed. If parsing is successful it
 * is loaded to the list of JNL entries. As the parsing may take a while
 * it is done on a separate thread. <code>refresh</code> will return
 * immediately, and spawn a thread (if parsing isn't currently happening)
 * to do the parsing. The parser will then spawn threads to handle parsing
 * the jnl files (the max number of threads is currently 5). As a JNL
 * file is successfully parsed, listeners are notified.
 *
 * @version 1.20 02/14/01
 */
public class HTMLEntryManager extends EntryManager {
    /**
     * This list of JNLEntries obtained from the cache.
     */
    private ArrayList _entries;
    /**
     * The URL to parse.
     */
    private URL _url;
    /**
     * Set to true while parsing in <code>refresh</code>.
     */
    private boolean _loading;
    /**
     * Handles the actual parsing of the html file.
     */
    private CallbackHandler _handler;
    
    /**
     * Creates an HTMLEntryManager with a default URL
     * (http://ignoramus.eng.sun.com/jump/index.html).
     */
    public HTMLEntryManager() {
        _entries = new ArrayList();
        try {
            String urlString = ConfigProperties.getInstance().getRemoteURL();
            if (urlString != null) {
                _url = new URL(urlString);
            }
        } catch (MalformedURLException murle) {}
    }
    
    URL getURL() {
	return _url;
    }

    /**
     * Creates an HTMLEntryManager with a specific URL.
     */
    public HTMLEntryManager(URL url) {
        this();
        _url = url;
    }
    
    /**
     * If currently parsing, this does nothing, otherwise this will spawn
     * a thread to handle the parsing and return immediately.
     */
    public void refresh() {
        boolean startThread = false;
        
        if (_url == null) {
            return;
        }
        synchronized(this) {
            if (!_loading) {
                _loading = true;
                startThread = true;
            }
        }
        if (startThread) {
            _entries.clear();
            new Thread() {
                public void run() {
                    parseURL();
                }
            }.start();
        }
    }
    
    /**
     * Returns the JNLEntry at the specified index.
     */
    public JNLEntry get(int index) {
        return (JNLEntry)_entries.get(index);
    }
    
    /**
     * Returns the number of JNLEntries.
     */
    public int size() {
        return _entries.size();
    }
    
    /**
     * Returns the current state of the EntryManager, either
     * <code>LOADED</code> or <code>LOADING</code>. While there is
     * no setter, this is a bound property.
     */
    public int getState() {
        return _loading ? 1 : 0;
    }

    void stop() {
	if (_handler != null) {
	    _handler.stop();
	}
    }
    
    /**
     * Starts the process of parsing the web page. The parsing is
     * handled by the swing html parser with <code>CallbackHandler</code>
     * as the delegate.
     */
    private void parseURL() {
        InputStream is = null;
        if (Globals.TraceHTMLEntryParsing) {
            Debug.println("starting jnl file loading from: " + _url);
        }
        _handler = new CallbackHandler();

	if (_url.toString().endsWith(".jnlp")) {
	    _handler.scheduleParsingOfJNLFile(_url);
	} else {

	    try {
		HttpRequest httpreq = JavawsFactory.getHttpRequestImpl();
		HttpResponse response = httpreq.doGetRequest(_url);
		is = response.getInputStream();
		if (is != null) {
		    Reader reader = new InputStreamReader(is);
		    ParserDelegator parser = new ParserDelegator();
		    parser.parse(reader, _handler, true);
		}
	    } catch (IOException ioe) {
		if (Globals.TraceHTMLEntryParsing) {
		    Debug.println("IOException parsing html entries " + ioe);
		}
	    }
	}
        if (is != null) {
            try {
                is.close();
            } catch (IOException ioe) {}
        }
        _handler.notifyWhenFinishedLoadingJNLFiles();
    }
    
    /**
     * This is invoked from the parser as a new entry is found. As this
     * can come in on any thread, <code>SwingUtilities.invokeLater</code>
     * is used before the entry is added.
     */
    void addEntry(final JNLEntry entry) {
        SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
			if (!_entries.contains(entry)) {
                            _entries.add(entry);
        		    JNLEntry selEntry = getSelectedEntry();
			    Collections.sort(_entries);
                            fireStateChanged();
			    if (selEntry != null) {
			        setSelectedIndex(_entries.indexOf(selEntry));
			    } else if (getSelectedIndex() == -1 && size() > 0) {
                                setSelectedIndex(0);
                            }
                        }
		    }
                });
    }
    
    /**
     * Invoked when the parser has finished parsing all the JNL files from
     * the web page. This will update the loading state and fire a property
     * change on the event dispatching thread.
     */
    void finishedLoadingJNLFiles() {
        if (Globals.TraceHTMLEntryParsing) {
            Debug.println("Finished loading remote JNL files");
        }
        synchronized(this) {
            _loading = false;
        }
        SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        firePropertyChange("state", new Integer(1), new Integer(0));
                    }
                });
    }
    
    
    /**
     * Maximum number of threads that can be spawned to handle loading
     * of the JNL file URLs.
     */
    private static final int MAX_T_COUNT = 5;
    
    
    /**
     * Delegate of the html parser. When an ANCHOR (A) with an href
     * that ends in .jnlp is encountered and URL is added to a list of
     * JNL files to be parsed. Another thread is spawned to parse the
     * JNL file (with a max of MAX_T_COUNT threads running at any given
     * time) to parse the JNL file. If successful, <code>addEntry</code>
     * is invoked to add the new JNLEntry.
     * <p>
     * When all web page has and all the potential URLs have been parsed
     * <code>finishedLoadingJNLFiles</code> will be invoked.
     */
    private class CallbackHandler extends HTMLEditorKit.ParserCallback
        implements Runnable {
        /** List of URLs to parse. */
        private ArrayList _urls;
        /** If this is true, it indicates parsing should stop. */
        private boolean _bail;
        /** Number of threads currently active. */
        private int _threadCount;
        /** The document base. */
        private URL _baseURL;
        
        public CallbackHandler() {
            _urls = new ArrayList();
        }
        
        /**
         * This can be invoked to stop the current processing. This will
         * only stop future processing of JNL URLs, not the actual URL
         * itself.
         */
        public void stop() {
            _bail = true;
        }
        
        /**
         * This will do one of two things. Immediately invoke
         * <code>finishedLoadingJNLFiles</code> if there are no threads
         * currently running, or spawn a thread and wait until there
         * are no threads running (all the potential URLs have been parsed)
         * and then invoke <code>finishedLoadingJNLFiles</code>.
         */
        public void notifyWhenFinishedLoadingJNLFiles() {
            boolean done;
            synchronized(CallbackHandler.this) {
                done = (_threadCount == 0);
            }
            if (done) {
                finishedLoadingJNLFiles();
            }
            else {
                new Thread() {
                    public void run() {
                        boolean stop = false;
                        while (!stop) {
                            synchronized(CallbackHandler.this) {
                                stop = (_threadCount == 0);
                                if (!stop) {
                                    try {
                                        CallbackHandler.this.wait();
                                    } catch (InterruptedException ie) {}
                                }
                            }
                        }
                        finishedLoadingJNLFiles();
                    }
                }.start();
            }
        }
        
        /**
         * Obtains the next URL to parse from <code>_urls</code> and
         * invokes <code>loadJNLFile</code>. This will exit when
         * <code>_urls</code> is empty (that is there are no more URLs
         * to parse).
         * <p>
         * This shouldn't be directly invoked. It is a public by way of
         * implementing the Runnable interface.
         */
        public void run() {
            boolean done = _bail;
            while (!done) {
                URL url = null;
                int tCount = 0;
                synchronized(CallbackHandler.this) {
                    if (_urls.size() == 0) {
                        done = true;
                        tCount = --_threadCount;
                        notifyAll();
                    }
                    else {
                        url = (URL)_urls.remove(0);
                    }
                }
                if (!done) {
                    loadJNLFile(url);
                    done = _bail;
                }
                else if (Globals.TraceHTMLEntryParsing) {
                    Debug.println("Thread stopping, remaining threads: " +
                                      tCount);
                }
            }
        }
        
        /**
         * Schedules the URL <code>url</code> denoting a JNL file for parsing.
         * If there are less than <code>MAX_T_COUNT</code> threads running,
         * this will start a new thread, otherwise it will just add it to
         * the list of URLs to parse and one of the active threads will then
         * parse it later.
         */
        private void scheduleParsingOfJNLFile(URL url) {
            boolean start = false;
            
            if (Globals.TraceHTMLEntryParsing) {
                Debug.println("Scheduling URL: " + url);
            }
            synchronized(CallbackHandler.this) {
                _urls.add(url);
                start = _threadCount < MAX_T_COUNT;
                if (start) {
                    _threadCount++;
                }
            }
            if (start) {
                new Thread(this).start();
                if (Globals.TraceHTMLEntryParsing) {
                    Debug.println("started thread new thread to parse");
                }
            }
        }
        
        /**
         * Does the actually loading of the JNL file. If successful this
         * will invoke <code>addEntry</code> to handle adding the newly
         * created JNLEntry.
         */
        private void loadJNLFile(URL url) {
            if (Globals.TraceHTMLEntryParsing) {
                Debug.println("Loading JNL file from: " + url);
            }
            try {
                LaunchDesc ld = LaunchDescFactory.buildDescriptor(url);
                // Ignore those without a home as we won't be able to
                // lookup the properties file in the cache.
                if (ld != null && ld.getCanonicalHome() != null &&
                        (ld.getLaunchType() == LaunchDesc.APPLICATION_DESC_TYPE ||
                             ld.getLaunchType() == LaunchDesc.APPLET_DESC_TYPE)) {
                    InformationDesc id = ld.getInformation();
                    LocalApplicationProperties lap = InstallCache.getCache().
                        getLocalApplicationProperties
                        (ld.getCanonicalHome(), ld);
                    if (id != null && lap != null) {
                        JNLEntry je = new JNLEntry(ld, id, lap, null, 0l,
                                                   false, url);
                        if (Globals.TraceHTMLEntryParsing) {
                            Debug.println("parsed: " + je);
                        }
                        addEntry(je);
                    }
                }
            } catch (Exception e) {
                // Assume its invalid
                if (Globals.TraceHTMLEntryParsing) {
                    Debug.println("Error loading JNL file from " + url +
                                      " error " + e);
                    e.printStackTrace();
                }
                
            }
        }
        
        //
        // HTMLEditorKit.ParserCallback methods
        //
        
        /**
         * Invoked at the end of the stream. We do nothing as we don't
         * really cache anything.
         */
        public void flush() throws BadLocationException {
        }
        
        /**
         * Invoked when text in the html document is encountered.
         */
        public void handleText(char[] data, int pos) {
        }
        
        /**
         * Invoked when a comment is encountered. Since we don't care about
         * comments, this does nothing.
         */
        public void handleComment(char[] data, int pos) {
        }
        
        /**
         * Invoked when a start tag is encountered. If the tag is an anchor,
         * and the href ends with .jnlp we consider it an entry and try to
         * parse it.
         */
        public void handleStartTag(HTML.Tag t, MutableAttributeSet a,
                                   int pos) {
            if (t == HTML.Tag.BASE) {
                String urlText = (String)a.getAttribute(HTML.Attribute.HREF);
                if (urlText != null) {
                    try {
                        _baseURL = new URL(urlText);
                    } catch (MalformedURLException murle) {
                    }
                }
            }
            if (t == HTML.Tag.A) {
                // URL
                String urlText = (String)a.getAttribute(HTML.Attribute.HREF);
                if (urlText != null && urlText.endsWith(".jnlp")) {
                    URL url = null;
                    try {
                        // Test absolute first
                        url = new URL(urlText);
                    } catch (MalformedURLException murle) {
                        // Then relative to the base
                        URL base = (_baseURL != null) ? _baseURL : _url;
                        if (base != null) {
                            try {
                                url = new URL(base, urlText);
                            } catch (MalformedURLException murle2) {}
                        }
                    }
                    if (url != null) {
                        scheduleParsingOfJNLFile(url);
                    }
                }
            }
        }
        
        /**
         * Invoked when the end of a tag is encountered. We don't caret
         * about this, so we do nothing.
         */
        public void handleEndTag(HTML.Tag t, int pos) {
        }
        
        /**
         * Invoked when unknown (invalid html) tags are encountered. We don't
         * care, so we do nothing.
         */
        public void handleSimpleTag(HTML.Tag t, MutableAttributeSet a,
                                    int pos) {
        }
        
        /**
         * Invoked in the event of an error. We don't care, so we do nothing.
         */
        public void handleError(String errorMsg, int pos){
        }
        
    }
}
