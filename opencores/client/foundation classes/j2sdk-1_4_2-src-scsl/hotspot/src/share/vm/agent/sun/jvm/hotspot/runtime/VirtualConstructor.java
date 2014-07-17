/*
 * @(#)VirtualConstructor.java	1.4 03/01/23 11:46:17
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

/** This class provides generalized "virtual constructor"
    functionality for VMObjects. In simple terms, it creates
    correctly-typed Java wrapper objects for underlying Addresses,
    using the "RTTI-like" functionality of TypeDataBase. For example,
    if the given Address really is a TrainGeneration*, the Java object
    created for it will be of type
    sun.jvm.hotspot.memory.TrainGeneration, assuming the mapping from
    type "TrainGeneration" to class
    sun.jvm.hotspot.memory.TrainGeneration has been set up. */

public class VirtualConstructor {
  private TypeDataBase db;
  private Map          map; // Map<String, Class>

  public VirtualConstructor(TypeDataBase db) {
    this.db = db;
    map     = new HashMap();
  }

  /** Adds a mapping from the given C++ type name to the given Java
      class. The latter must be a subclass of
      sun.jvm.hotspot.runtime.VMObject. Returns false if there was
      already a class for this type name in the map. */
  public boolean addMapping(String cTypeName, Class clazz) {
    if (map.get(cTypeName) != null) {
      return false;
    }

    map.put(cTypeName, clazz);
    return true;
  }

  /** Instantiate the most-precisely typed wrapper object available
      for the type of the given Address. If no type in the mapping
      matched the type of the Address, throws a WrongTypeException.
      Returns null for a null address (similar behavior to
      VMObjectFactory). */
  public VMObject instantiateWrapperFor(Address addr) throws WrongTypeException {
    if (addr == null) {
      return null;
    }

    for (Iterator iter = map.keySet().iterator(); iter.hasNext(); ) {
      String typeName = (String) iter.next();
      if (db.addressTypeIsEqualToType(addr, db.lookupType(typeName))) {
        return (VMObject) VMObjectFactory.newObject((Class) map.get(typeName), addr);
      }
    }
    throw new WrongTypeException("No suitable match for type of address " + addr);
  }
}
