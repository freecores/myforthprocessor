/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)Klist.java	1.10 03/06/24
 *
 * Portions Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * ===========================================================================
 *  IBM Confidential
 *  OCO Source Materialsx
 *  Licensed Materials - Property of IBM
 * 
 *  (C) Copyright IBM Corp. 1999 All Rights Reserved.
 * 
 *  The source code for this program is not published or otherwise divested of
 *  its trade secrets, irrespective of what has been deposited with the U.S.
 *  Copyright Office.
 * 
 *  Copyright 1997 The Open Group Research Institute.  All rights reserved.
 * ===========================================================================
 * 
 */
package sun.security.krb5.internal.tools;

import sun.security.krb5.*;
import sun.security.krb5.internal.*;
import sun.security.krb5.internal.ccache.*;
import sun.security.krb5.internal.ktab.*;
import sun.security.krb5.KrbCryptoException;
import java.lang.RuntimeException;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.File;

/** 
 * This class can execute as a command-line tool to list entries in
 * credential cache and key tab.  
 *
 * @author Yanni Zhang
 * @author Ram Marti 
 * @version 1.00 10 Jul 30
 */
public class Klist {
    Object target;
    // for credentials cache, options are 'f'  and 'e';
    // for  keytab, optionsare 't' and 'K' and 'e'
    char[] options = new char[3]; 
    String name;       // the name of credentials cache and keytable.
    char action;       // actions would be 'c' for credentials cache
    // and 'k' for keytable.
    private static boolean DEBUG = Krb5.DEBUG;

    /**
     * The main program that can be invoked at command line.
     * <br>Usage: klist
     * [[-c] [-f] [-e]] [-k [-t] [-K]] [name] 
     * -c specifes that credential cache is to be listed 
     * -k specifies that key tab is to be listed
     * name name of the credentials cache or keytab
     * <br>available options for credential caches:
     * <ul>
     * <li><b>-f</b>  shows credentials flags
     * <li><b>-e</b>  shows the encryption type
     * </ul>
     * available options for keytabs:
     * <li><b>-t</b> shows keytab entry timestamps
     * <li><b>-K</b> shows keytab entry DES keys
     */
    public static void main(String[] args) {
        Klist klist = new Klist();
        if ((args == null) || (args.length == 0)) {
            klist.action = 'c'; // default will list default credentials cache.
        } else {
            klist.processArgs(args);
        }  
	switch (klist.action) {
	case 'c':
	    if (klist.name == null) {
		klist.target = CredentialsCache.getInstance();
		klist.name = CredentialsCache.cacheName();
	    } else 
		klist.target = CredentialsCache.getInstance(klist.name);
	    
	    if (klist.target != null)  {
		klist.displayCache();
	    } else {
		klist.displayMessage("Credentials cache");
		System.exit(-1);
	    }
	    break;
	case 'k':
	    if (klist.name == null) {
		klist.target = KeyTab.getInstance();
		klist.name = KeyTab.tabName();
	    } else klist.target = KeyTab.getInstance(klist.name);
	    if (klist.target != null) {
		klist.displayTab();
	    } else {
		klist.displayMessage("KeyTab");
		System.exit(-1);
	    }
	    break;   
	default:
	    if (klist.name != null) {
		klist.printHelp();
		System.exit(-1);
	    } else {
		klist.target = CredentialsCache.getInstance();
		klist.name = CredentialsCache.cacheName();
		if (klist.target != null) {
		    klist.displayCache();
		} else {
		    klist.displayMessage("Credentials cache");
		    System.exit(-1);
		}
	    }
	}
    }

    /**
     * Parses the command line arguments.
     */
    void processArgs(String[] args) {
        Character arg;
        for (int i = 0; i < args.length; i++) {
            if ((args[i].length() >= 2) && (args[i].startsWith("-"))) {
                arg = new Character(args[i].charAt(1));
                switch (arg.charValue()) {
                case 'c':
                    action = 'c';
                    break;
                case 'k':
                    action = 'k';
                    break;
                case 'f': 
                    options[1] = 'f';    
                    break;
                case 'e':
                    options[0] = 'e'; 
                    break;
                case 'K':
                    options[1] = 'K';                     
                    break;
                case 't':
                    options[2] = 't';
                    break;
                default:
                    printHelp();
                    System.exit(-1);
                }

	    } else {
                if (!args[i].startsWith("-") && (i == args.length - 1)) {  
		    // the argument is the last one.
                    name = args[i];
                    arg = null;
                } else {
                    printHelp(); // incorrect input format.
                    System.exit(-1);
                }
            }
        }
    }

    void displayTab() {
        KeyTab table = (KeyTab)target;
        KeyTabEntry[] entries = table.getEntries();
        if (entries.length == 0) {
            System.out.println("\nKey tab: " + name + 
			       ", " + " 0 entries found.\n");
        } else {
            if (entries.length == 1) 
                System.out.println("\nKey tab: " + name + 
				   ", " + entries.length + " entry found.\n");
            else 
		System.out.println("\nKey tab: " + name + ", " + 
				   entries.length + " entries found.\n");
            for (int i = 0; i < entries.length; i++) {
                System.out.println("[" + (i + 1) + "] " +
				   "Service principal: "  +
				   entries[i].getService().toString());
                System.out.println("\t KVNO: " +
				   entries[i].getKey().getKeyVersionNumber());
                if (options[0] == 'e') {
                    EncryptionKey key = ((KeyTabEntry)entries[i]).getKey();
                    System.out.println("\t Key type: " +
				       key.getEType());
                }
                if (options[1] == 'K') {
                    EncryptionKey key = ((KeyTabEntry)entries[i]).getKey();
                    System.out.println("\t Key: " +
				       ((KeyTabEntry)entries[i]).getKeyString());
                }
                if (options[2] == 't') {
                    System.out.println("\t Time stamp: " +
				       reformat(entries[i].getTimeStamp().toDate().toString()));
                }
            }
        }
    }

    void displayCache() {
        CredentialsCache cache = (CredentialsCache)target;
        sun.security.krb5.internal.ccache.Credentials[] creds = 
	    cache.getCredsList();
	if (creds == null) {
	    System.out.println ("No credentials available in the cache " +
				name);
	    System.exit(-1);
	}
        System.out.println("\nCredentials cache: " +  name);
        String defaultPrincipal = cache.getPrimaryPrincipal().toString();
        int num = creds.length;
 
        if (num == 1) 
            System.out.println("\nDefault principal: " +
			       defaultPrincipal + ", " +
			       creds.length + " entry found.\n"); 
        else
            System.out.println("\nDefault principal: " +
			       defaultPrincipal + ", " +
			       creds.length + " entries found.\n"); 
        String starttime = null;
        String endtime = null;
        String servicePrincipal = null;
        String etype = null;
        if (creds != null) {
            for (int i = 0; i < creds.length; i++) {
                try {
                    starttime =
			reformat(creds[i].getAuthTime().toDate().toString());
                    endtime =
			reformat(creds[i].getEndTime().toDate().toString());
                    servicePrincipal = 
			creds[i].getServicePrincipal().toString();
                    System.out.println("[" + (i + 1) + "] " +
				       " Service Principal:  " +
				       servicePrincipal);
                    System.out.println("     Valid starting:  " + starttime);
                    System.out.println("     Expires:         " + endtime);
                    if (options[0] == 'e') {
                        etype = getType(creds[i].getEType());
                        System.out.println("\t Encryption type: " + etype);
                    }
                    if (options[1] == 'f') {
                        System.out.println("\t Flags:           " + 
					   creds[i].getTicketFlags().toString());
                    }
                } catch (RealmException e) {
                    System.out.println("Error reading principal from "+ 
				       "the entry.");
		    if (DEBUG) {
		        e.printStackTrace();
		    }
		    System.exit(-1);
                }
            }
        } else { 
	    System.out.println("\nNo entries found.");
	}
    }      

    void displayMessage(String target) {
        if (name == null) {
            name = "";
        }
        System.out.println(target + " " + name + " not found."); 
    }
    /**
     * Reformats the date from the form - 
     *     dow mon dd hh:mm:ss zzz yyyy to mon/dd/yyyy hh:mm
     * where dow is the day of the week, mon is the month, 
     * dd is the day of the month, hh is the hour of
     * the day, mm is the minute within the hour, 
     * ss is the second within the minute, zzz is the time zone,
     * and yyyy is the year.
     * @param date the string form of Date object.
     */
    String reformat(String date) {
        return (date.substring(4, 7) + " " + date.substring(8, 10) +
		", " + date.substring(24) 
		+ " " + date.substring(11, 16));
    }
    /**
     * Printes out the help information.
     */
    void printHelp() {
        System.out.println("\nUsage: klist " +
                           "[[-c] [-f] [-e]] [-k [-t] [-K]] [name]");
        System.out.println("   name\t name of credentials cache or " +
			   " keytab with the prefix. File-based cache or "
			   + "keytab's prefix is FILE:.");
        System.out.println("   -c specifes that credential cache is to be " + 
			   "listed");
        System.out.println("   -k specifies that key tab is to be listed");
        System.out.println("   options for credentials caches:");
        System.out.println("\t-f \t shows credentials flags");
        System.out.println("\t-e \t shows the encryption type");
        System.out.println("   options for keytabs:");
        System.out.println("\t-t \t shows keytab entry timestamps");  
        System.out.println("\t-K \t shows keytab entry key value"); 
        System.out.println("\t-e \t shows keytab entry key type");
        System.out.println("\nUsage: java sun.security.krb5.tools.Klist " + 
			   "-help for help.");
    }

    String getType(int type) {
        switch (type) {
        case 0:
            return "NULL"; 
        case 1:
            return "DES CBC mode with CRC-32";
        case 2:
            return "DES CBC mode with MD4";
        case 3:
            return "DES CBC mode with MD5";
        case 4:
            return "reserved";
        case 5:
            return "DES3 CBC mode with MD5";
        case 6:
            return "reserved";
        case 7:
            return "DES3 CBC mode with SHA1";
        case 9:
            return "DSA with SHA1- Cms0ID";
        case 10:
            return "MD5 with RSA encryption - Cms0ID";
        case 11:
            return "SHA1 with RSA encryption - Cms0ID";
        case 12:
            return "RC2 CBC mode with Env0ID";
        case 13:
            return "RSA encryption with Env0ID";
        case 14:
            return "RSAES-0AEP-ENV-0ID";
        case 15:
            return "DES-EDE3-CBC-ENV-0ID";
        case 16: 
            return "DES3 CBC mode with SHA1-KD";
        case 23:
            return "RC4 with HMAC";
        case 24:
            return "RC4 with HMAC EXP";

        }
        return "Unknown";
    }
}
