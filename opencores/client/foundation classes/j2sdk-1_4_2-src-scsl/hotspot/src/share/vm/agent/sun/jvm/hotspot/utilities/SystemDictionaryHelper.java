/*
 * @(#)SystemDictionaryHelper.java	1.2 03/01/23 11:52:15
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

import java.util.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.runtime.*;

public class SystemDictionaryHelper {
   static {
      VM.registerVMInitializedObserver(new Observer() {
         public void update(Observable o, Object data) {
            initialize();
         }
      });
   }

   private static synchronized void initialize() {
      klasses = null;
   }

   // Instance klass array sorted by name.
   private static InstanceKlass[] klasses;

   // side-effect!. caches the instance klass array.
   public static synchronized InstanceKlass[] getAllInstanceKlasses() {
      if (klasses != null) {
         return klasses; 
      }

      final Vector tmp = new Vector();
      SystemDictionary dict = VM.getVM().getSystemDictionary();
      dict.classesDo(new SystemDictionary.ClassVisitor() {
                        public void visit(Klass k) {
                           if (k instanceof InstanceKlass) {
                              InstanceKlass ik = (InstanceKlass) k;
                              tmp.add(ik);
                           }
                        }
                     });

      Object[] tmpArray = tmp.toArray();
      klasses = new InstanceKlass[tmpArray.length];  
      System.arraycopy(tmpArray, 0, klasses, 0, tmpArray.length);
      Arrays.sort(klasses, new Comparator() {
                          public int compare(Object o1, Object o2) {
                             InstanceKlass k1 = (InstanceKlass) o1;
                             InstanceKlass k2 = (InstanceKlass) o2;
                             Symbol s1 = k1.getName();
                             Symbol s2 = k2.getName();
                             return s1.asString().compareTo(s2.asString());
                          }
                      });
      return klasses;
   }

   // returns array of instance klasses whose name contains given namePart
   public static InstanceKlass[] findInstanceKlasses(String namePart) {
      namePart = namePart.replace('.', '/');
      InstanceKlass[] tmpKlasses = getAllInstanceKlasses();

      Vector tmp = new Vector();
      for (int i = 0; i < tmpKlasses.length; i++) {
         String name = tmpKlasses[i].getName().asString();
         if (name.indexOf(namePart) != -1) {
            tmp.add(tmpKlasses[i]);
         }
      }

      Object[] tmpArray = tmp.toArray();
      InstanceKlass[] searchResult = new InstanceKlass[tmpArray.length];  
      System.arraycopy(tmpArray, 0, searchResult, 0, tmpArray.length);
      return searchResult;
   }
}
