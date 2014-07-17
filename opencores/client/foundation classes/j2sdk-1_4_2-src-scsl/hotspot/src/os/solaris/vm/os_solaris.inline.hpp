#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_solaris.inline.hpp	1.45 03/02/06 14:23:30 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline const char* os::file_separator() { return "/"; }
inline const char* os::line_separator() { return "\n"; }
inline const char* os::path_separator() { return ":"; }

inline const char* os::jlong_format_specifier()   { return "%lld"; }
inline const char* os::julong_format_specifier()  { return "%llu"; }

// Tells whether we're running on an MP machine
inline bool os::is_MP() { return Solaris::is_MP();}

// File names are case-sensitive on windows only
inline int os::file_name_strcmp(const char* s1, const char* s2) {
  return strcmp(s1, s2);
}

inline bool os::uses_stack_guard_pages() {
  return true;
}

inline bool os::allocate_stack_guard_pages() {
  assert(uses_stack_guard_pages(), "sanity check");
  return thr_main();
}


// Bang the shadow pages if they need to be touched to be mapped.
inline void os::bang_stack_shadow_pages() {
}


inline DIR* os::opendir(const char* dirname)
{
  assert(dirname != NULL, "just checking");
  return ::opendir(dirname);
}

inline int os::readdir_buf_size(const char *path)
{
  int size = pathconf(path, _PC_NAME_MAX);
  return (size < 0 ? MAXPATHLEN : size) + sizeof(dirent) + 1;
}

inline struct dirent* os::readdir(DIR* dirp, dirent* dbuf)
{
  assert(dirp != NULL, "just checking");
#ifdef _LP64
  dirent* p;
  int status;

  if((status = ::readdir_r(dirp, dbuf, &p)) != 0) {
    errno = status;
    return NULL;
  } else
    return p;
#else
  return ::readdir_r(dirp, dbuf);
#endif
}

inline char* os::asctime(const struct tm *tm, char *buf, int buflen)
{
  return ::asctime_r(tm, buf, buflen);
}

inline struct tm* os::localtime(const time_t *clock, struct tm *res)
{
  return ::localtime_r(clock, res);
}

inline int os::closedir(DIR *dirp)
{
  assert(dirp != NULL, "just checking");
  return ::closedir(dirp);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// macros for interruptible io and system calls and system call restarting 

#define _INTERRUPTIBLE(_setup, _cmd, _result, _thread, _clear, _before, _after) \
do { \
  _setup; \
  _before; \
  OSThread* _osthread = _thread->osthread(); \
  if (_thread->has_last_Java_frame()) { \
    /* this is java interruptible io stuff */ \
      if ((os::is_interrupted(_thread, _clear)) \
       || ((_cmd) < 0 && errno == EINTR \
         && os::is_interrupted(_thread, _clear))) { \
        _result = OS_INTRPT; \
      } \
  } else { \
    /* this is normal blocking io stuff */ \
    _cmd; \
  } \
  _after; \
} while(false)

// Interruptible io support + restarting of interrupted system calls

#define INTERRUPTIBLE(_cmd, _result, _clear) do { \
  _INTERRUPTIBLE(JavaThread* _thread = os::Solaris::setup_interruptible(), _result = _cmd, _result, _thread, _clear, , os::Solaris::cleanup_interruptible(_thread) ); \
} while((_result == OS_ERR) && (errno == EINTR))


/* Use NORESTART when the system call cannot return EINTR, when something other
   than a system call is being invoked, or when the caller must do EINTR 
   handling. */

#define INTERRUPTIBLE_NORESTART(_cmd, _result, _clear) \
  _INTERRUPTIBLE(JavaThread* _thread = os::Solaris::setup_interruptible(), _result = _cmd, _result, _thread, _clear, , os::Solaris::cleanup_interruptible(_thread) )

#define INTERRUPTIBLE_RETURN_INT(_cmd, _clear) do { \
  int _result; \
  do { \
    INTERRUPTIBLE(_cmd, _result, _clear); \
  } while((_result == OS_ERR) && (errno == EINTR)); \
  return _result; \
} while(false)

#define INTERRUPTIBLE_RETURN_INT_NORESTART(_cmd, _clear) do { \
  int _result; \
  INTERRUPTIBLE_NORESTART(_cmd, _result, _clear); \
  return _result; \
} while(false)

/* Use the RESTARTABLE macros when interruptible io is not needed */

#define RESTARTABLE(_cmd, _result) do { \
  do { \
    _result = _cmd; \
  } while((_result == OS_ERR) && (errno == EINTR)); \
} while(false)

#define RESTARTABLE_RETURN_INT(_cmd) do { \
  int _result; \
  RESTARTABLE(_cmd, _result); \
  return _result; \
} while(false)

