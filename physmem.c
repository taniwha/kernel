#include "types.h"
#include "physmem.h"

ulong get_extended_memory(void);

#if 0
/* no longer used, but saved so this code can be used for allocating pages on the
 * swap device
 */
static ulong freePages[32768];			/* 1Mbit (supports 4GB of memory) */
static ulong total_mem;

extern uchar stext[] asm("stext");
extern uchar end[] asm("end");

static inline void _freePage(long page)
{
	asm("btsl %1,%0":"=m"(*freePages):"r"(page));
}

static inline void _allocPage(long page)
{
	asm("btrl %1,%0":"=m"(*freePages):"r"(page));
}

static inline char _pageFree(long page)
{
	char free;
	asm("btl %2,%1; setcb %b0":"=g"(free):"m"(*freePages),"r"(page));
	return free;
}

static inline long _findFreePage(long start)
{
	int i;
	char found;
	ulong pages;
	ulong ind;
	if ((i=start%32)) {
		if ((pages=freePages[start/32]&(((ulong)-1)<<i))) {
			ulong page;
			asm ("bsfl %k1,%k0":"=g"(page):"g"(pages));
			return start/32+page;
		}
		start+=32-i;
	}
	asm(""
		"repz;"
		"scasl;"
		"setnz	%b0;"
		"subl	%k2,%k1;"
		"shrl	$2,%k1;"
		""
		:"=r"(found),"=D"(ind)
		:"i"(freePages),"1"(freePages),"c"((total_mem-start)/32),"a"(0)
		:"%ecx","cc"
	);
	if (found) {
		ulong page;
		asm ("bsfl %k1,%k0":"=g"(page):"g"(freePages[ind]));
		return ind*32+page;
	}
	return -1;
}

void p_free_pages(long page, long count)
{
	if (page<0 || page>=total_mem) return;
	if (page+count>total_mem) count=total_mem-page;
	while (count--) _freePage(page++);
}

long p_allocate_pages(long count, int contiguous)
{
	if (count>1 && contiguous) {
		int page=_findFreePage(0);
		while (page>=0) {
			int i;
			for (i=0; i<count; i++) {
				if (!_pageFree(page+i)) break;
			}
			if (i==count) {
				for (i=0; i<count; i++) {
					_allocPage(page+i);
				}
				return page;
			}
			page=_findFreePage(0);
		}
	} else {
		int page=_findFreePage(0);
		if (page>=0) {
			_allocPage(page);
			return page;
		}
	}
	return -1;
}
#endif

ulong linear_to_physical(void *lin)
{
	ulong phys=pageTables[((ulong)lin)>>12];
	if (phys&1) {
		return phys;							/* includes control bits */
	} else {
		return (ulong)-1;
	}
}

ulong physical_to_linear(ulong phys)
{
	ulong lin;
	phys&=0xfffff000;
	for (lin=0; lin<0x100000; lin++) {
		if ((pageTables[lin]&1) && (pageTables[lin]&0xfffff000)==phys)
			return lin<<12;
	}
	return (ulong)-1;
}

int map_memory(ulong lin, ulong phys, ushort flags, ulong len)
{
	len+=lin;
	len=(len>>12)+((len&0xfff)!=0);
	lin>>=12;
	len-=lin;
	phys=(phys&0xfffff000)|(flags&0x0fff);
	while (len--) {
		pageTables[lin++]=phys;
		phys+=0x1000;
	}
	return 0;
}

#if 0
static void __attribute__((constructor)) valloc_init(void)
{
	ulong i=(get_extended_memory()>>12),j;		/* in pages */
	total_mem=256+i;							/* in pages */
	i-=2;										/* PDT and 1st MB pages */
	j=(end-stext+0xfff)>>12;					/* size of kernel in pages */
	i-=j+((j+1023)>>10);						/* number of page tables for kernel */
	for (j=0; j<i/32; j++) {
		freePages[j+8]=0xff;					/* skip the first meg */
	}
	j=i&~31;
	i&=31;
	while (i--) _freePage(j++);
}
#endif

asm("\n\
		.globl	_pageTables,_pageTableDirectory								\n\
		# Define the location of the page tables in LINEAR address space.	\n\
		.set	_pageTables,			0xffc00000							\n\
		.set	_pageTableDirectory,	0xfffff000							\n\
");
