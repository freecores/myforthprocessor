package sun.jvm.hotspot.runtime;

public interface ClassConstants
{
    // constant pool constant types - from JVM spec.

    public static final int JVM_CONSTANT_Utf8               = 1;
    public static final int JVM_CONSTANT_Unicode            = 2; // unused
    public static final int JVM_CONSTANT_Integer            = 3;
    public static final int JVM_CONSTANT_Float              = 4;
    public static final int JVM_CONSTANT_Long               = 5;
    public static final int JVM_CONSTANT_Double             = 6;
    public static final int JVM_CONSTANT_Class              = 7;
    public static final int JVM_CONSTANT_String             = 8;
    public static final int JVM_CONSTANT_Fieldref           = 9;
    public static final int JVM_CONSTANT_Methodref          = 10;
    public static final int JVM_CONSTANT_InterfaceMethodref = 11;
    public static final int JVM_CONSTANT_NameAndType        = 12;

    // HotSpot specific constant pool constant types.

    // For bad value initialization
    public static final int JVM_CONSTANT_Invalid            = 0;

    // Temporary tag until actual use
    public static final int JVM_CONSTANT_UnresolvedClass    = 100;

    // Temporary tag while constructing constant pool
    public static final int JVM_CONSTANT_ClassIndex         = 101;

    // Temporary tag until actual use
    public static final int JVM_CONSTANT_UnresolvedString   = 102;

    // Temporary tag while constructing constant pool
    public static final int JVM_CONSTANT_StringIndex        = 103;

    public static final short MAJOR_VERSION = 46;
    public static final short MINOR_VERSION = 0;

    // From jvm.h
    public static final long JVM_ACC_PUBLIC                  = 0x0001;
    public static final long JVM_ACC_PRIVATE                 = 0x0002;
    public static final long JVM_ACC_PROTECTED               = 0x0004;
    public static final long JVM_ACC_STATIC                  = 0x0008;
    public static final long JVM_ACC_FINAL                   = 0x0010;
    public static final long JVM_ACC_SYNCHRONIZED            = 0x0020;
    public static final long JVM_ACC_SUPER                   = 0x0020;
    public static final long JVM_ACC_VOLATILE                = 0x0040;
    public static final long JVM_ACC_TRANSIENT               = 0x0080;
    public static final long JVM_ACC_NATIVE                  = 0x0100;
    public static final long JVM_ACC_INTERFACE               = 0x0200;
    public static final long JVM_ACC_ABSTRACT                = 0x0400;
    public static final long JVM_ACC_STRICT                  = 0x0800;
  
    // from accessFlags.hpp
    public static final long JVM_ACC_WRITTEN_FLAGS           = 0x00001FFF;
    public static final long JVM_ACC_SYNTHETIC               = 0x00002000;
    public static final long JVM_ACC_MONITOR_MATCH           = 0x10000000;
    public static final long JVM_ACC_HAS_MONITOR_BYTECODES   = 0x20000000;
    public static final long JVM_ACC_HAS_LOOPS               = 0x40000000;
    public static final long JVM_ACC_LOOPS_FLAG_INIT         = 0x80000000;
    public static final long JVM_ACC_QUEUED                  = 0x01000000;
    public static final long JVM_ACC_HAS_OTHER_VTABLE_INDEX1 = 0x02000000;
    public static final long JVM_ACC_HAS_OTHER_VTABLE_INDEX2 = 0x04000000;
    public static final long JVM_ACC_NOT_OSR_COMPILABLE      = 0x08000000;
    public static final long JVM_ACC_HAS_LINE_NUMBER_TABLE   = 0x00100000;
    public static final long JVM_ACC_HAS_LOCAL_VARIABLE_TABLE= 0x00200000;
    public static final long JVM_ACC_HAS_CHECKED_EXCEPTIONS  = 0x00400000;
    public static final long JVM_ACC_HAS_MIRANDA_METHODS     = 0x10000000;
    public static final long JVM_ACC_HAS_VANILLA_CONSTRUCTOR = 0x20000000;
    public static final long JVM_ACC_HAS_FINALIZER           = 0x40000000;
    public static final long JVM_ACC_IS_CLONEABLE            = 0x80000000;

    // from jvm.h

    public static final long JVM_RECOGNIZED_CLASS_MODIFIERS   = (JVM_ACC_PUBLIC |
                                        JVM_ACC_FINAL | 
                                        JVM_ACC_SUPER |
                                        JVM_ACC_INTERFACE |
                                        JVM_ACC_ABSTRACT);


    public static final long JVM_RECOGNIZED_FIELD_MODIFIERS  = (JVM_ACC_PUBLIC | 
                                        JVM_ACC_PRIVATE | 
                                        JVM_ACC_PROTECTED | 
                                        JVM_ACC_STATIC | 
                                        JVM_ACC_FINAL |
                                        JVM_ACC_VOLATILE |
                                        JVM_ACC_TRANSIENT);

    public static final long JVM_RECOGNIZED_METHOD_MODIFIERS  = (JVM_ACC_PUBLIC | 
                                         JVM_ACC_PRIVATE | 
                                         JVM_ACC_PROTECTED | 
                                         JVM_ACC_STATIC | 
                                         JVM_ACC_FINAL | 
                                         JVM_ACC_SYNCHRONIZED | 
                                         JVM_ACC_NATIVE | 
                                         JVM_ACC_ABSTRACT | 
                                         JVM_ACC_STRICT);
}
