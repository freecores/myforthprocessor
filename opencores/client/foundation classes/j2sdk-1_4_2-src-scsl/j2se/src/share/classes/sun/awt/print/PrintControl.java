/*
 * @(#)PrintControl.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.print;

import java.awt.Frame;
import java.awt.JobAttributes;
import java.awt.JobAttributes.*;
import java.awt.PageAttributes;
import java.awt.PageAttributes.*;
import java.io.FilePermission;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.MissingResourceException;
import java.util.ResourceBundle;
import javax.swing.UIManager;
import javax.swing.LookAndFeel;
import javax.swing.UnsupportedLookAndFeelException;
import sun.awt.DebugHelper;

import javax.print.attribute.Size2DSyntax;
import javax.print.attribute.standard.MediaSize;
import javax.print.attribute.standard.MediaSizeName;

/** 
 * A facade object which provides a simpler interface to JobAttributes and
 * PageAttributes, and provides control over the cross-platform print dialog.
 *
 * @version 	1.9, 01/23/03
 * @author 	Amy Fowler
 * @author      David Mendenhall
 */
public abstract class PrintControl {
    public static final MediaType SIZES[] = {
        MediaType.ISO_4A0, MediaType.ISO_2A0, MediaType.ISO_A0,
	MediaType.ISO_A1, MediaType.ISO_A2, MediaType.ISO_A3,
	MediaType.ISO_A4, MediaType.ISO_A5, MediaType.ISO_A6, MediaType.ISO_A7,
	MediaType.ISO_A8, MediaType.ISO_A9, MediaType.ISO_A10,
	MediaType.ISO_B0, MediaType.ISO_B1, MediaType.ISO_B2, MediaType.ISO_B3,
	MediaType.ISO_B4, MediaType.ISO_B5, MediaType.ISO_B6, MediaType.ISO_B7,
	MediaType.ISO_B8, MediaType.ISO_B9, MediaType.ISO_B10,
	MediaType.JIS_B0, MediaType.JIS_B1, MediaType.JIS_B2, MediaType.JIS_B3,
	MediaType.JIS_B4, MediaType.JIS_B5, MediaType.JIS_B6, MediaType.JIS_B7,
	MediaType.JIS_B8, MediaType.JIS_B9, MediaType.JIS_B10,
	MediaType.ISO_C0, MediaType.ISO_C1, MediaType.ISO_C2, MediaType.ISO_C3,
	MediaType.ISO_C4, MediaType.ISO_C5, MediaType.ISO_C6, MediaType.ISO_C7,
	MediaType.ISO_C8, MediaType.ISO_C9, MediaType.ISO_C10,
	MediaType.ISO_DESIGNATED_LONG, MediaType.EXECUTIVE, MediaType.FOLIO,
	MediaType.INVOICE, MediaType.LEDGER, MediaType.NA_LETTER,
	MediaType.NA_LEGAL, MediaType.QUARTO, MediaType.A, MediaType.B,
	MediaType.C, MediaType.D, MediaType.E, MediaType.NA_10X15_ENVELOPE,
	MediaType.NA_10X14_ENVELOPE, MediaType.NA_10X13_ENVELOPE,
	MediaType.NA_9X12_ENVELOPE, MediaType.NA_9X11_ENVELOPE,
	MediaType.NA_7X9_ENVELOPE, MediaType.NA_6X9_ENVELOPE,
	MediaType.NA_NUMBER_9_ENVELOPE, MediaType.NA_NUMBER_10_ENVELOPE,
	MediaType.NA_NUMBER_11_ENVELOPE, MediaType.NA_NUMBER_12_ENVELOPE,
	MediaType.NA_NUMBER_14_ENVELOPE, MediaType.INVITE_ENVELOPE,
	MediaType.ITALY_ENVELOPE, MediaType.MONARCH_ENVELOPE,
	MediaType.PERSONAL_ENVELOPE
    };

    /* This array maps the above array to the objects used by the
     * javax.print APIs
     */
    public static final MediaSizeName JAVAXSIZES[] = {
	null, null, MediaSizeName.ISO_A0,
	MediaSizeName.ISO_A1, MediaSizeName.ISO_A2, MediaSizeName.ISO_A3,
	MediaSizeName.ISO_A4, MediaSizeName.ISO_A5, MediaSizeName.ISO_A6,
	MediaSizeName.ISO_A7, MediaSizeName.ISO_A8, MediaSizeName.ISO_A9,
	MediaSizeName.ISO_A10,
	MediaSizeName.ISO_B0, MediaSizeName.ISO_B1, MediaSizeName.ISO_B2,
	MediaSizeName.ISO_B3, MediaSizeName.ISO_B4, MediaSizeName.ISO_B5,
	MediaSizeName.ISO_B6, MediaSizeName.ISO_B7, MediaSizeName.ISO_B8,
	MediaSizeName.ISO_B9, MediaSizeName.ISO_B10,
	MediaSizeName.ISO_C0, MediaSizeName.ISO_C1, MediaSizeName.ISO_C2,
	MediaSizeName.ISO_C3, MediaSizeName.ISO_C4, MediaSizeName.ISO_C5,
	MediaSizeName.ISO_C6, null, null, null, null,
	MediaSizeName.ISO_DESIGNATED_LONG, MediaSizeName.EXECUTIVE,
	MediaSizeName.FOLIO, MediaSizeName.INVOICE, MediaSizeName.LEDGER,
	MediaSizeName.NA_LETTER, MediaSizeName.NA_LEGAL,
	MediaSizeName.QUARTO, MediaSizeName.A, MediaSizeName.B,
	MediaSizeName.C, MediaSizeName.D, MediaSizeName.E,
	MediaSizeName.NA_10X15_ENVELOPE, MediaSizeName.NA_10X14_ENVELOPE,
	MediaSizeName.NA_10X13_ENVELOPE, MediaSizeName.NA_9X12_ENVELOPE,
	MediaSizeName.NA_9X11_ENVELOPE, MediaSizeName.NA_7X9_ENVELOPE,
	MediaSizeName.NA_6X9_ENVELOPE,
	MediaSizeName.NA_NUMBER_9_ENVELOPE,
	MediaSizeName.NA_NUMBER_10_ENVELOPE,
	MediaSizeName.NA_NUMBER_11_ENVELOPE,
	MediaSizeName.NA_NUMBER_12_ENVELOPE,
	MediaSizeName.NA_NUMBER_14_ENVELOPE,
	null, MediaSizeName.ITALY_ENVELOPE,
	MediaSizeName.MONARCH_ENVELOPE, MediaSizeName.PERSONAL_ENVELOPE,
    };

    // widths and lengths in PostScript points (1/72 in.)
    public static final int WIDTHS[] = {
        /*iso-4a0*/ 4768, /*iso-2a0*/ 3370, /*iso-a0*/ 2384, /*iso-a1*/ 1684,
        /*iso-a2*/ 1191, /*iso-a3*/ 842, /*iso-a4*/ 595, /*iso-a5*/ 420, 
	/*iso-a6*/ 298, /*iso-a7*/ 210, /*iso-a8*/ 147, /*iso-a9*/ 105,
        /*iso-a10*/ 74, /*iso-b0*/ 2835, /*iso-b1*/ 2004, /*iso-b2*/ 1417,
        /*iso-b3*/ 1001, /*iso-b4*/ 709, /*iso-b5*/ 499, /*iso-b6*/ 354,
        /*iso-b7*/ 249, /*iso-b8*/ 176, /*iso-b9*/ 125, /*iso-b10*/ 88, 
        /*jis-b0*/ 2920, /*jis-b1*/ 2064, /*jis-b2*/ 1460, /*jis-b3*/ 1032, 
        /*jis-b4*/ 729, /*jis-b5*/ 516, /*jis-b6*/ 363, /*jis-b7*/ 258,
        /*jis-b8*/ 181, /*jis-b9*/ 128, /*jis-b10*/ 91, /*iso-c0*/ 2599,
        /*iso-c1*/ 1837, /*iso-c2*/ 1298, /*iso-c3*/ 918, /*iso-c4*/ 649,
        /*iso-c5*/ 459, /*iso-c6*/ 323, /*iso-c7*/ 230, /*iso-c8*/ 162, 
        /*iso-c9*/ 113, /*iso-c10*/ 79, /*iso-designated-long*/ 312,
        /*executive*/ 522, /*folio*/ 612, /*invoice*/ 396, /*ledger*/ 792,
        /*na-letter*/ 612, /*na-legal*/ 612, /*quarto*/ 609, /*a*/ 612,
        /*b*/ 792, /*c*/ 1224, /*d*/ 1584, /*e*/ 2448,
        /*na-10x15-envelope*/ 720, /*na-10x14-envelope*/ 720,
        /*na-10x13-envelope*/ 720, /*na-9x12-envelope*/ 648,
        /*na-9x11-envelope*/ 648, /*na-7x9-envelope*/ 504,
        /*na-6x9-envelope*/ 432, /*na-number-9-envelope*/ 279,
        /*na-number-10-envelope*/ 297, /*na-number-11-envelope*/ 324,
        /*na-number-12-envelope*/ 342, /*na-number-14-envelope*/ 360,
        /*invite-envelope*/ 624, /*italy-envelope*/ 312,
        /*monarch-envelope*/ 279, /*personal-envelope*/ 261
    };
    public static final int LENGTHS[] = {
        /*iso-4a0*/ 6741, /*iso-2a0*/ 4768, /*iso-a0*/ 3370, /*iso-a1*/ 2384,
        /*iso-a2*/ 1684, /*iso-a3*/ 1191, /*iso-a4*/ 842, /*iso-a5*/ 595, 
	/*iso-a6*/ 420, /*iso-a7*/ 298, /*iso-a8*/ 210, /*iso-a9*/ 147,
        /*iso-a10*/ 105, /*iso-b0*/ 4008, /*iso-b1*/ 2835, /*iso-b2*/ 2004,
        /*iso-b3*/ 1417, /*iso-b4*/ 1001, /*iso-b5*/ 729, /*iso-b6*/ 499,
        /*iso-b7*/ 354, /*iso-b8*/ 249, /*iso-b9*/ 176, /*iso-b10*/ 125, 
        /*jis-b0*/ 4127, /*jis-b1*/ 2920, /*jis-b2*/ 2064, /*jis-b3*/ 1460, 
        /*jis-b4*/ 1032, /*jis-b5*/ 729, /*jis-b6*/ 516, /*jis-b7*/ 363,
        /*jis-b8*/ 258, /*jis-b9*/ 181, /*jis-b10*/ 128, /*iso-c0*/ 3677,
        /*iso-c1*/ 2599, /*iso-c2*/ 1837, /*iso-c3*/ 1298, /*iso-c4*/ 918,
        /*iso-c5*/ 649, /*iso-c6*/ 459, /*iso-c7*/ 323, /*iso-c8*/ 230, 
        /*iso-c9*/ 162, /*iso-c10*/ 113, /*iso-designated-long*/ 624,
        /*executive*/ 756, /*folio*/ 936, /*invoice*/ 612, /*ledger*/ 1224,
        /*na-letter*/ 792, /*na-legal*/ 1008, /*quarto*/ 780, /*a*/ 792,
        /*b*/ 1224, /*c*/ 1584, /*d*/ 2448, /*e*/ 3168,
        /*na-10x15-envelope*/ 1080, /*na-10x14-envelope*/ 1008,
        /*na-10x13-envelope*/ 936, /*na-9x12-envelope*/ 864,
        /*na-9x11-envelope*/ 792, /*na-7x9-envelope*/ 648,
        /*na-6x9-envelope*/ 648, /*na-number-9-envelope*/ 639,
        /*na-number-10-envelope*/ 684, /*na-number-11-envelope*/ 747,
        /*na-number-12-envelope*/ 792, /*na-number-14-envelope*/ 828,
        /*invite-envelope*/ 624, /*italy-envelope*/ 652,
        /*monarch-envelope*/ 540, /*personal-envelope*/ 468
    };

    private static final DebugHelper dbg =
        DebugHelper.create(PrintControl.class);
    private static final double CM_TO_IN = 2.54;
    private static ResourceBundle messageRB;

    protected Frame dialogOwner;
    protected String doctitle;
    private JobAttributes jobAttributes;
    private PageAttributes pageAttributes;
    private String options = ""; // for backward compatibility
    private FilePermission printToFilePermission;
    private boolean printToFileEnabled = false;

    public static int[] getSize(MediaType mType) {
	int []dim = new int[2];
	dim[0] = 612;
	dim[1] = 792;

	for (int i=0; i < SIZES.length; i++) {
	    if (SIZES[i] == mType) {
		dim[0] = WIDTHS[i];
		dim[1] = LENGTHS[i];
		break;
	    }
	}
	return dim;
    }

    public static MediaSizeName mapMedia(MediaType mType) {
	MediaSizeName media = null;
	
	// compiler doesn't seem to like asserts yet.
	// assert SIZES.length == JAVAXSIZES.length;
	if (SIZES.length != JAVAXSIZES.length) {
	    throw new RuntimeException("array lengths differ");
	}

	for (int i=0; i < SIZES.length; i++) {
	    if (SIZES[i] == mType) {
		if (JAVAXSIZES[i] != null) {
		    media = JAVAXSIZES[i];
		    break;
		} else {
		    /* try & find a close match */
		    float w = (float)Math.rint(WIDTHS[i] *
					      Size2DSyntax.INCH / 72.0);
		    float h = (float)Math.rint(LENGTHS[i] *
					       Size2DSyntax.INCH / 72.0);
		    media = MediaSize.findMedia(w, h, Size2DSyntax.INCH);
		    break;
		}
	    }
	}
	return media;
    }

    public PrintControl(Frame dialogOwner, String doctitle,
			JobAttributes jobAttributes,
			PageAttributes pageAttributes) {
        this.dialogOwner = dialogOwner;
	this.doctitle = (doctitle != null)
	    ? doctitle : "";
	this.jobAttributes = (jobAttributes != null)
	    ? jobAttributes : new JobAttributes();
	this.pageAttributes = (pageAttributes != null)
	    ? pageAttributes : new PageAttributes();

	// Currently, we always reduce page ranges to xxx or xxx-xxx
	int[][] pageRanges = this.jobAttributes.getPageRanges();
	int first = pageRanges[0][0];
	int last = pageRanges[pageRanges.length - 1][1];
	this.jobAttributes.setPageRanges(new int[][] {
	    new int[] { first, last }
	});
	this.jobAttributes.setToPage(last);
	this.jobAttributes.setFromPage(first);

	// Verify that the native print dialog, if specified, has a non-null
	// parent
	if (dialogOwner == null &&
	    (jobAttributes == null ||
	     jobAttributes.getDialog() == JobAttributes.DialogType.NATIVE)) {
	    throw new NullPointerException("frame");
	}

	// Verify that the cross feed and feed resolutions are the same
	int[] res = this.pageAttributes.getPrinterResolution();
	if (res[0] != res[1]) {
	    throw new IllegalArgumentException("Differing cross feed and feed"+
					       " resolutions not supported.");
	}

	// Verify that the app has access to the file system
	DestinationType dest= this.jobAttributes.getDestination();
	if (dest == DestinationType.FILE) {
	    throwPrintToFile();
	    printToFileEnabled = true;
	} else {
	    printToFileEnabled = checkPrintToFile();
	}
    }

    public final ColorType getColorAttrib() {
        return pageAttributes.getColor();
    }
    public final void setColorAttrib(ColorType color) {
        pageAttributes.setColor(color);
    }
    public final int getCopiesAttrib() {
        return jobAttributes.getCopies();
    }
    public final void setCopiesAttrib(int copies) {
        jobAttributes.setCopies(copies);
    }
    public final DefaultSelectionType getSelectAttrib() {
        return jobAttributes.getDefaultSelection();
    }
    public final void setSelectAttrib(DefaultSelectionType select) {
        jobAttributes.setDefaultSelection(select);
    }
    public final DestinationType getDestAttrib() {
        return jobAttributes.getDestination();
    }
    public final void setDestAttrib(DestinationType dest) {
        jobAttributes.setDestination(dest);
    }
    public final DialogType getDialogAttrib() {
        return jobAttributes.getDialog();
    }
    public final String getFileNameAttrib() {
        return jobAttributes.getFileName();    // may return null
    }
    public final void setFileNameAttrib(String fileName) {
        jobAttributes.setFileName(fileName);
    }
    public final int getFromPageAttrib() {
        return jobAttributes.getFromPage();
    }
    public final void setFromPageAttrib(int fromPage) {
        jobAttributes.setFromPage(fromPage);
	jobAttributes.setPageRanges(new int[][] {
	    new int[] { fromPage, jobAttributes.getToPage() }
	});
    }
    public final int getMaxPageAttrib() {
        return jobAttributes.getMaxPage();
    }
    // returns index into SIZES
    public final int getMediaAttrib() {
        return pageAttributes.getMedia().hashCode();
    }
    // media is an index into SIZES
    public final void setMediaAttrib(int media) {
        pageAttributes.setMedia(SIZES[media]);
    }
    public final int getMinPageAttrib() {
        return jobAttributes.getMinPage();
    }
    public final MultipleDocumentHandlingType getMDHAttrib() {
        return jobAttributes.getMultipleDocumentHandling();
    }
    public final void setMDHAttrib(MultipleDocumentHandlingType mdh) {
        jobAttributes.setMultipleDocumentHandling(mdh);
    }
    public final String getOptions() {
        return options;
    }
    public final void setOptions(String options) {
        this.options = (options != null) ? options : "";
    }
    public final OrientationRequestedType getOrientAttrib() {
        return pageAttributes.getOrientationRequested();
    }
    public final void setOrientAttrib(OrientationRequestedType orient) {
        pageAttributes.setOrientationRequested(orient);
    }
    public final OriginType getOriginAttrib() {
        return pageAttributes.getOrigin();
    }
    public final PrintQualityType getQualityAttrib() {
        return pageAttributes.getPrintQuality();
    }
    public final void setQualityAttrib(PrintQualityType quality) {
        pageAttributes.setPrintQuality(quality);
    }
    public final String getPrinterAttrib() {
        String printerName = jobAttributes.getPrinter();
        return (printerName != null) ? printerName : getDefaultPrinterName();
    }
    public final void setPrinterAttrib(String printer) {
        jobAttributes.setPrinter(printer);
    }
    public final boolean getPrintToFileEnabled() {
        return printToFileEnabled;
    }
    public final int getResAttrib() {
        int[] res = pageAttributes.getPrinterResolution();
	if (res[2] == 3) {
	    return res[0];
	} else /* if (res[2] == 4) */ {
	    return (int)(res[0] * CM_TO_IN);
	}
    }
    public SidesType getSidesAttrib() {
        return jobAttributes.getSides();
    }
    public void setSidesAttrib(SidesType sides) {
        jobAttributes.setSides(sides);
    }
    public int getToPageAttrib() {
        return jobAttributes.getToPage();
    }
    public void setToPageAttrib(int toPage) {
        jobAttributes.setToPage(toPage);
	jobAttributes.setPageRanges(new int[][] {
	    new int[] { jobAttributes.getFromPage(), toPage }
	});
    }

    public abstract String getDefaultPrinterName();
    public abstract boolean getCapabilities(PrinterCapabilities capabilities);

    // updatable's updatePrinterList function may be called synchronously
    // or asynchronously depending on the capabilities of the platform.
    public abstract void getPrinterList(PrinterListUpdatable updatable);

    /**
     * Initialize ResourceBundle
     */
    static void initResource() {
        AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
	        try {
		    messageRB = ResourceBundle.getBundle(
                        "sun.awt.print.resources.printcontrol");
		    return null;
		} catch (MissingResourceException e) {
		    throw new Error("Fatal: Resource for AwtPrintControl is "+
				    "missing.");
		}
	    }
	});
    }

    /**
     * get message string from resource
     */
    static synchronized String getMsg(String key) {
        if (messageRB == null) {
	    initResource();
	}
	try {
	    return messageRB.getString(key);
	} catch (MissingResourceException e) {
	    throw new Error("Fatal: Resource for AwtPrintControl is broken. "+
			    "There is no " + key + " key in resource");
	}
    }

    public boolean displayDialog() {
	if (getDialogAttrib() == DialogType.COMMON) {
            return displayCommonDialog();
        } else if (getDialogAttrib() == DialogType.NATIVE) {
            return displayNativeDialog();
        } else /* if (getDialogAttrib() == DialogType.NONE) */ {
            return true;
        }
    }

    protected final boolean displayCommonDialog() {
        LookAndFeel laf = UIManager.getLookAndFeel();
	try {
	    UIManager.setLookAndFeel(
                UIManager.getCrossPlatformLookAndFeelClassName());
	} catch (ClassNotFoundException e) {
	} catch (IllegalAccessException e) {
	} catch (InstantiationException e) {
	} catch (UnsupportedLookAndFeelException e) {
	}

        PrintDialog printDialog = (PrintDialog)
	    java.security.AccessController.doPrivileged(
                new java.security.PrivilegedAction() {
		    public Object run() {
                        return new PrintDialog(dialogOwner, PrintControl.this);
		    }
	    });

	printDialog.setDocumentTitle(doctitle);
	printDialog.setDestType(getDestAttrib());
	printDialog.setDestString(DestinationType.PRINTER, getPrinterAttrib());
	printDialog.setDestString(DestinationType.FILE, getFileNameAttrib());
	printDialog.setPrintToFileEnabled(getPrintToFileEnabled());
	printDialog.setOptionsString(getOptions());
	printDialog.setPrintRange(getSelectAttrib());
	printDialog.setMinMaxPage(getMinPageAttrib(), getMaxPageAttrib());
	printDialog.setFromPage(getFromPageAttrib());
	printDialog.setToPage(getToPageAttrib());
	printDialog.setCopies(getCopiesAttrib());
	printDialog.setCollate(getMDHAttrib() ==
            MultipleDocumentHandlingType.SEPARATE_DOCUMENTS_COLLATED_COPIES);
	printDialog.setMedia(PrintControl.SIZES[getMediaAttrib()]);
	printDialog.setOrientation(getOrientAttrib());
	printDialog.setColor(getColorAttrib());
	printDialog.setQuality(getQualityAttrib());
	printDialog.setSides(getSidesAttrib());

	boolean ret = printDialog.display();
	if (ret) {
	    if (printDialog.getDestType() == DestinationType.FILE) {
		String filename = 
		    printDialog.getDestString(DestinationType.FILE);
		if (filename != null && !filename.equals("") &&
		    (new java.io.File(filename)).exists()) {
		    String cancelCmd = getMsg("warning_button.cancel");
		    String overwriteCmd = getMsg("warning_button.overwrite");
		    PrintStatusDialog d =
			new sun.awt.print.PrintStatusDialog(printDialog,
                            getMsg("warning_dialog.title"),
                            getMsg("warning_msg.file_exists") + filename,
                            overwriteCmd,
                            cancelCmd);
		    d.setLocationRelativeTo(printDialog);
		    d.setVisible(true);
		    if (d.getCommand().equals(cancelCmd)) {
			ret = false;
		    }
		    d.dispose();
                }
	    }
	    if (ret) {
		createAttributes(printDialog);
	    }
	}
	printDialog.dispose();

	try {
	    UIManager.setLookAndFeel(laf);
	} catch (UnsupportedLookAndFeelException e) {
	}
  
	return ret;
    }

    public JobAttributes getJobAttributes() {
	return jobAttributes;
    }

    public PageAttributes getPageAttributes() {
	return pageAttributes;
    }


    // Default implementation is to use cross-platform dialog. Override
    // for real native support.
    protected boolean displayNativeDialog() {
        return displayCommonDialog();
    }

    // Called by the cross-platform PrintDialog when the user clicks the "OK"
    // button indicating the start of a print job.
    protected void createAttributes(PrintDialog dialog) {
        // Make attribute modifications to a temporary copies in case the
        // PrintJob is cancelled during construction.
        JobAttributes oldJobAttributes = jobAttributes;
	PageAttributes oldPageAttributes = pageAttributes;
	jobAttributes = (JobAttributes)(jobAttributes.clone());
	pageAttributes = (PageAttributes)(pageAttributes.clone());

	doctitle = dialog.getDocumentTitle();

        setDestAttrib(dialog.getDestType());

	// Set printer even when printing to a file because specific
	// printers could produce different output files (e.g., PCL v.
	// PostScript) or support different paper sizes
	String printer = dialog.getDestString(DestinationType.PRINTER);
	if (printer != null && !printer.equals("")) {
	    setPrinterAttrib(printer);
	}

	if (getDestAttrib() == DestinationType.FILE) {
	    String filename = dialog.getDestString(DestinationType.FILE);
	    if (filename != null && !filename.equals("")) {
	        setFileNameAttrib(filename);
	    }
	}

	String lpOptions = dialog.getOptionsString();
	if (lpOptions != null && !lpOptions.equals("")) {
	    setOptions(lpOptions);
	}
 
	setSelectAttrib(dialog.getPrintRange());
	if (getSelectAttrib() == DefaultSelectionType.RANGE) {
	    setToPageAttrib(dialog.getToPage());
	    setFromPageAttrib(dialog.getFromPage());
	}

	int copies = dialog.getCopies();
	if (copies > 0) {
	    setCopiesAttrib(copies);
	}
	setMDHAttrib((dialog.getCollate())
          ? MultipleDocumentHandlingType.SEPARATE_DOCUMENTS_COLLATED_COPIES
          : MultipleDocumentHandlingType.SEPARATE_DOCUMENTS_UNCOLLATED_COPIES);
	setMediaAttrib(dialog.getMedia().hashCode());
	setOrientAttrib(dialog.getOrientation());
	setColorAttrib(dialog.getColor());
	setQualityAttrib(dialog.getQuality());
	setSidesAttrib(dialog.getSides());

	// commit to updated Properties object
	oldJobAttributes.set(jobAttributes);
	oldPageAttributes.set(pageAttributes);

	jobAttributes = oldJobAttributes;
	pageAttributes = oldPageAttributes;
	return;
    }

    private boolean checkPrintToFile() {
        try {
	    throwPrintToFile();
	    return true;
	} catch (SecurityException e) {
	    return false;
	}
    }
    private void throwPrintToFile() {
        SecurityManager security = System.getSecurityManager();
	if (security != null) {
	    if (printToFilePermission == null) {
                printToFilePermission =
	            new FilePermission("<<ALL FILES>>", "read,write");
	    }
	    security.checkPermission(printToFilePermission);
	}
    }
}
