#ifndef __msd_fsys_h
#define __msd_fsys_h
/* MS DOS file system (FAT 12/16) */

typedef struct {
	ushort bps		__attribute__((packed));
	uchar spc		__attribute__((packed));
	ushort nrsec	__attribute__((packed));
	uchar nfat		__attribute__((packed));
	ushort nrde		__attribute__((packed));
	ushort stsec	__attribute__((packed));
	uchar media		__attribute__((packed));
	ushort spf		__attribute__((packed));
	ushort spt		__attribute__((packed));
	ushort nhds		__attribute__((packed));
	ulong nhsec		__attribute__((packed));
	ulong tsec		__attribute__((packed));

	uchar drive		__attribute__((packed));
	uchar rsvd		__attribute__((packed));
	uchar sig		__attribute__((packed));
	ulong volID		__attribute__((packed));
	char label[11]	__attribute__((packed));
	char type[8]	__attribute__((packed));
/*	uchar unk[6]	__attribute__((packed));
	ushort ncyl		__attribute__((packed));
	uchar type		__attribute__((packed));
	ushort attrs	__attribute__((packed));*/
} MSD_BPB;

#define DBRCSize 512-3-8-sizeof(MSD_BPB)-2

typedef struct {
	uchar jmp[3]			__attribute__((packed));
	char OEMname[8]			__attribute__((packed));
	MSD_BPB bpb				__attribute__((packed));
	uchar code[DBRCSize]	__attribute__((packed));
	ushort magic			__attribute__((packed));
} DOSBootRecord;

typedef struct {
	int partition;	/* lfsys partition number */
	ulong lba;		/* relative to the start of the partition (0 for DOS primary) */
	ulong size;		/* from the partition table, get dos size from BPB. This is max. */
	ulong fat;		/* starting sector of FAT */
	ulong fat_size;	/* size of fat in sectors */
	ulong root;		/* starting sector of root dir */
	ulong root_size;/* size of root directory in sectors */
	ulong data;		/* starting sector of data (cluster number 2) */
	ulong data_size;/* size of data area in clusters */
	MSD_BPB bpb;	/* copy of the volume's BPB from the boot sector */
} MSD_Volume;

#endif __msd_fsys_h
