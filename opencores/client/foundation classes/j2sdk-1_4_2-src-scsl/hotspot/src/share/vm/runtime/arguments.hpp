#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)arguments.hpp	1.55 03/02/20 12:29:09 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Arguments parses the command line and recognizes options

// Invocation API hook typedefs (these should really be defined in jni.hpp)
extern "C" {
  typedef void (JNICALL *abort_hook_t)(void);
  typedef void (JNICALL *exit_hook_t)(jint code);
  typedef jint (JNICALL *vfprintf_hook_t)(FILE *fp, const char *format, va_list args);
}

// Element describing user defined property using -Dkey=value flags

class UserProperty: public CHeapObj {
 private:
  char*         _key;
  char*         _value;
  UserProperty* _next;

 public:
  // Accessors
  char* key() const                         { return _key; }
  char* value() const                       { return _value; }
  UserProperty* next() const                { return _next; }
  void set_next(UserProperty* next)         { _next = next; }

  // Constructor
  UserProperty(char* key, char* value)      { _key = key; _value = value; _next = NULL; }
};

class Arguments : AllStatic {
 public:
  // Operation modi
  enum Mode {
    _int,       // corresponds to -Xint
    _mixed,     // corresponds to -Xmixed
    _comp       // corresponds to -Xcomp
  };

 private:  

  // string containing all flags specified in the .hotspotrc file
  static char* _jvm_flags;
  // string containing all jvm arguments specified in the command line
  static char* _jvm_args;
  // string containing all java command (class/jarfile name and app args)
  static char* _java_command;

  // Property list 
  static UserProperty* _user_properties;

  // Option flags       
  static bool   _has_profile;  
  static bool   _has_alloc_profile;
  static char*  _sysclasspath;
  static char*  _gc_log_filename;
  static uintx  _initial_heap_size;
  static uintx  _max_heap_size;

  // -Xrun arguments
  static bool _init_libraries_at_startup;
  static UserProperty* _libraries;
  static UserProperty* libraries()                    { return _libraries; }
  static void set_libraries(UserProperty* library)    { _libraries = library; }
  static void set_init_libraries_at_startup(bool arg) { _init_libraries_at_startup = arg; }
  static void add_init_library(char* name,char* options);

  // Operation modi
  static Mode _mode;
  static void set_mode_flags(Mode mode);
  static bool _java_compiler;
  static void set_java_compiler(bool arg) { _java_compiler = arg; }
  static bool java_compiler()   { return _java_compiler; }

  // Used to save default settings
  static bool _AlwaysCompileLoopMethods;
  static bool _UseOnStackReplacement;
  static bool _BackgroundCompilation;
  static bool _ClipInlining;
  static bool _CIDynamicCompilePriority;
  static intx _Tier2CompileThreshold;

  // GC processing
  static bool _is_default_new_size;
  static bool _is_default_max_new_size;
  static bool _is_default_old_size;
  static bool _is_default_max_heap_size;
  static bool _is_default_new_ratio;
  // CMS/ParNew garbage collectors
  static bool _is_default_parnew_gc;
  static bool _is_default_max_tenuring_threshold;
  static bool _is_default_survivor_ratio;
  static bool _is_default_parallel_gc_threads;
  static void set_parallel_gc_threads();
  static void set_parnew_gc_flags();
  static void set_cms_and_parnew_gc_flags();
  // UseAdaptiveSizePolicy and UseParallelGC
  static void set_adaptive_size_policy_flags();

  // Invocation API hooks
  static abort_hook_t     _abort_hook;
  static exit_hook_t      _exit_hook;
  static vfprintf_hook_t  _vfprintf_hook;

  // User properties
  static bool add_property(char* prop);

  // UserProptery List manipulation
  static void userPropertyList_add(UserProperty** plist, char* k, char* v);
  static const char* userPropertyList_get_property(UserProperty* plist, const char* key);
  static int userPropertyList_count(UserProperty* pl);
  static char* userPropertyList_name_at(UserProperty* pl,int index);
  static char* userPropertyList_value_at(UserProperty* pl,int index);

  // Argument parsing
  static void do_pd_flag_adjustments();
  static bool parse_argument(char* arg);
  static bool process_argument(char* arg, bool ignore_unrecognized);
  static void process_java_compiler_argument(char* arg);
  static jint parse_java_options_environment_variable();
  static void parse_java_compiler_environment_variable();
  static jint parse_vm_init_args(JavaVMInitArgs* args);
  static bool is_bad_option(JavaVMOption* option, jboolean ignore,
    const char* option_type);
  static bool is_bad_option(JavaVMOption* option, jboolean ignore) {
    return is_bad_option(option, ignore, NULL);
  }

  // methods to build strings from individual args
  static void build_string(char** bldstr, char* arg);
  static void build_jvm_args(char* arg);
  static void build_jvm_flags(char* arg);

  static bool methodExists(
    char* className, char* methodName, 
    int classesNum, char** classes, bool* allMethods,
    int methodsNum, char** methods, bool* allClasses
  );

  static void parseOnlyLine(
    char* line,
    short* classesNum, short* classesMax, char*** classes, bool** allMethods,
    short* methodsNum, short* methodsMax, char*** methods, bool** allClasses
  ); 

  static short	CompileOnlyClassesNum;
  static short	CompileOnlyClassesMax;
  static char**	CompileOnlyClasses;
  static bool*	CompileOnlyAllMethods;

  static short	CompileOnlyMethodsNum;
  static short	CompileOnlyMethodsMax;
  static char**	CompileOnlyMethods;
  static bool*	CompileOnlyAllClasses;
  
  static short	InterpretOnlyClassesNum;
  static short	InterpretOnlyClassesMax;
  static char**	InterpretOnlyClasses;
  static bool*	InterpretOnlyAllMethods;

  static bool	CheckCompileOnly;

 public:
  // Parses the arguments
  static jint parse(JavaVMInitArgs* args);
  // Check consistecy or otherwise of VM argument settings
  static bool check_vm_args_consistency();
  // Used by os_solaris
  static bool process_settings_file(const char* file_name, bool should_exist, bool ignore_unrecognized);

  // return string containing all options 
  static const char* jvm_flags()           { return _jvm_flags; }
  static const char* jvm_args()            { return _jvm_args; }
  static const char* java_command()        { return _java_command; }
    
  // -Dkey=value flags
  static UserProperty*  user_properties()   { return _user_properties; }
  static const char*    get_property(const char* key);

  // -Xsysclasspath, if not speficied default sysclasspath
  static char* sysclasspath()               { return _sysclasspath; }

  // -Xloggc:<file>, if not specified will be NULL
  static char* gc_log_filename()            { return _gc_log_filename; }

  // -Xprof/-Xaprof
  static bool has_profile()                 { return _has_profile; }
  static bool has_alloc_profile()           { return _has_alloc_profile; }

  // -Xms , -Xmx
  static uintx initial_heap_size()          { return _initial_heap_size; }
  static uintx max_heap_size()              { return _max_heap_size; }

  // -Xrun
  static int library_count();
  static char* library_name(int index);
  static char* library_options(int index);
  static bool init_libraries_at_startup()   { return _init_libraries_at_startup; }

  // abort, exit, vfprintf hooks
  static abort_hook_t    abort_hook()          { return _abort_hook; }
  static exit_hook_t     exit_hook()           { return _exit_hook; }
  static vfprintf_hook_t vfprintf_hook()       { return _vfprintf_hook; }

  static bool GetCheckCompileOnly ()	    { return CheckCompileOnly; }

  static bool CompileMethod(char* className, char* methodName) {
    return
      methodExists(
        className, methodName, 
        CompileOnlyClassesNum, CompileOnlyClasses, CompileOnlyAllMethods,
        CompileOnlyMethodsNum, CompileOnlyMethods, CompileOnlyAllClasses
      );
  }

  // Operation modi
  static Mode mode()                        { return _mode; }
};
