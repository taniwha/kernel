		.text
startup:.globl	startup
		movl	$top_of_stack,%esp
		# turn of the floppy drive if it is on
		movw	$0x3f2,%dx
		movb	$0xc,%al
		outb	%al,%dx
		# setup for a warm boot
		movw	$0x1234,0x472
		# clear out bss
		leal	end,%ecx
		leal	edata,%edi
		subl	%edi,%ecx
		shrl	$2,%ecx
		xorl	%eax,%eax
		cld
		rep
		stosl
		# start up the system with interrupts disabled
		call	_main
		sti
1:		hlt
		jmp		1b
