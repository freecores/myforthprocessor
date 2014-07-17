/*
 * @(#)ArrayReferenceImpl.java	1.1 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;

import sun.jvm.hotspot.oops.Oop;

public class ArrayReferenceImpl extends ObjectReferenceImpl
    implements ArrayReference
{
    int length = -1;

    ArrayReferenceImpl(VirtualMachine aVm, sun.jvm.hotspot.oops.Instance aRef) {
        super(aVm,aRef);
    }

    ArrayTypeImpl arrayType() {
        return (ArrayTypeImpl)type();
    }

    /**
     * Return array length.
     * Need not be synchronized since it cannot be provably stale.
     */
    public int length() {  //fixme jjh
        if (length == -1) {
            length = -1;
//             try {
//                 length = JDWP.ArrayReference.Length.
//                     process(vm, this).arrayLength;
//             } catch (JDWPException exc) {
//                 throw exc.toJDIException();
//             }
        }
        return length;
    }

    public Value getValue(int index) {
        List list = getValues(index, 1);
        return (Value)list.get(0);
    }

    public List getValues() {
        return getValues(0, -1);
    }
    
    /**
     * Validate that the range to set/get is valid.
     * length of -1 (meaning rest of array) has been converted
     * before entry.
     */
    private void validateArrayAccess(int index, int length) {
        // because length can be computed from index,
        // index must be tested first for correct error message
        if ((index < 0) || (index > length())) {
            throw new IndexOutOfBoundsException(
                        "Invalid array index: " + index);
        }
        if (length < 0) {
            throw new IndexOutOfBoundsException(
                        "Invalid array range length: " + length);
        }
        if (index + length > length()) {
            throw new IndexOutOfBoundsException(
                        "Invalid array range: " + 
                        index + " to " + (index + length - 1));
        }
    }

    public List getValues(int index, int length) {  //fixme jjh
        if (length == -1) { // -1 means the rest of the array
           length = length() - index;
        }
        validateArrayAccess(index, length);
        if (length == 0) {
            return new ArrayList();
        }

        List vals;
        vals = new ArrayList();
//         try {
//             vals = JDWP.ArrayReference.GetValues.
//                 process(vm, this, index, length).values;
//         } catch (JDWPException exc) {
//             throw exc.toJDIException();
//         }

        return vals;
    }

    public void setValue(int index, Value value)
            throws InvalidTypeException,
                   ClassNotLoadedException {
        throw new UnsupportedOperationException("Not Read Only: ArrayReference.setValue(...)");
    }

    public void setValues(List values)
            throws InvalidTypeException,
                   ClassNotLoadedException {
        setValues(0, values, 0, -1);
    }

    public void setValues(int index, List values, 
                          int srcIndex, int length)
            throws InvalidTypeException,
                   ClassNotLoadedException {

        throw new UnsupportedOperationException("Not Read Only: ArrayReference.setValue(...)");
        
    }

    public String toString() {
        return "instance of " + arrayType().componentTypeName() + 
               "[" + length() + "] (id=" + uniqueID() + ")";
    }

//     byte typeValueKey() {
//         return JDWP.Tag.ARRAY;
//     }

//     void validateAssignment(ValueContainer destination) 
//                             throws InvalidTypeException, ClassNotLoadedException {
//         try {
//             super.validateAssignment(destination);
//         } catch (ClassNotLoadedException e) {
//             /*
//              * An array can be used extensively without the 
//              * enclosing loader being recorded by the VM as an 
//              * initiating loader of the array type. In addition, the 
//              * load of an array class is fairly harmless as long as 
//              * the component class is already loaded. So we relax the
//              * rules a bit and allow the assignment as long as the 
//              * ultimate component types are assignable. 
//              */
//             boolean valid = false;
//             JNITypeParser destParser = new JNITypeParser(
//                                        destination.signature());
//             JNITypeParser srcParser = new JNITypeParser(
//                                        arrayType().signature());
//             int destDims = destParser.dimensionCount();
//             if (destDims <= srcParser.dimensionCount()) {
//                 /*
//                  * Remove all dimensions from the destination. Remove
//                  * the same number of dimensions from the source. 
//                  * Get types for both and check to see if they are 
//                  * compatible. 
//                  */
//                 String destComponentSignature = 
//                     destParser.componentSignature(destDims);
//                 Type destComponentType = 
//                     destination.findType(destComponentSignature);
//                 String srcComponentSignature = 
//                     srcParser.componentSignature(destDims);
//                 Type srcComponentType = 
//                     arrayType().findComponentType(srcComponentSignature);
//                 valid = ArrayTypeImpl.isComponentAssignable(destComponentType,
//                                                           srcComponentType);
//             }

//             if (!valid) {
//                 throw new InvalidTypeException("Cannot assign " +
//                                                arrayType().name() + 
//                                                " to " +
//                                                destination.typeName());
//             }
//         }
//    }

    /*
     * Represents an array component to other internal parts of this 
     * implementation. This is not exposed at the JDI level. Currently,
     * this class is needed only for type checking so it does not even
     * reference a particular component - just a generic component 
     * of this array. In the future we may need to expand its use.
     */
    class Component implements ValueContainer {
        public Type type() throws ClassNotLoadedException {
            return arrayType().componentType();
        }
        public String typeName() {
            return arrayType().componentTypeName();
        }
        public String signature() {
            return arrayType().componentSignature();
        }
        public Type findType(String signature) throws ClassNotLoadedException {
            return arrayType().findComponentType(signature);
        }
    }
}
