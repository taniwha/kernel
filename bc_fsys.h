#ifndef __bc_fsys_h
#define __bc_fsys_h

typedef struct {
	ulong kmagic;
	#define KMAGIC 0xd08ec031 /* xor ax,ax; mov ss,ax */
	uchar code[0x1dc];
	ulong root_sector;
	ulong root_size;
	/* the following forms the last 16 bytes of the sector */
	ulong secmap_sector;
	ulong secmap_size;
	ulong load_sector;
	ushort load_count;
	ushort sectors_per_track;
	ushort tracks_per_cylinder;
	uchar boot_drive;
	uchar rsvd[3];
	ushort magic;
} BC_BootRecord;

typedef struct {
	int partition;		/* lfsys partition number */
	ulong size;			/* In sectors. From the partition table. */
	ulong root;			/* Location of root directory */
	ulong root_size;	/* Size of root directory */
	ulong secmap;		/* Location of sector allocation bitmap */
	ulong secmap_size;	/* Size of sector allocation bitmap */
} BC_Volume;

typedef struct {
	ulong sectors[127];	/* list of sectors allocated to the file. 0 indicated eof */
	ulong next;			/* relative lba of next sector list if non-zero, else end */
} BC_SectorList;

#endif//__bc_fsys_h
