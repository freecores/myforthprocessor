/*
 * @(#)ControlPanel_it.java	1.34 03/04/24 
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Italian verison of ControlPanel strings.
 *
 * @author Jerome Dochez
 */

public class ControlPanel_it extends ListResourceBundle {

    public Object[][] getContents() {
    return contents;
    }

    static final Object[][] contents = {
    { "advanced.jre_format", "JRE {0} in {1}" },
    { "advanced.jdk_format", "SDK {0} in {1}" },
    { "panel.about", "Informazioni su" },
    { "panel.basic", "Base" },
    { "panel.advanced", "Avanzate" },
    { "panel.browser", "Browser" },
    { "panel.proxies", "Proxy" },
	{ "panel.cache", "Cache" },
    { "panel.cert", "Certificati" },
	{ "panel.update", "Aggiorna" },
    { "panel.apply", "Applica" },
        { "panel.apply.acceleratorKey", new Integer(KeyEvent.VK_A)},
    { "panel.cancel", "Ripristina" },
	{ "panel.cancel.acceleratorKey", new Integer(KeyEvent.VK_R)},
    { "panel.apply_failed", "Impossibile scrivere sul file delle propriet\u00e0" },
    { "panel.apply_failed_title", "Applicazione non riuscita" },
	{ "panel.help", "Guida" },
	{ "panel.help.acceleratorKey", new Integer(KeyEvent.VK_G)},
	{ "panel.help_title", "Guida - Pannello di controllo di Java Plug-in" },
	{ "panel.help_close", "Chiudi" },
	{ "panel.help_close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "panel.help.error.text", "<html><b>File inesistente</b></html>Impossibile caricare il file della guida.\n" },
	{ "panel.help.error.caption", "Errore - Pannello di controllo di Java Plug-in" },

    { "basic.show_exception", "Mostra finestra di dialogo Eccezioni" },
    { "basic.recycle_classloader", "Ricicla il caricatore di classi" },
    { "basic.java_console", "Console Java" },
    { "basic.show_console", "Mostra console" },
    { "basic.hide_console", "Nascondi console" },
    { "basic.no_console", "Non avviare console" },
    { "basic.show_systray", "Mostra Java nella barra di sistema"},

    { "advanced.jre_name", "Java Runtime Environment" },
    { "advanced.path", "Altro SDK/JRE " },
    { "advanced.other_jdk", "Altro..." },
        { "advanced.default_jdk", "Utilizzare i valori predefiniti per Java Plug-in" },
	{ "advanced.jre_selection_warning.info", "<html><b>Operazione non supportata</b></html>Si sconsiglia di selezionare un Runtime Java diverso da \"Predefinito\".\n"},
	{ "advanced.jre_selection_warning.caption", "Attenzione - Avanzate"},
        { "advanced.error.caption", "Errore - Avanzate" },
        { "advanced.error.text", "<html><b>Directory inesistente</b></html>Verificare di non selezionare un file o una directory inesistente.\n" },
	{ "advanced.java_parms", "Parametri del Runtime Java" },
	{ "advanced.warning_popup_ok", "OK" },
	{ "advanced.warning_popup_cancel", "Annulla" },

	{ "browser.settings", "Impostazioni" },
	{ "browser.desc.text", "Java(TM) Plug-in sar\u00e0 utilizzato come Runtime Java predefinito nei seguenti browser:" },
	{ "browser.ie.text", "Microsoft Internet Explorer" },
	{ "browser.ns6.text", "Netscape 6 o versione successiva" },
       { "browser.moz.text", "Mozilla 1.1 o versione successiva"},

	{ "browser.settings.success.caption", "Riuscito - Browser" },
	{ "browser.settings.fail.caption", "Attenzione - Browser" },

	{ "browser.settings.success.text", "<html><b>Impostazioni browser modificate</b></html>Le modifiche avranno effetto dopo il riavvio del/dei browser.\n" },

	{ "browser.settings.fail.ie.text", "<html><b>Impossibile modificare le impostazioni del browser</b></html>Verificare di disporre "
					+ "dei permessi sufficienti per modificare le impostazioni di sistema.\n" },

	{ "browser.settings.fail.ns6.text", "<html><b>Impossibile modificare le impostazioni del browser</b></html>"
					+ "Verificare che Netscape 6 sia correttamente installato nel sistema e/o "
					+ "di disporre "
					+ "dei permessi sufficienti per modificare le impostazioni di sistema.\n" },

        { "browser.settings.fail.moz.text", "<html><b>Impossibile modificare le impostazioni del browser</b></html>"
                                        + "Verificare che Mozilla sia correttamente installato nel sistema e/o "
                                        + "di disporre "
                                        + "dei permessi sufficienti per modificare le impostazioni di sistema.\n" },

	{ "browser.settings.alert.text", "<html><b>Versione pi\u00f9 recente di Runtime Java gi\u00e0 presente</b></html>Internet Explorer dispone gi\u00e0 di una versione pi\u00f9 recente di Runtime Java. Sostituirla?\n" },

    { "proxy.use_browser", "Usa impostazioni browser" },
    { "proxy.proxy_settings", "Impostazioni proxy" },
    { "proxy.protocol_type", "Tipo" },
    { "proxy.proxy_protocol", "Protocollo" },
    { "proxy.proxy_address", "Indirizzo" },
    { "proxy.proxy_port", "Porta" },
    { "proxy.http", "HTTP" },
    { "proxy.ftp", "FTP" },
    { "proxy.gopher", "Gopher" },
    { "proxy.https", "Secure" },
    { "proxy.socks", "Socks" },
    { "proxy.same_for_all_protocols", "Stesso server proxy per tutti i protocolli" },
	{ "proxy.bypass", "Nessun host proxy (separare gli host con la virgola)" },
	{ "proxy.autourl", "URL configurazione proxy automatica" },

    { "cert.remove_button", "Rimuovi" },
	{ "cert.remove_button.acceleratorKey", new Integer(KeyEvent.VK_U)},
    { "cert.import_button", "Importa" },
	{ "cert.import_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
    { "cert.export_button", "Esporta" },
	{ "cert.export_button.acceleratorKey", new Integer(KeyEvent.VK_E)},
    { "cert.details_button", "Dettagli" },
	{ "cert.details_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
    { "cert.viewcert_button", "Visualizza certificato" },
	{ "cert.viewcert_button.acceleratorKey", new Integer(KeyEvent.VK_V)},
        { "cert.rbutton_signed_applet", "Applet firmata" },
	{ "cert.rbutton_secure_site", "Sito protetto" },
    { "cert.rbutton_signer_ca", "Firmatario CA" },
	{ "cert.rbutton_secure_site_ca", "Sito CA protetto" },
    { "cert.SignedApplet_value", "AppletFirmato" },
    { "cert.SecureSite_value", "SitoSicuro" },
    { "cert.SignerCA_value", "FirmatarioCA" },
    { "cert.SecureSiteCA_value", "SitoSicuroCA" },
    { "cert.settings", "Certificati" },
       { "cert.dialog.import.error.caption", "Errore - Importazione certificato" },
	{ "cert.dialog.import.format.text", "<html><b>Formato di file non riconosciuto</b></html>Non verr\u00e0 importato alcun certificato." },
	{ "cert.dialog.import.file.text", "<html><b>File inesistente</b></html>Non verr\u00e0 importato alcun certificato." },
	{ "cert.dialog.import.password.text", "<html><b>Password non valida</b></html>La password digitata non \u00e8 corretta." },
	{ "cert.dialog.password.caption", "Password - Importazione" },
	{ "cert.dialog.password.text", "<html><b>Digitare la password per accedere al file<b></html>" },
	{ "cert.dialog.password.okButton", "OK" },
	{ "cert.dialog.password.cancelButton", "Annulla" },
	{ "cert.dialog.export.error.caption", "Errore - Esportazione certificato" },
	{ "cert.dialog.export.text", "<html><b>Impossibile eseguire esportazione</b></html>Non \u00e8 stato esportato nessun certificato." },

    { "main.control_panel_caption", "Pannello di controllo di Java(TM) Plug-in" },

    // Each line in the property_file_header must start with "#"
    { "config.property_file_header", "# Propriet\u00e0 Java(TM) Plug-in\n"
            + "# NON MODIFICARE QUESTO FILE. \u00c8 un file generato dalla macchina.\n"
            + "# Usare il Pannello di controllo di Activator per modificare le propriet\u00e0." },
	{ "config.unknownSubject", "Soggetto sconosciuto" },
	{ "config.unknownIssuer", "Autorit\u00e0 emittente sconosciuta" },
	{ "config.certShowName", "{0}({1})" },
	{ "config.certShowOOU", "{0} {1}" },
	{ "config.proxy.autourl.invalid.text", "<html><b>URL errato</b></html>URL configurazione proxy automatico non valido." },
	{ "config.proxy.autourl.invalid.caption", "Errore - Proxy" },
     
     { "jarcache.location", "Percorso" },
	 { "jarcache.select", "Seleziona" },
	 { "jarcache.kb", "KB" },
	 { "jarcache.bytes", "byte" },
	{ "jarcache.clear", "Cancella" },
        { "jarcache.clear.acceleratorKey", new Integer(KeyEvent.VK_N)},
	{ "jarcache.view", "Visualizza" },
	{ "jarcache.view.acceleratorKey", new Integer(KeyEvent.VK_Z) },
        { "jarcache.browseDirectory.acceleratorKey", new Integer(KeyEvent.VK_O) },
	 { "jarcache.no_compression", "Nessuna" },
	 { "jarcache.select_tooltip", "Utilizzare il percorso selezionato" },
	 { "jarcache.select_mnemonic", "S" },
	 { "jarcache.maximum", "Massimo" },
	 { "jarcache.unlimited", "Illimitato" },
	 { "jarcache.high_compression", "Alta" },
	 { "jarcache.compression", "Compressione Jar" },
	 { "jarcache.mb", "MB" },
	 { "jarcache.size", "Dimensioni" },
	 { "jarcache.settings", "Impostazioni cache" },
	{ "jarcache.erase.confirm.caption", "Conferma necessaria - Cache" },
	{ "jarcache.erase.confirm.text", "Eliminare tutti i file nella {0}?" },
	 { "jarcache.select_title", "Percorso cache" },
	 { "jarcache.enabled", "Abilita cache" },   

	{ "update.update_button.text", "Aggiorna adesso" },
        { "update.advanced_button.text", "Avanzate..." },
        { "update.desc.text", "<html>L'utility di Aggiornamento Java(TM) consente di disporre della versione pi\u00f9 aggiornata della piattaforma Java.<br>Le opzioni di seguito consentono di controllare il processo di scaricamento e applicazione degli aggiornamenti.</html>" },
          { "update.notify.text", "Notifica:" },
          { "update.notify_install.text", "Prima dell'installazione" },
          { "update.notify_download.text", "Prima di scaricare e di installare" },
          { "update.autoupdate.text", "Cerca automaticamente i nuovi aggiornamenti" },
          { "update.advanced_title.text", "Impostazioni avanzate aggiornamento automatico" },
          { "update.advanced_title1.text", "Selezionare la frequenza e l'ora di esecuzione del controllo." },
          { "update.advanced_title2.text", "Frequenza" },
          { "update.advanced_title3.text", "Ora" },
          { "update.advanced_desc1.text", "Controlla ogni giorno alle {0}" },
          { "update.advanced_desc2.text", "Controlla ogni {0} alle {1}" },
          { "update.advanced_desc3.text", "Controlla il {0} di ogni mese alle {1}" },
          { "update.check_daily.text", "Giornalmente" },
          { "update.check_weekly.text", "Settimanalmente" },
          { "update.check_monthly.text", "Mensilmente" },
          { "update.check_date.text", "Giorno:" },
          { "update.check_day.text", "Ogni:" },
          { "update.check_time.text", "Alle:" },
          { "update.lastrun.text", "Ultima esecuzione dell''aggiornamento Java alle {0} il {1}." },
          { "update.desc_autooff.text", "<html>Per cercare gli aggiornamenti, utilizzare il pulsante \"Aggiorna ora\".<br></html>" },
          { "update.desc_check_daily.text", "<html>Ogni giorno alle {0}" },
          { "update.desc_check_weekly.text", "<html>Ogni {0} alle {1}" },
          { "update.desc_check_monthly.text", "<html>Il giorno {0} di ogni mese alle {1}" },
          { "update.desc_check.text", ", Aggiornamento Java cercher\u00e0 gli aggiornamenti. " },
          { "update.desc_notify.text", "Se vengono individuati <br>nuovi aggiornamenti, un messaggio notificher\u00e0 prima che l'aggiornamento sia " },
          { "update.desc_notify_install.text", "installato.</html>" },
          { "update.desc_notify_download.text", "scaricato e installato.</html>" },
         { "update.launchbrowser.error.text", "<html><b>Impossibile avviare il programma di controllo di Aggiornamento Java</b></html>Per scaricare l'Aggiornamento Java(TM) pi\u00f9 recente, visitare il sito http://java.sun.com/getjava/javaupdate" },
  	     { "update.launchbrowser.error.caption", "Errore - Aggiorna" },
    };
}



	
	

