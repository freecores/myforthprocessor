#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ostream.hpp	1.25 03/01/23 12:28:43 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Output streams for printing
class outputStream : public ResourceObj {
 protected:
   int _indentation; // current indentation
   int _width;       // width of the page
   int _position;    // position on the current line
   int _newlines;    // number of '\n' output so far
   julong _precount; // number of chars output, less _position
   TimeStamp _stamp; // for time stamps

   void update_position(const char* s, size_t len);
   static const char* do_vsnprintf(char* buffer, size_t buflen,
                                   const char* format, va_list ap,
                                   bool add_cr,
                                   size_t& result_len);

 public:
   // creation
   outputStream(int width = 80);
   outputStream(int width, bool has_time_stamps);

   // indentation
   void indent();
   void inc() { _indentation++; };
   void dec() { _indentation--; };
   int  indentation() const    { return _indentation; }
   void set_indentation(int i) { _indentation = i;    }
   void fill_to(int col);

   // sizing
   int width()    const { return _width;    }
   int position() const { return _position; }
   int newlines() const { return _newlines; }
   julong count() const { return _precount + _position; }
   void set_count(julong count) { _precount = count - _position; }
   void set_position(int pos)   { _position = pos; }

   // printing
   void print(const char* format, ...);
   void print_cr(const char* format, ...);
   void vprint(const char *format, va_list argptr);
   void vprint_cr(const char* format, va_list argptr);
   void print_raw(const char* str)          { write(str, strlen(str)); }
   void print_raw(const char* str, int len) { write(str,         len); }
   void put(char ch);
   void sp();
   void cr();
   void bol() { if (_position > 0)  cr(); }

   // Time stamp
   TimeStamp& time_stamp() { return _stamp; }
   void stamp();

   // portable printing of 64 bit integers
   void print_jlong(jlong value);
   void print_julong(julong value);

   // flushing
   virtual void flush() {}
   virtual void write(const char* str, size_t len) = 0;
   virtual ~outputStream() {}  // close properly on deletion

   void dec_cr() { dec(); cr(); }
   void inc_cr() { inc(); cr(); }
};

// standard output
				// ANSI C++ name collision
extern outputStream* tty;	    // tty output
extern outputStream* gclog_or_tty;  // stream for gc log if -Xloggc:<f>, or tty

// advisory locking for the shared tty stream:
class ttyLocker: StackObj {
 private:
  intptr_t _holder;

 public:
  static intptr_t hold_tty();                 // returns a "holder" token
  static void  release_tty(intptr_t holder);  // must witness same token
  static void  break_tty_lock_for_safepoint(intptr_t holder);

  ttyLocker()  { _holder = hold_tty(); }
  ~ttyLocker() { release_tty(_holder); }
};

// for writing to strings; buffer will expand automatically
class stringStream : public outputStream {
 protected:
  char*  buffer;
  size_t buffer_pos;
  size_t buffer_length;
  bool   buffer_fixed;
 public:
  stringStream(size_t initial_bufsize = 256);
  stringStream(char* fixed_buffer, size_t fixed_buffer_size);
  ~stringStream();
  virtual void write(const char* c, size_t len);
  size_t fixed_buffer_size() const { return buffer_fixed ? buffer_length : -1; }
  size_t      size() { return buffer_pos; }
  const char* base() { return buffer; }
  void  reset() { buffer_pos = 0; _precount = 0; _position = 0; }
  char* as_string();
};

class fileStream : public outputStream {
 protected:
  FILE* _file;
 public:
  fileStream(const char* file_name);
  fileStream(FILE* file) { _file = file; }
  ~fileStream();
  int is_open() const { return _file != NULL; }
  virtual void write(const char* c, size_t len);
  void flush();
};

void ostream_init();
void ostream_init_log();
void ostream_exit();

