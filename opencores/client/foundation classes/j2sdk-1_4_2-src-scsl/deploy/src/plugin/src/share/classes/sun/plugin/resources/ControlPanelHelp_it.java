/*
 * @(#)ControlPanelHelp_it.java	1.2 03/03/20
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

public class ControlPanelHelp_it extends ListResourceBundle {

    private static String newline = "\n";

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "help.file", newline +
                   "Guida di Pannello di controllo di Java Plug-in" + newline + newline + newline +
                   "Nella guida vengono trattati i seguenti argomenti:" + newline +
                   "      Panoramica" + newline + 
                   "      Salvataggio delle opzioni" + newline + 
                   "      Impostazione delle opzioni del Pannello di controllo" + newline + 
                   "      Pannello Base" + newline + 
                   "      Pannello Avanzate" + newline +
                   "      Pannello Browser" + newline + 
                   "      Panello Proxy" + newline + 
                   "      Pannello Cache" + newline + 
                   "      Pannello Certificati" + newline + 
                   "      Pannello aggiornamento" + newline + newline + 
                   "Panoramica" + newline + newline +
                   "Il Pannello di controllo di Java Plug-in consente di modificare le impostazioni predefinite caricate all'avvio da Java Plug-in." + newline +
                   "Tutti gli applet in esecuzione in una istanza attiva di Java Plug-in utilizzeranno queste impostazioni." + newline +
                   "La Guida dello sviluppatore di Java Plug-in, a cui si fa riferimento nel presente documento, \u00e8 disponibile all'indirizzo (URL soggetto a modifica)." + newline +
                   "http://java.sun.com/j2se/1.4/docs/guide/plugin/developer_guide/contents.html" + newline + newline +
                   "Salvataggio delle opzioni" + newline + newline +
                   "Una volta completate le modifiche nelle opzioni del Pannello di controllo, fare clic su Applica per salvare le modifiche." + newline +
                   "Fare clic su Ripristina per annullare le modifiche e ricaricare gli ultimi valori inseriti e in uso." + newline +
                   "Tenere presente che non sono gli stessi dei valori predefiniti impostati in origine per Java Plug-in al momento dell'installazione." + newline + newline +
                   "Impostazione delle opzioni del Pannello di controllo" + newline + newline +
                   "In Pannello di controllo di Java Plug-in sono disponibili sei pannelli nei quali \u00e8 possibile impostare le varie opzioni." + newline +
                   "I pannelli sono: " + newline +
                   "      Base" + newline + 
                   "      Avanzate" + newline + 
                   "      Browser" + newline + 
                   "      Proxy" + newline + 
                   "      Cache" + newline + 
                   "      Certificati" + newline + 
                   "      Aggiornamento" + newline + newline +
                   "Di seguito \u00e8 riportata la descrizione di ciascun pannello." + newline + newline + newline +
                   " Base" + newline +
                   "Mostra console" + newline + newline + 
                   "      Mostra la console Java mentre sono in esecuzione gli applet. Nella console vengono visualizzati i messaggi stampati da System.out e System.err." + newline +
                   "      Si rivela utile per problemi di debugging." + newline + newline +
                   "Nascondi console" + newline + newline + 
                   "      La console Java \u00e8 in esecuzione ma \u00e8 nascosta. Questa \u00e8 l'impostazione predefinita (selezionata)." + newline + newline +
                   "Non avviare console" + newline + newline + 
                   "      La console Java non viene avviata." + newline + newline + 
                   "Mostra finestra di dialogo Eccezioni" + newline + newline + 
                   "      Mostra la finestra di dialogo delle eccezioni se si verificano delle eccezioni. L'impostazione predefinita \u00e8 quella di non mostrare la finestra di dialogo delle eccezioni (deselezionata)." + newline + newline +
                   "Mostra Java nella barra di sistema (solo per Windows)" + newline + newline + 
                   "      Quando questa opzione \u00e8 abilitata, il logo Java a forma di tazzina di caff\u00e8 viene visualizzato nella barra di sistema quando viene avviato Java Plug-in" + newline +
                   "      e viene rimosso dalla barra quando Java Plug-in viene chiuso." + newline +
                   "      Il logo Java a forma di tazzina di caff\u00e8 indica all'utente che \u00e8 in esecuzione un JVM e fornisce informazioni sulla" + newline +
                   "      release Java e il controllo sulla console Java Console." + newline +
                   "      Questa opzione \u00e8 abilitata per impostazione predefinita (selezionata)." + newline + newline +
                   "      Funzionalit\u00e0 della barra di sistema Java:" + newline + newline + 
                   "      Quando il mouse punta al logo Java a forma di tazzina di caff\u00e8, viene visualizzato il testo \"Java\"." + newline + newline +

                   "      Quando si fa doppio clic sull'icona di Java nella barra di sistema, viene visualizzata la finestra della console Java." + newline + newline + 
                   "      Quando si fa clic con il pulsante destro del mouse sull'icona Java nella barra di sistema, viene visualizzato un menu a discesa contenente le seguenti opzioni:" + newline + newline +
                   "            Apri/chiudi console" + newline + 
                   "            Informazioni su Java" + newline + 
                   "            Disabilita" + newline + 
                   "            Esci" + newline + newline + 
                   "      Apri/chiudi console consente di aprire e chiudere la finestra della console Java. La voce di menu \u00e8 Apri console quando la console Java \u00e8 nascosta" + newline +
                   "      e Chiudi console quando la console Java \u00e8 attiva." + newline + newline +
                   "      Informazioni su Java consente di aprire una finestra di informazioni per Java 2 Standard Edition." + newline + newline +
                   "      Disabilita consente di disabilitare e rimuovere l'icona di Java dalla barra di sistema per la sessione corrente e per quelle successive. Quando Java Plug-in" + newline +
                   "      viene riavviato, l'icona di Java non viene visualizzata nella barra di sistema." + newline +
                   "      Per informazioni su come visualizzare l'icona di Java nella barra di sistema dopo che \u00e8 stata disabilitata, vedere le note riportate di seguito." + newline + newline +
                   "      Esci consente di rimuovere l'icona di Java dalla barra di sistema solo per la sessione corrente. Quando Java Plug-in viene riavviato," + newline +
                   "      l'icona di Java viene visualizzata nuovamente nella barra di sistema." + newline + newline + newline + 
                   "                Note" + newline + newline +
                   "                1. Se l'opzione \"Mostra Java nella barra di sistema\" \u00e8 selezionata, l'icona di Java viene visualizzata nella barra di sistema anche se l'opzione \"Non avviare" + newline +
                   "                console\" \u00e8 selezionata." + newline + newline +
                   "                2. Per abilitare l'icona di Java dopo averla disabilitata, avviare il Pannello di controllo di Java Plug-in, selezionare \"Mostra Java" + newline +
                   "                nella barra di sistema\" e premere \"Applica\"." + newline + newline +
                   "                3. Se altri JVM sono gi\u00e0 in esecuzione e nella barra di sistema sono state aggiunte altre icone di Java, la modifica" + newline +
                   "                dell'impostazione nel Pannello di controllo di Java Plug-in non incide su tali icone." + newline +
                   "                Questa impostazione incide sul funzionamento dell'icona di Java solo quando viene avviato JVM in un secondo momento." + newline + newline + newline +
                   " Avanzate" + newline + newline + newline +
                   "Java Run Time Environment" + newline + newline +
                   "      Consente l'esecuzione di Java Plug-in con qualunque ambiente runtime Java 2 (JRE) o SDK, Standard Edition v 1.3 o 1.4 installato sul computer." + newline +
                   "      Java Plug-in 1.3/1.4 \u00e8 fornito con un ambiente runtime Java (JRE) predefinito." + newline +
                   "      Tuttavia \u00e8 possibile disattivare il JRE predefinito e utilizzarne una versione meno recente o pi\u00f9 recente. Le versioni Java 2 SDK o JRE installate sul computer" + newline +
                   "      vengono rilevate automaticamente dal Pannello di controllo. Le versioni utilizzabili sono elencate" + newline +
                   "      nella casella di riepilogo." + newline +
                   "      La prima voce nell'elenco \u00e8 sempre il JRE predefinito; l'ultima voce \u00e8 sempre Altro. Se si sceglie Altro, occorre" + newline +
                   "      specificare il percorso di Java 2 JRE o SDK, Standard Edition v 1.3/1.4" + newline + newline + 
                   "                Nota" + newline + newline +
                   "                Opzione consigliata soltanto agli utenti esperti. Si sconsiglia di modificare la JRE predefinita." + newline + newline + newline +
                   "Parametri del runtime Java" + newline + newline + 
                   "      Se si specificano delle opzioni personalizzate \u00e8 possibile sostituire i parametri di avvio predefiniti di Java Plug-in. La sintassi \u00e8 identica a quella utilizzata" + newline +
                   "      con i parametri nella chiamata da linea di comando Java. Per un elenco completo delle opzioni della linea di comando" + newline +
                   "      consultare la documentazione di Java 2 Standard Edition (J2SE)." + newline +
                   "      Il seguente URL \u00e8 soggetto a modifiche:" + newline + newline +
                   "            http://java.sun.com/j2se/1.4/docs/tooldocs/<piattaforma>/java.html" + newline + newline + 
                   "            dove <piattaforma> \u00e8 uno dei sistemi operativi: solaris, linux, win32." + newline + newline + 
                   "      Di seguito sono riportati alcuni esempi di parametri del runtime Java." + newline + newline +
                   "      Abilitazione e disabilitazione del supporto dell'asserzione" + newline + newline +
                   "            Per abilitare il supporto dell'asserzione, occorre specificare le seguenti propriet\u00e0 di sistema in Parametri del runtime Java:" + newline + newline +
                   "                  -D[ enableassertions | ea ][:<nome pacchetto>\"...\" | : <nome classe> ]" + newline + newline +
                   "            Per disabilitare l'asserzione in Java Plug-in, specificare quanto segue in Parametri del runtime Java:" + newline + newline + 
                   "                  -D[ disableassertions | da ][:<nome pacchetto>\"...\" | : <nome classe> ]" + newline + newline + 
                   "            Per ulteriori informazioni sull'abilitazione/disabilitazione delle asserzioni vedere Funzione asserzione." + newline +
                   "            http://java.sun.com/j2se/1.4/docs/guide/lang/assert.html (URL soggetto a modifica)." + newline + newline +
                   "            Per impostazione predefinita l'asserzione \u00e8 disabitata nel codice Java Plug-in. Poich\u00e9 l'effetto dell'asserzione \u00e8 determinato durante l'avvio di Java Plug-in" + newline +
                   "            se si modifica l'impostazione dell'asserzione in Pannello di controllo di Java Plug-in occorrer\u00e0 riavviare il browser" + newline +
                   "            affinch\u00e9 le nuove impostazioni abbiano effetto." + newline + newline + 
                   "            Poich\u00e9 il codice Java in Java Plug-in contiene un'asserzione incorporata, \u00e8 possibile abilitare l'asserzione" + newline +
                   "            nel codice Java Plug-in nel seguente modo:" + newline + newline +
                   "                  -D[ enableassertions | ea ]:sun.plugin" + newline + newline + 
                   "      Supporto del tracciamento e del log" + newline + newline +
                   "            Il tracciamento \u00e8 una funzionalit\u00e0 che consente di reindirizzare un output qualunque nella console Java al file di traccia (.plugin<versione>.trace)." + newline + newline +
                   "                  -Djavaplugin.trace=true" + newline +
                   "                  -Djavaplugin.trace.option=basic|net|security|ext|liveconnect" + newline + newline +
                   "            Se non si desidera utilizzare il nome del file di traccia predefinito:" + newline + newline +
                   "                  -Djavaplugin.trace.filename=<nomedelfileditraccia>" + newline + newline + 
                   "            Simile al tracciamento, il log \u00e8 un'altra funzionalit\u00e0 che consente di reindirizzare un output qualunque nella console Java al file di log (.plugin<versione>.log)" + newline +
                   "            utilizzando Java Logging API." + newline +
                   "            L'esecuzione del log pu\u00f2 essere attivata abilitando la propriet\u00e0 javaplugin.logging." + newline + newline +
                   "                  -Djavaplugin.logging=true" + newline + newline +
                   "            Se non si desidera utilizzare il nome del file di log predefinito, immettere:" + newline + newline +
                   "                  -Djavaplugin.log.filename=<nomedelfiledilog>" + newline + newline + 
                   "            Inoltre, se per ogni sessione non si desidera sovrascrivere i file di log e di traccia, \u00e8 possibile impostare la propriet\u00e0:" + newline + newline + 
                   "                  -Djavaplugin.outputfiles.overwrite=false." + newline + newline + 
                   "            Se la propriet\u00e0 \u00e8 impostata su false, i file di log e di traccia avranno un nome univoco per ciascuna sessione. Se si utilizzano" + newline +
                   "            i nomi dei file di log e di traccia predefiniti, i nomi assegnati ai file saranno creati nel seguente modo" + newline + newline +
                   "                  .plugin<nomeutente><data cancelletto codice>.trace" + newline +
                   "                  .plugin<nomeutente><data cancelletto codice>.log" + newline + newline +
                   "            Il tracciamento e il log impostati mediante il Pannello di controllo saranno effettivi all'avvio del Plug-in, ma le modifiche" + newline +
                   "            apportate tramite il Pannello di controllo mentre un Plug-in \u00e8 in esecuzione non avranno effetto sino al successivo riavvio." + newline + newline + 
                   "            Per ulteriori informazioni su tracciamento e log, vedere Tracciamento e log nella Guida dello sviluppatore di Java Plug-in." + newline + newline +
                   "      Debugging degli applet in Java Plug-in" + newline + newline +
                   "            Le seguenti azioni sono utilizzate durante il debugging degli applet in Java Plug-in." + newline +
                   "            Per ulteriori informazioni su questo argomento vedere Supporto del debugging in Guida dello sviluppatore di Java Plug-in." + newline + newline +
                   "                  -Djava.compiler=NONE" + newline +
                   "                  -Xnoagent" + newline +
                   "                  -Xdebug" + newline + 
                   "                  -Xrunjdwp:transport=dt_shmem,address=<indirizzo-collegamento>,server=y,suspend=n" + newline + newline +
                   "            <indirizzo-collegamento> pu\u00f2 essere una stringa qualsiasi (per esempio: 2502) che verr\u00e0 poi utilizzata da Java Debugger (jdb)" + newline +
                   "            per collegarsi alla JVM." + newline + newline + 
                   "      Timeout della connessione predefinito" + newline + newline +
                   "            Quando si esegue la connessione al server tramite un applet e il server non risponde correttamente, l'applet pu\u00f2" + newline +
                   "            bloccarsi e bloccare anche il browser, perch\u00e9 non vi \u00e8 nessun timeout della connessione predefinito" + newline +
                   "            (per impostazione predefinita \u00e8 impostato su -1, il che significa che non \u00e8 impostato nessun timeout)." + newline + newline +
                   "            Per ovviare al problema, in Java Plug-in \u00e8 stato aggiunto un valore di timeout di rete predefinito (2 minuti) per tutte le connessioni HTTP." + newline +
                   "            Questa impostazione pu\u00f2 essere sostituita in Parametri del runtime Java:" + newline + newline +
                   "                  -Dsun.net.client.defaultConnectTimeout=valore in millisecondi" + newline + newline +
                   "            Un'altra propriet\u00e0 di rete che possibile impostare \u00e8 sun.net.client.defaultReadTimeout." + newline + newline +
                   "                  -Dsun.net.client.defaultReadTimeout=valore in millisecondi" + newline + newline +
                   "                  Nota" + newline + newline +
                   "                  Per impostazione predefinita in Java Plug-in non viene impostato sun.net.client.defaultReadTimeout. Se si desidera impostarlo, occorre farlo tramite" + newline +
                   "                  Parametri del runtime Java, come mostrato sopra." + newline + newline + newline +
                   "            Descrizione delle propriet\u00e0 di rete:" + newline + newline +
                   "            sun.net.client.defaultConnectTimeout" + newline +
                   "            sun.net.client.defaultReadTimeout" + newline + newline +
                   "                  Tramite queste propriet\u00e0 \u00e8 possibile specificare, rispettivamente, il timeout di connessione e di lettura per i gestori di protocollo" + newline +
                   "                  utilizzati da java.net.URLConnection. Il valore predefinito impostato dai gestori di protocollo \u00e8 -1, il che significa che" + newline +
                   "                  non \u00e8 stato impostato alcun timeout." + newline + newline +
                   "                  sun.net.client.defaultConnectTimeout specifica il timeout (in millisecondi) per stabilire la connessione all'host." + newline +
                   "                  Ad esempio, per le connessioni http \u00e8 il timeout quando si esegue la connessione al server http." + newline +
                   "                  Per le connessioni ftp \u00e8 il timeout quando si esegue la connessione ai server ftp." + newline + newline +
                   "                  specifica il timeout (in millisecondi) durante la lettura da uno stream di input" + newline +
                   "                  quando si \u00e8 stabilita una connessione a una risorsa." + newline + newline + 
                   "            Per la descrizione ufficiale di queste propriet\u00e0 di rete," + newline +
                   "            vedere http://java.sun.com/j2se/1.4/docs/guide/net/properties.html." + newline + newline + newline +
                   " Browser" + newline + newline + newline +
                   "Questo pannello fa riferimento solo alle installazioni in ambiente Microsoft Windows; non \u00e8 visualizzato nelle altre installazioni. Controllare tutti i browser" + newline +
"per i quali si desidera che Java Plug-in sia il runtime Java predefinito, invece della JVM interna del browser." + newline +
"Questo serve per abilitare il supporto del tag APPLET in Internet Explorer e Netscape 6 tramite Java Plug-in." + newline + newline + newline +
                   " Proxy" + newline + newline +newline +
                   "Utilizzare il pannello Proxy per utilizzare le impostazioni predefinite del browser o per sostituire la porta e l'indirizzo proxy per i vari protocolli." + newline + newline + 
                   "Usa impostazioni del browser" + newline + newline + 
                   "      Selezionare questa voce per utilizzare le impostazioni proxy predefinite del browser. Questa \u00e8 l'impostazione predefinita (selezionata)." + newline + newline +
                   "Tabella informazioni proxy" + newline + newline + 
                   "      Le impostazioni predefinite possono essere sostituite selezionando la casella di controllo \"Usa impostazioni browser\", quindi inserendo le informazioni" + newline +
                   "      relative ai proxy nella tabella posta sotto la casella di controllo. Si possono immettere la porta e l'indirizzo del proxy per ciascuno dei protocolli supportati:" + newline +
                   "      HTTP, Secure (HTTPS), FTP, Gopher e Socks." + newline + newline + 
                   "Nessun host proxy" + newline + newline + 
                   "      Si tratta di un elenco di host per i quali non devono essere utilizzati proxy. L'opzione Nessun host proxy \u00e8 in genere utilizzata per un host interno" + newline +
                   "      in ambiente intranet." + newline + newline + 
                   "URL configurazione proxy automatico" + newline + 
                   "      Si tratta dell'URL del file JavaScript (estensione .js o .pac) contenente la funzione FindProxyForURL." + newline +
                   "      FindProxyForURL dispone della logica sufficiente per determinare il server proxy da utilizzare per una richiesta di connessione." + newline + newline + 
                   "Per ulteriori dettagli sulla configurazione del proxy, consultare il capitolo denominato Configurazione proxy nella Guida dello sviluppatore" + newline +
                   "di Java Plug-in." + newline + newline + newline +
                   " Cache" + newline + newline + newline +
                   "           Nota" + newline + newline +
                   "           La cache a cui si fa qui riferimento \u00e8 la cache plug-in; ovvero la cache del disco creata e controllata da Java Plug-in" + newline +
                   "           e che il browser non pu\u00f2 sovrascrivere. Per ulteriori informazioni consultare Cache degli applet nella Guida dello sviluppatore di Java Plug-in." + newline + newline + newline +
                   "Abilita cache" + newline + newline + 
                   "      Selezionare la casella per abilitare l'uso della cache. Questa \u00e8 l'impostazione predefinita (selezionata). Abilitando la cache dell'applet, le prestazioni" + newline +
                   "      migliorano perch\u00e9 eseguendo la cache dell'applet non \u00e8 pi\u00f9 necessario scaricarlo se viene nuovamente referenziato." + newline + newline +
                   "      I file delle cache di Java Plug-in dei seguenti tipi scaricati via HTTP/HTTPS:" + newline + newline +
                   "            .jar (file jar)" + newline +
                   "            .zip (file zip)" + newline +
                   "            .class (file classe java)" + newline +
                   "            .au (file audio)" + newline +
                   "            .wav (file audio)" + newline +
                   "            .jpg (file immagine)" + newline +
                   "            .gif (file immagine)" + newline + newline +
                   "Visualizza file nella cache" + newline + newline + 
                   "      Premere per visualizzare i file per i quali \u00e8 abilitata la cache. Comparir\u00e0 un'altra finestra di dialogo (Visualizzatore cache di Java Plug-in) in cui sono visualizzati i file per i quali \u00e8 abilitata la cache." + newline +
                   "      Nel Visualizzatore cache sono visualizzate le seguenti informazioni sui file presenti nella cache: Nome, Tipo, Dimensioni, Data di scadenza," + newline +
                   "      Ultima modifica, Versione e URL. Nel Visualizzatore cache \u00e8 inoltre possibile scegliere selettivamente i file da eliminare dalla cache." + newline +
                   "      Si tratta di un'alternativa all'opzione Cancella cache descritta sotto, con la quale \u00e8 possibile eliminare tutti i file presenti nella cache." + newline + newline +
                   "Cancella cache" + newline + newline + 
                   "      Premere per eliminare tutti i file presenti nella cache. Verr\u00e0 richiesto di confermare l'eliminazione (Eliminare tutti i file nella... _cache?) prima che i file vengano rimossi." + newline + newline + 
                   "Percorso" + newline + newline + 
                   "      Da utilizzare per specificare il percorso della cache. Il percorso predefinito della cache \u00e8 <user home>/.jpi_cache, dove" + newline +
                   "      <user home> \u00e8 il valore della propriet\u00e0 sistema user.home. Questo valore dipende dal sistema operativo." + newline + newline +
                   "Dimensioni" + newline + newline + 
                   "      Si pu\u00f2 scegliere tra Illimitato, per non porre limiti alle dimensioni della cache, e Massimo per selezionare la dimensione massima della cache." + newline +
                   "      Se la dimensione della cache supera il limite specificato, i file meno recenti presenti nella cache verranno rimossi finch\u00e9 le dimensioni della cache non scendono al di sotto" + newline +
                   "      del limite specificato." + newline + newline + 
                   "Compressione JAR" + newline + newline + 
                   "      Si pu\u00f2 impostare la compressione dei file della cache JAR su un valore compreso tra Nessuna e Alta. Specificando un valore di compressione elevato" + newline +
                   "      si risparmia memoria, tuttavia ci\u00f2 influir\u00e0 negativamente sulla prestazioni; le prestazioni ottimali si ottengono" + newline +
                   "      senza compressione." + newline + newline + newline + 
                   " Certificati" + newline + newline + newline +
                   "Si possono selezionare quattro tipi di certificati:" + newline + newline +
                   "      Applet firmato" + newline + 
                   "      Sito sicuro" + newline + 
                   "      Firmatario CA" + newline +
                   "      Sito sicuro CA" + newline + newline + 
                   "Applet firmato" + newline + newline +
                   "      Si tratta di certificati per applet firmati che vengono accettati dall'utente. I certificati presenti nell'elenco degli applet firmati " + newline +
                   "      vengono letti dal file del certificato jpicerts<version> situato nella directory <user home>/.java." + newline + newline +
                   "Sito sicuro" + newline + newline +
                   "      Si tratta di certificati per siti sicuri. I certificati presenti nell'elenco degli applet firmati vengono letti dal file del certificato" + newline +
                   "      jpihttpscerts<version> situato nella directory <user home>/.java." + newline + newline + 
                   "Firmatario CA" + newline + newline +
                   "      Sono certificati di Authority di certificazione (CA, Certificate Authorities) per gli applet firmati; queste authority rilasciano i certificati" + newline +
                   "       ai firmatari degli applet firmati. I certificati presenti nell'elenco Firmatario CA vengono letti dal file del certificato" + newline +
                   "       cacerts, situato nella directory <jre>/lib/security." + newline + newline +
                   "Sito sicuro CA" + newline + newline +
                   "      Sono certificati di Authority di certificazione (CA, Certificate Authorities) per siti sicuri; queste authority rilasciano i certificati" + newline +
                   "      per i siti sicuri. I certificati presenti nell'elenco Sito sicuro CA vengono letti dal file del certificato jssecacerts," + newline +
                   "      situato nella directory <jre>/lib/security." + newline + newline +
                   "Per i certificati Applet firmato e Sito sicuro, sono disponibili quattro opzioni: Importa, Esporta, Rimuovi e Dettagli." + newline +
                   "L'utente pu\u00f2 importare, esportare, rimuovere e visualizzare i dettagli di un certificato." + newline + newline + 
                   "Per Firmatario CA e Sito sicuro CA, \u00e8 disponibile soltanto un'opzione: Dettagli. L' utente pu\u00f2 visualizzare soltanto i dettagli di un certificato." + newline + newline + newline +
                   " Aggiornamento" + newline + newline + newline +
                   "In questo pannello, \u00e8 presente il pulsante \"Scarica aggiornamento Java\" che consente agli utenti di scaricare l'ultimo aggiornamento dell'ambiente runtime Java " + newline +
"dal sito web Java Update. Questo pannello fa riferimento solo alle installazioni su piattaforma Microsoft Windows; non viene visualizzato" + newline +
"nelle altre installazioni (ad esempio Solaris/Linux)."}
};
} 

 


