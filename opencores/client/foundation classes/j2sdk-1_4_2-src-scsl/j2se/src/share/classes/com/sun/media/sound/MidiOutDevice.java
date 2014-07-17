/*
 * @(#)MidiOutDevice.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.util.Vector;
import java.util.StringTokenizer;	// for parsing classpath

import java.net.URL;
import java.net.MalformedURLException;

import java.io.File;
import java.io.IOException;

import javax.sound.midi.*;



/******************************************************************************************
IMPLEMENTATION TODO:


******************************************************************************************/



/**
 * MidiOutDevice class representing functionality of MidiOut devices.
 *
 * @version 1.14, 03/01/23
 * @author David Rivas
 * @author Kara Kytle
 * @author Florian Bomers
 */
class MidiOutDevice extends AbstractMidiDevice /*implements Receiver*/ {


    // INSTANCE VARIABLES

    private Vector receivers	= new Vector();

    // CONSTRUCTOR

    MidiOutDevice(AbstractMidiDeviceProvider.Info info) {
	super(info);
	if(Printer.trace) Printer.trace("MidiOutDevice CONSTRUCTOR");
    }


    // IMPLEMENTATION OF ABSTRACT MIDI DEVICE METHODS

    synchronized void implOpen() throws MidiUnavailableException {
	id = nOpen( ((AbstractMidiDeviceProvider.Info)getDeviceInfo()).getIndex() );
	if (id == 0) {
	    throw new MidiUnavailableException("Unable to open native device");
	}
    }


    synchronized void implClose() {
	// prevent further action
	long oldId = id;
	id = 0;

	// close all receivers
	for(int i = 0; i < receivers.size(); i++) {
	    ((Receiver)receivers.elementAt(i)).close();
	}
	receivers.removeAllElements();

	// close the device
	nClose(oldId);
    }


    // OVERRIDES OF ABSTRACT MIDI DEVICE METHODS

    public int getMaxReceivers() {
	// $$kk: 09.27.99: need a not-specified value here
	return -1;
    }


    public synchronized Receiver getReceiver() throws MidiUnavailableException {
	Receiver receiver = new MidiOutReceiver();
	receivers.addElement(receiver);
	return receiver;
    }


    // INNER CLASSES

    class MidiOutReceiver implements Receiver {

	private boolean open = true;

	public synchronized void send(MidiMessage message, long timeStamp) {
	    if (open) {
		int length = message.getLength();
		if (length <= 3) {
		    int packedMsg;
		    if (message instanceof ShortMessage) {
			if (message instanceof FastShortMessage) {
			    packedMsg = ((FastShortMessage) message).getPackedMsg();
			} else {
			    ShortMessage msg = (ShortMessage) message;
			    packedMsg = (msg.getStatus() & 0xFF)
				| ((msg.getData1() & 0xFF) << 8)
				| ((msg.getData2() & 0xFF) << 16);
			}
		    } else {
			packedMsg = 0;
			byte[] data = message.getMessage();
			if (length>0) {
			    packedMsg = data[0] & 0xFF;
			    if (length>1) {
				packedMsg |= (data[1] & 0xFF) << 8;
				if (length>2) {
				    packedMsg |= (data[2] & 0xFF) << 16;
				}
			    }
			}
		    }
		    nSendShortMessage(id, packedMsg, timeStamp);
		} else {
		    if (message instanceof FastSysexMessage) {
			nSendLongMessage(id, ((FastSysexMessage) message).getReadOnlyMessage(),
					 length, timeStamp);
		    } else {
			nSendLongMessage(id, message.getMessage(), length, timeStamp);
		    }
		}
	    }
	}

	public void close() {
	    receivers.removeElement(this);
	    open = false;
	}

    } // class MidiOutReceiver


    // NATIVE METHODS

    private native long nOpen(int index) throws MidiUnavailableException;
    private native void nClose(long id);

    private native void nSendShortMessage(long id, int packedMsg, long timeStamp);
    private native void nSendLongMessage(long id, byte[] data, int size, long timeStamp);

} // class MidiOutDevice



