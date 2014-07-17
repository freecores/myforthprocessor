/*
 * @(#)LaunchSelection.java	1.24 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.net.URL;
import java.util.Locale;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;
import java.io.File;
import java.util.Properties;
import java.util.Enumeration;
import com.sun.javaws.jnl.*;
import com.sun.javaws.util.VersionID;
import com.sun.javaws.util.VersionString;
import com.sun.javaws.debug.*;

/*
 * Class containing single method to determine what JRE to use.
 *
 */
public class LaunchSelection {
    
    /** Select the JRE to choose. The selection algorithm will try to
     *  match based on the order specified in the JNLP file. For example,
     *
     *    <j2se version="1.3 1.2"/>
     *    <j2se version="1.4"/>
     *
     * If a wildcard is used, e.g., 1.2+ or 1.2*, we will try to match on
     * the current running or highest version installed
     *
     * Will match on the platform versions 1.3 1.2 1.4 in that given order
     */
    static ConfigProperties.JREInformation selectJRE(LaunchDesc ld) {
        final JREDesc selectedJREDesc[] = new JREDesc [1];
        final ConfigProperties.JREInformation selectedJRE[] = new ConfigProperties.JREInformation[1];
	
        // Iterate through all JREDesc's
        ResourcesDesc rdescs = ld.getResources();
        rdescs.visit(new ResourceVisitor() {
		    public void visitJARDesc(JARDesc jad) { /* ignore */ }
		    public void visitPropertyDesc(PropertyDesc prd)  { /* ignore */ }
		    public void visitPackageDesc(PackageDesc pad)  { /* ignore */ }
		    public void visitExtensionDesc(ExtensionDesc ed)  { /* ignore */ }
		    public void visitJREDesc(JREDesc jre) {
			if (selectedJRE[0] == null) {
			    handleJREDesc(jre, selectedJRE, selectedJREDesc);
			}
		    }
		});
        // Mark the selected JRE in launchDesc
        selectedJREDesc[0].markAsSelected();
        return selectedJRE[0];
    }
    
    /* Split up version attribute and search for a match in order */
    static private void handleJREDesc(JREDesc jd,
				      ConfigProperties.JREInformation[] selectedJRE,
				      JREDesc[] selectedJREDesc) {
        URL location = jd.getHref();
        String version = jd.getVersion();
	
	/* Split version-string at spaces so we match on each one in order */
	StringTokenizer tokenStream = new StringTokenizer(version, " ", false);
	int count = tokenStream.countTokens();

        if (count > 0) {
	    String versionParts[] = new String[count];
	    for (int i=0; i<count; i++) {
	        versionParts[i] = tokenStream.nextToken();
	    }
	    matchJRE(jd, versionParts, selectedJRE, selectedJREDesc);
	    if (selectedJRE[0] != null) return;
	}
    }
    
    /* Find a match for a particular location and array of versions */
    /* note - use currently running jre if match, otherwise use first match */
    static private void matchJRE(JREDesc jd,
				 String [] versions,
				 ConfigProperties.JREInformation[] selectedJRE,
				 JREDesc[] selectedJREDesc) {
	URL location = jd.getHref();
        VersionString vs;  // Make sure to use passed in versions
	
	
        ConfigProperties cp = ConfigProperties.getInstance();
        ArrayList al = cp.getJREInformationList();
        if (al == null) return;
	
        for (int j=0; j<versions.length; j++) {
            vs = new VersionString(versions[j]);
            for(int i = 0; i < al.size(); i++) {
	        ConfigProperties.JREInformation je =
		    (ConfigProperties.JREInformation)al.get(i);

		// first check if JRE osName and osArch matches
		// (only if osName and osArch exist)
		if (je.isOsInfoMatch(Globals.osName, Globals.osArch)) {
	
		    if (je.isEnabled()) {

			boolean jreMatch =
			    (location == null && je.isPlatformMatch(vs)) ||
			    (location != null && je.isProductMatch(location, vs));
			boolean pathMatch = je.isCurrentRunningJRE();
			boolean heapMatch = JnlpxArgs.isCurrentRunningJREHeap(
									      jd.getMinHeap(), jd.getMaxHeap());
			
			if (jreMatch && pathMatch && heapMatch) {
			    if (Globals.TraceStartup) {
				Debug.println("LaunchSelection: findJRE: Match on current JRE");
			    }
			    // Match on current JRE!
			    selectedJRE[0] = je;
			    selectedJREDesc[0] = jd ;
			    return;  // We are done
			} else if (jreMatch && (selectedJRE[0] == null)) {
			    // Match, but not on current. Remember the first match ,
			    // and keep scanning to see if we get a
			    // match on current running JRE
			    if (Globals.TraceStartup) {
				Debug.print("LaunchSelection: findJRE: No match on current JRE because ");
				if (!jreMatch) Debug.print("versions dont match, ");
				if (!pathMatch) Debug.print("paths dont match, ");
				if (!heapMatch) Debug.print("heap sizes dont match");
				Debug.println("");
			    }
			    
			    selectedJRE[0] = je;
			    selectedJREDesc[0] = jd ;
			}
		    }
		}
	    }
	}
	// Always remember the first one
	if (selectedJREDesc[0] == null) selectedJREDesc[0] = jd;
    }
}




