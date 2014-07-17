/*
 * @(#)InvalidAudioFormatException.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;
import java.io.IOException;

/**
 * Signals an invalid audio stream for the stream handler.
 */
class InvalidAudioFormatException extends IOException {


    /**
     * Constructor.
     */
    public InvalidAudioFormatException() {
	super();
    }

    /**
     * Constructor with a detail message.
     */
    public InvalidAudioFormatException(String s) {
	super(s);
    }
}
