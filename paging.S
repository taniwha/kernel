		.file	"paging.s"
#include "traps.h"

		.text
page_fault_handler:
		jmp	_INTR(pgf)

		.align	2
pageing_init:
		movl	$page_fault_handler,%edx
		movw	%cs,%cx
		movb	$T_pgf,%al
		call	set_int_vector
		ret
