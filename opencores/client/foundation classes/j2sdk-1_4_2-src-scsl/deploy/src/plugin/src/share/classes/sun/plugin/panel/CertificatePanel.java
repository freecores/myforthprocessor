/*
 * @(#)CertificatePanel.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/**
 * Panel to display information about the certificates to be used by 
 * the javaplugin
 *
 * @version 	1.0 
 * @author	Stanley Man-Kit Ho
 *
 * @version 	1.1 
 * @author	Dennis Gu	
 */

import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;


final class CertificatePanel extends ActivatorSubPanel 
				implements ActionListener {


    /**
     * Contruct a new certificate panel with radio buttons and display window 
     */
    CertificatePanel(ConfigurationInfo model) {
	super(model);

        setLayout(new BoxLayout(this, BoxLayout.X_AXIS));

	// Create "Settings" panel and border
        JPanel settingsPanel = new JPanel();
        Border border = BorderFactory.createEtchedBorder();
        settingsPanel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
				BorderFactory.createTitledBorder(border, mh.getMessage("settings"))));

	// Add a panel to hold four radio buttons 
	JPanel RadioPanel = new JPanel();
	RadioPanel.setLayout(new BoxLayout(RadioPanel, BoxLayout.Y_AXIS));

	// Create four radio buttons
	RButton1 = new JRadioButton(mh.getMessage("rbutton_signed_applet"));
        RButton1.setSelected(true);

        RButton2 = new JRadioButton(mh.getMessage("rbutton_secure_site"));
        RButton3 = new JRadioButton(mh.getMessage("rbutton_signer_ca"));
        RButton4 = new JRadioButton(mh.getMessage("rbutton_secure_site_ca"));

	// Add four radio buttons into a ButtonGroup 
        CertGroup = new ButtonGroup();
        CertGroup.add(RButton1);
        CertGroup.add(RButton2);
        CertGroup.add(RButton3);
        CertGroup.add(RButton4);

	// Add listener to each radio button
        RButton1.addActionListener(this);
        RButton2.addActionListener(this);
        RButton3.addActionListener(this);
        RButton4.addActionListener(this);

	// Add four radio buttons to panel
	RadioPanel.add(RButton1);
	RadioPanel.add(RButton2);
	RadioPanel.add(RButton3);
	RadioPanel.add(RButton4);
	
	// Add panel to Main panel
	settingsPanel.add(RadioPanel);
   	add(settingsPanel);

	// Add a panel to display certificates 
	CertAllPanel = new CertAllPanel(model);
	CertAllPanel.setBorder(BorderFactory.createEmptyBorder(6, 0, 4, 4));
	add(CertAllPanel);
    }

    public void actionPerformed(ActionEvent e) {

	if (e.getSource()==RButton1)
	{
	    CertAllPanel.setRadioPos(mh.getMessage("SignedApplet_value"));
	}
        if (e.getSource()==RButton2)
	{
	    CertAllPanel.setRadioPos(mh.getMessage("SecureSite_value"));
	}
        if (e.getSource()==RButton3)
	{
	    CertAllPanel.setRadioPos(mh.getMessage("SignerCA_value"));
	}
        if (e.getSource()==RButton4)
	{
	    CertAllPanel.setRadioPos(mh.getMessage("SecureSiteCA_value"));
	}

	CertAllPanel.reset();
    }

    /**
     * Refill the list box with the active set of certificates. Disable
     * UI if necessary.
     */
    void reset() {

	CertAllPanel.reset();

    }

    private JRadioButton RButton1, RButton2, RButton3, RButton4;
    private ButtonGroup  CertGroup;
    private CertAllPanel CertAllPanel;

    private MessageHandler mh = new MessageHandler("cert");
}

