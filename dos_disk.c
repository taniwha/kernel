#include "types.h"
#include "kmalloc.h"
#include "rmcb.h"
#include "lfsys.h"
#include "fdc.h"
#include "dos.h"
#include "dos_disk.h"
#include "screen.h"

typedef struct {
	ushort spt;
	ushort hpc;
} FloppyParams;

typedef struct {
	ulong sector __attribute__((packed));
	ushort count __attribute__((packed));
	FarPointer buffer __attribute__((packed));
} DosDiskRWPacket;

static FloppyParams floppy[2];
static DOS_Volume fvolumes[2];

static int num_volumes;
static DOS_Volume *volumes;
extern union {	/* sectors buffer for general/floppy use */
	uchar sb[512];
	MasterBootRecord mbr;
	DOSBootRecord dbr;
} sector_buffer[4];
asm(".set _sector_buffer,0x800");

static void setup_volume(int partition, ulong lba, ulong size, DOS_Volume *volume)
{
	volume->partition=partition;
	volume->lba=lba;
	if (size) {
		volume->size=size;
	} else {
		if (sector_buffer[0].dbr.bpb.stsec) {
			volume->size=sector_buffer[0].dbr.bpb.stsec;
		} else {
			volume->size=sector_buffer[0].dbr.bpb.tsec;
		}
	}
	if (sector_buffer[0].dbr.magic==BIOS_MAGIC) {
		ulong size;
		memcpy(&volume->bpb,&sector_buffer[0].dbr.bpb,sizeof(DOS_BPB));
		volume->fat=volume->bpb.nrsec;
		if (!(volume->fat_size=volume->bpb.spf)) {
			/* FAT32 */
			volume->fat_size=volume->bpb.x.f32.spf32;
			volume->fat_type=3;	/* 32 bit fat (is this always true??) */
		}
		volume->clust0=volume->fat+volume->fat_size*volume->bpb.nfat;
		if (!(volume->clust0_size=(volume->bpb.nrde+15)/16)) {
			volume->root=volume->bpb.x.f32.rdirclust;
		} else {
			volume->root=0;
		}
		volume->data=volume->clust0+volume->clust0_size;
		if (!(size=volume->bpb.stsec))
			size=volume->bpb.tsec;
		if (size>volume->size)
			size=volume->size;
		size-=volume->data;
		volume->data_size=size/volume->bpb.spc;
		if (!volume->fat_type) {
			if (volume->data_size<0x0ff0);
		}
	} else {
		const Partition *part=lfsys_get_partition(partition);
		/* unformatted (?) partition. cannot access it yet */
		volume->fat_type=0;
		volume->fat=0;
		volume->fat_size=0;
		volume->clust0=0;
		volume->clust0_size=0;
		volume->root=0;
		volume->data=0;
		volume->data_size=0;
		memset(&volume->bpb,0,sizeof(DOS_BPB));
		kprintf("bad volume (%d) not accessable until formatted\n",num_volumes);
		if (size) {
			kprintf("part:%d (lba:%d sz:%d type:%bx hd:%d)\n",
				partition,
				part->start_sector,
				part->num_sectors,
				part->type,
				part->hd
			);
		}
	}
	kprintf("vol:%d on pt:%d (lba:%d sz:%d fat:%d fsz:%d root:%d rsz:%d data:%d dsz:%d)\n",
		num_volumes,
		volume->partition,
		volume->lba,
		volume->size,
		volume->fat,
		volume->fat_size,
		volume->clust0,
		volume->clust0_size,
		volume->data,
		volume->data_size
	);
}

static ushort update_floppy(int drive)
{
	int res;
	ushort spt,hpc;

	if ((res=fdc_read(drive,0,0,1,1,&sector_buffer[0])))
		return res;
	setup_volume(drive,0,0,&fvolumes[drive]);
	spt=sector_buffer[0].dbr.bpb.spt;
	hpc=sector_buffer[0].dbr.bpb.nhds;
	if (fvolumes[drive].size%(spt*hpc))
		return 0x0c;	/* ?? */
	floppy[drive].spt=spt;
	floppy[drive].hpc=hpc;
	return 0;
}

ushort dos_read_sectors(int drive, ulong sector, ulong count, void *buf)
{
	if (drive<0 || drive>=num_volumes+2) {
		return 0x0201;
	}
	if (drive<2) {
		ulong cyl;
		ulong head;
		ulong sec;
		ulong cnt;
		int res;

		if (!floppy[drive].spt) {
			res=update_floppy(drive);
			if (res) return res;
		}
		sec=(sector%floppy[drive].spt)+1;
		head=sector/floppy[drive].spt;
		cyl=head/floppy[drive].hpc;
		head=head%floppy[drive].hpc;
		cnt=floppy[drive].spt-sec+1;
		do {
			if (cnt>count) cnt=count;
			do {
				res=fdc_read(drive,cyl,head,sec,cnt,buf);
				if (res==0x06) {
					res=update_floppy(drive);
					if (!res) res=0x06;
				}
			} while (res!=0x06);
			if (!res) {
				buf+=512*cnt;
				count-=cnt;
				sec=1;
				if (++head>=floppy[drive].hpc) {
					head=0;
					cyl++;
				}
			}
		} while (!res && count);
		return res;
	} else {
		drive-=2;
		if (sector>=volumes[drive].size || sector+count>volumes[drive].size) {
			return 0x020c;
		}
		return lfsys_read_sectors(volumes[drive].partition,
								  sector+volumes[drive].lba,count,buf);
	}
}

ushort dos_write_sectors(int drive, ulong sector, ulong count, void *buf)
{
	if (drive<0 || drive>=num_volumes+2) {
		return 0x0201;
	}
	if (drive<2) {
		ulong cyl;
		ulong head;
		ulong sec;
		ulong cnt;
		int res;

		if (!floppy[drive].spt) {
			res=update_floppy(drive);
			if (res) return res;
		}
		sec=(sector%floppy[drive].spt)+1;
		head=sector/floppy[drive].spt;
		cyl=head/floppy[drive].hpc;
		head=head%floppy[drive].hpc;
		cnt=floppy[drive].spt-sec+1;
		do {
			if (cnt>count) cnt=count;
			do {
				res=fdc_write(drive,cyl,head,sec,cnt,buf);
				if (res==0x06) {
					res=update_floppy(drive);
					if (!res) res=0x06;
				}
			} while (res!=0x06);
			if (!res) {
				buf+=512*cnt;
				count-=cnt;
				sec=1;
				if (++head>=floppy[drive].hpc) {
					head=0;
					cyl++;
				}
			}
		} while (!res && count);
		return res;
	} else {
		drive-=2;
		if (sector>=volumes[drive].size || sector+count>volumes[drive].size) {
			return 0x020c;
		}
		return lfsys_write_sectors(volumes[drive].partition,
								   sector+volumes[drive].lba,count,buf);
	}
}

static void disk_return(RMCB_Regs *regs)
{
	ushort *stack=(ushort*)MK_FP(regs->ss,regs->esp&0xffff);
	regs->eip&=~0xffff;
	regs->eip|=stack[0];
	regs->cs=stack[1];
	regs->esp+=4;
}

static void read_sectors(RMCB_Regs *regs)
{
	uchar drive;
	ulong sector;
	ushort count;
	void *buffer;

	asm("sti");
	regs->eflags&=~1;	/* clear carry flag */
	drive=regs->eax&0xff;
	if ((regs->ecx&0xffff)==0xffff) {
		DosDiskRWPacket *packet=(DosDiskRWPacket*)MK_FP(regs->ds,regs->ebx&0xffff);
		sector=packet->sector;
		count=packet->count;
		buffer=MK_FP(packet->buffer.segment,packet->buffer.offset);
	} else {
		sector=regs->edx&0xffff;
		count=regs->ecx&0xffff;
		buffer=MK_FP(regs->ds,regs->ebx&0xffff);
		if (drive>=2 && drive<num_volumes+2 && volumes[drive-2].size>0xffff) {
			dos_error(regs,0x0207);	/* use new style call */
		}
	}
	if (!(regs->eflags&1)) {
		ushort ax=dos_read_sectors(drive,sector,count,buffer);
		regs->eax&=~0xffff;
		regs->eax|=ax;
		if (ax)
			regs->eflags|=1;
	}
	disk_return(regs);
}

static void write_sectors(RMCB_Regs *regs)
{
	uchar drive;
	ulong sector;
	ushort count;
	void *buffer;

	asm("sti");
	regs->eflags&=~1;	/* clear carry flag */
	drive=regs->eax&0xff;
	if ((regs->ecx&0xffff)==0xffff) {
		DosDiskRWPacket *packet=(DosDiskRWPacket*)MK_FP(regs->ds,regs->ebx&0xffff);
		sector=packet->sector;
		count=packet->count;
		buffer=MK_FP(packet->buffer.segment,packet->buffer.offset);
	} else {
		sector=regs->edx&0xffff;
		count=regs->ecx&0xffff;
		buffer=MK_FP(regs->ds,regs->ebx&0xffff);
		if (drive>=2 && drive<num_volumes+2 && volumes[drive-2].size>0xffff) {
			dos_error(regs,0x0207);	/* use new style call */
		}
	}
	if (!(regs->eflags&1)) {
		ushort ax=dos_write_sectors(drive,sector,count,buffer);
		regs->eax&=~0xffff;
		regs->eax|=ax;
		if (ax)
			regs->eflags|=1;
	}
	disk_return(regs);
}

static void scan_volumes(int partition, ulong sec, int fill)
{
	/* NOTE! this will go into an infinite loop if any of the extended
	 * partitions form a loop (same with dos, so if the system works with
	 * dos, no problem).  Actually, as this is a recursive function, the kernel will
	 * get a fault (ill,segv or page) as the stack grows into the code/data space.
	 */
	int ind;
	PartitionRecord partitionTable[4];

	lfsys_read_sectors(partition,sec,1,&sector_buffer[0]);
	/* make a local copy of the partition table so it doesn't get scragged when we
	 * recurse.
	 */
	memcpy(partitionTable,sector_buffer[0].mbr.partitionTable,sizeof(partitionTable));
	for (ind=0; ind<4; ind++) {
		switch (partitionTable[ind].opsys) {
		case DOS_EXT:
			scan_volumes(partition,partitionTable[ind].prev_sectors,fill);
			break;
		case DOS_12:
		case DOS_16s:
		case DOS_16b:
		case DOS_32:
		case DOS_32lba:
			if (fill) {
				ulong lba=partitionTable[ind].prev_sectors+sec;
				lfsys_read_sectors(partition,lba,1,&sector_buffer[0]);
				setup_volume(partition,lba,partitionTable[ind].num_sectors,
							 &volumes[num_volumes]);
			} else if (ind>=2) {
				kprintf("Is this normal? (num_volumes=%d, index=%d)\n",num_volumes,ind);
			}
			num_volumes++;
			break;
		case UNUSED:
			break;
		default:
			if (!fill) {
				kprintf("Unexpected file system (%bx) ignored.\n",
					partitionTable[ind].opsys);
			}
			break;
		}
	}
}

static void __attribute__((constructor)) dos_disk_init(void)
{
	int num_partitions=lfsys_num_partitions();
	int partition;
	/* hook int 25 and int 26 read/write absolute vectors */
	set_rmcb_int(0x25,read_sectors);
	set_rmcb_int(0x26,write_sectors);

	for (partition=0; partition<num_partitions; partition++) {
		const Partition *part=lfsys_get_partition(partition);
		switch (part->type) {
		case DOS_12:
		case DOS_16s:
		case DOS_16b:
		case DOS_32:
		case DOS_32lba:
			num_volumes++;
			break;
		case DOS_EXT:
			scan_volumes(partition,0,0);	/* count volumes */
			break;
		}
	}
	if (!num_volumes) return;	/* nothing for us */
	kprintf("found %d ms-dos volume%s\n",num_volumes,num_volumes==1?"":"s");
	volumes=(DOS_Volume*)kmalloc(num_volumes*sizeof(DOS_Volume));
	num_volumes=0;
	for (partition=0; partition<num_partitions; partition++) {
		const Partition *part=lfsys_get_partition(partition);
		switch (part->type) {
		case DOS_12:
		case DOS_16s:
		case DOS_16b:
		case DOS_32:
		case DOS_32lba:
			lfsys_read_sectors(partition,0,1,&sector_buffer[0]);
			setup_volume(partition,0,part->num_sectors,&volumes[num_volumes]);
			num_volumes++;
			break;
		case DOS_EXT:
			scan_volumes(partition,0,1);	/* collect volume information */
			break;
		}
	}
}
