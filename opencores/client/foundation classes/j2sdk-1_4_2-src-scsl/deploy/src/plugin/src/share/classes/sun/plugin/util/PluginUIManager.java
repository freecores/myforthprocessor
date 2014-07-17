/*
 * @(#)PluginUIManager.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

/**
  * Customized metal theme to be used in Java Plug-in.
  *
  * @author Stanley Man-Kit Ho
  */

import java.awt.Font;
import java.util.WeakHashMap;
import javax.swing.LookAndFeel;
import javax.swing.UIManager;
import javax.swing.plaf.FontUIResource;
import javax.swing.plaf.metal.DefaultMetalTheme;
import javax.swing.plaf.metal.MetalLookAndFeel;
import javax.swing.plaf.metal.MetalTheme;
import sun.awt.AppContext;

public class PluginUIManager 
{
    static class PluginMetalTheme extends DefaultMetalTheme
    {
	private FontUIResource controlTextFont = null;
	private FontUIResource menuTextFont = null;
	private FontUIResource windowTitleFont = null;

	PluginMetalTheme()
	{
	    FontUIResource resource = super.getControlTextFont();

	    controlTextFont = new FontUIResource(resource.getName(),
						 resource.getStyle() & ~(Font.BOLD),
						 resource.getSize());

	    resource = super.getMenuTextFont();

	    menuTextFont = new FontUIResource(resource.getName(),
					      resource.getStyle() & ~(Font.BOLD),
					      resource.getSize());

	    resource = super.getWindowTitleFont();

	    windowTitleFont = new FontUIResource(resource.getName(),
						 resource.getStyle() & ~(Font.BOLD),
						 resource.getSize());
	}

	public FontUIResource getControlTextFont() 
	{
	    return controlTextFont;
	}
	public FontUIResource getMenuTextFont() 
	{
	    return menuTextFont;
	}
	public FontUIResource getWindowTitleFont() 
	{
	    return windowTitleFont;
	}
    }

    // Weak hash map to store theme information 
    // for metal look and feel.
    //
    private static WeakHashMap themeMap = new WeakHashMap();

    /**
     * Change metal theme to plugin customized theme
     */
    public static LookAndFeel setTheme()
    {
	LookAndFeel lookAndFeel = UIManager.getLookAndFeel();

	// We should store the current theme for the MetalLookAndFeel,
	// so we may restore it properly afterwards - developers may
	// have set the theme to something other than default, so it
	// is very important to restore to the previous state afterwards.
	//
	if (lookAndFeel instanceof MetalLookAndFeel)
	    themeMap.put(lookAndFeel, getCurrentMetalTheme());

	try
	{
	    // Change metal theme - make all fonts PLAIN instead of
	    // BOLD.
	    //
	    MetalLookAndFeel.setCurrentTheme(new PluginMetalTheme());
	    UIManager.setLookAndFeel(new MetalLookAndFeel());
	}
	catch(Exception e)
	{
	}

	return lookAndFeel;
    }


    /**
     * Restore metal theme to default one
     */
    public static void restoreTheme(LookAndFeel lookAndFeel)
    {
	try
	{
	    // We should restore the previous theme for the MetalLookAndFeel -
	    // developers may have set the theme to something other than default, 
	    // so it is very important to restore to the previous state.
	    //
	    if (lookAndFeel instanceof MetalLookAndFeel)
	    {
		MetalTheme theme = (MetalTheme) themeMap.get(lookAndFeel);

		if (theme != null)
		    MetalLookAndFeel.setCurrentTheme(theme);
		else
		    MetalLookAndFeel.setCurrentTheme(new DefaultMetalTheme());
	    }	

	    if (lookAndFeel == null)
		UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
	    else 
		UIManager.setLookAndFeel(lookAndFeel);
	}
	catch(Exception e)
	{
	}
    }

    // This is a method copied from MetalLookAndFeel to obtain 
    // the current metal theme - too bad it is not public in Swing.
    //
    private static MetalTheme getCurrentMetalTheme() 
    {
        AppContext context = AppContext.getAppContext();

	return (MetalTheme)context.get("currentMetalTheme");
    }
}
