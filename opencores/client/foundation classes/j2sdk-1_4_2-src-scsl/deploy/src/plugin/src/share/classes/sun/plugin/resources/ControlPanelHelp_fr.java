/*
 * @(#)ControlPanelHelp_fr.java	1.3 03/03/31
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

public class ControlPanelHelp_fr extends ListResourceBundle {

    private static String newline = "\n";

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "help.file", newline +
                   "Aide sur le Panneau de configuration Java Plug-in" + newline + newline + newline +
                   "Les rubriques abord\u00e9es dans cette aide sont les suivantes :" + newline +
                   "      Vue d'ensemble" + newline + 
                   "      Enregistrement des options" + newline + 
                   "      D\u00e9finition des options du panneau de configuration" + newline + 
                   "      Onglet Propri\u00e9t\u00e9s de base" + newline + 
                   "      Onglet Propri\u00e9t\u00e9s avanc\u00e9es" + newline +
                   "      Onglet Navigateur" + newline + 
                   "      Onglet Proxies" + newline + 
                   "      Onglet Cache" + newline + 
                   "      Onglet Certificats" + newline + 
                   "      Onglet Mise \u00e0 jour" + newline + newline + 
                   "Vue d'ensemble" + newline + newline +
                   "Le panneau de configuration Java Plug-in vous permet de modifier les param\u00e8tres par d\u00e9faut utilis\u00e9s au d\u00e9marrage de Java Plug-in" + newline +
                   "Tous les applets s'ex\u00e9cutant dans une instance active de Java Plug-in utilisent ces param\u00e8tres" + newline +
                   "Le Guide du d\u00e9veloppeur de Java Plug-in, cit\u00e9 en r\u00e9f\u00e9rence dans ce document, est accessible \u00e0 l'adresse suivante (URL susceptible de modification)." + newline +
                   "http://java.sun.com/j2se/1.4/docs/guide/plugin/developer_guide/contents.html" + newline + newline +
                   "Enregistrement des options" + newline + newline +
                   "Une fois que vous avez apport\u00e9 les modifications voulues aux options du Panneau de configuration, cliquez sur Appliquer pour les enregistrer." + newline +
                   "Cliquez sur Restaurer pour annuler les modifications et recharger les derni\u00e8res valeurs qui \u00e9taient entr\u00e9es et appliqu\u00e9es." + newline +
                   "Notez qu'il ne s'agit pas du jeu de valeurs par d\u00e9faut qui \u00e9taient d\u00e9finies \u00e0 l'origine pour Java Plug-in au moment o\u00f9 il a \u00e9t\u00e9 install\u00e9." + newline + newline +
                   "D\u00e9finition des options du Panneau de configuration" + newline + newline +
                   "Six onglets vous permettent de configurer diverses options du Panneau de configuration Java Plug-in." + newline +
                   "Ces onglets sont les suivants : " + newline +
                   "      Propri\u00e9t\u00e9s de base" + newline + 
                   "      Propri\u00e9t\u00e9s avanc\u00e9es" + newline + 
                   "      Navigateur" + newline + 
                   "      Proxies" + newline + 
                   "      Cache" + newline + 
                   "      Certificats" + newline + 
                   "      Mise \u00e0 jour" + newline + newline +
                   "Tous ces onglets sont d\u00e9crits s\u00e9par\u00e9ment ci-dessous." + newline + newline + newline +
                   "Propri\u00e9t\u00e9s de base" + newline +
                   "Afficher la console" + newline + newline + 
                   "      Affiche la console Java pendant l'ex\u00e9cution des applets. La console affiche les messages imprim\u00e9s par System.out et System.err" + newline +
                   "      Cette fonctionnalit\u00e9 est utile pour les probl\u00e8mes de d\u00e9bogage." + newline + newline +
                   "Masquer la console" + newline + newline + 
                   "      La console Java Console est ex\u00e9cut\u00e9e mais masqu\u00e9e. C'est le param\u00e8tre par d\u00e9faut (option s\u00e9lectionn\u00e9e)." + newline + newline +
                   "Ne pas d\u00e9marrer la console" + newline + newline + 
                   "      La console Java n'est pas d\u00e9marr\u00e9e." + newline + newline + 
                   "Afficher la bo\u00eete de dialogue Exception" + newline + newline + 
                   "      Affiche la bo\u00eete de dialogue Exception lorsque des exceptions se produisent. Le param\u00e8tre par d\u00e9faut est de ne pas afficher la bo\u00eete de dialogue Exception (option d\u00e9s\u00e9lectionn\u00e9e)." + newline + newline +
                   "Afficher Java dans la barre d'\u00e9tat du Syst\u00e8me (Windows uniquement)" + newline + newline + 
                   "      L'activation de cette option d\u00e9clenche l'affichage du logo Java en forme de tasse de caf\u00e9 dans la barre d'\u00e9tat du Syst\u00e8me quand le Java Plug-in d\u00e9marre" + newline +
                   "      et dispara\u00eet de la barre d'\u00e9tat du Syst\u00e8me \u00e0 l'arr\u00eat du Java Plug-in." + newline +
                   "      Le logo Java en forme de tasse \u00e0 caf\u00e9 indique \u00e0 l'utilisateur qu'un Java VM s'ex\u00e9cute et donne des informations sur le" + newline +
                   "      Version Java et contr\u00f4le de la Console Java." + newline +
                   "      Cette option est activ\u00e9e par d\u00e9faut (coch\u00e9e)." + newline + newline +
                   "      Fonctionnalit\u00e9 Java de la barre d'\u00e9tat du Syst\u00e8me :" + newline + newline + 
                   "      Lorsque la souris pointe vers le logo Java en forme de tasse \u00e0 caf\u00e9, le texte \"Java\" s'affiche." + newline + newline +

                   "      Lorsque l'ic\u00f4ne de la barre d'\u00e9tat du syst\u00e8me Java est s\u00e9lectionn\u00e9e \u00e0 l'aide du bouton gauche de la souris, la fen\u00eatre Java Console s'affiche." + newline + newline + 
                   "      Lorsque l'ic\u00f4ne de la barre d'\u00e9tat du Syst\u00e8me Java est s\u00e9lectionn\u00e9e \u00e0 l'aide du bouton droit de la souris, un menu s'affiche avec les options de menu ci-dessous :" + newline + newline +
                   "            Ouvrir/Fermer Console" + newline + 
                   "            \u00c0 propos de Java" + newline + 
                   "            D\u00e9sactiver" + newline + 
                   "            Quitter" + newline + newline + 
                   "      Ouvrir/Fermer la console ouvre/ferme la fen\u00eatre de la Console Java. L'option de menu affiche Ouvrir la Console si la Console" + newline +
                   "      Java est masqu\u00e9e et Fermer la console si la Console Java s'affiche." + newline + newline +
                   "      \u00c0 propos de Java affiche la fen\u00eatre \u00c0 propos de l'Edition Java 2 Standard." + newline + newline +
                   "      D\u00e9sactiver d\u00e9sactive et supprime l'ic\u00f4ne Java de la barre d'\u00e9tat du Syst\u00e8me pour les sessions pr\u00e9sentes et \u00e0 venir. Au red\u00e9marrage du" + newline +
                   "       Java Plug-in, l'ic\u00f4ne Java n'appara\u00eet pas dans la barre d'\u00e9tat du Syst\u00e8me." + newline +
                   "      Pour savoir comment afficher l'ic\u00f4ne Java dans la barre d'\u00e9tat du Syst\u00e8me, apr\u00e8s sa d\u00e9sactivation, voir la remarque ci-dessous." + newline + newline +
                   "      Quitter supprime l'ic\u00f4ne Java de la barre d'\u00e9tat du Syst\u00e8me pour la session actuelle uniquement. Une fois le Java Plug-in red\u00e9marr\u00e9, l'ic\u00f4ne" + newline +
                   "      Java r\u00e9appara\u00eet dans la barre d'\u00e9tat du Syst\u00e8me." + newline + newline + newline + 
                   "                Remarques" + newline + newline +
                   "                1. Si \"Afficher Java dans la barre d'\u00e9tat du Syst\u00e8me\" est s\u00e9lectionn\u00e9, l'ic\u00f4ne Java s'affiche dans la barre d'\u00e9tat du Syst\u00e8me m\u00eame si \"Ne pas d\u00e9marrer" + newline +
                   "                la console\" est s\u00e9lectionn\u00e9." + newline + newline +
                   "                2. Pour activer l'ic\u00f4ne Java apr\u00e8s sa d\u00e9sactivation, d\u00e9marrez le panneau de configuration Java Plug-in, cochez \"Afficher Java" + newline +
                   "                dans la barre d'\u00e9tat du Syst\u00e8me\", et appuyez sur \"Appliquer\"." + newline + newline +
                   "                3. Si d'autres Java VM s'ex\u00e9cutent d\u00e9j\u00e0 et que d'autres ic\u00f4nes Java ont \u00e9t\u00e9 ajout\u00e9es \u00e0 la barre d'\u00e9tat du Syst\u00e8me," + newline +
                   "                la modification du param\u00e9trage du panneau de configuration Java Plug-in n'affectera pas ces ic\u00f4nes." + newline +
                   "                Le param\u00e9trage affectera uniquement le comportement de l'ic\u00f4ne Java quand le Java VM est d\u00e9marr\u00e9 apr\u00e8s." + newline + newline + newline +
                   " Propri\u00e9t\u00e9s avanc\u00e9es" + newline + newline + newline +
                   "Java Run Time Environment" + newline + newline +
                   "      Permet \u00e0 Java Plug-in de fonctionner avec n'importe quelle version de Java 2 JRE ou SDK, Standard Edition v 1.3 ou 1.4 install\u00e9e sur votre ordinateur." + newline +
                   "      Le logiciel Java Plug-in 1.3/1.4 est fourni avec un JRE par d\u00e9faut." + newline +
                   "      Vous pouvez, cependant, remplacer le JRE par d\u00e9faut et utiliser une version plus ancienne ou plus r\u00e9cente. Le Panneau de configuration d\u00e9tecte automatiquement" + newline +
                   "      toutes les versions de Java 2 SDK ou JRE install\u00e9es sur l'ordinateur. La zone de liste contient toutes les versions" + newline +
                   "       que vous pouvez utiliser." + newline +
                   "      Le premier \u00e9l\u00e9ment de la liste est toujours le JRE par d\u00e9faut ; et le dernier \u00e9l\u00e9ment est toujours Autres. Si vous choisissez l'option Autres, vous devez" + newline +
                   "      indiquer le chemin d'acc\u00e8s de Java 2 JRE ou SDK, Standard Edition v 1.3/1.4." + newline + newline + 
                   "                Remarque" + newline + newline +
                   "                Cette option ne doit \u00eatre modifi\u00e9e que par des utilisateurs exp\u00e9riment\u00e9s. Il est d\u00e9conseill\u00e9 de modifier le JRE par d\u00e9faut." + newline + newline + newline +
                   "Param\u00e8tres de Java Run Time" + newline + newline + 
                   "      Remplace les param\u00e8tres de d\u00e9marrage par d\u00e9faut de Java Plug-in par les options personnalis\u00e9es sp\u00e9cifi\u00e9es. La syntaxe est identique \u00e0 celle des " + newline +
                   "      param\u00e8tres d'appel \u00e0 partir de la ligne de commande Java. Consultez la documentation de Java 2 Standard Edition (J2SE) pour obtenir" + newline +
                   "      la liste compl\u00e8te des options de ligne de commande." + newline +
                   "      L'URL ci-dessous est susceptible de modification :" + newline + newline +
                   "            http://java.sun.com/j2se/1.4/docs/tooldocs/<platform>/java.html" + newline + newline + 
                   "            o\u00f9 <plate-forme> est l'un des syst\u00e8mes d'exploitation suivants : solaris, linux, win32." + newline + newline + 
                   "      Vous trouverez ci-dessous quelques exemples des param\u00e8tres Java Runtime." + newline + newline +
                   "      Activation et d\u00e9sactivation de la prise en charge des assertions" + newline + newline +
                   "            Pour activer la prise en charge des assertions, vous devez sp\u00e9cifier la propri\u00e9t\u00e9 syst\u00e8me suivante dans les param\u00e8tres de Java Runtime :" + newline + newline +
                   "                  -D[ enableassertions | ea ][:<package name>\"...\" | : <class name> ]" + newline + newline +
                   "            Pour d\u00e9sactiver les assertions dans le Java Plug-in, sp\u00e9cifiez ce qui suit dans les param\u00e8tres de Java Runtime :" + newline + newline + 
                   "                  -D[ disableassertions | da ][:<package name>\"...\" | : <class name> ]" + newline + newline + 
                   "            Pour plus de d\u00e9tails sur l'activation/la d\u00e9sactivation des assertions, consultez la rubrique concernant la fonctionnalit\u00e9 correspondante \u00e0 l'adresse suivante" + newline +
                   "            http://java.sun.com/j2se/1.4/docs/guide/lang/assert.html (URL susceptible de modification)." + newline + newline +
                   "            L'assertion est d\u00e9sactiv\u00e9e dans le code de Java Plug-in par d\u00e9faut. Dans la mesure o\u00f9 l'effet de l'assertion est d\u00e9termin\u00e9 au moment du d\u00e9marrage de" + newline +
                   "            Java Plug-in, la modification des param\u00e8tres d'assertion dans le Panneau de configuration Java Plug-in exige le red\u00e9marrage du navigateur pour que" + newline +
                   "            les nouveaux param\u00e8tres prennent effet." + newline + newline + 
                   "            Etant donn\u00e9 que le code Java de Java Plug-in poss\u00e8de \u00e9galement une assertion int\u00e9gr\u00e9e, il est possible d'activer l'assertion dans " + newline +
                   "            le code Java Plug-in via la ligne suivante :" + newline + newline +
                   "                  -D[ enableassertions | ea ]:sun.plugin" + newline + newline + 
                   "      Prise en charge du tra\u00e7age et de la consignation" + newline + newline +
                   "            Le tra\u00e7age est un m\u00e9canisme qui permet de rediriger n'importe quelle sortie sur la console Java vers un fichier de trace (.plugin<version>.trace)." + newline + newline +
                   "                  -Djavaplugin.trace=true" + newline +
                   "                  -Djavaplugin.trace.option=basic|net|security|ext|liveconnect" + newline + newline +
                   "            Si vous ne souhaitez pas utiliser le nom du fichier de trace par d\u00e9faut :" + newline + newline +
                   "                  -Djavaplugin.trace.filename=<tracefilename>" + newline + newline + 
                   "            Similaire au tra\u00e7age, la consignation est une fonctionnalit\u00e9 qui permet de rediriger n'importe quelle sortie sur la console Java vers un fichier journal (.plugin<version>.log)" + newline +
                   "            en utilisant l'API Java Logging." + newline +
                   "            La consignation peut \u00eatre mise en service en activant la propri\u00e9t\u00e9 javaplugin.logging." + newline + newline +
                   "                  -Djavaplugin.logging=true" + newline + newline +
                   "            Si vous ne souhaitez pas utiliser le nom du fichier journal par d\u00e9faut, entrez :" + newline + newline +
                   "                  -Djavaplugin.log.filename=<logfilename>" + newline + newline + 
                   "            Par ailleurs, si vous ne souhaitez pas remplacer les fichiers de trace et journal \u00e0 chaque session, vous pouvez d\u00e9finir la propri\u00e9t\u00e9 suivante :" + newline + newline + 
                   "                  -Djavaplugin.outputfiles.overwrite=false." + newline + newline + 
                   "            Si la propri\u00e9t\u00e9 a la valeur false, les fichiers de trace et journal seront nomm\u00e9s de fa\u00e7on unique pour chaque session. Si les noms des fichiers de trace" + newline +
                   "            et journal par d\u00e9faut sont utilis\u00e9s, ils sont alors nomm\u00e9s de la fa\u00e7on suivante" + newline + newline +
                   "                  .plugin<username><date hash code>.trace" + newline +
                   "                  .plugin<username><date hash code>.log" + newline + newline +
                   "            Les fonctionnalit\u00e9s de tra\u00e7age et de consignation d\u00e9finies via le Panneau de configuration prennent effet au lancement du Plug-in, " + newline +
                   "            mais les modifications effectu\u00e9es via le Panneau de configuration pendant qu'un Plug-in est en cours d'ex\u00e9cution restent sans effet tant qu'un red\u00e9marrage n'a pas \u00e9t\u00e9 effectu\u00e9." + newline + newline + 
                   "            Pour plus d'informations sur les fonctions de tra\u00e7age et de consignation, consultez les chapitres correspondants dans le Guide du d\u00e9veloppeur de Java Plug-in." + newline + newline +
                   "      D\u00e9bogage des applets dans Java Plug-in" + newline + newline +
                   "            Les options suivantes sont utilis\u00e9es lors du d\u00e9bogage des applets dans le Java Plug-in." + newline +
                   "            Pour plus d'informations sur cette rubrique, consultez la rubrique de prise en charge du d\u00e9bogage dans le Guide du d\u00e9veloppeur de Java Plug-in." + newline + newline +
                   "                  -Djava.compiler=NONE" + newline +
                   "                  -Xnoagent" + newline +
                   "                  -Xdebug" + newline + 
                   "                  -Xrunjdwp:transport=dt_shmem,address=<connect-address>,server=y,suspend=n" + newline + newline +
                   "            La partie <adresse-connexion> peut \u00eatre n'importe quelle cha\u00eene (par exemple : 2502) utilis\u00e9e par le d\u00e9bogueur Java (jdb) ult\u00e9rieurement" + newline +
                   "            pour se connecter \u00e0 JVM." + newline + newline + 
                   "      D\u00e9lai de connexion par d\u00e9faut" + newline + newline +
                   "            Lorsqu'une connexion est \u00e9tablie par un applet sur un serveur et que celui-ci n'y r\u00e9pond pas correctement, l'applet peut" + newline +
                   "            sembler se bloquer et peut aussi provoquer le blocage du navigateur, car il n'existe pas de d\u00e9lai de connexion au r\u00e9seau " + newline +
                   "            (il a la valeur -1 par d\u00e9faut, ce qui signifie qu'aucun d\u00e9lai n'est d\u00e9fini)." + newline + newline +
                   "            Pour \u00e9viter ce probl\u00e8me, Java Plug-in a ajout\u00e9 une valeur de d\u00e9lai de connexion au r\u00e9seau par d\u00e9faut (2 minutes) pour toutes les connexions HTTP :" + newline +
                   "            Vous pouvez ignorer ce param\u00e8tre dans les param\u00e8tres de Java Runtime :" + newline + newline +
                   "                  -Dsun.net.client.defaultConnectTimeout=value in milliseconds" + newline + newline +
                   "            L'autre propri\u00e9t\u00e9 r\u00e9seau que vous pouvez d\u00e9finir est sun.net.client.defaultReadTimeout." + newline + newline +
                   "                  -Dsun.net.client.defaultReadTimeout=value in milliseconds" + newline + newline +
                   "                  Remarque" + newline + newline +
                   "                  Java Plug-in ne d\u00e9finit pas sun.net.client.defaultReadTimeout par d\u00e9faut. Si vous voulez la d\u00e9finir, utilisez" + newline +
                   "                  l'option Param\u00e8tres de Java Runtime comme indiqu\u00e9 ci-dessus." + newline + newline + newline +
                   "            Description des propri\u00e9t\u00e9s de gestion de r\u00e9seau :" + newline + newline +
                   "            sun.net.client.defaultConnectTimeout" + newline +
                   "            sun.net.client.defaultReadTimeout" + newline + newline +
                   "                  Ces propri\u00e9t\u00e9s sp\u00e9cifient, respectivement, les valeurs par d\u00e9faut des d\u00e9lais d'attente de lecture et de connexion pour les gestionnaires de protocoles" + newline +
                   "                  Les valeurs par d\u00e9faut d\u00e9finies par les gestionnaires de protocoles sont \u00e9gales \u00e0 -1, ce qui signifie" + newline +
                   "                  qu'aucun d\u00e9lai n'est d\u00e9fini." + newline + newline +
                   "                  sun.net.client.defaultConnectTimeout sp\u00e9cifie le d\u00e9lai (en millisecondes) pour \u00e9tablir la connexion avec l'h\u00f4te." + newline +
                   "                  Par exemple, pour les connexions http, il s'agit du d\u00e9lai lors de l'\u00e9tablissement de la connexion au serveur http." + newline +
                   "                  Pour les connexions ftp, il s'agit du d\u00e9lai lors de l'\u00e9tablissement de la connexion au serveur ftp." + newline + newline +
                   "                  sun.net.client.defaultReadTimeout sp\u00e9cifie le d\u00e9lai (en millisecondes) lors de la lecture d'un flux en entr\u00e9e" + newline +
                   "                  lorsqu'une connexion est \u00e9tablie avec une ressource." + newline + newline + 
                   "            Pour la description officielle de ces propri\u00e9t\u00e9s de gestion de r\u00e9seau," + newline +
                   "            consultez http://java.sun.com/j2se/1.4/docs/guide/net/properties.html." + newline + newline + newline +
                   "Navigateur" + newline + newline + newline +
                   "Cet onglet se rapporte seulement aux installations sur la plate-forme Microsoft Windows ; il n'appara\u00eet pas dans les autres installations. S\u00e9lectionnez le navigateur" + newline +
"pour lequel Java Plug-in doit \u00eatre le Java Runtime par d\u00e9faut, \u00e0 la place de la JVM interne du navigateur en question." + newline +
"Cette op\u00e9ration permet d'activer la prise en charge de la balise APPLET dans Internet Explorer et Netscape 6 via Java Plug-in." + newline + newline + newline +
                   " Proxies" + newline + newline +newline +
                   "Utilisez l'onglet Proxies pour appliquer les param\u00e8tres par d\u00e9faut du navigateur ou remplacer le port et l'adresse du proxy en fonction des protocoles employ\u00e9s." + newline + newline + 
                   "Utiliser les param\u00e8tres du navigateur" + newline + newline + 
                   "      S\u00e9lectionnez cette option pour utiliser les param\u00e8tres proxy par d\u00e9faut du navigateur. C'est le param\u00e8tre par d\u00e9faut (option s\u00e9lectionn\u00e9e)." + newline + newline +
                   "Tableau d'informations relatives au proxy" + newline + newline + 
                   "      Vous pouvez remplacer les param\u00e8tres par d\u00e9faut en d\u00e9sactivant la case \u00e0 cocher \"Utiliser les param\u00e8tres du navigateur\"puis en compl\u00e9tant le tableau d'informations" + newline +
                   "      du proxy qui se trouve sous la case \u00e0 cocher. Vous pouvez indiquer l'adresse et le port du proxy pour chaque protocole" + newline +
                   "      pris en charge : HTTP, Secure (HTTPS), FTP, Gopher, et Socks." + newline + newline + 
                   "Aucun h\u00f4te proxy" + newline + newline + 
                   "      Il s'agit de l'h\u00f4te ou de la liste d'h\u00f4tes pour lesquels aucun proxy ne doit \u00eatre utilis\u00e9. L'option Aucun h\u00f4te proxy est g\u00e9n\u00e9ralement utilis\u00e9e pour un " + newline +
                   "      h\u00f4te interne dans un environnement intranet." + newline + newline + 
                   "URL de configuration automatique du proxy" + newline + 
                   "      Il s'agit de l'URL du fichier JavaScript (extension .js ou .pac) contenant la fonction FindProxyForURL." + newline +
                   "      FindProxyForURL dispose de la logique lui permettant de d\u00e9terminer le serveur proxy \u00e0 utiliser pour une demande de connexion." + newline + newline + 
                   "Pour plus de d\u00e9tails sur la configuration du proxy, consultez le chapitre consacr\u00e9 \u00e0 la configuration du proxy dans le " + newline +
                   "Guide du d\u00e9veloppeur de Java Plug-in." + newline + newline + newline +
                   " Cache" + newline + newline + newline +
                   "           Remarque" + newline + newline +
                   "           Le cache auquel il est fait r\u00e9f\u00e9rence ici est le cache sticky c'est-\u00e0-dire le cache disque cr\u00e9\u00e9 et contr\u00f4l\u00e9 par le Java Plug-in " + newline +
                   "           que le navigateur ne peut pas remplacer. Pour plus d'informations, consultez la rubrique sur la mise en cache des applets dans le Guide du d\u00e9veloppeur de Java Plug-in." + newline + newline + newline +
                   "Activer la mise en cache" + newline + newline + 
                   "      Pour activer la mise en cache, cochez la case correspondante. C'est le param\u00e8tre par d\u00e9faut (option s\u00e9lectionn\u00e9e). Lorsque la fonction de mise en cache des applets est activ\u00e9e, les performances" + newline +
                   "      s'am\u00e9liorent car une fois l'applet en cache, il n'est plus n\u00e9cessaire de la t\u00e9l\u00e9charger lorsqu'elle est r\u00e9f\u00e9renc\u00e9e de nouveau." + newline + newline +
                   "      Java Plug-in met en cache les types de fichiers suivants t\u00e9l\u00e9charg\u00e9s via HTTP/HTTPS:" + newline + newline +
                   "            .jar (fichier jar)" + newline +
                   "            .zip (fichier zip)" + newline +
                   "            .class (fichier java class)" + newline +
                   "            .au (fichier audio)" + newline +
                   "            .wav (fichier audio)" + newline +
                   "            .jpg (fichier image)" + newline +
                   "            .gif (fichier image)" + newline + newline +
                   "Afficher les fichiers du cache" + newline + newline + 
                   "      Pour afficher les fichiers du cache, cliquez sur le bouton correspondant. Une autre bo\u00eete de dialogue (celle de l'afficheur du cache Java Plug-in) s'ouvre en affichant les fichiers du cache." + newline +
                   "      L'afficheur du cache affiche les informations suivantes sur les fichiers du cache : Nom, Type, Taille, Date d'expiration," + newline +
                   "      Derni\u00e8re modification, Version et URL. Dans l'afficheur du cache, vous pouvez aussi proc\u00e9der \u00e0 une suppression s\u00e9lective des fichiers du cache." + newline +
                   "      Cette m\u00e9thode est identique \u00e0 l'option Effacer le cache d\u00e9crite ci-dessous, qui supprime tous les fichiers du cache." + newline + newline +
                   "Effacer le Cache" + newline + newline + 
                   "      Pour effacer tous les fichiers qui se trouvent dans le cache, cliquez sur le bouton correspondant. Vous serez invit\u00e9 \u00e0 confirmer l'op\u00e9ration (Effacer tous les fichiers dans ... _le cache ?) avant la suppression des fichiers." + newline + newline + 
                   "Emplacement" + newline + newline + 
                   "      Cette option permet de sp\u00e9cifier l'emplacement du cache. L'emplacement par d\u00e9faut du cache est <accueil utilisateur>/.jpi_cache, o\u00f9" + newline +
                   "      <accueil utilisateur> est la valeur de la propri\u00e9t\u00e9 syst\u00e8me user.home. Sa valeur d\u00e9pend du syst\u00e8me d'exploitation." + newline + newline +
                   "Taille" + newline + newline + 
                   "      Vous pouvez s\u00e9lectionner l'option Illimit\u00e9e afin de donner au cache une taille illimit\u00e9e ou vous pouvez opter pour la taille Maximum du cache." + newline +
                   "      Si la taille du cache d\u00e9passe la limite sp\u00e9cifi\u00e9e, les fichiers les plus anciens mis en cache sont supprim\u00e9s jusqu'\u00e0 ce que le cache retrouve une taille " + newline +
                   "      se situant dans la limite permise." + newline + newline + 
                   "Compression" + newline + newline + 
                   "      Vous pouvez d\u00e9finir la compression des fichiers du cache JAR en utilisant la valeur Aucune ou Elev\u00e9e. Si vous pouvez \u00e9conomiser de la m\u00e9moire en" + newline +
                   "      sp\u00e9cifiant une compression plus \u00e9lev\u00e9e, les performances s'en ressentiront ; les meilleures performances sont obtenues" + newline +
                   "      sans compression." + newline + newline + newline + 
                   " Certificats" + newline + newline + newline +
                   "Quatre types de certificats peuvent \u00eatre s\u00e9lectionn\u00e9s :" + newline + newline +
                   "      Applet sign\u00e9" + newline + 
                   "      Site s\u00e9curis\u00e9" + newline + 
                   "      CA signataire " + newline +
                   "      CA site s\u00e9curis\u00e9" + newline + newline + 
                   "Applet sign\u00e9" + newline + newline +
                   "      Il s'agit des certificats des applets sign\u00e9s qui sont approuv\u00e9s par l'utilisateur. Les certificats qui apparaissent dans la" + newline +
                   "      liste des applets sign\u00e9s sont lus \u00e0 partir du fichier de certificat jpicerts<version> qui se trouve dans le r\u00e9pertoire <accueil utilisateur>/.java." + newline + newline +
                   "Site s\u00e9curis\u00e9" + newline + newline +
                   "      Il s'agit des certificats des sites s\u00e9curis\u00e9s. Les certificats qui apparaissent dans la liste des sites s\u00e9curis\u00e9s sont lus \u00e0 partir du fichier de certificat" + newline +
                   "      jpihttpscerts<version> qui se trouve dans le r\u00e9pertoire <accueil utilisateur>/.java." + newline + newline + 
                   "CA signataire " + newline + newline +
                   "      Il s'agit de certificats \u00e9mis par des autorit\u00e9s de certification (Certificate Authorities, CA) pour les applets sign\u00e9s ; ce sont ces autorit\u00e9s qui d\u00e9livrent les certificats" + newline +
                   "       aux signataires d'applets sign\u00e9s. Les certificats qui apparaissent dans la liste CA signataire sont lus \u00e0 partir du fichier" + newline +
                   "       de certificat cacerts, qui se trouve dans le r\u00e9pertoire <jre>/lib/security." + newline + newline +
                   "CA site s\u00e9curis\u00e9" + newline + newline +
                   "      Il s'agit de certificats \u00e9mis par les autorit\u00e9s de certification (Certificate Authorities, CA) pour les sites s\u00e9curis\u00e9s ; ce sont ces autorit\u00e9s qui d\u00e9livrent les certificats " + newline +
                   "      pour les sites s\u00e9curis\u00e9s. Les certificats qui apparaissent dans la liste CA signataire sont lus \u00e0 partir du fichier de certificat jssecacerts, qui se trouve dans le r\u00e9pertoire" + newline +
                   "      dans le r\u00e9pertoire <jre>/lib/security." + newline + newline +
                   "Pour les certificats de type Applet sign\u00e9 et Site s\u00e9curis\u00e9, il existe quatre options: Importer, Exporter, Supprimer et D\u00e9tails." + newline +
                   "L'utilisateur peut importer, exporter, supprimer et afficher les d\u00e9tails d'un certificat." + newline + newline + 
                   "Pour les certificats de type CA Signataire et CA site s\u00e9curis\u00e9, il n'existe qu'une seule option : D\u00e9tails. L'utilisateur peut seulement afficher les d\u00e9tails d'un certificat." + newline + newline + newline +
                   "Mise \u00e0 jour" + newline + newline + newline +
                   "Sous cet onglet, il existe un bouton \"Obtenez la mise \u00e0 jour de Java\" qui permet aux utilisateurs d'obtenir la derni\u00e8re mise \u00e0 jour disponible pour Java Runtime" + newline +
"Environment \u00e0 partir du site Web Java Update. Cet onglet se rapporte seulement \u00e0 la plate-forme Microsoft Windows ; il n'appara\u00eet pas dans" + newline +
"les autres plates-formes (\u00e0 savoir Solaris/Linux)."}
};
} 

 



