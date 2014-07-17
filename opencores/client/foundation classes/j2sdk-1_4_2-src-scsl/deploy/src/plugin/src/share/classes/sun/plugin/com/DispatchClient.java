/*
 * @(#)DispatchClient.java	1.2 02/06/10
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import sun.plugin.javascript.ocx.*;

/**
 *  Dispatch is the Java side companion to the IDispatch COM interface.
 */
public class DispatchClient
{
    private int dispPtr=0;
    private int threadId=0;

    public DispatchClient(int ptr, int tid) {
	dispPtr = ptr;
	threadId = tid;
    }

    public Object invoke(int handle, String methodName, int flag, Object args[]) {
	Object retVal;
	Object [] convertedArgs = null;
	
	//Convert the non primitive types into a DispatchImpl object
	if(args != null) {
	    convertedArgs = Utils.convertArgs(args);
	}

	//invoke the method
	Object result = nativeInvoke(handle, methodName, flag, convertedArgs, dispPtr, threadId);
	if(result != null && result instanceof DispatchClient) {
	    retVal = new JSObject((DispatchClient)result);
	} else {
	    retVal = result;
	}
	
	return retVal; 
    }

    public void release(int handle) {
	nativeRelease(handle, dispPtr);
    }

    public int getDispatchWrapper(){
	return dispPtr;
    }

    public String getDispType(int handle) {
	return nativeGetDispType(handle, dispPtr);
    }

    private native void nativeRelease(int handle, int ptr);
    private native Object nativeInvoke( int handle, String methodName, int flag, 
					Object[] args, int ptr, int tid);
    private native String nativeGetDispType(int handle, int ptr);
}
