/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ClassMetaData.java	1.6 03/01/23
 */

package sun.tools.pack.verify;

import java.io.*;
import java.util.*;

import com.sun.classinfo.*;
import com.sun.classinfo.parser.*;
import com.sun.classinfo.validator.*;
import com.sun.classinfo.classfile.*;
import com.sun.classinfo.classfile.exceptions.*;
import com.sun.classinfo.validator.*;
import com.sun.classinfo.validator.Validator.*;

public class ClassMetaData  {
    
    /**
     * Used to compare the Class Meta Data namely the following:
     * 	1. Class Name
     * 	2. SuperClass Name
     * 	3. Magic
     * 	4. Version
     * 	5. Access Flags
     *  6. Class Attributes
     *	7. Fields
     *	8. Methods
     *	9. Implemented Interfaces
     **/
    
    private ClassFile _cf = null;
    private ClassPath  _cp = null;
    
 
    //Counts and basic class data
    private String _name;
    private int[] _version_number;
    private int[] _magic;
    private int _accessflags;
    private String _scname;
    private int _cpool_count;
    private int _fld_count;
    private int _method_count;
    private int _interface_count;
    private int _attribute_count;
    
    /**
     * Method to initialize a ClassMetaData object so that we
     * can reuse the same object.
     */

    public void initClassMetaData(ClassFile cf, ClassPath cp) {
	try {
	    _cp = cp;
	    _cf = cf;
	    _name = _cf.getName();
	    _magic = _cf.getMagicNumber();
	    _accessflags = _cf.getAccessFlags();
	    _version_number = _cf.getVersionNumber();
	    _scname = ( _name.compareTo("java.lang.Object") == 0 || _name.compareTo("java/lang/Object") ==0) ?
		null : _cf.getSuperName();
	    _cpool_count= (_cf.getConstantPool()).getCount();
	    _fld_count = (_cf.getFields()).getCount();
	    _method_count = (_cf.getMethods()).getCount();
	    _interface_count = (_cf.getInterfaces()).getCount();
	    _attribute_count = (_cf.getAttributes()).getCount();
	}
	catch (ClassFileException e) {
	    e.printStackTrace();
	}
    }
 

    private String printArraytoHex(int [] numbers) {
	StringBuffer s = new StringBuffer();
	for (int i = 0; i < numbers.length ; i++) {
	    s = s.append("0x"+Integer.toHexString(numbers[i]) + " ");
	}
	return new String(s);
    }
    
    public String toString() {
	String s = new String("Whence:" + _cp +
	    			"\nClass name:" + _name+
				"\nAccessFlags:" + AccessFlags.classFlagsToString(_accessflags) +
				"\nMagic Number:" + printArraytoHex(_magic) +
				"\nVersionNumber:" + printArraytoHex(_version_number) +
				"\nSuper ClassName:" + _scname +
				"\nCPool Count:" + _cpool_count +
				"\nField Count:" + _fld_count +
				"\nMethod Count:" + _method_count +
				"\nInterfaces Count:" + _interface_count +
				"\nAttribute Count:" + _attribute_count +
				"\n");
	return s;
    }
    
    public boolean equals(Object o) {
	if (o instanceof ClassMetaData) {
	    ClassMetaData that = (ClassMetaData)o;
	    if (this._name.compareTo(that._name) != 0) return false;
	    if (this._accessflags != that._accessflags) return false;
	    if (this._version_number != that._version_number) return false;
	    if ( (this._scname != null && that._scname != null) &&
		this._scname.compareTo(that._scname)!=0) return false;
	    if (this._cpool_count != that._cpool_count) return false;
	    if (this._fld_count != that._fld_count) return false;
	    if (this._method_count != that._method_count) return false;
	    if (this._interface_count != that._interface_count) return false;
	    if (this._attribute_count != that._attribute_count) return false;
	    
	} else {
	    return false;
	}
	return true;
    }
    private void print(String name, String left, String right) {
	Globals.log("Pass1:" + name + ":" + left + "<>" + right);
    }
    
    private void print(String name, int left, int right) {
	Globals.log("Pass1:" + name + ":" + left + "<>" + right);
    }
    
    public void doClassMetaDataDiffs(ClassMetaData that) {
	
	if (this.equals(that)) {
	    Globals.log("Pass1: No differences encountered");
	    return;
	}

	if (this._name.compareTo(that._name) != 0) {
	    print("Name",this._name,that._name);
	}
	
	if (this._version_number[0] != that._version_number[0] ||
	    this._version_number[1] != that._version_number[1]) {
	    print("Version number", printArraytoHex(this._version_number), printArraytoHex(that._version_number));
	}
	
	if (this._magic[0] != that._magic[0] || this._magic[1] != that._magic[1]) {
	    print("Magic", printArraytoHex(this._magic), printArraytoHex(that._magic));
	}
	
	if (this._accessflags != that._accessflags) {
	    print("Access Flags",Integer.toHexString(this._accessflags),Integer.toHexString(that._accessflags));
	}

	if (this._scname != null && this._scname.compareTo(that._scname )!=0) {
	    print("Super Class", this._scname, that._scname);
	} else if (this._scname == null && that._scname != null) {
	    print("Super Class", this._scname, that._scname);
	}
	
	if (this._cpool_count != that._cpool_count) {
	    print("Cpool count", this._cpool_count, that._cpool_count);
	}
	
	if (this._fld_count != that._fld_count) {
	    print("Field count", this._fld_count, that._fld_count);
	}
	
	if (this._method_count != that._method_count) {
	    print("Method count", this._method_count, that._method_count);
	}
	
	if (this._interface_count != that._interface_count) {
	    print("Interface count", this._interface_count, that._interface_count);
	}
	
	if (this._attribute_count != that._attribute_count) {
	    print("Attribute count", this._attribute_count, that._attribute_count);
	}
	
	return ;
    }
}


