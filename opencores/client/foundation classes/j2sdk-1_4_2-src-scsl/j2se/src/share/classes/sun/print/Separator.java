/*
 * @(#)Separator.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

import java.awt.*;

class Separator extends Canvas {
    public final static int HORIZONTAL = 0;
    public final static int VERTICAL = 1;    
    int orientation;

    public Separator(int length, int thickness, int orient) {
        super();
        orientation = orient;
        if (orient == HORIZONTAL) {
            resize(length, thickness);
        } else { // VERTICAL 
            resize(thickness, length);
        }
    }

    public void paint(Graphics g) {
        int x1, y1, x2, y2;
        Rectangle bbox = bounds();
        Color c = getBackground();
        Color brighter = c.brighter();
        Color darker = c.darker();

        if (orientation == HORIZONTAL) {
            x1 = 0;
            x2 = bbox.width - 1;
            y1 = y2 = bbox.height/2 - 1;

        } else { // VERTICAL
            x1 = x2 = bbox.width/2 - 1;
            y1 = 0;
            y2 = bbox.height - 1;
        }
        g.setColor(darker);
        g.drawLine(x1, y2, x2, y2);
        g.setColor(brighter);
        if (orientation == HORIZONTAL)
            g.drawLine(x1, y2+1, x2, y2+1);
        else
            g.drawLine(x1+1, y2, x2+1, y2);
    }
}
