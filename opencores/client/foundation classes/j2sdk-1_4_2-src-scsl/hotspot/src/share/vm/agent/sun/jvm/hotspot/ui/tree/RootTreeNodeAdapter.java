/* 
 * @(#)RootTreeNodeAdapter.java	1.1 02/10/07 16:10:05
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui.tree;

import sun.jvm.hotspot.oops.*;

/** Root oop pointer */

public class RootTreeNodeAdapter extends FieldTreeNodeAdapter {
  private String val;

  public RootTreeNodeAdapter(String val, FieldIdentifier id) {
    this(val, id, false);
  }

  /** The identifier may be null, i.e., for the root of the tree */
  public RootTreeNodeAdapter(String val, FieldIdentifier id, boolean treeTableMode) {
    super(id, treeTableMode);
    this.val = val;
  }

  public int getChildCount() {
    return 0;
  }

  public SimpleTreeNode getChild(int index) {
    return null;
  }

  public boolean isLeaf() {
    return true;
  }

  public int getIndexOfChild(SimpleTreeNode child) {
    return 0;
  }

  public String getValue() {
    return val;
  }
}
