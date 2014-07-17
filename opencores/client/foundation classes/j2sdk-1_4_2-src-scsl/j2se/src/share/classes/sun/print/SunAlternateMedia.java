/*
 * @(#)SunAlternateMedia.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

import javax.print.attribute.PrintRequestAttribute;
import javax.print.attribute.standard.Media;

/*
 * An implementation class used by services which can distinguish media
 * by size and media by source. Values are expected to be MediaTray
 * instances, but this is not enforced by API. 
 */
public class SunAlternateMedia implements PrintRequestAttribute {

    private Media media;

    public SunAlternateMedia(Media altMedia) {
        media = altMedia;
    }

    public Media getMedia() {
        return media;
    }

    public final Class getCategory() {
        return SunAlternateMedia.class;
    }

    public final String getName() {
        return "sun-alternate-media";
    }
 
    public String toString() {
       return "alternatate-media: " + media.toString();
    }

    /**
     * Returns a hash code value for this enumeration value. The hash code is
     * just this enumeration value's integer value.
     */
    public int hashCode() {
        return media.hashCode();
    }
}
