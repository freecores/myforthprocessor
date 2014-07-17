#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ostream.cpp	1.41 03/02/28 16:49:32 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_ostream.cpp.incl"

extern "C" void jio_print(const char* s); // Declarationtion of jvm method

outputStream::outputStream(int width) {
  _width       = width;
  _position    = 0;
  _newlines    = 0;
  _precount    = 0;
  _indentation = 0;
}

outputStream::outputStream(int width, bool has_time_stamps) {
  _width       = width;
  _position    = 0;
  _newlines    = 0;
  _precount    = 0;
  _indentation = 0;
  if (has_time_stamps)  _stamp.update();
}

void outputStream::update_position(const char* s, size_t len) {
  for (size_t i = 0; i < len; i++) {
    char ch = s[i];
    if (ch == '\n') {
      _newlines += 1;
      _precount += _position + 1;
      _position = 0;
    } else if (ch == '\t') {
      _position += 8;
      _precount -= 7;  // invariant:  _precount + _position == total count
    } else {
      _position += 1;
    }
  }
}

#define BUFLEN 2000   /* max size of output of individual print() methods */

// Execute a vsprintf, using the given buffer if necessary.
// Return a pointer to the formatted string.
const char* outputStream::do_vsnprintf(char* buffer, size_t buflen,
                                       const char* format, va_list ap,
                                       bool add_cr,
                                       size_t& result_len) {
  const char* result;
  if (add_cr)  buflen--;
  if (!strchr(format, '%')) {
    // constant format string
    result = format;
    result_len = strlen(result);
  } else if (format[0] == '%' && format[1] == '%s' && format[2] == '\0') {
    // trivial copy-through format string
    result = va_arg(ap, const char*);
    result_len = strlen(result);
  } else if (vsnprintf(buffer, buflen, format, ap) >= 0) {
    result = buffer;
    result_len = strlen(result);
  } else {
    warning("increase BUFLEN in ostream.cpp -- output truncated");
    result = buffer;
    result_len = buflen - 1;
    buffer[result_len] = 0;
  }
  if (add_cr) {
    if (result != buffer) {
      strcpy(buffer, result);
      result = buffer;
    }
    buffer[result_len++] = '\n';
    buffer[result_len] = 0;
  }
  return result;
}

void outputStream::print(const char* format, ...) {
  char buffer[BUFLEN];
  va_list ap;
  va_start(ap, format);
  size_t len;
  const char* str = do_vsnprintf(buffer, BUFLEN, format, ap, false, len);
  write(str, len);
  va_end(ap);
}

void outputStream::print_cr(const char* format, ...) { 
  char buffer[BUFLEN];
  va_list ap;
  va_start(ap, format);
  size_t len;
  const char* str = do_vsnprintf(buffer, BUFLEN, format, ap, true, len);
  write(str, len);
  va_end(ap);
}

void outputStream::vprint(const char *format, va_list argptr) { 
  char buffer[BUFLEN];
  size_t len;
  const char* str = do_vsnprintf(buffer, BUFLEN, format, argptr, false, len);
  write(str, len);
}

void outputStream::vprint_cr(const char* format, va_list argptr) {
  char buffer[BUFLEN];
  size_t len;
  const char* str = do_vsnprintf(buffer, BUFLEN, format, argptr, true, len);
  write(str, len);
}

void outputStream::fill_to(int col) {
  while (position() < col) sp();
}

void outputStream::put(char ch) {
  assert(ch != 0, "please fix call site");
  char buf[] = { ch, '\0' };
  write(buf, 1);
}

void outputStream::sp() {
  this->write(" ", 1);
}

void outputStream::cr() {
  this->write("\n", 1);
}

void outputStream::stamp() {
  if (! _stamp.is_updated()) {
    _stamp.update(); // start at 0 on first call to stamp()
  }
  print("%.3f", _stamp.seconds());
}

void outputStream::indent() {
  while (_position < _indentation) sp();
}

void outputStream::print_jlong(jlong value) {
  // N.B. Same as INT64_FORMAT
  print(os::jlong_format_specifier(), value);
}

void outputStream::print_julong(julong value) {
  // N.B. Same as UINT64_FORMAT
  print(os::julong_format_specifier(), value);
}

stringStream::stringStream(size_t initial_size) : outputStream() {
  buffer_length = initial_size;
  buffer        = NEW_RESOURCE_ARRAY(char, buffer_length);
  buffer_pos    = 0;  
  buffer_fixed  = false;
}

// useful for output to fixed chunks of memory, such as performance counters
stringStream::stringStream(char* fixed_buffer, size_t fixed_buffer_size) : outputStream() {
  buffer_length = fixed_buffer_size;
  buffer        = fixed_buffer;
  buffer_pos    = 0;  
  buffer_fixed  = true;
}

void stringStream::write(const char* s, size_t len) {
  size_t end = buffer_pos + len;
  if (end >= buffer_length) {
    if (buffer_fixed) {
      // if buffer cannot resize, silently truncate
      end = buffer_length-1;
    } else {
      // resize the buffer by doubling
      if (end < buffer_length * 2) {
        end = buffer_length * 2;
      }
      char* oldbuf = buffer;
      buffer = NEW_RESOURCE_ARRAY(char, end);
      strncpy(buffer, oldbuf, buffer_pos);
      buffer_length = end;
    }
  }
  // invariant: buffer is always null-terminated
  buffer[buffer_pos + len] = 0;
  strncpy(buffer + buffer_pos, s, len);
  buffer_pos += len;

  // Note that the following does not depend on end.
  // This means that position and count get updated
  // even when overflow occurs.
  update_position(s, len);
}

char* stringStream::as_string() {
  char* copy = NEW_RESOURCE_ARRAY(char, buffer_pos+1);
  strncpy(copy, buffer, buffer_pos);
  copy[buffer_pos] = 0;  // terminating null
  return copy;
}

stringStream::~stringStream() {}

xmlStream*   xtty;
outputStream* tty;
outputStream* gclog_or_tty;
extern Mutex* tty_lock;

fileStream::fileStream(const char* file_name) {
  _file = fopen(file_name, "w");
}

void fileStream::write(const char* s, size_t len) {
  if (_file != NULL)  fwrite(s, 1, len, _file);
  update_position(s, len);
}

fileStream::~fileStream() {
  if (_file != NULL) {
    fclose(_file);
    _file = NULL;
  }
}

void fileStream::flush() {
  fflush(_file);
}


class defaultStream : public xmlTextStream {
 public:
  enum { NO_WRITER = -1 };
 private:
  bool         _inited;
  fileStream*  _log_file;  // XML-formatted file shared by all threads
  bool has_log_file() { 
    // lazily create log file (at startup, LogVMOutput is false even
    // if +LogVMOutput is used, because the flags haven't been parsed yet)
    if (!_inited)  init();
    return _log_file != NULL; 
  }

  void init();
  void init_log();
  void finish_log();
 public:
  // must defer time stamp due to the fact that os::init() hasn't
  // yet been called and os::elapsed_counter() may not be valid
  defaultStream() { 
    _log_file = NULL;
    _inited = false;
    _writer = -1;
    _last_writer = -1;
  }

  ~defaultStream() { 
    if (has_log_file())  finish_log();
  }

  virtual void write(const char* s, size_t len);

  void flush() {
    // assert(false, "can not flush buffer inside signal handler");
  }

  // advisory lock/unlock of _writer field:
 private:
  jint  _writer;    // thread_id with current rights to output
  jint  _last_writer;
 public:
  intptr_t hold(intptr_t writer_id);
  void release(intptr_t holder);
  intptr_t writer() { return _writer; }

  static defaultStream* instance;  // sole instance
};

defaultStream* defaultStream::instance = NULL;

#define LOG_MAJOR_VERSION 142
#define LOG_MINOR_VERSION 1

void defaultStream::init() {
  _inited = true;
  if (LogVMOutput || LogCompilation) {
    init_log();
  }
}

static const char* make_log_name(const char* log_name, const char* force_directory, char* buf) {
  const char* basename = log_name;
  char file_sep = os::file_separator()[0];
  const char* cp;
  for (cp = log_name; *cp != '\0'; cp++) {
    if (*cp == '/' || *cp == file_sep) {
      basename = cp+1;
    }
  }
  const char* nametail = log_name;

  strcpy(buf, "");
  if (force_directory != NULL) {
    strcat(buf, force_directory);
    strcat(buf, os::file_separator());
    nametail = basename;       // completely skip directory prefix
  }

  const char* star = strchr(basename, '*');
  int star_pos = (star == NULL) ? -1 : (star - nametail);

  if (star_pos >= 0) {
    // convert foo*bar.log to foo123bar.log
    int buf_pos = (int) strlen(buf);
    strncpy(&buf[buf_pos], nametail, star_pos);
    sprintf(&buf[buf_pos + star_pos], "%u", os::current_process_id());
    nametail += star_pos + 1;  // skip prefix and star
  }

  strcat(buf, nametail);      // append rest of name, or all of name
  return buf;
}

void defaultStream::init_log() {
  // %%% Need a MutexLocker?
  const char* log_name = strlen(LogFile) > 0 ? LogFile : "hotspot.log";
  char buf[BUFLEN*2];
  const char* try_name = make_log_name(log_name, NULL, buf);
  fileStream* file = new(ResourceObj::C_HEAP) fileStream(try_name);
  if (!file->is_open()) {
    // Try again to open the file.
    char warnbuf[BUFLEN*2];
    sprintf(warnbuf, "Warning:  Cannot open log file: %s\n", try_name);
    // Note:  This feature is for maintainer use only.  No need for L10N.
    jio_print(warnbuf);
    try_name = make_log_name("hs_pid*.log", os::get_temp_directory(), buf);
    sprintf(warnbuf, "Warning:  Forcing option -XX:LogFile=%s\n", try_name);
    jio_print(warnbuf);
    delete file;
    file = new(ResourceObj::C_HEAP) fileStream(try_name);
  }
  if (file->is_open()) {
    _log_file = file;
    xmlStream* xs = new(ResourceObj::C_HEAP) xmlStream(file);
    _outer_xmlStream = xs;
    if (this == tty)  xtty = xs;
    // Write XML header.
    xs->print_cr("<?xml version='1.0' encoding='UTF-8'?>");
    // (For now, don't bother to issue a DTD for this private format.)
    xs->head("hotspot_log version='%d %d'"
             " process='%d' time_ms='"INT64_FORMAT"'",
             LOG_MAJOR_VERSION, LOG_MINOR_VERSION,
             os::current_process_id(), os::javaTimeMillis());
    // Write VM version header immediately.
    xs->head("vm_version");
    xs->head("name"); xs->text("%s", VM_Version::vm_name()); xs->cr();
    xs->tail("name");
    xs->head("release"); xs->text("%s", VM_Version::vm_release()); xs->cr();
    xs->tail("release");
    xs->head("info"); xs->text("%s", VM_Version::internal_vm_info_string()); xs->cr();
    xs->tail("info");
    xs->tail("vm_version");
    // tty output per se is grouped under the <tty>...</tty> element.
    xs->head("tty");
    // All further non-markup text gets copied to the tty:
    xs->_text = this;  // requires friend declaration!
  } else {
    delete(file);
    // and leave xtty as NULL
    LogVMOutput = false;
    DisplayVMOutput = true;
    LogCompilation = false;
  }
}

void defaultStream::finish_log() {
  xmlStream* xs = _outer_xmlStream;
  xs->done("tty");

#ifndef CORE
  // Other log forks are appended here, at the End of Time:
  CompileLog::finish_log(xs->out());  // write compile logging, if any, now
#endif //CORE

  xs->done("hotspot_log");
  xs->flush();

  fileStream* file = _log_file;
  _log_file = NULL;
  _outer_xmlStream = NULL;
  
  file->flush();
  delete file;
}

intptr_t defaultStream::hold(intptr_t writer_id) {
  bool has_log = has_log_file();  // check before locking
  if (writer_id == NO_WRITER ||                   // impossible, but who knows?
      !SerializeVMOutput ||                       // developer hook
      is_error_reported() ||                      // VM already unhealthy
      (SafepointSynchronize::is_synchronizing()   // safepoint == global lock
       && Thread::current()->is_VM_thread()) ||   //   (for VM only)
      tty_lock == NULL                            // bootstrap problem
      ) {
    // do not attempt to lock unless we know the thread and the VM is healthy
    return NO_WRITER;
  }
  if (_writer == writer_id) {
    // already held, no need to re-grab the lock
    return NO_WRITER;
  }
  tty_lock->lock_without_safepoint_check();
  // got the lock
  if (writer_id != _last_writer) {
    if (has_log) {
      _log_file->bol();
      // output a hint where this output is coming from:
      _log_file->print_cr("<writer thread='%d'/>", (int) writer_id);
    }
    _last_writer = writer_id;
  }
  _writer = writer_id;
  return writer_id;
}

void defaultStream::release(intptr_t holder) {
  if (holder == NO_WRITER) {
    // nothing to release:  either a recursive lock, or we scribbled (too bad)
    return;
  }
  if (_writer != holder) {
    return;  // already unlocked, perhaps via break_tty_lock_for_safepoint
  }
  _writer = NO_WRITER;
  tty_lock->unlock();
}


// Yuck:  jio_print does not accept char*/len.
static void call_jio_print(const char* s, size_t len) {
  char buffer[BUFLEN+100];
  if (len > sizeof(buffer)-1) {
    warning("increase BUFLEN in ostream.cpp -- output truncated");
    len = sizeof(buffer)-1;
  }
  strncpy(buffer, s, len);
  buffer[len] = '\0';
  jio_print(s);
}


void defaultStream::write(const char* s, size_t len) {
  intptr_t thread_id = (intptr_t) os::current_thread_id();
  intptr_t holder = hold(thread_id);

  if (DisplayVMOutput &&
      (_outer_xmlStream == NULL || !_outer_xmlStream->inside_attrs())) {
    // print to output stream. It can be redirected by a vfprintf hook
    if (s[len] == '\0') {
      jio_print(s);
    } else {
      call_jio_print(s, len);
    }
  }

  // print to log file
  if (has_log_file()) {
    int nl0 = _newlines;
    xmlTextStream::write(s, len);
    // flush the log file too, if there were any newlines
    if (nl0 != _newlines){
      _log_file->flush();
    }
  } else {
    update_position(s, len);
  }

  release(holder);
}

intptr_t ttyLocker::hold_tty() {
  if (defaultStream::instance == NULL)  return defaultStream::NO_WRITER;
  intptr_t thread_id = (intptr_t) os::current_thread_id();
  return defaultStream::instance->hold(thread_id);
}

void ttyLocker::release_tty(intptr_t holder) {
  if (holder == defaultStream::NO_WRITER)  return;
  defaultStream::instance->release(holder);
}

void ttyLocker::break_tty_lock_for_safepoint(intptr_t holder) {
  if (defaultStream::instance != NULL &&
      defaultStream::instance->writer() == holder) {
    if (xtty != NULL) {
      xtty->print_cr("<!-- safepoint while printing -->");
    }
    if (UseNewCode)
    defaultStream::instance->release(holder);
  }
  // (else there was no lock to break)
}

void ostream_init() {
  if (defaultStream::instance == NULL) {
    defaultStream::instance = new(ResourceObj::C_HEAP) defaultStream();
    tty = defaultStream::instance;
  }
}

void ostream_init_log() {

  // For -Xloggc:<file> option - called in runtime/thread.cpp
  // Note : this must be called AFTER ostream_init()

  gclog_or_tty = tty; // default to tty
  if (Arguments::gc_log_filename() != NULL) {
    fileStream * gclog = new(ResourceObj::C_HEAP)
                           fileStream(Arguments::gc_log_filename());
    if (gclog->is_open()) {
      gclog_or_tty = gclog;
    }
  }
}

void ostream_exit() {
  static bool ostream_exit_called = false;
  if (ostream_exit_called)  return;
  ostream_exit_called = true;
  if (gclog_or_tty != tty)
    delete gclog_or_tty;
  if (tty != defaultStream::instance)
    delete tty;
  if (defaultStream::instance != NULL)
    delete defaultStream::instance;
  tty = NULL;
  xtty = NULL;
  gclog_or_tty = NULL;
  defaultStream::instance = NULL;
}
