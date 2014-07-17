#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vmStructs_ia64.hpp	1.8 03/01/23 10:58:47 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// These are the CPU-specific fields, types and integer
// constants required by the Serviceability Agent. This file is
// referenced by vmStructs.cpp.

#define VM_STRUCTS_CPU(nonstatic_field, static_field, unchecked_nonstatic_field, volatile_nonstatic_field, nonproduct_nonstatic_field, nonproduct_noncore_nonstatic_field, c2_nonstatic_field, noncore_nonstatic_field, noncore_static_field, unchecked_c1_static_field, unchecked_c2_static_field, last_entry)            \
                                                                                                                         \
  /******************************/                                                                                       \
  /* JavaCallWrapper            */                                                                                       \
  /******************************/                                                                                       \
                                                                                                                         \
  /******************************/                                                                                                   \
  /* JavaFrameAnchor            */                                                                                                   \
  /******************************/                                                                                                   \
  volatile_nonstatic_field(JavaFrameAnchor,             _last_Java_pc,                                  address)         \
  volatile_nonstatic_field(JavaFrameAnchor,             _last_Java_fp,                                  intptr_t*)       \
  volatile_nonstatic_field(JavaFrameAnchor,             _flushed_windows,                               bool)            \
                                                                                                                         \
                                                                                                                                     
  /* NOTE that we do not use the last_entry() macro here; it is used  */
  /* in vmStructs_<os>_<cpu>.hpp's VM_STRUCTS_OS_CPU macro (and must  */
  /* be present there)                                                */


#define VM_TYPES_CPU(declare_type, declare_toplevel_type, declare_oop_type, declare_integer_type, declare_unsigned_integer_type, declare_noncore_type, declare_noncore_toplevel_type, declare_noncore_unsigned_integer_type, declare_c1_toplevel_type, declare_c2_type, declare_c2_toplevel_type, last_entry)                               \
        declare_toplevel_type(interpreterState)
                                                                                                                                     
  /* NOTE that we do not use the last_entry() macro here; it is used  */
  /* in vmStructs_<os>_<cpu>.hpp's VM_TYPES_OS_CPU macro (and must    */
  /* be present there)                                                */


#define VM_INT_CONSTANTS_CPU(declare_constant, declare_preprocessor_constant, declare_noncore_constant, declare_c1_constant, declare_c2_constant, declare_c2_preprocessor_constant, last_entry)                                                              \
                                                                        
  /* NOTE that we do not use the last_entry() macro here; it is used        */
  /* in vmStructs_<os>_<cpu>.hpp's VM_INT_CONSTANTS_OS_CPU macro (and must  */
  /* be present there)                                                      */

#define VM_LONG_CONSTANTS_CPU(declare_constant, declare_preprocessor_constant, declare_c1_constant, declare_c2_constant, declare_c2_preprocessor_constant, last_entry)                                                              \
                                                                        
  /* NOTE that we do not use the last_entry() macro here; it is used         */
  /* in vmStructs_<os>_<cpu>.hpp's VM_LONG_CONSTANTS_OS_CPU macro (and must  */
  /* be present there)                                                       */
