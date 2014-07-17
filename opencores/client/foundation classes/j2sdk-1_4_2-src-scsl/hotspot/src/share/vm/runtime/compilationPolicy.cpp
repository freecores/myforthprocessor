#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)compilationPolicy.cpp	1.25 03/01/23 12:21:44 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_compilationPolicy.cpp.incl"

CompilationPolicy* CompilationPolicy::_policy;
elapsedTimer       CompilationPolicy::_accumulated_time;
bool               CompilationPolicy::_in_vm_startup;

// Determine compilation policy based on command line argument
void compilationPolicy_init() {
  CompilationPolicy::set_in_vm_startup(DelayCompilationDuringStartup);

  switch(CompilationPolicyChoice) {
  case 0:
    CompilationPolicy::set_policy(new SimpleCompPolicy());
    break;

  case 1:
#ifdef COMPILER2
    CompilationPolicy::set_policy(new StackWalkCompPolicy());
#else
    Unimplemented();
#endif
    break;

  default:
    fatal("CompilationPolicyChoice must be in the range: [0-1]");
  }
}

void CompilationPolicy::completed_vm_startup() {
  if (TraceCompilationPolicy) {
    tty->print("CompilationPolicy: completed vm startup.\n");
  }
  _in_vm_startup = false;
}

// Returns true if m must be compiled before executing it
// This is intended to force compiles for methods (usually for
// debugging) that would otherwise be interpreted for some reason. 
bool CompilationPolicy::mustBeCompiled(methodHandle m) {
  if (m->has_compiled_code()) return false;       // already compiled
  if (!canBeCompiled(m))      return false;

  return !UseInterpreter ||                                              // must compile all methods
         (UseCompiler && AlwaysCompileLoopMethods && m->has_loops()); // eagerly compile loop methods
}

// Returns true if m is allowed to be compiled   
bool CompilationPolicy::canBeCompiled(methodHandle m) {
  if (m->is_abstract()) return false;
  if (DontCompileHugeMethods && m->code_size() > HugeMethodLimit) return false;

  return !m->is_not_compilable();
}

#ifndef PRODUCT
void CompilationPolicy::print_time() {
  tty->print_cr ("Accumulated compilationPolicy times:");
  tty->print_cr ("---------------------------");
  tty->print_cr ("  Total: %3.3f sec.", _accumulated_time.seconds());
}

static void trace_osr_completion(nmethod* osr_nm) {
  if (TraceOnStackReplacement) {
    if (osr_nm == NULL) tty->print_cr("compilation failed");
    else tty->print_cr("nmethod " INTPTR_FORMAT, osr_nm);
  }
}
#endif // !PRODUCT

void CompilationPolicy::reset_counter_for_invocation_event(methodHandle m) {
  // Make sure invocation and backedge counter doesn't overflow again right away
  // as would be the case for native methods.
  m->invocation_counter()->set_carry();
  m->backedge_counter()->set_carry();

  // BUT also make sure the method doesn't look like it was never executed.
  int icount = m->invocation_counter()->count();
  int bcount = m->backedge_counter()->count();
  int min_threshold;
  int half_min_inlining = MinInliningThreshold / 2;

  // We have to do this, because the invocation
  // counter and backedge counter combined make
  // up the inliningthreshold, but if we have a
  // 0 count backedge counter then we should use
  // MinInlingThreshold, otherwise half that
  if (bcount < half_min_inlining) {
    min_threshold = MinInliningThreshold;
  } else {
    min_threshold = half_min_inlining;
  }

  icount = MIN2(icount >> 1, (int) (min_threshold + 1));
  bcount = MIN2(bcount >> 1, (int) (min_threshold + 1));

  m->invocation_counter()->set(m->invocation_counter()->state(), icount);
  m->backedge_counter()->set(m->backedge_counter()->state(), bcount);

  assert(!m->was_never_executed(), "don't reset to 0 -- could be mistaken for never-executed");
}

void CompilationPolicy::reset_counter_for_back_branch_event(methodHandle m) {
  // delay next back-branch event
  InvocationCounter* i = m->invocation_counter();
  InvocationCounter* b = m->backedge_counter();

  // Combined these two stats make up CompileThreshold
  i->set(i->state(), CompileThreshold / 2);
  b->set(b->state(), CompileThreshold / 2);
  
}

// SimpleCompPolicy - compile current method

void SimpleCompPolicy::method_invocation_event( methodHandle m, TRAPS) {
  assert(UseCompiler || CompileTheWorld, "UseCompiler should be set by now.");

  int hot_count = m->invocation_count();
  reset_counter_for_invocation_event(m);
  const char* comment = "count";

  if (m->code() == NULL && !delayCompilationDuringStartup() && canBeCompiled(m) && UseCompiler) {
    CompileBroker::compile_method(m, InvocationEntryBci,
                                  m, hot_count, comment, CHECK);
  }
}

void SimpleCompPolicy::method_back_branch_event(methodHandle m, int branch_bci, int loop_top_bci, TRAPS) {
  assert(UseCompiler || CompileTheWorld, "UseCompiler should be set by now.");

  int hot_count = m->backedge_count();
  const char* comment = "backedge_count";

  if (!m->is_not_osr_compilable() && !delayCompilationDuringStartup()) {
    CompileBroker::compile_method(m, loop_top_bci, m, hot_count, comment, CHECK);

    NOT_PRODUCT(trace_osr_completion(m->lookup_osr_nmethod_for(loop_top_bci));)
  }
}

int SimpleCompPolicy::compilation_level(methodHandle m, int branch_bci)
{
  return AbstractCompiler::CompLevel_full_optimization;
}

// StackWalkCompPolicy - walk up stack to find a suitable method to compile

#ifdef COMPILER2
// Consider m for compilation
void StackWalkCompPolicy::method_invocation_event(methodHandle m, TRAPS) {
  assert(UseCompiler || CompileTheWorld, "UseCompiler should be set by now.");

  int hot_count = m->invocation_count();
  reset_counter_for_invocation_event(m);
  const char* comment = "count";

  if (m->code() == NULL && !delayCompilationDuringStartup() && canBeCompiled(m) && UseCompiler) {
    ResourceMark rm(THREAD);  
    JavaThread *thread = (JavaThread*)THREAD;
    frame       fr     = thread->last_frame();
    assert(fr.is_interpreted_frame(), "must be interpreted");
    assert(fr.interpreter_frame_method() == m(), "bad method");

    if (TraceCompilationPolicy) {
      tty->print("method invocation trigger: %s ( interpreted %#x) ", m->name()->as_C_string(), (char*)m());
    }
    RegisterMap reg_map(thread, false);
    javaVFrame* triggerVF = thread->last_java_vframe(&reg_map);
    // triggerVF is the frame that triggered its counter
    RFrame* first = new InterpretedRFrame(triggerVF->fr(), thread, m);

    if (first->top_method()->code() != NULL) {
      // called obsolete method/nmethod -- no need to recompile
      if (TraceCompilationPolicy) tty->print(" --> %#x\n", first->top_method()->code());    
    } else {
      if (TimeCompilationPolicy) accumulated_time()->start();
      GrowableArray<RFrame*>* stack = new GrowableArray<RFrame*>(50);
      stack->push(first);
      RFrame* top = findTopInlinableFrame(stack);
      if (TimeCompilationPolicy) accumulated_time()->stop();
      assert(top != NULL, "findTopInlinableFrame returned null");
      if (TraceCompilationPolicy) top->print();
      CompileBroker::compile_method(top->top_method(), InvocationEntryBci,
                                    m, hot_count, comment, CHECK);
    }
  }
}

void StackWalkCompPolicy::method_back_branch_event(methodHandle m, int branch_bci, int loop_top_bci, TRAPS) {
  assert(UseCompiler || CompileTheWorld, "UseCompiler should be set by now.");

  int hot_count = m->backedge_count();
  const char* comment = "backedge_count";

  if (!m->is_not_osr_compilable() && !delayCompilationDuringStartup()) {
    CompileBroker::compile_method(m, loop_top_bci, m, hot_count, comment, CHECK);

    NOT_PRODUCT(trace_osr_completion(m->lookup_osr_nmethod_for(loop_top_bci));)
  }
}

int StackWalkCompPolicy::compilation_level(methodHandle m, int osr_bci)
{
  int comp_level = AbstractCompiler::CompLevel_full_optimization;
#ifdef COMPILER2
  if (TieredCompilation && osr_bci == InvocationEntryBci) {
    if (CompileTheWorld) {
      // Under CTW, the first compile is tier1, the second tier2
      if (!m->tier1_compile_done()) {
	comp_level = AbstractCompiler::CompLevel_fast_compile;
      }
    } else {
      // Before tier1 is done, use invocation_count + backedge_count to
      // compare against the threshold.  After that, the counters may/will
      // be reset, so rely on the straight interpreter_invocation_count.
      if (m->tier1_compile_done()) {
	if (m->interpreter_invocation_count() < Tier2CompileThreshold) {
	  comp_level = AbstractCompiler::CompLevel_fast_compile;
	}
      } else if (m->invocation_count() + m->backedge_count() < 
		 Tier2CompileThreshold) { 
	comp_level = AbstractCompiler::CompLevel_fast_compile;
      }
    }

    // Tag the method as having completed a tier 1 compile.
    if (comp_level == AbstractCompiler::CompLevel_fast_compile) {
      m->set_tier1_compile_done();
    }
  }
#endif
  return comp_level;
}


RFrame* StackWalkCompPolicy::findTopInlinableFrame(GrowableArray<RFrame*>* stack) {
  // go up the stack until finding a frame that (probably) won't be inlined
  // into its caller
  RFrame* current = stack->at(0); // current choice for stopping
  assert( current && !current->is_compiled(), "" );
  const char* msg = NULL; 

  while (1) {

    // before going up the stack further, check if doing so would get us into
    // compiled code
    RFrame* next = senderOf(current, stack);
    if( !next )               // No next frame up the stack?
      break;                  // Then compile with current frame

    methodHandle m = current->top_method();
    methodHandle next_m = next->top_method();

    if( !Inline ) {           // Inlining turned off
      msg = "(Inlining turned off)";
      break;
    }
    if (next_m->is_not_compilable()) { // Did fail to compile this before/
      msg = "(caller not compilable)";
      break;
    }
    if (next->num() > MaxRecompilationSearchLength) {
      // don't go up too high when searching for recompilees
      msg = "(don't go up any further: > MaxRecompilationSearchLength)";
      break;
    }
    if (next->distance() > MaxInterpretedSearchLength) {
      // don't go up too high when searching for recompilees
      msg = "(don't go up any further: next > MaxInterpretedSearchLength)";
      break;
    }
    // Compiled frame above already decided not to inline;
    // do not recompile him.
    if (next->is_compiled()) {
      msg = "(not going up into optimized code)";
      break;
    }

    // Interpreted frame above us was already compiled.  Do not force
    // a recompile, although if the frame above us runs long enough an
    // OSR might still happen.
    if( current->is_interpreted() && next_m->has_compiled_code() ) {
      msg = "(not going up -- already compiled caller)";
      break;
    }

    // Compute how frequent this call site is.  We have current method 'm'.
    // We know next method 'next_m' is interpreted.  Find the call site and
    // check the various invocation counts.
    int invcnt = 0;             // Caller counts
#ifdef COMPILER2
    if (ProfileInterpreter) {
      invcnt = next_m->interpreter_invocation_count();
    }
#endif
    int cnt = 0;                // Call site counts
#ifdef COMPILER2
    if (ProfileInterpreter && next_m->method_data() != NULL) {
      ResourceMark rm;
      int bci = next->top_vframe()->bci();
      ProfileData* data = next_m->method_data()->bci_to_data(bci);
      if (data != NULL && data->is_CounterData())
	cnt = data->as_CounterData()->count();
    }
#endif

    // Caller counts / call-site counts; i.e. is this call site
    // a hot call site for method next_m?
    int freq = (invcnt) ? cnt/invcnt : cnt;
    
    // Check size and frequency limits
    if ((msg = InliningPolicy::shouldInline(m, freq, cnt)) != NULL) {
      break;
    }
    // Check inlining negative tests
    if ((msg = InliningPolicy::shouldNotInline(m, NULL)) != NULL) {
      break;
    }


    // If the caller method is too big or something then we do not want to
    // compile it just to inline a method
    if (!canBeCompiled(next_m)) {
      msg = "caller cannot be compiled";
      break;
    }

    if( next_m->name() == vmSymbols::class_initializer_name() ) {
      msg = "do not compile class initializer (OSR ok)";
      break;
    }

    current = next;
  }

  assert( !current || !current->is_compiled(), "" );

  if (TraceCompilationPolicy && msg) tty->print("%s\n", msg);

  return current;
}

RFrame* StackWalkCompPolicy::senderOf(RFrame* rf, GrowableArray<RFrame*>* stack) {
  RFrame* sender = rf->caller();
  if (sender && sender->num() == stack->length()) stack->push(sender);
  return sender;
}

#endif

