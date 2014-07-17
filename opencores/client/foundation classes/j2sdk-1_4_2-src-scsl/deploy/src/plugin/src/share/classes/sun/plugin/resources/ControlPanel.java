/*
 * @(#)ControlPanel.java	1.75 03/04/10
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * US English verison of ControlPanel strings.
 *
 * @author Graham Hamilton
 */

public class ControlPanel extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "advanced.jre_format", "JRE {0} in {1}" },
	{ "advanced.jdk_format", "SDK {0} in {1}" },
	{ "panel.about", "About" }, 
	{ "panel.basic", "Basic" },
	{ "panel.advanced", "Advanced" },
	{ "panel.browser", "Browser" },
	{ "panel.proxies", "Proxies" },
	{ "panel.cache", "Cache" },
	{ "panel.cert", "Certificates" },
	{ "panel.update", "Update" },
	{ "panel.apply", "Apply" },
        { "panel.apply.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "panel.cancel", "Reset" },
	{ "panel.cancel.acceleratorKey", new Integer(KeyEvent.VK_R)},
        { "panel.apply_failed", "Failed to write property file" },
	{ "panel.apply_failed_title", "Apply Failed" },
	{ "panel.help", "Help" },
	{ "panel.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "panel.help_title", "Help - Java Plug-in Control Panel" },
	{ "panel.help_close", "Close" },
	{ "panel.help_close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "panel.help.error.text", "<html><b>File Doesn't Exist</b></html>Unable to load help file.\n" },
	{ "panel.help.error.caption", "Error - Java Plug-in Control Panel" },

	{ "basic.show_exception", "Show Exception Dialog Box" },
	{ "basic.recycle_classloader", "Recycle Classloader" },
        { "basic.java_console", "Java Console" },
        { "basic.show_console", "Show console" },
        { "basic.hide_console", "Hide console" },
        { "basic.no_console", "Do not start console" },
        { "basic.show_systray", "Show Java in System Tray" },
        
	{ "advanced.jre_name", "Java Runtime Environment" },
	{ "advanced.path", "Other SDK/JRE " },
	{ "advanced.other_jdk", "Other ..." },
	{ "advanced.default_jdk", "Use Java Plug-in Default" },
	{ "advanced.jre_selection_warning.info", "<html><b>Unsupported Operation</b></html>Selecting a Java Runtime other than \"Default\" is not recommended.\n"},
	{ "advanced.jre_selection_warning.caption", "Warning - Advanced"},
        { "advanced.error.caption", "Error - Advanced" },
        { "advanced.error.text", "<html><b>Directory Doesn't Exist</b></html>Please make sure the selection is not a file or non-existing directory.\n" },
	{ "advanced.java_parms", "Java Runtime Parameters" },
	{ "advanced.warning_popup_ok", "OK" },
	{ "advanced.warning_popup_cancel", "Cancel" },

	{ "browser.settings", "Settings" },
	{ "browser.desc.text", "Java(TM) Plug-in will be used as the default Java Runtime in the following browser(s):" },
	{ "browser.ie.text", "Microsoft Internet Explorer" },
	{ "browser.ns6.text", "Netscape 6 or later" },
        { "browser.moz.text", "Mozilla 1.1 or later"},

	{ "browser.settings.success.caption", "Success - Browser" },
	{ "browser.settings.fail.caption", "Warning - Browser" },

	{ "browser.settings.success.text", "<html><b>Browser Settings Changed</b></html>Changes will be in effect after restart of browser(s).\n" },

	{ "browser.settings.fail.ie.text", "<html><b>Unable to Change Browser Settings</b></html>Please check if you have "
					+ "sufficient permissions to change system settings.\n" },

	{ "browser.settings.fail.ns6.text", "<html><b>Unable to Change Browser Settings</b></html>"
					+ "Please check that Netscape 6 is properly installed on the system and/or "
					+ "that you have "
					+ "sufficient permissions to change system settings.\n" },

        { "browser.settings.fail.moz.text", "<html><b>Unable to change Browser Settings</b></html>"
                                        + "Please check that Mozilla is properly installed on the system and/or "
                                        + "that you have "
                                        + "sufficient permissions to change system settings.\n" },

	{ "browser.settings.alert.text", "<html><b>Newer Java Runtime exists</b></html>Internet Explorer already has a newer version of Java Runtime. Would you like to replace it?\n" },

	{ "proxy.use_browser", "Use Browser Settings" },
	{ "proxy.proxy_settings", "Proxy Settings" },
	{ "proxy.protocol_type", "Type" },
	{ "proxy.proxy_protocol", "Protocol" },
	{ "proxy.proxy_address", "Address" },
	{ "proxy.proxy_port", "Port" },
	{ "proxy.http", "HTTP" },
	{ "proxy.ftp", "FTP" },
	{ "proxy.gopher", "Gopher" },
	{ "proxy.https", "Secure" },
	{ "proxy.socks", "Socks" },
	{ "proxy.same_for_all_protocols", "Same proxy server for all protocols" },
	{ "proxy.bypass", "No Proxy Host (Use comma to separate multiple hosts)" },
	{ "proxy.autourl", "Automatic Proxy Configuration URL" },

	{ "cert.remove_button", "Remove" },
	{ "cert.remove_button.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "cert.import_button", "Import" },
	{ "cert.import_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
        { "cert.export_button", "Export" },
	{ "cert.export_button.acceleratorKey", new Integer(KeyEvent.VK_E)},
        { "cert.details_button", "Details" },
	{ "cert.details_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "cert.viewcert_button", "View Certificate" },
	{ "cert.viewcert_button.acceleratorKey", new Integer(KeyEvent.VK_V)},
        { "cert.rbutton_signed_applet", "Signed Applet" },
	{ "cert.rbutton_secure_site", "Secure Site" },
	{ "cert.rbutton_signer_ca", "Signer CA" },
	{ "cert.rbutton_secure_site_ca", "Secure Site CA" },
	{ "cert.SignedApplet_value", "SignedApplet" },
	{ "cert.SecureSite_value", "SecureSite" },
	{ "cert.SignerCA_value", "SignerCA" },
	{ "cert.SecureSiteCA_value", "SecureSiteCA" },
	{ "cert.settings", "Certificates" },
        { "cert.dialog.import.error.caption", "Error - Import Certificate" },
	{ "cert.dialog.import.format.text", "<html><b>Unrecognized File Format</b></html>No certificate will be imported." },
	{ "cert.dialog.import.file.text", "<html><b>File Doesn't Exist</b></html>No certificate will be imported." },
	{ "cert.dialog.import.password.text", "<html><b>Invalid Password</b></html>The password you entered is incorrect." },
	{ "cert.dialog.password.caption", "Password - Import" },
	{ "cert.dialog.password.text", "<html><b>Enter a password to access this file:<b></html>" },
	{ "cert.dialog.password.okButton", "OK" },
	{ "cert.dialog.password.cancelButton", "Cancel" },
	{ "cert.dialog.export.error.caption", "Error - Export Certificate" },
	{ "cert.dialog.export.text", "<html><b>Unable to Export</b></html>No certificate is exported." },

	{ "main.control_panel_caption", "Java(TM) Plug-in Control Panel" },

	// Each line in the property_file_header must start with "#"
	{ "config.property_file_header", "# Java(TM) Plug-in Properties\n"
			+ "# DO NOT EDIT THIS FILE.  It is machine generated.\n"
			+ "# Use the Activator Control Panel to edit properties." },
	{ "config.unknownSubject", "Unknown Subject" },
	{ "config.unknownIssuer", "Unknown Issuer" },
	{ "config.certShowName", "{0} ({1})" },
	{ "config.certShowOOU", "{0} {1}" },
	{ "config.proxy.autourl.invalid.text", "<html><b>Malformed URL</b></html>Automatic proxy configuration URL is invalid." },
	{ "config.proxy.autourl.invalid.caption", "Error - Proxies" },

	{ "jarcache.location", "Location" },
	{ "jarcache.select", "Select" },
	{ "jarcache.kb", "KB" },
	{ "jarcache.bytes", "bytes" },
	{ "jarcache.clear", "Clear" },
        { "jarcache.clear.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "jarcache.view", "View" },
	{ "jarcache.view.acceleratorKey", new Integer(KeyEvent.VK_V) },
        { "jarcache.browseDirectory.acceleratorKey", new Integer(KeyEvent.VK_O) },
        { "jarcache.no_compression", "None" },
	{ "jarcache.select_tooltip", "Use selected location" },
	{ "jarcache.select_mnemonic", "S" },
	{ "jarcache.maximum", "Maximum" },
	{ "jarcache.unlimited", "Unlimited" },
	{ "jarcache.high_compression", "High" },
	{ "jarcache.compression", "Jar Compression" },
	{ "jarcache.mb", "MB" },
	{ "jarcache.size", "Size" },
	{ "jarcache.settings", "Cache Settings" },
	{ "jarcache.erase.confirm.caption", "Confirmation Needed - Cache" },
	{ "jarcache.erase.confirm.text", "Erase all files in {0}?" },
	{ "jarcache.select_title", "Cache Location" },
	{ "jarcache.enabled", "Enable Caching" },    

	{ "update.update_button.text", "Update Now" },
	{ "update.advanced_button.text", "Advanced..." },
	{ "update.desc.text", "<html>The Java(TM) Update mechanism ensures you have the most updated version of the Java platform.<br>The options below allow you to control the process for how updates are obtained and applied.</html>" },
        { "update.notify.text", "Notify Me:" },
        { "update.notify_install.text", "Before installing" },
        { "update.notify_download.text", "Before downloading and before installing" },
        { "update.autoupdate.text", "Check for Updates Automatically" },
        { "update.advanced_title.text", "Automatic Update Advanced Settings" },
        { "update.advanced_title1.text", "Select how often and when you want the scan to occur." },
        { "update.advanced_title2.text", "Frequency" },
        { "update.advanced_title3.text", "When" },
        { "update.advanced_desc1.text", "Perform scan every day at {0}" },
        { "update.advanced_desc2.text", "Perform scan every {0} at {1}" },
        { "update.advanced_desc3.text", "Perform scan on day {0} of every month at {1}" },
        { "update.check_daily.text", "Daily" },
        { "update.check_weekly.text", "Weekly" },
        { "update.check_monthly.text", "Monthly" },
        { "update.check_date.text", "Day:" },
        { "update.check_day.text", "Every:" },
        { "update.check_time.text", "At:" },
        { "update.lastrun.text", "Java Update was last run at {0} on {1}." },
        { "update.desc_autooff.text", "<html>You must use the \"Update Now\" button below to check for updates.<br></html>" },
        { "update.desc_check_daily.text", "<html>Every day at {0}" },
        { "update.desc_check_weekly.text", "<html>Every {0} at {1}" },
        { "update.desc_check_monthly.text", "<html>On the day {0} of each month at {1}" },
        { "update.desc_check.text", ", Java Update will check for updates. " },
        { "update.desc_notify.text", "If new updates <br>are discovered, you will be notified before the update is " },
        { "update.desc_notify_install.text", "installed.</html>" },
        { "update.desc_notify_download.text", "downloaded and before it is installed.</html>" },
	{ "update.launchbrowser.error.text", "<html><b>Unable to Launch Java Update Checker</b></html>To obtain the latest Java(TM) Update, please goto http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "Error - Update" },
    };
}

