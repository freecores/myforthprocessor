/*
 * @(#)ControlPanel_zh_CN.java	1.38 03/04/27
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
 
/**
 * Simplified Chinese version of ControlPanel strings.
 *
 * @author Graham Hamilton
 */

public class ControlPanel_zh_CN extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{ "advanced.jre_format", "JRE {0} \u5728 {1} \u4e2d" },
	{ "advanced.jdk_format", "SDK {0} \u5728 {1} \u4e2d" },
	{ "panel.about", "\u5173\u4e8e" },
	{ "panel.basic", "\u521d\u7ea7" },
	{ "panel.advanced", "\u9ad8\u7ea7" },
	{ "panel.browser", "\u6d4f\u89c8\u5668" },
	{ "panel.proxies", "\u4ee3\u7406" },
        { "panel.cache", "\u9ad8\u901f\u7f13\u5b58" },
	{ "panel.cert", "\u8bc1\u4e66" },
	{ "panel.update","\u66f4\u65b0"},
	{ "panel.apply", "\u5e94\u7528(A)" },
        { "panel.apply.acceleratorKey", new Integer(KeyEvent.VK_A)}, 
	{ "panel.cancel", "\u91cd\u8bbe(R)" },
        { "panel.cancel.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "panel.apply_failed", "\u5199\u5c5e\u6027\u6587\u4ef6\u5931\u8d25" },
	{ "panel.apply_failed_title", "\u5e94\u7528\u5931\u8d25" },
        { "panel.help", "\u5e2e\u52a9(H)" },
        { "panel.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
        { "panel.help_title", "\u5e2e\u52a9 - Java Plug-in \u63a7\u5236\u9762\u677f" },
        { "panel.help_close", "\u5173\u95ed(C)" },
        { "panel.help_close.acceleratorKey", new Integer(KeyEvent.VK_C)},
        { "panel.help.error.text", "<html><b>\u6587\u4ef6\u4e0d\u5b58\u5728</b></html>\u4e0d\u80fd\u8f7d\u5165\u5e2e\u52a9\u6587\u4ef6\u3002\n" },
        { "panel.help.error.caption", "\u9519\u8bef - Java Plug-in \u63a7\u5236\u9762\u677f" },
 



	
	{ "basic.show_exception", "\u663e\u793a\u5f02\u5e38\u5bf9\u8bdd\u6846" },
	{ "basic.recycle_classloader", "\u56de\u6536\u7c7b\u8f7d\u5165\u7a0b\u5e8f" },
        { "basic.java_console", "Java \u63a7\u5236\u53f0" },
        { "basic.show_console", "\u663e\u793a\u63a7\u5236\u53f0" },
        { "basic.hide_console", "\u9690\u85cf\u63a7\u5236\u53f0" },
        { "basic.no_console", "\u4e0d\u8981\u542f\u52a8\u63a7\u5236\u53f0" },
	{ "basic.show_systray", "\u5728\u4efb\u52a1\u680f\u72b6\u6001\u533a\u663e\u793a Java \u56fe\u6807" },
        
	{ "advanced.jre_name", "Java Runtime Environment" },
	{ "advanced.path", "\u5176\u4ed6 SDK/JRE" },
	


	{ "advanced.other_jdk", "\u5176\u5b83..." },
	{ "advanced.default_jdk", "\u4f7f\u7528 Java Plug-in \u7f3a\u7701\u503c" },
        { "advanced.jre_selection_warning.info", "<html><b>\u4e0d\u652f\u6301\u64cd\u4f5c</b></html>\u4e0d\u63a8\u8350\u9009\u62e9\u975e\"\u7f3a\u7701\"\u7684 Java Runtime\n" }, 
        { "advanced.jre_selection_warning.caption", "\u8b66\u544a - \u9ad8\u7ea7" }, 
        { "advanced.error.caption", "\u9519\u8bef - \u9ad8\u7ea7" }, 
        { "advanced.error.text", "<html><b>\u76ee\u5f55\u4e0d\u5b58\u5728</b></html>\u8bf7\u786e\u5b9a\u9009\u62e9\u7684\u4e0d\u662f\u6587\u4ef6\u6216\u4e0d\u5b58\u5728\u7684\u76ee\u5f55\u3002\n" }, 
	{ "advanced.java_parms", "Java Runtime \u53c2\u6570" },
        { "advanced.warning_popup_ok", "\u786e\u5b9a" },
        { "advanced.warning_popup_cancel", "\u53d6\u6d88" },

        { "browser.settings", "\u8bbe\u7f6e" },
        { "browser.desc.text", "Java(TM) Plug-in \u5c06\u7528\u4f5c\u4ee5\u4e0b\u6d4f\u89c8\u5668\u7684\u7f3a\u7701 Java Runtime:" },
	{ "browser.ie.text", "Microsoft Internet Explorer" },
	{ "browser.ns6.text", "Netscape 6 \u6216\u66f4\u9ad8\u7248\u672c" },
        { "browser.moz.text", "Mozilla 1.1 \u6216\u66f4\u9ad8\u7248\u672c"},

        { "browser.settings.success.caption", "\u6210\u529f - \u6d4f\u89c8\u5668" }, 
        { "browser.settings.fail.caption", "\u8b66\u544a - \u6d4f\u89c8\u5668" }, 
  
        { "browser.settings.success.text", "<html><b>\u6d4f\u89c8\u5668\u8bbe\u7f6e\u5df2\u66f4\u6539</b></html>\u66f4\u6539\u7684\u8bbe\u7f6e\u5728\u6d4f\u89c8\u5668\u91cd\u65b0\u542f\u52a8\u540e\u751f\u6548\u3002\n" }, 
  
        { "browser.settings.fail.ie.text", "<html><b>\u4e0d\u80fd\u66f4\u6539\u6d4f\u89c8\u5668\u8bbe\u7f6e </b></html>\u8bf7\u68c0\u67e5\u60a8" 
                                          + "\u662f\u5426\u6709\u8db3\u591f\u7684\u6743\u9650\u66f4\u6539\u7cfb\u7edf\u8bbe\u7f6e\n" }, 
  
        { "browser.settings.fail.ns6.text", "<html><b>\u4e0d\u80fd\u66f4\u6539\u6d4f\u89c8\u5668\u8bbe\u7f6e</b></html>" 
                                         + "\u8bf7\u68c0\u67e5\u662f\u5426\u5728\u7cfb\u7edf\u4e2d\u6b63\u786e\u5b89\u88c5\u4e86 Netscape 6\uff0c" 
                                         + "\u6216\u60a8"
                                         + "\u662f\u5426\u5177\u6709\u8db3\u591f\u7684\u6743\u9650\u66f4\u6539\u7cfb\u7edf\u8bbe\u7f6e\u3002\n" }, 

        { "browser.settings.fail.moz.text", "<html><b>\u65e0\u6cd5\u66f4\u6539\u6d4f\u89c8\u5668\u8bbe\u7f6e</b></html>"
                                        + "\u8bf7\u68c0\u67e5\u662f\u5426\u5728\u7cfb\u7edf\u4e0a\u6b63\u786e\u5b89\u88c5\u4e86 Mozilla \u548c/\u6216 "
                                        + "\u60a8\u662f\u5426\u5177\u6709"
                                        + "\u66f4\u6539\u7cfb\u7edf\u8bbe\u7f6e\u7684\u8db3\u591f\u6743\u9650\u3002\n" },

        { "browser.settings.alert.text", "<html><b>\u8f83\u65b0\u7684 Java Runtime \u5b58\u5728</b></html>Internet Explorer \u5df2\u6709\u8f83\u65b0\u7248\u672c\u7684 Java Runtime\u3002\u60a8\u613f\u610f\u66ff\u6362\u5b83\u5417\uff1f\n" }, 

	{ "proxy.use_browser", "\u4f7f\u7528\u6d4f\u89c8\u5668\u8bbe\u7f6e" },
	{ "proxy.proxy_settings", "\u4ee3\u7406\u8bbe\u7f6e" },
	{ "proxy.protocol_type", "\u7c7b\u578b" },
	{ "proxy.proxy_protocol", "\u534f\u8bae" },
	{ "proxy.proxy_address", "\u5730\u5740" },
	{ "proxy.proxy_port", "\u7aef\u53e3" },
	{ "proxy.http", "HTTP" },
	{ "proxy.ftp", "FTP" },
	{ "proxy.gopher", "Gopher" },
	{ "proxy.https", "Secure" },
	{ "proxy.socks", "Socks" },
	{ "proxy.same_for_all_protocols", "\u652f\u6301\u6240\u6709\u534f\u8bae\u7684\u76f8\u540c\u4ee3\u7406\u670d\u52a1\u5668" },
	{ "proxy.bypass", "\u65e0\u4ee3\u7406\u4e3b\u673a \uff08\u4f7f\u7528\u9017\u53f7\u5206\u9694\u591a\u4e2a\u4e3b\u673a\uff09" },
	{ "proxy.autourl", "\u81ea\u52a8\u4ee3\u7406\u914d\u7f6e URL" },
	
	{ "cert.remove_button", "\u5220\u9664(M)" },
        { "cert.remove_button.acceleratorKey", new Integer(KeyEvent.VK_M)},
	{ "cert.import_button", "\u5bfc\u5165(I)" },
        { "cert.import_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "cert.export_button", "\u5bfc\u51fa(E)" },
        { "cert.export_button.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "cert.details_button", "\u8be6\u7ec6\u4fe1\u606f(D)" },
        { "cert.details_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "cert.viewcert_button", "\u67e5\u770b\u8bc1\u4e66(V)" },
        { "cert.viewcert_button.acceleratorKey", new Integer(KeyEvent.VK_V)},
        { "cert.rbutton_signed_applet", "\u5e26\u7b7e\u540d\u7684\u5c0f\u7a0b\u5e8f" },
        { "cert.rbutton_secure_site", "\u5b89\u5168\u7ad9\u70b9" },
        { "cert.rbutton_signer_ca", "\u7b7e\u540d\u8005 CA" },
        { "cert.rbutton_secure_site_ca", "\u5b89\u5168\u7ad9\u70b9 CA" },
        { "cert.SignedApplet_value", "\u5e26\u7b7e\u540d\u7684\u5c0f\u7a0b\u5e8f" },
        { "cert.SecureSite_value", "\u5b89\u5168\u7ad9\u70b9" },
        { "cert.SignerCA_value", "\u7b7e\u540d\u8005 CA" },
        { "cert.SecureSiteCA_value", "\u5b89\u5168\u7ad9\u70b9 CA" },
        { "cert.settings", "\u8bc1\u4e66" },
        { "cert.dialog.import.error.caption", "\u9519\u8bef - \u5bfc\u5165\u8bc1\u4e66" },
        { "cert.dialog.import.format.text", "<html><b>\u4e0d\u80fd\u8bc6\u522b\u7684\u6587\u4ef6\u683c\u5f0f</b></html>\u6ca1\u6709\u5bfc\u5165\u8bc1\u4e66\u3002" },
        { "cert.dialog.import.file.text", "<html><b>\u6587\u4ef6\u4e0d\u5b58\u5728</b></html>\u6ca1\u6709\u5bfc\u5165\u8bc1\u4e66 \u3002" },
        { "cert.dialog.import.password.text", "<html><b>\u53e3\u4ee4\u65e0\u6548</b></html>\u8f93\u5165\u7684\u53e3\u4ee4\u4e0d\u6b63\u786e \u3002" },
         { "cert.dialog.password.caption", "\u53e3\u4ee4 - \u5bfc\u5165" },
         { "cert.dialog.password.text", "<html><b>\u8f93\u5165\u53e3\u4ee4\u4ee5\u8bbf\u95ee\u6b64\u6587\u4ef6\uff1a<b></html>" },
         { "cert.dialog.password.okButton", "\u786e\u5b9a" },
         { "cert.dialog.password.cancelButton", "\u53d6\u6d88" },
         { "cert.dialog.export.error.caption", "\u9519\u8bef - \u5bfc\u51fa\u8bc1\u4e66" },
         { "cert.dialog.export.text", "<html><b>\u4e0d\u80fd\u5bfc\u51fa</b></html>\u6ca1\u6709\u5bfc\u51fa\u8bc1\u4e66\u3002" },


	
	{ "main.control_panel_caption", "Java(TM) Plug-in \u63a7\u5236\u9762\u677f" },

	//Each line in the property_file_header \u4e2d\u7684\u6bcf\u4e00\u884c\u90fd\u5fc5\u987b\u4ee5 "#" \u5f00\u5934
	{ "config.property_file_header", "# Java(TM) Plug-in \u5c5e\u6027\n"
			+ "# \u4e0d\u8981\u7f16\u8f91\u6b64\u6587\u4ef6\u3002\u8fd9\u662f\u7531\u673a\u5668\u751f\u6210\u7684\u3002\n"
			+ "# \u4f7f\u7528\u201c\u6fc0\u6d3b\u7a0b\u5e8f\u201d\u63a7\u5236\u9762\u677f\u6765\u7f16\u8f91\u5c5e\u6027\u3002" },
        { "config.unknownSubject", "\u672a\u77e5\u4e3b\u9898" },
        { "config.unknownIssuer", "\u672a\u77e5\u53d1\u884c\u65b9" },
        { "config.certShowName", "{0}({1})" },
        { "config.certShowOOU", "{0} {1}" },
        { "config.proxy.autourl.invalid.text", "<html><b>\u6b8b\u7f3a\u7684 URL</b></html>\u81ea\u52a8\u4ee3\u7406\u914d\u7f6e URL \u65e0\u6548\u3002" },
        { "config.proxy.autourl.invalid.caption", "\u9519\u8bef - \u4ee3\u7406" },


	
	

	{ "jarcache.location", "\u4f4d\u7f6e" },
	{ "jarcache.select", "\u9009\u62e9" },
	{ "jarcache.kb", "KB" },
	{ "jarcache.bytes", "\u5b57\u8282" },
	{ "jarcache.clear", "\u6e05\u9664(C)" },
	{ "jarcache.clear.acceleratorKey", new Integer(KeyEvent.VK_C)}, 
        { "jarcache.view", "\u67e5\u770b(V)" }, 
        { "jarcache.view.acceleratorKey", new Integer(KeyEvent.VK_V) }, 
        { "jarcache.browseDirectory.acceleratorKey", new Integer(KeyEvent.VK_O) }, 
	{ "jarcache.no_compression", "\u65e0" },
	{ "jarcache.select_tooltip", "\u4f7f\u7528\u9009\u62e9\u7684\u4f4d\u7f6e" },
	{ "jarcache.select_mnemonic", "S" },
	{ "jarcache.maximum", "\u6700\u5927" },
	{ "jarcache.unlimited", "\u65e0\u9650\u5236\u7684" },
	{ "jarcache.high_compression", "\u9ad8" },
	{ "jarcache.compression", "Jar \u538b\u7f29" },
	{ "jarcache.mb", "MB" },
	{ "jarcache.size", "\u5927\u5c0f" },
	{ "jarcache.settings", "\u9ad8\u901f\u7f13\u5b58\u8bbe\u7f6e" },
	{ "jarcache.erase.confirm.caption", "\u9700\u8981\u786e\u8ba4 - \u9ad8\u901f\u7f13\u5b58" }, 
        { "jarcache.erase.confirm.text", "\u6e05\u9664 {0} \u4e2d\u7684\u6240\u6709\u6587\u4ef6?" }, 
	{ "jarcache.select_title", "\u9ad8\u901f\u7f13\u5b58\u4f4d\u7f6e" },
	{ "jarcache.enabled", "\u542f\u7528\u9ad8\u901f\u7f13\u5b58" },    
	{ "update.update_button.text", "\u7acb\u5373\u66f4\u65b0" },
	{ "update.advanced_button.text", "\u9ad8\u7ea7..." },
	{ "update.desc.text", "<html>Java(TM) Update \u673a\u5236\u786e\u4fdd\u60a8\u5177\u6709\u6700\u65b0\u7684 Java \u5e73\u53f0\u7248\u672c\u3002<br>\u60a8\u53ef\u4ee5\u4f7f\u7528\u4e0b\u9762\u7684\u9009\u9879\u6765\u63a7\u5236\u83b7\u53d6\u548c\u5e94\u7528\u66f4\u65b0\u7684\u8fc7\u7a0b\u3002</html>" },
	{ "update.notify.text", "\u901a\u77e5\u6211\uff1a" },
	{ "update.notify_install.text", "\u5b89\u88c5\u4e4b\u524d" },
	{ "update.notify_download.text", "\u4e0b\u8f7d\u4e4b\u524d\u548c\u5b89\u88c5\u4e4b\u524d" },
	{ "update.autoupdate.text", "\u81ea\u52a8\u68c0\u67e5\u66f4\u65b0" },
	{ "update.advanced_title.text", "\u81ea\u52a8\u66f4\u65b0\u9ad8\u7ea7\u8bbe\u7f6e" },
	{ "update.advanced_title1.text", "\u8bf7\u9009\u62e9\u626b\u63cf\u7684\u9891\u7387\u548c\u65f6\u95f4\u3002" },
	{ "update.advanced_title2.text", "\u9891\u7387" },
	{ "update.advanced_title3.text", "\u65f6\u95f4" },
	{ "update.advanced_desc1.text", "\u6bcf\u5929\u7684 {0} \u6267\u884c\u626b\u63cf" },
	{ "update.advanced_desc2.text", "\u6bcf{0}\u7684 {1} \u6267\u884c\u626b\u63cf" },
	{ "update.advanced_desc3.text", "\u6bcf\u6708 {0} \u65e5\u7684 {1} \u6267\u884c\u626b\u63cf" },
	{ "update.check_daily.text", "\u6bcf\u5929" },
	{ "update.check_weekly.text", "\u6bcf\u5468" },
	{ "update.check_monthly.text", "\u6bcf\u6708" },
	{ "update.check_date.text", "\u65e5\u671f\uff1a" },
	{ "update.check_day.text", "\u6bcf\uff1a" },
	{ "update.check_time.text", "\u5728\uff1a" },
	{ "update.lastrun.text", "Java Update \u6700\u540e\u4e8e {1} \u7684 {0} \u8fd0\u884c\u3002" },
	{ "update.desc_autooff.text", "<html>\u60a8\u5fc5\u987b\u4f7f\u7528\u4e0b\u9762\u7684\u201c\u7acb\u5373\u66f4\u65b0\u201d\u6309\u94ae\u6765\u68c0\u67e5\u66f4\u65b0\u3002<br></html>" },
	{ "update.desc_check_daily.text", "<html>\u6bcf\u5929\u7684 {0}" },
	{ "update.desc_check_weekly.text", "<html>\u6bcf{0}\u7684 {1}" },
	{ "update.desc_check_monthly.text", "<html>\u6bcf\u6708 {0} \u65e5\u7684 {1}" },
	{ "update.desc_check.text", "\uff0cJava Update \u5c06\u68c0\u67e5\u66f4\u65b0\u3002 " },
	{ "update.desc_notify.text", "\u5982\u679c\u53d1\u73b0<br>\u65b0\u7684\u66f4\u65b0\uff0c\u7cfb\u7edf\u4f1a\u5728 " },
	{ "update.desc_notify_install.text", "\u5b89\u88c5\u66f4\u65b0\u4e4b\u524d\u901a\u77e5\u60a8\u3002</html>" },
	{ "update.desc_notify_download.text", "\u4e0b\u8f7d\u548c\u5b89\u88c5\u66f4\u65b0\u4e4b\u524d\u901a\u77e5\u60a8\u3002</html>" },
	{ "update.launchbrowser.error.text", "<html><b>\u65e0\u6cd5\u542f\u52a8 Java Update \u68c0\u67e5\u7a0b\u5e8f</b></html>\u8981\u83b7\u5f97\u6700\u65b0\u7684 Java(TM) Update\uff0c\u8bf7\u8f6c\u81f3 http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "\u9519\u8bef \uff0d \u66f4\u65b0" }
    };
}

