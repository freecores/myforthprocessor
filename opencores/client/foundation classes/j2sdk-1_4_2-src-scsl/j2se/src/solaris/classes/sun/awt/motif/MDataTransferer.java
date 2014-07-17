/*
 * @(#)MDataTransferer.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.Image;

import java.io.InputStream;
import java.io.IOException;

import sun.awt.datatransfer.DataTransferer;
import sun.awt.datatransfer.ToolkitThreadBlockedHandler;

/**
 * Platform-specific support for the data transfer subsystem.
 *
 * @author Roger Brinkley
 * @author Danila Sinopalnikov
 * @version 1.18, 01/23/03
 *
 * @since 1.3.1
 */
public class MDataTransferer extends DataTransferer {
    private static final long FILE_NAME_ATOM;
    private static final long DT_NET_FILE_ATOM;
    private static final long PNG_ATOM;
    private static final long JFIF_ATOM;

    static {
	FILE_NAME_ATOM = getAtomForTarget("FILE_NAME");
	DT_NET_FILE_ATOM = getAtomForTarget("_DT_NETFILE");
	PNG_ATOM = getAtomForTarget("PNG");
	JFIF_ATOM = getAtomForTarget("JFIF");
    }
  
    /**
     * Singleton constructor
     */
    private MDataTransferer() {
    }

    private static MDataTransferer transferer;

    public static MDataTransferer getInstanceImpl() {
        if (transferer == null) {
            synchronized (MDataTransferer.class) {
                if (transferer == null) {            
                    transferer = new MDataTransferer();
                }
            }
        }
        return transferer;
    }

    public String getDefaultUnicodeEncoding() {
	return "iso-10646-ucs-2";
    }
    
    public boolean isLocaleDependentTextFormat(long format) {
        return false;
    }

    public boolean isFileFormat(long format) {
        return format == FILE_NAME_ATOM || format == DT_NET_FILE_ATOM;
    }

    public boolean isImageFormat(long format) {
        return format == PNG_ATOM || format == JFIF_ATOM;
    }

    protected Long getFormatForNativeAsLong(String str) {
        // Just get the atom. If it has already been retrived
        // once, we'll get a copy so this should be very fast.
        long atom = getAtomForTarget(str);
        if (atom <= 0) {
            throw new InternalError("Cannot register a target");
        }
        return new Long(atom);
    }

    protected String getNativeForFormat(long format) {
        return getTargetNameForAtom(format);
    }

    public ToolkitThreadBlockedHandler getToolkitThreadBlockedHandler() {
        return MToolkitThreadBlockedHandler.getToolkitThreadBlockedHandler();
    }

    /**
     * Gets an atom for a format name. 
     */
    static native long getAtomForTarget(String name);

    /**
     * Gets an format name for a given format (atom)
     */
    private static native String getTargetNameForAtom(long atom);

    protected byte[] imageToPlatformBytes(Image image, long format) 
      throws IOException {
        String mimeType = null;
        if (format == PNG_ATOM) {
            mimeType = "image/png";
        } else if (format == JFIF_ATOM) {
            mimeType = "image/jpeg";
        }
        if (mimeType != null) {
            return imageToStandardBytes(image, mimeType);
        } else {
            String nativeFormat = getNativeForFormat(format);
            throw new IOException("Translation to " + nativeFormat + 
                                  " is not supported."); 
        }
    }

    /**
     * Translates either a byte array or an input stream which contain
     * platform-specific image data in the given format into an Image.
     */
    protected Image platformImageBytesOrStreamToImage(InputStream inputStream, 
                                                      byte[] bytes, 
                                                      long format) 
      throws IOException {
        String mimeType = null;
        if (format == PNG_ATOM) {
            mimeType = "image/png";
        } else if (format == JFIF_ATOM) {
            mimeType = "image/jpeg";
        }
        if (mimeType != null) {
            return standardImageBytesOrStreamToImage(inputStream, bytes, mimeType);
        } else {
            String nativeFormat = getNativeForFormat(format);
            throw new IOException("Translation from " + nativeFormat + 
                                  " is not supported."); 
        }
    }
}
