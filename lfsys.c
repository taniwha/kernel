#include "types.h"
#include "ide.h"
#include "fdc.h"
#include "lfsys.h"
#include "kmalloc.h"
#include "screen.h"
#include "s386.h"

static int num_hd;
static int num_part;
static Partition *partitions;

static void __attribute__((constructor)) lfsys_init(void)
{
	MasterBootRecord bootRecord;
	int drive,part_rec,part_num;

	if (!(num_hd=ide_num_drives())) return;
	for (drive=0; drive<num_hd; drive++) {
		if (ide_read_sectors(drive,0,1,&bootRecord) || bootRecord.magic!=BIOS_MAGIC) {
			/* no valid boot records on this drive */
			continue;
		}
		
		for (part_rec=0; part_rec<4; part_rec++) {
			if (bootRecord.partitionTable[part_rec].opsys &&
					(char)bootRecord.partitionTable[part_rec].bootInd<=0) {
/*				ulong stSec;*/
				/* map all types, just some may not be able to be accessed. */
/*				kprintf("primary:%c type:%bx start:%d size:%d (%d Mb)\n",
					bootRecord.partitionTable[part_rec].bootInd?'y':'n',
					bootRecord.partitionTable[part_rec].opsys,
					stSec=bootRecord.partitionTable[part_rec].prev_sectors,
					bootRecord.partitionTable[part_rec].num_sectors,
					(bootRecord.partitionTable[part_rec].num_sectors/2+512)/1024
				);*/
				/* don't care at this point if the info in the boot block is invalid as
				 * there is no way of telling at this level of the file system as
				 * the formats are very OS dependent.
				 */
				num_part++;
			}
		}
	}
	if (!num_part) return;
	if (!(partitions=(Partition*)kmalloc(sizeof(Partition)*num_part))) {
		return;
	}
	for (drive=part_num=0; drive<num_hd; drive++) {
		if (ide_read_sectors(drive,0,1,&bootRecord) || bootRecord.magic!=BIOS_MAGIC) {
			/* no valid boot records on this drive */
			continue;
		}
		for (part_rec=0; part_rec<4; part_rec++) {
			if (bootRecord.partitionTable[part_rec].opsys &&
					(char)bootRecord.partitionTable[part_rec].bootInd<=0) {
/*				int stSec;*/
				/* map all types, just some may not be able to be accessed. */
				partitions[part_num].start_sector=
					bootRecord.partitionTable[part_rec].prev_sectors;
				partitions[part_num].num_sectors=
					bootRecord.partitionTable[part_rec].num_sectors;
				partitions[part_num].type=bootRecord.partitionTable[part_rec].opsys;
				partitions[part_num].hd=drive;
/*				kprintf("start:%d size:%d type:%bx drive:%d\n",
					stSec=partitions[part_num].start_sector,
					partitions[part_num].num_sectors,
					partitions[part_num].type,
					partitions[part_num].hd
				);*/
				part_num++;
			}
		}
	}
}

int lfsys_write_sectors(int part, ulong sector, ulong count, void *buf)
{
	if (part<0 || part>=num_part) return -1;
	if (sector>=partitions[part].num_sectors ||
		sector+count>partitions[part].num_sectors) return -2;
	return ide_write_sectors(partitions[part].hd,
							 sector+partitions[part].start_sector,count,buf);
}

int lfsys_read_sectors(int part, ulong sector, ulong count, void *buf)
{
	if (part<0 || part>=num_part) return -1;
	if (sector>=partitions[part].num_sectors ||
		sector+count>partitions[part].num_sectors) return -2;
	return ide_read_sectors(partitions[part].hd,
							sector+partitions[part].start_sector,count,buf);
}

int lfsys_num_partitions(void)
{
	return num_part;
}

const Partition *lfsys_get_partition(int part)
{
	if (part<0 || part>=num_part) return 0;
	return &partitions[part];
}
