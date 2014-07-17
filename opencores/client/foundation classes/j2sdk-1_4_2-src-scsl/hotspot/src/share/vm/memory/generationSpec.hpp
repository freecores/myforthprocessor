#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)generationSpec.hpp	1.7 03/01/23 12:08:51 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The specification of a generation.  This class also encapsulates
// some generation-specific behavior.  This is done here rather than as a
// virtual function of Generation because these methods are needed in
// initialization of the Generations.
class GenerationSpec : public CHeapObj {
  friend class VMStructs;
private:
  Generation::Name _name;
  size_t           _init_size;
  size_t           _max_size;

public:
  GenerationSpec(Generation::Name name, size_t init_size, size_t max_size) {
    _name = name;
    _init_size = init_size;
    _max_size = max_size;
  }

  Generation* init(ReservedSpace rs, int level, GenRemSet* remset);

  // Accessors
  Generation::Name name()        const { return _name; }
  size_t init_size()             const { return _init_size; }
  void set_init_size(size_t size)      { _init_size = size; }
  size_t max_size()              const { return _max_size; }
  void set_max_size(size_t size)       { _max_size = size; }
};

typedef GenerationSpec* GenerationSpecPtr;

// The specification of a permanent generation. This class is very
// similar to GenerationSpec in use. Due to PermGen's not being a
// true Generation, we cannot combine the spec classes either.
class PermanentGenerationSpec : public CHeapObj {
  friend class VMStructs;
private:
  PermGen::Name    _name;
  size_t           _init_size;
  size_t           _max_size;

public:
  PermanentGenerationSpec(PermGen::Name name, size_t init_size, size_t max_size) {
    _name = name;
    _init_size = init_size;
    _max_size = max_size;
  }

  PermGen* init(ReservedSpace rs, size_t init_size, GenRemSet* remset);

  // Accessors
  PermGen::Name name()           const { return _name; }
  size_t init_size()             const { return _init_size; }
  void set_init_size(size_t size)      { _init_size = size; }
  size_t max_size()              const { return _max_size; }
  void set_max_size(size_t size)       { _max_size = size; }
};
