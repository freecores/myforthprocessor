/*
 * @(#)Steamer.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;


import java.awt.Point;
import java.awt.Image;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.lang.InterruptedException;
import java.net.URL;
import java.net.MalformedURLException;
import javax.swing.Timer;
import sun.plugin.AppletViewer;
import java.awt.MediaTracker;
import javax.swing.ImageIcon;

/**
 * This class is to animate the java cup in the gray box.  It uses cup1.gif - cup10.gif
 * to create animated look.
 *
 * @author  mfisher
 * @version 
 */
public class Steamer implements ActionListener 
{ 
    private Timer ticker;
    private static final int NUMBER_OF_IMAGES = 1;
    private static final String img_base = "sun/plugin/util/cup";    
    private static final String broken_image = "sun/plugin/util/graybox_broken.gif"; 
    private static Image[] cups = new Image[NUMBER_OF_IMAGES];
    private static Image broken_graybox_image = null;
    private String custom_image_file = null;
    private Image custom_image;
    private int current_image_number = 0;
    private URL codebase;
    private boolean applet_error = false;
    private int X_coordinate = 6;
    private int Y_coordinate = 6;
    AppletViewer av;
    private static MediaTracker tracker;

    /** Creates new Steamer */
    public Steamer(AppletViewer av) 
    {
	this.av = av;        
    }

    private static boolean init = false;
    private void loadDefaultImages()
    {
	if (init)
	    return;
    
	init = true;

        // Load first image - we need at least one image to display.     
        loadFirstImage();
    
        // Load the rest of the images to use for animation.
        loadOtherImages();
    }

    public void init()
    {
        tracker = new MediaTracker(av);
        
	// Load default images
	loadDefaultImages();

	if (custom_image_file != null)
	{
	    // Load custom image if available
	    loadCustomImage();
	}
	else
	{
	    // Start animation if no custom image is provided
	    startTimer();
	}
    }

    public void stop()
    {
	// Stop animation if necessary
	stopTimer();
        av.repaint();
    }

    public void error()
    {      
        applet_error = true;
        X_coordinate = 6;
        Y_coordinate = 6;
        custom_image = null;
    }

    public void destroy()
    {
	// Stop animation if necessary
	stopTimer();	
    }

    /*
     * Start the timer if it is not running yet.
     */
    public synchronized void startTimer()
    {
    }
    
    /*
     * Stop the timer if it is running.
     */
    public synchronized void stopTimer()
    {
    }

    /*
     * Load first image - we need at least one image to be ready
     * for displaying by the first tick.
     */
    private void loadFirstImage()
    {
	if (cups[0] == null)
	{        
	    Runnable work = new Runnable(){
	            public void run(){            
		cups[0] = Toolkit.getDefaultToolkit().getImage(ClassLoader.getSystemResource(img_base + "1.gif"));           
		tracker.addImage(cups[0], 2);
                
		try{                
		    tracker.waitForID(2);                
		    av.repaint();            
		}
		catch(InterruptedException e) {}
                
	            }            
	        };
                    
	    Thread img_loader = new Thread(work, "Default Image Loader");
	    img_loader.start();            
	}
    }        

    /*
     * Load the rest of the images in a separate thread.
     */
    private synchronized static void loadOtherImages()
    {
        Runnable work = new Runnable()
	{
	    public void run()
	    {
                //load images here.
                for (int i=1; i<NUMBER_OF_IMAGES; i++)
		{
                    String img_name = img_base + (i+1) + ".gif";
		    cups[i] = Toolkit.getDefaultToolkit().getImage(ClassLoader.getSystemResource(img_name));
                }

             }
        };
        
        Thread img_loader = new Thread(work, "Graybox Image Loader");
        img_loader.start();
    }

    private void loadCustomImage()
    {
        Runnable work = new Runnable()
	{
	    public void run()
	    {
		try
		{
		    // try creating custom image.  If user forgot to put their
		    // image to the right place - nothing will be displayed.

		    if (custom_image_file != null)
		    {                
			URL image_url = new URL (codebase, custom_image_file); 
			custom_image = Toolkit.getDefaultToolkit().getImage(image_url); 
                        tracker.addImage(custom_image, 1);
                                    
			X_coordinate = 0;
			Y_coordinate = 0;
                        
                        try{ 
                            tracker.waitForID(1);
                            av.repaint();
                        }
                        catch(InterruptedException e) {}
                        
		    }
		}
		catch (MalformedURLException eh)
		{
		    Trace.msgPrintln("image_failed");
		}
	    }
	};

        Thread img_loader = new Thread(work, "Custom Image Loader");
        img_loader.start();
    }
    
    /*
     * Will return current image to be displayed.  If it is no there yet - not loaded,
     * then display the previous image.  If previous image is not ready, keep
     * looking for the last loaded image.  In the worst case we'll get to the first image.
     */
    public Image getCurrentImage()
    {
        // Check if error occured while loading applet.
        if (applet_error){
            stopTimer();
            broken_graybox_image = new ImageIcon(ClassLoader.getSystemResource(broken_image)).getImage();
            
            return broken_graybox_image;        
        }

        if (custom_image != null)           
            return custom_image;
        
        // Use default image.

        // Start timer if not started yet
        startTimer();

        return getCurrentImage(current_image_number);
    }
    
    /*
     * Search for the last available image
     */    
    private Image getCurrentImage(int num)
    {        
	if (num < 0)
	    return null;

	for (int i=num; i >= 0; i--)
	{
	    if (cups[i] != null)
		return cups[i];
	}

	return null;
    }
    
    
    public Point getImgCoordinates()
    {
        return new Point(X_coordinate, Y_coordinate);
    }
    

    /*
     * Set user-defined image to be used instead of ours.
     */
   public void setCustomImage(URL cb, String str)
   {
	X_coordinate = 0;
	Y_coordinate = 0;
	codebase = cb;
	custom_image_file = str;
    }    

    /**
     * Invoked when an action occurs.
     */
    public void actionPerformed(ActionEvent e) 
    {
        // This is an event from Steamer's timer -
        // repaint.
        current_image_number++;
	current_image_number %= NUMBER_OF_IMAGES;

	av.repaint();
    }  
}
