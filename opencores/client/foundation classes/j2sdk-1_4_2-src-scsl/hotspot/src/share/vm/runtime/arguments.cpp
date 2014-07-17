#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)arguments.cpp	1.212 03/02/25 17:10:21 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_arguments.cpp.incl"


char*  Arguments::_jvm_flags                    = NULL;
char*  Arguments::_jvm_args                     = NULL;
char*  Arguments::_java_command                 = NULL;
UserProperty* Arguments::_user_properties       = NULL;
char*  Arguments::_sysclasspath                 = NULL;
char*  Arguments::_gc_log_filename              = NULL;
bool   Arguments::_has_profile                  = false;
bool   Arguments::_has_alloc_profile            = false;
uintx  Arguments::_initial_heap_size            = 0;
uintx  Arguments::_max_heap_size                = 0;
Arguments::Mode Arguments::_mode                = _mixed;
bool   Arguments::_java_compiler                = false;

// These parameters are reset in method parse_vm_init_args(JavaVMInitArgs*)
bool   Arguments::_AlwaysCompileLoopMethods     = AlwaysCompileLoopMethods;
bool   Arguments::_UseOnStackReplacement        = UseOnStackReplacement;
bool   Arguments::_BackgroundCompilation        = BackgroundCompilation;
bool   Arguments::_ClipInlining                 = ClipInlining;
intx   Arguments::_Tier2CompileThreshold        = Tier2CompileThreshold;

short  Arguments::CompileOnlyClassesNum	        = 0;
short  Arguments::CompileOnlyClassesMax	        = 0;
char** Arguments::CompileOnlyClasses		= NULL;
bool*  Arguments::CompileOnlyAllMethods	        = NULL;

short  Arguments::CompileOnlyMethodsNum	        = 0;
short  Arguments::CompileOnlyMethodsMax	        = 0;
char** Arguments::CompileOnlyMethods		= NULL;
bool*  Arguments::CompileOnlyAllClasses	        = NULL;

bool   Arguments::CheckCompileOnly              = false;

UserProperty* Arguments::_libraries             = NULL;
bool   Arguments::_init_libraries_at_startup    = false;

abort_hook_t     Arguments::_abort_hook         = NULL;
exit_hook_t      Arguments::_exit_hook          = NULL;
vfprintf_hook_t  Arguments::_vfprintf_hook      = NULL;

// Used by UseParallelGC and UseAdaptiveSizePolicy
bool  Arguments::_is_default_new_size           = true;
bool  Arguments::_is_default_old_size           = true;
bool  Arguments::_is_default_max_heap_size      = true;

// Used by CMS+ParNew garbage collectors
bool  Arguments::_is_default_parnew_gc          = true;
bool  Arguments::_is_default_max_new_size       = true;
bool  Arguments::_is_default_survivor_ratio     = true;
bool  Arguments::_is_default_max_tenuring_threshold = true;
bool  Arguments::_is_default_parallel_gc_threads = true;
bool  Arguments::_is_default_new_ratio          = true;

// Constructs the system class path (aka boot class path) from the following
// components, in order:
// 
//     prefix		// from -Xbootclasspath/p:...
//     endorsed		// the expansion of -Djava.endorsed.dirs=...
//     base		// from os::get_system_properties() or -Xbootclasspath=
//     suffix		// from -Xbootclasspath/a:...
// 
// java.endorsed.dirs is a list of directories; any jar or zip files in the
// directories are added to the sysclasspath just before the base.
//
// This could be AllStatic, but it isn't needed after argument processing is
// complete.
class SysClassPath: public StackObj {
public:
  SysClassPath(const char* base);
  ~SysClassPath();

  inline void set_base(const char* base);
  inline void add_prefix(const char* prefix);
  inline void add_suffix(const char* suffix);
  inline void reset_path(const char* base);

  // Expand the jar/zip files in each directory listed by the java.endorsed.dirs
  // property.  Must be called after all command-line arguments have been
  // processed (in particular, -Djava.endorsed.dirs=...) and before calling
  // combined_path().
  void expand_endorsed();

  inline const char* get_base()     const { return _items[_scp_base]; }
  inline const char* get_prefix()   const { return _items[_scp_prefix]; }
  inline const char* get_suffix()   const { return _items[_scp_suffix]; }
  inline const char* get_endorsed() const { return _items[_scp_endorsed]; }

  // Combine all the components into a single c-heap-allocated string; caller
  // must free the string if/when no longer needed.
  char* combined_path();

private:
  // Utility routines.
  static char* add_to_path(const char* path, const char* str, bool prepend);
  static char* add_jars_to_path(char* path, const char* directory);

  inline void reset_item_at(int index);

  // Array indices for the items that make up the sysclasspath.  All except the
  // base are allocated in the C heap and freed by this class.
  enum {
    _scp_prefix,	// from -Xbootclasspath/p:...
    _scp_endorsed,	// the expansion of -Djava.endorsed.dirs=...
    _scp_base,		// the default sysclasspath
    _scp_suffix,	// from -Xbootclasspath/a:...
    _scp_nitems		// the number of items, must be last.
  };

  const char* _items[_scp_nitems];
  DEBUG_ONLY(bool _expansion_done;)
};

SysClassPath::SysClassPath(const char* base) {
  memset(_items, 0, sizeof(_items));
  _items[_scp_base] = base;
  DEBUG_ONLY(_expansion_done = false;)
}

SysClassPath::~SysClassPath() {
  // Free everything except the base.
  for (int i = 0; i < _scp_nitems; ++i) {
    if (i != _scp_base) reset_item_at(i);
  }
  DEBUG_ONLY(_expansion_done = false;)
}

inline void SysClassPath::set_base(const char* base) {
  _items[_scp_base] = base;
}

inline void SysClassPath::add_prefix(const char* prefix) {
  _items[_scp_prefix] = add_to_path(_items[_scp_prefix], prefix, true);
}

inline void SysClassPath::add_suffix(const char* suffix) {
  _items[_scp_suffix] = add_to_path(_items[_scp_suffix], suffix, false);
}

inline void SysClassPath::reset_item_at(int index) {
  assert(index < _scp_nitems && index != _scp_base, "just checking");
  if (_items[index] != NULL) {
    FREE_C_HEAP_ARRAY(char, _items[index]);
    _items[index] = NULL;
  }
}

inline void SysClassPath::reset_path(const char* base) {
  // Clear the prefix and suffix.
  reset_item_at(_scp_prefix);
  reset_item_at(_scp_suffix);
  set_base(base);
}

//------------------------------------------------------------------------------

void SysClassPath::expand_endorsed() {
  assert(_items[_scp_endorsed] == NULL, "can only be called once.");

  const char* path = Arguments::get_property("java.endorsed.dirs");
  if (path == NULL) {
    path = os::get_system_properties()->endorsed_dirs;
    assert(path != NULL, "no default for java.endorsed.dirs");
  }

  char* expanded_path = NULL;
  const char separator = *os::path_separator();
  const char* const end = path + strlen(path);
  while (path < end) {
    const char* tmp_end = strchr(path, separator);
    if (tmp_end == NULL) {
      expanded_path = add_jars_to_path(expanded_path, path);
      path = end;
    } else {
      char* dirpath = NEW_C_HEAP_ARRAY(char, tmp_end - path + 1);
      memcpy(dirpath, path, tmp_end - path);
      dirpath[tmp_end - path] = '\0';
      expanded_path = add_jars_to_path(expanded_path, dirpath);
      FREE_C_HEAP_ARRAY(char, dirpath);
      path = tmp_end + 1;
    }
  }
  _items[_scp_endorsed] = expanded_path;
  DEBUG_ONLY(_expansion_done = true;)
}

// Combine the bootclasspath elements, some of which may be null, into a single
// c-heap-allocated string.
char* SysClassPath::combined_path() {
  assert(_items[_scp_base] != NULL, "empty default sysclasspath");
  assert(_expansion_done, "must call expand_endorsed() first.");

  size_t lengths[_scp_nitems];
  size_t total_len = 0;

  const char separator = *os::path_separator();

  // Get the lengths.
  int i;
  for (i = 0; i < _scp_nitems; ++i) {
    if (_items[i] != NULL) {
      lengths[i] = strlen(_items[i]);
      // Include space for the separator char (or a NULL for the last item).
      total_len += lengths[i] + 1;
    }
  }
  assert(total_len > 0, "empty sysclasspath not allowed");

  // Copy the _items to a single string.
  char* cp = NEW_C_HEAP_ARRAY(char, total_len);
  char* cp_tmp = cp;
  for (i = 0; i < _scp_nitems; ++i) {
    if (_items[i] != NULL) {
      memcpy(cp_tmp, _items[i], lengths[i]);
      cp_tmp += lengths[i];
      *cp_tmp++ = separator;
    }
  }
  *--cp_tmp = '\0';	// Replace the extra separator.
  return cp;
}

// Note:  path must be c-heap-allocated (or NULL); it is freed if non-null.
char*
SysClassPath::add_to_path(const char* path, const char* str, bool prepend) {
  char *cp;

  assert(str != NULL, "just checking");
  if (path == NULL) {
    size_t len = strlen(str) + 1;
    cp = NEW_C_HEAP_ARRAY(char, len);
    memcpy(cp, str, len);			// copy the trailing null
  } else {
    const char separator = *os::path_separator();
    size_t old_len = strlen(path);
    size_t str_len = strlen(str);
    size_t len = old_len + str_len + 2;

    if (prepend) {
      cp = NEW_C_HEAP_ARRAY(char, len);
      char* cp_tmp = cp;
      memcpy(cp_tmp, str, str_len);
      cp_tmp += str_len;
      *cp_tmp = separator;
      memcpy(++cp_tmp, path, old_len + 1);	// copy the trailing null
      FREE_C_HEAP_ARRAY(char, path);
    } else {
      cp = REALLOC_C_HEAP_ARRAY(char, path, len);
      char* cp_tmp = cp + old_len;
      *cp_tmp = separator;
      memcpy(++cp_tmp, str, str_len + 1);	// copy the trailing null
    }
  }
  return cp;
}

// Scan the directory and append any jar or zip files found to path.
// Note:  path must be c-heap-allocated (or NULL); it is freed if non-null.
char* SysClassPath::add_jars_to_path(char* path, const char* directory) {
  DIR* dir = os::opendir(directory);
  if (dir == NULL) return path;
  
  char dir_sep[2] = { '\0', '\0' };
  size_t directory_len = strlen(directory);
  const char fileSep = *os::file_separator();
  if (directory[directory_len - 1] != fileSep) dir_sep[0] = fileSep;
    
  /* Scan the directory for jars/zips, appending them to path. */ 
  struct dirent *entry;
  char *dbuf = NEW_C_HEAP_ARRAY(char, os::readdir_buf_size(directory));
  while ((entry = os::readdir(dir, (dirent *) dbuf)) != NULL) {
    const char* name = entry->d_name;
    const char* ext = name + strlen(name) - 4;
    bool isJarOrZip = ext > name &&
      (os::file_name_strcmp(ext, ".jar") == 0 ||
       os::file_name_strcmp(ext, ".zip") == 0);
    if (isJarOrZip) {
      char* jarpath = NEW_C_HEAP_ARRAY(char, directory_len + 2 + strlen(name));
      sprintf(jarpath, "%s%s%s", directory, dir_sep, name);
      path = add_to_path(path, jarpath, false);
      FREE_C_HEAP_ARRAY(char, jarpath);
    }
  }
  FREE_C_HEAP_ARRAY(char, dbuf);
  os::closedir(dir);
  return path;
}

// Parses a memory size specification string.
static bool atomll(char *s, jlong* result) {
  jlong n = 0;
  int args_read = sscanf(s, os::jlong_format_specifier(), &n);
  if (args_read != 1) {
    return false;
  }
  while (*s != '\0' && isdigit(*s)) {
    s++;
  }
  // 4705540: illegal if more characters are found after the first non-digit
  if (strlen(s) > 1) {
    return false;
  }
  switch (*s) {
    case 'T': case 't':
      *result = n * G * K;
      return true;
    case 'G': case 'g':
      *result = n * G;
      return true;
    case 'M': case 'm':
      *result = n * M;
      return true;
    case 'K': case 'k':
      *result = n * K;
      return true;
    case '\0':
      *result = n;
      return true;
    default:
      return false;
  }
}


bool check_memory_size(jlong size, jlong min_size) {
  if (size < min_size) return false;
  // Check that size will fit in a size_t (only relevant on 32-bit)
  if ((julong) size > max_uintx) return false;
  return true;
}


static bool set_bool_flag(char* name, bool value) {
  return DebugFlags::boolAtPut(name, &value);
}


static bool set_numeric_flag(char* name, char* value) {
  jlong v;
  if (!atomll(value, &v)) {
    return false;
  }
  intx intx_v = (intx) v;
  if (DebugFlags::intxAtPut(name, &intx_v)) {
    return true;
  }
  uintx uintx_v = (uintx) v;
  if (DebugFlags::uintxAtPut(name, &uintx_v)) {
    return true;
  }
  return false;
}


static bool set_string_flag(char* name, const char* value) {
  if (!DebugFlags::ccstrAtPut(name, &value))  return false;
  // Contract:  DebugFlags always returns a pointer that needs freeing.
  FREE_C_HEAP_ARRAY(char, value);
  return true;
}

static bool append_to_string_flag(char* name, const char* new_value) {
  const char* old_value = "";
  if (!DebugFlags::ccstrAt(name, &old_value))  return false;
  size_t old_len = strlen(old_value);
  size_t new_len = strlen(new_value);
  const char* value;
  char* free_this_too = NULL;
  if (old_len == 0) {
    value = new_value;
  } else if (new_len == 0) {
    value = old_value;
  } else {
    char* buf = NEW_C_HEAP_ARRAY(char, old_len + 1 + new_len + 1);
    // each new setting adds another LINE to the switch:
    sprintf(buf, "%s\n%s", old_value, new_value);
    value = buf;
    free_this_too = buf;
  }
  (void) DebugFlags::ccstrAtPut(name, &value);
  // DebugFlags always returns a pointer that needs freeing.
  FREE_C_HEAP_ARRAY(char, value);
  if (free_this_too != NULL) {
    // DebugFlags made its own copy, so I must delete my own temp. buffer.
    FREE_C_HEAP_ARRAY(char, free_this_too);
  }
  return true;
}


void Arguments::parseOnlyLine (char* line, 
  short* classesNum, short* classesMax, char*** classes, bool** allMethods,
  short* methodsNum, short* methodsMax, char*** methods, bool** allClasses) {

  int i;
  char name[1024];
  bool className = true;	// current string is class name.
  bool addedClass = false;	
  bool addedMethod = false;

  while (*line != '\0') {
    for (i = 0 ; i < 1024 && *line != '\0' && *line != '.' && *line != ',' && !isspace(*line); line++, i++)
      name[i] = *line;

    if (i > 0) {
      char* newName = NEW_C_HEAP_ARRAY( char, i + 1);
      if (newName == NULL)
        return;
      strncpy( newName, name, i);
      newName[i] = '\0';

      if (className) {
        addedClass = true;
        if (*classesNum == *classesMax) {
          *classesMax += 5;
          *classes = REALLOC_C_HEAP_ARRAY( char*, *classes, *classesMax);
          *allMethods = REALLOC_C_HEAP_ARRAY( bool, *allMethods, *classesMax);
          if (*classes == NULL || *allMethods == NULL)
            return;
        }
        (*classes)[*classesNum] = newName;
        (*allMethods)[*classesNum] = false;
        (*classesNum)++;
      }
      else {
        addedMethod = true;
        if (*methodsNum == *methodsMax) {
          *methodsMax += 5;

          *methods = REALLOC_C_HEAP_ARRAY( char*, *methods, *methodsMax);
          *allClasses = REALLOC_C_HEAP_ARRAY( bool, *allClasses, *methodsMax);
          if (*methods == NULL || *allClasses == NULL)
            return;
        }
        (*methods)[*methodsNum] = newName;
        (*allClasses)[*methodsNum] = false;
        (*methodsNum)++;
      }
    }

    if (*line == '.')
      className = false;

    if (*line == ',' || isspace(*line) || *line == '\0') {
      if (addedClass && !addedMethod) {
        (*allMethods)[*classesNum - 1] = true;
      }
      if (addedMethod && !addedClass) {
        (*allClasses)[*methodsNum - 1] = true;
      }
      className = true;
      addedClass = false;
      addedMethod = false;
    }

    line = *line == '\0' ? line : line + 1;
  }
}

bool Arguments::parse_argument(char* arg) {

  // range of acceptable characters spelled out for portability reasons
  char name[256];
  #define NAME_RANGE  "[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]"

  if (sscanf(arg, "-%" NAME_RANGE, name) == 1) {
    return set_bool_flag(name, false);
  }
  if (sscanf(arg, "+%" NAME_RANGE, name) == 1) {
    return set_bool_flag(name, true);
  }

  char punct;
  if (sscanf(arg, "%" NAME_RANGE "%c", name, &punct) == 2 && punct == '=') {
    char* value = strchr(arg, '=') + 1;
    // Note that normal -XX:Foo=WWW accumulates.
    bool success = append_to_string_flag(name, value);

    if (success && strcmp(name, "CompileOnly") == 0) {
      // Record the classes and methods that should always be compiled.
      // %%% This stuff belongs in the CompilerOracle, not here.
      parseOnlyLine(value,
                    &CompileOnlyClassesNum, &CompileOnlyClassesMax, &CompileOnlyClasses, &CompileOnlyAllMethods,
                    &CompileOnlyMethodsNum, &CompileOnlyMethodsMax, &CompileOnlyMethods, &CompileOnlyAllClasses);
      if (CompileOnlyClassesNum > 0 || CompileOnlyMethodsNum > 0) {
        CheckCompileOnly = true;
      }
    }

    if (success)  return success;
  }

  if (sscanf(arg, "%" NAME_RANGE ":%c", name, &punct) == 2 && punct == '=') {
    char* value = strchr(arg, '=') + 1;
    // -XX:Foo:=xxx will reset the string flag to the given value.
    return set_string_flag(name, value);
  }

  char value[256];
  #define VALUE_RANGE "[-kmgtKMGT0123456789]"
  if (sscanf(arg, "%" NAME_RANGE "=" "%" VALUE_RANGE, name, value) == 2) 
    return set_numeric_flag(name, value);

  return false;
}


void Arguments::build_string(char** bldstr, char* arg) {
  
  assert(bldstr != NULL, "illegal argument");

  if (arg == NULL) {
    return;
  }

  if (*bldstr == NULL) {
     // allocate enough space for the new build string to hold
     // the given argument string and a null terminator
     *bldstr = NEW_C_HEAP_ARRAY(char, strlen(arg) + 1);
     **bldstr = '\0';
  }
  else {
     // allocate enough space for the new build string to hold
     // the existing build string, the argument string, a space
     // to separate the existing string and the new argument,
     // and the null terminator
     size_t new_length = strlen(*bldstr) + strlen(arg) + 2;
     *bldstr = REALLOC_C_HEAP_ARRAY(char, *bldstr, new_length);
  }

  if (strlen(*bldstr) > 0)
    // append a space character onto the existing build string
    strcat(*bldstr, " ");

  // append the argument string onto the build string
  strcat(*bldstr, arg);
}

void Arguments::build_jvm_args(char* arg) {
  build_string(&_jvm_args, arg);
}

void Arguments::build_jvm_flags(char* arg) {
  build_string(&_jvm_flags, arg);
}

bool Arguments::process_argument(char* arg, bool ignore_unrecognized) {

  if (parse_argument(arg)) {
    if (PrintVMOptions) {
      jio_fprintf(stdout, "VM option '%s'\n", arg);
    }
  } else {
    if (!ignore_unrecognized) {
      jio_fprintf(stderr, "Unrecognized VM option '%s'\n", arg);
      // allow for commandline "commenting out" options like -XX:#+Verbose
      if (strlen(arg) == 0 || arg[0] != '#') {
        return false;
      }
    }
  }
  return true;
}


bool Arguments::process_settings_file(const char* file_name, bool should_exist, bool ignore_unrecognized) {
  FILE* stream = fopen(file_name, "rb");
  if (stream == NULL) {
    if (should_exist) {
      jio_fprintf(stderr, "Could not open settings file %s\n", file_name);
      return false;
    } else {
      return true;
    }
  }

  char token[1024];
  int  pos = 0;

  bool in_white_space = true;
  bool in_comment     = false;
  bool in_quote       = false;
  char quote_c        = 0;
  bool result         = true;

  int c = getc(stream);
  while(c != EOF) {
    if (in_white_space) {
      if (in_comment) {
	if (c == '\n') in_comment = false;
      } else {
        if (c == '#') in_comment = true;
        else if (!isspace(c)) {
          in_white_space = false;
	  token[pos++] = c;
        }
      }
    } else {
      if (c == '\n' || (!in_quote && isspace(c))) {
	// token ends at newline, or at unquoted whitespace
	// this allows a way to include spaces in string-valued options
        token[pos] = '\0';
        result &= process_argument(token, ignore_unrecognized);
        build_jvm_flags(token);
	pos = 0;
	in_white_space = true;
	in_quote = false;
      } else if (!in_quote && (c == '\'' || c == '"')) {
	in_quote = true;
	quote_c = c;
      } else if (in_quote && (c == quote_c)) {
	in_quote = false;
      } else {
        token[pos++] = c;
      }
    }
    c = getc(stream);
  }
  if (pos > 0) {
    token[pos] = '\0';
    result &= process_argument(token, ignore_unrecognized);
    build_jvm_flags(token);
  }
  fclose(stream);
  return result;
}

//=============================================================================================================
// Parsing of properties (-D) 

const char* Arguments::get_property(const char* key) {
  return userPropertyList_get_property(user_properties(), key);
}

bool Arguments::add_property(char* prop) {
  char* eq = strchr(prop, '=');
  char* key;
  // ns must be static--its address may be stored in a UserProperty object.
  static char ns[1] = {0};
  char* value = ns;
  if (eq == NULL) {
    key = prop;
  } else {
    int key_len = eq - prop;
    key = AllocateHeap(key_len + 1, "add_property");
    strncpy(key, prop, key_len);
    key[key_len] = '\0';

    int value_len = (int)strlen(prop) - key_len - 1;
    value = AllocateHeap(value_len + 1, "add_property");
    strncpy(value, &prop[key_len + 1], value_len + 1);    
  }

  if (strcmp(key, "java.compiler") == 0) {
    process_java_compiler_argument(value);
    return true; //FIXME note: value (and sometimes key) are not FreeHeap'd...
  }
  else if (strcmp(key, "sun.java.command") == 0) {

    _java_command = value;

    // don't add this property to the properties exposed to the java application
    return true;
  }

  // Create new property and add at the end of the list
  userPropertyList_add(&_user_properties, key, value);
  return true;
}

//===========================================================================================================
// Setting int/mixed/comp mode flags 

void Arguments::set_mode_flags(Mode mode) {
  // Set up default values for all flags.
  // If you add a flag to any of the branches below,
  // add a default value for it here.
  set_java_compiler(false);
  _mode                      = mode;
  UseInterpreter             = true;
  UseCompiler                = true;
  UseLoopCounter             = true;

  // Default values may be platform/compiler dependent -
  // use the saved values
  ClipInlining               = Arguments::_ClipInlining;
  AlwaysCompileLoopMethods   = Arguments::_AlwaysCompileLoopMethods;
  UseOnStackReplacement      = Arguments::_UseOnStackReplacement;
  BackgroundCompilation      = Arguments::_BackgroundCompilation;
  Tier2CompileThreshold      = Arguments::_Tier2CompileThreshold;

  // Change from defaults based on mode
  switch (mode) {
  default:
    ShouldNotReachHere();
    break;
  case _int:
    UseCompiler              = false;
    UseLoopCounter           = false;
    AlwaysCompileLoopMethods = false;
    UseOnStackReplacement    = false;
    break;
  case _mixed:
    // same as default
    break;
  case _comp:
    UseInterpreter           = false;
    BackgroundCompilation    = false;
    ClipInlining             = false;
    Tier2CompileThreshold    = 1000;
    break;
  }
}

// Set of flags used for UseAdaptiveSizePolicy
void Arguments::set_adaptive_size_policy_flags() {
  if (!UseParallelGC || !UseAdaptiveSizePolicy) return;

  // Initial values per Adaptive policy design spec.
  // ms = 8MB,  initial NewSize = 4MB
  if (_is_default_new_size) {
    NewSize = 4 * M;
  }
  if (_is_default_old_size) {
    OldSize = 4 * M;
  }
  if (_is_default_max_heap_size) {
    // mx = max (16MB, half of physical pages) 
    julong total_memory = os::physical_memory();
    julong max_heap     = os::allocatable_physical_memory(total_memory);
    MaxHeapSize = MAX2((julong)16 * M, max_heap/(julong)2);
  }
}

// If the user has chosen ParallelGCThreads > 0, we set UseParNewGC
// if it's not explictly set or unset. If the use has chosen
// UseParNewGC and not explicitly set ParallelGCThreads we 
// set it as in set_parallel_gc_threads, except that we don't
// UseParNewGC if we are on a single cpu machine.
void Arguments::set_parnew_gc_flags() {
  assert(!UseParallelGC && !UseTrainGC, "control point invariant");

  if (_is_default_parnew_gc && ParallelGCThreads > 1) {
    UseParNewGC = true;
  } else if (UseParNewGC && ParallelGCThreads == 0) {
    set_parallel_gc_threads();
    if (_is_default_parallel_gc_threads && ParallelGCThreads == 1) {
      UseParNewGC = false;
    }
  }
  if (!UseParNewGC) {
    ParallelGCThreads = 0;
  }
}

// CAUTION: this code is currently shared by UseParallelGC, UseParNewGC and
// UseconcMarkSweepGC. Further tuning of individual collectors might 
// dictate refinement on a per-collector basis.
void Arguments::set_parallel_gc_threads() {
  if (_is_default_parallel_gc_threads) {
    // For very large machines, there are diminishing returns
    // for large numbers of worker threads.  Instead of
    // hogging the whole system, use 5/8ths of a worker for every
    // processor after the first 8.  For example, on a 72 cpu
    // machine use 8 + (72 - 8) * (5/8) == 48 worker threads.
    // This is just a start and needs further tuning and study in
    // Tiger.
    int ncpus = os::active_processor_count();
    ParallelGCThreads = (ncpus <= 8) ? ncpus : 3 + ((ncpus * 5) / 8);
  }
}

// Adjust some sizes to suit CMS and/or ParNew needs; these work well on
// sparc/solaris for certain applications, but would gain from
// further optimization and tuning efforts, and would almost
// certainly gain from analysis of platform and environment.
void Arguments::set_cms_and_parnew_gc_flags() {
  if (UseParallelGC || UseTrainGC) {
    return;
  }

  // If we are using CMS, we prefer to UseParNewGC,
  // unless explicitly forbidden.
  if (UseConcMarkSweepGC && !UseParNewGC && _is_default_parnew_gc) {
    UseParNewGC = true;
  }

  // In either case, adjust ParallelGCThreads and/or UseParNewGC
  // as needed.
  set_parnew_gc_flags();

  if (!UseConcMarkSweepGC) {
    return;
  }

  // Now make adjustments for CMS

  // Preferred young gen size for "short" pauses
  size_t preferred_max_new_size =
    4 * M * (ParallelGCThreads == 0 ? 1 : ParallelGCThreads);

  // Unless explicitly requested otherwise, size young gen
  // for "short" pauses ~ 4M*ParallelGCThreads
  if (_is_default_max_new_size) {  // MaxNewSize not set at command-line
    if (!_is_default_new_size) {   // NewSize explicitly set at command-line
      MaxNewSize = MAX2(NewSize, preferred_max_new_size);
    } else {
      MaxNewSize = preferred_max_new_size;
    }
  }
  size_t min_new  = 4*M;
  size_t max_heap = _max_heap_size > 0 ? _max_heap_size :
                    align_size_down(MaxHeapSize,
                                    CardTableRS::ct_max_alignment_constraint());
                    // MaxHeapSize is aligned down in collectorPolicy
  if (max_heap > min_new) {
    // Unless explicitly requested otherwise, make young gen
    // at least min_new, and at most preferred_max_new_size.
    if (_is_default_new_size) {
      NewSize = MAX2(NewSize, min_new);
      NewSize = MIN2(preferred_max_new_size, NewSize);
    }
    // Unless explicitly requested otherwise, size old gen
    // so that it's at least 3X of NewSize to begin with;
    // later NewRatio will decide how it grows; see below.
    if (_is_default_old_size) {
      if (max_heap > NewSize) {
        OldSize = MIN2(3*NewSize,  max_heap - NewSize);
      }
    }
  }
  // Unless explicitly requested otherwise, prefer a large
  // Old to Young gen size so as to shift the collection load
  // to the old generation concurrent collector
  if (_is_default_new_ratio) {
    NewRatio = MAX2(NewRatio, (intx)15);
  }
  // Unless explicitly requested otherwise, prefer to
  // promote all objects surviving a scavenge
  if (_is_default_max_tenuring_threshold) {
    MaxTenuringThreshold = 0;
  }
  // Unless explicitly requested otherwise, prefer
  // small survivor spaces, since we prefer to promote
  // all objects surviving a scavenge
  if (_is_default_survivor_ratio || MaxTenuringThreshold == 0) {
    SurvivorRatio = 1024;
  }
}


//===========================================================================================================
// Parsing of java.compiler property and JAVA_COMPILER environment variable

void Arguments::process_java_compiler_argument(char* arg) {
  // Ignore java.compiler property and JAVA_COMPILER environment variable. 
  // It will cause the java.lang.Compiler static initializer to try to dynamically 
  // link the dll, which is not going to work since HotSpot does not support the 
  // old JIT interface. 
  // For backwards compatibility, we switch to interpreted mode if "NONE" or "" is 
  // specified.
  if (strlen(arg) == 0 || strcasecmp(arg, "NONE") == 0) {
    set_java_compiler(true);    // "-Djava.compiler[=...]" most recently seen.
  }
}

void Arguments::parse_java_compiler_environment_variable() {
  char buffer[64];
  if (os::getenv("JAVA_COMPILER", buffer, sizeof(buffer))) {
	process_java_compiler_argument(buffer);
  }
}


//===========================================================================================================
// Parsing of main arguments

bool Arguments::methodExists (char* className, char* methodName, 
  int classesNum, char** classes, bool* allMethods,
  int methodsNum, char** methods, bool* allClasses) {

  int i;
  bool classExists = false, methodExists = false;

  for (i = 0; i < classesNum; i++)
    if (strstr( className, classes[i]) != NULL) {
      if (allMethods[i])
        return true;
      classExists = true;
    }

  for (i = 0; i < methodsNum; i++)
    if (strcmp( methods[i], methodName) == 0) {
      if (allClasses[i])
        return true;
      methodExists = true;
    }

  return classExists && methodExists;
}


// Check if head of 'option' matches 'name', and sets 'tail' remaining part of option string

static bool match_option(JavaVMOption *option, const char* name, char** tail) {  
  int len = (int)strlen(name);
  if (!strncmp(option->optionString, name, len)) {
    *tail = option->optionString + len;
    return true;
  } else {
    return false;
  }
}

// Check the consistency of vm_init_args
bool Arguments::check_vm_args_consistency() {
  // Method for adding checks for flag consistency.
  // The intent is to warn the user of all possible conflicts,
  // before returning an error.
  // Note: Needs platform-dependent factoring.
  bool status = true;
  
  #if ( (defined(WIN32) || defined(LINUX)))
  if (UseISM || UsePermISM) {
    jio_fprintf(stderr, "Large pages not supported on this OS.\n");    
    status = false;
  }
  // MPSS has been set to true as default so we need to turn it off
  // for windows and linux.  We could have set the default to false 
  // and turned on the flag for Solaris but then for whatever reason,
  // there wouldn't be a mechanism to turn off MPSS for Solaris.
  UseMPSS = false;
  #endif 

  // In all cases, ISM has precedence over MPSS
  if (UseISM || UsePermISM) {
    UseMPSS = false;
  }

  #if ( (defined(COMPILER2) && defined(SPARC)))
  VM_Version::initialize();
  if (!VM_Version::has_v9()) {
    jio_fprintf(stderr, "V8 Machine detected, Server requires V9\n");
    status = false;
  }
  #endif

  // Allow both -XX:-UseStackBanging and -XX:-UseBoundThreads in non-product
  // builds so the cost of stack banging can be measured.
  #if (defined(PRODUCT) && defined(SOLARIS))
  if (!UseBoundThreads && !UseStackBanging) {
    jio_fprintf(stderr, "-UseStackBanging conflicts with -UseBoundThreads\n");
     
     status = false;
  }
  #endif

  if (MaxLiveObjectEvacuationRatio > 100) {
    jio_fprintf(stderr,
                "MaxLiveObjectEvacuationRatio should be a percentage "
                "between 0 and 100, "
                "not " SIZE_FORMAT "\n",
                MaxLiveObjectEvacuationRatio);
    status = false;
  }
  if (MaxTLABRatio == 0) {
    jio_fprintf(stderr,
                "MaxTLABRatio should be a denominator,"
                " not " SIZE_FORMAT "\n",
                MaxTLABRatio);
    status = false;
  }
  if (TLABThreadRatio == 0) {
    jio_fprintf(stderr,
                "TLABThreadRatio should be a denominator, "
                "not " SIZE_FORMAT "\n",
                TLABThreadRatio);
    status = false;
  }
  if (TLABFragmentationRatio == 0) {
    jio_fprintf(stderr,
                "TLABFragmentationRatio should be a denominator, "
                "not " SIZE_FORMAT "\n",
                TLABFragmentationRatio);
    status = false;
  }
  if (AdaptiveSizePolicyWeight > 100) {
    jio_fprintf(stderr,
                "AdaptiveSizePolicyWeight should be between 0 and 100, "
                "not " SIZE_FORMAT "\n",
                AdaptiveSizePolicyWeight);
    status = false;
  }
  if (AdaptivePermSizeWeight > 100) {
    jio_fprintf(stderr,
                "AdaptivePermSizeWeight should be between 0 and 100, "
                "not " SIZE_FORMAT "\n",
                AdaptivePermSizeWeight);
    status = false;
  }
  if (ThresholdTolerance > 100) {
    jio_fprintf(stderr,
                "ThresholdTolerance should be between 0 and 100, "
                "not " SIZE_FORMAT "\n",
                ThresholdTolerance);
    status = false;
  }

  // Ensure that the user has not selected conflicting sets
  // of collectors. [Note: this check is merely a user convenience;
  // collectors over-ride each other so that only a non-conflicting
  // set is selected; however what the user gets is not what they
  // may have expected from the combination they asked for. It's
  // better to reduce user confusion by not allowing them to
  // select conflicting combinations.
  uint i = 0;
  if (UseConcMarkSweepGC || UseParNewGC) i++;
  if (UseTrainGC)                        i++;
  if (UseParallelGC)                     i++;
  if (i > 1) {
    jio_fprintf(stderr,
                "Conflicting collector combinations in option list; "
                "please refer to the release notes for the combinations "
                "allowed\n");
    status = false;
  }

  if (CMSIncrementalMode) {
    if (!UseConcMarkSweepGC) {
      jio_fprintf(stderr, "error:  invalid argument combination.\n"
		  "The CMS collector (-XX:+UseConcMarkSweepGC) must be "
		  "selected in order\nto use CMSIncrementalMode.\n");
      status = false;
    } else if (!UseTLAB) {
      jio_fprintf(stderr, "error:  CMSIncrementalMode requires thread-local "
		  "allocation buffers\n(-XX:+UseTLAB).\n");
      status = false;
    } else {
      guarantee(CMSIncrementalDutyCycle <= 100,
		"invalid value for CMSIncrementalDutyCycle");
      guarantee(CMSIncrementalDutyCycleMin <= 100,
		"invalid value for CMSIncrementalDutyCycleMin");
      guarantee(CMSIncrementalSafetyFactor <= 100,
		"invalid value for CMSIncrementalSafetyFactor");
      guarantee(CMSIncrementalOffset <= 100,
		"invalid value for CMSIncrementalOffset");
      guarantee(CMSExpAvgFactor <= 100,
		"invalid value for CMSExpAvgFactor");
      // If it was not set on the command line, set
      // CMSInitiatingOccupancyFraction to 1 so icms can initiate cycles early.
      if (CMSInitiatingOccupancyFraction < 0) {
	CMSInitiatingOccupancyFraction = 1;
      }
    }
  }

  #ifndef PRODUCT
  // CMS space iteration, which FLSVerifyAllHeapreferences entails,
  // insists that we hold the requisite locks so that the iteration is
  // MT-safe. For the verification at start-up and shut-down, we don't
  // yet have a good way of acquiring and releasing these locks,
  // which are not visible at the CollectedHeap level. We want to
  // be able to acquire these locks and then do the iteration rather
  // than just disable the lock verification. This will be fixed under
  // bug 4788986.
  if (UseConcMarkSweepGC && FLSVerifyAllHeapReferences) {
    if (VerifyGCStartAt == 0) {
      warning("Heap verification at start-up disabled "
              "(due to current incompatibility with FLSVerifyAllHeapReferences)");
      VerifyGCStartAt = 1;      // Disable verification at start-up
    }
    if (VerifyBeforeExit) {
      warning("Heap verification at shutdown disabled "
              "(due to current incompatibility with FLSVerifyAllHeapReferences)");
      VerifyBeforeExit = false; // Disable verification at shutdown
    }
  }
  #endif // !PRODUCT

  #ifndef PRODUCT
  // CMS space iteration, which FLSVerifyAllHeapreferences entails,
  // insists that we hold the requisite locks so that the iteration is
  // MT-safe. For the verification at start-up and shut-down, we don't
  // yet have a good way of acquiring and releasing these locks,
  // which are not visible at the CollectedHeap level. We want to
  // be able to acquire these locks and then do the iteration rather
  // than just disable the lock verification. This will be fixed under
  // bug 4788986.
  if (UseConcMarkSweepGC && FLSVerifyAllHeapReferences) {
    if (VerifyGCStartAt == 0) {
      warning("Heap verification at start-up disabled "
              "(due to current incompatibility with FLSVerifyAllHeapReferences)");
      VerifyGCStartAt = 1;      // Disable verification at start-up
    }
    if (VerifyBeforeExit) {
      warning("Heap verification at shutdown disabled "
              "(due to current incompatibility with FLSVerifyAllHeapReferences)");
      VerifyBeforeExit = false; // Disable verification at shutdown
    }
  }
  #endif // !PRODUCT

  return status;
}

bool Arguments::is_bad_option(JavaVMOption* option, jboolean ignore,
  const char* option_type) {
  if (ignore) return false;

  const char* spacer = " ";
  if (option_type == NULL) {
    option_type = ++spacer; // Set both to the empty string.
  }

  if (os::obsolete_option(option)) {
    jio_fprintf(stderr, "Obsolete %s%soption: %s\n", option_type, spacer,
      option->optionString);
    return false;
  } else {
    jio_fprintf(stderr, "Unrecognized %s%soption: %s\n", option_type, spacer,
      option->optionString);
    return true;
  }
}

static const char* user_assertion_options[] = {
  "-da", "-ea", "-disableassertions", "-enableassertions", 0
};

static const char* system_assertion_options[] = {
  "-dsa", "-esa", "-disablesystemassertions", "-enablesystemassertions", 0
};

// Return true if any of the strings in null-terminated array 'names' matches.
// If tail_allowed is true, then the tail must begin with a colon; otherwise,
// the option must match exactly.
static bool match_option(JavaVMOption* option, const char** names, char** tail,
  bool tail_allowed) {
  for (/* empty */; *names != NULL; ++names) {
    if (match_option(option, *names, tail)) {
      if (**tail == '\0' || tail_allowed && **tail == ':') {
	return true;
      }
    }
  }
  return false;
}

// Parse JavaVMInitArgs structure

jint Arguments::parse_vm_init_args(JavaVMInitArgs* args) {
  // Remaining part of option string
  char* tail;

  // For components of the system classpath.
  SysClassPath scp(_sysclasspath);
  bool scp_assembly_required = false;

  // Save default settings for some mode flags
  Arguments::_AlwaysCompileLoopMethods = AlwaysCompileLoopMethods;
  Arguments::_UseOnStackReplacement    = UseOnStackReplacement;
  Arguments::_ClipInlining             = ClipInlining;
  Arguments::_BackgroundCompilation    = BackgroundCompilation;
  Arguments::_Tier2CompileThreshold    = Tier2CompileThreshold;

  // iterate over arguments  
  for (int index = 0; index < args->nOptions; index++) {

    JavaVMOption* option = args->options + index;    

    if (!match_option(option, "-Djava.class.path", &tail) &&
        !match_option(option, "-Dsun.java.command", &tail)) { 

        // add all jvm options to the jvm_args string. This string
        // is used later to set the java.vm.args PerfData string constant.
        // the -Djava.class.path and the -Dsun.java.command options are
        // omitted from jvm_args string as each have their own PerfData
        // string constant object.
	build_jvm_args(option->optionString);
    }

    // -verbose:[class/gc/jni]
    if (match_option(option, "-verbose", &tail)) {
      if (!strcmp(tail, ":class") || !strcmp(tail, "")) {
        TraceClassLoading = true;
        TraceClassUnloading = true;
      } else if (!strcmp(tail, ":gc")) {
        PrintGC = true;
        TraceClassUnloading = true;
      } else if (!strcmp(tail, ":jni")) {
        PrintJNIResolving = true;
      }    
    // -da / -ea / -disableassertions / -enableassertions
    // These accept an optional class/package name separated by a colon, e.g.,
    // -da:java.lang.Thread.
    } else if (match_option(option, user_assertion_options, &tail, true)) {
      bool enable = option->optionString[1] == 'e';	// char after '-' is 'e'
      if (*tail == '\0') {
	JavaAssertions::setUserClassDefault(enable);
      } else {
	assert(*tail == ':', "bogus match by match_option()");
	JavaAssertions::addOption(tail + 1, enable);
      }
    // -dsa / -esa / -disablesystemassertions / -enablesystemassertions
    } else if (match_option(option, system_assertion_options, &tail, false)) {
      bool enable = option->optionString[1] == 'e';	// char after '-' is 'e'
      JavaAssertions::setSystemClassDefault(enable);
    // -bootclasspath:
    } else if (match_option(option, "-Xbootclasspath:", &tail)) {
      scp.reset_path(tail);
      scp_assembly_required = true;
    // -bootclasspath/a:
    } else if (match_option(option, "-Xbootclasspath/a:", &tail)) {
      scp.add_suffix(tail);
      scp_assembly_required = true;
    // -bootclasspath/p:
    } else if (match_option(option, "-Xbootclasspath/p:", &tail)) {
      scp.add_prefix(tail);
      scp_assembly_required = true;
    } else if (match_option(option, "-Xrun", &tail)) {
      char* name = tail;
      char* options = strchr(name,':');
      if (options != NULL) {
        *options = 0;
        options++;
      }

      if(name != NULL) {
        name = strcpy(NEW_C_HEAP_ARRAY(char,strlen(name) + 1),name);
        if(options != NULL) {
          options = strcpy(NEW_C_HEAP_ARRAY(char,strlen(options) + 1),options);
        }
        add_init_library(name,options);
      }
    // -Xnoclassgc
    } else if (match_option(option, "-Xnoclassgc", &tail)) {
      ClassUnloading = false;
    // -Xincgc
    } else if (match_option(option, "-Xincgc", &tail)) {
      UseTrainGC = true;
    // -Xnoincgc
    } else if (match_option(option, "-Xnoincgc", &tail)) {
      UseTrainGC = false;
    // -Xconcgc
    } else if (match_option(option, "-Xconcgc", &tail)) {
      UseConcMarkSweepGC = true;
    // -Xnoconcgc
    } else if (match_option(option, "-Xnoconcgc", &tail)) {
      UseConcMarkSweepGC = false;
    // -XX:+UseParNewGC
    } else if (match_option(option, "-XX:+UseParNewGC", &tail)) {
      UseParNewGC = true;
      _is_default_parnew_gc = false;
    // -XX:-UseParNewGC
    } else if (match_option(option, "-XX:-UseParNewGC", &tail)) {
      UseParNewGC = false;
      _is_default_parnew_gc = false;
    // -Xbatch
    } else if (match_option(option, "-Xbatch", &tail)) {
      BackgroundCompilation = false;
    // -Xmn for compatibility with other JVM vendors
    } else if (match_option(option, "-Xmn", &tail)) {
      jlong long_initial_eden_size = 0;
      if (!atomll(tail, &long_initial_eden_size) || !check_memory_size(long_initial_eden_size, 1)) {
        jio_fprintf(stderr, "Invalid initial eden size: %s\n", option->optionString);
        return JNI_EINVAL;
      }
      MaxNewSize = (size_t) long_initial_eden_size;
      NewSize = (size_t) long_initial_eden_size;
      _is_default_new_size = false;
    // -Xms
    } else if (match_option(option, "-Xms", &tail)) {
      jlong long_initial_heap_size = 0;
      if (!atomll(tail, &long_initial_heap_size) || !check_memory_size(long_initial_heap_size, 1)) {
        jio_fprintf(stderr, "Invalid initial heap size: %s\n", option->optionString);
        return JNI_EINVAL;
      }
      _initial_heap_size = (size_t) long_initial_heap_size;
    // -Xmx
    } else if (match_option(option, "-Xmx", &tail)) {
      jlong long_max_heap_size = 0;
      if (!atomll(tail, &long_max_heap_size) || !check_memory_size(long_max_heap_size, 1)) {
        jio_fprintf(stderr, "Invalid maximum heap size: %s\n", option->optionString);
        return JNI_EINVAL;
      }
      _max_heap_size = (size_t) long_max_heap_size;
      _is_default_max_heap_size = false;
    // Xmaxf
    } else if (match_option(option, "-Xmaxf", &tail)) {
      int maxf = atof(tail) * 100;
      if (maxf < 0 || maxf > 100) {
        jio_fprintf(stderr, "Bad max heap free percentage size: %s\n", option->optionString);
        return JNI_EINVAL;
      } else {
        MaxHeapFreeRatio = maxf;
      }
    // Xminf
    } else if (match_option(option, "-Xminf", &tail)) {
      int minf = atof(tail) * 100;
      if (minf < 0 || minf > 100) {
        jio_fprintf(stderr, "Bad min heap free percentage size: %s\n", option->optionString);
        return JNI_EINVAL;
      } else {
        MinHeapFreeRatio = minf;
      }
    // -Xss
    } else if (match_option(option, "-Xss", &tail)) {
      jlong long_ThreadStackSize = 0;
      if (!atomll(tail, &long_ThreadStackSize) || !check_memory_size(long_ThreadStackSize, 1000)) { 
        jio_fprintf(stderr, "Invalid thread stack size: %s\n", option->optionString);
        return JNI_EINVAL;
      }
      ThreadStackSize = (int) long_ThreadStackSize;
      // Internally track ThreadStackSize in units of 1024 bytes, 2^10.
      ThreadStackSize = round_to(ThreadStackSize, K) >> 10;
    // -Xoss
    } else if (match_option(option, "-Xoss", &tail)) {
	  // HotSpot does not have separate native and Java stacks, ignore silently for compatibility
    // -Xmaxjitcodesize
    } else if (match_option(option, "-Xmaxjitcodesize", &tail)) {
      jlong long_ReservedCodeCacheSize = 0;
      if (!atomll(tail, &long_ReservedCodeCacheSize) || !check_memory_size(long_ReservedCodeCacheSize, InitialCodeCacheSize)) {
        jio_fprintf(stderr, "Invalid maximum code cache size: %s\n", option->optionString);
        return JNI_EINVAL;
      }
      ReservedCodeCacheSize = (int) long_ReservedCodeCacheSize;
    // -green
    } else if (match_option(option, "-green", &tail)) {
      jio_fprintf(stderr, "Green threads support not available\n");
	  return JNI_EINVAL;
    // -native
    } else if (match_option(option, "-native", &tail)) {
	  // HotSpot always uses native threads, ignore silently for compatibility
    // -Xsqnopause
    } else if (match_option(option, "-Xsqnopause", &tail)) {
	  // EVM option, ignore silently for compatibility
    // -Xrs
    } else if (match_option(option, "-Xrs", &tail)) {
	  // Classic/EVM option, new functionality
      ReduceSignalUsage = true;
    } else if (match_option(option, "-Xusealtsigs", &tail)) {
          // change default internal VM signals used - lower case for back compat
      UseAltSigs = true;
    // -Xoptimize
    } else if (match_option(option, "-Xoptimize", &tail)) {
	  // EVM option, ignore silently for compatibility
    // -Xprof
    } else if (match_option(option, "-Xprof", &tail)) {
      _has_profile = true;
    // -Xaprof
    } else if (match_option(option, "-Xaprof", &tail)) {
      _has_alloc_profile = true;
    // -Xconcurrentio
    } else if (match_option(option, "-Xconcurrentio", &tail)) {
      UseLWPSynchronization = true;
      BackgroundCompilation = false;
      DeferThrSuspendLoopCount = 1;
      UseTLAB = false;
      NewSizeThreadIncrease  = 16 * K;  // 20Kb per thread added to new generation

      // -Xinternalversion
    } else if (match_option(option, "-Xinternalversion", &tail)) {
      jio_fprintf(stdout, "%s\n", VM_Version::internal_vm_info_string());
      vm_exit(0);
#ifndef PRODUCT
    // -Xprintflags
    } else if (match_option(option, "-Xprintflags", &tail)) {
      DebugFlags::printFlags();
      vm_exit(0);
#endif
    // -D
    } else if (match_option(option, "-D", &tail)) {      
      if (!add_property(tail)) {
        return JNI_ENOMEM;
      }
    // -Xint
    } else if (match_option(option, "-Xint", &tail)) {
	  set_mode_flags(_int);
    // -Xmixed
    } else if (match_option(option, "-Xmixed", &tail)) {
	  set_mode_flags(_mixed);
    // -Xcomp
    } else if (match_option(option, "-Xcomp", &tail)) {
      // for testing the compiler; turn off all flags that inhibit compilation
	  set_mode_flags(_comp);
    // -Xverify
    } else if (match_option(option, "-Xverify", &tail)) {      
      if (strcmp(tail, ":all") == 0 || strcmp(tail, "") == 0) {
        BytecodeVerificationLocal  = true;
        BytecodeVerificationRemote = true;
      } else if (strcmp(tail, ":remote") == 0) {
        BytecodeVerificationLocal  = false;
        BytecodeVerificationRemote = true;
      } else if (strcmp(tail, ":none") == 0) {
        BytecodeVerificationLocal  = false;
        BytecodeVerificationRemote = false;
      } else if (is_bad_option(option, args->ignoreUnrecognized, "verification")) {
	return JNI_EINVAL;
      }
    // -Xdebug
    } else if (match_option(option, "-Xdebug", &tail)) {
      const char* mode;
      jvmdi::enable();
      if (*tail == '\0') {
        // NOTE: currently a little more work is needed to make
        // mixed-mode debugging work with C2 (see below as well)
        NOT_CORE( mode = ":fast");
        CORE_ONLY(mode = ":int");
      } else {
        assert(*tail == ':', "bogus match by match_option()");
        mode = (const char*)tail;
      }
#ifdef CORE
      if (strcmp(mode, ":fast") == 0) {
        warning("-Xdebug:fast not supported in CORE builds");
        mode = ":int";
      }
#endif /* CORE */
      UseFastEmptyMethods = false;
      UseFastAccessorMethods = false;
      if (strcmp(mode, ":int") == 0) {
        set_mode_flags(_int);
      } else if (strcmp(mode, ":fast") == 0) {
        // use default mode flags (can be overridden by explicit mode flags)
      } else if (is_bad_option(option, args->ignoreUnrecognized, 
                                     "debug")) {
        return JNI_EINVAL;
      }
    // -Xnoagent 
    } else if (match_option(option, "-Xnoagent", &tail)) {    
      // For compatibility with classic. HotSpot refuses to load the old style agent.dll.
    } else if (match_option(option, "-Xboundthreads", &tail)) {    
      // Bind user level threads to kernel threads (Solaris only)
      UseBoundThreads = true;
    } else if (match_option(option, "-Xloggc:", &tail)) {
      // Redirect GC output to the file. -Xloggc:<filename>
      // ostream_init_log(), when called will use this filename
      // to initialize a fileStream.
      _gc_log_filename = tail;
      PrintGC = true;
      PrintGCTimeStamps = true;
      TraceClassUnloading = true;

    // JNI hooks
    } else if (match_option(option, "-Xcheck", &tail)) {    
      if (!strcmp(tail, ":jni")) {
        CheckJNICalls = true;
      } else if (is_bad_option(option, args->ignoreUnrecognized, 
                                     "check")) {
        return JNI_EINVAL;
      }
    } else if (match_option(option, "vfprintf", &tail)) {    
      _vfprintf_hook = CAST_TO_FN_PTR(vfprintf_hook_t, option->extraInfo);
    } else if (match_option(option, "exit", &tail)) {    
      _exit_hook = CAST_TO_FN_PTR(exit_hook_t, option->extraInfo);
    } else if (match_option(option, "abort", &tail)) {    
      _abort_hook = CAST_TO_FN_PTR(abort_hook_t, option->extraInfo);
    // -XX:+AggressiveHeap
    } else if (match_option(option, "-XX:+AggressiveHeap", &tail)) {

      // This option inspects the machine and attempts to set various
      // parameters to be optimal for long-running, memory allocation
      // intensive jobs.  It is intended for machines with large
      // amounts of cpu and memory.

      // initHeapSize is needed since _initial_heap_size is 4 bytes on a 32 bit
      // VM, but we may not be able to represent the total physical memory
      // available (like having 8gb of memory on a box but using a 32bit VM).
      // Thus, we need to make sure we're using a julong for intermediate
      // calculations.
      julong initHeapSize;
      julong total_memory = os::physical_memory();

      if (total_memory < (julong)256*M) {
        jio_fprintf(stderr, "You need at least 256mb of memory to use -XX:+AggressiveHeap\n");
        vm_exit(1);
      }

      // The heap size is half of available memory, or (at most)
      // all of possible memory less 160mb (leaving room for the OS
      // when using ISM).  This is the maximum; because adaptive sizing
      // is turned on below, the actual space used may be smaller.
      initHeapSize = MIN2(total_memory / (julong)2,
                          total_memory - (julong)160*M);

      // Make sure that if we have a lot of memory we cap the 32 bit
      // process space.  The 64bit VM version of this function is a nop.
      initHeapSize = os::allocatable_physical_memory(initHeapSize);
      if (_is_default_max_heap_size) {
         _max_heap_size = initHeapSize;
         _initial_heap_size = _max_heap_size;
         _is_default_max_heap_size = false;
      }
      if (_is_default_new_size) {
         // Make the young generation 3/8ths of the total heap.
         NewSize = ((julong)_max_heap_size / (julong)8) * (julong)3;
         MaxNewSize = NewSize;
         _is_default_new_size = false;
      }

      // Enable TLABs
      UseTLAB = true;

      // Increase some data structure sizes for efficiency
      BaseFootPrintEstimate = _max_heap_size;
      ResizeTLAB = false;
      TLABSize = 256*K;
      YoungPLABSize = 256*K;      // Note: this is in words

      // Enable parallel GC and adaptive generation sizing
      UseParallelGC = true;
      UseAdaptiveSizePolicy = true;

      set_parallel_gc_threads();

      // Encourage steady state memory management
      ThresholdTolerance = 100;

      // This appears to improve mutator locality
      ScavengeBeforeFullGC = false;

    } else if (match_option(option, "-XX:+NeverTenure", &tail)) {    
      // The last option must always win.
      AlwaysTenure = false;
      NeverTenure = true;
    } else if (match_option(option, "-XX:+AlwaysTenure", &tail)) {    
      // The last option must always win.
      NeverTenure = false;
      AlwaysTenure = true;
    } else if (match_option(option, "-XX:-UseParallelGC", &tail)) {    
      // We need to make sure the parallel thread count is reset to zero
      UseParallelGC = false;
      ParallelGCThreads = 0;
    } else if (match_option(option, "-XX:ParallelGCThreads", &tail)) {
      _is_default_parallel_gc_threads = false;
      //  Back up so we can use normal argument processing
      match_option(option, "-XX:", &tail);
      if (!process_argument(tail, args->ignoreUnrecognized)) {
        return JNI_EINVAL;
      }
    } else if (match_option(option, "-XX:NewRatio", &tail)) {
      _is_default_new_ratio = false;
      //  Back up so we can use normal argument processing
      match_option(option, "-XX:", &tail);
      if (!process_argument(tail, args->ignoreUnrecognized)) {
        return JNI_EINVAL;
      }
    } else if (match_option(option, "-XX:NewSize", &tail)) {
      _is_default_new_size = false;
      // Back up so we can use normal argument processing
      match_option(option, "-XX:", &tail);
      if (!process_argument(tail, args->ignoreUnrecognized)) {
        return JNI_EINVAL;
      }
    } else if (match_option(option, "-XX:OldSize", &tail)) {
      _is_default_old_size = false;
      // Back up so we can use normal argument processing
      match_option(option, "-XX:", &tail);
      if (!process_argument(tail, args->ignoreUnrecognized)) {
        return JNI_EINVAL;
      }
    } else if (match_option(option, "-XX:MaxHeapSize", &tail)) {
      _is_default_max_heap_size = false;
      // Back up so we can use normal argument processing
      match_option(option, "-XX:", &tail);
      if (!process_argument(tail, args->ignoreUnrecognized)) {
        return JNI_EINVAL;
      }
    // The TLE options are for compatibility with 1.3 and will be
    // removed without notice in a future release.  These options
    // are not to be documented.
    } else if (match_option(option, "-XX:MaxTLERatio=", &tail)) {
      MaxTLABRatio = atof(tail);
    } else if (match_option(option, "-XX:+ResizeTLE", &tail)) {
      ResizeTLAB = true;
    } else if (match_option(option, "-XX:-ResizeTLE", &tail)) {
      ResizeTLAB = false;
    } else if (match_option(option, "-XX:+PrintTLE", &tail)) {
      PrintTLAB = true;
    } else if (match_option(option, "-XX:-PrintTLE", &tail)) {
      PrintTLAB = false;
    } else if (match_option(option, "-XX:TLEFragmentationRatio=", &tail)) {
      TLABFragmentationRatio = atof(tail);
    } else if (match_option(option, "-XX:TLESize=", &tail)) {
      jlong long_tlab_size = 0;
      if (!atomll(tail, &long_tlab_size) || !check_memory_size(long_tlab_size, 1)) {
        jio_fprintf(stderr, "Invalid TLAB size: %s\n", option->optionString);
        return JNI_EINVAL;
      }
      TLABSize = long_tlab_size;
    } else if (match_option(option, "-XX:TLEThreadRatio=", &tail)) {
      TLABThreadRatio = atof(tail);
    } else if (match_option(option, "-XX:+UseTLE", &tail)) {
      UseTLAB = true;
    } else if (match_option(option, "-XX:-UseTLE", &tail)) {
      UseTLAB = false;
    } else if (match_option(option, "-XX:", &tail)) { // -XX:xxxx
      // Skip -XX:Flags= since that case has already been handled
      if (strncmp(tail, "Flags=", strlen("Flags=")) != 0) {
        if (!process_argument(tail, args->ignoreUnrecognized)) {
          return JNI_EINVAL;
        }
      }
    // Unknown option
    } else if (is_bad_option(option, args->ignoreUnrecognized)) {
      return JNI_ERR;
    }
  }
  // This must be done after all -D arguments have been processed.
  scp.expand_endorsed();
  scp_assembly_required |= scp.get_endorsed() != NULL;

  if (scp_assembly_required) {
    // Assemble the bootclasspath elements into the final path.
    _sysclasspath = scp.combined_path();
  }

  // This must be done after all arguments have been processed.
  if (java_compiler() && !jvmdi::enabled()) {
    // For backwards compatibility, we switch to interpreted mode if
    // -Djava.compiler="NONE" or "" is specified AND "-Xdebug" was
    // not specified.
    set_mode_flags(_int);
  }

  if (!check_vm_args_consistency())
     return JNI_ERR;
  return JNI_OK;
}


jint Arguments::parse_java_options_environment_variable() {
  char buffer[1024];
  if (os::getenv("_JAVA_OPTIONS", buffer, sizeof(buffer))) {
    const int N_MAX_OPTIONS = 32;
    // Construct JavaVMOption array
    JavaVMOption options[N_MAX_OPTIONS];
    jio_fprintf(stderr, "Picked up _JAVA_OPTIONS: %s\n", buffer);
    char* c = buffer;
    int i;
    for (i = 0; i < N_MAX_OPTIONS;) {
      // Skip whitespace
      while (isspace(*c)) c++;
      if (*c == 0) break;
      // Fill in option
      options[i++].optionString = c;
      while (*c != 0 && !isspace(*c)) c++;
      // Check for end
      if (*c == 0) break;
      // Zero terminate option
      *c++ = 0;
    }
    // Construct JavaVMInitArgs structure
    JavaVMInitArgs vm_args;
    vm_args.version = JNI_VERSION_1_2;
    vm_args.options = options;
    vm_args.nOptions = i;
    vm_args.ignoreUnrecognized = false;
    return(parse_vm_init_args(&vm_args));
  }
  return JNI_OK;
}


// Parse entry point called from JNI_CreateJavaVM

jint Arguments::parse(JavaVMInitArgs* args) {
  // Set default sysclasspath
  assert(_sysclasspath == NULL, "Should only parse arguments once");
  _sysclasspath = os::get_system_properties()->sysclasspath;

  // Remaining part of option string
  char* tail;
  
  // If flag "-XX:Flags=flags-file" is used it will be the first option to be processed.
  bool settings_file_specified = false;
  for (int index = 0; index < args->nOptions; index++) {
    JavaVMOption *option = args->options + index;
    if (match_option(option, "-XX:Flags=", &tail)) {
      if (!process_settings_file(tail, true, args->ignoreUnrecognized)) {
        return JNI_EINVAL;
      }
      settings_file_specified = true;
    } 
  }

  // Parse default .hotspotrc settings file
  if (!settings_file_specified) {
    if (!process_settings_file(".hotspotrc", false, args->ignoreUnrecognized)) {
      return JNI_EINVAL;
    }
  }

  // Parse JavaVMInitArgs structure passed in
  jint result = parse_vm_init_args(args);
  if (result != JNI_OK) {
    return result;
  }

  // Parse _JAVA_OPTIONS environment variable (if present) (mimics classic VM)
  result = parse_java_options_environment_variable();
  if (result != JNI_OK) {
    return result;
  }

  // Parse JAVA_COMPILER environment variable (if present) (mimics classic VM)
  parse_java_compiler_environment_variable();

#ifndef PRODUCT
  if (TraceBytecodesAt != 0) {
    TraceBytecodes = true;
  }
  if (CountCompiledCalls) {
#ifdef CORE
    warning("CountCalls is set but may not work (no invocation counters in CORE build)");
#else
    if (UseCounterDecay) {
      warning("UseCounterDecay disabled because CountCalls is set");
      UseCounterDecay = false;
    }
#endif // CORE
  }
  
  if (PrintBCIHistogram) {
    if (_mode != _int) 
      warning("PrintBCIHistogram is set, but mode is not interpreter-only.\n"
              "Compiled bci's will not be recorded (use -Xint to fix mode)");
    if (ClassUnloading) {
      warning("Disabling ClassUnloading in order to print bci histogram");
      ClassUnloading = false;
    }
  }
#endif // PRODUCT

  // Interactions between breakpoints and bytecode rewriting.
  if (jvmdi::enabled() && UseFastBreakpoints) {
    RewriteFrequentPairs = false;
  }

  // We are debugging and using the compiler aka full-speed debugging.
  // Test is done at this point since -Xint, -Xmixed, -Xcomp, can override.
  // In the future FullSpeedJVMDI should be replaced by this test
  if (jvmdi::enabled() && UseCompiler) {
    FullSpeedJVMDI = true;
  }

  if (!RewriteBytecodes) {
    RewriteFrequentPairs = false;
  }

  if (PrintGCDetails) {
    // Turn on -verbose:gc options as well
    PrintGC = true;
    TraceClassUnloading = true;
  }

  // Set flags for adaptive size policy (currently only UseParallelGC)
  set_adaptive_size_policy_flags();

  // Set some flags for CMS and/or ParNew collectors, as needed.
  set_cms_and_parnew_gc_flags();

  // For extra robustness
  CORE_ONLY(set_mode_flags(_int));
  return JNI_OK;
}

// %%% Should be Arguments::GetCheckCompileOnly(), Arguments::CompileMethod().

bool CheckCompileOnly () { return Arguments::GetCheckCompileOnly();}

bool CompileMethod (char* className, char* methodName) {
  return Arguments::CompileMethod( className, methodName);
}

int Arguments::library_count() {
  return userPropertyList_count(_libraries);
}

char* Arguments::library_name(int index) {
  return userPropertyList_name_at(_libraries, index);
}

char* Arguments::library_options(int index) {
  return userPropertyList_value_at(_libraries, index);
}

void Arguments::add_init_library(char* name, char* options) {

  userPropertyList_add(&_libraries, name, options);

  set_init_libraries_at_startup(true);

}

int Arguments::userPropertyList_count(UserProperty* pl) {
  int count = 0;
  while(pl != NULL) {
    count++;
    pl = pl->next();
  }
  return count;
}

const char* Arguments::userPropertyList_get_property(UserProperty *pl, const char* key) {
  assert(key != NULL, "just checking");
  assert(pl != NULL, "just checking");
  UserProperty* prop;
  for (prop = pl; prop != NULL; prop = prop->next()) {
    if (strcmp(key, prop->key()) == 0) return prop->value();
  }
  return NULL;
}

char* Arguments::userPropertyList_name_at(UserProperty *pl, int index) {
  int count = 0;
  char* ret_val = NULL;

  while(pl != NULL) {
    if(count >= index) {
      ret_val = pl->key();
      break;
    }
    count++;
    pl = pl->next();
  }

  return ret_val;
}

char* Arguments::userPropertyList_value_at(UserProperty* pl, int index) {
  int count = 0;
  char* ret_val = NULL;

  while(pl != NULL) {
    if(count >= index) {
      ret_val = pl->value();
      break;
    }
    count++;
    pl = pl->next();
  }

  return ret_val;
}

void Arguments::userPropertyList_add(UserProperty** plist, char* k, char* v) {

  if (plist == NULL)
    return;

  UserProperty* new_p = new UserProperty(k, v);
  UserProperty* p = *plist;
  if (p == NULL) {
    *plist = new_p;
  } else {
    while (p->next() != NULL) {
      p = p->next();
    }
    p->set_next(new_p);
  }
}

