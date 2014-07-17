/*
 * @(#)ConverterHelp_sv.java	1.2 03/04/24
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Swedish version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_sv extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");;

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Java(tm) Plug-in HTML Converter Readme" + newline + newline +
      "Version:  1.4.2" + newline + newline + newline +
      "*****   S\u00c4KERHETSKOPIERA ALLA FILER INNAN DU KONVERTERAR DEM MED DETTA VERKTYG" + newline +
      "*****   \u00c4NDRINGARNA \u00c5NGRAS INTE \u00c4VEN OM DU AVBRYTER KONVERTERINGEN." + newline +
      "*****   KOMMENTARER INOM APPLET-TAGGAR IGNORERAS" + newline + newline + newline +
      "Inneh\u00e5ll:" + newline +
      "   1. Nya funktioner" + newline +
      "   2. Korrigeringar" + newline +
      "   3. Om Java(tm) Plug-in HTML Converter" + newline +
      "   4. Konverteringsprocessen" + newline +
      "   5. V\u00e4lja filer i mappar f\u00f6r konvertering" + newline +
      "   6. V\u00e4lja mapp f\u00f6r s\u00e4kerhetskopiering" + newline +
      "   7. Skapa en loggfil" + newline +
      "   8. V\u00e4lja en mall f\u00f6r konvertering" + newline +
      "   9. Konvertering" + newline +
      "  10. Fler konverteringar eller Avsluta" + newline +
      "  11. Mer information om mallar" + newline +
      "  12. K\u00f6ra HTML Converter (Windows & Solaris)" + newline + newline +
      "1)  Nya funktioner:" + newline + newline +     
      "    o Uppdaterade ut\u00f6kade mallar st\u00f6der Netscape 6." + newline +
      "    o Alla mallar uppdaterade s\u00e5 att de nya multiversionsfunktionerna i Java Plug-in st\u00f6ds." + newline +
      "    o F\u00f6rb\u00e4ttrat anv\u00e4ndargr\u00e4nssnitt med Swing 1.1 f\u00f6r i18n-st\u00f6d." + newline +
      "    o Ut\u00f6kad dialogruta f\u00f6r avancerade inst\u00e4llningar som st\u00f6der nya" + newline +
      "      malltaggar f\u00f6r SmartUpdate och MimeType." + newline +
      "    o Ut\u00f6kad HTML Converter som anv\u00e4nds med b\u00e5de Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x , Java Plug-in 1.3.x och Java Plug-in" + newline +
      "      1.4.x." + newline +
      "    o Ut\u00f6kat st\u00f6d f\u00f6r SmartUpdate och MimeType i alla konverterings-" + newline +
      "      mallar." + newline +
      "    o Till\u00e4gg av \"scriptable=false\" i taggen OBJECT/EMBED f\u00f6r alla mallar." + newline + newline + 
      "     Den anv\u00e4nds f\u00f6r att inaktivera typelib-generering n\u00e4r" + newline + newline +
      "    Java Plug-in inte anv\u00e4nds f\u00f6r skript." + newline + newline + newline +
      "2)  Korrigeringar:" + newline + newline +
      "    o F\u00f6rb\u00e4ttrad felhantering n\u00e4r egenskapsfiler inte kan hittas." + newline +
      "    o F\u00f6rb\u00e4ttrad HTML-konvertering s\u00e5 att resulterande EMBED/OBJECT-tagg" + newline +
      "      kan anv\u00e4ndas i AppletViewer i JDK 1.2.x." + newline +
      "    o On\u00f6diga filer som blivit \u00f6ver fr\u00e5n HTML Converter 1.1.x har eliminerats." + newline +
      "    o Genererade EMBED/OBJECT med attributnamnen CODE, CODEBASE osv" + newline +
      "      i st\u00e4llet f\u00f6r JAVA_CODE, JAVA_CODEBASE osv. Detta g\u00f6r" + newline +
      "      att den genererade sidan kan anv\u00e4ndas i JDK 1.2.x AppletViewer." + newline +
      "    o St\u00f6d f\u00f6r MAYSCRIPT-konvertering om den finns i APPLET-" + newline +
      "      taggen." + newline + newline +
      "3) Om Java(tm) Plug-in HTML Converter:" + newline + newline +
      "        Java(tm) Plug-in HTML Converter \u00e4r ett verktyg som du kan anv\u00e4nda f\u00f6r att" + newline +
      "        konvertera HTML-sidor som inneh\u00e5ller appletar till ett format d\u00e4r Java(tm)" + newline +
      "        Plug-in anv\u00e4nds." + newline + newline +       
      "4)  Konverteringsprocessen:" + newline + newline +
      "        Java(tm) Plug-in HTML Converter konverterar alla filer som inneh\u00e5ller" + newline +
      "        appletar till ett format som kan anv\u00e4ndas med Java(tm) Plug-in." + newline + newline +
      "        F\u00f6ljande process anv\u00e4nds f\u00f6r att konvertera en fil:" + newline +
      "        F\u00f6rst flyttas all HTML som inte ing\u00e5r i en applet fr\u00e5n k\u00e4llfilen till en" + newline +
      "        tillf\u00e4llig fil. N\u00e4r en <APPLET-tagg hittas tolkas appleten fram" + newline +
      "        till den f\u00f6rsta </APPLET-taggen (och som inte finns inom citattecken)" + newline +
      "        och d\u00e4refter sl\u00e5s appletdata samman med mallen. (Mer information om mallar" + newline +
      "        finns nedan.) Om inga fel uppst\u00e5r kommer den ursprungliga html-filen att flyttas" + newline +
      "        till mappen f\u00f6r s\u00e4kerhetskopiering, och den tillf\u00e4lliga filen f\u00e5r den" + newline +
      "        ursprungliga filens namn. Dina ursprungliga filer flyttas d\u00e4rf\u00f6r aldrig fr\u00e5n enheten." + newline + newline +
      "        Observera att konverteringsverktyget konverterar alla filer p\u00e5 plats.  " + newline +
      "        N\u00e4r du v\u00e4l har k\u00f6rt konverteringsverktyget kommer filerna d\u00e4rf\u00f6r att vara klara f\u00f6r anv\u00e4ndning med Java(tm) Plug-in." + newline +
       

      "5)  V\u00e4lja filer i mappar f\u00f6r konvertering:" + newline + newline +
      "       Du konverterar alla filer i en mapp genom att skriva in s\u00f6kv\u00e4gen till mappen" + newline +
      "       eller ocks\u00e5 kan du klicka p\u00e5 Bl\u00e4ddra-knappen f\u00f6r att v\u00e4lja mapp i en dialogruta." + newline  +
      "       N\u00e4r du har valt s\u00f6kv\u00e4g kan du ange valfritt antal filspecifikationer i" + newline +
      "       \"Matchande filnamn\". Skriv ett komma mellan varje specifikation. Du kan anv\u00e4nda *" + newline +
      "       som ett jokertecken. Om du anger ett filnamn med jokertecken kommer endast denna enda" + newline +
      "       fil att konverteras. Markera till sist kryssrutan \"Inkludera undermappar\", om du vill" + newline +
      "       att alla filer i underliggande mappar vars filnamn st\u00e4mmer ocks\u00e5 ska konverteras." + newline + newline +      
      "6)  V\u00e4lja mapp f\u00f6r s\u00e4kerhetskopiering:" + newline +

      "       Standards\u00f6kv\u00e4gen till mappen f\u00f6r s\u00e4kerhetskopior \u00e4r k\u00e4lls\u00f6kv\u00e4gen med \"_BAK\" som" + newline +
      "       till\u00e4gg till namnet. Om k\u00e4lls\u00f6kv\u00e4gen t.ex. \u00e4r c:/html/applet.html (konverterar en fil)" + newline +
      "       blir s\u00f6kv\u00e4gen till s\u00e4kerhetskopian c:/html_BAK. Om k\u00e4lls\u00f6kv\u00e4gen" + newline +
      "       \u00e4r c:/html (konverterar alla filer i s\u00f6kv\u00e4gen) blir s\u00f6kv\u00e4gen f\u00f6r s\u00e4kerhetskopior" + newline +
      "       c:/html_BAK. Du kan \u00e4ndra den h\u00e4r s\u00f6kv\u00e4gen genom att sj\u00e4lv skriva in en s\u00f6kv\u00e4g i f\u00e4ltet" + newline +
      "       invid \"S\u00e4kerhetskopiera filer till mappen:\" eller genom att bl\u00e4ddra fram en mapp." + newline + newline +

      "       Unix (Solaris):" + newline +
      "       Standards\u00f6kv\u00e4gen till mappen f\u00f6r s\u00e4kerhetskopior \u00e4r k\u00e4lls\u00f6kv\u00e4gen med \"_BAK\" som" + newline +
      "       till\u00e4gg till namnet. Om k\u00e4lls\u00f6kv\u00e4gen t.ex. \u00e4r /home/user1/html/applet.html (konverterar en fil)" + newline +
      "       blir s\u00f6kv\u00e4gen till s\u00e4kerhetskopian /home/user1/html_BAK. Om k\u00e4lls\u00f6kv\u00e4gen" + newline +
      "       \u00e4r /home/user1/html (konverterar alla filer i s\u00f6kv\u00e4gen) blir s\u00f6kv\u00e4gen f\u00f6r s\u00e4kerhetskopior" + newline +
      "       /home/user1/html_BAK. Du kan \u00e4ndra s\u00f6kv\u00e4gen f\u00f6r s\u00e4kerhetskopior genom att skriva" + newline +
      "       en s\u00f6kv\u00e4g i f\u00e4ltet invid \"S\u00e4kerhetskopiera filer till mappen:\" eller genom att bl\u00e4ddra fram en mapp." + newline + newline +      
      "7)  Skapa en loggfil:" + newline + newline +
      "       Om du vill ha en loggfil ska du markera kryssrutan" + newline +
      "       \"Generera loggfil\". Du kan skriva in s\u00f6kv\u00e4g och filnamn, eller bl\u00e4ddra" + newline +
      "       fram en mapp och d\u00e4refter skriva filnamnet, och v\u00e4lja \u00d6ppna." + newline +
      "       Loggfilen inneh\u00e5ller grundl\u00e4ggande information om konverterings-" + newline +
      "       processen." + newline + newline +        
      "8)  V\u00e4lja en mall f\u00f6r konvertering:" + newline + newline +
      "       Om du inte v\u00e4ljer n\u00e5gon mall anv\u00e4nds en standardmall. Denna mall skapar" + newline +
      "       konverterade html-filer som kan anv\u00e4ndas i IE och Netscape." + newline  +
      "       Om du vill anv\u00e4nda en annan mall kan du v\u00e4lja en fr\u00e5n menyn i" + newline +
      "       huvudsk\u00e4rmen. Om du v\u00e4ljer en annan mall kommer du att kunna v\u00e4lja en fil" + newline +
      "       att anv\u00e4nda som mall." + newline +
      "       Om du v\u00e4ljer en fil, se d\u00e5 till att det \u00e4r en MALL." + newline + newline +
      "9)  Konvertering:" + newline + newline +
      "       Klicka p\u00e5 knappen \"Konvertera...\" f\u00f6r att starta konverteringen. En dialogruta" + newline +
      "       visas, d\u00e4r du kan se vilka filer som bearbetas, antalet filer som bearbetas," + newline +
      "       antal hittade appletar samt antalet hittade fel." + newline + newline +       
      "10) Fler konverteringar eller Avsluta:" + newline + newline +
      "       N\u00e4r konverteringen \u00e4r f\u00e4rdig kommer knappen i bearbetningsdialogrutan att \u00e4ndras" + newline +
      "       fr\u00e5n \"Avbryt\" till \"Klar\". V\u00e4lj \"Klar\" om du vill st\u00e4nga dialogrutan." + newline  +
      "       Du kan sedan v\u00e4lja \"Avsluta\" f\u00f6r att st\u00e4nga Java(tm) Plug-in HTML Converter" + newline +
      "       eller v\u00e4lja en annan upps\u00e4ttning filer att konvertera, och d\u00e4refter v\u00e4lja \"Konvertera...\" igen." + newline + newline +       
      "11) Mer information om mallar:" + newline + newline +
      "       Mallfilen \u00e4r nyckeln till hur appletarna konverteras.  Det \u00e4r en enkel textfil" + newline +
      "       som inneh\u00e5ller taggar som representerar delar av den urpsrungliga appleten." + newline +
      "       Genom att l\u00e4gga till, ta bort eller flytta runt taggarna i en mallfil kan du \u00e4ndra" + newline +
      "       resultatet av konverteringen." + newline + newline +
      "       Taggar som st\u00f6ds:" + newline + newline +
      "        $OriginalApplet$    Denna tagg ers\u00e4tts med den fullst\u00e4ndiga texten" + newline +
      "        fr\u00e5n ursprungsappleten." + newline + newline +
      "        $AppletAttributes$   Denna tagg ers\u00e4tts med samtliga applet-" + newline +
      "        attribut. (code, codebase, width, height osv.)" + newline + newline +
      "        $ObjectAttributes$   Denna tagg ers\u00e4tts med samtliga attribut" + newline +
      "        som kr\u00e4vs f\u00f6r objekttaggen." + newline + newline +
      "        $EmbedAttributes$   Denna tagg ers\u00e4tts med samtliga attribut" + newline +
      "        som kr\u00e4vs f\u00f6r embed-taggen." + newline + newline +
      "        $AppletParams$    Denna tagg ers\u00e4tts med samtliga <param...>-taggar" + newline +
      "        fr\u00e5n appleten" + newline + newline +
      "        $ObjectParams$    Denna tagg ers\u00e4tts med samtliga <param...>-" + newline +
      "        taggar som kr\u00e4vs f\u00f6r objekttaggen." + newline + newline +
      "        $EmbedParams$     Denna tagg ers\u00e4tts med samtliga <param...>-" + newline +
      "        taggar som kr\u00e4vs f\u00f6r embed-taggen i formatet NAME=VALUE" + newline + newline +
      "        $AlternateHTML$  Denna tagg ers\u00e4tts med texten i avsnittet No support" + newline +
      "        for applets fr\u00e5n den ursprungliga appleten" + newline + newline +
      "        $CabFileLocation$   Detta \u00e4r URL:en f\u00f6r den cab-fil som b\u00f6r anv\u00e4ndas" + newline +
      "        i alla mallar som har IE som m\u00e5l." + newline + newline +
      "        $CabFileLocation$   Detta \u00e4r URL:en f\u00f6r den Netscape-plugin som b\u00f6r" + newline +
      "        anv\u00e4ndas i alla mallar som har Netscape som m\u00e5l." + newline + newline +
      "        $SmartUpdate$   Detta \u00e4r URL:en till Netscape SmartUpdate" + newline +
      "        som b\u00f6r anv\u00e4ndas i alla mallar som har Netscape Navigator 4.0 eller senare som m\u00e5l." + newline + newline +
      "        $MimeType$    Detta \u00e4r Java-objektets MIME-typ" + newline + newline +     

      "      default.tpl \u00e4r konverteringsverktygets standardmall. Den konverterade sidan" + newline +
      "      kan anv\u00e4ndas i IE och Navigator i Windows f\u00f6r att anropa Java(TM) Plug-in." + newline +
      "      Det g\u00e5r ocks\u00e5 att anv\u00e4nda den h\u00e4r mallen med Netscape p\u00e5 Unix (Solaris)." + newline + newline +
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
      "      ieonly.tpl - den konverterade sidan kan anv\u00e4ndas f\u00f6r att anropa Java(TM)" + newline +
      "      Plug-in enbart i IE i Windows." + newline + newline +
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
      "      nsonly.tpl - den konverterade sidan kan anv\u00e4ndas f\u00f6r att anropa Java(TM)" + newline +
      "      Plug-in enbart i Navigator i Windows och Solaris." + newline + newline +
      "      <!-- HTML CONVERTER -->" + newline +
      "      <EMBED type=\"$MimeType$\" $EmbedAttributes$" + newline +
      "      $EmbedParams$ scriptable=false" + newline +
      "      pluginspage=\"$NSFileLocation$\"><NOEMBED>" + newline +
      "      $AlternateHTML$" + newline +
      "      </NOEMBED></EMBED>" + newline + newline +
      "      <!--" + newline +
      "      $ORIGINALAPPLET$" + newline +
      "      -->" + newline + newline +       
      "      extend.tpl - den konverterade sidan kan anv\u00e4ndas i alla webbl\u00e4sare och p\u00e5 alla plattformar." + newline +
      "      Om webbl\u00e4saren \u00e4r IE eller Navigator i Windows (Navigator p\u00e5 Solaris) kommer" + newline +
      "      Java(TM) Plug-in att anropas. I annat fall anv\u00e4nds webbl\u00e4sarens standard-JVM." + newline + newline +
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
      "12)  K\u00f6ra HTML Converter:" + newline + newline +
      "      K\u00f6ra GUI-versionen av HTML Converter" + newline + newline +
      "      HTML Converter finns i SDK:n, inte i JRE:n. Du k\u00f6r konverteringsverktyget genom" + newline +
      "      att g\u00e5 till katalogen lib sub som finns i installationskatalogen f\u00f6r SDK:n. Om du till exempel" + newline +
      "      installerade SDK:n i Windows, p\u00e5 C-enheten, flyttar du med cd till" + newline + newline +
      "            C:\\j2sdk1.4.2\\lib\\" + newline + newline +
      "      Du hittar konverteraren (htmlconverter.jar) i denna katalog." + newline + newline +
      "      Du startar verktyget genom att skriva:" + newline + newline +
      "            C:\\j2sdk1.4.2\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      Du startar konverteraren i UNIX/Linux p\u00e5 ungef\u00e4r samma s\u00e4tt." + newline +
      "      Nedan kan du hitta n\u00e5gra alternativa metoder f\u00f6r att starta verktyget" + newline + newline +
      "      I Windows" + newline +
      "      Starta Converter via Utforskaren." + newline +
      "      Anv\u00e4nd Utforskaren f\u00f6r att g\u00e5 till f\u00f6ljande katalog:" + newline + newline +
      "      C:\\j2sdk1.4.2\\bin" + newline + newline +
      "      Dubbelklicka p\u00e5 programmet HtmlConverter." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      K\u00f6r f\u00f6ljande kommandon" + newline + newline +
      "      cd /j2sdk1.4.2/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +             
      "      K\u00f6ra Converter fr\u00e5n kommandoraden:" + newline + newline +
      "      Format:" + newline + newline +
      "      java -jar htmlconverter.jar [-alternativ1 v\u00e4rde1 [-alternativ2 v\u00e4rde2" + newline +
      "      [...]]] [-simulate] [filspec]" + newline + newline +
      "      filspec:  blankstegsavgr\u00e4nsad lista med filspecifikationer, * jokertecken. " + newline +
      "      (*.html *.htm)" + newline + newline +
      "      Alternativ:" + newline + newline +
      "       source:    S\u00f6kv\u00e4g till filer.  (c:\\htmldocs in Windows," + newline +
      "                  /home/user1/htmldocs i Unix) Standard: <userdir>" + newline +
      "                  Om s\u00f6kv\u00e4gen \u00e4r relativ antas den vara relativ den katalog" + newline +
      "                  som HTMLConverter startades fr\u00e5n." + newline + newline +
      "       backup:    S\u00f6kv\u00e4g till s\u00e4kerhetskopierade filer. Standard:" + newline +
      "                  <userdir>/<k\u00e4lla>_bak" + newline +
      "                  Om s\u00f6kv\u00e4gen \u00e4r relativ antas den vara relativ den katalog" + newline +
      "                  som HTMLConverter startades fr\u00e5n." + newline + newline +
      "       subdirs:   Om filer i underkataloger ska bearbetas. " + newline +
      "                  Standard:  FALSE" + newline + newline +
      "       template:  Namn p\u00e5 mallfilen. Standard:  default.tpl-Standard " + newline +
      "                  (IE & Navigator) endast f\u00f6r Windows & Solaris. ANV\u00c4ND STANDARD OM DU \u00c4R OS\u00c4KER." + newline + newline +
      "       log:       S\u00f6kv\u00e4g och filnamn f\u00f6r skrivningslogg. (Standard <userdir>/convert.log)" + newline + newline +
      "       progress:  Visar standardiserat f\u00f6rlopp under konverteringen. " + newline +
      "                  Standard: false" + newline + newline +
      "       simulate:  Visar uppgifterna om konverteringen utan att utf\u00f6ra konverteringen." + newline +
      "                  ANV\u00c4ND DET H\u00c4R ALTERNATIVET OM DU \u00c4R OS\u00c4KER P\u00c5 KONVERTERINGENS EFFEKTER." + newline +
      "                  DU F\u00c5R EN LISTA \u00d6VER UPPGIFTER SOM G\u00c4LLER F\u00d6R" + newline +
      "                  DENNA KONVERTERING." + newline + newline +
      "      Om du endast anger \"java -jar htmlconverter.jar -gui\" (endast -gui" + newline +
      "      alternativet utan filspec) kommer GUI-versionen av Converter att startas." + newline  +
      "      I annat fall anv\u00e4nds inte GUI." + newline + newline +
      "      Mer information finns p\u00e5 f\u00f6ljande adress:" + newline + newline +
      "      http://java.sun.com/j2se/1.4.2/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
} 

 


