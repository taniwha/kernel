#define MK_FP(s,o) ((void*)(((s)<<4)|(o)))

#define DOS_MEM_FIRST_FIT	0x00
#define DOS_MEM_BEST_FIT	0x01
#define DOS_MEM_LAST_FIT	0x02
#define DOS_MEM_HIGH_ONLY	0x40
#define DOS_MEM_HIGH_FIRST	0x80

void dos_allocate_memory(RMCB_Regs *regs);
void dos_free_memory(RMCB_Regs *regs);
void dos_resize_memory(RMCB_Regs *regs);
void dos_memory_allocation_strategy(RMCB_Regs *regs);

static inline void dos_error(RMCB_Regs *regs, ushort error_code)
{
	regs->eflags|=1;
	regs->eax&=~0xffff;
	regs->eax|=error_code;
}

extern ushort dos_first_mcb;
extern ushort dos_current_psp;
