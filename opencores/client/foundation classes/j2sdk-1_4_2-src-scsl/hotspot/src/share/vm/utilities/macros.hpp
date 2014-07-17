#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)macros.hpp	1.28 03/01/23 12:28:30 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Use this to mark code that needs to be cleaned up (for development only)
#define NEEDS_CLEANUP

// Makes a string of the argument (which is not macro-expanded)
#define STR(a)  #a

// Makes a string of the macro expansion of a
#define XSTR(a) STR(a)

// CORE variant
#ifdef CORE
  Error: CORE should not be defined explicitly
#endif

#if !defined(COMPILER1) && !defined(COMPILER2)
#define CORE
#define CORE_ONLY(code) code
#define NOT_CORE(code)
#define CORE_RETURN  {}
#define CORE_RETURN0 { return 0; }
#else // CORE
#define CORE_ONLY(code)
#define NOT_CORE(code) code
#define CORE_RETURN  /*next token must be ;*/
#define CORE_RETURN0 /*next token must be ;*/
#endif // CORE

// COMPILER1 variant
#ifdef COMPILER1
#ifdef COMPILER2
  Error: COMPILER1 and COMPILER2 should not be defined simultaneously
#endif
#define COMPILER1_ONLY(code) code
#define NOT_COMPILER1(code)
#else // COMPILER1
#define COMPILER1_ONLY(code)
#define NOT_COMPILER1(code) code
#endif // COMPILER1

// COMPILER2 variant
#ifdef COMPILER2
#ifdef COMPILER1
  Error: COMPILER1 and COMPILER2 should not be defined simultaneously
#endif
#define COMPILER2_ONLY(code) code
#define NOT_COMPILER2(code)
#else // COMPILER2
#define COMPILER2_ONLY(code)
#define NOT_COMPILER2(code) code
#endif // COMPILER2

// PRODUCT variant
#ifdef PRODUCT
#define PRODUCT_ONLY(code) code
#define NOT_PRODUCT(code)
#define PRODUCT_RETURN  {}
#define PRODUCT_RETURN0 { return 0; }
#define PRODUCT_RETURN_(code) { code }
#else // PRODUCT
#define PRODUCT_ONLY(code)
#define NOT_PRODUCT(code) code
#define PRODUCT_RETURN  /*next token must be ;*/
#define PRODUCT_RETURN0 /*next token must be ;*/
#define PRODUCT_RETURN_(code)  /*next token must be ;*/
#endif // PRODUCT

#ifdef ASSERT
#define DEBUG_ONLY(code) code
#define NOT_DEBUG(code)
// Historical.
#define debug_only(code) code
#else // ASSERT
#define DEBUG_ONLY(code)
#define NOT_DEBUG(code) code
#define debug_only(code)
#endif // ASSERT

#ifdef  _LP64
#define LP64_ONLY(code) code
#define NOT_LP64(code)
#else  // !LP64
#define LP64_ONLY(code)
#define NOT_LP64(code) code
#endif // LP64

#define FIX_THIS(code) report_assertion_failure("FIX_THIS",__FILE__, __LINE__, "")

#define define_pd_global(type, name, value) const type pd_##name = value;

