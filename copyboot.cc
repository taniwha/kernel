#include <iostream.h>
#include <dpmi.h>
#include <bios.h>
#include <stdio.h>
#include <malloc.h>
#include "types.h"
#include "lfsys.h"
#include "bc_fsys.h"

class DriveCtl {
	ushort spt;		/* number of sectors per track						*/
	ushort tpc;		/* number of tracks per cylinder					*/
	ushort cyls;	/* number of cylinders on hd (is this needed? maybe	*/
					/* for partition validation?)						*/
	uchar drv_num;	/* bios drive number								*/
	ulong start;	/* beginning lba of `drive' (for partitions)		*/
	ulong size;		/* size of `drive' (for partitions)					*/
	uchar type;		/* os type (0xff used for floppies)					*/
	void lba_to_chs(ulong lba, int &c, int &h, int &s)
	{
		s=lba%spt+1;
		int t=lba/spt;
		h=t%tpc;
		c=t/tpc;
	}
	int xfer(int op, void *buff, ulong lba, ulong cnt);
public:
	DriveCtl(int biosDrive, int partition=0);
	int read(void *buff, ulong lba, ulong cnt)
	{
		return xfer(2,buff,lba,cnt);
	}
	int write(void *buff, ulong lba, ulong cnt)
	{
		return xfer(3,buff,lba,cnt);
	}
	ushort sectors(){return spt;}
	ushort tracks(){return tpc;}
	ushort cylinders(){return cyls;}
	uchar drive(){return drv_num;}
	ulong part_start(){return start;}
	ulong part_size(){return size;}
	uchar os_type(){return type;}

	friend inline ostream &operator << (ostream &, const DriveCtl &);
};

inline ostream &operator << (ostream &os, const DriveCtl &d)
{
	os<<'('<<d.spt<<','
		   <<d.tpc<<','
		   <<d.cyls<<','
		   <<(unsigned)d.drv_num<<','
		   <<d.start<<','
		   <<d.size<<','
		   <<(unsigned)d.type<<')';
	return os;
}

DriveCtl::DriveCtl(int biosDrive, int partition=0)
{
	__dpmi_regs regs;
	regs.h.ah=8;
	regs.h.dl=biosDrive;
	__dpmi_int(0x13,&regs);
	if (!(regs.x.flags&1) && (biosDrive&0x7f)<regs.h.dl) {
		spt=regs.h.cl&0x3f;
		tpc=regs.h.dh+1;
		cyls=(regs.h.ch+((regs.h.cl&0xc0)<<2))+1;
		start=0;
		size=cyls*tpc*spt;
		type=KERNEL_FS;
		if ((drv_num=biosDrive)>=0x80) {
			MasterBootRecord mbr;
			read(&mbr,0,1);
			if (partition>=0 && partition<4 && mbr.partitionTable[partition].opsys) {
				// limit access to the specified parition;
				start=mbr.partitionTable[partition].prev_sectors;
				size=mbr.partitionTable[partition].num_sectors;
				type=mbr.partitionTable[partition].opsys;
			} else {
				spt=tpc=cyls=drv_num=start=size=type=0;	// make partition unaccessable
			}
		}
	} else {
		spt=tpc=cyls=drv_num=start=size=type=0;	// make partition unaccessable
	}
}

int DriveCtl::xfer(int op, void *buf, ulong lba, ulong cnt)
{
	int c,h,s;
	if (lba>=size || lba+cnt>size)
		return -1;
	lba_to_chs(start+lba,c,h,s);
	while (cnt) {
		// if you get write errors when writing to a floppy, comment out the
		// following line and uncomment the next.  It seems that some systems
		// can't write to sector 18 with more than one sector (eg start at
		// sector 17 with 2 sectors). This seems to be a bug in m$dos, as
		// it works correctly with OpenDos.
		unsigned n=spt+1-s;
		//unsigned n=spt-s;
		if (n<1) n=1;
		if (n>cnt) n=cnt;
		if (n>18) n=18;	//djgpp has a hard limit on the number of sectors
		int err;
		for (int i=0; i<3 && (err=biosdisk(op,drv_num,h,c,s,n,buf)); i++)
			if (i==2) return err;
		buf+=n*512;
		s+=n;
		if (s>spt) {
			s=1;
			if (++h>=tpc) {
				h=0;
				c++;
			}
		}
		cnt-=n;
	}
	return 0;
}

DriveCtl *floppies[4];		// support up to 4 floppy drives
DriveCtl *hardDrives[16];	// support up to 4 hard drives with 4 partitions each

int main(int argc, char **argv)
{
	char *bootRecord="bootstrap.bin";
	char *bootFile="boot";
	DriveCtl **drive=&floppies[0];						// default to a:
	int verbose=0;

	for (int i=1; i<argc; i++) {
		if (argv[i][0]=='-') {
			switch (argv[i][1]) {
			case 'b':
				if (argv[i][2]) {
					bootRecord=&argv[i][2];
				} else {
					bootRecord=argv[++i];
				}
				break;
			case 'f':
				if (argv[i][2]) {
					bootFile=&argv[i][2];
				} else {
					bootFile=argv[++i];
				}
				break;
			case 'p': {
				int drv;
				char *p;
				if (argv[i][2]) {
					drv=strtol(argv[i]+2,&p,0);
				} else {
					drv=strtol(argv[++i],&p,0);
				}
				if (drv>=20 || drv<0) {
					fprintf(stderr,"%s: invalid drive/parition (%d)\n",argv[0],drv);
					return 1;
				}
				if (drv<4) {
					drive=&floppies[drv];
				} else {
					drive=&hardDrives[drv-4];
				}
				break;}
			case 'v':
				verbose++;
				break;
			default:
				fprintf(stderr,"%s: invalid arguement `%s'\n",argv[0],argv[i]);
				return 1;
			}
		} else {
			fprintf(stderr,"%s: invalid arguement `%s'\n",argv[0],argv[i]);
			return 1;
		}
	}

	for (int i=0; i<4; i++) {
		floppies[i]=new DriveCtl(i);
		if (verbose>1)
			cout<<*floppies[i]<<endl;
	}
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			hardDrives[i*4+j]=new DriveCtl(0x80+i,j);	// 0x80->bios hard drives
			if (verbose>1)
				cout<<*hardDrives[i*4+j]<<endl;
		}
	}

	if (verbose) {
		cout<<"Writing `"<<bootRecord<<"' & `"<<bootFile<<"' to drive "<<**drive<<endl;
	}

	if ((*drive)->os_type()!=KERNEL_FS) {
		fprintf(stderr,"%s: invalid drive/parition (not kernel fs)\n",argv[0]);
		return 1;
	}

	FILE *file=0;
	long len;
	void *buf;
	BC_BootRecord boot;

	if (!(file=fopen(bootFile,"rb"))) {
		fprintf(stderr,"could not read `%s'\n",bootFile);
		return 1;
	}
	fseek(file,0,SEEK_END);
	len=ftell(file);
	fseek(file,0,SEEK_SET);
	buf=malloc(len);
	fread(buf,1,len,file);
	fclose(file);
	if (!(file=fopen(bootRecord,"rb"))) {
		fprintf(stderr,"could not read `%s'\n",bootRecord);
		return 1;
	}
	fread(&boot,1,512,file);
	fclose(file);
	len=(len+511)/512;
	if (verbose) {
		cout<<"`"<<bootFile<<"' is "<<len<<" sector";
		if (len!=1) cout<<"s long"<<endl;
		else cout<<" long"<<endl;
	}
	boot.load_sector=(*drive)->part_start()+1;
	boot.load_count=len;
	boot.sectors_per_track=(*drive)->sectors();
	boot.tracks_per_cylinder=(*drive)->tracks();
	boot.boot_drive=(*drive)->drive();
	int err;
	if ((err=(*drive)->write(&boot,0,1))) {
		fprintf(stderr,"write error (%d)\n",err);
		return 1;
	}
	if ((err=(*drive)->write(buf,1,len))) {
		fprintf(stderr,"write error (%d)\n",err);
		return 1;
	}
	return 0;
}
