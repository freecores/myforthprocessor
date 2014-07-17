/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)PluginPrint.h	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifndef PLUGINPRINT_H
#define PLUGINPRINT_H

#include "nsplugindefs.h"

class JavaPluginInstance5;

class PluginPrint{
 public:
  /* Construction of PluginPrint */
  PluginPrint(JavaPluginInstance5 *inst, nsPluginPrint* printinfo);
  
  /* Desctruction */
  virtual ~PluginPrint(void);

  /* Actual Printing method */
  NS_IMETHOD Print();
  
  /* If the request was a fullPrint */
  NS_IMETHOD FullPrint();

  /* For the request of embedPrint */
  NS_IMETHOD EmbedPrint();

 protected:

  void sendRequest();
  
  /*Pointer to JavaPluginInstance */
  JavaPluginInstance5 *mInst;

  /* pointer to print info */
  nsPluginPrint *mPrintInfo;

};

#endif /*PLUGINPRINT_H */
  
