/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;


/**
 * SSL/TLS records, as pulled off (and put onto) a TCP stream.  This is
 * the base interface, which defines common information and interfaces 
 * used by both Input and Output records.
 *
 * @version 1.21, 06/24/03
 * @author David Brownell
 */
interface Record {
    /*
     * There are four SSL record types, which are part of the interface
     * to this level (along with the maximum record size)
     *
     * enum { change_cipher_spec(20), alert(21), handshake(22),
     *      application_data(23), (255) } ContentType;
     */
    static final byte	ct_change_cipher_spec = 20;
    static final byte	ct_alert = 21;
    static final byte	ct_handshake = 22;
    static final byte	ct_application_data = 23;
    
    static final int    	headerSize = 5;		// SSLv3 record header
    static final int    	maxExpansion = 1024;	// for bad compression
    static final int    	trailerSize = 20;	// SHA1 hash size
    static final int		maxDataSize = 16384;	// 2^14 bytes of data
    static final int		maxPadding = 256;	// block cipher padding

    /*
     * SSL has a maximum record size.  It's header, (compressed) data,
     * padding, and a trailer for the MAC. 
     * Some compression algorithms have rare cases where they expand the data.
     * As we don't support compression at this time, leave that out.
     */
    static final int    	maxRecordSize =
				      headerSize	// header
				    + maxDataSize	// data
				    + maxPadding	// padding
				    + trailerSize;	// MAC
				    
    /*
     * Maximum record size for alert and change cipher spec records.
     * They only contain 2 and 1 bytes of data, respectively.
     * Allocate a smaller array.
     */				    
    static final int maxAlertRecordSize =
    			headerSize + 2 + maxPadding + trailerSize;

}
