/*
 * @(#)ProgressDialog.java	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.Dimension;
import java.awt.Toolkit;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import sun.plugin.resources.ResourceHandler;
import java.awt.Color;

public class ProgressDialog extends JFrame {
    
    //Time at which dialog was shown.
    private long upTime = 0;
    private long minTime = 2000; //2 seconds.
    
    // The dialog UI components
    private JLabel fileLabel;
    private JProgressBar progressBar;
    private JLabel progressLabel;
    
    private String percentStr;
    private String titleStr;
    private String filename;
    private String labelStr;
    
    private boolean iconified = false;
    
    public ProgressDialog()
    {
    }

    /**
     * Initialize the ProgressDialog.
     *
     * @param str - name of file being loaded (full URL location).
     */    
    public void init(String str) {
        filename = str;
        titleStr = ResourceHandler.getMessage("progress_dialog.downloading");
        if (str.lastIndexOf('/') != -1)
            filename = str.substring(str.lastIndexOf('/')+1);
        
        if(str.lastIndexOf(":/") != -1)
        {
            String tmp = str.trim().substring(0, str.lastIndexOf(":/"));
            
            /*
             * Get locations where file is being downloaded from.
             */
            String http_s = tmp.substring(tmp.lastIndexOf(':')+1);
            if (http_s.equals("http") || http_s.equals("https"))
            {
                /*
                 * This means that format of the string is http://java.sun.com/
                 */
                tmp = str.substring(str.lastIndexOf("://")+3);
                tmp = tmp.substring(0, tmp.indexOf('/'));
                labelStr = filename + " " + ResourceHandler.getMessage("progress_dialog.from")
                + " " + http_s + "://" + tmp + "/";
            }
            
            else if(http_s.trim().equals("file"))
            {
                /*
                 * This means that format of the string is file:/dir_name/
                 */
                tmp = str.substring(str.lastIndexOf(":/")+2);
                tmp = tmp.substring(0, tmp.indexOf('/'));
                labelStr = filename + " " + ResourceHandler.getMessage("progress_dialog.from")
                + " " + http_s + ":/" + tmp + "/";
            }
        }
        
        /*
         * If somehow labelStr is null, set it to be filename.
         */
        if (labelStr == null)
        {
            labelStr = filename;
        }
        
        createComponents();
    }
    
    
    /**
     * Create the dialog's UI components, position Progress Dialog in the
     * center of the screen, show it.
     */
    private void createComponents() {
        
        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
        panel.add(Box.createGlue());
        Dimension vpad5 = new Dimension(1,5);
        panel.add(Box.createRigidArea(vpad5));
        
        fileLabel = new JLabel(labelStr);
        JPanel labelPanel = new JPanel();
        labelPanel.setLayout(new BoxLayout(labelPanel, BoxLayout.X_AXIS));
        labelPanel.add(fileLabel);
        labelPanel.add(Box.createGlue());
        panel.add(labelPanel);
        
        panel.add(Box.createRigidArea(vpad5));
        
        progressBar = new JProgressBar();
        progressBar.setStringPainted(true);
        progressBar.setForeground(new Color(153, 153, 204));
        panel.add(progressBar);
        
        panel.add(Box.createGlue());
        
        JButton closeButton = new JButton(ResourceHandler.getMessage("progress_dialog.dismiss_button"));
        closeButton.setMnemonic(ResourceHandler.getAcceleratorKey("progress_dialog.dismiss_button")); 
        closeButton.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                setVisible(false);
            }
        });
        JPanel tinyPanel = new JPanel();
        tinyPanel.setLayout(new BoxLayout(tinyPanel, BoxLayout.X_AXIS));
        tinyPanel.add(Box.createGlue());
        tinyPanel.add(closeButton);
        tinyPanel.add(Box.createGlue());
        
        panel.add(tinyPanel);
        
        panel.add(Box.createRigidArea(vpad5));
        
        JPanel main_panel = new JPanel();
        main_panel.setLayout(new BoxLayout(main_panel, BoxLayout.X_AXIS));
        main_panel.add(Box.createRigidArea(new Dimension(20, 1)));
        main_panel.add(panel);
        main_panel.add(Box.createRigidArea(new Dimension(20, 1)));
        
        setContentPane(main_panel);
        setTitle(titleStr);
        setSize(500, 150);
        setResizable(false);
        
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                // The user has closed the window
                setVisible(false);
            }
            public void windowIconified(WindowEvent e) {
                // When the window is iconified, show the progress percentage
                // in the title bar so it can be seen in the Windows task bar.
                percentStr = progressBar.getString();
                if (percentStr != null) {
                    setTitle("(" + percentStr + ") - " + filename);
                }
                iconified = true;
            }
            public void windowDeiconified(WindowEvent e) {
                // Remove the progress percentage from the title bar when the
                // image is deiconified.
                setTitle(titleStr);
                iconified = false;
            }
        });
        
        Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension dialog = getSize();
        setLocation((screen.width - dialog.width) / 2,
        (screen.height - dialog.height) / 2);
        
        setVisible(true);
        upTime = System.currentTimeMillis();
    }
    
    
    /**
     *  Adds the given number of bytes to the progress
     *
     * @param n - number of bytes that represent current progress.
     */
    public void addBytes(int n) {
        progressBar.setValue(n);
	if (iconified){
	    percentStr = progressBar.getString();
	    setTitle("(" + percentStr + ") - " + filename);
	}           
    }
    
    /**
     * Set maximum for the JProgressBar.
     * @param n - length of the file we currently loading.
     */
    public void setTotal(int n)
    {
        progressBar.setMaximum(n);
    }
    
    /**
     * Close Progress Dialog window.
     */
    public void close()
    {
        progressBar.setValue(progressBar.getMaximum());
        /*
         * Make sure that the dialog was up for at lease 2 seconds.
         */
        while ((System.currentTimeMillis()-upTime)<minTime)
        {
            try{
                    Thread.sleep(100);
                }
            catch(InterruptedException e){};
        }
        setVisible(false);
    }

    /* 
     * Show the progress dialog again, make progress=0, and start the timer.
     */
    public void reopen(){
        setVisible(true);
        progressBar.setValue(1);
        upTime = System.currentTimeMillis();
    }

}


