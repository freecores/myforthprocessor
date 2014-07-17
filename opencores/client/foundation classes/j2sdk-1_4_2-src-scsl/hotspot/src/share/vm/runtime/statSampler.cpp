#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)statSampler.cpp	1.11 03/01/23 12:25:25 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_statSampler.cpp.incl"

// --------------------------------------------------------
// StatSamplerTask

class StatSamplerTask : public PeriodicTask {
  public:
    StatSamplerTask(int interval_time) : PeriodicTask(interval_time) {}
    void task() { StatSampler::collect_sample(); }
};


//----------------------------------------------------------
// Implementation of StatSampler

StatSamplerTask*              StatSampler::_task   = NULL;
PerfDataList*                 StatSampler::_sampled = NULL;

/*
 * the initialize method is called from the engage() method
 * and is responsible for initializing various global variables.
 */
void StatSampler::initialize() {

  if (!UsePerfData) return;

  // create performance data that could not be created prior
  // to vm_init_globals() or otherwise have no logical home.

  create_misc_perfdata();

  // get copy of the sampled list
  _sampled = PerfDataManager::sampled();

}

/*
 * The engage() method is called at initialization time via
 * Thread::create_vm() to initialize the StatSampler and
 * register it with the WatcherThread as a periodic task.
 */
void StatSampler::engage() {

  if (!UsePerfData) return;

  if (!is_active()) {

    initialize();

    // start up the periodic task
    _task = new StatSamplerTask(PerfDataSamplingInterval);
    _task->enroll();    
  }
}


/*
 * the disengage() method is responsible for deactivating the periodic
 * task and, if logging was enabled, for logging the final sample. This
 * method is called from before_exit() in java.cpp and is only called
 * after the WatcherThread has been stopped.
 */
void StatSampler::disengage() {

  if (!UsePerfData) return;

  if (!is_active())
    return;

  // remove StatSamplerTask
  _task->disenroll();
  delete _task;
  _task = NULL;

  // force a final sample
  sample_data(_sampled);
}

/*
 * the destroy method is responsible for releasing any resources used by
 * the StatSampler prior to shutdown of the VM. this method is called from
 * before_exit() in java.cpp and is only called after the WatcherThread
 * has stopped.
 */
void StatSampler::destroy() {

  if (!UsePerfData) return;

  if (_sampled != NULL) {
    delete(_sampled);
    _sampled = NULL;
  }
}

/*
 * The sample_data() method is responsible for sampling the
 * the data value for each PerfData instance in the given list.
 */
void StatSampler::sample_data(PerfDataList* list) {

  assert(list != NULL, "null list unexpected");

  for (int index = 0; index < list->length(); index++) {
    PerfData* item = list->at(index);
    item->sample();
  }
}

/*
 * the collect_sample() method is the method invoked by the
 * WatcherThread via the PeriodicTask::task() method. This method
 * is responsible for collecting data samples from sampled
 * PerfData instances every PerfDataSamplingInterval milliseconds.
 * It is also responsible for logging the requested set of
 * PerfData instances every _sample_count milliseconds. While
 * logging data, it will output a column header after every _print_header
 * rows of data have been logged.
 */
void StatSampler::collect_sample() {

  // future - check for new PerfData objects. PerfData objects might
  // get added to the PerfDataManager lists after we have already
  // built our local copies.
  //
  // if (PerfDataManager::count() > previous) {
  //   // get a new copy of the sampled list
  //   if (_sampled != NULL) {
  //     delete(_sampled);
  //     _sampled = NULL;
  //   }
  //   _sampled = PerfDataManager::sampled();
  // }
  
  assert(_sampled != NULL, "list not initialized");

  sample_data(_sampled);
}

/*
 * method to upcall into Java to return the value of the specified
 * property as a utf8 string, or NULL if does not exist. The caller
 * is responsible for setting a ResourceMark for proper cleanup of
 * the utf8 strings.
 */
const char* StatSampler::get_system_property(const char* name, TRAPS) {

  // setup the arguments to getProperty
  Handle key_str   = java_lang_String::create_from_str(name, CHECK_0);

  // return value
  JavaValue result(T_OBJECT);

  // public static String getProperty(String key, String def);
  JavaCalls::call_static(&result,
                         SystemDictionary::system_klass(),
                         vmSymbolHandles::getProperty_name(),
                         vmSymbolHandles::string_string_signature(),
                         key_str,
                         CHECK_0);

  oop value_oop = (oop)result._value.h;
  if (value_oop == NULL) {
    return NULL;
  }

  // convert Java String to utf8 string
  char* value = java_lang_String::as_utf8_string(value_oop);

  return value;
}

/*
 * The list of System Properties that have corresponding PerfData
 * string instrumentation created by retrieving the named property's
 * value from System.getProperty() and unconditionally creating a
 * PerfStringConstant object initialized to the retreived value. This
 * is not an exhustive list of Java properties with corresponding string
 * instrumentation as the create_system_property_instrumentation() method
 * creates other property based instrumentation conditionally.
 */
static const char* perf_prop_strings[] = {
  "java.vm.specification.version",
  "java.vm.specification.name",
  "java.vm.specification.vendor",
  "java.vm.version",
  "java.vm.name",
  "java.vm.vendor",
  "java.vm.info",
  "java.ext.dirs",
  "java.endorsed.dirs",
  "sun.boot.library.path",
  "java.library.path",
  "java.home",
  "java.class.path",
  NULL
};

/*
 * Method to create PerfData string instruments that contain
 * the values of various system properties. String instruments are
 * unconditionally created for each property specified in the
 * in char* perf_prop_strings[]. This method also conditionally
 * creates string instruments for other system properties.
 */
void StatSampler::create_system_property_instrumentation(TRAPS) {

  ResourceMark rm;

  for (int i = 0; perf_prop_strings[i] != NULL; i++) {

    const char* name = perf_prop_strings[i];
    assert(name != NULL, "property name should not be NULL");

    const char* value = get_system_property(name, CHECK);
    assert(value != NULL, "property name should be valid");

    if (value != NULL) {
      PerfDataManager::create_string_constant(name, value, CHECK);
    }
  }

  // sun.boot.class.path is only created if its value isn't NULL.
  // its value may be NULL if -Xoldjava is set.
  //
  const char* name = "sun.boot.class.path";
  const char* value = get_system_property(name, CHECK);
  if (value != NULL) {
    PerfDataManager::create_string_constant(name, value, CHECK);
  }
}

// Production Performance counter names
#define PERF_HRT       PERF_RUNTIME ".hrt"

/*
 * The create_misc_perfdata() method provides a place to create
 * PerfData instances that would otherwise have no better place
 * to exist.
 */
void StatSampler::create_misc_perfdata() {

  EXCEPTION_MARK;

  // numeric constants

  // frequency of the native high resolution timer
  PerfDataManager::create_constant(PERF_HRT ".frequency",
                                   PerfData::U_Hertz, os::elapsed_frequency(),
                                   CHECK);

  // string constants

  // create string instrumentation for various Java properties.
  create_system_property_instrumentation(CHECK);

  // hotspot flags (from .hotpsotrc) and args (from command line)
  //
  PerfDataManager::create_string_constant(PERF_HOTSPOT ".vm.flags",
                                          Arguments::jvm_flags(), CHECK);
  PerfDataManager::create_string_constant(PERF_HOTSPOT ".vm.args",
                                          Arguments::jvm_args(), CHECK);

  // java class name/jar file and arguments to main class
  // note: name is cooridnated with launcher and Arguments.cpp
  PerfDataManager::create_string_constant("sun.java.command",
                                          Arguments::java_command(), CHECK);

  // the Java VM Internal version string
  PerfDataManager::create_string_constant(PERF_HOTSPOT ".internal.version",
                                         VM_Version::internal_vm_info_string(), 
                                         CHECK);

  // create sampled instrumentation objects
  create_sampled_perfdata();
}

/*
 * helper class to provide for sampling of the elapsed_counter value
 * maintained in the OS class.
 */
class HighResTimeSampler : public PerfSampleHelper {
  public:
    jlong take_sample() { return os::elapsed_counter(); }
};

/*
 * the create_sampled_perdata() method provides a place to instantiate
 * sampled PerfData instances that would otherwise have no better place
 * to exist.
 */
void StatSampler::create_sampled_perfdata() {

  EXCEPTION_MARK;

  // setup sampling of the elapsed time counter maintained in the
  // the os class. This counter can be used as either a time stamp
  // for each logged entry or as a liveness indicator for the VM.
  PerfSampleHelper* psh = new HighResTimeSampler();
  PerfDataManager::create_counter(PERF_HRT ".ticks",
                                  PerfData::U_Ticks, psh, CHECK);
}

/*
 * the statSampler_exit() function is called from os_init.cpp on
 * exit of the vm.
 */
void statSampler_exit() {

  if (!UsePerfData) return;

  StatSampler::destroy();
}

