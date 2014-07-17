/* 
 * @(#)CharTreeNodeAdapter.java	1.4 03/01/23 11:50:22
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui.tree;

import sun.jvm.hotspot.oops.*;

/** Encapsulates a char value in a tree handled by SimpleTreeModel */

public class CharTreeNodeAdapter extends FieldTreeNodeAdapter {
  private char val;

  public CharTreeNodeAdapter(char val, FieldIdentifier id) {
    this(val, id, false);
  }

  public CharTreeNodeAdapter(char val, FieldIdentifier id, boolean treeTableMode) {
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
    return "'" + val + "'";
  }
}
