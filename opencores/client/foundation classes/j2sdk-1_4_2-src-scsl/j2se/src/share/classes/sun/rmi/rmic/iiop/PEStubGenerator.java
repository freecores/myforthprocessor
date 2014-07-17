/*
 * @(#)PEStubGenerator.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Licensed Materials - Property of IBM
 * RMI-IIOP v1.0
 * Copyright IBM Corp. 1998 1999  All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

package sun.rmi.rmic.iiop;

import java.io.File;
import java.io.IOException;
import java.util.Vector;
import java.util.Hashtable;
import java.util.Enumeration;
import sun.tools.java.Identifier;
import sun.tools.java.ClassNotFound;
import sun.tools.java.ClassDefinition;
import sun.tools.java.ClassDeclaration;
import sun.tools.java.CompilerError;
import sun.rmi.rmic.IndentingWriter;
import java.util.HashSet;
import java.util.Arrays;
import sun.rmi.rmic.Main;

/**
 * An example, smart IIOP stub/tie generator for rmic.
 * To be used when calls fail due to NO_PERMISSION
 *
 * @author	Max Mortazavi
 */

public class PEStubGenerator extends sun.rmi.rmic.iiop.StubGenerator {

    private String preInvokeClass = null; //XXX

    /**
     * Examine and consume command line arguments.
     * @param argv The command line arguments. Ignore null
     * and unknown arguments. Set each consumed argument to null.
     * @param error Report any errors using the main.error() methods.
     * @return true if no errors, false otherwise.
     */
    public boolean parseArgs(String argv[], Main main) {
	// Parse options...
        boolean result = super.parseArgs(argv,main);
        if(result){
	    for (int i = 0; i < argv.length; i++) {
		if (argv[i] != null) {
		    String arg = argv[i].toLowerCase();
		    if (arg.equals("-peiiop")) { //XXX
			argv[i] = null;
                    } else if (arg.equals("-preinvokeclass")){
                        argv[i]= null;
            	        if (++i < argv.length && argv[i] != null && !argv[i].startsWith("-")) {
                            preInvokeClass = argv[i];
			    argv[i] = null;
                        } else {
			    main.error("rmic.option.requires.argument", "-preinvokeclass");
			    result = false;
			}
		    } 
		}
	    }
        }
	return result;
    }

    void writeStubMethod (  IndentingWriter p,
                            CompoundType.Method method,
                            CompoundType theType) throws IOException {
        
        // Wtite the method declaration and opening brace...
        
	String methodName = method.getName();
	String methodIDLName = method.getIDLName();

	Type paramTypes[] = method.getArguments();
	String paramNames[] = method.getArgumentNames();
	Type returnType = method.getReturnType();
	ValueType[] exceptions = getStubExceptions(method,false);
        
        addNamesInUse(method);
        addNameInUse("_type_ids");

	p.p("public " + getName(returnType) + " " + methodName + "(");
	for(int i = 0; i < paramTypes.length; i++) {
	    if (i > 0)
		p.p(", ");
	    p.p(getName(paramTypes[i]) + " " + paramNames[i]);
	}

	p.p(")");
	if (exceptions.length > 0) {
	    p.p(" throws ");
	    for(int i = 0; i < exceptions.length; i++) {
		if (i > 0) {
		    p.p(", ");
		}
		p.p(getName(exceptions[i]));
	    }
	}

	p.plnI(" {");
        
        // Now create the method body...

        p.pln("int numTries = 0 ;"); //XXX
        p.plnI("while (true) { "); //XXX
        
        if (localStubs) {
            writeLocalStubMethodBody(p,method,theType);
        } else {
            writeNonLocalStubMethodBody(p,method,theType);
        }
 
        //XXX end of while(true) loop
        p.pOln("}");

        // Close out the method...
        

	p.pOln("}");
    }


    void writeNonLocalStubMethodBody (  IndentingWriter p,
					CompoundType.Method method,
					CompoundType theType) throws IOException {
        //XXX::
        if(preInvokeClass == null){
            System.out.println(sun.rmi.rmic.Main.getText
                               ("rmic.option.requires.argument", "-preinvokeclass"));
            System.exit(1);                
        }
        //::XXX
                 
	String methodName = method.getName();
	String methodIDLName = method.getIDLName();

	Type paramTypes[] = method.getArguments();
	String paramNames[] = method.getArgumentNames();
	Type returnType = method.getReturnType();
	ValueType[] exceptions = getStubExceptions(method,true);
    
        String in = getVariableName("in");
        String out = getVariableName("out");
        String ex = getVariableName("ex");
        
        // Decide if we need to use the new streams for
        // any of the read calls...

        boolean needNewReadStreamClass = false;
        for (int i = 0; i < exceptions.length; i++) {
            if (exceptions[i].getIdentifier() != idRemoteException &&
		exceptions[i].isType(TYPE_VALUE) &&
		needNewReadStreamClass(exceptions[i])) {
                needNewReadStreamClass = true;
                break;
            }
        }
        if (!needNewReadStreamClass) {
            for (int i = 0; i < paramTypes.length; i++) {
                if (needNewReadStreamClass(paramTypes[i])) {
                    needNewReadStreamClass = true;
                    break;
                }
            }
        }
        if (!needNewReadStreamClass) {
            needNewReadStreamClass = needNewReadStreamClass(returnType);
        }

        // Decide if we need to use the new streams for
        // any of the write calls...
        
        boolean needNewWriteStreamClass = false;
        for (int i = 0; i < paramTypes.length; i++) {
            if (needNewWriteStreamClass(paramTypes[i])) {
                needNewWriteStreamClass = true;
                break;
            }
        }

        // Now write the method, inserting casts where needed...
        
	p.plnI("try {");
	if (needNewReadStreamClass) {
	    p.pln(idExtInputStream + " "+in+" = null;");
	} else {
	    p.pln(idInputStream + " "+in+" = null;");
	}
	p.plnI("try {");
        p.pln(preInvokeClass+".preinvoke( this, ++ numTries ) ;"); //XXX

	String argStream = "null";
        
        if (needNewWriteStreamClass) {
	    p.plnI(idExtOutputStream + " "+out+" = ");
	    p.pln("(" + idExtOutputStream + ")");
	    p.pln("_request(\"" + methodIDLName + "\", true);");
	    p.pO();
        } else {
	    p.pln("OutputStream "+out+" = _request(\"" + methodIDLName + "\", true);");
        }
        
	if (paramTypes.length > 0) {
	    writeMarshalArguments(p, out, paramTypes, paramNames);
	    p.pln();
	}
	argStream = out;

	if (returnType.isType(TYPE_VOID)) {
	    p.pln("_invoke(" + argStream + ");" );
            p.pln("return;");
	} else {
	    if (needNewReadStreamClass) {
		p.plnI(in+" = (" + idExtInputStream + ")_invoke(" + argStream + ");");
		p.pO();
	    } else {
		p.pln(in+" = _invoke(" + argStream + ");");
	    }
	    p.p("return ");
	    writeUnmarshalArgument(p, in, returnType, null);
	    p.pln();
	}
        
        // Handle ApplicationException...
        
	p.pOlnI("} catch ("+getName(idApplicationException)+" "+ex+") {");
	if (needNewReadStreamClass) {
	    p.pln(in + " = (" + idExtInputStream + ") "+ex+".getInputStream();");
        } else {
	    p.pln(in + " = "+ex+".getInputStream();");
        }

        boolean idRead = false;
        boolean idAllocated = false;
	for(int i = 0; i < exceptions.length; i++) {
	    if (exceptions[i].getIdentifier() != idRemoteException) {
	
		// Is this our special-case IDLEntity exception?
	            
		if (exceptions[i].isIDLEntityException() && !exceptions[i].isCORBAUserException()) {
				    
		    // Yes.
				    
		    if (!idAllocated && !idRead) {
			p.pln("String id = "+ex+".getId();");
			idAllocated = true;
		    }
				    
		    String helperName = IDLNames.replace(exceptions[i].getQualifiedIDLName(false),"::",".");
		    helperName += "Helper";
		    p.plnI("if (id.equals("+helperName+".id())) {");
		    p.pln("throw "+helperName+".read("+in+");");
				    
		} else {
				    
		    // No.
				    
		    if (!idAllocated && !idRead) {
	p.pln("String id = "+in+".read_string();");
			idAllocated = true;
			idRead = true;
		    } else if (idAllocated && !idRead) {
			p.pln("id = "+in+".read_string();");
			idRead = true;
		    }
		    p.plnI("if (id.equals(\""+getExceptionRepositoryID(exceptions[i])+"\")) {");
		    p.pln("throw ("+getName(exceptions[i])+") "+in+".read_value(" + getName(exceptions[i]) + ".class);");
		}
		p.pOln("}");
	    }
	}
	if (!idAllocated && !idRead) {
	    p.pln("String id = "+in+".read_string();");
	    idAllocated = true;
	    idRead = true;
	} else if (idAllocated && !idRead) {
	    p.pln("id = "+in+".read_string();");
	    idRead = true;
	}
	p.pln("throw new UnexpectedException(id);");
        
        // Handle RemarshalException...

	p.pOlnI("} catch ("+getName(idRemarshalException)+" "+ex+") {");

        /* XXX
	if (!returnType.isType(TYPE_VOID)) {
	    p.p("return ");
	}
	p.p(methodName + "(");
	for(int i = 0; i < paramTypes.length; i++) {
	    if (i > 0) {
		p.p(",");
	    }
	    p.p(paramNames[i])
	}
	p.pln(");");
        XXX */

        //XXX Reinvokation Check
	p.pOlnI("} catch (org.omg.CORBA.NO_PERMISSION npex) {");
        p.pln("if ( ! (" + preInvokeClass + 
              ".isMinorCodeMatch( this, numTries, npex.minor )) ||");
        p.pln("    (" + preInvokeClass + ".isTooManyTries( this, numTries )))");
        p.pln("    throw npex ;");

	// Ensure that we release the reply...
		
	p.pOlnI("} finally {");
	p.pln("_releaseReply("+in+");");
		
	p.pOln("}"); 
        
        // Handle SystemException...
        
	p.pOlnI("} catch (SystemException "+ex+") {");
	p.pln("throw Util.mapSystemException("+ex+");");		
	p.pOln("}");

        // returnResult(p,returnType);

    }


    void writeLocalStubMethodBody (IndentingWriter p,
				   CompoundType.Method method,
				   CompoundType theType) throws IOException {
    
	String paramNames[] = method.getArgumentNames();
	Type returnType = method.getReturnType();
	ValueType[] exceptions = getStubExceptions(method,false);
        String methodName = method.getName();
        String methodIDLName = method.getIDLName();
        
        p.plnI("if (!Util.isLocal(this)) {");
        writeNonLocalStubMethodBody(p,method,theType);
        p.pOlnI("} else {");
        String so = getVariableName("so");

        p.pln("ServantObject "+so+" = _servant_preinvoke(\""+methodIDLName+"\","+getName(theType)+".class);");
        p.plnI("if ("+so+" == null) {");
        if (!returnType.isType(TYPE_VOID)) {
            p.p("return ");
        }
        p.p(methodName+"(");
        for (int i = 0; i < paramNames.length; i++) {
	    if (i > 0)
		p.p(", ");
	    p.p(paramNames[i]);
        }
        p.pln(");");


        ////Begin Fix for void return types for this generator
	if (returnType.isType(TYPE_VOID)) {
            p.pln("return;");
	}
        ////End Fix for void return types for this generator

        p.pOln("}");
        p.plnI("try {");
        
        // Generate code to copy required arguments, and
        // get back the names by which all arguments are known...
        
        String[] argNames = writeCopyArguments(method,p);

        // Now write the method...
        
        boolean copyReturn = mustCopy(returnType);
        String resultName = null;
        if (!returnType.isType(TYPE_VOID)) {
            if (copyReturn) {
                resultName = getVariableName("result");
                p.p(getName(returnType)+" "+resultName + " = ");  
            } else {
                p.p("return ");
            }
        }


        p.p("(("+getName(theType)+")"+so+".servant)."+methodName+"(");       
        
        for (int i = 0; i < argNames.length; i++) {
	    if (i > 0)
		p.p(", ");
	    p.p(argNames[i]);
        }

        p.pln(");");

        ////// Begin Fix for return type of void for this generator
        
        if (copyReturn) {
            p.pln("return ("+getName(returnType)+")Util.copyObject("+resultName+",_orb());");  
        } 

	if (returnType.isType(TYPE_VOID)) {
            p.pln("return;");
	}

        ////// End Fix for return type of void for this generator

        String e1 = getVariableName("ex");
        String e2 = getVariableName("exCopy");
        p.pOlnI("} catch (Throwable "+e1+") {");
          
        p.pln("Throwable "+e2+" = (Throwable)Util.copyObject("+e1+",_orb());");
	for(int i = 0; i < exceptions.length; i++) {
	    if (exceptions[i].getIdentifier() != idRemoteException &&
		exceptions[i].isType(TYPE_VALUE)) {
		p.plnI("if ("+e2+" instanceof "+getName(exceptions[i])+") {");
		p.pln("throw ("+getName(exceptions[i])+")"+e2+";");
		p.pOln("}");
	    }
	}
        
        p.pln("throw Util.wrapException("+e2+");");
	p.pOlnI("} finally {");
	p.pln("_servant_postinvoke("+so+");");
	p.pOln("}");
	p.pOln("}");    
    }


}

