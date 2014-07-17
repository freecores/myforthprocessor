/*
 * @(#)CertificateDialog.java	1.30 -  03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.util.Date;
import com.sun.javaws.Resources;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.Main;


/** Shows the Certificate dialog */
public class CertificateDialog {
    

    static boolean verifyCertificate(final Frame parent,
			             final String title,
			             final Certificate[] chain,
				     int type) {
        boolean trusted = false;
        boolean isInstaller = false;
        String appType = null;
	
        switch(type) {
	    case LaunchDesc.APPLICATION_DESC_TYPE:
		appType = Resources.getString("certs.verify.application-type");
		isInstaller = false;
		break;
		
	    case LaunchDesc.APPLET_DESC_TYPE:
		appType = Resources.getString("certs.verify.applet-type");
		isInstaller = false;
		break;
		
	    case LaunchDesc.LIBRARY_DESC_TYPE:
		appType = Resources.getString("certs.verify.library-type");
		isInstaller = true;
		break;
		
	    case LaunchDesc.INSTALLER_DESC_TYPE:
		appType = Resources.getString("certs.verify.installer-type");
		isInstaller = true;
		break;
	}
        
        try {
	    CertificateInfo cinfo;
	    CertificateInfo vInfo;
            int trustedIndex = -1;
            cinfo = vInfo = CertificateInfo.getCertificateInfo(chain[0]);
            for (int i=0; i< chain.length; i++) {
                vInfo = CertificateInfo.getCertificateInfo(chain[i]);
                if (KeyStoreManager.isCertificateTrusted(chain[i])) {
                    trustedIndex = i;
                    break;               // end loop at trusted cert
                }
	    }
            SunSecurityUtil.checkTrustedChain(chain);

	    final int trustedIdx = trustedIndex;

	    // Get data from certificate into local variables
	    String unknownString  = Resources.getString("certs.verify.unkown");
	    String subjectName    = buildString(cinfo.getSubjectName(), 
						unknownString);
	    String issuerName     = buildString(vInfo.getSubjectName(), 
						unknownString);
	    Date expirationDate   = cinfo.getExpirationDate();
	    String msg = Resources.getString("certs.verify.msg");
	    
	    int defaultAction = 0;
	    String bottomMsg;
	    
	    if (trustedIdx == -1) {
		bottomMsg = Resources.getString("certs.verify.untrustedCert");
		defaultAction = 2; // Make cancle default action
	    } else {
		msg += Resources.getString("certs.verify.msg.trusted");
		bottomMsg = Resources.getString("certs.verify.trustedCert");
		defaultAction = 0;
	    }
	    JTextPane top = GeneralUtilities.addBoldText(null, msg,
		new Object[] { appType, title, subjectName, issuerName });
	    JTextPane bottom = GeneralUtilities.addBoldText(null,
		bottomMsg, new Object[] { subjectName, subjectName });
	    
	    // The launch/install button text depend if we are 
	    // executing an installer or not
	    String okResource = (isInstaller) ? 
		"certs.verify.okinst" : "certs.verify.ok";
	    
	    final JButton okButton = new JButton(
		Resources.getString(okResource));
	    okButton.setMnemonic(Resources.getVKCode((isInstaller) ? 
		"certs.verify.okinstMnemonic" : "certs.verify.okMnemonic"));
	    okButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
			    Component c = (Component) ae.getSource();
			    while (c != null) {
				c = c.getParent();
				if (c instanceof JOptionPane) {
				    ((JOptionPane)c).setValue(okButton);
				    break;
				}
			    }
			}
		    });
	    
	    
	    JButton detailsButton = new JButton(
		Resources.getString("certs.verify.details"));
	    detailsButton.setMnemonic(
		Resources.getVKCode("certs.verify.detailsMnemonic"));
	    detailsButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
			    showDetails(title, parent, chain, trustedIdx);
			}
		    });
	    
	    Object[] options = {
		okButton,
		    detailsButton,
		    Resources.getString("certs.verify.cancel")
	    };
	    
	    int chosen = GeneralUtilities.showOptionDialog
		(parent, new Object[] { top, bottom },
		 Resources.getString("certs.verify.title"),
		 JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE,
		 options, options[defaultAction]);
	    
	    // Quit JavaWS if user does not accept certificate
	    if (chosen != 0) Main.systemExit(-1);
	    
        } catch(CertificateException ce) {
	    if (Globals.TraceSecurity) {
		Debug.println("CertificateException: "+ce);
	    }
	    bogusCertificate(title, parent);
	    return true;
        }
        
        return true;
    }
    
    private static void showDetails(String title, Frame parent, 
			Certificate[] chain, int trustedIdx) {
        try {
	    CertificateInfo[] certs = new CertificateInfo[chain.length];
	    for (int counter = 0; counter < chain.length; counter++) {
		certs[counter] = CertificateInfo.getCertificateInfo
		    (chain[counter]);
	    }
	    
	    CertificatePanel.showCertificateDialog(parent, certs, trustedIdx);
        } catch (CertificateException ce) {
	    if (Globals.TraceSecurity) {
		Debug.println("CertificateException: "+ce);
	    }
	    bogusCertificate(title, parent);
        }
    }
    
    private static void bogusCertificate(String title, Frame parent) {
        // Code has been signed by an unknown type of certificte
        Object[] options = {
		Resources.getString("certs.verify.cancel")
        };
        
        int chosen = GeneralUtilities.showOptionDialog
	    (parent, Resources.getString("certs.verify.unknownCert", title),
	     Resources.getString("certs.verify.title"),
	     JOptionPane.DEFAULT_OPTION,
	     JOptionPane.WARNING_MESSAGE,
	     options, options[0]);
        
        // Quit JavaWS if user does not accept certificate
        if (chosen == 0) Main.systemExit(-1);
    }
    
    /** Helper methods to avoid null and empty strings */
    private static String buildString(String c1, String c2) {
        return (c1 == null || c1.length() == 0) ? c2 : c1;
    }
}


