#include "types.h"
#include "screen.h"
#include "isa.h"
#include "s386.h"
#include "cpu.h"
#include "ide.h"
#include "kmalloc.h"

#define DEBUG 0

#if DEBUG
#define DP(v, str, args...) do {if ((!v)||(DEBUG&(v))) kprintf(str, ## args);} while(0)
#else
#define DP(v, str, args...)
#endif

typedef struct {
	ushort config;
	ushort num_cylinders;
	ushort :16;
	ushort num_heads;
	ushort :16;
	ushort :16;
	ushort num_sectors;
	ushort :16;
	ushort :16;
	ushort :16;
	ushort serial_number[10];
	ushort buffer_type;
	ushort buffer_size;
	ushort long_bytes;
	ushort firmware_revision[4];
	ushort model_number[20];
	ushort multiple_support;
	ushort dword_transfers;
	ushort capablities;
	ushort security;
	ushort PIOtiming;
	ushort DMAtiming;
	ushort field_validity;
	ushort current_logical_cylinders;
	ushort current_logical_heads;
	ushort current_logical_sectors;
	ulong current_total_sectors;
	ushort multiple_sector;
	ulong total_LBA_sectors;
	ushort single_word_DMA;
	ushort multiword_DMA;
	ushort PIO_modes;
	ushort multiword_DMA_min_time;
	ushort multiword_DMA_mid_time;
	ushort nfc_PIO_min_time;
	ushort PIO_min_time;
	ushort :16;
	ushort :16;
	ushort :16;
	ushort :16;
	ushort :16;
	ushort :16;
	ushort :16;
	ushort :16;
	ushort :16;
	ushort :16;
	ushort :16;
	ushort spec_revision;
	ushort feature_set_1;
	ushort feature_set_2;
	ushort r1[45];
	ushort security_status;
	ushort vs[31];
	ushort r2[96];
} __attribute__((packed)) IDEInfo;

typedef struct {
	ulong totalSectors;
	ushort secs;
	ushort heads;
	ushort cyls;
	uchar maxRead;
	uchar lba;
} DriveData;

typedef struct DiskOp {
	struct DiskOp *next;
	void *buffer;
	uchar command;
	uchar drive;
	volatile ushort status;
	ulong sector;
	uchar nsectors;
} DiskOp;

typedef struct {
	uchar lastStatus;
	uchar lastError;
	DiskOp *diskOp;
} ControllerStatus;

static DriveData *driveData[4];
static int driveMap[4]={-1,-1,-1,-1};
static ControllerStatus contStatus[2];

static int wait_for_ide_idle(int base)
{
	ulong i=331439;/* about 5 seconds */
	uchar e;
	uchar c=_inb(0x61)&0x10;

	while (1) {
		e=inb(IDE_STATUS(base));
		if (!(e&0x80)||(e&1)) return e;
		if (c && !(_inb(0x61)&0x10)) {
			c=0;
			if (!--i) return -1;
		} else if (!c && (_inb(0x61)&0x10)) {
			c=1;
			if (!--i) return -1;
		}
	}
}

static int wait_for_ide_buffer(int base)
{
	ulong i=331439;/* about 5 seconds */
	uchar e;
	uchar c=_inb(0x61)&0x10;

	while (1) {
		e=inb(IDE_STATUS(base));
		if ((e&0x08)||(e&1)) return e;
		if (c && !(_inb(0x61)&0x10)) {
			c=0;
			if (!--i) return -1;
		} else if (!c && (_inb(0x61)&0x10)) {
			c=1;
			if (!--i) return -1;
		}
	}
}

static char *getascii (ushort *in_data, int off_start, int off_end)
{
  static char ret_val [255];
  int loop, loop1;

  for (loop = off_start, loop1 = 0; loop <= off_end; loop++)
	{
	  ret_val [loop1++] = (char) (in_data [loop] / 256);  /* Get High byte */
	  ret_val [loop1++] = (char) (in_data [loop] % 256);  /* Get Low byte */
	}
  ret_val [loop1] = '\0';  /* Make sure it ends in a NULL character */
  return (ret_val);
}

static int read_ide_info(void)
{
	int drive;

	for (drive=0; drive<4; drive++) {
		IDEInfo ideInfo;
		int e;
		int base=(drive&2)?IDE2:IDE1;

		driveData[drive]=0;

		if ((e=wait_for_ide_idle(base))<0 || (e&1)) {
			continue;
		}
		outb(IDE_DRIVE_HEAD(base),(drive&1)?0xf0:0xe0);
		outb(IDE_COMMAND(base),0xec);
		e=wait_for_ide_idle(base);
		if (e<0 || !(e&0x08) || (e&0x21)) {
			/* timed out or sector buffer doesn't need servicing (implies no drive as the
			 * sector buffer always needs servicing after this command), or an error
			 * occured.
			 */
			if (e&1) {
				inb(IDE_ERROR(base));
			}
			continue;
		}

		insw(IDE_DATA(base),&ideInfo,256);

		if (!(driveData[drive]=(DriveData*)kmalloc(sizeof(DriveData)))) {
			return 0;
		}
		memset(driveData[drive],0,sizeof(DriveData));
		driveData[drive]->secs=ideInfo.num_sectors;
		driveData[drive]->heads=ideInfo.num_heads;
		driveData[drive]->cyls=ideInfo.num_cylinders;
		if (ideInfo.total_LBA_sectors) {
			driveData[drive]->totalSectors=ideInfo.total_LBA_sectors;
			driveData[drive]->lba=1;
		} else {
			driveData[drive]->totalSectors=driveData[drive]->secs*
										   driveData[drive]->heads*
										   driveData[drive]->cyls;
			driveData[drive]->lba=0;
		}
		DP(1,"%bx ",ideInfo.multiple_sector);
		if (ideInfo.multiple_sector&0x80) {
			if (!(driveData[drive]->maxRead=ideInfo.multiple_sector&0x7f))
				driveData[drive]->maxRead=1;
		} else {
			driveData[drive]->maxRead=1;
		}
		if (!(driveData[drive]->maxRead=ideInfo.multiple_support&0x7f))
			driveData[drive]->maxRead=1;
	}
	return 1;
}

static int setup_command(DiskOp *dOp,int base)
{
	DriveData *dd=driveData[dOp->drive];
	ulong sector;
	ulong cyl;
	ulong head;

	DP(2,"\ndisk op:(%x) cmd:%bx drv:%d sec:%d nsec:%d\n",
		dOp->buffer,
		dOp->command,
		dOp->drive,
		dOp->sector,
		dOp->nsectors
	);
	switch (dOp->command) {
	case 0x20:	/* read sectors with retry */
	case 0xc4:	/* read multiple */
	case 0x30:	/* write sectors with retry */
	case 0xc5:	/* write multiple */
		DP(4,"nsec=%bx ",dOp->nsectors);
		outb(IDE_SECTOR_COUNT(base),dOp->nsectors);
	case 0x70:	/* seek */
		if (dd->lba) {
			sector=(dOp->sector    )&0x00ff;
			cyl   =(dOp->sector>> 8)&0xffff;
			head  =(dOp->sector>>24)&0x000f;
			head|=0x40;		/* set lba mode */
		} else {
			sector=(dOp->sector%dd->secs)+1;
			head=dOp->sector/dd->secs;
			cyl=head/dd->heads;
			head%=dd->heads;
		}
		outb(IDE_CONTROL,head&0x08);
		head|=0xa0|((dOp->drive&1)<<4);
		DP(4,"sec=%bx cyll=%bx cylh=%bx drvh=%bx\n",sector,cyl&0xff,(cyl>>8)&0xff,head);
		outb(IDE_WPC(base),0);	/* ?? all cylinders have pre-comp. seems to be default */
		outb(IDE_SECTOR_NUMBER(base),sector);
		outb(IDE_CYLINDER_LOW(base),cyl&0xff);
		outb(IDE_CYLINDER_HIGH(base),(cyl>>8)&0xff);
		outb(IDE_DRIVE_HEAD(base),head);
		break;
#if 0
	case 0xc8:	/* read DMA */
	case 0xca:	/* write DMA */
		break;
#endif
	}
/*	wait_for_ide_idle(base);*/
	outb(IDE_COMMAND(base),dOp->command);
	switch (dOp->command) {
	case 0x30:	/* write sectors with retry */
		if (wait_for_ide_buffer(base)>=0) {
			int c=256;
			DP(4,"\nwriting data (%d,%d)",cont,c);
			outsw(IDE_DATA(base),dOp->buffer,c);
		}
		break;
	case 0xc5:	/* write multiple */
		if (wait_for_ide_buffer(base)>=0) {
			int c=dOp->nsectors*256;
			DP(4,"\nwriting data (%d,%d)",cont,c);
			outsw(IDE_DATA(base),dOp->buffer,c);
		}
		break;
	}
	return 0;
}

static void ide_isr(int cont)
{
	int base=cont?IDE2:IDE1;
	ushort stat;

	intoff();
	wait_for_ide_idle(base);
	stat=contStatus[cont].lastStatus=inb(IDE_STATUS(base));
	if ((contStatus[cont].lastStatus&0x31)!=0x10) {
		contStatus[cont].lastError=inb(IDE_ERROR(base));
		stat|=contStatus[cont].lastError<<8;
		if (contStatus[cont].diskOp) {
			contStatus[cont].diskOp->status=stat;
			if ((contStatus[cont].diskOp=contStatus[cont].diskOp->next)) {
				setup_command(contStatus[cont].diskOp,base);
			}
		}
	} else {
		contStatus[cont].lastError=0;
		if (contStatus[cont].diskOp) {
			switch (contStatus[cont].diskOp->command) {
			case 0x20:	/* read sectors with retry */
				if (wait_for_ide_buffer(base)) {
					int c=256;
					/*DP("\nreading data (%d,%d)",cont,c);*/
					insw(IDE_DATA(base),contStatus[cont].diskOp->buffer,c);
					contStatus[cont].diskOp->nsectors--;
					contStatus[cont].diskOp->buffer+=c*2;
				}
				break;
			case 0xc4:	/* read multiple */
				if (wait_for_ide_buffer(base)) {
					int c=contStatus[cont].diskOp->nsectors*256;
					DP(4,"\nreading data (%d,%d)",cont,c);
					insw(IDE_DATA(base),contStatus[cont].diskOp->buffer,c);
					contStatus[cont].diskOp->nsectors=0;
					contStatus[cont].diskOp->buffer+=c*2;
				}
				break;
			case 0x30:	/* write sectors with retry */
				contStatus[cont].diskOp->nsectors--;
				contStatus[cont].diskOp->buffer+=512;
				if (contStatus[cont].diskOp->nsectors) {
					if (wait_for_ide_buffer(base)>=0) {
						int c=256;
						DP(4,"\nwriting data (%d,%d)",cont,c);
						outsw(IDE_DATA(base),contStatus[cont].diskOp->buffer,c);
					}
				}
				break;
			case 0xc5:	/* write multiple */
				contStatus[cont].diskOp->nsectors=0;
				contStatus[cont].diskOp->buffer+=contStatus[cont].diskOp->nsectors*512;
				break;
			case 0x70:	/* seek */
				DP(4,"\nseek done (%d)",cont);
				break;
#if 0
			case 0xc8:	/* read DMA */
			case 0xca:	/* write DMA */
				break;
#endif
			}
			if (!contStatus[cont].diskOp->nsectors) {
				/* indicate operation complete */
				contStatus[cont].diskOp->status=stat;
				if ((contStatus[cont].diskOp=contStatus[cont].diskOp->next)) {
					setup_command(contStatus[cont].diskOp,base);
				}
			}
		}
	}
	DP(4,"\ncontroller %d status %hx\n",cont,stat);
/*	_outb(0x61,_inb(0x61)^2);*/
}

static void ide_isr_1(void)
{
	ide_isr(0);
}

static void ide_isr_2(void)
{
	ide_isr(1);
}

static void __attribute__((constructor)) init_ide(void)
{
	int i,j;
	void *p=getascii;
	p=p;

	read_ide_info();
	for (i=j=0; i<4; i++) {
		if (driveData[i]) {
			driveMap[j++]=i;
			DP(0,"%d %d cyls, %d heads %d, secs (%d total) lba=%c multi=",i,
				driveData[i]->cyls,
				driveData[i]->heads,
				driveData[i]->secs,
				driveData[i]->totalSectors,
				driveData[i]->lba?'y':'n'
			);
			if (driveData[i]->maxRead>1) {
				DP(0,"%d\n",driveData[i]->maxRead);
			} else {
				DP(0,"n\n");
			}
		}
	}
	if (driveData[0] || driveData[1]) {
		inb(IDE_STATUS(IDE1));
		inb(IDE_ERROR(IDE1));
		set_irq_vector(14,ide_isr_1);
		enable_irq(14);
	}
	if (driveData[2] || driveData[3]) {
		inb(IDE_STATUS(IDE2));
		inb(IDE_ERROR(IDE2));
		set_irq_vector(15,ide_isr_2);
		enable_irq(15);
	}
}

static int do_operation(int command, int drive, ulong sector, int count, void *buff)
{
	int drv;
	int cnt;
	int e;

	if (drive>4 || (drv=driveMap[drive])<0 || sector>driveData[drv]->totalSectors)
		return -2;
	cnt=(count+driveData[drv]->maxRead-1)/driveData[drv]->maxRead;
	if (cnt) {
		DiskOp op[cnt];
		int i;
		for (i=0; i<cnt; i++) {
			int c=driveData[drv]->maxRead;
			if (c>count) c=count;
			op[i].next=(DiskOp*)(i<(cnt-1)?&op[i+1]:0);
			op[i].buffer=buff;
			op[i].command=command;
			op[i].drive=drive;
			op[i].status=0;
			op[i].sector=sector;
			op[i].nsectors=c;
			count-=c;
			buff+=512*c;
			sector+=c;
			DP(0,"op %d:%x %x %bx %d %hx %d %d\n",i,
				op[i].next,
				op[i].buffer,
				op[i].command,
				op[i].drive,
				op[i].status,
				op[i].sector,
				op[i].nsectors
			);
		}
		DP(1,"waiting for other ops to finish\n");
		{
			DiskOp *volatile* d=&contStatus[drv/2].diskOp;
			while (*d);
		}
		contStatus[drv/2].diskOp=op;
		if ((e=setup_command(op,drv/2?IDE2:IDE1))<0 || (e&1)) {
			contStatus[drv/2].diskOp=0;
			return e|(inb(IDE_ERROR(drv/2?IDE2:IDE1))*256);
		}
		for (i=0; i<cnt; i++) {
			DP(0,"waiting op %d of %d to finish\n",i+1,cnt);
			while(!op[i].status);
			if ((op[i].status&0x31)!=0x10) return op[i].status;
		}
	}
	return 0;
}

int ide_num_drives(void)
{
	int num=0;
	while (num<4 && driveMap[num]>=0) num++;
	return num;
}

int ide_read_sectors(int drive, ulong sector, int count, void *buff)
{
	return do_operation(0x20,drive,sector,count,buff);
}

int ide_write_sectors(int drive, ulong sector, int count, void *buff)
{
	return do_operation(0x30,drive,sector,count,buff);
}
