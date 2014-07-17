#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)generationSpec.cpp	1.12 03/01/23 12:08:49 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_generationSpec.cpp.incl"

Generation* GenerationSpec::init(ReservedSpace rs, int level,
				 GenRemSet* remset) {
  switch (name()) {
    case Generation::DefNew:
      return new DefNewGeneration(rs, init_size(), level);

    case Generation::ParNew:
      return new ParNewGeneration(rs, init_size(), level);

    case Generation::MarkSweepCompact:
      return new TenuredGeneration(rs, init_size(), level, remset);
    
    case Generation::TrainGen: {
      CardTableRS* ctrs = remset->as_CardTableRS();
      if (ctrs == NULL) {
	vm_exit_during_initialization("Rem set incompatibility.");
      }
      // Otherwise
      return new TrainGeneration(rs, init_size(), level, ctrs);
    }

    case Generation::ConcurrentMarkSweep: {
      assert(UseConcMarkSweepGC, "UseConcMarkSweepGC should be set");
      CardTableRS* ctrs = remset->as_CardTableRS();
      if (ctrs == NULL) {
	vm_exit_during_initialization("Rem set incompatibility.");
      }
      // Otherwise
      // The constructor creates the CMSCollector if needed,
      // else registers with an existing CMSCollector
      ConcurrentMarkSweepGeneration* g = new ConcurrentMarkSweepGeneration(rs,
                   init_size(), level, ctrs, UseCMSAdaptiveFreeLists,
                   (FreeBlockDictionary::DictionaryChoice)CMSDictionaryChoice);

      g->initialize_performance_counters();

      return g;
    }

    default:
      guarantee(false, "unrecognized GenerationName");
      return NULL;
  }
}

PermGen* PermanentGenerationSpec::init(ReservedSpace rs, size_t init_size,
                                       GenRemSet *remset) {
  switch (name()) {
    case PermGen::MarkSweepCompact:
      return new CompactingPermGen(rs, init_size, remset);
      
    case PermGen::MarkSweep:
      guarantee(false, "NYI");
      return NULL;
      
    case PermGen::ConcurrentMarkSweep: {
      assert(UseConcMarkSweepGC, "UseConcMarkSweepGC should be set");
      CardTableRS* ctrs = remset->as_CardTableRS();
      if (ctrs == NULL) {
        vm_exit_during_initialization("RemSet/generation incompatibility.");
      }
      // XXXPERM
      return new CMSPermGen(rs, init_size, ctrs,
                   (FreeBlockDictionary::DictionaryChoice)CMSDictionaryChoice);
    }
    default:
      guarantee(false, "unrecognized GenerationName");
      return NULL;
  }
}

