/*
 * @(#)ModalityListener.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

/**
 * Listener interface so Java Plug-in can be notified
 * of changes in AWT modality
 *
 * May be migrated to sun.awt at a later date.
 */
public interface ModalityListener {
   /**
    * Called by AWT when it enters a new level of modality
    */
    public void modalityPushed(ModalityEvent ev);
    
   /**
    * Called by AWT when it exits a level of modality
    */
    public void modalityPopped(ModalityEvent ev);
}
