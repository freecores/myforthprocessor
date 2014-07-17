/*
 * @(#)MidiInDevice.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.util.Vector;
import javax.sound.midi.*;


/******************************************************************************************
IMPLEMENTATION TODO:


******************************************************************************************/



/**
 * MidiInDevice class representing functionality of MidiIn devices.
 *
 * @version 1.18, 03/01/23
 * @author David Rivas
 * @author Kara Kytle
 * @author Florian Bomers
 */
class MidiInDevice extends AbstractMidiDevice implements Runnable {

    private Thread midiInThread = null;
    private Vector transmitters = new Vector();

    // CONSTRUCTOR

    MidiInDevice(AbstractMidiDeviceProvider.Info info) {
	super(info);
	if(Printer.trace) Printer.trace("MidiInDevice CONSTRUCTOR");
    }


    // IMPLEMENTATION OF ABSTRACT MIDI DEVICE METHODS

    // $$kk: 06.24.99: i have this both opening and starting the midi in device.
    // may want to separate these??
    synchronized void implOpen() throws MidiUnavailableException {
	if (Printer.trace) Printer.trace("> MidiInDevice: implOpen()");

	// can throw MidiUnavailableException
	id = nOpen( ((MidiInDeviceProvider.MidiInDeviceInfo)getDeviceInfo()).getIndex() );

	if (id == 0) {
	    throw new MidiUnavailableException("Unable to open native device");
	}

	// create / start a thread to get messages
	if (midiInThread == null) {
	    midiInThread = new Thread(this);
	    midiInThread.start();
	}

	nStart(id); // can throw MidiUnavailableException
	if (Printer.trace) Printer.trace("<            implOpen()");
    }


    // $$kk: 06.24.99: i have this both stopping and closing the midi in device.
    // may want to separate these??
    synchronized void implClose() {
	long oldId = id; id = 0;

	// close all transmitters
	for(int i = 0; i < transmitters.size(); i++) {
	    ((Transmitter)transmitters.elementAt(i)).close();
	}
	transmitters.removeAllElements();

	// close the device
	nStop(oldId);
	nClose(oldId);
    }


    // OVERRIDES OF ABSTRACT MIDI DEVICE METHODS


    public int getMaxTransmitters() {
	// $$kk: 09.27.99: need a not-specified value here
	return -1;
    }


    public synchronized Transmitter getTransmitter() throws MidiUnavailableException {
	Transmitter transmitter = new MidiInTransmitter();
	transmitters.addElement(transmitter);
	return transmitter;
    }


    // INNER CLASSES

    class MidiInTransmitter implements Transmitter {

	private Receiver receiver = null;

	public void setReceiver(Receiver receiver) {
	    this.receiver = receiver;
	}

	public Receiver getReceiver() {
	    return receiver;
	}

	public void close() {
	    transmitters.removeElement(this);
	}

    } // class MidiInTransmitter


    // RUNNABLE METHOD

    public void run() {
	// while the device is started, keep trying to get messages.
	// this thread returns from native code whenever stop() or close() is called
	while (id!=0) {
	    // go into native code and retrieve messages
	    nGetMessages(id);
	    if (id!=0) {
		try {
		    Thread.sleep(1);
		} catch (InterruptedException e) {}
	    }
	}
	if(Printer.verbose) Printer.verbose("MidiInDevice Thread exit");
	// let the thread exit
	midiInThread = null;
    }


    // CALLBACKS FROM JAVA

    /**
     * Callback from native code when a short MIDI event is received from hardware.
     * @param packedMsg: status | data1 << 8 | data2 << 8
     * @param timeStamp time-stamp in microseconds
     */
    void callbackShortMessage(int packedMsg, long timeStamp) {
	if (packedMsg == 0 || id == 0) {
	    return;
	}

	/*if(Printer.verbose) {
	  int status = packedMsg & 0xFF;
	  int data1 = (packedMsg & 0xFF00)>>8;
	  int data2 = (packedMsg & 0xFF0000)>>16;
	  Printer.verbose(">> MidiInDevice callbackShortMessage: status: " + status + " data1: " + data1 + " data2: " + data2 + " timeStamp: " + timeStamp);
	  }*/

	try {
	    synchronized(transmitters) {
		for (int i = 0; i < transmitters.size(); i++) {
		    Receiver receiver = ((Transmitter)transmitters.elementAt(i)).getReceiver();
		    if (receiver != null) {
			//$$fb 2002-04-02: ShortMessages are mutable, so
			// an application could change the contents of this object,
			// or try to use the object later. So we can't get around object creation
			receiver.send(new FastShortMessage(packedMsg), timeStamp);
		    }
		}
	    }
	} catch (InvalidMidiDataException e) {
	    // this happens when invalid data comes over the wire. Ignore it.
	    return;
	}
    }

    void callbackLongMessage(byte[] data, long timeStamp) {
	if (id == 0 || data == null) {
	    return;
	}
	try {
	    synchronized(transmitters) {
		for (int i = 0; i < transmitters.size(); i++) {
		    Receiver receiver = ((Transmitter)transmitters.elementAt(i)).getReceiver();
		    if (receiver != null) {
			//$$fb 2002-04-02: SysexMessages are mutable, so
			// an application could change the contents of this object,
			// or try to use the object later. So we can't get around object creation
			// But the array need not be unique for each FastSysexMessage object,
			// because it cannot be modified.
			receiver.send(new FastSysexMessage(data), timeStamp);
		    }
		}
	    }
	} catch (InvalidMidiDataException e) {
	    // this happens when invalid data comes over the wire. Ignore it.
	    return;
	}
    }

    // NATIVE METHODS

    private native long nOpen(int index) throws MidiUnavailableException;
    private native void nClose(long id);

    private native void nStart(long id) throws MidiUnavailableException;
    private native void nStop(long id);

    // go into native code and get messages. May be blocking
    private native void nGetMessages(long id);


}

