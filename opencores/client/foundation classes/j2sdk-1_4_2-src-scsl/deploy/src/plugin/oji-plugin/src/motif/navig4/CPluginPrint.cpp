/*
 * @(#)CPluginPrint.cpp	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include "Navig4.h"
#include "CPluginPrint.h"

CPluginPrint::CPluginPrint(CJavaPluginInstance *inst, 
			 nsPluginPrint* printinfo)
  :mInst(inst), mPrintInfo(printinfo)
{
  mInst->AddRef();
}

/* Desctruction */
CPluginPrint::~CPluginPrint()
{
  if (mInst != NULL)
    mInst->Release();
  
}

/* Actual Printing method */
NS_IMETHODIMP CPluginPrint::Print()
{
  if (mPrintInfo->mode == nsPluginMode_Full)
    {
      return FullPrint();
    }
  else //if (mPrintInfo->mode == nsPluginEmbedPrint)
    {
      return EmbedPrint();
    }
}


/* If the request was a fullPrint */
NS_IMETHODIMP CPluginPrint::FullPrint()
{
  mPrintInfo->print.fullPrint.pluginPrinted=PR_FALSE;
  return NS_OK;
}

/*
 * Embed Print does the following:
 * 1) Send message to Plugin to print requested area
 * 2) Request is send and received ack that print pipe is ready
 * 3) Poll the PrintFD
 * 
 *
 */
NS_IMETHODIMP CPluginPrint::EmbedPrint()
{
  FILE *fp;
  CJavaVM *jvm ;
  sendRequest();
  nsPluginEmbedPrint ep;
  ep = mPrintInfo->print.embedPrint;
  void *platformPrint = (nsPluginPrintCallbackStruct*)(ep.platformPrint);
  
  fp = ((nsPluginPrintCallbackStruct *)(platformPrint))->fp;
  mInst->GetPluginFactory()->GetPluginJavaVM(&jvm);
  jvm->ReceivePrinting(fp);
  
  return NS_OK;
}

void CPluginPrint::sendRequest()
{
  nsPluginEmbedPrint embedPrint= mPrintInfo->print.embedPrint;
  nsPluginWindow window=embedPrint.window;
  char buff[24];
  put_int(buff, 0 , JAVA_PLUGIN_PRINT);
  put_int(buff, 4,  mInst->GetPluginNumber());
  put_int(buff, 8, window.x);
  put_int(buff, 12, window.y);
  put_int(buff, 16, window.width);
  put_int(buff, 20, window.height);
  mInst->GetPluginFactory()->SendRequest(buff, 24, FALSE);
} 
