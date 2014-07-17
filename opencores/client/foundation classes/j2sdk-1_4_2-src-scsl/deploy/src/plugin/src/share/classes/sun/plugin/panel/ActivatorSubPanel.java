/*
 * @(#)ActivatorSubPanel.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/*
 * A superclass for all sub panels displayed in the Activator Control
 * Panel.
 *
 * @version 	1.1
 * @author	Jerome Dochez
 */

import javax.swing.JPanel;
import sun.plugin.util.Trace;

abstract public class ActivatorSubPanel extends JPanel {

    /**
     * Construct a new subpanel
     * 
     * @param model  model 
     */
    public ActivatorSubPanel(ConfigurationInfo model) {
	this.model = model;
    }	

    /** 
     * @return the PlatformDependentInterface object responsible 
     * for implementing all platform dependent code for the 
     * Activator Control Panel.
     * The name of the class implementing this platform specific
     * behaviour is sun.plugin.panel.<os.name>Platform
     * <p>
     * The platform dependent object will be called with the
     * onLoad, onSave method
     */
    static PlatformDependentInterface getPlatformObject() {

    	String osName = System.getProperty("os.name");
	try {
	    String n = osName;
	    if (osName.indexOf(' ')!=-1)
		n = osName.substring(0, osName.indexOf(' '));		
	    String platformClassName = "sun.plugin.panel." +  n + "Platform";
	    java.lang.Class platformClass = Class.forName(platformClassName);
	    if (platformClass != null) {
		Object pc = platformClass.newInstance();
		if (pc instanceof PlatformDependentInterface) {
		    PlatformDependentInterface pdi = (PlatformDependentInterface) pc;
		    return pdi;
		}
	    }
	} catch(Exception e) 
	{
	    Trace.printException(e);
	}
	return null;
    }

    protected ConfigurationInfo model;
}

