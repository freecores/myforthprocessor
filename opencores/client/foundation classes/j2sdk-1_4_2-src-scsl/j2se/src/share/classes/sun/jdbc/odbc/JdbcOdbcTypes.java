/*
 * @(#)JdbcOdbcTypes.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcTypes.java
//
// Description: Defines ODBC constants
//
// Product:     JDBCODBC (Java DataBase Connectivity using
//              Open DataBase Connectivity)
//
// Author:      Jesse Davis
//
// Date:        March, 2001
//
//----------------------------------------------------------------------------

package sun.jdbc.odbc;

import java.sql.Types;

public class JdbcOdbcTypes {

    /* This is for ODBC drivers returning the SQL_WCHAR datatype */
	public final static int NCHAR 		=  -8;

    /* This is for ODBC drivers returning the SQL_WCHAR datatype */
	public final static int NVARCHAR 		=  -9;

    /* This is for ODBC drivers returning the SQL_WCHAR datatype */
	public final static int NLONGVARCHAR 		=  -10;



    // Prevent instantiation
    private JdbcOdbcTypes() {}
}


