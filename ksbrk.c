#include "types.h"
#include "xmm.h"
#include "physmem.h"

extern uchar end[] asm("end");

static uchar *end_of_memory=end;

void *ksbrk(int delta)
{
	void *ret=end_of_memory;
	ushort handle;
	ulong base;

	/* if delta is 0, just return the current end of memory */
	if (!delta) {
		return end_of_memory;
	}
	/* align delta to 4k */
	delta=(delta+0xfff)&~0xfff;
	if (end_of_memory+delta>(uchar*)pageTables) {
		return 0;
	}
	if (!(handle=allocExtendedMemory(delta/1024))) {
		return 0;
	}
	if (!(base=lockExtendedMemory(handle))) {
		freeExtendedMemory(handle);
		return 0;
	}
	map_memory((ulong)end_of_memory,base,3,delta);
	end_of_memory+=delta;
	return ret;
}
