#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)debug.cpp	1.146 03/01/23 12:27:50 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_debug.cpp.incl"


#ifndef ASSERT
#  ifdef _DEBUG
   // NOTE: don't turn the lines below into a comment -- if you're getting
   // a compile error here, change the settings to define ASSERT
   ASSERT should be defined when _DEBUG is defined.  It is not intended to be used for debugging
   functions that do not slow down the system too much and thus can be left in optimized code.
   On the other hand, the code should not be included in a production version.
#  endif // _DEBUG
#endif // ASSERT


#ifdef _DEBUG
#  ifndef ASSERT
     configuration error: ASSERT must be defined in debug version
#  endif // ASSERT
#endif // _DEBUG


#ifdef PRODUCT
#  if -defined _DEBUG || -defined ASSERT
     configuration error: ASSERT et al. must not be defined in PRODUCT version
#  endif
#endif // PRODUCT


// Convert the file_name and line_no into an obfuscated string.
// The algorithm does the following:
// Given, for example:
// 
//   file_name:  d:\hotspot\src\share\vm\runtime.cpp
//   line_no:    257
//
// Extract the file_name without path, e.g., runtime.cpp. Then outputs
// all the characters in 2-digit hexidecimal form where 32 has been 
// subtracted from each character. Followed by the linenumber in
// 4-digit hexidecimal.
//
// In the above example we will get something like:
//
//    4D5D827452A3A30101
//
// (Note: the above hexidecimal numbers are totally random)
//
// This only happens in PRODUCT builds. In DEBUG, and RELEASE builds
// the filename and linenuber is printed out directly.

void obfuscate_location(const char *file_name, int line_no, char* buf, int buflen) {
#ifndef PRODUCT
  int len = (int)strlen(file_name);
  strncpy(buf, file_name, buflen);
  if (len + 10 < buflen) {
    sprintf(buf + len, ", %d", line_no);
  }    
#else
  int len = (int)strlen(file_name);  
  if (len > 0) {
    // Find last part of string
    int i;
    for(i = len -1; i > 0 && (file_name[i] != '\\' && file_name[i] != '/'); i--);
    file_name = &file_name[i+1];
  }
  
  // Output filename into buffer. The 10 is so there is alway enough room
  // for both the line number, and a terminating zero, plus a little extra
  // to be safe.
  while(*file_name && buflen > 10) {
    sprintf(buf, "%02X", *file_name - ' ');
    buf += 2;
    buflen -= 2;
    file_name++;
  }  
  // Add then the line number
  sprintf(buf, "%04X", line_no);  
#endif // PRODUCT

  pd_obfuscate_location(buf,buflen);
}

void warning(const char* format, ...) {
  // In case error happens before init or during shutdown
  if (tty == NULL) ostream_init();

  tty->print("%s warning: ", VM_Version::vm_name());
  va_list ap;
  va_start(ap, format);
  tty->vprint_cr(format, ap);
  va_end(ap);
  if (BreakAtWarning) BREAKPOINT;
}


void report_vm_state() {
#ifndef PRODUCT
  if (CountBytecodes || TraceBytecodes || StopInterpreterAt) {
    tty->print("# ");
    BytecodeCounter::print();
  }
#endif
#ifndef CORE
#ifndef PRODUCT
  CompileBroker::print_last_compile();
  {
    // Print statistics as if before normal exit:
    extern void print_statistics();
    print_statistics();
  }
#endif // PRODUCT
#endif // CORE
  if (!DumpEventsAtError) return;
  static bool recursive_error = false;
  if (!recursive_error) {
    recursive_error = true;
    tty->print("# Last 10 internal VM events:\n");
    Events::print_last(10);
    recursive_error = false;
    tty->print_cr("###");
  }
}


void report_fatal(const char* file_name, int line_no, const char* format, ...) {
  char buffer[2048];  
  va_list ap;
  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);  
  report_error(true, file_name, line_no, "Internal Error", "Fatal: %s", buffer);
}


#ifndef PRODUCT

#define is_token_break(ch) (isspace(ch) || (ch) == ',')

static const char* last_file_name = NULL;
static int         last_line_no   = -1;

bool assert_is_suppressed(const char* file_name, int line_no) {
  // The following 1-element cache requires that passed-in
  // file names are always only constant literals.
  if (file_name == last_file_name && line_no == last_line_no)  return true;

  int file_name_len = (int)strlen(file_name);
  const char* base_name = strrchr(file_name, '/');
  if (base_name == NULL)
    base_name = strrchr(file_name, '\\');
  if (base_name == NULL)
    base_name = file_name;
  // scan the SuppressErrorAt option
  const char* cp = SuppressErrorAt;
  for (;;) {
    const char* sfile;
    int sfile_len;
    int sline;
    bool noisy;
    while ((*cp) != '\0' && is_token_break(*cp))  cp++;
    if ((*cp) == '\0')  break;
    sfile = cp;
    while ((*cp) != '\0' && !is_token_break(*cp) && (*cp) != ':')  cp++;
    sfile_len = cp - sfile;
    if ((*cp) == ':')  cp++;
    sline = 0;
    while ((*cp) != '\0' && isdigit(*cp)) {
      sline *= 10;
      sline += (*cp) - '0';
      cp++;
    }
    // "file:line!" means the assert suppression is not silent
    noisy = ((*cp) == '!');
    while ((*cp) != '\0' && !is_token_break(*cp))  cp++;
    // match the line
    if (sline != 0) {
      if (sline != line_no)  continue;
    }
    // match the file
    if (sfile_len > 0) {
      const char* look = file_name;
      const char* look_max = file_name + file_name_len - sfile_len;
      const char* foundp;
      bool match = false;
      while (!match
             && (foundp = strchr(look, sfile[0])) != NULL
             && foundp <= look_max) {
        match = true;
        for (int i = 1; i < sfile_len; i++) {
          if (sfile[i] != foundp[i]) {
            match = false;
            break;
          }
        }
        look = foundp + 1;
      }
      if (!match)  continue;
    }
    // got a match!
    if (noisy) {
      tty->print_cr("[error suppressed at %s:%d]", base_name, line_no);
    } else {
      // update 1-element cache for fast silent matches
      last_file_name = file_name;
      last_line_no   = line_no;
    }
    return true;
  }
  // print a friendly hint:
  extern void set_error_reported();
  set_error_reported();
  tty->print_cr("# To suppress the following error report, specify this argument");
  tty->print_cr("# after -XX: or in .hotspotrc:  SuppressErrorAt=%s:%d",
                base_name, line_no);
  return false;
}

#undef is_token_break

#else

// Place-holder for non-existent suppression check:
#define assert_is_suppressed(file_name, line_no) (false)

#endif //PRODUCT

void report_assertion_failure(const char* code_str, const char* file_name, int line_no, const char* message) {
  if (assert_is_suppressed(file_name, line_no))  return;
  report_error(true, file_name, line_no,
               "assertion failure",
               "assert(%s, \"%s\")", code_str, message);
}


void report_should_not_call(const char* file_name, int line_no) {
  if (assert_is_suppressed(file_name, line_no))  return;
  report_error(true, file_name, line_no,
               "Internal Error", "ShouldNotCall()");
}


void report_should_not_reach_here(const char* file_name, int line_no) {
  if (assert_is_suppressed(file_name, line_no))  return;
  report_error(true, file_name, line_no,
               "Internal Error", "ShouldNotReachHere()");
}


void report_subclass_responsibility(const char* file_name, int line_no) {
  if (assert_is_suppressed(file_name, line_no))  return;
  report_error(true, file_name, line_no,
               "Internal Error", "SubclassResponsibility()");
}


void report_unimplemented(const char* file_name, int line_no) {
  if (assert_is_suppressed(file_name, line_no))  return;
  report_error(true, file_name, line_no,
               "Internal Error", "Unimplemented()");
}


void report_untested(const char* file_name, int line_no, const char* msg) {
#ifndef PRODUCT
  warning("Untested: %s in %s: %d\n", msg, file_name, line_no);
#endif // PRODUCT
}


// helper function, returns the active vm operation if present.
static VM_Operation* active_vm_operation() {
  Thread* thread = ThreadLocalStorage::thread();
  if (thread == NULL)          return NULL;
  if (!thread->is_VM_thread()) return NULL;
  return VMThread::vm_operation();
}


extern "C" void ps();

static jint error_level = 1;
static bool error_reported = false;


// call this when the VM is dying--it might loosen some asserts
void set_error_reported() {
  error_reported = true;
}

bool is_error_reported() {
    return error_reported || error_level > 1;
}

jint inc_error_level() {
    return atomic::add(1, &error_level);
}

jint dec_error_level() {
    return atomic::add(-1, &error_level);
}

void report_error(bool is_vm_internal_error, const char* file_name, int line_no, const char* title, const char* format,  ...) {  
  // Handle the recursive case  
  switch(inc_error_level()) {
   case 2:  // first time, do nothing
            break; 
   case 3:  // second time print recursive problem 
            if (tty) tty->print_cr("[error occured during error reporting]"); 
            else puts("[error occured during error reporting]");
            if (ShowMessageBoxOnError) {
              os::infinite_sleep();
            }
            return;
   default: // otherwise just say NO.
            if (ShowMessageBoxOnError) {
              os::infinite_sleep();
            }
            exit(-1);
  }
  
  // In case error happens before init or during shutdown
  if (tty == NULL) ostream_init();

  // Compute location string
  char loc_buf[256];
  if (file_name != NULL) {
    obfuscate_location(file_name, line_no, loc_buf, sizeof(loc_buf));
  } else {
    strcpy(loc_buf, "<unknown>");
  }

  // Compute the message
  char message[2*K];
  va_list ap;
  va_start(ap, format);
  vsprintf(message, format, ap);
  va_end(ap);

  if (is_vm_internal_error) {
    // Print error has happen
    tty->print_cr("#");
    tty->print_cr("# HotSpot Virtual Machine Error, %s", title);
    tty->print_cr("# Please report this error at");
    tty->print_cr("# http://java.sun.com/cgi-bin/bugreport.cgi");
    tty->print_cr("#");
    tty->print_cr("# Java VM: %s (%s %s)",
                   Abstract_VM_Version::vm_name(),
                   Abstract_VM_Version::vm_release(),
                   Abstract_VM_Version::vm_info_string()
                 );
    tty->print_cr("#");
  }
#ifdef PRODUCT
  else            // Do not print out any message in PRODUCT mode
#endif // PRODUCT
  {
    char* begin = message;
    // print line by line
    for (;;) {
      char* end = strchr(begin, '\n');
      if (!end) {
        break;
      }
      *end = '\0';
      tty->print_raw("# ");
      tty->print_raw(begin);
      *end = '\n';
      begin = end + 1;
    }
    // print last line
    if (*begin) {
      tty->print_raw("# ");
      tty->print_raw(begin);
      tty->cr();
    }
    // print end mark
    tty->print_cr("#");
  }

#ifndef PRODUCT
  if (WizardMode || DumpEventsAtError) {
    tty->print_cr("#");
    report_vm_state();
  }
#ifdef COMPILER2
  if (DumpInlineImage) {
    // For easy diffing, re-read the image file
    // and write it back in sorted form
    InlineImage::parse_from_file();
    InlineImage::sort_by_name();
    InlineImage::dump_to_file();
  }
#endif // COMPILER2
#endif // !PRODUCT

  Thread* current = ThreadLocalStorage::thread();
  bool really_current = true;
  if (is_vm_internal_error) {
    // printout is busted  
    // Dispatch the error to active vm operation if present
    VM_Operation* op = active_vm_operation();
    if (op != NULL) {
      op->report_error(title, message);
      current = op->calling_thread();
      really_current = false;
    }
    tty->print_cr("# Error ID: %s", loc_buf);
    tty->print_cr("#");
  }

  if (current != NULL) {
    if (really_current) tty->print("# Problematic Thread: ");
    else tty->print("# Thread requesting VM operation: ");
    current->print();
    tty->print_cr("");
    tty->print_cr("#");
  }

#ifndef PRODUCT
  static int continue_counter = 0;
  if (Debugging && continue_counter++ < 5) {
    // don't stop/abort VM when assertion was triggered during 
    // expression evaluation in the debugger
    tty->print("Continuing...\n");
    dec_error_level();
    return;
  } 
#endif // !PRODUCT

  // make sure all the output up to this point is emitted
  tty->flush();

  if (ShowMessageBoxOnError) {
    strcat(message, "\n\nDo you want to debug the problem?");
    if (!os::message_box(title, message)) {
      os::print_vm_state_before_abort(tty);
      os::abort();
    }
  } else {
#ifndef PRODUCT
    if (WizardMode) {
      tty->print_cr("Dumping thread state before exit....");
      ps();
    }
#endif // !PRODUCT
    if (is_vm_internal_error) {
      os::print_vm_state_before_abort(tty);
      os::abort();
    }
  }
  dec_error_level();
}


// ------ helper functions for debugging go here ------------

#ifndef PRODUCT
// All debug entries should be wrapped with a stack allocated
// Command object. It makes sure a resource mark is set and
// flushes the logfile to prevent file sharing problems.

class Command : public StackObj {
 private:
  ResourceMark rm;
  ResetNoHandleMark rnhm;
  HandleMark   hm;
 public:
  static int level;
  Command(const char* str) {
    if (level++ > 0)  return;
    tty->cr();
    tty->print_cr("\"Executing %s\"", str);
    Debugging = true;
  }

  ~Command() { tty->flush(); Debugging = (--level > 0); }
};

int Command::level = 0;


#ifndef CORE

extern "C" void blob(CodeBlob* cb) {  
  Command c("blob");
  cb->print();  
}


extern "C" void dump_vtable(address p) {
  Command c("dump_vtable");
  klassOop k = (klassOop)p;
  instanceKlass::cast(k)->vtable()->print();
}


extern "C" void nm(intptr_t p) {
  // Actually we look through all CodeBlobs (the nm name has been kept for backwards compatability)
  Command c("nm");
  CodeBlob* cb = CodeCache::find_blob((address)p);
  if (cb == NULL) {
    tty->print_cr("NULL");
  } else {
    cb->print();
  }
}


extern "C" void disnm(intptr_t p) {
  Command c("disnm");
  CodeBlob* cb = CodeCache::find_blob((address) p);
  cb->print();
  Disassembler::decode(cb);
}


extern "C" void printnm(intptr_t p) {
  Command c("printnm");
  CodeBlob* cb = CodeCache::find_blob((address) p);
  if (cb->is_nmethod()) {
    nmethod* nm = (nmethod*)cb;
    nm->print();
    nm->print_code();
    nm->print_pcs();
    nm->oop_maps()->print();
    if (PrintDebugInfo) {
      nm->print_scopes();
    }
  }
}


#endif // !CORE


extern "C" void universe() {
  Command c("universe");
  Universe::print();
}


extern "C" void verify() {
  // try to run a verify on the entire system
  // note: this may not be safe if we're not at a safepoint; for debugging,
  // this manipulates the safepoint settings to avoid assertion failures
  Command c("universe verify");
  bool safe = SafepointSynchronize::is_at_safepoint();
  if (!safe) {
    tty->print_cr("warning: not at safepoint -- verify may fail");
    SafepointSynchronize::set_is_at_safepoint();
  }
  // Ensure Eden top is correct before verification
  Universe::heap()->prepare_for_verify();
  Universe::verify(true);
  if (!safe) SafepointSynchronize::set_is_not_at_safepoint();
}


extern "C" void pp(void* p) {
  Command c("pp");
  FlagSetting fl(PrintVMMessages, true);
  if (Universe::heap()->is_in(p)) {
    oop obj = oop(p);
    obj->print();
  } else {
    tty->print("%#p", p);
  }
}


// pv: print vm-printable object
extern "C" void pa(intptr_t p)   { ((AllocatedObj*) p)->print(); }
extern "C" void findpc(intptr_t x);

extern "C" void ps() { // print stack
  Command c("ps");


  // Prints the stack of the current Java thread
  JavaThread* p = JavaThread::active();
  tty->print(" for thread: ");
  p->print();
  tty->cr();

  if (p->has_last_Java_frame()) {
    // If the last_Java_fp is set we are in C land and
    // can call the standard stack_trace function.
    p->trace_stack();
  } else {    
  frame f = JavaThread::current_frame_guess();
    RegisterMap reg_map(p);
    f = f.sender(&reg_map);
    tty->print("(guessing starting frame id=%#p based on current fp)\n", f.id());
    p->trace_stack_from(vframe::new_vframe(&f, &reg_map, p));
  pd_ps(f);
  }

}


extern "C" void psf() { // print stack frames
  {
    Command c("psf");
    JavaThread* p = JavaThread::active();
    tty->print(" for thread: ");
    p->print();
    tty->cr();
    if (p->has_last_Java_frame()) {
      p->trace_frames();
    }
  }
}


extern "C" void threads() {
  Command c("threads");
  Threads::print(false, true);
}


extern "C" void psd() {
  Command c("psd");
  SystemDictionary::print();
}


extern "C" void safepoints() {
  Command c("safepoints");
  SafepointSynchronize::print_state();
}


extern "C" void pss() { // print all stacks
  Command c("pss");
  Threads::print(true, true);
}


extern "C" void debug() {		// to set things up for compiler debugging
  Command c("debug");
  WizardMode = true;
  PrintVMMessages = PrintCompilation = true;
  PrintInlining = PrintAssembly = true;
  tty->flush();
}


extern "C" void ndebug() {		// undo debug()
  Command c("ndebug");
  PrintCompilation = false;
  PrintInlining = PrintAssembly = false;
  tty->flush();
}


extern "C" void flush()  {
  Command c("flush");
  tty->flush();
}


extern "C" void events() {
  Command c("events");
  Events::print_last(50);
}


extern "C" void nevents(int n) {
  Command c("events");
  Events::print_last(n);
}


#ifndef CORE

extern "C" methodOop findm(intptr_t pc) { 
  Command c("findm");
  nmethod* nm = CodeCache::find_nmethod((address)pc);
  return (nm == NULL) ? NULL : nm->method(); 
}


extern "C" nmethod* findnm(intptr_t addr) {
  Command c("findnm");
  return  CodeCache::find_nmethod((address)addr);  
}

#endif // !CORE


static address same_page(address x, address y) {
  intptr_t page_bits = -os::vm_page_size();
  if ((intptr_t(x) & page_bits) == (intptr_t(y) & page_bits)) {
    return x;
  } else if (x > y) {
    return (address)(intptr_t(y) | ~page_bits) + 1;
  } else {
    return (address)(intptr_t(y) & page_bits);
  }
}


static void find(intptr_t x, bool print_pc) {
  address addr = (address)x;

  CodeBlob* b = CodeCache::find_blob(addr);
  if (b != NULL) {
    if (b->is_buffer_blob()) {
      // the interpreter is generated into a buffer blob
      InterpreterCodelet* i = Interpreter::codelet_containing(addr);
      if (i != NULL) { 
        i->print(); 
        return; 
      }
      if (Interpreter::contains(addr)) {
        tty->print_cr(INTPTR_FORMAT " is pointing into interpreter code (not bytecode specific)", addr);
        return;
      }
      // the stubroutines are generated into a buffer blob
      StubCodeDesc* d = StubCodeDesc::desc_for(addr);
      if (d != NULL) { 
        d->print(); 
        if (print_pc) tty->cr();
        return; 
      }
      if (StubRoutines::contains(addr)) {
        tty->print_cr(INTPTR_FORMAT " is pointing to an (unnamed) stub routine", addr);
        return;
      }
      // the InlineCacheBuffer is using stubs generated into a buffer blob
#ifndef CORE
      if (InlineCacheBuffer::contains(addr)) {
        tty->print_cr(INTPTR_FORMAT "is pointing into InlineCacheBuffer", addr);
        return;
      }
      VtableStub* v = VtableStubs::stub_containing(addr);
      if (v != NULL) {
        v->print();
        return;
      }
#endif // CORE
    }
#ifndef CORE
    if (print_pc && b->is_nmethod()) {
      ResourceMark rm;
      tty->print("%#p: Compiled ", addr);
      ((nmethod*)b)->method()->print_value_on(tty); 
      tty->print("  = (CodeBlob*)" INTPTR_FORMAT, b);
      tty->cr();
      return; 
    }
#endif // CORE
    b->print(); 
    return; 
  }

  HeapWord* p = NULL;
  if (Universe::heap()->is_in(addr)) {
    p = Universe::heap()->block_start(addr);
  }
  if (p != NULL && Universe::heap()->block_is_obj(p)) { 
    oop(p)->print(); 
    if (p != (HeapWord*)x && oop(p)->is_method() &&
	methodOop(p)->contains(addr) && !methodOop(p)->is_native()) {
      tty->print_cr("bci_from(%p) = %d; print_codes():", addr, methodOop(p)->bci_from(address(x)));
      methodOop(p)->print_codes();
    }
    return; 
  }
  if (JNIHandles::is_global_handle((jobject) addr)) {
    tty->print_cr(INTPTR_FORMAT "is a global jni handle", addr);
    return;
  }
  if (JNIHandles::is_weak_global_handle((jobject) addr)) {
    tty->print_cr(INTPTR_FORMAT "is a weak global jni handle", addr);
    return;
  }
  if (JNIHandleBlock::any_contains((jobject) addr)) {
    tty->print_cr(INTPTR_FORMAT "is a local jni handle", addr);
    return;
  }

#ifndef CORE
  for(JavaThread *thread = Threads::first(); thread; thread = thread->next()) {
    // Check for safepoint code buffer
    address cpc = thread->safepoint_state()->compute_adjusted_pc(addr);
    if (cpc != addr) {
      tty->print_cr(INTPTR_FORMAT "is pointing into safepoint codebuffer for thread: " INTPTR_FORMAT, addr, thread);
      CodeBlob* n = CodeCache::find_blob((address)cpc);
      if (n != NULL) { 
        n->print(); 
        return; 
      }
    }      
    // Check for priviledge stack
    if (thread->privileged_stack_top() != NULL && thread->privileged_stack_top()->contains(addr)) {
      tty->print_cr(INTPTR_FORMAT "is pointing into the priviledge stack for thread: " INTPTR_FORMAT, addr, thread);
      return;
    }
  }
#endif // !CORE
  // If it's a java thread print information about that.
  for(JavaThread *jt = Threads::first(); jt; jt=jt->next()) {
    if (addr == (address)jt) {
       jt->print();
       return;
    }
  }
  
  // Try an OS specific find
  if (os::find(addr)) {
    return;
  }

  if (print_pc) {
    tty->print_cr(INTPTR_FORMAT ": probably in C++ code; check debugger", addr);
    Disassembler::decode(same_page(addr-40,addr),same_page(addr+40,addr));
    return;
  }

  tty->print_cr(INTPTR_FORMAT "is pointing to unknown location", addr);
}

// Another interface that isn't ambiguous in dbx.
// Can we someday rename the other find to hsfind?
extern "C" void hsfind(intptr_t x) {
  Command c("hsfind");
  find(x, false);
}

extern "C" void find(intptr_t x) {
  Command c("find");
  find(x, false);
}


extern "C" void findpc(intptr_t x) {
  Command c("findpc");
  find(x, true);
}


// int versions of all methods to avoid having to type type casts in the debugger

void pp(intptr_t p)          { pp((void*)p); }
void pp(oop p)               { pp((void*)p); }

void help() { 
  Command c("help");
  tty->print_cr("basic");
  tty->print_cr("  pp(void* p)   - try to make sense of p");
  tty->print_cr("  pv(intptr_t p)- ((PrintableResourceObj*) p)->print()");
  tty->print_cr("  ps()          - print current thread stack");
  tty->print_cr("  pss()         - print all thread stacks");
  tty->print_cr("  pm(int pc)    - print methodOop given compiled PC");
  tty->print_cr("  findm(intptr_t pc) - finds methodOop");
  tty->print_cr("  find(intptr_t x)   - finds & prints nmethod/stub/bytecode/oop based on pointer into it");
 
  tty->print_cr("misc.");
  tty->print_cr("  flush()       - flushes the log file");
  tty->print_cr("  events()      - dump last 50 events");


  tty->print_cr("compiler debugging");
  tty->print_cr("  debug()       - to set things up for compiler debugging");
  tty->print_cr("  ndebug()      - undo debug");
}

#endif // PRODUCT


