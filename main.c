#include "types.h"
#include "screen.h"
#include "lfsys.h"
#include "fdc.h"
#include "ide.h"
#include "kmalloc.h"
#include "dos_disk.h"

void dump(int base, uchar *buf)
{
	uchar *b=buf;

	while (buf-b<512) {
		int i;
		kprintf("%hx:",base+(buf-b));
		for (i=0; i<16; i++) {
			kprintf(" %bx",buf[i]);
			if (buf[i]<' ') buf[i]='.';
		}
		kputc(' ');
		for (i=0; i<16; i++) kputc(buf[i]);
		kputc('\n');
		buf+=16;
	}
}

/* this buffer needs to be below 1Mb for dma access (floppy's) to work.
 * you'ld think `they' would have redefined the dma page registers for
 * ATs with their 16Mb address space (the theoretical maximum for 8 bit
 * dmas). I wander if there's a register I have to write to for enabling
 * the upper four bits of the 8 bit dma page registers.
 */
extern MasterBootRecord br;
asm (".set _br,0x90000");

void dump_part_table(int i, ulong sec)
{
	/* NOTE! this will go into an infinite loop if any of the extended
	 * partitions form a loop (same with dos, so if the system works with
	 * dos, no problem)
	 */
	static int level=0;
	int j;

	lfsys_read_sectors(i,sec,1,&br);
	for (j=0; j<4; j++) {
		if (br.partitionTable[j].opsys) {
			int k;
			for (k=0; k<level; k++) kputc(' ');
			kprintf("bi:%bx sh:%bx ss:%bx sc:%bx ty:%bx eh:%bx es:%bx ec:%bx lb:%x sz:%x\n",
				br.partitionTable[j].bootInd,
				br.partitionTable[j].start_head,
				br.partitionTable[j].start_sector,
				br.partitionTable[j].start_cylinder,
				br.partitionTable[j].opsys,
				br.partitionTable[j].end_head,
				br.partitionTable[j].end_sector,
				br.partitionTable[j].end_cylinder,
				br.partitionTable[j].prev_sectors,
				br.partitionTable[j].num_sectors
			);
			if (br.partitionTable[j].opsys==DOS_EXT) {
				level++;
				dump_part_table(i,br.partitionTable[j].prev_sectors);
				level--;
			}
		}
	}
}

void debug_xmm(void);

int main()
{
	int i,num_part=lfsys_num_partitions();

	kinitscr();
	kprintf("Hi There\n");
	for (i=0; i<num_part; i++) {
		const Partition *part=lfsys_get_partition(i);
		switch (part->type) {
		case ONTRACK_RO:
		case ONTRACK_RW:
		case ONTRACK_WO:
			kprintf("p:%d status:%d\n",i,lfsys_read_sectors(i,0,1,&br));
			dump(0,(uchar*)&br);
			break;
		case ONTRACK_DDO:
			for (i=61; i<70; i++) {
				kprintf("sector %d\n",i);
				lfsys_read_sectors(0,i,1,&br);
				dump(0,(uchar*)&br);
			}
			break;
		}
	}
	return 0;
}
