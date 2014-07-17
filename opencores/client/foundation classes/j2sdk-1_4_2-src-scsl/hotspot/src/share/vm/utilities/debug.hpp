#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)debug.hpp	1.32 03/01/23 12:27:53 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// assertions
#ifdef ASSERT
// Turn this off by default:
//#define USE_REPEATED_ASSERTS
#ifdef USE_REPEATED_ASSERTS
  #define assert(p,msg)                                              \
    { for (int __i = 0; __i < AssertRepeat; __i++) {                 \
        if (!(p)) {                                                  \
          report_assertion_failure(XSTR(p),__FILE__, __LINE__, msg); \
          BREAKPOINT;                                                \
        }                                                            \
      }                                                              \
    }
#else
  #define assert(p,msg)                                          \
    if (!(p)) {                                                  \
      report_assertion_failure(XSTR(p),__FILE__, __LINE__, msg); \
      BREAKPOINT;                                                \
    }
#endif
#else
  #define assert(p,msg)
#endif


// fatals
#define fatal(m)                             { report_fatal(__FILE__, __LINE__, m                                    ); BREAKPOINT; }
#define fatal1(m,x1)                         { report_fatal(__FILE__, __LINE__, m, x1                                ); BREAKPOINT; }
#define fatal2(m,x1,x2)                      { report_fatal(__FILE__, __LINE__, m, x1, x2                            ); BREAKPOINT; }
#define fatal3(m,x1,x2,x3)                   { report_fatal(__FILE__, __LINE__, m, x1, x2, x3                        ); BREAKPOINT; }
#define fatal4(m,x1,x2,x3,x4)                { report_fatal(__FILE__, __LINE__, m, x1, x2, x3, x4                    ); BREAKPOINT; }
#define fatal5(m,x1,x2,x3,x4,x5)             { report_fatal(__FILE__, __LINE__, m, x1, x2, x3, x4, x5                ); BREAKPOINT; }
#define fatal6(m,x1,x2,x3,x4,x5,x6)          { report_fatal(__FILE__, __LINE__, m, x1, x2, x3, x4, x5, x6            ); BREAKPOINT; }
#define fatal7(m,x1,x2,x3,x4,x5,x6,x7)       { report_fatal(__FILE__, __LINE__, m, x1, x2, x3, x4, x5, x6, x7        ); BREAKPOINT; }
#define fatal8(m,x1,x2,x3,x4,x5,x6,x7,x8)    { report_fatal(__FILE__, __LINE__, m, x1, x2, x3, x4, x5, x6, x7, x8    ); BREAKPOINT; }
#define fatal9(m,x1,x2,x3,x4,x5,x6,x7,x8,x9) { report_fatal(__FILE__, __LINE__, m, x1, x2, x3, x4, x5, x6, x7, x8, x9); BREAKPOINT; }


// guarantee is like assert except it's always executed -- use it for cheap tests that catch errors 
// that would otherwise be hard to find
#define guarantee(b,msg)         { if (!(b)) fatal(msg); }

#define ShouldNotCallThis()      { report_should_not_call        (__FILE__, __LINE__); BREAKPOINT; }
#define ShouldNotReachHere()     { report_should_not_reach_here  (__FILE__, __LINE__); BREAKPOINT; }
#define Unimplemented()          { report_unimplemented          (__FILE__, __LINE__); BREAKPOINT; }
#define Untested(msg)            { report_untested               (__FILE__, __LINE__, msg); BREAKPOINT; }

// No plain-text messages should be printed out in product mode
void obfuscate_location(const char *file_name, int line_no, char* buf, int buflen);

// main error reporting function.
// is_vm_internal_error allows us to print a different error message
// for crashes known to have occurred in native code outside the VM.
// NOTE: This routine returns control to the caller if
// is_vm_internal_error is set to false!
void report_error(bool is_vm_internal_error, const char *file_name, int file_no, const char* title, const char* format, ...);

// error reporting helper functions
void report_assertion_failure(const char* code_str, const char* file_name, int line_no, const char* message);
void report_fatal(const char* file_name, int line_no, const char* format, ...);
void report_should_not_call(const char* file_name, int line_no);
void report_should_not_reach_here(const char* file_name, int line_no);
void report_subclass_responsibility(const char* file_name, int line_no);
void report_unimplemented(const char* file_name, int line_no);
void report_untested(const char* file_name, int line_no, const char* msg);
void report_vm_state();
void warning(const char* format, ...);

// Support for self-destruct
bool is_error_reported();

void pd_ps(frame f);
void pd_obfuscate_location(char *buf, int buflen);

jint inc_error_level();
jint dec_error_level();
