/*
 * @(#)JNITypeParser.java	1.1 02/01/07
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import java.util.List;
import java.util.ArrayList;

public class JNITypeParser {

    static final char SIGNATURE_ENDCLASS = ';';
    static final char SIGNATURE_FUNC = '(';
    static final char SIGNATURE_ENDFUNC = ')';

    private String signature;
    private List typeNameList;
    private List signatureList;
    private int currentIndex;

    JNITypeParser(String signature) {
        this.signature = signature;
    }

    static String typeNameToSignature(String signature) {
        StringBuffer buffer = new StringBuffer();
        int firstIndex = signature.indexOf('[');
        int index = firstIndex;
        while (index != -1) {
            buffer.append('[');
            index = signature.indexOf('[', index + 1);
        }

        if (firstIndex != -1) {
            signature = signature.substring(0, firstIndex);
        }

        if (signature.equals("boolean")) {
            buffer.append('Z');
        } else if (signature.equals("byte")) {
            buffer.append('B');
        } else if (signature.equals("char")) {
            buffer.append('C');
        } else if (signature.equals("short")) {
            buffer.append('S');
        } else if (signature.equals("int")) {
            buffer.append('I');
        } else if (signature.equals("long")) {
            buffer.append('J');
        } else if (signature.equals("float")) {
            buffer.append('F');
        } else if (signature.equals("double")) {
            buffer.append('D');
        } else {
            buffer.append('L');
            buffer.append(signature.replace('.', '/'));
            buffer.append(';');
        }

        return buffer.toString();
    }

    String typeName() {
        return (String)typeNameList().get(typeNameList().size()-1);
    }

    List argumentTypeNames() {
        return typeNameList().subList(0, typeNameList().size() - 1);
    }
        
    String signature() {
        return (String)signatureList().get(signatureList().size()-1);
    }

    List argumentSignatures() {
        return signatureList().subList(0, signatureList().size() - 1);
    }

    int dimensionCount() {
        int count = 0;
        String signature = signature();
        while (signature.charAt(count) == '[') {
            count++;
        }
        return count;
    }

    String componentSignature(int level) {
        return signature().substring(level);
    }
        
    private synchronized List signatureList() {
        if (signatureList == null) {
            signatureList = new ArrayList(10);
            String elem;
            
            currentIndex = 0;
    
            while(currentIndex < signature.length()) {
                elem = nextSignature();
                signatureList.add(elem);
            }
            if (signatureList.size() == 0) {
                throw new IllegalArgumentException("Invalid JNI signature '" +
                                                   signature + "'");
            }
        }
        return signatureList;
    }

    private synchronized List typeNameList() {
        if (typeNameList == null) {
            typeNameList = new ArrayList(10);
            String elem;
            
            currentIndex = 0;
    
            while(currentIndex < signature.length()) {
                elem = nextTypeName();
                typeNameList.add(elem);
            }
            if (typeNameList.size() == 0) {
                throw new IllegalArgumentException("Invalid JNI signature '" +
                                                   signature + "'");
            }
        }
        return typeNameList;
    }

    private String nextSignature() {
        char key = signature.charAt(currentIndex++);

        switch(key) {
            case (JDWP.Tag.ARRAY):
                return  key + nextSignature();
                
            case (JDWP.Tag.OBJECT):
                int endClass = signature.indexOf(SIGNATURE_ENDCLASS, 
                                                 currentIndex);
                String retVal = signature.substring(currentIndex - 1, 
                                                    endClass + 1);
                currentIndex = endClass + 1;
                return retVal;
            
            case (JDWP.Tag.VOID):
            case (JDWP.Tag.BOOLEAN):
            case (JDWP.Tag.BYTE):
            case (JDWP.Tag.CHAR):
            case (JDWP.Tag.SHORT):
            case (JDWP.Tag.INT):
            case (JDWP.Tag.LONG):
            case (JDWP.Tag.FLOAT):
            case (JDWP.Tag.DOUBLE):
                return String.valueOf(key);

            case SIGNATURE_ENDFUNC:
            case SIGNATURE_FUNC:
                return nextSignature();
                
            default:
                throw new IllegalArgumentException(
                    "Invalid JNI signature character '" + key + "'");
                
        }
    }

    private String nextTypeName() {
        char key = signature.charAt(currentIndex++);

        switch(key) {
            case (JDWP.Tag.ARRAY):
                return  nextTypeName() + "[]";
                
            case (JDWP.Tag.BYTE):
                return "byte";
                
            case (JDWP.Tag.CHAR):
                return "char";
                
            case (JDWP.Tag.OBJECT):
                int endClass = signature.indexOf(SIGNATURE_ENDCLASS, 
                                                 currentIndex);
                String retVal = signature.substring(currentIndex, 
                                                    endClass);
                retVal = retVal.replace('/','.');
                currentIndex = endClass + 1;
                return retVal;
                
            case (JDWP.Tag.FLOAT):
                return "float";
                
            case (JDWP.Tag.DOUBLE):
                return "double";
                
            case (JDWP.Tag.INT):
                return "int";
                
            case (JDWP.Tag.LONG):
                return "long";
                
            case (JDWP.Tag.SHORT):
                return "short";
                
            case (JDWP.Tag.VOID):
                return "void";
                
            case (JDWP.Tag.BOOLEAN):
                return "boolean";

            case SIGNATURE_ENDFUNC:
            case SIGNATURE_FUNC:
                return nextTypeName();
                
            default:
                throw new IllegalArgumentException(
                    "Invalid JNI signature character '" + key + "'");
                
        }
    }
}

class JDWP {
    static class Tag {
        static final int ARRAY = 91;
        static final int BYTE = 66;
        static final int CHAR = 67;
        static final int OBJECT = 76;
        static final int FLOAT = 70;
        static final int DOUBLE = 68;
        static final int INT = 73;
        static final int LONG = 74;
        static final int SHORT = 83;
        static final int VOID = 86;
        static final int BOOLEAN = 90;
        static final int STRING = 115;
        static final int THREAD = 116;
        static final int THREAD_GROUP = 103;
        static final int CLASS_LOADER = 108;
        static final int CLASS_OBJECT = 99;
    }

}
