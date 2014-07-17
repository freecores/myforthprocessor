#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vm_version_i486.cpp	1.36 03/01/23 10:56:06 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vm_version_i486.cpp.incl"


int VM_Version::_cpu;
int VM_Version::_cpuFeatures;

static jint stubCode[30];

extern "C" {
    typedef void (*_getPsrInfo_stub_t)(jint *, jint *);
}
static _getPsrInfo_stub_t getPsrInfo_stub = NULL;


#define   EFL_AC  0x40000
#define  EFL_ID  0x200000
#define  CpuFamily_shift   8
#define  CpuFamily_mask    0xf
#define  ExtendedCpuFamily_shift   16
#define  ExtendedCpuFamily_mask    0xff

class VM_Version_StubGenerator: public StubCodeGenerator {
 public:

  VM_Version_StubGenerator(CodeBuffer *c) : StubCodeGenerator(c) {}

  address generate_getPsrInfo() {

    Label cpu386, cpu486, done;

    StubCodeMark mark(this, "VM_Version", "getPsrInfo_stub");
#   define __ _masm->

    address start = __ pc();

    //
    // void cpuInfo(jint *cpu, jint *featureFlags)
    //
    __ pushl(ebp);
    __ movl(ebp, esp);
    __ pushl(ebx);
    __ pushfd();		// preserve ebx and flags
    __ popl(eax);
    __ pushl(eax);
    __ movl(ecx, eax);
    __ xorl(edx,edx);		// initialize feature flags return to 0
    //
    // if we are unable to change the AC flag, we have a 386
    //
    __ xorl(eax, EFL_AC);
    __ pushl(eax);
    __ popfd();
    __ pushfd();
    __ popl(eax);
    __ cmpl(eax, ecx);
    __ jcc(Assembler::equal, cpu386);
    //
    // If we are unable to change the ID flag, we have a 486 which does
    // not support the "cpuid" instruction.
    //
    __ movl(eax, ecx);
    __ xorl(eax, EFL_ID);
    __ pushl(eax);
    __ popfd();
    __ pushfd();
    __ popl(eax);
    __ cmpl(ecx, eax);
    __ jcc(Assembler::equal, cpu486);

    //
    // at this point, we have a chip which supports the "cpuid" instruction
    //
    __ xorl(eax, eax);
    __ cpuid();
    __ orl(eax, eax);
    __ jcc(Assembler::equal, cpu486);	// if cpuid doesn't support an input value
					// value of at least 1, we give up and
					// assume a 486
    __ movl(eax, 1);
    __ cpuid();
    __ jmp(done);

    __ bind(cpu386);
    __ movl(eax, 3 << CpuFamily_shift);
    __ jmp(done);

    __ bind(cpu486);
    __ movl(eax, 4 << CpuFamily_shift);
    // fall-through

    //
    // store result and return
    //
    __ bind(done);
    __ movl(ecx, Address(ebp, 8));
    __ movl(Address(ecx, 0), eax);  // store cpu info
    __ movl(ecx, Address(ebp, 12));
    __ movl(Address(ecx, 0), edx);  // store feature flags
    __ popfd();
    __ popl(ebx);
    __ popl(ebp);
    __ ret(0);

#   undef __

    return start;
  };
};


void VM_Version::get_processor_features() {

  if (Use486InstrsOnly) {
    _cpu = 4;
    _cpuFeatures = 0;
  } else {
    jint cpu, featureFlags;

    getPsrInfo_stub(&cpu, &featureFlags);

    _cpu = (cpu >> CpuFamily_shift) & CpuFamily_mask;
    if (_cpu == 0xf)
      _cpu |= ((cpu >> ExtendedCpuFamily_shift) & ExtendedCpuFamily_mask) << 4;
    _cpuFeatures = featureFlags;
  }
  _supports_cx8 = supports_cmpxchg8();
#ifndef PRODUCT
  if (PrintMiscellaneous && Verbose) {
    tty->print_cr("Processor = 0x%x", _cpu);
    tty->print_cr("Processor feature flags = 0x%x", _cpuFeatures);
  }
#endif
}

void VM_Version::initialize() {
  ResourceMark rm;
  // Making this stub must be FIRST use of assembler
  CodeBuffer* c = new CodeBuffer(address(stubCode), sizeof(stubCode));
  VM_Version_StubGenerator g(c);
  getPsrInfo_stub = CAST_TO_FN_PTR(_getPsrInfo_stub_t, g.generate_getPsrInfo());

  get_processor_features();
}

void VM_Version_init() {
  VM_Version::initialize();
}
