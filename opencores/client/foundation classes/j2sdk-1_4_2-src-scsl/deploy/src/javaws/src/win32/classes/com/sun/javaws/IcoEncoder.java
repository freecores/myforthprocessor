/*
 * @(#)IcoEncoder.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.io.FileInputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.File;
import java.awt.Image;
import java.awt.image.PixelGrabber;
import com.sun.javaws.debug.*;

/* IcoEncoder create a windows icon file (.ico) from a Image */
public class IcoEncoder {

    private OutputStream outputS;
    private static byte ICON_SIZE = 32;
    private static byte NUM_COLORS = (byte)0;    // 0 = 256
    private static int BYTE_SIZE =8;
   
    private Image awtImage;

    public IcoEncoder(OutputStream output, Image awtImage) {
	this.outputS = output;	
	this.awtImage = awtImage;
    }

    private void createBitmap() throws IOException {

	int w= 32; // icon width and height
	int h= 32;
	int k= 0;

	// original image pixels
	int[] pixels = new int [w*h];
	
	// XOR bitmap
	byte[] xorPixels = new byte [w*h*3];
	byte[] final_xorPixels = new byte[w*h*3];
	
	// AND bitmap
	byte[] andPixels = new byte[w*4];
	byte[] final_andPixels = new byte[w*4];

	// Scale the orignal image to icon size (32 by 32) and grab the pixels
	PixelGrabber pg = new PixelGrabber(awtImage.getScaledInstance(32, 32, awtImage.SCALE_DEFAULT), 0, 0, w, h, pixels, 0, w);
 
	try {
	    if (pg.grabPixels()) {
		if (Globals.TraceIcoCreation) {
		    Debug.println("pixels grabbed successfully");
		}
	    }
	    else
		if (Globals.TraceIcoCreation) {
		    Debug.println("cannot grab pixels!");
		}
	} catch (InterruptedException e) {	    
	    e.printStackTrace();
	}

	byte maskByte=0;
	int andIndex=0;
	int bitcount=0;
	int bitprinted=0;

	// Reorganize original image pixels into XOR bitmap and AND bitmap
	for (int j=0; j<w*h; j++) {

	    int alpha = (pixels[j] >> 24) & 0xff;
	    int red = (pixels[j] >> 16) & 0xff;
	    int green = (pixels[j] >> 8) & 0xff;
	    int blue = (pixels[j]) & 0xff;
	   
	    // DEBUG:  printout the orignal alpha bitmap
	    if (Globals.TraceIcoCreation) {
		if (alpha != 0)
		    Debug.print(" 1");
		else
		    Debug.print(" "+alpha);
		bitprinted++;
		if (bitprinted==32) {
		    Debug.println(" ");
		    bitprinted=0;
		}
	    }

	    // AND bitmap (1bit for each pixel)
	    if (alpha == 0) { 
	    	maskByte |= (0x80 >> bitcount);             // 0x80 >> j ? 
	    }    
	    bitcount++;
	    if (bitcount==8) {
	   	andPixels[andIndex++] = maskByte;
		maskByte=0;
	    	bitcount=0;
	    }

	    // XOR bitmap  (windows bitmap uses BGR format)
	    xorPixels[k++] = (byte)blue;
	    xorPixels[k++] = (byte)green;
	    xorPixels[k++] = (byte)red;

	}
    
	// DEBUG:  printout the generated AND bitmap
	if (Globals.TraceIcoCreation) {
	    int four=0;
	    // printout andPixels AND bitmap
	    Debug.println("andPxiels bitmap");
	    for (int ng=0; ng<128; ng++) {
		
		for (byte u=0; u<8; u++) {
		    if ((andPixels[ng] & (0x80 >> u)) !=0)
			Debug.print(" 1");
		    else
			Debug.print(" 0");
		}
		
		four++;
		if (four == 4) {
		    Debug.println(" ");
		four =0;
		}
	    }
	}
	
	int ssi;  // source scan index
	int dsi;  // destination scan index

	// reverse the scanline for proper orientation
 	for (int scan=0; scan<h; scan++) {
	    
	    // XOR bitmap
	    dsi = scan * w * 3;
	    ssi = (h - scan - 1) * w * 3;
	  
	    // XOR bitmap
	    for (int z=0; z<w*3; z++) {
		final_xorPixels[dsi+z] = xorPixels[ssi+z];
	    }

	    // AND bitmap
	    dsi = scan * (w / 8);
	    ssi = (h - scan - 1) * (w / 8);
	  
	    // AND bitmap
	    for (int z=0; z<(w/8); z++) {
		final_andPixels[dsi+z] = andPixels[ssi+z];
	    }
	}


	// write the XOR bitmap
	outputS.write(final_xorPixels);

	// write the AND bitmap
	outputS.write(final_andPixels);

	
    }

    public void encode() {

	// Write ico header
	writeIcoHeader();

	// Write the ICONDIRENTRY
	writeIconDirEntry();

	try {
	    // Write the infoHeader (40 byte header, 24 bpp)
	    writeInfoHeader(40, 24);

	    // XOR and AND bitmap
	    createBitmap();	  

	} catch (Exception e) {
	    e.printStackTrace();
	}


    }

    private void writeInfoHeader(int headerSize, int bitsPerPixel) 
	throws IOException {

	// size of header
	writeDWord(headerSize);
	
	// width
	writeDWord(32);
	
	// height (of both XOR and AND bitmap)
	writeDWord(64);
	
	// number of planes
	writeWord(1);
	
	// Bits Per Pixel
	writeWord(bitsPerPixel);

	// compression
	writeDWord(0);
	
	// imageSize
	writeDWord(0);
	
	// xPelsPerMeter
	writeDWord(0);
	
	// yPelsPerMeter
	writeDWord(0);
	
	// Colors Used
	writeDWord(0);
	
	// Colors Important
	writeDWord(0);
	
    }


    private void writeIconDirEntry() {
	byte output;

	try {
	// width
	output = ICON_SIZE;
	outputS.write(output);

	// height
	outputS.write(output);
	
	// number of colors (256)
	output = NUM_COLORS;
	outputS.write(output);

	// reserved
	output = 0;
	outputS.write(output);

	// planes
	output = 1;
	writeWord(output);

	// bit count (1,4,8)
	output = 24;
	writeWord(output);
	
	// Size in bytes (32*32*3 + 32*4 + 40)
	int length = 3240;	
	writeDWord(length);

	// FileOffset
	int offset = 22;
	writeDWord(offset);

	} catch (IOException ioe) {
	    ioe.printStackTrace();
	}
    }

    private void writeIcoHeader() {
	short output;

	try {
	//write "reserved" Word
	output = 0;
	writeWord(output);

	// write "type" Word
	output = 1;
	writeWord(output);

	// write "number of entries"
	writeWord(output);

	} catch (IOException ioe) {
	    ioe.printStackTrace();
	}
    }

    // Methods for little-endian writing
    public void writeWord(int word) throws IOException {
        outputS.write(word & 0xff);
        outputS.write((word & 0xff00) >> 8);
    }
    
    public void writeDWord(int dword) throws IOException {
        outputS.write(dword & 0xff);
        outputS.write((dword & 0xff00) >> 8);
        outputS.write((dword & 0xff0000) >> 16);
        outputS.write((dword & 0xff000000) >> 24);
    } 

}
