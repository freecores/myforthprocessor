Extension Installer Sample
--------------------------

Brief Description :

This sample demonstrates features of ExtensionInstallerService API.
This application consists of two parts. The first part is extension
installer which uses ExtensionInstallerService interface to communicate 
with the JNLP Client. Java web start upon encountering extension tag in 
the .jnlp file starts the specified extension installer class. 
Extension installer writes a native shared library (libinst.so or 
inst.dll) to the recommended directory on the local client machine. 
It then informs the JNLP Client of a directory where it should search 
for native libraries. This Shared library we installed has implementation 
only for native method getString(). Also please note that the Extension 
installer is executed only one time that is the first time when the 
application is run. The second part of demo (Inst.class) when executed by 
Java Web Start displays GUI components and executes the native method 
getString().

The following JNLP API is used by Extension Installer application :

ExtensionInstallerService : getInstallPath() returns the directory where 
the installer is recommended to install the extension in. Another method
setNativeLibraryInfo() informs the JNLP Client of a directory where it 
should search for native libraries. installSucceeded() and installFailed()
will be used to determine if installation was a success or a failure.

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
            inst/Inst.java has some basic installer stuff. inst.c 
            and inst.h are used for building the shared libraries
            libinst.so or inst.dll. The extension directory
            has Extension installer service sources required
            for writing a shared library to the JNLP client machine.

lib       - This directory has any .jar files required at the
            compilation time.

class     - This directory containing .class files is generated
            by the build process. Shared library (libinst.so or
            inst.dll) files exist inside extension. These files are
            placed here so that they can be archived into .jar
            files. so please make sure that these shared libraries
            exist here before building the jar files.


war       - This directory contains all the files required
            for deploying a web based application. Application
            related .jnlp files and .jar files are inside app. 
            inst.jnlp and inst.jar are the main application 
            resources. JWS determines when to invoke extension
            stuff in inst_ext.jnlp and inst_ext.jar file.
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

   Note : Please make sure you have built native shared libraries
   and copied them to the appropriate (class/extension) directory 
   before running the above command.

2) Applications by default run in a restricted environment where they 
   have limited access to local computing resources, such as storage 
   devices and the local network.  Extension Installer needs
   unrestricted access, so we need to sign the jar file using
   keytool and jarsigner utility. To do this run :

   keytool -genkey -keystore myKeystore -alias myself 
 
   jarsigner -keystore myKeystore war/app/inst.jar myself
   jarsigner -keystore myKeystore war/app/inst_ext.jar myself

   Read more details about Signing JAR Files process in 
   Java Web Start Developer's Guide.
     

3) To ease the deployment of application to on any J2EE-complaint
   server all the resources required including .jnlp and jar files
   are packed into a Web Archive (.war) file.

   To build .war file, run below command inside the war directory


   jar -cvf ../Extinstaller.war .


4) Copy Extinstaller.war file to the your web server directory.
   For Tomcat server, copy Extinstaller.war file to 
   $TOMCAT_HOME/webapps directory and restart the web server.


Known issues :

This application when executed for the first time by Java Web Start
may raise "Launch File Error" if your webserver is running on a
non-default port. Subsequent invocations of the same application
will execute successfully without any error messages. This is a known 
bug 4468969 and will  be fixed in 1.1 release of Java Web Start.
If an installer extension uses a non-default port in the codebase of 
the extension JNLP, Java Web Start will always say that the installation 
failed, even if it succeeded.  If the success is misreported like this, 
the application will still work next time it is run.

