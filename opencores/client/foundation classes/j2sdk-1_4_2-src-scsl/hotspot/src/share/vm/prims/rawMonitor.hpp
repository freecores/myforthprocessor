#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)rawMonitor.hpp	1.4 03/01/23 12:21:04 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

//
//
// class RawMonitor
// Used by all JVMPI RawMonitor methods: 
//   (CreateRawMonitor, EnterRawMonitor, etc.)
//
// Wrapper for ObjectMonitor class that saves the Monitor's name
// and links thread's owned raw monitors

class RawMonitor: public ObjectMonitor {
private:
  int           _magic;
  char *        _name;
  // maintaining list of locked raw monitors
  RawMonitor* _rmnext;    
  RawMonitor* _rmprev;

public:
  RawMonitor(const char *name, const int magic);
  ~RawMonitor();                             
  int       magic()                         { return _magic;  }
  void      print(outputStream& out)        { out.print(_name); }
  RawMonitor* next_raw() const              { return _rmnext; }
  const char *name() const                  { return _name; }
  void add_to_locked_list();
  void remove_from_locked_list();
  int       raw_destroy();
};


static int PROF_RM_MAGIC = (int)(('P' << 24) | ('I' << 16) | ('R' << 8) | 'M');
#define PROF_RM_CHECK(m)                                 \
    ((m) != NULL && (m)->magic() == PROF_RM_MAGIC) 
