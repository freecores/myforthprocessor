/*
 * @(#)AddressTreeNodeAdapter.java	1.3 03/01/23 11:23:07
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.bugspot.tree;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.ui.tree.SimpleTreeNode;

/** Encapsulates a float value in a tree handled by SimpleTreeModel */

public class AddressTreeNodeAdapter extends FieldTreeNodeAdapter {
  private Address val;

  public AddressTreeNodeAdapter(Address val, FieldIdentifier id) {
    this(val, id, false);
  }

  public AddressTreeNodeAdapter(Address val, FieldIdentifier id, boolean treeTableMode) {
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
    if (val != null) {
      return val.toString();
    }
    return "NULL";
  }
}
