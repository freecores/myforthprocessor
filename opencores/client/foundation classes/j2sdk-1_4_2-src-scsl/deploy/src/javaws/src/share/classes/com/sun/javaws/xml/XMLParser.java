/*
 * @(#)XMLParser.java	1.16 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.xml;

import java.io.IOException;
import com.sun.javaws.exceptions.JNLParseException;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;

public class XMLParser {
    public XMLNode _root;
    public String _source;
    public String _current;
    public int    _tokenType;
    public String _tokenData;
    public static final int TOKEN_EOF = 1;			//  null
    public static final int TOKEN_END_TAG = 2;			//  </
    public static final int TOKEN_BEGIN_TAG = 3;		//  <
    public static final int TOKEN_CLOSE_TAG = 4;		//  >
    public static final int TOKEN_EMPTY_CLOSE_TAG = 5;		//  />
    public static final int TOKEN_PCDATA = 6;			//  qwerty
        

    public XMLParser(String source) {

        if (Globals.TraceXMLParsing) {
            Debug.println("new XMLParser with source:");
            Debug.println(source);
        }
        _source = _current = source;
        _root = null;
        _tokenData = null;
    }

    public XMLNode parse() throws JNLParseException {
	

        try {
            nextToken(_current);
            _root = parseXMLElement(_current);
	} catch (NullPointerException npe) {
            if (Globals.TraceXMLParsing) {
                Debug.println("NULL Pointer Exception: "+npe);
            }
            throw new JNLParseException(_source, npe, 
		"wrong kind of token found", getLineNumber());
        } catch (JNLParseException jnlpe) {
            if (Globals.TraceXMLParsing) {
                Debug.println("JNLP Parse Exception: "+jnlpe);
            }
            throw jnlpe;
	}
        if (Globals.TraceXMLParsing) {
            Debug.println("\n\nreturning ROOT as follows:\n"+_root);
        }
        return _root;
    }


    private void nextToken(String source) {

        _current = skipFilling(source);

        if (_current == null) {				// TOKEN_EOF
            _tokenType = TOKEN_EOF;
        } else if (_current.startsWith("</")) {		// TOKEN_END_TAG
            _tokenType = TOKEN_END_TAG;
            _current = skipXMLName(skipForward(_current,2,0));
        } else if (_current.startsWith("<")) {		// TOKEN_BEGIN_TAG
            _tokenType = TOKEN_BEGIN_TAG;
            _current = skipXMLName(skipForward(_current,1,0));
        } else if (_current.startsWith(">")) { 		// TOKEN_CLOSE_TAG
            _tokenType = TOKEN_CLOSE_TAG;
            _current = skipForward(_current,1,0);
        } else if (_current.startsWith("/>")) {		// TOKEN_EMPTY_CLOSE_TAG
            _tokenType = TOKEN_EMPTY_CLOSE_TAG;
            _current = skipForward(_current,2,0);
        } else {					// TOKEN_PCDATA
            _tokenType = TOKEN_PCDATA;
	    _current = skipPCData(source, '<');         // skip till next "<"
	}
    }

    private static final String PCDStart = "<![CDATA[";
    private static final String PCDEnd = "]]>";


    private String skipPCData(String source, char delimitChar) {
	String retval;
        int index = source.indexOf(delimitChar);
        int pcdi = source.indexOf(PCDStart);
        if (index >=0) {
	    if ((pcdi >= 0) && (pcdi <= index)) {
		String pre = source.substring(0,pcdi);
		String remain = source.substring(pcdi+PCDStart.length());
		if (remain != null) {
		    int end = remain.indexOf(PCDEnd);
		    if (end >=0) {
			retval = skipPCData( 
			    remain.substring(end+PCDEnd.length()), delimitChar);
			_tokenData = pre + remain.substring(0, end) + 
				     _tokenData;
			return retval;
		    }
		}
	    }
            _tokenData = source.substring(0,index);;
            if (index < source.length()) {
                retval = source.substring(index);
            } else {
                retval = null;
            }
	    return retval;
        } else {
            _tokenData = _current;   // all the rest;
	    return null;
	}
    }

    private XMLNode parseXMLElement(String source) throws JNLParseException {
        XMLNode self, parent, child, sibling;
        XMLAttribute attribute, nextAttribute, firstAttribute;
        String name;

        if (_tokenType == TOKEN_BEGIN_TAG) {

            name = _tokenData;

            // Parse attributes. This section eats all input until
            // an EOF, a > or a />
            firstAttribute = parseXMLAttribute(_current);
            attribute = firstAttribute;
            while (attribute != null) {
                nextAttribute = parseXMLAttribute(_current);
		attribute.setNext(nextAttribute);
		attribute = nextAttribute;
            }

            // Create node for new element tag
            self =  new XMLNode(name, firstAttribute);

            // This will eihter be a TOKEN_EOF, TOKEN_CLOSE_TAG, or a
            // TOKEN_EMPTY_CLOSE_TAG
            nextToken(_current);
            if ((_tokenType != TOKEN_EMPTY_CLOSE_TAG) &&
                (_tokenType != TOKEN_CLOSE_TAG) &&
                (_tokenType != TOKEN_EOF)) {
                throw new JNLParseException(_source, null,
                        "wrong kind of token found", getLineNumber());
            }

            if (_tokenType == TOKEN_EMPTY_CLOSE_TAG) {
                nextToken(_current);
                // We are done with the sublevel - fall through to
                // continue parsing tags at the same level */
            } else if (_tokenType == TOKEN_CLOSE_TAG) {
                nextToken(_current);

                // Parse until end tag if found
                child = parseXMLElement(_current);
                if (child != null) {
                    self.setNested(child);
                    child.setParent(self);
                }
                if (_tokenType == TOKEN_END_TAG) {
                    // Find closing bracket '>' for end tag
                    do {
                        nextToken(_current);
                    } while ((_tokenType != TOKEN_EOF) &&
                             (_tokenType != TOKEN_CLOSE_TAG));
                    nextToken(_current);
                }
            }
            // Continue parsing rest on same level
            if (_tokenType != TOKEN_EOF) {
                // Parse rest of stream at same level
                sibling = parseXMLElement(_current);
                self.setNext(sibling);
            }
            return self;

        } else if (_tokenType == TOKEN_PCDATA) {
            // Create node for pcdata
            self = new XMLNode(_tokenData);
            nextToken(_current);
            return self;
        }

	return null;

    }

    private XMLAttribute parseXMLAttribute(String source)
                         throws JNLParseException {
        if (source == null) {
            return null;
        }
        _current = skipFilling(source);

        if ((_current == null) || _current.startsWith(">") ||
            _current.startsWith("/>")) {
            return null;
        }
        // extract name
        _current = skipAttributeName(_current);
        String name = _tokenData;
        _current = skipFilling(_current);
        if (!_current.startsWith("=")) {
            // This is really an error. We ignore this, and just try
            // to parse an attribute out of the rest of the string
            if (source.equals(_current)) {
                // avoid infinate loop - move foward 1 ...
                _current = skipForward(_current, 1, 0);
            }
            return parseXMLAttribute(_current);
        }
        _current = skipForward(_current, 1, 0);
        _current = skipWhitespace(_current);

        String value;
        if ((_current.startsWith("\"")) || (_current.startsWith("\'"))) {
	    char quote = _current.charAt(0);
            _current = skipForward(_current, 1, 0);  // past quote
	    _current = skipPCData(_current, quote);  // past PCData
	    value = _tokenData;
            _current = skipForward(_current, 1, 0);  // past endquote
        } else {
            _current = skipNonSpace(_current);
            value = _tokenData;
        }
	if (value != null) {
	    value = value.trim();
	}
        return new XMLAttribute(name, value);
    }

    private String skipForward(String source, int index, int length) {
        if (index < 0 || (index+length) >= source.length()) return null;
        return source.substring(index+length);
    }
        
    private String skipNonSpace(String source) {
        int index = 0;
        if (source == null) {
            return null;
        }
        int length = source.length();
        while ((index < length) && !Character.isWhitespace(
		source.charAt(index))) {
            index++;
        }
	return skipPCData(source, source.charAt(index));
    }

    private String skipWhitespace(String source) {
        int index = 0;
        if (source == null) {
            return null;
        }
        while ((index < source.length()) &&
	        Character.isWhitespace(source.charAt(index))) {
            ++index;
        }
        return source.substring(index);
    }

    private boolean legalTokenStartChar(char c) {
        return ((c >= 'a' && c <= 'z') ||
                (c >= 'a' && c <= 'Z') ||
                (c == '_') || (c == ':' ));
    }

    private boolean legalTokenChar(char c) {
        return ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                (c == '_') || (c == ':' ) ||
                (c == '.') || (c == '-'));
    }

    private String skipAttributeName(String source) {

        if (source == null) {
            return null;
        }
        int index = source.indexOf("=");
        if (index >= 0) {
            _tokenData = source.substring(0, index);
	    if (_tokenData != null) {
		_tokenData = _tokenData.trim();
	    }
        } else {
            _tokenData = null;
        }
        return skipForward(source, index, 0);
    }

    private String skipXMLName(String source) {
        int index = 0;
        if (source == null) {
            return null;
        }
        if (legalTokenStartChar(source.charAt(0))) {
            index = 1;
            while ((index < source.length()) &&
                   legalTokenChar(source.charAt(index))) {
                index++;
            }
        }
        _tokenData = source.substring(0, index);
        if (_tokenData != null) {
	    _tokenData = _tokenData.trim();
        }
        return skipForward(source, index, 0);
    }

    private String skipXMLComment(String source) {
        if ((source != null) && (source.startsWith("<!--"))) {
            int index = source.indexOf("-->", 4);
            return skipForward(source, index, 3);
        }
        return source;
    }

    private String skipXMLDocType(String source) {
        if ((source != null) && (source.startsWith("<!"))) {
            int index = source.indexOf(">",2);
            return skipForward(source, index, 1);
        }
        return source;
    }

    private String skipXMLProlog(String source) {
        if ((source != null) && (source.startsWith("<?"))) {
            int index = source.indexOf("?>",2);
            return skipForward(source, index, 2);
        }
        return source;
    }

    private String skipFilling(String source) {
        String next, prev;
        next = source;
        do {
            prev = next;
            next = skipWhitespace(next);
            next = skipXMLComment(next);
            next = skipXMLDocType(next);
            next = skipXMLProlog(next);
        } while (next != prev);
        return next;
    }

    private int getLineNumber() {
        int end, lineCount;
        if (_current == null) {
            end = _source.length();
        } else {
            end = _source.indexOf(_current);
        }
        lineCount = 0;
        int index = 0;
        while ((index < end) && (index != -1)) {
            index = _source.indexOf("\n", index);
            if (index >= 0) {
		index++;
                lineCount++;
	    }
        } 
        return lineCount;
    }

}


