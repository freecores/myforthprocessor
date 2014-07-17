/*
 * @(#)ActivatorAppletAudioClipRef.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.applet;


/*
 * Helper class that make the AudioClip useable in 1.2 
 *
 * @version 	1.1
 * @author	Stanley Man-Kit Ho
 */
import java.applet.AudioClip;
import java.net.URL;
import java.io.*;
import java.security.*;
import sun.plugin.security.*;


public class ActivatorAppletAudioClipRef extends sun.misc.Ref 
{
    URL url;
    boolean doDownload = true;

    /**
     * Create the Ref
     */
    public ActivatorAppletAudioClipRef(URL url) {
	this.url = url;
    }

    public void flush() 
    {
	super.flush();
    }

    /**
     * Reconsitute the image.  Only called when the ref has been flushed.
     */
    public Object reconstitute() 
    {
	if ( doDownload )
	{
	    AudioClip clip = (AudioClip) AccessController.doPrivileged(
		new PrivilegedAction() 
		{
		    public Object run() 
		    {
			// The following code is to resolve JDK 1.1
			// compatibility with Internet Explorer.
			// 
			// In IE, resources are always searched in 
			// the archives first before searching the
			// codebase, but this is not the case in
			// NS or appletviewer. 
			//
			// To enhance compatibility, Java Plug-in
			// will search the archives first before
			// the codebase.
			//
			// The following step has been taken:
			//
			// We check if the url is from the base URL
			// of the classloader. If so, we try to 
			// load the resources using getResourcesAsStream
			// with the following sequences:
			//
			// 1. Load resources from archives
			// 2. Load resources from codebase
			//
			// [stanleyh]
			try
			{
			    Thread t = Thread.currentThread();
			    ClassLoader cl = t.getContextClassLoader();

			    if (cl != null && cl instanceof PluginClassLoader)
			    {
				PluginClassLoader pcl = (PluginClassLoader) cl;

				// Determine classloader URL
				String clsLoaderURLString = pcl.getBaseURL().toString();

				// Separate path and resource name - resource name 
				// may contain directory structure
				//
				String audioURLString = url.toString();
				int i = audioURLString.indexOf(clsLoaderURLString);

				if (i == 0) 
				{
				    String clipName;

				    // This is to fix problem like 
				    //
				    // http://fdl.msn.com/zone/games/SAC/DIMI//sounds/click2.au
				    //
				    // so the extra '/' is eliminated
				    //
				    if (audioURLString.charAt(clsLoaderURLString.length()) == '/')
					clipName = audioURLString.substring(clsLoaderURLString.length() + 1);
				    else
					clipName = audioURLString.substring(clsLoaderURLString.length());

				    // Get resources from classloader as stream
				    InputStream is = pcl.getResourceAsStream(clipName);

				    // If stream exists
				    if (is != null)
				    {
					BufferedInputStream bis = new BufferedInputStream(is);
					ByteArrayOutputStream bos = new ByteArrayOutputStream();
					byte[] buffer = new byte[8192];

					int byteRead = 0;

					// Read the stream until it is EOF
					while ((byteRead = bis.read(buffer, 0, 8192)) != -1)
					    bos.write(buffer, 0, byteRead);
		
					// Close input stream
					bis.close();

					// Convert to byte array
					byte[] data = bos.toByteArray();

    					// Return audio clip only if data length is not zero
					if (data != null && data.length > 0)  
					    return new AppletAudioClip(data);
				    }
				}
			    }
			
			    // The base URL doesn't match or no
			    // context classloader, so load
			    // the resources from URL directly.
			    //
			    return new AppletAudioClip(url);
			}
			catch (Exception e)
			{
			    e.printStackTrace();

			    // If we have any exception or unable to load the 
			    // resource, return null;
			    //
			    return null;
			}

		    }
		});

	    if (clip != null)
		return new PluginAudioClip(clip);
	    else
		return null;
	}
	else
	    return null;
    }


    //
    // TODO: This is a hack to accomodate ActivatorAppletContext.stopAudioClips().
    // When trying to stop all audioclips, a download is being triggered,
    // which causes a deadlock.  We will revisit this when we have more time.
    //
    public void setDoDownload(boolean doDownload) {
	this.doDownload = doDownload;
    }
}


class PluginAudioClip implements AudioClip
{
    private AudioClip clip = null;

    public PluginAudioClip(AudioClip clip)
    {
	this.clip = clip;
    }

    public void loop()
    {
	if (clip != null)
	    clip.loop();
    }

    public void play()
    {
	if (clip != null)
	    clip.play();
    }

    public void stop()
    {
	if (clip != null)
	    clip.stop();
    }

    public void finalize()
    {
	// Make sure audio clip is stopped before GC,
	// this is to avoid locking up the audio system.
	stop();
	
	clip = null;
    }
}

