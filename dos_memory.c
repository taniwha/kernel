#include "types.h"
#include "xmm.h"
#include "physmem.h"
#include "rmcb.h"
#include "dos.h"

typedef struct {
	uchar type		__attribute__((packed));
	ushort psp		__attribute__((packed));
	ushort size		__attribute__((packed));
	uchar res[3]	__attribute__((packed));
	uchar name[8]	__attribute__((packed));
} DOS_MCB;

static ushort strategy;
static uchar link_state;
static DOS_MCB *umb_link=MK_FP(0x9fff,0);

static int find_block(RMCB_Regs *regs, DOS_MCB *mcb, ushort *bx)
{
	ushort size=regs->ebx&0xffff;	/* number of paragraphs to allocate */
	ushort largest=0;
	ushort closest=0xffff;
	DOS_MCB *best=0;
	DOS_MCB *last=0;

	while (1) {
		if (mcb->type!='M' && mcb->type!='Z') {
			dos_error(regs,7); /* memory control block destroyed */
			return 0;
		}
		if (!mcb->psp) {
			DOS_MCB *next;
			for (next=mcb+mcb->size+1; !next->psp; next=mcb+mcb->size+1) {
				mcb->size+=next->size+1;
				mcb->type=next->type;
			}
			if (mcb->size>largest)
				largest=mcb->size;
			if (mcb->size>=size) {
				ushort diff=mcb->size-size;
				last=mcb;
				if (diff<closest) {
					closest=diff;
					best=last;
				}
				if (strategy&(DOS_MEM_BEST_FIT|DOS_MEM_LAST_FIT)) {
					if (!diff && !(strategy&DOS_MEM_LAST_FIT))
						break;
				} else {
					break;
				}
			}
			if (mcb->type=='Z')
				break;
			mcb+=mcb->size+1;	/* user size plus 1 para for mcb */
		}
	}
	if ((strategy&DOS_MEM_BEST_FIT)) {
		mcb=best;
	} else {
		mcb=last;
	}
	if (mcb && !mcb->psp) {
		if (mcb->size>=size) {
			mcb->psp=dos_current_psp;
			if (mcb->size>size) {
				DOS_MCB *split=mcb+size+1;
				split->type=mcb->type;
				split->psp=0;
				split->size=mcb->size-size-1;
				mcb->type='M';
				mcb->size=size;
			}
			regs->eax&=~0xffff;
			regs->eax|=(ushort)(((ulong)(mcb+1))>>4);
			return 1;
		}
	}
	*bx=largest;
	return 0;
}

void dos_allocate_memory(RMCB_Regs *regs)
{
	ushort largest;
	if (umb_link && strategy&(DOS_MEM_HIGH_FIRST|DOS_MEM_HIGH_ONLY)) {
		if (find_block(regs,umb_link,&largest))
			return;
	}
	if (!(regs->eflags&1)) {
		if (!(strategy&DOS_MEM_HIGH_ONLY)) {
			if (find_block(regs,(DOS_MCB*)MK_FP(dos_first_mcb,0),&largest))
				return;
		}
	}
	/* don't return `no memory' if `corrupted' has already been set */
	if (!(regs->eflags&1)) {
		dos_error(regs,8); /* insufficient memory */
		regs->ebx&=~0xffff;
		regs->ebx|=largest;
	}
}

void dos_free_memory(RMCB_Regs *regs)
{
	DOS_MCB *mcb=MK_FP(regs->es-1,0);

	if (!regs->es || (mcb->type!='M' && mcb->type!='Z') || !mcb->psp) {
		dos_error(regs,9); /* memory block address invalid */
	} else {
		mcb->psp=0;
	}
}

void dos_resize_memory(RMCB_Regs *regs)
{
	DOS_MCB *mcb=MK_FP(regs->es-1,0);
	ushort size=regs->ebx&0xffff;	/* number of paragraphs for new size */

	if (!regs->es || (mcb->type!='M' && mcb->type!='Z') || !mcb->psp) {
		dos_error(regs,9); /* memory block address invalid */
	} else {
		if (size==mcb->size)	/* no-op */
			return;
		/* Now merge in all adjoining free blocks immediatedly above this one.
		 */
		if (mcb->type!='Z') {
			DOS_MCB *next;
			for (next=mcb+mcb->size+1; !next->psp; next=mcb+mcb->size+1) {
				mcb->size+=next->size+1;
				mcb->type=next->type;
			}
		}
		/* Is it the required size?
		 */
		if (size==mcb->size)
			return;
		/* Nope, if it's big enough, split it creating a new free block
		 */
		if (size<mcb->size) {
			DOS_MCB *split=mcb+size+1;
			split->type=mcb->type;
			split->psp=0;
			split->size=mcb->size-size-1;
			mcb->type='M';
			mcb->size=size;
			return;
		}
		/* Not as big as requested. Return the new size.
		 */
		regs->ebx&=!0xffff;
		regs->ebx|=mcb->size;
		dos_error(regs,8); /* insufficient memory */
	}
}

void dos_memory_allocation_strategy(RMCB_Regs *regs)
{
	uchar al=regs->eax&0xff;

	switch (al) {
	case 0x00:	/* get allocation strategy */
		regs->eax&=~0xffff;
		regs->eax|=strategy;
		break;
	case 0x01:	/* set allocation strategy */
		strategy=regs->ebx&0xffff;
		break;
	case 0x02:	/* get UMB link state */
		regs->eax&=~0xff;
		regs->eax|=link_state;
		break;
	case 0x03:	/* set UMB link state */
		if (umb_link) {
			link_state=(regs->ebx&0xffff)!=0;
			if (link_state) {
				umb_link->type='M';
			} else {
				umb_link->type='Z';
			}
			break;
		}
		/* if the umb doesn't exist, set state is not supported */
		/* fall through */
	default:
		dos_error(regs,1);	/* function number invalid */
		break;
	}
}

static void __attribute__((constructor)) init_dos_memory(void)
{
	ushort size;
	ushort handle;
	/* Place the first memory control block at 4k */
	DOS_MCB *mcb=MK_FP(dos_first_mcb=0x0100,0);
	/* initialize the first memory control block */
	mcb->type='M';							/* first block */
	mcb->psp=0;								/* no owner, free */
	mcb->size=umb_link-mcb-1;				/* 0x01010-0x9ffe0 inclusive */
	/* initialize the upper memory link control block. It will skip over the
	 * video memory addresses (0xa0000-0xc7fff) marking them as allocated and
	 * owned by dos.  As the default umb link state is unlinked, mark it as
	 * the last block in the memory chain.
	 */
	umb_link->type='Z';						/* last block (unlinked) */
	umb_link->psp=8;						/* owned by dos */
	umb_link->size=(DOS_MCB*)MK_FP(0xc800,0)-umb_link-1;
	umb_link->name[0]='S';
	umb_link->name[1]='C';
	umb_link->name[2]=umb_link->name[3]=0;
	umb_link->name[4]=umb_link->name[5]=umb_link->name[6]=umb_link->name[7]=0;
	/* attempt to initialize the upper memory control block. If no physical
	 * memory can be allocated, remove the umb link.
	 */
	mcb=umb_link+umb_link->size+1;
	size=(DOS_MCB*)MK_FP(0xe000,0)-mcb;
	if ((handle=allocExtendedMemory(size*16/1024))) {
		ulong base=lockExtendedMemory(handle);
		map_memory((ulong)mcb,base,3,size*16);
		mcb->type='Z';
		mcb->psp=0;
		mcb->size=size-1;
	} else {
		mcb=MK_FP(dos_first_mcb,0);
		mcb->type='Z';	/* turn into last block */
		mcb->size++;	/* expand over umb link */
	}
}
