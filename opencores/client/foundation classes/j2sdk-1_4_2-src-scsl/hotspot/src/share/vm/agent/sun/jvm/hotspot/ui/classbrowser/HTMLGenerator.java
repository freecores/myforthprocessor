/*
 * @(#)HTMLGenerator.java	1.11 03/01/23 11:50:04
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui.classbrowser;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.asm.*;
import sun.jvm.hotspot.asm.sparc.*;
import sun.jvm.hotspot.asm.x86.*;
import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.compiler.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.interpreter.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.tools.jcore.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

public class HTMLGenerator implements /* imports */ ClassConstants {
   private static final String DUMP_KLASS_OUTPUT_DIR = ".";
   private static final int NATIVE_CODE_SIZE = 200;
   private static final String spaces = "&nbsp;&nbsp;";
   private static final String tab = "&nbsp;&nbsp;&nbsp;&nbsp;";

   private static CPUHelper cpuHelper;
   static {
      VM.registerVMInitializedObserver(new Observer() {
         public void update(Observable o, Object data) {
            initialize();
         }
      });
   }

   private static synchronized void initialize() {
      String cpu = VM.getVM().getCPU();
      if (cpu.equals("sparc")) {
         cpuHelper = new SPARCHelper();
      } else if (cpu.equals("x86")) {
         cpuHelper = new X86Helper();
      } else {
          throw new RuntimeException("cpu '" + cpu + "' is not yet supported!");
      }
   }

   protected static synchronized CPUHelper getCPUHelper() {
      return cpuHelper;
   }

   protected String escapeHTMLSpecialChars(String value) {
      StringBuffer buf = new StringBuffer();
      int len = value.length();
      for (int i=0; i < len; i++) {
         char c = value.charAt(i);
         switch (c) {
            case '<':
               buf.append("&lt;");
               break;
            case '>':
               buf.append("&gt;");
               break;
            case '&':
               buf.append("&amp;");
               break;
            default:
               buf.append(c);
               break;
         }
      }
      return buf.toString();
   }

   public String genHTMLForMessage(String message) {
      StringBuffer buf = new StringBuffer();
      buf.append(genHTMLPrologue(message));
      buf.append(genHTMLEpilogue());
      return buf.toString();
   }

   public String genHTMLErrorMessage(Exception exp) {
      exp.printStackTrace();
      return genHTMLForMessage(exp.getClass().getName() + " : " + exp.getMessage());
   }

   public String genHTMLForWait(String message) {
      StringBuffer buf = new StringBuffer();
      buf.append(genHTMLPrologue("Please wait .."));
      buf.append("<h2>");
      buf.append(message);
      buf.append("</h2>");
      return buf.toString();
   }

   public String genEmptyHTML() {
      return "<html></html>";
   }

   protected String genHTMLPrologue() {
      return "<html><body>";
   }

   protected String genHTMLPrologue(String title) {
      StringBuffer buf = new StringBuffer();
      buf.append("<html><head><title>");
      buf.append(title);
      buf.append("</title></head>");
      buf.append("<body>");
      buf.append("<h2>");
      buf.append(title);
      buf.append("</h2>");
      return buf.toString();
   }

   protected String genHTMLEpilogue() {
      return "</body></html>";
   }

   protected String genKlassTitle(InstanceKlass klass) {
      StringBuffer buf = new StringBuffer();
      if (klass.isInterface()) {
         buf.append("interface");
      } else {
         buf.append("class");
      }

      buf.append(' ');
      buf.append(klass.getName().asString().replace('/', '.'));
      buf.append(' ');
      buf.append('@');
      buf.append(klass.getHandle().toString());
      return buf.toString();
   }

   protected String genBaseHref() {
      return "";
   }

   protected String genKlassHref(InstanceKlass klass) {
      return genBaseHref() + "klass=" + klass.getHandle();
   }

   protected String genKlassLink(InstanceKlass klass) {
      StringBuffer buf = new StringBuffer();
      buf.append("<a href='");
      buf.append(genKlassHref(klass));
      buf.append("'>");
      buf.append(genKlassTitle(klass));
      buf.append("</a>");
      return buf.toString();
   }

   protected String genMethodModifierString(AccessFlags acc) {
      StringBuffer buf = new StringBuffer();
      if (acc.isPrivate()) {
         buf.append("private ");
      } else if (acc.isProtected()) {
         buf.append("protected ");
      } else if (acc.isPublic()) {
         buf.append("public ");
      }

      if (acc.isStatic()) {
         buf.append("static ");
      } else if (acc.isAbstract() ) {
         buf.append("abstract ");
      } else if (acc.isFinal()) {
         buf.append("final ");
      }

       if (acc.isNative()) {
         buf.append("native ");
      }

      if (acc.isStrict()) {
         buf.append("strict ");
      }

      if (acc.isSynchronized()) {
         buf.append("synchronized ");
      }
      return buf.toString();
   }

   protected String genMethodNameAndSignature(Method method) {
      StringBuffer buf = new StringBuffer();
      buf.append(genMethodModifierString(method.getAccessFlagsObj()));
      Symbol sig = method.getSignature();
      new SignatureConverter(sig, buf).iterateReturntype();
      buf.append(" ");
      String methodName = method.getName().asString();
      buf.append(escapeHTMLSpecialChars(methodName));
      buf.append('(');
      new SignatureConverter(sig, buf).iterateParameters();
      buf.append(')');
      return buf.toString().replace('/', '.');
   }

   protected String genMethodTitle(Method method) {
      StringBuffer buf = new StringBuffer();
      buf.append(genMethodNameAndSignature(method));
      buf.append(' ');
      buf.append('@');
      buf.append(method.getHandle().toString());
      return buf.toString();
   }

   protected String genMethodHref(Method m) {
      return genBaseHref() + "method=" + m.getHandle();
   }

   protected String genMethodLink(Method m) {
      StringBuffer buf = new StringBuffer();
      buf.append("<a href='");
      buf.append(genMethodHref(m));
      buf.append("'>");
      buf.append(genMethodTitle(m));
      buf.append("</a>");
      return buf.toString();
   }

   protected String genMethodAndKlassLink(Method m) {
      StringBuffer buf = new StringBuffer();
      buf.append(genMethodLink(m));
      buf.append(" of ");
      buf.append(genKlassLink((InstanceKlass) m.getMethodHolder()));
      return buf.toString();
   }

   protected String genNMethodHref(NMethod nm) {
      return genBaseHref() + "nmethod=" + nm.getAddress();
   }

   public String genNMethodTitle(NMethod nmethod) {
      StringBuffer buf = new StringBuffer();
      Method m = nmethod.getMethod();

      buf.append("Disassembly for compiled method [");
      buf.append(genMethodTitle(m));
      buf.append(" ] ");
      buf.append('@');
      buf.append(nmethod.getAddress().toString());
      return buf.toString();
   }

   protected String genNMethodLink(NMethod nm) {
      StringBuffer buf = new StringBuffer();
      buf.append("<a href='");
      buf.append(genNMethodHref(nm));
      buf.append("'>");
      buf.append(genNMethodTitle(nm));
      buf.append("</a>");
      return buf.toString();
   }

   protected BytecodeDisassembler createBytecodeDisassembler(Method m) {
      return new BytecodeDisassembler(m);
   }

   private String genLowHighShort(int val) {
      StringBuffer buf = new StringBuffer();
      buf.append('#');
      buf.append(new Integer(val & 0xFFFF).toString());
      buf.append(" #");
      buf.append(new Integer((val >> 16) & 0xFFFF).toString());
      return buf.toString();
   }

   protected String genHTMLTableForConstantPool(ConstantPool cpool) {
      StringBuffer buf = new StringBuffer();
      buf.append("<table border='1'>");

      buf.append("<tr>");
      buf.append("<th>");
      buf.append("Index");
      buf.append("</th>");
      buf.append("<th>");
      buf.append("Constant Type");
      buf.append("</th>");
      buf.append("<th>");
      buf.append("Constant Value");
      buf.append("</th>");
      buf.append("</tr>");

      final int length = (int) cpool.getLength();
      // zero'th pool entry is always invalid. ignore it.
      for (int index = 1; index < length; index++) {
         buf.append("<tr>");

         buf.append("<td>");
         buf.append(new Integer(index).toString());
         buf.append("</td>");

         int ctag = (int) cpool.getTags().getByteAt((int) index);
         switch (ctag) {
            case JVM_CONSTANT_Integer:
               buf.append("<td>JVM_CONSTANT_Integer</td>");
               buf.append("<td>");
               buf.append(new Integer(cpool.getIntAt(index)).toString());
               buf.append("</td>");
               break;

            case JVM_CONSTANT_Float:
               buf.append("<td>JVM_CONSTANT_Float</td>");
               buf.append("<td>");
               buf.append(new Float(cpool.getFloatAt(index)).toString());
               buf.append("F</td>");
               break;

            case JVM_CONSTANT_Long:
               buf.append("<td>JVM_CONSTANT_Long</td>");
               buf.append("<td>");
               buf.append(new Long(cpool.getLongAt(index)).toString());
               buf.append("L</td>");
               // long entries occupy two slots
               index++;
               break;

            case JVM_CONSTANT_Double:
               buf.append("<td>JVM_CONSTANT_Double</td>");
               buf.append("<td>");
               buf.append(new Double(cpool.getDoubleAt(index)).toString());
               buf.append("D</td>");
               // double entries occupy two slots
               index++;
               break;

            case JVM_CONSTANT_UnresolvedClass:
               buf.append("<td>JVM_CONSTANT_UnresolvedClass</td>");
               buf.append("<td>");
               buf.append(cpool.getSymbolAt(index).asString());
               buf.append("</td>");
               break;

            case JVM_CONSTANT_Class:
               buf.append("<td>JVM_CONSTANT_Class</td>");
               buf.append("<td>"); 
               Klass klass = (Klass) cpool.getObjAt(index);
               if (klass instanceof InstanceKlass) {
                  buf.append(genKlassLink((InstanceKlass) klass));
               } else {
                  buf.append(klass.getName().asString().replace('/', '.'));
               }
               buf.append("</td>");
               break;

            case JVM_CONSTANT_UnresolvedString:
               buf.append("<td>JVM_CONSTANT_UnresolvedString</td>");
               buf.append("<td>\"");
               buf.append(escapeHTMLSpecialChars(cpool.getSymbolAt(index).asString()));
               buf.append("\"</td>");
               break;

            case JVM_CONSTANT_Utf8:
               buf.append("<td>JVM_CONSTANT_Utf8</td>");
               buf.append("<td>\"");
               buf.append(escapeHTMLSpecialChars(cpool.getSymbolAt(index).asString()));
               buf.append("\"</td>");
               break;

            case JVM_CONSTANT_String:
               buf.append("<td>JVM_CONSTANT_String</td>");
               buf.append("<td>\"");
               buf.append(escapeHTMLSpecialChars(OopUtilities.stringOopToString(cpool.getObjAt(index))));
               buf.append("\"</td>");
               break;

            case JVM_CONSTANT_Fieldref:
               buf.append("<td>JVM_CONSTANT_Fieldref</td>");
               buf.append("<td>");
               buf.append(genLowHighShort(cpool.getIntAt(index)));
               buf.append("</td>");
               break;

            case JVM_CONSTANT_Methodref:
               buf.append("<td>JVM_CONSTANT_Methodref</td>");
               buf.append("<td>");
               buf.append(genLowHighShort(cpool.getIntAt(index)));
               buf.append("</td>");
               break;

            case JVM_CONSTANT_InterfaceMethodref:
               buf.append("<td>JVM_CONSTANT_InterfaceMethodref</td>");
               buf.append("<td>");
               buf.append(genLowHighShort(cpool.getIntAt(index)));
               buf.append("</td>");
               break;

            case JVM_CONSTANT_NameAndType:
               buf.append("<td>JVM_CONSTANT_NameAndType</td>");
               buf.append("<td>");
               buf.append(genLowHighShort(cpool.getIntAt(index)));
               buf.append("</td>");
               break;

            case JVM_CONSTANT_ClassIndex:
               buf.append("<td>JVM_CONSTANT_ClassIndex</td>");
               buf.append("<td>");
               buf.append(new Integer(cpool.getIntAt(index)).toString());
               buf.append("</td>");
               break;

            case JVM_CONSTANT_StringIndex:
               buf.append("<td>JVM_CONSTANT_StringIndex</td>");
               buf.append("<td>");
               buf.append(new Integer(cpool.getIntAt(index)).toString());
               buf.append("</td>");
               break;
         }

         buf.append("</tr>");
      }

      buf.append("</table>");
      return buf.toString();
   }

   public String genHTML(ConstantPool cpool) {
      try {
         StringBuffer buf = new StringBuffer();
         buf.append(genHTMLPrologue(genConstantPoolTitle(cpool)));
         buf.append("<h3>Holder Class</h3>");
         buf.append(genKlassLink((InstanceKlass) cpool.getPoolHolder()));
         buf.append("<h3>Constants</h3>");
         buf.append(genHTMLTableForConstantPool(cpool));
         buf.append(genHTMLEpilogue());
         return buf.toString();
      } catch (Exception exp) {
         return genHTMLErrorMessage(exp);
      }
   }

   protected String genConstantPoolHref(ConstantPool cpool) {
      return genBaseHref() + "cpool=" + cpool.getHandle();
   }

   protected String genConstantPoolTitle(ConstantPool cpool) {
      StringBuffer buf = new StringBuffer();
      buf.append("Constant Pool of [");
      buf.append(genKlassTitle((InstanceKlass) cpool.getPoolHolder()));
      buf.append("] @");
      buf.append(cpool.getHandle());
      return buf.toString();
   }

   protected String genConstantPoolLink(ConstantPool cpool) {
      StringBuffer buf = new StringBuffer();
      buf.append("<a href='");
      buf.append(genConstantPoolHref(cpool));
      buf.append("'>");
      buf.append(genConstantPoolTitle(cpool));
      buf.append("</a>");
      return buf.toString();
   }

   public String genHTML(Method method) {
      try {
         final StringBuffer buf = new StringBuffer();
         buf.append(genHTMLPrologue(genMethodTitle(method)));

         buf.append("<h3>Holder Class</h3>");
         buf.append(genKlassLink((InstanceKlass) method.getMethodHolder()));

         NMethod nmethod = method.getNativeMethod();
         if (nmethod != null) {
            buf.append("<h3>Compiled Code</h3>");
            buf.append(genNMethodLink(nmethod));
         }

         boolean hasThrows = method.hasCheckedExceptions();
         ConstantPool cpool = ((InstanceKlass) method.getMethodHolder()).getConstants();
         if (hasThrows) {
            buf.append("<h3>Checked Exception(s)</h3>");
            CheckedExceptionElement[] exceptions = method.getCheckedExceptions();
            buf.append("<ul>");
            for (int exp = 0; exp < exceptions.length; exp++) {
               short cpIndex = (short) exceptions[exp].getClassCPIndex();
               Oop obj = cpool.getObjAt(cpIndex);
               buf.append("<li>");
               if (obj instanceof Symbol) {
                  buf.append(((Symbol)obj).asString().replace('/', '.'));
               } else {
                  buf.append(genKlassLink((InstanceKlass)obj));
               } 
               buf.append("</li>");
            }
            buf.append("</ul>");
         }

         if (method.isNative() || method.isAbstract()) {
           buf.append(genHTMLEpilogue());
           return buf.toString();
         }
 
         buf.append("<h3>Bytecode</h3>");
         BytecodeDisassembler disasm = createBytecodeDisassembler(method);
         final boolean hasLineNumbers = method.hasLineNumberTable();
         disasm.decode(new BytecodeVisitor() {
                          private Method method;
                          public void prologue(Method m) {
                             method = m;
                             buf.append("<table border='0'>");
                             buf.append("<tr>");
                             if (hasLineNumbers) {
                                buf.append("<th>line</th>");
                             }
                             buf.append("<th>bci");
                             buf.append(spaces);
                             buf.append("</th>");
                             buf.append("<th>bytecode</th>");
                             buf.append("</tr>");
                          }

                          public void visit(Bytecode instr) {
                             int curBci = instr.bci();
                             buf.append("<tr>");
                             if (hasLineNumbers) {
                                int lineNumber = method.getLineNumberFromBCI(curBci);
                                buf.append("<td>");
                                buf.append(new Integer(lineNumber).toString());
                                buf.append("</td>");
                             } 
                             buf.append("<td>");
                             buf.append(new Integer(curBci).toString());
                             buf.append(spaces);
                             buf.append("</td>");

                             buf.append("<td>");
                             String instrStr = escapeHTMLSpecialChars(instr.toString());

                             if (instr instanceof BytecodeNew) {
                                BytecodeNew newBytecode = (BytecodeNew) instr;
                                InstanceKlass klass = newBytecode.getNewKlass();
                                if (klass != null) {
                                   buf.append("<a href='");
                                   buf.append(genKlassHref(klass));
                                   buf.append("'>");
                                   buf.append(instrStr);
                                   buf.append("</a>");
                                } else {
                                   buf.append(instrStr);
                                }
                             } else if(instr instanceof BytecodeInvoke) {
                                BytecodeInvoke invokeBytecode = (BytecodeInvoke) instr;
                                Method m = invokeBytecode.getInvokedMethod();
                                if (m != null) {
                                   buf.append("<a href='");
                                   buf.append(genMethodHref(m));
                                   buf.append("'>");
                                   buf.append(instrStr);
                                   buf.append("</a> of <a href='");
                                   InstanceKlass klass = (InstanceKlass) m.getMethodHolder();
                                   buf.append(genKlassHref(klass));
                                   buf.append("'>");
                                   buf.append(genKlassTitle(klass));
                                   buf.append("</a>");
                                } else {
                                   buf.append(instrStr);
                                }   
                             } else if (instr instanceof BytecodeGetPut) {
                                BytecodeGetPut getPut = (BytecodeGetPut) instr;
                                sun.jvm.hotspot.oops.Field f = getPut.getField();
                                buf.append(instrStr);
                                if (f != null) {
                                   InstanceKlass klass = f.getFieldHolder();
                                   buf.append(" of <a href='");
                                   buf.append(genKlassHref(klass));
                                   buf.append("'>");
                                   buf.append(genKlassTitle(klass));
                                   buf.append("</a>");
                                } 
                             } else {
                                buf.append(instrStr);
                             }
                             buf.append("</td>");
                             buf.append("</tr>");
                          }

                          public void epilogue() {
                             buf.append("</table>");
                          }
                       });

         // display exception table for this method
         TypeArray exceptionTable = method.getExceptionTable();
         // exception table is 4 tuple array of shorts
         int numEntries = (int)exceptionTable.getLength() / 4;
         if (numEntries != 0) {
            buf.append("<h4>Exception Table</h4>");
            buf.append("<table border='1'>");
            buf.append("<tr>");
            buf.append("<th>start bci</th>");
            buf.append("<th>end bci</th>");
            buf.append("<th>handler bci</th>");
            buf.append("<th>catch type</th>");
            buf.append("</tr>");

            for (int e = 0; e < numEntries; e += 4) {
               buf.append("<tr>");
               buf.append("<td>");
               buf.append(new Integer(exceptionTable.getIntAt(e)).toString());
               buf.append("</td>");
               buf.append("<td>");
               buf.append(new Integer(exceptionTable.getIntAt(e + 1)).toString());
               buf.append("</td>");
               buf.append("<td>");
               buf.append(new Integer(exceptionTable.getIntAt(e + 2)).toString());
               buf.append("</td>");
               buf.append("<td>");
               short cpIndex = (short) exceptionTable.getIntAt(e + 3);
               Oop obj = cpool.getObjAt(cpIndex);
               if (obj instanceof Symbol) {
                  buf.append(((Symbol)obj).asString().replace('/', '.'));
               } else {
                  buf.append(genKlassLink((InstanceKlass)obj));
               }
               buf.append("</td>");
               buf.append("</tr>"); 
            }

            buf.append("</table>");
         }

         // display constant pool hyperlink
         buf.append("<h3>View Constant Pool</h3>");
         buf.append(genConstantPoolLink(cpool));
         buf.append(genHTMLEpilogue());
         return buf.toString();
      } catch (Exception exp) {
         return genHTMLErrorMessage(exp);
      }
   }

   protected Disassembler createDisassembler(long startPc, byte[] code) {
      return getCPUHelper().createDisassembler(startPc, code);
   }

   protected SymbolFinder createSymbolFinder() {
      return new DummySymbolFinder();
   }

   // genHTML for a given address. Address may be a PC or
   // methodOop or klassOop.

   public String genHTMLForAddress(String addrStr) {
      return genHTML(parseAddress(addrStr));
   }

   public String genHTML(sun.jvm.hotspot.debugger.Address pc) {
      NMethod nmethod = null;
      try {
         nmethod = (NMethod)VM.getVM().getCodeCache().findNMethod(pc);
      } catch (Exception exp) {
         // ignore
      }

      if (nmethod != null) {
         return genHTML(nmethod);
      }

      // did not find nmethod. try methodOop and klassOop.
      try {
         Oop obj = getOopAtAddress(pc);
         if (obj != null) {
            if (obj instanceof Method) {
               return genHTML((Method) obj);
            } else if (obj instanceof InstanceKlass) {
               return genHTML((InstanceKlass) obj);
            }
         }
      } catch (Exception exp) {
         // ignore
      }

      // didn't find method/klass oop. may be interpreter code.
      Interpreter interp = VM.getVM().getInterpreter();
      if (interp.contains(pc)) {
         InterpreterCodelet codelet = interp.getCodeletContaining(pc);
         return genHTML(codelet);
      }

      // didn't find interpreter code. raw disassembly.
      return genHTMLForRawDisassembly(pc, null);
   }

   protected byte[] readBuffer(sun.jvm.hotspot.debugger.Address addr, int size) {
      byte[] buf = new byte[size];
      for (int b = 0; b < size; b++) {
         buf[b] = (byte) addr.getJByteAt(b);
      }
      return buf;
   }

   protected String genHTMLForRawDisassembly(sun.jvm.hotspot.debugger.Address startPc,
                                             String prevPCs) {
      try {
         return genHTMLForRawDisassembly(startPc, prevPCs, readBuffer(startPc, NATIVE_CODE_SIZE));
      } catch (Exception exp) {
         return genHTMLErrorMessage(exp);
      }
   }
   
   protected String genPCHref(long targetPc) {
      return genBaseHref() + "pc=0x" + Long.toHexString(targetPc);
   }

   protected String genMultPCHref(String pcs) {
      StringBuffer buf = new StringBuffer(genBaseHref());
      buf.append("pc_multiple=");
      buf.append(pcs);
      return buf.toString();
   }

   protected String genPCHref(long currentPc, sun.jvm.hotspot.asm.Address addr) {
      String href = null;
      if (addr instanceof PCRelativeAddress) {
         PCRelativeAddress pcRelAddr = (PCRelativeAddress) addr;
         href = genPCHref(currentPc + pcRelAddr.getDisplacement());
      } else if(addr instanceof DirectAddress) {
         href =  genPCHref(((DirectAddress) addr).getValue());
      }

      return href;
   }

   class RawCodeVisitor implements InstructionVisitor {
      private int instrSize = 0;
      private StringBuffer buf;
      private SymbolFinder symFinder = createSymbolFinder();

      RawCodeVisitor(StringBuffer buf) {
         this.buf = buf;
      }

      public int getInstructionSize() {
         return  instrSize;
      }

      public void prologue() {
      }

      public void visit(long currentPc, Instruction instr) {
         String href = null;
          if (instr.isCall()) {
             CallInstruction call = (CallInstruction) instr;
             sun.jvm.hotspot.asm.Address addr = call.getBranchDestination();
             href = genPCHref(currentPc, addr);
          }

          instrSize += instr.getSize();
          buf.append("0x");
          buf.append(Long.toHexString(currentPc));
          buf.append(':');
          buf.append(tab);

          if (href != null) {
             buf.append("<a href='");
             buf.append(href);
             buf.append("'>");
          }

          buf.append(instr.asString(currentPc, symFinder));

          if (href != null) {
             buf.append("</a>");
          }
          buf.append("<br>");
      }
         
      public void epilogue() {
      }
   };

   protected String genHTMLForRawDisassembly(sun.jvm.hotspot.debugger.Address addr, 
                                             String prevPCs,
                                             byte[] code) {
      try {
         long startPc = addressToLong(addr);
         Disassembler disasm = createDisassembler(startPc, code);
         final StringBuffer buf = new StringBuffer(); 
         buf.append(genHTMLPrologue("Disassembly @0x" + Long.toHexString(startPc)));

         if (prevPCs != null) {
            buf.append("<p>");
            buf.append("<a href='");
            buf.append(genMultPCHref(prevPCs));
            buf.append("'>");
            buf.append("show previous code ..");
            buf.append("</a>");
            buf.append("</p>");
         }

         buf.append("<h3>Code</h3>");
         RawCodeVisitor visitor = new RawCodeVisitor(buf);
         disasm.decode(visitor);

         buf.append("<p>");
         buf.append("<a href='");
         StringBuffer tmpBuf = new StringBuffer();
         tmpBuf.append("0x");
         tmpBuf.append(Long.toHexString(startPc + visitor.getInstructionSize()).toString());
         tmpBuf.append(",0x");
         tmpBuf.append(Long.toHexString(startPc));
         if (prevPCs != null) {
            tmpBuf.append(',');
            tmpBuf.append(prevPCs);
         }
         buf.append(genMultPCHref(tmpBuf.toString()));
         buf.append("'>");
         buf.append("show more code ..");
         buf.append("</a>");
         buf.append("</p>");

         buf.append(genHTMLEpilogue());
         return buf.toString();
      } catch (Exception exp) {
         return genHTMLErrorMessage(exp);
      }
   }  
 
   protected Map getSafepoints(NMethod nmethod) {
      Map safepoints = new HashMap(); // Map<pc, PcDesc>

      TypeDataBase db = VM.getVM().getTypeDataBase();
      long pcDescSize = db.lookupType("PcDesc").getSize();
      sun.jvm.hotspot.debugger.Address p = null;
      for (p = nmethod.scopesPCsBegin(); p.lessThan(nmethod.scopesPCsEnd()); 
                       p = p.addOffsetTo(pcDescSize)) {
         PCDesc pcDesc = new PCDesc(p);
         sun.jvm.hotspot.debugger.Address pc = pcDesc.getRealPC(nmethod);
         safepoints.put(new Long(addressToLong(pc)), pcDesc);
      }

      return safepoints;
   }

   protected String genSafepointInfo(NMethod nm, PCDesc pcDesc) {
      ScopeDesc sd = nm.getScopeDescAt(pcDesc.getRealPC(nm), pcDesc.isAtCall());
      StringBuffer buf = new StringBuffer();
      StringBuffer tabs = new StringBuffer();
 
      buf.append("<pre>");
      while (sd != null) {
         buf.append(tabs);
         Method m = sd.getMethod();
         buf.append(genMethodAndKlassLink(m));
         int bci = sd.getBCI();
         buf.append(" @ bci = ");
         buf.append(new Integer(bci).toString());

         int line = m.getLineNumberFromBCI(bci);
         if (line != -1) {
            buf.append(", line = ");
            buf.append(new Integer(line).toString());
         }
             
         List locals = sd.getLocals();
         if (locals != null) {
            buf.append("<br>");
            buf.append(tabs);
            buf.append(genHTMLForLocals(sd, locals));
         }

         List expressions = sd.getExpressions();
         if (expressions != null) {
            buf.append("<br>");
            buf.append(tabs);
            buf.append(genHTMLForExpressions(sd, expressions));
         }

         List monitors = sd.getMonitors();
         if (monitors != null) {
            buf.append("<br>");
            buf.append(tabs);
            buf.append(genHTMLForMonitors(sd, monitors));
         }

         tabs.append(tab);
         sd = sd.sender();
         buf.append("<br>");
      }
      buf.append("</pre>");
      buf.append(genOopMapInfo(nm, pcDesc));
      
      return buf.toString();
   }

   protected String genHTMLForOopMap(OopMap map) {
      final int stack0 = SharedInfo.getStack0().getValue();
      StringBuffer buf = new StringBuffer();

      final class OopMapValueIterator {
         final StringBuffer iterate(OopMapStream oms, String type, boolean printContentReg) {
            StringBuffer tmpBuf = new StringBuffer();
            boolean found = false; 
            tmpBuf.append("<tr>");
            tmpBuf.append("<td>");
            tmpBuf.append(type);
            tmpBuf.append("</td><td>");
            for (; ! oms.isDone(); oms.next()) {
               OopMapValue omv = oms.getCurrent();
               if (omv == null) {
                  continue;
               }
               found = true;
               VMReg vmReg = omv.getReg();
               int reg = vmReg.getValue();
               if (reg < stack0) {
                  Register r = getCPUHelper().getIntegerRegister(reg);
                  if (r != null) {
                     tmpBuf.append(r.toString());
                  } else {
                     tmpBuf.append("<invalid reg>");
                  }
               } else {
                  tmpBuf.append('[');
                  tmpBuf.append(new Integer(reg - stack0).toString());
                  tmpBuf.append(']');
               }
               if (printContentReg) {
                  tmpBuf.append(" = ");
                  int contentReg = omv.getContentReg().getValue();
                  Register cr = getCPUHelper().getIntegerRegister(contentReg);
                  if (cr != null) {
                     tmpBuf.append(cr.toString());
                  } else {
                     tmpBuf.append("<invalid reg>");
                  }
               }
               tmpBuf.append(spaces);
            }
            tmpBuf.append("</td></tr>");
            return found ? tmpBuf : new StringBuffer();
         }
      }

      buf.append("<table border='0'>");

      OopMapValueIterator omvIterator = new OopMapValueIterator();
      OopMapStream oms = new OopMapStream(map, OopMapValue.OopTypes.OOP_VALUE);
      buf.append(omvIterator.iterate(oms, "Oop:", false));

      oms = new OopMapStream(map, OopMapValue.OopTypes.VALUE_VALUE);
      buf.append(omvIterator.iterate(oms, "Value:", false));

      oms = new OopMapStream(map, OopMapValue.OopTypes.DEAD_VALUE);
      buf.append(omvIterator.iterate(oms, "Dead:", false));

      oms = new OopMapStream(map, OopMapValue.OopTypes.CALLEE_SAVED_VALUE);
      buf.append(omvIterator.iterate(oms, "Callee saved:",  true));
    
      oms = new OopMapStream(map, OopMapValue.OopTypes.DERIVED_OOP_VALUE);
      buf.append(omvIterator.iterate(oms, "Derived oop:", true));

      buf.append("</table>");
      return buf.toString();
   }


   protected String genOopMapInfo(NMethod nmethod, PCDesc pcDesc) {
      OopMapSet mapSet = nmethod.getOopMaps();
      boolean atCall = pcDesc.isAtCall(); 
      int pcOffset = pcDesc.getPCOffset();
      OopMap map = mapSet.findMapAtOffset(pcOffset, atCall, true, VM.getVM().isDebugging());
      if (map == null) {
         throw new IllegalArgumentException("no oopmap at safepoint!");
      }

      StringBuffer buf = new StringBuffer();
      buf.append("<h4>oopmap - at call = ");
      buf.append(atCall? "true" : "false");
      buf.append("</h4>");
      buf.append(genHTMLForOopMap(map));

      // look for oopmap at !atCall too
      try {
         map = mapSet.findMapAtOffset(pcOffset, !atCall, true, VM.getVM().isDebugging());
      } catch (Exception exp) {
         // ignore
      }

      if ((map != null) && (map.isAtCall() == !atCall) && (map.getOffset() == pcOffset)) {
         buf.append("<h4>oopmap - at call? = ");
         buf.append(!atCall? "true" : "false");
         buf.append("</h4>");
         buf.append(genHTMLForOopMap(map));
      }

      return buf.toString();
   }

   protected String locationAsString(Location loc) {
      StringBuffer buf = new StringBuffer();
      if (loc.isIllegal()) {
         buf.append("illegal");
      } else {
         Location.Where  w  = loc.getWhere();
         Location.Type type = loc.getType();

         if (w == Location.Where.ON_STACK) {
            buf.append("stack[" + loc.getStackOffset() + "]");
         } else if (w == Location.Where.IN_REGISTER) {
            boolean isFloat = (type == Location.Type.FLOAT_IN_DBL ||
                               type == Location.Type.DBL);
            int regNum = loc.getRegisterNumber();
            Register reg = null;
            if (isFloat) {
               reg = getCPUHelper().getFloatRegister(regNum);
            } else {
              reg = getCPUHelper().getIntegerRegister(regNum);
            }
            if (reg != null) {
               buf.append(reg.toString());
            } else {
               buf.append("reg #");
               buf.append(new Integer(regNum).toString());
            }
         }

         buf.append(", ");
         if (type == Location.Type.NORMAL) {
            buf.append("normal");
         } else if (type == Location.Type.OOP) {
            buf.append("oop");
         } else if (type == Location.Type.LNG) {
            buf.append("long");
         } else if (type == Location.Type.FLOAT_IN_DBL) {
            buf.append("float");
         } else if (type == Location.Type.DBL) {
            buf.append("double");
         } else if (type == Location.Type.ADDR) {
            buf.append("address");
         } else {
            buf.append("Wrong location type " + type.getValue());
         }
      }
      return buf.toString();
   }

   private String scopeValueAsString(ScopeValue sv) {
      StringBuffer buf = new StringBuffer();
      if (sv.isConstantInt()) {
         buf.append("int ");
         ConstantIntValue intValue = (ConstantIntValue) sv;
         buf.append(new Integer(intValue.getValue()).toString());
      } else if (sv.isConstantLong()) {
         buf.append("long ");
         ConstantLongValue longValue = (ConstantLongValue) sv;
         buf.append(new Long(longValue.getValue()).toString());
         buf.append("L");
      } else if (sv.isConstantDouble()) {
         buf.append("double ");
         ConstantDoubleValue dblValue = (ConstantDoubleValue) sv;
         buf.append(new Double(dblValue.getValue()).toString());
         buf.append("D");
      } else if (sv.isConstantOop()) {
         buf.append("oop ");
         ConstantOopReadValue oopValue = (ConstantOopReadValue) sv;
         OopHandle oopHandle = oopValue.getValue();
         if (oopHandle != null) {
            buf.append(oopHandle.toString());
         } else {
            buf.append("null");
         }
      } else if (sv.isLocation()) {
         buf.append("location ");
         LocationValue lvalue = (LocationValue) sv;
         Location loc = lvalue.getLocation();
         if (loc != null) {
            buf.append(locationAsString(loc));
         } else {
            buf.append("null");
         }
      }
      return buf.toString();
   }

   protected String genHTMLForScopeValues(ScopeDesc sd, boolean locals, List values) {
      int length = values.size();
      StringBuffer buf = new StringBuffer();
      buf.append(locals? "locals " : "expressions ");
      for (int i = 0; i < length; i++) {
         ScopeValue sv = (ScopeValue) values.get(i);
         if (sv == null) {
            continue;
         }
         buf.append('(');
         if (locals) {
            Symbol name = sd.getMethod().getLocalVariableName(sd.getBCI(), i);
            if (name != null) {
               buf.append("'");
               buf.append(name.asString());
               buf.append('\'');
            } else {
               buf.append("[");
               buf.append(new Integer(i).toString());
               buf.append(']');
            }
         } else {
            buf.append("[");
            buf.append(new Integer(i).toString());
            buf.append(']');
         }

         buf.append(", ");
         buf.append(scopeValueAsString(sv));
         buf.append(") ");
      }
      
      return buf.toString();
   }

   protected String genHTMLForLocals(ScopeDesc sd, List locals) {
      return genHTMLForScopeValues(sd, true, locals);
   }

   protected String genHTMLForExpressions(ScopeDesc sd, List expressions) {
      return genHTMLForScopeValues(sd, false, expressions);
   }

   protected String genHTMLForMonitors(ScopeDesc sd, List monitors) {
      int length = monitors.size();
      StringBuffer buf = new StringBuffer(); 
      buf.append("monitors ");
      for (int i = 0; i < length; i++) {
         MonitorValue mv = (MonitorValue) monitors.get(i);
         if (mv == null) {
            continue;
         }
         buf.append("(owner = ");
         ScopeValue owner = mv.owner(); 
         if (owner != null) {
            buf.append(scopeValueAsString(owner));
         } else {
            buf.append("null");
         }
         buf.append(", lock = ");

         Location loc = mv.basicLock();
         if (loc != null) {
            buf.append(locationAsString(loc));
         } else {
            buf.append("null");
         }
         buf.append(") ");
      }
      return buf.toString();
   }

   public String genHTML(final NMethod nmethod) { 
      try {
         final StringBuffer buf = new StringBuffer();
         buf.append(genHTMLPrologue(genNMethodTitle(nmethod)));
         buf.append("<h3>Method</h3>");
         buf.append(genMethodAndKlassLink(nmethod.getMethod()));

         buf.append("<h3>Compiled Code</h3>");
         sun.jvm.hotspot.debugger.Address codeBegin = nmethod.codeBegin();
         final int codeSize = nmethod.codeSize();
         final long startPc = addressToLong(codeBegin);
         final byte[] code = new byte[codeSize];
         for (int i=0; i < code.length; i++)
            code[i] = codeBegin.getJByteAt(i);

         final Map safepoints = getSafepoints(nmethod);

         final SymbolFinder symFinder = createSymbolFinder(); 
         final Disassembler disasm = createDisassembler(startPc, code);
         class NMethodVisitor implements InstructionVisitor {
            public void prologue() {
            }

            public void visit(long currentPc, Instruction instr) {
               String href = null;
               if (instr.isCall()) {
                  CallInstruction call = (CallInstruction) instr;
                  sun.jvm.hotspot.asm.Address addr = call.getBranchDestination();
                  href = genPCHref(currentPc, addr);
               }
                             
               PCDesc pcDesc = (PCDesc) safepoints.get(new Long(currentPc));
               boolean isSafepoint = (pcDesc != null);
 
               buf.append("0x");
               buf.append(Long.toHexString(currentPc));
               buf.append(':');
               buf.append(tab);

               if (href != null) {
                  buf.append("<a href='");
                  buf.append(href);
                  buf.append("'>");
               }
               buf.append(instr.asString(currentPc, symFinder));
               if (href != null) {
                  buf.append("</a>");
               }

               if (isSafepoint) {
                  buf.append(genSafepointInfo(nmethod, pcDesc));
               }
               buf.append("<br>");
            }

            public void epilogue() {
            }
         };

         disasm.decode(new NMethodVisitor());

         sun.jvm.hotspot.debugger.Address expBegin = nmethod.exceptionBegin();
         if (expBegin != null) {
            sun.jvm.hotspot.debugger.Address expEnd   = nmethod.exceptionEnd();
            buf.append("<h3>Exceptions</h3>");
            long expStartPc = addressToLong(expBegin);
            long expEndPc = addressToLong(expEnd);
            int range = (int) (expEndPc - expStartPc);
            byte[] expCode = readBuffer(expBegin, range);
            Disassembler disasm1 = createDisassembler(expStartPc, expCode);
            disasm1.decode(new NMethodVisitor());
         }

         sun.jvm.hotspot.debugger.Address stubBegin = nmethod.stubBegin();
         if (stubBegin != null) {
            sun.jvm.hotspot.debugger.Address stubEnd   = nmethod.stubEnd();
            buf.append("<h3>Stub</h3>");
            long stubStartPc = addressToLong(stubBegin);
            long stubEndPc = addressToLong(stubEnd);
            int range = (int) (stubEndPc - stubStartPc);
            byte[] stubCode = readBuffer(stubBegin, range);
            Disassembler disasm2 = createDisassembler(stubStartPc, stubCode);
            disasm2.decode(new NMethodVisitor());
         }
         buf.append(genHTMLEpilogue());
         return buf.toString();
      } catch (Exception exp) {
         return genHTMLErrorMessage(exp);
      }
   }

   protected String genInterpreterCodeletTitle(InterpreterCodelet codelet) {
      StringBuffer buf = new StringBuffer();
      buf.append("Interpreter codelet [");
      buf.append(codelet.codeBegin().toString());
      buf.append(',');
      buf.append(codelet.codeEnd().toString());
      buf.append(") - ");
      buf.append(codelet.getDescription());
      if (codelet.isSafepointSafe()) {
         buf.append(" - safepoint safe");
      } else {
         buf.append(" - not safepoint safe");
      }

      return buf.toString();
   }

   protected String genInterpreterCodeletLinkPageHref(StubQueue stubq) {
      return genBaseHref() + "interp_codelets";
   }

   public String genInterpreterCodeletLinksPage() {
      StringBuffer buf = new StringBuffer();
      buf.append(genHTMLPrologue("Interpreter Codelets"));
      buf.append("<ul>");

      Interpreter interp = VM.getVM().getInterpreter();
      StubQueue code = interp.getCode();
      InterpreterCodelet stub = (InterpreterCodelet) code.getFirst();
      while (stub != null) {
         buf.append("<li>");
         sun.jvm.hotspot.debugger.Address addr = stub.codeBegin();
         buf.append("<a href='");
         buf.append(genPCHref(addressToLong(addr)));
         buf.append("'>");
         buf.append(stub.getDescription());
         buf.append(" @");
         buf.append(addr.toString());
         buf.append("</a></li>");
         stub = (InterpreterCodelet) code.getNext(stub);
      }

      buf.append("</ul>");
      buf.append(genHTMLEpilogue());
      return buf.toString();
   }

   public String genHTML(InterpreterCodelet codelet) {
      StringBuffer buf = new StringBuffer();
      buf.append(genHTMLPrologue(genInterpreterCodeletTitle(codelet)));
      Interpreter interp = VM.getVM().getInterpreter();
      StubQueue stubq = interp.getCode();

      buf.append("<h3>");
      buf.append("<a href='");
      buf.append(genInterpreterCodeletLinkPageHref(stubq));
      buf.append("'>View links for all codelets</a>");
      buf.append("</h3>");

      Stub prev = stubq.getPrev(codelet);
      if (prev != null) {
         buf.append("<h3>");
         buf.append("<a href='");
         String addrStr = prev.codeBegin().toString();
         buf.append(genPCHref(Long.parseLong(addrStr.substring(2), 16)));
         buf.append("'>View Previous Codelet</a>");
         buf.append("</h3>");
      }

      buf.append("<h3>Code</h3>");
      long stubStartPc = addressToLong(codelet.codeBegin());
      long stubEndPc = addressToLong(codelet.codeEnd());
      int range = (int) (stubEndPc - stubStartPc);
      byte[] stubCode = readBuffer(codelet.codeBegin(), range);
      Disassembler disasm = createDisassembler(stubStartPc, stubCode);
      disasm.decode(new RawCodeVisitor(buf));
    
 
      Stub next = stubq.getNext(codelet);
      if (next != null) {
         buf.append("<h3>");
         buf.append("<a href='");
         String addrStr = next.codeBegin().toString();
         buf.append(genPCHref(Long.parseLong(addrStr.substring(2), 16)));
         buf.append("'>View Next Codelet</a>");
         buf.append("</h3>");
      }

      buf.append(genHTMLEpilogue());
      return buf.toString();
   }

   protected String genDumpKlassesTitle(InstanceKlass[] klasses) {
      return (klasses.length == 1) ? "Create .class for this class" 
                                   : "Create .class for all classes";
   }

   protected String genDumpKlassesHref(InstanceKlass[] klasses) {
      StringBuffer buf = new StringBuffer(genBaseHref());
      buf.append("jcore_multiple=");
      for (int k = 0; k < klasses.length; k++) {
         buf.append(klasses[k].getHandle().toString());
         buf.append(',');
      }
      return buf.toString();
   }

   protected String genDumpKlassesLink(InstanceKlass[] klasses) {
      StringBuffer buf = new StringBuffer();
      buf.append("<a href='");
      buf.append(genDumpKlassesHref(klasses));
      buf.append("'>");
      buf.append(genDumpKlassesTitle(klasses));
      buf.append("</a>");
      return buf.toString();
   }

   public String genHTMLForKlassNames(InstanceKlass[] klasses) {
      try {
         StringBuffer buf = new StringBuffer();
         buf.append(genHTMLPrologue());
         buf.append("<h3>");
         buf.append(genDumpKlassesLink(klasses));
         buf.append("</h3>");

         buf.append(genHTMLListForKlassNames(klasses));
         buf.append(genHTMLEpilogue());
         return buf.toString();
      } catch (Exception exp) {
         return genHTMLErrorMessage(exp);
      }
   }

   protected String genHTMLListForKlassNames(InstanceKlass[] klasses) {
      final StringBuffer buf = new StringBuffer();
      buf.append("<table border='0'>");
      for (int i = 0; i < klasses.length; i++) {
         InstanceKlass ik = klasses[i];
         buf.append("<tr><td>");
         buf.append(genKlassLink(ik));
         buf.append("</td></tr>");
      }
        
      buf.append("</table>");
      return buf.toString();
   }

   public String genHTMLForMethodNames(InstanceKlass klass) {
      try {
         StringBuffer buf = new StringBuffer();
         buf.append(genHTMLPrologue());
         buf.append(genHTMLListForMethods(klass));
         buf.append(genHTMLEpilogue());
         return buf.toString();
      } catch (Exception exp) {
         return genHTMLErrorMessage(exp);
      }
   }

   protected String genHTMLListForMethods(InstanceKlass klass) {
      StringBuffer buf = new StringBuffer();
      ObjArray methods = klass.getMethods();
      int numMethods = (int) methods.getLength();
      if (numMethods != 0) {
         buf.append("<h3>Methods</h3>");
         buf.append("<ul>");
         for (int m = 0; m < numMethods; m++) {
            Method mtd = (Method) methods.getObjAt(m);
            buf.append("<li>");
            buf.append(genMethodLink(mtd));
            buf.append(";</li>");
         }
         buf.append("</ul>");
      }
      return buf.toString();
   }

   protected String genHTMLListForInterfaces(InstanceKlass klass) {
      try {
         StringBuffer buf = new StringBuffer();
         ObjArray interfaces = klass.getLocalInterfaces();
         int numInterfaces = (int) interfaces.getLength();
         if (numInterfaces != 0) {
            buf.append("<h3>Interfaces</h3>");
            buf.append("<ul>");
            for (int i = 0; i < numInterfaces; i++) {
               InstanceKlass inf = (InstanceKlass) interfaces.getObjAt(i);
               buf.append("<li>");
               buf.append(genKlassLink(inf));
               buf.append("</li>");
            }
            buf.append("</ul>");
         }
         return buf.toString();
      } catch (Exception exp) {
         return genHTMLErrorMessage(exp);
      }
   }

   protected String genFieldModifierString(AccessFlags acc) {
      StringBuffer buf = new StringBuffer();
      if (acc.isPrivate()) {
         buf.append("private ");
      } else if (acc.isProtected()) {
         buf.append("protected ");
      } else if (acc.isPublic()) {
         buf.append("public ");
      }

      if (acc.isStatic()) {
         buf.append("static ");
      }

      if (acc.isFinal()) {
         buf.append("final ");
      }
      if (acc.isVolatile()) {
         buf.append("volatile ");
      }
      if (acc.isTransient()) {
         buf.append("transient ");
      }
      return buf.toString();
   }

   public String genHTMLForFieldNames(InstanceKlass klass) {
      try {
         StringBuffer buf = new StringBuffer();
         buf.append(genHTMLPrologue());
         buf.append(genHTMLListForFields(klass));
         buf.append(genHTMLEpilogue());
         return buf.toString();
      } catch (Exception exp) {
         return genHTMLErrorMessage(exp);
      }
   }

   protected String genHTMLListForFields(InstanceKlass klass) {
      StringBuffer buf = new StringBuffer();
      TypeArray fields = klass.getFields();
      int numFields = (int) fields.getLength();
      ConstantPool cp = klass.getConstants();
      if (numFields != 0) {
         buf.append("<h3>Fields</h3>");
         buf.append("<ul>");
         for (int f = 0; f < numFields; f += InstanceKlass.NEXT_OFFSET) {
           int nameIndex = fields.getShortAt(f + InstanceKlass.NAME_INDEX_OFFSET);
           int sigIndex  = fields.getShortAt(f + InstanceKlass.SIGNATURE_INDEX_OFFSET);
           Symbol f_name = cp.getSymbolAt(nameIndex);
           Symbol f_sig  = cp.getSymbolAt(sigIndex);
           AccessFlags acc = new AccessFlags(fields.getShortAt(f + InstanceKlass.ACCESS_FLAGS_OFFSET));

           buf.append("<li>");
           buf.append(genFieldModifierString(acc));
           buf.append(' ');
           StringBuffer sigBuf = new StringBuffer();
           new SignatureConverter(f_sig, sigBuf).dispatchField();
           buf.append(sigBuf.toString().replace('/', '.'));
           buf.append(' ');
           buf.append(f_name.asString());
           buf.append(";</li>");
         }
         buf.append("</ul>");
      }
      return buf.toString();
   }

   protected String genKlassHierarchyHref(InstanceKlass klass) {
      return genBaseHref() + "hierarchy=" + klass.getHandle();
   }

   protected String genKlassHierarchyTitle(InstanceKlass klass) {
      StringBuffer buf = new StringBuffer();
      buf.append("Class Hierarchy of ");
      buf.append(genKlassTitle(klass));
      return buf.toString();
   }

   protected String genKlassHierarchyLink(InstanceKlass klass) {
      StringBuffer buf = new StringBuffer();
      buf.append("<a href='");
      buf.append(genKlassHierarchyHref(klass));
      buf.append("'>");
      buf.append(genKlassHierarchyTitle(klass));
      buf.append("</a>");
      return buf.toString();
   }

   protected String genHTMLListForSubKlasses(InstanceKlass klass) {
      StringBuffer buf = new StringBuffer();
      Klass subklass = klass.getSubklassKlass();
      if (subklass != null) {
         buf.append("<ul>");
         while (subklass != null) {
            if (subklass instanceof InstanceKlass) {
               buf.append("<li>");
               buf.append(genKlassLink((InstanceKlass)subklass));
               buf.append("</li>");
            } 
            subklass = subklass.getNextSiblingKlass();
         }
         buf.append("</ul>");
      }
      return buf.toString();
   }

   public String genHTMLForKlassHierarchy(InstanceKlass klass) {
      StringBuffer buf = new StringBuffer();
      buf.append(genHTMLPrologue(genKlassHierarchyTitle(klass)));

      buf.append("<pre>");
      buf.append(genKlassLink(klass));
      buf.append("<br>");
      StringBuffer tabs = new StringBuffer(tab);
      InstanceKlass superKlass = klass;
      while ( (superKlass = (InstanceKlass) superKlass.getSuper()) != null ) { 
         buf.append(tabs);
         buf.append(genKlassLink(superKlass));
         tabs.append(tab);
         buf.append("<br>");
      }
      buf.append("</pre>");

      // generate subklass list
      Klass subklass = klass.getSubklassKlass();
      if (subklass != null) {
         buf.append("<h3>Direct Subclasses</h3>");
         buf.append(genHTMLListForSubKlasses(klass));
      }

      buf.append(genHTMLEpilogue());
      return buf.toString();
   }

   protected String genDumpKlassHref(InstanceKlass klass) {
      return genBaseHref() + "jcore=" + klass.getHandle();
   }

   protected String genDumpKlassLink(InstanceKlass klass) {
      StringBuffer buf = new StringBuffer();
      buf.append("<a href='");
      buf.append(genDumpKlassHref(klass));
      buf.append("'>");
      buf.append("Create .class File");
      buf.append("</a>");
      return buf.toString();
   }

   public String genHTML(InstanceKlass klass) {
      StringBuffer buf = new StringBuffer();
      buf.append(genHTMLPrologue(genKlassTitle(klass)));
      InstanceKlass superKlass = (InstanceKlass) klass.getSuper();

      // super class tree and subclass list
      buf.append("<h3>");
      buf.append("<a href='");
      buf.append(genKlassHierarchyHref(klass));
      buf.append("'>View Class Hierarchy</a>");
      buf.append("</h3>");

      // jcore - create .class link
      buf.append("<h3>");
      buf.append(genDumpKlassLink(klass));
      buf.append("</h3>");

      // super class
      if (superKlass != null) {
         buf.append("<h3>Super Class</h3>");
         buf.append(genKlassLink(superKlass));
      }

      // interfaces
      buf.append(genHTMLListForInterfaces(klass));

      // fields
      buf.append(genHTMLListForFields(klass));

      // methods
      buf.append(genHTMLListForMethods(klass));

      // constant pool link
      buf.append("<h3>View Constant Pool</h3>");
      buf.append(genConstantPoolLink(klass.getConstants()));

      buf.append(genHTMLEpilogue());
      return buf.toString();
   }

   protected sun.jvm.hotspot.debugger.Address parseAddress(String address) {
      VM vm = VM.getVM();
      sun.jvm.hotspot.debugger.Address addr = vm.getDebugger().parseAddress(address);
      return addr;
   }

   protected long addressToLong(sun.jvm.hotspot.debugger.Address addr) {
      String addrStr = addr.toString();
      return Long.parseLong(addrStr.substring(2), 16);
   }

   protected Oop getOopAtAddress(sun.jvm.hotspot.debugger.Address addr) {
      OopHandle oopHandle = addr.addOffsetToAsOopHandle(0);
      return VM.getVM().getObjectHeap().newOop(oopHandle);
   }

   protected Oop getOopAtAddress(String address) {
      sun.jvm.hotspot.debugger.Address addr = parseAddress(address);
      return getOopAtAddress(addr);
   }

   private void dumpKlass(InstanceKlass kls) throws IOException {
      String klassName = kls.getName().asString();
      klassName = klassName.replace('/', File.separatorChar);
      int index = klassName.lastIndexOf(File.separatorChar);
      File dir = null;
      if (index != -1) {
        String dirName = klassName.substring(0, index);
        dir =  new File(DUMP_KLASS_OUTPUT_DIR,  dirName);
      } else {
        dir = new File(DUMP_KLASS_OUTPUT_DIR);
      }

      dir.mkdirs();
      File f = new File(dir, klassName.substring(klassName.lastIndexOf(File.separatorChar) + 1)
                              + ".class");
      f.createNewFile();
      FileOutputStream fis = new FileOutputStream(f);
      ClassWriter cw = new ClassWriter(kls, fis);
      cw.write();
   }

   public String genDumpKlass(InstanceKlass kls) {
      try {
         dumpKlass(kls);
         StringBuffer buf = new StringBuffer();
         buf.append(genHTMLPrologue(genKlassTitle(kls)));
         buf.append(".class created for ");
         buf.append(genKlassLink(kls));
         buf.append(genHTMLEpilogue());
         return buf.toString();
      } catch(IOException exp) {
         return genHTMLErrorMessage(exp);
      }
   }

   protected String genJavaStackTraceTitle(JavaThread thread) {
      StringBuffer buf = new StringBuffer();
      buf.append("Java Stack Trace for ");
      buf.append(thread.getThreadName());
      return buf.toString();
   }
       
   public String genHTMLForJavaStackTrace(JavaThread thread) {
      StringBuffer buf = new StringBuffer();
      buf.append(genHTMLPrologue(genJavaStackTraceTitle(thread)));

      buf.append("Thread state = ");
      buf.append(thread.getThreadState().toString());
      buf.append("<br>");
      buf.append("<pre>");
      for (JavaVFrame vf = thread.getLastJavaVFrameDbg(); vf != null; vf = vf.javaSender()) {
         Method method = vf.getMethod();
         buf.append(" - ");
         buf.append(genMethodLink(method));
         buf.append(" @bci = " + vf.getBCI());

         int lineNumber = method.getLineNumberFromBCI(vf.getBCI());
         if (lineNumber != -1) {
            buf.append(", line = ");
            buf.append(lineNumber);
         }

         sun.jvm.hotspot.debugger.Address pc = vf.getFrame().getPC();
         if (pc != null) {
            buf.append(", pc = ");
            buf.append("<a href='");
            buf.append(genPCHref(addressToLong(pc)));
            buf.append("'>");
            buf.append(pc.toString());
            buf.append("</a>");
         }

         if (vf.isCompiledFrame()) {
            buf.append(" (Compiled");
         }
         else if (vf.isInterpretedFrame()) {
            buf.append(" (Interpreted");
         }

         if (vf.mayBeImpreciseDbg()) {
            buf.append("; information may be imprecise");
         }
         buf.append(")");
         buf.append("<br>");
      }

      buf.append("</pre>");
      buf.append(genHTMLEpilogue());
      return buf.toString();
   }

   public String genHTMLForHyperlink(String href) {
      if (href.startsWith("klass=")) {
         href = href.substring(href.indexOf('=') + 1);
         Oop obj = getOopAtAddress(href);
         if (Assert.ASSERTS_ENABLED) {
            Assert.that(obj instanceof InstanceKlass, "class= href with improper InstanceKlass!");
         }
         return genHTML((InstanceKlass) obj); 
      } else if (href.startsWith("method=")) {
         href = href.substring(href.indexOf('=') + 1);
         Oop obj = getOopAtAddress(href);
         if (Assert.ASSERTS_ENABLED) {
            Assert.that(obj instanceof Method, "method= href with improper Method!");
         }
         return genHTML((Method) obj);
      } else if (href.startsWith("nmethod=")) {
         String addr = href.substring(href.indexOf('=') + 1);
         Object obj = VMObjectFactory.newObject(NMethod.class, parseAddress(addr));
         if (Assert.ASSERTS_ENABLED) {
            Assert.that(obj instanceof NMethod, "nmethod= href with improper NMethod!");
         }
         return genHTML((NMethod) obj);
      } else if (href.startsWith("pc=")) {
         String address = href.substring(href.indexOf('=') + 1);
         return genHTML(parseAddress(address));
      } else if (href.startsWith("pc_multiple=")) {
         int indexOfComma = href.indexOf(',');
         if (indexOfComma == -1) {
            String firstPC = href.substring(href.indexOf('=') + 1);
            return genHTMLForRawDisassembly(parseAddress(firstPC), null);
         } else {
            String firstPC = href.substring(href.indexOf('=') + 1, indexOfComma);
            return genHTMLForRawDisassembly(parseAddress(firstPC), href.substring(indexOfComma + 1));
         }
      } else if (href.startsWith("interp_codelets")) {
         return genInterpreterCodeletLinksPage();
      } else if (href.startsWith("hierarchy=")) {
         href = href.substring(href.indexOf('=') + 1);
         Oop obj = getOopAtAddress(href);
         if (Assert.ASSERTS_ENABLED) {
            Assert.that(obj instanceof InstanceKlass, "class= href with improper InstanceKlass!");
         }
         return genHTMLForKlassHierarchy((InstanceKlass) obj);
      } else if (href.startsWith("cpool=")) {
         href = href.substring(href.indexOf('=') + 1);
         Oop obj = getOopAtAddress(href);
         if (Assert.ASSERTS_ENABLED) {
            Assert.that(obj instanceof ConstantPool, "cpool= href with improper ConstantPool!");
         }
         return genHTML((ConstantPool) obj);
      } else if (href.startsWith("jcore=")) {
         href = href.substring(href.indexOf('=') + 1);
         Oop obj = getOopAtAddress(href);
         if (Assert.ASSERTS_ENABLED) {
            Assert.that(obj instanceof InstanceKlass, "jcore= href with improper InstanceKlass!");
         }
         return genDumpKlass((InstanceKlass) obj);
      } else if (href.startsWith("jcore_multiple=")) {
         href = href.substring(href.indexOf('=') + 1);
         StringBuffer buf = new StringBuffer();
         buf.append(genHTMLPrologue());
         StringTokenizer st = new StringTokenizer(href, ",");
         while (st.hasMoreTokens()) {
            Oop obj = getOopAtAddress(st.nextToken());
            if (Assert.ASSERTS_ENABLED) {
               Assert.that(obj instanceof InstanceKlass, "jcore_multiple= href with improper InstanceKlass!");
            }

            InstanceKlass kls = (InstanceKlass) obj;
            try {
               dumpKlass(kls);
               buf.append(".class created for ");
               buf.append(genKlassLink(kls));
            } catch(Exception exp) {
               buf.append("<b>can't .class for ");
               buf.append(genKlassTitle(kls));
               buf.append(" : ");
               buf.append(exp.getMessage());
               buf.append("</b>");
            }
            buf.append("<br>");
         }

         buf.append(genHTMLEpilogue());
         return buf.toString();
      } else {
         if (Assert.ASSERTS_ENABLED) {
            Assert.that(false, "unknown href link!");
         }
         return null;
      }
   }
}
