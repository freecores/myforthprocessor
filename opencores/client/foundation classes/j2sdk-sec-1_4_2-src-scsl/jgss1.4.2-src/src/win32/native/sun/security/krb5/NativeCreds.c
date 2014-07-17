/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <string.h>
#define SECURITY_WIN32
#include <security.h>   
#include <ntsecapi.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <jni.h>
#include <winsock.h>

#undef LSA_SUCCESS
#define LSA_SUCCESS(Status) ((Status) >= 0) 
#define EXIT_FAILURE -1 // mdu

/*
 * Library-wide static references
 */

jclass derValueClass = NULL;
jclass ticketClass = NULL;
jclass principalNameClass = NULL;
jclass encryptionKeyClass = NULL;
jclass ticketFlagsClass = NULL;
jclass kerberosTimeClass = NULL;
jclass javaLangStringClass = NULL;

jmethodID derValueConstructor = 0;
jmethodID ticketConstructor = 0;
jmethodID principalNameConstructor = 0;
jmethodID encryptionKeyConstructor = 0;
jmethodID ticketFlagsConstructor = 0;
jmethodID kerberosTimeConstructor = 0;
jmethodID krbcredsConstructor = 0;

/* 
 * Function prototypes for internal routines
 *
 */

BOOL PackageConnectLookup(PHANDLE,PULONG);

VOID ShowNTError(LPSTR,NTSTATUS);

VOID
InitUnicodeString(
	PUNICODE_STRING DestinationString,
    PCWSTR SourceString OPTIONAL
    );

jobject BuildTicket(JNIEnv *env, PUCHAR encodedTicket, ULONG encodedTicketSize);

//mdu
jobject BuildClientPrincipal(JNIEnv *env, PKERB_EXTERNAL_NAME principalName);
jobject BuildTGSPrincipal(JNIEnv *env, UNICODE_STRING domainName);

jobject BuildEncryptionKey(JNIEnv *env, PKERB_CRYPTO_KEY cryptoKey);
jobject BuildTicketFlags(JNIEnv *env, PULONG flags);
jobject BuildKerberosTime(JNIEnv *env, PLARGE_INTEGER kerbtime);

/*
 * Class:     sun_security_krb5_KrbCreds
 * Method:    JNI_OnLoad
 */

JNIEXPORT jint JNICALL JNI_OnLoad(
		JavaVM	*jvm,
		void	*reserved) {

	jclass cls;
	JNIEnv *env;

	if ((*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_2)) {
		return JNI_EVERSION; /* JNI version not supported */
	}

	cls = (*env)->FindClass(env,"sun/security/krb5/internal/Ticket");

	if (cls == NULL) {
		printf("Couldn't find Ticket\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
   printf("Found Ticket\n");
   #endif /* DEBUG */

	ticketClass = (*env)->NewWeakGlobalRef(env,cls);
	if (ticketClass == NULL) {
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Made NewWeakGlobalRef\n");
   #endif /* DEBUG */

	cls = (*env)->FindClass(env,"sun/security/krb5/PrincipalName");

	if (cls == NULL) {
		printf("Couldn't find PrincipalName\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found PrincipalName\n");
   #endif /* DEBUG */

	principalNameClass = (*env)->NewWeakGlobalRef(env,cls);
	if (principalNameClass == NULL) {
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Made NewWeakGlobalRef\n");
   #endif /* DEBUG */

	cls = (*env)->FindClass(env,"sun/security/util/DerValue");

	if (cls == NULL) {
		printf("Couldn't find DerValue\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found DerValue\n");
   #endif /* DEBUG */

	derValueClass = (*env)->NewWeakGlobalRef(env,cls);
	if (derValueClass == NULL) {
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Made NewWeakGlobalRef\n");
   #endif /* DEBUG */

	cls = (*env)->FindClass(env,"sun/security/krb5/EncryptionKey");

	if (cls == NULL) {
		printf("Couldn't find EncryptionKey\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found EncryptionKey\n");
   #endif /* DEBUG */

	encryptionKeyClass = (*env)->NewWeakGlobalRef(env,cls);
	if (encryptionKeyClass == NULL) {
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Made NewWeakGlobalRef\n");
   #endif /* DEBUG */

	cls = (*env)->FindClass(env,"sun/security/krb5/internal/TicketFlags");

	if (cls == NULL) {
		printf("Couldn't find TicketFlags\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found TicketFlags\n");
   #endif /* DEBUG */

	ticketFlagsClass = (*env)->NewWeakGlobalRef(env,cls);
	if (ticketFlagsClass == NULL) {
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Made NewWeakGlobalRef\n");
   #endif /* DEBUG */

	cls = (*env)->FindClass(env,"sun/security/krb5/internal/KerberosTime");

	if (cls == NULL) {
		printf("Couldn't find KerberosTime\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found KerberosTime\n");
   #endif /* DEBUG */

	kerberosTimeClass = (*env)->NewWeakGlobalRef(env,cls);
	if (kerberosTimeClass == NULL) {
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Made NewWeakGlobalRef\n");
   #endif /* DEBUG */

	cls = (*env)->FindClass(env,"java/lang/String");

	if (cls == NULL) {
		printf("Couldn't find String\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found String\n");
   #endif /* DEBUG */

	javaLangStringClass = (*env)->NewWeakGlobalRef(env,cls);
	if (javaLangStringClass == NULL) {
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Made NewWeakGlobalRef\n");
   #endif /* DEBUG */

	derValueConstructor = (*env)->GetMethodID(env, derValueClass, "<init>", "([B)V");
	if (derValueConstructor == 0) {
		printf("Couldn't find DerValue constructor\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found DerValue constructor\n");
   #endif /* DEBUG */

	ticketConstructor = (*env)->GetMethodID(env, ticketClass, "<init>", "(Lsun/security/util/DerValue;)V");
	if (derValueConstructor == 0) {
		printf("Couldn't find Ticket constructor\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found Ticket constructor\n");
   #endif /* DEBUG */

	principalNameConstructor = (*env)->GetMethodID(env, principalNameClass, "<init>", "([Ljava/lang/String;)V");
	if (principalNameConstructor == 0) {
		printf("Couldn't find PrincipalName constructor\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found PrincipalName constructor\n");
   #endif /* DEBUG */

	encryptionKeyConstructor = (*env)->GetMethodID(env, encryptionKeyClass, "<init>", "(I[B)V");
	if (encryptionKeyConstructor == 0) {
		printf("Couldn't find EncryptionKey constructor\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found EncryptionKey constructor\n");
   #endif /* DEBUG */

	ticketFlagsConstructor = (*env)->GetMethodID(env, ticketFlagsClass, "<init>", "(I[B)V");
	if (ticketFlagsConstructor == 0) {
		printf("Couldn't find TicketFlags constructor\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found TicketFlags constructor\n");
   #endif /* DEBUG */

	kerberosTimeConstructor = (*env)->GetMethodID(env, kerberosTimeClass, "<init>", "(Ljava/lang/String;)V");
	if (kerberosTimeConstructor == 0) {
		printf("Couldn't find KerberosTime constructor\n");
		return JNI_ERR;
	}
	#ifdef DEBUG
	printf("Found KerberosTime constructor\n");
   #endif /* DEBUG */
		
	#ifdef DEBUG
	printf("Finished OnLoad processing\n");
   #endif /* DEBUG */

	return JNI_VERSION_1_2;
}

/*
 * Class:     sun_security_jgss_KrbCreds
 * Method:    JNI_OnUnload
 */

JNIEXPORT void JNICALL JNI_OnUnload(
		JavaVM	*jvm,
		void	*reserved) {

	JNIEnv *env;

	if ((*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_2)) {
		return; /* Nothing else we can do */
	}

	if (ticketClass != NULL) {
		(*env)->DeleteWeakGlobalRef(env,ticketClass);
	}
	if (derValueClass != NULL) {
		(*env)->DeleteWeakGlobalRef(env,derValueClass);
	}
	if (principalNameClass != NULL) {
		(*env)->DeleteWeakGlobalRef(env,principalNameClass);
	}
	if (encryptionKeyClass != NULL) {
		(*env)->DeleteWeakGlobalRef(env,encryptionKeyClass);
	}
	if (ticketFlagsClass != NULL) {
		(*env)->DeleteWeakGlobalRef(env,ticketFlagsClass);
	}
	if (kerberosTimeClass != NULL) {
		(*env)->DeleteWeakGlobalRef(env,kerberosTimeClass);
	}
	if (javaLangStringClass != NULL) {
		(*env)->DeleteWeakGlobalRef(env,javaLangStringClass);
	}

	return;
}

/*
 * Class:     sun_security_krb5_Credentials
 * Method:    acquireDefaultNativeCreds
 * Signature: ()Lsun/security/krb5/Credentials;
 */
JNIEXPORT jobject JNICALL Java_sun_security_krb5_Credentials_acquireDefaultNativeCreds(
		JNIEnv *env,
		jclass krbcredsClass) {

   HANDLE LogonHandle = NULL;
   ULONG PackageId;
	PKERB_RETRIEVE_TKT_REQUEST CacheRequest = NULL;
	PKERB_RETRIEVE_TKT_RESPONSE CacheResponse = NULL;
	ULONG rspSize = 0;
	DWORD errorCode;
	NTSTATUS Status,SubStatus;
	PUCHAR pEncodedTicket = NULL;
	jobject ticket, clientPrincipal, targetPrincipal, encryptionKey;
	jobject ticketFlags, startTime, endTime, krbCreds = NULL;
   jobject authTime, renewTillTime, hostAddresses = NULL;
	UNICODE_STRING Target = {0};
	UNICODE_STRING Target2 = {0};
	PDOMAIN_CONTROLLER_INFO DomainControllerInfo = NULL;
	WCHAR *tgtName = L"krbtgt";
	WCHAR *fullName;

	while (TRUE) {

	if (krbcredsConstructor == 0) {
		krbcredsConstructor = (*env)->GetMethodID(env, krbcredsClass, "<init>", 
       "(Lsun/security/krb5/internal/Ticket;Lsun/security/krb5/PrincipalName;Lsun/security/krb5/PrincipalName;Lsun/security/krb5/EncryptionKey;Lsun/security/krb5/internal/TicketFlags;Lsun/security/krb5/internal/KerberosTime;Lsun/security/krb5/internal/KerberosTime;Lsun/security/krb5/internal/KerberosTime;Lsun/security/krb5/internal/KerberosTime;Lsun/security/krb5/internal/HostAddresses;)V");
		if (krbcredsConstructor == 0) {
			printf("Couldn't find com.ibm.security.krb5.Credentials constructor\n");
			break;
		}
	}

	//printf("Found KrbCreds constructor\n");

    //
    // Get the logon handle and package ID from the
    // Kerberos package
    //
    if(!PackageConnectLookup(&LogonHandle, &PackageId))
        break;

	#ifdef DEBUG
	printf("Got handle to Kerberos package\n");
   #endif /* DEBUG */

	//InitUnicodeString(&Target2, L"krbtgt"); // this doesn't work 'cause I need the domain name too
	// OK, I don't give up that easily
	// Go get the current domain name
	errorCode = DsGetDcName(
					(LPCTSTR) NULL, // machine name
					(LPCTSTR) NULL, // DomainName, if NULL, I'm asking what it is
					(GUID *)  NULL, // DomainGuid,
					(LPCTSTR) NULL, // SiteName,
					DS_GC_SERVER_REQUIRED, //Flags
					&DomainControllerInfo);
	if (errorCode != NO_ERROR) {
		printf("DsGetDcName returned %d\n", errorCode);
		break;
	}

	#ifdef DEBUG
	printf("The domain name is %S\n", DomainControllerInfo->DomainName);
   #endif /* DEBUG */
	// Build a fully-qualified name
	fullName = (WCHAR *) LocalAlloc(LMEM_ZEROINIT,((wcslen(tgtName)+wcslen(L"/")+wcslen(DomainControllerInfo->DomainName)) * sizeof(WCHAR) + sizeof(UNICODE_NULL)));
	wcscat(fullName, tgtName);
	wcscat(fullName, L"/");
	wcscat(fullName, DomainControllerInfo->DomainName);
	#ifdef DEBUG
	printf("The fully-qualified name is %S\n", fullName);
   #endif /* DEBUG */
	InitUnicodeString(&Target2, fullName);       

    CacheRequest = (PKERB_RETRIEVE_TKT_REQUEST)
            LocalAlloc(LMEM_ZEROINIT, Target2.Length + sizeof(KERB_RETRIEVE_TKT_REQUEST));

    CacheRequest->MessageType = KerbRetrieveEncodedTicketMessage ;
 
    Target.Buffer = (LPWSTR) (CacheRequest + 1);
    Target.Length = Target2.Length;
    Target.MaximumLength = Target2.MaximumLength;

    CopyMemory(
        Target.Buffer,
        Target2.Buffer,
        Target2.Length
        );

    CacheRequest->TargetName = Target;	
    CacheRequest->EncryptionType = KERB_ETYPE_DES_CBC_MD5; // mdu

	Status = LsaCallAuthenticationPackage(
                LogonHandle,
                PackageId,
                CacheRequest,
                Target2.Length + sizeof(KERB_RETRIEVE_TKT_REQUEST),
                (PVOID *) &CacheResponse,
                &rspSize,
                &SubStatus
                );

	#ifdef DEBUG
	printf("Response size is %d\n", rspSize);
   #endif /* DEBUG */
	LocalFree(fullName);

	if (!LSA_SUCCESS(Status) || !LSA_SUCCESS(SubStatus))
    {
        if (!LSA_SUCCESS(Status)) {
			ShowNTError("LsaCallAuthenticationPackage", Status);
		}
		else {
			ShowNTError("Protocol status", SubStatus);
		}
		break;
    }

	// Now we need to skip over most of the junk in the buffer to get to the ticket
	// Here's what we're looking at...

/*

typedef struct _KERB_RETRIEVE_TKT_RESPONSE {
    KERB_EXTERNAL_TICKET Ticket;
} KERB_RETRIEVE_TKT_RESPONSE, *PKERB_RETRIEVE_TKT_RESPONSE;

typedef struct _KERB_EXTERNAL_TICKET {
    PKERB_EXTERNAL_NAME ServiceName;
    PKERB_EXTERNAL_NAME TargetName;
    PKERB_EXTERNAL_NAME ClientName;
    UNICODE_STRING DomainName;
    UNICODE_STRING TargetDomainName;
    UNICODE_STRING AltTargetDomainName;
    KERB_CRYPTO_KEY SessionKey;
    ULONG TicketFlags;
    ULONG Flags;
    LARGE_INTEGER KeyExpirationTime;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    LARGE_INTEGER RenewUntil;
    LARGE_INTEGER TimeSkew;
    ULONG EncodedTicketSize;
    PUCHAR EncodedTicket;					<========== Here's the good stuff
} KERB_EXTERNAL_TICKET, *PKERB_EXTERNAL_TICKET;

typedef struct _KERB_EXTERNAL_NAME {
    SHORT NameType;
    USHORT NameCount;
    UNICODE_STRING Names[ANYSIZE_ARRAY];
} KERB_EXTERNAL_NAME, *PKERB_EXTERNAL_NAME;

typedef struct _LSA_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING;

typedef LSA_UNICODE_STRING UNICODE_STRING, *PUNICODE_STRING;

typedef struct KERB_CRYPTO_KEY {
    LONG KeyType;
    ULONG Length;
    PUCHAR Value;
} KERB_CRYPTO_KEY, *PKERB_CRYPTO_KEY;

*/

	// Build a com.ibm.security.krb5.Ticket
	ticket = BuildTicket(env, CacheResponse->Ticket.EncodedTicket, CacheResponse->Ticket.EncodedTicketSize);
	if (ticket == NULL) {
		break;
	}
	// OK, have a Ticket, now need to get the client name
	clientPrincipal = BuildClientPrincipal(env, CacheResponse->Ticket.ClientName); // mdu
	if (clientPrincipal == NULL) break;
	// and the "name" of tgt
	targetPrincipal = BuildTGSPrincipal(env, CacheResponse->Ticket.TargetDomainName); // mdu
	if (targetPrincipal == NULL) break;
	// Get the encryption key
	encryptionKey = BuildEncryptionKey(env, &(CacheResponse->Ticket.SessionKey));
	if (encryptionKey == NULL) break;
	// and the ticket flags
	ticketFlags = BuildTicketFlags(env, &(CacheResponse->Ticket.TicketFlags));
	if (ticketFlags == NULL) break;
	// Get the start time
	startTime = BuildKerberosTime(env, &(CacheResponse->Ticket.StartTime));
	if (startTime == NULL) break;
	/*
	 * mdu: No point storing the eky expiration time in the auth
	 * time field. Set it to be same as startTime. Looks like
	 * windows does not have post-dated tickets.
	 */
	authTime = startTime;
	// and the end time
	endTime = BuildKerberosTime(env, &(CacheResponse->Ticket.EndTime));
	if (endTime == NULL) break;
	// Get the renew till time
	renewTillTime = BuildKerberosTime(env, &(CacheResponse->Ticket.RenewUntil));
	if (renewTillTime == NULL) break;
	// and now go build a KrbCreds object
	krbCreds = (*env)->NewObject(
		env,
		krbcredsClass,
		krbcredsConstructor,
		ticket,
		clientPrincipal,
		targetPrincipal,
		encryptionKey,
		ticketFlags,
		authTime, // mdu
		startTime,
		endTime,
		renewTillTime, //mdu
		hostAddresses);
	break;
	} // end of WHILE

	if (DomainControllerInfo != NULL) {
		NetApiBufferFree(DomainControllerInfo);
	}
    if (CacheResponse != NULL) {
        LsaFreeReturnBuffer(CacheResponse);
    }
    if (CacheRequest) {
        LocalFree(CacheRequest);
    }
	return krbCreds;
}


BOOL 
PackageConnectLookup(
    HANDLE *pLogonHandle, 
    ULONG *pPackageId
    )
{
    LSA_STRING Name;
    NTSTATUS Status;

    Status = LsaConnectUntrusted(
                pLogonHandle
                );

    if (!LSA_SUCCESS(Status))
    {
        ShowNTError("LsaConnectUntrusted", Status);
        return FALSE;
    }

    Name.Buffer = MICROSOFT_KERBEROS_NAME_A;
    Name.Length = strlen(Name.Buffer);
    Name.MaximumLength = Name.Length + 1;

    Status = LsaLookupAuthenticationPackage(
                *pLogonHandle,
                &Name,
                pPackageId
                );

    if (!LSA_SUCCESS(Status))
    {
        ShowNTError("LsaLookupAuthenticationPackage", Status);
        return FALSE;
    }

    return TRUE;

}

VOID 
ShowLastError(
	LPSTR szAPI, 
	DWORD dwError
	)
{
   #define MAX_MSG_SIZE 256

   static WCHAR szMsgBuf[MAX_MSG_SIZE];
   DWORD dwRes;

   printf("Error calling function %s: %lu\n", szAPI, dwError);

   dwRes = FormatMessage (
      FORMAT_MESSAGE_FROM_SYSTEM,
      NULL,
      dwError,
      MAKELANGID (LANG_ENGLISH, SUBLANG_ENGLISH_US),
      szMsgBuf,
      MAX_MSG_SIZE,
      NULL);
   if (0 == dwRes) {
      printf("FormatMessage failed with %d\n", GetLastError());
      ExitProcess(EXIT_FAILURE);
   }

   printf("%S",szMsgBuf);
}

VOID 
ShowNTError( 
	LPSTR szAPI, 
	NTSTATUS Status 
	) 
{     
    // 
    // Convert the NTSTATUS to Winerror. Then call ShowLastError().     
    // 
    ShowLastError(szAPI, LsaNtStatusToWinError(Status));
} 

VOID
InitUnicodeString(
	PUNICODE_STRING DestinationString,
    PCWSTR SourceString OPTIONAL
    )
{
    ULONG Length;

    DestinationString->Buffer = (PWSTR)SourceString;
    if (SourceString != NULL) {
        Length = wcslen( SourceString ) * sizeof( WCHAR );
        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = (USHORT)(Length + sizeof(UNICODE_NULL));
        }
    else {
        DestinationString->MaximumLength = 0;
        DestinationString->Length = 0;
        }
}

jobject BuildTicket(JNIEnv *env, PUCHAR encodedTicket, ULONG encodedTicketSize) {

	/* To build a Ticket, we first need to build a DerValue out of the EncodedTicket.
	 * But before we can do that, we need to make a byte array out of the ET.
	 */

	jobject derValue, ticket;
	jbyteArray ary;

	ary = (*env)->NewByteArray(env,encodedTicketSize);
	if ((*env)->ExceptionOccurred(env)) {
		return (jobject) NULL;
	}

	(*env)->SetByteArrayRegion(env, ary, (jsize) 0, encodedTicketSize, (jbyte *)encodedTicket);
	if ((*env)->ExceptionOccurred(env)) {
		(*env)->DeleteLocalRef(env, ary);
		return (jobject) NULL;
	}

	derValue = (*env)->NewObject(env, derValueClass, derValueConstructor, ary);
	if ((*env)->ExceptionOccurred(env)) {
		(*env)->DeleteLocalRef(env, ary);
		return (jobject) NULL;
	}

	(*env)->DeleteLocalRef(env, ary);
	ticket = (*env)->NewObject(env, ticketClass, ticketConstructor, derValue);
	if ((*env)->ExceptionOccurred(env)) {
		(*env)->DeleteLocalRef(env, derValue);
		return (jobject) NULL;
	}
	(*env)->DeleteLocalRef(env, derValue);
	return ticket;	
}

// mdu
jobject BuildClientPrincipal(JNIEnv *env, PKERB_EXTERNAL_NAME principalName) {
	/*
	 * To build the Principal, we need to get the names out of this goofy MS structure
	 */
	jobject principal = NULL, stringArray;
	jstring tempString;
	int nameCount,i;
	PUNICODE_STRING scanner;
#define MINBUFFSIZE 256
	ULONG bufferSize = MINBUFFSIZE;
	HANDLE processHeap = GetProcessHeap();
//	WCHAR *fullName = (WCHAR *) HeapAlloc(processHeap, HEAP_ZERO_MEMORY, bufferSize);

	#ifdef DEBUG
	printf("Name type is %x\n", principalName->NameType);
	printf("Name count is %x\n", principalName->NameCount);
   #endif /* DEBUG */
	nameCount = principalName->NameCount;

	stringArray = (*env)->NewObjectArray(env, nameCount, javaLangStringClass, NULL);

	if (stringArray == NULL) {
		printf("Can't allocate String array for Principal\n");
		return principal;
	}

	for (i=0; i<nameCount; i++) {
		scanner = &(principalName->Names[i]); // Pick out a UNICODE_STRING
		//while (TRUE) { // Provide a retry loop for memory allocation
		//	if (scanner->Length + sizeof(WCHAR) + wcslen(fullName) + sizeof(UNICODE_NULL) > bufferSize) {
		//		fullName = (WCHAR *) HeapReAlloc(processHeap, HEAP_ZERO_MEMORY, fullName, bufferSize*2);
		//		bufferSize += bufferSize;
		//	}
		//	// gotta copy memory here, as the "string" might not be zero-terminated
		//	CopyMemory(fullName+wcslen(fullName), scanner->Buffer, scanner->Length);
		//	break;
		//}
		// OK, got a Char array, so construct a String
		tempString = (*env)->NewString(env, (const jchar*)scanner->Buffer, scanner->Length/sizeof(WCHAR)); // mdu
		// Set the String into the StringArray
		(*env)->SetObjectArrayElement(env, stringArray, i, tempString);		
		// Do I have to worry about storage reclamation here?
	}
	principal = (*env)->NewObject(env, principalNameClass, principalNameConstructor, stringArray);

	return principal;
}

// mdu
jobject BuildTGSPrincipal(JNIEnv *env, 
			  UNICODE_STRING domainName) {
	jobject principal = NULL, stringArray;
	jstring tempString;
	int nameCount;
	PUNICODE_STRING scanner;
        UNICODE_STRING KRBTGT = {0};
	InitUnicodeString(&KRBTGT, L"krbtgt");

	stringArray = (*env)->NewObjectArray(env, 2, 
					     javaLangStringClass, NULL);

	if (stringArray == NULL) {
		printf("Can't allocate String array for TGS Principal\n");
		return principal;
	}

	// OK, got a Char array, so construct a String

	scanner = &KRBTGT;
	tempString = (*env)->NewString(env, 
				       (const jchar*)scanner->Buffer, 
				       scanner->Length/sizeof(WCHAR));

	// Set the String into the StringArray
	(*env)->SetObjectArrayElement(env, stringArray, 0, tempString);		
	// Do I have to worry about storage reclamation here?

	// OK, got a Char array, so construct a String
	scanner = &domainName; // The unicode domain name
	tempString = (*env)->NewString(env, 
				       (const jchar*)scanner->Buffer, 
				       scanner->Length/sizeof(WCHAR));

	// Set the String into the StringArray
	(*env)->SetObjectArrayElement(env, stringArray, 1, tempString);		
	// Do I have to worry about storage reclamation here?

	principal = (*env)->NewObject(env, principalNameClass, 
				      principalNameConstructor, stringArray);

	return principal;
}


jobject BuildEncryptionKey(JNIEnv *env, PKERB_CRYPTO_KEY cryptoKey) {
	// First, need to build a byte array
	jbyteArray ary;
	jobject encryptionKey = NULL;

	ary = (*env)->NewByteArray(env,cryptoKey->Length);
	(*env)->SetByteArrayRegion(env, ary, (jsize) 0, cryptoKey->Length, (jbyte *)cryptoKey->Value);
	if ((*env)->ExceptionOccurred(env)) {
		(*env)->DeleteLocalRef(env, ary);
	} else {
		encryptionKey = (*env)->NewObject(env, encryptionKeyClass, encryptionKeyConstructor, cryptoKey->KeyType, ary);
	}

	return encryptionKey;
}

jobject BuildTicketFlags(JNIEnv *env, PULONG flags) {
	jobject ticketFlags = NULL;
	jbyteArray ary;
	/*
	 * mdu: Convert the bytes to nework byte order before copying
	 * them to a Java byte array.
	 */
	ULONG nlflags = htonl(*flags);

	ary = (*env)->NewByteArray(env, sizeof(*flags));
	(*env)->SetByteArrayRegion(env, ary, (jsize) 0, sizeof(*flags), (jbyte *)&nlflags);//mdu 
	if ((*env)->ExceptionOccurred(env)) {
		(*env)->DeleteLocalRef(env, ary);
	} else {
		ticketFlags = (*env)->NewObject(env, ticketFlagsClass, ticketFlagsConstructor, sizeof(*flags)*8, ary);
	}

	return ticketFlags;
}

jobject BuildKerberosTime(JNIEnv *env, PLARGE_INTEGER kerbtime) {
	jobject kerberosTime = NULL;
	jstring stringTime = NULL;
	SYSTEMTIME systemTime;
	WCHAR timeString[16];
	WCHAR month[3];
	WCHAR day[3];
	WCHAR hour[3];
	WCHAR minute[3];
	WCHAR second[3];

	if (FileTimeToSystemTime((FILETIME *)kerbtime, &systemTime)) {
//		printf( "%ld/%ld/%ld %ld:%2.2ld:%2.2ld\n",
//                systemTime.wYear,
//                systemTime.wMonth,
//                systemTime.wDay,
//                systemTime.wHour,
//                systemTime.wMinute,
//                systemTime.wSecond );
//		swprintf( (wchar_t *)timeString,
//				L"%ld%ld%ld%02.2ld%02.2ld%02.2ld\n",
//                systemTime.wYear,
//                systemTime.wMonth,
//                systemTime.wDay,
//                systemTime.wHour,
//                systemTime.wMinute,
//                systemTime.wSecond );
// OK, this is frustrating.  I want the month, day, hour, minute, second to be two digits.
// So, gee, I can use %02.2ld.  Well, no, I can't because the leading 0 is ignored for
// integers.  So, print them to strings, and then print them to the master string with a
// format pattern that says make it two digits and prefix with a 0 if necessary.
//FIXME: Hmm, is there a value for "infinite" that I need to accomodate here?
		swprintf( (wchar_t *)month, L"%2.2d", systemTime.wMonth);
		swprintf( (wchar_t *)day, L"%2.2d", systemTime.wDay);
		swprintf( (wchar_t *)hour, L"%2.2d", systemTime.wHour);
		swprintf( (wchar_t *)minute, L"%2.2d", systemTime.wMinute);
		swprintf( (wchar_t *)second, L"%2.2d", systemTime.wSecond);
		swprintf( (wchar_t *)timeString,
				L"%ld%02.2s%02.2s%02.2s%02.2s%02.2sZ",
                systemTime.wYear,
                month,
                day,
                hour,
                minute,
                second );
	#ifdef DEBUG
		printf("%S\n", (wchar_t *)timeString);
   #endif /* DEBUG */
		stringTime = (*env)->NewString(env, timeString, (sizeof(timeString)/sizeof(WCHAR))-1);
		if (stringTime != NULL) { // everything's OK so far
			kerberosTime = (*env)->NewObject(env, kerberosTimeClass, kerberosTimeConstructor, stringTime);
		}
	}
	return kerberosTime;
}

