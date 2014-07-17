/**
 * @(#)Comment.java	1.38 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import java.util.Locale;

import com.sun.javadoc.*;

import com.sun.tools.javac.v8.util.ListBuffer;

import com.sun.tools.javac.v8.util.Enumeration;


/**
 * Comment contains all information in comment part.
 *      It allows users to get first sentence of this comment, get
 *      comment for different tags...
 *
 * @author Kaiyang Liu (original)
 * @author Robert Field (rewrite)
 * @author Atul M Dambalkar
 * @author Neal Gafter (rewrite)
 */
class Comment {

    /**
     * sorted comments with different tags.
     */
    private final ListBuffer tagList = new ListBuffer();

    /**
     * text minus any tags.
     */
    private String text;

    /**
     * Doc environment
     */
    private final DocEnv docenv;

    /**
     * constructor of Comment.
     */
    Comment(final DocImpl holder, final String commentString) {
        super();
        this.docenv = holder.env;

        /**
         * Separate the comment into the text part and zero to N tags.
         * Simple state machine is in one of three states:
         * <pre>
         * IN_TEXT: parsing the comment text or tag text.
         * TAG_NAME: parsing the name of a tag.
         * TAG_GAP: skipping through the gap between the tag name and
         * the tag text.
         * </pre>
         */
        class CommentStringParser {

            CommentStringParser() {
                super();
            }

            /**
              * The entry point to the comment string parser
              */
            void parseCommentStateMachine() {
                final int IN_TEXT = 1;
                final int TAG_GAP = 2;
                final int TAG_NAME = 3;
                int state = TAG_GAP;
                boolean newLine = true;
                String tagName = null;
                int tagStart = 0;
                int textStart = 0;
                int lastNonWhite = -1;
                int len = commentString.length();
                for (int inx = 0; inx < len; ++inx) {
                    char ch = commentString.charAt(inx);
                    boolean isWhite = Character.isWhitespace(ch);
                    switch (state) {
                    case TAG_NAME:
                        if (isWhite) {
                            tagName = commentString.substring(tagStart, inx);
                            state = TAG_GAP;
                        }
                        break;

                    case TAG_GAP:
                        if (isWhite) {
                            break;
                        }
                        textStart = inx;
                        state = IN_TEXT;

                    case IN_TEXT:
                        if (newLine && ch == '@') {
                            parseCommentComponent(tagName, textStart,
                                    lastNonWhite + 1);
                            tagStart = inx;
                            state = TAG_NAME;
                        }
                        break;

                    } ;
                    if (ch == '\n') {
                        newLine = true;
                    } else if (!isWhite) {
                        lastNonWhite = inx;
                        newLine = false;
                    }
                }
                switch (state) {
                case TAG_NAME:
                    tagName = commentString.substring(tagStart, len);

                case TAG_GAP:
                    textStart = len;

                case IN_TEXT:
                    parseCommentComponent(tagName, textStart, lastNonWhite + 1);
                    break;

                } ;
            }

            /**
              * Save away the last parsed item.
              */
            void parseCommentComponent(String tagName, int from, int upto) {
                String tx = upto <= from ? "" : commentString.substring(from, upto);
                if (tagName == null) {
                    text = tx;
                } else {
                    TagImpl tag;
                    if (tagName.equals("@exception") || tagName.equals("@throws")) {
                        warnIfEmpty(tagName, tx);
                        tag = new ThrowsTagImpl(holder, tagName, tx);
                    } else if (tagName.equals("@param")) {
                        warnIfEmpty(tagName, tx);
                        tag = new ParamTagImpl(holder, tagName, tx);
                    } else if (tagName.equals("@see")) {
                        warnIfEmpty(tagName, tx);
                        tag = new SeeTagImpl(holder, tagName, tx);
                    } else if (tagName.equals("@serialField")) {
                        warnIfEmpty(tagName, tx);
                        tag = new SerialFieldTagImpl(holder, tagName, tx);
                    } else if (tagName.equals("@return")) {
                        warnIfEmpty(tagName, tx);
                        tag = new TagImpl(holder, tagName, tx);
                    } else if (tagName.equals("@author")) {
                        warnIfEmpty(tagName, tx);
                        tag = new TagImpl(holder, tagName, tx);
                    } else if (tagName.equals("@version")) {
                        warnIfEmpty(tagName, tx);
                        tag = new TagImpl(holder, tagName, tx);
                    } else {
                        tag = new TagImpl(holder, tagName, tx);
                    }
                    tagList.append(tag);
                }
            }

            void warnIfEmpty(String tagName, String tx) {
                if (tx.length() == 0) {
                    docenv.warning(holder, "tag.tag_has_no_arguments", tagName);
                }
            }
        }
        new CommentStringParser().parseCommentStateMachine();
    }

    /**
      * Return the text of the comment.
      */
    String commentText() {
        return text;
    }

    /**
      * Return all tags in this comment.
      */
    Tag[] tags() {
        return (Tag[]) tagList.toArray(new Tag[tagList.length()]);
    }

    /**
      * Return tags of the specified kind in this comment.
      */
    Tag[] tags(String tagname) {
        ListBuffer found = new ListBuffer();
        String target = tagname;
        if (target.charAt(0) != '@') {
            target = "@" + target;
        }
        for (Enumeration it = tagList.elements(); it.hasMoreElements();) {
            Tag tag = (Tag) it.nextElement();
            if (tag.kind().equals(target)) {
                found.append(tag);
            }
        }
        return (Tag[]) found.toArray(new Tag[found.length()]);
    }

    /**
      * Return throws tags in this comment.
      */
    ThrowsTag[] throwsTags() {
        ListBuffer found = new ListBuffer();
        for (Enumeration it = tagList.elements(); it.hasMoreElements();) {
            Tag next = (Tag) it.nextElement();
            if (next instanceof ThrowsTag) {
                found.append((ThrowsTag) next);
            }
        }
        return (ThrowsTag[]) found.toArray(new ThrowsTag[found.length()]);
    }

    /**
      * Return param tags in this comment.
      */
    ParamTag[] paramTags() {
        ListBuffer found = new ListBuffer();
        for (Enumeration it = tagList.elements(); it.hasMoreElements();) {
            Tag next = (Tag) it.nextElement();
            if (next instanceof ParamTag) {
                found.append((ParamTag) next);
            }
        }
        return (ParamTag[]) found.toArray(new ParamTag[found.length()]);
    }

    /**
      * Return see also tags in this comment.
      */
    SeeTag[] seeTags() {
        ListBuffer found = new ListBuffer();
        for (Enumeration it = tagList.elements(); it.hasMoreElements();) {
            Tag next = (Tag) it.nextElement();
            if (next instanceof SeeTag) {
                found.append((SeeTag) next);
            }
        }
        return (SeeTag[]) found.toArray(new SeeTag[found.length()]);
    }

    /**
      * Return serialField tags in this comment.
      */
    SerialFieldTag[] serialFieldTags() {
        ListBuffer found = new ListBuffer();
        for (Enumeration it = tagList.elements(); it.hasMoreElements();) {
            Tag next = (Tag) it.nextElement();
            if (next instanceof SerialFieldTag) {
                found.append((SerialFieldTag) next);
            }
        }
        return (SerialFieldTag[]) found.toArray(new SerialFieldTag[found.length()]);
    }

    /**
      * Return array of tags with text and inline See Tags for a Doc comment.
      */
    static Tag[] getInlineTags(DocImpl holder, String inlinetext) {
        ListBuffer taglist = new ListBuffer();
        int delimend = 0;
        int textstart = 0;
        int len = inlinetext.length();
        DocEnv docenv = holder.env;
        if (len == 0) {
            return (Tag[]) taglist.toArray(new Tag[taglist.length()]);
        }
        while (true) {
            int linkstart;
            if ((linkstart = inlineTagFound(holder, inlinetext, textstart)) == -1) {
                taglist.append( new TagImpl(holder, "Text",
                        inlinetext.substring(textstart)));
                break;
            } else {
                int seetextstart = linkstart;
                for (int i = linkstart; i < inlinetext.length(); i++) {
                    char c = inlinetext.charAt(i);
                    if (Character.isWhitespace(c) || c == '}') {
                        seetextstart = i;
                        break;
                    }
                }
                String linkName = inlinetext.substring(linkstart + 2, seetextstart);
                while (Character.isWhitespace(inlinetext.charAt(seetextstart))) {
                    if (inlinetext.length() <= seetextstart) {
                        taglist.append( new TagImpl(holder, "Text",
                                inlinetext.substring(textstart, seetextstart)));
                        docenv.warning(holder, "tag.Improper_Use_Of_Link_Tag",
                                inlinetext);
                        return (Tag[]) taglist.toArray(new Tag[taglist.length()]);
                    } else {
                        seetextstart++;
                    }
                }
                taglist.append( new TagImpl(holder, "Text",
                        inlinetext.substring(textstart, linkstart)));
                textstart = seetextstart;
                if ((delimend = findInlineTagDelim(inlinetext, textstart)) == -1) {
                    taglist.append( new TagImpl(holder, "Text",
                            inlinetext.substring(textstart)));
                    docenv.warning(holder, "tag.End_delimiter_missing_for_possible_SeeTag",
                            inlinetext);
                    return (Tag[]) taglist.toArray(new TagImpl[taglist.length()]);
                } else {
                    if (linkName.equals("see") || linkName.equals("link") ||
                            linkName.equals("linkplain")) {
                        taglist.append( new SeeTagImpl(holder, "@" + linkName,
                                inlinetext.substring(textstart, delimend)));
                    } else {
                        taglist.append( new TagImpl(holder, "@" + linkName,
                                inlinetext.substring(textstart, delimend)));
                    }
                    textstart = delimend + 1;
                }
            }
            if (textstart == inlinetext.length()) {
                break;
            }
        }
        return (Tag[]) taglist.toArray(new Tag[taglist.length()]);
    }

    /**
      * Recursively find the index of the closing '}' character for an inline tag
      * and return it.  If it can't be found, return -1.
      * @param inlineText the text to search in.
      * @param searchStart the index of the place to start searching at.
      * @return the index of the closing '}' character for an inline tag.
      * If it can't be found, return -1.
      */
    private static int findInlineTagDelim(String inlineText, int searchStart) {
        int delimEnd;
        int nestedInlineTag;
        if (searchStart == -1 ||
                (delimEnd = inlineText.indexOf("}", searchStart)) == -1) {
            return -1;
        } else if ( ((nestedInlineTag = inlineText.indexOf("{@", searchStart)) !=
                -1) && nestedInlineTag < delimEnd) {
            int nestedInlineTagDelim =
                    findInlineTagDelim(inlineText, nestedInlineTag + 1);
            return (nestedInlineTagDelim != -1) ?
                    findInlineTagDelim(inlineText, nestedInlineTagDelim + 1) : -1;
        } else {
            return delimEnd;
        }
    }

    /**
      * Recursively search for the string "{@" followed by
      * name of inline tag and white space,
      * if found
      *    return the index of the text following the white space.
      * else
      *    return -1.
      */
    private static int inlineTagFound(DocImpl holder, String inlinetext, int start) {
        DocEnv docenv = holder.env;
        int linkstart;
        if (start == inlinetext.length() ||
                (linkstart = inlinetext.indexOf("{@", start)) == -1) {
            return -1;
        } else if (inlinetext.indexOf('}', start) == -1) {
            docenv.warning(holder, "tag.Improper_Use_Of_Link_Tag",
                    inlinetext.substring(linkstart, inlinetext.length()));
            return -1;
        } else {
            return linkstart;
        }
    }

    /**
      * Return array of tags for the locale specific first sentence in the text.
      */
    static Tag[] firstSentenceTags(DocImpl holder, String text) {
        DocLocale doclocale = holder.env.doclocale;
        return getInlineTags(holder,
                doclocale.localeSpecificFirstSentence(holder, text));
    }

    /**
      * Return text for this Doc comment.
      */
    public String toString() {
        return text;
    }
}
