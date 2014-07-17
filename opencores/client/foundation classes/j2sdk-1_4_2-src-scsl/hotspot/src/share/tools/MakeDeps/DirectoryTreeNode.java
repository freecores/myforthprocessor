/*
 * @(#)DirectoryTreeNode.java	1.4 03/01/23 11:11:52
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

import java.util.*;

public interface DirectoryTreeNode {
    public boolean isFile();
    public boolean isDirectory();
    public String getName();
    public String getParent();
    public Iterator getChildren() throws IllegalArgumentException;
    public int getNumChildren() throws IllegalArgumentException;
    public DirectoryTreeNode getChild(int i)
	throws IllegalArgumentException, ArrayIndexOutOfBoundsException;
}
