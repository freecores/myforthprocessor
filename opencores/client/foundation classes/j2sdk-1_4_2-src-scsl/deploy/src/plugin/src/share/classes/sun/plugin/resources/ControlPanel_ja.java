/*
 * @(#)ControlPanel_ja.java	1.33 03/04/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Japanese verison of ControlPanel strings.
 *
 * @author Graham Hamilton
 */

public class ControlPanel_ja extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "advanced.jre_format", "{1} \u306e JRE {0}" },
	{ "advanced.jdk_format", "{1} \u306e SDK {0}" },
	{ "panel.about", "Java Plug-in \u306b\u3064\u3044\u3066" },
	{ "panel.basic", "\u57fa\u672c" },
	{ "panel.advanced", "\u8a73\u7d30" },
	{ "panel.browser", "\u30d6\u30e9\u30a6\u30b6" },
	{ "panel.proxies", "\u30d7\u30ed\u30ad\u30b7" },
	{ "panel.cache", "\u30ad\u30e3\u30c3\u30b7\u30e5" },
	{ "panel.cert", "\u8a3c\u660e\u66f8" },
	{ "panel.update", "\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8" },
	{ "panel.apply", "\u9069\u7528(A)" },
        { "panel.apply.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "panel.cancel", "\u30ea\u30bb\u30c3\u30c8(R)" },
	{ "panel.cancel.acceleratorKey", new Integer(KeyEvent.VK_R)},
        { "panel.apply_failed", "\u30d7\u30ed\u30d1\u30c6\u30a3\u30d5\u30a1\u30a4\u30eb\u3078\u306e\u66f8\u304d\u8fbc\u307f\u306b\u5931\u6557\u3057\u307e\u3057\u305f" },
	{ "panel.apply_failed_title", "\u9069\u7528\u306b\u5931\u6557" },
	{ "panel.help", "\u30d8\u30eb\u30d7(H)" },
	{ "panel.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "panel.help_title", "\u30d8\u30eb\u30d7 - Java Plug-in \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb" },
	{ "panel.help_close", "\u9589\u3058\u308b(C)" },
	{ "panel.help_close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "panel.help.error.text", "<html><b>\u30d5\u30a1\u30a4\u30eb\u306e\u975e\u5b58\u5728</b></html>\u30d8\u30eb\u30d7\u30d5\u30a1\u30a4\u30eb\u3092\u30ed\u30fc\u30c9\u3067\u304d\u307e\u305b\u3093\u3002\n" },
	{ "panel.help.error.caption", "\u30a8\u30e9\u30fc - Java Plug-in \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb" },

	{ "basic.show_exception", "\u4f8b\u5916\u30c0\u30a4\u30a2\u30ed\u30b0\u30dc\u30c3\u30af\u30b9\u306e\u8868\u793a" },
	{ "basic.recycle_classloader", "\u30af\u30e9\u30b9\u30ed\u30fc\u30c0\u306e\u518d\u5229\u7528" },
        { "basic.java_console", "Java \u30b3\u30f3\u30bd\u30fc\u30eb" },
        { "basic.show_console", "\u30b3\u30f3\u30bd\u30fc\u30eb\u3092\u8868\u793a" },
        { "basic.hide_console", "\u30b3\u30f3\u30bd\u30fc\u30eb\u3092\u975e\u8868\u793a" },
        { "basic.no_console", "\u30b3\u30f3\u30bd\u30fc\u30eb\u3092\u958b\u59cb\u3057\u306a\u3044" },
        { "basic.show_systray", "Java \u30a2\u30a4\u30b3\u30f3\u3092\u30b7\u30b9\u30c6\u30e0\u30c8\u30ec\u30a4\u306b\u8868\u793a" },
        
	{ "advanced.jre_name", "Java Runtime Environment" },
	{ "advanced.path", "\u4ed6\u306e SDK/JRE" },
	{ "advanced.other_jdk", "\u305d\u306e\u4ed6 ..." },
	{ "advanced.default_jdk", "Java Plug-in \u306e\u30c7\u30d5\u30a9\u30eb\u30c8\u3092\u4f7f\u7528" },
	{ "advanced.jre_selection_warning.info", "<html><b>\u672a\u30b5\u30dd\u30fc\u30c8\u306e\u64cd\u4f5c</b></html>\"\u30c7\u30d5\u30a9\u30eb\u30c8\" \u4ee5\u5916\u306e Java Runtime \u306e\u9078\u629e\u306f\u63a8\u5968\u3055\u308c\u3066\u3044\u307e\u305b\u3093\u3002\n"},
	{ "advanced.jre_selection_warning.caption", "\u8b66\u544a - \u8a73\u7d30"},
        { "advanced.error.caption", "\u30a8\u30e9\u30fc - \u8a73\u7d30" },
        { "advanced.error.text", "<html><b>\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u306e\u975e\u5b58\u5728</b></html>\u30d5\u30a1\u30a4\u30eb\u307e\u305f\u306f\u5b58\u5728\u3057\u306a\u3044\u30c7\u30a3\u30ec\u30af\u30c8\u30ea\u3092\u9078\u629e\u3057\u3066\u3044\u306a\u3044\u304b\u3069\u3046\u304b\u78ba\u8a8d\u3057\u3066\u304f\u3060\u3055\u3044\u3002\n" },
	{ "advanced.java_parms", "Java \u5b9f\u884c\u6642\u306e\u30d1\u30e9\u30e1\u30fc\u30bf" },
	{ "advanced.warning_popup_ok", "\u4e86\u89e3" },
	{ "advanced.warning_popup_cancel", "\u53d6\u6d88\u3057" },

	{ "browser.settings", "\u8a2d\u5b9a" },
	{ "browser.desc.text", "Java(TM) Plug-in \u306f\u3001\u4ee5\u4e0b\u306e\u30d6\u30e9\u30a6\u30b6\u3067\u30c7\u30d5\u30a9\u30eb\u30c8\u306e Java \u5b9f\u884c\u74b0\u5883\u3068\u3057\u3066\u4f7f\u7528\u3055\u308c\u307e\u3059:" },
	{ "browser.ie.text", "Microsoft Internet Explorer" },
	{ "browser.ns6.text", "Netscape 6 \u4ee5\u964d" },
        { "browser.moz.text", "Mozilla 1.1 \u4ee5\u964d" },

	{ "browser.settings.success.caption", "\u6210\u529f - \u30d6\u30e9\u30a6\u30b6" },
	{ "browser.settings.fail.caption", "\u8b66\u544a - \u30d6\u30e9\u30a6\u30b6" },

	{ "browser.settings.success.text", "<html><b>\u30d6\u30e9\u30a6\u30b6\u8a2d\u5b9a\u306e\u5909\u66f4\u5b8c\u4e86</b></html>\u5909\u66f4\u306f\u30d6\u30e9\u30a6\u30b6\u306e\u518d\u8d77\u52d5\u5f8c\u306b\u6709\u52b9\u306b\u306a\u308a\u307e\u3059\u3002\n" },

	{ "browser.settings.fail.ie.text", "<html><b>\u30d6\u30e9\u30a6\u30b6\u8a2d\u5b9a\u306e\u5909\u66f4\u4e0d\u53ef</b></html>\u30b7\u30b9\u30c6\u30e0\u8a2d\u5b9a\u306e\u5909\u66f4\u306b"
					+ "\u5fc5\u8981\u306a\u30a2\u30af\u30bb\u30b9\u6a29\u304c\u3042\u308b\u304b\u3069\u3046\u304b\u78ba\u8a8d\u3057\u3066\u304f\u3060\u3055\u3044\u3002\n" },

	{ "browser.settings.fail.ns6.text", "<html><b>\u30d6\u30e9\u30a6\u30b6\u8a2d\u5b9a\u306e\u5909\u66f4\u4e0d\u53ef</b></html>"
					+ "Netscape 6 \u304c\u30b7\u30b9\u30c6\u30e0\u306b\u9069\u5207\u306b\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3055\u308c\u3066\u3044\u308b\u304b\u3069\u3046\u304b\u3001"
					+ "\u307e\u305f\u306f\u30b7\u30b9\u30c6\u30e0\u8a2d\u5b9a\u306e"
					+ "\u5909\u66f4\u306b\u5fc5\u8981\u306a\u30a2\u30af\u30bb\u30b9\u6a29\u304c\u3042\u308b\u304b\u3069\u3046\u304b\u78ba\u8a8d\u3057\u3066\u304f\u3060\u3055\u3044\u3002\n" },

        { "browser.settings.fail.moz.text", "<html><b>\u30d6\u30e9\u30a6\u30b6\u8a2d\u5b9a\u306e\u5909\u66f4\u4e0d\u53ef</b></html>"
                                        + "Mozilla \u304c\u30b7\u30b9\u30c6\u30e0\u306b\u9069\u5207\u306b\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u3055\u308c\u3066\u3044\u308b\u304b\u3069\u3046\u304b\u3001"
                                        + "\u307e\u305f\u306f\u30b7\u30b9\u30c6\u30e0\u8a2d\u5b9a\u306e"
                                        + "\u5909\u66f4\u306b\u5fc5\u8981\u306a\u30a2\u30af\u30bb\u30b9\u6a29\u304c\u3042\u308b\u304b\u3069\u3046\u304b\u78ba\u8a8d\u3057\u3066\u304f\u3060\u3055\u3044\u3002\n" },

	{ "browser.settings.alert.text", "<html><b>\u3088\u308a\u65b0\u3057\u3044 JRE \u306e\u5b58\u5728</b></html>Internet Explorer \u306f\u3088\u308a\u65b0\u3057\u3044\u30d0\u30fc\u30b8\u30e7\u30f3\u306e JRE \u3092\u4fdd\u6301\u3057\u3066\u3044\u307e\u3059\u3002\u7f6e\u304d\u63db\u3048\u307e\u3059\u304b?\n" },

	{ "proxy.use_browser", "\u30d6\u30e9\u30a6\u30b6\u306e\u8a2d\u5b9a\u3092\u4f7f\u7528" },
	{ "proxy.proxy_settings", "\u30d7\u30ed\u30ad\u30b7\u8a2d\u5b9a" },
	{ "proxy.protocol_type", "\u578b" },
	{ "proxy.proxy_protocol", "\u30d7\u30ed\u30c8\u30b3\u30eb" },
	{ "proxy.proxy_address", "\u30a2\u30c9\u30ec\u30b9" },
	{ "proxy.proxy_port", "\u30dd\u30fc\u30c8" },
	{ "proxy.http", "HTTP" },
	{ "proxy.ftp", "FTP" },
	{ "proxy.gopher", "Gopher" },
	{ "proxy.https", "Secure" },
	{ "proxy.socks", "Socks" },
	{ "proxy.same_for_all_protocols", "\u3059\u3079\u3066\u306e\u30d7\u30ed\u30c8\u30b3\u30eb\u306b\u540c\u3058\u30d7\u30ed\u30ad\u30b7\u30b5\u30fc\u30d0\u3092\u4f7f\u7528" },
	{ "proxy.bypass", "\u30d7\u30ed\u30ad\u30b7\u30db\u30b9\u30c8\u306a\u3057 (\u8907\u6570\u306e\u30db\u30b9\u30c8\u540d\u3092\u533a\u5207\u308b\u5834\u5408\u306f\u30ab\u30f3\u30de\u3092\u4f7f\u7528)" },
	{ "proxy.autourl", "\u81ea\u52d5\u30d7\u30ed\u30ad\u30b7\u69cb\u6210 URL" },

	{ "cert.remove_button", "\u524a\u9664(M)" },
	{ "cert.remove_button.acceleratorKey", new Integer(KeyEvent.VK_M)},
        { "cert.import_button", "\u30a4\u30f3\u30dd\u30fc\u30c8(I)" },
	{ "cert.import_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
        { "cert.export_button", "\u30a8\u30af\u30b9\u30dd\u30fc\u30c8(E)" },
	{ "cert.export_button.acceleratorKey", new Integer(KeyEvent.VK_E)},
        { "cert.details_button", "\u8a73\u7d30(D)" },
	{ "cert.details_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
        { "cert.viewcert_button", "\u8a3c\u660e\u66f8\u3092\u8868\u793a(V)" },
	{ "cert.viewcert_button.acceleratorKey", new Integer(KeyEvent.VK_V)},
        { "cert.rbutton_signed_applet", "\u7f72\u540d\u6e08\u307f\u30a2\u30d7\u30ec\u30c3\u30c8" },
	{ "cert.rbutton_secure_site", "\u30bb\u30ad\u30e5\u30a2\u30b5\u30a4\u30c8" },
	{ "cert.rbutton_signer_ca", "\u7f72\u540d\u8005\u306e CA" },
	{ "cert.rbutton_secure_site_ca", "\u30bb\u30ad\u30e5\u30a2\u30b5\u30a4\u30c8\u306e CA" },
	{ "cert.SignedApplet_value", "SignedApplet" },
	{ "cert.SecureSite_value", "SecureSite" },
	{ "cert.SignerCA_value", "SignerCA" },
	{ "cert.SecureSiteCA_value", "SecureSiteCA" },
	{ "cert.settings", "\u8a3c\u660e\u66f8" },
        { "cert.dialog.import.error.caption", "\u30a8\u30e9\u30fc - \u8a3c\u660e\u66f8\u306e\u30a4\u30f3\u30dd\u30fc\u30c8" },
	{ "cert.dialog.import.format.text", "<html><b>\u672a\u8a8d\u8b58\u306e\u30d5\u30a1\u30a4\u30eb\u5f62\u5f0f</b></html>\u8a3c\u660e\u66f8\u306f\u30a4\u30f3\u30dd\u30fc\u30c8\u3055\u308c\u307e\u305b\u3093\u3002" },
	{ "cert.dialog.import.file.text", "<html><b>\u30d5\u30a1\u30a4\u30eb\u306e\u975e\u5b58\u5728</b></html>\u8a3c\u660e\u66f8\u306f\u30a4\u30f3\u30dd\u30fc\u30c8\u3055\u308c\u307e\u305b\u3093\u3002" },
	{ "cert.dialog.import.password.text", "<html><b>\u7121\u52b9\u306a\u30d1\u30b9\u30ef\u30fc\u30c9</b></html>\u5165\u529b\u3055\u308c\u305f\u30d1\u30b9\u30ef\u30fc\u30c9\u304c\u4e0d\u6b63\u3067\u3059\u3002" },
	{ "cert.dialog.password.caption", "\u30d1\u30b9\u30ef\u30fc\u30c9 - \u30a4\u30f3\u30dd\u30fc\u30c8" },
	{ "cert.dialog.password.text", "<html><b>\u3053\u306e\u30d5\u30a1\u30a4\u30eb\u306b\u30a2\u30af\u30bb\u30b9\u3059\u308b\u305f\u3081\u306e\u30d1\u30b9\u30ef\u30fc\u30c9\u3092\u5165\u529b:<b></html>" },
	{ "cert.dialog.password.okButton", "\u4e86\u89e3" },
	{ "cert.dialog.password.cancelButton", "\u53d6\u6d88\u3057" },
	{ "cert.dialog.export.error.caption", "\u30a8\u30e9\u30fc - \u8a3c\u660e\u66f8\u306e\u30a8\u30af\u30b9\u30dd\u30fc\u30c8" },
	{ "cert.dialog.export.text", "<html><b>\u30a8\u30af\u30b9\u30dd\u30fc\u30c8\u306e\u4e0d\u53ef</b></html>\u8a3c\u660e\u66f8\u306f\u30a8\u30af\u30b9\u30dd\u30fc\u30c8\u3055\u308c\u307e\u305b\u3093\u3002" },

	{ "main.control_panel_caption", "Java(TM) Plug-in \u30b3\u30f3\u30c8\u30ed\u30fc\u30eb\u30d1\u30cd\u30eb" },

	// Each line in the property_file_header must start with "#"
	{ "config.property_file_header", "# Java(TM) Plug-in Properties\n"
			+ "# DO NOT EDIT THIS FILE.  It is machine generated.\n"
			+ "# Use the Activator Control Panel to edit properties." },
	{ "config.unknownSubject", "Unknown Subject" },
	{ "config.unknownIssuer", "Unknown Issuer" },
	{ "config.certShowName", "{0} ({1})" },
	{ "config.certShowOOU", "{0} {1}" },
	{ "config.proxy.autourl.invalid.text", "<html><b>\u4e0d\u6b63\u306a URL</b></html>\u81ea\u52d5\u30d7\u30ed\u30ad\u30b7\u69cb\u6210 URL \u304c\u7121\u52b9\u3067\u3059\u3002" },
	{ "config.proxy.autourl.invalid.caption", "\u30a8\u30e9\u30fc - \u30d7\u30ed\u30ad\u30b7" },

	{ "jarcache.location", "\u30c7\u30a3\u30ec\u30af\u30c8\u30ea" },
	{ "jarcache.select", "\u9078\u629e(S)" },
	{ "jarcache.kb", "K \u30d0\u30a4\u30c8" },
	{ "jarcache.bytes", "\u30d0\u30a4\u30c8" },
	{ "jarcache.clear", "\u30af\u30ea\u30a2(C)" },
        { "jarcache.clear.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "jarcache.view", "\u8868\u793a(V)" },
	{ "jarcache.view.acceleratorKey", new Integer(KeyEvent.VK_V) },
        { "jarcache.browseDirectory.acceleratorKey", new Integer(KeyEvent.VK_O) },
        { "jarcache.no_compression", "\u306a\u3057" },
	{ "jarcache.select_tooltip", "\u9078\u629e\u3057\u305f\u5834\u6240\u3092\u4f7f\u7528(S)" },
	{ "jarcache.select_mnemonic", "S" },
	{ "jarcache.maximum", "\u6700\u5927\u5024" },
	{ "jarcache.unlimited", "\u7121\u5236\u9650" },
	{ "jarcache.high_compression", "\u9ad8" },
	{ "jarcache.compression", "JAR \u5727\u7e2e" },
	{ "jarcache.mb", "M \u30d0\u30a4\u30c8" },
	{ "jarcache.size", "\u30b5\u30a4\u30ba" },
	{ "jarcache.settings", "\u30ad\u30e3\u30c3\u30b7\u30e5\u8a2d\u5b9a" },
	{ "jarcache.erase.confirm.caption", "\u78ba\u8a8d - \u30ad\u30e3\u30c3\u30b7\u30e5" },
	{ "jarcache.erase.confirm.text", "{0} \u5185\u306e\u30d5\u30a1\u30a4\u30eb\u3092\u3059\u3079\u3066\u6d88\u53bb\u3057\u307e\u3059\u304b?" },
	{ "jarcache.select_title", "\u30ad\u30e3\u30c3\u30b7\u30e5\u306e\u5834\u6240" },
	{ "jarcache.enabled", "\u30ad\u30e3\u30c3\u30b7\u30e5\u3092\u6709\u52b9" },    

	{ "update.update_button.text", "\u4eca\u3059\u3050\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8" },
	{ "update.advanced_button.text", "\u8a73\u7d30\u8a2d\u5b9a..." },
	{ "update.desc.text", "<html>Java(TM) Update \u30e1\u30ab\u30cb\u30ba\u30e0\u3092\u4f7f\u3048\u3070\u3001\u6700\u65b0\u7248\u306e Java \u30d7\u30e9\u30c3\u30c8\u30d5\u30a9\u30fc\u30e0\u3092\u78ba\u5b9f\u306b\u5165\u624b\u3067\u304d\u307e\u3059\u3002<br>\u4ee5\u4e0b\u306e\u30aa\u30d7\u30b7\u30e7\u30f3\u3092\u9078\u629e\u3059\u308b\u3053\u3068\u3067\u3001\u6700\u65b0\u7248\u306e\u53d6\u5f97\u65b9\u6cd5\u3084\u9069\u7528\u65b9\u6cd5\u306b\u95a2\u3059\u308b\u51e6\u7406\u3092\u5236\u5fa1\u3067\u304d\u307e\u3059\u3002</html>" },
        { "update.notify.text", "\u901a\u77e5:" },
        { "update.notify_install.text", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u524d" },
        { "update.notify_download.text", "\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u524d\u3068\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u524d" },
        { "update.autoupdate.text", "\u81ea\u52d5\u7684\u306b\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u3059\u308b" },
        { "update.advanced_title.text", "\u81ea\u52d5\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u306e\u8a73\u7d30\u8a2d\u5b9a" },
        { "update.advanced_title1.text", "\u30b9\u30ad\u30e3\u30f3\u3092\u5b9f\u884c\u3059\u308b\u983b\u5ea6\u3084\u6642\u523b\u3092\u9078\u629e\u3057\u307e\u3059\u3002" },
        { "update.advanced_title2.text", "\u983b\u5ea6" },
        { "update.advanced_title3.text", "\u6642\u523b" },
        { "update.advanced_desc1.text", "\u6bce\u65e5 {0} \u306b\u30b9\u30ad\u30e3\u30f3\u3092\u5b9f\u884c" },
        { "update.advanced_desc2.text", "\u6bce\u9031{0}\u306e {1} \u306b\u30b9\u30ad\u30e3\u30f3\u3092\u5b9f\u884c" },
        { "update.advanced_desc3.text", "\u6bce\u6708 {0} \u65e5\u306e {1} \u306b\u30b9\u30ad\u30e3\u30f3\u3092\u5b9f\u884c" },
        { "update.check_daily.text", "\u6bce\u65e5" },
        { "update.check_weekly.text", "\u6bce\u9031" },
        { "update.check_monthly.text", "\u6bce\u6708" },
        { "update.check_date.text", "\u65e5:" },
        { "update.check_day.text", "\u66dc\u65e5:" },
        { "update.check_time.text", "\u6642\u523b:" },
        { "update.lastrun.text", "Java Update \u304c\u6700\u5f8c\u306b\u5b9f\u884c\u3055\u308c\u305f\u306e\u306f {1} \u306e {0} \u3067\u3059\u3002" },
        { "update.desc_autooff.text", "<html>\u6700\u65b0\u7248\u3092\u30c1\u30a7\u30c3\u30af\u3059\u308b\u306b\u306f\u3001\u4ee5\u4e0b\u306e\u300c\u4eca\u3059\u3050\u30a2\u30c3\u30d7\u30c7\u30fc\u30c8\u300d\u30dc\u30bf\u30f3\u3092\u30af\u30ea\u30c3\u30af\u3057\u3066\u304f\u3060\u3055\u3044\u3002<br></html>" },
        { "update.desc_check_daily.text", "<html>\u6bce\u65e5 {0} \u306b " },
        { "update.desc_check_weekly.text", "<html>\u6bce\u9031{0}\u306e {1} \u306b " },
        { "update.desc_check_monthly.text", "<html>\u6bce\u6708 {0} \u65e5\u306e {1} \u306b " },
        { "update.desc_check.text", "Java Update \u306f\u6700\u65b0\u7248\u3092\u30c1\u30a7\u30c3\u30af\u3057\u307e\u3059\u3002" },
        { "update.desc_notify.text", "<br>\u6700\u65b0\u7248\u304c\u3042\u308b\u5834\u5408\u306f" },
        { "update.desc_notify_install.text", "\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u524d\u306b\u901a\u77e5\u30e1\u30c3\u30bb\u30fc\u30b8\u3092\u8868\u793a\u3057\u307e\u3059\u3002</html>" },
        { "update.desc_notify_download.text", "\u30c0\u30a6\u30f3\u30ed\u30fc\u30c9\u524d\u3068\u30a4\u30f3\u30b9\u30c8\u30fc\u30eb\u524d\u306b\u901a\u77e5\u30e1\u30c3\u30bb\u30fc\u30b8\u3092\u8868\u793a\u3057\u307e\u3059\u3002</html>" },
	{ "update.launchbrowser.error.text", "<html><b>Java Update Checker \u3092\u8d77\u52d5\u3067\u304d\u307e\u305b\u3093</b></html>\u6700\u65b0\u306e Java(TM) Update \u3092\u53d6\u5f97\u3059\u308b\u306b\u306f\u3001http://java.sun.com/getjava/javaupdate \u306b\u30a2\u30af\u30bb\u30b9\u3057\u3066\u304f\u3060\u3055\u3044" },
	{ "update.launchbrowser.error.caption", "\u30a8\u30e9\u30fc - \u30a2\u30c3\u30d7\u30c7\u30fc\u30c8" },
    };
}

