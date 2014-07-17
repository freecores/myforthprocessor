/*
 * @(#)PrinterCapabilities.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.print;

import java.awt.PageAttributes.MediaType;


public final class PrinterCapabilities implements Cloneable {
    public static final int COLOR = 0x1;
    public static final int DUPLEX = 0x2;
    public static final int MEDIA = 0x4;

    private final String printerName;
    private long capabilities;
    private int[] sizes;
    private Object userData;

    public PrinterCapabilities(String printerName) {
        if (printerName == null) {
            throw new IllegalArgumentException("printerName cannot be null");
        }
        this.printerName = printerName;
    }
    public PrinterCapabilities(String printerName, long capabilities,
                               int[] sizes, Object userData) {
        this(printerName);
        setCapabilities(capabilities);
        setSizes(sizes);
        setUserData(userData);
    }
    public PrinterCapabilities(PrinterCapabilities rhs) {
        this.printerName = rhs.printerName;
        this.capabilities = rhs.capabilities;
        this.sizes = rhs.sizes;
        this.userData = rhs.userData;
    }
    public Object clone() {
        try {
            return super.clone();
        } catch (CloneNotSupportedException e) {
            // Since we implement Cloneable, this should never happen
            throw new InternalError();
        }
    }
    public void set(PrinterCapabilities rhs) {
        if (!printerName.equals(rhs.printerName)) {
            throw new IllegalArgumentException("setting capabilities for "+
                                               "different printers");
        }
        this.capabilities = rhs.capabilities;
        this.sizes = rhs.sizes;
    }
    public String getPrinterName() {
        return printerName;
    }
    public long getCapabilities() {
        return capabilities;
    }
    public void setCapabilities(long capabilities) {
        if ((capabilities & ~(COLOR ^ DUPLEX ^ MEDIA)) != 0) {
            throw new IllegalArgumentException("invalid value for "+
                                               "capabilities");
        }
        this.capabilities = capabilities;
        if ((capabilities & MEDIA) == 0) {
            sizes = null;
        }
    }
    public int[] getSizes() {
        return (sizes != null) ? (int[])sizes.clone() : null;
    }
    public void setSizes(int[] sizes) {
        if (sizes == null && (capabilities & MEDIA) == 0) {
            return;
        }
        if ((capabilities & MEDIA) == 0) {
            throw new IllegalStateException("sizes cannot be set if printer "+
                                            "does not enumerate media types");
        }
        if (sizes == null) {
            throw new IllegalArgumentException("sizes cannot be null");
        }
        this.sizes = (int[])sizes.clone();
    }
    public Object getUserData() {
        return userData;
    }
    public void setUserData(Object userData) {
        this.userData = userData;
    }
    public String toString() {
        String str = printerName + " (";
        boolean outerFirst = true;
        if ((capabilities & COLOR) != 0) {
            outerFirst = false;
            str += "COLOR";
        }
        if ((capabilities & DUPLEX) != 0) {
            if (outerFirst) {
                outerFirst = false;
            } else {
                str += ",";
            }
            str += "DUPLEX";
        }
        if ((capabilities & MEDIA) != 0) {
            if (outerFirst) {
                outerFirst = false;
            } else {
                str += ",";
            }
            str += "MEDIA=[";
            boolean innerFirst = true;
            for (int i = 0; i < sizes.length; i++) {
                if (innerFirst) {
                    innerFirst = false;
                } else {
                    str += ",";
                }
                str += PrintControl.SIZES[sizes[i]].toString();
            }
            str += "]";
        }
        str += ")";
        return str;
    }
}
