#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_linux.inline.hpp	1.15 03/01/23 11:03:59 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline const char* os::file_separator() {
  return "/";
}

inline const char* os::line_separator() {
  return "\n";
}

inline const char* os::path_separator() {
  return ":";
}

inline const char* os::jlong_format_specifier() {
  return "%lld";
}

inline const char* os::julong_format_specifier() {
  return "%llu";
}

// Tells whether we're running on an MP machine
inline bool os::is_MP() {
  return os::Linux::is_MP();
}

// File names are case-sensitive on windows only
inline int os::file_name_strcmp(const char* s1, const char* s2) {
  return strcmp(s1, s2);
}

inline bool os::obsolete_option(JavaVMOption *option) {
  return false;
}

inline bool os::uses_stack_guard_pages() {
  return true;
}

inline bool os::allocate_stack_guard_pages() {
  assert(uses_stack_guard_pages(), "sanity check");
  return true;
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
  return NAME_MAX + sizeof(dirent) + 1;
}

inline struct dirent* os::readdir(DIR* dirp, dirent *dbuf)
{
  dirent* p;
  int status;
  assert(dirp != NULL, "just checking");

  // NOTE: Linux readdir_r (on RH 6.2 and 7.2 at least) is NOT like the POSIX
  // version. Here is the doc for this function:
  // http://www.gnu.org/manual/glibc-2.2.3/html_node/libc_262.html

  if((status = ::readdir_r(dirp, dbuf, &p)) != 0) {
    errno = status;
    return NULL;
  } else
    return p;
}

inline char* os::asctime(const struct tm *tm, char *buf, int buflen) 
{
  return ::asctime_r(tm, buf);
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

// macros for restartable system calls

#define RESTARTABLE(_cmd, _result) do { \
    _result = _cmd; \
  } while((_result == OS_ERR) && (errno == EINTR))

#define RESTARTABLE_RETURN_INT(_cmd) do { \
  int _result; \
  RESTARTABLE(_cmd, _result); \
  return _result; \
} while(false)

// Reconciliation History
// os_solaris.inline.hpp	1.20 99/09/08 20:41:26
// End

