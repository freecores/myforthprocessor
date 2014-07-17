/*
 * @(#)AbstractController.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.general;

import java.util.ArrayList;

/**
 * Maintains an ordered collection of SubControllers, as well as the notion
 * of an active (selected) Subcontroller. As the selected Subcontroller
 * changes, the old Subcontroller is invoked with <code>stop</code> and
 * the new invoked with <code>start</code>.
 * <p>
 * Subclasses must implement <code>createSubcontroller</code> to handle
 * the creation of the actual Subcontroller.
 */
public abstract class AbstractController {
    /** The SubControllers. */
    private ArrayList subcontrollers;
    /** Index of the active SubController. */
    private int activeSubcontrollerIndex;

    public AbstractController() {
	activeSubcontrollerIndex = -1;
	subcontrollers = new ArrayList();
    }

    public void setActiveSubcontrollerIndex(int index) {
	if (index == activeSubcontrollerIndex) {
	    return;
	}

	Subcontroller sc = getActiveSubcontroller();

	if (sc != null) {
	    sc.stop();
	}

	ensureSize(index + 1);

	activeSubcontrollerIndex = index;
	if (index >= 0) {
	    sc = (Subcontroller)subcontrollers.get(index);
	    if (sc == null) {
	        sc = createSubcontroller(index);
	        subcontrollers.set(index, sc);
	    }
	    if (sc != null) {
	      sc.start();
	    }
	}
    }

    public void setSubcontroller(int index, Subcontroller controller) {
	ensureSize(index + 1);
	if (index == getActiveSubcontrollerIndex()) {
	    getActiveSubcontroller().stop();
	}
	subcontrollers.set(index, controller);
	controller.start();
    }

    public int getActiveSubcontrollerIndex() {
	return activeSubcontrollerIndex;
    }

    public Subcontroller getActiveSubcontroller() {
	if (activeSubcontrollerIndex == -1) {
	    return null;
	}
	return (Subcontroller)subcontrollers.get(activeSubcontrollerIndex);
    }

    /**
     * Returns the Subcontroller at <code>index</code>. This will NOT
     * create the Subcontroller if it hasn't been created yet.
     */
    public Subcontroller getSubcontroller(int index) {
	if (index >= subcontrollers.size()) {
	    return null;
	}
	return (Subcontroller)subcontrollers.get(index);
    }

    public void apply(boolean toAll) {
	if (toAll) {
	    for (int counter = subcontrollers.size() - 1; counter >= 0;
		 counter--) {
		Subcontroller controller = (Subcontroller)subcontrollers.
		                           get(counter);
		if (controller != null) {
		    controller.apply();
		}
	    }
	}
	else {
	    Subcontroller as = getActiveSubcontroller();
	    if (as != null) {
		as.apply();
	    }
	}
    }

    public void revert(boolean toAll) {
	if (toAll) {
	    for (int counter = subcontrollers.size() - 1; counter >= 0;
		 counter--) {
		Subcontroller controller = (Subcontroller)subcontrollers.
		                           get(counter);
		if (controller != null) {
		    controller.revert();
		}
	    }
	}
	else {
	    Subcontroller as = getActiveSubcontroller();
	    if (as != null) {
		as.revert();
	    }
	}
    }

    protected abstract Subcontroller createSubcontroller(int index);

    private void ensureSize(int index) {
	int count = subcontrollers.size();
	while (count < index) {
	    subcontrollers.add(null);
	    count++;
	}
    }
}
