/*
 * @(#)ControlPanel_zh_TW.java	1.34 03/04/27
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Traditional Chinese version of ControlPanel strings.
 * from
 * US English version of ControlPanel strings.
 *
 * @author Graham Hamilton
 */

public class ControlPanel_zh_TW extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "advanced.jre_format", "JRE {0} \u65bc {1}" },
	{ "advanced.jdk_format", "SDK {0} \u65bc {1}" },
        { "panel.about", "\u95dc\u65bc" },
	{ "panel.basic", "\u57fa\u672c" },
	{ "panel.advanced", "\u9032\u968e" },
        { "panel.browser", "\u700f\u89bd\u5668" },
	{ "panel.proxies", "\u4ee3\u7406" },
	{ "panel.cache", "\u5feb\u53d6" },
	{ "panel.cert", "\u8b49\u66f8" },
	{ "panel.update", "\u66f4\u65b0" },
	{ "panel.apply", "\u5957\u7528(A)" },
        { "panel.apply.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "panel.cancel", "\u91cd\u8a2d(R)" },
	{ "panel.cancel.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "panel.apply_failed", "\u5beb\u5165\u5c6c\u6027\u6a94\u6848\u5931\u6557" },
	{ "panel.apply_failed_title", "\u5957\u7528\u5931\u6557" },
	{ "panel.help", "\u8aaa\u660e(H)" },
	{ "panel.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "panel.help_title", "\u8aaa\u660e - Java Plug-in \u63a7\u5236\u9762\u677f" },
	{ "panel.help_close", "\u95dc\u9589(C)" },
	{ "panel.help_close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "panel.help.error.text", "<html><b>\u6a94\u6848\u4e0d\u5b58\u5728</b></html>\u7121\u6cd5\u8f09\u5165\u8aaa\u660e\u6a94\u3002\n" },
	{ "panel.help.error.caption", "\u932f\u8aa4 - Java Plug-in \u63a7\u5236\u9762\u677f" },

	{ "basic.show_exception", "\u986f\u793a\u7570\u5e38\u5c0d\u8a71\u65b9\u584a" },
	{ "basic.recycle_classloader", "\u56de\u6536\u985e\u5225\u8f09\u5165\u5668" },
        { "basic.java_console", "Java \u4e3b\u63a7\u53f0" },
        { "basic.show_console", "\u986f\u793a\u4e3b\u63a7\u53f0" },
        { "basic.hide_console", "\u96b1\u85cf\u4e3b\u63a7\u53f0" },
        { "basic.no_console", "\u4e0d\u555f\u52d5\u4e3b\u63a7\u53f0" },
	{ "basic.show_systray", "\u5728\u7cfb\u7d71\u5de5\u5177\u5217\u4e2d\u986f\u793a Java" },
        
	{ "advanced.jre_name", "Java Runtime Environment" },
	{ "advanced.path", "\u5176\u4ed6\u7684 SDK/JRE " },
	{ "advanced.other_jdk", "\u5176\u4ed6\u2026" },
	{ "advanced.default_jdk", "\u4f7f\u7528 Java Plug-in \u9810\u8a2d\u503c" },
	{ "advanced.jre_selection_warning.info", "<html><b>\u4e0d\u652f\u63f4\u7684\u4f5c\u696d</b></html>\u4e0d\u5efa\u8b70\u9078\u53d6\u300c \u9810\u8a2d\u503c\u300d \u4ee5\u5916\u7684 Java Runtime\u3002\n"},
	{ "advanced.jre_selection_warning.caption", "\u8b66\u544a - \u9032\u968e"},
        { "advanced.error.caption", "\u932f\u8aa4 - \u9032\u968e" },
        { "advanced.error.text", "<html><b>\u76ee\u9304\u4e0d\u5b58\u5728</b></html>\u8acb\u78ba\u5b9a\u6b64\u9078\u9805\u4e0d\u662f\u6a94\u6848\uff0c\u4e5f\u4e0d\u662f\u4e0d\u5b58\u5728\u7684\u76ee\u9304\u3002\n" },
        { "advanced.java_parms", "Java Runtime \u53c3\u6578" },
	{ "advanced.warning_popup_ok", "\u78ba\u5b9a" },
	{ "advanced.warning_popup_cancel", "\u53d6\u6d88" },

        { "browser.settings", "\u8a2d\u5b9a" },
	{ "browser.desc.text", "\u4e0b\u5217\u700f\u89bd\u5668\u6703\u5c07 Java(TM) Plug-in \u7576\u4f5c\u662f\u9810\u8a2d\u7684 Java Runtime \u4f7f\u7528:" },
	{ "browser.ie.text", "Microsoft Internet Explorer" },
	{ "browser.ns6.text", "Netscape 6 \u6216\u66f4\u9ad8\u7248\u672c" },
        { "browser.moz.text", "Mozilla 1.1 \u6216\u66f4\u9ad8\u7248\u672c"},

	{ "browser.settings.success.caption", "\u9806\u5229\u5b8c\u6210 - \u700f\u89bd\u5668" },
	{ "browser.settings.fail.caption", "\u8b66\u544a - \u700f\u89bd\u5668" },

	{ "browser.settings.success.text", "<html><b>\u700f\u89bd\u5668\u8a2d\u5b9a\u503c\u5df2\u8b8a\u66f4</b></html>\u6b64\u8b8a\u66f4\u5728\u700f\u89bd\u5668\u91cd\u65b0\u555f\u52d5\u6642\u751f\u6548\u3002\n" },

	{ "browser.settings.fail.ie.text", "<html><b>\u7121\u6cd5\u8b8a\u66f4\u700f\u89bd\u5668\u8a2d\u5b9a\u503c</b></html>\u8acb\u6aa2\u67e5\u60a8\u662f\u5426\u5177\u6709\u8db3\u5920\u8a31\u53ef\u6b0a\n"
					+ "\u53ef\u4ee5\u8b8a\u66f4\u7cfb\u7d71\u8a2d\u5b9a\u503c\u3002\n" },

	{ "browser.settings.fail.ns6.text", "<html><b>\u7121\u6cd5\u8b8a\u66f4\u700f\u89bd\u5668\u8a2d\u5b9a\u503c</b></html>"
					+ "\u8acb\u6aa2\u67e5 Netscape 6 \u662f\u5426\u5df2\u6b63\u78ba\u5b89\u88dd\u5728\u7cfb\u7d71\u4e0a\uff0c "
					+ "\u6216\u60a8\u662f\u5426\u5177\u6709\u8db3\u5920\u8a31\u53ef\u6b0a\u53ef\u4ee5\u8b8a\u66f4\u7cfb\u7d71"
					+ "\u8a2d\u5b9a\u503c\u3002\n" },

        { "browser.settings.fail.moz.text", "<html><b>\u7121\u6cd5\u8b8a\u66f4\u700f\u89bd\u5668\u8a2d\u5b9a\u503c</b></html>"
                                        + "\u8acb\u6aa2\u67e5 Mozilla \u662f\u5426\u5df2\u6b63\u78ba\u5b89\u88dd\u5728\u7cfb\u7d71\u4e0a\uff0c"
                                        + "\u548c/\u6216\u60a8\u662f\u5426\u5177\u6709\u8db3\u5920\u8a31\u53ef\u6b0a\u53ef\u4ee5\u8b8a\u66f4\u7cfb\u7d71"
                                        + "\u8a2d\u5b9a\u503c\u3002\n" },

	{ "browser.settings.alert.text", "<html><b>\u6709\u66f4\u65b0\u7248\u672c\u7684 Java Runtime</b></html>Internet Explorer \u5df2\u6709\u66f4\u65b0\u7248\u672c\u7684 Java Runtime\u3002\u60a8\u60f3\u8981\u53d6\u4ee3\u5b83\u55ce\uff1f\n" },

	{ "proxy.use_browser", "\u4f7f\u7528\u700f\u89bd\u5668\u9810\u8a2d\u503c" },
	{ "proxy.proxy_settings", "\u4ee3\u7406\u8a2d\u5b9a\u503c" },
	{ "proxy.protocol_type", "\u985e\u578b" },
	{ "proxy.proxy_protocol", "\u901a\u4fe1\u5354\u5b9a" },
	{ "proxy.proxy_address", "\u4f4d\u5740" },
	{ "proxy.proxy_port", "\u9023\u63a5\u57e0" },
	{ "proxy.http", "HTTP" },
	{ "proxy.ftp", "FTP" },
	{ "proxy.gopher", "Gopher" },
	{ "proxy.https", "Secure" },
	{ "proxy.socks", "Socks" },
	{ "proxy.same_for_all_protocols", "\u6240\u6709\u5354\u5b9a\u90fd\u7528\u540c\u4e00\u4ee3\u7406\u7a0b\u5f0f\u4f3a\u670d\u5668" },
	{ "proxy.bypass", "\u7121\u4ee3\u7406\u4e3b\u6a5f\uff08\u8acb\u4f7f\u7528\u9017\u9ede\u4f86\u5206\u958b\u591a\u90e8\u4e3b\u6a5f\uff09" },
	{ "proxy.autourl", "\u81ea\u52d5\u4ee3\u7406\u914d\u7f6e URL" },

	{ "cert.remove_button", "\u79fb\u9664(M)" },
	{ "cert.remove_button.acceleratorKey", new Integer(KeyEvent.VK_M)},
	{ "cert.import_button", "\u532f\u5165(I)" },
	{ "cert.import_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "cert.export_button", "\u532f\u51fa(E)" },
	{ "cert.export_button.acceleratorKey", new Integer(KeyEvent.VK_E)},
        { "cert.details_button", "\u660e\u7d30(D)" },
	{ "cert.details_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "cert.viewcert_button", "\u6aa2\u8996\u8b49\u66f8(V)" },
	{ "cert.viewcert_button.acceleratorKey", new Integer(KeyEvent.VK_V)},
        { "cert.rbutton_signed_applet", "\u5e36\u7c3d\u540d\u7684 Applet" },
        { "cert.rbutton_secure_site", "\u5b89\u5168\u7db2\u7ad9" },
        { "cert.rbutton_signer_ca", "\u7c3d\u7f72\u8005 CA" },
        { "cert.rbutton_secure_site_ca", "\u5b89\u5168\u7db2\u7ad9 CA" },
	{ "cert.SignedApplet_value", "SignedApplet" },
	{ "cert.SecureSite_value", "SecureSite" },
	{ "cert.SignerCA_value", "SignerCA" },
	{ "cert.SecureSiteCA_value", "SecureSiteCA" },
        { "cert.settings", "\u8b49\u66f8" },
        { "cert.dialog.import.error.caption", "\u932f\u8aa4 - \u532f\u5165\u8b49\u66f8" },
	{ "cert.dialog.import.format.text", "<html><b>\u7121\u6cd5\u8fa8\u8b58\u7684\u6a94\u6848\u683c\u5f0f</b></html>\u6c92\u6709\u532f\u5165\u8b49\u66f8\u3002" },
	{ "cert.dialog.import.file.text", "<html><b>\u6a94\u6848\u4e0d\u5b58\u5728</b></html>\u6c92\u6709\u532f\u5165\u8b49\u66f8\u3002" },
	{ "cert.dialog.import.password.text", "<html><b>\u7121\u6548\u7684\u5bc6\u78bc</b></html>\u60a8\u8f38\u5165\u7684\u5bc6\u78bc\u4e0d\u6b63\u78ba\u3002" },
	{ "cert.dialog.password.caption", "\u5bc6\u78bc - \u532f\u5165" },
	{ "cert.dialog.password.text", "<html><b>\u8f38\u5165\u5bc6\u78bc\u4ee5\u5b58\u53d6\u6b64\u6a94\u6848\uff1a<b></html>" },
	{ "cert.dialog.password.okButton", "\u78ba\u8a8d" },
	{ "cert.dialog.password.cancelButton", "\u53d6\u6d88" },
	{ "cert.dialog.export.error.caption", "\u932f\u8aa4 - \u532f\u51fa\u8b49\u66f8" },
	{ "cert.dialog.export.text", "<html><b>\u7121\u6cd5\u532f\u51fa</b></html>\u8b49\u66f8\u6c92\u6709\u532f\u51fa\u3002" },

	{ "main.control_panel_caption", "Java(TM) Plug-in \u63a7\u5236\u9762\u677f" },

	// Each line in the property_file_header must start with "#"
	{ "config.property_file_header", "# Java(TM) Plug-in \u5c6c\u6027\n"
			+ "#\u8acb\u52ff\u7de8\u8f2f\u9019\u500b\u6a94\u6848\u3002\u662f\u7531\u6a5f\u5668\u751f\u6210\u3002\n"
			+ "#\u4f7f\u7528 Activator \u63a7\u5236\u9762\u677f\u4f86\u7de8\u8f2f\u5c6c\u6027\u3002" },
	{ "config.unknownSubject", "\u4e0d\u660e\u7684\u4e3b\u984c" },
	{ "config.unknownIssuer", "\u4e0d\u660e\u7684\u7c3d\u7f72\u8005" },
	{ "config.certShowName", "{0}({1})" },
	{ "config.certShowOOU", "{0} {1}" },
	{ "config.proxy.autourl.invalid.text", "<html><b>\u5f62\u614b\u7570\u5e38\u7684 URL</b></html>\u81ea\u52d5 Proxy \u914d\u7f6e URL \u7121\u6548\u3002" },
	{ "config.proxy.autourl.invalid.caption", "\u932f\u8aa4 - Proxy" },


	{ "jarcache.location", "\u4f4d\u7f6e" },
	{ "jarcache.select", "\u9078\u64c7" },
	{ "jarcache.kb", "KB" },
	{ "jarcache.bytes", "\u4f4d\u5143\u7d44" },
	{ "jarcache.clear", "\u6e05\u9664(C)" },
        { "jarcache.clear.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "jarcache.view", "\u6aa2\u8996(V)" },
	{ "jarcache.view.acceleratorKey", new Integer(KeyEvent.VK_V) },
        { "jarcache.browseDirectory.acceleratorKey", new Integer(KeyEvent.VK_O) },
	{ "jarcache.no_compression", "\u7121" },
	{ "jarcache.select_tooltip", "\u4f7f\u7528\u6240\u9078\u7684\u4f4d\u7f6e" },
	{ "jarcache.select_mnemonic", "S" },
	{ "jarcache.maximum", "\u6700\u5927\u503c" },
	{ "jarcache.unlimited", "\u7121\u9650\u5236" },
	{ "jarcache.high_compression", "\u9ad8" },
	{ "jarcache.compression", "JAR \u58d3\u7e2e" },
	{ "jarcache.mb", "MB" },
	{ "jarcache.size", "\u5927\u5c0f" },
	{ "jarcache.settings", "\u5feb\u53d6\u8a2d\u5b9a" },
	{ "jarcache.erase.confirm.caption", "\u9700\u8981\u78ba\u8a8d - \u5feb\u53d6" },
	{ "jarcache.erase.confirm.text", "\u6e05\u9664 {0} \u4e2d\u5168\u90e8\u7684\u6a94\u6848\uff1f" },
	{ "jarcache.select_title", "\u5feb\u53d6\u4f4d\u7f6e" },
	{ "jarcache.enabled", "\u555f\u7528\u5feb\u53d6" },   
	
	{ "update.update_button.text", "\u7acb\u5373\u66f4\u65b0" },
        { "update.advanced_button.text", "\u9032\u968e..." },
        { "update.desc.text", "<html>Java(TM) Update \u6a5f\u5236\u78ba\u4fdd\u60a8\u64c1\u6709\u6700\u65b0\u7248\u672c\u7684 Java \u5e73\u53f0\u3002<br>\u4e0b\u5217\u9078\u9805\u5141\u8a31\u60a8\u63a7\u5236\u5982\u4f55\u53d6\u5f97\u8207\u5957\u7528\u66f4\u65b0\u7684\u7a0b\u5e8f\u3002</html>" },
        { "update.notify.text", "\u901a\u77e5\u6211\uff1a" },
        { "update.notify_install.text", "\u5b89\u88dd\u4e4b\u524d" },
        { "update.notify_download.text", "\u4e0b\u8f09\u4e4b\u524d\u548c\u5b89\u88dd\u4e4b\u524d" },
        { "update.autoupdate.text", "\u81ea\u52d5\u6aa2\u67e5\u66f4\u65b0" },
        { "update.advanced_title.text", "\u81ea\u52d5\u66f4\u65b0\u9032\u968e\u8a2d\u5b9a" },
        { "update.advanced_title1.text", "\u9078\u53d6\u60a8\u8981\u57f7\u884c\u6383\u63cf\u7684\u983b\u7387\u548c\u6642\u9593\u3002" },
        { "update.advanced_title2.text", "\u983b\u7387" },
        { "update.advanced_title3.text", "\u6642\u9593" },
        { "update.advanced_desc1.text", "\u6bcf\u5929\u65bc {0} \u57f7\u884c\u6383\u63cf" },
        { "update.advanced_desc2.text", "\u6bcf\u9031 {0} \u65bc {1} \u57f7\u884c\u6383\u63cf" },
        { "update.advanced_desc3.text", "\u5728\u6bcf\u6708\u7684 {0} \u65e5\u65bc {1} \u57f7\u884c\u6383\u63cf" },
        { "update.check_daily.text", "\u6bcf\u5929" },
        { "update.check_weekly.text", "\u6bcf\u9031" },
        { "update.check_monthly.text", "\u6bcf\u6708" },
        { "update.check_date.text", "\u65e5\u671f\uff1a" },
        { "update.check_day.text", "\u6bcf\uff1a" },
        { "update.check_time.text", "\u65bc\uff1a" },
        { "update.lastrun.text", "\u4e0a\u6b21\u57f7\u884c Java Update \u7684\u6642\u9593\u662f {1} \u7684 {0}\u3002" },
        { "update.desc_autooff.text", "<html>\u60a8\u5fc5\u9808\u4f7f\u7528\u4e0b\u9762\u7684\u300c\u7acb\u5373\u66f4\u65b0\u300d\u6309\u9215\u4f86\u6aa2\u67e5\u66f4\u65b0\u3002<br></html>" },
        { "update.desc_check_daily.text", "<html>\u6bcf\u5929\u65bc {0}" },
        { "update.desc_check_weekly.text", "<html>\u6bcf\u9031 {0} \u65bc {1}" },
        { "update.desc_check_monthly.text", "<html>\u5728\u6bcf\u6708\u7684 {0} \u65e5\u65bc {1}" },
        { "update.desc_check.text", ", Java Update \u5c07\u6aa2\u67e5\u66f4\u65b0\u3002" },
        { "update.desc_notify.text", "\u5982\u679c\u767c\u73fe<br>\u65b0\u7684\u66f4\u65b0\uff0c\u5247\u7cfb\u7d71\u6703\u5728" },
        { "update.desc_notify_install.text", "\u5b89\u88dd\u66f4\u65b0\u4e4b\u524d\u901a\u77e5\u60a8\u3002</html>" },
        { "update.desc_notify_download.text", "\u4e0b\u8f09\u66f4\u65b0\u4e4b\u524d\u548c\u5b89\u88dd\u66f4\u65b0\u4e4b\u524d\u901a\u77e5\u60a8\u3002</html>" },
        { "update.launchbrowser.error.text", "<html><b>\u7121\u6cd5\u555f\u52d5 Java Update \u6aa2\u67e5\u7a0b\u5f0f</b></html>\u82e5\u8981\u53d6\u5f97\u6700\u65b0\u7684 Java(TM) Update\uff0c\u8acb\u79fb\u81f3 http://java.sun.com/getjava/javaupdate" },
        { "update.launchbrowser.error.caption", "\u932f\u8aa4 - \u66f4\u65b0" },

    };
}

