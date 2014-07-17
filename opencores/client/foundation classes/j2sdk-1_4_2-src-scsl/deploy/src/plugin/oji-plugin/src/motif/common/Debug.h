/*
 * @(#)Debug.h	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __cplusplus
extern "C" {
#endif
    void trace(const char *format, ...);
    void trace_verbose(const char *format, ...);
    extern int tracing;
#ifdef __cplusplus
}
#endif
