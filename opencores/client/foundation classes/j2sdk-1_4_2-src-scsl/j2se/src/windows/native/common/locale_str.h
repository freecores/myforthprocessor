/*
 * @(#)locale_str.h	1.32 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#define DllExport __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

DllExport const char * getEncodingFromLangID(LANGID langID);
DllExport const char * getJavaIDFromLangID(LANGID langID);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
