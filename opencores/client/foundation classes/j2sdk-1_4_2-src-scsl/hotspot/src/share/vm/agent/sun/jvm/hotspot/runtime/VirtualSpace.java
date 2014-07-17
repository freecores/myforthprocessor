/*
 * @(#)VirtualSpace.java	1.5 03/01/23 11:46:19
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

public class VirtualSpace extends VMObject {
  private static AddressField lowField;
  private static AddressField highField;
  private static AddressField lowBoundaryField;
  private static AddressField highBoundaryField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("VirtualSpace");
    
    lowField          = type.getAddressField("_low");
    highField         = type.getAddressField("_high");
    lowBoundaryField  = type.getAddressField("_low_boundary");
    highBoundaryField = type.getAddressField("_high_boundary");
  }

  public VirtualSpace(Address addr) {
    super(addr);
  }

  public Address low()                          { return lowField.getValue(addr);          }
  public Address high()                         { return highField.getValue(addr);         }
  public Address lowBoundary()                  { return lowBoundaryField.getValue(addr);  }
  public Address highBoundary()                 { return highBoundaryField.getValue(addr); }

  /** Testers (all sizes are byte sizes) */
  public long committedSize()                   { return high().minus(low());                                    }
  public long reservedSize()                    { return highBoundary().minus(lowBoundary());                    }
  public long uncommittedSize()                 { return reservedSize() - committedSize();                       }
  public boolean contains(Address addr)         { return (low().lessThanOrEqual(addr) && addr.lessThan(high())); }
}
