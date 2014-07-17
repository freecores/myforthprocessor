/*
 * @(#)FloatTreeNodeAdapter.java	1.3 03/01/23 11:23:16
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.bugspot.tree;

import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.ui.tree.SimpleTreeNode;

/** Encapsulates a float value in a tree handled by SimpleTreeModel */

public class FloatTreeNodeAdapter extends FieldTreeNodeAdapter {
  private float val;

  public FloatTreeNodeAdapter(float val, FieldIdentifier id) {
    this(val, id, false);
  }

  public FloatTreeNodeAdapter(float val, FieldIdentifier id, boolean treeTableMode) {
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
    return Float.toString(val);
  }
}
