/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.net.ssl;

/**
 * This class is the base interface for providing
 * algorithm-specific information to a KeyManagerFactory or
 * TrustManagerFactory.
 * <P>
 * In some cases, initialization parameters other than keystores
 * may be needed by a provider.  Users of that particular provider
 * are expected to pass an implementation of the appropriate
 * sub-interface of this class as defined by the
 * provider.  The provider can then call the specified methods in
 * the <CODE>ManagerFactoryParameters</CODE> implementation to obtain the
 * needed information.
 *
 * @author Brad R. Wetmore
 * @version 1.3, 06/24/03
 * @since 1.4
 */

public interface ManagerFactoryParameters {
}
