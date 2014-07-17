Installing Libraries using <nativelib> element
----------------------------------------------

Brief Description :

This sample demonstrates usage of <nativelib> element under resources
in .jnlp file. A nativelib element specifies a JAR file that contains 
native libraries.  For example: 

          <nativelib href="corelib.jar"/> 

The JNLP Client must ensure that each file entry in the root directory 
of the JAR file (i.e., /) can be loaded into the running process using
the System.loadLibrary() method.  Each entry must contain a platform-
dependent shared library with the correct naming convention,
e.g., *.dll on Windows, or lib*.so on Solaris/Linux.  

corelib.jar file in this sample consists of  native shared libraries
for both Solaris (libinst.so) and Windows(inst.dll) platforms.
The application code is responsible for calling System.loadLibrary("inst") 
as demonstrated by the Inst class. Inst class when executed by Java Web 
Start displays GUI components and executes the native method getString().

Another method for installing native libraries on the JNLP client machine
is using Extension installer feature of Java Web Start product. JNLP API 
ExtensionInstallerService class is used for this purpose as demonstrated
by the Extension Installer Sample.

Java Web Start is launched automatically by browser when the user clicks on
a link which is associated with a JNLP file.It then downloads and caches the
installer application on the local client machine where browser is running.
This cached application when invoked next time is already downloaded and
runs as fast as a traditionally installed application.By default applications
are run in a protective environment (sandbox) with restricted access to
local disk and network resources. Java Web Start only transfers JAR files
from the Web server to the location determined by it on the client machine. 

To provide a simple and convenient packaging format of Applications, Java
Web Start includes a servlet in the developer's pack. The name of the servlet
is JnlpDownloadServlet and is packaged into the jnlp-servlet.jar file. This
servlet eases deployment of application on any J2EE-complaint application
server by packing a JNLP files and its associated resources in a Web Archive
(.war) file. 

For more details on Java Web Start product and JNLP API, Please check out
online docs at
http://java.sun.com/products/javawebstart/developers.html



Files :

src       - This directory has all the required source files :
            Inst.java has some basic installer stuff. Files inst.c
            and inst.h are used for building the shared libraries. 
            src/lib directory has the actual native libraries.

lib       - This directory has any .jar files required at the
            compilation time.

class     - This directory containing .class files is generated
            by the build process. 

war       - This directory contains all the files required
            for deploying a web based application. Application
            related .jnlp files and .jar files are inside app. 
            inst.jnlp and inst.jar are the main application 
            resources. corelib.jar file consists of native libs.
            myKeystore is a keystore used mainly for the purpose
            of signing jar files.

build.xml - This file is used by Ant for building the application.
            It is kind of a Makefile for ant tool.


Building & Deployment of Application :


1) Ant is a Java based build tool we used for building this application. 
   For more information, check out http://jakarta.apache.org/.

   Build .class and .jar files by running 
   
   ant main

   in the parent directory where build.xml file resides.
   
   Note: Please make sure that native libraries (libinst.so or inst
   .dll) exist in src/lib directory before executing this command.

2) Applications by default run in a restricted environment where they 
   have limited access to local computing resources, such as storage 
   devices and the local network. Installer needs unrestricted access, 
   so we need to sign the jar file using keytool and jarsigner utility. 

   keytool -genkey -keystore myKeystore -alias myself 
 
   jarsigner -keystore myKeystore war/app/inst.jar myself
   jarsigner -keystore myKeystore war/app/corelib.jar myself

   Read more details about Signing JAR Files process in 
   Java Web Start Developer's Guide.
     

3) To ease the deployment of application to on any J2EE-complaint
   server all the resources required including .jnlp and jar files
   are packed into a Web Archive (.war) file.

   To build .war file, run below command inside the war directory


   jar -cvf ../Libinstaller.war .


4) Copy Libinstaller.war file to the your web server directory.
   For Tomcat server, copy Libinstaller.war file to 
   $TOMCAT_HOME/webapps directory and restart the web server.



