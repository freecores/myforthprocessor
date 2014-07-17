/*
 * @(#)Inet4AddressImpl.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include <winsock2.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>

#include "java_net_InetAddress.h"
#include "java_net_Inet4AddressImpl.h"
#include "net_util.h"


/*
 * Returns true if hostname is in dotted IP address format. Note that this 
 * function performs a syntax check only. For each octet it just checks that
 * the octet is at most 3 digits.
 */
jboolean isDottedIPAddress(const char *hostname, unsigned int *addrp) {
    char *c = (char *)hostname;
    int octets = 0;
    unsigned int cur = 0;
    int digit_cnt = 0;

    while (*c) {
	if (*c == '.') {
	    if (digit_cnt == 0) {
		return JNI_FALSE;
	    } else {
		if (octets < 4) {
		    addrp[octets++] = cur;
		    cur = 0;
		    digit_cnt = 0;
		} else {		
		    return JNI_FALSE;
		}
	    }
	    c++;
	    continue;
	}

	if ((*c < '0') || (*c > '9')) {
	    return JNI_FALSE;	
	}

	digit_cnt++;
	if (digit_cnt > 3) {
	    return JNI_FALSE;
	}

	/* don't check if current octet > 255 */
	cur = cur*10 + (*c - '0');			  
			    
	/* Move onto next character and check for EOF */
	c++;
	if (*c == '\0') {
	    if (octets < 4) {
		addrp[octets++] = cur;
	    } else {		
		return JNI_FALSE;
	    }
	}
    }

    return (jboolean)(octets == 4);
}

/*
 * Inet4AddressImpl
 */

/*
 * Class:     java_net_Inet4AddressImpl
 * Method:    getLocalHostName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL 
Java_java_net_Inet4AddressImpl_getLocalHostName (JNIEnv *env, jobject this) {
    char hostname[256];

    if (JVM_GetHostName(hostname, sizeof hostname) == -1) {
	strcpy(hostname, "localhost");
    }
    return JNU_NewStringPlatform(env, hostname);
}

/*
 * Find an internet address for a given hostname.  Not this this
 * code only works for addresses of type INET. The translation
 * of %d.%d.%d.%d to an address (int) occurs in java now, so the
 * String "host" shouldn't be a %d.%d.%d.%d string. The only 
 * exception should be when any of the %d are out of range and
 * we fallback to a lookup.
 *
 * Class:     java_net_Inet4AddressImpl
 * Method:    lookupAllHostAddr
 * Signature: (Ljava/lang/String;)[[B
 *
 * This is almost shared code
 */

JNIEXPORT jobjectArray JNICALL 
Java_java_net_Inet4AddressImpl_lookupAllHostAddr(JNIEnv *env, jobject this, 
						jstring host) {
    const char *hostname;
    struct hostent *hp;
    unsigned int addr[4];

    jobjectArray ret = NULL;
    jclass byteArrayCls;

    if (IS_NULL(host)) {
	JNU_ThrowNullPointerException(env, "host argument");
	return NULL;
    }
    hostname = JNU_GetStringPlatformChars(env, host, JNI_FALSE);
    CHECK_NULL_RETURN(hostname, NULL);

    /*
     * The NT/2000 resolver tolerates a space in front of localhost. This
     * is not consistent with other implementations of gethostbyname.
     * In addition we must do a white space check on Solaris to avoid a
     * bug whereby 0.0.0.0 is returned if any host name has a white space.
     */
    if (isspace(hostname[0])) {
	JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", hostname);
	goto cleanupAndReturn;
    } 

    /*
     * If the format is x.x.x.x then don't use gethostbyname as Windows
     * is unable to handle octets which are out of range.
     */
    if (isDottedIPAddress(hostname, &addr[0])) {
	unsigned int address;
	jbyteArray barray;
	jobjectArray oarray;  

	/* 
	 * Are any of the octets out of range?
	 */
	if (addr[0] > 255 || addr[1] > 255 || addr[2] > 255 || addr[3] > 255) {
	    JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", hostname);
	    goto cleanupAndReturn;
	} 

	/*
	 * Return an byte array with the populated address.
	 */
	address = (addr[3]<<24) & 0xff000000;
	address |= (addr[2]<<16) & 0xff0000;
	address |= (addr[1]<<8) & 0xff00;
	address |= addr[0];

	byteArrayCls = (*env)->FindClass(env, "[B");
	if (byteArrayCls == NULL) {
	    goto cleanupAndReturn;
	}
	
	barray = (*env)->NewByteArray(env, 4);
	oarray = (*env)->NewObjectArray(env, 1, byteArrayCls, NULL);

	if (barray == NULL || oarray == NULL) {
	    /* pending exception */
	    goto cleanupAndReturn;
	}
	(*env)->SetByteArrayRegion(env, barray, 0, 4, (jbyte *)&address);
	(*env)->SetObjectArrayElement(env, oarray, 0, barray);

	JNU_ReleaseStringPlatformChars(env, host, hostname);
	return oarray;	
    }

    /*
     * Perform the lookup
     */
    if ((hp = JVM_GetHostByName((char*)hostname)) != NULL) {
	struct in_addr **addrp = (struct in_addr **) hp->h_addr_list;
	int len = sizeof(struct in_addr);
	int i = 0;

	while (*addrp != (struct in_addr *) 0) {
	    i++;
	    addrp++;
	}

	byteArrayCls = (*env)->FindClass(env, "[B");
	if (byteArrayCls == NULL) {
	    goto cleanupAndReturn;
	}

        ret = (*env)->NewObjectArray(env, i, byteArrayCls, NULL);
	if (IS_NULL(ret)) {
	    goto cleanupAndReturn;
	}

	addrp = (struct in_addr **) hp->h_addr_list;
	i = 0;
	while (*addrp != (struct in_addr *) 0) {
	    jbyteArray barray = (*env)->NewByteArray(env, len);
	    if (IS_NULL(barray)) {
		JNU_ThrowOutOfMemoryError(env, "lookupAllHostAddr");
		ret = NULL;
		goto cleanupAndReturn;
	    }
	    (*env)->SetByteArrayRegion(env, barray, 0, len, (jbyte *)(*addrp));
	    (*env)->SetObjectArrayElement(env, ret, i, barray);
	    addrp++;
	    i++;
	}
    } else {
        JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", hostname);
    }

cleanupAndReturn:
    JNU_ReleaseStringPlatformChars(env, host, hostname);
    return ret;
}

/*
 * Class:     java_net_Inet4AddressImpl
 * Method:    getHostByAddr
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_java_net_Inet4AddressImpl_getHostByAddr(JNIEnv *env, jobject this, 
					    jbyteArray addrArray) {
    struct hostent *hp;
    jbyte caddr[4];
    jint addr;
    (*env)->GetByteArrayRegion(env, addrArray, 0, 4, caddr);
    addr = ((caddr[0]<<24) & 0xff000000);
    addr |= ((caddr[1] <<16) & 0xff0000);
    addr |= ((caddr[2] <<8) & 0xff00);
    addr |= (caddr[3] & 0xff); 
    addr = htonl(addr);

    hp = JVM_GetHostByAddr((char *)&addr, sizeof(addr), AF_INET);
    if (hp == NULL) {
	JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", 0);
	return NULL;
    }
    if (hp->h_name == NULL) { /* Deal with bug in Windows XP */
	JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", 0);
	return NULL;
    }
    return JNU_NewStringPlatform(env, hp->h_name);
}
