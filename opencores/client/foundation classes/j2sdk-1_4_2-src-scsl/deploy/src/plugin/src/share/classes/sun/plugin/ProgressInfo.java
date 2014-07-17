/*
 * @(#)ProgressInfo.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;
import sun.plugin.util.Trace;
import sun.plugin.resources.ResourceHandler;
import java.util.StringTokenizer;
import sun.applet.AppletPanel;


/**
 * ProgressInfo is a class that encapsulated the progress status
 * of the overall progress when an applet is loading. Information
 * captured in this class is mainly used by the applet gray box
 * to draw the progress bar properly.
 *
 * If the applet contains no jar file, progress are updated as 
 * follows:
 *
 * - For each file being downloaded, the progress bar step size 
 *   is half of the remaining unpainted progress bar.
 * - For each file being downloaded in the process, the progress is
 *   updated according to the percentage of the work completed
 *   within the step size.
 *
 * Thus, the progress bar is updated as 50%, 75%, 87.5%, etc.
 *
 *
 * If the applet contains jar files, progress bar are updated
 * as follows:
 *
 * - The progress bar is divided into equal pieces based on 
 *   number of preload files. 
 * - For each jar file being downloaded in the process, the 
 *   progress is updated accordingly to the percentage of the
 *   work completed within the step size.
 *
 * For any case other than the above two, the progress bar is
 * not updated. 
 */
public class ProgressInfo
{
    // Current progress.  
    private int progress = 0;

    // Base progress for incremental progress update
    private int base_progress = 0;    

    // A step is the overall delta progress for each progress update
    private int step;

    // Maximum progress - use 10000 to provide more
    // precise division.
    private int progress_max = 10000;
      
    // Total number of JAR files for this applet.
    private int jar_num = 0;
    
    // Current JAR file (being loaded).
    private int current_jar = 0;

    // Applet viewer corresponded to this progress info
    private AppletViewer av = null;

    // Pattern to match file name
    private Pattern pattern;

    int count = 0;

    /**
     * Construct ProgressInfo object.
     */
    ProgressInfo()
    {
	try
	{
	    // Create pattern for matching file name
	    pattern = Pattern.compile(".*(jar|jarjar|zip)$", Pattern.CASE_INSENSITIVE);
	}
	catch (PatternSyntaxException e1)
	{
	    e1.printStackTrace();
	}	
	catch (IllegalArgumentException e2)
	{
	    e2.printStackTrace();
	}
    }

    /**
     * Initialize ProgressInfo.
     *
     * @param av AppletViewer object associated
     */
    public void init(AppletViewer av)
    {
	this.av = av;

	// Determine the number of jar files
	jar_num = calcJars();

	if (jar_num != 0)
	{
	    // If there is jar file
	    step = (int) (progress_max * 1.0 / jar_num);
	}
	else
	{
	    // Set the step to maximum
	    step = progress_max;
	}
    }

    public synchronized void doOnStart (Object identifier)
    {
	if (jar_num != 0)
	{
	    if (av.getLoadingStatus() == AppletPanel.APPLET_START)
	    {
		// Check if identifier is jar file
		Matcher m = pattern.matcher(identifier.toString());
 
 		if (m.matches())
		{
		    av.showStatusText(ResourceHandler.getMessage("progress_info.downloading"));
		}
	    }
	}
	else
	{
	    // Divide steps into half
	    step = step / 2;
	}
    } 

    public synchronized void doOnProgress(Object identifier, int current, int max)
    {
	if (jar_num != 0)
	{
	    // Check if identifier is jar file
	    Matcher m = pattern.matcher(identifier.toString());

	    if (m.matches())
	    {
		progress = (int) (step * (current_jar + (current * 1.0 / max)));                    
	    }   
	}            
        else
        {
            progress = base_progress + (int) (step * (current * 1.0 / max));
        }             

	// Bounds-checking
	if (progress > progress_max)
	    progress = progress_max;
    }
    
    public synchronized void doOnFinish(Object identifier)
    {
        if (jar_num != 0)
	{
	    // Check if identifier is jar file
	    Matcher m = pattern.matcher(identifier.toString());

	    if (m.matches())
	    {
		// Increment number of jar files downloaded
		current_jar++;                    

		// Updated progress with respect to jar files
		progress = step * current_jar;
	    }
	}        
        else
        {
            /*
             * This applet is not in a JAR.
             */
            progress = base_progress + step;
	    base_progress = progress;
        }

	// Bounds-checking
	if (progress > progress_max)
	{
	    progress = progress_max;
	    base_progress = progress_max;
	}
    }
    
    /**
     * Return current progress.
     */
    public synchronized int getProgress ()
    {
        return progress;
    }
    
    /**
     * Return maximum progress.
     */
    public synchronized int getProgressMax ()
    {
        return progress_max;
    }
    
    public int calcJars ()
    {
        int total_jars = 0;
        StringTokenizer st;
        
        /*
         * See if preload tag specified.  If it is, then use jar files
         * from preload tag only.
         */
        if (av.getParameter("cache_archive_ex") != null){
            st = new StringTokenizer(av.getParameter("cache_archive_ex"), ",", false);
	    while (st.hasMoreTokens()) {
	        String elem = st.nextToken().trim();
	        int optionIndex = elem.indexOf(";preload");
	        if(optionIndex != -1) {
                    total_jars++;
	        }
	    }                    
        }
        
        /*
         * If preload is NOT specified, use all jar files to reflect progress.
         */        
        else if(av.getJarFiles() != null){
	    st = new StringTokenizer(av.getJarFiles(), ",", false);
            while (st.hasMoreTokens()){
                String str = st.nextToken();                    
                if ( ! str.equalsIgnoreCase("null")){
                    total_jars++;
		}
	    }	
        } 
        return total_jars;
    }
}
