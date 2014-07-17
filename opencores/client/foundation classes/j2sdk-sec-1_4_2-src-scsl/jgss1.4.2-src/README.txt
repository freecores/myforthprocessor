              Java GSS and Kerberos Source Distribution for the 
                  Java(TM) 2 SDK Standard Edition, v 1.4.2
                            Source Release


----------------------------------------------------------------------
INTRODUCTION
----------------------------------------------------------------------
Thank you for downloading the source distribution for the Java GSS 
and Kerberos. This source code was used to generate some of the 
Java GSS and Kerberos related classes provided in Sun Microsystems' 
Java(TM) 2 SDK, Standard Edition, v 1.4.2 distribution.

----------------------------------------------------------------------
UNDERSTANDING THE IMPORT/EXPORT ISSUES
----------------------------------------------------------------------
While there has been some relaxation in the US export requirements,
there are many restrictions still in place for strong encryption
products. In addition, some countries may have import restrictions.
Contrary to some articles in the press, export of encryption 
technology is still a complicated issue.  You are advised to consult 
your export/import control counsel or attorney to determine the
exact requirements.

The binary release of Java 2 SDK, v 1.4.2 (which includes Java GSS and
Kerberos) has ENC/Retail status. Note that this status only applies to
Sun Microsystems; the use of this source code may require your
implementation undergo a similar review.  Again, you are advised to
consult your export/import control counsel or attorney to determine the
requirements for your situation, as import/export laws are subject to
change.

----------------------------------------------------------------------
BUILDING THE RELEASE
----------------------------------------------------------------------
You should be able to compile the entire source tree using javac. Note
that this source code depends on the sun.* hierarchy for many of its 
support classes since it's considered part of Java 2 SDK v 1.4.2.

Here are a list of components inside this source distribution and its 
associated directories:

        o Java GSS Classes 
                src/share/classes/sun/security/jgss/spi/GSSContextSpi
        
        o Kerberos Classes 
                src/share/classes/sun/security/krb5/

        Please note that there are other Java GSS and Kerberos-related
        files that are part of the standard Java 2 SDK v 1.4.2 source 
        distribution but are not part of this download.

For export reasons, Sun's Java GSS and Kerberos implementation has 
been obfuscated. There are a number of benefits to obfuscating the 
library. If you choose to obfuscate, there are many obfuscation tools 
available; we used DashoPro 2.3 (Build 6) tool from preEmptive 
Solutions (www.preemptive.com). If you also choose to use Dasho, 
be sure to use at least this release or later.

----------------------------------------------------------------------
PACKAGING THE RELEASE
----------------------------------------------------------------------
Depending on your specific requirements, you may choose to package Java
GSS and Kerberos classes in one jar file with your other classes (as we
have done), or you may keep it separate.

When packaging your release for distribution, you may want to include 
specific manifest entries in your jar file(s). You should also construct 
any version information as suited to your organization's policies. For 
more information on packaging and versioning, please refer to the 
following documentation:

        http://java.sun.com/j2se/1.4/docs/guide/jar/index.html
        http://java.sun.com/docs/books/tutorial/jar/
