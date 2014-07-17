/*
 * @(#)ControlPanelHelp_sv.java	1.3 03/05/16
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

public class ControlPanelHelp_sv extends ListResourceBundle {

    private static String newline = "\n";

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "help.file", newline +
                   "Kontrollpanelen f\u00f6r Java Plug-in Hj\u00e4lp" + newline + newline + newline +
                   "F\u00f6ljande \u00e4mnen ber\u00f6rs:" + newline +
                   "      \u00d6versikt" + newline + 
                   "      Sparalternativ" + newline + 
                   "      St\u00e4lla in alternativ i Kontrollpanelen" + newline + 
                   "      Baspanel" + newline + 
                   "      Avancerad panel" + newline +
                   "      Webbl\u00e4sarpanel" + newline + 
                   "      Proxy-panel" + newline + 
                   "      Cache-panel" + newline + 
                   "      Certifikatpanel" + newline + 
                   "      Uppdateringspanel" + newline + newline + 
                   "\u00d6versikt" + newline + newline +
                   "Kontrollpanelen f\u00f6r Java Plug-in g\u00f6r det m\u00f6jligt att \u00e4ndra de standardinst\u00e4llningar som Java Plug-in anv\u00e4nder vid uppstart." + newline +
                   "Alla appletar som k\u00f6rs av Java Plug-in anv\u00e4nder sig av dessa inst\u00e4llningar." + newline +
                   "Java Plug-in Developer Guide, som n\u00e4mns i texten, finns p\u00e5 (\u00e4ndringar av URL kan f\u00f6rekomma):" + newline +
                   "http://java.sun.com/j2se/1.4/docs/guide/plugin/developer_guide/contents.html" + newline + newline +
                   "Sparalternativ" + newline + newline +
                   "N\u00e4r du \u00e4r f\u00e4rdig med dina \u00e4ndringar i kontrollpanelen ska du klicka p\u00e5 Till\u00e4mpa f\u00f6r att spara \u00e4ndringarna." + newline +
                   "Klicka \u00c5terst\u00e4ll f\u00f6r att avbryta \u00e4ndringarna och ladda om de v\u00e4rden som senast skrevs in och sparades." + newline +
                   "Observera att det inte \u00e4r samma set av standardv\u00e4rden som ursprungligen st\u00e4lldes in f\u00f6r Java Plug-in n\u00e4r det installerades." + newline + newline +
                   "St\u00e4lla in alternativ i Kontrollpanelen" + newline + newline +
                   "Det finns sex paneler i kontrollpanelen d\u00e4r du kan st\u00e4lla in olika alternativ." + newline +
                   "Dessa paneler ben\u00e4mns: " + newline +
                   "      Bas" + newline + 
                   "      Avancerad" + newline + 
                   "      Webbl\u00e4sare" + newline + 
                   "      Proxies" + newline + 
                   "      Cache" + newline + 
                   "      Certifikat" + newline + 
                   "      Uppdatering" + newline + newline +
                   "Var och en beskrivs separat i texten nedan." + newline + newline + newline +
                   " Bas" + newline +
                   "Visa Java-systemf\u00f6nster" + newline + newline + 
                   "      Visar Java-systemf\u00f6nster medan appletar k\u00f6rs. Systemf\u00f6nstret visar meddelanden som skrivs ut av System.out och System.err." + newline +
                   "      Det \u00e4r anv\u00e4ndbart n\u00e4r problem ska \u00e5terst\u00e4llas." + newline + newline +
                   "D\u00f6lj systemf\u00f6nster" + newline + newline + 
                   "      Java-systemf\u00f6nstret k\u00f6r men visas ej. Detta \u00e4r standardinst\u00e4llningen (ikryssad)." + newline + newline +
                   "Starta ej systemf\u00f6nster" + newline + newline + 
                   "      Java-systemf\u00f6nster startas ej." + newline + newline + 
                   "Visa dialogruta f\u00f6r undantag" + newline + newline + 
                   "      Visar en dialogruta f\u00f6r undantag n\u00e4r dessa sker. Standard \u00e4r att inte visa dialogrutan f\u00f6r undantag (ej ikryssad)." + newline + newline +
                   "Visa Java i Aktivitetsf\u00e4ltet (g\u00e4ller endast Windows)" + newline + newline + 
                   "      Visar Java-logotypen (kaffekoppen) i Aktivitetsf\u00e4ltet n\u00e4r Java Plug-in-programmet \u00e4r startat" + newline +
                   "      och tar bort den fr\u00e5n Aktivitetsf\u00e4ltet n\u00e4r Java Plug-in-programmet \u00e4r st\u00e4ngt." + newline +
                   "      Java-logotypen (kaffekoppen) med kaffekoppen visar f\u00f6r anv\u00e4ndaren att en Java VM k\u00f6rs. Logotypen inneh\u00e5ller " + newline +
                   "      information om den aktuella Java-versionen och g\u00f6r att du kan kontrollera Java-systemf\u00f6nstret." + newline +
                   "      Det h\u00e4r alternativet \u00e4r aktiverat som standard (ikryssad)." + newline + newline +
                   "      Java-funktioner i Aktivitetsf\u00e4ltet:" + newline + newline + 
                   "      N\u00e4r musen pekar p\u00e5 Java-logotypen (kaffekoppen) visas texten \"Java\"." + newline + newline +

                   "      N\u00e4r du dubbelklickar med v\u00e4nster musknapp p\u00e5 Java-ikonen i Aktivitetsf\u00e4ltet \u00f6ppnas Java-systemf\u00f6nstret." + newline + newline + 
                   "      N\u00e4r du h\u00f6gerklickar p\u00e5 Java-ikonen visas en popup-meny med f\u00f6ljande menyalternativ:" + newline + newline +
                   "            \u00d6ppna/st\u00e4ng systemf\u00f6nster" + newline + 
                   "            Om Java" + newline + 
                   "            St\u00e4ng av" + newline + 
                   "            Avsluta" + newline + newline + 
                   "      \u00d6ppna/st\u00e4ng systemf\u00f6nster \u00f6ppnar och st\u00e4nger Java-systemf\u00f6nstret. Menyalternativet visar \u00d6ppna systemf\u00f6nstret om" + newline +
                   "       Java-systemf\u00f6nstret \u00e4r dold och St\u00e4ng systemf\u00f6nstret om Java-systemf\u00f6nstret visas." + newline + newline +
                   "      Om Java visar dialogrutan Om f\u00f6r Java 2 Standard Edition." + newline + newline +
                   "      St\u00e4ng av st\u00e4nger av och tar bort Java-ikonen fr\u00e5n Aktivitetsf\u00e4ltet under b\u00e5de den aktuella sessionen och framtida sessioner. N\u00e4r Java Plug-in startas" + newline +
                   "      om visas inte Java-ikonen i Aktivitetsf\u00e4ltet." + newline +
                   "      Information om hur du visar Java-ikonen i Aktivitetsf\u00e4ltet efter att den har inaktiverats finns i kommentaren nedan." + newline + newline +
                   "      Avsluta tar enbart bort Java-ikonen fr\u00e5n Aktivitetsf\u00e4ltet under den aktuella sessionen. N\u00e4r Java Plug-in startas om visas" + newline +
                   "      Java-ikonen i Aktivitetsf\u00e4ltet igen." + newline + newline + newline + 
                   "                Obs" + newline + newline +
                   "                1. Om \"Visa Java i Aktivitetsf\u00e4ltet\" \u00e4r ikryssad visas Java-ikonen i Aktivitetsf\u00e4ltet \u00e4ven om \"Starta inte" + newline +
                   "                systemf\u00f6nstret\" \u00e4r ikryssad." + newline + newline +
                   "                2. Starta kontrollpanelen f\u00f6r Java Plug-in, kryssa i \"Visa Java i Aktivitetsf\u00e4ltet\" och tryck p\u00e5 \"Anv\u00e4nd\" om du" + newline +
                   "                vill aktivera Java-ikonen igen efter att den har inaktiverats." + newline + newline +
                   "                3. Om andra Java VM-sessioner redan k\u00f6rs och andra Java-ikoner har lagts till i Aktivitetsf\u00e4ltet p\u00e5verkas inte dessa" + newline +
                   "                ikoner n\u00e4r du \u00e4ndrar inst\u00e4llningen i kontrollpanelen f\u00f6r Java Plug-in." + newline +
                   "                Inst\u00e4llningen p\u00e5verkar bara ikoner f\u00f6r Java VM-sessioner som startas efter \u00e4ndringen." + newline + newline + newline +
                   " Avancerad" + newline + newline + newline +
                   "Java Run Time Environment" + newline + newline +
                   "      G\u00f6r det m\u00f6jligt f\u00f6r Java Plug-in att k\u00f6ra med vilken Java 2 JRE eller SDK, Standard Edition v 1.3 eller 1.4 som helst installerad p\u00e5 din dator." + newline +
                   "      Java Plug-in 1.3/1.4 levereras med en standard-JRE." + newline +
                   "      Du kan dock \u00e5sidos\u00e4tta standard-JRE och anv\u00e4nda en \u00e4ldre eller nyare version. Kontrollpanelen uppt\u00e4cker automatiskt" + newline +
                   "      alla versioner av Java 2 SDK eller JRE som finns installerade p\u00e5 dator. I listrutan visas alla de versioner" + newline +
                   "      du kan anv\u00e4nda." + newline +
                   "      Den f\u00f6rsta posten i listan \u00e4r alltid standard-JRE; den sista \u00e4r alltid Annan. Om du v\u00e4ljer Annan, m\u00e5ste du ange" + newline +
                   "      s\u00f6kv\u00e4gen till Java 2 JRE eller SDK, Standard Edition v 1.3/1.4." + newline + newline + 
                   "                Obs" + newline + newline +
                   "                Endast anv\u00e4ndare med tillr\u00e4ckliga kunskaper b\u00f6r anv\u00e4nda detta alternativ. Att \u00e4ndra standard-JRE \u00e4r inte att rekommendera." + newline + newline + newline +
                   "Java Runtime-parametrar" + newline + newline + 
                   "      \u00c5sidos\u00e4tter standardparametrarna i Java Plug-in genom att ange andra alternativ. Samma syntax anv\u00e4nds till" + newline +
                   "      parametrarna som till Javas kommandolinjer. Se dokumentation f\u00f6r Java 2 Standard Edition (J2SE)" + newline +
                   "      f\u00f6r en fullst\u00e4ndig lista \u00f6ver kommandolinjealternativ." + newline +
                   "      \u00c4ndringar av URL kan f\u00f6rekomma:" + newline + newline +
                   "            http://java.sun.com/j2se/1.4/docs/tooldocs/<platform>/java.html" + newline + newline + 
                   "            d\u00e4r <platform> \u00e4r ett av f\u00f6ljande operativsystem: Solaris, Linux, Win32." + newline + newline + 
                   "      Nedan finns n\u00e5gra exempel p\u00e5 Java Runtime-parametrar." + newline + newline +
                   "      Aktivera och deaktivera f\u00f6rs\u00e4kransupport" + newline + newline +
                   "            F\u00f6ljande systemegenskaper m\u00e5ste anges i Java Runtime-parametrarna f\u00f6r att f\u00f6rs\u00e4kransupporten ska aktiveras:" + newline + newline +
                   "                  -D[ enableassertions | ea ][:<package name>\"...\" | : <class name> ]" + newline + newline +
                   "            Ange f\u00f6ljande Java Runtime-parametrar f\u00f6r att deaktivera f\u00f6rs\u00e4kran i Java Plug-in:" + newline + newline + 
                   "                  -D[ disableassertions | da ][:<package name>\"...\" | : <class name> ]" + newline + newline + 
                   "            Se Assertion Facility f\u00f6r mer detaljer om att aktivera/deaktivera f\u00f6rs\u00e4kran." + newline +
                   "            http://java.sun.com/j2se/1.4/docs/guide/lang/assert.html (\u00c4ndringar av URL kan f\u00f6rekomma)." + newline + newline +
                   "            F\u00f6rs\u00e4kran deaktiveras som standard i Java Plug-inkod. Eftersom effekten av f\u00f6rs\u00e4kran best\u00e4ms under uppstarten av Java Plug-in" + newline +
                   "            s\u00e5 kr\u00e4ver en \u00e4ndring av f\u00f6rs\u00e4kransinst\u00e4llningarna i Kontrollpanelen f\u00f6r Java Plug-in att webbl\u00e4saren startas" + newline +
                   "            om f\u00f6r att de nya inst\u00e4llningarna ska ge effekt." + newline + newline + 
                   "            Eftersom Java-koden i Java Plug-in \u00e4ven har inbyggd f\u00f6rs\u00e4kran, s\u00e5 \u00e4r det m\u00f6jligt att aktivera f\u00f6rs\u00e4kran i" + newline +
                   "            Java Plug-inkod genom:" + newline + newline +
                   "                  -D[ enableassertions | ea ]:sun.plugin" + newline + newline + 
                   "      Sp\u00e5ra och logga support" + newline + newline +
                   "            Sp\u00e5rning anv\u00e4nds f\u00f6r att omdirigera utdata i Java-systemf\u00f6nstret till en sp\u00e5rfil (.plugin<version>.trace)." + newline + newline +
                   "                  -Djavaplugin.trace=true" + newline +
                   "                  -Djavaplugin.trace.option=basic|net|security|ext|liveconnect" + newline + newline +
                   "            Om du inte vill anv\u00e4nda standardnamnet p\u00e5 sp\u00e5rfilen:" + newline + newline +
                   "                  -Djavaplugin.trace.filename=<tracefilename>" + newline + newline + 
                   "            Liksom sp\u00e5rning, anv\u00e4nds loggning f\u00f6r att omdirigera utdata i Java-systemf\u00f6nstret till en loggfil (.plugin<version>.log)" + newline +
                   "            med hj\u00e4lp av Java Logging API." + newline +
                   "            Loggning kan startas genom att aktivera egenskapen javaplugin.logging." + newline + newline +
                   "                  -Djavaplugin.logging=true" + newline + newline +
                   "            Om du inte vill anv\u00e4nda standardnamnet p\u00e5 loggfilen:" + newline + newline +
                   "                  -Djavaplugin.log.filename=<logfilename>" + newline + newline + 
                   "            Dessutom, om du inte vill skriva \u00f6ver sp\u00e5r- eller loggfilerna vid varje session, kan du st\u00e4lla in egenskapen:" + newline + newline + 
                   "                  -Djavaplugin.outputfiles.overwrite=false." + newline + newline + 
                   "            Om egenskapen st\u00e4lls in som false, kommer sp\u00e5r- och loggfiler att f\u00e5 unika namn vid varje session. Om standardnamnen p\u00e5 sp\u00e5r-" + newline +
                   "            och loggfilerna anv\u00e4nds, kommer filerna att namnges s\u00e5 h\u00e4r:" + newline + newline +
                   "                  .plugin<username><date hash code>.trace" + newline +
                   "                  .plugin<username><date hash code>.log" + newline + newline +
                   "            Sp\u00e5rning och loggning som st\u00e4llts in med hj\u00e4lp av kontrollpanelen ger effekt n\u00e4r Plug-in startas" + newline +
                   "            men \u00e4ndringar som utf\u00f6rs medan Plug-in k\u00f6r har ingen effekt f\u00f6rr\u00e4n efter omstart." + newline + newline + 
                   "            F\u00f6r mer information om sp\u00e5rning och loggning, se Tracing and Logging i Java Plug-in Developer Guide." + newline + newline +
                   "      Debugga program i Java Plug-in" + newline + newline +
                   "            F\u00f6ljande alternativ anv\u00e4nds n\u00e4r program debuggas i Java Plug-in." + newline +
                   "            F\u00f6r mer information om detta \u00e4mne, se Debugging Support i Java Plug-in Developer Guide." + newline + newline +
                   "                  -Djava.compiler=NONE" + newline +
                   "                  -Xnoagent" + newline +
                   "                  -Xdebug" + newline + 
                   "                  -Xrunjdwp:transport=dt_shmem,address=<connect-address>,server=y,suspend=n" + newline + newline +
                   "            <connect-address> kan vara vilken str\u00e4ng som helst (exempel: 2502) som anv\u00e4nds av Java Debugger (jdb) f\u00f6r" + newline +
                   "            att senare ansluta till JVM." + newline + newline + 
                   "      Standard f\u00f6r anslutningstimeout" + newline + newline +
                   "            N\u00e4r en anslutning till en server utf\u00f6rs av ett program och servern inte svarar, kan programmet h\u00e4nga sig" + newline +
                   "            och dessutom orsaka att webbl\u00e4saren h\u00e4nger sig eftersom det inte finns n\u00e5gon anslutningstimeout i n\u00e4tverket" + newline +
                   "            (som standard \u00e4r den inst\u00e4lld till -1 vilket inneb\u00e4r att det inte finns n\u00e5gon timeout)." + newline + newline +
                   "            F\u00f6r att undvika det h\u00e4r problemet har Java Plug-in lagt in ett standardv\u00e4rde p\u00e5 timeout (2 minuter) f\u00f6r alla HTTP-anslutningar.:" + newline +
                   "            Du kan \u00e5sidos\u00e4tta denna inst\u00e4llning i Java Runtime-parametrarna:" + newline + newline +
                   "                  -Dsun.net.client.defaultConnectTimeout=value in milliseconds" + newline + newline +
                   "            En annan n\u00e4tverksegenskap som du kan st\u00e4lla in \u00e4r sun.net.client.defaultReadTimeout." + newline + newline +
                   "                  -Dsun.net.client.defaultReadTimeout=value in milliseconds" + newline + newline +
                   "                  Obs" + newline + newline +
                   "                  Java Plug-in st\u00e4ller inte in sun.net.client.defaultReadTimeout som standard. Om du vill g\u00f6ra en s\u00e5dan inst\u00e4llning" + newline +
                   "                  g\u00f6r du det med hj\u00e4lp av de Java Runtime-parametrar som visats ovan." + newline + newline + newline +
                   "            Beskrivning av n\u00e4tverksegenskaper:" + newline + newline +
                   "            sun.net.client.defaultConnectTimeout" + newline +
                   "            sun.net.client.defaultReadTimeout" + newline + newline +
                   "                  Dessa egenskaper anger standardv\u00e4rdena f\u00f6r anslutnings- respektive l\u00e4stimeout f\u00f6r de protokollhanterare som anv\u00e4nds" + newline +
                   "                  av java.net.URLConnection. Det standardv\u00e4rde som st\u00e4llts in av protokollhanterarna \u00e4r -1, vilket inneb\u00e4r" + newline +
                   "                  att det inte finns n\u00e5gon timeout inst\u00e4lld." + newline + newline +
                   "                  sun.net.client.defaultConnectTimeout anger timeout (i millisekunder) f\u00f6r att etablera kontakt med v\u00e4rden." + newline +
                   "                  F\u00f6r http-anslutningar \u00e4r det till exempel timeout f\u00f6r anslutningen till http-servern." + newline +
                   "                  F\u00f6r ftp-anslutningar \u00e4r det timeout f\u00f6r anslutningen till ftp-servrar." + newline + newline +
                   "                  sun.net.client.defaultReadTimeout anger timeout (i millisekunder) n\u00e4r man l\u00e4ser fr\u00e5n en indatastr\u00f6m d\u00e5 en" + newline +
                   "                  anslutning etablerats med en resurs." + newline + newline + 
                   "            Den officiella beskrivningen av dessa n\u00e4tverksegenskaper finns" + newline +
                   "            p\u00e5 http://java.sun.com/j2se/1.4/docs/guide/net/properties.html." + newline + newline + newline +
                   " Webbl\u00e4sare" + newline + newline + newline +
                   "Den h\u00e4r panelen h\u00e4nvisar endast till Microsoft Windows-installationer; den f\u00f6rekommer inte i andra installationer. Kryssa f\u00f6r den webbl\u00e4sare" + newline +
"f\u00f6r vilken du vill att Java Plug-in ska vara standard Java-k\u00f6rtid, ist\u00e4llet f\u00f6r l\u00e4sarens interna JVM." + newline +
"Det \u00e4r f\u00f6r att aktivera APPLET tag support i Internet Explorer och Netscape 6 via Java Plug-in." + newline + newline + newline +
                   " Proxies" + newline + newline +newline +
                   "Anv\u00e4nd Proxy-panelen f\u00f6r att anv\u00e4nda webbl\u00e4sarens standardinst\u00e4llningar eller f\u00f6r att \u00e5sidos\u00e4tta proxy-adress och -port f\u00f6r de olika protokollen." + newline + newline + 
                   "Anv\u00e4nd webbl\u00e4sarinst\u00e4llningar" + newline + newline + 
                   "      Kryssa i h\u00e4r f\u00f6r att anv\u00e4nda webbl\u00e4sarens standardinst\u00e4llningar f\u00f6r proxy. Detta \u00e4r standardinst\u00e4llningen (ikryssad)." + newline + newline +
                   "Informationstabell f\u00f6r proxy" + newline + newline + 
                   "      Du kan \u00e5sidos\u00e4tta standardinst\u00e4llningarna genom att klicka bort krysset ur kryssrutan \"Anv\u00e4nd webbl\u00e4sarinst\u00e4llningar\" och sedan" + newline +
                   "      g\u00f6ra f\u00e4rdig informationstabellen nedanf\u00f6r kryssrutan. Du kan skriva in proxy-adress och -port f\u00f6r var och" + newline +
                   "      en av de underst\u00f6dda protokollen: HTTP, Secure (HTTPS), FTP, Gopher och Socks." + newline + newline + 
                   "Ingen proxy-v\u00e4rd" + newline + newline + 
                   "      Detta \u00e4r en v\u00e4rd eller lista \u00f6ver v\u00e4rdar f\u00f6r vilka inga proxy/proxies ska anv\u00e4ndas. \"Ingen proxy-v\u00e4rd\" anv\u00e4nds" + newline +
                   "      oftast f\u00f6r en intern v\u00e4rd i en intranet-milj\u00f6." + newline + newline + 
                   "Automatisk proxykonfigurations-URL" + newline + 
                   "      Detta \u00e4r en URL f\u00f6r JavaScript-filen (.js eller .pac suffix) som inneh\u00e5ller FindProxyForURL-funktionen." + newline +
                   "      FindProxyForURL best\u00e4mmer vilken proxy-server som kan anv\u00e4ndas f\u00f6r en anslutningsf\u00f6rfr\u00e5gan." + newline + newline + 
                   "F\u00f6r mer information om proxy-konfiguration, se Proxy Configuration i Java Plug-in" + newline +
                   "Developer Guide." + newline + newline + newline +
                   " Cache" + newline + newline + newline +
                   "           Obs" + newline + newline +
                   "           Cachen som det h\u00e4nvisas till h\u00e4r \u00e4r sticky cache; dvs den disk-cache som skapats och kontrolleras av Java Plug-in och som" + newline +
                   "           webbl\u00e4saren inte kan skriva \u00f6ver. Se Applet Caching i Java Plug-in Developer Guide f\u00f6r mer information." + newline + newline + newline +
                   "Aktivera Caching" + newline + newline + 
                   "      Kryssa i h\u00e4r f\u00f6r att aktivera caching. Detta \u00e4r standardinst\u00e4llningen (ikryssad). Med Applet Caching aktiverad" + newline +
                   "      f\u00f6rb\u00e4ttras prestanda eftersom en applet som en g\u00e5ng blivit cachad inte l\u00e4ngre beh\u00f6ver laddas ner n\u00e4r man h\u00e4nvisar till den igen." + newline + newline +
                   "      Java Plug-in cachar f\u00f6ljande typer av filer som laddas ner via HTTP/HTTPS:" + newline + newline +
                   "            .jar (jar-fil)" + newline +
                   "            .zip (zip-fil)" + newline +
                   "            .class (java-klassfil)" + newline +
                   "            .au (ljudfil)" + newline +
                   "            .wav (ljudfil)" + newline +
                   "            .jpg (bilddfil)" + newline +
                   "            .gif (bilddfil)" + newline + newline +
                   "Visa filer i cachen" + newline + newline + 
                   "      Tryck h\u00e4r f\u00f6r att se de cachade filerna. En annan dialog (Java Plug-in Cache Viewer) dyker upp som visar de cachade filerna." + newline +
                   "      Cache Viewer visar f\u00f6ljande information om filer i cache: Namn, Typ, Storlek, F\u00f6rfallodatum," + newline +
                   "      Senast \u00e4ndrad, Version och URL. I Cache Viewer kan du ocks\u00e5 selektivt ta bort filer i cachen." + newline +
                   "      Detta \u00e4r ett alternativ till T\u00f6m cache-alternativet (beskrivs nedan), som tar bort alla filer i cachen." + newline + newline +
                   "T\u00f6m cachen" + newline + newline + 
                   "      Tryck h\u00e4r f\u00f6r att ta bort alla filer i cachen. Du kommer att bli tillfr\u00e5gad (Ta bort alla filer i ... _cachen?) innan filerna tas bort." + newline + newline + 
                   "L\u00e4ge" + newline + newline + 
                   "      Du kan anv\u00e4nda f\u00f6ljande f\u00f6r att ange cachens l\u00e4ge. Cachens standardl\u00e4ge \u00e4r <user home>/.jpi_cache d\u00e4r <user home>" + newline +
                   "      \u00e4r v\u00e4rdet p\u00e5 systemegenskapen user.home. Dess v\u00e4rde beror p\u00e5 operativsystemet." + newline + newline +
                   "Storlek" + newline + newline + 
                   "      Du kan kryssa f\u00f6r Obegr\u00e4nsad f\u00f6r att g\u00f6ra cachen obegr\u00e4nsat stor eller st\u00e4lla in Maxstorlek f\u00f6r cachen." + newline +
                   "      Om cachestorleken \u00f6verskrider den gr\u00e4ns som angivits, kommer de \u00e4ldsta filerna att tas bort tills cachestorleken" + newline +
                   "      \u00e4r innanf\u00f6r gr\u00e4nsen." + newline + newline + 
                   "Kompression" + newline + newline + 
                   "      Du kan st\u00e4lla in JAR-cachefilernas kompression mellan Ingen och H\u00f6g. Medan du sparar minne genom att" + newline +
                   "      ange h\u00f6gre kompression, minskar du samtidigt prestandan. B\u00e4st prestanda uppn\u00e5s" + newline +
                   "      utan kompression." + newline + newline + newline + 
                   " Certifikat" + newline + newline + newline +
                   "Fyra typer av certifikat kan v\u00e4ljas:" + newline + newline +
                   "      Signerad applet" + newline + 
                   "      S\u00e4ker site" + newline + 
                   "      Signerar-CA" + newline +
                   "      S\u00e4ker site CA" + newline + newline + 
                   "Signerad applet" + newline + newline +
                   "      Dessa \u00e4r certifikat f\u00f6r signerade appletar som anv\u00e4ndaren accepterar. Certifikaten som visar sig i listan \u00f6ver appletar, l\u00e4ses" + newline +
                   "      fr\u00e5n certifikatfilen jpicerts<version> som finns i <user home>/.java-katalogen." + newline + newline +
                   "S\u00e4ker site" + newline + newline +
                   "      Dessa \u00e4r certifikat f\u00f6r s\u00e4kra sites. Certifikaten som visar sig i den listan \u00f6ver S\u00e4kra sites l\u00e4ses fr\u00e5n certifikatfilen" + newline +
                   "      jpicerts<version> som finns i <user home>/.java -katalogen." + newline + newline + 
                   "Signerar-CA" + newline + newline +
                   "      Dessa \u00e4r certifikat fr\u00e5n Certificate Authorities (CAs) f\u00f6r signerade appletar. Det \u00e4r de som ger ut certifikaten till" + newline +
                   "       signerarna. Certifikaten som visar sig i listan \u00f6ver Signerar-CA l\u00e4ses fr\u00e5n certifikatfilen cacerts som finns" + newline +
                   "       i <jre>/lib/security-katalogen." + newline + newline +
                   "S\u00e4ker site CA" + newline + newline +
                   "      Dessa \u00e4r certifikat fr\u00e5n Certificate Authorities (CAs) f\u00f6r s\u00e4kra sites. Det \u00e4r de som ger ut certifikaten f\u00f6r" + newline +
                   "      s\u00e4kra sites. Certifikaten som visar sig i S\u00e4kra sites CA-listan l\u00e4ses fr\u00e5n certifikatfilen jssecacerts som finns" + newline +
                   "      i <jre>/lib/security-katalogen." + newline + newline +
                   "F\u00f6r Signerad applet- och S\u00e4ker site-certifikat finns det fyra alternativ: Importera, Exportera, Ta bort och Detaljer." + newline +
                   "Anv\u00e4ndaren kan importera, exportera, ta bort och se detaljer om ett certifikat." + newline + newline + 
                   "F\u00f6r Signerar- CA- och S\u00e4ker site CA-certifikat finns det bara ett alternativ: Detaljer. Anv\u00e4ndaren kan endast se detaljer om ett certifikat." + newline + newline + newline +
                   " Uppdatera" + newline + newline + newline +
                   "I den h\u00e4r panelen finns det en \"Java Uppdatering\"-knapp som l\u00e5ter anv\u00e4ndare f\u00e5 den senaste versionen f\u00f6r Java Runtime" + newline +
"Environment fr\u00e5n webbsidan Java Update. Den h\u00e4r panelen h\u00e4nvisar endast till Microsoft Windows-plattformen; den f\u00f6rekommer inte" + newline +
"i andra plattformar (t ex Solaris/Linux)."}
};
} 

 



