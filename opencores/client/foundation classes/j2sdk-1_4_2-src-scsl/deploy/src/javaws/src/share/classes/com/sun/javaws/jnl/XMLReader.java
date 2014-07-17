/*
 * $Id: XMLReader.java,v 1.1 2000/03/31 06:18:56 mode Exp $
 *
 * The Apache Software License, Version 1.1
 *
 *
 * Copyright (c) 2000 The Apache Software Foundation.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:  
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Crimson" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written 
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation and was
 * originally based on software copyright (c) 1999, Sun Microsystems, Inc., 
 * http://www.sun.com.  For more information on the Apache Software 
 * Foundation, please see <http://www.apache.org/>.
 */

package com.sun.javaws.jnl;

import java.io.*;
import java.util.Hashtable;


// NOTE:  Add I18N support to this class when JDK gets the ability to
// defer selection of locale for exception messages ... use the same
// technique for both.


/**
 * This handles several XML-related tasks that normal java.io Readers
 * don't support, inluding use of IETF standard encoding names and
 * automatic detection of most XML encodings.  The former is needed
 * for interoperability; the latter is needed to conform with the XML
 * spec.  This class also optimizes reading some common encodings by
 * providing low-overhead unsynchronized Reader support.
 *
 * <P> Note that the autodetection facility should be used only on
 * data streams which have an unknown character encoding.  For example,
 * it should never be used on MIME text/xml entities.
 *
 * <P> Note that XML processors are only required to support UTF-8 and
 * UTF-16 character encodings.  Autodetection permits the underlying Java
 * implementation to provide support for many other encodings, such as
 * US-ASCII, ISO-8859-5, Shift_JIS, EUC-JP, and ISO-2022-JP.
 *
 * @author David Brownell
 * @version $Revision: 1.1 $
 */
// package private
final class XMLReader extends Reader
{
    private static final int MAXPUSHBACK = 512;

    private Reader	in;
    private String	assignedEncoding;
    private boolean	closed;

    //
    // This class always delegates I/O to a reader, which gets
    // its data from the very beginning of the XML text.  It needs
    // to use a pushback stream since (a) autodetection can read
    // partial UTF-8 characters which need to be fully processed,
    // (b) the "Unicode" readers swallow characters that they think
    // are byte order marks, so tests fail if they don't see the
    // real byte order mark.
    //
    // It's got do this efficiently:  character I/O is solidly on the
    // critical path.  (So keep buffer length over 2 Kbytes to avoid
    // excess buffering. Many URL handlers stuff a BufferedInputStream
    // between here and the real data source, and larger buffers keep
    // that from slowing you down.)
    //

    /**
     * Constructs the reader from an input stream, autodetecting
     * the encoding to use according to the heuristic specified
     * in the XML 1.0 recommendation.
     *
     * @param in the input stream from which the reader is constructed
     * @exception IOException on error, such as unrecognized encoding
     */
    public static Reader createReader (InputStream in) throws IOException
    {
	return new XMLReader (in);
    }

    /**
     * Creates a reader supporting the given encoding, mapping
     * from standard encoding names to ones that understood by
     * Java where necessary.
     *
     * @param in the input stream from which the reader is constructed
     * @param encoding the IETF standard name of the encoding to use;
     *	if null, autodetection is used.
     * @exception IOException on error, including unrecognized encoding
     */
    public static Reader createReader (InputStream in, String encoding)
    throws IOException
    {
	if (encoding == null)
	    return new XMLReader (in);
	//
	// What we really want is an administerable resource mapping
	// encoding names/aliases to classnames.  For example a property
	// file resource, "readers/mapping.props", holding and a set
	// of readers in that (sub)package... defaulting to this call
	// only if no better choice is available.
	//
	return new InputStreamReader (in, std2java (encoding));
    }

    //
    // JDK doesn't know all of the standard encoding names, and
    // in particular none of the EBCDIC ones IANA defines (and
    // which IBM encourages).
    //
    static private final Hashtable charsets = new Hashtable (31);

    static {
	charsets.put ("UTF-16", "Unicode");
	charsets.put ("ISO-10646-UCS-2", "Unicode");

	// NOTE: no support for ISO-10646-UCS-4 yet.

	charsets.put ("EBCDIC-CP-US", "cp037");
	charsets.put ("EBCDIC-CP-CA", "cp037");
	charsets.put ("EBCDIC-CP-NL", "cp037");
	charsets.put ("EBCDIC-CP-WT", "cp037");

	charsets.put ("EBCDIC-CP-DK", "cp277");
	charsets.put ("EBCDIC-CP-NO", "cp277");
	charsets.put ("EBCDIC-CP-FI", "cp278");
	charsets.put ("EBCDIC-CP-SE", "cp278");

	charsets.put ("EBCDIC-CP-IT", "cp280");
	charsets.put ("EBCDIC-CP-ES", "cp284");
	charsets.put ("EBCDIC-CP-GB", "cp285");
	charsets.put ("EBCDIC-CP-FR", "cp297");

	charsets.put ("EBCDIC-CP-AR1", "cp420");
	charsets.put ("EBCDIC-CP-HE", "cp424");
	charsets.put ("EBCDIC-CP-BE", "cp500");
	charsets.put ("EBCDIC-CP-CH", "cp500");

	charsets.put ("EBCDIC-CP-ROECE", "cp870");
	charsets.put ("EBCDIC-CP-YU", "cp870");
	charsets.put ("EBCDIC-CP-IS", "cp871");
	charsets.put ("EBCDIC-CP-AR2", "cp918");

	// IANA also defines two that JDK 1.2 doesn't handle:
	//	EBCDIC-CP-GR		--> CP423
	//	EBCDIC-CP-TR		--> CP905
    }

    // returns an encoding name supported by JDK >= 1.1.6
    // for some cases required by the XML spec
    private static String std2java (String encoding)
    {
	String temp = encoding.toUpperCase ();
	temp = (String) charsets.get (temp);
	return temp != null ? temp : encoding;
    }

    /** Returns the standard name of the encoding in use */
    public String getEncoding ()
    {
	return assignedEncoding;
    }

    private XMLReader (InputStream stream) throws IOException
    {
	super (stream);

	PushbackInputStream	pb;
        byte			buf [];
	int			len;

	pb = new PushbackInputStream (stream, MAXPUSHBACK);
        //
        // See if we can figure out the character encoding used
        // in this file by peeking at the first few bytes.
        //
	buf = new byte [4];
	len = pb.read (buf);
	if (len > 0)
	    pb.unread (buf, 0, len);

	if (len == 4) switch (buf [0] & 0x0ff) {
            case 0:
              // 00 3c 00 3f == illegal UTF-16 big-endian
              if (buf [1] == 0x3c && buf [2] == 0x00 && buf [3] == 0x3f) {
		  setEncoding (pb, "UnicodeBig");
                  return;
              }
	      // else it's probably UCS-4
	      break;

            case '<':      // 0x3c: the most common cases!
              switch (buf [1] & 0x0ff) {
                // First character is '<'; could be XML without
		// an XML directive such as "<hello>", "<!-- ...",
		// and so on.
                default:
                  break;

                // 3c 00 3f 00 == illegal UTF-16 little endian
                case 0x00:
                  if (buf [2] == 0x3f && buf [3] == 0x00) {
		      setEncoding (pb, "UnicodeLittle");
		      return;
                  }
		  // else probably UCS-4
		  break;

                // 3c 3f 78 6d == ASCII and supersets '<?xm'
                case '?': 
                  if (buf [2] != 'x' || buf [3] != 'm')
		      break;
		  //
		  // One of several encodings could be used:
                  // Shift-JIS, ASCII, UTF-8, ISO-8859-*, etc
		  //
		  useEncodingDecl (pb, "UTF8");
                  return;
              }
	      break;

            // 4c 6f a7 94 ... some EBCDIC code page
            case 0x4c:
              if (buf [1] == 0x6f
		    && (0x0ff & buf [2]) == 0x0a7
		    && (0x0ff & buf [3]) == 0x094) {
		  useEncodingDecl (pb, "CP037");
		  return;
	      }
	      // whoops, treat as UTF-8
	      break;

            // UTF-16 big-endian
            case 0xfe:
              if ((buf [1] & 0x0ff) != 0xff)
                  break;
	      setEncoding (pb, "UTF-16");
              return;

            // UTF-16 little-endian
            case 0xff:
              if ((buf [1] & 0x0ff) != 0xfe)
                  break;
	      setEncoding (pb, "UTF-16");
	      return;

            // default ... no XML declaration
            default:
              break;
        }

	//
        // If all else fails, assume XML without a declaration, and
        // using UTF-8 encoding.
	//
	setEncoding (pb, "UTF-8");
    }

    /*
     * Read the encoding decl on the stream, knowing that it should
     * be readable using the specified encoding (basically, ASCII or
     * EBCDIC).  The body of the document may use a wider range of
     * characters than the XML/Text decl itself, so we switch to use
     * the specified encoding as soon as we can.  (ASCII is a subset
     * of UTF-8, ISO-8859-*, ISO-2022-JP, EUC-JP, and more; EBCDIC
     * has a variety of "code pages" that have these characters as
     * a common subset.)
     */
    private void useEncodingDecl (PushbackInputStream pb, String encoding)
    throws IOException
    {
	byte			buffer [] = new byte [MAXPUSHBACK];
	int			len;
	Reader			r;
	int			c;

	//
	// Buffer up a bunch of input, and set up to read it in
	// the specified encoding ... we can skip the first four
	// bytes since we know that "<?xm" was read to determine
	// what encoding to use!
	//
	len = pb.read (buffer, 0, buffer.length);
	pb.unread (buffer, 0, len);
	r = new InputStreamReader (
		new ByteArrayInputStream (buffer, 4, len),
		encoding);

	//
	// Next must be "l" (and whitespace) else we conclude
	// error and choose UTF-8.
	//
	if ((c = r.read ()) != 'l') {
	    setEncoding (pb, "UTF-8");
	    return;
	}

	//
	// Then, we'll skip any
	// 	S version="..." 	[or single quotes]
	// bit and get any subsequent 
	// 	S encoding="..." 	[or single quotes]
	//
	// We put an arbitrary size limit on how far we read; lots
	// of space will break this algorithm.
	//
	StringBuffer	buf = new StringBuffer ();
	StringBuffer	keyBuf = null;
	String		key = null;
	boolean		sawEq = false;
	char		quoteChar = 0;
	boolean		sawQuestion = false;

    XmlDecl:
	for (int i = 0; i < MAXPUSHBACK - 5; ++i) {
	    if ((c = r.read ()) == -1)
		break;

	    // ignore whitespace before/between "key = 'value'"
	    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
		continue;

	    // ... but require at least a little!
	    if (i == 0)
		break;
	    
	    // terminate the loop ASAP
	    if (c == '?')
		sawQuestion = true;
	    else if (sawQuestion) {
		if (c == '>')
		    break;
		sawQuestion = false;
	    }
	    
	    // did we get the "key =" bit yet?
	    if (key == null || !sawEq) {
		if (keyBuf == null) {
		    if (Character.isWhitespace ((char) c))
			continue;
		    keyBuf = buf;
		    buf.setLength (0);
		    buf.append ((char)c);
		    sawEq = false;
		} else if (Character.isWhitespace ((char) c)) {
		    key = keyBuf.toString ();
		} else if (c == '=') {
		    if (key == null)
			key = keyBuf.toString ();
		    sawEq = true;
		    keyBuf = null;
		    quoteChar = 0;
		} else
		    keyBuf.append ((char)c);
		continue;
	    }

	    // space before quoted value
	    if (Character.isWhitespace ((char) c))
		continue;
	    if (c == '"' || c == '\'') {
		if (quoteChar == 0) {
		    quoteChar = (char) c;
		    buf.setLength (0);
		    continue;
		} else if (c == quoteChar) {
		    if ("encoding".equals (key)) {
			assignedEncoding = buf.toString ();

			// [81] Encname ::= [A-Za-z] ([A-Za-z0-9._]|'-')*
			for (i = 0; i < assignedEncoding.length(); i++) {
			    c = assignedEncoding.charAt (i);
			    if ((c >= 'A' && c <= 'Z')
				    || (c >= 'a' && c <= 'z'))
				continue;
			    if (i == 0)
				break XmlDecl;
			    if (i > 0 && (c == '-'
				    || (c >= '0' && c <= '9')
				    || c == '.' || c == '_'))
				continue;
			    // map illegal names to UTF-8 default
			    break XmlDecl;
			}

			setEncoding (pb, assignedEncoding);
			return;

		    } else {
			key = null;
			continue;
		    }
		}
	    }
	    buf.append ((char) c);
	}

	setEncoding (pb, "UTF-8");
    }

    private void setEncoding (InputStream stream, String encoding)
    throws IOException
    {
	assignedEncoding = encoding;
	in = createReader (stream, encoding);
    }

    /**
     * Reads the number of characters read into the buffer, or -1 on EOF.
     */
    public int read (char buf [], int off, int len) throws IOException
    {
	int	val;

	if (closed)
	    return -1;		// throw new IOException ("closed");
	val = in.read (buf, off, len);
	if (val == -1)
	    close ();
	return val;
    }

    /**
     * Reads a single character.
     */
    public int read () throws IOException
    {
	int	val;

	if (closed)
	    throw new IOException ("closed");
	val = in.read ();
	if (val == -1)
	    close ();
	return val;
    }

    /**
     * Returns true iff the reader supports mark/reset.
     */
    public boolean markSupported ()
    {
	return in == null ? false : in.markSupported ();
    }

    /**
     * Sets a mark allowing a limited number of characters to
     * be "peeked", by reading and then resetting.
     * @param value how many characters may be "peeked".
     */
    public void mark (int value) throws IOException
    {
	if (in != null) in.mark (value);
    }

    /**
     * Resets the current position to the last marked position.
     */
    public void reset () throws IOException
    {
	if (in != null) in.reset ();
    }

    /**
     * Skips a specified number of characters.
     */
    public long skip (long value) throws IOException
    {
	return in == null ? 0 : in.skip (value);
    }

    /**
     * Returns true iff input characters are known to be ready.
     */
    public boolean ready () throws IOException
    {
	return in == null ? false : in.ready ();
    }

    /**
     * Closes the reader.
     */
    public void close () throws IOException
    {
	if (closed)
	    return;
	in.close ();
	in = null;
	closed = true;
    }

}
