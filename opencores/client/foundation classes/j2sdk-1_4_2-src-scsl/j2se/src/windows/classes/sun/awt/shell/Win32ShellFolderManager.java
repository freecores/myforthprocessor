/*
 * @(#)Win32ShellFolderManager.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.shell;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.security.AccessController;
import java.util.*;
import sun.security.action.LoadLibraryAction;

/**
 * @author Michael Martak
 * @author Leif Samuelsson
 * @since 1.4
 */

public class Win32ShellFolderManager extends ShellFolderManager {
    
    static {
        // Load library here
        AccessController.doPrivileged(new LoadLibraryAction("awt"));
    }
    
    public ShellFolder createShellFolder(File file) throws FileNotFoundException {
        return new Win32ShellFolder(null, file);
    }
    
    // Special folders
    private static Win32ShellFolder desktop;
    private static Win32ShellFolder drives;
    private static Win32ShellFolder recent;
    private static Win32ShellFolder network;
    private static Win32ShellFolder personal;
    
    private static String osName = System.getProperty("os.name");
    private static boolean isXP = (osName != null && osName.startsWith("Windows XP"));

    static Win32ShellFolder getDesktop() {
        if (desktop == null) {
	    try {
		desktop = new Win32ShellFolder(null, Win32ShellFolder.DESKTOP);
	    } catch (IOException e) {
		desktop = null;
	    }
        }
        return desktop;
    }
    
    static Win32ShellFolder getDrives() {
        if (drives == null) {
	    try {
		drives = new Win32ShellFolder(getDesktop(), Win32ShellFolder.DRIVES);
	    } catch (IOException e) {
		drives = null;
	    }
        }
        return drives;
    }
    
    static Win32ShellFolder getRecent() {
        if (recent == null) {
	    try {
		String path = Win32ShellFolder.getFileSystemPath(Win32ShellFolder.RECENT);
		if (path != null) {
		    recent = new Win32ShellFolder(null, path);
		}
	    } catch (IOException e) {
		recent = null;
	    }
        }
        return recent;
    }
    
    static Win32ShellFolder getNetwork() {
        if (network == null) {
	    try {
		network = new Win32ShellFolder(getDesktop(), Win32ShellFolder.NETWORK);
	    } catch (IOException e) {
		network = null;
	    }
        }
        return network;
    }
    
    static Win32ShellFolder getPersonal() {
        if (personal == null) {
	    try {
		String path = Win32ShellFolder.getFileSystemPath(Win32ShellFolder.PERSONAL);
		if (path != null) {
		    personal = getDesktop().getChildByPath(path);
		    if (personal == null) {
			personal = new Win32ShellFolder(null, path);
		    }
		}
	    } catch (IOException e) {
		personal = null;
	    }
        }
        return personal;
    }
    
    private static File[] roots;

    /**
     * @param key a <code>String</code>
     *  "fileChooserDefaultFolder":
     *    Returns a <code>File</code> - the default shellfolder for a new filechooser
     *	"roots":
     *    Returns a <code>File[]</code> - containing the root(s) of the displayable hierarchy
     *  "fileChooserComboBoxFolders":
     *    Returns a <code>File[]</code> - an array of shellfolders representing the list to
     *    show by default in the file chooser's combobox
     *   "fileChooserShortcutPanelFolders":
     *    Returns a <code>File[]</code> - an array of shellfolders representing well-known
     *    folders, such as Desktop, Documents, History, Network, Home, etc.
     *    This is used in the shortcut panel of the filechooser on Windows 2000
     *    and Windows Me.
     *	"fileChooserIcon nn":
     *    Returns an <code>Image</code> - icon nn from resource 124 in comctl32.dll (Windows only).
     *
     * @return An Object matching the key string.
     */
    public Object get(String key) {
	if (key.equals("fileChooserDefaultFolder")) {
	    File file = getPersonal();
	    if (file == null) {
		file = getDesktop();
	    }
	    return file;
	} else if (key.equals("roots")) {
	    // Should be "History" and "Desktop" ?
	    if (roots == null) {
		File desktop = getDesktop();
		if (desktop != null) {
		    roots = new File[] { desktop };
		} else {
		    roots = (File[])super.get(key);
		}
	    }
	    return roots;
	} else if (key.equals("fileChooserComboBoxFolders")) {
	    Win32ShellFolder desktop = getDesktop();

	    if (desktop != null) {
		ArrayList folders = new ArrayList();
		Win32ShellFolder drives = getDrives();

		folders.add(desktop);
		// Add all second level folders
		File[] secondLevelFolders = desktop.listFiles();
		Arrays.sort(secondLevelFolders);
		for (int j = 0; j < secondLevelFolders.length; j++) {
		    Win32ShellFolder folder = (Win32ShellFolder)secondLevelFolders[j];
		    if (folder.getPIDL() == 0 || folder.isDirectory()) {
			folders.add(folder);
			// Add third level for "My Computer"
			if (folder.equals(drives)) {
			    File[] thirdLevelFolders = folder.listFiles();
			    if (thirdLevelFolders != null) {
				Arrays.sort(thirdLevelFolders, driveComparator);
				for (int k = 0; k < thirdLevelFolders.length; k++) {
				    folders.add(thirdLevelFolders[k]);
				}
			    }
			}
		    }
		}
		return folders.toArray(new File[folders.size()]);
	    } else {
		return super.get(key);
	    }
	} else if (key.equals("fileChooserShortcutPanelFolders")) {
	    File[] folders = new File[] {
		getRecent(), getDesktop(), getPersonal(), getDrives(), getNetwork()
	    };
	    // Remove null references
	    ArrayList list = new ArrayList();
	    for (int i = 0; i < folders.length; i++) {
		if (folders[i] != null) {
		    list.add(folders[i]);
		}
	    }
	    return list.toArray(new File[list.size()]);
	} else if (key.startsWith("fileChooserIcon ")) {
	    int i = -1;
	    String name = key.substring(key.indexOf(" ")+1);
	    try {
		i = Integer.parseInt(name);
	    } catch (NumberFormatException ex) {
		if (name.equals("ListView")) {
		    i = (isXP) ? 21 : 2;
		} else if (name.equals("DetailsView")) {
		    i = (isXP) ? 23 : 3;
		} else if (name.equals("UpFolder")) {
		    i = (isXP) ? 28 : 8;
		} else if (name.equals("NewFolder")) {
		    i = (isXP) ? 31 : 11;
		}
	    }
	    if (i >= 0) {
		return Win32ShellFolder.getFileChooserIcon(i);
	    }
	}
	return null;
    }

    /**
     * Does <code>dir</code> represent a "computer" such as a node on the network, or
     * "My Computer" on the desktop.
     */
    public boolean isComputerNode(File dir) {
	if (dir != null && dir.equals(getDrives())) {
	    return true;
	} else {
	    String path = dir.getAbsolutePath();
	    return (path.startsWith("\\\\") && path.indexOf("\\", 2) < 0);	//Network path
	}
    }

    public boolean isFileSystemRoot(File dir) {
	//Note: Removable drives don't "exist" but are listed in "My Computer"
	if (dir != null) {
	    Win32ShellFolder drives = getDrives();
	    if (dir instanceof Win32ShellFolder) {
		Win32ShellFolder sf = (Win32ShellFolder)dir;
		if (sf.isFileSystem()) {
		    if (sf.parent != null) {
			return sf.parent.equals(drives);
		    }
		    // else fall through ...
		} else {
		    return false;
		}
	    }
	    String path = dir.getPath();
	    return (path.length() == 3
		    && path.charAt(1) == ':'
		    && Arrays.asList(drives.listFiles()).contains(dir));
	}
	return false;
    }

    private Comparator driveComparator = new Comparator() {
	public int compare(Object o1, Object o2) {
	    return ((ShellFolder)o1).getPath().compareTo(((ShellFolder)o2).getPath());
	}
    };


    public void sortFiles(List files) {
	Collections.sort(files, fileComparator);
    }

    private Comparator fileComparator = new Comparator() {
	public int compare(Object a, Object b) {
	    return compare((File)a, (File)b);
	}

	public int compare(File f1, File f2) {
	    Win32ShellFolder sf1 = null;
	    Win32ShellFolder sf2 = null;

	    if (f1 instanceof Win32ShellFolder) {
		sf1 = (Win32ShellFolder)f1;
		if (sf1.getPIDL() != 0) {
		    sf1 = null;
		}
	    }
	    if (f2 instanceof Win32ShellFolder) {
		sf2 = (Win32ShellFolder)f2;
		if (sf2.getPIDL() != 0) {
		    sf2 = null;
		}
	    }

	    if (sf1 != null && sf2 != null) {
		return sf1.compareTo(sf2);
	    } else if (sf1 != null) {
		return -1;	// Non-file shellfolders sort before files
	    } else if (sf2 != null) {
		return 1;
	    } else {
		String name1 = f1.getName();
		String name2 = f2.getName();

		// First ignore case when comparing
		int diff = name1.toLowerCase().compareTo(name2.toLowerCase());
		if (diff != 0) {
		    return diff;
		} else {
		    // May differ in case (e.g. "mail" vs. "Mail")
		    // We need this test for consistent sorting
		    return name1.compareTo(name2);
		}
	    }
	}
    };
}
