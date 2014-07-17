/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)CPluginPrint.h	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifndef PLUGINPRINT_H
#define PLUGINPRINT_H

#include "nsplugindefs.h"

class CJavaPluginInstance;

class CPluginPrint{
 public:
  /* Construction of CPluginPrint */
  CPluginPrint(CJavaPluginInstance *inst, nsPluginPrint* printinfo);
  
  /* Desctruction */
  virtual ~CPluginPrint(void);

  /* Actual Printing method */
  NS_IMETHOD Print();
  
  /* If the request was a fullPrint */
  NS_IMETHOD FullPrint();

  /* For the request of embedPrint */
  NS_IMETHOD EmbedPrint();

 protected:

  void sendRequest();
  
  /*Pointer to JavaPluginInstance */
  CJavaPluginInstance *mInst;

  /* pointer to print info */
  nsPluginPrint *mPrintInfo;

  /*File Name */
  char mFileName[50];

};

#endif /*PLUGINPRINT_H */
  
