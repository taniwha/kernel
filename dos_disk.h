#ifndef __dos_disk_h
#define __dos_disk_h
/* MS DOS file system (FAT 12/16/32) */

typedef struct {
	ushort bps					__attribute__((packed));
	uchar spc					__attribute__((packed));
	ushort nrsec				__attribute__((packed));
	uchar nfat					__attribute__((packed));
	ushort nrde					__attribute__((packed));
	ushort stsec				__attribute__((packed));
	uchar media					__attribute__((packed));
	ushort spf					__attribute__((packed));
	ushort spt					__attribute__((packed));
	ushort nhds					__attribute__((packed));
	ulong nhsec					__attribute__((packed));
	ulong tsec					__attribute__((packed));
	union {
		struct {
			uchar drive			__attribute__((packed));
			uchar rsvd			__attribute__((packed));
			uchar sig			__attribute__((packed));
			ulong volID			__attribute__((packed));
			char label[11]		__attribute__((packed));
			char type[8]		__attribute__((packed));
		} f16;
		struct {
			/* the following is for FAT32 volumes */
			ulong spf32			__attribute__((packed));
			ushort activeFAT:4	__attribute__((packed));
			ushort :3			__attribute__((packed));
			ushort noMirror:1	__attribute__((packed));
			ushort :8			__attribute__((packed));
			ushort fsvers		__attribute__((packed));
			ulong rdirclust		__attribute__((packed));
			ulong fsisec		__attribute__((packed));
			/* ^^ RB says WORD, but things seem to dissagree */
			ushort reserved[6]	__attribute__((packed));

			uchar drive			__attribute__((packed));
			uchar rsvd			__attribute__((packed));
			uchar sig			__attribute__((packed));
			ulong volID			__attribute__((packed));
			char label[11]		__attribute__((packed));
			char type[8]		__attribute__((packed));
		} f32;
	} x							__attribute__((packed));
} DOS_BPB;

#define DBRCSize 512-3-8-sizeof(DOS_BPB)-2

typedef struct {
	uchar jmp[3]			__attribute__((packed));
	char OEMname[8]			__attribute__((packed));
	DOS_BPB bpb				__attribute__((packed));
	uchar code[DBRCSize]	__attribute__((packed));
	ushort magic			__attribute__((packed));
} DOSBootRecord;

typedef struct {
	int partition;	/* lfsys partition number */
	ulong lba;		/* relative to the start of the partition (0 for DOS primary) */
	ulong size;		/* from the partition table, get dos size from BPB. This is max. */
	int fat_type;	/* 0=unknown, 1=12 bit, 2=16 bit, 3=32 bit */
	ulong fat;		/* starting sector of FAT */
	ulong fat_size;	/* size of fat in sectors */
	ulong clust0;	/* stating sector of cluster 0 (root dir on fat12/16) */
	ulong clust0_size;	/* size of cluster 0 (root dir on fat12/16) */
	ulong root;		/* starting cluster of root dir (0 for fat12/16, >=2 for fat32) */
	ulong data;		/* starting sector of data section (cluster number 2) */
	ulong data_size;/* size of data area in clusters */
	DOS_BPB bpb;	/* copy of the volume's BPB from the boot sector */
} DOS_Volume;

ushort dos_read_sectors(int drive, ulong sector, ulong count, void *buf);
ushort dos_write_sectors(int drive, ulong sector, ulong count, void *buf);

#endif __dos_disk_h
