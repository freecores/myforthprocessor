/*
 * @(#)ClassDump.java	1.4 03/01/23 11:46:58
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.tools.jcore;

import java.io.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.tools.*;

public class ClassDump extends Tool {
   private ClassFilter classFilter;
   private String      outputDirectory;

   public void run() {
      // Ready to go with the database...
      try {

         // load class filters

         String filterClassName = System.getProperty("sun.jvm.hotspot.tools.jcore.filter");
         if (filterClassName != null) {
            try {
               Class filterClass = Class.forName(filterClassName);
               classFilter = (ClassFilter) filterClass.newInstance();
            } catch(Exception exp) {
               System.err.println("Warning: Can not create class filter!");
            }
         }

         outputDirectory = System.getProperty("sun.jvm.hotspot.tools.jcore.outputDir");
         if (outputDirectory == null) 
            outputDirectory = ".";

         // walk through the system dictionary
         SystemDictionary dict = VM.getVM().getSystemDictionary();
         dict.classesDo(new SystemDictionary.ClassVisitor() {
                              public void visit(Klass k) {
                                 if (k instanceof InstanceKlass)
                                   dumpKlass((InstanceKlass) k);
                              }
                       });
      }
      catch (AddressException e) {
         System.err.println("Error accessing address 0x" 
                                + Long.toHexString(e.getAddress()));
         e.printStackTrace();
      }
   }

   public String getName() {
      return "jcore";
   }

   private void dumpKlass(InstanceKlass kls) {
      if (classFilter != null && ! classFilter.canInclude(kls) ) {
         return;
      }

      String klassName = kls.getName().asString();
      klassName = klassName.replace('/', File.separatorChar);
      int index = klassName.lastIndexOf(File.separatorChar);
      File dir = null;
      if (index != -1) {
        String dirName = klassName.substring(0, index);
        dir =  new File(outputDirectory,  dirName);
      } else {
        dir = new File(outputDirectory);
      }
 
      dir.mkdirs();
      File f = new File(dir, klassName.substring(klassName.lastIndexOf(File.separatorChar) + 1)
                              + ".class");
      try {
         f.createNewFile();
         FileOutputStream fis = new FileOutputStream(f);
         ClassWriter cw = new ClassWriter(kls, fis);
         cw.write();
      } catch(IOException exp) {
         exp.printStackTrace(); 
      } 
   }

   public static void main(String[] args) {
      ClassDump cd = new ClassDump();
      cd.start(args);
      cd.stop();
   }
}
