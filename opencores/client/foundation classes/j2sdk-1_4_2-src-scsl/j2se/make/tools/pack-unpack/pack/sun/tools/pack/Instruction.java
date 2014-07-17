/*
 * @(#)Instruction.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.pack;

/**
 * A parsed bytecode instruction.
 * Provides accessors to various relevant bits.
 * @author John Rose
 * @version 1.4, 01/23/03
 */
public
class Instruction implements Constants {
    protected byte[] bytes;  // bytecodes
    protected int pc;        // location of this instruction
    protected int bc;        // opcode of this instruction
    protected int w;         // 0 if normal, 1 if a _wide prefix at pc
    protected int length;    // bytes in this instruction

    protected boolean special;

    protected Instruction(byte[] bytes, int pc, int bc, int w, int length) {
	reset(bytes, pc, bc, w, length);
    }
    private void reset(byte[] bytes, int pc, int bc, int w, int length) {
	this.bytes = bytes;
	this.pc = pc;
	this.bc = bc;
	this.w = w;
	this.length = length;
    }

    public int getBC() {
	return bc;
    }
    public boolean isWide() {
	return w != 0;
    }
    public byte[] getBytes() {
	return bytes;
    }
    public int getPC() {
	return pc;
    }
    public int getLength() {
	return length;
    }
    public int getNextPC() {
	return pc + length;
    }

    public Instruction next() {
	int npc = pc + length;
	if (npc == bytes.length)
	    return null;
	else
	    return Instruction.at(bytes, npc, this);
    }

    /** A fake instruction at this pc whose next() will be at nextpc. */
    public Instruction forceNextPC(int nextpc) {
	int length = nextpc - pc;
	return new Instruction(bytes, pc, -1, -1, length);
    }

    public static Instruction at(byte[] bytes, int pc) {
	return Instruction.at(bytes, pc, null);
    }

    public static Instruction at(byte[] bytes, int pc, Instruction reuse) {
	int bc = getByte(bytes, pc);
	int prefix = -1;
	int w = 0;
	int length = BC_LENGTH[w][bc];
	if (length == 0) {
	    // Hard cases:
	    switch (bc) {
	    case _wide:
		bc = getByte(bytes, pc+1);
		w = 1;
		length = BC_LENGTH[w][bc];
		break;
	    case _tableswitch:
		return new TableSwitch(bytes, pc);
	    case _lookupswitch:
		return new LookupSwitch(bytes, pc);
	    }
	}
	if(D)assert0(length > 0);
	if(D)assert0(pc+length <= bytes.length);
	// Speed hack:  Instruction.next reuses self if possible.
	if (reuse != null && !reuse.special) {
	    reuse.reset(bytes, pc, bc, w, length);
	    return reuse;
	}
	return new Instruction(bytes, pc, bc, w, length);
    }

    // Return the constant pool reference type, or 0 if none.
    public byte getCPTag() {
	return BC_TAG[w][bc];
    }

    // Return the constant pool index, or -1 if none.
    public int getCPIndex() {
	int indexLoc = BC_INDEX[w][bc];
	if (indexLoc == 0)  return -1;
	if(D)assert0(w == 0);
	if (length == 2)
	    return getByte(bytes, pc+indexLoc);  // _ldc opcode only
	else
	    return getShort(bytes, pc+indexLoc);
    }

    public ConstantPool.Entry getCPRef(ConstantPool.Entry[] cpMap) {
	int index = getCPIndex();
	return (index < 0) ? null : cpMap[index];
    }

    // Return the slot of the affected local, or -1 if none.
    public int getLocalSlot() {
	int slotLoc = BC_SLOT[w][bc];
	if (slotLoc == 0)  return -1;
	if (w == 0)
	    return getByte(bytes, pc+slotLoc);
	else
	    return getShort(bytes, pc+slotLoc);
    }
    
    // Return the target of the branch, or -1 if none.
    public int getBranchLabel() {
	int branchLoc = BC_BRANCH[w][bc];
	if (branchLoc == 0)  return -1;
	if(D)assert0(w == 0);
	if(D)assert0(length == 3 || length == 5);
	int offset;
	if (length == 3)
	    offset = (short)getShort(bytes, pc+branchLoc);
	else
	    offset = getInt(bytes, pc+branchLoc);
	if(D)assert0(offset+pc >= 0);
	if(D)assert0(offset+pc <= bytes.length);
	return offset+pc;
    }

    // Return the trailing constant in the instruction (as a signed value).
    // Return 0 if there is none.
    public int getConstant() {
	int conLoc = BC_CON[w][bc];
	if (conLoc == 0)  return 0;
	switch (length - conLoc) {
	case 1: return (byte) getByte(bytes, pc+conLoc);
	case 2: return (short) getShort(bytes, pc+conLoc);
	}
	if(D)assert0(false);
	return 0;
    }

    public abstract static class Switch extends Instruction {
	public abstract int getCaseValue(int i); // 0 <= i < caseCount
	public abstract int getCaseLabel(int i); // 0 <= i < caseCount
	public abstract int getDefaultLabel();

	protected int apc;        // aligned pc (table base)
	protected int caseCount;  // number of cases in this switch
	protected int intAt(int n) { return getInt(bytes, apc + n*4); }
	protected Switch(byte[] bytes, int pc, int bc) {
	    super(bytes, pc, bc, /*w*/0, /*length*/0);
	    this.apc = alignPC(pc+1);
	    this.special = true;
	}
	public int getCaseCount() { return caseCount; }
	public int getAlignedPC() { return apc; }
	public String toString() {
	    String s = super.toString();
	    s += " Default:"+labstr(getDefaultLabel());
	    for (int i = 0; i < caseCount; i++) {
		s += "\n\tCase "+getCaseValue(i)+":"+labstr(getCaseLabel(i));
	    }
	    return s;
	}
	public static int alignPC(int apc) {
	    while (apc % 4 != 0)  ++apc;
	    return apc;
	}
    }

    public static class TableSwitch extends Switch {
	// apc:  (df, lo, hi, (hi-lo+1)*(label))
	public int getDefaultLabel()   { return intAt(0)+pc; }
	public int getLowCase()        { return intAt(1); }
	public int getHighCase()       { return intAt(2); }
	public int getCaseValue(int n) { return getLowCase()+n; }
	public int getCaseLabel(int n) { return intAt(3+n)+pc; }

	TableSwitch(byte[] bytes, int pc) {
	    super(bytes, pc, _tableswitch);
	    caseCount = getHighCase() - getLowCase() + 1;
	    length = (apc-pc) + (3 + caseCount) * 4;
	}
    }

    public static class LookupSwitch extends Switch {
	// apc:  (df, nc, nc*(case, label))
	public int getDefaultLabel()   { return intAt(0)+pc; }
	public int getCaseValue(int n) { return intAt(2+n*2+0); }
	public int getCaseLabel(int n) { return intAt(2+n*2+1)+pc; }

	LookupSwitch(byte[] bytes, int pc) {
	    super(bytes, pc, _lookupswitch);
	    caseCount = intAt(1);
	    length = (apc-pc) + (2 + caseCount*2) * 4;
	}
    }

    /** Two insns are equal if they have the same bytes. */
    public boolean equals(Object o) {
	return (o instanceof Instruction) && equals((Instruction)o);
    }
    public boolean equals(Instruction that) {
	if (this.bc != that.bc)            return false;
	if (this.w  != that.w)             return false;
	if (this.length  != that.length)   return false;
	for (int i = 1; i < length; i++) {
	    if (this.bytes[this.pc+i] != that.bytes[that.pc+i])
		return false;
	}
	return true;
    }

    static String labstr(int pc) {
	if (pc >= 0 && pc < 100000)
	    return ((100000+pc)+"").substring(1);
	return pc+"";
    }
    public String toString() {
	return toString(null);
    }
    public String toString(ConstantPool.Entry[] cpMap) {
	String s = labstr(pc) + ": ";
	if (bc >= _bytecode_limit) {
	    s += Integer.toHexString(bc);
	    return s;
	}
	if (w == 1)  s += " wide";
	s += BC_NAME[bc];
	int tag = getCPTag();
	if (tag != 0)  s += " "+ConstantPool.tagName(tag)+":";
	int idx = getCPIndex();
	if (idx >= 0)  s += (cpMap == null) ? ""+idx : "="+cpMap[idx].stringValue();
	int slt = getLocalSlot();
	if (slt >= 0)  s += " Local:"+slt;
	int lab = getBranchLabel();
	if (lab >= 0)  s += " To:"+labstr(lab);
	int con = getConstant();
	if (con != 0)  s += " Con:"+con;
	return s;
    }


    //public static byte constantPoolTagFor(int bc) { return BC_TAG[0][bc]; }

    /// Fetching values from byte arrays:

    public static int getInt(byte[] bytes, int pc) {
	return (getShort(bytes, pc+0) << 16) + (getShort(bytes, pc+2) << 0);
    }
    public static int getShort(byte[] bytes, int pc) {
	return (getByte(bytes, pc+0) << 8) + (getByte(bytes, pc+1) << 0);
    }
    public static int getByte(byte[] bytes, int pc) {
	return bytes[pc] & 0xFF;
    }
    

    public static void setInt(byte[] bytes, int pc, int x) {
	setShort(bytes, pc+0, x >> 16);
	setShort(bytes, pc+2, x >> 0);
    }
    public static void setShort(byte[] bytes, int pc, int x) {
	setByte(bytes, pc+0, x >> 8);
	setByte(bytes, pc+1, x >> 0);
    }
    public static void setByte(byte[] bytes, int pc, int x) {
	bytes[pc] = (byte)x;
    }

    // some bytecode classifiers

    public static int opLength(int bc) {
	int l = BC_LENGTH[0][bc];
	if(D)assert0(l > 0);
	return l;
    }
    public static int opWideLength(int bc) {
	int l = BC_LENGTH[1][bc];
	if(D)assert0(l > 0);
	return l;
    }

    public static boolean isLocalSlotOp(int bc) {
	return (bc < BC_SLOT[0].length && BC_SLOT[0][bc] > 0);
    }

    public static boolean isBranchOp(int bc) {
	return (bc < BC_BRANCH[0].length && BC_BRANCH[0][bc] > 0);
    }

    public static boolean isCPRefOp(int bc) {
	if (bc < BC_INDEX[0].length && BC_INDEX[0][bc] > 0)  return true;
	if (bc >= _xldc_op && bc < _xldc_limit)  return true;
	return false;
    }

    public static byte getCPRefOpTag(int bc) {
	if (bc < BC_INDEX[0].length && BC_INDEX[0][bc] > 0)  return BC_TAG[0][bc];
	if (bc >= _xldc_op && bc < _xldc_limit)  return CONSTANT_Literal;
	return CONSTANT_None;
    }

    public static boolean isFieldOp(int bc) {
	return (bc >= _getstatic && bc <= _putfield);
    }

    public static boolean isInvokeInitOp(int bc) {
	return (bc >= _invokeinit_op && bc < _invokeinit_limit);
    }

    public static boolean isSelfLinkerOp(int bc) {
	return (bc >= _self_linker_op && bc < _self_linker_limit);
    }

    /// Format definitions.

    static private final byte[][] BC_LENGTH  = new byte[2][_bytecode_limit];
    static private final byte[][] BC_INDEX   = new byte[2][_bytecode_limit];
    static private final byte[][] BC_TAG     = new byte[2][_bytecode_limit];
    static private final byte[][] BC_BRANCH  = new byte[2][_bytecode_limit];
    static private final byte[][] BC_SLOT    = new byte[2][_bytecode_limit];
    static private final byte[][] BC_CON     = new byte[2][_bytecode_limit];
    static private final String[] BC_NAME    = new String[_bytecode_limit]; // debug only
    static private final String[][] BC_FORMAT  = new String[2][_bytecode_limit]; // debug only
    static {
	for (int i = 0; i < _bytecode_limit; i++) {
	    BC_LENGTH[0][i] = -1;
	    BC_LENGTH[1][i] = -1;
	}
	def("b", _nop, _dconst_1);
	def("bx", _bipush);
	def("bxx", _sipush);
	def("bk", _ldc);				// do not pack
	def("bkk", _ldc_w, _ldc2_w);		// do not pack
	def("blwbll", _iload, _aload);
	def("b", _iload_0, _saload);
	def("blwbll", _istore, _astore);
	def("b", _istore_0, _lxor);
	def("blxwbllxx", _iinc);
	def("b", _i2l, _dcmpg);
	def("boo", _ifeq, _jsr);			// pack oo
	def("blwbll", _ret);
	def("", _tableswitch, _lookupswitch);	// pack all ints, omit padding
	def("b", _ireturn, _return);
	def("bkf", _getstatic, _putfield);		// pack kf (base=Field)
	def("bkm", _invokevirtual, _invokestatic);	// pack kn (base=Method)
	def("bkixx", _invokeinterface);		// pack ki (base=IMethod), omit xx
	def("", _xxxunusedxxx);
	def("bkc", _new);				// pack kc
	def("bx", _newarray);
	def("bkc", _anewarray);			// pack kc
	def("b", _arraylength, _athrow);
	def("bkc", _checkcast, _instanceof);	// pack kc
	def("b", _monitorenter, _monitorexit);
	def("", _wide);
	def("bkcx", _multianewarray);		// pack kc
	def("boo", _ifnull, _ifnonnull);		// pack oo
	def("boooo", _goto_w, _jsr_w);		// pack oooo
	for (int i = 0; i < _bytecode_limit; i++) {
	    //System.out.println(i+": l="+BC_LENGTH[0][i]+" i="+BC_INDEX[0][i]);
	    if(D)assert0(BC_LENGTH[0][i] != -1);
	}

	String names =
  "nop aconst_null iconst_m1 iconst_0 iconst_1 iconst_2 iconst_3 iconst_4 "+
  "iconst_5 lconst_0 lconst_1 fconst_0 fconst_1 fconst_2 dconst_0 dconst_1 "+
  "bipush sipush ldc ldc_w ldc2_w iload lload fload dload aload iload_0 "+
  "iload_1 iload_2 iload_3 lload_0 lload_1 lload_2 lload_3 fload_0 fload_1 "+
  "fload_2 fload_3 dload_0 dload_1 dload_2 dload_3 aload_0 aload_1 aload_2 "+
  "aload_3 iaload laload faload daload aaload baload caload saload istore "+
  "lstore fstore dstore astore istore_0 istore_1 istore_2 istore_3 lstore_0 "+
  "lstore_1 lstore_2 lstore_3 fstore_0 fstore_1 fstore_2 fstore_3 dstore_0 "+
  "dstore_1 dstore_2 dstore_3 astore_0 astore_1 astore_2 astore_3 iastore "+
  "lastore fastore dastore aastore bastore castore sastore pop pop2 dup "+
  "dup_x1 dup_x2 dup2 dup2_x1 dup2_x2 swap iadd ladd fadd dadd isub lsub "+
  "fsub dsub imul lmul fmul dmul idiv ldiv fdiv ddiv irem lrem frem drem "+
  "ineg lneg fneg dneg ishl lshl ishr lshr iushr lushr iand land ior lor "+
  "ixor lxor iinc i2l i2f i2d l2i l2f l2d f2i f2l f2d d2i d2l d2f i2b i2c "+
  "i2s lcmp fcmpl fcmpg dcmpl dcmpg ifeq ifne iflt ifge ifgt ifle if_icmpeq "+
  "if_icmpne if_icmplt if_icmpge if_icmpgt if_icmple if_acmpeq if_acmpne "+
  "goto jsr ret tableswitch lookupswitch ireturn lreturn freturn dreturn "+
  "areturn return getstatic putstatic getfield putfield invokevirtual "+
  "invokespecial invokestatic invokeinterface xxxunusedxxx new newarray "+
  "anewarray arraylength athrow checkcast instanceof monitorenter "+
  "monitorexit wide multianewarray ifnull ifnonnull goto_w jsr_w ";
	for (int bc = 0; names.length() > 0; bc++) {
	    int sp = names.indexOf(' ');
	    BC_NAME[bc] = names.substring(0, sp);
	    names = names.substring(sp+1);
	}
    }
    public static String byteName(int bc) {
	String iname;
	if (bc < BC_NAME.length && BC_NAME[bc] != null) {
	    iname = BC_NAME[bc];
	} else if (isSelfLinkerOp(bc)) {
	    int idx = (bc - _self_linker_op);
	    boolean isSuper = (idx >= _self_linker_super_flag);
	    if (isSuper)  idx -= _self_linker_super_flag;
	    boolean isAload = (idx >= _self_linker_aload_flag);
	    if (isAload)  idx -= _self_linker_aload_flag;
	    int origBC = _first_linker_op + idx;
	    if(D)assert0(origBC >= _first_linker_op && origBC <= _last_linker_op);
	    iname = BC_NAME[origBC];
	    iname += (isSuper ? "_super" : "_this");
	    if (isAload)  iname = "aload_0&" + iname;
	    iname = "*"+iname;
	} else if (isInvokeInitOp(bc)) {
	    int idx = (bc - _invokeinit_op);
	    switch (idx) {
	    case _invokeinit_self_option:
		iname = "*invokespecial_init_this"; break;
	    case _invokeinit_super_option:
		iname = "*invokespecial_init_super"; break;
	    default:
		if(D)assert0(idx == _invokeinit_new_option);
		iname = "*invokespecial_init_new"; break;
	    }
	} else {
	    switch (bc) {
	    case _ildc:  iname = "*ildc"; break;
	    case _fldc:  iname = "*fldc"; break;
	    case _ildc_w:  iname = "*ildc_w"; break;
	    case _fldc_w:  iname = "*fldc_w"; break;
	    case _dldc2_w:  iname = "*dldc2_w"; break;
	    case _dup_pattern_op:  iname = "*dup_pattern"; break;
	    case _invoke_pattern_op:  iname = "*invoke_pattern"; break;
	    case _end_marker:  iname = "*end"; break;
	    default:  iname = "*bc#"+bc; break;
	    }
	}
	return iname;
    }
    private static int BW = 4;  // width of classification field
    private static void def(String fmt, int bc) {
	def(fmt, bc, bc);
    }
    private static void def(String fmt, int from_bc, int to_bc) {
	String[] fmts = { fmt, null };
	if (fmt.indexOf('w') > 0) {
	    fmts[1] = fmt.substring(fmt.indexOf('w'));
	    fmts[0] = fmt.substring(0, fmt.indexOf('w'));
	}
	for (int w = 0; w <= 1; w++) {
	    fmt = fmts[w];
	    if (fmt == null)  continue;
	    int length = fmt.length();
	    int index  = Math.max(0, fmt.indexOf('k'));
	    int tag    = CONSTANT_None;
	    int branch = Math.max(0, fmt.indexOf('o'));
	    int slot   = Math.max(0, fmt.indexOf('l'));
	    int con    = Math.max(0, fmt.indexOf('x'));
	    if (index > 0 && index+1 < length) {
		switch (fmt.charAt(index+1)) {
		    case 'c': tag = CONSTANT_Class; break;
		    case 'k': tag = CONSTANT_Literal; break;
		    case 'f': tag = CONSTANT_Fieldref; break;
		    case 'm': tag = CONSTANT_Methodref; break;
		    case 'i': tag = CONSTANT_InterfaceMethodref; break;
		}
		if(D)assert0(tag != CONSTANT_None);
	    } else if (index > 0 && length == 2) {
		if(D)assert0(from_bc == _ldc);
		tag = CONSTANT_Literal;  // _ldc opcode only
	    }
	    for (int bc = from_bc; bc <= to_bc; bc++) {
		BC_FORMAT[w][bc] = fmt;
		if(D)assert0(BC_LENGTH[w][bc] == -1);
		BC_LENGTH[w][bc] = (byte) length;
		BC_INDEX[w][bc]  = (byte) index;
		BC_TAG[w][bc]    = (byte) tag;
		if(D)assert0(!(index == 0 && tag != CONSTANT_None));
		BC_BRANCH[w][bc] = (byte) branch;
		BC_SLOT[w][bc]   = (byte) slot;
		if(D)assert0(branch == 0 || slot == 0);   // not both branch & local
		if(D)assert0(branch == 0 || index == 0);  // not both branch & cp
		if(D)assert0(slot == 0   || index == 0);  // not both local & cp
		BC_CON[w][bc]    = (byte) con;
	    }
	}
    }

    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }
}
