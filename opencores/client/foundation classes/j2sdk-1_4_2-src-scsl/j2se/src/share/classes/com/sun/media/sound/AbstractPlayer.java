/*
 * @(#)AbstractPlayer.java	1.41 03/01/23
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
import javax.sound.sampled.LineUnavailableException;
			 


/******************************************************************************************
IMPLEMENTATION TODO:


******************************************************************************************/



/**
 * Abstract AbstractPlayer class representing functionality shared by the
 * Sequencer and Synthesizer components of the Headspace Mixer.  This
 * class is extended by the concrete classes MixerSynth and MixerSequencer.
 *
 * @version 1.41, 03/01/23
 * @author David Rivas
 * @author Kara Kytle
 */
abstract class AbstractPlayer extends AbstractMidiDevice implements Synthesizer {


    // STATIC VARIABLES

    // soundbank names in descending quality order.  
    // i am leaving "soundbank.gm" in for historical reasons:
    // in JDK 1.2 releases, this is identical to "soundbank-mid.gm."
    // in early JDK 1.3 releases, this is identical to "soundbank-min.gm."
    private static final String[] defaultSoundbankNames = { "soundbank-deluxe.gm", "soundbank-mid.gm", "soundbank.gm", "soundbank-min.gm" };	

    // $$jb: 04.21.99: is this getable somewhere?
    // $$kk: 04.222.99: put this variable in Platform.java with the other
    // magic strings.
    private static final String soundJarName = "sound.jar";
    private static final String jmfJarName = "jmf.jar";

    // the set of supported midi out classes
    private static final Class[] supportedMidiOutClasses = { SimpleOutputDevice.class, MidiOutDevice.class };						

    static HeadspaceMixer mixer = HeadspaceMixer.getMixerInstance();

    /**
     * Default soundbank for this player.  Might as well be static
     * right now, but theoretically could differ between instances.
     */
    // $$jb: 07.08.99: making this static so there's only one instance
    // of the soundbank for all sequences.
    static private Soundbank defaultSoundbank = null;


    // INSTANCE VARIABLES

    // set of supported classes for midi out, in order
    private Class[] orderedMidiOutClasses				= { SimpleOutputDevice.class, MidiOutDevice.class };

    /**
     * The internal synthesis unit.  I think of it like the built-in sound
     * generating circuitry; it doesn't go on the list of receivers.
     */
    AbstractMidiDevice internalSynth = null;


    /**
     * Set of channels for the synthesizer.
     */	 
    protected MixerMidiChannel[] channels;

    /**
     * List of loaded instruments
     */
    private Vector instruments = new Vector();



    private Vector transmitters		= new Vector();
    private Vector receivers		= new Vector();

	
    // CONSTRUCTOR
	
    protected AbstractPlayer(MidiDevice.Info info) {

	super(info);

	if(Printer.trace) Printer.trace(">> AbstractPlayer CONSTRUCTOR: " + this);

	// create and initialize the MidiChannel objects
	channels = new MixerMidiChannel[16];			
	for (int i = 0; i < channels.length; i++) {
	    channels[i] = new MixerMidiChannel(this, i);
	}

	if(Printer.trace) Printer.trace("<< AbstractPlayer CONSTRUCTOR completed: " + this);
    }


    // OVERRIDES OF ABSTRACT MIDI DEVICE METHODS


    /**
     * this method opens the native resources *and* sets the state to open.
     * implOpen() does everything but change the state.  we need this because
     * MixerSequencer.setSequence() actually needs to close and reopen the 
     * native resources without changing the Sequencer open/close state.
     */
    public synchronized void open() throws MidiUnavailableException {

	if(Printer.trace) Printer.trace(">> AbstractPlayer open(): " + this);

	if (isOpen()) {
	    if(Printer.trace) Printer.trace("<< AbstractPlayer open(): already open: " + this);
	    return;
	}

	try {
	    mixer.open(this);
	} catch (LineUnavailableException lue) {
	    throw new MidiUnavailableException(lue.getMessage());
	}
	implOpen();	// can throw MidiUnavailableException
	open = true;
	if(Printer.trace) Printer.trace("<< AbstractPlayer open(): open completed: " + this);
    }


    /**
     * this method closes the native resources *and* sets the state to closed.
     * implClose() does everything but change the state.  we need this because
     * MixerSequencer.setSequence() actually needs to close and reopen the 
     * native resources without changing the Sequencer open/close state.
     */
    public synchronized void close() {

	if(Printer.trace) Printer.trace(">> AbstractPlayer close(): " + this);

	if (isOpen()) {

	    // close all transmitters
	    for(int i = 0; i < transmitters.size(); i++) {
		((Transmitter)transmitters.elementAt(i)).close();
	    }
	    transmitters.removeAllElements();

	    // close all receivers
	    for(int i = 0; i < receivers.size(); i++) {
		((Receiver)receivers.elementAt(i)).close();
	    }
	    receivers.removeAllElements();

	    // close the player
	    open = false;
	    implClose();
	    mixer.close(this);
	}

	if(Printer.trace) Printer.trace("<< AbstractPlayer close() completed: " + this);
    }


    // OVERRIDES OF ABSTRACT MIDI DEVICE METHODS

					
    public int getMaxTransmitters() {

	// $$kk: 09.27.99: need a not-specified value here
	return -1;		
    }


    public synchronized Transmitter getTransmitter() throws MidiUnavailableException {
		
	Transmitter transmitter = new PlayerOutTransmitter();
	transmitters.addElement(transmitter);
	return transmitter;
    }


    public int getMaxReceivers() {

	// $$kk: 09.27.99: need a not-specified value here
	return -1;		
    }


    public synchronized Receiver getReceiver() throws MidiUnavailableException {
		
	Receiver receiver = new PlayerReceiver();
	receivers.addElement(receiver);
	return receiver;
    }


    // INNER CLASSES

    class PlayerOutTransmitter implements Transmitter {


	private boolean open = true;
	private Receiver receiver = null;


	public void setReceiver(Receiver receiver) {

	    // $$kk: 09.27,99: if there is a current receiver, need to 
	    // remove it using nRemoveReceiver

	    // $$kk: 11.03.99: TODO

	    this.receiver = receiver;

	    // $$kk: 09.27,99: need to add receiver using nAddReceiver

	    // $$kk: 11.03.99: TODO
	}


	public Receiver getReceiver() {
	    return receiver;
	}
			

	public void close() {
	    transmitters.removeElement(this);
	    open = false;
	}
    } // class PlayerOutTransmitter


    class PlayerReceiver implements Receiver {


	private boolean open = true;			 


	public void send(MidiMessage message, long timeStamp) {

	    // if the receiver is not open, return
	    if (!open) {
		return;
	    }

	    // if the device is not open, throw an exception
	    if (id == 0) {
		throw new IllegalStateException("Synthesizer is not open.");
	    }

	    int i;
	    Receiver receiver;
			
	    // send the received message to each of the transmitters
	    synchronized(transmitters) {
		for (i = 0; i < transmitters.size(); i++) {
		    receiver = ((Transmitter)transmitters.elementAt(i)).getReceiver();
		    if (receiver != null) {
			receiver.send(message, timeStamp);
		    }	
		}
	    }
			
	    // send the received message to the internal synth
	    if ( ! (message instanceof ShortMessage) ) {
		if(Printer.err) Printer.err("Unsupported message type: " + message);
		return;
	    }
	    ShortMessage shortMessage = (ShortMessage)message;
	    parse(shortMessage.getCommand(), shortMessage.getChannel(), shortMessage.getData1(), shortMessage.getData2(), timeStamp);
	}


	public void close() {
	    receivers.removeElement(this);
	    open = false;
	}
    } // class MidiOutReceiver


    // SYNTHESIZER METHODS


    public int getMaxPolyphony() {						
	return (HeadspaceMixer.getMixerInstance().getMidiVoices());
    }


    public long getLatency() {
	return 0;
    }


    public MidiChannel[] getChannels() {

	MidiChannel[] returnedChannels = new MidiChannel[channels.length];
	System.arraycopy(channels, 0, returnedChannels, 0, channels.length);
	return returnedChannels;
    }


    public VoiceStatus[] getVoiceStatus() {
	return new VoiceStatus[0];
    }


    public boolean isSoundbankSupported(Soundbank soundbank) {
	return (soundbank instanceof HeadspaceSoundbank) ? true : false; 

	// $$kk: 11.03.99: TODO: native midi case
    }


    public boolean loadInstrument(Instrument instrument) {

	if (instruments.contains(instrument)) {
	    return true;
	}

	try {
	    if (nLoadInstrument(id, ((HeadspaceInstrument)instrument).getId())) {
		instruments.addElement(instrument);
		return true;
	    } else {
		return false;
	    }
	} catch (ClassCastException e) {			
	    throw new IllegalArgumentException("Unsupported soundbank: " + instrument.getSoundbank());
	}

	// $$kk: 11.03.99: TODO: native midi case
    }


    public void unloadInstrument(Instrument instrument) {

	try {
	    if (nUnloadInstrument(id, ((HeadspaceInstrument)instrument).getId())) {
		instruments.removeElement(instrument);
	    }
	} catch (ClassCastException e) {			
	    throw new IllegalArgumentException("Unsupported soundbank: " + instrument.getSoundbank());
	}
    }


    public boolean remapInstrument(Instrument from, Instrument to) {

	try {
	    return nRemapInstrument(id, ((HeadspaceInstrument)from).getId(), ((HeadspaceInstrument)to).getId());
	} catch (ClassCastException e) {			
	    throw new IllegalArgumentException("Unsupported soundbank: " + from.getSoundbank() + " or " + to.getSoundbank());
	}
    }

	
    public Soundbank getDefaultSoundbank() {
	return defaultSoundbank;
    }

	
    public Instrument[] getAvailableInstruments() {
		
	if (defaultSoundbank != null) {
	    return defaultSoundbank.getInstruments();
	} else {
	    return new Instrument[0];
	}
    }


    public Instrument[] getLoadedInstruments() {
		
	Instrument[] returnedArray;

	synchronized(instruments) {
	    returnedArray = new Instrument[instruments.size()];
	    for (int i = 0; i < returnedArray.length; i++) {
		returnedArray[i] = (Instrument)instruments.elementAt(i);
	    }
	}

	return returnedArray;
    }


    public boolean loadAllInstruments(Soundbank soundbank) {

	boolean allOk = true;
	Instrument[] instrumentArray = soundbank.getInstruments();

	for (int i = 0; i < instrumentArray.length; i++) {
				
	    // may throw IllegalArgumentException
	    if (!loadInstrument(instrumentArray[i])) {
		allOk = false;
	    }
	}

	return allOk;
    }


    public void unloadAllInstruments(Soundbank soundbank) {

	Instrument[] instrumentArray = soundbank.getInstruments();

	for (int i = 0; i < instrumentArray.length; i++) {				
	    // may throw IllegalArgumentException
	    unloadInstrument(instrumentArray[i]);
	}
    }


    public boolean loadInstruments(Soundbank soundbank, Patch[] patchList) {

	boolean allOk = true;
		
	for (int i = 0; i < patchList.length; i++) {

	    Instrument instrument = soundbank.getInstrument(patchList[i]);

	    if (instrument != null) {
				// may throw IllegalArgumentException
		if (!loadInstrument(instrument)) {
		    allOk = false;
		}
	    } else {
		allOk = false;
	    }
	}

	return allOk;
    }


    public void unloadInstruments(Soundbank soundbank, Patch[] patchList) {

	for (int i = 0; i < patchList.length; i++) {

	    Instrument instrument = soundbank.getInstrument(patchList[i]);

	    if (instrument != null) {
				// may throw IllegalArgumentException
		unloadInstrument(instrument);
	    }
	}
    }



    // PUBLIC CONFIGURATION METHODS	


    /**
     * Sets the ordered list of targets for MIDI output by class.
     */
    public void setMidiOutClasses(Class[] classes) {
		
	int i, j;
	boolean ok = false;

	// make sure these classes are supported!
	for (i = 0; i < classes.length; i++) {

	    ok = false;

	    for (j = 0; j < supportedMidiOutClasses.length; j++) {
		if (supportedMidiOutClasses[j].isAssignableFrom(classes[i])) {
		    ok = true;
		    break;
		}
	    }

	    if (!ok) {
		throw new IllegalArgumentException("Unsupported class for midi out services: " + classes[i]);
	    }
	}

	// set the new order
	orderedMidiOutClasses = classes;
    }


    /**
     * Obtains the ordered list of targets for MIDI output by class.
     */
    public Class[] getMidiOutClasses() {
	return orderedMidiOutClasses;
    }	 



    // HELPER METHODS

    /**
     * Return the programmed time-stamp in microseconds.  
     * MixerSynth always returns -1.
     */
    abstract long getTimeStamp();


    /**
     * Set the programmed time-stamp in microseconds.  
     * MixerSynth ignores this.
     */
    abstract void setTimeStamp(long timeStamp);

	
    /**
     * true if we're using the software synth, 
     * false if we're using platform midi services.
     */
    boolean isUsingSoftwareSynth() {		
	return (internalSynth == this) ? true : false;
    }


    /**
     * This method gets the default synth for the player.
     * this operates as the built-in sound generating unit
     * for the synth/sequencer. 
     * 
     * Right now, i'm not listing this receiver / synth in 
     * the list of receivers 'cause i'm viewing it as built
     * in.  
     * $$kk: 07.12.99: reconsider this later.
     *  
     * $$kk: 07.12.99: Right now, i'm setting this up to 
     * use hardware midi if it's available (i'm looking for
     * a MidiInDevice), otherwise the software synth.  this
     * way we test the hardware midi!  we might want to change
     * this to use the software synth if it can load a soundbank,
     * and only going to hardware if we can't.  this provides
     * better backwards-compatibility, a more ubiquitous experience,
     * better rmf support, etc.
     *
     * $$kk: 07.16.99: i'm changing this to use the software synth
     * if we did find a soundbank.
     */
    protected void openInternalSynth() throws MidiUnavailableException {

	if (Printer.trace) Printer.trace(">> AbstractPlayer: openInternalSynth: " + this);

	// if the internalSynth is already set, just return
	if (internalSynth != null) {
	    if (Printer.trace) Printer.trace("<< AbstractPlayer: openInternalSynth: internalSynth already set: " + internalSynth);
	    return;
	}


	// iterate through the list of supported classes in
	// order, and try to find something we can use
	for (int i = 0; i < orderedMidiOutClasses.length; i++) {

	    if (Printer.debug) Printer.debug("AbstractPlayer: openInternalSynth: checking orderedMidiOutClasses[" + i + "] : " + orderedMidiOutClasses[i]);

	    // check for MidiOutDevice support / availability
	    if (MidiOutDevice.class.isAssignableFrom(orderedMidiOutClasses[i])) {

		if (Printer.debug) Printer.debug("AbstractPlayer: openInternalSynth: trying midi out");
				
				// assign the internal synth value for this abstract player
		internalSynth = openMidiOut();
				
				// if it fails, internalSynth is null
		if (internalSynth != null) {
		    if (Printer.debug) Printer.debug("AbstractPlayer: openInternalSynth: got it! internalSynth: " + internalSynth);
		    return;
		}
				
	    } else if (SimpleOutputDevice.class.isAssignableFrom(orderedMidiOutClasses[i])) {								
				
		if (Printer.debug) Printer.debug("AbstractPlayer: openInternalSynth: trying software midi");

				// try to load the default soundbank
		if (defaultSoundbank == null) {
		    defaultSoundbank = loadDefaultSoundbank();
		}

				// if we found the default soundbank, use the software synth
		if (defaultSoundbank != null) {
		    internalSynth = openSoftwareSynth();
		    if (Printer.debug) Printer.debug("AbstractPlayer: openInternalSynth: got it! internalSynth: " + internalSynth);
		    return;
		}
	    }
	}
	
	// if we have worked through all our options and the internal synth is still null,
	// use the software synth.  but if we can't load the soundbank, we won't get any 
	// sound!

	// use the software synth
	internalSynth = openSoftwareSynth();

	// try again to load the default soundbank
	if (defaultSoundbank == null) {
	    defaultSoundbank = loadDefaultSoundbank();
	    if (defaultSoundbank == null) {
		if(Printer.err) Printer.err("\nWARNING: The default soundbank is not loaded." + 
					    "\nAudio synthesis may be unavailable until instruments are loaded.\n");
	    }
	}
		
	//throw new MidiUnavailableException("Unable to open an internal synthesis unit");
	if (Printer.trace) Printer.trace("<< AbstractPlayer: openInternalSynth: internalSynth" + internalSynth);
    }



    /**
     * remove and close the default synth
     */
    protected void closeInternalSynth() {

	if (Printer.trace) Printer.trace(">> AbstractPlayer: closeInternalSynth");

	if (internalSynth != this) {

	    nRemoveReceiver(id, internalSynth.getId());
			
	    // use our hidden device sharing mechanism....
	    internalSynth.close(this);

	} else {
	    nRemoveReceiver(id, 0);
	}

	internalSynth = null;
	if (Printer.trace) Printer.trace("<< AbstractPlayer: closeInternalSynth completed");
    }


    /**
     * add the software synth (always succeeds)
     */
    protected AbstractMidiDevice openSoftwareSynth() {
	if (Printer.trace) Printer.trace(">> AbstractPlayer: openSoftwareSynth");
	if (Printer.trace) Printer.trace("<< AbstractPlayer: openSoftwareSynth returning: " + this);
	return this;
    }


    /**
     * add a platform synth (may fail)
     */
    protected AbstractMidiDevice openMidiOut() {

	if (Printer.trace) Printer.trace(">> AbstractPlayer: openMidiOut");

	MidiDevice.Info[] infoArray = MidiSystem.getMidiDeviceInfo();
	AbstractMidiDevice midiDevice;

	// iterate through the list of midi devices
	for (int j = 0; j < infoArray.length; j++) {

	    // find a MidiOutDevice
	    if (infoArray[j] instanceof MidiOutDeviceProvider.MidiOutDeviceInfo) {

				// try to open it and add it as an external synth
		try {
					
		    // get and open the device								
		    midiDevice = (AbstractMidiDevice)MidiSystem.getMidiDevice(infoArray[j]);
		    midiDevice.open();

		    // got it; we're done!
		    if (Printer.trace) Printer.trace("<< AbstractPlayer: openMidiOut returning: " + midiDevice);
		    return midiDevice;

		} catch (MidiUnavailableException e) {
		    // keep going...
		    if (Printer.debug) Printer.debug("<< AbstractPlayer: openMidiOut: failed to open midi out device: " + e);
		}						
	    }
	}

	// nope!
	if (Printer.trace) Printer.trace("<< AbstractPlayer: openMidiOut returning null");
	return null;
    }


    /** 
     * Set the internal synth as a receiver.  Note
     * that the order of events matters here:  with
     * ths sofware synth, you need the soundbank loaded
     * before the sequence gets set.  But you need to
     * connect to the internal synth *after* setting the
     * sequence because the id value gets set there!!
     */
    protected void connectToInternalSynth() {

	if (Printer.trace) Printer.trace(">> AbstractPlayer: connectToInternalSynth");

	if (internalSynth == null) {
	    if(Printer.err) Printer.err("AbstractPlayer.connectToInternalSynth: internalSynth is null!");
	    return;
	}

	if (internalSynth == this) {			
	    // add the software synth
	    synchronized(this) { nAddReceiver(id, 0); }
		
	} else {
	    // add the external synth
	    synchronized(this) { nAddReceiver(id, internalSynth.getId()); }
	}

	if (Printer.trace) Printer.trace("<< AbstractPlayer: connectToInternalSynth completed");
    }


    /**
     * this method closes the native resources but does not change the state.
     *
     * we need this because MixerSequencer.setSequence() actually needs to 
     * close and reopen the native resources without changing the Sequencer 
     * open/close state.
     */
    synchronized void implClose() {

	if (Printer.trace) Printer.trace(">> AbstractPlayer: implClose");

	// remove and close the default synth
	closeInternalSynth();
	
	long oldId = id;
	id = 0;

	// update the channels with the new id
	for (int i = 0; i < channels.length; i++) {
	    channels[i].setId(id);
	}

	// $$kk: 04.14.99: concrete subclasses should call this
	// method and then do their own cleanup.
	// $$kk: 04.14.99: wait, we actually use the same method
	// for closing sequencers and synths, so might as well
	// put it here.
	nClose(oldId);
	if (Printer.trace) Printer.trace("<< AbstractPlayer: implClose completed");
    }



    // returns true if successfully parsed, otherwise false
    protected boolean parse(int command, int channel, int data1, int data2, long timeStamp) {
    	if (((command & 0x0F) != 0) || (command < 0x80) || (command > 0xE0)) {
    	    // invalid status
    	    return false;
    	}
	setTimeStamp(timeStamp);
    	switch (command) {
	case 0x80:	// Note off
	    channels[channel].noteOff(data1, data2);
	    break;
	case 0x90: 	// Note on
	    channels[channel].noteOn(data1, data2);
	    break;
	case 0xA0:	// key pressure (aftertouch)
	    channels[channel].setPolyPressure(data1, data2);
	    break;
	case 0xB0:	// controllers
	    // $$kk: 11.06.98: what about 14-bit controllers?? 
	    channels[channel].controlChange(data1, data2);
	    break;
	case 0xC0:	// Program change
	    //$$fb 2002-10-30: fix for 4425843: MIDI Program Change messages sent to the synthesizer are ignored
	    channels[channel].programChange(data1);
	    break;
	case 0xD0:	// channel pressure (aftertouch)
	    channels[channel].setChannelPressure(data1);
	    break;
	case 0xE0:	// SetPitchBend
	    // $$fb what's that ?
	    //channels[channel].setPitchBend((data1 & 0xF0) | (data2 & 0x0F));
	    // $$fb better like this:
	    channels[channel].setPitchBend(((data2 & 0x7F) << 7) | (data1 & 0x7F));
	    break;
	}
	return true;
    }


    /**
     * This method currently looks in the current working directory and then
     * relative to javahome for each element in the hard-coded set of 
     * default soundbank names.  This allows us, for instance, to ship 
     * different soundbanks on different platforms, and look for them in
     * quality order.
     * 
     * $$kk: 04.16.99: we may want to add support for a special property name
     * and / or searching the classpath....
     */
    private Soundbank loadDefaultSoundbank() {

	String soundbankName;
	URL url;
	String path;
	String classpath;
	String classpathElement;
	String issoundjar;

	Soundbank bank = null;

	for (int i = 0; i < defaultSoundbankNames.length; i++) {

	    soundbankName = defaultSoundbankNames[i];

	    // look locally
	    try {

		path = "file:" + soundbankName;
		url = new URL(path);

		if(Printer.debug) Printer.debug("Looking for soundbank: " + url);

		bank = new HeadspaceSoundbank(url);

		if (bank != null) {

		    break;
		}

	    } catch (MalformedURLException e) {
	    } catch (IOException e2) {
	    } catch (IllegalArgumentException e3) {
	    }			

	    // look where sound.jar is
	    classpath = Platform.getClasspath();
	    StringTokenizer st = new StringTokenizer(classpath, File.pathSeparator);

	    while (st.hasMoreTokens() && (bank==null) ) {
			
				// this is the next element in the classpath
		classpathElement = st.nextToken();
				// find sound.jar in classpath, replace with defaultSoundbankNames
		issoundjar = classpathElement.substring(classpathElement.lastIndexOf(File.separatorChar)+1,classpathElement.length());
		if( issoundjar.equals(soundJarName) || issoundjar.equals(jmfJarName) ){
		    path = classpathElement.substring(0, classpathElement.lastIndexOf(File.separatorChar));
		    path += File.separatorChar + soundbankName;
		    if(Printer.debug) Printer.debug("Looking for soundbank: " + path);
		    try {
			bank = new HeadspaceSoundbank(path);
		    } catch (IllegalArgumentException e) {
		    }
		}
	    }

	    if( bank != null ) break;

	    // look relative to java.home.
	    path = Platform.getJavahome();
	    path += File.separatorChar + "lib" + File.separatorChar + "audio" + File.separatorChar + soundbankName;

	    try  {

		if(Printer.debug) Printer.debug("Looking for soundbank: " + path);

		bank = new HeadspaceSoundbank(path);

		if (bank != null) {

		    break;
		}

	    } catch (IllegalArgumentException e2) {
	    }
	}

	if(Printer.debug) Printer.debug("Default soundbank: " + bank);
	return bank;
    }



    /*
      // OLD STUFF

      protected float setChannelLinearGain(long id, float linearGain) {
      return nSetChannelLinearGain(id, linearGain);
      }


      protected float setChannelPan(long id, float pan) {
      return nSetChannelPan(id, pan);
      }
    */	

    // NATIVE METHODS

    // SYNTHESIZER / SEQUENCER MANAGEMENT

    // GM_FreeSong
    protected native void nClose(long id);


    // MIDI OUT / SYNTH MANAGEMENT

    // GM_AddSongSynth
    protected native void nAddReceiver(long id, long receiverId);

    // GM_RemoveSongSynth
    protected native void nRemoveReceiver(long id, long receiverId);


    // INSTRUMENT MANAGEMENT
	
    // GM_LoadInstrument
    private native boolean nLoadInstrument(long id, int instrumentId);

    // GM_UnloadInstrument
    private native boolean nUnloadInstrument(long id, int patchIdentifier);

    // GM_RemapInstrument
    private native boolean nRemapInstrument(long id, int from, int to);


    /*
      // OLD STUFF

      // GM_SetSongVolume
      protected native float nSetChannelLinearGain(long id, float linearGain);

      // GM_SetSongStereoPosition
      protected native float nSetChannelPan(long id, float pan);
    */
}
