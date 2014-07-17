/*
 * @(#)launchFile.h	1.14 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Interprets a JNL file an populates
 * the JNLFile structure. This interface
 * separates the structure of the JNL file from
 * the logic of the launcing.
 *
 */

typedef struct _jnlFile {
    char* jreVersion;   /* Version String for JRE */
    char* jreLocation;  /* URL for JRE or NULL if none specified */
    int   isPlayer;     /* TRUE, if is player JNLP */
    char* jnlp_url;	/* codebase+href (or null if href dosn't exist) */
    char* canonicalHome;
    char* initialHeap;
    char* maxHeap;
    int   auxArgCount;
    char* auxArg[20];   /* be carful not to overfill */
} JNLFile;

/*
 * Parse a JNL file, and returns a structure with the information
 * needed by the C launcher 
 *
 * The interpretation of the JNL file is completly encapsulated into
 * this method, so several formats such as XML and property files can
 * be supported at the same time 
 */
JNLFile* ParseJNLFile(char *s);

/* Release all memory allocated to a JNLFile */
void FreeJNLFile(JNLFile* jnlfile);


