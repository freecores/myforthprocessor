#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)methodIterator.cpp	1.129 03/01/23 12:01:27 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_methodIterator.cpp.incl"


// Implementation of MethodInterval

void MethodInterval::initialize(methodHandle method, MethodInterval* parent, int begin_bci, int end_bci) {
  _method    = method;
  _parent    = parent;
  _begin_bci = begin_bci;
  _end_bci   = end_bci;
}


MethodInterval::MethodInterval(methodHandle method, MethodInterval* parent) {
  initialize(method, parent, 0, method->code_size());
}


MethodInterval::MethodInterval(methodHandle method, MethodInterval* parent, int begin_bci, int end_bci) {
  initialize(method, parent, begin_bci, end_bci);
}


// Implementation of Abstract-/MethodIntervalFactory

MethodInterval* MethodIntervalFactory::new_MethodInterval(methodHandle method, MethodInterval* parent) {
  return new MethodInterval(method, parent);
}


MethodInterval* MethodIntervalFactory::new_MethodInterval(methodHandle method, MethodInterval* parent, int begin_bci, int end_bci) {
  return new MethodInterval(method, parent, begin_bci, end_bci);
}


//----------------------------------------------------------
// Implementation of MethodClosure

MethodClosure::MethodClosure() {
  _method = methodHandle();
  _bci = -1;
  _paired_instruction = false;
  _exception_thrown = false;
}



//-------------------------------------------------------------------------------------------------------------
// Implementation of MethodIterator

MethodIntervalFactory MethodIterator::_default_factory;
AbstractMethodIntervalFactory* MethodIterator::_factory;


MethodIterator::MethodIterator(methodHandle method, AbstractMethodIntervalFactory* factory) {
  assert(method.not_null() && factory != NULL, "method & factory must exist");
  _factory  = factory;
  _method   = method;
  _interval = factory->new_MethodInterval(method, NULL);
  _stream   = NULL;
}


MethodIterator::MethodIterator(MethodInterval* interval, AbstractMethodIntervalFactory* factory) {
  assert(interval != NULL && factory != NULL, "interval & factory must exist");
  _factory  = factory;
  _method   = interval->method_handle();
  _interval = interval;
  _stream   = NULL;
}


// All transformations

void MethodIterator::push_constant(MethodClosure* blk, int i) {
  constantPoolOop constants = method_handle()->constants();
  constantTag tag = constants->tag_at(i);
  if (tag.is_int())                      { blk->push_int(constants->int_at(i));
  } else if (tag.is_long())              { blk->push_long(constants->long_at(i));
  } else if (tag.is_float())             { blk->push_float(constants->float_at(i));
  } else if (tag.is_double())            { blk->push_double(constants->double_at(i));
  } else if (tag.is_string())            { oop string=constants->resolved_string_at(i); blk->push_object(string);
  } else if (tag.is_unresolved_string()) { blk->push_unresolved_string_at(i);
  } else ShouldNotReachHere();
}

void MethodIterator::identify_code(MethodClosure* blk, Bytecodes::Code code) {
  // Usually, this method merely forwards the call to dispatch.
  // In some subclasses, it behaves differently.
  if (blk->visit_this_bci()) {
    dispatch(blk, code);
  }
};


// Note on inlining: 
//   The index passed to load_local_xxx and similar methods must have
//   the inlining offset added to it. The inlining offset is set
//   before register optimization and code generation (RegAllocClosure
//   and CodeGenClosure) and after all other passes ( see
//   CompilationScope for explanation). Therefore the affected
//   closures must add the offset themselves (instead of generic
//   adding offset in MethodIterator).  Additionally the index
//   transformation may not be a simple addition,but a mapping through
//   a table.

void MethodIterator::dispatch(MethodClosure* blk, Bytecodes::Code code) {
  blk->bytecode_prolog(code);

  if (TraceMethodIterator) {
    tty->print_cr("bcp: " INTPTR_FORMAT "\tbci: %03d : %s", _stream->bcp(), _stream->bci(), Bytecodes::name(code));
  }

  switch(code) {
    // Java specific bytecodes
    case Bytecodes::_bipush	        : blk->push_int(get_byte());	                break;
    case Bytecodes::_sipush	        : blk->push_int(get_short());	                break;
    case Bytecodes::_ldc                : { push_constant(blk, get_index());            break; }

    case Bytecodes::_ldc_w	        :                                               // fall through
    case Bytecodes::_ldc2_w	        : { push_constant(blk, get_big_index());        break; }

    case Bytecodes::_aconst_null	: blk->push_object(NULL);	                break;
    case Bytecodes::_iconst_m1          : blk->push_int(-1);	                        break;
    case Bytecodes::_iconst_0	        : blk->push_int(0);	                        break;
    case Bytecodes::_iconst_1	        : blk->push_int(1);	                        break;
    case Bytecodes::_iconst_2	        : blk->push_int(2);	                        break;
    case Bytecodes::_iconst_3	        : blk->push_int(3);	                        break;
    case Bytecodes::_iconst_4	        : blk->push_int(4);	                        break;
    case Bytecodes::_iconst_5	        : blk->push_int(5);	                        break;
    case Bytecodes::_lconst_0	        : blk->push_long(0);                            break;
    case Bytecodes::_lconst_1	        : blk->push_long(1);                            break;
    case Bytecodes::_fconst_0	        : blk->push_float(0.0F);                        break;
    case Bytecodes::_fconst_1	        : blk->push_float(1.0F);                        break;
    case Bytecodes::_fconst_2	        : blk->push_float(2.0F);                        break;
    case Bytecodes::_dconst_0	        : blk->push_double(0.0);                        break;
    case Bytecodes::_dconst_1	        : blk->push_double(1.0);                        break;
    case Bytecodes::_iload	        : blk->load_local_int(get_index_special());	break;
    case Bytecodes::_iload_0	        : blk->load_local_int(0);	                break;
    case Bytecodes::_iload_1	        : blk->load_local_int(1);                       break;
    case Bytecodes::_iload_2            : blk->load_local_int(2);	                break;
    case Bytecodes::_iload_3	        : blk->load_local_int(3);	                break;
    case Bytecodes::_lload	        : blk->load_local_long(get_index_special());	break;
    case Bytecodes::_lload_0	        : blk->load_local_long(0);	                break;
    case Bytecodes::_lload_1	        : blk->load_local_long(1);	                break;
    case Bytecodes::_lload_2	        : blk->load_local_long(2);	                break;
    case Bytecodes::_lload_3	        : blk->load_local_long(3);	                break;
    case Bytecodes::_fload	        : blk->load_local_float(get_index_special());   break;
    case Bytecodes::_fload_0	        : blk->load_local_float(0);	                break;
    case Bytecodes::_fload_1	        : blk->load_local_float(1);	                break;
    case Bytecodes::_fload_2	        : blk->load_local_float(2);	                break;
    case Bytecodes::_fload_3	        : blk->load_local_float(3);	                break;
    case Bytecodes::_dload	        : blk->load_local_double(get_index_special());  break;
    case Bytecodes::_dload_0	        : blk->load_local_double(0);	                break;
    case Bytecodes::_dload_1	        : blk->load_local_double(1);	                break;
    case Bytecodes::_dload_2	        : blk->load_local_double(2);	                break;
    case Bytecodes::_dload_3	        : blk->load_local_double(3);	                break;
    case Bytecodes::_aload	        : blk->load_local_object(get_index_special());  break;
    case Bytecodes::_aload_0	        : blk->load_local_object(0);	                break;
    case Bytecodes::_aload_1	        : blk->load_local_object(1);	                break;
    case Bytecodes::_aload_2	        : blk->load_local_object(2);	                break;
    case Bytecodes::_aload_3	        : blk->load_local_object(3);	                break;
    case Bytecodes::_istore	        : blk->store_local_int(get_index_special());	break;
    case Bytecodes::_istore_0	        : blk->store_local_int(0);	                break;
    case Bytecodes::_istore_1	        : blk->store_local_int(1);	                break;
    case Bytecodes::_istore_2	        : blk->store_local_int(2);	                break;
    case Bytecodes::_istore_3	        : blk->store_local_int(3);	                break;
    case Bytecodes::_lstore	        : blk->store_local_long(get_index_special());	break;
    case Bytecodes::_lstore_0	        : blk->store_local_long(0);	                break;
    case Bytecodes::_lstore_1	        : blk->store_local_long(1);	                break;
    case Bytecodes::_lstore_2	        : blk->store_local_long(2);	                break;
    case Bytecodes::_lstore_3	        : blk->store_local_long(3);	                break;
    case Bytecodes::_fstore	        : blk->store_local_float(get_index_special());  break;
    case Bytecodes::_fstore_0	        : blk->store_local_float(0);	                break;
    case Bytecodes::_fstore_1	        : blk->store_local_float(1);	                break;
    case Bytecodes::_fstore_2	        : blk->store_local_float(2);	                break;
    case Bytecodes::_fstore_3	        : blk->store_local_float(3);	                break;
    case Bytecodes::_dstore	        : blk->store_local_double(get_index_special()); break;
    case Bytecodes::_dstore_0	        : blk->store_local_double(0);	                break;
    case Bytecodes::_dstore_1	        : blk->store_local_double(1);	                break;
    case Bytecodes::_dstore_2	        : blk->store_local_double(2);	                break;
    case Bytecodes::_dstore_3	        : blk->store_local_double(3);	                break;
    case Bytecodes::_astore	        : blk->store_local_object(get_index_special()); break;
    case Bytecodes::_astore_0	        : blk->store_local_object(0);	                break;
    case Bytecodes::_astore_1	        : blk->store_local_object(1);	                break;
    case Bytecodes::_astore_2	        : blk->store_local_object(2);	                break;
    case Bytecodes::_astore_3	        : blk->store_local_object(3);	                break;
    case Bytecodes::_iinc	        :
      { int index = get_index_special();
        jint offset = _stream->is_wide() ? get_short() : get_byte();    
        blk->increment_local_int(index, offset);
      }
      break;    
    case Bytecodes::_newarray	        : blk->new_basic_array(get_index());	        break;
    case Bytecodes::_anewarray  : 
      { int klass_index = get_big_index();
        blk->new_object_array_by_index(klass_index); 
      }
      break;
    case Bytecodes::_multianewarray	:
      { int klass_index = get_big_index();
        int nof_dims = get_index();
        blk->new_multi_array_by_index(klass_index, nof_dims);
      }
      break;
    case Bytecodes::_arraylength	: blk->array_length();	                        break;
    case Bytecodes::_iaload	        : blk->load_array_int();                        break;
    case Bytecodes::_laload	        : blk->load_array_long();	                break;
    case Bytecodes::_faload	        : blk->load_array_float();	                break;
    case Bytecodes::_daload	        : blk->load_array_double();	                break;
    case Bytecodes::_aaload	        : blk->load_array_object();	                break;
    case Bytecodes::_baload	        : blk->load_array_byte();	                break;
    case Bytecodes::_caload	        : blk->load_array_char();	                break;
    case Bytecodes::_saload	        : blk->load_array_short();	                break;
    case Bytecodes::_iastore	        : blk->store_array_int();	                break;
    case Bytecodes::_lastore	        : blk->store_array_long();	                break;
    case Bytecodes::_fastore	        : blk->store_array_float();	                break;
    case Bytecodes::_dastore	        : blk->store_array_double();	                break;
    case Bytecodes::_aastore	        : blk->store_array_object();	                break;
    case Bytecodes::_bastore	        : blk->store_array_byte();	                break;
    case Bytecodes::_castore	        : blk->store_array_char();	                break;
    case Bytecodes::_sastore	        : blk->store_array_short();	                break;
    case Bytecodes::_nop	        : blk->nop();	                                break;
    case Bytecodes::_pop	        : blk->pop();	                                break;
    case Bytecodes::_pop2	        : blk->pop2();	                                break;
    case Bytecodes::_dup	        : blk->dup();	                                break;
    case Bytecodes::_dup2	        : blk->dup2();	                                break;
    case Bytecodes::_dup_x1	        : blk->dup_x1();                                break;
    case Bytecodes::_dup2_x1	        : blk->dup2_x1();	                        break;
    case Bytecodes::_dup_x2	        : blk->dup_x2();                                break;
    case Bytecodes::_dup2_x2	        : blk->dup2_x2();	                        break;
    case Bytecodes::_swap	        : blk->swap();	                                break;
    case Bytecodes::_iadd	        : blk->arithmetic_int   (MethodClosure::add);   break;
    case Bytecodes::_ladd	        : blk->arithmetic_long  (MethodClosure::add);   break;
    case Bytecodes::_fadd	        : blk->arithmetic_float (MethodClosure::add);   break;
    case Bytecodes::_dadd	        : blk->arithmetic_double(MethodClosure::add);   break;
    case Bytecodes::_isub	        : blk->arithmetic_int   (MethodClosure::sub);   break;
    case Bytecodes::_lsub	        : blk->arithmetic_long  (MethodClosure::sub);   break;
    case Bytecodes::_fsub	        : blk->arithmetic_float (MethodClosure::sub);   break;
    case Bytecodes::_dsub	        : blk->arithmetic_double(MethodClosure::sub);   break;
    case Bytecodes::_imul	        : blk->arithmetic_int   (MethodClosure::mul);   break;
    case Bytecodes::_lmul	        : blk->arithmetic_long  (MethodClosure::mul);   break;
    case Bytecodes::_fmul	        : blk->arithmetic_float (MethodClosure::mul);   break;
    case Bytecodes::_dmul	        : blk->arithmetic_double(MethodClosure::mul);   break;
    case Bytecodes::_idiv	        : blk->arithmetic_int   (MethodClosure::div);   break;
    case Bytecodes::_ldiv	        : blk->arithmetic_long  (MethodClosure::div);   break;
    case Bytecodes::_fdiv	        : blk->arithmetic_float (MethodClosure::div);   break;
    case Bytecodes::_ddiv	        : blk->arithmetic_double(MethodClosure::div);   break;
    case Bytecodes::_irem	        : blk->arithmetic_int   (MethodClosure::rem);   break;
    case Bytecodes::_lrem	        : blk->arithmetic_long  (MethodClosure::rem);   break;
    case Bytecodes::_frem	        : blk->arithmetic_float (MethodClosure::rem);   break;
    case Bytecodes::_drem	        : blk->arithmetic_double(MethodClosure::rem);   break;
    case Bytecodes::_ineg	        : blk->neg_int();	                        break;
    case Bytecodes::_lneg	        : blk->neg_long();	                        break;
    case Bytecodes::_fneg	        : blk->neg_float();	                        break;
    case Bytecodes::_dneg	        : blk->neg_double();	                        break;
    case Bytecodes::_ishl	        : blk->logical_int      (MethodClosure::_shl ); break;
    case Bytecodes::_ishr	        : blk->arithmetic_int   (MethodClosure::shr );  break;
    case Bytecodes::_iushr	        : blk->logical_int      (MethodClosure::_ushr); break;
    case Bytecodes::_lshl	        : blk->logical_long     (MethodClosure::_shl ); break;
    case Bytecodes::_lshr	        : blk->arithmetic_long  (MethodClosure::shr );  break;
    case Bytecodes::_lushr	        : blk->logical_long(MethodClosure::_ushr);      break;
    case Bytecodes::_iand	        : blk->logical_int (MethodClosure::_and );      break;
    case Bytecodes::_land	        : blk->logical_long(MethodClosure::_and );      break;
    case Bytecodes::_ior	        : blk->logical_int (MethodClosure::_or  );      break;
    case Bytecodes::_lor	        : blk->logical_long(MethodClosure::_or  );      break;
    case Bytecodes::_ixor	        : blk->logical_int (MethodClosure::_xor );      break;
    case Bytecodes::_lxor	        : blk->logical_long(MethodClosure::_xor );      break;
    case Bytecodes::_i2l	        : blk->int_2_long();	                        break;
    case Bytecodes::_i2f	        : blk->int_2_float();	                        break;
    case Bytecodes::_i2d	        : blk->int_2_double();	                        break;
    case Bytecodes::_l2i	        : blk->long_2_int();	                        break;
    case Bytecodes::_l2f	        : blk->long_2_float();	                        break;
    case Bytecodes::_l2d	        : blk->long_2_double();	                        break;
    case Bytecodes::_f2i	        : blk->float_2_int();	                        break;
    case Bytecodes::_f2l	        : blk->float_2_long();	                        break;
    case Bytecodes::_f2d	        : blk->float_2_double();	                break;
    case Bytecodes::_d2i	        : blk->double_2_int();	                        break;
    case Bytecodes::_d2l	        : blk->double_2_long();	                        break;
    case Bytecodes::_d2f	        : blk->double_2_float();	                break;
    case Bytecodes::_i2b	        : blk->int_2_byte();	                        break;
    case Bytecodes::_i2c	        : blk->int_2_char();	                        break;
    case Bytecodes::_i2s	        : blk->int_2_short();	                        break;
    case Bytecodes::_ifeq	        : blk->branch_if_int(MethodClosure::eq,      get_dest());   break;
    case Bytecodes::_ifnull	        : blk->branch_if_oop(MethodClosure::null,    get_dest());   break;
    case Bytecodes::_iflt	        : blk->branch_if_int(MethodClosure::lt,      get_dest());   break;
    case Bytecodes::_ifle	        : blk->branch_if_int(MethodClosure::le,      get_dest());   break;
    case Bytecodes::_ifne	        : blk->branch_if_int(MethodClosure::ne,      get_dest());   break;
    case Bytecodes::_ifnonnull          : blk->branch_if_oop(MethodClosure::nonnull, get_dest());   break;
    case Bytecodes::_ifgt	        : blk->branch_if_int(MethodClosure::gt,      get_dest());   break;
    case Bytecodes::_ifge	        : blk->branch_if_int(MethodClosure::ge,      get_dest());   break;
    case Bytecodes::_if_icmpeq          : blk->branch_if_icmp(MethodClosure::eq, get_dest());   break;
    case Bytecodes::_if_icmpne          : blk->branch_if_icmp(MethodClosure::ne, get_dest());   break;
    case Bytecodes::_if_icmplt          : blk->branch_if_icmp(MethodClosure::lt, get_dest());   break;
    case Bytecodes::_if_icmpgt          : blk->branch_if_icmp(MethodClosure::gt, get_dest());   break;
    case Bytecodes::_if_icmple          : blk->branch_if_icmp(MethodClosure::le, get_dest());   break;
    case Bytecodes::_if_icmpge          : blk->branch_if_icmp(MethodClosure::ge, get_dest());   break;
    case Bytecodes::_lcmp	        : blk->long_cmp();	                        break;
    case Bytecodes::_fcmpl	        : blk->float_cmpl();	                        break;
    case Bytecodes::_fcmpg	        : blk->float_cmpg();	                        break;
    case Bytecodes::_dcmpl	        : blk->double_cmpl();	                        break;
    case Bytecodes::_dcmpg	        : blk->double_cmpg();	                        break;
    case Bytecodes::_if_acmpeq          : blk->branch_if_acmp(MethodClosure::eq, get_dest());   break;
    case Bytecodes::_if_acmpne          : blk->branch_if_acmp(MethodClosure::ne, get_dest());   break;
    case Bytecodes::_goto	        : blk->branch(get_dest());	                break;
    case Bytecodes::_goto_w	        : blk->branch(get_far_dest());	                break;
    case Bytecodes::_jsr	        : blk->jsr(get_dest());	                        break;
    case Bytecodes::_jsr_w	        : blk->jsr(get_far_dest());	                break;
    case Bytecodes::_ret	        : blk->ret(get_index_special());                break;
    case Bytecodes::_ireturn	        : blk->return_int();	                        break;
    case Bytecodes::_lreturn	        : blk->return_long();	                        break;
    case Bytecodes::_freturn	        : blk->return_float();	                        break;
    case Bytecodes::_dreturn	        : blk->return_double();	                        break;
    case Bytecodes::_areturn	        : blk->return_object();	                        break;
    case Bytecodes::_return	        : blk->return_void();	                        break;
    case Bytecodes::_tableswitch	:
      { align();
        int  default_dest = get_far_dest();
        int  lo           = get_int();
        int  hi           = get_int();
        int  len          = hi - lo + 1;
        jint* dest        = NEW_RESOURCE_ARRAY(jint, len);
        for (int i = 0; i < len; i++) {
          dest[i] = get_far_dest();
        }
        blk->table_switch(lo, hi, dest, default_dest);
      }
      break;
    case Bytecodes::_lookupswitch     :
    case Bytecodes::_fast_linearswitch:
    case Bytecodes::_fast_binaryswitch:
      { align();
        int  default_dest = get_far_dest();
        int  len          = get_int();
        jint* key         = NEW_RESOURCE_ARRAY(jint, len);
        jint* dest        = NEW_RESOURCE_ARRAY(jint, len);
        for (int i = 0; i < len; i++) {
          key [i] = get_int();
          dest[i] = get_far_dest();
        };
        blk->lookup_switch(len, key, dest, default_dest);
      }
      break;
    case Bytecodes::_putfield         : {
      int i = get_big_index();
      constantPoolOop constants = method_handle()->constants();
      symbolOop name = constants->name_ref_at(i);
      blk->put_field_by_index(i, name);
      break;
    }
    case Bytecodes::_getfield         : {
      int i = get_big_index();
      constantPoolOop constants = method_handle()->constants();
      symbolOop name = constants->name_ref_at(i);
      blk->get_field_by_index(i, name);
      break;
    }
    case Bytecodes::_putstatic        : {
      int i = get_big_index();
      blk->put_static_by_index(i);
      break;
    }
    case Bytecodes::_getstatic        : {
      int i = get_big_index();
      blk->get_static_by_index(i);
      break;
    }
    case Bytecodes::_invokevirtual    :
      { int i = get_big_index();
        constantPoolOop constants = method_handle()->constants();
        blk->invoke_virtual_klass_by_index(i , constants->name_ref_at(i), constants->signature_ref_at(i));
      }
      break;
    case Bytecodes::_invokespecial    :
      { int i = get_big_index();
        constantPoolOop constants = method_handle()->constants();
        blk->invoke_nonvirtual_by_index(i, constants->name_ref_at(i), constants->signature_ref_at(i));
      }
      break;
    case Bytecodes::_invokestatic     :
      { int i = get_big_index();
        constantPoolOop constants = method_handle()->constants();
        blk->invoke_static_by_index(i, constants->name_ref_at(i), constants->signature_ref_at(i));
      }
      break;
    case Bytecodes::_invokeinterface  :
      { int i = get_big_index();
        int n = get_index();
        get_index();
        constantPoolOop constants = method_handle()->constants();
        blk->invoke_interface_by_index(i, constants->name_ref_at(i), constants->signature_ref_at(i), n);
      }
      break;
    case Bytecodes::_athrow: 
	blk->throw_exception();
	break;
    case Bytecodes::_new:
        { int klass_index = get_big_index();
	  blk->new_object_by_index(klass_index);
        }
	break;
    case Bytecodes::_checkcast:
        { int klass_index = get_big_index();
	  blk->check_cast_by_index(klass_index);
        }
	break;
    case Bytecodes::_instanceof:
        { int klass_index = get_big_index();
	  blk->instance_of_by_index(klass_index);
        }
	break;
    case Bytecodes::_monitorenter:
	blk->monitor_enter();
	break;
    case Bytecodes::_monitorexit:
	blk->monitor_exit();
	break;
    // Java specific bytecodes    
    case Bytecodes::_wide: 
      ShouldNotReachHere();
      break;

    case Bytecodes::_fast_aload_0     : blk->load_local_object(0); break;
    case Bytecodes::_fast_iaccess_0   : 
      get_byte();
      blk->fast_iaccess_0(get_fast_index());
      break;
    case Bytecodes::_fast_aaccess_0   : 
      get_byte();
      blk->fast_aaccess_0(get_fast_index());
      break;

    case Bytecodes::_fast_igetfield   :
      blk->get_field(T_INT   , get_fast_index());   break;
    case Bytecodes::_fast_agetfield   :
      blk->get_field(T_OBJECT, get_fast_index());   break;

    // Unknown bytecodes
    default:
	blk->unknown_bytecode();
	break;
  }
  blk->bytecode_epilog();
}

// Virtual method that returns the next bytecode. Can be used by subclasses
// to alter the control flow. Returns -1 when there are no more bytecodes.
Bytecodes::Code MethodIterator::get_next_bytecode() {
  return _stream->next();
}

void MethodIterator::iterate(MethodClosure* blk) {
  _method = interval()->method_handle(); 
  
  if (TraceMethodIterator) {
    tty->print("MethodIterator [%d-%d]: ", interval()->begin_bci(), interval()->end_bci());
    method_handle()->print_value();
    tty->cr();
  }

  // Setup underlying iterator
  assert(_stream==NULL, "iterate is not re-entrant");
  BytecodeStream stream(_method);        
  stream.set_interval(interval()->begin_bci(), interval()->end_bci());
  _stream = &stream;
  
  // Initialize closure
  blk->set_method (_method); 
  blk->reset_is_paired_instruction();
  blk->set_exception_thrown(false);
  
  // Iteration is basicly hidden in the bytecode stream
  Bytecodes::Code bc;
  while ((bc = get_next_bytecode()) >= 0 && !blk->is_exception_thrown()) {
    // Setup data about new instruction
    _this_pc = stream.bcp();
    _next_pc = _this_pc + (_stream->is_wide() ? 2 : 1);   // skip wide
    blk->set_bci(stream.bci()); 
    identify_code(blk, bc);
  }

  // iteration is done, no stream used.
  _stream = NULL;
}


//----------------------------------------------------------------------------
// MethodByteCodeStream code

// Handling for the wide bytcode
Bytecodes::Code MethodByteCodeStream::wide()
{
  // Get following bytecode; do not return wide
  Bytecodes::Code bc = (Bytecodes::Code)_pc[1];	
  _pc += 2;			// Skip both bytecodes
  _pc += 2;			// Skip index always
  if( bc == Bytecodes::_iinc ) 
    _pc += 2;			// Skip optional constant
  _was_wide = _pc;		// Flag last wide bytecode found
  return bc;
}

Bytecodes::Code MethodByteCodeStream::table( Bytecodes::Code bc ) {
  switch( bc ) {		// Check for special bytecode handling
    
  case Bytecodes::_fast_linearswitch: 
  case Bytecodes::_fast_binaryswitch: 
  case Bytecodes::_lookupswitch:
    _pc++;			// Skip wide bytecode
    _pc += (_start-_pc)&3;	// Word align
    _table_base = (jint*)_pc;	// Capture for later usage
				// table_base[0] is default far_dest
    // Table has 2 lead elements (default, length), then pairs of u4 values.
    // So load table length, and compute address at end of table
    _pc = (address)&_table_base[2+ 2*Bytes::get_Java_u4((address)&_table_base[1])];
    break;

  case Bytecodes::_tableswitch: { 
    _pc++;			// Skip wide bytecode
    _pc += (_start-_pc)&3;	// Word align
    _table_base = (jint*)_pc;	// Capture for later usage
				// table_base[0] is default far_dest
    int lo = Bytes::get_Java_u4((address)&_table_base[1]);// Low bound
    int hi = Bytes::get_Java_u4((address)&_table_base[2]);// High bound
    int len = hi - lo + 1;	// Dense table size
    _pc = (address)&_table_base[3+len];	// Skip past table
    break;
  }

  default:			 
    fatal("unhandled bytecode");	 
  }
  return bc;
}

static u_char java_buf[4] ={
  Bytecodes::_aload_0,
  Bytecodes::_fast_igetfield,
  0,
  0
};

// Handle End-Of-Bytecodes.  Special handling for _go_native here.
Bytecodes::Code MethodByteCodeStream::EOBCs() {
  if( _restore_pc ) {		// Was running out of a temp bytecode buffer?
    _start -= java_buf+4-_restore_pc; // Restore original starting point
    _pc = _restore_pc;		// Yes, restore original pc past temp area
    _end = _restore_end;	// Restore end of original bytecode buffer
    _restore_pc = 0;		// Flag as running out of original buffer
    return next();		// Now fetch normally
  }
  return (Bytecodes::Code)EOBC;
}

void MethodByteCodeStream::reset_to_bci( int bci ) {
  if( _restore_pc ) {
    _start -= java_buf+4-_restore_pc; // Restore original starting point
    _end = _restore_end;	// Restore end of original bytecode buffer
  }
  _bc_start=_was_wide=_restore_pc = 0; 
  _pc = _start+bci; 
}


Bytecodes::Code MethodByteCodeStream::java( Bytecodes::Code bc )
{
  switch( bc ) {		// Check for special bytecode handling

  case Bytecodes::_fast_aload_0       : bc = Bytecodes::_aload_0;       break;

  case Bytecodes::_fast_iaccess_0: {
    // _aload_0, _getfield(itos) pair
    java_buf[1] = Bytecodes::_getfield;
    java_buf[2] = _pc[2];	// Copy native-order index bits to temp buffer
    java_buf[3] = _pc[3];
    _start += (java_buf-_pc);	// Move "as-if" start point
    _restore_pc = _pc+4;	// Capture current pc past end of java
    _restore_end = _end;	// Capture current buffer end
    _pc = (address)java_buf;	// Set _pc to nested buffer
    _end = (address)java_buf+4;
    return next();		// Fetch bytecodes from nested buffer
  }

  case Bytecodes::_fast_aaccess_0: {  
    // _aload_0, _getfield(atos) pair
    java_buf[1] = Bytecodes::_getfield;
    java_buf[2] = _pc[2];	// Copy native-order index bits to temp buffer
    java_buf[3] = _pc[3];
    _restore_pc = _pc+4;	// Capture current pc past end of java
    _start += (java_buf+4-_restore_pc);	// Move "as-if" start point
    _restore_end = _end;	// Capture current buffer end
    _pc = (address)java_buf;	// Set _pc to nested buffer
    _end = (address)java_buf+4;
    return next();		// Fetch bytecodes from nested buffer
  }

  case Bytecodes::_fast_igetfield: 
  case Bytecodes::_fast_agetfield: bc = Bytecodes::_getfield; break;

  default:			 
    fatal("unhandled bytecode");	 
  }

  // Handle fast variants
  _pc += Bytecodes::length_at(_pc);
  return bc;
}
