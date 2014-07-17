/*
 * @(#)AudioDataStream.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;

import java.io.*;
import javax.sound.sampled.*;
import javax.sound.midi.*;

/**
 * An input stream to play AudioData.
 *
 * @see AudioPlayer
 * @see AudioData
 * @author Arthur van Hoff
 * @author Kara Kytle
 * @version 1.8 03/01/23
 */
public class AudioDataStream extends ByteArrayInputStream {
    
    AudioData ad;
    
    /**
     * Constructor
     */
    public AudioDataStream(AudioData data) {
	
	super(data.buffer);
	this.ad = data;
    }
    
    AudioData getAudioData() {
	return ad;
    }
}
