/*
 * @(#)ControlPanel_fr.java	1.43 03/05/16
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * French verison of ControlPanel strings.
 *
 * @author Jerome Dochez
 */

public class ControlPanel_fr extends ListResourceBundle {

    public Object[][] getContents() {
    return contents;
    }

    static final Object[][] contents = {
    { "advanced.jre_format", "JRE {0} dans {1}" },
    { "advanced.jdk_format", "SDK {0} dans {1}" },
    { "panel.about", "\u00c0 propos"  },
    { "panel.basic", "Propri\u00e9t\u00e9s de base" },
    { "panel.advanced", "Propri\u00e9t\u00e9s avanc\u00e9es" },
    { "panel.browser", "Navigateur" },
    { "panel.proxies", "Proxies" },
	{ "panel.cache", "Cache" },
    { "panel.cert", "Certificats" },
	{ "panel.update", "Mettre \u00e0 jour" },
    { "panel.apply", "Appliquer" },
        { "panel.apply.acceleratorKey", new Integer(KeyEvent.VK_A)},
    { "panel.cancel", "Restaurer" },
	{ "panel.cancel.acceleratorKey", new Integer(KeyEvent.VK_T)},
    { "panel.apply_failed", "Impossible d'\u00e9crire le fichier de propri\u00e9t\u00e9s" },
    { "panel.apply_failed_title", "Echec de la commande Appliquer" },
	{ "panel.help", "Aide" },
	{ "panel.help.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "panel.help_title", "Aide - Panneau de configuration Java Plug-in" },
	{ "panel.help_close", "Fermer" },
	{ "panel.help_close.acceleratorKey", new Integer(KeyEvent.VK_R)},
	{ "panel.help.error.text", "<html><b>Le fichier n'existe pas</b></html>Impossible de charger le fichier d'aide.\n" },
	{ "panel.help.error.caption", "Erreur - Panneau de configuration Java Plug-in" },

    { "basic.show_exception", "Afficher la bo\u00eete de dialogue Exception" },
    { "basic.recycle_classloader", "Recycler le chargeur de classes" },
    { "basic.java_console", "Console Java" },
    { "basic.show_console", "Afficher la console" },
    { "basic.hide_console", "Masquer la console" },
    { "basic.no_console", "Ne pas d\u00e9marrer la console" },
    { "basic.show_systray", "Afficher Java dans la barre d'\u00e9tat du syst\u00e8me"},    

    { "advanced.jre_name", "Java Runtime Environment" },
    { "advanced.path", "Autre SDK/JRE " },
    { "advanced.other_jdk", "Autres..." },
        { "advanced.default_jdk", "Utiliser le plug-in Java par d\u00e9faut" },
	{ "advanced.jre_selection_warning.info", "<html><b>Op\u00e9ration non prise en charge</b></html>La s\u00e9lection d'un Java Runtime autre que \"celui par d\u00e9faut\" n'est pas recommand\u00e9e.\n"},
	{ "advanced.jre_selection_warning.caption", "Avertissement - Propri\u00e9t\u00e9s avanc\u00e9es"},
        { "advanced.error.caption", "Erreur - Propri\u00e9t\u00e9s avanc\u00e9es" },
        { "advanced.error.text", "<html><b>Le r\u00e9pertoire n'existe pas</b></html>Assurez-vous que la s\u00e9lection n'est pas un fichier ou un r\u00e9pertoire qui n'existe pas.\n" },
	{ "advanced.java_parms", "Param\u00e8tres de Java Runtime" },
	{ "advanced.warning_popup_ok", "OK" },
	{ "advanced.warning_popup_cancel", "Annuler" },

	{ "browser.settings", "Param\u00e8tres" },
	{ "browser.desc.text", "Java(TM) Plug-in sera utilis\u00e9 en tant que Java Runtime par d\u00e9faut dans le(s) navigateur(s) suivant(s) :" },
	{ "browser.ie.text", "Microsoft Internet Explorer" },
	{ "browser.ns6.text", "Netscape 6 ou version ult\u00e9rieure" },
       { "browser.moz.text", "Mozilla 1.1 ou version ult\u00e9rieure"},

	{ "browser.settings.success.caption", "Succ\u00e8s - Navigateur" },
	{ "browser.settings.fail.caption", "Avertissement - Navigateur" },

	{ "browser.settings.success.text", "<html><b>Les param\u00e8tres du navigateur ont \u00e9t\u00e9 modifi\u00e9s</b></html>Ces modifications prendront effet une fois le(s) navigateur(s) red\u00e9marr\u00e9(s).\n" },

	{ "browser.settings.fail.ie.text", "<html><b>Impossible de modifier les param\u00e8tres du navigateur</b></html>Assurez-vous de disposer "
					+ "de droits suffisants pour effectuer cette op\u00e9ration.\n" },

	{ "browser.settings.fail.ns6.text", "<html><b>Impossible de modifier les param\u00e8tres du navigateur</b></html>"
					+ "V\u00e9rifiez que Netscape 6 est correctement install\u00e9 sur votre syst\u00e8me et/ou "
					+ "que vous disposez "
					+ "de droits suffisants pour effectuer cette op\u00e9ration.\n" },

        { "browser.settings.fail.moz.text", "<html><b>Impossible de modifier les param\u00e8tres du navigateur</b></html>"
                                        + "V\u00e9rifiez que Mozilla est correctement install\u00e9 sur votre syst\u00e8me et/ou "
                                        + "que vous disposez "
                                        + "de droits suffisants pour effectuer cette op\u00e9ration.\n" },

	{ "browser.settings.alert.text", "<html><b>Une version plus r\u00e9cente de Java Runtime existe</b></html>Internet Explorer dispose d\u00e9j\u00e0 d'une version plus r\u00e9cente de Java Runtime. Souhaitez-vous la remplacer ?\n" },

    { "proxy.use_browser", "Utiliser les param\u00e8tres du navigateur" },
    { "proxy.proxy_settings", "Param\u00e8tres du proxy" },
    { "proxy.protocol_type", "Type" },
    { "proxy.proxy_protocol", "Protocole" },
    { "proxy.proxy_address", "Adresse" },
    { "proxy.proxy_port", "Port" },
    { "proxy.http", "HTTP" },
    { "proxy.ftp", "FTP" },
    { "proxy.gopher", "Gopher" },
    { "proxy.https", "Secure" },
    { "proxy.socks", "Socks" },
    { "proxy.same_for_all_protocols", "M\u00eame serveur proxy pour tous les protocoles" },
	{ "proxy.bypass", "Pas d'h\u00f4te proxy (utilisez des virgules pour s\u00e9parer plusieurs h\u00f4tes.)" },
	{ "proxy.autourl", "URL de configuration automatique du proxy" },

    { "cert.remove_button", "Supprimer" },
	{ "cert.remove_button.acceleratorKey", new Integer(KeyEvent.VK_M)},
    { "cert.import_button", "Importer" },
	{ "cert.import_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
    { "cert.export_button", "Exporter" },
	{ "cert.export_button.acceleratorKey", new Integer(KeyEvent.VK_X)},
    { "cert.details_button", "D\u00e9tails" },
	{ "cert.details_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
    { "cert.viewcert_button", "Afficher le certificat" },
	{ "cert.viewcert_button.acceleratorKey", new Integer(KeyEvent.VK_C)},
        { "cert.rbutton_signed_applet", "Applet sign\u00e9" },
	{ "cert.rbutton_secure_site", "Site s\u00e9curis\u00e9" },
    { "cert.rbutton_signer_ca", "CA signataire" },
	{ "cert.rbutton_secure_site_ca", "CA de site s\u00e9curis\u00e9" },
    { "cert.SignedApplet_value", "SignedApplet" },
    { "cert.SecureSite_value", "SecureSite" },
    { "cert.SignerCA_value", "SignerCA" },
    { "cert.SecureSiteCA_value", "SecureSiteCA" },
    { "cert.settings", "Certificats" },
       { "cert.dialog.import.error.caption", "Erreur - Importer un certificat" },
	{ "cert.dialog.import.format.text", "<html><b>Format de fichier non reconnu</b></html>Aucun certificat ne sera import\u00e9." },
	{ "cert.dialog.import.file.text", "<html><b>Le fichier n'existe pas</b></html>Aucun certificat ne sera import\u00e9." },
	{ "cert.dialog.import.password.text", "<html><b>Mot de passe incorrect.</b></html>Le mot de passe que vous avez entr\u00e9 est incorrect." },
	{ "cert.dialog.password.caption", "Mot de passe - Importer" },
	{ "cert.dialog.password.text", "<html><b>Entrez un mot de passe pour acc\u00e9der \u00e0 ce fichier :<b></html>" },
	{ "cert.dialog.password.okButton", "OK" },
	{ "cert.dialog.password.cancelButton", "Annuler" },
	{ "cert.dialog.export.error.caption", "Erreur - Exporter un certificat" },
	{ "cert.dialog.export.text", "<html><b>Impossible d'exporter</b></html>Aucun certificat n'est export\u00e9." },

    { "main.control_panel_caption", "Panneau de configuration du plug-in Java(TM)" },

    // Each line in the property_file_header must start with "#"
    { "config.property_file_header", "# Propri\u00e9t\u00e9s du plug-in Java(TM)\n"
            + "# NE MODIFIEZ PAS CE FICHIER. Il est g\u00e9n\u00e9r\u00e9 automatiquement.\n"
            + "# Utilisez le panneau de configuration de l'activateur pour modifier les propri\u00e9t\u00e9s." },
	{ "config.unknownSubject", "Sujet inconnu" },
	{ "config.unknownIssuer", "Emetteur inconnu" },
	{ "config.certShowName", "{0}({1})" },
	{ "config.certShowOOU", "{0} {1}" },
	{ "config.proxy.autourl.invalid.text", "<html><b>URL mal form\u00e9e</b></html>L'URL de configuration automatique du proxy n'est pas valide." },
	{ "config.proxy.autourl.invalid.caption", "Erreur - Proxies" },
			
  	{ "jarcache.location", "Emplacement" },
	{ "jarcache.select", "S\u00e9lectionner" },
	{ "jarcache.kb", "Ko" },
	{ "jarcache.bytes", "octets" },
	{ "jarcache.clear", "Effacer" },
        { "jarcache.clear.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "jarcache.view", "Afficher" },
	{ "jarcache.view.acceleratorKey", new Integer(KeyEvent.VK_F) },
        { "jarcache.browseDirectory.acceleratorKey", new Integer(KeyEvent.VK_O) },
	{ "jarcache.no_compression", "Aucune" },
	{ "jarcache.select_tooltip", "Utiliser l'emplacement s\u00e9lectionn\u00e9" },
	{ "jarcache.select_mnemonic", "S" },
	{ "jarcache.maximum", "Maximum" },
	{ "jarcache.unlimited", "Illimit\u00e9e" },
	{ "jarcache.high_compression", "Elev\u00e9e" },
	{ "jarcache.compression", "Compression JAR" },
	{ "jarcache.mb", "Mo" },
	{ "jarcache.size", "Taille" },
	{ "jarcache.settings", "Param\u00e8tres du cache" },
	{ "jarcache.erase.confirm.caption", "Confirmation requise - Cache" },
	{ "jarcache.erase.confirm.text", "Effacer tous les fichiers contenus dans {0}?" },
	{ "jarcache.select_title", "Emplacement du cache" },
	{ "jarcache.enabled", "Activer la mise en cache" },   

	{ "update.update_button.text", "Mettre \u00e0 jour maintenant" },
	{ "update.advanced_button.text", "Avanc\u00e9..." },
	{ "update.desc.text", "<html>Le m\u00e9canisme de Java(TM) Update vous offre la derni\u00e8re version de la plateforme Java.<br>Les options ci-dessous vous permettent de contr\u00f4ler le processus d'obtention et d'application des mises \u00e0 jour.</html>" },
	{ "update.notify.text", "Me pr\u00e9venir :" },
	{ "update.notify_install.text", "Avant d'installer" },
	{ "update.notify_download.text", "Avant de t\u00e9l\u00e9chager et d'installer" },
	{ "update.autoupdate.text", "V\u00e9rification automatique des mises \u00e0 jour" },
	{ "update.advanced_title.text", "Param\u00e8tres avanc\u00e9s de mise \u00e0 jour automatique" },
	{ "update.advanced_title1.text", "Choisissez la fr\u00e9quence et le moment de la num\u00e9risation." },
	{ "update.advanced_title2.text", "Fr\u00e9quence" },
	{ "update.advanced_title3.text", "Quand" },
	{ "update.advanced_desc1.text", "Effectuer la num\u00e9risation tous les jours \u00e0 {0} " },
	{ "update.advanced_desc2.text", "Effectuer la num\u00e9risation tous les {0} \u00e0 {1} " },
	{ "update.advanced_desc3.text", "Effectuez la num\u00e9risation le {0} de chaque mois \u00e0 {1}" },
	{ "update.check_daily.text", "Quotidien" },
	{ "update.check_weekly.text", "Hebdomadaire" },
	{ "update.check_monthly.text", "Mensuel" },
	{ "update.check_date.text", "Jour :" },
	{ "update.check_day.text", "Tous les :" },
	{ "update.check_time.text", "A :" },
	{ "update.lastrun.text", "La derni\u00e8re ex\u00e9cution de Java Update a eu lieu \u00e0 {0} le {1}." },
	{ "update.desc_autooff.text", "<html>Vous devez cliquez sur le bouton \"Mettre \u00e0 jour maintenant\" ci-dessous pour v\u00e9rifier l'existence des mises \u00e0 jour.<br></html>" },
	{ "update.desc_check_daily.text", "<html>Tous les jours \u00e0 {0}" },
	{ "update.desc_check_weekly.text", "<html>Tous les {0} \u00e0 {1}" },
	{ "update.desc_check_monthly.text", "<html>Le {0} de chaque mois \u00e0 {1}" },
	{ "update.desc_check.text", ", Java Update va v\u00e9rifier l'existence des mises \u00e0 jour. " },
	{ "update.desc_notify.text", "Si de nouvelles mises \u00e0 jour <br>sont disponibles, vous serez pr\u00e9venu avant que la mise \u00e0 jour soit " },
	{ "update.desc_notify_install.text", "install\u00e9e.</html>" },
	{ "update.desc_notify_download.text", "t\u00e9l\u00e9charg\u00e9e et install\u00e9e.</html>" },
	{ "update.launchbrowser.error.text", "<html><b>Impossible de lancer Java Update Checker</b></html>Pour obtenir la derni\u00e8re version de Java(TM) Update, veuillez consulter le site Web http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "Erreur - Mise \u00e0 jour" },
    };
}

