.file "inter.s"
#include "traps.h"

INTR(div)
		pushl	$0
		TRAP(T_div)

INTR(deb)
		pushl	$0
		TRAP(T_deb)

INTR(nmi)
		pushl	$0
		TRAP(T_nmi)

INTR(brk)
		pushl	$0
		TRAP(T_brk)

INTR(ovf)
		pushl	$0
		TRAP(T_ovf)

INTR(bnd)
		pushl	$0
		TRAP(T_bnd)

INTR(iop)
		pushl	$0
		TRAP(T_iop)

INTR(cna)
		pushl	$0
		TRAP(T_cna)

INTR(dbl)
		TRAP(T_dbl)

INTR(cso)
		pushl	$0
		TRAP(T_cso)

INTR(its)
		TRAP(T_its)

INTR(snp)
		TRAP(T_snp)

INTR(stk)
		TRAP(T_stk)

INTR(gpf)
		testl	$0x20000,12(%esp)
		jnz		vm86
		TRAP(T_gpf)

INTR(pgf)
		TRAP(T_pgf)

INTR(r0f)
		pushl	$0
		TRAP(T_r0f)

INTR(cop)
		pushl	$0
		TRAP(T_cop)

INTR(alg)
		TRAP(T_alg)

INTR(r12)
		pushl	$0
		TRAP(T_r12)

INTR(r13)
		pushl	$0
		TRAP(T_r13)

INTR(r14)
		pushl	$0
		TRAP(T_r14)

INTR(r15)
		pushl	$0
		TRAP(T_r15)

INTR(r16)
		pushl	$0
		TRAP(T_r16)

INTR(r17)
		pushl	$0
		TRAP(T_r17)

INTR(r18)
		pushl	$0
		TRAP(T_r18)

INTR(r19)
		pushl	$0
		TRAP(T_r19)

INTR(r1a)
		pushl	$0
		TRAP(T_r1a)

INTR(r1b)
		pushl	$0
		TRAP(T_r1b)

INTR(r1c)
		pushl	$0
		TRAP(T_r1c)

INTR(r1d)
		pushl	$0
		TRAP(T_r1d)

INTR(r1e)
		pushl	$0
		TRAP(T_r1e)

INTR(r1f)
		pushl	$0
		TRAP(T_r1f)

		.align	2
alltraps:.globl alltraps
		pushal
		pushl	%ds
		pushl	%es
		pushl	%fs
		pushl	%gs
		movw	$0x10,%ax
		movw	%ax,%ds
		movw	%ax,%es
		call	_trap
		popl	%gs
		popl	%fs
		popl	%es
		popl	%ds
		popal
		nop
		addl	$8,%esp
		iret

		.align	2
		.globl	set_int_vector
set_int_vector:
		pushl	%ebx
		pushfl
		cli
		movzbl	%al,%ebx
		movw	%dx,_idt+0(,%ebx,8)
		rorl	$16,%edx
		movw	%dx,_idt+6(,%ebx,8)
		rorl	$16,%edx
		movw	%cx,_idt+2(,%ebx,8)
		popfl
		popl	%ebx
		ret

		.align	2
		.globl	get_int_vector
get_int_vector:
		pushl	%ebx
		movzbl	%al,%ebx
		movl	_idt+4(,%ebx,8),%eax
		movw	_idt+0(,%ebx,8),%ax
		movw	_idt+2(,%ebx,8),%dx
		popl	%ebx
		ret
