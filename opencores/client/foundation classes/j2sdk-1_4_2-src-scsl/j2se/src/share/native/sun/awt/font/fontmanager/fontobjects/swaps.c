/*
 * @(#)swaps.c	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "swaps.h"

UInt16
swapWord(UInt16 value) {
    return (((Byte)(value >> 8)) | (value << 8));
}

UInt32
swapLong(UInt32 value) {
    return SWAPW((UInt16)(value >> 16)) | (SWAPW((UInt16)value) << 16);
}

UInt16
swapWordPtr(UInt16** value) {
    UInt16 result = **value;

    ++(*value);
    return SWAPW(result);
}
