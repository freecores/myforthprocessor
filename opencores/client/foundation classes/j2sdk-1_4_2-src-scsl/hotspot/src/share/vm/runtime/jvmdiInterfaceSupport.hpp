#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmdiInterfaceSupport.hpp	1.21 03/01/23 12:23:36 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Interface support definitions for JVMDI

class NoTransition {
public:
  NoTransition(JavaThread *thr) {
    VM_Exit::block_if_vm_exited();
  }
};

#define JVMDI_END                                                                    \
    }                                                                                \
    }

#define __JVMDI_ENTER(TransitionClass, trace, result_type, methodName, signature)    \
extern "C" {                                                                         \
  result_type JNICALL methodName signature {                                         \
    JavaThread* thread = (JavaThread*) ThreadLocalStorage::thread();                 \
    TransitionClass __tiv(thread);                                                   \
    __ENTRY(result_type, methodName##signature, thread)                              \
    debug_only(VMNativeEntryWrapper __vew;)                                          \
    debug_only(const char* const _jvmdi_methodName_ = #methodName;)                  \
    debug_only(bool  _trace_ = trace; );                                             \
    debug_only(if (TraceJVMDICalls && _trace_) { JvmdiTraceWrapper _ignore(#methodName); });    

#define JVMDI_ENTER(result_type, methodName, signature)                              \
  __JVMDI_ENTER(ThreadInVMfromNative, true, result_type, methodName, signature)

#define JVMDI_ENTER_IN_NATIVE(result_type, methodName, signature)                    \
  __JVMDI_ENTER(NoTransition, true, result_type, methodName, signature)                  

#define JVMDI_ENTER_IN_NATIVE_NO_TRACE(result_type, methodName, signature)           \
  __JVMDI_ENTER(NoTransition, false, result_type, methodName, signature) 

#define JVMDI_ENTER_NO_TRACE(result_type, methodName, signature)                     \
  __JVMDI_ENTER(ThreadInVMfromNative, false, result_type, methodName, signature)     
  
#define JVMDI_RETURN(result)                                                         \
  {                                                                                  \
    jvmdiError _result = (result);                                                   \
    debug_only(                                                                      \
      if (TraceJVMDICalls && _trace_) {                                              \
        ResourceMark rm;                                                             \
        if (_result == JVMDI_ERROR_NONE) {                                           \
          tty->print_cr("  %s returns without error", _jvmdi_methodName_);           \
        } else {                                                                     \
          tty->print_cr("  %s returns with error %d", _jvmdi_methodName_, _result);  \
        }                                                                            \
      }                                                                              \
      if (StopOnJVMDIErrorReturn) {                                                  \
        switch (_result) {                                                           \
          /*                                                                         \
           * One might want to add more labels to this branch                        \
           * if you wanted not to stop on "normal" error returns.                    \
           */                                                                        \
        case JVMDI_ERROR_INTERRUPT:                                                  \
        case JVMDI_ERROR_NONE:                                                       \
          /* Nothing to do. */                                                       \
          break;                                                                     \
        default:                                                                     \
          /* Stop on error */                                                        \
          assert(false, "JVMDI returns error");                                      \
          break;                                                                     \
        }                                                                            \
      }                                                                              \
    )                                                                                \
    return _result;                                                                  \
  }                                                                                  \


