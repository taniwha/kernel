#include "types.h"
#include "cpu.h"
#include "s386.h"
#include "isa.h"
#include "__main.h"

typedef void (*FUNC)(void);
extern FUNC djgpp_first_ctor[] __asm__("djgpp_first_ctor");
extern FUNC djgpp_last_ctor[] __asm__("djgpp_last_ctor");
extern ulong top_of_stack[] asm("top_of_stack");

struct {
	Tss tss;
	VMStack vmstack;
	char bitmap[8192];
} tss;

Descriptor gdt[NUM_DESCRIPTORS]={
	{pseudo:	PSEUDO_DESCRIPTOR(&gdt,NUM_DESCRIPTORS*sizeof(Descriptor)-1)},
	{_segment:	_MAKE_DESCRIPTOR(0,0xffffffff>>12,dt_codeER,pl0,1,0,1,1)},
	{_segment:	_MAKE_DESCRIPTOR(0,0xffffffff>>12,dt_dataRW,pl0,1,0,1,1)},
	{_segment:	_MAKE_DESCRIPTOR(&gdt,NUM_DESCRIPTORS*sizeof(Descriptor)-1,dt_dataRW,pl0,1,0,1,0)},
	{_segment:	_MAKE_DESCRIPTOR(&tss,sizeof(tss),dt_tss32,pl0,1,0,0,0)},
};

#define INT(x) _X_##x
void INT(div)(void); void INT(deb)(void); void INT(nmi)(void); void INT(brk)(void);
void INT(ovf)(void); void INT(bnd)(void); void INT(iop)(void); void INT(cna)(void);
void INT(dbl)(void); void INT(cso)(void); void INT(its)(void); void INT(snp)(void);
void INT(stk)(void); void INT(gpf)(void); void INT(pgf)(void); void INT(r0f)(void);
void INT(cop)(void); void INT(alg)(void); void INT(r12)(void); void INT(r13)(void);
void INT(r14)(void); void INT(r15)(void); void INT(r16)(void); void INT(r17)(void);
void INT(r18)(void); void INT(r19)(void); void INT(r1a)(void); void INT(r1b)(void);
void INT(r1c)(void); void INT(r1d)(void); void INT(r1e)(void); void INT(r1f)(void);

void INT(irq0)(void); void INT(irq1)(void); void INT(irq2)(void); void INT(irq3)(void);
void INT(irq4)(void); void INT(irq5)(void); void INT(irq6)(void); void INT(irq7)(void);
void INT(irq8)(void); void INT(irq9)(void); void INT(irqa)(void); void INT(irqb)(void);
void INT(irqc)(void); void INT(irqd)(void); void INT(irqe)(void); void INT(irqf)(void);

void INT(sw30)(void);

Descriptor idt[]={
	/* cpu exceptions */
	{_gate:	_MAKE_GATE(INT(div),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(deb),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(nmi),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(brk),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(ovf),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(bnd),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(iop),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(cna),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(dbl),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(cso),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(its),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(snp),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(stk),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(gpf),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(pgf),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r0f),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(cop),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(alg),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r12),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r13),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r14),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r15),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r16),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r17),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r18),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r19),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r1a),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r1b),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r1c),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r1d),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r1e),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(r1f),8,0,dt_interruptGate32,0,1)},
	/* hardware interrupts */
	{_gate:	_MAKE_GATE(INT(irq0),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irq1),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irq2),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irq3),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irq4),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irq5),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irq6),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irq7),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irq8),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irq9),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irqa),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irqb),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irqc),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irqd),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irqe),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(irqf),8,0,dt_interruptGate32,0,1)},
	/* misc s/w intterrupts */
	{_gate:	_MAKE_GATE(INT(sw30),8,0,dt_interruptGate32,0,1)},
/*	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
	{_gate:	_MAKE_GATE(INT(),8,0,dt_interruptGate32,0,1)},
*/};
#undef INT

PseudoDescriptor idt_pseudo=PSEUDO_DESCRIPTOR(idt,sizeof(idt)-1);

static void convertDescriptor(Descriptor *des)
{
	switch (des->_segment.type) {
	case dt_unused:
	case dt_tss16:
	case dt_ldt:
	case dt_busytss16:
	case dt_tss32:
	case dt_busytss32:
	case dt_dataRO:
	case dt_dataROA:
	case dt_dataRW:
	case dt_dataRWA:
	case dt_dataROU:
	case dt_dataROUA:
	case dt_dataRWU:
	case dt_dataRWUA:
	case dt_codeEO:
	case dt_codeEOA:
	case dt_codeER:
	case dt_codeERA:
	case dt_codeEOC:
	case dt_codeEOCA:
	case dt_codeERC:
	case dt_codeERCA:
		asm("
			movw %2,%%ax
			xchgw %%ax,%0
			movb %%al,%1
			"
			:"=m"(*((char*)des+5)),
			 "=m"(*((char*)des+7))
			:"m"(*((char*)des+6))
			:"%eax");
		break;
	case dt_callGate16:
	case dt_taskGate:
	case dt_interruptGate16:
	case dt_trapGate16:
	case dt_callGate32:
	case dt_interruptGate32:
	case dt_trapGate32:
		asm("
			movw %3,%%ax
			xchgw %%ax,%0
			xchgw %%ax,%1
			xchgb %%al,%%ah
			movw %%ax,%2
			"
			:"=m"(*((char*)des+2)),
			 "=m"(*((char*)des+6)),
			 "=m"(*((char*)des+4))
			:"m"(*((char*)des+4)),
			 "m"(*((char*)des+2))
			:"%eax");
		break;
	}
}

void __main(void)
{
	int i;
	for (i=1; i<sizeof(gdt)/sizeof(gdt[0]); i++) {
		convertDescriptor(&gdt[i]);
	}
	for (i=0; i<sizeof(idt)/sizeof(idt[0]); i++) {
		convertDescriptor(&idt[i]);
	}
	set_gdt(&gdt[0].pseudo);
	asm("
		movw	%w0,%%ds
		movw	%w0,%%es
		movw	%w0,%%fs
		movw	%w0,%%gs
		movw	%w0,%%ss
		pushl	%1
		pushl	$1f
		lret
	1:	"
		:
		:"r"(0x10),"i"(0x08)
	);
	set_idt(&idt_pseudo);
	tss.tss.esp0=(ulong)top_of_stack-256;
	tss.tss.ss0=0x10;
	tss.tss.esp1=0;
	tss.tss.ss1=0x10;
	tss.tss.esp2=0;
	tss.tss.ss2=0x10;
	asm("movl %%cr3,%k0":"=r"(tss.tss.cr3));
/*	tss.tss.ldtr=0;
	tss.tss.trap=0;*/
	tss.tss.bitmap=(int)&tss.bitmap-(int)&tss;
	tss.vmstack.intsp=INTSTACKSPACE*4;
	set_tr(0x20);
	gdt[4].segment.type=dt_tss32;
	jmptss(0x20);
	clearnt();

	outb(0xf1,0);

	outb(ICU1,0x11);
	outb(ICU1+1,0x20);
	outb(ICU1+1,4);
	outb(ICU1+1,1);
	outb(ICU1+1,0xff);

	outb(ICU2,0x11);
	outb(ICU2+1,0x28);
	outb(ICU2+1,2);
	outb(ICU2+1,1);
	outb(ICU2+1,0xff);
	inton();
	for (i=0; i<djgpp_last_ctor-djgpp_first_ctor; i++)
		djgpp_first_ctor[i]();
}
