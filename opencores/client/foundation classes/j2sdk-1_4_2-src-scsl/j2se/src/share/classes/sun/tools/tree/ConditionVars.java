/*
 * @(#)ConditionVars.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

/**
 * This class is used to hold two sets of variables,
 * one for the true branch, one for the false branch.
 */
class ConditionVars {
    Vset vsTrue;
    Vset vsFalse;
}
