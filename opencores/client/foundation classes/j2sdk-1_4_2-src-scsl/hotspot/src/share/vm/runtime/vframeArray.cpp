#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vframeArray.cpp	1.115 03/01/23 12:26:38 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vframeArray.cpp.incl"


vframeArray* vframeArray::allocate(JavaThread* thread, int frame_size, GrowableArray<compiledVFrame*>* chunk,
                                   RegisterMap *reg_map, frame sender, frame caller, frame self, frame adapter_caller) {

  // Compute the size of the storage we need
  int size = vframeArray::array_length_for(chunk);

  // Allocate the vframeArray
  vframeArray * result = (vframeArray*) AllocateHeap(size * sizeof(intptr_t) + sizeof(vframeArray),"vframeArray::allocate");
  result->_length = size;
  result->set_adapter_caller(adapter_caller);
  result->set_sender(sender);
  result->set_old_adapter(NULL);
  result->set_caller(caller);
  result->set_original(self);
#ifdef COMPILER1
  result->set_unroll_block(NULL); // initialize it (may be needed for C2 too)
  result->fill_in(thread, frame_size, chunk, reg_map, false /*needs no adapter*/);
#endif
#ifdef COMPILER2
  // Initial value that may be replaced with a C2I adapter.
  result->set_new_adapter(NULL);
  result->set_adjust_adapter_caller(false);
  result->fill_in(thread, frame_size, chunk, reg_map, adapter_caller.sp() == NULL);
#endif
  return result;
}


javaVFrame* vframeArray::vframe_at(int index) {
  return new deoptimizedVFrame(this, index);
}


int vframeArray::fill_in_locals(int index, compiledVFrame* vf) {
  StackValueCollection *locs = vf->locals();
  for(int i = 0; i < locs->size(); i++) {
    StackValue *value = locs->at(i);
    switch(value->type()) {      
      case T_OBJECT:
        obj_at_put(index + (i*2), value->get_obj()()); 
        int_at_put(index + (i*2) + 1, 1);
        break;
      case T_INT:
        int_at_put(index + (i*2), value->get_int()); 
        int_at_put(index + (i*2) + 1, 0);
        break;
      case T_CONFLICT:
        int_at_put(index + (i*2), 0);
        int_at_put(index + (i*2) + 1, 0);
        break;
      default:
        ShouldNotReachHere();
    }
  }
  return locs->size() * 2;
}


int vframeArray::fill_in_expression_stack(int index, compiledVFrame* vf) {
  StackValueCollection *exprs = vf->expressions();  
  for(int i = 0; i < exprs->size(); i++) {
    StackValue *value = exprs->at(i);
    switch(value->type()) {
      case T_OBJECT: 
        obj_at_put(index + (i*2), value->get_obj()());
        int_at_put(index + (i*2) + 1, 1);
        break;
      case T_INT:
        int_at_put(index + (i*2), value->get_int());
        int_at_put(index + (i*2) + 1, 0);
        break;
      default:
        ShouldNotReachHere();
    }
  }
  return exprs->size() * 2;
}


void vframeArray::migrate_monitors_off_stack() {
  for (int index = first_index(); index < length(); index = sender_index_for(index)) {
     MonitorArray* array = monitor_array_for(index);
     if (array != NULL) {
       array->migrate_locks_off_stack(owner_thread());
     }
  }
}

class MonitorArrayElement VALUE_OBJ_CLASS_SPEC {
 public:
  oop        owner;
  BasicLock* lock;
};

MonitorArray::MonitorArray(int length, JavaThread* owner) {
  _length        = length;
  _monitors      = NEW_C_HEAP_ARRAY(MonitorArrayElement, _length);
  _owner         = owner;
  _monitor_chunk = NULL;
  _next          = NULL;
  owner->add_monitor_array(this);
}

MonitorArray::~MonitorArray() {
  assert(_monitors == NULL, "must be deallocated by now");;
  assert(!has_locks_on_stack(), "must have locks off stack");
  delete monitor_chunk();
}

MonitorArrayElement* MonitorArray::at(int index) {
  assert(has_locks_on_stack(), "must have locks on stack");
  assert(index >= 0 &&  index < _length, "out of bounds check");
  return &_monitors[index];
}

void MonitorArray::at_put(int index, oop owner, BasicLock* lock) {
  assert(has_locks_on_stack(), "must have locks on stack");
  at(index)->owner = owner;
  at(index)->lock  = lock;
}

void MonitorArray::oops_do(OopClosure* f) {
  if (!has_locks_on_stack()) return; // handled by MonitorChunk registerd in thread
  for (int index = 0; index < length(); index++) {
    f->do_oop(&at(index)->owner);
  }
}

void MonitorArray::monitors_do(void f(oop, BasicLock*)) {
  for (int index = 0; index < length(); index++) {
    f(at(index)->owner, at(index)->lock);
  }
}

void MonitorArray::migrate_locks_off_stack(JavaThread* owner) {
  // Allocate monitor chunk
  MonitorChunk* chunk = new MonitorChunk(length());
  owner->add_monitor_chunk(chunk);

  // Migrate the BasicLocks from the stack to the monitor chunk
  for (int index = 0; index < length(); index++) {
    MonitorArrayElement* monitor = at(index);
    assert(monitor->owner == NULL || !monitor->owner->is_unlocked(), "object must be null or locked");
    BasicObjectLock* dest = chunk->at(index);
    dest->set_obj(monitor->owner);
    monitor->lock->move_to(monitor->owner, dest->lock());
    monitor->lock = dest->lock();
  }

  owner->remove_monitor_array(this);
  FREE_C_HEAP_ARRAY(MonitorArrayElement, _monitors);
  _monitors = NULL;

  _monitor_chunk = chunk;
}

GrowableArray<MonitorInfo*>* MonitorArray::as_monitor_list() {
  GrowableArray<MonitorInfo*>* result = new GrowableArray<MonitorInfo*>(length());
  if (has_locks_on_stack()) {
    for (int index = 0; index < length(); index++) {
      result->push(new MonitorInfo(at(index)->owner, at(index)->lock));
    }
  } else {
    for (int index = 0; index < length(); index++) {
      BasicObjectLock* ol = _monitor_chunk->at(index);
      result->push(new MonitorInfo(ol->obj(), ol->lock()));
    }
  }
  return result;
}

MonitorArray* vframeArray::register_monitors(compiledVFrame* vf) {
  GrowableArray<MonitorInfo*>* list = vf->monitors();
  if (list->is_empty()) return NULL;

  MonitorArray* result = new MonitorArray(list->length(), owner_thread());
  for (int index = 0; index < list->length(); index++) {
    MonitorInfo* monitor = list->at(index);
    // when the thread is blocked in wait, some objects may be unlocked temporarily,
    // therefore we must loosen the assert
    assert(vf->thread()->thread_state() == _thread_blocked || monitor->owner() == NULL || !monitor->owner()->is_unlocked(), "object must be null or locked or blocked in wait");
    result->at(index)->owner = monitor->owner();
    result->at(index)->lock  = monitor->lock();
  }
  return result;
}

int vframeArray::fill_in_vframe(int index, compiledVFrame* vf) {
  // Static part of activation
  obj_at_put(index + method_offset,       vf->method());
  int_at_put(index + bci_offset,          vf->raw_bci());
  int_at_put(index + monitor_list_offset, (intptr_t) register_monitors(vf));

  // Locals
  int locals_size = fill_in_locals(index + vframe_fixed_size, vf);
  int_at_put(index + locals_size_offset, locals_size);

  // Expression stack
  int expression_stack_size = fill_in_expression_stack(index + vframe_fixed_size + locals_size, vf);
  int_at_put(index + expression_stack_size_offset, expression_stack_size);

  // Sender index
  int sender_index = index + vframe_fixed_size + locals_size + expression_stack_size;
  int_at_put(index + sender_index_offset, sender_index);
  return sender_index;
}

void vframeArray::fill_in(JavaThread* thread, int frame_size, GrowableArray<compiledVFrame*>* chunk, const RegisterMap *reg_map, bool needs_adapter) {
  // Set owner first, it is used when adding monitor chunks
  _owner_thread = thread;
  _frame_size = frame_size;
  int index = first_index();
  for(int i = 0; i < chunk->length(); i++) {
    index = fill_in_vframe(index, chunk->at(i));
  }
  assert(index == length(), "checking vframe array length");

  // Copy registers for callee-saved registers
  if (reg_map != NULL) {
    for(int i = 0; i < RegisterMap::reg_count; i++) {
      jint* src = (jint*) reg_map->location(VMReg::Name(i));
      jint* dst = (jint*) register_location(i);      
      *dst = src != NULL ? *src : NULL;
    }
  }

#ifdef COMPILER2
  // If the root vframe is preceded by an I2C adapter, that adapter will be removed.
  // Otherwise, a C2I adapter will be needed.  This avoids paired I2C/C2I adapters.
  if (needs_adapter) {
    assert(adapter_caller().sp() == NULL, "should not be set");
    javaVFrame *jvf = chunk->at(chunk->length() - 1);
    methodOop callee = jvf->method();
    set_new_adapter(C2IAdapterGenerator::adapter_for(callee));
  }
#endif
}

void vframeArray::unpack_to_stack(frame &unpack_frame, int exec_mode) {
  // stack picture
  //   unpack_frame
  //   [new interpreter frames ] (frames are skeletal but walkable)
  //   caller_frame (may be a c2i created during deopt)
  //
  //  This routine fills in the missing data for the skeletal interpreter frames
  //  in the above picture.

  // Collect interpreter frames to unpack
  GrowableArray<deoptimizedVFrame*>* chunk = new GrowableArray<deoptimizedVFrame*>(10);
  RegisterMap map(JavaThread::current(), false);
  // Get the youngest frame we will unpack (last to be unpacked)
  frame me = unpack_frame.sender(&map);
  int index;
  for (index = first_index(); index < length(); index = sender_index_for(index)) {
    deoptimizedVFrame* vf = new deoptimizedVFrame(&me, &map, this, index);
    chunk->push(deoptimizedVFrame::cast(vf));
    // Get the caller frame (possibly skeletal)
    me = me.sender(&map);
  }

  frame caller_frame = me;

  // The unpacking code now will make certain that if the caller of the current
  // deoptimized frame was deoptimized since the current frame was deoptimized
  // that the return address in the initial frame we created for the current
  // deoptimizee will have the value that returns it to the deoptimzation
  // handler (blob). So we no longer have to do anything special here.

#ifdef COMPILER2
  if (new_adapter()) {
    assert(adapter_caller().sp() == NULL, "should not be set");
    // Initialize the C2I adapter frame; stuff its registers, especially the frame pointer
    new_adapter()->unpack_c2i_adapter(unpack_frame, caller_frame, this);

  }
#endif


  // Do the unpacking of interpreter frames; the frame at index 0 represents the top activation, so it has no callee

  // Unpack the frames from the oldest (length -1) to the youngest (0)

  for (index = chunk->length() - 1; index >= 0 ; index--) {
    int callee_parameters = index == 0 ? 0 : chunk->at(index-1)->method()->size_of_parameters();
    int callee_locals     = index == 0 ? 0 : chunk->at(index-1)->method()->max_locals();
    me = chunk->at(index)->fr();
    chunk->at(index)->unpack_vframe_on_stack(callee_parameters, callee_locals, &caller_frame, &me, index == 0, exec_mode);
    caller_frame = me;
  }

#ifdef CC_INTERP
#ifndef IA64
  ShouldNotReachHere();
#endif
#endif

  // Update return address and other bits in top (deopt/uncommontrap) frame
  // so that when it returns it will return to the interpreter.
  // Note: the writing of the pc has already happened now with the
  // changes to unpack_vframe_on_stack and the walkable frames.
  // This call also updates ebp on x86 which doesn't look to be needed
  // anymore and if it does it need cleanup anyway NEEDS_CLEANUP.
  //
  unpack_frame.set_sender_pc_for_unpack(caller_frame, exec_mode);

  deallocate_monitor_chunks();
}

void vframeArray::validate_index(int index) const {
  // Validate the index as start of deoptimized vframe
}

int vframeArray::sender_index_for(int index) const {
  validate_index(index);
  return int_at(index + sender_index_offset);
}

methodOop vframeArray::method_for(int index) const {
  validate_index(index);
  methodOop result = methodOop(obj_at(index + method_offset));
  assert(result->is_method(), "just ckecking");
  return result;
}

int vframeArray::bci_for(int index) const {
  validate_index(index);
  return int_at(index + bci_offset);
}

void vframeArray::set_bci_for(int index, int rawbci) {
  validate_index(index);
  int_at_put(index + bci_offset, rawbci);
}

MonitorArray* vframeArray::monitor_array_for(int index) const {
  validate_index(index);
  return (MonitorArray*) int_at(index + monitor_list_offset);
}

int vframeArray::locals_size_for(int index) const {
  return int_at(index + locals_size_offset) / 2;
}

int vframeArray::expression_stack_size_for(int index) const  {
  return int_at(index + expression_stack_size_offset) / 2 ;
}

int vframeArray::locals_index_for(int index, int offset) const {
  return index + vframe_fixed_size + (offset*2);
}

int vframeArray::expression_stack_index_for(int index, int offset) const {
  return index + vframe_fixed_size + (locals_size_for(index) * 2) + (offset * 2);
}


bool vframeArray::locals_is_obj_for(int index, int offset) const {
  return int_at(locals_index_for(index, offset) + 1) == 1;
}

oop vframeArray::locals_obj_for(int index, int offset) const {
  return obj_at(locals_index_for(index, offset));
}

void vframeArray::set_locals_obj_for(int index, int offset, oop value) {
  obj_at_put(locals_index_for(index, offset), value);
}

oop* vframeArray::locals_obj_addr_for(int index, int offset) const {
  return obj_at_addr(locals_index_for(index, offset));
}

intptr_t vframeArray::locals_int_for(int index, int offset) const {
  return int_at(locals_index_for(index, offset));
}

void vframeArray::set_locals_int_for(int index, int offset, intptr_t value) {
  int_at_put(locals_index_for(index, offset), value);
}

bool vframeArray::expression_stack_is_obj_for(int index, int offset) const {
  return int_at(expression_stack_index_for(index, offset) + 1) == 1;
}

oop vframeArray::expression_stack_obj_for(int index, int offset) const {
  return obj_at(expression_stack_index_for(index, offset));
}

oop* vframeArray::expression_stack_obj_addr_for(int index, int offset) const {
  return obj_at_addr(expression_stack_index_for(index, offset));
}


intptr_t vframeArray::expression_stack_int_for(int index, int offset) const {
  return int_at(expression_stack_index_for(index, offset));
}


int vframeArray::vframe_store_size(compiledVFrame* vf) {
  int result = 0;
  result += vframe_fixed_size;
  result += vf->method()->max_locals()  * 2;
  result += vf->expressions()->size() * 2;
  return result;
}

int vframeArray::array_length_for(GrowableArray<compiledVFrame*>* chunk) {
  int result = prologue_size;
  for(int index = 0; index < chunk->length(); index++) {
    result += vframe_store_size(chunk->at(index));
  }
  return result;
}

void vframeArray::deallocate_monitor_chunks() {
  for (int index = first_index(); index < length(); index = sender_index_for(index)) {
     MonitorArray* array = monitor_array_for(index);
     if (array != NULL) {
       assert(!array->has_locks_on_stack(), "must have locks off stack");
       owner_thread()->remove_monitor_chunk(array->monitor_chunk());
       delete array;
     }
  }
}

#ifndef PRODUCT

bool vframeArray::structural_compare(JavaThread* thread, GrowableArray<compiledVFrame*>* chunk) {
  if (owner_thread() != thread) return false;
  int index = 0;
  // Compare only within vframe array.
  for (deoptimizedVFrame* vf = deoptimizedVFrame::cast(vframe_at(first_index())); vf; vf = vf->deoptimized_sender_or_null()) {
    if (index >= chunk->length() || !vf->structural_compare(chunk->at(index))) return false;
    index++;
  }
  if (index != chunk->length()) return false;
  return true;
}

#endif

address vframeArray::register_location(int i) const {
  assert(0 <= i && i < RegisterMap::reg_count, "index out of bounds");
  return (address) int_at_addr(first_register_entry_offset + (i*2));
}

void vframeArray::update_register_map(RegisterMap *map) {
  for(int i = 0; i < RegisterMap::reg_count; i++) {
    map->set_location(VMReg::Name(i), register_location(i));
  }
  map->set_include_argument_oops(false);
}

void vframeArray::oops_do(OopClosure* f) {
  // Handle the frames
  for (int index = first_index(); index < length(); index = sender_index_for(index)) {
    int locals_size = locals_size_for(index);
    int stack_size  = expression_stack_size_for(index);
    int i;
    
    // Locals
    for (i = 0; i < locals_size; i++) {
      if (locals_is_obj_for(index, i)) {
        f->do_oop(locals_obj_addr_for(index, i));
      }
    }
    
    // Expression stack
    for (i = 0; i < stack_size; i++) {
      if (expression_stack_is_obj_for(index, i)) {
        f->do_oop(expression_stack_obj_addr_for(index, i));
      }
    }
    
    // Static part of frame
    f->do_oop(obj_at_addr(index + vframeArray::method_offset));
  }
}

#ifndef PRODUCT

// Printing

// Note: we cannot have print_on as const, as we allocate inside the method
void vframeArray::print_on_2(outputStream* st)  {
  st->print_cr(" - sp: " INTPTR_FORMAT, sp());
  st->print(" - thread: ");
  owner_thread()->print();
  st->print_cr(" - frame size: %d", frame_size());
  for (deoptimizedVFrame* vf = new deoptimizedVFrame(this, vframeArray::first_index()); vf; vf = vf->deoptimized_sender_or_null()) {
    vf->print();
  }
}


void vframeArray::print_value_on(outputStream* st) const {
  st->print_cr("vframeArray [%d] ", length());
}


#endif
