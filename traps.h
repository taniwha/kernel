#define T_div	0x00
#define T_deb	0x01
#define T_nmi	0x02
#define T_brk	0x03
#define T_ovf	0x04
#define T_bnd	0x05
#define T_iop	0x06
#define T_cna	0x07
#define T_dbl	0x08
#define T_cso	0x09
#define T_its	0x0a
#define T_snp	0x0b
#define T_stk	0x0c
#define T_gpf	0x0d
#define T_pgf	0x0e
#define T_r0f	0x0f
#define T_cop	0x10
#define T_alg	0x11
#define T_r12	0x12
#define T_r13	0x13
#define T_r14	0x14
#define T_r15	0x15
#define T_r16	0x16
#define T_r17	0x17
#define T_r18	0x18
#define T_r19	0x19
#define T_r1a	0x1a
#define T_r1b	0x1b
#define T_r1c	0x1c
#define T_r1d	0x1d
#define T_r1e	0x1e
#define T_r1f	0x1f

#define _INTR(x) __X_##x
#define INTR(x) .align 2;__X_##x:.globl __X_##x
#define TRAP(a) pushl $##a ; jmp alltraps
#define IRQ(a) \
		pushal				;\
		pushl	%ds			;\
		pushl	%es			;\
		movw	$0x10,%ax	;\
		movw	%ax,%ds		;\
		movw	%ax,%es		;\
		pushl	$##a		;\
		jmp		irq
