#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)invocationCounter.hpp	1.38 03/01/23 12:05:28 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// InvocationCounters are used to trigger actions when a limit (threshold) is reached.
// For different states, different limits and actions can be defined in the initialization
// routine of InvocationCounters.
//
// Implementation notes: For space reasons, state & counter are both encoded in one word,
// The state is encoded using some of the least significant bits, the counter is using the
// more significant bits. The counter is incremented before a method is activated and an
// action is triggered when when count() > limit().

class InvocationCounter VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
 private:                                        // bit no: |31   |    0|
  unsigned int _counter;                         // format: [count|state]

  enum PrivateConstants {
    number_of_state_bits = 2,
    number_of_carry_bits = 1,
    number_of_noncount_bits = number_of_state_bits + number_of_carry_bits,
    number_of_count_bits = BitsPerInt - number_of_noncount_bits,
    state_limit          = nth_bit(number_of_state_bits),
    count_grain          = nth_bit(number_of_state_bits + number_of_carry_bits),
    count_limit          = nth_bit(number_of_count_bits - 1),
    carry_mask           = right_n_bits(number_of_carry_bits) << number_of_state_bits,
    state_mask           = right_n_bits(number_of_state_bits),
    status_mask          = right_n_bits(number_of_state_bits + number_of_carry_bits),
    count_mask           = ((int)(-1) ^ status_mask)
  };

 public:
  static int InterpreterInvocationLimit;        // CompileThreshold scaled for interpreter use

  static int InterpreterBackwardBranchLimit;    // A separate threshold for on stack replacement

  static int InterpreterProfileLimit;           // Profiling threshold scaled for interpreter use

  typedef address (*Action)(methodHandle method, TRAPS);

  enum PublicConstants {
    count_increment      = count_grain,          // use this value to increment the 32bit _counter word
    count_mask_value     = count_mask            // use this value to mask the backedge counter
  };

  enum State {
    wait_for_nothing,                            // do nothing when count() > limit()
    wait_for_compile,                            // introduce nmethod when count() > limit()   
    number_of_states                             // must be <= state_limit
  };

  // Manipulation
  void reset();                                  // sets state into original state
  void set_state(State state);                   // sets state and initializes counter correspondingly
  inline void set(State state, int count);       // sets state and counter
  inline void decay();                           // decay counter (divide by two)
  void set_carry();                              // set the sticky carry bit
  
  // Accessors
  State  state() const                           { return (State)(_counter & state_mask); }
  bool   carry() const                           { return (_counter & carry_mask) != 0; }
  int    limit() const                           { return CompileThreshold; }
  Action action() const                          { return _action[state()]; }
  int    count() const                           { return _counter >> number_of_noncount_bits; }
  bool   has_overflowed() const                  { return count() >= limit(); }

  void increment()                               { _counter += count_increment; }


  // Printing
  void   print();
  void   print_short();

  // Miscellaneous
  static ByteSize counter_offset()               { return byte_offset_of(InvocationCounter, _counter); }
  static void reinitialize(bool delay_overflow);
  
 private:
  static int         _init  [number_of_states];  // the counter limits
  static Action      _action[number_of_states];  // the actions

  static void        def(State state, int init, Action action);
  static const char* state_as_string(State state);
  static const char* state_as_short_string(State state);
};

inline void InvocationCounter::set(State state, int count) {
  assert(0 <= state && state < number_of_states, "illegal state");
  int carry = (_counter & carry_mask);    // the carry bit is sticky
  _counter = (count << number_of_noncount_bits) | carry | state;
}

inline void InvocationCounter::decay() {
  int c = count();
  int new_count = c >> 1;
  // prevent from going to zero, to distinguish from never-executed methods
  if (c > 0 && new_count == 0) new_count = 1; 
  set(state(), new_count);
}


