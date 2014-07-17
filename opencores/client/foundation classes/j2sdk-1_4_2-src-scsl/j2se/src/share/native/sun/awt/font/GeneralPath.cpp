/*
 * @(#)GeneralPath.cpp	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GeneralPath.h"

#include <stdlib.h>

static const jint DEFAULT_LEN_TYPES = 10;
static const jint DEFAULT_LEN_COORDS = 50;

static const jint TYPES_GROW_SIZE = 15;
static const jint TYPES_GROW_MASK = 0xfffffff0;

static const jint COORDS_GROW_SIZE = 31;
static const jint COORDS_GROW_MASK = 0xffffffe0;

GeneralPath::GeneralPath(jint windingRule)
  : pointTypes((jbyte*)malloc(sizeof(jbyte) * DEFAULT_LEN_TYPES))
  , pointCoords((jfloat*)malloc(sizeof(jfloat) * DEFAULT_LEN_COORDS))
  , numTypes(0)
  , numCoords(0)
  , lenTypes(DEFAULT_LEN_TYPES)
  , lenCoords(DEFAULT_LEN_COORDS)
  , wr(windingRule)
{
}

GeneralPath::~GeneralPath() 
{
  if (pointTypes) {
    free(pointTypes); 
    pointTypes = 0;
  }
  if (pointCoords) {
    free(pointCoords); 
    pointCoords = 0;
  }
}

jboolean
GeneralPath::needRoom(jint newTypes, jint newCoords) {
  jint reqTypes = numTypes + newTypes;
  if (reqTypes > lenTypes) {
    lenTypes = (reqTypes + TYPES_GROW_SIZE) & TYPES_GROW_MASK;
    pointTypes = (jbyte*)realloc(pointTypes, lenTypes * sizeof(jbyte));
  }

  jint reqCoords = numCoords + newCoords;
  if (reqCoords > lenCoords) {
    lenCoords = (reqCoords + COORDS_GROW_SIZE) & COORDS_GROW_MASK;
    pointCoords = (jfloat*)realloc(pointCoords, lenCoords * sizeof(jfloat));
  }

  return pointTypes && pointCoords;
}

jobject 
GeneralPath::getShape(JNIEnv *env) {
  static jclass gpClass = 0;
  static jmethodID gpCC = 0;
  static const char* gpClassName = "java/awt/geom/GeneralPath";
  static const char* gpCCName = "<init>";
  static const char* gpCCArgs = "(I[BI[FI)V";

  if (!gpClass) {
    gpClass = env->FindClass(gpClassName);
    if (!gpClass) {
	JNU_ThrowClassNotFoundException(env, gpClassName);
	return NULL;
    }
    gpClass = (jclass)env->NewGlobalRef(gpClass);
    if (!gpClass) {
	JNU_ThrowInternalError(env, "could not create global ref");
	return NULL;
    }
    gpCC = env->GetMethodID(gpClass, gpCCName, gpCCArgs);
    if (!gpCC) {
      gpClass = 0;
      JNU_ThrowNoSuchMethodException(env, gpCCName);
      return NULL;
    }
  }

  jbyteArray types = env->NewByteArray(numTypes);
  jfloatArray coords = env->NewFloatArray(numCoords);

  if (types && coords) {
      env->SetByteArrayRegion(types, 0, numTypes, pointTypes);
      env->SetFloatArrayRegion(coords, 0, numCoords, pointCoords);
  
      return env->NewObject(gpClass, gpCC, wr, types, numTypes, coords, numCoords);
  }

  return NULL;
}
