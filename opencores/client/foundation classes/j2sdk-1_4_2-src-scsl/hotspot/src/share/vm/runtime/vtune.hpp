#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vtune.hpp	1.12 03/01/23 12:27:23 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Interface to Intel's VTune profiler. 

class VTune : AllStatic {
 public:
#ifndef CORE
   static void create_nmethod(nmethod* nm);      // register newly created nmethod
   static void delete_nmethod(nmethod* nm);      // unregister nmethod before discarding it
#endif

   static void register_stub(const char* name, address start, address end);    
                                                 // register internal VM stub
   static void start_GC();                       // start/end of GC or scavenge
   static void end_GC();

   static void start_class_load();               // start/end of class loading
   static void end_class_load();  

   static void exit();                           // VM exit
};


// helper objects
class VTuneGCMarker : StackObj {
 public:
   VTuneGCMarker() { VTune::start_GC(); }
  ~VTuneGCMarker() { VTune::end_GC(); }
};

class VTuneClassLoadMarker : StackObj {
 public:
   VTuneClassLoadMarker() { VTune::start_class_load(); }
  ~VTuneClassLoadMarker() { VTune::end_class_load(); }
};

