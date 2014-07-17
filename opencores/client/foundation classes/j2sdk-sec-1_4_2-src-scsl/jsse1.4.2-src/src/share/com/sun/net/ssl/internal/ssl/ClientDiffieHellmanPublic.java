/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.IOException;
import java.io.PrintStream;
import java.math.BigInteger;


/*
 * Message used by clients to send their Diffie-Hellman public
 * keys to servers.
 *
 * @version 1.11, 06/24/03
 * @author David Brownell
 */
final class ClientDiffieHellmanPublic extends HandshakeMessage {

    protected int messageType() {
        return ht_client_key_exchange;
    }

    /*
     * This value may be empty if it was included in the
     * client's certificate ...
     */
    private byte dh_Yc[];               // 1 to 2^16 -1 bytes

    public BigInteger getClientPublicKey() {
        return new BigInteger(1, dh_Yc);
    }

    /*
     * Either pass the client's public key explicitly (because it's
     * using DHE or DH_anon), or implicitly (the public key was in the
     * certificate).
     */
    ClientDiffieHellmanPublic(BigInteger publicKey) {
	dh_Yc = toByteArray(publicKey);
    }

    ClientDiffieHellmanPublic() {
	dh_Yc = null;
    }

    /*
     * Get the client's public key either explicitly or implicitly.
     * (It's ugly to have an empty record be sent in the latter case,
     * but that's what the protocol spec requires.)
     */
    ClientDiffieHellmanPublic(HandshakeInStream input) throws IOException {
        dh_Yc = input.getBytes16();
    }

    protected int messageLength() {
	if (dh_Yc == null) {
	    return 0;
	} else {
	    return dh_Yc.length + 2;
	}
    }

    protected void send(HandshakeOutStream s) throws IOException {
	s.putBytes16(dh_Yc);
    }

    void print(PrintStream s) throws IOException {
        s.println("*** ClientDiffieHellmanPublic");

	if (debug != null && Debug.isOn("verbose")) {
	    Debug.println(s, "DH Public key", dh_Yc);
	}
    }
}
