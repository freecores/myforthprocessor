/*
 * @(#)PrintServiceImpl.java	1.16 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import javax.jnlp.PrintService;
import java.awt.print.Pageable;
import java.awt.print.PageFormat;
import java.awt.print.Printable;
import java.awt.print.PrinterJob;
import java.awt.print.PrinterException;
import com.sun.javaws.Resources;
import java.security.AccessController;
import java.security.PrivilegedAction;
import com.sun.javaws.debug.Debug;

public final class PrintServiceImpl implements PrintService {
    static private PrintServiceImpl _sharedInstance = null;
    
    private PrinterJob _sysPrinterJob;
    private SmartSecurityDialog _securityDialog = null;
    private PageFormat _pageFormat = null;
    
    /* This is run with all priviledges */
    private PrintServiceImpl() {
	_securityDialog = new SmartSecurityDialog(Resources.getString("APIImpl.print.message"));
	_sysPrinterJob = PrinterJob.getPrinterJob();
    }
    
    /* This is run with all priviledges */
    public static synchronized PrintServiceImpl getInstance() {
        if (_sharedInstance == null) {
	    _sharedInstance = new PrintServiceImpl();
        }
        return _sharedInstance;
    }
    
    // sets up printer job (there's no access to printer itself, so no
    // security hazard and therefore no need for security dialog popup):
    public PageFormat getDefaultPage() {
	if (_sysPrinterJob != null) {
	    return (PageFormat)AccessController.doPrivileged(
		new PrivilegedAction() {
		    public Object run() {
			return _sysPrinterJob.defaultPage();
		    }
		});
	}
	return null;
    }
    
    // sets up printer job (there's no access to printer itself, so no
    // security hazard and therefore no need for security dialog popup):
    public PageFormat showPageFormatDialog(final PageFormat page) {
	if (_sysPrinterJob != null) {
	    return (PageFormat)AccessController.doPrivileged(
		new PrivilegedAction() {
		    public Object run() {
			// Make sure to remember pageformat
			_pageFormat = _sysPrinterJob.pageDialog(page);
			return _pageFormat;
		    }
		});
	}
	return null;
    }
    
    // print if permission granted, i.e., if askUser() returns true:
    public synchronized boolean print(final Pageable document) {
	return doPrinting(null, document);
    }
    
    // print if permission granted, i.e., if askUser() returns true:
    public synchronized boolean print(final Printable painter) {
	return doPrinting(painter, null);
    }
    
    
    /** This is either called with a painter or a document */
    private boolean doPrinting(final Printable painter, final Pageable document) {
	if (!askUser()) return false;
	
	Boolean result = (Boolean)AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			// Specify document to print
			if (document != null) {
			    _sysPrinterJob.setPageable(document);
			} else {
			    if (_pageFormat == null ) {
				_sysPrinterJob.setPrintable(painter);
			    } else {
				_sysPrinterJob.setPrintable(painter, _pageFormat);
			    }
			}
			// Show printing dialog
			if (_sysPrinterJob.printDialog()) {
			    // Starts a thread in the background to do print on to avoid blocking
			    Thread t = new Thread(new Runnable() {
					public void run() {
					    try {
						_sysPrinterJob.print();
					    } catch (PrinterException pe) {
						Debug.ignoredException(pe);
					    }
					}
				    });
			    t.start();
			    return Boolean.TRUE;
			} else {
			    return Boolean.FALSE;
			}
		    }
		});
	return result.booleanValue();
    }
    
    private synchronized boolean askUser() {
	// if user already grants unrestricted access, return true:
	if (CheckServicePermission.hasPrintAccessPermissions()) return true;
	// Show security dialog
	return _securityDialog.showDialog();
    }
}

