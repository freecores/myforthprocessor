/*
 * @(#)X86Registers.java	1.3 03/01/23 11:22:34
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.x86;

public class X86Registers {
  private static final int NUM_REGISTERS = 8;

  public static final X86Register EAX = new X86Register(0);
  public static final X86Register ECX = new X86Register(1);
  public static final X86Register EDX = new X86Register(2);
  public static final X86Register EBX = new X86Register(3);
  public static final X86Register ESP = new X86Register(4);
  public static final X86Register EBP = new X86Register(5);
  public static final X86Register ESI = new X86Register(6);
  public static final X86Register EDI = new X86Register(7);

  public static int getNumberOfRegisters() {
    return NUM_REGISTERS;
  }

  private static String[] registerNames;

  // Should only have this in non-runtime system?
  static {
    registerNames = new String[NUM_REGISTERS];

    registerNames[EAX.getNumber()] = "EAX";
    registerNames[ECX.getNumber()] = "ECX";
    registerNames[EDX.getNumber()] = "EDX";
    registerNames[EBX.getNumber()] = "EBX";
    registerNames[ESP.getNumber()] = "ESP";
    registerNames[EBP.getNumber()] = "EBP";
    registerNames[ESI.getNumber()] = "ESI";
    registerNames[EDI.getNumber()] = "EDI";
  }

  public static String getRegisterName(int regNum) {
    return registerNames[regNum];
  }
}
