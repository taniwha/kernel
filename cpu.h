
typedef enum {
	dt_unused,
	dt_tss16,
	dt_ldt,
	dt_busytss16,
	dt_callGate16,
	dt_taskGate,
	dt_interruptGate16,
	dt_trapGate16,
	dt_tss32=9,
	dt_busytss32=11,
	dt_callGate32,
	dt_interruptGate32=14,
	dt_trapGate32=15,
	dt_dataRO,
	dt_dataROA,
	dt_dataRW,
	dt_dataRWA,
	dt_dataROU,
	dt_dataROUA,
	dt_dataRWU,
	dt_dataRWUA,
	dt_codeEO,
	dt_codeEOA,
	dt_codeER,
	dt_codeERA,
	dt_codeEOC,
	dt_codeEOCA,
	dt_codeERC,
	dt_codeERCA,
} DescriptorTypes;

typedef struct {
	unsigned short limit __attribute__((packed));
	unsigned long base __attribute__((packed));
} PseudoDescriptor;

typedef struct {
	unsigned long limit __attribute__((packed));
	unsigned long base __attribute__((packed));
} _PseudoDescriptor;

typedef struct {
	unsigned limit0:16 __attribute__((packed));
	unsigned base0:24 __attribute__((packed));
	DescriptorTypes type:5 __attribute__((packed));
	unsigned dpl:2 __attribute__((packed));
	unsigned present:1 __attribute__((packed));
	unsigned limit1:4 __attribute__((packed));
	unsigned avail:1 __attribute__((packed));
	unsigned :1 __attribute__((packed));
	unsigned use32:1 __attribute__((packed));
	unsigned gran:1 __attribute__((packed));
	unsigned base1:8 __attribute__((packed));
} SegmentDescriptor;

typedef struct {
	unsigned short limit0 __attribute__((packed));
	unsigned long base __attribute__((packed));
	
	DescriptorTypes type:5 __attribute__((packed));
	unsigned dpl:2 __attribute__((packed));
	unsigned present:1 __attribute__((packed));
	
	unsigned limit1:4 __attribute__((packed));
	unsigned avail:1 __attribute__((packed));
	unsigned :1 __attribute__((packed));
	unsigned use32:1 __attribute__((packed));
	unsigned gran:1 __attribute__((packed));
	
} _SegmentDescriptor;

typedef struct {
	unsigned offset0:16 __attribute__((packed));
	unsigned selector:16 __attribute__((packed));
	unsigned dwords:5 __attribute__((packed));
	unsigned :3;
	DescriptorTypes type:5 __attribute__((packed));
	unsigned dpl:2 __attribute__((packed));
	unsigned present:1 __attribute__((packed));
	unsigned offset1:16 __attribute__((packed));
} GateDescriptor;

typedef struct {
	unsigned offset:32 __attribute__((packed));
	
	unsigned selector:16 __attribute__((packed));
	
	DescriptorTypes type:5 __attribute__((packed));
	unsigned dpl:2 __attribute__((packed));
	unsigned present:1 __attribute__((packed));
	
	unsigned dwords:5 __attribute__((packed));
	unsigned :3;
} _GateDescriptor;

typedef union {
	PseudoDescriptor pseudo;	/* only for the null pointer in gdt */
	_PseudoDescriptor _pseudo;
	SegmentDescriptor segment;
	_SegmentDescriptor _segment;
	GateDescriptor gate;
	_GateDescriptor _gate;
} Descriptor;

typedef struct {
	ushort rpl:2;
	ushort ldt:1;
	ushort index:13;
} Selector;

#define MAKE_DESCRIPTOR(b,l,t,pl,p,a,u,g) \
		{limit0:(l)&0xffff,base0:(unsigned long)(b)&0xffffff,type:(t),dpl:(pl),present:(p),limit1:(l)>>16,avail:(a),use32:(u),gran:(g),base1:(unsigned long)(b)>>24}

#define MAKE_GATE(o,s,d,t,pl,p) \
3		{offset0:(unsigned short)(o),selector:(selector),dwords:(d),type:(t),dpl:(pl),present:(p),offset1:(unsigned long)(o)>>16}

/* this is so the first entry of the gdt can be used for the gdt pointer */
#define PSEUDO_DESCRIPTOR(b,l) \
		{limit:(l),base:(unsigned long)(b)}

#define _MAKE_DESCRIPTOR(b,l,t,pl,p,a,u,g) \
		{limit0:(l)&0xffff,base:(unsigned long)(b),type:(t),dpl:(pl),present:(p),limit1:(l)>>16,avail:(a),use32:(u),gran:(g)}

#define _MAKE_GATE(o,s,d,t,pl,p) \
		{offset:(unsigned long)(o),dwords:(d),type:(t),dpl:(pl),present:(p),selector:(s)}

typedef struct {
	unsigned short	backlink;
	unsigned		:16;
	unsigned long	esp0;
	unsigned short	ss0;
	unsigned		:16;
	unsigned long	esp1;
	unsigned short	ss1;
	unsigned		:16;
	unsigned long	esp2;
	unsigned short	ss2;
	unsigned		:16;
	unsigned long	cr3,
					eip,
					eflags,
					eax,
					ecx,
					edx,
					ebx,
					esp,
					ebp,
					esi,
					edi;
	unsigned short	es,
					:16,
					cs,
					:16,
					ss,
					:16,
					ds,
					:16,
					fs,
					:16,
					gs,
					:16,
					ldtr,
					:16,
					trap:1,
					:15,
					bitmap;
} Tss;

#define INTSTACKSPACE 19
typedef struct {
	long	sav_es;
	long	sav_ds;
	long	sav_fs;
	long	sav_gs;
	long	intsp;
	long	intstack[INTSTACKSPACE];
} VMStack;

typedef enum {
	pl0,
	pl1,
	pl2,
	pl3,
} Dpls;



static __inline__
void set_gdt(PseudoDescriptor *gdt)
{
	asm volatile ("lgdt %0" : /* no output */ : "m"(*gdt));
}

static __inline__
void set_idt(PseudoDescriptor *idt)
{
	asm volatile ("lidt %0" : /* no output */ : "m"(*idt));
}

static __inline__
void set_ldt(unsigned short ldt)
{
	asm volatile ("lldt %0" : /* no output */ : "g"(ldt));
}

static __inline__
void set_tr(unsigned short tr)
{
	asm volatile ("ltr %0" : /* no output */ : "r"(tr));
}

static __inline__
void get_gdt(PseudoDescriptor *gdt)
{
	asm volatile ("lgdt %0" : "=m"(*gdt));
}

static __inline__
void get_idt(PseudoDescriptor *idt)
{
	asm volatile ("lidt %0" : "=m"(*idt));
}

static __inline__
unsigned short get_ldt(void)
{
	unsigned short ldt;
	asm volatile ("sldt %0" : "=g"(ldt));
	return ldt;
}

static __inline__
unsigned short get_tr(void)
{
	unsigned short tr;
	asm volatile ("str %0" : "=g"(tr));
	return tr;
}

static __inline__
unsigned long get_segment_base(Descriptor *des)
{
	unsigned long base;
	asm (""
		"movl	%1,%0	# bits 0-23 of base;"
		"shll	$8,%0;"
		"movb	%2,%b0	# bits 24-31 of base;"
		"rorl	$8,%0	# rotate bits into position;"
		""
		:"=&q"(base)
		:"m"(*((char*)des+2)),
		 "m"(*((char*)des+7))
		:"cc"
	);
	return base;
}

static __inline__
char set_bit(void *bit_map, int bit)
{
	char ret;
	asm volatile ("btsl %k1,%2;setc %b0":"=a"(ret):"r"(bit),"m"(*(char*)bit_map):"cc","memory");
	return ret;
}

static __inline__
char reset_bit(void *bit_map, int bit)
{
	char ret;
	asm volatile ("btrl %k1,%2;setc %b0":"=a"(ret):"r"(bit),"m"(*(char*)bit_map):"cc","memory");
	return ret;
}

static __inline__
char test_bit(void *bit_map, int bit)
{
	char ret;
	asm volatile ("btl %k1,%2;setc %b0":"=a"(ret):"r"(bit),"m"(*(char*)bit_map):"cc");
	return ret;
}

#define _inb(port) ({unsigned char dat; asm volatile ("inb %b1,%b0":"=a"(dat):"i"(port)); dat;})
#define _outb(port,dat) asm volatile("outb %b0,%b1": :"a"(dat),"i"(port))

#define _inw(port,dat) ({unsigned short dat;volatile  asm("inw %b1,%w0":"=a"(dat):"i"(port)); dat;})
#define _outw(port,dat) asm volatile("outw %w0,%b1": :"a"(dat),"i"(port))

#define _inl(port,dat) ({unsigned long dat; asm volatile("inl %b1,%k0":"=a"(dat):"i"(port)); dat;})
#define _outl(port,dat) asm volatile("outl %k0,%b1": :"a"(dat),"i"(port))
