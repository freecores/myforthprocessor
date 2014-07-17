#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)compileLog.cpp	1.7 03/02/28 16:49:32 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_compileLog.cpp.incl"

CompileLog* CompileLog::_first = NULL;

// ------------------------------------------------------------------
// CompileLog::CompileLog
CompileLog::CompileLog(const char* file, FILE* fp, int thread_id)
  : _context(_context_buffer, sizeof(_context_buffer))
{
  initialize(new(ResourceObj::C_HEAP) fileStream(fp));
  _file = file;
  _file_end = 0;
  _thread_id = thread_id;

  _identities_limit = 0;
  _identities_capacity = 400;
  _identities = NEW_C_HEAP_ARRAY(char, _identities_capacity);

  // link into the global list
  { MutexLocker locker(CompileTaskAlloc_lock);
    _next = _first;
    _first = this;
  }
}

CompileLog::~CompileLog() {
  delete _out;
  _out = NULL;
  FREE_C_HEAP_ARRAY(char, _identities);
}


// Advance kind up to a null or space, return this tail.
// Make sure kind is null-terminated, not space-terminated.
// Use the buffer if necessary.
static const char* split_attrs(const char* &kind, char* buffer) {
  const char* attrs = strchr(kind, ' ');
  // Tease apart the first word from the rest:
  if (attrs == NULL) {
    return "";  // no attrs, no split
  } else if (kind == buffer) {
    ((char*) attrs)[-1] = 0;
    return attrs;
  } else {
    // park it in the buffer, so we can put a null on the end
    assert(!(kind >= buffer && kind < buffer+100), "not obviously in buffer")
    int klen = attrs - kind;
    strncpy(buffer, kind, klen);
    buffer[klen] = 0;
    kind = buffer;  // return by reference
    return attrs;
  }
}

// see_tag, pop_tag:  Override the default do-nothing methods on xmlStream.
// These methods provide a hook for managing the the extra context markup.
void CompileLog::see_tag(const char* tag, bool push) {
  if (_context.size() > 0 && _out != NULL) {
    _out->write(_context.base(), _context.size());
    _context.reset();
  }
  xmlStream::see_tag(tag, push);
}
void CompileLog::pop_tag(const char* tag) {
  _context.reset();  // toss any context info.
  xmlStream::pop_tag(tag);
}


// ------------------------------------------------------------------
// CompileLog::identify
int CompileLog::identify(ciObject* obj) {
  if (obj == NULL)  return 0;
  int id = obj->ident();
  if (id < 0)  return id;
  // If it has already been identified, just return the id.
  if (id < _identities_limit && _identities[id] != 0)  return id;
  // Lengthen the array, if necessary.
  if (id >= _identities_capacity) {
    int new_cap = _identities_capacity * 2;
    if (new_cap <= id)  new_cap = id + 100;
    _identities = REALLOC_C_HEAP_ARRAY(char, _identities, new_cap);
    _identities_capacity = new_cap;
  }
  while (id >= _identities_limit) {
    _identities[_identities_limit++] = 0;
  }
  assert(id < _identities_limit, "oob");
  // Mark this id as processed.
  // (Be sure to do this before any recursive calls to identify.)
  _identities[id] = 1;  // mark 

  // Now, print the object's identity once, in detail.
  if (obj->is_klass()) {
    ciKlass* klass = obj->as_klass();
    begin_elem("klass id='%d'", id);
    name(klass->name());
    if (!klass->is_loaded()) {
      print(" unloaded='1'");
    } else {
      print(" flags='%d'", klass->modifier_flags());
    }
    end_elem();
  } else if (obj->is_method()) {
    ciMethod* method = obj->as_method();
    ciSignature* sig = method->signature();
    // Pre-identify items that we will need!
    identify(sig->return_type());
    for (int i = 0; i < sig->count(); i++) {
      identify(sig->type_at(i));
    }
    begin_elem("method id='%d' holder='%d'",
               id, identify(method->holder()));
    name(method->name());
    print(" return='%d'", identify(sig->return_type()));
    if (sig->count() > 0) {
      print(" arguments='");
      for (int i = 0; i < sig->count(); i++) {
        print((i == 0) ? "%d" : " %d", identify(sig->type_at(i)));
      }
      print("'");
    }
    if (!method->is_loaded()) {
      print(" unloaded='1'");
    } else {
      print(" flags='%d'", (jchar) method->flags().as_int());
      // output a few metrics
      print(" bytes='%d'", method->code_size());
      int nmsize = method->instructions_size();
      if (nmsize != 0)  print(" nmsize='%d'", nmsize);
      //print(" count='%d'", method->invocation_count());
      //int bec = method->backedge_count();
      //if (bec != 0)  print(" backedge_count='%d'", bec);
#ifdef COMPILER2
      print(" iicount='%d'", method->interpreter_invocation_count());
#endif
    }
    end_elem();
  } else if (obj->is_symbol()) {
    begin_elem("symbol id='%d'", id);
    name(obj->as_symbol());
    end_elem();
  } else if (obj->is_null_object()) {
    elem("null_object id='%d'", id);
  } else if (obj->is_type()) {
    BasicType type = obj->as_type()->basic_type();
    elem("type id='%d' name='%s'", id, type2name(type));
  } else {
    // Should not happen.
    elem("unknown id='%d'", id);
  }
  return id;
}

void CompileLog::name(ciSymbol* name) {
  if (name == NULL)  return;
  print(" name='");
  name->print_symbol_on(text());  // handles quoting conventions
  print("'");
}


// ------------------------------------------------------------------
// CompileLog::clear_identities
// Forget which identities have been printed.
void CompileLog::clear_identities() {
  _identities_limit = 0;
}

// Call when exiting.
void CompileLog::finish_log(outputStream* file) {
  static bool called_exit = false;
  if (called_exit)  return;
  called_exit = true;

  char buf[4*K];
  for (CompileLog* log = _first; log != NULL; log = log->_next) {
    log->flush();
    const char* partial_file = log->file();
    FILE* partial = fopen(partial_file, "rt");
    if (partial != NULL) {
      file->print_cr("<compilation_log thread='%d'>", log->thread_id());
      size_t nr; // number read into buf from partial log
      // Copy data up to the end of the last <event> element:
      julong to_read = log->_file_end;
      while (to_read > 0) {
        if (to_read < (julong)sizeof(buf))
              nr = (size_t)to_read;
        else  nr = sizeof(buf);
        nr = fread(buf, 1, nr, partial);
        if (nr <= 0)  break;
        to_read -= (julong)nr;
        file->write(buf, nr);
      }
      // Copy any remaining data inside a quote:
      bool saw_slop = false;
      int end_cdata = 0;  // state machine [0..2] watching for too many "]]"
      while ((nr = fread(buf, 1, sizeof(buf), partial)) > 0) {
        if (!saw_slop) {
          file->print_cr("<fragment>");
          file->print_cr("<![CDATA[");
          saw_slop = true;
        }
        // The rest of this loop amounts to a simple copy operation:
        // { file->write(buf, nr); }
        // However, it must sometimes output the buffer in parts,
        // in case there is a CDATA quote embedded in the fragment.
        const char* bufp;  // pointer into buf
        size_t nw; // number written in each pass of the following loop:
        for (bufp = buf; nr > 0; nr -= nw, bufp += nw) {
          // Write up to any problematic CDATA delimiter (usually all of nr).
          for (nw = 0; nw < nr; nw++) {
            // First, scan ahead into the buf, checking the state machine.
            switch (bufp[nw]) {
            case ']':
              if (end_cdata < 2)   end_cdata += 1;  // saturating counter
              continue;  // keep scanning
            case '>':
              if (end_cdata == 2)  break;  // found CDATA delimiter!
              // else fall through:
            default:
              end_cdata = 0;
              continue;  // keep scanning
            }
            // If we get here, nw is pointing at a bad '>'.
            // It is very rare for this to happen.
            // However, this code has been tested by introducing
            // CDATA sequences into the compilation log.
            break;
          }
          // Now nw is the number of characters to write, usually == nr.
          file->write(bufp, nw);
          if (nw < nr) {
            // We are about to go around the loop again.
            // But first, disrupt the ]]> by closing and reopening the quote.
            file->print("]]><![CDATA[");
            end_cdata = 0;  // reset state machine
          }
        }
      }
      if (saw_slop) {
        file->print_cr("]]>");
        file->print_cr("</fragment>");
      }
      file->print_cr("</compilation_log>");
      fclose(partial);
      unlink(partial_file);
    }
  }
}

