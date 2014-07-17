/*
 * @(#)ControlPanel_de.java	1.37 03/04/24
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * German version of ControlPanel strings.
 *
 * @author Jerome Dochez
 */

public class ControlPanel_de extends ListResourceBundle {

    public Object[][] getContents() {
    return contents;
    }

    static final Object[][] contents = {
    { "advanced.jre_format", "JRE {0} in {1}" },
    { "advanced.jdk_format", "SDK {0} in {1}" },
    { "panel.about", "Info" },
    { "panel.basic", "Standard" },
    { "panel.advanced", "Erweitert" },
    { "panel.browser", "Browser" },
    { "panel.proxies", "Proxies" },
	{ "panel.cache", "Cache" },
    { "panel.cert", "Zertifikate" },
	{ "panel.update", "Aktualisieren" },
    { "panel.apply", "\u00dcbernehmen" },
        { "panel.apply.acceleratorKey", new Integer(KeyEvent.VK_B)},
    { "panel.cancel", "Zur\u00fccksetzen" },
	{ "panel.cancel.acceleratorKey", new Integer(KeyEvent.VK_Z)},
    { "panel.apply_failed", "Eigenschaftendatei konnte nicht geschrieben werden." },
    { "panel.apply_failed_title", "\u00dcbernehmen nicht m\u00f6glich" },
	{ "panel.help", "Hilfe" },
	{ "panel.help.acceleratorKey", new Integer(KeyEvent.VK_H)},
	{ "panel.help_title", "Hilfe - Java Plug-in Bedienungsfeld" },
	{ "panel.help_close", "Schlie\u00dfen" },
	{ "panel.help_close.acceleratorKey", new Integer(KeyEvent.VK_S)},
	{ "panel.help.error.text", "<html><b>Datei nicht vorhanden</b></html>Hilfedatei kann nicht geladen werden.\n" },
	{ "panel.help.error.caption", "Fehler - Java Plug-in Bedienungsfeld" },

    { "basic.show_exception", "Ausnahmedialogfeld anzeigen"},
    { "basic.recycle_classloader", "Klassenlader wiederverwenden" },
    { "basic.java_console", "Java-Konsole" },
    { "basic.show_console", "Konsole einblenden" },
    { "basic.hide_console", "Konsole ausblenden" },
    { "basic.no_console", "Konsole nicht starten" },
    { "basic.show_systray", "Java im System Tray anzeigen"},

    { "advanced.jre_name", "Java Runtime Environment" },
    { "advanced.path", "Anderes SDK/JRE " },
    { "advanced.other_jdk", "Sonstige..." },
    { "advanced.default_jdk", "Java Plug-in-Standard verwenden" },
	{ "advanced.jre_selection_warning.info", "<html><b>Operation nicht unterst\u00fctzt</b></html>Von der Auswahl eines anderen Java-Laufzeitprogramms als \"Default\" wird abgeraten.\n"},
	{ "advanced.jre_selection_warning.caption", "Warnung - Erweitert"},
        { "advanced.error.caption", "Fehler - Erweitert" },
        { "advanced.error.text", "<html><b>Verzeichnis nicht vorhanden</b></html>Stellen Sie sicher, dass nicht eine Datei oder ein nicht vorhandenes Verzeichnis ausgew\u00e4hlt ist.\n" },
	{ "advanced.java_parms", "Java-Laufzeitparameter" },
	{ "advanced.warning_popup_ok", "OK" },
	{ "advanced.warning_popup_cancel", "Abbrechen" },

	{ "browser.settings", "Einstellungen" },
	{ "browser.desc.text", "Java(TM) Plug-in wird als Standard-Java-Laufzeitprogramm bei folgenden Browsern verwendet:" },
	{ "browser.ie.text", "Microsoft Internet Explorer" },
	{ "browser.ns6.text", "Netscape 6 oder h\u00f6her" },
       { "browser.moz.text", "Mozilla 1.1 oder h\u00f6her"},

	{ "browser.settings.success.caption", "Erfolg - Browser" },
	{ "browser.settings.fail.caption", "Warnung - Browser" },

	{ "browser.settings.success.text", "<html><b>Browser-Einstellungen ge\u00e4ndert</b></html>Sie sind nach einem Neustart des/der Browser(s) g\u00fcltig.\n" },

	{ "browser.settings.fail.ie.text", "<html><b>Browser-Einstellungen konnten nicht ge\u00e4ndert werden</b></html>Pr\u00fcfen Sie, ob Sie "
					+ "ausreichende Rechte zum \u00c4ndern von Systemeinstellungen haben.\n" },

	{ "browser.settings.fail.ns6.text", "<html><b>Browser-Einstellungen konnten nicht ge\u00e4ndert werden</b></html>"
					+ "Pr\u00fcfen Sie, ob Netscape 6 ordnungsgem\u00e4\u00df auf dem System installiert ist und/oder "
					+ "ob Sie "
					+ "ausreichende Rechte zum \u00c4ndern von Systemeinstellungen haben.\n" },

        { "browser.settings.fail.moz.text", "<html><b>Browser-Einstellungen konnten nicht ge\u00e4ndert werden</b></html>"
                                        + "Pr\u00fcfen Sie, ob Mozilla ordnungsgem\u00e4\u00df auf dem System installiert ist und/oder "
                                        + "ob Sie "
                                        + "ausreichende Rechte zum \u00c4ndern von Systemeinstellungen haben.\n" },

	{ "browser.settings.alert.text", "<html><b>Neueres Java-Laufzeitprogramm vorhanden</b></html>Internet Explorer verf\u00fcgt bereits \u00fcber eine neuere Version des Java-Laufzeitprogramms. M\u00f6chten Sie sie ersetzen?\n" },

   { "proxy.use_browser", "Browser-Einstellungen verwenden" },
    { "proxy.proxy_settings", "Proxy-Einstellungen" },
    { "proxy.protocol_type", "Typ" },
    { "proxy.proxy_protocol", "Protokoll" },
    { "proxy.proxy_address", "Adresse" },
    { "proxy.proxy_port", "Anschluss" },
    { "proxy.http", "HTTP" },
    { "proxy.ftp", "FTP" },
    { "proxy.gopher", "Gopher" },
    { "proxy.https", "Sichern" },
    { "proxy.socks", "Socks" },
    { "proxy.same_for_all_protocols", "Identischer Proxy-Server f\u00fcr alle Protokolle" },
	{ "proxy.bypass", "Kein Proxy-Host (mehrere Hosts mit Kommata voneinander trennen)" },
	{ "proxy.autourl", "URL der automatischen Proxy-Konfiguration " },

    { "cert.remove_button", "Entfernen" },
	{ "cert.remove_button.acceleratorKey", new Integer(KeyEvent.VK_F)},
    { "cert.import_button", "Importieren" },
	{ "cert.import_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "cert.export_button", "Exportieren" },
	{ "cert.export_button.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "cert.details_button", "Details" },
	{ "cert.details_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
    { "cert.viewcert_button", "Zertifikat anzeigen" },
	{ "cert.viewcert_button.acceleratorKey", new Integer(KeyEvent.VK_A)},
	{ "cert.rbutton_signed_applet", "Signiertes Applet" },
	{ "cert.rbutton_secure_site", "Sichere Site" },
	{ "cert.rbutton_signer_ca", "Zertifizierungsstellen-Signierer" },
	{ "cert.rbutton_secure_site_ca", "Zertifizierungsstelle der sicheren Site" },
	{ "cert.SignedApplet_value", "SignedApplet" },
	{ "cert.SecureSite_value", "SecureSite" },
	{ "cert.SignerCA_value", "SignerCA" },
	{ "cert.SecureSiteCA_value", "SecureSiteCA" },
	{ "cert.settings", "Zertifikate" },
       { "cert.dialog.import.error.caption", "Fehler - Import des Zertifikats" },
	{ "cert.dialog.import.format.text", "<html><b>Unbekanntes Dateiformat</b></html>Es wird kein Zertifikat importiert." },
	{ "cert.dialog.import.file.text", "<html><b>Datei nicht vorhanden</b></html>Es wird kein Zertifikat importiert." },
	{ "cert.dialog.import.password.text", "<html><b>Ung\u00fcltiges Passwort</b></html>Das eingegebene Passwort ist falsch." },
	{ "cert.dialog.password.caption", "Passwort - Import" },
	{ "cert.dialog.password.text", "<html><b>Geben Sie ein Passwort ein f\u00fcr den Zugriff auf diese Datei:<b></html>" },
	{ "cert.dialog.password.okButton", "OK" },
	{ "cert.dialog.password.cancelButton", "Abbrechen" },
	{ "cert.dialog.export.error.caption", "Fehler - Export des Zertifikats" },
	{ "cert.dialog.export.text", "<html><b>Exportieren nicht m\u00f6glich</b></html>Kein Zertifikat exportiert." },

    { "main.control_panel_caption", "Java(TM) Plug-in Bedienungsfeld" },

    // Each line in the property_file_header must start with "#"
    { "config.property_file_header", "# Java(TM) Plug-in-Eigenschaften\n"
            + "# BEARBEITEN SIE DIESE DATEI NICHT. Sie wird automatisch generiert.\n"
            + "# Verwenden Sie das Control Panel des Activators, um die Eigenschaften zu bearbeiten." },
	{ "config.unknownSubject", "Unbekannter Betreff" },
	{ "config.unknownIssuer", "Unbekannter Aussteller" },
	{ "config.certShowName", "{0}({1})" },
	{ "config.certShowOOU", "{0} {1}" },
	{ "config.proxy.autourl.invalid.text", "<html><b>URL-Format fehlerhaft</b></html>URL der automatischen Proxy-Konfiguration ist ung\u00fcltig." },
	{ "config.proxy.autourl.invalid.caption", "Fehler - Proxies" },

    { "jarcache.location", "Speicherort" },
    { "jarcache.select", "Ausw\u00e4hlen" },
    { "jarcache.kb", "KB" },
    { "jarcache.bytes", "Byte" },
	{ "jarcache.clear", "L\u00f6schen" },
        { "jarcache.clear.acceleratorKey", new Integer(KeyEvent.VK_L)},
	{ "jarcache.view", "Anzeigen" },
	{ "jarcache.view.acceleratorKey", new Integer(KeyEvent.VK_N) },
        { "jarcache.browseDirectory.acceleratorKey", new Integer(KeyEvent.VK_O) },
    { "jarcache.no_compression", "Keine" },
    { "jarcache.select_tooltip", "Ausgew\u00e4hlten Speicherort verwenden" },
    { "jarcache.select_mnemonic", "S" },
    { "jarcache.maximum", "Maximal" },
    { "jarcache.unlimited", "Unbegrenzt" },
    { "jarcache.high_compression", "Hoch" },
    { "jarcache.compression", "JAR-Kompression" },
    { "jarcache.mb", "MB" },
    { "jarcache.size", "Gr\u00f6\u00dfe" },
    { "jarcache.settings", "Cache-Einstellungen" },
	{ "jarcache.erase.confirm.caption", "Best\u00e4tigung erforderlich - Cache" },
	{ "jarcache.erase.confirm.text", "Alle Dateien in {0} l\u00f6schen?" },
    { "jarcache.select_title", "Speicherort des Cache" },
    { "jarcache.enabled", "Cache aktivieren" },

	{ "update.update_button.text", "Jetzt aktualisieren" },
	{ "update.advanced_button.text", "Erweitert..." },
	{ "update.desc.text", "<html>Der Java(TM)-Update-Mechanismus gew\u00e4hrleistet, dass Sie stets \u00fcber die aktuelle Version der Java-Plattform verf\u00fcgen.<br>Mit den Optionen unten k\u00f6nnen Sie festlegen, wie Updates bezogen und in Ihr System \u00fcbernommen werden.</html>" },
        { "update.notify.text", "Benachrichtigen:" },
        { "update.notify_install.text", "Vor der Installation" },
        { "update.notify_download.text", "Vor dem Herunterladen und vor der Installation" },
        { "update.autoupdate.text", "Automatisch nach Updates suchen" },
        { "update.advanced_title.text", "Erweiterte Einstellungen f\u00fcr die automatische Aktualisierung" },
        { "update.advanced_title1.text", "W\u00e4hlen Sie aus, wie oft und zu welchem Zeitpunkt gesucht werden soll." },
        { "update.advanced_title2.text", "H\u00e4ufigkeit" },
        { "update.advanced_title3.text", "Zeitpunkt" },
        { "update.advanced_desc1.text", "Jeden Tag um {0} suchen" },
        { "update.advanced_desc2.text", "Jeden {0} um {1} suchen" },
        { "update.advanced_desc3.text", "An Tag {0} jedes Monats um {1} suchen" },
        { "update.check_daily.text", "T\u00e4glich" },
        { "update.check_weekly.text", "W\u00f6chentlich" },
        { "update.check_monthly.text", "Monatlich" },
        { "update.check_date.text", "Tag:" },
        { "update.check_day.text", "Jeden:" },
        { "update.check_time.text", "Um:" },
        { "update.lastrun.text", "Java-Update wurde zuletzt um {0} am {1} ausgef\u00fchrt." },
        { "update.desc_autooff.text", "<html>Sie m\u00fcssen die Schaltfl\u00e4che \"Jetzt aktualisieren\" unten verwenden, um nach Updates zu suchen.<br></html>" },
        { "update.desc_check_daily.text", "<html>Jeden Tag um {0}" },
        { "update.desc_check_weekly.text", "<html>Jeden {0} um {1}" },
        { "update.desc_check_monthly.text", "<html>An Tag {0} jedes Monats um {1}" },
        { "update.desc_check.text", "sucht Java-Update nach Updates. " },
        { "update.desc_notify.text", "Wenn neue Updates <br>vorliegen, werden Sie benachrichtigt, bevor das Update " },
        { "update.desc_notify_install.text", "installiert wird.</html>" },
        { "update.desc_notify_download.text", "heruntergeladen und bevor es installiert wird.</html>" },
	{ "update.launchbrowser.error.text", "<html><b>Java Update Checker kann nicht ausgef\u00fchrt werden.</b></html>Um das neueste Java(TM)-Update zu erhalten, besuchen Sie http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "Fehler - Update" },
    };
}

