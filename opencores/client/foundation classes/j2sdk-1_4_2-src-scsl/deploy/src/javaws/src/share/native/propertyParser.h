/*
 * @(#)propertyParser.h	1.9 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef PROPERTYPARSER_H
#define PROPERTYPARSER_H

/*
 * Configuration file
 */
typedef struct _propertyFileEntry { 
  char* key;
  char* value;
  struct _propertyFileEntry* next;
} PropertyFileEntry;

void storePropertyFile(char *, PropertyFileEntry* , PropertyFileEntry*);
PropertyFileEntry* parsePropertyFile(char *filename, PropertyFileEntry* head);
PropertyFileEntry* parsePropertyStream(char *s, PropertyFileEntry* head);

/* Find an entry */
char* GetPropertyValue(PropertyFileEntry* head, char* key);

/* Free's all memory allocated by the structure */
void FreePropertyEntry(PropertyFileEntry* head);

/* Debugging */
void PrintPropertyEntry(PropertyFileEntry* entry);

#endif


