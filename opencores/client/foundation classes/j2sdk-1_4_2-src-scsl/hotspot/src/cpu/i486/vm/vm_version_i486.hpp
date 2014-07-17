#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vm_version_i486.hpp	1.15 03/01/23 10:56:09 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class VM_Version: public Abstract_VM_Version {

protected:
   static int _cpu;
   static int _cpuFeatures;	// features returned by the "cpuid" instruction
				// 0 if this instruction is not available

   enum {
     CPU_CX8  = (1 << 8),
     CPU_CMOV = (1 << 15),
     CPU_MMX  = (1 << 23),
     CPU_FXSR = (1 << 24),
     CPU_SSE  = (1 << 25),
     CPU_SSE2 = (1 << 26)
   } cpuFeatureFlags;

  static void get_processor_features();

public:
  static void initialize();
  //
  // Processor family:
  //       3   -  386
  //       4   -  486
  //       5   -  Pentium
  //       6   -  PentiumPro, Pentium II, Celeron, Zenon, Pentium III
  //    0x0f   -  Pentium 4
  //
  // Note: The cpu family should be used to select between
  //       instruction sequences which are valid on all Intel
  //       processors.  Use the feature test functions below to
  //       determine whether a particular instruction is supported.
  //
  static int  cpu_family()        { return _cpu;}
  static bool is_P6()             { return cpu_family() >= 6; }

  //
  // Feature identification
  //
  static bool supports_cpuid()		{ return _cpuFeatures  != 0; }
  static bool supports_cmov()		{ return (_cpuFeatures & CPU_CMOV) != 0; }
  static bool supports_cmpxchg8()	{ return (_cpuFeatures & CPU_CX8) != 0; }
  static bool supports_mmx()		{ return (_cpuFeatures & CPU_MMX) != 0; }
  static bool supports_fxsr()		{ return (_cpuFeatures & CPU_FXSR) != 0; }
  static bool supports_sse()		{ return (_cpuFeatures & CPU_SSE) != 0; }
  static bool supports_sse2()		{ return (_cpuFeatures & CPU_SSE2) != 0; }

  static bool supports_compare_and_exchange()    { return true; }
};
