/*
 * @(#)AwtPrintControl.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.print;

import sun.awt.windows.WPrintDialog;

import sun.awt.print.PrintControl;
import sun.awt.print.PrinterCapabilities;
import sun.awt.print.PrinterListUpdatable;
import java.awt.FileDialog;
import java.awt.Frame;
import java.awt.Frame;
import java.awt.PrintJob;
import java.awt.JobAttributes;
import java.awt.JobAttributes.DestinationType;
import java.awt.PageAttributes;
import java.awt.PageAttributes.MediaType;
import java.io.File;
import java.util.Iterator;
import java.util.TreeSet;
import java.util.HashMap;
import java.util.Properties;

public class AwtPrintControl extends PrintControl {
    static {
        initIDs();
    }

    private static final MediaType[] dmPaperToPrintControl = {
      MediaType.NA_LETTER, MediaType.NA_LETTER, MediaType.LEDGER,
      MediaType.LEDGER, MediaType.NA_LEGAL, MediaType.INVOICE,
      MediaType.EXECUTIVE, MediaType.ISO_A3, MediaType.ISO_A4,
      MediaType.ISO_A4, MediaType.ISO_A5, MediaType.ISO_B4,
      MediaType.JIS_B5, MediaType.FOLIO, MediaType.QUARTO,
      MediaType.NA_10X14_ENVELOPE, MediaType.B, MediaType.NA_LETTER,
      MediaType.NA_NUMBER_9_ENVELOPE, MediaType.NA_NUMBER_10_ENVELOPE,
      MediaType.NA_NUMBER_11_ENVELOPE, MediaType.NA_NUMBER_12_ENVELOPE,
      MediaType.NA_NUMBER_14_ENVELOPE, MediaType.C, MediaType.D,
      MediaType.E, MediaType.ISO_DESIGNATED_LONG, MediaType.ISO_C5,
      MediaType.ISO_C3, MediaType.ISO_C4, MediaType.ISO_C6,
      MediaType.ITALY_ENVELOPE, MediaType.ISO_B4, MediaType.ISO_B5,
      MediaType.ISO_B6, MediaType.ITALY_ENVELOPE,
      MediaType.MONARCH_ENVELOPE, MediaType.PERSONAL_ENVELOPE,
      MediaType.NA_10X15_ENVELOPE, MediaType.NA_9X12_ENVELOPE,
      MediaType.FOLIO
    };

    private java.awt.peer.ComponentPeer dialogOwnerPeer = null;

    public AwtPrintControl(Frame dialogOwner, String doctitle,
			   Properties props) {
	super(dialogOwner, doctitle, null, null);
    }

    public AwtPrintControl(Frame dialogOwner, String doctitle,
			 JobAttributes jobAttributes,
			 PageAttributes pageAttributes) {
        super(dialogOwner, doctitle, jobAttributes, pageAttributes);
        if (dialogOwner != null) {
            dialogOwnerPeer = (java.awt.peer.ComponentPeer)dialogOwner.getPeer();
        }

    }

    private static final double TENTHS_MM_TO_POINTS = 3.527777778;
    private static final HashMap mediaCache = new HashMap();
    private static final HashMap capabilitiesCache = new HashMap();
    private class CacheKey {
        int dmIndex, width, length;
        CacheKey(int dmIndex, int width, int length) {
	    this.dmIndex = dmIndex;
	    this.width = width;
	    this.length = length;
	}
        public int hashCode() {
	    return dmIndex;
	}
        public boolean equals(Object obj) {
	    CacheKey key = (CacheKey)obj;
	    return (dmIndex == key.dmIndex &&
	           ((width == key.width && length == key.length) ||
		    (width == key.length && length == key.width)));
	}
    }

    private native char _getWin32MediaAttrib(int pcIndex, String printer);
    public char getWin32MediaAttrib() {
        return _getWin32MediaAttrib(getMediaAttrib(), getPrinterAttrib());
    }

    private int findWin32Media(int dmIndex, int width, int length,
			       boolean exactOnly) {
        if (dmIndex >= 1 && dmIndex <= 41) {
	    // The sizes between 1 and 41, inclusive (DMPAPER_LETTER to
	    // DMPAPER_FANFOLD_LGL_GERMAN in wingdi.h) are well defined
	    // and generally supported by all printer drivers. We can use
	    // a static lookup table for those mappings.
	    //
	    // Note that we don't have exact mappings for DMPAPER_ENV_C65 (32)
	    // DMPAPER_FANFOLD_US (39) and DMPAPER_FANFOLD_STD_GERMAN (40)
	    if (exactOnly && (dmIndex == 32 || dmIndex == 39 ||
			      dmIndex == 40)) {
	        return -1;
	    } else {
	        return dmPaperToPrintControl[dmIndex - 1].hashCode();
	    }
	} else {
	    // The sizes between 42 and 68, inclusive (DMPAPER_ISO_B4 to
	    // DMPAPER_A3_EXTRA_TRANSVERSE in wingdi.h), while supposedly
	    // standardized as of Win95 and WinNT 4.0, are printer driver
	    // dependent. Based on the width and height, we'll try to find
	    // a close match.

	    Integer cached =
	        (Integer)mediaCache.get(new CacheKey(dmIndex, width, length));
	    if (cached != null) {
	        int i = cached.intValue();
	        if (exactOnly) {
		    // verify this is an exact match
		    int psWidth = (int)(width / TENTHS_MM_TO_POINTS + .5);
		    int psLength = (int)(length / TENTHS_MM_TO_POINTS + .5);
		    if ((psWidth == PrintControl.WIDTHS[i] &&
			 psLength == PrintControl.LENGTHS[i]) ||
			(psWidth == PrintControl.LENGTHS[i] &&
			 psLength == PrintControl.WIDTHS[i])) {
		        return i;
		    } else {
		        return -1;
		    }
		} else {
		    return i;
		}
	    }

	    int psWidth = (int)(width / TENTHS_MM_TO_POINTS + .5);
	    int psLength = (int)(length / TENTHS_MM_TO_POINTS + .5);

	    int pError, lError;
	    int bestError = Integer.MAX_VALUE;
	    int pcIndex = -1;

	    for (int i = 0; i < PrintControl.SIZES.length; i++) {
	        pError = Math.abs(psWidth - PrintControl.WIDTHS[i]) +
		         Math.abs(psLength - PrintControl.LENGTHS[i]);
		
		// try rotated orientation as well
	        lError = Math.abs(psWidth - PrintControl.LENGTHS[i]) +
		         Math.abs(psLength - PrintControl.WIDTHS[i]);

		pError = (pError < lError) ? pError : lError;

		if (pError < bestError) {
		    bestError = pError;
		    pcIndex = i;
		    if (bestError == 0) {
		        // found an exact match
		        break;
		    }
		}
	    }

	    mediaCache.put(new CacheKey(dmIndex, width, length),
			   new Integer(pcIndex));
	    if (!exactOnly || bestError == 0) {
		return pcIndex;
	    }
	}
	
	return -1;
    }

    public void setWin32MediaAttrib(int dmIndex, int width, int length) {
        setMediaAttrib(findWin32Media(dmIndex, width, length, false));
    }

    public native String getDefaultPrinterName();
    private native boolean _getCapabilities(PrinterCapabilities capabilities);
    public boolean getCapabilities(PrinterCapabilities capabilities) {
        PrinterCapabilities cached = (PrinterCapabilities)
	    capabilitiesCache.get(capabilities.getPrinterName());
	if (cached != null) {
	    capabilities.set(cached);
	    return true;
	}

        if (!_getCapabilities(capabilities)) {
	    return false;
	}

	int[] sizes = capabilities.getSizes();

	if (sizes != null) {
	    TreeSet set = new TreeSet();

	    for (int i = 0; i < sizes.length; i++) {
	        if (sizes[i] != -1) {
		    set.add(new Integer(sizes[i]));
		}
	    }
	    int[] sortedSizes = new int[set.size()];
	    Iterator itor = set.iterator();
	    for (int i = 0; itor.hasNext(); i++) {
	        sortedSizes[i] = ((Integer)itor.next()).intValue();
	    }
	    capabilities.setSizes(sortedSizes);
	}

	capabilitiesCache.put(capabilities.getPrinterName(), capabilities);
	return true;
    }
    private native String[] _getPrinterList();
    public void getPrinterList(final PrinterListUpdatable updatable) {
        new Thread(new Runnable() {
	    public void run() {
	        String[] list = _getPrinterList();

		if (list != null) {
		    TreeSet set = new TreeSet();
	        
		    for (int i = 0; i < list.length; i++) {
		        set.add(list[i]);
		    }
		    Iterator itor = set.iterator();
		    for (int i = 0; i < list.length; i++) {
		        list[i] = (String)itor.next();
		    }
		}

		updatable.updatePrinterList(list);
	    }
	}).start();
    }

    protected boolean displayNativeDialog() {
	WPrintDialog dialog = new WPrintDialog(dialogOwner, this);
	dialog.setRetVal(false);
        dialog.show();
	boolean prv = dialog.getRetVal();
	if (getDestAttrib() == DestinationType.PRINTER || prv == false) {
	    return prv;
	} else {
	    FileDialog fileDialog =
		new FileDialog(dialogOwner, "", FileDialog.SAVE);
            String pathName = getFileNameAttrib();
            if (pathName != null) {
               File file = new File(pathName);
               fileDialog.setFile(file.getName());
               File parent = file.getParentFile();
               if (parent != null) {
                   fileDialog.setDirectory(parent.getPath());
               }
            } else {  
	        fileDialog.setFile("out.prn");
            }
	    fileDialog.show();
	    String fileName = fileDialog.getFile();
	    if (fileName == null) {
		return false;
	    } else {
		String dirName = fileDialog.getDirectory();
                if (dirName != null) {
                    fileName = dirName+File.separator+fileName;
                }
		setFileNameAttrib(fileName);
		return true;
	    }
	}
    }

    /**
     * Initialize JNI field and method ids
     */
    private static native void initIDs();
}
