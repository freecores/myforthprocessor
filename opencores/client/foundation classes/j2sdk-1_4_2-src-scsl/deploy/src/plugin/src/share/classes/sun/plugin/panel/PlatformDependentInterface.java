/*
 * @(#)PlatformDependentInterface.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.panel;

interface PlatformDependentInterface 
{
    public void init();

    public void onSave(ConfigurationInfo info);

    public void onLoad(ConfigurationInfo info);

    public boolean showURL(String url);
}

