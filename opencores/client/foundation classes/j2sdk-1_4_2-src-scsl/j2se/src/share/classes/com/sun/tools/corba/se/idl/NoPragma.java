/*
 * @(#)NoPragma.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * COMPONENT_NAME: idl.parser
 *
 * ORIGINS: 27
 *
 * Licensed Materials - Property of IBM
 * 5639-D57 (C) COPYRIGHT International Business Machines Corp. 1997, 1999
 * RMI-IIOP v1.0
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * @(#)NoPragma.java	1.11 03/01/23
 */

package com.sun.tools.corba.se.idl;

// NOTES:

import java.io.IOException;

class NoPragma extends PragmaHandler
{
  public boolean process (String pragma, String currentToken) throws IOException
  {
    parseException (Util.getMessage ("Preprocessor.unknownPragma", pragma));
    skipToEOL ();
    return true;
  } // process
} // class NoPragma
