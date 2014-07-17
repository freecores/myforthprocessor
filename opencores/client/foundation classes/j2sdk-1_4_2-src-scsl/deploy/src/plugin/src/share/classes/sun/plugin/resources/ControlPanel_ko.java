/*
 * @(#)ControlPanel_ko.java	1.40 03/04/25
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Korean verison of ControlPanel strings.
 *
 * @author Graham Hamilton
 */

public class ControlPanel_ko extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "advanced.jre_format", "{1}\uc758 JRE {0}" },
	{ "advanced.jdk_format", "{1}\uc758 SDK {0}" },
	{ "panel.about", "\uc815\ubcf4" }, 
	{ "panel.basic", "\uae30\ubcf8" },
	{ "panel.advanced", "\uace0\uae09" },
	{ "panel.browser", "\ube0c\ub77c\uc6b0\uc800" },
	{ "panel.proxies", "\ud504\ub85d\uc2dc" },
	{ "panel.cache", "\uce90\uc2dc" },
	{ "panel.cert", "\uc778\uc99d\uc11c" },
	{ "panel.update", "\uac31\uc2e0" },
	{ "panel.apply", "\uc801\uc6a9 (A)" },
        { "panel.apply.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "panel.cancel", "\uc7ac\uc124\uc815 (R)" },
	{ "panel.cancel.acceleratorKey", new Integer(KeyEvent.VK_R)},
        { "panel.apply_failed", "\uc18d\uc131 \ud30c\uc77c \uc791\uc131 \uc2e4\ud328" },
	{ "panel.apply_failed_title", "\uc801\uc6a9 \uc2e4\ud328" },
	{ "panel.help", "\ub3c4\uc6c0\ub9d0 (H)" },
	{ "panel.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "panel.help_title", "\ub3c4\uc6c0\ub9d0 - Java Plug-in \uc81c\uc5b4\ud310" },
	{ "panel.help_close", "\ub2eb\uae30 (C)" },
	{ "panel.help_close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "panel.help.error.text", "<html><b>\ud30c\uc77c\uc774 \uc5c6\uc74c</b></html>\ub3c4\uc6c0\ub9d0 \ud30c\uc77c\uc744 \ub85c\ub4dc\ud560 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4.\n" },
	{ "panel.help.error.caption", "\uc624\ub958 - Java Plug-in \uc81c\uc5b4\ud310" },

	{ "basic.show_exception", "\uc608\uc678 \ub300\ud654 \uc0c1\uc790 \ud45c\uc2dc" },
	{ "basic.recycle_classloader", "\ud074\ub798\uc2a4\ub85c\ub354 \uc7ac\uc21c\ud658" },
        { "basic.java_console", "Java \ucf58\uc194" },
        { "basic.show_console", "\ucf58\uc194 \ud45c\uc2dc" },
        { "basic.hide_console", "\ucf58\uc194 \uc228\uae30\uae30" },
        { "basic.no_console", "\ucf58\uc194\uc744 \uc2dc\uc791\ud558\uc9c0 \ub9c8\uc2ed\uc2dc\uc624" },
        { "basic.show_systray", "\uc2dc\uc2a4\ud15c \ud2b8\ub808\uc774\uc5d0 Java \ud45c\uc2dc" },
        
	{ "advanced.jre_name", "Java Runtime Environment" },
	{ "advanced.path", "\ub2e4\ub978 SDK/JRE " },
	{ "advanced.other_jdk", "\uae30\ud0c0 ..." },
	{ "advanced.default_jdk", "Java Plug-in\uc744 \uae30\ubcf8\uac12\uc73c\ub85c \uc0ac\uc6a9" },
	{ "advanced.jre_selection_warning.info", "<html><b>\uc9c0\uc6d0\ub418\uc9c0 \uc54a\ub294 \uc791\uc5c5</b></html>\"\uae30\ubcf8\uac12\"\uc678\uc758 Java Runtime\uc744 \uc120\ud0dd\ud558\uc9c0 \ub9c8\uc2ed\uc2dc\uc624.\n"},
	{ "advanced.jre_selection_warning.caption", "\uacbd\uace0 - \uace0\uae09"},
        { "advanced.error.caption", "\uc624\ub958 - \uace0\uae09" },
        { "advanced.error.text", "<html><b>\ub514\ub809\ud1a0\ub9ac \uc5c6\uc74c</b></html>\uc120\ud0dd\uc0ac\ud56d\uc774 \ud30c\uc77c\uc774 \uc544\ub2c8\uac70\ub098 \ub514\ub809\ud1a0\ub9ac\uac00 \uc874\uc7ac\ud558\uc9c0 \uc54a\ub294 \uc9c0\ub97c \ud655\uc778\ud558\uc2ed\uc2dc\uc624.\n" },
	{ "advanced.java_parms", "Java \ub7f0\ud0c0\uc784 \ub9e4\uac1c\ubcc0\uc218" },
	{ "advanced.warning_popup_ok", "\ud655\uc778" },
	{ "advanced.warning_popup_cancel", "\ucde8\uc18c" },

	{ "browser.settings", "\uc124\uc815" },
	{ "browser.desc.text", "Java(TM) Plug-in\uc774 \ub2e4\uc74c \ube0c\ub77c\uc6b0\uc800\uc5d0\uc11c \uae30\ubcf8 Java \ub7f0\ud0c0\uc784\uc73c\ub85c \uc0ac\uc6a9\ub429\ub2c8\ub2e4." },
	{ "browser.ie.text", "Microsoft Internet Explorer" },
	{ "browser.ns6.text", "Netscape 6 \uc774\uc0c1" },
        { "browser.moz.text", "Mozilla 1.1 \uc774\uc0c1"},

	{ "browser.settings.success.caption", "\uc131\uacf5 - \ube0c\ub77c\uc6b0\uc800" },
	{ "browser.settings.fail.caption", "\uacbd\uace0 - \ube0c\ub77c\uc6b0\uc800" },

	{ "browser.settings.success.text", "<html><b>\ube0c\ub77c\uc6b0\uc800 \uc124\uc815 \ubcc0\uacbd\ub428</b></html>\ube0c\ub77c\uc6b0\uc800\uac00 \ub2e4\uc2dc \uc2dc\uc791\ud55c \ud6c4\uc5d0\uc57c \ubcc0\uacbd\uc0ac\ud56d\uc774 \uc801\uc6a9\ub429\ub2c8\ub2e4.\n" },

	{ "browser.settings.fail.ie.text", "<html><b>\ube0c\ub77c\uc6b0\uc800 \uc124\uc815 \ubcc0\uacbd\ud560 \uc218 \uc5c6\uc74c</b></html>\uc2dc\uc2a4\ud15c \uc124\uc815 \ubcc0\uacbd\uc5d0 \ub300\ud55c "
					+ "\uad8c\ud55c\uc774 \uc788\ub294\uc9c0 \ud655\uc778\ud558\uc2ed\uc2dc\uc624.\n" },

	{ "browser.settings.fail.ns6.text", "<html><b>\ube0c\ub77c\uc6b0\uc800 \uc124\uc815 \ubcc0\uacbd\ud560 \uc218 \uc5c6\uc74c</b></html>"
					+ "Netscape 6\uac00 \uc2dc\uc2a4\ud15c\uc5d0 \uc801\uc808\ud788 \uc124\uce58\ub418\uc5b4 \uc788\uace0 \uc2dc\uc2a4\ud15c \uc124\uc815 \ubcc0\uacbd \uad8c\ud55c\uc774 "
					+ "\uc788\ub294\uc9c0 "
					+ "\ud655\uc778\ud558\uc2ed\uc2dc\uc624.\n" },

        { "browser.settings.fail.moz.text", "<html><b>\ube0c\ub77c\uc6b0\uc800 \uc124\uc815 \ubcc0\uacbd\ud560 \uc218 \uc5c6\uc74c</b></html>"
                                        + "Mozilla\uac00 \uc2dc\uc2a4\ud15c\uc5d0 \uc801\uc808\ud788 \uc124\uce58\ub418\uc5b4 \uc788\uace0 \uc2dc\uc2a4\ud15c \uc124\uc815 \ubcc0\uacbd \uad8c\ud55c\uc774 "
                                        + "\uc788\ub294\uc9c0 "
                                        + "\ud655\uc778\ud558\uc2ed\uc2dc\uc624.\n" },

	{ "browser.settings.alert.text", "<html><b>\ub354 \ucd5c\uc2e0 Java Runtime \uc788\uc74c</b></html>Internet Explorer\uac00 \uc774\ubbf8 \ub354 \ucd5c\uc2e0 \ubc84\uc804\uc758 Java Runtime\uc744 \uac00\uc9c0\uace0 \uc788\uc2b5\ub2c8\ub2e4. \ub300\uccb4\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?\n" },

	{ "proxy.use_browser", "\ube0c\ub77c\uc6b0\uc800 \uc124\uc815 \uc0ac\uc6a9" },
	{ "proxy.proxy_settings", "\ud504\ub85d\uc2dc \uc124\uc815" },
	{ "proxy.protocol_type", "\uc720\ud615" },
	{ "proxy.proxy_protocol", "\ud504\ub85c\ud1a0\ucf5c" },
	{ "proxy.proxy_address", "\uc8fc\uc18c" },
	{ "proxy.proxy_port", "\ud3ec\ud2b8" },
	{ "proxy.http", "HTTP" },
	{ "proxy.ftp", "FTP" },
	{ "proxy.gopher", "Gopher" },
	{ "proxy.https", "Secure" },
	{ "proxy.socks", "Socks" },
	{ "proxy.same_for_all_protocols", "\ubaa8\ub4e0 \ud504\ub85c\ud1a0\ucf5c\uc5d0 \ub300\ud574 \ub3d9\uc77c\ud55c \ud504\ub85d\uc2dc \uc11c\ubc84" },
	{ "proxy.bypass", "\ud504\ub85d\uc2dc \ud638\uc2a4\ud2b8 \uc5c6\uc74c(\uc5ec\ub7ec \ud638\uc2a4\ud2b8\ub294 \uc27c\ud45c\ub85c \uad6c\ubd84)" },
	{ "proxy.autourl", "\uc790\ub3d9 \ud504\ub85d\uc2dc \uad6c\uc131 URL" },

	{ "cert.remove_button", "\uc81c\uac70 (M)" },
	{ "cert.remove_button.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "cert.import_button", "\uac00\uc838\uc624\uae30 (I)" },
	{ "cert.import_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
        { "cert.export_button", "\ub0b4\ubcf4\ub0b4\uae30 (E)" },
	{ "cert.export_button.acceleratorKey", new Integer(KeyEvent.VK_E)},
        { "cert.details_button", "\uc790\uc138\ud788 (D)" },
	{ "cert.details_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "cert.viewcert_button", "\uc778\uc99d\uc11c \ubcf4\uae30 (V)" },
	{ "cert.viewcert_button.acceleratorKey", new Integer(KeyEvent.VK_V)},
        { "cert.rbutton_signed_applet", "\uc11c\uba85\ub41c \uc560\ud50c\ub9bf" },
	{ "cert.rbutton_secure_site", "\ubcf4\uc548 \uc0ac\uc774\ud2b8" },
	{ "cert.rbutton_signer_ca", "\uc11c\uba85\uc790 CA" },
	{ "cert.rbutton_secure_site_ca", "\ubcf4\uc548 \uc0ac\uc774\ud2b8 CA" },
	{ "cert.SignedApplet_value", "SignedApplet" },
	{ "cert.SecureSite_value", "SecureSite" },
	{ "cert.SignerCA_value", "SignerCA" },
	{ "cert.SecureSiteCA_value", "SecureSiteCA" },
	{ "cert.settings", "\uc778\uc99d\uc11c" },
        { "cert.dialog.import.error.caption", "\uc624\ub958 - \uc778\uc99d\uc11c \uac00\uc838\uc624\uae30" },
	{ "cert.dialog.import.format.text", "<html><b>\ud30c\uc77c \ud3ec\ub9f7 \uc778\uc2dd\ud560 \uc218 \uc5c6\uc74c</b></html>\uc778\uc99d\uc11c\ub97c \uac00\uc838\uc624\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4." },
	{ "cert.dialog.import.file.text", "<html><b>\ud30c\uc77c \uc5c6\uc74c</b></html>\uc778\uc99d\uc11c\ub97c \uac00\uc838\uc624\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4." },
	{ "cert.dialog.import.password.text", "<html><b>\uc554\ud638 \uc720\ud6a8\ud558\uc9c0 \uc54a\uc74c</b></html>\uc785\ub825\ud55c \uc554\ud638\uac00 \uc62c\ubc14\ub974\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "cert.dialog.password.caption", "\uc554\ud638 - \uac00\uc838\uc624\uae30" },
	{ "cert.dialog.password.text", "<html><b>\ub2e4\uc74c \ud30c\uc77c\uc5d0 \uc561\uc138\uc2a4\ud560 \uc554\ud638 \uc785\ub825:<b></html>" },
	{ "cert.dialog.password.okButton", "\ud655\uc778" },
	{ "cert.dialog.password.cancelButton", "\ucde8\uc18c" },
	{ "cert.dialog.export.error.caption", "\uc624\ub958 - \uc778\uc99d\uc11c \ub0b4\ubcf4\ub0b4\uae30" },
	{ "cert.dialog.export.text", "<html><b>\ub0b4\ubcf4\ub0bc \uc218 \uc5c6\uc74c</b></html>\uc778\uc99d\uc11c\ub97c \ub0b4\ubcf4\ub0b4\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4." },

	{ "main.control_panel_caption", "Java(TM) Plug-in \uc81c\uc5b4\ud310" },

	// Each line in the property_file_header must start with "#"
	{ "config.property_file_header", "# Java(TM) Plug-in \ub4f1\ub85d \uc815\ubcf4\n"
			+ "# \uc774 \ud30c\uc77c\uc744 \ud3b8\uc9d1\ud558\uc9c0 \ub9c8\uc2ed\uc2dc\uc624.  \uc774\uac83\uc740 \uc2dc\uc2a4\ud15c\uc774 \uc791\uc131\ud55c \uac83\uc785\ub2c8\ub2e4.\n"
			+ "Activator \uc81c\uc5b4\ud310\uc744 \uc0ac\uc6a9\ud574\uc11c \ub4f1\ub85d \uc815\ubcf4\ub97c \ud3b8\uc9d1\ud558\uc2ed\uc2dc\uc624." },
	{ "config.unknownSubject", "\uc54c \uc218 \uc5c6\ub294 \uc8fc\uc81c" },
	{ "config.unknownIssuer", "\uc54c \uc218 \uc5c6\ub294 \ubc1c\ud589\uc778" },
	{ "config.certShowName", "{0} ({1})" },
	{ "config.certShowOOU", "{0} {1}" },
	{ "config.proxy.autourl.invalid.text", "<html><b>\uc798\ubabb\ub41c URL</b></html>\uc790\ub3d9 \ud504\ub85d\uc2dc \uad6c\uc131\uc774 \uc62c\ubc14\ub974\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "config.proxy.autourl.invalid.caption", "\uc624\ub958 - \ud504\ub85d\uc2dc" },

	{ "jarcache.location", "\uc704\uce58" },
	{ "jarcache.select", "\uc120\ud0dd" },
	{ "jarcache.kb", "KB" },
	{ "jarcache.bytes", "\ubc14\uc774\ud2b8" },
	{ "jarcache.clear", "\uc9c0\uc6b0\uae30 (C)" },
        { "jarcache.clear.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "jarcache.view", "\ubcf4\uae30 (V)" },
	{ "jarcache.view.acceleratorKey", new Integer(KeyEvent.VK_V) },
        { "jarcache.browseDirectory.acceleratorKey", new Integer(KeyEvent.VK_O) },
        { "jarcache.no_compression", "\uc5c6\uc74c" },
	{ "jarcache.select_tooltip", "\uc120\ud0dd\ub41c \uc704\uce58 \uc0ac\uc6a9(S)" },
	{ "jarcache.select_mnemonic", "S" },
	{ "jarcache.maximum", "\ucd5c\ub300" },
	{ "jarcache.unlimited", "\uc81c\ud55c \uc5c6\uc74c" },
	{ "jarcache.high_compression", "\ub192\uc74c" },
	{ "jarcache.compression", "Jar \uc555\ucd95" },
	{ "jarcache.mb", "MB" },
	{ "jarcache.size", "\ud06c\uae30" },
	{ "jarcache.settings", "\uce90\uc2dc \uc124\uc815" },
	{ "jarcache.erase.confirm.caption", "\ud655\uc778 \ud544\uc694 - \uce90\uc2dc" },
	{ "jarcache.erase.confirm.text", "{0}\uc5d0\uc11c \ubaa8\ub4e0 \ud30c\uc77c\uc744 \uc0ad\uc81c\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?" },
	{ "jarcache.select_title", "\uce90\uc2dc \uc704\uce58" },
	{ "jarcache.enabled", "\uce90\uc2f1 \uc0ac\uc6a9" },    

	{ "update.update_button.text", "\uc9c0\uae08 \uc5c5\ub370\uc774\ud2b8" },
	{ "update.advanced_button.text", "\uace0\uae09..." },
	{ "update.desc.text", "<html>Java(TM) Update \uba54\ucee4\ub2c8\uc998\uc5d0\uc11c\ub294 \uac00\uc7a5 \ucd5c\uadfc\uc5d0 \uc5c5\ub370\uc774\ud2b8\ub41c Java \ud50c\ub7ab\ud3fc\uc744 \uc720\uc9c0\ud558\ub3c4\ub85d \ud574 \uc90d\ub2c8\ub2e4.<br>\uc544\ub798 \uc635\uc158\uc744 \uc0ac\uc6a9\ud558\uc5ec \uc5c5\ub370\uc774\ud2b8\ub97c \uc5bb\uace0 \uc801\uc6a9\ud558\ub294 \ubc29\ubc95\uc5d0 \ub300\ud55c \ud504\ub85c\uc138\uc2a4\ub97c \uc81c\uc5b4\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4.</html>" },
        { "update.notify.text", "\uc54c\ub9bc:" },
        { "update.notify_install.text", "\uc124\uce58\ud558\uae30 \uc804" },
        { "update.notify_download.text", "\ub2e4\uc6b4\ub85c\ub4dc \ubc0f \uc124\uce58\ud558\uae30 \uc804" },
        { "update.autoupdate.text", "\uc5c5\ub370\uc774\ud2b8 \uc790\ub3d9 \uac80\uc0c9" },
        { "update.advanced_title.text", "\uc790\ub3d9 \uc5c5\ub370\uc774\ud2b8 \uace0\uae09 \uc124\uc815" },
        { "update.advanced_title1.text", "\uac80\uc0c9 \ube48\ub3c4 \ubc0f \uc2dc\uae30\ub97c \uc120\ud0dd\ud558\uc2ed\uc2dc\uc624." },
        { "update.advanced_title2.text", "\ube48\ub3c4" },
        { "update.advanced_title3.text", "\uc2dc\uae30" },
        { "update.advanced_desc1.text", "\ub9e4\uc77c {0}\uc5d0 \uac80\uc0c9 " },
        { "update.advanced_desc2.text", "\ub9e4 {0}\ub9c8\ub2e4 {1}\uc5d0 \uac80\uc0c9 " },
        { "update.advanced_desc3.text", "\ub9e4\uc6d4 {0}\uc77c\uc758 {1}\uc5d0 \uac80\uc0c9 " },
        { "update.advanced_at.text", "{0}" },
        { "update.check_daily.text", "\uc77c \ub2e8\uc704" },
        { "update.check_weekly.text", "\uc8fc \ub2e8\uc704" },
        { "update.check_monthly.text", "\uc6d4 \ub2e8\uc704" },
        { "update.check_date.text", "\ub0a0\uc9dc:" },
        { "update.check_day.text", "\uc694\uc77c:" },
        { "update.check_time.text", "\uc2dc\uac04:" },
        { "update.lastrun.text", "Java Update\uac00 {1}, {0}\uc5d0 \ub9c8\uc9c0\ub9c9\uc73c\ub85c \uc2e4\ud589\ub418\uc5c8\uc2b5\ub2c8\ub2e4." },
        { "update.desc_autooff.text", "<html>\uc544\ub798\uc758 \"\uc9c0\uae08 \uc5c5\ub370\uc774\ud2b8\" \ubc84\ud2bc\uc744 \uc0ac\uc6a9\ud558\uc5ec \uc5c5\ub370\uc774\ud2b8\ub97c \uac80\uc0c9\ud574\uc57c \ud569\ub2c8\ub2e4.<br></html>" },
        { "update.desc_check_daily.text", "<html>\ub9e4\uc77c {0}\uc5d0 " },
        { "update.desc_check_weekly.text", "<html>\ub9e4 {0}\uc758 {1}\uc5d0 " },
        { "update.desc_check_monthly.text", "<html>\ub9e4\uc6d4 {0}\uc77c\uc758 {1}\uc5d0 " },
        { "update.desc_check.text", ", Java Update\uc5d0\uc11c \uc5c5\ub370\uc774\ud2b8\ub97c \uac80\uc0c9\ud569\ub2c8\ub2e4. " },
        { "update.desc_notify.text", "\uc0c8 \uc5c5\ub370\uc774\ud2b8 <br>\uac00 \ubc1c\uacac\ub418\uba74 \uc5c5\ub370\uc774\ud2b8\uac00 " },
        { "update.desc_notify_install.text", "\uc124\uce58\ub418\uae30 \uc804\uc5d0 \uc54c\ub9bc\uc744 \ubc1b\uc2b5\ub2c8\ub2e4.</html>" },
        { "update.desc_notify_download.text", "\ub2e4\uc6b4\ub85c\ub4dc \ubc0f \uc124\uce58\ub418\uae30 \uc804\uc5d0 \uc54c\ub9bc\uc744 \ubc1b\uc2b5\ub2c8\ub2e4.</html>" },
	{ "update.launchbrowser.error.text", "<html><b>Java Update Checker\ub97c \uc2e4\ud589\ud560 \uc218 \uc5c6\uc74c</b></html>\ucd5c\uc2e0 Java(TM) Update\ub97c \uad6c\ud558\ub824\uba74 http://java.sun.com/getjava/javaupdate\ub97c \ubc29\ubb38\ud558\uc2ed\uc2dc\uc624." },
	{ "update.launchbrowser.error.caption", "\uc624\ub958 - \uc5c5\ub370\uc774\ud2b8" },
    };
}

