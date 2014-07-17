/*
 * @(#)AbstractMidiDevice.java	1.15 03/01/23
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
 * Abstract AbstractMidiDevice class representing functionality shared by 
 * MidiInDevice and MidiOutDevice objects. 
 *
 * @version 1.15, 03/01/23
 * @author David Rivas
 * @author Kara Kytle
 */
abstract class AbstractMidiDevice implements MidiDevice {


    // STATIC VARIABLES

    /**
     * Global event thread
     * $$kk: 06.22.99: we are not using this now....
     * $$kk: 10.13.99: MixerSequencer is using this
     * for meta anc controller events!
     */
    //private static final EventDispatcher eventDispatcher;
    protected static final EventDispatcher eventDispatcher;

    /**
     * list of who has this device open, so we can share it.
     */
    private static Vector openDeviceList = new Vector();


    // INSTANCE VARIABLES
	

    // DEVICE ATTRIBUTES

    protected /*private*/ MidiDevice.Info info;


    // DEVICE STATE

    protected /*private*/ boolean open		= false;
	
    /**
     * This is the device handle returned from native code
     */
    /*
     * $$rratta Solaris 64 bit holds pointer must be long
     */	
    protected long id			= 0;



    static {

	// create and start the global event thread

	// $$kk: 12.21.98: 
	// 1) probably don't want a single global event queue
	// 2) need a way to stop this thread when the engine is done

	eventDispatcher = new EventDispatcher();
	eventDispatcher.start();
    }



    // CONSTRUCTOR
	

    /**
     * Constructs an AbstractMidiDevice with the specified info object.
     * @param info the description of the device
     */
    /*
     * The initial mode and and only supported mode default to OMNI_ON_POLY.
     */
    protected AbstractMidiDevice(MidiDevice.Info info) {

	if(Printer.trace) Printer.trace(">> AbstractMidiDevice CONSTRUCTOR");
		
	this.info = info;

	//this.mode = OMNI_ON_POLY;
	//this.modes = new int[1];
	//modes[0] = OMNI_ON_POLY;

	if(Printer.trace) Printer.trace("<< AbstractMidiDevice CONSTRUCTOR completed");
    }


    // MIDI DEVICE METHODS

    public MidiDevice.Info getDeviceInfo() {		
	return info;
    }


    public void open() throws MidiUnavailableException {

	if (Printer.trace) Printer.trace("> AbstractMidiDevice: open()");

	synchronized(this) {

	    if (open == false) {
			
		implOpen();
		open = true;
	    }
	}

	if (Printer.trace) Printer.trace("< AbstractMidiDevice: open() completed");
    }


    public void close() {

	if (Printer.trace) Printer.trace("> AbstractMidiDevice: close()");

	synchronized(this) {

	    if ( (open == true) && (openDeviceList.isEmpty()) ) {
			
		implClose();
		open = false;
	    }
	}
			
	if (Printer.trace) Printer.trace("< AbstractMidiDevice: close() completed");
    }


    public boolean isOpen() {
	return open;
    }


    /**
     * This implementation always returns -1.
     * Devices that actually provide this should over-ride 
     * this method.
     */
    public long getMicrosecondPosition() {
	return -1;
    }


    /**
     * This implementation always returns 0.
     * Devices that support 1 or more receivers should over-ride 
     * this method.
     */
    public int getMaxReceivers() {
	return 0;		
    }


    /**
     * This implementation always returns 0.
     * Devices that support 1 or more transmitters should over-ride 
     * this method.
     */
    public int getMaxTransmitters() {
	return 0;
    }


    /**
     * This implementation always returns 0.
     * Devices that support 1 or more thru transmitters should over-ride 
     * this method.
     */
    //public int getMaxThruTransmitters() {
    //	return 0;
    //}


    /**
     * This implementation always throws an exception.
     * Devices that support 1 or more receivers should over-ride 
     * this method.
     */
    public Receiver getReceiver() throws MidiUnavailableException {
	throw new MidiUnavailableException("MIDI IN receiver not available");
    }


    /**
     * This implementation always throws an exception.
     * Devices that support 1 or more transmitters should over-ride 
     * this method.
     */
    public Transmitter getTransmitter() throws MidiUnavailableException {
	throw new MidiUnavailableException("MIDI OUT transmitter not available");
    }


    /**
     * This implementation always throws an exception.
     * Devices that support 1 or more thru transmitters should over-ride 
     * this method.
     */
    //public Transmitter getThruTransmitter() throws MidiUnavailableException {
    //	throw new MidiUnavailableException("MIDI THRU transmitter not available");
    //}


    // HELPER METHODS
	
    long getId() {
	return id;
    }


    /**
     * for device sharing.  used within our implementation.
     */
    void open(MidiDevice device) throws MidiUnavailableException {

	synchronized(openDeviceList) {

	    if ( ! (openDeviceList.contains(device)) ) {
		openDeviceList.addElement(device);
	    } 
	
	    open();
	}
    }


    /**
     * for device sharing.  used within our implementation.
     */
    void close(MidiDevice device) {

	synchronized(openDeviceList) {

	    openDeviceList.removeElement(device);
	    close();
	}
    }


    // ABSTRACT METHODS

    abstract void implOpen() throws MidiUnavailableException;
    abstract void implClose();


    // OLD
    //protected /*private*/ int mode;
    //protected /*private*/ int[] modes;
    /**
     * Constructs an AbstractMidiDevice with the specified initial mode and
     * set of supported modes.
     * @param mode the initial mode
     * @param modes the set of supported modes
     * @param info the description of the device
     */
    /*
      protected AbstractMidiDevice(MidiDevice.Info info, int mode, int[] modes) {

      if(Printer.trace) Printer.trace(">> AbstractMidiDevice CONSTRUCTOR");
		
      this.info = info;
      //this.mode = mode;
      //this.modes = modes;

      if(Printer.trace) Printer.trace("<< AbstractMidiDevice CONSTRUCTOR completed");
      }
    */
    /*
      public void setMode(int mode) {

      // $$kk: 04.07.99: we do not support this now.
      }

      public int getMode() {
      return mode;
      }


      public int[] getModes() {

      int[] returnedModes = new int[modes.length];
      System.arraycopy(modes, 0, returnedModes, 0, modes.length);
      return returnedModes;
      }
    */
}

										
