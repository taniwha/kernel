#include "types.h"
#include "keybdio.h"
#include "cpu.h"
#include "isa.h"
#include "traps.h"
#include "s386.h"
#include "screen.h"
#include "physmem.h"

#define printreg(x) kputs("%"#x"=");kputx(x);kputc(' ');
#define printsreg(x) kputs("%"#x"=");kputsx(x);kputc(' ');

typedef struct {
	ushort gs, :16, fs, :16, es, :16, ds;
	ulong edi, esi, ebp, dummy;
	ulong ebx, edx, ecx, eax;
	ulong fault; ushort ec, :16;
	ulong eip; ushort cs, :16; ulong eflags;
	/* these are valid only if the cpl is not 0 or the vm flag is set */
	ulong esp; ushort ss, :16;
	/* these are valid only if the vm flag is set */
	ushort vm_es, :16, vm_ds, :16, vm_fs, :16, vm_gs, :16;
} TrapRegs;

extern Descriptor gdt[];

extern char stack[];

void reboot(void);

static void dump_stack(ulong eflags, ushort ss, ulong esp)
{
	int i;
	if (eflags&0x00020000) {
		ushort sp=esp&0xffff;
		ushort *p=(ushort*)(((ulong)ss<<4)+sp);
		kputs("stack(ss:sp)="); kputsx(ss); kputc(':'); kputx(sp); kputc('\n');
		for (i=0; i<64; i++) {
			if (sp>=0xffff) break;
			kputsx(*p++);
			kputc(' ');
			sp+=2;
		}
	} else {
		Descriptor *des=&gdt[ss/8];
		if (des->segment.use32) {	/*32 bit segment*/
			ulong *p=(ulong*)(get_segment_base(des)+esp);
			kputs("stack(ss:esp)="); kputsx(ss); kputc(':'); kputx(esp); kputc('\n');
			for (i=0; i<64; i++) {
				if (linear_to_physical(p)==(ulong)-1) {
					kputc('\n');
					break;
				}
				kputx(*p++);
				if ((i+1)%8) {
					kputc(' ');
				} else {
					kputc('\n');
				}
			}
		} else {
			ushort sp=esp&0xffff;
			ushort *p=(ushort*)(get_segment_base(des)+sp);
			kputs("stack(ss:sp)="); kputsx(ss); kputc(':'); kputx(sp); kputc('\n');
			for (i=0; i<64; i++) {
				if (linear_to_physical(p)==(ulong)-1) {
					kputc('\n');
					break;
				}
				if (sp>=0xffff) break;
				kputsx(*p++);
				kputc(' ');
				sp+=2;
			}
		}
	}
}

void trap(TrapRegs regs)
{
	int i;
	ulong esp;
	ushort sel,ss,es,ds,fs,gs;

	if (regs.eflags&0x00020000) {
		esp=regs.esp;
		ss=regs.ss;
		es=regs.vm_es;
		ds=regs.vm_ds;
		fs=regs.vm_fs;
		gs=regs.vm_gs;
	} else {
		if (regs.cs&3) {
			esp=regs.esp;
			ss=regs.ss;
		} else {
			ss=({ushort s;asm("movw %%ss,%w0":"=g"(s));s;});
			/*esp=regs.dummy;*/
			esp=(ulong)&regs.esp;
		}
		es=regs.es;
		ds=regs.ds;
		fs=regs.fs;
		gs=regs.gs;
	}
	kinitscr();
	kcls();
	switch (regs.fault) {
	case T_div:
		kputs("\ndivide fault at ");
		break;
	case T_deb:
		kputs("\ndebug trap at ");
		break;
	case T_nmi:
		kputs("\nnon maskable interrupt at ");
		break;
	case T_brk:
		kputs("\nbreakpoint! at ");
		break;
	case T_ovf:
		kputs("\ninteger overflow at ");
		break;
	case T_bnd:
		kputs("\nout of bounds at ");
		break;
	case T_iop:
		kputs("\nillegal opcode at ");
		break;
	case T_cna:
		kputs("\ncoprocessor fault at ");
		break;
	case T_dbl:
		kputs("\ndouble fault!! arghh!! [");
		kputsx(regs.ec);
		kputs("] at ");
		break;
	case T_cso:
		kputs("\n coprocessor segment overrun at ");
		break;
	case T_its:
		kputs("\nillegal task segment [");
		kputsx(regs.ec);
		kputs("] at ");
		break;
	case T_snp:
		kputs("\nsegment not present [");
		kputsx(regs.ec);
		kputs("] at ");
		break;
	case T_stk:
		kputs("\nstack fault [");
		kputsx(regs.ec);
		kputs("] at ");
		break;
	case T_gpf:
		kputs("\ngeneral protection fault [");
		kputsx(regs.ec);
		kputs("] at ");
		break;
	case T_pgf:
		kputs("\npage fault (cr2=");
		kputx( ({int cr2;asm("movl %%cr2,%0":"=r"(cr2)); cr2;}) );
		kputs(", cr3=");
		kputx( ({int cr3;asm("movl %%cr3,%0":"=r"(cr3)); cr3;}) );
		kputs(") [");
		kputsx(regs.ec);
		kputs("] at ");
		break;
	case T_cop:
		kputs("\ncoprocessor error at ");
		break;
	case T_alg:
		kputs("\nalignment check [");
		kputsx(regs.ec);
		kputs("] at ");
		break;
	case T_r0f:
	case T_r12:
	case T_r13:
	case T_r14:
	case T_r15:
	case T_r16:
	case T_r17:
	case T_r18:
	case T_r19:
	case T_r1a:
	case T_r1b:
	case T_r1c:
	case T_r1d:
	case T_r1e:
	case T_r1f:
		kputd(regs.fault);
		kputs(" chto?\n");
		break;
	}
	kputsx(regs.cs);
	kputc(':');
	kputx(regs.eip);
	kputs(".\n");
	kputs("eax="); kputx(regs.eax); kputc(' ');
	kputs("ecx="); kputx(regs.ecx); kputc(' ');
	kputs("edx="); kputx(regs.edx); kputc(' ');
	kputs("ebx="); kputx(regs.ebx); kputc('\n');
	kputs("ebp="); kputx(regs.ebp); kputc(' ');
	kputs("esi="); kputx(regs.esi); kputc(' ');
	kputs("edi="); kputx(regs.edi); kputc('\n');
	kputs("es="); kputsx(es); kputc(' ');
	kputs("ds="); kputsx(ds); kputc(' ');
	kputs("fs="); kputsx(fs); kputc(' ');
	kputs("gs="); kputsx(gs); kputc('\n');
	kputs("cs:eip="); kputsx(regs.cs); kputc(':'); kputx(regs.eip); kputc('\n');
	kputs("eflags="); kputx(regs.eflags);
    kputs((regs.eflags&0x00040000)?" ac":" a-");
    kputs((regs.eflags&0x00020000)?" vm":" v-");
    kputs((regs.eflags&0x00010000)?" rf":" nr");
	kputs((regs.eflags&0x00004000)?" nt":" n-");
	kputc(' '); kputc('i'); kputd((regs.eflags&0x0000300)>>8);
	kputs((regs.eflags&0x00000800)?" of":" no");
	kputs((regs.eflags&0x00000400)?" dn":" up");
	kputs((regs.eflags&0x00000200)?" ie":" id");
	kputs((regs.eflags&0x00000100)?" tf":" t-");
	kputs((regs.eflags&0x00000080)?" ng":" pl");
	kputs((regs.eflags&0x00000040)?" ze":" nz");
	kputs((regs.eflags&0x00000010)?" af":" na");
	kputs((regs.eflags&0x00000004)?" pe":" po");
	kputs((regs.eflags&0x00000001)?" cy":" nc");
	kputc('\n');
	dump_stack(regs.eflags,ss,esp);
	asm("str %0":"=g"(sel));
	if (sel) {
		Descriptor *des=&gdt[sel/8];
		Tss *tss=(Tss*)get_segment_base(des);
		kputs("tss base["); kputsx(sel); kputs("]="); kputx((long)tss); kputc('\n');
		kputs("blink="); kputsx(tss->backlink); kputc('\n');
		kputs("stack 0="); kputsx(tss->ss0); kputc(':'); kputx(tss->esp0); kputc('\n');
		kputs("stack 1="); kputsx(tss->ss1); kputc(':'); kputx(tss->esp1); kputc('\n');
		kputs("stack 2="); kputsx(tss->ss2); kputc(':'); kputx(tss->esp2); kputc('\n');
		kputs("cr3="); kputsx(tss->cr3); kputc('\n');
		kputs("cs:eip="); kputsx(tss->cs); kputc(':'); kputx(tss->eip); kputc('\n');
		kputs("flags="); kputsx(tss->eflags); kputc('\n');
		kputs("eax="); kputx(tss->eax); kputc(' ');
		kputs("ecx="); kputx(tss->ecx); kputc(' ');
		kputs("edx="); kputx(tss->edx); kputc(' ');
		kputs("ebx="); kputx(tss->ebx); kputc('\n');
		kputs("stack="); kputsx(tss->ss); kputc(':'); kputx(tss->esp); kputc('\n');
		dump_stack(tss->eflags,tss->ss,tss->esp);
/*		des=&gdt[tss->ss/8];
		p=(long*)((des->object.base0_15)+
			(des->object.base16_23<<16)+
			(des->object.base24_31<<24)+tss->esp);
		for (i=0; i<16; i++) {
			kputx(*p++);
			if ((i+1)%8) {
				kputc(' ');
			} else {
				kputc('\n');
			}
		}
*/		kputs("ebp="); kputx(tss->ebp); kputc(' ');
		kputs("esi="); kputx(tss->esi); kputc(' ');
		kputs("edi="); kputx(tss->edi); kputc('\n');
		kputs("es="); kputsx(tss->es); kputc(' ');
		kputs("ds="); kputsx(tss->ds); kputc(' ');
		kputs("fs="); kputsx(tss->fs); kputc(' ');
		kputs("gs="); kputsx(tss->gs); kputc('\n');
		kputs("ldtr="); kputsx(tss->ldtr); kputc('\n');
		kputs("trap="); kputs(tss->trap?"on":"off"); kputc('\n');
		kputs("bitmap="); kputsx(tss->bitmap); kputc('\n');
		if (tss->bitmap>104) {
			kputs("misc data\n");
			for (i=104; i<tss->bitmap; i+=4) {
				kputx(*(long*)((char*)tss+i));
				if ((i/4+1)%8) {
					kputc(' ');
				} else {
					kputc('\n');
				}
			}
		}
	}
	intoff();
	for (i=0; i<1325755; i++) {
		while ( (_inb(0x61)&0x10));
		while (!(_inb(0x61)&0x10));
	}
	reboot();
}

void reboot(void)
{
	int k;
	outb(0x3f2,0x00);
	outb(0x64,0xfe);
	for (k=0; k<4000; k++);
	outb(0x64,0xff);
	asm("cli;hlt");
}

static int clk=0;

static void irq_0(void)
{
	static short d[]={0x4e2f,0x4e2d,0x4e5c,0x4e7c};
	*(short*)0xb809e=d[clk&3];
	clk++;
}

static void irq_1(void)
{
	static int k,c=0,a=0;

	kputs("kbd data ");
	kputh(k=inb(0x60));
	kputs(", clk ");
	kputd(clk);
	kputs("\n");
	switch (k) {
	case 0x38:
		a=1;
		break;
	case 0xb8:
		a=0;
		break;
	case 0x1d:
		c=1;
		break;
	case 0x9d:
		c=0;
		break;
	case 0xd3:
		if (c&&a) {
			*(short*)0xb804e=0x4f2a;
			reboot();
		}
		break;
	}
}

static int _ivec;

static void irq_nop()
{
	kputs("intr ");
	kputd(_ivec);
	kputs(", clk ");
	kputd(clk);
	kputs("\n");
}

static void (*handle_interrupt[16])(void)={
	irq_0,	irq_1,	irq_nop,irq_nop,irq_nop,irq_nop,irq_nop,irq_nop,
	irq_nop,irq_nop,irq_nop,irq_nop,irq_nop,irq_nop,irq_nop,irq_nop,
};

void intr(int ivec)
{
	int omsk1=0xff,omsk2=0xff;
	if (ivec>7) {
		omsk2=inb(ICU2+1);
		outb(ICU2+1,omsk2|(1<<(ivec-8)));
	} else {
		omsk1=inb(ICU1+1);
		outb(ICU1+1,omsk1|(1<<ivec));
	}
	inton();
	(handle_interrupt[_ivec=ivec])();
	intoff();
	if (ivec>7) {
		outb(ICU2+1,omsk2);
		outb(ICU2,0x20);
	} else
		outb(ICU1+1,omsk1);
	outb(ICU1,0x20);
}

void (*get_irq_vector(int ivec))(void)
{
	return handle_interrupt[ivec];
}

void set_irq_vector(int ivec, void (*vec)(void))
{
	handle_interrupt[ivec]=vec;
}

static void __attribute__((constructor)) init_traps(void)
{
	k_set_CtrlAltDel(reboot);
}

void enable_irq(uchar irq)
{
	if (irq>15) return;
	if (irq>=8) {
		_outb(ICU1+1,_inb(ICU1+1)&~0x04);  /* enable irq 2 */
		_outb(ICU2+1,_inb(ICU2+1)&~(0x01<<(irq-8)));
	} else {
		_outb(ICU1+1,_inb(ICU1+1)&~(0x01<<irq));
	}
}

void disable_irq(uchar irq)
{
	if (irq>15) return;
	if (irq>=8) {
		/* leave irq 2 alone as the others may still be in use */
		_outb(ICU2+1,_inb(ICU2+1)|(0x01<<(irq-8)));
	} else {
		_outb(ICU1+1,_inb(ICU1+1)|(0x01<<irq));
	}
}
