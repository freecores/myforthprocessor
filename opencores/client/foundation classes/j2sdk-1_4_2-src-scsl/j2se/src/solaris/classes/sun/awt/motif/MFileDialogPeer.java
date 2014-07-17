/*
 * @(#)MFileDialogPeer.java	1.32 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.motif;

import java.awt.*;
import java.awt.peer.*;
import java.io.*;
import java.awt.datatransfer.*;
import sun.awt.datatransfer.ToolkitThreadBlockedHandler;

public class MFileDialogPeer extends MDialogPeer implements FileDialogPeer {
    private FilenameFilter filter;
    native void create(MComponentPeer parent);
    void create(MComponentPeer parent, Object arg) {
	create(parent);
    }
    public MFileDialogPeer(FileDialog target) {
	super(target);
	FileDialog	fdialog = (FileDialog)target;
	String		dir = fdialog.getDirectory();
        String          file = fdialog.getFile();
	FilenameFilter  filter = fdialog.getFilenameFilter();

	insets = new Insets(0, 0, 0, 0);
	setDirectory(dir);
	if (file != null) {
	    setFile(file);
	}
	    setFilenameFilter(filter);
    }
    native void		pReshape(int x, int y, int width, int height);
    native void         pDispose();
    native void		pShow();
    native void		pHide();
    native void		setFileEntry(String dir, String file, String[] ffiles);
    native void insertReplaceFileDialogText(String l);
    public native void	setFont(Font f);

    String getFilteredFile(String file) {
        if (file == null) {
            file = ((FileDialog)target).getFile();
        }
        String dir = ((FileDialog)target).getDirectory();
        if (dir == null) {
            dir = "./";
        }
        if (file == null) {
            file = "";
        }
        if (filter != null && !filter.accept(new File(dir), file)) {
            file = "";
        }        
        return file;
    }
    // NOTE: This method is called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void		handleSelected(final String file) {
        final FileDialog fileDialog = (FileDialog)target;
        MToolkit.executeOnEventHandlerThread(fileDialog, new Runnable() {
            public void run() {
                int index = file.lastIndexOf(java.io.File.separatorChar);/*2509*//*ibm*/
                String dir;

                if (index == -1) {
                    dir = "."+java.io.File.separator;
                    fileDialog.setFile(file);
                } else {
                    dir = file.substring(0, index + 1);
                    fileDialog.setFile(file.substring(index + 1));
                }
                fileDialog.setDirectory(dir);
                fileDialog.hide();
            }
        });
    } // handleSelected()

    // NOTE: This method is called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void		handleCancel() {
        final FileDialog fileDialog = (FileDialog)target;
        MToolkit.executeOnEventHandlerThread(fileDialog, new Runnable() {
            public void run() {
                fileDialog.setFile(null);
                fileDialog.hide();
            }
        });
    } // handleCancel()

    // NOTE: This method is called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void		handleQuit() {
        final FileDialog fileDialog = (FileDialog)target;
        MToolkit.executeOnEventHandlerThread(fileDialog, new Runnable() {
            public void run() {
                fileDialog.hide();
            }
        });
    } // handleQuit()

    public  void setDirectory(String dir) {
        String file = ((FileDialog)target).getFile();
	setFileEntry((dir != null) ? dir : "./", (file != null) ? file
		     : "", null);
    }


    public  void setFile(String file) {
        String dir = ((FileDialog)target).getDirectory();
        if (dir == null) {
            dir = "./";
        }
        setFileEntry((dir != null) ? dir : "./", getFilteredFile(null), null);
    }
    class DirectoryFilter implements FilenameFilter {
        FilenameFilter userFilter;
        DirectoryFilter(FilenameFilter userFilter) {
            this.userFilter = userFilter;
        }
        public boolean accept(File parent, String name) {
            File toTest = new File(parent, name);
            if (toTest.isDirectory()) {
                return false;
            } else if (userFilter != null) {
                return userFilter.accept(parent, name);
            } else {
                return true;
            }
        }
    }
    public void doFilter(FilenameFilter filter, String dir) {
        String d = (dir == null) ? (((FileDialog)target).getDirectory()):(dir);
        String f = getFilteredFile(null);       
	File df = new File((d != null) ? d : ".");        
	String[] files = df.list(new DirectoryFilter(filter));
	if ( files == null ) {
	    files = new String[1];
	    files[0] = "";
	}

	setFileEntry((d != null) ? d : ".", (f != null) ? f : "", files);
    }
    private boolean proceedFiltering(final String dir, boolean isPrivileged) {
        // If we are not on the Toolkit thread we can call doFilter() directly.
        // If the filter is null no user code will be invoked
        if (!isPrivileged || filter == null) {
            try {
                doFilter(filter, dir);
                return true;
            } catch(Exception e) {
                e.printStackTrace();
                return false;
            }               
        }
        // Otherwise we have to call user code on EvenDispatchThread
        final ToolkitThreadBlockedHandler priveleged_lock = 
            MToolkitThreadBlockedHandler.getToolkitThreadBlockedHandler();
        final boolean[] finished = new boolean[1];
        final boolean[] result = new boolean[1];
        finished[0] = false;
        result[0] = false;


        // Use the same Toolkit blocking mechanism as in DnD.
        priveleged_lock.lock();

        MToolkit.executeOnEventHandlerThread((FileDialog)target, new Runnable() {
            public void run() {
                priveleged_lock.lock();
                try {
                    doFilter(filter, dir);
                    result[0] = true;
                } catch (Exception e) {
                    e.printStackTrace();
                    result[0] = false;
                } finally {
                    finished[0] = true;
                    priveleged_lock.exit();
                    priveleged_lock.unlock();
                }
            }
        });     

        while (!finished[0]) {
            priveleged_lock.enter();
        }

        priveleged_lock.unlock();

        return result[0];
    }

    public void setFilenameFilter(FilenameFilter filter) {
        this.filter = filter;
        FileDialog      fdialog = (FileDialog)target;
        String          dir = fdialog.getDirectory();
        String          file = fdialog.getFile();
        setFile(file);
        doFilter(filter, null);
    }

    // Called from native widget when paste key is pressed and we
    // already own the selection (prevents Motif from hanging while
    // waiting for the selection)
    //
    public void pasteFromClipboard() {
        Clipboard clipboard = target.getToolkit().getSystemClipboard();
        
	Transferable content = clipboard.getContents(this);
	if (content != null) {
	    try {
		String data = (String)(content.getTransferData(DataFlavor.stringFlavor));
		insertReplaceFileDialogText(data);
	    } catch (Exception e) {       
	    }
        }
    }

// CAVEAT:
// Peer coalescing code turned over the fact that the following functions
// were being inherited from Dialog and were not implemented in awt_FileDialog.c
// Five methods decribed by the peer interface are at fault (setResizable, setTitle,
// toFront, toBack and handleFocusTraversalEvent).  Additionally show has to be overridden
// as it was necessary to add a show function in MDialogPeer for modality flag passing.
// As a result we were winding up in  awt_Dialog.c (now coalesced into awt_TopLevel).
// As Filedialogs are modal and its unclear to me that any of these functions
// can be called while the FD is on-screen let it go.  RJM.
    public void show() {
        // must have our own show or we wind up in pShow for Window. Bad. Very bad.
        setVisible(true);
        setFilenameFilter(filter);
    }
}
