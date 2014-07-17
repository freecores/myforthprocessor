/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KeyTab.java	1.11 03/06/24
 *
 * Portions Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * ===========================================================================
 *  IBM Confidential
 *  OCO Source Materials
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

package sun.security.krb5.internal.ktab;

import sun.security.krb5.*;
import sun.security.krb5.internal.*;
import sun.security.krb5.internal.crypto.*;
import java.util.Vector;
import java.util.ArrayList;
import java.io.IOException;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.File;
import java.util.StringTokenizer;
/**
 * This class represents key table. The key table functions deal with storing and
 * retrieving service keys for use in authentication exchanges.
 *
 * @author Yanni Zhang
 * @version 1.00  09 Mar 2000
 */
public class KeyTab implements KeyTabConstants {
    int kt_vno;
    private static KeyTab singleton = null;
    private static final boolean DEBUG = Krb5.DEBUG;
    private static String name;
    private Vector entries = new Vector();

    private KeyTab(String filename) throws IOException, RealmException {
	init(filename);
    }

    public static KeyTab getInstance(String s) {
	name = parse(s);
	if (name == null) { 
	    return getInstance();
	}
	return getInstance(new File(name));
    }

    /**
     * Gets the single instance of KeyTab class.
     * @param file the key tab file.
     * @return single instance of KeyTab; 
     *  return null if error occurs while reading data out of the file.
     */
    public static KeyTab getInstance(File file) {
	try {
            if (!(file.exists())) {
                singleton = null;
            } else {
                String fname = file.getAbsolutePath();
                // Since this class deals with file I/O operations, 
		// we want only one class instance existing.
                if ((singleton != null) && (singleton.name.equalsIgnoreCase(fname))) ;
		else {
		    singleton = new KeyTab(fname);
		}
            }
	} catch (Exception e) {
	    singleton = null;
	    if (DEBUG) {
		System.out.println("Could not obtain an instance of KeyTab" + 
				   e.getMessage());
	    }
	}
	return singleton;
    }              

    /**
     * Gets the single instance of KeyTab class. 
     * @return single instance of KeyTab; return null if default keytab file 
     *  does not exist, or error occurs while reading data from the file. 
     */
    public static KeyTab getInstance() {
	try {
	    name = getDefaultKeyTab();
	    if (name != null) {
		singleton = getInstance(new File(name));
	    }
	} catch (Exception e) {
	    singleton = null;
	    if (DEBUG) {
		System.out.println("Could not obtain an instance of KeyTab" + 
				   e.getMessage());
	    }
	}
	return singleton;
    }

    /* The location of keytab file will be read from the configuration file
     * If it is not specified, consider user.home as the keytab file's 
     * default location.
     */
    private static String getDefaultKeyTab() {
        if (name != null) {
	    return name;
        } else {
	    String kname = null;
	    try {
		String keytab_names = Config.getInstance().getDefault
		    ("default_keytab_name", "libdefaults");
		if (keytab_names != null) {
		    StringTokenizer st = new StringTokenizer(keytab_names, " ");
		    while (st.hasMoreTokens()) {
			kname = parse(st.nextToken());
			if (kname != null) {
			    break;
			}
		    }
		}
	    } catch (KrbException e) {
		kname = null;
	    }

	    if (kname == null) {
		String user_home = (String)java.security.AccessController.doPrivileged(
										       new sun.security.action.GetPropertyAction("user.home"));

		if (user_home == null) {
		    user_home = (String)java.security.AccessController.doPrivileged(
										    new sun.security.action.GetPropertyAction("user.dir"));
		}

		if (user_home != null) {
		    kname = user_home + File.separator  + "krb5.keytab";
		}
	    }
	    return kname;
        } 
    }
    
    private static String parse(String name) {
	String kname = null;
	if (name == null) {
	    return null;
	}
	if ((name.length() >= 5) && (name.substring(0, 5).equalsIgnoreCase("FILE:"))) {
            kname = name.substring(5);
	} else if ((name.length() >= 9) && (name.substring(0, 9).equalsIgnoreCase("ANY:FILE:"))) { //this format found in MIT's krb5.ini.
            kname = name.substring(9);
	} else if ((name.length() >= 7) && (name.substring(0, 7).equalsIgnoreCase("SRVTAB:"))) { //this format found in MIT's krb5.ini.
            kname = name.substring(7);
	} else
	    kname = name;
        return kname;
    }

    private synchronized void init(String filename) throws IOException, RealmException {
	if (filename != null) {
	    KeyTabInputStream kis = new KeyTabInputStream(new FileInputStream(filename));
	    load(kis);
	    kis.close();
	    name = filename;
	}
    }

    private void load(KeyTabInputStream kis) throws IOException, RealmException {
	entries.clear();
	kt_vno = kis.readVersion();
	int entryLength = 0;
	KeyTabEntry entry;
	while (kis.available() > 0) {
	    entryLength = kis.readEntryLength();
	    if (DEBUG) {
		System.out.println(">>> KeyTab: load() entry length: " + entryLength);
	    }
	    entry = kis.readEntry(entryLength, kt_vno);
            if (entry != null) 
		entries.addElement(entry);
	}
    }

    /**
     * Reads the service key from the keytab file.
     * @param service the PrincipalName of the requested service.
     * @return the last service key in the keytab
     */
    public EncryptionKey readServiceKey(PrincipalName service) {
	KeyTabEntry entry = null;
	if (entries != null) {
	    // Find latest entry for this service
	    for (int i = entries.size()-1; i >= 0; i--) {
		entry = (KeyTabEntry)(entries.elementAt(i));
		if (entry.service.match(service)) {  		
		    return new EncryptionKey(entry.keyblock,
					     entry.keyType,
					     new Integer(entry.keyVersion));
		}
	    }
	}
	return null;
    }

    /**
     * Reads all keys for a service from the keytab file.
     * @param service the PrincipalName of the requested service
     * @return an array containing all the service keys
     */
    public EncryptionKey[] readServiceKeys(PrincipalName service) {
	KeyTabEntry entry;
	EncryptionKey key;
	int size = entries.size();
	ArrayList keys = new ArrayList(size);
	if (entries != null) {
	    for (int i = 0; i < size; i++) {
		entry = (KeyTabEntry)(entries.elementAt(i));
		if (entry.service.match(service)) {  		
		    key = new EncryptionKey(entry.keyblock,
					    entry.keyType,
					    new Integer(entry.keyVersion));
		    keys.add(key);
		}
	    }
	}
	size = keys.size();
	if (size == 0)
	return null;
	EncryptionKey[] retVal = new EncryptionKey[size];
	return (EncryptionKey[]) keys.toArray(retVal);
    }

    /**
     * Searches for the service entry in the keytab file.
     * @param service the PrincipalName of the requested service.
     * @return true if the entry is found, otherwise, return false.
     */
    public boolean findServiceEntry (PrincipalName service) {
        KeyTabEntry entry;
	if (entries != null) {
	    for (int i = 0; i < entries.size(); i++) {
		entry = (KeyTabEntry)(entries.elementAt(i));
		if (entry.service.match(service)) 
		    return true;
	    }
	}
	return false;
    }

    public static String tabName() {
        return name;
    }

    /**
     * Adds a new entry in the key table. 
     * @param service the service which will have a new entry in the key table.
     * @param psswd the password which generates the key.
     */
    public void addEntry(PrincipalName service, StringBuffer psswd) throws KrbCryptoException {
	int keyType = Krb5.KEYTYPE_DES;
	byte[] keyValue = Des.string_to_key_bytes(psswd.append(service.getSalt()));
	int result = retrieveEntry(service);
	int kvno = 1;
	if (result != -1) {
	    KeyTabEntry oldEntry = (KeyTabEntry)(entries.elementAt(result));
	    kvno = oldEntry.keyVersion;
	    entries.removeElementAt(result);
	    kvno += 1;
	}
	else kvno = 1;

	KeyTabEntry newEntry = new KeyTabEntry(service, service.getRealm(), new KerberosTime(System.currentTimeMillis()),
					       kvno, keyType, keyValue);
	if (entries == null) 
	    entries = new Vector();
	entries.addElement(newEntry);
    }

    /**
     * Retrieves the key table entry with the specified service name.
     * @param service the service which may have an entry in the key table.
     * @return -1 if the entry is not found, else return the entry index in the list.
     */
    int retrieveEntry(PrincipalName service) {
	int found = -1;
	if (entries != null) {
	    for (int i = 0; i < entries.size(); i++) {
		if (service.match(((KeyTabEntry)(entries.elementAt(i))).getService())) {
		    return i;
		}
	    }
	}
	return found;
    }

    /**
     * Gets the list of service entries in key table.
     * @return array of <code>KeyTabEntry</code>.
     */
    public KeyTabEntry[] getEntries() {
	if (entries != null) {
	    KeyTabEntry[] kentries = new KeyTabEntry[entries.size()];
	    for (int i = 0; i < kentries.length; i++) {
		kentries[i] = (KeyTabEntry)(entries.elementAt(i));
	    }
	    return kentries;
	}
	else return null;
    }

    /**
     * Creates a new default key table.
     */
    public synchronized static KeyTab create() throws IOException, RealmException {
	String dname = getDefaultKeyTab();
	return create(dname);
    }

    /**
     * Creates a new default key table.
     */
    public synchronized static KeyTab create(String name) throws IOException, RealmException {
	KeyTabOutputStream kos = new KeyTabOutputStream(new FileOutputStream(name));
	kos.writeVersion(KRB5_KT_VNO);
	kos.close();
	singleton = new KeyTab(name);
	return singleton;
    }

    /**
     * Saves the file at the directory.
     */
    public synchronized void save() throws IOException {
	KeyTabOutputStream kos = new KeyTabOutputStream(new FileOutputStream(name));
	kos.writeVersion(kt_vno);
	for (int i = 0; i < entries.size(); i++) {
	    kos.writeEntry((KeyTabEntry)entries.elementAt(i));
	}
	kos.close();
    }
    
    /**
     * Removes an entry from the key table.
     * @param service the service <code>PrincipalName</code>.
     */
    public void deleteEntry(PrincipalName service) {
	int result = retrieveEntry(service);
	if (result != -1) {
	    entries.removeElementAt(result);
	}
    }

    /**
     * Creates key table file version.
     * @param file the key table file.
     * @exception IOException.
     */
    public synchronized void createVersion(File file) throws IOException {
	KeyTabOutputStream kos = new KeyTabOutputStream(new FileOutputStream(file));
	kos.write16(KRB5_KT_VNO);
	kos.close();
    }

}
