             Java Secure Socket Extension for the
          Java(TM) 2 SDK, Standard Edition, v 1.4.2
                       Source Release


----------------------------------------------------------------------
INTRODUCTION
----------------------------------------------------------------------
Thank you for downloading the source distribution for the Java(TM)
Secure Socket Extension (JSSE).  This source code was used to generate
the "jsse.jar" file provided in Sun Microsystems' Java(TM) 2 SDK,
Standard Edition, v 1.4.2 distribution.

----------------------------------------------------------------------
UNDERSTANDING THE IMPORT/EXPORT ISSUES
----------------------------------------------------------------------
While there has been some relaxation in the US export requirements,
there are many restrictions still in place for strong encryption
products.  In addition, some countries may have import restrictions.
Contrary to some articles in the press, export of encryption technology
is still a complicated issue.  You are advised to consult your export/
import control counsel or attorney to determine the exact requirements.

The binary release of the Java 2 SDK, v 1.4.2 (which includes JSSE) has
ENC/Retail status.  Note that this status only applies to Sun
Microsystems; the use of this source code may require your
implementation undergo a similar review.  Again, you are advised to
consult your export/import control counsel or attorney to determine the
requirements for your situation, as import/export laws are subject to
change.

----------------------------------------------------------------------
CRYPTOGRAPHY LIMITATIONS FOR THIS RELEASE
----------------------------------------------------------------------
Sun's implementation of JSSE uses cryptographic algorithms from several
sources:

        o our own internal implementations
        o the Java(TM) Cryptography Architecture (JCA) and Java(TM)
          Cryptography Extension (JCE)
        o ACME Labs (http://www.acme.com/java/)
        o RSA Security, Inc. (http://www.rsasecurity.com/)

Sun's JSSE implementation includes source code licensed from RSA
Security.  Due to license limitations, Sun cannot distribute that
source code.

If you develop your own version of JSSE, you will need to make some
minor modifications.  See "BUILDING THE RELEASE" below.

----------------------------------------------------------------------
COMPILE-TIME CONFIGURATION OPTIONS
----------------------------------------------------------------------
This source distribution includes two compile-time switches:

        hasStrongCrypto:
                Controls whether so-called "strong cryptographic"
                algorithms are available. You can use this switch to
                disable features such as symmetric encryption using key
                data of certain sizes, key exchange using large
                asymmetric keys, and some kinds of data compression.

        isReplaceable:
                Controls whether the implementation allows non-Sun SSL/TLS
                implementations ("pluggability") when using the JSSE APIs.

Due to export requirements, the default switches for the binary
implementation of this release are:

        hasStrongCrypto = true
        isReplaceable = false

For more information and the location of these switches, consult:

        src/share/com/sun/net/ssl/internal/ssl/ExportControl.java

Provider information is located in:

        src/share/com/sun/net/ssl/internal/ssl/Provider.java

This file is specific to the SunJSSE provider. Please be sure to
change it to reflect your environment.

----------------------------------------------------------------------
BUILDING THE RELEASE
----------------------------------------------------------------------
As mentioned above, this distribution does not contain any RSA classes.
For ease of porting, this distribution does not remove any of the RSA
BSAFE Crypto-J calls or any cipher suites that depend on those algorithms.
Before making any porting decisions, here are three options to consider:

    1)  You could continue to use RSA's Crypto-J library. You will need
        to contact RSA Security, Inc. to obtain source-licensing
        information. It should be relatively easy to determine the
        wrapper classes that need to be placed into the
        com.sun.net.ssl.internal.ssl package so you can access the
        Crypto-J classes.

    2)  You could remove the Crypto-J calls and rely on the registered
        JCA/JCE providers. Sun's JSSE was modified recently to use the
        JCA/JCE to find algorithm implementations. If JSSE finds an
        algorithm instance registered in a provider registered before
        the JSSE provider, it will use that algorithm in place of the
        built-in (Crypto-J or local) algorithm. The cipher suite will be
        unusable if you remove the built-in Crypto-J calls and the
        provider of a particular algorithm is not available.

        Note: The PKCS12 keystore code has not been retrofitted with
        the JCA/JCE calls, and would need some modifications.

    3)  You could use another library. If you use another cryptographic
        library, you must retrofit the Crypto-J calls with the appropriate
        calls for that replacement library.

Once a suitable library is available and any appropriate source changes
have been made, you should be able to compile the entire source tree
using javac. For any sustained engineering effort, we highly recommend
that you use a dependency-based build tool such as "make" (or equivalents
such as "GNUmake" or "ANT").

In addition, this source code depends heavily on the sun.* hierarchy for
many of its support classes.  Unfortunately, these dependencies will make
it difficult if you cannot include the sun.* classes in your distribution.

For export reasons, Sun's JSSE implementation has been obfuscated.
There are a number of benefits to obfuscating the library. If you
choose to obfuscate, there are many obfuscation tools available;
we used DashO-Pro 2.3 (Build 6) from preEmptive Solutions
(www.preemptive.com). If you also choose to use DashO, be sure to use
at least this release or later.

----------------------------------------------------------------------
PACKAGING THE RELEASE
----------------------------------------------------------------------
Depending on your specific requirements, you may choose either to package
JSSE in one jar file with your other classes, or to keep it separate
as we have done.

When packaging your release for distribution, you may want to include
specific manifest entries in your jar file(s).  You should also construct
any version information as suited to your organization's policies. For
more information on packaging and versioning, please refer to the
following documentation:

        http://java.sun.com/j2se/1.4/docs/guide/jar/index.html
        http://java.sun.com/docs/books/tutorial/jar/
