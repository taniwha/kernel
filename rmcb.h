#ifndef __rmcb_h
#define __rmcb_h

typedef struct {
	ushort offset	__attribute__((packed));
	ushort segment	__attribute__((packed));
} FarPointer;

typedef struct {
	ulong edi;
	ulong esi;
	ulong ebp;
	ulong :32;		/* esp at pushall */
	ulong ebx;
	ulong edx;
	ulong ecx;
	ulong eax;
	ulong eip;
	ushort cs;
	ushort :16;
	ulong eflags;
	ulong esp;
	ushort ss;
	ushort :16;
	ushort es;
	ushort :16;
	ushort ds;
	ushort :16;
	ushort fs;
	ushort :16;
	ushort gs;
	ushort :16;
} RMCB_Regs;

void set_rmcb_int(int intr, void (*hook)(RMCB_Regs*));
void (*get_rmcb_int(int intr))(RMCB_Regs*);

#define RMCB_int_seg 0x60		/* segment of int nn hooks */

#endif	__rmcb_h
