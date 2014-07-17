/*
 * @(#)ConverterHelp_es.java	1.2 03/04/24
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * Spanish version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_es extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");;

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Archivo L\u00e9ame del conversor de HTML de Java(tm) Plug-in" + newline + newline +
      "Versi\u00f3n:  1.4.2" + newline + newline + newline +
      "*****   REALICE UNA COPIA DE SEGURIDAD DE TODOS LOS ARCHIVOS ANTES DE CONVERTIRLOS CON ESTA HERRAMIENTA" + newline +
      "*****   LA CANCELACI\u00d3N DE LA CONVERSI\u00d3N NO REDUCIR\u00c1 LOS CAMBIOS." + newline +
      "*****   LOS COMENTARIOS DE LA INSTRUCCI\u00d3N APPLET SE IGNORAN" + newline + newline + newline +
      "Contenido:" + newline +
      "   1.  Funciones nuevas" + newline +
      "   2.  Correcci\u00f3n de errores" + newline +
      "   3.  Acerca del conversor de HMTL de Java(tm) Plug-in" + newline +
      "   4.  El proceso de conversi\u00f3n" + newline +
      "   5.  Elecci\u00f3n en las carpetas de los archivos que se van a convertir" + newline +
      "   6.  Elecci\u00f3n de la carpeta para copias de seguridad" + newline +
      "   7.  Generaci\u00f3n de un archivo de registro" + newline +
      "   8.  Elecci\u00f3n de la plantilla de conversi\u00f3n" + newline +
      "   9.  Conversi\u00f3n" + newline +
      "  10.  M\u00e1s conversiones o salir" + newline +
      "  11.  Detalles sobre las plantillas" + newline +
      "  12.  Ejecuci\u00f3n del conversor de HTML (Windows y Solaris)" + newline + newline +
      "1)  Funciones nuevas:" + newline + newline +     
      "    o Actualizaci\u00f3n de plantillas ampliadas para compatibilizarlas con Netscape 6." + newline +
      "    o Actualizaci\u00f3n de todas las plantillas para compatibilizarlas con las nuevas funciones multiversi\u00f3n de Java Plug-in." + newline +
      "    o Mejora de la interfaz de usuario con Swing 1.1 para compatibilizarla con i18n." + newline +
      "    o Mejora del cuadro de di\u00e1logo Opciones avanzadas para poder utilizar las nuevas instrucciones" + newline +
      "      de plantilla SmartUpdate y MimeType." + newline +
      "    o Mejora del conversor de HTML que se va a utilizar con Java Plug-in 1.1.x," + newline +
      "      Java Plug-in 1.2.x , Java Plug-in 1.3.x y Java Plug-in" + newline +
      "      1.4.x." + newline +
      "    o Mejora de la compatibilidad con SmartUpdate y MimeType en todas las plantillas de" + newline +
      "      conversi\u00f3n." + newline +
      "    o Incorporaci\u00f3n de \"scriptable=false\" a la instrucci\u00f3n OBJECT/EMBED en todas las plantillas." + newline + newline + 
      "     Esto se utiliza para desactivar la generaci\u00f3n de typelib cuando Java" + newline + newline +
      "    Plug-in no se utiliza para generar scripts." + newline + newline + newline +
      "2)  Correcci\u00f3n de errores:" + newline + newline +
      "    o Mejora en la gesti\u00f3n de errores cuando no se encuentran los archivos de propiedades." + newline +
      "    o Mejora en la conversi\u00f3n de HTML, con lo que el resultado de la instrucci\u00f3n EMBED/OBJECT" + newline +
      "      se puede utilizar en la aplicaci\u00f3n AppletViewer de JDK 1.2.x." + newline +
      "    o Eliminaci\u00f3n de los archivos innecesarios que sobraron del conversor de HTML 1.1.x." + newline +
      "    o Generaci\u00f3n de EMBED/OBJECT con los nombres de atributos CODE, CODEBASE, etc." + newline +
      "      en lugar de JAVA_CODE, JAVA_CODEBASE, etc. \u00c9sta es la p\u00e1gina" + newline +
      "      generada que se va a utilizar en la aplicaci\u00f3n AppletViewer de JDK 1.2.x." + newline +
      "    o Compatibilidad con la conversi\u00f3n MAYSCRIPT, siempre que \u00e9sta se encuentre en la" + newline +
      "      instrucci\u00f3n APPLET." + newline + newline +
      "3)  Acerca del conversor de HMTL de Java(tm) Plug-in:" + newline + newline +
      "        El conversor de HMTL de Java(tm) Plug-in es una utilidad que permite convertir" + newline +
      "        cualquier p\u00e1gina HTML que contenga applets a un formato que utilizar\u00e1 Java(tm)" + newline +
      "        Plug-in." + newline + newline +       
      "4)  El proceso de conversi\u00f3n:" + newline + newline +
      "        El conversor de HMTL de Java(tm) Plug-in convertir\u00e1 cualquier archivo que contenga" + newline +
      "        applets a un formato que pueda utilizarse con Java(tm) Plug-in." + newline + newline +
      "        \u00c9ste es el proceso de conversi\u00f3n de los archivos:" + newline +
      "        En primer lugar, el HTML que no forma parte de ning\u00fan applet se transfiere desde el archivo" + newline +
      "        fuente a un archivo temporal.  Al llegar a una instrucci\u00f3n <APPLET, el conversor" + newline +
      "        analiza el applet con la primera instrucci\u00f3n </APPLET (que no est\u00e9 entre comillas)," + newline +
      "        y fusiona los datos del applet con la plantilla. (V\u00e9ase m\u00e1s abajo el apartado Detalles sobre las" + newline +
      "        plantillas). Si no se produce ning\u00fan error en este proceso, el archivo html original se mover\u00e1" + newline +
      "        a la carpeta para copias de seguridad y se asignar\u00e1 al archivo temporal el nombre" + newline +
      "        del archivo original.  Por consiguiente, los archivos originales nunca se eliminar\u00e1n del disco." + newline + newline +
      "        Observe que el conversor realmente convertir\u00e1 los archivos existentes.  Por tanto," + newline +
      "        una vez que haya ejecutado el conversor, los archivos estar\u00e1n preparados para utilizar Java(tm) Plug-in." + newline +
       

      "5)  Elecci\u00f3n en las carpetas de los archivos que se van a convertir:" + newline + newline +
      "       Para convertir todos los archivos de una carpeta puede elegir entre escribir la ruta de acceso a la carpeta" + newline +
      "       o utilizar el bot\u00f3n Examinar para seleccionar una carpeta en un cuadro de di\u00e1logo." + newline  +
      "       Tras elegir la ruta de acceso, puede incluir cualquier n\u00famero en los especificadores de archivos de" + newline +
      "       \"Nombres de archivo coincidentes\".  Los especificadores deben estar separados por comas.  Puede utilizar * como" + newline +
      "       comod\u00edn.  Si escribe un nombre de archivo con comod\u00edn, s\u00f3lo se convertir\u00e1" + newline +
      "       dicho archivo. Por \u00faltimo, active la casilla de verificaci\u00f3n \"Incluir subcarpetas\" si desea que" + newline +
      "       se conviertan todos los archivos de las carpetas anidadas cuyos nombres coincidan con el nombre del archivo." + newline + newline +      
      "6)  Elecci\u00f3n de la carpeta para copias de seguridad:" + newline +

      "       La ruta de acceso de la carpeta para copias de seguridad predeterminada es la ruta de origen con el sufijo \"_BAK\"" + newline +
      "       en el nombre. Dicho de otra forma:  si la ruta de origen es c:/html/applet.html (conversi\u00f3n de un archivo)," + newline +
      "       la ruta de acceso de la carpeta para copias de seguridad ser\u00eda c:/html_BAK.  Si la ruta de origen" + newline +
      "       es c:/html (conversi\u00f3n de todos los archivos de la ruta de acceso), la ruta de acceso de la carpeta para copias de seguridad ser\u00eda" + newline +
      "       c:/html_BAK. Esta ruta de acceso puede cambiarse escribiendo otra ruta en el campo que est\u00e1" + newline +
      "       junto a \"Copia de seguridad de archivos en carpeta:\" o buscando una carpeta." + newline + newline +

      "       Unix(Solaris):" + newline +
      "       La ruta de acceso de la carpeta para copias de seguridad predeterminada es la ruta de origen con el sufijo \"_BAK\" en" + newline +
      "       el nombre. Dicho de otra forma:  si la ruta de origen es /home/user1/html/applet.html (conversi\u00f3n de un archivo)" + newline +
      "       la ruta de acceso de la carpeta para copias de seguridad ser\u00eda /home/user1/html_BAK. Si la ruta de" + newline +
      "       origen es /home/user1/html (conversi\u00f3n de todos los archivos de la ruta de acceso), la ruta de acceso de la carpeta para copias de seguridad" + newline +
      "       ser\u00eda /home/user1/html_BAK. Esta ruta de acceso de la carpeta para copias de seguridad puede cambiarse escribiendo" + newline +
      "       otra ruta en el campo que se encuentra junto a \"Copia de seguridad de archivos en carpeta:\" o buscando una carpeta." + newline + newline +      
      "7)  Generaci\u00f3n de un archivo de registro:" + newline + newline +
      "       Si desea que se genere un archivo de registro, active la casilla de verificaci\u00f3n" + newline +
      "       \"Generar archivo de registro\". Puede escribir la ruta de acceso y el nombre de archivo o utilizar el bot\u00f3n Examinar" + newline +
      "       para seleccionar una carpeta, a continuaci\u00f3n escribir el nombre de archivo y, finalmente, seleccionar Abrir." + newline +
      "       El archivo de registro contiene informaci\u00f3n b\u00e1sica relacionada con el proceso de" + newline +
      "       conversi\u00f3n." + newline + newline +        
      "8)  Elecci\u00f3n de la plantilla de conversi\u00f3n:" + newline + newline +
      "       Si no se elige ninguna plantilla, se utilizar\u00e1 alguna de las predeterminadas.  Esta plantilla" + newline +
      "       generar\u00e1 archivos html convertidos que funcionar\u00e1n con Internet Explorer y Netscape." + newline  +
      "       Si desea utilizar otra plantilla, puede elegirla en el men\u00fa o de" + newline +
      "       la pantalla principal.  Si elige Otra, podr\u00e1 seleccionar el archivo" + newline +
      "       que se utilizar\u00e1 como plantilla." + newline +
      "       Si elige un archivo, ASEG\u00daRESE DE QUE ES UNA PLANTILLA." + newline + newline +
      "9)  Conversi\u00f3n:" + newline + newline +
      "       Para iniciar el proceso de conversi\u00f3n, haga clic en el bot\u00f3n \"Convertir...\".  Aparecer\u00e1 un" + newline +
      "       cuadro de di\u00e1logo del proceso que muestra los archivos que se est\u00e1n procesando, el n\u00famero del proceso de archivos," + newline +
      "       el n\u00famero de applets encontrados y el n\u00famero de errores encontrados." + newline + newline +       
      "10) M\u00e1s conversiones o salir:" + newline + newline +
      "       Cuando la conversi\u00f3n haya terminado, el bot\u00f3n del cuadro de di\u00e1logo del proceso cambiar\u00e1" + newline +
      "       de \"Cancelar\" a \"Listo\".  Al seleccionar\"Listo\", se cierra el cuadro de di\u00e1logo." + newline  +
      "       En este punto, puede elegir \"Salir\" para cerrar el conversor de HTML de Java(tm) Plug-in," + newline +
      "       o seleccionar otro conjunto de archivos que desee convertir y volver a elegir \"Convertir...\"." + newline + newline +       
      "11)  Detalles sobre las plantillas:" + newline + newline +
      "       El archivo de plantillas es la base de la conversi\u00f3n de applets.  No es m\u00e1s que un archivo" + newline +
      "       de texto que contiene instrucciones que representan partes del applet original." + newline +
      "       Si a\u00f1ade, elimina o mueve las instrucciones de un archivo de plantillas, puede alterar la salida" + newline +
      "       del archivo convertido." + newline + newline +
      "       Instrucciones admitidas:" + newline + newline +
      "        $OriginalApplet$    Esta instrucci\u00f3n se sustituye por el texto completo" + newline +
      "        del applet original." + newline + newline +
      "        $AppletAttributes$   Esta instrucci\u00f3n se sustituye por todos los" + newline +
      "        atributos de los applets (code, codebase, width, height, etc.)" + newline + newline +
      "        $ObjectAttributes$   Esta instrucci\u00f3n se sustituye por todos los" + newline +
      "        atributos que requiere la instrucci\u00f3n object." + newline + newline +
      "        $EmbedAttributes$   Esta instrucci\u00f3n se sustituye por todos los atributos" + newline +
      "        que requiere la instrucci\u00f3n embed." + newline + newline +
      "        $AppletParams$   Esta instrucci\u00f3n se sustituye por todas las instrucciones" + newline +
      "        <param ...> del applet" + newline + newline +
      "        $ObjectParams$    Esta instrucci\u00f3n se sustituye por todas las instrucciones" + newline +
      "        <param...> que requiere la instrucci\u00f3n object." + newline + newline +
      "        $EmbedParams$     Esta instrucci\u00f3n se sustituye por todas las instrucciones" + newline +
      "        <param...> que requiere la instrucci\u00f3n embed en forma de NAME=VALUE" + newline + newline +
      "         $AlternateHTML$  Esta instrucci\u00f3n se sustituye por el texto del \u00e1rea No" + newline +
      "        support for applets del applet original" + newline + newline +
      "        $CabFileLocation$   Es la URL del archivo cab que deber\u00eda utilizarse" + newline +
      "        en todas las plantillas destinadas a Internet Explorer." + newline + newline +
      "        $NSFileLocation$    Es la URL del plugin de Netscape que deber\u00eda utilizarse" + newline +
      "        en todas las plantillas destinadas a Netscape." + newline + newline +
      "        $SmartUpdate$   Es la URL del plugin de la instrucci\u00f3n SmartUpdate de Netscape" + newline +
      "        que deber\u00eda utilizarse en todas las plantillas destinadas a Netscape Navigator 4.0 o superior." + newline + newline +
      "        $MimeType$    Es el tipo MIME del objeto de Java" + newline + newline +     

      "      default.tpl es la plantilla predeterminada del conversor. La p\u00e1gina convertida puede" + newline +
      "      utilizarse en Internet Explorer y en Netscape Navigator en Windows para llamar a Java(TM) Plug-in." + newline +
      "      Esta plantilla tambi\u00e9n puede utilizarse con Netscape en Unix(Solaris)" + newline + newline +
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
      "      ieonly.tpl: La p\u00e1gina convertida se puede utilizar para llamar a Java(TM)" + newline +
      "      Plug-in en Internet Explorer solamente en Windows." + newline + newline +
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
      "      nsonly.tpl: La p\u00e1gina convertida se puede utilizar para llamar a Java(TM)" + newline +
      "      Plug-in en Netscape Navigator en Windows y Solaris." + newline + newline +
      "      <!-- HTML CONVERTER -->" + newline +
      "      <EMBED type=\"$MimeType$\" $EmbedAttributes$" + newline +
      "      $EmbedParams$ scriptable=false" + newline +
      "      pluginspage=\"$NSFileLocation$\"><NOEMBED>" + newline +
      "      $AlternateHTML$" + newline +
      "      </NOEMBED></EMBED>" + newline + newline +
      "      <!--" + newline +
      "      $ORIGINALAPPLET$" + newline +
      "      -->" + newline + newline +       
      "      extend.tpl: La p\u00e1gina convertida se puede utilizar en cualquier navegador y cualquier plataforma." + newline +
      "      Si el navegador es Internet Explorer o Netscape Navigator en Windows (Netscape Navigator en Solaris), se llamar\u00e1 a" + newline +
      "      Java(TM) Plug-in. En caso contrario se utiliza la JVM predeterminada del navegador." + newline + newline +
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
      "12)  Ejecuci\u00f3n del conversor de HTML:" + newline + newline +
      "      Ejecuci\u00f3n de la versi\u00f3n GUI del conversor de HTML" + newline + newline +
      "      El conversor de HTML se incluye en el SDK, no en el JRE. Para ejecutarlo, dir\u00edjase al" + newline +
      "      subdirectorio lib del directorio de instalaci\u00f3n de SDK. Por ejemplo," + newline +
      "      si ha instalado el SDK en Windows en la unidad C, dir\u00edjase a" + newline + newline +
      "            C:\\j2sdk1.4.2\\lib\\" + newline + newline +
      "      El conversor (htmlconverter.jar) se encuentra en ese directorio." + newline + newline +
      "      Para iniciar el tipo de conversor:" + newline + newline +
      "            C:\\j2sdk1.4.2\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      En UNIX/Linux se inicia de la misma forma con los comandos anteriores." + newline +
      "      A continuaci\u00f3n encontrar\u00e1 otras formas de iniciar el conversor" + newline + newline +
      "      En Windows" + newline +
      "      Para iniciar el conversor con el Explorador:" + newline +
      "      Utilice el Explorador para desplazarse al siguiente directorio:" + newline + newline +
      "      C:\\j2sdk1.4.2\\bin" + newline + newline +
      "      Haga doble clic en la aplicaci\u00f3n HtmlConverter." + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      Ejecute los siguientes comandos:" + newline + newline +
      "      cd /j2sdk1.4.2/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +             
      "      Uso del conversor desde la l\u00ednea de comandos:" + newline + newline +
      "      Formato:" + newline + newline +
      "      java -jar htmlconverter.jar [-opciones1 valor1 [-opci\u00f3n2 valor2" + newline +
      "      [...]]] [-simulate] [filespecs]" + newline + newline +
      "      especificaci\u00f3n de los archivos:  lista delimitada por espacios de las especificaciones de los archivos, el asterisco (*) es el comod\u00edn. " + newline +
      "      (*.html *.htm)" + newline + newline +
      "      Opciones:" + newline + newline +
      "       fuente:    Ruta de acceso a los archivos.  (c:\\htmldocs en Windows," + newline +
      "                  /home/user1/htmldocs en Unix) Valor predeterminado: <userdir>" + newline +
      "                  Si la ruta de acceso es relativa, se supone que lo es con respecto al" + newline +
      "                  directorio desde el que se inici\u00f3 HTMLConverter." + newline + newline +
      "       copia de seguridad:    Ruta de acceso para escribir archivos de copia de seguridad.  Valor predeterminado:" + newline +
      "                  <userdir>/<source>_bak" + newline +
      "                  Si la ruta de acceso es relativa, se supone que lo es con respecto al" + newline +
      "                  directorio desde el que se inici\u00f3 HTMLConverter." + newline + newline +
      "       subdirectorios:   Si se procesan los archivos de los subdirectorios. " + newline +
      "                  Valor predeterminado:  FALSE" + newline + newline +
      "       plantilla:  Nombre del archivo de plantillas.  Valor predeterminado:  default.tpl-Est\u00e1ndar " + newline +
      "                  (Internet Explorer y Netscape Navigator) s\u00f3lo para Windows y Solaris. UTILICE EL VALOR PREDETERMINADO SI NO EST\u00c1 SEGURO." + newline + newline +
      "       registro:       Ruta de acceso y nombre del archivo en que se escribe el registro.  (predeterminado: <userdir>/convert.log)" + newline + newline +
      "       progreso:  Muestra el progreso de salida est\u00e1ndar durante el proceso de conversi\u00f3n. " + newline +
      "                  Valor predeterminado: false" + newline + newline +
      "       simulaci\u00f3n:  Muestra los valores espec\u00edficos de la conversi\u00f3n sin realizarla." + newline +
      "                  UTILICE ESTA OPCI\u00d3N SI NO DOMINA LA CONVERSI\u00d3N." + newline +
      "                  APARECER\u00c1 UNA LISTA DE DETALLES ESPEC\u00cdFICOS DE" + newline +
      "                  LA CONVERSI\u00d3N." + newline + newline +
      "      Si s\u00f3lo se especifica \"java -jar htmlconverter.jar -gui\" (s\u00f3lo la opci\u00f3n" + newline +
      "      -gui sin especificaciones de archivos), se iniciar\u00e1 la versi\u00f3n GUI del conversor." + newline  +
      "      En caso contrario, se suprimir\u00e1 esta versi\u00f3n." + newline + newline +
      "      Para obtener m\u00e1s informaci\u00f3n, v\u00e9ase la siguiente url:" + newline + newline +
      "      http://java.sun.com/j2se/1.4.2/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
} 

 


