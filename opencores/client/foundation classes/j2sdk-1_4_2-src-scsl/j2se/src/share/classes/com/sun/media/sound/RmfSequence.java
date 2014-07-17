/*
 * @(#)RmfSequence.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;		  	 

import javax.sound.midi.Sequence;
import javax.sound.midi.InvalidMidiDataException;


/**
 * Represents an RMF sequence.
 *
 * @version 1.11 03/01/23
 * @author Kara Kytle
 */
public class RmfSequence extends Sequence {

    /**
     * Constructs a new RmfSequence;
     *
     * @param divisionType the timing division type.
     * @param resolution the timing resolution.
     * @throws InvalidMidiDataException if the divisionType is not valid.
     */
    RmfSequence(float divisionType, int resolution) throws InvalidMidiDataException {

	super(divisionType, resolution);
    }


    /**
     * Obtains the song title.
     * @return title
     */
    public String getTitle() {
		
	return null;
    }


    /**
     * Sets the song title.
     * @param song title
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setTitle(String title) {
		
    }


    /**
     * Obtains the song performer.
     * @return performer
     */
    public String getPerformer() {
		
	return null;
    }


    /**
     * Sets the song performer.
     * @param performer performer
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setPerformer(String performer) {
		
    }


    /**
     * Obtains the song composer.
     * @return composer
     */
    public String getComposer() {
		
	return null;
    }


    /**
     * Sets the song composer.
     * @param composer composer
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setComposer(String composer) {
		
    }


    /**
     * Obtains the song composer notes.
     * @return composer notes
     */
    public String getComposerNotes() {
		
	return null;
    }


    /**
     * Sets the song composer notes.
     * @param notes composer notes
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setComposerNotes(String notes) {
		
    }


    /**
     * Obtains the song copyright date.
     * @return copyright date
     */
    public String getCopyright() {
		
	return null;
    }


    /**
     * Sets the song copyright.
     * @param copyright copyright
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setCopyright(String copyright) {
		
    }


    /**
     * Obtains the song publisher contact info.
     * @return publisher contact info
     */
    public String getPublisher() {
		
	return null;
    }


    /**
     * Sets the song publisher.
     * @param publisher publisher
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setPublisher(String publisher) {
		
    }


    /**
     * Obtains the song use of license.
     * @return use of license
     */
    public String getUseOfLicense() {
		
	return null;
    }


    /**
     * Sets the song use of license.
     * @param useOfLicense use of license
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setUseOfLicense(String useOfLicense) {
		
    }


    /**
     * Obtains the song licensed URL.
     * @return licensed URL
     */
    public String getLicensedUrl() {
		
	return null;
    }


    /**
     * Sets the song licensed URL.
     * @param licensedURL licensed URL
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setLicensedURL(String licensedURL) {
		
    }


    /**
     * Obtains the song license term.
     * @return license term
     */
    public String getLicenseTerm() {
		
	return null;
    }


    /**
     * Sets the song license term.
     * @param term license term
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setLicenseTerm(String term) {
		
    }


    /**
     * Obtains the song expiration date.
     * @return expiration date
     */
    public String getExpirationDate() {
		
	return null;
    }


    /**
     * Sets the song expiration date.
     * @param date expiration date
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setExpirationDate(String date) {
		
    }


    /**
     * Obtains the song index number.
     * @return index number
     */
    public String getIndexNumber() {
		
	return null;
    }


    /**
     * Sets the song index number.
     * @param index index number
     * @throws IllegalStateException if the sequence is not editable.
     */
    public void setIndexNumber(String index) {
		
    }


    /**
     * Sets the password for the sequence.  If set, the password must be used
     * to gain access to private data in the sequence.
     * @throws SecurityException thrown if the caller does not have the right 
     * to set the password.
     */
    public void setPassword(String password) {

    }


    /**
     * Determines whether the sequence is currently editable.
     * @return true if the sequence is currently editable, otherwise false.
     */
    public boolean isEditable() {

	return false;
    }


    /**
     * Enables editing for the sequence.
     * @param password password for the sequence.  If no password has been
     * set, may be null.
     * @throws SecurityException if the sequence cannot be made editable.
     * This may occur if the wrong password is supplied, or potentially 
     * if the file has been marked permanently uneditable, etc.
     */
    public void enableEdit(String password) {

    }


    /**
     * Disables editing for the sequence.
     */
    public void disableEdit() {

    }
}
