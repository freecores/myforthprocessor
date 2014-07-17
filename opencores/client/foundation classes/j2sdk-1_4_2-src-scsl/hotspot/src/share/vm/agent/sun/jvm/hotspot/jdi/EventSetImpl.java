/*
 * @(#)EventSetImpl.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jvm.hotspot.jdi;
import com.sun.jdi.*;
import com.sun.jdi.event.*;
import com.sun.jdi.request.*;

import java.util.*;

public class EventSetImpl extends ArrayList implements EventSet {
    
    private VirtualMachineImpl vm; // we implement Mirror
    private byte suspendPolicy;
    private boolean needsFiltering = false;

    public String toString() {
        String string = "event set, policy:" + suspendPolicy + 
                        ", count:" + this.size() + " = {";
        Iterator iter = this.iterator();
        boolean first = true;
        while (iter.hasNext()) {
            Event event = (Event)iter.next();
            if (!first) {
                string += ", ";
            }
            string += event.toString();
            first = false;
        }
        string += "}";
        return string;
    }

    EventSetImpl(VirtualMachine aVm) {
        super();
        vm = (VirtualMachineImpl)aVm;
    }
        
    /** 
     * Constructor for special events like VM disconnected
     */
    EventSetImpl(VirtualMachine aVm, byte eventCmd) {
        this(aVm);
        suspendPolicy = 0;
        switch (eventCmd) {
            case EventRequestManagerImpl.EventKind.VM_DISCONNECTED:
                // jjh super.add(new VMDisconnectEventImpl());
                break;

            default:
                throw new InternalException("Bad singleton event code");
        }       
    }

    /** 
     * Constructor for filtered cloning
     */
    EventSetImpl(EventSetImpl es, boolean internal) {
        this(es.vm);
        suspendPolicy = es.suspendPolicy;
//jjh      for (Iterator it = es.iterator(); it.hasNext();) {
//             EventImpl evt = (EventImpl)it.next();
//             if (evt.internalEvent == internal) {
//                 super.add(evt);
//             }
//         }
    }

    /** 
     * Filter out internal events.
     */
    EventSet userFilter() {
        if (needsFiltering) {
            return new EventSetImpl(this, false);
        } else {
            return this;
        }
    }       
            
    /** 
     * Filter out user events.
     */
    EventSet internalFilter() {
        if (needsFiltering) {
            return new EventSetImpl(this, true);
        } else {
            return null;
        }
    }       

    synchronized void build() {
    }

    public VirtualMachine virtualMachine() {
        return vm;
    }

    public int suspendPolicy() {
        return suspendPolicy;
    }

    public void resume() {
    }

    public Iterator iterator() {
        return new Itr();
    }

    public EventIterator eventIterator() {
        return new Itr();
    }

    public class Itr implements EventIterator {
	/**
	 * Index of element to be returned by subsequent call to next.
	 */
	int cursor = 0;

	public boolean hasNext() {
	    return cursor != size();
	}

	public Object next() {
	    try {
		Object nxt = get(cursor);
                ++cursor;
                return nxt;
	    } catch(IndexOutOfBoundsException e) {
		throw new NoSuchElementException();
	    }
	}

        public Event nextEvent() {
            return (Event)next();
        }

	public void remove() {
            throw new UnsupportedOperationException();
	}
    }

    /* below make this unmodifiable */

    public boolean add(Object o){
        throw new UnsupportedOperationException();
    }
    public boolean remove(Object o) {
        throw new UnsupportedOperationException();
    }
    public boolean addAll(Collection coll) {
        throw new UnsupportedOperationException();
    }
    public boolean removeAll(Collection coll) {
        throw new UnsupportedOperationException();
    }
    public boolean retainAll(Collection coll) {
        throw new UnsupportedOperationException();
    }
    public void clear() {
        throw new UnsupportedOperationException();
    }
}

