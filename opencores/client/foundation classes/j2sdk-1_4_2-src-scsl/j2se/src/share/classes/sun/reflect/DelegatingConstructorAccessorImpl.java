/*
 * @(#)DelegatingConstructorAccessorImpl.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

import java.lang.reflect.InvocationTargetException;

/** Delegates its invocation to another ConstructorAccessorImpl and can
    change its delegate at run time. */

class DelegatingConstructorAccessorImpl extends ConstructorAccessorImpl {
    private ConstructorAccessorImpl delegate;

    DelegatingConstructorAccessorImpl(ConstructorAccessorImpl delegate) {
        setDelegate(delegate);
    }    

    public Object newInstance(Object[] args)
      throws InstantiationException,
             IllegalArgumentException,
             InvocationTargetException
    {
        return delegate.newInstance(args);
    }

    void setDelegate(ConstructorAccessorImpl delegate) {
        this.delegate = delegate;
    }
}
