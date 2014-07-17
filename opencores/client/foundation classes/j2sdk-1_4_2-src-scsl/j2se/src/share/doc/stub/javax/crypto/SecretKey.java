/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SecretKey.java	1.3 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto;

/** 
 * A secret (symmetric) key.
 *
 * <p>This interface contains no methods or constants.
 * Its only purpose is to group (and provide type safety for) secret keys.
 *
 * <p>Provider implementations of this interface must overwrite the
 * <code>equals</code> and <code>hashCode</code> methods inherited from
 * <code>java.lang.Object</code>, so that secret keys are compared based on
 * their underlying key material and not based on reference.
 *
 * <p>Keys that implement this interface return the string <code>RAW</code>
 * as their encoding format (see <code>getFormat</code>), and return the
 * raw key bytes as the result of a <code>getEncoded</code> method call. (The
 * <code>getFormat</code> and <code>getEncoded</code> methods are inherited
 * from the <code>java.security.Key</code> parent interface.)
 *
 * @author Jan Luehe
 *
 * @version 1.3, 01/23/03
 *
 * @see SecretKeyFactory
 * @see Cipher
 * @since 1.4
 */
public interface SecretKey extends java.security.Key
{
}
