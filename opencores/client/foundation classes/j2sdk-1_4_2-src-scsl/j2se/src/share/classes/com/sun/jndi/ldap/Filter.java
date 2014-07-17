/*
 * @(#)Filter.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import javax.naming.NamingException;
import javax.naming.directory.InvalidSearchFilterException;

import java.io.IOException;

/**
 * LDAP (RFC-1960) and LDAPv3 (RFC-2254) search filters.
 * 
 * @author Vincent Ryan
 * @author Jagane Sundar
 * @author Rosanna Lee
 */

final class Filter {

    /**
     * First convert filter string into byte[].
     * For LDAP v3, the conversion uses Unicode -> UTF8
     * For LDAP v2, the conversion uses Unicode -> ISO 8859 (Latin-1)
     *
     * Then parse the byte[] as a filter, converting \hh to
     * a single byte, and encoding the resulting filter
     * into the supplied BER buffer
     */
    static void encodeFilterString(BerEncoder ber, String filterStr, 
	boolean isLdapv3) throws IOException, NamingException {

	if ((filterStr == null) || (filterStr.equals(""))) {
	    throw new InvalidSearchFilterException("Empty filter");
	}
	byte[] filter;
	int filterLen;
	if (isLdapv3) {
	    filter = filterStr.getBytes("UTF8");
	} else {
	    filter = filterStr.getBytes("8859_1");
	}
	filterLen = filter.length;
	if (dbg) {
	    dbgIndent = 0;
	    System.err.println("String filter: " + filterStr);
	    System.err.println("size: " + filterLen);
	    dprint("original: ", filter, 0, filterLen);
	}

	encodeFilter(ber, filter, 0, filterLen);
    }

    private static void encodeFilter(BerEncoder ber, byte[] filter, 
	int filterStart, int filterEnd)	throws IOException, NamingException {

	if (dbg) {
	    dprint("encFilter: ",  filter, filterStart, filterEnd);
	    dbgIndent++;
	}

	if ((filterEnd - filterStart) <= 0) {
	    throw new InvalidSearchFilterException("Empty filter");
	}

	int nextOffset;
	int parens, balance;
	boolean escape;

	parens = 0;

	int filtOffset[] = new int[1];

	for (filtOffset[0] = filterStart; 
	     filtOffset[0] < filterEnd; 
	     filtOffset[0]++) {
	    switch (filter[filtOffset[0]]) {
	    case '(':
		filtOffset[0]++;
		parens++;
		switch (filter[filtOffset[0]]) {
		case '&':
		    encodeComplexFilter(ber, filter,
			LDAP_FILTER_AND, filtOffset, filterEnd);
		    parens--;
		    break;

		case '|':
		    encodeComplexFilter(ber, filter,
			LDAP_FILTER_OR, filtOffset, filterEnd);
		    parens--;
		    break;

		case '!':
		    encodeComplexFilter(ber, filter,
			LDAP_FILTER_NOT, filtOffset, filterEnd);
		    parens--;
		    break;

		default:
		    balance = 1;
		    escape = false;
		    nextOffset = filtOffset[0];
		    while (nextOffset < filterEnd && balance > 0) {
			if (!escape) {
			    if (filter[nextOffset] == '(')
				balance++;
			    else if (filter[nextOffset] == ')')
				balance--;
			}
			if (filter[nextOffset] == '\\' && !escape)
			    escape = true;
			else
			    escape = false;
			if (balance > 0)
			    nextOffset++;
		    }
		    if (balance != 0)
			throw new InvalidSearchFilterException(
				  "Unbalanced parenthesis");

		    encodeSimpleFilter(ber, filter, filtOffset[0], nextOffset);

		    // points to right parens; for loop will increment beyond parens
		    filtOffset[0] = nextOffset; 

		    parens--;
		    break;

		}
		break;

	    case ')':
		//
		// End of sequence
		//
		ber.endSeq();
		filtOffset[0]++;
		parens--;
		break;

	    case ' ':
		filtOffset[0]++;
		break;

	    default:	// assume simple type=value filter
		encodeSimpleFilter(ber, filter, filtOffset[0], filterEnd);
		filtOffset[0] = filterEnd; // force break from outer
		break;
	    }
	}

	if (parens > 0) {
	    throw new InvalidSearchFilterException("Unbalanced parenthesis");
	}

	if (dbg) {
	    dbgIndent--;
	}

    }

    /**
     * convert character 'c' that represents a hexadecimal digit to an integer.
     * if 'c' is not a hexidecimal digit [0-9A-Fa-f], -1 is returned.
     * otherwise the converted value is returned.
     */
    private static int hexchar2int( byte c ) {
	if ( c >= '0' && c <= '9' ) {
	    return( c - '0' );
	}
	if ( c >= 'A' && c <= 'F' ) {
	    return( c - 'A' + 10 );
	}
	if ( c >= 'a' && c <= 'f' ) {
	    return( c - 'a' + 10 );
	}
	return( -1 );
    }

    // called by the LdapClient.compare method
    static byte[] unescapeFilterValue(byte[] orig, int start, int end)
	throws NamingException {
	boolean escape = false, escStart = false;
	int ival;
	byte ch;

	if (dbg) {
	    dprint("unescape: " , orig, start, end);
	}

	int len = end - start;
	byte tbuf[] = new byte[len];
	int j = 0;
	for (int i = start; i < end; i++) {
	    ch = orig[i];
	    if (escape) {
		// Try LDAP V3 escape (\\xx)
		if ((ival = hexchar2int(ch)) < 0) {
		    if (escStart) {
			// V2: \* \( \)
			escape = false;
			tbuf[j++] = ch;
		    } else {
			// escaping already started but we can't find 2nd hex
			throw new InvalidSearchFilterException("invalid escape sequence: " + orig);
		    }
		} else {
		    if (escStart) {
			tbuf[j] = (byte)(ival<<4);
			escStart = false;
		    } else {
			tbuf[j++] |= (byte)ival;
			escape = false;
		    }
		}
	    } else if (ch != '\\') {
		tbuf[j++] = ch;
		escape = false;
	    } else {
		escStart = escape = true;
	    }
	}
	byte[] answer = new byte[j];
	System.arraycopy(tbuf, 0, answer, 0, j);
	if (dbg) {
	    Ber.dumpBER(System.err, null, answer, 0, j);
	}
	return answer;
    }

    private static int indexOf(byte[] str, char ch, int start, int end) {
	for (int i = start; i < end; i++) {
	    if (str[i] == ch)
		return i;
	}
	return -1;
    }

    private static int indexOf(byte[] str, String target, int start, int end) {
	int where = indexOf(str, target.charAt(0), start, end);
	if (where >= 0) {
	    for (int i = 1; i < target.length(); i++) {
		if (str[where+i] != target.charAt(i)) {
		    return -1;
		}
	    }
	}
	return where;
    }

    private static int findUnescaped(byte[] str, char ch, int start, int end) {
	while (start < end) {
	    int where = indexOf(str, ch, start, end);
	    // if at start of string, or not there at all, or if not escaped
	    if (where == start || where == -1 || str[where-1] != '\\')
		return where;

	    // start search after escaped star
	    start = where + 1;
	}
	return -1;
    }


    private static void encodeSimpleFilter(BerEncoder ber, byte[] filter, 
	int filtStart, int filtEnd) throws IOException, NamingException {

	if (dbg) {
	    dprint("encSimpleFilter: ", filter, filtStart, filtEnd);
	    dbgIndent++;
	}

	String type, value;
	int valueStart, valueEnd, typeStart, typeEnd;

	int eq;
	if ((eq = indexOf(filter, '=', filtStart, filtEnd)) == -1) {
	    throw new InvalidSearchFilterException("Missing 'equals'");
	}

	    
	valueStart = eq + 1;		// value starts after equal sign
	valueEnd = filtEnd;
	typeStart = filtStart;      // beginning of string

	int ftype;

	switch (filter[eq - 1]) {
	case '<':
	    ftype = LDAP_FILTER_LE;
	    typeEnd = eq - 1;
	    break;
	case '>':
	    ftype = LDAP_FILTER_GE;
	    typeEnd = eq - 1;
	    break;
	case '~':
	    ftype = LDAP_FILTER_APPROX;
	    typeEnd = eq - 1;
	    break;
	case ':':
	    ftype = LDAP_FILTER_EXT;
	    typeEnd = eq - 1;
	    break;
	default:
	    typeEnd = eq;
	    if (findUnescaped(filter, '*', valueStart, valueEnd) == -1) {
		ftype = LDAP_FILTER_EQUALITY;
	    } else if (filter[valueStart] == '*' && valueStart == (valueEnd - 1)) {
		ftype = LDAP_FILTER_PRESENT;
	    } else {
		encodeSubstringFilter(ber, filter, 
		    typeStart, typeEnd, valueStart, valueEnd);
		return;
	    }
	    break;
	}
	if (dbg) {
	    System.err.println("type: " + typeStart + ", " + typeEnd);
	    System.err.println("value: " + valueStart + ", " + valueEnd);
	}
	
	if (ftype == LDAP_FILTER_PRESENT) {
	    ber.encodeOctetString(filter, ftype, typeStart, typeEnd-typeStart);
	} else if (ftype == LDAP_FILTER_EXT) {
	    encodeExtensibleMatch(ber, filter, 
		typeStart, typeEnd, valueStart, valueEnd);
	} else {
	    ber.beginSeq(ftype);
		ber.encodeOctetString(filter, Ber.ASN_OCTET_STR, 
		    typeStart, typeEnd-typeStart);
		ber.encodeOctetString(
		    unescapeFilterValue(filter, valueStart, valueEnd),
		    Ber.ASN_OCTET_STR);
	    ber.endSeq();
	}

	if (dbg) {
	    dbgIndent--;
	}
    }

    private static void encodeSubstringFilter(BerEncoder ber, byte[] filter,
	int typeStart, int typeEnd, int valueStart, int valueEnd)
	throws IOException, NamingException {
	
	if (dbg) {
	    dprint("encSubstringFilter: type ", filter, typeStart, typeEnd);
	    dprint(", val : ", filter, valueStart, valueEnd);
	    dbgIndent++;
	}

	ber.beginSeq(LDAP_FILTER_SUBSTRINGS);
  	    ber.encodeOctetString(filter, Ber.ASN_OCTET_STR, 
		    typeStart, typeEnd-typeStart);
	    ber.beginSeq(LdapClient.LBER_SEQUENCE);
		int index;
		int previndex = valueStart;
		while ((index = findUnescaped(filter, '*', previndex, valueEnd)) != -1) {
		    if (previndex == valueStart) {
		      if (previndex < index) {
			  if (dbg)
			      System.err.println(
				  "initial: " + previndex + "," + index);
			ber.encodeOctetString(
			    unescapeFilterValue(filter, previndex, index),
			    LDAP_SUBSTRING_INITIAL);
		      }
		    } else {
		      if (previndex < index) {
			  if (dbg)
			      System.err.println("any: " + previndex + "," + index);
			ber.encodeOctetString(
			    unescapeFilterValue(filter, previndex, index),
			    LDAP_SUBSTRING_ANY);
		      }
		    }
		    previndex = index + 1;
		}
		if (previndex < valueEnd) {
		    if (dbg)		    
			System.err.println("final: " + previndex + "," + valueEnd);
		  ber.encodeOctetString(
		      unescapeFilterValue(filter, previndex, valueEnd),
		      LDAP_SUBSTRING_FINAL);
		}
	    ber.endSeq();
	ber.endSeq();

	if (dbg) {
	    dbgIndent--;
	}
    }

    private static void encodeComplexFilter(BerEncoder ber, byte[] filter, 
	int filterType,	int filtOffset[], int filtEnd) 
	throws IOException, NamingException {

	//
	// We have a complex filter of type "&(type=val)(type=val)"
	// with filtOffset[0] pointing to the &
	//

	if (dbg) {
	    dprint("encComplexFilter: ", filter, filtOffset[0], filtEnd); 
	    dprint(", type: ", Integer.toString(filterType, 16).getBytes());
	    dbgIndent++;
	}

	filtOffset[0]++;

	ber.beginSeq(filterType);

	    int[] parens = findRightParen(filter, filtOffset, filtEnd);
	    encodeFilterList(ber, filter, parens[0], parens[1]);

	ber.endSeq();

	if (dbg) {
	    dbgIndent--;
	}

    }

    //
    // filter at filtOffset[0] - 1 points to a (. Find ) that matches it
    // and return substring between the parens. Adjust filtOffset[0] to
    // point to char after right paren
    //
    private static int[] findRightParen(byte[] filter, int filtOffset[], int end)
    throws IOException, NamingException {

	int balance = 1;
	boolean escape = false;
	int nextOffset = filtOffset[0];

	while (nextOffset < end && balance > 0) {
	    if (!escape) {
		if (filter[nextOffset] == '(')
		    balance++;
		else if (filter[nextOffset] == ')')
		    balance--;
	    }
	    if (filter[nextOffset] == '\\' && !escape)
		escape = true;
	    else
		escape = false;
	    if (balance > 0)
		nextOffset++;
	}
	if (balance != 0) {
	    throw new InvalidSearchFilterException("Unbalanced parenthesis");
	}

	// String tmp = filter.substring(filtOffset[0], nextOffset);

	int[] tmp = new int[] {filtOffset[0], nextOffset};

	filtOffset[0] = nextOffset + 1;

	return tmp;

    }

    //
    // Encode filter list of type "(filter1)(filter2)..."
    //
    private static void encodeFilterList(BerEncoder ber, byte[] filter, 
	int start, int end) throws IOException, NamingException {

	if (dbg) {
	    dprint("encFilterList: ", filter, start, end);
	    dbgIndent++;
	}

	int filtOffset[] = new int[1];

	for (filtOffset[0] = start; filtOffset[0] < end;
							    filtOffset[0]++) {
	    if (Character.isSpaceChar((char)filter[filtOffset[0]]))
		continue;

	    if (filter[filtOffset[0]] == '(') {
		continue;
	    }

	    int[] parens = findRightParen(filter, filtOffset, end);

	    // add enclosing parens
	    int len = parens[1]-parens[0];
	    byte[] newfilter = new byte[len+2];
	    System.arraycopy(filter, parens[0], newfilter, 1, len);
	    newfilter[0] = (byte)'(';
	    newfilter[len+1] = (byte)')';
	    encodeFilter(ber, newfilter, 0, newfilter.length);
	}

	if (dbg) {
	    dbgIndent--;
	}

    }

    //
    // Encode extensible match
    //
    private static void encodeExtensibleMatch(BerEncoder ber, byte[] filter,
	int matchStart, int matchEnd, int valueStart, int valueEnd)
	throws IOException, NamingException {

	boolean matchDN = false;
	int colon;
	int colon2;
	int i;

	ber.beginSeq(LDAP_FILTER_EXT);

	    // test for colon separator
	    if ((colon = indexOf(filter, ':', matchStart, matchEnd)) >= 0) {

		// test for match DN
		if ((i = indexOf(filter, ":dn", colon, matchEnd)) >= 0) {
		    matchDN = true;
		}

		// test for matching rule
		if (((colon2 = indexOf(filter, ':', colon + 1, matchEnd)) >= 0)
		    || (i == -1)) {

		    if (i == colon) {
			ber.encodeOctetString(filter, LDAP_FILTER_EXT_RULE,
			    colon2 + 1, matchEnd - (colon2 + 1));
			    
		    } else if ((i == colon2) && (i >= 0)) {
			ber.encodeOctetString(filter, LDAP_FILTER_EXT_RULE,
			    colon + 1, colon2 - (colon + 1));

		    } else {
			ber.encodeOctetString(filter, LDAP_FILTER_EXT_RULE,
			    colon + 1, matchEnd - (colon + 1));
		    }
		}
		
		// test for attribute type
		if (colon > matchStart) {
		    ber.encodeOctetString(filter, 
			LDAP_FILTER_EXT_TYPE, matchStart, colon - matchStart);
		}
	    } else {
		ber.encodeOctetString(filter, LDAP_FILTER_EXT_TYPE, matchStart,
		    matchEnd - matchStart);
	    }

	    ber.encodeOctetString(
		unescapeFilterValue(filter, valueStart, valueEnd),
		LDAP_FILTER_EXT_VAL);

	    /*
	     * This element is defined in RFC-2251 with an ASN.1 DEFAULT tag.
	     * However, for Active Directory interoperability it is transmitted
	     * even when FALSE.
	     */
	    ber.encodeBoolean(matchDN, LDAP_FILTER_EXT_DN);

	ber.endSeq();
    }

    ////////////////////////////////////////////////////////////////////////////
    //
    // some debug print code that does indenting. Useful for debugging
    // the filter generation code
    //
    ////////////////////////////////////////////////////////////////////////////

    private static final boolean dbg = false;
    private static int dbgIndent = 0;
    private static void dprint(String msg, byte[] str) {
	dprint(msg, str, 0, str.length);
    }

    private static void dprint(String msg, byte[] str, int start, int end) {
	String dstr = "  ";
	int i = dbgIndent;
	while (i-- > 0) {
	    dstr += "  ";
	}
	dstr += msg;

	System.err.print(dstr);
	for (int j = start; j < end; j++) {
	    System.err.print((char)str[j]);
	}
	System.err.println();
    }

    /////////////// Constants used for encoding filter //////////////

    static final int LDAP_FILTER_AND = 0xa0;
    static final int LDAP_FILTER_OR = 0xa1;
    static final int LDAP_FILTER_NOT = 0xa2;
    static final int LDAP_FILTER_EQUALITY = 0xa3;
    static final int LDAP_FILTER_SUBSTRINGS = 0xa4;
    static final int LDAP_FILTER_GE = 0xa5;
    static final int LDAP_FILTER_LE = 0xa6;
    static final int LDAP_FILTER_PRESENT = 0x87;
    static final int LDAP_FILTER_APPROX = 0xa8;
    static final int LDAP_FILTER_EXT = 0xa9;		// LDAPv3

    static final int LDAP_FILTER_EXT_RULE = 0x81;	// LDAPv3
    static final int LDAP_FILTER_EXT_TYPE = 0x82;	// LDAPv3
    static final int LDAP_FILTER_EXT_VAL = 0x83;	// LDAPv3
    static final int LDAP_FILTER_EXT_DN = 0x84;		// LDAPv3

    static final int LDAP_SUBSTRING_INITIAL = 0x80;
    static final int LDAP_SUBSTRING_ANY = 0x81;
    static final int LDAP_SUBSTRING_FINAL = 0x82;
}
