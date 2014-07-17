/*
 * %W% %E%
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import java.net.URL;
import com.sun.javaws.xml.*;


public class IconDesc implements XMLable {
    private URL _location; // URL to GIF or JPEG
    private String _version; // Version requested
    private String _locationString; // _location.toString
    private int _height;   // Height in pixels
    private int _width;    // Width in pixels
    private int _depth;    // Color depth (unknown ==  0)
    private int _kind;     // Kind. See constants below
    
    /** Constants for icon kinds */
    final public static int ICON_KIND_DEFAULT  = 0;
    final public static int ICON_KIND_SELECTED = 1;
    final public static int ICON_KIND_DISABLED = 2;
    final public static int ICON_KIND_ROLLOVER = 3;
    final public static int ICON_KIND_SPLASH   = 4;
    
    public IconDesc(URL location, String version, int height, int width, int depth, int kind) {
	_location = location;
        if (_location != null) {
            _locationString = _location.toExternalForm();
        }
	_width = width;
	_depth = depth;
	_kind  = kind;
	_version = version;
    }
    
    public URL getLocation() { return _location; };
    public String getVersion() { return _version; }
    public int getHeight()   { return _height; }
    public int getWidth()    { return _width; }
    public int getDepth()    { return _depth; }
    public int getKind()     { return _kind; }
    
    public final boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o instanceof IconDesc) {
            IconDesc oid = (IconDesc)o;

            if (((_version == null && oid._version == null) ||
                 (_version != null && _version.equals(oid._version))) &&
                ((_locationString == null && oid._locationString == null) ||
                 (_locationString != null &&
                  _locationString.equals(oid._locationString)))) {
                return true;
            }
        }
        return false;
    }

    public int hashCode() {
        int hc = 0;

        if (_locationString != null) {
            hc |= _locationString.hashCode();
        }
        if (_version != null) {
            hc |= _version.hashCode();
        }
        return hc;
    }

    private static final String kinds[] = { "default", "selected", "disabled", "rollover", "splash" };
    
    public XMLNode asXML() {
	XMLAttributeBuilder ab = new XMLAttributeBuilder();
	ab.add("href", _location);
	ab.add("height", _height);
	ab.add("width", _width);
	ab.add("depth", _depth);
	ab.add("kind", kinds[_kind]);
	return new XMLNode("icon", ab.getAttributeList());
    }
};

