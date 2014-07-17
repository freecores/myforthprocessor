#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)inlineImage.hpp	1.8 03/01/23 12:01:19 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Scope;
class BitMap;
class InlineImageParser;
class InlineImageTree;

class InlineImageTreeClosure : public StackObj {
public:
  virtual void pre_visit(InlineImageTree* t)  {};
  virtual void post_visit(InlineImageTree* t) {};
};

// InlineImage is an interface for preserving inlining decisions
// for comparisions and reproducability
class InlineImage : public AllStatic {
private:
  struct MethodDescriptor {
    symbolOop c_name;
    symbolOop m_name;
    symbolOop sig;
  };

  static Arena*      arena;
  static const char* file_name;
  // Array of inline trees that hold the inline decisions
  // for each method in the image
  static GrowableArray<InlineImageTree*>* image_methods;
  // Map of methods that have been compiled in this run
  static GrowableArray<bool>*        reused_methods;

  // For creating the inline trees
  static void grow_forest(InlineImageParser& parser,
			  GrowableArray<InlineImageTree*>* siblings,
			  int level = 0);

  // simple depth-first preorder iteration across InlineImageTrees
  static void iterate(GrowableArray<InlineImageTree*>* siblings,
		      InlineImageTreeClosure* visitor);

  // For searching the trees
  static int         match_method(InlineImageTree *t, methodHandle m);
  static int         find_method(methodHandle method);
  static InlineImageTree* find_bci(GrowableArray<InlineImageTree*>* siblings, int bci);
  static InlineImageTree* locate_tree(Scope* caller, int bci);

  // For updating the image file
  static void append_comment(fileStream& stream, const char* message);
  static void append_method_header(fileStream& stream, MethodDescriptor* md);
  static void append_inlined_callee(fileStream& stream, int nesting, int bci,
				    MethodDescriptor* md);
  static void append_non_inlined_callee(fileStream& stream, int nesting,
					int bci, MethodDescriptor* md);
  static void append_method_descriptor(fileStream& stream,
				       MethodDescriptor* md);
  static void append_scope_end(fileStream& stream);
  static void dump_tree_to_file(InlineImageTree* tree);
  
  friend class InlineImageParser;
  friend class InlineImageScopeClosure;
  friend class InlineImageTreeDumpClosure;
  friend class InlineImageTree;

public:
  // incrementally create image file
  static void init_file();
  static void append_scope_begin_to_file(Scope *scope, int depth);
  static void append_call_begin_to_file(methodHandle method, int depth, int bci);
  static void append_scope_end_to_file();
  static void append_scope_to_file(Scope *scope);

  // read/write inline trees from/to file
  static void parse_from_file();
  static void dump_to_file();

  // sort top level inline trees (compiled methods)
  static void sort_by_addr();
  static void sort_by_name();

  // lookup inlining decision for method
  static bool         has_method(methodHandle method) {
    return find_method(method) >= 0;
  }
  static bool         query_method(methodHandle method);
  static bool         is_inlined(Scope *caller, int bci);
  static methodHandle resolve_virtual_method(Scope *caller, int bci,
					     bool inlined = false);
  static methodHandle resolve_virtual_method_if_inlined(Scope *caller, int bci) {
    return resolve_virtual_method(caller, bci, true);
  }
};

// Describes each call inlined by the compiler
class InlineCallSiteDescriptor VALUE_OBJ_CLASS_SPEC {
private:
  // is the call site inlined (or just resolved vcall)
  bool     _inlined;
  int      _level;
  // For top-level methods, ndx stores the order in which methods were
  // parsed.  For other call sites, ndx stores the bci
  int      _ndx;
  jobject  _c_name;
  jobject  _m_name;
  jobject  _sig;

public:
  void init(bool inlined, int level, int ndx,
	    char *c_name_buf,
	    char *m_name_buf,
	    char *sig_buf);

  bool inlined() const { return _inlined; }
  int  level()   const { return _level; }
  int  bci()     const { assert (_level > 0,""); return _ndx;   }
  int  parse_order() const { assert (_level == 0,""); return _ndx; }
  inline symbolOop class_name()  const;
  inline symbolOop method_name() const;
  inline symbolOop signature()   const;
};

// An InlineImageTree holds the descriptor for a single inlined call site
// and points to all call site descriptors within that sub-scope.
class InlineImageTree : public ResourceObj {
private:
  InlineCallSiteDescriptor    _descriptor;
  GrowableArray<InlineImageTree*>* _children;

public:
  void *operator new( size_t x );
  InlineImageTree(const InlineCallSiteDescriptor* descriptor);
  // accessors
  const InlineCallSiteDescriptor* const descriptor() { return &_descriptor; }
  GrowableArray<InlineImageTree*>* children() const { return _children; }
};
