		.file	"vm86.s"
#include "traps.h"

		.text

		.align	2
vm86:	.globl	vm86
		cli
		addl	$4,%esp
		pushl	%eax
		pushl	%ebx
		pushl	%ds
		pushl	%ebp
		movl	%esp,%ebp
		addl	$0x10,%ebp
		movw	$0x10,%ax
		movw	%ax,%ds
		movzwl	4(%ebp),%ebx
		shll	$4,%ebx
		addl	(%ebp),%ebx
		xorl	%eax,%eax
		jmp		inloop
fset:	orl		$0x80000000,%eax
inloop:	movb	(%ebx),%al
		incb	%ah
		incl	%ebx
		cmpb	$0x66,%al
		jz		fset
		cmpb	$0x9d,%al
		jz		dopopf
		cmpb	$0x9c,%al
		jz		dopushf
		cmpb	$0xfa,%al
		jz		docli
		cmpb	$0xfb,%al
		jz		dosti
		cmpb	$0xcd,%al
		jz		dointnn
		cmpb	$0xcf,%al
		jz		doiret
		cmpb	$0xf0,%al
		jz		dolock
		popl	%ebp
		popl	%ds
		popl	%ebx
		popl	%eax
		pushl	8(%esp)
		popf
		pushl	0
		TRAP(T_gpf)

#define FIXIP				 \
		movw	(%ebp),%bx	;\
		addb	%ah,%bl		;\
		adcb	$0,%bh		;\
		movw	%bx,(%ebp)
#define GETP(b,r)			 \
		movzwl	b+4(%ebp),r	;\
		shll	$4,r		;\
		addl	b(%ebp),r
#define LEAVEISR			 \
		popl	%ebp		;\
		popl	%ds			;\
		popl	%ebx		;\
		popl	%eax		;\
		iret

#define GS		0x20(%ebp)
#define FS		0x1c(%ebp)
#define DS		0x18(%ebp)
#define ES		0x14(%ebp)
#define SS		0x10(%ebp)
#define ESP		0x0c(%ebp)
#define EFLAGS	0x08(%ebp)
#define CS		0x04(%ebp)
#define EIP		0x00(%ebp)

#define F_CF	0x00000001
#define F_PF	0x00000004
#define F_AF	0x00000010
#define F_ZF	0x00000040
#define F_SF	0x00000080
#define F_TF	0x00000100
#define F_IF	0x00000200
#define F_DF	0x00000400
#define F_OF	0x00000800
#define F_IOPL	0x00003000
#define F_NT	0x00004000
#define F_RF	0x00010000
#define F_VM	0x00020000
#define F_AC	0x00040000

#define IOPL_SH	12

dopopf:
		FIXIP
		GETP(0xc,%ebx)					/* vm86 stack */
		movzwl	(%ebx),%eax				/* vm86 flags */
		movl	EFLAGS,%ebx				/* real flags */
		andw	$(F_IOPL|F_NT),%bx
		andw	$~(F_IOPL|F_NT),%ax
		orl		%ebx,%eax
		movl	%eax,EFLAGS
		movl	$2,%ebx
		add		%ebx,ESP
		andl	$~F_RF,EFLAGS
		LEAVEISR

dopushf:
		FIXIP
		movl	EFLAGS,%eax
		GETP(0xc,%ebx)					/* vm86 stack */
		movw	%ax,-2(%ebx)
		subl	$2,ESP
		andl	$~F_RF,EFLAGS
		LEAVEISR

docli:
		FIXIP
		movl	EFLAGS,%eax
		orl		$F_VM,%eax
		andl	$~(F_RF|F_IOPL|F_IF),%eax
		movl	%eax,EFLAGS
		LEAVEISR

dosti:
		FIXIP
		movl	EFLAGS,%eax
		orl		$(F_VM|F_IF),%eax
		andl	$~(F_RF|F_IOPL),%eax
		movl	%eax,EFLAGS
		LEAVEISR

#define sav_es		104
#define sav_ds		108
#define sav_fs		112
#define sav_gs		116
#define intsp		120
#define intstack	124

dointnn:
		# check for RM callback
		cmpb	$0x30,(%ebx)
		jne		normdointnn
		popl	%ebp
		popl	%ds
		popl	%ebx
		popl	%eax
		pushal
		call	_rmcb
		popal
		nop						# pesky 386 bug (mine suffers :( )
		iret

normdointnn:
		pushl	%edx
		pushl	%ecx
		GETP(0xc,%edx)			# get vm stack
		movw	EFLAGS,%cx		# move flags and qsir addres to vm stack
		movw	%cx,-2(%edx)
		movw	$0x40,-4(%edx)
		movw	$0xf0,-6(%edx)
		subl	$6,ESP			# adjust esp
		movw	EIP,%cx			# ip
# adjust ip
		incb	%ah
		addb	%ah,%cl
		adcb	$0,%ch
		movw	%cx,EIP
# get tss
		strw	%cx				# task segment selector
		andl	$0xfff8,%ecx	# convert to index
		movl	_gdt+2(%ecx),%edx # bits 0-23 of base
		shll	$8,%edx
		movb	_gdt+7(%ecx),%dl # bits 24-31 of base
		rorl	$8,%edx			# edx points to tss base
# push pl0 esp from tss to local stack (within tss)
		movl	intsp(%edx),%ecx
		subl	$4,%ecx
		movl	%ecx,intsp(%edx)
		pushl	4(%edx)			# task esp0
		popl	intstack(%edx,%ecx)
		movl	%esp,%ecx		# adjust stack for _vm86
		addl	$60,%ecx
		movl	%ecx,4(%edx)	# task esp0
# test for 0, if so called from _vm86
		orb		%ah,%ah
		movzbl	%al,%ecx
		jz		from_vm86
# get int vector from instruction stream
		movl	%esp,%ecx		# adjust stack for no _vm86
		addl	$24,%ecx
		movl	%ecx,4(%edx)	# task esp0
		movzbl	(%ebx),%ecx
from_vm86:
		movl	%ecx,%edx
# interrupt vector*4 = VM86 interrupt vector address
		shll	$2,%edx
		movl	%edx,sto2
		popl	%ecx
		popl	%edx
		xchgl	%edx,sto2
		movl	%ecx,sto1
		movl	%ebp,sto3
		popl	%ebp
		xchgl	%ebp,sto3
		popl	%ecx			# saved DS
		movl	%ecx,sto4
		popl	%ebx
		popl	%eax
		pushl	GS
		pushl	FS
		pushl	DS
		pushl	ES
		pushl	SS
		pushl	ESP
		movl	EFLAGS,%ecx
		orl		$F_VM,%ecx
		andl	$~(F_RF|F_NT|F_IOPL|F_IF|F_TF),%ecx
		pushl	%ecx
		movzwl	2(%edx),%ecx
		pushl	%ecx
		movzwl	(%edx),%ecx
		pushl	%ecx
		pushl	sto4
		movl	sto1,%ecx
		movl	sto2,%edx
		movl	sto3,%ebp
		popl	%ds
		iret

doiret:
		GETP(0xc,%eax)			# get vm stack
		movl	(%eax),%ebx		# get cs:ip
		orl		%ebx,%ebx
		jz		farretint
		cmpl	$0x004000f0,%ebx
		jz		normiret

		addl	$6,ESP
		movzwl	(%eax),%ebx
		movl	%ebx,EIP
		movzwl	2(%eax),%ebx
		movl	%ebx,CS
		movzwl	4(%eax),%ebx
		orl		$F_VM,%ebx
		andl	$~(F_RF|F_IOPL),%ebx
		movl	%ebx,EFLAGS
		popl	%ebp
		popl	%ds
		popl	%ebx
		popl	%eax
		iret

farretint:
		movl	%ebp,%eax			# our ebp
		popl	%ebp				# vm ebp
		popl	%ds					# vm ds
		pushl	%ebp				# vm ebp
		pushl	%eax				# our ebp
		movw	%ds,%bx				# vm ds
		movw	$0x10,%ax
		movw	%ax,%ds
		movl	%ebx,sto3			# vm ds
		popl	%ebp				# our ebp
		movl	ESP,%eax			# vm esp
		addl	$6,%eax				# skip pushes from qiret
		movl	%eax,sto4			# vm esp
		movl	EFLAGS,%eax			# vm flags
		movl	%eax,sto2
		jmp		niret

normiret:
		movw	4(%eax),%bx			# vm flags
		movl	%ebp,%eax			# our ebp
		popl	%ebp				# vm ebp
		popl	%ds					# vm ds
		pushl	%ebp				# vm ebp
		pushl	%eax				# our ebp
		movw	%bx,%ax				# vm flags
		movw	%ds,%bx				# vm ds
		pushl	$0x10
		popl	%ds
		movl	%eax,sto2			# vm flags
		movl	%ebx,sto3			# vm ds
		popl	%ebp				# our ebp
		xorl	%eax,%eax
		movl	%eax,sto4			# vm esp?
niret:
		pushl	%esi
		xorl	%esi,%esi
		orl		$0,0x28(%ebp)
/*if CS=0 then int 30h asked for segment save*/
		jnz		v86iret
		movl	ES,%eax
		movl	%eax,sav_es(%edx)
		movl	DS,%eax
		movl	%eax,sav_ds(%edx)
		movl	FS,%eax
		movl	%eax,sav_fs(%edx)
		movl	GS,%eax
		movl	%eax,sav_gs(%edx)
		movl	$8,%esi

v86iret:
		movl	%edx,sto1
		popl	%ebp
		xchg	(%esp),%ebp # %ebp is from vm86 but %esi is still on the stack
		strw	%ax
		andl	$0xfff8,%eax
		movl	_gdt+2(%eax),%edx
		shll	$8,%edx
		movb	_gdt+7(%eax),%dl
		rorl	$8,%edx				# edx points to tss base
		movl	4(%edx),%eax		# get our current stack begin
/* see if we have to balance the vm86 stack */
		testl	$F_VM,%ss:8(%eax,%esi)
		jz		stkadjd
		movl	sto4,%ebx			# vm esp
		orl		%ebx,%ebx
		jz		adjstk
/* balance vm86 stack */
		mov		%ebx,%ss:0xc(%eax,%esi)
		jmp		stkadjd
adjstk:	addl	$6,%ss:0xc(%eax,%esi)

stkadjd:movl	sto2,%ebx			# vm flags
		pushl	%ss:8(%eax,%esi)	# flags
		movl	%ebx,%ss:8(%eax,%esi) # vm flags
leaveflags:
		andl	$0x1fff,%ss:8(%eax,%esi) #vm flags (8086 part only)
		popl	%ebx
		andl	$0xffffe200,%ebx	# save 386 part of old flags (and IF)
		orl		%ebx,%ss:8(%eax,%esi) #flags
		popl	%esi
		xchgl	(%esp),%eax
		pushl	%eax				# stack = ebx, new sp

		movl	intsp(%edx),%ebx
		movl	intstack(%ebx,%edx),%eax
		addl	$4,%ebx
		movl	%ebx,intsp(%edx)
		movl	%eax,4(%edx)

		popl	%ebx
		movl	sto1,%edx
		movw	sto3,%ds			# vm ds
		popl	%eax
		xchgl	(%esp),%eax
		popl	%esp
		xchgl	4(%esp),%eax
		orl		%eax,%eax
		xchgl	4(%esp),%eax
		jnz		goiret
		addl	$8,%esp
goiret:
		andl	$~(F_RF|F_IOPL),8(%esp)
		iret

dolock:
		popl	%ebp
		popl	%ds
		popl	%ebx
		popl	%eax
		pushl	8(%esp)
		popf
		pushl	$0xffff
		TRAP(T_gpf)


		.align	2
__vm86:	.globl	__vm86
		/* build an int stack frame */
		subl	$8,%esp
		pushl	%ebp
		movl	%esp,%ebp
		pushl	12(%ebp)	/* return %eip */
		popl	4(%ebp)
		pushl	%cs			/* return %cs */
		popl	8(%ebp)
		pushfl				/* %flags */
		popl	12(%ebp)
		lesl	16(%ebp),%ebx
		popl	%ebp
		.align	4
INTR(sw30)
		cmpb	$0,%es:(%ebx)
		jz		nosegsav
		pushl	$0
		pushl	$0
nosegsav:
		pushl	%es:32(%ebx)
		pushl	%es:28(%ebx)
		pushl	%es:24(%ebx)
		pushl	%es:20(%ebx)
		pushl	%es:16(%ebx)
		pushl	%es:12(%ebx)
		xchgl	%es:8(%ebx),%eax
		orl		$0x20000,%eax
		andl	$0xfffecfff,%eax
		pushl	%eax
		xchgl	%es:8(%ebx),%eax
		pushl	$0
		pushl	$0
		movl	%esp,%ebp
		pushl	%eax
		pushl	%es:40(%ebx)		# ebx
		pushl	%ds
		pushl	%es:36(%ebx)		# ebp
		movw	$0x10,%ax
		movw	%ax,%ds
		movb	%es:4(%ebx),%al
		movb	$0xff,%ah
		jmp		dointnn

		.align	4
_vm86:	.globl	_vm86
		pushl	%ebp
		movl	%esp,%ebp
		subl	$44,%esp
		pushl	%gs
		pushl	%fs
		pushl	%es
		pushl	%ds
		pushal
		pushl	%ebp
		leal	-44(%ebp),%edi
		pushl	%ss
		pushl	%edi
		movl	8(%ebp),%esi
		movl	$11,%ecx
		rep
		movsl
		movl	(%esi),%edx
		movl	4(%esi),%ecx
		movl	8(%esi),%eax
		movl	12(%esi),%edi
		movl	16(%esi),%esi
		call	__vm86
		pushfl						# save vm eflags (?)
		xchgl	%ebp,12(%esp)		# save vm ebp and get pm ebp
		pushl	%esi				# temp save esi
		movl	-60(%ebp),%esi		# get data segment selector
		movw	%si,%ds
		movw	$0x10,%si
		movw	%si,%es				# linear address space
		movl	8(%ebp),%esi		# get address of vmblock
		popl	60(%esi)			# save vm esi
		movl	%edi,56(%esi)		# save vm edi
		movl	%eax,52(%esi)		# save vm eax
		movl	%ecx,48(%esi)		# save vm ecx
		movl	%edx,44(%esi)		# save vm edx
		movl	%ebx,40(%esi)		# save vm ebx
		popl	8(%esi)				# save vm eflags
		addl	$8,%esp
		popl	36(%esi)			# save vm ebp

		strw	%ax
		andl	$0xfff8,%eax
		movl	_gdt+2(%eax),%edx
		shll	$8,%edx
		movb	_gdt+7(%eax),%dl
		rorl	$8,%edx				# edx points to tss base

		movl	%es:sav_es(%edx),%eax
		movl	%eax,20(%esi)
		movl	%es:sav_ds(%edx),%eax
		movl	%eax,24(%esi)
		movl	%es:sav_fs(%edx),%eax
		movl	%eax,28(%esi)
		movl	%es:sav_gs(%edx),%eax
		movl	%eax,32(%esi)

		popal
		nop							# take care of buggy 386s
		popl	%ds
		popl	%es
		popl	%fs
		popl	%gs
		leave
		ret

		.align	4
init_vm86:
		pushl	%esi
		pushl	%edi
		leal	qsir_data,%esi
		leal	qsir_end,%ecx
		movl	$0x4f0,%edi
		subl	%esi,%ecx
		cld
		rep
		movsb
		popl	%edi
		popl	%esi
		ret

		.data
sto1:	.long	0
sto2:	.long	0
sto3:	.long	0
sto4:	.long	0
qsir_data:
#include "qsir.ah"
qsir_end:

		.section	.ctor
		.long	init_vm86
