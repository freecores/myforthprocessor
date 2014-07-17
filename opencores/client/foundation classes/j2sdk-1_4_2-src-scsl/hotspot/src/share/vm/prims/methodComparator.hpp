#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)methodComparator.hpp	1.4 03/01/23 12:20:47 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class BciMap;

// methodComparator provides an interface for determining if methods of
// different versions of classes are equivalent or switchable

class MethodComparator {
 private:
  static BytecodeStream *_s_old, *_s_new;
  static constantPoolOop _old_cp, _new_cp;
  static BciMap *_bci_map;
  static bool _switchable_test;
  static GrowableArray<int> *_fwd_jmps;

  static bool args_same(Bytecodes::Code c_old, Bytecodes::Code c_new);
  static bool check_stack_and_locals_size(methodOop old_method, methodOop new_method);

 public:
  // Check if the new method is equivalent to the old one modulo constant pool (EMCP).
  // Intuitive definition: two versions of the same method are EMCP, if they don't differ
  // on the source code level. Practically, we check whether the only difference between
  // method versions is some constantpool indices embedded into the bytecodes, and whether
  // these indices eventually point to the same constants for both method versions.
  static bool methods_EMCP(methodOop old_method, methodOop new_method);

  static bool methods_switchable(methodOop old_method, methodOop new_method, BciMap &bci_map);
};


// ByteCode Index Map. For two versions of the same method, where the new version may contain
// fragments not found in the old version, provides a mapping from an index of a bytecode in
// the old method to the index of the same bytecode in the new method.

class BciMap {
 private:
  int *_old_bci, *_new_st_bci, *_new_end_bci;
  int _cur_size, _cur_pos;
  int _pos;

 public:
  BciMap() {
    _cur_size = 50;
    _old_bci = (int*) malloc(sizeof(int) * _cur_size);
    _new_st_bci = (int*) malloc(sizeof(int) * _cur_size);
    _new_end_bci = (int*) malloc(sizeof(int) * _cur_size);
    _cur_pos = 0;
  }

  ~BciMap() {
    free(_old_bci);
    free(_new_st_bci);
    free(_new_end_bci);
  }

  // Store the position of an added fragment, e.g.
  //
  //                              |<- old_bci
  // -----------------------------------------
  // Old method   |invokevirtual 5|aload 1|...
  // -----------------------------------------
  //                                                         
  //                                 |<- new_st_bci          |<- new_end_bci    
  // --------------------------------------------------------------------
  // New method       |invokevirual 5|aload 2|invokevirtual 6|aload 1|...
  // --------------------------------------------------------------------
  //                                 ^^^^^^^^^^^^^^^^^^^^^^^^ 
  //                                    Added fragment 

  void store_fragment_location(int old_bci, int new_st_bci, int new_end_bci) {
    if (_cur_pos == _cur_size) {
      _cur_size += 10;
      _old_bci = (int*) realloc(_old_bci, sizeof(int) * _cur_size);
      _new_st_bci = (int*) realloc(_new_st_bci, sizeof(int) * _cur_size);
      _new_end_bci = (int*) realloc(_new_end_bci, sizeof(int) * _cur_size);
    }
    _old_bci[_cur_pos] = old_bci;
    _new_st_bci[_cur_pos] = new_st_bci;
    _new_end_bci[_cur_pos] = new_end_bci;
    _cur_pos++;
  }

  int new_bci_for_old(int old_bci) {
    if (_cur_pos == 0 || old_bci < _old_bci[0]) return old_bci;
    _pos = 1;
    while (_pos < _cur_pos && old_bci >= _old_bci[_pos])
      _pos++;
    return _new_end_bci[_pos-1] + (old_bci - _old_bci[_pos-1]);
  }

  // Test if two indexes - one in the old method and another in the new one - correspond
  // to the same bytecode
  bool old_and_new_locations_same(int old_dest_bci, int new_dest_bci) {
    if (new_bci_for_old(old_dest_bci) == new_dest_bci)
      return true;
    else if (_old_bci[_pos-1] == old_dest_bci)
      return (new_dest_bci == _new_st_bci[_pos-1]);
    else return false;
  }
};
