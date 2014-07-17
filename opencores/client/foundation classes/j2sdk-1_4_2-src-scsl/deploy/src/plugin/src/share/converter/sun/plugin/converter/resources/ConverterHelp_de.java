/*
 * @(#)ConverterHelp_de.java	1.2 03/04/24
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * German version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_de extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");;

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Infodatei f\u00fcr Java(tm) Plug-in HTML Converter" + newline + newline +
      "Version: 1.4.2" + newline + newline + newline +
      "*****   SICHERN SIE ALLE DATEIEN, BEFOR SIE SIE MIT DIESEM PROGRAMM KONVERTIEREN." + newline +
      "*****   BEI EINEM ABBRUCH DER KONVERTIERUNG WERDEN DIE \u00c4NDERUNGEN NICHT R\u00dcCKG\u00c4NGIG GEMACHT." + newline +
      "*****   KOMMENTARE IM TAG \"APPLET\" WERDEN IGNORIERT." + newline + newline + newline +
      "Inhalt" + newline +
      "   1. Neue Funktionen" + newline +
      "   2. Behobene Fehler" + newline +
      "   3. Info \u00fcber Java(tm) Plug-in HTML Converter" + newline +
      "   4. Der Konvertierungsvorgang" + newline +
      "   5. Ausw\u00e4hlen der zu konvertierenden Dateien in Ordnern" + newline +
      "   6. Ausw\u00e4hlen eines Ordners f\u00fcr Sicherheitskopien" + newline +
      "   7. Erzeugen einer Protokolldatei" + newline +
      "   8. Ausw\u00e4hlen einer Konvertierungsvorlage" + newline +
      "   9. Konvertieren" + newline +
      "  10. Konvertieren von weiteren Dateien oder Beenden des Programms" + newline +
      "  11. Einzelheiten \u00fcber Vorlagen" + newline +
      "  12. Ausf\u00fchren von HTML Converter (Windows und Solaris)" + newline + newline +
      "1)  Neue Funktionen" + newline + newline +
      "    o Die erweiterten Vorlagen wurden aktualisiert, um Netscape 6 zu unterst\u00fctzen." + newline +
      "    o Alle Vorlagen wurden aktualisiert, um die neuen Funktionen f\u00fcr die Verwendung verschiedener Versionen von Java Plug-in zu unterst\u00fctzen." + newline +
      "    o Die Benutzeroberfl\u00e4che wurde durch den Einsatz von Swing 1.1 verbessert, um die Internationalisierung zu unterst\u00fctzen." + newline +
      "    o Das Dialogfeld \"Advanced Option\" wurde verbessert, um die neuen Vorlagen-Tags \"SmartUpdate\" und" + newline +
      "      \"MimeType\" zu unterst\u00fctzen." + newline +
      "    o HTML Converter wurde weiterentwickelt, um Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x , Java Plug-in 1.3.x und Java Plug-in" + newline +
      "      1.4.x zu unterst\u00fctzen." + newline +
      "    o In allen Konvertierungsvorlagen wurde die Unterst\u00fctzung von" + newline +
      "      \"SmartUpdate\" und \"MimeType\" verbessert." + newline +
      "    o Dem Tag \"object/embed\" wurde in allen Vorlagen \"scriptable=false\" hinzugef\u00fcgt." + newline + newline +
      "     Hiermit wird verhindert, dass eine Typenbibliothek erzeugt wird, wenn Java" + newline + newline +
      "    Plug-in nicht f\u00fcr Skripte verwendet wird." + newline + newline + newline +
      "2)  Behobene Fehler" + newline + newline +
      "    o Fehler auf Grund von fehlenden Eigenschaftendateien werden jetzt besser verarbeitet." + newline +
      "    o Die HTML-Konvertierung wurde verbessert, so dass das resultierende Tag \"embed/object\"" + newline +
      "      im AppletViewer von JDK 1.2.x verwendet werden kann." + newline +
      "    o Es wurden unn\u00f6tige Dateien entfernt, die noch von HTML Converter 1.1.x vorhanden waren." + newline +
      "    o \"embed/object\" wird jetzt mit den Attributnamen \"code\", \"codebase\" usw." + newline +
      "      statt mit \"java_code\", \"java_codebase\" usw. erzeugt. Dadurch kann" + newline +
      "      die erzeugte Seite im AppletViewer von JDK 1.2.x verwendet werden." + newline +
      "    o Die Konvertierung von \"mayscript\" wird unterst\u00fctzt, wenn dieses Tag im" + newline +
      "      Tag \"APPLET\" vorhanden ist." + newline + newline +
      "3)  Info \u00fcber Java(tm) Plug-in HTML Converter" + newline + newline +
      "        Java(tm) Plug-in HTML Converter ist ein Hilfsprogramm, mit dem HTML-Seiten" + newline +
      "        mit eingebetteten Applets in ein Format konvertiert werden k\u00f6nnen, bei dem Java(tm)" + newline +
      "        Plug-in verwendet wird." + newline + newline +
      "4)  Der Konvertierungsvorgang" + newline + newline +
      "        Java(tm) Plug-in HTML Converter konvertiert Dateien, die Applets" + newline +
      "        enthalten, in ein Format, das mit Java(tm) Plug-in verwendet werden kann." + newline + newline +
      "        Dieser Vorgang l\u00e4uft folgenderma\u00dfen ab:" + newline +
      "        Zun\u00e4chst wird der HTML-Code, der nicht Teil eines Applets ist, aus der Quelldatei" + newline +
      "        in eine tempor\u00e4re Datei \u00fcbertragen. Wenn das Tag \"<applet>\" erkannt wird, analysiert HTML Converter" + newline +
      "        das Applet bis zum ersten Tag \"</applet>\" (das nicht in Anf\u00fchrungszeichen gesetzt ist)," + newline +
      "        und f\u00fchrt die Applet-Daten mit der Vorlage zusammen. (Einzelheiten \u00fcber Vorlagen" + newline +
      "        finden Sie weiter unten.) Wenn dieser Vorgang ohne Fehler durchgef\u00fchrt werden kann, wird die urspr\u00fcngliche HTML-Datei in den" + newline +
      "        Ordner f\u00fcr Sicherheitskopien verschoben, und die tempor\u00e4re Datei erh\u00e4lt" + newline +
      "        den Namen der urspr\u00fcnglichen Datei. Die urspr\u00fcnglichen Dateien werden also nicht von der Festplatte gel\u00f6scht." + newline + newline +
      "        Beachten Sie, dass die Dateien am urspr\u00fcnglichen Speicherort konvertiert werden. Nach" + newline +
      "        der Konvertierung liegen die Dateien daher in einem Format vor, bei dem Java(tm) Plug-in verwendet wird." + newline +


      "5)  Ausw\u00e4hlen der zu konvertierenden Dateien in Ordnern" + newline + newline +
      "       Wenn Sie alle Dateien in einem Ordner konvertieren m\u00f6chten, k\u00f6nnen Sie den Pfad des Ordners eingeben" + newline +
      "       oder auf die Schaltfl\u00e4che \"Browse\" klicken, um einen Ordner mit Hilfe eines Dialogfelds auszuw\u00e4hlen." + newline  +
      "       Nachdem Sie einen Ordner ausgew\u00e4hlt haben, k\u00f6nnen Sie im Feld \"Matching File Names\"" + newline +
      "       eine beliebige Anzahl an Dateibezeichnern eintragen. Die einzelnen Bezeichner m\u00fcssen durch Kommas voneinander getrennt werden. Sie k\u00f6nnen das Zeichen \"*\" als" + newline +
      "       Platzhalter verwenden. Wenn Sie einen Dateinamen mit einem Platzhalter eingeben, werden nur Dateien konvertiert, deren Namen diesem" + newline +
      "       Muster entsprechen. Aktivieren Sie das Kontrollk\u00e4stchen \"Include Subfolders\", wenn" + newline +
      "       alle Dateien in den Unterordnern konvertiert werden sollen, deren Namen dem Muster entsprechen." + newline + newline +
      "6)  Ausw\u00e4hlen eines Ordners f\u00fcr Sicherheitskopien" + newline +

      "       Der Standardpfad des Ordners f\u00fcr die Sicherheitskopien entspricht dem Quellpfad, an dessen Name \"_BAK\"" + newline +
      "       angeh\u00e4ngt wird. Wenn der Quellpfad also \"c:/html/applet.html\" (Konvertierung einer Datei) lautet," + newline +
      "       ist der Pfad f\u00fcr die Sicherheitskopien \"c:/html_BAK\". Wenn der Quellpfad" + newline +
      "       \"c:/html\" (Konvertierung aller Dateien im Pfad) lautet, ist der Pfad f\u00fcr die Sicherheitskopien" + newline +
      "       \"c:/html_BAK\". Sie k\u00f6nnen den Pfad des Ordners f\u00fcr Sicherheitskopien \u00e4ndern, indem Sie im Feld" + newline +
      "       \"Backup files to folder:\" einen Pfad eingeben oder mit Hilfe der Schaltfl\u00e4che \"Browse\" nach dem gew\u00fcnschten Ordner suchen." + newline + newline +

      "       Unix(Solaris):" + newline +
      "       Der Standardpfad des Ordners f\u00fcr die Sicherheitskopien entspricht dem Quellpfad, an dessen Name \"_BAK\"" + newline +
      "       angeh\u00e4ngt wird. Wenn der Quellpfad also \"/home/user1/html/applet.html\" (Konvertierung einer Datei) lautet," + newline +
      "       ist der Pfad f\u00fcr die Sicherheitskopien \"/home/user1/html_BAK\". Wenn der Quellpfad" + newline +
      "       \"/home/user1/html\" (Konvertierung aller Dateien im Pfad) lautet, ist der Pfad f\u00fcr die Sicherheitskopien" + newline +
      "       \"/home/user1/html_BAK\". Sie k\u00f6nnen den Pfad des Ordners f\u00fcr" + newline +
      "       Sicherheitskopien \u00e4ndern, indem Sie im Feld \"Backup files to folder:\" einen Pfad eingeben oder mit Hilfe der Schaltfl\u00e4che \"Browse\" nach dem gew\u00fcnschten Ordner suchen." + newline + newline +
      "7)  Erzeugen einer Protokolldatei" + newline + newline +
      "       Wenn eine Protokolldatei erzeugt werden soll, aktivieren Sie das Kontrollk\u00e4stchen" + newline +
      "       \"Generate Log File\". Sie k\u00f6nnen den Pfad und Dateinamen eingeben oder mit Hilfe der Schaltfl\u00e4che \"Browse\"" + newline +
      "       einen Ordner ausw\u00e4hlen, den Dateinamen eingeben und dann \"Open\" ausw\u00e4hlen." + newline +
      "       Die Protokolldatei enth\u00e4lt grundlegende Informationen zum" + newline +
      "       Konvertierungsvorgang." + newline + newline +
      "8)  Ausw\u00e4hlen einer Konvertierungsvorlage" + newline + newline +
      "       Wenn Sie keine Vorlage ausw\u00e4hlen, wird eine Standardvorlage verwendet. Mit dieser Vorlage werden" + newline +
      "       konvertierte HTML-Dateien erzeugt, die mit IE und Netscape genutzt werden k\u00f6nnen." + newline  +
      "       Wenn Sie eine andere Vorlage verwenden m\u00f6chten, k\u00f6nnen Sie diese aus dem Men\u00fc im Haupt-Dialogfeld" + newline +
      "       ausw\u00e4hlen. Bei der Auswahl \"Other Template\" k\u00f6nnen Sie eine Datei ausw\u00e4hlen," + newline +
      "       die als Vorlage verwendet wird." + newline +
      "       Stellen Sie sicher, dass es sich bei der ausgew\u00e4hlten Datei UM EINE VORLAGE HANDELT." + newline + newline +
      "9)  Konvertieren" + newline + newline +
      "       Klicken Sie auf die Schaltfl\u00e4che \"Convert...\", um mit dem Konvertierungsvorgang zu beginnen. In einem" + newline +
      "       Dialogfeld wird die jeweils verarbeitete Datei, die Anzahl der zu verarbeitenden Dateien," + newline +
      "       die Anzahl der gefundenen Applets und die Anzahl der erkannten Fehler angezeigt." + newline + newline +
      "10) Konvertieren von weiteren Dateien oder Beenden des Programms" + newline + newline +
      "       Nach Abschluss der Konvertierung \u00e4ndert sich die Beschriftung der Schaltfl\u00e4che im Dialogfeld" + newline +
      "       von \"Cancel\" in \"Done\". Klicken Sie auf \"Done\", um das Dialogfeld zu schlie\u00dfen." + newline  +
      "       Sie k\u00f6nnen jetzt \"Quit\" ausw\u00e4hlen, um Java(tm) Plug-in HTML Converter" + newline +
      "       zu schlie\u00dfen, oder weitere Dateien zur Konvertierung ausw\u00e4hlen und erneut auf \"Convert...\" klicken." + newline + newline +
      "11) Einzelheiten \u00fcber Vorlagen" + newline + newline +
      "       Die Vorlagendatei bildet die Grundlage f\u00fcr die Konvertierung der Applets. Hierbei handelt es sich um eine einfache" + newline +
      "       Textdatei mit Tags, die Teile des urspr\u00fcnglichen Applets darstellen." + newline +
      "       Durch Hinzuf\u00fcgen, Entfernen und Verschieben der Tags in einer Vorlagendatei k\u00f6nnen Sie das Format der ausgegebenen," + newline +
      "       konvertierten Dateien \u00e4ndern." + newline + newline +
      "       Unterst\u00fctzte Tags:" + newline + newline +
      "        $OriginalApplet$    Dieses Tag wird durch den gesamten Text des" + newline +
      "        urspr\u00fcnglichen Applets ersetzt." + newline + newline +
      "        $AppletAttributes$   Dieses Tag wird durch alle Attribute des" + newline +
      "        Applets ersetzt (\"code\", \"codebase\", \"width\", \"height\" usw.)." + newline + newline +
      "        $ObjectAttributes$   Dieses Tag wird durch alle Attribute" + newline +
      "        ersetzt, die f\u00fcr das Tag \"object\" erforderlich sind." + newline + newline +
      "        $EmbedAttributes$   Dieses Tag wird durch alle Attribute ersetzt," + newline +
      "        die f\u00fcr das Tag \"embed\" erforderlich sind." + newline + newline +
      "        $AppletParams$    Dieses Tag wird durch alle Tags \"<param ...>\"" + newline +
      "        des Applets ersetzt." + newline + newline +
      "        $ObjectParams$    Dieses Tag wird durch alle Tags \"<param...>\"" + newline +
      "        ersetzt, die f\u00fcr das Tag \"object\" erforderlich sind." + newline + newline +
      "        $EmbedParams$     Dieses Tag wird durch alle Tags \"<param...>\"" + newline +
      "        ersetzt, die f\u00fcr das Tag \"embed\" erforderlich sind. Ersetzungen erfolgen in der Form NAME=WERT." + newline + newline +
      "        $AlternateHTML$  Dieses Tag wird durch den Text in dem Bereich" + newline +
      "        des urspr\u00fcnglichen Applets ersetzt, der bei fehlender Unterst\u00fctzung von Applets verwendet wird." + newline + newline +
      "        $CabFileLocation$   Dies ist der URL der CAB-Datei f\u00fcr Vorlagen," + newline +
      "        die f\u00fcr IE vorgesehen sind." + newline + newline +
      "        $NSFileLocation$    Dies ist der URL des Netscape-Plug-ins f\u00fcr Vorlagen," + newline +
      "        die f\u00fcr Netscape vorgesehen sind." + newline + newline +
      "        $SmartUpdate$   Dies ist der URL von Netscape SmartUpdate" + newline +
      "        f\u00fcr Vorlagen, die f\u00fcr Netscape Navigator 4.0 oder h\u00f6her vorgesehen sind." + newline + newline +
      "        $MimeType$    Dies ist der MIME-Typ des Java-Objekts." + newline + newline +

      "      \"default.tpl\" ist die Standardvorlage f\u00fcr HTML Converter. Die konvertierte Seite kann" + newline +
      "      in IE und Navigator unter Windows verwendet werden, um Java(TM) Plug-in aufzurufen." + newline +
      "      Diese Vorlage kann auch f\u00fcr Netscape unter Unix(Solaris) verwendet werden." + newline + newline +
      "      <!-- HTML CONVERTER -->" + newline +
      "      <OBJECT classid=\"clsid:E19F9330-3110-11d4-991C-005004D3B3DB\"" + newline +
      "      $ObjectAttributes$ codebase=\"$CabFileLocation$\">" + newline +
      "      $ObjectParams$" + newline +
      "      <PARAM NAME=\"type\" VALUE=\"$MimeType$\">" + newline +
      "      <PARAM NAME=\"scriptable\" VALUE=\"false\">" + newline +
      "      $AppletParams$<COMMENT>" + newline +
      "      <EMBED type=\"$MimeType$\" $EmbedAttributes$" + newline +
      "      $EmbedParams$ scriptable=false" + newline +
      "      pluginspage=\"$NSFileLocation$\"><NOEMBED>" + newline +
      "      </COMMENT>" + newline +
      "      $AlternateHTML$" + newline +
      "       </NOEMBED></EMBED>" + newline +
      "      </OBJECT>" + newline + newline +
      "      <!--" + newline +
      "      $ORIGINALAPPLET$" + newline +
      "      -->" + newline + newline +
      "      \"ieonly.tpl\": Die konvertierte Seite kann nur verwendet werden, um" + newline +
      "      Java (tm) Plug-in in IE unter Windows aufzurufen." + newline + newline +
      "      <!-- HTML CONVERTER -->" + newline +
      "      <OBJECT classid=\"clsid:E19F9330-3110-11d4-991C-005004D3B3DB\"" + newline +
      "      $ObjectAttributes$ codebase=\"$CabFileLocation$\">" + newline +
      "      $ObjectParams$" + newline +
      "      <PARAM NAME=\"type\" VALUE=\"$MimeType$\">" + newline +
      "      <PARAM NAME=\"scriptable\" VALUE=\"false\">" + newline +
      "      $AppletParams$" + newline +
      "      $AlternateHTML$" + newline +
      "      </OBJECT>" + newline + newline +
      "      <!--" + newline +
      "      $ORIGINALAPPLET$" + newline +
      "      -->" + newline + newline +
      "      \"nsonly.tpl\": Die konvertierte Seite kann verwendet werden, um Java(TM)" + newline +
      "      Plug-in in Navigator unter Windows und Solaris aufzurufen." + newline + newline +
      "      <!-- HTML CONVERTER -->" + newline +
      "      <EMBED type=\"$MimeType$\" $EmbedAttributes$" + newline +
      "      $EmbedParams$ scriptable=false" + newline +
      "      pluginspage=\"$NSFileLocation$\"><NOEMBED>" + newline +
      "      $AlternateHTML$" + newline +
      "      </NOEMBED></EMBED>" + newline + newline +
      "      <!--" + newline +
      "      $ORIGINALAPPLET$" + newline +
      "      -->" + newline + newline +
      "      \"extend.tpl\": Die konvertierte Seite kann in einem beliebigen Browser auf allen Plattformen verwendet werden." + newline +
      "      Falls es sich bei dem Browser um IE oder Navigator unter Windows (bzw. Navigator unter Solaris) handelt, wird Java(TM)" + newline +
      "      Plug-in aufgerufen. Andernfalls wird die regul\u00e4re JVM des Browsers verwendet." + newline + newline +
      "      <!-- HTML CONVERTER -->" + newline +
      "      <SCRIPT LANGUAGE=\"JavaScript\"><!--" + newline +
      "      var _info = navigator.userAgent; var _ns = false; var _ns6 = false;" + newline +
      "      var _ie = (_info.indexOf(\"MSIE\") > 0 && _info.indexOf(\"Win\") > 0 &&" + newline +
      "      _info.indexOf(\"Windows 3.1\") < 0);" + newline +
      "      //--></SCRIPT>" + newline +
      "      <COMMENT><SCRIPT LANGUAGE=\"JavaScript1.1\"><!--" + newline +
      "      var _ns = (navigator.appName.indexOf(\"Netscape\") >= 0 &&" + newline +
      "      ((_info.indexOf(\"Win\") > 0 && _info.indexOf(\"Win16\") < 0 &&" + newline +
      "      java.lang.System.getProperty(\"os.version\").indexOf(\"3.5\") < 0) ||" + newline +
      "      _info.indexOf(\"Sun\") > 0));" + newline +
      "      var _ns6 = ((_ns == true) && (_info.indexOf(\"Mozilla/5\") >= 0));" + newline +
      "      //--></SCRIPT></COMMENT>" + newline + newline +
      "      <SCRIPT LANGUAGE=\"JavaScript\"><!--" + newline +
      "      if (_ie == true) document.writeln('<OBJECT" + newline +
      "      classid=\"clsid:E19F9330-3110-11d4-991C-005004D3B3DB\"" + newline +
      "      $ObjectAttributes$" + newline +
      "      codebase=\"$CabFileLocation$\"><NOEMBED><XMP>');" + newline +
      "      else if (_ns == true && _ns6 == false) document.writeln('<EMBED" + newline +
      "      type=\"$MimeType$\" $EmbedAttributes$" + newline +
      "      $EmbedParams$ scriptable=false" + newline +
      "      pluginspage=\"$NSFileLocation$\"><NOEMBED><XMP>');" + newline +
      "      //--></SCRIPT>" + newline +
      "      <APPLET $AppletAttributes$></XMP>" + newline +
      "      $ObjectParams$" + newline +
      "      <PARAM NAME=\"type\" VALUE=\"$MimeType$\">" + newline +
      "      <PARAM NAME=\"scriptable\" VALUE=\"false\">" + newline +
      "      $AppletParams$" + newline +
      "      $AlternateHTML$" + newline +
      "      </APPLET>" + newline +
      "      </NOEMBED></EMBED></OBJECT>" + newline + newline +
      "      <!--" + newline +
      "      $ORIGINALAPPLET$" + newline +
      "      -->" + newline + newline +
      "12)  Ausf\u00fchren von HTML Converter" + newline + newline +
      "      Ausf\u00fchren von HTML Converter mit grafischer Benutzeroberfl\u00e4che" + newline + newline +
      "      HTML Converter ist im SDK enthalten, nicht in der JRE. Um HTML Converter zu starten, wechseln Sie" + newline +
      "      in das Unterverzeichnis \"lib\" des SDK-Installationsverzeichnisses. Wenn Sie das SDK" + newline +
      "      beispielsweise unter Windows direkt auf Laufwerk C installiert haben, wechseln Sie in das Verzeichnis " + newline + newline +
      "            C:\\j2sdk1.4.2\\lib\\." + newline + newline +
      "      HTML Converter (htmlconverter.jar) befindet sich in diesem Verzeichnis." + newline + newline +
      "      Geben Sie den folgenden Befehl ein, um HTML Converter zu starten:" + newline + newline +
      "            C:\\j2sdk1.4.2\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      Unter UNIX bzw. Linux wird HTML Converter auf \u00e4hnliche Weise mit dem oben genannten Befehl gestartet." + newline +
      "      Im Folgenden finden Sie einige andere Methoden, mit denen HTML Converter gestartet werden kann:" + newline + newline +
      "      Windows" + newline +
      "      So starten Sie HTML Converter aus dem Explorer:" + newline +
      "      Navigieren Sie im Explorer zu folgendem Verzeichnis:" + newline + newline +
      "      C:\\j2sdk1.4.2\\bin" + newline + newline +
      "      Doppelklicken Sie auf die Anwendung \"HtmlConverter\"." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      Geben Sie die folgenden Befehle ein:" + newline + newline +
      "      cd /j2sdk1.4.2/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +
      "      Starten von HTML Converter an der Befehlszeile:" + newline + newline +
      "      Format:" + newline + newline +
      "      java -jar htmlconverter.jar [-Option1 Wert1 [-Option2 Wert2" + newline +
      "      [...]]] [-simulate] [Dateibezeichner]" + newline + newline +
      "      Dateibezeichner: durch Leerzeichen getrennte Liste mit Dateibezeichnern; \"*\" kann als Platzhalter verwendet werden. " + newline +
      "      (*.html *.htm)" + newline + newline +
      "      Optionen:" + newline + newline +
      "       source:    Pfad zu den Dateien. (\"c:\\htmldocs\" unter Windows," + newline +
      "                  \"/home/user1/htmldocs\" unter Unix). Standardwert: <Benutzerverzeichnis>" + newline +
      "                  Wenn es sich um einen relativen Pfad handelt, wird angenommen, dass er relativ zu dem Verzeichnis ist," + newline +
      "                  in dem HTMLConverter gestartet wurde." + newline + newline +
      "       backup:    Pfad, in den Sicherungskopien der Dateien geschrieben werden. Standardwert:" + newline +
      "                  <Benutzerverzeichnis>/<Quellverzeichnis>_bak" + newline +
      "                  Wenn es sich um einen relativen Pfad handelt, wird angenommen, dass er relativ zu dem Verzeichnis ist," + newline +
      "                  in dem HTMLConverter gestartet wurde." + newline + newline +
      "       subdirs:   Legt fest, ob Dateien in Unterverzeichnissen verarbeitet werden sollen. " + newline +
      "                  Standardwert: FALSE" + newline + newline +
      "       template:  Name der Vorlagendatei. Standardwert: default.tpl (Standard " + newline +
      "                  (IE & Navigator) nur f\u00fcr Windows & Solaris) VERWENDEN SIE DIE STANDARDVORLAGE, WENN SIE SICH NICHT SICHER SIND, WELCHE VORLAGE SIE VERWENDEN SOLLTEN." + newline + newline +
      "       log:       Pfad und Name der Protokolldatei. (Standardwert: <Benutzerverzeichnis>/convert.log)" + newline + newline +
      "       progress:  Legt fest, ob der Fortschritt bei der Konvertierung an der Standardausgabe angezeigt wird. " + newline +
      "                  Standardwert: false" + newline + newline +
      "       simulate:  Es werden Detailangaben zur Konvertierung angezeigt, ohne die Konvertierung durchzuf\u00fchren." + newline +
      "                  VERWENDEN SIE DIESE OPTION, WENN UNKLARHEITEN BEZ\u00dcGLICH DER KONVERTIERUNG BESTEHEN." + newline +
      "                  SIE ERHALTEN DANN EINE LISTE MIT DETAILANGABEN ZUR" + newline +
      "                  KONVERTIERUNG." + newline + newline +
      "      Wenn Sie nur \"java -jar htmlconverter.jar -gui\" angeben (also nur die Option \"-gui\"" + newline +
      "      ohne Dateibezeichner), wird HTML Converter mit einer grafischen Benutzeroberfl\u00e4che gestartet." + newline  +
      "      Andernfalls wird die grafische Benutzeroberfl\u00e4che nicht angezeigt." + newline + newline +
      "      Weitere Information finden Sie unter der folgenden URL:" + newline + newline +
      "      http://java.sun.com/j2se/1.4.2/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
}




