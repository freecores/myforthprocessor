/*
 * @(#)LineNumData.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.tools.javap;

import java.util.*;
import java.io.*;

/**
 * Strores LineNumberTable data information.
 *
 * @author  Sucheta Dambalkar (Adopted code from jdis)
 */
class LineNumData {
    short start_pc, line_number;
    
    public LineNumData() {}
    
    /**
     * Read LineNumberTable attribute.
     */
    public LineNumData(DataInputStream in) throws IOException {
	start_pc = in.readShort();
	line_number=in.readShort();
	
    }
}

