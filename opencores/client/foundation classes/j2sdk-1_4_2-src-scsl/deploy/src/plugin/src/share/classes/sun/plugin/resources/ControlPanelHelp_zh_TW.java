/*
 * @(#)ControlPanelHelp_zh_TW.java	1.2 03/03/22
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

public class ControlPanelHelp_zh_TW extends ListResourceBundle {

    private static String newline = "\n";

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "help.file", newline +
                   "Java Plug-in \u63a7\u5236\u9762\u677f\u8aaa\u660e" + newline + newline + newline +
                   "\u5728\u8aaa\u660e\u4e2d\u6db5\u84cb\u4e0b\u5217\u7684\u4e3b\u984c\uff1a" + newline +
                   "      \u6982\u89c0" + newline + 
                   "      \u5132\u5b58\u9078\u9805" + newline + 
                   "      \u8a2d\u5b9a\u63a7\u5236\u9762\u677f\u9078\u9805" + newline + 
                   "      \u57fa\u672c" + newline + 
                   "      \u9032\u968e" + newline +
                   "      \u700f\u89bd\u5668" + newline + 
                   "      \u4ee3\u7406" + newline + 
                   "      \u5feb\u53d6" + newline + 
                   "      \u8b49\u66f8" + newline + 
                   "      \u66f4\u65b0" + newline + newline + 
                   "\u6982\u89c0" + newline + newline +
                   "Java Plug-in \u63a7\u5236\u9762\u677f\u53ef\u4ee5\u8b93\u60a8\u8b8a\u66f4 Java Plug-in \u5728\u555f\u52d5\u6642\u6240\u4f7f\u7528\u7684\u9810\u8a2d\u503c\u3002" + newline +
                   "\u6240\u6709\u5728 Java Plug-in \u4f5c\u7528\u4e2d\u5be6\u4f8b\u4e2d\u57f7\u884c\u7684 Applet\uff0c\u90fd\u6703\u4f7f\u7528\u9019\u4e9b\u8a2d\u5b9a\u3002" + newline +
                   "\u672c\u6587\u4ef6\u4e2d\u6240\u63d0\u53ca\u7684 Java Plug-in Developer Guide\uff0c\u53ef\u4ee5\u5728\u4e0b\u5217\u7db2\u7ad9\u4e0a\u627e\u5230\uff08URL \u6703\u96a8\u6642\u8b8a\u66f4\uff09\uff1a" + newline +
                   "http://java.sun.com/j2se/1.4/docs/guide/plugin/developer_guide/contents.html" + newline + newline +
                   "\u5132\u5b58\u9078\u9805" + newline + newline +
                   "\u7576\u60a8\u5b8c\u6210\u300c\u63a7\u5236\u9762\u677f\u300d\u9078\u9805\u7684\u8b8a\u66f4\u4e4b\u5f8c\uff0c\u8acb\u6309\u4e00\u4e0b\u300c\u5957\u7528\u300d\u4f86\u5132\u5b58\u6240\u505a\u7684\u8b8a\u66f4\u3002" + newline +
                   "\u6309\u4e00\u4e0b\u300c\u91cd\u8a2d\u300d\uff0c\u53ef\u4ee5\u53d6\u6d88\u6240\u505a\u7684\u8b8a\u66f4\uff0c\u4e26\u91cd\u65b0\u8f09\u5165\u6700\u5f8c\u8f38\u5165\u4e14\u5957\u7528\u7684\u503c\u3002" + newline +
                   "\u8acb\u6ce8\u610f\uff0c\u6b64\u503c\u8207\u5b89\u88dd Java Plug-in \u4e4b\u5f8c\u6700\u521d\u6240\u8a2d\u5b9a\u7684\u9810\u8a2d\u503c\u4e0d\u540c\u3002" + newline + newline +
                   "\u8a2d\u5b9a\u63a7\u5236\u9762\u677f\u9078\u9805" + newline + newline +
                   "\u5728 Java Plug-in \u63a7\u5236\u9762\u677f\u4e2d\uff0c\u5171\u6709\u516d\u500b\u9762\u677f\u53ef\u8b93\u60a8\u8a2d\u5b9a\u5404\u7a2e\u7684\u9078\u9805\u3002" + newline +
                   "\u9019\u4e9b\u9762\u677f\u7684\u6a19\u7c64\u5982\u4e0b\uff1a" + newline +
                   "      \u57fa\u672c" + newline + 
                   "      \u9032\u968e" + newline + 
                   "      \u700f\u89bd\u5668" + newline + 
                   "      \u4ee3\u7406" + newline + 
                   "      \u5feb\u53d6" + newline + 
                   "      \u8b49\u66f8" + newline + 
                   "      \u66f4\u65b0" + newline + newline +
                   "\u6bcf\u4e00\u9805\u5747\u5728\u4e0b\u9762\u5206\u5225\u8aaa\u660e\u3002" + newline + newline + newline +
                   " \u57fa\u672c" + newline +
                   "\u986f\u793a\u4e3b\u63a7\u53f0" + newline + newline + 
                   "      \u5728\u57f7\u884c Applet \u6642\uff0c\u6703\u986f\u793a Java \u4e3b\u63a7\u53f0\u3002\u5728\u4e3b\u63a7\u53f0\u4e2d\uff0c\u6703\u986f\u793a\u7531 System.out \u53ca System.err \u6240\u5217\u5370\u7684\u8a0a\u606f\u3002" + newline +
                   "      \u5b83\u5728\u554f\u984c\u9664\u932f\u65b9\u9762\u975e\u5e38\u6709\u7528\u3002" + newline + newline +
                   "\u96b1\u85cf\u4e3b\u63a7\u53f0" + newline + newline + 
                   "      Java \u4e3b\u63a7\u53f0\u96d6\u7136\u5728\u57f7\u884c\u4e2d\uff0c\u4f46\u662f\u662f\u96b1\u85cf\u7684\u3002\u6b64\u70ba\u9810\u8a2d\u7684\u8a2d\u5b9a\uff08\u5df2\u52fe\u9078\uff09\u3002" + newline + newline +
                   "\u4e0d\u555f\u52d5\u4e3b\u63a7\u53f0" + newline + newline + 
                   "      Java \u4e3b\u63a7\u53f0\u4e0d\u6703\u555f\u52d5\u3002" + newline + newline + 
                   "\u986f\u793a\u7570\u5e38\u5c0d\u8a71\u65b9\u584a" + newline + newline + 
                   "      \u7576\u767c\u751f\u7570\u5e38\u6642\uff0c\u6703\u986f\u793a\u7570\u5e38\u5c0d\u8a71\u65b9\u584a\u3002\u9810\u8a2d\u503c\u70ba\u4e0d\u986f\u793a\u7570\u5e38\u5c0d\u8a71\u65b9\u584a\uff08\u5df2\u53d6\u6d88\u52fe\u9078\uff09\u3002" + newline + newline +
                   "\u5728\u7cfb\u7d71\u5323\u4e2d\u986f\u793a Java\uff08\u50c5\u9650\u65bc Windows\uff09" + newline + newline + 
                   "      \u82e5\u555f\u7528\u6b64\u9078\u9805\uff0c\u7576\u555f\u52d5 Java Plug-in \u6642\uff0c\u7cfb\u7d71\u5323\u4e2d\u6703\u986f\u793a Java \u5496\u5561\u676f\u6a19\u8a8c\uff0c" + newline +
                   "      \u7576 Java Plug-in \u95dc\u9589\u6642\uff0c\u6b64\u6a19\u8a8c\u6703\u5f9e\u7cfb\u7d71\u5323\u4e2d\u79fb\u9664\u3002" + newline +
                   "      Java \u5496\u5561\u676f\u6a19\u8a8c\u6307\u793a\u4f7f\u7528\u8005 Java VM \u6b63\u5728\u57f7\u884c\uff0c\u4e26\u63d0\u4f9b" + newline +
                   "      \u6709\u95dc Java \u7684\u7248\u6b21\u8cc7\u8a0a\uff0c\u9084\u53ef\u900f\u904e Java \u4e3b\u63a7\u53f0\u57f7\u884c\u63a7\u5236\u3002" + newline +
                   "      \u4f9d\u9810\u8a2d\uff0c\u6703\u555f\u7528\u6b64\u9078\u9805\uff08\u5df2\u52fe\u9078\uff09\u3002" + newline + newline +
                   "      Java \u7cfb\u7d71\u5323\u529f\u80fd\u5982\u4e0b\uff1a" + newline + newline + 
                   "      \u7576\u6ed1\u9f20\u6307\u5411 Java \u5496\u5561\u676f\u6a19\u8a8c\u6642\uff0c\u6703\u986f\u793a\u6587\u5b57\u300cJava\u300d\u3002" + newline + newline +

                   "      \u7576\u7528\u6ed1\u9f20\u5de6\u9375\u6309\u5169\u4e0b Java \u7cfb\u7d71\u5323\u5716\u793a\u6642\uff0c\u5c07\u51fa\u73fe\u300cJava \u4e3b\u63a7\u53f0\u300d\u8996\u7a97\u3002" + newline + newline + 
                   "      \u7576\u7528\u6ed1\u9f20\u53f3\u9375\u6309\u5169\u4e0b Java \u7cfb\u7d71\u5323\u5716\u793a\u6642\uff0c\u5c07\u986f\u793a\u5177\u6709\u5982\u4e0b\u529f\u80fd\u8868\u9805\u76ee\u7684\u5373\u73fe\u5f0f\u529f\u80fd\u8868\uff1a" + newline + newline +
                   "            \u958b\u555f/\u95dc\u9589\u4e3b\u63a7\u53f0" + newline + 
                   "            \u95dc\u65bc Java" + newline + 
                   "            \u505c\u7528" + newline + 
                   "            \u7d50\u675f" + newline + newline + 
                   "      \u300c\u958b\u555f/\u95dc\u9589\u4e3b\u63a7\u53f0\u300d\u6703\u958b\u555f/\u95dc\u9589\u300cJava \u4e3b\u63a7\u53f0\u300d\u8996\u7a97\u3002\u5982\u679c Java \u4e3b\u63a7\u53f0\u70ba\u96b1\u85cf\u72c0\u614b\uff0c\u6b64\u529f\u80fd\u8868\u9805\u76ee\u70ba\u300c\u958b\u555f\u4e3b\u63a7\u53f0\u300d\uff0c" + newline +
                   "       \u5982\u679c Java \u4e3b\u63a7\u53f0\u70ba\u986f\u793a\u72c0\u614b\uff0c\u5247\u70ba\u300c\u95dc\u9589\u4e3b\u63a7\u53f0\u300d\u3002" + newline + newline +
                   "      \u300c\u95dc\u65bc Java\u300d\u5c07\u986f\u793a Java 2 \u6a19\u6e96\u7248\u7684\u300c\u95dc\u65bc\u300d\u65b9\u584a\u3002" + newline + newline +
                   "      \u300c\u505c\u7528\u300d\u5c07\u5728\u76ee\u524d\u4ee5\u53ca\u5c07\u4f86\u7684\u968e\u6bb5\u4f5c\u696d\u4e2d\u505c\u7528 Java \u5716\u793a\uff0c\u4e26\u5f9e\u7cfb\u7d71\u5323\u4e2d\u79fb\u9664\u8a72\u5716\u793a\u3002\u7576\u91cd\u65b0\u555f\u52d5 Java Plug-in \u6642\uff0c" + newline +
                   "      \u7cfb\u7d71\u5323\u4e2d\u4e0d\u6703\u518d\u51fa\u73fe Java \u5716\u793a\u3002" + newline +
                   "      \u6709\u95dc\u5982\u4f55\u5728\u7cfb\u7d71\u5323\u4e2d\u986f\u793a\u505c\u7528\u7684 Java \u5716\u793a\u7684\u8cc7\u8a0a\uff0c\u8acb\u53c3\u95b1\u4e0b\u9762\u7684\u9644\u8a3b\u3002" + newline + newline +
                   "      \u300c\u7d50\u675f\u300d\u50c5\u6703\u5f9e\u76ee\u524d\u7684\u968e\u6bb5\u4f5c\u696d\u79fb\u9664\u7cfb\u7d71\u5323\u4e2d\u7684 Java \u5716\u793a\u3002\u7576\u91cd\u65b0\u555f\u52d5 Java Plug-in \u6642\uff0c" + newline +
                   "      Java \u5716\u793a\u6703\u518d\u6b21\u51fa\u73fe\u5728\u7cfb\u7d71\u5323\u4e2d\u3002" + newline + newline + newline + 
                   "                \u9644\u8a3b" + newline + newline +
                   "                1. \u5982\u679c\u52fe\u9078\u4e86\u300c\u5728\u7cfb\u7d71\u5323\u4e2d\u986f\u793a Java\u300d\uff0c\u5247\u5373\u4f7f\u9078\u53d6\u4e86\u300c\u4e0d\u8981\u555f\u52d5\u4e3b\u63a7\u53f0\u300d\uff0cJava \u5716\u793a\u4e5f\u6703" + newline +
                   "                \u986f\u793a\u5728\u7cfb\u7d71\u5323\u4e2d\u3002" + newline + newline +
                   "                2. \u82e5\u8981\u555f\u7528\u5df2\u505c\u7528\u7684 Java \u5716\u793a\uff0c\u8acb\u555f\u52d5 Java Plug-in \u63a7\u5236\u9762\u677f\uff0c\u52fe\u9078\u300c\u5728\u7cfb\u7d71\u5323" + newline +
                   "                \u4e2d\u986f\u793a Java\u300d\uff0c\u7136\u5f8c\u6309\u4e0b\u300c\u5957\u7528\u300d\u3002" + newline + newline +
                   "                3. \u5982\u679c\u6b63\u5728\u57f7\u884c\u5176\u4ed6 Java VM\uff0c\u4e14\u5176\u4ed6\u7684 Java \u5716\u793a\u5df2\u52a0\u5165\u5230\u7cfb\u7d71\u5323\u4e2d\uff0c\u8b8a\u66f4 " + newline +
                   "                Java Plug-in \u63a7\u5236\u9762\u677f\u4e2d\u7684\u8a2d\u5b9a\u4e0d\u6703\u5f71\u97ff\u9019\u4e9b\u5716\u793a\u3002" + newline +
                   "                \u6b64\u8a2d\u5b9a\u50c5\u6703\u5f71\u97ff\u4ee5\u5f8c Java VM \u555f\u52d5\u6642 Java \u5716\u793a\u7684\u986f\u793a\u65b9\u5f0f\u3002" + newline + newline + newline +
                   " \u9032\u968e" + newline + newline + newline +
                   "Java Run Time Environment" + newline + newline +
                   "      \u8b93 Java Plug-in \u53ef\u4ee5\u5728\u60a8\u6a5f\u5668\u4e0a\u6240\u5b89\u88dd\u7684\u4efb\u4f55 Java 2 JRE \u6216 SDK \u6a19\u6e96\u7248 v 1.3 \u6216 1.4 \u4e2d\u57f7\u884c\u3002" + newline +
                   "      Java Plug-in 1.3/1.4 \u5728\u767c\u9001\u6642\uff0c\u6703\u96a8\u9644\u9810\u8a2d\u7684 JRE\u3002" + newline +
                   "      \u4e0d\u904e\uff0c\u60a8\u53ef\u4ee5\u7f6e\u63db\u6b64\u9810\u8a2d\u7684 JRE\uff0c\u4f7f\u7528\u8f03\u820a\u6216\u8f03\u65b0\u7684\u7248\u672c\u3002\u300c\u63a7\u5236\u9762\u677f\u300d\u53ef\u4ee5\u81ea\u52d5\u5075\u6e2c" + newline +
                   "      \u5728\u6a5f\u5668\u4e0a\uff0c\u5df2\u5b89\u88dd\u4e4b\u6240\u6709\u7248\u672c\u7684 Java 2 SDK \u6216 JRE\u3002\u5728\u6b64\u6e05\u55ae\u65b9\u584a\u4e2d\uff0c\u6703\u986f\u793a" + newline +
                   "      \u60a8\u53ef\u4ee5\u4f7f\u7528\u7684\u6240\u6709\u7248\u672c\u3002" + newline +
                   "      \u6e05\u55ae\u4e2d\u7b2c\u4e00\u500b\u9805\u76ee\u6c38\u9060\u662f\u9810\u8a2d\u7684 JRE\uff1b\u6700\u5f8c\u4e00\u9805\u5247\u6c38\u9060\u662f\u500b\u70ba\u300c\u5176\u4ed6\u300d\u3002\u5982\u679c\u60a8\u9078\u64c7\u300c\u5176\u4ed6\u300d\uff0c\u5247\u5fc5\u9808" + newline +
                   "      \u6307\u5b9a\u5230 Java 2 JRE \u6216 SDK \u6a19\u6e96\u7248 v 1.3/1.4 \u7684\u8def\u5f91\u3002" + newline + newline + 
                   "                \u9644\u8a3b" + newline + newline +
                   "                \u53ea\u6709\u9032\u968e\u4f7f\u7528\u8005\u624d\u53ef\u8b8a\u66f4\u6b64\u9078\u9805\u3002\u4e0d\u5efa\u8b70\u8b8a\u66f4\u9810\u8a2d\u7684 JRE\u3002" + newline + newline + newline +
                   "Java Run Time \u53c3\u6578" + newline + newline + 
                   "      \u5229\u7528\u6307\u5b9a\u81ea\u8a02\u9078\u9805\uff0c\u4f86\u7f6e\u63db Java Plug-in \u9810\u8a2d\u7684\u555f\u52d5\u53c3\u6578\u3002\u6b64\u8a9e\u6cd5\u8207 java \u6307\u4ee4" + newline +
                   "      \u884c\u547c\u53eb\u7684\u53c3\u6578\u6240\u4f7f\u7528\u7684\u8a9e\u6cd5\u76f8\u540c\u3002\u8acb\u53c3\u95b1 Java 2 \u6a19\u6e96\u7248 (J2SE) \u6587\u4ef6\uff0c" + newline +
                   "      \u4ee5\u53d6\u5f97\u5b8c\u6574\u7684\u6307\u4ee4\u884c\u9078\u9805\u6e05\u55ae\u3002" + newline +
                   "      \u4ee5\u4e0b\u7684 URL \u6703\u96a8\u6642\u8b8a\u66f4\uff1a" + newline + newline +
                   "            http://java.sun.com/j2se/1.4/docs/tooldocs/<platform>/java.html" + newline + newline + 
                   "            \u5176\u4e2d <platform> \u70ba\u4e0b\u5217\u5176\u4e2d\u4e00\u9805\u4f5c\u696d\u7cfb\u7d71\uff1asolaris\u3001linux \u53ca win32\u3002" + newline + newline + 
                   "      \u4ee5\u4e0b\u70ba\u4e00\u4e9b Java runtime \u53c3\u6578\u7684\u7bc4\u4f8b\u3002" + newline + newline +
                   "      \u555f\u7528\u8207\u505c\u7528\u5047\u8a2d\u652f\u63f4" + newline + newline +
                   "            \u5982\u679c\u8981\u555f\u7528\u5047\u8a2d\u652f\u63f4\uff0c\u5fc5\u9808\u5728 Java Runtime \u53c3\u6578\u4e2d\uff0c\u6307\u5b9a\u4e0b\u5217\u7684\u7cfb\u7d71\u5c6c\u6027\uff1a" + newline + newline +
                   "                  -D[ enableassertions | ea ][:<package name>\"...\" | : <class name> ]" + newline + newline +
                   "            \u5982\u679c\u8981\u505c\u7528 Java Plug-in \u4e2d\u7684\u5047\u8a2d\uff0c\u8acb\u65bc Java Plug-in \u4e2d\u6307\u5b9a\u4e0b\u5217\u9805\u76ee\uff1a" + newline + newline + 
                   "                  -D[ disableassertions | da ][:<package name>\"...\" | : <class name> ]" + newline + newline + 
                   "            \u8acb\u53c3\u95b1\u300c\u5047\u8a2d\u6a5f\u80fd\u300d\uff0c\u4ee5\u53d6\u5f97\u555f\u7528/\u505c\u7528\u5047\u8a2d\u76f8\u95dc\u7684\u8a73\u7d30\u8cc7\u8a0a\uff1a" + newline +
                   "            http://java.sun.com/j2se/1.4/docs/guide/lang/assert.html\uff08URL \u53ef\u80fd\u96a8\u6642\u6703\u8b8a\u66f4\uff09\u3002" + newline + newline +
                   "            \u4f9d\u9810\u8a2d\uff0c\u5728 Java Plug-in \u7a0b\u5f0f\u78bc\u4e2d\u6703\u505c\u7528\u5047\u8a2d\u3002\u56e0\u70ba\u5047\u8a2d\u7684\u5f71\u97ff\u662f\u5728 Java Plug-in \u555f\u52d5\u6642\u6c7a\u5b9a\u7684\uff0c" + newline +
                   "            \u6240\u4ee5\u8b8a\u66f4 Java Plug-in \u63a7\u5236\u9762\u677f\u4e2d\u5047\u8a2d\u7684\u8a2d\u5b9a\u6642\uff0c\u5c07\u6703\u9700\u8981\u91cd\u65b0\u555f\u52d5\u700f\u89bd\u5668\uff0c" + newline +
                   "            \u4ee5\u4fbf\u65b0\u7684\u8a2d\u5b9a\u80fd\u751f\u6548\u3002" + newline + newline + 
                   "            \u56e0\u70ba Java Plug-in \u4e2d Java \u7a0b\u5f0f\u78bc\u4e5f\u5177\u6709\u5167\u5efa\u7684\u5047\u8a2d\uff0c\u6240\u4ee5\u53ef\u4ee5\u900f\u904e\u4e0b\u5217\u9805\u76ee\uff0c" + newline +
                   "            \u4f86\u555f\u7528 Java Plug-in \u7a0b\u5f0f\u78bc\u4e2d\u7684\u5047\u8a2d\uff1a" + newline + newline +
                   "                  -D[ enableassertions | ea ]:sun.plugin" + newline + newline + 
                   "      \u8ffd\u8e64\u8207\u8a18\u8f09\u652f\u63f4" + newline + newline +
                   "            \u8ffd\u8e64\u662f\u4e00\u9805\u6a5f\u80fd\uff0c\u53ef\u4ee5\u5c07 Java \u4e3b\u63a7\u53f0\u4e2d\u4efb\u4f55\u7684\u8f38\u51fa\uff0c\u91cd\u65b0\u5c0e\u5411\u81f3\u8ffd\u8e64\u6a94 (.plugin<version>.trace)\u3002" + newline + newline +
                   "                  -Djavaplugin.trace=true" + newline +
                   "                  -Djavaplugin.trace.option=basic|net|security|ext|liveconnect" + newline + newline +
                   "            \u5982\u679c\u60a8\u4e0d\u60f3\u4f7f\u7528\u9810\u8a2d\u8ffd\u8e64\u6a94\u7684\u540d\u7a31\uff1a" + newline + newline +
                   "                  -Djavaplugin.trace.filename=<tracefilename>" + newline + newline + 
                   "            \u8207\u8ffd\u8e64\u76f8\u4f3c\u7684\u662f\uff0c\u8a18\u8f09\u4e5f\u662f\u4e00\u9805\u6a5f\u80fd\uff0c\u53ef\u4ee5\u5c07 Java \u4e3b\u63a7\u53f0\u4e2d\u4efb\u4f55\u7684\u8f38\u51fa\uff0c\u4f7f\u7528 Java Logging API \u4f86" + newline +
                   "            \u91cd\u65b0\u5c0e\u5411\u81f3\u65e5\u8a8c\u6a94 (.plugin<version>.log)\u3002" + newline +
                   "            \u53ef\u4ee5\u5229\u7528\u555f\u7528\u5c6c\u6027 javaplugin.logging \u4f86\u555f\u52d5\u8a18\u8f09\u3002" + newline + newline +
                   "                  -Djavaplugin.logging=true" + newline + newline +
                   "            \u5982\u679c\u60a8\u4e0d\u60f3\u4f7f\u7528\u9810\u8a2d\u7684\u65e5\u8a8c\u6a94\u540d\u7a31\uff0c\u8acb\u8f38\u5165\uff1a" + newline + newline +
                   "                  -Djavaplugin.log.filename=<logfilename>" + newline + newline + 
                   "            \u6b64\u5916\uff0c\u5982\u679c\u60a8\u4e0d\u60f3\u6539\u5beb\u6bcf\u4e00\u968e\u6bb5\u4f5c\u696d\u7684\u8ffd\u8e64\u53ca\u65e5\u8a8c\u6a94\uff0c\u60a8\u53ef\u4ee5\u8a2d\u5b9a\u4e0b\u5217\u7684\u5c6c\u6027\uff1a" + newline + newline + 
                   "                  -Djavaplugin.outputfiles.overwrite=false." + newline + newline + 
                   "            \u5982\u679c\u6b64\u5c6c\u6027\u8a2d\u6210 false\uff0c\u5247\u5728\u6bcf\u4e00\u500b\u968e\u6bb5\u4f5c\u696d\u4e2d\uff0c\u5c07\u6703\u5c0d\u8ffd\u8e64\u53ca\u65e5\u8a8c\u6a94\u4ee5\u552f\u4e00\u7684\u540d\u7a31\u4f86\u547d\u540d\u3002\u5982\u679c" + newline +
                   "            \u5df2\u4f7f\u7528\u9810\u8a2d\u7684\u8ffd\u8e64\u53ca\u65e5\u8a8c\u6a94\uff0c\u5247\u6a94\u6848\u7684\u547d\u540d\u65b9\u5f0f\u5982\u4e0b\u6240\u793a\uff1a" + newline + newline +
                   "                  .plugin<username><date hash code>.trace" + newline +
                   "                  .plugin<username><date hash code>.log" + newline + newline +
                   "            \u900f\u904e\u300c\u63a7\u5236\u9762\u677f\u300d\u4f86\u8a2d\u5b9a\u7684\u8ffd\u8e64\u53ca\u8a18\u8f09\uff0c\u7576 Plug-in \u555f\u52d5\u6642\u4fbf\u6703\u751f\u6548\uff0c\u4f46\u662f\u5982\u679c\u5728 Plug-in " + newline +
                   "            \u57f7\u884c\u6642\uff0c\u4fbf\u900f\u904e\u300c\u63a7\u5236\u9762\u677f\u300d\u4f86\u9032\u884c\u8b8a\u66f4\uff0c\u5247\u8981\u5230\u91cd\u65b0\u555f\u52d5\u4e4b\u5f8c\uff0c\u8b8a\u66f4\u624d\u6703\u751f\u6548\u3002" + newline + newline + 
                   "            \u5982\u9700\u5176\u4ed6\u95dc\u65bc\u8ffd\u8e64\u53ca\u8a18\u8f09\u7684\u8cc7\u8a0a\uff0c\u8acb\u53c3\u95b1 Java Plug-in Developer Guide \u4e2d\u7684 Tracing and Logging\u3002" + newline + newline +
                   "      \u9664\u932f Java Plug-in \u4e2d\u7684 Applet" + newline + newline +
                   "            \u5728\u9664\u932f Java Plug-in \u4e2d\u7684 Applet \u6642\uff0c\u6703\u4f7f\u7528\u4e0b\u5217\u7684\u9078\u9805\u3002" + newline +
                   "            \u5982\u9700\u5176\u4ed6\u95dc\u65bc\u9019\u9805\u4e3b\u984c\u7684\u8cc7\u8a0a\uff0c\u8acb\u53c3\u95b1 Java Plug-in Developer Guide \u4e2d\u7684 Debugging Support\u3002" + newline + newline +
                   "                  -Djava.compiler=NONE" + newline +
                   "                  -Xnoagent" + newline +
                   "                  -Xdebug" + newline + 
                   "                  -Xrunjdwp:transport=dt_shmem,address=<connect-address>,server=y,suspend=n" + newline + newline +
                   "            <connect-address> \u53ef\u4ee5\u662f\u4efb\u610f\u7684\u5b57\u4e32\uff08\u7bc4\u4f8b\uff1a2502\uff09\uff0c\u7a0d\u5f8c Java \u9664\u932f\u7a0b\u5f0f (jdb) \u4fbf\u6703" + newline +
                   "            \u4f7f\u7528\u6b64\u9805\u76ee\u4f86\u9023\u63a5\u5230 JVM\u3002" + newline + newline + 
                   "      \u9810\u8a2d\u7684\u9023\u7dda\u903e\u6642" + newline + newline +
                   "            \u5982\u679c\u5230\u4f3a\u670d\u5668\u7684\u9023\u7dda\u662f\u7531 Applet \u6240\u5b8c\u6210\uff0c\u800c\u4e14\u4f3a\u670d\u5668\u4e26\u672a\u9069\u7576\u5730\u56de\u61c9\u6642\uff0c\u5247\u9019\u9805 Applet " + newline +
                   "            \u53ef\u80fd\u770b\u8d77\u4f86\u5df2\u7d93\u7576\u6a5f\uff0c\u800c\u4e14\u53ef\u80fd\u4e5f\u6703\u9020\u6210\u700f\u89bd\u5668\u7576\u6a5f\uff0c\u56e0\u70ba\u6c92\u6709\u7db2\u8def\u9023\u7dda\u903e\u6642" + newline +
                   "            \uff08\u4f9d\u9810\u8a2d\u6b64\u9805\u76ee\u8a2d\u6210 -1\uff0c\u8868\u793a\u6c92\u6709\u903e\u6642\u8a2d\u5b9a\uff09\u3002" + newline + newline +
                   "            \u5982\u679c\u8981\u907f\u514d\u6b64\u554f\u984c\uff0cJava Plug-in \u5df2\u7d93\u5c0d\u6240\u6709\u7684 HTTP \u9023\u7dda\uff0c\u65b0\u589e\u9810\u8a2d\u7684\u7db2\u8def\u903e\u6642\u503c\uff082 \u5206\u9418\uff09\u3002" + newline +
                   "            \u60a8\u53ef\u4ee5\u7f6e\u63db Java Runtime \u53c3\u6578\u4e2d\u7684\u9019\u9805\u8a2d\u5b9a\uff1a" + newline + newline +
                   "                  -Dsun.net.client.defaultConnectTimeout=value in milliseconds" + newline + newline +
                   "            \u53e6\u4e00\u9805\u60a8\u53ef\u4ee5\u8a2d\u5b9a\u7684\u7db2\u8def\u5c6c\u6027\u70ba sun.net.client.defaultReadTimeout\u3002" + newline + newline +
                   "                  -Dsun.net.client.defaultReadTimeout=value in milliseconds" + newline + newline +
                   "                  \u9644\u8a3b" + newline + newline +
                   "                  \u4f9d\u9810\u8a2d\uff0cJava Plug-in \u4e0d\u6703\u8a2d\u5b9a sun.net.client.defaultReadTimeout\u3002\u5982\u679c\u60a8\u60f3\u8981\u8a2d\u5b9a\u6b64\u9805\u76ee\uff0c" + newline +
                   "                  \u53ef\u4ee5\u900f\u904e\u4e0a\u8ff0\u7684 Java Runtime \u53c3\u6578\u4f86\u5b8c\u6210\u6b64\u4f5c\u696d\u3002" + newline + newline + newline +
                   "            \u7db2\u8def\u5c6c\u6027\u63cf\u8ff0\uff1a" + newline + newline +
                   "            sun.net.client.defaultConnectTimeout" + newline +
                   "            sun.net.client.defaultReadTimeout" + newline + newline +
                   "                  \u9019\u4e9b\u5c6c\u6027\u5206\u5225\u6307\u5b9a java.net.URLConnection \u6240\u4f7f\u7528\u4e4b" + newline +
                   "                  \u5354\u5b9a\u8655\u7406\u5668\u7684\u9810\u8a2d\u9023\u7dda\u53ca\u8b80\u53d6\u903e\u6642\u503c\u3002\u7531\u5354\u5b9a\u8655\u7406\u5668\u6240\u8a2d\u5b9a\u7684\u9810\u8a2d\u503c\u70ba -1\uff0c" + newline +
                   "                  \u8868\u793a\u6c92\u6709\u903e\u6642\u8a2d\u5b9a\u3002" + newline + newline +
                   "                  sun.net.client.defaultConnectTimeout \u53ef\u6307\u5b9a\u903e\u6642\uff08\u4ee5\u6beb\u79d2\u70ba\u55ae\u4f4d\uff09\uff0c\u4ee5\u4fbf\u5efa\u7acb\u5230\u4e3b\u6a5f\u7684\u9023\u7dda\u3002" + newline +
                   "                  \u4f8b\u5982\uff0c\u91dd\u5c0d http \u9023\u7dda\u800c\u8a00\uff0c\u5b83\u4fbf\u662f\u5728\u5efa\u7acb\u5230 http \u4f3a\u670d\u5668\u9023\u7dda\u6642\u7684\u903e\u6642\u3002" + newline +
                   "                  \u91dd\u5c0d ftp \u9023\u7dda\u800c\u8a00\uff0c\u5b83\u4fbf\u662f\u5728\u5efa\u7acb\u5230 ftp \u4f3a\u670d\u5668\u9023\u7dda\u6642\u7684\u903e\u6642\u3002" + newline + newline +
                   "                  sun.net.client.defaultReadTimeout \u53ef\u4ee5\u6307\u5b9a\u7576\u5b8c\u6210\u5efa\u7acb\u5230\u8cc7\u6e90\u7684\u9023\u7dda\u4e4b\u5f8c\uff0c" + newline +
                   "                  \u5f9e\u8f38\u51fa\u4e32\u6d41\u4e2d\u8b80\u53d6\u6642\u7684\u903e\u6642\uff08\u4ee5\u6beb\u79d2\u70ba\u55ae\u4f4d\uff09\u3002" + newline + newline + 
                   "            \u5982\u9700\u95dc\u65bc\u9019\u4e9b\u7db2\u8def\u5c6c\u6027\u7684\u63cf\u8ff0\uff0c" + newline +
                   "            \u8acb\u53c3\u95b1\uff1ahttp://java.sun.com/j2se/1.4/docs/guide/net/properties.html \u3002" + newline + newline + newline +
                   " \u700f\u89bd\u5668" + newline + newline + newline +
                   "\u6b64\u9762\u677f\u53ea\u95dc\u806f\u5230 Microsoft Windows \u7684\u5b89\u88dd\u4f5c\u696d\uff1b\u5b83\u4e0d\u6703\u51fa\u73fe\u5728\u5176\u4ed6\u7684\u5b89\u88dd\u4f5c\u696d\u4e2d\u3002\u8acb\u52fe\u9078\u4efb\u4f55" + newline +
"\u60a8\u5e0c\u671b Java Plug-in \u70ba\u5176\u9810\u8a2d Java runtime \u7684\u700f\u89bd\u5668\uff0c\u800c\u4e0d\u662f\u8a72\u700f\u89bd\u5668\u7684\u5167\u90e8 JVM\u3002" + newline +
"\u9019\u4fbf\u662f\u5728 Internet Explorer \u8207 Netscape 6 \u4e2d\uff0c\u900f\u904e Java Plug-in \u4f86\u555f\u7528 Applet \u6a19\u7c64\u652f\u63f4\u3002" + newline + newline + newline +
                   " \u4ee3\u7406" + newline + newline +newline +
                   "\u4f7f\u7528\u300c\u4ee3\u7406\u300d\u9762\u677f\u4f86\u4f7f\u7528\u700f\u89bd\u5668\u9810\u8a2d\u503c\uff0c\u6216\u7f6e\u63db\u4e0d\u540c\u5354\u5b9a\u7684\u4ee3\u7406\u4f4d\u5740\u548c\u901a\u8a0a\u57e0\u3002" + newline + newline + 
                   "\u4f7f\u7528\u700f\u89bd\u5668\u9810\u8a2d\u503c" + newline + newline + 
                   "      \u8acb\u52fe\u9078\u6b64\u9805\u76ee\uff0c\u4f7f\u7528\u700f\u89bd\u5668\u9810\u8a2d\u7684\u4ee3\u7406\u8a2d\u5b9a\u3002\u6b64\u70ba\u9810\u8a2d\u7684\u8a2d\u5b9a\uff08\u5df2\u52fe\u9078\uff09\u3002" + newline + newline +
                   "Proxy \u8cc7\u8a0a\u8868" + newline + newline + 
                   "      \u60a8\u53ef\u4ee5\u5229\u7528\u53d6\u6d88\u52fe\u9078\u300c\u4f7f\u7528\u700f\u89bd\u5668\u9810\u8a2d\u503c\u300d\u6838\u53d6\u65b9\u584a\uff0c\u4f86\u7f6e\u63db\u9810\u8a2d\u8a2d\u5b9a\uff0c\u7136\u5f8c\u518d" + newline +
                   "      \u5b8c\u6210\u6838\u53d6\u65b9\u584a\u4e0b\u65b9\u7684\u4ee3\u7406\u8cc7\u8a0a\u3002\u60a8\u53ef\u4ee5\u5c0d\u6bcf\u4e00\u500b\u652f\u63f4\u7684\u5354\u5b9a\uff0c\u8f38\u5165\u4ee3\u7406\u4f4d\u5740\u548c\u901a\u8a0a\u57e0" + newline +
                   "      \uff1aHTTP\u3001Secure (HTTPS)\u3001FTP\u3001Gopher \u548c Socks\u3002" + newline + newline + 
                   "\u7121\u4ee3\u7406\u4e3b\u6a5f" + newline + newline + 
                   "      \u9019\u662f\u4e00\u90e8\u4e3b\u6a5f\u6216\u4e3b\u6a5f\u6e05\u55ae\uff0c\u5176\u4e2d\u4e0d\u4f7f\u7528\u4ee3\u7406\u3002\u300c\u7121\u4ee3\u7406\u4e3b\u6a5f\u300d\u901a\u5e38\u662f\u7528\u65bc" + newline +
                   "      \u4f01\u696d\u7db2\u8def\u74b0\u5883\u4e2d\u7684\u5167\u90e8\u4e3b\u6a5f\u3002" + newline + newline + 
                   "\u81ea\u52d5\u4ee3\u7406\u914d\u7f6e URL" + newline + 
                   "      \u9019\u662f JavaScript \u6a94\uff08\u526f\u6a94\u540d\u70ba .js \u6216 .pac\uff09\u7684 URL\uff0c\u5176\u4e2d\u5305\u542b FindProxyForURL \u51fd\u6578\u3002" + newline +
                   "      FindProxyForURL \u5177\u6709\u4e00\u9805\u908f\u8f2f\uff0c\u53ef\u6c7a\u5b9a\u5c0d\u9023\u7dda\u8981\u6c42\u61c9\u4f7f\u7528\u7684\u4ee3\u7406\u4f3a\u670d\u5668\u3002" + newline + newline + 
                   "\u5982\u9700\u5176\u4ed6\u95dc\u65bc\u4ee3\u7406\u914d\u7f6e\u7684\u8a73\u7d30\u8cc7\u8a0a\uff0c\u8acb\u53c3\u95b1 Java Plug-in Developer Guide \u4e2d\u540d\u7a31\u70ba Proxy Configuration \u7684" + newline +
                   "\u4e00\u7ae0\u3002" + newline + newline + newline +
                   " \u5feb\u53d6" + newline + newline + newline +
                   "           \u9644\u8a3b" + newline + newline +
                   "           \u9019\u4e9b\u8655\u6240\u6307\u7684\u5feb\u53d6\u70basticky\u5feb\u53d6\uff1b\u5982 Java Plug-in \u6240\u5efa\u7acb\u548c\u63a7\u5236\u7684\u78c1\u789f\u5feb\u53d6\uff0c" + newline +
                   "           \u700f\u89bd\u5668\u7121\u6cd5\u52a0\u4ee5\u6539\u5beb\u3002\u8acb\u53c3\u95b1 Java Plug-in Developer Guide \u4e2d\u7684 Applet Caching\uff0c\u4ee5\u53d6\u5f97\u5176\u4ed6\u8cc7\u8a0a\u3002" + newline + newline + newline +
                   " \u555f\u7528\u5feb\u53d6" + newline + newline + 
                   "      \u8acb\u52fe\u9078\u6b64\u9805\u76ee\u4f86\u555f\u7528\u5feb\u53d6\u3002\u6b64\u70ba\u9810\u8a2d\u7684\u8a2d\u5b9a\uff08\u5df2\u52fe\u9078\uff09\u3002\u5728\u555f\u52d5 Applet \u5feb\u53d6\u4e4b\u5f8c\uff0c" + newline +
                   "      \u6548\u80fd\u4fbf\u6703\u6539\u9032\uff0c\u56e0\u70ba\u4e00\u65e6\u5feb\u53d6 Applet \u4e4b\u5f8c\uff0c\u7576\u518d\u6b21\u53c3\u7167\u6642\uff0c\u4fbf\u4e0d\u9700\u8981\u518d\u4e0b\u8f09\u3002" + newline + newline +
                   "      \u4e0b\u5217\u985e\u578b\u70ba\u900f\u904e HTTP/HTTPS \u4f86\u4e0b\u8f09\u7684 Java Plug-in \u5feb\u53d6\u6a94\uff1a" + newline + newline +
                   "            .jar\uff08jar \u6a94\u6848\uff09" + newline +
                   "            .zip\uff08zip \u6a94\u6848\uff09" + newline +
                   "            .class\uff08java \u985e\u5225\u6a94\uff09" + newline +
                   "            .au\uff08\u97f3\u8a0a\u6a94\u6848\uff09" + newline +
                   "            .wav\uff08\u97f3\u8a0a\u6a94\u6848\uff09" + newline +
                   "            .jpg\uff08\u5f71\u50cf\u6a94\uff09" + newline +
                   "            .gif\uff08\u5f71\u50cf\u6a94\uff09" + newline + newline +
                   "\u6aa2\u8996\u5feb\u53d6\u4e2d\u7684\u6a94\u6848" + newline + newline + 
                   "      \u8acb\u6309\u4e0b\u6b64\u9805\u76ee\u4f86\u6aa2\u8996\u5feb\u53d6\u7684\u6a94\u6848\u3002\u9019\u6642\u6703\u5f48\u51fa\u53e6\u4e00\u500b\u5c0d\u8a71\u65b9\u584a\uff08Java Plug-in \u5feb\u53d6\u6aa2\u8996\u5668\uff09\uff0c\u5176\u4e2d\u6703\u986f\u793a\u5feb\u53d6\u7684\u6a94\u6848\u3002" + newline +
                   "      \u300c\u5feb\u53d6\u6aa2\u8996\u5668\u300d\u4e2d\u6703\u986f\u793a\u4e0b\u5217\u95dc\u65bc\u5feb\u53d6\u4e2d\u6a94\u6848\u7684\u8cc7\u8a0a\uff1a\u540d\u7a31\u3001\u985e\u578b\u3001\u5927\u5c0f\u3001\u904e\u671f\u65e5\u3001" + newline +
                   "      \u6700\u5f8c\u4fee\u6539\u6642\u9593\u3001\u7248\u672c\u53ca URL\u3002\u5728\u300c\u5feb\u53d6\u6aa2\u8996\u5668\u300d\u4e2d\uff0c\u60a8\u4e5f\u53ef\u4ee5\u9078\u64c7\u6027\u5730\u522a\u9664\u5feb\u53d6\u4e2d\u7684\u6a94\u6848\u3002" + newline +
                   "      \u9019\u662f\u4ee5\u4e0b\u6240\u8aaa\u660e\u7684\u300c\u6e05\u9664\u5feb\u53d6\u300d\u9078\u9805\u7684\u53e6\u66ff\u4ee3\u4f5c\u6cd5\uff0c\u53ef\u4ee5\u522a\u9664\u5feb\u53d6\u4e2d\u6240\u6709\u7684\u6a94\u6848\u3002" + newline + newline +
                   "\u6e05\u9664\u5feb\u53d6" + newline + newline + 
                   "      \u8acb\u6309\u4e0b\u6b64\u9805\u76ee\u4f86\u6e05\u9664\u5feb\u53d6\u4e2d\u6240\u6709\u7684\u6a94\u6848\u3002\u5728\u79fb\u9664\u6a94\u6848\u4e4b\u524d\uff0c\u7cfb\u7d71\u6703\u63d0\u793a\u60a8\uff1a\u300c\u6e05\u9664 ... \u4e2d\u5168\u90e8\u7684\u6a94\u6848\uff1f\u300d\u3002" + newline + newline + 
                   " \u4f4d\u7f6e" + newline + newline + 
                   "      \u60a8\u53ef\u4ee5\u4f7f\u7528\u6b64\u9805\u76ee\uff0c\u4f86\u6307\u5b9a\u5feb\u53d6\u7684\u4f4d\u7f6e\u3002\u5feb\u53d6\u7684\u9810\u8a2d\u4f4d\u7f6e\u70ba <user home>/.jpi_cache\uff0c" + newline +
                   "      \u5176\u4e2d <user home> \u70ba\u7cfb\u7d71\u5c6c\u6027 user.home \u7684\u503c\u3002\u5176\u503c\u8996 OS \u800c\u5b9a\u3002" + newline + newline +
                   "\u5927\u5c0f" + newline + newline + 
                   "      \u60a8\u53ef\u4ee5\u52fe\u9078\u300c\u7121\u9650\u5236\u300d\u4f86\u9032\u884c\u7121\u9650\u5236\u5927\u5c0f\u7684\u5feb\u53d6\uff1b\u6216\u662f\u60a8\u53ef\u4ee5\u8a2d\u5b9a\u5feb\u53d6\u7684\u300c\u6700\u5927\u503c\u300d\u5927\u5c0f\u3002" + newline +
                   "      \u5982\u679c\u5feb\u53d6\u5927\u5c0f\u8d85\u51fa\u6307\u5b9a\u7684\u9650\u5236\uff0c\u4fbf\u6703\u5c07\u5df2\u5feb\u53d6\u7684\u6700\u820a\u6a94\u6848\u79fb\u9664\uff0c\u76f4\u5230\u5feb\u53d6\u5927\u5c0f" + newline +
                   "      \u4e0d\u8d85\u904e\u9650\u5236\u70ba\u6b62\u3002" + newline + newline + 
                   "\u58d3\u7e2e" + newline + newline + 
                   "      \u60a8\u53ef\u4ee5\u5c07 JAR \u5feb\u53d6\u6a94\u6848\u7684\u58d3\u7e2e\u8a2d\u6210\u300c\u7121\u300d\u6216\u300c\u9ad8\u300d\u3002\u7576\u60a8\u6307\u5b9a\u8f03\u9ad8\u7684\u58d3\u7e2e\u6642\uff0c" + newline +
                   "      \u96d6\u53ef\u7bc0\u7701\u8a18\u61b6\u9ad4\uff0c\u4f46\u662f\u6548\u80fd\u4fbf\u6703\u964d\u7d1a\uff1b\u6700\u4f73\u7684\u6548\u80fd\u5c07\u6703" + newline +
                   "      \u662f\u4ee5\u4e0d\u58d3\u7e2e\u624d\u80fd\u9054\u6210\u3002" + newline + newline + newline + 
                   " \u8b49\u66f8" + newline + newline + newline +
                   "\u53ef\u4ee5\u9078\u53d6\u56db\u7a2e\u985e\u578b\u7684\u6191\u8b49\uff1a" + newline + newline +
                   "      \u5e36\u7c3d\u540d\u7684 Applet" + newline + 
                   "      \u5b89\u5168\u7db2\u7ad9" + newline + 
                   "      \u7c3d\u7f72\u8005 CA" + newline +
                   "      \u5b89\u5168\u7db2\u7ad9 CA" + newline + newline + 
                   "\u5e36\u7c3d\u540d\u7684 Applet" + newline + newline +
                   "      \u9019\u4e9b\u70ba\u4f7f\u7528\u8005\u6240\u4fe1\u4efb\u5e36\u7c3d\u540d\u7684 Applet \u7684\u8b49\u66f8\u3002\u9019\u4e9b\u986f\u793a\u65bc\u5e36\u7c3d\u540d Applet \u6e05\u55ae\u4e2d\u7684\u8b49\u66f8\uff0c" + newline +
                   "      \u662f\u5f9e\u4f4d\u65bc <user home>/.java \u76ee\u9304\u4e2d\u7684\u8b49\u66f8\u6a94 jpicerts<version> \u4f86\u8b80\u53d6\u7684\u3002" + newline + newline +
                   "\u5b89\u5168\u7db2\u7ad9" + newline + newline +
                   "      \u9019\u4e9b\u70ba\u5b89\u5168\u7db2\u7ad9\u7684\u8b49\u66f8\u3002\u9019\u4e9b\u986f\u793a\u65bc\u5b89\u5168\u7db2\u7ad9\u6e05\u55ae\u4e2d\u7684\u8b49\u66f8\uff0c\u662f\u5f9e\u4f4d\u65bc <user home>/.java \u76ee\u9304\u4e2d\u7684" + newline +
                   "      \u8b49\u66f8\u6a94 jpihttpscerts<version> \u4f86\u8b80\u53d6\u7684\u3002" + newline + newline + 
                   "\u7c3d\u7f72\u8005 CA" + newline + newline +
                   "      \u9019\u4e9b\u70ba\u300c\u8a8d\u8b49\u4e2d\u5fc3 (CA)\u300d \u767c\u51fa\u7d66\u5e36\u7c3d\u540d\u7684 Applet \u7684\u8b49\u66f8\uff1b\u9019\u4e9b\u8a8d\u8b49\u4e2d\u5fc3\u767c\u51fa\u8b49\u66f8\u7d66" + newline +
                   "      \u5e36\u7c3d\u540d\u7684 Applet \u7684\u7c3d\u7f72\u8005\u3002\u9019\u4e9b\u986f\u793a\u65bc\u300c\u7c3d\u7f72\u8005 CA\u300d\u6e05\u55ae\u4e2d\u7684\u8b49\u66f8\uff0c" + newline +
                   "      \u662f\u5f9e\u4f4d\u65bc <jre>/lib/security \u76ee\u9304\u4e2d\u7684\u8b49\u66f8\u6a94 cacerts \u4f86\u8b80\u53d6\u7684\u3002" + newline + newline +
                   "\u5b89\u5168\u7db2\u7ad9 CA" + newline + newline +
                   "      \u9019\u4e9b\u70ba\u5b89\u5168\u7db2\u7ad9\u4e4b\u300c\u8a8d\u8b49\u4e2d\u5fc3 (CA)\u300d\u7684\u8b49\u66f8\uff1b\u9019\u4e9b\u4fbf\u662f\u767c\u51fa\u5b89\u5168\u7db2\u7ad9\u8b49\u66f8\u7684\u4f7f\u7528\u8005\u3002" + newline +
                   "      \u9019\u4e9b\u986f\u793a\u65bc\u300c\u5b89\u5168\u7db2\u7ad9 CA\u300d\u6e05\u55ae\u4e2d\u7684\u8b49\u66f8\uff0c\u662f\u5f9e\u4f4d\u65bc <jre>/lib/security \u76ee\u9304\u4e2d\u7684" + newline +
                   "      \u8b49\u66f8\u6a94 jssecacerts \u4f86\u8b80\u53d6\u7684\u3002" + newline + newline +
                   "\u5c0d\u65bc\u5e36\u7c3d\u540d\u7684 Applet \u53ca \u5b89\u5168\u7db2\u7ad9\u8b49\u66f8\u800c\u8a00\uff0c\u5171\u6709\u56db\u7a2e\u9078\u9805\uff1a\u300c\u532f\u5165\u300d\u3001\u300c\u532f\u51fa\u300d\u3001\u300c\u79fb\u9664\u300d\u548c\u300c\u660e\u7d30\u300d\u3002" + newline +
                   "\u4f7f\u7528\u8005\u53ef\u4ee5\u532f\u5165\u3001\u532f\u51fa\u3001\u79fb\u9664\u53ca\u6aa2\u8996\u8b49\u66f8\u7684\u660e\u7d30\u3002" + newline + newline + 
                   "\u5c0d\u65bc\u7c3d\u7f72\u7a0b\u5f0f CA \u53ca\u5b89\u5168\u7db2\u7ad9 CA \u800c\u8a00\uff0c\u53ea\u6709\u300c\u660e\u7d30\u300d\u4e00\u7a2e\u9078\u9805\u3002\u4f7f\u7528\u8005\u53ea\u80fd\u6aa2\u8996\u8b49\u66f8\u7684\u660e\u7d30\u3002" + newline + newline + newline +
                   " \u66f4\u65b0" + newline + newline + newline +
                   "\u5728\u6b64\u9762\u677f\u4e2d\uff0c\u5177\u6709\u4e00\u500b\u300c\u66f4\u65b0 Java\u300d\u6309\u9215\uff0c\u53ef\u5141\u8a31\u4f7f\u7528\u8005\u5f9e Java Update \u7db2\u7ad9\u4f86\u53d6\u5f97 Java Runtime Environment \u6700\u65b0" + newline +
"\u7684\u66f4\u65b0\u9805\u76ee\u3002\u6b64\u9762\u677f\u53ea\u95dc\u806f\u5230 Microsoft Windows \u5e73\u53f0\uff1b\u5b83\u4e0d\u6703\u51fa\u73fe\u5728" + newline +
"\u5176\u4ed6\u7684\u5e73\u53f0\u4e2d\uff08\u5982 Solaris/Linux\uff09\u3002"}
};
} 

 

