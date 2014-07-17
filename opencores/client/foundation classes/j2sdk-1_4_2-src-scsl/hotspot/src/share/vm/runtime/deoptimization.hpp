#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)deoptimization.hpp	1.71 03/01/23 12:21:57 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Deoptimization : AllStatic {
 public:
  // Interface for staring deoptimization
   enum DeoptInfo {
     Deopt_unloaded              =   0, // positive values describe a constant pool index that needs to be loaded
     Deopt_athrow                =  -1, // Bytecode _athrow,             do not recompile 
     Deopt_null_check            =  -2, // failed null check,            do not recompile
     Deopt_div0_check            =  -3, // failed div0 check,            do not recompile
     Deopt_range_check           =  -4, // failed range check,           do not recompile
     Deopt_cast_check            =  -5, // failed optimized checkcast,   do not recompile
     Deopt_array_store_check     =  -6, // failed optimized checkcast, record failure and recompile
     Deopt_uninitialized         =  -7, // uninitialized class,    recompile after interpreter executes
     Deopt_unloaded_array_class  =  -8, // array-class not loaded, recompile after interpreter loads
     Deopt_unexpected_klass      =  -9, // type prediction failure, invalidate profile and recompile
     Deopt_unexpected_null_cast  = -10, // cast-never-null failed; record failure and recompile
     Deopt_unhandled_bytecode    = -11, // unimplemented bytecode,       do not recompile
     Deopt_math_pow_check        = -12, // Math.pow intrinsic returns NaN, do not intrinsify
     Deopt_tier1_counter         = -13  // tier one invocation threshold reached
   };

   
   enum UnpackType {
     Unpack_deopt                = 0, // normal deoptimization, use pc computed in unpack_vframe_on_stack
     Unpack_exception            = 1, // exception is pending
     Unpack_uncommon_trap        = 2, // redo last byte code (C2 only)
     Unpack_reexecute            = 3  // reexecute bytecode (C1 only)
   };

  // Checks all compiled methods. Invalid methods are deleted and
  // corresponding activations are deoptimized.
  static int deoptimize_dependents();
  
  // Deoptimizes a frame into a deoptimized frame.
  static void deoptimize(JavaThread* thread, frame fr, frame callee_fr, RegisterMap *reg_map);
  static vframeArray* create_vframeArray(JavaThread* thread, frame fr, RegisterMap *reg_map);
 
  // Interface used for unpacking deoptimized frames 

  // UnrollBlock is returned by fetch_unroll_info() to the deoptimization handler (blob).
  class UnrollBlock : public CHeapObj {
   private:
    int       _size_of_deoptimized_frame; // Size, in bytes, of current deoptimized frame
    int       _adapter_size;              // size of I2C or OSR adapter frame if popping one
    int       _new_adapter;               // non-zero if adding a new adapter
    int       _caller_adjustment;         // Adjustment, in bytes, to caller's SP when popping adapter
    int       _number_of_frames;          // Number frames to unroll 
    intptr_t* _frame_sizes;               // Array of frame sizes, in bytes, for unrolling the stack
    address*  _frame_pcs;                 // Array of frame pc's, in bytes, for unrolling the stack
    intptr_t* _register_block;            // Block for storing callee-saved registers.
    BasicType _return_type;               // Tells if we have to restore double or long return value
    // The following fields are used as temps during the unpacking phase
    // (which is tight on registers, especially on x86). They really ought
    // to be PD variables but that involves moving this class into its own
    // file to use the pd include mechanism. Maybe in a later cleanup ...
    intptr_t  _counter_temp;	          // SHOULD BE PD VARIABLE (x86 frame count temp)
    intptr_t  _initial_fp;		  // SHOULD BE PD VARIABLE (x86/c2 initial ebp)
    intptr_t  _unpack_kind;		  // SHOULD BE PD VARIABLE (x86 unpack kind)
    intptr_t  _sender_sp_temp;		  // SHOULD BE PD VARIABLE (x86 sender_sp)
   public:
    // Constructor
    UnrollBlock(int  size_of_deoptimized_frame,
		int  adapter_size,
		int  new_adapter,
                int  caller_adjustment,
                int  number_of_frames,
                intptr_t* frame_sizes,
                address* frames_pcs,
                BasicType return_type);
    ~UnrollBlock();

    // Returns where a register is located.
    intptr_t* value_addr_at(int register_number) const;

    // Accessors
    intptr_t* frame_sizes()  const { return _frame_sizes; }
    int number_of_frames()  const { return _number_of_frames; }
    address*  frame_pcs()   const { return _frame_pcs ; }
   
    // Returns the total size of frames
    int size_of_frames() const;

    // Accessors used by the code generator for the unpack stub.
    static int size_of_deoptimized_frame_offset_in_bytes() { return (intptr_t)&((UnrollBlock*)NULL)->_size_of_deoptimized_frame; }
    static int adapter_size_offset_in_bytes()              { return (intptr_t)&((UnrollBlock*)NULL)->_adapter_size;              }
    static int new_adapter_offset_in_bytes()               { return (intptr_t)&((UnrollBlock*)NULL)->_new_adapter;               }
    static int caller_adjustment_offset_in_bytes()         { return (intptr_t)&((UnrollBlock*)NULL)->_caller_adjustment;         }
    static int number_of_frames_offset_in_bytes()          { return (intptr_t)&((UnrollBlock*)NULL)->_number_of_frames;          }
    static int frame_sizes_offset_in_bytes()               { return (intptr_t)&((UnrollBlock*)NULL)->_frame_sizes;               }
    static int frame_pcs_offset_in_bytes()                 { return (intptr_t)&((UnrollBlock*)NULL)->_frame_pcs;                 }
    static int register_block_offset_in_bytes()            { return (intptr_t)&((UnrollBlock*)NULL)->_register_block;            }  
    static int return_type_offset_in_bytes()               { return (intptr_t)&((UnrollBlock*)NULL)->_return_type;               }  
    static int counter_temp_offset_in_bytes()              { return (intptr_t)&((UnrollBlock*)NULL)->_counter_temp;              }  
    static int initial_fp_offset_in_bytes()                { return (intptr_t)&((UnrollBlock*)NULL)->_initial_fp;                }  
    static int unpack_kind_offset_in_bytes()               { return (intptr_t)&((UnrollBlock*)NULL)->_unpack_kind;               }  
    static int sender_sp_temp_offset_in_bytes()            { return (intptr_t)&((UnrollBlock*)NULL)->_sender_sp_temp;            }  

    BasicType return_type() const { return _return_type; }
    void print();
  };

  //** Returns an UnrollBlock continuing information 
  // how to make room for the resulting interpreter frames. 
  // Called by assembly stub after execution has returned to 
  // deoptimized frame.
  // @argument thread.     Thread where stub_frame resides.
  // @see OptoRuntime::deoptimization_fetch_unroll_info_C
  static UnrollBlock* fetch_unroll_info(JavaThread* thread);

  //** Unpacks vframeArray onto execution stack
  // Called by assembly stub after execution has returned to
  // deoptimized frame and after the stack unrolling.
  // @argument thread.     Thread where stub_frame resides.
  // @argument exec_mode.  Determines how execution should be continuted in top frame.
  //                       0 means continue after current byte code
  //                       1 means exception has happened, handle exception
  //                       2 means reexecute current bytecode (for uncommon traps).
  // @see OptoRuntime::deoptimization_unpack_frames_C
  // Return BasicType of call return type, if any
  static BasicType unpack_frames(JavaThread* thread, int exec_mode);

  //** Performs an uncommon trap for compiled code.
  // The top most compiler frame is converted into interpreter frames
  static UnrollBlock* uncommon_trap(JavaThread* thread, jint unloaded_class_index);

  //** Deoptimizes the frame identified by sp.
  // Only called from VMDeoptimizeFrame
  // @argument thread.     Thread where stub_frame resides.
  // @argument id.         id of frame that should be deoptimized.
  static void deoptimize_frame(JavaThread* thread, intptr_t* id);

  // Statistics
  static int  print_stat_line(const char *name, long r)                  PRODUCT_RETURN0;
  static void gather_uncommon_trap_statistics(int unloaded_class_index)  PRODUCT_RETURN;
  static void print_statistics()                                         PRODUCT_RETURN;  

  // How much room to adjust the last frame's SP by, to make space for
  // the callee's interpreter frame (which expects locals to be next to
  // incoming arguments)
  static int last_frame_adjust(int callee_parameters, int callee_locals);

  // Advice to compilers, whether they should compile unexpected_klass traps.
  // A call site that traps too much (even once?) should not trap again.
  // %%% This is a Temporary Kludge.  TO DO: The trap history should be kept,
  // per bci, in the methodDataOop that describes the method.
  static bool unexpected_klass_traps_enabled() {
    return _unexpected_klass_traps_enabled;
  }

  static const char* deopt_info_name(int deopt_info);

#ifdef HOTSWAP
  // PopFrame support

  // Preserves incoming arguments to the popped frame when it is
  // returning to a deoptimized caller
  static void popframe_preserve_args(JavaThread* thread, int bytes_to_save, void* start_address);

#endif HOTSWAP

 private:
  // class loading support for uncommon trap
  static void load_class_by_index(constantPoolHandle constant_pool, int index, TRAPS);
  static void load_class_by_index(constantPoolHandle constant_pool, int index);

  static UnrollBlock* fetch_unroll_info_helper(JavaThread* thread);

  static int  _unexpected_klass_traps;
  static bool _unexpected_klass_traps_enabled;

#ifndef PRODUCT
  static long _nof_deoptimizations;       // Total count
  static long _nof_unloaded;
  static long _nof_athrow;
  static long _nof_null_check;
  static long _nof_div0_check;
  static long _nof_negative_array_length;
  static long _nof_range_check;
  static long _nof_cast_check;
  static long _nof_array_store_check;
  static long _nof_uninitialized;
  static long _nof_unloaded_array_class;
  static long _nof_unexpected_klass;
  static long _nof_unexpected_null_cast;
  static long _nof_unhandled_bytecode;
  static long _nof_math_pow_check;
  static long _nof_tier1_counter;
#endif
};

class DeoptimizationMarker : StackObj {  // for profiling
  static bool _is_active;  
public:
  DeoptimizationMarker()  { _is_active = true; }
  ~DeoptimizationMarker() { _is_active = false; }
  static bool is_active() { return _is_active; }
};


