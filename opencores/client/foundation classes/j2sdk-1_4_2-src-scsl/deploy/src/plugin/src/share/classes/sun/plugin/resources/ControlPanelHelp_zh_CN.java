/*
 * @(#)ControlPanelHelp_zh_CN.java	1.2 03/03/22
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * US English verison of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ControlPanelHelp_zh_CN extends ListResourceBundle {

    private static String newline = "\n";

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "help.file", newline +
                   "Java Plug-in \u63a7\u5236\u9762\u677f\u5e2e\u52a9" + newline + newline + newline +
                   "\u672c\u5e2e\u52a9\u5305\u62ec\u4ee5\u4e0b\u4e3b\u9898\uff1a" + newline +
                   "      \u6982\u8ff0" + newline + 
                   "      \u4fdd\u5b58\u9009\u9879" + newline + 
                   "      \u8bbe\u7f6e\u63a7\u5236\u9762\u677f\u9009\u9879" + newline + 
                   "      \u201c\u57fa\u672c\u201d\u9762\u677f" + newline + 
                   "      \u201c\u9ad8\u7ea7\u201d\u9762\u677f" + newline +
                   "      \u201c\u6d4f\u89c8\u5668\u201d\u9762\u677f" + newline + 
                   "      \u201c\u4ee3\u7406\u201d\u9762\u677f" + newline + 
                   "      \u201c\u9ad8\u901f\u7f13\u5b58\u201d\u9762\u677f" + newline + 
                   "      \u201c\u8bc1\u4e66\u201d\u9762\u677f" + newline + 
                   "      \u201c\u66f4\u65b0\u201d\u9762\u677f" + newline + newline + 
                   "\u6982\u8ff0" + newline + newline +
                   "\u5229\u7528 Java Plug-in \u63a7\u5236\u9762\u677f\u53ef\u4ee5\u66f4\u6539 Java Plug-in \u542f\u52a8\u65f6\u4f7f\u7528\u7684\u7f3a\u7701\u8bbe\u7f6e\u3002" + newline +
                   "\u6240\u6709\u5728 Java Plug-in \u7684\u6d3b\u52a8\u5b9e\u4f8b\u4e2d\u8fd0\u884c\u7684\u5c0f\u5e94\u7528\u7a0b\u5e8f\u90fd\u5c06\u4f7f\u7528\u8fd9\u4e9b\u8bbe\u7f6e\u3002" + newline +
                   "\u53ef\u4ee5\u5728\u4ee5\u4e0b\u94fe\u63a5\uff08URL \u53ef\u80fd\u968f\u65f6\u66f4\u6539\uff09\u5904\u627e\u5230\u672c\u6587\u6863\u4e2d\u63d0\u5230\u7684 Java Plug-in Developer Guide\uff1a" + newline +
                   "http://java.sun.com/j2se/1.4/docs/guide/plugin/developer_guide/contents.html" + newline + newline +
                   "\u4fdd\u5b58\u9009\u9879" + newline + newline +
                   "\u5b8c\u6210\u5bf9\u63a7\u5236\u9762\u677f\u9009\u9879\u7684\u66f4\u6539\u540e\uff0c\u5355\u51fb\u201c\u5e94\u7528\u201d\u4fdd\u5b58\u66f4\u6539\u3002" + newline +
                   "\u5355\u51fb\u201c\u91cd\u7f6e\u201d\u53d6\u6d88\u66f4\u6539\u5e76\u91cd\u8f7d\u4e0a\u4e00\u6b21\u8f93\u5165\u5e76\u5e94\u7528\u7684\u503c\u3002" + newline +
                   "\u8bf7\u6ce8\u610f\uff0c\u8fd9\u4e00\u7ec4\u503c\u4e0e\u5b89\u88c5 Java Plug-in \u65f6\u6700\u521d\u4e3a\u5176\u8bbe\u7f6e\u7684\u90a3\u4e00\u7ec4\u7f3a\u7701\u503c\u4e0d\u540c\u3002" + newline + newline +
                   "\u8bbe\u7f6e\u63a7\u5236\u9762\u677f\u9009\u9879" + newline + newline +
                   "\u53ef\u4ee5\u901a\u8fc7\u516d\u4e2a\u9762\u677f\u8bbe\u7f6e Java Plug-in \u63a7\u5236\u9762\u677f\u4e2d\u7684\u5404\u4e2a\u9009\u9879\u3002" + newline +
                   "\u8fd9\u4e9b\u9762\u677f\u88ab\u6807\u8bb0\u4e3a\uff1a" + newline +
                   "      \u57fa\u672c" + newline + 
                   "      \u9ad8\u7ea7" + newline + 
                   "      \u6d4f\u89c8\u5668" + newline + 
                   "      \u4ee3\u7406" + newline + 
                   "      \u9ad8\u901f\u7f13\u5b58" + newline + 
                   "      \u8bc1\u4e66" + newline + 
                   "      \u66f4\u65b0" + newline + newline +
                   "\u4e0b\u6587\u4e2d\u5bf9\u8fd9\u4e9b\u9762\u677f\u8fdb\u884c\u4e86\u5206\u8ff0\u3002" + newline + newline + newline +
                   " \u57fa\u672c" + newline +
                   "\u663e\u793a Java \u63a7\u5236\u53f0" + newline + newline + 
                   "      \u8fd0\u884c\u5c0f\u5e94\u7528\u7a0b\u5e8f\u65f6\u663e\u793a Java \u63a7\u5236\u53f0\u3002\u8be5\u63a7\u5236\u53f0\u663e\u793a System.out \u548c System.err \u6253\u5370\u7684\u6d88\u606f\u3002" + newline +
                   "      \u8fd9\u5bf9\u4e8e\u6392\u9664\u6545\u969c\u5f88\u6709\u7528\u5904\u3002" + newline + newline +
                   "\u9690\u85cf\u63a7\u5236\u53f0" + newline + newline + 
                   "      Java \u63a7\u5236\u53f0\u5728\u8fd0\u884c\u4e2d\uff0c\u4f46\u5904\u4e8e\u9690\u85cf\u72b6\u6001\u3002\u6b64\u4e3a\u7f3a\u7701\u8bbe\u7f6e\uff08\u9009\u4e2d\u72b6\u6001\uff09\u3002" + newline + newline +
                   "\u4e0d\u542f\u52a8\u63a7\u5236\u53f0" + newline + newline + 
                   "      \u4e0d\u4f1a\u542f\u52a8 Java \u63a7\u5236\u53f0\u3002" + newline + newline + 
                   "\u663e\u793a\u201c\u5f02\u5e38\u201d\u5bf9\u8bdd\u6846" + newline + newline + 
                   "      \u51fa\u73b0\u5f02\u5e38\u65f6\u663e\u793a\u201c\u5f02\u5e38\u201d\u5bf9\u8bdd\u6846\u3002\u7f3a\u7701\u503c\u4e3a\u4e0d\u663e\u793a\u201c\u5f02\u5e38\u201d\u5bf9\u8bdd\u6846\uff08\u672a\u9009\u4e2d\u72b6\u6001\uff09\u3002" + newline + newline +
                   "\u5728\u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u663e\u793a Java\uff08\u4ec5\u9002\u7528\u4e8e Windows\uff09" + newline + newline + 
                   "      \u5982\u679c\u542f\u7528\u6b64\u9009\u9879\uff0c\u5219\u5728\u542f\u52a8 Java Plug-in \u65f6\uff0c\u4f1a\u5728\u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u663e\u793a Java \u5496\u5561\u676f\u5fbd\u6807\uff1b" + newline +
                   "      \u5728\u5173\u95ed Java Plug-in \u65f6\uff0c\u4f1a\u4ece\u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u5220\u9664\u8be5\u5fbd\u6807\u3002" + newline +
                   "      Java \u5496\u5561\u676f\u5fbd\u6807\u53ef\u4ee5\u544a\u8bc9\u7528\u6237 Java VM \u6b63\u5728\u8fd0\u884c\uff0c\u63d0\u4f9b\u6709\u5173" + newline +
                   "      Java \u7248\u672c\u7684\u4fe1\u606f\uff0c\u5e76\u63d0\u4f9b\u5bf9 Java \u63a7\u5236\u53f0\u7684\u63a7\u5236\u3002" + newline +
                   "      \u6b64\u9009\u9879\u7f3a\u7701\u88ab\u542f\u7528\uff08\u9009\u4e2d\u72b6\u6001\uff09\u3002" + newline + newline +
                   "      Java \u7cfb\u7edf\u56fe\u6807\u680f\u529f\u80fd\uff1a" + newline + newline + 
                   "      \u5f53\u9f20\u6807\u6307\u5411 Java \u5496\u5561\u676f\u5fbd\u6807\u65f6\uff0c\u4f1a\u663e\u793a\u6587\u5b57\u201cJava\u201d\u3002" + newline + newline +

                   "      \u5355\u51fb Java \u7cfb\u7edf\u56fe\u6807\u680f\u56fe\u6807\u65f6\uff0c\u5c06\u663e\u793a\u201cJava \u63a7\u5236\u53f0\u201d\u7a97\u53e3\u3002" + newline + newline + 
                   "      \u53f3\u51fb Java \u7cfb\u7edf\u56fe\u6807\u680f\u56fe\u6807\u65f6\uff0c\u5c06\u663e\u793a\u5177\u6709\u4ee5\u4e0b\u83dc\u5355\u9879\u7684\u5f39\u51fa\u83dc\u5355\uff1a" + newline + newline +
                   "            \u6253\u5f00/\u5173\u95ed\u63a7\u5236\u53f0" + newline + 
                   "            \u5173\u4e8e Java" + newline + 
                   "            \u7981\u7528" + newline + 
                   "            \u9000\u51fa" + newline + newline + 
                   "      \u201c\u6253\u5f00/\u5173\u95ed\u63a7\u5236\u53f0\u201d\u7528\u4e8e\u6253\u5f00/\u5173\u95ed\u201cJava \u63a7\u5236\u53f0\u201d\u7a97\u53e3\u3002\u5982\u679c\u672a\u663e\u793a\u201cJava \u63a7\u5236\u53f0\u201d\uff0c\u5219\u6b64\u83dc\u5355\u9879\u4e3a" + newline +
                   "      \u201c\u6253\u5f00\u63a7\u5236\u53f0\u201d\uff1b\u800c\u5982\u679c\u5df2\u663e\u793a\u201cJava \u63a7\u5236\u53f0\u201d\uff0c\u5219\u6b64\u83dc\u5355\u9879\u4e3a\u201c\u5173\u95ed\u63a7\u5236\u53f0\u201d\u3002" + newline + newline +
                   "      \u201c\u5173\u4e8e Java\u201d\u5c06\u663e\u793a Java 2 Standard Edition \u7684\u201c\u5173\u4e8e\u201d\u5bf9\u8bdd\u6846\u3002" + newline + newline +
                   "      \u201c\u7981\u7528\u201d\u7528\u4e8e\u4ece\u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u5220\u9664 Java \u56fe\u6807\uff0c\u5e76\u5728\u5f53\u524d\u548c\u5c06\u6765\u7684\u4f1a\u8bdd\u4e2d\u7981\u7528 Java \u56fe\u6807\u3002\u5f53\u542f\u52a8 Java Plug-in \u65f6\uff0c" + newline +
                   "      \u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u4e0d\u4f1a\u663e\u793a Java \u56fe\u6807\u3002" + newline +
                   "      \u8981\u4e86\u89e3\u5728\u7981\u7528 Java \u56fe\u6807\u4e4b\u540e\u5982\u4f55\u518d\u5728\u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u663e\u793a\u5b83\uff0c\u8bf7\u53c2\u89c1\u4e0b\u9762\u7684\u201c\u6ce8\u201d\u3002" + newline + newline +
                   "      \u201c\u9000\u51fa\u201d\u7528\u4e8e\u5728\u5f53\u524d\u4f1a\u8bdd\u4e2d\u4ece\u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u5220\u9664 Java \u56fe\u6807\u3002\u5f53\u542f\u52a8 Java Plug-in \u65f6\uff0c" + newline +
                   "      \u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u4f1a\u518d\u6b21\u663e\u793a Java \u56fe\u6807\u3002" + newline + newline + newline + 
                   "                \u6ce8" + newline + newline +
                   "                1. \u5982\u679c\u9009\u4e2d\u4e86\u201c\u5728\u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u663e\u793a Java\u201d\uff0c\u90a3\u4e48\u5373\u4f7f\u9009\u4e2d\u4e86\u201c\u4e0d\u542f\u52a8\u63a7\u5236\u53f0\u201d\uff0c\u4e5f\u4ecd\u7136\u4f1a\u5728\u7cfb\u7edf\u56fe\u6807\u680f\u4e2d" + newline +
                   "                \u663e\u793a Java \u56fe\u6807\u3002" + newline + newline +
                   "                2. \u8981\u5728\u7981\u7528 Java \u56fe\u6807\u540e\u518d\u6b21\u542f\u7528\u5b83\uff0c\u8bf7\u542f\u52a8 Java Plug-in \u63a7\u5236\u9762\u677f\uff0c\u9009\u4e2d\u201c\u5728\u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u663e\u793a Java\u201d\uff0c" + newline +
                   "                \u7136\u540e\u6309\u201c\u5e94\u7528\u201d\u3002" + newline + newline +
                   "                3. \u5982\u679c\u6709\u5176\u5b83 Java VM \u6b63\u5728\u8fd0\u884c\uff0c\u5e76\u4e14\u7cfb\u7edf\u56fe\u6807\u680f\u4e2d\u5df2\u7ecf\u6dfb\u52a0\u4e86\u5176\u5b83 Java \u56fe\u6807\uff0c\u5219\u5728 Java Plug-in \u63a7\u5236\u9762\u677f\u4e2d" + newline +
                   "                \u66f4\u6539\u8bbe\u7f6e\u4e0d\u4f1a\u5f71\u54cd\u8fd9\u4e9b\u56fe\u6807\u3002" + newline +
                   "                \u8be5\u8bbe\u7f6e\u53ea\u4f1a\u5f71\u54cd\u4ee5\u540e\u542f\u52a8 Java VM \u65f6 Java \u56fe\u6807\u7684\u8868\u73b0\u3002" + newline + newline + newline +
                   " \u9ad8\u7ea7" + newline + newline + newline +
                   "Java Run Time Environment" + newline + newline +
                   "      \u542f\u7528 Java Plug-in\uff0c\u4ee5\u5728\u5b89\u88c5\u5728\u673a\u5668\u4e2d\u7684\u4efb\u4f55 Java 2 JRE \u6216 SDK\uff08Standard Edition v 1.3 \u6216 1.4\uff09\u73af\u5883\u4e2d\u8fd0\u884c\u3002" + newline +
                   "      Java Plug-in 1.3/1.4 \u968f\u7f3a\u7701 JRE \u4e00\u5e76\u63d0\u4f9b\u3002" + newline +
                   "      \u4f46\u53ef\u4ee5\u91cd\u5199\u7f3a\u7701 JRE \u5e76\u4f7f\u7528\u8f83\u65e7\u6216\u8f83\u65b0\u7684\u7248\u672c\u3002\u63a7\u5236\u9762\u677f\u4f1a\u81ea\u52a8\u68c0\u6d4b" + newline +
                   "      \u5b89\u88c5\u5728\u673a\u5668\u4e0a\u7684\u6240\u6709\u7248\u672c\u7684 Java 2 SDK \u6216 JRE\u3002\u5217\u8868\u6846\u4e2d\u4f1a\u663e\u793a\u53ef\u4ee5\u4f7f\u7528\u7684" + newline +
                   "      \u6240\u6709\u7248\u672c\u3002" + newline +
                   "      \u5217\u8868\u4e2d\u7684\u7b2c\u4e00\u9879\u59cb\u7ec8\u4e3a\u7f3a\u7701 JRE\uff0c\u6700\u540e\u4e00\u9879\u59cb\u7ec8\u4e3a\u201c\u5176\u5b83\u201d\u3002\u5982\u679c\u9009\u62e9\u201c\u5176\u5b83\u201d\uff0c\u5219\u5fc5\u987b" + newline +
                   "      \u6307\u5b9a\u5230 Java 2 JRE \u6216 SDK\uff08Standard Edition v 1.3/1.4.\uff09\u7684\u8def\u5f84\u3002" + newline + newline + 
                   "                \u6ce8" + newline + newline +
                   "                \u53ea\u6709\u9ad8\u7ea7\u7528\u6237\u624d\u53ef\u66f4\u6539\u6b64\u9009\u9879\u3002\u5efa\u8bae\u4e0d\u8981\u66f4\u6539\u7f3a\u7701 JRE\u3002" + newline + newline + newline +
                   "Java \u8fd0\u884c\u65f6\u53c2\u6570" + newline + newline + 
                   "      \u901a\u8fc7\u6307\u5b9a\u81ea\u5b9a\u4e49\u9009\u9879\u91cd\u5199 Java Plug-in \u7f3a\u7701\u542f\u52a8\u53c2\u6570\u3002\u8bed\u6cd5\u4e0e java \u547d\u4ee4\u884c\u8c03\u7528\u7684" + newline +
                   "      \u53c2\u6570\u6240\u7528\u7684\u8bed\u6cd5\u76f8\u540c\u3002\u6709\u5173\u547d\u4ee4\u884c\u9009\u9879\u7684\u5b8c\u6574\u5217\u8868\uff0c\u8bf7\u53c2\u9605\u4e0b\u65b9\u94fe\u63a5\u5904\u7684" + newline +
                   "      Java 2 Standard Edition (J2SE) \u6587\u6863\uff0c" + newline +
                   "      \u8be5 URL \u53ef\u80fd\u968f\u65f6\u66f4\u6539\uff1a" + newline + newline +
                   "            http://java.sun.com/j2se/1.4/docs/tooldocs/<platform>/java.html" + newline + newline + 
                   "            \u5176\u4e2d <platform> \u662f\u4e00\u79cd\u64cd\u4f5c\u7cfb\u7edf\uff1asolaris\u3001linux\u3001win32\u3002" + newline + newline + 
                   "      \u4ee5\u4e0b\u662f Java \u8fd0\u884c\u65f6\u53c2\u6570\u7684\u4e00\u4e9b\u793a\u4f8b\u3002" + newline + newline +
                   "      \u542f\u7528\u548c\u7981\u7528\u65ad\u8a00\u652f\u6301" + newline + newline +
                   "            \u8981\u542f\u7528\u65ad\u8a00\u652f\u6301\uff0c\u5fc5\u987b\u5728\u201cJava \u8fd0\u884c\u65f6\u53c2\u6570\u201d\u4e2d\u6307\u5b9a\u4ee5\u4e0b\u7cfb\u7edf\u5c5e\u6027\uff1a" + newline + newline +
                   "                  -D[ enableassertions | ea ][:<package name>\"...\" | : <class name> ]" + newline + newline +
                   "            \u8981\u5728 Java Plug-in \u4e2d\u7981\u7528\u65ad\u8a00\uff0c\u8bf7\u5728\u201cJava \u8fd0\u884c\u65f6\u53c2\u6570\u201d\u4e2d\u6307\u5b9a\u4ee5\u4e0b\u5c5e\u6027\uff1a" + newline + newline + 
                   "                  -D[ disableassertions | da ][:<package name>\"...\" | : <class name> ]" + newline + newline + 
                   "            \u6709\u5173\u542f\u7528/\u7981\u7528\u65ad\u8a00\u7684\u66f4\u8be6\u7ec6\u7684\u4fe1\u606f\uff0c\u8bf7\u53c2\u9605\u201c\u65ad\u8a00\u529f\u80fd\u201d\uff1a" + newline +
                   "            http://java.sun.com/j2se/1.4/docs/guide/lang/assert.html\uff08URL \u53ef\u80fd\u968f\u65f6\u66f4\u6539\uff09\u3002" + newline + newline +
                   "            \u7f3a\u7701\u60c5\u51b5\u4e0b\uff0c\u65ad\u8a00\u5728 Java Plug-in \u4ee3\u7801\u4e2d\u5904\u4e8e\u7981\u7528\u72b6\u6001\u3002\u56e0\u4e3a\u65ad\u8a00\u7684\u5f71\u54cd\u662f\u5728 Java Plug-in" + newline +
                   "            \u542f\u52a8\u8fc7\u7a0b\u4e2d\u786e\u5b9a\u7684\uff0c\u6240\u4ee5\u66f4\u6539\u201cJava Plug-in \u63a7\u5236\u9762\u677f\u201d\u4e2d\u7684\u65ad\u8a00\u8bbe\u7f6e\u540e\uff0c\u7cfb\u7edf\u4f1a\u8981\u6c42" + newline +
                   "            \u91cd\u65b0\u542f\u52a8\u6d4f\u89c8\u5668\uff0c\u4ee5\u4f7f\u65b0\u8bbe\u7f6e\u751f\u6548\u3002" + newline + newline + 
                   "            \u7531\u4e8e Java Plug-in \u4e2d\u7684 Java \u4ee3\u7801\u4e5f\u6709\u5185\u90e8\u65ad\u8a00\uff0c\u53ef\u4ee5\u901a\u8fc7\u8bbe\u7f6e\u4ee5\u4e0b\u5c5e\u6027\u542f\u7528 " + newline +
                   "            Java Plug-in \u4ee3\u7801\u4e2d\u7684\u65ad\u8a00\uff1a" + newline + newline +
                   "                  -D[ enableassertions | ea ]:sun.plugin" + newline + newline + 
                   "      \u8ddf\u8e2a\u548c\u8bb0\u5f55\u652f\u6301" + newline + newline +
                   "            \u8ddf\u8e2a\u662f\u4e00\u79cd\u529f\u80fd\uff0c\u7528\u4e8e\u5c06\u201cJava \u63a7\u5236\u53f0\u201d\u4e2d\u7684\u4efb\u4f55\u8f93\u51fa\u91cd\u5b9a\u5411\u5230\u8ddf\u8e2a\u6587\u4ef6 (.plugin<version>.trace)\u3002" + newline + newline +
                   "                  -Djavaplugin.trace=true" + newline +
                   "                  -Djavaplugin.trace.option=basic|net|security|ext|liveconnect" + newline + newline +
                   "            \u5982\u679c\u4e0d\u60f3\u4f7f\u7528\u7f3a\u7701\u7684\u8ddf\u8e2a\u6587\u4ef6\u540d\uff1a" + newline + newline +
                   "                  -Djavaplugin.trace.filename=<tracefilename>" + newline + newline + 
                   "            \u4e0e\u8ddf\u8e2a\u76f8\u4f3c\uff0c\u8bb0\u5f55\u4e5f\u662f\u4e00\u79cd\u529f\u80fd\uff0c\u5b83\u5229\u7528\u201cJava \u8bb0\u5f55 API\u201d\u5c06\u201cJava \u63a7\u5236\u53f0\u201d\u4e2d\u7684\u4efb\u4f55\u8f93\u51fa\u91cd\u5b9a\u5411\u5230\u65e5\u5fd7\u6587\u4ef6 (.plugin<version>.log)" + newline +
                   "            \u3002" + newline +
                   "            \u901a\u8fc7\u542f\u7528\u5c5e\u6027 javaplugin.logging \u53ef\u4ee5\u5f00\u542f\u8bb0\u5f55\u529f\u80fd\u3002" + newline + newline +
                   "                  -Djavaplugin.logging=true" + newline + newline +
                   "            \u5982\u679c\u4e0d\u60f3\u4f7f\u7528\u7f3a\u7701\u7684\u65e5\u5fd7\u6587\u4ef6\u540d\uff0c\u8bf7\u8f93\u5165\uff1a" + newline + newline +
                   "                  -Djavaplugin.log.filename=<logfilename>" + newline + newline + 
                   "            \u6b64\u5916\uff0c\u5982\u679c\u4e0d\u60f3\u91cd\u5199\u6bcf\u4e2a\u4f1a\u8bdd\u7684\u8ddf\u8e2a\u6587\u4ef6\u548c\u65e5\u5fd7\u6587\u4ef6\uff0c\u5219\u53ef\u8bbe\u7f6e\u4ee5\u4e0b\u5c5e\u6027\uff1a" + newline + newline + 
                   "                  -Djavaplugin.outputfiles.overwrite=false." + newline + newline + 
                   "            \u5982\u679c\u8be5\u5c5e\u6027\u8bbe\u7f6e\u4e3a false\uff0c\u5219\u6bcf\u4e2a\u4f1a\u8bdd\u7684\u8ddf\u8e2a\u6587\u4ef6\u540d\u548c\u65e5\u5fd7\u6587\u4ef6\u540d\u90fd\u5c06\u552f\u4e00\u3002\u5982\u679c\u4f7f\u7528\u7684\u662f" + newline +
                   "            \u7f3a\u7701\u7684\u8ddf\u8e2a\u6587\u4ef6\u540d\u548c\u65e5\u5fd7\u6587\u4ef6\u540d\uff0c\u5219\u8fd9\u4e9b\u6587\u4ef6\u5c06\u6309\u5982\u4e0b\u65b9\u5f0f\u547d\u540d" + newline + newline +
                   "                  .plugin<username><date hash code>.trace" + newline +
                   "                  .plugin<username><date hash code>.log" + newline + newline +
                   "            \u901a\u8fc7\u63a7\u5236\u9762\u677f\u8fdb\u884c\u7684\u8ddf\u8e2a\u548c\u8bb0\u5f55\u8bbe\u7f6e\u5c06\u5728\u542f\u52a8 Plug-in \u65f6\u751f\u6548\uff0c\u4f46\u5728" + newline +
                   "            Plug-in \u8fd0\u884c\u65f6\u901a\u8fc7\u63a7\u5236\u9762\u677f\u6240\u505a\u7684\u66f4\u6539\u91cd\u65b0\u542f\u52a8\u540e\u624d\u4f1a\u751f\u6548\u3002" + newline + newline + 
                   "            \u6709\u5173\u8ddf\u8e2a\u548c\u8bb0\u5f55\u7684\u8be6\u7ec6\u4fe1\u606f\uff0c\u8bf7\u53c2\u9605 Java Plug-in Developer Guide \u4e2d\u7684\u201c\u8ddf\u8e2a\u548c\u8bb0\u5f55\u201d\u90e8\u5206\u3002" + newline + newline +
                   "      \u8c03\u8bd5 Java Plug-in \u4e2d\u7684\u5c0f\u5e94\u7528\u7a0b\u5e8f" + newline + newline +
                   "            \u8c03\u8bd5 Java Plug-in \u4e2d\u7684\u5c0f\u5e94\u7528\u7a0b\u5e8f\u65f6\u4f1a\u4f7f\u7528\u4ee5\u4e0b\u9009\u9879\u3002" + newline +
                   "            \u6709\u5173\u672c\u4e3b\u9898\u7684\u8be6\u7ec6\u4fe1\u606f\uff0c\u8bf7\u53c2\u9605 Java Plug-in Developer Guide \u4e2d\u7684\u201c\u8c03\u8bd5\u652f\u6301\u201d\u90e8\u5206\u3002" + newline + newline +
                   "                  -Djava.compiler=NONE" + newline +
                   "                  -Xnoagent" + newline +
                   "                  -Xdebug" + newline + 
                   "                  -Xrunjdwp:transport=dt_shmem,address=<connect-address>,server=y,suspend=n" + newline + newline +
                   "            <connect-address> \u53ef\u4ee5\u662f Java Debugger (jdb) \u540e\u6765\u7528\u4e8e\u8fde\u63a5\u5230 JVM \u7684\u4efb\u4f55\u5b57\u7b26\u4e32" + newline +
                   "            \uff08\u4f8b\u5982\uff1a2502\uff09\u3002" + newline + newline + 
                   "      \u7f3a\u7701\u8fde\u63a5\u8d85\u65f6" + newline + newline +
                   "            \u5982\u679c\u5c0f\u5e94\u7528\u7a0b\u5e8f\u4e0e\u670d\u52a1\u5668\u8fdb\u884c\u8fde\u63a5\u65f6\u670d\u52a1\u5668\u672a\u80fd\u6b63\u5e38\u54cd\u5e94\uff0c\u8be5\u5c0f\u5e94\u7528\u7a0b\u5e8f\u53ef\u80fd" + newline +
                   "            \u4f1a\u770b\u4f3c\u5904\u4e8e\u6302\u8d77\u72b6\u6001\uff0c\u8fd8\u53ef\u80fd\u4f1a\u4f7f\u6d4f\u89c8\u5668\u5904\u4e8e\u6302\u8d77\u72b6\u6001\uff0c\u51fa\u73b0\u8fd9\u79cd\u60c5\u51b5\u7684\u539f\u56e0\u5728\u4e8e\u6ca1\u6709\u8bbe\u7f6e\u7f51\u7edc\u8fde\u63a5\u8d85\u65f6" + newline +
                   "            \uff08\u7f3a\u7701\u60c5\u51b5\u4e0b\u8bbe\u7f6e\u4e3a -1\uff0c\u610f\u5373\u6ca1\u6709\u8d85\u65f6\u8bbe\u7f6e\uff09\u3002" + newline + newline +
                   "            \u4e3a\u907f\u514d\u51fa\u73b0\u6b64\u95ee\u9898\uff0cJava Plug-in \u5df2\u4e3a\u6240\u6709 HTTP \u8fde\u63a5\u6dfb\u52a0\u4e86\u4e00\u4e2a\u7f3a\u7701\u7f51\u7edc\u8d85\u65f6\u503c\uff082 \u5206\u949f\uff09\u3002" + newline +
                   "            \u53ef\u4ee5\u5728\u201cJava \u8fd0\u884c\u65f6\u53c2\u6570\u201d\u4e2d\u91cd\u5199\u6b64\u8bbe\u7f6e\uff1a" + newline + newline +
                   "                  -Dsun.net.client.defaultConnectTimeout=value in milliseconds" + newline + newline +
                   "            \u53e6\u4e00\u4e2a\u53ef\u4ee5\u8bbe\u7f6e\u7684\u8054\u7f51\u5c5e\u6027\u4e3a sun.net.client.defaultReadTimeout\u3002" + newline + newline +
                   "                  -Dsun.net.client.defaultReadTimeout=\u4ee5\u6beb\u79d2\u4e3a\u5355\u4f4d\u7684\u503c" + newline + newline +
                   "                  \u6ce8" + newline + newline +
                   "                  \u7f3a\u7701\u60c5\u51b5\u4e0b\uff0cJava Plug-in \u4e0d\u8bbe\u7f6e sun.net.client.defaultReadTimeout\u3002\u5982\u679c\u8981\u8bbe\u7f6e\u8be5\u5c5e\u6027\uff0c\u8bf7\u901a\u8fc7" + newline +
                   "                  \u4e0a\u6587\u6240\u793a\u7684\u201cJava \u8fd0\u884c\u65f6\u53c2\u6570\u201d\u8fdb\u884c\u8bbe\u7f6e\u3002" + newline + newline + newline +
                   "            \u8054\u7f51\u5c5e\u6027\u8bf4\u660e\uff1a" + newline + newline +
                   "            sun.net.client.defaultConnectTimeout" + newline +
                   "            sun.net.client.defaultReadTimeout" + newline + newline +
                   "                  \u8fd9\u4e9b\u5c5e\u6027\u5206\u522b\u6307\u5b9a java.net.URLConnection \u4f7f\u7528\u7684\u534f\u8bae\u5904\u7406\u7a0b\u5e8f\u7684\u7f3a\u7701\u8fde\u63a5\u548c" + newline +
                   "                  \u8bfb\u53d6\u8d85\u65f6\u503c\u3002\u8be5\u534f\u8bae\u5904\u7406\u7a0b\u5e8f\u8bbe\u7f6e\u7684\u7f3a\u7701\u503c\u4e3a -1\uff0c\u610f\u5373" + newline +
                   "                  \u65e0\u8d85\u65f6\u8bbe\u7f6e\u3002" + newline + newline +
                   "                  sun.net.client.defaultConnectTimeout \u6307\u5b9a\u5efa\u7acb\u5230\u4e3b\u673a\u7684\u8fde\u63a5\u7684\u8d85\u65f6\uff08\u4ee5\u6beb\u79d2\u4e3a\u5355\u4f4d\uff09\u3002" + newline +
                   "                  \u4f8b\u5982\uff0c\u5bf9\u4e8e http \u8fde\u63a5\uff0c\u5b83\u662f\u5efa\u7acb\u5230 http \u670d\u52a1\u5668\u7684\u8fde\u63a5\u7684\u8d85\u65f6\u3002" + newline +
                   "                  \u5bf9\u4e8e ftp \u8fde\u63a5\uff0c\u5b83\u662f\u5efa\u7acb\u5230 ftp \u670d\u52a1\u5668\u7684\u8fde\u63a5\u7684\u8d85\u65f6\u3002" + newline + newline +
                   "                  sun.net.client.defaultReadTimeout \u6307\u5b9a\u5728\u5df2\u5efa\u7acb\u4e86\u5230\u8d44\u6e90\u7684\u8fde\u63a5\u7684\u60c5\u51b5\u4e0b\u4ece\u8f93\u5165\u6d41\u4e2d\u8bfb\u53d6\u6570\u636e\u65f6\u7684" + newline +
                   "                  \u8d85\u65f6\uff08\u4ee5\u6beb\u79d2\u4e3a\u5355\u4f4d\uff09\u3002" + newline + newline + 
                   "            \u6709\u5173\u8fd9\u4e9b\u8054\u7f51\u5c5e\u6027\u7684\u6b63\u5f0f\u8bf4\u660e\uff0c" + newline +
                   "            \u8bf7\u53c2\u9605 http://java.sun.com/j2se/1.4/docs/guide/net/properties.html\u3002" + newline + newline + newline +
                   " \u6d4f\u89c8\u5668" + newline + newline + newline +
                   "\u6b64\u9762\u677f\u53ea\u4e0e Microsoft Windows \u5b89\u88c5\u6709\u5173\u3002\u5b83\u4e0d\u51fa\u73b0\u5728\u5176\u5b83\u5b89\u88c5\u4e2d\u3002\u9009\u4e2d\u60f3\u8ba9 " + newline +
"Java Plug-in\uff08\u800c\u4e0d\u662f\u6d4f\u89c8\u5668\u7684\u5185\u90e8 JVM\uff09\u4f5c\u4e3a\u7f3a\u7701 Java \u8fd0\u884c\u65f6\u7684\u4efb\u4e00\u6d4f\u89c8\u5668\u3002" + newline +
"\u8fd9\u6837\u505a\u7684\u76ee\u7684\u662f\u901a\u8fc7 Java Plug-in \u5728 Internet Explorer \u548c Netscape 6 \u4e2d\u542f\u7528 APPLET \u6807\u5fd7\u652f\u6301\u3002" + newline + newline + newline +
                   " \u4ee3\u7406" + newline + newline +newline +
                   "\u5229\u7528\u201c\u4ee3\u7406\u201d\u9762\u677f\u53ef\u4ee5\u4f7f\u7528\u6d4f\u89c8\u5668\u7684\u7f3a\u7701\u8bbe\u7f6e\u6216\u91cd\u5199\u4e0d\u540c\u534f\u8bae\u7684\u4ee3\u7406\u5730\u5740\u548c\u7aef\u53e3\u3002" + newline + newline + 
                   "\u4f7f\u7528\u6d4f\u89c8\u5668\u8bbe\u7f6e" + newline + newline + 
                   "      \u9009\u4e2d\u6b64\u9009\u9879\u4ee5\u4f7f\u7528\u6d4f\u89c8\u5668\u7684\u7f3a\u7701\u4ee3\u7406\u8bbe\u7f6e\u3002\u6b64\u4e3a\u7f3a\u7701\u8bbe\u7f6e\uff08\u9009\u4e2d\u72b6\u6001\uff09\u3002" + newline + newline +
                   "\u4ee3\u7406\u4fe1\u606f\u8868" + newline + newline + 
                   "      \u53d6\u6d88\u9009\u4e2d\u201c\u4f7f\u7528\u6d4f\u89c8\u5668\u8bbe\u7f6e\u201d\u590d\u9009\u6846\uff0c\u7136\u540e\u586b\u5199\u8be5\u590d\u9009\u6846\u4e0b\u65b9\u7684\u4ee3\u7406\u4fe1\u606f\u8868\uff0c" + newline +
                   "      \u5c31\u53ef\u91cd\u5199\u7f3a\u7701\u8bbe\u7f6e\u3002\u53ef\u4ee5\u8f93\u5165\u6240\u652f\u6301\u7684\u6bcf\u4e2a\u534f\u8bae\u7684\u4ee3\u7406\u5730\u5740\u548c\u7aef\u53e3\uff1a" + newline +
                   "      HTTP\u3001Secure (HTTPS)\u3001FTP\u3001Gopher \u548c Socks\u3002" + newline + newline + 
                   "\u65e0\u4ee3\u7406\u4e3b\u673a" + newline + newline + 
                   "      \u5b83\u662f\u4e00\u4e2a\u4e3b\u673a\u6216\u4e3b\u673a\u5217\u8868\uff0c\u5728\u8be5\u4e3b\u673a\uff08\u8fd9\u4e9b\u4e3b\u673a\uff09\u4e0a\u4e0d\u4f7f\u7528\u4ee3\u7406\u3002\u201c\u65e0\u4ee3\u7406\u4e3b\u673a\u201d\u901a\u5e38\u7528\u4e8e\u5185\u90e8\u7f51\u73af\u5883\u4e2d\u7684" + newline +
                   "      \u5185\u90e8\u4e3b\u673a\u4e0a\u3002" + newline + newline + 
                   "\u81ea\u52a8\u4ee3\u7406\u914d\u7f6e URL" + newline + 
                   "      \u5b83\u662f\u542b\u6709 FindProxyForURL \u51fd\u6570\u7684 JavaScript \u6587\u4ef6\uff08.js \u6216 .pac \u6269\u5c55\u540d\uff09\u7684 URL\u3002" + newline +
                   "      FindProxyForURL \u5177\u6709\u786e\u5b9a\u662f\u5426\u4f7f\u7528\u4ee3\u7406\u670d\u52a1\u5668\u5904\u7406\u8fde\u63a5\u8bf7\u6c42\u7684\u903b\u8f91\u3002" + newline + newline + 
                   "\u6709\u5173\u4ee3\u7406\u914d\u7f6e\u7684\u5176\u5b83\u4fe1\u606f\uff0c\u8bf7\u53c2\u9605 Java Plug-in Developer Guide \u4e2d\u7684" + newline +
                   "\u201c\u4ee3\u7406\u914d\u7f6e\u201d\u4e00\u7ae0\u3002" + newline + newline + newline +
                   " \u9ad8\u901f\u7f13\u5b58" + newline + newline + newline +
                   "           \u6ce8" + newline + newline +
                   "           \u6b64\u5904\u6240\u6307\u9ad8\u901f\u7f13\u5b58\u662f\u6307\u7c98\u6027\u9ad8\u901f\u7f13\u5b58\uff0c\u5373\u6d4f\u89c8\u5668\u65e0\u6cd5\u91cd\u5199\u7684\u3001\u7531 Java Plug-in \u521b\u5efa\u5e76\u63a7\u5236\u7684" + newline +
                   "           \u78c1\u76d8\u9ad8\u901f\u7f13\u5b58\u3002\u6709\u5173\u8be6\u7ec6\u4fe1\u606f\uff0c\u8bf7\u53c2\u9605 Java Plug-in Developer Guide \u4e2d\u7684\u201c\u5c0f\u5e94\u7528\u7a0b\u5e8f\u9ad8\u901f\u7f13\u5b58\u201d\u90e8\u5206\u3002" + newline + newline + newline +
                   "\u542f\u7528\u9ad8\u901f\u7f13\u5b58" + newline + newline + 
                   "      \u9009\u4e2d\u6b64\u9009\u9879\u4ee5\u542f\u7528\u9ad8\u901f\u7f13\u5b58\u3002\u6b64\u4e3a\u7f3a\u7701\u8bbe\u7f6e\uff08\u9009\u4e2d\u72b6\u6001\uff09\u3002\u542f\u7528\u5c0f\u5e94\u7528\u7a0b\u5e8f\u9ad8\u901f\u7f13\u5b58\u4f1a\u6539\u5584\u7cfb\u7edf\u6027\u80fd\uff0c" + newline +
                   "      \u56e0\u4e3a\u5c06\u5c0f\u5e94\u7528\u7a0b\u5e8f\u5199\u5165\u9ad8\u901f\u7f13\u5b58\u540e\uff0c\u518d\u6b21\u5f15\u7528\u65f6\u5c31\u4e0d\u5fc5\u518d\u884c\u4e0b\u8f7d\u3002" + newline + newline +
                   "      Java Plug-in \u53ef\u5c06\u901a\u8fc7 HTTP/HTTPS \u4e0b\u8f7d\u7684\u4ee5\u4e0b\u7c7b\u578b\u7684\u6587\u4ef6\u5199\u5165\u9ad8\u901f\u7f13\u5b58\uff1a" + newline + newline +
                   "            .jar\uff08jar \u6587\u4ef6\uff09" + newline +
                   "            .zip\uff08zip \u6587\u4ef6\uff09" + newline +
                   "            .class\uff08java \u7c7b\u6587\u4ef6\uff09" + newline +
                   "            .au\uff08\u58f0\u97f3\u6587\u4ef6\uff09" + newline +
                   "            .wav\uff08\u58f0\u97f3\u6587\u4ef6\uff09" + newline +
                   "            .jpg\uff08\u56fe\u50cf\u6587\u4ef6\uff09" + newline +
                   "            .gif\uff08\u56fe\u50cf\u6587\u4ef6\uff09" + newline + newline +
                   "\u5728\u9ad8\u901f\u7f13\u5b58\u4e2d\u67e5\u770b\u6587\u4ef6" + newline + newline + 
                   "      \u6309\u6b64\u9009\u9879\u53ef\u67e5\u770b\u5199\u5165\u9ad8\u901f\u7f13\u5b58\u7684\u6587\u4ef6\u3002\u4f1a\u5f39\u51fa\u53e6\u4e00\u4e2a\u5bf9\u8bdd\u6846\uff08\u201cJava Plug-in \u9ad8\u901f\u7f13\u5b58\u67e5\u770b\u5668\u201d\uff09\uff0c\u5176\u4e2d\u663e\u793a\u6709\u5199\u5165\u9ad8\u901f\u7f13\u5b58\u7684\u6587\u4ef6\u3002" + newline +
                   "      \u201c\u9ad8\u901f\u7f13\u5b58\u67e5\u770b\u5668\u201d\u663e\u793a\u9ad8\u901f\u7f13\u5b58\u4e2d\u7684\u6587\u4ef6\u7684\u4ee5\u4e0b\u4fe1\u606f\uff1a\u201c\u540d\u79f0\u201d\u3001\u201c\u7c7b\u578b\u201d\u3001\u201c\u5927\u5c0f\u201d\u3001\u201c\u5931\u6548\u65e5\u671f\u201d\u3001" + newline +
                   "      \u201c\u4e0a\u6b21\u4fee\u6539\u65e5\u671f\u201d\u3001\u201c\u7248\u672c\u201d\u548c URL\u3002\u5728\u201c\u9ad8\u901f\u7f13\u5b58\u67e5\u770b\u5668\u201d\u4e2d\uff0c\u8fd8\u53ef\u4ee5\u9009\u62e9\u6027\u5730\u5220\u9664\u9ad8\u901f\u7f13\u5b58\u4e2d\u7684\u6587\u4ef6\u3002" + newline +
                   "      \u5b83\u53ef\u5b9e\u73b0\u4e0b\u6587\u4e2d\u6240\u8ff0\u7684\u201c\u6e05\u9664\u9ad8\u901f\u7f13\u5b58\u201d\u9009\u9879\uff08\u5220\u9664\u9ad8\u901f\u7f13\u5b58\u4e2d\u7684\u6240\u6709\u6587\u4ef6\uff09\u7684\u76f8\u540c\u529f\u80fd\u3002" + newline + newline +
                   "\u6e05\u9664\u9ad8\u901f\u7f13\u5b58" + newline + newline + 
                   "      \u6309\u6b64\u9009\u9879\u6e05\u9664\u9ad8\u901f\u7f13\u5b58\u4e2d\u7684\u6240\u6709\u6587\u4ef6\u3002\u5220\u9664\u8fd9\u4e9b\u6587\u4ef6\u524d\uff0c\u7cfb\u7edf\u4f1a\u63d0\u793a\uff08\u786e\u5b9a\u8981\u6e05\u9664\u9ad8\u901f\u7f13\u5b58\u4e2d\u7684\u6240\u6709\u6587\u4ef6\u5417\uff1f\uff09\u3002" + newline + newline + 
                   "\u4f4d\u7f6e" + newline + newline + 
                   "      \u53ef\u4ee5\u4f7f\u7528\u6b64\u9009\u9879\u6307\u5b9a\u9ad8\u901f\u7f13\u5b58\u7684\u4f4d\u7f6e\u3002\u9ad8\u901f\u7f13\u5b58\u7684\u7f3a\u7701\u4f4d\u7f6e\u662f <user home>/.jpi_cache\uff0c\u5176\u4e2d" + newline +
                   "      <user home> \u662f\u7cfb\u7edf\u5c5e\u6027 user.home \u7684\u503c\u3002\u5176\u503c\u7531 OS \u786e\u5b9a\u3002" + newline + newline +
                   "Size" + newline + newline + 
                   "      \u53ef\u4ee5\u9009\u4e2d\u201c\u65e0\u9650\u5236\u201d\u4f7f\u9ad8\u901f\u7f13\u5b58\u7684\u5927\u5c0f\u4e0d\u53d7\u9650\u5236\uff0c\u4e5f\u53ef\u8bbe\u7f6e\u9ad8\u901f\u7f13\u5b58\u7684\u201c\u6700\u5927\u201d\u5927\u5c0f\u3002" + newline +
                   "      \u5982\u679c\u9ad8\u901f\u7f13\u5b58\u7684\u5927\u5c0f\u8d85\u51fa\u6307\u5b9a\u7684\u9650\u5236\uff0c\u5219\u6700\u65e9\u5199\u5165\u9ad8\u901f\u7f13\u5b58\u7684\u6587\u4ef6\u5c06\u88ab\u5220\u9664\uff0c\u76f4\u81f3\u9ad8\u901f" + newline +
                   "      \u7f13\u5b58\u5927\u5c0f\u6062\u590d\u5230\u8be5\u9650\u5236\u5185\u3002" + newline + newline + 
                   "\u538b\u7f29" + newline + newline + 
                   "      \u53ef\u4ee5\u5728\u201c\u65e0\u201d\u548c\u201c\u9ad8\u201d\u4e4b\u95f4\u8bbe\u7f6e JAR \u9ad8\u901f\u7f13\u5b58\u6587\u4ef6\u7684\u538b\u7f29\u7387\u3002\u5c3d\u7ba1\u6307\u5b9a" + newline +
                   "      \u8f83\u9ad8\u7684\u538b\u7f29\u7387\u53ef\u8282\u7ea6\u5185\u5b58\uff0c\u4f46\u7cfb\u7edf\u6027\u80fd\u4ecd\u4f1a\u4e0b\u964d\u3002\u4e0d\u8fdb\u884c\u538b\u7f29\u65f6\u53ef\u83b7\u5f97" + newline +
                   "      \u6700\u4f73\u7cfb\u7edf\u6027\u80fd\u3002" + newline + newline + newline + 
                   " \u8bc1\u4e66" + newline + newline + newline +
                   "\u53ef\u4ee5\u9009\u62e9\u4ee5\u4e0b\u56db\u79cd\u8bc1\u4e66\uff1a" + newline + newline +
                   "      \u7b7e\u7f72\u5c0f\u5e94\u7528\u7a0b\u5e8f" + newline + 
                   "      \u5b89\u5168\u7ad9\u70b9" + newline + 
                   "      \u7b7e\u7f72\u4eba CA" + newline +
                   "      \u5b89\u5168\u7ad9\u70b9 CA" + newline + newline + 
                   "\u7b7e\u7f72\u5c0f\u5e94\u7528\u7a0b\u5e8f" + newline + newline +
                   "      \u6b64\u7c7b\u8bc1\u4e66\u662f\u7528\u6237\u4fe1\u4efb\u7684\u7b7e\u7f72\u7684\u5c0f\u5e94\u7528\u7a0b\u5e8f\u7684\u8bc1\u4e66\u3002\u51fa\u73b0\u5728\u7b7e\u7f72\u7684\u5c0f\u5e94\u7528\u7a0b\u5e8f\u5217\u8868\u4e2d\u7684\u8bc1\u4e66" + newline +
                   "      \u662f\u4ece <user home>/.java \u76ee\u5f55\u4e0b\u7684\u8bc1\u4e66\u6587\u4ef6 jpicerts<version> \u4e2d\u8bfb\u53d6\u7684\u3002" + newline + newline +
                   "\u5b89\u5168\u7ad9\u70b9" + newline + newline +
                   "      \u6b64\u7c7b\u8bc1\u4e66\u662f\u5b89\u5168\u7ad9\u70b9\u7684\u8bc1\u4e66\u3002\u51fa\u73b0\u5728\u5b89\u5168\u7ad9\u70b9\u5217\u8868\u4e2d\u7684\u8bc1\u4e66\u662f\u4ece" + newline +
                   "      <user home>/.java \u76ee\u5f55\u4e0b\u7684\u8bc1\u4e66\u6587\u4ef6 jpihttpscerts<version> \u4e2d\u8bfb\u53d6\u7684\u3002" + newline + newline + 
                   "\u7b7e\u7f72\u4eba CA" + newline + newline +
                   "      \u6b64\u7c7b\u8bc1\u4e66\u662f\u201c\u8bc1\u4e66\u6388\u6743\u201d(CA) \u4e3a\u7b7e\u7f72\u5c0f\u5e94\u7528\u7a0b\u5e8f\u9881\u53d1\u7684\u8bc1\u4e66\u3002\u5b83\u4eec\u662f\u5411\u7b7e\u7f72" + newline +
                   "      \u5c0f\u5e94\u7528\u7a0b\u5e8f\u7684\u7b7e\u7f72\u4eba\u9881\u53d1\u8bc1\u4e66\u7684\u673a\u6784\u3002\u51fa\u73b0\u5728\u201c\u7b7e\u7f72\u4eba CA\u201d\u5217\u8868\u4e2d\u7684\u8bc1\u4e66\u662f" + newline +
                   "      \u4ece <jre>/lib/security \u76ee\u5f55\u4e0b\u7684\u8bc1\u4e66\u6587\u4ef6 cacerts \u4e2d\u8bfb\u53d6\u7684\u3002" + newline + newline +
                   "\u5b89\u5168\u7ad9\u70b9 CA" + newline + newline +
                   "      \u6b64\u7c7b\u8bc1\u4e66\u662f\u201c\u8bc1\u4e66\u6388\u6743\u201d(CA) \u4e3a\u5b89\u5168\u7ad9\u70b9\u9881\u53d1\u7684\u8bc1\u4e66\u3002\u5b83\u4eec\u662f\u4e3a\u5b89\u5168\u7ad9\u70b9\u9881\u53d1" + newline +
                   "      \u8bc1\u4e66\u7684\u673a\u6784\u3002\u51fa\u73b0\u5728\u201c\u5b89\u5168\u7ad9\u70b9 CA\u201d\u5217\u8868\u4e2d\u7684\u8bc1\u4e66\u662f\u4ece <jre>/lib/security " + newline +
                   "      \u76ee\u5f55\u4e0b\u7684\u8bc1\u4e66\u6587\u4ef6 jssecacerts \u4e2d\u8bfb\u53d6\u7684\u3002" + newline + newline +
                   "\u5bf9\u4e8e\u7b7e\u7f72\u5c0f\u5e94\u7528\u7a0b\u5e8f\u548c\u5b89\u5168\u7ad9\u70b9\u8bc1\u4e66\uff0c\u6709\u56db\u4e2a\u9009\u9879\uff1a\u201c\u5bfc\u5165\u201d\u3001\u201c\u5bfc\u51fa\u201d\u3001\u201c\u5220\u9664\u201d\u548c\u201c\u8be6\u7ec6\u4fe1\u606f\u201d\u3002" + newline +
                   "\u7528\u6237\u53ef\u4ee5\u5bfc\u5165\u3001\u5bfc\u51fa\u3001\u5220\u9664\u548c\u67e5\u770b\u8bc1\u4e66\u7684\u8be6\u7ec6\u4fe1\u606f\u3002" + newline + newline + 
                   "\u5bf9\u4e8e\u7b7e\u7f72\u4eba CA \u548c\u5b89\u5168\u7ad9\u70b9 CA\uff0c\u53ea\u6709\u4e00\u4e2a\u9009\u9879\uff1a\u201c\u8be6\u7ec6\u4fe1\u606f\u201d\u3002\u7528\u6237\u53ea\u80fd\u67e5\u770b\u8bc1\u4e66\u7684\u8be6\u7ec6\u4fe1\u606f\u3002" + newline + newline + newline +
                   " \u66f4\u65b0" + newline + newline + newline +
                   "\u6b64\u9762\u677f\u4e2d\u6709\u4e00\u4e2a\u201c\u83b7\u5f97 Java Update\u201d\u6309\u94ae\uff0c\u7528\u6237\u53ef\u4ee5\u5229\u7528\u5b83\u4ece Java Update \u7f51\u7ad9\u4e0a\u83b7\u5f97\u6700\u65b0\u7684 Java Run Time" + newline +
" Environment \u7684\u66f4\u65b0\u3002\u6b64\u9762\u677f\u53ea\u4e0e Microsoft Windows \u5e73\u53f0\u6709\u5173\uff0c\u5176\u5b83\u5e73\u53f0\uff08\u5373 Solaris/Linux\uff09\u4e2d" + newline +
"\u4e0d\u663e\u793a\u8be5\u9762\u677f\u3002"}
};
} 

 

