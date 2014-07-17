/*
 * @(#)OldStubs.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.tools.javah;

import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.Vector;
import java.util.Enumeration;
import com.sun.javadoc.*;



/**
 * Generate stubs for JDK 1.0 style native method interface.
 *
 * Note that we go great lengths to generate code that looks as much as
 * possible like what the C version of javah generated.  Though this goal
 * has made this code awkward at certain places, it has the advantage that
 * we can stress test this implementation against the reference C
 * implementation manually.
 *
 * @author  Sucheta Dambalkar(Revised)
 */

public class OldStubs extends Gen {
    
    public OldStubs(RootDoc root){
	super(root);
    }
    
    protected String getIncludes() {
	return "#include <StubPreamble.h>\n";
    }
    
    protected void write(OutputStream o, ClassDoc clazz)
	throws ClassNotFoundException {
	
	PrintWriter pw = wrapWriter(o);
	MethodDoc[] methods = clazz.methods();
	String mangledClazz = Mangle.mangle(clazz.qualifiedName(), Mangle.Type.CLASS);
	pw.println("/* Stubs for class " + mangledClazz + " */");
	
	for (int i = 0; i < methods.length; i++) {
	    if(methods[i].isNative()){
		StringBuffer  sb = new StringBuffer(200);
		MethodDoc method     = methods[i];
		String methodName = method.name();
		String mangledMethod = Mangle.mangle(methodName, 
						     Mangle.Type.FIELDSTUB);
		Type rType = method.returnType();
		String typesig = method.signature();
		TypeSignature newTypeSig = new TypeSignature(root);
		String methodSig = newTypeSig.getTypeSignature(typesig, rType);
		String implName = mangledClazz + '_' + mangledMethod;
		String stubName = Mangle.mangleMethod(method, root,clazz,
						      Mangle.Type.METHOD_JDK_1);
		
		/*
		 * Comment string.
		 */
		sb.append("/* SYMBOL: \"");
		sb.append(mangledClazz);
		sb.append('/');
		sb.append(mangledMethod);
		sb.append(methodSig);
		sb.append("\", ");
		sb.append(stubName);
		sb.append(" */\n");
		
		/*
		 * Stub method call.
		 */
		String exp = Util.getPlatformString("dll.export");
		if (exp != null)
		    sb.append(exp);
		sb.append("stack_item *");
		sb.append(stubName);
		sb.append("(stack_item *_P_,struct execenv *_EE_) {\n");
		
		/* 
		 * Prototype and call to real method. 
		 */
		StringBuffer proto   = new StringBuffer(100);
		StringBuffer call    = new StringBuffer(100);
		StringBuffer fwds    = new StringBuffer(100);
		
		Type    retType      = method.returnType();
		String     rtc       = retType.typeName();
		boolean returnIs64   =  (rtc.equals("long") || rtc.equals("double"));
		boolean returnIsVoid = rtc.equals("void");
		
		/* return value */
		proto.append("\textern ");
		proto.append(getStubCType(retType, true));
		proto.append(' ');
		
		call.append('\t');
		if (rtc.equals("void")) {
		    call.append("(void) ");
		} else if (rtc.equals("double")){
		    call.append("SET_DOUBLE(_tval, _P_, ");
		} else if (rtc.equals("long")) {
		    call.append("SET_INT64(_tval, _P_, ");
		} else {
		    call.append("_P_[0].");
		    call.append(getStackItemName32(rtc));
		    call.append(" = ");
		}
		
		if (rtc.equals("boolean"))
		    call.append('(');
		
		if (returnIs64) 
		    fwds.append("\tjvalue _tval;\n");
		
		/* method name */
		proto.append(implName);
		proto.append('(');
		call.append(implName);
		call.append('(');
		proto.append("void *");
		
		if (method.isStatic()) {
		    call.append("NULL");
		} else {
		    call.append("_P_[0].p");
		}
		
		/* arguments */
		Parameter[] pargs = method.parameters();
		Type[] argTypes = new Type[pargs.length];
		
		for(int p = 0; p < pargs.length; p++){
		    argTypes[p] = pargs[p].type();
		}
		
		int    nargs = argTypes.length;
		
		for (int j = 0, nthWord = 0; j < nargs; j++) {
		    if (j == 0) {
			proto.append(',');
			call.append(',');
			if (!method.isStatic()) {
			    nthWord++;
			}
			
		    }
		    
		    proto.append(getStubCType(argTypes[j], false));
		    String atc = argTypes[j].typeName();
		    
		    if (atc.equals("long")) {
			call.append("GET_INT64(_t");
			call.append(nthWord);
			call.append(", _P_+");
			call.append(nthWord);
			call.append(')');
		    } else if (atc.equals("double")) {
			call.append("GET_DOUBLE(_t");
			call.append(nthWord);
			call.append(", _P_+");
			call.append(nthWord);
			call.append(')');
		    } else {
			call.append("((_P_[");
			call.append(nthWord);
			call.append("].");
			call.append(getStackItemName32(atc));
			call.append("))");
		    }
		    
		    if (atc.equals("long") || atc.equals("double")) {
			fwds.append("\tjvalue _t");
			fwds.append(nthWord);
			fwds.append(";\n");
			nthWord += 2;
		    } else {
			nthWord += 1;
		    }
		    
		    if (j < (nargs - 1)) {
			proto.append(',');
			call.append(',');
		    }
		}
		
		if (rtc.equals("boolean"))
		    call.append(") ? TRUE : FALSE");
		
		/* finished arguments */
		proto.append(");\n");
		if (returnIs64)
		    call.append(')');
		call.append(");\n");
	  
		/* now really print prototype and call */
		sb.append(fwds);
		sb.append(proto);
		sb.append(call);
		
		/*
		 * Return statement.
		 */
		sb.append("\treturn _P_");
		if (returnIs64)
		    sb.append(" + 2");
		else if (!returnIsVoid)
		    sb.append(" + 1");
		
		sb.append(";\n}");
		
		pw.println(sb);
	    }
	}
    }
    
    private String getStackItemName32(String typecode) {
	if (typecode.equals("int") ||
	    typecode.equals("boolean") || 
	    typecode.equals("char") ||
	    typecode.equals("short") ||
	    typecode.equals("byte"))
	    return "i";
	
	if (typecode.equals("float"))
	    return "f";
	
	return "p";
    }
    
    private String getStubCType(Type p, boolean isReturn) {
	String tc = p.typeName();
	
	if((p.asClassDoc() != null) || (p.dimension().indexOf("[]") != -1)){
	    if (isReturn) 
		return "void*";
	    else 
		return "void *";
	}else if((p.asClassDoc() == null)
		 || (p.dimension().indexOf("[]") == -1)){ 
	    if(tc.equals("void")) return "void";
	    else if(tc.equals("boolean") || tc.equals("byte") 
		    || tc.equals("short") || tc.equals("char") 
		    || tc.equals("int"))
		return "int32_t";
	    else if(tc.equals("float")) return "float";
	    else if(tc.equals("double")) return "double";
	    else if(tc.equals("long")) return "int64_t";
	} else {
	    Util.bug("unknown.type.in.method.signature");
	}
	return null; /* dead code */
    }
    
    protected String getFileSuffix() {
	return ".c";
    }
}


