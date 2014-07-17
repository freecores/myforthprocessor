#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)onStackReplacement.hpp	1.21 03/01/23 12:00:08 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifdef COMPILER2
// Class OSRAdapter describes an interpreter frame that has been 
// on-stack replaced. All oops will be dead in this frame. 
// This adapter frame keeps track of the size of the frame, and
// where the link (ebp on Win32) is stored, so stack-traversal
// works.
//
class OSRAdapter: public CodeBlob {
  friend class VMStructs;
  
 private:
  OSRAdapter::OSRAdapter(CodeBuffer *cb, OopMapSet *oop_maps, int size, int frame_size);
  void* operator new(size_t s, unsigned size);

 public:
  // Creation
  static OSRAdapter *new_osr_adapter(CodeBuffer *cb, OopMapSet *oop_maps, int frame_size);

  // Define Codeblob behaviour
  bool is_osr_adapter()                  const  { return true; }
  bool is_alive()                        const  { return true; }
  void preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map,
				     OopClosure* f)
  { /* nothing to do */ }  
  void follow_roots_or_mark_for_unloading(bool unloading_occurred, bool& marked_for_unloading)     { /* do-nothing*/ }
  void follow_roots_or_mark_for_unloading_for_CMS(
    CMSIsAliveClosure* is_alive, CMSKeepAliveClosure* keep_alive,
    bool unloading_occurred, bool& marked_for_unloading)
  { /* do-nothing*/ }

  // Iteration.
  void oops_do(OopClosure* f) {}        

  // Debugging code
  void verify()                                PRODUCT_RETURN; 
  void print()                          const  PRODUCT_RETURN;
  void print_value_on(outputStream* st) const  PRODUCT_RETURN; 
};
#endif // COMPILER2


// Namespace for OnStackReplacement functionality

class OnStackReplacement: public AllStatic {
#ifdef COMPILER2
 private:
  // List of adapters for different frame sizes.
  static GrowableArray<OSRAdapter*>* _osr_adapters_other;
  // Same, for adapters that return a float or double
  static GrowableArray<OSRAdapter*>* _osr_adapters_fp;

  // List of on-stack replacement methods
  static GrowableArray<nmethod*>* _osr_nmethods;

  enum Constants {    
    InitialAdapterVectorSize   = 64,     // Initial size of adapter array
    MinTypicalAdapterSize      = 6,      // The typical range is precomputed 
    MaxTypicalAdapterSize      = 16      //   (framesize is in words)
  };
#endif // COMPILER2

 public:
#ifdef COMPILER2
  // Initialization
  static void initialize();  

  // Returns an OSRAdapter for the given framesize
  static OSRAdapter* get_osr_adapter(int framesize, bool returning_fp);
#endif // COMPILER2
};


