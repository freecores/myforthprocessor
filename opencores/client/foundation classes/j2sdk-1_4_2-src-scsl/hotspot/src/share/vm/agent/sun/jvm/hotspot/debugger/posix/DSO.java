/*
 * @(#)DSO.java	1.4 03/01/23 11:30:08
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
package sun.jvm.hotspot.debugger.posix;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.posix.elf.*;

/** Provides a simple wrapper around the ELF library which handles
    relocation. */
class DSO {

    private ELFFile file;
    private Address addr;

    DSO(String filename, Address relocation) throws ELFException {
        file = ELFFileParser.getParser().parse(filename);
        addr = relocation;
    }

    DSO(Address addr) throws ELFException {
        file = ELFFileParser.getParser().parse(new AddressDataSource(addr));
        this.addr = addr;
    }

    /** Not currently sure what the ELF parser will not parse.  Always
        return true until we have a problem. */
    boolean isDSO() {
        return true;
    }

    /** Look up a symbol; returns absolute address or null if symbol was
        not found. */
    Address lookupSymbol(String symbol) throws ELFException {
        return addr.addOffsetTo(
                file.getHeader().getELFSymbol(symbol).getOffset());
    }
}
