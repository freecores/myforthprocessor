Java Web Start Random Access File Demo
--------------------------------------

Brief Description :

This sample demonstrates features of JNLPRandomAccessFile API.
The file which user picks is opened as a random file for both 
Writing and Reading. Application first writes some hard coded 
strings to the file selected by the user. It then reads whatever 
was written to the file and displays it on to the screen using 
awt Containter and the TextArea components. 
The following JNLP API is used by the Application : 

FileOpenService : openFileDialog will allow the user to choose a 
single file. Contents of the selected file is returned as a 
FileContents object.

FileContents : This object encapsulates the name and contents of a 
file. getLength(), getMaxLength() and setMaxLength are used. If the
the maximum file size defined by the client is not more than 1024
bytes of the current length, then it will be adjusted accordingly. 
getName() returns the name of the file selected.getRandomAccessFile() 
method is used for creating JNLPRandomAccessFile with read/write 
permissions.

JNLPRandomAccessFile : Methods provided by this interface are used
for reading and writing to random file. seek() sets the file-pointer
offset, measured from the beginning of this file, at which the next
read or write occurs. writeUTF() and writeBytes() are demonstrated
for writing to a file. readLine() is used for reading the next line
from the file.

Java Web Start is launched automatically by browser when the user clicks on
a link which is associated with JNLP file. It then downloads and caches the
RAFSample application on the local client machine where browser is running.
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
            randomFile.java
lib       - This directory has any .jar files required at the
            compilation time.
class     - This directory containing .class files is generated
            by the build process.
war       - This directory contains all the files required
            for deploying a web based application. Application
            related .jnlp files and .jar files are inside app. 
build.xml - This file is used by Ant for building the application.
            It is kind of a Makefile for ant tool.



Building & Deployment of Application :


1) Ant is a Java based build tool we used for building this application. 
   For more information, check out http://jakarta.apache.org/.

   Build .class and .jar files by running 
   
   ant main

   in the parent directory where build.xml file resides.


2) To ease the deployment of application to on any J2EE-complaint
   server all the resources required including .jnlp and jar files
   are packed into a Web Archive (.war) file.
   
   To build .war file, run below command inside the war directory

   jar -cvf ../RAFSample.war .

3) Copy RAFSample.war file generated to the your web server directory.
   For Tomcat server, copy RAFSample.war file to 
   $TOMCAT_HOME/webapps directory and restart the web server.
