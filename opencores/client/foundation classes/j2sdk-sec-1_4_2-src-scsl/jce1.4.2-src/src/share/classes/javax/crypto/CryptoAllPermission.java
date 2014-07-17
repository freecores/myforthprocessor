/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.security.*;
import java.util.Enumeration;
import java.util.Vector;

/**
 * The CryptoAllPermission is a permission that implies
 * any other crypto permissions.
 * <p>
 * 
 * @see java.security.Permission
 * @see java.security.AllPermission
 *
 * @version 1.6 06/24/03
 * @author Sharon Liu
 * @since 1.4
 */

final class CryptoAllPermission extends CryptoPermission {

    // This class is similar to java.security.AllPermission.
    static final String ALG_NAME = "CryptoAllPermission";
    static final CryptoAllPermission INSTANCE = 
	new CryptoAllPermission();

    private CryptoAllPermission() {
        super(ALG_NAME);
    }

    /**
     * Checks if the specified permission is implied by
     * this object.
     *
     * @param p the permission to check against.
     *
     * @return true if the specified permission is an
     * instance of CryptoPermission.
     */
    public boolean implies(Permission p) {
	 return (p instanceof CryptoPermission);
    }

    /**
     * Checks two CryptoAllPermission objects for equality.
     * Two CryptoAllPermission objects are always equal.
     *
     * @param obj the object to test for equality with this object.
     *
     * @return true if <i>obj</i> is a CryptoAllPermission object.
     */
    public boolean equals(Object obj) {
	return (obj == INSTANCE);
    }

    /**
     *
     * Returns the hash code value for this object.
     * 
     * @return a hash code value for this object.
     */ 
    public int hashCode() {
	return 1;
    }

    /**
     * Returns a new PermissionCollection object for storing 
     * CryptoAllPermission objects.
     * <p>
     * 
     * @return a new PermissionCollection object suitable for 
     * storing CryptoAllPermissions.
     */
    public PermissionCollection newPermissionCollection() {
	return new CryptoAllPermissionCollection();
    }
}

/**
 * A CryptoAllPermissionCollection stores a collection
 * of CryptoAllPermission permissions.
 *
 * @see java.security.Permission
 * @see java.security.Permissions
 * @see javax.crypto.CryptoPermission
 *
 * @version 1.6 06/24/03
 *
 * @author Sharon Liu
 */
final class CryptoAllPermissionCollection extends PermissionCollection 
    implements java.io.Serializable 
{
    // true if a CryptoAllPermission has been added
    private boolean all_allowed;

    /**
     * Create an empty CryptoAllPermissions object.
     */
    CryptoAllPermissionCollection() {
	all_allowed = false;
    }

    /**
     * Adds a permission to the CryptoAllPermissions.
     *
     * @param permission the Permission object to add.
     *
     * @exception SecurityException - if this CryptoAllPermissionCollection
     * object has been marked readonly
     */
    public void add(Permission permission)
    {
	if (isReadOnly())
	    throw new SecurityException("attempt to add a Permission to " +
					"a readonly PermissionCollection");

        if (permission != CryptoAllPermission.INSTANCE)
            return;

	all_allowed = true;
    }

    /**
     * Check and see if this set of permissions implies the permissions 
     * expressed in "permission".
     *
     * @param p the Permission object to compare
     *
     * @return true if the given permission is implied by this
     * CryptoAllPermissionCollection.
     */    
    public boolean implies(Permission permission) 
    {
	if (!(permission instanceof CryptoPermission)) {
	    return false;
	}
	return all_allowed;
    }

    /**
     * Returns an enumeration of all the CryptoAllPermission 
     * objects in the  container.
     *
     * @return an enumeration of all the CryptoAllPermission objects.
     */
    public Enumeration elements() {
	Vector v = new Vector(1);
	if (all_allowed) v.add(CryptoAllPermission.INSTANCE);
	return v.elements();
    }
}

