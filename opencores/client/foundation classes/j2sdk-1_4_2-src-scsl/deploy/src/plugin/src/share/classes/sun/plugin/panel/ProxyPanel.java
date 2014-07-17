/*
 * @(#)ProxyPanel.java	1.20 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/**
 * Panel to display information about the proxies to be used by 
 * the javaplugin
 *
 * @version 	1.0 
 * @author	Jerome Dochez
 */

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.table.*;
import javax.swing.event.*;
import java.awt.*;   
import java.awt.event.*;
import java.awt.Dimension;
import sun.plugin.net.proxy.BrowserProxyInfo;
import sun.plugin.net.proxy.ProxyUtils;
import sun.plugin.util.Trace;

public class ProxyPanel extends ActivatorSubPanel 
			implements  ActionListener,
				    KeyListener,
				    TableModelListener {

    /**
     * Contruct a new proxy panel
     */
    public ProxyPanel(ConfigurationInfo model) 
    {
	super(model);

	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

	JPanel defaultPanel = new JPanel();
	defaultPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
	useDefault = new JCheckBox(mh.getMessage("use_browser"));
	useDefault.addActionListener(this);
	defaultPanel.add(useDefault);
	add(defaultPanel);

	JPanel panel = new JPanel();

	panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
	panel.setBorder(new EmptyBorder(4, 4, 4, 4));

	Object[] columns = { mh.getMessage("proxy_protocol"), 
			     mh.getMessage("proxy_address"),
			     mh.getMessage("proxy_port") };
	Object[][] data = { { mh.getMessage("http"), null, null },
			    { mh.getMessage("https"), null, null },
			    { mh.getMessage("ftp"), null, null },
			    { mh.getMessage("gopher"), null, null },
			    { mh.getMessage("socks"), null, null } };

	// Create custom table model 
	TableModel tableModel = new DefaultTableModel(data, columns)
	{
	    public boolean isCellEditable(int row, int column) 
	    {
		if (column < 1)
		    return false;
		else
		    return super.isCellEditable(row, column);
	    }
	};

	table = new JTable(tableModel);
	table.getModel().addTableModelListener(this);

	JScrollPane spane = new JScrollPane(table);
	spane.setPreferredSize(new Dimension(100, 100));
	panel.add(spane);
	add(panel);

	add(Box.createRigidArea(new java.awt.Dimension(1,5)));

	JPanel proxyException = new JPanel();
	proxyException.setLayout(new BoxLayout(proxyException, BoxLayout.Y_AXIS));
	proxyException.add(new JLabel(mh.getMessage("bypass")));
	proxyException.add(proxyByPass);
	proxyException.setBorder(new EmptyBorder(0, 5, 0, 5));
	proxyByPass.addKeyListener(this);
	add(proxyException);

	add(Box.createRigidArea(new java.awt.Dimension(1,5)));

	JPanel autoConfig = new JPanel();
	autoConfig.setLayout(new BoxLayout(autoConfig, BoxLayout.Y_AXIS));
	autoConfig.add(new JLabel(mh.getMessage("autourl")));
	autoConfig.add(autoProxyURL);
	autoConfig.setBorder(new EmptyBorder(0, 5, 0, 5));
	autoProxyURL.addKeyListener(this);
	add(autoConfig);

	add(Box.createRigidArea(new java.awt.Dimension(1,5)));

	reset();

    }

    /**
     * Enable / Disable the fields depending on the use choices
     * Forward the choices to the model
     */
    public void actionPerformed(ActionEvent e) {
	if (e.getSource()==useDefault) {
	    boolean b = useDefault.isSelected();
	    model.setDefaultProxySettings(b);
	    reset();
	}
    }

    /** 
     * Key Listener implementation.
     */
    public void keyTyped(KeyEvent e) {
 	setProxySettings();  
    }

    public void keyReleased(KeyEvent e) {
	// This shouldn't be necessary.  Unfortunately it seems to be.
	//					KGH 2/9/98
 	setProxySettings();
    }

    public void keyPressed(KeyEvent e) {
    }

    /** 
     * Reset proxy panel setting. Proxy panel setting should
     * be reinitialized according to model.
     */
    public void reset() 
    {
	boolean usedef  = model.useDefaultProxy();
	useDefault.setSelected(usedef);

	boolean enable = !usedef;

	setEnabled(table, enable);
	setEnabled(proxyByPass, enable);
	setEnabled(autoProxyURL, enable);

	// Populate proxy settings
	initProxySettings();
    }

    /** 
     * Saves the proxy settings 
     * The string is will be of the following format
     *	    protocol=proxy:proxyPort
     * Each protocol info is separated with a semicolumn (;)
     */
    private void setProxySettings() 
    {
	model.setProxyByPass(proxyByPass.getText());
    	model.setAutoProxyURL(autoProxyURL.getText());

	// Determine proxy for HTTP
	StringBuffer sb = new StringBuffer();

	TableModel tableModel = table.getModel();

	String host = (String) tableModel.getValueAt(0, 1);

	if (host != null && !host.equals(""))
	{
	    sb.append("http=");
	    sb.append(host);

	    Object port = tableModel.getValueAt(0, 2);
	    
	    try
	    {
		if (port != null)
		{
		    int value = Integer.parseInt(port.toString());

		    if (value != -1)
		    {
			sb.append(":");
			sb.append(port);
		    }
		}
	    }
	    catch (Throwable e)
	    {
    	    }
	}

	host = (String) tableModel.getValueAt(1, 1);

	if (host != null && !host.equals(""))
	{
	    sb.append(";");
	    sb.append("https=");
	    sb.append(host);

	    Object port = tableModel.getValueAt(1, 2);
	    
	    try
	    {
		if (port != null)
		{
		    int value = Integer.parseInt(port.toString());

		    if (value != -1)
		    {
			sb.append(":");
			sb.append(port);
		    }
		}
	    }
	    catch (Throwable e)
	    {
    	    }
	}

	host = (String) tableModel.getValueAt(2, 1);

	if (host != null && !host.equals(""))
	{
	    sb.append(";");
	    sb.append("ftp=");
	    sb.append(host);

	    Object port = tableModel.getValueAt(2, 2);
	    
	    try
	    {
		if (port != null)
		{
		    int value = Integer.parseInt(port.toString());

		    if (value != -1)
		    {
			sb.append(":");
			sb.append(port);
		    }
		}
	    }
	    catch (Throwable e)
	    {
    	    }
	}

	host = (String) tableModel.getValueAt(3, 1);

	if (host != null && !host.equals(""))
	{
	    sb.append(";");
	    sb.append("gopher=");
	    sb.append(host);

	    Object port = tableModel.getValueAt(3, 2);
	    
	    try
	    {
		if (port != null)
		{
		    int value = Integer.parseInt(port.toString());

		    if (value != -1)
		    {
			sb.append(":");
			sb.append(port);
		    }
		}
	    }
	    catch (Throwable e)
	    {
    	    }
	}

	host = (String) tableModel.getValueAt(4, 1);

	if (host != null && !host.equals(""))
	{
	    sb.append(";");
	    sb.append("socks=");
	    sb.append(host);

	    Object port = tableModel.getValueAt(4, 2);
	    
	    try
	    {
		if (port != null)
		{
		    int value = Integer.parseInt(port.toString());

		    if (value != -1)
		    {
			sb.append(":");
			sb.append(port);
		    }
		}
	    }
	    catch (Throwable e)
	    {
    	    }
	}

	String str = sb.toString();	

	if (str.length()!=0)
	    model.setProxySettingsString(str);
	else
	    model.setProxySettingsString(null);
    }

    /**
     * Load the proxy settings string in the proxy fields.
     * This method should only be called when reset() is
     * trigged, so we will populate all the fields 
     * according to proxySettings from the model.
     */
    private void initProxySettings() 
    {
	proxyByPass.setText(model.getProxyByPass());
	autoProxyURL.setText(model.getAutoProxyURL());

	String proxySettings = model.getProxySettingsString();

	if (proxySettings!=null && proxySettings.length()!=0)
	{
	    try
	    {
		BrowserProxyInfo bpi = new BrowserProxyInfo();

		// Parse proxy string
		ProxyUtils.parseProxyServer(proxySettings, bpi);

		TableModel tableModel = table.getModel();

		tableModel.setValueAt(bpi.getHttpHost(), 0, 1);

		if (bpi.getHttpPort() != -1)
		    tableModel.setValueAt(new Integer(bpi.getHttpPort()), 0, 2);

		tableModel.setValueAt(bpi.getHttpsHost(), 1, 1);

		if (bpi.getHttpsPort() != -1)
    		    tableModel.setValueAt(new Integer(bpi.getHttpsPort()), 1, 2);

		tableModel.setValueAt(bpi.getFtpHost(), 2, 1);

		if (bpi.getFtpPort() != -1)
		    tableModel.setValueAt(new Integer(bpi.getFtpPort()), 2, 2);

		tableModel.setValueAt(bpi.getGopherHost(), 3, 1);

		if (bpi.getGopherPort() != -1)
		    tableModel.setValueAt(new Integer(bpi.getGopherPort()), 3, 2);

		tableModel.setValueAt(bpi.getSocksHost(), 4, 1);

		if (bpi.getSocksPort() != -1)
    		    tableModel.setValueAt(new Integer(bpi.getSocksPort()), 4, 2);
	    }
	    catch (Exception e)
	    {
		Trace.printException(this, e);
	    }	
	}
    }

    /**
     * Enable/Disabled a component
     *
     * @param field the component to enable/disable
     * @param b true to enable to text field, false to disable it
     */
    private void setEnabled(JComponent field, boolean b) {
	field.setEnabled(b); 
	field.repaint();
    }

    public void tableChanged(TableModelEvent e)
    {
 	setProxySettings();  
    }

    private JTable table;
    private JTextField proxyByPass = new JTextField();
    private JTextField autoProxyURL = new JTextField();
    private JCheckBox  useDefault;

    private MessageHandler mh = new MessageHandler("proxy");
}

