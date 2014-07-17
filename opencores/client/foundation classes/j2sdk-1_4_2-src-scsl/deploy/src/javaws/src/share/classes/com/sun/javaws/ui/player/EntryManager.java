/*
 * @(#)EntryManager.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.player;

import java.beans.*;
import java.net.URL;
import javax.swing.event.*;

/**
 * An EntryManager maintains a set of JNLEntrys. This being an abstract
 * class, it doesn't define where they come from, that is up to a concrete
 * implementation to define.
 * <p>
 * Two listener types are supported: a ChangeListener this is notified
 * when the set of JNLEntrys some how changes, and a PropertyChangeListener
 * for the bound properties (such as selected index and state). If the
 * <code>refresh</code> method will take a while (perhaps involving loading
 * from a web page) it is recommended it return immediately and do the
 * loading in another thread. Listeners should then be notified as the
 * entries are found and loaded.
 * <p>
 * The selectedIndex property doesn't really make sense in this class, and
 * will likely be removed sometime in the future.
 *
 * @version 1.7 02/14/01
 */
public abstract class EntryManager {
    /**
     * Indicates that the EntryManager has loaded everything.
     */
    public static final int LOADED = 0;
    /**
     * Indicates that the EntryManager is in the process of loading the
     * JNLEntries.
     */
    public static final int LOADING = 1;

    /**
     * PropertyChangeListeners
     */
    private SwingPropertyChangeSupport _changeSupport;
    /**
     * The currently selected JNLEntry.
     */
    private int _selectedIndex;
    /**
     * EventListeners, currently holds ChangeListeners, but that may
     * change.
     */
    private EventListenerList _listenerList;
    /**
     * The global change event.
     */
    private ChangeEvent _changeEvent;


    public EntryManager() {
        _listenerList = new EventListenerList();
        _selectedIndex = -1;
    }

    /**
     * Sets the selected index. This is a bound property.
     */
    public void setSelectedIndex(int index) {
        int oldValue = _selectedIndex;
        _selectedIndex = index;
        firePropertyChange("selectedIndex", oldValue, index);
    }

    /**
     * Returns the selected entry as an index.
     */
    public int getSelectedIndex() {
        return _selectedIndex;
    }

    /**
     * Selects the entry with the passed in codebase. If none of the entries
     * have a codebase equal to <code>codebase</code>, the selection is not
     * changed.
     */
    public void selectEntryWithCodebase(URL codebase) {
        if (codebase != null) {
            String codebaseS = codebase.toExternalForm();

            for (int counter = size() - 1; counter >= 0; counter--) {
                JNLEntry entry = get(counter);

                if (entry != null && entry.getCodebase() != null &&
                    codebaseS.equals(entry.getCodebase().toExternalForm())) {
                    setSelectedIndex(counter);
                    break;
                }
            }
        }
    }

    /**
     * Returns the currently selected JNLEntry, or null if there is no
     * selection.
     */
    public JNLEntry getSelectedEntry() {
        int index = getSelectedIndex();
        if (index >= 0 && index < size()) {
            return get(index);
        }
        return null;
    }

    /**
     * Returns the JNLEntry at the specified index.
     */
    public abstract JNLEntry get(int index);

    /**
     * Returns the number of JNLEntries.
     */
    public abstract int size();

    /**
     * Refreshes the current set of JNLEntries. It is up to subclasses to
     * determine just what the heck this does. If this is can take a while
     * (say involves a network connection) this should spawn a thread and
     * return immediately, then notify listeners as the entries are read.
     */
    public abstract void refresh();

    /**
     * Saves any pending state. This may not be appropriate for all
     * EntryManagers. The default implementation does nothing.
     */
    public void save() {
    }

    /**
     * Returns a user presentable string as to where this EntryManager is
     * getting its entries from.
     */
    public String getDescription() {
        String resourceName = getClass().getName();
        int lastDot = resourceName.lastIndexOf('.');
        if (lastDot != -1) {
            resourceName = resourceName.substring(lastDot + 1);
        }
        resourceName = "player." + resourceName + ".label";
        return com.sun.javaws.Resources.getString(resourceName);
    }

    /**
     * Return true if the user can add entries to this EntryManager. This
     * may not make sense for all EntryMangers, and the default is to return
     * false.
     */
    public boolean canAdd() {
        return false;
    }

    /**
     * Adds a new JNLEntry at the specified index. This will only be invoked
     * if <code>canAdd</code> returns true. The default implementation does
     * nothing.
     */
    public void add(int index, JNLEntry entry) {
    }

    /**
     * Returns true if the user can remove entries from this EntryManger.
     * This may not make sense for all EntryMangers, and the default is to
     * return false.
     */
    public boolean canRemove() {
        return false;
    }

    /**
     * Returns a string that can be presented to the user when an entry
     * is removed.
     */
    public String getRemoveString() {
        return null;
    }

    /**
     * Removes the JNLEntry at the specified index. This will only be
     * invoked if <code>canRemove</code> return true. The default
     * implementation simply resets the selected index if it is beyond the
     * new <code>size</code>.
     */
    public void remove(int index) {
        if (getSelectedIndex() >= size()) {
            setSelectedIndex(size() - 1);
        }
        fireStateChanged();
    }

    /**
     * Returns the current state of the EntryManager, either
     * <code>LOADED</code> or <code>LOADING</code>. While there is
     * no setter, this is a bound property.
     */
    public int getState() {
        return LOADED;
    }

    //
    // Miscelaneous Listener type methods.
    //
    public void firePropertyChange(String propertyName, int oldValue,
                                   int newValue) {
        if ((_changeSupport != null) && (oldValue != newValue)) {
            _changeSupport.firePropertyChange(propertyName,
                                              new Integer(oldValue),
                                              new Integer(newValue));
        }
    }

    protected void firePropertyChange(String propertyName, Object oldValue,
                                      Object newValue) {
        if (_changeSupport != null) {
            _changeSupport.firePropertyChange(propertyName, oldValue,
                                              newValue);
        }
    }

    protected void fireStateChanged() {
        // Guaranteed to return a non-null array
        Object[] listeners = _listenerList.getListenerList();
        // Process the listeners last to first, notifying
        // those that are interested in this event
        for (int i = listeners.length-2; i>=0; i-=2) {
            if (listeners[i]==ChangeListener.class) {
                // Lazily create the event:
                if (_changeEvent == null)
                    _changeEvent = new ChangeEvent(this);
                ((ChangeListener)listeners[i+1]).stateChanged(_changeEvent);
            }          
        }
    }   

    public void addChangeListener(ChangeListener l) {
        _listenerList.add(ChangeListener.class, l);
    }
    
    public void removeChangeListener(ChangeListener l) {
        _listenerList.remove(ChangeListener.class, l);
    }

    public synchronized void addPropertyChangeListener
                                 (PropertyChangeListener listener) {
	if (_changeSupport == null) {
	    _changeSupport = new SwingPropertyChangeSupport(this);
	}
        _changeSupport.addPropertyChangeListener(listener);
    }

    public synchronized void removePropertyChangeListener
                                 (PropertyChangeListener listener) {
	if (_changeSupport != null) {
            _changeSupport.removePropertyChangeListener(listener);
	}
    }
}
