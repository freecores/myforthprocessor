/*
 * @(#)ConverterHelp_fr.java	1.2 03/04/24
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * French version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_fr extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");;

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Convertisseur HTML Java(tm) Plug-in - Fichier Readme" + newline + newline +
      "Version\u00a0: 1.4.2" + newline + newline + newline +
      "*****   SAUVEGARDEZ LES FICHIERS AVANT DE LES CONVERTIR AVEC CET OUTIL." + newline +
      "*****   L'ANNULATION D'UNE CONVERSION N'ENTRA\u00eeNE PAS UNE ANNULATION DES MODIFICATIONS." + newline +
      "*****   LES COMMENTAIRES AU SEIN DE LA BALISE APPLET SONT IGNOR\u00c9S." + newline + newline + newline +
      "Sommaire" + newline +
      "   1.  Nouvelles fonctionnalit\u00e9s" + newline +
      "   2.  Corrections de bogue" + newline +
      "   3.  \u00c0 propos du convertisseur HTML de Java(tm) Plug-in" + newline +
      "   4.  Processus de conversion" + newline +
      "   5.  S\u00e9lection des fichiers \u00e0 convertir" + newline +
      "   6.  S\u00e9lection du dossier de sauvegarde" + newline +
      "   7.  G\u00e9n\u00e9ration d'un fichier journal" + newline +
      "   8.  S\u00e9lection d'un mod\u00e8le de conversion" + newline +
      "   9.  Conversion" + newline +
      "  10.  Nouvelles conversions / Quitter" + newline +
      "  11.  D\u00e9tails relatifs aux mod\u00e8les" + newline +
      "  12.  Ex\u00e9cution du convertisseur HTML (Windows & Solaris)" + newline + newline +
      "1)  Nouvelles fonctionnalit\u00e9s\u00a0:" + newline + newline +     
      "    o Mise \u00e0 jour et extension des mod\u00e8les pour la prise en charge de Netscape\u00a06." + newline +
      "    o Mise \u00e0 jour de tous les mod\u00e8les pour la prise en charge des nouvelles fonctionnalit\u00e9s de gestion des versions dans Java Plug-in." + newline +
      "    o Am\u00e9lioration de l'interface utilisateur relative \u00e0 Swing 1.1 pour la prise en charge de l'internationalisation." + newline +
      "    o Am\u00e9lioration de la bo\u00eete de dialogue Options avanc\u00e9es pour la prise en charge des nouvelles balises de mod\u00e8le SmartUpdate et" + newline +
      "      MimeType." + newline +
      "    o Am\u00e9lioration du convertisseur HTML pour une utilisation avec Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x, Java Plug-in 1.3.x et Java Plug-in" + newline +
      "      1.4.x." + newline +
      "    o Am\u00e9lioration de la prise en charge SmartUpdate et MimeType dans tous les mod\u00e8les de conversion." + newline +
      "      ." + newline +
      "    o Ajout de \"scriptable=false\" \u00e0 la balise OBJECT/EMBED dans tous les mod\u00e8les." + newline + newline + 
      "     Cette valeur de param\u00e8tre est employ\u00e9e pour d\u00e9sactiver la g\u00e9n\u00e9ration de la biblioth\u00e8que de types lorsque Java" + newline + newline +
      "    Plug-in n'est pas utilis\u00e9 pour les scripts." + newline + newline + newline +
      "2)  Corrections de bogue\u00a0:" + newline + newline +
      "    o Am\u00e9lioration du traitement des erreurs lorsque les fichiers de propri\u00e9t\u00e9s sont introuvables." + newline +
      "    o Am\u00e9lioration de la conversion HTML de fa\u00e7on \u00e0 ce que la balise EMBED/OBJECT r\u00e9sultante puisse \u00eatre" + newline +
      "      utilis\u00e9e dans le visualiseur d'applet de JDK 1.2.x." + newline +
      "    o \u00c9limination des fichiers inutiles issus du convertisseur HTML 1.1.x." + newline +
      "    o G\u00e9n\u00e9ration de la balise EMBED/OBJECT avec les noms d'attribut CODE, CODEBASE etc." + newline +
      "      plut\u00f4t qu'avec JAVA_CODE, JAVA_CODEBASE etc. Cette modification" + newline +
      "      permet d'utiliser la page g\u00e9n\u00e9r\u00e9e dans le visualiseur d'applet de JDK\u00a01.2.x." + newline +
      "    o Prise en charge de la conversion MAYSCRIPT si cet \u00e9l\u00e9ment est pr\u00e9sent\u00e9 dans la balise" + newline +
      "      APPLET." + newline + newline +
      "3)  \u00c0 propos du convertisseur HTML de Java(tm) Plug-in\u00a0:" + newline + newline +
      "        Le convertisseur HTML de Java(tm) Plug-in est un utilitaire qui vous permet de convertir des" + newline +
      "        pages HTML contenant des applets en un format qui utilisera Java(tm)" + newline +
      "        Plug-in." + newline + newline +       
      "4)  Processus de conversion\u00a0:" + newline + newline +
      "        Le convertisseur HTML de Java(tm) Plug-in convertit les fichiers contenant" + newline +
      "        des applets en un format susceptible d'\u00eatre utilis\u00e9 avec Java(tm) Plug-in." + newline + newline +
      "        Le processus de conversion d'un fichier est le suivant\u00a0:" + newline +
      "        Dans un premier temps, le langage HTML qui ne fait pas partie d'une applet est transf\u00e9r\u00e9 du fichier source" + newline +
      "        vers un fichier temporaire. Lorsqu'une balise <APPLET est d\u00e9tect\u00e9e, le convertisseur" + newline +
      "        analyse l'applet jusqu'\u00e0 la premi\u00e8re balise </APPLET (non contenue entre guillemets)" + newline +
      "        et fusionne les donn\u00e9es de l'applet avec le mod\u00e8le (reportez-vous \u00e0 la rubrique D\u00e9tails relatifs aux mod\u00e8les" + newline +
      "        ci-dessous). Si cette op\u00e9ration s'ex\u00e9cute sans erreur, le fichier HTML d'origine est d\u00e9plac\u00e9" + newline +
      "        vers le dossier de sauvegarde et le fichier temporaire est alors renomm\u00e9 avec le" + newline +
      "        nom du fichier d'origine. Par cons\u00e9quent, les fichiers d'origine ne sont jamais supprim\u00e9s du disque." + newline + newline +
      "        Le convertisseur convertit effectivement les fichiers \"sur place\". Ainsi," + newline +
      "        une fois le convertisseur ex\u00e9cut\u00e9, vos fichiers sont configur\u00e9s de fa\u00e7on \u00e0 utiliser Java(tm) Plug-in." + newline +
       

      "5)  S\u00e9lection des fichiers \u00e0 convertir\u00a0:" + newline + newline +
      "       Pour convertir tous les fichiers d'un dossier, vous pouvez entrer le chemin d'acc\u00e8s au dossier" + newline +
      "       ou utiliser le bouton Parcourir pour s\u00e9lectionner un dossier \u00e0 partir d'une bo\u00eete de dialogue." + newline  +
      "       Une fois le chemin s\u00e9lectionn\u00e9, vous pouvez indiquer autant de sp\u00e9cificateurs de fichier que vous souhaitez dans la zone" + newline +
      "       \"Noms de fichier correspondants\". Les sp\u00e9cificateurs doivent \u00eatre s\u00e9par\u00e9s par une virgule. Vous pouvez utiliser le caract\u00e8re g\u00e9n\u00e9rique *" + newline +
      "       . Si vous indiquez un nom de fichier avec un caract\u00e8re g\u00e9n\u00e9rique, seul ce fichier sera" + newline +
      "       converti. En dernier lieu, activez la case \u00e0 cocher \"Inclure les sous-dossiers\" si vous souhaitez" + newline +
      "       trouver tous les fichiers des dossiers imbriqu\u00e9s qui correspondent au nom du fichier \u00e0 convertir." + newline + newline +      
      "6)  S\u00e9lection du dossier de sauvegarde\u00a0:" + newline +

      "       Le chemin du dossier de sauvegarde par d\u00e9faut correspond au chemin source auquel l'indication \"_BAK\" a \u00e9t\u00e9 ajout\u00e9e." + newline +
      "       En d'autres termes, si le chemin source est c:/html/applet.html (conversion d'un fichier)," + newline +
      "       le chemin de sauvegarde est alors c:/html_BAK. Si le chemin source" + newline +
      "       est c:/html (conversion de tous les fichiers du chemin), le chemin de sauvegarde est alors" + newline +
      "       c:/html_BAK. Vous pouvez modifier le chemin de sauvegarde en entrant un chemin dans le champ de la zone" + newline +
      "       \"Sauvegarder les fichiers dans le dossier\u00a0:\" ou en recherchant un dossier (Parcourir)." + newline + newline +

      "       Unix (Solaris)\u00a0:" + newline +
      "       Le chemin du dossier de sauvegarde par d\u00e9faut correspond au chemin source auquel l'indication \"_BAK\" a \u00e9t\u00e9 ajout\u00e9e." + newline +
      "       En d'autres termes, si le chemin source est /home/user1/html/applet.html (conversion d'un fichier)," + newline +
      "       le chemin de sauvegarde est alors /home/user1/html_BAK. Si le chemin source" + newline +
      "       est /home/user1/html (conversion de tous les fichiers du chemin), le chemin de sauvegarde est alors" + newline +
      "       /home/user1/html_BAK. Vous pouvez modifier le chemin de sauvegarde en entrant" + newline +
      "       un chemin dans le champ de la zone \"Sauvegarder les fichiers dans le dossier\u00a0:\" ou en recherchant un dossier (Parcourir)." + newline + newline +      
      "7)  G\u00e9n\u00e9ration d'un fichier journal\u00a0:" + newline + newline +
      "       Si vous souhaitez g\u00e9n\u00e9rer un fichier journal, activez la case \u00e0 cocher" + newline +
      "       \"G\u00e9n\u00e9rer un fichier journal\". Vous pouvez entrer le nom et le chemin du fichier ou encore rechercher" + newline +
      "       un dossier, entrer le nom du fichier et s\u00e9lectionner Ouvrir." + newline +
      "       Le fichier journal contient des informations de base li\u00e9es au processus de conversion" + newline +
      "       ." + newline + newline +        
      "8)  S\u00e9lection d'un mod\u00e8le de conversion\u00a0:" + newline + newline +
      "       Si vous ne s\u00e9lectionnez pas de mod\u00e8le, le mod\u00e8le par d\u00e9faut est utilis\u00e9. Ce mod\u00e8le" + newline +
      "       produit des fichiers HTML convertis compatibles avec IE et Netscape." + newline  +
      "       Si vous souhaitez utiliser un mod\u00e8le diff\u00e9rent, s\u00e9lectionnez-le dans le menu" + newline +
      "       de l'\u00e9cran principal. Dans ce cas, vous \u00eates autoris\u00e9 \u00e0 choisir un fichier" + newline +
      "       qui sera utilis\u00e9 en tant que mod\u00e8le." + newline +
      "       Si vous choisissez un fichier, VEILLEZ \u00c0 CE QU'IL S'AGISSE D'UN MOD\u00c8LE." + newline + newline +
      "9)  Conversion\u00a0:" + newline + newline +
      "       Cliquez sur le bouton \"Convertir...\" pour lancer le processus de conversion. Une bo\u00eete de dialogue" + newline +
      "       indique les fichiers en cours de traitement, le nombre de fichiers trait\u00e9s," + newline +
      "       le nombre d'applets trouv\u00e9es et le nombre d'erreurs." + newline + newline +       
      "10) Nouvelles conversions / Quitter\u00a0:" + newline + newline +
      "       Lorsque la conversion est termin\u00e9e, le bouton \"Annuler\" de la bo\u00eete de dialogue est remplac\u00e9 par" + newline +
      "       le bouton \"Termin\u00e9\". Vous pouvez choisir \"Termin\u00e9\" pour fermer la bo\u00eete de dialogue." + newline  +
      "       \u00c0 ce stade, choisissez \"Quitter\" pour fermer le convertisseur HTML de Java(tm) Plug-in" + newline +
      "       ou s\u00e9lectionnez une autre s\u00e9rie de fichiers \u00e0 convertir puis cliquez de nouveau sur \"Convertir...\"." + newline + newline +       
      "11)  D\u00e9tails relatifs aux mod\u00e8les\u00a0:" + newline + newline +
      "       Le fichier mod\u00e8le est la base de la conversion des applets.  Il s'agit d'un simple fichier texte" + newline +
      "       qui contient des balises repr\u00e9sentant les parties de l'applet d'origine." + newline +
      "       L'ajout de balises \u00e0 un fichier mod\u00e8le ou encore la suppression/le d\u00e9placement de ces derni\u00e8res vous permettent de modifier la sortie" + newline +
      "       du fichier converti." + newline + newline +
      "       Balises prises en charge\u00a0:" + newline + newline +
      "        $OriginalApplet$    Cette balise est remplac\u00e9e par le texte complet" + newline +
      "        de l'applet d'origine." + newline + newline +
      "        $AppletAttributes$   Cette balise est remplac\u00e9e par tous les" + newline +
      "        attributs de l'applet (code, codebase, width, height etc.)." + newline + newline +
      "        $ObjectAttributes$   Cette balise est remplac\u00e9e par tous les" + newline +
      "        attributs requis par la balise \"object\"." + newline + newline +
      "        $EmbedAttributes$   Cette balise est remplac\u00e9e par tous les attributs" + newline +
      "        requis par la balise \"embed\"." + newline + newline +
      "        $AppletParams$    Cette balise est remplac\u00e9e par toutes les balises" + newline +
      "        <param ...> de l'applet." + newline + newline +
      "        $ObjectParams$    Cette balise est remplac\u00e9e par toutes les balises <param...>" + newline +
      "        requises par la balise \"object\"." + newline + newline +
      "        $EmbedParams$     Cette balise est remplac\u00e9e par toutes les balises <param...>" + newline +
      "        requises par la balise \"embed\" sous la forme NAME=VALUE." + newline + newline +
      "        $AlternateHTML$  Cette balise est remplac\u00e9e par le texte de la zone" + newline +
      "        \"no-support-for-applets\" (pas de prise en charge des applets) de l'applet d'origine." + newline + newline +
      "        $CabFileLocation$   Il s'agit de l'URL du fichier cab qui doit \u00eatre" + newline +
      "        utilis\u00e9 dans chaque mod\u00e8le ayant pour cible IE." + newline + newline +
      "        $NSFileLocation$    Il s'agit de l'URL du plug-in Netscape qui doit \u00eatre" + newline +
      "        utilis\u00e9 dans chaque mod\u00e8le ayant pour cible Netscape." + newline + newline +
      "        $SmartUpdate$   Il s'agit de l'URL du composant SmartUpdate Netscape" + newline +
      "        qui doit \u00eatre utilis\u00e9 dans chaque mod\u00e8le ayant pour cible Netscape Navigator\u00a04.0 ou version ult\u00e9rieure." + newline + newline +
      "        $MimeType$    Il s'agit du type MIME de l'objet Java." + newline + newline +     

      "      default.tpl est le mod\u00e8le par d\u00e9faut du convertisseur. Vous pouvez utiliser la page convertie" + newline +
      "      dans IE et Navigator sous Windows pour appeler Java(TM) Plugin." + newline +
      "      Vous pouvez \u00e9galement utiliser ce mod\u00e8le avec Netscape sous Unix (Solaris)." + newline + newline +
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
      "      ieonly.tpl - Vous pouvez utiliser la page convertie pour appeler Java(TM)" + newline +
      "      Plug-in uniquement dans IE sous Windows." + newline + newline +
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
      "      nsonly.tpl - Vous pouvez utiliser la page convertie pour appeler Java(TM)" + newline +
      "      Plug-in dans Navigator sous Windows et Solaris." + newline + newline +
      "      <!-- HTML CONVERTER -->" + newline +
      "      <EMBED type=\"$MimeType$\" $EmbedAttributes$" + newline +
      "      $EmbedParams$ scriptable=false" + newline +
      "      pluginspage=\"$NSFileLocation$\"><NOEMBED>" + newline +
      "      $AlternateHTML$" + newline +
      "      </NOEMBED></EMBED>" + newline + newline +
      "      <!--" + newline +
      "      $ORIGINALAPPLET$" + newline +
      "      -->" + newline + newline +       
      "      extend.tpl - Vous pouvez utiliser la page convertie dans n'importe quel navigateur et sur n'importe quelle plateforme." + newline +
      "      Si le navigateur est IE ou Navigator sous Windows (Navigator sous Solaris), Java(TM)" + newline +
      "      Plug-in sera appel\u00e9. Sinon, la JVM (Java Virtual Machine) par d\u00e9faut du navigateur est utilis\u00e9e." + newline + newline +
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
      "12)  Ex\u00e9cution du convertisseur HTML\u00a0:" + newline + newline +
      "      Ex\u00e9cution de la version GUI (interface utilisateur graphique) du convertisseur HTML" + newline + newline +
      "      Le convertisseur HTML est inclus dans le SDK, non dans le JRE. Pour ex\u00e9cuter le convertisseur, atteignez le" + newline +
      "      sous-r\u00e9pertoire lib de votre r\u00e9pertoire d'installation SDK. Par exemple," + newline +
      "      si vous avez install\u00e9 le SDK sous Windows, disque C, effectuez une op\u00e9ration cd pour atteindre" + newline + newline +
      "            C:\\j2sdk1.4.2\\lib\\" + newline + newline +
      "      Le convertisseur (htmlconverter.jar) est inclus dans ce r\u00e9pertoire." + newline + newline +
      "      Pour lancer le convertisseur, entrez\u00a0:" + newline + newline +
      "            C:\\j2sdk1.4.2\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      Le lancement du convertisseur sous UNIX/Linux est une op\u00e9ration identique \u00e0 l'op\u00e9ration pr\u00e9c\u00e9dente." + newline +
      "      Vous trouverez ci-dessous d'autres modalit\u00e9s de lancement du convertisseur." + newline + newline +
      "      Sous Windows" + newline +
      "      Pour lancer le convertisseur \u00e0 l'aide de l'Explorateur\u00a0:" + newline +
      "      Utilisez l'Explorateur pour atteindre le r\u00e9pertoire suivant" + newline + newline +
      "      C:\\j2sdk1.4.2\\bin" + newline + newline +
      "      Double-cliquez sur l'application HtmlConverter." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      Ex\u00e9cutez les commandes suivantes" + newline + newline +
      "      cd /j2sdk1.4.2/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +             
      "      Ex\u00e9cution du convertisseur \u00e0 partir de la ligne de commande\u00a0:" + newline + newline +
      "      Format\u00a0:" + newline + newline +
      "      java -jar htmlconverter.jar [-options1 value1 [-option2 value2" + newline +
      "      [...]]] [-simulate] [filespecs]" + newline + newline +
      "      filespecs\u00a0:  liste de sp\u00e9cificateurs de fichier, d\u00e9limit\u00e9s par des espaces (possibilit\u00e9 d'inclure un caract\u00e8re g\u00e9n\u00e9rique *). " + newline +
      "      (*.html *.htm)" + newline + newline +
      "      Options\u00a0:" + newline + newline +
      "       source\u00a0:    Chemin des fichiers (c:\\htmldocs sous Windows," + newline +
      "                  /home/user1/htmldocs sous Unix) Par d\u00e9faut\u00a0: <userdir>" + newline +
      "                  Si le chemin est relatif, il est suppos\u00e9 \u00eatre li\u00e9 au" + newline +
      "                  r\u00e9pertoire \u00e0 partir duquel HTMLConverter a \u00e9t\u00e9 lanc\u00e9." + newline + newline +
      "       backup\u00a0:    Chemin des fichiers de sauvegarde. Par d\u00e9faut\u00a0:" + newline +
      "                  <userdir>/<source>_bak" + newline +
      "                  Si le chemin est relatif, il est suppos\u00e9 \u00eatre li\u00e9 au" + newline +
      "                  r\u00e9pertoire \u00e0 partir duquel HTMLConverter a \u00e9t\u00e9 lanc\u00e9." + newline + newline +
      "       subdirs\u00a0:   D\u00e9finit si les fichiers des sous-r\u00e9pertoires doivent \u00eatre trait\u00e9s ou non. " + newline +
      "                  Par d\u00e9faut\u00a0: FALSE" + newline + newline +
      "       template\u00a0:  Nom du fichier mod\u00e8le. Par d\u00e9faut\u00a0: default.tpl - Standard " + newline +
      "                  (IE et Navigator) pour Windows et Solaris uniquement. EN CAS D'INCERTITUDE, UTILISEZ LA VALEUR PAR D\u00c9FAUT." + newline + newline +
      "       log\u00a0:       Chemin et nom du fichier journal (Par d\u00e9faut <userdir>/convert.log)" + newline + newline +
      "       progress\u00a0:  Affiche la progression en sortie durant la conversion. " + newline +
      "                  Par d\u00e9faut\u00a0: FALSE" + newline + newline +
      "       simulate\u00a0:  Affiche un aper\u00e7u de la conversion sans pour autant ex\u00e9cuter le processus." + newline +
      "                  EN CAS D'INCERTITUDE SUR UNE CONVERSION, UTILISEZ CETTE OPTION." + newline +
      "                  VOUS OBTIENDREZ UNE LISTE DE D\u00c9TAILS RELATIFS \u00c0" + newline +
      "                  LA CONVERSION." + newline + newline +
      "      Si vous sp\u00e9cifiez uniquement \"java -jar htmlconverter.jar -gui\" (uniquement l'option -gui" + newline +
      "      sans filespecs), la version GUI du convertisseur sera lanc\u00e9e." + newline  +
      "      Sinon, l'interface utilisateur graphique est supprim\u00e9e." + newline + newline +
      "      Pour plus d'informations, consultez l'URL suivant\u00a0:" + newline + newline +
      "      http://java.sun.com/j2se/1.4.2/docs/guide/plugin/developer_guide/html_converter_more.html"}
};
} 

 


