/**
 * @(#)Code.java	1.23 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.Symbol.*;


/**
 * An internal structure that corresponds to the code attribute of
 *  methods in a classfile. The class also provides some utility operations to
 *  generate bytecode instructions.
 */
public class Code implements ByteCodes, TypeTags {
    public static final boolean debugCode = false;

    /**
     * The maximum stack size.
     */
    public int max_stack = 0;

    /**
     * The maximum number of local variable slots.
     */
    public int max_locals = 0;

    /**
     * The code buffer.
     */
    public byte[] code = new byte[64];

    /**
     * the current code pointer.
     */
    public int cp = 0;

    /**
     * Check the code against VM spec limits; if
     *  problems report them and return true.
     */
    public boolean checkLimits(int pos, Log log) {
        if (cp > ClassFile.MAX_CODE) {
            log.error(pos, "limit.code");
            return true;
        }
        if (max_locals > ClassFile.MAX_LOCALS) {
            log.error(pos, "limit.locals");
            return true;
        }
        if (max_stack > ClassFile.MAX_STACK) {
            log.error(pos, "limit.stack");
            return true;
        }
        return false;
    }

    /**
      * A buffer for expression catch data. Each enter is a vector
      *  of four unsigned shorts.
      */
    ListBuffer catchInfo = new ListBuffer();

    /**
     * A buffer for line number information. Each entry is a vector
     *  of two unsigned shorts.
     */
    List lineInfo = new List();

    /**
     * The CharacterRangeTable
     */
    public CRTable crt;

    /**
     * Are we generating code with jumps >= 32K?
     */
    public boolean fatcode;

    /**
     * Code generation enabled?
     */
    private boolean alive = true;

    /**
     * The current stacksize.
     */
    public int stacksize = 0;

    /**
     * Is it forbidden to compactify code, because something is
     *  pointing to current location?
     */
    private boolean fixedPc = false;

    /**
     * The next available register.
     */
    public int nextreg = 0;

    /**
     * A chain for jumps to be resolved before the next opcode is emitted.
     *  We do this lazily to avoid jumps to jumps.
     */
    Chain pendingJumps = null;

    /**
     * The position of the currently statement, if we are at the
     *  start of this statement, NOPOS otherwise.
     *  We need this to emit line numbers lazily, which we need to do
     *  because of jump-to-jump optimization.
     */
    int pendingStatPos = 0;

    /**
     * Switch: emit variable debug info.
     */
    boolean varDebugInfo;

    /**
     * Switch: emit line number info.
     */
    boolean lineDebugInfo;

    /**
     * Construct a code object, given the settings of the fatcode,
     *  debugging info switches and the CharacterRangeTable.
     */
    public Code(boolean fatcode, boolean lineDebugInfo, boolean varDebugInfo,
            CRTable crt) {
        super();
        this.fatcode = fatcode;
        this.lineDebugInfo = lineDebugInfo;
        this.varDebugInfo = varDebugInfo;
        this.crt = crt;
        if (varDebugInfo) {
            defined = new Bits();
            lvar = new LocalVar[20];
        }
    }

    /**
      * Given a type, return its type code (used implicitly in the
      *  JVM architecture).
      */
    public static int typecode(Type type) {
        switch (type.tag) {
        case BYTE:
            return BYTEcode;

        case SHORT:
            return SHORTcode;

        case CHAR:
            return CHARcode;

        case INT:
            return INTcode;

        case LONG:
            return LONGcode;

        case FLOAT:
            return FLOATcode;

        case DOUBLE:
            return DOUBLEcode;

        case BOOLEAN:
            return BYTEcode;

        case VOID:
            return VOIDcode;

        case CLASS:

        case ARRAY:

        case METHOD:

        case BOT:
            return OBJECTcode;

        default:
            throw new AssertionError("typecode " + type.tag);

        }
    }

    /**
      * Collapse type code for subtypes of int to INTcode.
      */
    public static int truncate(int tc) {
        switch (tc) {
        case BYTEcode:

        case SHORTcode:

        case CHARcode:
            return INTcode;

        default:
            return tc;

        }
    }

    /**
      * The width in bytes of objects of the type.
      */
    public static int width(int typecode) {
        switch (typecode) {
        case LONGcode:

        case DOUBLEcode:
            return 2;

        case VOIDcode:
            return 0;

        default:
            return 1;

        }
    }

    public static int width(Type type) {
        return width(typecode(type));
    }

    /**
      * The total width taken up by a vector of objects.
      */
    public static int width(List types) {
        int w = 0;
        for (List l = types; l.nonEmpty(); l = l.tail)
            w = w + width((Type) l.head);
        return w;
    }

    /**
      * Given a type, return its code for allocating arrays of that type.
      */
    public static int arraycode(Type type) {
        switch (type.tag) {
        case BYTE:
            return 8;

        case BOOLEAN:
            return 4;

        case SHORT:
            return 9;

        case CHAR:
            return 5;

        case INT:
            return 10;

        case LONG:
            return 11;

        case FLOAT:
            return 6;

        case DOUBLE:
            return 7;

        case CLASS:
            return 0;

        case ARRAY:
            return 1;

        default:
            throw new AssertionError("arraycode " + type);

        }
    }

    /**
      * The current output code pointer.
      */
    public int curPc() {
        if (pendingJumps != null)
            resolvePending();
        if (pendingStatPos != Position.NOPOS)
            markStatBegin();
        fixedPc = true;
        return cp;
    }

    /**
      * Emit a byte of code.
      */
    public void emit1(int od) {
        if (alive) {
            if (cp == code.length) {
                byte[] newcode = new byte[cp * 2];
                System.arraycopy(code, 0, newcode, 0, cp);
                code = newcode;
            }
            code[cp++] = (byte) od;
        }
    }

    /**
      * Emit two bytes of code.
      */
    public void emit2(int od) {
        if (alive) {
            if (cp + 2 > code.length) {
                emit1(od >> 8);
                emit1(od);
            } else {
                code[cp++] = (byte)(od >> 8);
                code[cp++] = (byte) od;
            }
        }
    }

    /**
      * Emit four bytes of code.
      */
    public void emit4(int od) {
        if (alive) {
            if (cp + 4 > code.length) {
                emit1(od >> 24);
                emit1(od >> 16);
                emit1(od >> 8);
                emit1(od);
            } else {
                code[cp++] = (byte)(od >> 24);
                code[cp++] = (byte)(od >> 16);
                code[cp++] = (byte)(od >> 8);
                code[cp++] = (byte) od;
            }
        }
    }

    /**
      * Emit an opcode, adjust stacksize by sdiff.
      */
    public void emitop(int op, int sdiff) {
        if (pendingJumps != null)
            resolvePending();
        if (alive) {
            if (pendingStatPos != Position.NOPOS)
                markStatBegin();
            if (debugCode)
                System.err.println(cp + ":" + stacksize + ": " + mnem(op));
            emit1(op);
            if (sdiff <= -1000) {
                stacksize = stacksize + sdiff + 1000;
                alive = false;
                assert stacksize == 0;
            } else {
                stacksize = stacksize + sdiff;
                assert stacksize >= 0;
                if (stacksize > max_stack)
                    max_stack = stacksize;
            }
        }
    }

    /**
      * Emit an opcode, adjust stacksize by stackdiff[op].
      */
    public void emitop(int op) {
        emitop(op, stackdiff[op]);
    }

    /**
      * Emit an opcode with a one-byte operand field.
      */
    public void emitop1(int op, int od) {
        emitop(op);
        emit1(od);
    }

    /**
      * Emit an opcode with a one-byte operand field;
      *  widen if field does not fit in a byte.
      */
    public void emitop1w(int op, int od) {
        if (od > 255) {
            emitop(wide);
            emitop2(op, od);
        } else {
            emitop1(op, od);
        }
    }

    /**
      * Emit an opcode with a two-byte operand field.
      */
    public void emitop2(int op, int od) {
        emitop(op);
        emit2(od);
    }

    /**
      * Emit an opcode with a four-byte operand field.
      */
    public void emitop4(int op, int od) {
        emitop(op);
        emit4(od);
    }

    /**
      * Align code pointer to next `incr' boundary.
      */
    public void align(int incr) {
        if (alive)
            while (cp % incr != 0)
                emit1(0);
    }

    /**
      * Place a byte into code at address pc. Pre: pc + 1 <= cp.
      */
    public void put1(int pc, int op) {
        code[pc] = (byte) op;
    }

    /**
      * Place two bytes into code at address pc. Pre: pc + 2 <= cp.
      */
    public void put2(int pc, int od) {
        put1(pc, od >> 8);
        put1(pc + 1, od);
    }

    /**
      * Place four  bytes into code at address pc. Pre: pc + 4 <= cp.
      */
    public void put4(int pc, int od) {
        put1(pc, od >> 24);
        put1(pc + 1, od >> 16);
        put1(pc + 2, od >> 8);
        put1(pc + 3, od);
    }

    /**
      * Return code byte at position pc as an unsigned int.
      */
    public int get1(int pc) {
        return code[pc] & 255;
    }

    /**
      * Return two code bytes at position pc as an unsigned int.
      */
    public int get2(int pc) {
        return (get1(pc)<< 8) | get1(pc + 1);
    }

    /**
      * Return four code bytes at position pc as an int.
      */
    public int get4(int pc) {
        return (get1(pc)<< 24) | (get1(pc + 1)<< 16) | (get1(pc + 2)<< 8) |
                (get1(pc + 3));
    }

    /**
      * Is code generation currently enabled?
      */
    public boolean isAlive() {
        return alive || pendingJumps != null;
    }

    /**
      * Switch code generation on/off.
      */
    public void markDead() {
        alive = false;
    }

    /**
      * Declare an entry point; return current code pointer
      */
    public int entryPoint() {
        alive = true;
        return curPc();
    }

    /**
      * Declare an entry point with initial stack size;
      *  return current code pointer
      */
    public int entryPoint(int size) {
        alive = true;
        stacksize = size;
        if (stacksize > max_stack)
            max_stack = stacksize;
        return curPc();
    }

    /**
      * A chain represents a list of unresolved jumps. Jump locations
      *  are sorted in decreasing order.
      */
    public static class Chain {

        /**
         * The position of the jump instruction.
         */
        public final int pc;

        /**
         * The stacksize after the jump instruction.
         *  Invariant: all elements of a chain list have the same stacksize.
         */
        public final int stacksize;

        /**
         * The next jump in the list.
         */
        public final Chain next;

        /**
         * The set of variables alive at the jump.
         */
        public final Bits defined;

        /**
         * Construct a chain from its jump position, stacksize, previous
         *  chain, and set of defined variables.
         */
        public Chain(int pc, int stacksize, Chain next, Bits defined) {
            super();
            this.pc = pc;
            this.stacksize = stacksize;
            this.next = next;
            this.defined = defined;
        }
    }

    /**
      * Negate a branch opcode.
      */
    public static int negate(int opcode) {
        if (opcode == if_acmp_null)
            return if_acmp_nonnull;
        else if (opcode == if_acmp_nonnull)
            return if_acmp_null;
        else return ((opcode + 1) ^ 1)
                    - 1;
    }

    /**
      * Emit a jump instruction.
      *  Return code pointer of instruction to be patched.
      */
    public int emitJump(int opcode) {
        if (fatcode) {
            if (opcode == goto_ || opcode == jsr) {
                emitop4(opcode + goto_w - goto_, 0);
            } else {
                emitop2(negate(opcode), 8);
                emitop4(goto_w, 0);
            }
            return cp - 5;
        } else {
            emitop2(opcode, 0);
            return cp - 3;
        }
    }

    /**
      * Emit a branch with given opcode; return its chain.
      *  branch differs from jump in that jsr is treated as no-op.
      */
    public Chain branch(int opcode) {
        Chain result = null;
        if (opcode == goto_) {
            result = pendingJumps;
            pendingJumps = null;
        }
        if (opcode != dontgoto && isAlive()) {
            result = new Chain(emitJump(opcode), stacksize, result,
                    varDebugInfo ? defined.dup() : null);
            fixedPc = fatcode;
            if (opcode == goto_)
                alive = false;
        }
        return result;
    }

    /**
      * Resolve chain l to point to given target.
      */
    public void resolve(Chain chain, int target) {
        Bits newDefined = defined;
        for (; chain != null; chain = chain.next) {
            assert target > chain.pc || stacksize == 0;
            if (target >= cp) {
                target = cp;
            } else if (get1(target) == goto_) {
                if (fatcode)
                    target = target + get4(target + 1);
                else
                    target = target + get2(target + 1);
            }
            if (get1(chain.pc) == goto_ && chain.pc + 3 == target &&
                    target == cp && !fixedPc) {
                cp = cp - 3;
                target = target - 3;
            } else {
                if (fatcode)
                    put4(chain.pc + 1, target - chain.pc);
                else if (target - chain.pc < Short.MIN_VALUE ||
                        target - chain.pc > Short.MAX_VALUE)
                    fatcode = true;
                else
                    put2(chain.pc + 1, target - chain.pc);
                assert ! alive || chain.stacksize == stacksize;
            }
            fixedPc = true;
            if (cp == target) {
                if (alive) {
                    assert stacksize == chain.stacksize;
                    if (varDebugInfo)
                        newDefined = chain.defined.andSet(defined);
                } else {
                    stacksize = chain.stacksize;
                    if (varDebugInfo)
                        newDefined = chain.defined;
                    alive = true;
                }
            }
        }
        setDefined(newDefined);
    }

    /**
      * Set the current variable defined state.
      */
    public void setDefined(Bits newDefined) {
        if (alive && varDebugInfo && newDefined != defined) {
            Bits diff = defined.dup().xorSet(newDefined);
            for (int adr = diff.nextBit(0); adr >= 0; adr = diff.nextBit(adr + 1)) {
                if (adr >= nextreg)
                    defined.excl(adr);
                else if (defined.isMember(adr))
                    setUndefined(adr);
                else
                    setDefined(adr);
            }
        }
    }

    /**
      * Resolve chain l to point to current code pointer.
      */
    public void resolve(Chain chain) {
        pendingJumps = mergeChains(chain, pendingJumps);
    }

    /**
      * Resolve any pending jumps.
      */
    public void resolvePending() {
        Chain x = pendingJumps;
        pendingJumps = null;
        resolve(x, cp);
    }

    /**
      * Merge the jumps in of two chains into one.
      */
    public static Chain mergeChains(Chain chain1, Chain chain2) {
        if (chain2 == null)
            return chain1;
        if (chain1 == null)
            return chain2;
        if (chain1.pc < chain2.pc)
            return new Chain(chain2.pc, chain2.stacksize,
                    mergeChains(chain1, chain2.next), chain2.defined);
        return new Chain(chain1.pc, chain1.stacksize,
                mergeChains(chain1.next, chain2), chain1.defined);
    }

    /**
      * Add a catch clause to code.
      */
    public void addCatch(char startPc, char endPc, char handlerPc, char catchType) {
        catchInfo.append(new char[]{startPc, endPc, handlerPc, catchType});
    }

    /**
      * Add a line number entry.
      */
    public void addLineNumber(char startPc, char lineNumber) {
        if (lineDebugInfo) {
            if (lineInfo.nonEmpty() && ((char[]) lineInfo.head)[0] == startPc)
                lineInfo = lineInfo.tail;
            if (lineInfo.isEmpty() || ((char[]) lineInfo.head)[1] != lineNumber)
                lineInfo = lineInfo.prepend(new char[]{startPc, lineNumber});
        }
    }

    /**
      * Mark beginning of statement.
      */
    public void statBegin(int pos) {
        if (pos != Position.NOPOS) {
            pendingStatPos = pos;
        }
    }

    /**
      * Force stat begin eagerly
      */
    public void markStatBegin() {
        int line = Position.line(pendingStatPos);
        pendingStatPos = Position.NOPOS;
        if (alive && lineDebugInfo) {
            char cp1 = (char) cp;
            char line1 = (char) line;
            if (cp1 == cp && line1 == line)
                addLineNumber(cp1, line1);
        }
    }

    /**
      * A live range of a local variable.
      */
    static class LocalVar {
        final VarSymbol var;
        final char reg;
        char start_pc = Character.MAX_VALUE;
        char length = Character.MAX_VALUE;

        LocalVar(VarSymbol v) {
            super();
            this.var = v;
            this.reg = (char) v.adr;
        }

        public LocalVar dup() {
            return new LocalVar(var);
        }
    }
    {
    }

    /**
      * Local variables, indexed by register.
      */
    LocalVar[] lvar;
    public Bits defined;

    /**
     * Add a new local variable.
     */
    private void addLocalVar(VarSymbol v) {
        int adr = v.adr;
        if (adr >= lvar.length) {
            int newlength = lvar.length * 2;
            if (newlength <= adr)
                newlength = adr + 10;
            LocalVar[] new_lvar = new LocalVar[newlength];
            System.arraycopy(lvar, 0, new_lvar, 0, lvar.length);
            lvar = new_lvar;
        }
        assert lvar[adr] == null;
        lvar[adr] = new LocalVar(v);
        defined.excl(adr);
    }

    /**
      * Mark a register as being defined.
      */
    public void setDefined(int adr) {
        if (!varDebugInfo)
            return;
        defined.incl(adr);
        if (cp < Character.MAX_VALUE && adr < lvar.length) {
            LocalVar v = lvar[adr];
            if (v != null && v.start_pc == Character.MAX_VALUE)
                v.start_pc = (char) cp;
        }
    }

    /**
      * Mark a register as being undefined.
      */
    public void setUndefined(int adr) {
        defined.excl(adr);
        if (adr < lvar.length && lvar[adr] != null &&
                lvar[adr].start_pc != Character.MAX_VALUE) {
            LocalVar v = lvar[adr];
            char length = (char)(curPc() - v.start_pc);
            if (length < Character.MAX_VALUE) {
                lvar[adr] = v.dup();
                v.length = length;
                putVar(v);
            } else {
                v.start_pc = Character.MAX_VALUE;
            }
        }
    }

    /**
      * End the scope of a variable.
      */
    private void endScope(int adr) {
        defined.excl(adr);
        if (adr < lvar.length && lvar[adr] != null) {
            LocalVar v = lvar[adr];
            lvar[adr] = null;
            if (v.start_pc != Character.MAX_VALUE) {
                char length = (char)(curPc() - v.start_pc);
                if (length < Character.MAX_VALUE) {
                    v.length = length;
                    putVar(v);
                }
            }
        }
    }

    /**
      * Put a live variable range into the buffer to be output to the
      *  class file.
      */
    void putVar(LocalVar var) {
        if (varBuffer == null)
            varBuffer = new LocalVar[20];
        if (varBufferSize >= varBuffer.length) {
            LocalVar[] newVarBuffer = new LocalVar[varBufferSize * 2];
            System.arraycopy(varBuffer, 0, newVarBuffer, 0, varBuffer.length);
            varBuffer = newVarBuffer;
        }
        varBuffer[varBufferSize++] = var;
    }

    /**
      * Previously live local variables, to be put into the variable table.
      */
    LocalVar[] varBuffer;
    int varBufferSize;

    /**
     * Create a new local variable address and return it.
     */
    public int newLocal(int typecode) {
        int reg = nextreg;
        int w = width(typecode);
        nextreg = reg + w;
        if (nextreg > max_locals)
            max_locals = nextreg;
        return reg;
    }

    public int newLocal(Type type) {
        return newLocal(typecode(type));
    }

    public int newLocal(VarSymbol v) {
        int reg = v.adr = newLocal(v.erasure());
        if (varDebugInfo)
            addLocalVar(v);
        return reg;
    }

    /**
      * Start a set of fresh registers.
      */
    public void newRegSegment() {
        nextreg = max_locals;
    }

    /**
      * End scopes of all variables with registers >= first.
      */
    public void endScopes(int first) {
        if (varDebugInfo)
            for (int i = first; i < nextreg; i++)
                endScope(i);
        nextreg = first;
    }

    /**
      * How opcodes affect the stack size.
      *  Opcodes with stackdiff value -999 need to be teated specially.
      *  Opcodes with stackdiff value < -1000 are jumps after which code is dead.
      */
    public static final int[] stackdiff = new int[ByteCodeCount];
    static {
        stackdiff[nop] = 0;
        stackdiff[aconst_null] = 1;
        stackdiff[iconst_m1] = 1;
        stackdiff[iconst_0] = 1;
        stackdiff[iconst_1] = 1;
        stackdiff[iconst_2] = 1;
        stackdiff[iconst_3] = 1;
        stackdiff[iconst_4] = 1;
        stackdiff[iconst_5] = 1;
        stackdiff[lconst_0] = 2;
        stackdiff[lconst_1] = 2;
        stackdiff[fconst_0] = 1;
        stackdiff[fconst_1] = 1;
        stackdiff[fconst_2] = 1;
        stackdiff[dconst_0] = 2;
        stackdiff[dconst_1] = 2;
        stackdiff[bipush] = 1;
        stackdiff[sipush] = 1;
        stackdiff[ldc1] = -999;
        stackdiff[ldc2] = -999;
        stackdiff[ldc2w] = -999;
        stackdiff[iload] = 1;
        stackdiff[lload] = 2;
        stackdiff[fload] = 1;
        stackdiff[dload] = 2;
        stackdiff[aload] = 1;
        stackdiff[iload_0] = 1;
        stackdiff[lload_0] = 2;
        stackdiff[fload_0] = 1;
        stackdiff[dload_0] = 2;
        stackdiff[aload_0] = 1;
        stackdiff[iload_1] = 1;
        stackdiff[lload_1] = 2;
        stackdiff[fload_1] = 1;
        stackdiff[dload_1] = 2;
        stackdiff[aload_1] = 1;
        stackdiff[iload_2] = 1;
        stackdiff[lload_2] = 2;
        stackdiff[fload_2] = 1;
        stackdiff[dload_2] = 2;
        stackdiff[aload_2] = 1;
        stackdiff[iload_3] = 1;
        stackdiff[lload_3] = 2;
        stackdiff[fload_3] = 1;
        stackdiff[dload_3] = 2;
        stackdiff[aload_3] = 1;
        stackdiff[iaload] = -1;
        stackdiff[laload] = 0;
        stackdiff[faload] = -1;
        stackdiff[daload] = 0;
        stackdiff[aaload] = -1;
        stackdiff[baload] = -1;
        stackdiff[caload] = -1;
        stackdiff[saload] = -1;
        stackdiff[istore] = -1;
        stackdiff[lstore] = -2;
        stackdiff[fstore] = -1;
        stackdiff[dstore] = -2;
        stackdiff[astore] = -1;
        stackdiff[istore_0] = -1;
        stackdiff[lstore_0] = -2;
        stackdiff[fstore_0] = -1;
        stackdiff[dstore_0] = -2;
        stackdiff[astore_0] = -1;
        stackdiff[istore_1] = -1;
        stackdiff[lstore_1] = -2;
        stackdiff[fstore_1] = -1;
        stackdiff[dstore_1] = -2;
        stackdiff[astore_1] = -1;
        stackdiff[istore_2] = -1;
        stackdiff[lstore_2] = -2;
        stackdiff[fstore_2] = -1;
        stackdiff[dstore_2] = -2;
        stackdiff[astore_2] = -1;
        stackdiff[istore_3] = -1;
        stackdiff[lstore_3] = -2;
        stackdiff[fstore_3] = -1;
        stackdiff[dstore_3] = -2;
        stackdiff[astore_3] = -1;
        stackdiff[iastore] = -3;
        stackdiff[lastore] = -4;
        stackdiff[fastore] = -3;
        stackdiff[dastore] = -4;
        stackdiff[aastore] = -3;
        stackdiff[bastore] = -3;
        stackdiff[castore] = -3;
        stackdiff[sastore] = -3;
        stackdiff[pop] = -1;
        stackdiff[pop2] = -2;
        stackdiff[dup] = 1;
        stackdiff[dup_x1] = 1;
        stackdiff[dup_x2] = 1;
        stackdiff[dup2] = 2;
        stackdiff[dup2_x1] = 2;
        stackdiff[dup2_x2] = 2;
        stackdiff[swap] = 0;
        stackdiff[iadd] = -1;
        stackdiff[ladd] = -2;
        stackdiff[fadd] = -1;
        stackdiff[dadd] = -2;
        stackdiff[isub] = -1;
        stackdiff[lsub] = -2;
        stackdiff[fsub] = -1;
        stackdiff[dsub] = -2;
        stackdiff[imul] = -1;
        stackdiff[lmul] = -2;
        stackdiff[fmul] = -1;
        stackdiff[dmul] = -2;
        stackdiff[idiv] = -1;
        stackdiff[ldiv] = -2;
        stackdiff[fdiv] = -1;
        stackdiff[ddiv] = -2;
        stackdiff[imod] = -1;
        stackdiff[lmod] = -2;
        stackdiff[fmod] = -1;
        stackdiff[dmod] = -2;
        stackdiff[ineg] = 0;
        stackdiff[lneg] = 0;
        stackdiff[fneg] = 0;
        stackdiff[dneg] = 0;
        stackdiff[ishl] = -1;
        stackdiff[lshl] = -1;
        stackdiff[ishr] = -1;
        stackdiff[lshr] = -1;
        stackdiff[iushr] = -1;
        stackdiff[lushr] = -1;
        stackdiff[iand] = -1;
        stackdiff[land] = -2;
        stackdiff[ior] = -1;
        stackdiff[lor] = -2;
        stackdiff[ixor] = -1;
        stackdiff[lxor] = -2;
        stackdiff[iinc] = 0;
        stackdiff[i2l] = 1;
        stackdiff[i2f] = 0;
        stackdiff[i2d] = 1;
        stackdiff[l2i] = -1;
        stackdiff[l2f] = -1;
        stackdiff[l2d] = 0;
        stackdiff[f2i] = 0;
        stackdiff[f2l] = 1;
        stackdiff[f2d] = 1;
        stackdiff[d2i] = -1;
        stackdiff[d2l] = 0;
        stackdiff[d2f] = -1;
        stackdiff[int2byte] = 0;
        stackdiff[int2char] = 0;
        stackdiff[int2short] = 0;
        stackdiff[lcmp] = -3;
        stackdiff[fcmpl] = -1;
        stackdiff[fcmpg] = -1;
        stackdiff[dcmpl] = -3;
        stackdiff[dcmpg] = -3;
        stackdiff[ifeq] = -1;
        stackdiff[ifne] = -1;
        stackdiff[iflt] = -1;
        stackdiff[ifge] = -1;
        stackdiff[ifgt] = -1;
        stackdiff[ifle] = -1;
        stackdiff[if_icmpeq] = -2;
        stackdiff[if_icmpne] = -2;
        stackdiff[if_icmplt] = -2;
        stackdiff[if_icmpge] = -2;
        stackdiff[if_icmpgt] = -2;
        stackdiff[if_icmple] = -2;
        stackdiff[if_acmpeq] = -2;
        stackdiff[if_acmpne] = -2;
        stackdiff[goto_] = 0;
        stackdiff[jsr] = 0;
        stackdiff[ret] = 0;
        stackdiff[tableswitch] = -1;
        stackdiff[lookupswitch] = -1;
        stackdiff[ireturn] = -1001;
        stackdiff[lreturn] = -1002;
        stackdiff[freturn] = -1001;
        stackdiff[dreturn] = -1002;
        stackdiff[areturn] = -1001;
        stackdiff[return_] = -1000;
        stackdiff[getstatic] = -999;
        stackdiff[putstatic] = -999;
        stackdiff[getfield] = -999;
        stackdiff[putfield] = -999;
        stackdiff[invokevirtual] = -999;
        stackdiff[invokespecial] = -999;
        stackdiff[invokestatic] = -999;
        stackdiff[invokeinterface] = -999;
        stackdiff[newfromname] = 0;
        stackdiff[new_] = 1;
        stackdiff[newarray] = 0;
        stackdiff[anewarray] = 0;
        stackdiff[arraylength] = 0;
        stackdiff[athrow] = -1001;
        stackdiff[checkcast] = 0;
        stackdiff[instanceof_] = 0;
        stackdiff[monitorenter] = -1;
        stackdiff[monitorexit] = -1;
        stackdiff[wide] = 0;
        stackdiff[multianewarray] = -999;
        stackdiff[if_acmp_null] = -1;
        stackdiff[if_acmp_nonnull] = -1;
        stackdiff[goto_w] = 0;
        stackdiff[jsr_w] = 0;
        stackdiff[breakpoint] = 0;
    }

    public static String mnem(int opcode) {
        return Mneumonics.mnem[opcode];
    }

    private static class Mneumonics {

        private Mneumonics() {
            super();
        }
        private static final String[] mnem = new String[ByteCodeCount];
        static {
            mnem[nop] = "nop";
            mnem[aconst_null] = "aconst_null";
            mnem[iconst_m1] = "iconst_m1";
            mnem[iconst_0] = "iconst_0";
            mnem[iconst_1] = "iconst_1";
            mnem[iconst_2] = "iconst_2";
            mnem[iconst_3] = "iconst_3";
            mnem[iconst_4] = "iconst_4";
            mnem[iconst_5] = "iconst_5";
            mnem[lconst_0] = "lconst_0";
            mnem[lconst_1] = "lconst_1";
            mnem[fconst_0] = "fconst_0";
            mnem[fconst_1] = "fconst_1";
            mnem[fconst_2] = "fconst_2";
            mnem[dconst_0] = "dconst_0";
            mnem[dconst_1] = "dconst_1";
            mnem[bipush] = "bipush";
            mnem[sipush] = "sipush";
            mnem[ldc1] = "ldc1";
            mnem[ldc2] = "ldc2";
            mnem[ldc2w] = "ldc2w";
            mnem[iload] = "iload";
            mnem[lload] = "lload";
            mnem[fload] = "fload";
            mnem[dload] = "dload";
            mnem[aload] = "aload";
            mnem[iload_0] = "iload_0";
            mnem[lload_0] = "lload_0";
            mnem[fload_0] = "fload_0";
            mnem[dload_0] = "dload_0";
            mnem[aload_0] = "aload_0";
            mnem[iload_1] = "iload_1";
            mnem[lload_1] = "lload_1";
            mnem[fload_1] = "fload_1";
            mnem[dload_1] = "dload_1";
            mnem[aload_1] = "aload_1";
            mnem[iload_2] = "iload_2";
            mnem[lload_2] = "lload_2";
            mnem[fload_2] = "fload_2";
            mnem[dload_2] = "dload_2";
            mnem[aload_2] = "aload_2";
            mnem[iload_3] = "iload_3";
            mnem[lload_3] = "lload_3";
            mnem[fload_3] = "fload_3";
            mnem[dload_3] = "dload_3";
            mnem[aload_3] = "aload_3";
            mnem[iaload] = "iaload";
            mnem[laload] = "laload";
            mnem[faload] = "faload";
            mnem[daload] = "daload";
            mnem[aaload] = "aaload";
            mnem[baload] = "baload";
            mnem[caload] = "caload";
            mnem[saload] = "saload";
            mnem[istore] = "istore";
            mnem[lstore] = "lstore";
            mnem[fstore] = "fstore";
            mnem[dstore] = "dstore";
            mnem[astore] = "astore";
            mnem[istore_0] = "istore_0";
            mnem[lstore_0] = "lstore_0";
            mnem[fstore_0] = "fstore_0";
            mnem[dstore_0] = "dstore_0";
            mnem[astore_0] = "astore_0";
            mnem[istore_1] = "istore_1";
            mnem[lstore_1] = "lstore_1";
            mnem[fstore_1] = "fstore_1";
            mnem[dstore_1] = "dstore_1";
            mnem[astore_1] = "astore_1";
            mnem[istore_2] = "istore_2";
            mnem[lstore_2] = "lstore_2";
            mnem[fstore_2] = "fstore_2";
            mnem[dstore_2] = "dstore_2";
            mnem[astore_2] = "astore_2";
            mnem[istore_3] = "istore_3";
            mnem[lstore_3] = "lstore_3";
            mnem[fstore_3] = "fstore_3";
            mnem[dstore_3] = "dstore_3";
            mnem[astore_3] = "astore_3";
            mnem[iastore] = "iastore";
            mnem[lastore] = "lastore";
            mnem[fastore] = "fastore";
            mnem[dastore] = "dastore";
            mnem[aastore] = "aastore";
            mnem[bastore] = "bastore";
            mnem[castore] = "castore";
            mnem[sastore] = "sastore";
            mnem[pop] = "pop";
            mnem[pop2] = "pop2";
            mnem[dup] = "dup";
            mnem[dup_x1] = "dup_x1";
            mnem[dup_x2] = "dup_x2";
            mnem[dup2] = "dup2";
            mnem[dup2_x1] = "dup2_x1";
            mnem[dup2_x2] = "dup2_x2";
            mnem[swap] = "swap";
            mnem[iadd] = "iadd";
            mnem[ladd] = "ladd";
            mnem[fadd] = "fadd";
            mnem[dadd] = "dadd";
            mnem[isub] = "isub";
            mnem[lsub] = "lsub";
            mnem[fsub] = "fsub";
            mnem[dsub] = "dsub";
            mnem[imul] = "imul";
            mnem[lmul] = "lmul";
            mnem[fmul] = "fmul";
            mnem[dmul] = "dmul";
            mnem[idiv] = "idiv";
            mnem[ldiv] = "ldiv";
            mnem[fdiv] = "fdiv";
            mnem[ddiv] = "ddiv";
            mnem[imod] = "imod";
            mnem[lmod] = "lmod";
            mnem[fmod] = "fmod";
            mnem[dmod] = "dmod";
            mnem[ineg] = "ineg";
            mnem[lneg] = "lneg";
            mnem[fneg] = "fneg";
            mnem[dneg] = "dneg";
            mnem[ishl] = "ishl";
            mnem[lshl] = "lshl";
            mnem[ishr] = "ishr";
            mnem[lshr] = "lshr";
            mnem[iushr] = "iushr";
            mnem[lushr] = "lushr";
            mnem[iand] = "iand";
            mnem[land] = "land";
            mnem[ior] = "ior";
            mnem[lor] = "lor";
            mnem[ixor] = "ixor";
            mnem[lxor] = "lxor";
            mnem[iinc] = "iinc";
            mnem[i2l] = "i2l";
            mnem[i2f] = "i2f";
            mnem[i2d] = "i2d";
            mnem[l2i] = "l2i";
            mnem[l2f] = "l2f";
            mnem[l2d] = "l2d";
            mnem[f2i] = "f2i";
            mnem[f2l] = "f2l";
            mnem[f2d] = "f2d";
            mnem[d2i] = "d2i";
            mnem[d2l] = "d2l";
            mnem[d2f] = "d2f";
            mnem[int2byte] = "int2byte";
            mnem[int2char] = "int2char";
            mnem[int2short] = "int2short";
            mnem[lcmp] = "lcmp";
            mnem[fcmpl] = "fcmpl";
            mnem[fcmpg] = "fcmpg";
            mnem[dcmpl] = "dcmpl";
            mnem[dcmpg] = "dcmpg";
            mnem[ifeq] = "ifeq";
            mnem[ifne] = "ifne";
            mnem[iflt] = "iflt";
            mnem[ifge] = "ifge";
            mnem[ifgt] = "ifgt";
            mnem[ifle] = "ifle";
            mnem[if_icmpeq] = "if_icmpeq";
            mnem[if_icmpne] = "if_icmpne";
            mnem[if_icmplt] = "if_icmplt";
            mnem[if_icmpge] = "if_icmpge";
            mnem[if_icmpgt] = "if_icmpgt";
            mnem[if_icmple] = "if_icmple";
            mnem[if_acmpeq] = "if_acmpeq";
            mnem[if_acmpne] = "if_acmpne";
            mnem[goto_] = "goto_";
            mnem[jsr] = "jsr";
            mnem[ret] = "ret";
            mnem[tableswitch] = "tableswitch";
            mnem[lookupswitch] = "lookupswitch";
            mnem[ireturn] = "ireturn";
            mnem[lreturn] = "lreturn";
            mnem[freturn] = "freturn";
            mnem[dreturn] = "dreturn";
            mnem[areturn] = "areturn";
            mnem[return_] = "return_";
            mnem[getstatic] = "getstatic";
            mnem[putstatic] = "putstatic";
            mnem[getfield] = "getfield";
            mnem[putfield] = "putfield";
            mnem[invokevirtual] = "invokevirtual";
            mnem[invokespecial] = "invokespecial";
            mnem[invokestatic] = "invokestatic";
            mnem[invokeinterface] = "invokeinterface";
            mnem[newfromname] = "newfromname";
            mnem[new_] = "new_";
            mnem[newarray] = "newarray";
            mnem[anewarray] = "anewarray";
            mnem[arraylength] = "arraylength";
            mnem[athrow] = "athrow";
            mnem[checkcast] = "checkcast";
            mnem[instanceof_] = "instanceof_";
            mnem[monitorenter] = "monitorenter";
            mnem[monitorexit] = "monitorexit";
            mnem[wide] = "wide";
            mnem[multianewarray] = "multianewarray";
            mnem[if_acmp_null] = "if_acmp_null";
            mnem[if_acmp_nonnull] = "if_acmp_nonnull";
            mnem[goto_w] = "goto_w";
            mnem[jsr_w] = "jsr_w";
            mnem[breakpoint] = "breakpoint";
        }
    }
}
