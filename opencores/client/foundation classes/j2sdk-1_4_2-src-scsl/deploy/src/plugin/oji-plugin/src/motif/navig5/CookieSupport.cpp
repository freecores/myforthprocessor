/*
 * @(#)CookieSupport.cpp	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdio.h>
#include <stdlib.h>

#include "remotejni.h"
#include "nsIJVMPluginInstance.h"
#include "Navig5.h"
#include "nsICookieStorage.h"
#include "CookieSupport.h"

CookieSupport::CookieSupport(JavaVM5 *jvm) : mJvm(jvm)
{
}

/*
 * Gets the cookies from nsICookieStorage and sends it to the requester
 */
void CookieSupport::FindCookieForURL(JavaPluginInstance5 *inst, 
				     const char *url){

  PRUint32 cookieSize =  MAX_COOKIE;
  char cookieResult[MAX_COOKIE];
  nsICookieStorage *cookieStorage=mJvm->GetPluginFactory()->GetCookieStorage();
  if (cookieStorage->GetCookie(url, 
			       (void*) cookieResult, 
			       cookieSize ) == NS_OK)
    {
      ReplyCookie(cookieResult, cookieSize, inst->GetPluginNumber());
    }
    else
    {
      /* The applet is waiting on the pipe for a reply.  We must provide one!! */
      ReplyCookie(" ",1,inst->GetPluginNumber());
    }
  return;
}

/*
* Sets the cookies in nsICookieStorage from to the requester
*/
void CookieSupport::SetCookieForURL(const char *url, const char* cookie)
{
    nsICookieStorage *cookieStorage=mJvm->GetPluginFactory()->GetCookieStorage();

    // Set the cookie in the browser
    cookieStorage->SetCookie(url, (void*) cookie, strlen(cookie));

    return;
}

/*
 * Replies to cookie requestor message
 */
void CookieSupport::ReplyCookie(const char *cookie, int len, int pnum){

    CWriteBuffer wb;
    trace("CookieSupport reply for an original JAVA_PLUGIN_COOKIE_REQUEST\n");
    wb.putInt(JAVA_PLUGIN_COOKIE);
    wb.putInt(pnum);
    wb.putString(cookie);
    mJvm->SendRequest(wb, FALSE);
}
