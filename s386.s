		.text

		.align	4
_inton:	.globl _inton
		sti
		ret

		.align	2
_intoff:.globl _intoff
		cli
		ret

		.align	2
_lidt:	.globl _lidt
		movl	4(%esp),%eax
		movl	%eax,idesc+2
		movl	8(%esp),%eax
		movw	%ax,idesc
		lidt	idesc
		ret
		.align	2
idesc:	.word	0
		.long	0

		.align	2
_lgdt:	.globl _lgdt
		movl	4(%esp),%eax
		movl	%eax,gdesc+2
		movl	8(%esp),%eax
		movw	%ax,gdesc
		lgdt	gdesc
		jmp		1f
		.align	2,0x90
1:		movw	$0x10,%ax
		movw	%ax,%ds
		movw	%ax,%es
		movw	%ax,%fs
		movw	%ax,%gs
		movw	%ax,%ss
		pushl	(%esp)		# duplicate the return address
		movl	$8,4(%esp)	# convert return address to a far pointer
		lret
		.align	2
gdesc:	.word	0
		.long	0

		.align	2
_lldt:	.globl _lldt
		lldt	4(%esp)
		ret

		.align	2
_ltr:	.globl _ltr
		ltr		4(%esp)
		ret

		.align	2
_jmptss:.globl _jmptss
		ljmp	(%esp)
		ret

		.align	2
_clearnt:.globl _clearnt
		pushfl
		popl	%eax
		andl	$0xffffbfff,%eax
		pushl	%eax
		popfl
		ret

		.align	2
_lcr3:	.globl _cr3
		movl	4(%esp),%eax
		movl	%eax,%cr3
		ret

		.align	2
_lcr0:	.globl _lcr0
		movl	4(%esp),%eax
		movl	%eax,%cr0
		ret

		.align	2
_outb:	.globl _outb
		movl	4(%esp),%edx
		movl	8(%esp),%eax
		outb	%al,%dx
		ret

		.align	2
_outw:	.globl _outw
		movl	4(%esp),%edx
		movl	8(%esp),%eax
		outw	%ax,%dx
		ret

		.align	2
_outl:	.globl _outl
		movl	4(%esp),%edx
		movl	8(%esp),%eax
		outl	%eax,%dx
		ret

		.align	2
_inb:	.globl _inb
		movl	4(%esp),%edx
		movl	8(%esp),%eax
		inb		%dx,%al
		ret

		.align	2
_inw:	.globl _inw
		movl	4(%esp),%edx
		movl	8(%esp),%eax
		inw		%dx,%ax
		ret

		.align	2
_inl:	.globl _inl
		movl	4(%esp),%edx
		movl	8(%esp),%eax
		inl		%dx,%eax
		ret

		.align	2
_outsb:	.globl _outsb
		pushl	%esi
		movl	8(%esp),%edx
		movl	12(%esp),%esi
		movl	16(%esp),%ecx
		cld
		rep
		outsb
		popl	%esi
		ret

		.align	2
_outsw:	.globl _outsw
		pushl	%esi
		movl	8(%esp),%edx
		movl	12(%esp),%esi
		movl	16(%esp),%ecx
		cld
		rep
		outsw
		popl	%esi
		ret

		.align	2
_outsl:	.globl _outsl
		pushl	%esi
		movl	8(%esp),%edx
		movl	12(%esp),%esi
		movl	16(%esp),%ecx
		cld
		rep
		outsl
		popl	%esi
		ret

		.align	2
_insb:	.globl _insb
		pushl	%edi
		movl	8(%esp),%edx
		movl	12(%esp),%edi
		movl	16(%esp),%ecx
		cld
		rep
		insb
		popl	%edi
		ret

		.align	2
_insw:	.globl _insw
		pushl	%edi
		movl	8(%esp),%edx
		movl	12(%esp),%edi
		movl	16(%esp),%ecx
		cld
		rep
		insw
		popl	%edi
		ret

		.align	2
_insl:	.globl _insl
		pushl	%edi
		movl	8(%esp),%edx
		movl	12(%esp),%edi
		movl	16(%esp),%ecx
		cld
		rep
		insl
		popl	%edi
		ret

		.align	4,0
