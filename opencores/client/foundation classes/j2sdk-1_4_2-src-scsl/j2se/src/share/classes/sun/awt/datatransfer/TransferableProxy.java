/*
 * @(#)TransferableProxy.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.datatransfer;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;


/**
 * Proxies for another Transferable so that Serializable objects are never
 * returned directly by DnD or the Clipboard. Instead, a new instance of the
 * object is returned.
 *
 * @author Lawrence P.G. Cable
 * @author David Mendenhall
 * @version 1.4, 01/23/03
 *
 * @since 1.4
 */
public class TransferableProxy implements Transferable {
    public TransferableProxy(Transferable t, boolean local) {
        transferable = t;
        isLocal = local;
    }
    public DataFlavor[] getTransferDataFlavors() {
        return transferable.getTransferDataFlavors();
    }
    public boolean isDataFlavorSupported(DataFlavor flavor) {
	return transferable.isDataFlavorSupported(flavor);
    }
    public Object getTransferData(DataFlavor df)
        throws UnsupportedFlavorException, IOException
    {
	Object data = transferable.getTransferData(df);

	// If the data is a Serializable object, then create a new instance
	// before returning it. This insulates applications sharing DnD and
	// Clipboard data from each other.
	if (data != null && isLocal && df.isFlavorSerializedObjectType()) {
	    ByteArrayOutputStream baos = new ByteArrayOutputStream();
                
	    new ObjectOutputStream(baos).writeObject(data);
                
	    ByteArrayInputStream bais =
		new ByteArrayInputStream(baos.toByteArray());
                
	    try {
		data = new ObjectInputStream(bais).readObject();
	    } catch (ClassNotFoundException cnfe) {
		throw new IOException(cnfe.getMessage());
	    }
	}
        
	return data;
    }

    protected final Transferable transferable;
    protected final boolean isLocal;
}
