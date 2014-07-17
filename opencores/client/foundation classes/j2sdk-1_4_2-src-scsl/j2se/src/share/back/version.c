/*
 * @(#)version.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <string.h>

#include "util.h"
#include "debugInit.h"

#define PROP_VERSION    "java.version"
#define PROP_VM_NAME    "java.vm.name"
#define PROP_VENDOR     "java.vm.vendor"

#define VM_NAME_CLASSIC "Classic VM"
#define VERSION_12      "1.2"
#define VERSION_121     "1.2.1"
#define VENDOR_SUN      "Sun Microsystems Inc."

typedef struct {
    char *name;
    char *value;
} Property;

static Property properties[] = {
    {PROP_VERSION, NULL},
    {PROP_VM_NAME, NULL},
    {PROP_VENDOR,  NULL},
    {NULL,         NULL}
};

jboolean strict;     /* Use strict interpretation of JVMDI */

void
version_initialize()
{
    Property * property = properties;
    while (JNI_TRUE) {
        char *name = property->name;
        if (name != NULL) {
            property->value = getPropertyCString(property->name);
            property++;
        } else {
            break;
        }
    }
    strict = debugInit_isStrict();
}

void
version_reset()
{
}

static char *
findPropertyValue(char *name) 
{
    char *value = NULL;
    Property * property = properties;
    while (JNI_TRUE) {
        if (property->name == NULL) {
            break;
        } else if (strcmp(name, property->name) == 0) {
            value = property->value;
        }
        property++;
    }
    return value;
}

char *
version_vmVersion() 
{
    return findPropertyValue(PROP_VERSION);
}

char *
version_vmName()
{
    return findPropertyValue(PROP_VM_NAME);
}

static jboolean propertyMatches(char *name, char *expectedValue)
{
    char *value = findPropertyValue(name);
    return ((value != NULL) && (strcmp(value, expectedValue) == 0));
}

static jboolean isSunVM(void) 
{
    return propertyMatches(PROP_VENDOR, VENDOR_SUN);
}

static jboolean isClassicVM(void) 
{
    return propertyMatches(PROP_VM_NAME, VM_NAME_CLASSIC);
}

static jboolean isVersion1_2(void)
{
    return propertyMatches(PROP_VERSION, VERSION_12);
}

static jboolean isVersion1_2_1(void)
{
    return propertyMatches(PROP_VERSION, VERSION_121);
}

jboolean
version_supportsEventOrdering()
{
    /*
     * Assume JVMDI event ordering is supported unless using a VM
     * known to have critical bugs in that area.
     */

    /* See bug 4195505 */
    if (!strict && isSunVM() && isClassicVM() && 
        (isVersion1_2() || isVersion1_2_1())) {
        return JNI_FALSE;
    } else {
        return JNI_TRUE;
    }
}


/*
 * A lack of the following features can be worked around easily 
 * in this implementation, so, since there are known bugs in the reference 
 * VM implementations for 1.2, we return false unless in strict mode.
 */
jboolean 
version_supportsClassLoadEvents(void)
{
    return strict;
}

jboolean 
version_supportsFrameCount(void)
{
    return strict;
}

jboolean 
version_supportsPrimitiveClassSignatures(void)
{
    return strict;
}

jboolean 
version_supportsImmediateEventModeSet(void)
{
    return strict;
}

jboolean 
version_supportsMethodEntryLocation(void)
{
    return strict;
}

jboolean 
version_supportsAllocHooks(void)
{
    /*
     * Does it fully support alloc hooks? JNI_FALSE means that 
     * SetAllocationHooks might return NOT_IMPLEMENTED.
     */
    return strict;
}


