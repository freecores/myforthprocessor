/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)JSA_RSAnonCRTPrivateKey.java	1.3 03/06/24
 *
 * Portions Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.security.*;
import java.security.interfaces.*;
import java.math.BigInteger;
import java.io.*;
import COM.rsa.jsafe.*;

// non-CRT RSA private keys. Based on JSA_RSAPrivateKey.

/**
 * Copyright (C) RSA Data Security, Inc. created 1997.  This is an
 * unpublished work protected as such under copyright law.  This work
 * contains proprietary, confidential, and trade secret information of
 * RSA Data Security, Inc.  Use, disclosure or reproduction without the
 * express written authorization of RSA Data Security, Inc. is
 * prohibited.
 *
 * <p>This class specifies RSA for the superclass.
 *
 * <p>Only our internal code is capable of creating an instance of this
 * class, users will not directly instantiate this class, either through
 * a constructor or a getInstance method. Users can call methods whose
 * return value is a PrivateKey. Hence, they get back an instance of the
 * provider's class that implements PrivateKey. The JSAFE provider will
 * return an instance of this class.
 *
 * @author Steve Burnett <burnetts@rsa.com>
 * @author David Rudder <drudder@rsa.com>
 *
 */

public class JSA_RSAnonCRTPrivateKey extends JS_PrivateKey
   implements RSAPrivateKey, Cloneable, Serializable
{

  private static final long serialVersionUID = -6438884938615559810L;

  JSA_RSAnonCRTPrivateKey(JSA_RSAPrivateKey jsaKey) {
      super();
      this.thePrivateKey = jsaKey.thePrivateKey;
  }

  /** 
   * @return The name of the algorithm associated with the key
   */
  public String getAlgorithm ()
  {
    return ("RSA");
  }

  /**
   * @return The format in which the key is returned by getEncoded.
   */
  public String getFormat ()
  {
    return ("PKCS8");
  }

  /**
   * @return A byte array containing the key encoded in the format
   * specified by getFormat ();
   */
  public byte[] getEncoded ()
  {
    // Get the BER encoding out of the JSAFE_PrivateKey.
    try {
      byte[][] keyData = thePrivateKey.getKeyData ("RSAPrivateKeyBER");
      if (keyData == null || keyData.length == 0)
        return null;
      return (keyData[0]);
    } catch (JSAFE_Exception jsafeException) {
      // This should not happen.
    }
    return (null);
  }

  /**
   * @return A BigInteger representation of the modulus.
   */
  public BigInteger getModulus ()
  {
    BigInteger returnValue = null;
    try {
      // Get the info out of the JSAFE key.
      byte[][] keyData = thePrivateKey.getKeyData ("RSAPrivateKey");
      if (keyData == null || keyData.length == 0)
        return null;
      // Make a BigInteger out of the modulus byte Array. Since
      //   JSAFE returns the byte array without guaranteeing the leading
      //   bit (the sign bit) is 0, use the constructor that specifies a
      //   positive value.
      returnValue = new BigInteger (1, keyData[0]);

      // Overwrite the private exponent for security.
      for (int index = 0; index < keyData[1].length; index++)
        keyData[1][index] = 0;
    } catch (JSAFE_Exception jsafeException) {
      // Just make sure the return value is null.
      returnValue = null;
    }

    return (returnValue);
  }

  /**
   * @return A BigInteger representation of the private exponent
   */
  public BigInteger getPrivateExponent ()
  {
    BigInteger returnValue = null;
    try {
      // Get the info out of the JSAFE key.
      byte[][] keyData = thePrivateKey.getKeyData ("RSAPrivateKey");
      if (keyData == null || keyData.length == 0)
        return null;
      // Make a BigInteger out of the private exponent byte Array. Since
      //   JSAFE returns the byte array without guaranteeing the leading
      //   bit (the sign bit) is 0, use the constructor that specifies a
      //   positive value.
      returnValue = new BigInteger (1, keyData[1]);

      // Overwrite the private exponent for security.
      for (int index = 0; index < keyData[1].length; index++)
        keyData[1][index] = 0;
    } catch (JSAFE_Exception jsafeException) {
      // Just make sure the return value is null.
      returnValue = null;
    }

    return (returnValue);
  }

    /**
     * Returns the hashCode for the RSA private keys
     */
    public int hashCode() {
	return getModulus().hashCode() + getPrivateExponent().hashCode();
    }

  /**
   * Checks if two RSA private keys are equal, comparing all parameters.
   * @param key RSA private key 
   * @return 'true' if all the parameters are the same, 'false' otherwise.
   */ 
  public boolean equals (Object key)
  {
    if (key == this) {
	return true;
    }

    if (key instanceof RSAPrivateKey) {
        BigInteger value = getModulus();
        if (value.equals(((RSAPrivateKey)key).getModulus()) == false)
          return(false);
        value = getPrivateExponent ();
        if (value.equals(((RSAPrivateKey)key).getPrivateExponent ()) == false)
          return (false);
        return (true);
      }
    return (false);
  }
  
  private static String toString(BigInteger b) {
      return sun.security.util.Debug.toHexString(b);
  }
  
  public String toString() {
      StringBuffer sb = new StringBuffer();
      sb.append("SunJSSE RSA private key:\n  private exponent:\n");
      sb.append(toString(getPrivateExponent()));
      sb.append("\n  modulus:\n");
      sb.append(toString(getModulus()));
      return sb.toString();
  }

  /**fffffffff
   * Clear any sensitive data.
   */
  void clearSensitiveData ()
  {
    // This class contains no senstive data.
  }

  /**
   * Before destroying the object, clear any sensitive data.
   */
//  protected void finalize ()
//  {
//    clearSensitiveData ();
//  }
}
