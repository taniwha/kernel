.file "irq.s"
#include "traps.h"

INTR(irq0)
	IRQ(0x00)
INTR(irq1)
	IRQ(0x01)
INTR(irq2)
	IRQ(0x02)
INTR(irq3)
	IRQ(0x03)
INTR(irq4)
	IRQ(0x04)
INTR(irq5)
	IRQ(0x05)
INTR(irq6)
	IRQ(0x06)
INTR(irq7)
	IRQ(0x07)
INTR(irq8)
	IRQ(0x08)
INTR(irq9)
	IRQ(0x09)
INTR(irqa)
	IRQ(0x0a)
INTR(irqb)
	IRQ(0x0b)
INTR(irqc)
	IRQ(0x0c)
INTR(irqd)
	IRQ(0x0d)
INTR(irqe)
	IRQ(0x0e)
INTR(irqf)
	IRQ(0x0f)

		.align	2
irq:
		call	_intr
		popl	%eax		
		popl	%es			
		popl	%ds			
		popal				
		nop					
		iret

		.align	2
		.globl	set_irq_vector
set_irq_vector:
		pushl	%ebx
		pushfl
		cli
		movzbl	%al,%ebx
		addl	$0x20,%ebx
		movw	%dx,_idt+0(,%ebx,8)
		rorl	$16,%edx
		movw	%dx,_idt+6(,%ebx,8)
		rorl	$16,%edx
		movw	%cx,_idt+2(,%ebx,8)
		popfl
		popl	%ebx
		ret

		.align	2
		.globl	get_irq_vector
get_irq_vector:
		pushl	%ebx
		movzbl	%al,%ebx
		addl	$0x20,%ebx
		movl	_idt+4(,%ebx,8),%eax
		movw	_idt+0(,%ebx,8),%ax
		movw	_idt+2(,%ebx,8),%dx
		popl	%ebx
		ret
