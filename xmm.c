#include "types.h"
#include "xmm.h"
#include "physmem.h"

#include "screen.h"

typedef struct {
	uchar flags;
	uchar lockCount;
	/* The base and len values are in K to be compatable with HIMEM.SYS 3.09+.
	 * That doesn't realy bother me as that allows me to map 4TB (42 bits)
	 * which is a rediculously large amount of memory and I don't think even
	 * the Pentium Pro can directly access this much (I do know about it's
	 * extended page table stuff, but I don't know the details). However,
	 * there is the infamouse "640k ought to do anybody.." quote to consider.
	 */
	ulong base;		/* 32-bit base address in 4kb units */
	ulong len;			/* 32-bit length in 4kb units */
} __attribute__((packed)) XMSHandle;

/* This is used to convert a linear address/length to what's used in the
 * handle structure above.  Currently stores the values in units of 4kb.
 */
#define XMS_shift 12
#define FREEFLAG 1
#define USEDFLAG 2
#define UNUSEDFLAG 4

/* The handle table for XMS memory handles.  The XMS spec does not place a
 * limit on the number of handles (other than the fact that the handles are
 * 16 bit values and thus there is a hard limit of 65536 handles). 1024
 * handles should do for now (I hope).
 */
static XMSHandle handles[1024];
#define NUM_HANDLES (sizeof(handles)/sizeof(handles[0]))
#define RESERVED_HANDLES 2

uchar XMS_error;

ushort allocExtendedMemory(ulong amount)
{
	XMSHandle *freeBlock=0;
	XMSHandle *freeHandle=0;
	int i;

	XMS_error=0;
	/* convert K to whatever base is realy used */
	amount+=(1<<(XMS_shift-10))-1;
	amount>>=XMS_shift-10;
	for (i=0; i<NUM_HANDLES; i++) {
		if (!freeBlock && handles[i].flags==FREEFLAG && handles[i].len>=amount) {
			freeBlock=&handles[i];
		}
		if (!freeHandle && handles[i].flags==UNUSEDFLAG) {
			freeHandle=&handles[i];
		}
	}
	if (freeBlock) {
		if (freeHandle) {
			freeHandle->flags=FREEFLAG;
			freeHandle->base=freeBlock->base+amount;
			freeHandle->len=freeBlock->len-amount;
			freeBlock->len=amount;
		}
		freeBlock->flags=USEDFLAG;
		return freeBlock-handles;
	}
	if (freeHandle) {
		if (amount) {
			XMS_error=XMS_err_out_of_memory;
			return 0;
		} else {
			freeHandle->flags=USEDFLAG;
			return freeHandle-handles;
		}
	}
	XMS_error=XMS_err_out_of_handles;
	return 0;
}

static int validHandle(ushort handle)
{
	if (handle<NUM_HANDLES && handles[handle].flags==USEDFLAG) {
		return 1;
	}
	return 0;
}

static void find_adjoining_free_blocks(XMSHandle *old, XMSHandle **up, XMSHandle **down)
{
	XMSHandle *p=handles;

	*up=*down=0;
	while (p-handles<NUM_HANDLES) {
		if (!*up && p->flags==FREEFLAG && p->base==old->base+old->len) {
			*up=p;
			if (*down) break;
		}
		if (!*down && p->flags==FREEFLAG && p->base+p->len==old->base) {
			*down=p;
			if (*up) break;
		}
	}
}

int freeExtendedMemory(ushort handle)
{
	int i;
	XMSHandle *oldBlock;

	XMS_error=0;
	if (handle<RESERVED_HANDLES || !validHandle(handle)) {
		XMS_error=XMS_err_invalid_handle;
		return 0;
	}
	if (handles[handle].lockCount) {
		XMS_error=XMS_err_emb_locked;
		return 0;
	}
	handles[handle].flags=FREEFLAG;
	handles[handle].lockCount=0;
	oldBlock=&handles[handle];

	for (i=0; i<NUM_HANDLES;) {
		ulong base=handles[handle].base;
		ulong top=base+handles[handle].len;
		XMSHandle *freeBlock=handles;
		
		for (i=0; i<NUM_HANDLES; i++,freeBlock++) {
			if (freeBlock->flags==FREEFLAG) {
				if (top==freeBlock->base) {
					break;
				}
				if (base==freeBlock->base+freeBlock->len) {
					asm("xchgl %k0,%k1":"=r"(freeBlock),"=r"(oldBlock)
									   :"0"(freeBlock),"1"(oldBlock));
					break;
				}
			}
		}
		if (i<NUM_HANDLES) {
			oldBlock->len+=freeBlock->len;
			freeBlock->flags=UNUSEDFLAG;
			freeBlock->base=freeBlock->len=0;
		}
	}
	return 1;
}

ushort reallocExtendedMemory(ushort handle, ulong newSize)
{
	XMSHandle *old,*up,*down;

	XMS_error=0;
	if (!validHandle(handle)) {
		XMS_error=XMS_err_invalid_handle;
		return 0;
	}
	if (handles[handle].lockCount) {
		XMS_error=XMS_err_emb_locked;
		return 0;
	}
	/* convert K to whatever base is realy used */
	newSize+=(1<<(XMS_shift-10))-1;
	newSize>>=XMS_shift-10;
	old=&handles[handle];
	if (old->len==newSize) {
		return handle;
	} else if (old->len>newSize) {
		find_adjoining_free_blocks(old,&up,&down);
		if (up) {
			ulong diff=old->len-newSize;
			old->len=newSize;
			up->len+=diff;
			up->base-=diff;
			return handle;
		} else {
			XMSHandle *p=handles;
			while (p-handles<NUM_HANDLES) {
				if (p->flags==UNUSEDFLAG) {
					ulong diff=old->len-newSize;
					old->len=newSize;
					p->flags=FREEFLAG;
					p->lockCount=0;
					p->len=diff;
					p->base=old->base+newSize;
					return handle;
				}
			}
			XMS_error=XMS_err_out_of_handles;
			return 0;
		}
	} else {
		ulong diff=newSize-old->len;
		find_adjoining_free_blocks(old,&up,&down);
		if (up && up->len>=diff) {
			if (up->len>diff) {
				up->len-=diff;
				up->base+=diff;
			} else {
				up->flags=UNUSEDFLAG;
			}
			old->len=newSize;
			return handle;
		} else if (down && (down->len+old->len+(up?up->len:0)>=newSize)) {
			ulong oldLen=old->len;
			ulong oldBase=old->base;
			if (up) {
				old->len+=up->len;
				up->flags=UNUSEDFLAG;
			}
			old->len+=down->len;
			old->base=down->base;
			down->lockCount++;
			old->lockCount++;
			moveExtendedMemory(oldLen<<XMS_shift,
							   handle,(oldBase-oldBase)<<XMS_shift,
							   handle,0);
			down->lockCount--;
			old->lockCount--;
			diff=old->len-newSize;
			old->len=newSize;
			if (diff) {
				down->base=old->base+newSize;
				down->len=diff;
			} else {
				down->flags=UNUSEDFLAG;
			}
			return handle;
		} else {
			XMSHandle tmp;
			ushort newHandle=allocExtendedMemory(newSize);
			XMSHandle *new=&handles[newHandle];
			if (!newHandle) {
				XMS_error=XMS_err_out_of_memory;
				return 0;
			}
			new->lockCount++;
			old->lockCount++;
			moveExtendedMemory(old->len<<XMS_shift,
							   handle,0,
							   handle,0);
			tmp=*old;
			*old=*new;
			*new=tmp;
			new->lockCount--;
			old->lockCount--;
			freeExtendedMemory(newHandle);
			return handle;
		}
	}
}

ulong lockExtendedMemory(ushort handle)
{
	XMS_error=0;
	if (!validHandle(handle)) {
		XMS_error=XMS_err_invalid_handle;
		return 0;
	}
	if (handles[handle].lockCount==0xff) {
		XMS_error=XMS_err_lock_overflow;
		return 0;
	}
	/* reserved handles are permanently locked, so don't bother incrementing
	 * the lock count.
	 */
	if (handle>=RESERVED_HANDLES) {
		handles[handle].lockCount++;
	}
	return handles[handle].base<<XMS_shift;
}

int unlockExtendedMemory(ushort handle)
{
	XMS_error=0;
	if (!validHandle(handle)) {
		XMS_error=XMS_err_invalid_handle;
		return 0;
	}
	if (!handles[handle].lockCount) {
		XMS_error=XMS_err_emb_unlocked;
		return 0;
	}
	/* reserved handles are permanently locked, so don't bother decrementing
	 * the lock count (this would unlock the handle when lockCount reaches 0,
	 * which is undesirable.
	 */
	if (handle>=RESERVED_HANDLES) {
		handles[handle].lockCount--;
	}
	return 1;
}

int moveExtendedMemory(ulong len, ushort srcH, ulong srcO, ushort dstH, ulong dstO)
{
	XMSHandle *src=&handles[srcH];
	XMSHandle *dst=&handles[dstH];
	ulong d,s;
	
	XMS_error=0;
	if (!validHandle(srcH)) {
		XMS_error=XMS_err_sh_invalid;
		return 0;
	}
	if (!validHandle(dstH)) {
		XMS_error=XMS_err_dh_invalid;
		return 0;
	}
	if (len&1 || len>src->len || len>dst->len) {
		XMS_error=XMS_err_len_invalid;
		return 0;
	}
	if (srcO>src->len<<XMS_shift || srcO+len>src->len<<XMS_shift)
	{
		XMS_error=XMS_err_so_invalid;
		return 0;
	}
	if (dstO>dst->len<<XMS_shift || dstO+len>dst->len<<XMS_shift)
	{
		XMS_error=XMS_err_do_invalid;
		return 0;
	}
	d=(dst->base<<XMS_shift)+dstO;
	s=(src->base<<XMS_shift)+srcO;
	if (s>d) {
		asm volatile ("cld;rep;movsl":"=S"(s),"=D"(d):"0"(s),"1"(d),"c"(len/4):"%ecx");
		if (len&2) {
			asm volatile ("movsw":"=S"(s),"=D"(d):"0"(s),"1"(d));
		}
	} else if (s<d) {
		s+=len;
		d+=len;
		asm volatile ("std");
		if (len&2) {
			asm volatile ("movsw":"=S"(s),"=D"(d):"0"(s),"1"(d));
		}
		asm volatile ("rep;movsl;cld":"=S"(s),"=D"(d):"0"(s),"1"(d),"c"(len/4):"%ecx");
	}
	return 1;
}

extern uchar stext[] asm("stext");

static void __attribute__((constructor)) init_xmm(void)
{
	/* Get the end of available extended memory. Memory has been allocated so that
	 * the kernel code and static data (including the stack) are at the very top of
	 * extended memory, the page tables for the kernel and the first Mb of memory
	 * are immediately below the kernel, and the page table directory is immediately
	 * below the page tables.  This means that the start of already allocated extended
	 * memory (and all non-existent memory) starts at the physical address of the page
	 * table directory. This has been setup this way in init.S.
	 *
	 * NOTE: linear_to_physical returns the control bits of the physical page
	 * associated with the linear address and these must be masked off.
	 */
	ulong end_phys=linear_to_physical(pageTableDirectory)&~0xfff;
	int i;

	for (i=0; i<NUM_HANDLES; i++) {
		handles[i].flags=UNUSEDFLAG;
	}
	/* create a handle to describe the first Mb of memory and thus make it unavailable
	 * for allocation.
	 */
	handles[0].base=0;
	handles[0].len=0x100000>>XMS_shift;	/* 1Mb */
	handles[0].lockCount=1;
	handles[0].flags=USEDFLAG;
	/* create a handle to describe both the physical memory used by the kernel and the
	 * page tables and also all non-existant memory.  This memory is also unavailable
	 * for allocation.
	 */
	handles[1].base=end_phys>>XMS_shift;
	handles[1].len=((ulong)-end_phys)>>XMS_shift;
	handles[1].lockCount=1;
	handles[1].flags=USEDFLAG;
	/* create a handle to describe the free extended memory. This starts at
	 * 1MB and extends to the physical address of the page table directory.
	 */
	handles[RESERVED_HANDLES].base=0x100000>>XMS_shift;
	handles[RESERVED_HANDLES].len=(end_phys-0x100000)>>XMS_shift;
	handles[RESERVED_HANDLES].lockCount=0;
	handles[RESERVED_HANDLES].flags=FREEFLAG;
}

void debug_xmm(void)
{
	int i;

	for (i=0; i<NUM_HANDLES; i++) {
		if (handles[i].flags!=UNUSEDFLAG) {
			kprintf("%hx %d %d %x %x (%d Mb %dkb)\n",i,
				handles[i].flags,handles[i].lockCount,
				handles[i].base,handles[i].len,
				handles[i].len>>8,(handles[i].len&0xff)<<2);
		}
	}
}
