#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)inlineImage.cpp	1.14 03/01/23 12:01:17 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// %%% NOTE: When the system supports parrellel compilation
// a mutex will have to be added
// Also, some buffering has to be added to avoid writing
// call sites to the file until the top level scope is
// finished (otherwise they will get interleaved).

#include "incls/_precompiled.incl"
#include "incls/_inlineImage.cpp.incl"

Arena*      InlineImage::arena             = NULL;
const char* InlineImage::file_name         = ".hotspot_inline_image";

GrowableArray<InlineImageTree*>* InlineImage::image_methods  = NULL;
// %% should be a bitmap
GrowableArray<bool>*        InlineImage::reused_methods = NULL;

void inlineImage_init() {
  if (UseInlineImage) {    
    InlineImage::parse_from_file();
    InlineImage::sort_by_addr();
  }
  if (DumpInlineImage) {
    InlineImage::init_file();
  }
}

symbolOop InlineCallSiteDescriptor::class_name()  const {
  return (symbolOop) JNIHandles::resolve_non_null(_c_name);
} 

symbolOop InlineCallSiteDescriptor::method_name() const {
  return (symbolOop) JNIHandles::resolve_non_null(_m_name);
} 

symbolOop InlineCallSiteDescriptor::signature()   const {
  return (symbolOop) JNIHandles::resolve_non_null(_sig);
}

void *InlineImageTree::operator new( size_t x ) {
  assert (InlineImage::arena != NULL, "Inline image arena not created.");
  return InlineImage::arena->Amalloc(x);
}

InlineImageTree::InlineImageTree(const InlineCallSiteDescriptor* descriptor) {
  _descriptor = *descriptor;
  assert (InlineImage::arena != NULL, "Inline image arena not created.");
  _children   = new (InlineImage::arena) GrowableArray<InlineImageTree*>(2,true);
}

//
// For creating the inline image file during compilation (incrementally)
//

void InlineImage::init_file() {
  fileStream stream(fopen(file_name, "wt"));
  append_comment(stream, "Inline Image");
}

// The compiler calls this for each compiled method
// This writes the method's inline tree to the image file.
void InlineImage::append_scope_begin_to_file(Scope *scope, int depth) {
  fileStream stream(fopen(file_name, "at"));
  InlineImage::MethodDescriptor md;
  md.c_name = scope->method()->klass_name();
  md.m_name = scope->method()->name();
  md.sig    = scope->method()->signature();
  // check for root scope
  if (scope->caller() == NULL)
    InlineImage::append_method_header(stream, &md);
  else
    InlineImage::append_inlined_callee(stream, depth, scope->callerBCI(), &md);
}

void InlineImage::append_call_begin_to_file(methodHandle method, int depth, int bci) {
  fileStream stream(fopen(file_name, "at"));
  InlineImage::MethodDescriptor md;
  md.c_name = method()->klass_name();
  md.m_name = method()->name();
  md.sig    = method()->signature();
  InlineImage::append_non_inlined_callee(stream, depth, bci, &md);
}

void InlineImage::append_scope_end_to_file() {
  fileStream stream(fopen(file_name, "at"));
  InlineImage::append_scope_end(stream);
}

// The InlineImageScopeClosure can be used to dump an entire
// CompilationScope, along with it's sub-scopes to a file
// It is not currently used by the compiler because we want
// to record call sites that do not necessary have a sub-scope--
// instead, the scopes are written out incrementally by the parser
class InlineImageScopeClosure : public ScopeClosure {
 private:
  int _nesting;
  fileStream& _stream;

 public:
  InlineImageScopeClosure(Scope* s, fileStream& stream, int n = 0) :
    ScopeClosure(s), _stream(stream) {
      _nesting = n;
  }
  void enter_scope ();
  void exit_scope ();
  ScopeClosure* clone_for_subiteration(Scope* subscope) {
    return new InlineImageScopeClosure(subscope, _stream, _nesting + 1);
  }
};

void InlineImageScopeClosure::enter_scope () {
  InlineImage::MethodDescriptor md;
  md.c_name = _scope->method()->klass_name();
  md.m_name = _scope->method()->name();
  md.sig    = _scope->method()->signature();
  // check for root scope
  if (_scope->caller() == NULL)
    InlineImage::append_method_header(_stream, &md);
  else
    InlineImage::append_inlined_callee(_stream, _nesting, _scope->callerBCI(), &md);
}

void InlineImageScopeClosure::exit_scope () {
  InlineImage::append_scope_end(_stream);
}

// The compiler calls this for each compiled method
// This writes the method's inline tree to the image file.
void InlineImage::append_scope_to_file(Scope *scope) {
  fileStream stream(fopen(file_name, "at"));
  scope->iterate(new InlineImageScopeClosure(scope, stream));
}

//
// For creating inline trees from an input image file
//

void InlineCallSiteDescriptor::init(bool inlined, int level, int ndx,
				    char *c_name_buf,
				    char *m_name_buf,
				    char *sig_buf)
{
  EXCEPTION_MARK;
  symbolHandle c_name = oopFactory::new_symbol_handle(c_name_buf, CATCH);
  symbolHandle m_name = oopFactory::new_symbol_handle(m_name_buf, CATCH);
  symbolHandle sig    = oopFactory::new_symbol_handle(sig_buf,    CATCH);
  _inlined = inlined;
  _level   = level;
  _ndx     = ndx;
  _c_name  = JNIHandles::make_global(c_name, false);
  _m_name  = JNIHandles::make_global(m_name, false);
  _sig     = JNIHandles::make_global(sig,    false);
}

class InlineImageParser : public StackObj {
private:
  static const int MAX_INPUT_LINE;
  FILE* _stream;
  int   _line_no;
  int   _num_methods;
  int   _level;
  bool  _valid;
  InlineCallSiteDescriptor _descriptor;

  const InlineCallSiteDescriptor* parse_descriptor();
public:
  InlineImageParser(FILE* stream) {
    _stream = stream;
    _line_no = 1;
    _num_methods = 0;
    _level = -1;
    _valid = false;
  }
  const InlineCallSiteDescriptor* last_descriptor() const {
    return _valid ? &_descriptor : NULL;
  }
  const InlineCallSiteDescriptor* parse_next_scope();
};

const int InlineImageParser::MAX_INPUT_LINE = 1024;

// return true if the next scope is successfully read
// store the output in _line
const InlineCallSiteDescriptor*  InlineImageParser::parse_next_scope() {
  int  pos = 0;
  int  c = getc(_stream);
  _valid = false;
  while(c != EOF) {
    // skip comment line
    if (c == '#') {
      while((c = getc(_stream)) != EOF) {
	if (c == '\n')
	  break;
      }
      continue;
    }
    if (c == '\n')
      _line_no++;
    else if (c == ')')
      _level--;
    else if (c == '(') {
      _level++;
      return parse_descriptor();
    }
    else if (c != ' ' && c != '\t')
      break;
    c = getc(_stream);
  }
  if (c != EOF || _level != -1)
    tty->print_cr("InlineImage, line %3d: bad nesting", _line_no);
  return NULL;
}


const InlineCallSiteDescriptor* InlineImageParser::parse_descriptor() {
  // input buffers
  int level;
  int bci; 
  char c_name_buf[MAX_INPUT_LINE];
  char m_name_buf[MAX_INPUT_LINE];
  char sig_buf[MAX_INPUT_LINE];

  // assume this parse will succeed
 
  if (_level == 0) {
    // check for method header
    if (fscanf(_stream, "%s %s %s", c_name_buf, m_name_buf, sig_buf) == 3) {
      _descriptor.init(true, 0, _num_methods++, c_name_buf, m_name_buf, sig_buf);
      if (PrintInlining)
	tty->print_cr("InlineImage: %s %s %s", c_name_buf, m_name_buf, sig_buf);
      _valid = true;
      return &_descriptor;
    }
  }
  else {
    bool match = false;
    if (fscanf(_stream, "%d", &level) == 1) {
      // check for inlined callee 
      if (fscanf(_stream, " @ %d %s %s %s", &bci, c_name_buf, m_name_buf, sig_buf) == 4) {
	_descriptor.init(true, _level, bci, c_name_buf, m_name_buf, sig_buf);
	if (PrintInlining) {
	  tty->print_cr("InlineImage:%*s %2d @%3d %s %s %s", _level, " ",
			_level, bci, c_name_buf, m_name_buf, sig_buf);
	}
	match = true;
      }
      // check for resolved virtual callee 
      else if (fscanf(_stream, " = %d %s %s %s", &bci, c_name_buf, m_name_buf, sig_buf) == 4) {
	_descriptor.init(false, _level, bci, c_name_buf, m_name_buf, sig_buf);
	if (PrintInlining) {
	  tty->print_cr("InlineImage:%*s %2d =%3d %s %s %s", _level, " ",
			_level, bci, c_name_buf, m_name_buf, sig_buf);
	}
	match = true;
      }
    }
    if (match) {
      if (PrintInlining && _level != level) {
	tty->print_cr("Warning: Inline image call site level %d does not match nesting level %d.", level, _level);
      }
      _valid = true;
      return &_descriptor;
    }
  }
  tty->print_cr("InlineImage, line %d : unrecognized input", _line_no);
  return NULL;
}

// recursively create a forest of inline trees for current inline level
void InlineImage::grow_forest(InlineImageParser& parser, 
			      GrowableArray<InlineImageTree*>* siblings,
			      int level) {
  const InlineCallSiteDescriptor* descriptor;

  descriptor = parser.last_descriptor();
  assert (descriptor != NULL, "InlineImageParser must return a valid descriptor");
  assert (descriptor->level() == level, "Incorrect tree level while reading InlineImage");

  // read this tree level and, recursively, all higher levels
  InlineImageTree *last_child = NULL;
  while (descriptor != NULL && descriptor->level() >= level) {
    if (descriptor->level() == level) {
      last_child = new InlineImageTree(descriptor);
      siblings->append(last_child);
      descriptor = parser.parse_next_scope();
    }
    else {
      assert (last_child != NULL, "InlineImage parser skipped a level");
      grow_forest(parser, last_child->children(), level + 1);
      descriptor = parser.last_descriptor();
    }
  };
}

// Free all memory used by the inline image
void delete_tree(InlineImageTree* t) {
  t->children()->apply(delete_tree);
  t->children()->clear_and_deallocate();
}

void InlineImage::parse_from_file() {
  MutexLocker mc(Compile_lock);
  // initialize arena (destroy existing image)
  if (arena  == NULL) {
    arena = new Arena;
  } else {    
    // %%% This unfortunate kludge is necessary until someone
    // modifies GrowableArray to allocate from a regular arena
    image_methods->apply(delete_tree);
    image_methods->clear_and_deallocate();
    reused_methods->clear_and_deallocate();

    arena->destruct_contents();
  }
  // Create an array to hold the inline tree for each compiled method
  image_methods = new (arena) GrowableArray<InlineImageTree*>(2, true);

  FILE* stream = fopen(file_name, "rt");
  if (stream == NULL) return;

  InlineImageParser parser(stream);
  if (parser.parse_next_scope() != NULL)
    grow_forest(parser, image_methods);

  fclose(stream);

  reused_methods = new (arena) GrowableArray<bool>(image_methods->length(),
						   image_methods->length(),
						   false, true);
}

//
// For sorting trees
//

static int bci_compare_tree(InlineImageTree** x, InlineImageTree** y) {
  return (*x)->descriptor()->bci() - (*y)->descriptor()->bci();
}

static int addr_compare_tree(InlineImageTree** x, InlineImageTree** y) {
  symbolOop x_name = (*x)->descriptor()->class_name();
  symbolOop y_name = (*y)->descriptor()->class_name();
  int result = x_name->fast_compare(y_name);
  // if classes are the same, check the method name
  if (result == 0) {
    x_name = (*x)->descriptor()->method_name();
    y_name = (*y)->descriptor()->method_name();
    result = x_name->fast_compare(y_name);
  }
  // if method names are the same, check signature
  if (result == 0) {
    x_name = (*x)->descriptor()->signature();
    y_name = (*y)->descriptor()->signature();
    result = x_name->fast_compare(y_name);
  }
  // if methods are duplicates, sort by parse order
  if (result == 0) {
    result = (*x)->descriptor()->parse_order() - (*y)->descriptor()->parse_order();
  }
  return result;
}
 
static int string_compare_tree(InlineImageTree** x, InlineImageTree** y) {
  symbolOop x_name = (*x)->descriptor()->class_name();
  symbolOop y_name = (*y)->descriptor()->class_name();
  int result = strcmp(x_name->as_C_string(), y_name->as_C_string());
  // if classes are the same, check the method name
  if (result == 0) {
    x_name = (*x)->descriptor()->method_name();
    y_name = (*y)->descriptor()->method_name();
    result = strcmp(x_name->as_C_string(), y_name->as_C_string());
  }
  // if method names are the same, check signature
  if (result == 0) {
    x_name = (*x)->descriptor()->signature();
    y_name = (*y)->descriptor()->signature();
    result = strcmp(x_name->as_C_string(), y_name->as_C_string());
  }
  // if methods are duplicates, sort by compilation order
  if (result == 0) {
    result = (*x)->descriptor()->parse_order() - (*y)->descriptor()->parse_order();
  }
  return result;
}

// recursive depth first preorder traversal of inline trees
void InlineImage::iterate(GrowableArray<InlineImageTree*>* siblings,
			  InlineImageTreeClosure* visitor) {
  assert (siblings != NULL, "Cannot iterate over empty array");
  for (int i = 0; i < siblings->length(); i++) {
    InlineImageTree* sibling = siblings->at(i);
    visitor->pre_visit(sibling);
    GrowableArray<InlineImageTree*>* children = sibling->children();
    if (children->length() > 0)
      iterate(children, visitor);
    visitor->post_visit(sibling);
  }
}

class InlineImageTreeSortClosure : public InlineImageTreeClosure {
private:
  int (*_comp_fn)(InlineImageTree** x, InlineImageTree** y);
public:
  InlineImageTreeSortClosure( int (*comp_fn)(InlineImageTree** x, InlineImageTree** y)) : 
    _comp_fn(comp_fn) {}
  virtual void pre_visit(InlineImageTree* t);
};

void InlineImageTreeSortClosure::pre_visit(InlineImageTree* t) {
  t->children()->sort(_comp_fn);
}

// Sorting by address is usefull for faster lookups
void InlineImage::sort_by_addr() {
  assert (image_methods != NULL, "The InlineImage has not been created");
  // sort top level methods
  image_methods->sort(addr_compare_tree);
  // sort call sites within each method
  InlineImageTreeSortClosure sorter(bci_compare_tree);
  iterate(image_methods, &sorter);
}

// Sorting by C string is usefull for doing a diff between runs
void InlineImage::sort_by_name() {
  assert (image_methods != NULL, "The InlineImage has not been created");
  // sort top level methods
  image_methods->sort(string_compare_tree);
  // sort call sites within each method
  InlineImageTreeSortClosure sorter(bci_compare_tree);
  iterate(image_methods, &sorter);
}

//
// For locating an inline decision
//

// %% Assumes the methods have been sorted by symbol address
int InlineImage::match_method(InlineImageTree* t, methodHandle m) {
  symbolOop t_name;
  int result;
  assert(t != NULL, "match_method requires a valid InlineImageTree");
  t_name = t->descriptor()->class_name();
  result = t_name->fast_compare(m->klass_name());
  // if classes are the same, check the method name
  if (result == 0) {
    t_name = t->descriptor()->method_name();
    result = t_name->fast_compare(m->name());
  }
  // if method names are the same, check signature
  if (result == 0) {
    t_name = t->descriptor()->signature();
    result = t_name->fast_compare(m->signature());
  }
  return result;
}

// search the sorted array for a root method
// return index of method, or -1 if not found
int InlineImage::find_method(methodHandle method) {
  static int last_method_found = -1;

  if (last_method_found >= 0 &&
      match_method(image_methods->at(last_method_found), method) == 0 &&
      !reused_methods->at(last_method_found)) {
    return last_method_found;
  }

  unsigned int imin = 0;
  unsigned int imax = image_methods->length();
  while (imin + 5 < imax) {
    int i = (imin + imax) / 2;
    // there may be duplicates.  We must find the first one
    if (match_method(image_methods->at(i), method) >= 0) {
      imax = i+1;
    } else {
      imin = i;
    }
  }
  for (unsigned int i = imin; i < imax; i++) {
    if (match_method(image_methods->at(i), method) == 0) {
      // Match duplicate inline trees in original compilation order
      while (reused_methods->at(i)) {
	if (match_method(image_methods->at(i+1), method) != 0)
	  break;
	i++;
      }
      last_method_found = i;
      return i;
    }
  }
  return -1;
}

// Search an array of inline trees (call sites) for one at this bci
// %% Assumes "siblings" has been sorted by bci
InlineImageTree* InlineImage::find_bci(GrowableArray<InlineImageTree*>* siblings, int bci) {
  unsigned int imin = 0;
  unsigned int imax = siblings->length();
  while (imin + 5 < imax) {
    int i = (imin + imax) / 2;
    if (siblings->at(i)->descriptor()->bci() > bci) {
      imax = i;
    } else {
      imin = i;
    }
  }
  for (unsigned int i = imin; i < imax; i++) {
    if (siblings->at(i)->descriptor()->bci() == bci)
      return siblings->at(i);
  }
  return NULL;
}

// recursively match the calling scope with an inline decision tree
// %% If this were integrated with CompilationScope::inline_scopes()
// (by giving each sub-scope a pointer to its inline decision tree)
// a lot of work could be saved - by not repeatedly traversing
// the inline tree hierarchy
InlineImageTree* InlineImage::locate_tree(Scope* caller, int bci) {
  InlineImageTree *parent = NULL;
  if (caller->is_inlined()) {
    parent = locate_tree(caller->caller(), caller->callerBCI());
  }
  else {
    // this is the top level scope
    int index = find_method(caller->method());
    if (index >= 0)
      parent = image_methods->at(index);
  }
  if (parent == NULL)
    return NULL;
  // search all call sites on this level for bci
  return find_bci(parent->children(), bci);
}

// called for each method after it's inlining decisions
// are reused by the compiler
// returns true if the method exists in the image
bool InlineImage::query_method(methodHandle method) {
  // mark the method as reused so that no more inling decisions are
  // pulled from its tree this allows reusing inlining decisions from
  // multiple compilations of the same method
  int index = find_method(method);
  if (index >= 0 && reused_methods->at(index) == false) {
    reused_methods->at_put(index, true);
    return true;
  }
  return false;
}

// return true if this call site is inlined
bool InlineImage::is_inlined(Scope *caller, int bci) {
  return (locate_tree(caller, bci) != NULL);
}

// return non-null method if this call site is inlined
methodHandle InlineImage::resolve_virtual_method(Scope *caller, int bci,
						 bool inlined) {
  InlineImageTree* tree = locate_tree(caller, bci);
  if (tree == NULL)
    return methodHandle();

  // check if the call must be inlined
  if (inlined && !tree->descriptor()->inlined())
    return methodHandle();

  Thread *thread = Thread::current();
  symbolHandle c_name (thread, tree->descriptor()->class_name());
  symbolHandle m_name (thread, tree->descriptor()->method_name());
  symbolHandle sig    (thread, tree->descriptor()->signature());
  // lookup the klass oop (normally done through contant pool)
  instanceKlass* constant_pool_holder =
    instanceKlass::cast(caller->method()->constants()->pool_holder());
  Handle loader (thread, constant_pool_holder->class_loader());
  Handle protection_domain (thread, constant_pool_holder->protection_domain());
  KlassHandle klass(thread, SystemDictionary::find(c_name, loader, protection_domain, thread));
  if (klass.is_null()) {
    tty->print("Cannot inline method, klass not loaded: ");
    c_name->print_symbol_on(tty);
    tty->print("::");
    m_name->print_symbol_on(tty);
    sig->print_symbol_on(tty);
    tty->cr();
    return methodHandle();
  }
  assert(!klass->is_interface(), "must be exactly-typed class");
  assert(!klass->is_abstract(),  "must be call to real object");
  KlassHandle mh (thread, caller->method()->method_holder());
  methodHandle method = LinkResolver::resolve_virtual_call_or_null(klass, klass, m_name, sig, mh);
  if (method.is_null()) {
    tty->print("Cannot inline method, error resolving call: ");
    c_name->print_symbol_on(tty);
    tty->print("::");
    m_name->print_symbol_on(tty);
    sig->print_symbol_on(tty);
    tty->cr();
  }
  return method;
}

//
// For creating an image file from inline trees
//

class InlineImageTreeDumpClosure : public InlineImageTreeClosure {
private:
  fileStream& _stream;
public:
  InlineImageTreeDumpClosure(fileStream& stream) : _stream(stream) {}
  virtual void pre_visit(InlineImageTree* t);
  virtual void post_visit(InlineImageTree* t);
};

void InlineImageTreeDumpClosure::pre_visit(InlineImageTree* t) {
  int level = t->descriptor()->level();
  InlineImage::MethodDescriptor md;
  md.c_name = t->descriptor()->class_name();
  md.m_name = t->descriptor()->method_name();
  md.sig    = t->descriptor()->signature();
  if (level == 0)
    InlineImage::append_method_header(_stream, &md);
  else {
    int bci = t->descriptor()->bci();
    if (t->descriptor()->inlined())
      InlineImage::append_inlined_callee(_stream, level, bci, &md);
    else
      InlineImage::append_non_inlined_callee(_stream, level, bci, &md);
  }
}

void InlineImageTreeDumpClosure::post_visit(InlineImageTree* t) {
  InlineImage::append_scope_end(_stream);
}

void InlineImage::dump_to_file() {
  // make sure no compilations are active
  // (otherwise image file gets garbled)
  MutexLocker mc(Compile_lock);
  init_file();
  fileStream stream(fopen(file_name, "at"));
  InlineImageTreeDumpClosure dump_closure(stream);
  iterate(image_methods, &dump_closure);
}

//
// Image file helpers
//

void InlineImage::append_comment(fileStream& stream, const char* message) {
  stream.print("# ");
  for (int index = 0; message[index] != '\0'; index++) {
    stream.put(message[index]);
    if (message[index] == '\n') stream.print("# ");
  }
  stream.cr();
}

void InlineImage::append_method_header(fileStream& stream, InlineImage::MethodDescriptor* md) {
  stream.cr();
  stream.print_cr("###");
  stream.print("( ");
  append_method_descriptor(stream, md);
}

void InlineImage::append_inlined_callee(fileStream& stream, int nesting, int bci, InlineImage::MethodDescriptor* md) {
  stream.cr();
  stream.print("%*s(%d @%3d ", nesting*2, " ", nesting, bci);
  append_method_descriptor(stream, md);
}

void InlineImage::append_non_inlined_callee(fileStream& stream, int nesting, int bci, InlineImage::MethodDescriptor* md) {
  stream.cr();
  stream.print("%*s(%d =%3d ", nesting*2, " ", nesting, bci);
  append_method_descriptor(stream, md);
}

void InlineImage::append_method_descriptor(fileStream& stream, InlineImage::MethodDescriptor* md) {
  md->c_name->print_symbol_on(&stream);
  stream.print(" ");
  md->m_name->print_symbol_on(&stream);
  stream.print(" ");
  md->sig->print_symbol_on(&stream);
}

void InlineImage::append_scope_end(fileStream& stream) {
  stream.print(" )");
}
