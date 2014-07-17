/**
 * @(#)Scanner.java	1.36 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.parser;
import java.io.*;

import com.sun.tools.javac.v8.util.*;


/**
 * The lexical analyzer maps an input stream consisting of
 *  ASCII characters and Unicode escapes into a token sequence.
 */
public class Scanner implements Tokens, LayoutCharacters {

    /**
     * The token, set by nextToken().
     */
    int token;

    /**
     * The token's position. pos = line << Position.LINESHIFT + col.
     *  Line and column numbers start at 1.
     */
    int pos;

    /**
     * The last character position of the token.
     */
    int endPos;

    /**
     * The last character position of the previous token.
     */
    int prevEndPos;

    /**
     * The position where a lexical error occurred;
     */
    int errPos = Position.NOPOS;

    /**
     * The name of an identifier or token:
     */
    Name name;

    /**
     * The radix of a numeric literal token.
     */
    int radix;

    /**
     * Has a @deprecated been encountered in last doc comment?
     *  this needs to be reset by client.
     */
    boolean deprecatedFlag = false;

    /**
     * A character buffer for literals.
     */
    private char[] sbuf = new char[128];
    private int sp;

    /**
     * The input buffer, index of next chacter to be read,
     *  index of one past last character in buffer.
     */
    private char[] buf;
    private int bp;
    private int buflen;

    /**
     * The current character.
     */
    private char ch;

    /**
     * The line number position of the current character.
     */
    private int line;

    /**
     * The column number position of the current character.
     */
    private int col;

    /**
     * The buffer index of the last converted unicode character
     */
    private int unicodeConversionBp = 0;

    /**
     * The log to be used for error reporting.
     */
    private final Log log;

    /**
     * The name table.
     */
    private final Name.Table names;

    /**
     * The keyword table.
     */
    private final Keywords keywords;

    /**
     * Documentation string of the current token.
     */
    String docComment = null;

    /**
     * Buffer for doc comment.
     */
    private char[] buffer;

    /**
     * Number of characters in doc comment buffer.
     */
    private int count;

    /**
     * Construct a scanner from a file input stream, a log, and a
     *  character encoding.
     */
    public Scanner(Context context, InputStream in, String encoding) {
        super();
        this.log = Log.instance(context);
        this.names = Name.Table.instance(context);
        this.keywords = Keywords.instance(context);
        try {
            int bufsize = in.available() + 1;
            if (buf == null || buf.length < bufsize)
                buf = new char[bufsize];
            buflen = 0;
            InputStreamReader reader =
                    (encoding == null) ? new InputStreamReader(in) :
                    new InputStreamReader(in, encoding);
            while (true) {
                int nread = reader.read(buf, buflen, buf.length - buflen);
                if (nread < 0)
                    nread = 0;
                buflen = buflen + nread;
                if (buflen < buf.length)
                    break;
                char[] newbuf = new char[buflen * 2];
                System.arraycopy(buf, 0, newbuf, 0, buflen);
                buf = newbuf;
            }
        } catch (UnsupportedEncodingException e) {
            lexError("unsupported.encoding", encoding);
            buf = new char[1];
            buflen = 0;
        }
        catch (IOException e) {
            lexError("io.exception", e.toString());
            buf = new char[1];
            buflen = 0;
        }
        buf[buflen] = EOI;
        line = 1;
        col = 0;
        bp = -1;
        scanChar();
        nextToken();
    }

    /**
      * Report an error at the given position using the provided argument.
      */
    private void lexError(int pos, String msg, String arg) {
        log.error(pos, msg, arg);
        token = ERROR;
        errPos = pos;
    }

    /**
      * Report an error at the given position.
      */
    private void lexError(int pos, String key) {
        lexError(pos, key, null);
    }

    /**
      * Report an error at the current token position.
      */
    private void lexError(String key) {
        lexError(pos, key, null);
    }

    /**
      * Report an error at the current token position using the provided
      *  argument.
      */
    private void lexError(String key, String arg) {
        lexError(pos, key, arg);
    }

    /**
      * Report a warning at the given position.
      */
    private void lexWarning(int pos, String key) {
        log.warning(pos, key);
    }

    /**
      * Convert an ASCII digit from its base (8, 10, or 16)
      *  to its value.
      */
    private int digit(int base) {
        char c = ch;
        int result = Character.digit(c, base);
        if (result >= 0 && c > 127) {
            lexWarning(pos + 1, "illegal.nonascii.digit");
            ch = "0123456789abcdef".charAt(result);
        }
        return result;
    }

    /**
      * Convert unicode escape; bp points to initial '\' character
      *  (Spec 3.3).
      */
    private void convertUnicode() {
        int startcol = col;
        if (ch == '\\') {
            bp++;
            ch = buf[bp];
            col++;
            if (ch == 'u') {
                do {
                    bp++;
                    ch = buf[bp];
                    col++;
                } while (ch == 'u')
                    ;
                int limit = bp + 3;
                if (limit < buflen) {
                    int d = digit(16);
                    int code = d;
                    while (bp < limit && d >= 0) {
                        bp++;
                        ch = buf[bp];
                        col++;
                        d = digit(16);
                        code = (code << 4) + d;
                    }
                    if (d >= 0) {
                        ch = (char) code;
                        unicodeConversionBp = bp;
                        return;
                    }
                }
                lexError(Position.make(line, startcol), "illegal.unicode.esc");
            } else {
                bp--;
                ch = '\\';
                col--;
            }
        }
    }

    /**
      * Read next character.
      */
    private void scanChar() {
        bp++;
        ch = buf[bp];
        switch (ch) {
        case '\r':
            col = 0;
            line++;
            break;

        case '\n':
            if (bp == 0 || buf[bp - 1] != '\r') {
                col = 0;
                line++;
            }
            break;

        case '\t':
            col = (col / TabInc * TabInc) + TabInc;
            break;

        case '\\':
            col++;
            convertUnicode();
            break;

        default:
            col++;
            break;

        }
    }

    /**
      * Read next character in comment, skipping over double '\' characters.
      */
    private void scanCommentChar() {
        scanChar();
        if (ch == '\\') {
            if (buf[bp + 1] == '\\' && unicodeConversionBp != bp) {
                bp++;
                col++;
            } else {
                convertUnicode();
            }
        }
    }

    /**
      * Unconditionally expand the comment buffer.
      */
    private void expandCommentBuffer() {
        char[] newBuffer = new char[buffer.length * 2];
        System.arraycopy(buffer, 0, newBuffer, 0, buffer.length);
        buffer = newBuffer;
    }

    /**
      *
      * Read next character in doc comment, skipping over double '\' characters.
      * If a double '\' is skipped, put in the buffer and update buffer count.
      */
    private void scanDocCommentChar() {
        scanChar();
        if (ch == '\\') {
            if (buf[bp + 1] == '\\' && unicodeConversionBp != bp) {
                if (count == buffer.length)
                    expandCommentBuffer();
                buffer[count++] = ch;
                bp++;
                col++;
            } else {
                convertUnicode();
            }
        }
    }

    /**
      * Append a character to sbuf.
      */
    private void putChar(char ch) {
        if (sp == sbuf.length) {
            char[] newsbuf = new char[sbuf.length * 2];
            System.arraycopy(sbuf, 0, newsbuf, 0, sbuf.length);
            sbuf = newsbuf;
        }
        sbuf[sp++] = ch;
    }

    /**
      * For debugging purposes: print character.
      */
    private void dch() {
        System.err.print((char) ch);
        System.out.flush();
    }

    /**
      * Read next character in character or string literal and copy into sbuf.
      */
    private void scanLitChar() {
        if (ch == '\\') {
            if (buf[bp + 1] == '\\' && unicodeConversionBp != bp) {
                bp++;
                col++;
                putChar('\\');
                scanChar();
            } else {
                scanChar();
                switch (ch) {
                case '0':

                case '1':

                case '2':

                case '3':

                case '4':

                case '5':

                case '6':

                case '7':
                    char leadch = ch;
                    int oct = digit(8);
                    scanChar();
                    if ('0' <= ch && ch <= '7') {
                        oct = oct * 8 + digit(8);
                        scanChar();
                        if (leadch <= '3' && '0' <= ch && ch <= '7') {
                            oct = oct * 8 + digit(8);
                            scanChar();
                        }
                    }
                    putChar((char) oct);
                    break;

                case 'b':
                    putChar('\b');
                    scanChar();
                    break;

                case 't':
                    putChar('\t');
                    scanChar();
                    break;

                case 'n':
                    putChar('\n');
                    scanChar();
                    break;

                case 'f':
                    putChar('\f');
                    scanChar();
                    break;

                case 'r':
                    putChar('\r');
                    scanChar();
                    break;

                case '\'':
                    putChar('\'');
                    scanChar();
                    break;

                case '\"':
                    putChar('\"');
                    scanChar();
                    break;

                case '\\':
                    putChar('\\');
                    scanChar();
                    break;

                default:
                    lexError(Position.make(line, col), "illegal.esc.char");

                }
            }
        } else if (bp != buflen) {
            putChar(ch);
            scanChar();
        }
    }

    /**
      * Read fractional part of floating point number.
      */
    private void scanFraction() {
        while (digit(10) >= 0) {
            putChar(ch);
            scanChar();
        }
        int sp1 = sp;
        if (ch == 'e' || ch == 'E') {
            putChar(ch);
            scanChar();
            if (ch == '+' || ch == '-') {
                putChar(ch);
                scanChar();
            }
            if ('0' <= ch && ch <= '9') {
                do {
                    putChar(ch);
                    scanChar();
                } while ('0' <= ch && ch <= '9')
                    ;
                return;
            }
            lexError("malformed.fp.lit");
            sp = sp1;
        }
    }

    /**
      * Read fractional part and 'd' or 'f' suffix of floating point number.
      */
    private void scanFractionAndSuffix() {
        scanFraction();
        if (ch == 'f' || ch == 'F') {
            putChar(ch);
            scanChar();
            token = FLOATLITERAL;
        } else {
            if (ch == 'd' || ch == 'D') {
                putChar(ch);
                scanChar();
            }
            token = DOUBLELITERAL;
        }
    }

    /**
      * Read a number.
      *  @param radix  The radix of the number; one of 8, 10, 16.
      */
    private void scanNumber(int radix) {
        this.radix = radix;
        int digitRadix = (radix <= 10) ? 10 : 16;
        while (digit(digitRadix) >= 0) {
            putChar(ch);
            scanChar();
        }
        if (radix <= 10 && ch == '.') {
            putChar(ch);
            scanChar();
            scanFractionAndSuffix();
        } else if (radix <= 10 &&
                (ch == 'e' || ch == 'E' || ch == 'f' || ch == 'F' || ch == 'd' ||
                ch == 'D')) {
            scanFractionAndSuffix();
        } else {
            if (ch == 'l' || ch == 'L') {
                scanChar();
                token = LONGLITERAL;
            } else {
                token = INTLITERAL;
            }
        }
    }

    /**
      * Read an identifier.
      */
    private void scanIdent() {
        do {
            if (sp == sbuf.length)
                putChar(ch);
            else
                sbuf[sp++] = ch;
            scanChar();
            switch (ch) {
            case 'A':

            case 'B':

            case 'C':

            case 'D':

            case 'E':

            case 'F':

            case 'G':

            case 'H':

            case 'I':

            case 'J':

            case 'K':

            case 'L':

            case 'M':

            case 'N':

            case 'O':

            case 'P':

            case 'Q':

            case 'R':

            case 'S':

            case 'T':

            case 'U':

            case 'V':

            case 'W':

            case 'X':

            case 'Y':

            case 'Z':

            case 'a':

            case 'b':

            case 'c':

            case 'd':

            case 'e':

            case 'f':

            case 'g':

            case 'h':

            case 'i':

            case 'j':

            case 'k':

            case 'l':

            case 'm':

            case 'n':

            case 'o':

            case 'p':

            case 'q':

            case 'r':

            case 's':

            case 't':

            case 'u':

            case 'v':

            case 'w':

            case 'x':

            case 'y':

            case 'z':

            case '$':

            case '_':

            case '0':

            case '1':

            case '2':

            case '3':

            case '4':

            case '5':

            case '6':

            case '7':

            case '8':

            case '9':
                break;

            default:
                if (!Character.isJavaIdentifierPart(ch) || bp >= buflen) {
                    name = names.fromChars(sbuf, 0, sp);
                    token = keywords.key(name);
                    return;
                }

            }
        } while (true)
            ;
    }

    /**
      * Return true if ch can be part of an operator.
      */
    private boolean isSpecial(char ch) {
        switch (ch) {
        case '!':

        case '%':

        case '&':

        case '*':

        case '?':

        case '+':

        case '-':

        case ':':

        case '<':

        case '=':

        case '>':

        case '^':

        case '|':

        case '~':
            return true;

        default:
            return false;

        }
    }

    /**
      * Read longest possible sequence of special characters and convert
      *  to token.
      */
    private void scanOperator() {
        while (true) {
            putChar(ch);
            Name newname = names.fromChars(sbuf, 0, sp);
            if (keywords.key(newname) == IDENTIFIER) {
                sp--;
                break;
            }
            name = newname;
            token = keywords.key(newname);
            scanChar();
            if (!isSpecial(ch))
                break;
        }
    }

    /**
      * Scan a doccomment line after the inital '*''s for
      *  a @deprecated tag. This should be extended to support all javadoc tags.
      */
    private void scanDocCommentTag() {
        int start = bp + 1;
        do {
            scanDocCommentChar();
        } while ('a' <= ch && ch <= 'z')
            ;
        if (names.fromChars(buf, start, bp - start) == names.deprecated) {
            deprecatedFlag = true;
        }
    }

    /**
      * Skip a non-documentation comment. This method should be called once
      * the initial /, * and the next character have been read.
      */
    private void skipComment() {
        while (bp < buflen) {
            switch (ch) {
            case '*':
                scanChar();
                if (ch == '/')
                    return;
                break;

            default:
                scanCommentChar();
                break;

            }
        }
    }

    /**
      *
      * Scan a documention comment and return it as a string. This method
      * should be called once the initial /, * and * have been read. It
      * gathers the content of the comment (witout leading spaces and '*'s)
      * in the string buffer. Stops on the close '/'.
      */
    private String scanDocComment() {
        if (buffer == null)
            buffer = new char[1024];
        count = 0;
        boolean linestart = false;
        boolean firstLine = true;
        while (bp < buflen && ch == '*') {
            scanDocCommentChar();
        }
        if (bp < buflen && ch == '/') {
            return "";
        }
        if (bp < buflen) {
            if (ch == LF) {
                scanDocCommentChar();
                firstLine = false;
            } else if (ch == CR) {
                scanDocCommentChar();
                if (ch == LF) {
                    scanDocCommentChar();
                    firstLine = false;
                }
            }
        }
        outerLoop:
        while (bp < buflen) {
            wsLoop:
            while (bp < buflen) {
                switch (ch) {
                case ' ':
                    scanDocCommentChar();
                    break;

                case '\t':
                    col = ((col - 1) / TabInc * TabInc) + TabInc;
                    scanDocCommentChar();
                    break;

                case FF:
                    col = 0;
                    scanDocCommentChar();
                    break;

                default:
                    break wsLoop;

                }
            }
            if (ch == '*') {
                do {
                    scanDocCommentChar();
                } while (ch == '*')
                    ;
                if (ch == '/') {
                    break outerLoop;
                }
            } else if (!firstLine) {
                for (int i = 1; i < col; i++) {
                    if (count == buffer.length)
                        expandCommentBuffer();
                    buffer[count++] = ' ';
                }
            }
            linestart = true;
            textLoop:
            while (bp < buflen) {
                switch (ch) {
                case '*':
                    linestart = false;
                    scanDocCommentChar();
                    if (ch == '/') {
                        break outerLoop;
                    }
                    if (count == buffer.length)
                        expandCommentBuffer();
                    buffer[count++] = '*';
                    break;

                case ' ':

                case '\t':
                    if (count == buffer.length)
                        expandCommentBuffer();
                    buffer[count++] = (char) ch;
                    scanDocCommentChar();
                    break;

                case FF:
                    scanDocCommentChar();
                    break textLoop;

                case CR:
                    scanDocCommentChar();
                    if (ch != LF) {
                        if (count == buffer.length)
                            expandCommentBuffer();
                        buffer[count++] = (char) LF;
                        break;
                    }

                case LF:
                    if (count == buffer.length)
                        expandCommentBuffer();
                    buffer[count++] = (char) ch;
                    scanDocCommentChar();
                    break textLoop;

                default:
                    if (ch == '@' && linestart) {
                        int start = bp + 1;
                        do {
                            if (count == buffer.length)
                                expandCommentBuffer();
                            buffer[count++] = (char) ch;
                            scanDocCommentChar();
                        } while ('a' <= ch && ch <= 'z')
                            ;
                        if (names.fromChars(buf, start, bp - start) ==
                                names.deprecated) {
                            deprecatedFlag = true;
                        }
                    } else {
                        if (count == buffer.length)
                            expandCommentBuffer();
                        buffer[count++] = (char) ch;
                        scanDocCommentChar();
                    }
                    linestart = false;

                }
            }
            firstLine = false;
        }
        if (count > 0) {
            int i = count - 1;
            trailLoop:
            while (i > -1) {
                switch (buffer[i]) {
                case '*':
                    i--;
                    break;

                default:
                    break trailLoop;

                }
            }
            count = i + 1;
            return new String(buffer, 0, count);
        } else {
            return "";
        }
    }

    /**
      * The value of a literal token, recorded as a string.
      *  For integers, leading 0x and 'l' suffixes are suppressed.
      */
    public String stringVal() {
        return new String(sbuf, 0, sp);
    }

    /**
      * Read token.
      */
    public void nextToken() {
        try {
            prevEndPos = endPos;
            sp = 0;
            docComment = null;
            while (true) {
                pos = (line << Position.LINESHIFT) + col;
                int start = bp;
                switch (ch) {
                case ' ':

                case '\t':

                case FF:

                case CR:

                case LF:
                    scanChar();
                    break;

                case 'A':

                case 'B':

                case 'C':

                case 'D':

                case 'E':

                case 'F':

                case 'G':

                case 'H':

                case 'I':

                case 'J':

                case 'K':

                case 'L':

                case 'M':

                case 'N':

                case 'O':

                case 'P':

                case 'Q':

                case 'R':

                case 'S':

                case 'T':

                case 'U':

                case 'V':

                case 'W':

                case 'X':

                case 'Y':

                case 'Z':

                case 'a':

                case 'b':

                case 'c':

                case 'd':

                case 'e':

                case 'f':

                case 'g':

                case 'h':

                case 'i':

                case 'j':

                case 'k':

                case 'l':

                case 'm':

                case 'n':

                case 'o':

                case 'p':

                case 'q':

                case 'r':

                case 's':

                case 't':

                case 'u':

                case 'v':

                case 'w':

                case 'x':

                case 'y':

                case 'z':

                case '$':

                case '_':
                    scanIdent();
                    return;

                case '0':
                    scanChar();
                    if (ch == 'x' || ch == 'X') {
                        scanChar();
                        if (digit(16) < 0) {
                            lexError("invalid.hex.number");
                        }
                        scanNumber(16);
                    } else {
                        putChar('0');
                        scanNumber(8);
                    }
                    return;

                case '1':

                case '2':

                case '3':

                case '4':

                case '5':

                case '6':

                case '7':

                case '8':

                case '9':
                    scanNumber(10);
                    return;

                case '.':
                    scanChar();
                    if ('0' <= ch && ch <= '9') {
                        putChar('.');
                        scanFractionAndSuffix();
                    } else {
                        token = DOT;
                    }
                    return;

                case ',':
                    scanChar();
                    token = COMMA;
                    return;

                case ';':
                    scanChar();
                    token = SEMI;
                    return;

                case '(':
                    scanChar();
                    token = LPAREN;
                    return;

                case ')':
                    scanChar();
                    token = RPAREN;
                    return;

                case '[':
                    scanChar();
                    token = LBRACKET;
                    return;

                case ']':
                    scanChar();
                    token = RBRACKET;
                    return;

                case '{':
                    scanChar();
                    token = LBRACE;
                    return;

                case '}':
                    scanChar();
                    token = RBRACE;
                    return;

                case '/':
                    scanChar();
                    if (ch == '/') {
                        do {
                            scanCommentChar();
                        } while (ch != CR && ch != LF && bp < buflen)
                            ;
                        break;
                    } else if (ch == '*') {
                        scanChar();
                        if (ch == '*') {
                            docComment = scanDocComment();
                        } else {
                            skipComment();
                        }
                        if (ch == '/') {
                            scanChar();
                            break;
                        } else {
                            lexError("unclosed.comment");
                            return;
                        }
                    } else if (ch == '=') {
                        name = names.slashequals;
                        token = SLASHEQ;
                        scanChar();
                    } else {
                        name = names.slash;
                        token = SLASH;
                    }
                    return;

                case '\'':
                    scanChar();
                    if (ch == '\'') {
                        lexError("empty.char.lit");
                    } else {
                        if (ch == CR || ch == LF)
                            lexError(pos, "illegal.line.end.in.char.lit");
                        scanLitChar();
                        if (ch == '\'') {
                            scanChar();
                            token = CHARLITERAL;
                        } else {
                            lexError(pos, "unclosed.char.lit");
                        }
                    }
                    return;

                case '\"':
                    scanChar();
                    while (ch != '\"' && ch != CR && ch != LF && bp < buflen)
                        scanLitChar();
                    if (ch == '\"') {
                        token = STRINGLITERAL;
                        scanChar();
                    } else {
                        lexError(pos, "unclosed.str.lit");
                    }
                    return;

                default:
                    if (isSpecial(ch)) {
                        scanOperator();
                    } else if (Character.isJavaIdentifierStart(ch)) {
                        scanIdent();
                    } else if (bp == buflen || ch == EOI && bp + 1 == buflen) {
                        token = EOF;
                    } else {
                        lexError("illegal.char", String.valueOf((int) ch));
                        scanChar();
                    }
                    return;

                }
            }
        }
        finally { endPos = (line << Position.LINESHIFT) + col - 1;
                } }
}
