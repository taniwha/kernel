typedef struct {
	unsigned
		limit0_15:16,
		base0_15:16,
		base16_23:8,
		accessed:1,
		type:3,
		segment:1,
		dpl:2,
		present:1,
		limit16_19:4,
		avail:1,
		:1,
		native:1,
		granularity:1,
		base24_31:8;
} TSSegment;

typedef struct {
	unsigned
		limit0_15:16,
		base0_15:16,
		base16_23:8,
		type:4,
		segment:1,
		dpl:2,
		present:1,
		limit16_19:4,
		avail:1,
		:2,
		granularity:1,
		base24_31:8;
} TSObject;

typedef struct {
	unsigned
		offset0_15:16,
		selector:16,
		count:5,
		:3,
		type:4,
		segment:1,
		dpl:2,
		present:1,
		offset16_31:16;
} TSGate;

typedef union {
	TSSegment	segment;
	TSObject	object;
	TSGate		gate;
} TSDescriptor;

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
} TSTss;

#define INTSTACKSPACE 19
typedef struct {
	long	sav_es;
	long	sav_ds;
	long	sav_fs;
	long	sav_gs;
	long	intsp;
	long	intstack[INTSTACKSPACE];
} TSVMStack;

typedef enum {
	DataRO,
	DataRW,
	DataRWD=3,
	CodeEO,
	CodeER,
	CodeEOC,
	CodeERC,
} ESegmentTypes;

typedef enum {
	Invalid,
	TSS286,
	LDT,
	TSS286B,
	CallGate286,
	TaskGate,
	IntGate286,
	TrapGate286,
	TSS386=9,
	TSS386B=11,
	CallGate386,
	IntGate386=14,
	TrapGate386,
} EObjectTypes;

typedef enum {
	pl0,
	pl1,
	pl2,
	pl3,
} EDpls;
