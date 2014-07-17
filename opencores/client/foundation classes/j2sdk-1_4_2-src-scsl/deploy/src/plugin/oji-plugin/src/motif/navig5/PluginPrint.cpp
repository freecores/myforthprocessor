/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)PluginPrint.cpp	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <stdio.h>
#include "remotejni.h"
#include "nsIPluginInstance.h"
#include "nsIJVMPluginInstance.h"
#include "nsIEventHandler.h"
#include "nsplugin.h"
#include "Navig5.h"
#include "nsplugindefs.h"
#include "PluginPrint.h"

PluginPrint::PluginPrint(JavaPluginInstance5 *inst, 
			 nsPluginPrint* printinfo)
  :mInst(inst), mPrintInfo(printinfo)
{
  mInst->AddRef();
}

/* Desctruction */
PluginPrint::~PluginPrint()
{
  if (mInst != NULL)
    mInst->Release();
  
}

/* Actual Printing method */
NS_IMETHODIMP PluginPrint::Print()
{
  if (mPrintInfo->mode == nsPluginMode_Full)
    {
      return FullPrint();
    }
  else 
    {
      return EmbedPrint();
    }
}


/* If the request was a fullPrint */
NS_IMETHODIMP PluginPrint::FullPrint()
{
  mPrintInfo->print.fullPrint.pluginPrinted=PR_FALSE;
  return NS_OK;
}

/*
 * Embed Print does the following:
 * 1) Send message to Plugin to print requested area
 * 2) Request is send and received ack that print pipe is ready
 * 3) Poll the printpipe untile you receive ok message
 * 
 *
 */
NS_IMETHODIMP PluginPrint::EmbedPrint()
{
  FILE *fp;
  JavaVM5 *jvm ;
  sendRequest();
  nsPluginEmbedPrint ep;
  ep = mPrintInfo->print.embedPrint;
  void *platformPrint = (nsPluginPrintCallbackStruct*)(ep.platformPrint);
  
  fp = ((nsPluginPrintCallbackStruct *)(platformPrint))->fp;
  jvm =mInst->GetPluginFactory()->GetJavaVM();
  jvm->ReceivePrinting(fp);
  
  return NS_OK;
}

void PluginPrint::sendRequest()
{
  CWriteBuffer wb;
  nsPluginEmbedPrint embedPrint= mPrintInfo->print.embedPrint;
  nsPluginWindow window=embedPrint.window;
  wb.putInt(JAVA_PLUGIN_PRINT);
  wb.putInt( mInst->GetPluginNumber());
  wb.putInt(window.x);
  wb.putInt(window.y);
  wb.putInt(window.width);
  wb.putInt(window.height);
  mInst->GetPluginFactory()->SendRequest(wb, FALSE);
} 
