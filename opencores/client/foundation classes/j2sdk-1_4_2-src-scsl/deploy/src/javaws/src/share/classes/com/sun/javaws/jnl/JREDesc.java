/*
 * @(#)JREDesc.java	1.13 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import java.net.URL;
import com.sun.javaws.xml.*;

/**
 *  Interface that describes the JRE requirements
 *  for an application
 */
public class JREDesc implements ResourceType {
    private String _version;
    private long   _maxHeap;
    private long   _minHeap;
    private URL    _href;
    private boolean _isSelected;
    private ResourcesDesc _resourceDesc;
    
    // Link to ExtensionDesc for this JRE. This is used if the JRE is being downloaded
    private LaunchDesc _extensioDesc;
    
    public JREDesc(String version, long minHeap, long maxHeap, URL href, ResourcesDesc resourcesDesc) {
        _version = version;
        _maxHeap = maxHeap;
        _minHeap = minHeap;
        _href = href;
        _isSelected = false;
        _resourceDesc = resourcesDesc;
        _extensioDesc = null;
    }
        
    /*
     *  JRE specification
     */
    public String getVersion() { return _version; }
    public URL getHref() { return _href; }
    
    /*
     *  JVM arguments. Returns NULL or 0 if not-specified
     */
    public long getMinHeap() { return _minHeap; }
    public long getMaxHeap() { return _maxHeap; }
    
    /*
     * Is selected or not
     */
    public boolean isSelected() { return _isSelected; };
    public void markAsSelected() { _isSelected = true; }
    
    /** Get nested resources */
    public ResourcesDesc getNestedResources() { return _resourceDesc; }
    
    /*
     *  Get/Set ExtensionDesc for this JRE
     */
    public LaunchDesc getExtensionDesc() { return _extensioDesc; }
    public void setExtensionDesc(LaunchDesc ld) { _extensioDesc = ld; }
    
    /* visitor dispatch */
    public void visit(ResourceVisitor rv) {
        rv.visitJREDesc(this);
    }
    
    /** Outputs XML structure for contents */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("initial-heap-size", _minHeap);
        ab.add("max-heap-size", _maxHeap);
        ab.add("href", _href);
        ab.add("version", _version);
        XMLNode extcNode = (_extensioDesc != null) ? _extensioDesc.asXML() : null;
	if (_resourceDesc != null) extcNode = _resourceDesc.asXML();
        return new XMLNode("j2se", ab.getAttributeList(), extcNode, null);
    }
}

