/*
 * @(#)ControlPanel_sv.java	1.38 03/04/24
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Swedish version of ControlPanel strings.
 *
 * @author Jerome Dochez
 */

public class ControlPanel_sv extends ListResourceBundle {

    public Object[][] getContents() {
    return contents;
    }

    static final Object[][] contents = {
    { "advanced.jre_format", "JRE {0} i {1}" },
    { "advanced.jdk_format", "SDK {0} i {1}" },
    { "panel.about", "Om" },
    { "panel.basic", "Standard" },
    { "panel.advanced", "Avancerade" },
    { "panel.browser", "Webbl\u00e4sare" },
    { "panel.proxies", "Proxy-servrar" },
	{ "panel.cache", "Cache" },
    { "panel.cert", "Certifikat" },
	{ "panel.update", "Uppdatera" },
    { "panel.apply", "Utf\u00f6r" },
        { "panel.apply.acceleratorKey", new Integer(KeyEvent.VK_U)},
    { "panel.cancel", "\u00c5terst\u00e4ll" },
	{ "panel.cancel.acceleratorKey", new Integer(KeyEvent.VK_T)},
    { "panel.apply_failed", "Kunde inte skriva i egenskapsfil" },
    { "panel.apply_failed_title", "Utf\u00f6r misslyckades" },
	{ "panel.help", "Hj\u00e4lp" },
	{ "panel.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "panel.help_title", "Hj\u00e4lp - Kontrollpanelen f\u00f6r Java Plug-in" },
	{ "panel.help_close", "St\u00e4ng" },
	{ "panel.help_close.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "panel.help.error.text", "<html><b>Filen finns inte</b></html>Det g\u00e5r inte att ladda hj\u00e4lpfilen.\n" },
	{ "panel.help.error.caption", "Fel - Kontrollpanelen f\u00f6r Java Plug-in" },

    { "basic.show_exception", "Visa dialogruta f\u00f6r undantagsfel" },
    { "basic.recycle_classloader", "\u00c5teranv\u00e4nd klassladdare" },
    { "basic.java_console", "Java systemf\u00f6nster" },
    { "basic.show_console", "Visa systemf\u00f6nster" },
    { "basic.hide_console", "D\u00f6lj systemf\u00f6nster" },
    { "basic.no_console", "Starta inte systemf\u00f6nster" },
     { "basic.show_systray", "Visa Java i Aktivitetsf\u00e4ltet" },

    { "advanced.jre_name", "Java Runtime Environment" },
    { "advanced.path", "Annan SDK/JRE " },
    { "advanced.other_jdk", "Annan..." },
    { "advanced.default_jdk", "Anv\u00e4nd Java Plugin-standard" },
	{ "advanced.jre_selection_warning.info", "<html><b>\u00c5tg\u00e4rden st\u00f6ds inte</b></html>Att v\u00e4lja n\u00e5gon annan Java Runtime \u00e4n \"Standard\" kan inte rekommenderas.\n"},
	{ "advanced.jre_selection_warning.caption", "Varning - Avancerade"},
        { "advanced.error.caption", "Fel - Avancerade" },
        { "advanced.error.text", "<html><b>Katalogen finns inte</b></html>Kontrollera att det du har valt inte \u00e4r en fil eller en icke-existerande katalog.\n" },
    { "advanced.java_parms", "Java RunTime-parametrar" },
	{ "advanced.warning_popup_ok", "OK" },
	{ "advanced.warning_popup_cancel", "Avbryt" },
    
    { "browser.settings", "Inst\u00e4llningar"},
    { "browser.desc.text", "Java(TM) Plug-in kommer anv\u00e4ndas som standard Java Runtime i f\u00f6ljande webbl\u00e4sare:"},
    { "browser.ie.text", "Microsoft Internet Explorer"},
    { "browser.ns6.text", "Netscape 6 eller senare"},
       { "browser.moz.text", "Mozilla 1.1 eller senare"},
    
	{ "browser.settings.success.caption", "Inst\u00e4llningarna har \u00e4ndrats - Webbl\u00e4sare" },
	{ "browser.settings.fail.caption", "Varning - Webbl\u00e4sare" },

	{ "browser.settings.success.text", "<html><b>Webbl\u00e4sarens inst\u00e4llningar har \u00e4ndrats</b></html>\u00c4ndringarna tr\u00e4der i kraft n\u00e4r webbl\u00e4saren eller -l\u00e4sarna har startats om.\n" },

	{ "browser.settings.fail.ie.text", "<html><b>Det g\u00e5r inte att \u00e4ndra inst\u00e4llningarna f\u00f6r webbl\u00e4saren</b></html>Kontrollera att du har "
					+ "tillr\u00e4cklig beh\u00f6righet f\u00f6r att \u00e4ndra systeminst\u00e4llningar.\n" },

	{ "browser.settings.fail.ns6.text", "<html><b>Det g\u00e5r inte att \u00e4ndra inst\u00e4llningarna f\u00f6r webbl\u00e4saren</b></html>"
					+ "Kontrollera att Netscape 6 \u00e4r korrekt installerat i systemet och/eller "
					+ "att du har "
					+ "tillr\u00e4cklig beh\u00f6righet f\u00f6r att \u00e4ndra systeminst\u00e4llningar.\n" },
        { "browser.settings.fail.moz.text", "<html><b>Det g\u00e5r inte att \u00e4ndra inst\u00e4llningarna f\u00f6r webbl\u00e4saren</b></html>"
                                        + "Kontrollera att Mozilla \u00e4r korrekt installerat i systemet och/eller "
                                        + "att du har "
                                        + "tillr\u00e4cklig beh\u00f6righet f\u00f6r att \u00e4ndra systeminst\u00e4llningar.\n" },

	{ "browser.settings.alert.text", "<html><b>Det finns en nyare Java Runtime</b></html>Internet Explorer har redan tillg\u00e5ng till en nyare version av Java Runtime. Vill du ers\u00e4tta den?\n" },
    
    { "proxy.use_browser", "Inst\u00e4llningar"},
    { "proxy.proxy_settings", "Proxy-inst\u00e4llningar" },
    { "proxy.protocol_type", "Typ" },
    { "proxy.proxy_protocol", "Protokoll" },
    { "proxy.proxy_address", "Adress" },
    { "proxy.proxy_port", "Port" },
    { "proxy.http", "HTTP" },
    { "proxy.ftp", "FTP" },
    { "proxy.gopher", "Gopher" },
    { "proxy.https", "S\u00e4ker" },
    { "proxy.socks", "Socks" },
    { "proxy.same_for_all_protocols", "Samma proxy-server f\u00f6r alla protokoll" },
	{ "proxy.bypass", "Ingen proxyv\u00e4rd (anv\u00e4nd kommatecken som skiljetecken vid flera v\u00e4rdar)" },
	{ "proxy.autourl", "URL f\u00f6r automatisk proxykonfiguration" },

    { "cert.remove_button", "Ta bort" },
	{ "cert.remove_button.acceleratorKey", new Integer(KeyEvent.VK_B)},
    { "cert.import_button", "Importera" },
	{ "cert.import_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
    { "cert.export_button", "Exportera" },
	{ "cert.export_button.acceleratorKey", new Integer(KeyEvent.VK_E)},
    { "cert.details_button", "Detaljer" },
	{ "cert.details_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
    { "cert.viewcert_button", "Visa certifikat" },
	{ "cert.viewcert_button.acceleratorKey", new Integer(KeyEvent.VK_C)},
        { "cert.rbutton_signed_applet", "Signerad applet" },
	{ "cert.rbutton_secure_site", "S\u00e4ker site" },
    { "cert.rbutton_signer_ca", "CA f\u00f6r signerare" },
	{ "cert.rbutton_secure_site_ca", "CA f\u00f6r s\u00e4ker site" },
    { "cert.SignedApplet_value", "SigneradApplet"},
    { "cert.SecureSite_value", "S\u00e4kerSite"},
    { "cert.SignerCA_value", "SignerarCA"},
    { "cert.SecureSiteCA_value", "S\u00e4kerSiteCA"},
    { "cert.settings", "Certifikat"},
       { "cert.dialog.import.error.caption", "Fel - Importera certifikat" },
	{ "cert.dialog.import.format.text", "<html><b>Ok\u00e4nt filformat</b></html>Inget certifikat kommer att importeras." },
	{ "cert.dialog.import.file.text", "<html><b>Filen finns inte</b></html>Inget certifikat kommer att importeras." },
	{ "cert.dialog.import.password.text", "<html><b>Ogiltigt l\u00f6senord</b></html>Det angivna l\u00f6senordet \u00e4r felaktigt." },
	{ "cert.dialog.password.caption", "L\u00f6senord - Importera" },
	{ "cert.dialog.password.text", "<html><b>Ange l\u00f6senord f\u00f6r \u00e5tkomst till denna fil</b></html>" },
	{ "cert.dialog.password.okButton", "OK" },
	{ "cert.dialog.password.cancelButton", "Avbryt" },
	{ "cert.dialog.export.error.caption", "Fel - Exportera certifikat" },
	{ "cert.dialog.export.text", "<html><b>Det g\u00e5r inte att exportera</b></html>Inget certifikat exporteras." },
    
    { "main.control_panel_caption", "Kontrollpanelen f\u00f6r Java Plug-in" },

    // Each line in the property_file_header must start with "#"
    { "config.property_file_header", "# Egenskaper f\u00f6r Java(TM) Plug-in\n"
            + "# \u00c4NDRA INTE DENNA FIL. Den genereras automatiskt.\n"
            + "# Redigera egenskaperna via Kontrollpanelen Activator." },
	{ "config.unknownSubject", "Ok\u00e4nt \u00e4mne" },
	{ "config.unknownIssuer", "Ok\u00e4nd utf\u00e4rdare" },
	{ "config.certShowName", "{0}({1})" },
	{ "config.certShowOOU", "{0} {1}" },
	{ "config.proxy.autourl.invalid.text", "<html><b>Ogiltig URL</b></html>URL:en f\u00f6r automatisk proxykonfiguration \u00e4r ogiltig." },
	{ "config.proxy.autourl.invalid.caption", "Fel - Proxy" },

    { "jarcache.location", "Placering" },
    { "jarcache.select", "V\u00e4lj" },
    { "jarcache.kb", "kB" },
    { "jarcache.bytes", "byte" },
	{ "jarcache.clear", "Rensa" },
        { "jarcache.clear.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "jarcache.view", "Visa" },
	{ "jarcache.view.acceleratorKey", new Integer(KeyEvent.VK_V) },
        { "jarcache.browseDirectory.acceleratorKey", new Integer(KeyEvent.VK_O) },
    { "jarcache.no_compression", "Inget" },
    { "jarcache.select_tooltip", "Anv\u00e4nd vald placering" },
    { "jarcache.select_mnemonic", "S" },
    { "jarcache.maximum", "Max" },
    { "jarcache.unlimited", "Obegr\u00e4nsat" },
    { "jarcache.high_compression", "H\u00f6g" },
     { "jarcache.compression", "JAR-komprimering" },
    { "jarcache.mb", "MB" },
    { "jarcache.size", "Storlek" },
     { "jarcache.settings", "Inst\u00e4llningar f\u00f6r cache" },
	{ "jarcache.erase.confirm.caption", "Bekr\u00e4ftelse kr\u00e4vs - Cache" },
	{ "jarcache.erase.confirm.text", "Vill du radera alla filer i {0}?" },
     { "jarcache.select_title", "Placering av cache" },
     { "jarcache.enabled", "Aktivera cachning" },   

	{ "update.update_button.text", "Uppdatera nu" },
	{ "update.advanced_button.text", "Avancerat..." },
	{ "update.desc.text", "<html>Uppdateringsfunktionen i Java(TM) ser till att du alltid har den senaste versionen av Java-plattformen.<br>Med alternativen nedan kan du styra uppdateringsprocessens utformning och anv\u00e4ndning.</html>" },
        { "update.notify.text", "Meddela mig:" },
        { "update.notify_install.text", "F\u00f6re installation" },
        { "update.notify_download.text", "F\u00f6re nedladdning och f\u00f6re installation" },
        { "update.autoupdate.text", "S\u00f6k efter uppdateringar automatiskt" },
        { "update.advanced_title.text", "Automatisk uppdatering avancerade inst\u00e4llningar" },
        { "update.advanced_title1.text", "V\u00e4lj hur ofta och n\u00e4r du vill att s\u00f6kningen ska ske." },
        { "update.advanced_title2.text", "Frekvens" },
        { "update.advanced_title3.text", "N\u00e4r" },
        { "update.advanced_desc1.text", "G\u00f6r s\u00f6kning varje dag klockan {0}" },
        { "update.advanced_desc2.text", "G\u00f6r s\u00f6kning varje {0} klockan {1}" },
        { "update.advanced_desc3.text", "G\u00f6r s\u00f6kning dag {0} varje m\u00e5nad klockan {1}" },
        { "update.check_daily.text", "Dagligen" },
        { "update.check_weekly.text", "Varje vecka" },
        { "update.check_monthly.text", "M\u00e5natligen" },
        { "update.check_date.text", "Dag:" },
        { "update.check_day.text", "Var:" },
        { "update.check_time.text", "Kl.:" },
        { "update.lastrun.text", "Java Update k\u00f6rdes senast {0} p\u00e5 {1}." },
        { "update.desc_autooff.text", "<html>Du m\u00e5ste anv\u00e4nda knappen \"Uppdatera nu\" nedan f\u00f6r att s\u00f6ka efter uppdateringar.<br></html>" },
        { "update.desc_check_daily.text", "<html>Varje dag klockan {0}" },
        { "update.desc_check_weekly.text", "<html>Varje {0} klockan {1}" },
        { "update.desc_check_monthly.text", "<html>Dag {0} varje m\u00e5nad klockan {1}" },
        { "update.desc_check.text", ", Java Update s\u00f6ker efter uppdateringar. " },
        { "update.desc_notify.text", "Om nya uppdateringar <br>uppt\u00e4cks, meddelas du innan uppdateringen " },
        { "update.desc_notify_install.text", "installeras.</html>" },
        { "update.desc_notify_download.text", "laddas ned och innan den installeras.</html>" },
	{ "update.launchbrowser.error.text", "<html><b>Det g\u00e5r inte att starta Java Update Checker</b></html>Om du vill ha senaste versionen av Java(TM) Update, g\u00e5 till http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "Fel - uppdatering" },
    };
}

