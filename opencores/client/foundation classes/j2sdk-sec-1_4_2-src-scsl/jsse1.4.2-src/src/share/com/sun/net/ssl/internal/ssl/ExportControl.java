/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;


/**
 * This interface is used to provide compile time support for building two
 * versions of a software package.  One is a full-strength version, for
 * "US Domestic" use.  The other version is a weakened "global" version,
 * which may be exported in compliance with US Export regulations.
 *
 * <P>As a rule, source code only tests for an uncontrolled version in
 * order to add functionality to US domestic versions.  Otherwise, US
 * domestic and global versions would not interoperate.  That is, most
 * source code should look like this: <PRE>
 *	if (ExportControl.hasStrongCrypto) {
 *	    ... "strong" cryptography goes here
 *	}
 * </PRE>
 * <P> Also, the ability to plug in or configure other SSL
 * implementations to work with the JSSE api classes may be
 * restricted. Most JSSE api implementation source code which
 * allows replaceability shuold look this like: <PRE>
 *	if (ExportControl.isReplaceable) {
 *	    ... "provider-based" cryptography goes here
 *	}
 * </PRE>
 *
 * <P><em>NOTE</em> that export regulations are subject to interpretation.
 * Individual rulings are provided on a product-by-product, and sometimes
 * even on a customer-by-customer basis.  Don't change lines of code which
 * check for export control status without understanding the legal
 * ramifications of such changes.
 *
 * <P> Of particular note is an evident change in policy to allow "strong"
 * cryptography to be exported to certain large classes of customers.
 */
public interface ExportControl {
    /**
     * This compile-time constant is <em>true</em> if and only if the
     * release being compiled is allowed to have technologies which, when
     * exported, are subject to US Export regulations.
     *
     * <P>The value is set to <em>false</em> in order to disable features
     * such as symmetric encryption using key data of more than 40 bits,
     * key exchange using large asymmetric keys, and some kinds of data
     * compression.
     *
     * This flag implements encryption limitations that are now
     * generally obsolete. Building JSSE for "global" release can
     * now leave this flag at its default "true" setting and ship
     * strong encryption. The flag name should be changed in the future
     * because it is now misleading.
     */
    public static final boolean hasStrongCrypto = true;


    /*
     * This compile-time constant is <em>true</em> if and only if the
     * release being compiled is allowed to have non-Sun
     * implementations of SSLContext configured into it. U.S.
     * export regulations may require that the JSSE api be tied to
     * the Sun implementation even if strong encryption is allowed.
     */
    public static final boolean isReplaceable = false;
}
