#include "types.h"
#include "rmcb.h"
#include "screen.h"

static void (*emulate_int[256])(RMCB_Regs *);

extern ushort rmcb_ints[256];
asm(".set _rmcb_ints, 0x600");

extern FarPointer ivects[256];
asm(".set _ivects, 0;.globl _ivects");	/* real mode interrupt vector table at 0 linear */

static ulong hooked_rm_ints[8]={
	0x0000ffff,	0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
};

static inline int is_int_hooked(int intr)
{
	int hooked;

	asm("bt %1,%2; setcb %b0":"=g"(hooked):"r"(intr),"m"(*hooked_rm_ints),"0"(0));
	return hooked;
}

static inline void hook_int(int intr)
{
	asm("bts %0,%1": :"r"(intr),"m"(*hooked_rm_ints));
	ivects[intr].offset=intr*2;
	ivects[intr].segment=RMCB_int_seg;
}

void (*get_rmcb_int(int intr))(RMCB_Regs*)
{
	if (is_int_hooked(intr)) {
		return emulate_int[intr];
	}
	return 0;
}

void set_rmcb_int(int intr, void (*hook)(RMCB_Regs*))
{
	if (!is_int_hooked(intr)) {
		hook_int(intr);
	}
	emulate_int[intr]=hook;
}

void rmcb(RMCB_Regs regs)
{
	if (regs.cs==RMCB_int_seg && regs.eip<256*2) {
		/* it's one of the kernel hooked interrupts (eg int 0x21) */
		if (regs.eip&1) {
			/* barf */
		} else {
			emulate_int[regs.eip/2](&regs);
		}
	} else if (regs.cs*16+regs.eip==0xffff0) {
		/* reboot was attempted */
	} else {
		/* hmm, how to handle these ones? */
	}
}

static void unsupported(RMCB_Regs *regs)
{
	kprintf("Unsupported interrupt %x\n",regs->eip/2);
	kprintf("System halted (reset)\n");
	asm("cli; 1:hlt; jmp 1b");	/* nmi? */
}

static void __attribute__((constructor)) init_rmcb(void)
{
	int i;

	for (i=0; i<256; i++) {
		emulate_int[i]=unsupported;
		rmcb_ints[i]=0x30cd;	/* int 0x30 */
	}
	for (i=0; i<0x10; i++) {
		ivects[i].offset=i*2;
		ivects[i].segment=RMCB_int_seg;
	}
	/* for now, leave the BIOS interrupts (0x10-0x1f) alone */
	for (i=0x20; i<0x100; i++) {
		ivects[i].offset=i*2;
		ivects[i].segment=RMCB_int_seg;
	}
}
