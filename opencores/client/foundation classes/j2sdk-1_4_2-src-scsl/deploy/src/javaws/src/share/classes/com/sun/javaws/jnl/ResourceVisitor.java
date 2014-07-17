/*
 * @(#)ResourceVisitor.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;
import com.sun.javaws.xml.XMLable;

/**
 * A visitor interface for the various ResourceType objects
 */
public interface ResourceVisitor {
    public void visitJARDesc(JARDesc jad);
    public void visitPropertyDesc(PropertyDesc prd);
    public void visitPackageDesc(PackageDesc pad);
    public void visitExtensionDesc(ExtensionDesc ed);
    public void visitJREDesc(JREDesc jrd);
}

