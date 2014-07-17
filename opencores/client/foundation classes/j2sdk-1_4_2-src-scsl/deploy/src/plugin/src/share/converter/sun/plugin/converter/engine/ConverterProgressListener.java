/*
 * @(#)ConverterProgressListener.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.engine;

public interface ConverterProgressListener extends java.util.EventListener {

    public abstract void converterProgressUpdate(ConverterProgressEvent e);

}

