#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)perfData.hpp	1.9 03/01/23 12:24:16 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * Counter naming macros
 */
#define PERF_HOTSPOT "hotspot"
#define PERF_RUNTIME PERF_HOTSPOT ".rt"
#define PERF_GC      PERF_HOTSPOT ".gc"
#define PERF_CI      PERF_HOTSPOT ".ci"
#define PERF_C1      PERF_HOTSPOT ".c1"
#define PERF_C2      PERF_HOTSPOT ".c2"
#define PERF_INTERP  PERF_HOTSPOT ".interp"

/*
 * Classes to support access to production performance data
 *
 * The PerfData class structure is provided for creation, access, and update
 * of performance data (a.k.a. instrumentation) in a specific memory region
 * which is possibly possibly shared. Although not explicitly prevented from
 * doing so, developers should not use the values returned by these methods
 * to make algorithmic decisions as they are potentially extracted from a
 * shared memory region. If the PerfData memory region is created in a shared
 * memory name space, it is created with appropriate access restrictions
 * providing read-write access only to the principal that created the JVM.
 * It is believed that a PerfData memory region created as a shared memory
 * object facilitates an easier attack path than attacks launched through
 * mechanisms such as /proc. For this reason, it is recommended that
 * data accessed via the PerfData objects be treated as 'write only' data
 * until further notice.
 *
 * There are three variability classifications of performance data
 *   Constants  -  value is written to the PerfData memory once, on creation
 *   Variables  -  value is modifiable, with no particular restrictions
 *   Counters   -  value is monotonically changing (increasing or decreasing)
 *
 * The performance data items can also have various types. The class
 * hierarchy and the structure of the memory region are designed to
 * accommodate new types as they are needed. Types are specified in
 * terms of Java basic types, which accommodates client applications
 * written in the Java programming language. The class hierarchy is:
 *
 * - PerfData (Abstract)
 *     - PerfLong (Abstract)
 *         - PerfLongConstant        (alias: PerfConstant)
 *         - PerfLongVariant (Abstract)
 *             - PerfLongVariable    (alias: PerfVariable)
 *             - PerfLongCounter     (alias: PerfCounter)
 *    
 *     - PerfByteArray (Abstract)
 *         - PerfString (Abstract)
 *             - PerfStringVariable
 *             - PerfStringConstant
 * 
 *
 * As seen in the class hierarchy, the initially supported types are:
 *
 *    Long      - performance data holds a Java long type
 *    ByteArray - performance data holds an array of Java bytes
 *                used for holding C++ char arrays.
 *
 * The String type is derived from the ByteArray type.
 *
 * A PerfData subtype is not required to provide an implementation for
 * each variability classification. For example, the String type provides
 * Variable and Constant variablility classifications in the PerfStringVariable 
 * and PerfStringConstant classes, but does not provide a counter type.
 *
 * Performance data are also described by a unit of measure. Units allow
 * client applications to make reasonable decisions on how to treat
 * performance data generically, preventing the need to hard-code the
 * specifics of a particular data item. The current set of units are:
 *
 *   None        - the data has no units of measure
 *   Bytes       - data is measured in bytes
 *   Ticks       - data is measured in clock ticks
 *   Events      - data is measured in events. For example,
 *                 the number of garbage collection events or the
 *                 number of methods compiled.
 *   String      - data is not numerical. For example,
 *                 the java command line options
 *   Hertz       - data is a frequency
 *   Objects     - data is measured in objects
 *
 * Currently, instances of PerfData subtypes are considered to have
 * a lifetime equal to that of the VM and are managed by the
 * PerfDataManager class. All constructors for the PerfData class and
 * its subtypes have protected constructors. Creation of PerfData
 * instances is performed by invoking various create methods on the
 * PerfDataManager class. Users should not attempt to delete these
 * instances as the PerfDataManager class expects to perform deletion
 * operations on exit of the VM.
 *
 * Examples:
 *
 * Creating performance counter that holds a monotonically increasing
 * long data value with units specified in U_Bytes.
 *
 *    [static] PerfLongCounter* my_counter;
 *
 *    my_counter = PerfDataManager::create_long_counter("my.counter", PerfData::U_Bytes, optionalInitialValue, CHECK);
 *    my_counter->inc();
 *
 *    In this example, PerfCounter can be substituted for PerfLongCounter
 *    and create_counter() for create_long_counter() for brevity.
 * 
 * Creating a performance counter that holds a long value that is not
 * monotonically changing. For example, a counter that contains the
 * current utilization of a heap space. 
 *
 *    [static] PerfLongVariable* my_variable;
 *    my_variable = PerfDataManager::create_long_variable("my.variable", PerfData::U_Bytes, optionalInitialValue, CHECK);
 *
 *    my_variable->inc();
 *    my_variable->set_value(0);
 *
 *    As in the previous example, PerfVariable can be substituted for
 *    PerfLongVariable and create_variable() for create_long_variable()
 *    for brevity.
 *
 * Creating a performance counter that holds a constant string value.
 *
 *    PerfDataManager::create_string_constant("my.string.constant", string, CHECK);
 *
 *    Although the create_string_constant() factory method returns a pointer
 *    to the PerfStringConstant object, it can safely be ignored. Developers
 *    are not encouraged to access the string constant's value via this
 *    pointer at this time due to security concerns.
 *
 * Creating a performance counter that holds a value that is sampled
 * by the StatSampler periodic task.
 *
 *     PerfDataManager::create_counter("my.sampled", PerfData::U_Events, &my_jlong, CHECK);
 *
 *     Here the PerfData pointer can be ignored as the caller is
 *     relying on the StatSampler PeriodicTask to sample the given
 *     address at a regular interval. The interval is defined by the
 *     PerfDataSamplingInterval global variable, and is applyied on
 *     a system wide basis, not on an per-counter basis.
 *
 * Creating a performance counter that utilizes a helper object to
 * return a value to the StatSampler via the take_sample() method.
 *
 *     class MyTimeSampler : public PerfLongSampleHelper {
 *       public:
 *         jlong take_sample() { return os::elapsed_counter(); }
 *     };
 *
 *     PerfDataManager::create_counter("my.sampled.helper", PerfData::U_Ticks,  new MyTimeSampler(), CHECK);
 *
 *     In this example, a subtype of PerfLongSampleHelper is instantiated
 *     and its take_sample() method is overridden to perform whatever
 *     operation is necessary to generate the data sample. This method
 *     will be called by the StatSampler at a regular interval, defined
 *     by the PerfDataSamplingInterval global variable.
 *
 *     As before, PerfSampleHelper is an alias for PerfLongSampleHelper.
 *
 * For additional uses of PerfData subtypes, see the utility classes
 * PerfTraceTime and PerfTraceTimedEvent below.
 *
 * Until further noice, all PerfData objects should be created and
 * manipulated within a guarded block. The guard variable is
 * UsePerfData, a product flag set to true by default. Prior to
 * FCS, this flag will be make a develop flag and eventually, the
 * flag and all its uses will be removed from the product.
 *
 */
class PerfData : public CHeapObj {

  friend class StatSampler;      // for access to protected void sample()
  friend class PerfDataManager;  // for access to protected destructor

  public:

    // the Variability enum must be kept in synchronization with the
    // the com.sun.hotspot.perfdata.Variability class
    enum Variability {
      V_Constant = 1,
      V_Monotonic = 2,
      V_Variable = 3
    };

    // the Units enum must be kept in synchronization with the
    // the com.sun.hotspot.perfdata.Units class
    enum Units {
      U_None = 1,
      U_Bytes = 2,
      U_Ticks = 3,
      U_Events = 4,
      U_String = 5,
      U_Hertz = 6
    };

  private:
    char* _name;
    Variability _v;
    Units _u;
    bool _on_c_heap;

    PerfDataEntry* _pdep;

  protected:

    void *_valuep;

    PerfData(const char* name, Units u, Variability v);
    ~PerfData();

    // create the entry for the PerfData item in the PerfData memory region. 
    // this region is maintained separately from the PerfData objects to
    // facilitate its use by external processes.
    void create_entry(BasicType dtype, size_t dsize, size_t dlen = 0);

    // sample the data item given at creation time and write its value
    // into the its corresponding PerfMemory location.
    virtual void sample() = 0;

  public:

    // returns a boolean indicating the validity of this object.
    // the object is valid if and only if memory in PerfMemory
    // region was successfully allocated.
    inline bool is_valid() { return _valuep != NULL; }

    // returns a boolean indicating whether the underlying object
    // was allocated in the PerfMemory region or on the C heap.
    inline bool is_on_c_heap() { return _on_c_heap; }

    // returns a pointer to a char* containing the name of the item.
    // The pointer returned is the pointer to a copy of the name
    // passed to the constructor, not the pointer to the name in the
    // PerfData memory region. This redundancy is maintained for
    // security reasons as the PerfMemory region may be in shared
    // memory.
    const char* name() { return _name; }

    // lexicographically compare the name of this PerfData item to the
    // name of the PerfData item passed. If the name of this item occurs
    // lexicographically before the name of the passed item, then -1 is
    // returned. if lexicographically equal, then 0 is returned. If
    // lexicographically greater, then 1 is returned.
    int compare_by_name(PerfData* p);

    // returns the variability classification associated with this item
    Variability variability() { return _v; }

    // returns the units associated with this item.
    Units units() { return _u; }

    // returns the address of the data portion of the item in the
    // PerfData memory region.
    inline void* get_address() { return _valuep; }

    // returns the value of the data portion of the item in the
    // PerfData memory region formatted as a string.
    virtual int format(char* cp, int length) = 0;
};

/*
 * PerfLongSampleHelper, and its alias PerfSamplerHelper, is a base class
 * for helper classes that rely upon the StatSampler periodic task to
 * invoke the take_sample() method and write the value returned to its
 * appropriate location in the PerfData memory region.
 */
class PerfLongSampleHelper : public CHeapObj {
  public:
    virtual jlong take_sample() = 0;
};

typedef PerfLongSampleHelper PerfSampleHelper;


/*
 * PerfLong is the base class for the various Long PerfData subtypes.
 * it contains implementation details that are common among its derived
 * types.
 */
class PerfLong : public PerfData {

  protected:

    PerfLong(const char* namep, Units u, Variability v);

  public:
    int format(char* buffer, int length);

    // returns the value of the data portion of the item in the
    // PerfData memory region.
    inline jlong get_value() { return *(jlong*)_valuep; }
};

/*
 * The PerfLongConstant class, and its alias PerfConstant, implement
 * a PerfData subtype that holds a jlong data value that is set upon
 * creation of an instance of this class. This class provides no
 * methods for changing the data value stored in PerfData memory region.
 */
class PerfLongConstant : public PerfLong {

  friend class PerfDataManager; // for access to protected constructor

  private:
    // hide sample() - no need to sample constants
    void sample() { }

  protected:

    PerfLongConstant(const char* namep, Units u, jlong initial_value) : PerfLong(namep, u, V_Constant) {

       if (is_valid()) *(jlong*)_valuep = initial_value;
    }
};

typedef PerfLongConstant PerfConstant;

/*
 * The PerfLongVariant class, and its alias PerfVariant, implement
 * a PerfData subtype that holds a jlong data value that can be modified
 * in an unrestricted manner. This class provides the implementation details
 * for common functionality among its derived types.
 */
class PerfLongVariant : public PerfLong {

  protected:
    jlong* _sampled;
    PerfLongSampleHelper* _sample_helper;

    PerfLongVariant(const char* namep, Units u, Variability v, jlong initial_value=0) : PerfLong(namep, u, v) {
      if (is_valid()) *(jlong*)_valuep = initial_value;
    }

    PerfLongVariant(const char* namep, Units u, Variability v, jlong* sampled);

    PerfLongVariant(const char* namep, Units u, Variability v, PerfLongSampleHelper* sample_helper);

    void sample();

  public:
    inline void inc() { (*(jlong*)_valuep)++; }
    inline void inc(jlong val) { (*(jlong*)_valuep) += val; }
    inline void add(jlong val) { (*(jlong*)_valuep) += val; }
};

/*
 * The PerfLongCounter class, and its alias PerfCounter, implement
 * a PerfData subtype that holds a jlong data value that can (should)
 * be modified in a monotonic manner. The inc(jlong) and add(jlong)
 * methods can be passed negative values to implement a monotonically
 * decreasing value. However, we rely upon the programmer to honor
 * the notion that this counter always moves in the same direction -
 * either increasing or decreasing.
 */
class PerfLongCounter : public PerfLongVariant {

  friend class PerfDataManager; // for access to protected constructor

  protected:

    PerfLongCounter(const char* namep, Units u, jlong initial_value=0) : PerfLongVariant(namep, u, V_Monotonic, initial_value) { }

    PerfLongCounter(const char* namep, Units u, jlong* sampled) : PerfLongVariant(namep, u, V_Monotonic, sampled) { }

    PerfLongCounter(const char* namep, Units u, PerfLongSampleHelper* sample_helper) : PerfLongVariant(namep, u, V_Monotonic, sample_helper) { }

};

typedef PerfLongCounter PerfCounter;

/*
 * The PerfLongVariable class, and its alias PerfVariable, implement
 * a PerfData subtype that holds a jlong data value that can
 * be modified in an unrestricted manner.
 */
class PerfLongVariable : public PerfLongVariant {

  friend class PerfDataManager; // for access to protected constructor

  protected:

    PerfLongVariable(const char* namep, Units u, jlong initial_value=0) : PerfLongVariant(namep, u, V_Variable, initial_value) { }

    PerfLongVariable(const char* namep, Units u, jlong* sampled) : PerfLongVariant(namep, u, V_Variable, sampled) { }

    PerfLongVariable(const char* namep, Units u, PerfLongSampleHelper* sample_helper) : PerfLongVariant(namep, u, V_Variable, sample_helper) { }

  public:
    inline void set_value(jlong val) { (*(jlong*)_valuep) = val; }
};

typedef PerfLongVariable PerfVariable;

/*
 * The PerfByteArray provides a PerfData subtype that allows the creation
 * of a contiguous region of the PerfData memory region for storing a vector
 * of bytes. This class is currently intended to be a base class for 
 * the PerfString class, and cannot be instantiated directly.
 */
class PerfByteArray : public PerfData {

  protected:
    jint _length;

    PerfByteArray(const char* namep, jint length, Units u, Variability v);
};

class PerfString : public PerfByteArray {

  protected:

    void set_string(const char* s2);

    PerfString(const char* namep, jint length, Variability v, const char* initial_value): PerfByteArray(namep, length, U_String, v) {
       if (is_valid()) set_string(initial_value);
    }

  public:

    int format(char* buffer, int length);
};

/*
 * The PerfStringConstant class provides a PerfData sub class that
 * allows a null terminated string of single byte characters to be
 * stored in the PerfData memory region.
 */
class PerfStringConstant : public PerfString {

  friend class PerfDataManager; // for access to protected constructor

  private:

    // hide sample() - no need to sample constants
    void sample() { }

  protected:

    // Restrict string constant lengths to be <= PerfMaxStringConstLength.
    // This prevents long string constants, as can occur with very
    // long classpaths or java command lines, from consuming too much
    // PerfData memory.
    PerfStringConstant(const char* namep, const char* initial_value);
};

/*
 * The PerfStringVariable class provides a PerfData sub class that
 * allows a null terminated string of single byte character data
 * to be stored in PerfData memory region. The string value can be reset
 * after initialization. If the string value is >= max_length, then
 * it will be truncated to max_length characters. The copied string
 * is always null terminated.
 */
class PerfStringVariable : public PerfString {

  friend class PerfDataManager; // for access to protected constructor

  protected:

    // sampling of string variables are not yet supported
    void sample() { }

    PerfStringVariable(const char* namep, const char* initial_value, jint max_length) : PerfString(namep, max_length+1, V_Variable, initial_value) {
    }

  public:
    inline void set_value(const char* val) { set_string(val); }
};


/*
 * The PerfDataList class is a container class for managing lists
 * of PerfData items. The intention of this class is to allow for
 * alternative implementations for management of list of PerfData
 * items without impacting the code that uses the lists.
 *
 * The initial implementation is based upon GrowableArray. Searches
 * on GrowableArray types is linear in nature and this may become
 * a performance issue for creation of PerfData items, particularly
 * from Java code where a test for existence is implemented as a
 * search over all existing PerfData items.
 *
 * The abstraction is not complete. A more general container class
 * would provide an Iterator abstraction that could be used to
 * traverse the lists. This implementation still relys upon integer
 * iterators and the at(int index) method. However, the GrowableArray
 * is not directly visible outside this class and can be replaced by
 * some other implementation, as long as that implementation provides
 * a mechanism to iterate over the container by index.
 */
class PerfDataList : public CHeapObj {

  private:
  
    // GrowableArray implementation
    typedef GrowableArray<PerfData*> PerfDataArray;

    PerfDataArray* _set;

    // methods to support searching and sorting for the
    // Growable Array based implementation.
    static bool by_name(void* name, PerfData* pd);
    static int compare_name_ascending(PerfData** p1, PerfData** p2); 
    static int compare_name_descending(PerfData** p1, PerfData** p2); 

  protected:
    // we expose the implementation here to facilitate the clone
    // method.
    PerfDataArray* get_impl() { return _set; }

  public:
    enum SortOrder {
      ASCENDING, DESCENDING
    };

    // create a PerfDataList with the given initial length
    PerfDataList(int length);

    // create a PerfDataList as a shallow copy of the given PerfDataList
    PerfDataList(PerfDataList* p);

    ~PerfDataList();

    // return the PerfData item indicated by name,
    // or NULL if it doesn't exist.
    PerfData* find_by_name(const char* name);

    // return a PerfDataList of PerfData items whose names match
    // the pattern given in the regex argument.
    // This method is currently unimplemented - need a platform
    // independent implementation.
    PerfDataList* find_by_regex(const char* regex);

    // sort this list as specified by the given SortOrder.
    void sort_by_name(SortOrder s);

    // return true if a PerfData item with the name specified in the
    // argument exists, otherwise return false.
    bool contains(const char* name) { return find_by_name(name) != NULL; }

    // return the number of PerfData items in this list
    int length() { return _set->length(); }

    // add a PerfData item to this list
    void append(PerfData *p) { _set->append(p); }

    // remove the given PerfData item from this list. When called
    // while iterating over the list, this method will result in a
    // change in the length of the container. The at(int index)
    // method is also impacted by this method as elements with an
    // index greater than the index of the element removed by this
    // method will be shifted down by one.
    void remove(PerfData *p) { _set->remove(p); }

    // create a new PerfDataList from this list. The new list is
    // a shallow copy of the original list and care should be taken
    // with respect to delete operations on the elements of the list
    // as the are likely in use by another copy of the list.
    PerfDataList* clone();

    // for backward compatibility with GrowableArray - need to implement
    // some form of iterator to provide a cleaner abstraction for
    // iteration over the container.
    PerfData* at(int index) { return _set->at(index); }
};

/*
 * The PerfDataManager class is responsible for creating PerfData
 * subtypes via a set a factory methods and for managing lists
 * of the various PerfData types.
 */
class PerfDataManager : AllStatic {

  friend class StatSampler;   // for access to protected PerfDataList methods

  private:
    static PerfDataList* _all;
    static PerfDataList* _sampled;
    static PerfDataList* _constants;

    // add a PerfData item to the list(s) of know PerfData objects
    static void add_item(PerfData* p, bool sampled);

  protected:
    // return the list of all known PerfData items
    static PerfDataList* all();
    static int count() { return _all->length(); }

    // return the list of all known PerfData items that are to be
    // sampled by the StatSampler.
    static PerfDataList* sampled();
    static int sampled_count() { return _sampled->length(); }

    // return the list of all known PerfData items that have a
    // variability classification of type Constant
    static PerfDataList* constants();
    static int constants_count() { return _constants->length(); }

  public:

    // method to check for the existence of a PerfData item with
    // the given name.
    static bool exists(const char* name) { return _all->contains(name); }

    // methods to create a counter from a parent name space and a child
    // counter name. This method does not verify the syntactic or semantic
    // structure of either the parent or counter name. It simply concatenates
    // the names together, with a '.' character separating the component
    // parts. These method returns a resource object and must be called
    // within a ResourceMark.
    //
    static char* counter_name(const char* parent, const char* name);
    static char* counter_name(const char* parent, unsigned int name);

    // methods to create a counter name space from a parent name space
    // and a child name space. This method does not verify the syntactic
    // or semantic structure of either the parent or child names. It
    // simply concatenates the names together, with a '.' characters
    // separating the component parts. These method returns a heap
    // allocated character string (which is the only distinct difference
    // between these methods and the counter_name() methods above).
    //
    static char* name_space(const char* parent, const char* child);
    static char* name_space(const char* parent, unsigned int name);

    // these methods provide the general interface for creating
    // performance data resources. The types of performance data
    // resources can be extended by adding additional create<type>
    // methods.

    // Constant Types
    static PerfStringConstant* create_string_constant(const char* name, const char *s, TRAPS);
    static PerfLongConstant* create_long_constant(const char* name, PerfData::Units u, jlong val, TRAPS);

    // Variable Types
    static PerfStringVariable* create_string_variable(const char* name, const char *s, int max_length, TRAPS);
    static PerfStringVariable* create_string_variable(const char* name, const char *s, TRAPS) {
      return create_string_variable(name, s, 0, CHECK_0);
    };

    static PerfLongVariable* create_long_variable(const char* name, PerfData::Units u, jlong ival, TRAPS);
    static PerfLongVariable* create_long_variable(const char* name, PerfData::Units u, TRAPS) {
      return create_long_variable(name, u, (jlong)0, CHECK_0);
    };
    static PerfLongVariable* create_long_variable(const char* name, PerfData::Units u, jlong* sp, TRAPS);
    static PerfLongVariable* create_long_variable(const char* name, PerfData::Units u, PerfLongSampleHelper* sh, TRAPS);

    // Counter Types
    static PerfLongCounter* create_long_counter(const char* name, PerfData::Units u, jlong ival, TRAPS);
    static PerfLongCounter* create_long_counter(const char* name, PerfData::Units u, TRAPS) {
      return create_long_counter(name, u, (jlong)0, CHECK_0);
    };
    static PerfLongCounter* create_long_counter(const char* name, PerfData::Units u, jlong* sp, TRAPS);
    static PerfLongCounter* create_long_counter(const char* name, PerfData::Units u, PerfLongSampleHelper* sh, TRAPS);


    // these creation methods are provided for ease of use. These allow
    // Long performance data types to be created with a shorthand syntax.

    static PerfConstant* create_constant(const char* name, PerfData::Units u, jlong val, TRAPS) {
      return create_long_constant(name, u, val, CHECK_0);
    }

    static PerfVariable* create_variable(const char* name, PerfData::Units u, jlong ival, TRAPS) {
      return create_long_variable(name, u, ival, CHECK_0);
    }

    static PerfVariable* create_variable(const char* name, PerfData::Units u, TRAPS) {
      return create_long_variable(name, u, (jlong)0, CHECK_0);
    }

    static PerfVariable* create_variable(const char* name, PerfData::Units u, jlong* sp, TRAPS) {
      return create_long_variable(name, u, sp, CHECK_0);
    }

    static PerfVariable* create_variable(const char* name, PerfData::Units u, PerfSampleHelper* sh, TRAPS) {
      return create_long_variable(name, u, sh, CHECK_0);
    }

    static PerfCounter* create_counter(const char* name, PerfData::Units u, jlong ival, TRAPS) {
      return create_long_counter(name, u, ival, CHECK_0);
    }

    static PerfCounter* create_counter(const char* name, PerfData::Units u, TRAPS) {
      return create_long_counter(name, u, (jlong)0, CHECK_0);
    }

    static PerfCounter* create_counter(const char* name, PerfData::Units u, jlong* sp, TRAPS) {
      return create_long_counter(name, u, sp, CHECK_0);
    }

    static PerfCounter* create_counter(const char* name, PerfData::Units u, PerfSampleHelper* sh, TRAPS) {
      return create_long_counter(name, u, sh, CHECK_0);
    }

    static void destroy();
};

// Utility Classes

/*
 * this class will administer a PerfCounter used as a time accumulator
 * for a basic block much like the TraceTime class.
 *
 * Example:
 *
 *    static PerfCounter* my_time_counter = PerfDataManager::create_counter("my.time.counter", PerfData::U_Ticks, 0LL, CHECK);
 *
 *    {
 *      PerfTraceTime ptt(my_time_counter);
 *      // perform the operation you want to measure
 *    }
 *
 * Note: use of this class does not need to occur within a guarded
 * block. The UsePerfData guard is used with the implementation
 * of this class.
 */
class PerfTraceTime : public StackObj {

  protected:
    elapsedTimer _t;
    PerfLongCounter* _timerp;

  public:
    inline PerfTraceTime(PerfLongCounter* timerp) : _timerp(timerp) {
      if (!UsePerfData) return;
      _t.start();
    }

    inline void suspend() { if (!UsePerfData) return; _t.stop(); }
    inline void resume() { if (!UsePerfData) return; _t.start(); }

    inline ~PerfTraceTime() {
      if (!UsePerfData) return;
      _t.stop();
      _timerp->inc(_t.ticks());
    }
};

/* The PerfTraceTimedEvent class is responsible for counting the
 * occurrence of some event and measuring the the elapsed time of
 * the event in two separate PerfCounter instances.
 *
 * Example:
 *
 *    static PerfCounter* my_time_counter = PerfDataManager::create_counter("my.time.counter", PerfData::U_Ticks, CHECK);
 *    static PerfCounter* my_event_counter = PerfDataManager::create_counter("my.event.counter", PerfData::U_Events, CHECK);
 *
 *    {
 *      PerfTraceTimedEvent ptte(my_time_counter, my_event_counter);
 *      // perform the operation you want to count and measure
 *    }
 *
 * Note: use of this class does not need to occur within a guarded
 * block. The UsePerfData guard is used with the implementation
 * of this class.
 *
 */
class PerfTraceTimedEvent : public PerfTraceTime {

  protected:
    PerfLongCounter* _eventp;

  public:
    inline PerfTraceTimedEvent(PerfLongCounter* timerp, PerfLongCounter* eventp): PerfTraceTime(timerp), _eventp(eventp) {
      if (!UsePerfData) return;
      _eventp->inc();
    }
};
