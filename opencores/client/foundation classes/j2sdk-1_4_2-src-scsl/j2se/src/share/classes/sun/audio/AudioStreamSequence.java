/*
 * @(#)AudioStreamSequence.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;

import java.io.IOException;
import java.io.InputStream;
import java.io.SequenceInputStream;
import java.util.Enumeration;

/**
 * Convert a sequence of input streams into a single InputStream.
 * This class can be used to play two audio clips in sequence.<p>
 * For example:
 * <pre>
 *	Vector v = new Vector();
 *	v.addElement(audiostream1);
 *	v.addElement(audiostream2);
 *	AudioStreamSequence audiostream = new AudioStreamSequence(v.elements());
 *	AudioPlayer.player.start(audiostream);
 * </pre>
 * @see AudioPlayer
 * @author Arthur van Hoff
 * @version 1.6 03/01/23
 */
public
    class AudioStreamSequence extends SequenceInputStream {
	Enumeration e;
	InputStream in;
    
	/**
	 * Create an AudioStreamSequence given an
	 * enumeration of streams.
	 */
	public AudioStreamSequence(Enumeration e) {
	    super(e);
	}
    
    }
