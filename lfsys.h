#ifndef __lfsys_h
#define __lfsys_h

typedef struct {
	uchar bootInd;			/* also bios drive number */
	uchar start_head;
	uchar start_sector;		/* (bits 0-5) */
	uchar start_cylinder;	/* (bits 8,9 in bits 6,7 of sector) */
	uchar opsys;			/* see fsys.h */
	uchar end_head;
	uchar end_sector;		/* (bits 0-5) */
	uchar end_cylinder;		/* (bits 8,9 in bits 6,7 of sector) */
	ulong prev_sectors;		/* same as starting sector (LBA) */
	ulong num_sectors;
} PartitionRecord;

typedef struct {
	uchar code[446]						__attribute__((packed));
	PartitionRecord partitionTable[4]	__attribute__((packed));
	ushort magic						__attribute__((packed));
} MasterBootRecord;

typedef struct {
	ulong start_sector;	/* starting sector of partition (0 based) */
	ulong num_sectors;	/* number of sectors allocated to partition */
	uchar type;			/* type of file system on partition (from partition table) */
	uchar hd;			/* physical hard drive this partition is on */
} Partition;

/* Known opsys types */
#define UNUSED		0x00	/* unused partition */
#define DOS_12		0x01	/* msdos 12 bit fat */
#define DOS_16s		0x04	/* msdos 16 bit fat (<32M) */
#define DOS_16b		0x06	/* msdos 16 bit fat (>32M) */
#define DOS_EXT		0x05	/* msdos extended partition */
#define DOS_32		0x0b	/* Windows95 with 32-bit FAT */
#define DOS_32lba	0x0c	/* Windows95 with 32-bit FAT (using LBA-mode INT 13 extensions) */
#define KERNEL_FS	0x07	/* kernel partition, number may need to change */
#define ONTRACK_RO	0x50	/* OnTrack Disk Manager, read-only partition */
#define ONTRACK_RW	0x51	/* OnTrack Disk Manager, read/write partition */
#define ONTRACK_WO	0x53	/* OnTrack Disk Manager, write-only partition??? */
#define ONTRACK_DDO	0x54	/* OnTrack Disk Manager (DDO) */

#define BIOS_MAGIC	0xaa55

int lfsys_write_sectors(int part, ulong sector, ulong count, void *buf);
int lfsys_read_sectors(int part, ulong sector, ulong count, void *buf);
int lfsys_num_partitions(void);
const Partition *lfsys_get_partition(int part);

#endif /*__lfsys_h*/
