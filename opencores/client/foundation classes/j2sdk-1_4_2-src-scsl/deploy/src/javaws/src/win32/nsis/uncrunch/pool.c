/*
 * @(#)pool.c	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Support for both global and local constant pools.\
 *
 * The global constant pool is populated once.
 * The local constant pools are cleaned and regenerated for
 * each classfile.
 *
 *							KGH Nov 98
 */

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "crunch.h"

//----------------------------------------------------------------------
// the global constant pools.

// The raw data of all the pools.
uchar *raw_pool;

// The number of entries in each of the global pools.
int utf_pool_size;
int class_pool_size;
int ref_pool_size;
int type_pool_size;
int value_pool_size;

// The give global pools
uchar **utf_pool;     		// points to UTF encoded values
uchar **class_pool;   		// points to UTF encoded values
uchar *ref_pool;     		// points to info on references
uchar **type_pool;    		// points to UTF encoded values
uchar **value_pool;   		// points to assorted constants

//----------------------------------------------------------------------

void
read_global_pools()
{
    int total_length, i;
    uchar *poolp;

    // Read the overal length of the five global pools.
    total_length = read_swapped_int();

    // Read the sizes of each of the global pools.
    utf_pool_size = read_swapped_short();
    class_pool_size = read_swapped_short();
    ref_pool_size = read_swapped_short();
    type_pool_size = read_swapped_short();
    value_pool_size = read_swapped_short();

    raw_pool = malloc(total_length);
    if (raw_pool == 0) {
	fprintf(stderr, "Couldn't allocate %d byte for pools\n", total_length);
	fflush(stderr);
	exit(5);
    }
    poolp = raw_pool;

    // printf("UTF pool size = %d\n", utf_pool_size);
    // printf("CLASS pool size = %d\n", class_pool_size);
    // printf("REF pool size = %d\n", ref_pool_size);
    // printf("TYPE pool size = %d\n", type_pool_size);
    // printf("VALUE pool size = %d\n", value_pool_size);

    // printf("Reading %d bytes of pool\n", total_length);
    // fflush(stdout);

    read_fully(raw_pool, total_length);

    // printf("done read\n");
    // fflush(stdout);

    // Allocate the indexes for the four string tables.
    utf_pool = calloc(utf_pool_size, sizeof(uchar *));
    class_pool = calloc(class_pool_size, sizeof(uchar *));
    type_pool = calloc(type_pool_size, sizeof(uchar *));
    value_pool = calloc(value_pool_size, sizeof(uchar *));
    if (utf_pool == 0 || class_pool == 0 || type_pool == 0 || value_pool == 0) {
	fprintf(stderr, "Couldn't allocate space for global pools\n");
	exit(6);
    }

    // Setup the utf pool.
    utf_pool[0] = 0;
    for (i = 1; i < utf_pool_size; i++) {
	ushort len = strlen(poolp);
	utf_pool[i] = poolp;
	poolp += (1 + len);
    }

    // Setup the class pool.
    class_pool[0] = 0;
    for (i = 1; i < class_pool_size; i++) {
	ushort len;
	class_pool[i] = poolp;
	if (*poolp == '%') {
	    poolp += 3;
	}
	len = strlen(poolp);
	poolp += (1 + len);
    }

    // The ref_pool simply points at the raw data.
    // We have to offset for the missing entry at zero.
    ref_pool = (poolp - 6);
    poolp = poolp + (6 * (ref_pool_size-1));

    // Setup the type pool.
    type_pool[0] = 0;
    for (i = 1; i < type_pool_size; i++) {
	type_pool[i] = poolp;
	for (;;) {
	    char ch = *poolp++;
	    if (ch == 0) {
		break;
	    }
	    if (ch == 'L') {
		poolp += 2;
	    }
	}
    }

    // Setup the value pool.
    value_pool[0] = 0;
    for (i = 1; i < value_pool_size; i++) {
	ushort len;
	uchar tag = *poolp;
	value_pool[i] = poolp;
	switch (tag) {
	  case CONSTANT_INTEGER:
	    poolp += 5;
	    break;
	  case CONSTANT_FLOAT:
	    poolp += 5;
	    break;
	  case CONSTANT_LONG:
	    poolp += 9;
	    break;
	  case CONSTANT_DOUBLE:
	    poolp += 9;
	    break;
	  case CONSTANT_STRING:
	    len = strlen(poolp+1);
	    poolp += (len + 2);
	    break;
	  default:
	    fprintf(stderr, "Unexpected tag %d in value pool\n", tag);
	    exit(4);
	}
    }

    if (poolp != (raw_pool + total_length)) {
	fprintf(stderr, "Size mismatch when processing global pools\n");
    }
    // printf("Setup global pools OK\n");
}

//----------------------------------------------------------------------
// Local constant pool and supporting hashtable.

// These are used as part of our hashtable keys.
#define UTF_KIND   0x10000
#define CLASS_KIND 0x20000
#define REF_KIND   0x30000
#define TYPE_KIND  0x40000
#define VALUE_KIND 0x50000

#define LOCAL_POOL_SIZE 200000
static uchar local_pool[LOCAL_POOL_SIZE];
static uchar *local_pool_limit = local_pool + LOCAL_POOL_SIZE;
static uchar *local_pool_ptr;
static ushort next_local_pool_index;

// Hashtable mapping from global constants to a lcoal constant pool.
struct bucket {
    uint      	    key;	// (kind << 16) | (global index)
    ushort          value;	// index into local pool
    struct bucket   *next;	// next bucket in chain.
};

#define BASE_BUCKETS 231
static struct bucket *base_buckets[BASE_BUCKETS];

#define BUCKETS 4000
static struct bucket buckets[BUCKETS];
static int next_bucket_index;

// Indexed into String allocated during prescan which will
// need to have UTF blocks added when prescan compleres.
uchar *prescan_fixups[256];
int max_prescan_fixups;

//----------------------------------------------------------------------

void
reset_local_pool()
{
    int i;
    local_pool_ptr = local_pool;
    next_local_pool_index = 1;
    for (i = 0; i < BASE_BUCKETS; i++) {
	base_buckets[i] = 0;
    }
    next_bucket_index = 0;

}

// Do a lookup in our hashtable to see if we have already
// mapped a given constant.  Returns zero on failure.
static ushort
lookup(uint key) 
{
    int hash = key % BASE_BUCKETS;
    struct bucket *b = base_buckets[hash];
    while (b) {
	if (b->key == key) {
	    return b->value;
	}
	b = b->next;
    }
    // We're going to have to alocate a new constant.
    // make a rough check here that there is still space.
    if ((local_pool_ptr + 4000) > local_pool_limit) {
	fprintf(stderr, "Out of local pool space\n");
	exit(101);
    }
    return 0;
}

// Insert a new key/value into our hashtable.
static void
insert(uint key, ushort value) 
{
    int hash = key % BASE_BUCKETS;
    struct bucket *b = buckets + next_bucket_index;
    next_bucket_index++;
    if (next_bucket_index > BUCKETS) {
	fprintf(stderr, "Ran out of local constant pool buckets\n");
 	exit(100);
    }
    b->key = key;
    b->value = value;
    b->next = base_buckets[hash];
    base_buckets[hash] = b;
}

static void
put1(uchar value)
{
    *local_pool_ptr++ = value;
}

static void
put2(ushort value)
{
    *local_pool_ptr++ = (value >> 8) & 0xFF;
    *local_pool_ptr++ = value & 0xFF;
}

static void
putN(uchar *src, uint len)
{
    while (len--) {
	*local_pool_ptr++ = *src++;
    }
}

// Input is a null-terinated UTF encoded string.
// Write it as a two-byte length followed by chars.
static void
put_string(uchar *s)
{
    ushort len = strlen(s);
    put2(len);
    putN(s, len);
}


// Map a global value constant to our local constant pool.
ushort
localize_value(ushort ix)
{
    uint key = VALUE_KIND | ix;
    ushort fix;
    uchar *valuep;
    uchar tag;
    if (ix == 0) {
	return 0;
    }
    fix = lookup(key);
    if (fix != 0) {
	return fix;
    }
    // This is a previously unmapped value.
    valuep = value_pool[ix];
    tag = *valuep++;

    // Strings need special handling.
    if (tag == CONSTANT_STRING) {
	// Allocate a UTF constant for the string body.
        ushort utf_index = next_local_pool_index++;
        put1(CONSTANT_UTF8);
	put_string(valuep);

	// Now generate the CONSTANT_STRING.
        fix = next_local_pool_index++;
	put1(CONSTANT_STRING);
        put2(utf_index);

    } else {
        fix = next_local_pool_index++;
        put1(tag);
	// Copy the body of the value into our constant pool.
        switch (tag) {
          case CONSTANT_INTEGER:
	    putN(valuep, 4);
	    break;
          case CONSTANT_FLOAT:
	    putN(valuep, 4);
	    break;
          case CONSTANT_LONG:
	    // long constants occupy two slots, for occult reasons.
	    next_local_pool_index++;
	    putN(valuep, 8);
	    break;
          case CONSTANT_DOUBLE:
	    // long constants occupy two slots, for occult reasons.
	    next_local_pool_index++;
	    putN(valuep, 8);
	    break;
          default:
	    fprintf(stderr, "Unexpected tag %d in value pool\n", tag);
	    exit(44);
        }
    }

    insert(key, fix);
    return fix;
}

static void
put_class_name(ushort ix)
{
    uchar *cp = class_pool[ix];
    if (*cp == '%') {
	put_class_name(get_swapped_short(cp+1));
	put1('/');
	cp += 3;
    }
    putN(cp, strlen(cp));
}


// Map a global class constant to our local constant pool.
ushort
localize_class(ushort ix)
{
    uint key = CLASS_KIND | ix;
    ushort fix, utf_index;
    uchar *headp;

    if (ix == 0) {
	return 0;
    }
    fix = lookup(key);
    if (fix != 0) {
	return fix;
    }
    // This is a previously unmapped class.
    // We will need to create both a class constant
    // and a separate UTF constant for the class name.

    // First, generate the UTF constant.
    utf_index = next_local_pool_index++;
    put1(CONSTANT_UTF8);
    // Skip over the UTF length.  We will fill this in later.
    headp = local_pool_ptr;
    put2(0);
    put_class_name(ix);
    set_swapped_short(headp, (ushort)(local_pool_ptr-(headp+2)));

    // Now generate the class constant.
    fix = next_local_pool_index++;
    put1(CONSTANT_CLASS);
    put2(utf_index);

    insert(key, fix);
    return fix;
}

// Map a global UTF constant to our local constant pool.
ushort
localize_utf(ushort ix)
{
    uint key = UTF_KIND | ix;
    ushort fix;

    if (ix == 0) {
	return 0;
    }
    fix = lookup(key);
    if (fix != 0) {
	return fix;
    }
    // This is a previously unmapped UTF string.
    fix = next_local_pool_index++;
    put1(CONSTANT_UTF8);
    put_string(utf_pool[ix]);

    insert(key, fix);
    return fix;
}

// Map a global TYPE constant to our local constant pool.
ushort
localize_type(ushort ix)
{
    uint key = TYPE_KIND | ix;
    ushort fix;
    uchar *typep;
    uchar *headp;

    if (ix == 0) {
	return 0;
    }
    fix = lookup(key);
    if (fix != 0) {
	return fix;
    }
    // This is a previously unmapped TYPE string.
    typep = type_pool[ix];

    // printf("Localized type %s\n", (typep+2));

    fix = next_local_pool_index++;
    put1(CONSTANT_UTF8);

    // Skip over the UTF length.  We will fill this in later.
    headp = local_pool_ptr;
    put2(0);

    for (;;) {
	uchar ch = *typep++;
	if (ch == 0) {
	    break;
	}
	put1(ch);
	if (ch == 'L') {
	    // An embedded classname reference.
	    ushort cix = get_swapped_short(typep);
	    typep += 2;
	    put_class_name(cix);
	    put1(';');
	}
    }
  
    set_swapped_short(headp, (ushort)(local_pool_ptr-(headp+2)));

    insert(key, fix);
    return fix;
}



// Private utility to map any of the three kinds
// of reference types.
static ushort
localize_ref(ushort ix, uchar tag)
{
    uint key = REF_KIND | ix;
    ushort fix, cix, nix, tix, ntix;
    uchar *refp;

    // printf("Localize ref %d\n", ix);  fflush(stdout);

    if (ix == 0) {
	return 0;
    }

    fix = lookup(key);
    if (fix != 0) {
	return fix;
    }

    // Localize the component class, name, and type.
    refp = ref_pool + (6 * ix);
    cix = localize_class(get_swapped_short(refp));
    nix = localize_utf(get_swapped_short(refp+2));
    tix = localize_type(get_swapped_short(refp+4));

    // generate a name and type constant.
    ntix = next_local_pool_index++;
    put1(CONSTANT_NAMEANDTYPE);
    put2(nix);
    put2(tix);

    // Now we can generate the reference constant.
    fix = next_local_pool_index++;
    put1(tag);
    put2(cix);
    put2(ntix);

    insert(key, fix);
    return fix;
}

ushort
localize_ref_name(ushort ix) {
    // Localize the component class, name, and type.
    uchar *refp = ref_pool + (6 * ix);
    return localize_utf(get_swapped_short(refp+2));
}

ushort
localize_ref_type(ushort ix) {
    // Localize the component class, name, and type.
    uchar *refp = ref_pool + (6 * ix);
    return localize_type(get_swapped_short(refp+4));
}


// Map a global field reference to our local constant pool.
ushort
localize_field_ref(ushort ix)
{
    return localize_ref(ix, CONSTANT_FIELDREF);
}

// Map a global method reference to our local constant pool.
ushort
localize_method_ref(ushort ix)
{
    return localize_ref(ix, CONSTANT_METHODREF);
}

// Map a global interface method reference to our local constant pool.
ushort
localize_interface_method_ref(ushort ix)
{
    return localize_ref(ix, CONSTANT_INTERFACEMETHODREF);
}

//----------------------------------------------------------------------

// This is called during the prescan phase to make sure we
// can subsequently use a one-byte index for the given value.
// Most values are easy, but strings need to be handled
// specially.
void
prescan_value(ushort ix)
{
    uint key = VALUE_KIND | ix;
    int i;
    ushort fix, len;
    uchar *valuep;
    uchar tag;
    if (ix == 0) {
	return;
    }
    fix = lookup(key);
    if (fix != 0) {
	return;
    }
    // This is a previously unmapped value.
    valuep = value_pool[ix];
    tag = *valuep++;

    // Only strings need special handling.
    if (tag != CONSTANT_STRING) {
	localize_value(ix);
	return;
    }

    // Generate the CONSTANT_STRING block.
    fix = next_local_pool_index++;
    put1(CONSTANT_STRING);
    // Record the slot that needs fixed up.
    prescan_fixups[fix] = local_pool_ptr;

    // printf("Allocated fixup for %d: %s\n", fix, (valuep+2));

    max_prescan_fixups = fix;

    // Write the global value index into the slot.
    // This will get replaced with a local index later.
    put2(ix);

    insert(key, fix);
}

// Check for any String values that were pre-allocated
// during prescan but which still need a real UTF block.

void
fixup_prescan_values()
{
    int i, j, len;
    ushort ix, fix;
    uchar tag;
    uchar *valuep;

    // Note: we realy do mean to use "<=" here.
    for (i = 0; i <= max_prescan_fixups; i++) {
	uchar *fixup = prescan_fixups[i];
	if (fixup == 0) {
	    continue;
	}

	ix = get_swapped_short(fixup);
        valuep = value_pool[ix];
	tag = *valuep++;

	if (tag != CONSTANT_STRING) {
	    fprintf(stderr, "Trouble with fixup on prescan string\n");
	    exit(79);
	}

	// Allocate a UTF constant for the string body.
        fix = next_local_pool_index++;
        put1(CONSTANT_UTF8);
	put_string(valuep);

	// Now we can fixup the index in the String constant.
	set_swapped_short(fixup, fix);

	prescan_fixups[i] = 0;
    }
    max_prescan_fixups = 0;
}

//------------------------------------------------------------

uint get_local_pool_length()
{
    return (2 + (local_pool_ptr-local_pool));
}


void
write_local_pool()
{
    uchar swapped_size[2];

    // Write the pool size.
    set_swapped_short(swapped_size, next_local_pool_index);
    write_data(swapped_size, 2);

    // Write the pool
    write_data(local_pool, (local_pool_ptr-local_pool));
}


