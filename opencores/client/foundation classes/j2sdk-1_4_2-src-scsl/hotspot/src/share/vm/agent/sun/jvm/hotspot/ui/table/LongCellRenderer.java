/* 
 * @(#)LongCellRenderer.java	1.2 03/01/23 11:50:06
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui.table;

import java.awt.Component;

import javax.swing.*;
import javax.swing.table.*;

/**
 * A renderer for long values.
 */
public class LongCellRenderer extends DefaultTableCellRenderer {

    private JFormattedTextField textField;

    // Subclassed to set the background value 
    public Component getTableCellRendererComponent(JTable table, Object value, 
						   boolean isSelected, boolean hasFocus,
						   int row, int column)  {
	if (textField == null) {
	    textField = new JFormattedTextField();
	    textField.setFont(table.getFont());
	    textField.setHorizontalAlignment(JTextField.RIGHT);
	}

	textField.setForeground(isSelected ? table.getSelectionForeground() :
				table.getForeground());
	textField.setBackground(isSelected ? table.getSelectionBackground() :
				table.getBackground());
	if (hasFocus) {
	    textField.setBorder(UIManager.getBorder("Table.focusCellHighlightBorder"));
	} else {
	    textField.setBorder(noFocusBorder);
	}

	textField.setValue((Long)value);
	    
	return textField;
    }
}
