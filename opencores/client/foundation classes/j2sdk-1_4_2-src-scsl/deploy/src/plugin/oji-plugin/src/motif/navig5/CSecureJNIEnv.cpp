/*
 * @(#)CSecureJNIEnv.cpp	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CSecureJNIEnv.cpp  by Stanley Man-Kit Ho
//
///=--------------------------------------------------------------------------=
//
// These functions create object, invoke method, get/set field in JNI with
// security context.
//

#include "nsplugin.h"
#include "remotejni.h"
#include "nsISecurityContext.h"
#include "nsAgg.h"
#include "jni.h"
#include "Debug.h"
//#include "nsISecureJNIEnv.h"
#include "nsISecureEnv.h"
#include "CSecureJNIEnv.h"

#if !defined(UNUSED)
#define UNUSED(x) x=x
#endif

// Mozilla changes : Name of IID changed
//NS_DEFINE_IID(kISecureJNIEnvIID, NS_ISECUREJNI2_IID);
NS_DEFINE_IID(kISecureJNIEnvIID, NS_ISECUREENV_IID);
NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

#define TRACE(m) if (tracing) trace("CSecureJNIEnv %s\n", m );

////////////////////////////////////////////////////////////////////////////
// from nsISupports and AggregatedQueryInterface:

// Thes macro expands to the aggregated query interface scheme.

NS_IMPL_AGGREGATED(CSecureJNIEnv);

NS_METHOD
CSecureJNIEnv::AggregatedQueryInterface(const nsIID& aIID, void** aInstancePtr)
{
    if (aIID.Equals(kISupportsIID)) {
      *aInstancePtr = GetInner();
      AddRef();
      return NS_OK;
    }
    if (aIID.Equals(kISecureJNIEnvIID)) {
      // Mozilla changes: Name of class changed
      //*aInstancePtr = (nsISecureJNIEnv *)this;
        *aInstancePtr = (nsISecureEnv *)this;
        AddRef();
        return NS_OK;
    }
    return NS_NOINTERFACE;
}



///=--------------------------------------------------------------------------=
// CSecureJNIEnv::Initialize
///=--------------------------------------------------------------------------=
// Initialize all the global variables for speeding up JNI invocation.
//
// @return  status code.
//
NS_IMETHODIMP CSecureJNIEnv::Initialize()
{
    return NS_OK;
}


///=--------------------------------------------------------------------------=
// CSecureJNIEnv::CSecureJNIEnv
///=--------------------------------------------------------------------------=
// Implements the CSecureJNI object for creating object, invoking method, 
// getting/setting field in JNI with security context.
//
// parameters :
//
// return :
// 
// notes :
//
CSecureJNIEnv::CSecureJNIEnv(nsISupports *aOuter, RemoteJNIEnv* env)
{
    TRACE("CSecureJNIEnv::CSecureJNIEnv\n");

    NS_INIT_AGGREGATED(aOuter);

    m_env = env;
    
}



///=--------------------------------------------------------------------------=
// CSecureJNIEnv::~CSecureJNIEnv
///=--------------------------------------------------------------------------=
// Implements the CSecureJNIEnv object for creating object, invoking method, 
// getting/setting field in JNI with security context.
//
// parameters :
//
// return :
// 
// notes :
//
CSecureJNIEnv::~CSecureJNIEnv()  
{
    TRACE("CSecureJNIEnv::~CSecureJNIEnv\n");
}


///=--------------------------------------------------------------------------=
// CSecureJNIEnv::Create
///=--------------------------------------------------------------------------=
// Create the CSecureJNIEnv object for creating object, invoking method, 
// getting/setting field in JNI with security context.
//
// parameters :
//
// return :
// 
// notes :
//
NS_METHOD
CSecureJNIEnv::Create(nsISupports* outer, RemoteJNIEnv* env, const nsIID& aIID, 
		    void* *aInstancePtr)
{
    if (outer && !aIID.Equals(kISupportsIID))
        return NS_NOINTERFACE;   // XXX right error?
    CSecureJNIEnv* jni = new CSecureJNIEnv(outer, env);
    if (jni == NULL)
        return NS_ERROR_OUT_OF_MEMORY;
    jni->AddRef();
    *aInstancePtr = jni->GetInner();
    *aInstancePtr = (outer != NULL)? (void *)jni->GetInner(): (void *)jni;
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////
// from nsISecureJNI:
//


///=--------------------------------------------------------------------------=
// CSecureJNIEnv::NewObject
///=--------------------------------------------------------------------------=
// Create new Java object in LiveConnect.
//
// @param env        -- JNIEnv pointer.
// @param clazz      -- Java Class object.
// @param methodID   -- Method id
// @param args       -- arguments for invoking the constructor.
// @param result     -- return new Java object.
// @param ctx        -- security context 
//
NS_IMETHODIMP CSecureJNIEnv::NewObject(/*[in]*/  jclass clazz, 
                                     /*[in]*/  jmethodID methodID, 
                                     /*[in]*/  jvalue *args, 
                                     /*[out]*/ jobject* result,
                                     /*[in]*/  nsISecurityContext* ctx)
{
    TRACE("CSecureJNIEnv::NewObject\n");
    return m_env->SecureNewObject(clazz, methodID, args, result, ctx);
}
   

///=--------------------------------------------------------------------------=
// CSecureJNIEnv::CallMethod
///=--------------------------------------------------------------------------=
// Invoke method on Java object in LiveConnect.
//
// @param type       -- Return type
// @param obj        -- Java object.
// @param methodID   -- Method id
// @param result     -- return result of invocation.
// @param ctx        -- security context 
//
NS_IMETHODIMP CSecureJNIEnv::CallMethod(/*[in]*/  jni_type type,
                                      /*[in]*/  jobject obj, 
                                      /*[in]*/  jmethodID methodID, 
                                      /*[in]*/  jvalue *args, 
                                      /*[out]*/ jvalue* result,
                                      /*[in]*/  nsISecurityContext* ctx)
{
    TRACE("CSecureJNIEnv::CallMethod\n");
    return m_env->SecureCallMethod(type, obj, methodID, args, result, ctx);
}


///=--------------------------------------------------------------------------=
// CSecureJNIEnv::CallNonvirtualMethod
///=--------------------------------------------------------------------------=
// Invoke non-virtual method on Java object in LiveConnect.
//
// @param type       -- Return type
// @param obj        -- Java object.
// @param class      -- Java class.
// @param methodID   -- Method id
// @param args       -- arguments for invoking the constructor.
// @param result     -- return result of invocation.
// @param ctx        -- security context 
//
NS_IMETHODIMP CSecureJNIEnv::CallNonvirtualMethod(/*[in]*/  jni_type type,
                                                /*[in]*/  jobject obj, 
                                                /*[in]*/  jclass clazz,
                                                /*[in]*/  jmethodID methodID,
                                                /*[in]*/  jvalue *args, 
                                                /*[out]*/ jvalue* result,
                                                /*[in]*/  nsISecurityContext* ctx)
{
    TRACE("CSecureJNIEnv::CallNonvirtualMethod\n");
    return m_env->SecureCallNonvirtualMethod(type, obj, clazz, methodID, args,
				      result, ctx);
}


///=--------------------------------------------------------------------------=
// CSecureJNIEnv::GetField
///=--------------------------------------------------------------------------=
// Get a field on Java object in LiveConnect.
//
// @param type       -- Field type
// @param obj        -- Java object.
// @param fieldID    -- field id
// @param result     -- return field value
// @param ctx        -- security context 
//
NS_IMETHODIMP CSecureJNIEnv::GetField(/*[in]*/  jni_type type,
                                    /*[in]*/  jobject obj, 
                                    /*[in]*/  jfieldID fieldID,
                                    /*[out]*/ jvalue* result,
                                    /*[in]*/  nsISecurityContext* ctx)
{
    TRACE("CSecureJNIEnv::GetField\n");
    return m_env->SecureGetField(type, obj, fieldID, result, ctx);
}


///=--------------------------------------------------------------------------=
// CSecureJNIEnv::SetField
///=--------------------------------------------------------------------------=
//
// Set a field on Java object in LiveConnect.
//
// @param type       -- field type
// @param obj        -- Java object.
// @param fieldID    -- field id
// @param result     -- field value to set
// @param ctx        -- security context 
//
NS_IMETHODIMP CSecureJNIEnv::SetField(/*[in]*/ jni_type type,
                                    /*[in]*/ jobject obj, 
                                    /*[in]*/ jfieldID fieldID,
                                    /*[in]*/ jvalue val,
                                    /*[in]*/ nsISecurityContext* ctx)
{
    TRACE("CSecureJNIEnv::SetField\n");
    
    return m_env->SecureSetField(type, obj, fieldID, val, ctx);
}


///=--------------------------------------------------------------------------=
// CSecureJNIEnv::CallStaticMethod
///=--------------------------------------------------------------------------=
//
// Invoke static method on Java object in LiveConnect.
//
// @param type       -- return type
// @param obj        -- Java object.
// @param methodID   -- method id
// @param args       -- arguments for invoking the constructor.
// @param result     -- return result of invocation.
// @param ctx        -- security context 
//
NS_IMETHODIMP CSecureJNIEnv::CallStaticMethod(/*[in]*/  jni_type type,
                                            /*[in]*/  jclass clazz,
                                            /*[in]*/  jmethodID methodID,
                                            /*[in]*/  jvalue *args, 
                                            /*[out]*/ jvalue* result,
                                            /*[in]*/  nsISecurityContext* ctx)
{
    TRACE("CSecureJNIEnv::CallStaticMethod\n");
    
    return m_env->SecureCallStaticMethod(type, clazz, methodID, args, 
					 result, ctx);
}


///=--------------------------------------------------------------------------=
// CSecureJNIEnv::GetStaticField
///=--------------------------------------------------------------------------=
// Get a static field on Java object in LiveConnect.
//
// @param type       -- static field type
// @param clazz      -- Java object.
// @param fieldID    -- field id
// @param result     -- return field value
// @param ctx        -- security context 
//
NS_IMETHODIMP CSecureJNIEnv::GetStaticField(/*[in]*/  jni_type type,
                                          /*[in]*/  jclass clazz, 
                                          /*[in]*/  jfieldID fieldID,
                                          /*[out]*/ jvalue* result,
                                          /*[in]*/  nsISecurityContext* ctx)
{
    TRACE("CSecureJNIEnv::GetStaticField\n");

    return m_env->SecureGetStaticField(type, clazz, fieldID, 
				result, ctx);
}


///=--------------------------------------------------------------------------=
// CSecureJNIEnv::SetStaticField
///=--------------------------------------------------------------------------=
// Set a static field on Java object in LiveConnect.
//
// @param type       -- static field type
// @param clazz      -- Java object.
// @param fieldID    -- field id
// @param result     -- field value to set
// @param ctx        -- security context XFAG
//
NS_IMETHODIMP CSecureJNIEnv::SetStaticField(/*[in]*/ jni_type type,
                                          /*[in]*/ jclass clazz, 
                                          /*[in]*/ jfieldID fieldID,
                                          /*[in]*/ jvalue val,
                                          /*[in]*/ nsISecurityContext* ctx)
{
    TRACE("CSecureJNIEnv::SetStaticField\n");

    return m_env->SecureSetStaticField(type, clazz, fieldID, 
				       val, ctx);
}


NS_IMETHODIMP CSecureJNIEnv::GetVersion(/*[out]*/ jint* version) 
{
    if (m_env == NULL || version)
        return NS_ERROR_NULL_POINTER;
    
    *version = m_env->GetVersion();

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::DefineClass(/*[in]*/  const char* name,
                                       /*[in]*/  jobject loader,
                                       /*[in]*/  const jbyte *buf,
                                       /*[in]*/  jsize len,
                                       /*[out]*/ jclass* clazz) 
{
    if (m_env == NULL || clazz == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *clazz = m_env->DefineClass(name, loader, buf, len);

    return NS_OK;
}
                                       

NS_IMETHODIMP CSecureJNIEnv::FindClass(/*[in]*/  const char* name, 
                                     /*[out]*/ jclass* clazz) 
{
    if (m_env == NULL || clazz == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *clazz = m_env->FindClass(name);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetSuperclass(/*[in]*/  jclass sub,
                                         /*[out]*/ jclass* super) 
{
    if (m_env == NULL || super == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *super = m_env->GetSuperclass(sub);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::IsAssignableFrom(/*[in]*/  jclass sub,
                                            /*[in]*/  jclass super,
                                            /*[out]*/ jboolean* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->IsAssignableFrom(sub, super);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::Throw(/*[in]*/  jthrowable obj,
                                 /*[out]*/ jint* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->Throw(obj);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::ThrowNew(/*[in]*/  jclass clazz,
                                    /*[in]*/  const char *msg,
                                    /*[out]*/ jint* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->ThrowNew(clazz, msg);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::ExceptionOccurred(/*[out]*/ jthrowable* result)
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->ExceptionOccurred();

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::ExceptionDescribe(void)
{
    if (m_env == NULL)
        return NS_ERROR_NULL_POINTER;
    
    m_env->ExceptionDescribe();

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::ExceptionClear(void)
{
    if (m_env == NULL)
        return NS_ERROR_NULL_POINTER;
    
    m_env->ExceptionClear();

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::FatalError(/*[in]*/ const char* msg)
{
    if (m_env == NULL)
        return NS_ERROR_NULL_POINTER;
    
    m_env->FatalError(msg);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::NewGlobalRef(/*[in]*/  jobject lobj, 
                                        /*[out]*/ jobject* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->NewGlobalRef(lobj);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::DeleteGlobalRef(/*[in]*/ jobject gref) 
{
    if (m_env == NULL)
        return NS_ERROR_NULL_POINTER;
    
    m_env->DeleteGlobalRef(gref);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::DeleteLocalRef(/*[in]*/ jobject obj)
{
    if (m_env == NULL)
        return NS_ERROR_NULL_POINTER;
    
    m_env->DeleteLocalRef(obj);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::IsSameObject(/*[in]*/  jobject obj1,
                                        /*[in]*/  jobject obj2,
                                        /*[out]*/ jboolean* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->IsSameObject(obj1, obj2);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::AllocObject(/*[in]*/  jclass clazz,
                                       /*[out]*/ jobject* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->AllocObject(clazz);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetObjectClass(/*[in]*/  jobject obj,
                                          /*[out]*/ jclass* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->GetObjectClass(obj);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::IsInstanceOf(/*[in]*/  jobject obj,
                                        /*[in]*/  jclass clazz,
                                        /*[out]*/ jboolean* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->IsInstanceOf(obj, clazz);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetMethodID(/*[in]*/  jclass clazz, 
                                       /*[in]*/  const char* name,
                                       /*[in]*/  const char* sig,
                                       /*[out]*/ jmethodID* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->GetMethodID(clazz, name, sig);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetFieldID(/*[in]*/  jclass clazz, 
                                      /*[in]*/  const char* name,
                                      /*[in]*/  const char* sig,
                                      /*[out]*/ jfieldID* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->GetFieldID(clazz, name, sig);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetStaticMethodID(/*[in]*/  jclass clazz, 
                                             /*[in]*/  const char* name,
                                             /*[in]*/  const char* sig,
                                             /*[out]*/ jmethodID* result)
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->GetStaticMethodID(clazz, name, sig);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetStaticFieldID(/*[in]*/  jclass clazz, 
                                            /*[in]*/  const char* name,
                                            /*[in]*/  const char* sig,
                                            /*[out]*/ jfieldID* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    // ?? m_env->ExceptionClear();
    *result = m_env->GetStaticFieldID(clazz, name, sig);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::NewString(/*[in]*/  const jchar* unicode,
                                     /*[in]*/  jsize len,
                                     /*[out]*/ jstring* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->NewString(unicode, len);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetStringLength(/*[in]*/  jstring str,
                                           /*[out]*/ jsize* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->GetStringLength(str);

    return NS_OK;
}
    

NS_IMETHODIMP CSecureJNIEnv::GetStringChars(/*[in]*/  jstring str,
                                          /*[in]*/  jboolean *isCopy,
                                          /*[out]*/ const jchar** result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->GetStringChars(str, isCopy);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::ReleaseStringChars(/*[in]*/  jstring str,
                                              /*[in]*/  const jchar *chars) 
{
    if (m_env == NULL)
        return NS_ERROR_NULL_POINTER;
    
    m_env->ReleaseStringChars(str, chars);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::NewStringUTF(/*[in]*/  const char *utf,
                                        /*[out]*/ jstring* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->NewStringUTF(utf);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetStringUTFLength(/*[in]*/  jstring str,
                                              /*[out]*/ jsize* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->GetStringUTFLength(str);

    return NS_OK;
}

    
NS_IMETHODIMP CSecureJNIEnv::GetStringUTFChars(/*[in]*/  jstring str,
                                             /*[in]*/  jboolean *isCopy,
                                             /*[out]*/ const char** result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->GetStringUTFChars(str, isCopy);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::ReleaseStringUTFChars(/*[in]*/  jstring str,
                                                 /*[in]*/  const char *chars) 
{
    if (m_env == NULL)
        return NS_ERROR_NULL_POINTER;
    
    m_env->ReleaseStringUTFChars(str, chars);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetArrayLength(/*[in]*/  jarray array,
                                          /*[out]*/ jsize* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;
    
    *result = m_env->GetArrayLength(array);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetObjectArrayElement(/*[in]*/  jobjectArray array,
                                                 /*[in]*/  jsize index,
                                                 /*[out]*/ jobject* result)
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;

    *result = m_env->GetObjectArrayElement(array, index);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::SetObjectArrayElement(/*[in]*/  jobjectArray array,
                                                 /*[in]*/  jsize index,
                                                 /*[in]*/  jobject val) 
{
    if (m_env == NULL)
        return NS_ERROR_NULL_POINTER;

    m_env->SetObjectArrayElement(array, index, val);

    return NS_OK;
}



NS_IMETHODIMP CSecureJNIEnv::NewObjectArray(/*[in]*/  jsize len,
					  /*[in]*/  jclass clazz,
                                          /*[in]*/  jobject init,
                                          /*[out]*/ jobjectArray* result)
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;

    *result = m_env->NewObjectArray(len, clazz, init);
            
    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::NewArray(/*[in]*/  jni_type element_type,
                                    /*[in]*/  jsize len,
                                    /*[out]*/ jarray* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;

    switch (element_type)
    {
        case jboolean_type: {
            *result = m_env->NewBooleanArray(len);
            break;
        }
    
        case jbyte_type:    {
            *result = m_env->NewByteArray(len);
            break;
        }

        case jchar_type:    {
            *result = m_env->NewCharArray(len);
            break;
        }

        case jshort_type:   {
            *result = m_env->NewShortArray(len);
            break;
        }

        case jint_type:     {
            *result = m_env->NewIntArray(len);
            break;
        }

        case jlong_type:{
            *result = m_env->NewLongArray(len);
            break;
        }

        case jfloat_type:{
            *result = m_env->NewFloatArray(len);
            break;
        }

        case jdouble_type:{
            *result = m_env->NewDoubleArray(len);
            break;
        }

        default:
            return NS_ERROR_FAILURE;
    }

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::GetArrayElements(/*[in]*/  jni_type element_type,
                                            /*[in]*/  jarray array,
                                            /*[in]*/  jboolean *isCopy,
                                            /*[out]*/ void* result)
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;

    switch (element_type)
    {
        case jboolean_type: {
            *(void**)result = (void*) m_env->GetBooleanArrayElements((jbooleanArray)array, isCopy);
            break;
        }
    
        case jbyte_type:    {
            *(void**)result = (void*) m_env->GetByteArrayElements((jbyteArray)array, isCopy);
            break;
        }

        case jchar_type:    {
            *(void**)result = (void*) m_env->GetCharArrayElements((jcharArray)array, isCopy);
            break;
        }

        case jshort_type:   {
            *(void**)result = (void*) m_env->GetShortArrayElements((jshortArray)array, isCopy);
            break;
        }

        case jint_type:     {
            *(void**)result = (void*) m_env->GetIntArrayElements((jintArray)array, isCopy);
            break;
        }

        case jlong_type:{
            *(void**)result = (void*) m_env->GetLongArrayElements((jlongArray)array, isCopy);
            break;
        }

        case jfloat_type:{
            *(void**)result = (void*) m_env->GetFloatArrayElements((jfloatArray)array, isCopy);
            break;
        }

        case jdouble_type:{
            *(void**)result = (void*) m_env->GetDoubleArrayElements((jdoubleArray)array, isCopy);
            break;
        }

        default:
            return NS_ERROR_FAILURE;
    }

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::ReleaseArrayElements(/*[in]*/ jni_type element_type,
                                                /*[in]*/ jarray array,
                                                /*[in]*/ void *elems,
                                                /*[in]*/ jint mode) 
{
    if (m_env == NULL)
        return NS_ERROR_NULL_POINTER;

    switch (element_type)
    {
        case jboolean_type: {
            m_env->ReleaseBooleanArrayElements((jbooleanArray)array, (jboolean*)elems, mode);
            break;
        }
    
        case jbyte_type:    {
            m_env->ReleaseByteArrayElements((jbyteArray)array, (jbyte*)elems, mode);
            break;
        }

        case jchar_type:    {
            m_env->ReleaseCharArrayElements((jcharArray)array, (jchar*)elems, mode);
            break;
        }

        case jshort_type:   {
            m_env->ReleaseShortArrayElements((jshortArray)array, (jshort*)elems, mode);
            break;
        }

        case jint_type:     {
            m_env->ReleaseIntArrayElements((jintArray)array, (jint*)elems, mode);
            break;
        }

        case jlong_type:{
            m_env->ReleaseLongArrayElements((jlongArray)array, (jlong*)elems, mode);
            break;
        }

        case jfloat_type:{
            m_env->ReleaseFloatArrayElements((jfloatArray)array, (jfloat*)elems, mode);
            break;
        }

        case jdouble_type:{
            m_env->ReleaseDoubleArrayElements((jdoubleArray)array, (jdouble*)elems, mode);
            break;
        }

        default:
            return NS_ERROR_FAILURE;
    }

    return NS_OK;
}

NS_IMETHODIMP CSecureJNIEnv::GetArrayRegion(/*[in]*/  jni_type element_type,
                                          /*[in]*/  jarray array,
                                          /*[in]*/  jsize start,
                                          /*[in]*/  jsize len,
                                          /*[out]*/ void* buf)
{
    if (m_env == NULL || buf == NULL)
        return NS_ERROR_NULL_POINTER;

    switch (element_type)
    {
        case jboolean_type: {
            m_env->GetBooleanArrayRegion((jbooleanArray)array, start, len, (jboolean*)buf);
            break;
        }
    
        case jbyte_type:    {
            m_env->GetByteArrayRegion((jbyteArray)array, start, len, (jbyte*)buf);
            break;
        }

        case jchar_type:    {
            m_env->GetCharArrayRegion((jcharArray)array, start, len, (jchar*)buf);
            break;
        }

        case jshort_type:   {
            m_env->GetShortArrayRegion((jshortArray)array, start, len, (jshort*)buf);
            break;
        }

        case jint_type:     {
            m_env->GetIntArrayRegion((jintArray)array, start, len, (jint*)buf);
            break;
        }

        case jlong_type:{
            m_env->GetLongArrayRegion((jlongArray)array, start, len, (jlong*)buf);
            break;
        }

        case jfloat_type:{
            m_env->GetFloatArrayRegion((jfloatArray)array, start, len, (jfloat*)buf);
            break;
        }

        case jdouble_type:{
            m_env->GetDoubleArrayRegion((jdoubleArray)array, start, len, (jdouble*)buf);
            break;
        }

        default:
            return NS_ERROR_FAILURE;
    }

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::SetArrayRegion(/*[in]*/  jni_type element_type,
                                          /*[in]*/  jarray array,
                                          /*[in]*/  jsize start,
                                          /*[in]*/  jsize len,
                                          /*[in]*/  void* buf) 
{
    if (m_env == NULL || buf == NULL)
        return NS_ERROR_NULL_POINTER;

    switch (element_type)
    {
        case jboolean_type: {
            m_env->SetBooleanArrayRegion((jbooleanArray)array, start, len, (jboolean*)buf);
            break;
        }
    
        case jbyte_type:    {
            m_env->SetByteArrayRegion((jbyteArray)array, start, len, (jbyte*)buf);
            break;
        }

        case jchar_type:    {
            m_env->SetCharArrayRegion((jcharArray)array, start, len, (jchar*)buf);
            break;
        }

        case jshort_type:   {
            m_env->SetShortArrayRegion((jshortArray)array, start, len, (jshort*)buf);
            break;
        }

        case jint_type:     {
            m_env->SetIntArrayRegion((jintArray)array, start, len, (jint*)buf);
            break;
        }

        case jlong_type:{
            m_env->SetLongArrayRegion((jlongArray)array, start, len, (jlong*)buf);
            break;
        }

        case jfloat_type:{
            m_env->SetFloatArrayRegion((jfloatArray)array, start, len, (jfloat*)buf);
            break;
        }

        case jdouble_type:{
            m_env->SetDoubleArrayRegion((jdoubleArray)array, start, len, (jdouble*)buf);
            break;
        }

        default:
            return NS_ERROR_FAILURE;
    }

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::RegisterNatives(/*[in]*/  jclass notyet_clazz,
                                           /*[in]*/  const JNINativeMethod *
					   notyet_methods,
                                           /*[in]*/  jint notyet_nMethods,
                                           /*[out]*/ jint* result) 
{
    UNUSED(notyet_clazz);
    UNUSED(notyet_methods);
    UNUSED(notyet_nMethods);

    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;

    /*    *result = m_env->RegisterNatives(clazz, methods, nMethods); */

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::UnregisterNatives(/*[in]*/  jclass clazz,
                                             /*[out]*/ jint* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;

    *result = m_env->UnregisterNatives(clazz);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::MonitorEnter(/*[in]*/  jobject obj,
                                        /*[out]*/ jint* result) 
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;

    *result = m_env->MonitorEnter(obj);

    return NS_OK;
}


NS_IMETHODIMP CSecureJNIEnv::MonitorExit(/*[in]*/  jobject obj,
                                       /*[out]*/ jint* result)
{
    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;

    *result = m_env->MonitorExit(obj);

    return NS_OK;
}

NS_IMETHODIMP CSecureJNIEnv::GetJavaVM(/*[in]*/  JavaVM **unused_vm,
                                     /*[out]*/ jint* result)
{
    UNUSED(unused_vm);

    if (m_env == NULL || result == NULL)
        return NS_ERROR_NULL_POINTER;

    *result = 0;

    return NS_OK;
}





