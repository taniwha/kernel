		.file	"memory.s"

		.text
		.align	2
read_cmos:
		# write the index register (and nmi mask)
		pushl	%eax
		orb		$0x80,%al
		outb	%al,$0x70
		jmp		0
		inb		$0x71,%al
		jmp		0
		xchgl	%eax,(%esp)
		outb	%al,$0x70
		popl	%eax
		ret

		.align	2
		.globl	_read_cmos
_read_cmos:
		movb	4(%esp),%al
		call	read_cmos
		ret

		.align	2
		.globl	_write_cmos
_write_cmos:
		movb	4(%esp),%al
		movb	8(%esp),%ah
		orb		$0x80,%al
		outb	%al,$0x70
		jmp		0
		movb	%ah,%al
		outb	%al,$0x71
		jmp		0
		movb	4(%esp),%al
		outb	%al,$0x70
		ret

		.align	2
		.globl	_get_base_memory
_get_base_memory:
		xorl	%ecx,%ecx
		movb	$0x15,%al	# base memory low byte
		call	read_cmos
		movb	%al,%cl
		movb	$0x16,%al	# base memory high byte
		call	read_cmos
		movb	%al,%ch
		shll	$10,%ecx	# cmos base memory is in kb, convert to bytes
		movl	%ecx,%eax
		ret

		.align	2
		.globl	_get_extended_memory
_get_extended_memory:
		xorl	%ecx,%ecx
		movb	$0x17,%al	# extended memory low byte
		call	read_cmos
		movb	%al,%cl
		movb	$0x18,%al	# extended memory high byte
		call	read_cmos
		movb	%al,%ch
		shll	$10,%ecx	# cmos extended memory is in kb, convert to bytes
		movl	%ecx,%eax
		ret
