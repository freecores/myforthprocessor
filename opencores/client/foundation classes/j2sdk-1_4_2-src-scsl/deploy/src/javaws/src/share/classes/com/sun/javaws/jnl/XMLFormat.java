/*
 * @(#)XMLFormat.java	1.38 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.javaws.xml.*;
import com.sun.javaws.util.GeneralUtil;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;
import java.io.ByteArrayInputStream;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Properties;
import java.util.Locale;
import java.util.StringTokenizer;
import com.sun.javaws.util.URLUtil;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.exceptions.MissingFieldException;
import com.sun.javaws.exceptions.JNLParseException;
import com.sun.javaws.exceptions.BadFieldException;

public class XMLFormat {
    
    public static LaunchDesc parse(byte[] bits) throws
        IOException, BadFieldException, MissingFieldException, JNLParseException {
        
        
        // Convert bytes to a source
        ByteArrayInputStream in = new ByteArrayInputStream(bits);
        XMLReader r = (XMLReader) XMLReader.createReader(in);
        String enc =  r.getEncoding();
        r.close();
        String source = new String(bits, enc);

        XMLNode root = (new XMLParser(source)).parse();
        
        InformationDesc info = null;
        ResourcesDesc resources = null;
        ApplicationDesc application = null;
        AppletDesc applet = null;
        LibraryDesc libraryDef = null;
        InstallerDesc installerDef = null;
        String internalCommand = null;
        
        // Check if we should launch the player
        if (root.getName().equals("player")) {
	    return LaunchDescFactory.buildPlayerLaunchDesc(source);
        }
        
        // Check that root element is a <jnlp> tag
        if (!root.getName().equals("jnlp")) {
	    throw new MissingFieldException(source, "<jnlp>");  }
        
        // Read <jnlp> attributes (path is empty, i.e., "")
        // (spec, version, codebase, href)
        String specVersion = XMLUtils.getAttribute(root, "", "spec", "1.0+");
        String version = XMLUtils.getAttribute(root, "", "version");
        // Make sure the codebase URL ends with a '/'.
        URL codebase = URLUtil.asPathURL(XMLUtils.getAttributeURL(source, root, "", "codebase"));
        // Get href for JNLP file
        URL href = XMLUtils.getAttributeURL(source, codebase, root, "", "href");
        
        // Read <security> attributes
        int security = LaunchDesc.SANDBOX_SECURITY;
        if (XMLUtils.isElementPath(root, "<security><all-permissions>")) {
	    security = LaunchDesc.ALLPERMISSIONS_SECURITY;
        } else if (XMLUtils.isElementPath(root, "<security><j2ee-application-client-permissions>")) {
	    security = LaunchDesc.J2EE_APP_CLIENT_SECURITY;
        }
        
        // Figure out which mode
        int type;
        if (XMLUtils.isElementPath(root, "<application-desc>")) {
	    type = LaunchDesc.APPLICATION_DESC_TYPE;
	    application = buildApplicationDesc(source, root);
        } else if (XMLUtils.isElementPath(root, "<component-desc>")) {
	    type = LaunchDesc.LIBRARY_DESC_TYPE;
	    libraryDef = new LibraryDesc();
        } else if (XMLUtils.isElementPath(root, "<installer-desc>")) {
	    type = LaunchDesc.INSTALLER_DESC_TYPE;
	    installerDef = buildInstallerDesc(source, codebase, root);
        } else if (XMLUtils.isElementPath(root, "<applet-desc>")) {
	    type = LaunchDesc.APPLET_DESC_TYPE;
	    applet = buildAppletDesc(source, codebase, root);
        } else {
	    throw new MissingFieldException(source, "<jnlp>(<application-desc>|<applet-desc>|<installer-desc>|<component-desc>)");
        }
        
        
        info = buildInformationDesc(source, codebase, root);
        resources = buildResourcesDesc(source, codebase, root, false);
        
        LaunchDesc launchDesc = new LaunchDesc(
	    specVersion,
	    codebase,
	    href,
	    version,
	    info,
	    security,
	    resources,
	    type,
	    application,
	    applet,
	    libraryDef,
	    installerDef,
	    internalCommand,
	    source);
        
        return launchDesc;
    }
    
    /** Create a combine informationDesc in the two informationDesc. The information present in id1
     *  overwrite the information present in id2
     */
    static private InformationDesc combineInformationDesc(InformationDesc id1, InformationDesc id2) {
        if (id1 == null) return id2;
        if (id2 == null) return id1;
        
        String title  = (id1.getTitle() != null) ? id1.getTitle() : id2.getTitle();
        String vendor = (id1.getVendor() != null) ? id1.getVendor() : id2.getVendor();
        URL    home   = (id1.getHome() != null) ? id1.getHome() :  id2.getHome();
        
        /** Copy descriptions */
        String[] descriptions = new String[InformationDesc.NOF_DESC];
        for(int i = 0; i < descriptions.length; i++) {
	    descriptions[i] = (id1.getDescription(i) != null) ? id1.getDescription(i) : id2.getDescription(i);
        }
        
        /** Icons */
        ArrayList iconList = new ArrayList();
        if (id2.getIcons() != null) iconList.addAll(Arrays.asList(id2.getIcons()));
        if (id1.getIcons() != null) iconList.addAll(Arrays.asList(id1.getIcons()));
        IconDesc[] icons = new IconDesc[iconList.size()];
        icons = (IconDesc[])iconList.toArray(icons);
        
        // If one of them say true, it is true
        boolean offline = id1.supportsOfflineOperation() || id1.supportsOfflineOperation() ;
        
        return new InformationDesc(title,
				   vendor,
				   home,
				   descriptions,
				   icons,
				   offline,
				   null);
    }
    
    /** Extract data from <information> tag */
    static private InformationDesc buildInformationDesc(final String source, final URL codebase, XMLNode root)
        throws MissingFieldException, BadFieldException {
        final ArrayList list = new ArrayList();
        
        // Iterates over all <information> nodes ignoring the type
        XMLUtils.visitElements(root, "<information>", new XMLUtils.ElementVisitor() {
		    public void visitElement(XMLNode e) throws BadFieldException, MissingFieldException {
			// Locale info. (null if not present)
			String[] localeList = GeneralUtil.getStringList(XMLUtils.getAttribute(e, "", "locale"));
			
			// Title, vendor, home
			String title  = XMLUtils.getElementContents(e, "<title>");
			String vendor = XMLUtils.getElementContents(e, "<vendor>");
			URL    home   = XMLUtils.getAttributeURL(source, codebase, e, "<homepage>", "href");
			
			// Descriptions
			String[] descriptions = new String[InformationDesc.NOF_DESC];
			descriptions[InformationDesc.DESC_DEFAULT] =
			    XMLUtils.getElementContentsWithAttribute(e, "<description>", "kind", "", null);
			descriptions[InformationDesc.DESC_ONELINE] =
			    XMLUtils.getElementContentsWithAttribute(e, "<description>", "kind", "one-line", null);
			descriptions[InformationDesc.DESC_SHORT] =
			    XMLUtils.getElementContentsWithAttribute(e, "<description>", "kind", "short", null);
			descriptions[InformationDesc.DESC_TOOLTIP] =
			    XMLUtils.getElementContentsWithAttribute(e, "<description>", "kind", "tooltip", null);
			
			// Icons
			final ArrayList iconList = new ArrayList();
			XMLUtils.visitElements(e, "<icon>", new XMLUtils.ElementVisitor() {
				    public void visitElement(XMLNode icon)
					throws MissingFieldException, BadFieldException {
					String kindStr = XMLUtils.getAttribute(icon, "", "kind", "");
					URL href = XMLUtils.getRequiredURL(source, codebase, icon, "", "href");
					String version = XMLUtils.getAttribute(icon, "", "version", null);
					int height = XMLUtils.getIntAttribute(source, icon, "", "height", 0);
					int width  = XMLUtils.getIntAttribute(source, icon, "", "width", 0);
					int depth  = XMLUtils.getIntAttribute(source, icon, "", "depth", 0);
					
					// Convert the size
					int kind   = IconDesc.ICON_KIND_DEFAULT;
					if (kindStr.equals("selected")) {
					    kind = IconDesc.ICON_KIND_SELECTED;
					} else if (kindStr.equals("disabled")) {
					    kind = IconDesc.ICON_KIND_DISABLED;
					} else if (kindStr.equals("rollover")) {
					    kind = IconDesc.ICON_KIND_ROLLOVER;
					} else if (kindStr.equals("splash")) {
					    kind = IconDesc.ICON_KIND_SPLASH;
					}
					iconList.add(new IconDesc(href, version, height, width, depth, kind));
				    }
				});
			
			IconDesc[] icons = new IconDesc[iconList.size()];
			icons = (IconDesc[])iconList.toArray(icons);
			
			InformationDesc info = new InformationDesc(
			    title,
			    vendor,
			    home,
			    descriptions,
			    icons,
			    XMLUtils.isElementPath(e, "<offline-allowed>"),
			    localeList);
			
			list.add(info);
		    }
		});
        
        /* Combine all information desc. information in a single one for
	 * the current locale using the following prorities:
	 *   1. locale == language_country_variant
	 *   2. locale == lauguage_country
	 *   3. locale == lauguage
	 *   4. no or empty locale
	 */
        InformationDesc normId = new InformationDesc(null, null,
						     null, null, null, false, null);
	
        for(int i = 0; i < list.size(); i++) {
	    InformationDesc id = (InformationDesc)list.get(i);
	    if (matchDefaultLocale(id.getLocaleList())) {
		normId = combineInformationDesc(id, normId);
	    }
        }
	
        // If no <information><title> tag was found, throw exception
        if (normId.getTitle() == null) {
	    throw new MissingFieldException(source, "<jnlp><information><title>");
        }
        // If no <information> tag was found, throw exception
        if (normId.getVendor() == null) {
	    throw new MissingFieldException(source, "<jnlp><information><vendor>");
        }
        
        return normId;
    }
    
    
    static public boolean matchDefaultLocale(String[] localeStr) {
	return GeneralUtil.matchLocale(localeStr, Globals.getLocale());
    }
    
    /** Extract data from <resources> tag. There is only one. */
    static final ResourcesDesc buildResourcesDesc(final String source, final URL codebase, XMLNode root, final boolean ignoreJres)
        throws MissingFieldException, BadFieldException {
        // Extract classpath directives
        final ResourcesDesc rdesc = new ResourcesDesc();
        
        // Iterate over all entries
        XMLUtils.visitElements(root, "<resources>", new XMLUtils.ElementVisitor() {
		    public void visitElement(XMLNode e) throws MissingFieldException, BadFieldException {
			// Check for right os, archictecture, and locale
			String[] os     = GeneralUtil.getStringList(XMLUtils.getAttribute(e, "", "os",   null));
			String[] arch   = GeneralUtil.getStringList(XMLUtils.getAttribute(e, "", "arch", null));
			String[] locale = GeneralUtil.getStringList(XMLUtils.getAttribute(e, "", "locale", null));
			if (GeneralUtil.prefixMatchStringList(os, Globals.getOperatingSystemID()) &&
			    GeneralUtil.prefixMatchStringList(arch, Globals.getArchitectureID()) &&
			    matchDefaultLocale(locale)) {
			    // Now visit all children in this node
			    XMLUtils.visitChildrenElements(e, new XMLUtils.ElementVisitor() {
					public void visitElement(XMLNode e2) throws MissingFieldException, BadFieldException {
					    handleResourceElement(source, codebase, e2, rdesc, ignoreJres);
					}
				    });
			}
		    }
		});
        return (rdesc.isEmpty()) ? null : rdesc;
    }
    
    /** Handle the individual entries in a resource desc */
    private static void handleResourceElement(String source, URL codebase, XMLNode e, ResourcesDesc rdesc, boolean ignoreJres)
        throws MissingFieldException, BadFieldException {
        String tag = e.getName();
        
        if (tag.equals("jar") || tag.equals("nativelib")) {
	    /*
	     * jar/nativelib elements
	     */
	    URL href =  XMLUtils.getRequiredURL(source, codebase, e, "", "href");
	    String vd = XMLUtils.getAttribute(e, "", "version", null);
	    String kindStr = XMLUtils.getAttribute(e, "", "download");
	    String mainStr = XMLUtils.getAttribute(e, "", "main");
	    String part    = XMLUtils.getAttribute(e, "", "part");
	    int    size    = XMLUtils.getIntAttribute(source, e, "", "size", 0);
	    boolean isNativeLib = tag.equals("nativelib");
	    boolean isLazy = false;
	    boolean isMain = false;
	    if ("lazy".equalsIgnoreCase(kindStr)) isLazy = true;
	    if ("true".equalsIgnoreCase(mainStr)) isMain = true;
	    rdesc.addResource(new JARDesc(href, vd, isLazy, isMain, isNativeLib, part, size, rdesc));
        } else if (tag.equals("property")) {
	    /*
	     *  property tag
	     */
	    String name  = XMLUtils.getRequiredAttribute(source, e, "", "name");
	    String value = XMLUtils.getRequiredAttributeEmptyOK(
				source, e, "", "value");
	    rdesc.addResource(new PropertyDesc(name, value));
        } else if (tag.equals("package")) {
	    /*
	     * package tag
	     */
	    String name = XMLUtils.getRequiredAttribute(source, e, "", "name");
	    String part = XMLUtils.getRequiredAttribute(source, e, "", "part");
	    String recursive = XMLUtils.getAttribute(e, "", "recursive", "false");
	    boolean isRecursive = "true".equals(recursive);
	    rdesc.addResource(new PackageDesc(name, part, isRecursive));
        } else if (tag.equals("extension")) {
	    String name = XMLUtils.getAttribute(e, "", "name");
	    URL href =  XMLUtils.getRequiredURL(source, codebase, e, "", "href");
	    String vd = XMLUtils.getAttribute(e, "", "version", null);
	    // Iterate through ext=download resources
	    ExtDownloadDesc[] eds = getExtDownloadDescs(source, e);
	    rdesc.addResource(new ExtensionDesc(name, href, vd, eds));
        } else if (tag.equals("j2se") && !ignoreJres) {
	    /*
	     * j2se element
	     */
	    String version  = XMLUtils.getRequiredAttribute(source, e, "", "version");
	    // Get optional href attribute (vendor specific URL)
	    URL href = XMLUtils.getAttributeURL(source, codebase, e, "", "href");
	    
	    String minheapstr = XMLUtils.getAttribute(e, "", "initial-heap-size");
	    String maxheapstr = XMLUtils.getAttribute(e, "", "max-heap-size");
	    
	    long minheap = -1;
	    long maxheap = -1;
	    minheap = GeneralUtil.heapValToLong(minheapstr);
	    maxheap = GeneralUtil.heapValToLong(maxheapstr);
	    
	    ResourcesDesc cbs = buildResourcesDesc(source, codebase, e, true);
	    
	    // JRE
	    JREDesc jreDesc = new JREDesc(
		version,
		minheap,
		maxheap,
		href,
		cbs);
	    
	    rdesc.addResource(jreDesc);
        }
    }
    
    /** Parses the ExtDownloadDesc[] elements */
    static private ExtDownloadDesc[] getExtDownloadDescs(final String source, XMLNode root)
        throws BadFieldException, MissingFieldException {
        final ArrayList al = new ArrayList();
        
        XMLUtils.visitElements(root, "<ext-download>", new XMLUtils.ElementVisitor() {
		    public void visitElement(XMLNode e) throws MissingFieldException {
			String extPart = XMLUtils.getRequiredAttribute(source, e, "", "ext-part");
			String part = XMLUtils.getAttribute(e, "", "part");
			String download = XMLUtils.getAttribute(e, "", "download", "eager");
			boolean isLazy = "lazy".equals(download);
			al.add(new ExtDownloadDesc(extPart, part, isLazy));
		    }
		});
        ExtDownloadDesc[] eds = new ExtDownloadDesc[al.size()];
        return (ExtDownloadDesc[])al.toArray(eds);
    }
    
    /** Extract data from the application-desc tag */
    static private ApplicationDesc buildApplicationDesc(final String source, XMLNode root) throws MissingFieldException, BadFieldException {
        String mainclass = XMLUtils.getAttribute(root, "<application-desc>", "main-class");
        
        final ArrayList al1 = new ArrayList();
        XMLUtils.visitElements(root, "<application-desc><argument>", new XMLUtils.ElementVisitor() {
		    public void visitElement(XMLNode e) throws MissingFieldException, BadFieldException {
			String arg = XMLUtils.getElementContents(e, "", null);
			if (arg == null) {
			    throw new BadFieldException(source, XMLUtils.getPathString(e), "");
			}
			
			al1.add(arg);
		    }
		});
        
        String[] args1 = new String[al1.size()];
        args1 = (String[])al1.toArray(args1);
        
        return new ApplicationDesc(mainclass, args1);
    }
    
    /** Extract data from the extension-desc tag */
    static private InstallerDesc buildInstallerDesc(String source, URL codebase, XMLNode root)
        throws MissingFieldException, BadFieldException {
        String mainclass = XMLUtils.getAttribute(root, "<installer-desc>", "main-class");
        return new InstallerDesc(mainclass);
    }
    
    /** Extract data from the applet tag */
    static private AppletDesc buildAppletDesc(final String source, URL codebase, XMLNode root) throws MissingFieldException, BadFieldException {
        String appletclass  = XMLUtils.getRequiredAttribute(source, root, "<applet-desc>", "main-class");
        String name         = XMLUtils.getRequiredAttribute(source, root, "<applet-desc>", "name");
        URL    documentBase = XMLUtils.getAttributeURL(source, codebase, root, "<applet-desc>", "documentbase");
        int    width        = XMLUtils.getRequiredIntAttribute(source, root, "<applet-desc>", "width");
        int    height       = XMLUtils.getRequiredIntAttribute(source, root, "<applet-desc>", "height");
        
        if (width  <= 0) throw new BadFieldException(source, XMLUtils.getPathString(root)+"<applet-desc>width",   new Integer(width).toString());
        if (height <= 0) throw new BadFieldException(source, XMLUtils.getPathString(root)+"<applet-desc>height",  new Integer(height).toString());
        
        final Properties params = new Properties();
        
        XMLUtils.visitElements(root, "<applet-desc><param>", new XMLUtils.ElementVisitor() {
		    public void visitElement(XMLNode e) throws MissingFieldException, BadFieldException {
			String pn = XMLUtils.getRequiredAttribute(
					source, e, "", "name");
			String pv = XMLUtils.getRequiredAttributeEmptyOK(
					source, e, "", "value");
			params.setProperty(pn, pv);
		    }
		});
        
        return new AppletDesc(name, appletclass, documentBase, width, height, params);
    }
}

