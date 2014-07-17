/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.security.*;
import java.security.spec.AlgorithmParameterSpec;
import java.io.Serializable;
import java.util.Enumeration;
import java.util.Vector;

import javax.crypto.spec.*;

/**
 * The CryptoPermission class extends the
 * java.security.Permission class. A
 * CryptoPermission object is used to represent
 * the ability of an application/applet to use certain
 * algorithms with certain key sizes and other
 * restrictions in certain environments. <p>
 *
 * @see java.security.Permission
 *
 * @version 1.7 06/24/03
 * @author Jan Luehe
 * @author Sharon Liu
 * @since 1.4
 */
class CryptoPermission extends java.security.Permission {
    private String alg;
    private int maxKeySize = -1; // no restriction on maxKeySize 
    private String exemptionMechanism = null;
    private AlgorithmParameterSpec algParamSpec = null;

    static final String ALG_NAME_WILDCARD = "*";
    
    /**
     * Constructor that takes an algorithm name.
     *
     * This constructor implies that the given algorithm can be
     * used without any restrictions.
     *
     * @param alg the algorithm name.
     */
    CryptoPermission(String alg) {
	super(null);
	this.alg = alg;
    }

    /**
     * Constructor that takes an algorithm name and a maximum
     * key size.
     *
     * This constructor implies that the given algorithm can be
     * used with a key size up to <code>maxKeySize</code>.
     *
     * @param alg the algorithm name.
     *
     * @param maxKeySize the maximum allowable key size,
     * specified in number of bits.
     */
    CryptoPermission(String alg, int maxKeySize) {
	super(null);
        this.alg = alg;
	this.maxKeySize = maxKeySize;
    }

    /**
     * Constructor that takes an algorithm name, a maximum
     * key size, and an AlgorithmParameterSpec object.
     *
     * This constructor implies that the given algorithm can be
     * used with a key size up to <code>maxKeySize</code>, and
     * algorithm
     * parameters up to the limits set in <code>algParamSpec</code>.
     *
     * @param alg the algorithm name.
     *
     * @param maxKeySize the maximum allowable key size,
     * specified in number of bits.
     *
     * @param algParamSpec the limits for allowable algorithm
     * parameters.
     */
    CryptoPermission(String alg,
		     int maxKeySize,
		     AlgorithmParameterSpec algParamSpec) {
	super(null);
        this.alg = alg;
	this.maxKeySize = maxKeySize;
	this.algParamSpec = algParamSpec;
    }

    /**
     * Constructor that takes an algorithm name and the name of
     * an exemption mechanism.
     *
     * This constructor implies that the given algorithm can be
     * used without any key size or algorithm parameter restrictions
     * provided that the specified exemption mechanism is enforced.
     *
     * @param alg the algorithm name.
     *
     * @param exemptionMechanism the name of the exemption mechanism.
     */
    CryptoPermission(String alg,
		     String exemptionMechanism) {
	super(null);
        this.alg = alg;
	this.exemptionMechanism = exemptionMechanism;
    }

    /**
     * Constructor that takes an algorithm name, a maximum key
     * size, and the name of an exemption mechanism.
     *
     * This constructor implies that the given algorithm can be
     * used with a key size up to <code>maxKeySize</code>
     * provided that the
     * specified exemption mechanism is enforced.
     *
     * @param alg the algorithm name.
     * @param maxKeySize the maximum allowable key size,
     * specified in number of bits.
     * @param exemptionMechanism the name of the exemption 
     * mechanism.
     */
    CryptoPermission(String alg,
		     int maxKeySize,
		     String exemptionMechanism) {
	super(null);
        this.alg = alg;
	this.exemptionMechanism = exemptionMechanism;
	this.maxKeySize = maxKeySize;
    }

    /**
     * Constructor that takes an algorithm name, a maximum key
     * size, the name of an exemption mechanism, and an
     * AlgorithmParameterSpec object.
     *
     * This constructor implies that the given algorithm can be
     * used with a key size up to <code>maxKeySize</code>
     * and algorithm
     * parameters up to the limits set in <code>algParamSpec</code>
     * provided that
     * the specified exemption mechanism is enforced.
     *
     * @param alg the algorithm name.
     * @param maxKeySize the maximum allowable key size,
     * specified in number of bits.
     * @param algParamSpec the limit for allowable algorithm
     *  parameter spec.
     * @param exemptionMechanism the name of the exemption
     * mechanism.
     */
    CryptoPermission(String alg,			    
		     int maxKeySize,
		     AlgorithmParameterSpec algParamSpec,
		     String exemptionMechanism) {
	super(null);
        this.alg = alg;
	this.exemptionMechanism = exemptionMechanism;
	this.maxKeySize = maxKeySize;
	this.algParamSpec = algParamSpec;
    }

    /**
     * Checks if the specified permission is "implied" by
     * this object.
     * <p>
     * More specifically, this method returns true if:<p>
     * <ul>
     * <li> <i>p</i> is an instance of CryptoPermission, and<p>
     * <li> <i>p</i>'s algorithm name equals or (in the case of wildcards)
     *       is implied by this permission's algorithm name, and<p>
     * <li> <i>p</i>'s maximum allowable key size is less or
     *       equal to this permission's maximum allowable key size, and<p>
     * <li> <i>p</i>'s algorithm parameter spec equals or is
     *        implied by this permission's algorithm parameter spec, and<p>
     * <li> <i>p</i>'s exemptionMechanism equals or
     *        is implied by this permission's
     *        exemptionMechanism (a <code>null</code> exemption mechanism
     *        implies any other exemption mechanism).
     * </ul>
     *
     * @param p the permission to check against.
     *
     * @return true if the specified permission is equal to or
     * implied by this permission, false otherwise.
     */
    public boolean implies(Permission p) {
	if (!(p instanceof CryptoPermission))
	    return false;

	CryptoPermission cp = (CryptoPermission)p;

	if ((!alg.equalsIgnoreCase(cp.alg)) &&
	    (!alg.equalsIgnoreCase(ALG_NAME_WILDCARD))) {
	    return false;
	}

	// alg is the same as cp's alg or 
	// alg is a wildcard.
	if ((this.maxKeySize == -1) || 
	    ((cp.maxKeySize != -1) && (cp.maxKeySize <= this.maxKeySize))) {
	    // check algParamSpec.
	    if (!impliesParameterSpec(cp.algParamSpec)) {
		return false;
	    }		

	    // check exemptionMechanism.
	    if (impliesExemptionMechanism(cp.exemptionMechanism)) {
		return true;
	    }
	}

	return false;	
    }

    /**
     * Checks two CryptoPermission objects for equality. Checks that 
     * <code>obj</code> is a CryptoPermission, and has the same 
     * algorithm name,
     * exemption mechanism name, maximum allowable key size and
     * algorithm parameter spec
     * as this object.
     * <P>
     * @param obj the object to test for equality with this object.
     * @return true if <code>obj</code> is equal to this object.
     */
    public boolean equals(Object obj) {
	if (obj == this)
	    return true;

	if (!(obj instanceof CryptoPermission))
	    return false;

	CryptoPermission that = (CryptoPermission) obj;

	if (!(alg.equalsIgnoreCase(that.alg)) ||
	    (maxKeySize != that.maxKeySize)) {
	    return false;
	}

	return (equalObjects(this.exemptionMechanism,
			     that.exemptionMechanism) &&
	        equalObjects(this.algParamSpec,
			     that.algParamSpec));
    }

    /**
     * Returns the hash code value for this object.
     * 
     * @return a hash code value for this object.
     */

    public int hashCode() {
	int retval = alg.hashCode();
	retval ^= maxKeySize;
	if (exemptionMechanism != null) {
	    retval ^= exemptionMechanism.hashCode();
	}
	if (algParamSpec != null) {
	    retval ^= algParamSpec.hashCode();
	}
	return retval;
    }

    /**
     * There is no action defined for a CryptoPermission
     * onject.
     */
    public String getActions()
    {
	return null;
    }

    /**
     * Returns a new PermissionCollection object for storing
     * CryptoPermission objects.
     *
     * @return a new PermissionCollection object suitable for storing 
     * CryptoPermissions.
     */

    public PermissionCollection newPermissionCollection() {
	return new CryptoPermissionCollection();
    } 

    /**
     * Returns the algorithm name associated with
     * this CryptoPermission object.
     */
    final String getAlgorithm() {
	return alg;
    }

    /**
     * Returns the exemption mechanism name
     * associated with this CryptoPermission
     * object.
     */
    final String getExemptionMechanism() {
	return exemptionMechanism;
    }

    /**
     * Returns the maximum allowable key size associated
     * with this CryptoPermission object.
     */
    final int getMaxKeySize() {
	return maxKeySize;
    }

    /**
     * Returns the AlgorithmParameterSpec
     * associated with this CryptoPermission
     * object.
     */
    final AlgorithmParameterSpec getAlgorithmParameterSpec() {
	return algParamSpec;
    }

    /**
     * Returns a string describing this CryptoPermission.  The convention is to
     * specify the class name, the algorithm name, the maximum allowable
     * key size, and the name of the exemption mechanism, in the following
     * format: '("ClassName" "algorithm" "keysize" "exemption_mechanism")'.
     * 
     * @return information about this CryptoPermission.
     */
    public String toString() {
	if (exemptionMechanism != null) { // OPTIONAL
	    return "(" + getClass().getName() + " " + alg + " " + maxKeySize
		+ " " + exemptionMechanism + ")";
	} else {
	    return "(" + getClass().getName() + " " + alg + " " + maxKeySize
		+ ")";
	}
    }

    private boolean impliesExemptionMechanism(String exemptionMechanism) {
	if (this.exemptionMechanism == null) {
	    return true;
	}

	if (exemptionMechanism == null) {
	    return false;
	}

	if (this.exemptionMechanism.equals(exemptionMechanism)) {
	    return true;
	}

	return false;
    }

    private boolean impliesParameterSpec(AlgorithmParameterSpec
					 algParamSpec)
    {
	if (this.algParamSpec == null) {
	    return true;
	}

	if (algParamSpec == null) {
	    return false;
	}

	if (this.algParamSpec.getClass() != algParamSpec.getClass()) {
	    return false;
	}

	if (algParamSpec instanceof RC2ParameterSpec) {
	    if (((RC2ParameterSpec)algParamSpec).getEffectiveKeyBits() <= 
		((RC2ParameterSpec)
		     (this.algParamSpec)).getEffectiveKeyBits()) {
		return true;
	    }
	}

	if (algParamSpec instanceof RC5ParameterSpec) {
	    if (((RC5ParameterSpec)algParamSpec).getRounds() <= 
	        ((RC5ParameterSpec)this.algParamSpec).getRounds()) {
		return true;
	    }
	}

	if (algParamSpec instanceof PBEParameterSpec) {
	    if (((PBEParameterSpec)algParamSpec).getIterationCount() <=
		((PBEParameterSpec)this.algParamSpec).getIterationCount()) {
		return true;
	    }
	}

	// For classes we don't know, the following
	// may be the best try.
	if (this.algParamSpec.equals(algParamSpec)) {
	    return true;
	}

	return false;
    }

    private boolean equalObjects(Object obj1, Object obj2) {
	if (obj1 == null) {
	    return (obj2 == null ? true : false);
	}

	return obj1.equals(obj2);
    }
}

/**
 * A CryptoPermissionCollection stores a set of CryptoPermission
 * permissions.
 *
 * @see java.security.Permission
 * @see java.security.Permissions
 * @see java.security.PermissionCollection
 *
 * @version 1.7 06/24/03
 *
 * @author Sharon Liu
 */
final class CryptoPermissionCollection extends PermissionCollection
implements Serializable {

    private Vector permissions;

    /**
     * Creates an empty CryptoPermissionCollection
     * object.
     */
    CryptoPermissionCollection() {
	permissions = new Vector(3);
    }

    /**
     * Adds a permission to the CryptoPermissionCollection.
     *
     * @param permission the Permission object to add.
     *
     * @exception SecurityException - if this CryptoPermissionCollection
     * object has been marked <i>readOnly</i>.
     */
    public void add(Permission permission)
    {
	if (isReadOnly())
	    throw new SecurityException("attempt to add a Permission " +
					"to a readonly PermissionCollection");

        if (!(permission instanceof CryptoPermission))
            return;
	    
	permissions.addElement(permission);
    }

    /**
      * Check and see if this CryptoPermission object  implies 
      * the given Permission object.
     *
     * @param p the Permission object to compare
     *
     * @return true if the given permission  is implied by this
     * CryptoPermissionCollection, false if not.
     */
    public boolean implies(Permission permission) {
	if (!(permission instanceof CryptoPermission))
	    return false;

	CryptoPermission cp = (CryptoPermission)permission;

	Enumeration e = permissions.elements();

	while (e.hasMoreElements()) {
	    CryptoPermission x = (CryptoPermission) e.nextElement();
	    if (x.implies(cp)) {
		return true;
	    } 
	}
	return false;
    }

    /**
     * Returns an enumeration of all the CryptoPermission objects 
     * in the container.
     *
     * @return an enumeration of all the CryptoPermission objects.
     */

    public Enumeration elements()
    {
	return permissions.elements();
    }
}
