		.file	"stacks.s"

		.text

		.set	stack_ptr,	8
		.set	function,	12
		.set	numargs,	16
		.set	args,		20

		.globl	_call_on_stack
_call_on_stack:
		pushl	%ebp
		movl	%esp,%ebp
		pushl	%esi
		pushl	%edi
		movl	numargs(%ebp),%ecx
		leal	args-4(%ebp,%ecx,4),%esi
		movl	stack_ptr(%ebp),%edi
		std
		rep;	movsl
		cld
		movl	%edi,stack_ptr(%ebp)
		popl	%edi
		popl	%esi
		movl	stack_ptr(%ebp),%esp
		call	function(%ebp)
		movl	%ebp,%esp
		popl	%ebp
		ret
