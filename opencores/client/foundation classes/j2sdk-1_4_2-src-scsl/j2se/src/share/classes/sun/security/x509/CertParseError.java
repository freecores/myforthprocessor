/*
 * @(#)CertParseError.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

/**
 * CertException indicates one of a variety of certificate problems.
 * @deprecated use one of the Exceptions defined in the
 * java.security.cert package.
 *
 * @version 1.14
 * @author David Brownell
 */

class CertParseError extends CertException
{
    CertParseError (String where)
    {
	super (CertException.verf_PARSE_ERROR, where);
    }
}

