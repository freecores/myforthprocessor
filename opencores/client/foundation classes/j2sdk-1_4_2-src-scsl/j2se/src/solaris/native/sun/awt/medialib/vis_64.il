/*
 * @(#)vis_64.il	4.2	98/09/09 SMI
 *
 * Copyright (c) 1996 - 1998, Sun Microsystems, Inc.
 */

! Copyright (C) 1998, Sun Microsystems, Inc.

! "@(#)vis_64.il	1.2	98/09/03 SMI"

#ifndef FGR_FFB_HLSIM
! @(#)vis_64.il	1.3	98/07/09 SMI
!
! This file is to be used in place of vis.il in 64-bit builds.

!--------------------------------------------------------------------------
! Pure edge handling instructions
!
! int vis_edge8(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge8,2
	edge8	%o0,%o1,%o0
	.end
!
! int vis_edge8l(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge8l,2
	edge8l	%o0,%o1,%o0
	.end
!
! int vis_edge16(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge16,2
	edge16	%o0,%o1,%o0
	.end
!
! int vis_edge16l(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge16l,2
	edge16l	%o0,%o1,%o0
	.end
!
! int vis_edge32(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge32,2
	edge32	%o0,%o1,%o0
	.end
!
! int vis_edge32l(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge32l,2
	edge32l	%o0,%o1,%o0
	.end

!--------------------------------------------------------------------
! Edge handling instructions with negative return values if cc set
!
! 29mar98: Change for 64-bit environment:
!	   Use %xcc instead of %icc, because the ptrs are now 64-bit.
!
! int vis_edge8cc(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge8cc,2
	edge8	%o0,%o1,%o0
	mov     0,%o1
	movgu   %xcc,-1024,%o1
	or      %o1,%o0,%o0
	.end
!
! int vis_edge8lcc(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge8lcc,2
	edge8l	%o0,%o1,%o0
	mov     0,%o1
	movgu   %xcc,-1024,%o1
	or      %o1,%o0,%o0
	.end
!
! int vis_edge16cc(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge16cc,2
	edge16	%o0,%o1,%o0
	mov     0,%o1
	movgu   %xcc,-1024,%o1
	or      %o1,%o0,%o0
	.end
!
! int vis_edge16lcc(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge16lcc,2
	edge16l	%o0,%o1,%o0
	mov     0,%o1
	movgu   %xcc,-1024,%o1
	or      %o1,%o0,%o0
	.end
!
! int vis_edge32cc(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge32cc,2
	edge32	%o0,%o1,%o0
	mov     0,%o1
	movgu   %xcc,-1024,%o1
	or      %o1,%o0,%o0
	.end
!
! int vis_edge32lcc(void */*frs1*/, void */*frs2*/);
!
	.inline vis_edge32lcc,2
	edge32l	%o0,%o1,%o0
	mov     0,%o1
	movgu   %xcc,-1024,%o1
	or      %o1,%o0,%o0
	.end

!--------------------------------------------------------------------
! Alignment instructions
!
! void *vis_alignaddr(void */*rs1*/, int /*rs2*/);
!
	.inline vis_alignaddr,2
	alignaddr	%o0,%o1,%o0
	.end
!
! void *vis_alignaddrl(void */*rs1*/, int /*rs2*/);
!
	.inline vis_alignaddrl,2
	alignaddrl	%o0,%o1,%o0
	.end
!
! double vis_faligndata(double /*frs1*/, double /*frs2*/);
!
	.inline vis_faligndata,4
	faligndata	%f0,%f2,%f0
	.end

!--------------------------------------------------------------------
! Partitioned comparison instructions
!
! int vis_fcmple16(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fcmple16,4
	fcmple16	%f0,%f2,%o0
	.end
!
! int vis_fcmpne16(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fcmpne16,4
	fcmpne16	%f0,%f2,%o0
	.end
!
! int vis_fcmple32(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fcmple32,4
	fcmple32	%f0,%f2,%o0
	.end
!
! int vis_fcmpne32(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fcmpne32,4
	fcmpne32	%f0,%f2,%o0
	.end
!
! int vis_fcmpgt16(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fcmpgt16,4
	fcmpgt16	%f0,%f2,%o0
	.end
!
! int vis_fcmpeq16(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fcmpeq16,4
	fcmpeq16	%f0,%f2,%o0
	.end
!
! int vis_fcmpgt32(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fcmpgt32,4
	fcmpgt32	%f0,%f2,%o0
	.end
!
! int vis_fcmpeq32(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fcmpeq32,4
	fcmpeq32	%f0,%f2,%o0
	.end

!--------------------------------------------------------------------
! Partitioned arithmetic
!
! double vis_fmul8x16(float /*frs1*/, double /*frs2*/);
!
	.inline vis_fmul8x16,3
	fmul8x16	%f1,%f2,%f0
	.end
!
! double vis_fmul8x16_dummy(float /*frs1*/, int /*dummy*/, double /*frs2*/);
!
	.inline vis_fmul8x16_dummy,4
	fmul8x16	%f1,%f4,%f0
	.end
!
! double vis_fmul8x16au(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fmul8x16au,2
	fmul8x16au	%f1,%f3,%f0
	.end
!
! double vis_fmul8x16al(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fmul8x16al,2
	fmul8x16al	%f1,%f3,%f0
	.end
!
! double vis_fmul8sux16(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fmul8sux16,4
	fmul8sux16	%f0,%f2,%f0
	.end

!
! double vis_fmul8ulx16(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fmul8ulx16,4
	fmul8ulx16	%f0,%f2,%f0
	.end
!
! double vis_fmuld8sux16(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fmuld8sux16,2
	fmuld8sux16	%f1,%f3,%f0
	.end

!
! double vis_fmuld8ulx16(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fmuld8ulx16,2
	fmuld8ulx16	%f1,%f3,%f0
	.end
!
! double vis_fpadd16(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fpadd16,4
	fpadd16	%f0,%f2,%f0
	.end
!
! float vis_fpadd16s(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fpadd16s,2
	fpadd16s	%f1,%f3,%f0
	.end
!
! double vis_fpadd32(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fpadd32,4
	fpadd32	%f0,%f2,%f0
	.end
!
! float vis_fpadd32s(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fpadd32s,2
	fpadd32s	%f1,%f3,%f0
	.end
!
! double vis_fpsub16(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fpsub16,4
	fpsub16	%f0,%f2,%f0
	.end
!
! float vis_fpsub16s(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fpsub16s,2
	fpsub16s	%f1,%f3,%f0
	.end
!
! double vis_fpsub32(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fpsub32,4
	fpsub32	%f0,%f2,%f0
	.end
!
! float vis_fpsub32s(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fpsub32s,2
	fpsub32s	%f1,%f3,%f0
	.end

!--------------------------------------------------------------------
! Pixel packing
!
! float vis_fpack16(double /*frs2*/);
!
	.inline vis_fpack16,2
	fpack16	%f0,%f0
	.end

!
! double vis_fpack16_pair(double /*frs2*/, double /*frs2*/);
!
	.inline vis_fpack16_pair,4
	fpack16	%f0,%f0
	fpack16	%f2,%f1
	.end
!
! void vis_st2_fpack16(double, double, double *)
!
	.inline vis_st2_fpack16,6
 	fpack16	%f0,%f0
 	fpack16	%f2,%f1
 	st	%f0,[%o2+0]
 	st	%f1,[%o2+4]
 	.end
!
! void vis_std_fpack16(double, double, double *)
!
	.inline vis_std_fpack16,6
	fpack16	%f0,%f0
	fpack16	%f2,%f1
	std	%f0,[%o2]
	.end
!
! void vis_st2_fpackfix(double, double, double *)
!
	.inline vis_st2_fpackfix,6
 	fpackfix %f0,%f0
 	fpackfix %f2,%f1
 	st	%f0,[%o2+0]
 	st	%f1,[%o2+4]
 	.end
!
! double vis_fpack16_to_hi(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fpack16_to_hi,4
	fpack16	%f2,%f0
	.end

! double vis_fpack16_to_lo(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fpack16_to_lo,4
	fpack16	%f2,%f3
	fmovs	%f3,%f1		/* without this, optimizer goes wrong */
	.end

!
! double vis_fpack32(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fpack32,4
	fpack32	%f0,%f2,%f0
	.end
!
! float vis_fpackfix(double /*frs2*/);
!
	.inline vis_fpackfix,2
	fpackfix	%f0,%f0
	.end
!
! double vis_fpackfix_pair(double /*frs2*/, double /*frs2*/);
!
	.inline vis_fpackfix_pair,4
	fpackfix	%f0,%f0
	fpackfix	%f2,%f1
	.end

!--------------------------------------------------------------------
! Motion estimation
!
! double vis_pdist(double /*frs1*/, double /*frs2*/, double /*frd*/);
!
!	.inline vis_pdist,6
!	std	%o4,[%sp+0x48]
!	ldd	[%sp+0x48],%f0
!	std	%o0,[%sp+0x48]
!	ldd	[%sp+0x48],%f4
!	std	%o2,[%sp+0x48]
!	ldd	[%sp+0x48],%f10
!	pdist	%f4,%f10,%f0
!	.end
!
!   Now accum (3rd arg) must be 1st arg, since it is also the ret val,
!   which must be %f0 (the equivalent %d0 fails to compile).
!   --How to deal with this? 
!   Define a new pixel distance VIS instruction with the accum as 1st 
!   arg.  Use a #define macro in a new vis_proto64.h to redefine the 
!   original vis_pdist() so that it invokes the new pixel distance
!   instruction.  That way, source code can remain unchanged, as long 
!   as it uses the new vis_proto.h:
!
! double vis_pxldist64(double accum /*frd*/, double pxls1 /*frs1*/, 
!		       double pxls2 /*frs2*/);
!
	.inline vis_pxldist64,6
	pdist	%f2,%f4,%f0
	.end

!--------------------------------------------------------------------
! Channel merging
!
! double vis_fpmerge(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fpmerge,2
	fpmerge	%f1,%f3,%f0
	.end

!--------------------------------------------------------------------
! Pixel expansion
!
! double vis_fexpand(float /*frs2*/);
!
	.inline vis_fexpand,1
	fexpand	%f1,%f0
	.end

! double vis_fexpand_hi(double /*frs2*/);
!
	.inline vis_fexpand_hi,2
	fexpand	%f0,%f0
	.end

! double vis_fexpand_lo(double /*frs2*/);
!
	.inline vis_fexpand_lo,2
	fexpand	%f1,%f0
	.end

!--------------------------------------------------------------------
! Bitwise logical operations
!
! double vis_fnor(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fnor,4
	fnor	%f0,%f2,%f0
	.end
!
! float vis_fnors(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fnors,2
	fnors	%f1,%f3,%f0
	.end
!
! double vis_fandnot(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fandnot,4
	fandnot1 %f0,%f2,%f0
	.end
!
! float vis_fandnots(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fandnots,2
	fandnot1s %f1,%f3,%f0
	.end
!
! double vis_fnot(double /*frs1*/);
!
	.inline vis_fnot,2
	fnot1	%f0,%f0
	.end
!
! float vis_fnots(float /*frs1*/);
!
	.inline vis_fnots,1
	fnot1s	%f1,%f0
	.end
!
! double vis_fxor(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fxor,4
	fxor	%f0,%f2,%f0
	.end
!
! float vis_fxors(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fxors,2
	fxors	%f1,%f3,%f0
	.end
!
! double vis_fnand(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fnand,4
	fnand	%f0,%f2,%f0
	.end
!
! float vis_fnands(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fnands,2
	fnands	%f1,%f3,%f0
	.end
!
! double vis_fand(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fand,4
	fand	%f0,%f2,%f0
	.end
!
! float vis_fands(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fands,2
	fands	%f1,%f3,%f0
	.end
!
! double vis_fxnor(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fxnor,4
	fxnor	%f0,%f2,%f0
	.end
!
! float vis_fxnors(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fxnors,2
	fxnors	%f1,%f3,%f0
	.end
!
! double vis_fsrc(double /*frs1*/);
!
	.inline vis_fsrc,2
	fsrc1	%f0,%f0
	.end
!
! float vis_fsrcs(float /*frs1*/);
!
	.inline vis_fsrcs,1
	fsrc1s	%f1,%f0
	.end
!
! double vis_fornot(double /*frs1*/, double /*frs2*/);
!
	.inline vis_fornot,4
	fornot1	%f0,%f2,%f0
	.end
!
! float vis_fornots(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fornots,2
	fornot1s %f1,%f3,%f0
	.end
!
! double vis_for(double /*frs1*/, double /*frs2*/);
!
	.inline vis_for,4
	for	%f0,%f2,%f0
	.end
!
! float vis_fors(float /*frs1*/, float /*frs2*/);
!
	.inline vis_fors,2
	fors	%f1,%f3,%f0
	.end

!
! double vis_fzero(/* void */)
!
	.inline	vis_fzero,0
	fzero	%f0
	.end
!
! float vis_fzeros(/* void */)
!
	.inline	vis_fzeros,0
	fzeros	%f0
	.end
!
! double vis_fone(/* void */)
!
	.inline	vis_fone,0
	fone	%f0
	.end
!
! float vis_fones(/* void */)
!
	.inline	vis_fones,0
	fones	%f0
	.end

!--------------------------------------------------------------------
! Partial store instructions
!
! vis_stdfa_ASI_PST8P(double frd, void *rs1, int rmask)
!
	.inline vis_stdfa_ASI_PST8P,5
	stda	%f0,[%o1]%o2,0xc0	! ASI_PST8_P
	.end
!
! vis_stdfa_ASI_PST8PL(double frd, void *rs1, int rmask)
!
	.inline vis_stdfa_ASI_PST8PL,5
	stda	%f0,[%o1]%o2,0xc8	! ASI_PST8_PL
	.end
!
! vis_stdfa_ASI_PST8P_int_pair(void *rs1, void *rs2, void *rs3, int rmask);
!
	.inline vis_stdfa_ASI_PST8P_int_pair,4
        ld	[%o0],%f4
        ld	[%o1],%f5
	stda	%f4,[%o2]%o3,0xc0	! ASI_PST8_P
	.end
!
! vis_stdfa_ASI_PST8S(double frd, void *rs1, int rmask)
!
	.inline vis_stdfa_ASI_PST8S,5
	stda	%f0,[%o1]%o2,0xc1	! ASI_PST8_S
	.end
!
! vis_stdfa_ASI_PST16P(double frd, void *rs1, int rmask)
!
	.inline vis_stdfa_ASI_PST16P,5
	stda	%f0,[%o1]%o2,0xc2	! ASI_PST16_P
	.end
!
! vis_stdfa_ASI_PST16S(double frd, void *rs1, int rmask)
!
	.inline vis_stdfa_ASI_PST16S,5
	stda	%f0,[%o1]%o2,0xc3	! ASI_PST16_S
	.end
!
! vis_stdfa_ASI_PST32P(double frd, void *rs1, int rmask)
!
	.inline vis_stdfa_ASI_PST32P,5
	stda	%f0,[%o1]%o2,0xc4	! ASI_PST32_P
	.end
!
! vis_stdfa_ASI_PST32S(double frd, void *rs1, int rmask)
!
	.inline vis_stdfa_ASI_PST32S,5
	stda	%f0,[%o1]%o2,0xc5	! ASI_PST32_S
	.end

!--------------------------------------------------------------------
! Short store instructions
!
!
! vis_stdfa_ASI_FL8P(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_FL8P,4
	stda	%f0,[%o1]0xd0	! ASI_FL8_P
	.end
!
! vis_stdfa_ASI_FL8P_index(double frd, void *rs1, long index)
!
	.inline vis_stdfa_ASI_FL8P_index,6
	stda	%f0,[%o1+%o2]0xd0 ! ASI_FL8_P
	.end
!
! vis_stdfa_ASI_FL8S(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_FL8S,4
	stda	%f0,[%o1]0xd1	! ASI_FL8_S
	.end
!
! vis_stdfa_ASI_FL16P(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_FL16P,4
	stda	%f0,[%o1]0xd2	! ASI_FL16_P
	.end

!
! vis_stdfa_ASI_FL16P_index(double frd, void *rs1, long index)
!
	.inline vis_stdfa_ASI_FL16P_index,6
	stda	%f0,[%o1+%o2]0xd2 ! ASI_FL16_P
	.end
!
! vis_stdfa_ASI_FL16S(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_FL16S,4
	stda	%f0,[%o1]0xd3	! ASI_FL16_S
	.end

!
! vis_stdfa_ASI_FL8PL(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_FL8PL,4
	stda	%f0,[%o1]0xd8	! ASI_FL8_PL
	.end
!
! vis_stdfa_ASI_FL8SL(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_FL8SL,4
	stda	%f0,[%o1]0xd9	! ASI_FL8_SL
	.end
!
! vis_stdfa_ASI_FL16PL(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_FL16PL,4
	stda	%f0,[%o1]0xda	! ASI_FL16_PL
	.end
!
! vis_stdfa_ASI_FL16SL(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_FL16SL,4
	stda	%f0,[%o1]0xdb	! ASI_FL16_SL
	.end

!--------------------------------------------------------------------
! Short load instructions
!
!
! double vis_lddfa_ASI_FL8P(void *rs1)
!
	.inline vis_lddfa_ASI_FL8P,1
	ldda	[%o0]0xd0,%f4	! ASI_FL8_P
	fmovd	%f4,%f0	        ! Compiler can clean this up
	.end
!
! double vis_lddfa_ASI_FL8P_index(void *rs1, long index)
!
	.inline vis_lddfa_ASI_FL8P_index,2
	ldda	[%o0+%o1]0xd0,%f4
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL8P_hi(void *rs1, vl_u32 index)
!
	.inline vis_lddfa_ASI_FL8P_hi,2
	sra     %o1,16,%o1
	ldda	[%o0+%o1]0xd0,%f4
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL8P_lo(void *rs1, vl_u32 index)
!
	.inline vis_lddfa_ASI_FL8P_lo,2
	sll     %o1,16,%o1
	sra     %o1,16,%o1
	ldda	[%o0+%o1]0xd0,%f4
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL8S(void *rs1)
!
	.inline vis_lddfa_ASI_FL8S,1
	ldda	[%o0]0xd1,%f4	! ASI_FL8_S
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL16P(void *rs1)
!
	.inline vis_lddfa_ASI_FL16P,1
	ldda	[%o0]0xd2,%f4	! ASI_FL16_P
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL16P_index(void *rs1, long index)
!
	.inline vis_lddfa_ASI_FL16P_index,2
	ldda	[%o0+%o1]0xd2,%f4 ! ASI_FL16_P
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL16S(void *rs1)
!
	.inline vis_lddfa_ASI_FL16S,1
	ldda	[%o0]0xd3,%f4	! ASI_FL16_S
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL8PL(void *rs1)
!
	.inline vis_lddfa_ASI_FL8PL,1
	ldda	[%o0]0xd8,%f4	! ASI_FL8_PL
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL8SL(void *rs1)
!
	.inline vis_lddfa_ASI_FL8SL,1
	ldda	[%o0]0xd9,%f4	! ASI_FL8_SL
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL16PL(void *rs1)
!
	.inline vis_lddfa_ASI_FL16PL,1
	ldda	[%o0]0xda,%f4	! ASI_FL16_PL
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL16SL(void *rs1)
!
	.inline vis_lddfa_ASI_FL16SL,1
	ldda	[%o0]0xdb,%f4	! ASI_FL16_SL
	fmovd	%f4,%f0
	.end

!--------------------------------------------------------------------
! Graphics status register
!
! unsigned int vis_read_gsr(void)
!
	.inline vis_read_gsr,0
	rd	%gsr,%o0
	.end
!
! void vis_write_gsr(unsigned int /* GSR */)
!
	.inline vis_write_gsr,1
	wr	%g0,%o0,%gsr
	.end

!--------------------------------------------------------------------
! Voxel texture mapping
!
! unsigned long vis_array8(unsigned long long /*rs1 */, int /*rs2*/)
!
	.inline	vis_array8,3
	array8	%o0,%o1,%o0
	.end
!
! unsigned long vis_array16(unsigned long long /*rs1*/, int /*rs2*/)
!
	.inline	vis_array16,3
	array16	%o0,%o1,%o0
	.end
!
! unsigned long vis_array32(unsigned long long /*rs1*/, int /*rs2*/)
!
	.inline	vis_array32,3
	array32	%o0,%o1,%o0
	.end

!--------------------------------------------------------------------
! Register aliasing and type casts
!
! float vis_read_hi(double /* frs1 */);
!
	.inline vis_read_hi,2
!	--In 64-bit ABI, hi of input is already in %f0 -- nothing to do!
!	--Doing nothing works OK with optimization turned off, but
!	  with "-xO4", it returns zero. So do some seemingly redundant
!	  register moves (this works with "-xO4"):
	fmovs	%f0,%f0
	.end
!
! float vis_read_lo(double /* frs1 */);
!
	.inline vis_read_lo,2
	fmovs	%f1,%f0		! %f0 = low word (frs1); return %f0;
	.end
!
! double vis_write_hi(double /* frs1 */, float /* frs2 */);
!
	.inline vis_write_hi,3
	fmovs	%f3,%f0		! %f3 = float frs2; return %f0:f1;
	.end
!
! double vis_write_lo(double /* frs1 */, float /* frs2 */);
!
	.inline vis_write_lo,3
	fmovs	%f3,%f1		! %f3 = float frs2; return %f0:f1;
	.end
!
! double vis_freg_pair(float /* frs1 */, float /* frs2 */);
!
	.inline vis_freg_pair,2
	fmovs	%f1,%f0		! %f1 = float frs1; put in hi;
	fmovs	%f3,%f1		! %f3 = float frs2; put in lo; return %f0:f1;
	.end
!
! float vis_to_float(unsigned /*value*/);
!
	.inline vis_to_float,1
	st	%o0,[%sp+2183]
	ld	[%sp+2183],%f0
	.end
!
! double vis_to_double(unsigned /*value1*/, unsigned /*value2*/);
!
	.inline vis_to_double,2
	st	%o0,[%sp+2183]
	ld	[%sp+2183],%f0
	st	%o1,[%sp+2183]
	ld	[%sp+2183],%f1
	.end
!
! double vis_to_double_dup(unsigned /*value*/);
!
	.inline vis_to_double_dup,1
	st	%o0,[%sp+2183]
	ld	[%sp+2183],%f1
	fmovs	%f1,%f0		! duplicate value
	.end
!
! double vis_ll_to_double(unsigned long long /*value*/);
!
	.inline vis_ll_to_double,2
	stx     %o0,[%sp+2183]
	ldd     [%sp+2183],%f0
        .end

!--------------------------------------------------------------------
! Contributions from Dave W. Phillips of SunSoft
!
! unsigned int vis_read_asi(void)
!
	.inline vis_read_asi,0
	rd	%asi,%o0
	.end
!
! void vis_write_asi(unsigned int /* ASI */)
!
	.inline vis_write_asi,1
	wr	%g0,%o0,%asi
	.end

!
! float vis_ldfa_ASI_REG(void *rs1)
!
	.inline vis_ldfa_ASI_REG,1
	lda	[%o0+0]%asi,%f4
	fmovs	%f4,%f0	        ! Compiler can clean this up
	.end
!
! float vis_ldfa_ASI_P(void *rs1)
!
	.inline vis_ldfa_ASI_P,1
	lda	[%o0]0x80,%f4	! ASI_P
	fmovs	%f4,%f0	        ! Compiler can clean this up
	.end
!
! float vis_ldfa_ASI_PL(void *rs1)
!
	.inline vis_ldfa_ASI_PL,1
	lda	[%o0]0x88,%f4	! ASI_PL
	fmovs	%f4,%f0	        ! Compiler can clean this up
	.end

!
! double vis_lddfa_ASI_REG(void *rs1)
!
	.inline vis_lddfa_ASI_REG,1
	ldda	[%o0+0]%asi,%f4
	fmovd	%f4,%f0	        ! Compiler can clean this up
	.end
!
! double vis_lddfa_ASI_P(void *rs1)
!
	.inline vis_lddfa_ASI_P,1
	ldda	[%o0]0x80,%f4	! ASI_P
	fmovd	%f4,%f0	        ! Compiler can clean this up
	.end
!
! double vis_lddfa_ASI_PL(void *rs1)
!
	.inline vis_lddfa_ASI_PL,1
	ldda	[%o0]0x88,%f4	! ASI_PL
	fmovd	%f4,%f0	        ! Compiler can clean this up
	.end

!
! vis_stfa_ASI_REG(float frs, void *rs1)
!
	.inline vis_stfa_ASI_REG,1
	sta	%f1,[%o1+0]%asi
	.end
!
! vis_stfa_ASI_P(float frs, void *rs1)
!
	.inline vis_stfa_ASI_P,1
	sta	%f1,[%o1]0x80	! ASI_P
	.end
!
! vis_stfa_ASI_PL(float frs, void *rs1)
!
	.inline vis_stfa_ASI_PL,1
	sta	%f1,[%o1]0x88	! ASI_PL
	.end

!
! vis_stdfa_ASI_REG(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_REG,1
	stda	%f0,[%o1+0]%asi
	.end
!
! vis_stdfa_ASI_P(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_P,1
	stda	%f0,[%o1]0x80	! ASI_P
	.end
!
! vis_stdfa_ASI_PL(double frd, void *rs1)
!
	.inline vis_stdfa_ASI_PL,1
	stda	%f0,[%o1]0x88	! ASI_PL
	.end
!
! unsigned short vis_lduha_ASI_REG(void *rs1)
!
	.inline vis_lduha_ASI_REG,1
	lduha	[%o0+0]%asi,%o0
	.end
!
! unsigned short vis_lduha_ASI_P(void *rs1)
!
	.inline vis_lduha_ASI_P,1
	lduha	[%o0]0x80,%o0	! ASI_P
	.end
!
! unsigned short vis_lduha_ASI_PL(void *rs1)
!
	.inline vis_lduha_ASI_PL,1
	lduha	[%o0]0x88,%o0	! ASI_PL
	.end
!
! unsigned short vis_lduha_ASI_P_index(void *rs1, long index)
!
	.inline vis_lduha_ASI_P_index,2
	lduha	[%o0+%o1]0x80,%o0	! ASI_P
	.end
!
! unsigned short vis_lduha_ASI_PL_index(void *rs1, long index)
!
	.inline vis_lduha_ASI_PL_index,2
	lduha	[%o0+%o1]0x88,%o0	! ASI_PL
	.end

!
! double vis_lddfa_ASI_FL16PL_index(void *rs1, long index)
!
	.inline vis_lddfa_ASI_FL16PL_index,2
	ldda	[%o0+%o1]0xda,%f4	! ASI_FL16_PL
	fmovd	%f4,%f0
	.end
!
! double vis_lddfa_ASI_FL8PL_index(void *rs1, long index)
!
	.inline vis_lddfa_ASI_FL8PL_index,2
	ldda	[%o0+%o1]0xd8,%f4	! ASI_FL8_PL
	fmovd	%f4,%f0
	.end
#endif
