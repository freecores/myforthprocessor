/*
 * @(#)SunWritableRaster.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.Point;
import java.awt.Rectangle;
import java.awt.image.DataBuffer;
import java.awt.image.Raster;
import java.awt.image.SampleModel;
import java.awt.image.WritableRaster;


/**
 * This class exists as a middle layer between WritableRaster and its
 * implementation specific subclasses (ByteComponentRaster, ShortBandedRaster,
 * etc).  It provides a means for notifying an associated listener that the
 * contents of the raster have been modified.
 */
public class SunWritableRaster extends WritableRaster {

    // REMIND: for now we keep track of only one RasterListener; in the future
    // we may have to keep a Vector of listeners and update each one of them
    // if we support many Images to one Raster
    protected RasterListener listener;


    public SunWritableRaster(SampleModel sampleModel, Point origin) {
        super(sampleModel, origin);
    }

    public SunWritableRaster(SampleModel sampleModel,
                             DataBuffer dataBuffer,
                             Point origin) 
    {
        super(sampleModel, dataBuffer, origin);
    }

    public SunWritableRaster(SampleModel sampleModel,
                             DataBuffer dataBuffer,
                             Rectangle aRegion,
                             Point sampleModelTranslate,
                             WritableRaster parent)
    {
        super(sampleModel, dataBuffer, aRegion, sampleModelTranslate, parent);
    }


    /**
     * Attaches a listener object to this raster
     */
    public void setRasterListener(RasterListener rl) {
        listener = rl;
    }

    /**
     * If a listener exists, sends notification that the raster's contents
     * have been modified
     */
    public void notifyChanged() {
        if (listener != null) {
            listener.rasterChanged();
        }
    }

    /**
     * If a listener exists, sends notification that the raster has been
     * taken in a way that places it out of control of the listner
     */
    public void notifyStolen() {
        if (listener != null) {
            listener.rasterStolen();
        }
    }


    /** 
     * Raster/WritableRaster overrides
     */


    public DataBuffer getDataBuffer() {
        notifyStolen();
        return super.getDataBuffer();
    }
    
    public void setDataElements(int x, int y, Object inData) {
        super.setDataElements(x, y, inData);
        notifyChanged();
    }

    public void setDataElements(int x, int y, Raster inRaster) {
        super.setDataElements(x, y, inRaster);
        notifyChanged();
    }

    public void setDataElements(int x, int y, int w, int h, Object inData) {
        super.setDataElements(x, y, w, h, inData);
        notifyChanged();
    }

    public void setRect(Raster srcRaster) {
        super.setRect(srcRaster);
        notifyChanged();
    }

    public void setRect(int dx, int dy, Raster srcRaster) {
        super.setRect(dx, dy, srcRaster);
        notifyChanged();
    }

    public void setPixel(int x, int y, int iArray[]) {
        super.setPixel(x, y, iArray);
        notifyChanged();
    }

    public void setPixel(int x, int y, float fArray[]) {
        super.setPixel(x, y, fArray);
        notifyChanged();
    }

    public void setPixel(int x, int y, double dArray[]) {
        super.setPixel(x, y, dArray);
        notifyChanged();
    }

    public void setPixels(int x, int y, int w, int h, int iArray[]) {
        super.setPixels(x, y, w, h, iArray);
        notifyChanged();
    }

    public void setPixels(int x, int y, int w, int h, float fArray[]) {
        super.setPixels(x, y, w, h, fArray);
        notifyChanged();
    }

    public void setPixels(int x, int y, int w, int h, double dArray[]) {
        super.setPixels(x, y, w, h, dArray);
        notifyChanged();
    }

    public void setSample(int x, int y, int b, int s) {
        super.setSample(x, y, b, s);
        notifyChanged();
    }

    public void setSample(int x, int y, int b, float s){
        super.setSample(x, y, b, s);
        notifyChanged();
    }

    public void setSample(int x, int y, int b, double s){
        super.setSample(x, y, b, s);
        notifyChanged();
    }

    public void setSamples(int x, int y, int w, int h, int b, 
                           int iArray[]) 
    {
        super.setSamples(x, y, w, h, b, iArray);
        notifyChanged();
    }

    public void setSamples(int x, int y, int w, int h, int b,
                           float fArray[])
    {
        super.setSamples(x, y, w, h, b, fArray);
        notifyChanged();
    }

    public void setSamples(int x, int y, int w, int h, int b,
                           double dArray[])
    {
        super.setSamples(x, y, w, h, b, dArray);
        notifyChanged();
    }

}
