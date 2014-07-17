/*
 * @(#)ConverterHelp_it.java	1.2 03/04/24
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Italian version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_it extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");;

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "File Readme del convertitore HTML di Java(tm) Plug-in" + newline + newline +
      "Versione:  1.4.2" + newline + newline + newline +
      "***** ESEGUIRE UN BACKUP DI TUTTI I FILE PRIMA DI CONVERTIRLI CON QUESTO STRUMENTO" + newline +
      "***** L'ANNULLAMENTO DI UNA CONVERSIONE NON PREVEDE IL ROLLBACK DELLE MODIFICHE." + newline +
      "***** I COMMENTI NEL TAG APPLET VENGONO IGNORATI" + newline + newline + newline +
      "Sommario:" + newline +
      "   1.  Nuove funzioni" + newline +
      "   2.  Correzione degli errori" + newline +
      "   3.  Informazioni sul convertitore HTML di Java(tm) Plug-in" + newline +
      "   4.  Processo di conversione" + newline +
      "   5.  Scelta dei file nelle cartelle da convertire" + newline +
      "   6.  Scelta della cartella di backup" + newline +
      "   7.  Generazione di un file di log" + newline +
      "   8.  Scelta di un modello di conversione" + newline +
      "   9.  Conversione" + newline +
      "  10.  Esecuzione di altre conversioni o chiusura" + newline +
      "  11.  Dettagli sui modelli" + newline +
      "  12.  Esecuzione del convertitore HTML (Windows e Solaris)" + newline + newline +
      "1)  Nuove funzioni:" + newline + newline +     
      "    o Aggiornamento dei modelli estesi per il supporto di Netscape 6." + newline +
      "    o Aggiornamento di tutti i modelli per il supporto delle nuove funzioni multiversioni in Java Plug-in." + newline +
      "    o Miglioramento dell'interfaccia utente con supporto di Swing 1.1 per i18n." + newline +
      "    o Miglioramento della finestra di dialogo delle opzioni avanzate per il supporto " + newline +
      "      dei nuovi tag di modello SmartUpdate e MimeType." + newline +
      "    o Miglioramento del convertitore HTML da usare con Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x , Java Plug-in 1.3.x e Java Plug-in" + newline +
      "      1.4.x." + newline +
      "    o Miglioramento del supporto SmartUpdate e MimeType in tutti i modelli" + newline +
      "      di conversione." + newline +
      "    o Aggiunta di \"scriptable=false\" al tag OBJECT/EMBED in tutti i modelli." + newline + newline + 
      "     Tale istruzione viene utilizzata per disattivare la generazione typelib quando" + newline + newline +
      "    Java Plug-in non viene utilizzato per lo scripting." + newline + newline + newline +
      "2)  Correzione degli errori:" + newline + newline +
      "    o Miglioramento della gestione degli errori quando non sono disponibili i file delle propriet\u00e0." + newline +
      "    o Miglioramento della conversione HTML in modo che il tag di risultato EMBED/OBJECT " + newline +
      "      possa essere utilizzato in AppletViewer di JDK 1.2.x." + newline +
      "    o Eliminazione dei file non necessari appartenenti al convertitore HTML 1.1.x." + newline +
      "    o Generazione di EMBED/OBJECT con i nomi di attributi CODE, CODEBASE e cos\u00ec via" + newline +
      "      anzich\u00e9 JAVA_CODE, JAVA_CODEBASE e cos\u00ec via. Ci\u00f2 permette di" + newline +
      "      utilizzare la pagina generata in AppletViewer di JDK 1.2.x." + newline +
      "    o Supporto della conversione MAYSCRIPT se inclusa nel tag" + newline +
      "      APPLET." + newline + newline +
      "3)  Informazioni sul convertitore HTML di Java(tm) Plug-in:" + newline + newline +
      "        il convertitore HTML Java(tm) Plug-in \u00e8 un'utility che consente di convertire" + newline +
      "        qualsiasi pagina HTML contenente applet in un formato che utilizza Java(tm)" + newline +
      "        Plug-in." + newline + newline +       
      "4)  Processo di conversione:" + newline + newline +
      "        Il convertitore HTML di Java(tm) Plug-in esegue la conversione di qualsiasi file contenente" + newline +
      "        applet in un formato che possa essere utilizzato da Java(tm) Plug-in." + newline + newline +
      "        Il processo di conversione di ciascun file viene effettuato come segue:" + newline +
      "        Innanzitutto, il codice HTML che non fa parte di un'applet viene trasferito dal file" + newline +
      "        di origine a un file temporaneo.  Quando viene raggiunto un tag <APPLET, il convertitore" + newline +
      "        analizza l'applet fino al primo tag </APPLET (non racchiuso tra virgolette)" + newline +
      "        ed esegue la fusione dei dati dell'applet con il modello. Vedere i Dettagli sui modelli di seguito." + newline +
      "        Se questa operazione viene completata senza errori, il file html originale viene" + newline +
      "        spostato nella cartella di backup e il file temporaneo viene rinominato in base al " + newline +
      "        nome del file originale.  Pertanto, i file originali non vengono mai rimossi dal disco." + newline + newline +
      "        Tenere presente che la conversione viene eseguita direttamente.  Pertanto," + newline +
      "        una volta eseguito il convertitore, i file saranno impostati per l'uso di Java(tm) Plug-in." + newline +
       

      "5)  Scelta dei file nelle cartelle da convertire:" + newline + newline +
      "       Per convertire tutti i file di una cartella, \u00e8 possibile specificare il percorso della cartella" + newline +
      "       o scegliere il pulsante Sfoglia per selezionare una cartella da una finestra di dialogo." + newline  +
      "       Una volta scelto un percorso, \u00e8 possibile specificare qualsiasi numero di identificatori di file nel campo" + newline +
      "       \"Nomi file corrispondenti\".  Ciascun identificatore deve essere separato da una virgola.  \u00c8 possibile utilizzare * come" + newline +
      "       carattere jolly.  Se si specifica un nomefile con un carattere jolly, verr\u00e0 convertito solo il file" + newline +
      "       corrispondente. Infine, selezionare la casella di controllo \"Includi sottocartelle\", se si desidera" + newline +
      "       convertire tutti i file presenti nelle cartelle nidificate e corrispondenti al nome del file da convertire." + newline + newline +      
      "6)  Scelta della cartella di backup:" + newline +

      "       Il percorso predefinito della cartella di backup corrisponde al nome del percorso di origine a cui viene aggiunto" + newline +
      "       \"_BAK\".  Ad esempio, se il percorso di origine \u00e8 c:/html/applet.html (per la conversione di un file)" + newline +
      "       il percorso di backup sar\u00e0 c:/html_BAK.  Se il percorso di origine" + newline +
      "       \u00e8 c:/html (per la conversione di tutti i file nel percorso), il percorso di backup sar\u00e0" + newline +
      "       c:/html_BAK. \u00c8 possibile modificare il percorso di backup digitando un percorso nel campo" + newline +
      "       accanto a \"Backup dei file nella cartella:\" oppure selezionando una cartella." + newline + newline +

      "       Unix(Solaris):" + newline +
      "       Il percorso predefinito della cartella di backup corrisponde al nome del percorso di origine a cui viene aggiunto" + newline +
      "       \"_BAK\".  Ad esempio, se il percorso di origine \u00e8 /home/utente1/html/applet.html (per la conversione di un file)" + newline +
      "       il percorso di backup sar\u00e0 /home/utente1/html_BAK. Se il percorso di origine" + newline +
      "       \u00e8 /home/utente1/html (per la conversione di tutti i file nel percorso), il percorso di backup sar\u00e0" + newline +
      "       /home/utente1/html_BAK. \u00c8 possibile modificare il percorso di backup digitando" + newline +
      "       un percorso nel campo accanto a \"Backup dei file nella cartella:\" oppure selezionando una cartella." + newline + newline +      
      "7)  Generazione di un file di log:" + newline + newline +
      "       Se si desidera che venga generato un file di log, selezionare la casella di controllo" + newline +
      "       \"Genera file di log\". \u00c8 possibile immettere il percorso e il nomefile oppure selezionare" + newline +
      "       una cartella, quindi digitare il nomefile e selezionare Apri." + newline +
      "       Il file di log contiene informazioni di base relative al processo" + newline +
      "       di conversione." + newline + newline +        
      "8)  Scelta di un modello di conversione:" + newline + newline +
      "       Se non si sceglie alcun modello, viene utilizzato quello predefinito.  Tale modello genera" + newline +
      "       file HTML convertiti che funzionano sia con Internet Explorer che con Netscape." + newline  +
      "       Se si desidera utilizzare un modello diverso, sceglierlo dal menu" + newline +
      "       nella schermata principale.  Se si sceglie Altro, sar\u00e0 possibile scegliere un file da" + newline +
      "       utilizzare come modello." + newline +
      "       Se si sceglie un file, ACCERTARSI CHE SI TRATTI DI UN MODELLO." + newline + newline +
      "9)  Conversione:" + newline + newline +
      "       Fare clic sul pulsante \"Converti...\" per iniziare il processo di conversione.  Viene visualizzata" + newline +
      "       una finestra di dialogo in cui sono riportati tutti i file in fase di elaborazione, il numero di file da elaborare," + newline +
      "       il numero di applet trovate e il numero di errori riscontrati." + newline + newline +       
      "10) Esecuzione di altre conversioni o chiusura:" + newline + newline +
      "       Al termine della conversione, il pulsante nella finestra di dialogo di elaborazione cambia" + newline +
      "       da \"Annulla\" a \"Chiudi\".  \u00c8 possibile scegliere \"Chiudi\" per chiudere la finestra di dialogo." + newline  +
      "       A questo punto, scegliere \"Esci\" per chiudere il convertitore HTML di Java(tm) Plug-in" + newline +
      "       oppure selezionare un altro gruppo di file da convertire e scegliere nuovamente \"Converti...\"." + newline + newline +       
      "11)  Dettagli sui modelli:" + newline + newline +
      "       Il file del modello \u00e8 alla base del processo di conversione delle applet.  Si tratta di un semplice file di" + newline +
      "       testo contenente tag che rappresentano parti dell'applet originale." + newline +
      "       Aggiungendo, rimuovendo o spostando i tag in un file del modello, \u00e8 possibile modificare l'output" + newline +
      "       del file convertito." + newline + newline +
      "       Tag supportati:" + newline + newline +
      "        $OriginalApplet$    Questo tag viene sostituito dal testo completo" + newline +
      "        dell'applet originale." + newline + newline +
      "        $AppletAttributes$   Questo tag viene sostituito da tutti gli" + newline +
      "        attributi delle applet (code, codebase, width, height e cos\u00ec via)." + newline + newline +
      "        $ObjectAttributes$   Questo tag viene sostituito da tutti gli" + newline +
      "        attributi richiesti dal tag object." + newline + newline +
      "        $EmbedAttributes$   Questo tag viene sostituito da tutti gli" + newline +
      "        attributi richiesti dal tag embed." + newline + newline +
      "        $AppletParams$    Questo tag viene sostituito da tutti i tag" + newline +
      "        <param ...> dell'applet." + newline + newline +
      "        $ObjectParams$    Questo tag viene sostituito da tutti i tag <param...>" + newline +
      "        richiesti dal tag object." + newline + newline +
      "        $EmbedParams$     Questo tag viene sostituito da tutti i tag <param...>" + newline +
      "        richiesti dal tag embed sotto forma di NAME=VALUE" + newline + newline +
      "        $AlternateHTML$  Questo tag viene sostituito dal testo presente nell'area" + newline +
      "        di nessun supporto per le applet dell'applet originale" + newline + newline +
      "        $CabFileLocation$   Questo \u00e8 l'URL del file cab che deve essere utilizzato in" + newline +
      "        ciascun modello che ha come destinazione Internet Explorer." + newline + newline +
      "        $NSFileLocation$    Questo \u00e8 l'URL del plugin Netscape che deve essere utilizzato in" + newline +
      "        ciascun modello che ha come destinazione Netscape." + newline + newline +
      "        $SmartUpdate$   Questo \u00e8 l'URL dello SmartUpdate di Netscape" + newline +
      "        da utilizzare in ciascun modello che ha come destinazione Netscape Navigator 4.0 o versione successiva." + newline + newline +
      "        $MimeType$    Questo \u00e8 il tipo di MIME dell'oggetto Java" + newline + newline +     

      "      default.tpl \u00e8 il modello predefinito per il convertitore. \u00c8 possibile utilizzare la pagina convertita in" + newline +
      "      Internet Explorer e Navigator su Windows per richiamare Java(TM) Plug-in." + newline +
      "      \u00c8 possibile utilizzare questo modello anche con Netscape su Unix(Solaris)" + newline + newline +
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
      "      ieonly.tpl -- \u00c8 possibile utilizzare la pagina convertita per richiamare Java(TM)" + newline +
      "      Plug-in solo in Internet Explorer su Windows." + newline + newline +
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
      "      nsonly.tpl -- \u00c8 possibile utilizzare la pagina convertita per richiamare Java(TM)" + newline +
      "      Plug-in solo in Navigator su Windows e Solaris." + newline + newline +
      "      <!-- HTML CONVERTER -->" + newline +
      "      <EMBED type=\"$MimeType$\" $EmbedAttributes$" + newline +
      "      $EmbedParams$ scriptable=false" + newline +
      "      pluginspage=\"$NSFileLocation$\"><NOEMBED>" + newline +
      "      $AlternateHTML$" + newline +
      "      </NOEMBED></EMBED>" + newline + newline +
      "      <!--" + newline +
      "      $ORIGINALAPPLET$" + newline +
      "      -->" + newline + newline +       
      "      extend.tpl -- \u00c8 possibile utilizzare la pagina convertita in qualsiasi browser e piattaforma." + newline +
      "      Se il browser \u00e8 Internet Explorer o Navigator su Windows (Navigator su Solaris), verr\u00e0 richiamato Java(TM)" + newline +
      "      Plug-in. Altrimenti, verr\u00e0 utilizzata la JVM predefinita del browser." + newline + newline +
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
      "12)  Esecuzione del convertitore HTML:" + newline + newline +
      "      Esecuzione della versione GUI del convertitore HTML" + newline + newline +
      "      Il convertitore HTML \u00e8 presente in SDK, non in JRE. Per eseguire il convertitore, passare alla" + newline +
      "      sottodirectory lib della directory di installazione SDK. Ad esempio," + newline +
      "      se \u00e8 stato installato SDK sull'unit\u00e0 C di Windows, passare alla directory" + newline + newline +
      "            C:\\j2sdk1.4.2\\lib\\" + newline + newline +
      "      Il convertitore (htmlconverter.jar) si trova in tale directory." + newline + newline +
      "      Per avviare il convertitore, digitare:" + newline + newline +
      "            C:\\j2sdk1.4.2\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      L'avvio del convertitore su UNIX/Linux \u00e8 analogo a quanto descritto sopra." + newline +
      "      Di seguito sono riportati alcuni metodi alternativi per l'avvio del convertitore." + newline + newline +
      "      Windows" + newline +
      "      Avvio del convertitore utilizzando Gestione/Esplora risorse." + newline +
      "      Utilizzare Gestione/Esplora risorse per passare alla seguente directory:" + newline + newline +
      "      C:\\j2sdk1.4.2\\bin" + newline + newline +
      "      Fare doppio clic sull'applicazione HtmlConverter." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      Eseguire i seguenti comandi:" + newline + newline +
      "      cd /j2sdk1.4.2/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +             
      "      Esecuzione del convertitore dalla riga di comando:" + newline + newline +
      "      Formato:" + newline + newline +
      "      java -jar htmlconverter.jar [-opzioni1 valore1 [-opzione2 valore2" + newline +
      "      [...]]] [-simulate] [spec_file]" + newline + newline +
      "      spec_file:  elenco delimitato da spazi di specifiche del file, carattere jolly *; " + newline +
      "      (*.html *.htm)" + newline + newline +
      "      Opzioni:" + newline + newline +
      "       source:    Percorso dei file.  (c:\\htmldocs in Windows," + newline +
      "                  /home/utente1/htmldocs in Unix) Valore predefinito: <dir_utente>" + newline +
      "                  Nel caso di un percorso relativo, si suppone che il percorso sia relativo alla" + newline +
      "                  directory da cui \u00e8 stato avviato HTMLConverter." + newline + newline +
      "       backup:    Percorso per la scrittura dei file di backup.  Valore predefinito:" + newline +
      "                  <dir_utente>/<origine>_bak" + newline +
      "                  Nel caso di un percorso relativo, si suppone che il percorso sia relativo alla" + newline +
      "                  directory da cui \u00e8 stato avviato HTMLConverter." + newline + newline +
      "       subdirs:   Indica se i file nelle sottodirectory devono essere elaborati. " + newline +
      "                  Valore predefinito:  FALSE" + newline + newline +
      "       template:  Nome del file del modello.  Valore predefinito:  default.tpl-Standard " + newline +
      "                  (Internet Explorer e Navigator) solo per Windows e Solaris. IN CASO DI INCERTEZZA, UTILIZZARE IL VALORE PREDEFINITO." + newline + newline +
      "       log:       Percorso e nome file in cui scrivere il log  (valore predefinito <dir_utente>/convert.log)" + newline + newline +
      "       progress:  Visualizza l'avanzamento dell'output standard della conversione. " + newline +
      "                  Valore predefinito: false" + newline + newline +
      "       simulate:  Visualizza le specifiche della conversione senza effettivamente eseguirla." + newline +
      "                  UTILIZZARE QUESTA OPZIONE SE NON SI \u00c8 CERTI DI ESEGUIRE LA CONVERSIONE." + newline +
      "                  VERR\u00c0 VISUALIZZATO UN ELENCO DI DETTAGLI SPECIFICI PER LA" + newline +
      "                  CONVERSIONE." + newline + newline +
      "      Se viene specificato solo \"java -jar htmlconverter.jar -gui\" (solo l'opzione -gui" + newline +
      "      senza spec_file), verr\u00e0 avviata la versione GUI del convertitore." + newline  +
      "      Altrimenti, la versione GUI non verr\u00e0 avviata." + newline + newline +
      "      Per ulteriori informazioni, accedere all'URL seguente:" + newline + newline +
      "      http://java.sun.com/j2se/1.4.2/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
} 

 


