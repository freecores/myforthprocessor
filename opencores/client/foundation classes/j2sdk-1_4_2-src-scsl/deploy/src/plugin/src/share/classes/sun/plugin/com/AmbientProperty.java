/*
 * @(#)AmbientProperty.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

public interface AmbientProperty {
    public void setBackground(int r, int g, int b);
    public void setForeground(int r, int g, int b);
    public void setFont(String name, int style, int size);

    public int getBackground();
    public int getForeground();
    public java.awt.Font getFont();
}
