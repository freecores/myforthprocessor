/*
 * @(#)LinuxPlatform.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/*
 * linux specific code to be invoked when properties are
 * being read in or written out.
 */
  
import java.io.*;
import java.util.StringTokenizer;
import java.util.Vector;

class LinuxPlatform implements PlatformDependentInterface {

    public void init() {
    }

    public void onSave(ConfigurationInfo info) {
    }

    public void onLoad(ConfigurationInfo info) {
        setActivatorJRE(info);
    }

    private void setActivatorJRE(ConfigurationInfo info) {
	// Set the javaplugin JRE in the JRE list.

	String home = System.getProperty("user.home");
	// new algorithm for determining the installed JRE's: find out where
	// the javaplugin.jar came from (that's where the plugin was installed
	// and then user's home directory.

	// Modified to search for rt.jar instead of javaplugin.jar, because
	// the plugin and the JRE can be separated, as in Solaris 8.

	String classpath = System.getProperty("java.class.path");
	String bootclasspath = System.getProperty("sun.boot.class.path");

	if (classpath == null)
	    classpath = bootclasspath;
	else
	    classpath = classpath + File.pathSeparator + bootclasspath;

	StringTokenizer parser = new StringTokenizer(classpath,
						     File.pathSeparator);
	String path = "";
	while (parser.hasMoreElements()) {
	    path = (String) parser.nextElement();
	    if (path.endsWith("rt.jar"))
		break;
	}
	int index = path.lastIndexOf(File.separatorChar);
	path = path.substring(0, index);
	index = path.lastIndexOf(File.separatorChar);
	path = path.substring(0, index);
	
	Vector vector = new Vector();
	vector.addElement(System.getProperty("java.version"));
	vector.addElement(path);

	info.setInstalledJREList(vector);

	// Initialize the JVM currently selected
	String javaRuntimeType = info.getJavaRuntimeType();
	if (javaRuntimeType != null && !javaRuntimeType.equalsIgnoreCase("Default")) {
	    // If the currently selected VM is not default. it may
	    // be one of the installed and recongized JREs or JDKs
	    String[][] list = info.getInstalledJREList();
	    if (list != null) {
		for (int i=0;i<list.length;i++) {
		    if (list[i][1].equalsIgnoreCase(javaRuntimeType)) {
			info.setJavaRuntimeType("JRE");
		    }
		}
	    }
	    list = info.getInstalledJDKList();
	    if (list != null) {
		for (int i=0; i<list.length; i++) {
		    if (list[i][1].equalsIgnoreCase(javaRuntimeType)) {
			info.setJavaRuntimeType("JDK");
		    }
		}
	    }
	}
	
    }

    public boolean showURL(String url)
    {
	// no-op
	return true;
    }
}
