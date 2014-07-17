/*
 * @(#)CertificateInfo.java	1.14 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import com.sun.javaws.debug.Debug;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Method;
import java.security.Principal;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Date;

/** This class is a high-level wrapper class for a certificate
*
*  It provides convience access to information about the certificate.
*  such as the Name, Expiration date, etc.
*
*  This wrapper class is designed to easy presentation of a certificate
*  (in contrast to actually sign or verify something)
*
*  The class also acts as a factory, with the static method:
*
*   CertificateInfo getCertificateInfo(Certificate cert)
*
*/
class CertificateInfo {
    private static ArrayList _keys;

    private String 	_subjectName;
    private String 	_issuerName;
    private Date   	_expirationDate;
    private Date        _startDate;
    private Certificate _certificate;

    static {
        _keys = new ArrayList(8);
        _keys.add(new X509KeyLookup("Issuer ID", "getIssuerUniqueID"));
        _keys.add(new X509KeyLookup("Issuer", "getIssuerDN"));
        _keys.add(new X509KeyLookup("Valid From", "getNotBefore"));
        _keys.add(new X509KeyLookup("Valid To", "getNotAfter"));
        _keys.add(new X509KeyLookup("Serial Number", "getSerialNumber"));
        _keys.add(new X509KeyLookup("Subject", "getSubjectDN"));
        _keys.add(new X509KeyLookup("Subject ID", "getSubjectUniqueID"));
        _keys.add(new X509KeyLookup("Version", "getVersion"));
    }
    
    // Private constructor. Use getCertificateInfo to build an info. object
    // This constructor builds a CertificateInfo object from a X509 certificate
    private CertificateInfo(X509Certificate cert) {
	_certificate = cert;
	_subjectName = getDisplayName(cert.getSubjectDN().getName());
	_issuerName  = getDisplayName(cert.getIssuerDN().getName());
	_expirationDate = cert.getNotAfter();
	_startDate = cert.getNotBefore();
    }
    
    private String getDisplayName(String s) {
	String name = parseString(s, "CN"); // Common Name
	if (name == null || name.length() == 0) name = parseString(s, "O");  // Organisation
	return name;
    }
    
    // Query methods
    String      getSubjectName()     { return _subjectName; }
    String      getIssuerName()	     { return _issuerName; }
    Date        getExpirationDate()  { return _expirationDate; }
    Date        getStartDate()       { return _startDate; }
    Certificate getCertificate()     { return _certificate; }

    /**
     * Returns the keys known for this Certificate. You can use
     * <code>toString</code> to get the descriptive value of the returned
     * Object, as well as calling back to <code>getValue</code> to return
     * the particular value of a returned key.
     */
    Iterator getKeys() {
        return _keys.iterator();
    }

    String getValue(Object key) {
        return ((X509KeyLookup)key).getValue(_certificate);
    }
    
    /** Factory method to construct a CertificateInfo object. 
    *   It throws a CertificateException if the kind of certificate is unknown.
    */
    static CertificateInfo getCertificateInfo(Certificate cert) 
					throws CertificateException {
	if (cert instanceof X509Certificate) {
	    return new CertificateInfo((X509Certificate)cert);
	} else {
	    // Unkown kind of certificate
	    throw new CertificateException("Unknown kind of certificate");
	}
    }
    
    
    
    // Private helper methods
    
    /** Returns the value of a key in the DN format, i.e., key=value,key=value, ... */
    static private String parseString(String str, String key) {
	int startIdx = str.indexOf(key + "=");
	if (startIdx == -1) {
	    // Key not found
	    return "";
	}
	startIdx += key.length() + 1;
	String remainder = (str.substring(startIdx)).trim();
	if (remainder.startsWith("\"")) {
	    int endIdx = remainder.indexOf("\"",1);
	    if (endIdx > 0) {
		return remainder.substring(1, endIdx);
	    }
	}
	int endIdx   = str.indexOf(',', startIdx);
	if (endIdx != -1) {
	    return str.substring(startIdx, endIdx);
	} else {
	    return str.substring(startIdx);
	}
    }


    private static class X509KeyLookup {
        private String _key;
        private Method _method;
        private String _methodName;

        X509KeyLookup(String key, String methodName) {
            _key = key;
            _methodName = methodName;
        }

        public String getValue(Object source) {
            Method m = getMethod(source, _methodName);
            return handleLookup(m, source);
        }

        public String toString() {
            return _key;
        }

        private Method getMethod(Object instance, String mName) {
            if (_method == null) {
                try {
                    _method = instance.getClass().getMethod(mName, null);
                } catch (NoSuchMethodException nsme) {
                    Debug.fatal("No method: " + mName);
                }
            }
            return _method;
        }

        private String handleLookup(Method m, Object source) {
            Object retValue = "";
            try {
                retValue = m.invoke(source, null);
            } catch (IllegalArgumentException iae) {
            } catch (IllegalAccessException iae2) {
            } catch (InvocationTargetException ite) {
            }
            return convertToString(retValue);
        }

        private String convertToString(Object value) {
            if (value instanceof String) {
                return (String)value;
            }
            else if (value instanceof byte[]) {
                StringBuffer sb = new StringBuffer();
                byte[] bArray = (byte[])value;

                if (bArray.length > 0) {
                    sb.append(Integer.toHexString((int)bArray[0]));
                    for (int counter = 1; counter < bArray.length; counter++) {
                        sb.append(':');
                        sb.append(Integer.toHexString((int)bArray[counter]));
                    }
                }
                return sb.toString();
            }
            else if (value instanceof Principal) {
                return ((Principal)value).getName();
            }
            else if (value instanceof Date) {
                DateFormat df = DateFormat.getDateInstance(DateFormat.SHORT);
                return df.format((Date)value);
            }
            else if (value instanceof boolean[]) {
                boolean[] bits = (boolean[])value;
                StringBuffer sb = new StringBuffer();

                for (int counter = 0; counter < bits.length; counter++) {
                    if (counter % 8 == 1 && counter != 1) {
                        sb.append(' ');
                    }
                    sb.append(bits[counter] ? '1' : '0');
                }
                return sb.toString();
            }
            else if (value != null) {
                return value.toString();
            }
            return "";
        }
    }
}
