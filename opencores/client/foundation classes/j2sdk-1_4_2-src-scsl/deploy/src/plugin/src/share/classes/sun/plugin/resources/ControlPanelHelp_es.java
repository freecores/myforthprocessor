/*
 * @(#)ControlPanelHelp_es.java	1.3 03/04/28
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

public class ControlPanelHelp_es extends ListResourceBundle {

    private static String newline = "\n";

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "help.file", newline +
                   "Ayuda del Panel de Control de Java Plug-in" + newline + newline + newline +
                   "Esta ayuda incluye los siguientes temas:" + newline +
                   "      Generalidades" + newline +
                   "      Opciones de Guardar" + newline +
                   "      Definir las opciones del Panel de Control" + newline +
                   "      Panel B\u00e1sico" + newline +
                   "      Panel Avanzado" + newline +
                   "      Panel Navegador" + newline +
                   "      Panel Proxies" + newline +
                   "      Panel Antememoria" + newline +
                   "      Panel Certificados" + newline +
                   "      Panel Actualizar" + newline + newline +
                   "Generalidades" + newline + newline +
                   "El Panel de Control de Java Plug-in permite cambiar los valores predeterminados que se utilizan al iniciar Java Plug-in." + newline +
                   "Todos los subprogramas que se ejecuten en una sesi\u00f3n activa de Java Plug-in utilizar\u00e1n estos valores." + newline +
                   "La Gu\u00eda del Desarrollador de Java Plug-in, mencionada en este documento, se encuentra en (URL sujeta a cambios)." + newline +
                   "http://java.sun.com/j2se/1.4/docs/guide/plugin/developer_guide/contents.html" + newline + newline +
                   "Opciones de guardar" + newline + newline +
                   "Una vez que haya terminado de introducir cambios en las opciones del Panel de Control, haga clic en Aplicar para guardar los cambios." + newline +
                   "Haga clic en Restablecer para cancelar los cambios y cargar de nuevo los \u00faltimos valores que se introdujeron y aplicaron." + newline +
                   "Tenga en cuenta que estos valores no son los mismos que los valores predeterminados que se establecen al instalar Java Plug-in." + newline + newline +
                   "Definir las opciones del Panel de Control" + newline + newline +
                   "En el Panel de Control de Java Plug-in hay seis paneles en los que pueden definirse varias opciones." + newline +
                   "Estos paneles son los siguientes: " + newline +
                   "      B\u00e1sico" + newline +
                   "      Avanzado" + newline +
                   "      Navegador" + newline +
                   "      Proxies" + newline +
                   "      Antememoria" + newline +
                   "      Certificados" + newline +
                   "      Actualizar" + newline + newline +
                   "A continuaci\u00f3n se describe cada uno de estos paneles." + newline + newline + newline +
                   " B\u00e1sico" + newline +
                   "Mostrar consola de Java" + newline + newline +
                   "      Muestra la consola de Java cuando se ejecutan subprogramas. La consola muestra los mensajes emitidos por System.out y System.err." + newline +
                   "      Es \u00fatil para resolver problemas." + newline + newline +
                   "Ocultar consola" + newline + newline +
                   "      La consola de Java se est\u00e1 ejecutando pero permancece oculta. Esta opci\u00f3n est\u00e1 activada de manera predeterminada." + newline + newline +
                   "No iniciar consola" + newline + newline +
                   "      La consola de Java no se inicia." + newline + newline +
                   "Cuadro de di\u00e1logo Mostrar excepci\u00f3n" + newline + newline +
                   "      Muestra un cuadro de di\u00e1logo cuando ocurren excepciones. Este cuadro de di\u00e1logo no se muestra de forma predeterminada (est\u00e1 desactivado)." + newline + newline +
                   "Mostrar Java en la barra de tareas (s\u00f3lo Windows)" + newline + newline +
                   "      Cuando esta opci\u00f3n est\u00e1 activada el logotipo de Java se muestra en la barra de tareas cuando se inicia Java Plug-in" + newline +
                   "      y desaparece de la barra de tareas cuando se cierra Java Plug-in." + newline +
                   "      El logotipo de Java indica al usuario que una m\u00e1quina virtual de Java (Java VM) se est\u00e1 ejecutando; adem\u00e1s, proporciona informaci\u00f3n sobre la" + newline +
                   "      versi\u00f3n de Java y control sobre la consola de Java." + newline +
                   "      El valor predeterminado de esta casilla es activado." + newline + newline +
                   "      Funcionalidad de la barra de tareas de Java:" + newline + newline +
                   "      Cuando el puntero se sit\u00faa sobre el logotipo de Java, el texto \"Java\" aparece." + newline + newline +

                   "      Si hace doble clic en el icono de Java de la barra de tareas, aparecer\u00e1 la ventana de la consola de Java." + newline + newline +
                   "      Si hace clic con el bot\u00f3n derecho en el icono de Java de la barra de tareas, aparecer\u00e1 un men\u00fa emergente con las siguientes opciones:" + newline + newline +
                   "            Abrir/Cerrar Consola" + newline +
                   "            Acerca de Java" + newline +
                   "            Desactivar" + newline +
                   "            Salir" + newline + newline +
                   "      Abrir/Cerrar Consola abre/cierra la ventana de la consola de Java. Este men\u00fa incluir\u00e1 la opci\u00f3n Abrir Consola si la consola" + newline +
                   "      est\u00e1 oculta y Cerrar Consola si la consola se est\u00e1 mostrando." + newline + newline +
                   "      La opci\u00f3n Acerca de Java permite mostrar el cuadro de di\u00e1logo Acerca de... de Java 2 Standard Edition." + newline + newline +
                   "      Desactivar desactiva y quita el icono de Java de la barra de tareas para la sesi\u00f3n en curso y las siguientes. De este modo, si se reinicia Java Plug-in" + newline +
                   "      el icono de Java ya no aparecer\u00e1 en la barra de tareas." + newline +
                   "      Para que el icono de Java aparezca de nuevo en la barra de tareas, tras haber sido desactivado, consulte la nota de abajo." + newline + newline +
                   "      Salir suprime el icono de Java de la barra de tareas, pero s\u00f3lo durante la sesi\u00f3n en curso. Si se reinicia Java Plug-in, el" + newline +
                   "      icono de Java aparecer\u00e1 de nuevo en la barra de tareas." + newline + newline + newline +
                   "                Notas" + newline + newline +
                   "                1. Si \"Mostrar Java en barra de tareas\" est\u00e1 activado, el icono de Java aparecer\u00e1 en la barra de tareas aunque \"No iniciar" + newline +
                   "                consola\" est\u00e9 seleccionado." + newline + newline +
                   "                2. Para activar el icono de Java tras ser desactivado, inicie el Panel de Control de Java Plug-in, active \"Mostrar Java" + newline +
                   "                en la barra de tareas\", y presione \"Aplicar\"." + newline + newline +
                   "                3. Si se est\u00e1 ejecutando alguna otra m\u00e1quina virtual de Java (Java VM) y se han agregado otros iconos de Java a la barra de tareas, los cambios" + newline +
                   "                de configuraci\u00f3n en el Panel de Control de Java Plug-in no afectar\u00e1n a estos iconos." + newline +
                   "                La configuraci\u00f3n s\u00f3lo afectar\u00e1 al comportamiento del icono de Java si la m\u00e1quina virtual de Java (Java VM) se inicia despu\u00e9s." + newline + newline + newline +
                   " Avanzado" + newline + newline + newline +
                   "Java Run Time Environment" + newline + newline +
                   "      Permite ejecutar Java Plug-in con cualquier versi\u00f3n Java 2 JRE o SDK, Standard Edition v 1.3 o 1.4 que haya instalada en su equipo." + newline +
                   "      Java Plug-in 1.3/1.4 se suministra con un JRE predeterminado." + newline +
                   "      Sin embargo, se puede sobreescribir este JRE predeterminado y utilizar una versi\u00f3n anterior o posterior. El Panel de Control detecta autom\u00e1ticamente" + newline +
                   "      todas las versiones de Java 2 SDK o JRE que haya instaladas en el equipo. En el cuadro de texto se muestran todas las versiones que" + newline +
                   "      pueden utilizarse." + newline +
                   "      El primer elemento de la lista ser\u00e1 siempre el JRE predeterminado; el \u00faltimo ser\u00e1 siempre Otros. Si elige Otros, deber\u00e1" + newline +
                   "      especificar la ruta de acceso a Java 2 JRE o SDK, Standard Edition v 1.3/1.4." + newline + newline +
                   "                Nota" + newline + newline +
                   "                Esta opci\u00f3n s\u00f3lo deben modificarla los usuarios avanzados. No se recomienda cambiar el JRE predeterminado." + newline + newline + newline +
                   "Par\u00e1metros de Runtime de Java" + newline + newline +
                   "      Con esta opci\u00f3n se sobreescriben los par\u00e1metros iniciales predeterminados de Java Plug-in mediante la especificaci\u00f3n de opciones personalizadas. La sintaxis es igual que la de los" + newline +
                   "      par\u00e1metros que se utilizan para invocar la l\u00ednea de comandos. Consulte la documentaci\u00f3n de Java 2 Standard Edition (J2SE)," + newline +
                   "      donde figura una lista completa de opciones de la l\u00ednea de comandos." + newline +
                   "      La URL incluida a continuaci\u00f3n est\u00e1 sujeta a cambios:" + newline + newline +
                   "            http://java.sun.com/j2se/1.4/docs/tooldocs/<platform>/java.html" + newline + newline +
                   "            donde <platforma> es uno de los sistemas operativos: solaris, linux, win32." + newline + newline +
                   "      A continuaci\u00f3n se incluyen varios ejemplos de par\u00e1metros del runtime de Java." + newline + newline +
                   "      Activar y desactivar soporte de confirmaci\u00f3n" + newline + newline +
                   "            Para activar el soporte de confirmaci\u00f3n, debe especificarse la siguiente propiedad del sistema en Par\u00e1metros del Runtime de Java:" + newline + newline +
                   "                  -D[ enableassertions | ea ][:<nombre de paquete>\"...\" | : <nombre de clase> ]" + newline + newline +
                   "            Para desactivar la confirmaci\u00f3n en Java Plug-in, debe especificarse lo siguiente en Par\u00e1metros del Runtime de Java:" + newline + newline +
                   "                  -D[ disableassertions | da ][:<nombre de paquete>\"...\" | : <nombre de clase> ]" + newline + newline +
                   "            Consulte Facilidad de confirmaci\u00f3n para obtener m\u00e1s informaci\u00f3n sobre activaci\u00f3n/desactivaci\u00f3n de confirmaciones." + newline +
                   "            http://java.sun.com/j2se/1.4/docs/guide/lang/assert.html (URL sujeta a cambios)." + newline + newline +
                   "            De forma predeterminada, la Facilidad de Confirmaci\u00f3n est\u00e1 desactivada en Java Plug-in. Dado que el efecto de la confirmaci\u00f3n se determina al iniciarse Java Plug-in," + newline +
                   "            para modificar los valores de confirmaci\u00f3n en el Panel de Control de Java Plug-in ser\u00e1 necesario reiniciar un navegador para que entren en vigor" + newline +
                   "            los nuevos valores." + newline + newline +
                   "            Puesto que el c\u00f3digo de Java en Java Plug-in tambi\u00e9n tiene una facilidad de confirmaci\u00f3n incorporada, es posible activar la confirmaci\u00f3n en" + newline +
                   "            el c\u00f3digo de Java Plug-in haciendo lo siguiente:" + newline + newline +
                   "                  -D[ enableassertions | ea ]:sun.plugin" + newline + newline +
                   "      Soporte de rastreo y registro" + newline + newline +
                   "            La facilidad de rastreo permite redireccionar una salida de la consola de Java a un archivo de rastreo (.plugin<versi\u00f3n>.trace)." + newline + newline +
                   "                  -Djavaplugin.trace=true" + newline +
                   "                  -Djavaplugin.trace.option=basic|net|security|ext|liveconnect" + newline + newline +
                   "            Si no desea utilizar el nombre predeterminado del archivo de rastreo:" + newline + newline +
                   "                  -Djavaplugin.trace.filename=<nombre de archivo de rastreo>" + newline + newline +
                   "            A semejanza de la facilidad de rastreo, la de registro permite redireccionar una salida de la consola de Java a un archivo de registro (.plugin<versi\u00f3n>.log)" + newline +
                   "            mediante API Java Logging." + newline +
                   "            La facilidad de registro se puede activar habilitando la propiedad javaplugin.logging." + newline + newline +
                   "                  -Djavaplugin.logging=true" + newline + newline +
                   "            Si no desea utilizar el nombre predeterminado del archivo de registro:" + newline + newline +
                   "                  -Djavaplugin.log.filename=<nombre de archivo de registro>" + newline + newline +
                   "            Igualmente, si no desea sobreescribir los archivos de rastreo y de registro en cada sesi\u00f3n, puede establecer la propiedad:" + newline + newline +
                   "                  -Djavaplugin.outputfiles.overwrite=false." + newline + newline +
                   "            Si la propiedad se establece en el valor false, los nombres de los archivos de rastreo y de registro ser\u00e1n \u00fanicos para cada sesi\u00f3n. Si se utilizan los nombres predeterminados" + newline +
                   "            de los archivos de rastreo y de registro, los nombres de los archivos ser\u00e1n los siguientes:" + newline + newline +
                   "                  .plugin<nombre de usuario><date hash code>.trace" + newline +
                   "                  .plugin<nombre de usuario><date hash code>.log" + newline + newline +
                   "            Las facilidades de rastreo y registro establecidas en el Panel de Control entrar\u00e1n en vigor al iniciar el Plug-in, pero los cambios" + newline +
                   "            introducidos en el Panel de Control mientras ejecuta un Plug-in no entrar\u00e1n en vigor hasta que \u00e9ste no se reinicie." + newline + newline +
                   "            Para obtener m\u00e1s informaci\u00f3n sobre las facilidades de rastreo y registro, v\u00e9ase Rastreo y Registro en la Gu\u00eda del Desarrollador de Java Plug-in." + newline + newline +
                   "      Depurar subprogramas en Java Plug-in" + newline + newline +
                   "            Las siguientes opciones se utilizan para depurar subprogramas en Java Plug-in." + newline +
                   "            Para obtener m\u00e1s informaci\u00f3n sobre este tema consulte Soporte de depuraci\u00f3n en la Gu\u00eda del Desarrollador de Java Plug-in." + newline + newline +
                   "                  -Djava.compiler=NONE" + newline +
                   "                  -Xnoagent" + newline +
                   "                  -Xdebug" + newline +
                   "                  -Xrunjdwp:transport=dt_shmem,address=<direcci\u00f3n-conexi\u00f3n>,server=y,suspend=n" + newline + newline +
                   "            <direcci\u00f3n-conexi\u00f3n> puede ser cualquier cadena (ejemplo: 2502) que utilizar\u00e1 Java Debugger (jdb) posteriormente" + newline +
                   "            para conectarse al JVM." + newline + newline +
                   "      Temporizaci\u00f3n predeterminada" + newline + newline +
                   "            Cuando un subprograma se conecta a un servidor y el servidor no responde correctamente, el subprograma puede quedar" + newline +
                   "            aparentemente bloqueado y tambi\u00e9n puede dejar bloqueado el navegador, puesto que no se ha establecido temporizaci\u00f3n para la conexi\u00f3n" + newline +
                   "            (el valor predeterminado es -1, que significa que no hay temporizaci\u00f3n)." + newline + newline +
                   "            Para evitar este problema, Java Plug-in ha agregado un valor predeterminado de temporizaci\u00f3n de red (2 minutos) para todas las conexiones HTTP.:" + newline +
                   "            Este valor se puede anular en Par\u00e1metro del Runtime de Java:" + newline + newline +
                   "                  -Dsun.net.client.defaultConnectTimeout=valor en milisegundos" + newline + newline +
                   "            Otra propiedad que puede establecerse para las conexiones en red es sun.net.client.defaultReadTimeout." + newline + newline +
                   "                  -Dsun.net.client.defaultReadTimeout=valor en milisegundos" + newline + newline +
                   "                  Nota" + newline + newline +
                   "                  Java Plug-in no establece sun.net.client.defaultReadTimeout de forma predeterminada. Si desea establecer esta opci\u00f3n, debe hacerlo" + newline +
                   "                  en Par\u00e1metros del Runtime de Java de la forma indicada anteriormente." + newline + newline + newline +
                   "            Descripci\u00f3n de las propiedades de la conexi\u00f3n en red:" + newline + newline +
                   "            sun.net.client.defaultConnectTimeout" + newline +
                   "            sun.net.client.defaultReadTimeout" + newline + newline +
                   "                  Estas propiedades especifican, respectivamente, los valores predeterminados de temporizaci\u00f3n de conexi\u00f3n y lectura para los controladores de protocolos utilizados" + newline +
                   "                  por java.net.URLConnection. El valor predeterminado establecido por los controladores de protocolo es -1, que significa" + newline +
                   "                  que no se ha establecido temporizaci\u00f3n." + newline + newline +
                   "                  sun.net.client.defaultConnectTimeout especifica la temporizaci\u00f3n (en milisegundos) para establecer la conexi\u00f3n con el sistema." + newline +
                   "                  Por ejemplo, para conexiones http, es la temporizaci\u00f3n para establecer la conexi\u00f3n con el servidor http." + newline +
                   "                  Para conexiones ftp, es la temporizaci\u00f3n para establecer la conexi\u00f3n con los servidores ftp." + newline + newline +
                   "                  sun.net.client.defaultReadTimeout especifica la temporizaci\u00f3n (en milisegundos) cuando se lee desde una entrada" + newline +
                   "                  y se establece una conexi\u00f3n con un recurso." + newline + newline +
                   "            Para obtener la descripci\u00f3n oficial de estas propiedades de conexi\u00f3n en red, consulte" + newline +
                   "            http://java.sun.com/j2se/1.4/docs/guide/net/properties.html." + newline + newline + newline +
                   " Navegador" + newline + newline + newline +
                   "Este panel est\u00e1 relacionado con la instalaci\u00f3n de Microsoft Windows. No se muestra en otras instalaciones. Active las casillas" + newline +
"correspondientes a los navegadores para los que desea que Java Plug-in sea el Runtime predeterminado de Java, en vez de la JVM interna del navegador." + newline +
"Esto sirve para habilitar el soporte de etiquetas APPLET en Internet Explorer y Netscape 6 mediante Java Plug-in." + newline + newline + newline +
                   " Proxies" + newline + newline +newline +
                   "Utilice el panel Proxies para usar los valores por omisi\u00f3n del navegador o para omitir la direcci\u00f3n y puerto del proxy para los distintos protocolos." + newline + newline +
                   "Utilizar valores de navegador" + newline + newline +
                   "      Active esta casilla para utilizar la configuraci\u00f3n predeterminada del proxy. El valor predeterminado de esta casilla es activado." + newline + newline +
                   "Tabla de informaci\u00f3n del Proxy" + newline + newline +
                   "      Puede ignorar los valores predeterminados desactivando la casilla de verificaci\u00f3n \"Utilizar valores de navegador\" y rellenando la tabla de informaci\u00f3n" + newline +
                   "      del proxy que se muestra debajo de la casilla de verificaci\u00f3n. Puede escribir la direcci\u00f3n y puerto del proxy correspondiente a cada uno de los protocolos" + newline +
                   "      admitidos: HTTP, Seguro (HTTPS), FTP, Gopher y Z\u00f3calos." + newline + newline +
                   "No hay sistema delegado" + newline + newline +
                   "      Corresponde a una lista de sistemas para los que no se utilizan proxies. Los sistemas sin proxy se suelen utilizar para" + newline +
                   "      entornos de tipo intranet." + newline + newline +
                   "URL de configuraci\u00f3n delegada autom\u00e1tica" + newline +
                   "      Es la URL correspondiente al archivo JavaScript (con extensi\u00f3n .js o .pac) que contiene la funci\u00f3n FindProxyForURL." + newline +
                   "      FindProxyForURL dispone de la l\u00f3gica necesaria para determinar el servidor proxy que se va a utilizar para una petici\u00f3n de conexi\u00f3n." + newline + newline +
                   "Para obtener informaci\u00f3n adicional sobre la configuraci\u00f3n del proxy, consulte el cap\u00edtulo Configuraci\u00f3n del Proxy en la Gu\u00eda del" + newline +
                   "Desarrollador de Java Plug-in." + newline + newline + newline +
                   " Antememoria" + newline + newline + newline +
                   "           Nota" + newline + newline +
                   "           La antememoria que se describe en este apartado es de tipo sticky, es decir, la antememoria que Java Plug-in crea y controla en el disco y que" + newline +
                   "           el navegador no puede sobreescribir. Para obtener m\u00e1s informaci\u00f3n, consulte Antememorias de subprograma en la Gu\u00eda del Desarrollador de Java Plug-in." + newline + newline + newline +
                   "Habilitar colocaci\u00f3n en antememoria" + newline + newline +
                   "      Active esta casilla para habilitar la colocaci\u00f3n en antememoria. El valor predeterminado de esta casilla es activado. El rendimiento aumenta cuando esta opci\u00f3n est\u00e1 activada," + newline +
                   "      ya que una vez que un subprograma se coloca en antememoria ya no es necesario volver a cargarlo cuando se hace referencia a \u00e9l." + newline + newline +
                   "      Java Plug-in coloca en antememoria los archivos de los tipos siguientes (descargados mediante HTTP/HTTPS):" + newline + newline +
                   "            .jar (archivo jar)" + newline +
                   "            .zip (archivo zip)" + newline +
                   "            .class (archivo de clase java)" + newline +
                   "            .au (archivo de audio)" + newline +
                   "            .wav (archivo de audio)" + newline +
                   "            .jpg (archivo de imagen)" + newline +
                   "            .gif (archivo de imagen)" + newline + newline +
                   "Ver antememoria" + newline + newline +
                   "      Presione aqu\u00ed para ver el contenido de la antememoria. Se abre otro cuadro de di\u00e1logo (Visor de antememoria de Java Plug-in) que muestra los archivos que se encuentran en la antememoria." + newline +
                   "      El Visor de la antememoria muestra la siguiente informaci\u00f3n acerca de los archivos contenidos en la antememoria: Nombre, Tipo, Tama\u00f1o, Fecha de expiraci\u00f3n," + newline +
                   "      \u00daltima modificaci\u00f3n, Versi\u00f3n y URL. El Visor de la antememoria permite eliminar archivos seleccionados de la antememoria." + newline +
                   "      Este procedimiento es una alternativa a la opci\u00f3n Borrar antememoria que se describe a continuaci\u00f3n y que elimina todos los archivos de la antememoria." + newline + newline +
                   "Borrar antememoria" + newline + newline +
                   "      Presione aqu\u00ed para borrar todos los archivos de la antememoria. Se le preguntar\u00e1 (\u00bfEst\u00e1 seguro de que desea borrar todos los archivos ... _de la antememoria?) antes de eliminar los archivos." + newline + newline +
                   "Ubicaci\u00f3n" + newline + newline +
                   "      Puede utilizar esta opci\u00f3n para especificar la ubicaci\u00f3n de la antememoria. La ubicaci\u00f3n predeterminada de la antememoria es <user home>/.jpi_cache, donde" + newline +
                   "      <user home> es el valor correspondiente al propietario del sistema user.home. Este valor depende del sistema operativo." + newline + newline +
                   "Tama\u00f1o" + newline + newline +
                   "      Puede seleccionar Ilimitado para que la antememoria no tenga l\u00edmite de espacio o bien establecer el tama\u00f1o M\u00e1ximo de la antememoria." + newline +
                   "      Si la antememoria excede el tama\u00f1o especificado se borrar\u00e1n los archivos m\u00e1s antiguos hasta que el tama\u00f1o de la memoria" + newline +
                   "      est\u00e9 dentro del l\u00edmite." + newline + newline +
                   "Compresi\u00f3n" + newline + newline +
                   "      Puede establecer la compresi\u00f3n de los archivos JAR de la antememoria entre Ninguno o Alta. Si decide ahorrar memoria" + newline +
                   "      especificando una compresi\u00f3n alta, disminuir\u00e1 el rendimiento y, por el contrario, conseguir\u00e1 un mayor rendimiento si" + newline +
                   "      decide no comprimir." + newline + newline + newline +
                   " Certificados" + newline + newline + newline +
                   "Puede seleccionar entre cuatro tipos de certificados:" + newline + newline +
                   "      Applet firmado" + newline +
                   "      Sitio seguro" + newline +
                   "      Firmante de CA" + newline +
                   "      Sitio seguro de CA" + newline + newline +
                   "Applet firmado" + newline + newline +
                   "      Son los certificados correspondientes a los applets firmados en los que conf\u00eda el usuario. Los certificados que se muestran en la lista de applets" + newline +
                   "      firmados proceden del archivo de certificados jpicerts<version> ubicado en el directorio <user home>/.java." + newline + newline +
                   "Sitio seguro" + newline + newline +
                   "      Son los certificados que corresponden a sitios seguros. Los certificados que se muestran en la lista de sitios seguros proceden del archivo de" + newline +
                   "      certificados jpihttpscerts<version> ubicado en el directorio <user home>/.java." + newline + newline +
                   "Firmante de CA" + newline + newline +
                   "      Son los certificados procedentes de las Autoridades de certificaci\u00f3n (CA) para los applets firmados. Estas entidades emiten los certificados para" + newline +
                   "       los firmantes de los applets firmados. Los certificados que se muestran en la lista Firmante de CA proceden del archivo de certificados" + newline +
                   "       cacerts ubicado en el directorio <jre>/lib/security." + newline + newline +
                   "Sitio seguro de CA" + newline + newline +
                   "      Son los certificados procedentes de las Autoridades de certificaci\u00f3n (CA) para los sitios seguros. Estas entidades emiten los certificados" + newline +
                   "      correspondientes a los sitios seguros. Los certificados que se muestran en la lista Sitio seguro de CA proceden del archivo de certificados jssecacerts ubicado" + newline +
                   "      en el directorio <jre>/lib/security." + newline + newline +
                   "Para los certificados de tipo Applet firmado y Sitio seguro hay cuatro opciones: Importar, Exportar, Eliminar y Detalles." + newline +
                   "El usuario puede importar, exportar, eliminar y ver los detalles de un certificado." + newline + newline +
                   "Para los certificados de tipo Firmante de CA y Sitio seguro de CA, s\u00f3lo hay una opci\u00f3n: Detalles. El usuario s\u00f3lo puede ver los detalles de un certificado." + newline + newline + newline +
                   " Actualizar" + newline + newline + newline +
                   "En este panel, hay un bot\u00f3n \"Obtener la actualizaci\u00f3n de Java\" que permite al usuario obtener la \u00faltima actualizaci\u00f3n de Java Runtime" + newline +
"Environment desde el sitio web de Java Update. Este panel s\u00f3lo est\u00e1 relacionado con la instalaci\u00f3n en plataformas de Microsoft Windows. No se muestra para otras plataformas" + newline +
"(por ejemplo para Solaris/Linux)."}
};
}



