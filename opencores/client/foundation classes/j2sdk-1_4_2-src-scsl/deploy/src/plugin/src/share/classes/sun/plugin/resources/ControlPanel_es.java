/*
 * @(#)ControlPanel_es.java	1.34 03/04/24
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Spanish version of ControlPanel strings.
 *
 * @author Jerome Dochez
 */

public class ControlPanel_es extends ListResourceBundle {

    public Object[][] getContents() {
    return contents;
    }

    static final Object[][] contents = {
	{ "advanced.jre_format", "JRE {0} en {1}" },
    { "advanced.jdk_format", "SDK {0} en {1}" },
    { "panel.basic", "B\u00e1sico" },
    { "panel.about", "Acerca de" },
    { "panel.advanced", "Avanzado" },
    { "panel.browser", "Navegador" },
    { "panel.proxies", "Proxies" },
    { "panel.cache", "Antememoria" },
    { "panel.cert", "Certificados" },
    { "panel.update", "Actualizar" },
    { "panel.apply", "Aplicar" },
    { "panel.apply.acceleratorKey", new Integer(KeyEvent.VK_A)},
    { "panel.cancel", "Restablecer" },
    { "panel.cancel.acceleratorKey", new Integer(KeyEvent.VK_R)},
    { "panel.apply_failed", "No se pudo escribir el archivo de propiedades" },
    { "panel.apply_failed_title", "No se pudo aplicar" },
	{ "panel.help", "Ayuda" },
	{ "panel.help.acceleratorKey", new Integer(KeyEvent.VK_Y)},
	{ "panel.help_title", "Ayuda - Panel de control de Java Plug-in" },
	{ "panel.help_close", "Cerrar" },
	{ "panel.help_close.acceleratorKey", new Integer(KeyEvent.VK_C)},
	{ "panel.help.error.text", "<html><b>El archivo no existe</b></html>No se puede cargar el archivo de ayuda.\n" },
	{ "panel.help.error.caption", "Error - Panel de control de Java Plug-in" },

	{ "basic.show_exception", "Cuadro de di\u00e1logo Mostrar excepci\u00f3n" },
	{ "basic.recycle_classloader", "Reciclar cargador de clase" },
	{ "basic.java_console", "Consola de Java" },
	{ "basic.show_console", "Mostrar consola" },
	{ "basic.hide_console", "Ocultar consola" },
	{ "basic.no_console", "No iniciar consola" },
	{ "basic.show_systray", "Mostrar Java en la bandeja del sistema"},

    { "advanced.jre_name", "Java Runtime Environment" },
    { "advanced.path", "Otros SDK/JRE " },
    { "advanced.other_jdk", "Otro..." },
    { "advanced.default_jdk", "Utilizar el valor predeterminado del Java Plug-in" },
	{ "advanced.jre_selection_warning.info", "<html><b>Operaci\u00f3n no admitida</b></html>No se recomienda seleccionar un Runtime de Java distinto al \"Predeterminado\".\n"},
	{ "advanced.jre_selection_warning.caption", "Advertencia - Avanzado"},
        { "advanced.error.caption", "Error - Avanzado" },
        { "advanced.error.text", "<html><b>El directorio no existe</b></html>Aseg\u00farese de no haber seleccionado un archivo o un directorio inexistente.\n" },
    { "advanced.java_parms", "Par\u00e1metros del Runtime de Java" },
	{ "advanced.warning_popup_ok", "Aceptar" },
	{ "advanced.warning_popup_cancel", "Cancelar" },

    { "browser.settings", "Valores" },
    { "browser.desc.text", "Java(TM) Plug-in se utilizar\u00e1 como valor predeterminado del Runtime de Java en los siguientes navegadores:" },
    { "browser.ie.text", " Microsoft Internet Explorer " },
    { "browser.ns6.text", "Netscape 6 o superior" },
       { "browser.moz.text", "Mozilla 1.1 o superior"},

	{ "browser.settings.success.caption", "Satisfactorio - Navegador" },
	{ "browser.settings.fail.caption", "Advertencia - Navegador" },

	{ "browser.settings.success.text", "<html><b>Los valores del navegador han cambiado.</b></html>Los cambios entrar\u00e1n en vigor despu\u00e9s de reiniciar el navegador o los navegadores.\n" },

	{ "browser.settings.fail.ie.text", "<html><b>Imposible cambiar los valores del navegador</b></html>Compruebe si tiene "
					+ "permiso para modificar los valores del sistema.\n" },

	{ "browser.settings.fail.ns6.text", "<html><b>Imposible cambiar los valores del navegador</b></html>"
					+ "Compruebe si Netscape 6 est\u00e1 instalado correctamente en el sistema y/o "
					+ "si tiene "
					+ "permiso para modificar los valores del sistema.\n" },

        { "browser.settings.fail.moz.text", "<html><b>No se pueden cambiar los valores del navegador</b></html>"
                                        + "Compruebe si Mozilla est\u00e1 instalado correctamente en el sistema y/o  "
                                        + "si tiene "
                                        + "permisos suficientes para modificar los valores del sistema.\n" },

	{ "browser.settings.alert.text", "<html><b>Existe una versi\u00f3n m\u00e1s reciente del Runtime de Java</b></html>Internet Explorer ya tiene una versi\u00f3n m\u00e1s reciente del Runtime de Java. \u00bfDesea sustituirla?\n" },

    { "proxy.use_browser", "Utilizar valores de navegador" },
    { "proxy.proxy_settings", "Valores proxy" },
    { "proxy.protocol_type", "Tipo" },
    { "proxy.proxy_protocol", "Protocolo" },
    { "proxy.proxy_address", "Direcci\u00f3n" },
    { "proxy.proxy_port", "Puerto" },
    { "proxy.http", "HTTP" },
    { "proxy.ftp", "FTP" },
    { "proxy.gopher", "Gopher" },
    { "proxy.https", "Seguro" },
    { "proxy.socks", "Z\u00f3calos" },
    { "proxy.same_for_all_protocols", "El mismo servidor proxy para todos los protocolos" },
	{ "proxy.bypass", "No hay ning\u00fan servidor proxy. (Los host deben ir separados por comas)" },
	{ "proxy.autourl", "URL de configuraci\u00f3n delegada autom\u00e1tica" },

    { "cert.remove_button", "Eliminar" },
	{ "cert.remove_button.acceleratorKey", new Integer(KeyEvent.VK_L)},
    { "cert.import_button", "Importar" },
	{ "cert.import_button.acceleratorKey", new Integer(KeyEvent.VK_I)},
	{ "cert.export_button", "Exportar" },
	{ "cert.export_button.acceleratorKey", new Integer(KeyEvent.VK_E)},
	{ "cert.details_button", "Detalles" },
	{ "cert.details_button.acceleratorKey", new Integer(KeyEvent.VK_D)},
	{ "cert.viewcert_button", "Ver certificado" },
	{ "cert.viewcert_button.acceleratorKey", new Integer(KeyEvent.VK_T)},
        { "cert.rbutton_signed_applet", "Applet firmado" },
	{ "cert.rbutton_secure_site", "Sitio seguro" },
	{ "cert.rbutton_signer_ca", "Firmante de CA" },
	{ "cert.rbutton_secure_site_ca", "Sitio seguro de CA" },
	{ "cert.SignedApplet_value", "SignedApplet" },
	{ "cert.SecureSite_value", "SecureSite" },
	{ "cert.SignerCA_value", "SignerCA" },
	{ "cert.SecureSiteCA_value", "SecureSiteCA" },
	{ "cert.settings", "Certificados" },
       { "cert.dialog.import.error.caption", "Error - Importar certificado" },
	{ "cert.dialog.import.format.text", "<html><b>No se reconoce el formato del archivo</b></html>No se importar\u00e1 ning\u00fan certificado." },
	{ "cert.dialog.import.file.text", "<html><b>El archivo no existe</b></html>No se importar\u00e1 ning\u00fan certificado." },
	{ "cert.dialog.import.password.text", "<html><b>La contrase\u00f1a no es v\u00e1lida</b></html>La contrase\u00f1a que ha introducido no es correcta." },
	{ "cert.dialog.password.caption", "Contrase\u00f1a - Importar" },
	{ "cert.dialog.password.text", "<html><b>Introduzca una contrase\u00f1a para acceder a este archivo:<b></html>" },
	{ "cert.dialog.password.okButton", "Aceptar" },
	{ "cert.dialog.password.cancelButton", "Cancelar" },
	{ "cert.dialog.export.error.caption", "Error - Exportar certificado" },
	{ "cert.dialog.export.text", "<html><b>No se puede exportar</b></html>No se ha exportado el certificado." },

    { "main.control_panel_caption", "Panel de control de Java(TM) Plug-in" },

    // Each line in the property_file_header must start with "#"
    { "config.property_file_header", "# Propiedades Java(TM) Plug-in\n"
            + "# NO MODIFICAR ESTE ARCHIVO. Se gener\u00f3 autom\u00e1ticamente.\n"
            + "# Utilice el Panel de control del activador para editar las propiedades." },
	{ "config.unknownSubject", "Tema desconocido" },
	{ "config.unknownIssuer", "Emisor desconocido" },
	{ "config.certShowName", "{0}({1})" },
	{ "config.certShowOOU", "{0} {1}" },
	{ "config.proxy.autourl.invalid.text", "<html><b>URL mal construida</b></html>La URL de configuraci\u00f3n autom\u00e1tica del proxy no es v\u00e1lida." },
	{ "config.proxy.autourl.invalid.caption", "Error - Proxys" },

    { "jarcache.location", "Ubicaci\u00f3n" },
	{ "jarcache.select", "Seleccionar" },
	{ "jarcache.kb", "KB" },
	{ "jarcache.bytes", "bytes" },
	{ "jarcache.clear", "Borrar" },
        { "jarcache.clear.acceleratorKey", new Integer(KeyEvent.VK_B)},
	{ "jarcache.view", "Ver" },
	{ "jarcache.view.acceleratorKey", new Integer(KeyEvent.VK_V) },
        { "jarcache.browseDirectory.acceleratorKey", new Integer(KeyEvent.VK_O) },
	{ "jarcache.no_compression", "Ninguno" },
	{ "jarcache.select_tooltip", "Utilizar ubicaci\u00f3n seleccionada" },
	{ "jarcache.select_mnemonic", "S" },
	{ "jarcache.maximum", "M\u00e1ximo" },
	{ "jarcache.unlimited", "Ilimitado" },
	{ "jarcache.high_compression", "Alta" },
	{ "jarcache.compression", "Compresi\u00f3n de JAR" },
	{ "jarcache.mb", "Mb" },
	{ "jarcache.size", "Tama\u00f1o" },
	{ "jarcache.settings", "Valores de antememoria" },
	{ "jarcache.erase.confirm.caption", "Se necesita cofirmaci\u00f3n - Antememoria" },
	{ "jarcache.erase.confirm.text", "\u00bfDesea borrar todos los archivos de {0}?" },
	{ "jarcache.select_title", "Ubicaci\u00f3n de antememoria" },
	{ "jarcache.enabled", "Habilitar colocaci\u00f3n en antememoria" },   

	{ "update.update_button.text", "Actualizar ahora" },
	{ "update.advanced_button.text", "Avanzado..." },
	{ "update.desc.text", "<html>Gracias al mecanismo Java(TM) Update tendr\u00e1 la seguridad de que dispone de la versi\u00f3n m\u00e1s actualizada de la plataforma Java.<br>Las opciones que aparecen a continuaci\u00f3n permiten controlar el proceso de obtenci\u00f3n y aplicaci\u00f3n de las actualizaciones.</html>" },
        { "update.notify.text", "Notificar:" },
        { "update.notify_install.text", "Antes de instalar" },
        { "update.notify_download.text", "Antes de descargar y antes de instalar" },
        { "update.autoupdate.text", "Comprobar autom\u00e1ticamente si existen actualizaciones" },
        { "update.advanced_title.text", "Configuraci\u00f3n avanzada de la actualizaci\u00f3n autom\u00e1tica" },
        { "update.advanced_title1.text", "Seleccione con qu\u00e9 frecuencia y cu\u00e1ndo desea que se realice el an\u00e1lisis." },
        { "update.advanced_title2.text", "Frecuencia" },
        { "update.advanced_title3.text", "Cu\u00e1ndo" },
        { "update.advanced_desc1.text", "Realizar an\u00e1lisis todos los d\u00edas a las {0}" },
        { "update.advanced_desc2.text", "Realizar an\u00e1lisis cada {0} a las {1}" },
        { "update.advanced_desc3.text", "Realizar an\u00e1lisis el d\u00eda {0} de cada mes a las {1}" },
        { "update.check_daily.text", "Cada d\u00eda" },
        { "update.check_weekly.text", "Cada semana" },
        { "update.check_monthly.text", "Cada mes" },
        { "update.check_date.text", "D\u00eda:" },
        { "update.check_day.text", "Cada:" },
        { "update.check_time.text", "A las:" },
        { "update.lastrun.text", "Java Update se ha ejecutado por \u00faltima vez a las {0} del {1}." },
        { "update.desc_autooff.text", "<html>Debe utilizar el bot\u00f3n \"Actualizar ahora\" que aparece m\u00e1s abajo para comprobar si existen actualizaciones.<br></html>" },
        { "update.desc_check_daily.text", "<html>Todos los d\u00edas a las {0}" },
        { "update.desc_check_weekly.text", "<html>Cada {0} a las {1}" },
        { "update.desc_check_monthly.text", "<html>El d\u00eda {0} de cada mes a las {1}" },
        { "update.desc_check.text", ", Java Update comprobar\u00e1 si existen actualizaciones. " },
        { "update.desc_notify.text", "Si encuentra nuevas actualizaciones, <br> se le notificar\u00e1 antes de que la actualizaci\u00f3n" },
        { "update.desc_notify_install.text", "se instale.</html>" },
        { "update.desc_notify_download.text", "se descargue y antes de que se instale.</html>" },
	{ "update.launchbrowser.error.text", "<html><b>No se puede ejecutar Java Update Checker</b></html>Para obtener la \u00faltima actualizaci\u00f3n de Java(TM), dir\u00edjase a la p\u00e1gina http://java.sun.com/getjava/javaupdate" },
	{ "update.launchbrowser.error.caption", "Error - Actualizaci\u00f3n" },
    };
}

