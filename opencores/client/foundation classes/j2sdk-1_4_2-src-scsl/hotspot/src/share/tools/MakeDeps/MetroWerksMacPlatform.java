/*
 * @(#)MetroWerksMacPlatform.java	1.3 03/01/23 11:12:09
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

import java.io.*;

public class MetroWerksMacPlatform extends Platform {
    public void setupFileTemplates() {
	inclFileTemplate = new FileName(this,
	    ":incls:", "_", "",                  ".incl", "", ""
	);
        giFileTemplate = new FileName(this,
	    "",        "",  "precompiledHeader", ".pch",  "", ""
	);
        gdFileTemplate = dummyFileTemplate;
    }

    private static String[] suffixes = { ".cpp", ".c", ".s" };

    public String[] outerSuffixes() {
	return suffixes;
    }

    public boolean includeGIInEachIncl() {
	return true;
    }

    public int defaultGrandIncludeThreshold() {
	return 150;
    }

    public void writeGIPragma(PrintWriter out) {
	out.println("#pragma precompile_target \"" +
		    giFileTemplate.preStemAltSuff() +
		    "\"");
	out.println();
    }
    
    public String objFileSuffix() {
	throw new RuntimeException("Unimplemented in original makeDeps");
    }

    public String asmFileSuffix() {
	throw new RuntimeException("Unimplemented in original makeDeps");
    }

    public String dependentPrefix() {
	throw new RuntimeException("Unimplemented in original makeDeps");
    }
}
