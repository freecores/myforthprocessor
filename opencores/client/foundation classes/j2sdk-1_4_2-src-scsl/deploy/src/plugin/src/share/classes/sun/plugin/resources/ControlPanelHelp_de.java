/*
 * @(#)ControlPanelHelp_de.java	1.2 03/03/20
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * US English verison of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ControlPanelHelp_de extends ListResourceBundle {

    private static String newline = "\n";

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "help.file", newline +
                   "Java Plug-in Bedienungsfeld-Hilfe" + newline + newline + newline +
                   "Folgende Themen werden in dieser Hilfe behandelt:" + newline +
                   "      \u00dcberblick" + newline +
                   "      Speichern von Optionen" + newline +
                   "      Einstellen der Bedienungsfeld-Optionen" + newline +
                   "      Bedienungsfeld \"Standard\"" + newline +
                   "      Bedienungsfeld \"Erweitert\"" + newline +
                   "      Bedienungsfeld \"Browser\"" + newline +
                   "      Bedienungsfeld \"Proxies\"" + newline +
                   "      Bedienungsfeld \"Cache\"" + newline +
                   "      Bedienungsfeld \"Zertifikate\"" + newline +
                   "      Bedienungsfeld \"Update\"" + newline + newline +
                   "\u00dcberblick" + newline + newline +
                   "Mit dem Java Plug-in Bedienungsfeld k\u00f6nnen Sie die Standardeinstellungen \u00e4ndern, die von Java Plug-in beim Start verwendet werden." + newline +
                   "Alle Applets, die in einer aktiven Instanz von Java Plug-in laufen, verwenden diese Einstellungen." + newline +
                   "Das in diesem Dokument erw\u00e4hnte Java Plug-in-Entwicklerhandbuch finden Sie unter (\u00c4nderung der URL vorbehalten): " + newline +
                   "http://java.sun.com/j2se/1.4/docs/guide/plugin/developer_guide/contents.html" + newline + newline +
                   "Speichern von Optionen" + newline + newline +
                   "Wenn Sie alle \u00c4nderungen an den Bedienungsfeld-Optionen vorgenommen haben, klicken Sie auf \"\u00dcbernehmen\", um die \u00c4nderungen zu speichern." + newline +
                   "Klicken Sie auf \"Zur\u00fccksetzen\", wenn Sie die \u00c4nderungen verwerfen und die zuletzt eingegebenen und \u00fcbernommenen Werte erneut laden m\u00f6chten." + newline +
                   "Beachten Sie, dass es sich hierbei nicht um dieselben Standardwerte handelt, die urspr\u00fcnglich beim Installieren von Java Plug-in eingestellt waren." + newline + newline +
                   "Einstellen der Bedienungsfeld-Optionen" + newline + newline +
                   "Sie k\u00f6nnen die verschiedenen Optionen auf sechs Feldern im Java Plug-in Bedienungsfeld festlegen." + newline +
                   "Diese Felder werden folgenderma\u00dfen bezeichnet: " + newline +
                   "      Standard" + newline +
                   "      Erweitert" + newline +
                   "      Browser" + newline +
                   "      Proxies" + newline +
                   "      Cache" + newline +
                   "      Zertifikate" + newline +
                   "      Update" + newline + newline +
                   "Jedes Bedienungsfeld wird im Folgenden beschrieben." + newline + newline + newline +
                   " Standard" + newline +
                   "Java-Konsole einblenden" + newline + newline +
                   "      Zeigt w\u00e4hrend der Ausf\u00fchrung von Applets die Java-Konsole an. Auf der Konsole werden die von \"System.out\" und \"System.err\" ausgegebenen Mitteilungen angezeigt." + newline +
                   "      Dies ist bei der Fehlersuche hilfreich." + newline + newline +
                   "Konsole ausblenden" + newline + newline +
                   "      Die Java-Konsole wird ausgef\u00fchrt, ist aber ausgeblendet. Dieses ist die Standardeinstellung (aktiviert)." + newline + newline +
                   "Konsole nicht starten" + newline + newline +
                   "      Die Java-Konsole wird nicht gestartet." + newline + newline +
                   "Ausnahmedialogfeld anzeigen" + newline + newline +
                   "      Beim Auftreten einer Ausnahme wird ein Ausnahmedialogfeld angezeigt. In der Standardeinstellung wird das Ausnahmedialogfeld nicht angezeigt (deaktiviert)." + newline + newline +
                   "Java im System Tray anzeigen (nur Windows)" + newline + newline +
                   "      Wenn diese Option aktiviert ist, wird das Symbol mit der Java-Kaffeetasse beim Start von Java Plug-in im System Tray eingeblendet." + newline +
                   "      Beim Beenden von Java Plug-in wird das Symbol aus dem System Tray entfernt." + newline +
                   "      Das Symbol mit der Java-Kaffeetasse zeigt dem Benutzer an, dass eine Java-VM ausgef\u00fchrt wird. Au\u00dferdem erm\u00f6glicht es den Zugriff auf Informationen \u00fcber die" + newline +
                   "      Java-Version und auf die Java-Konsole." + newline +
                   "      In der Standardeinstellung ist diese Option aktiviert." + newline + newline +
                   "      Funktionen des Java-Symbols im System Tray:" + newline + newline +
                   "      Wenn Sie mit dem Mauszeiger auf das Symbol der Java-Kaffeetasse zeigen, wird das Wort \"Java\" angezeigt." + newline + newline +

                   "      Wenn Sie mit der linken Maustaste auf das Java-Symbol im System Tray doppelklicken, wird die Java-Konsole eingeblendet." + newline + newline +
                   "      Wenn Sie mit der rechten Maustaste auf das Java-Symbol im System Tray klicken, wird ein Popupmen\u00fc mit den folgenden Befehlen angezeigt:" + newline + newline +
                   "            Konsole \u00f6ffnen/schlie\u00dfen" + newline +
                   "            Info \u00fcber Java" + newline +
                   "            Deaktivieren" + newline +
                   "            Beenden" + newline + newline +
                   "      Mit \"Konsole \u00f6ffnen\" bzw. \"Konsole schlie\u00dfen\" wird das Fenster der Java-Konsole ge\u00f6ffnet bzw. geschlossen. Wenn die Java-Konsole ausgeblendet ist, wird der Befehl \"Konsole \u00f6ffnen\" angezeigt," + newline +
                   "      wenn sie eingeblendet ist, der Befehl \"Konsole schlie\u00dfen\"." + newline + newline +
                   "      Mit dem Befehl \"Info \u00fcber Java\" \u00f6ffnen Sie das Dialogfeld \"Info - Java\" f\u00fcr Java 2 Standard Edition." + newline + newline +
                   "      Mit dem Befehl \"Deaktivieren\" wird das Java-Symbol f\u00fcr die aktuelle und zuk\u00fcnftige Sitzungen aus dem System Tray entfernt. Bei einem erneutem Start von Java Plug-in" + newline +
                   "      wird das Java-Symbol nicht im System Tray angezeigt." + newline +
                   "      Im folgenden Hinweis wird erl\u00e4utert, wie Sie festlegen, dass das Java-Symbol wieder im System Tray angezeigt wird, nachdem Sie es deaktiviert haben." + newline + newline +
                   "      Mit dem Befehl \"Beenden\" wird das Java-Symbol nur f\u00fcr die aktuelle Sitzung aus dem System Tray entfernt. Bei einem erneutem Start von Java Plug-in wird das" + newline +
                   "      Java-Symbol erneut im System Tray angezeigt." + newline + newline + newline +
                   "                Hinweise" + newline + newline +
                   "                1. Wenn \"Java im System Tray anzeigen\" aktiviert ist, wird das Java-Symbol im System Tray angezeigt, auch wenn \"Konsole nicht" + newline +
                   "                starten\" aktiviert ist." + newline + newline +
                   "                2. Um das Java-Symbol zu aktivieren, nachdem Sie es deaktiviert haben, starten Sie das Java Plug-in Bedienungsfeld, aktivieren Sie \"Java im" + newline +
                   "                System Tray anzeigen\", und klicken Sie auf \"\u00dcbernehmen\"." + newline + newline +
                   "                3. Wenn bereits andere Java-VMs ausgef\u00fchrt werden und dem System Tray andere Java-Symbole hinzugef\u00fcgt wurden, hat" + newline +
                   "                eine \u00c4nderung der Einstellung im Java Plug-in Bedienungsfeld keine Auswirkungen auf diese Symbole." + newline +
                   "                Die Einstellung wirkt sich nur auf das Verhalten des Java-Symbols aus, wenn die Java-VM nach dem Einstellen gestartet wird." + newline + newline + newline +
                   " Erweitert" + newline + newline + newline +
                   "Java Run Time Environment" + newline + newline +
                   "      Erm\u00f6glicht die Ausf\u00fchrung von Java Plug-in mit jeder Java 2 JRE bzw. jedem SDK, Standard Edition V. 1.3 oder 1.4, die bzw. das auf Ihrem Rechner installiert ist." + newline +
                   "      Java Plug-in 1.3/1.4 wird mit einer Standard-JRE ausgeliefert." + newline +
                   "      Sie k\u00f6nnen jedoch die Standard-JRE \u00fcbergehen und eine \u00e4ltere oder neuere Version verwenden. Das Bedienungsfeld erkennt automatisch" + newline +
                   "      alle auf dem Rechner installierten Java 2 SDK- oder JRE-Versionen. Im Listenfeld werden alle Versionen aufgef\u00fchrt," + newline +
                   "      die Sie verwenden k\u00f6nnen." + newline +
                   "      Das erste Element in der Liste ist immer die Standard-JRE, das letzte Element immer \"Sonstige\". Wenn Sie \"Sonstige\" w\u00e4hlen, m\u00fcssen Sie" + newline +
                   "      den Pfad zur Java 2 JRE bzw. zum SDK, Standard Edition V. 1.3/1.4 angeben." + newline + newline +
                   "                Hinweis" + newline + newline +
                   "                Nur erfahrene Benutzer sollten diese Option \u00e4ndern. Vom \u00c4ndern der Standard-JRE wird abgeraten." + newline + newline + newline +
                   "Java Laufzeitparameter" + newline + newline +
                   "      \u00dcberschreibt die Standard-Startparameter von Java Plug-in durch die Angabe benutzerdefinierter Optionen. Es wird dieselbe Syntax verwendet" + newline +
                   "      wie bei der Verwendung von Parametern beim Aufruf von Java \u00fcber die Befehlszeile. Eine vollst\u00e4ndige Liste der Befehlszeilenoptionen finden" + newline +
                   "      Sie in der Dokumentation der Java 2 Standard Edition (J2SE)." + newline +
                   "      Die \u00c4nderung der unten aufgef\u00fchrten URL ist vorbehalten:" + newline + newline +
                   "            http://java.sun.com/j2se/1.4/docs/tooldocs/<plattform>/java.html" + newline + newline +
                   "            wobei \"<plattform>\" f\u00fcr ein Betriebssystem steht: Solaris, Linux, Win32." + newline + newline +
                   "      Nachstehend sind einige Beispiele f\u00fcr Java-Laufzeitparameter aufgef\u00fchrt." + newline + newline +
                   "      Aktivieren und Deaktivieren der Unterst\u00fctzung von Annahmen" + newline + newline +
                   "            Zur Unterst\u00fctzung von Annahmen muss die folgende Systemeigenschaft in den Java-Laufzeitparametern festgelegt werden:" + newline + newline +
                   "                  -D[ enableassertions | ea ][:<package name>\"...\" | : <class name> ]" + newline + newline +
                   "            Zur Deaktivierung von Annahmen in Java Plug-in geben Sie Folgendes bei den Java-Laufzeitparametern an:" + newline + newline +
                   "                  -D[ disableassertions | da ][:<package name>\"...\" | : <class name> ]" + newline + newline +
                   "            Weitere Informationen \u00fcber das Aktivieren/Deaktivieren von Annahmen finden Sie unter \"Assertion Facility\"." + newline +
                   "            http://java.sun.com/j2se/1.4/docs/guide/lang/assert.html (\u00c4nderung der URL vorbehalten)." + newline + newline +
                   "            Standardm\u00e4\u00dfig sind Annahmen im Java Plug-in-Code deaktiviert. Da die Behandlung von Annahmen beim Start von Java Plug-in ermittelt wird," + newline +
                   "            muss der Browser nach dem \u00c4ndern der Einstellungen f\u00fcr Annahmen im Java Plug-in Bedienungsfeld neu gestartet werden," + newline +
                   "            damit die neuen Einstellungen wirksam werden." + newline + newline +
                   "            Weil im Java-Code in Java Plug-in ebenfalls Annahmen integriert sind, k\u00f6nnen Annahmen im Java Plug-in-Code" + newline +
                   "            folgenderma\u00dfen aktiviert werden:" + newline + newline +
                   "                  -D[ enableassertions | ea ]:sun.plugin" + newline + newline +
                   "      Unterst\u00fctzung f\u00fcr Ablaufverfolgung und Protokollierung" + newline + newline +
                   "            Die Ablaufverfolgung ist eine Funktion, mit der jede Ausgabe in der Java-Konsole zu einer Trace-Datei (\".plugin<version>.trace\") umgeleitet wird." + newline + newline +
                   "                  -Djavaplugin.trace=true" + newline +
                   "                  -Djavaplugin.trace.option=basic|net|security|ext|liveconnect" + newline + newline +
                   "            Wenn Sie den Standardnamen f\u00fcr die Trace-Datei nicht verwenden m\u00f6chten:" + newline + newline +
                   "                  -Djavaplugin.trace.filename=<tracefilename>" + newline + newline +
                   "            \u00c4hnlich wie die Ablaufverfolgung ist auch das Protokollieren eine M\u00f6glichkeit, jede Ausgabe in der Java-Konsole mithilfe des Java Logging API in eine Protokolldatei (\".plugin<version>.log\")" + newline +
                   "            umzuleiten." + newline +
                   "            Zum Einschalten der Protokollierung aktivieren Sie die Eigenschaft \"javaplugin.logging\"." + newline + newline +
                   "                  -Djavaplugin.logging=true" + newline + newline +
                   "            Wenn Sie den Standardnamen f\u00fcr die Protokolldatei nicht verwenden m\u00f6chten:" + newline + newline +
                   "                  -Djavaplugin.log.filename=<logfilename>" + newline + newline +
                   "            Au\u00dferdem k\u00f6nnen Sie folgende Eigenschaften festlegen, wenn die Trace- und Protokolldateien nicht bei jeder Sitzung \u00fcberschrieben werden sollen:" + newline + newline +
                   "                  -Djavaplugin.outputfiles.overwrite=false." + newline + newline +
                   "            Wenn diese Eigenschaft auf \"false\" gesetzt ist, erhalten die Trace- und Protokolldateien bei jeder Sitzung einen eindeutigen Namen. Bei Verwendung der" + newline +
                   "            Standardnamen f\u00fcr die Trace- und Protokolldateien werden diese Dateien folgenderma\u00dfen benannt:" + newline + newline +
                   "                  .plugin<username><date hash code>.trace" + newline +
                   "                  .plugin<username><date hash code>.log" + newline + newline +
                   "            Die \u00fcber das Bedienungsfeld eingestellten Funktionen Ablaufverfolgung und Protokollierung werden beim Starten des Plug-in aktiviert," + newline +
                   "            aber die \u00c4nderungen, die bei laufendem Plug-in \u00fcber das Bedienungsfeld vorgenommenen werden, sind erst nach einem Neustart wirksam." + newline + newline +
                   "            Weitere Informationen \u00fcber Ablaufverfolgung und Protokollierung finden Sie im entsprechenden Abschnitt im Java Plug-in-Entwicklerhandbuch." + newline + newline +
                   "      Fehlersuche in Applets in Java Plug-in" + newline + newline +
                   "            Die folgenden Optionen werden bei der Fehlersuche in Applets in Java Plug-in verwendet." + newline +
                   "            Weitere Informationen zu diesem Thema finden Sie im Java Plug-in-Entwicklerhandbuch im Abschnitt zur Unterst\u00fctzung der Fehlersuche." + newline + newline +
                   "                  -Djava.compiler=NONE" + newline +
                   "                  -Xnoagent" + newline +
                   "                  -Xdebug" + newline +
                   "                  -Xrunjdwp:transport=dt_shmem,address=<connect-address>,server=y,suspend=n" + newline + newline +
                   "            \"<connect-address>\" kann eine beliebige Zeichenkette sein (Beispiel: 2502), die sp\u00e4ter vom Java Debugger (jdb) verwendet wird," + newline +
                   "            um eine Verbindung zur JVM zu erstellen." + newline + newline +
                   "      Standard-Timeout f\u00fcr Verbindung" + newline + newline +
                   "            Wenn von einem Applet eine Verbindung zu einem Server hergestellt wird und der Server nicht wie erwartet reagiert, kann es den Anschein haben," + newline +
                   "            dass das Applet h\u00e4ngt. Das kann auch dazu f\u00fchren, dass der Browser h\u00e4ngt, weil kein Timeout der Netzwerkverbindung vorliegt" + newline +
                   "            (standardm\u00e4\u00dfig auf -1 gesetzt, d.h., dass kein Timeout festgelegt ist)." + newline + newline +
                   "            Zur Vermeidung dieses Problems hat das Java Plug-in einen Standard-Netzwerk-Timeout (2 Minuten) f\u00fcr alle HTTP-Verbindungen hinzugef\u00fcgt." + newline +
                   "            Sie k\u00f6nnen diese Einstellung in den Java Laufzeitparametern \u00e4ndern:" + newline + newline +
                   "                  -Dsun.net.client.defaultConnectTimeout=Wert in Millisekunden" + newline + newline +
                   "            Eine weitere einstellbare Netzwerkeigenschaft ist \"sun.net.client.defaultReadTimeout\"." + newline + newline +
                   "                  -Dsun.net.client.defaultReadTimeout=Wert in Millisekunden" + newline + newline +
                   "                  Hinweis" + newline + newline +
                   "                  Standardm\u00e4\u00dfig wird der Wert \"sun.net.client.defaultReadTimeout\" von Java Plug-in nicht gesetzt. Wenn Sie ihn festlegen m\u00f6chten," + newline +
                   "                  tun Sie dies wie oben angegeben \u00fcber die Java Laufzeitparameter." + newline + newline + newline +
                   "            Beschreibung der Netzwerkeigenschaften:" + newline + newline +
                   "            sun.net.client.defaultConnectTimeout" + newline +
                   "            sun.net.client.defaultReadTimeout" + newline + newline +
                   "                  Mit diesen Eigenschaften werden die Standardwerte f\u00fcr Verbindungs- und Lese-Timeouts f\u00fcr die von \"java.net.URLConnection\" verwendeten" + newline +
                   "                  Protokoll-Handler festgelegt. Die von den Protokoll-Handlern festgelegten Standardwerte sind jeweils \"-1\"," + newline +
                   "                  d.h., dass kein Timeout festgelegt ist." + newline + newline +
                   "                  \"sun.net.client.defaultConnectTimeout\" legt den Timeout (in Millisekunden) f\u00fcr den Aufbau der Verbindung zum Host fest." + newline +
                   "                  Bei HTTP-Verbindungen ist das zum Beispiel der Timeout f\u00fcr den Aufbau der Verbindung zum HTTP-Server." + newline +
                   "                  Bei FTP-Verbindungen ist das der Timeout f\u00fcr den Aufbau der Verbindung zu FTP-Servern." + newline + newline +
                   "                  \"sun.net.client.defaultReadTimeout\" legt den Timeout (in Millisekunden) f\u00fcr das Lesen eines Eingabestroms" + newline +
                   "                  bei einer bestehenden Verbindung zu einer Ressource fest." + newline + newline +
                   "            Die offizielle Beschreibung dieser Netzwerkeigenschaften finden Sie unter" + newline +
                   "            http://java.sun.com/j2se/1.4/docs/guide/net/properties.html." + newline + newline + newline +
                   " Browser" + newline + newline + newline +
                   "Dieses Bedienungsfeld bezieht sich nur auf Microsoft Windows-Installationen; bei anderen Installationen wird es nicht angezeigt. Aktivieren Sie jeden Browser," + newline +
"f\u00fcr den Java Plug-in anstelle der internen JVM des jeweiligen Browsers das Standard-Java-Laufzeitprogramm sein soll." + newline +
"Damit wird die Unterst\u00fctzung des Tags \"APPLET\" in Internet Explorer und Netscape 6 via Java Plug-in aktiviert." + newline + newline + newline +
                   " Proxies" + newline + newline +newline +
                   "\u00dcber das Bedienungsfeld \"Proxies\" wird festgelegt, ob die Standardeinstellungen des Browsers verwendet oder die Werte f\u00fcr Proxy-Adresse und Anschluss f\u00fcr die verschiedenen Protokolle \u00fcberschrieben werden sollen." + newline + newline +
                   "Browser-Einstellungen verwenden" + newline + newline +
                   "      Aktivieren Sie diese Option, um die Standard-Proxy-Einstellungen des Browsers zu verwenden. Dieses ist die Standardeinstellung (aktiviert)." + newline + newline +
                   "Proxy-Einstellungen" + newline + newline +
                   "      Sie k\u00f6nnen die Standardeinstellungen \u00fcberschreiben, indem Sie das Kontrollk\u00e4stchen \"Browser-Einstellungen verwenden\" deaktivieren und dann die Tabelle mit den Proxy-Einstellungen" + newline +
                   "      unter dem Kontrollk\u00e4stchen ausf\u00fcllen. Sie k\u00f6nnen die Proxy-Adresse und den Anschluss f\u00fcr jedes der unterst\u00fctzten Protokolle" + newline +
                   "      eingeben: HTTP, Secure (HTTPS), FTP, Gopher und Socks." + newline + newline +
                   "Kein Proxy-Host" + newline + newline +
                   "      Dies ist ein Host oder eine Liste von Hosts, f\u00fcr den/die keine Proxies verwendet werden sollen. F\u00fcr einen internen Host in einer Intranet-Umgebung" + newline +
                   "      wird normalerweise kein Proxy-Host verwendet." + newline + newline +
                   "URL der automatischen Proxy-Konfiguration" + newline +
                   "      Das ist die URL f\u00fcr die JavaScript-Datei (Erweiterung \".js\" oder \".pac\") mit der Funktion \"FindProxyForURL\"." + newline +
                   "      \"FindProxyForURL\" enth\u00e4lt die Logik zum Ermitteln des Proxy-Servers, der f\u00fcr eine Verbindungsanforderung zu verwenden ist." + newline + newline +
                   "Weitere Angaben zur Proxy-Konfiguration finden Sie im Kapitel zur Proxy-Konfiguration im Java Plug-in-" + newline +
                   "Entwicklerhandbuch." + newline + newline + newline +
                   " Cache" + newline + newline + newline +
                   "           Hinweis" + newline + newline +
                   "           Bei dem hier erw\u00e4hnten Cache handelt es sich um den \"Sticky\"-Cache, d.h. den von Java Plug-in erstellten und verwalteten Platten-Cache," + newline +
                   "           den der Browser nicht \u00fcberschreiben kann. Weitere Informationen dazu finden Sie unter \"Applet-Caching\" im Java Plug-in-Entwicklerhandbuch." + newline + newline + newline +
                   "Cache aktivieren" + newline + newline +
                   "      Aktivieren Sie diese Option, um das Caching zu aktivieren. Dieses ist die Standardeinstellung (aktiviert). Bei aktiviertem Applet-Caching ist die Leistung" + newline +
                   "      h\u00f6her, weil ein Applet, das bereits im Cache gespeichert wurde, nicht noch einmal heruntergeladen werden muss, wenn darauf wieder Bezug benommen wird." + newline + newline +
                   "      Java Plug-in speichert Dateien folgender Typen, die \u00fcber HTTP/HTTPS heruntergeladen wurden, im Cache:" + newline + newline +
                   "            .jar (JAR-Datei)" + newline +
                   "            .zip (ZIP-Datei)" + newline +
                   "            .class (Java-Class-Datei)" + newline +
                   "            .au (Audiodatei)" + newline +
                   "            .wav (Audiodatei)" + newline +
                   "            .jpg (Bilddatei)" + newline +
                   "            .gif (Bilddatei)" + newline + newline +
                   "Cache anzeigen" + newline + newline +
                   "      Bet\u00e4tigen Sie diese Schaltfl\u00e4che, um die Dateien im Cache anzuzeigen. Ein weiteres Dialogfeld (Java Plug-in-Cache-Viewer) wird angezeigt, und darin sind die Dateien im Cache aufgef\u00fchrt." + newline +
                   "      Der Cache-Viewer zeigt folgende Informationen \u00fcber die Dateien im Cache an: Name, Typ, Gr\u00f6\u00dfe, Verfallsdatum," + newline +
                   "      Datum der letzten \u00c4nderung, Version und URL. Im Cache-Viewer k\u00f6nnen Sie auch einzelne Dateien aus dem Cache l\u00f6schen." + newline +
                   "      Dies ist eine Alternative zu der unten beschriebenen Option \"Cache l\u00f6schen\", mit der alle Dateien im Cache gel\u00f6scht werden." + newline + newline +
                   "Cache l\u00f6schen" + newline + newline +
                   "      Bet\u00e4tigen Sie diese Schaltfl\u00e4che, um alle Dateien im Cache zu l\u00f6schen. Daraufhin werden Sie aufgefordert, diese Aktion zu best\u00e4tigen (\"Alle Dateien in ... _cache l\u00f6schen?\"), bevor die Dateien gel\u00f6scht werden." + newline + newline +
                   "Speicherort" + newline + newline +
                   "      Sie k\u00f6nnen mit dieser Option den Speicherort des Cache festlegen. Der Standardspeicherort des Cache ist \"<user home>/.jpi_cache\", wobei" + newline +
                   "      \"<user home>\" den Wert der Systemeigenschaft \"user.home\" hat. Der Wert ist betriebssystemabh\u00e4ngig." + newline + newline +
                   "Gr\u00f6\u00dfe" + newline + newline +
                   "      Wenn Sie die Option \"Unbegrenzt\" aktivieren, ist die Gr\u00f6\u00dfe des Cache nicht beschr\u00e4nkt. Alternativ k\u00f6nnen Sie die Maximalgr\u00f6\u00dfe des Cache festlegen." + newline +
                   "      Wenn die Gr\u00f6\u00dfe des Cache die festgelegte Grenze \u00fcberschreitet, wird solange die jeweils \u00e4lteste Datei aus dem Cache gel\u00f6scht, bis die Cache-Gr\u00f6\u00dfe" + newline +
                   "      wieder unterhalb des Grenzwerts liegt." + newline + newline +
                   "JAR-Kompression" + newline + newline +
                   "      Sie haben bei der JAR-Kompression die Wahl zwischen \"Keine\" und \"Hoch\". Bei einer h\u00f6heren Komprimierung" + newline +
                   "      sparen Sie zwar Speicherplatz ein, m\u00fcssen daf\u00fcr aber Leistungseinbu\u00dfen hinnehmen; die beste Leistung erzielen Sie" + newline +
                   "      ohne Komprimierung." + newline + newline + newline +
                   " Zertifikate" + newline + newline + newline +
                   "Vier Zertifikatstypen stehen zur Auswahl:" + newline + newline +
                   "      Signiertes Applet" + newline +
                   "      Sichere Site" + newline +
                   "      Zertifizierungsstellen-Signierer" + newline +
                   "      Zertifizierungsstelle der sicheren Site" + newline + newline +
                   "Signiertes Applet" + newline + newline +
                   "      Hierbei handelt es sich um Zertifikate f\u00fcr signierte Applets, denen der Benutzer vertraut. Die Zertifikate, die in der Liste der signierten Applets" + newline +
                   "      angezeigt werden, werden der Zertifikatsdatei \"jpicerts<version>\" aus dem Verzeichnis \"<user home>/.java\" entnommen." + newline + newline +
                   "Sichere Site" + newline + newline +
                   "      Hierbei handelt es sich um Zertifikate f\u00fcr sichere Sites. Die Zertifikate, die in der Liste der sicheren Sites angezeigt werden, werden der Zertifikatsdatei" + newline +
                   "      \"jpihttpscerts<version>\" aus dem Verzeichnis \"<user home>/.java\" entnommen." + newline + newline +
                   "Zertifizierungsstellen-Signierer" + newline + newline +
                   "      Hierbei handelt es sich um Zertifikate von Zertifizierungsstellen f\u00fcr signierte Applets; diese Stellen geben die Zertifikate" + newline +
                   "       f\u00fcr die Signierer von signierten Applets heraus. Die Zertifikate, die in der Liste der Zertifizierungsstellen-Signierer angezeigt werden, werden der Zertifikatsdatei" + newline +
                   "       \"cacerts\" aus dem Verzeichnis \"<jre>/lib/security\" entnommen." + newline + newline +
                   "Zertifizierungsstelle der sicheren Site" + newline + newline +
                   "      Hierbei handelt es sich um Zertifikate von Zertifizierungsstellen f\u00fcr sichere Sites; diese Stellen geben die Zertifikate" + newline +
                   "      f\u00fcr sichere Sites heraus. Die Zertifikate, die in der Liste der Zertifizierungsstellen f\u00fcr sichere Sites angezeigt werden, werden der Zertifikatsdatei \"jssecacerts\" aus dem Verzeichnis" + newline +
                   "      \"<jre>/lib/security\" entnommen." + newline + newline +
                   "F\u00fcr die Zertifikate \"Signiertes Applet\" und \"Sichere Site\" stehen vier Optionen zur Verf\u00fcgung: \"Importieren\", \"Exportieren\", \"Entfernen\" und \"Details\"." + newline +
                   "Der Benutzer kann ein Zertifikat importieren, exportieren, l\u00f6schen oder dessen Details anzeigen." + newline + newline +
                   "F\u00fcr \"Zertifizierungstellen-Signierer\" und \"Zertifizierungsstelle der sicheren Site\" steht nur eine Option zur Verf\u00fcgung: \"Details\". Der Benutzer kann nur die Details eines Zertifikats anzeigen." + newline + newline + newline +
                   " Update" + newline + newline + newline +
                   "In diesem Feld gibt es eine Schaltfl\u00e4che \"Java aktualisieren\", \u00fcber die der Benutzer das neueste Update f\u00fcr die Java Runtime" + newline +
"Environment von der Java Update-Website erhalten kann. Dieses Bedienungsfeld bezieht sich nur auf Microsoft Windows-Installationen;" + newline +
"bei anderen Plattformen (d.h. Solaris/Linux) wird es nicht angezeigt."}
};
}



