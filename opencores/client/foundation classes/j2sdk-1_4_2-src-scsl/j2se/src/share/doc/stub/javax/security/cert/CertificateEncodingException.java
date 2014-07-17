/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)CertificateEncodingException.java	1.5 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.security.cert;

/** 
 * Certificate Encoding Exception. This is thrown whenever an error
 * occurs whilst attempting to encode a certificate.
 *
 * <p><em>Note: The classes in the package <code>javax.security.cert</code>
 * exist for compatibility with earlier versions of the
 * Java Secure Sockets Extension (JSSE). New applications should instead
 * use the standard J2SE certificate classes located in
 * <code>java.security.cert</code>.</em></p>
 *
 * @since 1.4
 * @author Hemma Prafullchandra
 * @version 1.6
 */
public class CertificateEncodingException extends CertificateException
{

    /** 
     * Constructs a CertificateEncodingException with no detail message. A
     * detail message is a String that describes this particular
     * exception.
     */
    public CertificateEncodingException() { }

    /** 
     * Constructs a CertificateEncodingException with the specified detail
     * message. A detail message is a String that describes this
     * particular exception.
     *   
     * @param message the detail message.
     */
    public CertificateEncodingException(String message) { }
}
