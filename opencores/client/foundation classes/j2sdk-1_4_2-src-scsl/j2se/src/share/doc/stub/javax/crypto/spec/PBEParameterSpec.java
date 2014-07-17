/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)PBEParameterSpec.java	1.3 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto.spec;

import java.math.BigInteger;
import java.security.spec.AlgorithmParameterSpec;

/** 
 * This class specifies the set of parameters used with password-based
 * encryption (PBE), as defined in the
 * <a href="http://www.rsa.com/rsalabs/pubs/PKCS/html/pkcs-5.html">PKCS #5</a>
 * standard.
 * 
 * @author Jan Luehe
 *
 * @version 1.3, 01/23/03
 * @since 1.4
 */
public class PBEParameterSpec implements AlgorithmParameterSpec
{

    /** 
     * Constructs a parameter set for password-based encryption as defined in
     * the PKCS #5 standard.
     *
     * @param salt the salt.
     * @param iterationCount the iteration count.
     */
    public PBEParameterSpec(byte[] salt, int iterationCount) { }

    /** 
     * Returns the salt.
     *
     * @return the salt
     */
    public byte[] getSalt() { }

    /** 
     * Returns the iteration count.
     *
     * @return the iteration count
     */
    public int getIterationCount() { }
}
